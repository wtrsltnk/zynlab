#ifndef PATTERNSMANAGER_H
#define PATTERNSMANAGER_H

#include "applicationsession.h"

class PatternsManager
{
    ApplicationSession *_session;

public:
    PatternsManager();

    void SetUp(ApplicationSession *session);
    void Render2d();
};

#endif // PATTERNSMANAGER_H
