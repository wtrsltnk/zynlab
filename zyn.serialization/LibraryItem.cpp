#include "LibraryManager.h"

LibraryItem::LibraryItem(std::string const &name, std::string const &path, std::set<std::string> const &tags)
    : _name(name), _path(path), _tags(tags)
{}

LibraryItem::~LibraryItem() = default;

std::string const &LibraryItem::GetName()
{
    return _name;
}

std::string const &LibraryItem::GetPath()
{
    return _path;
}

std::set<std::string> const &LibraryItem::GetTags()
{
    return _tags;
}
