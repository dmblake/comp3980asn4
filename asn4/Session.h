#ifndef SESSION_H
#define SESSION_H
#include <Windows.h>
#include "stdio.h"
void Packetize(CHAR *buf, CHAR *packet);
BOOL Depacketize(CHAR *packet);
#endif