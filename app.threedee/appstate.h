#ifndef APPSTATE_H
#define APPSTATE_H

#include <zyn.seq/Sequencer.h>

extern char const *const AdSynthEditorID;
extern char const *const SubSynthEditorID;
extern char const *const PadSynthEditorID;
extern char const *const InsertionFxEditorID;
extern char const *const SystemFxEditorID;
extern char const *const ChannelFxEditorID;
extern char const *const LibraryID;
extern char const *const StepPatternEditorID;
extern char const *const EffectNames[];
extern int EffectNameCount;
extern const char *NoteNames[];
extern int NoteNameCount;
extern const char *ChannelPatternTypeNames[];
extern const float noteLabelWidth;
extern const float stepWidth;

enum class ChannelPatternTypes
{
    Step,
    Clip,
    Arpeggiator,
    Count,
};

class AppState
{
public:
    AppState(class Mixer *mixer, class Stepper *stepper);
    virtual ~AppState();

    class Mixer *_mixer;
    class Stepper *_stepper;
    bool _showLibrary;
    bool _showEditor;
    bool _showInspector;
    bool _showMixer;
    bool _showSmartControls;
    bool _showSystemEffectsEditor;
    bool _showInsertEffectsEditor;
    bool _showChannelEffectsEditor;
    bool _showADNoteEditor;
    bool _showSUBNoteEditor;
    bool _showPADNoteEditor;
    int _showChannelTypeChanger;
    int _currentInsertEffect;
    int _currentSystemEffect;
    int _currentChannelEffect;
    int _currentBank;
    int _activeChannel;
    int _activeChannelInstrument;
    int _activePattern;
    int _sequencerChannelHeight;
    Sequencer _sequencer;
    std::vector<TrackPattern> _clipboardPatterns;
    ChannelPatternTypes _channelPatternType[NUM_MIXER_CHANNELS];
};

#endif // APPSTATE_H
