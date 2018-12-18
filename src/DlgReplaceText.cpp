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

#include <stdio.h>
#include <Clipbrd.hpp>

#include "DlgReplaceText.h"
#include "DlgIncDec.h"
#include "DlgSetColors.h"
#include "ConvertFromHTML.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "TaeRichEdit"
#pragma resource "*.dfm"
TReplaceTextForm *ReplaceTextForm;
//---------------------------------------------------------------------------
__fastcall TReplaceTextForm::TReplaceTextForm(TComponent* Owner)
  : TForm(Owner)
{
  this->dts = DTSColor;

  // Clipboard formats
  cbHTML = (unsigned short)RegisterClipboardFormat(HTML_REGISTERED_NAME);
  cbRTF = (unsigned short)RegisterClipboardFormat(RTF_REGISTERED_NAME);

  // Configure as a Find-dialog by default
  bAbort = false;
  bReplacing = false;
  ButtonCancel->Enabled = true;

  // Set checkboxes
  bWholeWord = false;
  bMatchCase = false;
  bWrapAround = true;

  // This is a property that's set true prior to calling Show
  // if this is a replace dialog
  bEnableReplace = false;

  bFindFirst = true; // Set this to cause the search to reset

  // Tag property tells us which set of lockCounter, EditString,
  // EditLength and OldLength to access...
  EditFind->Tag = ID_FIND;
  EditReplace->Tag = ID_REPLACE;

  utils->SetOldLineVars(EditFind, true);
  utils->SetOldLineVars(EditReplace, true);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::FormCreate(TObject *Sender)
{
  InitCaptions(); // Just have to do this once...

  // Position to upper-left of YahCoLoRiZe
  int temp = dts->Left + dts->Width - this->Width;
  if (temp + this->Width > Screen->Width) temp = Screen->Width - this->Width;
  this->Left = temp;
  this->Top = dts->Top;

  bUnicodeKeySequence = false;

  FrReset(true); // Sets bUseSelected flag

  EditFind->HideScrollBars = false;
  EditReplace->HideScrollBars = false;

  EditFind->WordWrapMode = wwtNone;
  EditFind->WordWrap = false;

  EditFind->DefAttributes->BackColor = clWindow;
  EditReplace->DefAttributes->BackColor = clWindow;

  String sTemp;

  if (tae->SelLength && !bUseSelected)
  {
    sTemp = utils->MoveMainTextToString();

    if (tae->View != V_ORG && tae->View != V_IRC)
      sTemp = utils->StripAllCodes(sTemp);
  }
  else
    sTemp = dts->SaveFindText;

  EditString[ID_FIND] = sTemp; // Set initial FIND string
  EditString[ID_REPLACE] = dts->SaveReplaceText; // Set initial REPLACE string

  // Set the Design of the Form, Single or Multiline
  if (BothEditBoxesAreSingleLine())
    SetFormDesign(false); // Single-line mode
  else
    SetFormDesign(true); // Multi-line mode

  // Encode OutTxt (which is now in IRC raw-codes format) and put the
  // highlighted text in EditFind
  utils->EncodeHighlight(EditString[ID_FIND], EditFind);
  utils->EncodeHighlight(EditString[ID_REPLACE], EditReplace);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::FormShow(TObject *Sender)
// This is called after the Form is instantiated and AFTER the parent
// sets the EnableReplace property for a replace-dialog! It's called
// AFTER FormCreate(). Also - we hide and reshow the dialog during
// a replace-all operation... so this will get called again then!
{
  // Set checkboxes (properties may have been changed before Show!)
  CheckBoxWrapAround->Checked = bWrapAround;
  CheckBoxCaseSensitive->Checked = bMatchCase;
  CheckBoxWholeWord->Checked = bWholeWord;

  InitControls(); // Depends on bEnableReplace flag

  if (bEnableReplace && EditString[ID_REPLACE].Length() > 0 && EditString[ID_FIND].Length() > 0)
    EditReplace->SetFocus();
  else EditFind->SetFocus();

  SetFocusId();

  SetView(EditFind, V_IRC); // Enable OnChange

  // Do this in Show() to allow the EnableReplace property to be set!
  if (bEnableReplace)
  {
    if (utils->GetRealLength(ReplaceCodesStr) == 0 || !utils->TextContainsFormatCodes(ReplaceCodesStr))
      SetView(EditReplace, V_IRC);
    else SetView(EditReplace, V_RTF);
  }

  ChangeView();
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::InitCaptions(void)
{
  CheckBoxWrapAround->Caption = REPLACEMSG[5];
  CheckBoxCaseSensitive->Caption = REPLACEMSG[6];
  CheckBoxWholeWord->Caption = REPLACEMSG[7];
  ButtonFindUp->Caption = REPLACEMSG[9];
  ButtonFindDown->Caption = REPLACEMSG[10];
  ButtonReplace->Caption = REPLACEMSG[11];
  ButtonReplaceAll->Caption = REPLACEMSG[12];
  ButtonCancel->Caption = REPLACEMSG[13];
  LabelFind->Caption = REPLACEMSG[15];
  LabelReplaceWith->Caption = REPLACEMSG[16];
  LabelPressF3F4->Caption = REPLACEMSG[17];
  ButtonToggleView->Caption = REPLACEMSG[30];
  ButtonToggleView->Hint = REPLACEMSG[31];

  // Init Menus
  utils->LoadMenu(ReplaceDlgMainMenu, (char**)REPLACEDLGMAINMENU);
  utils->LoadMenu(ReplaceDlgPopupMenu, (char**)REPLACEDLGPOPUPMENU);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::InitControls(void)
{
  EditFind->Enabled = true;
  LabelPressF3F4->Visible = true;

  if (bEnableReplace)
  {
    ButtonToggleView->Visible = true;
    EditReplace->Visible = true;
    EditReplace->Enabled = true;
    LabelReplaceWith->Visible = true;
    ButtonReplace->Enabled = true;
    ButtonReplaceAll->Enabled = true;
    LabelViewMode->Enabled = true;
  }
  else
  {
    ButtonToggleView->Visible = false;
    EditReplace->Visible = false;
    EditReplace->Enabled = false;
    LabelReplaceWith->Visible = false;
    ButtonReplace->Enabled = false;
    ButtonReplaceAll->Enabled = false;
    LabelViewMode->Enabled = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ChangeView()
// Set view based on main-edit's view
{
  ChangeView(tae->View);
}

void __fastcall TReplaceTextForm::ChangeView(int View)
{
  // Initial EditReplace->View
  if (View == V_RTF) ViewRtf(EditReplace);
  else ViewIrc(EditReplace);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ViewRtf(TTaeRichEdit* re)
// Set EditReplace control to show RTF
{
  if (!bEnableReplace) return;

  int idx = re->Tag;

  if (idx != ID_FIND && idx != ID_REPLACE) return;

  utils->PushOnChange(re);

  MainMenuInsert->Enabled = true;

  // Set Cursor in rich-text to the same point as cursor in plain text
  int First;

  if (re->SelStart >= re->TextLength)
    First = -1; // set cursor to end
  else
    First = utils->GetRealIndex(EditString[idx], re->SelStart + re->Line);

  // avoid colors becoming Afg/ABg...
  utils->ConvertToRtf(EditString[idx], NULL, re, true);
  re->SelStart = First;

  SetView(re, V_RTF);

  utils->PopOnChange(re);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ViewIrc(TTaeRichEdit* re)
{
  if (!bEnableReplace)
    return;

  int idx = re->Tag;

  if (idx != ID_FIND && idx != ID_REPLACE)
    return;

  utils->PushOnChange(re);

  MainMenuInsert->Enabled = true;

  // Set EditReplace control to show IRC Codes
  int SaveSelStart = re->SelStart;
  int SaveSelLength = re->SelLength;
  int SaveLine = utils->GetLine(re);

  int OrigSelCRs = utils->CountCRs(re->SelTextW);

  // Set Cursor in plain-text to the same point as cursor in RTF text
  int iFirst, iLast, CI;
  utils->GetCodeIndices(EditString[idx], iFirst, iLast, CI,
                                              SaveSelStart, SaveSelLength);

  // Highlight special codes and load into edit-control
  utils->EncodeHighlight(EditString[idx], re);

  // Set Cursor in IRC text to the same point as cursor in RTF text...
  int temp = SaveLine;

  // (Handle user entering text at end of a line of RTF text...)
  if (CI > 0 && CI <= (int)re->TextLength)
  {
    if (re->Text[CI] == C_CR)
      temp++;

    SaveSelStart = CI-temp;
  }

  if (SaveSelLength)
    SaveSelLength = (iLast-CI)-OrigSelCRs;

  int NewTextLen = utils->GetTextLength(re);

  // Limit-checking...
  if (SaveSelStart >= NewTextLen)
  {
    SaveSelStart = -1;
    SaveSelLength = 0;
  }
  else if (SaveSelStart + SaveSelLength > NewTextLen)
    SaveSelLength = NewTextLen - SaveSelStart;

  // Set to beginning? Try to set to the line from the previous view
  if (!SaveSelStart)
    utils->SetLine(re, SaveLine);
  else
  {
    re->SelStart = SaveSelStart;
    re->SelLength = SaveSelLength;
  }

  SetView(re, V_IRC);

  utils->PopOnChange(re);
}
//---------------------------------------------------------------------------
WideString __fastcall TReplaceTextForm::GetFindStrippedStr(void)
// Strips out all format codes AND Line-Feed chars - leaves carrage return
// chars... Microsoft's RichEdit control can search over multiple lines
// separated by a CR only!
{
  // Do this to allow pasting of multiple lines of copied text
  // into EditFind - have to trim out the LF chars!
  String Stripped = utils->StripAllCodes(FindCodesStr);
  return utils->StripChar(Stripped, C_LF);
}
//---------------------------------------------------------------------------
WideString __fastcall TReplaceTextForm::GetReplaceStrippedStr(void)
{
  // Do this to allow pasting of multiple lines of copied text
  // into EditFind - have to trim out the LF chars!
  WideString Stripped = utils->StripAllCodes(ReplaceCodesStr);
  return utils->StripChar(Stripped, C_LF);
}
//---------------------------------------------------------------------------
int __fastcall TReplaceTextForm::GetFindStrippedLen(void)
{
  return utils->GetRealLength(FindCodesStr);
}
//---------------------------------------------------------------------------
int __fastcall TReplaceTextForm::GetReplaceStrippedLen(void)
{
  return utils->GetRealLength(ReplaceCodesStr);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ButtonReplaceClick(TObject *Sender)
{
  ReplaceText(false);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ButtonReplaceAllClick(TObject *Sender)
{
  ReplaceText(true);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ButtonFindDownClick(TObject *Sender)
{
  // Search forward
  FrFind(true);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ButtonFindUpClick(TObject *Sender)
{
  // Search backward
  FrFind(false);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::FrReset(bool bSetFindFirst)
// Public function, sets bSelected and sets the text range to the
// selected zone IF it contains one or more Line-Feed chars... otherwise
// we will search the entire document... also sets the dialog's title
// based on bSelected after it's set...
{
  if (bReplacing) return;

#if DEBUG_ON
      dts->CWrite("\r\nFRRESET CALLED!\r\n");
#endif

  if (bSetFindFirst) bFindFirst = true;

  if (tae->SelLength && utils->ContainsChar(tae->SelTextW, C_CR))
  {
    BeginningOfText = tae->SelStart;
    EndingOfText = BeginningOfText + tae->SelLength;
    bUseSelected = true;
  }
  else
  {
    BeginningOfText = 0;
    EndingOfText = tae->Text.Length();
    bUseSelected = false;
  }

  if (bUseSelected)
  {
    if (bEnableReplace)
      this->Caption = REPLACEMSG[0]; //"Find/Replace (selected text only)";
    else
      this->Caption = REPLACEMSG[1]; //"Find (selected text only)";
  }
  else
  {
    if (bEnableReplace)
      this->Caption = REPLACEMSG[2]; //"Find/Replace (entire document)";
    else
      this->Caption = REPLACEMSG[3]; //"Find (entire document)";
  }
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::FrClose(void)
{
  if (bReplacing) return;

#if DEBUG_ON
  dts->CWrite("\r\nFRCLOSE CALLED!\r\n");
#endif

  Close(); // Close now
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::FrFind(bool bForward)
// Called when the Find Up or Find Down buttons are pressed
{
  if (bReplacing || EditFind->LineCount == 0)
    return;

#if DEBUG_ON
  dts->CWrite("\r\nFRFIND CALLED!\r\n");
#endif

  // Init start-point of search - done on first button-press after
  // showing the dialog or if the user changes the search-text or check-boxes
  if (bFindFirst)
    InitCurrentFindPos(bForward);

  int RetVal = FindNext(bForward);

  if (RetVal == 0)
  {
    if (bWrapAround)
      utils->ShowMessageU(REPLACEMSG[20]); // Search complete!
    else
      utils->ShowMessageU(REPLACEMSG[19]); // Text not found!
  }
  else if (RetVal == 2)
    utils->ShowMessageU(REPLACEMSG[20]); // Search complete!
  else if (RetVal == -1)
    utils->ShowMessageU(REPLACEMSG[21]); // Error in search!
  else // in wordwrap you repeat-find the only word... so print message
  {
    if (PrevSelStart == tae->SelStart)
      utils->ShowMessageU(REPLACEMSG[20]); // Search complete!

    PrevSelStart = tae->SelStart;
  }

  bFindFirst = false;
}
//---------------------------------------------------------------------------
int __fastcall TReplaceTextForm::FrReplaceAll(WideString FindStr,
                                                      WideString ReplaceStr)
// Call this instead of Show() to do a replace-all without the dialog
// popping up!
{
#if DEBUG_ON
  dts->CWrite("\r\nFRREPLACEALL CALLED!\r\n");
#endif

  bEnableReplace = true;
  bWrapAround = true;
  bMatchCase = false;
  bWholeWord = true;

  EditString[ID_FIND] = FindStr;
  EditString[ID_REPLACE] = ReplaceStr;

  focusId = ID_FIND;
  return ReplaceAll();
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::InitCurrentFindPos(bool bForward)
{
  if (bUseSelected)
  {
    if (bForward)
      CurrentFindPos = BeginningOfText;
    else
      CurrentFindPos = EndingOfText;
  }
  else if (bForward)
    // Set caret past selected-area (if any)
    CurrentFindPos = tae->SelStart + tae->SelLength;
  else
    CurrentFindPos = tae->SelStart;

  PrevFindLength = 0;
  PrevSelStart = -1;
  prevDirection = DIR_UNSET;
}
//---------------------------------------------------------------------------
int __fastcall TReplaceTextForm::FindNext(bool bForward)
// returns -1 if error, 0 if not found, 1 if found, 2 if end of search
{
  if (EditFind->LineCount == 0)
    return 0;

  int RetVal;

  try
  {
    Flags.Clear();

    if (bWholeWord)
      Flags << st2WholeWord;

    if (bMatchCase)
      Flags << st2MatchCase;

    // st2Backward is set below...

    if (bForward)
    {
      if (prevDirection == DIR_REVERSE)
      {
        PrevSelStart = -1;
        PrevFindLength = 0;
      }

      if (CurrentFindPos >= EndingOfText)
      {
        if (bWrapAround)
          CurrentFindPos = BeginningOfText; // Back to top
        else
        {
          bFindFirst = true;
          return 2; // Search is complete
        }
      }

      prevDirection = DIR_FORWARD;
    }
    else // backward
    {
      if (prevDirection == DIR_FORWARD)
      {
        PrevSelStart = -1;
        PrevFindLength = 0;
      }

      if (CurrentFindPos <= BeginningOfText) // reverse
      {
        if (bWrapAround)
          CurrentFindPos = EndingOfText; // Back to bottom
        else
        {
          bFindFirst = true;
          return 2; // Search is complete
        }
      }

      prevDirection = DIR_REVERSE;
      Flags << st2Backward;
    }

    // Diagnostic
    //ShowMessage("Start:" + String(CurrentFindPos) + ", Length:" +
    //                                                         String(Length));

    // Special cases: 1) The search string has no codes and we are in
    // V_IRC or V_ORG... we want to search the RTF view!
    // 2) The search-string has codes and we are in V_RTF in the main edit,
    // we need to switch to V_IRC...
    // (Don't attempt to then switch back... big mess and unnecessary!)
    if (utils->TextContainsFormatCodes(FindCodesStr))
    {
      if (tae->View == V_RTF)
        dts->LoadView(V_IRC);
    }
    else if (tae->View == V_ORG || tae->View == V_IRC)
      dts->LoadView(V_RTF);

    // Choose the memory-stream with raw-data (if any)
    TStringsW* sl;

    if (tae->View == V_IRC)
      sl = dts->SL_IRC;
    else if (tae->View == V_ORG)
      sl = dts->SL_ORG;
    else sl = NULL;

    // Choose the string to look for, with or without format-codes...
    String FindStr;
    if (tae->View == V_ORG || tae->View == V_IRC)
      FindStr = FindCodesStr;
    else
      FindStr = FindStrippedStr;

    bool bRet = FindOne(FindStr, sl);

    if (bRet)
    {
      // Found! Text we found has already been highlighted
      PrevFindLength = tae->SelLength;

      if (bForward)
        CurrentFindPos = tae->SelStart+1;
      else
        CurrentFindPos = tae->SelStart + PrevFindLength - 1; // Search backward

      // Relocate this dialog if it's on top of the new selection!
      if (this->Visible)
        utils->RelocateDialog(this);

      RetVal = 1; // Found
    }
    else // Not found
    {
      if (bWrapAround)
      {
        if (bForward)
          CurrentFindPos = BeginningOfText;
        else
          CurrentFindPos = EndingOfText;

        // Call ourself, avoiding infinite loop if not found again!
        bool bSaveWrapAround = bWrapAround;
        bWrapAround = false;
        RetVal = FindNext(bForward); // Wrap around and look again

        bWrapAround = bSaveWrapAround;
      }
      else
      {
        if (PrevFindLength)
          RetVal = 2; // Search is complete if we had a previous find
        else
          RetVal = 0; // Text not found!

        bFindFirst = true;
      }
    }
  }
  catch(...)
  {
    RetVal = -1; // Error
    bFindFirst = true;
  }

  return RetVal;
}
//---------------------------------------------------------------------------
bool __fastcall TReplaceTextForm::FindOne(WideString FindStr, TStringsW* sl)
// This is called to do a find one for all views. Set ms NULL for
// V_HTML and V_RTF_SOURCE (or any text with no "raw-codes" memory-stream
// association). Returns true if found. The text found will be selected
// so use SelStart and SelLength to locate it. The search begins at
// CurrentFindPos. It searches backward if st2Backward is set in Flags.
{
  utils->PushOnChange(tae);

  // Save original position and carat
  TTaePosition* p = new TTaePosition(tae);
  p->SavePos = p->Position;

  int bRet;

  int SearchLength;

  if (Flags.Contains(st2Backward))
    SearchLength = CurrentFindPos-BeginningOfText;
  else
    SearchLength = EndingOfText-CurrentFindPos;

  bool bUseSL = tae->View != V_RTF && sl != NULL ? true : false;

  if (bUseSL) // Load edit-control with entire string-list's data
    tae->TextW = sl->Text;

  // Search in a TaeRichEdit requires cr line-termination, not cr/lf!
  FindStr = utils->StripChar(FindStr, C_LF);

  // Find the first match.
  int FoundPos; // returned start position
  bRet = tae->FindTextExW(FindStr, CurrentFindPos,
                                Flags, FoundPos, SearchLength);

  if (bUseSL) // Highlight the raw codes
    utils->EncodeHighlight(tae);

  // Restore original position and carat
  p->Position = p->SavePos;
  delete p;

  if (bRet)
  {
    tae->SelStart = FoundPos;
    tae->SelLength = SearchLength;
  }

  utils->PopOnChange(tae);

  return bRet;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ReplaceText(bool bReplaceAll)
// Called when the Replace or Replace All button is clicked from
// the Replace Dialog. OnReplace Event Handler.
{
  if (bReplacing)
    return;

  if (bReplaceAll)
  {
    int Count = ReplaceAll();

    // Tell the user what we did.
    // "Finished searching text.\r\n" X " replacements made."
    utils->ShowMessageU(REPLACEMSG[24] + String(Count) + REPLACEMSG[25]);

    // Set focus back to the main edit-control so the carat can blink
    // (if the replace-text is "nothing" you can't tell where we are...)
    tae->SetFocus();
  }
  else if (!ReplaceSelected(ReplaceCodesStr))
    ShowMessage("Error in Replace!"); // Error in replace!
  else // just successfully replaced the current selected chunk of text
  {
    CurrentFindPos = tae->SelStart + 1;

    // Auto-find next...
    if (prevDirection == DIR_REVERSE)
      FrFind(false);
    else
      FrFind(true);

    // Set focus back to the main edit-control so the caret can blink
    // (if the replace-text is "nothing" you can't tell where we are...)
    tae->SetFocus();
  }
}
//---------------------------------------------------------------------------
bool __fastcall TReplaceTextForm::ReplaceSelected(WideString S)
// Called when the Replace button is clicked after finding and
// selecting the word or phrase to be replaced. Also called
// from ReplaceAll if V_RTF...
//
// Returns true if success
{
  if (!tae->SelLength) return true; // Don't report error

  try
  {
    // Replace the found text with the new text.
    int deltaLen;
    if (!SingleReplace(S, deltaLen))
      return false;

    // Add or subtract from selected-zone's range (the difference
    // between what was cut and what was pasted)
    EndingOfText += deltaLen;
    // used in FindNext to advance CurrentFindPos over the text
    // we just pasted...
    PrevFindLength += deltaLen;

    return true; // Success
  }
  catch(...) {return false;}
}
//---------------------------------------------------------------------------
bool __fastcall TReplaceTextForm::SingleReplace(WideString ReplaceStr,
                                                            int &deltaLen)
// Returns true if no errors and with deltaLen returned as a reference
// drltaLen is the +/- length-change between the old text cut and the
// new text pasted.
//
// NOTE: The routines here, as called, don't use the clipboard! And - there
// are no shortcuts... the cut/paste functions in main.cpp are complex and
// handle the proper merging of colors and bold, underline and italic codes!
{
  deltaLen = 0;

  try
  {
    // Cut highlighted text and & paste new text
    PASTESTRUCT ps = dts->TaeEditPaste(true, ReplaceStr);

    // After replace, which might have new colors - deselect the replaced
    // text so the new colors can me seen - otherwise the select-colors
    // make it look like the wrong color is there...
    tae->SelLength = 0;

    if (ps.error < 0)
      return false;

    deltaLen = ps.delta; // Return +/- delta which includes cr/lf

    return true;
  }
  catch(...) {return false;}
}
//---------------------------------------------------------------------------
int __fastcall TReplaceTextForm::ReplaceAll(void)
// Called when the Replace or Replace All button is clicked from
// the Replace Dialog. OnReplace Event Handler.
//
// NOTE: DO NOT OPTIMIZE HERE (because it messes up Undo!)
{
  // Ignore if already running
  if (bReplacing || FindCodesStr.IsEmpty())
    return 0;

  // If user clicked Replace All, then do find/replace
  // operation until no more matches are found.

  // Set up the options (Forward search, default).
  Flags.Clear();
  if (bWholeWord) Flags << st2WholeWord;
  if (bMatchCase) Flags << st2MatchCase;

  if (EndingOfText-BeginningOfText <= 0)
  {
    ShowMessage("No text to search/replace!");
    return 0;
  }

  bReplacing = true; // Flag

  // "Replacing text..."
  dts->CpShow(REPLACEMSG[23], DS[70]);

  int Count;

  if (tae->View == V_RTF || tae->View == V_IRC)
    Count = ReplaceAll(dts->SL_IRC);
  else if (tae->View == V_ORG)
    Count = ReplaceAll(dts->SL_ORG);
  else
    Count = ReplaceAll(NULL); // V_RTF_SOURCE or V_HTML

  dts->CpHide();

  bReplacing = false; // Flag

  return Count;
}
//---------------------------------------------------------------------------
int __fastcall TReplaceTextForm::ReplaceAll(TStringsW* sl)
// This is called to do a find/replace all for all views. Set ms NULL for
// V_HTML and V_RTF_SOURCE (or any text with no "raw-codes" memory-stream
// association).
{
  utils->PushOnChange(tae);

  int Count = 0;

  // Shortcut vars to speed things up (stripped properties take time to execute!)
  int rsl = ReplaceStrippedLen;
  WideString rss = ReplaceStrippedStr;
  int fsl = FindStrippedLen;
  WideString fss = FindStrippedStr;

  dts->CpInit(1); // init progress bars

  if (sl == NULL) // Search replace for V_HTML and V_RTF_SOURCE
  {
    // Find the first match.
    int OrigLen = EndingOfText-BeginningOfText;
    int Length = OrigLen; // (also the returned length)
    int FoundPos; // returned start position

    int deltaLen = rsl-fsl;

    // Search in a TaeRichEdit requires cr line-termination, not cr/lf!
    fss = utils->StripChar(fss, C_LF);

    bool bRet = tae->FindTextExW(fss, BeginningOfText, Flags, FoundPos, Length);

    while (GetAsyncKeyState(VK_ESCAPE) < 0); // Dummy read to clear...
    bAbort = false;

    dts->CpSetMaxIterations(Length); // No message just main progress bar

    // Loop while we are still finding text.
    // (bAbort is a global that's set by the Replace Dialog's OnClose
    // event - need to call ProcessMessages in the loop for this to work!)
    while (bRet)
    {
      // User abort???
      Application->ProcessMessages();
      if ((int)GetAsyncKeyState(VK_ESCAPE) < 0 || bAbort) break; // Cancel

      tae->SelStart = FoundPos;
      tae->SelLength = Length;

      // Add an undo including the text we will remove and the length of
      // the text we will put in it's place
      ONCHANGEW oc = utils->GetInfoOC(tae, NULL);
      TOCUndo->Add(UNDO_REPLACE, FoundPos, rsl, oc, tae->SelTextW);

      // Perform replace
      tae->SelTextW = rss; // advances SelStart past new word!

      // original zone is growing or shrinking dynamically
      EndingOfText += deltaLen;
      Length = EndingOfText-tae->SelStart;

      if (Length <= 0)
        break;

      // Advance progress-bar
      dts->CpMaxIterations = Length;
      dts->CpUpdate(FoundPos-BeginningOfText);

      // Do another search.
      bRet = tae->FindTextExW(fss, tae->SelStart, Flags, FoundPos, Length);

      // Increment the counter so we can tell the user
      // how many replacements we made.
      Count++;
    }
  }
  else // Main.cpp's TaeEdit has a string-list in V_RTF, V_ORG or V_IRC view
  {
    int deltaLen;
    WideString FindStr;

    // Load edit-control with stream's data
    if (tae->View == V_RTF)
    {
      // Use stripped lengths if search is happening in RTF view!
      // (no visible codes)
      deltaLen = rsl-fsl;
      FindStr = fss;
    }
    else
    {
      tae->TextW = sl->Text;
      deltaLen = ReplaceCodesStr.Length()-FindCodesStr.Length();
      FindStr = FindCodesStr;
    }

    // Find the first match.
    int Length = EndingOfText - BeginningOfText; // (also the returned length)
    int FoundPos; // returned start position

    // Search in a TaeRichEdit requires cr line-termination, not cr/lf!
    FindStr = utils->StripChar(FindStr, C_LF);

    // Calling this function returns FoundPos and its Length as references!
    bool bRet = tae->FindTextExW(FindStr, BeginningOfText, Flags, FoundPos, Length);

    while (GetAsyncKeyState(VK_ESCAPE) < 0); // Dummy read to clear...
    bAbort = false;

    dts->CpSetMaxIterations(Length);

    // Loop while we are still finding text.
    // (bAbort is a global that's set by the Replace Dialog's OnClose
    // event - need to call ProcessMessages in the loop for this to work!)
    while (bRet)
    {
      // User abort???
      Application->ProcessMessages();
      if ((int)GetAsyncKeyState(VK_ESCAPE) < 0 || bAbort) break; // Cancel

      tae->SelStart = FoundPos;
      tae->SelLength = Length;

      if (tae->View != V_RTF)
      {
        // Add an undo including the text we will remove and the length of
        // the text we will put in it's place
        ONCHANGEW oc = utils->GetInfoOC(tae, NULL);
        TOCUndo->Add(UNDO_REPLACE, FoundPos, ReplaceCodesStr.Length(), oc, tae->SelTextW);

        tae->SelTextW = ReplaceCodesStr;
        EndingOfText += deltaLen;
      }
      else
      {
        bool bRepRet = ReplaceSelected(ReplaceCodesStr);
        if (bRepRet) tae->SelStart += rsl;
        else break;
      }

      // temporary "codes" zone is growing or shrinking dynamically
      Length = EndingOfText-tae->SelStart;

      if (Length <= 0) break;

      // Advance progress-bar
      dts->CpMaxIterations = Length;
      dts->CpUpdate(FoundPos-BeginningOfText);

      // Do another search.
      bRet = tae->FindTextExW(FindStr, tae->SelStart, Flags, FoundPos, Length);

      // Increment the counter so we can tell the user
      // how many replacements we made.
      Count++;
    }

    // Save old caret position
    int SaveSelStart = tae->SelStart;

    if (tae->View == V_RTF)
      // Convert StringList to RTF and display it in the edit-window
      utils->ConvertToRtf(sl, NULL, tae, true);
    else
    {
      // Write modified text back to the memory stream
      sl->Text = tae->TextW;
      // Highlight the raw codes
      utils->EncodeHighlight(tae);
    }

    tae->SelStart = SaveSelStart;
  }

  if (Count)
    tae->Modified = true;

  utils->PopOnChange(tae);

  return Count;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::CheckBoxWrapAroundClick(TObject *Sender)
{
  bWrapAround = CheckBoxWrapAround->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::CheckBoxCaseSensitiveClick(TObject *Sender)
{
  bMatchCase = CheckBoxCaseSensitive->Checked;
  bFindFirst = true;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::CheckBoxWholeWordClick(TObject *Sender)
{
  bWholeWord = CheckBoxWholeWord->Checked;
  bFindFirst = true;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ButtonCancelClick(TObject *Sender)
// Stop replace-loops then click Cancel again to close
{
  if (bReplacing)
    bAbort = true;
  else
    Close(); // Close now
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
  // Save the search strings...
  dts->SaveFindText = FindCodesStr;
  dts->SaveReplaceText = ReplaceCodesStr;

  ReplaceTextForm = NULL;
  this->Release(); // We cease to exist now!
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if (Key == VK_F3)
  {
    FrFind(true);
    Key = 0;
  }
  else if (Key == VK_F4)
  {
    FrFind(false);
    Key = 0;
  }
  else if (Key == VK_RETURN)
  {
    if (focusId == ID_NONE)
    {
      // "Click the find or replace box to edit it..."
      utils->ShowMessageU(REPLACEMSG[28]);
      return;
    }

    TTaeRichEdit* re;
    GetFocusedCtrl(re);

    // This little trick keeps a phantom space from appearing at the end
    // of each new-line...
    re->SelAttributes->BackColor = clWindow;
  }
}
//---------------------------------------------------------------------------
// EDIT CONTROL KEY HANDLERS
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::EditKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
  TTaeRichEdit* re = dynamic_cast<TTaeRichEdit*>(Sender);
  if (!re) return;

  // Want Paste even if no text!
  // this allows shift+Ins to paste but blocks turning overstrike-mode on!
  if (Key == VK_INSERT)
  {
    if (Shift.Contains(ssShift))
      // Shift+Ins
      EditPaste();
    Key = 0;
  }

  if (re->LineCount == 0)
    return;

  if (Key == VK_DELETE)
  {
    if (re->SelLength)
    {
      if (Shift.Contains(ssShift))
        EditCutCopy(true, true); // Shift+Del
      else
        EditCutCopy(true, false); // Del

      Key = 0;
    }
  }
  // Here, we want to see if the next OnChange from TaeRichEdit
  // is a deltaLength of -1 to -4 chars... that means the user typed in
  // "fe" and hit Alt-X or typed in "0abcd" and hit Alt-x and those chars
  // were replaced by a single Unicode char.
  else if (this->bUnicodeKeySequence)
    this->bUnicodeKeySequence = false;
  else if (Key == 'X' && Shift.Contains(ssAlt))
    this->bUnicodeKeySequence = true;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::EditKeyPress(TObject *Sender, char &Key)
{
  TTaeRichEdit* re = dynamic_cast<TTaeRichEdit*>(Sender);
  if (!re) return;

  if (isalnum(Key))
  {
    if (re->SelAttributes->Color != clBlack)
      re->SelAttributes->Color = clBlack;
    if (re->SelAttributes->BackColor != clWhite)
      re->SelAttributes->BackColor = clWhite;
  }
  else if (Key == C_TAB) // Replace tabs with spaces on-the-fly!
  {
    PASTESTRUCT ps = EditPaste(utils->GetTabStringW(dts->RegTabs));

    if (ps.error == 0)
      re->SelStart += ps.delta-ps.lines;

    Key = 0;
  }
}
//---------------------------------------------------------------------------
// Button Clicks (to the right of edit boxes)
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ButtonClearFindClick(TObject *Sender)
{
  HClear(EditFind);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ButtonClearReplaceClick(TObject *Sender)
{
  HClear(EditReplace);
}
//---------------------------------------------------------------------------
// Main Menu Edit Handlers
// (Note: some handlers are the  same as the popup-menu (cut/copy for example)
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::MainMenuEditClearClick(TObject *Sender)
{
  TTaeRichEdit* re;
  GetFocusedCtrl(re);
  HClear(re);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::Undo1Click(TObject *Sender)
{
  TTaeRichEdit* re;
  GetFocusedCtrl(re);
  HUndo(re);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::MainMenuEditSelectAllClick(TObject *Sender)
{
  TTaeRichEdit* re;
  GetFocusedCtrl(re);
  HSelectAll(re);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::MainMenuEditClick(TObject *Sender)
{
  TTaeRichEdit* re;
  GetFocusedCtrl(re);
  HMenuPopup(re);
}
//---------------------------------------------------------------------------
// Popup (right-click) menu handlers for either edit-control
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::MenuEditClearClick(TObject *Sender)
{
  TComponent* p = ReplaceDlgPopupMenu->PopupComponent;
  if (p != NULL && p->ClassNameIs("TTaeRichEdit"))
    HClear((TTaeRichEdit*)p);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::MenuEditUndoClick(TObject *Sender)
{
  TComponent* p = ReplaceDlgPopupMenu->PopupComponent;
  if (p && p->ClassNameIs("TTaeRichEdit"))
    HUndo((TTaeRichEdit*)p);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::MenuEditCutClick(TObject *Sender)
// same handler for both main and popup menus!
{
  EditCutCopy(true, true); // Cut
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::MenuEditCopyClick(TObject *Sender)
// same handler for both main and popup menus!
{
  EditCutCopy(false, true); // Copy
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::MenuEditPasteColorClick(TObject *Sender)
// Keeps color-codes and highlights
// same handler for both main and popup menus!
{
  EditPaste();
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::MenuEditPastePlainClick(TObject *Sender)
// Strips color-codes
// same handler for both main and popup menus!
{
  EditPaste(utils->StripAllCodes(utils->GetClipboardText()));
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::MenuEditSelectAllClick(TObject *Sender)
{
  TComponent* p = ReplaceDlgPopupMenu->PopupComponent;
  if (p && p->ClassNameIs("TTaeRichEdit")) HSelectAll((TTaeRichEdit*)p);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ButtonToggleViewClick(TObject *Sender)
// Handler for MenuEditToggleView, ButtonToggleView and
// MainMenuEditToggleView
{
  if (EditReplace->View == V_RTF)
    ChangeView(V_IRC);
  else if (ReplaceStrippedLen > 0)
    ChangeView(V_RTF);
  else
    utils->ShowMessageU(REPLACEMSG[26]); // Replace text has only format codes!
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::MenuEditOptimizeClick(TObject *Sender)
// Handler for MenuEditOptimize and MainMenuEditOptimize
{
  TTaeRichEdit* re; int idx;
  GetFocusedCtrl(re, idx);
  utils->PushOnChange(re);
  EditString[idx] = utils->Optimize(EditString[idx], false);
  re->SelStart = 0;
  re->Modified = true;
  ChangeView(re->View);
  utils->PopOnChange(re);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ReplaceDlgPopupMenuPopup(TObject *Sender)
// Don't let user paste color into EditFind if V_RTF
{
  TComponent* p = ReplaceDlgPopupMenu->PopupComponent;
  if (p && p->ClassNameIs("TTaeRichEdit")) HMenuPopup((TTaeRichEdit*)p);
}
//---------------------------------------------------------------------------
// Common Edit Menu Handlers for both popup and main menu
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::HClear(TTaeRichEdit* re)
{
  // Check the index from re->Tag (either ID_FIND or ID_REPLACE)
  int idx = re->Tag;
  if (idx != ID_FIND && idx != ID_REPLACE) return;

  utils->PushOnChange(re);
  re->Clear();
  EditString[idx] = "";
  
  if (BothEditBoxesAreSingleLine()) SetFormDesign(false); // Set form to single-line mode

  // Failsafe in case of unmatched push/pops!
  utils->InitOnChange(re, EditChange);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::HUndo(TTaeRichEdit* re)
{
  re->Undo();
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::HSelectAll(TTaeRichEdit* re)
{
  utils->PushOnChange(re);
  re->SelStart = 0;
  re->SelLength = utils->GetTextLength(re);
  utils->PopOnChange(re);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::HMenuPopup(TTaeRichEdit* re)
{
  WideString Temp = utils->GetClipboardText();

  if (!Temp.Length())
  {
    MenuEditPasteColor->Enabled = false;
    MenuEditPastePlain->Enabled = false;
    MainMenuEditPasteColor->Enabled = false;
    MainMenuEditPastePlain->Enabled = false;
  }
  else // have clipboard text
  {
    if (utils->TextContainsFormatCodes(Temp))
    {
      bool bCodesOnly = utils->GetRealLength(Temp) == 0 ? true : false;

      if (bCodesOnly)
      {
        MenuEditPastePlain->Enabled = false;
        MainMenuEditPastePlain->Enabled = false;
      }
      else
      {
        MenuEditPastePlain->Enabled = true;
        MainMenuEditPastePlain->Enabled = true;
      }

      MenuEditPasteColor->Enabled = true;
      MainMenuEditPasteColor->Enabled = true;
    }
    else
    {
      MenuEditPastePlain->Enabled = true;
      MenuEditPasteColor->Enabled = false;
      MainMenuEditPastePlain->Enabled = true;
      MainMenuEditPasteColor->Enabled = false;
    }
  }

  // Don't allow View toggle in EditFind
  if (re->Name == "EditFind")
  {
    MenuEditToggleView->Enabled = false;
    MainMenuEditToggleView->Enabled = false;
  }
  else
  {
    MenuEditToggleView->Enabled = true;
    MainMenuEditToggleView->Enabled = true;
  }

  // Need to focus the control on right-click!
  re->SetFocus();
}
//---------------------------------------------------------------------------
// MENU CLICKS FOR INSERTING INTO EDIT-BOX
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::ColorCode1Click(TObject *Sender)
{
  if (focusId == ID_NONE)
  {
    // "Click the find or replace box to edit it..."
    utils->ShowMessageU(REPLACEMSG[28]);
    return;
  }

  int fg, bg;
  bool bHaveFg = false;
  bool bHaveBg = false;

  Application->CreateForm(__classid(TSetColorsForm), &SetColorsForm);
  SetColorsForm->Mode = EM_FGBG;
  SetColorsForm->ShowModal();

  if (SetColorsForm->ModalResult == mrCancel)
  {
    SetColorsForm->Release();
    SetColorsForm = NULL;
    return;
  }

  fg = SetColorsForm->FgColor;
  bg = SetColorsForm->BgColor;
  SetColorsForm->Release();
  SetColorsForm = NULL;

  if (fg != NO_COLOR && fg != IRCNOCOLOR)
    bHaveFg = true;

  if (bg != NO_COLOR && bg != IRCNOCOLOR)
    bHaveBg = true;

  if (!bHaveFg && !bHaveBg)
    return;

  fg = utils->ConvertColor(fg, false);
  bg = utils->ConvertColor(bg, false);

  WideString ColorFmt;

  // Write the color-codes into a string
  utils->WriteColors(fg, bg, ColorFmt);

  TTaeRichEdit* re; int idx;
  GetFocusedCtrl(re, idx);

  utils->PushOnChange(re);

  // Save carat position
  int SaveSelStart = re->SelStart;

  // Inserting a code into a box with no printable chars? switch to codes view!
  if (re->View != V_IRC && utils->GetRealLength(re->TextW) == 0)
    ViewIrc(re);

  if (re->View == V_RTF)
  {
    if (!re->SelLength) // if no text selected...
    {
      // Strip existing color-codes
      if (bHaveFg && bHaveBg)
      {
        int len = EditString[idx].Length();
        EditString[idx] = utils->StripColorCodes(EditString[idx], 0, len);
      }
      else if (bHaveFg) EditString[idx] = utils->StripFgCodes(EditString[idx]);
      else EditString[idx] = utils->StripBgCodes(EditString[idx]); // bg only

      // Map the start and end indices of the RTF text to the
      // raw text in EditFindIRCString.
      int iFirst = utils->GetCodeIndex(re);

      if (iFirst < 0)
      {
        utils->PopOnChange(re);
        return;
      }

      // Write the colors imediately before the first printable character.
      EditString[idx] = utils->InsertW(EditString[idx], ColorFmt, iFirst+1);
    }
    else // text is selected...
    {
      // Set the cumulative text state that exists at the first character
      // in EditFindIRCString.
      PUSHSTRUCT BeginningState;
      utils->SetStateFlags(EditString[idx], STATE_MODE_FIRSTCHAR, BeginningState);

      // Write default color(s) at beginning if the wing has no default
      // color(s) and the user is trying to add a foreground or
      // background color somewhere in the wing.
      //
      // (reasoning: if we just added, say a background color to a
      // selected block of text in a wing with no background color
      // initially defined at the beginning of the wing, then there
      // is no background color to revert to at the end of the selected
      // text and the background color will be "stuck on" past the
      // selected zone. The same situation exists for the foreground color.)
      WideString InitColorFmt;

      // Set the logic...
      bool bFgWrite = (bHaveFg && (BeginningState.fg == NO_COLOR));
      bool bBgWrite = (bHaveBg && (BeginningState.bg == NO_COLOR));

      // Write the color-codes into a string
      if (bFgWrite && bBgWrite) utils->WriteColors(IRCBLACK, IRCWHITE, InitColorFmt);
      else if (bFgWrite) utils->WriteSingle(IRCBLACK, InitColorFmt, true);
      else utils->WriteSingle(IRCWHITE, InitColorFmt, false); // bg only

      // Write the colors at the start of the string.
      EditString[idx] = utils->InsertW(EditString[idx], InitColorFmt, 1);

      // Map the start and end indices of the RTF selected text to the
      // text in the buffer
      int iFirst, iLast, CI;
      if (!utils->GetCodeIndices(EditString[idx], iFirst, iLast, CI, re))
      {
        utils->PopOnChange(re);
        return;
      }

      // Set the cumulative text state that exists at the character
      // following the last highlighted character...
      int LastRealIndex = re->SelStart+re->SelLength;

      PUSHSTRUCT EndState;
      utils->SetStateFlags(EditString[idx], LastRealIndex, EndState);

      int len = ColorFmt.Length();

      // Insert the new color(s) in front of the first highlighted character.
      EditString[idx] = utils->InsertW(EditString[idx], ColorFmt, iFirst+1);
      iFirst += len;
      iLast += len;

      // Strip out color codes in the highlighted range.  "Last" may return
      // shorted by the # of color-code chars removed.
      EditString[idx] = utils->StripColorCodes(EditString[idx], iFirst, iLast);

      // Insert ColorFmt after every CRLF in the selected range...
      for (int ii = iFirst+1; ii <= iLast; ii++)
      {
        if (EditString[idx][ii] == C_LF)
        {
          EditString[idx] = utils->InsertW(EditString[idx], ColorFmt, ii+1);
          ii += len;
          iLast += len;
        }
      }

      // Write the color codes pertaining to the end state.
      ColorFmt = ""; // Must clear to avoid appending to it!
      utils->WriteColors(EndState.fg, EndState.bg, ColorFmt);

      // Insert the color-string in front of the character imediately
      // following the last highlighted char.
      EditString[idx] = utils->InsertW(EditString[idx], ColorFmt, iLast+1);
    }

    // Convert to RTF and display it in the edit-window
    utils->ConvertToRtf(EditString[idx], NULL, re, true);
  }
  else // re->View is V_IRC
    IrcInsert(re, ColorFmt, String(CTRL_K));

  re->SelStart = SaveSelStart;
  re->Modified = true;
  utils->SetOldLineVars(re);
  utils->PopOnChange(re);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::FontTypeCode1Click(TObject *Sender)
{
  if (focusId == ID_NONE)
  {
    // "Click the find or replace box to edit it..."
    utils->ShowMessageU(REPLACEMSG[28]);
    return;
  }

  int Type = 0;

  if (!dts->SelectFontType(String(DS[213]), Type)) return; // No font-code?

  // Write the font code pertaining to the end state.
  WideString FontStr = utils->FontTypeToString(Type);

  TTaeRichEdit* re; int idx;
  GetFocusedCtrl(re, idx);

  utils->PushOnChange(re);

  // Save carat position
  int SaveSelStart = re->SelStart;

  // Inserting a code into a box with no printable chars? switch to codes view!
  if (re->View != V_IRC && utils->GetRealLength(re->TextW) == 0)
    ViewIrc(re);

  if (re->View == V_IRC)
    IrcInsert(re, FontStr, String(CTRL_F));
  else // re->View is V_RTF
  {
    if (re->SelLength == 0) // Text selected?
    {
      // If not, write the font-code at the cursor.
      if (re->SelStart >= 0)
      {
        int iTemp = utils->GetCodeIndex(re->TextW, re->SelStart);

        if (iTemp >= 0)
          EditString[idx] = utils->InsertW(EditString[idx], FontStr, iTemp+1);
      }
      else
        EditString[idx] = utils->InsertW(EditString[idx], FontStr, 1);
    }
    else // Text selected
    {
      // Set the cumulative text state that exists at the first character
      // in EditString[idx].
      PUSHSTRUCT BeginningState;
      utils->SetStateFlags(EditString[idx], STATE_MODE_FIRSTCHAR, BeginningState);

      // Write default font-type at beginning if the wing has no default
      // font-type and the user is trying to add a font-type code
      // somewhere in the wing.
      if (BeginningState.fontType == -1)
      {
        WideString InitFontFmt = utils->FontTypeToString(-1);
        EditString[idx] = utils->InsertW(EditString[idx], InitFontFmt, 1);
      }

      // Map the start and end indices of the RTF selected text to the
      // text in the buffer
      int iFirst, iLast, CI;
      if (!utils->GetCodeIndices(EditString[idx], iFirst, iLast, CI, re))
      {
        utils->PopOnChange(re);
        return;
      }

      // Set the cumulative text state that exists up to the character
      // following the last highlighted character...
      int LastRealIndex = re->SelStart+re->SelLength;
      PUSHSTRUCT EndState;
      utils->SetStateFlags(EditString[idx], LastRealIndex, EndState);

      int len = FontStr.Length();

      // Insert the new font-code in front of the first highlighted character.
      EditString[idx] = utils->InsertW(EditString[idx], FontStr, iFirst+1);
      iFirst += len;
      iLast += len;

      // Strip out font codes in the highlighted range.  "Last" may return
      // shorted by the # of font-code chars removed.
      EditString[idx] = utils->StripFont(EditString[idx], iFirst, iLast, CTRL_F);

      // Insert FontStr after every CRLF in the selected range...
      for (int ii = iFirst+1; ii <= iLast; ii++)
      {
        if (EditString[idx][ii] == C_LF)
        {
          EditString[idx] = utils->InsertW(EditString[idx], FontStr, ii+1);
          ii += len;
          iLast += len;
        }
      }

      // Write the font code pertaining to the end state.
      if (EndState.fontType < 0)
      {
        utils->PopOnChange(re);
        return;
      }

      // Insert the font-string in front of the character imediately
      // following the last highlighted char.
      WideString Stemp = utils->FontTypeToString(EndState.fontType);
      EditString[idx] = utils->InsertW(EditString[idx], Stemp, iLast+1);
    }

    // Convert to RTF and display it in the edit-window
    utils->ConvertToRtf(EditString[idx], NULL, re, true);
  }

  re->SelStart = SaveSelStart;
  re->Modified = true;
  utils->SetOldLineVars(re);
  utils->PopOnChange(re);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::FontSizeCode1Click(TObject *Sender)
{
  if (focusId == ID_NONE)
  {
    // "Click the find or replace box to edit it..."
    utils->ShowMessageU(REPLACEMSG[28]);
    return;
  }

  int Size = 0;

  if (!dts->SelectFontSize(String(DS[214]), Size)) return; // no font-code

  // Write the font code pertaining to the end state.
  WideString FontStr = utils->FontSizeToString(Size);

  TTaeRichEdit* re; int idx;
  GetFocusedCtrl(re, idx);

  utils->PushOnChange(re);

  // Save carat position
  int SaveSelStart = re->SelStart;

  // Inserting a code into a box with no printable chars? switch to codes view!
  if (re->View != V_IRC && utils->GetRealLength(re->TextW) == 0)
    ViewIrc(re);

  if (re->View == V_IRC)
    IrcInsert(re, FontStr, String(CTRL_S));
  else // re->View is V_RTF
  {
    if (!re->SelLength) // Text selected?
    {
      // If not, write the font-code at the cursor.
      if (re->SelStart >= 0)
      {
        int iTemp = utils->GetCodeIndex(re->TextW, re->SelStart);

        if (iTemp >= 0)
          EditString[idx] = utils->InsertW(EditString[idx], FontStr, iTemp+1);
      }
      else
        EditString[idx] = utils->InsertW(EditString[idx], FontStr, 1);
    }
    else // Text selected
    {
      // Set the cumulative text state that exists at the first character
      // in EditString[idx].
      PUSHSTRUCT BeginningState;
      utils->SetStateFlags(EditString[idx], STATE_MODE_FIRSTCHAR, BeginningState);

      // Write default font-size at beginning if the wing has no default
      // font-size and the user is trying to add a font-size code
      // somewhere in the wing.
      if (BeginningState.fontSize == -1)
      {
        WideString InitFontFmt = utils->FontSizeToString(-1);
        EditString[idx] = utils->InsertW(EditString[idx], InitFontFmt, 1);
      }

      // Map the start and end indices of the RTF selected text to the
      // text in the buffer
      int iFirst, iLast, CI;
      if (!utils->GetCodeIndices(EditString[idx], iFirst, iLast, CI, re))
      {
        utils->PopOnChange(re);
        return;
      }

      // Set the cumulative text state that exists up to the character
      // following the last highlighted character...
      int LastRealIndex = re->SelStart+re->SelLength;
      PUSHSTRUCT EndState;
      utils->SetStateFlags(EditString[idx], LastRealIndex, EndState);

      int len = FontStr.Length();

      // Insert the new font-code in front of the first highlighted character.
      EditString[idx] = utils->InsertW(EditString[idx], FontStr, iFirst+1);
      iFirst += len;
      iLast += len;

      // Strip out font codes in the highlighted range.  "Last" may return
      // shorted by the # of font-code chars removed.
      EditString[idx] = utils->StripFont(EditString[idx], iFirst, iLast, CTRL_S);

      // Insert FontStr after every CRLF in the selected range...
      for (int ii = iFirst+1; ii <= iLast; ii++)
      {
        if (EditString[idx][ii] == C_LF)
        {
          EditString[idx] = utils->InsertW(EditString[idx], FontStr, ii+1);
          ii += len;
          iLast += len;
        }
      }

      // Write the font code pertaining to the end state.
      if (EndState.fontSize < 0)
      {
        utils->PopOnChange(re);
        return;
      }

      // Insert the font-string in front of the character imediately
      // following the last highlighted char.
      WideString Stemp = utils->FontSizeToString(EndState.fontSize);
      EditString[idx] = utils->InsertW(EditString[idx], Stemp, iLast+1);
    }

    // Convert to RTF and display it in the edit-window
    utils->ConvertToRtf(EditString[idx], NULL, re, true);
  }

  re->SelStart = SaveSelStart;
  re->Modified = true;
  utils->SetOldLineVars(re);
  utils->PopOnChange(re);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::BoldCtrlB1Click(TObject *Sender)
{
  EmbraceSelection(CTRL_B);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::InverseVidioClick(TObject *Sender)
{
  EmbraceSelection(CTRL_R);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::UnderlineCtrlU1Click(TObject *Sender)
{
  EmbraceSelection(CTRL_U);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::PlainTextCtrlO1Click(TObject *Sender)
{
  EmbraceSelection(CTRL_O, C_NULL);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::Push1Click(TObject *Sender)
{
  if (EditFind->SelLength)
    EmbraceSelection(CTRL_PUSH, CTRL_POP); // Text selected?
  else
    EmbraceSelection(CTRL_PUSH);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::Pop1Click(TObject *Sender)
{
  if (EditFind->SelLength)
    EmbraceSelection(CTRL_PUSH, CTRL_POP); // Text selected?
  else
    EmbraceSelection(CTRL_POP);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::PageBreak1Click(TObject *Sender)
{
  EmbraceSelection(C_FF);
//  TTaeRichEdit* re; int idx;
//  GetFocusedCtrl(re, idx);
//
//  PASTESTRUCT ps = EditPaste(String(C_FF));
//
//  if (ps.error == 0)
//    re->SelStart += ps.delta-ps.lines;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::EmbraceSelection(int i1, int i2)
{
  if (focusId == ID_NONE)
  {
    // "Click the find or replace box to edit it..."
    utils->ShowMessageU(REPLACEMSG[28]);
    return;
  }

  // If no terminating char, set it the same as beginning...
  if (i2 < 0)
    i2 = i1;

  wchar_t c1 = i1;
  wchar_t c2 = i2;

  TTaeRichEdit* re; int idx;
  GetFocusedCtrl(re, idx);

  utils->PushOnChange(re);

  // Save carat position
  int SaveSelStart = re->SelStart;

  // Inserting a code into a box with no printable chars? switch to codes view!
  if (re->View != V_IRC && utils->GetRealLength(re->TextW) == 0)
    ViewIrc(re);

  if (re->View == V_IRC)
    IrcInsert(re, WideString(c1), WideString(c2));
  else // re->View is V_RTF
  {
    // Note: It would be best if we could use Effects.cpp to do this, but
    // it's geared to the main edit window... take some work to modify and
    // test it, so - I've tried a few things here... stripping old codes seems
    // to be a bad idea because there is then no provision to remove an effect.
    // so this code just puts c1 in front of every selected line and c2 after,
    // plus c1/c2 at the selection's start and end

#if DEBUG_ON
  dts->CWrite("\r\nEmbraceA: " + EditString[idx] + "\r\n");
#endif

    // Add effect codes across a multi-line selection...
    if (!InsertEffectChars(re, c1, c2))
    {
      utils->PopOnChange(re);
      return;
    }

    // Convert to RTF and display it in the edit-window
    utils->ConvertToRtf(EditString[idx], NULL, re, true);
  }

  re->SelStart = SaveSelStart;
  re->Modified = true;
  utils->SetOldLineVars(re);
  utils->PopOnChange(re);
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::IrcInsert(TTaeRichEdit* re,
                                               WideString S1, WideString S2)
{
  int idx = re->Tag;

  if (idx != ID_FIND && idx != ID_REPLACE)
    return;

  // Add effect codes across a multi-line selection...
  InsertEffectChars(re, S1, S2);

  // Save SelStart, EncodeHighlight clears the control...
  int SaveSelStart = re->SelStart + utils->GetLine(re);

  // Highlight special codes
  utils->EncodeHighlight(EditString[idx], re);

  re->SelStart = SaveSelStart; // restore
}
//---------------------------------------------------------------------------
bool __fastcall TReplaceTextForm::InsertEffectChars(TTaeRichEdit* re,
                                                WideString S1, WideString S2)
{
  try
  {
    int idx = re->Tag;

    if (idx != ID_FIND && idx != ID_REPLACE)
      return false;

    int TempLen = re->SelLength;
    int iFirst, iLast;

    if (re->View == V_RTF)
    {
      // Map the start and end indices of the RTF selected text to the
      // text in EditString[idx].
      int CI;
      if (!utils->GetCodeIndices(EditString[idx], iFirst, iLast, CI, re))
        return false;
    }
    else
    {
      iFirst = re->SelStart + utils->GetLine(re);
      iLast = iFirst + TempLen + utils->CountCRs(re->SelTextW);
    }

    int len1 = S1.Length();
    int len2 = S2.Length();

    // Insert format chars...
    // This will insert Bold (etc.) at the start/end of a multi-line
    // selection and also surround complete lines within the selection with
    // S1/S2.
    if (TempLen)
    {
      for (int ii = iFirst+1; ii <= iLast; ii++)
      {
        if (ii == iFirst+1)
        {
          EditString[idx] = utils->InsertW(EditString[idx], S1, ii);
          ii += len1;
          iLast += len1;
        }
        else if (EditString[idx][ii] == C_LF)
        {
          EditString[idx] = utils->InsertW(EditString[idx], S1, ii+1);
          ii += len1;
          iLast += len1;
        }
        else if (ii == iLast)
        {
          EditString[idx] = utils->InsertW(EditString[idx], S2, ii+1);
          ii += len2;
          iLast += len2;
        }
        else if (EditString[idx][ii] == C_CR)
        {
          EditString[idx] = utils->InsertW(EditString[idx], S2, ii);
          ii += len2;
          iLast += len2;
        }
      }
    }
    else
      EditString[idx] = utils->InsertW(EditString[idx], S1, iFirst+1);

    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::SetView(TTaeRichEdit* re, int NewView)
// Set re to NewView
{
  int idx = re->Tag;
  if (idx != ID_FIND && idx != ID_REPLACE) return;

  // Set re->OnChange handler...
  if (NewView == V_RTF || NewView == V_IRC)
  {
    // Used to prevent the OnChange event from
    // Firing when we are processing or loading text
    // Enable OnChange Event last
    //
    // (NOTE: THe TopBottom, Left/Right border edit boxes do not
    // utilize an OnChange event!)
    utils->InitOnChange(re, EditChange);
  }
  else
    re->OnChange = NULL;

  switch(NewView)
  {
    case V_RTF:

      // Enable menu-insert items
      SetMenuInsertItemsState(true);

      re->ReadOnly = false;

    break;

    case V_IRC:

      // Enable menu-insert items
      SetMenuInsertItemsState(true);

      re->ReadOnly = false;

    break;

    case V_OFF:

      re->Modified = false; // reset modified flag
      re->ReadOnly = false;

    break;

    default:

      // Disable menu-insert items
      SetMenuInsertItemsState(false);

    break;
  }

  // Black text to write
  re->SelAttributes->Color = clBlack;
  re->SelAttributes->BackColor = clWhite;
// this was focusing the wrong control in Show()
//  if (!re->Focused()) re->SetFocus();
  re->View = NewView;

  if (NewView == V_RTF)
    LabelViewMode->Caption = "View: rtf";
  else
    LabelViewMode->Caption = "View: irc";
}
//---------------------------------------------------------------------------
PASTESTRUCT __fastcall TReplaceTextForm::EditCutCopy(bool bCut, bool bCopy)
// Returns the +/- change in the edit-control's length for the present
// view.
{
  PASTESTRUCT ps;

  if (focusId == ID_NONE)
  {
    // "Click the find or replace box to edit it..."
    utils->ShowMessageU(REPLACEMSG[28]);
    return ps;
  }

  TTaeRichEdit* re;
  int idx;

  GetFocusedCtrl(re, idx);

  if (re->SelLength == 0)
  {
    // "You must select text to perform this operation!", //25
    ShowMessage(String(EDITMSG[25]));
    return ps;
  }

  if (re->View == V_RTF)
    // Erases clipboard then copies Html, Rtf and Utf-16 text formats to it
    ps = utils->CutCopyRTF(bCut, bCopy, re, EditString[idx]);
  else
    // Erases clipboard then copies Utf-16 text format to it
    ps = utils->CutCopyIRC(bCut, bCopy, re, EditString[idx]);

  if (ps.error == 0)
  {
    if (bCut)
    {
      utils->SetOldLineVars(re);

      // No more text
      if (re->LineCount == 0 || EditString[idx].Length() == 0)
        re->Clear();

      re->Modified = true;
    }

    // Set form to single-line mode?
    if (BothEditBoxesAreSingleLine())
      SetFormDesign(false);

    return ps;
  }

  ShowMessage("CutCopy() in DlgReplaceText.cpp, Can't cut/copy text.\n"
                                      "Code: " + String(ps.error));

  return ps;
}
//---------------------------------------------------------------------------
PASTESTRUCT __fastcall TReplaceTextForm::EditPaste(void)
{
  PASTESTRUCT ps;

  try
  {
    WideString sOut = utils->GetClipboardText();

    if (sOut.Length())
      ps = this->EditPaste(sOut);
  }
  catch(...) { ps.error = -1; }

  return ps;
}
//---------------------------------------------------------------------------
PASTESTRUCT __fastcall TReplaceTextForm::EditPaste(WideString S)
// Returns the +/- change in the edit-control's length
{
  PASTESTRUCT ps;

  if (S.Length() == 0)
  {
    ps.error = -1;
    return ps;
  }

  if (focusId == ID_NONE)
  {
    // "Click the find or replace box to edit it..."
    utils->ShowMessageU(REPLACEMSG[28]);
    ps.error = -2;
    return ps;
  }

  TTaeRichEdit* re; int idx;
  GetFocusedCtrl(re, idx);

  if (re->View != V_RTF && re->View != V_IRC)
  {
    // "Operation not allowed in this View!", //24
    utils->ShowMessageU(EDITMSG[24]);
    ps.error = -3;
    return ps;
  }

  // If the user is pasting raw-codes only into an empty control
  // we want to be in V_IRC, not V_RTF! (EditFind should never be in V_RTF!)
  if (re->View == V_RTF && re->LineCount == 0 && utils->GetRealLength(S) == 0)
    SetView(re, V_IRC);

  // Strip codes if it's a Find and the main view is V_RTF
  if (tae->View == V_RTF && re->Name == "EditFind")
    S = utils->StripAllCodes(S);

  // Save original position and carat
  TTaePosition* p = new TTaePosition(tae);
  p->SavePos = p->Position;
  int SaveSelStart = re->SelStart;

  // Replacing text?
  if (re->SelLength)
  {
    ps = EditCutCopy(true, false); // Cut old text

    if (ps.error != 0)
    {
      delete p;
      return ps; // Error
    }

    // Note: don't need to set delta and lines in ps here because
    // what we cut won't affect where the caret is placed following
    // a paste (below)!

    utils->PushOnChange(re);
    re->SelTextW = "";
    re->Modified = true;

    // Reset for OnChange Event...
    utils->SetOldLineVars(re);

    utils->PopOnChange(re);
  }

  if (re->View == V_RTF)
  {
    try
    {
      utils->PushOnChange(re);

      // Resolve color and style states between new and old text (returns
      // EditString by reference!)
      if (utils->ResolveStateForPaste(p->Position.p, EditString[idx], S) >= 0)
      {
        ps.delta = utils->GetRealLength(S);
        ps.lines = utils->CountCRs(S);

        // Convert to RTF and display it in the edit-window
        utils->ConvertToRtf(EditString[idx], NULL, re, true);
        SetView(re, V_RTF);

        // Set form to multi-line mode?
        if (!BothEditBoxesAreSingleLine())
          SetFormDesign(true);

        // Restore original position and carat
        p->Position = p->SavePos;

        // Move SelStart to end of pasted text
        if (ps.error == 0)
        {
          re->SelStart = SaveSelStart + (ps.delta+ps.lines);

          // ...stop the pesky "phantom" colored space at the end of
          // RTF text
          if (re->SelStart == utils->GetTextLength(re))
          {
            re->SelLength = 1;
            re->SelAttributes->BackColor = clWindow;
            re->SelLength = 0;
          }
        }

        utils->SetOldLineVars(re);
        re->Modified = true;
      }
      utils->PopOnChange(re);
    }
    catch(...)
    {
      ShowMessage("Error Pasting Into Edit Box...");
      ps.error = -4;
    }
  }
  else // re->View is V_IRC...
  {
    utils->PushOnChange(re);

    // One-to-one code match for IRC View!
    ps.lines = utils->CountCRs(S); // Count cr/lfs
    ps.delta = S.Length(); // Length includes 2 for each cr/lf!

    if (re->View == V_IRC)
    {
      EditString[idx] = utils->InsertW(EditString[idx], S,
                                      re->SelStart + utils->GetLine(re) + 1);

      // NOTE: Don't call the optimizer! We may be replacing a simple code
      // that will be optimized out!!!

      utils->EncodeHighlight(EditString[idx], re);
    }
    else
      re->SelTextW = S;

    // Set form to multi-line mode?
    if (!BothEditBoxesAreSingleLine())
      SetFormDesign(true);

    // Restore original position and carat
    p->Position = p->SavePos;

    // Move SelStart to end of pasted text
    if (ps.error == 0)
      re->SelStart = SaveSelStart + (ps.delta-ps.lines);

    // Reset for OnChange Event...
    utils->SetOldLineVars(re);
    re->Modified = true;
    utils->PopOnChange(re);
  }

  delete p;

  return ps;
}
//---------------------------------------------------------------------------
//void __fastcall TReplaceTextForm::EditChange(TObject *Sender,
//      DWORD dwChangeType, void *pvCookieData, bool &AllowChange)
void __fastcall TReplaceTextForm::EditChange(TObject *Sender)
// We only set this handler after initially showing and filling
// the edit-control to block misfires. This event fires
// on a char or chars added to the control and then fires
// once more on ENTER, before the Edit1KeyPress function is called.
//
// This function adds user keypress codes to the MS_ORG or MS_IRC
// streams depending upon the ViewType. It handles codes both being
// added or removed from the TaeEdit control. This function translates
// raw text-control chars into red-highlighted letters that represent
// the raw code. The Memory stream version has the raw codes.
//
// With an empty edit control, when the user types the first letter,
// we enter this function in re->View "V_OFF".
//
// This function queues chars for processing by a background thread
// (ThreadOnChange.cpp) to keep this handler from bogging down.
{
  TTaeRichEdit* re = (TTaeRichEdit*)Sender;

  if (re == NULL || re->LockCounter != 0)
  {
#if DEBUG_ON
    if (re == NULL)
      dts->CWrite("\r\n" + String("change: NULL re") + "\r\n");
    else
      dts->CWrite("\r\n" + String("change: locked") + "\r\n");
#endif

    return;
  }

  int idx = re->Tag; // idx is the array index to use

  if (idx != ID_FIND && idx != ID_REPLACE)
    return;

  // Populate ONCHANGE thread's struct (ProcessOnChange.h)
  // (Get the deltas in length and line-count and point
  // to the right string-list!) (NOTE: the oc.p stringlist
  // pointer is set below!)
  ONCHANGEW oc = utils->GetInfoOC(re, NULL);

  if (oc.deltaLength == 0)
  {
#if DEBUG_ON
    dts->CWrite("\r\n" + String("change: deltaLength == 0") + "\r\n");
#endif
    return;
  }

  oc.p = (void*)idx; // pass the edit-control's ID

  // Disable changes (faster code)
  re->SaveOnChange = re->OnChange;
  re->OnChange = NULL;
  re->LockCounter++;

  // User added chars? (1 or 2 for cr/lf)
  if (oc.selStart >= 0 && oc.deltaLength > 0 && oc.deltaLength <= 2)
  {
    // Get the char added
    int first = oc.selStart-1;

    if (first < 0)
      first = 0;

    int last = oc.selStart;

    if (last > oc.length)
      last = oc.length;

    wchar_t* buf = re->GetRangeBufW(first, last);

    if (buf != NULL)
    {
      for (int ii = 0; ii < OC_BUFSIZE; ii++)
      {
        oc.c[ii] = buf[ii];
        
        if (!buf[ii])
          break;
      }

      if (re->View == V_IRC)
      {
        int SaveSelStart = oc.selStart;

        try
        {
          // Here we simply negate the default behavior of the
          // edit-control, which is to keep the red-highlight
          // color in-effect for new chars typed after a red
          // char. We return it to black.

          wchar_t Cnew = utils->GetHighlightLetter(oc.c[0]); // substitute char

          re->SelStart = oc.selStart-oc.deltaLength;;

          if (Cnew != oc.c[0])
          {
            // char entered is a code? change to red and replace char
            re->SelLength = 1; // Just one char to replace!

            if (tae->SelAttributes->Color != clRed)
              re->SelAttributes->Color = clRed;

            re->SelTextW = WideString(Cnew);
            re->SelLength = 0;
          }
          // Change range to black (could have inserted many chars
          // after a red!)
          else if (re->SelAttributes->Color == clRed)
            re->SelAttributes->Color = dts->cColor;

        }
        catch(...)
        {
          ShowMessage("Exception: TReplaceTextForm::DoEditChange(TTaeRichEdit* re)");
        }

        re->SelStart = SaveSelStart;
      }
    }

    delete [] buf;
  }
  // else chars deleted!

  this->ProcessOC(re, oc); // Process the char in real-time

  re->OldLength = oc.length;
  re->OldLineCount = oc.lineCount;

  re->Modified = true;

  re->OnChange = re->SaveOnChange;
  re->LockCounter--;
}
//---------------------------------------------------------------------------
bool __fastcall TReplaceTextForm::ProcessOC(TTaeRichEdit* re, ONCHANGEW oc)
// Use this for diagnostics! (set DEBUG_ON "true" in Main.h)
//#if DEBUG_ON
//  dts->CWrite("\r\nhereA\r\n");
//#endif
// Note: oc.view below has our local re->View in it, not the
// main edit's View!
{
  // For us, oc.p has an edit-control ID, ID_FIND or ID_REPLACE
  int idx = (int)oc.p;

  try
  {
    if (oc.deltaLength > 0) // User added chars?
    {
      // V_IRC is now essentially the same as the processed RTF View
      // except the color-text is highlighting raw IRC format-codes
      // and the codes have been substituted with letters.
      if (oc.view == V_IRC)
      {
        // oc.deltaLength is 1 for a char and 2 when you hit "Enter" (CR/LF)
        int temp = (oc.selStart+oc.line)-oc.deltaLength;

        if (temp < 0)
          temp = 0;
        else if (temp > oc.length-1)
          temp = oc.length-1;

        try
        {
          // Add char(s) to string-list...
          if (oc.c[0] == C_CR) // added a CR/LF?
          {
            EditString[idx] = utils->InsertW(EditString[idx], CRLF, temp+1);

            // Set form to multi-line mode?
            if (!BothEditBoxesAreSingleLine())
              SetFormDesign(true);
          }
          else
            EditString[idx] = utils->InsertW(EditString[idx],
                                                WideString(oc.c[0]), temp+1);
        }
        catch(...)
        {
#if DEBUG_ON
          dts->CWrite("\r\nException 1: TReplaceTextForm::ProcessOC(ONCHANGEW oc)\r\n");
#endif
          return false;
        }
      }
      else // RTF
      {
        try
        {
          // Map the start index of the RTF selected text to the
          // text in the buffer
          int iFirst, iLast, CI;
          // This points First to the IRC char at the insert-point...
          // CI will point to any codes before First.
          if (!utils->GetCodeIndices(EditString[idx], iFirst, iLast, CI,
                                            oc.selStart-1, oc.deltaLength))
          {
            // Comes here when the edit control is empty...
            if (oc.c[0] == C_CR)
              EditString[idx] = CRLF; // added a CR/LF?
            else
              EditString[idx] = WideString(oc.c);

            // Set form to multi-line mode?
            if (!BothEditBoxesAreSingleLine())
              SetFormDesign(true);
          }
          else
          {
            // Get state at insert-point...
            WideString LeadingState;

            if (oc.c[0] == C_CR) // added a CR/LF?
            {
              // Don't Copy Afg/Abg if NO_COLOR, don't skip spaces...(ok?)
              int RetIdx = utils->GetState(EditString[idx], LeadingState,
                           oc.selStart-oc.line+1, true, false);

              if (RetIdx < 0)
                LeadingState = "";

              String S = LeadingState;

              if (iFirst <= EditString[idx].Length())
              {
                S = S.Insert(CRLF, 1);

                EditString[idx] = utils->InsertW(EditString[idx], S, iFirst+1);

                // Set form to multi-line mode?
                if (!BothEditBoxesAreSingleLine())
                  SetFormDesign(true);
              }
            }
            else
            {
              int iTemp;

              if (oc.selStart == 1)
                iTemp = iFirst+1;
              else
                iTemp = CI+1;

              if (iTemp <= EditString[idx].Length()+1)
              {
                // It was hard to debug this and find the best solution.
                // Problems were: 1) End of document, first char of new
                // line needs color-codes written... 2) add a char to the end
                // of a pre-existing line needs to work too. So this is
                // a temporary hack.

                WideString S(oc.c[0]);

                // Previous char a line-break? Insert LeadingState!
                if (iTemp-2 > 0 && EditString[idx][iTemp-2] == C_CR)
                {
                  // Copy Afg/Abg if NO_COLOR, don't skip spaces...(ok?)
                  int RetIdx = utils->GetState(EditString[idx], LeadingState,
                                    oc.selStart-oc.line, true, true);

                  if (RetIdx < 0)
                    LeadingState = "";

                  S = utils->InsertW(S, LeadingState, 1);
                }
                else if (iTemp-1 > 0 && EditString[idx][iTemp-1] == C_CR)
                  iTemp--;

                EditString[idx] = utils->InsertW(EditString[idx], S, iTemp);
              }
            }
          }
        }
        catch(...)
        {
#if DEBUG_ON
          dts->CWrite("\r\nException 2: TReplaceTextForm::ProcessOC(ONCHANGEW oc)\r\n");
#endif
          return false;
        }
      }
    }
    else // if < 0 then chars were deleted
    {
      // Note: by the time this gets called, the char(s) have already
      // been removed from TaeEdit->TextW! A Selected range is handled
      // by the Cut/Copy routine, not here!

      // Empty String
      if (oc.length == 0)
      {
        EditString[idx] = "";

        // Set form to single-line mode
        if (BothEditBoxesAreSingleLine())
          SetFormDesign(false);
      }
      else
      {
        try
        {
          // CR/LF deleted?
          if (oc.c[0] == C_CR)
          {
            if (oc.view == V_RTF)
            {
              int iTemp = utils->GetCodeIndex(EditString[idx], oc.selStart);

              if (iTemp >= 0)
                EditString[idx] = utils->DeleteW(EditString[idx], iTemp+1, 2);
#if DEBUG_ON
              else
                dts->CWrite("\r\nError 9 TReplaceTextForm::ProcessOC()\r\n");
#endif
            }
            else
              EditString[idx] =
                  utils->DeleteW(EditString[idx], oc.selStart+oc.line+1, 2);

            // Set form to single-line mode?
            if (BothEditBoxesAreSingleLine())
              SetFormDesign(false);
          }
          else
          {
            // if the user pressed ctrl-x to replace hex-digits typed-in with
            // a Unicode char, then we need to remove 1 extra char than was
            // reported in deltaLength...
            if (bUnicodeKeySequence && oc.deltaLength < 0)
            {
              oc.deltaLength += -1;
              oc.selStart -= 1;
            }

            int iTemp;

            if (oc.view == V_RTF)
              iTemp = utils->GetCodeIndex(EditString[idx], oc.selStart);
            else // V_IRC, V_ORG or V_OFF
              iTemp = oc.selStart+oc.line;

            if (iTemp >= 0)
              EditString[idx] =
                    utils->DeleteW(EditString[idx], iTemp+1, -oc.deltaLength);
#if DEBUG_ON
            else
              dts->CWrite("\r\nError 10 ProcessOC()\r\n");
#endif

            // Add new Unicode char via recursive call...
            if (bUnicodeKeySequence)
            {
              if (oc.deltaLength < 0)
              {
                WideString sCaretChar =
                          re->GetRangeW(oc.selStart, oc.selStart+1);

                if (sCaretChar.Length() == 1)
                {
                  ONCHANGEW ocNew = oc;
                  ocNew.selStart = oc.selStart + 1;
                  ocNew.c[0] = sCaretChar[1];
                  ocNew.deltaLength = 1;

                  // Recursive call to add the new Unicode char
                  ProcessOC(re, ocNew);
                }
              }

              bUnicodeKeySequence = false;
            }
          }
        }
        catch(...)
        {
#if DEBUG_ON
          dts->CWrite("\r\nError 12 TReplaceTextForm::ProcessOC()\r\n");
#endif
          return false;
        }
      }
    }
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nError 13 TReplaceTextForm::ProcessOC()\r\n");
#endif
    return false;
  }

  return true;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::EditFindEnter(TObject *Sender)
{
  focusId = ID_FIND;
  EditFind->Color = dts->TaeWindowColor;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::EditFindExit(TObject *Sender)
{
  EditFind->Color = DTS_GRAY;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::EditReplaceEnter(TObject *Sender)
{
  focusId = ID_REPLACE;
  EditReplace->Color = dts->TaeWindowColor;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::EditReplaceExit(TObject *Sender)
{
  EditReplace->Color = DTS_GRAY;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::SetFocusId(void)
// We use the focusId to know which edit control had focus when
// we clicked the Insert menu...
{
  if (EditFind->Focused())
    focusId = ID_FIND;
  else if (EditReplace->Focused())
    focusId = ID_REPLACE;
  else
    focusId = ID_NONE;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::GetFocusedCtrl(TTaeRichEdit* &re)
{
  if (focusId == ID_FIND)
    re = EditFind;
  else
    re = EditReplace;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::GetFocusedCtrl(TTaeRichEdit* &re, int &idx)
{
  if (focusId == ID_FIND)
    re = EditFind;
  else
    re = EditReplace;

  idx = re->Tag; // idx is in the TTaeRichEdit's Tag property
  if (idx != ID_FIND && idx != ID_REPLACE)
  {
    idx = ID_FIND;
    ShowMessage("Bad re->Tag index in: TReplaceTextForm::GetFocusedCtrl()");
  }
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::SetMenuInsertItemsState(bool bEnable)
{
  if (bEnable)
  {
    ColorCode1->Enabled = true;
    FontTypeCode1->Enabled = true;
    FontSizeCode1->Enabled = true;
    BoldCtrlB1->Enabled = true;
    InverseVidio->Enabled = true;
    PlainTextCtrlO1->Enabled = true;
    UnderlineCtrlU1->Enabled = true;
    Push1->Enabled = true;
    Pop1->Enabled = true;
    Pop1->Enabled = true;
    PageBreak1->Enabled = true;
  }
  else
  {
    ColorCode1->Enabled = false;
    FontTypeCode1->Enabled = false;
    FontSizeCode1->Enabled = false;
    BoldCtrlB1->Enabled = false;
    InverseVidio->Enabled = false;
    UnderlineCtrlU1->Enabled = false;
    PlainTextCtrlO1->Enabled = false;
    Push1->Enabled = false;
    Pop1->Enabled = false;
    PageBreak1->Enabled = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::MainMenuHelpClick(TObject *Sender)
{
  utils->ShowMessageU(REPLACEMSG[29]);
}
//---------------------------------------------------------------------------
bool __fastcall TReplaceTextForm::BothEditBoxesAreSingleLine(void)
{
  return utils->CountCRs(EditString[ID_FIND]) == 0 &&
      utils->CountCRs(EditString[ID_REPLACE]) == 0 ? true : false;
}
//---------------------------------------------------------------------------
void __fastcall TReplaceTextForm::SetFormDesign(bool bMultiLine)
{
  utils->PushOnChange(EditFind);
  utils->PushOnChange(EditReplace);

  int SaveFindSelStart = EditFind->SelStart;
  int SaveReplaceSelStart = EditReplace->SelStart;

  if (bMultiLine)
  {
    EditFind->SetScrollBars(ssBoth);
    EditReplace->SetScrollBars(ssBoth);

    EditFind->HideScrollBars = false;
    EditReplace->HideScrollBars = false;

    ReplaceTextForm->Height = 293;
    EditFind->Height = 70;
    EditReplace->Height = 70;

    LabelFind->Top = 12;
    LabelReplaceWith->Top = 100;
    LabelPressF3F4->Top = 80;
    ButtonToggleView->Top = 114;
    CheckBoxWrapAround->Top = 180;
    CheckBoxCaseSensitive->Top = 180;
    CheckBoxWholeWord->Top = 180;
    ButtonFindUp->Top = 208;
    ButtonFindDown->Top = 208;
    ButtonReplace->Top = 208;
    ButtonReplaceAll->Top = 208;
    ButtonCancel->Top = 208;
    EditReplace->Top = 96;
  }
  else // Single-line find/replace
  {
    EditFind->SetScrollBars(ssNone);
    EditReplace->SetScrollBars(ssNone);

    EditFind->HideScrollBars = true;
    EditReplace->HideScrollBars = true;

    ReplaceTextForm->Height = 216;
    EditFind->Height = 25;
    EditReplace->Height = 25;

    LabelFind->Top = 8;
    LabelReplaceWith->Top = 68;
    LabelPressF3F4->Top = 40;
    ButtonToggleView->Top = 62;
    CheckBoxWrapAround->Top = 100;
    CheckBoxCaseSensitive->Top = 100;
    CheckBoxWholeWord->Top = 100;
    ButtonFindUp->Top = 128;
    ButtonFindDown->Top = 128;
    ButtonReplace->Top = 128;
    ButtonReplaceAll->Top = 128;
    ButtonCancel->Top = 128;
    EditReplace->Top = 64;
  }

  LabelViewMode->Top = ButtonToggleView->Top+ButtonToggleView->Height+1;

  this->bMultiLine = bMultiLine;

  EditFind->SelStart = SaveFindSelStart;
  EditReplace->SelStart = SaveReplaceSelStart;

  utils->PopOnChange(EditFind);
  utils->PopOnChange(EditReplace);
}
//---------------------------------------------------------------------------

