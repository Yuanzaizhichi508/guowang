#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

class MatchResult {
public:
    bool matched;
    std::string result;

    MatchResult(bool matched, std::string result) : matched(matched), result(result) {}
};

enum class MatchType {
    INSERTION,
    DELETION,
    REPLACEMENT,
    MATCH
};

class StampStringMatcher {
private:
    // 类别, 顺序对应id, 注意原始classes中不包含空格，初始化时空格会作为最后一个类别添加进去
    std::vector<std::string> classes;
    // 类别反向映射
    std::unordered_map<std::string, int> classesMap;
    // 类别距离, 非对称. distanceMap[i][j]表示识别为i实际为j的距离, 对角线为0, 其中最后一行和最后一列代表background，对应空格类别
    std::vector<std::vector<float>> distanceMap;
    // 距离阈值
    float distanceThreshold;
    // 距离差值阈值
    float distanceDiffThreshold;

    // 输入字符串分解匹配模式
    std::regex pattern = std::regex("( )|([A-Z0-9]\\_REV)|(\\{[^{}]+\\}\\_REV)|([A-Z0-9])|(\\{[^{}]+\\})");

    // 将字符串转换为id序列
    std::vector<int> StringToIds(const std::string& str);

    // 计算两个id序列的编辑距离，注意并非对称
    // @param rec: 待匹配序列
    // @param candidate: 候选序列
    // @return 编辑距禽
    float CalculateDistance(const std::vector<int>& rec, const std::vector<int>& candidate);

    // 计算两个id序列的编辑距离，注意并非对称，同时记录字符串差异
    // @param rec: 待匹配序列
    // @param candidate: 候选序列
    // @param diff: 字符串差异记录
    // @return 编辑距禽
    float CalculateDistance(const std::vector<int>& rec, const std::vector<int>& candidate, std::vector<MatchType>& diff);

    // 加载类别
    // @param classes_txt_path: 类别列表路径
    void LoadClasses(const std::string& classes_txt_path);

    // 根据混淆矩阵加载类别距离, 来自csv文件, 加载为二维数组
    // @param confusion_matrix_path: 类别距离路径
    void LoadDistanceMap(const std::string& confusion_matrix_path);

public:
    // 构造函数
    // @param classesTxtPath: 类别列表路径
    // @param confusionMatrixPath: 类别距离路径
    // @param distanceThreshold: 距离阈值
    // @param distanceDiffThreshold: 距离差值阈值
    StampStringMatcher(
        const std::string& classesTxtPath,
        const std::string& confusionMatrixPath,
        float distanceThreshold,
        float distanceDiffThreshold
    );
    
    // 匹配字符串
    // @param rec: 待匹配字符串
    // @param candidates: 候选字符串
    // @return 匹配结果
    MatchResult MatchString(const std::string& rec, const std::vector<std::string>& candidates);

    // 匹配字符串
    // @param rec: 待匹配id序列
    // @param candidates: 候选字符串
    // @return 匹配结果
    MatchResult MatchString(const std::vector<int>& rec, const std::vector<std::string>& candidates);
    
    // 匹配字符串, 同时显示字符串差异
    // @param rec: 待匹配字符串
    // @param candidates: 候选字符串
    // @param diff: 字符串差异记录
    // @return 匹配结果
    MatchResult MatchString(const std::string& rec, const std::vector<std::string>& candidates, std::vector<MatchType>& diff);
};