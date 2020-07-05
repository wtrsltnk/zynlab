#ifndef APPLICATION_H
#define APPLICATION_H

#include "applicationsession.h"
#include "automationeditor.h"
#include "effectseditor.h"
#include "instrumentspanel.h"
#include "patterneditor.h"
#include "patternsmanager.h"
#include "playercontrolspanel.h"
#include "syntheditor.h"
#include <iapplication.h>

class Application :
    public IApplication,
    public INoteSource
{
    ApplicationSession _session;

    PlayerControlsPanel _playerControlsPanel;
    PatternEditor _patternEditor;
    InstrumentsPanel _instruments;
    EffectsEditor _effectsEditor;
    PatternsManager _patternsManager;
    AutomationEditor _automationEditor;
    SynthEditor _synthEditor;

    unsigned int _sampleIndex;

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
