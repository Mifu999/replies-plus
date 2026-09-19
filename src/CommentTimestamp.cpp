#include "CommentTimestamp.hpp"
#include "TimeUtils.hpp"

#include <Geode/modify/GJComment.hpp>

// Capture the exact upload timestamp the server sends alongside every comment
// before GJComment::create throws it away. See CommentTimestamp.hpp.
class $modify(RepliesGJComment, GJComment) {
    static GJComment* create(cocos2d::CCDictionary* dict) {
        auto comment = GJComment::create(dict);
        if (!comment || !dict) return comment;

        auto value = dict->valueForKey("15");
        if (!value) return comment;

        // valueForKey never returns null for a missing key — it hands back an
        // empty CCString — so an absent key reads as 0 and is filtered here,
        // along with anything that is clearly not a timestamp.
        int stamp = value->intValue();
        if (!ReplyTime::isPlausible(stamp)) return comment;

        comment->setUserObject(COMMENT_TIMESTAMP_KEY, CCInteger::create(stamp));
        return comment;
    }
};
