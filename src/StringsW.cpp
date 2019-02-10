//---------------------------------------------------------------------------
// YahCoLoRiZe - Edit, process and send colorized text into IRC chat-rooms
// via various chat-clients such as mIRC, IceChat and LeafChat
//
// Author: Scott Swift
//
// Released to GitHub under GPL v3 October, 2016
//
//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
// Try to include header files common to all files before this directive!
#pragma hdrstop

#include "StringsW.h"

#if INCLUDE_UTILS
#include <stdio.h> // Used in the YcEdit build...
#else
#include "Main.h" // Used only in the Yahcolorize build...
#endif

#pragma package(smart_init)

#if INCLUDE_UTILS
#define utils this
#endif

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// TStringW class functions
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TStringW::TStringW(WideString s, void* tag)
// constructor
{
  this->S = s;
  this->Tag = tag;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// TStringsW class functions
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TStringsW::TStringsW(void)
// constructor
{
  pTList = new TList();
  m_length = 0;
}
//---------------------------------------------------------------------------
__fastcall TStringsW::TStringsW(WideString wIn)
// constructor
{
// Can't do this in the new RAD Studio...
//  TStringsW();
  pTList = new TList();
  m_length = 0;

  this->SetText(wIn);
}
//---------------------------------------------------------------------------
__fastcall TStringsW::~TStringsW(void)
// destructor
{
  this->Clear(); // Delete the items and clear the list

  // Delete the TList
  delete pTList;
}
//---------------------------------------------------------------------------
long __fastcall TStringsW::GetLineCount(void)
{
  return pTList->Count;
}
//---------------------------------------------------------------------------
void* __fastcall TStringsW::GetTag(long idx)
{
  try
  {
    if (this->pTList == NULL || idx < 0)
      return 0;

    long count = this->pTList->Count;

    if (count <= 0 || idx >= count)
      return 0;

    TStringW* pWString = (TStringW*)(this->pTList->Items[idx]);

    if (pWString == NULL)
      return 0;

    return pWString->Tag;
  }
  catch(...)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nTStringsW::GetTag(): Exception!\r\n");
#endif
    return 0;
  }
}
//---------------------------------------------------------------------------
WideString __fastcall TStringsW::GetString(long idx)
{
  try
  {
    if (this->pTList == NULL || idx < 0)
      return "";

    long count = this->pTList->Count;

    if (count <= 0 || idx >= count)
       return "";

    TStringW* pWString = (TStringW*)(this->pTList->Items[idx]);

    if (pWString == NULL)
       return "";

    return pWString->S;
  }
  catch(...)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nTStringsW::GetString(): Exception!\r\n");
#endif
    return "";
  }
}
//---------------------------------------------------------------------------
// SetStringEx should be called if there may be CR/LFs in the string.
// bAddState defaults false. Set it to transfer colors, etc from
// the last char of the previous string to the beginning of the next
// string, automatically.
//
// Returns number of new lines inserted into the string-list in p.y
// and the additional chars added to s for state-strings in p.x
TPoint __fastcall TStringsW::SetStringEx(WideString s, long idx, bool bAddState)
{
  long saveLen = this->GetLength(); // use the function or cr/lfs are not added!
  long saveLineCt = this->GetLineCount();
//DTSColor->CWrite("\r\nsaveLen: " + AnsiString(saveLen) + "\r\n");
//DTSColor->CWrite("\r\nsaveLineCt: " + AnsiString(saveLineCt) + "\r\n");

  // y will have the increase in lines, x will be new length including any
  // state-strings added
  TPoint p(0, 0);

  long len = s.Length();

  bool bContainsCRLFs = false;

  for (long jj = 1; jj <= len; jj++)
  {
    if (s[jj] == C_CR)
    {
      bContainsCRLFs = true;
      break;
    }
  }

  if (bContainsCRLFs)
  {
    // create a new instance of this class
    TStringsW* sl = new TStringsW();
    WideString wTemp;

//DTSColor->CWrite("\r\n" + utils->GetHex(s) + "\r\n");
    for (long jj = 1; jj <= len; jj++)
    {
      if (s[jj] == C_CR)
      {
        if (bAddState)
        {
          WideString wState = utils->GetTrailingState(wTemp);
          sl->Add(wTemp);
//DTSColor->CWrite("\r\nAdd A\r\n");
          wTemp = wState; // init next line with trailing state of previous line
        }
        else
        {
          sl->Add(wTemp);
//DTSColor->CWrite("\r\nAdd B\r\n");
          wTemp = "";
        }
      }
      else if (s[jj] != C_LF)
        wTemp += WideString(s[jj]);
    }

    // add last string (DO NOT CHECK IF EMPTY)
    // This If needs some explanation! We use SetString below to replace
    // the string at idx with the first string in sl then add subsequent
    // strings. So we always want two strings in sl, the first to replace
    // the string at idx and the second to represent a cr/lf (an empty
    // string in a stringlist).
    sl->Add(wTemp);
//DTSColor->CWrite("\r\nAdd C: " +String(sl->Count) + "\r\n");

    long count = sl->Count;

    if (count)
    {
      // Replace the original string with the first new string (will also
      // add a new string if idx == this->Count)
      this->SetString(sl->GetString(0), idx);
      // if sl has just one string added due to a cr/lf we must add a string here!

      // Insert the new strings
      for (long jj = 1; jj < count; jj++)
        this->Insert(idx+jj, sl->GetString(jj));
    }

    delete sl;
  }
  else
    this->SetString(s, idx);

//DTSColor->CWrite("\r\nthis->GetLength(): " + AnsiString(this->GetLength()) + "\r\n");
//DTSColor->CWrite("\r\nthis->GetLineCount(): " + AnsiString(this->GetLineCount()) + "\r\n");
  p.x = this->GetLength() - saveLen; // return # chars added
  p.y = this->GetLineCount() - saveLineCt; // return # lines added

  return p;
}
//---------------------------------------------------------------------------
// SetStringEx should be called if there are CR/LFs in the string!
// This routine is designed to replace an existing string but will add a
// string if the list is empty of if idx == count
//
// idx is the 0-based line-index
void __fastcall TStringsW::SetString(WideString s, long idx)
{
  try
  {
    if (this->pTList == NULL)
      return;

    long count = this->pTList->Count;

    if (count == 0)
    {
      if (idx == 0)
        this->Add(s);
    }
    else if (idx >= 0 && idx < count)
    {
      TStringW* p = (TStringW*)(this->pTList->Items[idx]);

      if (p != NULL)
      {
        long oldLen = p->S.Length();
        long newLen = s.Length();
        p->S = s;
        m_length += newLen - oldLen;
      }
#if DEBUG_ON
      else
        DTSColor->CWrite("\r\nTStringsW::SetString(): Null Pointer!\r\n");
#endif
    }
    else if (idx == count)
      this->Add(s);
#if DEBUG_ON
    else
      DTSColor->CWrite("\r\nTStringsW::SetString(): Out of range!\r\n");
#endif
  }
  catch(...)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nTStringsW::SetString(): Exception!\r\n");
#endif
  }
}
//---------------------------------------------------------------------------
// Concatinates all strings in the list separated by \r\n
WideString __fastcall TStringsW::GetText(void)
{
  if (this->pTList == NULL)
    return "";

  int count = this->pTList->Count;
  WideString sOut;

#if DEBUG_ON
  int testLen = 0;
#endif

  for (int ii = 0; ii < count;)
  {
    sOut += ((TStringW*)(this->pTList->Items[ii]))->S;

#if DEBUG_ON
    testLen += ((TStringW*)(this->pTList->Items[ii]))->S.Length();
#endif

    // Don't add cr/lf at the end of the last string
    if (++ii < count)
    {
      sOut += CRLF;
#if DEBUG_ON
      testLen += CRLFLEN;
#endif
    }
  }

#if DEBUG_ON
  int len = this->TotalLength;
  if (len != testLen)
    DTSColor->CWrite("\r\nTStringsW::GetText(): TotalLength is: " +
            AnsiString(TotalLength) + " should be: " + AnsiString(testLen) + CRLF);
#endif

  return sOut;
}
//---------------------------------------------------------------------------
// Adds a string for each \n - does not store \r\n in the list's
// wide-string items
void __fastcall TStringsW::SetText(WideString s)
{
  this->Clear(); // Clear

  int length = s.Length();

  WideString sOut;
  register WideChar c;

  // Here, we do NOT add cr/lf chars but add a new line for each \n
  // in a long string... but we do add CRLFLEN
  // to the maintained m_length via Add()...
  for (int ii = 1; ii <= length; ii++)
  {
    c = s[ii];

    if (c == C_LF)
    {
      this->Add(sOut);
      sOut.Empty();
    }
    else if (c != C_CR)
      sOut += WideString(c);
  }

  // add any remaining text
  if (!sOut.IsEmpty());
    this->Add(sOut);
}
//---------------------------------------------------------------------------
long __fastcall TStringsW::GetLength(void)
{
  // m_length is maintained as we add or remove strings,
  //
  // add 2 for the \r\n at the end of each line, except for the last line
  //
  int count = this->pTList->Count;

  if (count == 0)
    return 0L;

  if (count == 1)
    return m_length;

  return m_length + ((count-1)*CRLFLEN);
}
//---------------------------------------------------------------------------
void __fastcall TStringsW::Clear(void)
{
  if (this->pTList != NULL)
  {
    int ct = this->pTList->Count;

    // Delete the TStringW objects
    for(int ii = 0; ii < ct; ii++)
    {
      TStringW* p = (TStringW*)this->pTList->Items[ii];

      if (p)
        delete p;
#if DEBUG_ON
      else
        DTSColor->CWrite("\r\nTStringsW::Clear(): Null string pointer at: " +
                                                      AnsiString(ii) + CRLF);
#endif
    }

    this->pTList->Clear();
    this->m_length = 0;
  }
}
//---------------------------------------------------------------------------
int __fastcall TStringsW::IndexOf(WideString sIn, bool bMatchCase)
// bMatchCase defaults false
{
  int count = this->Count;

  if (count == 0)
    return -1;

  if (!bMatchCase)
    sIn = utils->LowerCaseW(sIn);

  WideString sCompare;

  for (int ii = 0; ii < count; ii++)
  {
    sCompare = this->GetString(ii);

    if (!bMatchCase)
      sCompare = utils->LowerCaseW(sCompare);

    if (sCompare == sIn)
      return ii;
  }

  return -1;
}
//---------------------------------------------------------------------------
wchar_t* __fastcall TStringsW::GetTextBuf(void)
{
  int len; // dummy - not used
  return GetTextBuf(len);
}

