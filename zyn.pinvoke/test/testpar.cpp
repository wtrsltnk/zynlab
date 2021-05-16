#include "acutest.h"

#include "../par.h"
#include <zyn.mixer/Track.h>
#include <zyn.mixer/Mixer.h>

Track t;
Mixer mixer;

void AddSynthDetuneTypeShouldWork()
{
    mixer.Init();
    t.Init(&mixer, &(mixer.microtonal));

    auto id = "/INSTRUMENT_KIT_ITEM[0]/ADD_SYNTH_PARAMETERS/detune";

    auto p = Par::GetPar(&t, id);

    TEST_CHECK(p != Par::Empty());
    TEST_CHECK(p.id == id);
    TEST_CHECK(p.ucvalue == nullptr);
    TEST_CHECK(p.usvalue != nullptr);
    TEST_CHECK(p.fvalue == nullptr);
    TEST_CHECK(p.bvalue == nullptr);
    TEST_CHECK(p.AsUnsignedShort() == 8192);
    TEST_CHECK(p.type == ParTypes::UnsignedShort);
}

void AddSynthCoarseDetuneTypeShouldWork()
{
    mixer.Init();
    t.Init(&mixer, &(mixer.microtonal));

    auto id = "/kit[0]/add/coarse-detune";

    auto p = Par::GetPar(&t, id);

    TEST_CHECK(p != Par::Empty());
    TEST_CHECK(p.id == id);
    TEST_CHECK(p.ucvalue == nullptr);
    TEST_CHECK(p.usvalue != nullptr);
    TEST_CHECK(p.fvalue == nullptr);
    TEST_CHECK(p.bvalue == nullptr);
    TEST_CHECK(p.AsUnsignedShort() == 0);
    TEST_CHECK(p.type == ParTypes::UnsignedShort);
}

void AddSynthDetuneShouldWork()
{
    mixer.Init();
    t.Init(&mixer, &(mixer.microtonal));

    auto id = "/kit[0]/add/detune-type";

    auto p = Par::GetPar(&t, id);

    TEST_CHECK(p != Par::Empty());
    TEST_CHECK(p.id == id);
    TEST_CHECK(p.ucvalue != nullptr);
    TEST_CHECK(p.usvalue == nullptr);
    TEST_CHECK(p.fvalue == nullptr);
    TEST_CHECK(p.bvalue == nullptr);
    TEST_CHECK(p.AsUnsignedChar() == 1);
    TEST_CHECK(p.type == ParTypes::UnsignedChar);
}

TEST_LIST = {
    {"Add Synth Detune should work", AddSynthDetuneShouldWork},
    {"Add Synth Coarse-Detune should work", AddSynthCoarseDetuneTypeShouldWork},
    {"Add Synth Detune-Type should work", AddSynthDetuneTypeShouldWork},
    {NULL, NULL} /* zeroed record marking the end of the list */
};
