#include <owl\owlpch.h>
#include <owl\listbox.h>
#include <owl\combobox.h>
#include <owl\edit.h>
#include <owl\checkbox.h>
#include <owl\radiobut.h>
#include <owl\validate.h>
#include <owl\groupbox.h>
#include <owl\printer.h>
#include <owl\hlpmanag.h>
#include <owl\opensave.h>
#include <owl\date.h>
#include <owl\time.h>
#pragma hdrstop

#include "stat_ty.h"



/***************************************************************************************/
//                                                                                     //
//                                                                                     //
//                            algemeen station type                                    //
//                                                                                     //
//                                                                                     //
/***************************************************************************************/
DEFINE_RESPONSE_TABLE1(TStationTypeDialog, TDialog)
	EV_COMMAND(IDHELP, CmHelp),
   EV_COMMAND(IDBACKUP, CmBackup),
   EV_COMMAND(IDLOGONSTATION, CmLogonStation),
END_RESPONSE_TABLE;


TStationTypeTransfer::TStationTypeTransfer()
{
	// initialisatie
	StationTypeRadioButtonResult1 = BF_UNCHECKED;
	StationTypeRadioButtonResult2 = BF_UNCHECKED;
	StationTypeUTCCheckBoxResult  = BF_UNCHECKED;

	// komt in alfabetische volgorde op scherm (zie resource workshop)
   StationTypeListboxResult.AddString(NOT_ASSIGNED);
	StationTypeListboxResult.AddString("Argentina");
	StationTypeListboxResult.AddString("Australia");
	StationTypeListboxResult.AddString("Belgium");
	StationTypeListboxResult.AddString("Brazil");
	StationTypeListboxResult.AddString("Canada");
	StationTypeListboxResult.AddString("Chile");
	StationTypeListboxResult.AddString("Denmark");
	StationTypeListboxResult.AddString("Finland");
	StationTypeListboxResult.AddString("France");
	StationTypeListboxResult.AddString("French Polynesia");
	StationTypeListboxResult.AddString("Democratic People's Republic of Korea");
	StationTypeListboxResult.AddString("Germany");
	StationTypeListboxResult.AddString("Greece");
	StationTypeListboxResult.AddString("Hong Kong, China");
	StationTypeListboxResult.AddString("Iceland");
	StationTypeListboxResult.AddString("India");
	StationTypeListboxResult.AddString("Ireland");
	StationTypeListboxResult.AddString("Israel");
	StationTypeListboxResult.AddString("Italy");
	StationTypeListboxResult.AddString("Japan");
	StationTypeListboxResult.AddString("Kenya");
	StationTypeListboxResult.AddString("Republic of Korea");
	StationTypeListboxResult.AddString("Netherlands");
   StationTypeListboxResult.AddString("Netherlands WMO");
	StationTypeListboxResult.AddString("Croatia");
	StationTypeListboxResult.AddString("New Zealand");
	StationTypeListboxResult.AddString("Norway");
	StationTypeListboxResult.AddString("Pakistan");
	StationTypeListboxResult.AddString("Philippines");
	StationTypeListboxResult.AddString("Poland");
	StationTypeListboxResult.AddString("Portugal");
	StationTypeListboxResult.AddString("St. Pierre and Miquelon");
	StationTypeListboxResult.AddString("Singapore");
	StationTypeListboxResult.AddString("South Africa");
	StationTypeListboxResult.AddString("Spain");
	StationTypeListboxResult.AddString("Sweden");
	StationTypeListboxResult.AddString("Switzerland");
	StationTypeListboxResult.AddString("Thailand");
	StationTypeListboxResult.AddString("Russian Federation");
	StationTypeListboxResult.AddString("United Kingdom of Great Britain and Northern Ireland");
	StationTypeListboxResult.AddString("United States of America");
	StationTypeListboxResult.AddString("Yugoslavia");
	StationTypeListboxResult.AddString("Bulgaria");
	StationTypeListboxResult.AddString("Bangladesh");
	StationTypeListboxResult.AddString("Cuba");
	StationTypeListboxResult.AddString("Jamaica");
	StationTypeListboxResult.AddString("United Republic of Tanzania");
	StationTypeListboxResult.AddString("Malaysia");
	StationTypeListboxResult.AddString("China");
	StationTypeListboxResult.AddString("Indonesia");
	StationTypeListboxResult.AddString("Sri Lanka");
	StationTypeListboxResult.AddString("Saudi Arabia");
	StationTypeListboxResult.AddString("Ecuador");
 	StationTypeListboxResult.AddString("Lithuania");
  	StationTypeListboxResult.AddString("Latvia");
  	StationTypeListboxResult.AddString("Azerbaijan");
   StationTypeListboxResult.AddString("Serbia and Montenegro");
}


TStationTypeDialog::TStationTypeDialog(TWindow* parent, TResId resId, TStationTypeTransfer& stationtypetransfer,
													 string station_type, string recruiting_country,
													 string system_time, string delimiter, string turbowin_dir, string backup_folder)
				 : TDialog(parent, resId)
{
	local_station_type       = station_type;
	local_recruiting_country = recruiting_country;
   local_system_time        = system_time;
   local_delimiter          = delimiter;
   local_backup_folder      = backup_folder;
   local_turbowin_dir       = turbowin_dir;

   stationpasswordedit = new TEdit(this, IDC_STATIONPASSWORDEDIT, sizeof(stationtypetransfer.StationPasswordEditResult));
   button_logon        = new TButton(this, IDLOGONSTATION);
	radiobutton1        = new TRadioButton(this, IDC_STATIONTYPERADIOBUTTON1);
	radiobutton2        = new TRadioButton(this, IDC_STATIONTYPERADIOBUTTON2);
	checkboxUTC         = new TCheckBox(this, IDC_STATIONTYPECHECKBOX2);
   edit_delimiter      = new TEdit(this, IDC_STATIONTYPEEDIT, sizeof(stationtypetransfer.StationTypeDelimiterResult));
	listboxrecruiting   = new TListBox(this, IDC_STATIONTYPELISTBOX);
   edit_backup         = new TEdit(this, IDC_STATIONTYPEBACKUP, sizeof(stationtypetransfer.StationTypeBackupFolderResult));
   ok_button           = new TButton(this, IDOK);      // expliciet gemaakt om te kunnen disablen
   backup_button       = new TButton(this, IDBACKUP);  // expliciet gemaakt om te kunnen disablen

   // ingegraveerde TurboWin
   text_turbowin = new TStatic(this, IDC_TEXT_TURBOWIN);

	SetTransferBuffer (&stationtypetransfer);

   /* laden HTMLHelp OCX voor HTML (chm) help */
   HTMLHelp = NULL;
   hHelpOCX = NULL;
   hHelpOCX = ::LoadLibrary("hhctrl.ocx");
   if (hHelpOCX == NULL)
      MessageBox("Cannot use the html help", "TurboWin Error", MB_OK | MB_ICONERROR);

	//BackupFileData.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_EXPLORER;
	BackupFileData.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_EXPLORER;
   BackupFileData.SetFilter("Text files (*.txt)|*.txt");
	*BackupFileData.FileName = 0;
      
}


TStationTypeDialog::~TStationTypeDialog()
{
	delete radiobutton1;
	delete radiobutton2;
	delete checkboxUTC;
   delete edit_delimiter;
	delete listboxrecruiting;
   delete edit_backup;
   delete text_turbowin;
   delete stationpasswordedit;
   delete button_logon;
   delete ok_button;
   delete backup_button;

   /* sluiten chm help file */
   if (HTMLHelp != NULL)
      HTMLHelp(NULL, NULL, HH_CLOSE_ALL, NULL);
}


