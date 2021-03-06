#pragma once

#include "blackhole/sink/file.hpp"

#include <fstream>
#include <limits>
#include <map>
#include <mutex>

#include <boost/assert.hpp>

#include "blackhole/cpp17/string_view.hpp"

namespace blackhole {
inline namespace v1 {
namespace sink {
namespace file {

struct backend_t {
    std::size_t counter;
    std::size_t interval;
    std::unique_ptr<std::ofstream> stream;

    backend_t(const std::string& filename, std::size_t interval) :
        counter(0),
        interval(interval),
        stream(new std::ofstream)
    {
        BOOST_ASSERT(interval > 0);

        stream->exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            stream->open(filename, std::ios::app);
        } catch (const std::system_error& err) {
            // Transform unspecified ios category into the system one to obtain readable message.
            throw std::system_error(err.code().value(), std::system_category());
        } catch (const std::exception&) {
            throw std::system_error(errno, std::system_category());
        }
    }

    auto write(const string_view& message) -> void {
        stream->write(message.data(), static_cast<std::streamsize>(message.size()));
        stream->put('\n');

        counter = (counter + 1) % interval;

        if (counter == 0) {
            flush();
        }
    }

    auto flush() -> void {
        stream->flush();
    }
};

class inner_t {
    struct {
        std::string filename;
        std::size_t interval;
        std::map<std::string, backend_t> backends;
    } data;

public:
    std::mutex mutex;

    inner_t(std::string filename, std::size_t interval) {
        data.filename = std::move(filename);
        data.interval = interval > 0 ? interval : std::numeric_limits<std::size_t>::max();
    }

    virtual ~inner_t() {}

    auto path() const noexcept -> const std::string& {
        return data.filename;
    }

    auto interval() const noexcept -> std::size_t {
        return data.interval;
    }

    virtual auto filename(const record_t&) const -> std::string {
        // TODO: Generate path from tokens, for now just return static path.
        return {data.filename};
    }

    virtual auto backend(const std::string& filename) -> backend_t& {
        const auto it = data.backends.find(filename);

        if (it == data.backends.end()) {
            return data.backends.insert(it, std::make_pair(filename, backend_t(filename, interval())))->second;
        }

        return it->second;
    }
};

}  // namespace file
}  // namespace sink
}  // namespace v1
}  // namespace blackhole
