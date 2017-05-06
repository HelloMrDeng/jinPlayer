// RockVPlayer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "RockVPlayer.h"

#include "CDlgPlayer.h"

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	CDlgPlayer dlgPlayer;
	dlgPlayer.Create (hInstance);
	return TRUE;
}