void TStationTypeDialog::SetupWindow()
{
	TDialog::SetupWindow();

   text_turbowin -> EnableWindow(false);

   // type of observation station (initialiseren om te voorkomen dat er 2 buttoms gelijk aan staan)
   radiobutton1 -> Uncheck();
   radiobutton2 -> Uncheck();

	if (local_station_type.compare("ship") == 0)
		radiobutton1 -> Check();
	else if (local_station_type.compare("fixed") == 0)
		radiobutton2 -> Check();

	// recruiting country
   //listboxrecruiting -> Clear();
	listboxrecruiting -> SetSelString(local_recruiting_country.c_str(), -1);

	// observation time according utc ?
	if (local_system_time.compare("UTC") == 0)
		checkboxUTC -> Check();
	else
		checkboxUTC -> Uncheck();

   // observation delimiter
	if (local_delimiter.compare("\0") != 0)
	{
		edit_delimiter -> Clear();
		edit_delimiter -> Insert(local_delimiter.c_str());
	}
   //else // blank
   //   edit_delimiter -> Insert("=");

   // backup folder
	if (local_backup_folder.compare("\0") != 0)
	{
		edit_backup -> Clear();
		edit_backup -> Insert(local_backup_folder.c_str());
	}
   else  // blank
   {
      // dan default geven
      local_backup_folder = local_turbowin_dir + "\\BACKUP\\IMMT_BACKUP_RUNNING.TXT";
		edit_backup -> Clear();
		edit_backup -> Insert(local_backup_folder.c_str());
   } // else (blank)

   // logon
   // om te voorkomen als een keer password is ingevuld en hierna weer openen van dit invul scherm het password er gewoon nog staat
   stationpasswordedit  -> Clear();

   // bij password intikken alleen *** laten zien (NB moet in SetupWindow, in de constructor zetten geeft geen effect)
   stationpasswordedit -> SetPasswordChar(42);                  // 42 dec ascii code = *

   // disablen van alle invoermogelijkheden
   radiobutton1      -> EnableWindow(false);
   radiobutton2      -> EnableWindow(false);
	checkboxUTC       -> EnableWindow(false);
   edit_delimiter    -> EnableWindow(false);
	listboxrecruiting -> EnableWindow(false);
   backup_button     -> EnableWindow(false);
   ok_button         -> EnableWindow(false);
}


void TStationTypeDialog::CmBackup()
{
	if (TFileSaveDialog(this, BackupFileData).Execute() == IDOK)
	{
      local_backup_folder = "";
      local_backup_folder = (string)BackupFileData.FileName;

		edit_backup -> Clear();
		edit_backup -> Insert(local_backup_folder.c_str());

   } // if (TFileOpenDialog(this, FileData).Execute() == IDOK)
}


void TStationTypeDialog::CmHelp()
{
   string volledig_help_path = "";
   volledig_help_path = local_turbowin_dir + station_help_file;
   HTMLHelp = (HtmlHelpFunc)::GetProcAddress(hHelpOCX, ATOM_HTMLHELP_API_ANSI);
   HTMLHelp(0, volledig_help_path.c_str(), HH_DISPLAY_TOPIC, 0);

}



void TStationTypeDialog::CmLogonStation()
{
   char buffer_station_password[MAX_STATION_PASSWORD_LENGTH];

   // password samenstellen (het zelfde als gebruikt bij en/decryptie van de vos-id file (zie function: Encrypt_Decrypt_VOSID_file() in file tmycall.cpp)
   string key = "_";
   string key2 = "u";
   string key3 = "-1";
   key += "Turbo";
   key2 += "Win33";
   key3 += "PMA";
   key  += "45";
   key2 += "y_";
   key3 += "PMO";

   // uitlezen password edit field
   stationpasswordedit   -> GetLine(buffer_station_password, MAX_STATION_PASSWORD_LENGTH, 0);

   if (strcmp(buffer_station_password, key.c_str()) != 0)
   {
      MessageBox("Password not correct\n Password forgotten? Contact your PMO", "TurboWin error", MB_OK | MB_ICONWARNING);
   }
   else
   {
      MessageBox("Password correct, you can insert/edit station data", "TurboWin information", MB_OK | MB_ICONINFORMATION);

      // enablen van alle invoermogelijkheden
      radiobutton1      -> EnableWindow(true);
      radiobutton2      -> EnableWindow(true);
	   checkboxUTC       -> EnableWindow(true);
      edit_delimiter    -> EnableWindow(true);
	   listboxrecruiting -> EnableWindow(true);
      backup_button     -> EnableWindow(true);
      ok_button         -> EnableWindow(true);

   } // else
}



/***********************************************************************************/
//                                                                                 //
//                                                                                 //
//                             platform (fixed sea station)                        //
//                                                                                 //
//                                                                                 //
/***********************************************************************************/

DEFINE_RESPONSE_TABLE1(TFixedDialog, TDialog)
	EV_COMMAND(IDHELP, CmHelp),
   EV_COMMAND(IDPRINT, CmPrint),
END_RESPONSE_TABLE;


