#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <chrono>

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

class TestRunner {
public:
    std::vector<TestResult> results;
    int passCount = 0;
    int failCount = 0;

    void Assert(bool condition, const std::string& name, const std::string& msg) {
        if (condition) passCount++;
        else failCount++;
        results.push_back({ name, condition, msg });
    }

    void Clear() {
        results.clear();
        passCount = 0;
        failCount = 0;
    }

    float GetPassPercentage() {
        if (results.empty()) return 0.0f;
        return (float)passCount / (float)results.size();
    }
};
