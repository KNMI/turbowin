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


#include "TMyWindo.h"



void TMyWindow::Temperature_Dialog_Fields_Invalid()
{
	obs_Airtemp                  = "\0";
	obs_Airtemptienden           = "\0";
	obs_Wetbulb                  = "\0";
	obs_Wetbulbtienden           = "\0";
	obs_Rh                       = "\0";
	obs_Seawater                 = "\0";
	obs_Seawatertienden          = "\0";

	char_obs_Tdew[0]             = '\0';

	char_TTT[0]                  = '\0';              num_TTT         = MAXINT;
	char_Sn_TTT[0]               = '\0';
	char_TnTnTn[0]               = '\0';              num_TnTnTn      = MAXINT;
	char_Sn_TnTnTn[0]            = '\0';
	char_TwTwTw[0]               = '\0';              num_TwTwTw      = MAXINT;
	char_Sn_TwTwTw[0]            = '\0';
	char_UUU[0]                  = '\0';              num_UUU         = MAXINT;
	char_Sn_TdTdTd[0]            = '\0';
	char_TdTdTd[0]               = '\0';              num_TdTdTd      = MAXINT;
   char_Rh[0]                   = '\0';
   char_Rhi[0]                  = '\0';

   seawater_method              = "\0";
   exposure                     = "\0";

}



