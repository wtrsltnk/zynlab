#ifndef UI_ADNOTE_H
#define UI_ADNOTE_H

#include "../appstate.h"
#include "ui.envelope.h"
#include "ui.filter.h"
#include "ui.lfo.h"
#include <zyn.mixer/Channel.h>
#include <zyn.synth/ADnoteParams.h>

namespace zyn {
namespace ui {

class AdNote
{
private:
    AppState *_state;
    Envelope _AmplitudeEnvelope;
    Envelope _FilterEnvelope;
    Envelope _FrequencyEnvelope;
    Envelope _ModulationAmplitudeEnvelope;
    Envelope _ModulationFrequencyEnvelope;
    Lfo _AmplitudeLfo;
    Lfo _FilterLfo;
    Lfo _FrequencyLfo;
    Filter _Filter;

    // AD note
    void ADNoteEditor(Channel *channel, int instrumentIndex);
    void ADNoteEditorAmplitude(ADnoteGlobalParam *parameters);
    void ADNoteEditorFilter(ADnoteGlobalParam *parameters);
    void ADNoteEditorFrequency(ADnoteGlobalParam *parameters);
    void ADNoteVoiceEditor(ADnoteVoiceParam *parameters);
    void ADNoteVoiceEditorOscillator(ADnoteVoiceParam *parameters);
    void ADNoteVoiceEditorOscillatorUnison(ADnoteVoiceParam *parameters);
    void ADNoteVoiceEditorAmplitude(ADnoteVoiceParam *parameters);
    void ADNoteVoiceEditorFilter(ADnoteVoiceParam *parameters);
    void ADNoteVoiceEditorFrequency(ADnoteVoiceParam *parameters);
    void ADNoteVoiceEditorModulation(ADnoteVoiceParam *parameters);

public:
    AdNote(AppState *state);

    bool Setup();
    void Render();
};

} // namespace ui
} // namespace zyn

#endif // UI_ADNOTE_H
