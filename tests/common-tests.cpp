#include <catch2/catch.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <zyn.common/PresetsSerializer.h>
#include <zyn.dsp/FilterParams.h>
#include <zyn.synth/EnvelopeParams.h>
#include <zyn.synth/LFOParams.h>
#include <zyn.synth/Resonance.h>
#include <zyn.synth/SUBnoteParams.h>
#include <zyn.mixer/Microtonal.h>

TEST_CASE("EnvelopeParams Presets", "[zyn.synth]")
{
    auto sut = EnvelopeParams('\0', '\0');
    sut.Defaults();
    sut.InitPresets();
    sut.Pforcedrelease = 1;

    auto serializerA = PresetsSerializer();
    sut.Serialize(&serializerA);
    auto presetsA = std::string(serializerA.getXMLdata());

    auto serializerB = PresetsSerializer();
    sut.WritePresetsToBlob(&serializerB);
    auto presetsB = std::string(serializerB.getXMLdata());

    REQUIRE(presetsA == presetsB);
}

TEST_CASE("EnvelopeParams Presets Deserialization", "[zyn.synth]")
{
    auto sut = EnvelopeParams('\0', '\0');
    sut.Defaults();
    sut.InitPresets();

    REQUIRE(sut.Pforcedrelease == 0);
    REQUIRE(sut.PA_dt == 10);
    REQUIRE(sut.PD_dt == 10);
    REQUIRE(sut.PR_dt == 10);
    REQUIRE(sut.PA_val == 64);
    REQUIRE(sut.PD_val == 64);
    REQUIRE(sut.PS_val == 64);
    REQUIRE(sut.PR_val == 64);
    REQUIRE(sut.Penvpoints == 4);
    REQUIRE(sut.Penvdt[0] == 0);
    REQUIRE(sut.Penvdt[1] == 10);
    REQUIRE(sut.Penvdt[2] == 10);
    REQUIRE(sut.Penvdt[3] == 10);
    REQUIRE(sut.Penvval[0] == 0);
    REQUIRE(sut.Penvval[1] == 127);
    REQUIRE(sut.Penvval[2] == 64);
    REQUIRE(sut.Penvval[3] == 0);

    auto serializer = PresetsSerializer();
    serializer.loadXMLfile(std::string(TEST_DATA_PATH) + "EnvelopeParams.txt");

    sut.ReadPresetsFromBlob(&serializer);

    REQUIRE(sut.Pforcedrelease == 1);
    REQUIRE(sut.PA_dt == 11);
    REQUIRE(sut.PD_dt == 12);
    REQUIRE(sut.PR_dt == 13);
    REQUIRE(sut.PA_val == 65);
    REQUIRE(sut.PD_val == 66);
    REQUIRE(sut.PS_val == 67);
    REQUIRE(sut.PR_val == 68);
    REQUIRE(sut.Penvpoints == 4);
    REQUIRE(sut.Penvdt[0] == 0);
    REQUIRE(sut.Penvdt[1] == 11);
    REQUIRE(sut.Penvdt[2] == 12);
    REQUIRE(sut.Penvdt[3] == 13);
    REQUIRE(sut.Penvval[0] == 1);
    REQUIRE(sut.Penvval[1] == 126);
    REQUIRE(sut.Penvval[2] == 65);
    REQUIRE(sut.Penvval[3] == 1);
}

TEST_CASE("LFOParams Presets", "[zyn.synth]")
{
    auto sut = LFOParams(70, 0, 64, 0, 0, 0, 0, 0);
    sut.Defaults();
    sut.InitPresets();

    auto serializerA = PresetsSerializer();
    sut.Serialize(&serializerA);
    auto presetsA = std::string(serializerA.getXMLdata());

    auto serializerB = PresetsSerializer();
    sut.WritePresetsToBlob(&serializerB);
    auto presetsB = std::string(serializerB.getXMLdata());

    REQUIRE(presetsA == presetsB);
}

TEST_CASE("Resonance Presets", "[zyn.synth]")
{
    auto sut = Resonance();
    sut.Defaults();
    sut.InitPresets();
    sut.Penabled = 1;

    auto serializerA = PresetsSerializer();
    sut.Serialize(&serializerA);
    auto presetsA = std::string(serializerA.getXMLdata());

    auto serializerB = PresetsSerializer();
    sut.WritePresetsToBlob(&serializerB);
    auto presetsB = std::string(serializerB.getXMLdata());

    REQUIRE(presetsA == presetsB);
}

TEST_CASE("Filter Presets", "[zyn.dsp]")
{
    auto sut = FilterParams(0, 64, 64);
    sut.Defaults();
    sut.InitPresets();

    auto serializerA = PresetsSerializer();
    sut.Serialize(&serializerA);
    auto presetsA = std::string(serializerA.getXMLdata());

    auto serializerB = PresetsSerializer();
    sut.WritePresetsToBlob(&serializerB);
    auto presetsB = std::string(serializerB.getXMLdata());

    REQUIRE(presetsA == presetsB);
}

TEST_CASE("SUB Synth Presets", "[zyn.synth]")
{
    auto sut = SUBnoteParameters();
    sut.Defaults();
    sut.InitPresets();

    auto serializerA = PresetsSerializer();
    serializerA.minimal = false;
    sut.Serialize(&serializerA);
    auto presetsA = std::string(serializerA.getXMLdata());

    auto serializerB = PresetsSerializer();
    serializerB.minimal = false;
    sut.WritePresetsToBlob(&serializerB);
    auto presetsB = std::string(serializerB.getXMLdata());

    REQUIRE(presetsA == presetsB);
}

TEST_CASE("Microtonal Presets", "[zyn.mixer]")
{
    auto sut = Microtonal();
    sut.Defaults();
    sut.InitPresets();
    sut.Penabled = 1;

    auto serializerA = PresetsSerializer();
    serializerA.minimal = false;
    sut.Serialize(&serializerA);
    auto presetsA = std::string(serializerA.getXMLdata());

    auto serializerB = PresetsSerializer();
    serializerB.minimal = false;
    sut.WritePresetsToBlob(&serializerB);
    auto presetsB = std::string(serializerB.getXMLdata());

    REQUIRE(presetsA == presetsB);
}
