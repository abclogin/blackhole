#pragma once

#include <array>
#include <iostream>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <boost/mpl/set.hpp>
#include <boost/optional.hpp>

#include "blackhole/attribute/set.hpp"
#include "blackhole/config.hpp"
#include "blackhole/detail/array.hpp"
#include "blackhole/detail/iterator/join.hpp"
#include "blackhole/detail/traits/tuple.hpp"
#include "blackhole/detail/traits/unique.hpp"

namespace blackhole {

namespace attribute {

template<class T>
struct extractor;

template<class... Args>
struct tuple_empty;

namespace compare_by {

struct name_t {
    const std::string& name;

    bool operator()(const set_t::value_type& v) const {
        return v.first == name;
    }
};

} // namespace

class set_view_t {
    template<class T> friend struct extractor;
    template<class... Args> friend struct tuple_empty;

public:
    typedef aux::iterator::join_t<set_t, true> const_iterator;

    struct internal_set_t { set_t v; };
    struct external_set_t { set_t v; };

private:
    internal_set_t internal;  // Severity, message, timestamp. Maybe pid, tid.
    external_set_t external;  // All other.

public:
    set_view_t() = default;

    set_view_t(set_t external, set_t&& internal) :
        internal({ std::move(internal) }),
        external({ std::move(external) })
    {}

    bool
    empty() const BLACKHOLE_NOEXCEPT {
        return internal.v.empty() && external.v.empty();
    }

    //! Message is the only(?) late attribute allowed to set internally.
    void message(const std::string& message) {
        internal.v.emplace_back("message", message);
    }

    void message(std::string&& message) {
        internal.v.emplace_back("message", std::move(message));
    }

    //! Intentionally allow to insert only into external attribute set.
    void insert(pair_t pair) {
        external.v.emplace_back(std::move(pair));
    }

    //! Intentionally allow to insert only into external attribute set.
    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last) {
        std::copy(first, last, std::back_inserter(external.v));
    }

    const_iterator begin() const BLACKHOLE_NOEXCEPT {
        return const_iterator(aux::make_array(&internal.v, &external.v));
    }

    const_iterator end() const BLACKHOLE_NOEXCEPT {
        return const_iterator::invalid(aux::make_array(&internal.v, &external.v));
    }

    boost::optional<const attribute_t&>
    find(const std::string& name) const BLACKHOLE_NOEXCEPT {
        const compare_by::name_t action { name };

        auto it = std::find_if(internal.v.begin(), internal.v.end(), action);
        if (it != internal.v.end()) {
            return it->second;
        }

        it = std::find_if(external.v.begin(), external.v.end(), action);
        if (it != external.v.end()) {
            return it->second;
        }

        return boost::none;
    }

    const attribute_t& at(const std::string& name) const {
        auto value = find(name);
        if (!value) {
            throw std::out_of_range(name);
        }

        return *value;
    }

    const set_t& partial() const {
        return external.v;
    }
};

} // namespace attribute

} // namespace blackhole
