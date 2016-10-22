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
#include "ConvertToHLT.h"

#pragma package(smart_init)

#define QUITSIZE 600000

//---------------------------------------------------------------------------
TConvertToHLT* ConvertToHLT = NULL;;
//---------------------------------------------------------------------------

// Used when the View will become "IRC Codes" but the IRC codes have not yet been
// converted to a readable form...
//
// Idea of this class is to take RTF text in the rich-edit control that
// has IRC control-codes in it... and convert them to a readable character
// colored red and stored in a TMemoryStream.
//
// The resulting stream is usually then loaded back into the Rich-Edit
// control using re->Lines->LoadFromStream(ms) to show IRC code-highlighting.

__fastcall TConvertToHLT::TConvertToHLT(TComponent* Owner, TMemoryStream* MS, TTaeRichEdit* RE)
{
  this->dts = static_cast<TDTSColor*>(Owner);

  ms = MS;
  re = RE;

  MS_Temp = NULL;
}

__fastcall TConvertToHLT::~TConvertToHLT(void)
{
}
//---------------------------------------------------------------------------

int __fastcall TConvertToHLT::Execute(bool bShowStatus)
// Returns 2 if error
{
  if (!ms)
    return 2;

  // Change processing status panel... Processing text-highlight chars
  if (bShowStatus)
    dts->CpShow(STATUS[9], DS[70]);

  MS_Temp = new TMemoryStream();

  int ReturnValue;

  try
  {
    try
    {
      ms->Clear();

      // Save rich-text version of IRC-formatted data in MS_Temp
      //
      // "stream" is a pointer to the destination stream.  If
      // "selectionOnly" is true, then only the text currently
      // selected in the control will be copied to the stream.
      // If "plainText" is true, the text will be converted to
      // plain ASCII text as it is copied to the stream; otherwise,
      // the text will be copied to the stream in RTF format.
      // If "noObjects" is true, embedded OLE objects will not be
      // copied to the stream.  If "plainRtf" is true, language-specific
      // RTF keywords are not used (only keywords common to all
      // languages are used).
      //
      // Stream, SelectionOnly, PlainText, NoObjects, PlainRTF
      re->CopyToStream(MS_Temp, false, false, true, false);

      // Test code to view this RTF file!!!!!!!!!!!!!!!!
      //MS_Temp->Position = 0;
      //DTSColor->TaeEdit->Clear();
      //DTSColor->TaeEdit->PasteFromStream( MS_Temp, false, true, true );
      //return(0);

      if (!MS_Temp->Size)
        return 2; // Error

      ReturnValue = ScanForString("{\\colortbl", MS_Temp, true, bShowStatus);

      if (ReturnValue == 2) // No color-table
      {
        ReturnValue = ScanForString("\\viewkind", MS_Temp, true, bShowStatus);

        if (ReturnValue == 0) // OK
        {
          // Scan back before "\viewkind" in both streams
          MS_Temp->Seek(-9,soFromCurrent);
          ms->Seek(-9,soFromCurrent);
        }
      }
      else if (ReturnValue == 0) // Found existing color table
      {
        // Scan back before "{\colortbl" in MS_Temp since we'll replace it
        ms->Seek(-10,soFromCurrent);

        // Scan past existing table in MS_Temp but don't write it to MS_Temp
        ReturnValue = ScanForString(";}", MS_Temp, false, bShowStatus);
      }

      if (ReturnValue == 0) // no errors or user-cancel yet?
      {
        ReplaceColortbl();
        ReturnValue = ScanForCtrlK(MS_Temp);
      }
    }
    catch(...)
    {
#if DEBUG_ON
      dts->CWrite("\r\nTConvertToHLT::Exception in Execute()\r\n");
#endif
      ReturnValue = 2; // Error
    }
  }
  __finally
  {
    if (MS_Temp)
      delete MS_Temp;

    dts->CpHide();
  }

  return ReturnValue;
}
//---------------------------------------------------------------------------
void __fastcall TConvertToHLT::ReplaceColortbl(void)
{
  // Make an RTF color-palette from DTSColor->Palette[]
  String ColorTbl = "{\\colortbl;";
  String Temp, Red, Green, Blue;

  int iRed, iGreen, iBlue;

  for (int ii = 0; ii < DTSColor->PaletteSize; ii++)
  {
    // RGB Palette entry
    Temp = IntToHex(DTSColor->Palette[ii], 6);

    Red = "0x" + String(Temp[1]) + String(Temp[2]);
    Green = "0x" + String(Temp[3]) + String(Temp[4]);
    Blue = "0x" + String(Temp[5]) + String(Temp[6]);

    iRed = Red.ToIntDef(0);
    iGreen = Green.ToIntDef(0);
    iBlue = Blue.ToIntDef(0);

    ColorTbl += "\\red" + String(iRed) +
                  "\\green" + String(iGreen) +
                          "\\blue" + String(iBlue) + ";\r\n";
  }

  ColorTbl += "}";

  ms->WriteBuffer(ColorTbl.c_str(), ColorTbl.Length());

  String Gen = "{\\*\\generator YahCoLoRiZe " + String(REVISION) + ";}";
  ms->WriteBuffer(Gen.c_str(), Gen.Length());
}
//---------------------------------------------------------------------------
int __fastcall TConvertToHLT::ScanForString(char CompareStr[],
                      TMemoryStream* MS_Temp, bool bWrite, bool bShowStatus)
