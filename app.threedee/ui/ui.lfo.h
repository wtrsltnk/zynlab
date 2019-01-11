#ifndef UI_LFO_H
#define UI_LFO_H

#include <zyn.synth/LFOParams.h>

namespace zyn {
namespace ui {

class Lfo
{
private:
    char const *_label;

public:
    Lfo(char const *label);

    void Render(LFOParams *params);
};

} // namespace ui
} // namespace zyn

#endif // UI_LFO_H
