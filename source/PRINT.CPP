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

#include "print.h"



/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                            voor printen ship-synops                       */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
TWindowPrintout::TWindowPrintout(const char* title, const char* char_obs, TWindow* window)
							: TPrintout(title)

{
   print_obs = "\0";//print_obs[0] = '\0';
   print_obs = "\n";                             // 5 lege regels
   print_obs += "\n";                            //      --"--
   print_obs += "\n";                            //      --"--
   print_obs += "\n";                            //      --"--
   print_obs += "\n";                            //      --"--
   print_obs += string(char_obs);//strcpy(print_obs, char_obs);

//   print_obs[0] = '\0';
//   strcpy(print_obs, "\n\n\n\n\n");
//   strcat(print_obs, char_obs);


   Window = window;
   Scale = TRUE;
}


void TWindowPrintout::PrintPage(int, TRect& rect, unsigned)
{
   // Conditionally scale the DC to the window so the printout will resemble the window
   //
   int prevMode;
   TSize oldVExt, oldWExt;


   /* ander font voor groter lettertype */
//   TFont font("Arial");
//   DC -> SelectObject(font);

   if (Scale)
   {
	   prevMode = DC -> SetMapMode(MM_ISOTROPIC);
	   TRect windowSize = Window->GetClientRect();
	   DC -> SetViewportExt(PageSize, &oldVExt);
	   DC -> SetWindowExt(windowSize.Size(), &oldWExt);
	   DC -> IntersectClipRect(windowSize);
	   DC -> DPtoLP(rect, 2);
   }

   DC -> DrawText(print_obs.c_str(), -1, rect, DT_LEFT | DT_WORDBREAK);
   //DC -> DrawText(print_obs, -1, rect, DT_LEFT | DT_WORDBREAK);

   //
   // Als je onderstaande out comment dan wordt de (desktop)foto geprint
   //
   // Call the window to paint itself
   //Window -> Paint(*DC, FALSE, rect);

   // Restore changes made to the DC
   if (Scale)
   {
	   DC -> SetWindowExt(oldWExt);
	   DC -> SetViewportExt(oldVExt);
	   DC -> SetMapMode(prevMode);
   }


   /* restore font */
//   DC -> RestoreFont();

}

// Do not enable page range in the print dialog since only one page is
// available to be printed
//
void TWindowPrintout::GetDialogInfo(int& minPage, int& maxPage, int& selFromPage, int& selToPage)
{
   minPage = 0;
   maxPage = 0;
   selFromPage = selToPage = 0;
}



/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                            voor printen ship plain text                   */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
TWindowPrintout2::TWindowPrintout2(const char* title,
												const string obs_callsign,
												const string obs_LatitudeDegrees,
												const string obs_LatitudeTenths,
												const string obs_North_or_South,
												const string obs_LongitudeDegrees,
												const string obs_LongitudeTenths,
												const string obs_East_or_West,
												const string obs_Year,
												const string obs_Month,
												const string obs_Day,
												const string obs_Time,
												const string obs_Airtemp,
												const string obs_Airtemptienden,
												const char char_UUU[],
												const string obs_Seawater,
												const string obs_Seawatertienden,
												const string obs_Pressure,
												const string obs_Pressuretienden,
												const string obs_MSL_Pressure,
												const string obs_MSL_Pressuretienden,
												const string obs_wind_speed,
												const string obs_wind_direction,
												const int    iw_units,
												const string obs_WavesFixedHeightTenths,
												const string obs_WavesFixedHeightMetres,
												const string obs_WavesFixedPeriod,
												const string obs_SeaWaveHeight,
												const string obs_SeaWavePeriod,
												const string obs_FirstSwellHeight,
												const string obs_FirstSwellPeriod,
												const string obs_FirstSwellDirection,
												const string obs_SecondSwellHeight,
												const string obs_SecondSwellPeriod,
												const string obs_SecondSwellDirection,
												const string obs_VV,
												const string obs_observername,
                                    const string obs_PresentWeather,
												const string station_type,
                                    const string golven_resultaat,
																			  TWindow* window)
							: TPrintout(title)


