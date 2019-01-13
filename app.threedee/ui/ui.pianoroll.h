#ifndef UI_PIANOROLL_H
#define UI_PIANOROLL_H

#include "../appstate.h"

namespace zyn {
namespace ui {

class PianoRoll
{
private:
    AppState *_state;

    void ImGuiPianoRollSequencer(int trackIndex, float trackHeight);
    void ImGuiPianoRollPatternEditorWindow();

public:
    PianoRoll(AppState *state);
    virtual ~PianoRoll();

    bool Setup();
    void Render(int trackIndex, int trackHeight);
    void EventHandling();
};

} // namespace ui
} // namespace zyn

#endif // UI_PIANOROLL_H
