#include "pch.h"
#include <iostream>
#include "hash_work.h"
#include <vector>
#include <string>
#include <sysinfoapi.h>
#include <tchar.h>
#include "C.h"
using namespace std;
#define _WIN32_WINNT 0x0400

DWORD WINAPI thread_function(LPVOID lpParameter) { while (true) { SleepEx(INFINITE, TRUE); } }
VOID CALLBACK apc_function_1(ULONG_PTR dwParam) { C* obj = (C*)dwParam; obj->do_something(); }
typedef struct _MYDATA {
	TCHAR* szText;
	DWORD dwValue;
} MYDATA;

VOID CALLBACK TimerAPCProc(
	LPVOID lpArg,               // Data value.
	DWORD dwTimerLowValue,      // Timer low value.
	DWORD dwTimerHighValue) {  // Timer high value.

	/*MYDATA* pMyData = (MYDATA*)lpArg;

	printf("Message: %s\nValue: %d\n\n", pMyData->szText,
		pMyData->dwValue);*/
	//MessageBeep(0);

}
HANDLE gDoneEvent;

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	if (lpParam == NULL)
	{
		printf("TimerRoutine lpParam is NULL\n");
	}
	else
	{
		// lpParam points to the argument; in this case it is an int

		printf("Timer routine called. Parameter is %d.\n",
			*(int*)lpParam);
		if (TimerOrWaitFired)
		{
			printf("The wait timed out.\n");
		}
		else
		{
			printf("The wait event was signaled.\n");
		}
	}

	SetEvent(gDoneEvent);
}

#define WINDOWS_7_BLOCK 0x7DF74744

typedef struct _VECTORED_NODE
{
	_VECTORED_NODE* NextNode;
	_VECTORED_NODE* PrevNode;
	BOOL            IsAllocated;
	PVOID           EncodedHandler;
}VECTORED_NODE, * PVECTORED_NODE;

typedef struct _RTL_BLOCK
{
	PVOID   Unknown;
	PVECTORED_NODE ExceptionList;
}RTL_BLOCK, * PRTL_BLOCK;

