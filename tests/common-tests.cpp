#include <catch2/catch.hpp>
#include <iostream>
#include <string>
#include <zyn.common/PresetsSerializer.h>
#include <zyn.synth/EnvelopeParams.h>
#include <zyn.synth/LFOParams.h>
#include <zyn.synth/Resonance.h>

TEST_CASE("EnvelopeParams Presets", "[zyn.common]")
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

TEST_CASE("LFOParams Presets", "[zyn.common]")
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

TEST_CASE("Resonance Presets", "[zyn.common]")
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
