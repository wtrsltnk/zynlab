#ifndef UI_STEPPATTERN_H
#define UI_STEPPATTERN_H

#include "../appstate.h"

namespace zyn {
namespace ui {

class StepPattern
{
private:
    AppState *_state;

    void ImGuiStepSequencer(int trackIndex, float trackHeight);

public:
    StepPattern(AppState *state);
    virtual ~StepPattern();

    bool Setup();
    void Render(int trackIndex, int trackHeight);
    void RenderStepPatternEditorWindow();
    void EventHandling();
};

} // namespace ui
} // namespace zyn

#endif // UI_STEPPATTERN_H
