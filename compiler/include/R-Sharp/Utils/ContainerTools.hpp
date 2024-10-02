#pragma once

#include <algorithm>
#include <vector>
#include <numeric>

namespace ContainerTools {

template <typename elementT, template <typename = elementT, typename...> typename containerT>
inline bool contains(containerT<elementT> const& container, elementT const& element) {
    return std::find(container.begin(), container.end(), element) != container.end();
}


template <typename T>
std::vector<T> flatten(std::vector<std::vector<T>> const& all) {
    std::vector<T> accum;

    accum.reserve(std::accumulate(all.begin(), all.end(), 0, [](size_t sizeUntilNow, auto const& range) {
        return range.size() + sizeUntilNow;
    }));

    for (auto const& sub : all)
        accum.insert(end(accum), begin(sub), end(sub));

    return accum;
}

}
