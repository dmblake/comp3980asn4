#ifndef PROTOCOL_H
#define PROTOCOL_H


#define PACKETLENGTH 516
#define DATALENGTH 512
#define MAXPACKETS 23740

#define SOH 0x01
#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06

#define DC1 0x11
#define P_ACK 0x11

#define DC2 0x12
#define P_ENQ 0x12

#define SYNC_0 0x0F
#define SYNC_1 0xF0

#define TIMEOUT 3000

#include <Windows.h>
#include "stdio.h"
#include "checksum.h"

DWORD CreatePackets(CHAR *bufToPacketize, CHAR buffer[MAXPACKETS][PACKETLENGTH]);
void Packetize(CHAR *buf, CHAR *packet);
BOOL ErrorCheck(CHAR *packet);
void Depacketize(CHAR *packet);
BOOL SendAck(HANDLE hComm);
CHAR* ReceivePacket(HANDLE hComm);

void Idle();
void WaitForAck();
BOOL Wait(HANDLE event, DWORD timeout);
void SendPacket();
void SendEnq();

#endif
