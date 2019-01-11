#ifndef _APP_THREE_DEE_H_
#define _APP_THREE_DEE_H_

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <imgui.h>

#include "../instrumentcategories.h"
#include "appstate.h"
#include "ui/app.mixer.h"
#include <chrono>
#include <zyn.common/Util.h>
#include <zyn.mixer/BankManager.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.nio/Nio.h>
#include <zyn.seq/Sequencer.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/FFTwrapper.h>

class AppThreeDee
{
private:
    AppState _state;
    AppMixer _appMixer;
    GLFWwindow *_window;
    Stepper _stepper;
    Sequencer _sequencer;
    std::vector<TrackPattern> _clipboardPatterns;
    std::vector<unsigned int> _toolbarIcons;
    bool _toolbarIconsAreLoaded;

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
    void LoadToolbarIcons();

    void ImGuiPlayback();
    void ImGuiSequencer();
    void ImGuiStepSequencer(int trackIndex, float trackHeight);
    void ImGuiStepSequencerEventHandling();
    void ImGuiStepPatternEditorWindow();
    void ImGuiPianoRollSequencer(int trackIndex, float trackHeight);
    void ImGuiPianoRollSequencerEventHandling();
    void ImGuiPianoRollPatternEditorWindow();

    void HitNote(int trackIndex, int note, int velocity, int durationInMs);

    // AD note
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
    void SUBNoteEditor(Channel *channel, int instrumentIndex);
    void SUBNoteEditorHarmonicsMagnitude(SUBnoteParameters *parameters);
    void SUBNoteEditorAmplitude(SUBnoteParameters *parameters);
    void SUBNoteEditorBandwidth(SUBnoteParameters *parameters);
    void SUBNoteEditorOvertones(SUBnoteParameters *parameters);
    void SUBNoteEditorFilter(SUBnoteParameters *parameters);
    void SUBNoteEditorFrequency(SUBnoteParameters *parameters);

    // PAD note
    void PADNoteEditor(Channel *channel, int instrumentIndex);
    void PADNoteEditorAmplitude(PADnoteParameters *parameters);

    // LFO
    void LFO(char const *label, class LFOParams *parameters);

    // Filter
    void FilterParameters(class FilterParams *parameters);

    // Envelope
    void Envelope(char const *label, class EnvelopeParams *envelope);

    // Effect
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
