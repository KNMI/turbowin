// ****************************************************************************
// Copyright (C) 1998,1999 by Dieter Windau
// All rights reserved
//
// HLinkCtl.cpp: implementation file
// Version:      1.01
// Date:         04/23/1999
// Author:       Dieter Windau
//
// THLinkCtrl is a freeware OWL class that supports Internet Hyperlinks from
// standard Windows applications just like they are displayed in Web browsers.
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// The code was tested using Microsoft Visual C++ 6.0 SR2 with OWL6 patch 5
// and Borland C++ 5.02 with OWL 5.02. Both under Windows NT 4.0 SP4.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// This code is based on MFC class CHLinkCtrl by PJ Naughter
// Very special thanks to PJ Naughter:
//   EMail: pjn@indigo.ie
//   Web:   http://indigo.ie/~pjn
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://www.members.aol.com/softengage/index.htm
// ****************************************************************************
#include <owl\pch.h>
#pragma hdrstop

//#ifndef HLINK_NOOLE             // staat al in hlinkctl.cpp
//#define INITGUID                // staat al in hlinkctl.cpp
//#endif                          // staat al in hlinkctl.cpp

//#ifndef HLINK_NOOLE             // staat al in hlinkctl.cpp
//#include <initguid.h>           // staat al in hlinkctl.cpp
//#endif

//#include <winnetwk.h>           // staat al in hlinkctl.cpp
//#include <winnls.h>             // staat al in hlinkctl.cpp
//#include <shlobj.h>             // staat al in hlinkctl.cpp

#ifndef HLINK_NOOLE
#include <intshcut.h>
#endif

//#include "hlinkctl.rh"
#include "hlinkmenu.h"

// ***************************** TWaitCursor **********************************

//#if (OWLInternalVersion < 0x06000000L)
TWaitCursor2::TWaitCursor2(TModule* module, TResId resId): OldCursor(0)
{
  HCURSOR cursor = LoadCursor(module ? (HINSTANCE) *module : 0, resId);
  if (cursor)
    OldCursor = SetCursor(cursor);
}

TWaitCursor2::~TWaitCursor2()
{
  if (OldCursor)
    SetCursor(OldCursor);
}
//#endif

// ***************************** THLinkMenu ***********************************


THLinkMenu::THLinkMenu()                  // origineel: THLinkMenu::THLinkMenu(const string sActualLink)
//**THLinkMenu::THLinkMenu(TWindow* parent, int resourceId):
//**                       TEdit(parent, resourceId)
{
   //m_sActualLink = sActualLink;
}


THLinkMenu::~THLinkMenu()
{
}


bool THLinkMenu::Open(const string sActualLink)    // bool THLinkMenu::Open() const
{
  // Give the user some feedback
  TWaitCursor2 cursor;

  m_sActualLink = sActualLink;

#ifndef HLINK_NOOLE
  // First try to open using IUniformResourceLocator
  bool bSuccess = OpenUsingCom();

  // As a last resort try ShellExecuting the URL, may
  // even work on Navigator!
  if (!bSuccess)
    bSuccess = OpenUsingShellExecute();
#else
  bool bSuccess = OpenUsingShellExecute();
#endif
  return bSuccess;
}

#ifndef HLINK_NOOLE
bool THLinkMenu::OpenUsingCom() const
{
  // Get the URL Com interface
  IUniformResourceLocator* pURL;
  HRESULT hres = CoCreateInstance(CLSID_InternetShortcut, 0,
  	CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (void**)&pURL);
  if (!SUCCEEDED(hres))
  {
    TRACE("Failed to get the IUniformResourceLocator interface\n");
    return false;
  }

  hres = pURL->SetURL(m_sActualLink.c_str(), IURL_SETURL_FL_GUESS_PROTOCOL);
  if (!SUCCEEDED(hres))
  {
    TRACE("Failed in call to SetURL\n");
    pURL->Release();
    return false;
  }

  //  Open the URL by calling InvokeCommand
  URLINVOKECOMMANDINFO ivci;
  ivci.dwcbSize = sizeof(URLINVOKECOMMANDINFO);
  ivci.dwFlags = IURL_INVOKECOMMAND_FL_ALLOW_UI;
  ivci.hwndParent = (HWND__*)GetParent();/*GetParent();*/
  ivci.pcszVerb = _T("open");
  hres = pURL->InvokeCommand(&ivci);
  if (!SUCCEEDED(hres))
  {
    TRACE("Failed to invoke URL using InvokeCommand\n");
    pURL->Release();
    return false;
  }

  //  Release the pointer to IUniformResourceLocator.
  pURL->Release();
  return true;
}
#endif

bool THLinkMenu::OpenUsingShellExecute() const
{
	HINSTANCE hRun = ShellExecute((HWND__*)GetParent(), _T("open"),/*ShellExecute(GetParent(), _T("open"),*/
  	m_sActualLink.c_str(), 0, 0, SW_SHOW);
  if ((int) hRun <= 32)
  {
    TRACE("Failed to invoke URL using ShellExecute\n");
    return false;
  }
  return true;
}

