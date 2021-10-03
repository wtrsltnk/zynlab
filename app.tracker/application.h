#ifndef APPLICATION_H
#define APPLICATION_H

#include "applicationsession.h"
#include "automationeditor.h"
#include "effectseditor.h"
#include "instrumentspanel.h"
#include "mixereditor.h"
#include "patterneditor.h"
#include "patternsmanager.h"
#include "playercontrolspanel.h"
#include <iapplication.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.ui/syntheditor.h>

class Application :
    public IApplication,
    public INoteSource
{
    std::unique_ptr<Mixer> _mixer;
    std::unique_ptr<LibraryManager> _library;
    ApplicationSession _session;

    AutomationEditor _automationEditor;
    EffectsEditor _effectsEditor;
    InstrumentsPanel _instruments;
    MixerEditor _mixerEditor;
    PatternEditor _patternEditor;
    PatternsManager _patternsManager;
    PlayerControlsPanel _playerControlsPanel;
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
    const int playerControlsPanelHeight = 36;
    const int playerControlsPanelWidth = 168;

    virtual void Render2d();

    virtual void Cleanup();

    virtual std::vector<SimpleNote> GetNotes(
        unsigned int frameCount,
        unsigned int sampleRate);

    void NextStep();

    std::vector<SimpleNote> GetCurrentStepNotes();
};

#endif // APPLICATION_H
