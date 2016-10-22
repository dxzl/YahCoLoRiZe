//---------------------------------------------------------------------------
// YahCoLoRiZe - Edit, process and send colorized text into IRC chat-rooms
// via various chat-clients such as mIRC, IceCHat and LeafCHat
//
// Author: Scott Swift
//
// Released to GitHub under GPL v3 October, 2016
//
/*------------------------------------------------------------------
// Author: Mr. Swift 2015
// Thanks to Scott Wisniewski for his 2010 C# subclassed file-dialog
// and to Denis Ponomarenko for his 2007 COSFDlg class implementation.
// I could not have done this without their excellent source-code
// to "show me how".
//
// https://github.com/scottwis/OpenFileOrFolderDialog
// http://www.codeproject.com/Articles/35529/CBrowseFolderDialog-Class
// http://www.codeproject.com/Articles/17916/Simple-MFC-independent-Open-Save-File-Dialog-class
// http://www.codeproject.com/Articles/3235/Multiple-Selection-in-a-File-Dialog
//
  ----------------------------------------------------------------*/
// NOTE: The original file-open dialog's code for Borland C++ Builder is in the source-code files:
// OPENSAVE.CPP and DLGFILE.CPP
//
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646839%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb776913(v=vs.85).aspx#ok_button_options
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms644995%28v=vs.85%29.aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646960(v=vs.85).aspx#_win32_Explorer_Style_Custom_Templates
//
// Control Identifiers for the default Explorer-Style box
//chx1  The read-only check box
//cmb1  Drop-down combo box that displays the list of file type filters
//stc2  Label for the cmb1 combo box
//cmb2  Drop-down combo box that displays the current drive or folder, and that allows the user to select a drive or folder to open
//stc4  Label for the cmb2 combo box
//cmb13  Drop-down combo box that displays the name of the current file, allows the user to type the name of a file to open, and select a file that has been opened or saved recently. This is for earlier Explorer-compatible applications without hook or dialog template. Compare with edt1.
//edt1  Edit control that displays the name of the current file, or allows the user to type the name of the file to open. Compare with cmb13.
//stc3  Label for the cmb13 combo box and the edt1 edit control
//lst1  List box that displays the contents of the current drive or folder
//stc1  Label for the lst1 list box
//IDOK  The OK command button (push button)
//IDCANCEL  The Cancel command button (push button)
//pshHelp  The Help command button (push button)
//
//DM_GETDEFID  You can send this message to a dialog box. The dialog box returns the control identifier of the default push button, if the dialog box has one; otherwise, it returns zero.
//DM_REPOSITION  You can send this message to a top-level dialog box. The dialog box repositions itself so it fits within the desktop area.
//DM_SETDEFID  You can send this message to a dialog box. The dialog box sets the default push button to the control specified by the control identifier in the wParam parameter.
//WM_ACTIVATE  Restores the input focus to the control identified by the previously saved handle if the dialog box is activated. Otherwise, the procedure saves the handle to the control having the input focus.
//WM_CHARTOITEM  Returns zero.
//WM_CLOSE  Posts the BN_CLICKED notification message to the dialog box, specifying IDCANCEL as the control identifier. If the dialog box has an IDCANCEL control identifier and the control is currently disabled, the procedure sounds a warning and does not post the message.
//WM_COMPAREITEM  Returns zero.
//WM_ERASEBKGND  Fills the dialog box client area by using either the brush returned from the WM_CTLCOLORDLG message or with the default window color.
//WM_GETFONT  Returns a handle to the application-defined dialog box font.
//WM_INITDIALOG  Returns zero.
//WM_LBUTTONDOWN  Sends a CB_SHOWDROPDOWN message to the combo box having the input focus, directing the control to hide its drop-down list box. The procedure calls DefWindowProc to complete the default action.
//WM_NCDESTROY  Releases global memory allocated for edit controls in the dialog box (applies to dialog boxes that specify the DS_LOCALEDIT style) and frees any application-defined font (applies to dialog boxes that specify the DS_SETFONT or DS_SHELLFONT style). The procedure calls the DefWindowProc function to complete the default action.
//WM_NCLBUTTONDOWN  Sends a CB_SHOWDROPDOWN message to the combo box having the input focus, directing the control to hide its drop-down list box. The procedure calls DefWindowProc to complete the default action.
//WM_NEXTDLGCTL  Sets the input focus to the next or previous control in the dialog box, to the control identified by the handle in the wParam parameter, or to the first control in the dialog box that is visible, not disabled, and has the WS_TABSTOP style. The procedure ignores this message if the current window with the input focus is not a control.
//WM_SETFOCUS  Sets the input focus to the control identified by a previously saved control window handle. If no such handle exists, the procedure sets the input focus to the first control in the dialog box template that is visible, not disabled, and has the WS_TABSTOP style. If no such control exists, the procedure sets the input focus to the first control in the template.
//WM_SHOWWINDOW  Saves a handle to the control having the input focus if the dialog box is being hidden, then calls DefWindowProc to complete the default action.
//WM_SYSCOMMAND  Saves a handle to the control having the input focus if the dialog box is being minimized, then calls DefWindowProc to complete the default action.
//WM_VKEYTOITEM  Returns zero.
#include <vcl.h>
#include "Main.h"
#pragma hdrstop

