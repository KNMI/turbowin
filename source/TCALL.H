#include <owl\owlpch.h>
#include "tw_17.rc"
#pragma hdrstop



struct TCallsignTransfer
{
	TCallsignTransfer();

	char CallsignEditResult[100];
   TComboBoxData EncryptionIndicationResult;
   char VOSidentifierEditResult[100];
   char VOSIDpasswordEditResult[MAX_STATION_PASSWORD_LENGTH];
};



class TCallsignDialog : public TDialog
{
	public :
		TCallsignDialog(TWindow* parent, TResId resId, TCallsignTransfer& callsigntransfer, string file_callsign, string turbowin_dir, string vosidentifier, string station_type, string file_encryption_identifier);
		virtual ~TCallsignDialog();
      void SetupWindow();

	protected :
		void CmHelp();
      void CmLogonVOSID();

	private :
      //void Ev_Edit_Call_Sign_Update();
      TComboBox* encryption_indication_combobox;
		TEdit* callsignedit;
      TEdit* vosidentifieredit;
      TStatic* text_callsignedit;
      TStatic* text_encrypted;
      TStatic* text_vosidentifieredit;
      TStatic* text_notevosid;
      TStatic* text_turbowin;
      TEdit* vosidpasswordedit;
      TStatic* text_vosidpasswordedit;
      TButton* button_logon;

      string local_station_type;
      string local_callsign;
      string local_vosidentifier;
      string local_file_encryption_identifier;
      string local_turbowin_dir;

      /* voor HTMLHelp (chm) */
      typedef HWND WINAPI (*HtmlHelpFunc)( HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD dwData );
      HtmlHelpFunc HTMLHelp;
      HINSTANCE hHelpOCX;

		DECLARE_RESPONSE_TABLE(TCallsignDialog);
};





