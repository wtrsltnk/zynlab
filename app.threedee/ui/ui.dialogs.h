#ifndef UI_DIALOGS_H
#define UI_DIALOGS_H

#include "../appstate.h"

namespace zyn {
namespace ui {

enum class DialogResults
{
    NoResult,
    Ok,
    Cancel,
};

class Dialogs
{
private:
    AppState *_state;

    struct
    {
        bool active;
        std::string title;
        std::string currentPath;
        std::vector<std::string> currentFolders;
        std::vector<std::string> currentFiles;
        char fileNameBuffer[256];
    } _saveFileDialog;
    void updateSaveFileDialog();

public:
    Dialogs(AppState *state);

    bool Setup();

    void SaveFileDialog(std::string const &extension);
    DialogResults RenderSaveFileDialog();
    std::string GetSaveFileName();
};

} // namespace ui
} // namespace zyn

#endif // UI_DIALOGS_H
