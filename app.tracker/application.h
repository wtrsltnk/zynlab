#ifndef APPLICATION_H
#define APPLICATION_H

#include "applicationsession.h"
#include "automationeditor.h"
#include "effectseditor.h"
#include "instrumentspanel.h"
#include "patterneditor.h"
#include "patternsmanager.h"
#include "playercontrolspanel.h"
#include <zyn.ui/syntheditor.h>
#include <iapplication.h>
#include <zyn.mixer/Mixer.h>

class Application :
    public IApplication,
    public INoteSource
{
    std::unique_ptr<Mixer> _mixer;
    std::unique_ptr<LibraryManager> _library;
    ApplicationSession _session;

    PlayerControlsPanel _playerControlsPanel;
    PatternEditor _patternEditor;
    InstrumentsPanel _instruments;
    EffectsEditor _effectsEditor;
    PatternsManager _patternsManager;
    AutomationEditor _automationEditor;
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

    void NextStep();

    std::vector<SimpleNote> GetCurrentStepNotes();
};

#endif // APPLICATION_H
