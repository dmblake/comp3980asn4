#include "Protocol.h"

BOOL Wait(HANDLE event, DWORD timeout) {
	DWORD result;
	OutputDebugString("Starting to wait...\n");
	result = WaitForSingleObject(event, timeout);
	switch (result) {
	case WAIT_TIMEOUT:
		OutputDebugString("Timed out\n");
		return FALSE;
		break;
	case WAIT_OBJECT_0:
		OutputDebugString("Object signalled\n");
		return TRUE;
		break;
	case WAIT_FAILED:
		OutputDebugString("Wait failed\n");
		return FALSE;
		break;
	}
	return TRUE;
}
void Idle() {
	// check comm mask for EV_RXCHAR
	// if one is found, process it
	// if not, check if there is a packet in the buffer
	// if there is, send an enq
}

void SendEnq(HANDLE hComm) {
	// put ENQ on the serial port
	// wait for ACK
	// if that returns false, timeout procedures
	// if true, send a packet
	DWORD bytesWritten = 0;
	CHAR enq = ENQ;
	if (!WriteFile(hComm, &enq, 1, &bytesWritten, NULL)) {
		OutputDebugString("Could not send ENQ\n");
	}
}

void WaitForAck() {
	// check comm mask for EV_RXCHAR
	// if one is found, process it
	// if it is an ACK return true
	// if not, return false
}

void SendPacket() {
	// put the packet onto the serial port
	// wait for ack
	// timeout procedure
}

BOOL SendAck(HANDLE hComm) {
	// put ACK on the serial port
	char ak = ACK;
	OVERLAPPED overlapped = { 0 };
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	WriteFile(hComm, &ak, 1, NULL, &overlapped);
	WaitForSingleObject(overlapped.hEvent, INFINITE);
	OutputDebugString("Sent ACK\n");
	return TRUE;
}

CHAR* ReceivePacket(HANDLE hComm, OVERLAPPED overlapped) {
	CHAR *receiveBuffer = (CHAR *)calloc(1, PACKETLENGTH);
	if (!receiveBuffer) {
		OutputDebugString("Failed to allocate memory in ReceivePacket\n");
		return FALSE;
	}
	if (!ReadFile(hComm, receiveBuffer, PACKETLENGTH, NULL, &overlapped)) {
		if (GetLastError() == ERROR_IO_PENDING) {
			if (WaitForSingleObject(overlapped.hEvent, INFINITE) == WAIT_OBJECT_0) {
				OutputDebugString("SUCCESSFUL WAIT\n");
			}
			else {
				OutputDebugString("FAILED WAIT\n");
			}
		}
	}
	return receiveBuffer;
}

void Depacketize(CHAR *packet) {
	int i, j;
	BOOL eotFlag = false;
	OutputDebugString("Inside depacketize\n");
	// copy the data bytes to the front of the packet
	strncpy_s(packet, PACKETLENGTH, packet + 4, DATALENGTH);
	// last 4 bytes contain junk; they must contain nulls
	for (i = 0; i < PACKETLENGTH; i++) {

		if (eotFlag)
			packet[i] = 0;
		if (packet[i] == EOT) {
			eotFlag = TRUE;
			packet[i] = '\n'; // puts a new line character where EOT was
		}
	}
	packet[DATALENGTH] = 0;
	packet[DATALENGTH+1] = 0;
	packet[DATALENGTH+2] = 0;
	packet[DATALENGTH+3] = 0;
}

BOOL ErrorCheck(const CHAR *packet) {
	int i;
	DWORD sum = 0;
	CHAR checkbytes[2] = { 0, 0 };
	// calculate sum
	for (i = 0; i < PACKETLENGTH && packet[i] != EOT; i++) {
		sum += packet[i];
	}
	// only handle the two least significant bytes
	sum &= 0xffff;
	// remove the values of the checksum
	sum -= packet[2] + packet[3];
	// the value of the checksum packets should reflect the adjusted sum
	checkbytes[0] = (sum & 0xff00) / 256;
	checkbytes[1] = (sum & 0x00ff);
	BOOL check = (packet[2] == checkbytes[0]) && (packet[3] == checkbytes[1] );
	OutputDebugString("Packet is ");
	OutputDebugString(check ? "valid\n" : "invalid\n");
	return check;
}


DWORD CreatePackets(CHAR *bufToPacketize, CHAR buffer[MAXPACKETS][PACKETLENGTH]) {
	int i;
	int j;
	int byteStart = 0;
	DWORD packetsCreated = 0;
	CHAR *temp = (CHAR *)calloc(1, 513);
	// failed to allocate memory
	if (!temp) {
		return 0;
	}
	// 1024 packets can possibly be created
	for (i = 0; i < MAXPACKETS; i++) {
		// start inspecting at byteStart + 0
		j = 0;
		// go until you find \0 or you have 512 bytes
		while (bufToPacketize[j + byteStart] != 0 && j < 512) { j++; }
		// copy data to a temp
		strncpy_s(temp, j+1, bufToPacketize + byteStart, j); 
		// packetize
		Packetize(temp, buffer[i]);
		// update the start position of the next packet within the original buffer
		packetsCreated++;
		// break if end of buffer
		if (bufToPacketize[j + byteStart] == 0) {
			break;
		}
		byteStart += j;
	}
	OutputDebugString("\n");
	free(temp);
	return packetsCreated;
}


void Packetize(CHAR *buf, CHAR *packet) {
	int i, j=4;
	DWORD sum = 0;
	for (i = 0; i < 512; i++) {
		packet[i] = 0;
	}
	packet[0] = SOH;
	//TODO: make this check which sync bit to write
	packet[1] = SYNC_0;
	// zero the 2 checksum bytes
	packet[2] = 0;
	packet[3] = 0;
	// copy until end of file (character read is 0 or 512 bytes read)
	for (i = 0; buf[i] != 0 && i < DATALENGTH;) {
		packet[j++] = buf[i++];

	} 
	// insert EOT if  it's not a full packet
	if (i < 512) {
		packet[j] = EOT;
	}
	// calculate checksum
	for (i = 0; i < j; i++) {
		sum += packet[i];
	}
	// retrieve the most significant byte and convert to char
	// packet[2] = sum & 0xff;
	packet[2] = (sum & 0x0000ff00) /256;
	// repeat for least significant byte
	packet[3] = (sum & 0x000000ff);
	// check the packet locally for errors
	OutputDebugString(ErrorCheck(packet) ? "Checksum reversed\n" : "Checksum failed\n");
 }
