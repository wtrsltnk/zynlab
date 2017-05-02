#include "NioUI.h"
#include "../../zyn.nio/Nio.h"
#include <MasterUI.h>

#include <string>

using namespace std;

NioUI::NioUI() { }

NioUI::~NioUI() {}

void NioUI::refresh(MasterUI* mui)
{
    //initialize audio list
    {
        set<string> audioList = Nio::getSinks();
        for (auto audio : audioList)
            mui->mastermenu->add((std::string("NIO/Audio - ") + audio).c_str(), audio.c_str(), NioUI::audioCallback_s);
    }

    //initialize midi list
    {
        set<string> midiList = Nio::getSources();
        for(auto midi : midiList)
            mui->mastermenu->add((std::string("NIO/Midi - ") + midi).c_str(), midi.c_str(), NioUI::midiCallback_s);
    }
}

void NioUI::midiCallback_s(Fl_Widget *c, void* ptr)
{
    auto s = std::string(static_cast<Fl_Choice *>(c)->text()).substr(7);
    Nio::setSource(s);
}

void NioUI::audioCallback_s(Fl_Widget *c, void* ptr)
{
    auto s = std::string(static_cast<Fl_Choice *>(c)->text()).substr(8);
    Nio::setSink(s);
}
