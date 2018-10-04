
#include "app.threedee.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <imgui.h>

#include "examples/imgui_impl_opengl3.h"

static int Pexitprogram = 0;

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
    auto synth = new SystemSettings;
    Config::Current().init();

    /* Get the settings from the Config*/
    synth->samplerate = Config::Current().cfg.SampleRate;
    synth->buffersize = Config::Current().cfg.SoundBufferSize;
    synth->oscilsize = Config::Current().cfg.OscilSize;

    synth->alias();

    std::cerr.precision(1);
    std::cerr << std::fixed;
    std::cerr << "\nSample Rate = \t\t" << synth->samplerate << std::endl;
    std::cerr << "Sound Buffer Size = \t" << synth->buffersize << " samples" << std::endl;
    std::cerr << "Internal latency = \t\t" << synth->buffersize_f * 1000.0f / synth->samplerate_f << " ms" << std::endl;
    std::cerr << "ADsynth Oscil.Size = \t" << synth->oscilsize << " samples" << std::endl;

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

    Nio::Stop();

    delete mixer->_synth;
    delete mixer;
    FFT_cleanup();

    return 0;
}

int main(int /*argc*/, char * /*argv*/ [])
{
    initprogram();

    //Run the Nio system
    if (!Nio::Start(mixer))
    {
        return -1;
    }

    Nio::SelectSink("PA");
    Nio::SelectSource("NET");

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

    AppThreeDee app(window, mixer);

    glfwSetWindowSizeCallback(window, AppThreeDee::ResizeCallback);
    glfwMakeContextCurrent(window);

    gladLoadGL();

    AppThreeDee::ResizeCallback(window, 1024, 768);

    if (app.SetUp())
    {
        glfwSetKeyCallback(window, AppThreeDee::KeyActionCallback);
        //    glfwSetFramebufferSizeCallback(window, AppThreeDee::ResizeCallback);

        while (glfwWindowShouldClose(window) == 0)
        {
            glfwPollEvents();

            glClear(GL_COLOR_BUFFER_BIT);

            app.Render();

            glfwSwapBuffers(window);
        }
        app.CleanUp();
    }

    glfwTerminate();

    return exitprogram();
}
