#include "RegionsSerializer.h"

char const *RegionsSerializer::BRANCH_NAME = "REGIONS";

RegionsSerializer::RegionsSerializer(
    RegionsManager *regions)
    : _regions(regions)
{}

RegionsSerializer::~RegionsSerializer() = default;

void RegionsSerializer::Serialize(
    IPresetsSerializer *xml)
{
}

void RegionsSerializer::Deserialize(
    IPresetsSerializer *xml)
{
}
