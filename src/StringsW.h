//---------------------------------------------------------------------------
#ifndef StringsWH
#define StringsWH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
// Class (THE MAIN CLASS IS BELOW!)
//---------------------------------------------------------------------------
// set true for TaeRichEdit and false for YahCoLoRiZe 
#define INCLUDE_UTILS false
//---------------------------------------------------------------------------

class TStringW
{
private:
  WideString s;
  void* tag;

public:
  __fastcall TStringW(WideString s, void* tag);

  __property WideString S = {read = s, write = s};
  __property void* Tag = {read = tag, write = tag};
};
//---------------------------------------------------------------------------
// Class
//---------------------------------------------------------------------------
class TStringsW
{
private:

#if INCLUDE_UTILS
  #define C_LF '\012' // '\n'
  #define C_CR '\015' // '\r'
  #define CRLF "\r\n"
  #define CRLFLEN 2
  #define C_NULL '\0'

  // Borrowed from YahCoLoRiZe' Utils.cpp
  static WideString __fastcall LowerCaseW(WideString s);
  static bool __fastcall WriteStringToFileW(WideString wPath, AnsiString sInfo);
  static AnsiString __fastcall ReadStringFromFileW(WideString wPath);
  static WideString __fastcall Utf8ToWide(AnsiString sIn);
  static WideString __fastcall Utf8ToWide(char* buf, int len);
  static String __fastcall WideToUtf8(WideString sIn);
#endif

  // Copy/Delete text internal methods
  WideString __fastcall CDText(long startIdx, long charCount, bool bCopy);
  WideString __fastcall CDText(TPoint locStart, long charCount, bool bCopy);
  WideString __fastcall CDText(TPoint locStart, TPoint locEnd, bool bCopy);

  TList* pTList; // Pointer to a TList of WideString Objects
  long m_length;

protected:
  WideString __fastcall GetText(void);
  void __fastcall SetText(WideString s);
  long __fastcall GetLineCount(void);
  long __fastcall GetLength(void);

public:
  __fastcall TStringsW(void); // constructor
  __fastcall TStringsW(WideString wIn); // constructor
  __fastcall ~TStringsW(void); // destructor

  void __fastcall DeleteText(long startIdx, long charCount);
  void __fastcall DeleteText(TPoint loc, long charCount);
  void __fastcall DeleteText(TPoint locStart, TPoint locEnd);
  WideString __fastcall CopyText(long startIdx, long charCount);
  WideString __fastcall CopyText(TPoint locStart, long charCount);
  WideString __fastcall CopyText(TPoint locStart, TPoint locEnd);
  WideString __fastcall GetString(long idx);
  void* __fastcall GetTag(long idx);
  int __fastcall IndexOf(WideString sIn, bool bMatchCase = false);
  wchar_t* __fastcall GetTextBuf(void);
  wchar_t* __fastcall GetTextBuf(int &iLen);
  void __fastcall SetTextBuf(wchar_t* buf);
  void __fastcall SetTextStr(WideString s);
  void __fastcall SetString(WideString s, long idx);
  TPoint __fastcall SetStringEx(WideString s, long idx, bool bAddState=false);
  void __fastcall Assign(TStringsW* dest); // Copy our properties
  void __fastcall AddStrings(TStringsW* sl);
  void __fastcall Trim(long idx);

  int __fastcall LoadFromStream(TMemoryStream* ms);
  bool __fastcall SaveToStream(TMemoryStream* ms);
  void __fastcall LoadFromFile(WideString path, bool bAnsi = false);
  bool __fastcall SaveToFile(WideString path, bool bAnsi = false);

  void __fastcall Add(WideString s, void* tag = NULL);
  void __fastcall Insert(long idx, WideString s, void* tag = NULL);
  void __fastcall Delete(long idx);
  void __fastcall Clear(void);

  __property long Count = {read = GetLineCount};
  __property long TotalLength = {read = GetLength};
  __property WideString Text = {read = GetText, write = SetText};

// Example of implementing operators
/*
  TStringsW operator + (TStringsW &);
  TStringsW operator ++ (); // Prefix ++
  TStringsW operator ++ (int); // Postfix ++
*/
};

// Example of implementing operators
/*
TStringsW TStringsW::operator+(TStringsW &Days)
{
  return TStringsW(this->hours + Days.hours);
}

TStringsW TStringsW::operator ++ () // Prefix ++
{
 ++hours;
  return TStringsW(hours);
}

TStringsW TStringsW::operator ++ (int) // Postfix ++
{
  hours++;
  return TStringsW(hours);
}
*/
//---------------------------------------------------------------------------
#endif
