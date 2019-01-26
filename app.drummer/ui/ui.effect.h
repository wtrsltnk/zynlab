#ifndef UI_EFFECT_H
#define UI_EFFECT_H

#include "../appstate.h"
#include "ui.filter.h"
#include <zyn.fx/EffectMgr.h>

namespace zyn {
namespace ui {

class Effect
{
private:
    AppState *_state;
    Filter _Filter;

    // Effect
    void EffectEditor(EffectManager *effectManager);
    void EffectReverbEditor(EffectManager *effectManager);
    void EffectEchoEditor(EffectManager *effectManager);
    void EffectChorusEditor(EffectManager *effectManager);
    void EffectPhaserEditor(EffectManager *effectManager);
    void EffectAlienWahEditor(EffectManager *effectManager);
    void EffectDistortionEditor(EffectManager *effectManager);
    void EffectEQEditor(EffectManager *effectManager);
    void EffectDynFilterEditor(EffectManager *effectManager);

public:
    Effect(AppState *state);

    bool Setup();
    void Render();
};

} // namespace ui
} // namespace zyn

#endif // UI_EFFECT_H
