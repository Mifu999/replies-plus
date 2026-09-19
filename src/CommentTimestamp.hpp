#pragma once

// GJComment has no field for the upload timestamp — it only keeps the relative
// string RobTop's server sends in key 9 ("2 months"). The server does however
// also send an exact unix timestamp in the comment object under key 15, which
// the vanilla parser drops on the floor.
//
// CommentTimestamp.cpp hooks GJComment::create(CCDictionary*) and stashes that
// value on the node as a user object, so the rest of the mod can read it back.
//
// This approach is taken from BetterInfo (cvolton.betterinfo), which does the
// same thing for its "Show Exact Comment Dates" setting. Both mods hooking the
// same static create is fine — Geode chains the hooks.

#include <Geode/Geode.hpp>
#include <cstdint>

using namespace geode::prelude;

// The user-object key, "cdc.replies/exact-timestamp".
#define COMMENT_TIMESTAMP_KEY "exact-timestamp"_spr

// Returns the exact upload time of a comment in unix seconds, or 0 when the
// server did not give one (older GD servers, GDPSes, account comments, ...).
inline int64_t getCommentExactTimestamp(GJComment* comment) {
    if (!comment) return 0;
    if (auto stamp = typeinfo_cast<CCInteger*>(comment->getUserObject(COMMENT_TIMESTAMP_KEY))) {
        return static_cast<int64_t>(stamp->getValue());
    }
    return 0;
}
