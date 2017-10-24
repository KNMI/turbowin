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

#include <owl\statusba.h>

#include "tmywindo.h"


/*****************************************************************************************************************************************/
// output         IMMT_log()  Test_IMMT_log()  update        Inm-C.adv    Obs compl.   BUFR   gts-header   Code_Log()   Compressed
//                                             statistics                                     optie
//
// A:\            x           x                x             x            x            x      x            x
// Screen         x           x                x             -            x                   x            x
// Custom         x           x                x             x            x            x      x            x
// Printer        x           x                x             -            x                   x            x
// Clipboard      x           x                x             -            x                   x            x
// text Editor    x           x                x             -            -                   x            x
// E-mail(Turbo)  x           x                x             -            x            x      x            x
// E-mail(OLE)    x           x                x             -            -            x      x            x
//
// Printer text   -           -                -             -            -            -      -            -
//
/*******************************************************************************************************************************************/




void TMyWindow::CmObsTextEditor()
{
	BOOL level_1_ok = TRUE;
	BOOL level_2_ok = TRUE;
	BOOL level_3_ok = TRUE;
   //string output_sort = ".....";  ..
   string turbowin_obs_text_editor_file = "";
   string aangeroepen_programma = "";
   ofstream os;
   int return_code;
   HINSTANCE WRETURN;
   bool statusbar_mode = false;


	//
	// samenstellen van de obs
	//
	checking_level_1(level_1_ok, statusbar_mode);
	if (level_1_ok == TRUE)
		checking_level_2(level_2_ok);
	if ((level_1_ok == TRUE) && (level_2_ok == TRUE))
		checking_level_3(level_3_ok);                // extremen

	if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
	{
		samenstellen_te_verzenden_obs();

      /* obs naar temp directory van waaruit de text editor hem kan inlezen */
      turbowin_obs_text_editor_file = "\0";                        // initialisatie;
      turbowin_obs_text_editor_file = turbowin_dir;                // turbowin_dir was al bepaald (direct na opstarten van TurboWin)
      turbowin_obs_text_editor_file += temp_sub_dir;
      turbowin_obs_text_editor_file += obs_text_editor_file;       // obstexteditor.txt

      os.open(turbowin_obs_text_editor_file.c_str(), ios::out);            // open om te schrijven
      if (!os)
      {
         string info = "";
         info += "Invoking text editor cancelled.\n";
         info += "(unable to write to: ";
         info += turbowin_obs_text_editor_file;                    // is absolute path naam
         info += ")";
         MessageBox(info.c_str(), "TurboWin error", MB_OK | MB_ICONSTOP);
      }
      else // os file ok
      {
         if (gts_header_resultaat == "yes")          // Denmark recruited ships kunnen hier gebruik van maken
         {
            string obs_gts_header_start_line_1;
            string obs_gts_header_start_line_2;
            string obs_gts_header_end_line;
            string destination = "text_editor";

            Maak_GTS_Header_DMI(obs_gts_header_start_line_1, obs_gts_header_start_line_2, obs_gts_header_end_line, destination);

            os.write(obs_gts_header_start_line_1.c_str(), strlen(obs_gts_header_start_line_1.c_str()));
            os.write(obs_gts_header_start_line_2.c_str(), strlen(obs_gts_header_start_line_2.c_str()));
            os.write(char_obs, strlen(char_obs));
            os.write(obs_gts_header_end_line.c_str(), strlen(obs_gts_header_end_line.c_str()));
         }
         else // geen gts header
            os.write(char_obs, strlen(char_obs));

         os.close();

         /* aanroepen text editor (die dan zelf de obs-text-editor-file inleest) */
         WRETURN = ShellExecute(0, NULL, turbowin_obs_text_editor_file.c_str(), NULL, NULL, SW_SHOW);

         return_code = (int)WRETURN;
         if (return_code <= 32)
         {
            aangeroepen_programma = "Text Editor";
            Return_Code_Invoked_Program(aangeroepen_programma, return_code);
         }
         else // text editor invoking ok
         {
            /* vullen IMMT log */
            IMMT_log();

            /* bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te '(re)moven') */
            Test_IMMT_grootte();

            /* naar logbook code file (voor private use, not for Met Service) */
            Code_Log();

            // eventueel updaten van de statistics (als ze op het scherm staan)
            if (StatisticsObserver_aangevraagd == TRUE)
            {
               // bij ingaan CmOptionsStatisticsObserver() wordt
               // StatisticsObserver_aangevraagd het 'tegenoversgestelde
               // i.v.m. vinken (dus hij was er normaal niet en na de
               // aanvraag wel, hier was hij er wel en moet hij blijven
               StatisticsObserver_aangevraagd = FALSE;
               CmOptionsStatisticsObserver();
            } // if (StatisticsObserver_aangevraagd == TRUE)

            //Observation_Complete_Message(output_sort); ..
         } // else (text editor invoking ok)
      } // else (os file ok)

	} // if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
}



void TMyWindow::CeObsTextEditor(TCommandEnabler &tce)
{
	if (code_mode == "BUFR")
		tce.Enable(false);
	else
		tce.Enable();
}



void TMyWindow::CmObsEmail1()                    // Turbo (TurboWin E-mail module)
{
	BOOL level_1_ok                    = TRUE;
	BOOL level_2_ok                    = TRUE;
	BOOL level_3_ok                    = TRUE;
   string turbowin_email              = "";              // volledig path + name TurboWin E-mail exe
   string turbowin_bufr_email         = "";              // volledig path + name TurboWin bufr E-mail exe
   string obs_email_file              = "";              // volledig path + name obs file te versturen
   string aangeroepen_programma       = "";
   string output_sort                 = "mail";
   string volledig_emailsettings_path = "";
   string info                        = "";
   string volledig_bufr_email_file    = "";
   string turbowin_email_cmd          = "";
   string turbowin_bufr_email_cmd     = "";
   HINSTANCE WRETURN;   //UINT WRETURN;
   ofstream os;
   ifstream is;
   bool win_ver_2000_xp_ok            = false;
   bool modem_con_ok                  = false;
   bool statusbar_mode                = false;
   bool bufr_encoding_geslaagd;
   bool bufr_source_geslaagd;                            // bufr "source" file aangemaakt geslaagd
   int return_code;


	//
	// samenstellen van de obs
	//
	checking_level_1(level_1_ok, statusbar_mode);
	if (level_1_ok == TRUE)
		checking_level_2(level_2_ok);
	if ((level_1_ok == TRUE) && (level_2_ok == TRUE))
		checking_level_3(level_3_ok);                     // extremen

	if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
	{
      if (code_mode == "BUFR")                          // BUFR observation
      {
         bufr_source_geslaagd = samenstellen_te_verzenden_BUFR_obs();
         if (bufr_source_geslaagd == true)
         {
            volledig_bufr_email_file = turbowin_dir + "\\temp\\" + (string)BUFR_EMAIL_FILE; // absolute pathnaam

            bufr_encoding_geslaagd = Coderen_BUFR_Wegschrijven(volledig_bufr_email_file);  // NB bufr_email_file is obsolute path naam
            if (bufr_encoding_geslaagd == true)            // succesvol omgezet naar BUFR en weggeschreven (naar volledig_bufr_email_file = ..\temp\obs.bin
            {

///////////////////////////
               // E-mail settings file moet aanwezig zijn (wordt uitgelezen door de E-mail module zelf)
               volledig_emailsettings_path = turbowin_dir + email_sub_dir + email_settings_file;
               is.open(volledig_emailsettings_path.c_str(), ios::in);
               if (is)
               {
                  is.close();                                      // werd alleen geopend voor testen op aanwezigheid

                  turbowin_bufr_email = turbowin_dir;
                  turbowin_bufr_email += email_sub_dir;
                  turbowin_bufr_email += turbowin_bufr_email_module_name;    // absolute path naam (indy_bufr_smtp.exe)

                  /* Windows 2000 en XP in combinatie met een internet connectie via modem + telefoon lijn   */
                  /* moeten zelf zorgen voor een eerste (RAS) internet verbinding                            */
                  /* dit kan via aanroepen rasphone.exe (in system32 dir van windows root dir)               */
                  /* i.v.m. synchronistie dit via een cmd (windows NT scripting file) aanroep laten verlopen */


                  // Windows 2000, XP ?
                  win_ver_2000_xp_ok = Check_Windows_Version();

                  // internet via Modem en telefoon line (dial-up connection) ?
                  if (win_ver_2000_xp_ok == true)
                     modem_con_ok = Check_Modem_Phone_Line_Connection();

                  if (modem_con_ok == true)    // dus dan: windows 2000/xp met een modem phone line internet verbinding
                  {
                     /* om het zelfde effect te krijgen bij aanroepen email module bij modem phone line en b.v. LAN */
                     /* moet de cmd file in de turbowin root dir aanwezig zijn (en b.v. niet in de email dir)       */
                     turbowin_bufr_email_cmd = turbowin_dir;
                     turbowin_bufr_email_cmd += "\\";                        //email_sub_dir;
                     turbowin_bufr_email_cmd += bufr_email_cmd_file;         // b.v. bufr_email.cmd

                     /* LET OP: WINEXEC geeft buiten IDE foutmelding onder Windows XP */
                     //WRETURN = WinExec(turbowin_email_cmd.c_str(), SW_MINIMIZE);
                     WRETURN = ShellExecute(0, NULL, "bufr_email.cmd", NULL, NULL, SW_MINIMIZE);
                     aangeroepen_programma = "TurboWin E-mail module (bufr_email.cmd)";  // eventueel voor foutmelding
                  } // if (modem_con_ok == true)
                  else // geen email.cmd aanroep nodig (direct aanroep naar email module)
                  {
                     WRETURN = ShellExecute(0, NULL, turbowin_bufr_email.c_str(), NULL, NULL, SW_SHOW);
                     aangeroepen_programma = "TurboWin E-mail module (indy_bufr_smtp.exe)"; // eventueel voor foutmelding
                  } // else (geen cmd nodig)

                  return_code = (int)WRETURN;
                  if (return_code <= 32)
                  {
                     Return_Code_Invoked_Program(aangeroepen_programma, return_code);
                  } // if (return_code <= 32)
                  else // Email module file ok
                  {
                     IMMT_log();                               // vullen IMMT log


                     // bij een fixed sea station ook positie weer ophalen was bij initialisatie gereset)
                     //if (station_type.compare("fixed") == 0)
                     //	Read_fixed_station_data();

                     /* bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te 'moven') */
                     Test_IMMT_grootte();

                     /* naar logbook code file (voor private use, not for Met Service) */
                     Code_Log();

                     /* eventueel updaten van de statistics (als ze op het scherm staan) */
                     if (StatisticsObserver_aangevraagd == TRUE)
                     {
                        // bij ingaan CmOptionsStatisticsObserver() wordt
                        // StatisticsObserver_aangevraagd het 'tegenoversgestelde
                        // i.v.m. vinken (dus hij was er normaal niet en na de
                        // aanvraag wel, hier was hij er wel en moet hij blijven
                        StatisticsObserver_aangevraagd = FALSE;
                        CmOptionsStatisticsObserver();
                     } // if (StatisticsObserver_aangevraagd == TRUE)

                     Observation_Complete_Message(output_sort);
                     /* i.v.m. hier ook komen na b.v. Cancel in de E-mail module  */
                     /* de complete text verplaatst naar TurboWin E-mail module   */
                     /* echter de timer begint wel te lopen                       */

                  } // else (E-mail module file ok)
               } // if (is)
               else // no e-mail settings file
               {
                  info = "Unable to open TurboWin E-mail settings file (";
                  info += volledig_emailsettings_path;
                  info += ").";
                  info += " Please select: Maintenance | E-mail settings.";
                  ::MessageBox(0, info.c_str(), "TurboWin error", MB_OK | MB_ICONERROR);
               } // else (no e-mail settings file)

/////////////////////////////////

            } // if (bufr_encoding_geslaagd == true)
         } // if (bufr_source_geslaagd == true)

      } // if (code_mode == "BUFR")
      else // geen BUFR
      {
         samenstellen_te_verzenden_obs();

         obs_email_file = turbowin_dir;
         obs_email_file += email_sub_dir;
         obs_email_file += "obs.txt";

         os.open(obs_email_file.c_str(), ios::out);         // open om te schrijven
         if (!os)
         {
            string info = "\0";
            info += "Unable to write obs to: ";
            info += obs_email_file;                         // is absolute path naam
            info += ".\n";
            MessageBox(info.c_str(), "TurboWin error", MB_OK | MB_ICONSTOP);
         } // if (!os)
         else // obs file ok
         {
            //os.write(char_obs, strlen(char_obs));
            if (gts_header_resultaat == "yes")          // Denmark recruited ships kunnen hier gebruik van maken
            {
               string obs_gts_header_start_line_1;
               string obs_gts_header_start_line_2;
               string obs_gts_header_end_line;
               string destination = "EMAIL_TURBO";

               Maak_GTS_Header_DMI(obs_gts_header_start_line_1, obs_gts_header_start_line_2, obs_gts_header_end_line, destination);
               os.write(obs_gts_header_start_line_1.c_str(), strlen(obs_gts_header_start_line_1.c_str()));
               os.write(obs_gts_header_start_line_2.c_str(), strlen(obs_gts_header_start_line_2.c_str()));
               os.write(char_obs, strlen(char_obs));
               os.write(obs_gts_header_end_line.c_str(), strlen(obs_gts_header_end_line.c_str()));
            }
            else // geen gts header
               os.write(char_obs, strlen(char_obs));

            os.close();

            // E-mail settings file moet aanwezig zijn
            volledig_emailsettings_path = turbowin_dir + email_sub_dir + email_settings_file;
            is.open(volledig_emailsettings_path.c_str(), ios::in);
            if (is)
            {
               is.close();                                      // werd alleen geopend voor testen op aanwezigheid

               turbowin_email = turbowin_dir;
               turbowin_email += email_sub_dir;
               turbowin_email += turbowin_email_module_name;    // absolute path naam

               /* Windows 2000 en XP in combinatie met een internet connectie via modem + telefoon lijn   */
               /* moeten zelf zorgen voor een eerste (RAS) internet verbinding                            */
               /* dit kan via aanroepen rasphone.exe (in system32 dir van windows root dir)               */
               /* i.v.m. synchronistie dit via een cmd (windows NT scripting file) aanroep laten verlopen */


               // Windows 2000, XP ?
               win_ver_2000_xp_ok = Check_Windows_Version();

               // internet via Modem en telefoon line (dial-up connection) ?
               if (win_ver_2000_xp_ok == true)
                  modem_con_ok = Check_Modem_Phone_Line_Connection();

               if (modem_con_ok == true)    // dus dan: windows 2000/xp met een modem phone line internet verbinding
               {
                  /* om het zelfde effect te krijgen bij aanroepen email module bij modem phone line en b.v. LAN */
                  /* moet de cmd file in de turbowin root dir aanwezig zijn (en b.v. niet in de email dir)       */
                  turbowin_email_cmd = turbowin_dir;
                  turbowin_email_cmd += "\\";                   //email_sub_dir;
                  turbowin_email_cmd += email_cmd_file;         // b.v. email.cmd

                  /* LET OP: WINEXEC geeft buiten IDE foutmelding onder Windows XP */
                  //WRETURN = WinExec(turbowin_email_cmd.c_str(), SW_MINIMIZE);
                  WRETURN = ShellExecute(0, NULL, "email.cmd", NULL, NULL, SW_MINIMIZE);
                  aangeroepen_programma = "TurboWin E-mail module (email.cmd)";  // eventueel voor foutmelding
               } // if (modem_con_ok == true)
               else // geen email.cmd aanroep nodig (direct aanroep naar email module)
               {
                  WRETURN = ShellExecute(0, NULL, turbowin_email.c_str(), NULL, NULL, SW_SHOW);
                  aangeroepen_programma = "TurboWin E-mail module (indy_smtp.exe)";              // eventueel voor foutmelding
               } // else (geen cmd nodig)

               return_code = (int)WRETURN;
               if (return_code <= 32)
               {
                  Return_Code_Invoked_Program(aangeroepen_programma, return_code);
               } // if (return_code <= 32)
               else // Email module file ok
               {
                  IMMT_log();                               // vullen IMMT log

                  // bij een fixed sea station ook positie weer ophalen was bij initialisatie gereset)
                  //if (station_type.compare("fixed") == 0)
                  //	Read_fixed_station_data();

                  // bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te 'moven')
                  Test_IMMT_grootte();

                  /* naar logbook code file (voor private use, not for Met Service) */
                  Code_Log();

                  // eventueel updaten van de statistics (als ze op het scherm staan)
                  if (StatisticsObserver_aangevraagd == TRUE)
                  {
                     // bij ingaan CmOptionsStatisticsObserver() wordt
                     // StatisticsObserver_aangevraagd het 'tegenoversgestelde
                     // i.v.m. vinken (dus hij was er normaal niet en na de
                     // aanvraag wel, hier was hij er wel en moet hij blijven
                     StatisticsObserver_aangevraagd = FALSE;
                     CmOptionsStatisticsObserver();
                  } // if (StatisticsObserver_aangevraagd == TRUE)

                  Observation_Complete_Message(output_sort);
                  /* i.v.m. hier ook komen na b.v. Cancel in de E-mail module  */
                  /* de complete text verplaatst naar TurboWin E-mail module   */
                  /* echter de timer begint wel te lopen                       */

               } // else (E-mail module file ok)

            } // if (is)
            else // no e-mail settings file
            {
               info = "Unable to open TurboWin E-mail settings file (";
               info += volledig_emailsettings_path;
               info += ").";
               info += " Please select: Maintenance | E-mail settings.";
               ::MessageBox(0, info.c_str(), "TurboWin error", MB_OK | MB_ICONERROR);
            } // else (no e-mail settings file)

         } // else (obs file ok)
      } // else (geen BUFR)
   } // if ((level_1_ok == true) etc.
}



//void TMyWindow::CeObsEmail1(TCommandEnabler &tce)
//{
//	if (code_mode == "BUFR")
//		tce.Enable(false);
//	else
//		tce.Enable();
//}



void TMyWindow::CmObsEmail2()      // OLE
{
	BOOL level_1_ok = TRUE;
	BOOL level_2_ok = TRUE;
	BOOL level_3_ok = TRUE;
   ifstream is;
   string volledig_emailsettings_path = "";
   string record;
   string info = "";
   string link = "";
   string linkto = "";
   string linksubject = "";
   string volledig_bufr_email_file = "";
   string output_sort = "mail";
   string var_date_time;               // de lopende dag-uur-minuut (b.v. 171200)
   bool win_ver_2000_xp_ok = false;
   bool modem_con_ok = false;
   bool statusbar_mode = false;
   bool bufr_encoding_geslaagd;
   bool bufr_source_geslaagd;                         // bufr "source" file aangemaakt geslaagd


	//
	// samenstellen van de obs
	//
	checking_level_1(level_1_ok, statusbar_mode);
	if (level_1_ok == TRUE)
		checking_level_2(level_2_ok);
	if ((level_1_ok == TRUE) && (level_2_ok == TRUE))
		checking_level_3(level_3_ok);                // extremen

	if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
	{
      if (code_mode == "BUFR")                          // BUFR observation
      {
         bufr_source_geslaagd = samenstellen_te_verzenden_BUFR_obs();
         if (bufr_source_geslaagd == true)
         {
            volledig_bufr_email_file = turbowin_dir + "\\temp\\" + (string)BUFR_EMAIL_FILE; // absolute pathnaam

            bufr_encoding_geslaagd = Coderen_BUFR_Wegschrijven(volledig_bufr_email_file);  // NB bufr_email_file is obsolute path naam
            if (bufr_encoding_geslaagd == true)            // succesvol omgezet naar BUFR en weggeschreven
            {
               // emailsettings_file
               volledig_emailsettings_path = turbowin_dir + email_sub_dir + email_settings_file;
               is.open(volledig_emailsettings_path.c_str(), ios::in);
               if (is)
               {
                  do // while (!is.eof());
                  {
                     record.read_line(is);
                     if (record.substr(0, 20) == text_toaddress)
                     {
                        if (record.length() >= 20)
                           linkto = record.substr(20);
                     } // if (record.substr(0, 20) == text_toaddress)
                     else if (record.substr(0, 20) == text_subject)
                     {
                        if (record.length() >= 20)
                        {
                           // als er ddhhmm in staat dan substitueren met echte dag-uur-minuut
                           if (record.find("ddhhmm") != NPOS)
                           {
                              // n.b. er moet dus geen ddhhmm staan in text string (voor de :)
                              var_date_time = (string)char_DD + (string)char_GG + "00";

                              try
                              {
                                 record.replace(record.find("ddhhmm"), 6, var_date_time);
                              }
                              catch (...)
                              {
                              }

                           } // if (record.find("ddhhmm") != NPOS)

                           linksubject = record.substr(20);

                           if (linksubject == "")                    // b.v. outlook kan niet tegen een leeg object field by : "?subject="
                              linksubject = "obs";

                        } // if (record.length() >= 20)
                     } // else if (record.substr(0, 20) == text_subject)

                  } while (!is.eof());

                  is.close();

                  // parameter string opbouwen
                  link = "mailto:";
                  link += linkto;
                  link += "?subject=";
                  link += linksubject;
                  link += "&body=";
                  link += "Please attach manually the file: ";
                  link += volledig_bufr_email_file;

                  /* reminder dat het in plain text verstuurd moet worden */
                  //info = "";
                  //info = "The observation message must be sent as plain text only. ";
                  //info += "Make sure that an HTML option is switched off in your E-mail program";
                  //MessageBox(info.c_str(), "TurboWin reminder \"plain text\"", MB_OK | MB_ICONINFORMATION);


                  MenuEmailLink2 -> Open(link);

                  // Windows 2000, XP ?
                  win_ver_2000_xp_ok = Check_Windows_Version();

                  // internet via Modem en telefoon line (dial-up connection) ?
                  if (win_ver_2000_xp_ok == true)
                     modem_con_ok = Check_Modem_Phone_Line_Connection();

                  if (modem_con_ok == true)    // dus dan: windows 2000/xp met een modem phone line internet verbinding
                  {
                     info = "(after you chose the 'Send' command) \n please ensure that the observation has been sent, \n in other words not queued in the mail outbox";
                     MessageBox(info.c_str(), "Turbowin mesage", MB_OK | MB_ICONINFORMATION);
                  }

                  IMMT_log();                               // vullen IMMT log

                  // bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te 'moven')
                  Test_IMMT_grootte();

                  /* naar logbook code file (voor private use, not for Met Service) */
                  Code_Log();

                  // eventueel updaten van de statistics (als ze op het scherm staan)
                  if (StatisticsObserver_aangevraagd == TRUE)
                  {
                     /* bij ingaan CmOptionsStatisticsObserver() wordt         */
                     /* StatisticsObserver_aangevraagd het 'tegenoversgestelde */
                     /* i.v.m. vinken (dus hij was er normaal niet en na de    */
                     /* aanvraag wel, hier was hij er wel en moet hij blijven  */
                     StatisticsObserver_aangevraagd = FALSE;
                     CmOptionsStatisticsObserver();
                  } // if (StatisticsObserver_aangevraagd == TRUE)

                  Observation_Complete_Message(output_sort);
                  /* N.B. voor geven messagebox met complete message text                              */
                  /* werkt dit niet goed omdat via OLE het geregistreerde mail programma is opgestard  */
                  /* en direct wordt de message complete gegeven (zonder dat je wat hebt verzonden)    */
                  /* wordt echter aangeroepn voor instellen timer */
               } // if (is)
               else // file not open
               {
                  info = "";
                  info = "Unable to open TurboWin E-mail settings file (";
                  info += volledig_emailsettings_path;
                  info += ").";
                  info += " Please select: Maintenance | E-mail settings.";
                  ::MessageBox(0, info.c_str(), "TurboWin error", MB_OK | MB_ICONERROR);
               } // else (file not open)

            } // if (bufr_encoding_geslaagd == true)
         } // if (bufr_source_geslaagd == true)

      } // if (code_mode == "BUFR")
      else // geen BUFR
      {
         samenstellen_te_verzenden_obs();

         //
         // emailsettings_file
         //
         volledig_emailsettings_path = turbowin_dir + email_sub_dir + email_settings_file;
         is.open(volledig_emailsettings_path.c_str(), ios::in);
         if (is)
         {
            do // while (!is.eof());
            {
               record.read_line(is);
               if (record.substr(0, 20) == text_toaddress)
               {
                  if (record.length() >= 20)
                     linkto = record.substr(20);
               } // if (record.substr(0, 20) == text_toaddress)
               else if (record.substr(0, 20) == text_subject)
               {
                  if (record.length() >= 20)
                  {
                     // als er ddhhmm in staat dan substitueren met echte dag-uur-minuut
                     if (record.find("ddhhmm") != NPOS)
                     {
                        // n.b. er moet dus geen ddhhmm staan in text string (voor de :)
                        var_date_time = (string)char_DD + (string)char_GG + "00";

                        try
                        {
                           record.replace(record.find("ddhhmm"), 6, var_date_time);
                        }
                        catch (...)
                        {
                        }

                     } // if (record.find("ddhhmm") != NPOS)

                     linksubject = record.substr(20);

                     if (linksubject == "")                    // b.v. outlook kan niet tegen een leeg object field by : "?subject="
                        linksubject = "obs";

                  } // if (record.length() >= 20)
               } // else if (record.substr(0, 20) == text_subject)

            } while (!is.eof());

            is.close();


            if (gts_header_resultaat == "yes")          // Denmark recruited ships kunnen hier gebruik van maken
            {
               string obs_gts_header_start_line_1;
               string obs_gts_header_start_line_2;
               string obs_gts_header_end_line;
               string destination = "EMAIL_OLE";

               Maak_GTS_Header_DMI(obs_gts_header_start_line_1, obs_gts_header_start_line_2, obs_gts_header_end_line, destination);

               // parameter string opbouwen
               link = "mailto:";
               link += linkto;
               link += "?subject=";
               link += linksubject;
               link += "&body=";

               link += obs_gts_header_start_line_1;
               link += "%0A";
               link += obs_gts_header_start_line_2;
               link += "%0A";
               link += string(char_obs_1);            // b.v. bij outlook 98 komt de newline niet goed over
               link += "%0A";                          // new-line
               //link += "%0D";                        // carriage return
               link += string(char_obs_2);

               /* indien nationale groepen dan char_obs_3 toevoegen */
               if (strcmp(char_obs_3, "\0") != 0)
               {
                  link += "%0A";                          // new-line
                  link += string(char_obs_3);
               } // if (strcpy(char_obs_3, "\0") != 0)

               link += "%0A";
               link += obs_gts_header_end_line;

               //link += "%03";

            } // if (gts_header_resultaat == "yes")
            else // geen gts header
            {
               // parameter string opbouwen
               link = "mailto:";
               link += linkto;
               link += "?subject=";
               link += linksubject;
               link += "&body=";
               link += string(char_obs_1);            // b.v. bij outlook 98 komt de newline niet goed over
               link += "%0A";                          // new-line
               //link += "%0D";                        // carriage return
               link += string(char_obs_2);

               /* indien nationale groepen dan char_obs_3 toevoegen */
               if (strcmp(char_obs_3, "\0") != 0)
               {
                  link += "%0A";                          // new-line
                  link += string(char_obs_3);
               } // if (strcpy(char_obs_3, "\0") != 0)
            } // else // geen gts header


            /* reminder dat het in plain text verstuurd moet worden */
            info = "";
            info = "The observation message must be sent as plain text only. ";
            info += "Make sure that an HTML option is switched off in your E-mail program";
            MessageBox(info.c_str(), "TurboWin reminder \"plain text\"", MB_OK | MB_ICONINFORMATION);


            MenuEmailLink2 -> Open(link);

            //ShellExecute(0, NULL, "mailto:m.m.stam@hetnet.nl?subject=test&body=test50body en nog wat", NULL, NULL, SW_SHOW);
            //ShellExecute(0, NULL, "mailto:", NULL, NULL, SW_SHOW);  // werkt wel opent een nieuw E-mail om te verzenden
            //delete MenuEmailLink2;
            //MenuEmailLink2 ->
            //link = "";
            //link = "mailto:";
            //MenuEmailLink2 -> Open(link);
            //WinExec("mailto", SW_SHOW);
            //MessageBox("gepasseerd", "test", MB_OK);

            // Windows 2000, XP ?
            win_ver_2000_xp_ok = Check_Windows_Version();

            // internet via Modem en telefoon line (dial-up connection) ?
            if (win_ver_2000_xp_ok == true)
               modem_con_ok = Check_Modem_Phone_Line_Connection();

            if (modem_con_ok == true)    // dus dan: windows 2000/xp met een modem phone line internet verbinding
            {
               info = "(after you chose the 'Send' command) \n please ensure that the observation has been sent, \n in other words not queued in the mail outbox";
               MessageBox(info.c_str(), "Turbowin mesage", MB_OK | MB_ICONINFORMATION);
            }


            IMMT_log();                               // vullen IMMT log

            // bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te 'moven')
            Test_IMMT_grootte();

            /* naar logbook code file (voor private use, not for Met Service) */
            Code_Log();

            // eventueel updaten van de statistics (als ze op het scherm staan)
            if (StatisticsObserver_aangevraagd == TRUE)
            {
               /* bij ingaan CmOptionsStatisticsObserver() wordt         */
               /* StatisticsObserver_aangevraagd het 'tegenoversgestelde */
               /* i.v.m. vinken (dus hij was er normaal niet en na de    */
               /* aanvraag wel, hier was hij er wel en moet hij blijven  */
               StatisticsObserver_aangevraagd = FALSE;
               CmOptionsStatisticsObserver();
            } // if (StatisticsObserver_aangevraagd == TRUE)

            Observation_Complete_Message(output_sort);
            /* N.B. voor geven messagebox met complete message text                              */
            /* werkt dit niet goed omdat via OLE het geregistreerde mail programma is opgestard  */
            /* en direct wordt de message complete gegeven (zonder dat je wat hebt verzonden)    */
            /* wordt echter aangeroepn voor instellen timer */

         } // if (is)
         else // file not open
         {
            info = "";
            info = "Unable to open TurboWin E-mail settings file (";
            info += volledig_emailsettings_path;
            info += ").";
            info += " Please select: Maintenance | E-mail settings.";
            ::MessageBox(0, info.c_str(), "TurboWin error", MB_OK | MB_ICONERROR);
         } // else (file not open)
      } // else (geen BUFR)
   } // if ((level_1_ok == true) etc.
}



