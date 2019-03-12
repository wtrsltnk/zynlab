#ifndef UI_OSCILGEN_H
#define UI_OSCILGEN_H

#include "../appstate.h"
#include "ui.envelope.h"
#include "ui.lfo.h"
#include <zyn.mixer/Track.h>
#include <zyn.synth/PADnoteParams.h>

namespace zyn {
namespace ui {

class OscilGen
{
private:
    AppState *_state;

public:
    OscilGen(AppState *state);
    virtual ~OscilGen();

    bool Setup();
    void Render();
};

} // namespace ui
} // namespace zyn

#endif // UI_OSCILGEN_H
