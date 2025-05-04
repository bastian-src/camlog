#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

void save_as(const std::string &filename, const std::vector<uint8_t> &data) {
    std::ofstream outFile;
    outFile.open(filename, std::ios::binary| std::ios::trunc);
    outFile.write(reinterpret_cast<const char*>(data.data()), data.size());
    outFile.close();
}

