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
        std::string currentPath;
        std::vector<std::string> currentFolders;
        std::vector<std::string> currentFiles;
        char fileNameBuffer[256];
    } _saveFileDialog;
    void updateSaveFileDialog();

    struct
    {
        bool active;
        std::string currentPath;
        std::vector<std::string> currentFolders;
        std::vector<std::string> currentFiles;
        char fileNameBuffer[256];
    } _openFileDialog;
    void updateOpenFileDialog();

public:
    Dialogs(AppState *state);

    bool Setup();

    void SaveFileDialog(char const *title);
    DialogResults RenderSaveFileDialog();
    std::string GetSaveFileName();

    static char const *SAVEFILE_DIALOG_ID;

    void OpenFileDialog(char const *title);
    DialogResults RenderOpenFileDialog();
    std::string GetOpenFileName();

    static char const *OPENFILE_DIALOG_ID;
};

} // namespace ui
} // namespace zyn

#endif // UI_DIALOGS_H
