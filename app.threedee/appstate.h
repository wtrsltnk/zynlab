#ifndef APPSTATE_H
#define APPSTATE_H

#include <vector>
#include <zyn.common/ILibraryManager.h>
#include <zyn.common/globals.h>
#include <zyn.seq/RegionsManager.h>

#define TRACK_WIDTH 150

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

enum class AppMode
{
    Regions,
    Editor,
    Mixer,
    Instrument,
    Effect,
};

class AppState
{
public:
    AppState(class Mixer *mixer, ILibraryManager *library);
    virtual ~AppState();

    class Mixer *_mixer;
    ILibraryManager *_library;
    int _currentInsertEffect;
    int _currentSystemEffect;
    int _currentTrackEffect;
    ILibrary *_currentLibrary;
    int _currentTrack;
    int _currentTrackInstrument;
    int _currentPattern;
    int _currentVoiceOscil;
    int _sequencerVerticalZoom;
    int _sequencerHorizontalZoom;
    int _pianoRollEditorHorizontalZoom;
    int _showTrackTypeChanger;

    int _bpm;
    bool _isPlaying;
    bool _isRecording;
    std::chrono::milliseconds::rep _playTime;
    std::chrono::milliseconds::rep _maxPlayTime;
    RegionsManager _regions;

    std::vector<tempnote> _tempnotes;

    class UiState
    {
    public:
        UiState();

        AppMode _activeMode;
    } _uiState;
};

#endif // APPSTATE_H
