#include "note.h"

void Note::Clear()
{
    _note = _velocity = _length = 0;
}

void Note::Set(
    int note,
    int length,
    int velocity)
{
    _note = note;
    _velocity = velocity;
    _length = length;
}

char const *Note::NoteToString(
    unsigned int note)
{
    switch (note)
    {
        case 127:
            return "G-9";
        case 126:
            return "F#9";
        case 125:
            return "F-9";
        case 124:
            return "E-9";
        case 123:
            return "D#9";
        case 122:
            return "D-9";
        case 121:
            return "C#9";
        case 120:
            return "C-9";
        case 119:
            return "B-8";
        case 118:
            return "A#8";
        case 117:
            return "A-8";
        case 116:
            return "G#8";
        case 115:
            return "G-8";
        case 114:
            return "F#8";
        case 113:
            return "F-8";
        case 112:
            return "E-8";
        case 111:
            return "D#8";
        case 110:
            return "D-8";
        case 109:
            return "C#8";
        case 108:
            return "C-8";
        case 107:
            return "B-7";
        case 106:
            return "A#7";
        case 105:
            return "A-7";
        case 104:
            return "G#7";
        case 103:
            return "G-7";
        case 102:
            return "F#7";
        case 101:
            return "F-7";
        case 100:
            return "E-7";
        case 99:
            return "D#7";
        case 98:
            return "D-7";
        case 97:
            return "C#7";
        case 96:
            return "C-7";
        case 95:
            return "B-6";
        case 94:
            return "A#6";
        case 93:
            return "A-6";
        case 92:
            return "G#6";
        case 91:
            return "G-6";
        case 90:
            return "F#6";
        case 89:
            return "F-6";
        case 88:
            return "E-6";
        case 87:
            return "D#6";
        case 86:
            return "D-6";
        case 85:
            return "C#6";
        case 84:
            return "C-6";
        case 83:
            return "B-5";
        case 82:
            return "A#5";
        case 81:
            return "A-5";
        case 80:
            return "G#5";
        case 79:
            return "G-5";
        case 78:
            return "F#5";
        case 77:
            return "F-5";
        case 76:
            return "E-5";
        case 75:
            return "D#5";
        case 74:
            return "D-5";
        case 73:
            return "C#5";
        case 72:
            return "C-5";
        case 71:
            return "B-4";
        case 70:
            return "A#4";
        case 69:
            return "A-4";
        case 68:
            return "G#4";
        case 67:
            return "G-4";
        case 66:
            return "F#4";
        case 65:
            return "F-4";
        case 64:
            return "E-4";
        case 63:
            return "D#4";
        case 62:
            return "D-4";
        case 61:
            return "C#4";
        case 60:
            return "C-4";
        case 59:
            return "B-3";
        case 58:
            return "A#3";
        case 57:
            return "A-3";
        case 56:
            return "G#3";
        case 55:
            return "G-3";
        case 54:
            return "F#3";
        case 53:
            return "F-3";
        case 52:
            return "E-3";
        case 51:
            return "D#3";
        case 50:
            return "D-3";
        case 49:
            return "C#3";
        case 48:
            return "C-3";
        case 47:
            return "B-2";
        case 46:
            return "A#2";
        case 45:
            return "A-2";
        case 44:
            return "G#2";
        case 43:
            return "G-2";
        case 42:
            return "F#2";
        case 41:
            return "F-2";
        case 40:
            return "E-2";
        case 39:
            return "D#2";
        case 38:
            return "D-2";
        case 37:
            return "C#2";
        case 36:
            return "C-2";
        case 35:
            return "B-1";
        case 34:
            return "A#1";
        case 33:
            return "A-1";
        case 32:
            return "G#1";
        case 31:
            return "G-1";
        case 30:
            return "F#1";
        case 29:
            return "F-1";
        case 28:
            return "E-1";
        case 27:
            return "D#1";
        case 26:
            return "D-1";
        case 25:
            return "C#1";
        case 24:
            return "C-1";
        case 23:
            return "B-0";
        case 22:
            return "A#0";
        case 21:
            return "A-0";
    }
    return "---";
}

