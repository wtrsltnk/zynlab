#include <catch2/catch.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <zyn.common/PresetsSerializer.h>
#include <zyn.dsp/FilterParams.h>
#include <zyn.mixer/Microtonal.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.mixer/Track.h>
#include <zyn.serialization/ADnoteParamsSerializer.h>
#include <zyn.serialization/ControllerSerializer.h>
#include <zyn.serialization/EnvelopeParamsSerializer.h>
#include <zyn.serialization/FilterParamsSerializer.h>
#include <zyn.serialization/LFOParamsSerializer.h>
#include <zyn.serialization/MicrotonalSerializer.h>
#include <zyn.serialization/OscilGenSerializer.h>
#include <zyn.serialization/PADnoteParamsSerializer.h>
#include <zyn.serialization/ResonanceSerializer.h>
#include <zyn.serialization/SUBnoteParamsSerializer.h>
#include <zyn.serialization/TrackSerializer.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/Controller.h>
#include <zyn.synth/EnvelopeParams.h>
#include <zyn.synth/FFTwrapper.h>
#include <zyn.synth/LFOParams.h>
#include <zyn.synth/OscilGen.h>
#include <zyn.synth/PADnoteParams.h>
#include <zyn.synth/Resonance.h>
#include <zyn.synth/SUBnoteParams.h>

