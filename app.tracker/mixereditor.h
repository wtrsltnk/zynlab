#ifndef MIXEREDITOR_H
#define MIXEREDITOR_H

#include "applicationsession.h"

class MixerEditor
{
public:
    MixerEditor();

    void SetUp(
        ApplicationSession *session);

    void Render2d();

    static char const *ID;

private:
    ApplicationSession *_session = nullptr;

    void RenderTrack(
        int trackIndex);
};

#endif // MIXEREDITOR_H
