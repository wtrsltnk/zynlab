#ifndef APPSTATE_H
#define APPSTATE_H

extern char const *const ADeditorID;
extern char const *const SUBeditorID;
extern char const *const PADeditorID;
extern char const *const InsertionFxEditorID;
extern char const *const SystemFxEditorID;
extern char const *const InstrumentFxEditorID;
extern char const *const effectNames[];
extern int effectNameCount;
extern const char *notes[];
extern int noteCount;

class AppState
{
public:
    AppState(class Mixer *mixer);
    virtual ~AppState();

    class Mixer *_mixer;
    bool _showLibrary;
    bool _showEditor;
    bool _showInspector;
    bool _showMixer;
    bool _showInstrumentEditor;
    bool _showSystemEffectsEditor;
    bool _showInsertEffectsEditor;
    bool _showInstrumentEffectsEditor;
    bool _showADNoteEditor;
    bool _showSUBNoteEditor;
    bool _showPADNoteEditor;
    int _openSelectInstrument;
    int _openChangeInstrumentType;
    int _currentInsertEffect;
    int _currentSystemEffect;
    int _currentInstrumentEffect;
    int _currentBank;
    int _activeInstrument;
    int _activePattern;
};

#endif // APPSTATE_H
