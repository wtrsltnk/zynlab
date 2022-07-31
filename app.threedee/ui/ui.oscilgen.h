#ifndef UI_OSCILGEN_H
#define UI_OSCILGEN_H

#include "../appstate.h"

extern std::vector<std::string> WaveShapingFunctions;
extern std::vector<std::string> OscilFilterTypes;
extern std::vector<std::string> ModulationTypes;
extern std::vector<std::string> SpectrumAdjustmentTypes;
extern std::vector<std::string> AddaptiveHarmonicsTypes;

namespace zyn
{
    namespace ui
    {

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
