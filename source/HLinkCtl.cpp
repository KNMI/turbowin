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

#ifndef HLINK_NOOLE
#define INITGUID
#endif

#ifndef HLINK_NOOLE
#include <initguid.h>
#endif

#include <winnetwk.h>
#include <winnls.h>
#include <shlobj.h>

#ifndef HLINK_NOOLE
#include <intshcut.h>
#endif

#include "hlinkctl.rh"
#include "hlinkctl.h"

// ***************************** TWaitCursor **********************************

#if (OWLInternalVersion < 0x06000000L)
TWaitCursor::TWaitCursor(TModule* module, TResId resId): OldCursor(0)
{
  HCURSOR cursor = LoadCursor(module ? (HINSTANCE) *module : 0, resId);
  if (cursor)
    OldCursor = SetCursor(cursor);
}

TWaitCursor::~TWaitCursor()
{
  if (OldCursor)
    SetCursor(OldCursor);
}
#endif

// ***************************** THLinkCtrl ***********************************

DEFINE_RESPONSE_TABLE1(THLinkCtrl, TEdit)
  //EV_WM_CONTEXTMENU,              // martin uitgeschakeld 07-01-2004 gaf geen mooie overblijfselen na klikken i.v.m. transparant zijn
  EV_WM_SETCURSOR,
  //EV_WM_ERASEBKGND,
  EV_WM_PAINT,
  EV_WM_LBUTTONDOWN,
  EV_WM_SETFOCUS,
  EV_WM_MOUSEMOVE,
  EV_COMMAND(ID_POPUP_COPYSHORTCUT, EvCopyShortcut),
  EV_COMMAND(ID_POPUP_PROPERTIES, EvProperties),
  EV_COMMAND(ID_POPUP_OPEN, EvOpen),
#ifndef HLINK_NOOLE
  EV_COMMAND(ID_POPUP_ADDTOFAVORITES, EvAddToFavorites),
  EV_COMMAND(ID_POPUP_ADDTODESKTOP, EvAddToDesktop),
#endif
END_RESPONSE_TABLE;

THLinkCtrl::THLinkCtrl(TWindow* parent, int Id, LPCTSTR text, int x,
	int y, int w, int h, uint textLimit, bool multiline, TModule* module):
  TEdit(parent, Id, text, x, y, w, h, textLimit, multiline, module)
{
  Init();
}

THLinkCtrl::THLinkCtrl(TWindow* parent, int resourceId, uint textLimit,
	TModule* module):
  TEdit(parent, resourceId, textLimit, module)
{
  Init();
}

THLinkCtrl::~THLinkCtrl()
{
}

void THLinkCtrl::Init()
{
	m_Color = RGB(0, 0, 255);
	m_VisitedColor = RGB(128, 0, 128);
	m_HighlightColor = RGB(255, 0, 0);

	m_bShrinkToFit = true;
   m_bUseHighlight = true;
   m_bUseShadow = true;
	m_State = ST_NOT_VISITED;
   m_OldState = ST_NOT_VISITED;
   m_bUnderline = true;
   m_bShowingContext = false;
   m_bShowPopupMenu = true;

	// Load up the cursors
   PRECONDITION(GetApplicationObject());
	m_hLinkCursor = GetApplication()/*GetApplicationObject()*/->LoadCursor(TResId(IDC_HLINK));
   m_hArrowCursor = ::LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW));

	// Control should be created read only
	CHECK(GetStyle() & ES_READONLY);

	//  Control should not be in the tab order
	CHECK(!(GetStyle() & WS_TABSTOP));
}

