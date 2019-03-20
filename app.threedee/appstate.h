#ifndef APPSTATE_H
#define APPSTATE_H

#include "imgui_addons/imgui_common.h"
#include <vector>
#include <zyn.common/globals.h>
#include <zyn.seq/RegionsManager.h>

extern char const *const AdSynthEditorID;
extern char const *const SubSynthEditorID;
extern char const *const SmplSynthEditorID;
extern char const *const PadSynthEditorID;
extern char const *const InsertionFxEditorID;
extern char const *const SystemFxEditorID;
extern char const *const TrackFxEditorID;
extern char const *const OscillatorEditorID;
extern char const *const LibraryID;
extern char const *const StepPatternEditorID;
extern char const *const EffectNames[];
extern unsigned int EffectNameCount;
extern char const *const NoteNames[];
extern unsigned int NoteNameCount;
extern char const *const SnappingModes[];
extern unsigned int SnappingModeCount;
extern timestep SnappingModeValues[];
extern char const *const ArpModeNames[];
extern unsigned int ArpModeCount;
extern char const *const ChordNames[];
extern unsigned int ChordCount;

struct tempnote
{
    timestep playUntil;
    unsigned int channel;
    unsigned int note;
    bool done;
};

void CleanupPreviewImage(unsigned int previewImage);
void UpdatePreviewImage(TrackRegion &region);

class AppState
{
public:
    AppState(class Mixer *mixer, class IBankManager *banks);
    virtual ~AppState();

    class Mixer *_mixer;
    class IBankManager *_banks;
    bool _showLibrary;
    bool _showEditor;
    bool _showInspector;
    bool _showMixer;
    bool _showSmartControls;
    bool _showQuickHelp;
    bool _showSystemEffectsEditor;
    bool _showInsertEffectsEditor;
    bool _showTrackEffectsEditor;
    bool _showADNoteEditor;
    bool _showSUBNoteEditor;
    bool _showSMPLNoteEditor;
    bool _showPADNoteEditor;
    bool _showOscillatorEditor;
    int _showTrackTypeChanger;
    int _currentInsertEffect;
    int _currentSystemEffect;
    int _currentTrackEffect;
    int _currentBank;
    int _currentTrack;
    int _currentTrackInstrument;
    int _currentPattern;
    int _currentVoiceOscil;
    int _sequencerVerticalZoom;
    int _sequencerHorizontalZoom;
    int _pianoRollEditorHorizontalZoom;

    int _bpm;
    bool _isPlaying;
    std::chrono::milliseconds::rep _playTime;
    std::chrono::milliseconds::rep _maxPlayTime;
    RegionsManager _regions;

    std::vector<tempnote> _tempnotes;
};

#endif // APPSTATE_H
