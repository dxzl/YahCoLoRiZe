//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

#include "PrintPreview.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
TPrintPreview* PrintPreview;
//---------------------------------------------------------------------------
// PrintPreview Functions
//---------------------------------------------------------------------------
__fastcall TPrintPreview::TPrintPreview(TComponent* Owner)
// constructor
{
  this->dts = static_cast<TDTSColor*>(Owner);
}
//---------------------------------------------------------------------------
__fastcall TPrintPreview::~TPrintPreview(void)
// destructor
{
}
//---------------------------------------------------------------------------
bool __fastcall TPrintPreview::Preview(TTaeRichEdit* re)
{
// hwnd is the HWND of the rich edit control.
// hdc is the HDC of the printer. This value can be obtained for the
// default printer as follows:
//
    PRINTDLG pd = { sizeof(pd) };
    pd.Flags = PD_RETURNDC | PD_RETURNDEFAULT;

//    Printer()->BeginDoc(); // BeginDoc() called before to ensure handle valid
    HDC hdc = Printer()->Handle;

//    if (PrintDlg(&pd))
//      hdc = pd.hDC;

//    if (!hdc)
//      return false;

    HWND hwnd = re->Handle;

    DOCINFO di = { sizeof(di) };

    if (!StartDoc(hdc, &di))
      return FALSE;

    int cxPhysOffset = GetDeviceCaps(hdc, PHYSICALOFFSETX);
    int cyPhysOffset = GetDeviceCaps(hdc, PHYSICALOFFSETY);

    int cxPhys = GetDeviceCaps(hdc, PHYSICALWIDTH);
    int cyPhys = GetDeviceCaps(hdc, PHYSICALHEIGHT);

    // Create "print preview".
    // If lParam is zero, no line breaks are created!
    SendMessage(hwnd, EM_SETTARGETDEVICE, (WPARAM)hdc, cxPhys-cxPhysOffset-1); // cxPhys/2

    FORMATRANGE fr;

    fr.hdc       = hdc;
    fr.hdcTarget = hdc;

    // Set page rect to physical page size in twips.
    fr.rcPage.top    = 0;
    fr.rcPage.left   = 0;
    fr.rcPage.right  = MulDiv(cxPhys, 1440, GetDeviceCaps(hdc, LOGPIXELSX));
    fr.rcPage.bottom = MulDiv(cyPhys, 1440, GetDeviceCaps(hdc, LOGPIXELSY));

    // Set the rendering rectangle to the printable area of the page.
    fr.rc.left   = cxPhysOffset;
    fr.rc.right  = cxPhysOffset + cxPhys;
    fr.rc.top    = cyPhysOffset;
    fr.rc.bottom = cyPhysOffset + cyPhys;

    // Get the selection range
    SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&fr.chrg);

    if (fr.chrg.cpMax == 0)
    {
      GETTEXTLENGTHEX gtlx = { GTL_PRECISE, CP_ACP };
      fr.chrg.cpMax = ::SendMessage(hwnd, EM_GETTEXTLENGTHEX,
                                                          (WPARAM) &gtlx, 0);
// note: this did not work before but then I discovered that TextLength includes
// a count of 2 for each line-terminator - so - just adding that... below
// but have not tried it yet :-)
      //fr.chrg.cpMax = tae->TextLength - tae->LineCount + 1;
    }

    BOOL fSuccess = TRUE;
/*
  PrintDialog1->Options.Clear();
  PrintDialog1->Options << poPageNums << poSelection;
  PrintDialog1->FromPage = 1;
  PrintDialog1->MinPage = 1;
  PrintDialog1->ToPage = PageControl1->PageCount;
  PrintDialog1->MaxPage = Pagecontrol1->PageCount;
  if (PrintDialog1->Execute())
  {
    int Start, Stop;
    // determine the range the user wants to print
    switch (PrintDialog1->PrintRange)
    {
      case prSelection:

        Start = PageControl1->ActivePage->PageIndex;
        Stop = Start;
        break;
      case prPageNums:
        Start = PrintDialog1->FromPage - 1;
        Stop =  PrintDialog1->ToPage - 1;
        break;
      default:  // prAllPages
        Start = PrintDialog1->MinPage - 1;
        Stop = PrintDialog1->MaxPage - 1;
        break;
    }
    // now, print the pages
    Printer()->BeginDoc();
    for (int i = Start; i <= Stop; i++)

    {
      PageControl1->Pages[i]->PaintTo(Printer()->Handle, 10, 10);
      if (i != Stop)
        Printer()->NewPage();
    }
    Printer()->EndDoc();
*/
    // SS
    // set printer spool title
    Printer()->Title = "Printer Test";
//    ::SendMessage(hwnd, EM_FORMATRANGE, TRUE, (LPARAM) 0);
//    ::SetMapMode(fr.hdc, MM_TEXT);


    // Use GDI to print successive pages.
    while (fr.chrg.cpMin < fr.chrg.cpMax && fSuccess)
    {
        fSuccess = StartPage(hdc) > 0;

        if (!fSuccess) break;

        int cpMin = SendMessage(hwnd, EM_FORMATRANGE, TRUE, (LPARAM)&fr);

        if (cpMin <= fr.chrg.cpMin)
        {
            fSuccess = FALSE;
            break;
        }

        fr.chrg.cpMin = cpMin;
        fSuccess = EndPage(hdc) > 0;
    }

    SendMessage(hwnd, EM_FORMATRANGE, FALSE, 0);

    if (fSuccess)
    {
      Printer()->EndDoc();
ShowMessage("Starting print job");
    }
    else
      Printer()->Abort();

    return fSuccess;
}
//---------------------------------------------------------------------------

