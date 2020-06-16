#ifndef PATTERNEDITOR_H
#define PATTERNEDITOR_H

#include <map>
#include <string>
#include <vector>
#include <zyn.common/globals.h>

#include <imgui.h>
#include  "song.h"

class PatternEditor
{
    IMixer *_mixer;
    Song *_song;

    ImFont *_monofont;
    bool _editMode;
    unsigned int _columnsWidths[NUM_MIXER_TRACKS];

public:
    PatternEditor();

    void SetUp(IMixer *mixer, ImFont *font);
    Song *CurrentSong();

    float tracksScrollx = 0;
    float tracksScrolly = 0;

    void Render2d();

    const unsigned int numTracks = NUM_MIXER_TRACKS;
    unsigned int currentTrack = 0;
    unsigned int currentRow = 0;
    unsigned int currentProperty = 0;
    unsigned int skipRowStepSize = 4;
    bool keepRowInFocus = false;

    const char *emptyCellNote = "---";
    const char *emptyCellParameter = "..";
    const char *emptyCellFx = "000";

    const int headerHeight = 125;
    const int footerHeight = 20;
    const int rowIndexColumnWidth = 30;
    const int scrollbarHeight = 20;

    std::map<char, unsigned int> _charToNoteMap{
        {'Z', 60}, // C-4
        {'S', 61}, // C#4
        {'X', 62}, // D-4
        {'D', 63}, // D#4
        {'C', 64}, // E-4
        {'V', 65}, // F-4
        {'G', 66}, // F#4
        {'B', 67}, // G-4
        {'H', 68}, // G#4
        {'N', 69}, // A-4
        {'J', 70}, // A#4
        {'M', 71}, // B-4

        {'Q', 72}, // C-5
        {'2', 73}, // C#5
        {'W', 74}, // D-5
        {'3', 75}, // D#5
        {'E', 76}, // E-5
        {'R', 77}, // F-5
        {'5', 78}, // F#5
        {'T', 79}, // G-5
        {'6', 80}, // G#5
        {'Y', 81}, // A-5
        {'7', 82}, // A#5
        {'U', 83}, // B-5

        {'I', 84}, // C-6
        {'9', 85}, // C#6
        {'O', 86}, // D-6
        {'0', 87}, // D#6
        {'P', 88}, // E-6
    };

    bool HandlePlayingNotes(bool repeat = false);

    bool HandleKeyboardNotes();
    bool HandleKeyboardNavigation();
    void MoveCurrentRowUp(bool largeStep);
    void MoveCurrentRowDown(bool largeStep);
    void ChangeCurrentTrack(bool moveLeft);
    void MoveToPreviousProperty();
    void MoveToNextProperty();
    bool IsRecording() const;
    void ToggleRecording();
};

#endif // PATTERNEDITOR_H
