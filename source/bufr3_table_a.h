/**************************************************************************

   bufr3_table_a.h - WMO Table A definitions

   Author: Dr. Rainer Johanni, Dec. 2002

 **************************************************************************/

#ifndef BUFR3_TABLE_A_H
#define BUFR3_TABLE_A_H

/* tab_a_text: Array with table A descriptions */

static char *(tab_a_text[]) = {
   "SURFACE DATA - LAND",
   "SURFACE DATA - SEA",
   "VERTICAL SOUNDINGS (OTHER THAN SATELLITE)",
   "VERTICAL SOUNDINGS (SATELLITE)",
   "SINGLE LEVEL UPPER-AIR DATA (OTHER THAN SATELLITE)",
   "SINGLE LEVEL UPPER-AIR DATA (SATELLITE)",
   "RADAR DATA",
   "SYNOPTIC FEATURES",
   "PHYSICAL/CHEMICAL CONSTITUENTS",
   "DISPERSAL AND TRANSPORT",
   "RADIOLOGICAL DATA",
   "BUFR TABLES, COMPLETE REPLACEMENT OR UPDATE",
   "SURFACE DATA (SATELLITE)",
   "STATUS INFORMATION",
   "RADIANCES (SATELLITE MEASURED)",
   "OCEANOGRAPHIC DATA",
   "IMAGE DATA"
};

/* tab_a_number: Array of the same size as tab_a_text with the numbers
   of the above table A descriptions */

static int tab_a_number[] =
   { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 20, 21, 31, 101 };

#endif
