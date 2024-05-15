#include "pch.h"
#include "preferences.h"

TradeSetting TradeSettings[] =
{
    { "wood",          true,  Trade::Buy,  48 },
    { "hops",          true,  Trade::Sell, 16 },
    { "stone",         true,  Trade::Sell, 48 },
    { "iron",          true,  Trade::Sell, 48 },
    { "pitch",         true,  Trade::Sell, 0 },
    { "wheat",         true,  Trade::Sell, 32 },
    { "bread",         false, Trade::Buy,  25 },
    { "cheese",        false, Trade::Buy,  25 },
    { "meat",          false, Trade::Buy,  25 },
    { "fruit",         false, Trade::Buy,  25 },
    { "ale",           true,  Trade::Buy,  16 },
    { "flour",         true,  Trade::Sell, 32 },
    { "bows",          false, Trade::Sell, 6 },
    { "crossbows",     true,  Trade::Sell, 6 },
    { "spears",        true,  Trade::Sell, 6 },
    { "pikes",         false, Trade::Sell, 6 },
    { "maces",         true,  Trade::Sell, 6 },
    { "swords",        false, Trade::Sell, 6 },
    { "leather armor", true,  Trade::Buy,  5 },
    { "metal armor",   true,  Trade::Sell, 6 }
};

struct UserSettings UserSettings;