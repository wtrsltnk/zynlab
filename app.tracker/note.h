#ifndef NOTE_H
#define NOTE_H

class Note
{
public:
    unsigned int _note;
    unsigned int _length;
    unsigned int _velocity;

    static char const *NoteToString(unsigned int note);

    static char const *ValueToString(unsigned int note);
};


#endif // NOTE_H
