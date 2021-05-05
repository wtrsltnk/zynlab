#include "LibraryManager.h"

Library::Library(std::string const &name, std::string const &path, ILibrary *parent)
    : _name(name), _path(path), _parent(parent)
{}

Library::~Library()
{
    for (auto item : _children)
    {
        delete item;
    }
    _children.clear();
}

std::string const &Library::GetName()const
{
    return _name;
}

std::string const &Library::GetPath()const
{
    return _path;
}

ILibrary *Library::GetParent()
{
    return _parent;
}

std::set<ILibrary *> &Library::GetChildren()
{
    return _children;
}

bool Library::IsParent(ILibrary *library)
{
    if (library == (ILibrary *)this)
    {
        return true;
    }

    if (_parent == nullptr)
    {
        return false;
    }

    return _parent->IsParent(library);
}

void Library::AddLibraryItem(ILibraryItem *item)
{
    _items.insert(item);
}

std::set<ILibraryItem *> &Library::GetItems()
{
    return _items;
}
