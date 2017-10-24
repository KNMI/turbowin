#include <owl\owlpch.h>
#pragma hdrstop

#include "tcall.h"


DEFINE_RESPONSE_TABLE1(TCallsignDialog, TDialog)
	EV_COMMAND(IDHELP, CmHelp),
END_RESPONSE_TABLE;


TCallsignTransfer::TCallsignTransfer()
{
	// initialisatie
	CallsignEditResult[0] = '\0';
}


TCallsignDialog::TCallsignDialog(TWindow* parent, TResId resId, TCallsignTransfer& callsigntransfer,
											string string_obs_callsign)
								  : TDialog(parent, resId)
{
	callsignedit = new TEdit(this, IDC_CALLSIGNEDIT, sizeof(callsigntransfer.CallsignEditResult));

	// filter aanleggen :
	// ABCDEFGHIJKLMNOPQRSTUVWXYZ
	// abcdefghijklmnopqrstuvwxyz
	// 0123456789
	callsignedit -> SetValidator(new TFilterValidator("A-Za-z0-9")); // bld 392 Swan

	string_callsign = string_obs_callsign;
	//string_obs_callsign.assign(string_callsign);

	//callsignedit -> Insert(obs_callsign);

	SetTransferBuffer (&callsigntransfer);
}


TCallsignDialog::~TCallsignDialog()
{
	delete callsignedit;
}


void TCallsignDialog::SetupWindow()
{
	TDialog::SetupWindow();

	callsignedit -> Clear();         // clears static control (of TEdit) otherwise the picked up callsign is doubled
	// in de edit box de uit de file (CALLSIGN.TXT) opgehaalde calsign zetten
	callsignedit -> Insert(string_callsign.c_str());
}


void TCallsignDialog::CmHelp()
{
	MessageBox("Help button clicked", "Callsign", MB_OK);
}






