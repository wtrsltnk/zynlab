#ifndef MIXEREDITOR_H
#define MIXEREDITOR_H

#include <imgui.h>

#include "applicationsession.h"

class MixerEditor
{
public:
    MixerEditor();

    void SetUp(
        ApplicationSession *session,
        ImFont *font);

    void Render2d();

    static char const *ID;

private:
    ApplicationSession *_session = nullptr;
    ImFont *_monofont = nullptr;

    void RenderTrack(
        int trackIndex);
};

#endif // MIXEREDITOR_H
