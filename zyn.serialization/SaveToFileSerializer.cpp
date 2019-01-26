#include "SaveToFileSerializer.h"

#include "MixerSerializer.h"
#include "TrackSerializer.h"
#include <zyn.common/PresetsSerializer.h>

int SaveToFileSerializer::LoadTrack(Track *track, const std::string &filename)
{
    PresetsSerializer xml;
    if (xml.loadXMLfile(filename) < 0)
    {
        return -1;
    }

    if (xml.enterbranch("INSTRUMENT") == 0)
    {
        return -10;
    }

    TrackSerializer(track).DeserializeInstrument(&xml);
    xml.exitbranch();

    return 0;
}

int SaveToFileSerializer::SaveTrack(Track *track, const std::string &filename)
{
    PresetsSerializer xml;

    xml.beginbranch("INSTRUMENT");
    TrackSerializer(track).SerializeInstrument(&xml);
    xml.endbranch();

    return xml.saveXMLfile(filename);
}

int SaveToFileSerializer::SaveMixer(class Mixer *mixer, std::string const &filename)
{
    PresetsSerializer xml;

    xml.beginbranch("MASTER");
    MixerSerializer(mixer).Serialize(&xml);
    xml.endbranch();

    return xml.saveXMLfile(filename);
}

int SaveToFileSerializer::LoadMixer(class Mixer *mixer, std::string const &filename)
{
    PresetsSerializer xml;
    if (xml.loadXMLfile(filename) < 0)
    {
        return -1;
    }

    if (xml.enterbranch("MASTER") == 0)
    {
        return -10;
    }

    MixerSerializer(mixer).Deserialize(&xml);
    xml.exitbranch();

    return 0;
}
