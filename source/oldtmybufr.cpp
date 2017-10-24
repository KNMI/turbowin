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
#include <classlib\date.h>
#include <classlib\time.h>
#pragma hdrstop

#include "tmywindo.h"

//#include <stdio.h>
//#include <stdlib.h>

#include "bufr3.h"         /* martin:<bufr3.h> */


#define NUM_SUBSETS 1      /* Number of Subsets - adjust as you like */
#define MAX_ENTRIES 1000   /* Max Number of Entries per subset */


bool TMyWindow::samenstellen_te_verzenden_BUFR_obs()
{
   unsigned char sect1[18];    /* Section 1 is always 18 */
   unsigned char sect3[4000];  /* 4000 should be enough for all cases */
   int *(entries[NUM_SUBSETS]);
   unsigned char **(centries[NUM_SUBSETS]);
   int num_entries[NUM_SUBSETS];

   /* begin martin */
   int schalings_factor;
   int scale;
   int ref;
   int gem_Vs;                              // hulp var.
   int gem_VV;                              // hulp var.
   int h_meter;                             // hulp var.
   int hulp_wind_speed;                     // hulp var.
   string turbowin_bufr_input_file;
   ofstream os;
   char buffer[256];


   long bufr_ship_direction;                // long: -2,147,483,648 to 2,147,483,647  (want #define BUFR3_UNDEF_VALUE (-1000000000) [9 nullen] in bufr3.h)
   long bufr_ship_speed;
   long bufr_ix;
   long bufr_year;
   long bufr_month;
   long bufr_day;
   long bufr_hour;
   long bufr_minute;
   long bufr_latitude;
   long bufr_longitude;
   long bufr_station_height_msl;
   long bufr_barometer_height_msl;
   long bufr_PoPoPoPo;
   long bufr_PPPP;
   long bufr_ppp;
   long bufr_a;
   long bufr_temp_height_deck;
   long bufr_temp_height_water;
   long bufr_method_wetbulb;
   long bufr_air_temp;
   long bufr_wet_bulb;
   long bufr_dewpoint;
   long bufr_rv;
   long bufr_visibility_height_deck;
   long bufr_visibility_height_water;
   long bufr_visibility;
   long bufr_visibility_height_water_missing;
   long bufr_short_delayed_descript_rep_factor_precip_24;
   long bufr_precipitation_height_deck_missing;
   long bufr_short_delayed_descript_rep_factor_cloud;
   long bufr_cloud_cover;
   long bufr_cloud_vertical_significance;
   long bufr_cloud_amount_low_medium;
   long bufr_cloud_base;
   long bufr_Cl;
   long bufr_Cm;
   long bufr_Ch;
   long bufr_delayed_descript_rep_factor_advanced_cloud;
   long bufr_cloud_vertical_significance_missing;
   long bufr_short_delayed_descript_rep_factor_ice;
   long bufr_thickness_ice_accretion;
   long bufr_rate_ice_accretion;
   long bufr_cause_ice_accretion;
   long bufr_sea_ice_concentration;
   long bufr_amount_type_ice;
   long bufr_ice_situation;
   long bufr_ice_development;
   long bufr_ice_edge_bearing;
   long bufr_short_delayed_descript_rep_factor_sst;
   long bufr_sst_method;
   long bufr_sst;
   long bufr_short_delayed_descript_rep_factor_waves_measured;
   long bufr_dir_waves_measured;
   long bufr_period_waves_measured;
   long bufr_height_waves_measured;
   long bufr_short_delayed_descript_rep_factor_waves_estimated;
   long bufr_dir_waves_estimated;
   long bufr_period_wind_waves_estimated;
   long bufr_height_wind_waves_estimated;
   long bufr_dir_swell_1_estimated;
   long bufr_period_swell_1_estimated;
   long bufr_height_swell_1_estimated;
   long bufr_dir_swell_2_estimated;
   long bufr_period_swell_2_estimated;
   long bufr_height_swell_2_estimated;
   long bufr_present_weather;
   long bufr_weather_time_period;
   long bufr_past_weather_1;
   long bufr_past_weather_2;
   long bufr_short_delayed_descript_rep_factor_precip_measure;
   long bufr_short_delayed_descript_rep_factor_temp_extreme;
   long bufr_wind_height_deck;
   long bufr_type_wind_instruments;
   long bufr_wind_time_significance;
   long bufr_wind_time_period;
   long bufr_wind_dir;
   long bufr_wind_speed;
   long bufr_gusts_time_significance;
   long bufr_delayed_descript_rep_factor_gusts;
   
   /* end martin */


   /* The following are the integer/character data arrays -
      for the sake of simplicity we use identical data for every subset
      and use only 1 array where all pointers in entries/centries
      are pointing to */

   int data[MAX_ENTRIES];
   unsigned char *(cdata[MAX_ENTRIES]);

   FILE *fd;
   bufr3data *bufdes;
   int i, res;


   /* Create sections - we omit the optional section 2 in this case */

   sect1[ 0] =  0; /* length */
   sect1[ 1] =  0; /* length */
   sect1[ 2] = 18; /* length */
   sect1[ 3] =  0; /* Master table number */
   sect1[ 4] =  0; /* Originating/generating sub-centre */
   sect1[ 5] = 78; /* Originating/generating centre */
   sect1[ 6] =  0; /* Update sequence number */
   sect1[ 7] =  0; /* Optional section */
   sect1[ 8] =  1; /* Data category */
   sect1[ 9] =  0; /* Data sub-category */
   sect1[10] =  9; /* Version number of master tables */
   sect1[11] =  7; /* Version number of local tables */
   sect1[12] =  2; /* Year of century */
   sect1[13] = 12; /* Month */
   sect1[14] = 31; /* Day */
   sect1[15] = 23; /* Hour */
   sect1[16] = 59; /* Minute */
   sect1[17] =  0; /* Pad to even size */

   sect3[ 3] =  0; /* Reserved */
   sect3[ 4] =  0;          /* Number subsets, high order byte */
   sect3[ 5] = NUM_SUBSETS; /* Number subsets, low order byte */
   sect3[ 6] =  0; /* Observed/compressed flag */

   i = 7;

   sect3[i] = 3*64+ 7; sect3[i+1]= 70; i+=2;  /* 3 07 070 VOS */

   //sect3[i] = 0; /* Pad to even size */
   //i++;

   /* Length of section 3 */

   sect3[0] = 0;
   sect3[1] = (i>>8)&0xff;
   sect3[2] = i&0xff;



   for(i=0;i<NUM_SUBSETS;i++)
   {
      num_entries[i] = 70;
      entries[i] = data;
      centries[i] = cdata;
   }

   for(i=0;i<num_entries[0];i++)
      cdata[i] = NULL;


   /*
   ///////////////////////////////////////////// 0 01 012     Ds  ///////////////////////////////
   */
   /* unit = deg true */
   scale = 1;
   ref   = 0;
   schalings_factor = (int)pow10(scale);

   if (num_Ds == 0)
      bufr_ship_direction = 0 * schalings_factor - ref;
   else if (num_Ds == 1)
      bufr_ship_direction = 45 * schalings_factor - ref;
   else if (num_Ds == 2)
      bufr_ship_direction = 90 * schalings_factor - ref;
   else if (num_Ds == 3)
      bufr_ship_direction = 135 * schalings_factor - ref;
   else if (num_Ds == 4)
      bufr_ship_direction = 180 * schalings_factor - ref;
   else if (num_Ds == 5)
      bufr_ship_direction = 225 * schalings_factor - ref;
   else if (num_Ds == 6)
      bufr_ship_direction = 270 * schalings_factor - ref;
   else if (num_Ds == 7)
      bufr_ship_direction = 315 * schalings_factor - ref;
   else if (num_Ds == 8)
      bufr_ship_direction = 360 * schalings_factor - ref;
   else if (num_Ds == 9)
      bufr_ship_direction = BUFR3_UNDEF_VALUE;
   else
      bufr_ship_direction = BUFR3_UNDEF_VALUE;


   /*
   ////////////////////////////////////////// 0 01 013     Vs /////////////////////////////////////
   */
   /* unit = m/s */
   scale = 1;
   ref   = 0;

   if (num_Vs == 0)
      gem_Vs = 0;                // knots
   else if (num_Vs == 1)
      gem_Vs = 3;                // knots
   else if (num_Vs == 2)
      gem_Vs = 8;
   else if (num_Vs == 3)
      gem_Vs = 13;
   else if (num_Vs == 4)
      gem_Vs = 18;
   else if (num_Vs == 5)
      gem_Vs = 23;
   else if (num_Vs == 6)
      gem_Vs = 28;
   else if (num_Vs == 7)
      gem_Vs = 33;
   else if (num_Vs == 8)
      gem_Vs = 38;
   else if (num_Vs == 9)
      gem_Vs = 42;
   else
      gem_Vs = MAXINT;

   if (gem_Vs != MAXINT)
   {
      bufr_ship_speed = int((float)gem_Vs * omzet_kn_ms + 0.5);        // omzet_kn_ms = 0.5144444
      bufr_ship_speed = bufr_ship_speed * schalings_factor - ref;
   }
   else
      bufr_ship_speed = BUFR3_UNDEF_VALUE;


   /*
   ////////////////////////////////////////////// 0 02 001     ix   //////////////////////////////////
   */
   /* unit  = code table (1=manned station) */
   /* scale = 0 */
   /* ref   = 0 */

   bufr_ix = 1;


   /*
   // 0 04 001     year
   */
   /* unit  = nvt */
   /* scale = nvt */
   /* ref   = nvt */

   bufr_year = 2000 + num_AA;


   /*
   /////////////////////////////////////////////// 0 04 002     month ///////////////////////////////////
   */
   /* unit  = nvt */
   /* scale = nvt */
   /* ref   = nvt */

   bufr_month = num_MM;


   /*
   //////////////////////////////////////////////// 0 04 003     day /////////////////////////////////////
   */
   /* unit  = nvt */
   /* scale = nvt */
   /* ref   = nvt */

   bufr_day = num_DD;



   /*
   //////////////////////////////////////////////// 0 04 004     hour ////////////////////////////////////////
   */
   /* unit  = nvt */
   /* scale = nvt */
   /* ref   = nvt */

   bufr_hour = num_GG;


   /*
   //////////////////////////////////////////////// 0 04 005     minute ////////////////////////////////////////
   */
   /* unit  = nvt */
   /* scale = nvt */
   /* ref   = nvt */

   bufr_minute = 0;        // per definitie voor een obs vanaf een schip


   /*
   //////////////////////////////////////////////// 0 05 002     LaLaLa ////////////////////////////////////////
   */
   /* unit  = degrees */
   scale = 2;
   ref   = -9000;
   schalings_factor = (int)pow10(scale);

   /* NB numLaLaLa staat (al) in  tienden graden -> schalings_factor delen door 10; dus LaLaLa / 10 = breedte in graden */
   bufr_latitude = num_LaLaLa * (schalings_factor / 10) - ref;


   /*
   //////////////////////////////////////////////// 0 06 002     LoLoLoLo ////////////////////////////////////////
   */
   /* unit  = degrees */
   scale = 2;
   ref   = -18000;
   schalings_factor = (int)pow10(scale);

   /* NB numLoLoLoLo staat (al) in  tienden graden -> schalings_factor delen door 10; dus LoLoLoLo / 10 = lengte in graden */
   bufr_longitude = num_LoLoLoLo * (schalings_factor / 10) - ref;


   /*
   /////////////////////////////////// 0 07 030 height station platform above msl /////////////////////////////////
   */
   /* unit  = meter */
   //scale = 1;
   //ref   = -4000;
   //schalings_factor = (int)pow10(scale);

   bufr_station_height_msl = BUFR3_UNDEF_VALUE;


   /*
   //////////////////////////////////////// 0 07 031 height barometer above msl /////////////////////////////////
   */
   /* unit  = meter */
   //scale = 1;
   //ref   = -4000;
   //schalings_factor = (int)pow10(scale);

   bufr_barometer_height_msl = BUFR3_UNDEF_VALUE;


   /*
   //////////////////////////////////////////////// 0 10 004     PoPoPoPo  ////////////////////////////////////////
   */
   /* unit  = Pa */
   //scale = -1;
   //ref   = 0;

   bufr_PoPoPoPo = BUFR3_UNDEF_VALUE;               // luchtdruk op sensor hoogte



   /*
   //////////////////////////////////////////////// 0 10 051     PPPP  ////////////////////////////////////////
   */
   /* unit  = Pa */
   scale = -1;
   ref   = 0;
   schalings_factor = (int)pow10(scale);

   /* NB PPPP in tienden hPa -> PPPPP = 10125 -> 1012.5 hPa -> 101250 Pa; dus PPPPP  * 10 = luchtdruk in Pa */
   bufr_PPPP = num_PPPP * (schalings_factor * 10) - ref;



   /*
   //////////////////////////////////////////////// 0 10 061     ppp ////////////////////////////////////////
   */
   /* unit  = Pa */
   scale = -1;
   ref   = -500;
   schalings_factor = (int)pow10(scale);

   /* NB ppp in tienden hPa -> ppp = 15 -> 1.5 hPa -> 150 Pa; dus ppp * 10 = luchtdruk in Pa */
   bufr_ppp = num_ppp * (schalings_factor * 10) - ref;



   /*
   //////////////////////////////////////////////// 0 10 063     a ////////////////////////////////////////
   */
   /* unit  = code table (0 - 8, 15=missing value) */
   /* scale = 0 */
   /* ref   = 0 */

   if (num_a == MAXINT)
      bufr_a = 15;                                           // 15 = missing value
   else
      bufr_a = num_a;


   /*
   //////////////////////////////////// 0 07 032     height sensor above marine deck for temp  ///////////////
   */
   /* unit  = meter */
   //scale = 2;
   //ref   = 0;
   //schalings_factor = (int)pow10(scale);
   
   bufr_temp_height_deck = BUFR3_UNDEF_VALUE;



   /*
   //////////////////////////////////// 0 07 033   height of sensor above water surf for temp  ///////////////
   */
   /* unit  = meter */
   //scale = 1;
   //ref   = 0;
   //schalings_factor = (int)pow10(scale);

   bufr_temp_height_water = BUFR3_UNDEF_VALUE;


   /*
   //////////////////////////////////////////////// 0 12 101     snTTT; dry bulb temp        ////////////////////////////////////////
   */
   /* unit  = Kelvin */
   scale = 2;
   ref   = 0;
   schalings_factor = (int)pow10(scale);

   /* NB num_TTT staat (al) in  tienden graden -> schalings_factor delen door 10; dus num_TTT / 10 = temp in Celsius */
   bufr_air_temp = (num_TTT + CELCIUS_TO_KELVIN_FACTOR) * (schalings_factor / 10) - ref;


   /*
   //////////////////////////////////////////////// 0 02 039     method of wet bulb temp measurement       ////////////////////////////////////////
   */
   /* unit  = code table (0 - 3; 7 = missing value) */
   /* scale = 0 */
   /* ref   = 0 */

   /* NB in FM 13 code sign and type wet bulb temp kan zijn: 0,1,2,5,6,7 */
   /* in bufr gaat het 0 -3; 7 missing */

   if (strncmp(char_Sn_TnTnTn, "0", 1) == 0)              // positive or zero measured
      bufr_method_wetbulb = 0;
   else if (strncmp(char_Sn_TnTnTn, "1", 1) == 0)         // negative measured
      bufr_method_wetbulb = 0;
   else if (strncmp(char_Sn_TnTnTn, "2", 1) == 0)         // iced bulb measured
      bufr_method_wetbulb = 1;
   else if (strncmp(char_Sn_TnTnTn, "5", 1) == 0)         // positive or zero computed
      bufr_method_wetbulb = 2;
   else if (strncmp(char_Sn_TnTnTn, "6", 1) == 0)         // negative computed
      bufr_method_wetbulb = 2;
   else if (strncmp(char_Sn_TnTnTn, "7", 1) == 0)         // iced bulb computed
      bufr_method_wetbulb = 3;
   else
      bufr_method_wetbulb = 7;                            // missing


   /*
   //////////////////////////////////////////////// 0 12 102     swTnTnTn; wet-bulb       ////////////////////////////////////////
   */
   /* unit  = K */
   scale = 2;
   ref   = 0;
   schalings_factor = (int)pow10(scale);

   /* NB num_TnTnTn staat (al) in  tienden graden -> schalings_factor delen door 10; dus num_TnTnTn / 10 = temp in Celsius */
   bufr_wet_bulb = (num_TnTnTn + CELCIUS_TO_KELVIN_FACTOR) * (schalings_factor / 10) - ref;


   /*
   //////////////////////////////////////////////// 0 12 103     swTdTdTd; dewpoint    ///////////////////////////////
   */
   /* unit  = K */
   scale = 2;
   ref   = 0;
   schalings_factor = (int)pow10(scale);

   /* NB num_TdTdTd staat (al) in  tienden graden -> schalings_factor delen door 10; dus num_TdTdTd / 10 = temp in Celsius */
   bufr_dewpoint = (num_TdTdTd + CELCIUS_TO_KELVIN_FACTOR) * (schalings_factor / 10) - ref;


   /*
   //////////////////////////////////////////////// 0 13 003     relative humidity   ////////////////////////////////
   */
   /* unit  = % */
   scale = 0;
   ref   = 0;
   schalings_factor = (int)pow10(scale);

   if (num_UUU == MAXINT)
      bufr_rv =  BUFR3_UNDEF_VALUE;
   else
      bufr_rv = num_UUU * schalings_factor - ref;;


   /*
   /////////////////////////// 0 07 032 (height sensor above marine deck for visibility  /////////////////////
   */
   /* unit  = meter */
   //scale = 2;
   //ref   = 0;
   //schalings_factor = (int)pow10(scale);

   bufr_visibility_height_deck = BUFR3_UNDEF_VALUE;


   /*
   //////////////////////////// 0 07 033 (height of sensor above water surface for visibility   //////////////////////
   */
   /* unit  = meter */
   //scale = 1;
   //ref   = 0;
   //schalings_factor = (int)pow10(scale);

   bufr_visibility_height_water = BUFR3_UNDEF_VALUE;


   /*
   /////////////////////////////////////////////// 0 20 001 VV  /////////////////////////////////////////////////
   */
   /* unit  = meter */
   scale = -1;
   ref   = 0;
   schalings_factor = (int)pow10(scale);

   if (num_VV == 90)
      gem_VV = 0;                     // nu in meters
   else if (num_VV == 91)
      gem_VV = 50;                    // nu in meters
   else if (num_VV == 92)
      gem_VV = 200;                   // nu in meters
   else if (num_VV == 93)
      gem_VV = 500;                   // nu in meters
   else if (num_VV == 94)
      gem_VV = 1000;                  // nu in meters
   else if (num_VV == 95)
      gem_VV = 2000;                  // nu in meters
   else if (num_VV == 96)
      gem_VV = 4000;                  // nu in meters
   else if (num_VV == 97)
      gem_VV = 10000;                 // nu in meters
   else if (num_VV == 98)
      gem_VV = 20000;                 // nu in meters
   else if (num_VV == 99)
      gem_VV = 50000;                 // nu in meters
   else
      gem_VV = MAXINT;

   if (gem_VV != MAXINT)
      bufr_visibility = gem_VV * schalings_factor - ref;
   else
      bufr_visibility = BUFR3_UNDEF_VALUE;


   /*
   /////////////// 0 07 033 (height sensor above water surface -set to missing to cancel previous value-) /////////////
   */
   /* unit  = meter */
   /* scale = 1 */
   /* ref   = 0 */

   bufr_visibility_height_water_missing = BUFR3_UNDEF_VALUE;        // cancel previous value


   /*
   //////////////////////////// 0 31 000 (short delayed description replication factor)       /////////////////////////
   */
   /* unit  = nvt */
   /* scale = nvt */
   /* ref   = nvt */

   bufr_short_delayed_descript_rep_factor_precip_24 = 0;              // 0 = no replication


   /*
   ///////////// 0 07 032 (height sensor above marine deck) -set to missing to cancel previous value-  //////////////////
   */
   /* unit  = nvt */
   /* scale = nvt */
   /* ref   = nvt */

   bufr_precipitation_height_deck_missing = BUFR3_UNDEF_VALUE;     // cancel previous value


   /*
   /////////////////////////// 0 31 000 (short delayed description replication factor)  ///////////////////////////
   */
   /* unit  = nvt */
   /* scale = nvt */
   /* ref   = nvt */

   bufr_short_delayed_descript_rep_factor_cloud = 1;              // 1 = yes replication


   /*
   //////////////////////////////////////////////// 0 20 010 (N; %) ////////////////////////////////////////
   */
   /* unit  = % */
   /* scale = 0; */
   /* ref   = 0; */
   /* schalings_factor = nvt */

   if (num_N == 1)
      bufr_cloud_cover = int(1.0 * 100 / 8.0 + 0.5);
   else if (num_N == 2)
      bufr_cloud_cover = int(2.0 * 100 / 8.0 + 0.5);
   else if (num_N == 3)
      bufr_cloud_cover = int(3.0 * 100 / 8.0 + 0.5);
   else if (num_N == 4)
      bufr_cloud_cover = int(4.0 * 100 / 8.0 + 0.5);
   else if (num_N == 5)
      bufr_cloud_cover = int(5.0 * 100 / 8.0 + 0.5);
   else if (num_N == 6)
      bufr_cloud_cover = int(6.0 * 100 / 8.0 + 0.5);
   else if (num_N == 7)
      bufr_cloud_cover = int(7.0 * 100 / 8.0 + 0.5);
   else if (num_N == 8)
      bufr_cloud_cover = int(8.0 * 100 / 8.0 + 0.5);
   else
      bufr_cloud_cover = BUFR3_UNDEF_VALUE;         // nb dus ook N = 9 (obscured) wordt hier bij bufr undefined ("missing")


   /*
   //////////////////////////////////////////////// 0 08 002 (vertical significance) ////////////////////////////////////////
   */
   /* unit  = code table (0) */
   /* scale = 0 */
   /* ref   = 0 */

   bufr_cloud_vertical_significance = 0;            // 0 = observing rules FM13 SHIP apply


   /*
   //////////////////////////////////////////// 0 20 011 (Nh; code table)/////////////////////////////////////////
   */
   /* unit  = code table (0..9) */
   /* scale = 0 */
   /* ref   = 0 */

   if (num_Nh == 0 || num_Nh == 1 || num_Nh == 2 || num_Nh == 3 || num_Nh == 4 || num_Nh == 5 ||
       num_Nh == 6 || num_Nh == 7 || num_Nh == 8 || num_Nh == 9)
      bufr_cloud_amount_low_medium = num_Nh;
   else
      bufr_cloud_amount_low_medium = BUFR3_UNDEF_VALUE;


   /*
   ///////////////////////////////////////////////////// 0 20 013 (h) /////////////////////////////////////////////
   */
   /* unit = meter */
   scale = -1;
   ref   = -40;
   schalings_factor = (int)pow10(scale);

   // note: bij cloudless en cloud height > 2500 m or more wordt voor beide h = 9 gegeven (num_h = 9 en char_h = "9")
   //       nu kan wel obs_h gebruikt worden
   //       (obs_h = h_0 voor cloudless of obs_h = h_2500_groter/h_2500_groter_meter echt >= 2500 meter)
   //
   if (num_h == 9 && obs_h == h_0)                   // cloudless
      h_meter = MAXINT;                                      // ?????????????????????????????????
   else if (num_h == 0)                               // 0 - 50 meter
      h_meter = 25;
   else if (num_h == 1)                               // 50 - 100 meter
      h_meter = 75;
   else if (num_h == 2)                               // 100 - 200 meter
      h_meter = 150;
   else if (num_h == 3)                               // 200 - 300 meter
      h_meter = 250;
   else if (num_h == 4)                               // 300 - 600 meter
      h_meter = 450;
   else if (num_h == 5)                               // 600 - 1000 meter
      h_meter = 800;
   else if (num_h == 6)                               // 1000 - 1500 meter
      h_meter = 1250;
   else if (num_h == 7)                               // 1500 - 2000 meter
      h_meter = 1750;
   else if (num_h == 8)                               // 2000 - 2500 meter
      h_meter = 2250;
   else if (num_h == 9 && obs_h != h_0)               // 2500 meter of meer
      h_meter = 2750;
   else
      h_meter = MAXINT;

   if (h_meter != MAXINT)
      bufr_cloud_base = h_meter * schalings_factor - ref;
   else
      bufr_cloud_base = BUFR3_UNDEF_VALUE;


   /*
   ////////////////////////////////////////////////  0 20 012 (Cl)       ////////////////////////////////////////
   */
   /* unit  = code table (30..39)*/
   /* scale = 0 */
   /* ref   = 0 */

   // NB de gewone Cl code loopt van 0 - 9 en de bufr Cl code van 30 - 39
   //
   // NB in de bufr Cl code is er ook een Cl = 62 = invisible owing to darkness etc.
   //    hiermee kan je een onderscheid maken met niet waargeneomen (dit onderscheid kan niet in FM23 ship code)
   //    voorlopig deze Cl = 62 niet gebruikt (zou dan een extra radio button op input form moeten komen)

   if (num_Cl != MAXINT)
      bufr_Cl = num_Cl + 30;
   else
      bufr_Cl = BUFR3_UNDEF_VALUE;


   /*
   ////////////////////////////////////////////////  0 20 012 (Cm)       ////////////////////////////////////////
   */
   /* unit  = code table (20..29)*/
   /* scale = 0 */
   /* ref   = 0 */

   // NB de gewone Cm code loopt van 0 - 9 en de bufr Cm code van 20 - 29
   //
   // NB in de bufr Cm code is er ook een Cm = 61 = invisible owing to darkness etc.
   //    hiermee kan je een onderscheid maken met niet waargeneomen (dit onderscheid kan niet in FM23 ship code)
   //    voorlopig deze Cm = 61 niet gebruikt (zou dan een extra radio button op input form moeten komen)

   if (num_Cm != MAXINT)
      bufr_Cm = num_Cm + 20;
   else
      bufr_Cm = BUFR3_UNDEF_VALUE;


   /*
   ////////////////////////////////////////////////  0 20 012 (Ch)       ////////////////////////////////////////
   */
   /* unit  = code table (10..19)*/
   /* scale = 0 */
   /* ref   = 0 */

   // NB de gewone Ch code loopt van 0 - 9 en de bufr Ch code van 10 - 19
   //
   // NB in de bufr Ch code is er ook een Ch = 60 = invisible owing to darkness etc.
   //    hiermee kan je een onderscheid maken met niet waargeneomen (dit onderscheid kan niet in FM23 ship code)
   //    voorlopig deze Ch = 60 niet gebruikt (zou dan een extra radio button op input form moeten komen)

   if (num_Ch != MAXINT)
      bufr_Ch = num_Ch + 10;
   else
      bufr_Ch = BUFR3_UNDEF_VALUE;

      
   /*
   ////////////////////////////////////0 31 001 (delayed description replication factor)  /////////////////////////////
   */
   /* unit  = numeric */
   /* scale = 0 */
   /* ref   = 0 */

   bufr_delayed_descript_rep_factor_advanced_cloud = 0;              // 0 = no replication ?????????


   /*
   ///////////////////// 0 08 002 (vertical significance-set to missing to cancel previous value-)  //////////////////
   */
   /* unit  = code table (0, 63 = missing) */
   /* scale = 0 */
   /* ref   = 0 */

   bufr_cloud_vertical_significance_missing = 63;                // cancel previous value  // of BUFR3_UNDEF_VALUE; ????????????????????


   /*
   ////////////////////////////////////// 0 31 000 (short delayed description replication  ICE   /////////////////////
   */
   /* unit  = numeriek (0 = no replivation; 1 = yes replication) */
   /* scale = 0 */
   /* ref   = 0 */

   // NB als icing and/or ice aanwezig is deze op 1 zetten anders op 0

   if ( (num_Is == MAXINT) && (num_Rs == MAXINT) && (num_EsEs == MAXINT) && (num_ci == MAXINT) &&
        (num_Si == MAXINT) && (num_bi == MAXINT) && (num_Di == MAXINT)   && (num_zi != MAXINT) )
   {
      bufr_short_delayed_descript_rep_factor_ice = 0;              // 0 = no replication
   }
   else // wel ice accretion + ice groep
   {
      bufr_short_delayed_descript_rep_factor_ice = 1;              // 1 = yes replication

      /*
      ///////////////////////////////////// 0 20 031 (EsEs- ice deposit thickness)  ////////////////////////////////////////
      */
      /* unit  = meter */
      scale = 2;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      /* NB num_EsEs staat (al) in centimeters -> schalings_factor delen door 100 = meters */

      if (num_EsEs == MAXINT)
         bufr_thickness_ice_accretion = BUFR3_UNDEF_VALUE;
      else
         bufr_thickness_ice_accretion = num_EsEs * (schalings_factor / 100) - ref;;


      /*
      ///////////////////////////////////// 0 20 032 (Rs - rate of ice accretion)   ///////////////////////////////////
      */
      /* unit  = code table (0..4; 7 = missing value) */
      /* scale = 0 */
      /* ref   = 0 */

      /* NB in FM 13 SHIP was geen Rs = 7 aanwezig */

      if (num_Rs == MAXINT)
         bufr_rate_ice_accretion = 7;                          // 7 = missing value
      else
         bufr_rate_ice_accretion = num_Rs;


      /*
      //////////////////////////////////////////// 0 20 033 (Is, cause of ice accretion) /////////////////////////////
      */
      /* unit  = flag table (data width 4 bits) */     // LET OP FLAG TABLE
      /* scale = 0 */
      /* ref   = 0 */

      // NB bit     1 = icing from ocean spray Is = 1
      //    bit     2 = icing from fog         Is = 2   + Is = 3 (nood oplossing)
      //    bit     3 = icing from rain        Is = 4   + Is = 5 (nood oplossing)
      //    bit all 4 = missing

      // NB bovenstaande komt niet goed uit voor Is = 3 icing from spray and fog
      //                                    voor Is = 5 icing from spray and rain
      // daarom maar toegekend aan bit 2 resp bit 3 (nood oplossing)

      // 8 =  1000             ????????????
      // 4 =  0100             ???????????
      // 2 =  0010             ???????????
      // 15 = 1111             ???????????

      if (num_Is == MAXINT)
         bufr_cause_ice_accretion = 15;            // bits: 1111 (all bits set) = missing value
      else if (num_Is == 1)
         bufr_cause_ice_accretion = 8;             // bits: 1000 (bit 1 gezet)
      else if (num_Is == 2 || num_Is == 3)
         bufr_cause_ice_accretion = 4;             // bits: 0100 (bit 2 gezet)
      else if (num_Is == 4 || num_Is == 5)
         bufr_cause_ice_accretion = 2;             // bits: 0010 (bit 3 gezet)
      else
         bufr_cause_ice_accretion = BUFR3_UNDEF_VALUE;


      /*
      /////////////////////////////////// 0 20 034 (ci; sea ice concentration)  //////////////////////////////////////
      */
      /* unit  = code table (0..9; 14 = "unable to report because of etc.."; 31 = missing value) */
      /* scale = 0 */
      /* ref   = 0 */

      // NB ook hier geld weer FM 13 SHIP geeft voor "niet aanwezig" maar ook voor "unable to report etc." een "/"
      //    bij de bufr kan dit gesplitst worden

      if (num_ci >= 0 && num_ci <= 9)
         bufr_sea_ice_concentration = num_ci;
      else if (num_ci == MAXINT && obs_concentration_ice == ci_unable_a)
         bufr_sea_ice_concentration = 14;
      else if (num_ci == MAXINT && obs_concentration_ice == ci_unable_b)
         bufr_sea_ice_concentration = 14;
      else
         bufr_sea_ice_concentration = 15;


      /*
      ///////////////////// 0 20 035 (bi, amount and type of ice, ice of land origin)    ///////////////////////////
      */
      /* unit  = code table (0..9; 14 = "unable to report because of etc.."; 15 = missing value) */
      /* scale = 0 */
      /* ref   = 0 */

      // NB ook hier geld weer FM 13 SHIP geeft voor "niet aanwezig" maar ook voor "unable to report etc." een "/"
      //    bij de bufr kan dit gesplitst worden

      if (num_bi >= 0 && num_bi <= 9)
         bufr_amount_type_ice = num_bi;
      else if(num_bi == MAXINT && obs_land_ice == bi_unable)
         bufr_amount_type_ice = 14;
      else
         bufr_amount_type_ice = 15;


      /*
      //////////////////////////////////////////////// 0 20 036 (zi; ice situation) ////////////////////////////////
      */
      /* unit  = code table (0..9; 30 = "unable to report because of etc.."; 31 = missing value) */
      /* scale = 0 */
      /* ref   = 0 */

      // NB ook hier geld weer FM 13 SHIP geeft voor "niet aanwezig" maar ook voor "unable to report etc." een "/"
      //    bij de bufr kan dit gesplitst worden

      if (num_zi >= 0 && num_zi <= 9)
         bufr_ice_situation = num_zi;
      else if(num_zi == MAXINT && obs_land_ice == zi_unable)
         bufr_ice_situation = 30;
      else
         bufr_ice_situation = 31;


      /*
      //////////////////////////////////////////////// 0 20 037 (Si; ice development) ////////////////////////////////
      */
      /* unit  = code table (0..9; 30 = "unable to report because of etc.."; 31 = missing value) */
      /* scale = 0 */
      /* ref   = 0 */

      // NB ook hier geld weer FM 13 SHIP geeft voor "niet aanwezig" maar ook voor "unable to report etc." een "/"
      //    bij de bufr kan dit gesplitst worden

      if (num_Si >= 0 && num_Si <= 9)
         bufr_ice_development = num_Si;
      else if(num_Si == MAXINT && obs_stage_ice == Si_unable)
         bufr_ice_development = 30;
      else
         bufr_ice_development = 31;


      /*
      //////////////////////////////////////// 0 20 038 (Di, bearing of ice edge  ////////////////////////////////////////
      */
      /* unit  = degrees true */
      scale = 0;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      if (num_Di == 0)                                    // ship in shore or flaw lead
         bufr_ice_edge_bearing = 0 * schalings_factor - ref;
      else if (num_Di == 1)                               // NE
         bufr_ice_edge_bearing = 45 * schalings_factor - ref;
      else if (num_Di == 2)                               // East
         bufr_ice_edge_bearing = 90 * schalings_factor - ref;
      else if (num_Di == 3)                               // SE
         bufr_ice_edge_bearing = 135 * schalings_factor - ref;
      else if (num_Di == 4)                               // south
         bufr_ice_edge_bearing = 180 * schalings_factor - ref;
      else if (num_Di == 5)                               // SW
         bufr_ice_edge_bearing = 225 * schalings_factor - ref;
      else if (num_Di == 6)                               // West
         bufr_ice_edge_bearing = 270 * schalings_factor - ref;
      else if (num_Di == 7)                               // NW
         bufr_ice_edge_bearing = 315 * schalings_factor - ref;
      else if (num_Di == 8)                               // North
         bufr_ice_edge_bearing = 360 * schalings_factor - ref;
      else if (num_Di == 9)                               // not determined (ship in ice)
         bufr_ice_edge_bearing = BUFR3_UNDEF_VALUE;
      else
         bufr_ice_edge_bearing = BUFR3_UNDEF_VALUE;

   } // else (wel ice accretion + ice groep)


   /*
   ////////////////////////////////////// 0 31 000 (short delayed description replication SST  /////////////////////
   */
   /* unit  = numeriek (0 = no replivation; 1 = yes replication) */
   /* scale = 0 */
   /* ref   = 0 */

   if (num_TwTwTw == MAXINT)
   {
      bufr_short_delayed_descript_rep_factor_sst = 0;              // 0 = no replication
   }
   else // wel sst
   {
      bufr_short_delayed_descript_rep_factor_sst = 1;              // 1 = yes replication

      /*
      //////////////////////////////////// 0 02 38 method sst (ss = sign and type sst)  ///////////////////////////////
      */
      /* unit  = code table */
      /* scale = 0 */
      /* ref   = 0 */

      // NB code table 0 02 38 geeft meer mogelijkheden dan FM 13 code (bv digital BT, eventuueel later in input form bijvoegen)

      if (char_Sn_TwTwTw[0] != '\0')
      {
         if (char_Sn_TwTwTw == "0" || char_Sn_TwTwTw == "1")                // intake
            bufr_sst_method = 0;                                            // intake (table 0 02 038)
         else if (char_Sn_TwTwTw == "2" || char_Sn_TwTwTw == "3")           // bucket
            bufr_sst_method = 1;                                            // bucket (table 0 02 038)
         else if (char_Sn_TwTwTw == "4" || char_Sn_TwTwTw == "5")           // hull contact
            bufr_sst_method = 2;                                            // hull contact (table 0 02 038)
         else
            bufr_sst_method = BUFR3_UNDEF_VALUE;;                           // geen andere mogelijkheid in table 0 02 038
      } // if (char_Sn_TwTwTw[0] != '\0');
      else
         bufr_sst_method = BUFR3_UNDEF_VALUE;

      /*
      //////////////////////////////////////////////// 0 22 043 SST (ssTwTwTw)  ////////////////////////////////////////
      */
      /* unit  = K */
      scale = 2;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      /* NB num_TwTwTw staat (al) in  tienden graden -> schalings_factor delen door 10; dus num_TwTwTw / 10 = temp in Celsius */
      bufr_sst = (num_TwTwTw + CELCIUS_TO_KELVIN_FACTOR) * (schalings_factor / 10) - ref;

   } // else (wel sst)


   /*
   //////////////////////////// 0 31 000 (short delayed description replication; WAVES MEASURED  /////////////////////
   */
   /* unit  = numeriek (0 = no replivation; 1 = yes replication) */
   /* scale = 0 */
   /* ref   = 0 */

   if ((num_Pwa == MAXINT) && (num_HwaHwaHwa == MAXINT))
   {
      bufr_short_delayed_descript_rep_factor_waves_measured = 0;              // 0 = no replication
   }
   else // wel measured waves
   {
      bufr_short_delayed_descript_rep_factor_waves_measured = 1;              // 1 = no replication

      /*
      /////////////////////////////////////// 0 22 001 (direction waves measured) ////////////////////////////////////
      */
      /* unit  = degrees true */
      /* scale = 0 */
      /* ref   = 0 */

      bufr_dir_waves_measured = BUFR3_UNDEF_VALUE;                             // voor VOS altijd niet bekend


      /*
      ////////////////////////////////////////// 0 22 011 (period waves measured) /////////////////////////////////////
      */
      /* unit  = sec */
      scale = 0;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      if (num_Pwa != MAXINT)
         bufr_period_waves_measured = num_Pwa * schalings_factor - ref;
      else
         bufr_period_waves_measured = BUFR3_UNDEF_VALUE;


      /*
      ////////////////////////////////////////// 0 22 021 (height waves measured) /////////////////////////////////////
      */
      /* unit  = meter */
      scale = 1;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      // NB Hwa staat in 0.1 meters dus delen door 10 om meters te krijgen
      bufr_height_waves_measured = num_HwaHwaHwa * (schalings_factor / 10) - ref;


   } // else (wel measured waves)


   /*
   //////////////////////////// 0 31 000 (short delayed description replication; WAVES ESTIMATED  /////////////////////
   */
   /* unit  = numeriek (0 = no replivation; 1 = yes replication) */
   /* scale = 0 */
   /* ref   = 0 */

   if (num_Pw1 == MAXINT && num_Hw1 == MAXINT && num_Dw1 == MAXINT &&
       num_Pw2 == MAXINT && num_Hw2 == MAXINT && num_Dw2 == MAXINT)
   {
      bufr_short_delayed_descript_rep_factor_waves_estimated = 0;              // 0 = no replication
   }
   else // dus wel estimated waves
   {
      bufr_short_delayed_descript_rep_factor_waves_estimated = 1;              // 1 = yes replication


      /*
      ////////////////////////////////////// 0 22 002 (dir of wind waves)  ////////////////////////////////////////
      */
      /* unit  = degrees true */
      /* scale = 0 */
      /* ref   = 0 */

      bufr_dir_waves_estimated = BUFR3_UNDEF_VALUE;                      // bij VOS geen dir estimated wind waves


      /*
      //////////////////////////////////// 0 22 012 (period wind waves, PwPw)      ////////////////////////////////////
      */
      /* unit  = sec */
      scale = 0;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      if (num_Pw != MAXINT)
         bufr_period_wind_waves_estimated = num_Pw * schalings_factor - ref;
      else
         bufr_period_wind_waves_estimated = BUFR3_UNDEF_VALUE;


      /*
      ////////////////////////////////////// 0 22 022 (height of wind waves; HwHw)   /////////////////////////////////
      */
      /* unit  = meter */
      scale = 1;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      // NB num_Hw staat in 0.1 meters dus delen door 10 om meters te krijgen
      if (num_Hw != MAXINT)
         bufr_height_wind_waves_estimated = num_Hw * (schalings_factor / 10) - ref;
      else
         bufr_height_wind_waves_estimated = BUFR3_UNDEF_VALUE;


      /*
      ////////////////////////////////////// 0 22 003 (dir first swell 1; dw1dw1)   ///////////////////////////////////
      */
      /* unit  =  degrees true */
      scale = 0;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      bufr_dir_swell_1_estimated = num_Dw1 * (schalings_factor / 10) - ref;


      /*
      //////////////////////////////////// 0 22 013 (period swell 1; Pw1Pw1)   /////////////////////////////////////
      */
      /* unit  = sec */
      scale = 0;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      if (num_Pw1 != MAXINT)
         bufr_period_swell_1_estimated = num_Pw1 * schalings_factor - ref;
      else
         bufr_period_swell_1_estimated = BUFR3_UNDEF_VALUE;


      /*
      //////////////////////////////////////// 0 22 023 (height swell 1; Hw1Hw1)    //////////////////////////////////
      */
      /* unit  = meter */
      scale = 1;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      // NB num_Hw1 staat in 0.1 meters dus delen door 10 om meters te krijgen
      if (num_Hw1 != MAXINT)
         bufr_height_swell_1_estimated = num_Hw1 * (schalings_factor / 10) - ref;
      else
         bufr_height_swell_1_estimated = BUFR3_UNDEF_VALUE;


      /*
      ////////////////////////////////////// 0 22 003 (dir first swell 2; dw2dw2)   ///////////////////////////////////
      */
      /* unit  =  degrees true */
      scale = 0;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      bufr_dir_swell_2_estimated = num_Dw2 * (schalings_factor / 10) - ref;


      /*
      //////////////////////////////////// 0 22 013 (period swell 2; Pw2Pw2)   /////////////////////////////////////
      */
      /* unit  = sec */
      scale = 0;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      if (num_Pw2 != MAXINT)
         bufr_period_swell_2_estimated = num_Pw2 * schalings_factor - ref;
      else
         bufr_period_swell_2_estimated = BUFR3_UNDEF_VALUE;


      /*
      //////////////////////////////////////// 0 22 023 (height swell 2; Hw2Hw)    //////////////////////////////////
      */
      /* unit  = meter */
      scale = 1;
      ref   = 0;
      schalings_factor = (int)pow10(scale);

      // NB num_Hw2 staat in 0.1 meters dus delen door 10 om meters te krijgen
      if (num_Hw2 != MAXINT)
         bufr_height_swell_2_estimated = num_Hw2 * (schalings_factor / 10) - ref;
      else
         bufr_height_swell_2_estimated = BUFR3_UNDEF_VALUE;

   } // else (dus wel estimated waves)


   /*
   /////////////////////////////////////////// 0 20 003 (present weather, ww)  ///////////////////////////////////////
   */
   /* unit  = code table */
   /* scale = 0 */
   /* ref   = 0 */
   if (num_ww != MAXINT)
      bufr_present_weather = num_ww;
   else
      bufr_present_weather = BUFR3_UNDEF_VALUE;


   /*
   //////////////////////////////////////////////// 0 04 024 (time period)   ////////////////////////////////////////
   */
   /* unit  = hour */
   scale = 0;
   ref   = -2048;
   schalings_factor = (int)pow10(scale);


   // DEZE SLAAT TOCH OP W1 en W2 ??????????????????????????

   if (num_GG == 0 || num_GG == 6 || num_GG == 12 || num_GG == 18)
      bufr_weather_time_period = 6 * (schalings_factor / 10) - ref;
   else if (num_GG == 3 || num_GG == 9 || num_GG == 15 || num_GG == 21)
      bufr_weather_time_period = 3 * (schalings_factor / 10) - ref;
   else
      bufr_weather_time_period = BUFR3_UNDEF_VALUE;              // OF VERDER UITWERKEN ??????????????????????????????


   /*
   ////////////////////////////////////////////// 0 20 004 (past weather; W1) ////////////////////////////////////////
   */
   /* unit  = code table */
   /* scale = 0 */
   /* ref   = 0 */

   if (num_W1 != MAXINT)
      bufr_past_weather_1 = num_W1;
   else
      bufr_past_weather_1 = BUFR3_UNDEF_VALUE;


   /*
   ////////////////////////////////////////////// 0 20 005 (past weather; W2) ////////////////////////////////////////
   */
   /* unit  = code table */
   /* scale = 0 */
   /* ref   = 0 */

   if (num_W2 != MAXINT)
      bufr_past_weather_2 = num_W2;
   else
      bufr_past_weather_2 = BUFR3_UNDEF_VALUE;


   /*
   //////////////////////////// 0 31 000 (short delayed description replication; PRECIPITATION  /////////////////////
   */
   /* unit  = numeriek (0 = no replication; 1 = yes replication) */
   /* scale = 0 */
   /* ref   = 0 */

   bufr_short_delayed_descript_rep_factor_precip_measure = 0;              // 0 = no replication


   /*
   //////////////////////// 0 31 000 (short delayed description replication; TEMPERATURE EXTREEM  /////////////////////
   */
   /* unit  = numeriek (0 = no replication; 1 = yes replication) */
   /* scale = 0 */
   /* ref   = 0 */

   bufr_short_delayed_descript_rep_factor_temp_extreme = 0;               // 0 = no replication


   /*
   ///////////////////////////////// 0 07 032 (height wind sensor above marine deck)  /////////////////////////////////
   */
   /* unit  = meter */
   //scale = 2;
   //ref   = 0;
   //schalings_factor = (int)pow10(scale);

   bufr_wind_height_deck = BUFR3_UNDEF_VALUE;


   /*
   ///////////////////////////////////// 0 02 002 (type of wind instruments; iw-gerelateerd)   ///////////////////////////////////
   */
   /* unit  = flag table */
   /* scale = 0 */
   /* ref   = 0 */


   // NB bit     0 = (measured in m/s unless otherwise indicated)
   //    bit     1 = certified instruments
   //    bit     2 = originally measured in knots
   //    bit     3 = originally measured in km/h
   //    bit all 4 = missing


   // 8 =  1000             ????????????
   // 4 =  0100             ???????????
   // 2 =  0010             ???????????
   // 0 =  0000             ???????????
   // 15 = 1111             ???????????


   // NB iw (geeft aan m/s or knots and measured/estimated) kan door TurboWin aangepast zijn
   // als de gerbruiker in maintenance heeft aangegeven output altijd in m/s (dan wordt bc iw = 4 -> iw = 1)
   // iw = 0 : geschat m/s
   //      1 : gemeten m/s
   //      3 : geschat knots (kan iw = 0 worden)
   //      4 : gemeten knots (kan iw = 1 worden)
   //
   // NB bij > 99 knots wordt het altijd m/s (iw = 0 or iw = 1)
   //
   // iw-source : 1 = knots
   //             2 = m/s

   // NAKIJKEN ONDESTAANDE !!!!!!!!!!!!!!!

   if ( (iw == 1 && iw_units == 2) || (iw == 4 && iw_units == 2) )      // iw = 1 of 4 = gemeten; iw_units = 2 = m/s
      bufr_type_wind_instruments = 0;                // bit 0 set
   else if ( (iw == 1 && iw_units == 1) || (iw == 4 && iw_units == 1) ) // iw = 1 of 4 = gemeten; iw_units = 1 = knots
      bufr_type_wind_instruments = 4;                // bit 2 set
   else
      bufr_type_wind_instruments = 15;               // all bits set


   /*
   //////////////////////////////////////////// 0 08 021 (time significance)  ////////////////////////////////////////
   */
   /* unit  = code table */
   /* scale = 0 */
   /* ref   = 0 */

   bufr_wind_time_significance = 2;                  // per definitie voor VOS


   /*
   //////////////////////////////////////////////// 0 04 025 (time period)   ////////////////////////////////////////
   */
   /* unit  = minutes */
   scale = 0;
   ref   = -2048;
   schalings_factor = (int)pow10(scale);

   // NB hier wordt 10 minuten genomen niet helemaal correct omadat after a significance change die periode moet
   //    worden genomen !!!!!!!!!!!!!!

   bufr_wind_time_period = 10 * schalings_factor - ref;


   /*
   //////////////////////////////////////////////// 0 11 001 (wind direction, ff)   ///////////////////////////////////
   */
   /* unit  = degrees true */
   scale = 0;
   ref   = 0;
   schalings_factor = (int)pow10(scale);

   // NB de dd kan 99 (var wind zijn) dit kan niet in bufr

   if (num_dd >= 0 && num_dd <= 36)
      bufr_wind_dir = num_dd * 10;
   else
      bufr_wind_dir = BUFR3_UNDEF_VALUE;


   /*
   //////////////////////////////////////////////// 0 11 002 (wind speed, ff)   ///////////////////////////////////
   */
   /* unit  = m/s */
   scale = 1;
   ref   = 0;
   schalings_factor = (int)pow10(scale);

   // moet altijd in m/s
   if (num_ff != MAXINT)
   {
      if (iw == 3 || iw == 4)              // knots (geschat resp gemeten)
      {
         hulp_wind_speed = floor((double(num_ff) * omzet_kn_ms) + 0.5); // van knopen double naar afgeronde m/s integer
         bufr_wind_speed = hulp_wind_speed * schalings_factor - ref;
      }
      else if (iw == 0 || iw == 1)
         bufr_wind_speed = num_ff * schalings_factor - ref;
      else
         bufr_wind_speed = BUFR3_UNDEF_VALUE;
   }
   else
      bufr_wind_speed = BUFR3_UNDEF_VALUE;


   /*
   //////////////////////////////////////////////// 0 08 021 (time significance)  ///////////////////////////////////
   */
   /* unit  = code table */
   /* scale = 0 */
   /* ref   = 0 */

   bufr_gusts_time_significance = 31;              // 32 = missing value per definitie voor VOS


   /*
   ////////////////////////// 0 31 001 (delayed descriptor replication factor GUSTS)  /////////////////////////////////
   */
   /* unit  = numeric  */
   /* scale = 0 */
   /* ref   = 0 */

   bufr_delayed_descript_rep_factor_gusts = 0;              // 0 = no replication ?????????



   /*******************************************************
   //
   // /////////////////// naar file schrijven /////////////
   //
   *******************************************************/

   /* path + file bepalen */
   turbowin_bufr_input_file = "\0";                     // initialisatie;
   turbowin_bufr_input_file = turbowin_dir;             // turbowin_dir was al bepaald (direct na opstarten van TurboWin)
   turbowin_bufr_input_file += temp_sub_dir;            // TurboWin sub directory
   turbowin_bufr_input_file += bufr_input_file_file;    // bufr_in.txt

   /*  */
   os.open(turbowin_bufr_input_file.c_str(), ios::out);         // open om te schrijven
   if (!os)
   {
      string info = "\0";
      info += "Unable to write: ";
      info += turbowin_bufr_input_file;                         // is absolute path naam
      info += "\n";
      info += "No bufr observation generated";
      MessageBox(info.c_str(), "TurboWin error", MB_OK | MB_ICONERROR);
   } // if (!os)
   else // bufr input file ok
   {
      /* call sign */
      os.write(obs_callsign.c_str(), strlen(obs_callsign.c_str()));
      os.write("\n", strlen("\n"));


      /* ship direction */
      buffer[0] = '\0';
      sprintf(buffer, "%lf", bufr_ship_direction);

      os.write(buffer, strlen(buffer));
      os.write("\n", strlen("\n"));



      os.close();
   } // else








      


//-------------------------------------------------------------------------------------------------------------------------
   /*
   ////////////////////////////////////////////////        ////////////////////////////////////////
   */
   /* unit  = nvt */
   /* scale = nvt */
   /* ref   = nvt */
//--------------------------------------------------------------------------------------------------------------------------

                            /* descriptor   name                                           unit   scale      ref   width   opmerkingen*/



   cdata[0] = "XXXX     ";  /* 0 01 011     call sign; D..D; Character Data                                        9 char  Must be padded to full length! */

   i = 0;
   /* ship meta data */
   data[i] = 0;             /* 0 01 011     call sign (D..D) - Character Data                                                                                            */

   data[++i] = 90;          /* 0 01 012     Ds                                         deg true                                                                          */
   data[++i] = 10;          /* 0 01 013     vs                                              m/s                                                                          */
   data[++i] = 1;           /* 0 02 001     ix                                        vode table                                                                          */
   data[++i] = 2004;        /* 0 04 001     year                                                                                                                         */
   data[++i] = 12;          /* 0 04 002     month                                                                                                                        */
   data[++i] = 28;          /* 0 04 003     day                                                                                                                          */
   data[++i] = 12;          /* 0 04 004     hour                                                                                                                         */
   data[++i] = 0;           /* 0 04 005     minute                                                                                                                       */
   data[++i] = 11350;       /* 0 05 002     LaLaLa                                                    2    -9000           bv  23-30N -> 23.5 * 100 -- 9000 = 11350)     */
   data[++i] = 5950;        /* 0 06 002     LoLoLoLo                                                  2   -18000           b.v. 120-30W -> -120.5 * 100 -- 18000 = 5950) */
   data[++i] = 4180;        /* 0 07 030     height station platform above msl                         1    -4000           bv 18 * 10 -- 4000 = 4180) */
   data[++i] = 4195;        /* 0 07 031     height of barometer above msl                             1    -4000           bv 19.5 * 10 -- 4000 = 4195) */

   /* pressure */
   data[++i] = 10126;       /* 0 10 004     PoPoPoPo                                                 -1         0          bv 1012.6 hPa -> 101260 Pa / 10 = 10126)      */
   data[++i] = 10146;       /* 0 10 051     PPPP; pressure msl                                       -1         0          bv 1014.6 hPa -> 101460 Pa / 10 = 10146)      */
   data[++i] = 506;         /* 0 10 061     ppp                                                      -1      -500          bv> 0.6 hPa -> 60 Pa / 10 = 6 -- 500 = 506)   */
   data[++i] = 2;           /* 0 10 063     a                                        code table */

   /* temperature */
   data[++i] = 80;          /* 0 07 032     height sensor above marine deck for temp                  2         0          bv 0.8 m -> 0.8 * 100 = 80)                   */
   data[++i] = 175;         /* 0 07 033     height of sensor above water surf for temp                1         0          bv 17.5 m -> 17.5 * 10 = 175)                 */
   data[++i] = 28583;       /* 0 12 101     snTTT; dry bulb temp                              K       2         0          bv 12.7 -> (12.7 + 273.15) * 100 = 28585)     */
   data[++i] = 0;           /* 0 02 039     method of wet bulb temp measurement      code table */
   data[++i] = 28586;       /* 0 12 102     swTbTbTb; wet-bulb                                K       2         0          bv 12.8 -> (12.8 + 273.15) * 100 = 28586)     */
   data[++i] = 28587;       /* 0 12 103     snTdTdTd; dew-point                               K       2         0          bv 12.9 -> (12.9 + 273.15) * 100 = 28587)     */
   data[++i] = 100;         /* 0 13 003     relative humidity                                         0         0

   /* visibility */
   data[++i] = 200;         /* 0 07 032 (height sensor above marine deck for visibility; scale=2; ref=0; bv 2 m -> 2 * 100 = 200) */
   data[++i] = 175;         /* 0 07 033 (height of sensor above water surface for visibility; scale=1; ref=0; bv 17.5 m -> 17.5 * 10 = 175) */
   data[++i] = 100;         /* 0 20 001 (VV; scale=-1; ref=0;  bv 1000 m / 10 = 100) */
   data[++i] = 9;         /* 0 07 033 (height sensor above water surface (set to missing to cancel previous value) */

   /* precipitation */
   //data[i] = ;            /* 1 01 000 (delayed replication of 1 descriptor) */
   data[++i] = 0;           /* 0 31 000 (short delayed description replication factor (0 = no replication; 1 = repication) */
   /* ------- */            /* 3 02 034 (precipitaion standaard weg laten voor VOS) */
   data[++i] = 9;         /* 0 07 032 (height sensor above marine deck (set to missing to cancel orevious value) */

   /* clouds */
   //data[i++] = ;          /* 1 01 000 (delayed replication of 1 descriptor */
   data[++i] = 1;           /* 0 31 000 (short delayed description replication factor (0 = no replication; 1 = repication) */
   data[++i] = 60;          /* 0 20 010 (N; %) */
   data[++i] = 0;           /* 0 08 002 (vertical significance) */
   data[++i] = 4;           /* 0 20 011 (Nh; code table) */
   data[++i] = 140;         /* 0 20 013 (h; scale=-1; ref=-40; bv 1000 m -> 1000 / 10 = 100 -- 40 = 140) */
   data[++i] = 38;          /* 0 20 012 (Cl; code table) */
   data[++i] = 20;          /* 0 20 012 (Cm; code table) */
   data[++i] = 10;          /* 0 20 012 (Ch; code table) */
   //data[i] = ;            /* 1 01 000 (delayed replication of 1 descriptor) */
   data[++i] = 0;           /* 0 31 001 (delayed description replication factor) */
   /* ------- */            /* 3 02 005 (NsChshs niet voor VOS) */
   data[++i] = 9;         /* 0 08 002 (vertical significance; set to missing to cancel the previous value) */

   /* icing and ice */
   //data[i] = ;            /* 1 01 000 (delayed replication of 1 descriptor */
   data[++i] = 0;           /* 0 31 000 (short delayed description replication factor (0 = no replication; 1 = repication) */

   /* sst */
   //data[i] = ;            /* 1 01 000 (delayed replication of 1 descriptor */
   data[++i] = 1;           /* 0 31 000 (short delayed description replication factor (0 = no replication; 1 = repication) */
   data[++i] = 1;           /* 0 02 038 (method of sst measurement; code table) */
   data[++i] = 28575;       /* 0 22 043 (sstwTwTw; unit=K; scale=2; ref=0; bv 12.7 -> (12.7 + 273.15) * 100 = 28585) */

   /* waves */
   //data[i] = ;            /* 1 01 000 (delayed replication of 1 descriptor */
   data[++i] = 0;           /* 0 31 000 (short delayed description replication factor (0 = no replication; 1 = repication) */
   /* ------- */            /* 3 02 021 (gemeten golven) */

   //data[i] = ;            /* 1 01 000 (delayed replication of 1 descriptor */
   data[++i] = 1;           /* 0 31 000 (short delayed description replication factor (0 = no replication; 1 = repication) */
   data[++i] = 180;         /* 0 22 002 (direction of wind waves; unit=deg; scale=0; ref=0) */
   data[++i] = 5;           /* 0 22 012 (PwPw; unit=s; scale=0; ref=0) */
   data[++i] = 25;          /* 0 22 022 (hwhw; unit=m; scale=1; ref=0; bv 2.5m -> 2.5 * 10 = 25) */

   //data[i] = ;            /* 1 01 002 (replicate 1 descriptor 2 times) */
   data[++i] = 90;          /* 0 22 003 (dw1dw1); onderdeel van 3 02 023; unit=deg; scale=0; ref=0) */
   data[++i] = 9;           /* 0 22 013 (Pw1Pw1); onderdeel van 3 02 023; unit=s; scale=0; ref=0 */
   data[++i] = 45;          /* 0 22 023 (Hw1Hw1); onderdeel van 3 02 023; unit=m; scale=1; ref=0; bv 4.5m -> 4.5 * 10 = 45)   */
   data[++i] = 130;         /* 0 22 003 (dw2dw2); onderdeel van 3 02 023; unit=deg; scale=0; ref=0) */
   data[++i] = 6;           /* 0 22 013 (Pw2Pw2); onderdeel van 3 02 023; unit=s; scale=0; ref=0 */
   data[++i] = 20;          /* 0 22 023 (Hw2Hw2); onderdeel van 3 02 023; unit=m; scale=1; ref=0; bv 2.0m -> 2.0 * 10 = 20)   */

   /* present and past weather */
   data[++i] = 10;          /* 0 20 003 (ww; code table) */
   data[++i] = 2048;        /* 0 04 024 (time period in hours; unit=hr; scale=0; ref=-2047; bv 1 hr = 1 --2047 = 2048) */
   data[++i] = 5;           /* 0 20 004 (W1; code table) */
   data[++i] = 5;           /* 0 20 005 (W2; code table) */

   /* precipitation measurement */
   //data[i] = ;            /* 1 01 000 (delayed replication of 1 descriptor */
   data[++i] = 0;           /* 0 31 000 (short delayed description replication factor (0 = no replication; 1 = repication) */
   /* ------- */            /* 3 02 040 (niet voor VOS) */

   /* extremen temperature data */
   //data[i] = ;            /* 1 01 000 (delayed replication of 1 descriptor */
   data[++i] = 0;           /* 0 31 000 (short delayed description replication factor (0 = no replication; 1 = repication) */
   /* ------- */            /* 3 02 058 (niet voor VOS) */

   /* wind */
   data[++i] = 2000;        /* 0 07 032 (height of sensor above marine deck for wind; scale=2; ref=0; bv 20 m -> 20 * 100 = 2000) */
   data[++i] = 475;         /* 0 07 033 (height of sensor above water surface; scale=1; ref=0; bv 47.5 m -> 47.5 * 10 = 475) */
   data[++i] = 2;           /* 0 02 002 (iw???; flag table) */
   data[++i] = 2;           /* 0 08 021 (time significance (=2(time averaged)) */
   data[++i] = 2058;        /* 0 04 025 (time period (= -10 minutes, or number min after change; scale=0; ref=-2048; bv 10 min= 10 -- 2048 = 2058) */
   data[++i] = 45;          /* 0 11 001 (dd; unit=deg; scale=0; ref=0) */
   data[++i] = 120;         /* 0 11 002 (ff; unit=m/s; scale=1; ref=0; bv 12 m/s -> 12 * 10 = 120) */
   data[++i] = 9;         /* 0 08 021 (time significance (= missing value) */
   //data[i] = ;            /* 1 03 000 (delayed replication of 3 descriptors) */
   data[++i] = 0;           /* 0 31 001 (delayed descriptor replication factor) */ //NAKIJKEN




   res = BUFR3_encode(3,sect1,NULL,sect3,entries,centries,num_entries,0,&bufdes);
   if(res) {
      printf("*** Fehler %d in BUFR3_encode\n",res);
      printf("*** Fehlermeldung: %s\n",BUFR3_get_error_string());
      exit(1);
   }

   printf("Buffer length: %d\n",bufdes->nbytes);

   fd=fopen("testbufr.data","wb");
   if(!fd) {
      fprintf(stderr,"Can not open testbufr.data");
      perror("fopen");
      exit(1);
   }
   res = BUFR3_write_dwd(fd,bufdes);
   if(res) {
      printf("*** Fehler %d in BUFR3_write_dwd\n",res);
      printf("*** Fehlermeldung: %s\n",BUFR3_get_error_string());
      exit(1);
   }
//   res = BUFR3_write_dwd(fd,bufdes); /* Write it twice */
//   if(res) {
//      printf("*** Fehler %d in BUFR3_write_dwd\n",res);
//      printf("*** Fehlermeldung: %s\n",BUFR3_get_error_string());
//      exit(1);
//   }
   res = BUFR3_close_dwd(fd);
   if(res) {
      printf("*** Fehler %d in BUFR3_write_dwd\n",res);
      printf("*** Fehlermeldung: %s\n",BUFR3_get_error_string());
      exit(1);
   }

   free(bufdes->bufr_data);
   BUFR3_destroy(bufdes);

   return 0; /* make the compiler happy */
}



bool TMyWindow::Coderen_BUFR_Wegschrijven(string bufr_output_path_and_file)
{
   return 0;
}
