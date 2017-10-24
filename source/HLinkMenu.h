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
#ifndef HLINKMENU_H
#define HLINKMENU_H

//#if !defined(OWL_EDIT_H)
//#include <owl\edit.h>
//#endif
//#if !defined(OWL_PROPSHT_H)
//#include <owl\propsht.h>
//#endif

// ***************************** TWaitCursor **********************************

//#if (OWLInternalVersion < 0x06000000L)
class TWaitCursor2
{
public:
  TWaitCursor2(TModule* module = 0, TResId resId = IDC_WAIT);
  ~TWaitCursor2();

private:
  HCURSOR OldCursor;
};
//#endif

// ***************************** THLinkMneu ***********************************

class THLinkMenu : public TWindow
//**class THLinkMenu : public TEdit
{
   public:
      THLinkMenu();                            // origineel: THLinkMenu(const string sActualLink);
      //**THLinkMenu(TWindow* parent, int resourceId);
	   virtual ~THLinkMenu();
      bool Open(const string sActualLink);     // origineel: bool Open()  const;   //Connects to the URL

   protected:
#ifndef HLINK_NOOLE
      bool OpenUsingCom() const;
#endif
      bool OpenUsingShellExecute() const;

   protected:
      string m_sActualLink;

};

#endif  // #ifndef HLINKMENU_H
