#include "Application.h"
TCHAR Name[] = "Assignment 4";
TCHAR FileName[100];
TCHAR Result[8000000] = "";
TCHAR ComName[] = "COM1";
CHAR pbuf[512];
CHAR packetBuffer[MAXPACKETS][PACKETLENGTH];
CHAR *receiveBuffer;
HANDLE hComm, hThrd;
HWND hMain, hBtnConnect, hBtnQuit, hSend, hReceive, hStats, hSendEnq, hSendPacket;
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
		NULL, OPEN_EXISTING, NULL, NULL)) == INVALID_HANDLE_VALUE) {
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

	if (!(hSendPacket = CreateWindow("BUTTON", "Packet", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		BTN_XSTART, BTN_YSTART + (BTN_HEIGHT + BTN_BUFFER) * 3, BTN_WIDTH, BTN_HEIGHT, hMain, (HMENU)ASN_PCK, (HINSTANCE)GetWindowLong(hMain, GWL_HINSTANCE), NULL))) {
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
			state = WENQACK;
			startWriting();
			if (!WriteFile(hComm, enq, 1, NULL, NULL)) {
				OutputDebugString("Couldn't write, sorry\n");
			}
			OutputDebugString("ENQ sent\n");
			finishWriting();
			break;
		case ASN_SET:
			if (!CommConfigDialog(TEXT("com1"), hMain, &cc)) {
				return FALSE;
			}
			else {
				SetCommConfig(hComm, &cc, cc.dwSize);
				PurgeComm(hComm, PURGE_RXCLEAR);
			}
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
		case ASN_PCK:
			OutputDebugString("Send packet\n");
			//for (int i = 0; i < packetsCreated; i++) 
			/*
			for (WORD i = 0; i < packetsCreated;) {
				startWriting();
				if (!WriteFile(hComm, packetBuffer[i], PACKETLENGTH, NULL, NULL)) {
					OutputDebugString("Couldn't write, sorry\n");
				}

				packetsSent++;
				SetStatistics();
				// ENQ the line to send the next packet
				// THIS SHOULD BE REPLACED BY THE APPROPRIATE PRIORTIY PROTOCOL
				if (++i < packetsCreated) {
					WriteFile(hComm, enq, 1, NULL, NULL);
				}
				finishWriting();
				*/
			//}
			//CancelIoEx(hComm, NULL);
			//send eot?
			
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
					startWriting();
					if (!WriteFile(hComm, packetBuffer[currentPacket++], PACKETLENGTH, NULL, NULL)) {
						OutputDebugString("Couldn't write, sorry\n");
						break;
					}
					OutputDebugString("Send a packet\n");
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
				state = WACK;
				break;
			}
			if (state == WACK) {
				OutputDebugString("Packet confirmed received\n");
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
	SetWindowText(hSend, Result);
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
		"PACKET ERROR RATE: %d%%\n", packetsSent, packetsReceived, acksReceived, packsAcked, packetsSent == 0 ? 0 : ((packetsSent - packsAcked) * 100 / packetsSent));
	SetWindowText(hStats, str);
}

static DWORD WINAPI ReadFromPort(LPVOID lpParam) {
	OVERLAPPED overlapped = { 0 };
	BOOL fWaitingOnRead = FALSE;
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	char buffer[516] = "";
	HWND hwnd = GetForegroundWindow();
	RECT windowSize;
	GetWindowRect(hwnd, &windowSize);
	HANDLE hnd = 0;

	DWORD dwEvent;
	SetCommMask(lpParam, EV_RXCHAR);

	//not used yet
	COMSTAT comstat;

	while (true) {
		//wait for comm event, can't timeout
		if (!writing && WaitCommEvent(lpParam, &dwEvent, NULL)) {
			//deal with packet including timeouts

			if (ReadFile(lpParam, &buffer, sizeof(char), NULL, &overlapped)) {
				//process char
				if (buffer[0] == ENQ) {
					state = RECEIVING;
					//if ENQ received
					OutputDebugString("ENQ received\n");
					// send ack
					if (SendAck((HANDLE)lpParam)) {
						// process the packet
						if (!(receiveBuffer = ReceivePacket((HANDLE)lpParam))) {
							OutputDebugString("Unable to receive packet\n");
						}
						// error check
						else if (ErrorCheck(receiveBuffer)) {
							// remove the header bits
							Depacketize(receiveBuffer);
							SendAck((HANDLE)lpParam);
							// state should go to idle after a timeout
							state = IDLE;
							packetsReceived++;
							SetStatistics();
							// open the file for writing and reading

							hnd = CreateFile(FileName, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

							if (WritePacketToFile(receiveBuffer, hnd)) {
								UpdateWindowFromFile(hReceive, hnd);
							}
							CloseHandle(hnd);
						}
					}
					else {
						
					}
				}
				// ACK received
				else if (buffer[0] == ACK) {
					SendMessage(hMain, WM_COMMAND, ACK_REC, NULL);
				}
			}
		}
	}
	return 0;
}

void startWriting() {
	writing = true;
	CancelIoEx(hComm, NULL);
}

void finishWriting() {
	writing = false;
}

BOOL UpdateWindowFromFile(HWND hwnd, HANDLE fileToBeRead) {
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
	while (totalBytesRead < totalBytes) {
		if (!ReadFile(fileToBeRead, readResult, bytesToRead, &bytesRead, NULL)) {
			// error check
			DWORD err = GetLastError();
			OutputDebugString("ReadFile failed\n");
			if (GetLastError() == ERROR_NOACCESS)
				OutputDebugString("NO ACCESS");
			return FALSE;
		}
		else {
			// PROCESS FILE
			strcat_s(Result, readResult);
			totalBytesRead += bytesRead;
		}
	}
	SetWindowText(hwnd, Result);
	return TRUE;
}

BOOL WritePacketToFile(CHAR *packet, HANDLE fileToBeWritten) {
	int i;
	if (fileToBeWritten == INVALID_HANDLE_VALUE) {
		OutputDebugString("Failed to CreateFile in WritePacketToFile\n");
		DWORD err = GetLastError();
		return FALSE;
	}
	SetFilePointer(fileToBeWritten, NULL, NULL, FILE_END);
	for (i = 0; i < PACKETLENGTH; i++) {
		if (packet[i] == 0) break;
	}
	if (WriteFile(fileToBeWritten, packet, i-1, NULL, NULL)) {
		OutputDebugString("Successful write\n");
		return TRUE;
	}
	else {
		OutputDebugString("Failed to write\n");
		return FALSE;
	}
}