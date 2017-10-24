#include <owl\owlpch.h>
#include "tw_17.rc"
#pragma hdrstp

#include "HLinkCtl.h"


struct TStationTypeTransfer
{
	TStationTypeTransfer();

   char StationPasswordEditResult[MAX_STATION_PASSWORD_LENGTH];
	uint16 StationTypeRadioButtonResult1;
	uint16 StationTypeRadioButtonResult2;
   uint16 StationTypeUTCCheckBoxResult;
   char StationTypeDelimiterResult[255];
	TListBoxData StationTypeListboxResult;
   char StationTypeBackupFolderResult[512];
};


struct TFixedTransfer
{
	TFixedTransfer();

	char FixedNameResult[255];
   char FixedIMOResult[256];
	char HeightAnemometerResult[10];
	uint16 FixedRadioButtonResultYesAnemometer;
	uint16 FixedRadioButtonResultNoAnemometer;
	TComboBoxData LatitudeDegreesResult;
	TComboBoxData LatitudeTenthsResult;
	uint16 FixedRadioButtonResultNorth;
	uint16 FixedRadioButtonResultSouth;
	TComboBoxData LongitudeDegreesResult;
	TComboBoxData LongitudeTenthsResult;
	uint16 FixedRadioButtonResultEast;
	uint16 FixedRadioButtonResultWest;
	TListBoxData DeterminingWavesResult;
};


struct TShipTransfer
{
	TShipTransfer();

	char ShipsNameResult[255];
   char IMOResult[256];
   uint16 messageformCLIresult;               // VOSClim
   uint16 messageformSELresult;
   uint16 messageformSUSresult;               // supplementary with sst
   uint16 messageformSUPresult;               // supplementary without sst
   uint16 messageformAUXresult;
   //uint16 VosClimCheckBoxResult;
   uint16 ASAPCheckBoxResult;
   uint16 SOOPCheckBoxResult;
   uint16 OtherCheckBoxResult;
   char OtherResult[128];
   char NationalGroupResult1[11];
   char NationalGroupResult2[11];
   char NationalGroupResult3[11];
   char NationalGroupResult4[11];
   char NationalGroupResult5[11];
	char HeightAnemometerShipResult[10];
	char SeawaterMeasurementResult[10];
   uint16 codeformAlphanumericResult;
   uint16 codeformBUFRResult;
   uint16 codeformCompressedResult;
   TComboBoxData DeterminingShipWavesResult;
   char BarometerSLLResult[10];
   char KeelSLLResult[10];
   uint16 GTSHeaderCheckBoxResult;
};



class TStationTypeDialog : public TDialog
{
	public :
		TStationTypeDialog(TWindow* parent, TResId resId, TStationTypeTransfer& stationtypetransfer,
								  string station_type, string recruiting_country,
								  string system_time, string delimiter, string turbowin_dir, string backup_folder);
		virtual ~TStationTypeDialog();
		void SetupWindow();

	protected :
		void CmHelp();
      void CmBackup();
      void CmLogonStation();

	private :
		string local_station_type;
		string local_recruiting_country;
		string local_system_time;
      string local_delimiter;
      string local_backup_folder;
      string local_turbowin_dir;

		TRadioButton* radiobutton1;
		TRadioButton* radiobutton2;
		TListBox* listboxrecruiting;
      TCheckBox* checkboxUTC;
      TEdit* edit_delimiter;
      TEdit* edit_backup;
      TStatic* text_turbowin;

      TEdit* stationpasswordedit;
      TButton* button_logon;
      TButton* ok_button;
      TButton* backup_button;

      /* voor HTMLHelp (chm) */
      typedef HWND WINAPI (*HtmlHelpFunc)( HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD dwData );
      HtmlHelpFunc HTMLHelp;
      HINSTANCE hHelpOCX;

      TFileSaveDialog::TData BackupFileData;

		DECLARE_RESPONSE_TABLE(TStationTypeDialog);
};


class TFixedDialog : public TDialog
{
	public :
		TFixedDialog(TWindow* parent, TResId resId, TFixedTransfer& fixedtransfer,
						 string hoogte_anemometer,
						 string anemometer_10_yes_no,
						 string breedte_graden, string breedte_tienden,
						 string halfrond_N_Z, string lengte_graden,
						 string lengte_tienden, string halfrond_W_E,
						 string golven_resultaat, string turbowin_dir,
                   string fixed_name, string fixed_imo_nummer);

		virtual ~TFixedDialog();
		void SetupWindow();
      bool print_button_pressed;
      
	protected :
	   void CmHelp();
      void CmPrint();

