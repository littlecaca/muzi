#include <iostream>
#include <set>
#include <memory>

// Custom comparator that allows heterogeneous comparison
struct Compare {
    using is_transparent = void; // Enables heterogeneous lookup

    bool operator()(const std::unique_ptr<int>& lhs, const std::unique_ptr<int>& rhs) const {
        return *lhs < *rhs;
    }

    bool operator()(const std::unique_ptr<int>& lhs, int rhs) const {
        return *lhs < rhs;
    }

    bool operator()(int lhs, const std::unique_ptr<int>& rhs) const {
        return lhs < *rhs;
    }
};

int main() {
    std::unique_ptr<int> up(new int(2));

    std::unique_ptr<int> up2(std::move(up));
    typedef std::unique_ptr<int> Entry;
    typedef std::set<Entry> EntrySet;

    EntrySet entry_set;

    Entry e1;
    Entry e2(std::move(e1));
    const Entry &e3 = e1;

    Entry e4(std::move(const_cast<Entry &>(e3)));

    for (auto it = entry_set.begin(); it != entry_set.end(); ++it)
    {
        typedef decltype(*it)  ss;
        // Entry myentry(std::move(*it));
    }



    return 0;
}