wchar_t* __fastcall TStringsW::GetTextBuf(int &iLen)
// Return pointer to buffer you must delete! NULL if fails...
{
  wchar_t* pBuf = NULL;

  try
  {
    WideString wTemp = this->Text;

    int len = wTemp.Length() + 1;
    pBuf = new wchar_t[len];

    if (pBuf != NULL)
    {
      // wcscpy won't copy null if source string is empty!
      pBuf[0] = C_NULL;

      if (!wTemp.IsEmpty())
      {
        wcscpy(pBuf, wTemp.c_bstr());
        iLen = len-1;
      }
      else
        iLen = 0;
    }
    else
      iLen = 0;
  }
  catch(...)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nError in TStringsW::GetTextBuf()\r\n");
#endif
    if (pBuf != NULL)
    {
      delete [] pBuf;
      pBuf = NULL;
    }
  }

  return pBuf;

// This works but the buffer isn't null-terminated!
//  return this->Text.Copy();
}
//---------------------------------------------------------------------------
void __fastcall TStringsW::SetTextBuf(wchar_t* buf)
{
  this->Text = WideString(buf);
}
//---------------------------------------------------------------------------
void __fastcall TStringsW::SetTextStr(WideString s)
{
  this->Text = s;
}
//---------------------------------------------------------------------------
void __fastcall TStringsW::AddStrings(TStringsW* sl)
{
  if (sl == NULL)
    return;

  for (int ii = 0; ii < sl->Count; ii++)
  {
    TStringW* p = (TStringW*)sl->pTList->Items[ii];

    if (p != NULL)
      this->Add(p->S, p->Tag);
  }
}
//---------------------------------------------------------------------------
// Convert to UTF8 and save in TMemoryStream
bool __fastcall TStringsW::SaveToStream(TMemoryStream* ms)
{
  AnsiString s = utils->WideToUtf8(this->Text);
  return ms->Write(s.c_str(), s.Length());
}
//---------------------------------------------------------------------------
// Read TMemoryStream's UTF-8 text and convert it to TStringsW
// Returns the character-count read.
int __fastcall TStringsW::LoadFromStream(TMemoryStream* ms)
{
  if (ms == NULL || ms->Size == 0)
    return 0;

  this->Clear();

  int savePos = ms->Position; // save

  ms->Position = 0; // start reading at beginning

  char* buf = new char[ms->Size];
  int iBytes = ms->Read(buf, ms->Size);
  this->Text = utils->Utf8ToWide(buf, iBytes);
  delete [] buf;

  ms->Position = savePos; // restore

  return this->GetLength();
}
//---------------------------------------------------------------------------
// Copies or Deletes text that spans line-breaks in a more-efficent way than
// just using the Text property.
//
// A line-break in charCount counts as 2
//
// Example: The list has two strings, the first with 10 chars and the second
// with 15. Calling DeleteText(0, 10) will leave a blank line in string 1
// and the second string remains as-is. DeleteText(0, 11) will delete the
// first string. DeleteText(11, 15) clears the second string, DeleteText(11, 16)
// removes the second string.
//
// startIdx is 0-based
void __fastcall TStringsW::DeleteText(long startIdx, long charCount)
{
  CDText(startIdx, charCount, false);
}

