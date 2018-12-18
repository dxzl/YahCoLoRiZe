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

#if DEBUG_ON
#include <time.h>
#include <dos.h>
DWORD before, after;
#endif

#include "Utils.h"
#include "ThreadOnChange.h"

#pragma package(smart_init)

// FYI: Type __thread declares a variable that will be global but
// private to each thread in the program in which the statement
// occurs. They preserve a unique copy of global and static class
// variables. Each program thread maintains a private copy of a __thread
// variable for each thread.
//---------------------------------------------------------------------------
TThreadOnChange* ThreadOnChange = NULL;
//---------------------------------------------------------------------------
// Create suspended so we can set some vars in main.cpp!
__fastcall TThreadOnChange::TThreadOnChange(void) : TThread(true)
{
  this->FreeOnTerminate = true;
  this->ReturnValue = 0;
  this->bLocked = false;

  // Create memory buffer
  ocbuf = new ONCHANGEW[OC_MAXOBJECTS];
}
//---------------------------------------------------------------------------
__fastcall TThreadOnChange::~TThreadOnChange(void)
// This thread as used now never goes away it's just
// suspended via utils->PushOnChange() and resumed on
// utils->PopOnChange()
{
  if (ocbuf) try {delete [] ocbuf;} catch(...) {}

  ThreadOnChange = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TThreadOnChange::Execute(void)
// do work here ...
// if you need to access the UI controls,
// use the TThread::Synchronize() method
// Synchronize((TThreadMethod)PushTheButton);
{
  this->Flush();

  // Loop
  for(;;)
  {
    if (ridx != widx)
    {
      if (!ProcessOC(this->Read()))
      {
#if DEBUG_ON
        DTSColor->CWrite( "\r\nProcessOC() returned FALSE "
                       "in ThreadOnChange()!\r\n");
#endif
        this->Flush();
      }
    }
    else if (this->Terminated) // do clean-up here...
    {
      this->ReturnValue = 1; // User Terminated
      break;
    }
    else
//      this->Suspend(); // OnChange in Main.cpp Resumes!
      SuspendThread((void*)this->Handle); // OnChange in Main.cpp Resumes!
  }
}
//---------------------------------------------------------------------------
void __fastcall TThreadOnChange::Flush(void)
{
  ridx = widx = 0;
}
//---------------------------------------------------------------------------
ONCHANGEW __fastcall TThreadOnChange::Read(void)
{
  ONCHANGEW oc = ocbuf[ridx++];

  if (ridx >= OC_MAXOBJECTS)
    ridx = 0;

  return oc;
}
//---------------------------------------------------------------------------
void __fastcall TThreadOnChange::Add(ONCHANGEW oc)
{
  ocbuf[widx++] = oc;

  if (widx >= OC_MAXOBJECTS)
    widx = 0;

//  this->Resume();
  ResumeThread((void*)this->Handle);
}
//---------------------------------------------------------------------------
bool __fastcall TThreadOnChange::ProcessOC(ONCHANGEW oc)
// Use this for diagnostics! (set DEBUG_ON "true" in Main.h)
//#if DEBUG_ON
//  DTSColor->CWrite("\r\nhereA\r\n");
//#endif
{
  if (!oc.p)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nError 1 ProcessOC()\r\n");
#endif
    return false;
  }

  bLocked = true;

  try
  {
    int UndoIdx, UndoLen, UndoLine;
    WideString UndoStr;

    TStringsW* TaeSL = (TStringsW*)oc.p; // Get pointer

    if (oc.deltaLength < 0) // User deleted chars?
    {
      // Note: by the time this gets called, the char(s) have already
      // been removed from TaeEdit->Text! A Selected range is handled
      // by the Cut/Copy routine, not here!

      try
      {
        UndoLine = oc.pos.y;

        if (oc.view == V_RTF)
        {
          // Use efficient routines to get the length of text that also
          // spans character-formatting codes.

          // NOTE: we need a string-list version of GetCodeIndices to get
          // both indices without having to scan the string twice...
          //
          // NOTE: the LineCount in a rich-edit does not change as Enter is
          // pressed. It changes with addition/removal of the first char in
          // a line.

          bool bCodes = (oc.pos.x == 0) ? false : true;

          if ((UndoIdx = utils->GetCodeIndex(TaeSL, oc.pos.x,
                                                    UndoLine, bCodes)) >= 0)
          {
            // get the same index if undo-length is 1, but this time get the
            // index AFTER any leading codes...
            UndoLen = -oc.deltaLength;

            int iLast = utils->GetCodeIndex(TaeSL, oc.pos.x+UndoLen-1,
                                                      UndoLine, false) + 1;
            if (iLast >= 0)
              // add in the second char of any line-terminators to the delta
              // of last and first. last is 1 greater than the actual index of
              // the last character...
              UndoLen = iLast - UndoIdx + -oc.deltaLineCount;
#if DEBUG_ON
            else
              DTSColor->CWrite("\r\nError 2 ProcessOC()\r\n");
#endif
          }
#if DEBUG_ON
          else
            DTSColor->CWrite("\r\nError 3 ProcessOC()\r\n");
#endif
        }
        else // V_IRC, V_ORG or V_OFF
        {
          UndoIdx = oc.pos.x;
          UndoLen = -oc.deltaLength + -oc.deltaLineCount;
        }

        if (UndoLen >= 0 && UndoIdx >= 0)
        {
          bool bLineDel = (oc.deltaLineCount == -1);

          TPoint loc(UndoIdx, UndoLine);

          if (bLineDel)
            UndoStr = CRLF;
          else
            UndoStr = TaeSL->CopyText(loc, UndoLen);

          TaeSL->DeleteText(loc, UndoLen);

//#if DEBUG_ON
// DTSColor->CWrite("\r\nloc.x:" + String(loc.x) + ", loc.y:\"" + String(loc.y) + "\"\r\n");
// DTSColor->CWrite("\r\nUndoLen:" + String(UndoLen) + ", UndoStr:\"" + UndoStr + "\"\r\n");
//#endif
          // Add to undo-objects
          if (TOCUndo != NULL && !UndoStr.IsEmpty())
          {
            // here we go to a little extra trouble to use a small-footprint
            // undo structure if only one char or line-break was deleted
            if (UndoLen == 1 || bLineDel)
            {
              wchar_t c = UndoStr[1];;
              TOCUndo->Add(UNDO_DEL_SHORT, oc.view, UndoLen, oc.p, loc, oc.pos,
                                                              c, oc.insert);
            }
            else
              TOCUndo->Add(UNDO_DEL, UndoIdx, UndoLen, oc, UndoStr);
          }
#if DEBUG_ON
          else
            DTSColor->CWrite("\r\nError 4 ProcessOC()\r\n");
#endif
        }
#if DEBUG_ON
        else
          DTSColor->CWrite("\r\nError 5 ProcessOC()\r\n");
#endif
      }
      catch(...)
      {
#if DEBUG_ON
        DTSColor->CWrite("\r\nError 6 ProcessOC()\r\n");
#endif
        bLocked = false;
        return false;
      }
    }
    else // user added chars or overstrike is on
    {
      try
      {
        // oc.deltaLength is 1 for a char and also 1 when you hit "Enter" (CR)
        // it is 0 for overstrikes

        // Get buffer of new chars into a string - these chars are added in the
        // OnChange handler and line-terminators are a single CR.

        // have to expand any cr in UndoStr into cr/lf...
        for (int ii = 0; ii < OC_BUFSIZE; ii++)
        {
          wchar_t c = oc.c[ii];

          if (c == C_NULL)
            break;

          UndoStr += WideString(c);

          if (c == C_CR)
            UndoStr += WideString(C_LF);

        }
        UndoLen = UndoStr.Length();

        UndoLine = oc.pos.y;
        WideString wTemp = TaeSL->GetString(UndoLine); // get current line

        if (oc.view == V_RTF)
        {
          if ((UndoIdx = utils->GetCodeIndex(wTemp, oc.pos.x, false)) < 0)
          {
#if DEBUG_ON
            DTSColor->CWrite("\r\nError 7 ProcessOC()\r\n");
#endif
            UndoIdx = oc.pos.x;
          }
        }
        else // V_IRC, V_ORG, V_OFF
          UndoIdx = oc.pos.x;

        TPoint ptDelta(0, 0); // used for UNDO_INS_SHORT

        // string grew? (insert mode or we added new chars at end of
        // string if overstrike mode)
        if (oc.deltaLength > 0)
        {
          // don't tack on state if editing irc codes (V_IRC)...
          bool bAddState = (oc.view == V_RTF) ? true : false;
          wTemp = utils->InsertW(wTemp, UndoStr, UndoIdx+1);
          ptDelta = TaeSL->SetStringEx(wTemp, UndoLine, bAddState);

          // force insert mode on for undo...
          oc.insert = true;
        }
        else if (UndoIdx > 0) // overstrike
        {
          // Overstrike, by nature, won't have any cr/lfs!
          WideChar wc;
          int idx; // 1-based index
          int len = wTemp.Length();

          for (int ii = 0; ii < UndoLen; ii++)
          {
            idx = UndoIdx+ii;
            if (idx <= len)
            {
              wc = wTemp[idx]; // save original char
              wTemp[idx] = oc.c[ii]; // replace old char in document with new...
              oc.c[ii] = wc; // preserve old chars for Undo
            }
          }

          TaeSL->SetString(wTemp, UndoLine); // write string back

          // force insert mode off for undo...
          oc.insert = false;
        }
#if DEBUG_ON
        else
          DTSColor->CWrite("\r\nProcessOC(): Range Error in UndoIdx: " +
                                               String(UndoIdx) + "\r\n");
#endif

        // Add to undo-objects
        if (TOCUndo != NULL)
        {
          // here we go to a little extra trouble to use a small-footprint
          // undo structure if only one char or line-break was deleted
          if (UndoLen == 1 || (UndoLen == 2 && oc.deltaLineCount == +1))
          {
            wchar_t c;

            if (UndoLen != 1)
              c = C_CR;
            else
              c = oc.c[0];

            TPoint loc(UndoIdx, UndoLine);
            TOCUndo->Add(UNDO_INS_SHORT, oc.view, ptDelta.x, oc.p, loc, oc.pos,
                                                            c, oc.insert);
          }
          else
            TOCUndo->Add(UNDO_INS, UndoIdx, UndoLen, oc, "");
        }
      }
      catch(...)
      {
#if DEBUG_ON
        DTSColor->CWrite("\r\nError 8 ProcessOC()\r\n");
#endif
        bLocked = false;
        return false;
      }
    }
//#if DEBUG_ON
//    DTSColor->CWrite("\r\nTaeSL->Text after change:\r\n\"" + TaeSL->Text + "\"\r\n");
//#endif
  }
  catch(...)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nError 9 ProcessOC()\r\n");
#endif
    bLocked = false;
    return false;
  }

  bLocked = false;

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TThreadOnChange::IsBusy(void)
{
  return (bLocked || widx != ridx) ? true : false;
}
//---------------------------------------------------------------------------

