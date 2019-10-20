#ifndef UI_MIXER_H
#define UI_MIXER_H

#include "../appstate.h"
#include "../instrumentcategories.h"
#include "ui.dialogs.h"

#define INSPECTOR_WIDTH 320

namespace zyn {
namespace ui {

class Mixer
{
private:
    AppState *_state;
    zyn::ui::Dialogs _dialogs;
    unsigned int _iconImages[int(InstrumentCategories::COUNT)];
    bool _iconImagesAreLoaded;

protected:
    void LoadInstrumentIcons();
    void ImGuiMasterTrack();
    void ImGuiMixer();
    void ImGuiTrack(int trackIndex, bool highlightTrack);
    void ImGuiChangeInstrumentTypePopup();
    void AddInsertFx(int track);
    void RemoveInsertFxFromTrack(int fx);

public:
    Mixer(AppState *appstate);
    virtual ~Mixer();

    bool Setup();
    void Render();

    bool ShowMixer() const;
    void ShowMixer(bool show);

    void RenderInspector();
    bool ShowInspector() const;
    void ShowInspector(bool show);
};

} // namespace ui
} // namespace zyn

#endif // UI_MIXER_H
