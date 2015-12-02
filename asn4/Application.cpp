#include "Application.h"
TCHAR Name[] = "Assignment 4";
TCHAR FileName[100];
TCHAR Result[8000000] = "";
TCHAR ComName[] = "COM1";
CHAR pbuf[512];
CHAR packetBuffer[MAXPACKETS][PACKETLENGTH];
CHAR *receiveBuffer;
HANDLE hComm, hThrd;
HWND hMain, hBtnConnect, hBtnQuit, hSend, hReceive, hStats, hSendEnq;
DWORD packetsCreated = 0, packetsReceived = 0, packetsSent = 0, acksReceived = 0, packsAcked = 0, threadId;
TCHAR enq[1] = "";
TCHAR ack[1] = "";
COMMCONFIG cc = { 0 };
BOOL writing = FALSE;
WORD currentPacket = 0;
typedef enum {IDLE, WAIT, SENDING, RECEIVING, WACK, WENQACK} STATE;
STATE state = IDLE;

BOOL CreateUI(HINSTANCE hInst) {
	WNDCLASSEX Wcl;

	

	enq[0] = ENQ;
	ack[0] = ACK;

	if ((hComm = CreateFile(TEXT("COM1"), GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE) {
		OutputDebugString("Failed to open COM port\n");
	}
	else {
		
		
	}
	

	
	// create the main window class
	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = TEXT("ASN4MENU"); // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl)) {
		OutputDebugString("Failed to register Wcl");
		return FALSE;
	}


	// show ui
	hMain = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
		800, 600, NULL, NULL, hInst, NULL);
	if (!(hBtnConnect = CreateWindow("BUTTON", "Send File", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		BTN_XSTART, BTN_YSTART, BTN_WIDTH, BTN_HEIGHT, hMain, (HMENU)ASN_OPN, (HINSTANCE)GetWindowLong(hMain, GWL_HINSTANCE), NULL))) {
		return FALSE;
	}
	if (!(hBtnQuit = CreateWindow("BUTTON", "Quit", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		BTN_XSTART, BTN_YSTART + BTN_HEIGHT + BTN_BUFFER, BTN_WIDTH, BTN_HEIGHT, hMain, (HMENU)ASN_QUIT, (HINSTANCE)GetWindowLong(hMain, GWL_HINSTANCE), NULL))) {
		return FALSE;
	}

	if (!(hSend = CreateWindow("EDIT", "Sent File", WS_CHILD | WS_VISIBLE |  WS_BORDER | WS_VSCROLL | ES_MULTILINE,
		0, 0, 400, 250, hMain, (HMENU)NULL, (HINSTANCE)GetWindowLong(hMain, GWL_HINSTANCE), NULL))) {
		return FALSE;
	}
	if (!(hSendEnq = CreateWindow("BUTTON", "Send ENQ", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		BTN_XSTART, BTN_YSTART + (BTN_HEIGHT + BTN_BUFFER) * 2, BTN_WIDTH, BTN_HEIGHT, hMain, (HMENU)ASN_ENQ, (HINSTANCE)GetWindowLong(hMain, GWL_HINSTANCE), NULL))) {
		return FALSE;
	}
	if (!(hReceive = CreateWindow("EDIT", "Received File", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
		0, 250, 400, 250, hMain, (HMENU)NULL, (HINSTANCE)GetWindowLong(hMain, GWL_HINSTANCE), NULL))) {
		return FALSE;
	}
	if (!(hStats = CreateWindow("STATIC", "Statistics", WS_VISIBLE | WS_CHILD | WS_BORDER,
		BTN_XSTART + BTN_WIDTH + BTN_BUFFER, BTN_YSTART, 200, 200, hMain, (HMENU)NULL, (HINSTANCE)GetWindowLong(hMain, GWL_HINSTANCE), NULL))) {
		return FALSE;
	}
	cc.dwSize = sizeof(COMMCONFIG);
	cc.wVersion = 0x100;
	if (!GetCommConfig(hComm, &cc, &cc.dwSize)) {
		OutputDebugString("Could not get CommConfig\n");
		return FALSE;
	}

	if ((hThrd = CreateThread(NULL, 0, ReadFromPort, (LPVOID)hComm, 0, &threadId)) == NULL) {
		//failure
		OutputDebugString("Failed to start read thread\n");
	}
	else {
		OutputDebugString("Thread created\n");
	}
	SetStatistics();
	return TRUE;
}

