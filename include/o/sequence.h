#pragma once
#include <algorithm>

namespace o {

    /**
     * Emplace n elements to the back of a sequence
     *
     * @tparam  Sequence    Type of the sequence.
     * @tparam  Count       Type of the count.
     * @tparam  Args        Type of the arguments.
     * @param [in,out]  seq     The sequence.
     * @param           c       The Number of elements
     * @param           args    Variable arguments providing the
     *                          arguments to emplace_back on the sequence.
     */
    template <typename Sequence, typename Count, typename... Args>
    void emplace_n(Sequence& seq, Count c, Args&&... args) {
        for (auto i = 0; i < c; ++i) {
            seq.emplace_back(std::forward<Args>(args)...);
        }
    }

} // namespace o
