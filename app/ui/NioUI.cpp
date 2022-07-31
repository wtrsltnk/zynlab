#include "NioUI.h"
#include "../../zyn.nio/Nio.h"
#include <MasterUI.h>

#include <string>

using namespace std;

NioUI::NioUI() = default;

NioUI::~NioUI() = default;

void NioUI::refresh(
    MasterUI *mui)
{
    //initialize audio list
    {
        std::vector<string> audioList = Nio::GetSinks();
        for (const auto &audio : audioList)
        {
            mui->mastermenu->add((std::string("NIO/Audio - ") + audio).c_str(), audio.c_str(), NioUI::audioCallback_s);
        }
    }

    //initialize midi list
    {
        std::vector<string> midiList = Nio::GetSources();
        for (const auto &midi : midiList)
        {
            mui->mastermenu->add((std::string("NIO/Midi - ") + midi).c_str(), midi.c_str(), NioUI::midiCallback_s);
        }
    }
}

void NioUI::midiCallback_s(
    Fl_Widget *c,
    void * /*ptr*/)
{
    auto s = std::string(static_cast<Fl_Choice *>(c)->text()).substr(7);
    Nio::SelectSource(s);
}

void NioUI::audioCallback_s(
    Fl_Widget *c,
    void * /*ptr*/)
{
    auto s = std::string(static_cast<Fl_Choice *>(c)->text()).substr(8);
    Nio::SelectSink(s);
}
