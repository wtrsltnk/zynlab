#include "acutest.h"

#include "../par.h"
#include <zyn.mixer/Track.h>

void testme()
{
    auto detuneId = "/kit[0]/add/detune-type";

    Track t;
    auto p = Par::GetPar(&t, detuneId);

    TEST_CHECK(p.id != Par::Empty().id);
    TEST_CHECK(p.id == detuneId);
    TEST_CHECK(p.ucvalue != nullptr);
    TEST_CHECK(p.type == ParTypes::UnsignedChar);
}

TEST_LIST = {
    {"testme", testme},
    {NULL, NULL} /* zeroed record marking the end of the list */
};
