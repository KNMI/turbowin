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

#include "tcall.h"


DEFINE_RESPONSE_TABLE1(TCallsignDialog, TDialog)
	EV_COMMAND(IDHELP, CmHelp),
   EV_COMMAND(IDLOGONVOSID, CmLogonVOSID),
END_RESPONSE_TABLE;


TCallsignTransfer::TCallsignTransfer()
{
	// initialisatie
	CallsignEditResult[0]      = '\0';
   VOSidentifierEditResult[0] = '\0';
   VOSIDpasswordEditResult[0] = '\0';

   // encryption indication (yes/no encryption required)
   EncryptionIndicationResult.AddString("\0");
   EncryptionIndicationResult.AddString("yes");
   EncryptionIndicationResult.AddString("no");
}


TCallsignDialog::TCallsignDialog(TWindow* parent, TResId resId, TCallsignTransfer& callsigntransfer,
											string file_callsign, string turbowin_dir, string file_vosidentifier, string station_type, string file_encryption_identifier)
								  : TDialog(parent, resId)
{
	callsignedit                   = new TEdit(this, IDC_CALLSIGNEDIT, sizeof(callsigntransfer.CallsignEditResult));
   encryption_indication_combobox = new TComboBox(this, IDC_ENCRYPTED_COMBOBOX);
   vosidentifieredit              = new TEdit(this, IDC_VOSIDENTIFIEREDIT, sizeof(callsigntransfer.VOSidentifierEditResult));
   vosidpasswordedit              = new TEdit(this, IDC_VOSIDPASSWORDEDIT, sizeof(callsigntransfer.VOSIDpasswordEditResult));
   text_vosidpasswordedit         = new TStatic(this, IDC_VOSIDPASSWORD_STATIC);
   text_callsignedit              = new TStatic(this, IDC_CALLSIGN_STATIC);            // om te kunnen hide/disable
   text_encrypted                 = new TStatic(this, IDC_ENCRYPTED_STATIC);           // om te kunnen hide/disable
   text_vosidentifieredit         = new TStatic(this, IDC_VOSIDENTIFIER_STATIC);       // om te kunnen hide/disable
   text_notevosid                 = new TStatic(this, IDC_NOTEVOSID_STATIC);           // om te kunnen hide/disable
   button_logon                   = new TButton(this, IDLOGONVOSID);

   // ingegraveerde TurboWin
   text_turbowin = new TStatic(this, IDC_TEXT_TURBOWIN);

	/* filter aanleggen :             */
	/* ABCDEFGHIJKLMNOPQRSTUVWXYZ     */
	/* abcdefghijklmnopqrstuvwxyz     */
	/* 0123456789                     */
	//callsignedit -> SetValidator(new TFilterValidator("A-Za-z0-9"));      // bld 390 - 392 Swan
  	callsignedit      -> SetValidator(new TFilterValidator("A-Z0-9"));      // vanaf versie 3.5 geen kleine letters meer
  	vosidentifieredit -> SetValidator(new TFilterValidator("A-Z0-9"));

 
   local_station_type               = station_type;
	local_callsign                   = file_callsign;
   local_vosidentifier              = file_vosidentifier;
   local_file_encryption_identifier = file_encryption_identifier;
   local_turbowin_dir               = turbowin_dir;

	SetTransferBuffer (&callsigntransfer);

   /* laden HTMLHelp OCX voor HTML (chm) help */
   HTMLHelp = NULL;
   hHelpOCX = NULL;
   hHelpOCX = ::LoadLibrary("hhctrl.ocx");
   if (hHelpOCX == NULL)
      MessageBox("Cannot use the html help", "TurboWin Error", MB_OK | MB_ICONERROR);

}


TCallsignDialog::~TCallsignDialog()
{
	delete callsignedit;
   delete encryption_indication_combobox;
   delete vosidentifieredit;
   delete text_turbowin;
   delete vosidpasswordedit;
   delete text_vosidpasswordedit;
   delete text_callsignedit;
   delete text_vosidentifieredit;
   delete text_notevosid;
   delete text_encrypted;
   delete button_logon;

   /* sluiten help file */
   if (HTMLHelp != NULL)
      HTMLHelp(NULL, NULL, HH_CLOSE_ALL, NULL);
   
}


