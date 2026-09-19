#pragma once
#include "Geode/Enums.hpp"
#include <Geode/Geode.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <matjson.hpp>

#include "CommentTimestamp.hpp"
#include "TimeUtils.hpp"

using namespace geode::prelude;
struct IconData {
    int id=1;
    int type=0;
    int primaryColor=1;
    int secondaryColor=1;
    int glowColor=1;
    bool glow=false;
};
struct Reply {
    std::string content;
    int64_t author_id;
    std::string author_name;
    int64_t timestamp;
    std::string id;
    int64_t likes;
    int64_t reply_count;
    IconData icon;

    // misc
    bool from_comment=false;
    bool account_comment=false;
    std::string comment_timestamp;
    std::vector<Reply> replies;
    bool last=false;
    Reply* parent=nullptr;

    // Extra metadata carried over from GJComment when this Reply stands in for
    // the root comment. Only meaningful when from_comment is true.
    int64_t comment_timestamp_unix=0;   // exact upload time, 0 when unknown
    int comment_percentage=0;           // the "x%" GD shows on level comments
    int comment_mod_badge=0;            // 0 = none, 1 = mod, 2 = elder mod
    ccColor3B comment_color={255,255,255}; // moderator chat colour
    int64_t player_id=0;                // user ID, distinct from account ID
    int level_id=0;
    bool has_level_id=false;
};

template <>
struct matjson::Serialize<IconData>
{
    static Result<IconData> fromJson(matjson::Value const &value)
    {
        IconData icon = IconData();
        icon.id = value["id"].asInt().unwrapOr(0);
        icon.type = value["type"].asInt().unwrapOr(0);
        icon.primaryColor = value["primary_color"].asInt().unwrapOr(0);
        icon.secondaryColor = value["secondary_color"].asInt().unwrapOr(0);
        icon.glowColor = value["glow_color"].asInt().unwrapOr(0);
        icon.glow = value["glow"].asBool().unwrapOr(false);
        return Ok(icon);
    }
    static matjson::Value toJson(IconData const &value){
        auto obj = matjson::Value();
        obj["id"] = value.id;
        obj["type"] = value.type;
        obj["primary_color"] = value.primaryColor;
        obj["secondary_color"] = value.secondaryColor;
        obj["glow_color"] = value.glowColor;
        obj["glow"] = value.glow;
        return obj;
    }
};

template <>
struct matjson::Serialize<Reply>
{
    static Result<Reply> fromJson(matjson::Value const &value)
    {
        Reply reply = Reply();
        reply.content = value["content"].asString().unwrapOr("");
        reply.author_id = geode::utils::numFromString<int64_t>(value["author_id"].asString().unwrapOr("0")).unwrapOr(0);
        reply.author_name = value["author_name"].asString().unwrapOr("");
        reply.timestamp = geode::utils::numFromString<int64_t>(value["timestamp"].asString().unwrapOr("0")).unwrapOr(0);
        reply.id = value["id"].asString().unwrapOr("");
        reply.likes = value["likes"].asInt().unwrapOr(0);
        reply.reply_count = value["reply_count"].asInt().unwrapOr(0);
        if (value.contains("icon")){
            reply.icon = value["icon"].as<IconData>().unwrapOrDefault();
        }
        reply.replies = value["replies"].as<std::vector<Reply>>().unwrapOrDefault();
        return Ok(reply);
    }
    static matjson::Value toJson(Reply const &value){
        auto obj = matjson::Value();
        obj["content"] = value.content;
        obj["author_id"] = value.author_id;
        obj["author_name"] = value.author_name;
        obj["timestamp"] = value.timestamp;
        obj["id"] = value.id;
        obj["likes"] = value.likes;
        obj["reply_count"] = value.reply_count;
        if (value.icon.id != 0){
            obj["icon"] = matjson::Value(value.icon);
        }
        return obj;
    }
};

inline Reply replyFromComment(GJComment* comment,int replies=0){
    Reply reply = Reply();
    reply.from_comment = true;

    reply.content = comment->m_commentString;
    reply.author_id = comment->m_accountID;
    reply.author_name = comment->m_userName;
    reply.comment_timestamp = comment->m_uploadDate;
    reply.id = fmt::format("{}",comment->m_commentID);
    reply.likes = comment->m_likeCount;
    reply.reply_count = replies;

    // Exact upload time, recovered by the GJComment::create hook.
    reply.comment_timestamp_unix = getCommentExactTimestamp(comment);
    reply.comment_percentage = comment->m_percentage;
    reply.comment_mod_badge = comment->m_modBadge;
    reply.comment_color = comment->m_color;
    reply.player_id = comment->m_userID;
    reply.level_id = comment->m_levelID;
    reply.has_level_id = comment->m_hasLevelID;

    if (!comment->m_userScore) {
        reply.account_comment = true;
        return reply;
    }

    reply.icon.type = (int)comment->m_userScore->m_iconType;
    reply.icon.id = comment->m_userScore->m_iconID;
    reply.icon.primaryColor = comment->m_userScore->m_color1;
    reply.icon.secondaryColor = comment->m_userScore->m_color2;
    reply.icon.glowColor = comment->m_userScore->m_color2;
    reply.icon.glow = comment->m_userScore->m_special == 2;

    return reply;
}

