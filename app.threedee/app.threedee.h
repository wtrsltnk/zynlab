#ifndef _APP_THREE_DEE_H_
#define _APP_THREE_DEE_H_

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <imgui.h>

#include <zyn.common/Util.h>
#include <zyn.mixer/BankManager.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.nio/Nio.h>
#include <zyn.seq/Track.h>
#include <zyn.seq/Tracker.h>
#include <zyn.synth/FFTwrapper.h>
#include <zyn.synth/ADnoteParams.h>

class AppThreeDee
{
private:
    Mixer *_mixer;
    GLFWwindow *_window;

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

    bool SetUp();
    void Render();
    void CleanUp();

private:
    void ImGuiPlayback();
    void ImGuiSequencer();
    void Stop();
    void PlayPause();

    void AddPattern(int trackIndex, int patternIndex, char const *label);
    void RemoveActivePattern();
    void MovePatternLeftIfPossible();
    void MovePatternLeftForced();
    void MovePatternRightIfPossible();
    void MovePatternRightForced();
    void SelectFirstPatternInTrack();
    void SelectLastPatternInTrack();
    void EditSelectedPattern();

    // AD note
    bool showAddSynthEditor;
    void ADNoteEditor(ADnoteParameters *parameters);
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

    // LFO
    void LFO(char const *label, class LFOParams *parameters);

    // Filter
    void FilterParameters(class FilterParams *parameters);

    // Envelope
    void Envelope(char const *label, class EnvelopeParams *envelope);
};

#endif // _APP_THREE_DEE_H_
