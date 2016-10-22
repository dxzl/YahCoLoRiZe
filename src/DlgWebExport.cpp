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

#include "DlgWebExport.h"

// ComboBoxW.h is customized (by me) for Wide (UTF-16) calls (Scott S.)
#include "ComboBoxW.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TWebExportForm *WebExportForm;
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::FormCreate(TObject *Sender)
{
  HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);

  int iStyle = WS_TABSTOP|WS_OVERLAPPED|WS_CHILD|ES_LEFT|
                                          ES_AUTOHSCROLL|WS_VISIBLE;

  // May want to add hints to the custom edit-controls...
  // About Tooltip Controls
  // https://msdn.microsoft.com/en-us/library/bb760250%28VS.85%29.aspx?f=255&MSPPError=-2147217396

  m_hImageUrlEdit = CreateWindowExW(WS_EX_CLIENTEDGE,
      L"EDIT", L"",
      iStyle,
      WD_LEFT_IMAGEURL, // x
      WD_TOP_IMAGEURL, // y
      WD_WIDTH, // width
      WD_HEIGHT, // height
      this->Handle, // parent
      (HMENU)ID_EditImageUrl,
      hInst,
      NULL);

  m_hTitleEdit = CreateWindowExW(WS_EX_CLIENTEDGE,
      L"EDIT", L"",
      iStyle,
      WD_LEFT_TITLE, // x
      WD_TOP_TITLE, // y
      WD_WIDTH, // width
      WD_HEIGHT, // height
      this->Handle, // parent
      (HMENU)ID_EditTitle,
      hInst,
      NULL);

  m_hAuthorEdit = CreateWindowExW(WS_EX_CLIENTEDGE,
      L"EDIT", L"",
      iStyle,
      WD_LEFT_AUTHOR, // x
      WD_TOP_AUTHOR, // y
      WD_WIDTH, // width
      WD_HEIGHT, // height
      this->Handle, // parent
      (HMENU)ID_EditAuthor,
      hInst,
      NULL);

  m_hHomeLinkEdit = CreateWindowExW(WS_EX_CLIENTEDGE,
      L"EDIT", L"",
      iStyle,
      WD_LEFT_HOMELINK, // x
      WD_TOP_HOMELINK, // y
      WD_WIDTH, // width
      WD_HEIGHT, // height
      this->Handle, // parent
      (HMENU)ID_EditHomeLink,
      hInst,
      NULL);

  if (!m_hImageUrlEdit || !m_hTitleEdit || !m_hAuthorEdit || !m_hHomeLinkEdit)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nm_hEdit control is NULL in DlgWebExport()!\r\n");
#endif
    return;
  }

  m_hFont = GetStockFont(DEFAULT_GUI_FONT);

  if (!m_hFont)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nm_hFont is NULL in DlgWebExport()!\r\n");