void TMyWindow::CmShipTemperatures()
{
  int result;
  char* endptr;
  char* endptr2;
  temperatures_ok = FALSE;
  BOOL Airtemp_ok = TRUE;
  BOOL Wetbulb_ok = TRUE;
  BOOL Rh_ok = TRUE;
  BOOL Seawater_ok = TRUE;
  int num_Tdry;
  int num_Tdry_tienden;
  float num_Tdry_total;
  int num_Twet;
  int num_Twet_tienden;
  float num_Twet_total;
  int num_Tsea;
  int num_Tsea_tienden;
  float num_Tsea_total;
  int num_Rh;                                      // (omgezette) ingevulde rel. hum.  (0 - 100%)
  float num_rv = MAXINT;                           // return berekende rel. hum.        (0 - 1)
  float num_Tdew = MAXINT;                         // return van dewpoint berekening
  BOOL ijs_wetbulb;
  string teken_Tdry;
  string teken_Twet;
  string teken_Tsea;
  bool gebruik_default_temp;                       // voor event. herleiden luchtdruk naar sea level



  Read_Temp_Settings();                            // uitlezen exposure en zeewater methode

  TTemperaturesDialog* dlg = new TTemperaturesDialog(this, IDD_TEMPERATURES, temperaturestransfer, file_seawater_method, file_exposure, message_mode, turbowin_dir, in_next_sequence, recruiting_country);

  /* caption input form zetten (alleen supplementary en auxiliary) */
  if (message_mode == "SUS")
     dlg -> SetCaption("Temperatures [supplementary ship with sst]");
  else if (message_mode == "SUP")
     dlg -> SetCaption("Temperatures [supplementary ship without sst]");
  else if (message_mode == "AUX")
     dlg -> SetCaption("Temperatures [auxiliary ship]");
  else
     dlg -> SetCaption("Temperatures");


  do // while(temperatures_ok == FALSE && result != IDCANCEL);
  {

     if ((result = dlg -> Execute()) == IDOK)
	  {
		  /* temperatures transfer buffer uilezen */
		  obs_Airtemp         = temperaturestransfer.AirtempResult.GetSelection();
		  obs_Airtemptienden  = temperaturestransfer.AirtemptiendenResult.GetSelection();
		  obs_Wetbulb         = temperaturestransfer.WetbulbResult.GetSelection();
		  obs_Wetbulbtienden  = temperaturestransfer.WetbulbtiendenResult.GetSelection();
		  obs_Rh              = temperaturestransfer.RhResult.GetSelection();
		  obs_Seawater        = temperaturestransfer.SeawaterResult.GetSelection();
		  obs_Seawatertienden = temperaturestransfer.SeawatertiendenResult.GetSelection();


        /* exposure of thermometers uitlezen (dewpoint berekeningsformule dan iets anders) */
        if (temperaturestransfer.TempRadioButtonResult9 == BF_CHECKED)
           exposure = SLING;
        else if (temperaturestransfer.TempRadioButtonResult10 == BF_CHECKED)
           exposure = SCREEN;
        else                                   // default
           exposure = SLING;

        /* seawater method uitlezen */
        if (temperaturestransfer.TempRadioButtonResult4 == BF_CHECKED)
           seawater_method = INTAKE;
		  else if (temperaturestransfer.TempRadioButtonResult5 == BF_CHECKED)
           seawater_method = BUCKET;
		  else if (temperaturestransfer.TempRadioButtonResult6 == BF_CHECKED)
           seawater_method = HULL_CONTACT_SENSOR;
		  else if (temperaturestransfer.TempRadioButtonResult7 == BF_CHECKED)
           seawater_method = OTHER;
		  else if (temperaturestransfer.TempRadioButtonResult8 == BF_CHECKED)
           seawater_method = "\0";

        /* voor de zekerheid net opgehaalde waarden bij sommige message_modes op NULL zetten (o.a. voor meteo progress dialog) */
        if (message_mode == "AUX" || message_mode == "SUS" || message_mode == "SUP")
        {
		     obs_Wetbulb         = "\0";
		     obs_Wetbulbtienden  = "\0";
		     obs_Rh              = "\0";
        }
        if (message_mode == "AUX" || message_mode == "SUP")
        {
           seawater_method     = "\0";
		     obs_Seawater        = "\0";
		     obs_Seawatertienden = "\0";
        }
        if (message_mode == "AUX")
        {
           //exposure = SLING;             // alleen van belang om het dauwpunt te berekenen  (gebeurd bij AUX toch niet)
           obs_Airtemptienden  = "\0";
        }


		  /* N.B. de state of wet bulb  uitlezen gebeurd verderop */

		  /* om te voorkomen dat na een eenmaal uitgevoerde TDew berekening */
		  /* deze altijd blijft staan (b.v. als je TWet en Rh niet invuld   */
		  /* dan is er geen nieuwe TDew meer                                */
		  strcpy(char_obs_Tdew, "\0");
		  num_TdTdTd = MAXINT;
		  strcpy(char_TdTdTd, "\0");
		  strcpy(char_Sn_TdTdTd, "\0");
        strcpy(char_Rh, "\0");
        strcpy(char_Rhi, "\0");

		  //
		  // Air temperature
		  //
        if (message_mode == "AUX")                           // heeft geen tienden gerapporteerd
        {
           if (obs_Airtemp.compare("\0") == 0)
           {
              strcpy(char_TTT, "\0");
              strcpy(char_Sn_TTT, "\0");
              num_TTT = MAXINT;
              Airtemp_ok = TRUE;
           }
           else // geen leeg graden veld
           {
              num_Tdry = int(strtol(obs_Airtemp.c_str(), &endptr, 10));
              if (*endptr == NULL)
              {
                 num_Tdry_total = num_Tdry;                           // hier zijn geen tienden
                 if (num_Tdry_total >= -50.0 && num_Tdry_total <= 60.0)
                 {
                    if (num_Tdry_total < 0.0)
                       strcpy(char_Sn_TTT, "1");
                    else // dus num_Tdry_total >= 0.0
                       strcpy(char_Sn_TTT, "0");

                    sprintf(char_TTT, "%02d", abs(num_Tdry_total));   // in hele graden
                    strncpy(char_TTT + 2, "/", 1);                    // "/" op laatste positie
                    char_TTT[3] = '\0';                               // voor de zekerheid

                    num_TTT = num_Tdry_total * 10;                    // voor uniformitiet in verdere -level 3- checks en herleiding luchtdruk (nu in tienden graden)
                    Airtemp_ok = TRUE;

                 } // if (num_Tdry_total >= -50.0 && num_Tdry_total <= 60.0)
                 else // dus dan buiten range
                 {
                    MessageBox("Air temperature must be in range -50 - 60", "TurboWin error", MB_OK | MB_ICONWARNING);
                    Airtemp_ok = FALSE;
                    Temperature_Dialog_Fields_Invalid();
                 }
              } // if (*endptr == NULL)
              else // dus dan foutief karakter
              {
                 MessageBox("Air temperature must be in range -50 - 60", "TurboWin error", MB_OK | MB_ICONWARNING);
                 Airtemp_ok = FALSE;
                 Temperature_Dialog_Fields_Invalid();
              } // else (dus dan foutief karakter
           } // else (geen leeg graden veld)

        } // if (message_mode == "AUX")
        else // geen AUX
        {
           if ((obs_Airtemp.compare("\0") == 0) && (obs_Airtemptienden.compare("\0") == 0))
           {
              strcpy(char_TTT, "\0");
              strcpy(char_Sn_TTT, "\0");
              num_TTT = MAXINT;
              Airtemp_ok = TRUE;
           }
           else  // dus dan geen 2 lege velden
           {
              if ( (obs_Airtemp.compare("\0") == 0 && obs_Airtemptienden.compare("\0") != 0) ||
                   (obs_Airtemp.compare("\0") != 0 && obs_Airtemptienden.compare("\0") == 0) )
              {
                 MessageBox("Both air temperature fields must be filled in or both must be blank", "TurboWin error", MB_OK | MB_ICONWARNING);
                 Airtemp_ok = FALSE;
                 Temperature_Dialog_Fields_Invalid();
              }
              else // dus dan 2 velden ingevuld
              {
                 num_Tdry = int(strtol(obs_Airtemp.c_str(), &endptr, 10));
                 num_Tdry_tienden = int(strtol(obs_Airtemptienden.c_str(), &endptr2, 10));
                 if (*endptr == NULL && *endptr2 == NULL)
                 {
                    // tienden kan ook hondersten zijn
                    // dus bij i cijfer in tienden veld delen door 10
                    // bij 2 cijfers in tienden veld delen door 100
                    // etc.
                    if (num_Tdry < 0)
                       num_Tdry_total = num_Tdry - float(num_Tdry_tienden) / (pow(10.0, obs_Airtemptienden.length()));
                    else if (num_Tdry > 0)
                       num_Tdry_total = num_Tdry + float(num_Tdry_tienden) / (pow(10.0, obs_Airtemptienden.length()));
                    else // dus 0
                    {
                       // 0 kan zijn "-0" of '0"
                       teken_Tdry = "\0";
                       teken_Tdry = obs_Airtemp.substr(0, 1);
                       if (teken_Tdry.compare("-") == 0)
                          num_Tdry_total = num_Tdry - float(num_Tdry_tienden) / (pow(10.0, obs_Airtemptienden.length()));
                       else
                          num_Tdry_total = num_Tdry + float(num_Tdry_tienden) / (pow(10.0, obs_Airtemptienden.length()));
                    } // else (dus 0)

                    if (num_Tdry_total >= -50.0 && num_Tdry_total <= 60.0)
                    {
                       if (num_Tdry_total < 0.0)
                       {
                          strcpy(char_Sn_TTT, "1");
                          num_TTT = num_Tdry_total * 10 - 0.5;
                       }
                       else // dus num_Tdry_total >= 0.0
                       {
                          strcpy(char_Sn_TTT, "0");
                          num_TTT = num_Tdry_total * 10 + 0.5;   // num_TTT dus in tienden
                       } // else (dus num_Tdry_total >= 0.0)
                       sprintf(char_TTT, "%03d", abs(num_TTT));
                       Airtemp_ok = TRUE;

                    } // if (num_Tdry_total >= -50.0 && num_Tdry_total <= 60.0)
                    else // dus dan buiten range
                    {
                       MessageBox("Air temperature must be in range -50.0 - 60.0", "TurboWin error", MB_OK | MB_ICONWARNING);
                       Airtemp_ok = FALSE;
                       Temperature_Dialog_Fields_Invalid();
                    }
                 } // if (*endptr == NULL && *endptr2 == NULL)
                 else // dus dan foutief karakter
                 {
                    MessageBox("Air temperature must be in range -50.0 - 60.0", "TurboWin error", MB_OK | MB_ICONWARNING);
                    Airtemp_ok = FALSE;
                    Temperature_Dialog_Fields_Invalid();
                 }
              } // else (dus dan 2 velden ingevuld)
           } // else (dus dan geen 2 lege velden)
        } // else (geen AUX)

		  //
		  // Wet-bulb temperature
		  //
		  if ((obs_Wetbulb.compare("\0") == 0) && (obs_Wetbulbtienden.compare("\0") == 0))
		  {
			  strcpy(char_TnTnTn, "\0");
			  strcpy(char_Sn_TnTnTn, "\0");
			  num_TnTnTn = MAXINT;
			  Wetbulb_ok = TRUE;
           ijs_wetbulb = FALSE;
		  }
		  else // dus geen 2 lege velden
		  {
			  if ( (obs_Wetbulb.compare("\0") == 0 && obs_Wetbulbtienden.compare("\0") != 0) ||
					 (obs_Wetbulb.compare("\0") != 0 && obs_Wetbulbtienden.compare("\0") == 0) )
			  {
				  MessageBox("Both wet-bulb temperature fields must be filled in or both must be blank", "TurboWin error", MB_OK | MB_ICONWARNING);
				  Wetbulb_ok = FALSE;
				  Temperature_Dialog_Fields_Invalid();
			  }
			  else // dus dan 2 velden ingevuld
			  {
				  num_Twet = int(strtol(obs_Wetbulb.c_str(), &endptr, 10));
				  num_Twet_tienden = int(strtol(obs_Wetbulbtienden.c_str(), &endptr2, 10));
				  if (*endptr == NULL && *endptr2 == NULL)
				  {
					  if (num_Twet < 0)
						  num_Twet_total = num_Twet - float(num_Twet_tienden) / (pow(10.0, obs_Wetbulbtienden.length()));
					  else if (num_Twet > 0)
						  num_Twet_total = num_Twet + float(num_Twet_tienden) / (pow(10.0, obs_Wetbulbtienden.length()));
					  else // dus 0
					  {
						  // 0 kan zijn "-0" of '0"
						  teken_Twet = "\0";
						  teken_Twet = obs_Wetbulb.substr(0, 1);
						  if (teken_Twet.compare("-") == 0)
							  num_Twet_total = num_Twet - float(num_Twet_tienden) / (pow(10.0, obs_Wetbulbtienden.length()));
						  else
							  num_Twet_total = num_Twet + float(num_Twet_tienden) / (pow(10.0, obs_Wetbulbtienden.length()));
					  } // else (dus 0)

					  if (num_Twet_total >= -50.0 && num_Twet_total <= 50.0)
					  {
						  if ((temperaturestransfer.TempRadioButtonResult1 == BF_UNCHECKED) &&
							  (temperaturestransfer.TempRadioButtonResult2 == BF_UNCHECKED))
						  {
							  MessageBox("State of wet-bulb must be checked ('not determined' can only be checked if wet-bulb fields are blank)", "TurboWin error", MB_OK | MB_ICONWARNING);
							  Wetbulb_ok = FALSE;
							  Temperature_Dialog_Fields_Invalid();
						  }

                    else if ((num_Twet_total > 0.0) &&  (temperaturestransfer.TempRadioButtonResult2 == BF_CHECKED))
                    {
							  MessageBox("Combination 'wet-bulb temperature > 0.0°C' + 'frozen wet-bulb state' not possible", "TurboWin error", MB_OK | MB_ICONWARNING);
							  Wetbulb_ok = FALSE;
							  Temperature_Dialog_Fields_Invalid();
						  }


						  else // dus state of wet bulb is checked (frozen or not frozen)
						  {
							  if (temperaturestransfer.TempRadioButtonResult2 == BF_CHECKED)
							  {
								  strcpy(char_Sn_TnTnTn, "2");    // frozen wet bulb
								  ijs_wetbulb = TRUE;
							  } // if (temperaturestransfer.TempRadioButtonResult2 == BF_CHECKED)
							  else if (temperaturestransfer.TempRadioButtonResult1 == BF_CHECKED)
							  {
								  if (num_Twet_total < 0)
								  {
									  strcpy(char_Sn_TnTnTn, "1");    // negative wet-bulb
									  ijs_wetbulb = FALSE;
								  }
								  else
								  {
									  strcpy(char_Sn_TnTnTn, "0");    // positive wet-bulb
									  ijs_wetbulb = FALSE;
								  }
							  } // else if (temperaturestransfer.TempRadioButtonResult1 == BF_CHECKED)

							  if (num_Twet_total >= 0.0)
								  num_TnTnTn = num_Twet_total * 10 + 0.5;
							  else
								  num_TnTnTn = num_Twet_total * 10 - 0.5;

							  sprintf(char_TnTnTn, "%03d", abs(num_TnTnTn));
							  Wetbulb_ok = TRUE;
						  } // else (dus state of wet bulb checked etc.
					  } // if (num_Twet_total >= -50.0 && num_Twet_total <= 50.0)
					  else // dus dan buiten range
					  {
						  MessageBox("Wet-bulb temperature must be in range -50.0 - 50.0", "TurboWin error", MB_OK | MB_ICONWARNING);
						  Wetbulb_ok = FALSE;
						  Temperature_Dialog_Fields_Invalid();
					  }
				  } // if (*endptr == NULL && *endptr2 == NULL)
				  else // dus dan foutief karakter
				  {
					  MessageBox("Wet-bulb temperature must be in range -50.0 - 50.0", "TurboWin error", MB_OK | MB_ICONWARNING);
					  Wetbulb_ok = FALSE;
					  Temperature_Dialog_Fields_Invalid();
				  }
			  } // else (dus dan 2 velden ingevuld)
		  } // else (dus geen 2 lege velden)

		  //
		  // onderlinge temperatuur controle
		  //

        /* N.B. als num_Tdry_total > 0.0 kan er ook ijs aan de natte bol zitten !!!! */


		  //
		  // Seawater temperature
		  //
		  if ((obs_Seawater.compare("\0") == 0) && (obs_Seawatertienden.compare("\0") == 0))
		  {
			  strcpy(char_TwTwTw, "\0");
			  strcpy(char_Sn_TwTwTw, "\0");
			  num_TwTwTw = MAXINT;
			  Seawater_ok = TRUE;
		  }
		  else // dus dan geen 2 lege velden
		  {
			  if ( (obs_Seawater.compare("") == 0 && obs_Seawatertienden.compare("") != 0) ||
					 (obs_Seawater.compare("") != 0 && obs_Seawatertienden.compare("") == 0) )
			  {
				  MessageBox("Both seawater temperature fields must be filled in or both must be blank", "TurboWin error", MB_OK | MB_ICONWARNING);
				  Seawater_ok = FALSE;
				  Temperature_Dialog_Fields_Invalid();
			  }
			  else // dus dan 2 velden ingevuld
			  {
				  num_Tsea = int(strtol(obs_Seawater.c_str(), &endptr, 10));
				  num_Tsea_tienden = int(strtol(obs_Seawatertienden.c_str(), &endptr2, 10));
				  if (*endptr == NULL && *endptr2 == NULL)
				  {
					  if (num_Tsea < 0)
						  num_Tsea_total = num_Tsea - float(num_Tsea_tienden) / (pow(10.0, obs_Seawatertienden.length()));
					  else if (num_Tsea > 0)
						  num_Tsea_total = num_Tsea + float(num_Tsea_tienden) / (pow(10.0, obs_Seawatertienden.length()));
					  else  // dus 0
					  {
						  // 0 kan zijn "-0" of '0"
						  teken_Tsea = "\0";
						  teken_Tsea = obs_Seawater.substr(0, 1);
						  if (teken_Tsea.compare("-") == 0)
							  num_Tsea_total = num_Tsea - float(num_Tsea_tienden) / (pow(10.0, obs_Seawatertienden.length()));
						  else
							  num_Tsea_total = num_Tsea + float(num_Tsea_tienden) / (pow(10.0, obs_Seawatertienden.length()));
					  } // else (dus 0)


					  if (num_Tsea_total >= -2.0 && num_Tsea_total <= 50.0)
					  {
						  //if ((temperaturestransfer.TempRadioButtonResult4 == BF_UNCHECKED) &&
						  //		(temperaturestransfer.TempRadioButtonResult5 == BF_UNCHECKED) &&
						  //		(temperaturestransfer.TempRadioButtonResult6 == BF_UNCHECKED) &&
						  //		(temperaturestransfer.TempRadioButtonResult7 == BF_UNCHECKED))

                    if ((seawater_method != INTAKE) && (seawater_method != BUCKET) &&
                        (seawater_method != HULL_CONTACT_SENSOR) && (seawater_method != OTHER))
 						  {
							  MessageBox("Seawater temperature type of measurement button must be checked ('not determined' can only be checked if seawater temp. fields are blank)", "TurboWin error", MB_OK | MB_ICONWARNING);
							  Seawater_ok = FALSE;
							  Temperature_Dialog_Fields_Invalid();
						  }
						  else // seawater method bekend
						  {
							  // zeewater temperatuur >= 0
							  if ((num_Tsea_total >= 0) && (seawater_method == INTAKE))
								  strcpy(char_Sn_TwTwTw, "0");
							  else if ((num_Tsea_total >= 0) && (seawater_method == BUCKET))
								  strcpy(char_Sn_TwTwTw, "2");
							  else if ((num_Tsea_total >= 0) && (seawater_method == HULL_CONTACT_SENSOR))
								  strcpy(char_Sn_TwTwTw, "4");
							  else if ((num_Tsea_total >= 0) && (seawater_method == OTHER))
								  strcpy(char_Sn_TwTwTw, "6");

							  // zeewatertemperatuur < 0
							  if ((num_Tsea_total < 0) && (seawater_method == INTAKE))
								  strcpy(char_Sn_TwTwTw, "1");
							  else if ((num_Tsea_total < 0) && (seawater_method == BUCKET))
								  strcpy(char_Sn_TwTwTw, "3");
							  else if ((num_Tsea_total < 0) && (seawater_method == HULL_CONTACT_SENSOR))
								  strcpy(char_Sn_TwTwTw, "5");
							  else if ((num_Tsea_total < 0) && (seawater_method == OTHER))
								  strcpy(char_Sn_TwTwTw, "7");

							  if (num_Tsea_total >= 0.0)
								  num_TwTwTw = num_Tsea_total * 10 + 0.5;       // coderen
							  else
								  num_TwTwTw = num_Tsea_total * 10 - 0.5;       // coderen

							  sprintf(char_TwTwTw, "%03d", abs(num_TwTwTw));
							  Seawater_ok = TRUE;
						  } // else (dus dan button checked)
					  } // if (num_Tsea_total >= -2.0 && num_Tsea_total <= 50.0)
					  else // dus dan buiten range
					  {
						  MessageBox("Seawater temperature must be in range -2.0 - 50.0", "TurboWin error", MB_OK | MB_ICONWARNING);
						  Seawater_ok = FALSE;
						  Temperature_Dialog_Fields_Invalid();
					  }
				  } //if (*endptr == NULL && *endptr2 == NULL)
				  else // dus dan foutief karakter
				  {
					  MessageBox("Seawater temperature must be in range -2.0 - 50.0", "TurboWin error", MB_OK | MB_ICONWARNING);
					  Seawater_ok = FALSE;
					  Temperature_Dialog_Fields_Invalid();
				  }
			  } // else (dus dan 2 velden ingevuld)
		  } // else (dus dan geen 2 lege velden)


		  //
		  // relatieve vochtigheid
		  //
		  if (obs_Rh.compare("\0") == 0)
		  {
			  strcpy(char_UUU, "\0");
			  num_UUU = MAXINT;
			  Rh_ok = TRUE;
           strcpy(char_Rh, "\0");
           strcpy(char_Rhi, "\0");
		  }
		  else // dus dan veld niet leeg
		  {
			  num_Rh = int(strtol(obs_Rh.c_str(), &endptr, 10));
			  if (*endptr == NULL)
			  {
				  if (num_Rh >= 1 && num_Rh <= 100)
				  {
					  sprintf(char_UUU, "%3d", num_Rh);
                 sprintf(char_Rh, "%04d", num_Rh);               // voor (vanaf) immt-4
                 strcpy(char_Rhi, "1");                          // voor (vanaf) immt-4   Rhi=1 -> rv in whole percentage measured and originally reported
					  Rh_ok = TRUE;
				  }
				  else // dus dan buiten range
				  {
					  MessageBox("Relative Humidity must be in range 1 - 100 or blank", "TurboWin error", MB_OK | MB_ICONWARNING);					  Rh_ok = FALSE;
					  Temperature_Dialog_Fields_Invalid();
				  }
			  } // if (*endptr == NULL)
			  else // dus dan foutief karakter
			  {
				  MessageBox("Relative Humidity must be in range 1 - 100 or blank", "TurboWin error", MB_OK | MB_ICONWARNING);
				  Rh_ok = FALSE;
				  Temperature_Dialog_Fields_Invalid();
			  }
		  } // else (dus dan veld niet leeg)


        //
        // onderlinge temperatuur controles
        //
		  if (Airtemp_ok == TRUE && Wetbulb_ok == TRUE && Seawater_ok == TRUE && Rh_ok == TRUE)
		  {

			  // controleren of de airtemp >= wetbulb  is
			  if ((obs_Airtemp.compare("") != 0) && (obs_Wetbulb.compare("") != 0))
			  {
				  if (num_Tdry_total >= num_Twet_total)
				  {
					  // dewpoint e.d. berekenen
					  Vochtigheid* v;
					  v = new Vochtigheid();
					  //v -> Bereken_Dauwpunt_en_RV(num_Tdry_total, num_Twet_total, num_rv, num_Tdew, ijs_wetbulb, exposure, obs_MSL_Pressure);
                 v -> Bereken_Dauwpunt_en_RV(num_Tdry_total, num_Twet_total, num_rv, num_Tdew, ijs_wetbulb, exposure);
					  delete v;

					  if (num_Tdew != MAXINT)
					  {
						  sprintf(char_obs_Tdew, "%01.1f", num_Tdew); // Tdew in graden + tienden b.v. 13.9
						  /* num_Tdew = na berekening in class Vochtigheid  b.v. 13.37 */
						  /* char_obs_Tdew is voor de meteo progress dialog  b.v. 13.4 */

						  if (num_Tdew >= 0.0)
						  {
							  num_TdTdTd = int(num_Tdew * 10 + 0.5);       // num_TdTdTd in tienden
							  strcpy(char_Sn_TdTdTd, "0");
						  }
						  else // dus Tdew < 0.0
						  {
							  num_TdTdTd = int(num_Tdew * 10 - 0.5);       // num_TdTdTd in tienden
							  strcpy(char_Sn_TdTdTd, "1");
						  }  // else (dus Tdew < 0.0)
						  /* num_TdTdTd is voor level checking later b.v. 134 (=13.4 °C) */

						  sprintf(char_TdTdTd, "%03d", abs(num_TdTdTd));
						  /* char_TdTdTd is de gecodeerde dewpoints waarde b.v. "134" */

					  } // if (num_Tdew != MAXINT)
					  else // dus ongeldig dewpoint
					  {
						  strcpy(char_obs_Tdew, "invalid");
						  num_TdTdTd = MAXINT;
						  strcpy(char_TdTdTd, "\0");
						  strcpy(char_Sn_TdTdTd, "\0");
					  } // else (dus ongeldig dewpoint)


					  if (num_rv != MAXINT)
					  {
						  num_UUU = int(num_rv * 100 + 0.5);
						  sprintf(char_UUU, "%d", num_UUU);            // voor progress screen
                    sprintf(char_Rh, "%04d", num_UUU);           // voor (vanaf) IMMT-4
                    strcpy(char_Rhi, "4");                       // voor (vanaf) IMMT-4     NB Rhi=4: rv in whole percentage computed
					  } // if (num_rv != MAXINT)
					  else // dus ongeldig r.v.
					  {
						  strcpy(char_UUU, "invalid");
						  num_UUU = MAXINT;
                    strcpy(char_Rh, "\0");
                    strcpy(char_Rhi, "\0");
					  } // else (dus ongeldig r.v.)
					  temperatures_ok = TRUE;
				  } // if (num_Tdry_total >= num_Twet_total)
				  else  // dus dan twet > tdry
				  {
				  	  MessageBox("Air temperature must be >= wet-bulb temperature", "TurboWin error", MB_OK | MB_ICONWARNING);
				  	  temperatures_ok = FALSE;
				  	  Temperature_Dialog_Fields_Invalid();
				  } // else (dus dan twet > tdry)
			  } // if ((obs_Airtemp.compare("") != 0) && (obs_Wetbulb.compare("") != 0))
			  else // niet beide (twet + tdry) ingevuld
			  {
				  // Tdry en rv ingevuld
				  if ((obs_Airtemp.compare("") != 0) && (obs_Rh.compare("") != 0))
				  {
					  // om te voorkomen dat de natte bol groep (8SnTbTbTb) wordt opgenomen
					  //strcpy(char_TnTnTn, "\0");           // reset
					  //strcpy(char_Sn_TnTnTn, "\0");        // reset
					  //num_TnTnTn = MAXINT;                 // reset


					  // dewpoint en wetbulb temp berekenen
					  Vochtigheid* v;
					  v = new Vochtigheid();
					  num_Tdew = v -> Bereken_Dauwpunt(num_Rh, num_Tdry_total);
					  num_Twet_total = v -> Bereken_NatteBol(num_Rh, num_Tdry_total);
					  delete v;

					  if (num_Tdew != MAXINT)
					  {
						  sprintf(char_obs_Tdew, "%01.1f", num_Tdew);    // Tdew in graden + tienden b.v. 13.9
						  /* num_Tdew = na berekening in class Vochtigheid  b.v. 13.37 */
						  /* char_obs_Tdew is voor de meteo progress dialog  b.v. 13.4 */

						  if (num_Tdew >= 0.0)
						  {
							  num_TdTdTd = int(num_Tdew * 10 + 0.5);            // num_TdTdTd in tienden
							  strcpy(char_Sn_TdTdTd, "0");
						  }
						  else // dus Tdew < 0.0
						  {
							  num_TdTdTd = int(num_Tdew * 10 - 0.5);            // num_TdTdTd in tienden
							  strcpy(char_Sn_TdTdTd, "1");
						  } // else (dus Tdew < 0.0)
						  /* num_TdTdTd is voor level checking later b.v. 134 (=13.4 °C) */

						  sprintf(char_TdTdTd, "%03d", abs(num_TdTdTd));
						  /* char_TdTdTd is de gecodeerde dewpoints waarde b.v. "134" */

					  } // if (num_Tdew != MAXINT)
					  else // dus ongeldig dewpoint
						  strcpy(char_obs_Tdew, "invalid");

					  if (num_Twet_total != MAXINT)
					  {
						  char char_obs_Twet_heel[10];        // tussen variable
						  char char_obs_Twet_tienden[2];      // tussen variable
						  char_obs_Twet_heel[0] = '\0';       // initialisatie
						  char_obs_Twet_tienden[0] = '\0';    // initialisatie
						  double heel_num_Twet = 0;           // tussen variable
						  int hulp_Twet;                      // tussen variable

						  //
						  // num_Twet_total (type float) splitsen in twee delen
						  // stel num_Twet_total = 10.2677
						  modf(num_Twet_total, &heel_num_Twet); // n.b. heel_num_Twet moet van type double zijn

						  //
						  // hele graden
						  sprintf(char_obs_Twet_heel, "%d", int(heel_num_Twet));
						  if ((num_Twet_total < 0.0) && (num_Twet_total > -1.0))
						  {
							  // tussen 0.0 en -1.0 wordt het - teken niet gegeven
							  // daarom deze dan hier toevoegen
							  obs_Wetbulb = "-";
							  obs_Wetbulb.append(string(char_obs_Twet_heel));
						  } // if (num_Twet_total < 0)
						  else
							  obs_Wetbulb = string(char_obs_Twet_heel);


						  //
						  // tienden
						  hulp_Twet = int(fabs(num_Twet_total) * 10 + 0.5);  // dus 10.3877 wordt 104
						  // hulp_Twet in tienden
						  // b.v.  10.38877(=num_Twet_total) * 10 = 103.8866 + 0.5 = 104,3866 -> hulp_Twet = 104
						  num_Twet_tienden = hulp_Twet % 10;           // 104 % 10 = 4
						  sprintf(char_obs_Twet_tienden, "%1d", num_Twet_tienden);
						  char_obs_Twet_tienden[1] = '\0';
						  obs_Wetbulbtienden = string(char_obs_Twet_tienden);

						  //
						  // voor IMMT log char_Sn_TnTnTn, char_TnTnTn en num_TnTnTn bepalen
						  //
						  if (num_Twet_total < 0)
							  strcpy(char_Sn_TnTnTn, "6");
						  else
							  strcpy(char_Sn_TnTnTn, "5");

						  num_TnTnTn = hulp_Twet;        // tienden
						  sprintf(char_TnTnTn, "%03d", num_TnTnTn);

					  } // if (num_Twet_total != MAXINT)
					  else // dus ongeldige num_Twet_total
					  {
						  char_Sn_TnTnTn[0] = '\0';
						  char_TnTnTn[0] = '\0';
						  num_TnTnTn = MAXINT;
						  obs_Wetbulb = "invalid";
					  } // else (dus ongeldige num_Twet_total)

					  temperatures_ok = TRUE;

				  } // if ((obs_Airtemp.compare("") != 0) && (obs_Rh.compare("") != 0))
				  else
					  temperatures_ok = TRUE;
			  } // else (niet beide (twet + tdry) ingevuld)


           if (temperatures_ok == TRUE)
           {
			     // opslaan methode zeewatertemperatuur & exposure meting
			     // (alleen als de zeewater bepalings methode of exposure is veranderd)
              if ( (file_seawater_method == INTAKE && seawater_method != INTAKE) ||
                   (file_seawater_method == BUCKET && seawater_method != BUCKET) ||
                   (file_seawater_method == HULL_CONTACT_SENSOR && seawater_method != HULL_CONTACT_SENSOR) ||
                   (file_seawater_method == OTHER && seawater_method != OTHER) ||
                   (file_seawater_method == "\0" && seawater_method != "\0") ||

                   (file_exposure == SLING && exposure != SLING) ||
                   (file_exposure == SCREEN && exposure != SCREEN) ||
                   (file_exposure == "\0") )
              {
                 Write_Temp_Settings();

              } // if ((file_seawater_method == etc.

              /* status bar updaten */
              Update_StatusBar_Obs();

              form_mode = STANDARD;

           } // if temperatures_ok == TRUE)
		  } // if (Airtemp_ok == TRUE && Wetbulb_ok == TRUE && Seawater_ok == TRUE && Rh_ok == TRUE)

        // geld alleen automatically next input screen mode
        if (in_next_sequence)
        {
           back_in_next_sequence = false;
           stop_in_next_sequence = false;
        } //  if (in_next_sequence)

	  } // if ((result = dlg.Execute()) == IDOK)
     else if (result == IDCANCEL)
     {
        // geld alleen automatically next input screen mode
        if (in_next_sequence)
        {
           if (dlg -> back_button_pressed)
           {
              back_in_next_sequence = true;
              stop_in_next_sequence = false;
           }
           else if (dlg -> stop_button_pressed)
           {
              stop_in_next_sequence = true;
              back_in_next_sequence = false;
           }
           else
           {
              back_in_next_sequence = false;
              stop_in_next_sequence = false;
           }
        } // if (in_next_sequence)
     } // else if (result == IDCANCEL)

  } while(temperatures_ok == FALSE && result != IDCANCEL);

  //
  /////// indien van toepassing en als nog niet gebeurd MSL pressure berekenen
  //
  if ((barometer_MSL_yes_no.compare("no") == 0) &&
      (hoogte_barometer != "" && atoi(hoogte_barometer.c_str()) >= 0 && atoi(hoogte_barometer.c_str()) <= 1000) &&
      (num_TTT != MAXINT) && (num_PPPP != MAXINT))
  {
     gebruik_default_temp = false;
     MSL_Herleiding_Luchtdruk(gebruik_default_temp);                 /* NB hier wordt dus de gemeten temp. meegegeven (en default temp zal dus niet gebruikt worden) */

     /* status bar updaten */
     Update_StatusBar_Obs();

	  if (MeteoProgressDialog -> MeteoProgressDialog_Created == TRUE)
	     MeteoProgressDialog -> UpdateAirpressureMeteoProgressDialog
		 										(obs_Pressure, obs_Pressuretienden,
                                     obs_MSL_Pressure, obs_MSL_Pressuretienden,
												 obs_Amounttendency, obs_Amounttiendentendency,
												 obs_Characteristics);
  } // if ((barometer_MSL_yes_no.compare("no") == 0) etc.

  delete dlg;

  // meteo progress updaten
  if (MeteoProgressDialog -> MeteoProgressDialog_Created == TRUE)
		MeteoProgressDialog -> UpdateTemperaturesMeteoProgressDialog
											 (obs_Airtemp, obs_Airtemptienden,
											  obs_Wetbulb, obs_Wetbulbtienden,
											  char_UUU, char_obs_Tdew,
											  obs_Seawater, obs_Seawatertienden);

}




