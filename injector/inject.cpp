#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <tlHelp32.h>

int findProcess(const char* szProc)
{	
    PROCESSENTRY32 PE32{ sizeof(PROCESSENTRY32) };
	PE32.dwSize = sizeof(PE32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE) 
    {
		printf("CreateToolhelp32Snapshot failed!");
		printf("LastError : 0x%x\n", GetLastError());
		system("PAUSE");
		return 0;
	}

    INT process_id = 0;
	BOOL bRet;
	while (bRet = Process32Next(hSnap, &PE32)) 
    {
		if (!strcmp((LPCSTR)szProc, PE32.szExeFile)) 
        {
			process_id = PE32.th32ProcessID;
            break;
		}
	}

    return process_id;
}

int main() 
{
	char szProc[80] = 
        "Stronghold Crusader.exe";
        //"dxwnd.exe";
        //"injector.exe";
    
    // Find the process with given name.
    int process_id = findProcess(szProc);
    if (!process_id) 
    {
        std::cout << szProc << ": Process could not be found." << std::endl;
        system("PAUSE");
        return -1;
    }
    
    // Get absolute path for dll. Dll should exist near this exe file.
    const char* relativePath = "trade_dll.dll";
    char fullPath[MAX_PATH];
    DWORD charsWritten = GetFullPathName(relativePath, MAX_PATH, fullPath, NULL);
    if (!charsWritten)
        return -1;

    const SIZE_T fullPathLength = strlen(fullPath) + 1;

    // Use GetFileAttributes to check the file
    DWORD attributes = GetFileAttributesA(fullPath);

    if (attributes == INVALID_FILE_ATTRIBUTES) 
    {
        printf("Error: Could not access file '%s'\n", fullPath);
    } 
    else if (attributes & FILE_ATTRIBUTE_DIRECTORY) 
    {
        printf("'%s' is a directory\n", fullPath);
    } 
    else 
    {
        printf("'%s' exists\n", fullPath);
    }

    // Open a handle to target process.
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);

    // Allocate memory for the dllpath in the target process.
    // length of the path string + null terminator.
    LPVOID dllStartAddress = VirtualAllocEx(hProcess, 0, fullPathLength,
        MEM_COMMIT, PAGE_READWRITE);

    if (!dllStartAddress)
    {
        std::cout << "Cannot allocate virtual memory." << std::endl;
        system("PAUSE");
        return -1;
    }

    // Write the path to the address of the memory we just allocated.
    // in the target process.
    WriteProcessMemory(hProcess, dllStartAddress, (LPVOID)fullPath,
        fullPathLength, 0);

    // Create a Remote Thread in the target process which
    // calls LoadLibraryA as our dllpath as an argument -> program loads our dll
    HANDLE hLoadThread = CreateRemoteThread(hProcess, 0, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32.dll"),
            "LoadLibraryA"), dllStartAddress, 0, 0);

    if (!hLoadThread)
    {
        std::cout << "Cannot start a remote thread." << std::endl;
        system("PAUSE");
        return -1;
    }

    // Wait for the execution of our loader thread to finish
    WaitForSingleObject(hLoadThread, INFINITE);

    std::cout << "Dll path allocated at: " << std::hex << dllStartAddress << std::endl;
    std::cout << std::endl << "Pressing enter will unload the dll and close this terminal window...";
    std::cin.get();

    // Free the memory allocated for our dll path
    VirtualFreeEx(hProcess, dllStartAddress, 0, MEM_RELEASE);

    return 0;
}