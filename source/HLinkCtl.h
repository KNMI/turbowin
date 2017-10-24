// ****************************************************************************
// Copyright (C) 1998,1999 by Dieter Windau
// All rights reserved
//
// HLinkCtl.h:   header file
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
#ifndef HLINKCTL_H
#define HLINKCTL_H

#if !defined(OWL_EDIT_H)
#include <owl\edit.h>
#endif
#if !defined(OWL_PROPSHT_H)
#include <owl\propsht.h>
#endif

// ***************************** TWaitCursor **********************************

#if (OWLInternalVersion < 0x06000000L)
class TWaitCursor
{
public:
  TWaitCursor(TModule* module = 0, TResId resId = IDC_WAIT);
  ~TWaitCursor();

private:
  HCURSOR OldCursor;
};
#endif

// ***************************** THLinkCtrl ***********************************

class THLinkCtrl: public TEdit
{
public:
	//Constructors / Destructors
   THLinkCtrl(TWindow* parent, int Id, LPCTSTR text, int x, int y,
   int w, int h, uint textLimit = 0, bool multiline = false,
   TModule* module = 0);
	THLinkCtrl(TWindow* parent, int resourceId, uint textLimit = 0,
   TModule* module = 0);
	virtual ~THLinkCtrl();

#if (OWLInternalVersion >= 0x06000000L)
	//Set or get the hyperlink to use
	void SetHyperLink(const owl_string& sActualLink);
	void SetHyperLink(const owl_string& sActualLink,
	const owl_string& sLinkDescription, bool bShrinkToFit=true);
	owl_string GetActualHyperLink() const { return m_sActualLink; };

   //Set or get the hyperlink description (really just the window text)
   void SetHyperLinkDescription(const owl_string& sDescription);
	owl_string GetHyperLinkDescription() const;
#else
	//Set or get the hyperlink to use
	void SetHyperLink(const string& sActualLink);
	void SetHyperLink(const string& sActualLink,
	const string& sLinkDescription, bool bShrinkToFit=true);
	string GetActualHyperLink() const { return m_sActualLink; };

   //Set or get the hyperlink description (really just the window text)
   void SetHyperLinkDescription(const string& sDescription);
	string GetHyperLinkDescription() const;
#endif

	//Set or get the hyperlink color
	void SetLinkColor(const COLORREF& color);
	COLORREF GetLinkColor() { return m_Color; }

	//Set or get the hyperlink color for visited links
	void SetVisitedLinkColor(const COLORREF& color);
	COLORREF GetVisitedLinkColor() { return m_VisitedColor; }

	//Set or get the hyperlink color for highlighted links
	void SetHighlightLinkColor(const COLORREF& color);
	COLORREF GetHighlightLinkColor() { return m_HighlightColor; };
   void SetUseHighlightColor(bool bUseHighlight) { m_bUseHighlight = bUseHighlight; }

   //Set or get whether the hyperlink should use an underlined font
	void SetUseUnderlinedFont(bool bUnderline);
	bool GetUseUnderlinedFont() { return m_bUnderline; }

   //Set or get whether the hyperlink should use a drop shadow
	void SetUseShadow(bool bUseShadow);
	bool GetUseShadow() { return m_bUseShadow; }

   //Set or get whether the hyperlink should show a popup menu on right button click
   void SetShowPopupMenu(bool bShowPopupMenu) { m_bShowPopupMenu = bShowPopupMenu; }
   bool GetShowPopupMenu() { return m_bShowPopupMenu; }

	//Gets whether the hyperlink has been visited
	bool GetVisited() { return m_State == ST_VISITED; }

   //Gets whether the window is automatically adjusted to
   //the size of the description text displayed
	bool GetShrinkToFit()	{ return m_bShrinkToFit; }

   //Saves the hyperlink to an actual shortcut on file
#ifndef HLINK_NOOLE
#if (OWLInternalVersion >= 0x06000000L)
   bool Save(const owl_string& sFilename) const;
#else
   bool Save(const string& sFilename) const;
#endif
#endif

   //Displays the properties dialog for this URL
   void ShowProperties() const;

   //Connects to the URL
   bool Open() const;

protected:
   enum State
   {
     ST_NOT_VISITED,
     ST_VISITED,
     ST_HIGHLIGHTED
   };

protected:
   void Init();

	bool EvSetCursor(THandle hWndCursor, uint hitTest, uint mouseMsg);
	void EvLButtonDown(uint modKeys, TPoint& point);
	void EvMouseMove(UINT modKeys, TPoint& point);
	bool EvEraseBkgnd(HDC hdc);
	void EvPaint();
	void EvCopyShortcut();
	void EvProperties();
	void EvOpen();
	void EvSetFocus(HWND hWndLostFocus);
   void EvContextMenu(HWND childHwnd, int x, int y);

#ifndef HLINK_NOOLE
	void EvAddToFavorites();
   void EvAddToDesktop();
#endif

#if (OWLInternalVersion >= 0x06000000L)
	void SetActualHyperLink(const owl_string& sActualLink);
#else
	void SetActualHyperLink(const string& sActualLink);
#endif

	void ShrinkToFitEditBox();
	void HighLight(bool state);

#ifndef HLINK_NOOLE
   bool AddToSpecialFolder(int nFolder) const;
   bool OpenUsingCom() const;
#endif
   bool OpenUsingShellExecute() const;

protected:

#if (OWLInternalVersion >= 0x06000000L)
	owl_string m_sLinkDescription;
	owl_string m_sActualLink;
#else
	string   m_sLinkDescription;
	string   m_sActualLink;
#endif
	HCURSOR  m_hLinkCursor;
   HCURSOR  m_hArrowCursor;
	COLORREF m_Color;
	COLORREF m_VisitedColor;
	bool     m_bShrinkToFit;
   bool     m_bUseHighlight;
   COLORREF m_HighlightColor;
   State    m_State;
   bool     m_bUnderline;
   bool     m_bUseShadow;
   State    m_OldState;
   bool     m_bShowingContext;
   bool     m_bShowPopupMenu;

	DECLARE_RESPONSE_TABLE(THLinkCtrl);
};

class THLinkPage: public TPropertyPage
{
public:
	THLinkPage(TPropertySheet* parent);
	~THLinkPage();
  void SetBuddy(THLinkCtrl* pBuddy)
  	{ m_pBuddy = pBuddy; }

protected:
	void SetupWindow();

protected:
   THLinkCtrl* m_pBuddy;
};

class THLinkSheet: public TPropertySheet
{
public:
	THLinkSheet(TWindow* parent);
	virtual ~THLinkSheet();
   void SetBuddy(THLinkCtrl* pBuddy)
  	{ m_page1->SetBuddy(pBuddy); }

protected:
   THLinkPage* m_page1;
};

#endif

