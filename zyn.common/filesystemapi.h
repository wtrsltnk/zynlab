#ifndef FILESYSTEMAPI_H
#define FILESYSTEMAPI_H

#include <string>
#include <vector>

namespace FilesystemApi
{
    bool DirectoryExists(std::string const &path);
    std::vector<std::string> DirectoryGetDirectories(std::string const &path, std::string const &searchPattern);
    std::vector<std::string> DirectoryGetFiles(std::string const &path, std::string const &searchPattern);
    std::string PathGetFileName(std::string const &path);
    std::string PathCombine(std::string const &path1, std::string const &path2);
    std::string PathGetExtension(std::string const &path);

}; // namespace FilesystemApi

#endif // FILESYSTEMAPI_H
