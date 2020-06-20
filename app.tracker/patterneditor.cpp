#include "patterneditor.h"

#include <imgui.h>
#include <iostream>
#include <zyn.mixer/Track.h>

ImVec2 operator+(ImVec2 const &a, ImVec2 const &b)
{
    return ImVec2(a.x + b.x, a.y + b.y);
}

ImVec2 operator-(ImVec2 const &a, ImVec2 const &b)
{
    return ImVec2(a.x - b.x, a.y - b.y);
}

namespace ImGui
{
    void MoveCursorPos(
        ImVec2 delta)
    {
        SetCursorPos(GetCursorPos() + delta);
    }
} // namespace ImGui

PatternEditor::PatternEditor()
    : _session(nullptr),
      _mixer(nullptr),
      _song(nullptr),
      _monofont(nullptr),
      _editMode(false)
{
    for (int i = 0; i < NUM_MIXER_TRACKS; i++)
    {
        _columnsWidths[i] = 0;
    }
}

void PatternEditor::SetUp(
    ApplicationSession *session,
    IMixer *mixer,
    ImFont *font)
{
    _session = session;
    _mixer = mixer;
    _monofont = font;
    _song = new Song();

    _song->AddPattern();
    auto pattern = _song->GetPattern(0);

    for (int i = 0; i < 16; i++)
    {
        pattern->Notes(0)[i * 4]._note = 60 + i;
        pattern->Notes(0)[i * 4]._length = 64;
        pattern->Notes(0)[i * 4]._velocity = 100;
    }
}

Song *PatternEditor::CurrentSong()
{
    return _song;
}