void THLinkCtrl::ShrinkToFitEditBox()
{
	TWindow* pParent = GetParentO();
	if (pParent == 0)
		return;

	TClientDC dc(GetHandle()/*HWindow*/);
	TSize TextSize = dc.GetTextExtent(GetHyperLinkDescription().c_str(),
  	_tcslen(GetHyperLinkDescription().c_str()));

	TRect ControlRect = GetWindowRect();
	pParent->ScreenToClient(ControlRect.TopLeft());
   pParent->ScreenToClient(ControlRect.BottomRight());

   int width = min((long)ControlRect.Size().cx, TextSize.cx);
	if (ControlRect.Size().cx != width)
	{
      // we need to Shink the edit box
      if (GetStyle() & ES_RIGHT)
      {
			// keep everything the same except the width. Set that to the width of text
			MoveWindow(ControlRect.right-width,
      	ControlRect.top, width, ControlRect.Height());
      }
      else if (GetStyle() & ES_CENTER)
      {
         MoveWindow(ControlRect.left+(ControlRect.Width()-width)/2,
      	ControlRect.top, width, ControlRect.Height());
      }
      else
      {
	      MoveWindow(ControlRect.left,
  	      ControlRect.top, width, ControlRect.Height());
		}
	}
}

#if (OWLInternalVersion >= 0x06000000L)
void THLinkCtrl::SetHyperLink(const owl_string& sActualLink)
{
	SetActualHyperLink(sActualLink);

  // if the edit field has no caption, set the description with the hyperlink
  //
  owl_string str = GetHyperLinkDescription();
#else
void THLinkCtrl::SetHyperLink(const string& sActualLink)
{
	SetActualHyperLink(sActualLink);

  // if the edit field has no caption, set the description with the hyperlink
  //
  string str = GetHyperLinkDescription();
#endif
  if (_tcslen(str.c_str()) == 0)
  		SetHyperLinkDescription(sActualLink);

	if (m_bShrinkToFit)
		ShrinkToFitEditBox();
}

#if (OWLInternalVersion >= 0x06000000L)
void THLinkCtrl::SetHyperLink(const owl_string& sActualLink,
	const owl_string& sLinkDescription, bool bShrinkToFit)
#else
void THLinkCtrl::SetHyperLink(const string& sActualLink,
	const string& sLinkDescription, bool bShrinkToFit)
#endif
{
  m_bShrinkToFit = bShrinkToFit;
	SetActualHyperLink(sActualLink);
	SetHyperLinkDescription(sLinkDescription);
	if (m_bShrinkToFit)
		ShrinkToFitEditBox();
}

#if (OWLInternalVersion >= 0x06000000L)
void THLinkCtrl::SetActualHyperLink(const owl_string& sActualLink)
#else
void THLinkCtrl::SetActualHyperLink(const string& sActualLink)
#endif
{
	m_sActualLink = sActualLink;
}

#if (OWLInternalVersion >= 0x06000000L)
void THLinkCtrl::SetHyperLinkDescription(const owl_string& sLinkDescription)
#else
void THLinkCtrl::SetHyperLinkDescription(const string& sLinkDescription)
#endif
{
  // if empty do nothing
	if (_tcslen(sLinkDescription.c_str()) != 0)
  {
		//  set the text on this control
		SetWindowText(sLinkDescription.c_str());
  }
}

#if (OWLInternalVersion >= 0x06000000L)
owl_string THLinkCtrl::GetHyperLinkDescription() const
#else
string THLinkCtrl::GetHyperLinkDescription() const
#endif
{
  int len = GetWindowTextLength();
  if (len > 0)
  {
    LPTSTR str = new TCHAR[len+1];
  	GetWindowText(str, len+1);
#if (OWLInternalVersion >= 0x06000000L)
 	  owl_string sDescription(str);
#else
 	  string sDescription(str);
#endif

    delete[] str;
	  return sDescription;
  }
  else
  {
#if (OWLInternalVersion >= 0x06000000L)
    owl_string str(_T(""));
#else
    string str(_T(""));
#endif
    return str;
  }
}

bool THLinkCtrl::EvSetCursor(THandle /*hWndCursor*/, uint /*hitTest*/,
	uint /*mouseMsg*/)
{
   if (m_bShowingContext)
     ::SetCursor(m_hArrowCursor);
   else
  	  ::SetCursor(m_hLinkCursor);
	return true;
}

void THLinkCtrl::EvLButtonDown(uint /*modKeys*/, TPoint& /*point*/)
{
  PostMessage(WM_COMMAND, ID_POPUP_OPEN);
}

void THLinkCtrl::EvSetFocus(HWND /*hWndLostFocus*/)
{
	// TEdit::EvSetFocus(pOldWnd);  // Eat the message
}

void THLinkCtrl::EvOpen()
{
  if (Open())
	  m_State = ST_VISITED;

    m_bUseShadow = false;                                           /* toegevoegd martin: 18-04-2006 */
	 m_HighlightColor = RGB(128, 0, 128);                            /* toegevoegd martin: 18-04-2006 */
}


void THLinkCtrl::SetLinkColor(const COLORREF& color)
{
	m_Color = color;
	UpdateWindow();
}

void THLinkCtrl::SetVisitedLinkColor(const COLORREF& color)
{
	m_VisitedColor = color;
	UpdateWindow();
}

void THLinkCtrl::SetHighlightLinkColor(const COLORREF& color)
{
	m_HighlightColor = color;
	UpdateWindow();
}

void THLinkCtrl::SetUseUnderlinedFont(bool bUnderline)
{
   m_bUnderline = bUnderline;
   UpdateWindow();
}

void THLinkCtrl::SetUseShadow(bool bUseShadow)
{
   m_bUseShadow = bUseShadow;
   UpdateWindow();
}

void THLinkCtrl::EvMouseMove(UINT modKeys, TPoint& point)
{
   if (!m_bUseHighlight)
     return;

	TRect rc;
	GetClientRect(rc);
	if (rc.Contains(point))
	{
		if (m_State != ST_HIGHLIGHTED)
		{
			SetCapture();
			HighLight(true);
		}
	}
	else
	{
		if (m_State == ST_HIGHLIGHTED)
		{
			HighLight(false);
			ReleaseCapture();
		}
	}
	TEdit::EvMouseMove(modKeys, point);
}

void THLinkCtrl::HighLight(bool state)
{
	if (state)
	{
		if (m_State != ST_HIGHLIGHTED)
		{
			m_OldState = m_State;
			m_State = ST_HIGHLIGHTED;
			Invalidate();
		}
	}
	else
	{
		if (m_State == ST_HIGHLIGHTED)
		{
			m_State = m_OldState;
			Invalidate();
		}
	}
}

bool THLinkCtrl::EvEraseBkgnd(HDC hdc)
{
	bool bSuccess = TEdit::EvEraseBkgnd(hdc);

  TRect r = GetClientRect();
  TDC dc(hdc);

	if ((m_State == ST_HIGHLIGHTED) && m_bUseShadow)
	{
    // draw the drop shadow effect if highlighted
    TFont wFont(GetWindowFont());
    LOGFONT lf;
    wFont.GetObject(lf);
    if (m_bUnderline)
      lf.lfUnderline = true;
    TFont font(&lf);
    dc.SelectObject(font);
    int nOldMode = dc.SetBkMode(TRANSPARENT);
    TCHAR sText[255];
    GetWindowText(sText, 255);
    COLORREF OldColor = dc.SetTextColor(GetSysColor(COLOR_3DSHADOW));
    r.Offset(2,2);
    uint16 format = DT_VCENTER;
    if (GetStyle() & ES_RIGHT)
    {
      format |= DT_RIGHT;
      r.right-=2;
    }
    else if (GetStyle() & ES_CENTER)
      format |= DT_CENTER;
    else
      format |= DT_LEFT;
    dc.DrawText(sText, _tcslen(sText), r, format);
    dc.SetTextColor(OldColor);
    dc.SetBkMode(nOldMode);
    dc.RestoreFont();
  }
  else
  {
    //dc.FillRect(r, TBrush(GetSysColor(COLOR_3DFACE)));   // origineel: maakt de link-edit control grijs
    //dc.FillRect(r, TBrush(TColor::White));               // martin: maakt de link-edit control wit
    dc.FillRect(r, TBrush(GetSysColor(COLOR_WINDOW)));     // martin
  }

  return bSuccess;
}

void THLinkCtrl::EvPaint()
{
	TPaintDC dc(GetHandle()/*HWindow*/);

  // Make the font underline just like a URL
  TFont wFont = GetWindowFont();
  LOGFONT lf;
  wFont.GetObject(lf);
  lf.lfUnderline = m_bUnderline;
  TFont font(&lf);
  dc.SelectObject(font);
  int nOldMode = dc.SetBkMode(TRANSPARENT);

  COLORREF OldColor;
  switch (m_State)
  {
		case ST_NOT_VISITED:
    	     OldColor = dc.SetTextColor(m_Color);
           break;
		case ST_VISITED:
    	     OldColor = dc.SetTextColor(m_VisitedColor);
           break;
		case ST_HIGHLIGHTED:
    	     OldColor = dc.SetTextColor(m_HighlightColor);
           break;
  }

  TRect r = GetClientRect();
  uint16 format = DT_VCENTER;
  if (GetStyle() & ES_RIGHT)
  {
    format |= DT_RIGHT;
    r.right-=2;
  }
  else if (GetStyle() & ES_CENTER)
    format |= DT_CENTER;
  else
    format |= DT_LEFT;

  dc.DrawText(GetHyperLinkDescription().c_str(),
  	_tcslen(GetHyperLinkDescription().c_str()), r, format);

  dc.SetTextColor(OldColor);
  dc.SetBkMode(nOldMode);
  dc.RestoreFont();
}

void THLinkCtrl::EvContextMenu(HWND /*childHwnd*/, int x, int y)
{
   if (m_bShowPopupMenu == false)
  	   return;

   HighLight(false);
	   ReleaseCapture();

	if (x == -1 && y == -1)
   {
		//  keystroke invocation
		TRect rect = GetClientRect();
		ClientToScreen(rect.TopLeft());
		x = rect.left + 5;
     y = rect.top + 5;
	}

   PRECONDITION(GetApplicationObject());
	TMenu menu(GetApplication()/*GetApplicationObject()*/->GetHandle()/*GetInstance()*/, TResId(IDR_HLINK_POPUP));

	HMENU pPopup = menu.GetSubMenu(0);
	PRECONDITION(pPopup);


   m_bShowingContext = true;
	::TrackPopupMenu(pPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, GetHandle()/*HWindow*/, 0);
   m_bShowingContext = false;
}

void THLinkCtrl::EvCopyShortcut()
{
   if (::OpenClipboard(GetHandle()/*HWindow*/))
   {
      int nBytes = sizeof(TCHAR) * (_tcslen(m_sActualLink.c_str()) + 1);
      HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, nBytes);
      TCHAR* pData = (TCHAR*) GlobalLock(hMem);
      _tcscpy(pData, (LPCTSTR) m_sActualLink.c_str());
      GlobalUnlock(hMem);
  	   SetClipboardData(CF_TEXT, hMem);
      CloseClipboard();
   }
}

