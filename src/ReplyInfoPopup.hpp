#pragma once

// The "Reply Info" popup.
//
// Everything the mod knows about a single reply, in one place: the full ID
// (which the cell has to elide when it is long), both the UTC and the local
// rendering of the exact timestamp, the author's account and player IDs, and
// the level the root comment lives on. Built on MDPopup so there is no layout
// code to maintain, with the second button dumping the whole thing to the
// clipboard as plain text.

#include <Geode/Geode.hpp>
#include <Geode/ui/MDPopup.hpp>
#include <Geode/utils/general.hpp>

#include <cstdlib>

#include "Structs.hpp"
#include "TimeUtils.hpp"

using namespace geode::prelude;

namespace ReplyInfo {
    // Markdown for the popup body. MDTextArea understands <cg>, <cl>, <cy>...
    inline std::string buildMarkdown(Reply const& reply) {
        std::string md;

        md += fmt::format("**ID:** <cy>{}</c>\n", reply.id.empty() ? "?" : reply.id);
        md += fmt::format("**Author:** <cg>{}</c>\n", replyDisplayName(reply));

        if (reply.author_id > 0) {
            md += fmt::format("**Account ID:** <cl>{}</c>\n", reply.author_id);
        }
        if (reply.from_comment && reply.player_id > 0 && reply.player_id != reply.author_id) {
            md += fmt::format("**Player ID:** <cl>{}</c>\n", reply.player_id);
        }

        auto unix = replyUnixSeconds(reply);
        if (ReplyTime::isPlausible(unix)) {
            md += fmt::format("**Posted:** <cj>{}</c>\n", ReplyTime::formatFull(unix, false));
            md += fmt::format("**UTC:** <cj>{}</c>\n", ReplyTime::formatFull(unix, true));
            md += fmt::format("**Unix:** <cp>{}</c>\n", unix);
        } else if (reply.from_comment) {
            // The server did not send an exact timestamp for this comment.
            md += "**Posted:** <cr>no exact date from the server</c>\n";
        }

        auto relative = replyRelativeString(reply);
        if (!relative.empty()) {
            md += fmt::format("**Age:** <cj>{}</c>\n", relative);
        }

        md += fmt::format("**Likes:** <co>{}</c>\n", reply.likes);
        md += fmt::format("**Replies:** <co>{}</c>\n", reply.reply_count);

        if (reply.from_comment) {
            md += "**Source:** <cl>Geometry Dash comment</c>\n";
            if (reply.comment_percentage > 0) {
                md += fmt::format("**Percent:** <cg>{}%</c>\n", reply.comment_percentage);
            }
            if (reply.has_level_id && reply.level_id != 0) {
                md += fmt::format(
                    "**{}:** <cl>{}</c>\n",
                    reply.level_id < 0 ? "List ID" : "Level ID",
                    std::abs(reply.level_id)
                );
            }
            switch (reply.comment_mod_badge) {
                case 1: md += "**Badge:** <cg>Moderator</c>\n"; break;
                case 2: md += "**Badge:** <co>Elder Moderator</c>\n"; break;
                default: break;
            }
        } else {
            md += "**Source:** <cp>Replies server</c>\n";
        }

        return md;
    }

    // Same content, stripped of markup, for the clipboard.
    inline std::string buildPlainText(Reply const& reply) {
        std::string text;

        text += fmt::format("ID: {}\n", reply.id.empty() ? "?" : reply.id);
        text += fmt::format("Author: {}\n", replyDisplayName(reply));
        if (reply.author_id > 0) text += fmt::format("Account ID: {}\n", reply.author_id);
        if (reply.from_comment && reply.player_id > 0 && reply.player_id != reply.author_id) {
            text += fmt::format("Player ID: {}\n", reply.player_id);
        }

        auto unix = replyUnixSeconds(reply);
        if (ReplyTime::isPlausible(unix)) {
            text += fmt::format("Posted: {}\n", ReplyTime::formatFull(unix, false));
            text += fmt::format("UTC: {}\n", ReplyTime::formatFull(unix, true));
            text += fmt::format("Unix: {}\n", unix);
        }

        auto relative = replyRelativeString(reply);
        if (!relative.empty()) text += fmt::format("Age: {}\n", relative);

        text += fmt::format("Likes: {}\n", reply.likes);
        text += fmt::format("Replies: {}\n", reply.reply_count);

        if (reply.from_comment) {
            text += "Source: Geometry Dash comment\n";
            if (reply.comment_percentage > 0) {
                text += fmt::format("Percent: {}%\n", reply.comment_percentage);
            }
            if (reply.has_level_id && reply.level_id != 0) {
                text += fmt::format(
                    "{}: {}\n",
                    reply.level_id < 0 ? "List ID" : "Level ID",
                    std::abs(reply.level_id)
                );
            }
        } else {
            text += "Source: Replies server\n";
        }

        return text;
    }

    inline void show(Reply const& reply) {
        // Copied by value: the cell that opened this popup can be destroyed by
        // a page change or a reload while the popup is still on screen.
        auto copyText = buildPlainText(reply);

        auto popup = MDPopup::create(
            "Reply Info",
            buildMarkdown(reply),
            "OK",
            "Copy",
            [copyText](bool btn2) {
                if (!btn2) return;
                if (geode::utils::clipboard::write(copyText)) {
                    geode::Notification::create("Copied to clipboard", NotificationIcon::Success)->show();
                } else {
                    geode::Notification::create("Failed to copy", NotificationIcon::Error)->show();
                }
            }
        );
        if (popup) popup->show();
    }
}