inline std::string toAgoString(int timestamp) {
    auto const fmtPlural = [](auto count, auto unit) {
        if (count == 1) {
            return fmt::format("{} {} ago", count, unit);
        }
        return fmt::format("{} {}s ago", count, unit);
    };
    auto value = std::chrono::seconds(timestamp);
    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
    auto len = std::chrono::duration_cast<std::chrono::seconds>(now - value).count();
    if (len <= 0){
        return fmt::format("0 seconds ago");
    }
    if (len < 60) {
        return fmtPlural(len, "second");
    }
    len = std::chrono::duration_cast<std::chrono::minutes>(now - value).count();
    if (len < 60) {
        return fmtPlural(len, "minute");
    }
    len = std::chrono::duration_cast<std::chrono::hours>(now - value).count();
    if (len < 24) {
        return fmtPlural(len, "hour");
    }
    len = std::chrono::duration_cast<std::chrono::days>(now - value).count();
    if (len < 31) {
        return fmtPlural(len, "day");
    }
    len = std::chrono::duration_cast<std::chrono::weeks>(now - value).count();
    if (len < 4) {
        return fmtPlural(len, "week");
    }
    len = std::chrono::duration_cast<std::chrono::months>(now - value).count();
    if (len < 12) {
        return fmtPlural(len, "month");
    }
    len = std::chrono::duration_cast<std::chrono::years>(now - value).count();
    if (len >= 1) {
        return fmtPlural(len, "year");
    }
    return fmt::format("this is the secret string");
}

// --- date & id display -------------------------------------------------------

// The exact upload time of a reply in unix seconds, or 0 when it is unknown.
// Replies posted through the mod's own server always carry a millisecond
// timestamp; the root comment only has one when RobTop's server sent key 15.
inline int64_t replyUnixSeconds(Reply const& reply) {
    if (reply.from_comment) return reply.comment_timestamp_unix;
    return reply.timestamp / 1000;
}

// The "2 months ago" form, whichever source it has to come from.
inline std::string replyRelativeString(Reply const& reply) {
    if (!reply.from_comment) return toAgoString(reply.timestamp / 1000);
    if (!reply.comment_timestamp.empty()) return reply.comment_timestamp + " ago";
    // No relative string from the server either — fall back to the timestamp.
    if (ReplyTime::isPlausible(reply.comment_timestamp_unix)) {
        return toAgoString(reply.comment_timestamp_unix);
    }
    return "";
}

// What actually goes on the cell, per the "date-display" setting. Falls back to
// the relative form whenever no exact timestamp is available, so the label
// never ends up empty just because a server did not send one.
inline std::string replyDateString(Reply const& reply) {
    auto relative = replyRelativeString(reply);
    auto mode = Mod::get()->getSettingValue<std::string>("date-display");

    if (mode == "Relative") return relative;

    auto exact = ReplyTime::format(
        replyUnixSeconds(reply),
        Mod::get()->getSettingValue<bool>("date-include-time")
    );
    if (exact.empty()) return relative;

    if (mode == "Exact") return exact;
    if (relative.empty()) return exact;
    return fmt::format("{} | {}", exact, relative);
}

// Reply IDs coming from the mod's server are opaque strings of unknown length.
// Numeric comment IDs are short enough to print whole; anything long gets
// elided, with the full value still available in the info popup.
inline std::string shortReplyId(std::string const& id) {
    if (id.size() <= 14) return id;
    return fmt::format("{}...{}", id.substr(0, 6), id.substr(id.size() - 4));
}

// A username that is safe to render: the server can hand back an empty name for
// deleted or unregistered accounts, which would otherwise draw nothing at all.
inline std::string replyDisplayName(Reply const& reply) {
    if (!reply.author_name.empty() && reply.author_name != "-") return reply.author_name;
    if (reply.author_id > 0) return fmt::format("- (ID: {})", reply.author_id);
    if (reply.player_id > 0) return fmt::format("- (ID: {})", reply.player_id);
    return "-";
}

static constexpr const std::string_view SERVER_URL = "https://replies.cdc-sys.com";
static const std::string MOD_VERSION_HEADER = Mod::get()->getVersion().toVString();

struct CacheEntry {
    std::vector<Reply> replies;
    std::chrono::seconds time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
};

struct ReplyCache {
    int page=1;
    int max_pages=1;
    int total_replies=0;
    std::string message;
    std::map<int,CacheEntry> cached;
    std::chrono::seconds time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
};

static std::map<std::string,ReplyCache> g_replyCache = {};
static std::vector<std::string> g_votedOn = {};
#define VECTOR_HAS_ITEM(vec, item) std::find(vec.begin(), vec.end(), item) != vec.end()

static bool g_syncedIcons = false;

enum class ModerationPermissions {
    None = 0,
    CommentModeration = 1,
    UserModeration = 2,
    FullAccess = 3
};

static ModerationPermissions g_permissions = (ModerationPermissions)Mod::get()->getSavedValue<int64_t>("moderation_permissions");