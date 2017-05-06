// jpUpdate.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "jpUpdate.h"
#include "CUpdateCopy.h"

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
   CUpdateCopy copy (hInstance);
   if (!copy.UpdateFiles ())
   {
	   MessageBox (NULL, _T("Copy file error!"), _T("Error"), MB_OK);
	   return S_FALSE;
   }

   return S_OK;
}
