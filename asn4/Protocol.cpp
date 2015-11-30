#include "Protocol.h"

void Wait() {
	// check comm mask for EV_RXCHAR
	// if not found, keep checking until a timer is done
	// if timer is done, go to Idle
	// if an ENQ is received, SendAck and ReceivePacket
}
void Idle() {
	// check comm mask for EV_RXCHAR
	// if one is found, process it
	// if not, check if there is a packet in the buffer
	// if there is, send an enq
}

void SendEnq() {
	// put ENQ on the serial port
	// wait for ACK
	// if that returns false, timeout procedures
	// if true, send a packet
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
	if (!WriteFile(hComm, &ak, 1, NULL, NULL)) {
		OutputDebugString("Unable to send ACK\n");
		return FALSE;
	}
	OutputDebugString("Sent ACK\n");
	return TRUE;
}

CHAR* ReceivePacket(HANDLE hComm) {
	// check the serial port
	// if there is a character and it is not EOT, add it to a buffer representing a packet
	// when max packet sized is reached or EOT is read, send the packet to the erorr checker
	// if that returns OK, send an ack
	// if not keep waiting (timeout procedures)
	DWORD currentLen = 0;
	DWORD bytesRead = 0;
	CHAR *receiveBuffer = (CHAR *)calloc(1, PACKETLENGTH);
	if (!receiveBuffer) {
		OutputDebugString("Failed to allocate memory in ReceivePacket\n");
		return FALSE;
	}
	while (currentLen < PACKETLENGTH) {
		if (ReadFile(hComm, receiveBuffer + currentLen, PACKETLENGTH, &bytesRead, NULL)) {
			currentLen += bytesRead;
			if (*(receiveBuffer + currentLen) == EOT) {
				break;
			}
		}
	}
	return receiveBuffer;
}

void Depacketize(CHAR *packet) {
	CHAR temp[PACKETLENGTH];
	int i;
	BOOL eotFlag = false;
	// copy the data bytes to the front of the packet
	strncpy_s(packet, PACKETLENGTH, packet + 4, DATALENGTH);
	// last 4 bytes contain junk; if they contain nulls there will be errors
	for (i = 0; i < PACKETLENGTH; i++) {
		if (eotFlag)
			packet[i] = 0;
		if (packet[i] == EOT) {
			eotFlag = TRUE;
			packet[i] = '\n';
		}
		}
	packet[DATALENGTH] = 0;
	packet[DATALENGTH+1] = 0;
	packet[DATALENGTH+2] = 0;
	packet[DATALENGTH+3] = 0;


}

BOOL ErrorCheck(CHAR *packet) {
	checksum *cs = new checksum(); 
	int i;
	for (i = 0; i < PACKETLENGTH && packet[i] != EOT; i++) {
		cs->add(packet[i]);
	}
	std::vector<char> k = cs->get();
	OutputDebugString("Packet is ");
	OutputDebugString(cs->check(k[0], k[1]) ? "valid\n" : "invalid\n");
	return cs->check(k[0], k[1]);
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
	checksum *cs = new checksum();
	cs->clear();
	for (i = 0; i < 512; i++) {
		packet[i] = 0;
	}
	packet[0] = SOH;
	//TODO: make this check which sync bit to write
	packet[1] = SYNC_0;
	packet[2] = 0;
	packet[3] = 0;
	for (i = 0; buf[i] != 0 && i < 512;) { 

		packet[j++] = buf[i++];
	} 
	if (i < 512) {
		packet[j] = EOT;
	}
	for (i = 0; i < j; i++) {
		cs->add(packet[i]);
	}
	std::vector<char> k = cs->get();
	packet[2] = k[0];
	packet[3] = k[1];
	delete[] cs;
}
