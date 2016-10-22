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

#include "DlgWingEdit.h"
#include "DlgIncDec.h"
#include "DlgSetColors.h"
#include "FormSFDlg.h"
#include "FormOFSSDlg.h"
#include "ConvertFromHTML.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "TaeRichEdit"
#pragma resource "*.dfm"

// Add color to list-box items use OnDrawItem
// The Items property of a TListBox has an AddObject method. You can use it when
// adding your string like this:
// clRed is an Integer, and you just cast it to an object so it can be added
// alongside your item text.
//  ListBox1.Items.AddObject('item', TObject(clRed));
// In your OnDrawItem (color is a TColor variable) where i is the index of your
// current item.
//  color = TColor(ListBox.Items.Objects[i]);
//
// We can create a TList of pointers to objects then set the TListBox Items
// to our list... each object has a rich-edit string and the OnDrawItem
// event would interpret the irc codes and directly draw them changing color and font...
// humm - sounds like moving another mountain with a shovel :-)

TWingEditForm *WingEditForm;

//---------------------------------------------------------------------------
// INIT ROUTINES
//---------------------------------------------------------------------------
__fastcall TWingEditForm::TWingEditForm(TComponent* Owner)
  : TForm(Owner)
{
  this->dts = DTSColor;

  // Clipboard formats
  cbHTML = (unsigned short)RegisterClipboardFormat(HTML_REGISTERED_NAME);
  cbRTF = (unsigned short)RegisterClipboardFormat(RTF_REGISTERED_NAME);
}
//---------------------------------------------------------------------------
// Confirmed that this gets called AFTER the constructor for this form...
// so "this->dts" does work.
void __fastcall TWingEditForm::FormCreate(TObject *Sender)
{
  Lwlb = new TMyCheckListBox(this);

  Lwlb->Align = alNone;
  Lwlb->AllowGrayed = false;
  Lwlb->BorderStyle = bsSingle;
  Lwlb->Color = clInfoBk;
  Lwlb->Enabled = true;
  Lwlb->Flat = true;
  Lwlb->Height = 132;
  Lwlb->IntegralHeight = true;
  Lwlb->ItemHeight = 14;
  Lwlb->Left = 16;
  Lwlb->Name = "Lwlb";
  Lwlb->PopupMenu = WingsPopupMenu;
  Lwlb->Sorted = false;
  Lwlb->Style = lbOwnerDrawVariable;
  Lwlb->Top = 72;
  Lwlb->Visible = true;
  Lwlb->Width = 233;
  Lwlb->Parent = this;

  Edit1->Tag = NULL; // OnChange handler save location

  OldLength = 0;

  CurrentIndex = -5000; // Any number not the same as a list-box index

  bLeftSelected = true;

  bDoubleclick = false;
  bUnicodeKeySequence = false;

  bSaveFileDialogOpen = false;
  bOpenFileDialogOpen = false;

  Lwlb->Color = dts->TaeWindowColor;
  Rwlb->Color = DTS_GRAY;

  // Kept in utf-8 - which we can call Utf8ToWide on to set the GDI font
  // with UTF-16 names.
  Lwlb->Font->Name = utils->WideToUtf8(dts->cFont);
  Rwlb->Font->Name = utils->WideToUtf8(dts->cFont);

  // Enable extended characters in the list-boxes
// not in my header-files!
//  HDC hDC = Lwlb->Canvas->Handle;
//  EnableEUDC(hDC, TRUE);
//  hDC = Rwlb->Canvas->Handle;
//  EnableEUDC(hDC, TRUE);
//#if DEBUG_ON
//   DTSColor->CWrite("\r\ncreated left list-box\r\n");
//#endif

  Lwlb->Font->Size = dts->cSize;
  Rwlb->Font->Size = dts->cSize;

  TopBottomBorders->Checked = dts->bTopEnabled;
  SideBorders->Checked = dts->bSideEnabled;

  // EDITMSG[12] =
  // "Press ENTER to exit and save.\n"
  // "Press ESC to exit without saving.\n"
  // "Right-click for the pop-up menu.";
  Edit1->Hint = String(EDITMSG[12]);

  // EDITMSG[13] =
  //  "Left-click an item to edit it.\n"
  //  "Right-click for the popup menu.\n"
  //  "Check the box to enable the wing-pair.\n"
  //  "Press the ESC key to abort.";
  String S = String(EDITMSG[13]);

  Lwlb->Hint = S;
  Lwlb->ShowHint = true;
  Rwlb->Hint = S;
  Rwlb->ShowHint = true;

  // EDITMSG[14] = "Click to Edit";
  S = String(EDITMSG[14]);

  TopBottomEdit->Hint = S;
  TopBottomEdit->ShowHint = true;
  LeftSideEdit->Hint = S;
  LeftSideEdit->ShowHint = true;
  RightSideEdit->Hint = S;
  RightSideEdit->ShowHint = true;

  // Set Labels/Captions
  Caption = String(EDITMSG[0]);
  Label5->Caption = String(EDITMSG[1]);
  Label6->Caption = String(EDITMSG[2]);
  Label7->Caption = String(EDITMSG[3]);
  Label8->Caption = String(EDITMSG[4]);
  Label9->Caption = String(EDITMSG[5]);
  Label10->Caption = String(EDITMSG[6]);
  TopBottomBorders->Caption = String(EDITMSG[7]);
  SideBorders->Caption = String(EDITMSG[8]);
  ExitButton->Caption = String(EDITMSG[9]);
  Button2->Caption = String(EDITMSG[10]);
  Button3->Caption = String(EDITMSG[11]);

  try
  {
    utils->LoadMenu(MainMenu1, (char**)WINGEDITMAINMENU);
    utils->LoadMenu(WingsPopupMenu, (char**)EDITPOPUPMENU1);
    utils->LoadMenu(WingEditorPopupMenu, (char**)EDITPOPUPMENU2);
  }
  catch(...)
  {
    ShowMessage("WingEditor(): Unable to load Menus!");
  }

  // Word-wrap is on by default if no scrollbars it appears!
//  Edit1->SetScrollBars(ssVertical);
  Edit1->SetScrollBars(ssHorizontal);
//  Edit1->SetScrollBars(ssNone);
  Edit1->Color = dts->TaeWindowColor;
  Edit1->DefAttributes->BackColor = clWindow;
  Edit1->HideScrollBars = true;

  TopBottomEdit->SetScrollBars(ssNone);
  TopBottomEdit->Color = dts->TaeWindowColor;
  TopBottomEdit->DefAttributes->BackColor = clWindow;

  LeftSideEdit->SetScrollBars(ssNone);
  LeftSideEdit->Color = dts->TaeWindowColor;
  LeftSideEdit->DefAttributes->BackColor = clWindow;

  RightSideEdit->SetScrollBars(ssNone);
  RightSideEdit->Color = dts->TaeWindowColor;
  RightSideEdit->DefAttributes->BackColor = clWindow;

  SetView(V_OFF);

  // Enable list-box event-handlers
  Lwlb->OnClick = LwlbClick;
  Lwlb->OnDblClick = ListBoxDblClick;
  Lwlb->OnDrawItem = LwlbDrawItem;
  Lwlb->OnExit = ListBoxExit;
  Lwlb->OnKeyDown = WingsListBoxKeyDown;
  Lwlb->OnKeyPress = LwlbKeyPress;
  Lwlb->OnMeasureItem = LwlbMeasureItem;
  Lwlb->OnMouseDown = LwlbMouseDown;

  Rwlb->OnClick = RwlbClick;
  Rwlb->OnDblClick = ListBoxDblClick;
  Rwlb->OnDrawItem = RwlbDrawItem;
  Rwlb->OnExit = ListBoxExit;
  Rwlb->OnKeyDown = WingsListBoxKeyDown;
  Rwlb->OnKeyPress = RwlbKeyPress;
  Rwlb->OnMeasureItem = RwlbMeasureItem;
  Rwlb->OnMouseDown = RwlbMouseDown;

//  Lwlb->View = View.Details;
//  Lwlb->FullRowSelect = true;

//  g_LwlbHFont = MakeFont((TListBox*)Lwlb);
//  g_RwlbHFont = MakeFont(Rwlb);
}
//---------------------------------------------------------------------------
/*
HFONT __fastcall TWingEditForm::MakeFont(TListBox* lb)
{
  HFONT hf = NULL;

  try
  {
    if (lb != NULL)
    {
      WideString sFont = utils->Utf8ToWide(lb->Font->Name);
      TCanvas* pCanvas = lb->Canvas;
      long lfHeight =
        -MulDiv(lb->Font->Size, GetDeviceCaps(pCanvas->Handle, LOGPIXELSY), 72);
//    p->Font->Charset = 176; // code-page 1200 is UTF-16, 0x4B0 - the B0 part is 176
//    p->Font->Charset = 233; // code-page 65001 is UTF8, 0xFDE9 - the E9 part is 233
//      hf = CreateFontW(lfHeight, 0, 0, 0, FW_MEDIUM, TRUE,
//                              0, 0, ANSI_CHARSET, 0, 0, 0, 0, sFont.c_bstr());
//      hf = CreateFontW(lfHeight, 0, 0, 0, FW_MEDIUM, TRUE,
//                              0, 0, 0, 0, 0, 0, 0, sFont.c_bstr());
      hf = CreateFontW(lfHeight, 0, 0, 0, FW_MEDIUM, TRUE,
                              0, 0, 0, 0, 0, 0, 0, sFont.c_bstr());
    }
  }
  catch(...) {}

#if DEBUG_ON
  if (hf)
    dts->CWrite("\r\nFont handle is non-zero\r\n");
#endif
  return hf;
}
*/
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::FormDestroy(TObject *Sender)
{
  if (Lwlb != NULL)
    delete Lwlb;

//  if (g_LwlbHFont != NULL)
//    DeleteObject(g_LwlbHFont);

//  if (g_RwlbHFont != NULL)
//    DeleteObject(g_RwlbHFont);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::FormShow(TObject *Sender)
{
  TopBottomEditString = dts->TopBorder;
  LeftEditString = dts->LeftSideBorder;
  RightEditString = dts->RightSideBorder;

  try
  {
    if (!LoadWingEditBoxes())
      return;
  }
  catch(...)
  {
    ShowMessage("WingEditor(): Unable to load wing-boxes!");
    return;
  }

  try
  {
    borderBgColor = dts->BorderBackground;

    // Force the colors of border-strings to change if needed...
    ChangeBorderColors(borderBgColor);

    // Load the border rich edit controls
    LoadBorderRichEditBoxes(TopBottomEditString, LeftEditString, RightEditString);
  }
  catch(...) {ShowMessage("WingEditor(): Unable to change border-colors!");}

  // removes the ugly dotted line around focused item (does not work)
//  SendMessage(Lwlb->Handle, WM_CHANGEUISTATE, (1<<sizeof(SHORT))+1, 0);
//  SendMessage(Rwlb->Handle, WM_CHANGEUISTATE, (1<<sizeof(SHORT))+1, 0);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  CloseOpenSaveDialogs();

  WingEditForm = NULL;
  this->Release();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::WingEditorPopupMenuPopup(TObject *Sender)
// Menu for cut, copy, paste in Edit1
{
  if (Edit1->SelLength)
  {
    MenuEditCut->Enabled = true;
    MenuEditCopy->Enabled = true;
    MenuEditDelete->Enabled = true;
  }
  else
  {
    MenuEditCut->Enabled = false;
    MenuEditCopy->Enabled = false;
    MenuEditDelete->Enabled = false;
  }

  if (Clipboard()->AsText.Length())
    MenuEditPaste->Enabled = true;
  else
    MenuEditPaste->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::WingsPopupMenuPopup(TObject *Sender)
// Menu for wing-check-boxes
{
  if (Clipboard()->AsText.Length())
    Paste1->Enabled = true;
  else
    Paste1->Enabled = false;

  AddNewWingPair1->Enabled = true;
  ImportWingFromMainWindow1->Enabled = true;

  if (bLeftSelected)
  {
    if (Lwlb->ItemIndex != -1)
    {
      EditSelectedWing1->Enabled = true;
      DeleteWingPair1->Enabled = true;
      Copy1->Enabled = true;
    }
    else
    {
      EditSelectedWing1->Enabled = false;
      DeleteWingPair1->Enabled = false;
      Copy1->Enabled = false;
    }
  }
  else
  {
    if (Rwlb->ItemIndex != -1)
    {
      EditSelectedWing1->Enabled = true;
      DeleteWingPair1->Enabled = true;
      Copy1->Enabled = true;
    }
    else
    {
      EditSelectedWing1->Enabled = false;
      DeleteWingPair1->Enabled = false;
      Copy1->Enabled = false;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::SaveAll(void)
{
  SaveEdit();
  HideEdit();

  SaveWingEditBoxes(); // Set totalwings

  bool flag1 = false;

  for (int ii = 0; ii < Lwlb->Items->Count; ii++)
  {
    if (Lwlb->Checked[ii] && (bool)dts->LeftWings->GetTag(ii) == false)
    {
      flag1 = true;
      break;
    }
  }

  bool bNoWingsChecked = true;

  for (int ii = 0; ii < Lwlb->Items->Count; ii++)
  {
    if (Lwlb->Checked[ii])
    {
      bNoWingsChecked = false;
      break;
    }
  }

  bool flag2 = false;

  for (int ii = 0; ii < dts->TotalWings; ii++)
  {
    if (bNoWingsChecked && (bool)dts->LeftWings->GetTag(ii) == true)
    {
      flag2 = true;
      break;
    }
  }

  if (flag1)
    // Do this because setting the Checked property
    // will invoke the handler for the checkbox which
    // will check WingActive[]
  {
    dts->Wingdings->OnClick = NULL;
    dts->Wingdings->Checked = true;
    dts->Wingdings->OnClick = dts->WingdingsClick;
  }
  else if (flag2)
  {
    dts->Wingdings->OnClick = NULL;
    dts->Wingdings->Checked = false;
    dts->Wingdings->OnClick = dts->WingdingsClick;
  }

  if (TopBottomBorders->Checked && !dts->bTopEnabled ||
       SideBorders->Checked && !dts->bSideEnabled)
  {
    dts->Borders->OnClick = NULL;
    dts->Borders->Checked = true;
    dts->Borders->OnClick = dts->BordersClick;
  }
  else if ((!TopBottomBorders->Checked &&
            !SideBorders->Checked) &&
            (dts->bTopEnabled || dts->bSideEnabled))
  {
    dts->Borders->OnClick = NULL;
    dts->Borders->Checked = false;
    dts->Borders->OnClick = dts->BordersClick;
  }

  dts->bTopEnabled = TopBottomBorders->Checked;
  dts->bSideEnabled = SideBorders->Checked;

  dts->BorderBackground = borderBgColor;
}
//---------------------------------------------------------------------------
// MENU CLICKS FOR INSERTING INTO EDIT-BOX
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::ColorCode1Click(TObject *Sender)
{
  if (Edit1->View != V_IRC && Edit1->View != V_RTF)
  {
    utils->ShowMessageU(EDITMSG[28]); // "Click or double-click an item to edit it..."
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

  if (fg != NO_COLOR)
    bHaveFg = true;
  if (bg != NO_COLOR)
    bHaveBg = true;

  if (!bHaveFg && !bHaveBg)
    return;

  fg = utils->ConvertColor(fg, false);
  bg = utils->ConvertColor(bg, false);

  WideString ColorFmt;

  // Write the color-codes into a string
  utils->WriteColors(fg, bg, ColorFmt);

  utils->PushOnChange(Edit1);

  // Save carat position
  int SaveSelStart = Edit1->SelStart;

  if (Edit1->View == V_IRC)
  {
    int TempStart = Edit1->SelStart + 1;
    int TempLen = Edit1->SelLength;

    EditString = utils->InsertW(EditString, ColorFmt, TempStart);

    if (TempLen)
      EditString = utils->InsertW(EditString, String(CTRL_K), TempStart +
                                                TempLen + ColorFmt.Length());

    // Highlight special codes
    utils->EncodeHighlight(EditString, Edit1);
    Edit1->SelStart = TempStart - 1;
  }
  else // Edit1->View is V_RTF
  {
    if (!Edit1->SelLength) // if no text selected...
    {
      // Strip existing color-codes
      if (bHaveFg && bHaveBg)
      {
        int len = EditString.Length();
        EditString = utils->StripColorCodes(EditString, 0, len);
      }
      else if (bHaveFg)
        EditString = utils->StripFgCodes(EditString);
      else // bg only
        EditString = utils->StripBgCodes(EditString);

      // Map the start and end indices of the RTF text to the
      // raw text in EditString.
      int iFirst = utils->GetCodeIndex(Edit1);

      if (iFirst < 0)
      {
        utils->PopOnChange(Edit1);
        return;
      }

      // Write the colors imediately before the first printable character.
      EditString = utils->InsertW(EditString, ColorFmt, iFirst+1);
    }
    else // text is selected...
    {
      // Set the cumulative text state that exists at the first character
      // in EditString.
      PUSHSTRUCT BeginningState;
      utils->SetStateFlags(EditString, STATE_MODE_FIRSTCHAR, BeginningState);

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
      if (bFgWrite && bBgWrite)
        utils->WriteColors(IRCBLACK, IRCWHITE, InitColorFmt);
      else if (bFgWrite)
        utils->WriteSingle(IRCBLACK, InitColorFmt, true);
      else // bg only
        utils->WriteSingle(IRCWHITE, InitColorFmt, false);

      // Write the colors at the start of the string.
      EditString = utils->InsertW(EditString, InitColorFmt, 1);

      // Map the start and end indices of the RTF selected text to the
      // text in the buffer
      int iFirst, iLast, CI;
      if (!utils->GetCodeIndices(EditString, iFirst, iLast, CI, Edit1))
      {
        utils->PopOnChange(Edit1);
        return;
      }

      // Set the cumulative text state that exists at the character
      // following the last highlighted character...
      int LastRealIndex = Edit1->SelStart+Edit1->SelLength;
      PUSHSTRUCT EndState;
      utils->SetStateFlags(EditString, LastRealIndex, EndState);

      // Insert the new color(s) in front of the first highlighted character.
      EditString = utils->InsertW(EditString, ColorFmt, iFirst+1);
      iFirst += ColorFmt.Length();
      iLast += ColorFmt.Length();

      // Strip out color codes in the highlighted range.  "Last" may return
      // shorted by the # of color-code chars removed.
      EditString = utils->StripColorCodes(EditString, iFirst, iLast);

      // Write the color codes pertaining to the end state.
      ColorFmt = ""; // Must clear to avoid appending to it!
      utils->WriteColors(EndState.fg, EndState.bg, ColorFmt);

      // Insert the color-string in front of the character imediately
      // following the last highlighted char.
      EditString = utils->InsertW(EditString, ColorFmt, iLast+1);
    }

    // Optimize the new string
    EditString = utils->Optimize(EditString, false, NO_COLOR);

    // Convert to RTF and display it in the edit-window
    utils->ConvertToRtf(EditString, NULL, Edit1, true);
  }

  Edit1->SelStart = SaveSelStart;
  Edit1->Modified = true;
  OldLength = Edit1->TextLength;
  utils->PopOnChange(Edit1);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::FontTypeCode1Click(TObject *Sender)
{
  if (Edit1->View != V_IRC && Edit1->View != V_RTF)
  {
    utils->ShowMessageU(EDITMSG[28]); // "Click or double-click an item to edit it..."
    return;
  }

  int Type = 0;

  if (!dts->SelectFontType(String(DS[213]), Type)) return; // no font-code...

  String FontStr;

  // Write the font code pertaining to the end state.
  FontStr = utils->FontTypeToString(Type);

  utils->PushOnChange(Edit1);

  // Save carat position
  int SaveSelStart = Edit1->SelStart;

  if (Edit1->View == V_IRC)
  {
    int TempStart = Edit1->SelStart + 1;
    int TempLen = Edit1->SelLength;

    EditString = utils->InsertW(EditString, FontStr, TempStart);

    if (TempLen)
      EditString = utils->InsertW(EditString, String(CTRL_F), TempStart + TempLen + FontStr.Length());

    // Highlight special codes
    utils->EncodeHighlight(EditString, Edit1);
    Edit1->SelStart = TempStart - 1;
  }
  else // Edit1->View is V_RTF
  {
    if (!Edit1->SelLength) // Text selected?
    {
      // If not, write the font-code at the cursor.
      if (Edit1->SelStart >= 0)
      {
        int iTemp = utils->GetCodeIndex(Edit1->TextW, Edit1->SelStart);

        if (iTemp >= 0)
          EditString = utils->InsertW(EditString, FontStr, iTemp+1);
      }
      else
        EditString = utils->InsertW(EditString, FontStr, 1);
    }
    else // Text selected
    {
      // Set the cumulative text state that exists at the first character
      // in EditString.
      PUSHSTRUCT BeginningState;
      utils->SetStateFlags(EditString, STATE_MODE_FIRSTCHAR, BeginningState);

      // Write default font-type at beginning if the wing has no default
      // font-type and the user is trying to add a font-type code
      // somewhere in the wing.
      if (BeginningState.fontType == -1)
      {
        String InitFontFmt = utils->FontTypeToString(-1);
        EditString = utils->InsertW(EditString, InitFontFmt, 1);
      }

      // Map the start and end indices of the RTF selected text to the
      // text in the buffer
      int iFirst, iLast, CI;
      if (!utils->GetCodeIndices(EditString, iFirst, iLast, CI, Edit1))
      {
        utils->PopOnChange(Edit1);
        return;
      }

      // Set the cumulative text state that exists up to the character
      // following the last highlighted character...
      int LastRealIndex = Edit1->SelStart+Edit1->SelLength;

      PUSHSTRUCT EndState;
      utils->SetStateFlags(EditString, LastRealIndex, EndState);

      // Insert the new font-code in front of the first highlighted character.
      EditString = utils->InsertW(EditString, FontStr, iFirst+1);
      iFirst += FontStr.Length();
      iLast += FontStr.Length();

      // Strip out font codes in the highlighted range.  "Last" may return
      // shorted by the # of font-code chars removed.
      EditString = utils->StripFont(EditString, iFirst, iLast, CTRL_F);

      // Write the font code pertaining to the end state.
      if (EndState.fontType < 0)
      {
        utils->PopOnChange(Edit1);
        return;
      }

      // Insert the font-string in front of the character imediately
      // following the last highlighted char.
      String Stemp = utils->FontTypeToString(EndState.fontType);

      EditString = utils->InsertW(EditString, Stemp, iLast+1);
    }

    // Optimize the new string
    EditString = utils->Optimize(EditString, false, NO_COLOR);

    // Convert to RTF and display it in the edit-window
    utils->ConvertToRtf(EditString, NULL, Edit1, true);
  }

  Edit1->SelStart = SaveSelStart;
  Edit1->Modified = true;
  OldLength = Edit1->TextLength;
  utils->PopOnChange(Edit1);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::FontSizeCode1Click(TObject *Sender)
{
  if (Edit1->View != V_IRC && Edit1->View != V_RTF)
  {
    utils->ShowMessageU(EDITMSG[28]); // "Click or double-click an item to edit it..."
    return;
  }

  int Size = 0;

  if (!dts->SelectFontSize(String(DS[214]), Size)) return; // no font-code...

  String FontStr;

  // Write the font code pertaining to the end state.
  FontStr = utils->FontSizeToString(Size);

  utils->PushOnChange(Edit1);

  // Save carat position
  int SaveSelStart = Edit1->SelStart;

  if (Edit1->View == V_IRC)
  {
    int TempStart = Edit1->SelStart + 1;
    int TempLen = Edit1->SelLength;

    EditString = utils->InsertW(EditString, FontStr, TempStart);

    if (TempLen)
      EditString = utils->InsertW(EditString, String(CTRL_S), TempStart +
                                  TempLen + FontStr.Length());

    // Highlight special codes
    utils->EncodeHighlight(EditString, Edit1);
    Edit1->SelStart = TempStart - 1;
  }
  else // Edit1->View is V_RTF
  {
    if (!Edit1->SelLength) // Text selected?
    {
      // If not, write the font-code at the cursor.
      if (Edit1->SelStart >= 0)
      {
        int iTemp = utils->GetCodeIndex(Edit1->TextW, Edit1->SelStart);
        if (iTemp >= 0)
          EditString = utils->InsertW(EditString, FontStr, iTemp+1);
      }
      else
        EditString = utils->InsertW(EditString, FontStr, 1);
    }
    else // Text selected
    {
      // Set the cumulative text state that exists at the first character
      // in EditString.
      PUSHSTRUCT BeginningState;
      utils->SetStateFlags(EditString, STATE_MODE_FIRSTCHAR, BeginningState);

      // Write default font-size at beginning if the wing has no default
      // font-size and the user is trying to add a font-size code
      // somewhere in the wing.
      if (BeginningState.fontSize == -1)
      {
        String InitFontFmt = utils->FontSizeToString(-1);
        EditString = utils->InsertW(EditString, InitFontFmt, 1);
      }

      // Map the start and end indices of the RTF selected text to the
      // text in the buffer
      int iFirst, iLast, CI;
      if (!utils->GetCodeIndices(EditString, iFirst, iLast, CI, Edit1))
      {
        utils->PopOnChange(Edit1);
        return;
      }

      // Set the cumulative text state that exists up to the character
      // following the last highlighted character...
      int LastRealIndex = Edit1->SelStart+Edit1->SelLength;

      PUSHSTRUCT EndState;
      utils->SetStateFlags(EditString, LastRealIndex, EndState);

      // Insert the new font-code in front of the first highlighted character.
      EditString = utils->InsertW(EditString, FontStr, iFirst+1);
      iFirst += FontStr.Length();
      iLast += FontStr.Length();

      // Strip out font codes in the highlighted range.  "Last" may return
      // shorted by the # of font-code chars removed.
      EditString = utils->StripFont(EditString, iFirst, iLast, CTRL_S);

      // Write the font code pertaining to the end state.
      if (EndState.fontSize < 0)
      {
        utils->PopOnChange(Edit1);
        return;
      }

      // Insert the font-string in front of the character imediately
      // following the last highlighted char.
      String Stemp = utils->FontSizeToString(EndState.fontSize);

      EditString = utils->InsertW(EditString, Stemp, iLast+1);
    }

    // Optimize the new string
    EditString = utils->Optimize(EditString, false, NO_COLOR);

    // Convert to RTF and display it in the edit-window
    utils->ConvertToRtf(EditString, NULL, Edit1, true);
  }

  Edit1->SelStart = SaveSelStart;
  Edit1->Modified = true;
  OldLength = Edit1->TextLength;
  utils->PopOnChange(Edit1);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Push1Click(TObject *Sender)
{
  if (Edit1->SelLength) // Text selected?
    EmbraceSelection(CTRL_PUSH, CTRL_POP);
  else
    EmbraceSelection(CTRL_PUSH);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Pop1Click(TObject *Sender)
{
  if (Edit1->SelLength) // Text selected?
    EmbraceSelection(CTRL_PUSH, CTRL_POP);
  else
    EmbraceSelection(CTRL_POP);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Clear1Click(TObject *Sender)
{
  utils->PushOnChange(Edit1);
  Edit1->Clear();
  EditString = "";
  utils->PopOnChange(Edit1);

  // Failsafe in case of unmatched push/pops!
  utils->InitOnChange(Edit1, EditChange);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::BoldCtrlB1Click(TObject *Sender)
{
  EmbraceSelection(CTRL_B);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::InverseVidioClick(TObject *Sender)
{
  EmbraceSelection(CTRL_R);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::UnderlineCtrlU1Click(TObject *Sender)
{
  EmbraceSelection(CTRL_U);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::PlainTextCtrlO1Click(TObject *Sender)
{
  EmbraceSelection(CTRL_O, C_NULL);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::EmbraceSelection(int i1, int i2)
{
  if (Edit1->View != V_IRC && Edit1->View != V_RTF)
  {
    utils->ShowMessageU(EDITMSG[28]); // "Click or double-click an item to edit it..."
    return;
  }

  utils->PushOnChange(Edit1);

  // Save carat position
  int SaveSelStart = Edit1->SelStart;

  if (i2 == -1) // Have a closing character?
    i2 = i1;

  wchar_t c1 = i1;
  wchar_t c2 = i2;

  if (Edit1->View == V_IRC)
  {
    int TempStart = Edit1->SelStart + 1;
    int TempLen = Edit1->SelLength;

    EditString = utils->InsertW(EditString, String(c1), TempStart);

    if (TempLen)
      EditString = utils->InsertW(EditString, String(c2), TempStart +
                                                   TempLen + 1);

    // Highlight special codes
    utils->EncodeHighlight(EditString, Edit1);

    Edit1->SelStart = TempStart - 1;
  }
  else // Edit1->View is V_RTF
  {
    if (!Edit1->SelLength) // Text selected?
      // If not, write the code at the start of the string.
      EditString = utils->InsertW(EditString, String(c1), 1);
    else // Text selected
    {
      // Map the start and end indices of the RTF selected text to the
      // text in EditString.
      int iFirst, iLast, CI;
      if (!utils->GetCodeIndices(EditString, iFirst, iLast, CI, Edit1))
      {
        utils->PopOnChange(Edit1);
        return;
      }

      EditString = utils->InsertW(EditString, String(c2), iLast+1);
      EditString = utils->InsertW(EditString, String(c1), iFirst+1);
    }

    // Optimize the new string
    EditString = utils->Optimize(EditString, false, NO_COLOR);

    // Convert to RTF and display it in the edit-window
    utils->ConvertToRtf(EditString, NULL, Edit1, true);
  }

  Edit1->SelStart = SaveSelStart;
  Edit1->Modified = true;
  OldLength = Edit1->TextLength;
  utils->PopOnChange(Edit1);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::WingClick(TObject *Sender)
{
  InsertInEditBox((TMenuItem *)Sender);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::InsertInEditBox(TMenuItem* P)
{
  String sWing = utils->AnsiStripCRLF(WINGEDITMAINMENU[P->Tag+2]); // UTF-8 Wing

  if (Edit1->Visible)
  {
    EditString = utils->Utf8ToWide(sWing);

    // Convert to RTF and display it in the edit-window
    utils->ConvertToRtf(EditString, NULL, Edit1, true);
    SetView(V_RTF);
  }
  else if (bLeftSelected)
  {
    if (Lwlb->ItemIndex >= 0)
    {
      ReplaceItem(Lwlb, sWing);
      Rwlb->TopIndex = Lwlb->TopIndex;
    }
    else
      AddItems(sWing, "");
  }
  else // Right
  {
    if (Rwlb->ItemIndex >= 0)
    {
      ReplaceItem(Rwlb, sWing);
      Lwlb->TopIndex = Rwlb->TopIndex;
    }
    else
      AddItems("", sWing);
  }
}
//---------------------------------------------------------------------------
// COLOR PANEL CLICK TO CHANGE BORDER BG COLOR
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::EditWingPanelMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
// Called when the color-panel is clicked to change the border background color
{
  // Right button invokes a popup menu: DTS-Color->ColorCopyPasteMenu
  if (Button == mbLeft) SetBorderColor();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::SetBorderColor(void)
{
  // Close the editor if open
  HideEdit();

  EditWingPanel->Caption = "";

  int C = utils->SelectCustomColor(String(DS[175]), EditWingPanel->Color, COLOR_FORM_BORDER);

  if (C != IRCCANCEL)
  {
    ChangeBorderColors(C);
    LoadBorderRichEditBoxes(TopBottomEditString, LeftEditString, RightEditString);
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::ChangeBorderColors(int Color)
{
  borderBgColor = Color;
  utils->SetPanelColorAndCaption(EditWingPanel, borderBgColor);

  TopBottomEditString = ChangeBackgroundColor(TopBottomEditString); // Top/Bottom border
  LeftEditString = ChangeBackgroundColor(LeftEditString); // Left border
  RightEditString = ChangeBackgroundColor(RightEditString); // Right border
}
//---------------------------------------------------------------------------
WideString __fastcall TWingEditForm::ChangeBackgroundColor(WideString Text)
{
  // Here, we are passed a string which may or may not contain
  // color-codes.  We want to scan the string for an initial
  // Ctrl-K -- if it is there, we want to add the new background
  // color to that first color-specification.

  // For each char...
  for (int jj = 1; jj <= Text.Length(); jj++)
  {
    if (Text[jj] == CTRL_K)
    {
      int fg = NO_COLOR;
      int bg = NO_COLOR;

      int len = utils->CountColorSequence(Text.c_bstr(), jj-1, Text.Length(), fg, bg) + 1;

      if (bg != NO_COLOR)
      {
        WideString C;

        int bbg = utils->ConvertColor(borderBgColor, false);
        utils->WriteColors(fg, bbg, C);

        Text = Text.SubString(1, jj-1) + C +
                              Text.SubString(jj+len, Text.Length()-len);
        break;
      }
    }
  }

  return Text;
}
//---------------------------------------------------------------------------
bool __fastcall TWingEditForm::LoadBorderRichEditBoxes(WideString tb, WideString lb, WideString rb)
{
  int retVal = utils->ConvertToRtf(utils->StripCRLF(tb), NULL, TopBottomEdit, -1);

  if (retVal == 0)
    retVal = utils->ConvertToRtf(utils->StripCRLF(lb), NULL, LeftSideEdit, -1);

  if (retVal == 0)
    retVal = utils->ConvertToRtf(utils->StripCRLF(rb), NULL, RightSideEdit, -1);

  if (retVal)
  {
    ShowMessage("WingEdit:LoadBorderRichEditBoxes: " + String(retVal));
    return false;
  }

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TWingEditForm::LoadWingEditBoxes(void)
{
  try
  {
    Lwlb->Clear();
    Rwlb->Clear();

    if (dts->LeftWings->Count != dts->RightWings->Count)
    {
      utils->ShowMessageU(EDITMSG[27]); // "Wing-list is corrupt... repaired it"
      dts->RightWings = dts->LeftWings;
    }

    for (int ii = 0 ; ii < dts->TotalWings ; ii++)
      AddItems(utils->WideToUtf8(dts->LeftWings->GetString(ii)),
         utils->WideToUtf8(dts->RightWings->GetString(ii)),
               (bool)dts->LeftWings->GetTag(ii));
  }
  catch(...){return false;}

  try{Lwlb->SetFocus();}
  catch(...){}

  return(true);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::SaveWingEditBoxes(void)
{
  dts->LeftWings->Clear();
  dts->RightWings->Clear();

  dts->TotalWings = Lwlb->Items->Count;

  if (dts->TotalWings != Rwlb->Items->Count)
  {
    utils->ShowMessageU(EDITMSG[27]); // "Wing-list is corrupt... repaired it"
    Rwlb->Items = Lwlb->Items;
  }

  for (int ii = 0; ii < dts->TotalWings; ii++)
  {
    dts->LeftWings->Add(utils->Utf8ToWide(Lwlb->Items->Strings[ii]), (void*)Lwlb->Checked[ii]);
    dts->RightWings->Add(utils->Utf8ToWide(Rwlb->Items->Strings[ii]));
  }

  dts->TopBorder = TopBottomEditString;
  dts->LeftSideBorder = LeftEditString;
  dts->RightSideBorder = RightEditString;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::ImportFromMainDesignWindow1Click(TObject *Sender)
{
  // Send IRC text-line to wing
  if (dts->TaeEdit->Lines->Count != 1)
  {
    utils->ShowMessageU(EDITMSG[20]); // Import instructions
    return;
  }

  int TextLen = tae->TextLength;

  if (TextLen < 1 || TextLen > 200)
  {
    ShowMessage("Wing import text too short or too long!");
    return;
  }

  WideString S;

  // Put text into a buffer
  if (utils->IsRtfIrcOrgView())
  {
    bool bHaveFontSize = false;
    bool bHaveFontType = false;
    int FontSize = 0;
    int FontType = 0;

    // Get first line
    if (utils->IsIrcView())
      S = dts->SL_IRC->GetString(0); // 1st line
    else
      S = dts->SL_ORG->GetString(0); // 1st line

    for(int ii = 0; ii < S.Length(); ii++)
    {
      if (S[ii+1] == CTRL_F)
      {
        int val = utils->CountFontSequence(S.c_bstr(), ii, S.Length());

        if (val >= 0)
        {
          ii += 2; // bypass the code

          // Wing has at least one font-change?
          if (val > 0)
          {
            FontType = val;
            bHaveFontType = true;
          }
        }
      }
      else if (S[ii+1] == CTRL_S)
      {
        int val = utils->CountFontSequence(S.c_bstr(), ii, S.Length());

        if (val >= 0)
        {
          ii += 2; // bypass the code

          // Wing has at least one font-change?
          if (val > 0)
          {
            FontSize = val;
            bHaveFontSize = true;
          }
        }
      }
    }

    // Last detected font code is NOT a return to the default-font (0)?
    if (bHaveFontType && FontType > 0)
      S += utils->FontTypeToString(FontType);

    if (bHaveFontSize && FontSize > 0)
      S += utils->FontSizeToString(FontSize);
  }
  else
    S = tae->GetStringW(0); // Get the main edit control's first line

  if (Edit1->Visible)
  {
    EditString = utils->StripCRLF(S);

    // Convert to RTF and display it in the edit-window
    utils->ConvertToRtf(EditString, NULL, Edit1, true);
    SetView(V_RTF);
  }
  else
  {
    if (Lwlb->ItemIndex >= 0 && bLeftSelected)
    {
      ReplaceItem(Lwlb, utils->WideToUtf8(S), RI_TRUE);
      Rwlb->TopIndex = Lwlb->TopIndex;
    }
    else if (Rwlb->ItemIndex >= 0 && !bLeftSelected)
    {
      ReplaceItem(Rwlb, utils->WideToUtf8(S), RI_TRUE);
      Lwlb->TopIndex = Rwlb->TopIndex;
    }
    else
      AddItems(S);
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::AddNewWingPair(void)
{
  String s("New Wing"); // avoid ambiguous call to overloaded ShowEdit!
  ShowEdit(s, ET_ADDWING);
}
//---------------------------------------------------------------------------
// WING LIST BOX POP-UP MENU HANDLERS
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::AddNewWingPair1Click(TObject *Sender)
{
  AddNewWingPair();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::DeleteWingPair1Click(TObject *Sender)
{
  DeleteWingPair();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::DeleteWingPair(void)
{
  if (Lwlb->ItemIndex >= 0 && Rwlb->ItemIndex >= 0)
  {
    // Quit editing...
    HideEdit();
    DeleteItems(Lwlb->ItemIndex);
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::EditSelectedWing1Click(TObject *Sender)
// Menu-item click
{
  if (bLeftSelected)
  {
    if (Lwlb->ItemIndex >= 0 && Lwlb->ItemIndex < Lwlb->Items->Count)
      ShowEdit(Lwlb->Items->Strings[Lwlb->ItemIndex], ET_EDITLISTLEFT);
  }
  else
  {
    if (Rwlb->ItemIndex >= 0 && Rwlb->ItemIndex < Rwlb->Items->Count)
      ShowEdit(Rwlb->Items->Strings[Rwlb->ItemIndex], ET_EDITLISTRIGHT);
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::SetView(int NewView)
{
  // Set Edit1 OnChange handler only for RTF view...
  if (NewView == V_RTF || NewView == V_IRC)
  {
    OldLength = utils->StripCRLF(Edit1->TextW).Length();

    // Used to prevent the Edit1 OnChange event from
    // Firing when we are processing or loading text
    // Enable OnChange Event last
    //
    // (NOTE: THe TopBottom, Left/Right border edit boxes do not
    // utilize an OnChange event!)
    utils->InitOnChange(Edit1, EditChange);
  }
  else
    Edit1->OnChange = NULL;

  switch(NewView)
  {
    case V_RTF:

      View1->Caption = "View (color edit, right-click for pop-up menu)";
      View1->Enabled = true;

      // Enable menu-insert items
      SetMenuInsertItemsState(true);

      MenuViewRtf->Checked = true;
      MenuViewRtf->ShortCut = 0;
      MenuViewIrc->Checked = false;
      MenuViewIrc->ShortCut = ShortCut(Word('T'), TShiftState() << ssCtrl);

      Edit1->ReadOnly = false;

    break;

    case V_IRC:

      View1->Caption = "View (plain text edit, click Options "
                                "to insert colors)";
      View1->Enabled = true;

      // Enable menu-insert items
      SetMenuInsertItemsState(true);

      MenuViewRtf->Checked = false;
      MenuViewRtf->ShortCut = ShortCut(Word('T'), TShiftState() << ssCtrl);
      MenuViewIrc->Checked = true;
      MenuViewIrc->ShortCut = 0;

      Edit1->ReadOnly = false;

    break;

    case V_OFF:

      Edit1->Modified = false; // reset modified flag
      Edit1->ReadOnly = false;

    break;

    default:

      View1->Caption = "View";
      View1->Enabled = false;

      // Disable menu-insert items
      SetMenuInsertItemsState(false);

      MenuViewRtf->Checked = false;
      MenuViewIrc->Checked = false;

    break;
  }

  Edit1->View = NewView;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::SetMenuInsertItemsState(bool bEnable)
{
  if (bEnable)
  {
    ColorCode1->Enabled = true;
    FontSizeCode1->Enabled = true;
    FontTypeCode1->Enabled = true;
    BoldCtrlB1->Enabled = true;
    InverseVidio->Enabled = true;
    PlainTextCtrlO1->Enabled = true;
    UnderlineCtrlU1->Enabled = true;
    Push1->Enabled = true;
    Pop1->Enabled = true;
    Optimize1->Enabled = true;
  }
  else
  {
    ColorCode1->Enabled = false;
    FontSizeCode1->Enabled = false;
    FontTypeCode1->Enabled = false;
    BoldCtrlB1->Enabled = false;
    InverseVidio->Enabled = false;
    UnderlineCtrlU1->Enabled = false;
    PlainTextCtrlO1->Enabled = false;
    Push1->Enabled = false;
    Pop1->Enabled = false;
    Optimize1->Enabled = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::HideEdit(void)
{
  if (Edit1->Visible)
  {
    SetView(V_OFF);

    Edit1->Visible = false;
    SourceEditID = ET_NONE;

    // Restore hint message to require a double-click...

    // EDITMSG[13] =
    //  "Left-click an item to edit it.\n"
    //  "Right-click for the popup menu.\n"
    //  "Check the box to enable the wing-pair.\n"
    //  "Press the ESC key to abort.";
    String S = String(EDITMSG[13]);
    Lwlb->Hint = S;
    Rwlb->Hint = S;
  }
}
//---------------------------------------------------------------------------
// Overloaded
void __fastcall TWingEditForm::ShowEdit(String Text, int Type)
{
  ShowEdit(utils->Utf8ToWide(Text), Type);
}

void __fastcall TWingEditForm::ShowEdit(WideString Text, int Type)
{
  Edit1->ShowHint = true;

  EditString = utils->StripCRLF(Text);
  SavedEditString = EditString;

  // avoid colors becoming Afg/ABg...
  utils->ConvertToRtf(EditString, NULL, Edit1, true);

  // Set hint message to require only a single-click...

  // EDITMSG[15] = "Left-Click to Edit. Right-click for the popup menu.";
  String S = String(EDITMSG[15]);
  Lwlb->Hint = S;
  Rwlb->Hint = S;

  Edit1->Visible = true;
  Edit1->SetFocus();
  SourceEditID = Type;
  SetView(V_RTF);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::ClearEntireList1Click(TObject *Sender)
{
  Lwlb->Clear();
  Rwlb->Clear();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Exit1Click(TObject *Sender)
{
  SaveAll();
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Quit1Click(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Button2Click(TObject *Sender)
{
  // Launch Character Map
  ShellExecute(Handle, "open", (char*)EDITMSG[30], NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Button3Click(TObject *Sender)
{
  // Launch Font-Editor
  ShellExecute(Handle, "open", (char*)EDITMSG[31], NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::FormClick(TObject *Sender)
{
  HideEdit();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Copy1Click(TObject *Sender)
{
  if (Rwlb->ItemIndex >= 0 && !bLeftSelected)
    utils->CopyTextToClipboard(GetItem(Rwlb));
  else if (Lwlb->ItemIndex >= 0 && bLeftSelected)
    utils->CopyTextToClipboard(GetItem(Lwlb));

  if (Clipboard()->AsText.Length())
    Paste1->Enabled = true;
}
//---------------------------------------------------------------------------
// MOUSE UP DOWN/CLICK HANDLERS
//---------------------------------------------------------------------------

//void __fastcall TWingEditForm::Edit1MouseDown(TObject *Sender,
//      TMouseButton Button, TShiftState Shift, int X, int Y)
//{
//  if (bHideCaret)
//    HideCaret(Edit1->Handle);
//
//  // Showing codes in non-edit mode and left-click?
//  // Switch to RTF view...
//  if (Edit1->View == VS_CODES && Button == mbLeft)
//    {
//    EditString = utils->StripCRLF(EditString);
//
//    // avoid colors becoming Afg/ABg...
//    utils->ConvertToRtf(EditString, Edit1, true);
//    SetEdit1->View(V_RTF);
//    }
//}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Edit1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
  // Turn off annoying hint message after we click on the mouse...
  Edit1->ShowHint = false;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::TopBottomEditMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
  bool bVis = Edit1->Visible;

  if (bVis && SourceEditID == ET_EDITTOPBOTTOM)
    HideEdit();
  else
  {
    ShowEdit(TopBottomEditString, ET_EDITTOPBOTTOM);
    TopBottomEdit->ShowHint = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::LeftSideEditMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
  bool bVis = Edit1->Visible;

  if (bVis && SourceEditID == ET_EDITLEFT)
    HideEdit();
  else
  {
    ShowEdit(LeftEditString, ET_EDITLEFT);
    LeftSideEdit->ShowHint = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::RightSideEditMouseDown(TObject *Sender,
                        TMouseButton Button, TShiftState Shift, int X, int Y)
{
  bool bVis = Edit1->Visible;

  if (bVis && SourceEditID == ET_EDITRIGHT)
    HideEdit();
  else
  {
    ShowEdit(RightEditString, ET_EDITRIGHT);
    RightSideEdit->ShowHint = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::RwlbClick(TObject *Sender)
{
  Lwlb->TopIndex = Rwlb->TopIndex;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::LwlbClick(TObject *Sender)
{
  Rwlb->TopIndex = Lwlb->TopIndex;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::ListBoxDblClick(TObject *Sender)
{
  // Tell ListBoxMouseDown we had a doubleclick
  bDoubleclick = true;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::LwlbMouseDown(TObject *Sender,
                        TMouseButton Button, TShiftState Shift, int X, int Y)
{
  Lwlb->ShowHint = false;
  Rwlb->ShowHint = true;

  Lwlb->Color = dts->TaeWindowColor;
  Rwlb->Color = DTS_GRAY;

  bLeftSelected = true;

  if (Button != mbLeft)
    return;

  int Index = Lwlb->ItemAtPos(Point(X,Y), true);

  // Return unless we clicked on a new item...
  if (SourceEditID == ET_EDITLISTLEFT && CurrentIndex == Index && Edit1->Visible)
    return;

  // Save edit in-progress
  SaveEdit();

  if (Index < 0)
  {
    Lwlb->ItemIndex = -1;
    Rwlb->ItemIndex = -1;

    HideEdit(); // Exit the editor if no wing selected.
  }
  else
  {
    Lwlb->ItemIndex = Index;

    if (Edit1->Visible || bDoubleclick)
      ShowEdit(Lwlb->Items->Strings[Lwlb->ItemIndex], ET_EDITLISTLEFT);

    if (Index < Rwlb->Items->Count)
      Rwlb->ItemIndex = Index;
    else
      Rwlb->ItemIndex = -1;
  }

  CurrentIndex = Rwlb->ItemIndex;

  bDoubleclick = false;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::RwlbMouseDown(TObject *Sender,
                         TMouseButton Button, TShiftState Shift, int X, int Y)
{
  Rwlb->ShowHint = false;
  Lwlb->ShowHint = true;

  Rwlb->Color = dts->TaeWindowColor;
  Lwlb->Color = DTS_GRAY;

  bLeftSelected = false;

  if (Button != mbLeft)
    return;

  int Index = Rwlb->ItemAtPos(Point(X,Y), true);

  // Return unless we clicked on a new item...
  if (SourceEditID == ET_EDITLISTRIGHT && CurrentIndex == Index &&
                                                              Edit1->Visible)
    return;

  // Save edit in-progress
  SaveEdit();

  if (Index < 0)
  {
    Lwlb->ItemIndex = -1;
    Rwlb->ItemIndex = -1;

    HideEdit(); // Exit the editor if no wing selected.
  }
  else
  {
    Rwlb->ItemIndex = Index;

    if (Edit1->Visible || bDoubleclick)
      ShowEdit(Rwlb->Items->Strings[Rwlb->ItemIndex], ET_EDITLISTRIGHT);

    if (Index < Lwlb->Items->Count)
      Lwlb->ItemIndex = Index;
    else
      Lwlb->ItemIndex = -1;
  }

  CurrentIndex = Lwlb->ItemIndex;

  bDoubleclick = false;
}
//---------------------------------------------------------------------------
int __fastcall TWingEditForm::SaveEdit(void)
{
  int retVal = 100;

  if (Edit1->Visible && Edit1->Modified)
  {
    // ET_NONE 0
    // ET_EDITTOPBOTTOM 1
    // ET_EDITLEFT 2
    // ET_EDITRIGHT 3
    // ET_EDITLISTLEFT 4
    // ET_EDITLISTRIGHT 5
    // ET_ADDWING 6

    // Optimize
    EditString = utils->Optimize(EditString, false, NO_COLOR);

    EditString = utils->StripCRLF(EditString);

    if (SourceEditID == ET_EDITTOPBOTTOM)
    {
      TopBottomEditString = EditString;
      utils->ConvertToRtf(TopBottomEditString, NULL, TopBottomEdit, -1);
    }
    else if (SourceEditID == ET_EDITLEFT)
    {
      LeftEditString = EditString;
      utils->ConvertToRtf(LeftEditString, NULL, LeftSideEdit, -1);
    }
    else if (SourceEditID == ET_EDITRIGHT)
    {
      RightEditString = EditString;
      utils->ConvertToRtf(RightEditString, NULL, RightSideEdit, -1);
    }
    else if (SourceEditID == ET_ADDWING)
    {
      try
      {
        AddItems(utils->WideToUtf8(EditString));
        retVal = 0;
      }
      catch(...){retVal = 101;}
    }
    else if (SourceEditID == ET_EDITLISTLEFT)
    {
      if (Lwlb->ItemIndex >= 0)
      {
        try
        {
          ReplaceItem(Lwlb, utils->WideToUtf8(EditString));
          Rwlb->TopIndex = Lwlb->TopIndex;
          retVal = 0;
        }
        catch(...){retVal = 102;}
      }
    }
    else if (SourceEditID == ET_EDITLISTRIGHT)
    {
      if (Rwlb->ItemIndex >= 0)
      {
        try
        {
          ReplaceItem(Rwlb, utils->WideToUtf8(EditString));
          Lwlb->TopIndex = Rwlb->TopIndex;
          retVal = 0;
        }
        catch(...){retVal = 103;}
      }
    }
    else retVal = 104; // Unknown mode
  }

  return retVal;
}
//---------------------------------------------------------------------------
// Paste a wing (strings in our custom check-list-box are UTF-8,
// we do a custom OwnerDraw to display them)
void __fastcall TWingEditForm::Paste1Click(TObject *Sender)
{
  String sText = utils->WideToUtf8(utils->GetClipboardText());

  if (sText.IsEmpty())
    return;

  try
  {
    if (!bLeftSelected) // right?
    {
      if (Rwlb->ItemIndex < 0) // trying to add new pair
      {
        Rwlb->Items->Add(sText);
        Lwlb->Items->Add("");
        Lwlb->Checked[Lwlb->Items->Count-1] = true;
      }
      else  // replacing an existing wing
      {
        ReplaceItem(Rwlb, sText, RI_TRUE);
        Lwlb->TopIndex = Rwlb->TopIndex;
      }
    }
    else // left
    {
      if (Lwlb->ItemIndex < 0) // trying to add new pair
        AddItems(sText, "");
      else  // replacing an existing wing
      {
        ReplaceItem(Lwlb, sText, RI_TRUE);
        Rwlb->TopIndex = Lwlb->TopIndex;
      }
    }
  }
  catch(...)
  {
    ShowMessage("Error Pasting Wing... clearing all wingdings...");
    Lwlb->Clear();
    Rwlb->Clear();
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::ReplaceItem(void* p, String S, int riFlag)
// Rewrite listbox UTF-8 string, riFlag always refers to Lwlb (left-wings list-box)
// since it's the only check-listbox
//
// riFlag types:
// RI_NONE  0 // Don't set the flag
// RI_TRUE  1
// RI_FALSE 2
{
  try
  {
    TCheckListBox* lb = (TCheckListBox*)p;

    if (!lb || CurrentIndex >= lb->Items->Count)
      return;

    bool bSaveChecked;

    bool bInRange = CurrentIndex < Lwlb->Items->Count && CurrentIndex >= 0;

    if (bInRange)
      bSaveChecked = Lwlb->Checked[CurrentIndex];
    else
      bSaveChecked = false;;

    lb->Items->BeginUpdate();
    lb->Items->Strings[CurrentIndex] = S;
    lb->Items->EndUpdate();

    // Set the Left box's check-box
    if (riFlag == RI_NONE)
      Lwlb->Checked[CurrentIndex] = bSaveChecked;
    else if (bInRange)
    {
      Lwlb->Items->BeginUpdate();

      if (riFlag == RI_TRUE)
        Lwlb->Checked[CurrentIndex] = true;
      else
        Lwlb->Checked[CurrentIndex] = false;

      Lwlb->Items->EndUpdate();
    }
  }
  catch(...){}
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::AddItems(String S, bool bChecked)
// Add listbox left, right UTF-8 strings, bChecked defaults true
{
  AddItems(S, S, bChecked);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::AddItems(String Sl, String Sr, bool bChecked)
// Add listbox left, right UTF-8 strings, bChecked defaults true
{
  try
  {
    Rwlb->Items->BeginUpdate();
    Lwlb->Items->BeginUpdate();
    Lwlb->Items->Add(Sl);
    Lwlb->Checked[Lwlb->Items->Count-1] = bChecked;
    Rwlb->Items->Add(Sr);
    Rwlb->Items->EndUpdate();
    Lwlb->Items->EndUpdate();
  }
  catch(...) { }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::DeleteItems(int idx)
// Delete items at idx, if idx == -1, delete current items
{
  try
  {
    if (idx < 0)
      idx = Lwlb->ItemIndex;

    Rwlb->Items->BeginUpdate();
    Lwlb->Items->BeginUpdate();

    if (idx < Lwlb->Items->Count)
      Lwlb->Items->Delete(idx);

    if (idx < Rwlb->Items->Count)
      Rwlb->Items->Delete(idx);

    Rwlb->Items->EndUpdate();
    Lwlb->Items->EndUpdate();
  }
  catch(...) { }
}
//---------------------------------------------------------------------------
WideString __fastcall TWingEditForm::GetItem(void* p)
// Get current item
{
  TListBox* lb = (TListBox*)p;

  if (!lb)
    return "";

  WideString sTemp;

  try
  {
    int idx = lb->ItemIndex;

    if (idx >= 0 && idx < lb->Items->Count)
      sTemp = utils->Utf8ToWide(lb->Items->Strings[idx]);
  }
  catch(...) { }

  return sTemp;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Undo1Click(TObject *Sender)
{
  HideEdit(); // close edit window without saving
  LoadWingEditBoxes();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Undo2Click(TObject *Sender)
{
  if (Edit1->Visible && (Edit1->View == V_RTF || Edit1->View == V_IRC))
    ShowEdit(SavedEditString, Edit1->Tag);
}
//---------------------------------------------------------------------------
// MENU EDIT-BOX HANDLERS
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::MenuEditCutClick(TObject *Sender)
// Cut
{
  EditCutCopy(true, true);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::MenuEditCopyClick(TObject *Sender)
// Copy
{
  EditCutCopy(false, true);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::MenuEditPasteClick(TObject *Sender)
{
  EditPaste();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::MenuEditDeleteClick(TObject *Sender)
// Delete
{
  EditCutCopy(true, false);
}
//---------------------------------------------------------------------------
PASTESTRUCT __fastcall TWingEditForm::EditCutCopy(bool bCut, bool bCopy)
// Returns the +/- change in the edit-control's length for the present
// view.
{
  PASTESTRUCT ps;

  if (Edit1->SelLength == 0)
  {
    // "You must select text to perform this operation!", //25
    utils->ShowMessageU(EDITMSG[25]);
    return ps;
  }

  if (Edit1->View == V_RTF)
    // Erases clipboard then copies Html, Rtf and Utf-16 text formats to it
    ps = utils->CutCopyRTF(bCut, bCopy, Edit1, EditString);
  else
    // Erases clipboard then copies Utf-16 text format to it
    ps = utils->CutCopyIRC(bCut, bCopy, Edit1, EditString);

  if (ps.error == 0)
  {
    if (bCut)
    {
      utils->SetOldLineVars(Edit1);

      // No more text
      if (!Edit1->TextLength || !EditString.Length())
        Edit1->Clear();

      Edit1->Modified = true;
    }

    return ps;
  }

  ShowMessage("CutCopy() in DlgReplaceText.cpp, Can't cut/copy text.\n"
         "Code: " + String(ps.error));

  return ps;
}
//---------------------------------------------------------------------------
PASTESTRUCT __fastcall TWingEditForm::EditPaste(void)
{
  PASTESTRUCT ps;

  try { ps = EditPaste(utils->GetClipboardText()); }
  catch(...) { ps.error = -1; } // error
  return ps;
}
//---------------------------------------------------------------------------
PASTESTRUCT __fastcall TWingEditForm::EditPaste(WideString S)
// Returns the +/- change in the edit-control's length
{
  PASTESTRUCT ps;

  // Just want one line!
  S = utils->StripCRLF(S);

  if (S.Length() == 0)
  {
    ps.error = -1;
    return ps;
  }

  if (Edit1->View != V_RTF && Edit1->View != V_IRC)
  {
    // "Operation not allowed in this View!", //24
    utils->ShowMessageU(EDITMSG[24]);
    ps.error = -3;
    return ps;
  }

  // If the user is pasting raw-codes only into an empty control
  // we want to be in V_IRC, not V_RTF! (EditFind should never be in V_RTF!)
  if (Edit1->View == V_RTF && Edit1->TextLength == 0 &&
                                                utils->GetRealLength(S) == 0)
    SetView(V_IRC);

  // Strip codes if it's a Find and the main view is V_RTF
  if (tae->View == V_RTF && Edit1->Name == "EditFind")
    S = utils->StripAllCodes(S);

  // Save original position and carat
  TTaePosition* p = new TTaePosition(Edit1);
  p->SavePos = p->Position;
  int SaveSelStart = Edit1->SelStart;

  // Replacing text?
  if (Edit1->SelLength)
  {
    ps = EditCutCopy(true, false); // Cut old text

    if (ps.error != 0)
    {
      delete p;
      return ps; // Error
    }

    // Note: don't need to set delta and lines in ps heEdit1 because
    // what we cut won't affect where the carat is placed following
    // a paste (below)!

    utils->PushOnChange(Edit1);
    Edit1->SelTextW = "";
    Edit1->Modified = true;

    // Reset for OnChange Event...
    utils->SetOldLineVars(Edit1);

    utils->PopOnChange(Edit1);
  }

  if (Edit1->View == V_RTF)
  {
    try
    {
      utils->PushOnChange(Edit1);

      // Resolve color and style states between new and old text (returns
      // EditString by reference!)
      if (utils->ResolveStateForPaste(p->Position.p, EditString, S) >= 0)
      {
        ps.delta = utils->GetRealLength(S);
        ps.lines = utils->CountCRs(S);

        // Convert to RTF and display it in the edit-window
        utils->ConvertToRtf(EditString, NULL, Edit1, true);
        SetView(V_RTF);

        // Restore Edit1 original position and carat
        p->Position = p->SavePos;

        // Move SelStart to end of pasted text
        if (ps.error == 0)
        {
          Edit1->SelStart = SaveSelStart + (ps.delta+ps.lines);

          // ...stop the pesky "phantom" colored space at the end of
          // RTF text
          if (Edit1->SelStart == (int)Edit1->TextLength-(int)Edit1->Line)
          {
            Edit1->SelLength = 1;
            Edit1->SelAttributes->BackColor = clWindow;
            Edit1->SelLength = 0;
          }
        }

        utils->SetOldLineVars(Edit1);
        Edit1->Modified = true;
      }
      else
      {
        ps.error = -4;
      }

      utils->PopOnChange(Edit1);
    }
    catch(...)
    {
      ShowMessage("Error Pasting Into Edit Box...");
      ps.error = -5;
    }
  }
  else // Edit1->View is V_IRC...
  {
    utils->PushOnChange(Edit1);

    // One-to-one code match for IRC View!
    ps.lines = utils->CountCRs(S); // Count cr/lfs
    ps.delta = S.Length(); // Length includes 2 for each cr/lf!

    if (Edit1->View == V_IRC)
    {
      EditString = utils->InsertW(EditString, S,
                          Edit1->SelStart + utils->GetLine(Edit1) + 1);

      // NOTE: Don't call the optimizer! We may be replacing a simple code
      // that will be optimized out!!!

      utils->EncodeHighlight(EditString, Edit1);
    }
    else
      Edit1->SelTextW = S;

    // Restore original position and carat
    p->Position = p->SavePos;

    // Move SelStart to end of pasted text
    if (ps.error == 0)
      Edit1->SelStart = SaveSelStart + (ps.delta-ps.lines);

    // Reset for OnChange Event...
    utils->SetOldLineVars(Edit1);
    Edit1->Modified = true;
    utils->PopOnChange(Edit1);
  }

  delete p;
  return ps;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::MenuEditSelectAllClick(TObject *Sender)
{
  if (!Edit1->Visible || EditString.IsEmpty())
    return;

  // Select All Click
  if (Edit1->View != V_RTF || Edit1->View == V_IRC)
  {
    // "Operation not allowed in this View!", //24
    utils->ShowMessageU(EDITMSG[24]);
    return;
  }

  if (Edit1->TextLength)
  {
    Edit1->SelStart = 0;
    Edit1->SelLength = Edit1->TextLength - Edit1->LineCount + 1;
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::MenuViewRtfClick(TObject *Sender)
{
  if (Sender != NULL && Edit1->View == V_RTF) return;

  Insert1->Enabled = true;

  int iFirst;

  // Set Cursor in rich-text to the same point as cursor in plain text
  if (Edit1->SelStart == (int)Edit1->TextLength - Edit1->LineCount + 1)
    iFirst = -1; // set cursor to end
  else
    iFirst = utils->GetRealIndex(EditString, Edit1->SelStart+Edit1->Line);

  // avoid colors becoming Afg/ABg...
  utils->ConvertToRtf(EditString, NULL, Edit1, true);
  SetView(V_RTF);

  Edit1->SelStart = iFirst;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::MenuViewIrcClick(TObject *Sender)
{
  if (Sender != NULL && Edit1->View == V_IRC)
    return;

  Insert1->Enabled = true;

  // Optimize
  EditString = utils->Optimize(EditString, false, NO_COLOR);

  // Set Cursor in plain-text to the same point as cursor in RTF text
  int iFirst = utils->GetCodeIndex(EditString, Edit1->SelStart);

  // Highlight special codes
  utils->EncodeHighlight(EditString, Edit1);

  SetView(V_IRC);

  Edit1->SelStart = iFirst;

  //!!!!!!!!!!!!!!!!!
  //ShowMessage("IRC:" + EditString);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::MenuEditQuitClick(TObject *Sender)
{
  HideEdit();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::MenuEditSaveClick(TObject *Sender)
{
  SaveEdit();
  HideEdit();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::MenuEditToggleClick(TObject *Sender)
{
  if (Edit1->View == V_OFF)
    return;

  int NewState = Edit1->View+1;

  if (NewState > V_IRC) NewState = V_RTF;

  switch(NewState)
  {
    case V_RTF: MenuViewRtfClick(NULL); break;
    case V_IRC: MenuViewIrcClick(NULL); break;
  }

  SetView(NewState);
}
//---------------------------------------------------------------------------

//void __fastcall TWingEditForm::Edit1KeyUp(TObject *Sender, WORD &Key,
//      TShiftState Shift)
//{
//  if (bHideCaret)
//    HideCaret(Edit1->Handle);
//}
//---------------------------------------------------------------------------
// KEY PRESS HANDLERS
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::WingsListBoxKeyDown(TObject *Sender, WORD &Key,
                                                            TShiftState Shift)
{
  if (Key == VK_DELETE)
  {
    DeleteWingPair();
    Key = 0;
  }
  else if (Key == VK_INSERT && !Edit1->Visible)
  {
    TListBox* P = (TListBox *)Sender;

    if (P->ItemIndex < 0)
    {
      P->ItemIndex = 0;
      CurrentIndex = 0;
    }

    ShowEdit(P->Items->Strings[P->ItemIndex],
                  bLeftSelected ? ET_EDITLISTLEFT : ET_EDITLISTRIGHT);

    Key = 0;
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::LwlbKeyPress(TObject *Sender, char &Key)
{
  if (Char(Key) == VK_ESCAPE)
  {
    LoadWingEditBoxes();
    Key = 0;
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::RwlbKeyPress(TObject *Sender, char &Key)
{
  if (Char(Key) == VK_ESCAPE)
  {
    LoadWingEditBoxes();
    Key = 0;
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Edit1KeyPress(TObject *Sender, char &Key)
{
  if (Key == C_TAB)
  {
    PASTESTRUCT ps = EditPaste(utils->GetTabStringW(dts->RegTabs));

    if (ps.error == 0)
      Edit1->SelStart += ps.delta-ps.lines;

    Key = 0;
    return;
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Edit1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
//dts->TaeEdit->Lines->Add(IntToHex(Key,8));

  // this allows shift+Ins to paste but blocks turning overstrike-mode on!
  if (Key == VK_INSERT)
  {
    if (Shift.Contains(ssShift))
      // Shift+Ins
      EditPaste();
    Key = 0;
  }
// can't get this to work...
//  else if (Shift.Contains(ssCtrl) && Key == 'V')
//  {
//dts->TaeEdit->Lines->Add(IntToHex(int(Key),8));
    // Shift+Ins
//    EditPaste();
//    Key = 0;
//  }
  else if (Key == VK_DELETE)
  {
    if (Edit1->SelLength)
    {
      if (Shift.Contains(ssShift))
        EditCutCopy(true, true); // Shift+Del
      if (Edit1->SelLength)
        EditCutCopy(true, false); // Del

      Key = 0;
    }
  }
  else if (Key == VK_RETURN)
  {
    SaveEdit();
    HideEdit();
    Key = 0;
  }
  else if (Key == VK_ESCAPE)
  {
    HideEdit();
    Key = 0;
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
// IMPORT/EXPORT WINGS & BORDERS
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::ImportFromTextFile1Click(TObject *Sender)
{
  CloseOpenSaveDialogs();

  // User clicked Import Text button
  try
  {
    WideString wFilter = WideString("All files (*.*)|*.*|"
                    "Text files (*.txt)|*.txt");

    // need this if this form closes so that we can delete the dialog...
    bOpenFileDialogOpen = true;

    // Run the open-file dialog in FormOFDlg.cpp
    WideString wFile = utils->GetOpenFileName(wFilter, 2, dts->DeskDir, String(DS[163]));

    bOpenFileDialogOpen = false;

    // Load and convert file as per the file-type (either plain or rich text)
    if (wFile.IsEmpty())
      return;

    TStringList* ImportList = new TStringList();

    ImportList->LoadFromFile(wFile);

    // First string says "YahCoLoRiZe wing-export identifier!"
    // Next 3 are TopBottom/Left/Right
    // Then are 2 spare strings...
    if (ImportList->Count < 6  || ImportList->Strings[0] != String(WING_EXPORT_ID))
    {
      utils->ShowMessageU("Invalid file. The first string in the file must be:\n"
              "\"" + String(WING_EXPORT_ID) + "\"");
      delete ImportList;
      return;
    }

    HideEdit();

    TopBottomEditString = dts->TopBorder = utils->Utf8ToWide(ImportList->Strings[1]);
    LeftEditString = dts->LeftSideBorder = utils->Utf8ToWide(ImportList->Strings[2]);
    RightEditString = dts->RightSideBorder = utils->Utf8ToWide(ImportList->Strings[3]);

    // Strings 4 and 5 are spares!!!!!!!!!!

    LoadBorderRichEditBoxes(TopBottomEditString, LeftEditString, RightEditString);

    Lwlb->Clear();
    Rwlb->Clear();

    String Str, EnaStr;
    bool bEna;

    // Loading UTF-8 from file into UTF-8 strings in the list-boxes...
    for (int ii = WING_EXPORT_CONST; ii < ImportList->Count; ii++)
    {
      try
      {
        // First char is 1/0 enabled or disabled
        EnaStr = ImportList->Strings[ii].SubString(1,1);
        bEna = EnaStr.ToIntDef(0); // default is 0 (disabled)

        Str = ImportList->Strings[ii].SubString(2, ImportList->Strings[ii].Length()-1);

        // Even items are left-wings, odd items are right-wings
        if (ii & 1)
          Rwlb->Items->Add(Str);
        else
        {
          Lwlb->Items->Add(Str);
          Lwlb->Checked[(ii-WING_EXPORT_CONST)/2] = bEna;
        }
      }
      catch(...) { }
    }

    delete ImportList;

    // We don't save this in the text file
    ChangeBorderColors(BORDERBACKGROUNDINIT);
    LoadBorderRichEditBoxes(TopBottomEditString, LeftEditString,
                                                     RightEditString);
    }
  catch(...)
  {
    utils->ShowMessageU(String(EDITMSG[21]));
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::ExportToTextFile1Click(TObject *Sender)
{
  CloseOpenSaveDialogs();

  // User clicked Export Text button
  if (!Lwlb->Items->Count && !Rwlb->Items->Count)
    return;

  try
  {
    WideString wFilter = WideString("All files (*.*)|*.*|"
                                     "Text files (*.txt)|*.txt");

    // need this if this form closes so that we can delete the dialog...
    bSaveFileDialogOpen = true;

    WideString wFile = utils->GetSaveFileName(wFilter, L"Wings.txt",
                                   dts->DeskDir, String(EDITMSG[22]));

    bSaveFileDialogOpen = false;

    if (wFile.IsEmpty())
      return; // Cancel

    if (utils->FileExistsW(wFile))
      if (utils->ShowMessageU(dts->Handle, DS[160],
                      MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1) == IDNO)
        return; // Cancel

    // open or create file
    TStringList* ExportList = new TStringList();

    String S;

    // Export the ID
    ExportList->Add(String(WING_EXPORT_ID));

    // Export the borders
    ExportList->Add(utils->WideToUtf8(TopBottomEditString));
    ExportList->Add(utils->WideToUtf8(LeftEditString));
    ExportList->Add(utils->WideToUtf8(RightEditString));

    // Strings 4 and 5 are spares!!!!!!!!!!
    ExportList->Add("spare 1");
    ExportList->Add("spare 2");

    int Count = Lwlb->Items->Count > Rwlb->Items->Count ? Lwlb->Items->Count : Rwlb->Items->Count;

    // UTF-8 list to UTF-8 List... no conversion needed!
    for (int ii = 0; ii < Count; ii++)
    {
      if (ii < Lwlb->Items->Count)
      {
        S = String((int)Lwlb->Checked[ii]) + Lwlb->Items->Strings[ii];
        ExportList->Add(S);
      }
      else
        ExportList->Add("<empty>");

      if (ii < Rwlb->Items->Count)
      {
        S = "0" + Rwlb->Items->Strings[ii];
        ExportList->Add(S);
      }
      else
        ExportList->Add("<empty>");
    }

    utils->WriteStringToFileW(wFile, ExportList->Text);

    delete ExportList;
  }
  catch(...) { utils->ShowMessageU(String(EDITMSG[23])); }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::ListBoxExit(TObject *Sender)
{
  // Turn off annoying hint message after we leave the 1st time...
  TTaeRichEdit* p = (TTaeRichEdit*)Sender;
  p->ShowHint = false;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Help1Click(TObject *Sender)
{
  utils->ShowMessageU(EDITMSG[26]);
}
//---------------------------------------------------------------------------
int __fastcall TWingEditForm::GetBorderBgColor(void)
{
  return borderBgColor;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::SetBorderBgColor(int Color)
{
  ChangeBorderColors(Color);
  LoadBorderRichEditBoxes(TopBottomEditString, LeftEditString, RightEditString);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::Optimize1Click(TObject *Sender)
{
  if (Edit1->Visible)
  {
    // Optimize the new string
    EditString = utils->Optimize(EditString, false, NO_COLOR);

    if (Edit1->View == V_IRC)
      MenuViewIrcClick(NULL);
    else
      MenuViewRtfClick(NULL);
  }
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::RwlbDrawItem(TWinControl *Control, int Index,
                                      const TRect &Rect, TOwnerDrawState State)
{
  if (!Rwlb->Canvas->TryLock())
    return;

  WideString wOut;

  String sIn = Rwlb->Items->Strings[Index]; // this is in utf-8!

  if (!sIn.IsEmpty())
    wOut = utils->Utf8ToWide(sIn);

  Convert(wOut, Rect, Rwlb->Canvas);

  if (State.Contains(odFocused))
    Rwlb->Canvas->DrawFocusRect(Rwlb->ItemRect(Index));

  Rwlb->Canvas->Unlock();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::LwlbDrawItem(TWinControl *Control,
                          int Index, const TRect &Rect, TOwnerDrawState State)
{
  if (!Lwlb->Canvas->TryLock())
    return;

  WideString wOut;

  String sIn = Lwlb->Items->Strings[Index]; // this is in utf-8!

  if (!sIn.IsEmpty())
    wOut = utils->Utf8ToWide(sIn);

  Convert(wOut, Rect, Lwlb->Canvas);

  if (State.Contains(odFocused))
  {
    // erase artifact around the item
    Lwlb->Canvas->DrawFocusRect(Lwlb->ItemRect(Index));

    // I added the CheckRect to MyCheckLst.pas and actually rolled my own
    // MyCheckListBox derrived component here...
    // (note: the compiler generates us a  MyCheckLst.hpp file to include)
    
    // Erase artifact around the checkbox
    Lwlb->Canvas->DrawFocusRect(Lwlb->CheckRect[Index]);

//#if DEBUG_ON
//    DTSColor->CWrite( "\r\n" + String(RCheck.Left) + " " +
//      String(RCheck.Right) + " " + String(RCheck.Top) + " " +
//         String(RCheck.Bottom) + "\r\n");
//#endif
  }

  Lwlb->Canvas->Unlock();
}
//---------------------------------------------------------------------------
//HPEN __fastcall TWingEditForm::CreateFocusPen()
//{
//  LONG width(1);
//  SystemParametersInfo(SPI_GETFOCUSBORDERHEIGHT, 0, &width, 0);
//  LOGBRUSH lb = { };     // initialize to zero
//  lb.lbColor = 0xffffff; // white
//  lb.lbStyle = BS_SOLID;
//  return ExtCreatePen(PS_GEOMETRIC | PS_DOT, width, &lb, 0, 0);
//}
//---------------------------------------------------------------------------
//void __fastcall TWingEditForm::DrawFocusRect(TCanvas* c, HPEN hpen, TRect &r)
//{
//  HDC hdc = c->Handle;
//  HPEN old_pen = SelectObject(hdc, hpen);
//  int old_rop = SetROP2(hdc, R2_XORPEN);
//  c->FrameRect(r);
//  SelectObject(hdc, old_pen);
//  SetROP2(hdc, old_rop);
//}
//---------------------------------------------------------------------------
/*
  char szSize[100];
    char szTitle[] = "These are the dimensions of your client area:";
    HFONT hfOld = SelectObject(hdc, hf);

    SetBkColor(hdc, g_rgbBackground);
    SetTextColor(hdc, g_rgbText);

    if(g_bOpaque)
    {
        SetBkMode(hdc, OPAQUE);
    }
    else
    {
        SetBkMode(hdc, TRANSPARENT);
    }

    DrawText(hdc, szTitle, -1, prc, DT_WORDBREAK);

    wsprintf(szSize, "{%d, %d, %d, %d}", prc->left, prc->top, prc->right, prc->bottom);
    DrawText(hdc, szSize, -1, prc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    SelectObject(hdc, hfOld);
*/
bool __fastcall TWingEditForm::Convert(WideString wIn, const TRect &rect, TCanvas* p)
// ShowMessage(String(GetACP())); // returns 1252 Ansi Code Page
// 1200 is utf-16 65001 is utf-8
// so with one-byte, in the TFont->Charset - try 01 hex for utf-8 and B0 hex for utf-16 ???
{
  bool bRet = false;
  wchar_t* buf = NULL;
  PUSHSTRUCT* ps = NULL;
  TRect r = rect;

  try
  {

//#if DEBUG_ON
//  if (hf)
//    CWrite("\r\nFont handle is non-zero\r\n");
//#endif
// This is defined in Borland's help as having values up to OEM_CHARSET which is 255
//    p->Font->Charset = 176; // code-page 1200 is UTF-16, 0x4B0 - the B0 part is 176
//    p->Font->Charset = 233; // code-page 65001 is UTF8, 0xFDE9 - the E9 part is 233
//----------------------------------------------
//    p->Font->Charset = ANSI_CHARSET;
//    p->TextFlags = ETO_OPAQUE;

    wchar_t c;

    WideString wOut;

    PUSHSTRUCT* ps = new PUSHSTRUCT(); // instantiation calls Clear()...

    int len = wIn.Length();

    wchar_t* buf = utils->StrNewW(wIn.c_bstr(), len);

    int outLen;

    // Convert
    for (int ii = 0; ii < len; ii++)
    {
      c = buf[ii];

      if ((unsigned)c >= (unsigned)C_SPACE)
      {
        wOut += c;
        continue;
      }

      // Write out chars we have so-far...
      outLen = wOut.Length();

      if (outLen > 0)
      {
        // (NOTE: Check out ExtTextOutW)
        // NOTE: If the Window was created with RegisterClassA you won't
        // get all of the font-renderings...
        ::TextOutW(p->Handle, r.Left, r.Top, wOut.c_bstr(), outLen);
        r.Left += p->TextWidth(wOut);
        wOut.Empty();
      }

      if (c == CTRL_O)
      {
        p->Font->Style.Clear();
        ps->Clear();
      }
      else if (c == CTRL_B)
      {
        if (p->Font->Style.Contains(fsBold))
          p->Font->Style << fsBold;
        else
          p->Font->Style >> fsBold;

        ps->bBold = !ps->bBold;
      }
      else if (c == CTRL_U)
      {
        if (p->Font->Style.Contains(fsUnderline))
          p->Font->Style << fsUnderline;
        else
          p->Font->Style >> fsUnderline;

        ps->bUnderline = !ps->bUnderline;
      }
      else if (c == CTRL_R)
      {
        if (p->Font->Style.Contains(fsItalic))
          p->Font->Style << fsItalic;
        else
          p->Font->Style >> fsItalic;

        ps->bItalics = !ps->bItalics;
      }
      else if (c == CTRL_PUSH)
      {
        if (!p->Font->Style.Contains(fsStrikeOut))
          p->Font->Style << fsStrikeOut;

        ps->pushCounter++;
      }
      else if (c == CTRL_POP)
      {
        if (ps->pushCounter > 0)
          if (--ps->pushCounter == 0)
            if (p->Font->Style.Contains(fsStrikeOut))
              p->Font->Style >> fsStrikeOut;
      }
      else if (c == CTRL_F)
      {
        int ft = utils->CountFontSequence(buf, ii, len);

        if (ft >= 0)
        {
          ii += 2; // skip over 2-digit decimal font-size code

          if (ft == 0)
            ps->fontType = dts->cType;
          else
            ps->fontType = ft;

          // Get the font-string as ANSI for now
// !!!!! TODO - we actually store font-names in the table in DefaultStrings
// as UTF-8 (but there are no non-ansi chars in any font-names at this time)...
// The List-Box control's class is created with the Ansi call... but we do an
// OwnerDraw and use TextOutW to display at least some of the available unicode
// chars... but TCanvas is still ANSI - we really need Unicode versions of these
// Windows controls and call utils->GetLocalFontStringW... a sticky-wicket,
// as they say.
          String sTemp = utils->GetLocalFontStringA(ps->fontType);

          if (!sTemp.IsEmpty())
            p->Font->Name = sTemp;
        }
      }
      else if (c == CTRL_S)
      {
        int fs = utils->CountFontSequence(buf, ii, len);

        if (fs >= 0)
        {
          ii += 2; // skip over 2-digit decimal font-size code

          if (fs == 0)
            ps->fontSize = dts->cSize;
          else
            ps->fontSize = fs;

          p->Font->Size = ps->fontSize;
        }
      }
      else if (c == CTRL_K)
      {
        int fg = NO_COLOR;
        int bg = NO_COLOR;

        ii += utils->CountColorSequence(buf, ii, len, fg, bg);

        if (fg != NO_COLOR && fg != IRCNOCOLOR)
        {
          p->Font->Color = utils->YcToTColor(fg);
          ps->fg = fg;
        }

        if (bg != NO_COLOR && bg != IRCNOCOLOR)
        {
          p->Brush->Color = utils->YcToTColor(bg);
          ps->bg = bg;
        }
      }
    }

    // Write out remaining chars...
    outLen = wOut.Length();

    if (outLen > 0)
      // (NOTE: Check out ExtTextOutW)
      // NOTE: If the Window was created with RegisterClassA you won't
      // get all of the font-renderings...
      ::TextOutW(p->Handle, r.Left, r.Top, wOut.c_bstr(), outLen);

    bRet = true;
  }
  __finally
  {
    if (buf != NULL)
      delete [] buf;

    if (ps != NULL)
      delete ps;
  }

  return bRet;
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::LwlbMeasureItem(TWinControl *Control,
                                                    int Index, int &Height)
{
  // here we need the height of the tallest char
  Height = Lwlb->Canvas->TextHeight(Lwlb->Items->Strings[Index]);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::RwlbMeasureItem(TWinControl *Control, int Index,
                                                            int &Height)
{
  // here we need the height of the tallest char
  Height = Rwlb->Canvas->TextHeight(Rwlb->Items->Strings[Index]);
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::FormActivate(TObject *Sender)
{
  if (SFDlgForm)
    BringWindowToTop(SFDlgForm->Handle);
  if (OFSSDlgForm)
    BringWindowToTop(OFSSDlgForm->Handle);
//  CloseOpenSaveDialogs();
}
//---------------------------------------------------------------------------
void __fastcall TWingEditForm::CloseOpenSaveDialogs(void)
{
  if (bSaveFileDialogOpen)
  {
    if (SFDlgForm)
    {
      SendMessage(SFDlgForm->DlgHandle, WM_CLOSE, 0, 0);
      try { SFDlgForm->Release(); } catch(...) {};
      SFDlgForm = NULL;
    }

    bSaveFileDialogOpen = false;
  }

  if (bOpenFileDialogOpen)
  {
    if (OFSSDlgForm)
    {
      SendMessage(OFSSDlgForm->DlgHandle, WM_CLOSE, 0, 0);
      try { OFSSDlgForm->Release(); } catch(...) {};
      OFSSDlgForm = NULL;
    }

    bOpenFileDialogOpen = false;
  }
}
//---------------------------------------------------------------------------
//void __fastcall TWingEditForm::EditChange(TObject *Sender,
//      DWORD dwChangeType, void *pvCookieData, bool &AllowChange)
void __fastcall TWingEditForm::EditChange(TObject *Sender)
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

  // Disable changes (faster code)
//  TTaeRichEditChange SaveOnChange = re->OnChange;
  re->SaveOnChange = re->OnChange;
  re->OnChange = NULL;
  re->LockCounter++;

  // User added chars? (usually 1 or 2 for cr/lf)
  if (oc.selStart >= 0 && oc.deltaLength > 0)
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

          re->SelStart = oc.selStart-oc.deltaLength;

          if (Cnew != oc.c[0])
          {
            // char entered is a code? change to red and replace char
            re->SelLength = 1; // Just one char to replace!

            if (re->SelAttributes->Color != clRed)
              re->SelAttributes->Color = clRed;

            re->SelTextW = WideString(Cnew);
            re->SelLength = 0;
          }
          // Change range to black (could have inserted many chars after a red!)
          else if (re->SelAttributes->Color == clRed)
            re->SelAttributes->Color = dts->cColor;
        }
        catch(...)
        {
          ShowMessage("Exception: TWingEditForm::DoEditChange(TTaeRichEdit* re)");
        }

        re->SelStart = SaveSelStart;
      }

      delete [] buf;
    }
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
bool __fastcall TWingEditForm::ProcessOC(TTaeRichEdit* re, ONCHANGEW oc)
// Use this for diagnostics! (set DEBUG_ON "true" in Main.h)
//#if DEBUG_ON
//  dts->CWrite("\r\nhereA\r\n");
//#endif
// Note: oc.view below has our local re->View in it, not the
// main edit's View!
{
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
            EditString = utils->InsertW(EditString, WideString(CRLF), temp+1);
          else
            EditString = utils->InsertW(EditString, WideString(oc.c), temp+1);
        }
        catch(...)
        {
#if DEBUG_ON
          dts->CWrite("\r\nException 1: TWingEditForm::ProcessOC(ONCHANGE oc)\r\n");
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
          if (!utils->GetCodeIndices(EditString, iFirst, iLast, CI,
                                               oc.selStart-1, oc.deltaLength))
          {
            // Comes here when the edit control is empty...
            if (oc.c[0] == C_CR)
              EditString = CRLF; // added a CR/LF?
            else
              EditString = WideString(oc.c);
          }
          else
          {
            // Get state at insert-point...
            WideString LeadingState;

            if (oc.c[0] == C_CR) // added a CR/LF?
            {
              // Don't Copy Afg/Abg if NO_COLOR, don't skip spaces...(ok?)
              int RetIdx = utils->GetState(EditString, LeadingState,
                               oc.selStart-oc.line+1, true, false, false);

              if (RetIdx < 0)
                LeadingState = "";

              WideString S = LeadingState;

              if (iFirst <= EditString.Length())
              {
                S = utils->InsertW(S, CRLF, 1);
                EditString = utils->InsertW(EditString, S, iFirst+1);
              }
            }
            else
            {
              int iTemp;

              if (oc.selStart == 1)
                iTemp = iFirst+1;
              else
                iTemp = CI+1;

              if (iTemp <= EditString.Length()+1)
              {
                // It was hard to debug this and find the best solution.
                // Problems were: 1) End of document, first char of new
                // line needs color-codes written... 2) add a char to the end
                // of a pre-existing line needs to work too. So this is
                // a temporary hack.

                WideString S(oc.c[0]);

                // Previous char a line-break? Insert LeadingState!
                if (iTemp-2 > 0 && EditString[iTemp-CRLFLEN] == C_CR)
                {
                  // Copy Afg/Abg if NO_COLOR, don't skip spaces...(ok?)
                  int RetIdx = utils->GetState(EditString, LeadingState,
                                     oc.selStart-oc.line, true, true, false);

                  if (RetIdx < 0)
                    LeadingState = "";

                  S = utils->InsertW(S, LeadingState,1);
                }
                else if (iTemp-1 > 0 && EditString[iTemp-1] == C_CR)
                  iTemp--;

                EditString = utils->InsertW(EditString, S, iTemp);
              }
            }
          }
        }
        catch(...)
        {
#if DEBUG_ON
          dts->CWrite("\r\nException 2: TWingEditForm::ProcessOC(ONCHANGE oc)\r\n");
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
        EditString = "";
      else
      {
        try
        {
          // CR/LF deleted?
          if (oc.c[0] == C_CR)
          {
            if (oc.view == V_RTF)
            {
              int iTemp = utils->GetCodeIndex(EditString, oc.selStart);

              if (iTemp >= 0)
                EditString = utils->DeleteW(EditString, iTemp+1, CRLFLEN);
#if DEBUG_ON
              else
                dts->CWrite("\r\nError 9 TWingEditForm::ProcessOC()\r\n");
#endif
            }
            else
              EditString = utils->DeleteW(EditString,
                                            oc.selStart+oc.line+1, CRLFLEN);
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
              iTemp = utils->GetCodeIndex(EditString, oc.selStart);
            else // V_IRC, V_ORG or V_OFF
              iTemp = oc.selStart+oc.line;

            if (iTemp >= 0)
              EditString =
                    utils->DeleteW(EditString, iTemp+1, -oc.deltaLength);
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
          dts->CWrite("\r\nError 12 TWingEditForm::ProcessOC()\r\n");
#endif
          return false;
        }
      }
    }
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nError 13 TWingEditForm::ProcessOC()\r\n");
#endif
    return false;
  }

  return true;
}
//---------------------------------------------------------------------------

