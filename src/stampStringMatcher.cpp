#include "stampStringMatcher.h"
#include <fstream>
#include <regex>
#include <iostream>
#include "timeUtil.hpp"
// #include <iomanip>
// #include <opencv2/opencv.hpp>

StampStringMatcher::StampStringMatcher(
    const std::string& classesTxtPath,
    const std::string& confusionMatrixPath,
    float distanceThreshold,
    float distanceDiffThreshold
) : 
    distanceThreshold(distanceThreshold), 
    distanceDiffThreshold(distanceDiffThreshold) 
{
    // 加载类别
    LoadClasses(classesTxtPath);
    // 加载类别距离
    LoadDistanceMap(confusionMatrixPath);
    // 添加空格类别
    this->classes.push_back(" ");
    // 初始化类别映射
    for (int i = 0; i < this->classes.size(); i++) {
        classesMap[this->classes[i]] = i;
    }
};

void StampStringMatcher::LoadClasses(const std::string& classesTxtPath) {
    std::ifstream file(classesTxtPath);
    std::string line;
    while (std::getline(file, line)) {
        classes.push_back(line);
    }
}

void StampStringMatcher::LoadDistanceMap(const std::string& confusionMatrixPath) {
    std::ifstream file(confusionMatrixPath);
    if (!file.is_open()) {
        Log_Warning("Failed to open confusion matrix file: " + confusionMatrixPath);
        Log_Warning("Use default distance map");
        // 默认距离矩阵, 即对角线为0，其余为1
        distanceMap.resize(classes.size(), std::vector<float>(classes.size(), 1.0f));
        for (int i = 0; i < classes.size(); i++) {
            distanceMap[i][i] = 0;
        }
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::vector<float> row;
        std::string num;
        for (char c : line) {
            if (c == ',') {
                row.push_back(1.0f - std::stof(num));
                num.clear();
            } else {
                num.push_back(c);
            }
        }
        row.push_back(1.0f - std::stof(num));
        distanceMap.push_back(row);
    }
    file.close();
    // 处理一下对角线，将其置为0
    for (int i = 0; i < distanceMap.size(); i++) {
        distanceMap[i][i] = 0;
    }
    // // 将distanceMap转换为灰度矩阵图并显示
    // cv::Mat distanceMat(distanceMap.size(), distanceMap[0].size(), CV_32FC1);
    // for (int i = 0; i < distanceMap.size(); i++) {
    //     for (int j = 0; j < distanceMap[i].size(); j++) {
    //         distanceMat.at<float>(i, j) = distanceMap[i][j];
    //     }
    // }
    // cv::Mat distanceMatGray;
    // cv::normalize(distanceMat, distanceMatGray, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    // cv::applyColorMap(distanceMatGray, distanceMatGray, cv::COLORMAP_JET);
    // cv::resize(distanceMatGray, distanceMatGray, cv::Size(5 * distanceMap.size(), 5 * distanceMap.size()), 0, 0, cv::INTER_NEAREST);
    // cv::imshow("distanceMat", distanceMatGray);
    // cv::waitKey(0);
}

std::vector<int> StampStringMatcher::StringToIds(const std::string& str) {
    std::vector<int> ids;
    // 字符元素可能为数字、大写字母、由{}包裹的字符串、前几种元素加上_REV标记以及横杠和空格
    std::sregex_iterator begin(str.begin(), str.end(), pattern);
    std::sregex_iterator end;

    while (begin != end) {
        std::smatch match = *begin;
        ids.push_back(classesMap[match.str()]);
        begin++;
    }

    return ids;
}

float StampStringMatcher::CalculateDistance(const std::vector<int>& rec, const std::vector<int>& candidate) {
    std::vector<std::vector<float>> dp(rec.size() + 1, std::vector<float>(candidate.size() + 1, INT16_MAX));
    dp[0][0] = 0;
    for (int i = 1; i <= rec.size(); i++) {
        dp[i][0] = dp[i - 1][0] + (rec[i - 1] == classesMap[" "] ? 1 : distanceMap[rec[i - 1]][classesMap[" "]]);
    }
    for (int j = 1; j <= candidate.size(); j++) {
        dp[0][j] = dp[0][j - 1] + distanceMap[classesMap[" "]][candidate[j - 1]];
    }
    for (int i = 1; i <= rec.size(); i++) {
        for (int j = 1; j <= candidate.size(); j++) {
            // 三种情况，插入、删除、替换
            dp[i][j] = std::min(
                std::min(
                    // 插入, 即rec[i - 1]匹配空格, 代表此处没有字符但识别出了结果, 使用空格距离，即误识别代价
                    // 注意若rec[i - 1]本身识别出空格，并不能直接使用空格距离，因为此时代表识别出了空格，而非未识别
                    dp[i - 1][j] + (rec[i - 1] == classesMap[" "] ? 1 : distanceMap[rec[i - 1]][classesMap[" "]]),
                    // 删除, 即candidate[j - 1]匹配空格, 使用空格距离，即未识别代价
                    dp[i][j - 1] + (candidate[j - 1] == classesMap[" "] ? 1 : distanceMap[classesMap[" "]][candidate[j - 1]])
                ),
                dp[i - 1][j - 1] + distanceMap[rec[i - 1]][candidate[j - 1]]
            );        
        }
    }
    return dp[rec.size()][candidate.size()];
}

