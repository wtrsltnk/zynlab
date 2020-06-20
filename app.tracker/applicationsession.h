#ifndef APPLICATIONSESSION_H
#define APPLICATIONSESSION_H

class ApplicationSession
{
public:
    ApplicationSession();

    unsigned int _bpm = 138;
    unsigned int currentTrack = 0;
    unsigned int currentRow = 0;
    unsigned int currentProperty = 0;
    unsigned int skipRowStepSize = 4;
};

#endif // APPLICATIONSESSION_H
