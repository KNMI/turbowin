#include <owl\owlpch.h>
#include "tw_17.rc"
#pragma hdrstp

class TWindowPrintout2 : public TPrintout
{
  public:
	 TWindowPrintout2(const char* title, const string obs_callsign,
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
																			  TWindow* window);


//	 TWindowPrintout2::TWindowPrintout2(const char* title, const string obs_callsign,
//											  TWindow* window);


	 void GetDialogInfo(int& minPage, int& maxPage,
							  int& selFromPage, int& selToPage);
	 void PrintPage(int page, TRect& rect, unsigned flags);
	 void SetBanding(BOOL b) {Banding = b;}
	 bool HasPage(int pageNumber) {return pageNumber == 1;}

  protected:
    string print_string;

	 string callsign;
	 string LatitudeDegrees;
	 string LatitudeTenths;
	 string North_or_South;
	 string LongitudeDegrees;
	 string LongitudeTenths;
	 string East_or_West;
	 string Year;
	 string Month;
	 string Day;
	 string Time;
	 string Airtemp;
	 string Airtemptienden;
    char UUU[10];
	 string Seawater;
	 string Seawatertienden;
	 string Pressure;
	 string Pressuretienden;
	 string MSL_Pressure;
	 string MSL_Pressuretienden;
	 string wind_speed;
	 string wind_direction;
	 int    wind_units;
	 string WavesFixedHeightTenths;
	 string WavesFixedHeightMetres;
	 string WavesFixedPeriod;
	 string SeaWaveHeight;
	 string SeaWavePeriod;
	 string FirstSwellHeight;
	 string FirstSwellPeriod;
	 string FirstSwellDirection;
	 string SecondSwellHeight;
	 string SecondSwellPeriod;
	 string SecondSwellDirection;
	 string VV;
	 string observername;
    string PresentWeather;

	 string stationtype;
    string golvenresultaat;

	 TWindow* Window;
	 BOOL     Scale;
};


