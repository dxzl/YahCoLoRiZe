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
// TaePreviewFrm.h - header file for TTaeRichEdit print preview.
//---------------------------------------------------------------------------
#ifndef TaePreviewFrmH
#define TaePreviewFrmH
//---------------------------------------------------------------------------
#include <vcl\Classes.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\Buttons.hpp>

#include "TaeRichEdit.h"
#include <vcl\ComCtrls.hpp>
//---------------------------------------------------------------------------
enum TPreviewMode { OneUp, TwoUp, Zoomed };

class PACKAGE TTaePreviewForm : public TForm
{
__published:  // IDE-managed Components;
  TSpeedButton *NextPageBtn;
  TSpeedButton *LastPageBtn;
  TSpeedButton *FirstPageBtn;
  TSpeedButton *PrevPageBtn;
  TStatusBar *StatusBar;
  TComboBox *ScaleCB;
  TPanel *ToolBar;
  TPanel *ClientPanel;
  TSpeedButton *OneUpBtn;
  TSpeedButton *TwoUpBtn;
  TSpeedButton *ZoomBtn;
  TSpeedButton *MarginsBtn;
  TSpeedButton *ZoomInBtn;
  TSpeedButton *ZoomOutBtn;
  TSpeedButton *PrintBtn;
  TSpeedButton *PrintSetupBtn;
  TPanel *ScrollBoxContainerPanel;
  TScrollBox *ScrollBox;
  TPanel *ScrollPanel;
  TUpDown *UpDownScaleY;
  TLabel *LabelScaleY;
  TLabel *LabelScaleX;
  TUpDown *UpDownScaleX;
  void __fastcall FormResize(TObject *Sender);

  void __fastcall FormShow(TObject *Sender);

  void __fastcall OneUpBtnClick(TObject *Sender);
  void __fastcall TwoUpBtnClick(TObject *Sender);
  void __fastcall ZoomBtnClick(TObject *Sender);

  void __fastcall FirstPageBtnClick(TObject *Sender);
  void __fastcall PrevPageBtnClick(TObject *Sender);
  void __fastcall NextPageBtnClick(TObject *Sender);
  void __fastcall LastPageBtnClick(TObject *Sender);
  void __fastcall MarginsBtnClick(TObject *Sender);
  void __fastcall ZoomInBtnClick(TObject *Sender);
  void __fastcall ZoomOutBtnClick(TObject *Sender);
  void __fastcall ScaleCBChange(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall PrintBtnClick(TObject *Sender);
  void __fastcall PrintSetupBtnClick(TObject *Sender);
  void __fastcall UpDownScaleYClick(TObject *Sender, TUDBtnType Button);
  void __fastcall UpDownScaleXClick(TObject *Sender, TUDBtnType Button);

private:  // User declarations
  int xPerInch, yPerInch;
  int cXsize, cYsize;
  
protected:
  TTaeRichEdit* FRichEdit;
  int PageCount;
  int FirstPage;
  TPreviewMode Mode;
  int Scale, FCorrectionX, FCorrectionY;
  HDC Hdc;
  TTaePreviewWindow* Page1;
  TTaePreviewWindow* Page2;
  TTaePreviewWindow* Page3;

  void __fastcall SetCorrectionY(int Value);
  void __fastcall SetCorrectionX(int Value);

public:    // User declarations
  __fastcall TTaePreviewForm(TComponent* Owner);
  __fastcall ~TTaePreviewForm();
  int __fastcall Execute(TTaeRichEdit* taeRichEdit); // use this instead of Show() or ShowModal()
  void __fastcall GotoPage(int page);

  __property int CorrectionX = { read = FCorrectionX, write = SetCorrectionX };
  __property int CorrectionY = { read = FCorrectionY, write = SetCorrectionY };
  __property HDC RendDC = { read = Hdc, write = Hdc, nodefault };
  __property TTaeRichEdit* TaeRichEdit = { read = FRichEdit,
                        write = FRichEdit, default = 0, stored = false };
};
//---------------------------------------------------------------------------
extern PACKAGE TTaePreviewForm *TaePreviewForm;
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