//void TMyWindow::CeObsEmail2(TCommandEnabler &tce)
//{
//	if (code_mode == "BUFR")
//		tce.Enable(false);
//	else
//		tce.Enable();
//}



void TMyWindow::CmObsClipboard()
{
	BOOL level_1_ok = TRUE;
	BOOL level_2_ok = TRUE;
	BOOL level_3_ok = TRUE;
   string output_sort = "clipboard";
   string clipboard_obs = "";
   string hulp_obs = "";
   bool statusbar_mode = false;


	//
	// samenstellen van de obs
	//
	checking_level_1(level_1_ok, statusbar_mode);
	if (level_1_ok == TRUE)
		checking_level_2(level_2_ok);
	if ((level_1_ok == TRUE) && (level_2_ok == TRUE))
		checking_level_3(level_3_ok);                // extremen

	if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
	{
      samenstellen_te_verzenden_obs();

////////////////////////////////////////////

      /* i.v.m. regelovergangen een variable hulp_obs ("\n" werkt niet bij b.v. kladblok) */
      hulp_obs = string(char_obs_1) + "\xD\xA" + string(char_obs_2);

      /* indien nationale groepen dan char_obs_3 toevoegen */
      if (strcmp(char_obs_3, "\0") != 0)
      {
         hulp_obs += "\xD\xA";
         hulp_obs += string(char_obs_3);
      } // if (strcpy(char_obs_3, "\0") != 0)


      if (gts_header_resultaat == "yes")          // Denmark recruited ships kunnen hier gebruik van maken
      {
         string obs_gts_header_start_line_1;
         string obs_gts_header_start_line_2;
         string obs_gts_header_end_line;
         string destination = "CLIPBOARD";

         Maak_GTS_Header_DMI(obs_gts_header_start_line_1, obs_gts_header_start_line_2, obs_gts_header_end_line, destination);

         clipboard_obs = obs_gts_header_start_line_1 + obs_gts_header_start_line_2 + hulp_obs + obs_gts_header_end_line;
      } // if (gts_header_resultaat == "yes")
      else // geen gts header
         clipboard_obs = string(hulp_obs);


      /* NB komt uit hlinkctl.cpp (op EmptyClipboard na) */
      if (::OpenClipboard(GetHandle()/*HWindow*/))
      {
         EmptyClipboard();
         int nBytes = sizeof(TCHAR) * (_tcslen(clipboard_obs.c_str()) + 1);
         HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, nBytes);
         TCHAR* pData = (TCHAR*) GlobalLock(hMem);
         _tcscpy(pData, (LPCTSTR) clipboard_obs.c_str());
         GlobalUnlock(hMem);
  	      SetClipboardData(CF_TEXT, hMem);
         CloseClipboard();
      } // if (::OpenClipboard(HWindow))


///////////////////////////////////////////////

// situatie voor gts_header (dec 2003)
//
//      // komt uit hlinkctl.cpp (op EmptyClipboard na)
//      if (::OpenClipboard(HWindow))
//      {
//         EmptyClipboard();
//         int nBytes = sizeof(TCHAR) * (_tcslen(char_obs) + 1);
//         HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, nBytes);
//         TCHAR* pData = (TCHAR*) GlobalLock(hMem);
//         _tcscpy(pData, (LPCTSTR) char_obs);
//         GlobalUnlock(hMem);
//  	      SetClipboardData(CF_TEXT, hMem);
//         CloseClipboard();
//      } // if (::OpenClipboard(HWindow))

		IMMT_log();                               // vullen IMMT log

 	   // bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te 'moven')
		Test_IMMT_grootte();

      /* naar logbook code file (voor private use, not for Met Service) */
      Code_Log();

		// eventueel updaten van de statistics (als ze op het scherm staan)
		if (StatisticsObserver_aangevraagd == TRUE)
		{
			/* bij ingaan CmOptionsStatisticsObserver() wordt            */
			/* StatisticsObserver_aangevraagd het 'tegenoversgestelde    */
			/* i.v.m. vinken (dus hij was er normaal niet en na de       */
			/* aanvraag wel, hier was hij er wel en moet hij blijven     */
			StatisticsObserver_aangevraagd = FALSE;
			CmOptionsStatisticsObserver();
		} // if (StatisticsObserver_aangevraagd == TRUE)

		Observation_Complete_Message(output_sort);


   } // if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
}


void TMyWindow::CeObsClipboard(TCommandEnabler &tce)
{
	if (code_mode == "BUFR")
		tce.Enable(false);
	else
		tce.Enable();
}



void TMyWindow::CmPlainText()
{
   Printing = 0;
   bool gebruik_default_temp = true;

   // bij checking level_3 gebeurd MSL_Herleiding luchtdruk ook als dit niet eerder
   // was gebeurd omdat er nog geen air temp was.
   // Echter bij direct printen als plain text (en niet eerst via een andere output mogelijkheid)
   // is dit dus dan nog niet gebeurd.
   //
   //
   // omdat er geen dry bulb temp ingevuld is en wel een NIET naar sea level herleide luchtdruk
   if ((barometer_MSL_yes_no.compare("no") == 0) && (num_TTT == MAXINT) &&
      (num_PPPP != MAXINT) && (num_MSL_PPPP == MAXINT))
   {
      MSL_Herleiding_Luchtdruk(gebruik_default_temp);         // hier wordt WEL een default air temp gebruikt

	   if (MeteoProgressDialog -> MeteoProgressDialog_Created == TRUE)
	      MeteoProgressDialog -> UpdateAirpressureMeteoProgressDialog
		  										 (obs_Pressure, obs_Pressuretienden,
                                      obs_MSL_Pressure, obs_MSL_Pressuretienden,
			 									  obs_Amounttendency, obs_Amounttiendentendency,
			  									  obs_Characteristics);
   } // if ((barometer_MSL_yes_no.compare("no") == 0) etc.


   // create Printer object if not already exists
   if (!Printer)
      Printer = new TPrinter;

   TWindowPrintout2 printout("Plain text weather report",
                                 obs_callsign,
                                 obs_LatitudeDegrees,
                                 obs_LatitudeTenths,
                                 obs_North_or_South,
                                 obs_LongitudeDegrees,
                                 obs_LongitudeTenths,
                                 obs_East_or_West,
                                 obs_Year,
                                 obs_Month,
                                 obs_Day,
                                 obs_Time,
                                 obs_Airtemp,
                                 obs_Airtemptienden,
                                 char_UUU,
                                 obs_Seawater,
                                 obs_Seawatertienden,
                                 obs_Pressure,
                                 obs_Pressuretienden,
                                 obs_MSL_Pressure,
                                 obs_MSL_Pressuretienden,
                                 obs_wind_speed,
                                 obs_wind_direction,
                                 iw_units,
                                 obs_WavesFixedHeightTenths,
                                 obs_WavesFixedHeightMetres,
                                 obs_WavesFixedPeriod,
                                 obs_SeaWaveHeight,
                                 obs_SeaWavePeriod,
                                 obs_FirstSwellHeight,
                                 obs_FirstSwellPeriod,
                                 obs_FirstSwellDirection,
                                 obs_SecondSwellHeight,
                                 obs_SecondSwellPeriod,
                                 obs_SecondSwellDirection,
                                 obs_VV,
                                 obs_observername,
                                 obs_PresentWeather,
                                 station_type,
                                 golven_resultaat,
                       GetApplication() -> GetMainWindow() -> GetClientWindow());


   printout.SetBanding(false);      /* werkt niet op sommige HP printers: printout.SetBanding(true); */
   Printer -> Print(GetApplication() -> GetMainWindow() -> GetClientWindow(), printout, true);


   //if (Printer -> Print(GetApplication() -> GetMainWindow() -> GetClientWindow(), printout, TRUE))
   //{
   //	MessageBox("test Observation completed, you can quit this program.", "TurboWin", MB_OK | MB_ICONASTERISK);
   //}
   //else // if (Printer -> Print(GetApplication() -> GetMainWindow() -> GetClientWindow(), printout, TRUE))
   //	MessageBox("test Obs NOT printed and NOT stored.", "TurboWin warning", MB_OK | MB_ICONEXCLAMATION);

}



void TMyWindow::CmObsScreen()
{
	BOOL level_1_ok = TRUE;
	BOOL level_2_ok = TRUE;
	BOOL level_3_ok = TRUE;
	string info;
   string output_sort = "screen";
   bool statusbar_mode = false;


	//
	// samenstellen van de obs
	//
	checking_level_1(level_1_ok, statusbar_mode);
	if (level_1_ok == TRUE)
		checking_level_2(level_2_ok);
	if ((level_1_ok == TRUE) && (level_2_ok == TRUE))
		checking_level_3(level_3_ok);                // extremen

	if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
	{
		samenstellen_te_verzenden_obs();

      if (gts_header_resultaat == "yes")          // Denmark recruited ships kunnen hier gebruik van maken
      {
         string obs_gts_header_start_line_1;
         string obs_gts_header_start_line_2;
         string obs_gts_header_end_line;
         string destination = "SCREEN";

         Maak_GTS_Header_DMI(obs_gts_header_start_line_1, obs_gts_header_start_line_2, obs_gts_header_end_line, destination);


		   info = "\0";
         info += obs_gts_header_start_line_1;
         info += obs_gts_header_start_line_2;
		   info += string(char_obs_1);
		   info += string(" ");
		   info += string(char_obs_2);

         /* indien nationale groepen dan char_obs_3 toevoegen */
         if (strcmp(char_obs_3, "\0") != 0)
         {
            info += " ";
            info += string(char_obs_3);
         } // if (strcpy(char_obs_3, "\0") != 0)

         info += obs_gts_header_end_line;

      } // if (gts_header_resultaat == "yes")
      else // geen gts header
      {
		   /* n.b. voor het naar scherm schrijven moet er geen CR in zitten */
		   info = "\0";
		   info += string(char_obs_1);
		   info += string(" ");
		   info += string(char_obs_2);

         /* indien nationale groepen dan char_obs_3 toevoegen */
         if (strcmp(char_obs_3, "\0") != 0)
         {
            info += " ";
            info += string(char_obs_3);
         } // if (strcpy(char_obs_3, "\0") != 0)
      } // else (geen gts header)

		MessageBox(info.c_str(), "TurboWin obs", MB_OK);

		IMMT_log();                               // vullen IMMT log

   	// bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te 'moven')
		Test_IMMT_grootte();

      /* naar logbook code file (voor private use, not for Met Service) */
      Code_Log();

		// eventueel updaten van de statistics (als ze op het scherm staan)
		if (StatisticsObserver_aangevraagd == TRUE)
		{
			// bij ingaan CmOptionsStatisticsObserver() wordt
			// StatisticsObserver_aangevraagd het 'tegenoversgestelde
			// i.v.m. vinken (dus hij was er normaal niet en na de
			// aanvraag wel, hier was hij er wel en moet hij blijven
			StatisticsObserver_aangevraagd = FALSE;
			CmOptionsStatisticsObserver();
		} // if (StatisticsObserver_aangevraagd == TRUE)

		Observation_Complete_Message(output_sort);

	} // if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
}



void TMyWindow::CeObsScreen(TCommandEnabler &tce)
{
	if (code_mode == "BUFR")
		tce.Enable(false);
	else
		tce.Enable();
}



void TMyWindow::CmFilePrint()
{
	BOOL level_1_ok = TRUE;
	BOOL level_2_ok = TRUE;
	BOOL level_3_ok = TRUE;
   string output_sort = "printer";
   char printer_obs[1024];
   bool statusbar_mode = false;


	//
	// samenstellen van de obs
	//
	checking_level_1(level_1_ok, statusbar_mode);
	if (level_1_ok == TRUE)
		checking_level_2(level_2_ok);
	if ((level_1_ok == TRUE) && (level_2_ok == TRUE))
		checking_level_3(level_3_ok);                // extremen

	if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
	{
		samenstellen_te_verzenden_obs();

		Printing = 0;

		// create Printer object if not already exists
		if (!Printer)
			Printer = new TPrinter;


      /* initialisatie */
      printer_obs[0] = '\0';

      if (gts_header_resultaat == "yes")          // Denmark recruited ships kunnen hier gebruik van maken
      {
         string obs_gts_header_start_line_1;
         string obs_gts_header_start_line_2;
         string obs_gts_header_end_line;
         string destination = "PRINTER";

         Maak_GTS_Header_DMI(obs_gts_header_start_line_1, obs_gts_header_start_line_2, obs_gts_header_end_line, destination);

         strcpy(printer_obs, obs_gts_header_start_line_1.c_str());
         strcat(printer_obs, obs_gts_header_start_line_2.c_str());
         strcat(printer_obs, char_obs);
         strcat(printer_obs, obs_gts_header_end_line.c_str());

         //printer_obs = obs_gts_header_start_line + (string)char_obs + obs_gts_header_end_line;

      } // if (gts_header_resultaat == "yes")
      else // geen gts header
         //printer_obs = string(hulp_obs);
         strcpy(printer_obs, char_obs);

		//TWindowPrintout printout("Observation", char_obs, GetApplication() -> GetMainWindow() -> GetClientWindow());
		TWindowPrintout printout("Observation", printer_obs, GetApplication() -> GetMainWindow() -> GetClientWindow());



		printout.SetBanding(false); /* werkt niet op sommige HP local printers: printout.SetBanding(true); */
                                  /* If Banding is true, the printout is banded and the PrintPage function is called once for */
                                  /* every band. Otherwise, PrintPage is called only once for every page. Banding a printout  */
                                  /* is more memory- and time-efficient than not banding. By default, Banding is set to false.*/
		if (Printer -> Print(GetApplication() -> GetMainWindow() -> GetClientWindow(), printout, true))
		{
			IMMT_log();                               // vullen IMMT log

			// bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te 'moven')
			Test_IMMT_grootte();

         /* naar logbook code file (voor private use, not for Met Service) */
         Code_Log();

			// eventueel updaten van de statistics (als ze op het scherm staan)
			if (StatisticsObserver_aangevraagd == TRUE)
			{
				// bij ingaan CmOptionsStatisticsObserver() wordt
				// StatisticsObserver_aangevraagd het 'tegenoversgestelde
				// i.v.m. vinken (dus hij was er normaal niet en na de
				// aanvraag wel, hier was hij er wel en moet hij blijven
				StatisticsObserver_aangevraagd = FALSE;
				CmOptionsStatisticsObserver();
			} // if (StatisticsObserver_aangevraagd == TRUE)

			Observation_Complete_Message(output_sort);
		}
		else // if (Printer -> Print(GetApplication() -> GetMainWindow() -> GetClientWindow(), printout, TRUE))
		  MessageBox("Obs NOT printed and NOT stored.", "TurboWin warning", MB_OK | MB_ICONEXCLAMATION);

	} // if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
}



void TMyWindow::CeFilePrint(TCommandEnabler &tce)
{
	if (code_mode == "BUFR")
		tce.Enable(false);
	else
		tce.Enable();
}



void TMyWindow::CmFilePrintSetup()
{
	if (!Printer)
		Printer = new TPrinter;

	Printer -> Setup(GetApplication() -> GetMainWindow());
}



