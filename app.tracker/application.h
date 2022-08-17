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

class MixerAutomation
{
public:
    enum AutomationValueTypes
    {
        UnsignedChar,
        UnsignedChar_Array,
        UnsignedChar_2dArray,
        ShortInt,
        ShortInt_Arrary,
        ShortInt_2dArrary,
        Float,
    };
    struct AutomationValue
    {
        const char *description;
        AutomationValueTypes valueType = AutomationValueTypes::UnsignedChar;
        void *value;
    };

public:
    virtual ~MixerAutomation();

    void Setup(
        Mixer *mixer);

private:
    Mixer *_mixer = nullptr;
    std::map<const char *, AutomationValue> _automationValues;
};

class Application :
    public IApplication,
    public INoteSource
{
    std::unique_ptr<Mixer> _mixer;
    std::unique_ptr<LibraryManager> _library;
    ApplicationSession _session;

    AutomationEditor _automationEditor;
    EffectsAndAutomationEditor _effectsEditor;
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
    const int effectsPanelHeight = 210;
    const int playerControlsPanelHeight = 36;
    const int playerControlsPanelWidth = 168;

    virtual void Render2d();

    virtual void Cleanup();

    virtual std::vector<SimpleNote> GetNotes(
        unsigned int frameCount,
        unsigned int sampleRate);

    void NextStep();

    void UpdateAutomatedParams();
    std::vector<SimpleNote> GetCurrentStepNotes();
};

#endif // APPLICATION_H
