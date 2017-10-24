#ifndef __SENDFILETO_H__
#define __SENDFILETO_H__

#include <mapi.h>

//#define TCHAR char
//#define _T LPCSTR
//#define CString char

#define TCHAR char
#define CString string




class CSendFileTo
{
public:
    bool SendMail(HWND hWndParent,
         CString const &strAttachmentFileName,
         CString const &strSubject=_T(""))
    {
        // The attachment must exist as a file on the system
        // or MAPISendMail will fail, so......
        if (strAttachmentFileName.IsEmpty())
            return false;

        // You may want to remove this check, but if a valid
        // HWND is passed in, the mail dialog will be made
        // modal to it's parent.
        if (!hWndParent || !::IsWindow(hWndParent))
            return false;

        HINSTANCE hMAPI = ::LoadLibraryA(_T("MAPI32.DLL"));
        if (!hMAPI)
            return false;

        // Grab the exported entry point for the MAPISendMail function
        ULONG (PASCAL *SendMail)(ULONG, ULONG_PTR, 
                      MapiMessage*, FLAGS, ULONG);
        (FARPROC&)SendMail = GetProcAddress(hMAPI, 
                              _T("MAPISendMail"));

        if (!SendMail)
            return false;

        TCHAR szFileName[_MAX_PATH];
        TCHAR szPath[_MAX_PATH];
        TCHAR szSubject[_MAX_PATH];
        ::StrCpy(szFileName, strAttachmentFileName.GetString());
        ::StrCpy(szPath, strAttachmentFileName.GetString());
        ::StrCpy(szSubject, strSubject.GetString());

        MapiFileDesc fileDesc;
        ::ZeroMemory(&fileDesc, sizeof(fileDesc));
        fileDesc.nPosition = (ULONG)-1;
        fileDesc.lpszPathName = szPath;
        fileDesc.lpszFileName = szFileName;

        MapiMessage message;
        ::ZeroMemory(&message, sizeof(message));
        message.lpszSubject = szSubject;
        message.nFileCount = 1;
        message.lpFiles = &fileDesc;

        // Ok to send
        int nError = SendMail(0, (ULONG_PTR)hWndParent, 
               &message, MAPI_LOGON_UI|MAPI_DIALOG, 0);

        if (nError != SUCCESS_SUCCESS && 
            nError != MAPI_USER_ABORT && 
            nError != MAPI_E_LOGIN_FAILURE)
              return false;

        return true;
    }
};

#endif






/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////









#if 0
If your customers are using Outlook or Outlook Express as their email client, then you can use the MailTo comamnd with a little help. Here is the C# code I use to programmatically add attachments to emails.

string filename = openFileDialog1.FileName;

// look at the registry mailto command to determine the default email client
RegistryKey key = Registry.ClassesRoot.OpenSubKey(@"mailto\shell\open\command");