void TMyWindow::checking_level_1(BOOL &level_1_ok, bool statusbar_mode)
{
	// in eerste instantie gebeurt het checken op level_1 per invul blok
	// b.v. temperature, callsign etc.
	// wat daar echter niet kan is de check of de verplichte groepen ingevuld
	// zijn
	// verplichte groepen zijn:
	//              callsign
	//              datum tijd
	//              positie
   //
   // Deze function wordt ook gebruik voor displayen obs in de status bar (statusbar_mode = true)
   // dan moeten op callsign, datum/tijd en positie geen warnings gegeven worden
   //
	ifstream is;
	BOOL file_aanwezig_ok = FALSE;


	//
	// check of callsign aanwezig is(zit in de callsign_file)
	//
	Check_file_aanwezig(file_aanwezig_ok, callsign_file);
	if (file_aanwezig_ok)
	{
		is.open((turbowin_dir + callsign_file).c_str(), ios::in);
		obs_callsign.read_line(is);                  // read callsign from file CALLSIGN.TXT
		is.close();
		if (obs_callsign == "\0")
		{
         if (statusbar_mode != true)
			   MessageBox("Call sign missing", "TurboWin error", MB_OK | MB_ICONWARNING);
			level_1_ok = FALSE;
		} // if (obs_callsign == "\0")
	} // if (file_aanwezig_ok)
	else // callsign file is niet aanwezig
	   level_1_ok = FALSE;


	// controle dat datum tijd is ingevuld (AA MM DD GG)
	if ((level_1_ok == TRUE) && (num_AA == MAXINT || num_MM == MAXINT || num_DD == MAXINT || num_GG == MAXINT))
	{
      if (statusbar_mode != true)
		   MessageBox("Date and time missing", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_1_ok = FALSE;
	} // if (num_AA == MAXINT || num_MM == MAXINT || num_DD == MAXINT || num_GG == MAXINT)


	// controle dat positie is ingevoerd (99LaLaLa QcLoLoLoLo)
	if ((level_1_ok == TRUE) && (num_LaLaLa == MAXINT || num_LoLoLoLo == MAXINT))
	{
      if (statusbar_mode != true)
		   MessageBox("Position missing", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_1_ok = FALSE;
	} // if ((level_1_ok)) && (num_LaLaLa == MAXINT || num_LoLoLoLo == MAXINT))


	// eventueel breukstrepen toevoegen  bij de verplichte groepen
   //
   
	// ir ix h VV is een verplichte groep
	if ((strcmp(char_h, "\0") == 0))
		strcpy(char_h, "/");               // dit is / (1*)
	if ((strcmp(char_VV, "\0") == 0))
		strcpy(char_VV, "//");            // dit zijn / (2*)

	// Nddff is een verplichte groep
	if ((strcmp(char_N, "\0") == 0))
		strcpy(char_N, "/");               // dit is / (1*)
	if ((strcmp(char_dd, "\0") == 0))
		strcpy(char_dd, "//");            // dit is / (2*)
	if ((strcmp(char_ff, "\0") == 0))
		strcpy(char_ff, "//");            // dit is / (2*)

	// 7wwW1W2 is een verplichte groep
	if ((strcmp(char_ww, "\0") == 0))
		strcpy(char_ww, "//");
	if ((strcmp(char_W1, "\0") == 0))
		strcpy(char_W1, "/");
	if ((strcmp(char_W2, "\0") == 0))
		strcpy(char_W2, "/");

	// 8NhClCmCh is een verplichte groep
	if ((strcmp(char_Nh, "\0") == 0))
		strcpy(char_Nh, "/");
	if ((strcmp(char_Cl, "\0") == 0))
		strcpy(char_Cl, "/");
	if ((strcmp(char_Cm, "\0") == 0))
		strcpy(char_Cm, "/");
	if ((strcmp(char_Ch, "\0") == 0))
		strcpy(char_Ch, "/");

	// 222DsVs is een verplichte groep
	if ((strcmp(char_Ds, "\0") == 0))
		strcpy(char_Ds, "/");
	if ((strcmp(char_Vs, "\0") == 0))
		strcpy(char_Vs, "/");

	// 1PwaPwaHwaHwa is een verplichte groep
	if ((strcmp(char_Pwa, "\0") == 0))
		strcpy(char_Pwa, "//");
	if ((strcmp(char_Hwa, "\0") == 0))
		strcpy(char_Hwa, "//");

	// 2PwPwHwHw is een verplichte groep
	if ((strcmp(char_Pw, "\0") == 0))
		strcpy(char_Pw, "//");
	if ((strcmp(char_Hw, "\0") == 0))
		strcpy(char_Hw, "//");

	// 3dw1dw1dw2dw2 is een verplichte groep
	if ((strcmp(char_Dw1, "\0") == 0))
		strcpy(char_Dw1, "//");
	if ((strcmp(char_Dw2, "\0") == 0))
		strcpy(char_Dw2, "//");

	// 4Pw1Pw1Hw1Hw1 is een verplichte groep
	if ((strcmp(char_Pw1, "\0") == 0))
		strcpy(char_Pw1, "//");
	if ((strcmp(char_Hw1, "\0") == 0))
		strcpy(char_Hw1, "//");

	// 5Pw1Pw1Hw1Hw1 is een verplichte groep
	if ((strcmp(char_Pw2, "\0") == 0))
		strcpy(char_Pw2, "//");
	if ((strcmp(char_Hw2, "\0") == 0))
		strcpy(char_Hw2, "//");

	// 70HwaHwaHwa is een verplichte groep
	if ((strcmp(char_HwaHwaHwa, "\0") == 0))
		strcpy(char_HwaHwaHwa, "///");

}


void TMyWindow::checking_level_2(BOOL &level_2_ok)
{
	//
	// wind <--> golven checks
	//
	if ((num_ff == 0) && (num_Pw != 0) && (num_Pw != MAXINT))  // check 700
	{
      if (form_mode == CLASSIC)
         MessageBox("If ff = 00 then PwPw must be 00 or //", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
     		MessageBox("If 'wind speed' = 0 then 'wind waves period' must be 0 or blank", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	else if ((num_ff == 0) && (num_Hw != 0) && (num_Hw != MAXINT)) // check 701
	{
      if (form_mode == CLASSIC)
         MessageBox("If ff = 00 then HwHw must be 00 or //", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
   		MessageBox("If 'wind speed' = 0 then 'wind waves height' must be 0 or blank", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	// note: iw = 0 of iw = 1 = m/s
	else if ((num_ff >= 0) &&  (num_ff <= 3) && ((iw == 0) || (iw == 1)) &&
				(num_Hw >= 20) && (num_Hw != MAXINT))
	{
		// remark Hw in 0.5 metre units
      if (form_mode == CLASSIC)
         MessageBox("If iw = 0,1 and ff = 0 - 3 then HwHw must be < 20 or //", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
   		MessageBox("If 'wind speed' in range 0 - 3 m/s (0 - 6 kts) then 'wind waves height' must be < 9.8 metre or blank", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	// note: iw = 3 or iw = 4 = knots
	else if ((num_ff >= 0) &&  (num_ff <= 6) && ((iw == 3) || (iw == 4)) &&
				(num_Hw >= 20) && (num_Hw != MAXINT))
	{
		// remark Hw on 0.5 metre units
      if (form_mode == CLASSIC)
         MessageBox("If iw = 3,4 and ff = 0 - 6 then HwHw must be < 20 or //", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
   		MessageBox("If 'wind speed' in range 0 - 6 knots then 'wind waves height' must be < 9.8 metre or blank", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}

   // N.B. versie 2.1: 4 wind-waves checks veplaatst naar warning(confirm gedeelte)

	// N.B. voor golven <--> golven checks zie tmywaves.cpp

	//
	// a <--> ppp checks (omdat er geen lus zit om de a bij tmypress.cpp hierafhandelen
	//
	else if ((num_a == 4) && (num_ppp != 0))
	{
      if (form_mode == CLASSIC)
         MessageBox("If a = 4 then ppp must be 000", "TurboWin error", MB_OK);
      else
		   MessageBox("If 'airpressure characteristics' = 'steady' then 'amount of pressure tendency' must be 0", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}


	//
	// bewolkingsgraad <--> wolken soort(en) checks
	//
   else if ( (message_mode != "AUX") && ((num_N == 0) && ((num_Cl != 0) || (num_Cm != 0) || (num_Ch != 0))) )
   {
      if (form_mode == CLASSIC)
         MessageBox("If N = 0 then Cl and Cm and Ch must be 0", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
         MessageBox("If 'total cloud cover' = 'cloudless' then 'Cl (Clouds low)' AND 'Cm'(Clouds middle) AND 'Ch'(Clouds high) must be 'no clouds'", "TurboWin error", MB_OK | MB_ICONWARNING);
      level_2_ok = FALSE;
   }
   else if ( (message_mode != "AUX") && ((num_Cl == 0) && (num_Cm == 0) && (num_Ch == 0) &&	(num_N != 0)) )
   {
      if (form_mode == CLASSIC)
         MessageBox("If Cl = 0 and Cm = 0 and Ch = 0 then N must be 0", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
         MessageBox("If 'Cl (Clouds low)' AND 'Cm (Clouds middle)' AND 'Ch (Clouds high)' = 'no clouds' then 'total cloud cover' must be 'cloudless'", "TurboWin error", MB_OK | MB_ICONWARNING);
      level_2_ok = FALSE;
   }
   else if ( (message_mode != "AUX") && ((num_Nh == 0) && ((num_Cl != 0) )/*versie 3.0 && (num_Cl != MAXINT)*/))
   {
      if (form_mode == CLASSIC)
         MessageBox("If Nh = 0 then Cl must be 0", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
         MessageBox("If 'amount of Cl (or Cm if Cl is not present)' = 0/8 then 'Cl' must be 'no clouds Cl'", "TurboWin error", MB_OK | MB_ICONWARNING);
      level_2_ok = FALSE;
   }
   else if ( (message_mode != "AUX") && ((num_Nh == 0) && (num_Cl == 0) && ((num_Cm != 0) )/* versie 3.0 && (num_Cm != MAXINT)*/))
   {
      if (form_mode == CLASSIC)
         MessageBox("If Nh = 0 and Cl = 0 then Cm must be 0", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
         MessageBox("If 'amount of Cl (or Cm if Cl is not present)' = 0/8 AND Cl = 'no clouds Cl' then 'Cm (Clouds middle)' must be 'no clouds Cm'", "TurboWin error", MB_OK | MB_ICONWARNING);
      level_2_ok = FALSE;
   }
   else if ( (message_mode != "AUX") && ((num_Cl == 0) && (num_Cm == 0) && (num_Nh != 0)) )
   {
      if (form_mode == CLASSIC)
         MessageBox("If Cl = 0 and Cm = 0 then Nh must be 0", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
         MessageBox("If 'Cl (Clouds low)' = 'no clouds Cl' AND 'Cm (Clouds middle)' = 'no clouds Cm' then 'amount of Cl (or Cm if Cl is not present)' must be 0/8", "TurboWin error", MB_OK | MB_ICONWARNING);
      level_2_ok = FALSE;
   }
   else if ( (message_mode != "AUX") && ((num_Nh == 8) && (num_Ch >= 0) && (num_Ch <= 9)) )
   {
      if (form_mode == CLASSIC)
         MessageBox("If Nh = 8 then Ch must be /", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
         MessageBox("If 'amount of Cl (or Cm if Cl is not present)' = 8/8 then 'Ch (Clouds high)' must be 'Ch not determined'", "TurboWin error", MB_OK | MB_ICONWARNING);
      level_2_ok = FALSE;
   }
   else if ( (message_mode != "AUX") && ((num_Nh == 8) && ((num_Cl >= 1) && (num_Cl <= 9)) && ((num_Cm >= 0) && (num_Cm <= 9))) )
   {
      if (form_mode == CLASSIC)
         MessageBox("If Nh = 8 and Cl in range 1 - 9 then Cm must be /", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
         MessageBox("If 'amount of Cl (or Cm if Cl is not present)' = 8/8 AND 'Cl (Clouds low)' is in range 1 - 9 then 'Cm (Clouds medium)' must be 'Cm not determined'", "TurboWin error", MB_OK | MB_ICONWARNING);
      level_2_ok = FALSE;
   }

	//
	// N <--> ww
	//
	else if (((num_ww == 43) || (num_ww == 45) || (num_ww == 47) || (num_ww == 49)) &&
				 ((num_N != MAXINT) && (num_N != 9)))
	{
		// N.B. N = 9 = obscured
      if (form_mode == CLASSIC)
         MessageBox("If ww = 43,45,37,49 then N must be 9 or /", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'present weather' = 'fog with sky not discernible' then 'total cloud cover' must be 'obscured' or 'not determined'", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	else if (((num_ww == 43) || (num_ww == 45) || (num_ww == 47) || (num_ww == 49)) &&
				 ((num_Cl != MAXINT) || (num_Cm != MAXINT) || (num_Ch != MAXINT)))
	{
      if (form_mode == CLASSIC)
		   MessageBox("If ww = 43,45,47,49 then Cl and Cm and Ch must be /", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
     		MessageBox("If 'present weather' = 'fog with sky not discernible' then Cl (Clouds low) and Cm (Clouds middle) and Ch (Clouds high) must be 'not determined'", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	else if ((num_N == 0) && (num_ww >= 50) && (num_ww <= 69))
	{
      if (form_mode == CLASSIC)
		   MessageBox("If N = 0 then ww cannot be 50 - 69", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
     		MessageBox("If 'total cloud cover' = 0 then 'present weather' can not indicate drizzle or rain at time of observation", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}

	//
	// Cl <--> h
	//
	else if ( (message_mode != "AUX") && ((num_Cl >= 1) && (num_Cl <= 9) && (num_h == 9)) )
	{
      if (form_mode == CLASSIC)
         MessageBox("If Cl = 1 - 9 then h cannot be 9", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'Cl (Clouds low)' = 1 - 9 then 'height of base of lowest cloud in the sky' can not be '>= 2500 m (>= 8000 ft) or cloudless'", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
   }

	//
	// Ch <--> h
	//
	else if ( (message_mode != "AUX") && ((num_Cl == 0) && (num_Cm == 0) && ((num_h == 0) || (num_h == 1) || (num_h == 2)) &&
				 ((num_Ch >= 1) && (num_Ch <= 9))) )
	{
      if (form_mode == CLASSIC)
  		   MessageBox("If Cl = 0 and Cm = 0 and Ch = 1 - 9 then h cannot be 0 - 2", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
         MessageBox("If 'Cl (Clouds low)' = 0 AND 'Cm (Clouds middle)' = 0 AND 'Ch (Clouds high) = 1 - 9' then 'height of base of lowest cloud in the sky' cannot be < 200 m (< 600 ft)", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}

	//
	// VV <--> ww
	//
	else if ((num_ww >= 42) && (num_ww <= 49) && (num_VV >= 95) && (num_VV <= 99))
	{
      if (form_mode == CLASSIC)
         MessageBox("If ww in range 42 - 49 then VV cannot be 95 - 99", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'present weather' indicates 'fog' then 'visibility' can not be > 0.5 nm (an exception is made for fogbanks, fog in patches and shallow fog)", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	// n.b. ww = 41 dan kan VV < 1000 meter zijn maar ook > 1000 meter zie 7wwW1W2 document (P.Y. de Vries)
	else if ((num_ww == 40) && (num_VV >= 90) && (num_VV <= 93))
	{
      if (form_mode == CLASSIC)
  		   MessageBox("If ww = 40 then VV cannot be in range 90 - 93", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'present weather' indicates '"ww_40"' then 'visibility' can not be < 0.5 nm", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}

	//
	//  ww <--> TTT
	//
	// n.b. num_TTT in tienden (ook negatief)
	// n.b. char_TTT is alleen positief
	// n.b. char_Sn_TTT is 0 (>= 0 graden Celsius) of 1 (< 0 graden Celsius)
	else if ((num_TTT > 200) && (num_TTT != MAXINT) && (num_ww >= 36) && (num_ww <= 39))
	{
      if (form_mode == CLASSIC)
         MessageBox("If TTT > 200 (and sn = 0) then ww cannot be 36 - 39", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'air temperature' > 20.0 °C then 'present weather' can not indicate 'drifting snow' or 'blowing snow'", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	else if ((num_TTT > 200) && (num_TTT != MAXINT) && (num_ww >= 48) && (num_ww <= 49))
	{
      if (form_mode == CLASSIC)
		   MessageBox("If TTT > 200 (and sn = 0) then ww cannot be 48 - 49", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'air temperature' > 20.0 °C then 'present weather' can not indicate 'depositing rime'", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	else if ((num_TTT > 200) && (num_TTT != MAXINT) && (num_ww >= 56) && (num_ww <= 57))
	{
      if (form_mode == CLASSIC)
         MessageBox("If TTT > 200 (and sn = 0) then ww cannot be 56 - 57", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'air temperature' > 20.0 °C then 'present weather' can not indicate 'freezing drizzle'", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	else if ((num_TTT > 200) && (num_TTT != MAXINT) && (num_ww >= 66) && (num_ww <= 67))
	{
      if (form_mode == CLASSIC)
         MessageBox("If TTT > 200 (and sn = 0) then ww cannot be 66 - 67", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'air temperature' > 20.0 °C then 'present weather' can not indicate 'freezing rain'", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	else if ((num_TTT > 200) && (num_TTT != MAXINT) && (num_ww >= 68) && (num_ww <= 69))
	{
      if (form_mode == CLASSIC)
         MessageBox("If TTT > 200 (and sn = 0) then ww cannot be 68 - 69", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'air temperature' > 20.0 °C then 'present weather' can not indicate 'snow'", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	else if ((num_TTT > 200) && (num_TTT != MAXINT) && (num_ww >= 70) && (num_ww <= 75))
	{
      if (form_mode == CLASSIC)
         MessageBox("If TTT > 200 (and sn = 0) then ww cannot be 70 - 75", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'air temperature' > 20.0 °C then 'present weather' can not indicate 'snow flakes'", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	else if ((num_TTT > 200) && (num_TTT != MAXINT) && (num_ww >= 83) && (num_ww <= 86))
	{
      if (form_mode == CLASSIC)
         MessageBox("If TTT > 200 (and sn = 0) then ww cannot be 83 - 86", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'air temperature' > 20.0 °C then 'present weather' can not indicate 'snow shower(s)'", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
	else if ((num_TTT > 200) && (num_TTT != MAXINT) && (num_ww >= 76) && (num_ww <= 79))
	{
      if (form_mode == CLASSIC)
         MessageBox("If TTT > 200 (and sn = 0) then ww cannot be 76 - 79", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'air temperature' > 20.0 °C then 'present weather' can not indicate 'snow grains/crystals' or 'ice prisms/pellets'", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}


	//
	// icing <--> TTT
	//
	else if ((num_TTT > 250) && (num_TTT != MAXINT) && ((num_Is != MAXINT) || (num_EsEs != MAXINT) || (num_Rs != MAXINT)))
	{
      if (form_mode == CLASSIC)
  		   MessageBox("If TTT > 250 (and sn = 0) then IsEsEsRs must be ////", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'air temperature' > 25.0 °C then 'icing (ice accretion)' is not possible", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}

	//
	// ice <--> TTT
	//
	else if ((num_TTT > 250) && (num_TTT != MAXINT) && ((num_ci != MAXINT) || (num_Si != MAXINT) ||
				 (num_bi != MAXINT) ||(num_Di != MAXINT) || (num_zi != MAXINT)))
	{
      if (form_mode == CLASSIC)
  		   MessageBox("If > 250 (and sn = 0) then ice group ciSibiDizi must be ///// and ", "TurboWin error", MB_OK | MB_ICONWARNING);
      else
		   MessageBox("If 'air temperature' > 25.0 °C then 'ice' is not possible", "TurboWin error", MB_OK | MB_ICONWARNING);
		level_2_ok = FALSE;
	}
}


void TMyWindow::checking_level_3(BOOL &level_3_ok)
{
	string info;
   bool gebruik_default_temp = true;
   bool doorgaan             = true;
   int num_vorige_obs_jaar;                               // voor check met laatst opgeslagen obs in immt
   int num_vorige_obs_maand;                              // voor check met laatst opgeslagen obs in immt
   int num_vorige_obs_dag;                                // voor check met laatst opgeslagen obs in immt
   int num_vorige_obs_uur;                                // voor check met laatst opgeslagen obs in immt
   double num_vorige_obs_breedte;                         // voor check met laatst opgeslagen obs in immt
   double num_vorige_obs_lengte;                          // voor check met laatst opgeslagen obs in immt
   long huidige_obs_sec_sinds_1_01_1901;                  // voor check met laatst opgeslagen obs in immt
   long vorige_obs_sec_sinds_1_01_1901;                   // voor check met laatst opgeslagen obs in immt
   int obs_verschil_uur;                                  // voor check met laatst opgeslagen obs in immt
   int afstand_vorige_huidige_obs;                        // voor check met laatst opgeslagen obs in immt


   // observatie tijd controleren (obs niet te laat ?).
   end_seconds_1970 = time(NULL);                          // Aantal sec sinds 1 Jan 1970 (voor controle op te laat wegsturen obs)
   //if (end_seconds_1970 - start_seconds_1970 > 3600L)      // >= 0.5 uur sinds laatste update date-time

   /* >= 0.5 uur sinds laatste update date-time + obs_Month etc. waren nog niet gereset (zie hiervoor) */
   /* function EvTimer() in file TmyWindow.cpp */
   if ( (end_seconds_1970 - start_seconds_1970 > 1800L) &&
        (obs_Month != "\0") && (obs_Day != "\0") && (obs_Year != "\0") && (obs_Time != "\0") )
   {
      info = "\0";
      info += "Date and time observation:\n";
      info += obs_Month;
	   info += " ";
	   info += obs_Day;
	   info += ", ";
	   info += obs_Year;
	   info += " ";
	   info += obs_Time;
	   info += ".00 UTC\n";

		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please update the date and time of observation (no final obs was coded)", "TurboWin message", MB_OK | MB_ICONINFORMATION);
         doorgaan = false;
		}
   } // if (end_seconds_1970 - start_seconds_1970 > 3600L)

   // positie controle d.m.v. land-sea mask (NIET bij start_option = 1, dan wordt deze check onderdrukt (bv voor ferry's))
   if (start_option != 1)
   {
      if (doorgaan && (Check_Land_Sea_Mask() == false))             // geen sea position
      {
		   info = "\0";
         info = "-land mask check-\n";
		   info += "observation position:\n";

         if (form_mode == CLASSIC)
         {
            info += "(99LaLaLa QcLoLoLoLo)\n\n";
            info += "99";
            info += string(char_LaLaLa);
            info += " ";
            info += string(char_Qc);
            info += string(char_LoLoLoLo);
         } // if (form_mode == CLASSIC)
         else // no classic_form
         {
            info += obs_LatitudeDegrees;
            info += "° ";
            info += obs_LatitudeTenths;
            info += "' ";
            info += obs_North_or_South;
            info += "  ";
            info += obs_LongitudeDegrees;
            info += "° ";
            info += obs_LongitudeTenths;
            info += "' ";
            info += obs_East_or_West;
		      info += "\n";
         } // else (no classic form)

		   if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		   {
			   level_3_ok = FALSE;
			   MessageBox("Please correct the position of observation (no final obs was coded)", "TurboWin message", MB_OK);
            doorgaan = false;
		   }
      } // if (Check_Land_Sea_Mask() == false)
   } // if (start_option != 1)
   else // dus start_option = 1 (dan wordt de sea-land mask check niet uitgevoerd)
      doorgaan = true;


	if (doorgaan && (num_Vs >= 8) && (num_Vs <= 9))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "vs > 7. ";
      else
		   info = "Ship's speed > 35 knots. ";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this average speed is ok";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if ((num_Vs >= 8) && (num_Vs <= 9))

	// note: iw = 0 or iw = 1 = m/s
	if (doorgaan && (num_ff > 28) && (num_ff != MAXINT) && ((iw == 0) || (iw == 1)))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "ff > 24 (iw = 0 or 1) ";
      else
		   info = "Wind speed > 28 m/s (55 kts). ";
		//info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this wind speed was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if ((num_ff > 24) && (num_ff != MAXINT) && ((iw == 0) || (iw == 1))

	// note; iw = 3 or iw = 4 = knots
	if (doorgaan && (num_ff > 55) && (num_ff != MAXINT) && ((iw == 3) || (iw == 4)))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "ff > 55 (iw = 3 or 4). ";
      else
		   info = "Wind speed > 55 knots. ";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this wind speed was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if ((num_ff > 48) && (num_ff != MAXINT) && ((iw == 3) || (iw == 4))


	if (doorgaan && (num_Pwa > 25) && (num_Pwa != MAXINT) && (num_Pwa != 99))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "PwaPwa > 25 (not 99). ";
      else
		   info = "Wave period > 25 sec. ";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this wave period was really measured";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if  ((num_Pwa > 25) && (num_Pwa != MAXINT))
	if (doorgaan && (num_Hwa > 24) && (num_Hwa != MAXINT))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "HwaHwa > 24. ";
      else
		   info = "Wave height > 12.2 metres. ";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this wave height was really measured";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if  ((num_Hwa > 12.2) && (num_Hwa != MAXINT))


	if (doorgaan && (num_Pw > 25) && (num_Pw != MAXINT) && (num_Pw != 99))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "PwPw > 25 (not 99). ";
      else
		   info = "Wind wave period > 25 sec. ";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this wind waves period was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if  ((num_Pw > 25) && (num_Pw != MAXINT))
	if (doorgaan && (num_Hw > 24) && (num_Hw != MAXINT))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "HwHw > 24. ";
      else
		   info = "Wind wave height > 12.2 metres. ";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this wind waves height was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if  ((num_Hw > 12.2) && (num_Hw != MAXINT))
	if (doorgaan && (num_Pw1 > 25) && (num_Pw1 != MAXINT))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "Pw1Pw1 > 25. ";
      else
		   info = "First swell period > 25 sec. ";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this swell waves period was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if  ((num_Pw1 > 25) && (num_Pw1 != MAXINT))
	if (doorgaan && (num_Hw1 > 24) && (num_Hw1 != MAXINT))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "Hw1Hw1 > 24. ";
      else
		   info = "First swell height > 12.2 metres. ";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this swell waves height was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if  ((num_Hw1 > 12.2) && (num_Hw1 != MAXINT))
	if (doorgaan && (num_Pw2 > 25) && (num_Pw2 != MAXINT))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "Pw2Pw2 > 25.";
      else
		   info = "Second swell period > 25 sec.";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this swell waves period was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if  ((num_Pw2 > 25) && (num_Pw2 != MAXINT))
	if (doorgaan && (num_Hw2 > 24) && (num_Hw2 != MAXINT))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "Hw2Hw2 > 24. ";
      else
		   info = "Second swell height > 12.2 metres ";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this swell waves height was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if  ((num_Hw2 > 12.2) && (num_Hw2 != MAXINT))

	// n.b. num_TTT in tienden (num_TTT kan ook negatief zijn)
	if (doorgaan && (num_TTT > 500) && (num_TTT != MAXINT))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "TTT > 500 (sn = 0)";
      else // standard form
      {
         if (message_mode == "AUX")
            info = "Air temperature > 50 °C. ";
         else
		      info = "Air temperature > 50.0 °C. ";
      } // else (standard form)
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this temperature was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if  ((num_TTT > 500) etc.
	if (doorgaan && num_TTT < -200)
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "TTT > 200 (sn = 1)";
      else // standard form
      {
         if (message_mode == "AUX")
            info = "Air temperature < -20 °C. ";
         else
		      info = "Air temperature < -20.0 °C. ";
      } // else (standard form)
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this temperature was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if  (num_TTT < -200)

	if (doorgaan && (num_TTT > 40) && (num_TTT != MAXINT) && ((num_Is != MAXINT) || (num_EsEs != MAXINT) || (num_Rs != MAXINT)))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "TTT > 040 (sn = 0) and IsEsEsRs not ////. ";
      else
		   info = "Air temperature > 4.0 °C and ICING. ";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this combination was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // 	if ((num_TTT > 40) && (num_TTT != MAXINT) && ((num_Is != MAXINT) || (num_EsEs != MAXINT) || (num_Rs != MAXINT)))

	if (doorgaan && (num_MSL_PPPP > 500) && (num_MSL_PPPP < 9100))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "PPPP < 9100 or > 0500. ";
      else
		   info = "Pressure (MSL) < 910.0 hPa or > 1050.0 hPa. ";
		info += "Press the NO button if it was a typing error, press the YES button if this pressure was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if ((num_PPPP > 500) && (num_PPPP < 9100))

	if (doorgaan && (num_ppp > 300) && (num_ppp != MAXINT))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "ppp > 300. ";
      else
		   info = "Amount of pressure tendency during the last 3 hrs. > 30.0 hPa. ";
		info += "Press the NO button if it was a typing error, press the YES button if this pressure tendency was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if ((num_ppp > 300) && (num_ppp != MAXINT))

	if (doorgaan && (num_ppp != MAXINT) && (num_a == MAXINT))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "ppp = present and a = not present. ";
      else
		   info = "Amount of pressure tendency available and characteristics of tendency not available. ";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if the characteristic was really not available";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if ((num_a == MAXINT) && (num_ppp != MAXINT))

	if (doorgaan && (num_TwTwTw > 350) && (num_TwTwTw != MAXINT))
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "TwTwTw > 350. ";              // komt hier alleen in als (ss = 0,2,4,6)*/
      else
		   info = "Seawater temperature > 35.0 °C.";
		info += "\n";
		info += "Press the NO button if it was a typing error, press the YES button if this seawater temp. was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if  ((num_TwTwTw > 350) etc.

	if (doorgaan && (num_EsEs > 20) && (num_EsEs != MAXINT))
	{
		info = "\0";
      if (form_mode == CLASSIC)
   		info = "EsEs > 20. ";
      else
         info = "Ice accretion > 20.0 centimetres. ";
		info += "Press the NO button if it was a typing error, press the YES button if this ice accretion was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if  ((num_EsEs > 20) etc.

	if ( doorgaan && ((num_ww == 48) || (num_ww == 49)) &&
		              ((num_Is == MAXINT) && (num_EsEs == MAXINT) && (num_Rs == MAXINT)) )
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "ww = 48 or 49 and IsEsEsRs = //// (or blank). ";
      else
		   info = "Fog, depositing rime (present weather) and no ICING. ";
		info += "Press the NO button if it was a typing error, press the YES button if ICING is really not present";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if ( ((num_ww == 48 || (num_ww == 49)) &&

	if ( doorgaan && ((num_ww == 56) || (num_ww == 57)) &&
		              ((num_Is == MAXINT) && (num_EsEs == MAXINT) && (num_Rs == MAXINT)) )
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "ww = 56 or 57 and IsEsEsRs = //// (or blank). ";
      else
		   info = "Freezing drizzle (present weather) and no ICING. ";
		info += "Press the NO button if it was a typing error, press the YES button if ICING is really not present";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if ( ((num_ww == 56 || (num_ww == 57)) &&

	if ( doorgaan && ((num_ww == 66) || (num_ww == 67)) &&
		              ((num_Is == MAXINT) && (num_EsEs == MAXINT) && (num_Rs == MAXINT)) )
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "ww = 66 or 67 and IsEsEsRs = //// (or blank). ";
      else
		   info = "Freezing rain (present weather) and no ICING. ";
		info += "Press the NO button if it was a typing error, press the YES button if ICING is really not present";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if ( ((num_ww == 66 || (num_ww == 67)) && etc.

   /* versie 2.1 */
   // note: iw = 0 or iw = 1 = m/s
	if ( doorgaan && (num_ff > 5) && (num_ff != MAXINT) && ((iw == 0) || (iw == 1)) && (num_Hw == 0) )
	{
		info = "\0";
      if (form_mode == CLASSIC)
         info = "ff > 5 (iw = 0 or 1) and HwHw = 00. \n";
      else
		   info = "Wind speed > 5 m/s (10 kts) and wind waves height 0 metre. \n";
		info += "Press the NO button if it was a typing error, press the YES button if this combination was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
	} // if (doorgaan && (num_ff > 5) etc.

   /* versie 2.1 */
	// note: iw = 3 or iw = 4 = knots
	if ( doorgaan && (num_ff > 10) && (num_ff != MAXINT) && ((iw == 3) || (iw == 4)) && (num_Hw == 0) )
	{
		info = "\0";
      if (form_mode == CLASSIC)
   		info = "ff > 10 (iw = 3 or 4) and HwHw = 00. \n";
      else
         info = "Wind speed > 10 knots and wind waves height 0 metre. \n";
		info += "Press the NO button if it was a typing error, press the YES button if this combination was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
		}
   } // if ( doorgaan && (num_ff > 10)

   /* versie 2.1 */
	// note: iw = 0 or iw = 1 = m/s
	if ( doorgaan && (num_ff > 21) && (num_ff != MAXINT) && ((iw == 0) || (iw == 1)) && (num_Hw >= 0) && (num_Hw <= 4) )
	{
		// remark Hw in 0.5 metre units
		info = "\0";
      if (form_mode == CLASSIC)
         info = "ff > 21 (iw = 0 or 1) and HwHw < 05. \n";
      else
		   info = "Wind speed > 21 m/s (41 kts) and wind waves height <= 2.2 metres. \n";
		info += "Press the NO button if it was a typing error, press the YES button if this combination was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
      }
   } // if ( doorgaan && (num_ff > 21) etc.

   /* versie 2.1 */
	// note: iw = 3 or iw = 4 = knots
	if ( doorgaan && (num_ff > 41) && (num_ff != MAXINT) && ((iw == 3) || (iw == 4)) &&	(num_Hw >= 0) && (num_Hw <= 4) )
	{
		// remark Hw in 0.5 metre units
		info = "\0";
      if (form_mode == CLASSIC)
         info = "ff > 41 (iw = 3 or 4) and HwHw < 05. \n";
      else
		   info = "Wind speed > 41 knots and wind waves height <= 2.2 metres. \n";
		info += "Press the NO button if it was a typing error, press the YES button if this combination was really observed";
		if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			level_3_ok = FALSE;
			MessageBox("Please correct the typing error (no final obs was coded)", "TurboWin message", MB_OK);
         doorgaan = false;
      }
   } // if ( doorgaan && (num_ff > 41) etc.



   /* datum/tijd t.o.v. vorige waarneming controleren */
   /* NB TTime en TDate niet binnen een if statement want dan niet bekend buiten dat if statement */

   /* laatste opgeslagen record in IMMT ophalen en datum/tijd en positie delen er uit halen en doorgeven */
   Read_IMMT_Log_For_Afstand_En_DatumTijd_Check(num_vorige_obs_jaar, num_vorige_obs_maand, num_vorige_obs_dag, num_vorige_obs_uur,
                                                num_vorige_obs_breedte, num_vorige_obs_lengte);

   /* datum-tijd vorige obs */
   TDate datum_vorige_obs(num_vorige_obs_dag, num_vorige_obs_maand, num_vorige_obs_jaar);
   TTime time_vorige_obs(datum_vorige_obs, num_vorige_obs_uur);

   /* datum-tijd huidige obs */
   TDate datum_huidige_obs(num_DD, num_MM, 2000 + num_AA);   // num_DD = dag; num_MM = maand; num_AA = jaar zonder 20 er voor
   TTime time_huidige_obs(datum_huidige_obs, num_GG);

   if (doorgaan)
   {
      /* huidige obs datum/tijd moet later zijn dan vorige obs datum/tijd */
      if (time_huidige_obs <= time_vorige_obs)
      {
		   info = "\0";
		   info = "obs date/time";
         info += " (";
         info += datum_huidige_obs.AsString();
         info += " ";
         info += string(char_GG);
         info += ".00 UTC";
         info += ")";
		   if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		   {
			   level_3_ok = FALSE;
			   MessageBox("Please correct the error (no final obs was coded)", "TurboWin message", MB_OK);
            doorgaan = false;
         }
      } // if (time_huidige_obs <= time_vorige_obs)
   } // if (doorgaan)

   /* obs positie t.o.v. vorige waarneming controleren */
   if (doorgaan)
   {
      /* eerst tijdverschil huidige en vorige obs bepalen */
      huidige_obs_sec_sinds_1_01_1901 = time_huidige_obs.Seconds();
      vorige_obs_sec_sinds_1_01_1901  = time_vorige_obs.Seconds();
      obs_verschil_uur = (huidige_obs_sec_sinds_1_01_1901 - vorige_obs_sec_sinds_1_01_1901) / 3600;

      //char buffer[10];
      //sprintf(buffer, "%d", obs_verschil_uur);
      //MessageBox(buffer, "test obs verschil uur", MB_OK);
      if (obs_verschil_uur >= 0)                   // alleen verdere checks als huidige obs datum/tijd is later dan vorige obs datum/tijd
      {
         /* NB er wordt als grens genomen dat er max 50 mijl per uur afgelegd kan zijn */
         afstand_vorige_huidige_obs = Bepaal_afstand_huidige_obs_pos_tot_vorige_obs_pos(num_vorige_obs_breedte, num_vorige_obs_lengte);

         //char buffer[10];
         //sprintf(buffer, "%d", afstand_vorige_huidige_obs);
         //MessageBox(buffer, "test afstand_vorige_huidige_obs", MB_OK);

         if ( ((obs_verschil_uur >= 0 && obs_verschil_uur <= 6)  && (afstand_vorige_huidige_obs >  300)) ||
              ((obs_verschil_uur >  6 && obs_verschil_uur <= 12) && (afstand_vorige_huidige_obs >  600)) ||
              ((obs_verschil_uur > 12 && obs_verschil_uur <= 18) && (afstand_vorige_huidige_obs >  900)) ||
              ((obs_verschil_uur > 18 && obs_verschil_uur <= 24) && (afstand_vorige_huidige_obs > 1200)) )
         {
      		info = "\0";
		      info = "-position sequence check-\n";
            info += "obs position:\n";

            if (form_mode == CLASSIC)
            {
              info += "(99LaLaLa QcLoLoLoLo)\n\n";
              info += "99";
              info += string(char_LaLaLa);
              info += " ";
              info += string(char_Qc);
              info += string(char_LoLoLoLo);
            } // if (form_mode == CLASSIC)
            else  // no classic_form
            {
               info += obs_LatitudeDegrees;
               info += "° ";
               info += obs_LatitudeTenths;
               info += "' ";
               info += obs_North_or_South;
               info += "  ";
               info += obs_LongitudeDegrees;
               info += "° ";
               info += obs_LongitudeTenths;
               info += "' ";
               info += obs_East_or_West;
		         info += "\n";
            } // else (no classic form)

		      if (MessageBox(info.c_str(), "TurboWin, please confirm", MB_YESNO | MB_ICONQUESTION) == IDNO)
		      {
			      level_3_ok = FALSE;
			      MessageBox("Please correct the obs position (no final obs was coded)", "TurboWin message", MB_OK);
               doorgaan = false;
		      }
         } // if ( ((obs_verschil_uur > 0 && obs_verschil <= 6) etc.
      } // if (obs_verschil_uur > 0)
   } // if (doorgaan)



   /* dit is eigenlijk geen check maar een MSL berkekening die niet eerder kon gebeuren          */
   /* omdat er geen dry bulb temp ingevuld is en wel een NIET naar sea level herleide luchtdruk  */
   if (doorgaan && (barometer_MSL_yes_no.compare("no") == 0) && (num_TTT == MAXINT) &&
                   (num_PPPP != MAXINT) && (num_MSL_PPPP == MAXINT))
   {
      MSL_Herleiding_Luchtdruk(gebruik_default_temp);         // hier wordt WEL een default air temp gebruikt

      /* status bar updaten */
      Update_StatusBar_Obs();

	   if (MeteoProgressDialog -> MeteoProgressDialog_Created == TRUE)
	      MeteoProgressDialog -> UpdateAirpressureMeteoProgressDialog
		  										 (obs_Pressure, obs_Pressuretienden,
                                      obs_MSL_Pressure, obs_MSL_Pressuretienden,
			 									  obs_Amounttendency, obs_Amounttiendentendency,
			  									  obs_Characteristics);
   } // if ((barometer_MSL_yes_no.compare("no") == 0) etc.


   /* geen waarnemer geselecteerd dan waarschuwing (afgesproken met Met Office UK)   */
   /* N.B. dit moet de laatste check zijn in deze function i.v.m. recursieve aanroep */
   //
   if (doorgaan && (obs_observername == "\0"))
   {
		info = "\0";
		info = "No observer selected";
      info += "\n";
      info += "(Press the \"Retry\" button to select an observer even now)";
		switch (MessageBox(info.c_str(), "TurboWin message", MB_ABORTRETRYIGNORE  | MB_ICONINFORMATION))
      {
         // n.b. voor MB_ABORTRETRYIGNORE etc. zie bld 71 van Programming OWL for Windows 95 van Vic Broquard
         case IDABORT  : level_3_ok = FALSE;
                         break;
         case IDRETRY  : CmShipObserver();
                         //checking_level_3(level_3_ok); // vanaf versie 3.0
                         break;
         case IDIGNORE : break;                        // dus level_3_ok blijft op true staan
         default       : break;
      } // switch
   } // if (obs_observername == "\0")

}



void TMyWindow::samenstellen_te_verzenden_obs()
{
   bool national_groups_present;

   /* initialisatie */
	char_obs_1[0] = '\0';
	char_obs_2[0] = '\0';
   char_obs_3[0] = '\0';
	char_obs[0]   = '\0';

	//
	// code
	//
	// bbxx callsign YYGGiw irixhVV Nddff (00fff) 1SnTTT 2SnTdTdTd 4PPPP 5apppp
	// 7wwW1W2 8NhClCmCh 222DsVs 0SsTwTwTw 2PwPwHwHw 3dw1dw1dw2dw2 4Pw1Pw1Hw1Hw1
	// 5Pw2Pw2Hw2Hw2 6IsEsEsRs (8swTbTbTb) (ICE ciSibiDizi) 555 ????? ?????


	// bbxx (verplichte groep)
	strcpy(char_obs_1, "BBXX");

	// callsign (verplichte groep)
	strcat(char_obs_1, " ");
	strcat(char_obs_1, obs_callsign.c_str());

	// YYGGiw (verplichte groep)
	strcat(char_obs_1, " ");
	strcat(char_obs_1, char_DD);
	strcat(char_obs_1, char_GG);
	//if ((iw == MAXINT) && (num_dd == MAXINT && num_ff == MAXINT))
	if (iw == MAXINT)
		strcat(char_obs_1, "0");     // volgens de code moet er wat staan bij iw (dus daarom een willekeurige waarde genomen)
	else
		strcat(char_obs_1, char_iw);

	// breedte (99LaLaLa) (verplichte groep)
	strcat(char_obs_1, " ");
	strcat(char_obs_1, "99");
	strcat(char_obs_1, char_LaLaLa);

	// Aardquadrant + lengte (QcLoLoLoLo) toevoegen (verplichte groep)
	strcat(char_obs_1, " ");
	strcat(char_obs_1, char_Qc);
	strcat(char_obs_1, char_LoLoLoLo);

	// irixhVV toevoegen (verplichte groep)
	strcat(char_obs_1, " ");
	strcat(char_obs_1, "4");                // ir
	strcat(char_obs_1, "1");                // ix
	strcat(char_obs_1, char_h);             // h
	strcat(char_obs_1, char_VV);            // VV

	// Nddff toevoegen (verplichte groep)
	strcat(char_obs_1, " ");
	strcat(char_obs_1, char_N);              // N
	strcat(char_obs_1, char_dd);             // dd
	strcat(char_obs_1, char_ff);             // ff

	// 00fff toevoegen indien van toepassing
	if ((num_ff > 99) && (num_ff != MAXINT))
	{
		strcat(char_obs_1, " ");
		strcat(char_obs_1, "00");
		strcat(char_obs_1, char_fff);
	} // if (num_ff > 99)

	// Tair toevoegen (1SnTTT) indien van toepassing
	if (num_TTT != MAXINT)
	{
		strcat(char_obs_1, " ");
		strcat(char_obs_1, "1");
		strcat(char_obs_1, char_Sn_TTT);
		strcat(char_obs_1, char_TTT);
	} // if (num_TTT != MAXINT)

	// 2SnTdTdTd toevoegen indien van toepassing
	if (num_TdTdTd != MAXINT)
	{
		strcat(char_obs_1, " ");
		strcat(char_obs_1, "2");
		strcat(char_obs_1, char_Sn_TdTdTd);
		strcat(char_obs_1, char_TdTdTd);
	} // if (num_TdTdTd != MAXINT)

	// air-pressure (4PPPP) toevoegen indien van toepassing
	if (num_MSL_PPPP != MAXINT)
	{
		strcat(char_obs_1, " ");
		strcat(char_obs_1, "4");
		strcat(char_obs_1, char_MSL_PPPP);
	} // if (num_MSL_PPPP != MAXINT)

	// air pressure characteristic and amount of tendency (5appp) toevoegen indien van toepassing
	if ((num_a != MAXINT) || (num_ppp != MAXINT))
	{
		strcat(char_obs_1, " ");
		strcat(char_obs_1, "5");
		if (num_a != MAXINT)
			strcat(char_obs_1, char_a);
		else
			strcat(char_obs_1, "/");
		if (num_ppp != MAXINT)
			strcat(char_obs_1, char_ppp);
		else
			strcat(char_obs_1, "///");                  // dit is / (3*)
	} // if ((num_a != MAXINT) || (num_ppp != MAXINT))

	// 7wwW1W2 toevoegen (verplichte groep)
	//strcat(char_obs_2, " ");
	strcat(char_obs_2, "7");
	strcat(char_obs_2, char_ww);
	strcat(char_obs_2, char_W1);
	strcat(char_obs_2, char_W2);

	// 8NhClCmCh toevoegen (verplichte groep; behalve voor AUX)
   if (message_mode != "AUX")
   {
	   strcat(char_obs_2, " ");
	   strcat(char_obs_2, "8");
	   strcat(char_obs_2, char_Nh);
	   strcat(char_obs_2, char_Cl);
	   strcat(char_obs_2, char_Cm);
	   strcat(char_obs_2, char_Ch);
   } // if (message_mode != "AUX")

	// 222DsVs toevoegen (verplichte groep)
	strcat(char_obs_2, " ");
	strcat(char_obs_2, "222");
	strcat(char_obs_2, char_Ds);
	strcat(char_obs_2, char_Vs);

	// 0SsTwTwTw toevoegen indien van toepassing
	if (num_TwTwTw != MAXINT)
	{
		strcat(char_obs_2, " ");
		strcat(char_obs_2, "0");
		strcat(char_obs_2, char_Sn_TwTwTw);
		strcat(char_obs_2, char_TwTwTw);
	} // if (num_TwTwTw != MAXINT)

	// verplichte groep (kan wel verschillen afh. van geschatte of gemeten golven) wordt gebruikt door ship en fixed sea stations als golven gemeten
	if ( ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_shipborne) == 0)) ||
        ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_buoy) == 0)) ||
        ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_other) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_shipborne) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_buoy) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_other) == 0)) )
	{
		// 1PwaPwaHwaHwa toevoegen (verplichte groep)
		strcat(char_obs_2, " ");
		strcat(char_obs_2, "1");
		strcat(char_obs_2, char_Pwa);
		strcat(char_obs_2, char_Hwa);
	} // if ((station_type.compare("fixed") == 0) && (golven_resultaat.compare("waves_measured_shipborne") == 0)) etc.
	else // dus geschatte golven
	{
		// 2PwPwHwHw toevoegen (verplichte groep; behalve voor SUS, SUP, AUX)
      if (message_mode != "SUS" && message_mode != "SUP" && message_mode != "AUX")
      {
		   strcat(char_obs_2, " ");
		   strcat(char_obs_2, "2");
		   strcat(char_obs_2, char_Pw);
		   strcat(char_obs_2, char_Hw);
      }

		// 3dw1dw1dw2dw2 toevoegen (verplichte groep; behalve voor SUS, SUP, AUX)
      if (message_mode != "SUS" && message_mode != "SUP" && message_mode != "AUX")
      {
		   strcat(char_obs_2, " ");
		   strcat(char_obs_2, "3");
		   strcat(char_obs_2, char_Dw1);
		   strcat(char_obs_2, char_Dw2);
      }

		// 4Pw1Pw1Hw1Hw1 toevoegen (verplichte groep; behalve voor SUS, SUP, AUX)
      if (message_mode != "SUS" && message_mode != "SUP" && message_mode != "AUX")
      {
		   strcat(char_obs_2, " ");
		   strcat(char_obs_2, "4");
		   strcat(char_obs_2, char_Pw1);
		   strcat(char_obs_2, char_Hw1);
      }

		// 5Pw2Pw2Hw2Hw2 toevoegen (verplichte groep; behalve voor SUS, SUP, AUX)
      if (message_mode != "SUS" && message_mode != "SUP" && message_mode != "AUX")
      {
		   strcat(char_obs_2, " ");
		   strcat(char_obs_2, "5");
		   strcat(char_obs_2, char_Pw2);
		   strcat(char_obs_2, char_Hw2);
      }
	} // else (dus geschatte golven)

	// IsEsEsRs toevoegen indien van toepassing
	if ((num_Is != MAXINT) || (num_EsEs != MAXINT) || (num_Rs != MAXINT))
	{
		strcat(char_obs_2, " ");
		strcat(char_obs_2, "6");
		if (num_Is != MAXINT)
			strcat(char_obs_2, char_Is);
		else
			strcat(char_obs_2, "/");

		if (num_EsEs != MAXINT)
			strcat(char_obs_2, char_EsEs);
		else
			strcat(char_obs_2, "//");

		if (num_Rs != MAXINT)
			strcat(char_obs_2, char_Rs);
		else
			strcat(char_obs_2, "/");

	} // if ((num_Is != MAXINT) || (num_EsEs != MAXINT) || (num_Rs != MAXINT))

	// 70HwaHwaHwa toevoegen indien van toepassing (verplichte groep indien van toepassing) gebruikt door ships en fixed sea stations indien golven gemeten
	if ( ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_shipborne) == 0)) ||
        ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_buoy) == 0)) ||
        ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_other) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_shipborne) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_buoy) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_other) == 0)) )
	{
		if (num_HwaHwaHwa != MAXINT)
		{
			strcat(char_obs_2, " ");
			strcat(char_obs_2, "70");
			strcat(char_obs_2, char_HwaHwaHwa);
		} // if (num_HwaHwaHwa != MAXINT)
	} // if ( ((station_type.compare("fixed") == 0) && (golven_resultaat.compare("waves_measured_shipborne") == 0)) etc.


	// Natte bol temperatuur (8SwTbTbTb) toevoegen indien van toepassing
	// n.b. nattebol alleen toevoegen als deze gebruikt is om het dewpoint te berekenen
	//      indien het hier niet zo was dan was char_TnTnTn niet 0, 1 of 2 (measured)
	if ((strcmp(char_Sn_TnTnTn, "0") == 0) ||
		 (strcmp(char_Sn_TnTnTn, "1") == 0) ||
		 (strcmp(char_Sn_TnTnTn, "2") == 0))
	{
		strcat(char_obs_2, " ");
		strcat(char_obs_2, "8");
		strcat(char_obs_2, char_Sn_TnTnTn);
		strcat(char_obs_2, char_TnTnTn);
	} // if ((strcmp(char_Sn_TnTnTn etc.


	// ice + ciSibiDizi toevoegen indien van toepassing
	if ((num_ci != MAXINT) || (num_Si != MAXINT) || (num_bi != MAXINT) ||
		 (num_Di != MAXINT) || (num_zi != MAXINT))
	{
		strcat(char_obs_2, " ");
		strcat(char_obs_2, "ICE");
		strcat(char_obs_2, " ");

		if (num_ci != MAXINT)
			strcat(char_obs_2, char_ci);
		else
			strcat(char_obs_2, "/");

		if (num_Si != MAXINT)
			strcat(char_obs_2, char_Si);
		else
			strcat(char_obs_2, "/");

		if (num_bi != MAXINT)
			strcat(char_obs_2, char_bi);
		else
			strcat(char_obs_2, "/");

		if (num_Di != MAXINT)
			strcat(char_obs_2, char_Di);
		else
			strcat(char_obs_2, "/");

		if (num_zi != MAXINT)
			strcat(char_obs_2, char_zi);
		else
			strcat(char_obs_2, "/");
	} // if ((num_ci != MAXINT etc.
	//fputs ("test", os);    uit stdio.h   \0 wordt niet gecopieerd


   /* nationale groep(en) toevoegen indien van toepassing */
   if (num_national_1 != MAXINT || num_national_2 != MAXINT || num_national_3 != MAXINT || num_national_4 != MAXINT || num_national_5 != MAXINT)
      national_groups_present = true;
   else
      national_groups_present = false;

   /* nationale groep(en) toevoegen indien van toepassing */
   if (national_groups_present)
   {
      //strcpy(char_obs_3, " ");
      strcat(char_obs_3, "555");

      if (num_national_1 != MAXINT)
      {
         strcat(char_obs_3, " ");
         strcat(char_obs_3, char_national_1);
      } // if (num_national_1 != MAXINT)

      if (num_national_2 != MAXINT)
      {
         strcat(char_obs_3, " ");
         strcat(char_obs_3, char_national_2);
      } // if (num_national_2 != MAXINT)

      if (num_national_3 != MAXINT)
      {
         strcat(char_obs_3, " ");
         strcat(char_obs_3, char_national_3);
      } // if (num_national_3 != MAXINT)

      if (num_national_4 != MAXINT)
      {
         strcat(char_obs_3, " ");
         strcat(char_obs_3, char_national_4);
      } // if (num_national_4 != MAXINT)

      if (num_national_5 != MAXINT)
      {
         strcat(char_obs_3, " ");
         strcat(char_obs_3, char_national_5);
      } // if (num_national_5 != MAXINT)

   } // if (num_national_1 != MAXINT || etc.


   /* eventueel delimiter toevoegen (moet bij char_obs_2 of obs_3 gebeuren) */
   if (delimiter != "")
   {
      if (national_groups_present)
         strcat(char_obs_3, delimiter.c_str());
      else
         strcat(char_obs_2, delimiter.c_str());
   } // if (delimiter != "")



	/* totale obs met regelovergang maken */
	/* alleen bij naar scherm schrijven en via Email OLE wordt char_obs_1, char_obs_2, char_obs_3 apart gebruikt */
	strcat(char_obs, char_obs_1);
	strcat(char_obs, "\n");                 // newline
	strcat(char_obs, char_obs_2);

   if (national_groups_present)
   {
	   strcat(char_obs, "\n");              // newline
	   strcat(char_obs, char_obs_3);
   } // if


}


void TMyWindow::Inmarsat_LES_Advies()
{
	BOOL in_aor_w = FALSE;
	BOOL in_aor_e = FALSE;
	BOOL in_por = FALSE;
	BOOL in_ior = FALSE;
	int scheeps_lengte;
	//int scheeps_breedte;
	BOOL file_aanwezig_ok;
	string dichtstbijzijnde_aor_e_les;
	string dichtstbijzijnde_aor_w_les;
	string dichtstbijzijnde_ior_les;
	string dichtstbijzijnde_por_les;
	string info;
	//double num_obs_lengte;
	//double num_obs_lengte_tienden;
   //char *endptr;


	// Bepalen waar de gebruiker ingelogd kan zijn
	//
	// eerst globaal afronden
	// afgeronde graden
	//
	//num_obs_lengte = strtod(obs_LongitudeDegrees.c_str(), &endptr);
	//num_obs_lengte_tienden = strtod(obs_LongitudeTenths.c_str(), &endptr);
	//scheeps_lengte = int(floor(num_obs_lengte + num_obs_lengte_tienden / 10));  // NIET GOED num_obs_lengte_tienden geeft het aantal MINUTEN aan !!!

   scheeps_lengte = atoi(obs_LongitudeDegrees.c_str());


	if (num_Qc == 5 || num_Qc == 7)                 // westerlengte
      scheeps_lengte *= -1;

	if ((scheeps_lengte >= westgrens_AOR_W) && (scheeps_lengte <= oostgrens_AOR_W))
		in_aor_w = TRUE;
	if ((scheeps_lengte >= westgrens_AOR_E) && (scheeps_lengte <= oostgrens_AOR_E))
		in_aor_e = TRUE;
	if ((scheeps_lengte >= westgrens_POR) && (scheeps_lengte <= oostgrens_POR)) // komt hier nooit in ????
		in_por = TRUE;
	if ((scheeps_lengte >= westgrens_IOR) && (scheeps_lengte <= oostgrens_IOR))
		in_ior = TRUE;
	if ((scheeps_lengte >= westgrens_POR) && (scheeps_lengte <= 180))
		in_por = TRUE;
	if ((scheeps_lengte >= -180) && (scheeps_lengte <= oostgrens_POR))
		in_por = TRUE;


   /* niet alle stations willen BUFR ontvangen */
   if (code_mode == "BUFR")
   {
      if (in_aor_e)
      {
         file_aanwezig_ok = FALSE;
         Check_file_aanwezig(file_aanwezig_ok, bufr_aor_e_file);
         if (file_aanwezig_ok)
         {
            dichtstbijzijnde_aor_e_les = "unknown";
            Bepaal_dichtstbijzijnde_les(bufr_aor_e_file, dichtstbijzijnde_aor_e_les);
            // kan 'unknown' blijven
         } // if (file_aanwezig_ok)
      } // if (in_aor_e)

      if (in_aor_w)
      {
         file_aanwezig_ok = FALSE;
         Check_file_aanwezig(file_aanwezig_ok, bufr_aor_w_file);
         if (file_aanwezig_ok)
         {
            dichtstbijzijnde_aor_w_les = "unknown";
            Bepaal_dichtstbijzijnde_les(bufr_aor_w_file, dichtstbijzijnde_aor_w_les);
            // kan 'unknown' blijven
         } // if (file_aanwezig_ok)
      } // if (in_aor_w)

      if (in_ior)
      {
         file_aanwezig_ok = FALSE;
         Check_file_aanwezig(file_aanwezig_ok, bufr_ior_file);
         if (file_aanwezig_ok)
         {
            dichtstbijzijnde_ior_les = "unknown";
            Bepaal_dichtstbijzijnde_les(bufr_ior_file, dichtstbijzijnde_ior_les);
            // kan 'unknown' blijven
         } // if (file_aanwezig_ok)
      } // if (in_ior)

      if (in_por)
      {
         file_aanwezig_ok = FALSE;
         Check_file_aanwezig(file_aanwezig_ok, bufr_por_file);
         if (file_aanwezig_ok)
         {
            dichtstbijzijnde_por_les = "unknown";
            Bepaal_dichtstbijzijnde_les(bufr_por_file, dichtstbijzijnde_por_les);
            // kan 'unknown' blijven
         } // if (file_aanwezig_ok)
      } // if (in_por)

   } // if (code_mode == "BUFR")
   else // geen BUFR
   {
      if (in_aor_e)
      {
         file_aanwezig_ok = FALSE;
         Check_file_aanwezig(file_aanwezig_ok, aor_e_file);
         if (file_aanwezig_ok)
         {
            dichtstbijzijnde_aor_e_les = "unknown";
            Bepaal_dichtstbijzijnde_les(aor_e_file, dichtstbijzijnde_aor_e_les);
            // kan 'unknown' blijven
         } // if (file_aanwezig_ok)
      } // if (in_aor_e)

      if (in_aor_w)
      {
         file_aanwezig_ok = FALSE;
         Check_file_aanwezig(file_aanwezig_ok, aor_w_file);
         if (file_aanwezig_ok)
         {
            dichtstbijzijnde_aor_w_les = "unknown";
            Bepaal_dichtstbijzijnde_les(aor_w_file, dichtstbijzijnde_aor_w_les);
            // kan 'unknown' blijven
         } // if (file_aanwezig_ok)
      } // if (in_aor_w)


      if (in_ior)
      {
         file_aanwezig_ok = FALSE;
         Check_file_aanwezig(file_aanwezig_ok, ior_file);
         if (file_aanwezig_ok)
         {
            dichtstbijzijnde_ior_les = "unknown";
            Bepaal_dichtstbijzijnde_les(ior_file, dichtstbijzijnde_ior_les);
            // kan 'unknown' blijven
         } // if (file_aanwezig_ok)
      } // if (in_ior)

      if (in_por)
      {
         file_aanwezig_ok = FALSE;
         Check_file_aanwezig(file_aanwezig_ok, por_file);
         if (file_aanwezig_ok)
         {
            dichtstbijzijnde_por_les = "unknown";
            Bepaal_dichtstbijzijnde_les(por_file, dichtstbijzijnde_por_les);
            // kan 'unknown' blijven
         } // if (file_aanwezig_ok)
      } // if (in_por)
   } // else (geen BUFR)


   //
	// advies message opstellen
	//
	info = "\0";
	if (in_aor_w)
	{
		info += "AOR_W : ";
		info += dichtstbijzijnde_aor_w_les;
		info += "\n";
	} // if (in_aor_w)
	if (in_aor_e)
	{
		info += "AOR_E : ";
		info += dichtstbijzijnde_aor_e_les;
		info += "\n";
	} // if (in_aor_e)
	if (in_ior)
	{
		info += "IOR : ";
		info += dichtstbijzijnde_ior_les;
		info += "\n";
	} // if (in_ior)
	if (in_por)
	{
		info += "POR : ";
		info += dichtstbijzijnde_por_les;
		info += "\n";
	} // if (in_por)

	info += "\n";

   if (code_mode == "BUFR")
   	MessageBox(info.c_str(), "Nearest code 41 [BUFR] Inmarsat-C LES", MB_OK | MB_ICONINFORMATION);
   else
   	MessageBox(info.c_str(), "Nearest code 41 Inmarsat-C LES", MB_OK | MB_ICONINFORMATION);
}



void TMyWindow::Bepaal_dichtstbijzijnde_les(string les_file, string &dichtsbijzijnde_les)
{
	ifstream is;
	string les_string  = "\0";
	string les_naam    = "\0";
	string les_breedte = "\0";
	string les_lengte  = "\0";
	int kortste_afstand_tot_les;
	int afstand_tot_les;

	is.open((turbowin_dir + les_file).c_str(), ios::in);

	kortste_afstand_tot_les = MAXINT;

	les_string.read_line(is);
	while (!(is.eof()))
	{
		// de les_string splitsen in les_naam, les_breedte en les_lengte
		les_naam =    les_string.substr(0, 19);      // naam begint op kolom 1
		les_breedte = les_string.substr(19, 3);      // breedte begint op kolom 20
		les_lengte =  les_string.substr(24, 4);      // lengte begint op kolom 25

		//MessageBox(les_naam.c_str(), "TurboWin les_naam", MB_OK);
		//MessageBox(les_breedte.c_str(), "TurboWin les_breedte", MB_OK);
		//MessageBox(les_lengte.c_str(), "TurboWin les_lengte", MB_OK);

		afstand_tot_les = Bepaal_afstand_tot_les(les_breedte, les_lengte, les_file);

		//char buffer[10];
		//sprintf(buffer, "%d", afstand_tot_les);
		//MessageBox(buffer, "TurboWin afstand tot LES", MB_OK);

		if (afstand_tot_les < kortste_afstand_tot_les)
		{
			kortste_afstand_tot_les = afstand_tot_les;
			dichtsbijzijnde_les = "\0";               // initialisatie
			dichtsbijzijnde_les = les_naam;
		} // if (afstand_tot_les < kortste_afstand_tot_les)

		les_string.read_line(is);

	} // while (!(is.eof()))
	//
	// bij een lege file blijft de string 'dichtsbijzijnde_les' dus op "unknown" staan
	//
	is.close();
}


int TMyWindow::Bepaal_afstand_tot_les(string les_breedte, string les_lengte, string les_file)
{
	// gebruikte formule :
	// cos_AB = sin_bA * sin_bB + cos_bA * cos_bB * cos_delta_l_AB
	//
	// afstand = 60 arccos(cos_AB)
	//
	// om van graden hoek radialen hoek te maken : graden * 60 * boogminuut

	int afstand;
	char *endptr;
	double delta_l_ab;
	double cos_delta_l_ab;
	double sin_breedte_a;
	double sin_breedte_b;
	double cos_breedte_a;
	double cos_breedte_b;
	double num_les_breedte;
	double num_les_lengte;
	double num_obs_breedte;
	double num_obs_breedte_tienden;
	double num_obs_lengte;
	double num_obs_lengte_tienden;
   double acos_argument;
	afstand = MAXINT;                               // initialisatie


	// van de les positie numerieke waarden maken
	num_les_breedte = strtod(les_breedte.c_str(), &endptr);
	num_les_lengte = strtod(les_lengte.c_str(), &endptr);

	// van de obs(scheeps)positie numerieke waarden maken
	num_obs_breedte = strtod(obs_LatitudeDegrees.c_str(), &endptr);
	num_obs_breedte_tienden = strtod(obs_LatitudeTenths.c_str(), &endptr);
	num_obs_breedte += num_obs_breedte_tienden / 10;
	if (num_Qc == 3 || num_Qc == 5)                 // Zuiderbreedte
		num_obs_breedte *= -1;

	num_obs_lengte = strtod(obs_LongitudeDegrees.c_str(), &endptr);
	num_obs_lengte_tienden = strtod(obs_LongitudeTenths.c_str(), &endptr);
	num_obs_lengte += num_obs_lengte_tienden / 10;
	if (num_Qc == 5 || num_Qc == 7)                 // Westerlengte
		num_obs_lengte *= -1;



	// i.v.m. 180 graden passage in de POR
	if (les_file == por_file)
	{
		//MessageBox("les_file == por_file", "TurboWin tussenstand", MB_OK);

		if (num_obs_lengte < 0) num_obs_lengte += 360;
		if (num_les_lengte < 0) num_les_lengte += 360;
	} // if (les_file == por_file)

	// lengte verschil bepalen
	delta_l_ab = num_obs_lengte - num_les_lengte;


	// lengteverschil > 180 niet berekenen maar een MAXINT geven
	// > 180 geeft namelijk problemen met formules
	if (fabs(delta_l_ab) > 180)
      afstand = MAXINT;
	else // lengteverschil < 180 graden
	{
		// de (grootcircel)berekening
		sin_breedte_a = sin(num_les_breedte * 60 * boogminuut);
		sin_breedte_b = sin(num_obs_breedte * 60 * boogminuut);
		cos_breedte_a = cos(num_les_breedte * 60 * boogminuut);
		cos_breedte_b = cos(num_obs_breedte * 60 * boogminuut);
		cos_delta_l_ab = cos(delta_l_ab * 60 * boogminuut);

      /* acos argument eerst testen om ACOS domain error te voorkomen */
      acos_argument = sin_breedte_a * sin_breedte_b + cos_breedte_a * cos_breedte_b * cos_delta_l_ab;
      if (acos_argument <= -1 || acos_argument >= 1)
         afstand = 0;   // maw bv bij pos -53-00 N 6-00 E- precies op positie station 12 -> acos_argument = 1
      else
      	afstand = floor(h180pi * acos(acos_argument) + 0.5);

		//afstand = floor(h180pi * acos(sin_breedte_a * sin_breedte_b + cos_breedte_a * cos_breedte_b * cos_delta_l_ab) + 0.5);
	} // else (lengteverschil < 180 graden)


	return afstand;
}





void TMyWindow::IMMT_log()
{
	char immt_rec[256];     //char immt_rec[136];
	char Qc_1[2];
	char Qc_2[2];
	char Qc_3[2];
	char Qc_4[2];
	char Qc_5[2];
	char Qc_6[2];
	char Qc_7[2];
	char Qc_8[2];
	char Qc_9[2];
	char Qc_10[2];
	char Qc_11[2];
	char Qc_12[2];
	char Qc_13[2];
	char Qc_14[2];
	char Qc_15[2];
	char Qc_16[2];
	char Qc_17[2];
	char Qc_18[2];
	char Qc_19[2];
	char Qc_20[2];


	// initialisatie
	immt_rec[0] = '\0';
	strcpy(Qc_1, "9");                              // 9 = the value of the elemnt is missing
	strcpy(Qc_2, "9");
	strcpy(Qc_3, "9");
	strcpy(Qc_4, "9");
	strcpy(Qc_5, "9");
	strcpy(Qc_6, "9");
	strcpy(Qc_7, "9");
	strcpy(Qc_8, "9");
	strcpy(Qc_9, "9");
	strcpy(Qc_10, "9");
	strcpy(Qc_11, "9");
	strcpy(Qc_12, "9");
	strcpy(Qc_13, "9");
	strcpy(Qc_14, "9");
	strcpy(Qc_15, "9");
	strcpy(Qc_16, "9");
	strcpy(Qc_17, "9");
	strcpy(Qc_18, "9");
	strcpy(Qc_19, "9");
	strcpy(Qc_20, "9");


	//
	// immt record velden vullen (volgens WMO version IMMT-2)
	//
   if (message_mode == "AUX")                      // Auxiliary ships vullen geen tienden in (/)
 	   strcpy(immt_rec, "5");                       // char number 1 (5=IMMT format with temp. in whole degrees C)
   else
	   strcpy(immt_rec, "3");                       // char number 1 (3=IMMT format with temp. in tenths of degrees C)


	if (num_AA > 90)
		strcat(immt_rec, "19");                      // char number 2-3
	else
		strcat(immt_rec, "20");                      // char number 2-3

	strcat(immt_rec, char_AA);                      // char number 4-5
	strcat(immt_rec, char_MM);                      // char number 6-7
	strcat(immt_rec, char_DD);                      // char number 8-9
	strcat(immt_rec, char_GG);                      // char number 10-11

	if (num_Qc == 1)
		strcat(immt_rec, "1");                       // char number 12
	else if (num_Qc == 3)
		strcat(immt_rec, "3");                       // char number 12
	else if (num_Qc == 7)
		strcat(immt_rec, "7");                       // char number 12
	else if (num_Qc == 5)
		strcat(immt_rec, "5");                       // char number 12
	else
      strcat(immt_rec, " ");                       // char number 12

	strcat(immt_rec, char_LaLaLa);                  // char number 13-15
	strcat(immt_rec, char_LoLoLoLo);                // char number 16-19
	strcpy(Qc_20, "1");

   strcat(immt_rec, "0");                          // char number 20

	if (num_h == MAXINT)
		strcat(immt_rec, " ");                       // char number 21
	else
	{
		strcat(immt_rec, char_h);                    // char number 21
		strcpy(Qc_1, "1");
	}

	if (num_VV == MAXINT)
		strcat(immt_rec, "  ");                      // char number 22-23
	else
	{
		strcat(immt_rec, char_VV);                   // char number 22-23
		strcpy(Qc_2, "1");
	}

	if (num_N == MAXINT)
   {
		strcat(immt_rec, " ");
		//sprintf(char_N, "%d", MAXINT);
		//strcat(immt_rec, char_N);
	}
	else
	{
		strcat(immt_rec, char_N);                    // char number 24
		strcpy(Qc_3, "1");
	}

	if (num_dd == MAXINT)
		strcat(immt_rec, "  ");                      // char number 25-26
	else
	{
		strcat(immt_rec, char_dd);                   // char number 25-26
		strcpy(Qc_4, "1");
	}

	if (iw == MAXINT)
		strcat(immt_rec, "0");                       // char number 27 (moet een waarde hebben)
	else
		strcat(immt_rec, char_iw);

	// n.b. ff altijd in m/s indien >= 100 knopen
	if (num_ff == MAXINT)
		strcat(immt_rec, "  ");                      // char number 28-29
	else // ff != MAXINT
	{
		if (strcmp(char_fff, "\0") != 0)
			strcpy(char_ff, char_fff + 1);

		strcat(immt_rec, char_ff);                   // char number 28-29
		strcpy(Qc_5, "1");
	} // ff != MAXINT

	if (strcmp(char_Sn_TTT, "\0") == 0)
		strcat(immt_rec, " ");                       // char number 30
	else
		strcat(immt_rec, char_Sn_TTT);               // char number 30

	if (num_TTT == MAXINT)
		strcat(immt_rec, "   ");                     // char number 31-33
	else
	{
		strcat(immt_rec, char_TTT);                  // char number 31-33
		strcpy(Qc_6, "1");
	}

	// sign of dewpoint must be converted to 'computed immt code'
	// also Sn_TdTdTd = 7(ice) is possible contrary to the operational branch
	if (strcmp(char_Sn_TdTdTd, "0") == 0)
		strcat(immt_rec, "5");                       // char number 34
	else if ((strcmp(char_Sn_TdTdTd, "1") == 0) &&
				(strcmp(char_Sn_TnTnTn, "2") != 0) &&
				(strcmp(char_Sn_TnTnTn, "7") != 0))
		strcat(immt_rec, "6");                       // char number 34
	else if ((strcmp(char_Sn_TdTdTd, "1") == 0) &&
				((strcmp(char_Sn_TnTnTn, "2") == 0) ||
				 (strcmp(char_Sn_TnTnTn, "7") == 0)))
		strcat(immt_rec, "7");                       // char number 34
	else
		strcat(immt_rec, " ");                       // char number 34

	//if ((num_TdTdTd == MAXINT) ||
	//	 (strcmp(char_Sn_TdTdTd, "\0") == 0))
	//	strcat(immt_rec, "   ");                     // char number 35-37
	if (num_TdTdTd == MAXINT)
		strcat(immt_rec, "   ");                     // char number 35-37
	else
	{
		strcat(immt_rec, char_TdTdTd);               // char number 35-37
		strcpy(Qc_7, "1");
	}

	if (num_MSL_PPPP == MAXINT)
		strcat(immt_rec, "    ");                    // char number 38-41
	else
	{
		strcat(immt_rec, char_MSL_PPPP);             // char number 38-41
		strcpy(Qc_8, "1");
	}

	if (num_ww == MAXINT)
		strcat(immt_rec, "  ");                      // char_number 42-43
	else
	{
		strcat(immt_rec, char_ww);                   // char number 42-43
		strcpy(Qc_9, "1");
	}

	if (num_W1 == MAXINT)
		strcat(immt_rec, " ");                       // char number 44
	else
	{
		strcat(immt_rec, char_W1);                   // char number 44
		strcpy(Qc_9, "1");
	}

	if (num_W2 == MAXINT)
		strcat(immt_rec, " ");                       // char number 45
	else
	{
		strcat(immt_rec, char_W2);                   // char number 45
		strcpy(Qc_9, "1");
	}

	if (num_Nh == MAXINT)
		strcat(immt_rec, " ");                       // char number 46
	else
	{
		strcat(immt_rec, char_Nh);                   // char number 46
		strcpy(Qc_3, "1");
	}

	if (num_Cl == MAXINT)
		strcat(immt_rec, " ");                       // char number 47
	else
	{
		strcat(immt_rec, char_Cl);                   // char number 47
		strcpy(Qc_3, "1");
	}

	if (num_Cm == MAXINT)
		strcat(immt_rec, " ");                       // char number 48
	else
	{
		strcat(immt_rec, char_Cm);                   // char number 48
		strcpy(Qc_3, "1");
	}

	if (num_Ch == MAXINT)
		strcat(immt_rec, " ");                       // char number 49
	else
	{
		strcat(immt_rec, char_Ch);                   // char number 49
		strcpy(Qc_3, "1");
	}

	if ((strcmp(char_Sn_TwTwTw, "0") == 0) ||
		 (strcmp(char_Sn_TwTwTw, "2") == 0) ||
		 (strcmp(char_Sn_TwTwTw, "4") == 0) ||
		 (strcmp(char_Sn_TwTwTw, "6") == 0))
		strcat(immt_rec, "0");                       // char number 50
	else if ((strcmp(char_Sn_TwTwTw, "1") == 0) ||
				(strcmp(char_Sn_TwTwTw, "3") == 0) ||
				(strcmp(char_Sn_TwTwTw, "5") == 0) ||
				(strcmp(char_Sn_TwTwTw, "7") == 0))
		strcat(immt_rec, "1");                       // char number 50
	else
		strcat(immt_rec, " ");                       // char number 50

	if (num_TwTwTw == MAXINT)
		strcat(immt_rec, "   ");                     // char number 51-53
	else
	{
		strcat(immt_rec, char_TwTwTw);               // char number 51-53
		strcpy(Qc_10, "1");
	}

	if ((strcmp(char_Sn_TwTwTw, "0") == 0) ||
		 (strcmp(char_Sn_TwTwTw, "1") == 0))
		strcat(immt_rec, "1");                       // char number 54
	else if ((strcmp(char_Sn_TwTwTw, "2") == 0) ||
				(strcmp(char_Sn_TwTwTw, "3") == 0))
		strcat(immt_rec, "0");                       // char number 54
	else if ((strcmp(char_Sn_TwTwTw, "4") == 0) ||
				(strcmp(char_Sn_TwTwTw, "5") == 0))
		strcat(immt_rec, "3");                       // char number 54
	else if ((strcmp(char_Sn_TwTwTw, "6") == 0) ||
				(strcmp(char_Sn_TwTwTw, "7") == 0))
		strcat(immt_rec, "7");                       // char number 54
	else
		strcat(immt_rec, " ");                       // char number 54

	/* indicator wave measurement "golven_resultaat" (0 (geschat) of 1,4,7 (measured)) of helemaal niets (SUS, SUP en AUX) */
   if ( (station_type.compare("ship") == 0) && (message_mode == "SUS" || message_mode == "SUP" || message_mode == "AUX") )
   {
      strcat(immt_rec, " ");                       // char number 55
   }
	else if ( ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_shipborne) == 0)) ||
             ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_shipborne) == 0)) )
   {
   	strcat(immt_rec, "1");                       // char number 55
   }
	else if ( ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_buoy) == 0)) ||
             ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_buoy) == 0)) )
   {
   	strcat(immt_rec, "4");                       // char number 55
   }
	else if ( ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_other) == 0)) ||
             ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_other) == 0)) )
   {
   	strcat(immt_rec, "7");                       // char number 55
   }
	else // geschat
		strcat(immt_rec, "0");                       // char number 55


	if ( ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_shipborne) == 0)) ||
        ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_buoy) == 0)) ||
        ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_other) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_shipborne) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_buoy) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_other) == 0)) )
	{
		if (num_Pwa == MAXINT)
			strcat(immt_rec, "  ");                   // char number 56-57
		else
		{
			strcat(immt_rec, char_Pwa);               // char number 56-57
			strcpy(Qc_11, "1");
		}

		if (num_Hwa == MAXINT)
			strcat(immt_rec, "  ");                   // char number 58-59
		else
		{
			strcat(immt_rec, char_Hwa);               // char number 58-59
			strcpy(Qc_12, "1");
		}

		strcat(immt_rec, "  ");                      // char number 60-61

		strcat(immt_rec, "  ");                      // char number 62-63

		strcat(immt_rec, "  ");                      // char number 64-65

	} // if ( ((station_type.compare("fixed") == 0) && (golven_resultaat.compare("waves_measured_sipborne") == 0)) etc.
	else // golven geschat
	{
		if (num_Pw == MAXINT)
			strcat(immt_rec, "  ");                   // char number 56-57
		else
		{
			strcat(immt_rec, char_Pw);                // char number 56-57
			strcpy(Qc_11, "1");
		}

		if (num_Hw == MAXINT)
			strcat(immt_rec, "  ");                   // char number 58-59
		else
		{
			strcat(immt_rec, char_Hw);                // char number 58-59
			strcpy(Qc_12, "1");
		}

		if (num_Dw1 == MAXINT)
			strcat(immt_rec, "  ");                   // char number 60-61
		else
		{
			strcat(immt_rec, char_Dw1);               // char number 60-61
			strcpy(Qc_13, "1");
		}

		if (num_Pw1 == MAXINT)
			strcat(immt_rec, "  ");                   // char number 62-63
		else
		{
			strcat(immt_rec, char_Pw1);               // char number 62-63
			strcpy(Qc_13, "1");
		}

		if (num_Hw1 == MAXINT)
			strcat(immt_rec, "  ");                   // char number 64-65
		else
		{
			strcat(immt_rec, char_Hw1);               // char number 64-65
			strcpy(Qc_13, "1");
		}
	} // else (golven geschat)

	if (num_Is == MAXINT)
		strcat(immt_rec, " ");                       // char number 66
	else
		strcat(immt_rec, char_Is);                   // char number 66

	if (num_EsEs == MAXINT)
		strcat(immt_rec, "  ");                      // char number 67-68
	else
		strcat(immt_rec, char_EsEs);                 // char number 67-68

	if (num_Rs == MAXINT)
		strcat(immt_rec, " ");                       // char number 69
	else
		strcat(immt_rec, char_Rs);                   // char number 69

	strcat(immt_rec, "4");                          // char number 70

	if ( (station_type.compare("ship") == 0) && (message_mode == "SEL") )
		strcat(immt_rec, "1");                       // char number 71
   else if ( (station_type.compare("ship") == 0) && (message_mode == "SUS" || message_mode == "SUP") )
		strcat(immt_rec, "2");                       // char number 71
	else if ( (station_type.compare("ship") == 0) && (message_mode == "AUX") )
		strcat(immt_rec, "3");                       // char number 71
	else if (station_type.compare("fixed") == 0)
		strcat(immt_rec, "5");                       // char number 71
	else
		strcat(immt_rec, "0");                       // char number 71


   /* in IMMT log callsign altijd in hoofdletters */
   if (obs_callsign.length() != 0)
      obs_callsign.to_upper();

	if (obs_callsign.length() == 0)
		strcat(immt_rec, "       ");                 // char number 72-78
	else if (obs_callsign.length() == 1)
	{
		strcat(immt_rec, obs_callsign.c_str());      // char number 72
		strcat(immt_rec, "      ");                  // char number 73-78
	}
	else if (obs_callsign.length() == 2)
	{
		strcat(immt_rec, obs_callsign.c_str());      // char number 72-73
		strcat(immt_rec, "     ");                   // char number 74-78
	}
	else if (obs_callsign.length() == 3)
	{
		strcat(immt_rec, obs_callsign.c_str());      // char number 72-74
		strcat(immt_rec, "    ");                    // char number 75-78
	}
	else if (obs_callsign.length() == 4)
	{
		strcat(immt_rec, obs_callsign.c_str());      // char number 72-75
		strcat(immt_rec, "   ");                     // char number 76-78
	}
	else if (obs_callsign.length() == 5)
	{
		strcat(immt_rec, obs_callsign.c_str());      // char number 72-76
		strcat(immt_rec, "  ");                      // char number 77-78
	}
	else if (obs_callsign.length() == 6)
	{
		strcat(immt_rec, obs_callsign.c_str());      // char number 72-77
		strcat(immt_rec, " ");                       // char number 78
	}
	else if (obs_callsign.length() == 7)
	{
		strcat(immt_rec, obs_callsign.c_str());      // char number 72-78
	}
	else
		strcat(immt_rec, "unknown");                 // char number 72-78

	//Get_recruiting_country();
	if (recruiting_country.compare("Argentina") == 0)
		strcat(immt_rec, "AR");                      // char number 79 - 80
	else if (recruiting_country.compare("Australia") == 0)
		strcat(immt_rec, "AU");                      // char number 79 - 80
	else if (recruiting_country.compare("Azerbaijan") == 0)
		strcat(immt_rec, "AZ");                      // char number 79 - 80
	else if (recruiting_country.compare("Bangladesh") == 0)
		strcat(immt_rec, "BD");                      // char number 79 - 80
	else if (recruiting_country.compare("Belgium") == 0)
		strcat(immt_rec, "BE");                      // char number 79 - 80
	else if (recruiting_country.compare("Bulgaria") == 0)
		strcat(immt_rec, "BG");                      // char number 79 - 80
	else if (recruiting_country.compare("Brazil") == 0)
		strcat(immt_rec, "BR");                      // char number 79 - 80
	else if (recruiting_country.compare("Canada") == 0)
		strcat(immt_rec, "CA");                      // char number 79 - 80
	else if (recruiting_country.compare("Switzerland") == 0)
		strcat(immt_rec, "CH");                      // char number 79 - 80
	else if (recruiting_country.compare("Chile") == 0)
		strcat(immt_rec, "CL");                      // char number 79 - 80
	else if (recruiting_country.compare("China") == 0)
		strcat(immt_rec, "CN");                      // char number 79 - 80
	else if (recruiting_country.compare("Cuba") == 0)
		strcat(immt_rec, "CU");                      // char number 79 - 80
	else if (recruiting_country.compare("Denmark") == 0)
		strcat(immt_rec, "DK");                      // char number 79 - 80
	else if (recruiting_country.compare("Finland") == 0)
		strcat(immt_rec, "FI");                      // char number 79 - 80
	else if (recruiting_country.compare("France") == 0)
		strcat(immt_rec, "FR");                      // char number 79 - 80
	else if (recruiting_country.compare("French Polynesia") == 0)
		strcat(immt_rec, "PF");                      // char number 79 - 80
	else if (recruiting_country.compare("Democratic People's Republic of Korea") == 0)
		strcat(immt_rec, "KP");                      // char number 79 - 80
	else if (recruiting_country.compare("Germany") == 0)
		strcat(immt_rec, "DE");                      // char number 79 - 80
	else if (recruiting_country.compare("Greece") == 0)
		strcat(immt_rec, "GR");                      // char number 79 - 80
	else if (recruiting_country.compare("Hong Kong, China") == 0)
		strcat(immt_rec, "HK");                      // char number 79 - 80
	else if (recruiting_country.compare("Iceland") == 0)
		strcat(immt_rec, "IS");                      // char number 79 - 80
	else if (recruiting_country.compare("India") == 0)
		strcat(immt_rec, "IN");                      // char number 79 - 80
	else if (recruiting_country.compare("Ireland") == 0)
		strcat(immt_rec, "IE");                      // char number 79 - 80
	else if (recruiting_country.compare("Israel") == 0)
		strcat(immt_rec, "IL");                      // char number 79 - 80
	else if (recruiting_country.compare("Italy") == 0)
		strcat(immt_rec, "IT");                      // char number 79 - 80
	else if (recruiting_country.compare("Japan") == 0)
		strcat(immt_rec, "JP");                      // char number 79 - 80
	else if (recruiting_country.compare("Kenya") == 0)
		strcat(immt_rec, "KE");                      // char number 79 - 80
	else if (recruiting_country.compare("Republic of Korea") == 0)
		strcat(immt_rec, "KR");                      // char number 79 - 80
	else if (recruiting_country.compare("Netherlands") == 0)
		strcat(immt_rec, "NL");                      // char number 79 - 80
	else if (recruiting_country.compare("Croatia") == 0)
		strcat(immt_rec, "HR");                      // char number 79 - 80
	else if (recruiting_country.compare("New Zealand") == 0)
		strcat(immt_rec, "NZ");                      // char number 79 - 80
	else if (recruiting_country.compare("Norway") == 0)
		strcat(immt_rec, "NO");                      // char number 79 - 80
	else if (recruiting_country.compare("Pakistan") == 0)
		strcat(immt_rec, "PK");                      // char number 79 - 80
	else if (recruiting_country.compare("Philippines") == 0)
		strcat(immt_rec, "PH");                      // char number 79 - 80
	else if (recruiting_country.compare("Poland") == 0)
		strcat(immt_rec, "PL");                      // char number 79 - 80
	else if (recruiting_country.compare("Portugal") == 0)
		strcat(immt_rec, "PT");                      // char number 79 - 80
	else if (recruiting_country.compare("St. Pierre and Miquelon") == 0)
		strcat(immt_rec, "PM");                      // char number 79 - 80
	else if (recruiting_country.compare("Singapore") == 0)
		strcat(immt_rec, "SG");                      // char number 79 - 80
	else if (recruiting_country.compare("South Africa") == 0)
		strcat(immt_rec, "ZA");                      // char number 79 - 80
	else if (recruiting_country.compare("Spain") == 0)
		strcat(immt_rec, "ES");                      // char number 79 - 80
	else if (recruiting_country.compare("Sweden") == 0)
		strcat(immt_rec, "SE");                      // char number 79 - 80
	else if (recruiting_country.compare("Thailand") == 0)
		strcat(immt_rec, "TH");                      // char number 79 - 80
	else if (recruiting_country.compare("Russian Federation") == 0)
		strcat(immt_rec, "RU");                      // char number 79 - 80
	else if (recruiting_country.compare("United Kingdom of Great Britain and Northern Ireland") == 0)
		strcat(immt_rec, "GB");                      // char number 79 - 80
	else if (recruiting_country.compare("United States of America") == 0)
		strcat(immt_rec, "US");                      // char number 79 - 80
	else if (recruiting_country.compare("Yugoslavia") == 0)
		strcat(immt_rec, "YU");                      // char number 79 - 80
	else if (recruiting_country.compare("Jamaica") == 0)
		strcat(immt_rec, "JM");                      // char number 79 - 80
	else if (recruiting_country.compare("United Republic of Tanzania") == 0)
		strcat(immt_rec, "TZ");                      // char number 79 - 80
	else if (recruiting_country.compare("Malaysia") == 0)
		strcat(immt_rec, "MY");                      // char number 79 - 80
	else if (recruiting_country.compare("Indonesia") == 0)
		strcat(immt_rec, "ID");                      // char number 79 - 80
	else if (recruiting_country.compare("Sri Lanka") == 0)
		strcat(immt_rec, "LK");                      // char number 79 - 80
	else if (recruiting_country.compare("Saudi Arabia") == 0)
		strcat(immt_rec, "SA");                      // char number 79 - 80
	else if (recruiting_country.compare("Lithuania") == 0)
		strcat(immt_rec, "LT");                      // char number 79 - 80
	else if (recruiting_country.compare("Latvia") == 0)
		strcat(immt_rec, "LV");                      // char number 79 - 80
	else if (recruiting_country.compare("Ecuador") == 0)
		strcat(immt_rec, "EC");                      // char number 79 - 80
	else
		strcat(immt_rec, "  ");                      // char number 79 - 80

	// character 81 = voor national use, wil hier zeggen Q = aardquadrant (en niet octant)
	strcat(immt_rec, "Q");                          // char number 81

	// staat nu op automated en WEL time-sequence checks (vanaf versie 3.5)
	strcat(immt_rec, "3");                          // char number 82

	strcat(immt_rec, "1");                          // char number 83

	strcat(immt_rec, "4");                          // char number 84

	strcat(immt_rec, "   ");                        // char number 85-87

	strcat(immt_rec, " ");                          // char number 88

	if (strcmp(char_Sn_TnTnTn, "\0") == 0)
		strcat(immt_rec, " ");                       // char number 89
	else
		strcat(immt_rec, char_Sn_TnTnTn);            // char number 89

	if (num_TnTnTn == MAXINT)
		strcat(immt_rec, "   ");                     // char number 90-92
	else
	//if (num_TnTnTn != MAXINT)
	{
		strcat(immt_rec, char_TnTnTn);               // char number 90-92
		strcpy(Qc_19, "1");
	}

	if (num_a == MAXINT)
		strcat(immt_rec, " ");                       // char number 93
	else
	{
		strcat(immt_rec, char_a);                    // char number 93
		strcpy(Qc_15, "1");
	}

	if (num_ppp == MAXINT)
		strcat(immt_rec, "   ");                     // char number 94-96
	else
	{
		strcat(immt_rec, char_ppp);                  // char number 94-96
		strcpy(Qc_16, "1");
   }

	if (num_Ds == MAXINT)
		strcat(immt_rec, " ");                       // char number 97
	else
	{
		strcat(immt_rec, char_Ds);                   // char number 97
		strcpy(Qc_17, "1");
	}

	if (num_Vs == MAXINT || num_Vs == 888)
		// num_Vs == 888 = unknown
		strcat(immt_rec, " ");                       // char number 98
	else
	{
		strcat(immt_rec, char_Vs);                   // char number 98
		strcpy(Qc_18, "1");
	}

	if ( ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_shipborne) == 0)) ||
        ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_buoy) == 0)) ||
        ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_other) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_shipborne) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_buoy) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_other) == 0)) )
	{
		strcat(immt_rec, "  ");                      // char number 99-100

		strcat(immt_rec, "  ");                      // char number 101-102

		strcat(immt_rec, "  ");                      // char number 103-104

	} // if ( ((station_type.compare("fixed") == 0) && (golven_resultaat.compare("waves_measured_shipborne") == 0)) etc.
	else // golven geschat
	{
		if (num_Dw2 == MAXINT)
			strcat(immt_rec, "  ");                   // char number 99-100
		else
		{
			strcat(immt_rec, char_Dw2);               // char number 99-100
			strcpy(Qc_13, "1");
		}

		if (num_Pw2 == MAXINT)
			strcat(immt_rec, "  ");                   // char number 101-102
		else
		{
			strcat(immt_rec, char_Pw2);               // char number 101-102
			strcpy(Qc_13, "1");
		}

		if (num_Hw2 == MAXINT)
			strcat(immt_rec, "  ");                   // char number 103-104
		else
		{
			strcat(immt_rec, char_Hw2);               // char number 103-104
			strcpy(Qc_13, "1");
		}

	} // else (golven geschat)


	if (num_ci == MAXINT)
		strcat(immt_rec, " ");                       // char number 105
	else
		strcat(immt_rec, char_ci);                   // char number 105

	if (num_Si == MAXINT)
		strcat(immt_rec, " ");                       // char number 106
	else
		strcat(immt_rec, char_Si);                   // char number 106

	if (num_bi == MAXINT)
		strcat(immt_rec, " ");                       // char number 107
	else
		strcat(immt_rec, char_bi);                   // char number 107

	if (num_Di == MAXINT)
		strcat(immt_rec, " ");                       // char number 108
	else
		strcat(immt_rec, char_Di);                   // char number 108

	if (num_zi == MAXINT)
		strcat(immt_rec, " ");                       // char number 109
	else
		strcat(immt_rec, char_zi);                   // char number 109

	strcat(immt_rec, "8");                          // char number 110

	strcat(immt_rec, "2");                          // char number 111

	strcat(immt_rec, Qc_1);                         // char number 112
	strcat(immt_rec, Qc_2);                         // char number 113
	strcat(immt_rec, Qc_3);                         // char number 114
	strcat(immt_rec, Qc_4);                         // char number 115
	strcat(immt_rec, Qc_5);                         // char number 116
	strcat(immt_rec, Qc_6);                         // char number 117
	strcat(immt_rec, Qc_7);                         // char number 118
	strcat(immt_rec, Qc_8);                         // char number 119
	strcat(immt_rec, Qc_9);                         // char number 120
	strcat(immt_rec, Qc_10);                        // char number 121
	strcat(immt_rec, Qc_11);                        // char number 122
	strcat(immt_rec, Qc_12);                        // char number 123
	strcat(immt_rec, Qc_13);                        // char number 124
	strcat(immt_rec, Qc_14);                        // char number 125
	strcat(immt_rec, Qc_15);                        // char number 126
	strcat(immt_rec, Qc_16);                        // char number 127
	strcat(immt_rec, Qc_17);                        // char number 128
	strcat(immt_rec, Qc_18);                        // char number 129
	strcat(immt_rec, Qc_19);                        // char number 130
	strcat(immt_rec, Qc_20);                        // char number 131

   /* onderstaande extra vanaf IMMT-2 (VOS-Clim gebaseerd) */

   strcat(immt_rec, " ");                          // char number 132  (MQCS)

   if (num_HDG == MAXINT)
      strcat(immt_rec, "   ");                     // char number 133-135
   else
      strcat(immt_rec, char_HDG);                  // char number 133-135

   if (num_COG == MAXINT)
      strcat(immt_rec, "   ");                     // char number 136-138
   else
      strcat(immt_rec, char_COG);                  // char number 136-138

   if (num_SOG == MAXINT)
      strcat(immt_rec, "  ");                      // char number 139-140
   else
      strcat(immt_rec, char_SOG);                  // char number 139-140

   if (num_SLL == MAXINT)
      strcat(immt_rec, "  ");                      // char number 141-142
   else
      strcat(immt_rec, char_SLL);                  // char number 141-142

   if (num_hh == MAXINT)
      strcat(immt_rec, "   ");                     // char number 143-145
   else                                            // N.B. omdat bij neg. 143 een "1" moet zijn en bij pos. een "0"
   {                                               // deze groep gesplitst in een sign en een char_hh
      strcat(immt_rec, char_sign_hh);              // [sub]char number 143 (sl volgens IMMT-2 vanaf June 2002)
      strcat(immt_rec, char_hh);                   // [sub]char number 144-145 (hh volgens IMMT-2 vanaf June 2002) 
   }

   if (num_RWD == MAXINT)
      strcat(immt_rec, "   ");                     // char number 146-148
   else
      strcat(immt_rec, char_RWD);                  // char number 146-148

   if (num_RWS == MAXINT)
      strcat(immt_rec, "   ");                     // char number 149-151
   else
      strcat(immt_rec, char_RWS);                  // char number 149-151

   /* dit was ook voor IMMT-2 aanwezig (alleen op andere posities) */
	if (obs_observername.compare("\0") != 0)
	{
		strcat(immt_rec, " ");                       // char number 152   (IMMT-1: char number 132)
		strcat(immt_rec, obs_observername.c_str());  // char number 153-? (IMMT-1:char number 133 - ?)
	} // if (obs_observername.compare("\0") != 0)


   strcat(immt_rec, "\n");                         // regelafsluiting

	Schrijven_IMMT_log(immt_rec);
}



