#ifndef PATTERNSMANAGER_H
#define PATTERNSMANAGER_H

#include "applicationsession.h"

class PatternsManager
{
    ApplicationSession *_session;
    char _patternPropertiesNameBuffer[128] = {0};
    unsigned int _selectedPattern = 0;

public:
    PatternsManager();

    void SetUp(ApplicationSession *session);
    void Render2d();

    void SelectPattern(unsigned int i);
};

#endif // PATTERNSMANAGER_H
