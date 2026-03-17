#pragma once
#include <string>
#include <vector>

namespace Localization {

enum Language {
    LANG_EN = 0,
    LANG_CN = 1,
    LANG_COUNT
};

struct Strings {
    const char* title_main;
    const char* title_sub;
    const char* continue_game;
    const char* new_game;
    const char* settings;
    const char* exit;
    const char* warning_delete_save;
    const char* control_hint_arrows;
    const char* control_hint_enter;
    const char* settings_title;
    const char* language_option;
    const char* lang_name_en;
    const char* lang_name_cn;
    const char* back;
    const char* funds;
    const char* reputation;
    const char* day;
    const char* agency_hub_title;
    const char* factory_overview;
    const char* status_online;
    const char* status_offline;
    const char* miners;
    const char* smelters;
    const char* assemblers;
    const char* warehouse;
    const char* resources;
    const char* rocket_parts;
    const char* no_items;
    const char* hub_vab;
    const char* hub_launchpad;
    const char* hub_factory;
    const char* hub_return;
};

static const Strings EN = {
    "VERTICAL LANDING",
    "SIMULATOR",
    "CONTINUE GAME",
    "NEW GAME",
    "SETTINGS",
    "EXIT",
    "WARNING: WILL DELETE SAVE",
    "USE UP/DOWN ARROWS TO SELECT",
    "PRESS ENTER TO CONFIRM",
    "SETTINGS",
    "LANGUAGE",
    "ENGLISH",
    "CHINESE (CN)",
    "BACK",
    "FUNDS",
    "REP",
    "DAY",
    "SPACE AGENCY HUB",
    "FACTORY OVERVIEW",
    "STATUS: ONLINE",
    "STATUS: OFFLINE",
    "MINERS",
    "SMELTERS",
    "ASSEMBLERS",
    "WAREHOUSE",
    "RESOURCES",
    "ROCKET PARTS",
    "NO ITEMS IN STORAGE",
    "VEHICLE ASSEMBLY (VAB)",
    "LAUNCHPAD",
    "ENTER FACTORY MODE",
    "RETURN TO MAIN MENU"
};

static const Strings CN = {
    "VERTICAL LANDING",
    "SIMULATOR",
    "\u7EE7\u7EED\u6E38\u620F",      // 继续游戏 (Continue)
    "\u65B0\u6E38\u620F",       // 新游戏 (New Game)
    "\u8BBE\u7F6E",          // 设置 (Settings)
    "\u9000\u51FA",          // 退出 (Exit)
    "\u8B66\u544A: \u5C06\u5206\u9664\u5B58\u6863", // 警告: 将删除存档 (Warning: Will delete save)
    "\u4F7F\u7528\u4E0A\u4E0B\u952E\u9009\u62E9",  // 使用上下键选择 (Use up/down to select)
    "\u6309\u56DE\u8F66\u786E\u8BA4",               // 按回车确认 (Press enter to confirm)
    "\u8BBE\u7F6E",          // 设置 (Settings Title)
    "\u8BED\u8A00",           // 语言 (Language)
    "\u82F1\u8BED",          // 英语 (English)
    "\u4E2D\u6587",        // 中文 (Chinese)
    "\u8FD4\u56DE",          // 返回 (Back)
    "\u8D44\u91D1",           // 资金 (Funds)
    "REP",       // Rep (Keeping English for now as Rep is common)
    "\u5929",            // 天 (Day)
    "\u822A\u5929\u5C40\u4E2D\u5FC3", // 航天局中心 (Agency Hub)
    "\u5DE5\u5382\u6982\u89C8",     // 工厂概览 (Factory Overview)
    "\u72B6\u6001: \u5728\u7EBF",  // 状态: 在线 (Online)
    "\u72B6\u6001: \u79BB\u7EBF",   // 状态: 离线 (Offline)
    "\u77FF\u673A",             // 矿机 (Miners)
    "\u7194\u7089",            // 熔炉 (Smelters)
    "\u88C5\u914D\u673A",         // 装配机 (Assemblers)
    "\u4ED3\u5E93",              // 仓库 (Warehouse)
    "\u8D44\u6E90",              // 资源 (Resources)
    "\u706B\u7BAD\u96F6\u4EF6",    // 火箭零件 (Rocket Parts)
    "\u6CA1\u6709\u7269\u4EA7",       // 没有物产 (No Items)
    "\u7EC4\u88C5\u8F66\u95F4 (VAB)", // VAB
    "\u53D1\u5C04\u573A",            // Launchpad
    "\u8FDB\u5165\u5DE5\u5382\u6A21\u5F0F", // Factory
    "\u8FD4\u56DE\u4E3B\u83DC\u5355"         // Return to Main Menu
};

inline const Strings& Get(Language lang) {
    if (lang == LANG_CN) return CN;
    return EN;
}

} // namespace Localization