LONG CALLBACK TopLevelHandler(EXCEPTION_POINTERS* info)
{
	if (info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
	{
		//std::cout << "Yep, caught" << std::endl;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

inline void log()
{
}

template <typename First, typename ...Rest>
void log(First&& message, Rest&& ...rest)
{
	std::cout << std::forward<First>(message) << std::endl;
	log(std::forward<Rest>(rest)...);
}
HANDLE event;

typedef int(WINAPI* ShellAboutProc)(HWND, LPCSTR, LPCSTR, HICON);

int main()
{
	// CreateFileA
	HANDLE hFile = hash_CreateFileA("log.txt", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE hFileW = hash_CreateFileW(L"logW.txt", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	DWORD sfp = hash_SetFilePointer(hFile,0,0, FILE_CURRENT);
	char buff88[4096];
	DWORD dwBytesRead88;
	bool readfile_ = hash_ReadFile(hFile, buff88, 4096, &dwBytesRead88, NULL);

	DWORD GetFullPathName_ = hash_GetFullPathNameW(L"", 2, nullptr, nullptr);
	DWORD GetFileAttributesW_ = hash_GetFileAttributesW(L"log.txt");

	FILETIME ft;
	hash_GetSystemTimeAsFileTime(&ft);

	if (hFile) {
		log("[OK] CreateFileA");
	}
	else if (hFile == INVALID_HANDLE_VALUE) {
		log("[FAIL] CreateFileA");
	}

	// VirtualAlloc/VirtualFree/VirtualProtect
	LPVOID p = nullptr;

	p = hash_VirtualAlloc(nullptr, 4000, MEM_RESERVE, PAGE_READWRITE);

	bool vp = hash_VirtualProtect(p, 0, PAGE_EXECUTE_READWRITE, 0); // work

	if (p != nullptr)
	{
		log("[OK] VirtualAlloc");
		if (hash_VirtualFree(p, 0, MEM_RELEASE)){
			log("[OK] VirtualFree");
		}
		else{
			log("[FAIL] VirtualFree");
		}
	} else {
		log("[FAIL] VirtualAlloc");
	}
	// -----------------------

	// VirtualAllocEx
	LPVOID p1 = nullptr;
	p1 = hash_VirtualAllocEx(GetCurrentProcess(), nullptr, 4000, MEM_RESERVE, PAGE_READWRITE);
	if (p1 != nullptr)
	{
		log("[OK] VirtualAllocEx");
		if (hash_VirtualFreeEx(GetCurrentProcess(),p, 0, MEM_RELEASE)) {
			log("[OK] VirtualFreeEx");
		}
		else {
			log("[FAIL] VirtualFreeEx");
		}
	}
	else {
		log("[FAIL] VirtualAllocEx");
	}
	// -----------------------

	// QueryDosDeviceW

	// Query these on my machine, using unicode/wide character
	// All the drives available in the tested machine except B:, A: is floppy and K: is CDRW

	LPCWSTR lpDeviceName[7] = { L"A:", L"B:", L"C:", L"E:", L"F:", L"G:", L"K:" };
	//The buffer for storage
	TCHAR lpTargetPath[1000];
	int i, j;
	DWORD test;
	// For the drives/devices...
	for (i = 0; i <= 6; i++)
	{
		// If the function succeeds, the return value is the number of TCHARs stored
		// into the buffer pointed to by lpTargetPath. If the function fails, the return
		// value is zero. To get extended error information, call GetLastError.
		// If the buffer is too small, the function fails and the last error code
		// is ERROR_INSUFFICIENT_BUFFER.

		test = hash_QueryDosDeviceW(lpDeviceName[i], (LPWSTR)lpTargetPath, 1000);

		// Test the return value and error if any
		printf("QueryDosDevice() return value: %d, Error: %d\n", test, GetLastError());
		printf("The DOS device name for %S is: ", lpDeviceName[i]);

		// Display the result of the device name

		for (j = 0; j < 30; j++)
			printf("%c", lpTargetPath[j]);
		printf("\n");

	}
	printf("\n");

	// -----------------------

	// pGetDiskFreeSpaceEx
	BOOL  fResult;
	char  *pszDrive  = NULL;

	unsigned __int64 i64FreeBytesToCaller,
		 i64TotalBytes,
		 i64FreeBytes;

	fResult = hash_GetDiskFreeSpaceExW((LPCWSTR)pszDrive,
		 (PULARGE_INTEGER)& i64FreeBytesToCaller,
		 (PULARGE_INTEGER)& i64TotalBytes,
		 (PULARGE_INTEGER)& i64FreeBytes); // If the function succeeds, the return value is nonzero
	// -----------------------

	// LoadLibraryW
	 auto LoadLibraryW_ = hash_LoadLibraryW(L"user32.dll"); //If the function succeeds, the return value is a handle to the module.
	// -----------------------

	// GetModuleHandleExW
	 char path[MAX_PATH];
	 HMODULE hm = NULL;
	 int functionInThisDll;

	 if (hash_GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		 GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		 (LPCWSTR)& functionInThisDll, &hm) == 0)
	 {
		 int ret = GetLastError();
		 fprintf(stderr, "GetModuleHandle failed, error = %d\n", ret);
		 // Return or however you want to handle an error.
	 }
	// -----------------------

	// GetModuleFileNameW
	 vector<wchar_t> pathBuf;
	 DWORD copied = 0;
	 do {
		 pathBuf.resize(pathBuf.size() + MAX_PATH);
		 copied = hash_GetModuleFileNameW(0, &pathBuf.at(0), pathBuf.size());
	 } while (copied >= pathBuf.size());

	 pathBuf.resize(copied);

	 std::string path2(pathBuf.begin(), pathBuf.end());

	 std::cout << path2;

	// -----------------------

	// GetModuleHandle
	 HMODULE hKernel32 = NULL;
	 hKernel32 = hash_GetModuleHandleA("Kernel32");
	// -----------------------

	// GetProcAddress
	HMODULE hModule = LoadLibrary(TEXT("Shell32.dll"));

	ShellAboutProc shellAbout = (ShellAboutProc)hash_GetProcAddress(hModule, "ShellAboutA");

	//shellAbout(NULL, "hello", "world", NULL);

	FreeLibrary(hModule);
	// -----------------------
	// GetModuleHandleW
	HMODULE hKernel32w = NULL;
	hKernel32w = hash_GetModuleHandleW(L"Kernel32.dll");
	// -----------------------
	// GetStdHandle
	HANDLE hStdout = hash_GetStdHandle(STD_INPUT_HANDLE);
	// -----------------------
	// GetConsoleScreenBufferInfo
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE hstdout2;
	hstdout2 = hash_GetStdHandle(STD_OUTPUT_HANDLE);

	bool gcsbi = hash_GetConsoleScreenBufferInfo(hstdout2, &csbiInfo);

	if (!gcsbi) {
		cout << "Error: " << GetLastError() << endl;
	}
	// -----------------------
	// SetConsoleTextAttribute
	bool scta = hash_SetConsoleTextAttribute(hstdout2, FOREGROUND_GREEN);

	if (!scta) {
		printf("Error hash_SetConsoleTextAttribute\n");
	} else {
		printf("\r\nGreen text\r\n");
	}
	// -----------------------
	// GetTickCount
	DWORD gt = hash_GetTickCount();
	// -----------------------
	// VerifyVersionInfoW
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
	DWORDLONG        const dwlConditionMask = VerSetConditionMask(
		VerSetConditionMask(
			VerSetConditionMask(
				0, VER_MAJORVERSION, VER_GREATER_EQUAL),
			VER_MINORVERSION, VER_GREATER_EQUAL),
		VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_WINXP);
	osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_WINXP);
	osvi.wServicePackMajor = 0;

	bool vviw= hash_VerifyVersionInfoW((LPOSVERSIONINFOEXA)&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask);
	// -----------------------
	// GetSystemWindowsDirectoryW
	TCHAR buff[MAX_PATH * 2];
	buff[0] = 0;
	hash_GetSystemWindowsDirectoryW(buff, sizeof(buff));
	// -----------------------
	// GetWindowsDirectoryW
	TCHAR buff2[MAX_PATH * 2];
	buff2[0] = 0;
	hash_GetWindowsDirectoryW(buff2, sizeof(buff2));
	// -----------------------
	// GetSystemDirectoryW
	TCHAR buff3[MAX_PATH * 2];
	buff3[0] = 0;
	hash_GetSystemDirectoryW(buff3, sizeof(buff3));
	// -----------------------
	// GetSystemDirectoryA
	char buff4[MAX_PATH * 2];
	buff4[0] = 0;
	hash_GetSystemDirectoryA(buff4, sizeof(buff4));
	// -----------------------
	// GetSystemInfo
	SYSTEM_INFO info;
	hash_GetSystemInfo(&info);
	// -----------------------
	// ExpandEnvironmentStringsW
	TCHAR szEnvPath[MAX_PATH];
	DWORD dwLen = 0;
	dwLen = hash_ExpandEnvironmentStringsW(_T("%USERPROFILE%"), szEnvPath, MAX_PATH);
	// -----------------------
	// QueryPerformanceCounter
	LARGE_INTEGER li;
	bool qpc = hash_QueryPerformanceCounter(&li);
	// -----------------------
	// IsProcessorFeaturePresent
	bool isPFP = hash_IsProcessorFeaturePresent(PF_FASTFAIL_AVAILABLE);
	// -----------------------
	// AddVectoredExceptionHandler

	// TODO: Need fix for hash
	AddVectoredExceptionHandler(TRUE, TopLevelHandler);

	// -----------------------

	// SetLastError
	DWORD error_=1488;
	hash_SetLastError(error_);
	// -----------------------

	// GetLastError
	DWORD lastError = hash_GetLastError();
	// -----------------------

	// OutputDebugStringW
	hash_OutputDebugStringW(L"debug");
	// -----------------------

	// FormatMessageW
	LPWSTR pBuffer = NULL;
	auto pMessage = "Message";
	va_list args1 = NULL;
	//va_start(args1, pMessage);

	hash_FormatMessageW(FORMAT_MESSAGE_FROM_STRING |
		FORMAT_MESSAGE_ALLOCATE_BUFFER,
		pMessage,
		0,
		0,
		(LPWSTR)& pBuffer,
		0,
		&args1);

	//va_end(args1);
	// -----------------------

	// CreateMutexW
	HANDLE mut;
	mut = hash_CreateMutexW(NULL, FALSE, L"FirstStep");
	// -----------------------

	// CreateEventW
	HANDLE responseWaitEvent = hash_CreateEventW(NULL,        // no security
		TRUE,       // manual-reset event
		TRUE,      // signaled
		(LPTSTR)_T("TESTEvent")); // event name
	// -----------------------

	// SetEvent
	hash_SetEvent(responseWaitEvent);
	// -----------------------

	// WaitForSingleObject
	DWORD nstatus;
	nstatus = hash_WaitForSingleObject(responseWaitEvent, INFINITE);
	// -----------------------

	// QueueUserAPC
	C obj;
	WORD thread_id;
	HANDLE thread_handle = hash_CreateThread(NULL, 0, thread_function, NULL, 0, (LPDWORD)&thread_id);
	Sleep(100);
	//If the function succeeds, the return value is nonzero.
	DWORD quAPC = hash_QueueUserAPC(apc_function_1, thread_handle, (ULONG_PTR)& obj);
	Sleep(100);
	// -----------------------

	// CreateWaitableTimer
	 HANDLE          hTimer;
	 hTimer = hash_CreateWaitableTimerW(
		 NULL,                   // Default security attributes.
		 FALSE,                  // Create auto-reset timer.
		 L"MyTimer");
	// -----------------------

	// SetWaitableTimer
	 __int64         qwDueTime;
	 BOOL            bSuccess;
	 LARGE_INTEGER   liDueTime;
	 MYDATA          MyData;
	 TCHAR           szError[255];
	 bSuccess = hash_SetWaitableTimer(
		 hTimer,                 // Handle to the timer object.
		 &liDueTime,             // When timer will become signaled.
		 2000,                   // Periodic timer interval of 2 seconds.
		 TimerAPCProc,           // Completion routine.
		 &MyData,                // Argument to the completion routine.
		 FALSE);                // Do not restore a suspended system.

	 HANDLE hTimerQueue = NULL;

	 hTimerQueue = CreateTimerQueue();
	 int arg = 123;
	 bool CreateTimerQueueTimer_ = hash_CreateTimerQueueTimer(&hTimer, hTimerQueue,
		 (WAITORTIMERCALLBACK)TimerRoutine, &arg, 10000, 0, 0);

	 bool CancelWaitableTimer_ = hash_CancelWaitableTimer(hTimer);


	// -----------------------

	// GetModuleHandleExW
	// -----------------------

	// GetModuleHandleExW
	// -----------------------

	// GetModuleHandleExW
	// -----------------------

	system("pause");

	return 1;
}