#include "SaveToFileSerializer.h"

#include "MixerSerializer.h"
#include "RegionsSerializer.h"
#include "TrackSerializer.h"
#include <zyn.common/PresetsSerializer.h>

char const *SaveToFileSerializer::WORKSPACE_BRANCH_NAME = "WORKSPACE";

int SaveToFileSerializer::LoadTrack(
    Track *track,
    const std::string &filename)
{
    PresetsSerializer xml;
    if (xml.loadXMLfile(filename) < 0)
    {
        return -1;
    }

    if (xml.enterbranch(TrackSerializer::BRANCH_NAME) == 0)
    {
        return -10;
    }

    TrackSerializer(track).DeserializeInstrument(&xml);
    xml.exitbranch();

    return 0;
}

int SaveToFileSerializer::LoadTrackFromData(
    class Track *track,
    const char *data)
{
    PresetsSerializer xml;
    if (xml.putXMLdata(data) < 0)
    {
        return -1;
    }

    if (xml.enterbranch(TrackSerializer::BRANCH_NAME) == 0)
    {
        return -10;
    }

    TrackSerializer(track).DeserializeInstrument(&xml);
    xml.exitbranch();

    return 0;
}

int SaveToFileSerializer::SaveTrack(
    Track *track,
    const std::string &filename)
{
    PresetsSerializer xml;

    xml.beginbranch(TrackSerializer::BRANCH_NAME);
    TrackSerializer(track).SerializeInstrument(&xml);
    xml.endbranch();

    return xml.saveXMLfile(filename);
}

int SaveToFileSerializer::SaveMixer(
    class Mixer *mixer,
    std::string const &filename)
{
    PresetsSerializer xml;

    xml.beginbranch(MixerSerializer::BRANCH_NAME);
    MixerSerializer(mixer).Serialize(&xml);
    xml.endbranch();

    return xml.saveXMLfile(filename);
}

int SaveToFileSerializer::LoadMixer(
    class Mixer *mixer,
    std::string const &filename)
{
    PresetsSerializer xml;
    if (xml.loadXMLfile(filename) < 0)
    {
        return -1;
    }

    if (xml.enterbranch(MixerSerializer::BRANCH_NAME) == 0)
    {
        return -10;
    }

    MixerSerializer(mixer).Deserialize(&xml);
    xml.exitbranch();

    return 0;
}

int SaveToFileSerializer::SaveWorkspace(
    class Mixer *mixer,
    class RegionsManager *regions,
    std::string const &filename)
{
    PresetsSerializer xml;

    xml.beginbranch(SaveToFileSerializer::WORKSPACE_BRANCH_NAME);

    xml.beginbranch(MixerSerializer::BRANCH_NAME);
    MixerSerializer(mixer).Serialize(&xml);
    xml.endbranch();

    xml.beginbranch(RegionsSerializer::BRANCH_NAME);
    RegionsSerializer(regions).Serialize(&xml);
    xml.endbranch();

    xml.endbranch();

    return xml.saveXMLfile(filename);
}

int SaveToFileSerializer::LoadWorkspace(
    class Mixer *mixer,
    class RegionsManager *regions,
    std::string const &filename)
{
    PresetsSerializer xml;
    if (xml.loadXMLfile(filename) < 0)
    {
        return -1;
    }

    if (xml.enterbranch(SaveToFileSerializer::WORKSPACE_BRANCH_NAME) == 0)
    {
        return -10;
    }

    if (xml.enterbranch(MixerSerializer::BRANCH_NAME) == 0)
    {
        return -20;
    }

    MixerSerializer(mixer).Deserialize(&xml);

    xml.exitbranch();

    if (xml.enterbranch(RegionsSerializer::BRANCH_NAME) == 0)
    {
        return -30;
    }

    RegionsSerializer(regions).Deserialize(&xml);

    xml.exitbranch();

    xml.exitbranch();

    return 0;
}
