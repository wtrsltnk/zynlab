#ifndef UI_SAMPLENOTE_H
#define UI_SAMPLENOTE_H

#include "../appstate.h"

namespace zyn {
namespace ui {

class SampleNote
{
private:
    AppState *_state;

public:
    SampleNote(AppState *state);
    virtual ~SampleNote();

    bool Setup();
    void Render();
};

} // namespace ui
} // namespace zyn

#endif // UI_SAMPLENOTE_H
