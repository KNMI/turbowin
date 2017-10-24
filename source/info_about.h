#include <owl\owlpch.h>
#include "tw_17.rc"                                
#pragma hdrstop

#include "HLinkCtl.h"

//#define WS_EX_LAYERED 0x00080000                /* i.v.m. transparancy */
//#define LWA_COLORKEY  0x00000001                /* i.v.m. transparancy */
//#define LWA_ALPHA     0x00000002                /* i.v.m. transparancy */

class TInfoAboutDialog : public TDialog
{
	public :
		TInfoAboutDialog(TWindow* parent, TResId resId/*, bool win_ver_2000_xp_ok*/);
		virtual ~TInfoAboutDialog();

	protected :
		void SetupWindow();

      THLinkCtrl* WebLinkCtrl;
      //THLinkCtrl* EMailLinkCtrl;

   private:
      //typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags); /* i.v.m. transparancy */
      //lpfnSetLayeredWindowAttributes m_pSetLayeredWindowAttributes; /* i.v.m. transparancy */
      //bool local_win_ver_2000_xp_ok;

     HBRUSH EvCtlColor(HDC hDC, HWND hWndChild, UINT ctlType);

   DECLARE_RESPONSE_TABLE(TInfoAboutDialog);   
};

