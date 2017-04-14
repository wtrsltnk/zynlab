#ifndef NIOUI_H
#define NIOUI_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>

class NioUI
{
    public:
        NioUI();
        ~NioUI();
        void refresh(class MasterUI* mui);
    private:
        static void midiCallback_s(Fl_Widget *c, void* ptr);
        static void audioCallback_s(Fl_Widget *c, void* ptr);
};

#endif
