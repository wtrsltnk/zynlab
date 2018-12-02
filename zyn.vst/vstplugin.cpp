#include "vstplugin.h"
#include <fstream>
#include <memory>   // For std::unique_ptr
#include <stdarg.h> // For va_start, etc.
#include <cmath>

std::string string_format(const std::string fmt_str, ...)
{
    int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
    std::unique_ptr<char[]> formatted;
    va_list ap;
    while (1)
    {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return std::string(formatted.get());
}
static std::mutex logMutex;

void write_text_to_log_file(const std::string &text)
{
//    std::lock_guard<std::mutex> lock(logMutex);

//    std::ofstream log_file(
//        "c:\\temp\\log_file.txt", std::ios_base::out | std::ios_base::app);
//    log_file << text << std::endl;
}

VstPlugin::VstPlugin(audioMasterCallback audioMaster, VstInt32 numPrograms, VstInt32 numParams)
    : AudioEffectX(audioMaster, numPrograms, numParams), _lastGeneratedBufferSize(0), _lastSampleFrames(0)
{
    Config::Current().init();

    /* Get the settings from the Config*/
    settings.samplerate = Config::Current().cfg.SampleRate;
    settings.buffersize = Config::Current().cfg.SoundBufferSize;
    settings.oscilsize = Config::Current().cfg.OscilSize;
    settings.alias();

    _tmpoutr = new float[settings.buffersize];
    _tmpoutl = new float[settings.buffersize];
    _mixedOutR = new float[settings.buffersize * 4];
    _mixedOutL = new float[settings.buffersize * 4];

    write_text_to_log_file("constructor");
}

VstPlugin::~VstPlugin()
{
    delete[] _tmpoutl;
    delete[] _tmpoutr;
    delete[] _mixedOutR;
    delete[] _mixedOutL;
}

void VstPlugin::processReplacing(float ** /*inputs*/, float **outputs, VstInt32 sampleFrames)
{
    write_text_to_log_file(string_format("processReplacing with %d sampleFrames", sampleFrames));

    std::lock_guard<std::mutex> lock(_notesMutex);

    if (playingNotes.empty())
    {
        // make sure we are quite when no notes are playing
        for (unsigned int i = 0; i < settings.buffersize * 4; i++)
        {
            _mixedOutL[i] = _mixedOutR[i] = 0;
        }
        outputs[0] = &_mixedOutL[0];
        outputs[1] = &_mixedOutR[0];

        return;
    }

    if (_lastGeneratedBufferSize != 0 && _lastSampleFrames != 0)
    {
        // Move all samples left from last time to the front of the buffer
        auto offset = _lastGeneratedBufferSize - _lastSampleFrames;

        write_text_to_log_file(string_format(" moving %d left over samples to the front of the buffer", offset));
        for (unsigned int i = 0; i < offset; i++)
        {
            _mixedOutL[i] = _mixedOutL[static_cast<unsigned int>(sampleFrames) + i];
            _mixedOutR[i] = _mixedOutR[static_cast<unsigned int>(sampleFrames) + i];
        }
        _lastGeneratedBufferSize = offset;
    }

    while (_lastGeneratedBufferSize < static_cast<unsigned int>(sampleFrames))
    {
        write_text_to_log_file(string_format(" resetting samples %d to %d", _lastGeneratedBufferSize, settings.buffersize * 4));

        // Clear out all samples we are about to fill again
        for (unsigned int i = _lastGeneratedBufferSize; i < settings.buffersize * 4; i++)
        {
            _mixedOutL[i] = _mixedOutR[i] = 0;
        }

        write_text_to_log_file(string_format(" inside while loop with %d playingNotes", playingNotes.size()));

        // For all notes still playing, get the samples and add them to the mix
        for (auto playingNote : playingNotes)
        {
            playingNote.second->noteout(&_tmpoutl[0], &_tmpoutr[0]);
            write_text_to_log_file(string_format("  mixing %d samples to the end mix", settings.buffersize));

            for (unsigned int i = 0; i < settings.buffersize; i++)
            {
                if (std::abs(_tmpoutl[i]) <= 0.0001f)
                {
                    write_text_to_log_file(string_format("   weird, tmpoutl[%d] is %f", i, _tmpoutl[i]));
                }
                _mixedOutL[_lastGeneratedBufferSize + i] += _tmpoutl[i];
                _mixedOutR[_lastGeneratedBufferSize + i] += _tmpoutr[i];
            }
        }

        // Progress through the buffer untill we have enough
        _lastGeneratedBufferSize += settings.buffersize;
    }

    write_text_to_log_file(string_format("mixed content to output"));

    outputs[0] = &_mixedOutL[0];
    outputs[1] = &_mixedOutR[0];

    _lastSampleFrames = static_cast<unsigned int>(sampleFrames);

    for (auto playingNote : playingNotes)
    {
        if (playingNote.second->finished())
        {
            write_text_to_log_file(string_format(" cleaning up note"));

            delete playingNote.second;
            playingNotes.erase(playingNote.first);
        }
    }
}

VstInt32 VstPlugin::processEvents(VstEvents *ev)
{
    write_text_to_log_file(string_format("processEvents with %d numEvents", ev->numEvents));

    std::lock_guard<std::mutex> lock(_notesMutex);

    for (VstInt32 i = 0; i < ev->numEvents; i++)
    {
        if ((ev->events[i])->type != kVstMidiType)
            continue;

        auto event = reinterpret_cast<VstMidiEvent *>(ev->events[i]);
        char *midiData = event->midiData;
        VstInt32 status = midiData[0] & 0xf0; // ignoring channel
        if (status == 0x90 || status == 0x80) // we only look at notes
        {
            VstInt32 note = midiData[1] & 0x7f;
            VstInt32 velocity = midiData[2] & 0x7f;
            if (status == 0x90)
            {
                if (playingNotes.find(note) == playingNotes.end())
                {
                    write_text_to_log_file(string_format("adding note"));

                    auto playingNote = createNote(note, velocity);
                    playingNotes.insert(std::make_pair(note, playingNote));
                }
            }
            else
            {
                auto found = playingNotes.find(note);
                if (found != playingNotes.end())
                {
                    write_text_to_log_file(string_format("release note"));

                    found->second->relasekey();
                }
                velocity = 0;
            }
        }
        event++;
    }
    return 1;
}

extern "C" {

#if defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
#define VST_EXPORT __attribute__((visibility("default")))
#else
#define VST_EXPORT
#endif

//------------------------------------------------------------------------
/** Prototype of the export function main */
//------------------------------------------------------------------------
VST_EXPORT AEffect *VSTPluginMain(audioMasterCallback audioMaster)
{
    // Get VST Version of the Host
    if (!audioMaster(nullptr, audioMasterVersion, 0, 0, nullptr, 0))
    {
        return nullptr; // old version
    }

    // Create the AudioEffect
    AudioEffect *effect = createEffectInstance(audioMaster);
    if (!effect)
    {
        return nullptr;
    }

    // Return the VST AEffect structur
    return effect->getAeffect();
}

// support for old hosts not looking for VSTPluginMain
#if (TARGET_API_MAC_CARBON && __ppc__)
VST_EXPORT AEffect *main_macho(audioMasterCallback audioMaster)
{
    return VSTPluginMain(audioMaster);
}
#elif WIN32
VST_EXPORT AEffect *MAIN(audioMasterCallback audioMaster)
{
    return VSTPluginMain(audioMaster);
}
#elif BEOS
VST_EXPORT AEffect *main_plugin(audioMasterCallback audioMaster)
{
    return VSTPluginMain(audioMaster);
}
#endif

} // extern "C"
