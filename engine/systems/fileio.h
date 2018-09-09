#ifndef FILEIO_H
#define FILEIO_H

#include <vector>
#include <string>

namespace FileIOSystem
{
    std::vector<char> ReadFileToVector(const std::string &filename);
}



#endif