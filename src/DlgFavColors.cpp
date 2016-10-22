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

#include "DlgFavColors.h"
#include "DlgFgBgColors.h"
#include "FormSFDlg.h"
#include "FormOFSSDlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
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
TFavColorsForm *FavColorsForm;
//---------------------------------------------------------------------------
__fastcall TFavColorsForm::TFavColorsForm(TComponent* Owner)
  : TForm(Owner)
{
  this->dts = DTSColor;
  focusIndex = -1;
  fgColor = NO_COLOR;
  bgColor = NO_COLOR;
  bChanged = false;
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::FormCreate(TObject *Sender)
{
  pSCF = new TFgBgColorsForm(this);
  ListBox->Font->Name = dts->cFont;
  ListBox->Font->Size = dts->cSize;
  this->Caption = utils->Utf8ToAnsi(FAVCOLORSDLG[0]);
  Label1->Caption = utils->Utf8ToAnsi(FAVCOLORSDLG[1]);
  ButtonAddCurrent->Caption = utils->Utf8ToAnsi(FAVCOLORSDLG[3]);
  ButtonAddNew->Caption = utils->Utf8ToAnsi(FAVCOLORSDLG[4]);
  ButtonDel->Caption = utils->Utf8ToAnsi(FAVCOLORSDLG[5]);
  ButtonOk->Caption = utils->Utf8ToAnsi(FAVCOLORSDLG[6]);
  ButtonCancel->Caption = utils->Utf8ToAnsi(FAVCOLORSDLG[7]);

  try
  {
    utils->LoadMenu(MainMenu, (char**)FAVCOLORSMAINMENU);
  }
  catch(...)
  {
    ShowMessage("DlgFavColors: Unable to load Menu!");
  }
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::FormDestroy(TObject *Sender)
{
  TPoint* p;

  int count = ListBox->Items->Count;

  // Delete TPoint objects that contain a fg/bg color for every list item
  for (int ii = 0; ii < count; ii++)
    if ((p = (TPoint*)this->ListBox->Items->Objects[ii]) != NULL)
      delete p;

  if (pSCF)
    pSCF->Release();
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if (Key == VK_ESCAPE)
    ModalResult = mrCancel;
  else if (Key == VK_DELETE)
    ButtonDelClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::Import1Click(TObject *Sender)
{
  Import();
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::Export1Click(TObject *Sender)
{
  Export();
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::ListBoxDrawItem(TWinControl *Control,
      int Index, TRect &Rect, TOwnerDrawState State)
{
  int count = ListBox->Items->Count;

  if (!count || Index < 0 || Index >= count)
    return;

  TPoint* p = (TPoint*)ListBox->Items->Objects[Index];

  if (!p)
    return;

  if (!ListBox->Canvas->TryLock())
    return;

  int fg = -p->x;
  int bg = -p->y;

  ListBox->Canvas->Font->Color = utils->YcToTColor(fg);
  ListBox->Canvas->Brush->Color = utils->YcToTColor(bg);
  ListBox->Canvas->FillRect(Rect);

  WideString wOut;

  String sIn = ListBox->Items->Strings[Index]; // this is in utf-8!

  if (!sIn.IsEmpty())
    wOut = utils->Utf8ToWide(sIn);

  // Write out remaining chars...
  int outLen = wOut.Length();

  if (outLen > 0)
    // (NOTE: Check out ExtTextOutW)
    // NOTE: If the Window was created with RegisterClassA you won't
    // get all of the font-renderings...
    ::TextOutW(ListBox->Canvas->Handle, Rect.Left, Rect.Top,
                                                wOut.c_bstr(), outLen);

  // erase artifact around the item odSelected odGrayed odChecked odDisabled
  if (State.Contains(odFocused))
  {
    if (focusIndex < 0 && Index >= 0)
    {
      ListBox->Canvas->DrawFocusRect(ListBox->ItemRect(Index));
      focusIndex = Index; // turn on
    }
  }
  else if (focusIndex >= 0 && Index == focusIndex)
  {
    ListBox->Canvas->DrawFocusRect(ListBox->ItemRect(Index));
    focusIndex = -1; // turn off
  }

  ListBox->Canvas->Unlock();
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::ListBoxClick(TObject *Sender)
{
  TRect r = ListBox->ItemRect(ListBox->ItemIndex);
  ListBox->Canvas->DrawFocusRect(r); // draw rect around the item
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::FormShow(TObject *Sender)
{
  focusIndex = -1;
  fgColor = NO_COLOR;
  bgColor = NO_COLOR;
  bChanged = false;

  ReadIniFile();
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
  if (this->ModalResult == mrOk)
  {
    int idx = ListBox->ItemIndex;
    int count = ListBox->Items->Count;

    if (idx < 0 && count)
      idx = 0;

    if (count && idx >= 0 && idx < count)
    {
      TPoint* p = (TPoint*)ListBox->Items->Objects[idx];

      if (p)
      {
        // Set properties
        fgColor = -p->x;
        bgColor = -p->y;
      }
    }
    else
      this->ModalResult = mrCancel; // force cancel

    if (bChanged)
      WriteColorsToIni();
  }
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::ButtonAddCurrentClick(TObject *Sender)
{
  int fg = -utils->YcToRgb(dts->Afg);
  int bg = -utils->YcToRgb(dts->Abg);

  ListBox->Items->AddObject(FAVCOLORSDLG[2], (TObject*)new TPoint(fg, bg));
  ListBox->ItemIndex = ListBox->Items->Count-1;
  bChanged = true;
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::ButtonAddNewClick(TObject *Sender)
{
  pSCF->ShowModal();

  if (pSCF->ModalResult == mrCancel)
    return;

  int fg = -utils->YcToRgb(pSCF->FgColor);
  int bg = -utils->YcToRgb(pSCF->BgColor);

  ListBox->Items->AddObject(FAVCOLORSDLG[2], (TObject*)new TPoint(fg, bg));
  ListBox->ItemIndex = ListBox->Items->Count-1;
  bChanged = true;
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::ButtonDelClick(TObject *Sender)
{
  int count = ListBox->Items->Count;
  int index = ListBox->ItemIndex;

  if (index >= 0 && index < count)
  {
    TPoint* p;

    // Delete TPoint objects that contain a fg/bg color for every list item
    if ((p = (TPoint*)this->ListBox->Items->Objects[index]) != NULL)
      delete p;
      
    ListBox->Items->Delete(index);
  }
}
//---------------------------------------------------------------------------
bool __fastcall TFavColorsForm::ReadIniFile(void)
// Reads the colorize.ini file
{
  bool bRet;

  TIniFile *pIni = NULL;

  try
  {
    try
    {

//      pIni = utils->OpenIniFile(L"C:\\Users\\Scott\\Documents\\Borland\\Projects\\YahCoLoRiZe\\Colorize.ini");
      pIni = utils->OpenIniFile();

      if (pIni == NULL)
        return false;

      ReadColorsFromIni(pIni);

//      int count = ReadColorsFromIni(pIni);

//      if (count)
//      {
//        TPoint* p;
//        for (int ii = 0; ii < count; ii++)
//        {
//          p = (TPoint*)ListBox->Items->Objects[ii];
//
//          if (p)
//            ShowMessage(IntToHex((int)p->x, 6) + " : " + IntToHex((int)p->y, 6));
//        }
//      }
    }
    catch(const std::exception& e)
    {
      printf("ReadIniFile() Exception: %s :\n", e.what());
      bRet = false;
    }
  }
  __finally
  {
    try
    {
      if (pIni != NULL)
        delete pIni;
    }
    catch(...) {}
  }

  return bRet;
}
//---------------------------------------------------------------------------
int __fastcall TFavColorsForm::ReadColorsFromIni(TIniFile* pIni)
{
  if (!pIni)
    return -1;

  int count = -1;

  try
  {
    String sReadIni;

    if (pIni->SectionExists(INI_FAVCOLORS_SECTION_NAME))
    {
      sReadIni = pIni->ReadString(INI_FAVCOLORS_SECTION_NAME, "count", "");

      if (!sReadIni.IsEmpty())
      {
        count = sReadIni.ToIntDef(-1);

        if (count > 0)
        {
          // read new-style of storage up to 100 lines of colors like:
          // colors0=ffffff 000000
          // colors1=aaaaaa 555555
          // (there must be no breaks between 0 and 199)
          for (int ii = 0; ii < count && ii < INI_FAVCOLORS_LINES_MAX; ii++)
          {
            sReadIni = pIni->ReadString(INI_FAVCOLORS_SECTION_NAME,
                                                "colors" + String(ii), "");

            sReadIni = sReadIni.Trim().LowerCase();

            if (sReadIni.IsEmpty())
              continue;

            try
            {
              int fg = NO_COLOR;
              int bg = NO_COLOR;

              int ret = sscanf(sReadIni.c_str(), "%x %x", &fg, &bg);

              // Scan the string to get fg/bg colors then add them to
              // our stringlist. The text is is saved in the list in UTF8
              // format.
              if (ret == 2 && fg != NO_COLOR && bg != NO_COLOR)
                ListBox->Items->AddObject(FAVCOLORSDLG[2],
                                               (TObject*)new TPoint(fg, bg));
            }
            catch(...) {} // error
          }
        }
      }
    }
  }
  catch(...) {}

  return count;
}
//---------------------------------------------------------------------------
bool __fastcall TFavColorsForm::WriteColorsToIni(void)
// Writes TPoint x (foreground) and y (background) colors in ListBox->Objects
// to ini-file strings in section "FavoriteColors"
{
  bool bRet;

  TIniFile *pIni = NULL;

  try
  {
    try
    {
//      pIni = utils->OpenIniFile(L"C:\\Users\\Scott\\Documents\\Borland\\Projects\\YahCoLoRiZe\\Colorize.ini");
      pIni = utils->OpenIniFile();

      if (pIni == NULL)
        return false;

      pIni->EraseSection(INI_FAVCOLORS_SECTION_NAME);
      int count = ListBox->Items->Count;

      String s;
      TPoint* p;

      // Write the count
      pIni->WriteString(INI_FAVCOLORS_SECTION_NAME, "count", String(count));

      for (int ii = 0; ii < count && ii < INI_FAVCOLORS_LINES_MAX; ii++)
      {
        if ((p = (TPoint*)ListBox->Items->Objects[ii]) != NULL)
        {
          s = IntToHex((int)p->x, 6) + " " + IntToHex((int)p->y, 6);
          pIni->WriteString(INI_FAVCOLORS_SECTION_NAME,
                                            "colors" + String(ii), s);
        }
      }


      bRet = true;
    }
    catch(const std::exception& e)
    {
      printf("WriteDictToIniFile() Exception: %s :\n", e.what());
      bRet = false;
    }
  }
  __finally
  {
    try {if (pIni != NULL) delete pIni;} catch(...) {}
  }

  return bRet;
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::Export(void)
{
  if (SFDlgForm || !ListBox->Items->Count) // just ONE!
    return;

  // User clicked Export Text button
  try
  {
    WideString wFilter = WideString("All files (*.*)|*.*|"
                                   "Text files (*.txt)|*.txt");

    // "FavColors.txt", // 27
    WideString wFile = utils->GetSaveFileName(wFilter, FN[27],
                                            dts->DeskDir, String(DS[159]));

    if (!wFile.Length())
      return;

    if (utils->FileExistsW(wFile))
    {
      if (utils->ShowMessageU(Handle, DS[160],
                      MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1) == IDNO)
        return; // Cancel
    }

    // open or create file
    FILE *f = _wfopen(wFile.c_bstr(), L"w");

    if (!f)
    {
      utils->ShowMessageU(String(DS[161]));
      return;
    }

    try
    {
      fprintf(f, "%s", "FAVCOLOR:\n");

      TPoint* p;

      // Print to ASCII hex format
      int count = ListBox->Items->Count;

      for (int ii = 0; ii < count; ii++)
      {
        if ((p = (TPoint*)ListBox->Items->Objects[ii]) != NULL)
          fprintf(f, "%06x %06x\n", p->x, p->y);
      }
    }
    __finally
    {
      fclose(f);
    }
  }
  catch(...)
  {
      utils->ShowMessageU(String(DS[162]));
  }
}
//---------------------------------------------------------------------------
void __fastcall TFavColorsForm::Import(void)
{
  if (OFSSDlgForm) // just ONE!
    return;

  // User clicked Import Text button
  try
  {
    WideString wFilter = WideString("All files (*.*)|*.*|"
                    "Text files (*.txt)|*.txt");

    // Run the open-file dialog in FormOFDlg.cpp
    WideString wFile = utils->GetOpenFileName(wFilter, 2, dts->DeskDir,
                                                            String(DS[163]));

    // Load and convert file as per the file-type (either plain or rich text)
    if (wFile.IsEmpty())
      return;

    FILE *in = _wfopen(wFile.c_bstr(), L"r");

    if (!in)
    {
      utils->ShowMessageU(String(DS[164]));
      return;
    }

    TPoint* p = new TPoint[INI_FAVCOLORS_LINES_MAX];

    if (!p)
    {
      utils->ShowMessageU(String(DS[165]));
      return;
    }

    try
    {
      char s[10];

      if (fscanf(in, "%9s\n", s) != 1 || strcmp(s, "FAVCOLOR:") != 0)
      {
        utils->ShowMessageU(String(DS[164]));
        return;
      }

      int ii;

      for (ii = 0; ii < INI_FAVCOLORS_LINES_MAX; ii++)
      {
        // read 8 chars max per field into TPoint structs
        if (fscanf(in, "%8x %8x\n", &p[ii].x, &p[ii].y) == EOF)
          break;
      }

      if (ii == 0)
        return;

      ListBox->Items->Clear();

      int fg, bg;

      for (int jj = 0; jj < ii && jj < INI_FAVCOLORS_LINES_MAX; jj++)
      {
        fg = p[jj].x;
        bg = p[jj].y;

        if (fg != NO_COLOR && bg != NO_COLOR)
          ListBox->Items->AddObject(FAVCOLORSDLG[2],
                                       (TObject*)new TPoint(fg, bg));
      }

      this->bChanged = true;
    }
    __finally
    {
      fclose(in);
      delete [] p;
    }

  }
  catch(...)
  {
      utils->ShowMessageU(String(DS[165]));
  }
}
//---------------------------------------------------------------------------