TFixedTransfer::TFixedTransfer()
{
   FixedNameResult[0]        = '\0';
   FixedIMOResult[0]         = '\0';
	HeightAnemometerResult[0] = '\0';

	FixedRadioButtonResultYesAnemometer = BF_UNCHECKED;
	FixedRadioButtonResultNoAnemometer  = BF_UNCHECKED;
	FixedRadioButtonResultNorth         = BF_UNCHECKED;
	FixedRadioButtonResultSouth         = BF_UNCHECKED;
	FixedRadioButtonResultEast          = BF_UNCHECKED;
	FixedRadioButtonResultWest          = BF_UNCHECKED;

	// waves determining method
	DeterminingWavesResult.AddString(waves_estimated);
	//DeterminingWavesResult.AddString(waves_measured);
	DeterminingWavesResult.AddString(waves_measured_shipborne);
	DeterminingWavesResult.AddString(waves_measured_buoy);
	DeterminingWavesResult.AddString(waves_measured_other);


	// breedte hele graden
	LatitudeDegreesResult.AddString("\0");
	LatitudeDegreesResult.AddString("0");
	LatitudeDegreesResult.AddString("1");
	LatitudeDegreesResult.AddString("2");
	LatitudeDegreesResult.AddString("3");
	LatitudeDegreesResult.AddString("4");
	LatitudeDegreesResult.AddString("5");
	LatitudeDegreesResult.AddString("6");
	LatitudeDegreesResult.AddString("7");
	LatitudeDegreesResult.AddString("8");
	LatitudeDegreesResult.AddString("9");
	LatitudeDegreesResult.AddString("10");
	LatitudeDegreesResult.AddString("11");
	LatitudeDegreesResult.AddString("12");
	LatitudeDegreesResult.AddString("13");
	LatitudeDegreesResult.AddString("14");
	LatitudeDegreesResult.AddString("15");
	LatitudeDegreesResult.AddString("16");
	LatitudeDegreesResult.AddString("17");
	LatitudeDegreesResult.AddString("18");
	LatitudeDegreesResult.AddString("19");
	LatitudeDegreesResult.AddString("20");
	LatitudeDegreesResult.AddString("21");
	LatitudeDegreesResult.AddString("22");
	LatitudeDegreesResult.AddString("23");
	LatitudeDegreesResult.AddString("24");
	LatitudeDegreesResult.AddString("25");
	LatitudeDegreesResult.AddString("26");
	LatitudeDegreesResult.AddString("27");
	LatitudeDegreesResult.AddString("28");
	LatitudeDegreesResult.AddString("29");
	LatitudeDegreesResult.AddString("30");
	LatitudeDegreesResult.AddString("31");
	LatitudeDegreesResult.AddString("32");
	LatitudeDegreesResult.AddString("33");
	LatitudeDegreesResult.AddString("34");
	LatitudeDegreesResult.AddString("35");
	LatitudeDegreesResult.AddString("36");
	LatitudeDegreesResult.AddString("37");
	LatitudeDegreesResult.AddString("38");
	LatitudeDegreesResult.AddString("39");
	LatitudeDegreesResult.AddString("40");
	LatitudeDegreesResult.AddString("41");
	LatitudeDegreesResult.AddString("42");
	LatitudeDegreesResult.AddString("43");
	LatitudeDegreesResult.AddString("44");
	LatitudeDegreesResult.AddString("45");
	LatitudeDegreesResult.AddString("46");
	LatitudeDegreesResult.AddString("47");
	LatitudeDegreesResult.AddString("48");
	LatitudeDegreesResult.AddString("49");
	LatitudeDegreesResult.AddString("50");
	LatitudeDegreesResult.AddString("51");
	LatitudeDegreesResult.AddString("52");
	LatitudeDegreesResult.AddString("53");
	LatitudeDegreesResult.AddString("54");
	LatitudeDegreesResult.AddString("55");
	LatitudeDegreesResult.AddString("56");
	LatitudeDegreesResult.AddString("57");
	LatitudeDegreesResult.AddString("58");
	LatitudeDegreesResult.AddString("59");
	LatitudeDegreesResult.AddString("60");
	LatitudeDegreesResult.AddString("61");
	LatitudeDegreesResult.AddString("62");
	LatitudeDegreesResult.AddString("63");
	LatitudeDegreesResult.AddString("64");
	LatitudeDegreesResult.AddString("65");
	LatitudeDegreesResult.AddString("66");
	LatitudeDegreesResult.AddString("67");
	LatitudeDegreesResult.AddString("68");
	LatitudeDegreesResult.AddString("69");
	LatitudeDegreesResult.AddString("70");
	LatitudeDegreesResult.AddString("71");
	LatitudeDegreesResult.AddString("72");
	LatitudeDegreesResult.AddString("73");
	LatitudeDegreesResult.AddString("74");
	LatitudeDegreesResult.AddString("75");
	LatitudeDegreesResult.AddString("76");
	LatitudeDegreesResult.AddString("77");
	LatitudeDegreesResult.AddString("78");
	LatitudeDegreesResult.AddString("79");
	LatitudeDegreesResult.AddString("80");
	LatitudeDegreesResult.AddString("81");
	LatitudeDegreesResult.AddString("82");
	LatitudeDegreesResult.AddString("83");
	LatitudeDegreesResult.AddString("84");
	LatitudeDegreesResult.AddString("85");

	// breedte tienden  (= eigenlijk minuten)
	LatitudeTenthsResult.AddString("\0");
	LatitudeTenthsResult.AddString("00");
	LatitudeTenthsResult.AddString("01");
	LatitudeTenthsResult.AddString("02");
	LatitudeTenthsResult.AddString("03");
	LatitudeTenthsResult.AddString("04");
	LatitudeTenthsResult.AddString("05");
	LatitudeTenthsResult.AddString("06");
	LatitudeTenthsResult.AddString("07");
	LatitudeTenthsResult.AddString("08");
	LatitudeTenthsResult.AddString("09");
	LatitudeTenthsResult.AddString("10");
	LatitudeTenthsResult.AddString("11");
	LatitudeTenthsResult.AddString("12");
	LatitudeTenthsResult.AddString("13");
	LatitudeTenthsResult.AddString("14");
	LatitudeTenthsResult.AddString("15");
	LatitudeTenthsResult.AddString("16");
	LatitudeTenthsResult.AddString("17");
	LatitudeTenthsResult.AddString("18");
	LatitudeTenthsResult.AddString("19");
	LatitudeTenthsResult.AddString("20");
	LatitudeTenthsResult.AddString("21");
	LatitudeTenthsResult.AddString("22");
	LatitudeTenthsResult.AddString("23");
	LatitudeTenthsResult.AddString("24");
	LatitudeTenthsResult.AddString("25");
	LatitudeTenthsResult.AddString("26");
	LatitudeTenthsResult.AddString("27");
	LatitudeTenthsResult.AddString("28");
	LatitudeTenthsResult.AddString("29");
	LatitudeTenthsResult.AddString("30");
	LatitudeTenthsResult.AddString("31");
	LatitudeTenthsResult.AddString("32");
	LatitudeTenthsResult.AddString("33");
	LatitudeTenthsResult.AddString("34");
	LatitudeTenthsResult.AddString("35");
	LatitudeTenthsResult.AddString("36");
	LatitudeTenthsResult.AddString("37");
	LatitudeTenthsResult.AddString("38");
	LatitudeTenthsResult.AddString("39");
	LatitudeTenthsResult.AddString("40");
	LatitudeTenthsResult.AddString("41");
	LatitudeTenthsResult.AddString("42");
	LatitudeTenthsResult.AddString("43");
	LatitudeTenthsResult.AddString("44");
	LatitudeTenthsResult.AddString("45");
	LatitudeTenthsResult.AddString("46");
	LatitudeTenthsResult.AddString("47");
	LatitudeTenthsResult.AddString("48");
	LatitudeTenthsResult.AddString("49");
	LatitudeTenthsResult.AddString("50");
	LatitudeTenthsResult.AddString("51");
	LatitudeTenthsResult.AddString("52");
	LatitudeTenthsResult.AddString("53");
	LatitudeTenthsResult.AddString("54");
	LatitudeTenthsResult.AddString("55");
	LatitudeTenthsResult.AddString("56");
	LatitudeTenthsResult.AddString("57");
	LatitudeTenthsResult.AddString("58");
	LatitudeTenthsResult.AddString("59");

	// lengte hele graden
	LongitudeDegreesResult.AddString("\0");
	LongitudeDegreesResult.AddString("0");
	LongitudeDegreesResult.AddString("1");
	LongitudeDegreesResult.AddString("2");
	LongitudeDegreesResult.AddString("3");
	LongitudeDegreesResult.AddString("4");
	LongitudeDegreesResult.AddString("5");
	LongitudeDegreesResult.AddString("6");
	LongitudeDegreesResult.AddString("7");
	LongitudeDegreesResult.AddString("8");
	LongitudeDegreesResult.AddString("9");
	LongitudeDegreesResult.AddString("10");
	LongitudeDegreesResult.AddString("11");
	LongitudeDegreesResult.AddString("12");
	LongitudeDegreesResult.AddString("13");
	LongitudeDegreesResult.AddString("14");
	LongitudeDegreesResult.AddString("15");
	LongitudeDegreesResult.AddString("16");
	LongitudeDegreesResult.AddString("17");
	LongitudeDegreesResult.AddString("18");
	LongitudeDegreesResult.AddString("19");
	LongitudeDegreesResult.AddString("20");
	LongitudeDegreesResult.AddString("21");
	LongitudeDegreesResult.AddString("22");
	LongitudeDegreesResult.AddString("23");
	LongitudeDegreesResult.AddString("24");
	LongitudeDegreesResult.AddString("25");
	LongitudeDegreesResult.AddString("26");
	LongitudeDegreesResult.AddString("27");
	LongitudeDegreesResult.AddString("28");
	LongitudeDegreesResult.AddString("29");
	LongitudeDegreesResult.AddString("30");
	LongitudeDegreesResult.AddString("31");
	LongitudeDegreesResult.AddString("32");
	LongitudeDegreesResult.AddString("33");
	LongitudeDegreesResult.AddString("34");
	LongitudeDegreesResult.AddString("35");
	LongitudeDegreesResult.AddString("36");
	LongitudeDegreesResult.AddString("37");
	LongitudeDegreesResult.AddString("38");
	LongitudeDegreesResult.AddString("39");
	LongitudeDegreesResult.AddString("40");
	LongitudeDegreesResult.AddString("41");
	LongitudeDegreesResult.AddString("42");
	LongitudeDegreesResult.AddString("43");
	LongitudeDegreesResult.AddString("44");
	LongitudeDegreesResult.AddString("45");
	LongitudeDegreesResult.AddString("46");
	LongitudeDegreesResult.AddString("47");
	LongitudeDegreesResult.AddString("48");
	LongitudeDegreesResult.AddString("49");
	LongitudeDegreesResult.AddString("50");
	LongitudeDegreesResult.AddString("51");
	LongitudeDegreesResult.AddString("52");
	LongitudeDegreesResult.AddString("53");
	LongitudeDegreesResult.AddString("54");
	LongitudeDegreesResult.AddString("55");
	LongitudeDegreesResult.AddString("56");
	LongitudeDegreesResult.AddString("57");
	LongitudeDegreesResult.AddString("58");
	LongitudeDegreesResult.AddString("59");
	LongitudeDegreesResult.AddString("60");
	LongitudeDegreesResult.AddString("61");
	LongitudeDegreesResult.AddString("62");
	LongitudeDegreesResult.AddString("63");
	LongitudeDegreesResult.AddString("64");
	LongitudeDegreesResult.AddString("65");
	LongitudeDegreesResult.AddString("66");
	LongitudeDegreesResult.AddString("67");
	LongitudeDegreesResult.AddString("68");
	LongitudeDegreesResult.AddString("69");
	LongitudeDegreesResult.AddString("70");
	LongitudeDegreesResult.AddString("71");
	LongitudeDegreesResult.AddString("72");
	LongitudeDegreesResult.AddString("73");
	LongitudeDegreesResult.AddString("74");
	LongitudeDegreesResult.AddString("75");
	LongitudeDegreesResult.AddString("76");
	LongitudeDegreesResult.AddString("77");
	LongitudeDegreesResult.AddString("78");
	LongitudeDegreesResult.AddString("79");
	LongitudeDegreesResult.AddString("80");
	LongitudeDegreesResult.AddString("81");
	LongitudeDegreesResult.AddString("82");
	LongitudeDegreesResult.AddString("83");
	LongitudeDegreesResult.AddString("84");
	LongitudeDegreesResult.AddString("85");
	LongitudeDegreesResult.AddString("86");
	LongitudeDegreesResult.AddString("87");
	LongitudeDegreesResult.AddString("88");
	LongitudeDegreesResult.AddString("89");
	LongitudeDegreesResult.AddString("90");
	LongitudeDegreesResult.AddString("91");
	LongitudeDegreesResult.AddString("92");
	LongitudeDegreesResult.AddString("93");
	LongitudeDegreesResult.AddString("94");
	LongitudeDegreesResult.AddString("95");
	LongitudeDegreesResult.AddString("96");
	LongitudeDegreesResult.AddString("97");
	LongitudeDegreesResult.AddString("98");
	LongitudeDegreesResult.AddString("99");
	LongitudeDegreesResult.AddString("100");
	LongitudeDegreesResult.AddString("101");
	LongitudeDegreesResult.AddString("102");
	LongitudeDegreesResult.AddString("103");
	LongitudeDegreesResult.AddString("104");
	LongitudeDegreesResult.AddString("105");
	LongitudeDegreesResult.AddString("106");
	LongitudeDegreesResult.AddString("107");
	LongitudeDegreesResult.AddString("108");
	LongitudeDegreesResult.AddString("109");
	LongitudeDegreesResult.AddString("110");
	LongitudeDegreesResult.AddString("111");
	LongitudeDegreesResult.AddString("112");
	LongitudeDegreesResult.AddString("113");
	LongitudeDegreesResult.AddString("114");
	LongitudeDegreesResult.AddString("115");
	LongitudeDegreesResult.AddString("116");
	LongitudeDegreesResult.AddString("117");
	LongitudeDegreesResult.AddString("118");
	LongitudeDegreesResult.AddString("119");
	LongitudeDegreesResult.AddString("120");
	LongitudeDegreesResult.AddString("121");
	LongitudeDegreesResult.AddString("122");
	LongitudeDegreesResult.AddString("123");
	LongitudeDegreesResult.AddString("124");
	LongitudeDegreesResult.AddString("125");
	LongitudeDegreesResult.AddString("126");
	LongitudeDegreesResult.AddString("127");
	LongitudeDegreesResult.AddString("128");
	LongitudeDegreesResult.AddString("129");
	LongitudeDegreesResult.AddString("130");
	LongitudeDegreesResult.AddString("131");
	LongitudeDegreesResult.AddString("132");
	LongitudeDegreesResult.AddString("133");
	LongitudeDegreesResult.AddString("134");
	LongitudeDegreesResult.AddString("135");
	LongitudeDegreesResult.AddString("136");
	LongitudeDegreesResult.AddString("137");
	LongitudeDegreesResult.AddString("138");
	LongitudeDegreesResult.AddString("139");
	LongitudeDegreesResult.AddString("140");
	LongitudeDegreesResult.AddString("141");
	LongitudeDegreesResult.AddString("142");
	LongitudeDegreesResult.AddString("143");
	LongitudeDegreesResult.AddString("144");
	LongitudeDegreesResult.AddString("145");
	LongitudeDegreesResult.AddString("146");
	LongitudeDegreesResult.AddString("147");
	LongitudeDegreesResult.AddString("148");
	LongitudeDegreesResult.AddString("149");
	LongitudeDegreesResult.AddString("150");
	LongitudeDegreesResult.AddString("151");
	LongitudeDegreesResult.AddString("152");
	LongitudeDegreesResult.AddString("153");
	LongitudeDegreesResult.AddString("154");
	LongitudeDegreesResult.AddString("155");
	LongitudeDegreesResult.AddString("156");
	LongitudeDegreesResult.AddString("157");
	LongitudeDegreesResult.AddString("158");
	LongitudeDegreesResult.AddString("159");
	LongitudeDegreesResult.AddString("160");
	LongitudeDegreesResult.AddString("161");
	LongitudeDegreesResult.AddString("162");
	LongitudeDegreesResult.AddString("163");
	LongitudeDegreesResult.AddString("164");
	LongitudeDegreesResult.AddString("165");
	LongitudeDegreesResult.AddString("166");
	LongitudeDegreesResult.AddString("167");
	LongitudeDegreesResult.AddString("168");
	LongitudeDegreesResult.AddString("169");
	LongitudeDegreesResult.AddString("170");
	LongitudeDegreesResult.AddString("171");
	LongitudeDegreesResult.AddString("172");
	LongitudeDegreesResult.AddString("173");
	LongitudeDegreesResult.AddString("174");
	LongitudeDegreesResult.AddString("175");
	LongitudeDegreesResult.AddString("176");
	LongitudeDegreesResult.AddString("177");
	LongitudeDegreesResult.AddString("178");
	LongitudeDegreesResult.AddString("179");
	LongitudeDegreesResult.AddString("180");

	// lengte tienden (= minuten)
	LongitudeTenthsResult.AddString("\0");
	LongitudeTenthsResult.AddString("00");
	LongitudeTenthsResult.AddString("01");
	LongitudeTenthsResult.AddString("02");
	LongitudeTenthsResult.AddString("03");
	LongitudeTenthsResult.AddString("04");
	LongitudeTenthsResult.AddString("05");
	LongitudeTenthsResult.AddString("06");
	LongitudeTenthsResult.AddString("07");
	LongitudeTenthsResult.AddString("08");
	LongitudeTenthsResult.AddString("09");
	LongitudeTenthsResult.AddString("10");
	LongitudeTenthsResult.AddString("11");
	LongitudeTenthsResult.AddString("12");
	LongitudeTenthsResult.AddString("13");
	LongitudeTenthsResult.AddString("14");
	LongitudeTenthsResult.AddString("15");
	LongitudeTenthsResult.AddString("16");
	LongitudeTenthsResult.AddString("17");
	LongitudeTenthsResult.AddString("18");
	LongitudeTenthsResult.AddString("19");
	LongitudeTenthsResult.AddString("20");
	LongitudeTenthsResult.AddString("21");
	LongitudeTenthsResult.AddString("22");
	LongitudeTenthsResult.AddString("23");
	LongitudeTenthsResult.AddString("24");
	LongitudeTenthsResult.AddString("25");
	LongitudeTenthsResult.AddString("26");
	LongitudeTenthsResult.AddString("27");
	LongitudeTenthsResult.AddString("28");
	LongitudeTenthsResult.AddString("29");
	LongitudeTenthsResult.AddString("30");
	LongitudeTenthsResult.AddString("31");
	LongitudeTenthsResult.AddString("32");
	LongitudeTenthsResult.AddString("33");
	LongitudeTenthsResult.AddString("34");
	LongitudeTenthsResult.AddString("35");
	LongitudeTenthsResult.AddString("36");
	LongitudeTenthsResult.AddString("37");
	LongitudeTenthsResult.AddString("38");
	LongitudeTenthsResult.AddString("39");
	LongitudeTenthsResult.AddString("40");
	LongitudeTenthsResult.AddString("41");
	LongitudeTenthsResult.AddString("42");
	LongitudeTenthsResult.AddString("43");
	LongitudeTenthsResult.AddString("44");
	LongitudeTenthsResult.AddString("45");
	LongitudeTenthsResult.AddString("46");
	LongitudeTenthsResult.AddString("47");
	LongitudeTenthsResult.AddString("48");
	LongitudeTenthsResult.AddString("49");
	LongitudeTenthsResult.AddString("50");
	LongitudeTenthsResult.AddString("51");
	LongitudeTenthsResult.AddString("52");
	LongitudeTenthsResult.AddString("53");
	LongitudeTenthsResult.AddString("54");
	LongitudeTenthsResult.AddString("55");
	LongitudeTenthsResult.AddString("56");
	LongitudeTenthsResult.AddString("57");
	LongitudeTenthsResult.AddString("58");
	LongitudeTenthsResult.AddString("59");
}