WideString __fastcall TStringsW::CopyText(long startIdx, long charCount)
{
  return CDText(startIdx, charCount, true);
}

void __fastcall TStringsW::DeleteText(TPoint locStart, long charCount)
{
  CDText(locStart, charCount, false);
}

WideString __fastcall TStringsW::CopyText(TPoint locStart, long charCount)
{
  return CDText(locStart, charCount, true);
}

void __fastcall TStringsW::DeleteText(TPoint locStart, TPoint locEnd)
{
  CDText(locStart, locEnd, false);
}

WideString __fastcall TStringsW::CopyText(TPoint locStart, TPoint locEnd)
{
  return CDText(locStart, locEnd, true);
}

// Counts are based on two chars per line-break. There are no line-breaks
// in a string in the string-list. The line-break is implicit.
// startIdx is 0-based.
WideString __fastcall TStringsW::CDText(long startIdx, long charCount, bool bCopy)
{
  long lineCount = this->pTList->Count;

  if (!lineCount || startIdx >= (int)this->TotalLength ||
                                            startIdx < 0 || charCount <= 0)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nError1: CDText(int startIdx, int charCount)\r\n");
#endif
    return "";
  }

  try
  {
    long ii;
    long len = 0;
    long prevLen = 0;
    TStringW* p;

    for (ii = 0; ii < lineCount; ii++)
    {
      if ((p = (TStringW*)this->pTList->Items[ii]) != NULL)
      {
        len += p->S.Length();

        if (ii < lineCount-1)
          len += CRLFLEN;

        if (len > startIdx)
          break;

        prevLen = len;
      }
    }

    // start-index not found?
    if (ii >= lineCount)
    {
#if DEBUG_ON
      DTSColor->CWrite("\r\nError2: CDText(int startIdx, int charCount)\r\n");
#endif
      return "";
    }

    TPoint locStart(startIdx-prevLen, ii);
    return CDText(locStart, charCount, bCopy);
  }
  catch(...)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nException: CDText(int startIdx, int charCount)\r\n");
