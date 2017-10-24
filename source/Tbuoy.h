#include <owl\owlpch.h>
#include "tw_17.rc"
#pragma hdrstop



struct TBuoyTransfer
{
	TBuoyTransfer();

   TListBoxData ListboxResults;
   char IDResult[30];                   // dit geeft ook max aantal char aan dat kan worden ingevuld (weer van belang voor tonen in listbox)
   char TypeResult[20];                 //             --""--
   char DateResult[15];                 //             --""--
   char TimeResult[15];                 //             --""--
   char LatResult[15];                  //             --""--
   char LongResult[15];                 //             --""--
   char HeightResult[15];
   char MethodResult[30];
   char PressureResult[15];
   char SSTResult[15];
   char AirResult[15];
   char WindspeedResult[15];
   char WinddirResult[15];
   char WaveheightResult[15];
   char ShipspeedResult[15];
   char ConditionResult[30];
   char BehalfResult[30];
   char MasterResult[30];
   char ShipnameResult[60];
   TListBoxData RecruitingListboxResult;
   char FreetextResult[512];
};


class TBuoyDialog : public TDialog
{
	public :
      TBuoyDialog(TWindow* parent, TResId resId, TBuoyTransfer& buoytransfer, string scheepsnaam, string* buoy_array, string turbowin_dir, string* captain_array);
		virtual ~TBuoyDialog();
      void SetupWindow();
      void Bepaal_Buoy_Email_Address();
      bool email_button_pressed;
      bool floppy_button_pressed;
      bool print_button_pressed;
      string buoy_email_address;

	protected :
		void CmHelp();
      void CmEmail();
      void CmPrint();
      void CmFloppy();
      void CmLog();
      
   private :
      void Ev_listbox_recruiting_SelChange();
      void Bepaal_Buoy_Recruiting_Country_Email_Address();

	   TListBox* listbox;
      TEdit* id_edit;
      TEdit* type_edit;
      TEdit* date_edit;
      TEdit* time_edit;
      TEdit* lat_edit;
      TEdit* long_edit;
      TEdit* height_edit;
      TEdit* method_edit;
      TEdit* pressure_edit;
      TEdit* sst_edit;
      TEdit* air_edit;
      TEdit* windspeed_edit;
      TEdit* winddir_edit;
      TEdit* waveheight_edit;
      TEdit* shipspeed_edit;
      TEdit* condition_edit;
      TEdit* behalf_edit;
      TEdit* master_edit;
      TEdit* shipname_edit;
      TListBox* recruiting_listbox;
      TEdit* freetext_edit;
      TStatic* text_turbowin;
      TButton* email_button;
      TButton* print_button;
      TButton* floppy_button;

      string local_turbowin_dir;
      string local_scheepsnaam;
      string buoy_recruiting_country;
      string local_buoy_array[MAX_AANTAL_BUOYS];
      string captain[MAX_AANTAL_CAPTAINS];

      /* voor HTMLHelp (chm) */
      typedef HWND WINAPI (*HtmlHelpFunc)( HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD dwData );
      HtmlHelpFunc HTMLHelp;
      HINSTANCE hHelpOCX;

   DECLARE_RESPONSE_TABLE(TBuoyDialog);
};



