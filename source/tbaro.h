#include <owl\owlpch.h>
#include "tw_17.rc"
#pragma hdrstop


struct TBarometerTransfer
{
	TBarometerTransfer();

	TComboBoxData BarometerResult;
	TComboBoxData BarometertiendenResult;
   TComboBoxData BarometerReadingIndicationResult;
   char BarometerDraftResult[100];
   char BarometerReadingObserverResult[100];
   char BarometerHeightResult[100];
   char BarometerCorrectionResult[100];
   char BarometerMSLResult[100];
};



class TBarometerDialog : public TDialog
{
	public :
		TBarometerDialog(TWindow* parent, TResId resId, TBarometerTransfer& barometertransfer,
                       string barometer_MSL_yes_no, string hoogte_barometer, string message_mode,
                       string turbowin_dir, bool in_next_sequence, string recruiting_country, string station_type,
                       string keel_sll, string barometer_sll, int num_TTT);
		virtual ~TBarometerDialog();
      bool back_button_pressed;
      bool stop_button_pressed;
      bool refresh_button_pressed;
      //string test;

	protected :
      void SetupWindow();
		void CmHelp();
      void CmBack();
      void CmStop();
      void CmRefresh();
      string Local_MSL_Herleiding_Luchtdruk(float hulp_num_pressure_total, string str_hulp_deepest_draft, string str_hulp_reading_indication, string local_station_type);

	private :
      void Ev_listbox_Barometer_Indication_SelChange();
      void Clearing_Na_Onmogelijke_Refresh();

		TComboBox* comboboxairpressure;
		TComboBox* comboboxairpressuretienden;
      TComboBox* reading_indication;
      TEdit* deepest_draft;
      TEdit* height_correction;
      TEdit* height_baro;
      TEdit* supplied_observer;
      TEdit* pressure_msl;
      TStatic* text_barometer_tenths;
      TStatic* text_draft;
      TStatic* text_fixed_sea_station;
      TStatic* text_turbowin;
      TStatic* text_height_barometer;
      TStatic* text_height_correction;
      //TStatic* text_reading_observer;
      TButton* back_button;                                                // voor automatically next input form
      TButton* stop_button;                                                // voor automatically next input form
      TButton* refresh_button;

      string local_barometer_MSL_yes_no;
      string local_message_mode;
      string local_turbowin_dir;
      string local_recruiting_country;
      string local_station_type;
      bool local_in_next_sequence;
      string local_keel_sll;
      string local_barometer_sll;
      string local_hoogte_barometer;
      string str_hulp_local_hoogte_corr;
      string str_hulp_local_hoogte_barometer;
      int local_num_TTT;

      /* voor HTMLHelp (chm) */
      typedef HWND WINAPI (*HtmlHelpFunc)( HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD dwData );
      HtmlHelpFunc HTMLHelp;
      HINSTANCE hHelpOCX;

		DECLARE_RESPONSE_TABLE(TBarometerDialog);
};



