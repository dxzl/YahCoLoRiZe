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

#include "Undo.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
TTOCUndo* TOCUndo = NULL; // Instantiated in ThreadOnChange.cpp
//---------------------------------------------------------------------------
// UNDOITEM struct functions
//---------------------------------------------------------------------------
__fastcall UNDOSHORT::UNDOSHORT(int Type, int View, int Len, void* P,
            TPoint Pos, TPoint Loc, wchar_t C, bool bInsert, bool bChain)
// constructor
{
  this->type = Type;
  this->view = View;
  this->len = Len;
  this->p = (TStringsW*)P;
  this->pos = Pos;
  this->loc = Loc;
  this->c = C;
  this->bInsert = bInsert;
  this->bChain = bChain;
}

__fastcall UNDOITEM::UNDOITEM(int Type, int Index, int Length,
                                    ONCHANGEW oc, WideString s, bool bChain)
// constructor
{
  this->Type = Type;
  this->Index = Index;
  this->Length = Length;
  this->bChain = bChain;
  this->oc = oc;
  this->s = s;
}
//---------------------------------------------------------------------------
// DtsUndo class functions
//---------------------------------------------------------------------------
__fastcall TTOCUndo::TTOCUndo(void)
// constructor
{
  pTList = new TList();
}
//---------------------------------------------------------------------------
__fastcall TTOCUndo::~TTOCUndo(void)
// destructor
{
  this->Clear(); // Delete the items and clear the list

  // Delete the TList
  try {delete pTList;} catch(...) {}

  TOCUndo = NULL;
}
//---------------------------------------------------------------------------
int __fastcall TTOCUndo::GetItemsCount(void)
{
  return pTList->Count;
}
//---------------------------------------------------------------------------
void __fastcall TTOCUndo::Clear(void)
{
  if (pTList != NULL)
  {
    DTSColor->UndoEnabled = false;

    // Delete the UNDOITEM objects
    for(int ii = 0; ii < pTList->Count; ii++)
      try {delete pTList->Items[ii];} catch(...) {}

    pTList->Clear();
  }
}
//---------------------------------------------------------------------------
void __fastcall TTOCUndo::ClearMost(void)
// Delete all events up to and including the first UNDO_CLEAR in the
// list - you need to make sure it's the ONLY undo in the list. So add
// your new UNDO_CLEAR after calling this!
{
  if (pTList != NULL)
  {
    UNDOITEM* ui;

    int ii;
    int count = pTList->Count;

    for(ii = 0; ii < count; ii++)
    {
      ui = (UNDOITEM*)pTList->Items[ii];

      if (ui == NULL)
      {
#if DEBUG_ON
        DTSColor->CWrite( "\r\nTTOCUndo::ClearMost(): ui is NULL!!\r\n");
#endif
        return;
      }

      if (ui->Type == UNDO_CLEAR)
        break;
    }

    if (ii >= count)
      goto noclears;
//      return; // no UNDO_CLEARs

    count = ii;

    // remove all events up to and including UNDO_CLEAR
    for(ii = 0; ii <= count; ii++)
    {
      if (!Delete(0))
      {
#if DEBUG_ON
        DTSColor->CWrite( "\r\nTTOCUndo::ClearMost(): Can't delete!\r\n");
#endif
        return;
      }
    }

noclears:
    // Now remove UNDO_INS
    for(ii = 0; ii < pTList->Count; )
    {
      ui = (UNDOITEM*)pTList->Items[ii];

      if (ui == NULL)
      {
#if DEBUG_ON
        DTSColor->CWrite( "\r\nTTOCUndo::ClearMost(): ui is NULL!!\r\n");
#endif
        return;
      }

      if (ui->Type == UNDO_INS)
      {
        if (!Delete(ii))
        {
#if DEBUG_ON
          DTSColor->CWrite( "\r\nTTOCUndo::ClearMost(): Can't delete!\r\n");
#endif
          return;
        }
      }
      else
        ii++;
    }
  }
}
//---------------------------------------------------------------------------
/*
void __fastcall TTOCUndo::ClearMost(void)
// Clear all except most-recent UNDO_CLEAR and UNDO_DELs
{
  if (pTList != NULL)
  {
    for(int ii = 0; ii < pTList->Count;)
    {
      UNDOITEM* ui = (UNDOITEM*)pTList->Items[ii];

      if (ui == NULL)
      {
#if DEBUG_ON
        DTSColor->CWrite( "\r\nTTOCUndo::ClearMost(): ui is NULL!!\r\n");
#endif
        break;
      }

      if (ui->Type != UNDO_CLEAR)
        Delete(ii);
      else
        ii++;
    }

    // delete all but the newest UNDO_CLEAR
    while(pTList->Count > 1)
      Delete(0);
  }
}
*/
//---------------------------------------------------------------------------
bool __fastcall TTOCUndo::Delete(int idx)
{
  if (pTList->Count > 0 && idx < pTList->Count)
  {
    try
    {
      UNDOITEM* ui = (UNDOITEM*)pTList->Items[idx];

      bool bRet;

      if (ui != NULL)
      {
        try
        {
          if (ui->Type < US_START)
            delete ui; // delete object
          else
          {
            UNDOSHORT* us = (UNDOSHORT*)ui;
            delete us; // delete object
          }
          bRet = true;
        }
        catch(...)
        {
#if DEBUG_ON
          DTSColor->CWrite( "\r\nTTOCUndo::Delete() Can't delete undo object!\r\n");
#endif
          bRet = false;
        }
      }

      pTList->Delete(idx); // Delete pointer to object

      if (pTList->Count == 0)
        DTSColor->UndoEnabled = false;

      return bRet;
    }
    catch(...)
    {
#if DEBUG_ON
      DTSColor->CWrite( "\r\nTTOCUndo::Delete() Exception!\r\n");
#endif
      return false;
    }
  }

  return false;
}
//---------------------------------------------------------------------------
void __fastcall TTOCUndo::Add(int Type, int View, int Len, void* P, TPoint Pos,
                          TPoint Loc, wchar_t C, bool bInsert, bool bChain)
{
  try
  {
    if (this->UndoList->Count >= MAX_UNDO)
      this->UndoList->Delete(0); // Delete oldest

    // Add new
    pTList->Add(new UNDOSHORT(Type, View, Len, P, Pos, Loc, C,
                                                            bInsert, bChain));

    if (!DTSColor->UndoEnabled)
      DTSColor->UndoEnabled = true;
//#if DEBUG_ON
//    DTSColor->CWrite("\r\nAddUndo:" + String(Type) + "\r\n");
//#endif
  }
  catch(...) {}
}

