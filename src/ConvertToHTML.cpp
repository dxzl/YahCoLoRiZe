//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

#include "DlgWebExport.h"
#include "FormSFDlg.h"
#include "LicenseKey.h"
#include "ConvertToHTML.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TConvertToHTML::TConvertToHTML(TComponent* Owner, WideString fName)
{
  this->dts = static_cast<TDTSColor*>(Owner);

  // fName in the constructor-argument defaults to an empty string.
  // Set it to what you want for the FileName. If it's left as an empty
  // string and bCreateDialog is set in Execute() a file-dialog will be
  // invoked to let the user select a file to write the webpage to.
  //
  // Set the bClip property to write to the clipboard in HTML format rather
  // than to a file... Clear the bClip property to write to a file.
  // bClip defaults to false.
  //
  // Set bWriteClipHtml to write HTML to the clipboard
  // Set bWriteClipPlain to write HTML to the clipboard
  // (they default true!)

  FFile = fName;

  // these three go together - if clip is false the other two are don't cares
  clip = false; // This can be overriden by the bClip property!
  saveplain = true; // bWriteClipPlain property
  savehtml = true; // bWriteClipHtml property

  showstatus = true; // show a status panel and progress bar

  savestream = false; // bWriteStream property (set to write to SL_HTM)

  // Old MS Word and browsers need non-breaking spaces to pad trailing
  // white-space. The white-space style in the PRE tag won't work at times.
  //
  // I guess we need to default non-breaking spaces ON if we are copying
  // to the clipboard and off if writing to a file...(???)
  //
  // Office 2007 will paste HTML instead of RTF if both are on the
  // clipboard... and its HTML decoder does NOT read the white-space style...
  //
  // Set to true for Office 2007 and earlier (I don't have the new MS Word
  // so I'm presuming everything's fixed and we don't need this...). I'm
  // setting it anyway, for now - when writing HTML to the clipboard.
  nonbreakingspaces = false;

  cbHTML = (unsigned short)RegisterClipboardFormat(HTML_REGISTERED_NAME);
  HTMLCHARSLEN = wcslen(HTMLCHARS); // do this once for speed!

  // Init global private vars used for clipboard-formated HTML
  this->GHTMLStart = -1;
  this->GHTMLEnd = -1;
  this->GFragStart = -1;
  this->GFragEnd = -1;
}

__fastcall TConvertToHTML::~TConvertToHTML(void)
{
}
//---------------------------------------------------------------------------
// PUBLIC!
int __fastcall TConvertToHTML::Convert(TStringsW* sl, bool bCreateDialog)
// bCreateDialog defaults true
{
  return Convert(sl->Text, bCreateDialog);
}

