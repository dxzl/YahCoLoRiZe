//---------------------------------------------------------------------------
#ifndef DlgSpellCheckH
#define DlgSpellCheckH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include "TaeRichEdit.h"
//---------------------------------------------------------------------------
// Language constants for radio-group
#define SC_ENGLISH 0
#define SC_FRENCH  1
#define SC_SPANISH 2

// Hun(garian) Spell-Checker
#define SC_MAXWORDLEN 31
//---------------------------------------------------------------------------
class TSpellCheckForm : public TForm
{
__published:  // IDE-managed Components
  TListBox *SuggestionListBox;
  TGroupBox *MyDictionaryGroupBox;
  TButton *AddButton;
  TButton *DelButton;
  TButton *LanguageButton;
  TButton *HelpButton;
  TButton *CloseButton;
  TPanel *ButtonPanel;
  TButton *IgnoreAllButton;
  TButton *IgnoreButton;
  TButton *ReplaceButton;
  TButton *ReplaceAllButton;
  TTaeRichEdit *WordEdit;
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall CloseButtonClick(TObject *Sender);
  void __fastcall LanguageRadioGroupClick(TObject *Sender);
  void __fastcall FormDestroy(TObject *Sender);
  void __fastcall SuggestionListBoxClick(TObject *Sender);
  void __fastcall ReplaceButtonClick(TObject *Sender);
  void __fastcall ReplaceAllButtonClick(TObject *Sender);
  void __fastcall IgnoreButtonClick(TObject *Sender);
  void __fastcall IgnoreAllButtonClick(TObject *Sender);
  void __fastcall AddButtonClick(TObject *Sender);
  void __fastcall DelButtonClick(TObject *Sender);
  void __fastcall HelpButtonClick(TObject *Sender);
  void __fastcall LanguageButtonClick(TObject *Sender);
  void __fastcall SuggestionListBoxDrawItem(TWinControl *Control,
          int Index, TRect &Rect, TOwnerDrawState State);
//  void __fastcall WordEditChange(TObject *Sender, DWORD dwChangeType,
//          void *pvCookieData, bool &AllowChange);
  void __fastcall WordEditChange(TObject *Sender);
private:  // User declarations
  void __fastcall InitDlg(void);
  bool __fastcall LoadLibHunspell(WideString libraryName);
  bool __fastcall InitDictionary(void);
  bool __fastcall SpellCheck(void);
  bool __fastcall ProcessNextWord(void);
  int __fastcall CountWord(int iStart, int iEnd, WideString sWord);
  bool __fastcall YCalpha(wchar_t c);

  typedef void* (*tHSInit)(char* affFile, char* dictFile); // "hunspell_initialize"
  typedef void (*tHSUninit)(void* spell); // "hunspell_uninitialize"
  typedef bool (*tHSSpell)(void* spell, char* word); // "hunspell_spell"
  typedef int (*tHSSuggest)(void* spell, char* word, char** &suggestions); // "hunspell_suggest"
  typedef void (*tHSSuggestFree)(void* spell, char** suggestions, int suggestLen); // "hunspell_suggest_free"
  typedef char* (*tHSGetDicEncoding)(void* spell); // "hunspell_get_dic_encoding"
  // add word to the run-time dictionary
  typedef int (*tHSAdd)(void* spell, char* word); // "hunspell_add"

  tHSInit HSInit;
  tHSUninit HSUninit;
  tHSSpell HSSpell;
  tHSSuggest HSSuggest;
  tHSSuggestFree HSSuggestFree;
  tHSGetDicEncoding HSGetDicEncoding;
  tHSAdd HSAdd;

  HANDLE HSDLLHandle;
  void* pSpell;
  int LangIdx, CurrentIdx, SelOffset, ReplaceCount;
  bool bUTF8;

  String Encoding;
  WideString wText;
  TStringList* IgnoreAll; // in utf-8

  TDTSColor* dts; // easy pointer to main form...

public:    // User declarations
  __fastcall TSpellCheckForm(TComponent* Owner);
  void __fastcall ScReset(void);
  void __fastcall ScClose(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TSpellCheckForm *SpellCheckForm;
//---------------------------------------------------------------------------
#endif