	private :
	   string local_hoogte_anemometer;
	   string local_anemometer_10_yes_no;
	   string local_breedte_graden;
	   string local_breedte_tienden;
	   string local_halfrond_N_Z;
	   string local_lengte_graden;
	   string local_lengte_tienden;
	   string local_halfrond_W_E;
	   string local_golven_resultaat;
      string local_turbowin_dir;
      string local_fixed_name;
      string local_fixed_imo_nummer;

      TEdit* fixedname;
      TEdit* fixedimonummer;
	   TEdit* heightanemometer;
	   TRadioButton* radiobutton_yes_anemometer;
	   TRadioButton* radiobutton_no_anemometer;
	   TComboBox* latitudedegrees;
	   TComboBox* latitudetenths;
	   TRadioButton* radiobutton_North;
	   TRadioButton* radiobutton_South;
	   TComboBox* longitudedegrees;
	   TComboBox* longitudetenths;
	   TRadioButton* radiobutton_East;
	   TRadioButton* radiobutton_West;
	   TListBox* determiningwaves;
      TStatic* text_turbowin;

      /* voor HTMLHelp (chm) */
      typedef HWND WINAPI (*HtmlHelpFunc)( HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD dwData );
      HtmlHelpFunc HTMLHelp;
      HINSTANCE hHelpOCX;

	   DECLARE_RESPONSE_TABLE(TFixedDialog);
};


class TShipDialog : public TDialog
{
	public :
	  TShipDialog(TWindow* parent, TResId resId, TShipTransfer& shiptransfer,
					  string scheepsnaam, string hoogte_anemometer_ship, string diepte_zeewater,
                 string nationale_groep_1,
                 string nationale_groep_2,
                 string nationale_groep_3,
                 string nationale_groep_4,
                 string nationale_groep_5,
                 /*string vos_clim_deelnemer,*/ string asap_deelnemer, string soop_deelnemer, string other_deelnemer,
                 string message_mode, string turbowin_dir, string imo_number,
                 string code_mode, string ship_golven_resultaat, string recruiting_country, /*string gts_header_resultaat,*/
                 string barometer_sll, string keel_sll, int start_option);

	  virtual ~TShipDialog();
	  void SetupWindow();
     bool print_button_pressed;

	protected :
	  void CmHelp();
     void CmPrint();

     THLinkCtrl* WebLinkCtrl;

	private :
     //void Ev_Checkbox_Vosclim_Indication_SelChange();
     void Ev_Checkbox_SOOP_Indication_SelChange();
     void Ev_Checkbox_ASAP_Indication_SelChange();

	  string local_scheepsnaam;
     string local_message_mode;                     // CLI, SEL, SUS, SUP of AUX
     //string local_vos_clim_deelnemer;
     string local_asap_deelnemer;
     string local_soop_deelnemer;
     string local_other_deelnemer;
     string local_nationale_groep_1;
     string local_nationale_groep_2;
     string local_nationale_groep_3;
     string local_nationale_groep_4;
     string local_nationale_groep_5;
	  string local_hoogte_anemometer_ship;
	  string local_diepte_zeewater;
     string local_turbowin_dir;
     string local_imo_number;
     string local_code_mode;                        // ALPHANUMERIC , BUFR
     string local_ship_golven_resultaat;
     string local_recruiting_country;
     //string local_gts_header_resultaat;
     string local_barometer_sll;
     string local_keel_sll;
     int local_start_option;

	  TEdit* shipsname;
     TEdit* imoedit;
     TRadioButton* radiobutton_cli;
     TRadioButton* radiobutton_sel;
     TRadioButton* radiobutton_sus;                 // supplementary with sst
     TRadioButton* radiobutton_sup;                 // supplementary without sst
     TRadioButton* radiobutton_aux;
     TRadioButton* radiobutton_alpha;
     TRadioButton* radiobutton_bufr;
     TRadioButton* radiobutton_compressed;
     //TCheckBox* vos_clim_checkbox;
     TCheckBox* asap_checkbox;
     TCheckBox* soop_checkbox;
     TCheckBox* other_checkbox;
     TEdit* edit_other;
     TEdit* edit_national_1;
     TEdit* edit_national_2;
     TEdit* edit_national_3;
     TEdit* edit_national_4;
     TEdit* edit_national_5;
	  TEdit* heightanemometer;
	  TEdit* seawatermeasurement;
     TStatic* text_turbowin;
	  TComboBox* determiningshipwaves;
     TEdit* edit_barometer_sll;
     TEdit* edit_keel_sll;
     //TCheckBox* gts_header_checkbox;
     TButton* print_button;

     /* voor HTMLHelp (chm) */
     typedef HWND WINAPI (*HtmlHelpFunc)( HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD dwData );
     HtmlHelpFunc HTMLHelp;
     HINSTANCE hHelpOCX;

	  DECLARE_RESPONSE_TABLE(TShipDialog);
};