void TMyWindow::Schrijven_IMMT_log(char immt_rec[])
{
	ofstream os;
   ifstream is;
	BOOL file_aanwezig_ok;
	string info;
   string record;
   string last_record = "";

	// controleren of de immt log file aanwezig is
	file_aanwezig_ok = FALSE;
	Check_file_aanwezig(file_aanwezig_ok, immt_file);

	if (file_aanwezig_ok)                           // immt log
	{
      is.open((turbowin_dir + immt_file).c_str(), ios::in);
      if (is)
      {
         do // while(!is.eof());
         {
            record.read_line(is);
            if (!is.eof())
               last_record = record;

         } while(!is.eof());

         is.close();
      } // if (is)
      // N.B.
      // als deze immt_file er niet is dan was er al een medeling gedaan via
      // Check_file_aanwezig()

		os.open((turbowin_dir + immt_file).c_str(), ios::app);    // open voor append
		if (os)
		{
         // alleen nieuwe obs-record toevoegen als hij nog niet in de immt file aanwezig is !!!
         // dit om dubbelen te vermijden
         //
         // N.B.
         // Bij gehele strings vergelijken zullen ze nooit het zelfde zijn
         // daarom tot de Qc's (positie 109 e.v.)
         if ((strlen(last_record.c_str()) > 109) && (strlen(immt_rec) > 109))
         {
            if (strncmp(last_record.c_str(), immt_rec, 109) != 0)
   		   {
               //::MessageBox(0, "weggeschreven was nog NIET aanwezig", "test", MB_OK);
         	   os.write(immt_rec, strlen(immt_rec));
            }
            //else
            //   ::MessageBox(0, "niet wegegeschreven was AL aanwezig", "test", MB_OK);
         } // if ((strlen(last_record.c_str() > 109) && (strlen(immt_rec) > 109))
         else
            if (last_record == "")            // geheel lege log
                os.write(immt_rec, strlen(immt_rec));

         os.close();
      } // if (os)
      else
      {
			info = "\0";
			info += "Unable to append: ";
         info += turbowin_dir;
			info += immt_file;
			info += "\n";
			MessageBox(info.c_str(), "TurboWin error", MB_OK | MB_ICONEXCLAMATION);
		} // else
	} // if (file_aanwezig_ok)
}



