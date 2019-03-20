#ifndef UI_OSCILGEN_H
#define UI_OSCILGEN_H

#include "../appstate.h"

extern char const *const WaveShapingFunctions[];
extern unsigned int WaveShapingFunctionCount;
extern char const *const OscilFilterTypes[];
extern unsigned int OscilFilterTypeCount;
extern char const *const ModulationTypes[];
extern unsigned int ModulationTypeCount;
extern char const *const SpectrumAdjustmentTypes[];
extern unsigned int SpectrumAdjustmentTypeCount;
extern char const *const AddaptiveHarmonicsTypes[];
extern unsigned int AddaptiveHarmonicsTypeCount;

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
