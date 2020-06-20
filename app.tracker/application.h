#ifndef APPLICATION_H
#define APPLICATION_H

#include "applicationsession.h"
#include "effectseditor.h"
#include "instrumentspanel.h"
#include "patterneditor.h"
#include <iapplication.h>
#include <zyn.mixer/Mixer.h>

enum class PlayStates
{
    Stopped,
    StartPlaying,
    Playing,
};

class Application :
    public IApplication,
    public INoteSource
{
    ApplicationSession _session;
    Mixer *_mixer;
    ILibraryManager *_library;

    PatternEditor _patternEditor;
    InstrumentsPanel _instruments;
    EffectsEditor _effectsEditor;

    unsigned int _sampleIndex;
    PlayStates _playState;

    ImFont *_monofont;
    ImFont *_fkFont;
    ImFont *_fadFont;

public:
    Application();

    virtual bool Setup();

    const int instrumentPanelWidth = 370;
    const int effectsPanelHeight = 160;
    const int tabbarPanelHeight = 40;
    const int playerControlsPanelWidth = 153;
    const int playerControlsPanelHeight = 40;

    virtual void Render2d();

    virtual void Cleanup();

    void StopPlaying();

    void StartPlaying();

    void TogglePlaying();

    virtual std::vector<SimpleNote> GetNotes(
        unsigned int frameCount,
        unsigned int sampleRate);

    void NextStep();

    std::vector<SimpleNote> GetCurrentStepNotes();
};

#endif // APPLICATION_H
