#pragma once
#include <algorithm>

namespace o {

    template <typename Sequence, typename Count, typename... Args>
    void emplace_n(Sequence& seq, Count c, Args&&... args) {
        for (auto i = 0; i < c; ++i) {
            seq.emplace_back(std::forward<Args>(args)...);
        }
    }

} // namespace o