// Scans the entire stream for the string and returns 0
// if it exists.  It writes the data to the "processed"
// stream up to the end of the compare string.
// Each time it is called it resets the pointer to the
// input and output streams.
//
// Setting bWrite will scan MS_Temp from the current
// position looking for the string and will not affect DTSColor->MS_RTF
//
// Returns 1 if user-cancel, 2 if no match and 0 if match
{
  char Data;
  int CompareIdx = 0;
  int Length;

  try
  {
    if (bWrite)
    {
      MS_Temp->Position = 0;
      ms->Clear();
    }

    Length = strlen(CompareStr);

    while (GetAsyncKeyState(VK_ESCAPE ) < 0); // Dummy read to clear...

    while (MS_Temp->Read(&Data,1))
    {
      // User abort???
      Application->ProcessMessages();
      if ((int)GetAsyncKeyState(VK_ESCAPE) < 0)
        return 1; // Cancel

      if (bWrite) ms->WriteBuffer(&Data,1);

      if (Data == CompareStr[CompareIdx])
      {
        if (++CompareIdx >= Length)
          return 0; // Match
      }
      else
        CompareIdx = 0;
    }
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nTConvertToHLT::Exception in ScanForString()\r\n");
#endif
  }

  return 2; // No Match
}

//---------------------------------------------------------------------------
int __fastcall TConvertToHLT::ScanForCtrlK(TMemoryStream * MS_Temp)

// Returns 1 if user-cancel, 2 if error and 0 if success
{
  char Data[15];
  char c;
  int BytesRead;

  int Color = dts->cColor;
  if (Color <= 0 || Color > DTSColor->PaletteSize) Color = IRCBLACK;

  int HlColor = HIGHLIGHT_COLOR;
  if (HlColor <= 0 || HlColor > DTSColor->PaletteSize) HlColor = IRCRED;

  // X is a placeholder!!!
  String Temp = "cf" + String(HlColor) + " X";
  int HlIdx = Temp.Length(); // 1-based index of X

  // Back to regular color...
  Temp += "\\cf" + String(Color) + " ";

  try
  {
    dts->CpSetMaxIterations(MS_Temp->Size);

    while (GetAsyncKeyState(VK_ESCAPE) < 0); // Dummy read to clear...

    while (MS_Temp->Read(Data, 1) && Data[0] != C_NULL)
    {
      ms->WriteBuffer(Data, 1);

      // User abort???
      Application->ProcessMessages();

      if ((int)GetAsyncKeyState(VK_ESCAPE) < 0)
        return 1; // Cancel

      // Advance progress-bar
      dts->CpUpdate(MS_Temp->Position);

      if (Data[0] == '\\') // Backslash
      {
        Data[3] = C_NULL;

        // Read ahead for embedded codes
        if ((BytesRead = MS_Temp->Read(Data, 3)) == 3)
        {
          // CTRL_K
          if (!strcmp(Data,"'03"))
            c = HL_K;
          // CTRL_O
          else if (!strcmp(Data,"'0f"))
            c = HL_O;
          // FF (page break)
          else if (!strcmp(Data,"'0c")) // C_FF
            c = HL_N;
            // TAB
          else if (!strcmp(Data,"'09"))
            c = HL_T;
          // CTRL_B
          else if (!strcmp(Data,"'02"))
            c = HL_B;
          // CTRL_U
          else if (!strcmp(Data,"'1f"))
            c = HL_U;
          // CTRL_R
          else if (!strcmp(Data,"'16"))
            c = HL_R;
          // CTRL_PUSH
          else if (!strcmp(Data,"'11"))
            c = HL_PUSH;
          // CTRL_POP
          else if (!strcmp(Data,"'12"))
            c = HL_POP;
          // CTRL_F
          else if (!strcmp(Data,"'13"))
            c = HL_F;
          // CTRL_S
          else if (!strcmp(Data,"'14"))
            c = HL_S;
          else
            c = C_NULL; // We don't have a control-char

          if (c != C_NULL)
          {
            Temp[HlIdx] = c;
            ms->WriteBuffer(Temp.c_str(),Temp.Length());
          }
          else
            MS_Temp->Seek(-BytesRead,soFromCurrent);
        }
        else // End of stream, seek back
          MS_Temp->Seek(-BytesRead,soFromCurrent);
      }
    }

    return 0;
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nTConvertToHLT::Exception in ScanForCtrlK()\r\n");
#endif
    return 2; // Error
  }
}
//---------------------------------------------------------------------------
