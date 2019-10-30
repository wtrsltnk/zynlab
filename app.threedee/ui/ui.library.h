#ifndef UI_LIBRARY_H
#define UI_LIBRARY_H

#include "../appstate.h"
#include <zyn.common/ILibraryManager.h>

#define LIBRARY_WIDTH 405

namespace zyn {
namespace ui {

class Library
{
private:
    AppState *_state;

    struct
    {
        ILibrary *selectedLibrary;
        ILibraryItem *selectedSample;
        char filter[64];
        std::set<ILibraryItem *> filteredSamples;
    } _selectSample;

    void updateFilesAndFolders();

    void filterSamples();
    ILibrary *libraryTree(ILibrary *library);
    void InstrumentLibrary();
    void SampleLibrary();

public:
    Library(AppState *state);
    virtual ~Library();

    bool Setup();
    void Render();
    void RenderSelectSample();
    ILibraryItem *GetSelectedSample();
};

} // namespace ui
} // namespace zyn

#endif // UI_LIBRARY_H