TFixedDialog::TFixedDialog(TWindow* parent, TResId resId, TFixedTransfer& fixedtransfer,
									string hoogte_anemometer,
									string anemometer_10_yes_no,
									string breedte_graden, string breedte_tienden,
									string halfrond_N_Z, string lengte_graden,
									string lengte_tienden, string halfrond_W_E,
									string golven_resultaat, string turbowin_dir,
                           string fixed_name, string fixed_imo_nummer)
				 : TDialog(parent, resId)
{
   /* initialisatie */
   print_button_pressed = false;

   local_fixed_name           = fixed_name;
	local_fixed_imo_nummer     = fixed_imo_nummer;
	local_hoogte_anemometer    = hoogte_anemometer;
	local_anemometer_10_yes_no = anemometer_10_yes_no;
	local_breedte_graden       = breedte_graden;
	local_breedte_tienden      = breedte_tienden;
	local_halfrond_N_Z         = halfrond_N_Z;
   local_lengte_graden        = lengte_graden;
	local_lengte_tienden       = lengte_tienden;
	local_halfrond_W_E         = halfrond_W_E;
	local_golven_resultaat     = golven_resultaat;
   local_turbowin_dir         = turbowin_dir;


   fixedname                  = new TEdit(this,IDC_PLATFORMNAMEEDIT, sizeof(fixedtransfer.FixedNameResult));
   fixedimonummer             = new TEdit(this,IDC_PLATFORMIMOEDIT, sizeof(fixedtransfer.FixedIMOResult));
	heightanemometer           = new TEdit(this,IDC_PLATFORMEDIT1, sizeof(fixedtransfer.HeightAnemometerResult));
	radiobutton_yes_anemometer = new TRadioButton(this, IDC_PLATFORMRADIOBUTTON1);   // yes van anemometer
	radiobutton_no_anemometer  = new TRadioButton(this, IDC_PLATFORMRADIOBUTTON2);   // no van anemomter
	latitudedegrees            = new TComboBox(this, IDC_PLATFORMCOMBOBOX2);
	latitudetenths             = new TComboBox(this, IDC_PLATFORMCOMBOBOX3);
	radiobutton_North          = new TRadioButton(this, IDC_PLATFORMRADIOBUTTON7);   // North
	radiobutton_South          = new TRadioButton(this, IDC_PLATFORMRADIOBUTTON8);   // South
	longitudedegrees           = new TComboBox(this, IDC_PLATFORMCOMBOBOX4);
	longitudetenths            = new TComboBox(this, IDC_PLATFORMCOMBOBOX5);
	radiobutton_East           = new TRadioButton(this, IDC_PLATFORMRADIOBUTTON9);   // East
	radiobutton_West           = new TRadioButton(this, IDC_PLATFORMRADIOBUTTON10);  // West
	determiningwaves           = new TListBox(this, IDC_PLATFORMLISTBOX1);

   // ingegraveerde TurboWin
   text_turbowin = new TStatic(this, IDC_TEXT_TURBOWIN);

   // validator hoogte anemomter (alleen hele meters)
   // 0123456789
   heightanemometer  -> SetValidator(new TFilterValidator("0-9"));                  // bld 390 - 392 Swan

	SetTransferBuffer (&fixedtransfer);

   /* laden HTMLHelp OCX voor HTML (chm) help */
   HTMLHelp = NULL;
   hHelpOCX = NULL;
   hHelpOCX = ::LoadLibrary("hhctrl.ocx");
   if (hHelpOCX == NULL)
      MessageBox("Cannot use the html help", "TurboWin Error", MB_OK | MB_ICONERROR);

}


