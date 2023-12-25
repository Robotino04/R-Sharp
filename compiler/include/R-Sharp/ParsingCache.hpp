#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <memory>

#include "R-Sharp/AstNodesFWD.hpp"

class ParsingCache {
public:
    ParsingCache() {}

    bool contains(std::string const& filename, std::string const& identifier);
    bool containsWildcard(std::string const& filename);
    bool containsNonWildcard(std::string const& filename, std::string const& identifier);
    void add(std::string const& filename, std::string const& identifier);
    void addWildcard(std::string const& filename);

private:
    std::unordered_map<std::string, std::vector<std::string>> filenamesToAlreadyImportedIdentifiers;
    std::unordered_set<std::string> wildcardIncludedFiles;
};