unsigned int Note::StringToValue(
    char const str[2])
{
    unsigned int result = 0;
    if (str[1] >= '0' && str[1] <= '9')
    {
        result += str[1] - '0';
    }
    else if (str[1] >= 'A' && str[1] <= 'F')
    {
        result += str[1] - 'A' + 10;
    }

    if (str[0] >= '0' && str[0] <= '9')
    {
        result += (str[0] - '0') * 16;
    }
    else if (str[0] >= 'A' && str[0] <= 'F')
    {
        result += (str[0] - 'A' + 10) * 16;
    }

    if (result >= 127)
    {
        return 127;
    }

    return result;
}

char const *Note::ValueToString(
    unsigned int value)
{
    switch (value)
    {
        case 1:
            return "01";
        case 2:
            return "02";
        case 3:
            return "03";
        case 4:
            return "04";
        case 5:
            return "05";
        case 6:
            return "06";
        case 7:
            return "07";
        case 8:
            return "08";
        case 9:
            return "09";
        case 10:
            return "0A";
        case 11:
            return "0B";
        case 12:
            return "0C";
        case 13:
            return "0D";
        case 14:
            return "0E";
        case 15:
            return "0F";
        case 16:
            return "10";
        case 17:
            return "11";
        case 18:
            return "12";
        case 19:
            return "13";
        case 20:
            return "14";
        case 21:
            return "15";
        case 22:
            return "16";
        case 23:
            return "17";
        case 24:
            return "18";
        case 25:
            return "19";
        case 26:
            return "1A";
        case 27:
            return "1B";
        case 28:
            return "1C";
        case 29:
            return "1D";
        case 30:
            return "1E";
        case 31:
            return "1F";
        case 32:
            return "20";
        case 33:
            return "21";
        case 34:
            return "22";
        case 35:
            return "23";
        case 36:
            return "24";
        case 37:
            return "25";
        case 38:
            return "26";
        case 39:
            return "27";
        case 40:
            return "28";
        case 41:
            return "29";
        case 42:
            return "2A";
        case 43:
            return "2B";
        case 44:
            return "2C";
        case 45:
            return "2D";
        case 46:
            return "2E";
        case 47:
            return "2F";
        case 48:
            return "30";
        case 49:
            return "31";
        case 50:
            return "32";
        case 51:
            return "33";
        case 52:
            return "34";
        case 53:
            return "35";
        case 54:
            return "36";
        case 55:
            return "37";
        case 56:
            return "38";
        case 57:
            return "39";
        case 58:
            return "3A";
        case 59:
            return "3B";
        case 60:
            return "3C";
        case 61:
            return "3D";
        case 62:
            return "3E";
        case 63:
            return "3F";
        case 64:
            return "40";
        case 65:
            return "41";
        case 66:
            return "42";
        case 67:
            return "43";
        case 68:
            return "44";
        case 69:
            return "45";
        case 70:
            return "46";
        case 71:
            return "47";
        case 72:
            return "48";
        case 73:
            return "49";
        case 74:
            return "4A";
        case 75:
            return "4B";
        case 76:
            return "4C";
        case 77:
            return "4D";
        case 78:
            return "4E";
        case 79:
            return "4F";
        case 80:
            return "50";
        case 81:
            return "51";
        case 82:
            return "52";
        case 83:
            return "53";
        case 84:
            return "54";
        case 85:
            return "55";
        case 86:
            return "56";
        case 87:
            return "57";
        case 88:
            return "58";
        case 89:
            return "59";
        case 90:
            return "5A";
        case 91:
            return "5B";
        case 92:
            return "5C";
        case 93:
            return "5D";
        case 94:
            return "5E";
        case 95:
            return "5F";
        case 96:
            return "60";
        case 97:
            return "61";
        case 98:
            return "62";
        case 99:
            return "63";
        case 100:
            return "64";
        case 101:
            return "65";
        case 102:
            return "66";
        case 103:
            return "67";
        case 104:
            return "68";
        case 105:
            return "69";
        case 106:
            return "6A";
        case 107:
            return "6B";
        case 108:
            return "6C";
        case 109:
            return "6D";
        case 110:
            return "6E";
        case 111:
            return "6F";
        case 112:
            return "70";
        case 113:
            return "71";
        case 114:
            return "72";
        case 115:
            return "73";
        case 116:
            return "74";
        case 117:
            return "75";
        case 118:
            return "76";
        case 119:
            return "77";
        case 120:
            return "78";
        case 121:
            return "79";
        case 122:
            return "7A";
        case 123:
            return "7B";
        case 124:
            return "7C";
        case 125:
            return "7D";
        case 126:
            return "7E";
        case 127:
            return "7F";
    }

    return "..";
}