TFixedDialog::~TFixedDialog()
{
   delete fixedname;
   delete fixedimonummer;
	delete heightanemometer;
	delete radiobutton_yes_anemometer;
	delete radiobutton_no_anemometer;
	delete latitudedegrees;
	delete latitudetenths;
	delete radiobutton_North;
	delete radiobutton_South;
	delete longitudedegrees;
	delete longitudetenths;
	delete radiobutton_East;
	delete radiobutton_West;
	delete determiningwaves;
   delete text_turbowin;

   /* sluiten chm help file */
   if (HTMLHelp != NULL)
      HTMLHelp(NULL, NULL, HH_CLOSE_ALL, NULL);
}


void TFixedDialog::SetupWindow()
{
	TDialog::SetupWindow();

   text_turbowin -> EnableWindow(false);

   //
   // initialiseren
   //

   // rig/platform name
   fixedname -> Clear();

   // imo number
   fixedimonummer -> Clear();

   // anemometer height
   heightanemometer -> Clear();

   // breedte graden
   latitudedegrees -> Clear();

   // breedte tienden
   latitudetenths -> Clear();

   // lengte graden
   longitudedegrees -> Clear();

   // lengte tienden
   longitudetenths -> Clear();



   //
   // velden vullen
   //

   // rig/platform snaam
	if (local_fixed_name != "")
		fixedname -> Insert(local_fixed_name.c_str());

   // IMO number
	if (local_fixed_imo_nummer != "")
		fixedimonummer -> Insert(local_fixed_imo_nummer.c_str());

	// hoogte anemometer
	if (local_hoogte_anemometer != "")
		heightanemometer -> Insert(local_hoogte_anemometer.c_str());

	// anemometer 10 m
	if (local_anemometer_10_yes_no ==  "yes")
		radiobutton_yes_anemometer -> Check();
	else if (local_anemometer_10_yes_no == "no")
		radiobutton_no_anemometer -> Check();

	// breedte graden
	if (local_breedte_graden != "")
		latitudedegrees -> SetText(local_breedte_graden.c_str());

	// breedte tienden
	if (local_breedte_tienden != "")
	{
		// i.v.m. typische eigenschap SetText(eerst in de listbox zoeken)
		// zonder onderstaande zou b.v. bij direct invullen in de edit van de
		// combobox een "1" een "10" worden
		//if (station_breedte_tienden.length() == 1)
		//	station_breedte_tienden.insert(0, "0");

		latitudetenths -> SetText(local_breedte_tienden.c_str());
	}

	// N of Zuid
	if (local_halfrond_N_Z ==  "N")
		radiobutton_North -> Check();
	else if (local_halfrond_N_Z == "S")
		radiobutton_South -> Check();

	// lengte graden
	if (local_lengte_graden != "")
		longitudedegrees -> SetText(local_lengte_graden.c_str());

	// lengte tienden
	if (local_lengte_tienden != "")
	{
		// i.v.m. typische eigenschap SetText(eerst in de listbox zoeken)
		// zonder onderstaande zou b.v. bij direct invullen in de edit van de
		// combobox een "1" een "10" worden
		//if (station_lengte_tienden.length() == 1)
		//	station_lengte_tienden.insert(0, "0");

		longitudetenths -> SetText(local_lengte_tienden.c_str());
	}

	// N of Zuid
	if (local_halfrond_W_E == "E")
		radiobutton_East -> Check();
	else if (local_halfrond_W_E == "W")
		radiobutton_West -> Check();

	// determiningwaves
	if (local_golven_resultaat.compare("\0") != 0)
		determiningwaves -> SetSelString(local_golven_resultaat.c_str(), -1);
}


