#ifndef LIBRARYDIALOG_H
#define LIBRARYDIALOG_H

#include <functional>
#include <set>
#include <zyn.common/ILibraryManager.h>

#include "applicationsession.h"

class LibraryDialog
{
    ApplicationSession *_session = nullptr;
    struct
    {
        ILibrary *selectedLibrary;
        ILibraryItem *selectedSample;
        char filter[64];
        std::set<ILibraryItem *> filteredSamples;
    } _selectSample;

    void filterSamples();

public:
    LibraryDialog();

    void SetUp(ApplicationSession *session);

    void ShowDialog(bool open, std::function<void(ILibraryItem *)> const &func);
};

#endif // LIBRARYDIALOG_H