void PatternEditor::Render2d()
{
    auto selectionColor = ImColor(20, 180, 20, 255);
    auto selectedRowBackgroundColorEditmode = ImColor(20, 220, 20, 55);
    auto selectedRowBackgroundColor = ImColor(70, 120, 70, 70);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::Begin(
        "PatternEditor",
        nullptr,
        flags);
    {
        ImGui::PushFont(_monofont);
        auto content = ImGui::GetContentRegionAvail() - ImVec2(rowIndexColumnWidth, 0);
        float lineHeight = 30;

        auto spaceWidth = ImGui::CalcTextSize(" ");
        auto cellNoteWidth = ImGui::CalcTextSize(emptyCellNote);
        auto cellParameterWidth = ImGui::CalcTextSize(emptyCellParameter);
        auto cellFxWidth = ImGui::CalcTextSize(emptyCellFx);
        auto columnWidth = cellNoteWidth +
                           spaceWidth + cellParameterWidth +
                           spaceWidth + cellParameterWidth +
                           spaceWidth + cellFxWidth;
        auto fullWidth = (columnWidth.x + 15) * NUM_MIXER_TRACKS;

        auto contentTop = ImGui::GetCursorPosY();
        ImGui::MoveCursorPos(ImVec2(rowIndexColumnWidth, headerHeight));
        ImGui::BeginChild(
            "container",
            ImVec2(content.x, -(footerHeight + scrollbarHeight)));
        {
            ImGui::SetScrollX(tracksScrollx);

            lineHeight = ImGui::GetTextLineHeightWithSpacing();

            auto pattern = CurrentSong()->GetPattern(CurrentSong()->currentPattern);

            if (pattern != nullptr)
            {
                ImGui::BeginChild(
                    "tracks",
                    ImVec2(fullWidth, lineHeight * pattern->Length()));
                {
                    auto tracksPos = ImGui::GetWindowContentRegionMin();
                    auto tracksMax = ImGui::GetWindowContentRegionMax();

                    auto selectionRowMin = ImVec2(tracksPos.x, tracksPos.y + _session->currentRow * lineHeight);
                    auto selectionRowMax = ImVec2(tracksMax.x, tracksPos.y + (_session->currentRow + 1) * lineHeight);

                    auto drawList = ImGui::GetWindowDrawList();

                    for (unsigned int i = 0; i < pattern->Length(); i += 4)
                    {
                        auto highlightRowMin = ImVec2(tracksPos.x, tracksPos.y + i * lineHeight);
                        auto highlightRowMax = ImVec2(tracksMax.x, tracksPos.y + (i + 1) * lineHeight);

                        drawList->AddRectFilled(
                            ImGui::GetWindowPos() + highlightRowMin,
                            ImGui::GetWindowPos() + highlightRowMax,
                            ImColor(120, 120, 120, 55));
                    }

                    // SELECTED ROW

                    auto color = _editMode ? selectedRowBackgroundColorEditmode : selectedRowBackgroundColor;
                    drawList->AddRectFilled(
                        ImGui::GetWindowPos() + selectionRowMin,
                        ImGui::GetWindowPos() + selectionRowMax,
                        color);

                    // MAKE ROOM FOR THE HEADERS

                    ImGui::Columns(NUM_MIXER_TRACKS);
                    for (unsigned int i = 0; i < NUM_MIXER_TRACKS; i++)
                    {
                        _columnsWidths[i] = columnWidth.x;
                        ImGui::SetColumnWidth(i, _columnsWidths[i] + 15);
                    }

                    // ALL TRACKS AND CELLS

                    for (unsigned int r = 0; r < pattern->Length(); r++)
                    {
                        for (unsigned int i = 0; i < NUM_MIXER_TRACKS; i++)
                        {
                            auto notes = pattern->Notes(i);
                            auto markerPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
                            if (i == _session->currentTrack && r == _session->currentRow)
                            {
                                auto cursorWidth = cellNoteWidth.x;
                                auto min = markerPos + ImVec2(2, 0) - ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
                                if (_session->currentProperty > 0)
                                {
                                    min.x += cellNoteWidth.x + spaceWidth.x - 1;
                                    cursorWidth = cellParameterWidth.x;
                                }
                                if (_session->currentProperty > 1)
                                {
                                    min.x += cellParameterWidth.x + spaceWidth.x - 1;
                                    cursorWidth = cellParameterWidth.x;
                                }
                                if (_session->currentProperty > 2)
                                {
                                    min.x += cellParameterWidth.x + spaceWidth.x;
                                    cursorWidth = cellFxWidth.x;
                                }

                                drawList->AddRectFilled(
                                    min - ImVec2(4, 0),
                                    min + ImVec2(cursorWidth, lineHeight),
                                    selectionColor);
                            }

                            char const *cellNote = emptyCellNote;
                            char const *cellParameter1 = emptyCellParameter;
                            char const *cellParameter2 = emptyCellParameter;
                            char const *cellFx = emptyCellFx;
                            if (notes[r]._note != 0)
                            {
                                cellNote = Note::NoteToString(notes[r]._note);
                                if (notes[r]._length != 0)
                                {
                                    cellParameter1 = Note::ValueToString(notes[r]._length);
                                }
                                if (notes[r]._velocity != 0)
                                {
                                    cellParameter2 = Note::ValueToString(notes[r]._velocity);
                                }
                            }
                            ImGui::Text("%s %s %s %s", cellNote, cellParameter1, cellParameter2, cellFx);
                            ImGui::NextColumn();
                        }
                    }
                }
                ImGui::EndChild();
            }

            // EVENTS

            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
            {
                if (HandleKeyboardNotes())
                {
                    ImGui::SetScrollY((_session->currentRow - (content.y / lineHeight) / 2 + 1) * lineHeight);
                }
                else if (HandleKeyboardNavigation())
                {
                    ImGui::SetScrollY((_session->currentRow - (content.y / lineHeight) / 2 + 1) * lineHeight);
                    auto scrollx = (_session->currentTrack - (content.x / columnWidth.x) / 2 + 1) * columnWidth.x;
                    tracksScrollx = scrollx > 0 ? scrollx : 0;
                    ImGui::SetScrollX(tracksScrollx);
                }
            }

            if (keepRowInFocus)
            {
                ImGui::SetScrollY((_session->currentRow - (content.y / lineHeight) / 2 + 1) * lineHeight);
                auto scrollx = (_session->currentTrack - (content.x / columnWidth.x) / 2 + 1) * columnWidth.x;
                ImGui::SetScrollX(scrollx);
                keepRowInFocus = false;
            }

            tracksScrolly = ImGui::GetScrollY();
        }
        ImGui::EndChild();

        // FOOTERS

        ImGui::MoveCursorPos(ImVec2(rowIndexColumnWidth, 0));
        ImGui::BeginChild(
            "footerscontainer",
            ImVec2(content.x, footerHeight));
        {
            ImGui::SetScrollX(tracksScrollx);

            ImGui::BeginChild(
                "footers",
                ImVec2(fullWidth + ImGui::GetStyle().ScrollbarSize, footerHeight));
            {
                ImGui::Columns(NUM_MIXER_TRACKS);
                for (unsigned int i = 0; i < NUM_MIXER_TRACKS; i++)
                {
                    ImGui::PushID(i);
                    auto drawList = ImGui::GetWindowDrawList();
                    auto markerPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
                    if (i == _session->currentTrack)
                    {
                        drawList->AddLine(
                            markerPos + ImVec2(-4, 2),
                            markerPos + ImVec2(-4, 16),
                            selectionColor,
                            2);
                        drawList->AddLine(
                            markerPos + ImVec2(-4, 16),
                            markerPos + ImVec2(10, 16),
                            selectionColor,
                            3);
                        drawList->AddLine(
                            markerPos + ImVec2(_columnsWidths[i] + 3, 2),
                            markerPos + ImVec2(_columnsWidths[i] + 3, 16),
                            selectionColor,
                            2);
                        drawList->AddLine(
                            markerPos + ImVec2(_columnsWidths[i] + 4, 16),
                            markerPos + ImVec2(_columnsWidths[i] - 11, 16),
                            selectionColor,
                            3);
                    }

                    auto w = ImGui::CalcTextSize("footer 00").x;
                    ImGui::SetColumnWidth(i, _columnsWidths[i] + 15);
                    ImGui::MoveCursorPos(ImVec2((ImGui::GetContentRegionAvailWidth() - w) / 2.0f, 0));
                    ImGui::Text("footer %02d", i + 1);
                    ImGui::PopID();
                    ImGui::NextColumn();
                }
                ImGui::Columns(1);
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

        // SCROLLBAR

        ImGui::MoveCursorPos(ImVec2(rowIndexColumnWidth, 0));
        ImGui::BeginChild(
            "scrollbar",
            ImVec2(content.x, scrollbarHeight),
            false,
            ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImGui::SetScrollX(tracksScrollx);

            ImGui::BeginChild(
                "scrollbarcontent",
                ImVec2(fullWidth + ImGui::GetStyle().ScrollbarSize, scrollbarHeight));
            {
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

        // ROWINDICES

        ImGui::SetCursorPosY(contentTop);
        ImGui::MoveCursorPos(ImVec2(0, headerHeight));
        ImGui::BeginChild(
            "rowinedicescontainer",
            ImVec2(rowIndexColumnWidth, -(footerHeight + scrollbarHeight)),
            false,
            ImGuiWindowFlags_NoScrollbar);
        {
            ImGui::SetScrollY(tracksScrolly);

            auto pattern = CurrentSong()->GetPattern(CurrentSong()->currentPattern);
            if (pattern != nullptr)
            {
                ImGui::BeginChild(
                    "rowindices",
                    ImVec2(rowIndexColumnWidth, lineHeight * pattern->Length()));
                {
                    auto drawList = ImGui::GetWindowDrawList();

                    for (unsigned int r = 0; r < pattern->Length(); r++)
                    {
                        auto markerPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
                        if (r % 4 == 0)
                        {
                            drawList->AddRectFilled(
                                markerPos,
                                markerPos + ImVec2(rowIndexColumnWidth, lineHeight),
                                ImColor(120, 120, 120, 55));
                        }
                        if (r == _session->currentRow)
                        {
                            drawList->AddRectFilled(
                                markerPos,
                                markerPos + ImVec2(rowIndexColumnWidth, lineHeight),
                                selectedRowBackgroundColor);
                        }
                        ImGui::Text("%02d", r);
                    }
                }
                ImGui::EndChild();
            }
        }
        ImGui::EndChild();

        // HEADERS

        ImGui::SetCursorPosY(contentTop);
        ImGui::MoveCursorPos(ImVec2(rowIndexColumnWidth, 0));
        ImGui::BeginChild(
            "headerscontainer",
            ImVec2(content.x, headerHeight));
        {
            ImGui::SetScrollX(tracksScrollx);
            ImGui::BeginChild(
                "headers",
                ImVec2(fullWidth + ImGui::GetStyle().ScrollbarSize, headerHeight));
            {
                ImGui::Columns(NUM_MIXER_TRACKS);

                auto drawList = ImGui::GetWindowDrawList();
                for (unsigned int i = 0; i < NUM_MIXER_TRACKS; i++)
                {
                    ImGui::PushID(i);
                    auto markerPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();

                    if (i == _session->currentTrack)
                    {
                        drawList->AddLine(
                            markerPos + ImVec2(-4, 0),
                            markerPos + ImVec2(-4, 14),
                            selectionColor,
                            2);
                        drawList->AddLine(
                            markerPos + ImVec2(-4, 0),
                            markerPos + ImVec2(10, 0),
                            selectionColor,
                            3);
                        drawList->AddLine(
                            markerPos + ImVec2(_columnsWidths[i] + 3, 0),
                            markerPos + ImVec2(_columnsWidths[i] + 3, 14),
                            selectionColor,
                            2);
                        drawList->AddLine(
                            markerPos + ImVec2(_columnsWidths[i] + 3, 0),
                            markerPos + ImVec2(_columnsWidths[i] - 11, 0),
                            selectionColor,
                            3);
                    }

                    drawList->AddRectFilled(
                        markerPos + ImVec2(0, 4),
                        markerPos + ImVec2(_columnsWidths[i], 8),
                        ImColor::HSV(i * 0.05f, 0.9f, 0.7f));

                    ImGui::SetColumnWidth(i, _columnsWidths[i] + 15);

                    auto w = ImGui::CalcTextSize("Track 00").x / 2.0f;
                    ImGui::MoveCursorPos(ImVec2((ImGui::GetContentRegionAvailWidth() / 2.0f) - w, 9));
                    ImGui::SetNextWindowSize(ImVec2(_columnsWidths[i] + 15, headerHeight));
                    ImGui::Text("Track %02d", i + 1);

                    bool v = _mixer->GetTrack(i)->Penabled == 1;
                    if (ImGui::Checkbox("##enabled", &v))
                    {
                        _mixer->GetTrack(i)->Penabled = v ? 1 : 0;
                        if (v)
                        {
                            _session->currentTrack = i;
                            _session->currentProperty = 0;
                        }
                    }
                    ImGui::SameLine();
                    ImGui::Button("edit", ImVec2(-1, 0));

                    if (_mixer->GetTrack(i)->Penabled)
                    {
                        ImGui::PlotLines(
                            "##l",
                            _mixer->GetTrack(i)->_bufferl.buf_.get(),
                            (int)_mixer->GetTrack(i)->_bufferl.size(),
                            0,
                            nullptr,
                            -0.5f,
                            0.5f,
                            ImVec2(_columnsWidths[i], 60));
                    }
                    ImGui::PopID();
                    ImGui::NextColumn();
                }
                ImGui::Columns(1);
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

        ImGui::PopFont();
    }
    ImGui::End();
}

bool PatternEditor::HandlePlayingNotes(bool repeat)
{
    for (auto p : _charToNoteMap)
    {
        if (ImGui::IsKeyPressed((ImWchar)p.first, repeat))
        {
            _mixer->PreviewNote(_session->currentTrack, p.second);
            return true;
        }
    }

    return false;
}

char GetByteCharPressed()
{
    char chars[16] = {
        '0',
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        'A',
        'B',
        'C',
        'D',
        'E',
        'F',
    };

    for (int i = 0; i < 16; i++)
    {
        if (ImGui::IsKeyPressed(chars[i]))
        {
            return chars[i];
        }
    }

    return '\0';
}

void PatternEditor::UpdateValue(char pressedChar, unsigned int &inputValue)
{
    char newValue[64];
    std::fill(newValue, newValue + 64, '\0');
    auto value = Note::ValueToString(inputValue);
    for (unsigned int i = 0; i < strlen(value); i++)
    {
        if (i == byteCursor)
        {
            newValue[i] = pressedChar;
            continue;
        }
        newValue[i] = value[i];
    }
    byteCursor++;
    if (byteCursor >= 2)
    {
        byteCursor = 0;
    }

    inputValue = Note::StringToValue(newValue);
}

bool PatternEditor::HandleKeyboardNotes()
{
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Insert)))
    {
        _editMode = !_editMode;
        return false;
    }

    if (!_editMode)
    {
        HandlePlayingNotes();
        return false;
    }

    if (_session->currentProperty == 0)
    {
        auto pattern = CurrentSong()->GetPattern(CurrentSong()->currentPattern);
        if (pattern != nullptr)
        {
            for (auto p : _charToNoteMap)
            {
                if (ImGui::IsKeyPressed((ImWchar)p.first))
                {
                    pattern->Notes(_session->currentTrack)[_session->currentRow]._note = p.second;
                    _mixer->PreviewNote(_session->currentTrack, p.second);
                    MoveCurrentRowDown(true);
                    return true;
                }
            }
        }
    }
    else if (_session->currentProperty == 1)
    {
        auto pattern = CurrentSong()->GetPattern(CurrentSong()->currentPattern);
        if (pattern != nullptr)
        {
            char c = GetByteCharPressed();
            if (c != '\0')
            {
                UpdateValue(c, pattern->Notes(_session->currentTrack)[_session->currentRow]._length);
            }
        }
    }
    else if (_session->currentProperty == 2)
    {
        auto pattern = CurrentSong()->GetPattern(CurrentSong()->currentPattern);
        if (pattern != nullptr)
        {
            char c = GetByteCharPressed();
            if (c != '\0')
            {
                UpdateValue(c, pattern->Notes(_session->currentTrack)[_session->currentRow]._velocity);
            }
        }
    }

    return false;
}

bool PatternEditor::HandleKeyboardNavigation()
{
    bool result = false;

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
    {
        MoveCurrentRowUp(ImGui::GetIO().KeyCtrl);

        result = true;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
    {
        MoveCurrentRowDown(ImGui::GetIO().KeyCtrl);

        result = true;
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
    {
        MoveToNextProperty();

        result = true;
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
    {
        MoveToPreviousProperty();

        result = true;
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab)))
    {
        ChangeCurrentTrack(ImGui::GetIO().KeyShift);

        result = true;
    }

    if (result)
    {
        byteCursor = 0;
    }

    return result;
}

void PatternEditor::MoveCurrentRowUp(bool largeStep)
{
    auto pattern = CurrentSong()->GetPattern(CurrentSong()->currentPattern);

    if (pattern == nullptr)
    {
        return;
    }

    unsigned int step = largeStep ? 4 : 1;
    if (_session->currentRow >= step)
    {
        _session->currentRow -= step;
    }
    else
    {
        _session->currentRow = pattern->Length() - 1;
    }
}

void PatternEditor::MoveCurrentRowDown(bool largeStep)
{
    auto pattern = CurrentSong()->GetPattern(CurrentSong()->currentPattern);

    if (pattern == nullptr)
    {
        return;
    }

    unsigned int step = largeStep ? _session->skipRowStepSize : 1;

    _session->currentRow += step;

    if (_session->currentRow >= pattern->Length())
    {
        _session->currentRow = 0;
    }
}

void PatternEditor::ChangeCurrentTrack(bool moveLeft)
{
    if (moveLeft)
    {
        if (_session->currentTrack > 0)
        {
            _session->currentTrack--;
        }
        else
        {
            _session->currentTrack = NUM_MIXER_TRACKS - 1;
        }
    }
    else
    {
        _session->currentTrack++;
        if (_session->currentTrack >= NUM_MIXER_TRACKS)
        {
            _session->currentTrack = 0;
        }
    }
}

void PatternEditor::MoveToPreviousProperty()
{
    if (_session->currentProperty == 0)
    {
        _session->currentProperty = 4;
        if (_session->currentTrack > 0)
        {
            _session->currentTrack--;
        }
        else
        {
            _session->currentTrack = NUM_MIXER_TRACKS - 1;
        }
    }
    else
    {
        _session->currentProperty--;
    }
}

void PatternEditor::MoveToNextProperty()
{
    _session->currentProperty++;
    if (_session->currentProperty >= 4)
    {
        _session->currentProperty = 0;
        _session->currentTrack++;
        if (_session->currentTrack >= NUM_MIXER_TRACKS)
        {
            _session->currentTrack = 0;
        }
    }
}

bool PatternEditor::IsRecording() const
{
    return _editMode;
}

void PatternEditor::ToggleRecording()
{
    _editMode = !_editMode;
}
