//---------------------------------------------------------------------------
// YahCoLoRiZe - Edit, process and send colorized text into IRC chat-rooms
// via various chat-clients such as mIRC, IceCHat and LeafCHat
//
// Author: Scott Swift
//
// Released to GitHub under GPL v3 October, 2016
//
//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

#include "Utils.h"
#include "Optimizer.h"
#include "ThreadOnChange.h"
#include "ConvertToHTML.h"
#include "ConvertFromHTML.h"
#include "ConvertToRTF.h"
#include "ConvertToHLT.h"
#include "DlgColor.h"
#include "DlgStrip.h"
#include "DlgReplaceText.h"
#include "FormOFSSDlg.h"
#include "FormSFDlg.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
TUtils* utils; // System-wide global pointer
//---------------------------------------------------------------------------
// Utils class functions
//---------------------------------------------------------------------------
__fastcall TUtils::TUtils(TComponent* Owner)
// constructor
{
  this->dts = static_cast<TDTSColor*>(Owner);

  // Register clipboard formats
  cbHTML = (unsigned short)RegisterClipboardFormat(HTML_REGISTERED_NAME);
  cbRTF = (unsigned short)RegisterClipboardFormat(RTF_REGISTERED_NAME);
}
//---------------------------------------------------------------------------
void __fastcall TUtils::InitOnChange(TTaeRichEdit* re, TNotifyEvent oce)
//void __fastcall TUtils::InitOnChange(TTaeRichEdit* re, TTaeRichEditChange oce)
{
  SetOldLineVars(re);
  re->LockCounter = 0;
  re->OnChange = oce;
}

void __fastcall TUtils::PushOnChange(TTaeRichEdit* re)
{
  if (re->LockCounter == 0)
  {
    re->SaveOnChange = re->OnChange;
    re->OnChange = NULL;
    WaitForThread();
  }

  re->LockCounter++;
}

void __fastcall TUtils::PopOnChange(TTaeRichEdit* re)
{
  int temp = re->LockCounter;

  if (--temp <= 0)
  {
    temp = 0;
    re->OnChange = re->SaveOnChange;
  }

  re->LockCounter = temp;
}

void __fastcall TUtils::WaitForThread(void)
{
  if (ThreadOnChange != NULL)
  {
    int time = GetTickCount();

    for(;;)
    {
      // Resume the thread and get its previous suspend-count
      int prevSuspendCount = (int)ResumeThread((void*)ThreadOnChange->Handle);

      // exit if it was suspended - that's what we want before we move on
      // It will re-suspend itself when no chars are found...
      if (prevSuspendCount > 0) break;

      if (prevSuspendCount == -1)
      {
        ShowMessage("Error returned by ResumeThread()!");
        break;
      }

      // Check for 5 second timeout
      int newtime = GetTickCount(); // milliseconds
      if (abs(newtime-time) > 5000)
      {
        ShowMessage("Timeout waiting for thread!");
        break;
      }
    }
  }
  return;
}
//---------------------------------------------------------------------------
// Overloaded
void __fastcall TUtils::ShowHex(void)
{
  if (tae->SelLength)
  {
    if (tae->SelLength > 10*16)
    {
      utils->ShowMessageU(DS[77]);
      return;
    }

    if (tae->View == V_IRC || tae->View == V_ORG)
    {
      WideString wTemp;

      if (tae->View == V_ORG)
        wTemp = dts->SL_ORG->Text;
      else
        wTemp = dts->SL_IRC->Text;

      int iFirst, iLast;
      if (GetSelectedZoneIndices(tae, iFirst, iLast))
        ShowHex(wTemp.SubString(iFirst+1, iLast-iFirst));
    }
    else
      ShowHex(tae->SelTextW);
  }
  else
  {
    WideString wTemp = GetClipboardText();

    if (!wTemp.IsEmpty())
      ShowHex(wTemp);
    else
      utils->ShowMessageU(DS[78]);
  }
}

void __fastcall TUtils::ShowHex(WideString S)
{
  this->ShowMessageW(GetHex(S));
}

void __fastcall TUtils::ShowHex(wchar_t* pBuf, int iSize)
{
  this->ShowMessageW(GetHex(pBuf, iSize));
}
//---------------------------------------------------------------------------
// Overloaded...
WideString __fastcall TUtils::GetHex(WideString S)
{
  return GetHex(S.c_bstr(), S.Length());
}