#endif
    return "";
  }
}

// NOTE: endIdx and endOffset are 1 greater than the actual "index"
// startIdx and startOffset are the actual 0-based index of the char.
WideString __fastcall TStringsW::CDText(TPoint locStart, long charCount,
                                                                    bool bCopy)
{
  long lineCount = this->pTList->Count;

  if (!lineCount || locStart.x < 0 || locStart.y >= lineCount || charCount <= 0)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nError1: CDText(TPoint locStart, int charCount)\r\n");
    DTSColor->CWrite("\r\nlineCount:" + String(lineCount) +
                  ", locStart.x:" + String(locStart.x) +
                  ", locStart.y:" + String(locStart.y) +
                  " charCount:" + String(charCount) + "\r\n");
#endif
    return "";
  }

  try
  {
    long ii;
    long len = 0;
    long prevLen = 0;
    TStringW* p;

    long startLine = locStart.y;
    long startOffset = locStart.x;

    long endIdx = startOffset + charCount;

    for (ii = startLine; ii < lineCount; ii++)
    {
      if ((p = (TStringW*)this->pTList->Items[ii]) != NULL)
      {
        len += p->S.Length();

        if (ii < lineCount-1)
          len += CRLFLEN;

        if (len > endIdx)
          break;

        prevLen = len;
      }
    }

    int endLine, endOffset;

    // end-index not found?
    if (ii >= lineCount)
    {
      endLine = lineCount-1;

      p = (TStringW*)this->pTList->Items[endLine];
      endOffset = p->S.Length();
    }
    else
    {
      endLine = ii;
      endOffset = endIdx - prevLen;
    }

//#if DEBUG_ON
//    DTSColor->CWrite("\r\nlen: " + String(len));
//    DTSColor->CWrite("\r\nprevLen: " + String(prevLen));
//    DTSColor->CWrite("\r\ncharCount: " + String(charCount));
//    DTSColor->CWrite("\r\nendIdx: " + String(endIdx));
//    DTSColor->CWrite("\r\nendOffset: " + String(endOffset));
//    DTSColor->CWrite("\r\nendLine: " + String(endLine) + "\r\n");
//#endif

    TPoint locEnd(endOffset, endLine);
    return CDText(locStart, locEnd, bCopy);
  }
  catch(...)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nException: CDText(TPoint locStart, int charCount)\r\n");
