/*
  ZynAddSubFX - a software synthesizer

  XMLwrapper.h - XML wrapper
  Copyright (C) 2003-2005 Nasca Octavian Paul
  Copyright (C) 2009-2009 Mark McCurry
  Author: Nasca Octavian Paul
          Mark McCurry

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2 or later) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#ifndef IPRESETSSERIALIZER_H
#define IPRESETSSERIALIZER_H

#include <string>

class IPresetsSerializer
{
public:
    /**
     * Destructor*/
    virtual ~IPresetsSerializer() = 0;

    /**
     * Saves the XML to a file.
     * @param filename the name of the destination file.
     * @returns 0 if ok or -1 if the file cannot be saved.
     */
    virtual int saveXMLfile(const std::string &filename) const = 0;

    /**
     * Return XML tree as a string.
     * Note: The string must be freed with free() to deallocate
     * @returns a newly allocated NULL terminated string of the XML data.
     */
    virtual char *getXMLdata() const = 0;

    /**
     * Add simple parameter.
     * @param name The name of the mXML node.
     * @param val  The string value of the mXml node
     */
    virtual void addpar(const std::string &name, int val) = 0;

    /**
     * Add simple parameter.
     * @param name The name of the mXML node.
     * @param val  The string value of the mXml node
     */
    virtual void addparunsigned(const std::string &name, unsigned int val) = 0;

    /**
     * Adds a realtype parameter.
     * @param name The name of the mXML node.
     * @param val  The float value of the node.
     */
    virtual void addparreal(const std::string &name, float val) = 0;

    /**
     * Add boolean parameter.
     * \todo Fix this reverse boolean logic.
     * @param name The name of the mXML node.
     * @param val The boolean value of the node (0->"yes" = 0;else->"no").
     */
    virtual void addparbool(const std::string &name, int val) = 0;

    /**
     * Add string parameter.
     * @param name The name of the mXML node.
     * @param val  The string value of the node.
     */
    virtual void addparstr(const std::string &name, const std::string &val) = 0;

    /**
     * Create a new branch.
     * @param name Name of new branch
     * @see void endbranch()
     */
    virtual void beginbranch(const std::string &name) = 0;
    /**
     * Create a new branch.
     * @param name Name of new branch
     * @param id "id" value of branch
     * @see void endbranch()
     */
    virtual void beginbranch(const std::string &name, int id) = 0;

    /**
     * Closes new branches.
     * This must be called to exit each branch created by beginbranch( ).
     * @see void beginbranch(const std::string &name)
     * @see void beginbranch(const std::string &name, int id)
     */
    virtual void endbranch() = 0;

    /**
     * Loads file into XMLwrapper.
     * @param filename file to be loaded
     * @returns 0 if ok or -1 if the file cannot be loaded
     */
    virtual int loadXMLfile(const std::string &filename) = 0;

    /**
     * Loads string into XMLwrapper.
     * @param xmldata NULL terminated string of XML data.
     * @returns true if successful.
     */
    virtual bool putXMLdata(const char *xmldata) = 0;

    /**
     * Enters the branch.
     * @param name Name of branch.
     * @returns 1 if is ok, or 0 otherwise.
     */
    virtual int enterbranch(const std::string &name) = 0;

    /**
     * Enter into the branch \c name with id \c id.
     * @param name Name of branch.
     * @param id Value of branch's "id".
     * @returns 1 if is ok, or 0 otherwise.
     */
    virtual int enterbranch(const std::string &name, int id) = 0;

    /**
     * Exits from a branch*/
    virtual void exitbranch() = 0;

    /**
     * Get the the branch_id and limits it between the min and max.
     * if min==max==0, it will not limit it
     * if there isn't any id, will return min
     * this must be called only imediately after enterbranch()
     */
    virtual int getbranchid(int min, int max) const = 0;

    /**
     * Returns the integer value stored in node name.
     * It returns the integer value between the limits min and max.
     * If min==max==0, then the value will not be limited.
     * If there is no location named name, then defaultpar will be returned.
     * @param name The parameter name.
     * @param defaultpar The default value if the real value is not found.
     * @param min The minimum return value.
     * @param max The maximum return value.
     */
    virtual int getpar(const std::string &name, int defaultpar, int min, int max) const = 0;

    /**
     * Returns the integer value stored in node name.
     * It returns the integer value between the limits min and max.
     * If min==max==0, then the value will not be limited.
     * If there is no location named name, then defaultpar will be returned.
     * @param name The parameter name.
     * @param defaultpar The default value if the real value is not found.
     * @param min The minimum return value.
     * @param max The maximum return value.
     */
    virtual unsigned int getparunsigned(const std::string &name, unsigned int defaultpar, unsigned int min, unsigned int max) const = 0;

    /**
     * Returns the integer value stored in the node with range [0,127].
     * @param name The parameter name.
     * @param defaultpar The default value if the real value is not found.
     */
    virtual unsigned char getpar127(const std::string &name, int defaultpar) const = 0;

    /**
     * Returns the boolean value stored in the node.
     * @param name The parameter name.
     * @param defaultpar The default value if the real value is not found.
     */
    virtual int getparbool(const std::string &name, int defaultpar) const = 0;

    /**
     * Get the string value stored in the node.
     * @param name The parameter name.
     * @param par  Pointer to destination string
     * @param maxstrlen Max string length for destination
     */
    virtual void getparstr(const std::string &name, char *par, int maxstrlen) const = 0;

    /**
     * Get the string value stored in the node.
     * @param name The parameter name.
     * @param defaultpar The default value if the real value is not found.
     */
    virtual std::string getparstr(const std::string &name, const std::string &defaultpar) const = 0;

    /**
     * Returns the real value stored in the node.
     * @param name The parameter name.
     * @param defaultpar The default value if the real value is not found.
     */
    virtual float getparreal(const std::string &name, float defaultpar) const = 0;

    /**
     * Returns the real value stored in the node.
     * @param name The parameter name.
     * @param defaultpar The default value if the real value is not found.
     * @param min The minimum value
     * @param max The maximum value
     */
    virtual float getparreal(const std::string &name, float defaultpar, float min, float max) const = 0;

    bool minimal; /**<false if all parameters will be stored (used only for clipboard)*/

    /**
     * Sets the current tree's PAD Synth usage
     */
    virtual void setPadSynth(bool enabled) = 0;
    /**
     * Checks the current tree for PADsynth usage
     */
    virtual bool hasPadSynth() const = 0;

};

#endif // IPRESETSSERIALIZER_H
