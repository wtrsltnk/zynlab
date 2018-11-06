#ifndef _APP_THREE_DEE_H_
#define _APP_THREE_DEE_H_

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <zyn.common/Util.h>
#include <zyn.mixer/BankManager.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.nio/Nio.h>
#include <zyn.seq/Track.h>
#include <zyn.seq/Tracker.h>
#include <zyn.synth/FFTwrapper.h>

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

public:
    // App specific
    void EditInstrument(int i);
    void SelectInstrument(int i);

    Tracker _tracker;

private:
    void ImGuiPlayback();
    void ImGuiSequencer(Track *tracks, int count);
    void ImGuiTrack(Track &track, int index, std::string const &name);
    void ImGuiPattern(Pattern &pattern, int trackIndex, int patternIndex);
    void ImGuiPatternEvent(PatternEvent &event, int trackIndex, int patternIndex, int eventIndex);
    void AddPatternToTrack(int trackIndex);
    void Stop();
    void PlayPause();
};

#endif // _APP_THREE_DEE_H_