#endif
    return "";
  }

}

// NOTE: locEnd.x is 1 greater than the actual "index" locStart.x is the actual
// 0-based index of the char.
WideString __fastcall TStringsW::CDText(TPoint locStart, TPoint locEnd,
                                                                    bool bCopy)
{
  long lineCount = this->pTList->Count;

  if (!lineCount || locStart.x < 0 || locEnd.x < 0 ||
                    locStart.y >= lineCount || locEnd.y >= lineCount)
  {
//#if DEBUG_ON
//    DTSColor->CWrite("\r\nError1: CDText(TPoint locStart, TPoint locEnd)\r\n");
//    DTSColor->CWrite("\r\nlineCount:" + String(lineCount) +
//                  ", locStart.x:" + String(locStart.x) +
//                  ", locStart.y:" + String(locStart.y) +
//                  ", locEnd.x:" + String(locEnd.x) +
//                  ", locEnd.y:" + String(locEnd.y) + "\r\n");
//#endif
    return "";
  }

  WideString wCopy;

  long startLine = locStart.y;
  long startOffset = locStart.x;
  long endLine = locEnd.y;
  long endOffset = locEnd.x;

//#if DEBUG_ON
//  DTSColor->CWrite("\r\nlineCount: " + String(lineCount));
//  DTSColor->CWrite("\r\nstartLine: " + String(startLine));
//  DTSColor->CWrite("\r\nstartOffset: " + String(startOffset));
//  DTSColor->CWrite("\r\nendLine: " + String(endLine));
//  DTSColor->CWrite("\r\nendOffset: " + String(endOffset) + "\r\n");
//#endif

  if (bCopy)
  {
    try
    {
      WideString wTemp = GetString(startLine);
      long len = wTemp.Length();

      if (startLine == endLine)
      {
        int delta = endOffset-startOffset;
        wCopy = wTemp.SubString(startOffset+1, delta);

        // add line-terminator if we copied the entire line
        if (startLine != lineCount-1 && len && len == delta)
          wCopy += WideString(CRLF);
      }
      else
      {
        wCopy = wTemp.SubString(startOffset+1, len-startOffset);

        if (startLine != lineCount-1)
          wCopy += WideString(CRLF);

        for (long ii = startLine+1; ii < endLine; ii++)
        {
          wCopy += GetString(ii);
          if (ii != lineCount-1)
            wCopy += WideString(CRLF);
        }

        wTemp = GetString(endLine);
        len = wTemp.Length();
        wCopy += wTemp.SubString(1, endOffset);

        // add line-terminator if we copied the entire line
        if (endLine != lineCount-1 && len && endOffset == len)
          wCopy += WideString(CRLF);
      }
    }
    catch(...) {}
  }
  else // Delete mode
  {
//#if DEBUG_ON
//  DTSColor->CWrite("\r\nstartLine: " + String(startLine) + " startOffset:" +
//    String(startOffset) + " endLine:" + String(endLine) + " endOffset:" +
//      String(endOffset));
//#endif
    try
    {
      TStringW* p;

      WideString wFirst, wLast;

      // Get substrings that remain following delete...

      // Get last substring
      p = (TStringW*)this->pTList->Items[endLine];
      wLast = p->S.SubString(endOffset+1, p->S.Length()-endOffset);
//#if DEBUG_ON
//    DTSColor->CWrite("\r\nwLast: \"" + wLast + "\"\r\n");
//#endif

      if (startLine == endLine)
      {
        wFirst = p->S.SubString(1, startOffset);
//  #if DEBUG_ON
//        DTSColor->CWrite("\r\nwFirst (one line): \"" + wFirst + "\"\r\n");
//  #endif
        this->SetString(wFirst+wLast, endLine);
        return "";
      }

      // Get first substring
      p = (TStringW*)this->pTList->Items[startLine];
      wFirst = p->S.SubString(1, startOffset);
//#if DEBUG_ON
//      DTSColor->CWrite("\r\nwFirst: \"" + wFirst + "\"\r\n");
//#endif

      // !!!!!!!!!! Had to fix/redo the code below 12/14/2018 ----------
      // NOTE: I'm not happy with this at all. It is a workaround for
      // a peculiarity of Microsoft's rich-edit control. The control
      // LineCount does not change when a new-line is entered or deleted.
      // LineCount changes when the first char of a new line is typed or the
      // last char in a line is deleted, not the line itself. So everything below
      // is just to "make it work" - allow us to track the characters and
      // add/remove them to out SL_IRC or SL_ORG string-lists intermingled with
      // our own color/text format codes...

      // delete last line if entire line selected
      if (wLast.IsEmpty())
      {
        if (startLine == 0 && startOffset == 0)
          endLine--;
        else
        {
          this->Delete(endLine);

          if (!wFirst.IsEmpty())
            endLine--;
        }
      }

      // delete in-between lines
      for (long ii = startLine+1; ii <= endLine; ii++)
        this->Delete(startLine+1);

      // concatenate first and last substrings
      if (wFirst.IsEmpty() && wLast.IsEmpty())
        this->Delete(startLine);
      else
        this->SetString(wFirst+wLast, startLine);
      //----------------------------------------------------------------
    }
    catch(...)
    {
#if DEBUG_ON
      DTSColor->CWrite("\r\nException: utils->CDText(TPoint locStart, TPoint locEnd)\r\n");
#endif
    }
  }

  return wCopy;
}
//---------------------------------------------------------------------------
void __fastcall TStringsW::Delete(long idx)
{
  long count = this->Count;

  if (idx < 0 || idx >= count)
    return;

  try
  {
    TStringW* p = (TStringW*)this->pTList->Items[idx];

    if (p != NULL)
    {
      this->m_length -= p->S.Length();

      if (this->m_length < 0)
      {
#if DEBUG_ON
        DTSColor->CWrite("\r\nTStringsW::Delete(): m_length < 0: " +
                                                  AnsiString(m_length) + "\r\n");
#endif
        m_length = 0;
      }

      delete p;
    }
  }
  __finally
  {
    this->pTList->Delete(idx);
  }
}
//---------------------------------------------------------------------------
// Insert 0 would replace the item at index 0 and shove the other items
// to higher indexes (I call that "down" some say "up"). Insert at
// pTList->Count would simply add a new item at the bottom of the list.
void __fastcall TStringsW::Insert(long idx, WideString s, void* tag)
{
  try
  {
    // Add new
    this->pTList->Insert(idx, new TStringW(s, tag));
    this->m_length += s.Length();
  }
  catch(...)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nTStringsW::Insert() failed\r\n");
#endif
  }
}
//---------------------------------------------------------------------------
// NOTE - if s has line-breaks in it they don't get added as seperate lines!
void __fastcall TStringsW::Add(WideString s, void* tag)
{
  try
  {
    // Add new
    this->pTList->Add(new TStringW(s, tag));

    this->m_length += s.Length();
  }
  catch(...)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nTStringsW::Add() failed\r\n");
#endif
  }
}
//---------------------------------------------------------------------------
void __fastcall TStringsW::Trim(long idx)
{
  try
  {
    if (this->Count > 0 && idx >= 0 && idx < this->Count)
    {
      WideString wTemp = this->GetString(idx);
      wTemp = utils->TrimW(wTemp);
      this->SetString(wTemp, idx);
    }
  }
  catch(...)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nTStringsW::Trim() failed\r\n");
#endif
  }
}
//---------------------------------------------------------------------------
bool __fastcall TStringsW::SaveToFile(WideString path, bool bAnsi)
// bAnsi defaults false
{
  if (bAnsi)
    return utils->WriteStringToFileW(path, AnsiString(this->Text));

  return utils->WriteStringToFileW(path, utils->WideToUtf8(this->Text));
}
//---------------------------------------------------------------------------
void __fastcall TStringsW::LoadFromFile(WideString path, bool bAnsi)
// bAnsi defaults false
{
  String s = utils->ReadStringFromFileW(path);

  if (bAnsi)
    this->Text = WideString(s);
  else
    this->Text = utils->Utf8ToWide(s);
}
//---------------------------------------------------------------------------
void __fastcall TStringsW::Assign(TStringsW* dest)
{
  try
  {
    dest->Text = this->Text; // This also sets the "Count" and TotalLength"
  }
  catch(...)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nTStringsW::Assign() failed\r\n");
#endif
  }
}

