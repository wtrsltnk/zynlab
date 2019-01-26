#ifndef UI_LIBRARY_H
#define UI_LIBRARY_H

#include "../appstate.h"

namespace zyn {
namespace ui {

class Library
{
private:
    AppState *_state;

public:
    Library(AppState *state);
    virtual ~Library();

    bool Setup();
    void Render();
};

} // namespace ui
} // namespace zyn

#endif // UI_LIBRARY_H
