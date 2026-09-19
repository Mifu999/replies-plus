#pragma once

// Date formatting helpers.
//
// Deliberately built on strftime rather than fmt/chrono so this compiles the
// same way on every platform Geode targets, without relying on which pieces of
// fmt happen to be bundled with the SDK.

#include <Geode/Geode.hpp>
#include <cstdint>
#include <ctime>
#include <string>

namespace ReplyTime {
    // 2010-01-01. Geometry Dash did not exist before this, so any "timestamp"
    // below it is not a timestamp. This guards against a field that turns out
    // to hold something else entirely rendering as a date in 1970.
    inline constexpr int64_t MIN_PLAUSIBLE_UNIX = 1262304000LL;

    inline bool isPlausible(int64_t unixSeconds) {
        return unixSeconds >= MIN_PLAUSIBLE_UNIX;
    }

    inline bool toTm(int64_t unixSeconds, bool utc, std::tm& out) {
        std::time_t t = static_cast<std::time_t>(unixSeconds);
#ifdef GEODE_IS_WINDOWS
        if (utc) return gmtime_s(&out, &t) == 0;
        return localtime_s(&out, &t) == 0;
#else
        if (utc) return gmtime_r(&t, &out) != nullptr;
        return localtime_r(&t, &out) != nullptr;
#endif
    }

    // "2026-09-19" or "2026-09-19 14:32"
    inline std::string format(int64_t unixSeconds, bool includeTime = false, bool utc = false) {
        if (!isPlausible(unixSeconds)) return "";

        std::tm tmv{};
        if (!toTm(unixSeconds, utc, tmv)) return "";

        char buf[64];
        char const* pattern = includeTime ? "%Y-%m-%d %H:%M" : "%Y-%m-%d";
        if (std::strftime(buf, sizeof(buf), pattern, &tmv) == 0) return "";
        return std::string(buf);
    }

    // "2026-09-19 14:32:07 UTC", used in the info popup where space is not tight
    inline std::string formatFull(int64_t unixSeconds, bool utc) {
        if (!isPlausible(unixSeconds)) return "";

        std::tm tmv{};
        if (!toTm(unixSeconds, utc, tmv)) return "";

        char buf[80];
        if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmv) == 0) return "";
        return std::string(buf) + (utc ? " UTC" : " (local)");
    }
}
