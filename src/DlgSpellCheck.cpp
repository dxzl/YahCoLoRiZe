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

#include "DlgSpellCheck.h"
#include "DlgReplaceText.h"
#include "DlgShowDict.h"
#include "DlgChooseDict.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "TaeRichEdit"
#pragma resource "*.dfm"
TSpellCheckForm *SpellCheckForm;
//---------------------------------------------------------------------------
__fastcall TSpellCheckForm::TSpellCheckForm(TComponent* Owner)
  : TForm(Owner)
{
  this->dts = DTSColor;
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::FormCreate(TObject *Sender)
{
  pSpell = NULL;
  HSDLLHandle = 0;
  LangIdx = -1;
  bUTF8 = false;

  InitDlg();

  try
  {
    WideString DllPath = utils->ExtractFilePathW(utils->GetExeNameW()) +
                                "\\" + utils->Utf8ToWide(SPELLINGMSG[35]);

    //WideString DllPath = utils->GetSystemDir(CSIDL_SYSTEM) +
    //                            "\\" + String(SPELLINGMSG[35]);

    // "hunspell.dll", HunSpell spell-checker DLL 35
    if (!LoadLibHunspell(DllPath))
    {
      DllPath = utils->ExtractFilePathW(utils->GetExeNameW() +
                                        utils->Utf8ToWide(SPELLINGMSG[35]));

      if (!LoadLibHunspell(DllPath))
      {
        // "Unable to check spelling on this system...",
        utils->ShowMessageU(SPELLINGMSG[27]);
        return;
      }
    }

    // Establish a sorted IgnoreList with no duplicates
    IgnoreAll = new TStringList();
    IgnoreAll->Sorted = true;
    IgnoreAll->Duplicates = dupIgnore;

    if (!InitDictionary())
      return;

    SpellCheck();
  }
  catch(...)
  {
    // "Unable to check spelling on this system..."
    utils->ShowMessageU(SPELLINGMSG[27]);
  }

}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::InitDlg(void)
// Set all the hints, lables, Etc.
{
  this->Caption = SPELLINGMSG[0];
  LanguageButton->Caption = SPELLINGMSG[6];
  ReplaceButton->Caption = SPELLINGMSG[7];
  ReplaceAllButton->Caption = SPELLINGMSG[8];
  HelpButton->Caption = SPELLINGMSG[9];
  IgnoreButton->Caption = SPELLINGMSG[10];
  IgnoreAllButton->Caption = SPELLINGMSG[11];
  CloseButton->Caption = SPELLINGMSG[12];
  AddButton->Caption = SPELLINGMSG[13];
  DelButton->Caption = SPELLINGMSG[14];

  // Hints
  AddButton->Hint = SPELLINGMSG[31];
  DelButton->Hint = SPELLINGMSG[32];
  LanguageButton->Hint = SPELLINGMSG[33];
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::ScReset(void)
// Public function, resets/re-starts spell-check
// Called from Main.cpp if the user clears, loads or modifies the text
{
#if DEBUG_ON
      DTSColor->CWrite( "\r\nSCRESET CALLED!\r\n");
#endif

  if (tae->SelLength > 0)
    tae->SelLength = 0;

  this->SpellCheck();
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::ScClose(void)
// Public function, closes spell-check
// Called from Main.cpp if the user clears, loads or processes the text
{
#if DEBUG_ON
      dts->CWrite( "\r\nSCCLOSE CALLED!\r\n");
#endif

  this->Close();
}
//---------------------------------------------------------------------------
bool __fastcall TSpellCheckForm::LoadLibHunspell(WideString libraryName)
{
  if (libraryName.IsEmpty())
    libraryName = "hunspell.dll";

  if (HSDLLHandle != 0)
    try {FreeLibrary(HSDLLHandle);} catch(...) {}

  try
  {
    HSDLLHandle = LoadLibraryW(libraryName.c_bstr());
  }
  catch(...)
  {
    return false;
  }

  /*
  // Additional Info (S.S.) - I started with an erlier version of hunspell.dll
  // from a build for Delphi on the SourceForge Hunspell website:
  // http://sourceforge.net/projects/hunspell/files/Hunspell/
  // Then I found a newer HunSpellx86.dll and Hunspell.x64.dll.
  // I used impdef.exe to list its entry points... then later I found the
  // header-file (below) that documents the entry-points...
  //

  if (HSDLLHandle == 0)
    return false;

  typedef struct Hunhandle Hunhandle;

  Hunhandle *Hunspell_create(const char * affpath, const char * dpath);
  Hunhandle *Hunspell_create_key(const char * affpath, const char * dpath,
    const char * key);
  void Hunspell_destroy(Hunhandle *pHunspell);
  // spell(word) - spellcheck word
  // output: 0 = bad word, not 0 = good word
  int Hunspell_spell(Hunhandle *pHunspell, const char *);
  char *Hunspell_get_dic_encoding(Hunhandle *pHunspell);
  // suggest(suggestions, word) - search suggestions
  // input: pointer to an array of strings pointer and the (bad) word
  //   array of strings pointer (here *slst) may not be initialized
  // output: number of suggestions in string array, and suggestions in
  //   a newly allocated array of strings (*slts will be NULL when number
  //   of suggestion equals 0.)
  //
  int Hunspell_suggest(Hunhandle *pHunspell, char*** slst, const char * word);

  // morphological functions

  // analyze(result, word) - morphological analysis of the word
  int Hunspell_analyze(Hunhandle *pHunspell, char*** slst, const char * word);
  // stem(result, word) - stemmer function
  int Hunspell_stem(Hunhandle *pHunspell, char*** slst, const char * word);
  // stem(result, analysis, n) - get stems from a morph. analysis
  // example:
  // char ** result, result2;
  // int n1 = Hunspell_analyze(result, "words");
  // int n2 = Hunspell_stem2(result2, result, n1);
  int Hunspell_stem2(Hunhandle *pHunspell, char*** slst, char** desc, int n);

  // generate(result, word, word2) - morphological generation by example(s)
  int Hunspell_generate(Hunhandle *pHunspell, char*** slst, const char * word,
    const char * word2);

  // generate(result, word, desc, n) - generation by morph. description(s)
  // example:
  // char ** result;
  // char * affix = "is:plural"; // description depends from dictionaries, too
  // int n = Hunspell_generate2(result, "word", &affix, 1);
  // for (int i = 0; i < n; i++) printf("%s\n", result[i]);
  int Hunspell_generate2(Hunhandle *pHunspell, char*** slst, const char * word,
    char** desc, int n);

  // functions for run-time modification of the dictionary

  // add word to the run-time dictionary
  int Hunspell_add(Hunhandle *pHunspell, const char * word);
  // add word to the run-time dictionary with affix flags of
  // the example (a dictionary word): Hunspell will recognize
  // affixed forms of the new word, too.
  int Hunspell_add_with_affix(Hunhandle *pHunspell, const char * word, const char * example);
  // remove word from the run-time dictionary
  int Hunspell_remove(Hunhandle *pHunspell, const char * word);
  // free suggestion lists
  void Hunspell_free_list(Hunhandle *pHunspell, char *** slst, int n);
  */

  // Hunspell DLL procedure addresses (.def file below generated
  // with impdef.exe)
  //
  //LIBRARY     HUNSPELL.DLL
  //
  //EXPORTS
  //    Hunspell_add                   @343 ; Hunspell_add
  //    Hunspell_add_with_affix        @344 ; Hunspell_add_with_affix
  //    Hunspell_analyze               @345 ; Hunspell_analyze
  //    Hunspell_create                @346 ; Hunspell_create
  //    Hunspell_create_key            @347 ; Hunspell_create_key
  //    Hunspell_destroy               @348 ; Hunspell_destroy
  //    Hunspell_free_list             @349 ; Hunspell_free_list
  //    Hunspell_generate              @350 ; Hunspell_generate
  //    Hunspell_generate2             @351 ; Hunspell_generate2
  //    Hunspell_get_dic_encoding      @352 ; Hunspell_get_dic_encoding
  //    Hunspell_remove                @353 ; Hunspell_remove
  //    Hunspell_spell                 @354 ; Hunspell_spell
  //    Hunspell_stem                  @355 ; Hunspell_stem
  //    Hunspell_stem2                 @356 ; Hunspell_stem2
  //    Hunspell_suggest               @357 ; Hunspell_suggest
  //    hunspell_add                   @358 ; hunspell_add
  //    hunspell_add_with_affix        @359 ; hunspell_add_with_affix
  //    hunspell_free_list             @360 ; hunspell_free_list
  //    hunspell_get_dic_encoding      @361 ; hunspell_get_dic_encoding
  //    hunspell_initialize            @362 ; hunspell_initialize
  //    hunspell_spell                 @363 ; hunspell_spell
  //    hunspell_suggest               @364 ; hunspell_suggest
  //    hunspell_suggest_free          @365 ; hunspell_suggest_free
  //    hunspell_uninitialize          @366 ; hunspell_uninitialize
  HSInit = (tHSInit)GetProcAddress(HSDLLHandle, "hunspell_initialize");
  HSUninit = (tHSUninit)GetProcAddress(HSDLLHandle, "hunspell_uninitialize");
  HSSpell = (tHSSpell)GetProcAddress(HSDLLHandle, "hunspell_spell");
  HSSuggest = (tHSSuggest)GetProcAddress(HSDLLHandle, "hunspell_suggest");
  HSSuggestFree = (tHSSuggestFree)GetProcAddress(HSDLLHandle, "hunspell_suggest_free");
  HSGetDicEncoding = (tHSGetDicEncoding)GetProcAddress(HSDLLHandle, "hunspell_get_dic_encoding");
  HSAdd = (tHSAdd)GetProcAddress(HSDLLHandle, "hunspell_add");

  if (HSInit == 0 ||
      HSUninit == 0 ||
      HSSpell == 0 ||
      HSSuggest == 0 ||
      HSSuggestFree == 0 ||
      HSGetDicEncoding == 0 ||
      HSAdd == 0) return false;

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TSpellCheckForm::InitDictionary(void)
{
  // Dictionary(s) will be in the install directory Dict sub-directory...
  WideString DictPath = utils->ExtractFilePathW(utils->GetExeNameW()) +
                                              utils->Utf8ToWide(DICTDIR);
  //WideString DictPath = utils->GetSpecialFolder(CSIDL_APPDATA) +
  //        "\\" + utils->Utf8ToWide(OUR_COMPANY) + "\\" + utils->Utf8ToWide(OUR_NAME) +
                                                  "\\" + utils->Utf8ToWide(DICTDIR);

  WideString wAff = DictPath + dts->GDictionary + ".aff";
  WideString wDct = DictPath + dts->GDictionary + ".dic";

  if (dts->GDictionary.IsEmpty() || !utils->FileExistsW(wAff) ||
                                              !utils->FileExistsW(wDct))
  {
    Application->CreateForm(__classid(TChooseDictForm), &ChooseDictForm);

    if (ChooseDictForm->NumDictionaries != 0)
    {
      if (ChooseDictForm->NumDictionaries > 0)
        ChooseDictForm->ShowModal();

      dts->GDictionary = ChooseDictForm->Dict;
    }
    else
      dts->GDictionary = ""; // No dictionary!

    ChooseDictForm->Release();

    if (dts->GDictionary.IsEmpty())
    {
      dts->GDictionary = utils->Utf8ToWide(DEF_DICTIONARY);
      utils->ShowMessageU(SPELLINGMSG[28]); // Instructions to find a dictionary...
      return false;
    }

    wAff = DictPath + dts->GDictionary + ".aff";
    wDct = DictPath + dts->GDictionary + ".dic";
  }

  SuggestionListBox->Clear();
  WordEdit->TextW = "";

  try
  {
    // Unload old dictionary
    if (pSpell != NULL)
    {
      try
      {
        HSUninit(pSpell);
        pSpell = NULL;
      }
      catch(...) {}
    }

    // pass in UTF-8 file paths
    pSpell = HSInit(utils->WideToUtf8(wAff).c_str(),
                            utils->WideToUtf8(wDct).c_str());

    if (pSpell == NULL)
      return false;

    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TSpellCheckForm::SpellCheck(void)
{
  try
  {
    if (pSpell == NULL)
    {
      // "Unable to check spelling on this system...", // 187
      utils->ShowMessageU(DS[187]);
      return false;
    }

    if (tae->TextLength == 0)
    {
      // "There is no text to process...", // 45
      utils->ShowMessageU(DS[45]);
      return false;
    }

    Encoding = HSGetDicEncoding(pSpell);
    Caption = String(SPELLINGMSG[0]) + " (" + dts->GDictionary + ":" + Encoding + ")";

    bUTF8 = (Encoding.LowerCase() == "utf-8") ? true : false;

    // add a space to end as a word-delimiter...
// NOTE: it's possible to set the rich edit EM_GETTEXTEX call to not return both CR and LF!
// would be faster! GT_DEFAULT in RichEdit.h only returns the CR... Just what we want here
//
// Also should change TStringsW() to only return CR - and mod all the program to be simpler to
// compute offsets... but a lot of stuff would need to be changed
    wText = utils->StripChar(tae->TextW, C_LF) + WideString(C_SPACE);

    CurrentIdx = 1; // Start at the beginning
    ReplaceCount = 0;

    // +/- Offset that accumulates as words are replaced with
    // different-sized words (allows us to keep the Text string
    // as-is, not replacing words in it, but compensating for
    // index changes as we replace SelText with new words...
    SelOffset = 0;

    if (!ProcessNextWord())
      return false;

    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TSpellCheckForm::ProcessNextWord(void)
{
  if (pSpell == NULL)
  {
    ShowMessage("pSpell is NULL in TSpellCheckForm::ProcessNextWord()!");
    return false;
  }

  try
  {
    int ii = CurrentIdx;
    int Idx; // Used by IgnoreAll->Find()

    sc_top: // not many goto's but sometimes they come in handy!

    // Skip non alphabetic chars (Text has an extra space char tacked on!)
    // so don't use ii <= wText.Length()!
    int len = wText.Length();
    for (; ii < len; ii++)
      if (YCalpha(wText[ii]))
        break;

    CurrentIdx = ii;
    SuggestionListBox->Items->Clear();

    WideString wCW; // Current Word
    String sUtf8CW;

    // Find the 1-based index of the next word
    for (; ii < len; ii++)
    {
      if (!YCalpha(wText[ii]))
      {
        // Check to see if word is the right size and is not in our local
        // IgnoreAll list or Custom dictionary...
        if (wCW.Length() > 0 &&
              wCW.Length() <= SC_MAXWORDLEN &&
                !IgnoreAll->Find(sUtf8CW, Idx) &&
                  !dts->GDictList->Find(sUtf8CW, Idx))
        {
          if (bUTF8)
            sUtf8CW = utils->WideToUtf8(wCW);
          else
            sUtf8CW = String(wCW);

          // Check spelling

          if (!HSSpell(pSpell, sUtf8CW.c_str()))
          {
            // Word not in dictionary...

            // Highlight
            tae->SelStart = CurrentIdx+SelOffset-1;
            tae->SelLength = ii-CurrentIdx;

            // Get suggestions
            char** strings = NULL;
            int iLen = HSSuggest(pSpell, sUtf8CW.c_str(), strings);

            if (strings != NULL)
            {
              if (iLen > 0)
              {
                //display
                String OutWord;

                for(int jj = 0; jj < iLen; jj++)
                {
                  OutWord = strings[jj];

                  if (!bUTF8)
                    OutWord = utils->AnsiToUtf8(OutWord);

                  SuggestionListBox->Items->Add(OutWord);
                }
              }

              //free the allocated memory
              HSSuggestFree(pSpell, strings, iLen);

              WordEdit->OnChange = NULL;
              WordEdit->TextW = wCW;
              WordEdit->OnChange = WordEditChange;

              if (!this->Visible)
                Show(); // Display ourself!

              // Relocate this dialog if it's on top of the new selection!
              if (this->Visible)
                utils->RelocateDialog(this);

              CurrentIdx = ii+1; // save for the next call...
              return true;
            }
          }
          //else // found in dictionary
          //  ShowMessage(CurrentWord);
        }

        ii++;
        CurrentIdx = ii; // save for the next call...
        goto sc_top;
      }

      wCW += wText[ii];
      sUtf8CW = utils->WideToUtf8(wCW); // any better way to do this?
    }

    // Back to the top!
    tae->SelStart = 0;
    tae->SetFocus();

    CurrentIdx = ii;

    // If no misspellings found, we don't want to close the dialog without
    // letting the user delete words from the custom dictionary...
    // Simulate Del button-click if they answer Yes...
    if (ReplaceCount == 0 && !Visible)
    {
      if (utils->ShowMessageU(Handle, SPELLINGMSG[26],
                      MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1) == IDYES)
      {
        Application->CreateForm(__classid(TShowDictForm), &ShowDictForm);

        // Returns 1 if items deleted
        ShowDictForm->ShowModal();
        bool bItemsDeleted = ShowDictForm->ItemsDeleted;
        ShowDictForm->Release();

        // Restart the search if any words deleted from My Dictionary
        if (bItemsDeleted)
          SpellCheck();
        else
          Close();
      }
      else
        Close();
    }
    else
      Close();

    return true;
  }
  catch(...)
  {
    ShowMessage("Exception thrown in TSpellCheckForm::ProcessNextWord()!");
    return false;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TSpellCheckForm::YCalpha(wchar_t c)
{
  wchar_t c_lc = towlower(c);
  return iswalpha(c_lc);
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::LanguageRadioGroupClick(TObject *Sender)
{
  if (InitDictionary())
    SpellCheck();
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::SuggestionListBoxClick(TObject *Sender)
{
  WordEdit->OnChange = NULL;
  String sIn = SuggestionListBox->Items->Strings[SuggestionListBox->ItemIndex];
  WordEdit->TextW = utils->Utf8ToWide(sIn);
  WordEdit->OnChange = WordEditChange;
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::ReplaceButtonClick(TObject *Sender)
{
  WideString ReplaceWord = WordEdit->TextW;
  WideString OrigWord = tae->SelTextW;

  if (tae->View == V_RTF && OrigWord.Length() > 0 && OrigWord != ReplaceWord)
  {
    utils->PushOnChange(tae);

    int iFirst, iLast, CI;
    if (!utils->GetCodeIndices(dts->SL_IRC, iFirst, iLast, CI, tae->SelStart, tae->SelLength))
      return;

    int lenDiff = ReplaceWord.Length() - tae->SelLength;
    SelOffset += lenDiff; // Cumulative offset to apply when setting SelStart

    // Add an undo including the text we will remove and the length of
    // the text we will put in it's place
    ONCHANGEW oc = utils->GetInfoOC(tae, dts->SL_IRC);
    WideString Temp = dts->SL_IRC->Text.SubString(iFirst+1, iLast-iFirst);
    TOCUndo->Add(UNDO_REPLACE, iFirst, ReplaceWord.Length(), oc, Temp);

    tae->SelTextW = ReplaceWord; // Replace in V_RTF
    Temp = dts->SL_IRC->Text;
    Temp = Temp.SubString(1, iFirst) + ReplaceWord +
                    Temp.SubString(iLast+1, Temp.Length()-iLast);
    dts->SL_IRC->Text = Temp;

    ReplaceCount++;

    utils->PopOnChange(tae);
  }

  ProcessNextWord();
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::ReplaceAllButtonClick(TObject *Sender)
{
  WideString ReplaceWord = WordEdit->TextW;
  WideString OrigWord = tae->SelTextW;

  if (tae->View == V_RTF && OrigWord.Length() > 0 && OrigWord != ReplaceWord)
  {
    // We have a word selected and want to replace all occurrances of that word.
    // After replacing all occurrances, we need to compensate SelStart by
    // the size-difference between the old and new word multiplied by the
    // number of occurrances of the old word BEFORE the current SelStart (where
    // the old word is highlighted and the user pressed "Replace All")
    int lenDiff = ReplaceWord.Length()-OrigWord.Length();
    int SaveSelStart = tae->SelStart;
    int OldCount = CountWord(0, SaveSelStart, OrigWord);
    SaveSelStart += (OldCount*lenDiff) + ReplaceWord.Length();

    // Call up our own find-replace
    try { if (ReplaceTextForm) ReplaceTextForm->Release(); } catch(...) {}
    Application->CreateForm(__classid(TReplaceTextForm), &ReplaceTextForm);
    if (ReplaceTextForm == NULL) return;
    int Count = ReplaceTextForm->FrReplaceAll(OrigWord, ReplaceWord);

    // Reload wText and set SelStart and CurrentIndex into wText to the
    // index where spell-checking needs to resume when ProcessNextWord()
    // is called.
    wText = utils->StripChar(tae->TextW, C_LF) + WideString(L' ');
    tae->SelStart = SaveSelStart;
    CurrentIdx = SaveSelStart + 1; // 1-based index!

    // Add new word to IgnoreAll list... (so we don't keep finding it if it's
    // not in the dictionary)
    IgnoreAll->Add(ReplaceWord);

    // Reset Cumulative offset (applied when setting SelStart in a normal word
    // substitution for a Replace...
    SelOffset = 0;

    // Tell the user what we did "X replacements made."
    if (Count != 0)
    {
      ReplaceCount += Count; // Add to total count
      utils->ShowMessageU(String(Count) + REPLACEMSG[25]); // x replacements made
    }

    // Set focus back to the main edit-control so the carat can blink
    // (if the replace-text is "nothing" you can't tell where we are...)
    tae->SetFocus();
  }

  ProcessNextWord();
}
//---------------------------------------------------------------------------
int __fastcall TSpellCheckForm::CountWord(int iStart, int iEnd, WideString sWord)
// Return the count of sWord up to iEnd but not including the word at iEnd.
{
  TSearchTypes2 Flags;

  // Setting WholeWord would not find hyphenated words!
  //Flags << st2WholeWord;
  //Flags << st2MatchCase;
  // st2Backward is set below...

  int Count = 0;
  int FoundPos = 0;
  int SearchLength = 0;

  for(;;)
  {
    // Search to end
    iStart = FoundPos+SearchLength;

    // New Search length (rest of text)
    SearchLength = -1;

    // Search...
    if (!tae->FindTextExW(sWord, iStart, Flags, FoundPos, SearchLength))
      break;

    if (FoundPos >= iEnd)
      break;

    Count++;
  };

  return Count;
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::IgnoreButtonClick(TObject *Sender)
{
// ignore
  ProcessNextWord();
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::IgnoreAllButtonClick(TObject *Sender)
{
  IgnoreAll->Add(WordEdit->TextW);
  ProcessNextWord();
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::AddButtonClick(TObject *Sender)
{
  if (dts->GDictList->Count < MAX_DICT_COUNT)
  {
    dts->GDictList->Add(WordEdit->TextW);

    // set changed flag
    if (!dts->MyDictChanged) dts->MyDictChanged = true;

    ProcessNextWord();
  }
  else // "Max custom dictionary entries is "
    utils->ShowMessageU(SPELLINGMSG[14] + String(MAX_DICT_COUNT));
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::DelButtonClick(TObject *Sender)
{
  Application->CreateForm(__classid(TShowDictForm), &ShowDictForm);

  // Returns 1 if items deleted
  ShowDictForm->ShowModal();
  bool bItemsDeleted = ShowDictForm->ItemsDeleted;
  ShowDictForm->Release();

  // Restart the search if any words deleted from My Dictionary
  if (bItemsDeleted) SpellCheck();
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::CloseButtonClick(TObject *Sender)
{
  Close(); // Causes Close...
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
  Release(); // Causes Destroy...
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::FormDestroy(TObject *Sender)
{
  // Clean up
  SpellCheckForm = NULL;
  try {HSUninit(pSpell);} catch(...) {}
  try {FreeLibrary(HSDLLHandle);} catch(...) {}
  try {delete IgnoreAll;} catch(...) {}
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::HelpButtonClick(TObject *Sender)
{
  utils->ShowMessageU(SPELLINGMSG[47]);
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::LanguageButtonClick(TObject *Sender)
{
  WideString SaveDict = dts->GDictionary;

  Application->CreateForm(__classid(TChooseDictForm), &ChooseDictForm);

  if (ChooseDictForm->NumDictionaries != 0)
  {
    ChooseDictForm->ShowModal();
    dts->GDictionary = ChooseDictForm->Dict;
  }
  else
    dts->GDictionary = ""; // No dictionary!

  ChooseDictForm->Release();

  if (dts->GDictionary.IsEmpty())
  {
    dts->GDictionary = utils->Utf8ToWide(DEF_DICTIONARY);
    utils->ShowMessageU(SPELLINGMSG[28]); // Instructions to find a dictionary...
    return;
  }

  // Don't reset spell-checking unless the dictionary changed!
  if (dts->GDictionary != SaveDict)
  {
    // Call InitDictionary() - it will find the files and not invoke
    // DlgChooseDict(). We need it to unload the old dictionary and load
    // the new...
    if (!InitDictionary())
      return;

    SpellCheck();
  }

}
//---------------------------------------------------------------------------
//void __fastcall TSpellCheckForm::WordEditChange(TObject *Sender,
//      DWORD dwChangeType, void *pvCookieData, bool &AllowChange)
void __fastcall TSpellCheckForm::WordEditChange(TObject *Sender)
{
  // User is editing word so provide real-time suggestions

  try
  {
    if (SuggestionListBox->Items->Count > 0)
      SuggestionListBox->Items->Clear();

    String CurrentWord = WordEdit->TextW;

    if (bUTF8)
      CurrentWord = utils->WideToUtf8(CurrentWord);

    // Check spelling
    if (!HSSpell(pSpell, CurrentWord.c_str()))
    {
      // Get suggestions
      char** strings = NULL;
      int iLen = HSSuggest(pSpell,
                    utils->WideToUtf8(WordEdit->TextW).c_str(), strings);

      if (strings != NULL)
      {
        if (iLen > 0)
        {
          //display
          for(int jj = 0; jj < iLen; jj++)
          {
            //display
            String OutWord;

            for(int jj = 0; jj < iLen; jj++)
            {
              OutWord = strings[jj];

              if (bUTF8)
                OutWord = utils->Utf8ToAnsi(OutWord);

              SuggestionListBox->Items->Add(OutWord);
            }
          }
        }

        //free the allocated memory
        HSSuggestFree(pSpell, strings, iLen);
      }
    }
  }
  catch(...) {}
}
//---------------------------------------------------------------------------
void __fastcall TSpellCheckForm::SuggestionListBoxDrawItem(
      TWinControl *Control, int Index, TRect &Rect, TOwnerDrawState State)
{
  if (!SuggestionListBox->Canvas->TryLock())
    return;

  WideString sOut;

  String sIn = SuggestionListBox->Items->Strings[Index];

  if (!sIn.IsEmpty())
    sOut = utils->Utf8ToWide(sIn);

  ::TextOutW(SuggestionListBox->Canvas->Handle, Rect.Left, Rect.Top,
                                                sOut.c_bstr(), sOut.Length());

  if (State.Contains(odFocused))
    SuggestionListBox->Canvas->DrawFocusRect(SuggestionListBox->ItemRect(Index));

  SuggestionListBox->Canvas->Unlock();
}
//---------------------------------------------------------------------------