void __fastcall TTOCUndo::Add(int Type, int Index, int Length, ONCHANGEW oc,
                                                    WideString s, bool bChain)
{
  try
  {
    if (this->UndoList->Count >= MAX_UNDO)
      this->UndoList->Delete(0); // Delete oldest

    // Add new
    pTList->Add(new UNDOITEM(Type, Index, Length, oc, s, bChain));

    if (!DTSColor->UndoEnabled)
      DTSColor->UndoEnabled = true;
//#if DEBUG_ON
//    DTSColor->CWrite("\r\nAddUndo:" + String(Type) + "\r\n");
//#endif
  }
  catch(...) {}
}
//---------------------------------------------------------------------------
void __fastcall TTOCUndo::Undo(void)
{
  if (tae->LockCounter != 0)
    return;

  utils->PushOnChange(tae);
  utils->WaitForThread();

  // Loop while one Undo object chains to the previous...
  while (DoUndo());

  utils->PopOnChange(tae);
}
//---------------------------------------------------------------------------
bool __fastcall TTOCUndo::DoUndo(void)
{
  if (pTList->Count <= 0)
    return false;

  try
  {
    // Get pointer to most-recent edit-change
    UNDOITEM* ui = (UNDOITEM*)pTList->Items[pTList->Count-1];

    if (ui == NULL)
    {
#if DEBUG_ON
      DTSColor->CWrite( "\r\nDoUndo(): Found NULL pointer in pTList!\r\n");
#endif
      this->Clear(); // List is corrupt, clear it
      return false;
    }

    // get to local var before we possibly re-cast to the "us" pointer...
    int type = ui->Type;

    if (pTList->Count == 1 && (type == UNDO_PROCESS ||
                                                type == UNDO_SHOW))
    {
      if (utils->ShowMessageU(DTSColor->Handle, DS[180],
                      MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDNO)
        return false; // Cancel
    }

    // i'm sure there is a more elegant way to do this when time permits - until
    // then be careful to only use the "us" pointer in >= US_START events
    // and only use the "ui" pointer and oc in < US_START events!!!!!!!!
    ONCHANGEW oc;
    UNDOSHORT* us;
    if (type >= US_START)
      us = (UNDOSHORT*)ui; // re-cast to undo-short struct format
    else
      oc = ui->oc;

    switch (type)
    {
      case UNDO_EFFECT_ORIG_TEXT:

        if (oc.p != NULL)
        {
          // Here we want to paste back the original text range from
          // before the effect was applied...
          if (oc.p != NULL)
          {
            TStringsW* sl = (TStringsW*)oc.p;

            // Set to view prior to Process
            sl->Text = utils->InsertW(sl->Text, ui->s, ui->Index+1);

            if (oc.view == V_RTF)
              utils->ConvertToRtf(sl, DTSColor->MS_RTF);

            DTSColor->LoadView(oc.view);
            tae->SelStart = oc.selStart;
          }
        }

      break;

      case UNDO_EFFECT_NEW_TEXT:

        // Here we want to cut out the effect-text then chain-up to
        // UNDO_EFFECT_ORIG_TEXT and paste in String s.
        if (oc.p != NULL)
        {
          TStringsW* sl = (TStringsW*)oc.p;

          // Set to view prior to Process
          sl->Text = utils->DeleteW(sl->Text, ui->Index+1, ui->Length);
        }

      break;

      case UNDO_CLEAR:
      case UNDO_PROCESS:

        if (oc.p != NULL)
        {
          if (!ui->s.IsEmpty())
          {
            TStringsW* sl = (TStringsW*)oc.p;
            sl->Text = ui->s;

            if (oc.view == V_RTF)
              utils->ConvertToRtf(sl, DTSColor->MS_RTF);

            DTSColor->LoadView(oc.view);
            tae->SelStart = oc.selStart;
          }
        }
      break;

      case UNDO_SHOW:

        // Here we want to undo the press of the Show button...
        // So really we just need to load the previous view and clear
        // the TextWasProcessed flag
        DTSColor->LoadView(oc.view);
        DTSColor->TextWasProcessed = false;
        tae->SelStart = oc.selStart;
        utils->SetOldLineVars(tae, true);

      break;

      case UNDO_DEL_SHORT:
      {
        if (us->p != NULL)
        {
          TStringsW* sl = (TStringsW*)us->p;

          // Paste to the string-list then load the old view
          try
          {
            WideString wTemp = sl->GetString(us->pos.y);

            // C_CR is a flag for a required cr/lf to be inserted, so
            // expand the string.
            WideString wC = us->c;
            if (us->c == C_CR)
              wC += WideString(C_LF); // MUST cast to WideString with "+=" (compiler issue!)

            // restore string and write to document
            wTemp = utils->InsertW(wTemp, wC, us->pos.x+1);
            sl->SetStringEx(wTemp, us->pos.y);

            if (sl->TotalLength == 0)
            {
              sl->Clear();
              tae->Clear();
              DTSColor->LoadView(V_OFF);
              utils->SetOldLineVars(tae, true);
            }
            else
            {
              if (us->view == V_RTF)
                utils->ConvertToRtf(sl, DTSColor->MS_RTF);

              DTSColor->LoadView(us->view);
              tae->CaretPos = us->loc;
            }
          }
          catch(...)
          {
#if DEBUG_ON
            DTSColor->CWrite( "\r\nDoUndo():UNDO_DEL_SHORT:Exception...\r\n");
#endif
          }
        }
#if DEBUG_ON
        else
          DTSColor->CWrite( "\r\nDoUndo():UNDO_DEL_SHORT: null string-list pointer!\r\n");
#endif
      }
      break;

      case UNDO_DEL:
      {
        if (oc.p != NULL)
        {
          TStringsW* sl = (TStringsW*)oc.p;

          // Paste to the string-list then load the old view
          try
          {
            WideString wTemp = sl->GetString(oc.line);
            wTemp = utils->InsertW(wTemp, ui->s, ui->Index+1);
            sl->SetStringEx(wTemp, oc.line);

            if (sl->TotalLength == 0)
            {
              sl->Clear();
              tae->Clear();
              DTSColor->LoadView(V_OFF);
              utils->SetOldLineVars(tae, true);
            }
            else
            {
              if (oc.view == V_RTF)
                utils->ConvertToRtf(sl, DTSColor->MS_RTF);

              DTSColor->LoadView(oc.view);
              tae->SelStart = oc.selStart;
            }
          }
          catch(...)
          {
#if DEBUG_ON
            DTSColor->CWrite( "\r\nDoUndo():UNDO_DEL:Exception...\r\n");
#endif
          }
        }
        else // Paste to the main edit-control
        {
          try
          {
            tae->SelStart = ui->Index;

            if (type == UNDO_REPLACE)
              tae->SelLength = ui->Length;
            else
              tae->SelLength = 0;

            tae->SelTextW = ui->s;
          }
          catch(...)
          {
#if DEBUG_ON
            DTSColor->CWrite( "\r\nDoUndo(): Exception 2 thrown...\r\n");
#endif
          }
        }
      }
      break;

      case UNDO_REPLACE:
      case UNDO_CUT:
      {
        // Note: When adding an UNDO_CUT, don't use the Length parameter.
        // We use it for UNDO_REPLACE to tell us how much text to cut!

        if (oc.p != NULL)
        {
          TStringsW* sl = (TStringsW*)oc.p;

          // Paste to the string-list then load the old view
          try
          {
            if (type == UNDO_REPLACE)
              sl->Text = utils->DeleteW(sl->Text, ui->Index+1, ui->Length);

            sl->Text = utils->InsertW(sl->Text, ui->s, ui->Index+1);

            if (sl->TotalLength == 0)
            {
              sl->Clear();
              tae->Clear();
              DTSColor->LoadView(V_OFF);
              utils->SetOldLineVars(tae, true);
            }
            else
            {
              if (oc.view == V_RTF)
                utils->ConvertToRtf(sl, DTSColor->MS_RTF);

              DTSColor->LoadView(oc.view);
              tae->SelStart = oc.selStart;
            }
          }
          catch(...)
          {
#if DEBUG_ON
            DTSColor->CWrite( "\r\nDoUndo():UNDO_CUT:Exception1...\r\n");
#endif
          }
        }
        else // Paste to the main edit-control
        {
          try
          {
            tae->SelStart = ui->Index;

            if (type == UNDO_REPLACE)
              tae->SelLength = ui->Length;
            else
              tae->SelLength = 0;

            tae->SelTextW = ui->s;
          }
          catch(...)
          {
#if DEBUG_ON
            DTSColor->CWrite( "\r\nDoUndo():UNDO_CUT:Exception2...\r\n");
#endif
          }
        }
      }
      break;

      case UNDO_INS_SHORT:
      {
        if (us->p != NULL)
        {
          TStringsW* sl = (TStringsW*)us->p;

          try
          {
            // us->c has the char to write back. If it's a carrage-return
            //   char then overstrike-mode is irrelevant.
            // us->pos has the 0-based string index in y and the index into
            //   the stringlist string (SL_IRC or SL_ORG) in x.
            // us->loc is the x/y caret position
            // us->bInsert is true for insert mode and false for overstrile mode
            bool bCR = (us->c == C_CR) ? true : false;

            if (us->bInsert || bCR)
//            {
              sl->DeleteText(us->pos, us->len);
//ShowMessage(String(us->len));
//            }
            else // text was typed overstrike...
            {
              WideString wLine = sl->GetString(us->pos.y);

              if (us->pos.x-1 >= 0 && us->pos.x-1 < wLine.Length())
              {
                wLine[us->pos.x-1+1] = us->c;
                sl->SetString(wLine, us->pos.y);
              }
            }

            if (sl->TotalLength == 0)
            {
              sl->Clear();
              tae->Clear();
              DTSColor->LoadView(V_OFF);
              utils->SetOldLineVars(tae, true);
            }
            else
            {
              if (us->view == V_RTF)
                utils->ConvertToRtf(sl, DTSColor->MS_RTF);

              DTSColor->LoadView(us->view);

              TPoint newSelPos(us->loc);
              if (!us->bInsert && newSelPos.x > 0)
                newSelPos.x--;
              tae->CaretPos = newSelPos;
            }
          }
          catch(...)
          {
  #if DEBUG_ON
            DTSColor->CWrite( "\r\nDoUndo():UNDO_INS_SHORT:Exception...\r\n");
  #endif
          }
        }
      }
      break;

      case UNDO_INS:
      {
        if (oc.p != NULL)
        {
          TStringsW* sl = (TStringsW*)oc.p;

          try
          {
            // ui->Length has length of text inserted or overstruck
            // ui->s is empty
            // oc.prevPos.y has the string index at the insert point
            // ui->Index has the offset into string at oc.prevPos.y
            // oc.c is a buffer, null-terminated if < OC_BUFSIZE of overstrike chars
            if (oc.insert)
            {
              TPoint loc;
              loc.x = ui->Index;
              loc.y = oc.pos.y;
              
              sl->DeleteText(loc, ui->Length);
            }
            else // text was typed overstrike...
            {
              WideString wLine = sl->GetString(oc.pos.y);
              for(int ii = 0; ii < ui->Length && ii < OC_BUFSIZE; ii++)
                wLine[ui->Index+ii] = oc.c[ii];
              sl->SetString(wLine, oc.pos.y);
            }

            if (sl->TotalLength == 0)
            {
              sl->Clear();
              tae->Clear();
              DTSColor->LoadView(V_OFF);
              utils->SetOldLineVars(tae, true);
            }
            else
            {
              if (oc.view == V_RTF)
                utils->ConvertToRtf(sl, DTSColor->MS_RTF);

              DTSColor->LoadView(oc.view);

              int newSelPos = oc.selStart-oc.deltaLength;
              if (!oc.insert && newSelPos > 0)
                newSelPos--;
              tae->SelStart = newSelPos;
            }
          }
          catch(...)
          {
#if DEBUG_ON
            DTSColor->CWrite( "\r\nDoUndo():UNDO_INS:Exception1...\r\n");
#endif
          }
        }
        else // Cut from the main edit-control
        {
          try
          {
            DTSColor->LoadView(oc.view);
            Windows::TPoint pt(ui->Index, oc.line);
            tae->CaretPos = pt;
            tae->SelLength = ui->Length;
            tae->SelTextW = "";
          }
          catch(...)
          {
#if DEBUG_ON
            DTSColor->CWrite( "\r\nDoUndo():UNDO_INS:Exception2...\r\n");
#endif
          }
        }
      }
      break;

      case UNDO_PASTE:
      {
        if (oc.p != NULL)
        {
          TStringsW* sl = (TStringsW*)oc.p;

          // Cut from the string-list then load the old view
          try
          {
            if (type == UNDO_INS && !oc.insert) // overstrike of 1 char
            {
              // this is inefficient... eventually use individual strings in
              // instead of loading the entire document into a string just to
              // replace one char!!!!!!

              // this is a dumb thing, WideString() with a length specifier - the
              // length if forced to BE len - when what we want is for len to be a
              // maximum of le - so have to do it "by hand"!
              utils->ReplaceW(sl,
                  WideString(oc.c, utils->min(OC_BUFSIZE, wcslen(oc.c))),
                                                                ui->Index+1);
            }
            else // insert or paste mode
              utils->DeleteW(sl, ui->Index+1, ui->Length);

            if (sl->TotalLength == 0)
            {
              sl->Clear();
              tae->Clear();
              DTSColor->LoadView(V_OFF);
              utils->SetOldLineVars(tae, true);
            }
            else
            {
              if (oc.view == V_RTF)
                utils->ConvertToRtf(sl, DTSColor->MS_RTF);

              DTSColor->LoadView(oc.view);

              if (type == UNDO_PASTE)
                tae->SelStart = oc.selStart;
              else
                tae->SelStart = oc.selStart-1;
            }
          }
          catch(...)
          {
#if DEBUG_ON
            DTSColor->CWrite( "\r\nDoUndo():UNDO_PASTE:Exception1...\r\n");
#endif
          }
        }
        else // Cut from the main edit-control
        {
          try
          {
            DTSColor->LoadView(oc.view);
            tae->SelStart = ui->Index;
            tae->SelLength = ui->Length;
            tae->SelTextW = "";
          }
          catch(...)
          {
#if DEBUG_ON
            DTSColor->CWrite( "\r\nDoUndo():UNDO_PASTE:Exception2...\r\n");
#endif
          }
        }
      }
      break;

      case UNDO_FG_COLOR:
        DTSColor->Foreground = ui->Index;
        // Restore previous colors
        DTSColor->UpdateColorButtons();
      break;

      case UNDO_BG_COLOR:
        DTSColor->Background = ui->Index;
        // Restore previous colors
        DTSColor->UpdateColorButtons();
      break;

      case UNDO_WING_COLOR:
        DTSColor->WingColor = ui->Index;
        // Restore previous colors
        DTSColor->UpdateColorButtons();
      break;

      default:
      break;
    };

    // The chain flag is set for multi-step Undo operations
    // It says "Go do one more Undo!"
    bool bChain = (type >= US_START) ? us->bChain : ui->bChain;

//#if DEBUG_ON
//    String Temp;
//
//    if (ui->s == CRLF)
//      Temp = "CRLF";
//    else
//      Temp = String(ui->s);
//
//    DTSColor->CWrite( "\r\nDelete Undo Item:" +
//         String(pTList->Count-1) +
//         " Type ID:" + String(type) + " \"" + Temp + "\"\r\n");
//
//    if (bChain)
//      DTSColor->CWrite( "...Chain flag is set!\r\n");
//#endif

    this->Delete(pTList->Count-1); // Delete newest

    return bChain;
  }
  catch(...)
  {
#if DEBUG_ON
      DTSColor->CWrite( "\r\nDoUndo(): Main Exception...\r\n");
#endif
    return false;
  }
}
//---------------------------------------------------------------------------

