#ifndef SAVE_TO_FILE_SERILIAZER_H
#define SAVE_TO_FILE_SERILIAZER_H

#include <string>

class SaveToFileSerializer
{
public:
    int SaveTrack(
        class Track *track,
        std::string const &filename);

    int LoadTrack(
        class Track *track,
        std::string const &filename);

    int LoadTrackFromData(
        class Track *track,
        const char *data);

    int SaveMixer(
        class Mixer *mixer,
        std::string const &filename);

    int LoadMixer(
        class Mixer *mixer,
        std::string const &filename);

    int SaveWorkspace(
        class Mixer *mixer,
        class RegionsManager *regions,
        std::string const &filename);

    int LoadWorkspace(
        class Mixer *mixer,
        class RegionsManager *regions,
        std::string const &filename);

    static char const *WORKSPACE_BRANCH_NAME;
};

#endif // SAVE_TO_FILE_SERILIAZER_H
