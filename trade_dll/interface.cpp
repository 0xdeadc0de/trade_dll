#include "pch.h"
#include "stronghold.h"
#include "preferences.h"
#include <unordered_map>

// Function type that is used for control callbacks.
typedef void(*EventHandler)(HWND hWnd, WORD eventId, WORD controlId);

// Map HWND ids of controls to their event handlers.
std::unordered_map<HWND, EventHandler> eventHandlers;

// Pointers for material setting controls
std::unordered_map<HWND, void*> materialSettingsPointers;

// We need to store HWNDs to use for radioBox event handlers.
HWND hWndComboBoxEuropean, hWndComboBoxArabian;

// Our DLL's custom WinProc callback function.
LRESULT CALLBACK dllWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto eventId = HIWORD(wParam);
    auto controlId = LOWORD(wParam);
    auto hWndControl = (HWND)lParam;

    switch (message)
    {
        case WM_COMMAND:
            if (eventHandlers.count(hWndControl))
            {
                auto callback = eventHandlers[hWndControl];
                callback(hWndControl, eventId, controlId);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Event handlers
void comboBoxEventEuropean(HWND hWnd, WORD eventId, WORD controlId)
{
    if (eventId != CBN_SELCHANGE)
        return;

    int selectedIndex = SendMessage(hWndComboBoxEuropean, CB_GETCURSEL, 0, 0);
    UserSettings.europeanUnit = selectedIndex;
}
void comboBoxEventArabian(HWND hWnd, WORD eventId, WORD controlId)
{
    if (eventId != CBN_SELCHANGE)
        return;

    int selectedIndex = SendMessage(hWndComboBoxArabian, CB_GETCURSEL, 0, 0);
    UserSettings.arabianUnit = selectedIndex;
}
void radioBoxEventEuropean(HWND hWnd, WORD eventId, WORD controlId)
{
    if (eventId != BN_CLICKED)
        return;

    UserSettings.training = Train::European;
}
void radioBoxEventArabian(HWND hWnd, WORD eventId, WORD controlId)
{
    if (eventId != BN_CLICKED)
        return;
    
    UserSettings.training = Train::Arabian;
}
void radioBoxEventMonk(HWND hWnd, WORD eventId, WORD controlId)
{
    if (eventId != BN_CLICKED)
        return;
    
    UserSettings.training = Train::Monk;
}
void radioBoxEventDontTrain(HWND hWnd, WORD eventId, WORD controlId)
{
    if (eventId != BN_CLICKED)
        return;
    
    UserSettings.training = Train::DontTrain;
}
void checkBoxEventAutoTrade(HWND hWnd, WORD eventId, WORD controlId)
{
    if (eventId != BN_CLICKED)
        return;

    bool isChecked = BST_CHECKED == SendMessage(hWnd, BM_GETCHECK, BST_CHECKED, 0);

    UserSettings.autoTrade = isChecked;
}

// Event handlers for material settings
void checkBoxEventMaterialSettings(HWND hWnd, WORD eventId, WORD controlId)
{
    if (eventId != BN_CLICKED)
        return;

    bool isChecked = BST_CHECKED == SendMessage(hWnd, BM_GETCHECK, BST_CHECKED, 0);

    auto target = (bool*)materialSettingsPointers[hWnd];

    *target = isChecked;
}
void comboBoxEventMaterialSettings(HWND hWnd, WORD eventId, WORD controlId)
{
    if (eventId != CBN_SELCHANGE)
        return;

    int selectedIndex = SendMessage(hWnd, CB_GETCURSEL, 0, 0);
    
    auto target = (Trade*)materialSettingsPointers[hWnd];

    *target = (Trade)selectedIndex;
}
void editBoxEventMaterialSettings(HWND hWnd, WORD eventId, WORD controlId)
{
    if (eventId != EN_KILLFOCUS)
        return;
    
    char buffer[5+1] = { 0 };
    SendMessage(hWnd, WM_GETTEXT, sizeof(buffer), (LPARAM)buffer);
    
    unsigned short threshold;
    if (1 != sscanf_s(buffer, "%hu", &threshold))
        return;
    
    auto target = (unsigned short*)materialSettingsPointers[hWnd];

    *target = threshold;
}
// Create a ComboBox with given option texts as its items added inside.
HWND createCombobox(HWND hWnd, HMODULE hModule, const char* optionTexts[], int optionCount, int x, int y, int w = 200, int h = 200)
{
    // Create combo box
    HWND hWndComboBox = CreateWindowEx(0, "COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
        x, y, w, h, hWnd, NULL, hModule, NULL);

    // Add items to combo box
    for (int i = 0; i < optionCount; ++i)
    {
        SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)optionTexts[i]);
    }

    // Set combobox selected index to first item.
    SendMessage(hWndComboBox, CB_SETCURSEL, 0, 0);

    return hWndComboBox;
}

// Registering our class will grant us to use our own WinProc.
// Without this approach calls to WinProc will be made to injected application's WinProc.
BOOL registerDLLWindowClass(HMODULE hModule, const char szClassName[])
{
    // Register window class, szClassName must match otherwise CALLBACK won't work.
    WNDCLASSEX wc;
    wc.hInstance =  hModule;
    wc.lpszClassName = (LPCSTR)szClassName;
    wc.lpfnWndProc = dllWinProc;
    wc.style = CS_DBLCLKS;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = NULL;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    if (!RegisterClassEx(&wc))
		return 0;
    return 1;
}

