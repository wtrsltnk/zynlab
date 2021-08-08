#ifndef SYNTHEDITOR_H
#define SYNTHEDITOR_H

#include "librarydialog.h"
#include <imgui.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/EnvelopeParams.h>
#include <zyn.synth/LFOParams.h>
#include <zyn.synth/PADnoteParams.h>
#include <zyn.synth/SUBnoteParams.h>
#include <zyn.synth/SampleNoteParams.h>

class SynthEditor
{
    IMixer *_mixer;
    ILibraryManager *_library;
    LibraryDialog _libraryDialog;

public:
    SynthEditor();

    void SetUp(
        IMixer *mixer,
        ILibraryManager *library);

    void Render2d(
        ImGuiWindowFlags flags = 0);

    void RenderEnvelope(
        char const *label,
        EnvelopeParams *envelope,
        unsigned char *enabled);

    void RenderLfo(
        char const *label,
        LFOParams *lfo,
        unsigned char *enabled);

    void RenderFilter(
        FilterParams *filter);

    void RenderDetune(
        unsigned char &detuneType,
        unsigned short int &coarseDetune,
        unsigned short int &detune);

    void RenderAbstractSynth(
        AbstractNoteParameters *params);

    void RenderAddSynth(
        ADnoteParameters *params);

    void RenderSubSynth(
        SUBnoteParameters *params);

    void RenderPadSynth(
        PADnoteParameters *params);

    void RenderSmplSynth(
        SampleNoteParameters *params);

    static char const *ID;
};

#endif // SYNTHEDITOR_H
