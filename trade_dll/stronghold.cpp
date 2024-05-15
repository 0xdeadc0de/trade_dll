#include "pch.h"
#include "stronghold.h"
#include "preferences.h"

struct PlayerMaterials 
{
    int _;
    int __;

    int wood;
    int hops;
    int stone;
    int ___;
    int iron;
    int pitch;

    int ____;
    int wheat;
    int bread;
    int cheese;
    int meat;
    int fruit;

    int ale;
    int gold;
    int flour;
    int bow;
    int xbow;
    int spear;

    int pike;
    int mace;
    int sword;
    int leather;
    int armor;
};

int arabianUnitIds[] = 
{
    77, // Arabian Archer
    71, // Slave
    72, // Slinger
    73, // Assassin
    74, // Horse
    75, // Arabian Swordsman
    76  // Fire Throwser
};

int europeanUnitIds[] = 
{
    22, // Archer
    24, // Spearman
    26, // Maceman
    23, // Crossbow
    25, // Pikeman
    27, // Swordsman
    28  // Knight
};

const unsigned char materialIds[] =
{
       // unknown
       // unknown

    2, // wood
    3, // hops
    4, // stone
       // unknown
    6, // iron
    7, // pitch

    9,  // wheat
    10, // bread
    11, // cheese
    12, // meat
    13, // fruit

    14, // ale
 // 15, // gold 
    16, // flour
    17, // bows
    18, // crossbows
    19, // spears

    20, // pikes
    21, // maces
    22, // swords
    23, // leather armor
    24  // metal armor
};

// Settings for each material when to sell
unsigned short materialStackSizes[] = 
{ 
    48u, // wood
    16u, // hops
    48u, // stone
    48u, // iron
    16u, // pitch
    32u, // wheat
    5u,  // bread
    5u,  // cheese
    5u,  // meat
    5u,  // fruit
    16u, // ale 
    32u, // flour

    5, // bows
    5, // crossbows
    5, // spears
    5, // pikes
    5, // maces
    5, // swords
    5, // leather armor
    5  // metal armor
};

// Address definitions for functions and memory locations in the Game.
const unsigned memoryAddressPeasants        = 0x9643AC;
const unsigned memoryAddressMaterials       = 0x9647F4;
const unsigned functionAddressTrade         = 0x0658C0;
const unsigned functionAddressTrainArabian  = 0x63CD0;
const unsigned functionAddressTrainEuropean = 0x63970;

// Function signatures in the Game.
typedef void(__cdecl *FuncOneInt)(int soldier_type);
typedef void(__cdecl *FuncThreeInts)(int player, int buy0_sell1, int item_type);

unsigned baseAddressRunningModule;
bool StrongholdThreadAlive = true;

void autoTrade() 
{
    if (!UserSettings.autoTrade)
        return;

    const auto materialAddress = (baseAddressRunningModule + memoryAddressMaterials);

    // Setup materials memory location (preview purposes in debugging).
    PlayerMaterials* resources = (PlayerMaterials*)materialAddress;

    // Setup trade function location.
    const auto trade = (FuncThreeInts)(baseAddressRunningModule + functionAddressTrade);
    
    #pragma loop(ivdep)
    for (size_t i = 0; i < sizeof(materialIds); i++) 
    {
        auto setting = TradeSettings[i];
        if (!setting.Enabled)
            continue;

        // ResourcePointer is the location in memory which represents all resources
        int* resourcePointer = (int*)materialAddress;

        // MaterialIndex is then used to jump to the desired location in memory
        // which represents the number of resources that player has
        auto materialIndex = materialIds[i];
        
        #define PLAYER 1
        switch (setting.Option)
        {
        case Trade::Sell:
            // Check if resource is greater than threshold set in settings
            if (resourcePointer[materialIndex] >= TradeSettings[i].Threshold) 
            {
                // Sell the resource
                trade(PLAYER, Trade::Sell, materialIndex);
            }
            break;
        
        case Trade::Buy:
            // Check if resource is smaller than threshold set in settings
            // 5 is the amount we can buy from the market in the game.
            if (resourcePointer[materialIndex] + 5 <= TradeSettings[i].Threshold) 
            {
                // Buy the resource
                trade(PLAYER, Trade::Buy, materialIndex);
            }
            break;

        default:
            break;
        }
    }
}

void autoTrain()
{
    if (UserSettings.training == Train::DontTrain)
    {
        return;
    }
    
    const auto peasants      = (int*)(baseAddressRunningModule + memoryAddressPeasants);
    const auto trainEuropean = (FuncOneInt)(baseAddressRunningModule + functionAddressTrainEuropean);
    const auto trainArabian  = (FuncOneInt)(baseAddressRunningModule + functionAddressTrainArabian);
    
    if (*peasants <= 0)
    {
        return;
    }

    switch (UserSettings.training)
    {
    case Train::European:
        trainEuropean(europeanUnitIds[UserSettings.europeanUnit]);
        break;
        
    case Train::Arabian:
        trainArabian(arabianUnitIds[UserSettings.arabianUnit]);
        break;

    case Train::Monk:
    default:
        break;
    }
}

DWORD WINAPI StrongholdThread(LPVOID param)
{
    // Assign running module's base memory address offset.
    baseAddressRunningModule = (unsigned)GetModuleHandle(NULL);

    // Enter loop.
    while (StrongholdThreadAlive)
    {
        autoTrade();
        autoTrain();
        Sleep(250);
    }

    return 0;
}