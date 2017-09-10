
#define GLEXTL_IMPLEMENTATION
#include <GL/glextl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"

#include "app.threedee.h"

using namespace std;

static int Pexitprogram = 0;

//cleanup on signaled exit
void sigterm_exit(int /*sig*/)
{
    Pexitprogram = 1;
}

/*
 * Program initialisation
 */
void initprogram(void)
{
    auto synth = new SystemSettings;
    Config::Current().init();

    /* Get the settings from the Config*/
    synth->samplerate = Config::Current().cfg.SampleRate;
    synth->buffersize = Config::Current().cfg.SoundBufferSize;
    synth->oscilsize  = Config::Current().cfg.OscilSize;

    synth->alias();

    cerr.precision(1);
    cerr << std::fixed;
    cerr << "\nSample Rate = \t\t" << synth->samplerate << endl;
    cerr << "Sound Buffer Size = \t" << synth->buffersize << " samples" << endl;
    cerr << "Internal latency = \t\t" << synth->buffersize_f * 1000.0f / synth->samplerate_f << " ms" << endl;
    cerr << "ADsynth Oscil.Size = \t" << synth->oscilsize << " samples" << endl;

    mixer = new Mixer(synth);
    mixer->swaplr = Config::Current().cfg.SwapStereo;

    Nio::preferedSampleRate(synth->samplerate);

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

    Nio::stop();

    delete mixer->_synth;
    delete mixer;
    FFT_cleanup();

    return 0;
}

int main(int argc, char *argv[])
{
    initprogram();

    //Run the Nio system
    if (!Nio::start(mixer))
        return -1;

    Nio::setSink("PA");

//    mixer->NoteOn(0, 60, 200);

    if (glfwInit() == GLFW_FALSE)
        return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "zynlab", NULL, NULL);
    if (window == 0)
    {
        glfwTerminate();
        return -1;
    }

    AppThreeDee app(window, mixer);

    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);

    glfwSetKeyCallback(window, AppThreeDee::KeyActionCallback);
//    glfwSetFramebufferSizeCallback(window, AppThreeDee::ResizeCallback);

    glfwSetWindowSizeCallback(window, AppThreeDee::ResizeCallback);
    glfwMakeContextCurrent(window);

    glExtLoadAll((PFNGLGETPROC*)glfwGetProcAddress);

    AppThreeDee::ResizeCallback(window, 800, 600);

    if (app.SetUp())
    {
        while (glfwWindowShouldClose(window) == 0)
        {
            glfwWaitEvents();

            glClear(GL_COLOR_BUFFER_BIT);

            app.Render();

            glfwSwapBuffers(window);

        }
        app.CleanUp();
    }

    glfwTerminate();

    return exitprogram();
}
