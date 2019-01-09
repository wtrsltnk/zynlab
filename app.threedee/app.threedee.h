#ifndef _APP_THREE_DEE_H_
#define _APP_THREE_DEE_H_

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <imgui.h>

#include <chrono>
#include <zyn.common/Util.h>
#include <zyn.mixer/BankManager.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.nio/Nio.h>
#include <zyn.seq/Sequencer.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/FFTwrapper.h>

extern char const *const ADeditorID;
extern char const *const SUBeditorID;
extern char const *const PADeditorID;
extern char const *const InsertionFxEditorID;
extern char const *const SystemFxEditorID;
extern char const *const InstrumentFxEditorID;

enum class InstrumentCategories
{
    Unknown,
    Piano,
    Chromatic_Percussion,
    Organ,
    Guitar,
    Bass,
    Solo_Strings,
    Ensemble,
    Brass,
    Reed,
    Pipe,
    Synth_Lead,
    Synth_Pad,
    Synth_Effects,
    Ethnic,
    Percussive,
    Sound_Effects,
    COUNT,
};

class AppThreeDee
{
private:
    Mixer *_mixer;
    GLFWwindow *_window;
    Stepper _stepper;
    Sequencer _sequencer;
    std::vector<TrackPattern> _clipboardPatterns;
    unsigned int _iconImages[int(InstrumentCategories::COUNT)];
    bool _iconImagesAreLoaded;
    bool _showInstrumentEditor;
    bool _showPatternEditor;
    bool _showPianoRollPatternEditor;
    bool _showSystemEffectsEditor;
    bool _showInsertEffectsEditor;
    bool _showInstrumentEffectsEditor;
    bool _showMixer;
    int _openSelectInstrument;
    int _openChangeInstrumentType;

    static char const *const effectNames[];
    static int effectNameCount;

public:
    static void KeyActionCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void ResizeCallback(GLFWwindow *window, int width, int height);

protected:
    int _display_w, _display_h;

    void onKeyAction(int key, int scancode, int action, int mods);
    void onResize(int width, int height);

public:
    AppThreeDee(GLFWwindow *window, Mixer *mixer);
    virtual ~AppThreeDee();

    bool Setup();
    void Tick();
    void Render();
    void Cleanup();

private:
    void LoadInstrumentIcons();

    void ImGuiPlayback();
    void ImGuiMasterTrack();
    void ImGuiMixer();
    void ImGuiTrack(int track, bool highlightTrack);
    bool _showSelectedTrack;
    void ImGuiSelectedTrack();
    void ImGuiSequencer();
    void ImGuiStepSequencer(int trackIndex, float trackHeight);
    void ImGuiStepSequencerEventHandling();
    void ImGuiStepPatternEditorWindow();
    void ImGuiPianoRollSequencer(int trackIndex, float trackHeight);
    void ImGuiPianoRollSequencerEventHandling();
    void ImGuiPianoRollPatternEditorWindow();
    void ImGuiSelectInstrumentPopup();
    void ImGuiChangeInstrumentTypePopup();
    int _currentBank;

    void HitNote(int trackIndex, int note, int velocity, int durationInMs);
    void AddInsertFx(int track);
    void RemoveInsertFxFromTrack(int fx);

    // AD note
    bool _showADNoteEditor;
    void ADNoteEditor(Channel *channel, int instrumentIndex);
    void ADNoteEditorAmplitude(ADnoteGlobalParam *parameters);
    void ADNoteEditorFilter(ADnoteGlobalParam *parameters);
    void ADNoteEditorFrequency(ADnoteGlobalParam *parameters);
    void ADNoteVoiceEditor(ADnoteVoiceParam *parameters);
    void ADNoteVoiceEditorOscillator(ADnoteVoiceParam *parameters);
    void ADNoteVoiceEditorOscillatorUnison(ADnoteVoiceParam *parameters);
    void ADNoteVoiceEditorAmplitude(ADnoteVoiceParam *parameters);
    void ADNoteVoiceEditorFilter(ADnoteVoiceParam *parameters);
    void ADNoteVoiceEditorFrequency(ADnoteVoiceParam *parameters);
    void ADNoteVoiceEditorModulation(ADnoteVoiceParam *parameters);

    // SUB note
    bool _showSUBNoteEditor;
    void SUBNoteEditor(Channel *channel, int instrumentIndex);
    void SUBNoteEditorHarmonicsMagnitude(SUBnoteParameters *parameters);
    void SUBNoteEditorAmplitude(SUBnoteParameters *parameters);
    void SUBNoteEditorBandwidth(SUBnoteParameters *parameters);
    void SUBNoteEditorOvertones(SUBnoteParameters *parameters);
    void SUBNoteEditorFilter(SUBnoteParameters *parameters);
    void SUBNoteEditorFrequency(SUBnoteParameters *parameters);

    // PAD note
    bool _showPADNoteEditor;
    void PADNoteEditor(Channel *channel, int instrumentIndex);
    void PADNoteEditorAmplitude(PADnoteParameters *parameters);

    // LFO
    void LFO(char const *label, class LFOParams *parameters);

    // Filter
    void FilterParameters(class FilterParams *parameters);

    // Envelope
    void Envelope(char const *label, class EnvelopeParams *envelope);

    // Effect
    int _currentInsertEffect;
    int _currentSystemEffect;
    int _currentInstrumentEffect;
    void InsertEffectEditor();
    void SystemEffectEditor();
    void InstrumentEffectEditor();
    void EffectEditor(EffectManager *effectManager);
    void EffectReverbEditor(EffectManager *effectManager);
    void EffectEchoEditor(EffectManager *effectManager);
    void EffectChorusEditor(EffectManager *effectManager);
    void EffectPhaserEditor(EffectManager *effectManager);
    void EffectAlienWahEditor(EffectManager *effectManager);
    void EffectDistortionEditor(EffectManager *effectManager);
    void EffectEQEditor(EffectManager *effectManager);
    void EffectDynFilterEditor(EffectManager *effectManager);
};

#endif // _APP_THREE_DEE_H_
