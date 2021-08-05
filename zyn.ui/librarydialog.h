#ifndef LIBRARYDIALOG_H
#define LIBRARYDIALOG_H

#include <functional>
#include <memory>
#include <set>
#include <zyn.common/ILibraryManager.h>
#include <zyn.common/globals.h>

class LibraryDialog
{
    IMixer *_mixer = nullptr;
    ILibraryManager *_library = nullptr;
    struct
    {
        ILibrary *selectedLibrary = nullptr;
        ILibraryItem *selectedSample = nullptr;
        char filter[64] = {};
        std::set<ILibraryItem *> filteredSamples;
    } _selectSample;

    void filterSamples();

public:
    LibraryDialog();

    void SetUp(
        IMixer *mixer,
        ILibraryManager *library);

    void ShowDialog(bool open, std::function<void(ILibraryItem *)> const &func);
};

#endif // LIBRARYDIALOG_H
