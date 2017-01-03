
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
    synth = new SYNTH_T;
    config.init();

    /* Get the settings from the Config*/
    synth->samplerate = config.cfg.SampleRate;
    synth->buffersize = config.cfg.SoundBufferSize;
    synth->oscilsize  = config.cfg.OscilSize;

    synth->alias();

    //produce denormal buf
    denormalkillbuf = new float [synth->buffersize];
    for(int i = 0; i < synth->buffersize; ++i)
        denormalkillbuf[i] = (RND - 0.5f) * 1e-16;

    cerr.precision(1);
    cerr << std::fixed;
    cerr << "\nSample Rate = \t\t" << synth->samplerate << endl;
    cerr << "Sound Buffer Size = \t" << synth->buffersize << " samples" << endl;
    cerr << "Internal latency = \t\t" << synth->buffersize_f * 1000.0f / synth->samplerate_f << " ms" << endl;
    cerr << "ADsynth Oscil.Size = \t" << synth->oscilsize << " samples" << endl;

    mixer = &Mixer::getInstance();
    mixer->swaplr = config.cfg.SwapStereo;

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

    delete [] denormalkillbuf;
    Mixer::deleteInstance();
    FFT_cleanup();

    return 0;
}

int main(int argc, char *argv[])
{
    initprogram();

    //Run the Nio system
    if (!Nio::start(mixer))
        return -1;

    if (glfwInit() == GLFW_FALSE)
        return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "zyn", NULL, NULL);
    if (window == 0)
    {
        glfwTerminate();
        return -1;
    }

    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);

    glfwSetKeyCallback(window, KeyActionCallback);
    glfwSetFramebufferSizeCallback(window, ResizeCallback);

    glfwMakeContextCurrent(window);

    glExtLoadAll((PFNGLGETPROC*)glfwGetProcAddress);

    if (SetUp())
    {
        while (glfwWindowShouldClose(window) == 0)
        {
            glfwPollEvents();

            glClear(GL_COLOR_BUFFER_BIT);

            Render();

            glfwSwapBuffers(window);

        }
        CleanUp();
    }

    glfwTerminate();

    return exitprogram();
}
