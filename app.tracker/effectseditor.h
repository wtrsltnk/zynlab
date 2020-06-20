#ifndef EFFECTSEDITOR_H
#define EFFECTSEDITOR_H

#include <zyn.common/globals.h>
#include "applicationsession.h"

class EffectsEditor
{
    ApplicationSession *_session;
    IMixer *_mixer;
public:
    EffectsEditor();

    void SetUp(ApplicationSession *session,IMixer *mixer);
    void Render2d();
};

#endif // EFFECTSEDITOR_H