MatchResult StampStringMatcher::MatchString(const std::string& rec, const std::vector<std::string>& candidates) {
    std::vector<int> recIds = StringToIds(rec);
    float minDistance = INT16_MAX;
    float secondMinDistance = INT16_MAX;
    std::string result;
    for (const std::string& candidate : candidates) {
        std::vector<int> candidateIds = StringToIds(candidate);
        float distance = CalculateDistance(recIds, candidateIds);
#ifdef DEBUG
        std::cout << "rec: " << rec << std::endl;
        std::cout << "candidate: " << candidate << std::endl;
        std::cout << "distance: " << distance << std::endl;
#endif
        if (distance < minDistance) {
            secondMinDistance = minDistance;
            minDistance = distance;
            result = candidate;
        } else if (distance < secondMinDistance) {
            secondMinDistance = distance;
        }
    }
    if (minDistance <= distanceThreshold && (secondMinDistance - minDistance) >= distanceDiffThreshold) {
        return MatchResult(true, result);
    }
    return MatchResult(false, "");
}

MatchResult StampStringMatcher::MatchString(const std::vector<int>& rec, const std::vector<std::string>& candidates) {
    float minDistance = INT16_MAX;
    float secondMinDistance = INT16_MAX;
    std::string result;
    for (const std::string& candidate : candidates) {
        std::vector<int> candidateIds = StringToIds(candidate);
        float distance = CalculateDistance(rec, candidateIds);
        if (distance < minDistance) {
            secondMinDistance = minDistance;
            minDistance = distance;
            result = candidate;
        } else if (distance < secondMinDistance) {
            secondMinDistance = distance;
        }
    }
    if (minDistance <= distanceThreshold && (secondMinDistance - minDistance) >= distanceDiffThreshold) {
        return MatchResult(true, result);
    }
    return MatchResult(false, result);
}

float StampStringMatcher::CalculateDistance(const std::vector<int>& rec, const std::vector<int>& candidate, std::vector<MatchType>& diff) {
    std::vector<std::vector<float>> dp(rec.size() + 1, std::vector<float>(candidate.size() + 1, INT16_MAX));
    std::vector<std::vector<MatchType>> path(rec.size() + 1, std::vector<MatchType>(candidate.size() + 1, MatchType::MATCH));
    dp[0][0] = 0;
    for (int i = 1; i <= rec.size(); i++) {
        dp[i][0] = dp[i - 1][0] + (rec[i - 1] == classesMap[" "] ? 1 : distanceMap[rec[i - 1]][classesMap[" "]]);
        path[i][0] = MatchType::INSERTION;
    }
    for (int j = 1; j <= candidate.size(); j++) {
        dp[0][j] = dp[0][j - 1] + distanceMap[classesMap[" "]][candidate[j - 1]];
        path[0][j] = MatchType::DELETION;
    }
    for (int i = 1; i <= rec.size(); i++) {
        for (int j = 1; j <= candidate.size(); j++) {
            // 三种情况，插入、删除、替换(特定情况下是匹配)
            float insert = dp[i - 1][j] + (rec[i - 1] == classesMap[" "] ? 1 : distanceMap[rec[i - 1]][classesMap[" "]]);
            float del = dp[i][j - 1] + (candidate[j - 1] == classesMap[" "] ? 1 : distanceMap[classesMap[" "]][candidate[j - 1]]);
            float replace = dp[i - 1][j - 1] + distanceMap[rec[i - 1]][candidate[j - 1]];
            if (insert < del && insert < replace) {
                dp[i][j] = insert;
                path[i][j] = MatchType::INSERTION;
            } else if (del < insert && del < replace) {
                dp[i][j] = del;
                path[i][j] = MatchType::DELETION;
            } else {
                dp[i][j] = replace;
                path[i][j] = rec[i - 1] == candidate[j - 1] ? MatchType::MATCH : MatchType::REPLACEMENT;
            }
        }
    }
    // 回溯路径
    int i = rec.size();
    int j = candidate.size();
    while (i > 0 || j > 0) {
        // std::cout << i << " " << j << " " << classes[rec[i - 1]] << " " << classes[candidate[j - 1]] << " ";
        // switch (path[i][j])
        // {
        //     case MatchType::MATCH:
        //         std::cout << " ";
        //         break;
        //     case MatchType::INSERTION:
        //         std::cout << "+";
        //         break;
        //     case MatchType::DELETION:
        //         std::cout << "-";
        //         break;
        //     case MatchType::REPLACEMENT:
        //         std::cout << "*";
        //         break;
        //     default:
        //         break;
        // }
        // std::cout << std::endl;
        if (path[i][j] == MatchType::INSERTION) {
            diff.push_back(MatchType::INSERTION);
            i--;
        } else if (path[i][j] == MatchType::DELETION) {
            diff.push_back(MatchType::DELETION);
            j--;
        } else {
            diff.push_back(path[i][j]);
            i--;
            j--;
        }
    }
    std::reverse(diff.begin(), diff.end());
    return dp[rec.size()][candidate.size()];
}

MatchResult StampStringMatcher::MatchString(const std::string& rec, const std::vector<std::string>& candidates, std::vector<MatchType>& diff) {
    std::vector<int> recIds = StringToIds(rec);
    float minDistance = INT16_MAX;
    float secondMinDistance = INT16_MAX;
    std::string result;
    for (const std::string& candidate : candidates) {
        std::vector<int> candidateIds = StringToIds(candidate);
        std::vector<MatchType> tmpDiff(0);
        float distance = CalculateDistance(recIds, candidateIds, tmpDiff);
        if (distance < minDistance) {
            secondMinDistance = minDistance;
            minDistance = distance;
            result = candidate;
            diff = tmpDiff;
        } else if (distance < secondMinDistance) {
            secondMinDistance = distance;
        }
    }
    if (minDistance <= distanceThreshold && (secondMinDistance - minDistance) >= distanceDiffThreshold) {

        return MatchResult(true, result);
    }
    return MatchResult(false, result);
}