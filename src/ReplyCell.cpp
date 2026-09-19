#include "ReplyCell.hpp"
#include "GUI/CCControlExtension/CCScale9Sprite.h"
#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/binding/GJAccountManager.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "Geode/cocos/menu_nodes/CCMenu.h"
#include "Geode/ui/SimpleAxisLayout.hpp"
#include "Structs.hpp"
#include "ReplyVotingLayer.hpp"
#include "ReplyInfoPopup.hpp"

void ReplyCell::onReply(CCObject* sender){
    auto rl = ReplyLayer::create(m_reply);
    rl->show();
}

void ReplyCell::onVote(CCObject* sender){
    if (VECTOR_HAS_ITEM(g_votedOn, this->m_reply.id)) return;

    if (m_reply.from_comment) return;
    auto likeLayer = ReplyVotingLayer::create(this->m_reply.id,this);
    likeLayer->show();
}
void ReplyCell::doDelete(){
    if (m_reply.from_comment) return;
    auto req = web::WebRequest();
    auto url = fmt::format("{}/replies/{}/",SERVER_URL,m_reply.id);
    req.header("Authorization", Mod::get()->getSavedValue<std::string>("token"));
    req.header("mod-version",MOD_VERSION_HEADER);
    this->m_webListener.spawn(req.send("DELETE", url),[this](web::WebResponse res){
        if (!res.ok()) {
            auto json = res.json().unwrapOrDefault();
            if (json.contains("err")){
                auto errorText = json["err"]["text"].asString().unwrapOr("Unknown");
                auto notif = geode::Notification::create(fmt::format("Failed to delete: {}",errorText),NotificationIcon::Error);
                notif->show();
            }
        } else {
            m_rl->m_page = std::ceil((m_rl->m_totalReplies-2)/10)+1;
            m_rl->loadReplies(true);
        }
    });
}
void ReplyCell::onDelete(CCObject* sender){
    createQuickPopup("Delete Reply","Are you sure you want to <cr>delete</c> this reply?","No","Yes",[this](auto alert, bool btn2){
        if (btn2){
            doDelete();
        }
    });
}
void scaleAreaToFit(SimpleTextArea* area,float max){
    // cant decide rn, but im keeping the bad version for now..
    bool scaled = false;
    while (area->getScaledContentHeight() > max){
        area->setScale(area->getScale()-0.01f);
        scaled = true;
    }
    if (scaled) {
        if (area->getLines().size() == 1){
            area->setPositionY(area->getPositionY()-area->getScaledContentHeight()/4);
        }
    }
    /*if (area->getLines().size()==1)return;
    float height = area->getLineHeight();
    float padding = area->getLinePadding();
    float total = height+padding;
    int max_lines = std::ceil(max/total);
    int area_lines = area->getLines().size();
    float a = max_lines*height;
    float c = a/area_lines;
    float offset = 0;
    if (max_lines==area_lines) offset = (max_lines-1)*0.2f;
    area->setScale((c/height)*area->getScale()-offset);
    if (area->getLines().size() == 1){
        area->setPositionY(area->getPositionY()-area->getScaledContentHeight()/4);
    }*/

}
bool ReplyCell::init(){
    if (!CCNode::init()) return false;
    // og width 36
    float offset = 23*m_replyLevel;
    this->setContentSize({335.f-offset,36.f});
    if (m_rl->m_displayMode==Mode::LargeCells){
        this->setContentHeight(90.f);
    }

    bool moreReplies = this->m_spriteType == ReplySpriteType::MoreReplies;

    if (moreReplies) {
        this->setContentHeight(23.f);
    }

    auto line = CCLayerColor::create();
    line->setColor({0,0,0});
    line->setContentSize({this->getContentSize().width,.425f});
    line->setOpacity(125);
    this->addChild(line);

    auto line2 = CCLayerColor::create();
    line2->setColor({0,0,0});
    line2->setContentSize({.425f,this->getContentHeight()});
    line2->setOpacity(125);
    this->addChild(line2);

    auto bg2 = CCLayerColor::create();
    bg2->setColor({0,0,0});
    bg2->setOpacity(120);
    bg2->setContentSize({offset,this->getContentSize().height});
    bg2->setAnchorPoint({0,0});
    bg2->setPosition({-offset,0});
    this->addChild(bg2);

    std::vector<ccColor3B> line_colours = {
        {255, 0, 255},
        {0,255,0},
        {0, 234, 255},
        {255, 242, 0}
    };

    bool colouredBranchesEnabled = Mod::get()->getSettingValue<bool>("coloured-branches");
    
    for (int i = m_skipLinesRight; i<m_replyLevel-m_skipLines; i++){
        if (moreReplies && i == 0) continue;
        auto spriteName = fmt::format("reply-{}.png"_spr,(i>0 ? 1 : (int)this->m_spriteType+1));
        auto sprite = CCSprite::createWithSpriteFrameName(spriteName.c_str());
        sprite->setScale(23/sprite->getContentWidth());
        if (moreReplies) sprite->setScaleY(23/sprite->getContentHeight());
        //sprite->setScale(4.f);
        sprite->setOpacity(50);
        sprite->setAnchorPoint({0,0});
        sprite->setPosition({-23.f*(i+1),0});
        if (colouredBranchesEnabled) {
            sprite->setColor(line_colours[abs(m_replyLevel-m_skipLines-i) % 4]);
            sprite->setOpacity(100);
        }
        this->addChild(sprite);
    }

    auto bg = CCLayerColor::create();
    switch (m_bgColor){
        case ReplyBackgroundColor::Highlighted: {
            bg->setColor({ 255, 208, 0 });
            break;
        }
        case ReplyBackgroundColor::Darker: {
            bg->setColor({0,0,0});
            break;
        }
        case ReplyBackgroundColor::Regular: {
            bg->setVisible(false);
            break;
        }
    }
    bg->setOpacity(50);
    bg->setContentSize(this->getContentSize());
    this->addChild(bg);
    
    float playerIconOffset = 0.f;
    if (!m_reply.account_comment && !moreReplies){
        auto playerIcon = SimplePlayer::create(m_reply.icon.type);
        playerIcon->setPosition({5.f,this->getContentHeight()-(30.f*0.45f)-4.f});
        playerIcon->setScale(0.45f);
        playerIcon->setColors(GameManager::get()->colorForIdx(m_reply.icon.primaryColor), GameManager::get()->colorForIdx(m_reply.icon.secondaryColor));
        playerIcon->updatePlayerFrame(m_reply.icon.id, (IconType)m_reply.icon.type);
        if (m_reply.icon.glow) playerIcon->setGlowOutline(GameManager::get()->colorForIdx(m_reply.icon.glowColor));
        for (auto child : CCArrayExt<CCNode*>(playerIcon->getChildren())){
            child->ignoreAnchorPointForPosition(true);
        }
        playerIcon->setID("player-icon"_spr);
        this->addChild(playerIcon);
        playerIconOffset = 30.f*0.45f;
    }

    auto authorMenu = CCMenu::create();
    authorMenu->setID("author-menu"_spr);

    // replyDisplayName falls back to "- (ID: x)" when the server hands back an
    // empty name, which happens for deleted and unregistered accounts.
    auto authorLabel = CCLabelBMFont::create(replyDisplayName(m_reply).c_str(),"goldFont.fnt");
    if (moreReplies) authorLabel->setString(fmt::format("+ {} Repl{}",m_reply.reply_count,m_reply.reply_count == 1 ? "y" : "ies").c_str());
    authorLabel->setAlignment(kCCTextAlignmentLeft);
    authorLabel->setScale(0.5f);

    // Moderators get their chat colour on the root comment, the way GD's own
    // comment cells show it. m_color defaults to white, so this is a no-op for
    // everyone else.
    if (!moreReplies && m_reply.from_comment && m_reply.comment_mod_badge > 0) {
        authorLabel->setColor(m_reply.comment_color);
    }

    auto clickableAuthor = CCMenuItemExt::createSpriteExtra(authorLabel, [this,moreReplies](auto) {
        if (moreReplies){
            auto rl = ReplyLayer::create(m_reply);
            rl->show();
        } else {
            auto profile = ProfilePage::create(m_reply.author_id,m_reply.author_id == GJAccountManager::get()->m_accountID);
            profile->show();
        }
    });
    clickableAuthor->setAnchorPoint({0,0.5});
    clickableAuthor->setSizeMult(1.1f);

    clickableAuthor->setID("author-button"_spr);
    authorMenu->addChild(clickableAuthor);

    // The percent the author had when they posted. Replies previously dropped
    // this when turning a GJComment into a Reply, so the root cell lost a piece
    // of information the vanilla comment cell shows.
    if (!moreReplies && m_reply.from_comment && m_reply.comment_percentage > 0) {
        auto percentLabel = CCLabelBMFont::create(
            fmt::format("{}%", m_reply.comment_percentage).c_str(),
            "goldFont.fnt"
        );
        percentLabel->setScale(0.4f);
        percentLabel->setAnchorPoint({0.f,0.5f});
        percentLabel->setPosition({clickableAuthor->getScaledContentWidth()+4.f,0.f});
        percentLabel->setID("percent-label"_spr);
        authorMenu->addChild(percentLabel);
    }

    authorMenu->setPosition({playerIconOffset+6.f+(playerIconOffset!=0 ? 2.0f : 0.f),this->getContentHeight()-10.f});

    this->addChild(authorMenu);

    if (moreReplies) return true;

    // come back to this idea later maybe
    //if (m_reply.likes < 0) return true;


    /*auto contentLabel = CCLabelBMFont::create(m_reply.content.c_str(),"chatFont.fnt",200.f,kCCTextAlignmentLeft);
    contentLabel->setAnchorPoint({0,0.5});
    contentLabel->setPosition({36.f,13.f});
    contentLabel->setScale(0.65f);*/

    //auto contentLabel = TextArea::create(m_reply.content,"chatFont.fnt",0.65f,200.f,{0,1},10.f,false);
    auto contentLabel = SimpleTextArea::create(m_reply.content,"chatFont.fnt",0.65f);
    contentLabel->setWidth(this->getContentWidth()-70.f);
    //contentLabel->setMaxLines(2);
    contentLabel->setWrappingMode(WrappingMode::WORD_WRAP);
    if (m_reply.content.find(" ")==-1) contentLabel->setWrappingMode(WrappingMode::CUTOFF_WRAP);
    if (m_rl->m_displayMode==Mode::CompactCells) contentLabel->setPosition({5.f,this->getContentHeight()-18.f});
    if (m_rl->m_displayMode==Mode::LargeCells) contentLabel->setPosition({5.f,this->getContentHeight()/2});
    contentLabel->setAnchorPoint({0,1});
    if (m_rl->m_displayMode==Mode::LargeCells) contentLabel->setAnchorPoint({0,0.5});
    //if (contentLabel->getLines().size()>=2) {contentLabel->setScale(0.45f);if (contentLabel->getLines().size()==1){contentLabel->setPositionY(contentLabel->getPositionY()-contentLabel->getScaledContentHeight()/2);}}
    scaleAreaToFit(contentLabel,16.f);
    contentLabel->setID("content-label"_spr);
    this->addChild(contentLabel);

    // --- bottom info row: optional reply ID, then the date ------------------
    // Both live on one right-aligned line rather than one per corner: in
    // compact mode the content text area reaches down to y = 2 on the left, so
    // a bottom-left label would sit underneath it.
    std::string infoText = replyDateString(m_reply);
    if (Mod::get()->getSettingValue<bool>("show-reply-ids") && !m_reply.id.empty()) {
        auto idPart = fmt::format("#{}", shortReplyId(m_reply.id));
        infoText = infoText.empty() ? idPart : fmt::format("{}  {}", idPart, infoText);
    }

    // Nothing to show at all (no date from anywhere, IDs turned off): skip the
    // row rather than adding a zero-width label. Must not early-return here —
    // the like and reply buttons are built below.
    if (!infoText.empty()) {
        auto dateLabel = CCLabelBMFont::create(infoText.c_str(),"chatFont.fnt");
        dateLabel->setAlignment(kCCTextAlignmentRight);
        dateLabel->setScale(0.45f);
        dateLabel->setColor({0,0,0});
        dateLabel->setOpacity(125);
        dateLabel->setID("date-label"_spr);

        // An exact date next to a long ID runs wider than the room left by the
        // like counter, so shrink rather than overlap.
        float maxInfoWidth = this->getContentWidth()-80.f;
        if (maxInfoWidth > 0.f && dateLabel->getScaledContentWidth() > maxInfoWidth) {
            dateLabel->limitLabelWidth(maxInfoWidth, 0.45f, 0.2f);
        }

        if (Mod::get()->getSettingValue<bool>("info-popup")) {
            auto infoMenu = CCMenu::create();
            infoMenu->setPosition({0.f,0.f});
            infoMenu->setID("info-menu"_spr);

            // Captured by value: the cell can be torn down by a reload or a page
            // change while the popup is still open.
            auto replyCopy = m_reply;
            replyCopy.parent = nullptr; // points into a tree that does not outlive the cell
            replyCopy.replies.clear();  // the popup never reads them, and they can be big

            auto infoBtn = CCMenuItemExt::createSpriteExtra(dateLabel, [replyCopy](auto) {
                ReplyInfo::show(replyCopy);
            });
            infoBtn->setAnchorPoint({1.f,0.f});
            infoBtn->setPosition({this->getContentWidth()-5.f,2.f});
            infoBtn->setSizeMult(1.1f);
            infoBtn->setID("info-button"_spr);

            infoMenu->addChild(infoBtn);
            this->addChild(infoMenu);
        } else {
            dateLabel->setAnchorPoint({1,0});
            dateLabel->setPosition({this->getContentWidth()-5.f,2.f});
            this->addChild(dateLabel);
        }
    }

    auto replySpr = CCSprite::createWithSpriteFrameName("GJ_undoBtn_001.png");
    replySpr->setScale(.6f);
    auto replyBtn = CCMenuItemSpriteExtra::create(replySpr,this,menu_selector(ReplyCell::onReply));
    //auto replyMenu = CCMenu::create();
    //replyMenu->addChild(replyBtn);
    //replyMenu->setPosition({0,0});
    //replyBtn->setPosition({this->getContentWidth()-25.f,this->getContentHeight()/2});
    //this->addChild(replyMenu);

    auto likeMenu = CCMenu::create();
    likeSpr = CCSprite::createWithSpriteFrameName("GJ_likesIcon_001.png");
    if (m_reply.likes < 0) {
        auto cs = likeSpr->getContentSize();
        likeSpr = CCSprite::createWithSpriteFrameName("GJ_dislikesIcon_001.png");
        likeSpr->setContentSize(cs);
    }
    auto likeBtn = CCMenuItemSpriteExtra::create(likeSpr,this,menu_selector(ReplyCell::onVote));
    likeLabel = CCLabelBMFont::create("0","bigFont.fnt");
    likeMenu->addChild(likeLabel);
    likeMenu->addChild(likeBtn);
    if ((g_permissions >= ModerationPermissions::CommentModeration || m_reply.author_id == GJAccountManager::get()->m_accountID) && !m_reply.from_comment){
        auto deleteSpr = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
        auto deleteBtn = CCMenuItemSpriteExtra::create(deleteSpr,this,menu_selector(ReplyCell::onDelete));
        likeMenu->addChild(deleteBtn);
    }
    if (!m_reply.from_comment && this->m_bgColor != Highlighted) likeMenu->addChild(replyBtn);
    auto layout = AxisLayout::create(Axis::Row);
    layout->setAxisReverse(true);
    layout->setAutoGrowAxis(1.f);
    layout->setGap(10.f);
    likeMenu->setLayout(layout);
    likeMenu->updateLayout();

    // temp code
    auto cs = likeLabel->getContentWidth();
    likeLabel->setString(fmt::format("{}",m_reply.likes).c_str());
    likeLabel->limitLabelWidth(cs, .5f, .01f);

    likeMenu->setID("like-menu"_spr);
    likeLabel->setID("like-label"_spr);
    likeBtn->setID("like-button"_spr);
    replyBtn->setID("reply-button"_spr);
    likeMenu->setAnchorPoint({1,1});
    likeMenu->setScale(0.55f);
    this->addChildAtPosition(likeMenu,Anchor::TopRight,{-5,-5});

    return true;
}

void ReplyCell::updateLikes(int likes){
    likeLabel->setString(fmt::format("{}",likes).c_str());
    auto temp = CCSprite::createWithSpriteFrameName("GJ_likesIcon_001.png");
    auto cs = temp->getContentSize();
    if (likes < 0) temp = CCSprite::createWithSpriteFrameName("GJ_dislikesIcon_001.png");
    likeSpr->setDisplayFrame(temp->displayFrame());
    likeSpr->setContentSize(cs);
}

ReplyCell* ReplyCell::create(ReplyLayer* rl,Reply reply,ReplyBackgroundColor bgColor, int replyLevel,ReplySpriteType spriteType, int skipLines, int skipLinesRight){
    auto ret = new ReplyCell();
    ret->m_rl = rl;
    ret->m_reply = reply;
    ret->m_bgColor = bgColor;
    ret->m_replyLevel = replyLevel;
    ret->m_spriteType = spriteType;
    ret->m_skipLines = skipLines;
    ret->m_skipLinesRight = skipLinesRight;
    if (ret && ret->init()) {
        ret->setID("reply-cell"_spr);
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}