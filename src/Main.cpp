//---------------------------------------------------------------------------
// YahCoLoRiZe - Edit, process and send colorized text into IRC chat-rooms
// via various chat-clients such as mIRC, IceCHat and LeafCHat
//
// Author: Scott Swift
//
// Released to GitHub under GPL v3 October, 2016
//
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

#include "TrialKeyLinkIn.h"
#include "LicenseKey.h"
#include "ThreadOnChange.h"
#include "Play.h"
#include "Playback.h"
#include "Utils.h"
#include "ConvertToIRC.h"
#include "ConvertToRTF.h"
#include "ConvertToYahoo.h"
#include "ConvertToHTML.h"
#include "ConvertFromHTML.h"
#include "..\TaeRichEdit\TaePrintDialog.h"
#include "..\TaeRichEdit\TaePageSetupDlg.h"
#include "..\TaeRichEdit\TaeRichEditAdvPrint.h"
#include "TaePreviewFrm.h"
#include "PrintPreview.h"
//#include "..\TaeRichEdit\TaeRichEdit.h"

#include "DlgIncDec.h"
#include "DlgAlternate.h"
#include "DlgUnderline.h"
#include "DlgSetColors.h"
#include "DlgFavColors.h"
#include "DlgReplaceText.h"
#include "DlgPadSpaces.h"
#include "DlgMorph.h"
#include "DlgFontSize.h"
#include "DlgFontType.h"
#include "DlgSmiley.h"
#include "DlgLicense.h"
#include "DlgAbout.h"
#include "DlgMonkey.h"
#include "DlgPlay.h"
#include "DlgColor.h"
#include "DlgWingEdit.h"
#include "DlgBlend.h"
#include "DlgWebExport.h"
#include "DlgSpellCheck.h"
#include "FormOFSSDlg.h"
#include "FormSFDlg.h"
#include "Paltalk.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "TaeRichEdit"
#pragma link "TaePageSetupDlg"
#pragma link "TaePrintDialog"
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
// FYI: For multi-monitor stuff look up TMonitor, TScreen and TPosition...
// Example:
// int s = Screen->DesktopLeft;
// int MonNum = Screen->Monitor->MonitorNum;
//---------------------------------------------------------------------------

#if LICENSE_ON
KeyClass* PK;
#endif

TDTSColor* DTSColor;

// Default blender-button colors
const int BlendPreset0Init[MAXBLENDCOLORS] =
{ 0x00ccff,0x33ccff,0x66ccff,0x99ccff,0x9d9dff,
  0xff84ff,0xff99cc,0xff6699,0xff3366,0xff0033 };

const int BlendPreset1Init[MAXBLENDCOLORS] =
{ 0xff00ff,0xff00ff,0xcc00ff,0xcc00ff,0x9900ff,
  0x6600ff,0x3300ff,0x3300ff,0x0000ff,0x0000ff };

const int BlendPreset2Init[MAXBLENDCOLORS] =
{ 0xff9900,0xff9900,0xcc9900,0x999900,0x669900,
  0x339900,0x009900,0x00cc33,0x00ff66,0x00ff66 };

const int BlendPreset3Init[MAXBLENDCOLORS] =
{ 0xff0000,0xff7f00,0xffff00,0x00ff00,0x0000ff,
  0x4b0082,0x8f00ff,0x000000,0x000000,0x000000 };

const int DefaultPalette[MAXDEFAULTCOLORS] =
  { 0xffffff,0x000000,0x000080,0x008000,0xff0000,0x800000,0x800080,0xffa500,
    0xffff00,0x00ff00,0x008080,0x00ffff,0x0000ff,0xff00ff,0x808080,0xc0c0c0 };

/*
const int ExtendedPalette[MAXEXTENDEDCOLORS] =
  { 0x00008B, 0x008B8B, 0x00CED1, 0x00FF7F, 0x191970, 0x1E90FF, 0x20B2AA, 0x228B22,
    0x2E8B57, 0x2F4F4F, 0x32CD32, 0x3CB371, 0x40E0D0, 0x4169E1, 0x4682B4, 0x483D8B,
    0x48D1CC, 0x4B0082, 0x556B2F, 0x5F9EA0, 0x6495ED, 0x663399, 0x66CDAA, 0x696969,
    0x6A5ACD, 0x6B8E23, 0x778899, 0x7B68EE, 0x7FFF00, 0x7FFFD4, 0x808000, 0x87CEFA,
    0x8B008B, 0x8B4513, 0x8FBC8F, 0x9370DB, 0x9400D3, 0x98FB98, 0x9ACD32, 0xA0522D,
    0xA9A9A9, 0xADD8E6, 0xADFF2F, 0xB0C4DE, 0xB22222, 0xB8860B, 0xBA55D3, 0xBC8F8F,
    0xBDB76B, 0xC71585, 0xCD5C5C, 0xCD853F, 0xD2691E, 0xD3D3D3, 0xD8BFD8, 0xDA70D6,
    0xDAA520, 0xDB7093, 0xDC143C, 0xDCDCDC, 0xDEB887, 0xE0FFFF, 0xE9967A, 0xEEE8AA,
    0xF08080, 0xF0E68C, 0xF4A460, 0xF5DEB3, 0xFA8072, 0xFF1493, 0xFF4500, 0xFF69B4,
    0xFF7F50, 0xFFA07A, 0xFFB6C1, 0xFFD700, 0xFFDAB9, 0xFFDEAD, 0xFFE4C4, 0xFFE4E1,
    0xFFEFD5, 0xFFF8DC, 0xFFFAFA };
*/
//---------------------------------------------------------------------------
__fastcall TDTSColor::TDTSColor(TComponent* Owner)
  : TForm(Owner)
// Surprisingly, this executes after the main FormCreate!!!!!!
// AFTER InitAllSettings is called...
{
  // Try to register the IceChat, Hydra and Trinity message
  if ((RWM_ColorizeNet = RegisterWindowMessage(WMS_COLORIZENET)) == 0)
    utils->ShowMessageU(String(DS[1]));

  // Try to register the SwiftMix messages
  if ((RWM_SwiftMixTime = RegisterWindowMessage(WMS_SWIFTMIXTIME)) == 0 ||
    (RWM_SwiftMixPlay = RegisterWindowMessage(WMS_SWIFTMIXPLAY)) == 0 ||
    (RWM_SwiftMixState = RegisterWindowMessage(WMS_SWIFTMIXSTATE)) == 0)
    utils->ShowMessageU(String(DS[1]));

  // Try to register the WM_DataCoLoRiZe, WM_ChanCoLoRiZe (XiRCON/Trinity)
  if ((RWM_DataCoLoRiZe = RegisterWindowMessage(WMS_DATACOLORIZE)) == 0 ||
        (RWM_ChanCoLoRiZe = RegisterWindowMessage(WMS_CHANCOLORIZE)) == 0 ||
            (RWM_PlayCoLoRiZe = RegisterWindowMessage(WMS_PLAYCOLORIZE)) == 0)
    utils->ShowMessageU(String(DS[1]));

  // Try to register the WM_YahELiteExt (YahELite)
  if ((RWM_YahELiteExt = RegisterWindowMessage(FN[0])) == 0)
    utils->ShowMessageU(String(DS[1]));

  // Register clipboard formats
  cbHTML = (unsigned short)RegisterClipboardFormat(HTML_REGISTERED_NAME);
  cbRTF = (unsigned short)RegisterClipboardFormat(RTF_REGISTERED_NAME);
}
//----------------------------------------------------------------------------
bool __fastcall TDTSColor::GetUndoEnabled(void)
{
  return EditUndoItem->Enabled;
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::SetUndoEnabled(bool bValue)
{
  EditUndoItem->Enabled = bValue;
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::AppException(TObject *Sender, Exception *E)
// !!!!Handle disaster!!!!
{
  try
  {
    if (ThreadOnChange)
      ThreadOnChange->Terminate();;
  }
  catch(...) {}

  Application->ShowException(E);
  this->Release();
  Application->Terminate();
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::FormCreate(TObject *Sender)
// This Executes FIRST!!! In FormCreate - create all new objects... don't
// populate WideString, String or other VCL entities here - do that in FormShow.
// Apparently they are created AFTER this is called! Even the constructor
// is created after this...
{
  Application->OnException = AppException;

  try
  {
    // Init License-Key Properties
#if LICENSE_ON
    PK = new KeyClass();
#endif

    utils = new TUtils(this);

#if DEBUG_ON
    CInit();
#endif
  }
  catch(...)
  {
    MessageBox(Handle, "There was a problem creating TUtils!",
                                    "Warning", MB_ICONWARNING|MB_OK);
    this->Release();
    Application->Terminate();
    return;
  }

  // Needed to paint in titlebar for XP or menu-bar for Vista and up...
  // and to set PageControl1->ShowHint true/false
  bIsVistaOrHigher = IsWinVistaOrHigher();

#if SECURITY_ON
  // Object security
  SID sid;
  SECURITY_INFORMATION si;
  SECURITY_DESCRIPTOR sd;

  // Set security-identifier authority
  SID_IDENTIFIER_AUTHORITY sia = {SECURITY_LOCAL_SID_AUTHORITY};

  try
  {
    // Object security
    // Set the security identifier (SID)
    AllocateAndInitializeSid(&sia,1, SECURITY_LOCAL_RID,0,0,0,0,0,0,0,(PSID *)&sid);

    // Get the security-descriptor (SD) given the SID
    SetSecurityDescriptorGroup(&sd,&sid,true);

    // change our security to Local Access Only!
    SetUserObjectSecurity(Handle,&si,&sid);

    // change our security to Local Access Only!
    SetUserObjectSecurity(tae->Handle,&si,&sid);

    FreeSid(&sid);
  }
  catch(...)
  {
    MessageBox(Handle, "There was a problem setting object security!",
                                          "Warning", MB_ICONWARNING|MB_OK);
  }
#endif

  // Persistent handle used in Playback.cpp
  hDll = NULL;

  GPaltalk = NULL; // Pointer to TPalTalk (set in SetNewClient())

  bFreeze = false;
  bHaveSpeller = false;

  captionFG = IRCBLACK;
  captionBG = IRCAQUA;

  MouseBlendIndex = -1;

  // Table used for RTF color-table
  RGB_Palette = NULL;
  RGB_PaletteSize = 0;

  // Main color table
  palette = NULL;
  paletteSize = 0;
  paletteExtent = 0;

  // PaltalkMaxColors property loaded from Colorize.ini
  paltalkMaxColors = -1;

  DialogSaveMaxColors = -1;

  // temp vars used for incoming WM_COPYDATA messages
  wmCopyMode = CM_IDLE;
  wmCopyClient = -1;
  wmCopyCommand = -1; // Remote command
  wmCopyServer = -1; // Remote server ID
  wmCopyChannel = -1; // Remote channel ID
  wmCopyData = -1; // Remote data

  bIsColorFractal = false;
  bWordWrapWarningShown = false;
  bIsPlaying = false;
  bMyDictChanged = false;
  bTextWasProcessed = false;
  bSaveWork = false;
  bUnicodeKeySequence = false;
  bFileOpened = false,
  bFileSaved = false;

  HintTimer = NULL;
  HintWindow = NULL;

  GPlayIndex = 0;
  GPlayBuffer = NULL;
  GPlaySize = 0;
  GLineCount = 0;
  GPlayer = -1;
  GLineBurstSize = 0;
  GLineBurstCounter = 0;

  FCpTotalSections = 0;
  FCpMaxIterations = 0;
  FCpCurrentSection = 0;

  ColorCopyFg = NO_COLOR;
  ColorCopyBg = NO_COLOR;

  ProcessIRC = NULL;
  ProcessEffect = NULL;

  fG_BlendPreset = 0;
  bG_BlendPreset = 0;

  fG_BlendEngine = NULL;
  bG_BlendEngine = NULL;
  pBE = NULL;

  fG_BlendColors = NULL;
  bG_BlendColors = NULL;

  TotalWings = 0;

  // Sent by XiRCON
  RWM_ChanCoLoRiZe = 0;
  RWM_DataCoLoRiZe = 0;
  RWM_PlayCoLoRiZe = 0;

  // IceChat, Hydra and Trinity
  RWM_ColorizeNet = 0;

  // YahELite's WM_COPY
  RWM_YahELiteExt = 0;

  // Sent by SwiftMiX
  RWM_SwiftMixTime = 0;
  RWM_SwiftMixPlay = 0;
  RWM_SwiftMixState = 0;

  linesInRtfHeader = 0;
  orgLinePosition = 0;

  // handle of window to reply to, set when a message arrives
  GReplyTo = 0;

  bEffectWasAdded = false;
  bShiftHeldOnPlay = false;
  BlendButtonSelectIndex = -1;

  // Init pointers to Objects we will create below...
  SL_ORG = NULL;
  SL_HTM = NULL;
  SL_IRC = NULL;
  MS_RTF = NULL;
  g_rooms = NULL;
  GDictList = NULL;
  TOCUndo = NULL;
  Iftf = NULL;
  pluginClass = NULL;
  pluginName = NULL;
  LeftWings = NULL;
  RightWings = NULL;
  Emotes = NULL;

  // Create Streams
  try
  {
    // This string-list has info from text-file "smilies.txt"
    // such as web-site and e-mail address... 6 locations reserved
    Iftf = new TStringList();

    // List of recent chat-rooms for PlayDlg.cpp
    g_rooms = new TStringList();
    // only works if Sorted = true and I want a chronological list!
    // GRooms->Duplicates = dupIgnore; // don't permit duplicates

    // This is the user's custom dictionary (kept in the registry)
    GDictList = new TStringList(); // has utf-8 strings!
    GDictList->Sorted = true;
    GDictList->Duplicates = dupIgnore; // don't permit duplicates

    SL_ORG = new TStringsW();
    SL_IRC = new TStringsW();
    SL_HTM = new TStringsW;
    MS_RTF = new TMemoryStream();

    // Plugin class/name TStringList read from the .ini file
    pluginClass = new TStringList();
    pluginName = new TStringList();

    // Instantiate Undo class before TThreadOnChange and before anything
    // uses it...
    TOCUndo = new TTOCUndo();

    // Wingding string-lists (UTF-16)
    LeftWings = new TStringsW();
    RightWings = new TStringsW();
  }
  catch(...)
  {
    utils->ShowMessageU("Error creating basic objects in FormCreate!!!");
    this->Release();
    Application->Terminate();
    return;
  }

  // Create the OnChange processing Thread
  ThreadOnChange = new TThreadOnChange();
  ThreadOnChange->OnTerminate = &ThreadTerminated;
  //tpIdle  The thread executes only when the system is idle-Windows
  //  won't interrupt other threads to execute a thread with tpIdle priority.
  //tpLowest  The thread's priority is two points below normal.
  //tpLower  The thread's priority is one point below normal.
  //tpNormal  The thread has normal priority.
  //tpHigher  The thread's priority is one point above normal.
  //tpHighest  The thread's priority is two points above normal.
  //tpTimeCritical  The thread gets highest priority.
  ThreadOnChange->Priority = tpNormal;

  if (!pluginName || !pluginClass || !GDictList || !g_rooms || !TOCUndo ||
     !ThreadOnChange || !Iftf || !SL_ORG || !SL_IRC || !SL_HTM || !MS_RTF)
  {
    utils->ShowMessageU("Unable to create string-lists...");
    this->Release();
    Application->Terminate();
    return;
  }

#if LICENSE_ON
  TLicenseKey* lk = new TLicenseKey();
  lk->ValidateLicenseKey(false);
  delete lk;
#endif

}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FormShow(TObject *Sender)
// When you run from the command prompt, you can specify
// a file path and name to open or /uninstall will uninstall it
{
  if (tae == NULL || utils == NULL || ThreadOnChange == NULL)
  {
    utils->ShowMessageU("Null pointers in FormShow()!!!!");
    this->Release();
    Application->Terminate();
    return;
  }

  BorderIcons = (TBorderIcons() << biSystemMenu << biMaximize << biMinimize);

  this->Caption = OUR_TITLE_S; // Window-name used by YahELite

  // Minimized caption
  // (Note: it's possible to make the caption Unicode - I do it in SwiftMiX
  // for C++ Builder in showing song-titles - but we don't need it for
  // "YahCoLoRiZe" :-)
  Application->Title = OUR_NAME_S + String(" ") + String(REVISION);

  Application->HelpFile = HELPNAME;

  Application->HintColor = clAqua;
  Application->HintPause = 500;
  Application->HintHidePause = 4000;
  Application->ShowHint = true;

  // Access this as property "DeskDir"
  wDeskDir = utils->GetSpecialFolder(CSIDL_DESKTOPDIRECTORY);

  // Select Process tab...
  PageControl1->ActivePage = TabControl;

  utils->SetOldLineVars(tae, true); // Init line vars

  // Set "Undo" limit OFF (we do our own undo)
  tae->ClearUndo();
  tae->UndoLimit = 0;
  EditUndoItem->Enabled = false; // Set true via property UndoEnabled

  // I modified TaeRichEdit to maintain a FScrollBars var which is used
  // when it adjusts ScrollBars for wordwrap... so this sets both
  // FScrollBars and ScrollBars...
  tae->SetScrollBars(ssBoth);

  // This is only for viewing - we can't edit in word-wrap mode without
  // rewriting everything. The rich-edit control "line" in word-wrap
  // includes the soft and hard line-breaks - and we need just the hard-line
  // count at the cursor to index into the SL_IRC stream. The rich-edit
  // SelIndex includes each cr/lf as ONE. That would be great if we
  // stored everything internally with just a LF, but all things including
  // TStringList use cr\lf and so does TaeRichEdit in streaming in/out...
  // So we are hosed! DON'T ALLOW EDITING in word-wrap!
  WordWrap->Enabled = true;

// seems to init ok on its own...
//  tae->InsertMode = false; // false turns it on???

  // This messes up the Borland source-debugger!
#if !DEBUG_ON && VALIDATE_ON
  DisplaySecurityCode1->Enabled = true; // Enable menu-item

  if (!CompareSecurityCode())
    tae->Color = DTS_GRAY;
  else
    tae->Color = taeWindowColor;
#else
  tae->Color = taeWindowColor;
#endif

  // Do this after instantiating TThreadOnChange since it instantiates
  // TTOCUndo which is called from ClearAndFocus()
  InitAllSettings(false);

  ThreadOnChange->Resume(); // Start background processing thread

  // Used to prevent the TaeEdit OnChange event from
  // Firing when we are processing or loading text
  // Uses the Edit Control's Tag property as a LockCounter!
  // Enable OnChange Event last
  utils->InitOnChange(tae, TaeEditChange);

  // Re-paint min interval for title-bar
  RepaintTimer->Interval = TIMER3TIME;

  // Start processing custom windows messages
  OldWinProc = WindowProc;
  WindowProc = CustomMessageHandler;

  //enable drag&drop files
  DragAcceptFiles(this->Handle,true);

  // THIS IS A PAIN TO FIND (so here are some keywords to search for...)
  // command-line args, command-line parameters, argv, argc
  if (ParamCount() >= 1 && ParamStr(1).Length() > 0)
  {
    if (ParamStr(1).LowerCase() == "/uninstall")
    {
      //UninstallFromRegistry1Click(0);
    }
    else
    {
      // Process a file-path passed via command-args using a timer-event...
      WmCopyStr = ExpandFileName(ParamStr(1));
      wmCopyMode = CM_REMOTE_COMMAND;
      wmCopyCommand = REMOTE_COMMAND_FILE;
      wmCopyData = 0; // 0 = don't play the file, 1 = play the file
      Timer2->Enabled = true; // Go process
    }
  }
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::InitAllSettings(bool RestoreFactory)
{
  if (Iftf != NULL)
  {
    Iftf->Clear();

    // Init
    for (int ii = 0; ii < IFTF_SIZE; ii++)
      Iftf->Add("");

    // Begin with defaults in case we have an error and return
    Iftf->Strings[INFO_WEB_SITE] = DEF_WEBSITE;
    Iftf->Strings[INFO_EMAIL] = DEF_EMAIL;

    // we replace "artist album song" with the actual info from SwiftMiX
    // c or C xx,xx sets fg/bg color (can be RGB C#rrggbb or C$rrggbb)
    Iftf->Strings[INFO_SONG_FORMAT] = DEF_SONGFORMAT;
  }

  // Load emoticons
  ReadSmileys();

  // We need to see if CSIDL_APPDATA (Users\(user)\AppData\Roaming)\Discrete-Iime Systems\YahCoLoRiZe)
  // has our ini file and dictionaries, if not - copy over the default files from
  // the install folder...
  CopyIniFile();

  // Load Iftf strings...
  // Also loads Palette. In GetRegInfo below we will overwrite the first 16 Palette values...
  ReadIniFile(); // Loads colors, web-site, e-mail, SwiftMiX format, Etc.

  // Build palette used to render blended colors in the RGB view
  fG_BlendColors = new BlendColor[MAXBLENDCOLORS]; // Colors for blend buttons
  bG_BlendColors = new BlendColor[MAXBLENDCOLORS]; // Colors for blend buttons

  // Do this before calling GetRegInfo()!
  bHaveSpeller =
    utils->FileExistsW(utils->ExtractFilePathW(utils->GetExeNameW()) +
             WideString("\\") + U16(SPELLINGMSG[35])) ? true : false;

  // Do this after we read the PaletteExtent and the web-site from
  // the ini file (above)
  GetRegInfo(RestoreFactory);

  // The paletteSize is the number of colors we actually will use
  // (it varies with ExtendPalette->Checked) Do this after we read
  // ExtendPalette->Checked from the registry
  paletteSize = ExtendPalette->Checked ? paletteExtent : MAXDEFAULTCOLORS;
  // Enable ExtendPalette checkbox only if extended colors were read...
  ExtendPalette->Enabled = paletteExtent <= MAXDEFAULTCOLORS ? false : true;

  if (!ExtendPalette->Enabled)
    ExtendPalette->Checked = false;

  // Set Client's size
  Width = ClientWidth;
  Height = ClientHeight;

  // Init Edit-Boxes
  LMarginEdit->Text = String(regLmargin);
  RMarginEdit->Text = String(regRmargin);
  TMarginEdit->Text = String(regTmargin);
  BMarginEdit->Text = String(regBmargin);
  IndentEdit->Text = String(regIndent);

  WidthEdit->Text = String(regWidth);
  HeightEdit->Text = String(regHeight);
  SpacingEdit->Text = String(regSpacing);

  TabsEdit->Text = String(regTabs);

  // When a \t is encountered in pasted text, space is generated up to the
  // next stop.
  tae->TabWidth = regTabs;
  tae->TabCount = -1; // max tabs

  // Auto-Linesize?
  if (AutoWidth->Checked)
    WidthEdit->Enabled = false;
  else
    WidthEdit->Enabled = true;

  // Auto-Height?
  if (AutoHeight->Checked)
    HeightEdit->Enabled = false;
  else
    HeightEdit->Enabled = true;

  // Clear the saved mouse-wheel-blend
  MouseBlendIndex = -1;

  // Nothing selected
  BlendButtonSelectIndex = -1;

  BlendScopeRadioButtons->Visible = false;
  BlendDirectionRadioButtons->Visible = false;
  BlendExtensionPanel->Visible = false;
  PageControl1->ActivePage->ShowHint = false;

  // Init random number generator
  randomize();

  InitMainMenu();

  // Set View menu checkmarks and enable/disable
  tae->View = V_OFF;
  SetView(tae->View);

  // Configure for Yahoo or IRC based on Cli (read this from registry
  // and init the main menu prior to calling SetNewClient()!
  SetNewClient(Cli); // Set the client
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::InitMainMenu(void)
{
  // COLORCOPYPASTEMENU[0]
  try
  {
    for (int ii = 0; ii < ColorCopyPasteMenu->Items->Count; ii++)
      ColorCopyPasteMenu->Items->Items[ii]->Caption = COLORCOPYPASTEMENU[ii];
  }
  catch(...){}

  // DefaultStrings.cpp MAINPOPUPMENU[]

  // Pop-Up Menu [0]
  try
  {
    for (int ii = 0; ii < PopupMenu1->Items->Count; ii++)
      PopupMenu1->Items->Items[ii]->Caption = MAINPOPUPMENU[ii];
  }
  catch(...){}

  int li = 0; // DefaultStrings.cpp MAINMENU[]

  // Main Menu [0]
  try
  {
    if (MainMenu->Items->Count > 0)
    {
      for (int ii = 0; ii < MainMenu->Items->Count; ii++)
      {
        // File, Tools, View, etc...
        String sCaption = MAINMENU[li++];
        MainMenu->Items->Items[ii]->Caption = sCaption;
        if (MainMenu->Items->Items[ii]->Count > 0)
        {
          for (int jj = 0; jj < MainMenu->Items->Items[ii]->Count; jj++)
          {
            // Add the handler if this is a Client menu client-item
            if (sCaption.LowerCase() == "client")
              MainMenu->Items->Items[ii]->Items[jj]->OnClick = MenuClientClick;

            // Open, Save, Save As, etc...
            MainMenu->Items->Items[ii]->Items[jj]->Caption = MAINMENU[li++];

            if (MainMenu->Items->Items[ii]->Items[jj]->Count > 0)
              for (int kk = 0; kk < MainMenu->Items->Items[ii]->Items[jj]->Count; kk++)
                // Mountain, Valley, etc...
                MainMenu->Items->Items[ii]->Items[jj]->Items[kk]->Caption = MAINMENU[li++];
          }
        }
      }
    }
  }
  catch(...){}

  // Set Blender-tab items...
  for (int ii = 0; ii < PageControl1->PageCount; ii++)
    PageControl1->Pages[ii]->Caption = TABITEMS[ii];
  for (int ii = 0; ii < BlendScopeRadioButtons->Items->Count; ii++)
    BlendScopeRadioButtons->Items->Strings[ii] = BLENDSCOPEITEMS[ii];
  for (int ii = 0; ii < BlendDirectionRadioButtons->Items->Count; ii++)
    BlendDirectionRadioButtons->Items->Strings[ii] = BLENDDIRECTIONITEMS[ii];

  // Right-click in title-bar to set the colors you see!
  if (bIsVistaOrHigher)
    PageControl1->ShowHint = false;
  else
    PageControl1->ShowHint = true;

  PageControl1->Hint = DS[198];

  // Blender-Tab Hint (Used for color-buttons)
  tsBlender->Hint = DS[197];

  // Set Checkbox captions...
  LeftJustify->Caption = DS[199];
  PackText->Caption = DS[200];
  PackLines->Caption = DS[202];
  Borders->Caption = DS[203];
  Wingdings->Caption = DS[204];
  AutoDetectEmotes->Caption = DS[205];
  PlayOnCR->Caption = DS[206];
  SwiftMixSync->Caption = DS[207];
  ExtendPalette->Caption = DS[208];
  ImportBlenderLabel->Caption = DS[209];
  RGBThresholdLabel->Caption = DS[210];

  // Tabs tab
  TabsLabel->Caption = DS[232];
  TabsToSpaces->Caption = DS[233]; // Checkbox
  TabsToSpaces->Hint = DS[234]; // Checkbox Hint
  TabsToSpaces->ShowHint = true;
  TabsFlattenRB->Caption = DS[235]; // RadioButton
  TabsFlattenRB->Hint = DS[236];
  TabsFlattenRB->ShowHint = true;
  TabsStripRB->Caption = DS[237]; // RadioButton
  TabsStripRB->Hint = DS[238];
  TabsStripRB->ShowHint = true;

  PlayOnCX->Caption = DS[225];

  WingColorLabel->Caption = DS[220];

  // Import ASCII blend-color button
  ImportAsciiButton->Hint = DS[2];

  // Pack Lines
  PackLines->Hint = DS[5];

  // Pack Text
  PackText->Hint = DS[6];

  // Left Justify
  LeftJustify->Hint = DS[7];

  // Extended Palette
  ExtendPalette->Hint = DS[8];

  // Process Tab
  TabControl->Hint = DS[9];

  BlendScopeRadioButtons->Hint = DS[10];
  BlendDirectionRadioButtons->Hint = DS[11];
  BlendExtensionPanel->Hint = DS[12];
  ColorBlenderPresetPanel->Hint = DS[15];
  ImportButton->Hint = DS[16];
  Borders->Hint = DS[17];
  Wingdings->Hint = DS[18];
  AutoDetectEmotes->Hint = DS[19];
  PlayOnCR->Hint = DS[20];
  PlayOnCX->Hint = DS[34];
  SwiftMixSync->Hint = DS[196];
  LeftJustify->Hint = DS[21];
  ProcessButton->Hint = DS[27];
  ShowButton->Hint = DS[244];
  PlayButton->Hint = DS[28];
  StopButton->Hint = DS[29];
  ResumeButton->Hint = DS[30];
  FgPanel->Hint = DS[31];
  BgPanel->Hint = DS[32];
  BlendColorPanel->Hint = DS[32];
  WingColorPanel->Hint = DS[33];

  // StatusBar
  StatusBar->Hint = DS[100];
  StatusBar->Panels->Items[SB_UNUSED]->Text = ""; // empty panel for now!

  AutoWidth->Caption = DS[245];
  AutoWidth->Hint = DS[246];

  WidthLabel->Caption = DS[247]; // LineWidthLabel Caption
  WidthLabel->Hint = DS[248]; // LineWidthLabel Hint
  WidthEdit->Hint = DS[248]; // WidthEdit Hint

  LMarginLabel->Caption = DS[249]; // MarginLabel Caption
  LMarginLabel->Hint = DS[250]; // MarginLabel Hint
  LMarginEdit->Hint = DS[250]; // MarginEdit Hint

  IndentLabel->Caption = DS[251]; // IndentLabel Caption
  IndentLabel->Hint = DS[252]; // IndentLabel Hint
  IndentEdit->Hint = DS[252]; // IndentEdit Hint

  RMarginLabel->Caption = DS[253]; // MarginLabel Caption
  RMarginLabel->Hint = DS[254]; // MarginLabel Hint
  RMarginEdit->Hint = DS[254]; // MarginEdit Hint

  AutoHeight->Caption = DS[255];
  AutoHeight->Hint = DS[256];

  HeightLabel->Caption = DS[257];
  HeightLabel->Hint = DS[258];
  HeightEdit->Hint = DS[258];

  SpacingLabel->Caption = DS[259];
  SpacingLabel->Hint = DS[260];
  SpacingEdit->Hint = DS[260];

  TMarginLabel->Caption = DS[261]; // MarginLabel Caption
  TMarginLabel->Hint = DS[262]; // MarginLabel Hint
  TMarginEdit->Hint = DS[262]; // MarginEdit Hint

  BMarginLabel->Caption = DS[263]; // MarginLabel Caption
  BMarginLabel->Hint = DS[264]; // MarginLabel Hint
  BMarginEdit->Hint = DS[264]; // MarginEdit Hint

  Margins->Caption = DS[265]; // Margins On/Off Checkbox
  Margins->Hint = DS[48]; // Margins On/Off Hint
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FormDestroy(TObject *Sender)
{
  try
  {
    utils->PushOnChange(tae);

    utils->WaitForThread();

    if (ThreadOnChange)
      ThreadOnChange->Terminate(); // Free on terminate is set

    // Delete objects
    if (TOCUndo) delete TOCUndo;
    if (SL_ORG) delete SL_ORG;
    if (SL_HTM) delete SL_HTM;
    if (SL_IRC) delete SL_IRC;
    if (MS_RTF) delete MS_RTF;
    if (utils) delete utils;
    if (Iftf) delete Iftf;
    if (g_rooms) delete g_rooms;
    if (GDictList) delete GDictList;
    if (pluginClass) delete pluginClass;
    if (pluginName) delete pluginName;
    if (PrintPreview) delete PrintPreview;

    if (palette) delete [] palette;

    // Free Colorize.dll if needed
    FreeLibrary(hDll);

#if DEBUG_ON
    FreeConsole();
#endif
  }
  catch(...)
  {
    MessageBox(Handle, "There was a problem deleting objects "
              "in FormDestroy()!", "Warning", MB_ICONWARNING|MB_OK);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FormCloseQuery(TObject *Sender, bool &CanClose)
{
#if DEBUG_ON
  CWrite("\r\nFormCloseQuery() cFont: " + String(cFont) + "\r\n");
#endif

  CanClose = true;

  if (SL_IRC == NULL || SL_ORG == NULL)
    return;

  if (bSaveWork && (SL_IRC->Count > 0 ||
                 (tae->View == V_ORG && SL_ORG->Count > 0)))
  {
    // "Unsaved work, close anyway?"
    if (utils->ShowMessageU(Handle, DS[228],
                MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDNO)
    {
      CanClose = false;
      return;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FormClose(TObject *Sender, TCloseAction &Action)
{
  try
  {
    // Add defaults to registry
    SaveToRegistry();
    WriteDictToIniFile(); // Save MyDictionary

    // See if user had been editing colors...
    SaveWorkingBlendPreset();

    if (fG_BlendEngine) delete fG_BlendEngine;
    fG_BlendEngine = NULL;
    if (bG_BlendEngine) delete bG_BlendEngine;
    bG_BlendEngine = NULL;

    if (fG_BlendColors) delete [] fG_BlendColors;
    fG_BlendColors = NULL;
    if (bG_BlendColors) delete [] bG_BlendColors;
    bG_BlendColors = NULL;

    if (LeftWings) delete LeftWings;
    LeftWings = NULL;
    if (RightWings) delete RightWings;
    RightWings = NULL;
    if (Emotes != NULL) delete Emotes;
    Emotes = NULL;

    // Close and delete open forms (VCL only does this for us if it created
    // the forms in DTSColor.cpp - which we disabled in Options->Forms tab...
    YcDestroyForm(PlayForm);
    YcDestroyForm(AboutForm);
    YcDestroyForm(WingEditForm);
    YcDestroyForm(ReplaceTextForm);
    YcDestroyForm(MonkeyForm);
    YcDestroyForm(BlendChoiceForm);
    YcDestroyForm(SetColorsForm);
    YcDestroyForm(AlternateForm);
    YcDestroyForm(IncDecForm);
    YcDestroyForm(UnderlineForm);
    YcDestroyForm(FontSizeForm);
    YcDestroyForm(FontTypeForm);
    YcDestroyForm(FavColorsForm);

    if (GPaltalk)
      delete GPaltalk;

    GPaltalk = NULL;
  }
  catch(...)
  {
#if DEBUG_ON
    DTSColor->CWrite("\r\nTDTSColor::FormClose(): Exception!\r\n");
#endif
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FormActivate(TObject *Sender)
{
  SetForegroundWindow(this->Handle);

  if (WindowState != wsMinimized)
    tae->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FormMouseWheel(TObject *Sender, TShiftState Shift,
                                int WheelDelta, TPoint &MousePos, bool &Handled)
{
  static bool bPreviousWheelForward;

  if (PageControl1->ActivePage != tsBlender || !fG_BlendColors ||
      BlendButtonSelectIndex < 0 ||
         !fG_BlendColors[BlendButtonSelectIndex].enabled)
  {
    Handled = false;
    return;
  }

  BlendColor TC = fG_BlendColors[BlendButtonSelectIndex];

  // Save original for restore with a press
  // of the mouse-wheel...
  if (MouseBlendIndex < 0)
  {
    // Save the initial color
    MouseBlendColor = TC;
    MouseBlendIndex = BlendButtonSelectIndex;
  }
  else // Restore color if mouse direction changed
  {
    if ((WheelDelta < 0 && bPreviousWheelForward) || (WheelDelta > 0 && !bPreviousWheelForward))
    {
      fG_BlendColors[BlendButtonSelectIndex] = MouseBlendColor;
      MouseBlendIndex = -1;

      // Get a pointer to the selected panel...
      TPanel* P = GetPointerToColorPanel(BlendButtonSelectIndex);

      P->Color = utils->BlendColorToTColor(fG_BlendColors[BlendButtonSelectIndex]);

      Handled = true;
      return;
    }
  }

  // Set prev. direction...
  if (WheelDelta < 0) bPreviousWheelForward = false;
  else bPreviousWheelForward = true;

  TC.red += (WheelDelta/120) * 5;
  TC.green += (WheelDelta/120) * 5;
  TC.blue += (WheelDelta/120) * 5;

  // Limits
  if (TC.red < 0) TC.red = 0;
  if (TC.green < 0) TC.green = 0;
  if (TC.blue < 0) TC.blue = 0;
  if (TC.red > 255) TC.red = 255;
  if (TC.green > 255) TC.green = 255;
  if (TC.blue > 255) TC.blue = 255;

  if (MouseBlendIndex >= 0 && (TC.red == 0 && TC.green == 0 && TC.blue == 0) || (TC.red == 255 && TC.green == 255 && TC.blue == 255))
    // Restore the color
    fG_BlendColors[BlendButtonSelectIndex] = MouseBlendColor;
  else
    fG_BlendColors[BlendButtonSelectIndex] = TC;

  // Get a pointer to the selected panel...
  TPanel* P = GetPointerToColorPanel(BlendButtonSelectIndex);

  P->Color = utils->BlendColorToTColor(fG_BlendColors[BlendButtonSelectIndex]);

  Handled = true;
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::CustomMessageHandler(TMessage &msg)
{
  // ReplyMessage(0) not needed since we handle lengthy
  // operations via timer-delay from another handler...
  try
  {
    // Intercept the WM_NCPAINT message
    if (msg.Msg == WM_NCPAINT || msg.Msg == WM_NCACTIVATE)
    {
      OldWinProc(msg); // Call main handler first

      // delay to prevent re-painting too often...
      if (!RepaintTimer->Enabled && msg.WParam != NULL)
        RepaintTimer->Enabled = true;

      return;
    }

    // Intercept the WM_COPYDATA message, msg.WParam has sender's handle
    if (msg.Msg == WM_COPYDATA)
    {
        if (!msg.WParam || !msg.LParam || wmCopyMode != CM_IDLE)
        {
          OldWinProc(msg); // Call main handler and leave...
          return;
        }

        bIsColorFractal = false;

        MYCOPYDATASTRUCT* cds = (MYCOPYDATASTRUCT *)msg.LParam;

        // cbData within the COPYDATASTRUCT has size of the
        // data object pointed to by msg.LParam
        int len = (unsigned int)cds->cbData;

        // limit data max to 10000 bytes
        if (len > MAX_WM_COPY_LENGTH || len < 1 || !cds->lpData)
        {
          OldWinProc(msg); // Call main handler and leave...
          return;
        }

        // Firing timer event to handle processing frees the
        // other application's calling thread...
        Timer2->Interval = TIMER2TIME;

        if (cds->dwData == RWM_SwiftMixState) // System integer representation
        {
          // "Play" is never received here...
          // 1 = stop, 2 = pause, 3 = resume or play, 10 = song loaded
          typedef struct
          {
            __int32 player;
            __int32 state;
          } SWIFTMIX_STRUCT;

          SWIFTMIX_STRUCT* sms = (SWIFTMIX_STRUCT *)cds->lpData;

          WmPlayer = sms->player;
          WmState = sms->state;

          if (SwiftMixSync->Checked)
          {
            wmCopyMode = CM_SWIFTMIX_STATE;
            Timer2->Enabled = true; // Go process and send the line
          }
        }
        else if (cds->dwData == RWM_SwiftMixPlay) // System integer representation
        {
          // "Play" state-change is received here...
          typedef struct
          {
            __int32 player;
            __int32 duration;
            __int32 len_path;
            __int32 len_name;
            __int32 len_artist;
            __int32 len_album;
            char path[4096];
            char name[1024];
            char artist[1024];
            char album[1024];
          } SWIFTMIX_STRUCT;

          SWIFTMIX_STRUCT* sms = (SWIFTMIX_STRUCT *)cds->lpData;

          WmPlayer = sms->player;
          WmDuration = sms->duration;

          // All in UTF-8!
          if (sms->len_path > 4096-1)
            sms->len_path = 4096-1;
          WmPath = String((char *)sms->path, sms->len_path);
          if (sms->len_name > 1024-1)
            sms->len_name = 1024-1;
          WmName = String((char *)sms->name, sms->len_name);
          if (sms->len_artist > 1024-1)
            sms->len_artist = 1024-1;
          WmArtist = String((char *)sms->artist, sms->len_artist);
          if (sms->len_album > 1024-1)
            sms->len_album = 1024-1;
          WmAlbum = String((char *)sms->album, sms->len_album);
          PopupProcessSongTitle->Enabled = true; // Enable item in popup menu

          if (SwiftMixSync->Checked)
          {
            wmCopyMode = CM_SWIFTMIX_PLAY;
            Timer2->Enabled = true; // Start synchronized text-play to chat-room via Timer2 handler
          }
        }
        else if (cds->dwData == RWM_SwiftMixTime) // System integer representation
        {
          typedef struct
          {
            __int32 player;
            __int32 duration;
            __int32 current;
            __int32 color;
            __int32 redtime;
            __int32 yellowtime;
          } SWIFTMIX_STRUCT;

          SWIFTMIX_STRUCT* sms = (SWIFTMIX_STRUCT *)cds->lpData;

          WmPlayer = sms->player;
          WmDuration = sms->duration;
          WmCurrent = sms->current;
          WmColor = sms->color;
          WmRedtime = sms->redtime;
          WmYellowtime = sms->yellowtime;

          if (SwiftMixSync->Checked)
          {
            wmCopyMode = CM_SWIFTMIX_TIME;
            Timer2->Enabled = true; // Go process and send the line
          }
        }
        // Received text from YahELite
        else if (cds->dwData == RWM_YahELiteExt)
        {
          if (IsCli(C_YAHELITE) && PlayOnCX->Checked)
          {
            // Save the Sender's handle
            GReplyTo = (HWND)msg.WParam;

            WmCopyStr = String((char*)cds->lpData, (unsigned int)len);

            wmCopyMode = CM_PROCESS_RX_DATA;
            wmCopyData = -1;
            wmCopyClient = 1; // permanent ID for YahELite
            Timer2->Enabled = true; // Go process and send the line
          }
        }
        // Received text from IceChat, LeafChat, HydraIRC or Trinity
        // Either Remotely command YahCoLoRiZe to do something...
        // or just process the text and return it.
        else if (cds->dwData == RWM_ColorizeNet)
        {
          if (PlayOnCX->Checked && WmCopyStr.IsEmpty())
          {
// this was cool - used named, mapped memory - but I ended up figuring
// out how to avoid it on the LeafChat side...
//            if (len != sizeof(COLORIZENET_STRUCT))
//            {
//              // This must be named, mapped memory marshaled from my
//              // JAVA LeafChat Plugin... len will be the length of
//              // the name-string for the file-mapped memory...
//              String MemName = String((char *)cds->lpData, len);
//
//              // Here we manually have to get data that WM_COPYDATA normally
//              // handles for us in a windows system. But the Java VM's "JNA"
//              // can't properly handle a WM_COPYDATA so on the Java side we
//              // write COLORIZENET_STRUCT to a memory-mapped file and pass
//              // the ascii name-string in the COPYDATA struct instead... it
//              // seems to pass a small string ok... go figure!
//              cds->lpData = GetMappedMemory(MemName,
//                                        sizeof(COLORIZENET_STRUCT));
//            }

            // Struct size is in cbData but it could vary,
            // sizeof(COLORIZENET_STRUCT) is 2596 but I add an initialization
            // constructor (Main.h) which seems to add 4 bytes at the end...
            // So leave it open to anything reasonable...
            if (cds->cbData <= sizeof(COLORIZENET_STRUCT)+4)
            {
              COLORIZENET_STRUCT* cns = (COLORIZENET_STRUCT*)cds->lpData;

              // Save the Sender's window-handle
              GReplyTo = (HWND)msg.WParam;

              wmCopyCommand = cns->commandID;

              wmCopyServer = cns->serverID;
              wmCopyChannel = cns->channelID;
              wmCopyClient = cns->clientID;

              // Assume the timer handler should process this as
              // a remote command (like /cplay start) unless the
              // data-string is present and the remote client sent
              // us REMOTE_COMMAND_TEXT with the intent for us to
              // process the text and send it back, which is a
              // different timer-handler sub-function.
              if (wmCopyCommand == REMOTE_COMMAND_TEXT)
                wmCopyMode = CM_PROCESS_RX_DATA;
              else
                wmCopyMode = CM_REMOTE_COMMAND;

              // a newer UTF-8 plugin sets this to 1 when wmCopyCommand
              // is REMOTE_COMMAND_TEXT
              wmCopyData = cns->commandData;

              // cns->lenData length of string in bytes (UTF-8 or ANSI chars)
              if (cns->data && cns->lenData > 0 && cns->lenData <= CNS_DATALEN-1)
                WmCopyStr = String((char*)cns->data, cns->lenData);
              else
                WmCopyStr = "";

              // cns->lenChanNick length of string bytes (UTF-8 or ANSI chars)
              if (cns->chanNick && cns->lenChanNick > 0 &&
                                        cns->lenChanNick <= CNS_CHANNICKLEN-1)
                WmChanNickStr = String((char*)cns->chanNick, cns->lenChanNick);
              else
                WmChanNickStr = "";

              Timer2->Enabled = true; // Go process
            }
          }
        }
        // Remotely change play-channel
        else if (cds->dwData == RWM_ChanCoLoRiZe)
        {
          if (PlayOnCX->Checked)
          {
            // len has buffer size so subtract 1 when converting
            // to a string!
            if (len > 1)
            {
              // Save the Sender's handle
              GReplyTo = (HWND)msg.WParam;
              AddRoom(String((char*)cds->lpData, (unsigned int)len-1));
            }
          }
        }
        // Remotely sent start, stop, pause or resume
        else if (cds->dwData == RWM_PlayCoLoRiZe)
        {
          if (PlayOnCX->Checked)
          {
            // len has buffer size so subtract 1 when converting
            // to a string!
            if (len > 1)
            {
              // Don't care...
              wmCopyClient = -1;
              wmCopyServer = -1;
              wmCopyChannel = -1;
              wmCopyData = -1;
              WmChanNickStr = "";

              WmCopyStr =
                String((char*)cds->lpData, (unsigned int)len-1).LowerCase();

              if (WmCopyStr == "start")
                wmCopyCommand = REMOTE_COMMAND_START;
              else if (WmCopyStr == "stop")
                wmCopyCommand = REMOTE_COMMAND_STOP;
              else if (WmCopyStr == "pause")
                wmCopyCommand = REMOTE_COMMAND_PAUSE;
              else if (WmCopyStr == "resume")
                wmCopyCommand = REMOTE_COMMAND_RESUME;
              else // may be a file-name
              {
                // NOTE: XIRCON will be the client sending this - and it will
                // never be sent because the DLL internally will open and play
                // the file - but "in-theory" we could handle it!
                wmCopyCommand = REMOTE_COMMAND_FILE;
                wmCopyData = 1; // 0 = don't play the file, 1 = play the file
              }

              GReplyTo = (HWND)msg.WParam; // Save the Sender's handle
              wmCopyMode = CM_REMOTE_COMMAND;

              Timer2->Enabled = true; // Go process
            }
          }
        }
        // Received text from XiRCON (/cx command) via our custom dll
        else if (cds->dwData == RWM_DataCoLoRiZe)
        {
          if (PlayOnCX->Checked && WmCopyStr.IsEmpty())
          {
            // Save the Sender's handle
            GReplyTo = (HWND)msg.WParam;
            WmCopyStr = String((char*)cds->lpData, (unsigned int)len);
            wmCopyMode = CM_PROCESS_RX_DATA;
            wmCopyData = -1;
            WmChanNickStr = ""; // don't add a room!
            wmCopyClient = 7; // Permanent XiRCON ID
            Timer2->Enabled = true; // Go process and send the line
          }
        }

      msg.Result = true; // We handled it
    }
    else if (msg.Msg == WM_NCLBUTTONDBLCLK && msg.WParam != NULL)
    {
      if (!Timer2->Enabled)
      {
        wmCopyMode = CM_NCDOUBLECLICK;
        Timer2->Enabled = true; // Go open web-site
      }
    }
    else if (msg.Msg == WM_NCRBUTTONDOWN)
    {
      if (!Timer2->Enabled)
      {
        wmCopyMode = CM_NCRIGHTCLICK;
        Timer2->Enabled = true; // Go set colors in title-bar
      }
    }
    // Shift + ALT
    else if (msg.Msg == WM_NCLBUTTONDOWN)
    {
      if (GetKeyState(VK_SHIFT) & 0x8000)
        bFreeze = !bFreeze;
      else // Repaint a random color on left-click
        RepaintTimer->Enabled = true;

      OldWinProc(msg); // Call main handler
    }
    else
      OldWinProc(msg); // Call main handler
  }
  catch(...)
  {
    OldWinProc(msg); // Call main handler
  }
}
//---------------------------------------------------------------------------

int __fastcall TDTSColor::GetColorizeNetClient(int Client)
// Return the value to encode in the ClientID field of the
// COLORIZENET struct given our current client in "Cli"
// We need this in case we change the indexes of the clients
// internally but the plugin's remain the same!
{
  int id;

  switch(Client)
  {
    case C_YAHELITE: id = ID_YAHELITE; break;
    case C_MIRC: id = ID_MIRC; break;
    case C_ICECHAT: id = ID_ICECHAT; break;
    case C_HYDRAIRC: id = ID_HYDRAIRC; break;
    case C_LEAFCHAT: id = ID_LEAFCHAT; break;
    case C_VORTEC: id = ID_VORTEC; break;
    case C_XIRCON: id = ID_XIRCON; break;
    case C_PALTALK: id = ID_PALTALK; break;
    //.....
    // Add future clients here...
    //.....
    case C_TRINITY: id = ID_TRINITY; break;
    default: id = 0; break;
  };

  return id;
}
//---------------------------------------------------------------------------

int __fastcall TDTSColor::GetInternalClient(int Client)
// Return the Cli (global value used internally)
// given a permanent ClientID from a client sending us info.
{
  switch(Client)
  {
    case ID_YAHELITE: return C_YAHELITE;
    case ID_MIRC: return C_MIRC;
    case ID_ICECHAT: return C_ICECHAT;
    case ID_HYDRAIRC: return C_HYDRAIRC;
    case ID_LEAFCHAT: return C_LEAFCHAT;
    case ID_VORTEC: return C_VORTEC;
    case ID_XIRCON: return C_XIRCON;
    case ID_PALTALK: return C_PALTALK;
    //.....
    // Add future clients here...
    //.....
    case ID_TRINITY: return C_TRINITY;
    default: return -1; // Undefined
  };
}
//---------------------------------------------------------------------------
/*
char* __fastcall TDTSColor::GetMappedMemory(String MemName, int MemSize)
{
  try
  {
    HANDLE hMapFile;

    hMapFile = OpenFileMapping(
                    FILE_MAP_ALL_ACCESS,   // read/write access
                    FALSE,                 // do not inherit the name
                    MemName.c_str());       // name of mapping object

    if (hMapFile == NULL)
      return NULL;

    char* pBuf = (char *)MapViewOfFile(hMapFile, // handle to map object
                FILE_MAP_ALL_ACCESS,  // read/write permission
                0,
                0,
                MemSize); // if this is 0 the entire file is mapped...

    if (pBuf == NULL)
    {
       CloseHandle(hMapFile);
       return NULL;
    }

    char* p = new char[MemSize];
    memmove(p, pBuf, MemSize);

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);

    return p;
  }
  catch(...)
  {
    return NULL;
  }
}
*/
//---------------------------------------------------------------------------
void __fastcall TDTSColor::AddEffect(int Effect, bool bUndo,
                                        bool bShowRTF, bool bShowStatus)
// bShowRTF will be cleared if the RTF view is not needed
// We normally want to call DoShowRtf() after adding an effect
// to show the RTF version...
// bShowRTF defaults to true, the only time it is
// cleared is A) when bold/underline/italic effects are added
// inside DoProcess(). B) When DoBlend() is called when in any
// View except V_RTF. C) When adding an effect remotely
//
// bUndo defaults true and writes to MS_ORG as a backup if set
// bShowStatus defaults true and is cleared when adding an effect by
// remote-command.
{
  if (!utils->IsRtfIrcOrgView())
    return;

  bool bRepRTF = bShowRTF && utils->IsRtfView();

  if (tae->TextLength == 0)
  {
    if (bShowStatus)
      utils->ShowMessageU(String(DS[45])); // No text to process

    return;
  }

  if (ProcessEffect)
  {
    delete ProcessEffect;
    ProcessEffect = NULL;
  }

  // Convert origional text to IRC colored text
  ProcessEffect = new TProcessEffect(this, Effect); // Create object

  if (!ProcessEffect)
    return;

  Application->ProcessMessages();

  // Set Properties... (all int except EP5)
  ProcessEffect->EP0 = this->EParm0;
  ProcessEffect->EP1 = this->EParm1; // Usually foreground color
  ProcessEffect->EP2 = this->EParm2; // Usually background color
  ProcessEffect->EP3 = this->EParm3; // Sometimes used for "mode" (EM_FG/EM_BG)
  ProcessEffect->EP4 = this->EParm4; // Sometimes used for "Skip Spaces"
  ProcessEffect->EP5 = this->EParm5; // double
  ProcessEffect->FullSpectFg = this->bRgbRandFG;
  ProcessEffect->FullSpectBg = this->bRgbRandBG;

  // this->MaxColors is only set now after invoking MorphDlg and it's only
  // used by the Morph effect right now
  ProcessEffect->MaxColors = DialogSaveMaxColors;

  utils->PushOnChange(tae);

  // Save present cursor-position
  int SaveSelStart = tae->SelStart;
  int SaveSelLength = tae->SelLength;
  int SaveSelLine = tae->Line;

  int segmentsToAdd = utils->IsRtfView() ? 4 : 3;
  CpInit(segmentsToAdd);

  try
  {
    // Read the MS_IRC or MS_ORG stream to a buffer, add the text-effect
    // and write the output to the IRC stream.
    ProcessEffect->Execute(bShowStatus);

    // ReturnValue is negative if an error occured, 0 if no error and -1 if
    // user-cancel.
    int ReturnValue = ProcessEffect->ReturnValue;

    // ReturnDelta is the # chars added or subtracted (needed to set the new
    // cursor-position). Can be negative!
    int ReturnDelta = ProcessEffect->ReturnDelta;

    delete ProcessEffect;
    ProcessEffect = NULL;

    switch (ReturnValue)
    {
      case 1: // push/pop warning
        if (bShowStatus)
          utils->ShowMessageU(String(DS[140]));
      case 0: // completed
      {
        // Clear bTextWasProcessed so we can DoProcess normally
        // if the user presses Process
        if (bTextWasProcessed && tae->View == V_ORG)
          bTextWasProcessed = false;

        if (bRepRTF)
          DoShowRtf(bShowStatus); // Convert to RTF
        else // Has to be bIrc or bOrg!
        {
          // Save original position and carat (Note: when an effect is added,
          // no printable text changes in the document!) - (Correction - it
          // will change if spaces are added or stripped or if codes are
          // stripped!)
          TTaePosition* p = new TTaePosition(tae);
          p->SavePos = p->Position;

          LoadView(V_IRC);

          // Restore original position and carat
          p->Position = p->SavePos;
          delete p;
        }

        // Restore old cursor-position
        int NewPosition;

        if (Effect != E_STRIP_CODES)
        {
          NewPosition = SaveSelStart + SaveSelLength;

          if (tae->View != V_RTF)
            NewPosition += ReturnDelta + 1;

          if (NewPosition < 0)
            NewPosition = 0;
          else
          {
            // TextLength counts line-terminators as two chars!
            int len = (int)tae->TextLength - tae->LineCount + 1;
            if (NewPosition > len)
              NewPosition = len;
          }
        }
        else
          NewPosition = tae->GetCharIdx(SaveSelLine);

        tae->SelStart = NewPosition;

        if (SL_IRC->Count > 0)
        {
          if (IsCli(C_YAHELITE))
            ConvertIrcToYahoo(bShowStatus);
          else
            ConvertIrcToHtml(bShowStatus); // Convert MS_IRC to HTML in MS_HTML stream
        }

        tae->Modified = true;
        bEffectWasAdded = true;
        bSaveWork = true;
      }
      break;

      case -1: // User canceled operation!
        if (bShowStatus)
          utils->ShowMessageU(String(DS[137]));
      break;

      case -2: // You cannot add an effect in this View!
        if (bShowStatus)
          utils->ShowMessageU(String(DS[134]));
      break;

      default:
         // error in text-processing
        if (bShowStatus)
          utils->ShowMessageU(String(DS[138]) + ": " + ReturnValue);
      break;
    }
  }
  __finally
  {
    CpHide(); // at least check...
    
    // put this here to add CpReset() but you can't do that
    // because if AddEffect() is nested in another CpInit() you
    // wipe out the progress! Can only put it in the method with the
    // original CpInit()!
  }

  utils->PopOnChange(tae);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::Timer2Timer(TObject *Sender)
// Handles Play when YahELite sends WM_COPY
// (or when our custom DLL for XiRCON sends it)
// Also handles Non-Client area (topic) mouse-clicks...
{
  Timer2->Enabled = false;

  if (wmCopyMode == CM_NCDOUBLECLICK)
    WMNCLDblClk();
  else if (wmCopyMode == CM_NCRIGHTCLICK)
    WMNCRDownClk();
  else if (wmCopyMode == CM_REMOTE_COMMAND)
  {
    // Data for command (if any) is in "long WmCopyData"
    if (wmCopyCommand == REMOTE_COMMAND_START)
      StartPlayClick(NULL);
    else if (wmCopyCommand == REMOTE_COMMAND_STOP)
      StopPlayClick(NULL);
    else if (wmCopyCommand == REMOTE_COMMAND_PAUSE)
      PausePlayClick(NULL);
    else if (wmCopyCommand == REMOTE_COMMAND_RESUME)
      ResumePlayClick(NULL);
    else if (wmCopyCommand == REMOTE_COMMAND_CHANNEL)
      AddRoom(WmCopyStr);
    else if (wmCopyCommand == REMOTE_COMMAND_TIME)
    {
      // Client can send this either as a string or integer and we handle it!
      if (wmCopyData > 0)
        PlayTime = wmCopyData;
      else
        PlayTime = WmCopyStr.ToIntDef(PlayTime);
    }
    // GReplyTo has already been set, here we set the plugin's version string
    else if (wmCopyCommand == REMOTE_COMMAND_ID)
      PluginVersion = WmCopyStr;
    else if (wmCopyCommand == REMOTE_COMMAND_FILE)
    {
      bIsAnsiFile = false; // "Assume" a UTF-8 file...

      if (OpenFileSetDefaultDirs(utils->Utf8ToWide(WmCopyStr)))
        if (wmCopyData == 1)
          StartPlayClick(NULL);
    }
  }
  else if (wmCopyMode == CM_SWIFTMIX_STATE)
  {
    if (bIsPlaying && WmPlayer == GPlayer)
    {
      TPlay* pb = new TPlay(this);

      if (WmState == PS_SWIFTMIX_STOP)
        pb->DoStopPlay();
      else if (WmState == PS_SWIFTMIX_PAUSE)
        pb->DoPausePlay();

      delete pb;
    }
  }
  else if (wmCopyMode == CM_SWIFTMIX_PLAY)
  {
    TPlayback* pb = new TPlayback(this);

    if (!bIsPlaying)
    {
      if (pb->DoStartPlay(PlayTime))
      {
        GLineBurstSize = 1; // Override line-bursting
        GPlayer = WmPlayer;
      }
    }
    else if (WmPlayer == GPlayer)
      pb->play->DoResumePlay();

    delete pb;
  }
  else if (wmCopyMode == CM_SWIFTMIX_TIME)
  {
    if (bIsPlaying && WmPlayer == GPlayer &&
              WmDuration > WmCurrent && GLineCount > 0)
    {
      int remainingSeconds = WmDuration - WmCurrent; // # seconds left
      GInterval = (remainingSeconds*1000)/GLineCount;

      if (GInterval < 100)
        GInterval = 100;
    }
  }
  else if (wmCopyMode == CM_PROCESS_RX_DATA)
  {
    if (PlayOnCX->Checked && !bIsPlaying && WmCopyStr.Length() > 0)
    {
      WideString S;

      // A Client Plugin that sends UTF-8 will set wmCopyData to 1.
      if ((IsPlugin() && wmCopyData == 1) || (!IsPlugin() && bSendUtf8))
        S = utils->Utf8ToWide(WmCopyStr);
      else
        S = WideString(WmCopyStr);

      S = utils->StripTrailingCRLFs(S);

      utils->WaitForThread();

      // The first parm of a text string may be an effects index
      // 0 = "random" up to 19
      if (S.Length() > 0)
      {
        // Automatically set a new client if we know what to set. -1
        // is "undefined" and 0 means "any client" - need to call this
        // before saving SL_ORG below...
        if (wmCopyClient > 0)
        {
          int internal = GetInternalClient(wmCopyClient); // -1 if unknown
          if (internal >= 0 && internal != Cli)
            SetNewClient(internal);
        }

        // We need to save SL_ORG, clear the streams, then call
        // DoProcess(). ProcessIRC will copy SL_ORG and use that if
        // anything is in it, rather than use text in TaeEdit.
        TStringsW* SL_TEMP = new TStringsW();
        SL_TEMP->Assign(SL_ORG);

        ClearAndFocus(true); // Clear everything

        if (S.Length() >= 3) // look for a parameter?
        {
          // read past leading spaces
          int ii = 1;
          for (; ii <= S.Length(); ii++)
          {
            if (S[ii] == ' ')
              continue;
            break;
          }

          WideString T;

          // read past non-spaces
          for (; ii <= S.Length(); ii++)
          {
            if (S[ii] == ' ')
              break;
            T += S[ii];
          }

          int index;

          if (T.Length() > 0 && T.Length() < 3) // Two chars
            index = utils->ToIntDefW(T, -1);
          else
            index = -1; // no good!

          int maxEffectIdx;

          if (IsYahTrinPal())
            maxEffectIdx = E_MAX_EFFECTS_EXTENDED;
          else
            maxEffectIdx = E_MAX_EFFECTS_BASIC;

          if (index >= 0 && index <= maxEffectIdx && SL_IRC)
          {
            // Get the text we want to process and send back...
            S = S.SubString(ii+1, S.Length()-ii);

            // Have to have text in the window
            SafeEditAdd(S);

            // Have to have data in a stream to add effect, and we
            // need to preserve the user's SL_ORG (if any)... so use SL_IRC
            SL_IRC->Text = S;
            tae->View = V_IRC;

            if (index == 0) // random effect?
              index = random(maxEffectIdx)+1;

            switch (index)
            {
              case 1: // Random Fg
                EParm3 = EM_FG;
                EParm4 = true; // Skip Spaces
                AddEffect(E_RAND_COLORS, false, false, true);
              break;

              case 2: // Random Bg
                EParm3 = EM_BG;
                EParm4 = true; // Skip Spaces
                AddEffect(E_RAND_COLORS, false, false, true);
              break;

              case 3: // Random Fg/Bg
                EParm3 = EM_FGBG;
                EParm4 = true; // Skip Spaces
                AddEffect(E_RAND_COLORS, false, false, true);
              break;

              case 4: // Inc Fg
                EParm1 = 0;
                EParm2 = paletteSize/2;
                EParm3 = EM_FG;
                EParm4 = true; // Skip Spaces
                AddEffect(E_INC_COLORS, false, false, true);
              break;

              case 5: // Inc Bg
                EParm1 = 0;
                EParm2 = paletteSize/2;
                EParm3 = EM_BG;
                EParm4 = true; // Skip Spaces
                AddEffect(E_INC_COLORS, false, false, true);
              break;

              case 6: // Inc Fg/Bg
                EParm1 = 0;
                EParm2 = paletteSize/2;
                EParm3 = EM_FGBG;
                EParm4 = true; // Skip Spaces
                AddEffect(E_INC_COLORS, false, false, true);
              break;

              case 7: // Alt Fg
              {
                EParm1 = random(16) + 1;
                int count = 20; // timeout
                while(((EParm2 = random(16) + 1) == EParm1) && count--);
                EParm3 = EM_FG;
                EParm4 = true; // Skip Spaces
                AddEffect(E_ALT_COLORS, false, false, true);
              }
              break;

              case 8: // Alt Bg
              {
                EParm1 = random(16) + 1;
                int count = 20; // timeout
                while(((EParm2 = random(16) + 1) == EParm1) && count--);
                EParm3 = EM_BG;
                EParm4 = true; // Skip Spaces
                AddEffect(E_ALT_COLORS, false, false, true);
              }
              break;

              case 9: // Alt Fg/Bg
              {
                EParm1 = random(16) + 1;
                int count = 20; // timeout
                while(((EParm2 = random(16) + 1) == EParm1) && count--);
                EParm3 = EM_FGBG;
                EParm4 = true; // Skip Spaces
                AddEffect(E_ALT_COLORS, false, false, true);
              }
              break;

              case 10: // Alternate underline/color

                EParm0 = Afg;
                EParm1 = Abg;
                EParm2 = CTRL_U;
                EParm4 = C_NULL;
                AddEffect(E_ALT_CHAR, false, false, true);

              break;

              case 11: // Alternate underline/rand color

                EParm0 = Afg;
                EParm1 = IRCRANDOM;
                EParm2 = CTRL_U;
                EParm4 = C_NULL;
                AddEffect(E_ALT_CHAR, false, false, true);

              break;

              case 12: //  Alternate underline/bold

                EParm0 = NO_COLOR;
                EParm1 = NO_COLOR;
                EParm2 = CTRL_U;
                EParm4 = CTRL_B;
                AddEffect(E_ALT_CHAR, false, false, true);

              break;

              case 13: // Alternate underline/italics

                EParm0 = NO_COLOR;
                EParm1 = NO_COLOR;
                EParm2 = CTRL_R;
                EParm4 = CTRL_U;
                AddEffect(E_ALT_CHAR, false, false, true);

              break;

              case 14: // Alternate bold/italics

                EParm0 = NO_COLOR;
                EParm1 = NO_COLOR;
                EParm2 = CTRL_R;
                EParm4 = CTRL_B;
                AddEffect(E_ALT_CHAR, false, false, true);

              break;

              case 15: // Alternate bold/color

                EParm0 = Afg;
                EParm1 = Abg;
                EParm2 = CTRL_B;
                EParm4 = C_NULL;
                AddEffect(E_ALT_CHAR, false, false, true);

              break;

              case 16: // Alternate italics/color

                EParm0 = Afg;
                EParm1 = Abg;
                EParm2 = CTRL_R;
                EParm4 = C_NULL;
                AddEffect(E_ALT_CHAR, false, false, true);

              break;


              case 17: // Alternate color

                EParm0 = Afg;
                EParm1 = Abg;
                EParm2 = C_NULL;
                EParm4 = C_NULL;
                AddEffect(E_ALT_CHAR, false, false, true);

              break;

              case 18: // Spare

                EParm0 = NO_COLOR;
                EParm1 = NO_COLOR;
                EParm2 = C_NULL;
                EParm4 = C_NULL;
                AddEffect(E_NONE, false, false, true);

              break;

              case 19: // Spare

                EParm0 = NO_COLOR;
                EParm1 = NO_COLOR;
                EParm2 = C_NULL;
                EParm4 = C_NULL;
                AddEffect(E_NONE, false, false, true);

              break;

              ////////////////////////////////////
              // End of Basic (IRC Client) Effects
              ////////////////////////////////////

              case 20:
                DoFontEffect(E_FONT_SAWTOOTH, cSize - (50*cSize/100), cSize + (200*cSize/100), 0);
              break;

              case 21:
                DoFontEffect(E_FONT_RANDOM, cSize - (25*cSize/100), cSize + (50*cSize/100), 0);
              break;

              case 22:
                DoFontEffect(E_FONT_MOUNTAIN, cSize - (25*cSize/100), cSize + (50*cSize/100), MYPI);
              break;

              case 23:
                DoFontEffect(E_FONT_VALLEY, cSize + (50*cSize/100), cSize - (25*cSize/100), MYPI);
              break;

              case 24:
                DoFontEffect(E_FONT_MOUNTAIN, cSize - (25*cSize/100), cSize + (50*cSize/100), MYPI/2);
              break;

              case 25:
                DoFontEffect(E_FONT_VALLEY, cSize + (50*cSize/100), cSize - (25*cSize/100), MYPI/2);
              break;

              case 26:
                DoFontEffect(E_FONT_MOUNTAIN, cSize + (50*cSize/100), cSize - (25*cSize/100), 2*MYPI);
              break;

              case 27:
              {
                // Get TaeEdit's leading colors
                int fg, bg;
                if (utils->GetTaeEditColors(fg, bg, false))
                {
                  // Get dialog's properties to save
                  FgFromColor = fg;
                  FgToColor = bg;
                  BgFromColor = bg;
                  BgToColor = fg;
                  bFgChecked = true;
                  bBgChecked = false;

                  EParm3 = EM_FG;
                  AddEffect(E_MORPH_FGBG, false, false, true);
                }
              }
              break;

              case 28:
              {
                // Get TaeEdit's leading selection-colors
                int fg, bg;
                if (utils->GetTaeEditColors(fg, bg, false))
                {
                  // Get dialog's properties to save
                  FgFromColor = fg;
                  FgToColor = bg;
                  BgFromColor = bg;
                  BgToColor = fg;
                  bFgChecked = false;
                  bBgChecked = true;

                  EParm3 = EM_BG;
                  AddEffect(E_MORPH_FGBG, false, false, true);
                }
              }
              break;

              case 29:
              {
                // Get TaeEdit's leading selection-colors
                int fg, bg;
                if (utils->GetTaeEditColors(fg, bg, false))
                {
                  // Get dialog's properties to save
                  FgFromColor = fg;
                  FgToColor = bg;
                  BgFromColor = bg;
                  BgToColor = fg;
                  bFgChecked = true;
                  bBgChecked = true;

                  EParm3 = EM_FGBG;
                  AddEffect(E_MORPH_FGBG, false, false, true);
                }
              }
              break;

              default:
              break;
            };
          }
          else
            SafeEditAdd(S);
        }
        else
          SafeEditAdd(S);

        DoProcess(false);

        // If the chat-client sent us a channel name or nickname to reply
        // to - set PlayChan to that!
        if (!WmChanNickStr.IsEmpty())
        {
          // ***Should do this on the plugin-side!!!
          if (IsCli(C_ICECHAT) &&
                  utils->LowerCaseW(WmChanNickStr) == WideString("console"))
            WmChanNickStr = STATUSCHAN; // set "status"

          AddRoom(WmChanNickStr); // Works for UTF-8 and ANSI both
        }

        TPlayback* pb = new TPlayback(this);
        pb->DoStartPlay(PlayTime == 0 ? 50 : PlayTime/4);
        delete pb;

        // Restore SL_ORG
        SL_ORG->Assign(SL_TEMP);
        delete SL_TEMP;
        LoadView(V_IRC); // Turn on V_ORG menu item :)
      }
    }
  } // end PROCESS_RX_DATA

  // Free temp vars to receive next WM_COPYDATA's info...
  WmChanNickStr = "";
  WmCopyStr = "";
  wmCopyMode = CM_IDLE;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::WMNCLDblClk(void)
// Called from Timer2 Handler
{
  // launch default web-browser
  //ShellExecute(Handle, "open", "iexplore.exe",
  //                 Iftf->Strings[INFO_WEB_SITE].c_str(), NULL, SW_SHOW);

  ShellExecute(NULL, "open", Iftf->Strings[INFO_WEB_SITE].c_str(),
                                                  NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::WMNCRDownClk(void)
// Called from Timer2 Handler
{
  if (tae->View == V_RTF && tae->SelLength > 0)
  {
    // Invoke color-set effect on selected text
    EParm1 = captionFG;
    EParm2 = captionBG;
    EParm3 = EM_FGBG;
    AddEffect(E_SET_COLORS);
  }
  else
  {
    if (TOCUndo != NULL)
    {
      ONCHANGEW oc = utils->GetInfoOC(tae, NULL);
      TOCUndo->Add(UNDO_FG_COLOR, Foreground, 0, oc, "");
      TOCUndo->Add(UNDO_BG_COLOR, Background, 0, oc, "", true); // chained undo
    }

    Foreground = captionFG;
    Background = captionBG;

    UpdateColorButtons();
  }

  bFreeze = false;
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::SafeEditAdd(WideString S)
// Adds S to the TaeRichEdit without triggering the OnChange thread!
{
  utils->PushOnChange(tae);
  tae->TextW = S;
  utils->PopOnChange(tae);
}
//----------------------------------------------------------------------------
bool __fastcall TDTSColor::IsWmCopyData(void)
// True if it's a client that uses the WM_COPYDATA windows
// message.
{
  return (Cli == C_TRINITY || Cli == C_ICECHAT || Cli == C_HYDRAIRC ||
            Cli == C_LEAFCHAT || Cli == C_YAHELITE ? true : false);
}
//----------------------------------------------------------------------------
bool __fastcall TDTSColor::IsPlugin(void)
// True if it's a client that uses the RWM_ColorizeNet windows
// message and COLORIZENET structure.
{
  return IsPlugin(Cli);
}
//----------------------------------------------------------------------------
bool __fastcall TDTSColor::IsPlugin(int c)
// True if it's a client that uses the RWM_ColorizeNet windows
// message and COLORIZENET structure.
{
  return (c == C_TRINITY || c == C_ICECHAT || c == C_HYDRAIRC ||
                            c == C_LEAFCHAT ? true : false);
}
//----------------------------------------------------------------------------
bool __fastcall TDTSColor::IsYahTrinPal(void)
{
  return (Cli == C_YAHELITE || Cli == C_TRINITY ||
                              Cli == C_PALTALK ? true : false);
}
//----------------------------------------------------------------------------
bool __fastcall TDTSColor::IsCli(tCli isCli)
{
  return (Cli == isCli ? true : false);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::Timer1Event(TObject *Sender)
{
  try
  {
    if (bNewInstallation)
    {
      Timer1->Enabled = false;

      LeftJustify->Checked = true;
      PackText->Checked = true;
      PackLines->Checked = true;
      Margins->Checked = true;
      Wingdings->Checked = false;
      AutoWidth->Checked = true;

      DoDemo(MANUALFILE);
    }
    else  // play back each line of text that was queued
    {
      TPlay *pb = new TPlay(this);

      if (GLineCount)
      {
        if (!pb->SendToClient(false))
        {
#if DEBUG_ON
          CWrite( "\r\nError in play->SendToClient(false)...\r\n");
#endif
          pb->DoStopPlay();
          delete pb;
          return;
        }

        if (GLineBurstSize > 1)
        {
          GLineBurstCounter++;

          // Note: DTSColor->Timer1->Interval < DTSColor->BurstInterval
          // when we first kick off the timer
          if (Timer1->Interval == GInterval ||
                                 Timer1->Interval < BurstInterval)
            Timer1->Interval = BurstInterval;
          else if (GLineBurstCounter >= GLineBurstSize)
          {
            Timer1->Interval = GInterval;
            GLineBurstCounter = 0;
          }
        }
        else
          Timer1->Interval = DTSColor->GInterval;

        // This is used to compute the play-interval in real-time
        // based upon the SwiftMix 1-second audio-playback
        // WM_SENDDATA commands
        GLineCount--;
      }

      if (bIsPlaying && !GLineCount)
        pb->DoStopPlay();

      delete pb;
    }
  }
  catch(...) { }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::TechDemoClick(TObject *Sender)
{
  LeftJustify->Checked = true;
  PackText->Checked = false;
  PackLines->Checked = true;
  Margins->Checked = true;
  Wingdings->Checked = false;
  AutoWidth->Checked = true;

  DoDemo(TECHFILE);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::TextWingsDemoClick(TObject *Sender)
{
  LeftJustify->Checked = false;
  PackText->Checked = true;
  PackLines->Checked = true;
  Margins->Checked = false;
  Wingdings->Checked = true;
  AutoWidth->Checked = false;

  DoDemo(MANUALFILE);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::TextDemoClick(TObject *Sender)
{
  LeftJustify->Checked = true;
  PackText->Checked = true;
  PackLines->Checked = true;
  Margins->Checked = true;
  Wingdings->Checked = false;
  AutoWidth->Checked = true;

  DoDemo(MANUALFILE);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ASCIIArtDemoClick(TObject *Sender)
{
  LeftJustify->Checked = false;
  PackText->Checked = false;
  PackLines->Checked = true;
  Margins->Checked = false;
  Wingdings->Checked = false;
  AutoWidth->Checked = true;

  DoDemo(ARTFILE);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::DoDemo(WideString wFile, bool bAnsi)
{
  WideString wPath = utils->ExtractFilePathW(utils->GetExeNameW());

  if (!utils->FileExistsW(wPath + wFile))
  {
    wPath += "Smileys\\";

    if (!utils->FileExistsW(wPath + wFile))
    {
      // Cannot find...
      utils->ShowMessageW(U16(DS[222]) + "\n\"" + wPath + wFile + "\"");
      return;
    }
  }

  utils->SetDefaultFont(tae);

  bIsAnsiFile = bAnsi; // set global flag (property)

  CpInit(1); // need this for LoadLines' progress bar
  
  if (LoadLines(wPath + wFile, true, true) && tae->Lines->Count)
  {
    bIsColorFractal = false;

    DoProcess(true); // this will reset progress bar...
  }

  bNewInstallation = false; // suppresses warning about pre-existing codes
}
//---------------------------------------------------------------------------
TFontStyles __fastcall TDTSColor::IntToStyle(int Value)
// Convert integer to "font styles"
{
  // fsBold, fsItalic, fsUnderline, fsStrikeOut

  TFontStyles Temp;

  if (Value & 0x1) Temp << fsBold;
  if (Value & 0x2) Temp << fsItalic;
  if (Value & 0x4) Temp << fsUnderline;
  if (Value & 0x8) Temp << fsStrikeOut;

  return Temp;
}

//---------------------------------------------------------------------------
int __fastcall TDTSColor::StyleToInt(TFontStyles Value)
// Convert "font styles" to integer
{
  // fsBold, fsItalic, fsUnderline, fsStrikeOut

  int Temp = 0;

  if (Value.Contains(fsBold)) Temp |= 0x1;
  if (Value.Contains(fsItalic)) Temp |= 0x2;
  if (Value.Contains(fsUnderline)) Temp |= 0x4;
  if (Value.Contains(fsStrikeOut)) Temp |= 0x8;
  return Temp;
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::FileOpenUtf8Menu(TObject *Sender)
{
  WideString wFilters = WideString("All files (*.*)|*.*|"
                  "Document files (*.rtf)|*.rtf|"
                  "Text Files (*.txt)|*.txt|"
                  "Web files (*.htm)|*.htm");

  // Run the open-file dialog in FormOFDlg.cpp
  // (wFilter has user's filter by reference on return!)
  WideString wFile = utils->GetOpenFileName(wFilters, 1, OpenD, String(DS[36]));

  // Load and convert file as per the file-type (either plain or rich text)
  if (!wFile.IsEmpty())
  {
    bIsAnsiFile = false;

    if (!OpenFileSetDefaultDirs(wFile))
      utils->ShowMessageU(DS[38]); // Can't load file
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FileOpenAnsiMenu(TObject *Sender)
{
  WideString wFilters = WideString("All files (*.*)|*.*|"
                  "Document files (*.rtf)|*.rtf|"
                  "Text Files (*.txt)|*.txt|"
                  "Web files (*.htm)|*.htm");

  // Run the open-file dialog in FormOFDlg.cpp
  // (wFilter has user's filter by reference on return!)
  WideString wFile = utils->GetOpenFileName(wFilters, 1, OpenD, String(DS[36]));

  // Load and convert file as per the file-type (either plain or rich text)
  if (!wFile.IsEmpty())
  {
    bIsAnsiFile = true;

    if (!OpenFileSetDefaultDirs(wFile))
      utils->ShowMessageU(DS[38]); // Can't load file
  }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::OpenFileSetDefaultDirs(WideString wFile)
// return true if success
{
  CpInit(1); // set progress-bar for one loop

  try
  {
    WideString wExt = utils->ExtractFileExtW(wFile);
    wExt = utils->LowerCaseW(wExt);

    // Our program actually HANGS when trying to open an EXE file on Windows8!
    if (wExt == WideString(".exe"))
    {
//    utils->ShowMessageW("Not able to read EXE files! (" + wFile + ")");
      return false;
    }

    bIsColorFractal = false;

    bool bSetNewPath;

    if (OpenYahcolorizeFile(wFile, true) && tae->Lines->Count)
      bSetNewPath = true;
    else if (LoadLines(wFile, true, true))
      bSetNewPath = true;
    else
    {
      OpenF = "";
      return false;
    }

    if (bSetNewPath)
    {
      FileSaveItem->Enabled = true;
      bSaveWork = false;

      OpenD = utils->ExtractFilePathW(wFile);
      OpenF = utils->ExtractFileNameW(wFile);
      bFileOpened = true;

      // After opening a file odds are that we want to save
      // it the the same path... In the registry we keep
      // OpenD and SaveD only.
      //
      // OpenF and SaveF are set empty as initial Strings
      // on program start.
      //
      // SaveF is set empty on TaeEditClear().
      SaveD = OpenD;
      SaveF = OpenF;
    }
  }
  __finally
  {
#if DEBUG_ON
    CpHide();
#else
    CpReset();
#endif
  }

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::OpenYahcolorizeFile(WideString wFile, bool bShowStatus)
// return true if success
{
  if (wFile.IsEmpty())
    return false;

  bool bRet = false;

  try
  {
    try
    {
      // Clears streams invalidates views
      ClearAndFocus(true);

      bIsColorFractal = false;

      WideString Ext = utils->ExtractFileExtW(wFile);
      Ext = utils->LowerCaseW(Ext);
      if (Ext == WideString(".rtf"))
        bRet = LoadLines(wFile, true, false); // RTF?
      else if (Ext == WideString(".htm") || Ext == WideString(".html")) // HTML?
      {
        WideString S = utils->HtmlToIrc(wFile, bShowStatus);

        if (!S.IsEmpty())
        {
          utils->WaitForThread();

          utils->PushOnChange(tae);

          SL_ORG->Text = S;

          // If new text has color, DoShow()
          if (utils->TextContainsFormatCodes(S))
          {
            tae->View = V_ORG; // View must be V_ORG to copy SL_ORG to SL_IRC!
            DoShowRtf(bShowStatus); // Convert to RTF
          }
          else
            LoadView(V_ORG);

          utils->PopOnChange(tae);

          bRet = true;
        }
      }
      else
      //else if (Ext == "" || Ext == ".txt" || Ext == ".cpp" ||
      //     Ext == ".c" || Ext == ".hpp" || Ext == ".h" || Ext == ".cs" ||
      //              Ext == ".tcl" || Ext == ".ini")
      {
        if (LoadLines(wFile, true, true))
        {
          // If new text has color, DoShow()
          if (utils->TextContainsFormatCodes(SL_ORG))
            DoShowRtf(bShowStatus); // Convert to RTF

          bRet = true;
        }
      }

      if (WindowState != wsMinimized)
        tae->SetFocus();

      tae->Modified = false;
      bSaveWork = false;

      PrintShortLineCount();
    }
    catch(...)
    {
      if (bShowStatus)
        utils->ShowMessageU(String(DS[38]));
    }
  }
  __finally
  {
#if DEBUG_ON
    CpHide();
#else
    CpReset();
#endif
  }

  // Clear the Undo buffer after loading a file!
  if (bRet)
    if (TOCUndo != NULL)
      TOCUndo->Clear();


  return bRet;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FileSaveItemClick(TObject *Sender)
// If you previously opened or Save As'ed a file and have not
// done a Clear() or LoadLines() we save to the same name
// with no warning prompt.
{
  if (SaveF.IsEmpty())
  {
    utils->ShowMessageU(DS[40]); // No Filename Specified
    FileSaveItem->Enabled = false;
    return;
  }

  WideString FileName = SaveD + "\\" + SaveF;

  // FYI, "Can't Save File..." is DS[42]

  if (!utils->FileExistsW(FileName))
  {
    utils->ShowMessageU(String(DS[42]) +
                 "\"" + FileName + "\""); // Error Saving Text to:

    FileSaveItem->Enabled = false;
    return;
  }

  DoSaveFile(FileName);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FileSaveAsItemClick(TObject *Sender)
// Saves main window or IRC stream to selected file
{
#if LICENSE_ON
  if (PK->ComputeDaysRemaining() <= 0)
  {
    //"Visit www.yahcolorize.com to buy this product!"
    utils->ShowMessageU(KEYSTRINGS[4] + Iftf->Strings[INFO_WEB_SITE] +
                                                              KEYSTRINGS[5]);
    return;
  }
#endif

  try
  {
    if (SaveD.IsEmpty() || !utils->DirectoryExistsW(SaveD))
      SaveD = DeskDir;

    WideString wDefFile;

    if (SaveF.IsEmpty())
      wDefFile = "NewFile.txt";
    else
      wDefFile = SaveF;

    WideString wFilters = WideString("All files (*.*)|*.*|"
                      "Text files (*.txt)|*.txt|"
                      "Document files (*.rtf)|*.rtf|"
                      "Single Web-page file (*.htm)|*.htm");

    // Run the TSFDlgForm and get a file name...
    // Pass strings in as utf-8... the default filter is chosen based on the
    // extension on sDefFile.
    WideString wFile = utils->GetSaveFileName(wFilters, wDefFile,
                                                 SaveD, String(DS[39]));

    if (wFile.IsEmpty())
      return;

    WideString Ext = utils->ExtractFileExtW(wFile);

    if (Ext.IsEmpty())
    {
      Ext = ".txt";
      wFile += Ext;
    }

    // 1 = .txt
    // 2 = .rtf
    // 3 = .htm
    // 4 = *.*

    if (utils->FileExistsW(wFile))
      if (utils->ShowMessageU(Handle, DS[160],
                MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1) == IDNO)
        return; // Cancel

    if (Ext == WideString(".rtf")) // *.rtf
    {
      if (tae->View == V_RTF)
      {
        SaveFileViaStream(wFile, false);
        tae->Modified = false;
        bSaveWork = false;
      }
      else if (tae->View == V_RTF_SOURCE)
      {
        SaveFileViaStream(wFile, true);
        tae->Modified = false;
        bSaveWork = false;
      }
      else
        utils->ShowMessageU("You can only save .rtf files in the\n\"Processed\" view or the \"RTF Source\" view!");
    }
    else if (Ext == WideString(".htm") || Ext == WideString(".html")) // *.htm
    {
      if ((SL_ORG && SL_ORG->Count) || (SL_IRC && SL_IRC->Count))
      {
        GenerateHTML(true, wFile); // Create a TWebExportForm then write to wFile
        tae->Modified = false;
        bSaveWork = false;
      }
      else
        utils->ShowMessageU("There is nothing to save!");
    }
    else
      DoSaveFile(wFile); // *.txt, *.*

    SaveD = utils->ExtractFilePathW(wFile);
    SaveF = utils->ExtractFileNameW(wFile);
    bFileSaved = true;

    FileSaveItem->Enabled = true;
  }
  catch(...) { utils->ShowMessageU(String(DS[42])); }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::DoSaveFile(WideString wFile)
{
  // If user is in the processed view and clicks "save as"
  // we try to save the IRC stream...
  if (utils->IsRtfIrcOrgView())
  {
    utils->PushOnChange(tae);

    TStringsW* sl = utils->IsOrgView() ? SL_ORG : SL_IRC;

    if (sl == NULL)
      return;

    // Copy string-list
    TStringsW* nsl = new TStringsW();
    sl->Assign(nsl);

    // Optimize copy
    utils->Optimize(nsl, false, NO_COLOR);

    nsl->SaveToFile(wFile); // this saves in UTF-8!

    delete nsl;

    utils->PopOnChange(tae);
  }
  else
    SaveFileViaStream(wFile, tae->View == V_RTF_SOURCE ? false : true);

  tae->Modified = false;
  bSaveWork = false;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SaveFileViaStream(WideString wFile, bool SavePlain)
{
  TMemoryStream* MS_Temp = new TMemoryStream();

  if (MS_Temp != NULL)
  {
    utils->PushOnChange(tae);

    bool SaveSelected;

    if (tae->SelLength > 0)
      SaveSelected = true; // Selected text
    else
      SaveSelected = false; // No Selected text

    // convert our "textual" <<page>> into a real RTF \page
    utils->PageBreaksToRtf(tae);

    // Flags: Stream, SelectionOnly, PlainText,
    //        NoObjects, PlainRTF
    tae->CopyToStream(MS_Temp, SaveSelected, SavePlain, true, false);

    if (MS_Temp->Size >= 1)
    {
      MS_Temp->SetSize(MS_Temp->Size-1); // Don't want a C_NULL in the file!

      // Write an ANSI string to a wide file-name
      utils->WriteStringToFileW(wFile, utils->StreamToStringA(MS_Temp));
    }

    // restore graphical pagebreaks
    if (utils->IsRtfView())
      DoShowRtf(false); // put the <<page>> breaks back
    else
      LoadView(tae->View);

    utils->PopOnChange(tae);

    delete MS_Temp;
  }
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::FileExit(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PopupTextStateClick(TObject *Sender)
{
  // View text state at caret

  if (!utils->IsRtfIrcOrgView())
    return;

  TPoint p = tae->CaretPos;

  WideString wS;

  // get just the line the caret is on of raw codes into wS
  utils->IsOrgView() ? wS = SL_ORG->GetString(p.y) : wS = SL_IRC->GetString(p.y);

  if (!utils->IsRtfView())
    p.x = utils->GetRealIndex(wS, p.x); // V_ORG or V_IRC, need real char index...

  PUSHSTRUCT ps; // filled by reference in SetStateFlags()

  if (utils->SetStateFlags(wS, p.x, ps) <= 0)
    return; // error...

  utils->ShowState(ps, p.y+1, p.x+1);
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::EditCut(TObject *Sender)
{
  TaeEditCutCopy(true, true);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EditCopy(TObject *Sender)
{
  TaeEditCutCopy(false, true);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PopupDeleteClick(TObject *Sender)
{
  TaeEditCutCopy(true, false);
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::HelpAbout(TObject *Sender)
{
  Application->CreateForm(__classid(TAboutForm), &AboutForm);
  AboutForm->ShowModal();
  YcDestroyForm(AboutForm);
  AboutForm = NULL;
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::EditSelectAllClick(TObject *Sender)
{
  if (tae->TextLength != 0)
    tae->SelectAll();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BordersClick(TObject *Sender)
{
  if (Borders->Checked)
  {
    if (bTopEnabled || bSideEnabled)
      return;

    utils->ShowMessageU(String(DS[43]));

    Borders->Checked = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::WingdingsClick(TObject *Sender)
{
  // If user tries to enamle the master wings but all boxes are unchecked,
  // warn them...
  if (Wingdings->Checked && LeftWings != NULL && LeftWings->Count > 0)
  {
    for (int ii = 0; ii < LeftWings->Count; ii++)
    {
      // any flags set... then ok to do master wing-enable...
      if ((bool)LeftWings->GetTag(ii) == true)
        return;
    }

    utils->ShowMessageU(String(DS[44]));

    Wingdings->Checked = false;
  }
}
//---------------------------------------------------------------------------
// Clear bShowStatus to avoid a "processing" message
//
// bWriteORG defaults to true and if set we save
// the current contents of the edit window in MS_ORG before
// processing.
void __fastcall TDTSColor::DoProcess(bool bShowStatus)
// Wrapper-function to push/pop onchange
{
  if (SpellCheckForm)
    SpellCheckForm->ScClose();

  if (ReplaceTextForm)
    ReplaceTextForm->FrClose();

  utils->PushOnChange(tae);

  // Get main-edit's undo-info
  ONCHANGEW oc = utils->GetInfoOC();

  // Add Undo point
  if (TOCUndo != NULL && oc.p != NULL)
  {
    TStringsW* sl = (TStringsW*)oc.p;
    TOCUndo->Add(UNDO_PROCESS, 0, sl->TotalLength, oc, sl->Text);
  }

  DoP(bShowStatus);
  utils->PopOnChange(tae);
}

void __fastcall TDTSColor::DoP(bool bShowStatus)
{
  // Don't let button be hit twice!
  if (!ProcessButton->Enabled)
    return;

  if (bShowStatus && tae->TextLength == 0)
  {
    utils->ShowMessageU(DS[45]); // "There is no text to process."
    tae->SetFocus();
    return;
  }

  // Don't allow user to keep pressing this!
  ProcessButton->Enabled = false;
  PopupProcess->Enabled = false;

  // Display "Cancel" modeless dialog
  bool bSmallAmountOfText = tae->TextLength < STATUSLENGTH;

  // Calling this forces this to be the base "original" method for
  // the next CpInit()
  CpReset(); // reset progress bar

  if (!bSmallAmountOfText && bShowStatus)
    CpShow(STATUS[1]); // "Loading memory-stream"

  try
  {
    try
    {
      if (tae->TextLength > WARNINGLENGTH)
      {
        if (utils->ShowMessageU(Handle, DS[130],
                MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDNO)
        {
          UpdateProcessButtons(tae->View);
          return;
        }
      }

      // You can find thhe number to put in here - 7 was obtained by watching
      // the screen while processing text repeatedly and gradually increasing
      // the number of progress bar sections until 1) the panel goes off and 2)
      // the CpAuxMsg field stops displaying an error! This could be automated
      // and saved as a registry value - but that's really not necessary I think...
      //
      // DoBlend() below calls its own CpInit() and its sections are added
      // there.
      int maxSections = 7;
      // we add another entire iteration to find the text-height if vertical-blend!
      if (Foreground == IRCVERTBLEND || Background == IRCVERTBLEND)
        maxSections += 2;
      if (!IsYahTrinPal()) // must be calling strip?
        maxSections += 2;

      CpInit(maxSections); // divide the progress bar into sections

      // Note: Be sure that Effects.cpp uses return-code
      // 0 for the E_FG_BLEND and E_BG_BLEND function.
      //
      // Horizontal FG blending needs to just be done on the initial
      // text, prior to running ProcessIRC which will move words around,
      // add wings and borders and pad out the lines with spaces.
      if (Foreground == IRCHORIZBLEND)
      {
        if (!CreateBlendEngine(BLEND_FG))
        {
          ProcessError(1);
          return;
        }

        // Before we add the blend, move the text to MS_IRC
        // if V_ORG to preserve MS_ORG!
        if (tae->View == V_ORG)
        {
          utils->WaitForThread();
          SL_IRC->Text = SL_ORG->Text;
          tae->View = V_IRC;
        }

        // This will return with highlighted codes in the V_IRC view state
        if (!DoBlend(E_FG_BLEND, false)) // Don't need write to MS_ORG!
        {
          ProcessError(2);
          return; // error
        }
      }

      if (Background == IRCHORIZBLEND)
      {
        if (!CreateBlendEngine(BLEND_BG))
        {
          ProcessError(3);
          return;
        }

        // Before we add the blend, move the text to MS_IRC
        // if V_ORG to preserve MS_ORG!
        if (tae->View == V_ORG)
        {
          utils->WaitForThread();
          SL_IRC->Text = SL_ORG->Text;
          tae->View = V_IRC;
        }

        // This will return with highlighted codes in the V_IRC view state
        if (!DoBlend(E_BG_BLEND, false)) // Don't need write to MS_ORG!
        {
          ProcessError(4);
          return; // error
        }
      }

      try
      {
        // Foreground/Background Vertical-blending...
        // Vertical blending is done line-by-line during ProcessIRC.
        if (Foreground == IRCVERTBLEND)
        {
          if (!CreateBlendEngine(BLEND_VERT_FG))
          {
            ProcessError(5);
            return; // error
          }
        }

        // If the user has selected IRCBLEND as the background
        // color (1002) -- we need to fire up BlendEngine...
        if (Background == IRCVERTBLEND)
        {
          if (!CreateBlendEngine(BLEND_VERT_BG))
          {
            ProcessError(6);
            return; // error
          }
        }
      }
      catch(...)
      {
        ProcessError(7);
        return;
      }

      // Technically we don't need to stop play since we have a seperate buffer
      // It uses the progress bar so that might bump around a bit - but we
      // need to be able to process new text as the old is playing don't we???
      //if (bIsPlaying)
      //{
      //  // Halt playback
      //  TPlay* pb = new TPlay(this);
      //  pb->DoStopPlay();
      //  delete pb;
      //}

      /////////////////////////////////////////////////////////////////
      // Processing section...
      /////////////////////////////////////////////////////////////////

      bool bStatusAW = false; // Auto-width initial status is off

      // AutoWidth on if V_RTF or V_IRC (makes nested borders neat!)
      if (utils->IsRtfIrcView() && bTextWasProcessed)
        bStatusAW = TempAutoWidth(true);

      // Save original position and carat
      TTaePosition* p = new TTaePosition(tae);

      if (!bStatusAW)
        p->RestoreSel = false;

      p->SavePos = p->Position;

      // Set flag for case where we are processing selected text
      bool bClearSel = tae->SelLength > 0 ? true : false;

      // Convert to Irc in SL_IRC
      int retVal = ConvertTextToIrc(bShowStatus);

      // If we are processing selected text nothing will be selected
      // and the caret should point to the beginning on exit
      if (bClearSel)
      {
        tae->SelLength = 0;
        tae->SelStart = 0;
      }

      if (bStatusAW)
        TempAutoWidth(false); // off if it was on

      if (retVal > 1) // Error
        ProcessError(8, retVal);
      else if (retVal == 0) // Process completed?
      {
        if (SL_IRC->Count != 0)
        {
          if (IsCli(C_YAHELITE))
            retVal = ConvertIrcToYahoo(bShowStatus);
          else // Convert MS_IRC to HTML in MS_HTM stream
            retVal = ConvertIrcToHtml(bShowStatus);
        }

        // Don't do WYSIWYG if we are doing auto-play on CR
        // or if this is a remote /CX message
        if (!bShowStatus)
        {
          if (retVal > 1)
            ProcessError(9, retVal); // Error
          else if (SL_IRC->Count > 0)
            LoadView(V_IRC);
        }
        else
        {
          linesInRtfHeader = ConvertIrcToRtf(SL_IRC, false, bShowStatus);

          if (linesInRtfHeader != 0 && MS_RTF != NULL)
          {
            // Set flag
            bTextWasProcessed = true;
            LoadView(V_RTF);
          }
          else
          {
            LoadView(V_IRC);
            ProcessError(10);
          }
        }
      }

      if (retVal == 1) // User Cancel
      {
        utils->ShowMessageU(String(DS[54]));

        LoadView(V_ORG);
      }

      // Restore original position and carat
      if (bClearSel)
      {
        tae->SelLength = 0;
        tae->SelStart = 0;
      }
      else
        p->Position = p->SavePos;

      delete p;

      tae->SetFocus();
    }
    catch(...) {}
  }
  __finally
  {
    if (CancelPanel->Visible || FCpTotalSections != FCpCurrentSection)
    {
#if DEBUG_ON
      CWrite("\r\nTDTSColor::DoProcess()::CpReset():Cancel Panel Mismatch!\r\n");
#else
      CpReset(); // Failsafe (essential!) force hide if we got stuck on...
#endif
    }
  }

  tae->Modified = false;

  ProcessError(0); // no error, delete resources
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ProcessError(int Error, int DefaultExtendedError)
{
  try
  {
    if (ProcessIRC)
    {
      delete ProcessIRC;
      ProcessIRC = NULL;
    }

    if (ConvertToYahoo)
    {
      delete ConvertToYahoo;
      ConvertToYahoo = NULL;
    }

    DeleteBlenderObjects();

    // Restore state and function of "Process" button
    UpdateProcessButtons(tae->View);

    Application->ProcessMessages();

    if (Error != 0)
    {
      // Unable to process
      if (DefaultExtendedError != -1000) // default in prototype main.h
        utils->ShowMessageU(String(DS[52]) + "[" + String(Error) +
              "," + String(DefaultExtendedError) + "]");
      else
        utils->ShowMessageU(String(DS[52]) + "[" + String(Error) + "]");
    }
  }
  catch(...)
  {
  }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::TempAutoWidth(bool bOn)
{
  static bool bOldPack;
  static bool bOldAuto;
  static int OldIndent;

  // Help the user set these to avoid chaotic processing!
  // (Unless user manually changed PackText or the width.)
  if (bOn)
  {
    // Save AutoWidth and PackText checkbox state
    bOldAuto = AutoWidth->Checked;
    bOldPack = PackText->Checked;
    OldIndent = RegIndent;

    AutoWidth->OnClick = NULL;
    AutoWidth->Checked = true;
    AutoWidth->OnClick = AutoWidthClick;

    PackText->OnClick = NULL;
    PackText->Checked = false;
    PackText->OnClick = PackTextClick;

    regIndent = 0;

    return true; // on
  }

  // Restore AutoWidth and PackText checkboxes...
  AutoWidth->OnClick = NULL;
  AutoWidth->Checked = bOldAuto;
  AutoWidth->OnClick = AutoWidthClick;

  PackText->OnClick = NULL;
  PackText->Checked = bOldPack;
  PackText->OnClick = PackTextClick;

  regIndent = OldIndent;

  return false; // off
}
//---------------------------------------------------------------------------
int __fastcall TDTSColor::ConvertIrcToRtf(TStringsW* sl,
                                  bool bBlackAndWhite, bool bShowStatus)
// Converts sl to MS_RTF (rich-text)
// Returns private property "linesInHeader" (0 if failure)
//
// Set bBlackAndWhite to avoid rtf init colors of Afg/Abg in conversion
// (usually this applies if we are "showing" text not "processing" text)
{
  if (sl == NULL || sl->Count == 0)
    return 0;

  try
  {
    int lih = 0;

    TConvertToRTF* rtf = new TConvertToRTF(this, tae, bShowStatus);

    if (bBlackAndWhite)
    {
      rtf->DefaultFgColor = IRCBLACK;
      rtf->DefaultBgColor = IRCWHITE;
    }

    if (rtf)
    {
      int retVal;

      // Notes: RTF colors "technically" have no limit - you can just keep
      // adding colors to the color-table - BUT, PalTalk has a limit PER POST
      // of colors - this call below will process an entire document and
      // the color-table represents the entire document - so even if we set
      // the mac Paltalk color limit - it would only work for a one-line
      // document - a quandry, indeed - there is no simple way to show the
      // user the way things will appear after colors per-line are
      // algorithmically "resolved" as they will be during playback to
      // the chat-room. So I think, for now I'll just comment out the
      // setting of MaxColors and let it default to -1 (infinite)...
      //if (IsCli(C_PALTALK))
      //  rtf->MaxColors = this->PaltalkMaxColors;

      TMemoryStream* NewMs = rtf->Execute(sl, retVal);

      if (NewMs != NULL && retVal == 0)
      {
        // Reassign the string-list
        if (MS_RTF != NULL) {try {delete MS_RTF;} catch(...) {}}
        MS_RTF = NewMs;
        lih = rtf->LinesInHeader; // Set private var
      }
      else
      {
        if (MS_RTF != NULL)
          MS_RTF->Clear();

        lih = 0;

        utils->ShowMessageU("Error in ConvertIrcToRtf(): " + String(retVal));
      }

      delete rtf;
    }

    return lih;
  }
  catch(...)
  {
    utils->ShowMessageU("Threw exception in ConvertIrcToRtf()!");
    return 0;
  }
}
//---------------------------------------------------------------------
int __fastcall TDTSColor::ConvertTextToIrc(bool bShowStatus)
// Returns 0 for no errors, 1 for user cancel, 2-9 is an error in this function
// Returns 10-19 for errors in ProcessIRC.cpp TConvertToIRC
{
  int retVal;

  try
  {
    if (ProcessIRC)
      delete ProcessIRC;

    ProcessIRC = new TConvertToIRC(this); // Create object

    if (ProcessIRC)
    {
      // For effects processing we want to suppress the addition
      // of borders and wings.
      retVal = ProcessIRC->Execute(bShowStatus);

      delete ProcessIRC;
      ProcessIRC = NULL;
    }
    else
      retVal = 2; // error
  }
  catch(...)
  {
    retVal = 3;

    if (ProcessIRC)
    {
      delete ProcessIRC;
      ProcessIRC = NULL;
    }
  }

  if (retVal == 0)
  {
    // User abort???
    Application->ProcessMessages();

    if ((int)GetAsyncKeyState(VK_ESCAPE) < 0)
      retVal = 1;
  }

  return(retVal);
}
//---------------------------------------------------------------------
int __fastcall TDTSColor::ConvertIrcToYahoo(bool bShowStatus)
{
  if (!IsCli(C_YAHELITE))
    return 0; // not yahoo, then don't waste time here

  if (SL_ORG == NULL || SL_IRC == NULL || SL_HTM == NULL)
    return 2;

  int ReturnValue;

  try
  {
    if (ConvertToYahoo != NULL)
      delete ConvertToYahoo;

    ConvertToYahoo = new TConvertToYahoo(this);

    if (ConvertToYahoo)
    {
      ReturnValue = ConvertToYahoo->Execute(bShowStatus);
      delete ConvertToYahoo;
      ConvertToYahoo = NULL;
    }
    else
      ReturnValue = 2;
  }
  catch(...)
  {
    ReturnValue = 2;

    if (ConvertToYahoo)
    {
      try {delete ConvertToYahoo;} catch(...) {}
      ConvertToYahoo = NULL;
    }
  }

  if (ReturnValue == 0)
  {
    // User abort???
    Application->ProcessMessages();

    if ((int)GetAsyncKeyState(VK_ESCAPE) < 0)
      ReturnValue = 1;
  }

  SetView(tae->View); // Update menu-items...

  return ReturnValue;
}
//---------------------------------------------------------------------
int __fastcall TDTSColor::ConvertIrcToHtml(bool bShowStatus)
// Converts MS_IRC to a TStringList and then calls ConvertLineToHTML()
// in TConvertToHTML() to convert it to HTML. The HTML is put into the
// stringlist and written to the MS_HTM stream.
// (Used to maintain the MS_HTM HTML View)
//
// This is invoked in normal processing to generate the SL_HTM View.
// It is not used by File->ExportAsWebPage() which uses GenerateHTML()!
{
  if (SL_IRC == NULL || SL_HTM == NULL || SL_IRC->Count == 0)
    return 2;

  int ReturnValue = 0;

  TConvertToHTML* ProcessHtml = NULL;

  // Change processing status panel... Processing to HTML format...
  if (bShowStatus)
    CpShow(STATUS[10], DS[70]);

  try
  {
    try
    {
      if ((ProcessHtml = new TConvertToHTML(this)) != NULL)
      {
        // Office 2007 will paste HTML instead of RTF if both are on the
        // clipboard... and its HTML decoder does NOT read the white-space
        // style...
        ProcessHtml->bNonBreakingSpaces = false;

        utils->WaitForThread();

        SL_HTM->Clear();

        // Copy string-list
        TStringsW* sl = new TStringsW();
        sl->Text = SL_IRC->Text;

        // Optimize copy
        utils->Optimize(sl, false, NO_COLOR);

        int count = sl->Count;

        CpSetMaxIterations(count);

        while (GetAsyncKeyState(VK_ESCAPE) < 0); // Dummy read to clear...

        // For each line, convert it to HTML and add it to SL_HTM...
        for (int ii = 0; ii < count; ii++)
        {
          // User abort???
          Application->ProcessMessages();

          if ((int)GetAsyncKeyState(VK_ESCAPE) < 0)
          {
            ReturnValue = 1;
            break; // Cancel
          }

          SL_HTM->Add(ProcessHtml->ConvertLineToHTML(sl->GetString(ii)), NULL);

          // Advance progress-bar
          CpUpdate(ii);
        }

        delete sl;
      }
      else
        ReturnValue = 2;
    }
    catch(...)
    {
      ReturnValue = 2;
    }

    if (ReturnValue == 0)
    {
      // User abort???
      Application->ProcessMessages();
      
      if ((int)GetAsyncKeyState(VK_ESCAPE) < 0)
        ReturnValue = 1;
    }

    SetView(tae->View); // Update menu-items...
  }
  __finally
  {
    try {if (ProcessHtml) delete ProcessHtml;} catch(...) {}

    CpHide();
  }

  return ReturnValue;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::WidthEditEnter(TObject *Sender)
// Called from width-box's OnChange event...
{
  if (WidthEdit->Text.IsEmpty()) return;

  if (WidthEdit->Color == clYellow) // Temporarily set?
  {
    // Set it back and return
    LineWidth = regWidth;
    WidthEdit->Text = String(LineWidth);
    WidthEdit->Color = clWindow;
    return;
  }

  regWidth = WidthEdit->Text.ToIntDef(WIDTHINIT);

  if (regWidth < MINLINESIZE) regWidth = MINLINESIZE;

  LineWidth = regWidth;

  WidthEdit->Text = String(LineWidth);

  // If user changes the width limit while processed text
  // is in the view, we clear a flag to keep DoProcess from
  // automatically activating AutoWidth.
  bTextWasProcessed = false;
  WidthEdit->Color = clWindow;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::WidthEditClick(TObject *Sender)
{
  if (WidthEdit->Color == clYellow)
  {
    WidthEdit->OnChange = NULL;
    WidthEdit->Color = clWindow;
    LineWidth = regWidth;
    WidthEdit->Text = String(LineWidth);
    bTextWasProcessed = false;
    WidthEdit->OnChange = WidthEditEnter;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::WidthEditDblClick(TObject *Sender)
// Set Default
{
  LineWidth = regWidth = WIDTHINIT;
  WidthEdit->Text = String(LineWidth);
  bTextWasProcessed = false;
  WidthEdit->Color = clWindow;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::AutoWidthClick(TObject *Sender)
// This will fire if the program sets/unsets the auto-width
// checkbox AND when you set the flag AutoWidth->Checked
// so disable it!
{
  // Auto-Linesize?
  if (AutoWidth->Checked)
    WidthEdit->Enabled = false;
  else
  {
    LineWidth = regWidth;

    WidthEdit->OnChange = NULL;
    WidthEdit->Text = String(LineWidth);
    WidthEdit->Color = clWindow;
    WidthEdit->OnChange = WidthEditEnter;

    WidthEdit->Enabled = true;

    // If user turns on AutoWidth while processed text
    // is in the view, we clear a flag to keep DoProcess from
    // automatically activating AutoWidth.
    bTextWasProcessed = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MaskEditEnter(TObject *Sender)
{
  //TMaskEdit* p = (TMaskEdit *)Sender;
  TMaskEdit* p = dynamic_cast<TMaskEdit*>(Sender);
  if (!p || p->Text.IsEmpty())
    return;

//  ShowMessage(String(p->ClassName));

  if (p->Name == "IndentEdit" && p->Text.Length() == 1 && p->Text[1] == '-')
    return; // User entering a negative #

  int Temp = p->Text.ToIntDef(-5000); // flag is -5000

  if (Temp != -5000)
  {
    if (p->Name == "TabsEdit")
    {
      regTabs = Temp;
      utils->SetTabs(tae, regTabs);
    }
    else if (p->Name == "LMarginEdit")
      regLmargin = Temp;
    else if (p->Name == "RMarginEdit")
      regRmargin = Temp;
    else if (p->Name == "IndentEdit")
      regIndent = Temp;
    else if (p->Name == "TMarginEdit")
      regTmargin = Temp;
    else if (p->Name == "BMarginEdit")
      regBmargin = Temp;
    else if (p->Name == "SpacingEdit")
      regSpacing = Temp;
    else if (p->Name == "HeightEdit")
      LineHeight = regHeight = Temp;
    else
      return;
  }
  else
  {
    if (p->Name == "TabsEdit")
    {
      Temp = regTabs = TABSINIT;
      utils->SetTabs(tae, regTabs);
    }
    else if (p->Name == "LMarginEdit")
      Temp = regLmargin = LMARGINIT;
    else if (p->Name == "RMarginEdit")
      Temp = regRmargin = RMARGINIT;
    else if (p->Name == "IndentEdit")
      Temp = regIndent = INDENTINIT;
    else if (p->Name == "TMarginEdit")
      Temp = regTmargin = TMARGINIT;
    else if (p->Name == "BMarginEdit")
      Temp = regBmargin = BMARGINIT;
    else if (p->Name == "SpacingEdit")
      Temp = regSpacing = SPACINGINIT;
    else if (p->Name == "HeightEdit")
      Temp = LineHeight = regHeight = HEIGHTINIT;
    else
      return;
  }

  p->OnChange = NULL;
  p->Text = String(Temp);
  p->OnChange = MaskEditEnter;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MaskEditDblClick(TObject *Sender)
// Restore Default
{
  //TMaskEdit* p = (TMaskEdit *)Sender;
  TMaskEdit* p = dynamic_cast<TMaskEdit*>(Sender);
  if (!p)
    return;

  int Temp;

  if (p->Name == "TabsEdit")
  {
    Temp = regTabs = TABSINIT;
    utils->SetTabs(tae, regTabs);
  }
  else if (p->Name == "LMarginEdit")
    Temp = regLmargin = LMARGINIT;
  else if (p->Name == "RMarginEdit")
    Temp = regRmargin = RMARGINIT;
  else if (p->Name == "IndentEdit")
    Temp = regIndent = INDENTINIT;
  else if (p->Name == "TMarginEdit")
    Temp = regTmargin = TMARGINIT;
  else if (p->Name == "BMarginEdit")
    Temp = regBmargin = BMARGINIT;
  else if (p->Name == "SpacingEdit")
    Temp = regSpacing = SPACINGINIT;
  else if (p->Name == "HeightEdit")
    Temp = LineHeight = regHeight = HEIGHTINIT;
  else
    return;

  p->OnChange = NULL;
  p->Text = String(Temp);
  p->OnChange = MaskEditEnter;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MarginsClick(TObject *Sender)
{
  if (Margins->Checked)
  {
    LMarginEdit->Enabled = true;
    RMarginEdit->Enabled = true;
    TMarginEdit->Enabled = true;
    BMarginEdit->Enabled = true;
    IndentEdit->Enabled = true;
  }
  else
  {
    LMarginEdit->Enabled = false;
    RMarginEdit->Enabled = false;
    TMarginEdit->Enabled = false;
    BMarginEdit->Enabled = false;
    IndentEdit->Enabled = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::AutoHeightClick(TObject *Sender)
{
  // AutoHeight?
  if (AutoHeight->Checked)
    HeightEdit->Enabled = false;
  else
  {
    LineHeight = RegHeight;
    HeightEdit->OnChange = NULL;
    HeightEdit->Text = String(LineHeight);
    HeightEdit->Color = clWindow;
    HeightEdit->OnChange = MaskEditEnter;

    HeightEdit->Enabled = true;
  }

  // If user changes auto-height enabled while processed text
  // is in the view, we clear a flag to keep DoProcess from
  // automatically activating AutoWidth.
  bTextWasProcessed = false;
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::MenuWingEditClick(TObject *Sender)
{
  try
  {
    if (WingEditForm)
    {
      BringWindowToTop(WingEditForm->Handle);
      return;

//      YcDestroyForm(WingEditForm);
//      WingEditForm = NULL;
    }

    // Close the replace dialog if it's open
    if (ReplaceTextForm && ReplaceTextForm->Visible)
      ReplaceTextForm->Close();
  }
  catch(...) {}

  Application->CreateForm(__classid(TWingEditForm), &WingEditForm);
  WingEditForm->Show();
}
//---------------------------------------------------------------------------
// Find/Replace-text routines
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EditFindItemClick(TObject *Sender)
// Called when the user selects "Find" from the Edit Menu
{
  DoFindReplace(false);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EditReplaceItemClick(TObject *Sender)
// Called when the user selects "Replace" from the Edit Menu
{
  DoFindReplace(true);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::DoFindReplace(bool bReplaceMode)
{
  bIsColorFractal = false;

  if (tae->Lines->Count == 0) return;

  try
  {
    if (ReplaceTextForm)
    {
      YcDestroyForm(ReplaceTextForm);
      ReplaceTextForm = NULL;
    }
  }
  catch(...) {}

  Application->CreateForm(__classid(TReplaceTextForm), &ReplaceTextForm);

  if (ReplaceTextForm)
  {
    ReplaceTextForm->EnableReplace = bReplaceMode;
    ReplaceTextForm->Show();
  }
}
//---------------------------------------------------------------------------

void __fastcall TDTSColor::MenuSetFgColorClick(TObject *Sender)
{
  SetFgColor();
}
//---------------------------------------------------------------------------

void __fastcall TDTSColor::MenuSetBgColorClick(TObject *Sender)
{
  SetBgColor();
}
//---------------------------------------------------------------------------

void __fastcall TDTSColor::MenuSetWingColorClick(TObject *Sender)
{
  SetWingColor();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuSetWindowColorClick(TObject *Sender)
{
  // Set the bg color of the entire TaeEdit Control
  if (utils != NULL && utils->GetTColorDialog(taeWindowColor))
    tae->Color = taeWindowColor;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SetFgColor(void)
{
  int C = utils->SelectCustomColor(String(DS[63]), FgPanel->Color, COLOR_FORM_FG);

  if (C == IRCCANCEL)
    return;

  if ((C == IRCVERTBLEND || C == IRCHORIZBLEND) && GBlendPreset >= 0 && fG_BlendColors)
  {
    // Load fG_BlendColors
    if (!LoadBlendPresetFromRegistry(fG_BlendColors, GBlendPreset))
      utils->ShowMessageU(String(DS[221])); // Preset is empty or corrupt
    else
      fG_BlendPreset = GBlendPreset;
  }

  // Invoke color-set effect on selected text
  if (tae->SelLength && tae->LockCounter == 0 &&
      (C < IRCCANCEL || C == IRCRANDOM || C == IRCHORIZBLEND) && utils->IsRtfIrcOrgView())
  {
    // Invoke color-set effect on selected text
    if (C == IRCRANDOM)
      EParm1 = IRCRANDOM;
    else
      EParm1 = utils->ConvertColor(C, false);

    EParm2 = NO_COLOR;
    EParm3 = EM_FG;

    if (C == IRCHORIZBLEND)
      AddEffect(E_FG_BLEND);
    else
      AddEffect(E_SET_COLORS);

    return;
  }

  if (C == Background && C != IRCVERTBLEND && C != IRCHORIZBLEND && C != IRCRANDOM)
    utils->ShowMessageU(String(DS[64]));

  if (TOCUndo != NULL)
  {
    ONCHANGEW oc = utils->GetInfoOC(tae, NULL);
    TOCUndo->Add(UNDO_FG_COLOR, Foreground, 0, oc, "");
  }

  Foreground = C;

  UpdateColorButtons(); // Panel needs to be painted if set to "Blend"
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SetBgColor(void)
{
  int C = utils->SelectCustomColor(String(DS[65]), BgPanel->Color, COLOR_FORM_BG);

  if (C == IRCCANCEL) return;

  if ((C == IRCVERTBLEND || C == IRCHORIZBLEND) &&
                                        GBlendPreset >= 0 && bG_BlendColors)
  {
    // Load bG_BlendColors
    if (!LoadBlendPresetFromRegistry(bG_BlendColors, GBlendPreset))
      utils->ShowMessageU(String(DS[221])); // Preset is empty or corrupt
    else
      bG_BlendPreset = GBlendPreset;
  }

  // Invoke color-set effect on selected text
  if (tae->SelLength && tae->LockCounter == 0 &&
      (C < IRCCANCEL || C == IRCRANDOM || C == IRCHORIZBLEND) &&
          (tae->View == V_RTF || tae->View == V_IRC || tae->View == V_ORG))
  {
    // Invoke color-set effect on selected text
    if (C == IRCRANDOM)
      EParm2 = IRCRANDOM;
    else
      EParm2 = utils->ConvertColor(C, false);

    EParm1 = NO_COLOR;
    EParm3 = EM_BG;

    if (C == IRCHORIZBLEND)
      AddEffect(E_BG_BLEND);
    else
      AddEffect(E_SET_COLORS);

    return;
  }

  if (C != IRCVERTBLEND && C != IRCHORIZBLEND && C != IRCRANDOM)
  {
    if (C == Foreground)
      utils->ShowMessageU(String(DS[66]));
    else if (C == WingColor)
      utils->ShowMessageU(String(DS[67]));
  }

  if (TOCUndo != NULL)
  {
    ONCHANGEW oc = utils->GetInfoOC(tae, NULL);
    TOCUndo->Add(UNDO_BG_COLOR, Background, 0, oc, "");
  }

  Background = C;

  UpdateColorButtons(); // Panel needs to be painted if set to "Blend"
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SetWingColor(void)
{
  int C = utils->SelectCustomColor(String(DS[68]), WingColorPanel->Color, COLOR_FORM_WING);

  if (C == IRCCANCEL) return;

  if (Background == C && C != IRCVERTBLEND && C != IRCHORIZBLEND && C != IRCRANDOM) utils->ShowMessageU(String(DS[69]));

  if (TOCUndo != NULL)
  {
    ONCHANGEW oc = utils->GetInfoOC(tae, NULL);
    TOCUndo->Add(UNDO_WING_COLOR, WingColor, 0, oc, "");
  }

  WingColor = C;

  UpdateColorButtons(); // Forground needs to be painted if set to "Blend"
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SetBlendBackgroundColor(void)
{
  int C = utils->SelectCustomColor(String(DS[65]), BlendColorPanel->Color, COLOR_FORM_BLEND);

  if (C != IRCCANCEL)
  {
    BlendBackground = C;
    UpdateColorButtons();
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::UpdateColorButtons(void)
{
  utils->SetPanelColorAndCaption(FgPanel, Foreground);
  utils->SetPanelColorAndCaption(BgPanel, Background);
  utils->SetPanelColorAndCaption(WingColorPanel, WingColor);
  utils->SetPanelColorAndCaption(BlendColorPanel, BlendBackground);
  SetAfgAbgAwcAbl();

  // Set title-bar colors the same as color-panels
  if (FgPanel->Color != BgPanel->Color)
  {
    WMNCPaint(FgPanel->Color, BgPanel->Color);
    RepaintTimer->Enabled = false; // Stop any pending update
  }

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Convienient place to put this for now...
  // Get TColor for font from Afg
//  cColor = utils->GetColor(Afg);
  cColor = clBlack;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SetAfgAbgAwcAbl(void)
// Note: ComputeAllColors() sets
// Afg, Abg, Abl and Awc during text-processing!
{
  Abl = utils->ConvertColor(BlendBackground, false);
  Awc = utils->ConvertColor(WingColor, false);

  if (IsYahTrinPal() && (Foreground == IRCVERTBLEND || Foreground == IRCHORIZBLEND))
    Afg = -utils->BlendColorToRgb(FG_BlendColors[0]);
  else
    Afg = utils->ConvertColor(Foreground, false);

  if (IsYahTrinPal() && (Background == IRCVERTBLEND || Background == IRCHORIZBLEND))
    Abg = Abl; // Set bg to blend BG color
  else
    Abg = utils->ConvertColor(Background, false);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ProcessButtonClick(TObject *Sender)
{
  DoProcess(true);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SetPlay1Click(TObject *Sender)
{
  Application->CreateForm(__classid(TPlayForm), &PlayForm);
  PlayForm->ShowModal();
  YcDestroyForm(PlayForm);
  PlayForm = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::StartPlayClick(TObject *Sender)
{
  TPlayback* pb = new TPlayback(this);
  pb->DoStartPlay(PlayTime);
  delete pb;

  if (WindowState != wsMinimized)
    tae->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ProcessSongTitleClick(TObject *Sender)
// Process SwiftMiX song info
{
  // If text is playing into a room user must stop it first!
  if (bIsPlaying)
    return;

  try
  {
    // Render SwiftMiX Song-title and album/artist in color in the main window
    String uFmt = Iftf->Strings[INFO_SONG_FORMAT];

    TReplaceFlags flags = (TReplaceFlags() << rfReplaceAll << rfIgnoreCase);

    if (WmAlbum.IsEmpty())
      WmAlbum = "(Unknown Album)";

    if (WmName.IsEmpty())
      WmName = "(Unknown Song)";

    if (WmArtist.IsEmpty())
      WmArtist = "(Unknown Artist)";

    if (WmPath.IsEmpty())
      WmPath = "(Unknown File Path)";

    if (uFmt.IsEmpty())
//      uFmt = WmPath;
      uFmt = "Playing: " + WmArtist + ", " + WmAlbum + ", " + WmName;
    else
    {
      // Replace all "%song" etc... must do BEFORE single-codes like &s!
      uFmt = StringReplace(uFmt, "%song", WmName, flags);
      uFmt = StringReplace(uFmt, "%artist", WmArtist, flags);
      uFmt = StringReplace(uFmt, "%album", WmAlbum, flags);
      uFmt = StringReplace(uFmt, "%path", WmPath, flags);
      uFmt = StringReplace(uFmt, "%duration", String(WmDuration), flags);
      uFmt = StringReplace(uFmt, "%player", String(WmPlayer), flags);
      uFmt = StringReplace(uFmt, "%c", String(CTRL_K), flags);
      uFmt = StringReplace(uFmt, "%f", String(CTRL_F), flags);
      uFmt = StringReplace(uFmt, "%s", String(CTRL_S), flags);
      uFmt = StringReplace(uFmt, "%b", String(CTRL_B), flags);
      uFmt = StringReplace(uFmt, "%u", String(CTRL_U), flags);
      uFmt = StringReplace(uFmt, "%i", String(CTRL_R), flags);
    }

    ClearAndFocus(true);

    utils->WaitForThread();
    SL_ORG->Text = utils->Utf8ToWide(uFmt);
    LoadView(V_ORG);

    // Process text and show RTF
    DoProcess(true);
  }
  catch(...) {}
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::StopPlayClick(TObject *Sender)
{
  if (bIsPlaying)
  {
    TPlay* pb = new TPlay(this);
    pb->DoStopPlay();
    delete pb;
  }

  if (WindowState != wsMinimized)
    tae->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PausePlayClick(TObject *Sender)
// Called from Play menu
{
  TPlay* pb = new TPlay(this);
  pb->DoPausePlay();
  delete pb;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ResumePlayClick(TObject *Sender)
// Called from Play menu
{
  TPlay* pb = new TPlay(this);
  pb->DoResumePlay();
  delete pb;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PauseButtonClick(TObject *Sender)
{
  TPlay* pb = new TPlay(this);

  if (IsYahTrinPal() || IsPlugin())
  {
    if (bIsPlaying)
    {
      if (Timer1->Enabled)
        pb->DoPausePlay();
      else
        pb->DoResumePlay();
    }
  }
  else
  {
    if (ResumeButton->Enabled)
      pb->DoResumePlay();
    else
      pb->DoPausePlay();
  }

  if (WindowState != wsMinimized)
    tae->SetFocus();

  delete pb;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FileNewItemClick(TObject *Sender)
{
  utils->PushOnChange(tae);

  // Clear will add an undo, saving SL_ORG...
  ClearAndFocus(true);

  // Clear entire Undo-list for "New"
  if (TOCUndo != NULL)
    TOCUndo->Clear();

  SetWordWrap(false); // Set this off since word-wrap mode is read-only...

  // Set the TAB to "Process"
  if (PageControl1->ActivePage != PageControl1->Pages[0])
  {
    PageControl1->ActivePage = PageControl1->Pages[0];
    PageControl1Change(NULL);
  }

  FileSaveItem->Enabled = false;

  bIsColorFractal = false;

  // This saves us if the lock-counter ever has unmatched push/pops!
  utils->InitOnChange(tae, TaeEditChange);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EditClearItemClick(TObject *Sender)
{
  utils->PushOnChange(tae);

  // If this gets stuck on (and it shouldn't) this clears it!
  // NOTE: do NOT call this in ClearAndFocus() because it will turn off
  // our status panel during processing!
  if (CancelPanel->Visible)
    ResetProgress();

  if (SpellCheckForm)
    SpellCheckForm->ScClose();

  if (ReplaceTextForm)
    ReplaceTextForm->FrClose();

  if (TOCUndo)
    TOCUndo->ClearMost();

  // Clear will add an undo, saving SL_ORG...
  ClearAndFocus(true);

  SetWordWrap(false); // Set this off since word-wrap mode is read-only...

  // Set the TAB to "Process"
  if (PageControl1->ActivePage != PageControl1->Pages[0])
  {
    PageControl1->ActivePage = PageControl1->Pages[0];
    PageControl1Change(NULL);
  }

  FileSaveItem->Enabled = false;

  bIsColorFractal = false;

  // This saves us if the lock-counter ever has unmatched push/pops!
  utils->InitOnChange(tae, TaeEditChange);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::Original1Click(TObject *Sender)
{
  if (!Original1->Checked)
  {
    bIsColorFractal = false;

    LoadView(V_ORG);

    // If the find/replace dialog is active, reset it
    if (ReplaceTextForm)
      ReplaceTextForm->FrReset(true);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::RTF1Click(TObject *Sender)
{
  // If user clicks the Processed view menu item and there
  // is no RTF stream, try to process the current view's stream
  // instead...
  if (!RTF1->Checked)
  {
    bIsColorFractal = false;

    if (!tae->Modified && MS_RTF->Size > 0)
      LoadView(V_RTF);
    else if (tae->View == V_ORG || tae->View == V_IRC)
      DoShowRtf(false); // no status panel...

    // If the find/replace dialog is active, reset it
    if (ReplaceTextForm)
      ReplaceTextForm->FrReset(true);
  }
}
//---------------------------------------------------------------------------
// Having trouble with the tae control not having a blinking cursor after SHOW?
// Is DEBUG_ON true? (that's WHY!)
void __fastcall TDTSColor::ShowButtonClick(TObject *Sender)
{
  if (TOCUndo != NULL && (tae->View == V_ORG || tae->View == V_IRC))
  {
    // Add Undo point
    ONCHANGEW oc = utils->GetInfoOC(tae, NULL);
    TOCUndo->Add(UNDO_SHOW, 0, 0, oc, "");
  }

  DoShowRtf(true);
}
//---------------------------------------------------------------------------
// Don't name this DoShow() something in the system calls that!
void __fastcall TDTSColor::DoShowRtf(bool bShowStatus)
{
  utils->PushOnChange(tae);

// Don't think we need to stop play if we process to rtf do we?
//  try
//  {
//    if (bIsPlaying)
//    {
//      // Halt playback
//      TPlay* pb = new TPlay(this);
//      pb->DoStopPlay();
//      delete pb;
//    }
//  }
//  catch(...) {}

  if (tae->View == V_ORG)
  {
    SL_IRC->Text = SL_ORG->Text; // Copy SL_ORG to SL_IRC
    tae->View = V_IRC;

    // We set bTextWasProcessed in DoProcess() and check it there also.
    // When set and the view is V_RTF, DoProcess() sets Auto line-width
    // on temporarily so the text formatting won't be messed up for
    // repeated re-processings so the user can add layered borders.
    //
    // But when the user switches to V_ORG and then presses Show
    // we want to clear that flag so the next press of Process will
    // properly format the raw text
    bTextWasProcessed = false;
  }

  CpInit(2);

  // Change processing status panel... Processing to RTF format...
  if (bShowStatus)
    CpShow(STATUS[2]);

  try
  {
    linesInRtfHeader = ConvertIrcToRtf(SL_IRC, true, bShowStatus);

    if (linesInRtfHeader != 0 && MS_RTF != NULL)
    {
      // Save original position and carat
      TTaePosition* p = new TTaePosition(tae);
      p->SavePos = p->Position;

      int SaveView = tae->View;

      LoadView(V_RTF);

      // Restore original position and caret if refreshing an RTF view
      if (SaveView == V_RTF)
        p->Position = p->SavePos;

      delete p;
    }
  }
  __finally
  {
#if DEBUG_ON
    CpHide();
#else
    CpReset();
#endif
  }

  utils->PopOnChange(tae);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::IRC1Click(TObject *Sender)
{
  if (!IRC1->Checked)
  {
    bIsColorFractal = false;

    LoadView(V_IRC);

    // If the find/replace dialog is active, reset it
    if (ReplaceTextForm)
      ReplaceTextForm->FrReset(true);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::HTML1Click(TObject *Sender)
{
  if (!HTML1->Checked)
  {
    bIsColorFractal = false;

    LoadView(V_HTML);

    // If the find/replace dialog is active, reset it
    if (ReplaceTextForm)
      ReplaceTextForm->FrReset(true);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::RTFSource1Click(TObject *Sender)
{
  if (!RTFSource1->Checked)
  {
    bIsColorFractal = false;

    LoadView(V_RTF_SOURCE);

    // If the find/replace dialog is active, reset it
    if (ReplaceTextForm)
      ReplaceTextForm->FrReset(true);
  }
}
//---------------------------------------------------------------------------
int __fastcall TDTSColor::SetView(int NewView)
{
  utils->PushOnChange(tae);

  bool bHaveText = (tae->TextLength > 0) ? true : false;

  SetViewMenuHandlers(false); // Disable Menu OnClick Handlers

  int ReturnView;

  ViewMenu->Enabled = true;

  // Set menu Enable/Disable
  if (NewView == V_OFF || NewView == V_IRC ||
                                    NewView == V_ORG || NewView == V_RTF)
  {
    EffectsMenu->Enabled = true;

    // Set TaeEdit vars used to determine how much text was typed in
    // to maintain a buffer of raw text codes in MS_ORG or MS_IRC
    utils->SetOldLineVars(tae);
  }
  else
    EffectsMenu->Enabled = false;

  UpdateProcessButtons(NewView);

  PrintShortLineCount();
  UpdateViewMenuState();

  // Disable Checkmarks
  Original1->Checked = false;
  RTF1->Checked = false;
  IRC1->Checked = false;
  RTFSource1->Checked = false;
  HTML1->Checked = false;

  if (bHaveSpeller)
//  if (bHaveSpeller && NewView == V_RTF)
    EditSpellCheckItem->Enabled = true;
  else
    EditSpellCheckItem->Enabled = false;

  switch(NewView)
  {
    case V_ORG:

      Original1->Checked = true;
      ReturnView = V_ORG;
      ViewStatus->Caption = String(VIEWS[0]); // "View: Original";
      break;

    case V_RTF:

      RTF1->Checked = true;
      ReturnView = V_RTF;
      ViewStatus->Caption = String(VIEWS[1]); // "View: Processed";
      break;

    case V_IRC:

      IRC1->Checked = true;
      ReturnView = V_IRC;
      ViewStatus->Caption = String(VIEWS[2]); // "View: IRC";
      break;

    case V_HTML:

      HTML1->Checked = true;
      ReturnView = V_HTML;
      ViewStatus->Caption = String(VIEWS[3]); // "View: HTML";
      break;

    case V_RTF_SOURCE:

      RTFSource1->Checked = true;
      ReturnView = V_RTF_SOURCE;
      ViewStatus->Caption = String(VIEWS[4]); // "View: RTF Source";
      break;

    case V_OFF:

      ReturnView = V_OFF;
      ViewStatus->Caption = String(VIEWS[5]); // "View: OFF";
      break;

    default:

      ReturnView = V_OFF;
      ViewStatus->Caption = String(VIEWS[6]); // "View: Unknown";
      break;
  }

  // This was causing a annoying editing problem after a view-change
  // you clicked the mouse where you wanted to edit and as soon as
  // you typed the first insert-char the cursor went to the beginning
  // of the line and started to insert there :(
  // tae->SelStart = 0;

  // Don't switch if minimized
  // (SetFocus throws an exception sometimes!!!)
#if !DEBUG_ON
  try
  {
    if (WindowState != wsMinimized)
      tae->SetFocus();
  }
  catch(...)
  {
  }
#endif

  // If the find/replace dialog is active switch the view if we
  // changed to V_IRC
  if (ReplaceTextForm && tae->View != ReturnView && ReturnView == V_IRC)
    ReplaceTextForm->ChangeView(V_IRC);

  // Black text to write
  tae->SelAttributes->Color = clBlack;
  tae->SelAttributes->BackColor = clWhite;
  // This changes color of everything...
  //  tae->DefAttributes->Color = clBlack;
  //  tae->DefAttributes->BackColor = clWhite;

  SetViewMenuHandlers(true); // Enable Menu OnClick Handlers

  // Set Find/Replace enables
  if (EditFindItem->Enabled && !bHaveText)
  {
    EditFindItem->Enabled = false;
    EditReplaceItem->Enabled = false;
  }
  else if (!EditFindItem->Enabled && bHaveText)
  {
    EditFindItem->Enabled = true;
    EditReplaceItem->Enabled = true;
  }

  utils->PopOnChange(tae);

  return ReturnView;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SetViewMenuHandlers(bool bEnable)
{
  if (bEnable)
  {
    Original1->OnClick = Original1Click;
    RTF1->OnClick = RTF1Click;
    IRC1->OnClick = IRC1Click;
    HTML1->OnClick = HTML1Click;
    RTFSource1->OnClick = RTFSource1Click;
  }
  else
  {
    Original1->OnClick = NULL;
    RTF1->OnClick = NULL;
    IRC1->OnClick = NULL;
    HTML1->OnClick = NULL;
    RTFSource1->OnClick = NULL;
  }
}
//---------------------------------------------------------------------------
int __fastcall TDTSColor::LoadView(int NewView)
{
  if (SpellCheckForm != NULL) SpellCheckForm->ScClose();

  utils->PushOnChange(tae);

  int SaveSelStart = tae->SelStart;
  int SaveSelLength = tae->SelLength;
  int SaveLine = utils->GetLine(tae);
  int SaveView = tae->View;

  // Save this so we can return to previous V_ORG line
  if (SaveView == V_ORG)
    orgLinePosition = SaveLine;

  // We save the modified RTF text to avoid having to call ConvertToRTF()
  // for the entire file when switching from V_IRC back to V_RTF...
  // (modifications are made in the MS_IRC stream as you type into the
  // RTF view by the TaeEdit OnChange Event and by ThreadOnChange.cpp)
  if (SaveView == V_RTF && NewView != V_RTF && tae->Modified)
  {
    MS_RTF->Clear();
    tae->Lines->SaveToStream(MS_RTF);
    MS_RTF->Position = 0;
  }

  try
  {
    switch (NewView)
    {
      case V_ORG:

        if (SL_ORG->Count > 0)
        {
          // Highlight special codes and load into the main edit control
          utils->EncodeHighlight(SL_ORG, tae);

          tae->Line = orgLinePosition;

          // Switch view
          tae->View = SetView(V_ORG);
        }

      break;

      case V_RTF:
      {
        if (MS_RTF->Size == 0)
          break;

        TStringsW* sl = NULL;

        if (utils->IsIrcView())
          sl = SL_IRC;
        else if (utils->IsOrgView())
          sl = SL_ORG;

        // If View is Org or Irc adjust cursor...
        if (sl != NULL)
        {
          // Get index into the raw-codes buffer.
          int Idx = SaveSelStart + SaveLine;

          // Limit the index to the max raw-codes text
          // TextLength counts line-terminators as two chars!
          int IrcTextLength = tae->TextLength - tae->LineCount + 1;

          if (Idx > IrcTextLength)
            Idx = IrcTextLength;

          // Set Cursor in RTF to the same point as cursor in IRC or ORG
          SaveSelStart = utils->GetRealIndex(sl, Idx);

          if (tae->SelLength)
          {
            // Map the start and end indices of the IRC or ORG selected text
            // to the text in the buffer.
            Idx += SaveSelLength + utils->CountCRs(tae->SelTextW);
            SaveSelLength = utils->GetRealIndex(sl, Idx) - SaveSelStart;
          }
        }
        else
        {
          SaveSelStart = 0;
          SaveSelLength = 0;
        }

        // Display processed Rich Text in main window
        LoadLines(MS_RTF, false, false);

        // Set to beginning? Try to set to the line from the previous view
        // unless we are processing selected text from V_RTF
        if (SaveSelStart == 0 && SaveView != V_RTF)
        {
          int temp = SaveLine;

          if (SaveView == V_RTF_SOURCE)
            temp -= linesInRtfHeader;

          utils->SetLine(tae, temp);
        }
        else
        {
          tae->SelStart = SaveSelStart;
          tae->SelLength = SaveSelLength;
        }

        // Switch view
        tae->View = SetView(V_RTF);
      }
      break;

      case V_IRC:
      {
        if (SL_IRC->Count == 0)
          break;

        // Don't optimize here!
        // PROBLEM: if we optimize it messes up all previous Undo indices!!!!

        // The text in the rich-edit has one-char per cr/lf pair
        // stored in the stream - so we will need to compensate below
        int LinesSel = utils->CountCRs(tae->SelTextW);

        WideString SlText = SL_IRC->Text;
        int SlTextLen = SlText.Length();

        // Map the start and end indices of the selected text to the
        // text in the stream. First points to the First IRC-buffer
        // index corresponding to the Selected Region in TaeEdit, Last
        // points to the last+1. CI is the same as First but it points
        // to the start of any preceeding formatting codes... "Code Index"
        int iFirst, iLast, CI;
        utils->GetCodeIndices(SlText, iFirst, iLast, CI, SaveSelStart,
                                                        SaveSelLength);

        // Highlight special codes and load to edit-control
        utils->EncodeHighlight(SlText, tae);

        if (SaveView == V_RTF)
        {
          // Set Cursor in IRC text to the same point as
          // cursor in RTF or ORG text...
          int temp = SaveLine;

          // (Handle user entering text at end of a line of RTF text...)
          if (CI > 0 && CI <= SlTextLen)
          {
            if (SlText[CI] == C_CR)
              temp++;

            SaveSelStart = CI-temp;
          }

          if (SaveSelLength)
            SaveSelLength = (iLast-CI)-LinesSel;

          // Limit-checking...
          if (SaveSelStart >= SlTextLen)
          {
            SaveSelStart = -1;
            SaveSelLength = 0;
          }
          else if (SaveSelStart + SaveSelLength > SlTextLen)
            SaveSelLength = SlTextLen - SaveSelStart;
        }
        else
        {
          SaveSelStart = 0;
          SaveSelLength = 0;
        }

        // Set to beginning? Try to set to the line from the previous view
        if (!SaveSelStart)
        {
          int temp = SaveLine;

          if (SaveView == V_RTF_SOURCE)
            temp -= linesInRtfHeader;

          utils->SetLine(tae, temp);
        }
        else
        {
          tae->SelStart = SaveSelStart;
          tae->SelLength = SaveSelLength;
        }

        // Switch view
        tae->View = SetView(V_IRC);
      }

      break;

      case V_HTML:

        // If we edited text in RTF, IRC or ORG view - update the rtf-stream
        if (SL_IRC->Count > 0 && (tae->Modified || SL_HTM->Count == 0))
          ConvertIrcToHtml(false); // Convert MS_IRC to MS_HTM (HTML)

        if (SL_HTM->Count > 0)
        {
          // Display processed IRC source in main window
          LoadLines(SL_HTM, false, true);

          // Switch view
          tae->View = SetView(V_HTML);

          int temp = SaveLine; // Back to other view's line #

          if (SaveView == V_RTF_SOURCE)
            temp -= linesInRtfHeader;

          utils->SetLine(tae, temp);
        }

      break;

      case V_RTF_SOURCE:

        {
          // If we edited text in RTF, IRC or ORG view - update the rtf-stream
          if (SL_IRC->Count > 0 && (tae->Modified || MS_RTF->Size == 0))
          {
            // don't display status
            linesInRtfHeader = ConvertIrcToRtf(SL_IRC, false, false);

            if (linesInRtfHeader != 0 && MS_RTF != NULL)
            {
              // Display processed Rich Text source in main window
              LoadLines(MS_RTF, false, true);

              // Switch view
              tae->View = SetView(V_RTF_SOURCE);

              // Back to other view's line #
              int temp = SaveLine+linesInRtfHeader;

              utils->SetLine(tae, temp);
            }
          }
          else if (MS_RTF->Size > 0)
          {
            // Display processed Rich Text source in main window
            LoadLines(MS_RTF, false, true);

            // Switch view
            tae->View = SetView(V_RTF_SOURCE);

            // Back to other view's line #
            int temp = SaveLine+linesInRtfHeader;

            utils->SetLine(tae, temp);
          }
          else if (tae->View != V_RTF_SOURCE && tae->TextLength)
          {
            tae->PlainText = false;
            MS_RTF->Clear();
            tae->Lines->SaveToStream(MS_RTF);

            // Display processed Rich Text source in main window
            LoadLines(MS_RTF, false, true);

            // Switch view
            tae->View = SetView(V_RTF_SOURCE);

            // Back to other view's line #
            int temp = SaveLine+linesInRtfHeader;

            utils->SetLine(tae, temp);
          }
        }

      break;

      default:

        tae->View = SetView(V_OFF);
      break;
    }
  }
  catch(...)
  {
    tae->View = SetView(V_OFF);
  }

  UpdateProcessButtons(tae->View);

  tae->Modified = false;

  utils->PopOnChange(tae);

  return tae->View;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::UpdateProcessButtons(int View)
{
  if (View == V_RTF_SOURCE || View == V_HTML)
  {
    ProcessButton->Enabled = false;
    PopupProcess->Enabled = false;
  }
  else
  {
    ProcessButton->Enabled = true;
    PopupProcess->Enabled = true;
  }

  ProcessButton->Caption = String(FN[1]);
  ProcessButton->Invalidate();

  // "Show" Button
  if (View == V_RTF || View == V_ORG || View == V_IRC)
    ShowButton->Enabled = true;
  else
    ShowButton->Enabled = false;

  ShowButton->Caption = String(DS[243]);
  ShowButton->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ClearAndFocus(bool bClearSL, bool bLeaveOrg)
// bLeaveOrg defaults to false, it will retain the V_ORG view
{
  utils->PushOnChange(tae);

  // Clears tae edit-control unless view is V_ORG and bLeaveOrg
  // is set...
  if (!(bLeaveOrg && tae->View == V_ORG))
  {
    tae->Clear();
    bFileOpened = false,
    bFileSaved = false;
  }

  // Clear string-lists if bClearSL is set...
  if (bClearSL)
  {
    // Have to wait for any pending undos to finish being added!
    utils->WaitForThread();

    // Clear undo objects except UNDO_CLEARs
    if (TOCUndo != NULL)
    {
      // Delete all events up to and including the previous UNDO_CLEAR.
      // Call ClearMost() prior to adding the new UNDO_CLEAR!
      TOCUndo->ClearMost();

      // Save whatever we were working on (Add undo object)
      ONCHANGEW oc = utils->GetInfoOC();
      if (oc.p != NULL)
      {
        // here we want to save either the SL_IRC or SL_ORG stringlist
        TStringsW* sl = (TStringsW*)oc.p;
        if (sl->TotalLength)
          TOCUndo->Add(UNDO_CLEAR, 0, sl->TotalLength, oc, sl->Text);
      }
    }

    // Clear memory streams
    if (SL_ORG != NULL && !bLeaveOrg)
      SL_ORG->Clear();

    if (SL_IRC != NULL)
      SL_IRC->Clear();

    if (MS_RTF != NULL)
      MS_RTF->Clear();

    if (SL_HTM != NULL)
      SL_HTM->Clear();

    // Restore the line-width and the color of its edit-window
    WidthEditClick(NULL);
  }

  bEffectWasAdded = false;
  bTextWasProcessed = false;

  // Switch view
  if (!(bLeaveOrg && tae->View == V_ORG))
    tae->View = SetView(V_OFF);

  tae->PlainText = true;
  tae->Modified = false;
  tae->SelStart = 0;

  utils->PopOnChange(tae);
}
//---------------------------------------------------------------------------
int __fastcall TDTSColor::Outside(double rec, double imc, int k)
{
  double re = rec,
         im = imc,
         re2,
         im2;

  for (int ii = 0; ii < 3 - k; ii++)
  {
    re2 = re*re;
    im2 = im*im;

    if (re2 + im2 > 256.0)
      return(ii);

    im = 2*re*im + imc;
    re = re2 - im2 + rec;
  }
  return -1;
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::LoadLines(TMemoryStream* ms, bool bClearSL, bool bPlainText)
// bClearSL defaults false and bPlainText defaults true.
{
  if (!ms || !ms->Size)
    return(false);

  try
  {
    Application->ProcessMessages();
    utils->PushOnChange(tae);

    bool bTemp = bTextWasProcessed; // messy save of flag
    ClearAndFocus(bClearSL);
    bTextWasProcessed = bTemp;

    utils->SetRichEditFont(tae);
    tae->PlainText = bPlainText;
    ms->Position = 0;
    tae->Lines->LoadFromStream(ms);
    utils->PopOnChange(tae);
    Application->ProcessMessages();
  }
  catch(...)
  {
    utils->ShowMessageU(String(DS[95]) + "LoadLines()");
    return false;
  }

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::LoadLines(TStringsW* sl, bool bClearSL, bool bPlainText)
// bClearSL defaults false and bPlainText defaults true.
{
  if (sl == NULL || sl->Count <= 0)
    return false;

  try
  {
    Application->ProcessMessages();
    utils->PushOnChange(tae);

    bool bTemp = bTextWasProcessed; // messy save of flag
    ClearAndFocus(bClearSL);
    bTextWasProcessed = bTemp;

    utils->SetRichEditFont(tae);
    utils->LoadFromStringList(tae, sl, bPlainText);

    utils->PopOnChange(tae);
    Application->ProcessMessages();
  }
  catch(...)
  {
    utils->ShowMessageU(String(DS[95]) + "LoadLines()");
    return false;
  }

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::LoadLines(WideString wFile, bool bClearSL, bool bPlainText)
{
  if (SL_ORG == NULL)
    return false;

  // Clear streams and edit-control
  bool bTemp = bTextWasProcessed; // messy save of flag
  ClearAndFocus(bClearSL);
  bTextWasProcessed = bTemp;

  if (bPlainText)
  {
    Application->ProcessMessages();

    utils->WaitForThread();

    SL_ORG->LoadFromFile(wFile, bIsAnsiFile); // Load TStringsW

    if (SL_ORG->Count == 0)
      return false;

    Application->ProcessMessages();

    LoadView(V_ORG); // Load TaeEdit
  }
  else
  {
    //"YahCoLoRiZe can display a .rtf file but\n"
    //"cannot convert it to the native IRC code format\n"
    //"YahCoLoRiZe works with normally. There is no\n"
    //"converter at this time!", // 170
    if (utils->ShowMessageU(Handle, DS[170],
                      MB_ICONQUESTION|MB_OKCANCEL|MB_DEFBUTTON2) == IDCANCEL)
      return false;

    // Load memory-stream via a Unicode filename
    String sTemp = utils->ReadStringFromFileW(wFile);

    // this flag does nothing right now for a .rtf!
    //if (bIsAnsiFile)...

    MS_RTF->Clear();
    MS_RTF->WriteBuffer(sTemp.c_str(), sTemp.Length());
    MS_RTF->Position = 0;

    if (MS_RTF->Size == 0)
      return false;

    LoadView(V_RTF); // Load TaeEdit
  }

  return true;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::RestoreFactorySettings1Click(TObject *Sender)
{
  // "Restore Defaults?", // 168
  if (utils->ShowMessageU(Handle, DS[168],
                MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDYES)
  {
    ClearAndFocus(true);
    utils->PushOnChange(tae);
    InitAllSettings(true); // Init OnChange

    if (ThreadOnChange)
      ThreadOnChange->Flush();

    utils->InitOnChange(tae, TaeEditChange);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MonkeyMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  Application->CreateForm(__classid(TMonkeyForm), &MonkeyForm);
  MonkeyForm->ShowModal(); // Show monkey
  YcDestroyForm(MonkeyForm);
  MonkeyForm = NULL; 
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuShowHexClick(TObject *Sender)
{
//!!!!!!!!!!!!!!!!!
//  // Code to convert DefaultStrings table-sections to utf-8 ANSI on the clipboard...
//  String sOut;
//  sOut += "\"" + utils->AnsiToUtf8(String(TB)) + "\",\r\n";
//  sOut += "\"" + utils->AnsiToUtf8(String(LB)) + "\",\r\n";
//  sOut += "\"" + utils->AnsiToUtf8(String(RB)) + "\",\r\n";
//  for (int ii = 1; ii < 23; ii++)
//    sOut += "\"" + utils->AnsiToUtf8(String(WINGEDITMAINMENU[ii])) + "\",\r\n";
//  utils->SetClipboardText(sOut);

#if DIAG_SHOWHEX_IS_UTF8
    // diagnostic to paste raw ANSI text after clicking Tools->ShowHex
    // to convert a block of UTF-8 text to ANSI codes for DefaultStrings.cpp!
  DisplayAnsiToUtf8();
#else
  utils->ShowHex();
#endif
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::Help1Click(TObject *Sender)
{
  String Temp = HELPNAME;
  ShellExecute(Application->Handle, NULL, Temp.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PrinterPageSetupClick(TObject *Sender)
{
  // Allow Printing
  tae->PrintSupport = true;

  if (tae->TaePrint)
  {
    if (tae->TextLength == 0)
      utils->ShowMessageU(String(DS[80]));
    else
    {
      TTaePageSetupDialog* p = new TTaePageSetupDialog(this);

      if (p != NULL)
      {
        p->Execute(tae);
        delete p;
      }
      else utils->ShowMessageU(String(FN[14])); // "TDTSTrinity"
    }
  }
  else
    utils->ShowMessageU(String(FN[14])); // "TDTSTrinity"
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PrintPreview1Click(TObject *Sender)
{
//  if (!PrintPreview)
//    PrintPreview = new TPrintPreview(this);

//  PrintPreview->Preview(tae);
//  return;
//---------------------------------------
  if (TaePreviewForm)
    return;

  Application->CreateForm(__classid(TTaePreviewForm), &TaePreviewForm);

  // Allow Printing
  tae->PrintSupport = true;

  if (tae->TaePrint && TaePreviewForm)
  {
    utils->PushOnChange(tae);

    // Displayed in the dialog...
    WideString wTemp;
    if (bFileSaved)
      wTemp = SaveD + SaveF;
    else if (bFileOpened)
      wTemp = OpenD + OpenF;

    if (!wTemp.IsEmpty() && utils->FileExistsW(wTemp))
      tae->FileName = utils->GetAnsiPathW(wTemp);
    else
      tae->FileName = "";

    // This brings up the WYSIWYG dialog which creates print-page
    // representations on TPanel objects and paints them with a custom
    // paint-hook that calls RenderPage in TaeRichEditPrint.cpp.
    //
    // When the dialog closes upon pressing the print button, FilePrintClick
    // Calls PrintToPrinter in TaeRichEditPrint.cpp which is independant
    // of RenderPage (but the two don't produce identical results at present!)

    // Set the scaling factors
    TaePreviewForm->CorrectionX = regWysiwygPrintScaleX;
    TaePreviewForm->CorrectionY = regWysiwygPrintScaleY;

    // replace \pagebreak strings with \page for FormatRange to work...
    utils->PageBreaksToRtf(tae);

ReShow:
    TModalResult mr = TaePreviewForm->Execute(tae);

    // Save the scaling factors to keep in registry
    regWysiwygPrintScaleX = TaePreviewForm->CorrectionX;
    regWysiwygPrintScaleY = TaePreviewForm->CorrectionY;

    switch (mr)
    {
      case mrAll + 1:
      {
        // S.S. I think we need to set FRendDC
        tae->TaePrint->RendDC = TaePreviewForm->RendDC;

        FilePrintClick(NULL);
        break;
      }
      case mrAll + 2:
        PrinterPageSetupClick(NULL);
        goto ReShow; // loop back into dialog
    }

    TaePreviewForm->Release();
    TaePreviewForm = NULL;

    SL_HTM->Clear(); // clear so HTML gets re-generated

    if (utils->IsRtfView())
      DoShowRtf(false); // put the \pagebreak strings back
    else
      LoadView(tae->View);

    utils->PopOnChange(tae);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FilePrintClick(TObject *Sender)
{
#if LICENSE_ON
  if (PK->ComputeDaysRemaining() <= 0)
  {
    //"Visit www.yahcolorize.com to buy this product!"
    utils->ShowMessageU(KEYSTRINGS[4] +
            Iftf->Strings[INFO_WEB_SITE] + KEYSTRINGS[5]);
    return;
  }
#endif

  // Allow Printing
  tae->PrintSupport = true;

  if (tae->TaePrint)
  {
    if (tae->TextLength == 0)
      utils->ShowMessageU(String(DS[80]));
    else
    {
      TTaePrintDialog* p = new TTaePrintDialog(tae);

      if (p != NULL)
      {
        utils->PushOnChange(tae);

        // Displayed in the dialog...
        WideString wTemp;
        if (bFileSaved)
          wTemp = SaveD + SaveF;
        else if (bFileOpened)
          wTemp = OpenD + OpenF;

        if (!wTemp.IsEmpty() && utils->FileExistsW(wTemp))
          tae->FileName = utils->GetAnsiPathW(wTemp);
        else
          tae->FileName = "";

        // replace \pagebreak strings with \page for FormatRange to work...
        utils->PageBreaksToRtf(tae);

        // This will set: FRichEditPrint->RendDC = Printer()->Handle;
        p->TaeExecute(tae->FileName);

        delete p;

        if (utils->IsRtfView())
          DoShowRtf(false); // put the \pagebreak strings back
        else
          LoadView(tae->View);

        utils->PopOnChange(tae);
      }
      else
        utils->ShowMessageU(String(FN[14])); // "TDTSTrinity"
    }
  }
  else
    utils->ShowMessageU(String(FN[14])); // "TDTSTrinity"
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuColorFractalClick(TObject *Sender)
{
//  Form9->Caption = "Fractal Parameters";
//  Form9->ShowModal();

  F_Width = LineWidth/2;
  F_Height = LineWidth/2;

  // bigger Scale == smaller fractal within field
  // positive imcen moves fractal down, negative
  // imcen moved image up -- 0 is centered.
  F_Scale = 1.5/F_Width;
  F_Level = -64;

  F_Recen = -0.75;
  F_Imcen = 0.0;

  AutoWidth->Checked = true;
  GenColorFract();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::GenColorFract(void)
{
  WideString TextLine;

  int OldColor = -1;

  utils->PushOnChange(tae);

  TTaePosition* p;

  // Save original position and carat
  if (bIsColorFractal)
  {
    p = new TTaePosition(tae);
    p->RestoreSel = false;
    p->SavePos = p->Position;
  }
  else p = NULL;

  ClearAndFocus(true);

  for (int y = 0; y < LineWidth; y++)
  {
    TextLine.Empty();

    utils->WriteColors(Afg, Abg, TextLine);

    for (int x = 0; x < LineWidth; x++)
    {
      int count = Outside(F_Scale*(x - F_Width) + F_Recen, F_Scale*(F_Height - y) + F_Imcen, F_Level);

      int TempColor = (int)(count % 16);

      if (count > -1)
      {
        if (TempColor == IRCBLACK)
          TempColor = IRCWHITE;

        if (TempColor != OldColor)
          TextLine += String(CTRL_K) + String(TempColor);
      }
      else if (TempColor != OldColor)
        TextLine += String(CTRL_K) + String(IRCBLACK);

      char c;

      switch(count)
      {
        case 0: c = 'a'; break;
        case 1: c = 'b'; break;
        case 2: c = 'c'; break;
        case 3: c = 'd'; break;
        case 4: c = 'e'; break;
        case 5: c = 'f'; break;
        case 6: c = 'g'; break;
        case 7: c = 'h'; break;
        case 8: c = 'i'; break;
        case 9: c = 'j'; break;
        case 10: c = 'x'; break;
        default: c = 'z'; break;
      }

      TextLine += c;

      OldColor = TempColor;
    }

    SL_ORG->Add(TextLine);
  }

  utils->PopOnChange(tae);

  LoadView(V_ORG);
  DoProcess(true);

  // Keep image centered as we repeatedly click to magnify or move it
  if (p != NULL)
  {
    p->Position = p->SavePos;
    delete p;
  }

  bIsColorFractal = true;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::HintTimerEvent(TObject *Sender)
{
  if (HintTimer)
  {
    if (HintWindow)
    {
      delete HintWindow;
      HintWindow = NULL;
    }

    delete HintTimer;
    HintTimer = NULL;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::TaeEditMouseDown(TObject *Sender,
                      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button != mbLeft || bIsColorFractal) return;

  if (tae->SelLength) tae->SelLength = 0; // clear selection

  // Cancel the find/replace...
// May need to click to cut copy something to paste in the replace edit box!!!!!
//  if (ReplaceTextForm && ReplaceTextForm->Visible)
//    ReplaceTextForm->Close();

  // Hack to get the fg text you type in visible if the BG is dark
  // or a line is all black spaced - otherwise the fg text is black
  // any you can't see what you type.
  if (tae->View == V_RTF)
  {
    if (utils->IsDarkColor(Abg))
    {
      tae->DefaultRtfFileFont->Color = clWhite;
      tae->SelAttributes->Color = clWhite;
    }
    else
    {
      tae->DefaultRtfFileFont->Color = utils->YcToTColor(Afg);
      tae->SelAttributes->Color = utils->YcToTColor(Afg);
    }
  }

  PrintLineCount();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::TaeEditMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (ReplaceTextForm)
    ReplaceTextForm->FrReset(true);

  if (SpellCheckForm)
    SpellCheckForm->ScClose();

  if (Button != mbLeft) return;

  if (tae->SelLength) bIsColorFractal = false;

  if (bIsColorFractal)
  {
    // X is 0 if mouse is left up to 1000 or so (pixels) as you click farther right
    // Y is 0 if mouse is at the top up to 500 or so (pixels) as you click farther down
    if (GetKeyState(VK_UP) & 0x8000)
      F_Imcen += F_Scale*(F_Height*0.1);
    else if (GetKeyState(VK_DOWN) & 0x8000)
      F_Imcen -= F_Scale*(F_Height*0.1);
    else if (GetKeyState(VK_LEFT) & 0x8000)
      F_Recen -= F_Scale*(F_Width*0.1);
    else if (GetKeyState(VK_RIGHT) & 0x8000)
      F_Recen += F_Scale*(F_Width*0.1);
    else if (Shift.Contains(ssShift))
      F_Scale += F_Scale*0.1; // up 10%
    else
      F_Scale -= F_Scale*0.1; // Down 10%

    GenColorFract();
    return;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::TaeEditKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
  if (ReplaceTextForm)
    ReplaceTextForm->FrReset(true);

  if (SpellCheckForm)
    SpellCheckForm->ScClose();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::TaeEditKeyPress(TObject *Sender, char &Key)
{
  if (isalnum(Key))
  {
    if (tae->SelAttributes->Color != clBlack)
      tae->SelAttributes->Color = clBlack;
    if (tae->SelAttributes->BackColor != clWhite)
      tae->SelAttributes->BackColor = clWhite;
  }
  else if (Key == C_TAB)
  {
    int SaveSelStart = tae->SelStart;
    PASTESTRUCT ps = TaeEditPaste(false, utils->GetTabStringW(regTabs));

    if (ps.error == 0)
      tae->SelStart = SaveSelStart+ps.delta;

    Key = 0;
    return;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::TaeEditKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
  // We start getting a mess if Application->ProcessMessages() causes
  // a paste or cut when one is already in-progress!
  if (tae->LockCounter != 0)
  {
    Key = 0; // ignore key
    return;
  }

  // Want Paste even if no text!
  if (Key == VK_INSERT && Shift.Contains(ssShift))
  {
    EditPasteColor(NULL);
    Key = 0; // Handled here no further processing!
    return;
  }

  if (Key == VK_RETURN)
  {
    // Manual page-break on Ctrl + Enter
    if (Shift.Contains(ssCtrl))
    {
      PageBreak1Click(NULL);

      Key = 0;
      return;
    }

    // This little trick keeps a phantom space from appearing at the end
    // of each new-line...
    tae->SelAttributes->BackColor = clWindow;
  }

  if (tae->Lines->Count == 0)
    return;

  if (Key == VK_UP || Key == VK_DOWN) // Scrolling text
    PrintLineCount();
  else if (Key == VK_DELETE)
  {
    if (tae->SelLength)
    {
      if (Shift.Contains(ssShift))
        // Shift+Del
        TaeEditCutCopy(true, true);
      else
        // Del
        TaeEditCutCopy(true, false);

      Key = 0;
    }
  }
  else if (Key == VK_RETURN)
  {
    // Auto process & play on CR
    if (!bIsPlaying && PlayOnCR->Checked)
    {
      DoProcess(true);

      TPlayback* pb = new TPlayback(this);
      pb->DoStartPlay(PlayTime/4);
      delete pb;

      ClearAndFocus(true);
      Key = 0;
    }
  }
  else if (Key == VK_SPACE)
  {
    if (tae->View == V_RTF && bIsPlaying && !Timer1->Enabled) // paused?
    {
      TPlay* pb = new TPlay(this);

      if (!pb->SendToClient(false))
      {
#if DEBUG_ON
        CWrite( "\r\nError in play->SendToClient(false)...\r\n");
#endif
      }

      delete pb;
      Key = 0;
    }
  }
  else if (Key == VK_F3)
  {
    if (ReplaceTextForm == NULL)
      DoFindReplace(false);
    else if (ReplaceTextForm->FindFirst)
    {
      // Open dialog modeless (floating) until user closes it
      ReplaceTextForm->EnableReplace = false;
      ReplaceTextForm->Show();
    }

    ReplaceTextForm->FrFind(true);

    Key = 0;
  }
  else if (Key == VK_F4)
  {
    if (ReplaceTextForm == NULL)
      DoFindReplace(false);
    else if (ReplaceTextForm->FindFirst)
    {
      // Open dialog modeless (floating) until user closes it
      ReplaceTextForm->EnableReplace = false;
      ReplaceTextForm->Show();
    }

    ReplaceTextForm->FrFind(false);

    Key = 0;
  }
  // Here, we want to see if the next OnChange from TaeRichEdit
  // is a deltaLength of -1 to -4 chars... that means the user typed in
  // "fe" and hit Alt-X or typed in "0abcd" and hit Alt-x and those chars
  // were replaced by a single Unicode char.
  else if (this->bUnicodeKeySequence)
    this->bUnicodeKeySequence = false;
  else if (Key == 'X' && Shift.Contains(ssAlt))
    this->bUnicodeKeySequence = true;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PrintShortLineCount(void)
{
  // Print line-count in status bar
  StatusBar->Font->Color = clBlue;

  int CharCt;
  if (tae->View == V_IRC || tae->View == V_RTF)
    CharCt = SL_IRC->TotalLength;
  else if (tae->View == V_ORG)
    CharCt = SL_ORG->TotalLength;
  else
    CharCt = tae->TextLength;

  // Line Count
  StatusBar->Panels->Items[SB_COUNT]->Text = String(DS[97]) + " " +
     String(tae->Lines->Count) + " (" + String(CharCt) + " chars)";
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PrintLineCount(void)
{
  // Don't read stream or change its position in a routine called
  // by an event handler if main-level program is accessing it...
  if (tae->LockCounter != 0)
    return;

  int Width, CharCt;
  int LineCt = utils->GetLine(tae);

  String sCaption;

  if (utils->IsRtfIrcOrgView())
  {
    TStringsW* sl = (tae->View == V_ORG) ? SL_ORG : SL_IRC;

    if (LineCt < sl->Count)
    {
      Width = utils->GetRealLineLengthFromSL(sl, LineCt);
      CharCt = sl->GetString(LineCt).Length();

      sCaption = String(DS[115]) + " " + String(LineCt+1) +
                             "  " + String(DS[91]) + " " + String(Width) +
                                  " (" + String(CharCt) + " chars)";
    }
  }
  else
  {
    Width = tae->Lines->Strings[LineCt].Length();

    sCaption = String(DS[115]) + " " + String(LineCt+1) +
                             "  " + String(DS[91]) + " " + String(Width);
  }

  // Print line-count in status bar
  StatusBar->Font->Color = clBlue;
  StatusBar->Panels->Items[SB_COUNT]->Text = sCaption;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuRandomNumClick(TObject *Sender)
{
  // ReInit random number generator
  randomize();

  utils->ShowMessageU(String(DS[82]));
}
//---------------------------------------------------------------------------
String __fastcall TDTSColor::YahELiteRandomColor(bool StrType, bool bRgbRand)
{
    String NewColor;

    if (bRgbRand)
    {
      if (StrType)
        NewColor = YAH_RGB(IntToHex(random(0x1000000), 6));
      else
        // This format is used in the "font" bgcolor tag
        NewColor = "\"#" + IntToHex(random(0x1000000), 6) + "\"";
    }
    else
    {
      if (StrType)
        NewColor = YAH_RGB(IntToHex(palette[random(paletteSize)],6));
      else
        // This format is used in the "font" bgcolor tag
        NewColor = "\"#" + IntToHex(palette[random(paletteSize)],6) + "\"";
    }

    return NewColor;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuChangeColorPaletteClick(TObject *Sender)
{
  TColorDialog* ColorDialog = new TColorDialog(this);

  utils->SetCustomColorsDialogFromPalette(ColorDialog);

  // Show user-defined color-dialog
  ColorDialog->Options << cdFullOpen;

  ColorDialog->Color = FgPanel->Color;

  if (!ColorDialog->Execute())
  {
    delete ColorDialog;
    return; // Cancel-button
  }

  utils->GetPaletteFromCustomColorsDialog(ColorDialog);

  UpdateColorButtons();
  delete ColorDialog;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FavoriteColorsClick(TObject *Sender)
{
  Application->CreateForm(__classid(TFavColorsForm), &FavColorsForm);

  if (FavColorsForm->ShowModal() == mrOk)
  {
    if (IsYahTrinPal())
    {
      Foreground = utils->RgbToYc(FavColorsForm->FgColor);
      Background = utils->RgbToYc(FavColorsForm->BgColor);
    }
    else // need to find closest palette match to rgb color if IRC!
    {
      Foreground = utils->ResolveRGBLocal(-FavColorsForm->FgColor);
      Background = utils->ResolveRGBLocal(-FavColorsForm->BgColor);
    }
    UpdateColorButtons(); // Panel needs to be painted, set Afg, Abg, Etc.
  }

  YcDestroyForm(FavColorsForm);
  FavColorsForm = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::Bold3Click(TObject *Sender)
{
  if (IsRawCodeMode())
    tae->SelTextW = WideString(CTRL_B); // OnChange handler handles this
  else
  {
    EParm3 = -1;
    AddEffect(E_BOLD);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::Underline3Click(TObject *Sender)
{
  if (IsRawCodeMode())
    tae->SelTextW = WideString(CTRL_U); // OnChange handler handles this
  else
  {
    EParm3 = -1;
    AddEffect(E_UNDERLINE);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::Italics2Click(TObject *Sender)
{
  if (IsRawCodeMode())
    tae->SelTextW = WideString(CTRL_R); // OnChange handler handles this
  else
  {
    EParm3 = -1;
    AddEffect(E_ITALIC);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PageBreak1Click(TObject *Sender)
{
  // Two completely different ways of doing the same thing...
  // decided to go with the non-paste method - pasting leaves
  // some codes that would need to be optimized out...

/*
  tae->SelLength = 0;

  WideChar wC;
  WideString wPaste;

  if (tae->SelStart > 0)
  {
    tae->SelStart = tae->SelStart - 1;
    wC = tae->SelTextW[1];
    tae->SelStart = tae->SelStart + 1;
    if (wC != C_CR)
      wPaste = String(CRLF);
  }

  wPaste += String(C_FF);

  wC = tae->SelTextW[1];

  if (wC != C_CR)
    wPaste += String(CRLF);

  PASTESTRUCT ps = TaeEditPaste(false, wPaste);

  if (ps.error == 0)
    tae->SelStart += ps.delta-ps.lines;

  // If V_RTF have to redraw from SL_IRC because the rich-edit control
  // will show a small square for the inserted C_FF char...
  if (tae->View == V_RTF)
    DoShowRtf(false);
*/

  utils->PushOnChange(tae);

  TPoint p = tae->CaretPos;
  TStringsW* sl;
  WideString wS;
  int idx;

  tae->SelLength = 0; // deselect text

  if (utils->IsRtfIrcOrgView())
  {
    sl = utils->IsOrgView() ? SL_ORG : SL_IRC;

    // get only the raw-codes line the caret is on - the line-index is the same
    // in both the RTF view and in the SL_IRC string-list...
    wS = sl->GetString(p.y);

    if (utils->IsRtfView())
      idx = utils->GetCodeIndex(wS, p.x, true);
    else
      idx = p.x;
  }
  else // some other view like html or rtf-source
  {
    sl = NULL;
    idx = p.x;
  }


  // If we are adding the pagebreak after a complete line we
  // don't need to prefix the \pagebreak with CRLF...
  WideString wText;
  bool bNoCrAtCaret, bNoCrBeforeCaret;
  if (tae->SelStart > 0)
  {
    tae->SelStart -= 1;
    wText = tae->SelTextW; // get the char at the caret
    tae->SelStart += 1;
//#if DEBUG_ON
//    CWrite("\r\nBefore: " + IntToHex((int)wText[1], 6) + "\r\n");
//#endif
    if (!wText.IsEmpty())
      bNoCrBeforeCaret = (wText[1] != C_CR) ? true : false;
    else
      bNoCrBeforeCaret = true;
  }
  else
    bNoCrBeforeCaret = true;

  wText = tae->SelTextW; // get the char at the caret
//#if DEBUG_ON
//  CWrite("\r\nAt: " + IntToHex((int)wText[1], 6) + "\r\n");
//#endif
  if (!wText.IsEmpty())
    bNoCrAtCaret = (wText[1] != C_CR) ? true : false;
  else
    bNoCrAtCaret = true;

  bool bInsertBefore = (bNoCrBeforeCaret && bNoCrAtCaret) ||
                                (bNoCrBeforeCaret && !bNoCrAtCaret);
  bool bInsertAfter = (bNoCrBeforeCaret && bNoCrAtCaret) ||
                                (!bNoCrBeforeCaret && bNoCrAtCaret);

  WideString wFormFeed = PAGE_BREAK;

  if (bInsertBefore)
    wFormFeed = utils->InsertW(wFormFeed, CRLF, 1);

  if (bInsertAfter)
  {
    // get state
    WideString wState;
    utils->GetState(wS, wState, idx, true, false);
    wFormFeed += CRLF + wState;
  }

  // Problem: The rich-edit can't graphically display page-breaks
  // so we use our own \pagebreak marker in the text and replace those
  // strings at print-preview time with \page in the RTF text.

  // Add page-break to appropriate string-list (if any)
  if (sl != NULL)
  {
    // Insert cr/lf + page-break + cr/lf into line p.y at index p.x
    // form a line that is allowed to have cr/lfs
    wS = utils->InsertW(wS, wFormFeed, idx+1);

    // flatten wS into the string-list (will add new-lines and add state
    // at cr/lfs)
    TPoint ptDelta = sl->SetStringEx(wS, p.y);

    if (TOCUndo != NULL)
    {
      // Add Undo
      ONCHANGEW oc = utils->GetInfoOC();
      TOCUndo->Add(UNDO_INS, idx, ptDelta.x, oc, "");
    }
  }

  // Add page-break to RTF
  String sText;

  if (bInsertBefore && bInsertAfter)
//  {
//#if DEBUG_ON
//    CWrite("\r\n(bInsertBefore && bInsertAfter)\r\n");
//#endif
    sText = "{\\rtf\\plain\\highlight0\\par \\\\pagebreak\\highlight0\\par}";
//  }
  else if (bInsertBefore)
//  {
//#if DEBUG_ON
//    CWrite("\r\n(bInsertBefore)\r\n");
//#endif
    sText = "{\\rtf\\plain\\highlight0\\par \\\\pagebreak\\highlight0}";
//  }
  else if (bInsertAfter)
//  {
//#if DEBUG_ON
//    CWrite("\r\n(bInsertAfter)\r\n");
//#endif
    sText = "{\\rtf\\plain\\highlight0 \\\\pagebreak\\highlight0\\par}";
//  }
  else
//  {
//#if DEBUG_ON
//    CWrite("\r\n(NONE)\r\n");
//#endif
    sText = "{\\rtf\\plain\\highlight0 \\\\pagebreak\\highlight0}";
//  }

  tae->SelTextRtf = sText;

//  tae->SelTextW = wFormFeed;

  // details... details...
  utils->SetOldLineVars(tae);
  tae->Modified = true;
  bSaveWork = true;
  bTextWasProcessed = false;
  PrintShortLineCount();

  utils->PopOnChange(tae);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::CtrlPushClick(TObject *Sender)
{
  if (IsRawCodeMode())
    tae->SelTextW = WideString(CTRL_PUSH); // OnChange handler handles this
  else
  {
    EParm3 = -1;
    AddEffect(E_PUSHPOP);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::CtrlPopClick(TObject *Sender)
{
  if (IsRawCodeMode())
    tae->SelTextW = WideString(CTRL_POP); // OnChange handler handles this
  else
  {
    EParm3 = -1;
    AddEffect(E_PUSHPOP);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PlainText1Click(TObject *Sender)
{
  if (IsRawCodeMode())
    tae->SelTextW = WideString(CTRL_O);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FontType1Click(TObject *Sender)
// Called when user clicks Effects->Font->Type
{
  PUSHSTRUCT ps = GetState();

  int iType;

  if (ps.fontType > 0)
    iType = ps.fontType;
  else
    iType = cType;


  if (SelectFontType(U16(DS[213]), iType))
  {
    if (IsRawCodeMode())
    {
      int SaveSelStart = tae->SelStart;
      WideString sTemp = utils->FontTypeToString(iType);
      TaeEditPaste(true, sTemp);
      tae->SelStart = SaveSelStart + sTemp.Length();
    }
    else
    {
      EParm1 = iType;
      EParm3 = -1;
      AddEffect(E_FONT_TYPE);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FontSize1Click(TObject *Sender)
// Called when user clicks Effects->Font->Size
{
  PUSHSTRUCT ps = GetState();

  int iSize;

  if (ps.fontSize > 0) iSize = ps.fontSize;
  else iSize = cSize;

  if (SelectFontSize(U16(DS[214]), iSize))
  {
    if (IsRawCodeMode())
    {
      int SaveSelStart = tae->SelStart;
      WideString sTemp = utils->FontSizeToString(iSize);
      TaeEditPaste(true, sTemp);
      tae->SelStart = SaveSelStart + sTemp.Length();
    }
    else
    {
      EParm1 = iSize;
      EParm3 = -1;
      AddEffect(E_FONT_SIZE);
    }
  }
}
//---------------------------------------------------------------------------
PUSHSTRUCT __fastcall TDTSColor::GetState(void)
// Figures out the state at the beginning of a selection or at the beginning of a document
// if nothing is selected - it only scans one string in the document and so is fast!
{
  PUSHSTRUCT ps;

  int iLine = tae->SelLength > 0 ? tae->Line : 0; // The 0-based line index
  int iColumn = tae->Column; // Gives the real character index into a line

  WideString S;

  if (iLine >= 0 && iLine < tae->Lines->Count)
    S = SL_IRC->GetString(iLine);

  if (!S.IsEmpty())
  {
    if (tae->View == V_RTF)
      utils->SetStateFlags(S, iColumn, ps);
    else if (tae->View == V_ORG || tae->View == V_IRC || tae->View == V_OFF)
    {
      if (tae->SelLength > 0)
      {
        int iReal = utils->GetRealIndex(S, iColumn);
        utils->SetStateFlags(S, iReal, ps);
      }
    }
  }

  return ps;
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::IsRawCodeMode(void)
{
  return (tae->View == V_ORG || tae->View == V_IRC ||
                                  tae->View == V_OFF) && !tae->SelLength;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuEmoticonsClick(TObject *Sender)
{
  try
  {
    SmileyForm = new TSmileyForm(this);

    if (SmileyForm != NULL)
    {
      SmileyForm->Left = Left - SmileyForm->Width/3;
      if (SmileyForm->Left < 0)
        SmileyForm->Left = 0;
      SmileyForm->Top = Top - SmileyForm->Height/2;
      if (SmileyForm->Top < 0)
        SmileyForm->Top = 0;
      SmileyForm->SmileyTitle = DS[182]; // "Insert Emoticon Shortcut..."
      SmileyForm->Show();
    }
  }
  catch(...)
  {
    utils->ShowMessageU("Unable to open Emoticons!");
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SelectBlendPreset(String Topic)
{
  Application->CreateForm(__classid(TBlendChoiceForm), &BlendChoiceForm);

  BlendChoiceForm->Caption = Topic;
  BlendChoiceForm->ShowModal();
  YcDestroyForm(BlendChoiceForm);
  BlendChoiceForm = NULL; 
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::wwwYahCoLoRiZecom1Click(TObject *Sender)
{
  // launch default web-browser
  //ShellExecute(Handle, "open", "iexplore.exe",
  //                 Iftf->Strings[INFO_WEB_SITE].c_str(), NULL, SW_SHOW);
  ShellExecute(NULL, "open", Iftf->Strings[INFO_WEB_SITE].c_str(),
                                                   NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::UpdateRgbRandLabels(void)
// Labels that appear just to the right of the color-set panels on
// the process tab. They indicate the status of the "Full spectrum"
// random color flags.
{
  if (IsYahTrinPal() && bRgbRandFG)
  {
    FgRgbStatLabel->Visible = true;
    FgRgbStatLabel->Caption = "rgb";
  }
  else
  {
//    FgRgbStatLabel->Visible = false;
    FgRgbStatLabel->Caption = "   "; // space to click on!
  }

  if (IsYahTrinPal() && bRgbRandBG)
  {
    BgRgbStatLabel->Visible = true;
    BgRgbStatLabel->Caption = "rgb";
  }
  else
  {
//    BgRgbStatLabel->Visible = false;
    BgRgbStatLabel->Caption = "   "; // space to click on!
  }

  if (IsYahTrinPal() && bRgbRandWing)
  {
    WingRgbStatLabel->Visible = true;
    WingRgbStatLabel->Caption = "rgb";
  }
  else
  {
//    WingRgbStatLabel->Visible = false;
    WingRgbStatLabel->Caption = "   "; // space to click on!
  }
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::ComputeAllColors(int ActualLineCount, int IRCLineCount)
// Sets Afg, Abg, Awc for all conditions.
{
  if (Foreground == IRCVERTBLEND && FG_BlendEngine != NULL &&
                  FG_BlendEngine->EBC && FG_BlendEngine->BlendEnabledCount >= 2)
  {
    int Temp;

    if (IRCLineCount == 0)
      Temp = FG_BlendEngine->Init(ActualLineCount); // ActualLineCount is computed on the dummy iteration
    else
      Temp = FG_BlendEngine->Blend(IRCLineCount);

    if (Temp == BLEND_ERR1)
      Afg = -utils->BlendColorToRgb(FG_BlendEngine->EBC[0]); // first blend button that's enabled
    else if (Temp != NO_COLOR)
      Afg = Temp;
  }
  else if (Foreground == IRCRANDOM)
  {
    // Get next random color
    for (int timer = 0; timer < 16; timer++)
    {
      Afg = utils->ConvertColor(Foreground, bRgbRandFG);

      if (Afg != PrevFG)
        break;
    }
  }
  else
    Afg = utils->ConvertColor(Foreground, bRgbRandFG);

  PrevFG = Afg;

  if (Background == IRCVERTBLEND && BG_BlendEngine != NULL &&
                BG_BlendEngine->EBC && BG_BlendEngine->BlendEnabledCount >= 2)
  {
    int Temp;

    if (IRCLineCount == 0)
      Temp = BG_BlendEngine->Init(ActualLineCount); // ActualLineCount is computed on the dummy iteration
    else
      Temp = BG_BlendEngine->Blend(IRCLineCount);

    if (Temp == BLEND_ERR1)
      Abg = Abl; // Set bg to blend BG color
    else if (Temp != NO_COLOR)
      Abg = Temp;
  }
  else if (Background == IRCRANDOM)
  {
    // Get next random color
    for (int timer = 0; timer < 16; timer++)
    {
      Abg = utils->ConvertColor(Background, bRgbRandBG);

      if (Abg != PrevBG && Abg != Afg)
        break;
    }
  }
  else if (Background == IRCHORIZBLEND)
    Abg = Abl; // Set bg to blend BG color
  else
    Abg = utils->ConvertColor(Background, bRgbRandBG);

  PrevBG = Abg;

  // Get next random color
  if (WingColor == IRCRANDOM)
  {
    for (int timer = 0; timer < 16; timer++)
    {
      Awc = utils->ConvertColor(WingColor, bRgbRandWing);

      if (Awc != PrevAwc && Awc != Afg && Awc != Abg)
        break;
    }
  }
  else
    Awc = utils->ConvertColor(WingColor, bRgbRandWing);

  PrevAwc = Awc;
}
//---------------------------------------------------------------------------
// Registry Functions
//---------------------------------------------------------------------------
void __fastcall TDTSColor::LoadBlendMemoryFromDefaultPreset(const int Init[], BlendColor* BC)
{
  if (BC != NULL)
  {
    for (int ii = 0; ii < MAXBLENDCOLORS; ii++)
    {
      BC[ii] = utils->RgbToBlendColor(Init[ii]);
      BC[ii].enabled = true;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::GetRegInfo(bool RestoreFactory)
{
  const String RegKey = REGISTRY_KEY;

  TRegistry* MyRegistry = new TRegistry();
  if (MyRegistry == NULL) return;

  MyRegistry->RootKey = HKEY_CURRENT_USER;

  TotalWings = DEF_TOTAL_WINGS;

  LeftWings->Clear();
  RightWings->Clear();

  LeftWings->Add(U16(WINGEDITMAINMENU[16]), (void*)true);
  RightWings->Add(U16(WINGEDITMAINMENU[17]));

  LeftWings->Add(U16(WINGEDITMAINMENU[18] + ' '), (void*)true);
  RightWings->Add(U16(' ' + WINGEDITMAINMENU[18]));

  LeftWings->Add(U16(WINGEDITMAINMENU[6]), (void*)true);
  RightWings->Add(U16(WINGEDITMAINMENU[7]));

  LeftWings->Add(U16(WINGEDITMAINMENU[14]), (void*)true);
  RightWings->Add(U16(WINGEDITMAINMENU[15]));

//  LeftWings->Add(U16(WINGEDITMAINMENU[22]));
//  RightWings->Add(U16(WINGEDITMAINMENU[22]));

  TopBorder = U16(TB);
  LeftSideBorder = U16(LB);
  RightSideBorder = U16(RB);

  regTabs = TABSINIT;
  regLmargin = LMARGINIT;
  regRmargin = RMARGINIT;
  regTmargin = TMARGINIT;
  regBmargin = BMARGINIT;
  regIndent = INDENTINIT;
  regSpacing = SPACINGINIT;
  regWidth = LineWidth = WIDTHINIT;
  regHeight = LineHeight = HEIGHTINIT;

  regWysiwygPrintScaleX = DEF_SCALE_X;
  regWysiwygPrintScaleY = DEF_SCALE_Y;

  Background = BACKGROUNDINIT;
  Foreground = FOREGROUNDINIT;
  WingColor = WINGCOLORINIT;
  taeWindowColor = WINDOWCOLORINIT;
  BorderBackground = BORDERBACKGROUNDINIT;
  BlendBackground = BLENDBACKGROUNDINIT;

  Wingdings->OnClick = NULL;
  Wingdings->Checked = ALLOW_WINGDINGS;
  Wingdings->OnClick = WingdingsClick;

  Borders->OnClick = NULL;
  Borders->Checked = ALLOW_BORDERS;
  Borders->OnClick = BordersClick;

  bSideEnabled = SIDES_ENABLED;
  bTopEnabled = TOPBOTTOM_ENABLED;

  PlayTime = PLAYTIME;
  BurstInterval = BURSTINTERVAL;

  PlayChan = STATUSCHAN;

  GRooms->Clear();
  roomIndex = 0;

  largestPlayLineWidth = 0;
  largestPlayLineBytes = 0;

  GDictionary = U16(DEF_DICTIONARY);

  this->Cli = DEF_CLIENT;

  Default_Playfile = utils->GetTempFileW(U16(FN[6]));

  bPadSpaces = NO_SPACES;
  bUseDLL = USE_DLL;
  bUseFile = USE_FILE;
  bSendUtf8 = SEND_UTF8;
  bStripFont = STRIP_PLAYBACK_FONT;
  GMaxPlayXmitBytes = DEF_PLAYBUFSIZE;
  GLineBurstSize = LINEBURSTSIZE;

  PackText->Checked = PACK_TEXT;
  LeftJustify->Checked = LEFT_JUSTIFY;
  AutoWidth->Checked = AUTO_WIDTH;
  AutoHeight->Checked = AUTO_SPACING;
  PlayOnCR->Checked = PLAY_ON_CR;
  PlayOnCX->Checked = PLAY_ON_CX;
  SwiftMixSync->Checked = SWIFTMIX_SYNC;
  PackLines->Checked = PACK_LINES;
  ExtendPalette->Checked = EXTEND_PALETTE;
  AutoDetectEmotes->Checked = BYPASS_SMILEYS;
  Margins->Checked = MARGINS_ENABLED;
  TabsToSpaces->Checked = DEF_TABS_TO_SPACES_CHECKED;

  // this can actually have 3 modes but the UI only supports 2
  // ... set the "manual" radio-buttion in the Tabs tab
  #if (DEF_TAB_MODE == FLATTEN_TABS_SINGLE_SPACE)
  {
    TabsStripRB->Checked = true;
    TabsFlattenRB->Checked = false;
  }
  #else
  {
    TabsStripRB->Checked = false;
    TabsFlattenRB->Checked = true;
  }
  #endif
  
  // Color Blender
  BlendScopeRadioButtons->ItemIndex = BLENDSCOPE;
  BlendDirectionRadioButtons->ItemIndex = BLENDDIRECTION;
  RGBThreshold = RGBTHRESHOLD;

  // Load BlendColors structs with Preset1 defaults
  // (SaveToRegistry() saves the internal memory into
  // BlendPreset)
  fG_BlendPreset = FGBLENDPRESET;
  LoadBlendMemoryFromDefaultPreset(BlendPreset0Init, fG_BlendColors);
  bG_BlendPreset = BGBLENDPRESET;
  LoadBlendMemoryFromDefaultPreset(BlendPreset1Init, bG_BlendColors);

  bRgbRandFG = RGBRANDFG;
  bRgbRandBG = RGBRANDBG;
  bRgbRandWing = RGBRANDWING;

  // "Morph Colors" Effect Default values
  bFgChecked = DEF_FG_CHECKED;
  bBgChecked = DEF_BG_CHECKED;

  // These are RGB Only (Stored as negative numbers)!
  FgFromColor = 0; // black
  FgToColor = -0xffffff; // white
  BgFromColor = -0xffffff; // white
  BgToColor = 0; // black

  ClientWidth = CLIENTWIDTHINIT;
  ClientHeight = CLIENTHEIGHTINIT;

  OpenD.Empty();
  SaveD.Empty();
  OpenF.Empty();
  SaveF.Empty();

  NSChar = NSCHAR;

  // Reg values for DlgWebExport.cpp
  WebPageLeftMargin = WEBEXPORT_LEFT_MARGIN;
  WebPageTopMargin = WEBEXPORT_TOP_MARGIN;
  WebPageBgColor = WEBEXPORT_BG_COLOR;
  WebPageLineHeight = WEBEXPORT_LINE_HEIGHT;
  WebPageWhiteSpaceStyle = WEBEXPORT_WHITESPACE_STYLE;
  WebPageBgImageStyle = WEBEXPORT_BG_IMAGE_STYLE;
  WebPageBgImageUrl = WEBEXPORT_BGIMAGE_URL;
  WebPage_bNonBreakingSpaces = WEBEXPORT_B_NON_BREAKING_SPACES;
  WebPage_bBgImage = WEBEXPORT_B_BGIMAGE;
  WebPage_bBgFixed = WEBEXPORT_B_BGFIXED;
  WebPage_bAuthor = WEBEXPORT_B_AUTHOR;
  WebPage_bTitle = WEBEXPORT_B_TITLE;
  WebPage_bHome = WEBEXPORT_B_HOME;

  // MUST load the INI file's info before the registry info!
  if (Iftf && !Iftf->Strings[INFO_WEB_SITE].IsEmpty())
    WebPageHomeButtonUrl = Iftf->Strings[INFO_WEB_SITE];
  else
    WebPageHomeButtonUrl = DEF_WEBSITE;

  WebPageTitle = WEBEXPORT_TITLE;
  WebPageAuthor = WEBEXPORT_AUTHOR;

  bNewInstallation = false;

  utils->SetDefaultFont(tae);

  try
  {
    bool bNewInstall = false;

    if (RestoreFactory)
    {
      // Blender colors may change, so go to Process page
      PageControl1->ActivePage = TabControl;

      MyRegistry->DeleteKey(RegKey);

      // Add defaults to registry
      SaveBlendPresets();
      utils->ShowMessageU(String(DS[92]));

      // Reload everything (and do any required setup)
      ReadFromRegistry();
      LoadBlendPresetsFromRegistry();
    }
    // NOTE: If the installer creates this as an empty key,
    // set "Always Create" FALSE or OpenKey below will succeed incorrectly!
    else if (MyRegistry->OpenKey(RegKey, false))
    {
      // If old version registered, delete it
      if (MyRegistry->ValueExists("Revision"))
      {
        if (MyRegistry->ReadString("Revision") != REVISION)
        {
#if (FORCEREGISTRY)
          MyRegistry->DeleteKey(RegKey);

          // Add defaults to registry
          SaveBlendPresets();

          utils->ShowMessageU(String(DS[93]));
#endif
          // Save old settings?
          if (utils->ShowMessageU(Handle, DS[94],
                        MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1) == IDYES)
          {
            // Force new revision and play buffer size
            MyRegistry->WriteString("Revision", REVISION);
            MyRegistry->WriteInteger("PlayBufSize", GMaxPlayXmitBytes);
          }
          else
          {
            MyRegistry->DeleteKey(RegKey);

            // Add defaults to registry
            SaveBlendPresets();
          }
        }

        MyRegistry->CloseKey();

        // Reload everything (and do any required setup)
        ReadFromRegistry();
        LoadBlendPresetsFromRegistry();
      }
      else
        bNewInstall = true;
    }
    else
      bNewInstall = true;

    if (bNewInstall)
    {
      // Add defaults to registry
      SaveBlendPresets();
      bNewInstallation = true;

#if VALIDATE_ON
      DisplaySecurityCode(); // Compute .EXE file sum
#endif

      // Reload everything (and do any required setup)
      ReadFromRegistry();
      LoadBlendPresetsFromRegistry();

      // Start timer for demo
      Timer1->Interval = 700;
      Timer1->Enabled = true;
    }
  }
  catch(ERegistryException &E)
  {
    utils->ShowMessageU(String(DS[95]) + "GetRegInfo()");
    delete MyRegistry;
    return;
  }

  delete MyRegistry;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SaveBlendPresets(void)
{
  // SaveToRegistry will save the memory preset into the default
  // registry key... but we need to store more than one preset...
  // so we need to load and save all presets except first-one (the default)
  // but then fill the internal memory with the default
  SaveToRegistry();

  // See if user had been editing colors...
  SaveWorkingBlendPreset();

  LoadBlendMemoryFromDefaultPreset(BlendPreset2Init, fG_BlendColors);
  SaveBlendPresetToRegistry(fG_BlendColors, 2);

  LoadBlendMemoryFromDefaultPreset(BlendPreset3Init, fG_BlendColors);
  fG_BlendColors[7].enabled = false;
  fG_BlendColors[8].enabled = false;
  fG_BlendColors[9].enabled = false;
  SaveBlendPresetToRegistry(fG_BlendColors, 3);

  fG_BlendPreset = FGBLENDPRESET;
  LoadBlendMemoryFromDefaultPreset(BlendPreset0Init, fG_BlendColors);
  SaveBlendPresetToRegistry(fG_BlendColors, fG_BlendPreset);
  bG_BlendPreset = BGBLENDPRESET;
  LoadBlendMemoryFromDefaultPreset(BlendPreset1Init, bG_BlendColors);
  SaveBlendPresetToRegistry(bG_BlendColors, bG_BlendPreset);
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::ReadFromRegistry(void)
// Always add new keys at end within TRY because they will throw an exception and we want old keys to all load
// first! (not really needed anymore since we added wrapper methods to catch exceptions and insert defaults)
{
  const String RegKey = REGISTRY_KEY;

  TRegistry* pReg = new TRegistry();
  pReg->RootKey = HKEY_CURRENT_USER;

  // Temp strings
  String sName, sRead, sTemp;
  WideString wRead, wTemp;
  bool bErr; // ref read-error flag

  try
  {
    pReg->OpenKey(RegKey, false);

    Wingdings->OnClick = NULL;
    Wingdings->Checked = RegReadBool(pReg, "Wingdings", ALLOW_WINGDINGS);
    Wingdings->OnClick = WingdingsClick;

    Borders->OnClick = NULL;
    Borders->Checked = RegReadBool(pReg, "Borders", ALLOW_BORDERS);
    Borders->OnClick = BordersClick;

    bTopEnabled = RegReadBool(pReg, "TopEnabled", TOPBOTTOM_ENABLED);
    bSideEnabled = RegReadBool(pReg, "SideEnabled", SIDES_ENABLED);

    TotalWings = RegReadInt(pReg, "TotalWings", DEF_TOTAL_WINGS);

    LeftWings->Clear();
    RightWings->Clear();

    for (int ii = 0; ii < TotalWings; ii++)
    {
      sName = "LeftWing" + String(ii);

      wRead = RegReadStringW(pReg, sName.c_str(), sName.c_str(), &bErr);

      if (bErr || wRead.Length() < 2) // invalid?
        continue;

      sTemp = String(wRead.SubString(1,1)); // read WingActive flag 0/1

      bool bWingActive = (sTemp == '0') ? false : true;

      wTemp = wRead.SubString(2, wRead.Length()-1);

      // WingActive is now saved in the Tag property of the LeftWings
      // TStringsW object!
      if (wTemp.Length() >= 1)
        LeftWings->Add(wTemp, (void*)bWingActive);
    }

    // Possibly adjust down...
    TotalWings = LeftWings->Count;

    sTemp = "RightWing";

    // Right-wings... only load up to LeftWings->Count!
    for (int ii = 0; ii < TotalWings; ii++)
    {
      sName = sTemp + String(ii);

      wRead = RegReadStringW(pReg, sName.c_str(), sName.c_str(), &bErr);

      if (bErr || wRead.Length() < 2) // invalid?
        RightWings->Add(""); // empty... but need to match LeftWings count!
      else
      {
        wTemp = wRead.SubString(2, wRead.Length()-1);

        if (wTemp.Length() >= 1)
          RightWings->Add(wTemp); // no Tag...
      }
    }

    TopBorder = RegReadStringW(pReg, "TopBorder", TB);

    LeftSideBorder = RegReadStringW(pReg, "LeftSideBorder", LB);
    if (LeftSideBorder.IsEmpty()) LeftSideBorder = "*";

    RightSideBorder = RegReadStringW(pReg, "RightSideBorder", RB);
    if (RightSideBorder.IsEmpty()) RightSideBorder = "*";

    // Kick off the initial word-wrap
    //WordWrap->Checked = pReg->ReadBool("WordWrap");
    //SetWordWrap(WordWrap->Checked);
    SetWordWrap(WORD_WRAP);

    PackText->Checked = RegReadBool(pReg, "PackText", PACK_TEXT);
    LeftJustify->Checked = RegReadBool(pReg, "LeftJustify", LEFT_JUSTIFY);
    AutoWidth->Checked = RegReadBool(pReg, "AutoWidth", AUTO_WIDTH);
    AutoHeight->Checked = RegReadBool(pReg, "AutoHeight", AUTO_SPACING);
    PlayOnCR->Checked = RegReadBool(pReg, "PlayOnCR", PLAY_ON_CR);
    PlayOnCX->Checked = RegReadBool(pReg, "PlayOnCX", PLAY_ON_CX);
    SwiftMixSync->Checked = RegReadBool(pReg, "SwiftMixSync", SWIFTMIX_SYNC);
    PackLines->Checked = RegReadBool(pReg, "PackLines", PACK_LINES);
    ExtendPalette->Checked = RegReadBool(pReg, "ExtendPalette", EXTEND_PALETTE); // added 2/9/2015
    AutoDetectEmotes->Checked = RegReadBool(pReg, "AutoEmotes", BYPASS_SMILEYS);
    Margins->Checked = RegReadBool(pReg, "Margins", MARGINS_ENABLED);
    TabsToSpaces->Checked = RegReadBool(pReg, "TabsToSpacesChecked", DEF_TABS_TO_SPACES_CHECKED);

    bFgChecked = RegReadBool(pReg, "FgChecked", DEF_FG_CHECKED);
    bBgChecked = RegReadBool(pReg, "BgChecked", DEF_BG_CHECKED);

    bUseDLL = (RegReadBool(pReg, "UseDLL", USE_DLL) ? true : false);
    bUseFile = (RegReadBool(pReg, "UseFile", USE_FILE) ? true : false);
    bSendUtf8 = (RegReadBool(pReg, "SendUtf8", SEND_UTF8) ? true : false);
    bRgbRandFG = (RegReadBool(pReg, "RGBrandomFG", RGBRANDFG) ? true : false);
    bRgbRandBG = (RegReadBool(pReg, "RGBrandomBG", RGBRANDBG) ? true : false);
    bRgbRandWing = (RegReadBool(pReg, "RGBrandomWing", RGBRANDWING) ? true : false);
    bPadSpaces = (RegReadBool(pReg, "NoSpaces", NO_SPACES) ? true : false);
    bStripFont = (RegReadBool(pReg, "StripFont", STRIP_PLAYBACK_FONT) ? true : false);

    regTabs = RegReadInt(pReg, "Tabs", TABSINIT);
    regLmargin = RegReadInt(pReg, "Lmargin", LMARGINIT);
    regRmargin = RegReadInt(pReg, "Rmargin", RMARGINIT);
    regTmargin = RegReadInt(pReg, "Tmargin", TMARGINIT);
    regBmargin = RegReadInt(pReg, "Bmargin", BMARGINIT);
    regIndent = RegReadInt(pReg, "Indent", INDENTINIT);
    regSpacing = RegReadInt(pReg, "LineSpacing", SPACINGINIT);
    regWidth = LineWidth = RegReadInt(pReg, "LineWidth", WIDTHINIT);
    regHeight = LineHeight = RegReadInt(pReg, "LineHeight", HEIGHTINIT);

    regWysiwygPrintScaleX = RegReadInt(pReg, "PrintScaleX", DEF_SCALE_X);
    regWysiwygPrintScaleY = RegReadInt(pReg, "PrintScaleY", DEF_SCALE_Y);

    regTabMode = RegReadInt(pReg, "TabMode", DEF_TAB_MODE);

    if (regTabMode == FLATTEN_TABS_SINGLE_SPACE)
    {
      TabsStripRB->Checked = true;
      TabsFlattenRB->Checked = false;
    }
    else
    {
      TabsStripRB->Checked = false;
      TabsFlattenRB->Checked = true;
    }


    FgToColor = RegReadInt(pReg, "FgToColor", -0xffffff); // RGB white
    BgToColor = RegReadInt(pReg, "BgToColor", 0); // RGB black

    Background = RegReadInt(pReg, "Background", BACKGROUNDINIT);
    Foreground = RegReadInt(pReg, "Foreground", FOREGROUNDINIT);

    WingColor = RegReadInt(pReg, "WingColor", WINGCOLORINIT);
    BorderBackground = RegReadInt(pReg, "BorderBackground", BORDERBACKGROUNDINIT);
    BlendBackground = RegReadInt(pReg, "BlendBackground", BLENDBACKGROUNDINIT);

    taeWindowColor = (TColor)RegReadInt(pReg, "TaeWindowColor", WINDOWCOLORINIT);
    tae->Color = taeWindowColor;

    PlayTime = RegReadInt(pReg, "PlayTime", PLAYTIME);
    BurstInterval = RegReadInt(pReg, "BurstInterval", BURSTINTERVAL);

    // Get index of current room (GRoomIndex is the property!)
    roomIndex = RegReadInt(pReg, "PlayChanIndex", 0);
    if (roomIndex >= MAXROOMHISTORY || roomIndex < 0)
      roomIndex = 0;

    sTemp = "PlayChans";

    // Add rooms to string-list (remove empties)
    for (int ii = 0; ii < MAXROOMHISTORY; ii++)
    {
      sName = sTemp + String(ii);

      sRead = RegReadString(pReg, sName.c_str(), "", &bErr);

      if (bErr) // not there?
        continue;

      if (sRead.IsEmpty() || utils->IsAllSpaces(sRead)) // invalid?
      {
        pReg->DeleteValue(sName); // Delete invalid slots...
        continue;
      }

      GRooms->Add(sRead);
    }

    if (GRooms->Count == 0)
    {
      GRooms->Add(STATUSCHAN); // add status room
      roomIndex = 0;
    }
    else if (roomIndex >= GRooms->Count)
      roomIndex = GRooms->Count-1;

    try
    {
      PlayChan = GRooms->Strings[roomIndex];

      if (PlayChan.IsEmpty())
        PlayChan = STATUSCHAN;
    }
    catch(...)
    {
      PlayChan = STATUSCHAN;
    }

    GDictionary = RegReadStringW(pReg, "Dict", DEF_DICTIONARY);

    sTemp = utils->WideToUtf8(utils->GetTempFileW(U16(FN[6])));
    Default_Playfile = RegReadStringW(pReg, "Default_Playfile", sTemp.c_str());

    // Get the client index from registry
    Cli = RegReadInt(pReg, "Client", DEF_CLIENT);

    // Limit-check
    if (Cli < 0 || Cli > MAXCLIENTS-1)
      Cli = 0;

    // User font characteristics
    cCharset = (TFontCharset)RegReadInt(pReg, "CCharset", USER_DEF_CHARSET);
//    cColor = (TColor)RegReadInt(pReg, "CColor", USER_DEF_COLOR);
    cPitch = (TFontPitch)RegReadInt(pReg, "CPitch", USER_DEF_PITCH);
    cStyle = IntToStyle(RegReadInt(pReg, "CStyle", 0));
    cSize = RegReadInt(pReg, "CSize", USER_DEF_SIZE);

    if (cSize < 0 || cSize > 99)
      cSize = USER_DEF_SIZE;

    this->cFont = RegReadStringW(pReg, "CFont", USER_DEF_FONT);

    int ii;
    wTemp = utils->LowerCaseW(cFont);

    for (ii = 0; ii < FONTITEMS; ii++)
      if (utils->LowerCaseW(U16(FONTS[ii])) == wTemp)
        break;

    // Can't find the font?
    if (ii == FONTITEMS)
      this->cFont = U16(USER_DEF_FONT);

    cType = utils->GetLocalFontIndex(cFont);

    utils->SetRichEditFont(tae);

    GMaxPlayXmitBytes = RegReadInt(pReg, "PlayBufSize", DEF_PLAYBUFSIZE);
    GLineBurstSize = RegReadInt(pReg, "LineBurstSize", LINEBURSTSIZE);

    // Blender
    BlendScopeRadioButtons->ItemIndex = RegReadInt(pReg, "BlendScope", BLENDSCOPE);
    BlendDirectionRadioButtons->ItemIndex = RegReadInt(pReg, "BlendDirection", BLENDDIRECTION);
    fG_BlendPreset = RegReadInt(pReg, "fG_BlendPreset", FGBLENDPRESET);
    bG_BlendPreset = RegReadInt(pReg, "bG_BlendPreset", BGBLENDPRESET);
    RGBThreshold = RegReadInt(pReg, "RGBThreshold", RGBTHRESHOLD);

    if (palette != NULL)
      pReg->ReadBinaryData("ColorPalette", palette, MAXDEFAULTCOLORS);

    // New strategy, allow to fail, but only will fail for
    // newly added reg-vars - so put them at the END of read!
    ClientWidth = RegReadInt(pReg, "ClientWidth", CLIENTWIDTHINIT);
    ClientHeight = RegReadInt(pReg, "ClientHeight", CLIENTHEIGHTINIT);

    OpenD = RegReadStringW(pReg, "OpenD", "");
    OpenF = RegReadStringW(pReg, "OpenF", "");
    bFileOpened = false;
    SaveD = RegReadStringW(pReg, "SaveD", "");
    SaveF = RegReadStringW(pReg, "SaveF", "");
    bFileSaved = false;

    NSChar = (char)RegReadInt(pReg, "NSChar", NSCHAR);

    WebPageLeftMargin = RegReadInt(pReg, "WebPageLeftMargin", WEBEXPORT_LEFT_MARGIN);
    WebPageTopMargin = RegReadInt(pReg, "WebPageTopMargin", WEBEXPORT_TOP_MARGIN);
    WebPageLineHeight = RegReadInt(pReg, "WebPageLineHeight", WEBEXPORT_LINE_HEIGHT);
    WebPageBgColor = RegReadInt(pReg, "WebPageBgColor", WEBEXPORT_BG_COLOR);
    WebPageWhiteSpaceStyle = RegReadInt(pReg, "WebPageWhiteSpaceStyle", WEBEXPORT_WHITESPACE_STYLE);
    WebPageBgImageStyle = RegReadInt(pReg, "WebPageBgImageStyle", WEBEXPORT_BG_IMAGE_STYLE);
    WebPageBgImageUrl = RegReadString(pReg, "WebPageBgImageUrl", WEBEXPORT_BGIMAGE_URL);
    WebPageHomeButtonUrl = RegReadString(pReg, "WebPageHomeButtonUrl", DEF_WEBSITE);
    WebPageTitle = RegReadString(pReg, "WebPageTitle", WEBEXPORT_TITLE);
    WebPageAuthor = RegReadString(pReg, "WebPageAuthor", WEBEXPORT_AUTHOR);
    WebPage_bNonBreakingSpaces = RegReadBool(pReg, "WebPage_bNonBreakingSpaces", WEBEXPORT_B_NON_BREAKING_SPACES);
    WebPage_bBgImage = RegReadBool(pReg, "WebPage_bBgImage", WEBEXPORT_B_BGIMAGE);
    WebPage_bBgFixed = RegReadBool(pReg, "WebPage_bBgFixed", WEBEXPORT_B_BGFIXED);
    WebPage_bAuthor = RegReadBool(pReg, "WebPage_bAuthor", WEBEXPORT_B_AUTHOR);
    WebPage_bTitle = RegReadBool(pReg, "WebPage_bTitle", WEBEXPORT_B_TITLE);
    WebPage_bHome = RegReadBool(pReg, "WebPage_bHome", WEBEXPORT_B_HOME);

    pReg->CloseKey();
  }
  catch(ERegistryException &E)
  {
    // Trick with this is that we always add new registry keys at END
    // above so that the old keys are all loaded before exception is thrown!
    // We would rather not just erase all settings here!
    pReg->CloseKey();
    return false;
  }

  delete pReg;
  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::RegReadBool(TRegistry* pReg, char* pVal, bool Def)
{
  try { return pReg->ReadBool(pVal); }
  catch(...) { return Def; }
}
//---------------------------------------------------------------------------
int __fastcall TDTSColor::RegReadInt(TRegistry* pReg, char* pVal, int Def)
{
  try { return pReg->ReadInteger(pVal); }
  catch(...) { return Def; }
}
//---------------------------------------------------------------------------
// Pass in a UTF-8 string (char*)
WideString __fastcall TDTSColor::RegReadStringW(TRegistry* pReg, char* pVal, char* pDef, bool* bErr)
// Returns pDef's string as UTF-16 and sets bool ref bErr if the entry does not exist...
{
  WideString wOut;

  try
  {
    if (!pReg->ValueExists(pVal))
    {
      wOut = utils->Utf8ToWide(String(pDef));

      if (bErr != NULL)
        *bErr = true;
    }
    else
    {
      wOut = utils->Utf8ToWide(String(pReg->ReadString(pVal)));

      if (wOut.IsEmpty())
        wOut = utils->Utf8ToWide(String(pDef));

      if (bErr != NULL)
        *bErr = false;
    }
  }
  catch(...)
  {
    wOut = utils->Utf8ToWide(String(pDef));

    if (bErr != NULL)
      *bErr = true;
  }

  return wOut;
}
//---------------------------------------------------------------------------
// Pass in a UTF-8 or ANSI string (char*)
String __fastcall TDTSColor::RegReadString(TRegistry* pReg, char* pVal, char* pDef, bool* bErr)
// Returns pDef's string as UTF-8 or ANSI and sets bool ref bErr if the entry does not exist...
{
  String sOut;

  try
  {
    if (!pReg->ValueExists(pVal))
    {
      if (bErr != NULL)
        *bErr = true;
    }
    else
    {
      sOut = String(pReg->ReadString(pVal));

      if (bErr != NULL)
        *bErr = false;
    }
  }
  catch(...)
  {
    sOut = String(pDef);

    if (bErr != NULL)
      *bErr = true;
  }

  return sOut;
}
//---------------------------------------------------------------------------
int __fastcall TDTSColor::LoadBlendPresetsFromRegistry(void)
{
  try
  {
    // Load the current color-blender preset
    if (!LoadBlendPresetFromRegistry(fG_BlendColors, fG_BlendPreset))
    {
      // Preset is empty or corrupt
      // Save default BlendColors to the preset
      fG_BlendPreset = FGBLENDPRESET;
      SaveBlendPresetToRegistry(fG_BlendColors, fG_BlendPreset); // (Updates preset-index also)
    }
    // Load the current color-blender preset
    if (!LoadBlendPresetFromRegistry(bG_BlendColors, bG_BlendPreset))
    {
      // Preset is empty or corrupt
      // Save default BlendColors to the preset
      bG_BlendPreset = BGBLENDPRESET;
      SaveBlendPresetToRegistry(bG_BlendColors, bG_BlendPreset); // (Updates preset-index also)
    }

    return true;
  }
  catch(ERegistryException &E) { return false; }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::LoadBlendPresetFromRegistry(BlendColor* BC, int Idx)
{
  if (!BC) return(false);

  bool RetVal = false;

  try
  {
    const String RegKey = REGISTRY_KEY;

    TRegistry* MyRegistry = new TRegistry();
    MyRegistry->RootKey = HKEY_CURRENT_USER;

    MyRegistry->OpenKey(RegKey, false);

    // Limit-check
    if (Idx < 0 || Idx >= MAXBLENDPRESETS) Idx = 0;

    // Form a name for the current preset
    String Name = "BP" + String(Idx);

    // Read BlendColor Structures into memory (from the current preset)
    if (BC != NULL) // memory was created?
    {
        try { if (MyRegistry->ReadBinaryData(Name, BC, MAXBLENDCOLORS* sizeof(BlendColor))) RetVal = true; }
        catch(...) {}
    }

    MyRegistry->CloseKey();

    delete MyRegistry;
    return(RetVal);
  }
  catch(...) { return(RetVal); }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::SaveBlendPresetToRegistry(BlendColor* BC, int Idx)
{
  try
  {
    const String RegKey = REGISTRY_KEY;

    TRegistry* MyRegistry = new TRegistry();
    MyRegistry->RootKey = HKEY_CURRENT_USER;

    MyRegistry->OpenKey(RegKey, false);

    // Limit-check
    if (Idx < 0 || Idx >= MAXBLENDPRESETS)
      Idx = 0;

    // Form a name for the current preset
    String Name = "BP" + String(Idx);

    // Save BlendColor Structures into memory (from the current preset)
    if (BC != NULL) // memory was created?
      MyRegistry->WriteBinaryData(Name, BC, MAXBLENDCOLORS* sizeof(BlendColor));

    MyRegistry->CloseKey();

    delete MyRegistry;
    return(true);
  }
  catch(...)
  {
    utils->ShowMessageU(String(DS[95]) + "SaveBlendPresetToRegistry()");
    return(false);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SaveToRegistry(void)
{
    const String RegKey = REGISTRY_KEY;

    TRegistry *MyRegistry = new TRegistry();
    MyRegistry->RootKey = HKEY_CURRENT_USER;

    // Temp strings
    String sName, sTemp;
    WideString wString, wTemp;
    int iTemp;

    try
    {
      if(MyRegistry->OpenKey(RegKey, true))
      {
        MyRegistry->WriteString("Revision", REVISION);

        MyRegistry->WriteBool("Borders", Borders->Checked);
        MyRegistry->WriteBool("Wingdings", Wingdings->Checked);
        MyRegistry->WriteBool("WordWrap", WordWrap->Checked);
        MyRegistry->WriteBool("PackText", PackText->Checked);
        MyRegistry->WriteBool("LeftJustify", LeftJustify->Checked);
        MyRegistry->WriteBool("AutoWidth", AutoWidth->Checked);
        MyRegistry->WriteBool("AutoHeight", AutoHeight->Checked);
        MyRegistry->WriteBool("PlayOnCR", PlayOnCR->Checked);
        MyRegistry->WriteBool("PlayOnCX", PlayOnCX->Checked);
        MyRegistry->WriteBool("SwiftMixSync", SwiftMixSync->Checked);
        MyRegistry->WriteBool("PackLines", PackLines->Checked);
        MyRegistry->WriteBool("ExtendPalette", ExtendPalette->Checked);
        MyRegistry->WriteBool("AutoEmotes", AutoDetectEmotes->Checked);
        MyRegistry->WriteBool("Margins", Margins->Checked);
        MyRegistry->WriteBool("TabsToSpacesChecked", TabsToSpaces->Checked);

        MyRegistry->WriteBool("FgChecked", bFgChecked);
        MyRegistry->WriteBool("BgChecked", bBgChecked);

        MyRegistry->WriteBool("TopEnabled", bTopEnabled);
        MyRegistry->WriteBool("SideEnabled", bSideEnabled);
        MyRegistry->WriteBool("UseDLL", bUseDLL);
        MyRegistry->WriteBool("UseFile", bUseFile);
        MyRegistry->WriteBool("SendUtf8", bSendUtf8);
        MyRegistry->WriteBool("RGBrandomFG", bRgbRandFG);
        MyRegistry->WriteBool("RGBrandomBG", bRgbRandBG);
        MyRegistry->WriteBool("RGBrandomWing", bRgbRandWing);
        MyRegistry->WriteBool("NoSpaces", bPadSpaces);
        MyRegistry->WriteBool("StripFont", bStripFont);

        // Left/Right should have the same count... Left is the master...
        int leftCount = LeftWings->Count;
        int rightCount = RightWings->Count;

        MyRegistry->WriteInteger("TotalWings", leftCount);

        // Save the wings
        for (int ii = 0; ii < leftCount; ii++)
        {
          sName = "LeftWing" + String(ii);
          sTemp = (LeftWings->GetTag(ii) == 0) ? "0" : "1";
          wTemp = WideString(sTemp) + LeftWings->GetString(ii);
          RegWriteStringW(MyRegistry, sName, wTemp);

          // limit just in-case it's gotten off...
          if (ii < rightCount)
          {
            sName = "RightWing" + String(ii);
            wTemp = "0" + RightWings->GetString(ii);
            RegWriteStringW(MyRegistry, sName, wTemp);
          }
        }

        RegWriteStringW(MyRegistry, "TopBorder", TopBorder);
        RegWriteStringW(MyRegistry, "LeftSideBorder", LeftSideBorder);
        RegWriteStringW(MyRegistry, "RightSideBorder", RightSideBorder);

        MyRegistry->WriteInteger("Tabs", regTabs);
        MyRegistry->WriteInteger("Lmargin", regLmargin);
        MyRegistry->WriteInteger("Rmargin", regRmargin);
        MyRegistry->WriteInteger("Tmargin", regTmargin);
        MyRegistry->WriteInteger("Bmargin", regBmargin);
        MyRegistry->WriteInteger("Indent", regIndent);
        MyRegistry->WriteInteger("LineSpacing", regSpacing);
        MyRegistry->WriteInteger("LineWidth", regWidth);
        MyRegistry->WriteInteger("LineHeight", regHeight);

        MyRegistry->WriteInteger("PrintScaleX", regWysiwygPrintScaleX);
        MyRegistry->WriteInteger("PrintScaleY", regWysiwygPrintScaleY);

        MyRegistry->WriteInteger("TabMode", regTabMode);

        MyRegistry->WriteInteger("FgToColor", FgToColor);
        MyRegistry->WriteInteger("BgToColor", BgToColor);

        MyRegistry->WriteInteger("Background", Background);
        MyRegistry->WriteInteger("Foreground", Foreground);

        MyRegistry->WriteInteger("WingColor", WingColor);
        MyRegistry->WriteInteger("TaeWindowColor", taeWindowColor);
        MyRegistry->WriteInteger("BorderBackground", BorderBackground);
        MyRegistry->WriteInteger("BlendBackground", BlendBackground);

        MyRegistry->WriteInteger("PlayTime", PlayTime);
        MyRegistry->WriteInteger("BurstInterval", BurstInterval);

        // Delete old unused PlayChan key if it exists...
        //MyRegistry->WriteString("PlayChan", PlayChan);
        if (MyRegistry->ValueExists("PlayChan"))
          MyRegistry->DeleteValue("PlayChan");

        // Write room-history, erasing empty slots
        sTemp = "PlayChans";
        iTemp = GRooms->Count;

        for (int ii = 0; ii < MAXROOMHISTORY; ii++)
        {
          sName = sTemp + String(ii);

          if (ii < iTemp)
          {
            String sRoom = GRooms->Strings[ii];

            MyRegistry->WriteString(sName, sRoom);
          }
          else if (MyRegistry->ValueExists(sName))
            MyRegistry->DeleteValue(sName);
        }

        MyRegistry->WriteInteger("PlayChanIndex", roomIndex);

        RegWriteStringW(MyRegistry, "Default_Playfile", Default_Playfile);
        RegWriteStringW(MyRegistry, "Dict", GDictionary);

        MyRegistry->WriteInteger("Client", Cli);

        // User font characteristics
        MyRegistry->WriteInteger("CCharset", cCharset);
//        MyRegistry->WriteInteger("CColor", cColor);

        RegWriteStringW(MyRegistry, "CFont", cFont);

        MyRegistry->WriteInteger("CPitch", cPitch);
        MyRegistry->WriteInteger("CSize", cSize);
        MyRegistry->WriteInteger("CStyle", StyleToInt(cStyle));

        MyRegistry->WriteInteger("PlayBufSize", GMaxPlayXmitBytes);
        MyRegistry->WriteInteger("LineBurstSize", GLineBurstSize);

        // Blender
        MyRegistry->WriteInteger("BlendScope", BlendScopeRadioButtons->ItemIndex);
        MyRegistry->WriteInteger("BlendDirection", BlendDirectionRadioButtons->ItemIndex);

        if (fG_BlendPreset < 0 || fG_BlendPreset >= MAXBLENDPRESETS)
          fG_BlendPreset = FGBLENDPRESET;
        MyRegistry->WriteInteger("fG_BlendPreset", fG_BlendPreset);

        if (bG_BlendPreset < 0 || bG_BlendPreset >= MAXBLENDPRESETS)
          bG_BlendPreset = BGBLENDPRESET;
        MyRegistry->WriteInteger("bG_BlendPreset", bG_BlendPreset);

        MyRegistry->WriteInteger("RGBThreshold", RGBThreshold);

        // Write just the first 16 colors (only ones user can modify)
        if (palette != NULL)
          MyRegistry->WriteBinaryData("ColorPalette", palette, MAXDEFAULTCOLORS);

        MyRegistry->WriteInteger("ClientWidth", Width);
        MyRegistry->WriteInteger("ClientHeight", Height);

        RegWriteStringW(MyRegistry, "OpenD", OpenD);
        RegWriteStringW(MyRegistry, "SaveD", SaveD);
        RegWriteStringW(MyRegistry, "OpenF", OpenF);
        RegWriteStringW(MyRegistry, "SaveF", SaveF);

        MyRegistry->WriteInteger("NSChar", (int)NSChar);

        MyRegistry->WriteInteger("WebPageLineHeight", WebPageLineHeight);
        MyRegistry->WriteInteger("WebPageBgColor", WebPageBgColor);
        MyRegistry->WriteInteger("WebPageLeftMargin", WebPageLeftMargin);
        MyRegistry->WriteInteger("WebPageTopMargin", WebPageTopMargin);
        MyRegistry->WriteInteger("WebPageWhiteSpaceStyle", WebPageWhiteSpaceStyle);
        MyRegistry->WriteInteger("WebPageBgImageStyle", WebPageBgImageStyle);
        MyRegistry->WriteString("WebPageBgImageUrl", WebPageBgImageUrl);
        MyRegistry->WriteString("WebPageHomeButtonUrl", WebPageHomeButtonUrl);
        MyRegistry->WriteString("WebPageTitle", WebPageTitle);
        MyRegistry->WriteString("WebPageAuthor", WebPageAuthor);
        MyRegistry->WriteBool("WebPage_bNonBreakingSpaces", WebPage_bNonBreakingSpaces);
        MyRegistry->WriteBool("WebPage_bBgImage", WebPage_bBgImage);
        MyRegistry->WriteBool("WebPage_bBgFixed", WebPage_bBgFixed);
        MyRegistry->WriteBool("WebPage_bAuthor", WebPage_bAuthor);
        MyRegistry->WriteBool("WebPage_bTitle", WebPage_bTitle);
        MyRegistry->WriteBool("WebPage_bHome", WebPage_bHome);

        MyRegistry->CloseKey();
      }
      else
        utils->ShowMessageU(DS[96]);
    }
    catch(ERegistryException &E)
    {
      utils->ShowMessageU(DS[95] + String("SaveToRegistry()"));
      MyRegistry->CloseKey();
    }

    delete MyRegistry;
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::RegWriteStringW(TRegistry* pReg, String sVal, WideString sIn)
{
  try
  {
    pReg->WriteString(sVal.c_str(), utils->WideToUtf8(sIn).c_str());
    return true;
  }
  catch(...)
  {
    return false;
  }
}
//----------------------------------------------------------------------------
bool __fastcall TDTSColor::AddRoom(String S)
// Adds S to the global room-list if it's not there already
// and sets the global room-index to the new room. The rooms-list
// is UTF-8!
{
  try
  {
    // If the chat-client sent us a channel name or nickname to reply
    // to - set PlayChan to that!
    if (!S.IsEmpty())
    {
      PlayChan = S;

      if (GRooms != NULL)
      {
        int idx = GRooms->IndexOf(PlayChan);

        if (idx >= 0)
          GRoomIndex = idx;
        else
        {
          if (GRooms->Count >= MAXROOMHISTORY)
            GRooms->Delete(1); // Delete oldest (not status)

          GRooms->Add(PlayChan);
          GRoomIndex = GRooms->Count-1;
        }
      }
    }
    return true;
  }
  catch(...){return false;}
}
//---------------------------------------------------------------------------
// End Registry Functions
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Security-Code Functions
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::CompareSecurityCode(void)
{
#if VALIDATE_ON
  try
  {
    const String RegKey = REGISTRY_KEY;
    DWORD dwOriginalSecurity;

    TRegistry* MyRegistry = new TRegistry();
    MyRegistry->RootKey = HKEY_LOCAL_MACHINE;

    if(!MyRegistry->OpenKeyReadOnly(RegKey))
    {
      delete MyRegistry;
      return(false);
    }

    try
    {
      dwOriginalSecurity = MyRegistry->ReadInteger("SecurityCode");
    }
    catch(...)
    {
      utils->ShowMessageU(String(DS[102]));
      MyRegistry->CloseKey();
      delete MyRegistry;
      return(false);
    }

    MyRegistry->CloseKey();
    delete MyRegistry;

//    utils->ShowMessageU(IntToHex((int)dwOriginalSecurity, sizeof(int)*2));

    DWORD PresentSecurity = ComputeSecurityCode();

    if (dwOriginalSecurity != PresentSecurity)
    {
      utils->ShowMessageU(String(DS[101]));
      return(false);
    }

    return(true);
  }
  catch(...)
  {
    utils->ShowMessageU(String(DS[102]));
    return(false);
  }
#else
  return(false);
#endif
}
//---------------------------------------------------------------------------
#if VALIDATE_ON
String __fastcall TDTSColor::DisplaySecurityCode(void)
{
  DWORD dwSecurityCode = ComputeSecurityCode();

  if (dwSecurityCode == 0)
  {
    utils->ShowMessageU(String(DS[103]));
    return("");
  }

  String Temp = IntToHex((int)dwSecurityCode, sizeof(int) * 2);
  utils->ShowMessageU(String(DS[104]) + Temp + "\n" + String(DS[105]));

  return String(Temp);
}
#endif
//---------------------------------------------------------------------------
DWORD __fastcall TDTSColor::ComputeSecurityCode(void)
{
#if VALIDATE_ON
    DWORD dwSecurityCode = 0;

    try
    {
        WideString wExe = utils->GetExeNameW();

        // Open our own .EXE in binary/read-only mode
        FILE *f = _wfopen(wExe.c_bstr(), L"rb");

        if (f == NULL)
        {
          utils->ShowMessageW(U16(DS[106]) + wExe);
          return(0);
        }

        int iFileLength = (int)utils->filesize(f);

        if (iFileLength <= 0)
        {
          utils->ShowMessageW(U16(DS[106]) + wExe);
          fclose(f);
          return 0;
        }

        char* buffer = new char[iFileLength + 1];
        int br = fread(buffer, 1, iFileLength, f);
        fclose(f);

        for (int ii = 0; ii < br; ii++)
          dwSecurityCode += buffer[ii];

        delete [] buffer;
    }
    catch(ERegistryException &E)
    {
      utils->ShowMessageU(String(DS[95]) + "ComputeSecurityCode()");
    }

  return dwSecurityCode;
#else
    return 0;
#endif
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::DisplaySecurityCode1Click(TObject *Sender)
{
#if VALIDATE_ON
  String Temp = DisplaySecurityCode();

  // Put it in the clipboard... useful in package-build
  if (Temp.Length())
    Clipboard()->AsText = Temp;
#endif
}
//---------------------------------------------------------------------------
// End Security-Code Functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// EFFECTS
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EditUndoItemClick(TObject *Sender)
// Undo Just loads the Original View...
{
//  if (tae->View == V_RTF)
//    LoadView(V_ORG);
//  else if (tae->Modified)
//    tae->Undo();
  if (TOCUndo != NULL)
    TOCUndo->Undo();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EffectSetFgBgClick(TObject *Sender)
{
  Application->CreateForm(__classid(TSetColorsForm), &SetColorsForm);

  if (IsCli(C_PALTALK))
    SetColorsForm->Mode = EM_FG;
  else
    SetColorsForm->Mode = EM_FGBG;

  SetColorsForm->ShowModal();

  if (SetColorsForm->ModalResult == mrCancel)
  {
    YcDestroyForm(SetColorsForm);
    SetColorsForm = NULL;
    return;
  }

  EParm1 = SetColorsForm->FgColor;
  EParm2 = SetColorsForm->BgColor;
  EParm3 = SetColorsForm->Mode;
  YcDestroyForm(SetColorsForm);
  SetColorsForm = NULL;

  if (IsRawCodeMode())
  {
    int SaveSelStart = tae->SelStart;
    WideString sColors;
    utils->WriteColors(EParm1, EParm2, sColors);
    TaeEditPaste(true, sColors);
    tae->SelStart = SaveSelStart + sColors.Length();
  }
  else
  {
    AddEffect(E_SET_COLORS);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EffectReplaceColorsClick(TObject *Sender)
{
  EffectMorphOrReplace(Sender != NULL ? true : false, false);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EffectMorphClick(TObject *Sender)
// Effects Menu Item Click
{
  EffectMorphOrReplace(Sender != NULL ? true : false, true);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EffectMorphOrReplace(bool bUseDialog, bool bMorph)
// Set bUseDialog if invoked from the Effects menu - if it's a remote
// chat-client invoking the effect, set bUseDialog false. bMorph is true for
// the Morph-Colors effect or false for the Replace-Colors effect
{
  // We push the lockcounter in DoProcess... don't want to initiate a DoProcess
  // from in here to apply a new color while processing!
  if (tae->LockCounter)
    return;

  // Get TaeEdit colors at the caret
  int fg, bg;
  if (!utils->GetTaeEditColors(fg, bg, true))
  {
    utils->ShowMessageU(DS[45]); // "no text to process"
    return;
  }

// this was a way to get color from the rich-edit itself... FYI
//    if (tae->View == V_RTF)
//    {
//      CHARFORMAT2 cf;
//      cf.cbSize = sizeof(CHARFORMAT2);
//      SendMessage(tae->Handle, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
//
//      FgFrom = -utils->TColorToRgb((TColor)cf.crTextColor);
//      FgTo = utils->YcToRgb(Abg);
//      //FgTo = FgToColor; // from registry (must be negative rgb!)
//      BgFrom = -utils->TColorToRgb((TColor)cf.crBackColor);
//      BgTo = utils->YcToRgb(Afg);
//      //BgTo = BgToColor; // from registry (must be negative rgb!)
//    }

  if (bUseDialog)
  {
    Application->CreateForm(__classid(TMorphForm), &MorphForm);

    if (bMorph)
    {
      MorphForm->Title = COLORDIALOGMSG[13]; // "Morph Colors";
      MorphForm->HideMaxColors = false;
    }
    else
    {
      MorphForm->Title = COLORDIALOGMSG[14]; // "Replace Colors";
      MorphForm->HideMaxColors = true;
    }

    // Set dialog's properties
    MorphForm->FgChecked = bFgChecked;
    MorphForm->BgChecked = bBgChecked;

    // FgToColor and BgToColor get saved in the registry and must always be
    // in negative RGB format...
    MorphForm->FgToColor = FgToColor;
    MorphForm->BgToColor = BgToColor;

    // FgFromColor and BgFromColor do NOT get saved in the registry but must always be
    // in negative RGB format...
    MorphForm->FgFromColor = utils->YcToRgb(fg);
    MorphForm->BgFromColor = utils->YcToRgb(bg);

    // If paltalk we init to 10 colors - otherwise it's -1 (infinite)
    if (IsCli(C_PALTALK))
      MorphForm->MaxColors = PaltalkMaxColors;

    MorphForm->ShowModal();

    if (MorphForm->ModalResult == mrCancel)
    {
      YcDestroyForm(MorphForm);
      MorphForm = NULL;
      return;
    }

    // Get dialog's properties to save
    FgFromColor = MorphForm->FgFromColor;
    FgToColor = MorphForm->FgToColor;
    BgFromColor = MorphForm->BgFromColor;
    BgToColor = MorphForm->BgToColor;
    bFgChecked = MorphForm->FgChecked;
    bBgChecked = MorphForm->BgChecked;
    DialogSaveMaxColors = MorphForm->MaxColors;

    YcDestroyForm(MorphForm);
    MorphForm = NULL;
  }
  else
  {
    // Get dialog's properties to save
    FgFromColor = fg;
    FgToColor = bg;
    BgFromColor = bg;
    BgToColor = fg;
    bFgChecked = true;
    bBgChecked = true;
    DialogSaveMaxColors = PaltalkMaxColors;
  }

  EParm3 = EM_FGBG;
  AddEffect(bMorph ? E_MORPH_FGBG : E_REPLACE_COLOR);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EffectRandomClick(TObject *Sender)
{
  int mode;
  bool bSkipSpaces;

  if (Sender)
  {
    Application->CreateForm(__classid(TAlternateForm), &AlternateForm);
    AlternateForm->ShowColors = false; // Hide the color-selection panels
    AlternateForm->DlgCaption = DS[25]; // Randomize Colors

    if (IsCli(C_PALTALK))
      AlternateForm->Mode = EM_FG;
    else
      AlternateForm->Mode = EM_FGBG;

    AlternateForm->ShowModal();

    if (AlternateForm->ModalResult == mrCancel)
    {
      YcDestroyForm(AlternateForm);
      AlternateForm = NULL;
      return;
    }

    bSkipSpaces = AlternateForm->SkipSpaces;
    mode = AlternateForm->Mode;

    YcDestroyForm(AlternateForm);
    AlternateForm = NULL; 
  }
  else
  {
    bSkipSpaces = false;

    if (IsCli(C_PALTALK))
      mode = EM_FG;
    else
      mode = EM_FGBG;
  }

  EParm3 = mode;
  EParm4 = bSkipSpaces;
  AddEffect(E_RAND_COLORS);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EffectAlternateClick(TObject *Sender)
{
  int ColorA, ColorB, mode;
  bool bSkipSpaces;

  if (Sender)
  {
    Application->CreateForm(__classid(TAlternateForm), &AlternateForm);
    AlternateForm->DlgCaption = DS[24]; // Alternate Colors

    if (IsCli(C_PALTALK) || IsCli(C_YAHELITE))
      AlternateForm->Mode = EM_FG;
    else
      AlternateForm->Mode = EM_FGBG;

    AlternateForm->ShowModal();

    if (AlternateForm->ModalResult == mrCancel)
    {
      YcDestroyForm(AlternateForm);
      AlternateForm = NULL;
      return;
    }

    ColorA = AlternateForm->ColorA;
    ColorB = AlternateForm->ColorB;
    bSkipSpaces = AlternateForm->SkipSpaces;
    mode = AlternateForm->Mode;

    YcDestroyForm(AlternateForm);
    AlternateForm = NULL;
  }
  else
  {
    ColorA = Afg;
    ColorB = Abg;
    bSkipSpaces = false;
    if (IsCli(C_PALTALK)) mode = EM_FG;
    else mode = EM_FGBG;
  }

  EParm1 = ColorA;
  EParm2 = ColorB;
  EParm3 = mode;
  EParm4 = bSkipSpaces;
  AddEffect(E_ALT_COLORS);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::IncrementForegroundBackgroundClick(TObject *Sender)
// if Sender is NULL don't use a dialog... called from a remote chat-client!
{
  int mode, offset;
  bool bSkipSpaces;

  // Delete old palette
  if (DTSColor->RGB_Palette)
  {
    try
    {
      delete [] DTSColor->RGB_Palette;
    }
    __finally
    {
      DTSColor->RGB_PaletteSize = 0;
      DTSColor->RGB_Palette = NULL;
    }
  }

  if (IsYahTrinPal())
  {
    int redDelta, greenDelta, blueDelta;

    if (Sender)
    {
      Application->CreateForm(__classid(TIncDecForm), &IncDecForm);
      IncDecForm->EnableEdits = true;
      IncDecForm->SkipSpaces = false;
      IncDecForm->PaletteSize = SMALLRGBPALETTESIZE;
      IncDecForm->Offset = SMALLRGBPALETTESIZE/2;
      IncDecForm->RedDelta = INCDEC_RED;
      IncDecForm->GreenDelta = INCDEC_GREEN;
      IncDecForm->BlueDelta = INCDEC_BLUE;

      if (IsCli(C_PALTALK))
        IncDecForm->Mode = EM_FG;
      else
        IncDecForm->Mode = EM_FGBG;

      IncDecForm->ShowModal();

      if (IncDecForm->ModalResult == mrCancel)
      {
        YcDestroyForm(IncDecForm);
        IncDecForm = NULL;
        return;
      }

      redDelta = IncDecForm->RedDelta;
      greenDelta = IncDecForm->GreenDelta;
      blueDelta = IncDecForm->BlueDelta;
      offset = IncDecForm->Offset;
      bSkipSpaces = IncDecForm->SkipSpaces;
      mode = IncDecForm->Mode;

      YcDestroyForm(IncDecForm);
      IncDecForm = NULL;
    }
    else
    {
      redDelta = INCDEC_RED;
      greenDelta = INCDEC_GREEN;
      blueDelta = INCDEC_BLUE;
      offset = SMALLRGBPALETTESIZE/2;
      bSkipSpaces = false;

      if (IsCli(C_PALTALK))
        mode = EM_FG;
      else
        mode = EM_FGBG;
    }

    // Make a new RGB Color-Palette
    DTSColor->RGB_PaletteSize = SMALLRGBPALETTESIZE;
    DTSColor->RGB_Palette = new int[DTSColor->RGB_PaletteSize];

    int cl;

    if (mode == EM_BG) // Background only? Use BG color as base for palette
      cl = utils->ConvertColor(Background, bRgbRandBG);
    else
      cl = utils->ConvertColor(Foreground, bRgbRandFG);

    BlendColor bc = utils->TColorToBlendColor(utils->YcToTColor(cl));

    bool bRedUp, bGreenUp, bBlueUp;

    if (redDelta < 0)
    {
      redDelta = -redDelta;
      bRedUp = false;
    }
    else
      bRedUp = true;

    if (greenDelta < 0)
    {
      greenDelta = -greenDelta;
      bGreenUp = false;
    }
    else
      bGreenUp = true;

    if (blueDelta < 0)
    {
      blueDelta = -blueDelta;
      bBlueUp = false;
    }
    else
      bBlueUp = true;

    for (int ii = 0; ii < DTSColor->RGB_PaletteSize; ii++)
    {
      IncDecColor(bc, bRedUp, bGreenUp, bBlueUp, redDelta, greenDelta, blueDelta);
      DTSColor->RGB_Palette[ii] = utils->BlendColorToRgb(bc);
    }
  }
  else // IRC Client... no editing deltas!
  {
    if (Sender)
    {
      Application->CreateForm(__classid(TIncDecForm), &IncDecForm);
      IncDecForm->EnableEdits = false; // Disable delta-color edit boxes
      IncDecForm->SkipSpaces = true; // Works better for IRC-Colors
      IncDecForm->PaletteSize = paletteSize;
      IncDecForm->Offset = 0;
      IncDecForm->Mode = EM_FGBG;

      IncDecForm->ShowModal();

      if (IncDecForm->ModalResult == mrCancel)
      {
        YcDestroyForm(IncDecForm);
        IncDecForm = NULL;
        return;
      }

      offset = IncDecForm->Offset;
      bSkipSpaces = IncDecForm->SkipSpaces;
      mode = IncDecForm->Mode;

      YcDestroyForm(IncDecForm);
      IncDecForm = NULL;
    }
    else
    {
      offset = 0;
      bSkipSpaces = true;
      mode = EM_FGBG;
    }
  }

  if (mode == EM_FG)
  {
    EParm1 = offset;
    EParm2 = 0;
  }
  else
  {
    EParm1 = 0;
    EParm2 = offset;
  }

  EParm3 = mode;
  EParm4 = bSkipSpaces;
  AddEffect(E_INC_COLORS);

  try
  {
    if (DTSColor->RGB_Palette)
    {
      delete [] DTSColor->RGB_Palette;
    }
  }
  __finally
  {
    DTSColor->RGB_Palette = NULL;
    DTSColor->RGB_PaletteSize = 0;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::IncDecColor(BlendColor &bc, bool &bRedUp,
  bool &bGreenUp, bool &bBlueUp, int redDelta, int greenDelta, int blueDelta)
// Can experiment here:
// Version A incremented/decremented by delta and capped the colors
// at 255/0. Version B halted inc/dec and reverses if the color will
// go out of range.
{
//  bool bVersionA = true;
//  int mult = 1;
  bool bVersionA = false;
  int mult = 2;

  if (bRedUp)
  {
    bc.red += redDelta;
    if (bc.red > 255)
    {
      if (bVersionA) bc.red = 255;
      else bc.red -= mult*redDelta;

      bRedUp = false;
    }
  }
  else
  {
    bc.red -= redDelta;
    if (bc.red < 0)
    {
      if (bVersionA) bc.red = 0;
      else bc.red += mult*redDelta;

      bRedUp = true;
    }
  }

  if (bGreenUp)
  {
    bc.green += mult*greenDelta;
    if (bc.green > 255)
    {
      if (bVersionA) bc.green = 255;
      else bc.green -= mult*greenDelta;

      bGreenUp = false;
    }
  }
  else
  {
    bc.green -= greenDelta;
    if (bc.green < 0)
    {
      if (bVersionA) bc.green = 0;
      else bc.green += mult*greenDelta;

      bGreenUp = true;
    }
  }

  if (bBlueUp)
  {
    bc.blue += blueDelta;
    if (bc.blue > 255)
    {
      if (bVersionA) bc.blue = 255;
      else bc.blue -= mult*blueDelta;

      bBlueUp = false;
    }
  }
  else
  {
    bc.blue -= blueDelta;
    if (bc.blue < 0)
    {
      if (bVersionA) bc.blue = 0;
      else bc.blue += mult*blueDelta;

      bBlueUp = true;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuStripCodesClick(TObject *Sender)
{
  // STRIP_ALL           0
  // STRIP_ALL_PRESERVE  1  // Keep codes within push/pop
  // STRIP_BOLD          2
  // STRIP_UNDERLINE     3
  // STRIP_ITALICS       4
  // STRIP_FG_COLOR      5
  // STRIP_BG_COLOR      6
  // STRIP_ALL_COLOR     7
  // STRIP_FONT_CODES    8
  // STRIP_PUSHPOP       9
  // STRIP_CTRL_O        10
  // STRIP_TAB           11
  // STRIP_FF            12

  EParm1 = utils->SelectStripMode();

  if (EParm1 >= 0)
  {
    switch (EParm1)
    {
      case STRIP_BOLD:
        EParm2 = CTRL_B;
        break;

      case STRIP_UNDERLINE:
        EParm2 = CTRL_U;
        break;

      case STRIP_ITALICS:
        EParm2 = CTRL_R;
        break;

      case STRIP_CTRL_O:
        EParm2 = CTRL_O;
        break;

      case STRIP_TAB:
        EParm2 = C_TAB;
        break;

      case STRIP_FF:
        EParm2 = C_FF;
        break;
    }

    EParm3 = -1;
    AddEffect(E_STRIP_CODES);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuStripSpacesClick(TObject *Sender)
{
  EParm1 = STRIP_TRIM_SPACES;
  EParm3 = -1;
  AddEffect(E_STRIP_CODES);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuPadSpacesClick(TObject *Sender)
{
/*
  Application->CreateForm(__classid(TPadSpacesForm), &PadSpacesForm);
//  PadSpacesForm->Mode =
//  PadSpacesForm->Width =
//  PadSpacesForm->Auto =
  PadSpacesForm->ShowModal();

  if (PadSpacesForm->ModalResult == mrCancel)
  {
    YcDestroyForm(PadSpacesForm);
    PadSpacesForm = NULL;
    return;
  }

  int Mode = PadSpacesForm->Mode;

  YcDestroyForm(PadSpacesForm);
  PadSpacesForm = NULL;

  // Init
  EParm3 = Mode;

  // PS_PAD_R        0
  // PS_PAD_L        1
  // PS_PAD_LR       2
  // PS_CENTER       3
  // PS_L_JUSTIFY    4
  // PS_R_JUSTIFY    5
  // PS_STRIP_R      6
  // PS_STRIP_L      7
  // PS_STRIP_LR     8
  switch(Mode)
  {
    default:
    case PS_PAD_R:

//      EParm0 = CA;
//      EParm1 = CB;
//      EParm2 = CTRL_U;
//      EParm4 = C_NULL;
      EParm1 = STRIP_PAD_SPACES;

    break;
  }
  AddEffect(E_STRIP_CODES);
*/

  // work in-progress above - new dialog

  EParm1 = STRIP_PAD_SPACES;
  EParm4 = regWidth;
  AddEffect(E_STRIP_CODES);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuJoinLinesClick(TObject *Sender)
{
  EParm1 = STRIP_CRLF;
  EParm3 = -1;
  AddEffect(E_STRIP_CODES);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuUnderlineClick(TObject *Sender)
{
  Application->CreateForm(__classid(TUnderlineForm), &UnderlineForm);
  UnderlineForm->Mode = AL_ULCOLOR;
  UnderlineForm->DlgCaption = DS[72]; // Alternate Character
  UnderlineForm->ShowModal();

  if (UnderlineForm->ModalResult == mrCancel)
  {
    YcDestroyForm(UnderlineForm);
    UnderlineForm = NULL;
    return;
  }

  int CA = UnderlineForm->DlgColorA;
  int CB = UnderlineForm->DlgColorB;
  int Mode = UnderlineForm->Mode;

  YcDestroyForm(UnderlineForm);
  UnderlineForm = NULL;

  // Init
  EParm3 = Mode;

  // AL_ULCOLOR        0
  // AL_BOLD           1
  // AL_ITALICS        2
  // AL_BOLDITALICS    3
  // AL_BOLDCOLOR      4
  // AL_ITALICSCOLOR   5
  // AL_COLOR          6
  switch(Mode)
  {
    case AL_ULCOLOR:

      EParm0 = CA;
      EParm1 = CB;
      EParm2 = CTRL_U;
      EParm4 = C_NULL;

    break;

    case AL_ULBOLD:

      EParm0 = NO_COLOR;
      EParm1 = NO_COLOR;
      EParm2 = CTRL_U;
      EParm4 = CTRL_B;

    break;

    case AL_ULITALICS:

      EParm0 = NO_COLOR;
      EParm1 = NO_COLOR;
      EParm2 = CTRL_U;
      EParm4 = CTRL_R;

    break;

    case AL_BOLDITALICS:

      EParm0 = NO_COLOR;
      EParm1 = NO_COLOR;
      EParm2 = CTRL_B;
      EParm4 = CTRL_R;

    break;

    case AL_BOLDCOLOR:

      EParm0 = CA;
      EParm1 = CB;
      EParm2 = CTRL_B;
      EParm4 = C_NULL;

    break;

    case AL_ITALICSCOLOR:

      EParm0 = CA;
      EParm1 = CB;
      EParm2 = CTRL_R;
      EParm4 = C_NULL;

    break;

    default:
    case AL_COLOR:

      EParm0 = CA;
      EParm1 = CB;
      EParm2 = C_NULL;
      EParm4 = C_NULL;

    break;
  };

  AddEffect(E_ALT_CHAR);
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::SelectFontSize(String Topic, int &Size)
// Called from DlgWingEdit FontSizeCode1Click
// and from FontSizeMenuItemClick in main when Effects Size
//
// Returns int of font-size or IRCCANCEL
{
  try
  {
    Application->CreateForm(__classid(TFontSizeForm), &FontSizeForm);

    if (FontSizeForm == NULL) return false;

    FontSizeForm->Caption = Topic;
    FontSizeForm->FontSize = Size;
    FontSizeForm->ShowModal();

    if (FontSizeForm->ModalResult == mrOk)
    {
      Size = FontSizeForm->FontSize;
      YcDestroyForm(FontSizeForm);
      FontSizeForm = NULL;
      return true;
    }

    Size = IRCCANCEL;
    YcDestroyForm(FontSizeForm);
    FontSizeForm = NULL;
    return false;
  }
  catch(...)
  {
    YcDestroyForm(FontSizeForm);
    FontSizeForm = NULL;
    return false;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::SelectFontType(String Topic, int &Type)
// Called from DlgWingEdit FontTypeCode1Click
// and from FontTypeMenuItemClick in main when Effects Type
//
// Returns int of font-size or IRCCANCEL
{
  try
  {
    Application->CreateForm(__classid(TFontTypeForm), &FontTypeForm);

    if (FontTypeForm == NULL)
      return false;

    FontTypeForm->Caption = Topic;
    FontTypeForm->FontType = Type;
    FontTypeForm->ShowModal();

    if (FontTypeForm->ModalResult == mrOk)
    {
      Type = FontTypeForm->FontType;
      YcDestroyForm(FontTypeForm);
      FontTypeForm = NULL;
      return true;
    }

    Type = IRCCANCEL;
    YcDestroyForm(FontTypeForm);
    FontTypeForm = NULL;
    return false;
  }
  catch(...)
  {
    YcDestroyForm(FontTypeForm);
    FontTypeForm = NULL;
    return false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuSetFontClick(TObject *Sender)
// Called when user clicks Tools->Font
{
  TFontDialog* FontDlg = new TFontDialog(this);

  FontDlg->Options.Clear();
  // fdAnsiOnly	Displays only fonts that use the Windows character set.
  //  Symbol fonts are not listed in the dialog.
  // fdApplyButton	Displays an Apply button in the dialog, whether or not
  //  there is an OnApply event handler.
  // fdEffects	Displays the Effects check boxes (Strikeout and Underline) and
  //  the Color list box in the dialog.
  // fdFixedPitchOnly	Displays only monospaced fonts in the dialog.
  //  Proportionally spaced fonts are not listed, nor are TrueType fonts that
  //  do not have the fixed-pitch flag set in the style header.
  //
  // fdForceFontExist	Allows the user to enter only fonts that are displayed in
  //  the dialog (that is, listed in the Font combo box). If the user tries to
  //  enter another font name, an error message appears.
  // fdLimitSize	Enables the MaxFontSize and MinFontSize properties, limiting
  //  the range of font sizes that appear in the dialog if these properties have values.
  // fdNoFaceSel	Causes the dialog to open without a preselected font name in
  //  the Font combo box.
  // fdNoOEMFonts	Removes OEM fonts from the dialog’s combo box. Lists only
  //  non-OEM fonts.
  //
  // fdScalableOnly	Displays only scalable fonts in the dialog. Non-scalable
  //  (bitmap) fonts are removed from the list.
  // fdNoSimulations	Displays only fonts and font styles that are directly
  //  supported by the font definition file. GDI-synthesized bold and italic
  //  styles (for bitmap fonts) are not listed.
  // fdNoSizeSel	Causes the dialog to open without a preselected size in the
  //  Size combo box.
  // fdNoStyleSel	Causes the dialog to open without a preselected style in the
  //  Font Style combo box.
  //
  // fdNoVectorFonts	Removes vector fonts from the dialog’s combo box; lists
  //  only non-vector fonts. (Vector fonts are Windows 1.0 fonts, such as Roman
  //  or Script, which resemble pen-plotter output.)
  // fdShowHelp	Displays a Help button in the dialog.
  // fdTrueTypeOnly	Displays only TrueType fonts in the dialog. Other fonts
  //  are not listed.
  // fdWysiwyg	Displays only fonts that are available to both the printer and
  //  the screen. Device-specific fonts are not listed in the dialog.
  //
  // By default, all options except fdEffects are off.
  //FontDlg->Options << fdWysiwyg;

  if (!IsYahTrinPal())
    utils->ShowMessageU(String(DS[76])); //warn only Bold, etc for IRC

  FontDlg->Font->Charset = cCharset;
  FontDlg->Font->Name = utils->WideToUtf8(cFont);
  FontDlg->Font->Pitch = cPitch;
  FontDlg->Font->Size = cSize;
  FontDlg->Font->Style = cStyle;

  if (FontDlg->Execute())
  {
    if (tae->View == V_RTF)
    {
      //"Changing the font requires changing to the\n"
      //"Original text view and reprocessing. Any changes\n"
      //"you have made will be lost. Continue?", // 73
      if (utils->ShowMessageU(Handle, DS[73],
                      MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDNO)
      {
        delete FontDlg;
        return;
      }
    }

    cCharset = FontDlg->Font->Charset;
    this->cFont = utils->Utf8ToWide(FontDlg->Font->Name);
    cPitch = FontDlg->Font->Pitch;
    cSize = FontDlg->Font->Size;

    if (cSize > 99)
      cSize = 99;
    else if (cSize < 1)
      cSize = 1;

    // Init an index into our local FONTS[] table in DefaultStrings.cpp
    cType = utils->GetLocalFontIndex(cFont);

    cStyle = FontDlg->Font->Style;

//ShowMessage("name: " + cFont + " Height: " + String(FontDlg->Font->Height) +
// " Pitch: " + String(FontDlg->Font->Pitch)+ " Size: " + String(FontDlg->Font->Size));

    utils->SetRichEditFont(tae);

    // HAVE to do this because changing the font messes up the
    // RTF Processed text.
    if (tae->View == V_RTF)
    {
      ClearAndFocus(true, true); // leave V_ORG
      LoadView(V_ORG);
    }
    else
      LoadView(tae->View); // Repaint
  }

  delete FontDlg;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FontSawtoothClick(TObject *Sender)
{
  DoFontEffect(E_FONT_SAWTOOTH, cSize - (50*cSize/100), cSize + (200*cSize/100), 0);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FontRandomClick(TObject *Sender)
{
  DoFontEffect(E_FONT_RANDOM, cSize - (25*cSize/100), cSize + (50*cSize/100), 0);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FontMountainClick(TObject *Sender)
{
  DoFontEffect(E_FONT_MOUNTAIN, cSize - (25*cSize/100), cSize + (50*cSize/100), MYPI);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FontValleyClick(TObject *Sender)
{
  DoFontEffect(E_FONT_VALLEY, cSize + (50*cSize/100), cSize - (25*cSize/100), MYPI);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FontRise1Click(TObject *Sender)
{
  DoFontEffect(E_FONT_MOUNTAIN, cSize - (25*cSize/100), cSize + (50*cSize/100), MYPI/2);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FontDipClick(TObject *Sender)
{
  DoFontEffect(E_FONT_VALLEY, cSize + (50*cSize/100), cSize - (25*cSize/100), MYPI/2);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FontDoubleMountainClick(TObject *Sender)
{
  DoFontEffect(E_FONT_MOUNTAIN, cSize + (50*cSize/100), cSize - (25*cSize/100), 2*MYPI);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::DoFontEffect(int type, int p1, int p2, double p5)
{
  if (Borders->Checked)
  {
    if (tae->View == V_RTF)
    {
      if (utils->ShowMessageU(Handle, DS[148],
                      MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDYES)
        return;
    }
    else
    {
      // Border edges warning asking to turn off borders...
      if (utils->ShowMessageU(Handle, DS[147],
                      MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDYES)
        Borders->Checked = false;
    }
  }

  EParm1 = p1; EParm2 = p2; EParm3 = -1; EParm5 = p5;
  AddEffect(type);
}
//---------------------------------------------------------------------------
// END EFFECTS
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Blender Functions
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BlendFGMenu(TObject *Sender)
// From Effects Menu
{
  if (tae->TextLength == 0)
    return;

  SelectBlendPreset(String(DS[63])); // Foreground

  if (GBlendPreset >= 0)
  {
    // Load bG_BlendColors
    if (!LoadBlendPresetFromRegistry(fG_BlendColors, GBlendPreset))
      utils->ShowMessageU(String(DS[221])); // Preset is empty or corrupt
    else
    {
      fG_BlendPreset = GBlendPreset;
      if (!CreateBlendEngine(BLEND_FG))
        return;

      if (!DoBlend(E_FG_BLEND, true))
        ProcessError(11);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BlendBGMenu(TObject *Sender)
// From Effects Menu
{
  if (tae->TextLength == 0)
    return;

  SelectBlendPreset(String(DS[65])); // Background

  if (GBlendPreset >= 0)
  {
    // Load bG_BlendColors
    if (!LoadBlendPresetFromRegistry(bG_BlendColors, GBlendPreset))
      utils->ShowMessageU(String(DS[221])); // Preset is empty or corrupt
    else
    {
      bG_BlendPreset = GBlendPreset;
      if (!CreateBlendEngine(BLEND_BG))
        return;

      if (!DoBlend(E_BG_BLEND, true))
        ProcessError(12);
    }
  }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::DoBlend(int Type, bool bUndo)
{
  // Blend-before-process or User clicked it in
  // the Effects menu
  try
  {
    if (Type == E_FG_BLEND)
      EParm3 = EM_FG;
    else
      EParm3 = EM_BG;

    // If view is RTF, we don't want to save the undo-buffer
    // but we do want to "re-process" the text to which we are
    // adding the new effect.
    if (tae->View == V_RTF)
      AddEffect(Type, false, true);
    else // Clear "bReprocess" or we recursivelly loop!
      AddEffect(Type, bUndo, false);

    if (!DeleteBlenderObjects())
      return false;

    // Need this for some reason... still some messages
    // slow to process otherwise...
    Application->ProcessMessages();

    if (WindowState != wsMinimized)
      tae->SetFocus();

    return true;
  }
  catch(...)
  {
    DeleteBlenderObjects();
    return false;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::DeleteBlenderObjects(void)
{
  try
  {
    if (fG_BlendEngine)
    {
      delete fG_BlendEngine;
      fG_BlendEngine = NULL;
    }
    if (bG_BlendEngine)
    {
      delete bG_BlendEngine;
      bG_BlendEngine = NULL;
    }

    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::CreateBlendEngine(int Type)
{
  try
  {
    BlendColor* BC;

    if (Type == BLEND_FG || Type == BLEND_VERT_FG)
    {
      BC = fG_BlendColors;

      if (fG_BlendEngine != NULL) delete fG_BlendEngine;
      fG_BlendEngine = new TBlendEngine();

      pBE = fG_BlendEngine;
    }
    else // BLEND_BG
    {
      BC = bG_BlendColors;

      if (bG_BlendEngine != NULL) delete bG_BlendEngine;
      bG_BlendEngine = new TBlendEngine();

      pBE = bG_BlendEngine;
    }

    if (!pBE)
      return(false);

    // have to have the objects to continue...
    if (!BC || !pBE->EBC)
    {
      DeleteBlendEngine();
      return false;
    }

    // Override default threshold if not vertical-blend
    if (Type == BLEND_FG || Type == BLEND_BG)
      pBE->Threshold = RGBThreshold;

    // Return the # of active blend buttons OR
    //
    // Return the real text-length if it is less than
    // the # of active blend buttons.
    int EnabledCount = GetEnabledBlendButtonCount(BC);

    if (EnabledCount < 0)
    {
      if (EnabledCount == -4) // < 2 buttons
        utils->ShowMessageU(String(DS[152]));
      else
        utils->ShowMessageU(String(DS[150]) + String(EnabledCount));

      DeleteBlendEngine();
      return false;
    }

    // Fill an array with the enabled blend-button colors.
    // If the text length is less than the number of
    // enabled buttons, we fill in a list equal to the text-length,
    // taking the left-most buttons inward and the right-most
    // buttons inward... this will mean that the starting and ending
    // colors will appear when the text is short.
    if (!PopulateEnabledBlendStructs(BC, EnabledCount, Type))
    {
      DeleteBlendEngine();
      return false;
    }

    return true;
  }
  catch(...)
  {
    DeleteBlendEngine();

    utils->ShowMessageU(String(DS[153]));
    return false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::DeleteBlendEngine(void)
{
  try
  {
    if (pBE)
      {
      delete pBE;

      if (pBE == fG_BlendEngine)
        fG_BlendEngine = NULL;
      else if (pBE == bG_BlendEngine)
        bG_BlendEngine = NULL;

      pBE = NULL;
      }
  }
  catch(...)
  {
  }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::PopulateEnabledBlendStructs(BlendColor* BC,
                                   int EnabledCount, int Type)
{
  if (!pBE || !pBE->EBC || !BC || EnabledCount < 2)
    return false;

  // We put in two-colors per iteration of the For-loop...
  int Count = EnabledCount/2;
  if (EnabledCount % 2) // Remainder, round up
    Count++;

  // Keep Count of values written into the structure
  int NewCount = 0;

  int index1 = -1;
  int index2 = MAXBLENDCOLORS;

  // Form list of enabled blend-structs
  // Work our way in from the left-most and right-most buttons
  // adding the enabled buttons...
  for (int ii = 0; ii < Count; ii++)
  {
    index1 = GetNextEnabledBlendIndex(index1+1, BC);

    pBE->EBC[ii] = BC[index1];
    NewCount++;

    // Breaks out for odd numbers...
    if (NewCount >= EnabledCount)
      break;

    index2 = GetPrevEnabledBlendIndex(index2-1, BC);

    pBE->EBC[EnabledCount-ii-1] = BC[index2];
    NewCount++;
  }

  tBlendDirection bd = (tBlendDirection)BlendDirectionRadioButtons->ItemIndex;

  // Invert the background-blend direction to avoid wash-out
  if ((Type == BLEND_VERT_BG || Type == BLEND_BG) &&
        (Foreground == IRCVERTBLEND || Foreground == IRCHORIZBLEND) &&
                                     fG_BlendPreset == bG_BlendPreset)
    {
    if (bd == RTL)
      bd = LTR;
    else if (bd == LTR)
      bd = RTL;
    else if (bd == CENTER1)
      bd = CENTER2;
    else
      bd = CENTER1;
    }

  // Left-to-Right blend is default... we are finished in that case
  if (bd == RTL || bd == CENTER2) // Right-to-Left or Center2?
    {
    // Invert the array
    BlendColor temp;
    int idx;

    for (int ii = 0; ii < EnabledCount/2; ii++)
      {
      temp = pBE->EBC[ii];
      idx = (EnabledCount-1)-ii;
      pBE->EBC[ii] = pBE->EBC[idx];
      pBE->EBC[idx] = temp;
      }
    }

  if (bd == CENTER1 || bd == CENTER2) // Center1 or Center2?
    {
    // We doubled the memory already
    int idx;

    for (int ii = 0; ii < EnabledCount; ii++)
      {
      idx = ((EnabledCount*2)-1)-ii;
      pBE->EBC[idx] = pBE->EBC[ii];
      }

    EnabledCount *= 2;
    }

  // Set the Engine's property
  pBE->BlendEnabledCount = EnabledCount;

  return true;
}
//---------------------------------------------------------------------------
int __fastcall TDTSColor::GetNextEnabledBlendIndex(int idx, BlendColor* BC)
{
  for (; idx < MAXBLENDCOLORS; idx++)
    if (BC[idx].enabled)
      break;

  return(idx);
}
//---------------------------------------------------------------------------
int __fastcall TDTSColor::GetPrevEnabledBlendIndex(int idx, BlendColor* BC)
{
  for (; idx > 0; idx--)
    if (BC[idx].enabled)
      return(idx);

  return(0);
}
//---------------------------------------------------------------------------
int __fastcall TDTSColor::GetEnabledBlendButtonCount(BlendColor* BC)
// Returns < 0 if error
{
  int Count = 0;

  if (BC == NULL)
    return(-1);

  // Get count of enabled blend-structs
  for (int ii = 0; ii < MAXBLENDCOLORS; ii++)
    if (BC[ii].enabled)
      Count++;

  if (Count < 2)
    return(-4);

  wchar_t* Buf = NULL;
  int Size;

  utils->MoveMainTextToBuffer(Buf, Size);

  if (Buf == NULL)
    return(-2);

  if (Size == 0)
  {
    delete [] Buf;
    return -3;
  }

  // Get the text without color-codes and emotion-codes
  int Length = utils->GetRealLength(Buf, Size, 0, '\0', false);

  delete [] Buf;

  if (Length < Count)
    {
    Count = Length;
    if (Count < 2)
      Count = 2;
    }

  // Return Count ot the number of blend-buttons we will use
  return(Count);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BlendMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (PageControl1->ActivePage != tsBlender || !fG_BlendColors)
    return;

  if (Button == mbLeft)
    BlendPanelSelectClick(Sender);
  else if (Button == mbRight)
  {
    TPanel* P = (TPanel*)Sender;

    if (P->Tag == BlendButtonSelectIndex)
      BlendPanelEnableClick(Sender);
    else // invoke the universal popup menu for colors
    {
      TPoint* in = new TPoint();
      if (GetCursorPos(in))
      {
        ColorCopyPasteMenu->PopupComponent = (TComponent*)P;
        ColorCopyPasteMenu->Popup(in->x, in->y);
      }
      delete in;
    }
  }
  else // Center
  {
    if (MouseBlendIndex >= 0 && BlendButtonSelectIndex >= 0)
    {
      fG_BlendColors[BlendButtonSelectIndex] = MouseBlendColor;

      // Get a pointer to the selected panel...
      TPanel* P = GetPointerToColorPanel(BlendButtonSelectIndex);
      P->Color = utils->BlendColorToTColor(fG_BlendColors[BlendButtonSelectIndex]);
      MouseBlendIndex = -1; // restored
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BlendPanelSelectClick(TObject *Sender)
// Selects the panel on a left-click
{
  TPanel* P = (TPanel*)Sender;

  // Get a pointer to each blend-panel
  for (int ii = 0; ii < MAXBLENDCOLORS; ii++)
  {
    if (ii == P->Tag)
    {
      if (BlendButtonSelectIndex == ii) // Already selected?
        BlendPanelColorClick(P);
      else
        BlendButtonSelectIndex = ii; // Select it...

      MouseBlendIndex = -1; // Erase the restore memory
    }
  }

  PaintBlendColorButtons(); // Need to paint new bevel-states
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BlendPanelColorClick(TPanel* P)
{
  // Show user-defined color-dialog
  if (fG_BlendColors == NULL)
    return;

  TColor c = P->Color;

  // returns c by reference!
  if (!utils->GetTColorDialog(c))
    return; // Cancel-button

  SetBlendColor(P, c);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SetBlendColor(TPanel* p, TColor c)
{
  // Be careful of this, don't depend on the name property being
  // "Blend0 - Blend9" -- set and use the Tag property
  fG_BlendColors[p->Tag] = utils->TColorToBlendColor(c);

  // Change panel to new color
  p->Color = c;

  // Enable this panel automatically
  p->BevelInner = bvRaised;
  p->Caption = "";
  fG_BlendColors[p->Tag].enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BlendPanelEnableClick(TObject *Sender)
{
  if (fG_BlendColors == NULL) return;

  TPanel* P = (TPanel *)Sender;

  // Be careful of this, don't depend on the name property being
  // "Blend0 - Blend9" -- set and use the Tag property
  if (fG_BlendColors[P->Tag].enabled)
  {
    if (P->Color == clBlack) P->Font->Color = clWhite;
    else P->Font->Color = clBlack;

    P->Caption = "X";
    fG_BlendColors[P->Tag].enabled = false;
  }
  else
  {
    P->Caption = "";
    fG_BlendColors[P->Tag].enabled = true;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SaveWorkingBlendPreset(void)
{
  BlendColor* TempColors = new BlendColor[MAXBLENDCOLORS];
  if (LoadBlendPresetFromRegistry(TempColors, fG_BlendPreset))
    if (!BlendColorsEqual(fG_BlendColors, TempColors))
      if (utils->ShowMessageU(Handle, DS[154],
                      MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDYES)
        SaveBlendPresetToRegistry(fG_BlendColors, fG_BlendPreset);
  delete [] TempColors;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ExportBlender(TObject *Sender)
{
  if (SFDlgForm) // just ONE!
    return;

  const char* Qual = OUR_TITLE; // write this to the file as-is...
  int Quallen = strlen(Qual);
  int SizeOfOnePreset = MAXBLENDCOLORS*sizeof(BlendColor);

  // See if user had been editing colors...
  SaveWorkingBlendPreset();

  try
  {
    WideString wFilter = WideString("All files (*.*)|*.*|"
                                   "Binary files (*.bin)|*.bin");

    // "Blendcolors1.bin""
    WideString wFile = utils->GetSaveFileName(wFilter, U16(FN[13]),
                                                 DeskDir, String(DS[156]));

    if (wFile.IsEmpty())
      return;

    // open or create binary file
    FILE *f = _wfopen(wFile.c_bstr(), L"wb");

    if (!f)
    {
      utils->ShowMessageU(String(DS[161]));
      return;
    }

    fwrite(Qual, 1, Quallen, f);

    BlendColor* Preset = new BlendColor[ MAXBLENDCOLORS  ];

    // Format data into the buffer
    for (int ii = MAXBLENDPRESETS; ii > 0; ii--)
    {
      if (!LoadBlendPresetFromRegistry(Preset, ii-1))
        // Init...
        for (int jj = 0; jj < MAXBLENDCOLORS; jj++)
        {
          Preset[jj].red = 0xaa;
          Preset[jj].green = 0xaa;
          Preset[jj].blue = 0xaa;
          Preset[jj].enabled = false;
        }

      fwrite((char *)Preset, 1, SizeOfOnePreset, f);
    }

    delete [] Preset;
    fclose(f);
  }
  catch(...)
  {
    utils->ShowMessageU(String(DS[158]));
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ExportBlenderText(TObject *Sender)
{
  if (SFDlgForm) // just ONE!
    return;

  // User clicked Export Text button
  try
  {
    WideString wFilter = WideString("All files (*.*)|*.*|"
                                   "Text files (*.txt)|*.txt");

    // "BlendColors1.txt" FN[26]
    WideString wFile = utils->GetSaveFileName(wFilter, FN[26],
                                                 DeskDir, String(DS[159]));

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

    fprintf(f, "%s", "BLEND:");

    int Count = 0;

    // Count # of continuous Xed out color panels from the
    // end back for this preset.
    for (int ii = MAXBLENDCOLORS-1; ii >= 0 ; ii--)
    {
      if (fG_BlendColors[ii].enabled)
        break;

      Count++;
    }

    // Print to ASCII hex format
    for (int ii = 0; ii < MAXBLENDCOLORS-Count; ii++)
      fprintf(f, "%02x%02x%02x ", fG_BlendColors[ii].red,
          fG_BlendColors[ii].green, fG_BlendColors[ii].blue);

    fclose(f);
  }
  catch(...)
  {
      utils->ShowMessageU(String(DS[162]));
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ImportBlenderText(TObject *Sender)
{
  if (OFSSDlgForm) // just ONE!
    return;

  // User clicked Import Text button
  try
  {
    WideString wFilter = WideString("All files (*.*)|*.*|"
                    "Text files (*.txt)|*.txt");

    // Run the open-file dialog in FormOFDlg.cpp
    WideString wFile = utils->GetOpenFileName(wFilter, 2, DeskDir, String(DS[163]));

    // Load and convert file as per the file-type (either plain or rich text)
    if (wFile.IsEmpty())
      return;

    FILE *in = _wfopen(wFile.c_bstr(), L"r");

    if (!in)
    {
      utils->ShowMessageU(String(DS[164]));
      return;
    }

    char s[7];

    if (fscanf(in, "%6s", s) != 1 || strcmp(s, "BLEND:") != 0)
    {
      utils->ShowMessageU(String(DS[164]));
      return;
    }

    int c[MAXBLENDCOLORS], ii;
    for (ii = 0; ii < MAXBLENDCOLORS; ii++)
      if (fscanf(in, "%06x", &c[ii]) == EOF)
        break;

    fclose(in);

    for (int jj = 0; jj < MAXBLENDCOLORS; jj++)
    {
      if (jj < ii)
      {
        BlendColor BC = utils->RgbToBlendColor(c[jj]);

        fG_BlendColors[jj].red = BC.red;
        fG_BlendColors[jj].green = BC.green;
        fG_BlendColors[jj].blue = BC.blue;
        fG_BlendColors[jj].enabled = true;
      }
      else
      {
        fG_BlendColors[jj].red = 0xaa;
        fG_BlendColors[jj].green = 0xaa;
        fG_BlendColors[jj].blue = 0xaa;
        fG_BlendColors[jj].enabled = false;
      }
    }

    // Paint the blend-color buttons
    PaintBlendColorButtons();
    // Paint the presets (do this AFTER calling PaintBlendColorButtons())
    PaintPresetButtons();
  }
  catch(...)
  {
      utils->ShowMessageU(String(DS[165]));
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ImportBlender(TObject *Sender)
{
  if (OFSSDlgForm) // just ONE!
    return;

  const char* Qual = OUR_TITLE;
  int Quallen = strlen(Qual);
  int SizeOfOnePreset = MAXBLENDCOLORS*sizeof(BlendColor);
  int br;

  try
  {
    WideString wFilter = WideString("All files (*.*)|*.*|"
                    "Binary files (*.bin)|*.bin");

    // Run the open-file dialog in FormOFDlg.cpp
    WideString wFile = utils->GetOpenFileName(wFilter, 2, DeskDir, String(DS[166]));

    // Load and convert file as per the file-type (either plain or rich text)
    if (wFile.IsEmpty())
      return;

    FILE *f = _wfopen(wFile.c_bstr(), L"rb");

    if (!f)
      return;

    long fileLen = utils->filesize(f);
    long expected = (MAXBLENDPRESETS*SizeOfOnePreset)+Quallen;

    if (fileLen != expected)
    {
      utils->ShowMessageU(String(DS[167]));
      fclose(f);
      return; // Cancel
    }

    char* Qualstr = new char[Quallen+1];

    if (Qualstr == NULL)
      return;

    br = fread(Qualstr, 1, Quallen, f);

    if (br != Quallen || strncmp(Qual, Qualstr, Quallen) != 0)
    {
      utils->ShowMessageU(String(DS[167]));
      fclose(f);
      delete [] Qualstr;
      return;
    }

    delete [] Qualstr;

    BlendColor* Preset = new BlendColor[MAXBLENDCOLORS];

    for (int ii = MAXBLENDPRESETS; ii > 0; ii--)
    {
      br = fread((char *)Preset, 1, SizeOfOnePreset, f);

      if (br != SizeOfOnePreset)
      {
        utils->ShowMessageU(String(DS[151]));
        fclose(f);
        return;
      }

      SaveBlendPresetToRegistry(Preset, ii-1);
    }

    // Update working structure
    fG_BlendPreset = 0;
    LoadBlendPresetFromRegistry(fG_BlendColors, fG_BlendPreset);
    PaintBlendColorButtons();
    PaintPresetButtons();

    delete [] Preset;
    fclose(f);
  }
  catch(...)
  {
    utils->ShowMessageU(String(DS[38]));
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ImportTextButtonMouseDown(TObject* Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (fG_BlendColors == NULL)
    return;

  if (Button == mbRight) // Export
    ExportBlenderText(this);
  else if (Button == mbLeft) // Import
    ImportBlenderText(this);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ImportButtonMouseDown(TObject* Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (fG_BlendColors == NULL)
    return;

  if (Button == mbRight) // Export
    ExportBlender(this);
  else if (Button == mbLeft) // Import
    ImportBlender(this);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BlendPresetMouseDown(TObject* Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  // button 0-9 mouse-down
  // first 6 letters of "Name" property must be "Caption"
  // use the Tag property for a 0-based index
  if (Button == mbRight)
    BlendPresetStoreClick(Sender);
  else if (Button == mbLeft)
    BlendPresetLoadClick(Sender);

  // Paint the blend-color buttons
  PaintBlendColorButtons();
  // Paint the presets (do this AFTER calling PaintBlendColorButtons())
  PaintPresetButtons();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BlendPresetLoadClick(TObject* Sender)
{
  // User left-clicked a preset button from the "Blend" tab
  if (fG_BlendColors == NULL || Sender == NULL)
    return;

  TButton* P = (TButton*)Sender;

  // Check the tag
  if (P->Tag < 0 || P->Tag >= MAXBLENDPRESETS)
    return;

  BlendColor* TempColors = new BlendColor[MAXBLENDCOLORS];

  // See if user tried to load an empty preset
  if (!LoadBlendPresetFromRegistry(TempColors, P->Tag))
    utils->ShowMessageU(String(DS[169]));
  else
  {
    // See if user had been editing colors...
    // Offer to save them in old preset before over-writing
    if (LoadBlendPresetFromRegistry(TempColors, fG_BlendPreset))
      if (!BlendColorsEqual(fG_BlendColors, TempColors))
        if (utils->ShowMessageU(Handle, DS[154],
                      MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDYES)
          SaveBlendPresetToRegistry(fG_BlendColors, fG_BlendPreset);

    // Load the new preset colors and change the preset index
    if (LoadBlendPresetFromRegistry(fG_BlendColors, P->Tag))
    {
      fG_BlendPreset = P->Tag; // Change the preset-index...

      // Erase the restore memory, clear selected button...
      BlendButtonSelectIndex = -1;
      MouseBlendIndex = -1;

      // Change color to horizontal-blender... user may be
      // confused if they select a blend preset but no blending
      // happens because the Foreground color is set to some
      // other color.
      if (Foreground != IRCVERTBLEND && Foreground != IRCHORIZBLEND)
        Foreground = IRCHORIZBLEND;
    }
  }

  delete [] TempColors;
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::BlendColorsEqual(BlendColor P1[], BlendColor P2[])
{
  for (int ii = 0; ii < MAXBLENDCOLORS; ii++)
    if (!BlendColorEqual(P1[ii], P2[ii]))
      return false;

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::BlendColorEqual(BlendColor BC1, BlendColor BC2)
{
  if (BC1.red == BC2.red &&
      BC1.green == BC2.green &&
       BC1.blue == BC2.blue &&
       BC1.enabled == BC2.enabled)
    return true;

  return false;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BlendPresetStoreClick(TObject* Sender)
{
  // User right-clicked a preset button from the "Blend" tab
  if (fG_BlendColors == NULL || Sender == NULL)
    return;

  TButton* P = (TButton*)Sender;

  // Check the tag
  if (P->Tag < 0 || P->Tag >= MAXBLENDPRESETS)
    return;

  // Save the preset to the registry from the BlendColors array
  if (SaveBlendPresetToRegistry(fG_BlendColors, P->Tag)) // (Updates preset-index also)
  {
    // Success...
    fG_BlendPreset = P->Tag;
    utils->ShowMessageU(String(DS[171]) + String(fG_BlendPreset+1) + "!");
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::RGBThresholdEditChange(TObject *Sender)
{
  // RGBThreshold was added because there was too much data being
  // sent for blends... a #000000 for every char is too much... we
  // hit the 500-byte limit frequently
//  try
//  {
//    RGBThresholdUpDown->Position = (short)RGBThresholdEdit->Text.ToIntDef(0);
//
//    if (RGBThresholdUpDown->Position < 0)
//      {
//      RGBThresholdUpDown->Position = 0;
//      RGBThresholdEdit->Text = String(RGBThresholdUpDown->Position);
//      }
//    else if (RGBThresholdUpDown->Position > 255)
//      {
//      RGBThresholdUpDown->Position = 255;
//      RGBThresholdEdit->Text = String(RGBThresholdUpDown->Position);
//      }
//
//    RGBThreshold = (int)RGBThresholdUpDown->Position;
//  }
//  catch(...)
//  {
//    RGBThresholdUpDown->Position = RGBTHRESHOLD; // default
//  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::RGBThresholdUpDownClick(TObject *Sender,
      TUDBtnType Button)
{
  RGBThreshold = RGBThresholdUpDown->Position;
  RGBThresholdEdit->Text = String(RGBThreshold);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PageControl1Change(TObject *Sender)
{
  if (PageControl1->ActivePage == tsBlender)
  {
    BlendScopeRadioButtons->Visible = true;
    BlendDirectionRadioButtons->Visible = true;
    BlendExtensionPanel->Visible = true;
    PageControl1->ActivePage->ShowHint = true;

    RGBThresholdEdit->Text = String(RGBThreshold);
    RGBThresholdUpDown->Position = (short)RGBThreshold;

    PaintBlendColorButtons();
    PaintPresetButtons();
  }
  else
  {
    // Clear the saved mouse-wheel-blend
    MouseBlendIndex = -1;

    // Nothing selected
    BlendButtonSelectIndex = -1;

    BlendScopeRadioButtons->Visible = false;
    BlendDirectionRadioButtons->Visible = false;
    BlendExtensionPanel->Visible = false;
    PageControl1->ActivePage->ShowHint = false;

    if (PageControl1->ActivePage == TabControl)
    {
      UpdateColorButtons(); // Forground needs to be painted if set to "Blend"
      UpdateProcessButtons(tae->View);
    }

    if (WindowState != wsMinimized)
      tae->SetFocus();
  }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::PaintBlendColorButtons(void)
{
  if (fG_BlendColors == NULL)
    return(false);

  if (PageControl1->ActivePage == tsBlender)
  {
    for (int ii = 0; ii < MAXBLENDCOLORS; ii++)
      SetBlendButtonColor(GetPointerToColorPanel(ii), fG_BlendColors);

    return(true);
  }

  return(false);
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::SetBlendButtonColor(TPanel* p, BlendColor* BC)
{
  bool Ret = false;

  try
  {
    if (p && BC != NULL)
    {
      p->Color = utils->BlendColorToTColor(BC[p->Tag]);

      // Show button selected or de-selected
      if (BlendButtonSelectIndex == p->Tag)
        p->BevelInner = bvLowered;
      else
        p->BevelInner = bvRaised;

      if (p->Color == clBlack)
        p->Font->Color = clWhite;
      else
        p->Font->Color = clBlack;

      if (BC[p->Tag].enabled)
        p->Caption = "";
      else
        p->Caption = "X";

      Ret = true;
    }
  }
  catch(...) {}

  return Ret;
}
//---------------------------------------------------------------------------
TPanel* __fastcall TDTSColor::GetPointerToColorPanel(int idx)
{
  // Returns the Panel who's Tag corresponds to idx
  try
  {
    for (int ii = 0; ii < tsBlender->ControlCount; ii++)
    {
      TControl* Temp = tsBlender->Controls[ii];

      if (Temp && Temp->Name.SubString(1, 5) == "Blend" && Temp->Tag == idx)
        return((TPanel*)Temp);
    }
  }
  catch(...) {}

  return NULL;
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::PaintPresetButtons(void)
// Call this when Blend Tab is selected
{
  if (PageControl1->ActivePage == tsBlender)
  {
    for (int ii = 0; ii < PageControl1->ActivePage->ControlCount; ii++)
    {
      try
      {
        // Pointer to each control on the "Blend" Tab
        TControl* P = dynamic_cast<TControl *>(PageControl1->ActivePage->Controls[ii]);

        // NOTE: PANEL NAMES MUST BE "Preset0 - Preset9"!
        // NO OTHER CONTROL ON THIS TAB CAN START WITH "Preset"!
        // Set the Tag to the index of the preset button

        if (P && P->Name == "ColorBlenderPresetPanel")
        {
          // Here's pointer to the presets panel
          TPanel* PresetPanel = (TPanel *)P;

          PresetPanel->Color = clFuchsia;

          BlendColor* DummyBC = new BlendColor[MAXBLENDCOLORS];

          for (int jj = 0; jj < PresetPanel->ControlCount; jj++)
          {
            // Has only the population of "preset" buttons
            TButton* Button = (TButton *)PresetPanel->Controls[jj];

            // Try to load the preset from the registry into a dummy array
            if (!LoadBlendPresetFromRegistry(DummyBC, Button->Tag))
              // paint this button "empty" (short hight)
              Button->Height = 7;
            else
            {
              // paint this button "full"
              Button->Height = 10;

              if (Button->Tag == fG_BlendPreset)
                // Focus on the current preset
                Button->SetFocus();
            }
          }

          delete [] DummyBC;
        }
      }
      catch(...)
      {
        continue;
      }
    }

    return true;
  }

  return false;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BlendDirectionRadioButtonsClick(TObject *Sender)
{
  switch(BlendDirectionRadioButtons->ItemIndex)
  {
    case 0: // Right-to-Left
    break;

    case 1: // Center
    break;

    case 2: // Left-to-Right
    break;

    default:
      // do nothing
    break;
  };
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BlendScopeRadioButtonsClick(TObject *Sender)
{
  switch(BlendScopeRadioButtons->ItemIndex)
  {
    case 0: // Word
    break;

    case 1: // Sentence
    break;

    case 2: // Paragraph
    break;

    case 3: // Entire Selection
    break;

    default:
      // do nothing
    break;
  };
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BlendColorPanelMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft)
    SetBlendBackgroundColor();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FgPanelMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft)
    SetFgColor();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BgPanelMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft)
    SetBgColor();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::WingColorPanelMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft)
    SetWingColor();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::WordWrapClick(TObject *Sender)
{
  if (WordWrap->Enabled)
  {
    WordWrap->Checked = !WordWrap->Checked;
    SetWordWrap(WordWrap->Checked);
    LoadView(tae->View); // reload
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SetWordWrap(bool bOn)
{
  utils->PushOnChange(tae); // freeze on-change text-entry

  // wwtPrinter another option FYI
  if (bOn && WordWrap->Enabled)
  {
    // "WARNING: You cannot edit any text in word-wrap mode!", // 49
    if (!bWordWrapWarningShown)
    {
      utils->ShowMessageU(DS[49]);
      bWordWrapWarningShown = true;
    }

    tae->WordWrapMode = wwtWindow;
    tae->WordWrap = true;
    tae->ReadOnly = true;
    WordWrap->Checked = true;
  }
  else if (tae->WordWrap != wwtNone)
  {
    tae->WordWrapMode = wwtNone;
    tae->WordWrap = false;
    tae->ReadOnly = false;
    WordWrap->Checked = false;
  }

  // For some reason the client needs to be re-aligned when the
  // wordwrap changes and no text is present
  tae->Align = alClient;
  Application->ProcessMessages();

  utils->PopOnChange(tae); // release on-change text-entry
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::InfoPanelMouseDown(TObject* Sender,
                        TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft)
    PrintShortLineCount();

/*
  if (Button == mbRight && tae->View == V_RTF)
  {
    if (PlayLineMaxBytes > PlayBufSize)
      InfoPanel->Font->Color = clRed;
    else
      InfoPanel->Font->Color = clBlue;

    // Print largest playback line width
    InfoPanel->Caption = String(DS[98]) + String(PlayLineMaxBytes);
  }
  else
    PrintShortLineCount();
*/
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ExportAsWebPage1Click(TObject *Sender)
// Convert IRC stream to HTML
{
#if LICENSE_ON
  if (PK->ComputeDaysRemaining() <= 0)
  {
    //"Visit www.yahcolorize.com to buy this product!"
    utils->ShowMessageU(KEYSTRINGS[4] +
                  Iftf->Strings[INFO_WEB_SITE] + KEYSTRINGS[5]);
    return;
  }
#endif

  // Create a TWebExportForm and then a TSFDlgForm then write the chosen file...
  GenerateHTML(true);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ViewInBrowser1Click(TObject *Sender)
{
  // No TWebExportForm is shown and no TSFDlgForm.
  // Generate HTML in temp-file for browser to read...
//  WideString fileName = GenerateHTML(false);
  String fileName = utils->WideToUtf8(GenerateHTML(false));

  if (!fileName.IsEmpty())
    ShellExecute(NULL, "open", fileName.c_str(), NULL, NULL, SW_SHOWNORMAL);
//    ShellExecuteW(NULL, L"open", fileName.c_bstr(), NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------
WideString __fastcall TDTSColor::GenerateHTML(bool bCreateDialog,
                                                    WideString FileName)
// Leave filename empty and ConvertToHTML will invoke its own filesave dialog.
// Set bCreateDialog and ConvertToHTML will ask user for web-page
// background-color, Etc. Returns the FileName used to write the HTML...
// (Used by File->ExportAsWebPage())
{
  WideString sRet;
  TConvertToHTML* c = NULL;

  CpInit(2); // Init progress-bar system

  try
  {
    // Create instance of object
    if ((c = new TConvertToHTML(this, FileName)) != NULL)
    {
      try
      {
        // Must optimize - old effects leave problematic codes -
        // but we don't want to optimize the main document
        // because we want to preserve "Undo" items
        //
        // (We only return 0 = success or 1 = cancel at present...)
        if (c->Convert(utils->Optimize(utils->MoveMainTextToString(),
                                                  false), bCreateDialog) == 0)
          sRet = c->FileName; // Read property
        else
          CpHide(true); // reset progress
      }
      catch(...) {}
    }
  }
  __finally
  {
    if (c)
      delete c;

    CpHide();
  }

  return sRet;
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::ReadSmileys(void)
{
  TStringsW* SL1 = NULL;
  TStringsW* SL2 = NULL;

  try
  {
    AutoDetectEmotes->Checked = false;
    WideString SmileyFile = utils->ExtractFilePathW(utils->GetExeNameW()) +
                                                           U16(SMILEYFILE);

//    String SmileyFile = (String)DeskDir + "\\" +
//          OUR_COMPANY_S + "\\" + OUR_NAME_S + "\\" + String(SMILEYFILE);

    if (!utils->FileExistsW(SmileyFile))
    {
      // "Cannot find... Please reinstall YahCoLoRiZe..."
      utils->ShowMessageU(DS[222] +
        String("\n") + SmileyFile + String("\n\n") + DS[223]);
      return false;
    }

    if (Emotes != NULL)
      delete Emotes;

    Emotes = new TStringsW();

    SL1 = new TStringsW();
    SL2 = new TStringsW();

    SL2->LoadFromFile(SmileyFile);

    // Clear blank lines and comments
    for (int ii = 0; ii < SL2->Count; ii++)
    {
      // Read a wide-string from our custom string-list, trim it and
      // reset it.
      SL2->SetString(utils->TrimW(SL2->GetString(ii)), ii);

      if (!SL2->GetString(ii).IsEmpty() && SL2->GetString(ii).Pos("//") != 1)
        SL1->Add(SL2->GetString(ii));
    }

    if (SL1->Count < SMILEY_SPARES || ((SL1->Count-SMILEY_SPARES) % 3))
      return false;

    // load emotes (add 1 since actual emote-code is second string
    // in a sequence of three strings per smiley:
    // image-file name, emot-code, emote-hint...
    for (int ii = SMILEY_SPARES+1; ii < SL1->Count; ii += 3)
      Emotes->Add(SL1->GetString(ii));

    AutoDetectEmotes->Checked = true;
  }
  __finally
  {
    if (SL1 != NULL) try {delete SL1;} catch (...) {}
    if (SL2 != NULL) try {delete SL2;} catch (...) {}
  }

  return true;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ExtendPaletteClick(TObject *Sender)
{
  int oldSize = paletteSize;

  paletteSize = ExtendPalette->Checked ? paletteExtent : MAXDEFAULTCOLORS;

  // if we went to a smaller palette... need to reset color panels
  if (paletteSize < oldSize)
  {
    RangeCheckColorPanels();
    UpdateColorButtons();
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::CopyIniFile(void)
// Force ini file-update if new format revision or file is missing
// in the AppData folder...
{
  TIniFile *pIni = NULL;

  try
  {
    try
    {
      // Look in the exe path first...
      WideString newIniPath = utils->ExtractFilePathW(utils->GetExeNameW()) +
                                                utils->Utf8ToWide(INIFILE);
      // utils->ShowMessageW(newIniPath);

      if (utils->FileExistsW(newIniPath))
      {
        pIni = utils->OpenIniFile(newIniPath);

        if (pIni == NULL)
          return;

        int newRev = pIni->ReadInteger(OUR_NAME_S, "revision", -1);

        if (newRev == -1)
          return;

        // ShowMessage(String(newRev));

        // get Users\(name)\AppData\Roaming\Discrete-Time Systems\YahCoLoRiZe path
        // (path MUST be created by the installer!)
        WideString oldIniPath = utils->GetSpecialFolder(CSIDL_APPDATA) +
          "\\" + utils->Utf8ToWide(OUR_COMPANY) + "\\" +
                utils->Utf8ToWide(OUR_NAME) + "\\";

// this won't work without a security descriptor, so must create path via installer!
//        if (CreateDirectoryW(oldIniPath, NULL) == 0)
//          ShowMessage("fail");

        oldIniPath += utils->Utf8ToWide(INIFILE);

        // utils->ShowMessageW(oldIniPath);

        if (utils->FileExistsW(oldIniPath))
        {
          delete pIni;
          pIni = utils->OpenIniFile(oldIniPath);

          int oldRev = pIni->ReadInteger(OUR_NAME_S, "revision", -1);

          // ShowMessage(String(oldRev));

          if (oldRev == -1 || newRev != oldRev)
            CopyFileW(newIniPath, oldIniPath, false); // bFailIfExists false
        }
        else
          CopyFileW(newIniPath, oldIniPath, false); // bFailIfExists false
      }

    }
    catch(const std::exception& e)
    {
      return;
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
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::ReadIniFile(void)
// Reads the colorize.ini file
{
  bool bRet;

  TIniFile *pIni = NULL;

  try
  {
    try
    {
      pIni = utils->OpenIniFile();

      if (pIni == NULL)
      {
        LoadPaletteFromString("", 0); // load default colors
        return false;
      }

      if (Iftf != NULL)
      {
        Iftf->Strings[INFO_WEB_SITE] =
                    pIni->ReadString("Website", "website", DEF_WEBSITE);

        Iftf->Strings[INFO_EMAIL] =
                    pIni->ReadString("Email", "email", DEF_EMAIL);

        Iftf->Strings[INFO_SONG_FORMAT] =
                    pIni->ReadString("SwiftMiX", "format", DEF_SONGFORMAT);
      }

      int count; // int passed by reference
      String S = ReadColorsFromIni(pIni, count);
      LoadPaletteFromString(S, count); // loads defaults if count == 0

      ReadPluginClassAndNameFromIni(pIni, count);

      // Add custom dictionary in colorize.ini to GDictList
      if (GDictList != NULL)
      {
        GDictList->Clear();

        // Can't directly load a sorted string-list using TIniFile()!
        // (have to create a temp-list and copy it)
        TStringList* tempsl = new TStringList();
        pIni->ReadSectionValues(INI_DICT_SECTION_NAME, tempsl);

        String sTemp;

        for (int ii = 0; ii < tempsl->Count; ii++)
        {
          sTemp = tempsl->Strings[ii];

          // Have to toss out the "D0000=" the hard way...
          int pos = sTemp.Pos("=");

          if (pos > 0)
            GDictList->Add(sTemp.SubString(pos+1, sTemp.Length()-pos));
        }

        delete tempsl;
        bMyDictChanged = false; // clear changed-flag
      }

      // Read PaltalkMaxColors property
      paltalkMaxColors =
        pIni->ReadInteger("Paltalk", "maxcolors", DEF_PALTALK_MAXCOLORS);

      bRet = true;
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
String __fastcall TDTSColor::ReadColorsFromIni(TIniFile* pIni, int &count)
{
  count = 0; // init referenced int

  if (pIni == NULL)
    return "";

  try
  {
    String sReadIni;

    if (pIni->SectionExists(INI_COLORS_SECTION_NAME))
    {
      sReadIni = pIni->ReadString(INI_COLORS_SECTION_NAME, "count", "");

      if (!sReadIni.IsEmpty())
      {
        count = sReadIni.ToIntDef(-1);

        if (count > 0)
        {
          // try the old-style of storage, colors=(16 space-separated 6-digit hex colors)
          if (count <= 16)
            sReadIni = pIni->ReadString(INI_COLORS_SECTION_NAME, "colors", "");
          else
            sReadIni = "";

          if (sReadIni.IsEmpty())
          {
            // read new-style of storage up to 100 lines of colors like:
            // colors0=ffffff 000000 3456cd
            // colors2=883311 112ACD
            // colors1=ffffff 000000 3456cd 98F2C8 034Bc8
            // (there must be no breaks between 0 and 99 but they can be in any order...)
            for (int ii = 0; ii < 100; ii++)
            {
              String sTemp = pIni->ReadString(INI_COLORS_SECTION_NAME,
                                                 "colors" + String(ii), "");

              if (sTemp.IsEmpty())
                break;

              if (ii > 0)
                sReadIni += ' ';

              sReadIni += sTemp;
            }
          }
        }
      }
    }
    return sReadIni;
  }
  catch(...) { return ""; }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::ReadPluginClassAndNameFromIni(TIniFile* pIni,
                                                                int &count)
{
  count = 0; // init referenced int

  if (pIni == NULL)
    return "";

  try
  {
    String sReadIni;

    this->pluginClass->Clear();
    this->pluginName->Clear();

    if (pIni->SectionExists(INI_CLIENTS_SECTION_NAME))
    {
      sReadIni = pIni->ReadString(INI_CLIENTS_SECTION_NAME, "count", "");

      if (!sReadIni.IsEmpty())
      {
        count = sReadIni.ToIntDef(-1);

        if (count > 0)
        {
          String sClass, sName;

          // read class0/name0...
          for (int ii = 0; ii < count; ii++)
          {
            sClass = pIni->ReadString(INI_CLIENTS_SECTION_NAME,
                                                  "class" + String(ii), "");
            sName = pIni->ReadString(INI_CLIENTS_SECTION_NAME,
                                                  "name" + String(ii), "");

            if (sClass.IsEmpty() || sName.IsEmpty())
              break;

            // add to our stringlist properties
            this->pluginClass->Add(sClass);
            this->pluginName->Add(sName);
          }
        }
      }
    }

    return true;
  }
  catch(...) { return false; }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::WriteDictToIniFile(void)
// Writes GDict strings to the colorize.ini file
{
  if (GDictList == NULL) return false;
  if (!bMyDictChanged) return true; // ok

  bool bRet;

  TIniFile *pIni = NULL;

  try
  {
    try
    {
      pIni = utils->OpenIniFile();

      if (pIni == NULL)
        return false;

      // Write custom dictionary in GDictList to colorize.ini
      // (0000 to 9999 is 10000 words max)
      pIni->EraseSection(INI_DICT_SECTION_NAME);
      for (int ii = 0; ii < GDictList->Count && ii < MAX_DICT_COUNT; ii++)
        pIni->WriteString(INI_DICT_SECTION_NAME, "D" +
                    utils->GoFormat("%.4d", ii), GDictList->Strings[ii]);

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
void __fastcall TDTSColor::LoadPaletteFromString(String S, int count)
// Creates/Recreates and loads Palette from a string of rrggbb rrggbb values
// and sets paletteSize
{
  bool bRet = true;
  int* c = NULL;

  if (count > 0)
  {
    int ii;
    c = new int[count];

    // Check for valid color-string
    try
    {
      char* p = S.c_str();

      for (ii = 0; ii < count; ii++)
      {
        int ret = sscanf(p, "%06x", &c[ii]);
        if (ret != 1) break;
        p += 7; // point to next
      }
    }
    catch(...) { bRet = false; } // error

    if (ii != count) bRet = false; // error
  }
  else bRet = false; // error

  // Delete old palette
  if (palette != NULL) try { delete [] palette; } catch(...) {}

  try
  {
    // load new color-palette - carefully written to load the ini colors
    // (if any) and to fill in up to MAXDEFAULTCOLORS (16) if only a few colors
    // were read from the ini file...
    int ii = 0;
    int size = (count < MAXDEFAULTCOLORS || !bRet) ? MAXDEFAULTCOLORS : count;

    palette = new int[size]; // create new palette

    // load colors read from ini file
    if (bRet) for (; ii < count; ii++)
      palette[ii] = c[ii];

    if (c != NULL)
      delete [] c; // through with ini colors now...

    // if < 16 colors, fill from default table
    for (; ii < size; ii++)
      palette[ii] = DefaultPalette[ii];

    // The extent is the # color actually "there" in the table
    paletteExtent = size;
  }
  catch (...)
  {
    paletteExtent = 0;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::RepaintTimeout(TObject *Sender)
{
  RepaintTimer->Enabled = false;

  #define RNG 200

  // if width too small stop painting
  bool bNoPaint = (bIsVistaOrHigher && this->Width < 650) ||
                                (!bIsVistaOrHigher && this->Width < 450);

  if (bNoPaint || palette == NULL || Iftf == NULL)
    return;

  static BlendColor FGColor, BGColor;

  if (!bFreeze)
  {
    // try 16 times,don't hang up program!
    for (int ii = 0; ii < paletteSize; ii++)
    {
      if (IsYahTrinPal())
      {
        if (bRgbRandFG)
        {
          captionFG = -(utils->GetRandomRGB());
          FGColor = utils->RgbToBlendColor(-captionFG);
        }
        else
        {
          captionFG = random(paletteSize)+1;
          FGColor = utils->RgbToBlendColor(palette[captionFG-1]);
        }

        if (bRgbRandBG)
        {
          captionBG = -(utils->GetRandomRGB());
          BGColor = utils->RgbToBlendColor(-captionBG);
        }
        else
        {
          captionBG = random(paletteSize)+1;
          BGColor = utils->RgbToBlendColor(palette[captionBG-1]);
        }
      }
      else
      {
        captionFG = random(paletteSize)+1;
        FGColor = utils->RgbToBlendColor(palette[captionFG-1]);
        captionBG = random(paletteSize)+1;
        BGColor = utils->RgbToBlendColor(palette[captionBG-1]);
      }

      if (abs(FGColor.red-BGColor.red) >= RNG || abs(FGColor.green-BGColor.green) >= RNG || abs(FGColor.blue-BGColor.blue) >= RNG)
        break;
    }
  }

  WMNCPaint(utils->BlendColorToTColor(FGColor),  utils->BlendColorToTColor(BGColor));
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::WMNCPaint(TColor fg, TColor bg)
{
  TCanvas* ACanvas = new TCanvas();

  try
  {
    ACanvas->Handle = GetWindowDC(Handle);
    ACanvas->Brush->Color = bg;
    ACanvas->Font->Name = "FixedSys";
    ACanvas->Font->Size = 8;
    ACanvas->Font->Color = fg;
    ACanvas->Font->Style.Clear();
    ACanvas->Font->Style << fsBold << fsUnderline;
    ACanvas->Font->Pitch = fpFixed;
    ACanvas->TextFlags = ETO_OPAQUE;

    POINT p;
    String s;

    if (bIsVistaOrHigher)
    {
      s = String(DS[211]); // "Right-click Me!"

      p.x = Width - ACanvas->TextWidth(s) - GetSystemMetrics(SM_CXBORDER) - 10;
      p.y = GetSystemMetrics(SM_CYSIZEFRAME)+GetSystemMetrics(SM_CYCAPTION)+
              GetSystemMetrics(SM_CYBORDER)+1;
    }
    else
    {
      s = Iftf->Strings[INFO_WEB_SITE];

      p.x = (Width - ACanvas->TextWidth(s))/2;
      p.y = (GetSystemMetrics(SM_CYCAPTION)+GetSystemMetrics(SM_CYBORDER)-
              GetSystemMetrics(SM_CYMENU)-ACanvas->Font->Height)/2;
    }

    TRect rText;
    rText.Left = p.x;
    rText.Top = p.y;
    rText.Right = p.x + ACanvas->TextWidth(s);
    rText.Bottom = p.y + ACanvas->TextHeight(s);

    ACanvas->TextOut(p.x, p.y, s);
    ACanvas->Brush->Color = fg;
    ACanvas->FrameRect(rText); // draw a border

    ReleaseDC(Handle, ACanvas->Handle);
  }
  __finally { delete ACanvas; }
}
//---------------------------------------------------------------------------
//bool __fastcall TDTSColor::IsWinXPOrHigher(void)
//{
//  OSVERSIONINFO vi;
//  vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
//  GetVersionEx(&vi);
//
//  return (vi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
//         ((vi.dwMajorVersion > 5) || ((vi.dwMajorVersion == 5) &&
//                                  (vi.dwMinorVersion >= 1)));
//}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::IsWinVistaOrHigher(void)
{
  OSVERSIONINFO vi;
  vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&vi);

  return vi.dwPlatformId == VER_PLATFORM_WIN32_NT && vi.dwMajorVersion >= 6;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuCharacterMapClick(TObject *Sender)
{
  // Launch Character Map
  ShellExecute(Handle, "open", "charmap.exe", NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuOptimizeClick(TObject *Sender)
{
  if (TOCUndo)
  {
    // All undos are garbage if we optimize! (So ask...)
    if (TOCUndo->Count > 0)
    {
      if (utils->ShowMessageU(Handle, DS[50],
                    MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1) == IDNO)
        return;
    }

    // When we optimize we can no longer rely on the undo-list's
    // saved indices!
    TOCUndo->Clear(); // Clear undo buffer
  }

  // Save original position and carat
  TTaePosition* p = new TTaePosition(tae);
  p->SavePos = p->Position;
  utils->Optimize(true);
  p->Position = p->SavePos;
  delete p;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ddetextPokeData(TObject *Sender)
// DDE Server.  Service Name: (same as executable name) Colorize
//              Topic: (same as window-name) yahcolorize
//              Item: data
//
// for mIRC:
// /cchan /dde colorize command ddechan *1
// /cx /dde colorize command ddetext *1
// /cx /dde colorize command ddetext $chan *1
//
// for Vortec:
// /CCHAN /ddepoke colorize command ddechan *1
// /CX ddepoke colorize command ddevtext *1
// /CX ddepoke colorize command ddevtext $activewin *1
//
// those aliases poke data to YahCoLoRiZe here!
//
// I tried sending the channel or nickname but it's not doable without
// a simple script on the mIRC or Vortec side... mIRC uses $chan which
// is empty if you send from the server-window and then you need $nick if
// it's a private conversation. Vortec uses $activewin which won't have
// a '#&~' prefix for a server-window or private chat... so you cannot
// parse out all permutations. If we said the 1st word is always to be
// parsed off, you break old user's chat-setup... so the best way is
// for mirc/vortec to have a script that sends us the "set channel"
// command first... then sends this hook's text...
//
// These are the permanent client-side IDs - only Trinity can be changed!
// 0=anyclient,1=yahelite,2=mirc,3=icechat,4=hydrairc,
// 5=leafchat,6=vortec,7=xircon,8=paltalk,9=trinity
{
  if (!PlayOnCX->Checked || bIsPlaying || wmCopyMode != CM_IDLE) return;

  try
  {
    TDdeServerItem* pDDE = (TDdeServerItem*)Sender;

    if (pDDE && pDDE->Text.Length())
    {
      if (pDDE->Name.LowerCase() == "ddevtext")
        wmCopyClient = 6; // Vortec
      else
        wmCopyClient = 2; // mIRC

      WmCopyStr = pDDE->Text;
      pDDE->Text = ""; //Clear
      wmCopyMode = CM_PROCESS_RX_DATA;
      wmCopyData = -1;
      WmChanNickStr = ""; // don't add a room! (we send a ddechanpokedata)
      Timer2->Interval = TIMER2TIME;
      Timer2->Enabled = true; // Go process and send the line
    }
  }
  catch(...){}
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ddechanPokeData(TObject *Sender)
{
  // DO NOT call this via Timer2! - reason is that we miss a command
  // when back-to-back channel-change then text to be processed.
  // We don't have a command queue for Timer2 commands!
  if (PlayOnCX->Checked && ddechan->Text.Length() > 0)
  {
    String sOrig = ddechan->Text.Trim();

    if (sOrig.Length() <= 0)
      return;

    String sRoom;

    if (bSendUtf8)
      sRoom = sOrig;
    else
      sRoom = utils->AnsiToUtf8(sOrig);

    sRoom = utils->StripTrailingCRLFsA(sRoom);

    if (sRoom.IsEmpty())
      sRoom = STATUSCHAN; // "status"

    AddRoom(sRoom);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ddeplayPokeData(TObject *Sender)
{
  // DO NOT call this via Timer2! - reason is that we miss a command
  // when back-to-back channel-change then text to be processed.
  // We don't have a command queue for Timer2 commands!
  if (PlayOnCX->Checked)
  {
    String sOrig = ddeplay->Text.Trim();

    if (sOrig.Length() <= 0)
      return;

    String sCmd;

    if (bSendUtf8)
      sCmd = sOrig;
    else
      sCmd = utils->AnsiToUtf8(sOrig);

    sCmd = utils->StripTrailingCRLFsA(sCmd); // keep this in case it's a file-name! (see below)

    String lcCmd = sCmd.LowerCase();

    if (lcCmd == "stop")
      StopPlayClick(NULL);
    else if (lcCmd == "pause")
      PausePlayClick(NULL);
    else if (lcCmd == "resume")
      ResumePlayClick(NULL);
    else if (lcCmd == "start")
      StartPlayClick(NULL);
    else
    {
      // file-name (possibly...)
      WideString wCmd = utils->Utf8ToWide(sCmd);

      bIsAnsiFile = false; // "Assume" a UTF-8 file...

      if (utils->FileExistsW(wCmd) && LoadLines(wCmd, true, true) && tae->Lines->Count)
      {
        DoProcess(true);
        StartPlayClick(NULL);
      }
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::WMDropFile(TWMDropFiles &Msg)
{
  try
  {
    //get dropped files count
    int cnt = ::DragQueryFileW((HDROP)Msg.Drop, -1, NULL, 0);

    if (cnt != 1)
      return; // only one file!

    wchar_t wBuf[MAX_PATH];

    // Get first file-name
    if (::DragQueryFileW((HDROP)Msg.Drop, 0, wBuf, MAX_PATH) > 0)
    {
      // Load and convert file as per the file-type (either plain or rich text)
      WideString wFile(wBuf);

      if (!wFile.IsEmpty())
      {
        bIsAnsiFile = false; // "Assume" a UTF-8 file...
        OpenFileSetDefaultDirs(wFile);
      }
    }
  }
  catch(...){}
}
//---------------------------------------------------------------------------
PASTESTRUCT __fastcall TDTSColor::TaeEditCutCopy(bool bCut, bool bCopy)
// Returns the +/- change in the edit-control's length for the present
// view.
{
  PASTESTRUCT ps; // this DOES call the default constructor BTW!

  int selLen = tae->SelLength;

  if (!selLen)
  {
    // "You must select text to perform this operation!", //25
    utils->ShowMessageU(String(EDITMSG[25]));
    return ps;
  }

  if (!utils->IsRtfIrcOrgView())
  {
    try
    {
      WideString wSelText = tae->SelTextW;

      if (bCopy)
        utils->CopyTextToClipboard(wSelText);

      if (bCut)
      {
        utils->PushOnChange(tae);

        // # of cr/lf pairs in the selected text
        ps.lines = utils->CountCRs(wSelText);

        // ps.delta is the number of chars cut including
        // each cr/lf as a count of 2
        ps.delta = -tae->SelLength; // negative # chars cut

        // Populate ONCHANGE thread's struct (ProcessOnChange.h) for Undo
        // (Get the deltas in length and line-count!)
        ONCHANGEW oc = utils->GetInfoOC(tae, NULL);
        // Add to undo-objects
        TOCUndo->Add(UNDO_CUT, tae->SelStart, 0, oc, wSelText);

        tae->SelTextW = ""; // Cut the selected text

        tae->Modified = true;
        bSaveWork = true;
        utils->SetOldLineVars(tae); // Update OldLength and OldLineCount
        PrintShortLineCount();
        utils->PopOnChange(tae);
      }
    }
    catch(...) { ps.error = -10; }

    return ps;
  }

  TStringsW* sl;

  if (utils->IsOrgView())
    sl = SL_ORG;
  else
    sl = SL_IRC;

  if (utils->IsRtfView())
    // Erases clipboard then copies Html, Rtf and Utf-16 text formats to it
    ps = utils->CutCopyRTF(bCut, bCopy, tae, sl);
  else
    // Erases clipboard then copies Utf-16 text format to it
    ps = utils->CutCopyIRC(bCut, bCopy, tae, sl);

  if (ps.error == 0) // ps.lines serves as an error-code if negative!
  {
    if (bCut)
    {
      utils->SetOldLineVars(tae);

      if (tae->TextLength == 0 || sl->TotalLength == 0)
      {
        // Switch view to V_OFF and clear streams
        if (tae->View == V_ORG)
          ClearAndFocus(true, false);
        else
          ClearAndFocus(true, true); // Keep SL_ORG and switch to V_OFF
      }

      tae->Modified = true;
      bSaveWork = true;
      PrintShortLineCount();
    }

    bTextWasProcessed = false;

    return ps;
  }

  utils->ShowMessageU("TaeEditCutCopy(), Can't cut/copy text,\n"
                          "text selected? Code: " + String(ps.error));

  return ps;
}
//----------------------------------------------------------------------------
void __fastcall TDTSColor::EditPaste(TObject *Sender)
{
#if DIAG_SHOWHEX_IS_UTF8
  // diagnostic to paste raw ANSI text after clicking Tools->ShowHex
  // to convert a block of UTF-8 text to ANSI codes for DefaultStrings.cpp!
  tae->Text = Clipboard()->AsText;
#else
  PASTESTRUCT ps = TaeEditPaste(false);

  if (ps.error == 0)
    tae->SelStart += ps.delta-ps.lines;

  PrintShortLineCount();
#endif
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EditPasteColor(TObject *Sender)
{
  try
  {
    utils->PushOnChange(tae);

    // Save original position and carat
    TTaePosition* p = new TTaePosition(tae);
    p->SavePos = p->Position;

    // Init PASTESTRUCT (receives info we need in order to set
    // the caret to end of the paste zone)
    PASTESTRUCT ps;
    ps.Clear();

    if (tae->View == V_RTF_SOURCE)
    {
      // I added this to debug paltalk's RTF format - it will paste raw RTF
      // formatted text, say from Wordpad, into a V_RTF_Source view...
      // IsClipboardFormatAvailable(cbRTF);
      if (Clipboard()->HasFormat(cbRTF))
      {
        try
        {
          Clipboard()->Open();
          int TextHandle = Clipboard()->GetAsHandle(cbRTF);
          char* pText = (char *)GlobalLock((HGLOBAL)TextHandle);
          String S = String(pText);
          GlobalUnlock((HGLOBAL)TextHandle);

          ps.delta = S.Length(); // Save paste-length

          if (ps.delta)
          {
            ps.lines = utils->CountCRs(S); // Count cr/lfs

            // SelLength needed to Undo the paste directly
            // in the control later!
            int len = S.Length() - ps.lines;

            // Add undo info (don't need the paste-string saved...)
            ONCHANGEW oc = utils->GetInfoOC(tae, NULL);
            TOCUndo->Add(UNDO_PASTE, tae->SelStart, len, oc, "");

            tae->SelLength = 0; // deselect if selected...
            tae->SelTextW = S; // paste text at caret
          }
        }
        __finally
        {
          try {Clipboard()->Close();} catch(...){}
        }
      }
      else
        ps = TaeEditPaste(true);
    }
    else if (tae->View == V_HTML)
    {
      try
      {
        // This allows you to paste HTML source into the HTML view...
        if (Clipboard()->HasFormat(cbHTML))
        {
          WideString S = utils->Utf8ToWide(utils->LoadHtmlFromClipboard());

          int len = S.Length();

          if (len)
          {
            // SelLength needed to Undo the paste directly
            // in the control later!
            int numCRs = utils->CountCRs(S);
            len = len - numCRs;

            // Add undo info (don't need the paste-string saved...)
            ONCHANGEW oc = utils->GetInfoOC(tae, NULL);
            TOCUndo->Add(UNDO_PASTE, tae->SelStart, len, oc, "");

            tae->SelTextW = S; // Replace selected text or insert at caret...

            ps.lines = numCRs; // Number of cr/lfs
            ps.delta = len;
          }
        }
        else
          ps = TaeEditPaste(true);
      }
      catch(...) {}
    }
    else if (Clipboard()->HasFormat(cbHTML))
    {
      int oldLen = tae->TextLength;

      // NOTE: a large quantity of HTML on the clipboard takes FOREVER
      // to convert! (so use the plain-text if it's in our color-format...)
      if (utils->TextContainsFormatCodes(utils->GetClipboardText()))
        ps = TaeEditPaste(true);
      else
        // Go convert the raw html on clipboard from utf-8 to utf-16 and then
        // convert it to IRC format...
        ps = TaeEditPaste(true, utils->ClipboardHtmlToIrc(true));

      // Convert to RTF if we have IRC codes and the edit-control
      // was empty before the paste...
      if (oldLen == 0 && utils->TextContainsFormatCodes(SL_ORG))
      {
        tae->View = V_ORG; // View must be V_ORG to copy SL_ORG to SL_IRC!
        DoShowRtf(true); // Convert to RTF
      }
    }
    else
      ps = TaeEditPaste(true);

    // Restore position and caret
    p->Position = p->SavePos;
    delete p;

    // Point caret to end of newly pasted text
    if (ps.delta > 0)
      tae->SelStart += ps.delta - ps.lines;

    utils->PopOnChange(tae);
  }
  catch(...)
  {
    utils->ShowMessageU("Error in EditPasteColor()...");
  }

  PrintShortLineCount();
}
//---------------------------------------------------------------------------
// Overloaded...
PASTESTRUCT __fastcall TDTSColor::TaeEditPaste(bool bColor)
{
  PASTESTRUCT ps;
  ps.Clear();

  try
  {
    WideString wOut = utils->GetClipboardText();

    if (!wOut.IsEmpty())
      ps = TaeEditPaste(bColor, wOut);
  }
  catch(...)
  {
#if DEBUG_ON
    CWrite("\r\nError in TaeEditPaste(): -1\r\n");
#endif
    ps.error = -1;
  }

  return ps;
}
//---------------------------------------------------------------------------
// Overloaded...
PASTESTRUCT __fastcall TDTSColor::TaeEditPaste(bool bColor, WideString wTemp)
// bColor is set false when pasting TAB spaces or when the user
// right-clicks and selects Paste (non-color)
//
// Returns the +/- change in the edit-control's length including each line-
// terminator as a count of 2 and the # line-terminators
// in the pasted text
{
  PASTESTRUCT ps;
  ps.Clear();

  bool bChain = false; // Flag to tell us to chain the paste Undo item...

  // Replacing text? (do this first because we could be replacing a string
  // with "nothing" wTemp CAN be empty!)
  if (tae->SelLength)
  {
    utils->PushOnChange(tae);

    try
    {
      ps = TaeEditCutCopy(true, false); // Cut old text

      if (ps.error != 0)
      {
#if DEBUG_ON
        CWrite("\r\nError [3] in TaeEditPaste(bool, string)...\r\n");
#endif
        utils->PopOnChange(tae);
        ps.error = -3;
        return ps;
      }
    }
    catch(...)
    {
#if DEBUG_ON
      CWrite("\r\nError [4] in TaeEditPaste(bool, string)...\r\n");
#endif
      utils->PopOnChange(tae);
      ps.error = -4;
      return ps;
    }

    bChain = true; // Flag to chain a paste Undo item (below)

    utils->PopOnChange(tae);
  }

  if (wTemp.IsEmpty())
    return ps;

  try
  {
    if (!bColor)
    {
      wTemp = utils->StripAllCodes(wTemp);

      if (wTemp.Length() == 0)
        return ps;
    }
    else if (!utils->TextContainsFormatCodes(wTemp))
      bColor = false; // don't need to do a DoShowRtf() (no flicker!)
  }
  catch(...)
  {
#if DEBUG_ON
    CWrite("\r\nError [1] in TaeEditPaste(bool, string)...\r\n");
#endif
    ps.error = -1;
    return ps;
  }

  // Have text being pasted and view is off, go to ORG view
  // to allow highlights and edits...
  if (tae->View == V_OFF)
  {
    utils->PushOnChange(tae);

    try
    {
      // Paste in SL_ORG to allow editing...
      SL_ORG->Text = wTemp;
      tae->View = LoadView(V_ORG);

      // Add undo info (don't need the paste-string saved...)
      ONCHANGEW oc = utils->GetInfoOC(tae, SL_ORG);
      TOCUndo->Add(UNDO_INS, 0, wTemp.Length(), oc, "", bChain);
//      TOCUndo->Add(UNDO_PASTE, 0, wTemp.Length(), oc, "", bChain);

      // Prevent autoset of line-width in RTF mode!
      bTextWasProcessed = false;
      ps.lines = utils->CountCRs(wTemp);
      ps.delta = wTemp.Length();
    }
    catch(...)
    {
#if DEBUG_ON
      CWrite( "\r\nError [2] in TaeEditPaste(bool, string)...\r\n");
#endif
      ps.error = -2;
    }

    utils->PopOnChange(tae);
    return ps;
  }

  if (tae->View != V_RTF)
  {
    utils->PushOnChange(tae);

    try
    {
      if (tae->View == V_IRC || tae->View == V_ORG)
      {
        // Future improvement...
        // we need to mod the highlight routine to take/return a string
        // then insert that string into TaeEdit as RTF - so a version of the
        // converter that just makes a { ;} string, no color table, etc.
        // Then insert that - plus insert just the paste-text
        // into the irc-stream via a string-list
        //
        // OR - we have to re convert the whole IRC-stream...

        TStringsW* TaeSL;

        if (tae->View == V_ORG)
        {
          TaeSL = SL_ORG;
          bTextWasProcessed = false; // Prevent autoset of line-width in RTF mode!
        }
        else
          TaeSL = SL_IRC;

        TPoint p = tae->CaretPos;

        // get only the raw-codes line the caret is on - the line-index is the
        // same in both the RTF view and in the SL_IRC string-list...
        WideString wS = TaeSL->GetString(p.y);

        // Insert new paste-text into a string cantaining the line the
        // caret is on
        wS = utils->InsertW(wS, wTemp, p.x+1);

        // Insert new text (including line-breaks) into string-list
        TPoint ptDelta = TaeSL->SetStringEx(wS, p.y);

        // Add undo point
        if (TOCUndo != NULL)
        {
          ONCHANGEW oc = utils->GetInfoOC();
          TOCUndo->Add(UNDO_INS, p.x, ptDelta.x, oc, "");
        }

        // convert the whole list into highlighted text
        //
        // TODO: make this more efficient by only encoding the section we
        // changed - ConvertToRtf also needs to be able to convert and insert
        // just the changed text so we don't have to re-process the entire
        // document...
        utils->EncodeHighlight(TaeSL, tae, false);
      }
      else // other view...
      {
        // SelLength needed to Undo the paste directly in the control later!
        int len = wTemp.Length() - utils->CountCRs(wTemp);

        // Add undo info (don't need the paste-string saved...)
        ONCHANGEW oc = utils->GetInfoOC(tae, NULL);
        TOCUndo->Add(UNDO_INS, tae->SelStart, len, oc, "", bChain);

        tae->SelTextW = wTemp;
      }

      ps.lines += utils->CountCRs(wTemp); // Count CRs
      ps.delta += wTemp.Length(); // includes 2 chars for each cr/lf!

      // Reset for OnChange Event...
      utils->SetOldLineVars(tae);
    }
    catch(...)
    {
#if DEBUG_ON
      CWrite("\r\nError [5] in TaeEditPaste(bool, string)...\r\n");
#endif
      ps.error = -5;
    }

    utils->PopOnChange(tae);
    return ps;
  }

  // tae->View is V_RTF...

  bool bShowStatus = (wTemp.Length() > STATUSLENGTH) ? true : false;

  if (bShowStatus)
    CpShow(STATUS[11]); // Pasting text

  try
  {
    try
    {
      // CR or LF paste...
      if (wTemp == C_CR || wTemp == C_LF)
        wTemp = CRLF;

      // cr/lf we need to avoid the phantom colored space phenomenon...
      if (wTemp == WideString(CRLF))
        tae->SelAttributes->BackColor = clWindow;

      utils->PushOnChange(tae);

      try
      {
        TPoint p = tae->CaretPos;

        // get only the raw-codes line the caret is on - the line-index is the
        // same in both the RTF view and in the SL_IRC string-list...
        WideString wS = SL_IRC->GetString(p.y);

        // Get text states settled (wS is in/out by reference!)
        int idxIns = utils->ResolveStateForPaste(p, wS, wTemp);

        if (idxIns >= 0)
        {
          // Insert new text (including line-breaks) into string-list
          TPoint ptDelta = SL_IRC->SetStringEx(wS, p.y);

          // Add undo point
          if (TOCUndo != NULL)
          {
            ONCHANGEW oc = utils->GetInfoOC(tae, SL_IRC);
            TOCUndo->Add(UNDO_INS, idxIns, ptDelta.x, oc, "");
          }

          // Now paste the text into our rich-text edit-control and adjust
          // the cursor...

          // Set vars used to compute new caret position after paste
          // (the calling function will set it to the end of the paste-text!)
          WideString Stripped = utils->StripAllCodes(wTemp);
          ps.lines += utils->CountCRs(Stripped);
          ps.delta += Stripped.Length(); // length added to RTF text-view

          if (!bColor) // this is used in pasting a string of TAB spaces!
          {
            // Setting SelText automatically advances the caret, which
            // we don't want... since the calling function will adjust
            // SelStart using ps.lines and ps.delta...
            int SaveSelStart = tae->SelStart;
            tae->SelTextW = Stripped;
            tae->SelStart = SaveSelStart;
          }
          else // Only need this if new text contains any format codes
            DoShowRtf(bShowStatus);

          // Reset for OnChange Event...
          utils->SetOldLineVars(tae);
          tae->Modified = true;
          bSaveWork = true;
        }
        else
        {
  #if DEBUG_ON
          CWrite("\r\nError [7] in TaeEditPaste(bool, string)...\r\n");
  #endif
          ps.error = -7;
        }
      }
      catch(...)
      {
  #if DEBUG_ON
        CWrite("\r\nError [8] in TaeEditPaste(bool, string)...\r\n");
  #endif
        ps.error = -8;
      }

      utils->PopOnChange(tae);
    }
    catch(...)
    {
      // Reset for OnChange Event...
      utils->SetOldLineVars(tae);
      tae->Modified = true;
      bSaveWork = true;
      utils->PopOnChange(tae);
  #if DEBUG_ON
      CWrite("\r\nError [9] in TaeEditPaste(bool, string)...\r\n");
  #endif
      ps.error = -9;
      return ps;
    }
  }
  __finally
  {
    CpHide();
  }

  return ps;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PackTextClick(TObject *Sender)
{
  // If user specifically turns on PackText while
  // processed text is in the view, prevent AutoWidth
  // from being applied to the re-rendering.
  if (PackText->Checked)
    bTextWasProcessed = false;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::UpdateViewMenuState(void)
{
  // Set menu Enable/Disable
  if (SL_ORG->Count > 0)
  {
    Original1->Enabled = true;
    PopupOriginal->Enabled = true;
  }
  else
  {
    Original1->Enabled = false;
    PopupOriginal->Enabled = false;
  }

  // Leave this on so we can show the rtf source even if no MS_RTF text...
  if (MS_RTF->Size > 0 || SL_IRC->Count > 0 || tae->TextLength)
    RTFSource1->Enabled = true;
  else
    RTFSource1->Enabled = false;

  if (SL_IRC->Count > 0)
  {
    IRC1->Enabled = true;
    RTF1->Enabled = true;
    HTML1->Enabled = true;
    ViewInBrowser1->Enabled = true;
  }
  else
  {
    IRC1->Enabled = false;
    RTF1->Enabled = false;
    HTML1->Enabled = false;
    ViewInBrowser1->Enabled = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::LeftJustifyClick(TObject *Sender)
{
  // If user specifically turns on LeftJustify while
  // processed text is in the view, prevent AutoWidth
  // from being applied to the re-rendering.
  if (LeftJustify->Checked) bTextWasProcessed = false;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EffectsMenuClick(TObject *Sender)
{
  if (tae->View == V_RTF)
  {
    PlainText1->Enabled = false;
    Pop1->Enabled = false;
  }
  else
  {
    PlainText1->Enabled = true;
    Pop1->Enabled = true;
  }
}
//---------------------------------------------------------------------------
// maxSegments divides the main progress bar into segments. The progress bar
// on the CancelPanel represents 0-100% of the current segment's progress.
// So maxSegments would be set to 3 if I had 3  sequential operations to do
// with the main progress bar (at the bottom) keeping the cumlative progress
// of each successive operation plus the current operation.
//
// Set maxSegments 0 to turn off the main progress bar .
//
// To use:
// 1) Call CpInit() and pass in the max number of progress bar segments
// expected.
// 2) Call CpShow() when you want a status message displayed.
// 3) Call CpSetMaxIterations() to set each loop-count just before the loop.
// 4) Call CpUpdate() inside a loop and pass it the iterator.
// 5) Call CpHide() in a __finally block at the end of a method containing a
// loop.
//
// The basic idea is to initialize the progress-bar system by passing
// the number of "loops" we expect to encounter as a complex set of actions
// are processed. The progress bar is effectively divided into that number
// of segments. FCpTotalSections holds this number.
//
// CpShow() can be called to show a message. You should still call CpInit(0)
// if there are no loops associated with the message so that CpHide()
// will close the message panel. (You could call CpShow() then CpReset()
// to close it without calling CpInit())
//
// We call CpSetMaxIterations() before each loop to set the reference for
// computing the progress bar's position within the current segment. The trick
// here is that FCpCurrentSection is pre-incremented up to FCpTotalSections.
// That's what advances the virtual "segment" in the main progress-bar.
//
// When we call CpHide() the progress bar is not reset and the message panel
// is not closed until FCpCurrentSection reaches FCpTotalSections. We then
// leave these vars as they are until reset by CpInit(). That allows
// CpSetMaxIterations() to detect when there are more loops in this sequence
// of operations than we accounted for and that we must increase the number
// passed to CpInit().
//
// I also ran into a problem when a sequence of operations' root method (the
// method that calls CpInit()) has to call another method that is in itself
// a root method (calls CpInit() also...). So my simple solution is to check
// in CpInit to see if FCpCurrentSection == FCpTotalSections. If so we can
// do a full initialization, if not, then this CpInit() is nested inside
// the scope of another and we want to simply add the additional "loops"
// passed in to FCpTotalSections... Brilliant!
void __fastcall TDTSColor::CpInit(int totalProgressSections)
{
  if (FCpTotalSections == FCpCurrentSection)
  {
    FCpTotalSections = totalProgressSections;
//#if DEBUG_ON
//    CWrite("\r\nCpInit::FCpTotalSections: " +
//     String(FCpTotalSections) + "\r\n");
//#endif

    // CpSetMaxIterations pre-increments this to 1. Then it goes up to
    // FCpTotalSections at which point calling CpHide closes the status-panel
    // and resets the progress-bars.
    //
    // The only way to clear this is here or with CpReset() - which is
    // a bit scary. What if things get out of whack in some process and we
    // return prematurely and FCpCurrentSection never increments all the
    // way up to FCpTotalSections? In that case, the ONLY way to reset
    // is to have a failsafe call to CpReset() in the __finally of EVERY
    // method that calls CpInit()! But you can't do that in the nested CpInit()'s
    // method or it wipes out your progress...
    //
    // Need to comment out all the failsafes while we debug to get the number
    // to pass to CpInit() - then turn them all back on... use DEBUG_ON switch!
    FCpCurrentSection = 0;
  }
  else
  {
//#if DEBUG_ON
//    CWrite("\r\nCpInit::FCpTotalSections: " + String(FCpTotalSections) +
//       " adding: " + String(totalProgressSections) + "\r\n");
//#endif
    FCpTotalSections += totalProgressSections;
  }

  CancelPanel->Visible = false;
  MainProgressBar->Position = 0;
  MainProgressBar->Visible = true;
  CpProgressBar->Visible = false;
  CpProgressBar->Position = 0;
  CpMsg->Caption = "";
  CpExtra->Caption = "";
  FCpMaxIterations = 0; // loop-count
}
//---------------------------------------------------------------------------
// Set sMsg empty ("") to turn off the status panel.
//
// This sets FCpMaxIterations 0 so if you have a loop after calling this
// be sure to set its max iterations via CpSetMaxIterations()!
void __fastcall TDTSColor::CpShow(String sMsg, String sAuxMsg)
{
  // Change processing status panel...
  if (!sMsg.IsEmpty())
  {
    CpMsg->Caption = sMsg;

    if (!CancelPanel->Visible)
      CancelPanel->Visible = true;
  }
  else if (CancelPanel->Visible)
    CancelPanel->Visible = false;

  if (!sAuxMsg.IsEmpty())
    CpExtra->Caption = sAuxMsg;

  Application->ProcessMessages();
}
//---------------------------------------------------------------------------
// Force reset
void __fastcall TDTSColor::CpReset(void)
{
  CpHide(true);
}
//---------------------------------------------------------------------------
// Reset the progress bars and hide the status panel when the last CpHide()
// is called (FCurrentSection == FMaxSections). Also reset on any CpHide()
// if the main progress-bar is off.
//
// NOTE: You should NOT check bShowStatus before calling CpHide() because
// we won't hide the panel until we actually run all of the loops we were
// set to expect when we called CpInit()!
void __fastcall TDTSColor::CpHide(bool bForceHide)
{
//#if DEBUG_ON
//  CWrite("\r\nCpHide::FCpCurrentSection: " + String(FCpCurrentSection) +
//                    " FCpTotalSections: " + String(FCpTotalSections) + "\r\n");
//#endif

  if (bForceHide || FCpTotalSections == 0 ||
                              FCpCurrentSection >= FCpTotalSections)
  {
    ResetProgress();

    if (bForceHide)
    {
      FCpTotalSections = 0;
      FCpCurrentSection = 0;
      FCpMaxIterations = 0;
      CpExtra->Caption = "";
      CpMsg->Caption = "";
    }
  }
}
//---------------------------------------------------------------------------
// This function lets us set the maxIterations for the current progress-bar
// process and it increments the current segment but won't affect the
// captions.
//
// Call this before a process loop!
//
// Set maxIterations 0 if you don't want any progress bars at all.
//
void __fastcall TDTSColor::CpSetMaxIterations(int maxIterations)
{
  if (FCpTotalSections <= 0)
    CpInit(1); // at least a main progress bar with one process!
  else if (FCpCurrentSection < FCpTotalSections)
    FCpCurrentSection++;
  else
    CpExtra->Caption = "(ERROR: Need to increase CpInit() count!)";

  FCpMaxIterations = maxIterations;

  // Bring up the small progress bar if more than one section
  if (!CpProgressBar->Visible && FCpTotalSections > 1)
  {
    CpProgressBar->Position = 0;
    CpProgressBar->Visible = true;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::CpUpdate(int Value)
// Set bProgress to show progress bar
{
  if (FCpMaxIterations > 0)
  {
    int CpPos = (Value*100)/FCpMaxIterations;

    if (FCpTotalSections > 0 && FCpCurrentSection <= FCpTotalSections)
    {
      int MainPos = (((FCpCurrentSection-1)*100)+CpPos)/FCpTotalSections;
      CpProgressBar->Position = CpPos;
      MainProgressBar->Position = MainPos;
    }
    else // to just use the main progress bar set FCpMaxValue only...
      MainProgressBar->Position = CpPos;
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ResetProgress(void)
{
  if (CancelPanel->Visible)
    CancelPanel->Visible = false;

  if (MainProgressBar->Visible)
    MainProgressBar->Visible = false;

  MainProgressBar->Position = 0;

  // don't clear these here because it will prevent the
  // "(ERROR: Need to increase CpInit() count!)" message the next time
  // CpSetMaxIterations() is called WITHOUT first calling CpInit()!
  // Keep for diagnostics...
  // FCpTotalSections = 0; // OFF
  // FCpCurrentSection = 0; // OFF
  // FCpMaxIterations = 0; // OFF don't really need this cleared either

  Application->ProcessMessages();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::FgRgbStatLabelClick(TObject *Sender)
{
  bRgbRandFG = !bRgbRandFG;
  bFreeze = false;
  UpdateRgbRandLabels();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::BgRgbStatLabelClick(TObject *Sender)
{
  bRgbRandBG = !bRgbRandBG;
  bFreeze = false;
  UpdateRgbRandLabels();
}
//---------------------------------------------------------------------------

void __fastcall TDTSColor::WingRgbStatLabelClick(TObject *Sender)
{
  bRgbRandWing = !bRgbRandWing;
  bFreeze = false;
  UpdateRgbRandLabels();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::MenuClientClick(TObject *Sender)
// Called when the user clicks an item in the Client menu.
{
  try
  {
    TMenuItem *p = (TMenuItem *)Sender;
    if (!p) return;

    if (p->Tag != Cli)
    {
      // Ask user if they mind having the edit text cleared...
      // have to do that if changing from an RGB client to a non-RGB client
      // and there is coded text...
      if (utils->TextContainsFormatCodes(SL_IRC->Text) && IsYahTrinPal() &&
        p->Tag != C_PALTALK && p->Tag != C_YAHELITE && p->Tag != C_TRINITY)
      {
        // "OK to clear the text and set a new client-mode?"
        if (utils->ShowMessageU(Handle, DS[212],
                      MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1) == IDNO)
          return;
      }

      SetNewClient(p->Tag, true);
      
      // Add 7/25/15 (In Play.cpp search for new client needs this to be NULL!)
      GReplyTo = NULL;

      // Set the TAB to "Process"
      if (PageControl1->ActivePage != PageControl1->Pages[0])
      {
        PageControl1->ActivePage = PageControl1->Pages[0];
        PageControl1Change(NULL);
      }
    }
  }
  catch(...)
  {
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::SetNewClient(int Client, bool bAskSetupPaltalk)
// bAskSetupPaltalk defaults false - set to ask to set font, etc. for Paltalk
{
  try
  {
    // Limit-check
    if (Client < 0 || Client > MAXCLIENTS-1)
      Client = 0;

    if (bIsPlaying)
    {
      TPlay* pb = new TPlay(this);
      pb->DoStopPlay(); // Close DDE link set play-buttons
      delete pb;
    }

    // Instantiate TPaltalk
    if (Client == C_PALTALK)
    {
      // Tell user they can hold SHIFT down to test
      PlayButton->Hint = String(DS[47]);

      if (!GPaltalk)
        try { GPaltalk = new TPaltalk(); } catch(...) {}

      if (GPaltalk)
      {
        if (bAskSetupPaltalk)
        {
          // "Auto-set everything for Paltalk?"
          if (utils->ShowMessageU(Handle, DS[46],
                      MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1) == IDYES)
          {
            // Set everything for Paltalk
            Borders->Checked = false;
            LeftJustify->Checked = true;
            AutoWidth->Checked = true;
            Margins->Checked = false;

            // Fudge to set the font before we set Cli (below)
            int temp = Cli;
            Cli = Client;
            utils->SetDefaultFont(tae);
            Cli = temp;

            Background = IRCWHITE;

            if (Foreground == IRCWHITE)
              Foreground = IRCBLUE;

            UpdateColorButtons();
          }
        }
      }
      else
        utils->ShowMessageU("Unable to instantiate TPaltalk()!");
    }
    else
    {
      PlayButton->Hint = String(DS[28]);

      if (GPaltalk != NULL)
      {
        try { delete GPaltalk; } catch(...) {}
        GPaltalk = NULL;
      }

      try
      {
        if (GRooms != NULL && GRoomIndex >= 0 && GRoomIndex < GRooms->Count)
          PlayChan = GRooms->Strings[GRoomIndex];
      }
      catch(...)
      {
        // Repair!
        GRooms->Clear();
        GRooms->Add(STATUSCHAN);
        GRoomIndex = 0;
        PlayChan = STATUSCHAN; // "status"

#if DEBUG_ON
        CWrite("\r\nTDTSColor::SetNewClient() ROOMS REPAIR!\r\n");
#endif
      }
    }

    if (Client == C_YAHELITE || Client == C_TRINITY || Client == C_PALTALK)
      ConfigureForPaltalk(Client);
    else
    {
      ConfigureForIRC(Client);

      // Set flags to non-registry values only if the client changed
      if (this->Cli != Client)
      {
        // Always use DLL for XiRCON
        bUseDLL = (Client == C_XIRCON) ? true : false;

        // Vortec can do play/pause/resume/stop by itself, mIRC can't
        // do pause/resume, mIRC and Vortec both strip spaces even
        // if the text ends in a CTRL_K... so we either must turn on
        // our internal PadSpaces algorithm or use the dll to send the
        // text by writing a line of text to a file and then sending
        // mIRC a /play <file> command.
        //
        // PadSpaces is a mIRC design-flaw workaround to allow ASCII art
        // to be sent.
        //
        // NOTE: As of 7/24/2015 mIRC still strips spaces - even if you use
        // /raw -nq PRIVMSG #room :"my     text" - the text goes to the server
        // space-stripped and does NOT get echoed to the chat window. The -q
        // switch seems to have no effect.
        //
        // Setting UseFile true means you do not need to use the pad-spaces
        // algorithm.
        bPadSpaces = (Client == C_VORTEC || Client == C_MIRC) ? true : false;

        bUseFile = false;
      }
    }

    // Set our global Cli var and status display and the
    // checks in the Client menu...
    this->Cli = SetClientStatus(Client); // Display client on front panel...
    UpdateRgbRandLabels();
  }
  catch(...){}
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ConfigureForPaltalk(int Client)
{
  BlendScopeRadioButtons->Enabled = true;
  BlendDirectionRadioButtons->Enabled = true;
  RGBThresholdEdit->Enabled = true;
  RGBThresholdUpDown->Enabled = true;

  tsBlender->Enabled = true; // Enable blender controls
  PageControl1->Pages[TAB_BLENDER]->TabVisible = true; // show blend tab

  MenuEffectFonts->Enabled = true;
  MenuFgBlend->Enabled = true;
  MenuBgBlend->Enabled = true;
  MenuChangeColorPalette->Enabled = true;
  MenuMorph->Enabled = true;
  FontType1->Enabled = true;
  FontSize1->Enabled = true;

  // Fudge to set the font before we set Cli (below)
  // Need this for IRC if we were in paltalk
  int temp = Cli;
  Cli = Client;
//  utils->SetDefaultFont(tae); S.S. 10/20/2015 bug
  utils->SetRichEditFont(tae);

  if (SL_ORG->Count > 0)
  {
    bool bProcessFlag = tae->View == V_RTF ? true : false;
    ClearAndFocus(true, true); // leave V_ORG
    LoadView(V_ORG);

    if (bProcessFlag)
      DoProcess(true);
  }
  else
    ClearAndFocus(true, true); // leave V_ORG


  Cli = temp;

  UpdateColorButtons();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ConfigureForIRC(int Client)
{
  BlendScopeRadioButtons->Enabled = false;
  BlendDirectionRadioButtons->Enabled = false;
  RGBThresholdEdit->Enabled = false;
  RGBThresholdUpDown->Enabled = false;

  tsBlender->Enabled = false; // Disable blender-tab controls
  PageControl1->Pages[TAB_BLENDER]->TabVisible = false; // hide blend tab

  MenuEffectFonts->Enabled = false;
  MenuFgBlend->Enabled = false;
  MenuBgBlend->Enabled = false;
  MenuChangeColorPalette->Enabled = false;
  MenuMorph->Enabled = false;
  FontType1->Enabled = false;
  FontSize1->Enabled = false;

  // Clear text if going from an RGB client to a plain-codes client
  if (IsYahTrinPal())
  {
    // Fudge to set the font before we set Cli (later)
    // Need this for IRC if we were in paltalk
    int temp = Cli;
    Cli = Client;

    RangeCheckColorPanels();

    utils->SetDefaultFont(tae);

    if (SL_ORG->Count > 0)
    {
      bool bProcessFlag = tae->View == V_RTF ? true : false;
      ClearAndFocus(true, true); // leave V_ORG
      LoadView(V_ORG);

      if (bProcessFlag)
        DoProcess(true);
    }
    else
      ClearAndFocus(true, true); // leave V_ORG

    Cli = temp;
  }

  UpdateColorButtons();
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::RangeCheckColorPanels(void)
{
  // Only limit for IRC clients!
  if (!IsYahTrinPal())
  {
    if (!ColorInRangeIRC(Foreground))
      Foreground = FOREGROUNDINIT;

    if (!ColorInRangeIRC(Background))
      Background = BACKGROUNDINIT;

    if (!ColorInRangeIRC(WingColor))
      WingColor = WINGCOLORINIT;

    if (!ColorInRangeIRC(BorderBackground))
      BorderBackground = BORDERBACKGROUNDINIT;

    if (!ColorInRangeIRC(BlendBackground))
      BlendBackground = BLENDBACKGROUNDINIT;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TDTSColor::ColorInRangeIRC(int C)
{
  if (C <= 0)
    return false; // no RGB allowed for IRC

  if (C > paletteSize && C != IRCRANDOM && C != IRCTRANSPARENT)
    return false; // no RGB allowed for IRC

  return true;
}
//---------------------------------------------------------------------------
int __fastcall TDTSColor::SetClientStatus(int Client)
// Called by MenuClientClick when the user clicks a new client in
// the Client menu or on initialization via SetNewClient() or when
// a WM_COPYDATA message comes in from a particular client and we
// need to automatically change clients...
//
// To add a new Client you need to add an entry here, and to the #defines
// in Main.h, and to the table in DefaultStrings.h at DS[233]
// Also look at the MenuClientClick handler...
{
  if (ClientMenu->Count != MAXCLIENTS)
  {
    utils->ShowMessageU("Number of Client menu-items does not match\n"
                "number of client-enumerated types!");
    return 0;
  }

  String S = DS[224]; // Client:

  if (Client >= 0 && Client < MAXCLIENTS) S += CLIENTS[Client];
  else S += "Unknown";

  ClientStatus->Caption = S;

  // Set menu checkmarks
  for (int ii = 0; ii < ClientMenu->Count; ii++)
  {
    if (ClientMenu->Items[ii]->Tag == Client) ClientMenu->Items[ii]->Checked = true;
    else ClientMenu->Items[ii]->Checked = false;
  }

  return Client;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ClientStatusClick(TObject *Sender)
{
// Don't want this to easily change the client after-all
// (and clear the GReplyTo handle!)
////  #define BM_CLICK 0x00f5
//  int temp = Cli;
//  if (++temp >= MAXCLIENTS)
//    temp = 0;
//
//  SetNewClient(temp); // Display client on front panel, etc...
//
//  // Add 7/25/15 (In Play.cpp search for new client needs this to be NULL!)
//  GReplyTo = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::CopyColorMenuItemClick(TObject *Sender)
{
  // Copy Color
  TComponent* c = ColorCopyPasteMenu->PopupComponent;

  if (!c)
    return;

  if (c->ClassNameIs("TPanel"))
  {
    TPanel* p = (TPanel*)c;
    Clipboard()->AsText = utils->TColorToHex(p->Color);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ColorCopyPasteMenuPopup(TObject *Sender)
// On menu popup we determine if there is any text to paste first,
// and enable the Paste menu item if there is...
{
  ColorCopyFg = NO_COLOR; // Clear globals
  ColorCopyBg = NO_COLOR;

  try
  {
    // Paste Color
    TComponent* c = ColorCopyPasteMenu->PopupComponent;

    if (!c || !c->ClassNameIs("TPanel") || !Clipboard()->HasFormat(CF_TEXT))
      return;

    // Get the name of the color-panel that was right-clicked
    TPanel* pan = (TPanel*)c;
    String name = pan->Name.LowerCase();

    String S = Clipboard()->AsText;

    PUSHSTRUCT ps;
    utils->SetStateFlags(S, STATE_MODE_FIRSTCHAR, ps);

    if (ps.fg == NO_COLOR && ps.bg == NO_COLOR)
    {
      if (S.Pos("0x") == 1)
        S = S.Delete(1,2);

      if (S.Length() == 6)
      {
        try
        {
          if (S.Pos("0x") != 1)
            S = utils->InsertW(S, "0x", 1); // Need this for ToIntDef, below

          int i = S.ToIntDef(-1); // Need this to see if it's a legit color

          if (i >= 0)
          {
            BlendColor bc = utils->HexToBlendColor(S); // handles "0x" prefix
            int rgb = utils->BlendColorToRgb(bc);
            ColorCopyFg = utils->ResolveRGBExact(rgb);

            if (ColorCopyFg == 0)
              ColorCopyFg = -rgb;
          }
        }
        catch(...) {}
      }
    }
    else
    {
      ColorCopyFg = ps.fg;
      ColorCopyBg = ps.bg;
    }
  }
  catch(...) {}

  if (ColorCopyFg == NO_COLOR)
    PasteFgColorMenuItem->Enabled = false;
  else
    PasteFgColorMenuItem->Enabled = true;

  if (ColorCopyBg == NO_COLOR)
    PasteBgColorMenuItem->Enabled = false;
  else
    PasteBgColorMenuItem->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PasteFgColorMenuItemClick(TObject *Sender)
{
  PasteColor(ColorCopyFg);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PasteBgColorMenuItemClick(TObject *Sender)
{
  PasteColor(ColorCopyBg);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PopupMenu1Popup(TObject *Sender)
{
  // enable/disable PopupTextState menu item
  PopupTextState->Enabled = utils->IsRtfIrcOrgView() && tae->Lines->Count ? true : false;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::PasteColor(int C)
// C is a negative RGB color
{
  TComponent* c = ColorCopyPasteMenu->PopupComponent;

  if (!c || C == NO_COLOR)
    return;

  try
  {
    // Get the name of the color-panel that was right-clicked
    TPanel* pan = (TPanel*)c;
    String name = pan->Name.LowerCase();

    if (name.Length() > 9 && name.Pos("setcolors") == 1)
    {
      name = name.Delete(1,9);

      if (pan != NULL && SetColorsForm != NULL)
      {
        if (name == "fgpanel")
          SetColorsForm->FgColor = C;
        else if (name == "bgpanel")
          SetColorsForm->BgColor = C;
      }
    }
    else if (name.Length() > 9 && name.Pos("alternate") == 1)
    {
      name = name.Delete(1,9);

      if (pan != NULL && AlternateForm != NULL)
      {
        if (name == "panelcolora")
          AlternateForm->ColorA = C;
        else if (name == "panelcolorb")
          AlternateForm->ColorB = C;
      }
    }
    else if (name.Length() > 9 && name.Pos("underline") == 1)
    {
      name = name.Delete(1,9);

      if (pan != NULL && AlternateForm != NULL)
      {
        if (name == "panelcolora")
          AlternateForm->ColorA = C;
        else if (name == "panelcolorb")
          AlternateForm->ColorB = C;
      }
    }
    else if (name.Length() > 5 && name.Pos("morph") == 1)
    {
      name = name.Delete(1,5);

      if (pan != NULL && MorphForm != NULL)
      {
        if (name == "fgtopanel")
          MorphForm->FgToColor = C;
        else if (name == "fgfrompanel")
          MorphForm->FgFromColor = C;
        else if (name == "fgtopanel")
          MorphForm->FgToColor = C;
        else if (name == "bgfrompanel")
          MorphForm->BgFromColor = C;
        else if (name == "bgtopanel")
          MorphForm->BgToColor = C;
      }
    }
    // No longer needed since we gradually added property setters (above) but it's interesting!
    //else if ((name.Length() > 9 && name.Pos("underline") == 1) ||
    //    (name.Length() > 7 && name.Pos("replace") == 1) ||
    //    (name.Length() > 5 && name.Pos("morph") == 1) ||
    //    (name.Length() > 3 && name.Pos("alt") == 1))
    //{
    //  TPanel* pan = (TPanel*)c;
    //  if (pan) pan->Color = utils->YcToTColor(C);
    //}
    else if (name.Length() > 5 && name.Pos("blend") == 1)
      SetBlendColor(pan, utils->YcToTColor(C));
    else if (name == "editwingpanel")
    {
      if (WingEditForm != NULL) WingEditForm->SetBorderBgColor(C);
    }
    else
    {
      if (name == "fgpanel")
      {
        Foreground = C;
        UpdateColorButtons();
      }
      else if (name == "bgpanel")
      {
        Background = C;
        UpdateColorButtons();
      }
      else if (name == "wingcolorpanel")
      {
        WingColor = C;
        UpdateColorButtons();
      }
      else if (name == "blendcolorpanel")
      {
        BlendBackground = C;
        UpdateColorButtons();
      }
    }
  }
  catch(...) {}
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ShowColorMenuItemClick(TObject *Sender)
{
  // Show Color
  TComponent* c = ColorCopyPasteMenu->PopupComponent;

  if (!c) return;

  if (c->ClassNameIs("TPanel"))
  {
    TPanel* p = (TPanel*)c;
    utils->ShowColor(p->Color);
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::EditSpellCheckItemClick(TObject *Sender)
{
  if (!bHaveSpeller)
  {
    // "No Spell-checker: Unable to locate hunspell.dll!\n"
    // "Try uninstalling then reinstalling YahCoLoRiZe...", // 182
    utils->ShowMessageU(DS[182]);
    return;
  }

  Application->CreateForm(__classid(TSpellCheckForm), &SpellCheckForm);
}
//---------------------------------------------------------------------------
WideString __fastcall TDTSColor::U16(const char* p)
{
  String sCopy = String(p);
  return utils->Utf8ToWide(sCopy);
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::YcDestroyForm(TForm* f)
{
  try
  {
    if (f)
    {
// looks to me like release CALLS close - so we get it TWICE!
//      f->Close();
      f->Release();
    }
  }
  catch(...)
  {
    ShowMessage("YcDestroyForm: Unable to delete form!");
  }
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::TabsFlattenRBClick(TObject *Sender)
{
  TabsFlattenRB->Checked = true;
  TabsStripRB->Checked = false;
  regTabMode = FLATTEN_TABS_MULTIPLE_SPACES;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::TabsStripRBClick(TObject *Sender)
{
  TabsFlattenRB->Checked = false;
  TabsStripRB->Checked = true;
  regTabMode = FLATTEN_TABS_SINGLE_SPACE;
}
//---------------------------------------------------------------------------
#if DIAG_SHOWHEX_IS_UTF8
// diagnostic to paste raw ANSI text after clicking Tools->ShowHex
// to convert a block of UTF-8 text to ANSI codes for DefaultStrings.cpp!
void __fastcall TDTSColor::DisplayAnsiToUtf8(void)
{
  utils->PushOnChange(tae);
  tae->Text = utils->AnsiToUtf8(tae->Text);
  utils->PopOnChange(tae);
}
#endif
//---------------------------------------------------------------------------
//void __fastcall TDTSColor::TaeEditChange(TObject *Sender,
//      DWORD dwChangeType, void *pvCookieData, bool &AllowChange)
void __fastcall TDTSColor::TaeEditChange(TObject *Sender)
// We only set this handler after initially showing and filling
// the edit-control to block misfires. This event fires
// on a char or chars added to the control and then fires
// once more on ENTER, before the Edit1KeyPress function is called.
//
// This function adds user keypress codes to the SL_ORG or SL_IRC
// streams depending upon the tae->View. It handles codes both being
// added or removed from the TaeEdit control. This function translates
// raw text-control chars into red-highlighted letters that represent
// the raw code. The Memory stream version has the raw codes.
//
// With an empty edit control, when the user types the first letter,
// we enter this function in tae->View "V_OFF".
//
// This function queues chars for processing by a background thread
// (ThreadOnChange.cpp) to keep this handler from bogging down.
{
  if (tae->LockCounter != 0 || tae->View == V_RTF_SOURCE || tae->View == V_HTML)
    return;

  // Disable changes (faster code)
//  TTaeRichEditChange SaveOnChange = tae->OnChange;
  TNotifyEvent SaveOnChange = tae->OnChange;
  tae->OnChange = NULL;
  tae->LockCounter++;

  if (tae->View == V_OFF)
  {
    utils->SetOldLineVars(tae, true); // init old length and line-count
    utils->WaitForThread();
    SL_ORG->Clear();
  }

  // Populate ONCHANGE thread's struct (ProcessOnChange.h)
  // (Get the deltas in length and line-count and point
  // to the right string-list!) (NOTE: the oc.p stringlist
  // pointer is set below!)
  ONCHANGEW oc = utils->GetInfoOC(tae, NULL);

  // deltaLength can be 0 in OVR (overstrike) mode!
  if (oc.deltaLength == 0 && oc.insert)
    return;

  if (oc.deltaLineCount)
    PrintShortLineCount();

  if (oc.view == V_RTF || oc.view == V_IRC)
    oc.p = SL_IRC;
  else
  {
    oc.p = SL_ORG;

    // Prevent autoset of line-width in RTF mode!
    bTextWasProcessed = false;
  }

  // User deleted chars? (1 or 2 for cr/lf)
  if (oc.deltaLength < 0)
  {
    // NOTE: When chars have been deleted from the Rich Edit control, there
    // is no way to know which char(s) were deleted directetly from the
    // rich-edit (they are gone!) - but fortunately we still have them
    // in SL_IRC or SL_ORG! It's just a matter of finding the char using
    // the new SelStart of the Rich-Edit... and we do that in
    // ThreadOnChange.cpp.

    // deleted a line-terminator then this gives us a flag... (may not be too
    // reliable though...)
// deltaLength is now 1 for +/- new line
//    if (oc.deltaLength == -CRLFLEN)
//      oc.c[0] = C_CR;

    // What happens if Unicode is entered is this: The user types in from 2 to 5
    // hex chars and presses Alt-x. Our key handler intercepts the Alt-x and sets
    // the bUnicodeKeySequence flag and passes the key-event on to TaeRichEdit which
    // replaces the hex chars with a Unicode char and reports an OnChange (here) of
    // between -1 and -4. Our OnChange thread then deletes the excess chars but
    // we still need to make it replace the first char typed with the new Unicode
    // char in our TStrings() list (SL_IRC or SL_ORG). How do we do that?
    //
    // Well, we want one more char removed than is being removed... so add -1 to
    // the already negative oc.deltaLength. Then add a new ONCHANGE struct to the
    // queue that tells our OnChange thread to add a character and where to add it.
    //
    // Do part of all of this here and the rest after we queue the current oc struct...
    if (this->bUnicodeKeySequence)
    {
      // since we replaced the 1st hex-digit with the char in the rich-edit
      // control, we are 1 short of the chars we need to remove from the
      // IRC TStringsW() list... compensate
      oc.deltaLength += -1;
      oc.selStart -= 1;
    }

    ThreadOnChange->Add(oc);

    // Queue to our background thread to add/remove from SL_IRC or SL_ORG
    if (this->bUnicodeKeySequence)
    {
      WideString sCaretChar = tae->GetRangeW(oc.selStart, oc.selStart+1);

      if (sCaretChar.Length() == 1)
      {
        ONCHANGEW ocNew = oc;
        ocNew.selStart = oc.selStart + 1;
        ocNew.c[0] = sCaretChar[1];
        ocNew.deltaLength = 1;
//#if DEBUG_ON
//        CWrite("\r\nadding: " + IntToHex((int)ocNew.c, 5) +
//          " selStart: " + String(oc.selStart) + "\r\n");
//#endif

        // Queue to our background thread to add new unicode char after it deletes
        // the hex-code chars.
        ThreadOnChange->Add(ocNew);
      }

      this->bUnicodeKeySequence = false;
    }

    if (oc.length == 0)
    {
      // Switch view to V_OFF and clear string-lists - this will also
      // remove undo-buffer items prior-to and including the previous
      // call to ClearAndFocus()!
      if (oc.view == V_ORG)
        ClearAndFocus(true, false);
      else // Keep SL_ORG and switch to V_ORG
        ClearAndFocus(true, true);
    }
    else
      UpdateViewMenuState();
  }
  else // added chars or overwrote chars
  {
    if (oc.selStart > 0)
    {
      // Get the char(s) added
      int first;
      if (oc.insert)
        first = oc.selStart-oc.deltaLength;
      else
        // not happy with this, for over-strike we don't have a change in the
        // # chars to go by - under normal conditions this is 1 but... should
        // I use oc.prevPos.x to form a delta???? TODO
        first = oc.selStart-1;

      if (first < 0)
        first = 0;

      int last = oc.selStart;

      if (last > oc.length)
        last = oc.length;

      // NOTE: first/last passed in are mapped like SelStart - I.E. a line-
      // terminator is counted as just one char and is returned in the buffer
      // as one char...
      wchar_t* buf = tae->GetRangeBufW(first, last);

      if (buf != NULL)
      {
        // this copies up to OC_BUFSIZE chars including null if it will fit...
        // but use OC_BUFSIZE when de-buffering as there might not be a
        // terminating null written...
        for (int ii = 0; ii < OC_BUFSIZE; ii++)
        {
          oc.c[ii] = buf[ii];

          if (!buf[ii])
            break;
        }

        if (oc.view == V_IRC || oc.view == V_ORG || oc.view == V_OFF)
        {
          int SaveSelStart = oc.selStart;

          try
          {
            // Here we simply negate the default behavior of the
            // edit-control, which is to keep the red-highlight
            // color in-effect for new chars typed after a red
            // char. We return it to black.

            wchar_t Cnew = utils->GetHighlightLetter(oc.c[0]); // substitute char

            tae->SelStart = oc.selStart-oc.deltaLength;

            if (Cnew != oc.c[0])
            {
              // char entered is a code? change to red and replace char
              tae->SelLength = 1; // Just one char to replace!

              if (tae->SelAttributes->Color != clRed)
                tae->SelAttributes->Color = clRed;

              tae->SelTextW = WideString(Cnew);
              tae->SelLength = 0;
            }
            // Change range to black (could have inserted many chars
            // after a red!)
            else if (tae->SelAttributes->Color == clRed)
              tae->SelAttributes->Color = cColor;
          }
          catch(...)
          {
            utils->ShowMessageU("2: Exception in Main OnChange handler!");
          }

          tae->SelStart = SaveSelStart;
        }

        delete [] buf;
      }
    }

    ThreadOnChange->Add(oc);

    if (oc.view == V_OFF) // Turn V_OFF off!
    {
      tae->View = SetView(V_ORG);

      // Let user "show" the real way the text will look
      if (!ShowButton->Enabled)
        ShowButton->Enabled = true;
    }
    else
      UpdateViewMenuState();
  }

//#if DEBUG_ON
//  CWrite("\r\nTDTSColor::TaeEditChange():\r\n");
//  CWrite("oc.selStart = " + String(oc.selStart) + "\r\n");
//  CWrite("oc.line = " + String(oc.line) + "\r\n");
//  CWrite("oc.lineCount = " + String(oc.lineCount) + "\r\n");
//  CWrite("oc.length = " + String(oc.length) + "\r\n");
//  CWrite("oc.deltaLength = " + String(oc.deltaLength) + "\r\n");
//  CWrite("oc.deltaLineCount = " + String(oc.deltaLineCount) + "\r\n");
//  CWrite("oc.c = " + String(WideString(oc.c)) + " " + IntToHex((int)oc.c,4) + "\r\n");
//  CWrite( "\r\ntae->Text = " + String(tae->Text) + "\r\n");
//#endif


  tae->OldLength = oc.length;
  tae->OldLineCount = oc.lineCount;

  if (!tae->Modified)
    tae->Modified = true;

  if (!bSaveWork)
    bSaveWork = true;

  tae->OnChange = SaveOnChange;
  tae->LockCounter--;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::TaeEditInsertModeChange(TObject *Sender)
{
  String sTxt = tae->InsertMode ? "INS" : "OVR";
  StatusBar->Panels->Items[SB_INS_MODE]->Text = sTxt;
}
//---------------------------------------------------------------------------
void __fastcall TDTSColor::ThreadTerminated(TObject *Sender)
{
  // thread is finished with its work ...
  utils->ShowMessageU("OnChange Thread quit!");
}
//---------------------------------------------------------------------------
#if DEBUG_ON
// DEBUG CONSOLE!!!!!!!
static HANDLE m_Screen = NULL;

void __fastcall TDTSColor::CInit(void)
{
  // allocate a console for this app
  // AllocConsole initializes standard input, standard output,
  // and standard error handles for the new console. The standard input
  // handle is a handle to the console's input buffer, and the standard
  // output and standard error handles are handles to the console's
  // screen buffer. To retrieve these handles, use the GetStdHandle function.
  AllocConsole();
  m_Screen = GetStdHandle (STD_OUTPUT_HANDLE);
}

void __fastcall TDTSColor::CWrite(String S)
{
  WriteConsole(m_Screen, S.c_str(),S.Length(),0,0);
}
#endif
//---------------------------------------------------------------------------