// Create Settings Window for user to control the injected dll.
bool CreateSettingsWindow(HMODULE hModule)
{
    const auto className = "SettingsWindow";

    // Register callback function
    if (!registerDLLWindowClass(hModule, className))
        return false;

    // Create the window
    auto hWndWindow = CreateWindowEx(WS_EX_CLIENTEDGE, className, "Settings panel", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        680, 750, NULL, NULL, hModule, NULL);

    if (hWndWindow == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Create ComboBox for European units.
    const char* europeanUnits[] =
    {
        "Archer",
        "Spearman",
        "Maceman",
        "Crossbow",
        "Pikeman",
        "Swordsman",
        "Knight",
    };
    const int europeanUnitsSize = sizeof(europeanUnits) / sizeof(europeanUnits[0]);
    hWndComboBoxEuropean = createCombobox(hWndWindow, hModule, europeanUnits, europeanUnitsSize, 50, 20);
    eventHandlers[hWndComboBoxEuropean] = comboBoxEventEuropean;

    // Create ComboBox for Arabian units.
    const char* arabianUnits[] =
    {
        "Arabian Archer",
        "Slave",
        "Slinger",
        "Assassin",
        "Horse Archer",
        "Arabian Swordsman",
        "Fire Thrower",
    };
    const int arabianUnitsSize = sizeof(arabianUnits) / sizeof(arabianUnits[0]);
    hWndComboBoxArabian = createCombobox(hWndWindow, hModule, arabianUnits, arabianUnitsSize, 50, 70);
    eventHandlers[hWndComboBoxArabian] = comboBoxEventArabian;

    // Create RadioBox for auto training European units.
    auto hWndCreated = CreateWindowEx(0, "BUTTON", "", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
        20, 20, 30, 30, hWndWindow, NULL, hModule, NULL);
    eventHandlers[hWndCreated] = radioBoxEventEuropean;
    
    // Create RadioBox for auto training Arabian units.
    hWndCreated = CreateWindowEx(0, "BUTTON", "", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
        20, 70, 30, 30, hWndWindow, NULL, hModule, NULL);
    eventHandlers[hWndCreated] = radioBoxEventArabian;
    
    // Create RadioBox for auto training Monks.
    hWndCreated = CreateWindowEx(0, "BUTTON", "Train monks", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
        20, 120, 200, 30, hWndWindow, NULL, hModule, NULL);
    eventHandlers[hWndCreated] = radioBoxEventMonk;
    
    // Create RadioBox for disabling auto training.
    hWndCreated = CreateWindowEx(0, "BUTTON", "Do not train any unit", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
        20, 170, 200, 30, hWndWindow, NULL, hModule, NULL);
    eventHandlers[hWndCreated] = radioBoxEventDontTrain;

    // Create Auto Trade checkbox.
    hWndCreated = CreateWindowEx(0, "BUTTON", "Auto Trade", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        20, 220, 200, 30, hWndWindow, NULL, hModule, NULL);
    eventHandlers[hWndCreated] = checkBoxEventAutoTrade;

    // Set Auto Trade checkbox to preference
    SendMessage(hWndCreated, BM_SETCHECK, UserSettings.autoTrade ? BST_CHECKED : BST_UNCHECKED, 0);
    
    // Set selected index to preferred Arabian unit
    SendMessage(hWndComboBoxArabian, CB_SETCURSEL, UserSettings.arabianUnit, 0);

    // Set selected index to preferred European unit
    SendMessage(hWndComboBoxEuropean, CB_SETCURSEL, UserSettings.europeanUnit, 0);

    // Create material settings.
    for (int i = 0; i < MATERIAL_COUNT; i++)
    {
        const int y = 20 + 250*(i<6) + 50*(i-6*(i>=6));
        const int x = 320*(i>=6);

        // Create checkBox
        hWndCreated = CreateWindowEx(0, "BUTTON", TradeSettings[i].Name, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            x + 20, y, 150, 30, hWndWindow, NULL, hModule, NULL);

        // Set handler and pointer
        eventHandlers[hWndCreated] = checkBoxEventMaterialSettings;
        materialSettingsPointers[hWndCreated] = &TradeSettings[i].Enabled;
        
        // Set initial value
        SendMessage(hWndCreated, BM_SETCHECK, TradeSettings[i].Enabled ? BST_CHECKED : BST_UNCHECKED, 0);
        
        // Create comboBox
        const char* optionTexts[] = { "Buy", "Sell" };
        hWndCreated = createCombobox(hWndWindow, hModule, optionTexts, 2,
            x + 180, y, 70);
        
        // Set handler and pointer
        eventHandlers[hWndCreated] = comboBoxEventMaterialSettings;
        materialSettingsPointers[hWndCreated] = &TradeSettings[i].Option;
        
        // Set initial value
        SendMessage(hWndCreated, CB_SETCURSEL, TradeSettings[i].Option, 0);

        // Create a edit control
        hWndCreated = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE | ES_NUMBER,
            x + 260, y, 50, 25, hWndWindow, NULL, hModule, NULL);

        // Set the maximum character limit
        int maxChars = 4;
        SendMessage(hWndCreated, EM_SETLIMITTEXT, (WPARAM)maxChars, 0);
        
        // Set handler and pointer
        eventHandlers[hWndCreated] = editBoxEventMaterialSettings;
        materialSettingsPointers[hWndCreated] = &TradeSettings[i].Threshold;

        // Set initial value
        char buffer[5];
        sprintf_s(buffer, "%d", TradeSettings[i].Threshold);
        SendMessage(hWndCreated, WM_SETTEXT, NULL, (LPARAM)buffer);
    }

    // Show the window
    ShowWindow(hWndWindow, SW_SHOWNORMAL);

    return true;
}