#include "FormOFSSDlg.h"

UINT m_buttonWidth = 0;
UINT m_buttonHeight = 0;
HMODULE m_hComCtl32 = NULL;
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TOFSSDlgForm *OFSSDlgForm;
//---------------------------------------------------------------------------
void __fastcall TOFSSDlgForm::FormCreate(TObject *Sender)
{
  TForm* f = static_cast<TForm*>(Sender);
  HWND h = f ? f->Handle : Application->Handle;

  FDlgHandle = NULL;

  FFolderIsSelected = false;

  p_szFileName = new WideChar[OF_BUFSIZE];
  p_szTitleName = new WideChar[OF_BUFSIZE];

  m_hComCtl32 = LoadLibrary("Comctl32.dll");

  if (m_hComCtl32 == NULL || p_szFileName == NULL || p_szTitleName == NULL)
  {
#if DEBUG_ON
    OFDbg->CWrite( "\r\nUnable to load Comctl32.dll!\r\n");
#endif
    return;
  }

  ZeroMemory(&m_ofn, sizeof(OPENFILENAMEW));

  m_ofn.lStructSize       = sizeof(OPENFILENAMEW);
  m_ofn.hInstance         = (void*)GetWindowLong(h, GWL_HINSTANCE);
  m_ofn.lpstrFile         = p_szFileName;   // Set to member variable address
  m_ofn.nMaxFile          = OF_BUFSIZE;
  m_ofn.lpstrFileTitle    = p_szTitleName;   // Set to member variable address
  m_ofn.nMaxFileTitle     = OF_BUFSIZE;
  m_ofn.lCustData         = (unsigned long)this; // lets us access our class data
  m_ofn.lpfnHook          = OFNHookProc;
  m_ofn.lpTemplateName    = NULL;

  p_filterBuf = NULL;

  FFilterCount = 0;
  FFilterIndex = 0;
}
//---------------------------------------------------------------------------
void __fastcall TOFSSDlgForm::FormDestroy(TObject *Sender)
{
  try { if (p_filterBuf != NULL) delete [] p_filterBuf; } catch(...) {}
  try { if (p_szTitleName != NULL) delete [] p_szTitleName; } catch(...) {}
  try { if (p_szFileName != NULL) delete [] p_szFileName; } catch(...) {}

#if DEBUG_ON
  OFDbg->CWrite("\r\nFormDestroy() in TOFSSDlgForm()!\r\n");
#endif
}
//--------------------------------------------------------------------------- FileOpenDlg function ----------------------
bool __fastcall TOFSSDlgForm::ExecuteU(String uFilter, String uInitialDir,
                                                            String uDlgTitle)
{
  m_ofn.lpstrFilter = this->SetFilter(); // sets FFilterCount property var!

  // Try to locate the 1-based filter-index of the extension on our sDefFile
  // in the null-separated list of filters in the lpstrFilter filters-string
  WideString wExt = WideString("*.") + OFUtil->Utf8ToWide(uFilter);
  int iFilter = FindFilter(this->p_filterBuf, wExt.c_bstr(), FFilterCount);
  return ExecuteU(iFilter, uInitialDir, uDlgTitle);
}

