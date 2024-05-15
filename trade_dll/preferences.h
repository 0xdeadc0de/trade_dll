#pragma once

enum Train
{
	European,
	Arabian,
	Monk,
	DontTrain,
};
struct UserSettings
{
	int   europeanUnit = 3;
	int   arabianUnit  = 4;
	Train training     = Train::DontTrain;
	bool  autoTrade    = false;
};
extern struct UserSettings UserSettings;

enum Trade
{
    Buy = 0,
    Sell = 1,
};
struct TradeSetting
{
    const char*    Name;
    bool           Enabled;
    Trade          Option;
    unsigned short Threshold;
};
#define MATERIAL_COUNT 20
extern TradeSetting TradeSettings[];