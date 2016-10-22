//---------------------------------------------------------------------------
// YahCoLoRiZe - Edit, process and send colorized text into IRC chat-rooms
// via various chat-clients such as mIRC, IceCHat and LeafCHat
//
// Author: Scott Swift
//
// Released to GitHub under GPL v3 October, 2016
//
// I believe we can safely construe that this software may be released under
// the Free Software Foundation's GPL - I adopted TaeRichEdit for my project
// more than 15 years ago and have made significant changes in various
// places. Below is the original license. (Scott Swift 2015 dxzl@live.com)
//===========================================================================
// Copyright © 1999 Thin Air Enterprises and Robert Dunn.  All rights reserved.
// Free for non-commercial use.  Commercial use requires license agreement.
// See http://home.att.net/~robertdunn/Yacs.html for the most current version.
//===========================================================================
//---------------------------------------------------------------------------
// TaePreviewFrm.cpp - implementation file for TTaeRichEdit print preview.
//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

#include "TaePreviewFrm.h"
#include "TaeRichEditAdvPrint.h"
#include "TaeRichEditStrings.h"
#include <sysutils.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTaePreviewForm *TaePreviewForm = 0;
//---------------------------------------------------------------------------
AnsiString defFormTitle;
//---------------------------------------------------------------------------
int ScaleFactors[] = { 200, 150, 100, 95, 90, 85, 80, 75, 50, 25 };
int NumScaleFactors = sizeof(ScaleFactors) / sizeof(ScaleFactors[0]);
//---------------------------------------------------------------------------
class PACKAGE TTaePreviewWindow : public TPanel
{
protected:
  TTaeRichEdit* FRichEdit;
  int FPage;

  void __fastcall SetPage(int page)
  {
    FPage = page;
    Invalidate();
  };

  void __fastcall SetRichEdit(TTaeRichEdit* edit)
  {
    FRichEdit = edit;
    Invalidate();
  };

public:
  int FHdc;

  __fastcall virtual TTaePreviewWindow(TComponent* Owner) : TPanel(Owner), FPage(0), FHdc(0)
  {
    Color = clWhite;
  };

  __fastcall virtual void Paint(void)
  {
    TPanel::Paint();
    if (!FRichEdit) return;
    // set canvas brush (otherwise, DrawFocusRect() draws white on white)
    Canvas->Brush->Color = clWhite;
    // copy background into rendering rect
    FRichEdit->FRichEditPrint->RenderPage(Canvas->Handle, ClientWidth, ClientHeight, FPage + 1);
  };

