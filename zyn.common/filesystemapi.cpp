#include "filesystemapi.h"

#include <windows.h>

namespace FilesystemApi
{

#ifdef _WIN32
    char PathDirectorySeparatorChar = '\\';
    char PathAltDirectorySeparatorChar = '/';
    char PathInvalidPathChars[] = {'\"', '<', '>', '|', '\0',
                                   static_cast<char>(1), static_cast<char>(2),
                                   static_cast<char>(3), static_cast<char>(4),
                                   static_cast<char>(5), static_cast<char>(6),
                                   static_cast<char>(7), static_cast<char>(8),
                                   static_cast<char>(9), static_cast<char>(10),
                                   static_cast<char>(11), static_cast<char>(12),
                                   static_cast<char>(13), static_cast<char>(14),
                                   static_cast<char>(15), static_cast<char>(16),
                                   static_cast<char>(17), static_cast<char>(18),
                                   static_cast<char>(19), static_cast<char>(20),
                                   static_cast<char>(21), static_cast<char>(22),
                                   static_cast<char>(23), static_cast<char>(24),
                                   static_cast<char>(25), static_cast<char>(26),
                                   static_cast<char>(27), static_cast<char>(28),
                                   static_cast<char>(29), static_cast<char>(30),
                                   static_cast<char>(31)};
    char PathVolumeSeparatorChar = ':';
#else
    char PathDirectorySeparatorChar = '/';
    char PathAltDirectorySeparatorChar = '\\';
    char PathInvalidPathChars[] = {'\"', '<', '>', '|', '\0',
                                   static_cast<char>(1), static_cast<char>(2),
                                   static_cast<char>(3), static_cast<char>(4),
                                   static_cast<char>(5), static_cast<char>(6),
                                   static_cast<char>(7), static_cast<char>(8),
                                   static_cast<char>(9), static_cast<char>(10),
                                   static_cast<char>(11), static_cast<char>(12),
                                   static_cast<char>(13), static_cast<char>(14),
                                   static_cast<char>(15), static_cast<char>(16),
                                   static_cast<char>(17), static_cast<char>(18),
                                   static_cast<char>(19), static_cast<char>(20),
                                   static_cast<char>(21), static_cast<char>(22),
                                   static_cast<char>(23), static_cast<char>(24),
                                   static_cast<char>(25), static_cast<char>(26),
                                   static_cast<char>(27), static_cast<char>(28),
                                   static_cast<char>(29), static_cast<char>(30),
                                   static_cast<char>(31)};
    char PathVolumeSeparatorChar = ':';
#endif

    bool PathIsPathRooted(std::string const &path)
    {
        auto length = path.length();

        // path starts with directory seperator
        if (length >= 1 && (path[0] == PathDirectorySeparatorChar || path[0] == PathAltDirectorySeparatorChar)) return true;

        // path starts with drive
        if (length >= 2 && (path[1] == PathVolumeSeparatorChar)) return true;

        return false;
    }

    std::string PathCombine(std::string const &path1, std::string const &path2)
    {
        //  If path2 includes a root, path2 is returned
        if (PathIsPathRooted(path2)) return path2;

        if (path1.length() == 0) return path2;

        std::string _path1(path1);
        std::string _path2(path2);

        if (_path1[_path1.length() - 1] != PathDirectorySeparatorChar && _path1[_path1.length() - 1] != PathAltDirectorySeparatorChar && _path1[_path1.length() - 1] != PathVolumeSeparatorChar)
            _path1 += PathDirectorySeparatorChar;

        return _path1 + _path2;
    }

    bool DirectoryExists(std::string const &path)
    {
#ifdef _WIN32
        DWORD attr = GetFileAttributes(path.c_str());
        if (attr == INVALID_FILE_ATTRIBUTES)
        {
            return false; //something is wrong with your path!
        }

        if (attr & FILE_ATTRIBUTE_DIRECTORY)
        {
            return true; // this is a directory!
        }
#endif // _WIN32

        return false; // this is not a directory!
    }

    std::vector<std::string> DirectoryGetFiles(std::string const &path, std::string const &searchPattern)
    {
        std::vector<std::string> files;

#ifdef _WIN32
        HANDLE hFind;
        WIN32_FIND_DATA data;

        auto tmp = FilesystemApi::PathCombine(path, searchPattern);
        hFind = FindFirstFile(tmp.c_str(), &data);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    files.push_back(FilesystemApi::PathCombine(path, data.cFileName));
                }
            } while (FindNextFile(hFind, &data));
            FindClose(hFind);
        }
#endif // _WIN32

        return files;
    }

    // Returns the names of files (including their paths) that match the specified search pattern in the specified directory.
    std::vector<std::string> DirectoryGetDirectories(std::string const &path, std::string const &searchPattern)
    {
        std::vector<std::string> files;

#ifdef _WIN32
        HANDLE hFind;
        WIN32_FIND_DATA data;

        hFind = FindFirstFileA(PathCombine(path, searchPattern).c_str(), &data);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (data.cFileName[0] == '.' && data.cFileName[1] == '\0') continue;
                if (data.cFileName[0] == '.' && data.cFileName[1] == '.') continue;

                if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    files.push_back(PathCombine(path, data.cFileName));
                }
            } while (FindNextFile(hFind, &data));
            FindClose(hFind);
        }
#endif // _WIN32

        return files;
    }

    std::string PathGetFileName(std::string const &path)
    {
        std::string seperators;
        seperators += PathDirectorySeparatorChar;
        seperators += PathAltDirectorySeparatorChar;

        auto lastSeperator = path.find_last_of(seperators);

        if (lastSeperator != std::string::npos)
        {
            auto result = path.substr(lastSeperator + 1);
            return result;
        }

        return path;
    }

    std::string PathGetExtension(std::string const &path)
    {
        auto filename = PathGetFileName(path);
        auto lastSeperator = filename.find_last_of(".");

        if (lastSeperator != std::string::npos)
        {
            return filename.substr(lastSeperator);
        }

        return "";
    }

} // namespace FilesystemApi
