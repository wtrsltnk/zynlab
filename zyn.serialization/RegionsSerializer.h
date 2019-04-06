#ifndef REGIONSSERIALIZER_H
#define REGIONSSERIALIZER_H

#include <zyn.common/IPresetsSerializer.h>
#include <zyn.seq/RegionsManager.h>

class RegionsSerializer
{
    RegionsManager *_regions;

public:
    RegionsSerializer(RegionsManager *regions);
    virtual ~RegionsSerializer();

    void Serialize(IPresetsSerializer *xml);
    void Deserialize(IPresetsSerializer *xml);

    static char const *BRANCH_NAME;
};

#endif // REGIONSSERIALIZER_H
