#ifndef AUTOMATIONEDITOR_H
#define AUTOMATIONEDITOR_H

#include "applicationsession.h"

class AutomationEditor
{
    ApplicationSession *_session;

public:
    AutomationEditor();

    void SetUp(ApplicationSession *session);
    void Render2d();
};

#endif // AUTOMATIONEDITOR_H
