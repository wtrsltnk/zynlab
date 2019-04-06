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

    ILibraryItem *_selectedSample;
    char _filter[64];
    std::set<ILibraryItem *> _filteredSamples;

    void libraryTree(ILibrary *library);
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