void THLinkCtrl::EvProperties()
{
	ShowProperties();
}

void THLinkCtrl::ShowProperties() const
{
	THLinkSheet propSheet(GetParentO());
  propSheet.SetBuddy(CONST_CAST(THLinkCtrl*,this));
  propSheet.Execute();
}

bool THLinkCtrl::Open() const
{
  // Give the user some feedback
  TWaitCursor cursor;

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
bool THLinkCtrl::OpenUsingCom() const
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
  ivci.hwndParent = (HWND__*)GetParent(); /*GetParent();*/
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

bool THLinkCtrl::OpenUsingShellExecute() const
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

#ifndef HLINK_NOOLE
bool THLinkCtrl::AddToSpecialFolder(int nFolder) const
{
  // Give the user some feedback
  TWaitCursor cursor;

  // Get the shell's allocator.
  IMalloc* pMalloc;
  if (!SUCCEEDED(SHGetMalloc(&pMalloc)))
  {
    TRACE("Failed to get the shell's IMalloc interface\n");
    return false;
  }

  // Get the location of the special Folder required
  LPITEMIDLIST pidlFolder;
  HRESULT hres = SHGetSpecialFolderLocation(0, nFolder, &pidlFolder);
  if (!SUCCEEDED(hres))
  {
    TRACE("Failed in call to SHGetSpecialFolderLocation\n");
    pMalloc->Release();
    return false;
  }

  // convert the PIDL to a file system name and
  // add an extension of URL to create an Internet
  // Shortcut file
  TCHAR sFolder[_MAX_PATH];
  if (!SHGetPathFromIDList(pidlFolder, sFolder))
  {
    TRACE("Failed in call to SHGetPathFromIDList");
    pMalloc->Release();
    return false;
  }
  TCHAR sShortcutFile[_MAX_PATH];

#if (OWLInternalVersion >= 0x06000000L)
  owl_string linkDescription = GetHyperLinkDescription();
#else
  string linkDescription = GetHyperLinkDescription();
#endif
  _tmakepath(sShortcutFile, 0, sFolder, linkDescription.c_str(), _T("URL"));

  // Free the pidl
  pMalloc->Free(pidlFolder);

  // Do the actual saving
  bool bSuccess = Save(sShortcutFile);

  // Release the pointer to IMalloc
  pMalloc->Release();

  return bSuccess;
}
#endif

#ifndef HLINK_NOOLE
void THLinkCtrl::EvAddToFavorites()
{
  AddToSpecialFolder(CSIDL_FAVORITES);
}
#endif

#ifndef HLINK_NOOLE
void THLinkCtrl::EvAddToDesktop()
{
  AddToSpecialFolder(CSIDL_DESKTOP);
}
#endif

#ifndef HLINK_NOOLE
#if (OWLInternalVersion >= 0x06000000L)
bool THLinkCtrl::Save(const owl_string& sFilename) const
#else
bool THLinkCtrl::Save(const string& sFilename) const
#endif
{
  // Get the URL Com interface
  IUniformResourceLocator* pURL;
  HRESULT hres = CoCreateInstance(CLSID_InternetShortcut, 0,
    CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (void**) &pURL);
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

  // Get the IPersistFile interface for
  // saving the shortcut in persistent storage.
  IPersistFile* ppf;
  hres = pURL->QueryInterface(IID_IPersistFile, (void **)&ppf);
  if (!SUCCEEDED(hres))
  {
    TRACE("Failed to get the IPersistFile interface\n");
    pURL->Release();
    return false;
  }

  // Save the shortcut via the IPersistFile::Save member function.
#ifndef _UNICODE
  wchar_t wsz[_MAX_PATH];
  MultiByteToWideChar(CP_ACP, 0, sFilename.c_str(), -1, wsz, _MAX_PATH);
  hres = ppf->Save(wsz, true);
#else
  hres = ppf->Save(sFilename.c_str(), true);
#endif
  if (!SUCCEEDED(hres))
  {
    TRACE("IPersistFile::Save failed!\n");
    ppf->Release();
    pURL->Release();
    return false;
  }

  // Release the pointer to IPersistFile.
  ppf->Release();

  // Release the pointer to IUniformResourceLocator.
  pURL->Release();

  return true;
}
#endif

THLinkPage::THLinkPage(TPropertySheet* parent):
	TPropertyPage(parent, IDD_HLINK_PROPERTIES)
{
}

THLinkPage::~THLinkPage()
{
}

void THLinkPage::SetupWindow()
{
	TPropertyPage::SetupWindow();

	PRECONDITION(m_pBuddy);
   ::SetWindowText(::GetDlgItem(GetHandle()/*HWindow*/, IDC_NAME),
  	m_pBuddy->GetHyperLinkDescription().c_str());
   ::SetWindowText(::GetDlgItem(GetHandle()/*HWindow*/, IDC_URL),
  	m_pBuddy->GetActualHyperLink().c_str());
}

THLinkSheet::THLinkSheet(TWindow* parent)
	:TPropertySheet(parent, 0)
{
  m_page1 = new THLinkPage(this);
  SetCaption(GetModule()->LoadString(IDS_HLINK_PROPERTIES).c_str());
  Attr.Style |= (PSH_NOAPPLYNOW);
}

THLinkSheet::~THLinkSheet()
{
}

