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

    static unsigned int StringToValue(char const str[2]);
};

#endif // NOTE_H
