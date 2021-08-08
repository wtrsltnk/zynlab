#ifndef LIBRARYDIALOG_H
#define LIBRARYDIALOG_H

#include <functional>
#include <memory>
#include <set>
#include <zyn.common/ILibraryManager.h>
#include <zyn.common/globals.h>

class LibraryDialog
{
public:
    LibraryDialog();

    void SetUp(
        IMixer *mixer,
        ILibraryManager *library);

    void ShowSampleDialog(
        bool open,
        std::function<void(ILibraryItem *)> const &func);

    void ShowInstrumentDialog(
        bool open,
        std::function<void(ILibraryItem *)> const &func);

private:
    IMixer *_mixer = nullptr;
    ILibraryManager *_library = nullptr;
    std::set<ILibraryItem *> _baseItemList;
    struct
    {
        ILibrary *selectedLibrary = nullptr;
        ILibraryItem *selectedSample = nullptr;
        char filter[64] = {};
        std::set<ILibraryItem *> filteredItems;
        std::set<ILibrary *> filteredLibaries;
    } _selectItem;

    void filterSamples();
    void ShowDialog(
        const char *title,
        bool open,
        std::function<void(ILibraryItem *)> const &func);
};

#endif // LIBRARYDIALOG_H
