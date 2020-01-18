#ifndef UI_SAMPLENOTE_H
#define UI_SAMPLENOTE_H

#include "../appstate.h"
#include "ui.library.h"

namespace zyn {
namespace ui {

class SampleNote
{
private:
    AppState *_state;
    zyn::ui::Library _library;

public:
    SampleNote(AppState *state);
    virtual ~SampleNote();

    bool Setup();
    void Render();

    static std::string NoteToString(unsigned char note);
};

} // namespace ui
} // namespace zyn

#endif // UI_SAMPLENOTE_H
