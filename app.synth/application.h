#ifndef APPLICATION_H
#define APPLICATION_H

#include <iapplication.h>
#include <imgui.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.serialization/LibraryManager.h>
#include <zyn.ui/syntheditor.h>

class Application :
    public IApplication,
    public INoteSource
{
    std::unique_ptr<Mixer> _mixer;
    std::unique_ptr<LibraryManager> _library;

    SynthEditor _synthEditor;

    unsigned int _sampleIndex = 0;

    ImFont *_monofont = nullptr;
    ImFont *_fkFont = nullptr;
    ImFont *_fadFont = nullptr;

public:
    Application();

    virtual bool Setup();

    const int instrumentPanelWidth = 370;
    const int effectsPanelHeight = 160;
    const int tabbarPanelHeight = 40;
    const int playerControlsPanelWidth = 153;
    const int playerControlsPanelHeight = 56;

    virtual void Render2d();

    virtual void Cleanup();

    virtual std::vector<SimpleNote> GetNotes(
        unsigned int frameCount,
        unsigned int sampleRate);
};

#endif // APPLICATION_H