{
	callsign               = obs_callsign;
	LatitudeDegrees        = obs_LatitudeDegrees;
	LatitudeTenths         = obs_LatitudeTenths;
	North_or_South         = obs_North_or_South;
	LongitudeDegrees       = obs_LongitudeDegrees;
	LongitudeTenths        = obs_LongitudeTenths;
	East_or_West           = obs_East_or_West;
	Year                   = obs_Year;
	Month                  = obs_Month;
	Day                    = obs_Day;
	Time                   = obs_Time;
	Airtemp                = obs_Airtemp;
	Airtemptienden         = obs_Airtemptienden;
   strcpy(UUU, char_UUU);
	Seawater               = obs_Seawater;
	Seawatertienden        = obs_Seawatertienden;
	Pressure               = obs_Pressure;
	Pressuretienden        = obs_Pressuretienden;
	MSL_Pressure           = obs_MSL_Pressure;
	MSL_Pressuretienden    = obs_MSL_Pressuretienden;
	wind_speed             = obs_wind_speed;
	wind_direction         = obs_wind_direction;
	wind_units             = iw_units;
	WavesFixedHeightTenths = obs_WavesFixedHeightTenths;
	WavesFixedHeightMetres = obs_WavesFixedHeightMetres;
	WavesFixedPeriod       = obs_WavesFixedPeriod;
	SeaWaveHeight          = obs_SeaWaveHeight;
	SeaWavePeriod          = obs_SeaWavePeriod;
	FirstSwellHeight       = obs_FirstSwellHeight;
	FirstSwellPeriod       = obs_FirstSwellPeriod;
	FirstSwellDirection    = obs_FirstSwellDirection;
	SecondSwellHeight      = obs_SecondSwellHeight;
	SecondSwellPeriod      = obs_SecondSwellPeriod;
	SecondSwellDirection   = obs_SecondSwellDirection;
	VV                     = obs_VV;
	observername           = obs_observername;
   PresentWeather         = obs_PresentWeather;

	stationtype            = station_type;
	golvenresultaat        = golven_resultaat;

	Window = window;
	Scale = TRUE;
}


