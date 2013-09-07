#include "caching/dblru_cache.h"

namespace meta {
namespace caching {

template <class Key, class Value, template <class, class> class Map>
dblru_cache<Key, Value, Map>::dblru_cache(uint64_t max_size)
    : max_size_{max_size}, current_size_{0} { /* nothing */ }

template <class Key, class Value, template <class, class> class Map>
dblru_cache<Key, Value, Map>::dblru_cache(dblru_cache && other)
    : max_size_{std::move(other.max_size_)},
      current_size_{other.current_size_.load()},
      primary_{std::move(other.primary_)},
      secondary_{std::move(other.secondary_)} { /* nothing */ }


template <class Key, class Value, template <class, class> class Map>
dblru_cache<Key, Value, Map> &
dblru_cache<Key, Value, Map>::operator=(dblru_cache rhs) {
    swap(rhs);
    return *this;
}

template <class Key, class Value, template <class, class> class Map>
void dblru_cache<Key, Value, Map>::swap(dblru_cache & other) {
    std::swap(max_size_, other.max_size_);
    current_size_.store(other.current_size_.exchange(current_size_.load()));
    std::swap(primary_, other.primary_);
    std::swap(secondary_, other.secondary_);
}

template <class Key, class Value, template <class, class> class Map>
void
dblru_cache<Key, Value, Map>::insert(const Key & key, const Value & value) {
    primary_.insert(key, value);
    handle_insert();
}

template <class Key, class Value, template <class, class> class Map>
template <class... Args>
void dblru_cache<Key, Value, Map>::emplace(Args &&... args) {
    primary_.emplace(std::forward<Args...>(args...));
    handle_insert();
}

template <class Key, class Value, template <class, class> class Map>
util::optional<Value> dblru_cache<Key, Value, Map>::find(const Key & key) {
    auto opt = primary_.find(key);
    if(opt)
        return opt;
    opt = secondary_.find(key);
    if(opt) {
        primary_.insert(key, *opt);
        handle_insert();
    }
    return opt;
}

template <class Key, class Value, template <class, class> class Map>
void dblru_cache<Key, Value, Map>::handle_insert() {
    if(current_size_.fetch_add(1) == max_size_) {
        // leaves primary_ empty, with secondary_ containing what used to
        // be in primary_
        secondary_ = std::move(primary_);
        // reset counter
        current_size_.store(0);
    }
}

}
}