int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE prevInstance, LPSTR lpCmdline, int nCmdShow) {
	MSG Msg;


	// a random file name for this session
	srand((unsigned)time(NULL));
	sprintf_s(FileName, "%d", rand());
	strncat_s(FileName, ".txt.", 4);
	
	CreateUI(hInst);
	
	ShowWindow (hMain, nCmdShow);
	UpdateWindow (hMain);

	while (GetMessage (&Msg, NULL, 0, 0))
	{
   		TranslateMessage (&Msg);
		DispatchMessage (&Msg);
	}

	return Msg.wParam;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT Message,
                          WPARAM wParam, LPARAM lParam) 
{

	switch (Message)
	{
	case WM_CREATE:
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ASN_ENQ:
			if (state != IDLE && state != WAIT) {
				OutputDebugString("ENQ but not idling\n");
				break;
			}
			startWriting();
			// send ENQ to the serial port and assume it never fails
			WriteFile(hComm, enq, 1, NULL, &OVERLAPPED());
			state = WENQACK;
			OutputDebugString("ENQ sent\n");
			finishWriting();
			break;
		case ASN_SET:
			startWriting();
			if (!CommConfigDialog(TEXT("com1"), hMain, &cc)) {
				return FALSE;
			}
			else {
				SetCommConfig(hComm, &cc, cc.dwSize);
				PurgeComm(hComm, PURGE_RXCLEAR);
			}
			finishWriting();
			break;
		case ASN_CON:
			OpenFileDialog();
			break;
		case ASN_CLR:
			ClearTextBoxes();
			break;
		case ASN_QUIT:
			PostQuitMessage(0);
			break;
		case ASN_HLP:
			break;
		case ASN_OPN:
			OpenFileDialog();
			break;
		case ACK_REC:
			if (state != WACK && state != WENQACK) {
				OutputDebugString("ACK received but not waiting for ACK before sending a packet\n");
				break;
			}
			if (state == WENQACK) {
				OutputDebugString("ACK received\n");
				acksReceived++;
				SetStatistics();
				if (packetBuffer[currentPacket][0] != 0) {
					OVERLAPPED overlapped = { 0 };
					overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
					startWriting();
					if (!WriteFile(hComm, packetBuffer[currentPacket++], PACKETLENGTH, NULL, &overlapped)) {
						WaitForSingleObject(overlapped.hEvent, TIMEOUT);						
					}

					OutputDebugString("Sent a packet\n");
					packetsSent++;
					SetStatistics();
					// ENQ the line to send the next packet
					// THIS SHOULD BE REPLACED BY THE APPROPRIATE PRIORTIY PROTOCOL
					/*
					if (packetBuffer[currentPacket][0] != 0) {
						WriteFile(hComm, enq, 1, NULL, NULL);
					}
					*/
					finishWriting();
				}
				OutputDebugString("Going to WACK state\n");
				state = WACK;
				break;
			}
			if (state == WACK) {
				OutputDebugString("Packet confirmed received\n");
				OutputDebugString("Going to IDLE state\n");
				state = IDLE;
				packsAcked++;
				acksReceived++;
				SetStatistics();
			}
			break;
		}
		break; // end WM_COMMAND
	case WM_CHAR:
		break;
	case WM_SIZE:
		break;
	case WM_DESTROY:	// Terminate program
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

DWORD PacketizeFile(HANDLE fileToBeRead) {
	DWORD bytesRead;
	DWORD totalBytes = GetFileSize(fileToBeRead, NULL);
	DWORD bytesToRead = (totalBytes > MAX_READ ? MAX_READ : totalBytes);
	DWORD totalBytesRead = 0;
	TCHAR readResult[MAX_READ + 1] = { 0 };
	
	ZeroMemory(Result, 8000000);
	SetFilePointer(fileToBeRead, NULL, NULL, FILE_BEGIN);
// as long as the whole file is not read, attempt to read the remaining file
while (totalBytesRead < totalBytes) {
	if (!ReadFile(fileToBeRead, readResult, bytesToRead, &bytesRead, NULL)) {
		// error check
		DWORD err = GetLastError();
		OutputDebugString("ReadFile failed\n");
		if (GetLastError() == ERROR_NOACCESS)
			OutputDebugString("NO ACCESS");
		return bytesRead;
	}
	else {
		// PROCESS FILE
		strcat_s(Result, readResult);
		totalBytesRead += bytesRead;
	}
}
UpdateWindowFromFile(hSend, fileToBeRead);
packetsCreated = CreatePackets(Result, packetBuffer);
SetStatistics();

return bytesRead;
}

void OpenFileDialog() {
	OPENFILENAME ofn;
	char szFile[256];
	HWND hwnd = NULL;
	HANDLE hf;

	// init structure
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

	// open the file dialog
	if (GetOpenFileName(&ofn) == TRUE) {
		// valid file selected
		// going to want to call packetize here
		hf = CreateFile(ofn.lpstrFile, GENERIC_READ,
			0, (LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL);
		if (hf == INVALID_HANDLE_VALUE) {
			return;
		}
		DWORD bytesRead = 0;
		OutputDebugString(ofn.lpstrFile);
		OutputDebugString("\n");
		if ((bytesRead = PacketizeFile(hf))) {
			currentPacket = 0;
		}
		OutputDebugString("\n");
		CloseHandle(hf);
	}
}

void ClearTextBoxes() {
	SetWindowText(hSend, "");
	SetWindowText(hReceive, "");
}

void SetStatistics() {
	TCHAR str[128] = "";
	sprintf_s(str, "PACKETS SENT: %d\n"
		"PACKETS RECEIVED: %d\n"
		"TOTAL ACKS RECEIVED: %d\n"
		"PACKETS ACKED: %d\n"
		"PACKET ERROR RATE: %d%%\n", 
		packetsSent, packetsReceived, acksReceived,
		packsAcked, 
		packetsSent == 0 ? 0 : ((packetsSent - packsAcked) * 100 / packetsSent));
	SetWindowText(hStats, str);
}

void DoNonReadingStuff(CHAR *packet) {
	Depacketize(receiveBuffer);

}

static DWORD WINAPI ReadFromPort(LPVOID lpParam) {
	OVERLAPPED overlapped = { 0 };
	HANDLE hnd = 0;
	BOOL fWaitingOnRead = FALSE;
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	CHAR buffer[PACKETLENGTH] = "";
	DWORD dwEvent;
	SetCommMask(lpParam, EV_RXCHAR);

	//not used yet
	COMSTAT comstat;

	while (true) {
		switch (state) {
		case WENQACK:
			if (!fWaitingOnRead) {
				if (!WaitCommEvent(lpParam, &dwEvent, &overlapped)) {
					if (GetLastError() == ERROR_IO_PENDING) {
						fWaitingOnRead = TRUE;
					}
				}
			}
			else {
				if (Wait(overlapped.hEvent, TIMEOUT)) {
					if (!ReadFile(lpParam, buffer, 1, NULL, &overlapped)) {
						WaitForSingleObject(overlapped.hEvent, TIMEOUT);
					};

					OutputDebugString("Finished reading in WENQACK\n");
					if (buffer[0] == ACK) {
						SendMessage(hMain, WM_COMMAND, ACK_REC, NULL);
						buffer[0] = 0;
					}
					else {
						OutputDebugString("Was not ACK, was ");
						OutputDebugString(buffer);
						OutputDebugString("\n");
					}
					fWaitingOnRead = FALSE;
				}
				else {
					state = IDLE;
				}
			}
			break;
		case WACK:
			if (!fWaitingOnRead) {
				if (!WaitCommEvent(lpParam, &dwEvent, &overlapped)) {
					if (GetLastError() == ERROR_IO_PENDING) {
						fWaitingOnRead = TRUE;
					}
				}
			}
			else {
				if (Wait(overlapped.hEvent, TIMEOUT)) {
					ReadFile(lpParam, &buffer, 1, NULL, &overlapped);
					if (buffer[0] == ACK) {
						SendMessage(hMain, WM_COMMAND, ACK_REC, NULL);
						buffer[0] = 0;
					}
					fWaitingOnRead = FALSE;
				}
				else {
					state = IDLE;
				}
			}
			break;
		case IDLE:
			if (!fWaitingOnRead) {
				if (state == IDLE && !WaitCommEvent(lpParam, &dwEvent, &overlapped)) {
					if (GetLastError() == ERROR_IO_PENDING) {
						fWaitingOnRead = TRUE;
					}

				}
			}
			else {
				if (state == IDLE && Wait(overlapped.hEvent, TIMEOUT)) {
					ReadFile(lpParam, &buffer, 1, NULL, &overlapped);
					if (buffer[0] == ENQ) {
						startWriting();
						SendAck(lpParam);
						finishWriting();
						state = RECEIVING;
					}
					// this is what you call a hack to get around race conditions
					else if ((state == WENQACK || state == WACK) && buffer[0] == ACK) {
						SendMessage(hMain, WM_COMMAND, ACK_REC, NULL);
					}
		
					fWaitingOnRead = FALSE;
				}
				else {
					state = IDLE;
				}
			}
			break;
		case RECEIVING:
			if (!fWaitingOnRead) {
				if (!WaitCommEvent(lpParam, &dwEvent, &overlapped)) {
					if (GetLastError() == ERROR_IO_PENDING) {
						fWaitingOnRead = TRUE;
					}

				}
			}
			else {
				if (Wait(overlapped.hEvent, INFINITE)) {
					receiveBuffer = ReceivePacket(lpParam, overlapped);
					if (ErrorCheck(receiveBuffer) || 1) {
						//DoNonReadingStuff(receiveBuffer);
						Depacketize(receiveBuffer);
						startWriting();
						SendAck(lpParam);
						finishWriting();
						packetsReceived++;
						SetStatistics();
						// open the file for writing and reading

						hnd = CreateFile(FileName, FILE_APPEND_DATA | GENERIC_WRITE | GENERIC_READ, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						startWriting();
						if (WritePacketToFile(receiveBuffer, hnd)) {
							UpdateWindowFromFile(hReceive, hnd);
						}
						finishWriting();
						CloseHandle(hnd);

						fWaitingOnRead = FALSE;
						state = IDLE;
						buffer[0] = 0;
						free(receiveBuffer);
					}
				}
				else {
					state = IDLE;
				}
			}
			break;
		}
	}
	return 1;
}

void startWriting() {
	writing = true;
	CancelIoEx(hComm, NULL);
}

void finishWriting() {
	writing = false;
}

BOOL UpdateWindowFromFile(HWND hwnd, HANDLE fileToBeRead) {
	OVERLAPPED overlapped = { 0 };
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (fileToBeRead == INVALID_HANDLE_VALUE) {
		OutputDebugString("Could not open handle in UpdateWindowFromFile\n");
		return FALSE;
	}
	DWORD totalBytes = GetFileSize(fileToBeRead, NULL);
	DWORD bytesToRead = (totalBytes > MAX_READ ? MAX_READ : totalBytes);
	DWORD totalBytesRead = 0;
	DWORD bytesRead = 0;
	CHAR readResult[MAX_READ + 1] = { 0 };
	ZeroMemory(Result, 8000000);

	// reset file pointer to start
	SetFilePointer(fileToBeRead, NULL, NULL, FILE_BEGIN);
	// as long as the whole file is not read, attempt to read the remaining file

	if (!ReadFile(fileToBeRead, readResult, bytesToRead, NULL, &overlapped)) {
		// error check
		DWORD err = GetLastError();
		if (GetLastError() == ERROR_NOACCESS) {
			OutputDebugString("ReadFile failed\n");
			OutputDebugString("NO ACCESS");
			return FALSE;
		}

	}

	// PROCESS FILE
	if (GetLastError() == ERROR_IO_PENDING) {
		if (WaitForSingleObject(overlapped.hEvent, TIMEOUT) != WAIT_OBJECT_0) {
			return FALSE;
		}
	}
	strcat_s(Result, readResult);


	SetWindowText(hwnd, Result);
	return TRUE;
}

BOOL WritePacketToFile(CHAR *packet, HANDLE fileToBeWritten) {
	int i;
	DWORD err = 0;
	OVERLAPPED overlapped = { 0 };
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	overlapped.Offset = 0xffffffff;
	overlapped.OffsetHigh = 0xffffffff;
	if (fileToBeWritten == INVALID_HANDLE_VALUE) {
		OutputDebugString("Failed to CreateFile in WritePacketToFile\n");
		DWORD err = GetLastError();
		return FALSE;
	}
	SetFilePointer(fileToBeWritten, NULL, NULL, FILE_END);
	for (i = 0; i < PACKETLENGTH; i++) {
		if (packet[i] == 0) break;
	}
	if (!WriteFile(fileToBeWritten, packet, i, NULL, &overlapped)) {
		if ((err = GetLastError()) == ERROR_IO_PENDING) {
			if (WaitForSingleObject(overlapped.hEvent, TIMEOUT) == WAIT_OBJECT_0) {
				OutputDebugString("Successful write\n");
				return TRUE;
			}
			else {
				OutputDebugString("Failed to write\n");
				return FALSE;
			}
		}
		else {
			OutputDebugString("Failed to write\n");
			return FALSE;
		}
	}
	else {
		OutputDebugString("Successful write\n");
		return TRUE;
	}
}
