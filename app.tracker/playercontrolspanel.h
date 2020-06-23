#ifndef PLAYERCONTROLSPANEL_H
#define PLAYERCONTROLSPANEL_H

#include "applicationsession.h"

class PlayerControlsPanel
{
    ApplicationSession *_session;

public:
    PlayerControlsPanel();

    void SetUp(ApplicationSession *session);
    void Render2d();
};

#endif // PLAYERCONTROLSPANEL_H
