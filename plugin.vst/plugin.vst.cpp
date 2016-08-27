#include <iostream>
#include <audioeffectx.h>
#include <math.h>
#include "zyn.common/Util.h"
#include "zyn.common/XMLwrapper.h"
#include "zyn.synth/FFTwrapper.h"
#include "zyn.mixer/Microtonal.h"
#include "zyn.mixer/Instrument.h"

extern const char* synth_brazz;

SYNTH_T* synth;

#define NUM_PROGRAMS    2
#define NUM_PARAMS      0

class YourProjectName : public AudioEffectX
{
public:
    YourProjectName(audioMasterCallback audioMaster);
    ~YourProjectName();

    void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
    VstInt32 processEvents(VstEvents* ev);

    virtual bool getEffectName (char* name) { strcpy(name, "zynstrument"); return true; }	///< Fill \e text with a string identifying the effect
    virtual bool getVendorString (char* text) { strcpy(text, "zyn project"); return true; }	///< Fill \e text with a string identifying the vendor
    virtual bool getProductString (char* text) { strcpy(text, "zynstrument"); return true; }///< Fill \e text with a string identifying the product name
    virtual VstInt32 getVendorVersion () { return 0; }			///< Return vendor-specific version

    virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text) { return false; } ///< Fill \e text with name of program \e index (\e category deprecated in VST 2.4)
    virtual void setProgram (VstInt32 program);	///< Set the current program to \e program

    Microtonal microtonal;
    FFTwrapper* fft;
    pthread_mutex_t mutex;
    Instrument* instrument;
};

YourProjectName::YourProjectName(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
    this->isSynth(true);

    synth = new SYNTH_T;
    synth->alias();

    //produce denormal buf
    denormalkillbuf = new float [synth->buffersize];
    for(int i = 0; i < synth->buffersize; ++i)
        denormalkillbuf[i] = (RND - 0.5f) * 1e-16;

    fft = new FFTwrapper(synth->oscilsize);
    pthread_mutex_init(&mutex, NULL);
    instrument = new Instrument(&microtonal, fft, &mutex);
}

YourProjectName::~YourProjectName()
{ }

void YourProjectName::setProgram (VstInt32 program)
{
    curProgram = program;
    if (curProgram == 0)
    {
        instrument->defaults();
    }
    else
    {
        XMLwrapper xml;
        xml.putXMLdata(synth_brazz);
        instrument->getfromXML(&xml);
    }
}

void YourProjectName::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{
    float* l = outputs[0];
    float* r = outputs[1];

    instrument->ComputePartSmps();
    // Real processing goes here
    for (int i = 0; i < sampleFrames; i++, l++, r++)
    {
//        l[0] = r[0] = float(1.0f * sin(float(i) / 10));
        l[0] = instrument->partoutl[i];
        r[0] = instrument->partoutr[i];
    }
}

VstInt32 YourProjectName::processEvents (VstEvents* ev)
{
    for (VstInt32 i = 0; i < ev->numEvents; i++)
    {
        if ((ev->events[i])->type != kVstMidiType)
            continue;

        VstMidiEvent* event = (VstMidiEvent*)ev->events[i];
        char* midiData = event->midiData;
        VstInt32 status = midiData[0] & 0xf0;	// ignoring channel
        if (status == 0x90 || status == 0x80)	// we only look at notes
        {
            VstInt32 note = midiData[1] & 0x7f;
            VstInt32 velocity = midiData[2] & 0x7f;
            if (status == 0x90)
                this->instrument->NoteOn(note, velocity, 0);
            else
            {
                velocity = 0;	// note off by velocity 0
                this->instrument->NoteOff(note);
            }
        }
        event++;
    }
    return 1;
}

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
    return new YourProjectName(audioMaster);
}

