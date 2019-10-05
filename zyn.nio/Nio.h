#ifndef NIO_H
#define NIO_H

#include <set>
#include <string>
#include <zyn.common/IAudioGenerator.h>
#include <zyn.common/IMidiEventHandler.h>

/**Interface to Nio Subsystem
 *
 * Should be only externally included header */
class Nio
{
public:
    static bool Start(IAudioGenerator *audioGenerator, IMidiEventHandler *midiEventHandler);
    static void Stop();

    static void SetDefaultSource(const std::string &name);
    static void SetDefaultSink(const std::string &name);

    static bool SelectSource(const std::string &name);
    static bool SelectSink(const std::string &name);

    static std::set<std::string> GetSources();
    static std::set<std::string> GetSinks();

    static std::string GetSelectedSource();
    static std::string GetSelectedSink();

    //Get the prefered sample rate from jack (if running)
    static void preferedSampleRate(unsigned int &rate);

    static std::string defaultSource;
    static std::string defaultSink;
};

#endif
