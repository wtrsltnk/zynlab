#ifndef UI_PADNOTE_H
#define UI_PADNOTE_H

#include "../appstate.h"
#include "ui.envelope.h"
#include "ui.lfo.h"
#include <zyn.mixer/Channel.h>
#include <zyn.synth/PADnoteParams.h>

namespace zyn {
namespace ui {

class PadNote
{
private:
    AppState *_state;
    Envelope _AmplitudeEnvelope;
    Lfo _AmplitudeLfo;

    // PAD note
    void PADNoteEditor(Channel *channel, int instrumentIndex);
    void PADNoteEditorAmplitude(PADnoteParameters *parameters);

public:
    PadNote(AppState *state);
    virtual ~PadNote();

    bool Setup();
    void Render();
};

} // namespace ui
} // namespace zyn

#endif // UI_PADNOTE_H