  __property TTaeRichEdit* TaeRichEdit = { read = FRichEdit, write = SetRichEdit, nodefault };
  __property int Page = { read = FPage, write = SetPage, nodefault };
};
//---------------------------------------------------------------------------
__fastcall TTaePreviewForm::TTaePreviewForm(TComponent* Owner)
  : TForm(Owner), FRichEdit(0), PageCount(0), FirstPage(0),
  Mode(OneUp), Scale(100)
{
  // create three TTaePreviewWindows -- left and right pages (for
  // two-page display) and another for zooming
  Page1 = new TTaePreviewWindow(this);
  Page2 = new TTaePreviewWindow(this);
  Page3 = new TTaePreviewWindow(this);
  Page1->Parent = ClientPanel;
  Page2->Parent = ClientPanel;
  Page3->Parent =  ScrollBox;

  // set the window alignment to none (default?)
  Page1->Align = alNone;
  Page2->Align = alNone;
  Page3->Align = alNone;

  // set the margins for the zoom window to the width/height of a title bar
  // button (arbitrary -- change if you want)
  Page3->Left = GetSystemMetrics(SM_CXSIZE);
  Page3->Top = GetSystemMetrics(SM_CYSIZE);

  // get pixels/inch and save in member-vars
  HDC hdc = ::GetDC(Handle);
  xPerInch = GetDeviceCaps(hdc, LOGPIXELSX);
  yPerInch = GetDeviceCaps(hdc, LOGPIXELSY);
  ::ReleaseDC(Handle, hdc);

  // get a size for the border between the rendered area and the
  // displayed page (arbitrarily, the width/height of title bar buttons)
  cXsize = GetSystemMetrics(SM_CXSIZE);
  cYsize = GetSystemMetrics(SM_CYSIZE);

  // set the rendering HDC to something compatible with the screen
  hdc = ::GetWindowDC(GetDesktopWindow());
  Hdc = ::CreateCompatibleDC(hdc);
  ::ReleaseDC(GetDesktopWindow(), hdc);

  // fill scale factors combobox
  for (int i = 0; i < NumScaleFactors; i++)
    ScaleCB->Items->Add(AnsiString(ScaleFactors[i]) + "%");

  ScaleCB->ItemIndex = 2;
  Scale = ScaleFactors[ScaleCB->ItemIndex];

  // set the default caption for the window
  defFormTitle = Application->Title + " Print Preview - ";

  PrintSetupBtn->Hint = DS[142]; // Page Setup
  PrintSetupBtn->ShowHint = true;
  PrintBtn->Hint = DS[143]; // Print
  PrintBtn->ShowHint = true;

  UpDownScaleX->Hint = "Correction factor for the X-Axis";
  UpDownScaleX->ShowHint = true;
  UpDownScaleY->Hint = "Correction factor for the Y-Axis";
  UpDownScaleY->ShowHint = true;

  // Default scale (label)
  FCorrectionX = DEF_SCALE_X;
  LabelScaleX->Caption = String(FCorrectionX);
  FCorrectionY = DEF_SCALE_Y;
  LabelScaleY->Caption = String(FCorrectionY);
}
// Informative (below)
//---------------------------------------------------------------------------
__fastcall TTaePreviewForm::~TTaePreviewForm()
{
  // dispose of dc
  ::DeleteDC(Hdc);

  // make sure nobody is trying to use FRichEdit
  Page1->TaeRichEdit = FRichEdit;
  Page2->TaeRichEdit = FRichEdit;
  Page3->TaeRichEdit = FRichEdit;
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::SetCorrectionX(int Value)
{
  UpDownScaleX->Position = (short)Value;
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::SetCorrectionY(int Value)
{
  UpDownScaleY->Position = (short)Value;
}
//---------------------------------------------------------------------------
int __fastcall TTaePreviewForm::Execute(TTaeRichEdit* taeRichEdit)
{
  // set the FRichEdit variable for the form -- quick out if not valid
  FRichEdit = taeRichEdit;
  if (!FRichEdit || !FRichEdit->FRichEditPrint) return mrAbort;

  // Prior to calling Execute, set the properties CorrectionX and CorrectionY!

  // Initialize Correction factors
  UpDownScaleXClick(NULL, (TUDBtnType)0);
  UpDownScaleYClick(NULL, (TUDBtnType)0);

  return ShowModal();
}
//---------------------------------------------------------------------------
// FormResize() calculates the page window size(s) and positions the
// window(s).  up to three rectangles may require calculation:
// (1) the page rectangle scaled to the screen dimensions (from twips),
// (2) the bounding rectangle within the window, and (3) a rectangle
// within the bounding rectangle adjusted to proportions equal
// to the actual page proportions.  these are, respectively below,
// rX & rY, clX & clY, and adjX & adjY.  bX & bY are the width and
// height of an arbitrary minimum margin of clX/clY within rX/rY.
//
void __fastcall TTaePreviewForm::FormResize(TObject *Sender)
{
  // convert PageRect from twips to device units (pixels)
  int rX = ::MulDiv(FRichEdit->FRichEditPrint->TargetPageRect.Right -
    FRichEdit->FRichEditPrint->TargetPageRect.Left, xPerInch, 1440);
  int rY = ::MulDiv(FRichEdit->FRichEditPrint->TargetPageRect.Bottom -
    FRichEdit->FRichEditPrint->TargetPageRect.Top, yPerInch, 1440);

  // if zoomed, add border * 2 to display size
  if (Mode == Zoomed)
  {
    // multiply by zoom factor
    Page3->Width = ::MulDiv(rX, Scale, 100);
    Page3->Height = ::MulDiv(rY, Scale, 100);
    ScrollPanel->Width = Page3->Width + cXsize * 2;
    ScrollPanel->Height = Page3->Height + cYsize * 2;

    // invalidate first, then make sure showing
    Page3->Invalidate();
    ScrollPanel->Invalidate();
    ScrollBoxContainerPanel->Visible = true;
    ClientPanel->Visible = false;

    // and get out of here...
    return;
  }

  // otherwise fit page client less border
  // make client panel visible so it sets its size properly
  ClientPanel->Visible = true;

  // set initial size for client area
  int clX = ClientPanel->Width;
  int clY = ClientPanel->Height;
  int adjX, adjY;
  int top, left;

  // if two-up, divide width by 2
  if (Mode == TwoUp)
    clX = (clX / 2) + (cXsize / 2);

  // set initial adjusted client size
  clX -= cXsize * 2;
  clY -= cYsize * 2;

  // calculate an optimal rectangle for one page
  // first force width to fit
  adjY = ::MulDiv(rY, clX, rX);
  adjX = clX;

  // if height is too large, resize to fit height
  if (adjY > clY)
  {
    adjX = ::MulDiv(rX, clY, rY);
    adjY = clY;
  }

  // adjusted values must be less than or equal to original,
  // so set top and left to half the difference plus the border
  left = (clX - adjX) / 2 + cXsize;
  top = (clY - adjY) / 2 + cYsize;

  // now have size for a single page to display
  // set size of page display and center
  Page1->Width = adjX;
  Page1->Height = adjY;
  Page1->Left = left;
  Page1->Top = top;

  // if second page is showing
  if (Mode == TwoUp)
  {
    Page2->Width = Page1->Width;
    Page2->Height = Page1->Height;
    Page2->Top = Page1->Top;
    Page1->Left = (ClientPanel->Width - cXsize) / 2 - Page1->Width;
    Page2->Left = Page1->Left + Page1->Width + cXsize;
  }

  // make sure page display is showing
  Page1->Invalidate();
  Page2->Invalidate();
  Page2->Visible = Mode == TwoUp && Page2->Page < PageCount;
  Page1->Visible = true;
  ClientPanel->Invalidate();
  ClientPanel->Visible = true;
  ScrollBoxContainerPanel->Visible = false;
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::FormShow(TObject *Sender)
{
  // set form caption
  Caption = defFormTitle + FRichEdit->FileName;

  // arrange controls
  ClientPanel->Visible = false;
  ScrollBoxContainerPanel->Visible = false;
  Page1->Visible = true;
  Page2->Visible = Mode == TwoUp;
  Page3->Visible = true;

  // paginate to get page count
  Page1->TaeRichEdit = FRichEdit;
  Page2->TaeRichEdit = FRichEdit;
  Page3->TaeRichEdit = FRichEdit;
  FRichEdit->FRichEditPrint->RendDC = this->Hdc;

  // some buttons must(?) be set up here
  FRichEdit->FRichEditPrint->ShowMargins = MarginsBtn->Down;

  FRichEdit->FRichEditPrint->BeginRender(MAXINT);
  PageCount = FRichEdit->FRichEditPrint->PageCount;

  // set initial pages to show
  // later change the following to look up page of current cursor position
  GotoPage(0);

  // show appropriate panel
  if (Mode == Zoomed)
    ScrollBoxContainerPanel->Visible = true;
  else
    ClientPanel->Visible = true;

  // force a resize
  FormResize(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::OneUpBtnClick(TObject *Sender)
{
  // user clicked the one page button
  if (Mode == OneUp) return;
  Mode = OneUp;
  dynamic_cast<TSpeedButton*>(Sender)->Down = true;
  GotoPage(Page1->Page);
  ZoomInBtn->Enabled = false;
  ZoomOutBtn->Enabled = false;
  ScaleCB->Enabled = false;
  FormResize(this);
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::TwoUpBtnClick(TObject *Sender)
{
  // user clicked the two page button
  if (Mode == TwoUp) return;
  Mode = TwoUp;
  GotoPage(Page1->Page);
  dynamic_cast<TSpeedButton*>(Sender)->Down = true;
  ZoomInBtn->Enabled = false;
  ZoomOutBtn->Enabled = false;
  ScaleCB->Enabled = false;
  FormResize(this);
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::ZoomBtnClick(TObject *Sender)
{
  // user clicked the zoomed page button
  if (Mode == Zoomed) return;
  Mode = Zoomed;
  dynamic_cast<TSpeedButton*>(Sender)->Down = true;
  GotoPage(Page1->Page);
  ZoomInBtn->Enabled = true;
  ZoomOutBtn->Enabled = true;
  ScaleCB->Enabled = true;
  FormResize(this);
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::FirstPageBtnClick(TObject *Sender)
{
  // show the first page
  GotoPage(0);
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::PrevPageBtnClick(TObject *Sender)
{
  // show the previous page(s)
  GotoPage(Page1->Page - (Mode == TwoUp ? 2 : 1));
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::NextPageBtnClick(TObject *Sender)
{
  // show the next page(s)
  GotoPage(Page1->Page + (Mode == TwoUp ? 2 : 1));
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::LastPageBtnClick(TObject *Sender)
{
  // show the last page(s)
  GotoPage(PageCount - 1);
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::GotoPage(int page)
{
  // show a specific page -- sanity checks
  if (page < 0) page = 0;
  if (page >= PageCount) page = PageCount - 1;

  // if showing two pages, make it even
  if (Mode == TwoUp) page &= ~1;

  // set page values
  Page1->Page = page;
  Page3->Page = page;
  Page2->Page = page + 1;

  // if page2 out of range...
  Page2->Visible = (Mode == TwoUp && Page2->Page < PageCount);

  // enable/disable navigation buttons
  FirstPageBtn->Enabled = Page1->Page != 0;
  PrevPageBtn->Enabled = FirstPageBtn->Enabled;
  LastPageBtn->Enabled = (Mode == TwoUp && Page2->Page < PageCount ?
    Page2->Page : Page1->Page) != PageCount - 1;
  NextPageBtn->Enabled = LastPageBtn->Enabled;

  AnsiString msg(Page2->Visible ? "Pages " : "Page ");
  msg += AnsiString(Page1->Page + 1);
  
  if (Page2->Visible)
    msg += " and " + AnsiString(Page2->Page + 1);
  
  msg += " of " + AnsiString(PageCount);
  
  StatusBar->SimpleText = msg;

  // force repaint
  if (Mode == Zoomed)
    ScrollPanel->Invalidate();
  else
    ClientPanel->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::MarginsBtnClick(TObject *Sender)
{
  // user clicked show/hide margins button
  FRichEdit->FRichEditPrint->ShowMargins = MarginsBtn->Down;
  FormResize(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::ZoomInBtnClick(TObject *Sender)
{
  // user clicked the zoom in button
  if (ScaleCB->ItemIndex > 0)
  {
    ScaleCB->ItemIndex--;
    ScaleCBChange(Sender);
  }
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::ZoomOutBtnClick(TObject *Sender)
{
  // user clicked the zoom out button
  if (ScaleCB->ItemIndex < NumScaleFactors - 1)
  {
    ScaleCB->ItemIndex++;
    ScaleCBChange(Sender);
  }
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::ScaleCBChange(TObject *Sender)
{
  // user selected a different zoom factor
  Scale = ScaleFactors[ScaleCB->ItemIndex];
  
  if (ScaleCB->ItemIndex == 0)
  {
    ZoomInBtn->Enabled = false;
    ZoomOutBtn->Enabled = true;
  }
  else if (ScaleCB->ItemIndex >= NumScaleFactors - 1)
  {
    ZoomInBtn->Enabled = true;
    ZoomOutBtn->Enabled = false;
  }
  else
    ZoomInBtn->Enabled = true;
  
  ZoomOutBtn->Enabled = true;
  FormResize(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  // end rendering before closing
  FRichEdit->FRichEditPrint->EndRender();
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::PrintBtnClick(TObject *Sender)
{
  // user clicked the print button, so close and return a useful value
  ModalResult = mrAll + 1;  // note: this closes form
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::PrintSetupBtnClick(TObject *Sender)
{
  // user clicked the print setup button, so close and return a useful value
  ModalResult = mrAll + 2;  // note: this closes form
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::UpDownScaleXClick(TObject *Sender,
      TUDBtnType Button)
{
  if (FRichEdit && FRichEdit->FRichEditPrint)
  {
    FCorrectionX = UpDownScaleX->Position;
    LabelScaleX->Caption = String(FCorrectionX);
    LabelScaleX->Update();
    FRichEdit->FRichEditPrint->ScaleX = FCorrectionX;
    Page1->Invalidate();
    Page2->Invalidate();
    Page3->Invalidate();
  }
}
//---------------------------------------------------------------------------
void __fastcall TTaePreviewForm::UpDownScaleYClick(TObject *Sender,
      TUDBtnType Button)
{
  if (FRichEdit && FRichEdit->FRichEditPrint)
  {
    FCorrectionY = UpDownScaleY->Position;
    LabelScaleY->Caption = String(FCorrectionY);
    LabelScaleY->Update();
    FRichEdit->FRichEditPrint->ScaleY = FCorrectionY;
    Page1->Invalidate();
    Page2->Invalidate();
    Page3->Invalidate();
  }
}
//---------------------------------------------------------------------------

