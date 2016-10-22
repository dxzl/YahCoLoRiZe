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

#include "ConvertToIRC.h"
#include "Utils.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
TConvertToIRC* ProcessIRC = NULL;
//---------------------------------------------------------------------------

// Constructor
__fastcall TConvertToIRC::TConvertToIRC(TComponent* Owner)
{
  this->dts = static_cast<TDTSColor*>(Owner);

  LineWidth = dts->LineWidth;
  LineHeight = dts->LineHeight;
  LineSpacing = dts->RegSpacing;

  if (dts->Margins->Checked)
  {
    Lmargin = dts->RegLmargin;
    Rmargin = dts->RegRmargin;
    Tmargin = dts->RegTmargin;
    Bmargin = dts->RegBmargin;
    Indent = dts->RegIndent;
  }
  else // Disable all margins
  {
    Lmargin = 0;
    Rmargin = 0;
    Tmargin = 0;
    Bmargin = 0;
    Indent = 0;
  }

  PUSHSTRUCTS = NULL;
  PushIdx = 0;
  pBuf = NULL;
  iSize = 0;
  iBuf = 0;
  OrigLineCount = 0;
  OrigLineWidth = 0;
  TotalWingLength = 0;
  IRCLineCount = 0;

  // Used to keep from repeating colors
  dts->PrevFG = NO_COLOR;
  dts->PrevBG = NO_COLOR;
  dts->PrevAwc = NO_COLOR;
}
//---------------------------------------------------------------------------
// Destructor
__fastcall TConvertToIRC::~TConvertToIRC(void)
{
}
//---------------------------------------------------------------------------
int __fastcall TConvertToIRC::Execute(bool bShowStatus)
{
  if (dts->SL_IRC == NULL)
    return 10;

  if (utils == NULL)
    return 11;

  int ReturnValue = 0;

  // Change processing status panel... Processing to IRC format
  if (bShowStatus)
    dts->CpShow(STATUS[3], DS[70]);

  try
  {
    try
    {
      // Move edit text to a buffer (iSize will include the NULL!)
      utils->MoveMainTextToBuffer(pBuf, iSize);

      // Flatten tabs
      utils->FlattenTabs(pBuf, iSize, dts->RegTabMode);

      // Flatten tabs
      // this will search for "<<page>" and replace them all with C_FF
      // so that we can delay-process them below and replace the C_FF
      // with "\pagebreak" again - but on their own lines, post-text-processing.
      utils->PageBreaksToFormFeed(pBuf, iSize);

      if (pBuf == NULL || iSize == 0)
      {
        if (pBuf != NULL)
          delete [] pBuf;

        return 12; // Error
      }

      // Clear target-stream (do this after MoveMainText()!
      dts->SL_IRC->Clear();

      // Scan text buffer for longest line width and original text height
      // values are returned by reference...
      bOrigTextHasColor = utils->LongestWidthAndLineCount(pBuf, iSize,
                                          OrigLineWidth, OrigLineCount);

      bOrigTextHasBold = utils->ContainsChar(WideString(pBuf, iSize), CTRL_B);
      bOrigTextHasUnderline = utils->ContainsChar(WideString(pBuf, iSize), CTRL_U);
      bOrigTextHasItalics = utils->ContainsChar(WideString(pBuf, iSize), CTRL_R);

      // No real chars here?
      if (OrigLineWidth == 0 && OrigLineCount == 0)
      {
        if (pBuf)
          delete [] pBuf;

        return 13; // Error
      }

      bool bHaveTopBottom = (dts->Borders->Checked && dts->bTopEnabled);
      bool bHaveBoxes = (dts->AutoHeight->Checked || dts->LineHeight != 0);

      bIndentEnabled = (dts->LeftJustify->Checked &&
                            !dts->Wingdings->Checked &&
                                   Indent != 0) ? true : false;

      // Auto-linesize
      //
      // Note: Search for the flag "bUserEditedWidth" in Main.cpp.
      // If not set, we set dts->AutoWidth->Checked when
      // re-processing RTF text (and also clear PackText->Checked).
      // bUserEditedWidth is cleared when the user manually changes
      // line-width or if the user types new text into the main window.
      if (dts->AutoWidth->Checked)
      {
        // Need to include width of largest enabled wing-pair
        // (Auto LWidth includes Wings but not Margins or Side-borders)
        LineWidth = OrigLineWidth + LargestWingLength();

        if (LineWidth != OrigLineWidth)
        {
          // Show the auto-value we have set (supress on-change event)
          dts->WidthEdit->OnChange = NULL;
          dts->WidthEdit->Color = clYellow;
          dts->WidthEdit->Text = String(LineWidth);
          dts->WidthEdit->OnChange = dts->WidthEditEnter;
        }
      }

      // Set Indent off if LineWidth will go < 1 char (Do this before
      // SetLineWidth() because it checks bIndentEnabled)
      if (bIndentEnabled)
      {
        int Temp;

        if (Indent < 0)
          Temp = -Indent;
        else
          Temp = Indent;

        if (LineWidth - Temp <  MINLINESIZE)
          bIndentEnabled = false;
      }

      // Adjust linesize up to allow top/bottom borders. LineWidth is set
      // and is the desired width of text with wings, but without Side borders
      // or margins. Expand line-width to fit top/bottom border
      // section-multiples.
      if (bHaveTopBottom)
        SetLineWidth(Lmargin, Rmargin, Indent);

      // Choose initial wings for very first word...
      int iDummy;
      NextWordLength = utils->GetWordLength(pBuf, iSize,
                  utils->SkipAllCodes(pBuf, 0, iSize), iDummy);

      TotalWingLength = SelectWings(NextWordLength);

      // If no top/bottom borders and auto-line width and there is just one
      // line of text... reset LWidth to length of wings just selected plus
      // the raw text.
      if (!bHaveTopBottom && dts->AutoWidth->Checked && OrigLineCount == 1)
      {
        LineWidth = TotalWingLength + OrigLineWidth;

        // Show the auto-value we have set (supress on-change event)
        dts->WidthEdit->OnChange = NULL;
        dts->WidthEdit->Color = clYellow;
        dts->WidthEdit->Text = String(LineWidth);
        dts->WidthEdit->OnChange = dts->WidthEditEnter;
      }

      SideLength = GetSideBorderTotalWidth(); // Set left+right lengths

      // Seed random-number generator
      time_t t;
      srand((unsigned) time(&t));

      // This will affect the progress-bar "max number of loops" var!
      if (dts->Foreground == IRCVERTBLEND || dts->Background == IRCVERTBLEND)
        MaxIterations = 2;
      else
        MaxIterations = 1;

      Iterations = 0;
      ActualLineCount = 0;

      // Processed text strings
      EditStrings = new TStringsW();

      // Dummy iteration to determine actual count of lines
      // for use in vertical blend...
      while (++Iterations <= MaxIterations)
      {
        PushPopLineCounter = 0;

        EditStrings->Clear();

        // For vertical-blend we iterate twice, once
        // to obtain the rendered line-count (too complex to compute)
        if (Iterations >= MaxIterations)
        {
          ActualLineCount = IRCLineCount;

          // Set colors after ActualLineCount is known (after dummy iteration)
          dts->ComputeAllColors(ActualLineCount, 0);
        }

        IRCLineCount = 0;

        // Set if hyphenated word's first-half will fit in a line
        // but the second-half won't... this flag triggers AddColorLine
        // to be called on the next '-' or '/' char.
        bAddLineOnHyphen = false;

        bIsFirstLineOfPar = true;

        bool bFoundCRLF = false;

        TextLine = "";
        PrintableText = "";

        // Need this to apply the user's font underline, bold or italics from
        // the main dialog!
        InitBoldUnderlineReverseForNextLine(); // Populate PUSHSTRUCT "State"

        // Append bold/underline/reverse to TextLine if State value false...
        ProperlyTerminateEffects(); // Add bold etc. codes for new state

        if (State.fg == NO_COLOR)
          State.fg = dts->Afg;

        if (State.bg == NO_COLOR)
          State.bg = dts->Abg;

        if (State.fontSize == -1)
          State.fontSize = dts->cSize;

        if (State.fontType == -1)
          State.fontType = dts->cType;

        // Add the top border if "Add Color" Checked
        if (dts->Borders->Checked && dts->bTopEnabled)
        {
          TopStr = "";

          int TotalWidth = LineWidth + Lmargin + Rmargin +
                                                GetSideBorderTotalWidth();

          for (int TempInt = 0; TempInt < TotalWidth; TempInt += TopLength)
          {
            // Add Font Type/Size - Optimizer will flatten this...
            TopStr += utils->FontTypeToString(dts->cType);
            TopStr += utils->FontSizeToString(dts->cSize);
            TopStr += dts->TopBorder;
          }

          EditStrings->Add(TopStr);
        }

        // Add "Margin" from top...
        for (int ii = 0; ii < Tmargin; ii++)
          AddColorLine(false);

        HeightCounter = LineHeight; // # lines per box

        // Init the Push/Pop memory
        InitPushPop();

        dts->CpSetMaxIterations(iSize);

        while (GetAsyncKeyState(VK_ESCAPE) < 0); // Dummy read to clear...

        wchar_t c;

        dts->CpSetMaxIterations(iSize); // set progress bar max iterator

        for (iBuf = 0; iBuf < iSize; iBuf++)
        {
          // User abort???
          Application->ProcessMessages();

          if ((int)GetAsyncKeyState(VK_ESCAPE) < 0)
          {
            ReturnValue = 1;
            break; // Cancel
          }

          c = pBuf[iBuf];

          // End of processing?
          if (c == C_NULL)
            break;

          // Tabs should be flattened prior to calling ConvertToIRC()!
          // If not - we filter them out here...
          if (c == C_TAB)
            continue;

          if (c == CTRL_PUSH)
          {
            Push(State);
            PushPopLineCounter = 0;
            continue; // don't add in the push/pop chars
          }

          if (c == CTRL_POP)
          {
            PUSHSTRUCT TempState;
            Pop(TempState);
            DoPop(TempState, TextLine);
            continue; // don't add in the push/pop chars
          }

          if (c == CTRL_F)
          {
            int ft = utils->CountFontSequence(pBuf, iBuf, iSize);

            if (ft >= 0)
            {
              iBuf += 2;

              if (ft == 0)
                // Ctrl-F 0 means we need to insert the default font type
                RestoreFontType(TextLine);
              else
              {
                TextLine += utils->FontTypeToString(ft);
                State.fontType = ft;
                State.bFontTypefast = true;
              }
            }
            else if (dts->IsYahTrinPal()) // need to write CTRL_F
              // Ctrl-F alone means we need to insert the default font type
              RestoreFontType(TextLine);

            continue;
          }

          if (c == CTRL_S)
          {
            int fs = utils->CountFontSequence(pBuf, iBuf, iSize);

            if (fs >= 0)
            {
              iBuf += 2;

              if (fs == 0)
                // Ctrl_S 0 means we need to insert the default font size
                RestoreFontSize(TextLine);
              else
              {
                TextLine += utils->FontSizeToString(fs);
                State.fontSize = fs;
                State.bFontSizefast = true;
              }
            }
            else if (dts->IsYahTrinPal()) // need to write CTRL_S
              // Ctrl_S alone means we need to insert the default font size
              RestoreFontSize(TextLine);

            continue;
          }

          if (c == CTRL_K)
          {
            int fg = NO_COLOR;
            int bg = NO_COLOR;

            int Count = utils->CountColorSequence(pBuf, iBuf, iSize, fg, bg);

            if (Count)
            {
              if (fg != NO_COLOR)
              {
                State.fg = fg;
                State.bFGfast = true; // Lock color-changes
              }

              if (bg != NO_COLOR)
              {
                State.bg = bg;
                State.bBGfast = true; // Lock color-changes
              }

              // Add the color-sequence to the current line
              utils->WriteColors(fg, bg, TextLine);
              iBuf += Count;
            }
            else
              RestoreFGBG(TextLine); // Ctrl-K alone...

            continue;
          }

          // Strip-out CTRL_Os (they will build up!)
          if (c == CTRL_O)
          {
            if (State.bBold)
            {
              TextLine += WideString(CTRL_B);
              State.bBold = false;
            }

            if (State.bUnderline)
            {
              TextLine += WideString(CTRL_U);
              State.bUnderline = false;
            }

            if (State.bItalics)
            {
              TextLine += WideString(CTRL_R);
              State.bItalics = false;
            }

            RestoreFGBG(TextLine);
            RestoreFontSize(TextLine);
            RestoreFontType(TextLine);
            continue;
          }

          if (c == CTRL_B)
            State.bBold = !State.bBold;
          else if (c == CTRL_U)
            State.bUnderline = !State.bUnderline;
          // Defer page-breaks until after a complete line is generated
          else if (c == CTRL_R)
            State.bItalics = !State.bItalics;
          // Line-break?
          else if (utils->FoundCRLF(pBuf, iSize, iBuf))
          {
            iBuf++;
            bFoundCRLF = true;

            // Look for a paragraph
            if (utils->FoundCRLF(pBuf, iSize, iBuf))
            {
              iBuf++;

              // Bypass extra lines between paragraphs
              while (utils->FoundCRLF(pBuf, iSize, iBuf))
              {
                if (!dts->PackLines->Checked)
                  AddColorLine(true);

                iBuf++;
              }

              // Add the line
              AddColorLine(true); // Don't pass on colors!

              if (!bHaveTopBottom || !bHaveBoxes)
                AddColorLine(true);

              PrintTopBottomBorders(EditStrings);

              bIsFirstLineOfPar = true;
            }
            else if (dts->PackText->Checked)
            {
              // Insert a space into the buffer one back
              // (tricky! - Replace LF with space)
              // This is the way to join two lines...
              pBuf[iBuf-1] = C_SPACE;
              PrintableText += C_SPACE;
              bFoundCRLF = false; // No crlf anymore!

              // Don't carry trailing colors to new spliced-in line!
              // (only flaw is that the space char used to splice two lines
              // will appear as a "gap" in an otherwise continuous color
              // text-effect... but it's the only way!)
              RestoreFontSize(TextLine);
              RestoreFGBG(TextLine);
            }
            else
            {
              AddColorLine(true);

              // Check for a page-break following a line-break
              if (iBuf < iSize && pBuf[iBuf] == C_FF)
              {
                // if page-break, add it
                EditStrings->Add(PAGE_BREAK);

                iBuf++; // skip the C_FF

                // Skip cr/lf after page-break (if any)
                if (utils->FoundCRLF(pBuf, iSize, iBuf))
                  iBuf++; // skip the C_LF
                else
                  EditStrings->Add(""); // add cr/lf after PAGE_BREAK
              }
            }

            // We are one beyond any CRLFs now... so go back one.
            // The for-loop will increment it next pass
            iBuf--;
          }
          else
          {
            // check for C_FF (a page-break that was NOT preceeded
            // by a line-break and handled above). Locate this AFTER
            // calling FoundCRLF()!
            if (c == C_FF)
            {
              AddColorLine(true); // add what we have so-far

              EditStrings->Add(PAGE_BREAK);

              int iTemp = iBuf + 1; // skip the C_FF

              // Skip cr/lf after page-break (if any)
              if (utils->FoundCRLF(pBuf, iSize, iTemp))
                iBuf += iTemp-iBuf; // skip the CR or CRLF

              continue; // Do NOT add C_FF to TextLine!
            }

            // This string only has the printable-characters, no codes
            PrintableText += WideString(pBuf[iBuf]);
          }

          if (iBuf >= iSize)
            break;

          // Build a string of raw text. This string has the
          // text formatting codes AND printable characters
          // But no CR/LFs.
          if (!bFoundCRLF)
            TextLine += pBuf[iBuf];

          // Length of printable chars for text and wings
          int TotalLength = PrintableText.Length() + TotalWingLength;

          // TempWidth used to account for indent
          int TempWidth = LineWidth;

          if (bIndentEnabled)
          {
            if (bIsFirstLineOfPar)
            {
              if (Indent >= 0)
                TempWidth -= Indent;
            }
            else
            {
              // Not 1st line of a paragraph
              if (Indent < 0)
                TempWidth -= -Indent;
            }
          }

          // If adding the next word causes us to go over, print
          // the line before the next word is added...
          if (TotalLength < TempWidth)
          {
            WideString Smiley;
            bool bCheckWordLength;
            NextWordLength = 0;

            // We don't want to chop off the line in the middle of a
            // word or smiley... point idx to the next char
            // (next char could be anything).
            int idx = iBuf+1;

            if (idx >= iSize)
              idx--; // limit it!

            // Looking for a word-length breaks out on a CR/LF so skip
            // any in our immediate path...
            if (bFoundCRLF && dts->PackText->Checked)
            {
              idx = utils->SkipCRLF(pBuf, iSize, iBuf);
              bCheckWordLength = true; // Check 1st word of next line
            }
            else
              bCheckWordLength = (pBuf[iBuf] == C_SPACE) ? true : false;

            int hyphidx = -1;

            // AutoDetectEmotes Checked keeps a smiley intact if its embedded
            // in a word. Ex:  "h:-)owdy!"
            if (dts->AutoDetectEmotes->Checked &&
                                utils->SkipSmileys(idx, pBuf, Smiley, iSize))
              NextWordLength = Smiley.Length();
            else if (bCheckWordLength)
              NextWordLength = utils->GetWordLength(pBuf, iSize, idx, hyphidx);

            if (bAddLineOnHyphen)
            {
              if (pBuf[iBuf] == HYPHENCHAR1 || pBuf[iBuf] == HYPHENCHAR2)
                AddColorLine(false);
            }
            // if at end of line, don't want to push the CR/LF
            // into the next line...
            else if (!utils->FoundCRLF(pBuf, iSize, idx, true))
            {
              if (TotalLength + NextWordLength > TempWidth)
              {
                if (hyphidx >= 0 && TotalLength + hyphidx + 1 <= TempWidth)
                  bAddLineOnHyphen = true;
                else
                  AddColorLine(false);
              }
            }

            // First line of a new paragraph is here (diagnostic)
  //        else if (bIsFirstLineOfPar)
  //          utils->ShowMessageU(PrintableText);

          hyphidx = -1; // reset
          }
          // If line is over-length it will be over by 1,
          // remove the char we just added and print the line.
          //
          // Reason we do not want to use:
          //          if (TotalLength == LWidth)...
          // Is that the line may still have color-codes followed
          // by a CR/LF - so if we add the line prior to processing
          // those codes we will end up with an extra line being
          // printed!
          else if (TotalLength > TempWidth)
          {
            if (dts->Wingdings->Checked)
            {
              // Remove this char and print the line
              PrintableText = utils->DeleteW(PrintableText, PrintableText.Length(), 1);
              TextLine = utils->DeleteW(TextLine, TextLine.Length(), 1);

              // Walk the buffer back one unless we want to skip over a space
              // or tab that would lead the next line...
              if (pBuf[iBuf] != C_SPACE && iBuf > 0)
                iBuf--;

                // Need to skip over non-printable chars
              AddColorLine(false);
            }
            else
            {
              // Remove this char and print the line
              PrintableText = utils->DeleteW(PrintableText, PrintableText.Length(), 1);
              TextLine = utils->DeleteW(TextLine, TextLine.Length(), 1);

              // Walk the buffer back one unless we want to skip over a space
              // or tab that would lead the next line...
              if (pBuf[iBuf] != C_SPACE && iBuf > 0)
                iBuf--;

              // Need to skip over non-printable chars
              AddColorLine(false);
            }
          }

          bFoundCRLF = false;

          // Advance progress-bar
          dts->CpUpdate(iBuf);
        } // End for loop

        if (!PrintableText.IsEmpty())
          AddColorLine(false);

        if (ReturnValue != 0)
          break;

      } // Iteration loop to find actual line-count

      try { delete [] pBuf; }
      catch(...){ReturnValue = 16;} // Error

      InitPushPop(); // Destroy object

      // Add "Margin" from bottom...
      for (int ii = 0 ; ii < Bmargin ; ii++)
        AddColorLine(false);

      // Add the bottom border if "Add Color" Checked
      // (CTRL_O tells mIRC to turn off text-effects)
      if (dts->Borders->Checked && dts->bTopEnabled)
        EditStrings->Add(TopStr); // Add bottom border

      // Optimize
      WideString sTemp = utils->Optimize(EditStrings->Text, bShowStatus);

      delete EditStrings;

      // Save plain-text version of IRC-formatted data in dts->SL_IRC
      dts->SL_IRC->Text = sTemp;
    }
    catch(...)
    {
      ReturnValue = 15; // Error
    }
  }
  __finally
  {
    dts->CpHide();
  }

  return ReturnValue;
}
//---------------------------------------------------------------------------
bool __fastcall TConvertToIRC::PrintTopBottomBorders(TStringsW* EditStrings)
{
  try
  {
    if (!PrintableText.IsEmpty())
      AddColorLine(false);

    // Add bottom-border and spacing if Auto-Height Checked
    if (dts->Borders->Checked && dts->bTopEnabled && dts->AutoHeight->Checked)
    {
      // Add "Margin" from bottom...
      for (int ii = 0 ; ii < Bmargin ; ii++)
        AddColorLine(false);

      EditStrings->Add(TopStr); // Add bottom of this box

      // Seperate into multiple boxes?
      if (LineSpacing)
      {
        int tls = LineSpacing;

        while (--tls)
          EditStrings->Add(""); // Add empty lines between boxes

        EditStrings->Add(TopStr);  // Add top of next box

        // Add "Margin" from top...
        for (int ii = 0 ; ii < Tmargin ; ii++)
          AddColorLine(false);
      }
    }
  }
  catch(...) {return false;}

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TConvertToIRC::AddColorLine(bool bInhibitColors)
// bInhibitColors defaults to false. Set to true to avoid passing the line's
// end-colors into the next line (used after a cr/lf).
//
// EditStrings is the output string-list.
{
  try
  {
    AddLine();

    // Add the bottom border if "Add Color" Checked (CTRL_O tells mIRC
    // to turn off text-effects)
    if (dts->Borders->Checked && dts->bTopEnabled && LineHeight &&
                                                !dts->AutoHeight->Checked)
    {
      if (--HeightCounter <= 0)
      {
        // Add "Margin" from bottom...
        for (int ii = 0 ; ii < Bmargin ; ii++)
        {
          InitNextLine(bInhibitColors);
          AddLine();
        }

        EditStrings->Add(TopStr); // Add bottom of this box
        HeightCounter = LineHeight;

        // Seperate into multiple boxes?
        if (LineSpacing)
        {
          int temp = LineSpacing;

          while (--temp)
            EditStrings->Add("");

          EditStrings->Add(TopStr); // Add top of next box

          // Add "Margin" from top...
          for (int ii = 0 ; ii < Tmargin ; ii++)
          {
            InitNextLine(bInhibitColors);
            AddLine();
          }
        }
      }
    }

    // Don't reset "first line of paragraph" flag if we printed
    // a blank line... (used for applying +/- paragraph indent)
    if (bIsFirstLineOfPar && !utils->IsAllSpaces(PrintableText))
      bIsFirstLineOfPar = false;

    InitNextLine(bInhibitColors);
    ProperlyTerminateEffects();

    if (bAddLineOnHyphen)
      bAddLineOnHyphen = false;
  }
  catch(...)
  {
    return false;
  }

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TConvertToIRC::AddLine(void)
// Here we come in with TextLine global containing the basic text-line.
// and EditStrings is the output string-list. TextLine is the raw-codes
// text-accumulator.
//
// We need to add padding, wings and borders and padding-spaces...
{
  try
  {
    // trim any extranious leading spaces
    // (THIS TOTALLY WRECKS ASCII ART!)
    // fixes space from word-break or new paragraph
    //  utils->StripLeadingSpaces(TextLine);

    // When re-processing for effects, we may write bold code across
    // one or more line-breaks (for example).  So we must terminate
    // the effect so it won't carry into the right wing/border.
    // So locate this before the bAddColor flag...
    ProperlyTerminateEffects(); // add to text-line

    // Set ColorFormatStr to default colors and font
    WideString ColorFormatStr;
    utils->WriteColors(dts->Afg, dts->Abg, ColorFormatStr);

    // Add Font Type/Size if YahTrinPal
    if (dts->IsYahTrinPal())
      ColorFormatStr += utils->FontTypeToString(dts->cType) +
                                      utils->FontSizeToString(dts->cSize);

    // Returns length of text with both wings, without color-codes
    int TempLength = AddScriptingChars(ColorFormatStr);

    // TempWidth used to account for indent
    int TempWidth = LineWidth;

    if (bIndentEnabled)
    {
      if (bIsFirstLineOfPar)
      {
        if (Indent >= 0)
          TempWidth -= Indent;
      }
      else if (Indent < 0)
        TempWidth -= -Indent; // Not 1st line of a paragraph
    }

    // Get length of padding needed get LWidth
    int PaddingLength;

    // For a very long "word"...
    if (TempLength > TempWidth)
      PaddingLength = 0;
    else
      PaddingLength = TempWidth-TempLength;

    if (PaddingLength >= 0)
    {
      int MargAdjust = 0;

      if (bIndentEnabled)
      {
        if (bIsFirstLineOfPar)
        {
          if (Indent >= 0)
            MargAdjust = Indent;
        }
        else if (Indent < 0)
          MargAdjust = -Indent; // Not 1st line of a paragraph
      }

      WideString LmarginStr = utils->StringOfCharW(C_SPACE, Lmargin + MargAdjust);
      WideString RmarginStr = utils->StringOfCharW(C_SPACE, Rmargin);

      WideString PaddingStr;

      // Pad with spaces if not "left justify"
      if (!dts->LeftJustify->Checked)
      {
        // No left-justify
        PaddingStr = utils->StringOfCharW(C_SPACE, (PaddingLength/2));

        // Add trailing padding, colors and margin to text
        // unless text is in a "push/pop zone
        TextLine += ColorFormatStr + RmarginStr + PaddingStr;

        // Add a space if odd
        if (PaddingLength & 1)
          PaddingStr += WideString(C_SPACE); // Adjust padding length up by 1

        // Add leading spaces unless text is in a "push/pop zone
        TextLine = utils->InsertW(TextLine, ColorFormatStr + LmarginStr + PaddingStr, 1);
      }
      else // Left-Justify
      {
        PaddingStr = utils->StringOfCharW(C_SPACE, PaddingLength);

        // Add trailing padding, colors and margin to text
        TextLine += ColorFormatStr + RmarginStr + PaddingStr;

        // Add leading margin and colors
        TextLine = utils->InsertW(TextLine, ColorFormatStr + LmarginStr , 1);
      }
    }

    if (dts->Borders->Checked && dts->bSideEnabled)
    {
      // Add the leading border
      TextLine = utils->InsertW(TextLine, dts->LeftSideBorder, 1);

      // Add color-format to beginning of text-string
      TextLine = utils->InsertW(TextLine, ColorFormatStr, 1);

      // Add the trailing border
      TextLine += dts->RightSideBorder;
    }

    EditStrings->Add(TextLine);
  }
  catch(...){return false;}

  return true;
}
//----------------------------------------------------------------------------
bool __fastcall TConvertToIRC::InitNextLine(bool bInhibitColors)
{
  try
  {
    TextLine = "";
    PrintableText = "";

    // Pick wings for next line
    TotalWingLength = SelectWings(NextWordLength);

    // Set colors for next line after ActualLineCount is
    // known (after dummy iteration)
    IRCLineCount++;

    if (PushIdx) PushPopLineCounter++;

    if (Iterations >= MaxIterations)
    {
      // set dts->Afg, dts->Abg and dts->Awc
      dts->ComputeAllColors(ActualLineCount, IRCLineCount);

      // Pass last line's colors into this new line if last line was
      // broken during formatting for width, etc. We don't want to
      // propogate the state for a line that ended with a cr/lf or
      // that had a cr/lf and was spliced together due to PackText
      // being on.
      if (!bInhibitColors)
      {
        if (!State.bFGfast && !State.bBGfast)
        {
          State.fg = dts->Afg;
          State.bg = dts->Abg;
        }
        else if (!State.bFGfast)
          State.fg = dts->Afg;
        else if (!State.bBGfast)
          State.bg = dts->Abg;

        // Write colors for next line
        utils->WriteColors(State.fg, State.bg, TextLine);

        // Write font for next line if YahTrinPal
        if (State.bFontTypefast)
          TextLine += utils->FontTypeToString(State.fontType);
          
        if (State.bFontSizefast)
          TextLine += utils->FontSizeToString(State.fontSize);

        // Set state-flags based upon leading format chars still unprocessed
        // in pBuf and the font-effects mode from the font-dialog
        InitBoldUnderlineReverseForNextLine();
      }
    }
  }
  catch(...) {return false;}
  return true;
}
//---------------------------------------------------------------------------
void __fastcall TConvertToIRC::DoPop(PUSHSTRUCT TempState, WideString &TempStr)
{
  RestoreBoldUnderlineItalics(TempState, TempStr);

  // different font-type?
  RestoreFontType(TempStr, TempState.fontType);

  // different font-size?
  RestoreFontSize(TempStr, TempState.fontSize);

  // Problem is for-example, with Random colors, and you
  // pre-embed push/pop in a text-file, and the push/pop
  // spans more than one line of text, we don't want
  // to restore the color from several lines back!
  // Write in the most recent blend-colors after a pop
  if (PushPopLineCounter) RestoreFGBG(TempStr);
  else RestoreFGBG(TempState.fg, TempState.bg, TempStr);

  // Write in the most recent blend-colors after a pop
  if (dts->Foreground == IRCVERTBLEND || dts->Foreground == IRCHORIZBLEND)
    utils->WritePrevBlendColors(dts->PrevFG, dts->PrevBG, TempStr);
}
//---------------------------------------------------------------------------
// Overloaded!!!!!!!!!!!!!!!!!!

void __fastcall TConvertToIRC::RestoreFGBG(WideString &TempStr)
{
  RestoreFGBG(dts->PrevFG, dts->PrevBG, TempStr);
}

void __fastcall TConvertToIRC::RestoreFGBG(int fg, int bg, WideString &TempStr)
{
  if (State.fg != fg && State.bg != bg)
  {
    utils->WriteColors(fg, bg, TempStr);
    State.fg = fg;
    State.bg = bg;
    State.bFGfast = false; // UnLock color-changes
    State.bBGfast = false; // UnLock color-changes
  }
  else if (State.fg != fg)
  {
    utils->WriteSingle(fg, TempStr, true);
    State.fg = fg;
    State.bFGfast = false; // UnLock color-changes
  }
  else if (State.bg != bg)
  {
    utils->WriteSingle(bg, TempStr, false);
    State.bg = bg;
    State.bBGfast = false; // UnLock color-changes
  }
}
//---------------------------------------------------------------------------
// ft defaults to -1
void __fastcall TConvertToIRC::RestoreFontType(WideString &TempStr, int ft)
{
  if (!dts->IsYahTrinPal())
    return;

  if (ft == -1)
    ft = dts->cType;

  if (State.fontType != ft)
  {
    TempStr += utils->FontTypeToString(ft);
    State.fontType = ft;
    State.bFontTypefast = false; // UnLock font-changes
  }
}
//---------------------------------------------------------------------------
// fs defaults to -1
void __fastcall TConvertToIRC::RestoreFontSize(WideString &TempStr, int fs)
{
  if (!dts->IsYahTrinPal())
    return;

  if (fs == -1)
    fs = dts->cSize;

  if (State.fontSize != fs)
  {
    TempStr += utils->FontSizeToString(fs);
    State.fontSize = fs;
    State.bFontSizefast = false; // UnLock font-changes
  }
}
//---------------------------------------------------------------------------
void __fastcall TConvertToIRC::RestoreBoldUnderlineItalics(PUSHSTRUCT NewState,
                                                            WideString &TempStr)
{
  if (State.bBold != NewState.bBold)
  {
    TempStr += WideString(CTRL_B);
    State.bBold = NewState.bBold;
  }

  if (State.bUnderline != NewState.bUnderline)
  {
    TempStr += WideString(CTRL_U);
    State.bUnderline = NewState.bUnderline;
  }

  if (State.bItalics != NewState.bItalics)
  {
    TempStr += WideString(CTRL_R);
    State.bItalics = NewState.bItalics;
  }
}
//----------------------------------------------------------------------------
void __fastcall TConvertToIRC::InitBoldUnderlineReverseForNextLine(void)
{
  PUSHSTRUCT NewState;

  // Get the initial text-state (before any printable chars) (Note, yes we
  // need the state at the beginning of the document not at the present
  // location in the document! This is all about setting underline, etc.
  // from the font-dialog!)
  int InsertIdx = utils->SetStateFlags(pBuf, iSize, STATE_MODE_FIRSTCHAR,
                                                                 NewState);

  if (InsertIdx >= 0)
  {
    // Unset bold, underline reverse if it is set in the font
    // and already set in the initial text-state
    // don't add Font-Dialog codes if text already has colors/codes...
    if (!bOrigTextHasUnderline && dts->cStyle.Contains(fsUnderline))
      State.bUnderline = !NewState.bUnderline;

    if (!bOrigTextHasBold && dts->cStyle.Contains(fsBold))
      State.bBold = !NewState.bBold;

    if (!bOrigTextHasItalics && dts->cStyle.Contains(fsItalic))
      State.bItalics = !NewState.bItalics;
  }
}
//----------------------------------------------------------------------------
void __fastcall TConvertToIRC::ProperlyTerminateEffects(void)
// This is called at the beginning when forming a new text-line, prior to
// adding any wings, padding, margins or color-format strings, borders or
// font-codes.
{
  // Now this is tricky: we want to force bold, underline or reverse OFF if
  // this is the end of a line and we have not turned the effect off yet.
  // But, if the very next raw char in the buffer, as yet unprocessed,  was
  // intended to turn the effect off, and we carry it to the next line, the
  // RTF converter and any chat client will now take it as an effect ON,
  // inappropriately.
  //
  // So we want to skip over all succeeding effect-chars that are not separated
  // by a printable char.

  // Don't carry special effects into wings and padding (this will appear at the
  // END of the new text-line, BEFORE wings, padding, margin, border!
  if (State.bUnderline)
    TextLine += CTRL_U;
  if (State.bBold)
    TextLine += CTRL_B;
  if (State.bItalics)
    TextLine += CTRL_R;
}
//----------------------------------------------------------------------------
int __fastcall TConvertToIRC::AddScriptingChars(WideString ColorFormatStr)
// Add Wings to text-line (not borders!)
// Return text-length with wing-length
{
  int NextWordLength;

  // Get actual length of text minus special sequences
  (void)utils->GetRealLength(TextLine, NextWordLength);

  // Return background to default color if color-codes
  // are embedded in original text

  // Scan string and insert ColorFormatStr wherever a
  // CTRL_O is found AND ALSO at the end of the string
  // REPLACE the embedded CTRL_Os
  int Length = TextLine.Length();

  for (int ii = 1; ii <= Length; ii++)
  {
    if (TextLine[ii] == CTRL_O)
    {
      TextLine = utils->DeleteW(TextLine, ii, 1); // Delete CTRL_O
      TextLine = utils->InsertW(TextLine, ColorFormatStr, ii+1);
      Length = TextLine.Length(); // Length has increased
    }
  }

  if (dts->Borders->Checked || dts->Wingdings->Checked)
  {
    TextLine += ColorFormatStr; // Add color-format to end of text-string
    TextLine = utils->InsertW(TextLine, ColorFormatStr, 1); // Add color-format to beginning of text-string
  }

  if (bWingActive && dts->Wingdings->Checked) AddWingToText(ColorFormatStr);
  else TotalWingLength = 0;

  return TotalWingLength + NextWordLength;
}
//----------------------------------------------------------------------------
int __fastcall TConvertToIRC::LargestWingLength(void)
{
  if (!dts->Wingdings->Checked)
    return 0;

  int RWingLength, LWingLength, TWingLength, MWingLength;

  MWingLength = 0;

  int count = dts->LeftWings->Count;

  for (int ii = 0; ii < count; ii++)
  {
    if ((bool)dts->LeftWings->GetTag(ii)) // Enable flag set?
    {
      (void)utils->GetRealLength(dts->LeftWings->GetString(ii), LWingLength);
      (void)utils->GetRealLength(dts->RightWings->GetString(ii), RWingLength);

      // We will pad out the shorter wing for symmetry so use 2 * Longest wing to compute max length
      if (RWingLength > LWingLength)
        TWingLength = 2*RWingLength;
      else
        TWingLength = 2*LWingLength;

      if (TWingLength > MWingLength)
        MWingLength = TWingLength;
    }
  }

 return MWingLength;
}
//----------------------------------------------------------------------------
int __fastcall TConvertToIRC::SelectWings(int NextWordLen)
// Returns actual length of both wings
// NextWordLen is an optional parameter and if set > 0
// we choose wings that when added to NextWordLen will fit into LineWidth.
{
  if (dts->Wingdings->Checked && LineWidth > 0)
  {
    int ii, TotalWingLen;

    int lCount = dts->LeftWings->Count;
    int rCount = dts->RightWings->Count;

    int Index = random(lCount);

    // EDIT: Looks better if we pad all the time?
    // Add padding to balance wings if we have top/bottom borders or if there is more than one line of text or manual line-width is set...
    //bool bAddWingPadding = (dts->Borders->Checked && dts->bTopEnabled) || OrigLineCount > 1 || !dts->AutoWidth->Checked;
    WingPadding = "";

    for (ii = 0; ii < lCount; ii++)
    {
      LeftWingStr = utils->StripCRLF(dts->LeftWings->GetString(Index));

      if (Index < rCount)
        RightWingStr = utils->StripCRLF(dts->RightWings->GetString(Index));

      bWingActive = (bool)dts->LeftWings->GetTag(Index);

      int LeftLen, RightLen;
      bLeftHasColor = utils->GetRealLength(LeftWingStr, LeftLen);
      bRightHasColor = utils->GetRealLength(RightWingStr, RightLen);

      TotalWingLen = LeftLen + RightLen;

      // How wing-padding works: we have a string, WingPadding and a flag, bPadLeftWing. If one wing's real length
      // is different than the other's we will need to pad with spaces either to the left side of the left wing
      // before adding its colors or to the right side of the right wing after restoring normal document colors...
      // So below we set the flag and padding string and actually use them in AddWingToText()

      //int diff = 0;

      //if (bAddWingPadding)
      //{
        // Get the length difference between left and right wings (we will end up padding one side
        // or the other to compensate
        int diff = LeftLen - RightLen;

        if (diff < 0)
        {
          bPadLeftWing = true;
          diff = -diff;
        }
        else
          bPadLeftWing = false;

        if (diff > 0)
          TotalWingLen += diff;
      //}

      // Look for wing-set that will fit
      if (bWingActive && TotalWingLen + NextWordLen <= LineWidth)
      {
        if (diff > 0)
          WingPadding = utils->StringOfCharW(C_SPACE, diff);

        return TotalWingLen;
      }

      if (++Index >= dts->TotalWings)
        Index = 0;
    }
  }

  bWingActive = false;
  bLeftHasColor = false;
  bRightHasColor = false;
  LeftWingStr = "";
  RightWingStr = "";

  return 0;
}
//----------------------------------------------------------------------------
void __fastcall TConvertToIRC::AddWingToText(WideString ColorFormatStr)
{
  // Put wing-color in TempStr
  WideString TempStr;
  utils->WriteSingle(dts->Awc, TempStr, true);

  WideString sL = LeftWingStr;
  WideString sR = RightWingStr;

  // Insert main wing color-code if user's wing has no pre-existing colors.
  if (!bLeftHasColor)
    sL = utils->InsertW(sL, TempStr,1);

  // Pad in front of left wing to balance right-wing if necessary
  if (bPadLeftWing && !WingPadding.IsEmpty())
    sL = utils->InsertW(sL, WingPadding, 1);

  // back to main foreground/background color
  sL += ColorFormatStr;

  // Insert main wing color-code if user's wing has no pre-existing colors.
  if (!bRightHasColor)
    sR = utils->InsertW(sR, TempStr, 1);

  // back to main foreground/background color
  sR += ColorFormatStr;

  // Pad after right wing to balance left-wing if necessary
  if (!WingPadding.IsEmpty() && !bPadLeftWing) sR += WingPadding;

  // add trailing wing
  TextLine += sR;

  // add leading wing
  TextLine = utils->InsertW(TextLine, sL, 1);
}
//---------------------------------------------------------------------------
void __fastcall TConvertToIRC::InitPushPop(void)
// Call this before processing to clear the
// Push/Pop memory
{
  if (PUSHSTRUCTS)
  {
    PushIdx = 0;
    delete [] PUSHSTRUCTS;
    PUSHSTRUCTS = NULL;
  }
}
//---------------------------------------------------------------------------
void __fastcall TConvertToIRC::Push(PUSHSTRUCT p)
{
  if (!PUSHSTRUCTS)
  {
    PUSHSTRUCTS = new PUSHSTRUCT[MAXPUSH];
    PushIdx = 0;
  }

  if (PushIdx < MAXPUSH)
  {
    PUSHSTRUCTS[PushIdx].fg = p.fg;
    PUSHSTRUCTS[PushIdx].bg = p.bg;
    PUSHSTRUCTS[PushIdx].fontSize = p.fontSize;
    PUSHSTRUCTS[PushIdx].fontType = p.fontType;
    PUSHSTRUCTS[PushIdx].bBold = p.bBold;
    PUSHSTRUCTS[PushIdx].bUnderline = p.bUnderline;
    PUSHSTRUCTS[PushIdx].bItalics = p.bItalics;
    PUSHSTRUCTS[PushIdx].bFGfast = p.bFGfast;
    PUSHSTRUCTS[PushIdx].bBGfast = p.bBGfast;
    PUSHSTRUCTS[PushIdx].bFontSizefast = p.bFontSizefast;
    PUSHSTRUCTS[PushIdx].bFontTypefast = p.bFontTypefast;
    PushIdx++;
  }
}
//---------------------------------------------------------------------------
void __fastcall TConvertToIRC::Pop(PUSHSTRUCT &p)
{
  if (!PUSHSTRUCTS || PushIdx == 0) return;

  PushIdx--;
  p.fg = PUSHSTRUCTS[PushIdx].fg;
  p.bg = PUSHSTRUCTS[PushIdx].bg;
  p.fontSize = PUSHSTRUCTS[PushIdx].fontSize;
  p.fontType = PUSHSTRUCTS[PushIdx].fontType;
  p.bBold = PUSHSTRUCTS[PushIdx].bBold;
  p.bUnderline = PUSHSTRUCTS[PushIdx].bUnderline;
  p.bItalics = PUSHSTRUCTS[PushIdx].bItalics;
  p.bFGfast = PUSHSTRUCTS[PushIdx].bFGfast;
  p.bBGfast = PUSHSTRUCTS[PushIdx].bBGfast;
  p.bFontSizefast = PUSHSTRUCTS[PushIdx].bFontSizefast;
  p.bFontTypefast = PUSHSTRUCTS[PushIdx].bFontTypefast;

  if (!PushIdx)
  {
    delete [] PUSHSTRUCTS;
    PUSHSTRUCTS = NULL;
  }
}
//---------------------------------------------------------------------------
void __fastcall TConvertToIRC::SetLineWidth(int Lmargin, int Rmargin, int Indent)
// LineWidth is desired width of text without Side borders or margins
// If we need to increase the line to fit the top (iterated segments
// of a fixed length) the easiest way is to actually increase the LWidth
// parameter (in the edit box).
{
  int TempLineWidth = LineWidth;

  if (TempLineWidth < MINLINESIZE)
    TempLineWidth = MINLINESIZE;

  int Temp = Lmargin + Rmargin; // Add Left/Right margins

  // Reverse-indent for numbered-list? Have to add it
  // (all lines except the 1st line of each paragraph are indented
  // by Lmargin + (-Indent) when Indent < 0).
  if (bIndentEnabled && Indent < 0)
    Temp += -Indent;

  int OldWidth = TempLineWidth + Temp + GetSideBorderTotalWidth();

  int NewWidth = OldWidth;

  // Find out if we need to pad LineWidth out to fit an even multiple of
  // top segments...
  AdjustMultiplesOfTopBorderSegment(NewWidth);

  if (NewWidth > OldWidth)
  {
    LineWidth = TempLineWidth + (NewWidth-OldWidth);

    if (bIndentEnabled && Indent < 0)
      LineWidth += -Indent;

    // Show the auto-value we have set (supress on-change event)
    dts->WidthEdit->OnChange = NULL;
    dts->WidthEdit->Color = clYellow;
    dts->WidthEdit->Text = String(LineWidth);
    dts->WidthEdit->OnChange = dts->WidthEditEnter;
  }
}
//---------------------------------------------------------------------------
void __fastcall TConvertToIRC::AdjustMultiplesOfTopBorderSegment(int &TotalWidth)
// Given total length of a line including borders and margins,
// returns the higher total needed to get even top/bottom borders
{
  if (dts->bTopEnabled)
  {
    if (dts->TopBorder.IsEmpty())
    {
      dts->bTopEnabled = false;
      TopLength = 0;
    }
    else
    {
      utils->GetRealLength(dts->TopBorder, TopLength);

      if (TopLength > TotalWidth)
        TotalWidth = TopLength;
      else
      {
        int TempInt = (TotalWidth % TopLength);

        // Need to make LWidth larger?
        if (TempInt)
          TotalWidth += TopLength - TempInt;
      }
    }
  }
  else
    TopLength = 0;
}
//---------------------------------------------------------------------------
int __fastcall TConvertToIRC::GetSideBorderTotalWidth(void)
{
  int LeftSide = 0;
  int RightSide = 0;

  if (dts->Borders->Checked && dts->bSideEnabled)
  {
    if (!dts->LeftSideBorder.IsEmpty())
      utils->GetRealLength(dts->LeftSideBorder, LeftSide);

    if (!dts->RightSideBorder.IsEmpty())
      utils->GetRealLength(dts->RightSideBorder, RightSide);
  }

  return LeftSide + RightSide;
}
//---------------------------------------------------------------------------

