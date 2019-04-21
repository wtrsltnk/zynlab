#ifndef UI_LIBRARY_H
#define UI_LIBRARY_H

#include "../appstate.h"
#include <zyn.common/ILibraryManager.h>

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

    struct
    {
        std::string currentPath;
        std::vector<std::string> currentFolders;
        std::vector<std::string> currentFiles;
        char fileNameBuffer[256];
    } _saveFileName;

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

    void RenderGetSaveFileName();
    std::string GetSaveFileName();
};

} // namespace ui
} // namespace zyn

#endif // UI_LIBRARY_H
