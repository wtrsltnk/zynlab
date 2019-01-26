#ifndef _APP_DRUMMER_H_
#define _APP_DRUMMER_H_

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <imgui.h>

#include "../instrumentcategories.h"
#include "appstate.h"
#include "ui/ui.library.h"
#include "ui/ui.effect.h"
#include "ui/ui.mixer.h"
#include "ui/ui.sequencer.h"
#include <chrono>
#include <zyn.common/Util.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.nio/Nio.h>
#include <zyn.serialization/BankManager.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/FFTwrapper.h>

class AppDrummer
{
private:
    AppState _state;
    zyn::ui::Effect _effectUi;
    zyn::ui::Library _libraryUi;
    zyn::ui::Mixer _mixerUi;
    zyn::ui::Sequencer _sequencerUi;

    GLFWwindow *_window;
    Stepper _stepper;

public:
    static void KeyActionCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void ResizeCallback(GLFWwindow *window, int width, int height);

protected:
    int _display_w, _display_h;

    void onKeyAction(int key, int scancode, int action, int mods);
    void onResize(int width, int height);

public:
    AppDrummer(GLFWwindow *window, Mixer *mixer, IBankManager *banks);
    virtual ~AppDrummer();

    bool Setup();
    void Tick();
    void Render();
    void Cleanup();

private:
    void ImGuiPlayback();
};

#endif // _APP_DRUMMER_H_
