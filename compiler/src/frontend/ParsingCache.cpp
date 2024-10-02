#include "R-Sharp/frontend/ParsingCache.hpp"

#include <stdexcept>
#include <algorithm>
#include <filesystem>

bool ParsingCache::contains(std::string const& filename, std::string const& identifier) {
    std::string absolute_filename = std::filesystem::absolute(filename);


    return containsWildcard(absolute_filename) || containsNonWildcard(absolute_filename, identifier);
}
bool ParsingCache::containsWildcard(std::string const& filename) {
    std::string absolute_filename = std::filesystem::absolute(filename);

    // handle paths that are not identical but equivalent
    auto equivalentFind = std::find_if(
        filenamesToAlreadyImportedIdentifiers.begin(),
        filenamesToAlreadyImportedIdentifiers.end(),
        [&](auto const& other) {
            return std::filesystem::equivalent(std::filesystem::path(filename), std::filesystem::path(other.first));
        }
    );
    if (equivalentFind != filenamesToAlreadyImportedIdentifiers.end()) {
        for (auto equiv : (*equivalentFind).second) {
            if (containsWildcard(equiv)) {
                return true;
            }
        }
    }

    return wildcardIncludedFiles.count(absolute_filename);
}
bool ParsingCache::containsNonWildcard(std::string const& filename, std::string const& identifier) {
    std::string absolute_filename = std::filesystem::absolute(filename);
    try {
        auto const& ids = filenamesToAlreadyImportedIdentifiers.at(absolute_filename);
        return std::find(ids.begin(), ids.end(), identifier) != ids.end();
    }
    catch (std::out_of_range) {
        // handle paths that are not identical but equivalent
        auto equivalentFind = std::find_if(
            filenamesToAlreadyImportedIdentifiers.begin(),
            filenamesToAlreadyImportedIdentifiers.end(),
            [&](auto const& other) {
                return std::filesystem::equivalent(
                    std::filesystem::path(filename), std::filesystem::path(other.first)
                );
            }
        );
        if (equivalentFind != filenamesToAlreadyImportedIdentifiers.end()) {
            return containsNonWildcard((*equivalentFind).first, identifier);
        }

        return false;
    }
}
void ParsingCache::add(std::string const& filename, std::string const& identifier) {
    std::string absolute_filename = std::filesystem::absolute(filename);
    try {
        auto& ids = filenamesToAlreadyImportedIdentifiers.at(absolute_filename);
        ids.push_back(identifier);
    }
    catch (std::out_of_range) {
        filenamesToAlreadyImportedIdentifiers.insert({absolute_filename, std::vector{identifier}});
    }
}
void ParsingCache::addWildcard(std::string const& filename) {
    std::string absolute_filename = std::filesystem::absolute(filename);
    wildcardIncludedFiles.insert(absolute_filename);
}
