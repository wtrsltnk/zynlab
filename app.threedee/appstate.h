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
extern std::vector<std::string> EffectNames;
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
    timestep playUntil = 0;
    unsigned int channel = 0;
    unsigned int note = 0;
    bool done = false;
};

void CleanupPreviewImage(
    unsigned int previewImage);

void UpdatePreviewImage(
    TrackRegion &region);

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
    AppState(
        class Mixer *mixer,
        ILibraryManager *library);

    virtual ~AppState();

    class Mixer *_mixer = nullptr;
    ILibraryManager *_library = nullptr;
    int _currentInsertEffect = 0;
    int _currentSystemEffect = 0;
    int _currentTrackEffect = 0;
    ILibrary *_currentLibrary = nullptr;
    int _currentTrack = 0;
    int _currentTrackInstrument = 0;
    int _currentPattern = 0;
    int _currentVoiceOscil = -1;
    int _sequencerVerticalZoom = 50;
    int _sequencerHorizontalZoom = 50;
    int _pianoRollEditorHorizontalZoom = 150;
    int _showTrackTypeChanger = -1;
    bool selectingFromLibrary = false;

    int _bpm = 138;
    bool _isPlaying = false;
    bool _isRecording = false;
    std::chrono::milliseconds::rep _playTime = 0;
    std::chrono::milliseconds::rep _maxPlayTime = 4 * 1024;
    RegionsManager _regions;

    std::vector<tempnote> _tempnotes;

    class UiState
    {
    public:
        UiState();

        AppMode _activeMode = AppMode::Regions;
    } _uiState;
};

#endif // APPSTATE_H