void TFixedDialog::CmHelp()
{
   string volledig_help_path = "";
   volledig_help_path = local_turbowin_dir + station_fixed_help_file;
   HTMLHelp = (HtmlHelpFunc)::GetProcAddress(hHelpOCX, ATOM_HTMLHELP_API_ANSI);
   HTMLHelp(0, volledig_help_path.c_str(), HH_DISPLAY_TOPIC, 0);
}


void TFixedDialog::CmPrint()
{
	print_button_pressed = true;
   TFixedDialog::CmOk();                                // continue the OK process
}



/***********************************************************************************/
//                                                                                 //
//                                                                                 //
//                                   ship                                          //
//                                                                                 //
//                                                                                 //
/***********************************************************************************/
DEFINE_RESPONSE_TABLE1(TShipDialog, TDialog)
	EV_COMMAND(IDHELP, CmHelp),
   //EV_BN_CLICKED(IDC_GLYPH_VOSCLIM, Vosclim_Brochure),
   //EV_BN_CLICKED(IDC_VOSCLIMCHECKBOX, Ev_Checkbox_Vosclim_Indication_SelChange),
   EV_BN_CLICKED(IDC_SOOPCHECKBOX, Ev_Checkbox_SOOP_Indication_SelChange),
   EV_BN_CLICKED(IDC_ASAPCHECKBOX, Ev_Checkbox_ASAP_Indication_SelChange),
   EV_COMMAND(IDPRINT, CmPrint),
END_RESPONSE_TABLE;


TShipTransfer::TShipTransfer()
{
   ShipsNameResult[0]            = '\0';
   IMOResult[0]                  = '\0';
   BarometerSLLResult[0]         = '\0';
   KeelSLLResult[0]              = '\0';
	HeightAnemometerShipResult[0] = '\0';
	SeawaterMeasurementResult[0]  = '\0';
   NationalGroupResult1[0]       = '\0';
   NationalGroupResult2[0]       = '\0';
   NationalGroupResult3[0]       = '\0';
   NationalGroupResult4[0]       = '\0';
   NationalGroupResult5[0]       = '\0';
   OtherResult[0]                = '\0';
   //VosClimCheckBoxResult         = BF_UNCHECKED;
   ASAPCheckBoxResult            = BF_UNCHECKED;
   SOOPCheckBoxResult            = BF_UNCHECKED;
   OtherCheckBoxResult           = BF_UNCHECKED;
   messageformCLIresult          = BF_UNCHECKED;
   messageformSELresult          = BF_UNCHECKED;
   messageformSUSresult          = BF_UNCHECKED;               // supplementary with sst
   messageformSUPresult          = BF_UNCHECKED;               // supplementary without sst
   messageformAUXresult          = BF_UNCHECKED;
   codeformAlphanumericResult    = BF_UNCHECKED;
   codeformBUFRResult            = BF_UNCHECKED;
   codeformCompressedResult      = BF_UNCHECKED;

   // waves determining method
   DeterminingShipWavesResult.AddString("\0");
	DeterminingShipWavesResult.AddString(waves_estimated);
	DeterminingShipWavesResult.AddString(waves_measured_shipborne);
	DeterminingShipWavesResult.AddString(waves_measured_buoy);
	DeterminingShipWavesResult.AddString(waves_measured_other);

   //GTSHeaderCheckBoxResult       = BF_UNCHECKED;
}


TShipDialog::TShipDialog(TWindow* parent, TResId resId, TShipTransfer& shiptransfer,
								 string scheepsnaam, string hoogte_anemometer_ship, string diepte_zeewater,
                         string nationale_groep_1,
                         string nationale_groep_2,
                         string nationale_groep_3,
                         string nationale_groep_4,
                         string nationale_groep_5,
                         /*string vos_clim_deelnemer,*/ string asap_deelnemer, string soop_deelnemer, string other_deelnemer,
                         string message_mode, string turbowin_dir, string imo_number,
                         string code_mode, string ship_golven_resultaat, string recruiting_country,
                         /*string gts_header_resultaat,*/ string barometer_sll, string keel_sll, int start_option)
					  : TDialog(parent, resId)
{
   /* initialisatie */
   print_button_pressed = false;

	local_scheepsnaam                 = scheepsnaam;
	local_hoogte_anemometer_ship      = hoogte_anemometer_ship;
	local_diepte_zeewater             = diepte_zeewater;
   local_nationale_groep_1           = nationale_groep_1;
   local_nationale_groep_2           = nationale_groep_2;
   local_nationale_groep_3           = nationale_groep_3;
   local_nationale_groep_4           = nationale_groep_4;
   local_nationale_groep_5           = nationale_groep_5;
   //local_vos_clim_deelnemer          = vos_clim_deelnemer;
   local_asap_deelnemer              = asap_deelnemer;
   local_soop_deelnemer              = soop_deelnemer;
   local_other_deelnemer             = other_deelnemer;
   local_message_mode                = message_mode;            // SEL, SUS, SUP, AUX
   local_turbowin_dir                = turbowin_dir;
   local_imo_number                  = imo_number;
   local_barometer_sll               = barometer_sll;
   local_keel_sll                    = keel_sll;
   local_code_mode                   = code_mode;               // ALPHANUMERIC, BUFR, COMPRESSED
	local_ship_golven_resultaat       = ship_golven_resultaat;
   local_recruiting_country          = recruiting_country;
   //local_gts_header_resultaat        = gts_header_resultaat;
   local_start_option                = start_option;

   
   WebLinkCtrl            = new THLinkCtrl(this, IDC_VOSCLIM_WEB);
	shipsname              = new TEdit(this, IDC_SHIPEDIT1, sizeof(shiptransfer.ShipsNameResult));
   imoedit                = new TEdit(this, IDC_IMOEDIT, sizeof(shiptransfer.IMOResult));
   radiobutton_cli        = new TRadioButton(this, IDC_MESSAGEFORMCLIRADIOBUTTON);
   radiobutton_sel        = new TRadioButton(this, IDC_MESSAGEFORMSELRADIOBUTTON);
   radiobutton_sus        = new TRadioButton(this, IDC_MESSAGEFORMSUSRADIOBUTTON);
   radiobutton_sup        = new TRadioButton(this, IDC_MESSAGEFORMSUPRADIOBUTTON);
   radiobutton_aux        = new TRadioButton(this, IDC_MESSAGEFORMAUXRADIOBUTTON);
   //vos_clim_checkbox      = new TCheckBox(this, IDC_VOSCLIMCHECKBOX);
   asap_checkbox          = new TCheckBox(this, IDC_ASAPCHECKBOX);
   soop_checkbox          = new TCheckBox(this, IDC_SOOPCHECKBOX);
   other_checkbox         = new TCheckBox(this, IDC_OTHERCHECKBOX);
   edit_other             = new TEdit(this, IDC_OTHEREDIT, sizeof(shiptransfer.OtherResult));
   edit_national_1        = new TEdit(this, IDC_SHIPEDITNATIONAL1, sizeof(shiptransfer.NationalGroupResult1));
   edit_national_2        = new TEdit(this, IDC_SHIPEDITNATIONAL2, sizeof(shiptransfer.NationalGroupResult2));
   edit_national_3        = new TEdit(this, IDC_SHIPEDITNATIONAL3, sizeof(shiptransfer.NationalGroupResult3));
   edit_national_4        = new TEdit(this, IDC_SHIPEDITNATIONAL4, sizeof(shiptransfer.NationalGroupResult4));
   edit_national_5        = new TEdit(this, IDC_SHIPEDITNATIONAL5, sizeof(shiptransfer.NationalGroupResult5));
	heightanemometer       = new TEdit(this, IDC_SHIPEDIT4, sizeof(shiptransfer.HeightAnemometerShipResult));
	seawatermeasurement    = new TEdit(this, IDC_SHIPEDIT5, sizeof(shiptransfer.SeawaterMeasurementResult));
   radiobutton_alpha      = new TRadioButton(this, IDC_CODEFORMALPHANUMERICRADIOBUTTON);
   radiobutton_bufr       = new TRadioButton(this, IDC_CODEFORMBUFRRADIOBUTTON);
   radiobutton_compressed = new TRadioButton(this, IDC_CODEFORMCOMPRESSEDRADIOBUTTON);
	determiningshipwaves   = new TComboBox(this, IDC_SHIPWAVESCOMBOBOX);
   edit_barometer_sll     = new TEdit(this, IDC_SHIPEDITBAROMETERMSL, sizeof(shiptransfer.BarometerSLLResult));
   edit_keel_sll          = new TEdit(this, IDC_SHIPEDITKEELMSL, sizeof(shiptransfer.KeelSLLResult));
   //gts_header_checkbox    = new TCheckBox(this, IDC_GTSHEADERCHECKBOX);


   // ingegraveerde TurboWin
   text_turbowin = new TStatic(this, IDC_TEXT_TURBOWIN);

   // validator hoogte anemomter / diepte zeewater sensor (alleen hele meters)
   // 0123456789
   heightanemometer    -> SetValidator(new TFilterValidator("0-9"));                  // bld 390 - 392 Swan
   seawatermeasurement -> SetValidator(new TFilterValidator("0-9"));                  // bld 390 - 392 Swan
   edit_keel_sll       -> SetValidator(new TFilterValidator("0-9."));
   edit_barometer_sll  -> SetValidator(new TFilterValidator("0-9."));

	SetTransferBuffer (&shiptransfer);

   /* laden HTMLHelp OCX voor HTML (chm) help */
   HTMLHelp = NULL;
   hHelpOCX = NULL;
   hHelpOCX = ::LoadLibrary("hhctrl.ocx");
   if (hHelpOCX == NULL)
      MessageBox("Cannot use the html help", "TurboWin Error", MB_OK | MB_ICONERROR);
   
}


