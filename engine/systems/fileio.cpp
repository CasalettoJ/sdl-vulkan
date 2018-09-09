#include <vector>
#include <string>
#include <fstream>

#include "fileio.h"

std::vector<char> FileIOSystem::ReadFileToVector(const std::string &filename)
{
    std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    uint32_t size = file.tellg();
    std::vector<char> fileContents(size);
    file.seekg(0);
    file.read(fileContents.data(), size);
    file.close();

    return fileContents;
}