bool __fastcall TOFSSDlgForm::ExecuteU(int iFilter, String uInitialDir,
                                                            String uDlgTitle)
{
  return ExecuteW(iFilter, OFUtil->Utf8ToWide(uInitialDir), uDlgTitle);
}

bool __fastcall TOFSSDlgForm::ExecuteW(int iFilter, WideString wInitialDir,
                                                            String uDlgTitle)
// Strings are passed in as UTF-8 and are converted to WideString
{
  p_szFileName[0] = p_szTitleName[0] = C_NULL;

  FFilterIndex = iFilter;
  FInitialDir = wInitialDir;
  FDlgTitle = OFUtil->Utf8ToWide(uDlgTitle);

  m_ofn.lpstrFilter = this->SetFilter(); // sets FFilterCount property var!

  // set to the first filter if the extension passed in is not there
  // (You would usually set the first filter to *.*)
  if (FFilterIndex <= 0 && FFilterCount > 0)
    FFilterIndex = 1;

  FCurrentFilter = WideString(GetFilter((wchar_t*)m_ofn.lpstrFilter,
                                        FFilterIndex, FFilterCount));
  m_ofn.nFilterIndex = FFilterIndex;

  m_ofn.lpstrDefExt = FCurrentFilter.c_bstr();
  m_ofn.lpstrTitle  = FDlgTitle.c_bstr();
  m_ofn.lpstrInitialDir = FInitialDir.c_bstr();

  m_ofn.Flags = OFN_NOTESTFILECREATE|OFN_HIDEREADONLY|OFN_EXPLORER|
        OFN_ENABLEHOOK|OFN_ENABLESIZING|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
  // OFN_ALLOWMULTISELECT|OFN_NODEREFERENCELINKS;

  // If the user specifies a file name and clicks the OK button, the return
  // value is nonzero. The buffer pointed to by the lpstrFile member of the
  // OPENFILENAME structure contains the full path and file name specified
  // by the user.

  return GetOpenFileNameW(&m_ofn); // Execute the system's built-in open dialog...
}
//---------------------------------------------------------------------------
WideChar* __fastcall TOFSSDlgForm::SetFilter(void)
{
  try
  {
    if (FFilters.IsEmpty())
      FFilters = "All files (*.*)|*.*";

    int lenStr = this->FFilters.Length();
    int lenBuf = (lenStr+2)*sizeof(WideChar); // two null-terminators at end!

    // Reallocate filter-string buffer if needed
    try
    {
      if (this->p_filterBuf != NULL)
        delete [] this->p_filterBuf;
    }
    catch(...) {}

    this->p_filterBuf = new WideChar[lenBuf];

    if (this->p_filterBuf == NULL)
      return NULL;

    WideChar c;

    int ii = 0;
    int count = 0;

    // Replace | with '\0' and pad out to end of buffer
    for (; ii < lenStr; ii++)
    {
      c = FFilters[ii+1];

      if (c == '|')
      {
        c = C_NULL;
        count++;
      }

      this->p_filterBuf[ii] = c;
    }

    this->p_filterBuf[ii] = C_NULL;

    // odd # of vertical-bar separators then add 1 to make number divisable by 2
    // and we need an extra null to make two at the end...
    if ((count & 1) != 0)
    {
      count++;
      this->p_filterBuf[ii+1] = C_NULL;
    }

    count /= 2; // two vertical bars for each filter

    FFilterCount = count; // set internal property var

    return this->p_filterBuf;
  }
  catch(...) { return NULL; }
}
//---------------------------------------------------------------------------
wchar_t* __fastcall TOFSSDlgForm::GetFilter(wchar_t* pFilterBuf, int iFilter, int iMax)
// given a 1-based index, returns a pointer to the filter-string. iMax is the
// number of filters in the string.
{
  if (pFilterBuf == NULL || pFilterBuf[0] == C_NULL)
    return pFilterBuf;

  wchar_t* p_start;
  int idx = 0;

  // limit check
  if (iFilter < 1 || iFilter > iMax)
    iFilter = iMax;

  do
  {
    // first part of the filter (used to display in the box)
    while (*pFilterBuf != C_NULL)
      pFilterBuf++;

    pFilterBuf++;

    p_start = pFilterBuf; // start on the part we want!

    // second part of the filter (the actual ".wma" filter)
    while (*pFilterBuf != C_NULL)
      pFilterBuf++;

    idx++;
    pFilterBuf++;

  } while (idx < iFilter && idx < iMax);

  return p_start;
}
//---------------------------------------------------------------------------
int __fastcall TOFSSDlgForm::FindFilter(wchar_t* pFilterBuf, wchar_t* pFilterToFind, int iMax)
// given a pointer to a filter like "*.wpl", returns its 0-based index in pFilterBuf
// to use as nFilterIndex in the OPENFILENAMEW struct on initialization. iMax
// should be set to this->FilterCount.
//
// Return -1 if not found
{
  int idx = 0;
  bool bEqual;
  int ii;

  do
  {
    // first part of the filter (used to display in the box)
    while (*pFilterBuf != C_NULL)
      pFilterBuf++;

    pFilterBuf++;

    bEqual = true;
    ii = 0;

    // second part of the filter (the actual ".wma" filter)
    for(;;)
    {
      if (*pFilterBuf == C_NULL)
        break;

      if (*pFilterBuf++ != pFilterToFind[ii++])
        if (bEqual)
          bEqual = false;
    }

    idx++;

    // bEqual still set? We found our filter!
    if (bEqual)
      return idx;

    pFilterBuf++;

  } while (idx < iMax);

  return 0;
}
//---------------------------------------------------------------------------
// Property getter for this->FileNameUtf8
String __fastcall TOFSSDlgForm::GetFileNameUtf8(void)
{
  return OFUtil->WideToUtf8(this->FileName);
}
//---------------------------------------------------------------------------
// Property getter for this->FileName
WideString __fastcall TOFSSDlgForm::GetFileName(void)
{
  return WideString(p_szFileName);
}
//---------------------------------------------------------------------------
// Property getter
String __fastcall TOFSSDlgForm::GetTitleUtf8(void)
{
  return OFUtil->WideToUtf8(WideString(p_szTitleName));
}
//---------------------------------------------------------------------------
/*
 * returns true if there is a valid shortcut (that we had permission to open)
 */