#if INCLUDE_UTILS
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Functions from Utils.cpp
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
WideString __fastcall TStringsW::LowerCaseW(WideString s)
{
  WideChar c;

  int len = s.Length();

  for (int ii = 1; ii <= len; ii++)
  {
    c = s[ii];

    if (iswlower(c))
      s[ii] = towupper(c);
  }
  return WideString(s);
}
//---------------------------------------------------------------------------
bool __fastcall TStringsW::WriteStringToFileW(WideString wPath, String sInfo)
// Writes sInfo (ANSI or UTF-8) to a UTF-16 path
{
  bool bRet = false;
  FILE* f = NULL;

  try
  {
    // open/create file for writing in text-mode
    f = _wfopen(wPath.c_bstr(), L"wb");

    unsigned length = sInfo.Length();

    if (f != NULL)
      if (fwrite(sInfo.c_str(), sizeof(char), length, f) == length)
        bRet = true;
  }
  __finally
  {
    try { if (f != NULL) fclose(f); } catch(...) {}
  }

  return bRet;
}
//---------------------------------------------------------------------------
// Reads a file into a String (assuming it's UTF-8!)
String __fastcall TStringsW::ReadStringFromFileW(WideString wPath)
{
  FILE* f = NULL;
  char* buf = NULL;

  String sOut;

  try
  {
    // open/create file for reading in text-mode
    f = _wfopen(wPath.c_bstr(), L"rb");

    if (f != NULL)
    {
      // get file-length
      fseek(f, 0L, SEEK_END);
      int length = ftell(f);
      rewind(f);

      buf = new char[length];
      int n = fread(buf, 1, length, f);

      if (n == length)
        sOut = AnsiString(buf, length);
    }
  }
  __finally
  {
    try { if (f != NULL) fclose(f); } catch(...) {}
    try { if (buf != NULL) delete [] buf; } catch(...) {}
  }

  return sOut;
}
//---------------------------------------------------------------------------
String __fastcall TStringsW::WideToUtf8(WideString sIn)
// Code to convert UTF-16 (WideString or WideChar wchar_t) to UTF-8
{
  if (sIn.IsEmpty()) return "";

  int nLenUtf16 = sIn.Length();

  int nLenUtf8 = WideCharToMultiByte(CP_UTF8, // UTF-8 Code Page
    0, // No special handling of unmapped chars
    sIn.c_bstr(), // wide-character string to be converted
    nLenUtf16,
    NULL, 0, // No output buffer since we are calculating length
    NULL, NULL); // Unrepresented char replacement - Use Default

  // nNameLen does NOT appear to include the NULL character!
  char* buf = new char[nLenUtf8];

  WideCharToMultiByte(CP_UTF8, // UTF-8 Code Page
    0, // No special handling of unmapped chars
    sIn.c_bstr(), // wide-character string to be converted
    nLenUtf16,
    buf,
    nLenUtf8,
    NULL, NULL); // Unrepresented char replacement - Use Default

  AnsiString sOut = AnsiString(buf, nLenUtf8); // there is no null written...
  delete [] buf;

  return sOut;
}
//---------------------------------------------------------------------------
// Overloaded...

