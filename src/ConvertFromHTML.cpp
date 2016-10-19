//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

//#include <string>
//#include <Rw\codecvt>
//#include <locale>

#include "ConvertFromHTML.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TConvertFromHTML::TConvertFromHTML(TComponent* Owner, WideString fName)
{
  this->dts = static_cast<TDTSColor*>(Owner);

  // Set FileName property to load HTML from a file
  // instead of clipboard!
  this->fileName = fName;

  cbHTML = (unsigned short)RegisterClipboardFormat(HTML_REGISTERED_NAME);
  HTMLCHARSLEN = wcslen(HTMLCHARS); // do this once for speed!
  bHaveURL = false;
  bPreOn = false;
  G_InLen = 0;
}

__fastcall TConvertFromHTML::~TConvertFromHTML(void)
{
}
//---------------------------------------------------------------------------
WideString __fastcall TConvertFromHTML::Execute(bool bShowStatus)
{
  WideString Temp;
  int pos;

  // Clear our global url member vars - we catch a url ("<a href" tag) and
  // wait to print it until we get the </a>
  bHaveURL = false;
  G_URL = "";

  // Show status "Loading string-list..."
  if (bShowStatus)
    dts->CpShow(STATUS[1], DS[70]);

  WideString OutText;

  try
  {
    try
    {
      // If no fileName - we presume the input data is UTF-8 Clipboard text...
      if (this->fileName.IsEmpty())
      {
        G_In = utils->Utf8ToWide(utils->LoadHtmlFromClipboard());

        if (G_In.IsEmpty())
        {
          ShowMessage("HTML Clipboard is empty!");
          return "";
        }

        // Get lower-case version to use globally
        G_InLC = utils->LowerCaseW(G_In);
        G_InLen = G_In.Length();

        // Get StartHTML:0000000176
        Temp = "starthtml:";

        pos = G_InLC.Pos(Temp);

        if (pos > 0)
        {
          pos += Temp.Length();

          Temp = "";

          for(; pos <= G_InLen; pos++)
          {
            if (G_In[pos] == ' ')
              continue; // ignore spaces

            if (!iswdigit(G_In[pos]))
              break; // quit on non-digit

            Temp += G_In[pos];
          }

          pos = utils->ToIntDefW(Temp, -1) + 1; // add one for Ansi WideString index
        }

        int startHTML = pos;

        // Get StartFragment:0000000176
        Temp = "startfragment:";

        pos = G_InLC.Pos(Temp);

        if (pos > 0)
        {
          pos += Temp.Length();

          Temp = "";

          for(; pos <= G_InLen; pos++)
          {
            if (G_In[pos] == ' ')
              continue; // ignore spaces

            if (!iswdigit(G_In[pos]))
              break; // quit on non-digit

            Temp += G_In[pos];
          }

          pos = utils->ToIntDefW(Temp, -1) + 1;
        }

        // Backup attempt to find start of HTML code to process
        if (pos <= 0)
        {
          Temp = "<!--startfragment-->";

          pos = G_InLC.Pos(Temp);

          if (pos <= 0)
          {
            utils->ShowMessageU("HTML Clipboard text is in the wrong format!");
            return "";
          }

          pos += Temp.Length();
        }

        int startFrag = pos;

        // Microsoft IE copy to clipboard puts the PRE tag in front of
        // the fragment... so let's find out if there is a PRE tag hiding up
        // there... If so we will just set our bPreOn flag.
        try
        {
          int len = startFrag-startHTML;

          if (len > 6)
          {
            Temp = G_InLC.SubString(startHTML, len);

            int prePos = Temp.Pos("<pre ");

            if (prePos > 0 && GetTag(prePos).Length() > 0 && Temp.Pos("</pre>") == 0)
              bPreOn = true;
          }
        }
        catch(...) { }
      }
      else
      {
        String sFile = utils->ReadStringFromFileW(fileName);

        if (dts->IsAnsiFile)
          sFile = utils->AnsiToUtf8(sFile); // convert an old ANSI file to Utf-8

        if (!GetBody(sFile))
        {
          utils->ShowMessageU("Unable to get HTML document body for:\r\n" + fileName);
          return "";
        }

        pos = 1;
      }

      WideString Tag;

      // Show status "Processing to IRC format......"
      if (bShowStatus)
        dts->CpShow(STATUS[3], DS[70]);

      dts->CpSetMaxIterations(G_InLen);

      while (GetAsyncKeyState(VK_ESCAPE) < 0); // Dummy read to clear...

      for(;;)
      {
        Tag = GetTag(pos, OutText);

        if (Tag.Length() == 0)
          break;

        // User abort???
        Application->ProcessMessages();
        if ((int)GetAsyncKeyState(VK_ESCAPE) < 0)
          break; // Cancel

        // Advance progress-bar
        dts->CpUpdate(pos);

        ProcessTag(Tag, OutText);
      }

      OutText = ReplaceSpecial(OutText); // Replace &lt; &quot; &gt;
    }
    catch(...) {}
  }
  __finally
  {
    dts->CpHide();
  }
  return OutText;
}
//---------------------------------------------------------------------------
bool __fastcall TConvertFromHTML::GetBody(String sRaw)
// Searches sRaw (from a file) and returns G_In set to the HTML body
// and converted as needed by the specified encoding.
// Returns true if no errors
{
  try
  {
    // Find the body start
    int start = sRaw.Pos("<body>");

    // Try again...
    if (start == 0)
      start = sRaw.Pos("<body ");

    if (start > 0)
    {
      String sRawLC = sRaw.LowerCase();

      // Get the text up to the body tag
      String sHead = sRawLC.SubString(1, start-1);

      // Search head for "charset  =   '" utf  -  8   "',;)"
      // Convert UTF-8 to ANSI if we need to
      if (sHead.Length() > 0)
      {
        String sTemp("charset");

        int pos = sHead.Pos(sTemp);

        if (pos > 0)
        {
          int idx = pos+sTemp.Length();
          sTemp = ParseArg(idx, sRawLC);

          if (sTemp == "utf-8")
            G_In = utils->Utf8ToWide(sRaw);
          else if (sTemp == "utf-16")
            G_In = WideString((wchar_t*)sRaw.c_str());
          else // ansi
            G_In = WideString(sRaw);
        }
        else
          G_In = WideString(sRaw);
      }
      else
        G_In = WideString(sRaw);

      G_InLC = utils->LowerCaseW(G_In);
      G_InLen = G_InLC.Length();

      int end = G_InLC.Pos("</body>");

      // Move past the BODY tag (adds to start...)
      //WideString BodyTag = GetTag(start); // Todo: process body-tag
      GetTag(start);

      if (end == 0)
        end = G_InLen+1; // no body tag so use whole document

      G_In = G_In.SubString(start, end-start);
    }
    else
      // No body - so just assume we have ANSI text in the file.
      // Put it in UTF-16 form...
      G_In = WideString(sRaw);

    G_InLC = utils->LowerCaseW(G_In);
    G_InLen = G_In.Length();

    if (G_In.Length() == 0)
      return false;

    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
WideString __fastcall TConvertFromHTML::GetTag(int &pos)
{
  WideString OutText; // Not used here, we just need the tag here

  return GetTag(pos, OutText);
}

WideString __fastcall TConvertFromHTML::GetTag(int &pos, WideString &OutText)
// Scans G_InLC for an HTML tag delimited by <>. Adds text outside of the tag
// to OutText. Returns the Tag. pos is the place within G_InLC to begin. We set
// pos to the next wchar_t at which to begin the next time this function is called.
{
  WideString Tag;
  bool bTagOn = false;
  wchar_t c;

  for (int ii = pos; ii <= G_InLen; ii++)
  {
    c = G_In[ii];

    if (c == '<')
    {
      if (bTagOn)
        OutText += Tag; // less-than sign so add text

      Tag = "";
      bTagOn = true;
    }
    else if (c == '>' && bTagOn)
    {
      // Return the tag
      Tag += ">";

      pos = ii + 1; // point index to next char

      return Tag;
    }

    // Add wchar_t either to tag or to OutText if it's not a cr/lf
//    if (!iswcntrl(c)) <= this is filtering out unicode!
    if (c >= ' ')
    {
      if (bTagOn)
        Tag += c;
      else
        OutText += c;
    }
    // Allow cr/lf to be added if we are in a "PRE" block
    else if (!bTagOn && bPreOn)
    {
      if (c == '\n')
        OutText += "\r\n"; // crlf
      // tab
      else if (c == '\t')
        OutText += TABSTRING; // guess 4 spaces :-)
    }
  }

  // Ended with text following "<" but no ">", assume it's a less-than sign
  // and add the accumulated text...
  if (bTagOn)
    OutText += Tag; // less-than sign so add text

  pos = G_InLen + 1; // set ref var to Ansi WideString index

  return ""; // no more tags
}
//---------------------------------------------------------------------------
bool __fastcall TConvertFromHTML::ProcessTag(WideString Tag, WideString &wOut)
// Process HTML Tag for color and font info and return an IRC equivalent
// string.
//
// *note: FontToWideString only writes font-codes for YahTrin() clients!!!
// returns false if an exception was thrown
//
// bPreOn is set when the <pre tag is detected and cleared on </pre>
{
  try
  {
    if (Tag.Length() < 3 || Tag[1] != '<' || Tag[Tag.Length()] != '>')
      return true;

    //ShowMessage(Tag);

    WideString tagLC = utils->LowerCaseW(Tag);

    int pos;
    WideString wTemp;

//(cFont has our font-string!)
//    wTemp = L"font-family:";
//    if ((pos = tagLC.Pos(wTemp)) > 0)
//    {
//      pos += wTemp.Length();
//      wTemp = GetArg(pos, S); // Return 'Courier New', etc.
//      if (!wTemp.IsEmpty())
//        wOut += WideString(CTRL_X) + wTemp;
//    }

    if (tagLC == WideString("<span class=\"apple-converted-space\">"))
    {
      wOut += " ";

      return true;
    }

    if (tagLC.Length() >= 10 && tagLC.SubString(1,3) == WideString("<a "))
    {
      try
      {
        int pos = tagLC.Pos("href");

        if (pos > 0)
        {
          G_URL = "";

          bHaveURL = false;
          bool bQ = false;

          for (int ii = pos+4; ii <= tagLC.Length(); ii++)
          {
            if (!bQ)
            {
              if (tagLC[ii] == '\"')
                bQ = true;
            }
            else
            {
              if (tagLC[ii] == '\"')
              {
                bHaveURL = true;
                break;
              }

              // build our url (use original tag case-sensitive!)
              G_URL += Tag[ii];
            }
          }
        }
      }
      catch(...)
      {
        G_URL = "";
        bHaveURL = false;
      }

      return true;
    }

    if (tagLC == WideString("</a>"))
    {
      try
      {
        // Print url if we have one
        if (bHaveURL && G_URL.Length() > 0)
          wOut += utils->FontTypeToString(dts->cType) +
              utils->FontSizeToString(dts->cSize) + " " + G_URL;
      }
      catch(...) {}

      G_URL = "";
      bHaveURL = false;

      return true;
    }

    if (tagLC == WideString("</pre>"))
    {
      bPreOn = false;
      return true;
    }

    if (tagLC == WideString("</p>"))
    {
      wOut += "\r\n";
      return true;
    }

    if (tagLC == WideString("<b>") || tagLC == WideString("</b>"))
    {
      wOut += CTRL_B;
      return true;
    }

    if (tagLC == WideString("<u>") || tagLC == WideString("</u>"))
    {
      wOut += CTRL_U;
      return true;
    }

    if (tagLC == WideString("<i>") || tagLC == WideString("</i>"))
    {
      wOut += CTRL_R;
      return true;
    }

    // here we check for any end-tag and just return for any of them...
    // do this AFTER checking all other specific "/" tags!
    if (tagLC.SubString(1,2) == WideString("</"))
      return true;

    bool bOpenTag = (tagLC.SubString(1,5) == WideString("<div "));

    if (bOpenTag || tagLC == WideString("<div>"))
    {
      if (wOut.Length() && wOut[wOut.Length()] != C_LF)
        wOut += CRLF; // Add break if no previous break

      if (!bOpenTag)
        return true;
    }

    bOpenTag = (tagLC == WideString("<br >"));

    if (bOpenTag || tagLC == WideString("<br>"))
    {
      wOut += CRLF;

      if (!bOpenTag)
        return true;
    }

    bOpenTag = (tagLC == WideString("<hr >"));

    if (bOpenTag || tagLC == WideString("<hr>"))
    {
      wOut += CRLF;

      if (!bOpenTag)
        return true;
    }

    bOpenTag = (tagLC.SubString(1,5) == WideString("<pre "));

    if (bOpenTag || tagLC == WideString("<pre>"))
    {
      bPreOn = true;

      if (!bOpenTag)
        return true;
    }

    bOpenTag = (tagLC.SubString(1,3) == WideString("<p "));

    if (bOpenTag || tagLC == WideString("<p>"))
    {
      // new paragraph we want two cr/lf pairs so look back
      int len = wOut.Length();
      int crlfcount = 2;

      if (wOut[len] == C_LF)
        crlfcount--;

      if (len >= 4 && wOut[len-2] == C_LF)
        crlfcount--;

      while (crlfcount--)
        wOut += "\r\n";

      if (!bOpenTag)
        return true;
    }

    // Parse out background-color: or background-color= and actually
    // remove it from tagLC so that it won't interfere with the subsequent
    // search for color: or color=. From that point on you can't use Tag
    // because it has not has background-color removed... so do anything
    // where upper/lower case matters before this point.
    int color;

    bool bWrite = false; // flag

    wTemp = "background-color";

    if ((pos = tagLC.Pos(wTemp)) > 0)
      bWrite = true;
    else
    {
      wTemp = "background";

      if ((pos = tagLC.Pos(wTemp)) > 0)
        bWrite = true;
    }

    int newPos;

    // Use a flag to consolidate code without a tedious function-call
    if (bWrite)
    {
      newPos = SkipColonOrEqual(pos+wTemp.Length(), tagLC);

      if (newPos)
      {
        // Return rgb(0,0,0) or #ACACAC (also returns the untrimmed start
        // of the next arg in newPos)
        wTemp = ParseArg(newPos, tagLC);

        if (!wTemp.IsEmpty() && (color = GetRgb(wTemp)) <= 0)
          utils->WriteSingle(color, wOut, false); // Convert to IRC bg

        // Remove "background-color   :  rgb(0,0,0)" or
        // "background=  #ACACAC" from tagLC
        // MUST do this because the search for "color" directly below will
        // re-find "background-color" unless we remove it.
        tagLC = tagLC.SubString(1, pos-1) +
                  tagLC.SubString(newPos, tagLC.Length()-newPos+1);
      }
    }

    wTemp = "color";

    if ((pos = tagLC.Pos(wTemp)) > 0)
    {
      // Discriminate between bg and fg color!
      if (pos == 1 || (tagLC[pos-1] != '-' && !iswalnum(tagLC[pos-1])))
      {
        newPos = SkipColonOrEqual(pos+wTemp.Length(), tagLC);

        if (newPos)
        {
          // Return rgb(0,0,0) or #ACACAC (also returns the untrimmed start
          // of the next arg in newPos)
          wTemp = ParseArg(newPos, tagLC);

          if (!wTemp.IsEmpty() && (color = GetRgb(wTemp)) <= 0)
            utils->WriteSingle(color, wOut, true); // Convert to IRC fg

          // Remove "color:rgb(0,0,0);" from tagLC (optional...)
          //tagLC = tagLC.SubString(1, pos-1) +
          //          tagLC.SubString(newPos, tagLC.Length()-newPos+1);
        }
      }
    }

    wTemp = "font-family";

    if ((pos = tagLC.Pos(wTemp)) > 0)
    {
      newPos = SkipColonOrEqual(pos+wTemp.Length(), tagLC);

      if (newPos)
      {
        wTemp = ParseArg(newPos, tagLC);

        // Get 1-based index into FONTS[] in DefaultWideStringd.cpp
        int idx = utils->GetLocalFontIndex(wTemp);

        // print the IRC font-family 1-based index-code
        if (idx >= 1)
          wOut += utils->FontTypeToString(idx);
      }
    }

    wTemp = "font-size";

    if ((pos = tagLC.Pos(wTemp)) > 0)
    {
      newPos = SkipColonOrEqual(pos+wTemp.Length(), tagLC);

      if (newPos)
      {
        wTemp = ParseArg(newPos, tagLC); // Return 20pt, 20%, 20cm, 20px,

        if (!wTemp.IsEmpty())
          HtmlToIrcFontSize(wTemp, wOut);
      }
    }

    wTemp = "page-break-before";

    if ((pos = tagLC.Pos(wTemp)) > 0)
    {
      newPos = SkipColonOrEqual(pos+wTemp.Length(), tagLC);

      if (newPos)
      {
        wTemp = ParseArg(newPos, tagLC); // Return always, auto...

        if (!wTemp.IsEmpty())
          HtmlToIrcPageBreak(wTemp, wOut);
      }
    }

    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TConvertFromHTML::HtmlToIrcFontSize(WideString wIn, WideString &wOut)
{
  try
  {
    int val;
    if (wIn == WideString("xx-small"))
      val = 6;
    else if (wIn == WideString("x-small"))
      val = 8;
    else if (wIn == WideString("small"))
      val = 10;
    else if (wIn == WideString("medium"))
      val = 12;
    else if (wIn == WideString("large"))
      val = 16;
    else if (wIn == WideString("x-large"))
      val = 20;
    else if (wIn == WideString("xx-large"))
      val = 30;
    else if (wIn == WideString("larger"))
      val = (dts->cSize < 30 ? dts->cSize+1 : 30);
    else if (wIn == WideString("smaller"))
      val = (dts->cSize > 1 ? dts->cSize-1 : 1);
    else // "inherit" or "normal" or a size and units...
    {
      float size;
      WideString Type = GetFontSizeAndType(wIn, size);

      if (size != 0)
      {
        if (Type == WideString("em"))
          val = (int)(size*dts->cSize);
        else if (Type == WideString("pt"))
          val = (int)size;
        else if (Type == WideString("px"))
          val = (int)(size*72/96);
        else if (Type == WideString("%") && dts->cSize != 0)
          val = (int)((size*dts->cSize)/100.);
        else
          val = (int)size; // unknown or no suffix
      }
      else
        val = dts->cSize; // "inherit" or "normal"
    }

    if (val > 0)
      wOut += utils->FontSizeToString(val);

    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TConvertFromHTML::HtmlToIrcPageBreak(WideString wIn, WideString &wOut)
{
  try
  {
    if (wIn == WideString("always") || wIn == WideString("auto"))
      wOut += WideString(CRLF) + WideString(PAGE_BREAK) +
                                                WideString(CRLF);

    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
WideString __fastcall TConvertFromHTML::ParseArg(int &pos, WideString wTag)
// Returns the part after the = or : sign and before the terminating
// char. Example: charset = " utf - 8 " ;) returns " utf - 8 "
// We have to keep the spaces in quotes so the string returned will have
// to be parsed more...
// Return the arguement following the "font-family:", etc that was
// found in S at pos. Also trims any spaces and quotes surrounding arg...
//
// pos is also returned by reference and is the 1-based index of the next
// char following this Arg
{
  try
  {
    WideString Temp;

    bool bQ1 = false; // "
    bool bQ2 = false; // '
    bool bP = false; // (

    int ii;

    for (ii = pos; ii < wTag.Length(); ii++)
    {
      // Only allow one type of quotes, other type can be nested inside them
      if (wTag[ii] == '\"')
      {
        if (!bQ1 && !bQ2)
        {
          // Break out if quote started before "pos" and there is
          // printable text already accumulated in Temp!
          if (Temp.Length())
            break;

          bQ1 = true;
          continue;
        }
        else if (bQ1)
          break;
      }

      // Only allow one type of quotes, other type can be nested inside them
      if (wTag[ii] == '\'')
      {
        if (!bQ1 && !bQ2)
        {
          // Break out if quote started before "pos" and there is
          // printable text already accumulated in Temp!
          if (Temp.Length())
            break;

          bQ2 = true;
          continue;
        }
        else if (bQ2)
          break;
      }

      // This flag keeps us from breaking out until we get the ')'
      if (!bP && wTag[ii] == '(' && !bQ1 && !bQ2)
        bP = true;

      if (bQ1 || bQ2)
        Temp += wTag[ii]; // Add anything in quotes
      else
      {
// the trailing ')' in an rgb(0,0,0) was not being added...
//        if (wTag[ii] == ';' || wTag[ii] == ')' ||
        if (wTag[ii] == ';' ||
                    (!bP && wTag[ii] == ',') || wTag[ii] == '>')
          break;

        if (wTag[ii] != ' ' && wTag[ii] != '=' && wTag[ii] != ':')
          Temp += wTag[ii];
      }
    }

    // return next index
    pos = ii + 1;

    return utils->TrimW(Temp);
  }
  catch(...)
  {
    return "";
  }
}
//---------------------------------------------------------------------------
// intended to bridge the gap between a style element and its ':' or '='
// and any spaces folowing that that are between the ':' and the parameter.
//
// Eg: "<pre background-color  :  '   #00aabb '; color:rgb(0,0,0)/>
// you find the Pos of background-color then call SkipColonOrEqual(pos+16, wTag)
// to return "   #00aabb"
//
// Returns 0 if failure or the position of the element if success
int __fastcall TConvertFromHTML::SkipColonOrEqual(int pos, WideString wTag)
{
  int tagLen = wTag.Length();

  if (pos > tagLen)
    return 0;

  while (wTag[pos] == C_SPACE)
  {
    pos++;

    if (pos > tagLen)
      return 0;
  }

  if (wTag[pos] != ':' && wTag[pos] != '=')
    return 0;

  pos++;

  if (pos > tagLen)
    return 0;

  while (wTag[pos] == C_SPACE)
  {
    pos++;

    if (pos > tagLen)
      return 0;
  }

  if (wTag[pos] == '>' || wTag[pos] == ';' || wTag[pos] == ',')
    return 0;

  return pos;
}
//---------------------------------------------------------------------------
WideString __fastcall TConvertFromHTML::GetFontSizeAndType(WideString S, float &size)
// Input is S, Trimmed of spaces and quotes
// We expect XXpt, XX%, etc. The "pt", "%" is returned
// You can have just digits too and the number can be floating-point for "em"
{
  WideString Size, Type;

  S = utils->LowerCaseW(S);

  for (int ii = 1; ii <= S.Length(); ii++)
  {
    if (iswdigit(S[ii]) || S[ii] == '.')
      Size += S[ii];
    else
      Type += S[ii];
  }

  try
  {
    size = StrToFloat(Size);
  }
  catch(...)
  {
    size = 0.;
  }

  return Type;
}
//---------------------------------------------------------------------------
int __fastcall TConvertFromHTML::GetRgb(WideString S)
// Input is S, Trimmed of spaces and quotes
// Outputs a binary color, 0 and negative numbers are RGB,
// positive integers are "index+1" into the color-table.
//
// Note: "background:" can have a bunch of attributes all separated by space
// or comma, one of which may be a color...
//
// Returns +1 if error (0 or negative number is an RGB color)
{
  BlendColor bc;

  int pos;
  WideString sr, sg, sb;
  WideString Arg("rgb");

  bc.red = 0;
  bc.green = 0;
  bc.blue = 0;

  S = utils->LowerCaseW(S);

  // Look for a color in S of the form rgb(r,g,b)
  // where r, g and b are decimal 0-255

  if ((pos = S.Pos(Arg)) > 0)
  {
    pos += Arg.Length();

    int ii;

    for (ii = pos ; ii <= S.Length(); ii++)
    {
      if (iswdigit(S[ii]))
        sr += S[ii];
      else if (S[ii] == ',')
      {
        ii++;
        break;
      }
    }
    for (; ii <= S.Length(); ii++)
    {
      if (iswdigit(S[ii]))
        sg += S[ii];
      else if (S[ii] == ',')
      {
        ii++;
        break;
      }
    }
    for (; ii <= S.Length(); ii++)
    {
      if (iswdigit(S[ii]))
        sb += S[ii];
      else if (S[ii] == ')')
        break;
    }

    bc.red = utils->ToIntDefW(sr, 0);
    bc.green = utils->ToIntDefW(sg, 0);
    bc.blue = utils->ToIntDefW(sb, 0);

    return -utils->BlendColorToRgb(bc);
  }

  // Look for a color in S of the form #rgb or #rrggbb
  // where r, g b are hex digits 0-9 and a-f

  Arg = "#";

  // Hex codes need this...
  sr = "0x";
  sb = "0x";
  sg = "0x";

  if ((pos = S.Pos(Arg)) > 0)
  {
    WideString C;

    // First try to parse out a 6-wchar_t string... if that fails try for
    // a 3-character string of hex-characters.
    try { C = S.SubString(pos+1, 6); }
    catch(...)
    {
      try { C = S.SubString(pos+1, 3); }
      catch(...) { return 1; } // failed
    }

    int len = C.Length();

    if (len != 3 && len != 6)
      return 1; // 3 or 6 is valid

    for (int ii = 1; ii <= len; ii++)
    {
      if (!utils->mishexw(C[ii]))
        return 1; // error

      if (len == 6) // 6-wchar_t hex color #rrggbb
      {
        if (ii <= 2)
          sr += C[ii];
        else if (ii <= 4)
          sg += C[ii];
        else
          sb += C[ii];
      }
      else // 3-wchar_t hex color #rgb
      {
        if (ii == 1)
          sr += C[ii];
        else if (ii == 2)
          sg += C[ii];
        else
          sb += C[ii];
      }
    }

    // Convert hex string starting with 0x to int, 16 is error
    if (len == 6) // 6-wchar_t hex color #rrggbb
    {
      bc.red = utils->ToIntDefW(sr, 256);
      bc.green = utils->ToIntDefW(sg, 256);
      bc.blue = utils->ToIntDefW(sb, 256);

      if (bc.red == 256 || bc.green == 256 || bc.blue == 256)
        return 1; // error
    }
    else
    {
      bc.red = utils->ToIntDefW(sr, 16);
      bc.green = utils->ToIntDefW(sg, 16);
      bc.blue = utils->ToIntDefW(sb, 16);

      if (bc.red == 16 || bc.green == 16 || bc.blue == 16)
        return 1; // error
    }

    return -utils->BlendColorToRgb(bc);
  }

  return 1; // error
}
//---------------------------------------------------------------------------
WideString __fastcall TConvertFromHTML::ReplaceSpecial(WideString wIn)
// Replace &lt; &quot; &gt;
{
  WideString wOut, Temp;

  int len = wIn.Length();

  for (int ii = 1; ii <= len; ii++)
  {
    if (wIn[ii] == '&') // start of &gt; etc.
    {
      int max = ii + 7; // &agrave; is longest...
      int min = ii + 3; // &lt; is shortest...
      int jj = ii; // start of special code

      for (; jj <= len && jj <= max; jj++)
      {
        wchar_t c = wIn[jj];

        Temp += WideString(c);

        if (c == ';' && jj >= min) // end of &gt; etc.
        {
          wOut += HtmlSpecialCharDecode(Temp);

          Temp = "";

          break;
        }

        if (c == '\"' || c == '\'' || c == '>' || c == ')')
          break;
      }

      // If this was not a special string, just an &...
      if (Temp.Length() > 0)
        wOut += Temp;

      ii = jj; // move past special string
    }
    else
      wOut += wIn[ii];
  }

  return wOut;
}
//---------------------------------------------------------------------------
wchar_t __fastcall TConvertFromHTML::HtmlSpecialCharDecode(WideString In)
// Code to decode HTML special chars (not including the & or ;)
{
  // Use tables in DefaultStrings.cpp to make the mapping efficient!
  // If In matches a string in HTMLCODES, return its corresponding char
  // in the HTMLCHARS const string.
  for (int ii = 0; ii < HTMLCHARSLEN; ii++)
    if (In == WideString(HTMLCODES[ii]))
      return HTMLCHARS[ii];

  // Now check for a code like &#55; or &#1234;
  if (In[2] == '#' && In.Length() >= 4)
  {
    WideString S = In.SubString(3, In.Length()-3);
    int num = utils->ToIntDefW(S, -1); // -1 if error

    // return ANSI or UTF-16, no control chars...
    if (num >= ' ')
      return num;
  }

  return C_NULL;
}
//---------------------------------------------------------------------------

