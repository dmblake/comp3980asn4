#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PACKETLENGTH 516
#define DATALENGTH 512
#define MAXPACKETS 23740

#include <Windows.h>
#include "stdio.h"
#include "checksum.h"

DWORD CreatePackets(CHAR *bufToPacketize, CHAR buffer[MAXPACKETS][PACKETLENGTH]);
void Packetize(CHAR *buf, CHAR *packet);
BOOL Depacketize(CHAR *packet);

void Idle();
void WaitForAck();
void Wait();
void SendPacket();
void ReceivePacket();
void SendEnq();
void SendAck();
#endif