void TMyWindow::Observation_Complete_Message(const string output_sort)
{
	string info = "\0";

   /* i.v.m. afsluiten TurboWin */
   observation_complete_message_gepasseerd = true;


   /* zie bld 117 e.v. "Programming OWL for Windows 95" van Vic Broquard */
   /* 65535 = ongeveer 1 minuut      */
   /* N.b. op PC MAXINT = 2147483647 */
   start_seconds_1970 = time(NULL);                      // ijkpunt weer resetten
   KillTimer(1);                                         // voor de zekerheid (indien niet aanwezig geeft deze als return false i.p.v. true
   if (SetTimer(1, 655350, 0) == NULL)                   // ongeveer om de 10 minuten
      MessageBox("Timer failure", "TurboWin system error", MB_OK | MB_ICONERROR);


   if (output_sort != "mail")                            // mail modules geven zelf al een complete message
   {
      info = "Observation completed, you can quit TurboWin.";
	   MessageBox(info.c_str(), "TurboWin", MB_OK | MB_ICONINFORMATION);
   } // if (output_sort != "mail")

}


bool TMyWindow::Check_Land_Sea_Mask()
{
   string volledig_sam_path;
   string volledig_lkw_path;
   int Octant;
   int la_vak_10;
   int la_vak_1;
   int la_vak_1_10;
   int la_voor_lkw;
   int lo_vak_10;
   int lo_vak_1;
   int lo_vak_1_10;
   int lo_voor_lkw;
   int vak_10;
   int sam_index;
   int lkw_index;
   ifstream is_1;
   ifstream is_2;
   string record_sam;
   string record_lkw;
   int regel_no;                       // gelezen regel (record) nummer in lkw file
   int aantal_1;                       // aantal 1'n in sam record nodig om record regel in lkw_file te vonden
   int land_zee_cijfer_sam;            // gevonden cijfer in ZGF_SAM (10 graads vakken file)
   int land_zee_cijfer_lkw;            // gevonden_cijfer in ZGF_LKW (1 graads vakken file)
   int egf_anf;                        // cijfer in sam file dient asls verwijzing naar lkw file
   //int egf_end;
   int lkw_record_Q;                   // positie 118 in lkw file
   int lkw_record_La;                  // positie 119 in lkw file
   int lkw_record_Lo;                  // positie 123 in lkw file
   int berekende_regel_lkw;
   bool zee_vak_ok = true;



   // land_sea mask files files (10 graads en 1/10 graads) moeten aanwezig zijn
   volledig_sam_path = turbowin_dir + mask_sub_dir + sam_file;
   is_1.open(volledig_sam_path.c_str(), ios::in);

   volledig_lkw_path = turbowin_dir + mask_sub_dir + lkw_file;
   is_2.open(volledig_lkw_path.c_str(), ios::in);

   if (is_1 && is_2)
   {

      //
      // Octant bepalen (WMO code table 0371)
      //
      if ((obs_North_or_South == "N") && (obs_East_or_West == "W"))
      {
         if (num_LoLoLoLo >= 0 && num_LoLoLoLo < 900)                      // n.b. 900 = 90.0 gr
            Octant = 0;
         else if (num_LoLoLoLo >= 900 && num_LoLoLoLo <= 1800)             // n.b. 1800 = 180.0 gr
            Octant = 1;
      } // if ((obs_North_or_South == "N") && (obs_East_or_West == "W"))

      else if ((obs_North_or_South == "N") && (obs_East_or_West == "E"))
      {
         if (num_LoLoLoLo >= 900 && num_LoLoLoLo <= 1800)
            Octant = 2;
         else if (num_LoLoLoLo >= 0 && num_LoLoLoLo < 900)
            Octant = 3;
      } // if ((obs_North_or_South == "N") && (obs_East_or_West == "E"))

      else if ((obs_North_or_South == "S") && (obs_East_or_West == "W"))
      {
         if (num_LoLoLoLo >= 0 && num_LoLoLoLo < 900)                      // n.b. 900 = 90.0 gr
            Octant = 5;
         else if (num_LoLoLoLo >= 900 && num_LoLoLoLo <= 1800)             // n.b. 1800 = 180.0 gr
            Octant = 6;
      } // if ((obs_North_or_South == "S") && (obs_East_or_West == "W"))

      else if ((obs_North_or_South == "S") && (obs_East_or_West == "E"))
      {
         if (num_LoLoLoLo >= 900 && num_LoLoLoLo <= 1800)
            Octant = 7;
         else if (num_LoLoLoLo >= 0 && num_LoLoLoLo < 900)
            Octant = 8;
      } // if ((obs_North_or_South == "S") && (obs_East_or_West == "E"))


      //
      // La-Vak bepalen
      //
      la_vak_10    = num_LaLaLa / 100;                        // 123 -> 1
      la_vak_1     = (num_LaLaLa / 10) % 10;                  // 123 -> 2
      la_vak_1_10  = num_LaLaLa % 10;                         // 123 -> 3
      la_voor_lkw  = num_LaLaLa / 10;                         // 123 -> 12

      //
      // Lo-Vak bepalen
      //
      lo_vak_10    = (num_LoLoLoLo / 100) % 10;               // 1234 -> 2
      lo_vak_1     = (num_LoLoLoLo / 10) % 10;                // 1234 -> 3
      lo_vak_1_10  = num_LoLoLoLo % 10;                       // 1234 -> 4
      lo_voor_lkw  = (num_LoLoLoLo / 10) % 100;               // 1234 -> 23

      //
      // 10 graads vak bepalen (WMO code tabel 0371)
      //
      vak_10 = Octant * 100 + la_vak_10 * 10 + lo_vak_10;

      do
      {
         record_sam.read_line(is_1);
         if (record_sam.length() == 127)                   // zijn allemaal 127 char. lang
         {
            if (atoi(record_sam.substr(0, 3).c_str()) == vak_10)
            {
               sam_index = 5 + (11 * la_vak_1) + lo_vak_1; // start op positie 0
               land_zee_cijfer_sam = atoi(record_sam.substr(sam_index, 1).c_str());

               if (land_zee_cijfer_sam == 0)               // zee position
                  zee_vak_ok = true;
               else if (land_zee_cijfer_sam == 4)          // nog niet in masker gezet (dus dan goed rekeken)
                  zee_vak_ok = true;
               else if (land_zee_cijfer_sam == 2)          // wrong position
                  zee_vak_ok = false;
               else if (land_zee_cijfer_sam == 3)          // land position
                  zee_vak_ok = false;
               else if (land_zee_cijfer_sam == 1)          // coast position
               {

                  /////// TEST ///////
                  //::MessageBox(0, "coast position (1)", "test", MB_OK);
                  /////// TEST ///////

                  // aantal 1'n in het record tot sam_index tellen
                  aantal_1 = 0;

                  for (int i = 0; i < sam_index; i++)
                     if (record_sam.substr(i, 1) == "1")
                        aantal_1++;

                  /////// TEST ///////
                  //char buffer[10];
                  //sprintf(buffer, "%d", aantal_1);
                  //::MessageBox(0, buffer, "aantal_1", MB_OK);
                  /////// TEST ///////

                  egf_anf = atoi(record_sam.substr(117, 5).c_str());  // begin regel file ZGF_LKW (1/10 vak)
                  //egf_end = atoi(record_sam.substr(122, 5).c_str());  // eind regel file ZGF_LKW (1/10 vak)
                  berekende_regel_lkw = egf_anf + aantal_1;

                  regel_no = 0;
                  do // while (!is_2.eof());
                  {
                     record_lkw.read_line(is_2);
                     regel_no++;
                     if (record_lkw.length() == 124)       // zijn allemaal 124 lang
                     {
                        if (regel_no == berekende_regel_lkw)
                        {

                           /////// TEST ///////
                           //char buffer[10];
                           //sprintf(buffer, "%d", regel_no);
                           //::MessageBox(0, buffer, "record no in lkw", MB_OK);
                           /////// TEST ///////

                           lkw_record_Q  = atoi(record_lkw.substr(117, 1).c_str());
                           lkw_record_La = atoi(record_lkw.substr(119, 2).c_str());
                           lkw_record_Lo = atoi(record_lkw.substr(122, 2).c_str());

                           // controlle op 1 graads vak indicatie in gevonden lkw record
                           if (lkw_record_Q == Octant && lkw_record_La == la_voor_lkw && lkw_record_Lo == lo_voor_lkw)
                           {
                              /////// TEST ///////
                              //::MessageBox(0, "gepasseerd lkw Q La Lo controle ", "test", MB_OK);
                              /////// TEST ///////

                              lkw_index = 5 + (11 * la_vak_1_10) + lo_vak_1_10;  // start op pos. 0
                              land_zee_cijfer_lkw = atoi(record_lkw.substr(lkw_index, 1).c_str());

                              if (land_zee_cijfer_lkw == 0)     // water positie
                                 zee_vak_ok = true;
                              else if (land_zee_cijfer_lkw == 3)// land positie
                                 zee_vak_ok = false;
                                                                 // ok, gevonden verlaten do-while lkw
                              break;
                           } // if (lkw_record_Q == Octant etc.
                        } // if (regel_no == (egf_anf + aantal_1)) etc.
                     } // if (record_lkw.length() == 128)
                     else // ongeldige regel lengte
                        break;
                  } while (!is_2.eof());

               } // else if (land_zee_cijfer == 1)

               break;                                           // ok, gevonden verlaten do-while sam

            } // if (atoi(record.SubString(0, 3)) == vak_10)
         } // if (record.length() == 127)
         else // ongeldige regel lengte
            break;
      } while (!is_1.eof());

      is_1.close();
      is_2.close();

   } // if (is_1 && is_2)

   return zee_vak_ok;
}



