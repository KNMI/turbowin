//----------------------------------------------------------------------------
//  Project Print
//  
//  Copyright © 1997. All Rights Reserved.
//
//  SUBSYSTEM:    Print Application
//  FILE:         printapp.h
//  AUTHOR:       
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TPrintApp (TApplication).
//
//----------------------------------------------------------------------------
#if !defined(printapp_h)              // Sentry, use file only if it's not already included.
#define printapp_h

#include <owl/opensave.h>
#include <owl/printer.h>

#include "apxprint.h"
//#include "apxprev.h"

#include "printapp.rh"            // Definition of all resources.


//
// FrameWindow must be derived to override Paint for Preview and Print.
//
//{{TDecoratedFrame = TSDIDecFrame}}
class TSDIDecFrame : public TDecoratedFrame {
  public:
    TSDIDecFrame(TWindow* parent, const char far* title, TWindow* clientWnd, bool trackMenuSelection = false, TModule* module = 0);
    ~TSDIDecFrame();
};    //{{TSDIDecFrame}}


//{{TApplication = TPrintApp}}
class TPrintApp : public TApplication {
  private:


  public:
    TPrintApp();
    virtual ~TPrintApp();

    TOpenSaveDialog::TData FileData;                    // Data to control open/saveas standard dialog.
    void OpenFile(const char* fileName = 0);

    // Public data members used by the print menu commands and Paint routine in MDIChild.
    //
    TPrinter*       Printer;                            // Printer support.
    int             Printing;                           // Printing in progress.

//{{TPrintAppVIRTUAL_BEGIN}}
  public:
    virtual void InitMainWindow();
//{{TPrintAppVIRTUAL_END}}

//{{TPrintAppRSP_TBL_BEGIN}}
  protected:
    void CmFileNew();
    void CmFileOpen();
    void CmFilePrint();
    void CmFilePrintSetup();
    void CmFilePrintPreview();
    void CmPrintEnable(TCommandEnabler& tce);
    void CmHelpAbout();
    void EvWinIniChange(char far* section);
//{{TPrintAppRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TPrintApp);
};    //{{TPrintApp}}


#endif  // printapp_h sentry.