if (key != null)
{
// look for Outlook
string mailto_command = (string) key.GetValue("");
if (mailto_command.ToLower().IndexOf("outlook.exe") > 0)
{

// execute mailto: command
string execute = @"mailto:?";
System.Diagnostics.Process.Start(execute);

// delay 1/2 second so that the email client can open
Thread.Sleep(500);

// send keys to add file as an attachment
SendKeys.Send("%(if)" + filename + "{TAB}{TAB}{ENTER}");
}
// look for Outlook Express
else if (mailto_command.ToLower().IndexOf("msimn") > 0)
{
// execute mailto: command
string execute = @"mailto:?";
System.Diagnostics.Process.Start(execute);

// delay 1/2 second so that the email client can open
Thread.Sleep(500);

// send keys to add file as an attachment
SendKeys.Send("%ia" + filename + "{TAB}{TAB}{ENTER}");
}
else // unsupported email client
{

// tell user only Outlook and Outlook Express are supported
MessageBox.Show("The MailTo button only works with Outlook and Outlook Express", "Your default email client is not supported", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
}
}
else // unsupported email client
{

// tell user only Outlook and Outlook Express are supported
MessageBox.Show("The MailTo button only works with Outlook and Outlook Express", "Your default email client is not supported", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
}
#endif

























/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
This Borland C++ version is a simple function that sends attachments, subject and body. The only thing missing I would like is an automatic SEND funcion

// EmailTo.cpp
//---------------------------------------------------------------------------

#include
#include
#include <mapidefs.h>
#include <mapi.h>
#pragma hdrstop

#include "EmailTo.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

int EMailTo(TCustomForm *MainForm, HWND hWndParent, string strAddress, string strName, string strAttachmentFileName,
string strSubject, string strBody)
{ // Send an email with attachments
// Typical call:
// EMailTo(this, m_hWnd, "sales@mycompany.com", "sales", "c:\\Devel_Test_Email\\attachment.txt",
// "Here's the subject", "Here is the body");
//
// by JLB 22 May 2006
//
// This was an adaption of an MFC routine
// There were several routines to SetFocus and other things that I have had real bad experiences with
// so they are all commented out and the Hide() and Show() commands take care of them all
// The original MFC commmands and the equivalent BCB commands commented out as follows:
// The MFC commands are commented out with //MFC// and the equivalent
// Borland commands are commented out with //BCB//
// If a command does both thenit is commented out with //BCB&MFC//
//
HINSTANCE hMail = NULL;
hMail = ::LoadLibraryA("MAPI32.DLL");

if (hMail == NULL)
{
//MessageBox(AFX_IDP_FAILED_MAPI_LOAD);
Application->MessageBox("Failed to load MAPI32.DLL","Email Error - Send Failed",
MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
return 1;
}
assert(hMail != NULL);

ULONG (PASCAL *lpfnSendMail)(ULONG, ULONG, MapiMessage*, FLAGS, ULONG);
(FARPROC&)lpfnSendMail = GetProcAddress(hMail, "MAPISendMail");
if (lpfnSendMail == NULL)
{
//MessageBox(AFX_IDP_INVALID_MAPI_DLL);
Application->MessageBox("Invalid MAPI32.DLL","Email Error - Send Failed",
MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
return 1;
}
assert(lpfnSendMail != NULL);

TCHAR szOriginator[_MAX_PATH];
TCHAR szAddress[_MAX_PATH];
TCHAR szName[_MAX_PATH];
TCHAR szFileName[_MAX_PATH];
TCHAR szPath[_MAX_PATH];
TCHAR szSubject[_MAX_PATH];
//TCHAR szBody[_MAX_PATH];
int iMaxPath = _MAX_PATH; // _MAX_PATH seems to be 260 so increase it as it doesn't seem to do any damage
char szBody[1024]; // as _MAX_PATH seems to be a minimum rather than a maximum and
// // lpszNoteText is a LPTSTR anyway so this could be increased if required
// Set the attachments
//
strcpy(szAddress, "SMTP:"); // Start with SMTP:
strcat(szAddress, strAddress.c_str()); // Now append the actual email address
strcpy(szName, strName.c_str()); // Name in To: box eg Authorization
strcpy(szPath, strAttachmentFileName.c_str()); // Attachment file name
strcpy(szFileName, strAttachmentFileName.c_str()); // If NULL then taken from szPath
strcpy(szSubject, strSubject.c_str()); // Subject
if(strBody.size() > 1023)
{ // See note just above about increasing body size
Application->MessageBox("Text Body Too Large","Email Error - Text Body > 1023 characters",
MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
return 1;
}
strcpy(szBody, strBody.c_str()); // Body separated by "\n" for carriage returns
//
// Set the file description
//
MapiFileDesc fileDesc;
::ZeroMemory(&fileDesc, sizeof(fileDesc));
fileDesc.nPosition = (ULONG)-1;
fileDesc.lpszPathName = szPath;
fileDesc.lpszFileName = szFileName;
//
// make a recipient
//
MapiRecipDesc recipDesc;
::ZeroMemory(&recipDesc, sizeof(recipDesc));
recipDesc.lpszName = szName; // Name in To: box eg Authorization
recipDesc.ulRecipClass = MAPI_TO; // As in MAPI_CC or MAPI_BC etc
recipDesc.lpszAddress = szAddress; // Actual email address prepended with SMTP:
//
//
// make an originator - Not used
//
strcpy(szOriginator, "originator@mycompany.com");
MapiRecipDesc origDesc;
::ZeroMemory(&origDesc, sizeof(origDesc));
origDesc.lpszName = szName; //
origDesc.ulRecipClass = MAPI_ORIG; // As in MAPI_CC or MAPI_BC etc
origDesc.lpszAddress = szOriginator; // Actual email address prepended with SMTP:
//
// prepare the message
//
// Set the Subject
MapiMessage message;
::ZeroMemory(&message, sizeof(message));
message.lpszSubject = szSubject;
message.nFileCount = 1;
message.lpFiles = &fileDesc;
message.nRecipCount = 1;
//message.lpOriginator = &origDesc;
message.lpRecips = &recipDesc;
message.lpszNoteText = szBody;

// prepare for modal dialog box
//MFC// AfxGetApp()->EnableModeless(FALSE);
//BCB// HRESULT STDMETHODCALLTYPE EnableModeless(FALSE);
/////HWND hWndTop;
/////HWND* pParentWnd = HWND::GetSafeOwner(NULL, &hWndTop);
// some extra precautions are required to use
// MAPISendMail as it tends to enable the parent
// window in between dialogs (after the login
// dialog, but before the send not dialog).
//MFC//pParentWnd->SetCapture();
//BCB// SetCapture(pParentWnd);
//BCB&MFC// ::SetFocus(NULL);
//MFC// pParentWnd->m_nFlags |= WF_STAYDISABLED;

MainForm->Hide();

int nError = lpfnSendMail(0, (ULONG_PTR)hWndParent, &message, MAPI_LOGON_UI|MAPI_DIALOG, 0);

// after returning from the MAPISendMail call,
// the window must be re-enabled and focus
// returned to the frame to undo the workaround
// done before the MAPI call.
//BCB&MFC// ::ReleaseCapture();
//MFC// pParentWnd->m_nFlags &= ~WF_STAYDISABLED;

MainForm->Show();

//MFC// pParentWnd->EnableWindow(TRUE);
//BCB&MFC// ::SetActiveWindow(NULL);
//MFC// pParentWnd->SetActiveWindow();
//MFC// pParentWnd->SetFocus();
//MFC// if (hWndTop != NULL)
//MFC// ::EnableWindow(hWndTop, TRUE);
//MFC// AfxGetApp()->EnableModeless(TRUE);

if (nError != SUCCESS_SUCCESS && nError != MAPI_USER_ABORT && nError != MAPI_E_LOGIN_FAILURE)
{
// AfxMessageBox(AFX_IDP_FAILED_MAPI_SEND);
Application->MessageBox("Failed to send email","Email Error - Send Failed",
MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
return 1;
}
::FreeLibrary(hMail);

return 0;
}
=======================================================================
// EmailTo.h
//---------------------------------------------------------------------------
#include

#ifndef EmailToH
#define EmailToH
//---------------------------------------------------------------------------
using namespace std;

int EMailTo(TCustomForm *MainForm, HWND hWndParent, string strAddress, string StrName, string strAttachmentFileName,
string strSubject, string strBody);
#endif








///////////////////////////////////////////////////////////////////////////////////////////////////    //



Needed to change a number of things to get this puppy to work.
But, it worked fine after that.

Here is the code with the changes:

#include "stdafx.h"
#include <<mapi.h>

class CSendFileTo
{
   HWND m_hWndParent;
   enum Error {SUCCESS, NO_ATTACHMENT=0x1001, NOT_A_WINDOW, LIB_LOAD_FAILED, GETPROC_FAILED};

   public:
     CSendFileTo(HWND hWnd)
     {
        m_hWndParent = hWnd;
     }


   unsigned int SendMail( CString const &strAttachmentFileName,
   CString const &strSubject=_T(""))
   {
      if (strAttachmentFileName.IsEmpty())
      return NO_ATTACHMENT;

      if (!m_hWndParent || !::IsWindow(m_hWndParent))
      return NOT_A_WINDOW;

      HINSTANCE hMAPI = ::LoadLibraryA(_T("MAPI32.DLL"));
      if (!hMAPI)
         return LIB_LOAD_FAILED;

      ULONG (PASCAL *SendMail)(ULONG, HWND, MapiMessage*, FLAGS, ULONG);
      (FARPROC&)SendMail = GetProcAddress(hMAPI, _T("MAPISendMail"));

      if (!SendMail)
        return GETPROC_FAILED;

      TCHAR szFileName[_MAX_PATH];
      TCHAR szPath[_MAX_PATH];
      TCHAR szSubject[_MAX_PATH];
      strcpy(szFileName, (LPCTSTR)strAttachmentFileName);
      strcpy(szPath, (LPCTSTR)strAttachmentFileName);
      strcpy(szSubject, (LPCTSTR)strSubject);

      MapiFileDesc fileDesc;
      ZeroMemory(&fileDesc, sizeof(fileDesc));
      fileDesc.nPosition = (ULONG)-1;
      fileDesc.lpszPathName = szPath;
      fileDesc.lpszFileName = szFileName;

      MapiMessage message;
      ZeroMemory(&message, sizeof(message));
      message.lpszSubject = szSubject;
      message.nFileCount = 1;
      message.lpFiles = &fileDesc;

      int nError = SendMail(0, m_hWndParent, &message, MAPI_LOGON_UI|MAPI_DIALOG, 0);

      if (nError != SUCCESS_SUCCESS && nError != MAPI_USER_ABORT && nError != MAPI_E_LOGIN_FAILURE)
         return nError;

      return SUCCESS;
   }


   void ShowError(UINT errCod)
   {
      if (errCod)
      {
         CString str;
         str.Format("Error: %X", errCod);
         AfxMessageBox(str);
      }
   }
};


