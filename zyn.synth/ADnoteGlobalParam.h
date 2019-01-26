#ifndef ADNOTEGLOBALPARAM_H
#define ADNOTEGLOBALPARAM_H

#include "EnvelopeParams.h"
#include "LFOParams.h"
#include "Resonance.h"
#include <zyn.dsp/FilterParams.h>

/*****************************************************************/
/*                    GLOBAL PARAMETERS                          */
/*****************************************************************/

class ADnoteGlobalParam : public WrappedPresets
{
public:
    ADnoteGlobalParam();
    ~ADnoteGlobalParam();

    void InitPresets();

    void Serialize(IPresetsSerializer *xml);
    void Deserialize(IPresetsSerializer *xml);
    void Defaults();

};

#endif // ADNOTEGLOBALPARAM_H
