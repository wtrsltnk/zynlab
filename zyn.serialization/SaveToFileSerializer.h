#ifndef SAVE_TO_FILE_SERILIAZER_H
#define SAVE_TO_FILE_SERILIAZER_H

#include <string>

class SaveToFileSerializer
{
public:
    //saves the instrument settings to a XML file
    //returns 0 for ok or <0 if there is an error
    int SaveTrack(class Track *track, std::string const &filename);
    int LoadTrack(class Track *track, std::string const &filename);

    /**Saves all settings to a XML file
         * @return 0 for ok or <0 if there is an error*/
    int SaveMixer(class Mixer *mixer, std::string const &filename);

    /**loads all settings from a XML file
         * @return 0 for ok or -1 if there is an error*/
    int LoadMixer(class Mixer *mixer, std::string const &filename);
};

#endif // SAVE_TO_FILE_SERILIAZER_H
