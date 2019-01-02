#include <catch2/catch.hpp>
#include <zyn.synth/EnvelopeParams.h>
#include <zyn.common/PresetsSerializer.h>
#include <iostream>

TEST_CASE("Presets", "[zyn.common]")
{
    auto sut = EnvelopeParams('\0', '\0');

    auto serializer = PresetsSerializer();

    sut.Defaults();
//    sut.Serialize(&serializer);

//    std::cout << serializer.getXMLdata() << std::endl;

//    serializer = PresetsSerializer();
    sut.InitPresets();
    sut.WriteToBlob(&serializer);

    std::cout << serializer.getXMLdata() << std::endl;
}