extern "C" {

#if defined (__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
    #define VST_EXPORT	__attribute__ ((visibility ("default")))
#else
    #define VST_EXPORT
#endif

//------------------------------------------------------------------------
/** Prototype of the export function main */
//------------------------------------------------------------------------
VST_EXPORT AEffect* VSTPluginMain (audioMasterCallback audioMaster)
{
    // Get VST Version of the Host
    if (!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
        return 0;  // old version

    // Create the AudioEffect
    AudioEffect* effect = createEffectInstance (audioMaster);
    if (!effect)
        return 0;

    // Return the VST AEffect structur
    return effect->getAeffect ();
}

// support for old hosts not looking for VSTPluginMain
#if (TARGET_API_MAC_CARBON && __ppc__)
VST_EXPORT AEffect* main_macho (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#elif WIN32
VST_EXPORT AEffect* MAIN (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#elif BEOS
VST_EXPORT AEffect* main_plugin (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#endif

} // extern "C"


const char* synth_brazz = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
        <!DOCTYPE ZynAddSubFX-data>\
        <ZynAddSubFX-data version-major=\"1\" version-minor=\"1\" ZynAddSubFX-author=\"Nasca Octavian Paul\">\
        <INFORMATION>\
        <par_bool name=\"PADsynth_used\" value=\"no\"/>\
        </INFORMATION>\
        <BASE_PARAMETERS>\
        <par name=\"max_midi_parts\" value=\"16\"/>\
        <par name=\"max_kit_items_per_instrument\" value=\"16\"/>\
        <par name=\"max_system_effects\" value=\"4\"/>\
        <par name=\"max_insertion_effects\" value=\"8\"/>\
        <par name=\"max_instrument_effects\" value=\"3\"/>\
        <par name=\"max_addsynth_voices\" value=\"8\"/>\
        </BASE_PARAMETERS>\
        <INSTRUMENT>\
        <INFO>\
        <string name=\"name\">Synth Brazz 2</string>\
        <string name=\"author\"></string>\
        <string name=\"comments\"></string>\
        <par name=\"type\" value=\"0\"/>\
        </INFO>\
        <INSTRUMENT_KIT>\
        <par name=\"kit_mode\" value=\"0\"/>\
        <par_bool name=\"drum_mode\" value=\"no\"/>\
        <INSTRUMENT_KIT_ITEM id=\"0\">\
        <par_bool name=\"enabled\" value=\"yes\"/>\
        <string name=\"name\">Synth Brazz 2</string>\
        <par_bool name=\"muted\" value=\"no\"/>\
        <par name=\"min_key\" value=\"0\"/>\
        <par name=\"max_key\" value=\"127\"/>\
        <par name=\"send_to_instrument_effect\" value=\"0\"/>\
        <par_bool name=\"add_enabled\" value=\"yes\"/>\
        <ADD_SYNTH_PARAMETERS>\
        <par_bool name=\"stereo\" value=\"yes\"/>\
        <AMPLITUDE_PARAMETERS>\
        <par name=\"volume\" value=\"80\"/>\
        <par name=\"panning\" value=\"62\"/>\
        <par name=\"velocity_sensing\" value=\"112\"/>\
        <par name=\"punch_strength\" value=\"0\"/>\
        <par name=\"punch_time\" value=\"60\"/>\
        <par name=\"punch_stretch\" value=\"64\"/>\
        <par name=\"punch_velocity_sensing\" value=\"72\"/>\
        <par name=\"harmonic_randomness_grouping\" value=\"0\"/>\
        <AMPLITUDE_ENVELOPE>\
        <par_bool name=\"free_mode\" value=\"no\"/>\
        <par name=\"env_points\" value=\"4\"/>\
        <par name=\"env_sustain\" value=\"2\"/>\
        <par name=\"env_stretch\" value=\"19\"/>\
        <par_bool name=\"forced_release\" value=\"yes\"/>\
        <par_bool name=\"linear_envelope\" value=\"no\"/>\
        <par name=\"A_dt\" value=\"0\"/>\
        <par name=\"D_dt\" value=\"0\"/>\
        <par name=\"R_dt\" value=\"34\"/>\
        <par name=\"A_val\" value=\"64\"/>\
        <par name=\"D_val\" value=\"64\"/>\
        <par name=\"S_val\" value=\"127\"/>\
        <par name=\"R_val\" value=\"64\"/>\
        </AMPLITUDE_ENVELOPE>\
        <AMPLITUDE_LFO>\
        <par_real name=\"freq\" value=\"0.629921\"/>\
        <par name=\"intensity\" value=\"0\"/>\
        <par name=\"start_phase\" value=\"64\"/>\
        <par name=\"lfo_type\" value=\"0\"/>\
        <par name=\"randomness_amplitude\" value=\"0\"/>\
        <par name=\"randomness_frequency\" value=\"0\"/>\
        <par name=\"delay\" value=\"0\"/>\
        <par name=\"stretch\" value=\"64\"/>\
        <par_bool name=\"continous\" value=\"no\"/>\
        </AMPLITUDE_LFO>\
        </AMPLITUDE_PARAMETERS>\
        <FREQUENCY_PARAMETERS>\
        <par name=\"detune\" value=\"8192\"/>\
        <par name=\"coarse_detune\" value=\"0\"/>\
        <par name=\"detune_type\" value=\"2\"/>\
        <par name=\"bandwidth\" value=\"64\"/>\
        <FREQUENCY_ENVELOPE>\
        <par_bool name=\"free_mode\" value=\"no\"/>\
        <par name=\"env_points\" value=\"3\"/>\
        <par name=\"env_sustain\" value=\"1\"/>\
        <par name=\"env_stretch\" value=\"0\"/>\
        <par_bool name=\"forced_release\" value=\"no\"/>\
        <par_bool name=\"linear_envelope\" value=\"no\"/>\
        <par name=\"A_dt\" value=\"50\"/>\
        <par name=\"D_dt\" value=\"10\"/>\
        <par name=\"R_dt\" value=\"60\"/>\
        <par name=\"A_val\" value=\"64\"/>\
        <par name=\"D_val\" value=\"64\"/>\
        <par name=\"S_val\" value=\"64\"/>\
        <par name=\"R_val\" value=\"64\"/>\
        </FREQUENCY_ENVELOPE>\
        <FREQUENCY_LFO>\
        <par_real name=\"freq\" value=\"0.598425\"/>\
        <par name=\"intensity\" value=\"34\"/>\
        <par name=\"start_phase\" value=\"64\"/>\
        <par name=\"lfo_type\" value=\"0\"/>\
        <par name=\"randomness_amplitude\" value=\"0\"/>\
        <par name=\"randomness_frequency\" value=\"0\"/>\
        <par name=\"delay\" value=\"0\"/>\
        <par name=\"stretch\" value=\"64\"/>\
        <par_bool name=\"continous\" value=\"no\"/>\
        </FREQUENCY_LFO>\
        </FREQUENCY_PARAMETERS>\
        <FILTER_PARAMETERS>\
        <par name=\"velocity_sensing_amplitude\" value=\"79\"/>\
        <par name=\"velocity_sensing\" value=\"64\"/>\
        <FILTER>\
        <par name=\"category\" value=\"0\"/>\
        <par name=\"type\" value=\"2\"/>\
        <par name=\"freq\" value=\"64\"/>\
        <par name=\"q\" value=\"35\"/>\
        <par name=\"stages\" value=\"0\"/>\
        <par name=\"freq_track\" value=\"40\"/>\
        <par name=\"gain\" value=\"64\"/>\
        </FILTER>\
        <FILTER_ENVELOPE>\
        <par_bool name=\"free_mode\" value=\"no\"/>\
        <par name=\"env_points\" value=\"4\"/>\
        <par name=\"env_sustain\" value=\"2\"/>\
        <par name=\"env_stretch\" value=\"0\"/>\
        <par_bool name=\"forced_release\" value=\"yes\"/>\
        <par_bool name=\"linear_envelope\" value=\"no\"/>\
        <par name=\"A_dt\" value=\"17\"/>\
        <par name=\"D_dt\" value=\"87\"/>\
        <par name=\"R_dt\" value=\"66\"/>\
        <par name=\"A_val\" value=\"27\"/>\
        <par name=\"D_val\" value=\"111\"/>\
        <par name=\"S_val\" value=\"64\"/>\
        <par name=\"R_val\" value=\"23\"/>\
        </FILTER_ENVELOPE>\
        <FILTER_LFO>\
        <par_real name=\"freq\" value=\"0.574803\"/>\
        <par name=\"intensity\" value=\"0\"/>\
        <par name=\"start_phase\" value=\"64\"/>\
        <par name=\"lfo_type\" value=\"0\"/>\
        <par name=\"randomness_amplitude\" value=\"0\"/>\
        <par name=\"randomness_frequency\" value=\"0\"/>\
        <par name=\"delay\" value=\"0\"/>\
        <par name=\"stretch\" value=\"64\"/>\
        <par_bool name=\"continous\" value=\"no\"/>\
        </FILTER_LFO>\
        </FILTER_PARAMETERS>\
        <RESONANCE>\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </RESONANCE>\
        <VOICE id=\"0\">\
        <par_bool name=\"enabled\" value=\"yes\"/>\
        <par name=\"type\" value=\"0\"/>\
        <par name=\"delay\" value=\"0\"/>\
        <par_bool name=\"resonance\" value=\"yes\"/>\
        <par name=\"ext_oscil\" value=\"-1\"/>\
        <par name=\"ext_fm_oscil\" value=\"-1\"/>\
        <par name=\"oscil_phase\" value=\"64\"/>\
        <par name=\"oscil_fm_phase\" value=\"64\"/>\
        <par_bool name=\"filter_enabled\" value=\"no\"/>\
        <par_bool name=\"filter_bypass\" value=\"no\"/>\
        <par name=\"fm_enabled\" value=\"0\"/>\
        <OSCIL>\
        <par name=\"harmonic_mag_type\" value=\"0\"/>\
        <par name=\"base_function\" value=\"7\"/>\
        <par name=\"base_function_par\" value=\"43\"/>\
        <par name=\"base_function_modulation\" value=\"0\"/>\
        <par name=\"base_function_modulation_par1\" value=\"64\"/>\
        <par name=\"base_function_modulation_par2\" value=\"64\"/>\
        <par name=\"base_function_modulation_par3\" value=\"32\"/>\
        <par name=\"modulation\" value=\"0\"/>\
        <par name=\"modulation_par1\" value=\"64\"/>\
        <par name=\"modulation_par2\" value=\"64\"/>\
        <par name=\"modulation_par3\" value=\"32\"/>\
        <par name=\"wave_shaping\" value=\"48\"/>\
        <par name=\"wave_shaping_function\" value=\"13\"/>\
        <par name=\"filter_type\" value=\"0\"/>\
        <par name=\"filter_par1\" value=\"68\"/>\
        <par name=\"filter_par2\" value=\"64\"/>\
        <par name=\"filter_before_wave_shaping\" value=\"0\"/>\
        <par name=\"spectrum_adjust_type\" value=\"0\"/>\
        <par name=\"spectrum_adjust_par\" value=\"64\"/>\
        <par name=\"rand\" value=\"0\"/>\
        <par name=\"amp_rand_type\" value=\"0\"/>\
        <par name=\"amp_rand_power\" value=\"64\"/>\
        <par name=\"harmonic_shift\" value=\"0\"/>\
        <par_bool name=\"harmonic_shift_first\" value=\"no\"/>\
        <par name=\"adaptive_harmonics\" value=\"0\"/>\
        <par name=\"adaptive_harmonics_base_frequency\" value=\"128\"/>\
        <par name=\"adaptive_harmonics_power\" value=\"100\"/>\
        <HARMONICS>\
        <HARMONIC id=\"1\">\
        <par name=\"mag\" value=\"127\"/>\
        <par name=\"phase\" value=\"64\"/>\
        </HARMONIC>\
        <HARMONIC id=\"2\">\
        <par name=\"mag\" value=\"127\"/>\
        <par name=\"phase\" value=\"52\"/>\
        </HARMONIC>\
        </HARMONICS>\
        </OSCIL>\
        <AMPLITUDE_PARAMETERS>\
        <par name=\"panning\" value=\"62\"/>\
        <par name=\"volume\" value=\"100\"/>\
        <par_bool name=\"volume_minus\" value=\"yes\"/>\
        <par name=\"velocity_sensing\" value=\"127\"/>\
        <par_bool name=\"amp_envelope_enabled\" value=\"no\"/>\
        <par_bool name=\"amp_lfo_enabled\" value=\"no\"/>\
        </AMPLITUDE_PARAMETERS>\
        <FREQUENCY_PARAMETERS>\
        <par_bool name=\"fixed_freq\" value=\"no\"/>\
        <par name=\"fixed_freq_et\" value=\"0\"/>\
        <par name=\"detune\" value=\"8274\"/>\
        <par name=\"coarse_detune\" value=\"0\"/>\
        <par name=\"detune_type\" value=\"0\"/>\
        <par_bool name=\"freq_envelope_enabled\" value=\"no\"/>\
        <par_bool name=\"freq_lfo_enabled\" value=\"yes\"/>\
        <FREQUENCY_LFO>\
        <par_real name=\"freq\" value=\"0.448819\"/>\
        <par name=\"intensity\" value=\"47\"/>\
        <par name=\"start_phase\" value=\"0\"/>\
        <par name=\"lfo_type\" value=\"0\"/>\
        <par name=\"randomness_amplitude\" value=\"0\"/>\
        <par name=\"randomness_frequency\" value=\"0\"/>\
        <par name=\"delay\" value=\"0\"/>\
        <par name=\"stretch\" value=\"64\"/>\
        <par_bool name=\"continous\" value=\"no\"/>\
        </FREQUENCY_LFO>\
        </FREQUENCY_PARAMETERS>\
        </VOICE>\
        <VOICE id=\"1\">\
        <par_bool name=\"enabled\" value=\"yes\"/>\
        <par name=\"type\" value=\"0\"/>\
        <par name=\"delay\" value=\"0\"/>\
        <par_bool name=\"resonance\" value=\"yes\"/>\
        <par name=\"ext_oscil\" value=\"0\"/>\
        <par name=\"ext_fm_oscil\" value=\"-1\"/>\
        <par name=\"oscil_phase\" value=\"64\"/>\
        <par name=\"oscil_fm_phase\" value=\"64\"/>\
        <par_bool name=\"filter_enabled\" value=\"no\"/>\
        <par_bool name=\"filter_bypass\" value=\"no\"/>\
        <par name=\"fm_enabled\" value=\"0\"/>\
        <OSCIL>\
        <par name=\"harmonic_mag_type\" value=\"0\"/>\
        <par name=\"base_function\" value=\"0\"/>\
        <par name=\"base_function_par\" value=\"64\"/>\
        <par name=\"base_function_modulation\" value=\"0\"/>\
        <par name=\"base_function_modulation_par1\" value=\"64\"/>\
        <par name=\"base_function_modulation_par2\" value=\"64\"/>\
        <par name=\"base_function_modulation_par3\" value=\"32\"/>\
        <par name=\"modulation\" value=\"0\"/>\
        <par name=\"modulation_par1\" value=\"64\"/>\
        <par name=\"modulation_par2\" value=\"64\"/>\
        <par name=\"modulation_par3\" value=\"32\"/>\
        <par name=\"wave_shaping\" value=\"64\"/>\
        <par name=\"wave_shaping_function\" value=\"0\"/>\
        <par name=\"filter_type\" value=\"0\"/>\
        <par name=\"filter_par1\" value=\"64\"/>\
        <par name=\"filter_par2\" value=\"64\"/>\
        <par name=\"filter_before_wave_shaping\" value=\"0\"/>\
        <par name=\"spectrum_adjust_type\" value=\"0\"/>\
        <par name=\"spectrum_adjust_par\" value=\"64\"/>\
        <par name=\"rand\" value=\"64\"/>\
        <par name=\"amp_rand_type\" value=\"0\"/>\
        <par name=\"amp_rand_power\" value=\"64\"/>\
        <par name=\"harmonic_shift\" value=\"0\"/>\
        <par_bool name=\"harmonic_shift_first\" value=\"no\"/>\
        <par name=\"adaptive_harmonics\" value=\"0\"/>\
        <par name=\"adaptive_harmonics_base_frequency\" value=\"128\"/>\
        <par name=\"adaptive_harmonics_power\" value=\"100\"/>\
        <HARMONICS>\
        <HARMONIC id=\"1\">\
        <par name=\"mag\" value=\"127\"/>\
        <par name=\"phase\" value=\"64\"/>\
        </HARMONIC>\
        </HARMONICS>\
        </OSCIL>\
        <AMPLITUDE_PARAMETERS>\
        <par name=\"panning\" value=\"0\"/>\
        <par name=\"volume\" value=\"100\"/>\
        <par_bool name=\"volume_minus\" value=\"no\"/>\
        <par name=\"velocity_sensing\" value=\"127\"/>\
        <par_bool name=\"amp_envelope_enabled\" value=\"no\"/>\
        <par_bool name=\"amp_lfo_enabled\" value=\"no\"/>\
        </AMPLITUDE_PARAMETERS>\
        <FREQUENCY_PARAMETERS>\
        <par_bool name=\"fixed_freq\" value=\"no\"/>\
        <par name=\"fixed_freq_et\" value=\"0\"/>\
        <par name=\"detune\" value=\"6280\"/>\
        <par name=\"coarse_detune\" value=\"0\"/>\
        <par name=\"detune_type\" value=\"0\"/>\
        <par_bool name=\"freq_envelope_enabled\" value=\"no\"/>\
        <par_bool name=\"freq_lfo_enabled\" value=\"yes\"/>\
        <FREQUENCY_LFO>\
        <par_real name=\"freq\" value=\"0.181102\"/>\
        <par name=\"intensity\" value=\"40\"/>\
        <par name=\"start_phase\" value=\"0\"/>\
        <par name=\"lfo_type\" value=\"0\"/>\
        <par name=\"randomness_amplitude\" value=\"0\"/>\
        <par name=\"randomness_frequency\" value=\"0\"/>\
        <par name=\"delay\" value=\"0\"/>\
        <par name=\"stretch\" value=\"64\"/>\
        <par_bool name=\"continous\" value=\"no\"/>\
        </FREQUENCY_LFO>\
        </FREQUENCY_PARAMETERS>\
        </VOICE>\
        <VOICE id=\"2\">\
        <par_bool name=\"enabled\" value=\"yes\"/>\
        <par name=\"type\" value=\"0\"/>\
        <par name=\"delay\" value=\"0\"/>\
        <par_bool name=\"resonance\" value=\"yes\"/>\
        <par name=\"ext_oscil\" value=\"0\"/>\
        <par name=\"ext_fm_oscil\" value=\"-1\"/>\
        <par name=\"oscil_phase\" value=\"64\"/>\
        <par name=\"oscil_fm_phase\" value=\"64\"/>\
        <par_bool name=\"filter_enabled\" value=\"no\"/>\
        <par_bool name=\"filter_bypass\" value=\"no\"/>\
        <par name=\"fm_enabled\" value=\"0\"/>\
        <OSCIL>\
        <par name=\"harmonic_mag_type\" value=\"0\"/>\
        <par name=\"base_function\" value=\"0\"/>\
        <par name=\"base_function_par\" value=\"64\"/>\
        <par name=\"base_function_modulation\" value=\"0\"/>\
        <par name=\"base_function_modulation_par1\" value=\"64\"/>\
        <par name=\"base_function_modulation_par2\" value=\"64\"/>\
        <par name=\"base_function_modulation_par3\" value=\"32\"/>\
        <par name=\"modulation\" value=\"0\"/>\
        <par name=\"modulation_par1\" value=\"64\"/>\
        <par name=\"modulation_par2\" value=\"64\"/>\
        <par name=\"modulation_par3\" value=\"32\"/>\
        <par name=\"wave_shaping\" value=\"64\"/>\
        <par name=\"wave_shaping_function\" value=\"0\"/>\
        <par name=\"filter_type\" value=\"0\"/>\
        <par name=\"filter_par1\" value=\"64\"/>\
        <par name=\"filter_par2\" value=\"64\"/>\
        <par name=\"filter_before_wave_shaping\" value=\"0\"/>\
        <par name=\"spectrum_adjust_type\" value=\"0\"/>\
        <par name=\"spectrum_adjust_par\" value=\"64\"/>\
        <par name=\"rand\" value=\"64\"/>\
        <par name=\"amp_rand_type\" value=\"0\"/>\
        <par name=\"amp_rand_power\" value=\"64\"/>\
        <par name=\"harmonic_shift\" value=\"0\"/>\
        <par_bool name=\"harmonic_shift_first\" value=\"no\"/>\
        <par name=\"adaptive_harmonics\" value=\"0\"/>\
        <par name=\"adaptive_harmonics_base_frequency\" value=\"128\"/>\
        <par name=\"adaptive_harmonics_power\" value=\"100\"/>\
        <HARMONICS>\
        <HARMONIC id=\"1\">\
        <par name=\"mag\" value=\"127\"/>\
        <par name=\"phase\" value=\"64\"/>\
        </HARMONIC>\
        </HARMONICS>\
        </OSCIL>\
        <AMPLITUDE_PARAMETERS>\
        <par name=\"panning\" value=\"0\"/>\
        <par name=\"volume\" value=\"98\"/>\
        <par_bool name=\"volume_minus\" value=\"no\"/>\
        <par name=\"velocity_sensing\" value=\"127\"/>\
        <par_bool name=\"amp_envelope_enabled\" value=\"no\"/>\
        <par_bool name=\"amp_lfo_enabled\" value=\"no\"/>\
        </AMPLITUDE_PARAMETERS>\
        <FREQUENCY_PARAMETERS>\
        <par_bool name=\"fixed_freq\" value=\"no\"/>\
        <par name=\"fixed_freq_et\" value=\"0\"/>\
        <par name=\"detune\" value=\"9466\"/>\
        <par name=\"coarse_detune\" value=\"0\"/>\
        <par name=\"detune_type\" value=\"0\"/>\
        <par_bool name=\"freq_envelope_enabled\" value=\"no\"/>\
        <par_bool name=\"freq_lfo_enabled\" value=\"yes\"/>\
        <FREQUENCY_LFO>\
        <par_real name=\"freq\" value=\"0.291339\"/>\
        <par name=\"intensity\" value=\"40\"/>\
        <par name=\"start_phase\" value=\"0\"/>\
        <par name=\"lfo_type\" value=\"0\"/>\
        <par name=\"randomness_amplitude\" value=\"0\"/>\
        <par name=\"randomness_frequency\" value=\"0\"/>\
        <par name=\"delay\" value=\"0\"/>\
        <par name=\"stretch\" value=\"64\"/>\
        <par_bool name=\"continous\" value=\"no\"/>\
        </FREQUENCY_LFO>\
        </FREQUENCY_PARAMETERS>\
        </VOICE>\
        <VOICE id=\"3\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </VOICE>\
        <VOICE id=\"4\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </VOICE>\
        <VOICE id=\"5\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </VOICE>\
        <VOICE id=\"6\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </VOICE>\
        <VOICE id=\"7\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </VOICE>\
        </ADD_SYNTH_PARAMETERS>\
        <par_bool name=\"sub_enabled\" value=\"no\"/>\
        <par_bool name=\"pad_enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"1\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"2\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"3\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"4\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"5\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"6\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"7\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"8\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"9\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"10\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"11\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"12\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"13\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"14\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        <INSTRUMENT_KIT_ITEM id=\"15\">\
        <par_bool name=\"enabled\" value=\"no\"/>\
        </INSTRUMENT_KIT_ITEM>\
        </INSTRUMENT_KIT>\
        <INSTRUMENT_EFFECTS>\
        <INSTRUMENT_EFFECT id=\"0\">\
        <EFFECT>\
        <par name=\"type\" value=\"0\"/>\
        </EFFECT>\
        <par name=\"route\" value=\"0\"/>\
        <par_bool name=\"bypass\" value=\"no\"/>\
        </INSTRUMENT_EFFECT>\
        <INSTRUMENT_EFFECT id=\"1\">\
        <EFFECT>\
        <par name=\"type\" value=\"0\"/>\
        </EFFECT>\
        <par name=\"route\" value=\"0\"/>\
        <par_bool name=\"bypass\" value=\"no\"/>\
        </INSTRUMENT_EFFECT>\
        <INSTRUMENT_EFFECT id=\"2\">\
        <EFFECT>\
        <par name=\"type\" value=\"0\"/>\
        </EFFECT>\
        <par name=\"route\" value=\"0\"/>\
        <par_bool name=\"bypass\" value=\"no\"/>\
        </INSTRUMENT_EFFECT>\
        </INSTRUMENT_EFFECTS>\
        </INSTRUMENT>\
        </ZynAddSubFX-data>";
