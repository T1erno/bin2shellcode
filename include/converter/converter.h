#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>
#include <vector>

namespace b2s {

std::vector<unsigned char> readFile(const std::string& path);
std::string convert(const std::vector<unsigned char>& buf, int fmt,
                    const std::string& varName);

}

#endif