void TMyWindow::CeShipTemperatures(TCommandEnabler &tce)
{
   /* deze form niet beschikbaar indien classic mode */
	if (form_mode == CLASSIC)
		tce.Enable(false);
	else
		tce.Enable();



	if ((num_TTT != MAXINT) || (num_TnTnTn != MAXINT) ||
		 (num_TwTwTw != MAXINT) || (num_UUU != MAXINT))
		tce.SetCheck (TCommandEnabler::Checked);
	else
		tce.SetCheck (TCommandEnabler::Unchecked);
}



void TMyWindow::Read_Temp_Settings()
{
	// method seawatertemp. measurement must be picked up from file \...\SEAWMETH.TXT
	ifstream is;
	BOOL file_aanwezig_ok = FALSE;
   string record;

   file_exposure        = "\0";               // initialisatie
   file_seawater_method = "\0";               // initialisatie


	Check_file_aanwezig(file_aanwezig_ok, temp_settings_file);
	if (file_aanwezig_ok)
	{
		is.open((turbowin_dir + temp_settings_file).c_str(), ios::in);
      do
      {
         record.read_line(is);
         if (record.length() > 20)
         {
            if (record.substr(0, 20) == zeewater_methode)
               file_seawater_method = record.substr(20);

            if (record.substr(0, 20) == exposure_txt)
               file_exposure = record.substr(20);

         } // if (record.length() > 20)
      } while (!is.eof());

		is.close();
      //... #define zeewater_methode                     "sea water method  : "
	} // if (file_aanwezig_ok)
}



void TMyWindow::Write_Temp_Settings()
{
   ofstream os;

   os.open((turbowin_dir + temp_settings_file).c_str(), ios::out);
   if (!os)
   {
  	   string buffer = "\0";
		buffer = "Unable to write to: ";
      buffer += turbowin_dir;
		buffer += temp_settings_file;
		buffer += "\n";
	   MessageBox(buffer.c_str(), "TurboWin error", MB_OK | MB_ICONERROR);
	} // if (!os)
	else // output file OK
	{
      // do not edit regel
      os.write(station_data_do_not_edit, strlen(station_data_do_not_edit));
      os.write("\n", strlen("\n"));

		// zeewater methode regel wegschrijven naar file
      os.write(zeewater_methode, strlen(zeewater_methode));
		os.write(seawater_method.c_str(), strlen(seawater_method.c_str()));
      os.write("\n", strlen("\n"));

      // exposure regel wegschrijven naar file
      os.write(exposure_txt, strlen(exposure_txt));
      os.write(exposure.c_str(), strlen(exposure.c_str()));
      os.write("\n", strlen("\n"));

      os.close();
   } // else (outpufile OK
}




