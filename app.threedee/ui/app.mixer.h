#ifndef APP_MIXER_H
#define APP_MIXER_H

#include "../instrumentcategories.h"
#include "../appstate.h"

class AppMixer
{
private:
    AppState *_state;
    unsigned int _iconImages[int(InstrumentCategories::COUNT)];
    bool _iconImagesAreLoaded;

protected:
    void LoadInstrumentIcons();
    void ImGuiMasterTrack();
    void ImGuiMixer();
    void ImGuiTrack(int track, bool highlightTrack);
    void ImGuiInspector();
    void ImGuiSelectInstrumentPopup();
    void ImGuiChangeInstrumentTypePopup();
    void AddInsertFx(int track);
    void RemoveInsertFxFromTrack(int fx);

public:
    AppMixer(AppState *appstate);
    virtual ~AppMixer();

    bool Setup();
    void Render();

    bool ShowMixer() const;
    void ShowMixer(bool show);

    bool ShowInspector() const;
    void ShowInspector(bool show);
};

#endif // APP_MIXER_H
