#ifndef NIO_H
#define NIO_H

#include <string>
#include <set>

/**Interface to Nio Subsystem
 *
 * Should be only externally included header */
class Nio
{
public:
    static bool start(class IMixer* mixer);
    static void stop(void);

    static void setDefaultSource(const std::string& name);
    static void setDefaultSink(const std::string& name);

    static bool setSource(const std::string& name);
    static bool setSink(const std::string& name);

    static std::set<std::string> getSources(void);
    static std::set<std::string> getSinks(void);

    static std::string getSource(void);
    static std::string getSink(void);

    //Get the prefered sample rate from jack (if running)
    static void preferedSampleRate(unsigned &rate);

    static std::string defaultSource;
    static std::string defaultSink;
};

#endif
