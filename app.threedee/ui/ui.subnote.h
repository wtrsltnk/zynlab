#ifndef UI_SUBNOTE_H
#define UI_SUBNOTE_H

#include "../appstate.h"
#include "ui.envelope.h"
#include "ui.filter.h"
#include <zyn.mixer/Track.h>
#include <zyn.synth/SUBnoteParams.h>

namespace zyn {
namespace ui {

class SubNote
{
private:
    AppState *_state;
    Envelope _AmplitudeEnvelope;
    Envelope _BandwidthEnvelope;
    Envelope _FilterEnvelope;
    Envelope _FrequencyEnvelope;
    Filter _FilterParameters;

    // SUB note
    void SUBNoteEditorHarmonicsMagnitude(SUBnoteParameters *parameters);
    void SUBNoteEditorAmplitude(SUBnoteParameters *parameters);
    void SUBNoteEditorBandwidth(SUBnoteParameters *parameters);
    void SUBNoteEditorOvertones(SUBnoteParameters *parameters);
    void SUBNoteEditorFilter(SUBnoteParameters *parameters);
    void SUBNoteEditorFrequency(SUBnoteParameters *parameters);

public:
    SubNote(AppState *state);
    virtual ~SubNote();

    bool Setup();
    void Render();
};

} // namespace ui
} // namespace zyn

#endif // UI_SUBNOTE_H
