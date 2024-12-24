#include <fstream>
#include <string>
#include <vector>

class TestStampDataset {
private:
    std::vector<std::pair<float, std::string>> dataset;

public:
    TestStampDataset(const std::string& datasetPath) {
        std::ifstream file(datasetPath);
        std::string line;
        // 格式为 length,stamp
        while (std::getline(file, line)) {
            std::string stamp;
            float length;
            auto pos = line.find(',');
            if (pos != std::string::npos) {
                length = std::stof(line.substr(0, pos));
                stamp = line.substr(pos + 1);
                dataset.push_back(std::make_pair(length, stamp));
            }
        }
        file.close();
    }

    // 根据长度查找对应的钢印字符串, 在一个范围内查找
    std::vector<std::string> FindStampString(float length, float threshold) {
        std::vector<std::string> result;
        // 首先找到下界位置
        auto lowerBound = std::lower_bound(dataset.begin(), dataset.end(), std::make_pair(length - threshold, std::string("")));
        // 从下界位置开始查找
        for (auto it = lowerBound; it != dataset.end(); it++) {
            std::cout << it->first << " " << it->second << std::endl;
            if (it->first > length + threshold) {
                break;
            }
            result.push_back(it->second);
        }
        return result;
    }
};