#include "song.h"

#include <sstream>

unsigned int Song::GetPatternCount() const
{
    return _patterns.size();
}

Pattern *Song::GetPattern(unsigned int index)
{
    if (index >= _patterns.size())
    {
        return nullptr;
    }

    return _patterns[index];
}

void Song::AddPattern()
{
    static int counter = 0;
    std::stringstream ss;
    ss << "Pattern " << counter++;
    auto pattern = new Pattern(ss.str(), 64);

    _patterns.push_back(pattern);
}

void Song::RemovePattern(unsigned int index)
{
    if (_patterns.size() == 1 || index >= _patterns.size())
    {
        return;
    }

    auto tmp = *(_patterns.begin() + index);

    _patterns.erase(_patterns.begin() + index);

    delete tmp;

    currentPattern--;

    if (currentPattern >= _patterns.size())
    {
        currentPattern = 0;
    }
}

void Song::MovePattern(unsigned int index, int direction)
{
    if (_patterns.size() == 1 || index >= _patterns.size())
    {
        return;
    }

    if (std::abs(direction) == direction)
    { // move down the list
        if (index + direction >= _patterns.size())
        {
            return;
        }
    }
    else
    { // move up the list
        if (index < static_cast<unsigned int>(-1 * direction))
        {
            return;
        }
    }

    unsigned int newIndex = index + direction;

    auto tmp = *(_patterns.begin() + index);

    _patterns.erase(_patterns.begin() + index);

    _patterns.insert(_patterns.begin() + newIndex, tmp);

    currentPattern = newIndex;
}

void Song::DuplicatePattern(unsigned int index)
{
    if (_patterns.size() == 0 || index >= _patterns.size())
    {
        return;
    }

    auto tmp = *(_patterns.begin() + index);

    auto newPattern = new Pattern(*tmp);

    _patterns.insert(_patterns.begin() + index + 1, newPattern);
}
