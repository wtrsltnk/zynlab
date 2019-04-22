#ifndef _APP_THREE_DEE_H_
#define _APP_THREE_DEE_H_

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <imgui.h>

#include "../instrumentcategories.h"
#include "appstate.h"
#include "ui/ui.adnote.h"
#include "ui/ui.dialogs.h"
#include "ui/ui.effect.h"
#include "ui/ui.library.h"
#include "ui/ui.mixer.h"
#include "ui/ui.oscilgen.h"
#include "ui/ui.padnote.h"
#include "ui/ui.samplenote.h"
#include "ui/ui.subnote.h"
#include <chrono>
#include <zyn.common/ILibraryManager.h>
#include <zyn.common/Util.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.nio/Nio.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/FFTwrapper.h>

class AppThreeDee
{
private:
    AppState _state;
    zyn::ui::AdNote _adNoteUI;
    zyn::ui::Effect _effectUi;
    zyn::ui::Library _libraryUi;
    zyn::ui::Mixer _mixerUi;
    zyn::ui::PadNote _padNoteUi;
    zyn::ui::SubNote _subNoteUi;
    zyn::ui::SampleNote _smplNoteUi;
    zyn::ui::OscilGen _oscilGenUi;
    zyn::ui::Dialogs _dialogs;

    GLFWwindow *_window;
    std::vector<unsigned int> _toolbarIcons;
    bool _toolbarIconsAreLoaded;
    std::chrono::milliseconds::rep _lastSequencerTimeInMs;

public:
    static void KeyActionCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void ResizeCallback(GLFWwindow *window, int width, int height);

protected:
    int _display_w, _display_h;

    void onKeyAction(int key, int scancode, int action, int mods);
    void onResize(int width, int height);

public:
    AppThreeDee(GLFWwindow *window, Mixer *mixer, ILibraryManager *library);
    virtual ~AppThreeDee();

    bool Setup();
    void Tick();
    void Render();
    void Cleanup();

private:
    void LoadToolbarIcons();

    void PianoRollEditor();
    void RegionEditor();
    void TempNoteOn(unsigned int channel, unsigned int note, unsigned int length);
    void TickRegion(TrackRegion &region, unsigned char trackIndex, float prevPlayTime, float currentPlayTime, int repeat = 0);
    void ImGuiPlayback();
};

#endif // _APP_THREE_DEE_H_