TShipDialog::~TShipDialog()
{
   delete WebLinkCtrl;
	delete shipsname;
   delete imoedit;
   delete radiobutton_cli;
   delete radiobutton_sel;
   delete radiobutton_sus;
   delete radiobutton_sup;
   delete radiobutton_aux;
   //delete vos_clim_checkbox;
   delete asap_checkbox;
   delete soop_checkbox;
   delete other_checkbox;
   delete edit_other;
   delete edit_national_1;
   delete edit_national_2;
   delete edit_national_3;
   delete edit_national_4;
   delete edit_national_5;
	delete heightanemometer;
	delete seawatermeasurement;
   delete radiobutton_alpha;
   delete radiobutton_bufr;
   delete radiobutton_compressed;
   delete text_turbowin;
   delete determiningshipwaves;
   //delete gts_header_checkbox;
   delete edit_barometer_sll;
   delete edit_keel_sll;

   /* sluiten chm help file */
   if (HTMLHelp != NULL)
      HTMLHelp(NULL, NULL, HH_CLOSE_ALL, NULL);
}


void TShipDialog::SetupWindow()
{
	TDialog::SetupWindow();

   /* web link naar VOSClim Fleet web site */
   WebLinkCtrl -> SetText(_T("(visit the web site)"));                                // via resource workshop komt dit niet goed over, daarom hier doen
   //WebLinkCtrl -> SetHyperLink(_T("http://www.bom.gov.au/jcomm/vos/vosclim.html"));
   WebLinkCtrl -> SetHyperLink(_T("http://sot.jcommops.org/vos/"));

   text_turbowin -> EnableWindow(false);


   //
   ////////// eerst alles initialiseren
   //

   // scheepsnaam
   shipsname -> Clear();

   // imo nummer
   imoedit -> Clear();

   // message form
   radiobutton_cli -> Uncheck();                         // initialisatie (anders kunnen na een error melding 2 radio buttons gechecked zijn)
   radiobutton_sel -> Uncheck();                         // initialisatie (anders kunnen na een error melding 2 radio buttons gechecked zijn)
   radiobutton_sus -> Uncheck();                         //        --"--
   radiobutton_sup -> Uncheck();                         //        --"--
   radiobutton_aux -> Uncheck();                         //        --"--

   // code form
   radiobutton_alpha      -> Uncheck();                   // initialisatie (anders kunnen na een error melding 2 radio buttons gechecked zijn)
   radiobutton_bufr       -> Uncheck();                   //        --"--
   radiobutton_compressed -> Uncheck();                   //        --"--

   // alleen bij start option 2 kan compressed obs'en voor niet Nederlands gerecruiteerde schepen gemaakt worden
   //if ( (local_start_option != 2) && (local_recruiting_country != "") && (local_recruiting_country != NETHERLANDS && local_recruiting_country != NETHERLANDS_WMO) )
   //{
   //   if (local_code_mode == COMPRESSED_OBS)              // voor de zekerheid resetten
   //   {
   //     local_code_mode = "";
   //   }
   //   radiobutton_compressed -> EnableWindow(false);      // disablen
   //} // if (start_option != 2) etc.

   // Nederlandse gerecruteerde schepen hebben default compressed form (behalve bij start option 4) en geen FM13 optie meer!
   if ( (local_recruiting_country == NETHERLANDS || local_recruiting_country == NETHERLANDS_WMO) && (local_start_option != 4) )
   {
      local_code_mode = COMPRESSED_OBS;                   // verderop in deze functie wordt de radiobutton zelf 'aan'gezet

      radiobutton_compressed -> EnableWindow(false);      // disablen
      radiobutton_alpha      -> EnableWindow(false);      // disablen
      radiobutton_bufr       -> EnableWindow(false);      // disablen

      // NB het checken van de compressed radio button gebeurd hier verderop in de code

   } // if ( (local_recruiting_country == "Netherlands") && (local_start_option != 4) )


   // UK gerecruteerde schepen hebben default compressed form (behalve bij start option 4) maar blijven de optie FM13 houden!
   //if ( (local_recruiting_country == UNITED_KINGDOM) && (local_start_option != 4) )
   //{
   //   local_code_mode = COMPRESSED_OBS;                   // verderop in deze functie wordt de radiobutton zelf 'aan'gezet
   //
   //   radiobutton_bufr       -> EnableWindow(false);      // disablen
   //   // NB het checken van de compressed radio button gebeurd hier verderop in de code
   //
   //} //


   // vos clim
   if (local_recruiting_country == NETHERLANDS || local_recruiting_country == NETHERLANDS_WMO || local_recruiting_country == UNITED_KINGDOM)
   {
      // maar dan ook gelijk VOSClim ship by default
      local_message_mode = "CLI";                                   // zie verderop indeze function
      
      radiobutton_cli -> EnableWindow(false);            // disablen
      radiobutton_sel -> EnableWindow(false);            // disablen
      radiobutton_sus -> EnableWindow(false);            // disablen
      radiobutton_sup -> EnableWindow(false);            // disablen
      radiobutton_aux -> EnableWindow(false);            // disablen
   }

   // asap
   asap_checkbox -> Uncheck();

   // soop
   soop_checkbox -> Uncheck();

   // other
   other_checkbox -> Uncheck();

   // other namelijk
   edit_other -> Clear();

   // nationale groepen
   edit_national_1 -> Clear();
   edit_national_2 -> Clear();
   edit_national_3 -> Clear();
   edit_national_4 -> Clear();
   edit_national_5 -> Clear();

   // hoogte anemometer
   heightanemometer -> Clear();

   // diepte sst sensor
   seawatermeasurement -> Clear();

   //determiningshipwaves LISTBOX behoeft niet op 0 o.i.d. gezet te worden

   // afstand barometer - sll
   edit_barometer_sll -> Clear();

   // afstan kiel = sll
   edit_keel_sll -> Clear();

   // gts header
   //gts_header_checkbox -> Uncheck();




   //
   //////////// velden vullen
   //

   // scheepsnaam
	if (local_scheepsnaam != "")
		shipsname -> Insert(local_scheepsnaam.c_str());

   // IMO number
	if (local_imo_number != "")
		imoedit -> Insert(local_imo_number.c_str());

   // message mode
   if (local_message_mode == "CLI")
      radiobutton_cli -> Check();
   else if (local_message_mode == "SEL")
      radiobutton_sel -> Check();
   else if (local_message_mode == "SUS")
      radiobutton_sus -> Check();
   else if (local_message_mode == "SUP")
      radiobutton_sup -> Check();
   else if (local_message_mode == "AUX")
      radiobutton_aux -> Check();
   else
      radiobutton_sel -> Check();                        // default is selected ship

   // code mode
   if (local_code_mode == ALPHANUMERIC_OBS)
      radiobutton_alpha -> Check();
   else if (local_code_mode == BUFR_OBS)
      radiobutton_bufr -> Check();
   else if (local_code_mode == COMPRESSED_OBS)
      radiobutton_compressed -> Check();
   else
      radiobutton_alpha -> Check();                       // default is alphanumeric

	// VOS-Clim deelnemer ?
	//if (local_vos_clim_deelnemer == "yes")
	//	vos_clim_checkbox -> Check();

	// ASAP deelnemer ?
	if (local_asap_deelnemer == "yes")
		asap_checkbox -> Check();

	// SOOP deelnemer ?
	if (local_soop_deelnemer == "yes")
		soop_checkbox -> Check();

	// Other deelnemer ?
	if ( (local_other_deelnemer != "") && (local_other_deelnemer != "no") )
	{
   	other_checkbox -> Check();

		edit_other -> Insert(local_other_deelnemer.c_str());
   }

   // national group 1
	if (local_nationale_groep_1 != "")
		edit_national_1 -> Insert(local_nationale_groep_1.c_str());

   // national group 2
	if (local_nationale_groep_2 != "")
		edit_national_2 -> Insert(local_nationale_groep_2.c_str());

   // national group 3
	if (local_nationale_groep_3 != "")
		edit_national_3 -> Insert(local_nationale_groep_3.c_str());

   // national group 4
	if (local_nationale_groep_4 != "")
		edit_national_4 -> Insert(local_nationale_groep_4.c_str());

   // national group 5
	if (local_nationale_groep_5 != "")
		edit_national_5 -> Insert(local_nationale_groep_5.c_str());

	// anemometer
	if (local_hoogte_anemometer_ship != "")
		heightanemometer -> Insert(local_hoogte_anemometer_ship.c_str());

	// zeewater
	if (local_diepte_zeewater != "")
		seawatermeasurement -> Insert(local_diepte_zeewater.c_str());

   // determiningwaves
	if (local_ship_golven_resultaat != "")
		determiningshipwaves -> SetSelString(local_ship_golven_resultaat.c_str(), -1); // If SearchIndex is -1, the entire list is searched, beginning with the first item.

   // barometer - SLL distance
	if (local_barometer_sll != "")
		edit_barometer_sll -> Insert(local_barometer_sll.c_str());

   // keel - SLL distance
	if (local_keel_sll != "")
		edit_keel_sll -> Insert(local_keel_sll.c_str());

   // gts header
   //if (local_recruiting_country == "Denmark")
   //{
   //   if (local_gts_header_resultaat == "yes")
   //      gts_header_checkbox -> Check();
   //}
   //else                                                   // dus ge-grayed bij recruiting country is not Denmark
   //{
   //   gts_header_checkbox -> EnableWindow(false);
   //} // else

}



