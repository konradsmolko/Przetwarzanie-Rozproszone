#include <Windows.h>

bool init(HINSTANCE);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
const char szClassName[] = "WirusMonitorujacy";
HWND hwndSlave, hwndMaster;
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

	hwndSlave = CreateWindowEx(
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

	if (hwndSlave == NULL) return false;
	hwndMaster = NULL;

	messageCode = RegisterWindowMessage((LPCSTR)"Nic podejrzanego");

	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
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

void AnnounceHWND()
{
	SendMessage(HWND_BROADCAST, messageCode, (WPARAM)hwndSlave);
}

void RequestAccount()
{
	if (hwndMaster == NULL)
	{
		AnnounceHWND();
		return;
	}
	else
	{
		SendMessage(HWND_BROADCAST, messageCode, 0, 0);
	}
}