WideString __fastcall TStringsW::Utf8ToWide(char* buf, int len)
// Code to convert UTF-8 to UTF-16 (wchar_t)
{
  return Utf8ToWide(AnsiString(buf, len));
}

WideString __fastcall TStringsW::Utf8ToWide(AnsiString sIn)
// Code to convert UTF-8 to UTF-16 (wchar_t)
{
  WideString sWide;

  if (sIn.Length() > 0)
  {
    wchar_t* pwText = NULL;

    try
    {
      // Use the MultiByteToWideChar function to determine the size of the UTF-16 representation of the string. You use
      // this size to allocate a new buffer that can hold the UTF-16 version of the string.
      DWORD dwNum = MultiByteToWideChar(CP_UTF8, 0, sIn.c_str(), -1, NULL, 0);
      wchar_t *pwText = new wchar_t[dwNum];

      if (pwText != NULL)
      {
        // The MultiByteToWideChar function takes the UTF-8 string and converts it into UTF-16, storing it in pwText.
        MultiByteToWideChar(CP_UTF8, 0, sIn.c_str(), -1, pwText, dwNum);

        // Convert to VCL WideString.
        if (dwNum > 1)
          sWide = WideString(pwText, dwNum-1);
      }
    }
    // NOTE: Unlike C#, __finally does NOT get called if we execute a return!!!
    __finally
    {
      try { if (pwText != NULL) delete [] pwText; } catch(...) {}
    }
  }

  return sWide;
}
//---------------------------------------------------------------------------
#undef utils
#endif