bool __fastcall TOFSSDlgForm::GetShortcut(WideString &sPath, bool &bIsDirectory)
{
  bIsDirectory = false;

  try
  {
#if DEBUG_ON
    OFDbg->CWrite("\r\nGetShortcut() sPath: " + sPath +"\r\n");
#endif

    // Do this first because we might have a .lnk file with no extension in our list-view control.
    if (OFUtil->DirectoryExistsW(sPath))
    {
      bIsDirectory = true;
      return true;
    }

    if (OFUtil->FileExistsW(sPath))
    {
      if (ExtractFileExt(sPath).LowerCase() == ".lnk")
      {
        sPath = GetShortcutTarget(sPath);

        if (OFUtil->FileExistsW(sPath))
          return true;

        if (OFUtil->DirectoryExistsW(sPath))
        {
          bIsDirectory = true;
          return true;
        }
      }
      else
        return true;
    }
    else
    {
      sPath += ".lnk"; // case where the .lnk extension is not displayed in the list-view control

#if DEBUG_ON
      OFDbg->CWrite("\r\nCheck file: " + sPath + "\r\n");
#endif
      if (OFUtil->FileExistsW(sPath))
      {
        sPath = GetShortcutTarget(sPath);
#if DEBUG_ON
        OFDbg->CWrite("\r\nShortcut target: " + sPath + "\r\n");
#endif

        if (OFUtil->FileExistsW(sPath))
        {
#if DEBUG_ON
          OFDbg->CWrite("\r\nShortcut exists: " + sPath + "\r\n");
#endif
          return true;
        }

        if (OFUtil->DirectoryExistsW(sPath))
        {
#if DEBUG_ON
          OFDbg->CWrite("\r\nDirectory exists: " + sPath + "\r\n");
#endif
          bIsDirectory = true;
          return true;
        }
      }
    }
  }
  catch (...)
  {
#if DEBUG_ON
    OFDbg->CWrite( "\r\nException in GetShortcut()\r\n");
#endif
  }

  return false;
}
//---------------------------------------------------------------------------
WideString __fastcall TOFSSDlgForm::GetShortcutTarget(WideString file)
{
  if (ExtractFileExt(file).LowerCase() != ".lnk")
    return "";

  WideString sOut = "";

  CoInitialize(NULL);

  try
  {
    IShellLinkW* psl = NULL;
    IPersistFile* ppf = NULL;
    TWin32FindDataW* wfs = NULL;

    try
    {
      WideChar Info[MAX_PATH+1];

      CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&psl);
      if (psl != NULL)
      {
        if (psl->QueryInterface(IID_IPersistFile, (void**)&ppf) == 0)
        {
          if (ppf != NULL)
          {
            ppf->Load(file.c_bstr(), STGM_READ);
            psl->GetPath(Info, MAX_PATH, wfs, SLGP_UNCPRIORITY);
            sOut = WideString(Info);
          }
        }
      }
    }
    __finally
    {
      if (ppf) ppf->Release();
      if (psl) psl->Release();
      CoUninitialize();
    }
  }
  catch(...) { }

  return sOut;
}
//---------------------------------------------------------------------------
WideString __fastcall TOFSSDlgForm::GetTextFromCommonDialog(HWND hDlg, UINT msg)
{
  try
  {
    WideChar* buf = NULL;
    WideString str;

    try
    {
      buf = new WideChar[OF_BUFSIZE];
      buf[0] = C_NULL;

      // NOTE: you are "supposed" to be able to get the required buffer size
      // - I tried 0, -1 - null-pointer, a buffer of 40 bytes... function always
      // just returns "1"
//      DWORD ret = SendMessageW(hDlg, msg, 0, NULL);
//#if DEBUG_ON
//      OFDbg->CWrite("\r\nBufsize required: " + String(ret) + "\r\n");
//#endif

      SendMessageW(hDlg, msg, OF_BUFSIZE, (LPARAM)buf);

      str = WideString(buf);
    }
    __finally
    {
      if (buf != NULL)
        try { delete [] buf; } catch (...) {}
    }

    return str;
  }
  catch(...) { return ""; }
}
//---------------------------------------------------------------------------
// **************** Hook function
// callback function (parameter of EnumChildWindows) to find controls
// WM_NOTIFY
//
// wParam (not used)
// lParam points to:
//
// typedef struct _OFNOTIFY {
//   NMHDR          hdr;
//   LPOPENFILENAMEW lpOFN; // OPENFILENAMEW struct
//   LPTSTR         pszFile;
// } OFNOTIFY, *LPOFNOTIFY;
//
// hdr points to:
//
// typedef struct tagNMHDR {
//   HWND     hwndFrom;
//   UINT_PTR idFrom;
//   UINT     code;
// } NMHDR;
//
// code is:
//   CDN_FILEOK
//   CDN_FOLDERCHANGE
//   CDN_HELP
//   CDN_INCLUDEITEM
//     The dialog box sends a CDN_INCLUDEITEM notification for each item in the folder. The dialog box sends this
//     notification only if the OFN_ENABLEINCLUDENOTIFY flag was set when the dialog box was created.
//   CDN_INITDONE
//   CDN_SELCHANGE
//   CDN_SHAREVIOLATION
//   CDN_TYPECHANGE
//
// If the hook procedure returns zero, the dialog box accepts the specified file name and closes.
//
// To reject the specified file name and force the dialog box to remain open, return a nonzero value
//  from the hook procedure and call the SetWindowLong function to set a nonzero DWL_MSGRESULT value.
//
// messages:
// CDM_GETFILEPATH
// CDM_GETFOLDERIDLIST
// CDM_GETFOLDERPATH
// CDM_GETSPEC           // Retrieves the file name (not including the path) of the currently selected file
//                       // Macro: int CommDlg_OpenSave_GetSpec(hwnd, lparam, wparam);
// CDM_HIDECONTROL
// CDM_SETCONTROLTEXT
// CDM_SETDEFEXT
// SETRGBSTRING
//
// The following sample code gets the SETRGBSTRING message identifier and then sets the color selection to blue.
// UINT uiSetRGB;
// uiSetRGB = RegisterWindowMessage(SETRGBSTRING);
// SendMessage(hdlg, uiSetRGB, 0, (LPARAM) RGB(0, 0, 255));
//
// The hook procedure receives notification messages sent from the dialog box. The hook procedure also receives
// messages for any additional controls that you defined by specifying a child dialog template. The hook procedure
// does not receive messages intended for the standard controls of the default dialog box.
//
// If the hook procedure returns zero, the default dialog box procedure processes the message.
// If the hook procedure returns a nonzero value, the default dialog box procedure ignores the message.
// For the CDN_SHAREVIOLATION and CDN_FILEOK notification messages, the hook procedure should return a nonzero
// value to indicate that it has used the SetWindowLong function to set a nonzero DWL_MSGRESULT value.
UINT CALLBACK TOFSSDlgForm::OFNHookProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
    case WM_CLOSE:
    {
      // Call SendMessage(SFDlgForm->DlgHandle, WM_CLOSE, 0, 0); to invoke
      // this hook to destroy the open-file dialog...
      INT r = 0;
      EndDialog(hDlg, (INT)&r);
    }
    break;

    case WM_INITDIALOG:
    {
      // The dialog box procedure should return TRUE to direct the system to set the keyboard focus to the
      // control specified by wParam. Otherwise, it should return FALSE to prevent the system from setting the
      // default keyboard focus.
      // The dialog box procedure should return the value directly. The DWL_MSGRESULT value set by the
      // SetWindowLong function is ignored.
      // The control to receive the default keyboard focus is always the first control in the dialog box that is
      // visible, not disabled, and that has the WS_TABSTOP style. When the dialog box procedure returns TRUE,
      // the system checks the control to ensure that the procedure has not disabled it. If it has been disabled,
      // the system sets the keyboard focus to the next control that is visible, not disabled, and has the WS_TABSTOP.
      // An application can return FALSE only if it has set the keyboard focus to one of the controls of the dialog box.

      // lParam points to OPENFILENAMEW struct with lCustData as our main class-object
      OPENFILENAMEW* p_ofn = (OPENFILENAMEW*)lParam;

      if (p_ofn == NULL)
        break;

      TOFSSDlgForm* p = (TOFSSDlgForm*)p_ofn->lCustData;

      if (p == NULL)
        break;

      // Save the OPENFILENAMEW pointer...
      if (!SetProp(GetParent(hDlg), "OFN", (void*)lParam))
      {
#if DEBUG_ON
        OFDbg->CWrite("\r\nSET Prop Failed in FormOFDlg()\r\n");
#endif
      }
    }
    return TRUE;

    case WM_DESTROY:
    {
      // Get the OPENFILENAMEW pointer...
      OPENFILENAMEW* p_ofn = (OPENFILENAMEW*)GetProp(GetParent(hDlg), "OFN");
      if (p_ofn == NULL) break;

      TOFSSDlgForm* p = (TOFSSDlgForm*)p_ofn->lCustData;
      if (p == NULL) break;

      RemoveProp(GetParent(hDlg), "OFN");
#if DEBUG_ON
      OFDbg->CWrite("\r\nWM_DESTROY in OFNHookProc()\r\n");
#endif
    }
    break;

    case WM_NOTIFY:
    {
      // Get the OPENFILENAMEW pointer...
      OPENFILENAMEW* p_ofn = (OPENFILENAMEW*)GetProp(GetParent(hDlg), "OFN");
      if (p_ofn == NULL) break;

      TOFSSDlgForm* p_osfDlg = (TOFSSDlgForm*)p_ofn->lCustData;

      LPOFNOTIFY p_notify = (LPOFNOTIFY)lParam;

      LRESULT r = p_osfDlg->ProcessNotifyMessage(hDlg, p_notify);

      if (r != 0)
      {
        SetWindowLongW(hDlg, DWL_MSGRESULT, r);
        return r;
      }
    }
    break;

  } // end switch msg
  return FALSE; // Calls default if you return 0
}
//---------------------------------------------------------------------------
int __fastcall TOFSSDlgForm::ProcessNotifyMessage(HWND hDlg, LPOFNOTIFY p_notify)
{
  if (p_notify == NULL)
    return 0;

  // notifyData.hdr.code can be: CDN_FILEOK, CDN_FOLDERCHANGE, CDN_HELP,
  // CDN_INITDONE, CDN_SELCHANGE, CDN_SHAREVIOLATION, CDN_TYPECHANGE.
  switch (p_notify->hdr.code)
  {
    case CDN_SELCHANGE:
    {
      // when a new file is selected, this sets the file-name box and window
      // title to the new file
#if DEBUG_ON
      OFDbg->CWrite("\r\nCDN_SELCHANGE in FormOFDlg()\r\n");
#endif
      HWND hParent = GetParent(hDlg);

      // Show current folder path in window-title-bar and in the file edit-box
      WideString newFile = this->GetTextFromCommonDialog(hParent, CDM_GETFILEPATH);

      if (!newFile.IsEmpty())
      {
        this->FFolderIsSelected =
                   OFUtil->DirectoryExistsW(newFile) ? true : false;

        HWND hFileName = GetDlgItem(hParent, ID_FileName);

        // clear the file-name edit-box when a new folder is opened
        if (hFileName != NULL)
          SetWindowTextW(hFileName, newFile);

        return TRUE;
      }

      this->FFolderIsSelected = false;
    }
    break;

    case CDN_TYPECHANGE:
    {
      if (p_notify->lpOFN == NULL || p_notify->lpOFN->lCustData == NULL)
        break;

      LPOPENFILENAME pOFN = p_notify->lpOFN;
      TOFSSDlgForm* pThis = (TOFSSDlgForm*)pOFN->lCustData;

      int idx = pOFN->nFilterIndex;

      // typechange: user changed filters - we want to append the new
      // filter to FDefFile and set the file edit box to it
      wchar_t* pFilter = pThis->GetFilter((wchar_t*)pOFN->lpstrFilter,
                                                idx, pThis->FilterCount);

      if (pFilter == NULL)
        break;

      if (pFilter[0] == '*') // skip leading *
        pFilter++;

      if (pFilter[0] == C_NULL)
        break;

      pThis->FCurrentFilter = WideString(pFilter); // still has the . on it
      pThis->FFilterIndex = idx;

#if DEBUG_ON
      OFDbg->CWrite("\r\nFormOFDlg CDN_TYPECHANGE: " +
                              String(pThis->FCurrentFilter) + "\r\n");
#endif
    }
    break;

#if DEBUG_ON
    case CDN_INCLUDEITEM:
    {
      OFDbg->CWrite("\r\nCDN_INCLUDEITEM in FormOFDlg()\r\n");
    }
    break;
#endif

    case CDN_FOLDERCHANGE:
    {
      // when a new folder is opened, this sets the file-name box
      // and window title to the new path
#if DEBUG_ON
      OFDbg->CWrite("\r\nCDN_FOLDERCHANGE in FormOFDlg()\r\n");
#endif
      HWND hParent = GetParent(hDlg);

      // Show current folder path in window-title-bar and in the file edit-box
      WideString newFolder =
                this->GetTextFromCommonDialog(hParent, CDM_GETFOLDERPATH);

      if (!newFolder.IsEmpty() && this->FCurrentFolder != newFolder)
      {
        this->FCurrentFolder = newFolder;

        // Set TOFSSDlgForm's window-title to the current folder
        SetWindowTextW(hParent, this->FDlgTitle + ": " + newFolder);

        HWND hFileName = GetDlgItem(hParent, ID_FileName);

        // clear the file-name edit-box when a new folder is opened
        if (hFileName != NULL)
          SetWindowTextW(hFileName, L"");

        return TRUE;
      }
    }
    break;

    case CDN_INITDONE:
    {
      // save the handle for later use by the destructor to send a WM_CLOSE
      this->FDlgHandle = GetParent(hDlg);

#if DEBUG_ON
      OFDbg->CWrite("\r\nTOFSSDlgForm() CDN_INITDONE\r\n");
#endif
      HWND hFileName = GetDlgItem(this->FDlgHandle, ID_FileName);
      if (hFileName != NULL)
        ::SetFocus(hFileName);
    }
    break;
  }

  return 0;
}
//---------------------------------------------------------------------------
/*
static BOOL SetFileNameToSelectedItem(HWND hParent, HWND hListView, HWND hFNCombo, INT selectedIndex)
{
  if (selectedIndex >= 0)
  {
    WideString name = GetListViewItemText(hListView, selectedIndex);

    // Get the OPENFILENAMEW pointer...
    OPENFILENAMEW* p_ofn = (OPENFILENAMEW*)GetProp(hParent, "OFN");
    if (p_ofn == NULL) return false;

    TOFSSDlgForm* p = (TOFSSDlgForm*)p_ofn->lCustData;
    if (p == NULL) return false;

    if (name.IsEmpty() || p->CurrentFolder.IsEmpty()) return false;

    try
    {
      // Fyi: ExpandFileName() also neat function
//      try { sPath = ExtractRelativePath(ExtractFilePath(sListFullPath), sSongFullPath); }
//      catch(...) {}

      WideString sPath = "";

      int len = p->CurrentFolder.Length();
      if (len > 0)
      {
        char c = p->CurrentFolder[p->CurrentFolder.Length()];
        if (c != '\\')
          sPath = p->CurrentFolder + WideString("\\") + name;
        else
          sPath = p->CurrentFolder + name;
      }
      else
        sPath = name;

      if (DirectoryExistsW(sPath))
      {
        SetWindowTextW(hFNCombo, sPath);
#if DEBUG_ON
        OFDbg->CWrite("\r\nJust set hFNCombo to: " + sPath + "\r\n");
#endif
      }

      return true;
    }
    catch(...){ return false; }
  }
  else
    return false;
}
//---------------------------------------------------------------------------
static WideString GetListViewItemText(HWND hListView, int selectedIndex)
{
      LVITEMW lvitem;
      lvitem.mask = LVIF_TEXT;
      WideChar* nativeBuffer = NULL;

      WideString name;

      try
      {
        nativeBuffer = new WideChar[OF_BUFSIZE];
        if (nativeBuffer != NULL)
        {
          lvitem.pszText = nativeBuffer;
          lvitem.cchTextMax = OF_BUFSIZE;
          int length = SendMessage(hListView, LVM_GETITEMTEXTW, selectedIndex, (LPARAM)&lvitem);
          name = WideString(lvitem.pszText, length);
        }
      }
      __finally
      {
        if (nativeBuffer != NULL)
          try { delete [] nativeBuffer; } catch(...) {}
      }

      return name;
}
*/
//-------------------- FileSaveDlg function -------------------
/*
bool __fastcall TOFSSDlgForm::FileSaveDlg(WideString szDefExt, WideString szDlgTitle)
{
  p_szFileName[0] = p_szTitleName[0] = C_NULL;

  for(unsigned int i = 0; i <= 10; i++)
    p_szFileName[i] = p_szTitleName[i] = szUntitled[i];

  m_ofn.lpstrFilter =  this->SetFilter();
  m_ofn.lpstrDefExt = szDefExt.c_bstr();
  m_ofn.lpstrTitle = szDlgTitle.c_bstr();
  m_ofn.Flags = OFN_OVERWRITEPROMPT;

  return GetSaveFileNameW(&m_ofn);
}
*/
//---------------------------------------------------------------------------
void __fastcall TOFSSDlgForm::FormActivate(TObject *Sender)
{
  if (this->FDlgHandle)
    BringWindowToTop(this->FDlgHandle);
}
//---------------------------------------------------------------------------