#endif
    return;
  }

  // Set the default GUI font (default font is NOT pretty!)
  SetWindowFont(m_hImageUrlEdit, m_hFont, FALSE);
  SetWindowFont(m_hTitleEdit, m_hFont, FALSE);
  SetWindowFont(m_hAuthorEdit, m_hFont, FALSE);
  SetWindowFont(m_hHomeLinkEdit, m_hFont, FALSE);

  TopMarginLabel->Caption = WEBEXPORTMSG[1];
  LeftMarginLabel->Caption = WEBEXPORTMSG[2];
  ColorPanelLabel->Caption = WEBEXPORTMSG[3];
  ColorPanel->Hint = WEBEXPORTMSG[4];
  ClipCheckBox->Hint = WEBEXPORTMSG[5];
  BgImageCheckBox->Hint = WEBEXPORTMSG[6];
  BgFixedCheckBox->Hint = WEBEXPORTMSG[7];
  AuthorCheckBox->Hint = WEBEXPORTMSG[8];
  TitleCheckBox->Hint = WEBEXPORTMSG[9];
  HomeCheckBox->Hint = WEBEXPORTMSG[10];
  BgImageRadioGroup->Hint = WEBEXPORTMSG[11];
  NBSPCheckBox->Hint = WEBEXPORTMSG[12];

  TitleCheckBox->Caption = WEBEXPORTMSG[17];
  AuthorCheckBox->Caption = WEBEXPORTMSG[18];
  HomeCheckBox->Caption = WEBEXPORTMSG[19];

  NBSPCheckBox->Caption = WEBEXPORTMSG[20];
  WhiteSpaceRadioGroup->Caption = WEBEXPORTMSG[21];

  LineHeight = DTSColor->WebPageLineHeight; // invokes SetLineHeight()
  BgColor = DTSColor->WebPageBgColor; // invokes SetBgColor()
  LeftMargin = DTSColor->WebPageLeftMargin;
  TopMargin = DTSColor->WebPageTopMargin;
  WhiteSpaceStyle = DTSColor->WebPageWhiteSpaceStyle; // invokes SetWhitespaceStyle()
  BgImageUrl = DTSColor->WebPageBgImageUrl;
  HomeButtonUrl = DTSColor->WebPageHomeButtonUrl;
  WebPageTitle = DTSColor->WebPageTitle;
  WebPageAuthor = DTSColor->WebPageAuthor;

  BNonBreakingSpaces = DTSColor->WebPage_bNonBreakingSpaces;
  BBgImage = DTSColor->WebPage_bBgImage;
  BBgFixed = DTSColor->WebPage_bBgFixed;
  BAuthor = DTSColor->WebPage_bAuthor;
  BTitle = DTSColor->WebPage_bTitle;
  BHome = DTSColor->WebPage_bHome;

  BClip = WEBEXPORT_COPY_TO_CLIPBOARD;
  SetControls(BClip);
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::FormDestroy(TObject *Sender)
{
  if (m_hImageUrlEdit)
    DestroyWindow(m_hImageUrlEdit);
  if (m_hTitleEdit)
    DestroyWindow(m_hTitleEdit);
  if (m_hAuthorEdit)
    DestroyWindow(m_hAuthorEdit);
  if (m_hHomeLinkEdit)
    DestroyWindow(m_hHomeLinkEdit);
  if (m_hFont)
    DeleteFont(m_hFont);
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::FormShow(TObject *Sender)
{
  TopMarginSlider->Position = topMargin;
  LeftMarginSlider->Position = leftMargin;

  TopLabel->Caption = "*" + String(TopMarginSlider->Position) + "*";
  LeftLabel->Caption = "*" + String(LeftMarginSlider->Position) + "*";

  LineHeightEdit->Hint = "0=\"normal\", -10=\"inherit\", use 120\r\n"
                     "to paste into some E-Mail clients...";

  LineHeightUpDown->Position = (unsigned short)lineHeight;
  ComputeLineHeightLabel(LineHeightUpDown->Position);

  ColorPanel->Color = utils->YcToTColor(bgColor);

  EditW_SetText(m_hImageUrlEdit, utils->Utf8ToWide(bgImageUrl).c_bstr());
  EditW_SetText(m_hTitleEdit, utils->Utf8ToWide(webPageTitle).c_bstr());
  EditW_SetText(m_hAuthorEdit, utils->Utf8ToWide(webPageAuthor).c_bstr());
  EditW_SetText(m_hHomeLinkEdit, utils->Utf8ToWide(homeButtonUrl).c_bstr());

  NBSPCheckBox->Checked = BNonBreakingSpaces;
  BgImageCheckBox->Checked = BBgImage;
  BgFixedCheckBox->Checked = BBgFixed;
  AuthorCheckBox->Checked = BAuthor;
  TitleCheckBox->Checked = BTitle;
  HomeCheckBox->Checked = BHome;

  ClipCheckBox->Checked = BClip;
  SetControls(BClip);

  // Set onclick handler to set other boxes when checked/unchecked
  ClipCheckBox->OnClick = ClipCheckBoxClick;
  // Set edit-OnChange
  LineHeightEdit->OnChange = LineHeightEditChange;
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
  // Copy properties from controls...
  leftMargin = LeftMarginSlider->Position;
  topMargin = TopMarginSlider->Position;
  whiteSpaceStyle = WhiteSpaceRadioGroup->ItemIndex;;
  bgImageStyle = BgImageRadioGroup->ItemIndex;
  bgColor = utils->TColorToYc(ColorPanel->Color);

  bgImageUrl = utils->WideToUtf8(EditBoxW_GetString(m_hImageUrlEdit));
  webPageTitle = utils->WideToUtf8(EditBoxW_GetString(m_hTitleEdit));
  webPageAuthor = utils->WideToUtf8(EditBoxW_GetString(m_hAuthorEdit));
  homeButtonUrl = utils->WideToUtf8(EditBoxW_GetString(m_hHomeLinkEdit));

  BNonBreakingSpaces = NBSPCheckBox->Checked;
  BBgImage = BgImageCheckBox->Checked;
  BBgFixed = BgFixedCheckBox->Checked;
  BAuthor = AuthorCheckBox->Checked;
  BTitle = TitleCheckBox->Checked;
  BHome = HomeCheckBox->Checked;
  BClip = ClipCheckBox->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::TopMarginSliderChange(TObject *Sender)
{
  TopLabel->Caption = "*" + String(TopMarginSlider->Position) + "*";
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::LeftMarginSliderChange(TObject *Sender)
{
  LeftLabel->Caption = "*" + String(LeftMarginSlider->Position) + "*";
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::BgImageCheckBoxClick(TObject *Sender)
{
  // To come here, ClipCheckBox->Checked must be false, by definition
  if (!BgImageCheckBox->Checked)
  {
    EditW_Enable(m_hImageUrlEdit, false);
    BgFixedCheckBox->Enabled = false;
    BgImageRadioGroup->Enabled = false;
  }
  else
  {
    EditW_Enable(m_hImageUrlEdit, true);
    BgFixedCheckBox->Enabled = true;
    BgImageRadioGroup->Enabled = true;
  }
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::AuthorCheckBoxClick(TObject *Sender)
{
  if (!AuthorCheckBox->Checked)
    EditW_Enable(m_hAuthorEdit, false);
  else
    EditW_Enable(m_hAuthorEdit, true);
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::TitleCheckBoxClick(TObject *Sender)
{
  if (!TitleCheckBox->Checked)
    EditW_Enable(m_hTitleEdit, false);
  else
    EditW_Enable(m_hTitleEdit, true);
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::HomeCheckBoxClick(TObject *Sender)
{
  if (!HomeCheckBox->Checked)
    EditW_Enable(m_hHomeLinkEdit, false);
  else
    EditW_Enable(m_hHomeLinkEdit, true);
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::SetControls(bool bSetSaveToClipboardMode)
{
  ClipCheckBox->Enabled = true;
  WhiteSpaceRadioGroup->Enabled = true;
  NBSPCheckBox->Enabled = true;
  LineHeightEdit->Enabled = true;

  if (bSetSaveToClipboardMode)
  {
    EditW_Enable(m_hHomeLinkEdit, false);
    EditW_Enable(m_hImageUrlEdit, false);
    EditW_Enable(m_hAuthorEdit, false);
    EditW_Enable(m_hTitleEdit, false);

    HomeCheckBox->Enabled = false;
    BgImageCheckBox->Enabled = false;
    BgFixedCheckBox->Enabled = false;
    AuthorCheckBox->Enabled = false;
    TitleCheckBox->Enabled = false;

    TopMarginSlider->Enabled = false;
    LeftMarginSlider->Enabled = false;

    BgImageRadioGroup->Enabled = false;
    ColorPanel->Enabled = false;

//    LineHeightUpDown->Position = 100; // 1em
  }
  else
  {
    HomeCheckBox->Enabled = true;
    BgImageCheckBox->Enabled = true;
    AuthorCheckBox->Enabled = true;
    TitleCheckBox->Enabled = true;
    TopMarginSlider->Enabled = true;
    LeftMarginSlider->Enabled = true;

    if (HomeCheckBox->Checked)
      EditW_Enable(m_hHomeLinkEdit, true);
    else
      EditW_Enable(m_hHomeLinkEdit, false);

    if (BgImageCheckBox->Checked)
    {
      EditW_Enable(m_hImageUrlEdit, true);
      BgFixedCheckBox->Enabled = true;
      BgImageRadioGroup->Enabled = true;
    }
    else
    {
      EditW_Enable(m_hImageUrlEdit, false);
      BgFixedCheckBox->Enabled = false;
      BgImageRadioGroup->Enabled = false;
    }

    if (AuthorCheckBox->Checked)
      EditW_Enable(m_hAuthorEdit, true);
    else
      EditW_Enable(m_hAuthorEdit, false);

    if (TitleCheckBox->Checked)
      EditW_Enable(m_hTitleEdit, true);
    else
      EditW_Enable(m_hTitleEdit, false);

    ColorPanel->Enabled = true;
//    LineHeightUpDown->Position = 0; // Normal
  }
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::ClipCheckBoxClick(TObject *Sender)
{
  if (ClipCheckBox->Checked)
    SetControls(true);
  else
    SetControls(false);
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if (Key == VK_ESCAPE )
    ModalResult = mrCancel;
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::ComputeLineHeightLabel(int Val)
{
  String SHeight;

  // works on Chrome, IE8, MS Word - but not Windows Live Mail
  if (Val == 0 ) SHeight = "normal";
  else if (Val < 0 )
    SHeight = "inherit";
  else
    SHeight = Format("%-.2f", ARRAYOFCONST(((float)Val/100.0))) + "em";

  LineHeightLabel->Caption = WEBEXPORTMSG[26] + SHeight;
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::ColorPanelMouseDown(TObject *Sender,
                      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  // Right button invokes a popup menu: DTS-Color->ColorCopyPasteMenu
  if (Button == mbLeft)
  {
    int C = utils->PanelColorDialog(ColorPanel, COLORDIALOGMSG[11],
                                                    clAqua, COLOR_FORM_HTML);

    if (C != IRCCANCEL)
      this->bgColor = C;
  }
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::SetBgColor(int val)
// property setter
{
  if (utils->SetPanelColorAndCaption(ColorPanel, val))
    this->bgColor = val;
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::LineHeightEditChange(TObject *Sender)
{
  // invoke property-setter SetLineHeight
  this->LineHeight = LineHeightEdit->Text.ToIntDef(0);
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::SetLineHeight(int val)
// property setter
{
  this->lineHeight = val;
  ComputeLineHeightLabel(this->lineHeight);
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::SetWhiteSpaceStyle(int val)
// property setter
{
  if (val < 0 || val >= WhiteSpaceRadioGroup->Items->Count)
    val = WEBEXPORT_WHITESPACE_STYLE;

  WhiteSpaceRadioGroup->ItemIndex = val;
  this->whiteSpaceStyle = val;
}
//---------------------------------------------------------------------------
void __fastcall TWebExportForm::SetBgImageStyle(int val)
// property setter
{
  if (val < 0 || val >= BgImageRadioGroup->Items->Count)
    val = WEBEXPORT_BG_IMAGE_STYLE;

  WhiteSpaceRadioGroup->ItemIndex = val;
  this->whiteSpaceStyle = val;
}
//---------------------------------------------------------------------------
WideString __fastcall TWebExportForm::EditBoxW_GetString(HWND hwndCtl)
{
  WideString wText;

  if (hwndCtl)
  {
    int len = EditW_GetTextLength(hwndCtl);
    wchar_t* buf = new wchar_t[len+1];
    EditW_GetText(hwndCtl, buf, len+1);
    wText = WideString(buf, len);
    delete [] buf;
  }

  return wText;
}
//---------------------------------------------------------------------------

