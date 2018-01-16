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

#include "TVocht.h"
#include <stdio.h>



Vochtigheid::Vochtigheid()
{
}


float Vochtigheid::SVP(float temp)
{
	float ew = c1 * exp(c2 * temp / (c3 + temp));
	return ew;
}


float Vochtigheid::SVP_ijs(float temp)
{
	float ei = c1_ijs * exp(c2_ijs * temp / (c3_ijs + temp));
	return ei;
}


float Vochtigheid::Bereken_Dauwpunt(float num_Rh, float num_Tdry_total)
{
   float num_Tdew;

   /* bij bereken met onderstaande formule: */
   /* het kan zijn dat num_Tdew = b.v. -0.0000000258 bij Tdry = 0.0 C en r.v. = 100 % */
   /* dit geeft dan een foute Sn (teken Tdew -> negatief i.p.v. 0 of positief */
   /* daarom bij 100% r.v. Tdew gelijk maken aan Tdry */

   if (num_Rh == 100)                                 // 100 % r.v.
      num_Tdew = num_Tdry_total;
   else
   	num_Tdew = c3 / (-1 + c2 / (log((num_Rh / 100) * SVP(num_Tdry_total) / c1)));

	return num_Tdew;
}


float Vochtigheid::fi_intern(float x, float e)
{
	float y = SVP(x);
	float fi = (y - e) / (c5 * y / pow(x + t0, 2) + c4 * SP);
	return fi;
}


float Vochtigheid::Bereken_NatteBol(int num_Rh, float num_Tdry_total)
{
	float f;
	float x = num_Tdry_total;                       // setting of start values
	float e = SVP(num_Tdry_total) * num_Rh / 100;

	do
	{
		f = fi_intern(x , e);
		x = x - f;
		e = e + c4 * SP * f;
	}while (fabs(f) >= eps);

	float num_Twet_total = x;
	return num_Twet_total;
}


void Vochtigheid::Bereken_Dauwpunt_en_RV(float num_Tdry_total, float num_Twet_total, float& num_rv, float& num_Tdew, BOOL ijs_wetbulb, string exposure)
{
   float A_water;
   float A_ice;


   /* uk marine screens werken voor het berekenen van dew point niet met ijs ja dan nee aan natte bol */
   /* als Twet < 0 dan moet bij Marine Screens altijd de boven ijs formule gebruikt worden, */
   /* ongeacht ze al of niet bevroren natte bol hebben gemeld (zie: tabellen in Marine Observers Handbook) */
   if ((exposure == SCREEN) && (num_Twet_total < 0.0))
      ijs_wetbulb = TRUE;                     // kunstmatig; om zo de goede formules te gebruiken


   /* voor sling psychrometers wordt er wel van uitgegaan dat een Twet < 0 wel met ijs maar ook */
   /* zonder ijs aan de natte bol kan voorkomen; ingang is melding van de waarnemer */
   /* al dan niet bevroren natte bol (zie: Handleiding voor het verrichten van meteorologische */
   /* waarnemingen op zee) */


   /* BOVEN WATER */
   if (ijs_wetbulb == FALSE)
	{
      // bepalen psychrometer coefficient (A)
      if (exposure == SCREEN)               // num_Twet_total is hier dus altijd >= 0 !!
         A_water = A_SCREEN_WATER;
      else
         A_water = 0.000653 * (1 + 0.000944 * num_Twet_total);


		num_rv = (SVP(num_Twet_total) - A_water * SP * (num_Tdry_total - num_Twet_total)) / SVP(num_Tdry_total);
		// rh dus nog niet in %, om dit te krijgen * 100

		// relatieve vochtigheid moet liggen tussen 1 - 100 %
		if (num_rv >= 0.01 && num_rv <= 1.0)
		{
			float term = log(num_rv) + (c2 * num_Tdry_total / (c3 + num_Tdry_total));
			num_Tdew = c3 * term / (c2 - term);
		}
		else // dus (num_rv < 0.01 of num_rv > 1.0)
		{
			num_Tdew = MAXINT;                           // ongeldig maken
			num_rv = MAXINT;                             // ongeldig maken
		}
	} //if (ijs_wetbulb == FALSE)

   /* BOVEN ICE */
   else if (ijs_wetbulb == TRUE)
	{
      // bepalen psychrometer coefficient (A)
      if (exposure == SCREEN)
         A_ice = A_SCREEN_ICE;
      else
         A_ice = A_SLING_ICE;


		num_rv = (SVP_ijs(num_Twet_total) - A_ice * SP * (num_Tdry_total - num_Twet_total)) / SVP(num_Tdry_total);

		if (num_rv >= 0.01 && num_rv <= 1.0)
		{
			float term = log(num_rv) + (c2 * num_Tdry_total / (c3 + num_Tdry_total));
			num_Tdew = c3 * term / (c2 - term);
		}
		else // dus (num_rv < 0.01 of num_rv > 1.0)
		{
			num_Tdew = MAXINT;                           // ongeldig maken
			num_rv = MAXINT;                             // ongeldig maken
		}
	} // else (dus ijs aan de nattebol)
}



