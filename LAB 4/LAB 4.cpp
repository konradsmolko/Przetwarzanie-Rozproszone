#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <list>

using namespace std;

int static __stdcall doNothing()
{
	
	return 0;
}

DWORD WINAPI theOtherDoNothing(_In_ LPVOID lpParameter)
{
	return 0;
}

int main()
{
	// 1 próba: 1607 wątków
	// 1608 x3
	list<HANDLE> threads;
	for(int i = 0; ; i++)
	{
		char c;
		HANDLE tmp = CreateThread(NULL, 0, theOtherDoNothing, NULL, CREATE_SUSPENDED, NULL);
		if (tmp != NULL)
		{
			threads.push_back(tmp);
			cout << "Watek #" << i << " utworzony." << endl;
			if (i % 100000 == 0)
			{
				cin >> c;
				if (c == 'x')
				{
					for (HANDLE t : threads)
					{
						TerminateThread(t, 0);
					}
					break;
				}
			}
		}
		else
		{
			cout << "Blad tworzenia watkow. Error code: " << GetLastError() << endl;
			for (HANDLE t : threads)
			{
				TerminateThread(t, 0);
			}
			break;
		}
	}
}