void TShipDialog::CmHelp()
{
   string volledig_help_path = "";
   volledig_help_path = local_turbowin_dir + station_ship_help_file;
   HTMLHelp = (HtmlHelpFunc)::GetProcAddress(hHelpOCX, ATOM_HTMLHELP_API_ANSI);
   HTMLHelp(0, volledig_help_path.c_str(), HH_DISPLAY_TOPIC, 0);
}



void TShipDialog::CmPrint()
{
	print_button_pressed = true;
   TShipDialog::CmOk();                                // continue the OK process
}



//void TShipDialog::Ev_Checkbox_Vosclim_Indication_SelChange()
//{
//   string info = "";
//
//
//   if (vos_clim_checkbox -> GetCheck() == BF_CHECKED)
//   {
//      info = "You have ticked the VOSCLIM checkbox";
//      info += "\n\n";
//      info += "- If your ship was already recruited as a VOSCLIM ship, please leave this box checked";
//      info += "\n";
//      info += "- If you wish to participate in the VOSCLIM project, please send a message to your PMO with the ship's name and call sign and the date of the first VOSCLIM observation";
//   }
//   else // unchecked
//   {
//      info = "You have unchecked the VOSCLIM checkbox";
//      info += "\n\n";
//      info += "- If your ship was recruited as a VOSCLIM ship and you want to continue, please check this box again";
//      info += "\n";
//      info += "- If you do not wish to participate in the VOSCLIM project anymore, please send a message to your PMO with the ship's name and call sign and the date of the last VOSCLIM observation";
//   } // else (unchecked)
//
//   MessageBox(info.c_str(), "TurboWin message", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL); // blijft altijd de bovenste checkbox
//}



void TShipDialog::Ev_Checkbox_SOOP_Indication_SelChange()
{
   string info = "";


   if (soop_checkbox -> GetCheck() == BF_CHECKED)
   {
      info = "You have ticked the SOOP checkbox";
      info += "\n\n";
      info += "- If your ship was already recruited as a SOOP ship, please leave this box checked";
      info += "\n";
      info += "- If you wish to participate in the SOOP project, please send a request message to a SOOP contact person or PMO";
   }
   else // unchecked
   {
      info = "You have unchecked the SOOP checkbox";
      info += "\n\n";
      info += "- If your ship was recruited as a SOOP ship and you want to continue, please check this box again";
      info += "\n";
      info += "- If you do not wish to participate in the SOOP project anymore, please send a message to your SOOP contact person or PMO with the ship's name and call sign and the date of the last SOOP observation";
   } // else (unchecked)

   MessageBox(info.c_str(), "TurboWin message", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL); // blijft altijd de bovenste checkbox
}



void TShipDialog::Ev_Checkbox_ASAP_Indication_SelChange()
{
   string info = "";


   if (asap_checkbox -> GetCheck() == BF_CHECKED)
   {
      info = "You have ticked the ASAP checkbox";
      info += "\n\n";
      info += "- If your ship was already recruited as a ASAP ship, please leave this box checked";
      info += "\n";
      info += "- If you wish to participate in the ASAP project, please send a request message to a ASAP contact person or PMO";
   }
   else // unchecked
   {
      info = "You have unchecked the ASAP checkbox";
      info += "\n\n";
      info += "- If your ship was recruited as a ASAP ship and you want to continue, please check this box again";
      info += "\n";
      info += "- If you do not wish to participate in the ASAP project anymore, please send a message to your ASAP contact person or PMO with the ship's name and call sign and the date of the last ASAP observation";
   } // else (unchecked)

   MessageBox(info.c_str(), "TurboWin message", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL); // blijft altijd de bovenste checkbox
}

