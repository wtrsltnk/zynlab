#ifndef UI_SEQUENCER_H
#define UI_SEQUENCER_H

#include "../appstate.h"
#include "ui.steppattern.h"

namespace zyn {
namespace ui {

class Sequencer
{
private:
    AppState *_state;
    StepPattern _stepPatternUi;

    void ImGuiSequencer();

public:
    Sequencer(AppState *state);
    virtual ~Sequencer();

    bool Setup();
    void Render();
};

} // namespace ui
} // namespace zyn

#endif // UI_SEQUENCER_H
