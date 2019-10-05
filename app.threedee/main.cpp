
#include "app.threedee.h"

#include <imgui.h>

#include "examples/imgui_impl_opengl3.h"
#include <zyn.serialization/LibraryManager.h>

static int Pexitprogram = 0;

static LibraryManager libraryManager;
static Mixer *mixer;

//cleanup on signaled exit
void sigterm_exit(int /*sig*/)
{
    Pexitprogram = 1;
}

/*
 * Program initialisation
 */
void initprogram()
{
    Config::Current().init();

    /* Get the settings from the Config*/
    SystemSettings::Instance().samplerate = Config::Current().cfg.SampleRate;
    SystemSettings::Instance().buffersize = Config::Current().cfg.SoundBufferSize;
    SystemSettings::Instance().oscilsize = Config::Current().cfg.OscilSize;
    SystemSettings::Instance().alias();

    std::cerr.precision(1);
    std::cerr << std::fixed;
    std::cerr << "\nSample Rate = \t\t" << SystemSettings::Instance().samplerate << std::endl;
    std::cerr << "Sound Buffer Size = \t" << SystemSettings::Instance().buffersize << " samples" << std::endl;
    std::cerr << "Internal latency = \t\t" << SystemSettings::Instance().buffersize_f * 1000.0f / SystemSettings::Instance().samplerate_f << " ms" << std::endl;
    std::cerr << "ADsynth Oscil.Size = \t" << SystemSettings::Instance().oscilsize << " samples" << std::endl;

    mixer = new Mixer();
    mixer->Init();
    mixer->swaplr = Config::Current().cfg.SwapStereo;

    Nio::preferedSampleRate(SystemSettings::Instance().samplerate);

    signal(SIGINT, sigterm_exit);
    signal(SIGTERM, sigterm_exit);
}

/*
 * Program exit
 */
int exitprogram()
{
    //ensure that everything has stopped with the mutex wait
    mixer->Lock();
    mixer->Unlock();

    Nio::Stop();

    delete mixer;
    FFT_cleanup();

    return 0;
}

int main(int /*argc*/, char * /*argv*/ [])
{
    initprogram();

    for (int i = 0; i < MAX_BANK_ROOT_DIRS; i++)
    {
        if (Config::Current().cfg.bankRootDirList[i].size() == 0)
        {
            continue;
        }
        libraryManager.AddLibraryLocation(Config::Current().cfg.bankRootDirList[i]);
    }
    //Run the Nio system
    if (!Nio::Start(mixer, mixer))
    {
        return -1;
    }

    Nio::SelectSink("PA");
    Nio::SelectSource("RT");

    Config::Current().init();

    if (glfwInit() == GLFW_FALSE)
    {
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(1024, 768, "zynlab", nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        return -1;
    }

    AppThreeDee app(window, mixer, &libraryManager);

    glfwSetWindowSizeCallback(window, AppThreeDee::ResizeCallback);
    glfwMakeContextCurrent(window);

    gladLoadGL();

    AppThreeDee::ResizeCallback(window, 1024, 768);

    if (app.Setup())
    {
        glfwSetKeyCallback(window, AppThreeDee::KeyActionCallback);
        //    glfwSetFramebufferSizeCallback(window, AppThreeDee::ResizeCallback);

        while (glfwWindowShouldClose(window) == 0)
        {
            app.Tick();

            glfwPollEvents();

            glClear(GL_COLOR_BUFFER_BIT);

            app.Render();

            glfwSwapBuffers(window);
        }
        app.Cleanup();
    }

    glfwTerminate();

    return exitprogram();
}
