#include <owl\owlpch.h>
#include "tw_17.rc"
#pragma hdrstop

#include <math.h>                                  // floor functie
#include <values.h>                                // MAXINT


class Vochtigheid
{
  public:
	  Vochtigheid();
	  void Bereken_Dauwpunt_en_RV(float num_Tdry_total, float num_Twet_total, float &num_rv, float &num_Tdew, BOOL ijs_wetbulb, string exposure);
	  float Bereken_Dauwpunt(float num_Rh, float num_Tdry_total);
	  float Bereken_NatteBol(int num_Rh, float num_Tdry_total);

  private:
	  float SVP(float temp);
	  float SVP_ijs(float temp);
	  float fi_intern(float x, float e);
};