bool TMyWindow::Check_Windows_Version()
{
   OSVERSIONINFO osvi;
   bool windows_version_2000_xp = false;

   /* Voor Windows 2000 -> correct wergegeven b.v. 5.0 build 2195             */
   /* Voor XP           -> niet correct b.v. 7.2 i.p.v. 5.1                   */
   /*                      ligt waarschuijnlijk ergens aan een oude winbase.h */
   /* maakt ook niet uit het gaat hier om onderkennen versies >=5             */
   /* (als je deze function BCB5 zet dan voor XP correcte weergave)           */

   memset(&osvi, 0, sizeof(OSVERSIONINFO));
   osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
   GetVersionEx (&osvi);

   /*
   // Om te testen (dan out-commenten)
   */
   //char szVersion [80];
   //
   //if (osvi.dwPlatformId == VER_PLATFORM_WIN32s)
   //   wsprintf (szVersion, "Microsoft Win32s %d.%d (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber & 0xFFFF);
   //else if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
   //   wsprintf (szVersion, "Microsoft Windows 95/98/Me %d.%d (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber & 0xFFFF);
   //else if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
   //{
   //   if (osvi.dwMajorVersion == 4)
   //      wsprintf (szVersion, "Microsoft Windows NT %d.%d (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber & 0xFFFF);
   //   else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
   //      wsprintf (szVersion, "Microsoft Windows 2000 %d.%d (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber & 0xFFFF);
   //   else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
   //      wsprintf (szVersion, "Microsoft Windows XP %d.%d (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber & 0xFFFF);
   //   else
   //      wsprintf (szVersion, "unknown Microsoft Windows NT-family %d.%d (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber & 0xFFFF);
   //}
   //else
   //   wsprintf (szVersion, "unknown Microsoft Windows platform %d.%d (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber & 0xFFFF);
   //
   //::MessageBox(0, szVersion, "test", MB_OK);

   if ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osvi.dwMajorVersion >= 5))
      windows_version_2000_xp = true;


   return windows_version_2000_xp;
}



bool TMyWindow::Check_Modem_Phone_Line_Connection()
{
   string link_modem_connected = "";
   bool modem_connected = false;
   string info = "";
   string volledig_emailsettings_path = "";
   string record;
   ifstream is;


   // emailsettings_file
   //
   volledig_emailsettings_path = turbowin_dir + email_sub_dir + email_settings_file;
   is.open(volledig_emailsettings_path.c_str(), ios::in);
   if (is)
   {
      do // while (!is.eof());
      {
         record.read_line(is);
         if (record.substr(0, 20) == text_modem)
         {
            if (record.length() >= 20)
            {
               link_modem_connected = record.substr(20);
               if (link_modem_connected == "yes")
                  modem_connected = true;
            }
         } // if (record.substr(0, 20) == text_modem)

      } while (!is.eof());
      is.close();
   } // if (is)
   else // file not open
   {
      info = "Unable to open TurboWin E-mail settings file (";
      info += volledig_emailsettings_path;
      info += ").";
      info += " Please select: Maintenance | E-mail settings.";
      ::MessageBox(0, info.c_str(), "TurboWin error", MB_OK | MB_ICONERROR);
   } // else (file not open)


   return modem_connected;
}