TEST_CASE("Presets")
{
    PresetsSerializer serializerA;
    serializerA.minimal = false;

    PresetsSerializer serializerB;
    serializerB.minimal = false;

    auto fft = FFTwrapper(1024);

    SECTION("AD Synth Presets", "[zyn.synth]")
    {
        auto sut = ADnoteParameters(&fft);
        sut.Defaults();
        sut.InitPresets();

        SECTION("Serialization should work")
        {
            ADnoteParametersSerializer(&sut).Serialize(&serializerA);

            sut.WritePresetsToBlob(&serializerB);

            REQUIRE(std::string(serializerA.getXMLdata()) == std::string(serializerB.getXMLdata()));
        }
    }

    SECTION("PAD Synth Presets", "[zyn.synth]")
    {
        auto sut = PADnoteParameters(&fft);
        sut.Defaults();
        sut.InitPresets();

        SECTION("Serialization should work")
        {
            PADnoteParametersSerializer(&sut).Serialize(&serializerA);

            sut.WritePresetsToBlob(&serializerB);

            // TODO We skip this assert as long there is no solution for the ugly 'setPadSynth()' method
            // REQUIRE(std::string(serializerA.getXMLdata()) == std::string(serializerB.getXMLdata()));
        }
    }

    SECTION("SUB Synth Presets", "[zyn.synth]")
    {
        auto sut = SUBnoteParameters();
        sut.Defaults();
        sut.InitPresets();

        SECTION("Serialization should work")
        {
            SUBnoteParametersSerializer(&sut).Serialize(&serializerA);

            sut.WritePresetsToBlob(&serializerB);

            REQUIRE(std::string(serializerA.getXMLdata()) == std::string(serializerB.getXMLdata()));
        }
    }

    SECTION("Track Presets", "[zyn.mixer]")
    {
        Mixer mixer;
        mixer.Setup();

        Microtonal micro;
        micro.Defaults();

        auto sut = Track();
        sut.Init(&mixer, &micro);
        sut.Defaults();
        sut.InitPresets();

        SECTION("Serialization should work")
        {
            TrackSerializer(&sut).Serialize(&serializerA);

            sut.WritePresetsToBlob(&serializerB);

            // TODO We skip this assert as long there is no solution for the ugly 'setPadSynth()' method
            // REQUIRE(std::string(serializerA.getXMLdata()) == std::string(serializerB.getXMLdata()));
        }
    }

    SECTION("EnvelopeParams Presets", "[zyn.synth]")
    {
        auto sut = EnvelopeParams('\0', '\0');
        sut.Defaults();
        sut.InitPresets();

        SECTION("Serialization should work")
        {
            sut.Pforcedrelease = 1;

            EnvelopeParamsSerializer(&sut).Serialize(&serializerA);

            sut.WritePresetsToBlob(&serializerB);

            REQUIRE(std::string(serializerA.getXMLdata()) == std::string(serializerB.getXMLdata()));
        }

        SECTION("Deserialization should work")
        {
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

            serializerA.loadXMLfile(std::string(TEST_DATA_PATH) + "EnvelopeParams.txt");

            sut.ReadPresetsFromBlob(&serializerA);

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
    }

    SECTION("LFOParams Presets", "[zyn.synth]")
    {
        PresetsSerializer serializerA;
        serializerA.minimal = false;

        PresetsSerializer serializerB;
        serializerB.minimal = false;

        auto sut = LFOParams(70, 0, 64, 0, 0, 0, 0, 0);
        sut.Defaults();
        sut.InitPresets();

        SECTION("Serialization should work")
        {
            LFOParamsSerializer(&sut).Serialize(&serializerA);

            sut.WritePresetsToBlob(&serializerB);

            REQUIRE(std::string(serializerA.getXMLdata()) == std::string(serializerB.getXMLdata()));
        }
    }

    SECTION("Resonance Presets", "[zyn.synth]")
    {
        PresetsSerializer serializerA;
        serializerA.minimal = false;
        PresetsSerializer serializerB;
        serializerB.minimal = false;

        auto sut = Resonance();
        sut.Defaults();
        sut.InitPresets();
        sut.Penabled = 1;

        SECTION("Serialization should work")
        {
            ResonanceSerializer(&sut).Serialize(&serializerA);

            sut.WritePresetsToBlob(&serializerB);

            REQUIRE(std::string(serializerA.getXMLdata()) == std::string(serializerB.getXMLdata()));
        }
    }

    SECTION("Filter Presets", "[zyn.dsp]")
    {
        auto sut = FilterParams(0, 64, 64);
        sut.Defaults();
        sut.InitPresets();

        SECTION("Serialization should work")
        {
            FilterParamsSerializer(&sut).Serialize(&serializerA);

            sut.WritePresetsToBlob(&serializerB);

            REQUIRE(std::string(serializerA.getXMLdata()) == std::string(serializerB.getXMLdata()));
        }
    }

    SECTION("Microtonal Presets", "[zyn.mixer]")
    {
        auto sut = Microtonal();
        sut.Defaults();
        sut.InitPresets();
        sut.Penabled = 1;

        SECTION("Serialization should work")
        {
            MicrotonalSerializer(&sut).Serialize(&serializerA);

            sut.WritePresetsToBlob(&serializerB);

            REQUIRE(std::string(serializerA.getXMLdata()) == std::string(serializerB.getXMLdata()));
        }
    }

    SECTION("OscilGen Presets", "[zyn.synth]")
    {
        auto res = Resonance();
        res.Defaults();
        res.InitPresets();

        auto sut = OscilGen(&fft, &res);
        sut.Defaults();
        sut.InitPresets();
        sut.Pcurrentbasefunc = 0;

        SECTION("Serialization should work")
        {
            OscilGenSerializer(&sut).Serialize(&serializerA);

            sut.WritePresetsToBlob(&serializerB);

            REQUIRE(std::string(serializerA.getXMLdata()) == std::string(serializerB.getXMLdata()));
        }
    }

    SECTION("Controller Presets", "[zyn.synth]")
    {
        auto sut = Controller();
        sut.Defaults();
        sut.InitPresets();

        SECTION("Serialization should work")
        {
            ControllerSerializer(&sut).Serialize(&serializerA);

            sut.WritePresetsToBlob(&serializerB);

            REQUIRE(std::string(serializerA.getXMLdata()) == std::string(serializerB.getXMLdata()));
        }
    }
}
