#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <list>

using namespace std;

DWORD WINAPI theOtherDoNothing(_In_ LPVOID lpParameter)
{
	return 0;
}

int main()
{
	//PTP_POOL tp = CreateThreadpool(NULL);
	//SetThreadpoolThreadMaximum(tp, 2000);
	// x86: 1608
	// x64: przy 260k wątków zwiecha
	list<HANDLE> threads;
	for(int i = 0; ; i++)
	{
		char c;
		HANDLE tmp = CreateThread(NULL, 64000, theOtherDoNothing, NULL,
			CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, NULL);
		if (tmp != NULL)
		{
			threads.push_back(tmp);
			cout << "Watek #" << i << " utworzony." << endl;
			if (i % 10000 == 0 && i != 0)
			{
				cout << "Kontynuować?" << endl << "y = tak" << endl;
				cin >> c;
				if (c != 'y')
				{
					for (HANDLE t : threads)
					{
						TerminateThread(t, 0);
						cout << "Terminating thread #" << i << "." << endl;
						i--;
					}
					break;
				}
			}
		}
		else
		{
			cout << "Blad tworzenia watkow. Error code: " << GetLastError() << endl;
			system("pause");
			for (HANDLE t : threads)
			{
				TerminateThread(t, 0);
				cout << "Terminating thread #" << i << "." << endl;
				i--;
			}
			break;
		}
	}
}