#include "patterneditor.h"

#include <imgui.h>
#include <iostream>
#include <zyn.mixer/Track.h>
#include <zyn.ui/include/imgui_helpers.h>
#include <zyn.ui/syntheditor.h>

namespace ImGui
{
    void ShowTooltipOnHover(
        char const *tooltip);

    void MoveCursorPos(
        ImVec2 delta);
} // namespace ImGui

PatternEditor::PatternEditor()
{
    for (int i = 0; i < NUM_MIXER_TRACKS; i++)
    {
        _columnsWidths[i] = 0;
    }
}

void PatternEditor::SetUp(
    ApplicationSession *session,
    ImFont *font)
{
    _session = session;
    _monofont = font;
}

char const *PatternEditor::ID = "PatternEditor";

void CurrentPropertyName(int currentProperty)
{
    if (currentProperty == 0)
        ImGui::Text("Note to play");
    else if (currentProperty == 1)
        ImGui::Text("Length of note");
    else if (currentProperty == 2)
        ImGui::Text("Velocity of note");
    else if (currentProperty == 3)
        ImGui::Text("FX value");
}

void PatternEditor::Render2d()
{
    auto selectionColorEditmode = ImColor(180, 20, 20, 255);
    auto selectionColor = ImColor(20, 180, 20, 255);
    auto selectedRowBackgroundColorEditmode = ImColor(220, 20, 20, 55);
    auto selectedRowBackgroundColor = ImColor(70, 120, 70, 70);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::Begin(
        PatternEditor::ID,
        nullptr,
        flags);
    {
        ImGui::PushFont(_monofont);
        auto content = ImGui::GetContentRegionAvail() - ImVec2(float(rowIndexColumnWidth), 0);
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

        auto contentTop = ImGui::GetCursorPos();

        ImGui::MoveCursorPos(
            ImVec2(
                float(rowIndexColumnWidth) + ((columnWidth.x + 15.0f) * _session->_mixer->State.currentTrack) - tracksScrollx,
                float(headerHeight) - ImGui::GetTextLineHeightWithSpacing()));
        CurrentPropertyName(_session->currentProperty);

        // CELLS

        ImGui::SetCursorPos(contentTop);
        ImGui::MoveCursorPos(ImVec2(float(rowIndexColumnWidth), float(headerHeight)));
        ImGui::BeginChild(
            "container",
            ImVec2(content.x, -float(footerHeight + scrollbarHeight)));
        {
            ImGui::SetScrollX(tracksScrollx);

            lineHeight = ImGui::GetTextLineHeightWithSpacing();

            auto pattern = _session->_song->GetPattern(_session->_song->currentPattern);

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

                    auto color = _session->IsRecording() ? selectedRowBackgroundColorEditmode : selectedRowBackgroundColor;
                    drawList->AddRectFilled(
                        ImGui::GetWindowPos() + selectionRowMin,
                        ImGui::GetWindowPos() + selectionRowMax,
                        color);

                    // MAKE ROOM FOR THE HEADERS

                    ImGui::Columns(NUM_MIXER_TRACKS);
                    for (unsigned int i = 0; i < NUM_MIXER_TRACKS; i++)
                    {
                        _columnsWidths[i] = int(columnWidth.x);
                        ImGui::SetColumnWidth(i, _columnsWidths[i] + 15.0f);
                    }

                    // ALL TRACKS AND CELLS

                    for (unsigned int r = 0; r < pattern->Length(); r++)
                    {
                        for (unsigned int i = 0; i < NUM_MIXER_TRACKS; i++)
                        {
                            auto notes = pattern->Notes(i);
                            auto markerPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
                            if (i == _session->_mixer->State.currentTrack && r == _session->currentRow)
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
                                    _session->IsRecording() ? selectionColorEditmode : selectionColor);

                                auto resetToCursor = ImGui::GetCursorPos();

                                ImGui::SetCursorScreenPos(min - ImVec2(4, 0));
                                ImGui::InvisibleButton("##selection tooltip", ImVec2(cursorWidth, lineHeight));
                                if (ImGui::IsItemHovered())
                                {
                                    ImGui::BeginTooltip();
                                    CurrentPropertyName(_session->currentProperty);
                                    ImGui::EndTooltip();
                                }

                                ImGui::SetCursorPos(resetToCursor);
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
                if (HandleKeyboard())
                {
                    ImGui::SetScrollY((_session->currentRow - (content.y / lineHeight) / 2 + 1) * lineHeight);
                }
                else if (HandleKeyboardNavigation())
                {
                    ImGui::SetScrollY((_session->currentRow - (content.y / lineHeight) / 2 + 1) * lineHeight);
                    auto scrollx = (_session->_mixer->State.currentTrack - (content.x / columnWidth.x) / 2 + 1) * columnWidth.x;
                    tracksScrollx = scrollx > 0 ? scrollx : 0;
                    ImGui::SetScrollX(tracksScrollx);
                }
            }

            if (keepRowInFocus)
            {
                ImGui::SetScrollY((_session->currentRow - (content.y / lineHeight) / 2 + 1) * lineHeight);
                auto scrollx = (_session->_mixer->State.currentTrack - (content.x / columnWidth.x) / 2 + 1) * columnWidth.x;
                ImGui::SetScrollX(scrollx);
                keepRowInFocus = false;
            }

            tracksScrolly = ImGui::GetScrollY();
        }
        ImGui::EndChild();

        // FOOTERS

        ImGui::SetCursorPos(contentTop);
        ImGui::MoveCursorPos(ImVec2(float(rowIndexColumnWidth), content.y - float(footerHeight + scrollbarHeight)));
        ImGui::BeginChild(
            "footerscontainer",
            ImVec2(content.x, float(footerHeight)));
        {
            ImGui::SetScrollX(tracksScrollx);

            ImGui::BeginChild(
                "footers",
                ImVec2(fullWidth + ImGui::GetStyle().ScrollbarSize, float(footerHeight)));
            {
                ImGui::Columns(NUM_MIXER_TRACKS);
                for (unsigned int i = 0; i < NUM_MIXER_TRACKS; i++)
                {
                    ImGui::PushID(i);
                    auto drawList = ImGui::GetWindowDrawList();
                    auto markerPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
                    if (i == _session->_mixer->State.currentTrack)
                    {
                        drawList->AddLine(
                            markerPos + ImVec2(-3.0f, 5.0f),
                            markerPos + ImVec2(-3.0f, footerHeight - 2.0f),
                            selectionColor,
                            3);
                        drawList->AddLine(
                            markerPos + ImVec2(-4.0f, footerHeight - 2.0f),
                            markerPos + ImVec2(11.0f, footerHeight - 2.0f),
                            selectionColor,
                            3);
                        drawList->AddLine(
                            markerPos + ImVec2(_columnsWidths[i] + 1.0f, 5.0f),
                            markerPos + ImVec2(_columnsWidths[i] + 1.0f, footerHeight - 2.0f),
                            selectionColor,
                            3);
                        drawList->AddLine(
                            markerPos + ImVec2(_columnsWidths[i] + 3.0f, footerHeight - 2.0f),
                            markerPos + ImVec2(_columnsWidths[i] - 12.0f, footerHeight - 2.0f),
                            selectionColor,
                            3);
                    }

                    auto w = ImGui::CalcTextSize("footer 00").x;
                    ImGui::SetColumnWidth(i, _columnsWidths[i] + 15.0f);
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

        ImGui::SetCursorPos(contentTop);
        ImGui::MoveCursorPos(ImVec2(float(rowIndexColumnWidth), content.y - float(scrollbarHeight)));
        ImGui::BeginChild(
            "scrollbar",
            ImVec2(content.x, float(scrollbarHeight)),
            false,
            ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImGui::SetScrollX(tracksScrollx);

            ImGui::BeginChild(
                "scrollbarcontent",
                ImVec2(fullWidth + ImGui::GetStyle().ScrollbarSize, float(scrollbarHeight)));
            {
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

        // ROWINDICES

        ImGui::SetCursorPos(contentTop);
        ImGui::MoveCursorPos(ImVec2(0.0f, float(headerHeight)));
        ImGui::BeginChild(
            "rowinedicescontainer",
            ImVec2(float(rowIndexColumnWidth), -float(footerHeight + scrollbarHeight)),
            false,
            ImGuiWindowFlags_NoScrollbar);
        {
            ImGui::SetScrollY(tracksScrolly);

            auto pattern = _session->_song->GetPattern(_session->_song->currentPattern);
            if (pattern != nullptr)
            {
                ImGui::BeginChild(
                    "rowindices",
                    ImVec2(float(rowIndexColumnWidth), lineHeight * pattern->Length()));
                {
                    auto drawList = ImGui::GetWindowDrawList();

                    for (unsigned int r = 0; r < pattern->Length(); r++)
                    {
                        auto markerPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
                        if (r % 4 == 0)
                        {
                            drawList->AddRectFilled(
                                markerPos,
                                markerPos + ImVec2(float(rowIndexColumnWidth), lineHeight),
                                ImColor(120, 120, 120, 55));
                        }
                        if (r == _session->currentRow)
                        {
                            drawList->AddRectFilled(
                                markerPos,
                                markerPos + ImVec2(float(rowIndexColumnWidth), lineHeight),
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

        ImGui::SetCursorPos(contentTop);
        ImGui::MoveCursorPos(ImVec2(float(rowIndexColumnWidth), 0));
        ImGui::BeginChild(
            "headerscontainer",
            ImVec2(content.x, float(headerHeight)));
        {
            ImGui::SetScrollX(tracksScrollx);
            ImGui::BeginChild(
                "headers",
                ImVec2(fullWidth + ImGui::GetStyle().ScrollbarSize, float(headerHeight)));
            {
                ImGui::Columns(NUM_MIXER_TRACKS);

                auto drawList = ImGui::GetWindowDrawList();
                for (unsigned int i = 0; i < NUM_MIXER_TRACKS; i++)
                {
                    ImGui::PushID(i);
                    auto markerPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();

                    if (i == _session->_mixer->State.currentTrack)
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
                            markerPos + ImVec2(_columnsWidths[i] + 3.0f, 0.0f),
                            markerPos + ImVec2(_columnsWidths[i] + 3.0f, 14.0f),
                            selectionColor,
                            2);
                        drawList->AddLine(
                            markerPos + ImVec2(_columnsWidths[i] + 3.0f, 0.0f),
                            markerPos + ImVec2(_columnsWidths[i] - 11.0f, 0.0f),
                            selectionColor,
                            3);
                    }

                    drawList->AddRectFilled(
                        markerPos + ImVec2(0, 4),
                        markerPos + ImVec2(float(_columnsWidths[i]), 8.0f),
                        ImColor::HSV(i * 0.05f, 0.9f, 0.7f));

                    ImGui::SetColumnWidth(i, _columnsWidths[i] + 15.0f);

                    auto w = ImGui::CalcTextSize("Track 00").x / 2.0f;
                    ImGui::MoveCursorPos(ImVec2((ImGui::GetContentRegionAvailWidth() / 2.0f) - w, 9.0f));
                    ImGui::SetNextWindowSize(ImVec2(_columnsWidths[i] + 15.0f, float(headerHeight)));
                    ImGui::Text("Track %02d", i + 1);

                    bool v = _session->_mixer->GetTrack(i)->Penabled == 1;
                    if (ImGui::Checkbox("##enabled", &v))
                    {
                        _session->_mixer->GetTrack(i)->Penabled = v ? 1 : 0;
                        if (v)
                        {
                            _session->_mixer->State.currentTrack = i;
                            _session->currentProperty = 0;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("synth", ImVec2(-1, 0)))
                    {
                        ImGui::SetWindowFocus(SynthEditor::ID);
                        _session->_mixer->State.currentTrack = i;
                        _session->selectedTab = SelectableTabs::Synth;
                    }

                    if (_session->_mixer->GetTrack(i)->Penabled)
                    {
                        ImGui::PlotLines(
                            "##l",
                            _session->_mixer->GetTrack(i)->_bufferl.buf_.get(),
                            (int)_session->_mixer->GetTrack(i)->_bufferl.size(),
                            0,
                            nullptr,
                            -0.5f,
                            0.5f,
                            ImVec2(float(_columnsWidths[i]), 40.0f));
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

bool PatternEditor::HandleKeyboard()
{
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Insert)))
    {
        _session->ToggleRecording();

        return false;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
    {
        _session->currentRow = 0;

        return false;
    }

    if (_session->IsRecording())
    {
        return HandleEditingNotes();
    }

    return HandlePlayingNotes();
}

void PatternEditor::UpdateValue(
    char pressedChar,
    unsigned int &inputValue)
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

bool PatternEditor::HandleEditingNotes()
{
    static Note noteInMemory;

    auto pattern = _session->_song->GetPattern(_session->_song->currentPattern);
    if (pattern == nullptr)
    {
        return false;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)) && ImGui::GetIO().KeyCtrl)
    {
        // Copy note
        noteInMemory = pattern->Notes(_session->_mixer->State.currentTrack)[_session->currentRow];

        return true;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_X)) && ImGui::GetIO().KeyCtrl)
    {
        // Cut note
        noteInMemory = pattern->Notes(_session->_mixer->State.currentTrack)[_session->currentRow];
        pattern->Notes(_session->_mixer->State.currentTrack)[_session->currentRow].Clear();

        return true;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)) && ImGui::GetIO().KeyCtrl)
    {
        // Paste note
        pattern->Notes(_session->_mixer->State.currentTrack)[_session->currentRow] = noteInMemory;

        return true;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
    {
        pattern->Notes(_session->_mixer->State.currentTrack)[_session->currentRow].Clear();

        return true;
    }

    if (_session->currentProperty == 0)
    {
        for (auto p : _charToNoteMap)
        {
            if (ImGui::IsKeyPressed((ImWchar)p.first))
            {
                pattern->Notes(_session->_mixer->State.currentTrack)[_session->currentRow].Set(p.second, 64, 100);
                _session->_mixer->PreviewNote(_session->_mixer->State.currentTrack, p.second, 400);
                MoveCurrentRowDown(true);
                return true;
            }
        }
    }
    else if (_session->currentProperty == 1)
    {
        char c = GetByteCharPressed();
        if (c != '\0')
        {
            UpdateValue(c, pattern->Notes(_session->_mixer->State.currentTrack)[_session->currentRow]._length);
        }
    }
    else if (_session->currentProperty == 2)
    {
        char c = GetByteCharPressed();
        if (c != '\0')
        {
            UpdateValue(c, pattern->Notes(_session->_mixer->State.currentTrack)[_session->currentRow]._velocity);
        }
    }

    return false;
}

bool PatternEditor::HandlePlayingNotes(
    bool repeat)
{
    for (auto p : _charToNoteMap)
    {
        if (ImGui::IsKeyPressed((ImWchar)p.first, repeat))
        {
            _session->_mixer->PreviewNote(_session->_mixer->State.currentTrack, p.second, 400);
            return true;
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

void PatternEditor::MoveCurrentRowUp(
    bool largeStep)
{
    auto pattern = _session->_song->GetPattern(_session->_song->currentPattern);

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

void PatternEditor::MoveCurrentRowDown(
    bool largeStep)
{
    auto pattern = _session->_song->GetPattern(_session->_song->currentPattern);

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

void PatternEditor::ChangeCurrentTrack(
    bool moveLeft)
{
    if (moveLeft)
    {
        if (_session->_mixer->State.currentTrack > 0)
        {
            _session->_mixer->State.currentTrack--;
        }
        else
        {
            _session->_mixer->State.currentTrack = NUM_MIXER_TRACKS - 1;
        }
    }
    else
    {
        _session->_mixer->State.currentTrack++;
        if (_session->_mixer->State.currentTrack >= NUM_MIXER_TRACKS)
        {
            _session->_mixer->State.currentTrack = 0;
        }
    }
}

const int notePropertyCount = 4;

void PatternEditor::MoveToPreviousProperty()
{
    if (_session->currentProperty == 0)
    {
        _session->currentProperty = (notePropertyCount - 1);
        if (_session->_mixer->State.currentTrack > 0)
        {
            _session->_mixer->State.currentTrack--;
        }
        else
        {
            _session->_mixer->State.currentTrack = NUM_MIXER_TRACKS - 1;
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
    if (_session->currentProperty > (notePropertyCount - 1))
    {
        _session->currentProperty = 0;
        _session->_mixer->State.currentTrack++;
        if (_session->_mixer->State.currentTrack >= NUM_MIXER_TRACKS)
        {
            _session->_mixer->State.currentTrack = 0;
        }
    }
}
