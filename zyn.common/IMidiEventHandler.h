#ifndef IMIDIEVENTHANDLER_H
#define IMIDIEVENTHANDLER_H

class IMidiEventHandler
{
public:
    virtual ~IMidiEventHandler();

    // Midi IN
    virtual void NoteOn(unsigned char chan, unsigned char note, unsigned char velocity) = 0;
    virtual void NoteOff(unsigned char chan, unsigned char note) = 0;
    virtual void SetController(unsigned char chan, int type, int par) = 0;
    virtual void SetProgram(unsigned char chan, unsigned int pgm) = 0;
    virtual void PolyphonicAftertouch(unsigned char chan, unsigned char note, unsigned char velocity) = 0;
};

#endif  // IMIDIEVENTHANDLER_H
