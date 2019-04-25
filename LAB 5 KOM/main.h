#pragma once
#include <Windows.h>

#define REQUEST_NUMBER	0xDEAD
#define POST_NUMBER		0xBEEF
#define MAXL			26

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
bool init(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void MoveToCb();
void PostNumber();