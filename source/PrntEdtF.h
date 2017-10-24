//----------------------------------------------------------------------------
//  Project Print
//  
//  Copyright © 1997. All Rights Reserved.
//
//  SUBSYSTEM:    Print Application
//  FILE:         prntedtf.h
//  AUTHOR:       
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TPrintEditFile (TEditFile).
//
//----------------------------------------------------------------------------
#if !defined(prntedtf_h)              // Sentry, use file only if it's not already included.
#define prntedtf_h

#include <owl/editfile.h>

//#include "printapp.rh"            // Definition of all resources.


//{{TEditFile = TPrintEditFile}}
class TPrintEditFile : public TEditFile {
  public:
    TPrintEditFile(TWindow* parent = 0, int id = 0, const char far* text = 0, int x = 0, int y = 0, int w = 0, int h = 0, const char far* fileName = 0, TModule* module = 0);
    virtual ~TPrintEditFile();

//{{TPrintEditFileVIRTUAL_BEGIN}}
  public:
    virtual void Paint(TDC& dc, bool erase, TRect& rect);
    virtual void SetupWindow();
//{{TPrintEditFileVIRTUAL_END}}
//{{TPrintEditFileRSP_TBL_BEGIN}}
  protected:
    void EvGetMinMaxInfo(MINMAXINFO far& minmaxinfo);
//{{TPrintEditFileRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TPrintEditFile);
};    //{{TPrintEditFile}}


#endif  // prntedtf_h sentry.
