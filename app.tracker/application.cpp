#include <application.h>
#include <imgui.h>
#include <iostream>
#include <zyn.common/Config.h>
#include <zyn.common/IAudioGenerator.h>
#include <zyn.common/IMidiEventHandler.h>
#include <zyn.common/globals.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.nio/MidiInputManager.h>
#include <zyn.nio/Nio.h>

ImVec2 operator+(ImVec2 const &a, ImVec2 const &b)
{
    return ImVec2(a.x + b.x, a.y + b.y);
}

ImVec2 operator-(ImVec2 const &a, ImVec2 const &b)
{
    return ImVec2(a.x - b.x, a.y - b.y);
}

class Application :
    public IApplication
{
    Mixer *_mixer;

    ImFont *_monofont;

public:
    Application()
        : _mixer(nullptr),
          _monofont(nullptr)
    {}

    virtual bool Setup()
    {
        ImGuiIO &io = ImGui::GetIO();
        io.Fonts->Clear();
        ImFont *font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
        if (font != nullptr)
        {
            io.FontDefault = font;
        }
        else
        {
            io.Fonts->AddFontDefault();
        }
        _monofont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\SourceCodePro-Bold.ttf", 12.0f);
        io.Fonts->Build();

        Config::init();

        SystemSettings::Instance().samplerate = Config::Current().cfg.SampleRate;
        SystemSettings::Instance().buffersize = Config::Current().cfg.SoundBufferSize;
        SystemSettings::Instance().oscilsize = Config::Current().cfg.OscilSize;
        SystemSettings::Instance().alias();

        _mixer = new Mixer();

        _mixer->Init();

        Nio::preferedSampleRate(SystemSettings::Instance().samplerate);

        if (!Nio::Start(_mixer, _mixer))
        {
            return false;
        }

        Nio::SelectSink("PA");
        Nio::SelectSource("RT");

        return true;
    }

    virtual void Render3d()
    {}

    const char *emptyCellNote = "---";
    const char *emptyCellParameter = "..";
    const char *emptyCellFx = "000";

    const int numTracks = 16;
    const int numRows = 64;
    int currentRow = 0;
    int currentTrack = 0;
    int currentProperty = 0;

    virtual void Render2d()
    {
        //show Main Window
        ImGui::ShowDemoWindow();

        ImGui::Begin("TracksContainer", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        auto content = ImGui::GetContentRegionAvail();

        auto contentTop = ImGui::GetCursorPosY();
        ImGui::SetCursorPosY(100);
        ImGui::BeginChild(
            "container", ImVec2(content.x, -40));

        ImGui::PushFont(_monofont);
        auto spaceWidth = ImGui::CalcTextSize(" ");
        auto cellNoteWidth = ImGui::CalcTextSize(emptyCellNote);
        auto cellParameterWidth = ImGui::CalcTextSize(emptyCellParameter);
        auto cellFxWidth = ImGui::CalcTextSize(emptyCellFx);
        auto columnWidth = cellNoteWidth +
                           spaceWidth + cellParameterWidth +
                           spaceWidth + cellParameterWidth +
                           spaceWidth + cellFxWidth;

        auto fullWidth = 30 + (columnWidth.x + 15) * numTracks;
        auto lineHeight = ImGui::GetTextLineHeightWithSpacing();

        ImGui::BeginChild(
            "tracks",
            ImVec2(fullWidth, lineHeight * numRows));
        auto tracksPos = ImGui::GetWindowContentRegionMin();
        auto tracksMax = ImGui::GetWindowContentRegionMax();

        auto selectionRowMin = ImVec2(tracksPos.x, tracksPos.y - 2 + currentRow * lineHeight);
        auto selectionRowMax = ImVec2(tracksMax.x, tracksPos.y + (currentRow + 1) * lineHeight);

        auto drawList = ImGui::GetWindowDrawList();

        drawList->AddRectFilled(
            ImGui::GetWindowPos() + selectionRowMin,
            ImGui::GetWindowPos() + selectionRowMax,
            ImColor(20, 220, 20, 55));

        ImGui::Columns(numTracks + 1);
        ImGui::SetColumnWidth(0, 30);
        for (int i = 0; i < numTracks; i++)
        {
            ImGui::SetColumnWidth(i + 1, columnWidth.x + 15);
        }

        for (int r = 0; r < 64; r++)
        {
            ImGui::Text("%02d", r);
            ImGui::NextColumn();

            for (int i = 0; i < numTracks; i++)
            {
                if (i == currentTrack && r == currentRow)
                {
                    auto cursorWidth = cellNoteWidth.x;
                    auto min = ImGui::GetWindowPos() + ImGui::GetCursorPos() - ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
                    if (currentProperty > 0)
                    {
                        min.x += cellNoteWidth.x + spaceWidth.x;
                        cursorWidth = cellParameterWidth.x;
                    }
                    if (currentProperty > 1)
                    {
                        min.x += cellParameterWidth.x + spaceWidth.x;
                        cursorWidth = cellParameterWidth.x;
                    }
                    if (currentProperty > 2)
                    {
                        min.x += cellParameterWidth.x + spaceWidth.x;
                        cursorWidth = cellFxWidth.x;
                    }
                    drawList->AddRectFilled(
                        min - ImVec2(4, 0),
                        min + ImVec2(cursorWidth, lineHeight),
                        ImColor(20, 20, 220, 255));
                }

                ImGui::Text("%s %s %s %s", emptyCellNote, emptyCellParameter, emptyCellParameter, emptyCellFx);
                ImGui::NextColumn();
            }
        }

        ImGui::EndChild();

        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
        {
            currentRow--;
            if (currentRow < 0) currentRow = numRows - 1;

            ImGui::SetScrollY((currentRow - (content.y / lineHeight) / 2 + 1) * lineHeight);
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
        {
            currentRow++;
            if (currentRow >= numRows) currentRow = 0;

            ImGui::SetScrollY((currentRow - (content.y / lineHeight) / 2 + 1) * lineHeight);
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
        {
            currentProperty++;
            if (currentProperty >= 4)
            {
                currentProperty = 0;
                currentTrack++;
                if (currentTrack >= numTracks) currentTrack = 0;
            }

            ImGui::SetScrollX((currentTrack - (content.x / columnWidth.x) / 2 + 1) * columnWidth.x);
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
        {
            currentProperty--;
            if (currentProperty < 0)
            {
                currentProperty = 4;
                currentTrack--;
                if (currentTrack < 0) currentTrack = numTracks - 1;
            }

            ImGui::SetScrollX((currentTrack - (content.x / columnWidth.x) / 2 + 1) * columnWidth.x);
        }

        auto tracksScrollx = ImGui::GetScrollX();

        ImGui::PopFont();

        ImGui::EndChild();

        // FOOTERS
        ImGui::BeginChild(
            "footerscontainer", ImVec2(0, 20));
        ImGui::SetScrollX(tracksScrollx);

        ImGui::BeginChild(
            "footers",
            ImVec2(fullWidth + ImGui::GetStyle().ScrollbarSize, 40));

        ImGui::Columns(numTracks + 1);
        ImGui::SetColumnWidth(0, 30);
        ImGui::NextColumn();
        for (int i = 0; i < numTracks; i++)
        {
            ImGui::SetColumnWidth(i + 1, columnWidth.x + 15);
            ImGui::Text("footer %d", i + 1);
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::EndChild();

        ImGui::EndChild();

        // SCROLLBAR

        ImGui::BeginChild(
            "scrollbar", ImVec2(0, 20),
            false,
            ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::SetScrollX(tracksScrollx);

        ImGui::BeginChild(
            "scrollbarcontent ", ImVec2(fullWidth + ImGui::GetStyle().ScrollbarSize, 20));
        ImGui::EndChild();
        ImGui::EndChild();

        // HEADERS

        ImGui::SetCursorPosY(contentTop);
        ImGui::BeginChild(
            "headerscontainer", ImVec2(0, -40));

        ImGui::SetScrollX(tracksScrollx);
        ImGui::BeginChild(
            "headers",
            ImVec2(fullWidth + ImGui::GetStyle().ScrollbarSize, 100));

        ImGui::Columns(numTracks + 1);
        ImGui::SetColumnWidth(0, 30);
        ImGui::NextColumn();
        for (int i = 0; i < numTracks; i++)
        {
            ImGui::SetColumnWidth(i + 1, columnWidth.x + 15);
            ImGui::Text("Track %d", i + 1);
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::EndChild();
        ImGui::EndChild();

        ImGui::End();
    }

    virtual void Cleanup()
    {
        //ensure that everything has stopped with the mutex wait
        _mixer->Lock();
        _mixer->Unlock();

        Nio::Stop();

        delete _mixer;
        _mixer = nullptr;
    }
};

extern IApplication *CreateApplication();

IApplication *CreateApplication()
{
    return new Application();
}