void TMyWindow::CmObsa()
{
	BOOL level_1_ok = TRUE;
	BOOL level_2_ok = TRUE;
	BOOL level_3_ok = TRUE;
   string output_sort = "A:";
   ofstream os;
   bool bufr_encoding_geslaagd;
   bool bufr_source_geslaagd;                         // bufr "source" file aangemaakt geslaagd
   bool statusbar_mode = false;
   bool doorgaan;
   bool compressed_obs_samenstellen_geslaagd;         // meteo-france compression
   char char_telex[2000];                             // meteo-france compression

   
   /* controleren of er wel een disk in A zit */
   // NB vanaf versie 3.7 staat deze check erin hiervoor, zonder deze check, kwam het soms voor dat hij
   // bij de eerste keer bij floppy er doen een foutmelding (ondaks dat alles in orde was) gaf na nog een
   // keer proberen ging het dan wel goed op de een of andere manier gaat het m.b.v. deze check nu wel
   // altijd goed de eerste keer (als alles verder orde is)
   Check_A_drive_Op_Disk_Voor_Obs_Output(doorgaan);

   if (doorgaan == false)
   {
      doorgaan = false;                  // als dit mislukt geeft het os via een keurige messagebox een error melding
   }
   else // dus doorgaan == true
   {
      //
      // samenstellen van de obs
      //
      checking_level_1(level_1_ok, statusbar_mode);
      if (level_1_ok == TRUE)
         checking_level_2(level_2_ok);
      if ((level_1_ok == TRUE) && (level_2_ok == TRUE))
         checking_level_3(level_3_ok);                     // extremen

      if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
      {
         if (code_mode == COMPRESSED_OBS)
         {
//..................
            char_telex[0] = '\0';                               // intialisatie
            compressed_obs_samenstellen_geslaagd = Samenstellen_te_verzenden_COMPRESSED_obs(char_telex);

            if (compressed_obs_samenstellen_geslaagd == true)
            {
               // status gauge tonen
               StatusDlg = new TStatusDlg(this, IDD_STATUSDLG);
               StatusDlg -> Create();
               GetApplication() -> PumpWaitingMessages();

               os.open(obs_output_file, ios::out);          // open om te schrijven
               if (!os)
               {
                  string info = "\0";
                  info += "Unable to write to: ";
                  info += obs_output_file;                    // is absolute path naam
                  info += "\n";
                  info += "Probably: disk write protected or internal error";
                  MessageBox(info.c_str(), "TurboWin error", MB_OK | MB_ICONSTOP);

                  // verwijderen van de status gauge
                  StatusDlg -> Destroy();
                  delete StatusDlg;
               }
               else // os file ok
               {
                  os.write(char_telex, strlen(char_telex));

                  StatusDlg -> UpdateGauge(60);
                  GetApplication() -> PumpWaitingMessages();

                  os.close();

                  StatusDlg -> UpdateGauge(80);
                  GetApplication() -> PumpWaitingMessages();

                  IMMT_log();                               // vullen IMMT log

                  StatusDlg -> UpdateGauge(100);
                  GetApplication() -> PumpWaitingMessages();

                  // verwijderen van de status gauge
                  StatusDlg -> Destroy();
                  delete StatusDlg;

                  if (station_type.compare("fixed") != 0)
                     Inmarsat_LES_Advies();                 // advies dichtsbijzijnde Inmarsat-C code 41 LES

                  // bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te 'moven')
                  Test_IMMT_grootte();

                  /* naar logbook code file (voor private use, not for WMO) */
                  Code_Log();

                  // eventueel updaten van de statistics (als ze op het scherm staan)
                  if (StatisticsObserver_aangevraagd == TRUE)
                  {
                     // bij ingaan CmOptionsStatisticsObserver() wordt
                     // StatisticsObserver_aangevraagd het 'tegenoversgestelde'
                     // i.v.m. vinken (dus hij was er normaal niet en na de
                     // aanvraag wel, hier was hij er wel en moet hij blijven
                     StatisticsObserver_aangevraagd = FALSE;
                     CmOptionsStatisticsObserver();
                  } // if (StatisticsObserver_aangevraagd == TRUE)

                  Observation_Complete_Message(output_sort);

               } // else (os file ok)
            } // if (compressed_obs_samenstellen_geslaagd == true)
//....................
         } // if (code_mode == COMPRESSED_OBS)
         else if (code_mode == BUFR_OBS)                    // BUFR observation
         {
            /* eerst testen of het wel mogelijk is naar A: te schrijven */
            /* via bufr3.exe kan dit niet via een mooie messagebox naar scherm */
            os.open(obs_output_file, ios::out);            // open om te schrijven  (obs_output_file = A:\OBS.TXT -defined)
            if (!os)
            {
               string info = "\0";
               info += "Unable to write to: ";
               info += obs_output_file;                    // is absolute path naam
               info += "\n";
               info += "Probably: disk write protected or internal error";
               //info += "\n";
               //info += "Please try again with disk in drive A:";
               MessageBox(info.c_str(), "TurboWin error", MB_OK | MB_ICONSTOP);
            } // if (!os)
            else // disk in A: ok
            {
               os.close();

               bufr_source_geslaagd = samenstellen_te_verzenden_BUFR_obs();
               if (bufr_source_geslaagd == true)
               {
                  bufr_encoding_geslaagd = Coderen_BUFR_Wegschrijven(obs_output_file);  // NB obs_output_file is obsolute path naam
                  if (bufr_encoding_geslaagd == true)            // succesvol omgezet naar BUFR en weggeschreven
                  {
                     IMMT_log();                                 // vullen IMMT log

                     if (station_type.compare("fixed") != 0)
                        Inmarsat_LES_Advies();                   // advies dichtsbijzijnde Inmarsat-C code 41 LES BUFR

                     // bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te 'moven')
                     Test_IMMT_grootte();

                     // eventueel updaten van de statistics (als ze op het scherm staan)
                     if (StatisticsObserver_aangevraagd == TRUE)
                     {
                        // bij ingaan CmOptionsStatisticsObserver() wordt
                        // StatisticsObserver_aangevraagd het 'tegenoversgestelde
                        // i.v.m. vinken (dus hij was er normaal niet en na de
                        // aanvraag wel, hier was hij er wel en moet hij blijven
                        StatisticsObserver_aangevraagd = FALSE;
                        CmOptionsStatisticsObserver();
                     } // if (StatisticsObserver_aangevraagd == TRUE)

                     Observation_Complete_Message(output_sort);

                  } // if (bufr_encoding_geslaagd == true)
               } // if (bufr_source_geslaagd == true)
            } // else (disk in A: ok)
         } // else if (code_mode == BUFR)
         else // geen BUFR_OBS en geen COMPRESSED_OBS
         {
            samenstellen_te_verzenden_obs();

            // status gauge tonen
            StatusDlg = new TStatusDlg(this, IDD_STATUSDLG);
            StatusDlg -> Create();
            GetApplication() -> PumpWaitingMessages();

            os.open(obs_output_file, ios::out);          // open om te schrijven
            if (!os)
            {
               string info = "\0";
               info += "Unable to write to: ";
               info += obs_output_file;                    // is absolute path naam
               info += "\n";
               info += "Probably: disk write protected or internal error";
               //info += "\n";
               //info += "Please try again with disk in drive A:";
               MessageBox(info.c_str(), "TurboWin error", MB_OK | MB_ICONSTOP);

               // verwijderen van de status gauge
               StatusDlg -> Destroy();
               delete StatusDlg;
            }
            else // os file ok
            {
               //os.write(char_obs, strlen(char_obs));
               if (gts_header_resultaat == "yes")          // Denmark recruited ships kunnen hier gebruik van maken
               {
                  string obs_gts_header_start_line_1;
                  string obs_gts_header_start_line_2;
                  string obs_gts_header_end_line;
                  string destination = "A";
                  Maak_GTS_Header_DMI(obs_gts_header_start_line_1, obs_gts_header_start_line_2, obs_gts_header_end_line, destination);
                  os.write(obs_gts_header_start_line_1.c_str(), strlen(obs_gts_header_start_line_1.c_str()));
                  os.write(obs_gts_header_start_line_2.c_str(), strlen(obs_gts_header_start_line_2.c_str()));
                  os.write(char_obs, strlen(char_obs));
                  os.write(obs_gts_header_end_line.c_str(), strlen(obs_gts_header_end_line.c_str()));
               }
               else // geen gts header
                  os.write(char_obs, strlen(char_obs));

               StatusDlg -> UpdateGauge(60);
               GetApplication() -> PumpWaitingMessages();

               os.close();

               StatusDlg -> UpdateGauge(80);
               GetApplication() -> PumpWaitingMessages();

               IMMT_log();                               // vullen IMMT log

               StatusDlg -> UpdateGauge(100);
               GetApplication() -> PumpWaitingMessages();

               // verwijderen van de status gauge
               StatusDlg -> Destroy();
               delete StatusDlg;

               if (station_type.compare("fixed") != 0)
                  Inmarsat_LES_Advies();                 // advies dichtsbijzijnde Inmarsat-C code 41 LES

               // bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te 'moven')
               Test_IMMT_grootte();

               /* naar logbook code file (voor private use, not for Met Service) */
               Code_Log();

               // eventueel updaten van de statistics (als ze op het scherm staan)
               if (StatisticsObserver_aangevraagd == TRUE)
               {
                  // bij ingaan CmOptionsStatisticsObserver() wordt
                  // StatisticsObserver_aangevraagd het 'tegenoversgestelde
                  // i.v.m. vinken (dus hij was er normaal niet en na de
                  // aanvraag wel, hier was hij er wel en moet hij blijven
                  StatisticsObserver_aangevraagd = FALSE;
                  CmOptionsStatisticsObserver();
               } // if (StatisticsObserver_aangevraagd == TRUE)

               Observation_Complete_Message(output_sort);

            } // else (os file ok)
         } // else (geen BUFR en geen COMPRESSED)
      } // if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))

   } // else (doorgaan == true)
}





void TMyWindow::CmObsCustom()
{
	BOOL level_1_ok = TRUE;
	BOOL level_2_ok = TRUE;
	BOOL level_3_ok = TRUE;
   string output_sort = "custom";
   bool bufr_encoding_geslaagd = false;
   bool bufr_source_geslaagd = false;
   bool statusbar_mode = false;



	//
	// samenstellen van de obs
	//
	checking_level_1(level_1_ok, statusbar_mode);
	if (level_1_ok == TRUE)
		checking_level_2(level_2_ok);
	if ((level_1_ok == TRUE) && (level_2_ok == TRUE))
		checking_level_3(level_3_ok);                // extremen

	if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
	{

		// eerst checken/bepalen naar welke file de obs gaat
		// daarvoor eerst kijken of de file waarin de custom file naam staat aanwezig is
		// om te kijken of de obs_custom_file bestaat, de file even openen om te lezen

		string obs_custom_output_file = "\0";        // clear

		BOOL file_aanwezig_ok = FALSE;
		Check_file_aanwezig(file_aanwezig_ok, obs_custom_verwijs_file);
		if (file_aanwezig_ok)
		{
			ifstream is;
			is.open((turbowin_dir + obs_custom_verwijs_file).c_str(), ios::in);
			// ophalen van (absolute) path + name uit obs_custom_verwijs_file
			// en deze toekennen aan obs_custom_output_file
			// obs_custom_output_file = absolute naam + path
			obs_custom_output_file.read_line(is);
			is.close();

			BOOL schrijven_ok = FALSE;
			string string_obs_custom = "\0";             // mag niet binnen case label
			ofstream os;      									// mag niet binnen case label
			int result;                                  // mag niet binnen case label
			string_obs_custom = obs_custom_output_file;  // moet staan voor de volgende regel

			TCustomDialog dlg(this, IDD_CUSTOMFILE, customtransfer, string_obs_custom, turbowin_dir); // mag niet binnen case label

			string info = "\0";
			info = "Obs to: ";
			info += obs_custom_output_file;
			info += ".\n";
			info += "(Press the NO button to change the path and name of the custom file).";
			switch (MessageBox(info.c_str(), "TurboWin message", MB_YESNOCANCEL | MB_ICONQUESTION))
			{
				case IDYES :    // dus nu wegschrijven naar de obs_custom_output_file
									 schrijven_ok = TRUE;
									 break;
				case IDNO :     // nu een edit scherm tonen met hierin het path en
									 // de naam van obs_custom_output_file deze staat dus
									 // als text string in de file obs_custom_file
									 // met mogelijkheid dit te veranderen
									 if ((result = dlg.Execute()) == IDOK)
									 {
										 // first checking the outputfile obs_custom_verwijs_file
										 os.open((turbowin_dir + obs_custom_verwijs_file).c_str(), ios::out);
										 if (!os)
										 {
											 info = "\0";
											 info = "Unable to write to: ";
                                  info += turbowin_dir;
											 info += obs_custom_verwijs_file;
                                  info += ".\n";
											 info += " (necessary for writing name and path custom file)";
											 MessageBox(info.c_str(), "TurboWin error", MB_OK | MB_ICONSTOP);
										 } // if (!is)
										 else // obs_custom_verwijs_file ok
										 {
											 // buffer read out
											 obs_custom = "\0";
											 obs_custom = customtransfer.CustomEditResult;

											 obs_custom_output_file = "\0";
											 obs_custom_output_file = obs_custom;

											 // in de var. obs_custom staat nu path en naam
											 // van file waar obs naartoe geschreven wordt

											 // de inhoud van var. obs_custom wegschrijven naar
											 // obs_custom_verwijs_file
											 os.write(obs_custom.c_str(), strlen(obs_custom.c_str()));
											 os.close();
										 } // else (obs_custom_file ok)
									 } // if ((result = dlg.Execute()) == IDOK)

									 if (result == IDOK)
									 {
										 info = "\0";
										 info = "Obs to: ";
										 info += obs_custom_output_file; // absolute path + name
										 info += ".\n";
										 if (MessageBox(info.c_str(), "TurboWin message", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
											 schrijven_ok = TRUE;
										 else
											 schrijven_ok = FALSE;
									 } // if (result == IDOK)

									 break;
				case IDCANCEL : // helemaal niet wegschrijven
									 schrijven_ok = FALSE;
									 break;
			} // switch (MessageBox(info.c_str(), "TurboWin message", MB_YESNOCANCEL | MB_ICONQUESTION))

			if (schrijven_ok)
			{
            if (code_mode == "BUFR")
            {
               /* eerst testen of het wel mogelijk is naar custom file te schrijven */
               /* bufr3.exe doet dit ook maar kan geen mooie melding onder windows naar scherm geven (console program) */
               os.open(obs_custom_output_file.c_str(), ios::out);            // open om te schrijven  (obs_custom_output_file = ..)
               if (!os)
               {
                  info = "\0";
                  info = "Unable to write to: ";
                  info += obs_custom_output_file;  // absolute path + name
                  info += "\n";
                  info += "Please check drive and directory and try agian";
                  MessageBox(info.c_str(), "TurboWin error", MB_OK | MB_ICONSTOP);
               } // if (!os)
               else // obs custum outputfile ok
               {
                  os.close();

                  bufr_source_geslaagd = samenstellen_te_verzenden_BUFR_obs();
                  if (bufr_source_geslaagd == true)
                  {
                     bufr_encoding_geslaagd = Coderen_BUFR_Wegschrijven(obs_custom_output_file);
                     if (bufr_encoding_geslaagd == true)         // succesvol omgezet naar BUFR en weggeschreven
                     {
                        IMMT_log();                              // vullen IMMT_LOG();

                        if (station_type.compare("fixed") != 0)
                           Inmarsat_LES_Advies();           // advies dichtsbijzijnde Inmarsat-C code 41 LES BUFR

                        // bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te 'moven')
                        Test_IMMT_grootte();

                        // eventueel updaten van de statistics (als ze op het scherm staan)
                        if (StatisticsObserver_aangevraagd == TRUE)
                        {
                           // bij ingaan CmOptionsStatisticsObserver() wordt
                           // StatisticsObserver_aangevraagd het 'tegenoversgestelde
                           // i.v.m. vinken (dus hij was er normaal niet en na de
                           // aanvraag wel, hier was hij er wel en moet hij blijven
                           StatisticsObserver_aangevraagd = FALSE;
                           CmOptionsStatisticsObserver();
                        } // if (StatisticsObserver_aangevraagd == TRUE)

                        Observation_Complete_Message(output_sort);

                     } // if (bufr_encoding_geslaagd == true)
                  } // if (bufr_source_geslaagd == true)
               } // else (custom output file ok)
            } // if (code_mode == BUFR)
            else // code_mode geen BUFR
            {
               samenstellen_te_verzenden_obs();

               // status gauge tonen
               StatusDlg = new TStatusDlg(this, IDD_STATUSDLG);
               StatusDlg -> Create();
               GetApplication() -> PumpWaitingMessages();

               os.open(obs_custom_output_file.c_str(), ios::out);
               if (!os)
               {
                  info = "\0";
                  info = "Unable to write to: ";
                  info += obs_custom_output_file;  // absolute path + name
                  info += "\n";
                  info += "Please check drive and directory and try agian";
                  MessageBox(info.c_str(), "TurboWin error", MB_OK | MB_ICONSTOP);

                  // verwijderen van de status gauge
                  StatusDlg -> Destroy();
                  delete StatusDlg;
               } // if (!os)
               else // file ok
               {
                  //os.write(char_obs, strlen(char_obs));
                  if (gts_header_resultaat == "yes")          // Denmark recruited ships kunnen hier gebruik van maken
                  {
                     string obs_gts_header_start_line_1;
                     string obs_gts_header_start_line_2;
                     string obs_gts_header_end_line;
                     string destination = "CUSTOM";

                     Maak_GTS_Header_DMI(obs_gts_header_start_line_1, obs_gts_header_start_line_2, obs_gts_header_end_line, destination);
                     os.write(obs_gts_header_start_line_1.c_str(), strlen(obs_gts_header_start_line_1.c_str()));
                     os.write(obs_gts_header_start_line_2.c_str(), strlen(obs_gts_header_start_line_2.c_str()));
                     os.write(char_obs, strlen(char_obs));
                     os.write(obs_gts_header_end_line.c_str(), strlen(obs_gts_header_end_line.c_str()));
                  }
                  else // geen gts header
                     os.write(char_obs, strlen(char_obs));


                  StatusDlg -> UpdateGauge(60);
                  GetApplication() -> PumpWaitingMessages();

                  os.close();

                  StatusDlg -> UpdateGauge(80);
                  GetApplication() -> PumpWaitingMessages();

                  IMMT_log();                         // vullen IMMT_LOG();

                  StatusDlg -> UpdateGauge(100);
                  GetApplication() -> PumpWaitingMessages();

                  // verwijderen van de status gauge
                  StatusDlg -> Destroy();
                  delete StatusDlg;

                  if (station_type.compare("fixed") != 0)
                     Inmarsat_LES_Advies();           // advies dichtsbijzijnde Inmarsat-C code 41 LES

                  // bepalen grootte (in bytes) van IMMT log (+ eventueel advies om te 'moven')
                  Test_IMMT_grootte();

                  /* naar logbook code file (voor private use, not for Met Service) */
                  Code_Log();

                  // eventueel updaten van de statistics (als ze op het scherm staan)
                  if (StatisticsObserver_aangevraagd == TRUE)
                  {
                     // bij ingaan CmOptionsStatisticsObserver() wordt
                     // StatisticsObserver_aangevraagd het 'tegenoversgestelde
                     // i.v.m. vinken (dus hij was er normaal niet en na de
                     // aanvraag wel, hier was hij er wel en moet hij blijven
                     StatisticsObserver_aangevraagd = FALSE;
                     CmOptionsStatisticsObserver();
                  } // if (StatisticsObserver_aangevraagd == TRUE)

                  Observation_Complete_Message(output_sort);

               } // else (file ok)
            } // else (code_mode geen BUFR)
			} // if (schrijven_ok = TRUE)
		} // if (file_aanwezig_ok)
	} // if ((level_1_ok == TRUE) && (level_2_ok == TRUE) && (level_3_ok == TRUE))
}



void TMyWindow::Maak_GTS_Header_DMI(string& obs_gts_header_start_line_1, string& obs_gts_header_start_line_2, string& obs_gts_header_end_line, string destination)
{
   string T1_designator;
   string T2_designator;
   string A1_designator;
   string A2_designator;
   string ii_designator;
   string header_CCCC;
   string header_YY;
   string header_GG;
   string header_gg;
   char *endptr;
//
//   // deze IA5 werkt niet goed omdat de non-printable characters SOH en ETX niet overkomen in een e-maoil programma
//   //
//   // op verzoek (DMI) Danish Meteorological Institute
//   //
//   // GTS header:
//   // <SOH><CR><CR><LF>T1T2A1A2ii<SP>CCCC<SP>YYGGgg<CR><CR><LF>
//   //
//   // ---- message -----
//   //
//   // <CR><CR><LF><ETX>
//   //


   //
   // op verzoek (DMI) Danish Meteorological Institute
   //
   // Header using telegraph alphabet no. 2
   //
   // ZCZC<sp><sp><sp><sp><sp><sp><sp><CR><LF>
   // T1T2A1A2ii<SP>CCCC<SP>YYGGgg<CR><CR><LF>
   // BBXX bla..... bla ........ =<CR><LF>
   // NNNN




   //
   // T1  [Table A OPERATIONAL PROCEDURES FOR THE GTS]
   //
   T1_designator = "S";               // surface data


   //
   // T2 [Table B1 OPERATIONAL PROCEDURES FOR THE GTS]
   //
   if (num_GG == 0 || num_GG == 6 || num_GG == 12 || num_GG == 18)
      T2_designator = "M";                                               // main hour
   else if (num_GG == 3 || num_GG == 9 || num_GG == 15 || num_GG == 21)
      T2_designator = "I";                                               // intermediate hour
   else
     T2_designator = "N";                                                // non standard hour


   //
   // A1 (geographical designator) [Table C2 OPERATIONAL PROCEDURES FOR THE GTS]
   //
   A1_designator = "V";               // for mobile ships and other marine stations (not for ocean weather ships)


   //
   // A2 (geographical designator) [Table C2 OPERATIONAL PROCEDURES FOR THE GTS]
   //
   double noord_grens_A = 30;
   double zuid_grens_A = -60;
   double west_grens_A = -35;
   double oost_grens_A = 70;

   double noord_grens_B = 90;
   double zuid_grens_B = 5;
   double west_grens_B = 70;
   double oost_grens_B = 180;

   double noord_grens_C = 5;
   double zuid_grens_C = -60;
   double west_grens_C = -120;
   double oost_grens_C = -35;

   double noord_grens_D = 90;
   double zuid_grens_D = 5;
   double west_grens_D = -180;
   double oost_grens_D = -35;

   double noord_grens_E1 = 5;                                 // i.v.m. om de 180 graden gebied lengte E even gesplitst
   double zuid_grens_E1 = -60;
   double west_grens_E1 = 70;
   double oost_grens_E1 = 180;

   double noord_grens_E2 = 5;
   double zuid_grens_E2 = -60;
   double west_grens_E2 = -180;
   double oost_grens_E2 = -120;

   double noord_grens_F = 90;
   double zuid_grens_F = 30;
   double west_grens_F = -35;
   double oost_grens_F = 70;

   double noord_grens_J1 = -60;                               // i.v.m. om de 180 graden gebied lengte J even gesplitst
   double zuid_grens_J1 = -90;
   double west_grens_J1 = 0;
   double oost_grens_J1 = 180;

   double noord_grens_J2 = -60;
   double zuid_grens_J2 = -90;
   double west_grens_J2 = -180;
   double oost_grens_J2 = 0;

	double num_obs_breedte;
	double num_obs_breedte_tienden;
	double num_obs_lengte;
	double num_obs_lengte_tienden;

////////////// test /////////////
//   MessageBox(form_mode.c_str(), "form mode", MB_OK);
//
//   char buffer[10];
//   sprintf(buffer, "%d", num_LoLoLoLo);
//   MessageBox(buffer, "num_LoLoLoLo", MB_OK);
//
//   MessageBox(obs_East_or_West.c_str(), "obs_East_or_West", MB_OK);
/////////////////////////////////



   //
   // in classic mode zijn beschikbaar:
   //            num_LoLoLoLo [0 - 1800]
   //            num_LaLaLa [0 - 900]
   //            obs_North_or_South ["N", "S"]
   //            obs_East_or_West ["E", "W"]
   //
   if (form_mode == CLASSIC)
   {
      num_obs_breedte = (double)num_LaLaLa / 10;
      if (obs_North_or_South == "S")
         num_obs_breedte *= -1;

      num_obs_lengte = (double)num_LoLoLoLo / 10;
      if (obs_East_or_West == "W")
         num_obs_lengte *= -1;
   } // if (form_mode == CLASSIC)
   else // geen classic mode
   {
	   // van de obs(scheeps)positie numerieke waarden maken
	   num_obs_breedte = strtod(obs_LatitudeDegrees.c_str(), &endptr);
	   num_obs_breedte_tienden = strtod(obs_LatitudeTenths.c_str(), &endptr);
	   num_obs_breedte += num_obs_breedte_tienden / 10;
	   if (num_Qc == 3 || num_Qc == 5)                 // Zuiderbreedte
		   num_obs_breedte *= -1;

	   num_obs_lengte = strtod(obs_LongitudeDegrees.c_str(), &endptr);
	   num_obs_lengte_tienden = strtod(obs_LongitudeTenths.c_str(), &endptr);
	   num_obs_lengte += num_obs_lengte_tienden / 10;
	   if (num_Qc == 5 || num_Qc == 7)                 // Westerlengte
		   num_obs_lengte *= -1;
   } // else (geen classic mode)


   // As designator gebied bepalen
   if (num_obs_breedte <= noord_grens_A && num_obs_breedte >= zuid_grens_A &&
       num_obs_lengte >= west_grens_A && num_obs_lengte <= oost_grens_A)
   {
      A2_designator = "A";
   }
   else if (num_obs_breedte <= noord_grens_B && num_obs_breedte >= zuid_grens_B &&
       num_obs_lengte >= west_grens_B && num_obs_lengte <= oost_grens_B)
   {
      A2_designator = "B";
   }
   else if (num_obs_breedte <= noord_grens_C && num_obs_breedte >= zuid_grens_C &&
       num_obs_lengte >= west_grens_C && num_obs_lengte <= oost_grens_C)
   {
     A2_designator = "C";
   }
   else if (num_obs_breedte <= noord_grens_D && num_obs_breedte >= zuid_grens_D &&
       num_obs_lengte >= west_grens_D && num_obs_lengte <= oost_grens_D)
   {
      A2_designator = "D";
   }
   else if (num_obs_breedte <= noord_grens_E1 && num_obs_breedte >= zuid_grens_E1 &&
       num_obs_lengte >= west_grens_E1 && num_obs_lengte <= oost_grens_E1)
   {
      A2_designator = "E";
   }
   else if (num_obs_breedte <= noord_grens_E2 && num_obs_breedte >= zuid_grens_E2 &&
      num_obs_lengte >= west_grens_E2 && num_obs_lengte <= oost_grens_E2)
   {
      A2_designator = "E";
   }
   else if (num_obs_breedte <= noord_grens_F && num_obs_breedte >= zuid_grens_F &&
       num_obs_lengte >= west_grens_F && num_obs_lengte <= oost_grens_F)
   {
      A2_designator = "F";
   }
   else if (num_obs_breedte <= noord_grens_J1 && num_obs_breedte >= zuid_grens_J1 &&
       num_obs_lengte >= west_grens_J1 && num_obs_lengte <= oost_grens_J1)
   {
      A2_designator = "J";
   }
   else if (num_obs_breedte <= noord_grens_J2 && num_obs_breedte >= zuid_grens_J2 &&
       num_obs_lengte >= west_grens_J2 && num_obs_lengte <= oost_grens_J2)
   {
      A2_designator = "J";
   }
   else
      A2_designator = "X";   // betekent eigenlijk: more than one area


   //
   // ii (deifferentiate 2 or more bulletins
   //
   //  ii_designator = "01";                      // altijd 01 voor ship met 1 obs in bulletin ?????????????????
   if (num_GG == 0 || num_GG == 6 || num_GG == 12 || num_GG == 18)
      ii_designator = "01";                                               // main hour
   else if (num_GG == 3 || num_GG == 9 || num_GG == 15 || num_GG == 21)
      ii_designator = "21";                                               // intermediate hour
   else
      ii_designator = "60";                                                // non standard hour



   //
   // CCCC (int. 4 letter location indicator of the station originating/compiling the bulletin) [OPERATIONAL PROCEDURES FOR THE GTS]
   //
   header_CCCC = "EKMI";                      //  (Danish Meteorological Institute)


   //
   // YY
   //
   header_YY = string(char_DD);               // dag van waarneming


   //
   // GG
   //
   header_GG = string(char_GG);               // uur van waarneming


   //
   // gg
   //
   header_gg = "00";                          // per definitie 0 minuten


#if 0
//   //
//   //
//   //
//   char SOH[2];
//   SOH[0] = '\0';
//   SOH[0] = '\x1';                            // hex 1
//
//   char CR[2];
//   CR[0] = '\0';
//   CR[0] = '\xD';                             // hex D
//
//   char LF[2];
//   LF[0] = '\0';
//   LF[0] = '\xA';                             // hex A
//
//   char ETX[2];
//   ETX[0] = '\0';
//   ETX[0] = '\x3';                            // hex 3
//
//   char SPACE[2];                             // om de een of andere reden geeft char SP geeft een foutmelding (SP reserved voor geheugen pointer o.i.d. ?)
//   SPACE[0] = '\0';
//   SPACE[0] = '\x20';                         // hex 20
//
//   obs_gts_header_start_line = "";
//   obs_gts_header_start_line = string(SOH) + string(CR) + string(CR) + string(LF) +
//                               T1_designator + T2_designator + A1_designator + A2_designator + ii_designator +
//                               string(SPACE) + header_CCCC + string(SPACE);
//                               header_YY + header_GG + header_gg +
//                               string(CR) + string(CR) + string(LF);
//
//   obs_gts_header_end_line = "";
//   obs_gts_header_end_line = string(CR) + string(CR) + string(LF) + string(ETX);
#endif


   // initialisatie
   string LF = "";
   string SPACE = "";

   if (destination == "EMAIL_OLE")              // "\xA" wordt genegeerd door b.v. outlook express
      LF = ""; //"%0A";                               // bij WINDOWS ZIT HIER DAN OOK GELIJK EEN CR VOOR !   // hex A
   if (destination == "CLIPBOARD")              // allen bij naar clipboard wel weer de CR er bij zetten anders niet doorgegeven naar b.v. klablok
      LF = "\xD\xA";
   else
      LF = "\xA";                               // WINDOWS ZELF ZIT HIER DAN OOK GELIJK EEN CR VOOR !

   //ETX   = "\x3";                             // hex 3
   SPACE = "\x20";                            // hex 20


   // obs_gts_header_start_line:
   //
   // ZCZC<sp><sp><sp><sp><sp><sp><sp><CR><LF>
   // T1T2A1A2ii<SP>CCCC<SP>YYGGgg<CR><CR><LF>
   //

   //obs_gts_header_start_line = "";
   //obs_gts_header_start_line = "ZCZC" + SPACE + SPACE + SPACE + SPACE + SPACE + SPACE + SPACE + LF +
   //                            T1_designator + T2_designator + A1_designator + A2_designator + ii_designator +
   //                            SPACE + header_CCCC + SPACE + header_YY + header_GG + header_gg + LF;

   /* initialisatie */
   obs_gts_header_start_line_1 = "";
   obs_gts_header_start_line_2 = "";

   obs_gts_header_start_line_1 = "ZCZC" + SPACE + SPACE + SPACE + SPACE + SPACE + SPACE + SPACE + LF;
   obs_gts_header_start_line_2 = T1_designator + T2_designator + A1_designator + A2_designator + ii_designator +
                                 SPACE + header_CCCC + SPACE + header_YY + header_GG + header_gg + LF;


   // obs_gts_header_end_line
   //
   // <CR><LF>NNNN
   //
   obs_gts_header_end_line = "";
   obs_gts_header_end_line = LF + "NNNN";

}



void TMyWindow::Code_Log()
{
   char char_code_obs[1024];

	//
	// WMO code
	//
	// bbxx callsign YYGGiw irixhVV Nddff (00fff) 1SnTTT 2SnTdTdTd 4PPPP 5apppp
	// 7wwW1W2 8NhClCmCh 222DsVs 0SsTwTwTw 2PwPwHwHw 3dw1dw1dw2dw2 4Pw1Pw1Hw1Hw1
	// 5Pw2Pw2Hw2Hw2 6IsEsEsRs (8swTbTbTb) (ICE ciSibiDizi)
   //
   //
   // opmerkingen voor deze code log (veranderingen t.o.v. verzend obs c.q. IMMT log):
   //
   // - groep 99fff komt er niet in voor
   // - groep 1PwaPwaHwaHwa (gemeten golven) wordt geschreven onder label 2PwPwHwHw
   // dit i.v.m. uniformiteit met Met Office Ship's Meteorological logbook
   //


   /* initialisatie */
	char_code_obs[0]   = '\0';

	// bbxx (verplichte groep)
	strcpy(char_code_obs, "BBXX");

	// callsign (verplichte groep)
	strcat(char_code_obs, " ");
	strcat(char_code_obs, obs_callsign.c_str());

   // year (verplicht)
   strcat(char_code_obs, " ");
	if (num_AA > 90)
		strcat(char_code_obs, "19");
	else
		strcat(char_code_obs, "20");
	strcat(char_code_obs, char_AA);

   // Month (verplicht)
   strcat(char_code_obs, " ");
	strcat(char_code_obs, char_MM);

	// YYGGiw (verplichte groep)
	strcat(char_code_obs, " ");
	strcat(char_code_obs, char_DD);
	strcat(char_code_obs, char_GG);
	//if ((iw == MAXINT) && (num_dd == MAXINT && num_ff == MAXINT))
	if (iw == MAXINT)
		strcat(char_code_obs, "0");     // volgens de code moet er wat staan bij iw (dus daarom een willekeurige waarde genomen)
	else
		strcat(char_code_obs, char_iw);

	// breedte (99LaLaLa) (verplichte groep)
	strcat(char_code_obs, " ");
	strcat(char_code_obs, "99");
	strcat(char_code_obs, char_LaLaLa);

	// Aardquadrant + lengte (QcLoLoLoLo) toevoegen (verplichte groep)
	strcat(char_code_obs, " ");
	strcat(char_code_obs, char_Qc);
	strcat(char_code_obs, char_LoLoLoLo);

	// irixhVV toevoegen (verplichte groep)
	strcat(char_code_obs, " ");
	strcat(char_code_obs, "4");                // ir
	strcat(char_code_obs, "1");                // ix
	strcat(char_code_obs, char_h);             // h
	strcat(char_code_obs, char_VV);            // VV

	// Nddff toevoegen (verplichte groep)
	strcat(char_code_obs, " ");
	strcat(char_code_obs, char_N);              // N
	strcat(char_code_obs, char_dd);             // dd
	strcat(char_code_obs, char_ff);             // ff

   /* in Ship's Meteorological Logbook wordt deze groep ook niet opgenomen) */
//	// 00fff toevoegen indien van toepassing
//	if ((num_ff > 99) && (num_ff != MAXINT))
//	{
//		strcat(char_code_obs, " ");
//		strcat(char_code_obs, "00");
//		strcat(char_code_obs, char_fff);
//	} // if (num_ff > 99)

	// Tair toevoegen (1SnTTT) (geen verplichte groep)
	if (num_TTT != MAXINT)
	{
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "1");
		strcat(char_code_obs, char_Sn_TTT);
		strcat(char_code_obs, char_TTT);
	} // if (num_TTT != MAXINT)
   else
   {
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "1////");
   }

	// 2SnTdTdTd toevoegen (geen verplichte groep)
	if (num_TdTdTd != MAXINT)
	{
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "2");
		strcat(char_code_obs, char_Sn_TdTdTd);
		strcat(char_code_obs, char_TdTdTd);
	} // if (num_TdTdTd != MAXINT)
   else
	{
   	strcat(char_code_obs, " ");
		strcat(char_code_obs, "2////");
   }


	// air-pressure (4PPPP) (geen verplichte groep)
	if (num_MSL_PPPP != MAXINT)
	{
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "4");
		strcat(char_code_obs, char_MSL_PPPP);
	} // if (num_MSL_PPPP != MAXINT)
   else
   {
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "4////");
   }

	// air pressure characteristic and amount of tendency (5appp) toevoegen (geen verplichte groep)
	if ((num_a != MAXINT) || (num_ppp != MAXINT))
	{
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "5");
		if (num_a != MAXINT)
			strcat(char_code_obs, char_a);
		else
			strcat(char_code_obs, "/");
		if (num_ppp != MAXINT)
			strcat(char_code_obs, char_ppp);
		else
			strcat(char_code_obs, "///");                  // dit is / (3*)
	} // if ((num_a != MAXINT) || (num_ppp != MAXINT))
   else
   {
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "5////");
   }

	// 7wwW1W2 toevoegen (verplichte groep)
	strcat(char_code_obs, " ");
	strcat(char_code_obs, "7");
	strcat(char_code_obs, char_ww);
	strcat(char_code_obs, char_W1);
	strcat(char_code_obs, char_W2);

	// 8NhClCmCh toevoegen (verplichte groep; behalve voor AUX)
   if (message_mode != "AUX")
   {
	   strcat(char_code_obs, " ");
	   strcat(char_code_obs, "8");
	   strcat(char_code_obs, char_Nh);
	   strcat(char_code_obs, char_Cl);
	   strcat(char_code_obs, char_Cm);
	   strcat(char_code_obs, char_Ch);
   } // if (message_mode != "AUX")
   else
   {
	   strcat(char_code_obs, " ");
	   strcat(char_code_obs, "8////");
   }

	// 222DsVs toevoegen (verplichte groep)
	strcat(char_code_obs, " ");
	strcat(char_code_obs, "222");
	strcat(char_code_obs, char_Ds);
	strcat(char_code_obs, char_Vs);

	// 0SsTwTwTw toevoegen (geen verplichte groep)
	if (num_TwTwTw != MAXINT)
	{
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "0");
		strcat(char_code_obs, char_Sn_TwTwTw);
		strcat(char_code_obs, char_TwTwTw);
	} // if (num_TwTwTw != MAXINT)
   else
   {
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "0////");
   }

	// verplichte groep (kan wel verschillen afh. van geschatte of gemeten golven) wordt gebruikt door ship en fixed sea stations als golven gemeten
	if ( ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_shipborne) == 0)) ||
        ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_buoy) == 0)) ||
        ((station_type.compare("fixed") == 0) && (golven_resultaat.compare(waves_measured_other) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_shipborne) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_buoy) == 0)) ||
        ((station_type.compare("ship") == 0) && (golven_ship_resultaat.compare(waves_measured_other) == 0)) )
	{
		// 1PwaPwaHwaHwa toevoegen (verplichte groep) WORFT ONDER LABEL 2PwPwHwHw gezet voor deze code log
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "1");              // eigenlijk een 2 als kencijfer maar voor deze code log in de groep 2PwPwHwHw
		strcat(char_code_obs, char_Pwa);
		strcat(char_code_obs, char_Hwa);

		// 3dw1dw1dw2dw2 toevoegen (altijd //// voor gemeten golven)
      strcat(char_code_obs, " ");
	   strcat(char_code_obs, "3////");

		// 4Pw1Pw1Hw1Hw1 toevoegen (altijd //// voor gemeten golven)
      strcat(char_code_obs, " ");
	   strcat(char_code_obs, "4////");

		// 5Pw2Pw2Hw2Hw2 toevoegen (altijd //// voor gemeten golven)
	   strcat(char_code_obs, " ");
	   strcat(char_code_obs, "5////");

	} // if ((station_type.compare("fixed") == 0) && (golven_resultaat.compare("waves_measured_shipborne") == 0)) etc.
	else // dus geschatte golven
	{
		// 2PwPwHwHw toevoegen (verplichte groep; behalve voor SUS, SUP, AUX)
      if (message_mode != "SUS" && message_mode != "SUP" && message_mode != "AUX")
      {
		   strcat(char_code_obs, " ");
		   strcat(char_code_obs, "2");
		   strcat(char_code_obs, char_Pw);
		   strcat(char_code_obs, char_Hw);
      }
      else
      {
		   strcat(char_code_obs, " ");
		   strcat(char_code_obs, "2////");
      }

		// 3dw1dw1dw2dw2 toevoegen (verplichte groep; behalve voor SUS, SUP, AUX)
      if (message_mode != "SUS" && message_mode != "SUP" && message_mode != "AUX")
      {
		   strcat(char_code_obs, " ");
		   strcat(char_code_obs, "3");
		   strcat(char_code_obs, char_Dw1);
		   strcat(char_code_obs, char_Dw2);
      }
      else
      {
		   strcat(char_code_obs, " ");
		   strcat(char_code_obs, "3////");
      }

		// 4Pw1Pw1Hw1Hw1 toevoegen (verplichte groep; behalve voor SUS, SUP, AUX)
      if (message_mode != "SUS" && message_mode != "SUP" && message_mode != "AUX")
      {
		   strcat(char_code_obs, " ");
		   strcat(char_code_obs, "4");
		   strcat(char_code_obs, char_Pw1);
		   strcat(char_code_obs, char_Hw1);
      }
      else
      {
		   strcat(char_code_obs, " ");
		   strcat(char_code_obs, "4////");
      }

		// 5Pw2Pw2Hw2Hw2 toevoegen (verplichte groep; behalve voor SUS, SUP, AUX)
      if (message_mode != "SUS" && message_mode != "SUP" && message_mode != "AUX")
      {
		   strcat(char_code_obs, " ");
		   strcat(char_code_obs, "5");
		   strcat(char_code_obs, char_Pw2);
		   strcat(char_code_obs, char_Hw2);
      }
      else
      {
		   strcat(char_code_obs, " ");
		   strcat(char_code_obs, "5////");
      }
	} // else (dus geschatte golven)

	// IsEsEsRs (geen verplichte groep)
	if ((num_Is != MAXINT) || (num_EsEs != MAXINT) || (num_Rs != MAXINT))
	{
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "6");
		if (num_Is != MAXINT)
			strcat(char_code_obs, char_Is);
		else
			strcat(char_code_obs, "/");

		if (num_EsEs != MAXINT)
			strcat(char_code_obs, char_EsEs);
		else
			strcat(char_code_obs, "//");

		if (num_Rs != MAXINT)
			strcat(char_code_obs, char_Rs);
		else
			strcat(char_code_obs, "/");
	} // if ((num_Is != MAXINT) || (num_EsEs != MAXINT) || (num_Rs != MAXINT))
   else
   {
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "6////");
   }


	// Natte bol temperatuur (8SwTbTbTb) (geen verplichte groep)
	// n.b. nattebol alleen toevoegen als deze gebruikt is om het dewpoint te berekenen
	//      indien het hier niet zo was dan was char_TnTnTn niet 0, 1 of 2 (measured)
	if ((strcmp(char_Sn_TnTnTn, "0") == 0) ||
		 (strcmp(char_Sn_TnTnTn, "1") == 0) ||
		 (strcmp(char_Sn_TnTnTn, "2") == 0))
	{
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "8");
		strcat(char_code_obs, char_Sn_TnTnTn);
		strcat(char_code_obs, char_TnTnTn);
	} // if ((strcmp(char_Sn_TnTnTn etc.
   else
	{
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "8////");
   }


	// ice + ciSibiDizi (geen verplichte groep)
	if ((num_ci != MAXINT) || (num_Si != MAXINT) || (num_bi != MAXINT) ||
		 (num_Di != MAXINT) || (num_zi != MAXINT))
	{
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "ICE");
		strcat(char_code_obs, " ");

		if (num_ci != MAXINT)
			strcat(char_code_obs, char_ci);
		else
			strcat(char_code_obs, "/");

		if (num_Si != MAXINT)
			strcat(char_code_obs, char_Si);
		else
			strcat(char_code_obs, "/");

		if (num_bi != MAXINT)
			strcat(char_code_obs, char_bi);
		else
			strcat(char_code_obs, "/");

		if (num_Di != MAXINT)
			strcat(char_code_obs, char_Di);
		else
			strcat(char_code_obs, "/");

		if (num_zi != MAXINT)
			strcat(char_code_obs, char_zi);
		else
			strcat(char_code_obs, "/");
	} // if ((num_ci != MAXINT etc.
	//fputs ("test", os);    uit stdio.h   \0 wordt niet gecopieerd
   else
   {
		strcat(char_code_obs, " ");
		strcat(char_code_obs, "ICE");
		strcat(char_code_obs, " ");
      strcat(char_code_obs, "/////");
   }

   strcat(char_code_obs, "\n");                         // regelafsluiting

   /* naar file schrijven */
   Schrijven_Code_Log(char_code_obs);
}