WideString __fastcall TUtils::GetHex(wchar_t* pBuf, int iSize)
{
  WideString TempStr1, TempStr2, TempStr3;
  int LineNum = 1;
  char HexTxt[6]; // 4 hex digits a space and null-terminator

  TempStr1 = String(LineNum) + ": \"";
  TempStr2 = TempStr1;

  bool bInitLine = true;

  for (int ii = 0; ii < iSize; ii++)
  {
    if (pBuf[ii] == C_NULL)
      break;

    if (ii >= 16 && (ii % 16) == 0)
    {
      TempStr1 += "\"\n";
      TempStr2 += "\"\n";
      TempStr3 += TempStr1 + TempStr2;
      LineNum++;
      TempStr1 = String(LineNum) + ": \"";
      TempStr2 = TempStr1;
      bInitLine = true;
    }

    if (pBuf[ii] == C_CR) TempStr1 += "<cr>";
    else if (pBuf[ii] == C_LF) TempStr1 += "<lf>";
    else if (pBuf[ii] == C_TAB) TempStr1 += "<tab>";
    else if (pBuf[ii] == C_FF) TempStr1 += "<ff>";
    else if (pBuf[ii] == CTRL_B) TempStr1 += "<b>";
    else if (pBuf[ii] == CTRL_K) TempStr1 += "<c>";
    else if (pBuf[ii] == CTRL_O) TempStr1 += "<o>";
    else if (pBuf[ii] == CTRL_R) TempStr1 += "<r>";
    else if (pBuf[ii] == CTRL_U) TempStr1 += "<u>";
    else if (pBuf[ii] == CTRL_Z) TempStr1 += "<z>";
    else if (pBuf[ii] == CTRL_F) TempStr1 += "<f>";
    else if (pBuf[ii] == CTRL_S) TempStr1 += "<s>";
    else if (pBuf[ii] == CTRL_PUSH) TempStr1 += "<push>";
    else if (pBuf[ii] == CTRL_POP) TempStr1 += "<pop>";
    else TempStr1 += pBuf[ii];

    sprintf(HexTxt, "%4x ", (unsigned short)pBuf[ii]);
    TempStr2 += String(HexTxt);

    if (bInitLine)
      bInitLine = false;
  }

  // Incomplete line to print?
  if (!bInitLine)
  {
    TempStr1 += "\"\n";
    TempStr2 += "\"\n";
    TempStr3 += TempStr1 + TempStr2;
  }
  return TempStr3;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::GetLocalFontIndex(WideString sFont)
{
  if (sFont.Length() == 0)
    return -1;

  // Look up the input font string and return a 1-based internal index,
  // returns -1 if not found
  sFont = LowerCaseW(sFont);
  for (int ii = 0; ii < FONTITEMS; ii++)
    if (sFont == LowerCaseW(Utf8ToWide(FONTS[ii])))
      return ii+1;

  if (dts->IsCli(C_PALTALK)) return PALTALK_DEF_TYPE;

  return USER_DEF_TYPE;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TUtils::GetLocalFontStringA(int idx)
{
  // Look up the 1-based index and return its associated string
  // returns empty if not found. Note: you may need to convert the
  // returned string to LowerCase to compare it!

  idx--;

  if (idx < 0 || idx >= FONTITEMS)
    return ""; // out of range

  return utils->Utf8ToAnsi(FONTS[idx]);
}

WideString __fastcall TUtils::GetLocalFontStringW(int idx)
{
  idx--;

  if (idx < 0 || idx >= FONTITEMS)
    return ""; // out of range

  return utils->Utf8ToWide(FONTS[idx]);
}

String __fastcall TUtils::GetLocalFontString(int idx)
{
  idx--;

  if (idx < 0 || idx >= FONTITEMS)
    return ""; // out of range

  return String(FONTS[idx]); // return utf-8
}
//---------------------------------------------------------------------------
ONCHANGEW __fastcall TUtils::GetInfoOC(void)
// Get Main edit's info and pointer to appropriate string-list
{
  ONCHANGEW oc = GetInfoOC(tae);

  if (tae->View == V_IRC || tae->View == V_RTF)
  {
    oc.view = tae->View;
    oc.p = dts->SL_IRC;
  }
  else if (tae->View == V_ORG)
  {
    oc.view = V_ORG;
    oc.p = dts->SL_ORG;
  }
  else if (dts->SL_IRC->Count > 0) // try for IRC...
  {
    oc.view = V_IRC;
    oc.p = dts->SL_IRC;
  }
  else if (dts->SL_ORG->Count > 0) // try for ORG...
  {
    oc.view = V_ORG;
    oc.p = dts->SL_ORG;
  }
  else // give up, nothing to restore!
    oc.p = NULL;

  return oc;
}
//---------------------------------------------------------------------------
ONCHANGEW __fastcall TUtils::GetInfoOC(TTaeRichEdit* re, TStringsW* sl)
{
  ONCHANGEW oc = GetInfoOC(re);

  oc.p = sl;

  return oc;
}
//---------------------------------------------------------------------------
ONCHANGEW __fastcall TUtils::GetInfoOC(TTaeRichEdit* re)
{
  ONCHANGEW oc;

  oc.selStart = re->SelStart;
  oc.line = GetLine(re);
  oc.lineCount = re->LineCount;
  oc.length = GetTextLength(re);
  oc.deltaLength = oc.length - re->OldLength;
  oc.deltaLineCount = oc.lineCount - re->OldLineCount;
  oc.view = re->View;
  oc.insert = re->InsertMode;
  if (oc.selStart)
  {
    // caret position before chars were added or after chars were deleted
    int idx = (oc.deltaLength < 0) ? oc.selStart : oc.selStart-oc.deltaLength;
    oc.pos = re->GetPosByIndex(idx);
  }
  else
  {
    oc.pos.x = 0;
    oc.pos.y = 0;
  }

  // null entire oc buffer! 12/7/18
  for (int ii=0; ii < OC_BUFSIZE; ii++)
    oc.c[ii] = C_NULL;

  oc.p = NULL;

  return oc;
}
//---------------------------------------------------------------------------
void __fastcall TUtils::SetOldLineVars(TTaeRichEdit* re, bool bInit)
{
  if (bInit)
  {
    re->OldLineCount = 0;
    re->OldLength = 0;
  }
  else
  {
    re->OldLineCount = re->LineCount;
    re->OldLength = GetTextLength(re);
  }
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::LoadFromStringList(TTaeRichEdit* re, TStringsW* sl, bool bPlainText)
// To load a TTaeRichEdit with RTF text it must be streamed-in!
{
  try
  {
    re->PlainText = bPlainText;
    TMemoryStream* ms = new TMemoryStream();
    sl->SaveToStream(ms); // also converts it to UTF-8
    ms->Position = 0;
    re->Lines->LoadFromStream(ms);
    delete ms;
    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
String __fastcall TUtils::GoFormat(String FormatStr, int Parm1)
{
  return(Format(FormatStr, ARRAYOFCONST((Parm1))));
}
//---------------------------------------------------------------------------
int __fastcall TUtils::WildcardDelete(WideString S)
// Delete multiple files by wildcard.
//
// Example: WildcardDelete(S + WideString(L"*.jpg"));
{
  int Ret = 0x402; // Unspecified error...

  try
  {
    if (!S.IsEmpty())
    {
      wchar_t src[MAX_PATH];
      memset(src, 0, sizeof(src)); // need a double null terminator...
      wcscpy(src, S.c_bstr());

      SHFILEOPSTRUCTW fos;
      memset(&fos, 0, sizeof(fos));

      fos.pFrom = src;
      fos.wFunc = FO_DELETE;
      fos.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;

      // Note this function can have multiple null-separated
      // filenames, but the last file must be terminated with
      // a double NULL "\0\0"!
      Ret = SHFileOperationW(&fos);
    }
  }
  catch(...) {}

  return Ret;
}
//---------------------------------------------------------------------------
// Overloaded
bool __fastcall TUtils::FoundCRLF(WideString S, int &ii, bool bNoInc)
// ii is 0-based index!
{
  return(FoundCRLF(S.c_bstr(), S.Length(), ii, bNoInc));
}

bool __fastcall TUtils::FoundCRLF(wchar_t *buf, int iSize, int &ii, bool bNoInc)
// ii is the present character index. This routine moves the present
// ii up by 1 for cr/lf pairs if bNoInc is false.
// (Also returns true if a form-feed (page-break) is present!)
{
  if (ii+1 < iSize)
  {
    if (buf[ii] == C_LF && buf[ii+1] == C_CR)
    {
      if (!bNoInc)
        ii++;

      return true;
    }

    if (buf[ii] == C_CR && buf[ii+1] == C_LF)
    {
      if (!bNoInc)
        ii++;

      return true;
    }

    if (buf[ii] == C_LF || buf[ii] == C_CR)
      return true;

    return false;
  }

  if (ii < iSize && (buf[ii] == C_LF || buf[ii] == C_CR))
    return true;

  return false;
}
//----------------------------------------------------------------------------
int __fastcall TUtils::FindNextParagraph(int iIndex, int iCharsRead, wchar_t *pBuf)
{
  int ii;

  // move past any current LFCR, CRLF or CR or LF
  for (ii = iIndex; ii < iCharsRead; ii++)
    if (pBuf[ii] != C_CR && pBuf[ii] != C_LF)
      break;

NextLine:

  // move past non LFCR, CRLF or CR or LF
  for (; ii < iCharsRead; ii++)
    if (pBuf[ii] == C_CR || pBuf[ii] == C_LF)
      break;

  // a single CR, LF, CRLF or LFCR constitutes a new line,
  // so continue...

  // only one wchar_t left? not enough for a new paragraph
  if (ii == iCharsRead - 1)
    return iCharsRead;

  // Line?
  if ((pBuf[ii] == C_CR || pBuf[ii] == C_LF) &&
        (pBuf[ii+1] != C_LF && pBuf[ii+1] != C_CR))
  {
    ii++;
    goto NextLine;
  }

  // only two chars left? not enough for a new paragraph
  if (ii == iCharsRead - 2)
    return(iCharsRead);

  // Line?
  if (((pBuf[ii] == C_CR && pBuf[ii+1] == C_LF) ||
              (pBuf[ii] == C_LF && pBuf[ii+1] == C_CR)) &&
                     (pBuf[ii+2] != C_LF && pBuf[ii+2] != C_CR))
  {
    ii += 2;
    goto NextLine;
  }

// move past any current LFCR, CRLF or CR or LF
//  for (; ii < iCharsRead; ii++)
//    if (pBuf[ii] != CR && pBuf[ii] != LF && pBuf[ii] != FF)
//      break;

  // Replaces above with this: 2/2006
  // (Force blank-lines into Next paragraph... why? because
  // when dividing the text into sections by-paragraph, if we keep the
  // empty lines at the end of a paragraph, they get printed before
  // we have a chance to detect them in ConvertToIRC() processing.)
  for (int jj = 0; jj < 2 && ii < iCharsRead; jj++, ii++)
    if (pBuf[ii] != C_CR && pBuf[ii] != C_LF)
      break;

  return ii; // found a paragraph
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::StripTrailingCRLFsA(char* buf, int &iSize)
{
  // Change size to effectively remove any trailing cr/lf chars...
  if (buf == NULL || iSize < 1)
    return false;

  int ii = iSize-1;

  try
  {
    for (; ii >= 0; ii--)
//    if (AnySpecialChar(buf[ii]))
      if (buf[ii] != C_NULL && buf[ii] != C_CR &&
                        buf[ii] != C_LF && buf[ii] != C_FF)
        break;

    if (ii < 0) // All cr/lf?
    {
      buf[0] = C_NULL;
      iSize = 0;
    }
    else
    {
      buf[ii+1] = C_NULL; // Move null back
      iSize = ii+1;
    }

    return true;
  }
  catch(...)
  {
    return false;
  }
}

String __fastcall TUtils::StripTrailingCRLFsA(String s)
{
  // Remove any trailing cr/lf chars...
  int sLen = s.Length();

  while(sLen)
  {
    if (s[sLen] != C_CR && s[sLen] != C_LF && s[sLen] != C_FF)
      break;

    s = s.Delete(sLen, 1);
    sLen--;
  }

  return s;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::StripTrailingCRLFs(wchar_t* &buf, int &iSize)
{
  // Change size to effectively remove any trailing cr/lf chars...
  if (buf == NULL || iSize < 1)
    return false;

  int ii = iSize-1;

  try
  {
    for (; ii >= 0; ii--)
//    if (AnySpecialChar(buf[ii]))
      if (buf[ii] != C_NULL && buf[ii] != C_CR &&
                                 buf[ii] != C_LF && buf[ii] != C_FF)
        break;

    if (ii < 0) // All cr/lf?
    {
      buf[0] = C_NULL;
      iSize = 0;
    }
    else
    {
      buf[ii+1] = C_NULL; // Move null back
      iSize = ii+1;
    }

    return true;
  }
  catch(...)
  {
    return false;
  }
}

WideString __fastcall TUtils::StripTrailingCRLFs(WideString s)
{
  // Remove any trailing cr/lf chars...
  int wLen = s.Length();

  while(wLen)
  {
    if (s[wLen] != C_CR && s[wLen] != C_LF && s[wLen] != C_FF)
      break;

    s = DeleteW(s, wLen, 1);
    wLen--;
  }

  return s;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::CountTrailingCRLFs(WideString s)
{
  // count trailing cr/lf chars...
  int t = s.Length();
  int Count = 0;

  while(t)
  {
    if (s[t] != C_CR && s[t] != C_LF)
      break;

    if (s[t] == C_CR)
      Count++;

    t--;
  }

  return Count;
}
//---------------------------------------------------------------------------
// MAIN ROUTINE THAT INVOKES THE TEXT-HIGHLIGHT CONVERTER
//---------------------------------------------------------------------------
// Overloaded
bool __fastcall TUtils::EncodeHighlight(TStringsW* sl, TTaeRichEdit* re,
                                                  bool bShowStatus)
{
  return EncodeHighlight(sl->Text, re, bShowStatus);
}

bool __fastcall TUtils::EncodeHighlight(WideString s, TTaeRichEdit* re,
                                                  bool bShowStatus)
{
  PushOnChange(re);

  re->Clear();

// NOTE: This was not working using WM_SETTEXT (which I added to
// TaeRichEdit) - so I changed the function to use EM_SETTEXTEX.
// I had to find the SETTEXTEX struct in the windows SDK (RichEdit.h)
// It works... but will it work with Win XP???? Win 7???
  re->TextW = s;
//  re->Text = s;

  PopOnChange(re);

  return EncodeHighlight(re, bShowStatus);
}

bool __fastcall TUtils::EncodeHighlight(TTaeRichEdit* re, bool bShowStatus)
// Replaces the control's text with a version that replaces text-
// format codes with a plain letter colored red.
//
// IRC-coded text should be in the edit-control's window when
// you call this.
{
  PushOnChange(re);

  bool bRet;

  try
  {
    int SaveSelStart = re->SelStart+GetLine(re);

    TMemoryStream* ms = new TMemoryStream();

    Application->ProcessMessages();

    // Writes edit-control's text to a temp-stream as RTF then
    // processes it into ms.
    TConvertToHLT* p = new TConvertToHLT(dts, ms, re);
    p->Execute(bShowStatus);
    delete p;
    re->Clear();

    ms->Position = 0;

    re->PlainText = false;
    re->Lines->LoadFromStream(ms);
    delete ms;

    re->SelStart = SaveSelStart;
    re->Modified = false; // start clean, no manual edits made...

    bRet = true;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nTUtils::Exception in EncodeHighlight()\r\n");
#endif
    bRet = false;
  }

  PopOnChange(re);
  return bRet;
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::DecodeHighlight(TTaeRichEdit* re)
// Takes the rich-edit text in re, creates a copy in a temp
// stream, scans the stream for single chars colored clRed,
// and replaces them with their control-code equivalents,
// the returns a plain-text string containing only the text-format
// codes...
{
  WideString RetStr;

  if (re->LineCount == 0)
    return RetStr; // not really an error...

  PushOnChange(re);

  try
  {
    int SaveSelStart = re->SelStart;
    int SaveSelLength = re->SelLength;

    TMemoryStream* ms = new TMemoryStream();
    re->CopyToStream(ms, false, false, true, false); // save rtf

    int LineCounter = 0;

    bool bAnyRed = false;

    WideString wText = re->TextW;

    int len = wText.Length();

    for (int ii = 1; ii <= len; ii++)
    {
      if (wText[ii] == C_CR)
        LineCounter++;

      re->SelStart = ii-LineCounter-1;
      re->SelLength = 1;

      if (re->SelAttributes->Color == clRed)
      {
        re->SelAttributes->Color = dts->cColor;
        re->SelTextW = WideString(GetHighlightCode(wText[ii]));

        if (!bAnyRed) bAnyRed = true;
      }
    }

    RetStr = re->TextW;

    // Restore original
    if (bAnyRed)
    {
      ms->Position = 0;
      re->PlainText = false;
      re->Lines->LoadFromStream(ms);
    }

    delete ms;

    re->SelStart = SaveSelStart;
    re->SelLength = SaveSelLength;
  }
  catch(...)
  {
    ShowMessage("Exception thrown in DecodeHighlight()");
    RetStr.Empty();
  }

  PopOnChange(re);

  return RetStr;
}
//---------------------------------------------------------------------------
wchar_t __fastcall TUtils::GetHighlightCode(wchar_t in)
{
  // Highlight letters that take the place of control-codes
  // HL_B 'B' // Bold
  // HL_K 'C' // Color
  // HL_O 'O' // All effects off
  // HL_R 'I' // Itallics
  // HL_U 'U' // Underline
  // HL_F 'F' // Font Type
  // HL_S 'S' // Font Size
  // HL_PUSH 'P' // Push
  // HL_POP 'p' // Pop
  // HL_N 'N' // New Page (page Break, Form-Feed)
  // HL_T 'T' // Tab
  wchar_t c;
  switch(in)
  {
    case HL_B: c = CTRL_B; break;
    case HL_K: c = CTRL_K; break;
    case HL_F: c = CTRL_F; break;
    case HL_R: c = CTRL_R; break;
    case HL_O: c = CTRL_O; break;
    case HL_S: c = CTRL_S; break;
    case HL_U: c = CTRL_U; break;
    case HL_PUSH: c = CTRL_PUSH; break;
    case HL_POP: c = CTRL_POP; break;
    case HL_N: c = C_FF; break;
    case HL_T: c = C_TAB; break;
    default: c = in; break;
  }

  return c;
}
//---------------------------------------------------------------------------
wchar_t __fastcall TUtils::GetHighlightLetter(wchar_t in)
{
  // HL_B ('B') // Bold
  // HL_K ('C') // Color
  // HL_O ('O') // All effects off
  // HL_R ('I') // Itallics
  // HL_U ('U') // Underline
  // HL_F ('F') // FontType
  // HL_S ('S') // FontSize
  // HL_PUSH ('P') // Push
  // HL_POP ('p') // Pop
  // HL_N 'N' // New Page (page Break, Form-Feed)
  // HL_T 'T' // Tab
  wchar_t c;
  switch(in)
  {
    case CTRL_B: c = HL_B; break;
    case CTRL_K: c = HL_K; break;
    case CTRL_O: c = HL_O; break;
    case CTRL_R: c = HL_R; break;
    case CTRL_U: c = HL_U; break;
    case CTRL_F: c = HL_F; break;
    case CTRL_S: c = HL_S; break;
    case CTRL_PUSH: c = HL_PUSH; break;
    case CTRL_POP: c = HL_POP; break;
    case C_FF: c = HL_N; break;
    case C_TAB: c = HL_T; break;
    default: c = in; break;
  }

  return c;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::SetFontStyle(TFontStyles style, TTaeRichEdit* re)
{
  PushOnChange(re);

  bool bRet;

  try
  {
    // This is the only way to set these flags without
    // having the background color drop out!
    // TaeEdit->Font->Style = dts->cStyle DOES NOT WORK.
    re->Font->Style.Clear();
    if (style.Contains(fsBold)) re->Font->Style << fsBold;
    if (style.Contains(fsItalic)) re->Font->Style << fsItalic;
    if (style.Contains(fsUnderline)) re->Font->Style << fsUnderline;

    re->DefAttributes->Style.Clear();
//    if (dts->cStyle.Contains(fsBold))
//      re->DefAttributes->Style << fsBold;
//    if (dts->cStyle.Contains(fsItalic))
//      re->DefAttributes->Style << fsItalic;
//    if (dts->cStyle.Contains(fsUnderline))
//      re->DefAttributes->Style << fsUnderline;

    re->SelAttributes->Style.Clear();
//    if (dts->cStyle.Contains(fsBold))
//      re->SelAttributes->Style << fsBold;
//    if (dts->cStyle.Contains(fsItalic))
//      re->SelAttributes->Style << fsItalic;
//    if (dts->cStyle.Contains(fsUnderline))
//      re->SelAttributes->Style << fsUnderline;

    re->DefaultRtfFileFont->Style.Clear();
//    if (dts->cStyle.Contains(fsBold))
//      re->DefaultRtfFileFont->Style << fsBold;
//    if (dts->cStyle.Contains(fsItalic))
//      re->DefaultRtfFileFont->Style << fsItalic;
//    if (dts->cStyle.Contains(fsUnderline))
//      re->DefaultRtfFileFont->Style << fsUnderline;

    re->DefaultTextFileFont->Style.Clear();
//    if (dts->cStyle.Contains(fsBold))
//      re->DefaultTextFileFont->Style << fsBold;
//    if (dts->cStyle.Contains(fsItalic))
//      re->DefaultTextFileFont->Style << fsItalic;
//    if (dts->cStyle.Contains(fsUnderline))
//      re->DefaultTextFileFont->Style << fsUnderline;

    bRet = true;
  }
  catch(...)
  {
    bRet = false;
  }

  PopOnChange(re);
  return bRet;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::SetDefaultFont(TTaeRichEdit* re)
{
  try
  {
    dts->cCharset = USER_DEF_CHARSET;
    dts->cColor = USER_DEF_COLOR;
    dts->cPitch = USER_DEF_PITCH;
    dts->cSize = USER_DEF_SIZE; // 0-99 ONLY!

    if (dts->IsCli(C_PALTALK))
      dts->cFont = Utf8ToWide(PALTALK_DEF_FONT);
    else
      dts->cFont = Utf8ToWide(USER_DEF_FONT);

    dts->cType = GetLocalFontIndex(dts->cFont);

// we allow style for IRC
//    dts->cStyle.Clear();

    if (!SetRichEditFont(re))
      return false;

    return true;
  }
  catch(...) {return false;}
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::SetRichEditFont(TTaeRichEdit* re, WideString FontName)
{
  PushOnChange(re);

  bool bRet;

  // Basic IRC Clients can't do fonts!
  if (!dts->IsYahTrinPal())
    FontName = USER_DEF_FONT;

  try
  {
    TFontCharset TempCS;
    WideString TempName;

    if (FontName.Length() > 0)
    {
      TempName = FontName;
      TempCS = OEM_CHARSET;
    }
    else
    {
      TempName = dts->cFont;
      TempCS = dts->cCharset;
    }

    // Use DefAttributes to discover or set the default font
    // characteristics that the rich edit control uses for newly
    // inserted text. These are the characteristics of the text
    // before any special attributes have been applied. Once any
    // special attributes are applied to a portion of text, no text
    // from that point on is considered to have the default attributes,
    // even if the attributes match the DefAttributes.

    // For example, if a rich edit control has had no attributes applied,
    // the entire text has the default attributes. Selecting a portion
    // of text in the middle and applying a specific attribute
    // (such as a font change) results in three sections: a first
    // section with the default attributes, a middle section with the
    // applied attribute, and a final section with a non-default set
    // of attributes that match the default attributes. Changing the
    // DefAttributes property affects only the first section.

    // When inserting new text, the font characteristics of the new
    // text will match the font characteristics at the cursor position,
    // or, if the typing replaces a selection, the font characteristics
    // of the selected text. New text will only have the default attributes
    // when typed into the section of text that has the default attributes.

    // "Pitch" describes the width of a character. Pitch equals the number of
    // characters that can fit side-by-side in 1 inch; for example, 10 pitch
    // equals 10 characters-per-inch or 10 CPI. Pitch is a term generally used
    // with non-proportional (fixed-width) fonts.

    // "Point" is a unit of measurement used in typography that is equal to
    // 1/72 inch. It is used primarily for representing the height of
    // characters and the amount of space between lines, also known as leading.

    // "Twip" is a unit of measurement equal to 1/20th of a printers point.
    // There are 1440 twips to and inch, 567 twips to a centimeter.

    // "Pica" refers to a unit of measurement equal to 1/6 of an inch or 12
    // points. Pica and points can be used interchangeably and many
    // typographers use pica as their standard unit of measurement.

    // Pitch = 120/Points. For example: 10 point = 120 / 10 = 12 Pitch.
    // Points = 120/Pitch. For example: 12 Pitch = 120 / 12 = 10 Points.
    // Twips = 20*Points. For example: 10 Points = 20 * 10 Points = 200 Twips.
    // Pica = Points/12. For example: 24 Points = 24 Points / 12 = 2 Pica.

    re->DefAttributes->BackColor = clWindow;
    re->DefAttributes->Name = TempName;
    re->DefAttributes->Charset = TempCS;
    re->DefAttributes->Color = dts->cColor;
    re->DefAttributes->Pitch = dts->cPitch;
    re->DefAttributes->Size = dts->cSize;

    // The default font used by the control.
    re->Font->BackColor = clWindow;
    re->Font->Name = TempName;
    re->Font->Charset = TempCS;
// This makes plain text invisible!
//    re->Font->Color = dts->cColor;
    re->Font->Pitch = dts->cPitch;
    re->Font->Size = dts->cSize;
    SetTabs(re, re->TabWidth);

    // Selected region attributes
    // (color used when typing into a cleared edit-control)
    re->SelAttributes->BackColor = clWindow;
    re->SelAttributes->Name = TempName;
    re->SelAttributes->Charset = TempCS;
    re->SelAttributes->Color = dts->cColor;
    re->SelAttributes->Pitch = dts->cPitch;
    re->SelAttributes->Size = dts->cSize;

    // Determines the default font used when opening RTF files.
    re->DefaultRtfFileFont->BackColor = clWindow;
    re->DefaultRtfFileFont->Name = TempName;
    re->DefaultRtfFileFont->Charset = TempCS;
    re->DefaultRtfFileFont->Color = dts->cColor;
    re->DefaultRtfFileFont->Pitch = dts->cPitch;
    re->DefaultRtfFileFont->Size = dts->cSize;

    // Determines the default font used when opening non-RTF files.
    re->DefaultTextFileFont->BackColor = clWindow;
    re->DefaultTextFileFont->Name = TempName;
    re->DefaultTextFileFont->Charset = TempCS;
    re->DefaultTextFileFont->Color = dts->cColor;
    re->DefaultTextFileFont->Pitch = dts->cPitch;
    re->DefaultTextFileFont->Size = dts->cSize;

    SetFontStyle(dts->cStyle, re);

    bRet = true;
  }
  catch(...) {bRet = false;}

  PopOnChange(re);
  return bRet;
}
//---------------------------------------------------------------------------
void __fastcall TUtils::SetTabs(TTaeRichEdit* re, int width)
{
  utils->PushOnChange(re);
  re->TabWidth = width; // set all tab-stops to user's value
  utils->PopOnChange(re);
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::IsRtfIrcOrgView(void)
{
  return IsRtfIrcView() || IsOrgView();
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::IsRtfIrcView(void)
{
  return IsRtfView() || IsIrcView();
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::IsIrcOrgView(void)
{
  return IsOrgView() || IsIrcView();
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::IsOrgView(void)
{
  return ValidORG() && tae->View == V_ORG;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::IsIrcView(void)
{
  return ValidIRC() && tae->View == V_IRC;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::IsRtfView(void)
{
  return ValidIRC() && tae->View == V_RTF;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::ValidORG(void)
{
  return dts->SL_ORG != NULL && dts->SL_ORG->Count > 0;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::ValidIRC(void)
{
  return dts->SL_IRC != NULL && dts->SL_IRC->Count > 0;
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::MoveMainTextToString(void)
// Given tae, this routine will take a selected zone or all of
// the text and return the mapped IRC codes from either SL_IRC or SL_ORG
// depending on ViewType. For other views we just return the text from
// TaeEdit as is.
{
  WideString Temp;
  wchar_t* pBuf = NULL;
  int iSize;

  try
  {
    if (MoveMainTextToBuffer(pBuf, iSize))
      if (iSize > 0)
        Temp = WideString(pBuf, iSize);
  }
  __finally
  {
    try
    {
      if (pBuf != NULL)
        delete [] pBuf;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: MoveMainTextToString()\r\n");
#endif
    }
  }

  return Temp;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::MoveMainTextToBuffer(wchar_t* &pBuf, int &iSize)
// Call with an undefined buffer!!!!
// Don't forget to delete it....
//
// Returns iSize as the size of the buffer in chars, minus 1 char
// iSize does NOT include the +1 wchar_t we add for the NULL!
{
  iSize = 0;
  pBuf = NULL;

  bool bIRC = IsRtfIrcView();
  bool bORG = IsOrgView();

  try
  {
    if (bIRC || bORG)
    {
      TStringsW* sl;

      if (bIRC)
        sl = dts->SL_IRC;
      else if (bORG)
        sl = dts->SL_ORG;
      else
        return false;

      WaitForThread();

      int iFirst, iLast;

      int LineIdx = GetLine(tae);

      if (tae->SelLength)
      {
        if (tae->View == V_RTF)
        {
          // Map the start and end indices of the RTF selected text to the
          // text in the buffer
          PUSHSTRUCT psFirst, psLast;
          if (!GetIndicesAndStates(sl, tae, psFirst, psLast))
            return false;

          WideString LeadingState = PrintStateString(psFirst, true);
          WideString TrailingState = PrintStateString(psLast, false);

          iFirst = psFirst.index;
          iLast = psLast.index;

          // Need to copy the codes from re->TextW?
          WideString Txt = LeadingState +
                  sl->Text.SubString(iFirst+1, iLast-iFirst) + TrailingState;

          iSize = Txt.Length();

          pBuf = new wchar_t[iSize+1];

          // wcscpy won't copy null if source string is empty
          pBuf[0] = C_NULL;

          if (!Txt.IsEmpty())
            wcscpy(pBuf, Txt.c_bstr());

          return true;
        }
        // else V_IRC

        int OrigSelCRs = CountCRs(tae->SelTextW);

        // V_IRC or V_ORG
        // Map the start and end indices of the IRC-code-view selected
        // text to the text in the buffer
        iFirst = tae->SelStart+LineIdx;
        iLast = iFirst+tae->SelLength+OrigSelCRs;
      }
      else
      {
        iFirst = 0;
        iLast = sl->TotalLength;
      }

      // Move the the mapped text/codes to a processing
      // buffer... if nothing was selected, move entire
      // contents of pSaveBuffer
      //
      // iSize will be returned as one greater than the actual
      // # of useful chars and includes the nullchar!
      int ii, jj;
      iSize = iLast-iFirst;

//#if DEBUG_ON
//      dts->CWrite("\r\niFirst: " + String(iFirst) + " iLast: " +
//                String(iLast) + " iSize: " + String(iSize) + "\r\n");
//#endif

      pBuf = new wchar_t[iSize+1];

      wchar_t* pText = sl->GetTextBuf();

      if (pText != NULL && pBuf != NULL)
      {
        for (ii = iFirst, jj = 0; ii < iLast; ii++, jj++)
          pBuf[jj] = pText[ii];

        pBuf[jj] = C_NULL;
      }
#if DEBUG_ON
      else
        dts->CWrite("\r\nNULL pointer(s) in TUtils::MoveMainTextToBuffer()!\r\n");
#endif

      try
      {
        delete [] pText;
      }
      catch(...)
      {
#if DEBUG_ON
        dts->CWrite("\r\nUnable to deallocate pText in: MoveMainTextToBuffer()\r\n");
#endif
      }
    }
    else
    {
      int textLen = tae->SelLength ? tae->SelLength : tae->TextLength;

      if (textLen > 0)
      {
        // need to add 1 for the null or it will truncate the text!
        int maxChars = textLen+1;
        pBuf = new wchar_t[maxChars];

        int charsCopied;

        if (tae->SelLength > 0)
          charsCopied = tae->GetSelTextBufW(pBuf, maxChars*sizeof(wchar_t));
        else
          charsCopied = tae->GetTextBufW(pBuf, maxChars*sizeof(wchar_t));
        //ShowMessage("charsCopied (not including null):" + String(charsCopied) +
        //"\r\nmaxChars (including null):" + String(maxChars) +
        //"\r\ntae->LineCount:" + String(tae->LineCount) +
        //"\r\ntae->TextLength:" + String(tae->TextLength) +
        //"\r\ntae->SelLength:" + String(tae->SelLength));
        iSize = textLen;
      }
    }

    return true;
  }
  catch(...)
  {
    // "Error In "
    utils->ShowMessageU(String(DS[95]) + "MoveMainTextToBuffer()");
    return false;
  }
}
//---------------------------------------------------------------------------
// Create a new buffer of wchar_t and move text from iFirst to iLast into it
// from pSource. Returns a new buffer and iSize is returned by-reference.
wchar_t* __fastcall TUtils::MoveTextToBuffer(wchar_t* pSource, int iFirst,
                                                      int iLast, int &iSize)
{
  try
  {
    iSize = iLast-iFirst; // return by reference
    wchar_t* pDest = new wchar_t[iSize+1];
    int jj = 0;
    for (int ii = iFirst; ii < iLast; ii++, jj++)
      pDest[jj] = pSource[ii];
    pDest[jj] = C_NULL;
    return pDest;
  }
  catch(...) { return NULL; }
}
//---------------------------------------------------------------------------
// Overloaded
bool __fastcall TUtils::GetIndicesAndStates(TStringsW* sl, TTaeRichEdit* re,
                                      PUSHSTRUCT &psFirst, PUSHSTRUCT &psLast)
{
  if (!sl)
    return false;

  long iFirst, iLast;
  re->GetSelFirstLast(iFirst, iLast); // Counts cr/lf as 1 char!

  int msSize = sl->TotalLength;
  wchar_t* pBuf = sl->GetTextBuf();

  psFirst.index = iFirst;
  psLast.index = iLast;

  bool bRet = GetIndicesAndStates(pBuf, msSize, psFirst, psLast);

  try
  {
    delete [] pBuf;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nUnable to deallocate pBuf in: GetIndicesAndStates()\r\n");
#endif
  }

  return bRet;
}

bool __fastcall TUtils::GetIndicesAndStates(WideString S, TTaeRichEdit* re,
                                      PUSHSTRUCT &psFirst, PUSHSTRUCT &psLast)
{
  long iFirst, iLast;
  re->GetSelFirstLast(iFirst, iLast); // Counts cr/lf as 1 char!

  psFirst.index = iFirst;
  psLast.index = iLast;

  return GetIndicesAndStates(S.c_bstr(), S.Length(), psFirst, psLast);
}

bool __fastcall TUtils::GetIndicesAndStates(WideString S, PUSHSTRUCT &psFirst,
                                                            PUSHSTRUCT &psLast)
{
  return GetIndicesAndStates(S.c_bstr(), S.Length(), psFirst, psLast);
}

bool __fastcall TUtils::GetIndicesAndStates(wchar_t* pBuf, int iSize,
                  TTaeRichEdit* re, PUSHSTRUCT &psFirst, PUSHSTRUCT &psLast)
{
  long iFirst, iLast;
  re->GetSelFirstLast(iFirst, iLast); // Counts cr/lf as 1 char!

  psFirst.index = iFirst;
  psLast.index = iLast;

  bool bRet = GetIndicesAndStates(pBuf, iSize, psFirst, psLast);

  return bRet;
}

bool __fastcall TUtils::GetIndicesAndStates(wchar_t* pBuf, int iSize,
                                    PUSHSTRUCT &psFirst, PUSHSTRUCT &psLast)
// Scan irc text from 0 up to iFirst real-index setting the state
// and continue to iLast setting endstate.
//
// chars must exist at psFirst.index and psLast.index or we return false
// CR/LF is counts as 1 char! (we presume a cr/lf or lf/cr)
{
  int iFirst = psFirst.index;
  int iLast = psLast.index;

  // Must have a valid buffer with at least one wchar_t to scan...
  if (!pBuf || iSize <= 0 || iFirst < 0 || iLast < 0 || iLast < iFirst)
    return false;

  psFirst.Clear();
  psLast.Clear();

  PUSHSTRUCT* psPtr = &psFirst;

  try
  {
    bool bFreezeFirst = false;

    int Idx = 0;
    int CrLfCounter = 0;

    int ii = 0;

    for(; ii < iSize; ii++)
    {
      if (pBuf[ii] == CTRL_K)
      {
        int fg = NO_COLOR;
        int bg = NO_COLOR;

        ii += this->CountColorSequence(pBuf, ii, iSize, fg, bg);

        if (fg != NO_COLOR)
          psPtr->fg = fg;

        if (bg != NO_COLOR)
          psPtr->bg = bg;

        continue;
      }

      if (pBuf[ii] == CTRL_F)
      {
        int fontType = CountFontSequence(pBuf, ii, iSize);

        if (fontType >= 0)
        {
          psPtr->fontType = fontType;
          ii += 2;
        }

        continue;
      }

      if (pBuf[ii] == CTRL_S)
      {
        int fontSize = CountFontSequence(pBuf, ii, iSize);

        if (fontSize >= 0)
        {
          psPtr->fontSize = fontSize;
          ii += 2;
        }

        continue;
      }

      if (pBuf[ii] == CTRL_B)
        psPtr->bBold = !psPtr->bBold;
      else if (pBuf[ii] == CTRL_U)
        psPtr->bUnderline = !psPtr->bUnderline;
      else if (pBuf[ii] == CTRL_R)
        psPtr->bItalics = !psPtr->bItalics;
      else if (pBuf[ii] == CTRL_PUSH)
      {
        psPtr->pushCounter++;
        psPtr->bPush = true;
      }
      else if (pBuf[ii] == CTRL_POP)
        psPtr->bPop = true;

      if (pBuf[ii] == C_CR)
        CrLfCounter++; // Keep count of cr/lfs
      else if (AnySpecialChar(pBuf[ii]))
        continue;

      if (!bFreezeFirst)
      {
        if (Idx >= iFirst)
        {
          bFreezeFirst = true;
          psFirst.index = ii; // Set iFirst
          psFirst.crlfs = CrLfCounter;
          psLast = psFirst; // copy what we've got so-far
          psPtr = &psLast; // Change pointer to psLast
          CrLfCounter = 0; // Reset CrLf counter

          if (Idx >= iLast) break;
        }
      }
      else if (Idx >= iLast)
        break;

      Idx++;
    }

    psLast.index = ii; // Set iLast
    psLast.crlfs = CrLfCounter;

    return true;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: GetIndicesAndStates()\r\n");
#endif
    return false;
  }
}
//---------------------------------------------------------------------------
// Overloaded

int __fastcall TUtils::GetState(WideString wIn, WideString &State, int Idx,
                       bool bLeading, bool bCopyAfgAbg, bool bTrackSpaceColorChanges)
// S is the string of raw-codes (see below)
{
  return GetState(wIn.c_bstr(), wIn.Length(), State, Idx, bLeading,
                                          bCopyAfgAbg, bTrackSpaceColorChanges);
}

int __fastcall TUtils::GetState(wchar_t* pBuf, int iSize, WideString &State,
                    int Idx, bool bLeading, bool bCopyAfgAbg, bool bTrackSpaceColorChanges)
// pBuf is the buffer of raw-codes.
// State is returned with the state formated into a WideString.
// Idx is the 0-based "Real Character index" up to which we record the state.
//
// Idx can also be:
//
//    STATE_MODE_FIRSTCHAR  0
//    STATE_MODE_LASTCHAR  -1
//    STATE_MODE_ENDOFBUF  -2
//
// If bTrackSpaceColorChanges is true we continue recording to the first non-space
// wchar_t past Idx. If bCopyAfgAbg is set we use Afg/Abg to replace NO_COLOR.
//
// CR/LFs are ignored and are NOT counted as one char!
//
// returns the 0-based index into pBuf we scan up to, -1 on failure.
{
  State.Empty();

  try
  {
    // Set the cumulative text state at real index Idx
    PUSHSTRUCT ps;
    int RetIdx = SetStateFlags(pBuf, iSize, Idx, ps, bTrackSpaceColorChanges);

    // Print a string of initial state
    // (default for bCopyAfgAbg is "true")
    if (bCopyAfgAbg)
    {
      if (ps.fg == NO_COLOR)
        ps.fg = dts->Afg;

      if (ps.bg == NO_COLOR)
        ps.bg = dts->Abg;
    }

    State = PrintStateString(ps, bLeading);
    return RetIdx;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: GetState()\r\n");
#endif
    return -1;
  }
}

int __fastcall TUtils::GetState(WideString wIn, PUSHSTRUCT &psState, int Idx,
                           bool bCopyAfgAbg, bool bTrackSpaceColorChanges)
{
  return GetState(wIn.c_bstr(), wIn.Length(), psState, Idx,
                                                bCopyAfgAbg, bTrackSpaceColorChanges);
}

int __fastcall TUtils::GetState(wchar_t* pBuf, int iSize, PUSHSTRUCT &psState,
                  int Idx, bool bCopyAfgAbg, bool bTrackSpaceColorChanges)
// pBuf is the buffer of raw-codes
// State is returned with the state formated into a PUSHSTRUCT
// Idx is the 0-based "Real Character index" up to which we record the state
// If bTrackSpaceColorChanges is true we continue recording to the first non-space
// wchar_t past Idx. If bCopyAfgAbg is set we use Afg/Abg to replace NO_COLOR.
//
// CR/LFs are ignored and are NOT counted as one char!
//
// returns the 0-based index into pBuf we scan up to, -1 on failure.
{
  try
  {
    psState.Clear();

    // Set the cumulative text state at real index Idx
    int RetIdx = SetStateFlags(pBuf, iSize, Idx, psState, bTrackSpaceColorChanges);

    // Print a string of initial state
    // (default for bCopyAfgAbg is "true")
    if (bCopyAfgAbg)
    {
      if (psState.fg == NO_COLOR)
        psState.fg = dts->Afg;

      if (psState.bg == NO_COLOR)
        psState.bg = dts->Abg;
    }

    return RetIdx;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: GetState()\r\n");
#endif
    return -1;
  }
}
//---------------------------------------------------------------------------
// gets the state at the first character in a string
PUSHSTRUCT __fastcall TUtils::GetLeadingState(WideString wIn, int &iEnd)
{
  PUSHSTRUCT ps;
  iEnd = SetStateFlags(wIn, STATE_MODE_FIRSTCHAR, ps);
  return ps;
}
//---------------------------------------------------------------------------
// Overloaded

// Used by TStringsW.
// This is useful for getting the state at the last char of a string to pass
// on to another string. It trims codes off at the end of S and returns S by
// reference. Returns the state formatted to use as a leading state.
WideString __fastcall TUtils::GetTrailingState(WideString &wInOut)
{
  WideString wState;
  int idx = GetState(wInOut, wState, STATE_MODE_LASTCHAR, true, false);

  // Trim codes from original (return by-reference)
  if (idx >= 0)
    wInOut = wInOut.SubString(1, idx+1);

  return wState;
}

// gets the state of any trailing codes in a string - this does not scan
// the string to determine cumulative state of the entire string but just
// gets the codes at the end after the last printable char (used where we
// are adding bold, underline or italics in Effects.cpp and have a buffer
// of selected text that also includes leading and trailing codes...
//
// returns the state and returns the indices by-reference
PUSHSTRUCT __fastcall TUtils::GetTrailingState(WideString wIn,
                                                    int &iStart, int &iEnd)
{
  PUSHSTRUCT ps;

  // get the index of the codes at end of buffer
  iStart = GetCodeIndex(wIn, STATE_MODE_ENDOFBUF);
  // set state based on those codes
  iEnd = SetStateFlags(wIn, iStart, STATE_MODE_ENDOFBUF, ps);
  return ps;
}
//---------------------------------------------------------------------------
// Overloaded

// ...starts at 0 in buffer (pass WideString in)
int __fastcall TUtils::SetStateFlags(WideString S, int iCharStopAt,
                                      PUSHSTRUCT &ps, bool bTrackSpaceColorChanges)
{
  return SetStateFlags(S.c_bstr(), S.Length(), 0, iCharStopAt,
                                                      ps, bTrackSpaceColorChanges);
}

// ...starts at 0 in buffer (pass buffer address and size in)
int __fastcall TUtils::SetStateFlags(wchar_t* pBuf, int iSize,
                    int iCharStopAt, PUSHSTRUCT &ps, bool bTrackSpaceColorChanges)
{
  return SetStateFlags(pBuf, iSize, 0, iCharStopAt, ps, bTrackSpaceColorChanges);
}

// ...can set buffer start index (pass WideString in)
int __fastcall TUtils::SetStateFlags(WideString S, int iBufStart,
                    int iCharStopAt, PUSHSTRUCT &ps, bool bTrackSpaceColorChanges)
{
  return SetStateFlags(S.c_bstr(), S.Length(), iBufStart,
                                      iCharStopAt, ps, bTrackSpaceColorChanges);
}

// ...can set buffer start index (pass buffer address and size in)
int __fastcall TUtils::SetStateFlags(wchar_t* pBuf, int iSize,
      int iBufStart, int iCharStopAt, PUSHSTRUCT &ps, bool bTrackSpaceColorChanges)
// This function scans a buffer and determines the state of colors and
// text-effects that exists at the last printable wchar_t in the buffer
// if iCharStopAt is STATE_MODE_LASTCHAR or at the specified character at
// iCharStopAt (0-based).
//
// NOTE: this is the 0-based index of a printable character's  position in a
// string that contains format-codes, it does not include the color-codes, etc.
//
// We can have spaces that change background-color, so the background color's
// state should freeze at the specified iCharStopAt.
//
// iBufStart is the 0-based start-index into the WideString "S" or buffer
// "pBuf".
//
// If iCharStopAt is STATE_MODE_ENDOFBUF we do a hard scan from iBufStart to
// iSize-1 not backing up for the most recent cr/lf and return the state at
// the end.
//
// From ProcessIRC.h...
//
// PUSHSTRUCT Members:
//  int fg;
//  int bg;
//  int fontsize;
//  int fonttype;
//  int index; // place to keep an index...
//  int crlfs; // count of CR/LFs
//  bool bBold;
//  bool bUnderline;
//  bool bItalics; // italics for Yahoo-chat
//  bool bFGfast;
//  bool bBGfast;
//  bool bFontTypefast;
//  bool bFontSizefast;
//
// returns the 0-based index into pBuf we scan up to.
//
// returns the index into pBuf of the last printable character and the state
// at the last printable wchar_t if iStopAt is STATE_MODE_LASTCHAR. If iStopAt
// is STATE_MODE_ENDOFBUF we return the state at the end of the buffer.
//
// returns -1 if an error (the state of ps is unusable). Returns 0 for
// an empty string (no-state)
//
// CR/LFs are ignored and are NOT counted as one char!
//
// Setting bDoNotTrackSpaceColorChanges will cause background color changes of leading
// space characters to not to be tracked.
//
// This is an important function... and I'm changing my "way of thinking"
// about YahCoLoRiZe documents... when you send a line of text over
// social media it must be "self contained". So, this function needs to
// reset the state when hitting any CR/LFs. And we should not need to scan
// an entire buffer. We should ignore any trailing codes that exist alone
// beyond the last CR/LF. So I need a function to reset iSize to avoid that.
// The function needs to scroll past the last cr/lf, then if 0 real chars,
// return a new iSize...
{
  ps.Clear();

  // NOTE: pBuf CAN be NULL for a WideString c_bstr()! We want to return 0
  // rather than an error...

  // Must have a valid buffer with at least one wchar_t to scan...
  if (iSize < 0 || iBufStart < 0 || iBufStart > iSize)
    return -1;

  // Handle empty string - this is not an error - its valid to pass back
  // a cleared ps or "no state"
  if (!pBuf || iSize == 0)
    return 0;

  int CharCounter = 0;

  // Compute a new effective iSize... without trailing cr/lfs and
  // format codes... (just temporary, not a reference!)
  iSize = StripLastCrLfAndCodes(pBuf, iSize);

  if (iSize < 0)
    return -1; // Error

  // Could have a string of cr/lfs and codes only!
  if (iSize == 0)
    return 0;

  try
  {
    int ii = iBufStart;
    int iSaveIdx;

    if (iCharStopAt == STATE_MODE_ENDOFBUF)
      iSaveIdx = iSize-1; // Scan to end of buffer
    else
    {
      iSaveIdx = -1;

      // Scan up from iBufStart to iSize and find the wchar_t we want
      for(; ii < iSize; ii++)
      {
        int iRet = this->SkipCode(pBuf, iSize, ii);
        if (iRet == S_NULL) break;
        if (iRet) continue;

        CharCounter++; // Count printable chars and spaces

        if (!bTrackSpaceColorChanges && pBuf[ii] == C_SPACE)
          continue;

        iSaveIdx = ii; // Save buffer-index of real wchar_t for later...

        // iCharStopAt of -1 (STATE_MODE_LASTCHAR) scans up to end of buffer
        if (iCharStopAt >= 0 && CharCounter > iCharStopAt)
          break;
      }

      // Buffer (up to iCharStopAt) had no real chars?
      // (iSaveIdx will be -1 if no real chars found!)
      // (if iCharStopAt is passed in as a -1, iSaveIdx will point
      // to the last real wchar_t in the buffer!)
      if (iSaveIdx < 0)
        iSaveIdx = iSize-1; // Scan to end of buffer

      // Scan back to find line-break, if any (we want to begin
      // acquiring the state AFTER a line-break)
      for (ii = iSaveIdx; ii >= iBufStart; ii--)
        if (pBuf[ii] == C_CR || pBuf[ii] == C_LF)
          break;

      if (ii < iBufStart)
        ii = iBufStart; // no CR/LF found...
      else
        ii++; // Found cr or lf... go to next wchar_t or code
    }

    PUSHSTRUCT tps;
    tps.Clear();

    bool bNeedToInitState = false;

    // Scroll back up, setting state this time,
    // and continuing beyond space-chars...
    for(; (ii <= iSaveIdx || pBuf[ii] == C_SPACE) && ii < iSize; ii++)
    {
      // We want to reset the state on first wchar_t AFTER every cr/lf sequence
      if (pBuf[ii] != C_CR && pBuf[ii] != C_LF)
      {
        if (bNeedToInitState)
        {
          // !!!!!!!! 11/27/2018 don't want to clear cr/lf counter, index, pushcounter...
          //tps.Clear();
          tps.ClearColors();
          tps.ClearFont();
          tps.ClearStyle();
          bNeedToInitState = false;
        }
      }
      else // CR or LF
      {
        if (pBuf[ii] == C_CR)
          tps.crlfs++; // Keep count of cr/lfs

        bNeedToInitState = true;
        continue;
      }

      if (pBuf[ii] == CTRL_K)
      {
        int fg = NO_COLOR;
        int bg = NO_COLOR;

        ii += this->CountColorSequence(pBuf, ii, iSize, fg, bg);

        // freeze this at iSaveIdx
        if (fg != NO_COLOR && ii <= iSaveIdx)
          tps.fg = fg;

        if (bg != NO_COLOR)
          tps.bg = bg;

        continue;
      }

      if (pBuf[ii] == CTRL_F)
      {
        int fontType = CountFontSequence(pBuf, ii, iSize);

        if (fontType >= 0)
        {
          tps.fontType = fontType;
          ii += 2;
        }

        continue;
      }

      if (pBuf[ii] == CTRL_S)
      {
        int fontSize = CountFontSequence(pBuf, ii, iSize);

        if (fontSize >= 0)
        {
          tps.fontSize = fontSize;
          ii += 2;
        }

        continue;
      }

      if (pBuf[ii] == CTRL_B)
        tps.bBold = !tps.bBold;
      else if (pBuf[ii] == CTRL_U)
        tps.bUnderline = !tps.bUnderline;
      else if (pBuf[ii] == CTRL_R)
        tps.bItalics = !tps.bItalics;
      else if (pBuf[ii] == CTRL_PUSH)
      {
        tps.pushCounter++;
        tps.bPush = true;
      }
      else if (pBuf[ii] == CTRL_POP)
        tps.bPop = true;
    }

    // this is messing up the case where we are trying to get the state of
    // leading codes-only in StringsW.cpp when adding a new-line in RTF mode and
    // trying to carry the state into the next line!
    //if (IsAllSpaces(pBuf, iSize))
    //  tps.fg = NO_COLOR;

    ps = tps;

    return iSaveIdx;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: SetStateFlags()\r\n");
#endif
    return -1;
  }
}
//----------------------------------------------------------------------------
// this must be wide-string because of unicode font-names!
// Set bLeading false if we will place this string after a selection
// instead of before it
WideString __fastcall TUtils::PrintStateString(PUSHSTRUCT ps, bool bLeading)
{
  WideString wState;

  if (!bLeading)
  {
    if (ps.bBold)
      wState += CTRL_B;

    if (ps.bUnderline)
      wState += CTRL_U;

    if (ps.bItalics)
      wState += CTRL_R;
  }
  else
  {
    if (ps.bPop)
      wState += CTRL_POP;
    if (ps.bPush)
      wState += CTRL_PUSH;
  }

  (void)WriteColors(ps.fg, ps.bg, wState);

  if (ps.fontType >= 0)
    wState += FontTypeToString(ps.fontType);

  if (ps.fontSize >= 0)
    wState += FontSizeToString(ps.fontSize);

  if (bLeading)
  {
    if (ps.bItalics)
      wState += CTRL_R;

    if (ps.bUnderline)
      wState += CTRL_U;

    if (ps.bBold)
      wState += CTRL_B;
  }
  else
  {
    if (ps.bPush)
      wState += CTRL_PUSH;
    if (ps.bPop)
      wState += CTRL_POP;
  }

  return wState;
}
//---------------------------------------------------------------------------
void __fastcall TUtils::ShowState(PUSHSTRUCT ps, int iRow, int iColumn)
{
  WideString wPrintStr;

  if (iRow >= 0 && iColumn >= 0)
    wPrintStr += "Line: " + String(iRow) + "," + String(iColumn);

  wPrintStr += "\n\nBold: ";
  ps.bBold ? wPrintStr += "on" : wPrintStr += "off";
  wPrintStr += "\nUnderline: ";
  ps.bUnderline ? wPrintStr += "on" : wPrintStr += "off";
  wPrintStr += "\nItalics (reverse video): ";
  ps.bItalics ? wPrintStr += "on" : wPrintStr += "off";
  wPrintStr += "\nProtected (push/pop): ";
  ps.pushCounter > 0 ? wPrintStr += "yes (level " + String(ps.pushCounter) + ")" : wPrintStr += "no";

  if (ps.fg == NO_COLOR)
    wPrintStr += "\n(no foreground color)";
  else
  {
    BlendColor fg = utils->YcToBlendColor(ps.fg);
    wPrintStr += "\n\nForeground RGB: " + String(fg.red) + "," +
                    String(fg.green) + "," + String(fg.blue);
    if (ps.fg > 0) // palette color?
      wPrintStr += "\n(palette color index: " + String(ps.fg-1) + ")";
  }

  if (ps.bg == NO_COLOR)
    wPrintStr += "\n(no background color)";
  else
  {
    BlendColor bg = YcToBlendColor(ps.bg);
    wPrintStr += "\n\nBackground RGB: " + String(bg.red) + "," +
                    String(bg.green) + "," + String(bg.blue);
    if (ps.bg > 0) // palette color?
      wPrintStr += "\n(palette color index: " + String(ps.bg-1) + ")";
  }

  WideString wFontString = GetLocalFontStringW(ps.fontType);

  if (ps.fontSize <= 0 || wFontString.IsEmpty())
    wPrintStr += "\n(no font)";
  else
    wPrintStr += "\n\nFont: \"" + wFontString +
                "\"," + String(ps.fontSize);

  ShowMessageW(wPrintStr);
}
//---------------------------------------------------------------------------
int __fastcall TUtils::GetNumberOfLinesSelected(bool &bPartialSelection)
{
  return(GetNumberOfLinesSelected(bPartialSelection, tae));
}
//---------------------------------------------------------------------------
int __fastcall TUtils::GetNumberOfLinesSelected(bool &bPartialSelection, TTaeRichEdit* re)
// Returns 0 if empty selection.
//
// bPartialSelection is returned set if the selection does
// not end in a CR/LF...
//
// (Note: The VT wchar_t seems to be used in rich text whereas the CR
// by itself is in plain text within the edit-control. In the actual
// text buffer it is a CR/LF but we have to get the indices in the
// buffer that corresponds with the user's highlighted selection in the
// rich edit control).
{
  bPartialSelection = false;

  int SelLen = re->SelLength;

  // Return if nothing is selected
  if (!SelLen) return 0;

  WideString S = re->SelTextW;

  // At least one-line...
  int NumberOfLines = 1;

  int len = S.Length();

  for (int ii = 1; ii <= len; ii++)
    if (S[ii] == C_VT || S[ii] == C_CR)
      NumberOfLines++;

  try
  {
    if (len == 0)
      return 0;

    wchar_t c = S[S.Length()];

    // Selection beyond end of text?
    if (c == C_NULL)
      if (--SelLen == 0)
        return 0;

    if (c != C_CR && c != C_VT)
    {
      bPartialSelection = true;
      NumberOfLines++;
    }
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: GetNumberOfLinesSelected()\r\n");
#endif
    return 0;
  }

  return NumberOfLines;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::GetTaeEditColors(int &fg, int &bg, bool bAtCaret)
// Gets leading fg/bg colors for TaeEdit if the MS_IRC or MS_ORG
// streams exist and we are viewing one of them.
//
// Returns true if no error
{
  if (!IsRtfIrcOrgView())
    return false;

  TPoint p;

  if (bAtCaret)
    p = tae->CaretPos;
  else
  {
    p.x = 0;
    p.y = 0;
  }

  TStringsW* sl = (tae->View == V_ORG) ? dts->SL_ORG : dts->SL_IRC;

  WideString wLine = sl->GetString(p.y);

  PUSHSTRUCT ps;
  SetStateFlags(wLine, p.x, ps);

  if (ps.fg == NO_COLOR)
    ps.fg = ConvertColor(dts->Foreground, dts->bRgbRandFG);

  if (ps.bg == NO_COLOR)
    ps.bg = ConvertColor(dts->Background, dts->bRgbRandBG);

  fg = ps.fg;
  bg = ps.bg;

  return true;
}
//---------------------------------------------------------------------
// Overloaded
WideString __fastcall TUtils::DeleteRealRange(WideString S, int iRealStart,
                                                          int iRealLength)
// iRealStart is a 0-based index (usually SelStart-Line)
{
  return DeleteRealRange(S.c_bstr(), S.Length(), iRealStart, iRealLength);
}

WideString __fastcall TUtils::DeleteRealRange(wchar_t* pBuf, int iSize,
                                        int iRealStart, int iRealLength)
// Returns a string of remaining chars after deleting
// a range of chars and leaving the color-codes.
// (also skips CR/LF chars in the range). You should eventually
// call the Optimizer to clean up residual format-codes!
//
// iRealStart is a 0-based index
{
  if (iRealStart < 0 || iRealLength <= 0 || iSize <= 0 || pBuf == NULL)
    return "";

  int RealCounter = 0;

  int StartAt = iRealStart+1;
  int StopAt = iRealStart+iRealLength;
  int Saveii;

  WideString NewString;

  for (int ii = 0; ii < iSize;)
  {
    Saveii = ii;

    int iRet = this->SkipCode(pBuf, iSize, ii);

    if (iRet == S_NULL)
      break;

    if (iRet)
      ii++; // Non-printable codes
    else
      RealCounter++; // Printable code

    if (ii != Saveii) // non-printable char(s)?
    {
      // Add them
      for (int jj = Saveii; jj < ii; jj++)
        NewString += WideString(pBuf[jj]);
    }
    else
    {
      if (RealCounter < StartAt || RealCounter > StopAt) // printable
        NewString += WideString(pBuf[ii]);

      ii++;
    }
  }

  return NewString;
}
//---------------------------------------------------------------------
// Overloaded
bool __fastcall TUtils::TextContainsFormatCodes(TStringsW* sl)
{
  return TextContainsFormatCodes(sl->Text);
}

bool __fastcall TUtils::TextContainsFormatCodes(WideString S)
{
  return TextContainsFormatCodes(S.c_bstr(), S.Length());
}

bool __fastcall TUtils::TextContainsFormatCodes(wchar_t* pBuf, int iSize)
// Returns true if any colors, font-codes, push/pop, ctrl-o or any
// codes except a cr/lf...
{
  for (int ii = 0; ii < iSize; ii++)
  {
    int iRet = this->SkipCode(pBuf, iSize, ii);
    if (iRet == S_NULL) break;
    if (iRet == S_CRLF) continue;
    if (iRet) return true;
  }

  return false;
}
//---------------------------------------------------------------------
// Overloaded
int __fastcall TUtils::SkipCodes(WideString S)
{
  wchar_t c;
  return(SkipCodes(S.c_bstr(), S.Length(), 0, c));
}

int __fastcall TUtils::SkipCodes(WideString S, int iBufStart, wchar_t &c)
// iBufStart is a 0-based index
{
  return(SkipCodes(S.c_bstr(), S.Length(), iBufStart, c));
}

int __fastcall TUtils::SkipCodes(wchar_t* pBuf, int iSize,
                                                    int iBufStart, wchar_t &c)
// iBufStart is a 0-based index into the buffer.
//
// Routine is intended to facillitate clean-up of superfluous leading or
// trailing color/font codes.
//
// We return the # of extra chars to remove.
//
// If we hit any printable char, including a space, before the end of
// the buffer and before any cr/lf we return the wchar_t in c. Hitting cr/lf
// leaves c set to C_NULL.
//
// We quit on the first cr/lf and it is NOT included in the returned count.
{
  c = C_NULL;

  if (iBufStart < 0 || iSize <= 0 || iBufStart >= iSize || pBuf == NULL)
    return 0;

  int ii;

  for (ii = iBufStart; ii < iSize; ii++)
  {
    int iRet = this->SkipCode(pBuf, iSize, ii);

    if (iRet == S_NULL)
      break;

    // (NOTE: here we still DO want the C_FF included in
    // the returned count!)
    if (iRet == S_CRLF)
    {
      ii--; // we don't include the cr/lf in our returned count!
      break;
    }

    if (iRet != S_NOCODE)
      continue;

    c = pBuf[ii]; // return the char
    break;
  }

  if (ii > iSize)
  {
    ShowMessage("Error: ii > iSize in SkipColorsAndFirstCRLF()");
    return 0;
  }

  return ii-iBufStart;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::SkipCode(WideString wIn, int &ii)
{
  return SkipCode(wIn.c_bstr(), wIn.Length(), ii);
}

int __fastcall TUtils::SkipCode(wchar_t* pBuf, int iSize, int &ii)
// Skips the first special code-sequence at ii
//
// Returns S_NOCODE if no special codes at ii
// Returns S_CRLF if CR/LF
// Returns S_COLOR if Color
// Returns S_FONTSIZE if FontSize
// Returns S_FONTTYPE if FontType
// Returns S_NULL if the null wchar_t '\0'
// Returns S_TAB if the tab char '\011' (octal)
// Returns S_FF if the foorm-feed char '\014' (octal)
// Returns S_OTHER if any other control-char
//
// ii is a reference and will return modified!
{
  if (pBuf[ii] == CTRL_K)
  {
    ii += this->CountColorSequence(pBuf, ii, iSize);
    return S_COLOR;
  }

  if (pBuf[ii] == CTRL_F)
  {
    if (this->CountFontSequence(pBuf, ii, iSize) >= 0)
      ii += 2;

    return S_FONTTYPE;
  }

  if (pBuf[ii] == CTRL_S)
  {
    if (this->CountFontSequence(pBuf, ii, iSize) >= 0)
      ii += 2;

    return S_FONTSIZE;
  }

  // will increment ii for cr/lf or lf/cr pair,
  // not if single cr or lf.
  if (FoundCRLF(pBuf, iSize, ii))
    return S_CRLF;

  if (pBuf[ii] == C_NULL)
    return S_NULL;

  if (pBuf[ii] == C_TAB)
    return S_TAB;

  if (pBuf[ii] == C_FF)
    return S_FF;

  // Skip any other control-chars
  if (AnySpecialChar(pBuf[ii]))
    return S_OTHER;

  return S_NOCODE;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::SkipAllCodes(WideString S)
// Returns the # of leading special code chars
{
  return SkipAllCodes(S.c_bstr(), 0, S.Length());
}
//---------------------------------------------------------------------------
int __fastcall TUtils::SkipAllCodes(wchar_t* pBuf, int ii, int iSize)
// Returns the # of leading special code chars
{
  int idx;

  for(idx = ii; idx < iSize; idx++)
    if (this->SkipCode(pBuf, iSize, idx) == S_NOCODE)
      break;

  return idx-ii;
}
//---------------------------------------------------------------------
void __fastcall TUtils::StripTrailingSpaces(wchar_t* &pBuf, int &iSize)
// strip trailing spaces
{
  TStringsW* sl = NULL;

  try
  {
    try
    {
      TStringsW* sl = new TStringsW();
      sl->Text = WideString(pBuf, iSize);
      delete [] pBuf;
      StripTrailingSpaces(sl);
      pBuf = sl->GetTextBuf(iSize);
    }
    catch(...) {}
  }
  __finally
  {
    if (sl)
      delete sl;
  }
}
//---------------------------------------------------------------------
void __fastcall TUtils::StripTrailingSpaces(TStringsW* sl)
// strip trailing spaces
{
  for(int ii = 0; ii < sl->Count; ii++)
    sl->SetString(StripTrailingSpaces(sl->GetString(ii)), ii);
}
//---------------------------------------------------------------------
WideString __fastcall TUtils::StripTrailingSpaces(WideString S)
// Delete trailing spaces and codes
{
  if (S.IsEmpty())
    return S;

  for(;;)
  {
    S = StripTrailingCodes(S); // strip trailing colors, font-size, etc

    if (S.IsEmpty())
      return S;

    if (S[S.Length()] != ' ')
      break; // no trailing spaces quit

    while(!S.IsEmpty() && S[S.Length()] == ' ')
      S = DeleteW(S, S.Length(), 1);
  }

  return S;
}
//---------------------------------------------------------------------
void __fastcall TUtils::PadTrailingSpaces(wchar_t* &pBuf, int &iSize,
                                                                int iWidth)
// pad trailing spaces
{
  TStringsW* sl = NULL;

  try
  {
    try
    {
      TStringsW* sl = new TStringsW();
      sl->Text = WideString(pBuf, iSize);
      delete [] pBuf;
      PadTrailingSpaces(sl, iWidth);
      pBuf = sl->GetTextBuf(iSize);
    }
    catch(...) {}
  }
  __finally
  {
    if (sl)
      delete sl;
  }
}
//---------------------------------------------------------------------
void __fastcall TUtils::PadTrailingSpaces(TStringsW* sl, int iWidth)
// pad trailing spaces
{
  for(int ii = 0; ii < sl->Count; ii++)
    sl->SetString(PadTrailingSpaces(sl->GetString(ii), iWidth), ii);
}
//---------------------------------------------------------------------
WideString __fastcall TUtils::PadTrailingSpaces(WideString wIn, int iWidth)
// Pad S with spaces out to dts->RegWidth real-length
// For an entire document - call this once for each line in a string-list!
{
  int realLen = GetRealLength(wIn);

  int padLen = iWidth - realLen;

  if (padLen <= 0)
    return wIn;

  return wIn + AnsiString::StringOfChar(C_SPACE, padLen);
}
//---------------------------------------------------------------------
bool __fastcall TUtils::StripLeadingAndTrailingSpaces(wchar_t* &pBuf, int &iSize)
// Delete leading and trailing spaces from all lines of a document
// that contains IRC formatting codes.
//
// iSize is 1 less than the actual buffer's size on entry
{
  if (pBuf == NULL)
    return false;

  try
  {
    TStringsW* sl = new TStringsW();

    if (pBuf[iSize] != C_NULL)
    {
      ShowMessage("Last wchar_t != C_NULL in TUtils::StripLeading...");
      pBuf[iSize] = C_NULL; // Make sure last wchar_t is null
    }

    sl->SetTextBuf(pBuf);

    try
    {
      delete [] pBuf;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: StripLeadingAndTrailingSpaces()\r\n");
#endif
    }

    for (int ii = 0; ii < sl->Count; ii++)
    {
      WideString S = sl->GetString(ii);
      S = StripLTspaces(S);
      sl->SetString(S, ii);
    }

    iSize = sl->TotalLength;

//    if (bNoTrailLF)
//      iSize -= 2; // We don't want a trailing cr/lf added!

    pBuf = new wchar_t[iSize+1];
    wcsncpy(pBuf, sl->Text.c_bstr(), iSize);
    delete sl;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: StripLeadingAndTrailingSpaces()\r\n");
#endif
    return false;
  }

  return true;
}
//---------------------------------------------------------------------
WideString __fastcall TUtils::StripLTspaces(WideString S)
// Delete leading and trailing spaces from string skipping over
// format-codes and preserving the state at the 1st character.
{
  wchar_t* pBuf = NULL;;
  WideString New;

  try
  {
    int iSize = S.Length();
    pBuf = new wchar_t[iSize+1];
    // wcscpy won't copy null if source string is empty!
    pBuf[iSize] = C_NULL;
    wcsncpy(pBuf, S.c_bstr(), iSize);

    if (StripLTspaces(pBuf, iSize))
      New = WideString(pBuf, iSize);
#if DEBUG_ON
    else
      dts->CWrite("\r\nStripLTspaces(pBuf, iSize) returned false!\r\n");
#endif

  }
  __finally
  {
    try
    {
      if (pBuf != NULL)
        delete [] pBuf;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: StripLTspaces()\r\n");
#endif
    }
  }

  return New;
}
//---------------------------------------------------------------------
bool __fastcall TUtils::StripLTspaces(wchar_t* &pBuf, int &iSize)
// Delete leading and trailing spaces from string skipping over
// format-codes and preserving the state at the 1st character
{
  try
  {
    int iRet;
    int ii = 0;

    for (; ii < iSize; ii++)
    {
      iRet = this->SkipCode(pBuf, iSize, ii);

      if (iRet == S_NULL) break;
      if (iRet) continue;
      if (pBuf[ii] != C_SPACE) break;
    }

    if (ii == iSize) // Was all spaces or empty buffer?
    {
      iSize = 0;
      pBuf[0] = C_NULL; // Make it empty
      return true;
    }

    int startIdx = ii; // first printable char's non-space 0-based index
    int endIdx = ii; // last printable non-space char's 0-based index

    // go through rest of buffer and come out with the index of the
    // last non-space char
    for (; ii < iSize; ii++)
    {
      iRet = this->SkipCode(pBuf, iSize, ii);

      if (iRet == S_NULL) break;
      if (iRet) continue;

      if (pBuf[ii] != C_SPACE)
        endIdx = ii;
    }

    endIdx++; // point 1 past last nonspace char...

    // Set the cumulative text state at the last non-space char.
    // Don't look past endIdx!
    PUSHSTRUCT ps;
    SetStateFlags(pBuf, endIdx, STATE_MODE_LASTCHAR, ps);

    // Terminate bold, underline or reverse
    WideString EndState;
    TerminateEffects(ps, EndState);

    // Get state at 1st non-space char
    WideString LeadState;
    if (GetState(pBuf, endIdx, LeadState, 0, true, false) < 0)
    {
#if DEBUG_ON
      dts->CWrite("\r\nError in StripLTspaces():GetState()\r\n");
#endif
      return false;
    }

    // Make a new buffer
    int iNewSize = (endIdx-startIdx) + LeadState.Length() + EndState.Length();

    if (iNewSize <= 0)
    {
#if DEBUG_ON
      dts->CWrite("\r\nError iNewSize <= 0 StripLTspaces()\r\n");
#endif
      return false;
    }

    wchar_t* pNew = new wchar_t[iNewSize+1];

    // wcscpy won't copy null if source string is empty!
    pNew[iNewSize] = C_NULL;

    if (!LeadState.IsEmpty())
      wcscpy(pNew, LeadState.c_bstr()); // Copy LeadState to new buf

    // copy old text without leading/trailing spaces
    int jj = LeadState.Length();
    for (ii = startIdx; ii < endIdx; ii++, jj++)
      pNew[jj] = pBuf[ii];

    if (!EndState.IsEmpty())
      wcscpy(&pNew[jj], EndState.c_bstr()); // Copy EndState to new buf

    pNew[iNewSize] = C_NULL;

    try
    {
      delete [] pBuf; // finished with original buffer
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: StripLTspaces()\r\n");
#endif
      return false;
    }

    pBuf = pNew; // Switch to the new buffer
    iSize = iNewSize;
    return true;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: StripLTspaces()\r\n");
#endif
    return false;
  }
}
//---------------------------------------------------------------------
bool __fastcall TUtils::StripParagraphCRLF(wchar_t* &pBuf, int &iSize)
// Delete cr/lf codes in each paragraph and substiture a space
// Each paragraph becomes one line - easy then to add a line
// between those lines to make new paragraphs...
{
  wchar_t* pNewBuf = new wchar_t[iSize+1];

  try
  {
    int iNew = 0;

    for (int ii = 0; ii < iSize; ii++)
    {
      int Saveii = ii;

      int iRet = this->SkipCode(pBuf, iSize, ii);

      if (iRet == S_NULL) break;

      if (iRet == S_CRLF)
      {
        ii++;

        if (!FoundCRLF(pBuf, iSize, ii))
        {
          pNewBuf[iNew++] = C_SPACE; // Join lines
          pNewBuf[iNew++] = pBuf[ii];
        }
        else // Paragraph...
        {
          pNewBuf[iNew++] = C_CR;
          pNewBuf[iNew++] = C_LF;

          while (pBuf[ii] == C_CR || pBuf[ii] == C_LF)
            ii++; // Skip over all lines

          if (ii < iSize && pBuf[ii] != C_NULL)
            pNewBuf[iNew++] = pBuf[ii];
        }

        continue;
      }

      for (int jj = Saveii; jj <= ii; jj++)
        pNewBuf[iNew++] = pBuf[jj];
    }

    pNewBuf[iNew] = C_NULL;

    try
    {
      delete [] pBuf;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: StripParagraphCRLF()\r\n");
#endif
    }

    pBuf = pNewBuf;
    iSize = iNew; // Change the reference's value
  }
  catch(...)
  {
    try
    {
      delete [] pNewBuf;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pNewBuf in: StripParagraphCRLF()\r\n");
#endif
    }
#if DEBUG_ON
    dts->CWrite("\r\nException: StripParagraphCRLF()\r\n");
#endif
    return false;
  }

  return true;
}
//---------------------------------------------------------------------
// Overloaded... (TODO!)
/*
WideString __fastcall TUtils::GetTrailingCodes(WideString S)
// Return trailing format codes stripped of CR/LFs
{
  try
  {
    try
    {
      int idx = GetLastNoncodeBufferIdx(S);
      if (idx >= 0)
        S = S.SubString(idx+1, S.Length()-idx);
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nException: StripTrailingCodes()\r\n");
#endif
      S = "";
    }
  }
  __finally
  {
    S = StripCRLF(S);
  }

  return S;
}
*/
//---------------------------------------------------------------------
// Overloaded...
WideString __fastcall TUtils::StripTrailingCodes(WideString S)
// Delete trailing format codes and CR/LFs
{
  try
  {
    int idx = GetLastNoncodeBufferIdx(S);
    if (idx < 0) return ""; // Return empty string if no printable chars
    return S.SubString(1, S.Length() - (S.Length()-idx) + 1);
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: StripTrailingCodes()\r\n");
#endif
    return "";
  }
}

bool __fastcall TUtils::StripTrailingCodes(wchar_t* &pBuf, int &iSize)
// Delete trailing spaces, format codes and CR/LFs
// iSize is 1 less than the actual buffer's size on entry and return!
{
  if (!pBuf)
    return false;

  if (!iSize)
    return true;

  try
  {
    int idx = GetLastNoncodeBufferIdx(WideString(pBuf, iSize));

    if (idx < 0)
      iSize = 0;  // Return empty string if no printable chars
    else if (idx < iSize)
      iSize = idx+1;

    pBuf[iSize] = C_NULL;

    return true;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: StripTrailingCodes()\r\n");
#endif
    return false;
  }
}
//---------------------------------------------------------------------
// Overloaded
WideString __fastcall TUtils::ReplaceSpaces(WideString sIn)
// Replace spaces with invisible '_' char.
{
  TStringsW* sl = new TStringsW();

  if (sl == NULL)
    return "";

  sl->Text = sIn;

  if (sl->Count <= 0)
    return "";

  bool bRet = ReplaceSpaces(sl);
  WideString sOut;

  if (bRet)
    sOut = sl->Text;

  delete sl;
  return sOut;
}

bool __fastcall TUtils::ReplaceSpaces(TStringsW* sl)
// Replace spaces with invisible '_' char.
{
  if (sl == NULL || sl->Count <= 0) return false;

  bool bRet = false;

  try
  {
    int size = sl->TotalLength;
    wchar_t* pBuf = sl->GetTextBuf();
    bRet = ReplaceSpaces(pBuf, size);
    sl->Text = WideString(pBuf, size);

    try
    {
      delete [] pBuf;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: ReplaceSpaces(TStringsW* sl)\r\n");
#endif
    }
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: ReplaceSpaces(MS)\r\n");
#endif
  }

  return bRet;
}

bool __fastcall TUtils::ReplaceSpaces(wchar_t* &pBuf, int &iSize)
// Replace spaces with invisible '_' char.
// NOTE: mIRC uses the | wchar_t as an operator to pipe together commands
// and so we detect " |" and do some trick below...
{
  WideString S;

  try
  {
    int SpaceCounter = 0;

    int gfg = NO_COLOR;
    int gbg = NO_COLOR;

    bool bInvisible = false;

    int RealLength = GetRealLength(pBuf, iSize);
    int RealCharCounter = 0;

    // Fix for mIRC when you have " |"
    bool bFixmIRC = false;

    for (int ii = 0; ii < iSize; ii++)
    {
      if (FoundCRLF(pBuf,iSize,ii)) // may increment ii
      {
        gfg = NO_COLOR;
        gbg = NO_COLOR;
        SpaceCounter = 0;
        S += "\r\n";
        bInvisible = false;

        RealLength = GetRealLength(pBuf, iSize, ii+1, '\n', true);
        RealCharCounter = 0;
        bFixmIRC = false; // Fix for mIRC when you have " |"
        continue;
      }

      wchar_t c = pBuf[ii];

      if (c == CTRL_F)
      {
        int val = CountFontSequence(pBuf, ii, iSize);

        if (val >= 0)
        {
          ii += 2;

          // print it (if Yahoo or Trinity)
          S += FontTypeToString(val);
        }

        continue;
      }

      if (c == CTRL_S)
      {
        int val = CountFontSequence(pBuf, ii, iSize);

        if (val >= 0)
        {
          ii += 2;

          // print it (if Yahoo or Trinity)
          S += FontSizeToString(val);
        }

        continue;
      }

      // Fix for mIRC when you have " |"
      if (dts->IsCli(C_MIRC) && IsNextPrintableSequence(pBuf, ii, iSize, " |"))
        bFixmIRC = true;

      if (c == C_SPACE)
      {
        SpaceCounter++;

        // Fix for mIRC when you have " |"
        if (bFixmIRC)
        {
          WriteSingle(gbg, S, true); // set fg color to bg (invisible)
          bInvisible = true;
          bFixmIRC = false;
        }
        else if ((SpaceCounter == 2 || (SpaceCounter == 1 &&
                (RealCharCounter == RealLength-1 ||
                     RealCharCounter == 0))) && gbg != NO_COLOR)
        {
          WriteSingle(gbg, S, true); // set fg color to bg (invisible)
          bInvisible = true;
        }

        if (bInvisible)
          S += WideString(dts->NSChar); // substitue underscore for space
        else
          S += c; // write a space

        RealCharCounter++;
      }
      else // Not a space
      {
        if (bInvisible)
        {
          if (gfg != NO_COLOR)
            WriteSingle(gfg, S, true); // set fg normal
          else
            WriteSingle(dts->Afg, S, true); // set fg normal

          bInvisible = false;
        }

        if (c == CTRL_K)
        {
          int fg = NO_COLOR;
          int bg = NO_COLOR;

          int iTemp = this->CountColorSequence(pBuf, ii, iSize, fg, bg);

          // print it and skip over it
          S += WideString(CTRL_K);

          for (int jj = 0; jj < iTemp && ii < iSize-1; jj++, ii++)
            S += WideString(pBuf[ii+1]);

          if (fg != NO_COLOR)
            gfg = fg;

          if (bg != NO_COLOR)
          {
            gbg = bg;

            if (bInvisible) // background changed
            {
              S += WideString(CTRL_K);
              WriteSingle(gbg, S, true); // set fg color to bg (invisible)
            }
          }
          continue;
        }

        S += c;

        if (!AnySpecialChar(c))
        {
          SpaceCounter = 0;
          RealCharCounter++;
        }
      }
    } // End for

    try
    {
      delete [] pBuf;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: ReplaceSpaces()\r\n");
#endif
    }

    iSize = S.Length();
    pBuf = new wchar_t[iSize+1];
    // wcscpy won't copy null if source string is empty!
    pBuf[0] = C_NULL;

    if (!S.IsEmpty())
      wcscpy(pBuf, S.c_bstr());

    return true;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: ReplaceSpaces()\r\n");
#endif
    return false ;
  }
}
//---------------------------------------------------------------------
WideString __fastcall TUtils::FlattenPushPop(WideString sIn)
// Strips Push chars and replace Pop with CTRL_K
{
  int len = sIn.Length();
  if (len == 0) return "";

  WideString sOut;

  try
  {
    // move non-push/pops
    for (int ii = 1; ii <= len; ii++)
    {
      if (sIn[ii] == CTRL_POP) sOut += CTRL_K;
      else if (sIn[ii] != CTRL_PUSH) sOut += sIn[ii];
    }

    return sOut;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: StripPushPop()\r\n");
#endif
    return "";
  }
}
//---------------------------------------------------------------------
bool __fastcall TUtils::LongestWidthAndLineCount(wchar_t* pBuf,
                                        int iSize, int &Width, int &Height)
// returns true if any embedded colors or format-codes or font-size codes found
{
  Width = 0;
  Height = 0;
  bool bFoundCRLF;
  bool bFoundCode = false;

  int TempWidth = 0;

  if (iSize <= 0)
    return false;

  int ii = 0;

  for (; ii < iSize; ii++)
  {
    bFoundCRLF = FoundCRLF(pBuf, iSize, ii); // may increment ii

    if (bFoundCRLF)
    {
      if (TempWidth > Width)
        Width = TempWidth;

      TempWidth = 0;

      Height++;
      continue;
    }

    if (pBuf[ii] == C_NULL)
    {
      if (TempWidth > Width)
        Width = TempWidth;

      // Incriment line-count if any real chars in last line...
      if (TempWidth)
        Height++;

      break;
    }

    if (pBuf[ii] == CTRL_K)
    {
      int iTemp = CountColorSequence(pBuf, ii, iSize);

      if (iTemp)
      {
        ii += iTemp;

        if (!bFoundCode)
          bFoundCode = true;
      }

      continue;
    }

    if (pBuf[ii] == CTRL_F || pBuf[ii] == CTRL_S)
    {
      if (CountFontSequence(pBuf, ii, iSize) >= 0)
        ii += 2;

      if (!bFoundCode)
        bFoundCode = true;

      continue;
    }

    if (pBuf[ii] == CTRL_B || pBuf[ii] == CTRL_U ||
                                  pBuf[ii] == CTRL_R || pBuf[ii] == CTRL_O)
    {
      if (!bFoundCode)
        bFoundCode = true;

      continue;
    }

    if (AnySpecialChar(pBuf[ii]))
      continue;

    TempWidth++; // Printable char
  }

  // hit last char with no null or cr/lf?
  if (ii >= iSize)
  {
    if (TempWidth > Width)
      Width = TempWidth;

    // Incriment line-count if any real chars in last line...
    if (TempWidth)
      Height++;
  }

  return bFoundCode;
}
//---------------------------------------------------------------------
int __fastcall TUtils::GetNextNonspaceRealIdx(WideString S, int Idx)
// Return buffer index of 1st non-space char
//
// Returns -1 if error or nothing found
{
  return GetNextNonspaceRealIdx(S.c_bstr(), S.Length(), Idx);
}
//---------------------------------------------------------------------
int __fastcall TUtils::GetNextNonspaceRealIdx(wchar_t * pBuf, int iSize,
                                                             int Idx)
// Return real-wchar_t index of 1st non-space wchar_t past Idx
// Idx is a real-wchar_t 0-based starting index.
//
// Returns -1 if error or nothing found
{
  try
  {
    int RealCount = 0;

    for (int ii = 0; ii < iSize; ii++)
    {
      int iRet = this->SkipCode(pBuf, iSize, ii);

      if (iRet == S_NULL) break;
      if (iRet) continue;
      if (++RealCount > Idx && pBuf[ii] != C_SPACE) return RealCount-1;
    }

    return -1;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: GetNextNonspaceRealIdx()\r\n");
#endif
    return -1;
  }
}
//---------------------------------------------------------------------
int __fastcall TUtils::StripLastCrLfAndCodes(wchar_t * pBuf, int iSize)
// Returns new effective iSize... without trailing cr/lfs and
// format codes but does NOT return a reference change to iSize
// or affect pBuf's contents.
//
// Return -1 if error.
{
  if (!pBuf || iSize < 1) return -1;

  int FirstCrLfIdx = -1;

  try
  {
    for (int ii = 0; ii < iSize ; ii++)
    {
      int iRet = this->SkipCode(pBuf, iSize, ii);

      if (iRet == S_NULL) break;

      if (iRet == S_CRLF)
      {
        if (FirstCrLfIdx == -1)
          FirstCrLfIdx = ii; // Could be 1st of a string of cr/lf and codes!

        continue;
      }

      if (iRet) continue;

      FirstCrLfIdx = -1;
    }

    if (FirstCrLfIdx == -1) return iSize;

    // Back up
    while (FirstCrLfIdx >= 0 && (pBuf[FirstCrLfIdx] == C_CR ||
                       pBuf[FirstCrLfIdx] == C_LF))
      FirstCrLfIdx--; // can go to -1!

    return FirstCrLfIdx+1; // Return new effective iSize...
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: StripLastCrLfAndCodes()\r\n");
#endif
    return -1;
  }
}
//---------------------------------------------------------------------
int __fastcall TUtils::GetLastNoncodeBufferIdx(WideString S)
// Find 0-based index of last printable wchar_t (including spaces)
// in a string that has formatting codes in it.
//
// Returns -1 if error or nothing found
{
  return GetLastNoncodeBufferIdx(S.c_bstr(), S.Length());
}
//---------------------------------------------------------------------
int __fastcall TUtils::GetLastNoncodeBufferIdx(wchar_t * pBuf, int iSize)
// Find 0-based buffer index of last printable wchar_t (including spaces)
// in a string that has formatting codes in it.
//
// Returns -1 if error or nothing found
{
  int LastIndex = -1; // Nothing found

  try
  {
    for (int ii = 0; ii < iSize ; ii++)
    {
      int iRet = this->SkipCode(pBuf, iSize, ii);

      if (iRet == S_NULL) break;
      if (iRet) continue;

      LastIndex = ii;
    }
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: GetLastNoncodeBufferIdx()\r\n");
#endif
    return -1;
  }

  return LastIndex;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::GetSelectedZoneIndices(TTaeRichEdit* re, int &First, int &Last)
// Get 0-based indices into the re->TextW string based on the selected
// zone. Each newline is one character (a CR).
{
  try
  {
    if (re->SelLength)
    {
      // Map the start and end indices of the IRC or ORG selected text
      // to the text in the buffer.
      int OrigSelCRs = CountCRs(re->SelTextW);
      First = re->SelStart + GetLine(re);
      Last = First + re->SelLength + OrigSelCRs;
    }
    else
    {
      First = 0;
      Last = re->TextLength;
    }

    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
// Overloaded
bool __fastcall TUtils::GetCodeIndices(WideString S, int &FirstIdx,
                  int &LastIdx, int &CodeIdx, int RealStart, int RealCount)
{
  return GetCodeIndices(S.c_bstr(), S.Length(), FirstIdx, LastIdx, CodeIdx,
                                                        RealStart, RealCount);
}

bool __fastcall TUtils::GetCodeIndices(int &FirstIdx, int &LastIdx,
                                              int &CodeIdx, TTaeRichEdit* re)
{
  return GetCodeIndices(re->TextW, FirstIdx, LastIdx, CodeIdx, re);
}

bool __fastcall TUtils::GetCodeIndices(TStringsW* sl, int &FirstIdx,
                                int &LastIdx, int &CodeIdx, TTaeRichEdit* re)
{
  int iSize = sl->TotalLength;
  wchar_t* pBuf = sl->GetTextBuf();

  int RealStart, RealCount;

  // first, do we have text selected?
  if (re->SelLength)
  {
    RealStart = re->SelStart;
    RealCount = re->SelLength;
  }
  else
  {
    RealStart = 0;
    RealCount = utils->GetTextLength(re);
  }

  bool bRet = GetCodeIndices(pBuf, iSize, FirstIdx, LastIdx, CodeIdx,
                                                      RealStart, RealCount);

  try
  {
    delete [] pBuf;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nUnable to deallocate pBuf in: GetCodeIndices() A\r\n");
#endif
  }

  return bRet;
}

bool __fastcall TUtils::GetCodeIndices(WideString S, int &FirstIdx,
                              int &LastIdx, int &CodeIdx, TTaeRichEdit* re)
{
  int RealStart, RealCount;

  // first, do we have text selected?
  if (re->SelLength)
  {
    RealStart = re->SelStart;
    RealCount = re->SelLength;
  }
  else
  {
    RealStart = 0;
    RealCount = utils->GetTextLength(re);
  }

  return GetCodeIndices(S.c_bstr(), S.Length(), FirstIdx, LastIdx,
                                            CodeIdx, RealStart, RealCount);
}

bool __fastcall TUtils::GetCodeIndices(wchar_t* pBuf, int iSize, int &FirstIdx,
                             int &LastIdx, int &CodeIdx, TTaeRichEdit* re)
{
  int RealStart, RealCount;

  // first, do we have text selected?
  if (re->SelLength)
  {
    RealStart = re->SelStart;
    RealCount = re->SelLength;
  }
  else
  {
    RealStart = 0;
    RealCount = utils->GetTextLength(re);
  }

  return GetCodeIndices(pBuf, iSize, FirstIdx, LastIdx, CodeIdx, RealStart,
                                                                  RealCount);
}

bool __fastcall TUtils::GetCodeIndices(TStringsW* sl, int &FirstIdx,
                 int &LastIdx, int &CodeIdx, int RealStart, int RealCount)
{
  wchar_t* pBuf = NULL;
  bool bRet;

  try
  {
    int iSize = sl->TotalLength;

    if (iSize > 0)
    {
      pBuf = sl->GetTextBuf();

      bRet = GetCodeIndices(pBuf, iSize, FirstIdx, LastIdx, CodeIdx,
                                                  RealStart, RealCount);
    }
    else
      bRet = false;
  }
  __finally
  {
    try
    {
      if (pBuf != NULL)
        delete [] pBuf;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: GetCodeIndices() B\r\n");
#endif
      bRet = false;
    }
  }

  // Clean up if error
  if (!bRet)
  {
    FirstIdx = 0;
    LastIdx = 0;
    CodeIdx = 0;

#if DEBUG_ON
    dts->CWrite("\r\nError in: GetCodeIndices() B\r\n");
#endif
  }

  return bRet;
}

bool __fastcall TUtils::GetCodeIndices(wchar_t* pBuf, int iSize,
      int &FirstIdx, int &LastIdx, int &CodeIdx, int RealStart, int RealCount)
// Given a buffer of raw color-codes and real ascii chars,
// returns FirstIdx and LastIdx into the buffer that encompasses
// the real characters specified, including their color/font codes,
// AND including CR/LF pairs as one character-count.
//
// LastIdx - FirstIdx = # of raw chars total, including real chars,
// starting from FirstIdx. LastIdx is returned one greater than the
// 0-based index of the character at RealStart+RealCount.
//
// Example:
//
//             0        1  2  3  4  5  6
// 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
// c 0 1 , 1 3 W c 0 9  H  Y  s  s  !  s
//                      *  *
//
// (where c is the color control-code, and s is spaces)
//
// When we highlight "HY", RealStartIdx = 1, RealCount = 2,
// and the function returns FirstIdx = 10, LastIdx = 12
// and CodeIdx = 7 (it points to "c09").
//
// Call "GetState()" to set the state up to RealStartIndex...
// Call "GetState()" again set the state at the first non-space char
// following the selected-zone.
//
// CodeIdx is returned pointing to the beginning of one or more
// formatting-codes that may be immediately in front of FirstIdx.
//
// If RealCount is 0 we bump it to 1... seems to work (:/)
// A cr/lf is treated as ONE char, the same as the rich-edit control
// computes SelStart.
{
  FirstIdx = -1;
  LastIdx = -1;
  CodeIdx = -1;

  if (!pBuf || iSize <= 0)
    return false;

  int SaveIdx = -1;

  if (RealCount < 0 || RealStart < 0 || RealStart > iSize)
// Note: RealStart >= iSize was causing this function to return false in
// DlgReplaceText in ProcessOC() for the RTF branch when entering the 2nd char
// in a replace box... this fixed it but there may be subtlties and
// ramifications I've not taken time to understand yet - let's just
// try it awhile and see what transpires :-)
//  if (RealCount < 0 || RealStart < 0 || RealStart >= iSize)
    return false;

  // Fudge for case where we pass a SelLength of 0...
  if (RealCount == 0)
    RealCount = 1;

  try
  {
    int RealCharCounter = 0;

    for (int ii = 0; ii < iSize; ii++)
    {
      if (pBuf[ii] == CTRL_K)
      {
        if (CodeIdx == -1)
          CodeIdx = ii;

        ii += this->CountColorSequence(pBuf, ii, iSize);

        continue;
      }

      if (pBuf[ii] == CTRL_F || pBuf[ii] == CTRL_S)
      {
        if (CodeIdx == -1)
          CodeIdx = ii;

        if (CountFontSequence(pBuf, ii, iSize) >= 0)
          ii += 2;

        continue;
      }

      if (AnySpecialChar(pBuf[ii]))
      {
        if (!FoundCRLF(pBuf, iSize, ii))
        {
          // Special wchar_t NOT cr/lf, set CodeIdx!
          if (CodeIdx == -1)
            CodeIdx = ii;

          continue;
        }
      }

      // Note: for a CR or LF by itself, SaveIdx will point to it.
      // For a CR/LF or LF/CR pair SaveIdx will point to the second char.
      // So for a CR/LF, SaveIdx points to the LF character...
      // As we re-iterate the for loop we move beyond the LF and
      // keep going... below we correct this and point to the CR in a
      // CR/LF sequence.

      SaveIdx = ii; // save buf-index of every real wchar_t or cr/lf...

      if (RealCharCounter == RealStart)
      {
        FirstIdx = SaveIdx;

        // No codes found? Use FirstIdx
        if (CodeIdx == -1)
          CodeIdx = FirstIdx;
      }

      if (RealCharCounter == RealStart + (RealCount-1))
      {
        LastIdx = SaveIdx+1;

        // Make a correction so we point to the start of a cr/lf pair
//!!!!!!!!!!!!!!!! search 5/8
        if (FirstIdx < iSize)
        {
          if (FirstIdx-1 >= 0 && pBuf[FirstIdx] == C_LF &&
                                              pBuf[FirstIdx-1] == C_CR)
            FirstIdx--;
        }
        else
        {
#if DEBUG_ON
          dts->CWrite("\r\nFirstIdx is out of range in: GetCodeIndices() C\r\n");
#endif
        }

        if (CodeIdx < iSize)
        {
          if (CodeIdx-1 >= 0 && pBuf[CodeIdx] == C_LF &&
                                            pBuf[CodeIdx-1] == C_CR)
            CodeIdx--;
        }
        else
        {
#if DEBUG_ON
          dts->CWrite("\r\nCodeIdx is out of range in: GetCodeIndices() C\r\n");
#endif
        }

        return true;
      }

      RealCharCounter++;

      if (FirstIdx == -1)
        CodeIdx = -1; // reset unless we reached FirstIdx
    }

    // If SaveIdx is -1 then we had 0 real (printable) characters
    // and the buffer has only codes in it.
    if (SaveIdx == -1)
    {
      CodeIdx = 0; // Point to the start of the codes
      FirstIdx = iSize; // Return FirstIdx == LastIdx
    }
    // Here, we ran out of buffer with either FirstIdx and LastIdx
    // still at -1, or just LastIdx at -1!
    else
    {
      // No real characters up to end of buffer
      if (FirstIdx == -1)
        FirstIdx = SaveIdx+1;

      if (CodeIdx == -1)
        CodeIdx = FirstIdx;
    }

    LastIdx = iSize; // Case where just lastidx is -1

//    ShowMessage("FI:" + WideString(FirstIdx) + " LI:" + WideString(LastIdx) +
//             "\r\nRS:" + WideString(RealStart) + " RC:" + WideString(RealCount) +
//             "\r\nCI:" + WideString(CodeIdx));

    return true;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: GetCodeIndices()\r\n");
#endif
    return false;
  }
}
//---------------------------------------------------------------------------
// Overloaded
int __fastcall TUtils::GetCodeIndex(TTaeRichEdit* re, bool bCodes)
{
  if (re == NULL)
    return -1;

  int RealStart;

  if (re->SelLength > 0)
    RealStart = re->SelStart;
  else
    RealStart = 0;

  return GetCodeIndex(re->TextW, RealStart, bCodes);
}

int __fastcall TUtils::GetCodeIndex(WideString S, int RealStart, bool bCodes)
{
  // Can have an empty string which will cause S.c_bstr() to be NULL!
  if (S.IsEmpty())
    return 0;

  return GetCodeIndex(S.c_bstr(), S.Length(), RealStart, bCodes);
}

int __fastcall TUtils::GetCodeIndex(TStringsW* sl, int RealStart, bool bCodes)
{
  int iSize = sl->TotalLength;
  wchar_t* pBuf = sl->GetTextBuf();
  int iRet = GetCodeIndex(pBuf, iSize, RealStart, bCodes);

  try
  {
    delete [] pBuf;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nUnable to deallocate pBuf in: GetCodeIndex()\r\n");
#endif
  }

  return iRet;
}

int __fastcall TUtils::GetCodeIndex(wchar_t* pBuf, int iSize,
                                                int RealStart, bool bCodes)
// Given RealStart, returns its corresponding 0-based index in pBuf.
// RealStart is the 0-based index into a rich-edit control and
// can be tae->SelStart by itself (which includes line-terminators
// as 1-wchar_t count)
// Returns the 0-based index of the printable wchar_t (or cr/lf) in pBuf
// or -1 if error. Returns iSize if no real chars were found
// or the RealStart given is not in the buffer.
// Set bCodes to return the index of the format-codes before the desired
// wchar_t or cr/lf at RealStart.
//
// To get the index at the last char:
//  GetCodeIndex(pBuf, iSize, STATE_MODE_LASTCHAR, false);
// To get the index of the codes before the last char:
//  GetCodeIndex(pBuf, iSize, STATE_MODE_LASTCHAR, true);
// To get the index of the trailing codes (Call StripCRLF() first if needed):
//  GetCodeIndex(pBuf, iSize, STATE_MODE_ENDOFBUF);
{
  if (pBuf == NULL || iSize <= 0)
    return 0;

  if (RealStart == STATE_MODE_LASTCHAR || RealStart == STATE_MODE_ENDOFBUF)
  {
    int iTemp = GetRealLength(pBuf, iSize);

    if (RealStart == STATE_MODE_LASTCHAR)
      iTemp--;
    else // STATE_MODE_ENDOFBUF implies that we want the trailing codes index
      bCodes = true;

    RealStart = iTemp;
  }

  // If we want CI, Get real index before start of codes for desired char
  if (bCodes)
    RealStart--;

  if (RealStart < 0)
  {
    if (bCodes)
      return 0; // If CI, codes start at 0... we're done

    RealStart = 0;
  }

  try
  {
    int RealCharCounter = 0;

    for (int ii = 0; ii < iSize; ii++)
    {
      int iRet = this->SkipCode(pBuf, iSize, ii);

      if (iRet == S_CRLF || iRet == S_NOCODE)
      {
        if (RealCharCounter++ >= RealStart)
        {
          if (bCodes)
            ii++; // Codes start after this real wchar_t or cr/lf
          else if (iRet == S_CRLF)
            ii--; // back up to start of cr/lf

          return ii;
        }
      }
    }

    return iSize;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: GetCodeIndexA()\r\n");
#endif
    return -1;
  }
}

// This is an efficient routine - uses the power of TStringsW instead
// of using a buffer S.S. Dec 2015
int __fastcall TUtils::GetCodeIndex(TStringsW* sl, int realIdx,
                                          int startLine, bool bCodes)
{
  if (!sl || !sl->Count || startLine < 0)
    return 0;

  int lineCount = sl->Count;

  if (!lineCount || startLine >= lineCount)
    return 0;

  bool bToEnd, bToLast;

  if (realIdx == STATE_MODE_ENDOFBUF)
  {
    bCodes = true;
    bToEnd = true;
  }
  else
    bToEnd = false;

  if (realIdx == STATE_MODE_LASTCHAR)
  {
    bCodes = false;
    bToLast = true;
  }
  else
    bToLast = false;

  // If we want CI, Get real index before start of codes for desired char
  if (bCodes)
    realIdx--;

  if (realIdx < 0)
  {
    if (bCodes)
      return 0; // If CI, codes start at 0... we're done

    realIdx = 0;
  }

  try
  {
    WideString wTemp;

    int lastRealIdx = -1;
    int realCharCounter = 0;
    int masterIndex = 0;

    for (int ii = startLine; ii < lineCount; ii++)
    {
      wTemp = sl->GetString(ii);
      int charCount = wTemp.Length();

      for (int jj = 0; jj < charCount; jj++)
      {
        int iRet = this->SkipCode(wTemp, jj);

        if (iRet == S_CRLF || iRet == S_NOCODE)
        {
          if (bToEnd || bToLast)
          {
            lastRealIdx = jj;
            if (bCodes)
              lastRealIdx++; // Codes start after this real wchar_t or cr/lf
            else if (iRet == S_CRLF)
              lastRealIdx--; // back up to start of cr/lf
            continue;
          }

          if (realCharCounter++ >= realIdx)
          {
            if (bCodes)
              jj++; // Codes start after this real wchar_t or cr/lf
            else if (iRet == S_CRLF)
              jj--; // back up to start of cr/lf

            return masterIndex + jj;
          }
        }
      }

      masterIndex += charCount;

      // Add one for new line
      if (realCharCounter++ >= realIdx)
        return masterIndex;
    }

    if (bToLast)
    {
      if (lastRealIdx < 0)
        return 0;
      return lastRealIdx;
    }

    if (bToEnd)
    {
      if (bCodes)
        return lastRealIdx+1;
      return masterIndex;
    }

    return masterIndex;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: GetCodeIndexB()\r\n");
#endif
    return -1;
  }
}
//---------------------------------------------------------------------
// Overloaded
int __fastcall TUtils::GetRealIndex(TStringsW* sl, int ii)
// Returns the next Real wchar_t index given a 0-based index
// into a string-list containing format-codes.
//
// Scan up from ii to next printable wchar_t or space...
// then Scan from beginning to there, counting printable chars or spaces.
// Counts a cr/lf as 1 real character.
{
  if (sl == NULL || sl->Count <= 0) return 0;

  return GetRealIndex(sl->Text.c_bstr(), sl->TotalLength, ii);
}

int __fastcall TUtils::GetRealIndex(WideString S, int ii)
// Returns the next Real wchar_t index given a 0-based index
// into an ANSI string containing format-codes.
//
// Scan up from ii to next printable wchar_t or space...
// then Scan from beginning to there, counting printable chars or spaces.
// Counts a cr/lf as 1 real character.
{
  return(GetRealIndex(S.c_bstr(), S.Length(), ii));
}

int __fastcall TUtils::GetRealIndex(wchar_t* pBuf, int iSize, int ii)
// Returns the next Real wchar_t index given an IRC-code buffer index.
//
// Scan up from ii to next printable wchar_t or space...
// then Scan from beginning to there, counting printable chars or spaces.
// Counts a cr/lf as 1 real character.
{
  if (ii < 0 || iSize <= 0 || pBuf == NULL)
    return 0;

  // Scan up from ii to next printable wchar_t or space...
  for (; ii < iSize; ii++)
    if (this->SkipCode(pBuf, iSize, ii) == S_NOCODE)
      break;

  // Scanned up and no more real chars
  if (ii >= iSize)
    ii--;

  int RealCounter = 0;

  for (int jj = 0; jj <= ii; jj++)
  {
    int iRet = this->SkipCode(pBuf, ii, jj);

    if (iRet == S_NULL)
      break;

    if (iRet == S_NOCODE || iRet == S_CRLF)
      RealCounter++;
  }

  if (RealCounter > 0)
    RealCounter--;

  return RealCounter;
}
//---------------------------------------------------------------------------
void __fastcall TUtils::SetLine(TTaeRichEdit* re, int idx)
{
  try
  {
    if (idx > re->Lines->Count) idx = re->Lines->Count;
    else if (idx < 0) idx = 0;

    re->Line = idx;
  }
  catch(...)
  {
    re->Line = -1;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::IsAnyOtherFontSize(wchar_t* pBuf, int iSize, int fs)
// Returns true if the document contains at least one font code
// different than fs. (Used to avoid unnecessary HTML font-tags)
{
  for(int ii = 0; ii < iSize; ii++)
  {
    if (pBuf[ii] == CTRL_S)
    {
      int fontSize = CountFontSequence(pBuf, ii, iSize);

      if (fontSize >= 0)
      {
        if (fontSize > 0 && fontSize != fs)
          return true;

        ii += 2;
      }
    }
  }

  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::IsAnyOtherFontType(wchar_t* pBuf, int iSize, int ft)
// Returns true if the document contains at least one font code
// different than fs. (Used to avoid unnecessary HTML font-tags)
{
  for(int ii = 0; ii < iSize; ii++)
  {
    if (pBuf[ii] == CTRL_F)
    {
      int fontType = CountFontSequence(pBuf, ii, iSize);

      if (fontType >= 0)
      {
        if (fontType > 0 && fontType != ft)
          return true;

        ii += 2;
      }
    }
  }

  return false;
}
//---------------------------------------------------------------------
bool __fastcall TUtils::IsNextPrintableSequence(wchar_t* pBuf, int ii,
                                                        int iSize, WideString S)
// Scan ahead for match of S, ignoring special codes.
{
  int Length = S.Length(); // # chars to acquire
  int RealCounter = 0;
  WideString NewStr;

  for (; ii < iSize; ii++)
  {
    int iRet = this->SkipCode(pBuf, iSize, ii);

    if (iRet == S_CRLF || iRet == S_FF || iRet == S_NULL)
      break; // CR/LF?

    if (iRet)
      continue;

    RealCounter++;
    NewStr += WideString(pBuf[ii]);

    if (RealCounter >= Length)
      break;
  }

  if (RealCounter != Length)
    return false;

  if (NewStr == S)
    return true;

  return false;
}
//---------------------------------------------------------------------
// Overloaded
void __fastcall TUtils::ReplaceChar(wchar_t* &pBuf, int iSize,
                                                  wchar_t cOld, wchar_t cNew)
{
  for (int ii = 0; ii < iSize; ii++)
    if (pBuf[ii] == cOld)
      pBuf[ii] = cNew;
}

WideString __fastcall TUtils::ReplaceChar(WideString wIn,
                                                  wchar_t cOld, wchar_t cNew)
{
  int len = wIn.Length();
  for (int ii = 1; ii <= len; ii++)
    if (wIn[ii] == cOld)
      wIn[ii] = cNew;

  return wIn;
}
//---------------------------------------------------------------------
bool __fastcall TUtils::ContainsChar(WideString S, wchar_t c)
// Returns true if S has the wchar_t c in it
{
  int len = S.Length();
  for (int ii = 1; ii <= len; ii++)
    if (S[ii] == c)
      return true;

  return false;
}
//---------------------------------------------------------------------
WideString __fastcall TUtils::StripChar(WideString S, wchar_t c)
// Strips wchar_t "c" out of a string and returns it stripped
{
  WideString New;

  int len = S.Length();
  for (int ii = 1; ii <= len; ii++)
    if (S[ii] != c)
      New += S[ii];

  return New;
}
//---------------------------------------------------------------------
WideString __fastcall TUtils::StripChar(WideString S, wchar_t c, int iStart, int &iEnd)
// Strips wchar_t "c" out of a string and returns it stripped
{
  WideString New;

  WideString sHead = S.SubString(1, iStart);
  WideString sTail = S.SubString(iEnd+1, S.Length()-iEnd);
  WideString sSub = S.SubString(iStart+1, iEnd-iStart);

#if DEBUG_ON
  dts->CWrite( "\r\nStripChar1: " + sHead + "\r\n");
  dts->CWrite( "\r\nStripChar3: " + sSub + "\r\n");
  dts->CWrite( "\r\nStripChar2: " + sTail + "\r\n");
#endif
  return sHead + StripChar(sSub, c) + sTail;
}
//---------------------------------------------------------------------
// Overloaded
WideString __fastcall TUtils::FlattenTabs(WideString S, int mode)
{
  // Copy S
  wchar_t* pBuf = StrNewW(S);
  int newSize = S.Length();
  FlattenTabs(pBuf, newSize, mode);
  S = WideString(pBuf, newSize);

  try
  {
    delete [] pBuf;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nUnable to deallocate pBuf in: FlattenTabs()\r\n");
#endif
  }

  return S;
}

bool __fastcall TUtils::FlattenTabs(wchar_t* &pBuf, int &iSize, int mode)
// mode = 0 => replace tabs with a space
// mode = 1 => remove tabs
// mode = 2 => flatten tabs to multiple spaces
{
  int ii = 0;

  // Check for tabs
  for (; ii < iSize; ii++)
    if (pBuf[ii] == C_TAB)
      break;

  // return if no tabs
  if (ii >= iSize)
    return true;

  ii = 0;

  // Replace the tab with a space
  if (mode == FLATTEN_TABS_SINGLE_SPACE)
  {
    for (; ii < iSize; ii++)
      if (pBuf[ii] == C_TAB)
        pBuf[ii] = C_SPACE;
  }
  // Strip the tab and do not replace with anything
  else if (mode == FLATTEN_TABS_STRIP)
  {
    int jj = 0;
    for (; ii < iSize; ii++)
      if (pBuf[ii] != C_TAB)
        pBuf[jj++] = pBuf[ii];
    pBuf[jj] = C_NULL;
    iSize = jj;
  }
  // Flatten each tab into a string of spaces
  else if (mode == FLATTEN_TABS_MULTIPLE_SPACES)
  {
    WideString wOut;
    WideString wTabs = StringOfCharW(C_SPACE, dts->RegTabs);

    for (; ii < iSize; ii++)
    {
      if (pBuf[ii] != C_TAB)
        wOut += pBuf[ii];
      else
        wOut += wTabs;
    }

    try
    {
      delete [] pBuf; // delete original buffer
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: FlattenTabs()\r\n");
#endif
      return false;
    }

    // Copy wOut
    pBuf = StrNewW(wOut);
    iSize = wOut.Length();
  }
  else
    return false;

  return true;
}
//---------------------------------------------------------------------
// Overloaded
WideString __fastcall TUtils::FormFeedToPageBreaks(WideString S)
{
  // Copy S
  wchar_t* pBuf = StrNewW(S);
  int newSize = S.Length();
  FormFeedToPageBreaks(pBuf, newSize);
  S = WideString(pBuf, newSize);

  try
  {
    delete [] pBuf;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nUnable to deallocate pBuf in: FormFeedToPageBreaks()\r\n");
#endif
  }

  return S;
}

bool __fastcall TUtils::FormFeedToPageBreaks(wchar_t* &pBuf, int &iSize)
{
  int ii = 0;

  // Check for form-feeds
  for (; ii < iSize; ii++)
    if (pBuf[ii] == C_FF)
      break;

  // return if no form-feeds
  if (ii >= iSize)
    return true;

  ii = 0;

  WideString wOut;

  for (; ii < iSize; ii++)
  {
    if (pBuf[ii] != C_FF)
      wOut += pBuf[ii];
    else
      wOut += PAGE_BREAK;
  }

  try
  {
    delete [] pBuf; // delete original buffer
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nUnable to deallocate pBuf in: FormFeedToPageBreaks()\r\n");
#endif
    return false;
  }

  // Copy wOut
  pBuf = StrNewW(wOut);
  iSize = wOut.Length();

  return true;
}
//---------------------------------------------------------------------
// Overloaded
// replace "\pagebreak" in text with C_FF
WideString __fastcall TUtils::PageBreaksToFormFeed(WideString S)
{
  // Copy S
  wchar_t* pBuf = StrNewW(S);
  int newSize = S.Length();
  PageBreaksToFormFeed(pBuf, newSize);
  S = WideString(pBuf, newSize);

  try
  {
    delete [] pBuf;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nUnable to deallocate pBuf in: PageBreaksToFormFeed()\r\n");
#endif
  }

  return S;
}

// replace "\pagebreak" in text with C_FF
bool __fastcall TUtils::PageBreaksToFormFeed(wchar_t* &pBuf, int &iSize)
{
  int ii = 0, jj, offset;
  int len = wcslen(PAGE_BREAK);

  int tempSize = iSize;

  // Check for page-breaks
  for (; ii < tempSize; ii++)
  {
    if (wcsncmp(&pBuf[ii], PAGE_BREAK, len) == 0)
    {
#if DEBUG_ON
      dts->CWrite("\r\nTUtils::PageBreaksToFormFeed, Pagebreak at: " + String(ii) + "\r\n");
#endif

      // ignore if \pagebreak is escaped with '\'
      if (ii > 0 && pBuf[ii-1] == '\\') continue;

      pBuf[ii] = C_FF;
      offset = len-1;

      // move text below, up
      for (jj = ii+1; jj < tempSize; jj++)
        pBuf[jj] = pBuf[jj+offset];
        
      pBuf[jj] = '\0';
    }
  }

  iSize = wcslen(pBuf);

  return true;
}
//---------------------------------------------------------------------------
// replace \pagebreak in text with a real RTF \page
void __fastcall TUtils::PageBreaksToRtf(TTaeRichEdit* re)
{
// adding the "\r" seems to fix our issue of an extra line at the top of
// a new page - caused by \page having a built-in \par in it by our
// \pagebreak having to have \par added...
//
// fix may be to search in two passes...

  WideString wFindStr = WideString(PAGE_BREAK);

  int startPos = 0;
  TSearchTypes2 so = (TSearchTypes2() << st2WholeWord);
  bool bRet;
  int foundPos, foundLength; // returned position and length

  for(;;)
  {
    foundLength = -1; // search to end
    bRet = re->FindTextExW(wFindStr, startPos, so, foundPos, foundLength);
    if (!bRet) break;
#if DEBUG_ON
    dts->CWrite("\r\npage-break pos: " + String(foundPos) + "\r\n");
#endif
    re->SelStart = foundPos;
    re->SelLength = foundLength+1; // get one more to check for a C_CR...

    // This little trick allows us to delete the C_CR which may or
    // may not be present after a \pagebreak. The RTF \page will insert a
    // line implicitly. Without this we get an extra line at the top
    // of all pages after the first page in the print-preview.

    // got this working now if needed!
    //utils->ShowMessageW(tae->GetStringW(line_index));

    WideChar wC = re->SelTextW[foundLength+1];

    if (wC != C_CR)
      re->SelLength = foundLength;

    // use \pagebb (break before the paragraph) instead of \page
    // (using \page causes a blank page to print if it's at the spot
    // auto page breaking happens)
// Only problem is that it won't work in TaeRichEdit FormatRange not
// recognizing it!!!!!!!!!!!
//    re->SelTextRtf = "{\\rtf\\pagebb}"; // replace selected text with raw RTF
    re->SelTextRtf = "{\\rtf\\page}"; // replace selected text with raw RTF
    startPos = foundPos + 1;
  }
}
//---------------------------------------------------------------------
// Overloaded
WideString __fastcall TUtils::StripFont(WideString S, int iStart, int &iEnd, wchar_t c)
// "end" is a reference that will return less than or equal to the
// original value based upon how many chars were deleted.
// set c to either CRTL_S (size) or CTRL_F (type)
{
  // Copy S
  wchar_t* pBuf = StrNewW(S);
  int newSize = S.Length();
  StripFont(pBuf, newSize, iStart, iEnd, c);
  S = WideString(pBuf, newSize);

  try
  {
    delete [] pBuf;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nUnable to deallocate pBuf in: StripFont()\r\n");
#endif
  }

  return S;
}

void __fastcall TUtils::StripFont(wchar_t* &pBuf, int &iSize, int iStart,
                                                        int &iEnd, wchar_t c)
// "end" is a reference that will return less than or equal to the
// original value based upon how many chars were deleted.
// set c to either CRTL_S (size) or CTRL_F (type)
{
  int jj = iStart;
  int iDeleteCount = 0;
  int ii;

  for (ii = iStart; ii < iEnd && ii < iSize; ii++)
  {
    if (pBuf[ii] == c)
    {
      if (CountFontSequence(pBuf, ii, iSize) >= 0)
      {
        ii += 2;
        iDeleteCount += 2+1; // Add the CTRL_F or CTRL_S!
      }

      continue;
    }

    pBuf[jj++] = pBuf[ii];
  }

  // Copy the remainder of the string.
  for (; ii < iSize; ii++) pBuf[jj++] = pBuf[ii];

  pBuf[jj] = C_NULL; // Set new "end of string"
  iSize = jj; // Set reference to possibly shorter string-length.
  iEnd -= iDeleteCount; // Adjust iEnd by # chars deleted.
}
//---------------------------------------------------------------------------
// Overloaded
WideString __fastcall TUtils::StripFgCodes(WideString S, bool bIgnorePushPop)
// bIgnorePushPop defalts false
{
  // Copy S
  wchar_t* pBuf = StrNewW(S);
  int newSize = S.Length();
  StripFgCodes(pBuf, newSize, bIgnorePushPop);
  S = WideString(pBuf, newSize);

  try
  {
    delete [] pBuf;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nUnable to deallocate pBuf in: StripFgCodes()\r\n");
#endif
  }

  return S;
}

void __fastcall TUtils::StripFgCodes(wchar_t* &pBuf, int &iSize,
                                                 bool bIgnorePushPop)
// bIgnorePushPop defalts false
{
  int idx = 0;
  int Pushes = 0;

  for (int ii = 0; ii < iSize; ii++)
  {
    if (!bIgnorePushPop && pBuf[ii] == CTRL_PUSH) Pushes++;
    else if (!bIgnorePushPop && pBuf[ii] == CTRL_POP)
    {
      Pushes--;
      if (Pushes < 0) Pushes = 0;
    }
    else if (!Pushes && pBuf[ii] == CTRL_K)
    {
      // strip
      int fg = NO_COLOR;
      int bg = NO_COLOR;

      ii += this->CountColorSequence(pBuf, ii, iSize, fg, bg);

      // Re-write background code
      if (bg != NO_COLOR)
      {
        WideString Temp;
        WriteSingle(bg, Temp, false);

        int len = Temp.Length();
        for (int jj = 1; jj <= len; jj++)
          pBuf[idx++] = Temp[jj];
      }

      continue;
    }

    pBuf[idx++] = pBuf[ii];
  }

  if (idx < iSize)
  {
    pBuf[idx] = C_NULL;
    iSize = idx;
  }
}
//---------------------------------------------------------------------------
// Overloaded
WideString __fastcall TUtils::StripBgCodes(WideString S, bool bIgnorePushPop)
// bIgnorePushPop defalts false
{
  // Copy S
  wchar_t* pBuf = StrNewW(S);
  int newSize = S.Length();
  StripBgCodes(pBuf, newSize, bIgnorePushPop);
  S = WideString(pBuf, newSize);

  try
  {
    delete [] pBuf;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nUnable to deallocate pBuf in: StripBgCodes()\r\n");
#endif
  }

  return S;
}

void __fastcall TUtils::StripBgCodes(wchar_t* &pBuf, int &iSize,
                                                    bool bIgnorePushPop)
// bIgnorePushPop defalts false
//
// problem: we might replace (Crtl-K)11,2 with (Crtl-K)11,96 which will ADD
// a char to the buffer - the buffer can be BIGGER than the original! So the
// output buffer has to be a WideString...
{
  int Pushes = 0;

  WideString wOut;

  bool bPrevWasSingFg = false;

  for (int ii = 0; ii < iSize; ii++)
  {
    if (!bIgnorePushPop && pBuf[ii] == CTRL_PUSH)
      Pushes++;
    else if (!bIgnorePushPop && pBuf[ii] == CTRL_POP)
    {
      Pushes--;
      if (Pushes < 0)
        Pushes = 0;
    }
    else if (!Pushes && pBuf[ii] == CTRL_K)
    {
      // strip
      int fg = NO_COLOR;
      int bg = NO_COLOR;

      ii += this->CountColorSequence(pBuf, ii, iSize, fg, bg);

      if (fg == NO_COLOR)
      {
        // if solitary bg color immediately after fg color was written,
        // we have to check for a ",1" type of sequence that can falsely
        // be construed as a bg color...
        if (bPrevWasSingFg && bg != NO_COLOR)
        {
          if (ii+1 < iSize && pBuf[ii+1] == ',' &&
                      ii+2 < iSize && iswdigit(pBuf[ii+2]))
            wOut += "," + String(IRCNOCOLOR-1);

          bPrevWasSingFg = false;
        }
      }
      else // have a fg color
      {
        // Re-write foreground code

        WideString wTemp;

        // We run into an odd situation when stripping background-colors...
        // if there is (Ctrl-K)11,22,33 and ",33" is TEXT not a color and we
        // change it to (Ctrl-K)11,33 - suddenly we have ",33" becoming a
        // background color! So to fix it I created IRCNOCOLOR to mean "do
        // not set a color here, ignore this index". Later on the optimizer can
        // replace it or maybe we can just leave it since mIRC ignores it...
        // (but some clients cycle colors viz % 16 so I don't know...)
        if (fg > 0 && ii+1 < iSize && pBuf[ii+1] == ',' &&
                    ii+2 < iSize && iswdigit(pBuf[ii+2]))
        {
#if DEBUG_ON
          dts->CWrite("\r\nTUtils::StripBgCodes(): INSERT IRCNOCOLOR!\r\n");
#endif
          WriteColors(fg, IRCNOCOLOR, wTemp);
        }
        else
        {
          WriteSingle(fg, wTemp, true);
          bPrevWasSingFg = true;
        }

        wOut += wTemp;
      }

      continue;
    }

    if (bPrevWasSingFg)
      bPrevWasSingFg = false;

    wOut += pBuf[ii];
  }

  // reallocate
  try
  {
    delete [] pBuf; // delete the buffer that came in...
    pBuf = StrNewW(wOut);
    iSize = wOut.Length();
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nTUtils::StripBgCodes(): Error re-allocating buffer!\r\n");
#endif
  }
}
//---------------------------------------------------------------------
// Overloaded
WideString __fastcall TUtils::StripColorCodes(WideString S, int iStart, int &iEnd)
// "end" is a reference that will return less than or equal to the
// original value based upon how many chars were deleted.
{
  // Copy S
  wchar_t* newBuf = StrNewW(S);
  int newSize = S.Length();
  StripColorCodes(newBuf, newSize, iStart, iEnd);
  S = WideString(newBuf, newSize);
  delete [] newBuf;
  return S;
}

void __fastcall TUtils::StripColorCodes(wchar_t* &pBuf, int &iSize,
                                             int iStart, int &iEnd)
// "end" is a reference that will return less than or equal to the
// original value based upon how many chars were deleted.
{
  int jj = iStart;
  int iDeleteCount = 0;
  int iTemp;
  int ii;

  for (ii = iStart; ii < iEnd && ii < iSize; ii++)
  {
    if (pBuf[ii] == CTRL_K)
    {
      iTemp = this->CountColorSequence(pBuf, ii, iSize);
      ii += iTemp;
      iDeleteCount += iTemp+1; // Add the CRTL_K!
      continue;
    }

    pBuf[jj++] = pBuf[ii];
  }

  // Copy the remainder of the string.
  for (; ii < iSize; ii++)
    pBuf[jj++] = pBuf[ii];

  pBuf[jj] = C_NULL; // Set new "end of string"
  iSize = jj; // Set reference to possibly shorter string-length.
  iEnd -= iDeleteCount; // Adjust iEnd by # chars deleted.
}
//---------------------------------------------------------------------
// Rich Edit does not return the proper # of hard line breaks in word-wrap
// mode - and going through the text up to the index for each new char
// is very slow... we could compute it at the start of typing then increment
// the line counter for each cr/lf added - but things begin getting complex -
// Rich edit does not actually add the cr/lf until the user types in the
// NEXT printable wchar_t after hitting Enter... so then you need two completely
// different algorithms for wordwrap and non-wordwrap. So - it's not worth it.
// In wordwrap, for now - we just set the control to read-only...

int __fastcall TUtils::GetLine(TTaeRichEdit* re)
{
//  if (re->WordWrap != wwtWindow)
    return re->Line;

//  int Count = 0;

  // FYI:
  // "soft line break consists of two carriage returns and a line feed and
  // is inserted at the end of a line that is broken because of wordwrapping"
  // (So use LF to count lines just in case...)
//  int selLen = re->SelStart+1;
//  int textLen = utils->GetTextLength(re);
//  for (int ii = 1; ii <= selLen && ii <= textLen; ii++)
//    if (re->Text[ii] == LF)
//      Count++;

//  return Count;
}
//---------------------------------------------------------------------
// Wrapper for TextLength (different between old C++ Builder 4 and
// RAD Studio versions!)
long __fastcall TUtils::GetTextLength(TTaeRichEdit* re)
{
  long textLength = re->TextLength;

  if (textLength == 0)
    return 0;

#if !ISRADSTUDIO
    textLength = textLength - re->LineCount + 1;
    if (textLength < 0)
      textLength = 0;
//#if DEBUG_ON
//  DTSColor->CWrite("BORLAND GetTextLength():" + String (textLength) + "\r\n");
//#endif
#else
//#if DEBUG_ON
//  DTSColor->CWrite("RAD GetTextLength():" + String (textLength) + "\r\n");
//#endif
#endif

  return textLength;
}
//---------------------------------------------------------------------
// Overloaded
int __fastcall TUtils::CountChar(WideString S, wchar_t Code)
// Set Code to C_NULL (the default) to count cr/lfs. Set it to a char
// to count the char. Returns the count.
{
  return CountChar(S.c_bstr(), S.Length(), Code);
}

int __fastcall TUtils::CountChar(wchar_t* pBuf, int iSize, wchar_t Code)
// Set Code to C_NULL (the default) to count cr/lfs. Set it to a char
// to count the char. Returns the count.
{
  int Count = 0;

  for (int ii = 0; ii < iSize; ii++)
  {
    if (Code != C_NULL)
    {
      if (pBuf[ii] == Code) Count++;
    }
    else if (FoundCRLF(pBuf,iSize,ii))
      Count++;
  }

  return Count;
}
//---------------------------------------------------------------------
// Overloaded
int __fastcall TUtils::CountCRs(WideString S)
// Returns the count of LF chars.
{
  return CountCRs(S.c_bstr(), S.Length());
}

int __fastcall TUtils::CountCRs(wchar_t* pBuf, int iSize)
// Returns the count of CR or VT chars.
// (don't substitute with a routine that counts LFs - tried and failed
// because the edit control's SelText has only CRs in it or VTs for
// RTF text) (might want to double-check that!)
{
  int Count = 0;

  for (int ii = 0; ii < iSize; ii++)
    if (pBuf[ii] == C_CR || pBuf[ii] == C_VT) Count++;

  return Count;
}
//---------------------------------------------------------------------------
//int __fastcall TUtils::CountCRs(WideString S)
//// Returns number of CR characters in the edit-control's selected-zone
//{
//  int SelectedCRs = 0;
//
//  for (int ii = 1; ii <= S.Length(); ii++)
//    if (S[ii] == C_VT || S[ii] == C_CR)
//      SelectedCRs++;
//
//  return SelectedCRs;
//}
//---------------------------------------------------------------------------
int __fastcall TUtils::CountFontSequence(wchar_t* pBuf, int ii, int iSize)
// Scan ahead for valid font-code (2-digits)
// Returns font if the code is valid, -1 if not
// 00 signifies "return to default font"
// (This function works for both CTRL_F and CTRL_S)
{
  int val = -1;
  wchar_t str[3];

  if (ii+2 < iSize && iswdigit(pBuf[ii+1]) && iswdigit(pBuf[ii+2]))
  {
    str[0] = pBuf[ii+1];
    str[1] = pBuf[ii+2];
    str[2] = C_NULL;
    val = ToIntDefW(WideString(str), -1);
  }

  return val;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::IsAnySpace(WideString S)
{
  return(IsAnySpace(S.c_bstr(), S.Length()));
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::IsAnySpace(wchar_t * pBuf, int iSize)
{
  // Scan string and do not count color-codes or formatting codes
  for (int ii = 0; ii < iSize; ii++)
  {
    int iRet = this->SkipCode(pBuf, iSize, ii);
    if (iRet == S_NULL) break;
    if (iRet) continue;
    if (pBuf[ii] == C_SPACE) return true;
  }

  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::IsAllSpaces(WideString S)
{
  return IsAllSpaces(S.c_bstr(), S.Length());
}
//---------------------------------------------------------------------------

bool __fastcall TUtils::IsAllSpaces(wchar_t* pBuf, int iSize)
{
  // Scan string and do not count color-codes or formatting codes
  for (int ii = 0; ii < iSize; ii++)
  {
    int iRet = this->SkipCode(pBuf, iSize, ii);
    if (iRet == S_NULL) break;
    if (iRet) continue;
    if (pBuf[ii] != C_SPACE) return false;
  }

  return true;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::GetRealLineLengthFromSL(TStringsW* sl,
                                                           int LineNumber)
// Returns the real length of a line in the text stored in a
// string-list. LineNumber is typically the 0-based line index.
//
// This gets called indirectly from a key or mouse-down event handler
// so save the stream's position and restore it!
{
  if (sl == NULL) return 0;

  int Length;

  // Include smileys in length, go to next line...
  if (LineNumber < sl->Count)
    Length = GetRealLength(sl->GetString(LineNumber));
  else
    Length = 0;

  return Length;
}
//---------------------------------------------------------------------
// Overloaded
WideString __fastcall TUtils::StripAllCodes(WideString S)
{
  // Copy S
  wchar_t* newBuf = StrNewW(S);
  int newSize = S.Length();
  StripAllCodes(newBuf, newSize);
  S = WideString(newBuf, newSize);
  delete [] newBuf;
  return S;
}

void __fastcall TUtils::StripAllCodes(wchar_t* &pBuf, int &iSize)
// On entry, pBuf has the text with format-codes and should be
// iSize+1 bytes. iSize should be the text-length... we return the
// text stripped and with a null terminator. The returned
// iSize does not include the terminating C_NULL.
{
  int jj = 0;

  for (int ii = 0; ii < iSize; ii++)
  {
    if (pBuf[ii] == C_NULL)
      break;

    // Keep CR/LF/TAB/FF
    if (pBuf[ii] == C_CR || pBuf[ii] == C_LF ||
                        pBuf[ii] == C_TAB || pBuf[ii] == C_FF)
    {
      pBuf[jj++] = pBuf[ii];
      continue;
    }

    if (this->SkipCode(pBuf, iSize, ii))
      continue;

    pBuf[jj++] = pBuf[ii];
  }

  pBuf[jj] = C_NULL;
  iSize = jj;
}
//---------------------------------------------------------------------------
// Overloaded
int __fastcall TUtils::GetRealLength(WideString S)
// Here is a fast version that gets the real length (smiley chars are
// always counted)
{
  return GetRealLength(S.c_bstr(), S.Length());
}

int __fastcall TUtils::GetRealLength(wchar_t* pBuf, int iSize)
// Here is a fast version that gets the real length (smiley chars are
// always counted)
{
  int count = 0;

  for (int ii = 0; ii < iSize; ii++)
  {
    if (pBuf[ii] == C_CR || pBuf[ii] == C_LF)
      continue;

    if (pBuf[ii] == CTRL_K)
    {
      ii += CountColorSequence(pBuf, ii, iSize);
      continue;
    }

    if (pBuf[ii] == CTRL_F || pBuf[ii] == CTRL_S)
    {
      if (CountFontSequence(pBuf, ii, iSize) >= 0)
        ii += 2;

      continue;
    }

    // Skip any other control-chars
    if (AnySpecialChar(pBuf[ii]))
      continue;

    count++;
  }

  return count;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::GetRealLength(WideString sIn, int &Length)
// Version that returns true if a color was found
{
  int OrigLength = sIn.Length();

  Length = 0;

  if (OrigLength == 0)
    return false;

  bool bHasColor = false;

  // Scan string and do not count color-codes or formatting codes
  for (int ii = 0; ii < OrigLength; ii++)
  {
    int iRet = this->SkipCode(sIn.c_bstr(), OrigLength, ii);

    if (iRet == S_NOCODE)
      Length++;
    else if (!bHasColor && iRet == S_COLOR)
      bHasColor = true;
  }

  return bHasColor;
}
//---------------------------------------------------------------------------
// Overloaded
int __fastcall TUtils::GetRealLength(WideString S, int idx, wchar_t c,
                                    bool bForceEmotesToBeIncludedInCount)
// idx is the 0-based starting index into the string and defaults to 0
// c tells the loop when to quit and defaults to '\0' (search to end)
//
// when bExcludeEmotesFromCount is set, smiley-sequences are excluded from
// the length regardless of dts->AutoDetectEmotes. It defaults true.
{
  return GetRealLength(S.c_bstr(), S.Length(), idx, c,
                                      bForceEmotesToBeIncludedInCount);
}

int __fastcall TUtils::GetRealLength(wchar_t* pBuf, int iSize, int idx,
                          wchar_t c, bool bForceEmotesToBeIncludedInCount)
// Return the "actual" printable text-length to do the color-blending...
// If c is defined, count up through the next occurrence of c unless
// c is C_SPACE.
//
// If '\0' is encountered the loop always breaks out.
// If c is '\0' the loop will not break out until the end of the buffer
//   or until a '\0' is encountered
// If c = "." the loop breaks out on a "." or "?"
//   and adds 1 to the char-count
// If c = " " the loop breaks out on first space before the
//   next word or on "\n"
// if c = "\n" the loop breaks out on "\n"
// If c is any other non-null wchar_t the loop breaks out on c
//
// This routine counts c if it is a printable non-space char.
//
// iSize should be the size of the null-terminated buffer containg
// a mix of raw text and color/font and format-codes
//
// idx is the beginning index into pBuf
{
  int RealLength = 0;

  for(int ii = idx; ii < iSize; ii++)
  {
    int iRet = this->SkipCode(pBuf, iSize, ii);

    if (iRet == S_NULL ||
                (c == C_LF && (iRet == S_CRLF || iRet == S_FF)))
      break; // CR/LF?

    if (iRet)
      continue;

    // We don't count smilies when adding blend/morph text-effects.
    // The AutoDetectEmotes user-setting is checked in ConvertToIRC
    // to keep smileys from being broken up across lines of text...
    // (Usually we want to skip the skipping of smileys!)
    if (!bForceEmotesToBeIncludedInCount && dts->AutoDetectEmotes->Checked)
    {
      WideString Dummy;

      // Advance ii over smileys
      if (SkipSmileys(ii, pBuf, Dummy, iSize)) continue;
    }

    // Check for delimiter
    if (c != C_NULL)
    {
      // Quit on 1st space and don't count it
      if (c == C_SPACE && pBuf[ii] == C_SPACE) break;

      // Quit on end of a sentence and count the char
      if (c == '.' && (pBuf[ii] == '.' || pBuf[ii] == '?'))
      {
        RealLength++;
        break;
      }

      // if c is any other char, break out on it
      if (pBuf[ii] == c)
      {
        RealLength++;
        break;
      }
    }

    RealLength++;
  }

  return RealLength;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::GetWordLength(wchar_t* pBuf, int iSize, int idx, int &hyphidx)
// Returns length of the next word. We read past any leading
// codes/spaces, through the word to the next space or end of line.
//
// idx is the start index into pBuf of size iSize.
//
// hyphidx has the hyphen's offset from idx on return.  For instance,
// for the word "brill-cream", hyphidx will be 5 and we return 11.
// The routine requires an alpha-numeric wchar_t before the hyphen char.
{
  int RealLength = 0;

  hyphidx = -1;

  for(int ii = idx; ii < iSize; ii++)
  {
    // Read past font or color codes
    int iRet = this->SkipCode(pBuf, iSize, ii);

    if (iRet == S_CRLF || iRet == S_FF || iRet == S_NULL)
      break; // CR/LF or FF?

    if (iRet)
      continue;

    if (pBuf[ii] == C_SPACE)
      break;

    // Detected hyphenated word? ('-' or '/')
    // it is concidered to be a hyphen if the wchar_t before
    // and after exists and is alphanumeric
    if (iSize-idx >= 3 && ii > idx && ii < iSize-1 &&
              (pBuf[ii] == HYPHENCHAR1 || pBuf[ii] == HYPHENCHAR2) &&
                   iswalnum(pBuf[ii-1]) && iswalnum(pBuf[ii+1]))
      hyphidx = RealLength;

    RealLength++;
  }

  return RealLength;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::AnySpecialChar(wchar_t c)
// Returns true for cr/lf and control chars but false
// for printable chars and the null char.
{
  if (c != C_NULL && c < C_SPACE)
    return true;

  return false;
}
//---------------------------------------------------------------------
int __fastcall TUtils::SkipCRLF(wchar_t* pBuf, int iSize, int iStart)
{
  for (int ii = iStart; ii < iSize; ii++)
    if (pBuf[ii] != C_CR && pBuf[ii] != C_LF && pBuf[ii] != C_FF)
      return(ii);

  return iStart;
}
//---------------------------------------------------------------------
// Overloaded
void __fastcall TUtils::StripCRLF(wchar_t* &pBuf, int &iSize, int iStart)
{
  wchar_t* p = pBuf;

  // this will move the null at iSize-1 also!
  for (int ii = iStart; ii < iSize; ii++)
    if (pBuf[ii] != C_CR && pBuf[ii] != C_LF && pBuf[ii] != C_FF)
      *p++ = pBuf[ii];

  iSize = p-pBuf;
}

WideString __fastcall TUtils::StripCRLF(WideString s)
{
  WideString TempS;

  int len = s.Length();
  for (int ii = 1; ii <= len; ii++)
    if (s[ii] != C_CR && s[ii] != C_LF && s[ii] != C_FF)
      TempS += s[ii];

  return TempS;
}

String __fastcall TUtils::AnsiStripCRLF(String s)
{
  String TempS;

  int len = s.Length();
  for (int ii = 1; ii <= len; ii++)
    if (s[ii] != C_CR && s[ii] != C_LF && s[ii] != C_FF)
      TempS += s[ii];

  return TempS;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::SkipSmileys(int &idx,  wchar_t buf[],
                                        WideString &TempStr, int Size)
{
  if (!dts->Emotes)
    return false;

  try
  {
    // Scan ahead to detect smiley-codes - don't want to blend them

    // for each smiley in the constant-array...
    for (int ii = 0; ii < dts->Emotes->Count; ii++)
    {
      int len = dts->Emotes->GetString(ii).Length();

      if (idx+len > Size)
        return false;

      int jj;

      // for each wchar_t in the smiley...
      for (jj = 0; jj < len; jj++)
        if (dts->Emotes->GetString(ii)[jj+1] != buf[idx+jj])
          break;

      if (jj == len) // good compare?
      {
        // add the emotion
        TempStr += dts->Emotes->GetString(ii);

        idx += len-1; // Skip over the smiley

        return true;
      }
    }
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: SkipSmileys()\r\n");
#endif
  }

  return false;
}
//---------------------------------------------------------------------------
//bool __fastcall TUtils::WriteFont(int Size, WideString &FontStr)
//// APPENDS a font sequence to the provided string... so be
//// sure to clear the string first!
////
//// Returns false if error or Size is not good
//{
//  if (Size < 0 || Size > 99 || Size == IRCCANCEL)
//    return(false); // -1 is "no font size"
//
//  FontStr += WideString(CTRL_F);
//
//  if (Size < 10)
//    FontStr += "0";
//
//  FontStr += WideString(Size);
//
//  return(true);
//}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::FontSizeToString(int val)
{
  return WideString(FontSizeToStringA(val));
}
//---------------------------------------------------------------------------
String __fastcall TUtils::FontSizeToStringA(int val)
{
  String Temp;

  if (dts->IsYahTrinPal())
  {
    if (val < 0)
      val = USER_DEF_SIZE;

    Temp = CTRL_S;

    if (val > 99)
      val = 99;
    else if (val < 10)
      Temp += '0';

    Temp += String(val);
  }

  return Temp;
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::FontTypeToString(int val)
{
  return WideString(FontTypeToStringA(val));
}
//---------------------------------------------------------------------------
String __fastcall TUtils::FontTypeToStringA(int val)
{
  String Temp;

  if (val < 0)
  {
    if (dts->IsCli(C_PALTALK))
      val = PALTALK_DEF_TYPE;
    else
      val = USER_DEF_TYPE;
  }

  if (dts->IsYahTrinPal())
  {
    Temp = CTRL_F;

    if (val > 99)
      val = 99;
    else if (val < 10)
      Temp += '0';

    Temp += String(val);
  }

  return Temp;
}
//---------------------------------------------------------------------------
// Overloaded
bool __fastcall TUtils::Optimize(bool bShowStatus)
// Optimize the main TaeEdit
{
  if (!IsRtfIrcOrgView()) return true; // no error

  PushOnChange(tae);

  TStringsW* sl;

  if (IsOrgView()) sl = dts->SL_ORG;
  else sl = dts->SL_IRC;

  bool bRet = Optimize(sl, bShowStatus);

  tae->Modified = true;

  // If the view is V_RTF, convert to MS_RTF stream
  if (IsRtfView())
    ConvertToRtf(sl, dts->MS_RTF, tae, bShowStatus);
  else
    dts->LoadView(tae->View);

  PopOnChange(tae);

#if DEBUG_ON
  if (!bRet)
    dts->CWrite( "\r\nTUtils::Optimize() error!\r\n");
#endif

  return bRet;
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// PROBLEM:     utils->Optimize(nsl, false, NO_COLOR);  prints leading codes in
// a document with no codes at all!

bool __fastcall TUtils::Optimize(TStringsW* sl, bool bShowStatus, int fgc)
// fgc defaults to DO_NOTHING which causes the optimizer to use dts->Afg as the
// default fg color. If you set it to NO_COLOR, the optimizer will not print any
// initial fg color. If you set it to IRCRANDOM, ConvertColor will pick a random
// color (below). You can also set it to any RGB or palette color.
{
  if (sl == NULL || sl->TotalLength == 0)
    return false;

  try
  {
    WideString sOut = this->Optimize(sl->Text, bShowStatus, fgc);
    if (sOut.IsEmpty()) return false;
    sl->Text = sOut; // no user cancel
    return true; // success
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: TUtils::Optimize(MS)\r\n");
#endif
    return false;
  }
}

WideString __fastcall TUtils::Optimize(WideString S, bool bShowStatus, int fgc)
// fgc defaults to DO_NOTHING which causes the optimizer to use dts->Afg as the
// default fg color. If you set it to NO_COLOR, the optimizer will not print any
// initial fg color. If you set it to IRCRANDOM, ConvertColor will pick a random
// color (below). You can also set it to any RGB or palette color.
{
  try
  {
    TOptimizer* r = new TOptimizer(dts);

    // Set this property only if we want an initial color
    // other than Afg! Setting it to NO_COLOR will cause the
    // Optimizer to not write any initial FG color.
    if (fgc != DO_NOTHING)
    {
      if (fgc == NO_COLOR)
        r->InitialFG = NO_COLOR;
      else
        r->InitialFG = ConvertColor(fgc, dts->bRgbRandFG);
    }

    S = r->Execute(S, bShowStatus);

    // Strip redundant rgb foreground color-codes in front of spaces
    r->StripRedundantFgRGB(S, bShowStatus);

    // Call twice (first pass may replace RGB codes with normal mIRC
    // colors if the mode is not YahTrinPal. Then we need to go back
    // through...)
    if (!dts->IsYahTrinPal())
      S = r->Execute(S, bShowStatus);

    delete r;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: TUtils::Optimize(S)\r\n");
#endif
  }

  return S;
}
//---------------------------------------------------------------------------
// RGB Conversion Functions
//---------------------------------------------------------------------------
// Overloaded
void __fastcall TUtils::ShowColor(TColor C)
// Display color-panel's status info
{
  ShowColor(-TColorToRgb(C));
}

void __fastcall TUtils::ShowColor(int C)
// Display color-panel's status info
{
  if (dts->Palette == NULL)
  {
    ShowMessage("There is no color palette!");
    return;
  }

  BlendColor BC;

  String S;

  if (C <= 0)
  {
    // Try to get exact palette-match for our RGB color
    int col = ResolveRGBExact(-C);

    if (col > 0)
      C = col;
  }

  if (C > 0)
  {
    C -= 1; // Convert to index
    S = "Palette Color index = " + String(C) + "\n";

    if (C < dts->PaletteSize)
    {
      BC = RgbToBlendColor(dts->Palette[C]);
      S += "RGB Color value = " + String(BlendColorToHex(BC)) + " hex\n";
    }
  }
  else
  {
    BC = RgbToBlendColor(-C);
    S = "RGB Color value = " + IntToHex(-C,6) + " hex\n";
  }

  S += "\nRed: " + String(BC.red) + ", "
       "Green: " + String(BC.green) + ", "
       "Blue: " + String(BC.blue);

  ShowMessage(S);
}
//---------------------------------------------------------------------------
int __fastcall TUtils::ConvertColor(int c, bool bRgbRand)
// c is a code you would get from the color-dialog, allowing 0
// and negative codes as RGB, codes 1-PaletteSize as IRC colors offset up
// by one. And special numbers for vertical-blend, horizontal-blend, random,
// transparent, no-color and "Cancel".
//
// Returns 1-PaletteSize for an index, 100 for transparent or a negative or 0
// for RGB
{
  if (c == IRCRANDOM)
  {
    if (dts->IsYahTrinPal() && bRgbRand)
      c = -GetRandomRGB();
    else
      c = random(dts->PaletteSize)+1;
  }
  else if (c == IRCVERTBLEND || c == IRCHORIZBLEND)
  {
    if (dts->IsYahTrinPal() && dts->FG_BlendColors)
      c = -BlendColorToRgb(dts->FG_BlendColors[0]);
    else
      c = IRCWHITE; // (error)
  }
  else if (c > dts->PaletteSize && c != IRCTRANSPARENT && c != IRCNOCOLOR)
    c = IRCWHITE;

  return c;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::GetRandomRGB(void)
{
  int c;
  BlendColor BC;

  // Require a minimum/maximum luminence
  do
  {
    c = random(0x1000000); // POSITIVE rgb
    BC = RgbToBlendColor(c);
  } while ((BC.red < 32 && BC.green < 32 && BC.blue < 32) ||
                 (BC.red > 256-32 && BC.green > 256-32 && BC.blue > 256-32));

  return c;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::SelectCustomColor(WideString Topic,
                                          TColor InitialColor, int ID)
// Returns color of IRCCANCEL, IRCWHITE etc, 1-PaletteSize or an RGB color
// is 0 or negative.
{
  int C = IRCCANCEL;

  try
  {
    Application->CreateForm(__classid(TColorForm), &ColorForm);

    ColorForm->Caption = Topic;
    ColorForm->CallerID = ID;

    ColorForm->ShowModal();

    C = ColorForm->DlgColor;
    ColorForm->Release();
    ColorForm = NULL;

    if (C == RGBCOLOR)
    {
      TColor newC = InitialColor;

      // Returns new TColor in newC
      if (!GetTColorDialog(newC)) C = IRCCANCEL; // Cancel-button?
      else C = -TColorToRgb(newC); // Convert to our RGB format (negative numbers are rgb)
    }
    else if (C == IRCRANDOM)
    {
      if (!dts->IsYahTrinPal())
        return C;

      // Only do this when setting the main panel colors!
      if (ID == COLOR_FORM_FG || ID == COLOR_FORM_BG || ID == COLOR_FORM_WING)
      {
        bool bTemp;

        // Allow full RGB Spectrum?
        if (MessageDlg(String(DS[176]), mtConfirmation, TMsgDlgButtons()<<mbYes<<mbNo, 0) == mrYes) bTemp = true;
        else bTemp = false;

        if (ID == COLOR_FORM_FG)
        {
          if (bTemp) dts->bRgbRandFG = true;
          else dts->bRgbRandFG = false;
        }
        else if (ID == COLOR_FORM_BG)
        {
          if (bTemp) dts->bRgbRandBG = true;
          else dts->bRgbRandBG = false;
        }
        else if (ID == COLOR_FORM_WING)
        {
          if (bTemp) dts->bRgbRandWing = true;
          else dts->bRgbRandWing = false;
        }
      }
    }
  }
  catch(...) { ShowMessage("Exception thrown in SelectCustomColor()"); }

  return C;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::GetTColorDialog(TColor &Color)
// Returns new Color by reference... Returns false if CANCEL was pressed
{
  TColorDialog* CD = new TColorDialog(DTSColor);
  SetCustomColorsDialogFromPalette(CD);
  CD->Options << cdFullOpen;
  CD->Color = Color;

  if (!CD->Execute())
  {
    delete CD;
    return false; // canceled
  }

  Color = CD->Color;

  // bRet is true if user modified a color...
  bool bRet = GetPaletteFromCustomColorsDialog(CD);

  delete CD;

  if (bRet) dts->UpdateColorButtons(); // update buttons if color was changed

  return true; // return OK
}
//---------------------------------------------------------------------------
void __fastcall TUtils::SetCustomColorsDialogFromPalette(TColorDialog* CD)
{
  if (CD == NULL || dts->Palette == NULL)
    return;

  // Must clear the old colors to Add the new...
  CD->CustomColors->Clear();

  // ColorA - ColorP (max of 16) can be set
  for (int ii = 'A'; ii <= 'P'; ii++)
  {
    // Color-palette hex values appear GREEN,BLUE,RED!!! (inverted)
    TColor C = RgbToTColor(dts->Palette[ii-'A']);
    CD->CustomColors->Add("Color" + WideString((char)ii) + "=" + IntToHex((int)C, 6));
  }
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::GetPaletteFromCustomColorsDialog(TColorDialog* CD)
// returns true if any were changed
{
  if (dts->Palette == NULL)
    return false;

  bool bRet = false;

  try
  {
    for (int ii = 0; ii < MAXDEFAULTCOLORS; ii++)
    {
      if (CD->CustomColors->Strings[ii].Pos("=") == 7)
      {
        String Temp = "0x" + CD->CustomColors->Strings[ii].SubString(8, 6);

        int c = TColorToRgb((TColor)Temp.ToIntDef(0)); // black is default

        if (dts->Palette[ii] != c)
        {
          dts->Palette[ii] = c;
          if (!bRet) bRet = true;
        }
      }
    }
  }
  catch(...) { ShowMessage("Problem ocurred loading color-palette!"); }
  return bRet;
}
//---------------------------------------------------------------------------
// Strip Search/Replace Modes for StripModeForm.cpp and StripCodesClick()
// STRIP_ALL           0
// STRIP_ALL_PRESERVE  1  // Keep codes within push/pop
// STRIP_BOLD          2
// STRIP_UNDERLINE     3
// STRIP_ITALICS       4
// STRIP_FG_COLOR      5
// STRIP_BG_COLOR      6
// STRIP_ALL_COLOR     7
// STRIP_FONT_CODES    8
// STRIP_PUSHPOP       9
// STRIP_CTRL_O        10
// STRIP_TAB           11
// STRIP_FF            12
// these are in seperate menu items, not in DlgStrip.cpp
// STRIP_TRIM_SPACES   30
// STRIP_PAD_SPACES    31
// STRIP_CRLF          32
int __fastcall TUtils::SelectStripMode(int InitMode)
// Invokes a form to get the strip-mode (bold, underline, etc.)
{
  int Mode = -1;

  try
  {
    Application->CreateForm(__classid(TStripModeForm), &StripModeForm);
    StripModeForm->StripMode = STRIP_ALL;

    if (StripModeForm->ShowModal() == mrOk)
      Mode = StripModeForm->StripMode;
      
    StripModeForm->Release();
  }
  catch(...)
  {
    ShowMessage("Exception thrown in SelectStripMode()");
  }

  return Mode;
}
//---------------------------------------------------------------------------
void __fastcall TUtils::WritePrevBlendColors(int PrevFG, int PrevBG,
                                                        WideString &TempStr)
// We call this if the effect is E_BLEND after a CR/LF and also
// after a CTRL_POP wchar_t to place the most-recent blend-color at the
// befinning of a new line or immediately after the state is being
// restored after a pop.
{
  if (dts->FG_BlendEngine && PrevFG != NO_COLOR && PrevFG != IRCNOCOLOR)
    WriteSingle(PrevFG, TempStr, true);

  if (dts->BG_BlendEngine && PrevBG != NO_COLOR && PrevBG != IRCNOCOLOR)
    WriteSingle(PrevBG, TempStr, false);
}
//---------------------------------------------------------------------------
int __fastcall TUtils::WriteSingle(int cl, WideString &S, bool bForeground)
// APPENDS a color sequence to the provided string... so be
// sure to clear the string first!
//
// Returns length added
{
  AnsiString sColor = GetSingleColorStringA(cl, bForeground);
  S += WideString(sColor);
  return sColor.Length();
}
//---------------------------------------------------------------------------
int __fastcall TUtils::WriteSingle(int cl, wchar_t* pBuf, bool bForeground)
// APPENDS a color sequence to the wchar_t string at pBuf
// Returns length added. p is not changed.
{
  AnsiString sColor = GetSingleColorStringA(cl, bForeground);

  int len = sColor.Length();

  for (int ii = 0; ii < len; ii++)
    *pBuf++ = (wchar_t)sColor[ii];

  return sColor.Length();
}
//---------------------------------------------------------------------------
int __fastcall TUtils::WriteSingleA(int cl, String &S, bool bForeground)
// APPENDS a color sequence to the provided string... so be
// sure to clear the string first!
//
// Returns length added
{
  AnsiString sColor = GetSingleColorStringA(cl, bForeground);
  S += sColor;
  return sColor.Length();
}
//---------------------------------------------------------------------------
AnsiString __fastcall TUtils::GetSingleColorStringA(int cl, bool bForeground)
// Returns AnsiString for the given color for either a foreground
// or background color
{
  if (cl == NO_COLOR)
    return "";

  AnsiString ColorStr;

  if (cl <= 0) // RGB?
  {
    if (dts->IsYahTrinPal())
    {
      ColorStr += CTRL_K;

      if (bForeground)
        ColorStr += "#";
      else
        ColorStr += "$";

      ColorStr += IntToHex(-cl, 6);
    }
    else
    {
      cl = ResolveRGBLocal(-cl);

      ColorStr += CTRL_K;

      if (!bForeground)
        // 8/2006 -- this is causing problems in PIRCH/Vortec and in
        // mIRC's newer versions...
        // PIRCH/Vortec can't handle:   (CTRL-C)11X(CTRL-C),12Y
        // (Y foreground color should be color 11 and is not)
        //
        // 8/2008 - fixing it now in Optimizer.cpp
        ColorStr += ",";

      if (cl-1 < 10)
        ColorStr += "0";

      ColorStr += String(cl-1);
    }
  }
  else if (cl < IRCCANCEL)
  {
    ColorStr += CTRL_K;

    if (!bForeground)
      // 8/2006 -- this is causing problems in PIRCH/Vortec and in
      // mIRC's newer versions...
      // PIRCH/Vortec can't handle:   (CTRL-C)11X(CTRL-C),12Y
      // (Y foreground color should be color 11 and is not)
      //
      // 8/2008 - fixing it now in Optimizer.cpp
      ColorStr += ",";

    if (cl-1 < 10)
      ColorStr += "0";

    ColorStr += String(cl-1);
  }

  return ColorStr;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::WriteColors(int fg, int bg, WideString &S)
// APPENDS a color sequence to the provided string... so be
// sure to clear the string first!
//
// NO_COLOR in either fg or bg is handled...
{
  AnsiString sColor = GetColorStringA(fg, bg);
  S += WideString(sColor);
  return sColor.Length();
}
//---------------------------------------------------------------------------
int __fastcall TUtils::WriteColorsA(int fg, int bg, AnsiString &S)
// APPENDS a color sequence to the provided string... so be
// sure to clear the string first!
//
// NO_COLOR in either fg or bg is handled...
{
  AnsiString sColor = GetColorStringA(fg, bg);
  S += sColor;
  return sColor.Length();
}
//---------------------------------------------------------------------------
AnsiString __fastcall TUtils::GetColorStringA(int fg, int bg)
// Returns AnsiString for the given color for foreground/background colors
{
  bool bPrintCtrlK = true;
  bool bWroteFgRGB = false;
  AnsiString ColorStr;

  if (fg <= 0) // RGB?
  {
    if (DTSColor->IsYahTrinPal())
    {
      // Can we resolve the color to an exact palette index?
      int cExact = ResolveRGBExact(-fg);

      if (cExact != 0)
      {
        fg = cExact;
        goto FgExact; // wow, a goto!!!!!!!
      }

      // Check for a transparent bg color - special case where we need to
      // write CTRL_K(00,99) if fg is NOT #ffffff (white) or CTRL_K(01,99)
      // if it is (so the optimizer won't optimize the RGB fg color out!)
      // It must be written BEFORE the fg color in this case so the fg color
      // won't be changed
      if (bg == IRCTRANSPARENT)
      {
        int c = (fg == 0) ? IRCWHITE : IRCBLACK;
        ColorStr = PrintFgA(c) + PrintBgA(bg) + String(CTRL_K) +
                                                "#" + IntToHex(-fg, 6);
        return ColorStr;
      }

      ColorStr = String(CTRL_K) + "#" + IntToHex(-fg, 6);
      bWroteFgRGB = true;
    }
    else // look up closest palette-color!
    {
      fg = ResolveRGBLocal(-fg);
      goto FgExact;
    }
  }
  else if (fg < IRCCANCEL)
  {
FgExact:
    ColorStr = PrintFgA(fg);
    bPrintCtrlK = false;
  }

  if (bg <= 0) // RGB?
  {
    if (DTSColor->IsYahTrinPal())
    {
      // Can we resolve the color to an exact palette index?
      int cExact = ResolveRGBExact(-bg);

      if (cExact != 0 && !bWroteFgRGB)
      {
        bg = cExact;
        goto BgExact; // wow, a goto!!!!!!!
      }

      ColorStr += String(CTRL_K) + "$" + IntToHex(-bg, 6);
    }
    else // look up closest palette-color!
    {
      bg = ResolveRGBLocal(-bg);

      // Special case, we need to print another CTRL_K
      if (bPrintCtrlK)
        ColorStr += CTRL_K;

      ColorStr += PrintBgA(bg);
    }
  }
  else if (bg < IRCCANCEL)
  {
    if (bWroteFgRGB)
      // If we wrote an RGB foreground color we DO NOT want to follow it with
      // and indexed background color (ambiguous) - so get the RGB value from
      // the main 16-color palette...
      ColorStr += String(CTRL_K) + "$" + IntToHex(DTSColor->Palette[bg-1], 6);
    else
    {
BgExact:
      // Special case, we need to print another CTRL_K
      if (bPrintCtrlK)
        ColorStr += CTRL_K;

      ColorStr += PrintBgA(bg);
    }
  }

  return ColorStr;
}
//---------------------------------------------------------------------------
String __fastcall TUtils::PrintFgA(int c)
{
  String S(CTRL_K);

  if (c-1 < 10)
    S += "0";

  S += String(c-1);

  return S;
}
//---------------------------------------------------------------------------
String __fastcall TUtils::PrintBgA(int c)
{
  String S(",");

  if (c-1 < 10)
    S += "0";

  S += String(c-1);

  return S;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::CountColorSequence(wchar_t *pBuf, int ii,
                                              int iSize, int &fg, int &bg)
// Scan ahead for valid color-code and don't count digits
// Returns the length of the color-code without the CTRL_K
// "CTRL_K," with bColorOn1 false returns 0.
// "CTRL_K" or "CTRL_K," with bColorOn1 true returns 0.
// "CTRL_Kn" with bColorOn1 true returns n.  (note that coloring
// highlighted text up to but not including a number or comma is possible
// and will result in that number being taken as a color-sequence number
// if the text is re-rendered. solutions: use CTRL-O instead, lock out
// a color change if color is already in-progress and take any number
// following CTRL-K as printable text. the XirCon chat client does
// not like lone CTRL-K characters that terminate color)
//
// April 13, 2005: Modifying to allow Yahoo-chat RGB values to be embedded
// in the IRC memory-stream (the IRC stream is used both to render the
// rich-text view and the "Yahoo" view.
//
// To facillitate this, a CTRL-K with a # sign folowing will denote that a
// six-digit hex string follows and represents the RGB foreground color.
// No background colors will be embedded. So this routine now checks for
// this custom modification to mIRC's original color-spec.
//
// We set the parameter "fg" to a negative number for RGB color codes.
{
  int R_Length = 0;
  wchar_t T1,T2,T3,T4,T5;
  wchar_t c_str[3];

  // This function will not affect fg or bg unless it assigns
  // one the other or both a color.  The color assigned is negative
  // or 0 for RGB and 1-16 for a color-index.  Set fg/bg to NO_COLOR
  // before calling this if you need to determine which color, if any
  // has changed.

  // First see if this is our custom RGB code...
  if (ii+7 < iSize && (pBuf[ii+1] == '#' || pBuf[ii+1] == '$'))
  {
    int idx = ii + 2; // point to first digit
    WideString HexStr = "0x";

    for (int jj = 0; jj < 6; jj++, idx++)
    {
      // All 6 chars are Hex digits?
      if (!mishexw(pBuf[idx]))
        return 0;

      HexStr += pBuf[idx];
    }

    if (pBuf[ii+1] == '#') // fg
      fg = -ToIntDefW(HexStr, 0x000000); // Use negative number for RGB
    else // bg
      bg = -ToIntDefW(HexStr, 0xffffff); // Use negative number for RGB

    return 7; // #000000 0r $000000
  }

  // Original IRC-oriented code
  T1 = T2 = T3 = T4 = T5 = C_NULL;

  ii += 5;
  if (ii < iSize)
    T5 = pBuf[ii];
  ii--;
  if (ii < iSize)
    T4 = pBuf[ii];
  ii--;
  if (ii < iSize)
    T3 = pBuf[ii];
  ii--;
  if (ii < iSize)
    T2 = pBuf[ii];
  ii--;
  if (ii < iSize)
    T1 = pBuf[ii];

  // CTRL_K xx,xx 5
  if (iswdigit(T1) && iswdigit(T2) && iswdigit(T4) && iswdigit(T5) && T3 == ',')
  {
    R_Length = 5;

    swprintf(c_str, L"%c%c", T1, T2);
    fg = _wtoi(c_str)+1;
    swprintf(c_str, L"%c%c", T4, T5);
    bg = _wtoi(c_str)+1;
  }
  // CTRL_K xx,x 4
  else if (iswdigit(T1) && iswdigit(T2) && iswdigit(T4) && T3 == ',')
  {
    R_Length = 4;

    swprintf(c_str, L"%c%c", T1, T2);
    fg = _wtoi(c_str)+1;
    swprintf(c_str, L"%c", T4);
    bg = _wtoi(c_str)+1;
  }
  // CTRL_K x,xx 4
  else if (iswdigit(T1) && iswdigit(T3) && iswdigit(T4) && T2 == ',')
  {
    R_Length = 4;

    swprintf(c_str, L"%c", T1);
    fg = _wtoi(c_str)+1;
    swprintf(c_str, L"%c%c", T3, T4);
    bg = _wtoi(c_str)+1;
  }
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// mIRC quit supporting this...
// 8/2008 - i am keeping it but will patch it
// in the code-optimizer Optimizer.cpp
  // CTRL_K ,xx 3
  else if (iswdigit(T2) && iswdigit(T3) && T1 == ',')
  {
    R_Length = 3;

    swprintf(c_str, L"%c%c", T2, T3);
    bg = _wtoi(c_str)+1;
  }
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // CTRL_K x,x 3
  else if (iswdigit(T1) && iswdigit(T3) && T2 == ',')
  {
    R_Length = 3;

    swprintf(c_str, L"%c", T1);
    fg = _wtoi(c_str)+1;
    swprintf(c_str, L"%c", T3);
    bg = _wtoi(c_str)+1;
  }
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// mIRC quit supporting this...
// 8/2008 - i am keeping it but will patch it
// in the code-optimizer Optimizer.cpp
  // CTRL_K ,x 2
  else if (iswdigit(T2) && T1 == ',')
  {
    R_Length = 2;

    swprintf(c_str, L"%c", T2);
    bg = _wtoi(c_str)+1;
  }
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // CTRL_K xx 2
  else if (iswdigit(T1) && iswdigit(T2))
  {
    R_Length = 2;

    swprintf(c_str, L"%c%c", T1, T2);
    fg = _wtoi(c_str)+1;
  }
  // CTRL_K x 1
  else if (iswdigit(T1))
  {
    R_Length = 1;

    swprintf(c_str, L"%c", T1);
    fg = _wtoi(c_str)+1;
  }

  return R_Length;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::CountColorSequence(wchar_t* pBuf, int ii, int iSize)
// Enter after detecting a CTRL_K at ii... returns the # of chars
// if it's a valid sequence or 0 if not.
{
  // First see if this is our custom RGB code...
  if (ii+7 < iSize && (pBuf[ii+1] == '#' || pBuf[ii+1] == '$'))
  {
    int idx = ii + 2; // point to first digit

    // All 6 chars are Hex digits?
    for (int jj = 0; jj < 6; jj++, idx++) if (!mishexw(pBuf[idx]))
      return 0;

    return 7; // #000000 0r $000000
  }

  wchar_t T1,T2,T3,T4,T5;
  T1 = T2 = T3 = T4 = T5 = C_NULL;

  int R_Length = 0;

  // Original IRC-oriented code

  ii += 5;
  if (ii < iSize) T5 = pBuf[ii]; ii--;
  if (ii < iSize) T4 = pBuf[ii]; ii--;
  if (ii < iSize) T3 = pBuf[ii]; ii--;
  if (ii < iSize) T2 = pBuf[ii]; ii--;
  if (ii < iSize) T1 = pBuf[ii];

  // CTRL_K xx,xx 5
  if (iswdigit(T1) && iswdigit(T2) && iswdigit(T4) && iswdigit(T5) && T3 == ',')
    R_Length = 5;
  // CTRL_K xx,x 4
  else if (iswdigit(T1) && iswdigit(T2) && iswdigit(T4) && T3 == ',')
    R_Length = 4;
  // CTRL_K x,xx 4
  else if (iswdigit(T1) && iswdigit(T3) && iswdigit(T4) && T2 == ',')
    R_Length = 4;
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// mIRC quit supporting this...
// 8/2008 - i am keeping it but will patch it
// in the code-optimizer Optimizer.cpp
  // CTRL_K ,xx 3
  else if (iswdigit(T2) && iswdigit(T3) && T1 == ',')
    R_Length = 3;
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // CTRL_K x,x 3
  else if (iswdigit(T1) && iswdigit(T3) && T2 == ',')
    R_Length = 3;
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// mIRC quit supporting this...
// 8/2008 - i am keeping it but will patch it
// in the code-optimizer Optimizer.cpp
  // CTRL_K ,x 2
  else if (iswdigit(T2) && T1 == ',')
    R_Length = 2;
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // CTRL_K xx 2
  else if (iswdigit(T1) && iswdigit(T2))
    R_Length = 2;
  // CTRL_K x 1
  else if (iswdigit(T1))
    R_Length = 1;

  return R_Length;
}
//----------------------------------------------------------------------------
bool __fastcall TUtils::mishexw(wchar_t c)
{
  if ((c >= L'0' && c <= L'9') || (c >= L'a' && c <= L'f') ||
                                              (c >= L'A' && c <= L'F'))
    return true;

  return false;
}
//----------------------------------------------------------------------------
bool __fastcall TUtils::mishex(char c)
{
  if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                                                  (c >= 'A' && c <= 'F'))
    return true;

  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::IsDarkColor(int C)
// Pass in a Yc style color like Afg or Abg
{
  if (C == IRCBLACK) return true;

  // Convert to -RGB if in the palette
  if (C > 0 && C <= dts->PaletteSize)
    C = -dts->Palette[C-1];

  if (C <= 0) // RGB?
  {
    BlendColor BC = RgbToBlendColor(-C);

    if (BC.red < 40 && BC.green < 40 && BC.blue < 40)
      return true;
  }

  return false;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::TColorToYc(TColor C)
// Given a TColor, returns a YahCoLoRiZe style color
// <= 0 is RGB, 1-PaletteSize is a 1-based IRC palette index to a color
// such as IRCBLACK
{
  int col = ResolveRGBExact(C);
  if (col != 0) return col;
  return -TColorToRgb(C);
}
//---------------------------------------------------------------------------
TColor __fastcall TUtils::YcToTColor(int C)
// Given IRC color (index 1-PaletteSize), or an RGB color (negative or 0),
// returns RGB color in TColor format.
//
// (Returns TColor as WHITE if error)
{
  if (C <= 0)
    return(RgbToTColor(-C)); // RGB

  // Should never be this when calling YcToTColor()!
  if (C > dts->PaletteSize)
    C = IRCWHITE; // Error

// Wrap to a 16-color palette (mIRC no-longer does this it displays
// user's default colors!)
// C = ((C-1+MAXPALETTECOLORS) % MAXPALETTECOLORS) + 1;

  return RgbToTColor(dts->Palette[C-1]);
}
//---------------------------------------------------------------------------
TColor __fastcall TUtils::RgbToTColor(int C)
// Send this either a positive or negative rrggbb value. Returns a TColor
{
  if (C < 0)
    C = -C;

  WideString Temp = IntToHex(C, 6);
  WideString Red = WideString(Temp[1]) + WideString(Temp[2]);
  WideString Green = WideString(Temp[3]) + WideString(Temp[4]);
  WideString Blue = WideString(Temp[5]) + WideString(Temp[6]);

  // Reverse Hex
  Temp = "0x00" + Blue + Green + Red;

  return StringToColor(Temp);
}
//---------------------------------------------------------------------------
BlendColor __fastcall TUtils::YcToBlendColor(int C)
// Given IRC color (index 1-PaletteSize), or an RGB color (negative or 0),
// returns RGB color in BlendColor format.
//
// (Returns TColor as WHITE if error)
{
  if (C <= 0)
    return RgbToBlendColor(-C); // RGB

  if (C > dts->PaletteSize)
    C = IRCWHITE; // Error

  return RgbToBlendColor(dts->Palette[C-1]);
}
//---------------------------------------------------------------------------
int __fastcall TUtils::YcToRgb(int C)
// Returns a negative RGB value given an index 1 to PaletteSize into
// our local Palette.
//
// If the value is already RGB it is just passed through...
// Returns NO_COLOR otherwize.
//
// PASS THIS EITHER A POSITIVE PALETTE INDEX OR A NEGATIVE RGB COLOR!!!!!
{
  if (C <= 0)
    return C; // pass through RGB

  if (C > dts->PaletteSize)
    C = IRCWHITE;

  return -dts->Palette[C-1];
}
//---------------------------------------------------------------------------
int __fastcall TUtils::RgbToYc(int C)
// Send this either a positive or negative rrggbb value (for example,
// you might pass in a computed rgb value)
//
// If the RGB color exists in the color-palette, its index is returned;
// otherwize, a negative RGB value is returned.
//
// DO NOT SEND A COLOR PALLETTE INDEX!!!!!
{
  if (C < 0)
    C = -C;

  int val = ResolveRGBExact(C);

  if (val > 0)
    return val;

  return -C;
}
//---------------------------------------------------------------------------
BlendColor __fastcall TUtils::RgbToBlendColor(int C)
// Send this either a positive or negative rrggbb value
// Values in the rgb palette are already positive...
//
// DO NOT SEND A COLOR PALLETTE INDEX!!!!!
{
  if (C < 0)
    C = -C;

  BlendColor NewC;

  try { NewC = HexToBlendColor(IntToHex(C, 6)); } // Given int, returns BlendColor struct
  catch(...) { NewC.red = 0; NewC.green = 0; NewC.blue = 0; }

  return NewC;
}
//---------------------------------------------------------------------------
TColor __fastcall TUtils::HexToTColor(WideString Hex)
// Send this a string like "RRGGBB" or "0xRRGGBB"
{
  return BlendColorToTColor(HexToBlendColor(Hex));
}
//---------------------------------------------------------------------------
BlendColor __fastcall TUtils::HexToBlendColor(WideString Hex)
// Send this a string like "RRGGBB" or "0xRRGGBB"
{
  BlendColor NewC;

  Hex = TrimW(Hex);

  if (Hex.Pos(L"0x") == 1)
    Hex = DeleteW(Hex, 1, 2);

  try
  {
    WideString Red = WideString(L"0x") + WideString(Hex[1]) + WideString(Hex[2]);
    WideString Green = WideString(L"0x") + WideString(Hex[3]) + WideString(Hex[4]);
    WideString Blue = WideString(L"0x") + WideString(Hex[5]) + WideString(Hex[6]);

    NewC.red = ToIntDefW(Red, 0);
    NewC.green = ToIntDefW(Green, 0);
    NewC.blue = ToIntDefW(Blue, 0);
  }
  catch(...)
  {
    NewC.red = 0;
    NewC.green = 0;
    NewC.blue = 0;
  }

  return NewC;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::TColorToRgb(TColor C)
// Given a TColor (COLORREF in bbggrr format), reverses the colors and returns
// a POSITIVE int in RGB format
{
  return BlendColorToRgb(TColorToBlendColor(C));
}
//---------------------------------------------------------------------------
int __fastcall TUtils::BlendColorToRgb(BlendColor C)
// Returns positive integer RGB number - in most cases you need to negate it
{
  // Given BlendColor struct, returns binary RGB - IMPORTANT, lead string with
  // 0 to avoid its conversion as a negitive, say for rgb=ffffff.
  WideString Hex = "0x0" + BlendColorToHex(C);

  return ToIntDefW(Hex, 0);
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::TColorToHex(TColor C)
{
  return BlendColorToHex(TColorToBlendColor(C));
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::BlendColorToHex(BlendColor C)
{
  // Given BlendColor struct, returns hex RGB
  WideString Hex = IntToHex(C.red, 2) + IntToHex(C.green, 2) + IntToHex(C.blue, 2);

  return Hex;
}
//---------------------------------------------------------------------------
//TColor __fastcall TUtils::BlendColorToTColor(BlendColor C)
//// Given RGB color as a BlendColor struct, returns color
//// as a TColor
//{
//  if (C.red > 255 || C.green > 255 || C.blue > 255)
//  {
//    utils->ShowMessageU(DS[95] + "BlendColorToTColor()");
//    return StringToColor("0x00000000");
//  }
//
//  WideString Red = IntToHex(C.red, 2);
//  WideString Green = IntToHex(C.green, 2);
//  WideString Blue = IntToHex(C.blue, 2);
//
//  // Reverse Hex
//  WideString Temp = "0x00" + Blue + Green + Red;
//
//  return StringToColor(Temp);
//}
//---------------------------------------------------------------------------
TColor __fastcall TUtils::BlendColorToTColor(BlendColor C)
// Given RGB color as a BlendColor struct, returns color
// as a TColor
{
  return (TColor)RGB(C.red, C.green, C.blue);
}
//---------------------------------------------------------------------------
BlendColor __fastcall TUtils::TColorToBlendColor(TColor C)
{
  BlendColor bc;
  bc.red = GetRValue(C);
  bc.green = GetGValue(C);
  bc.blue = GetBValue(C);
  return bc;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::PanelColorDialog(TPanel* p, WideString s, TColor c, int mode)
// p is the color panel to get a color for
// s is the color dialog caption
// c is the default panel color clAqua etc
// mode is COLOR_FORM_EBG etc
//
// Returns
{
  int C = SelectCustomColor(s, c, mode);
  if (C != IRCCANCEL) SetPanelColorAndCaption(p, C);
  return C;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::SetPanelColorAndCaption(TPanel* p, int c)
// Sets a color panel's TColor and optional caption-letter: R, T, V, H
// (for random, transparent, vertical and horizontal blending)
//
// c is a value from the color-dialog which can be RGB, a table-index or IRCCANCEL
// it can also be any of the special states like IRCRANDOM...
//
// Returns false if exception thrown
{
  if (c == IRCCANCEL || c == NO_COLOR)
    return true;

  try
  {
    // If we are going to blend, BG color won't get used
    if (c == IRCVERTBLEND || c == IRCHORIZBLEND)
    {
      if (p->Name == "FgPanel")
        p->Color = GetFirstEnabledBlendButtonColor(dts->FG_BlendColors);
      else
        p->Color = GetFirstEnabledBlendButtonColor(dts->BG_BlendColors);

      if (c == IRCVERTBLEND)
        p->Caption = BLEND_CAPTION_VERT;
      else
        p->Caption = BLEND_CAPTION_HORIZ;
    }
    else
    {
      if (c == IRCRANDOM)
      {
        p->Color = clWhite;
        p->Caption = RANDOM_CAPTION;
      }
      else if (c == IRCTRANSPARENT)
      {
        p->Color = clWhite;
        p->Caption = TRANSPARENT_CAPTION;
      }
      else
      {
        p->Color = YcToTColor(c);
        p->Caption = "";
      }
    }

    BlendColor BC = TColorToBlendColor(p->Color);

    if (BC.red > 220 || BC.green > 220 || BC.blue > 220)
      p->Font->Color = clBlue;
    else
      p->Font->Color = clWhite;

    return true;
  }
  catch(...) { return false; }
}
//---------------------------------------------------------------------------
TColor __fastcall TUtils::GetFirstEnabledBlendButtonColor(BlendColor* BC)
{
  TColor C = clWhite;

  if (BC == NULL) return C;

  int ii;

  // Get count of enabled blend-structs
  for (ii = 0; ii < MAXBLENDCOLORS; ii++) if (BC[ii].enabled) break;
  if (ii == MAXBLENDCOLORS) return C;
  C = BlendColorToTColor(BC[ii]);
  return C;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::ResolveRGBLocal(int Color)
// Get index 1-PaletteSize of closest color from default palette
//(input is positive RGB color)
{
  return(ResolveRGBLocal(RgbToBlendColor(Color)));
}
//---------------------------------------------------------------------------
int __fastcall TUtils::ResolveRGBLocal(BlendColor BC)
// Returns an index 1-PaletteSize into an array of int from our local Palette.
{
  int smallestdistance = 500000; // big miscelaneous number
  int retval = 0;
  BlendColor P;

  for (int ii = 0; ii < dts->PaletteSize; ii++)
  {
    P = RgbToBlendColor(dts->Palette[ii]);

    // Geometric distance
    // largest val (255*255)+(255*255)+(255*255) = 195075
    int distance = (BC.red-P.red)*(BC.red-P.red) +
              (BC.green-P.green)*(BC.green-P.green) +
                      (BC.blue-P.blue)*(BC.blue-P.blue);

    if (distance < smallestdistance)
    {
      smallestdistance = distance;
      retval = ii+1;
    }
  }

  return retval;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::ResolveRGBExact(TColor c)
// Returns an index 1 to PaletteSize into an array of int from our local
// Palette or 0 if exact color is not in table.
// (c is a Windows TColor (bbggrr))
{
  return ResolveRGBExact(TColorToRgb(c));
}

int __fastcall TUtils::ResolveRGBExact(int c)
// Returns an index 1 to PaletteSize into an array of int from our local
// Palette or 0 if exact color is not in table.
// (c is a positive binary RGB color)
{
  for (int ii = 0; ii < dts->PaletteSize; ii++)
    if (dts->Palette[ii] == c) return ii+1;

  return 0;
}
//---------------------------------------------------------------------------
// End RGB Conversion Functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Start Cut/Copy
//---------------------------------------------------------------------------
// Overloaded
PASTESTRUCT __fastcall TUtils::CutCopyIRC(bool bCut, bool bCopy,
                                            TTaeRichEdit* re, WideString &wText)
{
  PASTESTRUCT ps;
  ps.delta = 0;
  ps.lines = 0;

  TStringsW* sl = NULL;

  try
  {
    if ((sl = new TStringsW(wText)) != NULL)
      ps = CutCopyIRC(bCut, bCopy, re, sl);
    else
      ps.error = -1; // error code
  }
  __finally
  {
    if (sl)
      delete sl;
  }

  return ps;
}

// NOTE: Wrapper (above) handles Push/Pop of OnChange here!
PASTESTRUCT __fastcall TUtils::CutCopyIRC(bool bCut, bool bCopy,
                                        TTaeRichEdit* re, TStringsW* sl)
{
  PASTESTRUCT ps;
  ps.Clear();

  // Some checks...
  if (re == NULL)
  {
    ps.error = -10; // error code
    return ps;
  }

  int selLen = re->SelLength;
  if (!selLen || sl->Count == 0)
    return ps; // Don't want an error here

  // Block OnChange
  PushOnChange(re);

  // Get select-zone indices
  TPoint ptSelStart = re->GetSelStartPos();
  TPoint ptSelEnd = re->GetSelEndPos();

  try
  {
    // Need to copy the codes from re->TextW?
    if (bCopy)
    {
      try
      {
        // NOTE: Don't remove trailing codes here! We might be
        // copying a formatting char!
        ClearClipboard();
        CopyTextToClipboard(sl->CopyText(ptSelStart, ptSelEnd));
      }
      catch(...)
      {
        ps.error = -11; // error code
      }
    }

    // Need to cut the codes from re->TextW?
    if (bCut)
    {
      try
      {
        // Set return info (used in find/replace)
        // # of single CRs in the selected text
        // Negative since it's a cut!
        ps.lines = -(ptSelEnd.y-ptSelStart.y);

        // ps.delta is the number of chars cut including
        // each cr/lf as a count of 2. SelText has a CR
        // ONLY as a line-terminator so we have to ADD the
        // # CRs counted to convert to # cr/lfs...
        ps.delta = -(selLen + (-ps.lines));

        // If this is the main edit-control, add Undo info
        if (re->Name == tae->Name)
        {
          ONCHANGEW oc = GetInfoOC();
          TOCUndo->Add(UNDO_DEL, ptSelStart.x, 0, oc,
                                    sl->CopyText(ptSelStart, ptSelEnd));
        }

        sl->DeleteText(ptSelStart, ptSelEnd);

        re->SelTextW = ""; // Cut the highlighted text
        re->Modified = true;
      }
      catch(...)
      {
        ps.error = -12; // error code
      }
    }
  }
  catch(...)
  {
    ps.error = -13; // error code
  }

  PopOnChange(re);

  return ps;
}
//---------------------------------------------------------------------------
// Overloaded
PASTESTRUCT __fastcall TUtils::CutCopyRTF(bool bCut, bool bCopy,
                                      TTaeRichEdit* re, WideString &wText)
{
  PASTESTRUCT ps;
  ps.delta = 0;
  ps.lines = 0;

  TStringsW* sl = NULL;

  try
  {
    if ((sl = new TStringsW(wText)) != NULL)
      ps = CutCopyRTF(bCut, bCopy, re, sl);
    else
      ps.error = -1; // error code
  }
  __finally
  {
    if (sl)
      delete sl;
  }

  return ps;
}

// NOTE: Wrapper (above) handles Push/Pop of OnChange here!
PASTESTRUCT __fastcall TUtils::CutCopyRTF(bool bCut, bool bCopy,
                                         TTaeRichEdit* re, TStringsW* sl)
// returned ps.delta is negative if text cut!
{
  PASTESTRUCT ps;
  ps.Clear();

  int selLen = re->SelLength;
  if (!selLen || !sl->Count)
    return ps;

  TPoint ptSelStart = re->GetSelStartPos();
  TPoint ptSelEnd = re->GetSelEndPos();

  // Flag if this is the main edit-control
  bool bIsMainEdit = (re->Name == tae->Name) ? true : false;

  WideString wTemp;

  try
  {
    // Get state at first wchar_t of the selected text
    WideString LeadingState;

    if (GetState(sl->GetString(ptSelStart.y), LeadingState,
                                        ptSelStart.x, true, false) < 0)
    {
      ps.error = -10;
      return ps;
    }

//#if DEBUG_ON
//    int LinesSelected = ptSelEnd.y - ptSelStart.y;
//    dts->CWrite("\r\nCutCopyRTF():LinesSelected: " + String(LinesSelected) + "\r\n");
//#endif

    // Get state at first nonspace wchar_t after the selected text
    // but tracking the bg color changes for spaces up to that char.
    PUSHSTRUCT TrailingState;

    // Pass an EndIdx that is the real-wchar_t index from the start
    // of the wText string.
    GetState(sl->GetString(ptSelEnd.y), TrailingState, ptSelEnd.x, false);

    // Map the start and end indices of the RTF selected text to the
    // text in the buffer

    // TODO: need to re-write GetCodeIndices to be string-list efficient!

    int iFirst = this->GetCodeIndex(sl, ptSelStart.x, ptSelStart.y, false);
    int CI = this->GetCodeIndex(sl, ptSelStart.x, ptSelStart.y, true);
    int iLast = this->GetCodeIndex(sl, ptSelEnd.x, ptSelEnd.y, false);

    if (iFirst < 0 || iLast < 0)
    {
      ps.error = -11;
      return ps;
    }

    // Need to copy the codes from re->TextW?
    if (bCopy)
    {
      try
      {
        ClearClipboard();

        // Note: I've been up, down and around the bend with this issue...
        // to include terminators at the end of the copied text for bold,
        // underline, reverse and colors and font... after much cogitation
        // and writing and deleting of code - here is the conclusion: NO.
        // Do not terminate the text! Why? It is fine to paste this string
        // into a chat-client... it goes one line at a time... and when we
        // paste anything into YahCoLoRiZe in the V_RTF mode we get the
        // state at the insert point and add that to the end of the paste
        // string!
        TPoint ptStart, ptEnd;
        ptStart.y = ptSelStart.y;
        ptStart.x = iFirst;
        ptEnd.y = ptSelEnd.y;
        ptEnd.x = iLast;
        wTemp = sl->CopyText(ptStart, ptEnd);

        dts->CpReset(); // precaution (force reset of CancelPanel)

        wTemp = LeadingState + wTemp;

// ... be nice to concolodate this crap one day...
// First call to CpInit initializes, subsequent calls add panels... but it
// would be nice to embed the number of required segments in every class
// that has a loop... or a static method in each that computs the # loops based
// on the Client and View... then we just call the ones we need,
// MyClass::GetNumLoops() + MyOtherClass::GetNumLoops();
        int maxSections = utils->IsRtfView() ? 1 : 2;
        if (!dts->IsYahTrinPal()) // must be calling strip?
          maxSections += 2;

        dts->CpInit(maxSections);

        // Must optimize
        wTemp = Optimize(wTemp, false, NO_COLOR);

        dts->CpInit(3); // add 3

        // Turn off status panel... gets in the way...
        bool bShowStatus = false;

        // Write RTF to clipboard
        if (!CopyRtfToClipboard(ConvertToRtf(wTemp, -1,
                           IRCBLACK, IRCWHITE, false, bShowStatus)))
        {
#if DEBUG_ON
          dts->CWrite("\r\nError in TUtils::CutCopyRtf()\r\n");
#endif
        }

        // Write HTML and UTF-16 text to the clipboard
        TConvertToHTML* cHtm = new TConvertToHTML(dts);

        if (cHtm != NULL)
        {
          // Write to both HTML and UTF-16 text to the clipboard, not file
          cHtm->bClip = true;

          // Office 2007 will paste HTML instead of RTF if both are on the
          // clipboard... and its HTML decoder does NOT read the white-space
          // style...
          cHtm->bNonBreakingSpaces = true;

          cHtm->bShowStatus = bShowStatus;

          // cHtm->LineHeight defaults to the main
          // dts->HtmlLineHeight property!
          //cHtm->LineHeight = 0; // "inherit"
          //cHtm->LineHeight = -1; // "normal"
          //cHtm->LineHeight = 120; // x/100 em
          cHtm->Convert(wTemp, false); // no filename dialog needed
          delete cHtm;
          dts->CpHide();
// can use true to force the status panel to close but that could hide
// an error in the # expected loops passed to CpInit()!
//          dts->CpReset();
        }
      }
      catch(...)
      {
        ps.error = -12;
        return ps;
      }
    }

    // Need to cut the codes from re->TextW?

    if (bCut)
    {
      try
      {
        TPoint ptStart, ptEnd;
        ptStart.y = ptSelStart.y;
        ptStart.x = CI;
        ptEnd.y = ptSelEnd.y;
        ptEnd.x = iLast;

        // If this is the main edit-control, add Undo info
        if (bIsMainEdit)
        {
          ONCHANGEW oc = GetInfoOC();

          // If this is the main edit-control, add Undo info
          TOCUndo->Add(UNDO_DEL, CI, 0, oc, sl->CopyText(ptStart, ptEnd));
        }

        sl->DeleteText(ptStart, ptEnd); // delete old leading codes too (CI)

        // Get state after chars are cut
        PUSHSTRUCT NewState;
        wTemp = sl->GetString(ptSelStart.y);
        GetState(wTemp, NewState, ptSelStart.x, false);

//ShowHex(LeadingState);
//ShowHex(PrintStateString(TrailingState, true));
//ShowHex(PrintStateString(NewState, true));
//ShowHex(wNewState);

        WideString wNewState = ResolveStateForCut(TrailingState, NewState);

        // If this is the main edit-control, add Undo info
        // Previous Undo object is a sub-set of this one so set the "Chain"
        // flag... (it will auto-undo the previous undo object up the chain)
        if (bIsMainEdit)
        {
          ONCHANGEW oc = GetInfoOC();
          // set bChain flag
          TOCUndo->Add(UNDO_INS, CI, wNewState.Length(), oc, "", true);
        }

        // Write back line at SelStart with wNewState inserted
        sl->SetString(InsertW(wTemp, wNewState, CI+1), ptSelStart.y);

        // Block OnChange
        PushOnChange(re);

        // Set return info (used in find/replace)
        // # of single CRs in the selected text
        // Negative since it's a cut!
        ps.lines = -(ptSelEnd.y - ptSelStart.y);

        // ps.delta is the number of chars cut including
        // each cr/lf as a count of 2. SelText has a CR
        // ONLY as a line-terminator so we have to ADD the
        // # CRs counted to convert to # cr/lfs...
        ps.delta = -(selLen+(-ps.lines));

        re->SelTextW = ""; // Cut the highlighted text
        re->Modified = true;

        PopOnChange(re);
      }
      catch(...)
      {
        ps.error = -13;
        return ps;
      }
    }
  }
  catch(...)
  {
    ps.error = -14;
    return ps;
  }

  return ps;
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::ResolveStateForCut(PUSHSTRUCT TrailingState,
                                                        PUSHSTRUCT NewState)
{
  PUSHSTRUCT psTemp;
  psTemp.Clear();
  if (TrailingState.bBold != NewState.bBold)
    psTemp.bBold = true;
  if (TrailingState.bUnderline != NewState.bUnderline)
    psTemp.bUnderline = true;
  if (TrailingState.bItalics != NewState.bItalics)
    psTemp.bItalics = true;
  if (TrailingState.fontSize != NewState.fontSize)
    psTemp.fontSize = TrailingState.fontSize;
  if (TrailingState.fontType != NewState.fontType)
    psTemp.fontType = TrailingState.fontType;
  if (TrailingState.fg != NewState.fg)
    psTemp.fg = TrailingState.fg;
  if (TrailingState.bg != NewState.bg)
    psTemp.bg = TrailingState.bg;
  return PrintStateString(psTemp, true);
}
//---------------------------------------------------------------------------
// wInOut on In has the line we want to paste into
// wPaste has the string we want to paste, we will be stripping its old
// state codes and writing new...
// wInOut on Out has the modified text line with possible CRLF chars for
// any additional lines. The calling function then should call SetStringEx()
// in the TStringsW class to paste text containing line-breaks.
int __fastcall TUtils::ResolveStateForPaste(TPoint p, WideString &wInOut,
                                                      WideString wPaste)
{
  try
  {
    // Get the state and raw code index at our caret
    PUSHSTRUCT psInsertPoint;
    utils->SetStateFlags(wInOut, p.x, psInsertPoint);

    // Get trailing state of string we are pasting
    PUSHSTRUCT  psSnippetEnd;
    utils->SetStateFlags(wPaste, STATE_MODE_ENDOFBUF, psSnippetEnd);

    // if we are pasting into a plain string (no color)
    // then prepend a default string to wInOut
    // (This came about for the case of pasting colors
    // into a plain wInOutg... need to write an initial state!)
    PUSHSTRUCT psTemp;
    psTemp.Clear();
    if (psInsertPoint.fg == NO_COLOR && psSnippetEnd.fg != NO_COLOR)
      psTemp.fg = dts->Afg;
    if (psInsertPoint.bg == NO_COLOR && psSnippetEnd.bg != NO_COLOR)
      psTemp.bg = dts->Abg;
    if (psInsertPoint.fontSize == -1 &&  psSnippetEnd.fontSize != -1)
      psTemp.fontSize = dts->cSize;
    if (psInsertPoint.fontType == -1 && psSnippetEnd.fontType != -1)
      psTemp.fontType = dts->cType;

    if (!psTemp.IsClear())
    {
      wInOut = InsertW(wInOut, PrintStateString(psTemp, true), 1);
      // refresh the state...
      utils->SetStateFlags(wInOut, p.x, psInsertPoint);
    }

    // get index of printable char
    int idxChar = GetCodeIndex(wInOut, p.x, false);
    // get index at start of codes before caret
    int idxCodes = GetCodeIndex(wInOut, p.x, true);

    if (idxCodes >= 0 && idxChar >= 0)
    {
      int delLen = idxChar - idxCodes;

      // Get the old codes for analysis before we delete them
      PUSHSTRUCT psInsertPointCodes;
      utils->SetStateFlags(wInOut.SubString(idxCodes+1, delLen),
                                      idxCodes+1, psInsertPointCodes);

      // delete old codes
      wInOut = utils->DeleteW(wInOut, idxCodes+1, delLen);

      // get leading state of string we are pasting and put its length
      // in delLen
      PUSHSTRUCT psSnippetStart =
                utils->GetLeadingState(wPaste, delLen);

      // delete leading state from string we want to paste. we will prepend
      // a resolved state later...
      // (handle special case of a line-terminator in the leading-codes!)
      int crlfCount = CountCRs(wPaste.SubString(1, delLen));
      wPaste = utils->DeleteW(wPaste, 1, delLen);

      // resolve leading state... (this was worked out on paper with a
      // logic truth-table). Turned out to be an XOR and !XOR of
      // psInsertPoint and psSnippetStart depending on psInsertPointCodes.
      //
      // psInsertPointCodes | psInsertPoint | psSnippetStart || output
      // -------------------------------------------------------------
      //         0                 0                0             0
      //         0                 0                1             1
      //         0                 1                0             1
      //         0                 1                1             0
      // -------------------------------------------------------------
      //         1                 0                0             1
      //         1                 0                1             0
      //         1                 1                0             0
      //         1                 1                1             1
      PUSHSTRUCT  psSnippetStartNew;
      psSnippetStartNew.Clear();
      psSnippetStartNew.bBold =
                    (psInsertPoint.bBold ^ psSnippetStart.bBold);
      if (psInsertPointCodes.bBold)
        psSnippetStartNew.bBold = !psSnippetStartNew.bBold;
      psSnippetStartNew.bUnderline =
                    (psInsertPoint.bUnderline ^ psSnippetStart.bUnderline);
      if (psInsertPointCodes.bUnderline)
        psSnippetStartNew.bUnderline = !psSnippetStartNew.bUnderline;
      psSnippetStartNew.bItalics =
                    (psInsertPoint.bItalics ^ psSnippetStart.bItalics);
      if (psInsertPointCodes.bItalics)
        psSnippetStartNew.bItalics = !psSnippetStartNew.bItalics;
      if (psSnippetStart.fg != NO_COLOR &&
                                    psSnippetStart.fg != IRCNOCOLOR)
        psSnippetStartNew.fg = psSnippetStart.fg;
      else
        psSnippetStartNew.fg = psInsertPoint.fg;
      if (psSnippetStart.bg != NO_COLOR &&
                                    psSnippetStart.bg != IRCNOCOLOR)
        psSnippetStartNew.bg = psSnippetStart.bg;
      else
        psSnippetStartNew.bg = psInsertPoint.bg;
      if (psSnippetStart.fontSize != -1)
        psSnippetStartNew.fontSize = psSnippetStart.fontSize;
      else
        psSnippetStartNew.fontSize = psInsertPoint.fontSize;
      if (psSnippetStart.fontType != -1)
        psSnippetStartNew.fontType = psSnippetStart.fontType;
      else
        psSnippetStartNew.fontType = psInsertPoint.fontType;

      // resolve trailing state... turns out to be an XOR :-)
      //
      // psInsertPoint | psSnippetEnd || output
      // --------------------------------------
      //       0              0            0
      //       0              1            1
      //       1              0            1
      //       1              1            0

      // update just the style flags then add to end of paste-string
      psInsertPoint.bBold ^= psSnippetEnd.bBold;
      psInsertPoint.bUnderline ^= psSnippetEnd.bUnderline;
      psInsertPoint.bItalics ^= psSnippetEnd.bItalics;

      // add the leading and trailing states to our paste-string
//      psSnippetStartNew.bPush = true;
//      psInsertPoint.bPop = true;
      wPaste = PrintStateString(psSnippetStartNew, true) + wPaste +
                            PrintStateString(psInsertPoint, false);

      // Restore leading line-terminators...
      for (int ii = 0; ii < crlfCount; ii++)
        wPaste = InsertW(wPaste, CRLF, 1);
      
      // Return wInOut via reference
      wInOut = InsertW(wInOut, wPaste, idxCodes+1);
      
      // Insert new paste-text
      return idxCodes;
    }
    else
      return -1;
  }
  catch(...)
  {
    return -1;
  }
}
//---------------------------------------------------------------------------
/*
bool __fastcall TUtils::ResolveStateForPaste(TTaeRichEdit* re,
                                  WideString DestStr, WideString &PasteStr)
// re is the rich edit control we are pasting into
// DestStr is the codes string we are pasting into
// PasteStr is the string we are pasting into the document
{
  try
  {
    int DestIdx = re->SelStart-GetLine(re);

    // Save count of any trailing line-feeds so we can restore them,
    // but for now, strip them so we can write trailing format codes...
    int CrLfCount = CountTrailingCRLFs(PasteStr);

    if (CrLfCount)
      PasteStr = StripTrailingCRLFs(PasteStr);

    // Turn bold/underline/italics off or on as logic requires
    PUSHSTRUCT SnippetBeginState;
    SetStateFlags(PasteStr, STATE_MODE_FIRSTCHAR, SnippetBeginState);
    PUSHSTRUCT SnippetEndState;
    SetStateFlags(PasteStr, STATE_MODE_ENDOFBUF, SnippetEndState);
    PUSHSTRUCT InsertPointState;
    SetStateFlags(DestStr, DestIdx, InsertPointState);
    
    // Use the insert-point colors if colors are missing in the string
    // we are pasting AND this is the beginning of a new line.
    TPoint p = re->CaretPos;

    if (p.x == 0)
    {
      WideString C;

      if (SnippetBeginState.bg == NO_COLOR && InsertPointState.bg != NO_COLOR)
      {
        WriteSingle(InsertPointState.bg, C, false);
        PasteStr = InsertW(PasteStr, C, 1);
      }

      if (SnippetBeginState.fg == NO_COLOR && InsertPointState.fg != NO_COLOR)
      {
        WriteSingle(InsertPointState.fg, C, true);
        PasteStr = InsertW(PasteStr, C, 1);
      }
    }

    // Terminate effects
    TerminateEffects(SnippetEndState, PasteStr);

    // Put back trailing line-feeds
    if (CrLfCount)
      while (CrLfCount--)
        PasteStr += "\r\n";

    // Restore colors that existed prior to paste...
    WriteColors(InsertPointState.fg, InsertPointState.bg, PasteStr);

    // Restore font type/size that existed prior to paste...
    if (InsertPointState.fontType != -1)
      PasteStr += FontTypeToString(InsertPointState.fontType);

    if (InsertPointState.fontSize != -1)
      PasteStr += FontSizeToString(InsertPointState.fontSize);

    return true;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException: ResolveStateForPaste()\r\n");
#endif
    return false;
  }
}
*/
//---------------------------------------------------------------------------
bool __fastcall TUtils::TerminateEffects(PUSHSTRUCT EndState, WideString &Str)
{
  try
  {
    // Terminate effects
    if (EndState.bBold)
      Str += WideString(CTRL_B);
    if (EndState.bUnderline)
      Str += WideString(CTRL_U);
    if (EndState.bItalics)
      Str += WideString(CTRL_R);

    return true;
  }
  catch(...) {return false;}
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::ResolveState(PUSHSTRUCT TrailingState,
                                                            WideString &TempStr)
{
  try
  {
    // NOTE: We pre-initialize TempStr with the LeadingState's colors and
    // font size/type. Therefore, they get reflected into EffectTrailingState...
    // But bold/underline/italic are a real pain because they have no "on"/"off"
    // unique codes (thanks to old mIRC chat-codes).
    PUSHSTRUCT EffectTrailingState;
    SetStateFlags(TempStr, STATE_MODE_ENDOFBUF, EffectTrailingState);

    // Write the font type or size that existed prior to adding the effect
    if (EffectTrailingState.fontType >= 0 &&
                      TrailingState.fontType != EffectTrailingState.fontType)
      TempStr += FontTypeToString(TrailingState.fontType);
    if (EffectTrailingState.fontSize >= 0 &&
                      TrailingState.fontSize != EffectTrailingState.fontSize)
      TempStr += FontSizeToString(TrailingState.fontSize);

    // Write just the color we need if it is different at the end of the new
    // effect than it was prior to adding the effect
    if (EffectTrailingState.fg != -(NO_COLOR) &&
                                    EffectTrailingState.bg != -(NO_COLOR) &&
          TrailingState.fg != EffectTrailingState.fg &&
                                  TrailingState.bg != EffectTrailingState.bg)
      WriteColors(TrailingState.fg, TrailingState.bg, TempStr);
    else if (EffectTrailingState.fg != -(NO_COLOR) &&
                                  TrailingState.fg != EffectTrailingState.fg)
      WriteSingle(TrailingState.fg, TempStr, true);
    else if (EffectTrailingState.bg != -(NO_COLOR) &&
                                  TrailingState.bg != EffectTrailingState.bg)
      WriteSingle(TrailingState.bg, TempStr, false);

    //ShowHex(TempStr);

    return true;
  }
  catch(...) {return false;}
}
//---------------------------------------------------------------------------
// Overloaded
int __fastcall TUtils::ConvertToRtf(TStringsW* slIn, int msOut,
                 TTaeRichEdit* re, bool bSetBlackWhiteColors, bool bShowStatus)
// This just is to pass a NULL in msOut...
{
  TMemoryStream* dummyRefP = (TMemoryStream*)msOut;
  return ConvertToRtf(slIn->Text, dummyRefP, re,
                                     bSetBlackWhiteColors, bShowStatus);
}

int __fastcall TUtils::ConvertToRtf(WideString S, int msOut,
              TTaeRichEdit* re, bool bSetBlackWhiteColors, bool bShowStatus)
// This just is to pass a NULL in msOut...
{
  TMemoryStream* dummyRefP = (TMemoryStream*)msOut;
  return ConvertToRtf(S, dummyRefP, re,
                                    bSetBlackWhiteColors, bShowStatus);
}

int __fastcall TUtils::ConvertToRtf(TStringsW* slIn, TMemoryStream* &msOut,
                 TTaeRichEdit* re, bool bSetBlackWhiteColors, bool bShowStatus)
// Set lockCnt to -1 to skip push/pop of OnChange handler
// Set msOut NULL to use temporary memory stream for output
{
  return ConvertToRtf(slIn->Text, msOut, re, bSetBlackWhiteColors, bShowStatus);
}

int __fastcall TUtils::ConvertToRtf(WideString S, TMemoryStream* &msOut,
                TTaeRichEdit* re, bool bSetBlackWhiteColors, bool bShowStatus)
// Set lockCnt to -1 to skip push/pop of OnChange handler
// Set msOut NULL to use temporary memory stream for output
{
  if (!re)
    return 100;

  int retVal;

  try
  {
    TConvertToRTF* rtf = new TConvertToRTF(dts, re, bShowStatus);

    if (!rtf)
      return 101;

    if (bSetBlackWhiteColors)
    {
      rtf->DefaultFgColor = IRCBLACK;
      rtf->DefaultBgColor = IRCWHITE;
    }

    S = Optimize(S, false, NO_COLOR);

    // Sets retVal 0 if success, 1 if user-abort
    TMemoryStream* ms = rtf->Execute(S, retVal);

    if (ms && retVal == 0)
    {
      PushOnChange(re);

      re->Clear();

      SetDefaultFont(re);

      re->PlainText = false;
      ms->Position = 0; // just to be sure!
      re->Lines->LoadFromStream(ms);

      // Delete old output stream...
      if (msOut)
      {
        try
        {
          delete msOut;
        }
        catch(...)
        {
#if DEBUG_ON
          dts->CWrite("\r\nUnable to delete old RTF stream "
                                  "in TUtils::ConvertToRtf() A\r\n");
#endif
        }

        msOut = ms;
      }
      else
        delete ms; // delete temp stream

      PopOnChange(re);
    }
    else
    {
      if (ms)
        delete ms; // clean up if error returned in retVal

#if DEBUG_ON
      dts->CWrite("\r\nError in TUtils::ConvertToRtf() A: " +
                                              String(retVal) + "\r\n");
#endif
    }

    delete rtf;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nException in TUtils::ConvertToRtf() A\r\n");
#endif
    retVal = 100;
  }

  return retVal;
}

int __fastcall TUtils::ConvertToRtf(TStringsW* slIn, TMemoryStream* &msOut)
// Puts converted RTF in msOut but loads no edit-controls
{
  return ConvertToRtf(slIn->Text, msOut);
}

int __fastcall TUtils::ConvertToRtf(WideString S, TMemoryStream* &msOut)
// Puts converted RTF in msOut but loads no edit-controls
{
  int retVal;

  TConvertToRTF* rtf = NULL;

  try
  {
    TConvertToRTF* rtf = new TConvertToRTF(dts, tae, false);

    try
    {
      if (!rtf)
        retVal = 100;
      else
      {
        S = Optimize(S, false, NO_COLOR);

        // Sets retVal 0 if success, 1 if user-abort
        TMemoryStream* ms = rtf->Execute(S, retVal);

        if (ms && retVal == 0)
        {
          // Delete old output stream...
          if (msOut)
          {
            try
            {
              delete msOut;
            }
            catch(...)
            {
#if DEBUG_ON
              dts->CWrite("\r\nUnable to delete old RTF stream "
                                      "in TUtils::ConvertToRtf() B\r\n");
#endif
            }
          }

          msOut = ms;
        }
        else
        {
          if (ms)
            delete ms; // clean up if error returned in retVal

#if DEBUG_ON
          dts->CWrite("\r\nError in TUtils::ConvertToRtf() B: " +
                                                  String(retVal) + "\r\n");
#endif
        }
      }
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nException in TUtils::ConvertToRtf() B\r\n");
#endif
      retVal = 100;
    }
  }
  __finally
  {
    if (rtf)
      delete rtf;
  }

  return retVal;
}

String __fastcall TUtils::ConvertToRtf(WideString S, int maxColors, int fg,
                              int bg, bool bStripFontType, bool bShowStatus)
// Takes an IRC UTF-16 string in and returns an RTF ANSI string.
//
// Set maxColors to -1 if no limit... (-1 is the default)
// Set fg/bg to NO_COLOR to prevent it color from being added to the
// color-table.
//
// bStripFontType defaults false - set true for PalTalk
{
  int retVal;
  String Out;
  TConvertToRTF* rtf = NULL;
  TMemoryStream* ms = NULL;

  try
  {
    try
    {
      rtf = new TConvertToRTF(dts, tae, bShowStatus);

      if (rtf != NULL)
      {
        rtf->MaxColors = maxColors;
        rtf->DefaultFgColor = fg;
        rtf->DefaultBgColor = bg;
        rtf->StripFontType = bStripFontType;

        S = Optimize(S, false, NO_COLOR);

        // Sets retVal 0 if success, 1 if user-abort
        ms = rtf->Execute(S, retVal);

        if (ms && retVal == 0)
          Out = StreamToStringA(ms);
        else
        {
          // (ms is cleaned up in the __finally!)

#if DEBUG_ON
          dts->CWrite("\r\nError in TUtils::ConvertToRtf() C: " +
                                              String(retVal) + "\r\n");
#endif
        }
      }
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nError in TUtils::ConvertToRtf() C\r\n");
#endif
    }
  }
  __finally
  {
    if (rtf)
      delete rtf;

    if (ms)
      delete ms;
  }

  return Out;
}
//---------------------------------------------------------------------------
//WideString __fastcall TUtils::OneLineIRCtoHtml(WideString S)
//{
//  try
//  {
//    TConvertToHTML* ProcessHtml = new TConvertToHTML();
//
//    WideString Out;
//
//    if (ProcessHtml)
//      Out = ProcessHtml->ConvertLineToHTML(S);
//
//    return Out;
//  }
//  catch(...)
//  {
//    return "";
//  }
//}
//---------------------------------------------------------------------------
// End Cut/Copy
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Start Memory Stream
//---------------------------------------------------------------------------
AnsiString __fastcall TUtils::StreamToStringA(TMemoryStream* ms)
{
  String sOut;
  char* pBuf = NULL;

  try
  {
    if ((pBuf = StreamToBufferA(ms)) != NULL)
      sOut = String(pBuf);
#if DEBUG_ON
    else
      dts->CWrite("\r\nStreamToBufferA returning NULL in StreamToStringA()\r\n");
#endif
  }
  __finally
  {
    try
    {
      if (pBuf != NULL)
        delete [] pBuf;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: StreamToStringA()\r\n");
#endif
    }
  }

  return sOut;
}

WideString __fastcall TUtils::StreamToStringW(TMemoryStream* ms)
{
  WideString sOut;
  wchar_t* pBuf = NULL;

  try
  {
    if ((pBuf = StreamToBufferW(ms)) != NULL)
      sOut = WideString(pBuf);
#if DEBUG_ON
    else
      dts->CWrite("\r\nStreamToBufferW returning NULL in StreamToStringW()\r\n");
#endif
  }
  __finally
  {
    try
    {
      if (pBuf != NULL)
        delete [] pBuf;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: StreamToStringA()\r\n");
#endif
    }
  }

  return sOut;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::StringToStream(WideString S, TMemoryStream* &ms)
// Writes WideString S into memory stream ms (creates ms if ms is null)
// Returns true if success
{
  try
  {
    if (!ms)
      ms = new TMemoryStream();
    else
      ms->Clear();

    ms->WriteBuffer(S.c_bstr(), S.Length());
    ms->Position = 0;
    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
char* __fastcall TUtils::StreamToBufferA(TMemoryStream* ms)
// Reads the entire stream into a new buffer, preserving the existing position.
// You must delete the returned buffer!
{
  if (!ms || !ms->Size)
    return NULL;

  try
  {
    int byteLen = ms->Size;
    int maxChars = (byteLen/sizeof(char))+2; // plenty of room!
    char* buf = new char[maxChars];
    // Add nulls before we read...
    buf[0] = C_NULL;
    buf[maxChars-1] = C_NULL;
    buf[maxChars-2] = C_NULL;
    int savePos = ms->Position;
    ms->Position = 0;
    ms->ReadBuffer(buf, byteLen);
    ms->Position = savePos;

    return buf;
  }
  catch(...)
  {
    return NULL;
  }
}

wchar_t* __fastcall TUtils::StreamToBufferW(TMemoryStream* ms)
// Must delete the returned buffer!
{
  if (!ms || !ms->Size)
    return NULL;

  try
  {
    int byteLen = ms->Size;
    int maxChars = (byteLen/sizeof(wchar_t))+2; // plenty of room!
    wchar_t* buf = new wchar_t[maxChars];
    // Add nulls before we read...
    buf[0] = C_NULL;
    buf[maxChars-1] = C_NULL;
    buf[maxChars-2] = C_NULL;
    ms->Position = 0;
    ms->ReadBuffer(buf, byteLen);

    return buf;
  }
  catch(...)
  {
    return NULL;
  }
}
//---------------------------------------------------------------------------
// End Memory Stream
//---------------------------------------------------------------------------
void __fastcall TUtils::LoadMenu(TMenu* M, char** array)
// Menu items in DefStrings.cpp are in UTF-8; however, here we convert them
// to UTF-16, strip out IRC-codes then convert to ANSI (the old borland vcl has ANSI menu Captions!)
{
  // Main Menu
  try
  {
    int index = 0;

    WideString sTemp;

    if (M->Items->Count > 0)
    {
      for (int ii = 0; ii < M->Items->Count; ii++)
      {
        sTemp = Utf8ToWide(array[index++]);

        M->Items->Items[ii]->Caption = String(StripAllCodes(sTemp));

        if (M->Items->Items[ii]->Count > 0)
        {
          for (int jj = 0; jj < M->Items->Items[ii]->Count; jj++)
          {
            sTemp = Utf8ToWide(array[index++]);

            M->Items->Items[ii]->Items[jj]->Caption = String(StripAllCodes(sTemp));

            if (M->Items->Items[ii]->Items[jj]->Count > 0)
            {
              for (int kk = 0; kk < M->Items->Items[ii]->Items[jj]->Count; kk++)
              {
                sTemp = Utf8ToWide(array[index++]);

                M->Items->Items[ii]->Items[jj]->Items[kk]->Caption = String(StripAllCodes(sTemp));
              }
            }
          }
        }
      }
    }
  }
  catch(...)
  {
  }
}
//---------------------------------------------------------------------------
void __fastcall TUtils::RelocateDialog(TForm* f)
{
  try
  {
    // Relocate this dialog if it's on top of the new selection!
    if (f->Visible && SelectedIsHidden(f))
    {
      // Try three times to avoid being on-top of the selected text
      for (int ii=0; ii<3; ii++)
      {
        f->Left = random(Screen->Width - f->Width-1);
        f->Top = random(Screen->Height - f->Height-1);
        if (!SelectedIsHidden(f)) break;
      }
    }
  }
  catch(...) {}
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::SelectedIsHidden(TForm* f)
// Private function
// Returns true if OurLocation (usually a dialog box) is covering
// selected text
{
  TPoint OurLocation = f->ClientOrigin;
  TPoint SelCoordUL = tae->GetCharCoordinates(tae->SelStart);
  TPoint SelCoordBR = tae->GetCharCoordinates(tae->SelStart+tae->SelLength);
  TPoint DlgCoord = OurLocation;
  SelCoordUL.x += dts->Left + tae->Left + 10;
  SelCoordUL.y += dts->Top + tae->Top + 100;
  SelCoordBR.x += dts->Left + tae->Left + 10;
  SelCoordBR.y += dts->Top + tae->Top + 100;

  int Right = DlgCoord.x + f->Width;
  int Bottom = DlgCoord.y + f->Height;
  if (SelCoordUL.y >= DlgCoord.y && SelCoordBR.y <= Bottom &&
              SelCoordBR.x >= DlgCoord.x && SelCoordUL.x <= Right)
    return true;
  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::SetCurrentDirW(WideString wDir)
// Set the current directory from a WideString
{
  return SetCurrentDirectoryW(wDir.c_bstr());
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::DirectoryExistsW(WideString wPath)
{
  // mapiwin.h and winnt.h
  // FILE_ATTRIBUTE_READONLY         0x00000001
  // FILE_ATTRIBUTE_HIDDEN           0x00000002
  // FILE_ATTRIBUTE_SYSTEM           0x00000004
  // FILE_ATTRIBUTE_DIRECTORY        0x00000010
  // FILE_ATTRIBUTE_ARCHIVE          0x00000020
  // FILE_ATTRIBUTE_NORMAL           0x00000080
  // FILE_ATTRIBUTE_TEMPORARY        0x00000100
  DWORD dwAttrib = GetFileAttributesW(wPath.c_bstr());

//#if DEBUG_ON
//    if (dwAttrib == INVALID_FILE_ATTRIBUTES)
//      MainForm->CWrite( "\r\nDirectoryExistsW No Directory: " + String(sPath) + "\r\n");
//    else
//      MainForm->CWrite( "\r\nDirectoryExistsW Attributes: " + IntToHex((int)dwAttrib, 8) + "\r\n");
//#endif

  return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
         (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::FileExistsW(WideString wPath)
{
  // mapiwin.h and winnt.h
  // FILE_ATTRIBUTE_READONLY         0x00000001
  // FILE_ATTRIBUTE_HIDDEN           0x00000002
  // FILE_ATTRIBUTE_SYSTEM           0x00000004
  // FILE_ATTRIBUTE_DIRECTORY        0x00000010
  // FILE_ATTRIBUTE_ARCHIVE          0x00000020
  // FILE_ATTRIBUTE_NORMAL           0x00000080
  // FILE_ATTRIBUTE_TEMPORARY        0x00000100
  DWORD dwAttrib = GetFileAttributesW(wPath.c_bstr());

//#if DEBUG_ON
//    if (dwAttrib == INVALID_FILE_ATTRIBUTES)
//      MainForm->CWrite( "\r\nFileExistsW No Directory: " + String(sPath) + "\r\n");
//    else
//      MainForm->CWrite( "\r\nFileExistsW Attributes: " + IntToHex((int)dwAttrib, 8) + "\r\n");
//#endif

  return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
         !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::ExtractFileExtW(WideString sIn)
// Return ".txt" including the period, or empty-string...
{
  int len = sIn.Length();
  int idx = len;
  wchar_t c;

  for(;;)
  {
    if (idx == 0)
      break;

    c = sIn[idx];

    if (c == '.')
      break;

    idx--;
  }

  if (idx != 0)
    return sIn.SubString(idx, len-idx+1);

  return "";
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::ExtractFileNameW(WideString sIn)
{
  int len = sIn.Length();
  int idx = len;
  wchar_t c;

  for(;;)
  {
    if (idx == 0)
      break;

    c = sIn[idx];

    if (c == '\\' || c == '/')
      break;

    idx--;
  }

  if (idx != 0)
    return sIn.SubString(idx+1, len-(idx+1)+1);

  return sIn;
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::ExtractFilePathW(WideString sIn)
{
  WideString sOut = ExtractFileNameW(sIn);

  int posFileName = sIn.Pos(sOut);

  if (posFileName >= 0)
    sOut = sIn.SubString(1, posFileName-1);
  else
    sOut = sIn;

  return sOut;
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::WriteStringToFileW(WideString wPath, String sInfo)
// Writes sInfo (ANSI or UTF-8) to a UTF-16 path
{
  bool bRet = false;
  FILE* f = NULL;

  try
  {
    // open/create file for writing in binary-mode
    f = _wfopen(wPath.c_bstr(), L"wb");

    unsigned length = sInfo.Length();

    if (f != NULL)
      if (fwrite(sInfo.c_str(), sizeof(char), length, f) == length)
        bRet = true;
  }
  __finally
  {
    try { if (f != NULL) fclose(f); } catch(...) {}
  }

  return bRet;
}
//---------------------------------------------------------------------------
// Reads a file into a String (can be UTF-8 or ANSI)
String __fastcall TUtils::ReadStringFromFileW(WideString wPath)
{
  FILE* f = NULL;
  char* pBuf = NULL;

  String sOut;

  try
  {
    // open/create file for reading in binary-mode
    f = _wfopen(wPath.c_bstr(), L"rb");

    if (f != NULL)
    {
      // get file-length
      fseek(f, 0L, SEEK_END);
      int length = ftell(f);
      rewind(f);

      pBuf = new char[length];
      int n = fread(pBuf, 1, length, f);

      if (n == length)
        sOut = String(pBuf, length);
    }
  }
  __finally
  {
    try
    {
      if (f != NULL)
        fclose(f);
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to close file: ReadStringFromFileW()\r\n");
#endif
    }

    try
    {
      if (pBuf != NULL)
        delete [] pBuf;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: ReadStringFromFileW()\r\n");
#endif
    }
  }

  return sOut;
}
//----------------------------------------------------------------------------
// Displays an OpenFile DIALOG and returns the name selected
//----------------------------------------------------------------------------
WideString __fastcall TUtils::GetSaveFileName(WideString &wFilters,
                   WideString wDefFile, WideString wDir, String uTitle)
{
  if (SFDlgForm)
    try { delete SFDlgForm; } catch(...) {}

  WideString wRetFile;

  try
  {
    try
    {
      Application->CreateForm(__classid(TSFDlgForm), &SFDlgForm);

      if (SFDlgForm)
      {
        SFDlgForm->Filters = wFilters;

        if (SFDlgForm->ExecuteW(wDefFile, wDir, uTitle))
          wRetFile = SFDlgForm->FileName; // Get filepath

        // Set return reference
        wFilters = SFDlgForm->CurrentFilter;
      }
    }
    catch(...)
    {
      // Unfortunately - when we asynchronously abort OFSSDlgForm via sending
      // ->Close(), and its OnClose event handler calls EndDialog() to close
      // the system-dialog's thread... we throw an exception - cover it up
      // until we can deduce a better way...
//      utils->ShowMessageU(String(DS[42])); // Can't save file...
    }
  }
  __finally
  {
    if (SFDlgForm)
    {
      try { delete SFDlgForm; } catch(...) {}
      SFDlgForm = NULL;
    }
  }

  return wRetFile;
}
//----------------------------------------------------------------------------
// Displays an OpenFile DIALOG and returns the name selected
//----------------------------------------------------------------------------
WideString __fastcall TUtils::GetOpenFileName(WideString &wFilters, int iFilter,
                                         WideString wDir, String uTitle)
// wTitle should be in UTF-8 format (usually taken from a table in
// DefaultStrings.cpp). Everything else is UTF-16.
//
// Displays an open-file dialog and returns the selected fileName.
// Returns an empty string if the user cancelled.
//
// On input, wFilters is a list of filters:
//     WideString wFilters = WideString("All files (*.*)|*.*|"
//                  "Document files (*.rtf)|*.rtf|"
//                  "Text files (*.txt)|*.txt|"
//                  "Web files (*.htm)|*.htm");
//
// On return, wFilter has a reference to the user's selected filter

{
  if (OFSSDlgForm)
    try { delete OFSSDlgForm; } catch(...) {}

  WideString wRetFile;

  try
  {
    try
    {
      Application->CreateForm(__classid(TOFSSDlgForm), &OFSSDlgForm);

      if (OFSSDlgForm)
      {
        if (wDir.IsEmpty() || !utils->DirectoryExistsW(wDir))
          wDir = dts->DeskDir;

        OFSSDlgForm->Filters = wFilters;

        if (OFSSDlgForm->ExecuteW(iFilter, wDir, uTitle))
          wRetFile = OFSSDlgForm->FileName;

        // Set return reference
        wFilters = OFSSDlgForm->CurrentFilter;
      }
    }
    catch(...)
    {
      // Unfortunately - when we asynchronously abort OFSSDlgForm via sending
      // ->Close(), and its OnClose event handler calls EndDialog() to close
      // the system-dialog's thread... we throw an exception - cover it up
      // until we can deduce a better way...
//      utils->ShowMessageU(String(DS[38])); // Can't load file...
    }
  }
  __finally
  {
    if (OFSSDlgForm)
    {
      try { delete OFSSDlgForm; } catch(...) {}
      OFSSDlgForm = NULL;
    }
  }

  return wRetFile;
}
//---------------------------------------------------------------------------
// Find a file with UTF-16 path and name... this WILL take wildcards!
WideString __fastcall TUtils::FindFileW(WideString wFile, WideString wPath)
{
  WIN32_FIND_DATAW fd;
  FindFirstFileW(wFile.c_bstr(), &fd);
  wFile = WideString(fd.cFileName);
  return wPath + wFile;
}
//---------------------------------------------------------------------------
// Get the full path and file-name of our own EXE file
WideString __fastcall TUtils::GetExeNameW(void)
{
  WideString wFile;
  int len = 2*(MAX_PATH+1);
  wchar_t* buf = new wchar_t[len];
  GetModuleFileNameW(NULL, buf, len);
  wFile = WideString(buf);
  delete [] buf;
  return wFile;
}
//---------------------------------------------------------------------------
// Get the ANSI substitute path for a UTF-16 file-path
// GetShortPathNameW returns Latin chars only... (tricky!)
String __fastcall TUtils::GetAnsiPathW(WideString wPath)
{
  // add a Microsoft trick preamble-code to go past MAXPATH wide-chars
  wPath = WideString("\\\\?\\") + wPath;

  int len = 2*(MAXPATH+1);
  wchar_t* buf = new wchar_t[len];

  GetShortPathNameW(wPath.c_bstr(), buf, len);

  String sPath = String(WideString(buf)); // down-convert to an ANSI string

  delete [] buf;

  return sPath;
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::GetTempFileW(WideString wFile)
{
  wchar_t* p;
  WideString wTemp;

  p = _wgetenv(L"TMP");

  if (!p)
    p = _wgetenv(L"TEMP");

  if (p)
    wTemp = WideString(p) + "\\" + wFile; // Try to put it in temp directory
  else
    wTemp = ExtractFilePathW(GetExeNameW()) + wFile;

  return wTemp;
}
//---------------------------------------------------------------------------
long __fastcall TUtils::filesize(FILE *fs)
{
  rewind(fs);
  fseek(fs, 0L, SEEK_END);
  long length = ftell(fs);
  rewind(fs);
  return length;
}
//---------------------------------------------------------------------------
// Display Utf8 messages
int __fastcall TUtils::ShowMessageU(String sUtf8, int flags)
{
  return ShowMessageU(NULL, sUtf8, flags);
}

int __fastcall TUtils::ShowMessageU(HWND hwnd, String sUtf8, int flags)
{
  return ShowMessageW(hwnd, Utf8ToWide(sUtf8), flags);
}
//---------------------------------------------------------------------------
// Display Utf8 messages
int __fastcall TUtils::ShowMessageW(WideString wIn, int flags)
{
  return ShowMessageW(NULL, wIn, flags);
}

int __fastcall TUtils::ShowMessageW(HWND hwnd, WideString wIn, int flags)
{
  if (flags == 0)
    flags = MB_ICONINFORMATION|MB_OK;

  flags |= MB_TASKMODAL|MB_TOPMOST|MB_SETFOREGROUND;

  return MessageBoxW(hwnd, wIn.c_bstr(),
                     Utf8ToWide(OUR_NAME).c_bstr(), flags);
}
//---------------------------------------------------------------------------
// Property getter to return UTF-8 version of the desktop-directory
String __fastcall TUtils::GetDeskDir(void)
{
  return WideToUtf8(dts->DeskDir);
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::LowerCaseW(WideString s)
{
  WideChar c;

  int len = s.Length();

  for (int ii = 1; ii <= len; ii++)
  {
    c = s[ii];

    if (iswupper(c))
      s[ii] = towlower(c);
  }
  return WideString(s);
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::UpperCaseW(WideString s)
{
  WideChar c;

  int len = s.Length();

  for (int ii = 1; ii <= len; ii++)
  {
    c = s[ii];

    if (iswlower(c))
      s[ii] = towupper(c);
  }
  return WideString(s);
}
//---------------------------------------------------------------------------
// replaced on 5/8/15
WideString __fastcall TUtils::GetSpecialFolder(int csidl)
// Constants to pass in...
// Look in Windows SDK files ShlObj.h ShFolder.h and Windows.h
//  
//  CSIDL_DESKTOP
//  CSIDL_INTERNET
//  CSIDL_PROGRAMS
//  CSIDL_CONTROLS
//  CSIDL_PRINTERS
//  CSIDL_PERSONAL
//  CSIDL_FAVORITES
//  CSIDL_STARTUP
//  CSIDL_RECENT
//  CSIDL_SENDTO
//  CSIDL_BITBUCKET
//  CSIDL_STARTMENU
//  CSIDL_DESKTOPDIRECTORY
//  CSIDL_DRIVES
//  CSIDL_NETWORK
//  CSIDL_NETHOOD
//  CSIDL_FONTS
//  CSIDL_TEMPLATES
//  CSIDL_COMMON_STARTMENU - yes
//  CSIDL_COMMON_PROGRAMS
//  CSIDL_COMMON_STARTUP
//  CSIDL_COMMON_DESKTOPDIRECTORY - yes
//  CSIDL_COMMON_APPDATA
//  CSIDL_APPDATA
//  CSIDL_PRINTHOOD
//  CSIDL_ALTSTARTUP
//  CSIDL_COMMON_ALTSTARTUP
//  CSIDL_COMMON_FAVORITES
//  CSIDL_INTERNET_CACHE
//  CSIDL_COOKIES
//  CSIDL_HISTORY
//  CSIDL_PROGRAM_FILES
//  CSIDL_PROGRAM_FILES_COMMON
//
// Added in Main.h:
//  CSIDL_WINDOWS
//  CSIDL_SYSTEM
//  CSIDL_PROGRAM_FILES
//  CSIDL_PROGRAM_FILES_COMMON
//  CSIDL_PROGRAM_FILESX86
//  CSIDL_PROGRAM_FILES_COMMONX86
{
  HMODULE h = NULL;
  WideString sOut;
  wchar_t* pBuf = NULL;

  try
  {
    h = LoadLibraryW(L"Shell32.dll");

    if (h != NULL)
    {
      tGetFolderPath pGetFolderPath;
      pGetFolderPath = (tGetFolderPath)GetProcAddress(h, "SHGetFolderPathW");

      if (pGetFolderPath != NULL)
      {
        pBuf = new WideChar[MAX_PATH];
        pBuf[0] = C_NULL;

        if ((*pGetFolderPath)(Application->Handle, csidl, NULL, SHGFP_TYPE_CURRENT, (LPTSTR)pBuf) == S_OK)
          sOut = WideString(pBuf);
      }
    }

  }
  __finally
  {
    if (h != NULL)
      FreeLibrary(h);

    try
    {
      if (pBuf != NULL)
        delete [] pBuf;
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nUnable to deallocate pBuf in: GetSpecialFolder()\r\n");
#endif
    }
  }

  return sOut;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::ToIntDefW(WideString s, int def)
{
  return String(s).ToIntDef(def);
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::GetTabStringW(int count)
{
  return WideString(GetTabString(count));
}
//---------------------------------------------------------------------------
String __fastcall TUtils::GetTabString(int count)
{
  String sTemp;
  
  if (dts->TabsToSpaces->Checked)
  {
    if (count <= 0)
      count = TABSINIT;

    sTemp = StringOfCharW(C_SPACE, count);
  }
  else
    sTemp = C_TAB;

  return sTemp;
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::StringOfCharW(WideChar c, int count)
{
  WideString sOut;

  for (int ii = 0; ii < count; ii++)
    sOut += c;

  return sOut;
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::TrimW(WideString s)
{
  WideString sOut;

  int len = s.Length();

  int iStart = 1;

  for(; iStart <= len; iStart++)
    if (s[iStart] != L' ')
      break;

  // String all spaces? return empty
  if (iStart > len)
    return "";

  int iEnd = len;

  for(; iEnd > iStart; iEnd--)
    if (s[iEnd] != L' ')
      break;

  return s.SubString(iStart, iEnd-iStart+1);
}
//---------------------------------------------------------------------------
// UTF8 Encode/Decode
//---------------------------------------------------------------------------
String __fastcall TUtils::AnsiToUtf8(String sIn)
// Code to convert ANSI to UTF-8 (Works!)
{
  if (sIn.IsEmpty())
    return "";

  // Use the MultiByteToWideChar function to determine the size of the UTF-16 representation of the string. You use
  // this size to allocate a new buffer that can hold the UTF-16 version of the string.
  DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, sIn.c_str(), -1, NULL, 0);
  wchar_t *pwText = new wchar_t[dwNum];

  // The MultiByteToWideChar function takes the ASCII string and converts it into UTF-16, storing it in pwText.
  MultiByteToWideChar(CP_ACP, 0, sIn.c_str(), -1, pwText, dwNum);

  // The WideCharToMultiByte function tells you the size of the returned string so you can create a buffer for the UTF-8 representation.
  dwNum = WideCharToMultiByte(CP_UTF8, 0, pwText, -1, NULL, 0, NULL, NULL);
  char *psText = new char[dwNum];

  // Convert the UTF-16 string into UTF-8, storing the result into psText.
  WideCharToMultiByte(CP_UTF8, 0, pwText, -1, psText, dwNum, NULL, NULL);

  delete [] pwText;

  // Convert to VCL String.
  if (dwNum > 1)
    sIn = String(psText, dwNum-1);
  else
    sIn = "";

  delete [] psText;

  return sIn;
}
//---------------------------------------------------------------------------
String __fastcall TUtils::Utf8ToAnsi(String sIn)
// Code to convert UTF-8 to ASCII
{
  // Use the MultiByteToWideChar function to determine the size of the UTF-16 representation of the string. You use
  // this size to allocate a new buffer that can hold the UTF-16 version of the string.
  DWORD dwNum = MultiByteToWideChar(CP_UTF8, 0, sIn.c_str(), -1, NULL, 0);
  wchar_t *pwText = new wchar_t[dwNum];

  // The MultiByteToWideChar function takes the UTF-8 string and converts it into UTF-16, storing it in pwText.
  MultiByteToWideChar(CP_UTF8, 0, sIn.c_str(), -1, pwText, dwNum);

  // The WideCharToMultiByte function tells you the size of the returned string so you can create a buffer for the ANSI representation.
  dwNum = WideCharToMultiByte(CP_ACP, 0, pwText, -1, NULL, 0, NULL, NULL);
  char *psText = new char[dwNum];

  // Convert the UTF-16 string into ANSI, storing the result into psText.
  WideCharToMultiByte(CP_ACP, 0, pwText, -1, psText, dwNum, NULL, NULL);

  delete [] pwText;

  // Convert to VCL String.
  if (dwNum > 1)
    sIn = String(psText, dwNum-1);
  else
    sIn = "";

  delete [] psText;

  return sIn;
}
//---------------------------------------------------------------------------
// Overloaded...

WideString __fastcall TUtils::Utf8ToWide(char* buf)
// Code to convert UTF-8 to UTF-16 (wchar_t)
{
  return Utf8ToWide(String(buf, strlen(buf)));
}

WideString __fastcall TUtils::Utf8ToWide(char* buf, int len)
// Code to convert UTF-8 to UTF-16 (wchar_t)
{
  return Utf8ToWide(String(buf, len));
}

WideString __fastcall TUtils::Utf8ToWide(String sIn)
// Code to convert UTF-8 to UTF-16 (wchar_t)
{
  WideString sWide;

  int lenUtf8 = sIn.Length();

  if (lenUtf8 > 0)
  {
    wchar_t* pwText = NULL;

    try
    {
      // Use the MultiByteToWideChar function to determine the size of the UTF-16 representation of the string. You use
      // this size to allocate a new buffer that can hold the UTF-16 version of the string.
      DWORD lenUtf16 = MultiByteToWideChar(CP_UTF8, 0, sIn.c_str(), lenUtf8, NULL, 0);

      if (lenUtf16 > 0)
      {
        pwText = new wchar_t[lenUtf16];

        if (pwText != NULL)
        {
          // The MultiByteToWideChar function takes the UTF-8 string and converts it into UTF-16, storing it in pwText.
          MultiByteToWideChar(CP_UTF8, 0, sIn.c_str(), lenUtf8, pwText, lenUtf16);

          // Convert to VCL WideString.
          sWide = WideString(pwText, lenUtf16);
        }
      }
    }
    // NOTE: Unlike C#, __finally does NOT get called if we execute a return!!!
    __finally
    {
      try { if (pwText != NULL) delete [] pwText; } catch(...) {}
    }
  }

  return sWide;
}
//---------------------------------------------------------------------------
String __fastcall TUtils::WideToUtf8(WideString sIn)
// Code to convert UTF-16 (WideString or WideChar wchar_t) to UTF-8
{
  String sOut;

  int lenUtf16 = sIn.Length();

  if (lenUtf16 > 0)
  {
    char* pText = NULL;

    try
    {
      // Use the MultiByteToWideChar function to determine the size of the UTF-16 representation of the string. You use
      // this size to allocate a new buffer that can hold the UTF-16 version of the string.
      DWORD lenUtf8 = WideCharToMultiByte(CP_UTF8, 0, sIn.c_bstr(), lenUtf16, NULL, 0, NULL, NULL);

      if (lenUtf8 > 0)
      {
        pText = new char[lenUtf8];

        if (pText != NULL)
        {
          // The MultiByteToWideChar function takes the UTF-8 string and converts it into UTF-16, storing it in pwText.
          WideCharToMultiByte(CP_UTF8, 0, sIn.c_bstr(), lenUtf16, pText, lenUtf8, NULL, NULL);

          // Convert to VCL String.
          sOut = String(pText, lenUtf8); // There is no nullchar written
        }
      }
    }
    __finally
    {
      try { if (pText != NULL) delete [] pText; } catch(...) {}
    }
  }

  return sOut;
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::InsertW(WideString wThat,
                                              WideString wThis, int pos)
// Insert sSource before pos (pos is a vcl 1-based index)
//
// insert wThis into wThat...
{
  int lenThat = wThat.Length();

  if (lenThat == 0)
    return wThis;

  int lenThis = wThis.Length();

  if (lenThis <= 0 || pos <= 0)
    return wThat;

  // add wThat onto wThis if pos is 1
  if (pos == 1)
    return wThis + wThat;

  // add wThis onto wThat if pos is past the end of wThat
  if (pos >= lenThat + 1)
    return wThat + wThis;

  // insert wThis into wThat before pos
  int lenLeading = pos - 1;

  WideString sLeading = wThat.SubString(1, lenLeading);

  WideString sTrailing = wThat.SubString(pos, lenThat - lenLeading);

  return sLeading + wThis + sTrailing;
}
//---------------------------------------------------------------------------
// Overloaded...

// delete charCount chars beginning at startIdx. line-terminators are
// counted as 2 chars here. 1-based startIdx.
void __fastcall TUtils::DeleteW(TStringsW* sl, int startIdx, int numChars)
{
  if (!sl || numChars <= 0 || startIdx <= 0)
    return;

  sl->Text = DeleteW(sl->Text, startIdx, numChars);
}

WideString __fastcall TUtils::DeleteW(WideString wIn,
                                            int startIdx, int numChars)
// Delete numChars from sIn beginning at startIdx
{
  int wInLen = wIn.Length();

  if (wInLen <= 0 || startIdx <= 0 || startIdx > wInLen)
    return wIn;

  // Delete all
  if (startIdx == 1 && numChars >= wInLen)
    return "";

  WideString wLeading;

  if (startIdx-1 > 0)
    wLeading = wIn.SubString(1, startIdx-1);

  int iEnd = startIdx + numChars;

  if (iEnd > wInLen)
    return wLeading;

  // have at least 1 char for wTrailing
  return wLeading + wIn.SubString(iEnd, wInLen-iEnd+1);
}
//---------------------------------------------------------------------------
// Overloaded...

// delete charCount chars beginning at startIdx. line-terminators are
// counted as 2 chars here. 1-based startIdx.
void __fastcall TUtils::ReplaceW(TStringsW* sl, WideString wNew, int startIdx)
{
  if (!sl || startIdx <= 0)
    return;

  sl->Text = ReplaceW(sl->Text, wNew, startIdx);
}

WideString __fastcall TUtils::ReplaceW(WideString wIn, WideString wNew,
                                                              int startIdx)
// Replace numChars from sIn beginning at startIdx
{
  int wInLen = wIn.Length();

  if (wInLen <= 0 || startIdx <= 0 || startIdx > wInLen)
    return wIn;

  int wNewLen = wNew.Length();
  int destIdx;
  for (int ii = 1; ii <= wNewLen; ii++)
  {
    destIdx = startIdx+ii-1;
    if (destIdx <= wInLen)
      wIn[destIdx] = wNew[ii];
  }
  return wIn;
}
//---------------------------------------------------------------------------
wchar_t* __fastcall TUtils::StrNewW(WideString sIn)
// Allocate new memory and copy s into it
{
  wchar_t* buf = new wchar_t[sIn.Length()+1];
  // wcscpy won't copy null if source string is empty!
  buf[0] = C_NULL;

  if (!sIn.IsEmpty())
    wcscpy(buf, sIn.c_bstr());

  return buf;
}
//---------------------------------------------------------------------------
wchar_t* __fastcall TUtils::StrNewW(wchar_t* pIn)
// Allocate new memory and copy s into it
{
  int len = wcslen(pIn);

  wchar_t* buf = new wchar_t[len+1];
  buf[0] = C_NULL;

  if (*pIn != C_NULL)
    wcscpy(buf, pIn);

  return buf;
}
//---------------------------------------------------------------------------
wchar_t* __fastcall TUtils::StrNewW(wchar_t* pIn, int len)
// Allocate new memory and copy s into it, len is the # of wide chars
{
  wchar_t* buf = new wchar_t[len+1];
  pIn[len] = C_NULL;

  if (len)
  {
    wcsncpy(buf, pIn, len);
    buf[len] = C_NULL;
  }

  return buf;
}
//---------------------------------------------------------------------------
// Clipboard functions
//---------------------------------------------------------------------------
WideString __fastcall TUtils::GetClipboardText(void)
// Will try to get the text as UTF-16 first - than as ANSI
{
  WideString sOut;
  void* hText = NULL;

  Clipboard()->Open();

  try
  {
    try
    {
      if (Clipboard()->HasFormat(CF_UNICODETEXT))
      {
        hText = (void*)Clipboard()->GetAsHandle(CF_UNICODETEXT);
        wchar_t* pText = (wchar_t*)GlobalLock(hText);
        sOut = WideString(pText);
      }
      else if (Clipboard()->HasFormat(CF_TEXT))
      {
        hText = (void*)Clipboard()->GetAsHandle(CF_TEXT);
        char* pText = (char*)GlobalLock(hText);
        sOut = WideString(String(pText));
      }
    }
    catch(...) {}
  }
  __finally
  {
    if (hText != NULL)
      GlobalUnlock(hText);
  }

  Clipboard()->Close();

  return sOut;
}
//---------------------------------------------------------------------------
String __fastcall TUtils::LoadHtmlFromClipboard(void)
// Returns HTML (always in UTF-8 format), replete with header and tags
{
  String S;

  HGLOBAL h = NULL;;

  try
  {
    Clipboard()->Open();
    h = (HGLOBAL)Clipboard()->GetAsHandle(cbHTML);

    char* p = (char*)GlobalLock(h);

    if (p)
      S = String(p);
  }
  __finally
  {
    if (h) GlobalUnlock(h);
    try {Clipboard()->Close();} catch(...){}
  }

  return S;
}
//---------------------------------------------------------------------------
void __fastcall TUtils::ClearClipboard(void)
// Will try to get the text as UTF-16 first - than as ANSI
{
  if (OpenClipboard(dts->Handle))
  {
    EmptyClipboard();
    CloseClipboard();
  }
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::CopyTextToClipboard(WideString wIn)
// Will try to get the text as UTF-16 first - than as ANSI
{
  try
  {
    if (!OpenClipboard(dts->Handle))
      return false;

    //
    // Save the UTF-16 Version
    //
    char term[] = "\0\0\0\0"; // Terminator
    int termlen = 4;

    // First set the CF_UNICODETEXT handle...
    int len = wIn.Length() * sizeof(wchar_t);

    HGLOBAL hMem = GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, len+termlen);

    if (hMem == NULL)
      return false;

    try
    {
      // Move string into global-memory
      char* lp = (char*)GlobalLock(hMem);
      CopyMemory(lp, wIn.c_bstr(), len);
      CopyMemory(lp+len, &term, termlen);
      // Put the memory-handle on the clipboard
      // DO NOT FREE this memory!
      SetClipboardData(CF_UNICODETEXT, hMem);
      GlobalUnlock(hMem);
    }
    catch(...) { return false; }

    //
    // Save the ANSI Version
    //

    // Set the CF_TEXT handle...
// Don't need this because windows can convert to CF_TEXT
//    String sAnsi = String(wIn);
//    len = sAnsi.Length() * sizeof(char);
//
//    hMem = GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, len+termlen);
//
//    if (hMem == NULL)
//      return false;
//
//    try
//    {
//      // Move string into global-memory
//      char* lp = (char*)GlobalLock(hMem);
//      CopyMemory(lp, sAnsi.c_str(), len);
//      CopyMemory(lp+len, &term, termlen);
//      // Put the memory-handle on the clipboard
//      // DO NOT FREE this memory!
//      SetClipboardData(CF_TEXT, hMem);
//      GlobalUnlock(hMem);
//    }
//    catch(...) { return false; }

    CloseClipboard();

    return true;
  }
  catch(...) { return false; }
}
//---------------------------------------------------------------------------
bool __fastcall TUtils::CopyRtfToClipboard(String sIn)
// Returns true if success
{
  try
  {
    int strLen = sIn.Length();
    int bytes = strLen+4;

    if (!OpenClipboard(Application->Handle))
      return false;

    HANDLE hMem = NULL;

    try
    {
      hMem = GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, bytes);

      if (hMem != NULL)
      {
        // Move string into global-memory
        char* lp = (char*)GlobalLock(hMem);

        if (lp != NULL)
        {
          CopyMemory(lp, sIn.c_str(), strLen);
          CopyMemory(lp+strLen, "\0\0\0\0", 4); // zero-out the four padding bytes

          // Put the memory-handle on the clipboard
          // DO NOT FREE this memory!
          SetClipboardData(cbRTF, hMem);
        }
      }
    }
    __finally
    {
      if (hMem != NULL)
        GlobalUnlock(hMem);
    }

    CloseClipboard();

    return true;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nError in TUtils::WriteRtfToClipboard()\r\n");
#endif
    return false;
  }
}
//---------------------------------------------------------------------------
WideString __fastcall TUtils::ClipboardHtmlToIrc(bool bShowStatus)
// Converts clipboard text to Html
// Returns empty string if failure
{
  return HtmlToIrc("", bShowStatus);
}

WideString __fastcall TUtils::HtmlToIrc(WideString FileName, bool bShowStatus)
// Returns empty string if failure
{
  try
  {
    TConvertFromHTML* c = new TConvertFromHTML(dts, FileName);
    WideString S = c->Execute(bShowStatus);
    delete c;

    S = Optimize(S, bShowStatus, NO_COLOR);

    return S;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nError in TUtils::HtmlToIrc()\r\n");
#endif
    return "";
  }
}
//---------------------------------------------------------------------------
int __fastcall TUtils::min(int a, int b)
{
  if ( a < b)
    return a;
  return b;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::max(int a, int b)
{
  if ( a > b)
    return a;
  return b;
}
//---------------------------------------------------------------------------
TIniFile* __fastcall TUtils::OpenIniFile(WideString wFilePath, bool bShow)
// Clear bShow to mute error message...
// Returns a pointer to the TIniFile - you MUST delete it later!!!
{
  try
  {
    WideString IniFile;

    if (wFilePath.IsEmpty())
      // Look in "Users\(name)\Discrete-Time Systems\YahCoLoRiZe" folder
      IniFile = GetSpecialFolder(CSIDL_APPDATA) +
          "\\" + Utf8ToWide(OUR_COMPANY) + "\\" + Utf8ToWide(OUR_NAME) +
                                                  "\\" + Utf8ToWide(INIFILE);
    else
      IniFile = wFilePath;

    // look in common application data folder, if it's not there print a message...
    if (!FileExistsW(IniFile))
    {
      // Look in the exe path...
      IniFile = ExtractFilePathW(GetExeNameW()) + Utf8ToWide(INIFILE);

      if (!FileExistsW(IniFile))
      {
        // "x does not exist. Please reinstall YahCoLoRiZe..."
        if (bShow)
          ShowMessageU(String(DS[222]) + "\n" +
            WideToUtf8(IniFile) + "\n\n" + String(DS[223]));

        return NULL;
      }
    }

    String sShortPath = GetAnsiPathW(IniFile);

    // Fool our ANSI TIniFile class into opening a UTF-16 file-path
    // by converting it to a short-path format...
    if (!FileExists(sShortPath))
    {
      ShowMessage("Can't find Settings (.ini) file!\n\n" + sShortPath);
      return NULL;
    }

    return new TIniFile(sShortPath);
  }
  catch(...){ return NULL; }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// UNUSED CODE

/*
//---------------------------------------------------------------------------
WideString __fastcall TUtils::ReplaceAllW(String sIn, wchar_t c1, wchar_t c2)
{
  return ReplaceAllW(sIn, WideString(c1), WideString(c2));
}

WideString __fastcall TUtils::ReplaceAllW(WideString sIn, WideString S1, WideString S2)
{
  TReplaceFlags rFlags = (TReplaceFlags() << rfReplaceAll << rfIgnoreCase);
  return StringReplace(sIn, S1, S2, rFlags);
}
//---------------------------------------------------------------------------
// Overloaded
int __fastcall TUtils::MakeRGBPalette(WideString S, bool bNoView)
{
  return MakeRGBPalette(S.c_bstr(), S.Length(), bNoView);
}

int __fastcall TUtils::MakeRGBPalette(wchar_t * pBuf, int iSize, bool bNoView)
{
  if (!dts->RGB_Palette)
    return 2;

  int ReturnValue = 0; // Success

  try
  {
    dts->RGB_PaletteSize = 0;

    // Add Black & White as a minimum
    AddRGB(0x00ffffff); // White
    AddRGB(0x00000000); // Black

    // Analyze Buffer-data for color-codes, and save unique codes
    int ii;
    for (ii = 0; ii < iSize; ii++)
    {
      if (pBuf[ii] == CTRL_K)
      {
        int fg = NO_COLOR;
        int bg = NO_COLOR;

        ii += this->CountColorSequence(pBuf, ii, iSize, fg, bg);

        // RGB?
        if (fg <= 0)
        {
          if (AddRGB(-fg)) // full-table?
            break; // all colors won't fit
        }
        else if (bg <= 0)
        {
          if (AddRGB(-bg)) // full-table?
            break; // all colors won't fit
        }
      }
    }

    // Need to use old-style table?
    if (dts->WordColors->Checked)
    {
      if (dts->RGB_PaletteSize > SMALLRGBPALETTESIZE)
        MakeSmallPalette();
    }
    // Could not fit all of the needed document colors. We hit
    // MAXRGBPALETTECOLORS so make a Palette of MEDIUMRGBPALETTESIZE
    // and the program will have to resolve (interpolate) from the
    // medium table.
    else if (ii < iSize)
      MakeMediumPalette();
  }
  catch(...)
  {
    utils->ShowMessageU(DS[95] + "MakeRGBPalette()");
    ReturnValue = 2;
  }

  return ReturnValue;
}
//---------------------------------------------------------------------------
void __fastcall TUtils::MakeSmallPalette(void)
{
  // 6 to the 3rd power = 216 RGB colors.  RTF can support
  // a palette-index of 0-255, we already use 0-15. 3 to the
  // 7th power pushes us over 255, so we settle for 6 cubed
  // (three colors).  Now the step size is 51 to smoothly go from
  // black (0,0,0) to white (255,255,255).

  // 0,0,0 0,0,51 0,0,102, 0,0,153 0,0,204 0,0,255
  // 0,1,0 0,1,51 0,1,102, 0,1,153 0,1,204 0,1,255 etc...
  // .
  // .
  // 255,255,0 255,255,51 255,255,102, 255,255,153 255,255,204 255,255,255
  //
  BlendColor BC;

  dts->RGB_PaletteSize = 0;

  // Generate color-table
  for (int r = 0; r < 256; r += 51)
    for (int g = 0; g < 256; g += 51)
      for (int b = 0; b < 256; b += 51)
      {
        BC.red = r;
        BC.green = g;
        BC.blue = b;
        dts->RGB_Palette[dts->RGB_PaletteSize++] = BlendColorToBin(BC);
      }

  // Confirmed 216 entries!
  // ShowMessage(dts->RGB_PaletteSize);
}
//---------------------------------------------------------------------------
void __fastcall TUtils::MakeMediumPalette(void)
{
  // 6 to the 3rd power = 216 RGB colors.  RTF can support
  // a palette-index of 0-255, we already use 0-15. 3 to the
  // 7th power pushes us over 255, so we settle for 6 cubed
  // (three colors).  Now the step size is 51 to smoothly go from
  // black (0,0,0) to white (255,255,255).

  // 0,0,0 0,0,51 0,0,102, 0,0,153 0,0,204 0,0,255
  // 0,1,0 0,1,51 0,1,102, 0,1,153 0,1,204 0,1,255 etc...
  // .
  // .
  // 255,255,0 255,255,51 255,255,102, 255,255,153 255,255,204 255,255,255
  //
  // (Nov, 2005) looks like table CAN go over 255.  So I increased the
  // size to 982 (982+16+1 = 999).  For our auto-generated table
  // we can go up to 9 to the 3rd power now (729). Use 31 steps to get
  // max r/g/b values of 248.
  BlendColor BC;

  dts->RGB_PaletteSize = 0;

  // Generate color-table
  for (int r = 0; r < 256; r += 31)
    for (int g = 0; g < 256; g += 31)
      for (int b = 0; b < 256; b += 31)
      {
        BC.red = r;
        BC.green = g;
        BC.blue = b;
        dts->RGB_Palette[dts->RGB_PaletteSize++] =
                                               BlendColorToBin(BC);
      }

  // Confirmed 729 entries!
  // ShowMessage(dts->RGB_PaletteSize);
}
//---------------------------------------------------------------------------

bool __fastcall TUtils::AddRGB(int c)
{
  if (!dts->RGB_Palette)
    return true; // error

  if (FindRGB(c) >= 0)
    return false; // don't need to add new color

  if (dts->RGB_PaletteSize >= MAXRGBPALETTECOLORS) // 982 colors
    return true; // oops, can't fit all the colors

  dts->RGB_Palette[dts->RGB_PaletteSize++] = c;
  return false; // color added
}
//---------------------------------------------------------------------------

int __fastcall TUtils::GetRgbPaletteIndex(int c)
// We return a 1-based palette-index to avoid confusion with
// RGB colors which are negative or 0. If the returned value is
// 0 then we could not resolve the color!
{
  // OK, here we are given a color that reflects the main color
  // panels.  It can be 0 or negative which is an RGB color,
  // or 1-16 which is a 16-color palette-index.
  //
  // What we want returned is simply c if its 1-16 AND we are
  // not doing any RGB colors (I.E. an IRC client is selected).
  //
  // If we are in RGB mode and c is 1-16, we want to get the binary
  // RGB value from our local 16-color palette...
  //
  // If c <= 0 we have a RGB binary value already, just negate it.
  //
  // NOW we want to Resolve that rgb color to a Medium RGB table
  // pointed to by RGB_Palette of size RGB_PaletteSize.
  //
  // int __fastcall TUtils::ResolveRGB(int Color) gives us
  // an index into the table to return...
  //
  // For consistency, the returned index value is 1-based so
  // subtract 1 for a table-lookup!
  //
  // This routine is used in effects where we increment an index
  // into a table. E_INC_FGBG is about all it is used for at
  // present.
  //
  // We return a 1-based palette-index to avoid confusion with
  // RGB colors which are negative or 0. If the returned value is
  // 0 then we could not resolve the color!

  if (!dts->IsYahTrinPal() || !dts->RGB_Palette ||
                                      !dts->RGB_PaletteSize)
  {
    if (c <= 0)
      return ResolveRGBLocal(-c);

    return c;
  }

  // local palette index?
  if (c > 0)
  {
    c--; // Adjust to a 0-based index

    if (c >= MAXPALETTECOLORS)
      return 1; // Return some safe index!! (Error)

    c = -dts->PaletteInit[c]; // Get binary RGB value
  }

  int idx = FindRGB(-c); // Look up RGB color in Medium-table

  if (idx >= 0)
    return idx+1; // Found exact match! Return a 1-based index (non-RGB)

  // Have to resolve...
  return ResolveRGB(-c) + 1;
}
//---------------------------------------------------------------------------
int __fastcall TUtils::FindRGB(int c)
// Given an RGB color (as a positive int)
// returns the 0-based index into the RGB color-palette
{
  if (!dts->RGB_Palette)
    return(-1); // error

  for (int ii = 0; ii < dts->RGB_PaletteSize; ii++)
    if (dts->RGB_Palette[ii] == c)
      return(ii); // found

  return -1; // not found
}
//---------------------------------------------------------------------------
int __fastcall TUtils::ResolveRGB(int Color)
{
  return ResolveRGB(BinToBlendColor(Color));
}

//---------------------------------------------------------------------------
int __fastcall TUtils::ResolveRGB(BlendColor BC)
{
  // Returns an index 0-728 into an array of BlendColor structs
  // which represent a custom palette used in the RTF view.

  int i, limit, start;

  // Palette exists?
  if (dts->RGB_Palette == NULL) return -1;

  // Palette right size?
  if (dts->RGB_PaletteSize != SMALLRGBPALETTESIZE && dts->RGB_PaletteSize != MEDIUMRGBPALETTESIZE)
    return -1;

  BlendColor X,Y;

  if (dts->RGB_PaletteSize == SMALLRGBPALETTESIZE)
  {
    // get closest red...
    for (i = 0; i < dts->RGB_PaletteSize; i += 36)
    {
      X = BinToBlendColor(dts->RGB_Palette[i]);

      if (X.red >= BC.red)
      {
        if (i >= 36)
        {
          Y = BinToBlendColor(dts->RGB_Palette[i-36]);
          if (X.red-BC.red >= BC.red-Y.red) i -= 36; // back up, old values were closer
        }

        break;
      }
    }

    // get closest green...
    limit = i + 36;
    start = i;

    for (; i < dts->RGB_PaletteSize && i < limit; i += 6)
    {
      X = BinToBlendColor(dts->RGB_Palette[i]);

      if (X.green >= BC.green)
      {
        if (i >= start + 6)
        {
          Y = BinToBlendColor(dts->RGB_Palette[i-6]);
          if (X.green-BC.green >= BC.green-Y.green) i -= 6; // back up, old values were closer
        }

        break;
      }
    }

    // get closest blue...
    limit = i + 6;
    start = i;

    for (; i < dts->RGB_PaletteSize && i < limit; i++)
    {
      X = BinToBlendColor(dts->RGB_Palette[i]);

      if (X.blue >= BC.blue)
      {
        if (i >= start + 1)
        {
          Y = BinToBlendColor(dts->RGB_Palette[i-1]);
          if (X.blue-BC.blue >= BC.blue-Y.blue) i -= 1; // back up, old values were closer
        }

        break;
      }
    }
  }
  else // MEDIUMRGBPALETTESIZE (729) colors in table
  {
    // RGB values stop at 248, not 255!!!!!!!!
    if (BC.red > 248) BC.red = 248;
    if (BC.green > 248) BC.green = 248;
    if (BC.blue > 248) BC.blue = 248;

    // get closest red...
    for (i = 0; i < dts->RGB_PaletteSize; i += 81)
    {
      X = BinToBlendColor(dts->RGB_Palette[i]);

      if (X.red >= BC.red)
      {
        if (i >= 81)
        {
          Y = BinToBlendColor(dts->RGB_Palette[i-81]);
          if (X.red-BC.red >= BC.red-Y.red) i -= 81; // back up, old values were closer
        }

        break;
      }
    }

    // get closest green...
    limit = i + 81;
    start = i;

    for (; i < dts->RGB_PaletteSize && i < limit; i += 9)
    {
      X = BinToBlendColor(dts->RGB_Palette[i]);

      if (X.green >= BC.green)
      {
        if (i >= start + 9)
        {
          Y = BinToBlendColor(dts->RGB_Palette[i-9]);
          if (X.green-BC.green >= BC.green-Y.green) i -= 9; // back up, old values were closer
        }

        break;
      }
    }

    // get closest blue...
    limit = i + 9;
    start = i;

    for (; i < dts->RGB_PaletteSize && i < limit; i++)
    {
      X = BinToBlendColor(dts->RGB_Palette[i]);

      if (X.blue >= BC.blue)
      {
        if (i >= start + 1)
        {
          Y = BinToBlendColor(dts->RGB_Palette[i-1]);
          if (X.blue-BC.blue >= BC.blue-Y.blue) i -= 1; // back up, old values were closer
        }

        break;
      }
    }

  }

//  ShowMessage("i=" + WideString(i) +
//               " red=" + IntToHex(Red.ToIntDef(0x000000), 2) +
//               " green=" + IntToHex(Green.ToIntDef(0x000000), 2) +
//               " blue=" + IntToHex(Blue.ToIntDef(0x000000), 2));

  if (i >= dts->RGB_PaletteSize) return -1; // Unable to resolve

  return i;
}

void __fastcall TUtils::PrintWindow(HWND hwnd)
{
    //getting window title for use it on print name
    string windowtitle;
    GetWindowString(hwnd,windowtitle);
    DOCINFO doc = {sizeof(DOCINFO), windowtitle.c_str()};
    
    //getting the default printer name:
    //getting the string size for the printer name
    //and then the printer name
    unsigned long lPrinterNameLength;
    GetDefaultPrinter(NULL, &lPrinterNameLength ;
    char szPrinterName[lPrinterNameLength];
    GetDefaultPrinter(szPrinterName, &lPrinterNameLength);
    
    //getting the printer DC, using the default printer name
    HDC PrinterDC = CreateDC("WINSPOOL", szPrinterName, NULL, NULL);
    
    //getting the window DC and it client rect
    HDC WindowDC = GetDC(hwnd);
    RECT windowrect;
    GetClientRect(hwnd, &windowrect);

    //convert from pixels to twips
    int PrinterWidth = MulDiv(windowrect.right,
      GetDeviceCaps(PrinterDC , LOGPIXELSX),
        GetDeviceCaps(WindowDC, LOGPIXELSX));;
    int PrinterHeight = MulDiv(windowrect.bottom,
      GetDeviceCaps(PrinterDC , LOGPIXELSY),
        GetDeviceCaps(WindowDC , LOGPIXELSY)); ;

    //start print...
    StartDoc(PrinterDC, &doc);
    StartPage(PrinterDC);

    //here we can draw what we want using GDI functions
    //but never forget convert from pixels to twips

    //draw the windc to printerdc
    StretchBlt(PrinterDC, 0, 0, PrinterWidth, PrinterHeight,
      WindowDC, 0, 0, windowrect.right, windowrect.bottom, SRCCOPY);
    
    //end print...
    EndPage(PrinterDC);
    EndDoc(PrinterDC);
    
    //delete GDI resources
    DeleteDC(PrinterDC);
    ReleaseDC(hwnd, WindowDC);
    DeleteDC(WindowDC);
}
*/

