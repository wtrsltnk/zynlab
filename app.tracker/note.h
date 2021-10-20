#ifndef NOTE_H
#define NOTE_H

class Note
{
public:
    unsigned int _note = 0;
    unsigned int _length = 0;
    unsigned int _velocity = 0;

    void Clear();

    void Set(
        int note,
        int length,
        int velocity);

        static char const *NoteToString(unsigned int note);

    static char const *ValueToString(unsigned int note);

    static unsigned int StringToValue(char const str[2]);
};

#endif // NOTE_H