void TMyWindow::Schrijven_Code_Log(char code_rec[])
{
	ofstream os;
   ifstream is;
	BOOL file_aanwezig_ok;
	string info;
   string record;
   string last_record = "";


	// controleren of de code log file aanwezig is
	file_aanwezig_ok = FALSE;
	Check_file_aanwezig(file_aanwezig_ok, code_file);

	if (file_aanwezig_ok)                           // immt log
	{
      is.open((turbowin_dir + code_file).c_str(), ios::in);
      if (is)
      {
         do // while(!is.eof());
         {
            record.read_line(is);
            if (!is.eof())
               last_record = record;

         } while(!is.eof());

         is.close();
      } // if (is)
      // N.B.
      // als deze code_file er niet is dan was er al een medeling gedaan via
      // Check_file_aanwezig()

		os.open((turbowin_dir + code_file).c_str(), ios::app);    // open voor append
		if (os)
		{
         // alleen nieuwe obs-code record toevoegen als hij nog niet in de code file aanwezig is !!!
         // dit om dubbelen te vermijden
         //
         // N.B.
         // Bij gehele strings vergelijken zullen ze nooit het zelfde zijn
         // daarom de eerste 50 posities (50 beetje willekeurig in iedergeval zijn hier datum/tijd en pos. bij)
         if ((strlen(last_record.c_str()) > 50) && (strlen(code_rec) > 50))
         {
            if (strncmp(last_record.c_str(), code_rec, 50) != 0)
   		   {
               //::MessageBox(0, "weggeschreven was nog NIET aanwezig", "test", MB_OK);
         	   os.write(code_rec, strlen(code_rec));
            }
         } // if ((strlen(last_record.c_str() > 109) && (strlen(immt_rec) > 109))
         else
            if (last_record == "")            // geheel lege log
                os.write(code_rec, strlen(code_rec));

         os.close();
      } // if (os)
      else
      {
			info = "\0";
			info += "Unable to append: ";
         info += turbowin_dir;
			info += code_file;
			info += "\n";
			MessageBox(info.c_str(), "TurboWin error", MB_OK | MB_ICONEXCLAMATION);
		} // else
	} // if (file_aanwezig_ok)
}



void TMyWindow::Read_IMMT_Log_For_Afstand_En_DatumTijd_Check(int& num_vorige_obs_jaar, int& num_vorige_obs_maand, int& num_vorige_obs_dag, int& num_vorige_obs_uur,
                                                             double& num_vorige_obs_breedte, double& num_vorige_obs_lengte)
{
   ifstream is;
	BOOL file_aanwezig_ok;
   string record;
   string last_record = "";
   string jaar        = "";
   string maand       = "";
   string dag         = "";
   string uur         = "";
   string quadrant    = "";
   string latitude    = "";
   string longitude   = "";
   int num_quadrant;
   int num_latitude;
   int num_longitude;


	/* eerst controleren of de immt log file aanwezig is */
	file_aanwezig_ok = FALSE;
	Check_file_aanwezig(file_aanwezig_ok, immt_file);

   /* laatste record bepalen (sequentieel file doorlezen) */
	if (file_aanwezig_ok)                           // immt file (immt.txt)
	{
      is.open((turbowin_dir + immt_file).c_str(), ios::in);
      if (is)
      {
         do // while(!is.eof());
         {
            record.read_line(is);
            if (!is.eof())
               last_record = record;

         } while(!is.eof());

         is.close();
      } // if (is)


      if (last_record.length() >= 19)
      {
         /* string datum tijd uit laatste record */
         jaar      = last_record.substr(1, 4);
         maand     = last_record.substr(5, 2);
         dag       = last_record.substr(7, 2);
         uur       = last_record.substr(9, 2);

         /* obs code positie uit laatste record */
         quadrant  = last_record.substr(11, 1);                       // WMO code table 3333
         latitude  = last_record.substr(12, 3);                       // tenths of degrees
         longitude = last_record.substr(15, 4);                       // tenths of degrees

         /* datum/tijd omzetten naar num waarden */
         num_vorige_obs_jaar  = atoi(jaar.c_str());
         num_vorige_obs_maand = atoi(maand.c_str());
         num_vorige_obs_dag   = atoi(dag.c_str());
         num_vorige_obs_uur   = atoi(uur.c_str());

         /* positie omzetten naar num_waarden + afronden + "+/-" (afh. quadrant) maken */
         num_quadrant  = atoi(quadrant.c_str());
         num_latitude  = atoi(latitude.c_str());
         num_longitude = atoi(longitude.c_str());

         num_vorige_obs_breedte = (float)num_latitude / 10;                   // tienden
         num_vorige_obs_lengte  = (float)num_longitude / 10;                  // tienden

         if (num_quadrant == 3 || num_quadrant == 5)                   // Zuiderbreedte
         {
            num_vorige_obs_breedte *= -1;
         }
         if (num_quadrant == 5 || num_quadrant == 7)                   // Westerlengte
         {
            num_vorige_obs_lengte *= -1;
         }

      } // if (last_record.length() >= 19)
   } // if (file_aanwezig_ok)
}


///////////

int TMyWindow::Bepaal_afstand_huidige_obs_pos_tot_vorige_obs_pos(double num_vorige_obs_breedte, double num_vorige_obs_lengte)
{
	/* gebruikte formule :                                                   */
	/* cos_AB = sin_bA * sin_bB + cos_bA * cos_bB * cos_delta_l_AB           */
	/*                                                                       */
	/* afstand = 60 arccos(cos_AB)                                           */
	/*                                                                       */
	/* om van graden hoek radialen hoek te maken : graden * 60 * boogminuut  */

	int afstand;
	//char *endptr;
	double delta_l_ab;
	double cos_delta_l_ab;
	double sin_breedte_a;
	double sin_breedte_b;
	double cos_breedte_a;
	double cos_breedte_b;
	double num_huidige_obs_breedte;
	double num_huidige_obs_lengte;
   double acos_argument;
	afstand = MAXINT;                               // initialisatie
   bool huidige_obs_in_por;


   /* NB niet werken met obs_LongitudeDegrees etc. want deze zijn niet bekend in de CLASSIC mode */
   num_huidige_obs_breedte = (float)num_LaLaLa / 10;
   num_huidige_obs_lengte = (float)num_LoLoLoLo / 10;

   if (obs_North_or_South == "S")                 // Zuiderbreedte (is ook bekend in classic mode|)
		num_huidige_obs_breedte *= -1;

   if (obs_East_or_West == "W")
      num_huidige_obs_lengte *= -1;


   /* i.v.m. 180 graden passage in de POR */
	//if ((num_huidige_obs_lengte >= westgrens_POR) && (num_huidige_obs_lengte <= oostgrens_POR)) // komt hier nooit in ????

   if ( (num_huidige_obs_lengte >= westgrens_POR && num_huidige_obs_lengte <= 180) ||      // westgrens_por +90
        (num_huidige_obs_lengte >= -180 && num_huidige_obs_lengte <= oostgrens_POR) )      // oostgrens_por -90
		huidige_obs_in_por = true;
   else
      huidige_obs_in_por = false;

	if (huidige_obs_in_por == true)
	{
///////////
//		MessageBox("huidige obs in POR", "TurboWin tussenstand", MB_OK);
////////////
		if (num_huidige_obs_lengte < 0) num_huidige_obs_lengte += 360;
		if (num_vorige_obs_lengte < 0) num_vorige_obs_lengte += 360;
	} // if (les_file == por_file)


	/* lengte verschil bepalen */
	delta_l_ab = num_huidige_obs_lengte - num_vorige_obs_lengte;


	/* lengteverschil > 180 niet berekenen maar een MAXINT geven */
	/* > 180 geeft namelijk problemen met formules */
	if (fabs(delta_l_ab) > 180)
      afstand = MAXINT;
	else // lengteverschil < 180
	{
		/* de (grootcircel)berekening */
		sin_breedte_a = sin(num_vorige_obs_breedte  * 60 * boogminuut);
		sin_breedte_b = sin(num_huidige_obs_breedte * 60 * boogminuut);
		cos_breedte_a = cos(num_vorige_obs_breedte  * 60 * boogminuut);
		cos_breedte_b = cos(num_huidige_obs_breedte * 60 * boogminuut);
		cos_delta_l_ab = cos(delta_l_ab * 60 * boogminuut);

      /* acos argument eerst testen om ACOS domain error te voorkomen (bij 2x precies zelfde positie achterelkaar + obs to screen)*/
      acos_argument = sin_breedte_a * sin_breedte_b + cos_breedte_a * cos_breedte_b * cos_delta_l_ab;
      if (acos_argument <= -1 || acos_argument >= 1)
         afstand = 0;
      else
      	afstand = floor(h180pi * acos(acos_argument) + 0.5);
	} // else (lengteverschil < 180)

/////////////////////
//   char buffer[10];
//   sprintf(buffer, "%d", afstand);
//   MessageBox(buffer, "test afstand", MB_OK);
//////////////////


	return afstand;
}


void TMyWindow::Update_StatusBar_Obs()
{
   BOOL level_1_ok = TRUE;
   bool statusbar_mode = true;
   char local_char_obs[512];
   bool national_groups_present = false;

   /* initialisatie */
   local_char_obs[0] = '\0';

   /* bepalen of er national groups zijn */
   if (num_national_1 != MAXINT || num_national_2 != MAXINT || num_national_3 != MAXINT || num_national_4 != MAXINT || num_national_5 != MAXINT)
      national_groups_present = true;

   // NB statusbar wordt niet geupdate als of call sign en/of datum/tijd en/of positie niet aanwezig is
   checking_level_1(level_1_ok, statusbar_mode);

   if (level_1_ok == TRUE)                    // dan in ieder geval call sign + datum/tijd + positie aanwezig
   {
      samenstellen_te_verzenden_obs();        // hier worden char_obs_1, char_obs_2 en char_obs_3 gevuld

      strcpy(local_char_obs, char_obs_1);
      strcat(local_char_obs, " ");
      strcat(local_char_obs, char_obs_2);

      if (national_groups_present)
      {
         strcat(local_char_obs, " ");
         strcat(local_char_obs, char_obs_3);
      }

      /* status bar updaten */
      TStatusBar* sb = TYPESAFE_DOWNCAST(GetApplication() -> GetMainWindow() -> ChildWithId(IDW_STATUSBAR),TStatusBar);
      sb -> SetText(local_char_obs);

   } // if (level_1_ok == TRUE) 

   // TESTEN OP WAT ER GEBEURD OP DE TIMER (NA EEN HALF UUR EN NA EEN UUR

}




void TMyWindow::Check_A_drive_Op_Disk_Voor_Obs_Output(bool& doorgaan)
{
   int save_disk;
   char save_cwd[512];                        //


   doorgaan = true;

   /* getdisk, getcwd, setdisk en chdir nodig omdat na aanroepen van deze functie en hierna aanroepen van */
   /* b.v. utility Ocean Wave Atlas anders OWA zijn sub-directories (+files) niet meer kan vinden */

   /* via chdir (truckje) testen of er een disk in de A drive zit */
   /* dit om te voorkomen dat als er geen disk in A zit er 5/6 keer een systeem message wordt gegeven */
   save_disk = getdisk();                        // save current drive
   getcwd(save_cwd, 512);                        // save current working dir

   if (chdir("a:") == -1)
   {
      MessageBox("No disk in A: drive. Operation cancelled.", "TurboWin error", MB_OK | MB_ICONERROR);
      doorgaan = false;                  // als dit mislukt geeft het os via een keurige messagebox een error melding
   }

   setdisk(save_disk);                           // retrieve "gesavede" current drive
   chdir(save_cwd);                              // retrieve "gesavede" current working dir
}