void TCallsignDialog::SetupWindow()
{
	TDialog::SetupWindow();

   text_turbowin -> EnableWindow(false);


   /* fixed sea stations alleen een WMO id number (op plaats call sign) */
   if (local_station_type == FIXED_STATION)
   {
      // ander label by call sign edit box (bij fixed sea stations: in callsign edit komt wmo identification number)
      text_callsignedit -> Clear();
      text_callsignedit -> SetText("WMO identification number");

      // encryption indication helemaal weg laten
      text_encrypted                 -> ShowWindow(SW_HIDE);
      encryption_indication_combobox -> ShowWindow(SW_HIDE);

      // vos id box helemaal weg laten
      text_vosidentifieredit -> ShowWindow(SW_HIDE);
      vosidentifieredit      -> ShowWindow(SW_HIDE);

      // vos id password insert (logon) helemaal weglaten
      text_vosidpasswordedit -> ShowWindow(SW_HIDE);
      vosidpasswordedit      -> ShowWindow(SW_HIDE);
      button_logon           -> ShowWindow(SW_HIDE);

      // aanvullende vos id note
      text_notevosid         -> ShowWindow(SW_HIDE);

   } // if (local_station_type == FIXED_STATION)
   else // dus geen fixed sea station
   {
      // encrypted inserten
      if (local_file_encryption_identifier == "yes")
      {
         encryption_indication_combobox -> SetSelString("yes", -1);
      }
      else
      {
         encryption_indication_combobox -> SetSelString("no", -1);
      }
      encryption_indication_combobox -> EnableWindow(false);

      // VOS identifier inserten 
      vosidentifieredit -> Clear();
      vosidentifieredit -> Insert(local_vosidentifier.c_str());
      vosidentifieredit -> EnableWindow(false);

      // om te voorkomen als een keer password is ingevuld en hierna weer openen van dit invul scherm het password er gewoon nog staat
      vosidpasswordedit  -> Clear();

      // bij password intikken alleen *** laten zien (NB moet in SetupWindow, in de constructor zetten geeft geen effect)
      vosidpasswordedit -> SetPasswordChar(42);                  // 42 dec ascii code = *


   } // else (dus geen fixed sea station)

   /* call sign inserten (voor zowel fixed sea stations als ship) */
	callsignedit -> Clear();                                    // clears static control (of TEdit) otherwise the picked up callsign is doubled
	callsignedit -> Insert(local_callsign.c_str());

}



void TCallsignDialog::CmLogonVOSID()
{
   char buffer_password[MAX_STATION_PASSWORD_LENGTH];

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

   // uitlezen vosid edit field
   vosidpasswordedit   -> GetLine(buffer_password, MAX_STATION_PASSWORD_LENGTH, 0);

   if (strcmp(buffer_password, key.c_str()) != 0)
   {
      MessageBox("Password not correct", "TurboWin error", MB_OK | MB_ICONWARNING);
   }
   else
   {
      MessageBox("Password correct, you can insert/edit the VOS ID (masked call sign) or the call sign encryption mode (ompressed message)", "TurboWin information", MB_OK | MB_ICONINFORMATION);

      // VOS ID field dus enablen
      vosidentifieredit -> EnableWindow(true);

      // encryption yes/no conbobox field enablen
      encryption_indication_combobox -> EnableWindow(true);
   }
}


void TCallsignDialog::CmHelp()
{
   string volledig_help_path = "";
   volledig_help_path = local_turbowin_dir + callsign_help_file;
   HTMLHelp = (HtmlHelpFunc)::GetProcAddress(hHelpOCX, ATOM_HTMLHELP_API_ANSI);
   HTMLHelp(0, volledig_help_path.c_str(), HH_DISPLAY_TOPIC, 0);
}


/*
// onderstaande werkt wel, echter omdat bij creeren van deze call sign dialog de call sign altijd vanuit een files
// in de edit gezet wordt krijg je altijd een melding
//
//void TCallsignDialog::Ev_Edit_Call_Sign_Update()
//{
//   string info = "";
//
//
//   info = "Important: Please report your National Meteorological Service (PMO) that your Call Sign has been changed";
//   info += "\n";
//   info += "Your ship might become unrecognizable";
//   ::MessageBox(0, info.c_str(), "TurboWin message", MB_OK | MB_ICONINFORMATION);
//}
*/


