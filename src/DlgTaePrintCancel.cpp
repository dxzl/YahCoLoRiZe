//---------------------------------------------------------------------------
// YahCoLoRiZe - Edit, process and send colorized text into IRC chat-rooms
// via various chat-clients such as mIRC, IceCHat and LeafCHat
//
// Author: Scott Swift
//
// Released to GitHub under GPL v3 October, 2016
//
//===========================================================================
// Copyright © 1999 Thin Air Enterprises and Robert Dunn.  All rights reserved.
// Free for non-commercial use.  Commercial use requires license agreement.
// See http://home.att.net/~robertdunn/Yacs.html for the most current version.
//===========================================================================
//---------------------------------------------------------------------------
// TaePrintCancelDlg.cpp - implementation file for TaePrintCancelDlg.cpp
// (sample print cancel dialog).  note that this dialog (form) is not yet
// complete -- there are elements that are essentially placeholders for
// future revisions.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "DlgTaePrintCancel.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTaePrintCancelDialog* TaePrintCancelDialog;
//---------------------------------------------------------------------------
__fastcall TTaePrintCancelDialog::TTaePrintCancelDialog(TComponent* Owner)
  : TForm(Owner)
{
  FOnAbortPrint = 0;
}
//---------------------------------------------------------------------------
void __fastcall TTaePrintCancelDialog::CancelClick(TObject *Sender)
{
  if (OnAbortPrint)(OnAbortPrint)(this);
}
//---------------------------------------------------------------------------
void __fastcall TTaePrintCancelDialog::FormShow(TObject *Sender)
{
  DocNameLbl->Caption = TargetPrinter->Title;
  PrinterNameLbl->Caption = Printer()->Printers->Strings[Printer()->PrinterIndex];
  OrientationLbl->Caption = Printer()->Orientation == poPortrait ?
    "Portrait" : "Landscape";
  CopiesLbl->Caption = AnsiString(Printer()->Copies);
  PagesPrinted = 0; // use property to update progress bar...
}
//---------------------------------------------------------------------------
// end TaePrintCancelDlg.cpp
//---------------------------------------------------------------------------
//===========================================================================
// Copyright © 1999 Thin Air Enterprises and Robert Dunn.  All rights reserved.
// Free for non-commercial use.  Commercial use requires license agreement.
// See http://home.att.net/~robertdunn/Yacs.html for the most current version.
//===========================================================================

