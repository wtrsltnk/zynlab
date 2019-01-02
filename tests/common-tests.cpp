#include <catch2/catch.hpp>
#include <iostream>
#include <zyn.common/PresetsSerializer.h>
#include <zyn.synth/EnvelopeParams.h>

TEST_CASE("Presets", "[zyn.common]")
{
    auto sut = EnvelopeParams('\0', '\0');
    sut.Defaults();
    sut.Pforcedrelease = 1;

    auto serializerA = PresetsSerializer();

    sut.Serialize(&serializerA);

    std::cout << serializerA.getXMLdata() << std::endl
              << "------" << std::endl;

    auto serializerB = PresetsSerializer();
    sut.InitPresets();
    sut.WriteToBlob(&serializerB);

    std::cout << serializerB.getXMLdata() << std::endl;
}