void TWindowPrintout2::PrintPage(int, TRect& rect, unsigned)
{
  // Conditionally scale the DC to the window so the printout will resemble the window
  //
  int    prevMode;
  TSize  oldVExt, oldWExt;


  /* ander font voor groter lettertype */
  TFont font("Arial");
  DC -> SelectObject(font);


  if (Scale)
  {
	  prevMode = DC -> SetMapMode(MM_ISOTROPIC);
	  TRect windowSize = Window -> GetClientRect();
	  DC -> SetViewportExt(PageSize, &oldVExt);
	  DC -> SetWindowExt(windowSize.Size(), &oldWExt);
	  DC -> IntersectClipRect(windowSize);
	  DC -> DPtoLP(rect, 2);
  }

  //
  // print string opbouwen
  //
  print_string = "\0";
  print_string += "\n";
  print_string +=  "EXTRACTED PLAIN TEXT WEATHER REPORT (DO NOT TRANSMIT/INTERNAL USE ONLY)";
  print_string += "\n";
  print_string += "\n";

  // callsign/WMO identificatie-nummer
  print_string += text_identification;
  if (callsign != "\n");
  {
	  print_string += callsign;
  }
  print_string += "\n";

  // observer name
  print_string += text_observername;
  if (observername != "\0")
  {
	  print_string += observername;
  }
  print_string += "\n";

  // datum en tijd
  print_string += text_date_time;
  if ((Month != "\0") && (Day != "\0") && (Year != "\0") && (Time != "\0"))
  {
	  print_string += Month;
	  print_string += " ";
	  print_string += Day;
	  print_string += ", ";
	  print_string	+= Year;
	  print_string += " ";
	  print_string += Time;
	  print_string += ".00 UTC";
  }
  print_string += "\n";

  // positie
  print_string += text_position;
  if ((LatitudeDegrees != "\0") && (LatitudeTenths != "\0") && (North_or_South != "\0") &&
		(LongitudeDegrees != "\0") && (LongitudeTenths != "\0") && (East_or_West != "\0"))
  {
	  print_string += LatitudeDegrees;
	  print_string += "°";
	  print_string += LatitudeTenths;
	  print_string += "' ";
	  print_string += North_or_South;
	  print_string += "  ";
	  print_string += LongitudeDegrees;
	  print_string += "°";
	  print_string += LongitudeTenths;
	  print_string += "' ";
	  print_string += East_or_West;
  }
  print_string += "\n";

  // luchttemperatuur
  print_string += text_air_temperature;
  if ((Airtemp != "\0") && (Airtemptienden != "\0"))
  {
	  print_string += Airtemp;
	  print_string += ".";
	  print_string += Airtemptienden;
	  print_string += " °C";
  }
  print_string += "\n";

  // Relative vochtigheid
  print_string += text_relative_humidity;
  if (UUU[0] != '\0')
  {
	  print_string += string(UUU);
	  print_string += " %";
  }
  print_string += "\n";

  // zeewatertemperatuur
  print_string += text_seawater_temperature;
  if ((Seawater != "\0") && (Seawatertienden != "\0"))
  {
	  print_string += Seawater;
	  print_string += ".";
	  print_string += Seawatertienden;
	  print_string += " °C";
  }
  print_string += "\n";

  // Pressure (read)
  print_string += text_air_pressure;
  if ((Pressure != "\0") && (Pressuretienden != "\0"))
  {
	  print_string += Pressure;
	  print_string += ".";
	  print_string += Pressuretienden;
	  print_string += " hPa";
  }
  print_string += "\n";

  // Pressure (MSL)
  print_string += text_MSL_air_pressure;
  if ((MSL_Pressure != "\0") && (MSL_Pressuretienden != "\0"))
  {
	  print_string += MSL_Pressure;
	  print_string += ".";
	  print_string += MSL_Pressuretienden;
	  print_string += " hPa";
  }
  print_string += "\n";

  // wind richting
  print_string += text_wind_direction;
  if (wind_direction != "\0")
  {
	  print_string += wind_direction;
	  if ((wind_direction.compare("calm") != 0) && (wind_direction.compare("variable") != 0))
		  print_string += " °";
  }
  print_string += "\n";

  // wind speed
  print_string += text_wind_speed;
  if ((wind_speed != 0) && ((wind_units == 1) || (wind_units == 2)))
  {
	  print_string += wind_speed;
	  if (wind_units == 1)
		  print_string += " kts";
	  if (wind_units == 2)
		  print_string += " m/s";
  }
  print_string += "\n";

  // gemeten golfhoogte
  if ( ((stationtype.compare("fixed") == 0) && (golvenresultaat.compare(waves_measured_shipborne) == 0)) ||
       ((stationtype.compare("fixed") == 0) && (golvenresultaat.compare(waves_measured_buoy) == 0)) ||
       ((stationtype.compare("fixed") == 0) && (golvenresultaat.compare(waves_measured_other) == 0)) )
  {
	  print_string += text_wave_height_measured;
	  if ((WavesFixedHeightTenths != "\0") && (WavesFixedHeightMetres != "\0"))
	  {
		  print_string += WavesFixedHeightMetres;
		  print_string += ".";
		  print_string += WavesFixedHeightTenths;
		  print_string += " metres";
	  }
	  print_string += "\n";

	  // gemeten golfrichting
	  print_string += text_wave_period_measured;
	  if ((WavesFixedPeriod != "\0") && (WavesFixedPeriod != "\0"))
	  {
		  print_string += WavesFixedPeriod;
		  print_string += ".";
		  print_string += WavesFixedPeriod;
		  print_string += " seconds";
	  }
	  print_string += "\n";
  } // if ((station_type.compare("fixed") == 0) && ((golven_resultaat.compare("waves_measured") == 0) || etc.)
  else // dus geschatte golven
  {
	  // zeegang hoogte geschat
	  print_string += text_sea_wave_height;
	  if (SeaWaveHeight != "\0")
	  {
		  print_string += SeaWaveHeight;
		  if (SeaWaveHeight.compare("confused") != 0)
			  print_string += " metres";
	  }
	  print_string += "\n";

	  // zeegang periode geschat
	  print_string += text_sea_wave_period;
	  if (SeaWavePeriod != "\0")
	  {
		  print_string += SeaWavePeriod;
		  if (SeaWavePeriod.compare("confused") != 0)
			  print_string += " seconds";
	  }
	  print_string += "\n";

	  // first swell direction
	  print_string += text_first_swell_direction;
	  if (FirstSwellDirection != "\0")
	  {
		  print_string += FirstSwellDirection;
		  if (FirstSwellDirection.compare("confused") != 0)
			  print_string += " °";
	  }
	  print_string += "\n";

	  // first swell height
	  print_string += text_first_swell_height;
	  if (FirstSwellHeight != "\0")
	  {
		  print_string += FirstSwellHeight;
		  if (FirstSwellHeight.compare("confused") != 0)
			  print_string += " metres";
	  }
	  print_string += "\n";

	  // first swell period
	  print_string += text_first_swell_period;
	  if (FirstSwellPeriod != "\0")
	  {
		  print_string += FirstSwellPeriod;
		  if (FirstSwellPeriod.compare("confused") != 0)
			  print_string += " seconds";
	  }
	  print_string += "\n";

	  // second swell direction
	  print_string += text_second_swell_direction;
	  if (SecondSwellDirection != "\0")
	  {
		  print_string += SecondSwellDirection;
		  if (SecondSwellDirection.compare("confused") != 0)
			  print_string += " °";
	  }
	  print_string += "\n";

	  // second swell height
	  print_string += text_second_swell_height;
	  if (SecondSwellHeight != "\0")
	  {
		  print_string += SecondSwellHeight;
		  if (SecondSwellHeight.compare("confused") != 0)
			  print_string += " metres";
	  }
	  print_string += "\n";

	  // second swell period
	  print_string += text_second_swell_period;
	  if (SecondSwellPeriod != "\0")
	  {
		  print_string += SecondSwellPeriod;
		  if (SecondSwellPeriod.compare("confused") != 0)
			  print_string += " seconds";
	  }
	  print_string += "\n";
  } // else (dus geschatte golven

  // visibility
  print_string += text_VV;
  if (VV != "\0")
  {
	  print_string += VV;
  }
  print_string += "\n";


  // present weather
  //print_string += text_present_weather;
  //if (PresentWeather!= "\0")
  //{
  //	  print_string += PresentWeather;
  //}
  //else
  //   ::MessageBox(0, "lege ww string", "test", MB_OK);
  //print_string += "\n";
  //print_string += "\n";
  

  DC -> DrawText(print_string.c_str(), -1, rect, DT_LEFT | DT_WORDBREAK);

	//
  // Als je onderstaande out comment dan wordt de (desktop)foto geprint
  //
  // Call the window to paint itself
  //Window -> Paint(*DC, FALSE, rect);

  // Restore changes made to the DC
  if (Scale)
  {
	  DC->SetWindowExt(oldWExt);
	  DC->SetViewportExt(oldVExt);
	  DC->SetMapMode(prevMode);
  }

  /* restore font */
  DC -> RestoreFont();

}

