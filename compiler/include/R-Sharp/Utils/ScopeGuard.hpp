#pragma once

#include <functional>

class ScopeGuard {
public:
    ScopeGuard(std::function<void(void)> cleanupFunction) {
        this->cleanupFunction = cleanupFunction;
        this->isReleased = false;
    }

    ~ScopeGuard() {
        if (!isReleased) {
            cleanupFunction();
        }
    }

    ScopeGuard() = default;
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    void release() {
        isReleased = true;
    }

private:
    std::function<void(void)> cleanupFunction;
    bool isReleased = false;
};