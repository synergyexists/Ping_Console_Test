// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>
#include <Psapi.h>
#include <stdio.h>
#include <thread>

using namespace std;

// Need to link with Iphlpapi.lib and Ws2_32.lib
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "psapi.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

/* Note: could also use malloc() and free() */

int taskPing();
int iPing = 0;

HANDLE hIcmpFile;
unsigned long ipaddr = INADDR_NONE;
DWORD dwRetVal = 0;
char SendData[32] = "Data Buffer";
LPVOID ReplyBuffer = NULL;
DWORD ReplySize = 0;

int main()
{

	// Declare and initialize variables
	PMIB_TCPTABLE2 pTcpTable;
	ULONG ulSize = 0;
	DWORD dwRetVal = 0;

	char szLocalAddr[128];
	char szRemoteAddr[128];
	char svRemoteAddr[128];

	struct in_addr IpAddr;

	int i;

	pTcpTable = (MIB_TCPTABLE2 *)MALLOC(sizeof(MIB_TCPTABLE2));
	if (pTcpTable == NULL) {
		printf("Error allocating memory\n");
		return 1;
	}

	ulSize = sizeof(MIB_TCPTABLE);
	// Make an initial call to GetTcpTable2 to
	// get the necessary size into the ulSize variable
	if ((dwRetVal = GetTcpTable2(pTcpTable, &ulSize, TRUE)) ==
		ERROR_INSUFFICIENT_BUFFER) {
		FREE(pTcpTable);
		pTcpTable = (MIB_TCPTABLE2 *)MALLOC(ulSize);
		if (pTcpTable == NULL) {
			printf("Error allocating memory\n");
			return 1;
		}
	}
	// Make a second call to GetTcpTable2 to get
	// the actual data we require
	if ((dwRetVal = GetTcpTable2(pTcpTable, &ulSize, TRUE)) == NO_ERROR) {
		printf("\tNumber of entries: %d\n", (int)pTcpTable->dwNumEntries);
		for (i = 0; i < (int)pTcpTable->dwNumEntries; i++) {

			switch (pTcpTable->table[i].dwState) {
			case MIB_TCP_STATE_ESTAB:
				/*printf("\n\tTCP[%d] State: %ld - ", i,
					pTcpTable->table[i].dwState);
				printf("ESTABLISHED\n");*/
				break;
			default:
				//printf("UNKNOWN dwState value\n");
				continue;
				break;
			}
			IpAddr.S_un.S_addr = (u_long)pTcpTable->table[i].dwLocalAddr;
			strcpy_s(szLocalAddr, sizeof(szLocalAddr), inet_ntoa(IpAddr));
			/*printf("\tTCP[%d] Local Addr: %s\n", i, szLocalAddr);
			printf("\tTCP[%d] Local Port: %d \n", i,
				ntohs((u_short)pTcpTable->table[i].dwLocalPort));*/

			IpAddr.S_un.S_addr = (u_long)pTcpTable->table[i].dwRemoteAddr;
			strcpy_s(szRemoteAddr, sizeof(szRemoteAddr), inet_ntoa(IpAddr));
			if (pTcpTable->table[i].dwOwningPid == 6108) {
				printf("\tTCP[%d] Remote Addr: %s:%d\n", i, szRemoteAddr,ntohs((u_short)pTcpTable->table[i].dwRemotePort));
				printf("\tTCP[%d] Owning PID: %d\n", i, pTcpTable->table[i].dwOwningPid);

				HANDLE Handle = OpenProcess(
					PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
					FALSE,
					pTcpTable->table[i].dwOwningPid /* This is the PID, you can find one from windows task manager */
				);

				if (Handle)
				{
					TCHAR Buffer[MAX_PATH];
					if (GetModuleFileNameEx(Handle, 0, Buffer, MAX_PATH))
					{
						printf("%ls\n", Buffer);
						strncpy_s(svRemoteAddr, szRemoteAddr, 128);
					}
					else
					{
						// You better call GetLastError() here
					}
					CloseHandle(Handle);
				}
			}
			/*printf("\tTCP[%d] Offload State: %ld - ", i,
				pTcpTable->table[i].dwOffloadState);
			switch (pTcpTable->table[i].dwOffloadState) {
			case TcpConnectionOffloadStateInHost:
				printf("Owned by the network stack and not offloaded \n");
				break;
			case TcpConnectionOffloadStateOffloading:
				printf("In the process of being offloaded\n");
				break;
			case TcpConnectionOffloadStateOffloaded:
				printf("Offloaded to the network interface control\n");
				break;
			case TcpConnectionOffloadStateUploading:
				printf("In the process of being uploaded back to the network stack \n");
				break;
			default:
				printf("UNKNOWN Offload state value\n");
				break;
			}*/

		}
	}
	else {
		printf("\tGetTcpTable2 failed with %d\n", dwRetVal);
		FREE(pTcpTable);
		return 1;
	}

	if (pTcpTable != NULL) {
		FREE(pTcpTable);
		pTcpTable = NULL;
	}

	hIcmpFile = IcmpCreateFile();
	

	ipaddr = inet_addr("8.8.8.8"); 
	printf("%s\n", svRemoteAddr);
	thread tPing(taskPing);
	printf("%i\n", iPing);
	ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
	ReplyBuffer = (VOID*)malloc(ReplySize);
	//tPing.join();
	/*while (1 == 1) {
		//Sleep(1000);
		
		printf("%i\n", iPing);
		clock_t start = clock();
		if (IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), NULL, ReplyBuffer, ReplySize, 5000)) {
			printf("Recieved\n");
		}
		else
			printf("nope");
		clock_t stop = clock();
		iPing = (int)(((stop - start)*1000.0) / CLOCKS_PER_SEC);
		//free(ReplyBuffer);
	}*/
	printf("haha");
	while (1 == 1) {
		printf("%i\n", iPing);
		Sleep(1000);
	}
	free(ReplyBuffer);
	return 0;
}

int taskPing() {
	while (1 == 1) {
		
		clock_t start = clock();
		int i;

		if ((i = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), NULL, ReplyBuffer, ReplySize, 1000))) {
		}
		/*else
			printf("Fail\n");*/
		clock_t stop = clock();
		iPing = (int)(((stop - start)*1000.0) / CLOCKS_PER_SEC);
		//Sleep(1000);
		
		
	}
	return 0;
}
