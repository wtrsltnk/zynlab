#ifndef APPSTATE_H
#define APPSTATE_H

#include <zyn.seq/Sequencer.h>

extern char const *const AdSynthEditorID;
extern char const *const SubSynthEditorID;
extern char const *const PadSynthEditorID;
extern char const *const InsertionFxEditorID;
extern char const *const SystemFxEditorID;
extern char const *const TrackFxEditorID;
extern char const *const LibraryID;
extern char const *const StepPatternEditorID;
extern char const *const EffectNames[];
extern int EffectNameCount;
extern const char *NoteNames[];
extern int NoteNameCount;
extern const char *TrackPatternTypeNames[];
extern const float noteLabelWidth;
extern const float stepWidth;

enum class TrackPatternTypes
{
    Step,
    Clip,
    Arpeggiator,
    Count,
};

class AppState
{
public:
    AppState(class Mixer *mixer, class Stepper *stepper, class IBankManager *banks);
    virtual ~AppState();

    class Mixer *_mixer;
    class Stepper *_stepper;
    class IBankManager *_banks;
    bool _showLibrary;
    bool _showEditor;
    bool _showInspector;
    bool _showMixer;
    bool _showQuickHelp;
    bool _showSystemEffectsEditor;
    bool _showInsertEffectsEditor;
    bool _showTrackEffectsEditor;
    int _showTrackTypeChanger;
    int _currentInsertEffect;
    int _currentSystemEffect;
    int _currentTrackEffect;
    int _currentBank;
    int _activeTrack;
    int _activeTrackInstrument;
    int _activePattern;
    int _sequencerVerticalZoom;
    int _sequencerHorizontalZoom;
    Sequencer _sequencer;
    std::vector<TrackPattern> _clipboardPatterns;
};

#endif // APPSTATE_H