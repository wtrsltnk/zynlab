#ifndef UI_MIXER_H
#define UI_MIXER_H

#include "../appstate.h"
#include "../instrumentcategories.h"

namespace zyn {
namespace ui {

class Mixer
{
private:
    AppState *_state;
    unsigned int _iconImages[int(InstrumentCategories::COUNT)];
    bool _iconImagesAreLoaded;

protected:
    void LoadInstrumentIcons();
    void ImGuiMasterTrack();
    void ImGuiMixer();
    void ImGuiTrack(int trackIndex, bool highlightTrack);
    void ImGuiInspector();
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

    bool ShowInspector() const;
    void ShowInspector(bool show);
};

} // namespace ui
} // namespace zyn

#endif // UI_MIXER_H
