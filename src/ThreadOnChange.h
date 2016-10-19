//---------------------------------------------------------------------------
#ifndef ThreadOnChangeH
#define ThreadOnChangeH
//---------------------------------------------------------------------------

#define OC_MAXOBJECTS 256 // Max # ONCHANGEW structs

// this should be the max chars that can be added to a TaeRichEdit in
// one fell swoop (3 for now... including null)
#define OC_BUFSIZE 3

// OnChange structure
typedef struct
{
  long line; // TaeEdit->Line
  long selStart; // TaeEdit->SelStart
  long length; // TaeEdit->TextLength (counts line-breaks as 2 chars!)
  long lineCount; // TaeEdit->Lines->Count
  long deltaLength; // change in length
  long deltaLineCount; // change in # lines
  int view; // V_OFF, V_RTF etc.
  bool insert; // Insert mode flag
  void* p; // Memory stream (or other data-object)
  TPoint pos; // caret position before chars were added or after chars were deleted
  wchar_t c[OC_BUFSIZE]; // Input character(s) (NULL TERMINATED!)
} ONCHANGEW;

class TThreadOnChange : public TThread
{
  friend class TDTSColor;

private:
  ONCHANGEW val;
  ONCHANGEW* ocbuf;

  __thread bool bLocked;
  __thread int ridx, widx;

  bool __fastcall ProcessOC(ONCHANGEW oc);
  bool __fastcall IsBusy(void);

protected:
  virtual void __fastcall Execute();
  ONCHANGEW __fastcall Read(void);

public:
  __fastcall TThreadOnChange(void);
  __fastcall ~TThreadOnChange(void);

  void __fastcall Add(ONCHANGEW oc);
  void __fastcall Flush(void);

  __property bool Locked = {read = IsBusy};
};
//---------------------------------------------------------------------------
 extern TThreadOnChange* ThreadOnChange;
//---------------------------------------------------------------------------
#endif