// Do not enable page range in the print dialog since only one page is
// available to be printed
//
void TWindowPrintout2::GetDialogInfo(int& minPage, int& maxPage, int& selFromPage, int& selToPage)
{
	minPage = 0;
	maxPage = 0;
	selFromPage = selToPage = 0;
}



/*****************************************************************************/
/*                                                                           */
/*                              voor printen:                                */
/*                                                                           */
/*                            buoy deployments                               */
/*                                  en                                       */
/*                             station data                                  */
/*                                  en                                       */
/*                             E-mail settings                               */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
TWindowPrintout3::TWindowPrintout3(const char* title, const string print_text, TWindow* /*window*/)
							: TPrintout(title)

{
   local_print_text = "\n" + print_text;

   //Window = window;
   //Scale = TRUE;
}


void TWindowPrintout3::PrintPage(int, TRect& rect, unsigned)
{
   DC -> DrawText(local_print_text.c_str(), -1, rect, DT_LEFT | DT_WORDBREAK);
}



// Do not enable page range in the print dialog since only one page is
// available to be printed
//
void TWindowPrintout3::GetDialogInfo(int& minPage, int& maxPage, int& selFromPage, int& selToPage)
{
  minPage = 0;
  maxPage = 0;
  selFromPage = 0;
  selToPage = 0;
}