// PUBLIC!
int __fastcall TConvertToHTML::Convert(WideString S, bool bCreateDialog)
// bCreateDialog defaults true
{
  try
  {
    try
    {
      bool bBgImage, bBgFixed, bAuthor, bTitle, bHome;

      if (bCreateDialog)
      {
        // Get HTML margins
        Application->CreateForm(__classid(TWebExportForm), &WebExportForm);

        // Properties are all initialized from main form
        int Ret = WebExportForm->ShowModal();

        // Set some things even if user canceled! (applicable to the
        // clipboard)
        dts->WebPageLineHeight = WebExportForm->LineHeight;
        dts->WebPageWhiteSpaceStyle = WebExportForm->WhiteSpaceStyle;

        if (Ret == mrCancel)
        {
          WebExportForm->Release();
          return 1; // User canceled?
        }

        dts->WebPageLeftMargin = WebExportForm->LeftMargin;
        dts->WebPageTopMargin = WebExportForm->TopMargin;
        dts->WebPageWhiteSpaceStyle = WebExportForm->WhiteSpaceStyle;
        dts->WebPageBgImageStyle = WebExportForm->BgImageStyle;
        dts->WebPageBgColor = WebExportForm->BgColor;
        dts->WebPageBgImageUrl = WebExportForm->BgImageUrl;
        dts->WebPageHomeButtonUrl = WebExportForm->HomeButtonUrl;
        dts->WebPageTitle = WebExportForm->WebPageTitle;
        dts->WebPageAuthor = WebExportForm->WebPageAuthor;

        dts->WebPage_bNonBreakingSpaces = WebExportForm->bNonBreakingSpaces;
        dts->WebPage_bBgImage = WebExportForm->bBgImage;
        dts->WebPage_bBgFixed = WebExportForm->bBgFixed;
        dts->WebPage_bAuthor = WebExportForm->bAuthor;
        dts->WebPage_bTitle = WebExportForm->bTitle;
        dts->WebPage_bHome = WebExportForm->bHome;
        clip = WebExportForm->bClip;

        WebExportForm->Release();

        bBgImage = dts->WebPage_bBgImage;
        bBgFixed = dts->WebPage_bBgFixed;
        bAuthor = dts->WebPage_bAuthor;
        bTitle = dts->WebPage_bTitle;
        bHome = dts->WebPage_bHome;
      }
      else // no dialog
      {
        bBgImage = false; // dts->WebPage_bBgImage;
        bBgFixed = false; // dts->WebPage_bBgFixed;
        bAuthor = false; // dts->WebPage_bAuthor;
        bTitle = false; // dts->WebPage_bTitle;
        bHome = false; // dts->WebPage_bHome;
      }

      // Set the local vars
      leftMargin = dts->WebPageLeftMargin;
      topMargin = dts->WebPageTopMargin;
      whiteSpaceStyle = dts->WebPageWhiteSpaceStyle;
      bgImageStyle = dts->WebPageBgImageStyle;
      bgColor = dts->WebPageBgColor;
      bgImageUrl = dts->WebPageBgImageUrl;
      homeButtonUrl = dts->WebPageHomeButtonUrl;
      title = dts->WebPageTitle;
      author = dts->WebPageAuthor;
      bNonBreakingSpaces = dts->WebPage_bNonBreakingSpaces;

      String sWhiteSpace;

      switch(whiteSpaceStyle)
      {
        case WS_NORMAL:
          sWhiteSpace = "normal";
          break;
        case WS_NOWRAP:
          sWhiteSpace = "no-wrap";
          break;
        case WS_PRE:
          sWhiteSpace = "pre";
          break;
        case WS_PREWRAP:
          sWhiteSpace = "pre-wrap";
          break;
        case WS_PRELINE:
          sWhiteSpace = "pre-line";
          break;
        case WS_INHERIT:
          sWhiteSpace = "inherit";
          break;
        case WS_INITIAL:
          sWhiteSpace = "initial";
          break;
        case WS_UNSET:
          sWhiteSpace = "unset";
          break;
      };

      String ColorBg;
      String sUtf8;

      if (!clip)
      {
        if (bCreateDialog)
        {
          if (bgColor == IRCTRANSPARENT)
            ColorBg = "#00FFFFFF";
          else if (bgColor <= 0)
            ColorBg = "#" + IntToHex(-bgColor, 6);
          else if (bgColor <= dts->PaletteSize && dts->Palette != NULL)
            ColorBg = "#" + IntToHex(dts->Palette[bgColor-1], 6);
          else
            ColorBg = "#000000";
        }
        else
          ColorBg = "#FFFFFF"; // default white bg

        if (FFile.IsEmpty())
        {
          if (bCreateDialog)
          {
            WideString wFilter = WideString("All files (*.*)|*.*|"
                                    "HTM files (*.htm)|*.htm|"
                                    "HTML files (*.html)|*.html");

            FFile = utils->GetSaveFileName(wFilter, L"NewPage.htm", dts->DeskDir,
                                                            String(DS[39]));
          }
          else
            FFile = utils->GetTempFileW(FN[25]); // "colorize.htm"

          if (FFile.IsEmpty())
            return 1; // Report that user cancelled (empty file-name)
        }

        // Add the HTML header
        sUtf8 = String(STARTHTML) + String("<head>\r\n");

        if (bTitle && !title.IsEmpty())
          sUtf8 += String("<title>") + title + String("</title>\r\n");

        sUtf8 +=
          "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=" +
                                                String(CHARSET) + "\"/>\r\n";

        String sRev = String(REVISION) + "." + String(DEF_PRODUCT_ID) +
                                              "." + String(DEF_SUPER_REV);
        sUtf8 += "<meta name=\"generator\" content=\"" + sRev + "\"/>\r\n";
//        sUtf8 += "<meta name=\"generator\" content=\"" +
//                                  String(HTML_GENERATOR) + "\"/>\r\n";

        if (bAuthor && !author.IsEmpty())
          sUtf8 += String("<meta content=" + String("\"") + author + String("\"") +
            " name=\"AUTHOR\"/>\r\n");

        sUtf8 += String("</head>\r\n\r\n");

        // Add the body-tag
        sUtf8 += String("<body");

        // include all the time in-case no image!
        sUtf8 += String(" bgcolor=\"") + ColorBg + String("\"");

        if (bBgImage)
        {
          sUtf8 += String(" background=\"") + bgImageUrl + String("\"");

          // REPEAT_ON  0
          // REPEAT_X   1
          // REPEAT_Y   2
          // REPEAT_OFF 3
          switch(bgImageStyle)
          {
            case REPEAT_ON:
              sUtf8 += String(" style=\"background-repeat:repeat\"");
              break;

            case REPEAT_X:
              sUtf8 += String(" style=\"background-repeat:repeat-x\"");
              break;

            case REPEAT_Y:
              sUtf8 += String(" style=\"background-repeat:repeat-y\"");
              break;

            case REPEAT_OFF:
              sUtf8 += String(" style=\"background-repeat:no-repeat\"");
              break;

            default:
              break;
          };

          if (bBgFixed)
            sUtf8 += String(" bgproperties=\"fixed\"");
        }

        sUtf8 += String(" leftmargin=\"") + leftMargin + String("\"");
        sUtf8 += String(" topmargin=\"") + topMargin + String("\"");

        sUtf8 += String(">\r\n");

        // Add the HOME button and web-site for "home"
        if (bHome)
        {
    //      String ButtonColor = "rgb(#ffffff)";
          String ButtonFontSize = BUTTON_FONT_SIZE;
          sUtf8 += String("<p style=\"font-size: ") + ButtonFontSize + String(";\"");
    //      sUtf8 += String(" color: ") + ButtonColor + String(";\"");
          sUtf8 += String("><a href=\"") + homeButtonUrl + String("\"");
          sUtf8 += String(">" + String(HOME_BUTTON_TEXT) + "</a></p>");
        }

        sUtf8 += String("\r\n");
      }
      else
      {
        // Start of HTML marker
        this->GHTMLStart = String(FRAGMENTHEADER).Length();

        sUtf8 = String(STARTHTML) + String("<body>\r\n") + String(STARTFRAGMENT);

        // Start of fragment marker
        this->GFragStart = this->GHTMLStart + sUtf8.Length();
      }

      // put this after the dialogs close!
      if (this->bShowStatus)
        dts->CpShow(STATUS[10], DS[70]); // show status an CancelPanel

      // Add PRE tag. If HtmlLineHeight is set to 0 by the user, we don't
      // include line-height (note: white-space: pre-line; is the trick to get
      // Microsoft HTML to preserve the \r\n sequences. Without it you can
      // paste from YC into MS Mail ok but then when you copy the pasted text
      // from MS Mail back to YC, there are no line breaks!)
      sUtf8 += "<pre style=\"font-family: \'" + utils->WideToUtf8(dts->cFont) +
              "\', monospace; font-size: " + String(dts->cSize) + "pt;";

      // if dts->WebPageLineHeight == 0 the value is 'normal' (no need to add that)
      if (dts->WebPageLineHeight < 0)
        // use line-height: inherit if HtmlLineHeight is -10 (or  < 0)
        sUtf8 += " line-height: inherit;";
      else if (dts->WebPageLineHeight > 0)
      {
        // Compute a line-height in "em"
        String SHeight = Format("%-.2f",
                  ARRAYOFCONST(((float)dts->WebPageLineHeight/100.0))) + "em";
        sUtf8 += " line-height: " +  SHeight + ";";
      }

      if (sWhiteSpace != "normal")
        sUtf8 += " white-space: " + sWhiteSpace + ";";

      // terminate the tag
      sUtf8 += "\">";

      // For each line...
      WideString wLine;

      int len = S.Length();

      while (GetAsyncKeyState(VK_ESCAPE) < 0); // Dummy read to clear...

      dts->CpSetMaxIterations(len); // set max iterator value for progress bars

      for (int ii = 0; ii < len; ii++)
      {
        if (utils->FoundCRLF(S, ii))
        {
          // User abort???
          Application->ProcessMessages();
          if ((int)GetAsyncKeyState(VK_ESCAPE) < 0) break; // Cancel

          // NOTE: don't use <br /> tags because after you paste content
          // generated with them into Microsoft Mail then select/copy it
          // in the mail-client, the background color is not properly
          // carried over (for text with trailing spaces but no border)
          // It's Microsoft's problem...
          //
          // The problem WITH using \r\n instead of <br /> is in copying
          // our HTML back from IE to Mail or YC... IE puts the PRE tag in front
          // of the StartFragment tag and so there are no line-breaks when
          // you paste it. All the lines run together - this is only in IE, not
          // Firefox or Chrome...
          //
          // Also tried using the "white-space: pre" style parameter to no avail.
          sUtf8 += ConvertLineToHTML(wLine) + CRLF;
          wLine = "";

          // Advance progress-bar
          dts->CpUpdate(ii);

          continue;
        }

        wLine += S[ii+1];
      }

      // Add last line (no cr/lf at end!)
      if (wLine.Length() > 0)
        sUtf8 += ConvertLineToHTML(wLine);

      sUtf8 += "</pre>";

      // End of font-span and <pre>
      if (clip)
        sUtf8 += ENDFRAGMENT;

      sUtf8 += ENDHTML;

      // Write to Main Program's HTML stream
      if (savestream && dts->SL_HTM != NULL)
        dts->SL_HTM->Text = sUtf8;

      if (!clip)
        utils->WriteStringToFileW(FFile, sUtf8); // Write to file
      else
      {
        // End of HTML marker
        this->GHTMLEnd = this->GHTMLStart + sUtf8.Length();

        // End of fragment marker
        // Note: we do this here so the entire string with end-tags can be UTF-8
        // encoded first (see above), then we subtract off the length of the
        // trailing tags (since none of the chars is more than one byte).
        this->GFragEnd = this->GHTMLEnd - (String(ENDFRAGMENT).Length() +
                                                    String(ENDHTML).Length());


        // Copy CF_UNICODETEXT to clipboard...
        if (saveplain)
          utils->CopyTextToClipboard(S);

        // Copy CF_HTML to clipboard...
        if (savehtml)
          CopyHtmlToClipBoard(sUtf8);
      }
    }
    catch(...) {}
  }
  __finally
  {
    dts->CpHide();
  }

  return 0; // Success!
}
//---------------------------------------------------------------------------
// PUBLIC!
String __fastcall TConvertToHTML::ConvertLineToHTML(WideString sIn)
{
  try
  {
// (problem is - that this was blocking page-breaks!)
    // Look for "effectively" empty strings...
//    int ESize = utils->GetRealLength(sIn);
//    if (ESize == 0)
//      return "";

    // convert \pagebreak into C_FF for processing
    sIn = utils->PageBreaksToFormFeed(sIn);

    int size = sIn.Length();

    // Clear flags
    bSourceBold = bSourceUnderline = bSourceItalics =
      bWroteFirstBold = bWroteFirstUnderline = bWroteFirstItalics =
        bDestBold = bDestUnderline = bDestItalics = false;

    int PrevFG = NO_COLOR, PrevBG = NO_COLOR;

    // Span counter
    Spans = 0;

    // Main output string
    wOut = "";

    // For each char...
    for (int jj = 1 ; jj <= size ; jj++)
    {
      wchar_t c = sIn[jj];

      if (c == CTRL_B)
      {
        bSourceBold = !bSourceBold;
        continue;
      }

      if (c == CTRL_U)
      {
        bSourceUnderline = !bSourceUnderline;
        continue;
      }

      if (c == CTRL_R)
      {
        bSourceItalics = !bSourceItalics;
        continue;
      }

      if (c == C_FF) // page-break
      {
        wOut += "<div style=\"page-break-before: always;\"/>";
        continue;
      }

      if (c == C_TAB) // tab
      {
        wOut += utils->GetTabStringW(dts->RegTabs);
        continue;
      }

      if (c == CTRL_PUSH)
      {
        continue;
      }

      if (c == CTRL_POP)
      {
        continue;
      }

      if (c == CTRL_O)
      {
        if (c == CTRL_B)
          bSourceBold = false;

        if (c == CTRL_U)
          bSourceUnderline = false;

        if (c == CTRL_R)
          bSourceItalics = false;

        continue;
      }

      if (c == CTRL_F)
      {
        int ft = utils->CountFontSequence(sIn.c_bstr(), jj-1, size);

        if (ft >= 0)
        {
          // <span style="font-family: 'Courier New'">
          int idx;

          if (ft == 0)
            idx = dts->cType;
          else
            idx = ft;

          String sTemp = utils->GetLocalFontString(idx); // get as utf-8

          if (!sTemp.IsEmpty())
            WriteSpan("<span style=\"font-family: \'" + sTemp + "\';\">");

          jj += 2;
        }

        continue;
      }

      if (c == CTRL_S)
      {
        int fs = utils->CountFontSequence(sIn.c_bstr(), jj-1, size);

        if (fs >= 0)
        {
          // <span style='font-size: 9'>
          int idx;

          if (fs == 0)
            idx = dts->cSize;
          else
            idx = fs;

          if (idx > 0)
            WriteSpan("<span style=\"font-size: " + String(idx) + "pt;\">");

          jj += 2;
        }

        continue;
      }

      if (c == CTRL_K)
      {
        int fg = NO_COLOR, bg = NO_COLOR;

        jj += utils->CountColorSequence(sIn.c_bstr(), jj-1, size, fg, bg);

        bool bFgNoColor = (fg == NO_COLOR || fg == IRCNOCOLOR);
        bool bBgNoColor = (bg == NO_COLOR || bg == IRCNOCOLOR);

        if (!bFgNoColor && !bBgNoColor)
        {
          if (fg != PrevFG && bg != PrevBG)
          {
            WriteSpan("<span style=\"color: " + FormatColor(fg) +
                " background-color: " + FormatColor(bg) + "\">");

            PrevFG = fg;
            PrevBG = bg;
          }
          else if (fg != PrevFG)
          {
            WriteSpan("<span style=\"color: " + FormatColor(fg) + "\">");
            PrevFG = fg;
          }
          else if (bg != PrevBG)
          {
            WriteSpan("<span style=\"background-color: " + FormatColor(bg) + "\">");
            PrevBG = bg;
          }
        }
        else if (!bFgNoColor)
        {
          if (fg != PrevFG)
          {
            WriteSpan("<span style=\"color: " + FormatColor(fg) + "\">");
            PrevFG = fg;
          }
        }
        else if (!bBgNoColor)
        {
          if (bg != PrevBG)
          {
            WriteSpan("<span style=\"background-color: " + FormatColor(bg) + "\">");
            PrevBG = bg;
          }
        }

        continue;
      }

      if (bSourceBold)
      {
        if (!bWroteFirstBold) // Effect is on in source-IRC codes but not in HTML yet...
        {
          wOut += "<b>";
          bWroteFirstBold = true;
          bDestBold = true;
        }
      }
      else if (bDestBold) // Effect is on in HTML but off in source-IRC codes
      {
        wOut += "</b>";
        bWroteFirstBold = false;
        bDestBold = false;
      }

      if (bSourceUnderline)
      {
        if (!bWroteFirstUnderline) // Effect is on in source-IRC codes but not in HTML yet...
        {
          wOut += "<u>";
          bWroteFirstUnderline = true;
          bDestUnderline = true;
        }
      }
      else if (bDestUnderline) // Effect is on in HTML but off in source-IRC codes
      {
        wOut += "</u>";
        bWroteFirstUnderline = false;
        bDestUnderline = false;
      }

      if (bSourceItalics)
      {
        if (!bWroteFirstItalics) // Effect is on in source-IRC codes but not in HTML yet...
        {
          wOut += "<i>";
          bWroteFirstItalics = true;
          bDestItalics = true;
        }
      }
      else if (bDestItalics) // Effect is on in HTML but off in source-IRC codes
      {
        wOut += "</i>";
        bWroteFirstItalics = false;
        bDestItalics = false;
      }

      // Well, you would think in a <pre> tag you don't need &nbsp;
      // but you do for old MS Word at least...
      if (iswalnum(c) || c == ' ')
        wOut += c;
      else
        wOut += HtmlSpecialCharEncode(c);
    }

    if (clip || nonbreakingspaces)
    {
      // Scroll back and insert non-breaking spaces up to end of last tag
      int spaces = 0;

      // First count them
      for (int ii = wOut.Length(); ii > 0; ii--)
      {
        if (wOut[ii] != ' ')
          break;

        spaces++;
      }

      // Now replace them with &nbsp;
      if (spaces)
      {
        wOut.SetLength(wOut.Length()-spaces);

        for (int ii = 0; ii < spaces; ii++)
          wOut += NON_BREAKING_SPACE;
      }
    }

    if (bDestItalics)
      wOut += "</i>";

    if (bDestBold)
      wOut += "</b>";

    if (bDestUnderline)
      wOut += "</u>";

    // Unwind spans
    while (Spans--)
      wOut += "</span>";

    return utils->WideToUtf8(wOut); // Return this line converted to HTML
  }
  catch(...)
  {
    return "";
  }
}
//---------------------------------------------------------------------------
//void __fastcall TConvertToHTML::DoStyleSpan(int &Spans, String &sOut, char c)
//{
//  if (Spans > 0)
//  {
//   sOut += "</span>"; // off
//    Spans--;
//  }
//  else
//  {
//    String sStyle;
//    if (c == CTRL_U) sStyle = "text-decoration: underline"; // underline
//    else if (c == CTRL_R) sStyle = "font-style: italic"; // italic
//    else sStyle = "font-weight: bold"; // bold
//
//    sOut += "<span style=\"" + sStyle + ";\">"; // on
//    Spans++;
//  }
//}
//---------------------------------------------------------------------------
void __fastcall TConvertToHTML::WriteSpan(WideString wIn)
{
  // Turn effect off in inverse order...
  if (bDestItalics && bWroteFirstItalics)
  {
    wOut += "</i>";
    bWroteFirstItalics = false;
  }

  if (bDestBold && bWroteFirstBold)
  {
    wOut += "</b>";
    bWroteFirstBold = false;
  }

  if (bDestUnderline && bWroteFirstUnderline)
  {
    wOut += "</u>";
    bWroteFirstUnderline = false;
  }

  wOut += wIn;
  Spans++;
}
//---------------------------------------------------------------------------
String __fastcall TConvertToHTML::FormatColor(int c)
// Takes a color in our internal integer format (<= 0 is an rgb color
// and 1-PaletteSize is an IRC palette color) and converts it to rgb(r, g, b)
// (the way chrome puts colors on the clipboard!)
{
  // For transparent, have to set the bg color transparent with
  // z-index:1 and position: absolute
  if (c == IRCTRANSPARENT)
    return "transparent; position: absolute; z-index: inherit;";

  BlendColor bc = utils->YcToBlendColor(c);
  return "rgb(" + String(bc.red) + ", " + String(bc.green) + ", " +
                                                    String(bc.blue) + ");";
}
//---------------------------------------------------------------------------
WideString __fastcall TConvertToHTML::HtmlSpecialCharEncode(wchar_t c)
// Encode HTML special chars. Takes c and, if found, it
// uses the index of c in HTMLCHARS[] as the index of our
// desired replacement string in HTMLCODES[] and returns the
// replacement. If c is not in the table we return String(c);
{
  for (int ii = 0; ii < HTMLCHARSLEN; ii++)
    if (HTMLCHARS[ii] == c)
      return WideString(String(HTMLCODES[ii]));
  return WideString(c);
}
//---------------------------------------------------------------------------
void __fastcall TConvertToHTML::CopyHtmlToClipBoard(String htmlStr)
// We put three memory handles on the clipboard, two for
// CF_UNICODETEXT/CF_TEXT and one for CF_HTML (which we must register...)
{
  try
  {
    OpenClipboard(dts->Handle);

    char term[] = "\0\0\0\0"; // Terminator
    int termlen = 4;

    // Now set the CF_HTML handle...
    String str = FormatHeader(htmlStr);
    int htmlLen = str.Length();

    try
    {
      HGLOBAL hMemHtml = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE,
                                                          htmlLen+termlen);
      if (hMemHtml != NULL)
      {
        // Move string into global-memory
        char* lp = (char*)GlobalLock(hMemHtml);
        CopyMemory(lp, str.c_str(), htmlLen);
        CopyMemory(lp+htmlLen, &term, termlen);
        // Put the memory-handle on the clipboard
        // DO NOT FREE this memory!
        SetClipboardData(cbHTML, hMemHtml);
        GlobalUnlock(hMemHtml);
      }
    }
    catch(...) {}
  }
  __finally
  {
    CloseClipboard();
  }
}
//---------------------------------------------------------------------------
String __fastcall TConvertToHTML::FormatHeader(String HTMLText)
// StartFragment is byte count from beginning of clipboard to start of
// HTML fragment.
//
// EndFragment is byte count from beginning of clipboard to end of
// HTML fragment.
{
  TReplaceFlags rf; // TReplaceFlags() << rfIgnoreCase | rfReplaceAll

  String S = String(FRAGMENTHEADER);

  String Temp = utils->GoFormat("%8.8u" , this->GHTMLStart);
  S = StringReplace(S, "33333333", Temp, rf);

  // add length of "<!--StartFragment-->"
  Temp = utils->GoFormat("%8.8u", this->GFragStart);
  S = StringReplace(S, "11111111", Temp, rf);

  Temp = utils->GoFormat("%8.8u" , this->GHTMLEnd);
  S = StringReplace(S, "44444444", Temp, rf);

  // subtract the length of "<!--StartFragment-->" and "<!--EndFragment-->"
  Temp = utils->GoFormat("%8.8u" , this->GFragEnd);
  S = StringReplace(S, "22222222", Temp, rf);

  S += HTMLText;

  return S;
}
//---------------------------------------------------------------------------

