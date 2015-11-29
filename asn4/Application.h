#ifndef APPLICATION_H
#define APPLICATION_H
#include <Windows.h>
#include "stdio.h"
#include "Menu.h"
#include "Time.h"
#include "Session.h"
#include "Protocol.h"

#define BTN_WIDTH 75
#define BTN_HEIGHT 50
#define BTN_XSTART 450
#define BTN_YSTART 0
#define BTN_BUFFER 5
#define MAX_READ 50000

int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE prevInstance, LPSTR lpCmdline, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void OpenFileDialog();
void ClearTextBoxes();
void SetStatistics();
static DWORD WINAPI ReadFromPort(LPVOID lpParam);
#endif