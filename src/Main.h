//---------------------------------------------------------------------------
// YahCoLoRiZe - Edit, process and send colorized text into IRC chat-rooms
// via various chat-clients such as mIRC, IceCHat and LeafCHat
//
// Author: Scott Swift
//
// Released to GitHub under GPL v3 October, 2016
//
//Rich Edit version, DLL, Window Class:
// 1.0 Riched32.dll RICHEDIT_CLASS
// 2.0 Riched20.dll RICHEDIT_CLASS
// 3.0 Riched20.dll RICHEDIT_CLASS
// 4.1 Msftedit.dll MSFTEDIT_CLASS
//---------------------------------------------------------------------------
#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#define REVISION "7.42" // Unicode Version

// November 20, 2018, 7.42, Fix error in StripFGandUnderlineCodes() in
// Effects.cpp and add a new overload to WriteSingle() in utils.cpp
// (Actually, this function is not even used, presently!)
//
// Fix unhandled exception when setting vertical spacing between multiple boxes
// to 0 (a single line)
//
// November 12, 2018, 7.41, in Effects.cpp the bDoNotTrackSpaceColorChanges flag
// in two calls to utils->SetStateFlags() was reversed.
//
// November 6, 2018, 7.40, in Effects.cpp in CheckExistionCodes, old color
// codes were not being added for the E_SET_COLORS effect.
//
// color/font codes, not after. Plus the wrong states were being used. Also,
// E_SET_COLORS needed to set PrevFG and PrevBG so a pop would set the correct
// color.
//
// December 20, 2015 needed to add 1 for all tae->TextLength - tae->LineCount.
//
// December 18, 2015 1) bug in trim LT spaces if you stripped codes first.
// 2) tab-entry, cursor not advancing
//
// December 16, 2015 1) clear the selection on a replaced word so you can
// see its new colors (if any). Exported HTML file now has correct product
// rev in generator tag. Account for glitch when you select 1 char past
// the end of the document - required change to TaeRichEdit. Can now show
// hex of clipboard-text if nothing selected. ShowHex indices were wrong in
// the IRC/ORG views. All message boxes were displaying "Error" in the caption.
//
// December 12, 2015 Released 7.33 - Fixed extraneous paste-character and
// uninitialized vars in HTML top and left margins which caused the View As
// Webpage to show a blank page.
//
// November 30, 2015 - Build 7.31 fix ",NN" being taken as a BG color after
// stripping BG colors prior to adding the BG blend effect. Fix location of
// "pop" when auto adding push-pop to a selection. Pop should be before any
// trailing codes.
//
// November 28, 2015 - Build 7.30 add WYSIWYG printing, Tabs, Page-breaks,
// Overstrike-typing.
//
// September 10, 2015 - Build 7.29 puts tae->PrintSupport = true
// when user clicks Print or PageSettup
//
// August 10, 2015 - Build 7.24 Fixes Tools->ShowHex and a bug in
// ConvertToRtf.cpp that strips font type and size codes improperly for
// PalTalk. Also we were not setting PlayForm NULL after invoking PlayForm
// in Playback.cpp. Also, we need to delete GPaltalk when destroying objects
// when the main form closes. Re-write of Paltalk.cpp and PaltalkClass.cpp.
//
//---------------------------------------------------------------------------
// ENUMERATED TYPES
//---------------------------------------------------------------------------
// These are the permanent client-side IDs - only Trinity can be changed!
// 0=anyclient,1=yahelite,2=mirc,3=icechat,4=hydrairc,
// 5=leafchat,6=vortec,7=xircon,8=paltalk,9=trinity
// To add a new client, add it to the enumerated types,
// tCli and tCliID, add it to DefaultStrings.h (Clients[] and MS[])
// and in Main.cpp add it to GetColorizeNetClient()
// and GetInternalClient()
enum tCliID {ID_ANY, ID_YAHELITE, ID_MIRC, ID_ICECHAT, ID_HYDRAIRC,
               ID_LEAFCHAT, ID_VORTEC, ID_XIRCON, ID_PALTALK, ID_TRINITY};
// These are used to reference the user-selected client internally
enum tCli {C_YAHELITE, C_MIRC, C_ICECHAT, C_HYDRAIRC,
          C_LEAFCHAT, C_VORTEC, C_XIRCON, C_PALTALK, C_TRINITY, MAXCLIENTS};

enum tBlendDirection {LTR, CENTER1, CENTER2, RTL};
//---------------------------------------------------------------------------
#define REGISTRY_KEY "\\Software\\Discrete-Time Systems\\YahCoLoRiZe"

#define FORCEREGISTRY false
#define VALIDATE_ON false // Add up .EXE file and compare to code
#define LICENSE_ON false // Check license-key
#define SECURITY_ON false

// NOTE: With debug ON the main edit won't always be focused!
#define DEBUG_ON false // Include a debug console, use DTSColor->CWrite("")

#define DIAG_SHOWHEX_IS_UTF8 false // Tools->ShowHex converts UTF-8 to ANSI in main window...

#include <math.h>
#include <time.h>
#include <dir.h>
#include <stdio.h>
#include <windowsx.h>

#include <shlobj.hpp>
#include <sysutils.hpp>
#include <graphics.hpp>
#include <windows.hpp>
#include <messages.hpp>

#include <Buttons.hpp>
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <ExtCtrls.hpp>
#include <DdeMan.hpp>
#include <Mask.hpp>
#include <Dialogs.hpp>
#include <Menus.hpp>
#include <Clipbrd.hpp>
#include <Buttons.hpp>
#include <Filectrl.hpp>
#include <Mask.hpp>
#include <OleCtrls.hpp>
#include "TaeRichEdit.h"

#if DEBUG_ON
#include <iostream.h>
#include <conio.h>
#endif

#include "Undo.h"
#include "Blender.h"
#include "Effects.h"
#include "DefaultStrings.h"
//#include "RichEditConv.h"
#include "PASTESTRUCT.h"

#include "StringsW.h"
//---------------------------------------------------------------------------
// Settings Defaults
//---------------------------------------------------------------------------

// Max RTF table colors allowed for one post in Paltalk
#define DEF_PALTALK_MAXCOLORS 10

// Initial value of GMaxPlayXmitBytes (registry value: "PlayBufSize")
// This can be changed by the user in Play->Options
#define DEF_PLAYBUFSIZE 500 // Yahoo and IRC can take 500

// Default Fonts In UTF-8!
#define USER_DEF_FONT "Courier New"
#define PALTALK_DEF_FONT "Tahoma"
#define IRC_CODES_FONT "Terminal" // Font used when in the IRC-view (to show all control-codes)

// Web-Export Dialog (used in DlgWebExport.cpp and ConvertToHTML.cpp)
#define WEBEXPORT_LEFT_MARGIN 10
#define WEBEXPORT_TOP_MARGIN 10
#define WEBEXPORT_BG_COLOR IRCAQUA
// (-10 is "inherit", 0 is "normal", 120 is 120/100 em)
#define WEBEXPORT_LINE_HEIGHT 0
#define WEBEXPORT_WHITESPACE_STYLE 2 // pre
#define WEBEXPORT_BG_IMAGE_STYLE 0 // repeat
#define WEBEXPORT_BGIMAGE_URL ("bgimage.jpg")
#define WEBEXPORT_TITLE ("My Web Page")
#define WEBEXPORT_AUTHOR ("Mr. Swift")
#define WEBEXPORT_COPY_TO_CLIPBOARD false
#define WEBEXPORT_B_NON_BREAKING_SPACES false
#define WEBEXPORT_B_BGIMAGE false
#define WEBEXPORT_B_BGFIXED false
#define WEBEXPORT_B_AUTHOR false
#define WEBEXPORT_B_TITLE false
#define WEBEXPORT_B_HOME false

// Colorize.ini file
#define DEF_WEBSITE ("http://www.yahcolorize.com/")
#define DEF_EMAIL ("dxzl@live.com")
#define DEF_SONGFORMAT ("Playing: artist, album, C03,07song")

// MyDictionary (spelling-check personal word-list)
#define DEF_DICTIONARY ("en_US") // default spelling dictionary name (in UTF-8)
#define MAX_DICT_COUNT 10000 // max dictionary custom words in colorize.ini
#define INI_COLOR_LINES_MAX 100
#define INI_COLORS_SECTION_NAME "Colors"
#define INI_DICT_SECTION_NAME "MyDictionary"
#define INI_CLIENTS_SECTION_NAME "Clients"
#define INI_FAVCOLORS_LINES_MAX 300
#define INI_FAVCOLORS_SECTION_NAME "FavoriteColors"

// Border defines in UTF-8 (Converted via Clipboard()->AsText = utils->AnsiToUtf8(String(TB));)
#define TB "9,12Â¤~Â»}13@9{Â«~Â¤" // "9,12¤~»}13@9{«~¤"
#define LB "9,12*13@" // "9,12*13@"
#define RB "13,12@9*" // "13,12@9*"

// Client Width and Height
#define CLIENTWIDTHINIT  718
#define CLIENTHEIGHTINIT 453

// Default colors (NOTE: when we change feom RGB client-mode to
// IRC client mode we set these values so choose IRC colors,
// not things like IRCHORIZBLEND!). IRCRANDOM is ok.
#define BACKGROUNDINIT IRCWHITE // MUST be white for YahELite!
#define FOREGROUNDINIT IRCBLUE
#define WINGCOLORINIT IRCRANDOM
#define BLENDBACKGROUNDINIT IRCGRAY
#define BORDERBACKGROUNDINIT IRCBLUE

// Main edit window default background color
#define WINDOWCOLORINIT (TColor)RGB(238,242,242)
// Used for Wing Editor and Find/Replace inactive edit-control color
#define DTS_GRAY (TColor)RGB(235,235,235)

#define TOPBOTTOM_ENABLED true
#define ALLOW_WINGDINGS true
#define ALLOW_BORDERS true
#define SIDES_ENABLED true
#define PACK_TEXT true
#define LEFT_JUSTIFY false
#define AUTO_WIDTH false
#define AUTO_SPACING false
#define PLAY_ON_CR false
#define PLAY_ON_CX true
#define SWIFTMIX_SYNC false
#define PACK_LINES true
#define EXTEND_PALETTE false
#define BYPASS_SMILEYS true
#define MARGINS_ENABLED false
#define DEF_FG_CHECKED true
#define DEF_BG_CHECKED true
#define DEF_TABS_TO_SPACES_CHECKED true
#define DEF_TAB_MODE 2 // FLATTEN_TABS_MULTIPLE_SPACES

// Play "Options" dialog
#define USE_DLL false
#define USE_FILE false
#define NO_SPACES true
#define STRIP_PLAYBACK_FONT true
#define SEND_UTF8 true
#define STATUSCHAN (String(PLAYMSG[40]))
#define NSCHAR '_' // "Invisible Spaces" algorithm padding char
#define MAXROOMHISTORY 100
#define MAXROOMCHARS 81

#define WORD_WRAP false

// Blender initial values
#define RESOLVETOPALETTE true
#define BLENDSCOPE BLEND_BY_SELECTION
#define BLENDDIRECTION BLEND_RIGHT
#define FGBLENDPRESET 0
#define BGBLENDPRESET 1
#define RGBTHRESHOLD 3

// Full-spectrum rgb initially on/off
#define RGBRANDFG true
#define RGBRANDBG true
#define RGBRANDWING true

// User-configurable font defaults
#define USER_DEF_CHARSET DEFAULT_CHARSET
#define USER_DEF_COLOR clWindowText
#define USER_DEF_PITCH fpFixed
#define USER_DEF_SIZE 10 // Size 12 leaves lines in an HTML export!
#define USER_DEF_TYPE 1 // Courier New
#define PALTALK_DEF_TYPE 2 // Tahoma

// Used in TaePrintPreview.cpp
// these get propogated to the TaeRichEditPrint.cpp "fudge factors"
// for WYSIWYG print-view and get saved in the registry - at least
// the user can correct the view him/herself until we find a way
// to automate this as font-height changes...
#define DEF_SCALE_Y 10
#define DEF_SCALE_X 1

// Chars on which to auto-break a line on if the line
// is too long...
#define HYPHENCHAR1 '-'
#define HYPHENCHAR2 '/'

// Don't want some other app to flood us!
#define MAX_WM_COPY_LENGTH 10000

// Warning size that processing text will take a long time
#define WARNINGLENGTH 100000

// Size that is needed before processing status box is displayed
#define STATUSLENGTH 1000

//---------------------------------------------------------------------------
// Effect constants used in Effects.cpp
//---------------------------------------------------------------------------
#define E_NONE 0
#define E_INC_COLORS 1
#define E_ALT_COLORS 2
#define E_RAND_COLORS 3
#define E_ALT_CHAR 4
#define E_BOLD 5
#define E_UNDERLINE 6
#define E_ITALIC 7
#define E_PUSHPOP 8
#define E_FONT_MOUNTAIN 9 // Keep order the same here!!!
#define E_FONT_VALLEY 10   // ..
#define E_FONT_DIP 11      // .. (ok to insert new font effects
#define E_FONT_RISE 12     // .. between E_FONT_MOUNTAIN and
#define E_FONT_SAWTOOTH 13 // .. E_FONT_RANDOM)
#define E_FONT_RANDOM 14   // ..up to here!!!
#define E_FG_BLEND 15
#define E_BG_BLEND 16
#define E_SET_COLORS 17 // Requires parameters for colors
#define E_PLAIN 18 // (not used...)
#define E_STRIP_CODES 19
#define E_FONT_SIZE 20 // Requires parameter for font-size
#define E_FONT_TYPE 21 // Requires parameter for font-type
#define E_MORPH_FGBG 22 // Requires parameters
#define E_REPLACE_COLOR 23 // Requires to/from colors

//---------------------------------------------------------------------------
// Other Defaults
//---------------------------------------------------------------------------

#define DEF_TOTAL_WINGS 4

// First four non-blank, non-comment lines in smileys.txt file...
#define SMILEY_SPARES 4

// Iftf (holds web-site, email and swiftmix format)
#define IFTF_SIZE 3
#define INFO_WEB_SITE 0
#define INFO_EMAIL 1
#define INFO_SONG_FORMAT 2

#define TABSINIT 4  // tab-spaces, in characters
#define LMARGINIT 2  // in characters
#define RMARGINIT 2  // in characters
#define TMARGINIT 1  // in lines
#define BMARGINIT 1  // in lines
#define WIDTHINIT 50 // in characters
#define HEIGHTINIT 0
#define SPACINGINIT 2
#define INDENTINIT 0  // in characters

#define PLAYTIME 2000 // in milliseconds
#define BURSTINTERVAL 250 // in milliseconds
#define DEF_CLIENT C_TRINITY // set default client

// Views associated with the TaeRichEdit property "View"
#define V_OFF        0
#define V_ORG        1
#define V_RTF        2
#define V_IRC        3
#define V_HTML       4
#define V_RTF_SOURCE 5

// VSSpell button codes
#define VS_PROMPTREPLACE 4
#define VS_REPLACEALL    16
#define VS_REPLACE       32

#define LINEBURSTSIZE 2
#define TIMER2TIME 5 // We just use this to fire an event
#define TIMER3TIME 250 // Re-paint min interval for title-bar

// PageControl1 Tab-Constants
#define TAB_PROCESS   0
#define TAB_WIDTH     1
#define TAB_MARGINS   2
#define TAB_FORMAT    3
#define TAB_BLENDER   4
#define TAB_OPTIONS   5
#define TAB_TABS      6

// StatusBar Panel ID Constants
#define SB_INS_MODE     0
#define SB_COUNT        1
#define SB_UNUSED       2

// Client Tab button color-fade
#define FG_FADE_R     255
#define FG_FADE_G     255
#define FG_FADE_B     255
#define BG_FADE_R     0
#define BG_FADE_G     0
#define BG_FADE_B     255

// TColorForm's callerID constants used to define
// what color is being set.
#define COLOR_FORM_NONE    0
#define COLOR_FORM_FG      1
#define COLOR_FORM_BG      2
#define COLOR_FORM_EFG     3 // Effect Fg
#define COLOR_FORM_EBG     4 // Effect Bg
#define COLOR_FORM_BLEND   5
#define COLOR_FORM_WING    6
#define COLOR_FORM_BORDER  7
#define COLOR_FORM_HTML    8

// Color to use for highlighting IRC codes
#define HIGHLIGHT_COLOR IRCRED

// Highlight letters that take the place of control-codes
#define HL_B 'B' // Bold
#define HL_K 'C' // Color
#define HL_F 'F' // Font Type
#define HL_R 'I' // Itallics
#define HL_O 'O' // All effects off
#define HL_S 'S' // Font Size
#define HL_U 'U' // Underline
#define HL_PUSH 'P' // Push
#define HL_POP 'p' // Pop
#define HL_T 'T' // Tab
#define HL_N 'N' // FormFeed (we use "<<page>>" instead!)

// mIRC codes
#define CTRL_B '\002' // bold on/off
#define CTRL_K '\003' // color/end of color
#define CTRL_O '\017' // reset text attributes
#define CTRL_R '\026' // inverse video on/off
#define CTRL_U '\037' // underline on/off

// Custom to YahCoLoRiZe
#define CTRL_PUSH '\021' // DC1 ^Q
#define CTRL_POP '\022' // DC2 ^R
#define CTRL_F '\023' // DC3 "font type change" (followed by 00-99)
#define CTRL_S '\024' // DC4 "font size change" (followed by 00-99)

// YahELite (for Yahoo chat, now defunct)
#define CTRL_Z '\032' // YahELite line terminator
#define ALT_S '\023' // ^S YahELite "send text from multiline"

// Standard codes
#define C_NULL '\0'
#define C_TAB '\011' // '\t' 09 hex
#define C_LF '\012' // '\n'  0A hex

// Used in RTF text to denote line-breaks (strange!)
// This is inserted when you have \line instead of \par
#define C_VT '\013' // 0B hex

// we use a plain-old <<page>> in a document. ConvertToIRC
// temporarily converts \pagebreak to
// C_FF as it processes text but then converts it back. So we can
// read in a document with C_FF and they will be converted to
// <<page>> when you press PROCESS. Saving a document as RTF will
// convert <<page>> or C_FF to {\rtf\pard\page\par}. We don't presently
// have a converter from RTF into IRC.
#define C_FF '\014' // 0C hex

#define C_CR '\015' // '\r' 0D hex
#define C_SPACE '\040'
#define CRLF "\r\n"
#define CRLFLEN 2

#define PAGE_BREAK (L"\\pagebreak")

// 1-999 reserved for color-table index
// 1000 on are special modes
// negatives and 0 are RGB colors
// 1-100 have 1 subtracted before sending (0-99)
// (The mIRC chat-client causes all colors over 15 (16 in our case)
// to set the user's default color). "Officially" 99 (100 for us)
// is "Transparent" (User's color)).
//
// I needed a color to represent "return to YahCoLoRiZe's default color",
// (not the same as "Transparent" which the user can select...)
// Assuming future a color palette of multiples od 16, we have
// 96,97,98,99 remaining (97,98,99,100 for us) - 100 is taken for
// Transparent... so we can (in-theory) use 97, 98, 99 internally,
// and replace the code in the optimizer before sending it out to
// a chat-client.
//
// Internally, RGB colors are negative and RGB black is hex 000000,
// white is ffffff. Palette indexes are offset by +1.
//
// mIRC Color Table 01-16
#define IRCWHITE          1
#define IRCBLACK          2
#define IRCNAVY           3
#define IRCGREEN          4
#define IRCRED            5
#define IRCMAROON         6
#define IRCPURPLE         7
#define IRCORANGE         8
#define IRCYELLOW         9
#define IRCLIME           10
#define IRCTEAL           11
#define IRCAQUA           12
#define IRCBLUE           13
#define IRCFUCHSIA        14
#define IRCGRAY           15
#define IRCSILVER         16
// See file Colorize.ini
// EGA Color Table 17-80 (includes 6 grayscale and 6 extra colors)
// Indexes 81-96 are reserved for 16 user-defined palette colors,
// 97 is IRCNOCOLOR (place-holder to avoid errors in text with ",11" etc.)
// 98, 99 are reserved, 100 is "transparent"
//
// 97 I'm defining as IRCNOCOLOR and I need it for the case of where I am
// stripping background colors and have some text in the document like
// "suddenly(Crtl-K)11,22,14 people came in!" the ",14" will be turned into
// a background after we strip all background colors:
// "suddenly(Crtl-K)11,14 people came in!". So we fix it by using:
// "suddenly(Crtl-K)11,96,14 people came in!". The optimizer can replace 96
// with the prevailing background color.
#define IRCNOCOLOR        97   // color place-holder "no-op"
#define IRCRESERVED1      98   // Use Internally only!!!
#define IRCRESERVED2      99   // Use Internally only!!!
#define IRCTRANSPARENT    100  // Causes client to display user's default color
// Internal codes
#define IRCCANCEL         1000 // IRCCANCEL must be 1st of the special codes!
#define IRCRANDOM         1001
#define IRCVERTBLEND      1002
#define IRCHORIZBLEND     1003
#define BLEND_ERR1        1004
#define NO_COLOR          1005 // color not present
#define RGBCOLOR          1006

// Vertical and Horizontal blend color-panel captions
#define BLEND_CAPTION_VERT "V"
#define BLEND_CAPTION_HORIZ "H"
#define TRANSPARENT_CAPTION "T"
#define RANDOM_CAPTION "R"

// Blend-by values
#define BLEND_BY_WORD       0
#define BLEND_BY_LINE       1
#define BLEND_BY_SENTENCE   2
#define BLEND_BY_PARAGRAPH  3
#define BLEND_BY_SELECTION  4

// Blend-direction
#define BLEND_RIGHT         0
#define BLEND_IN            1
#define BLEND_OUT           2
#define BLEND_LEFT          3

// Blend-engine types
#define BLEND_FG       0
#define BLEND_BG       1
#define BLEND_VERT_FG  2
#define BLEND_VERT_BG  3

// Effect ranges for clients sending /ex # text
// (0 is random, 1-18 are for non-rgb clients, 20-29
// are for clients with rgb or font-change capability)
// These constants are not related to the Effect-constants below...
#define E_MAX_EFFECTS_BASIC 19
#define E_MAX_EFFECTS_EXTENDED 29

// copy-modes for Timer2
#define CM_IDLE             0
#define CM_SWIFTMIX_PLAY    1
#define CM_SWIFTMIX_TIME    2
#define CM_PROCESS_RX_DATA  3
#define CM_SWIFTMIX_STATE   4
#define CM_REMOTE_COMMAND   5
#define CM_NCDOUBLECLICK    6
#define CM_NCLEFTCLICK      7
#define CM_NCRIGHTCLICK     8

// Remote commands modes for Timer2
#define REMOTE_COMMAND_START   0
#define REMOTE_COMMAND_STOP    1
#define REMOTE_COMMAND_PAUSE   2
#define REMOTE_COMMAND_RESUME  3
#define REMOTE_COMMAND_CHANNEL 4
#define REMOTE_COMMAND_TIME    5
#define REMOTE_COMMAND_ID      6 // tell us the other app's handle...
#define REMOTE_COMMAND_FILE    7 // tell us a file to play
#define REMOTE_COMMAND_TEXT    8 // We set this to send text

// SwiftMiX play-states
#define PS_SWIFTMIX_IDLE    0
#define PS_SWIFTMIX_STOP    1
#define PS_SWIFTMIX_PAUSE   2

// Blender constants
#define MAXBLENDCOLORS 10
#define MAXDEFAULTCOLORS 16
//#define MAXCOLORS 99 // 00-99 (index 99) reserved for "transparent"
#define MAXBLENDPRESETS 10

// (Nov, 2005) looks like RTF color-table CAN go over 255.  So I
// increased the size to 982 (982+16+1 = 999).  For our auto-generated
// table we can go up to 9 to the 3rd power now (729). Use 31 steps
// to get max r/g/b values of 248.

// 16+982(+1 for RTF format index) = 999 (limit to 3-digits)
//#define MAXRGBPALETTECOLORS 982
// 6 to the 3rd power = 216 RGB colors (fits an 8-bit index)
#define SMALLRGBPALETTESIZE 216 // For MS Word (ostensibly)
//#define MEDIUMRGBPALETTESIZE 729

#define MINLINESIZE 1 // smallest # user can enter for Line-Width
#define MINMARGINSIZE 0
#define MAXARGS 2 // Max arg to get with GetCommandLine();
#define ICON_ARRAY_SIZE 95 // Emoteicon text-array size
#define MAXPUSH 10 // Max number of PushStructs
#define TOTALDEFAULTWINGS 20

#define MYPI 3.14159265358979
//---------------------------------------------------------------------------

// Emote file name (also has web-site and e-mail address)
#define INIFILE FN[15] // colorize.ini
#define SMILEYFILE FN[16] // smileys.txt
#define SMILEYDIR FN[17] // "Smileys\\"
#define DICTDIR FN[18] // "Dict\\"
#define MANUALFILE FN[19] // manual.txt
#define ARTFILE FN[20] // asciiart.txt
#define TECHFILE FN[21] // tech.txt

// Name of Help-file
#define HELPNAME FN[3]
//#define HELPNAME_S String(HELPNAME)
#define OUR_NAME FN[23] // "YahCoLoRiZe"
#define OUR_NAME_S String(OUR_NAME)
#define OUR_TITLE FN[24]
#define OUR_TITLE_S String(OUR_TITLE)
#define OUR_COMPANY FN[4]
//#define OUR_COMPANY_S String(OUR_COMPANY)
#define OUR_CLASS FN[5] // "TDTSColor"
//#define OUR_CLASS_S String(OUR_CLASS)
#define OUR_ICON FN[22] // "Colorize.ico"
//#define OUR_ICON_S String(OUR_ICON)

//---------------------------------------------------------------------------
// NOTE: I was looking for CSIDL_PROGRAM_FILES and it's not in C++ Builder 4
// but I found the definitions in both ShFolder.h and ShlObj.h in my Windows
// SDKs. I'm going to put ShFolder.h in YahCoLoRiZe "Misc" and just #define
// what I'm using here :-)
#ifndef CSIDL_LOCAL_APPDATA

#define CSIDL_LOCAL_APPDATA             0x001c      // non roaming, user\Local Settings\Application Data
#define CSIDL_INTERNET_CACHE            0x0020
#define CSIDL_COOKIES                   0x0021
#define CSIDL_HISTORY                   0x0022
#define CSIDL_COMMON_APPDATA            0x0023      // All Users\Application Data
#define CSIDL_WINDOWS                   0x0024      // GetWindowsDirectory()
#define CSIDL_SYSTEM                    0x0025      // GetSystemDirectory()
#define CSIDL_PROGRAM_FILES             0x0026      // C:\Program Files
#define CSIDL_MYPICTURES                0x0027      // My Pictures, new for Win2K
#define CSIDL_PROGRAM_FILESX86          0x002a      // x86 C:\Program Files on RISC
#define CSIDL_PROGRAM_FILES_COMMON      0x002b      // C:\Program Files\Common
#define CSIDL_PROGRAM_FILES_COMMONX86   0x002c      // x86 Program Files\Common on RISC
#define CSIDL_COMMON_DOCUMENTS          0x002e      // All Users\Documents
#define CSIDL_RESOURCES                 0x0038      // %windir%\Resources\, For theme and other windows resources.
#define CSIDL_RESOURCES_LOCALIZED       0x0039      // %windir%\Resources\<LangID>, for theme and other windows specific resources.
#define CSIDL_SYSTEMX86                 0x0029      // x86 system directory on RISC


#define CSIDL_FLAG_CREATE               0x8000      // new for Win2K, or this in to force creation of folder

#define CSIDL_COMMON_ADMINTOOLS         0x002f      // All Users\Start Menu\Programs\Administrative Tools
#define CSIDL_ADMINTOOLS                0x0030      // <user name>\Start Menu\Programs\Administrative Tools

#endif // CSIDL_LOCAL_APPDATA
//---------------------------------------------------------------------------
// Shorthand for the main edit-control
#define tae DTSColor->TaeEdit
//---------------------------------------------------------------------------
// Registered Windows Messages
#define WMS_COLORIZENET "WM_ColorizeNet"
#define WMS_SWIFTMIXTIME "WM_SwiftMixTime"
#define WMS_SWIFTMIXPLAY "WM_SwiftMixPlay"
#define WMS_SWIFTMIXSTATE "WM_SwiftMixState"
#define WMS_DATACOLORIZE "WM_DataCoLoRiZe"
#define WMS_CHANCOLORIZE "WM_ChanCoLoRiZe"
#define WMS_PLAYCOLORIZE "WM_PlayCoLoRiZe"
//---------------------------------------------------------------------------
// DO NOT CHANGE these sizes because SwiftMiX C++/C# depend on them
// as do the HydraIRC, LeafChat and IceChat plugins!!!
//
// Max size (in bytes) of message to send to/from YahCoLoRiZe
#define CNS_DATALEN 2048
// Max size (in bytes) of channel or nickname
#define CNS_CHANNICKLEN 512

// Rich-text (RTF) Clipboard formats
// "Rich Text Format Without Objects" CF_RTFNOOBJS,
// "RichEdit Text and Objects" CF_RETEXTOBJ
#define RTF_REGISTERED_NAME ("Rich Text Format") // CF_RTF
// HTML Clipboard Format
#define HTML_REGISTERED_NAME ("HTML Format")

// Custom control IDs (passed to CreateWindow())

// DlgPlay.cpp
#define ID_ComboRooms 155 // Assign a random control ID to our Unicode Combobox
#define ID_EditFile 156
// DlgWebExport.cpp
#define ID_EditImageUrl 157
#define ID_EditTitle 158
#define ID_EditAuthor 159
#define ID_EditHomeLink 160

// SendMessageTimeout flags
#define SMTO_NORMAL             0x0000 // apply timeout
#define SMTO_BLOCK              0x0001 // wait forever
#define SMTO_ABORTIFHUNG        0x0002 // no timeout if target hung
#define SMTO_NOTIMEOUTIFNOTHUNG 0x0008 // don't start timeout if target
                                       // message-loop is running...
#define SMTO_ERRORONEXIT        0x0020 // Return 0 if target window dies
#define SMTO_TIMEOUT            3000   // Wait 3 seconds

//---------------------------------------------------------------------------
// Custom classes
class TDTSColor : public TForm
{
  // Class forward references
  friend class TTaePageSetupDialog;
  friend class TTaePrintDialog;
  friend class TTaeRichEdit;

__published:
  TMenuItem *HelpMenu;
  TMenuItem *EditMenu;
  TMenuItem *FileMenu;
  TMainMenu *MainMenu;
  TMenuItem *FileOpenUtf8;
  TMenuItem *FileSaveAsItem;
  TMenuItem *FileExitItem;
  TMenuItem *EditClearItem;
  TMenuItem *EditCutItem;
  TMenuItem *EditCopyItem;
  TMenuItem *EditPasteItem;
  TMenuItem *HelpAboutItem;
  TPanel *SpeedBar;
  TMenuItem *EditSelectAllItem;
  TMenuItem *N1;
  TPopupMenu *PopupMenu1;
  TMenuItem *PopupPaste;
  TMenuItem *PopupCut;
  TMenuItem *PopupSelectAll;
  TMenuItem *PopupProcess;
  TMenuItem *PopupClear;
  TMenuItem *PopupCopy;
  TMenuItem *ViewMenu;
  TMenuItem *Original1;
  TMenuItem *EditFindItem;
  TMenuItem *TextDemo;
  TMenuItem *N11;
  TMenuItem *N12;
  TMenuItem *RestoreFactorySettings1;
  TMenuItem *ASCIIArtDemo;
  TMenuItem *PlayMenu;
  TMenuItem *StartPlay1;
  TMenuItem *StopPlay1;
  TMenuItem *SetPlay1;
  TMenuItem *N4;
  TMenuItem *PausePlay1;
  TMenuItem *Resume1;
  TMenuItem *N13;
  TMenuItem *ToolsMenu;
  TMenuItem *MenuWingEdit;
  TMenuItem *MenuSetFgColor;
  TMenuItem *MenuSetBgColor;
  TMenuItem *MenuShowHex;
  TMenuItem *MenuSetFont;
  TMenuItem *Help1;
  TMenuItem *N3;
  TMenuItem *EffectsMenu;
  TMenuItem *MenuIncriment;
  TMenuItem *FilePrintItem;
  TMenuItem *N9;
  TMenuItem *MenuAlternate;
  TMenuItem *MenuRandom;
  TMenuItem *MenuSet;
  TMenuItem *MenuColorFractal;
  TMenuItem *RTF1;
  TMenuItem *IRC1;
  TMenuItem *RTFSource1;
  TMenuItem *MenuRandomNum;
  TMenuItem *MenuChangeColorPalette;
  TMenuItem *HTML1;
  TMenuItem *MenuUnderline;
  TMenuItem *MenuEffectFormat;
  TMenuItem *PlainText1;
  TMenuItem *CtrlK1;
  TMenuItem *Italics2;
  TMenuItem *Underline3;
  TMenuItem *Bold3;
  TMenuItem *MenuEmoticons;
  TMenuItem *wwwYahCoLoRiZecom1;
  TMenuItem *FontSize1;
  TMenuItem *EditUndoItem;
  TMenuItem *Push1;
  TMenuItem *Pop1;
  TMenuItem *ImportBlendColors1;
  TMenuItem *ExportBlendColors1;
  TMenuItem *N10;
  TMenuItem *PopupOriginal;
  TMenuItem *FontSize;
  TMenuItem *FontMountain;
  TMenuItem *FontValley;
  TMenuItem *FontRize;
  TMenuItem *FontDip;
  TMenuItem *MenuEffectFonts;
  TMenuItem *FontDoubleMountain;
  TMenuItem *FontSawtooth;
  TTaeRichEdit *TaeEdit;
  TMenuItem *MenuSetWingColor;
  TRadioGroup *BlendDirectionRadioButtons;
  TRadioGroup *BlendScopeRadioButtons;
  TMenuItem *MenuFgBlend;
  TPanel *BlendExtensionPanel;
  TLabel *RGBThresholdLabel;
  TMenuItem *N7;
  TMenuItem *WordWrap;
  TMenuItem *MenuBgBlend;
  TPanel *ProgressPanel;
  TProgressBar *MainProgressBar;
  TMenuItem *ExportAsWebPage1;
  TMenuItem *N14;
  TMenuItem *FontRandom;
  TMenuItem *PopupPlay;
  TMenuItem *N6;
  TTimer *Timer2;
  TTimer *RepaintTimer;
  TMenuItem *N15;
  TMenuItem *MenuCharacterMap;
  TMenuItem *TechDemo;
  TPageControl *PageControl1;
  TTabSheet *TabControl;
  TLabel *FgColorLabel;
  TLabel *BgColorLabel;
  TLabel *WingColorLabel;
  TButton *ProcessButton;
  TPanel *FgPanel;
  TPanel *BgPanel;
  TPanel *WingColorPanel;
  TBitBtn *StopButton;
  TBitBtn *PlayButton;
  TBitBtn *PauseButton;
  TBitBtn *ResumeButton;
  TTabSheet *tsWidth;
  TLabel *WidthLabel;
  TCheckBox *AutoWidth;
  TTabSheet *tsFormat;
  TCheckBox *LeftJustify;
  TCheckBox *PackText;
  TCheckBox *PackLines;
  TCheckBox *Borders;
  TCheckBox *Wingdings;
  TTabSheet *tsBlender;
  TLabel *ImportBlenderLabel;
  TPanel *Blend0;
  TPanel *Blend2;
  TPanel *Blend3;
  TPanel *Blend4;
  TPanel *Blend5;
  TPanel *Blend6;
  TPanel *Blend7;
  TPanel *Blend8;
  TPanel *Blend9;
  TPanel *Blend1;
  TPanel *ColorBlenderPresetPanel;
  TButton *Preset0;
  TButton *Preset2;
  TButton *Preset1;
  TButton *Preset3;
  TButton *Preset4;
  TButton *Preset5;
  TButton *Preset6;
  TButton *Preset7;
  TButton *Preset8;
  TButton *Preset9;
  TButton *ImportButton;
  TButton *ImportAsciiButton;
  TTabSheet *tsOptions;
  TCheckBox *AutoDetectEmotes;
  TCheckBox *PlayOnCR;
  TCheckBox *SwiftMixSync;
  TPanel *CancelPanel;
  TLabel *CpMsg;
  TLabel *CpExtra;
  TTimer *Timer1;
  TMenuItem *MenuStripCodes;
  TMenuItem *MenuOptimize;
  TUpDown *RGBThresholdUpDown;
  TEdit *RGBThresholdEdit;
  TCheckBox *PlayOnCX;
  TMenuItem *PopupProcessSongTitle;
  TMenuItem *N17;
  TDdeServerItem *ddetext;
  TDdeServerItem *ddechan;
  TDdeServerConv *command;
  TMenuItem *PopupDelete;
  TMenuItem *N18;
  TMenuItem *EditPasteColorItem;
  TMenuItem *PopupPasteColor;
  TPanel *BlendColorPanel;
  TLabel *Label1;
  TMenuItem *N8;
  TStaticText *ViewStatus;
  TStaticText *ClientStatus;
  TMenuItem *ViewInBrowser1;
  TMenuItem *N19;
  TButton *ShowButton;
  TMenuItem *FileSaveItem;
  TMenuItem *FileNewItem;
  TTabSheet *tsMargins;
  TMaskEdit *WidthEdit;
  TMaskEdit *TMarginEdit;
  TMaskEdit *BMarginEdit;
  TLabel *TMarginLabel;
  TLabel *BMarginLabel;
  TMenuItem *PageSetup1;
  TMenuItem *MenuMorph;
  TMenuItem *MenuStripSpaces;
  TMenuItem *MenuJoinLines;
  TMaskEdit *LMarginEdit;
  TMaskEdit *HeightEdit;
  TLabel *LMarginLabel;
  TLabel *HeightLabel;
  TMaskEdit *SpacingEdit;
  TMaskEdit *IndentEdit;
  TLabel *IndentLabel;
  TLabel *SpacingLabel;
  TCheckBox *AutoHeight;
  TMaskEdit *RMarginEdit;
  TLabel *RMarginLabel;
  TCheckBox *Margins;
  TLabel *WingRgbStatLabel;
  TLabel *BgRgbStatLabel;
  TLabel *FgRgbStatLabel;
  TMenuItem *ClientMenu;
  TMenuItem *N01;
  TMenuItem *N110;
  TMenuItem *N21;
  TMenuItem *N31;
  TMenuItem *N41;
  TMenuItem *N51;
  TMenuItem *N61;
  TMenuItem *N71;
  TDdeServerItem *ddevtext;
  TDdeServerItem *ddeplay;
  TMenuItem *N81;
  TMenuItem *MenuReplace;
  TPopupMenu *ColorCopyPasteMenu;
  TMenuItem *CopyColorMenuItem;
  TMenuItem *PasteFgColorMenuItem;
  TMenuItem *ShowColorMenuItem;
  TMenuItem *N20;
  TMenuItem *N22;
  TMenuItem *N23;
  TMenuItem *MenuSetWindowColor;
  TMenuItem *PasteBgColorMenuItem;
  TMenuItem *EditReplaceItem;
  TMenuItem *TextWingsDemo;
  TMenuItem *N16;
  TMenuItem *EditSpellCheckItem;
  TMenuItem *FontType1;
  TMenuItem *FontType;
  TCheckBox *ExtendPalette;
  TMenuItem *FileOpenAnsi;
  TMenuItem *PrintPreview1;
  TTabSheet *tsTabs;
  TCheckBox *TabsToSpaces;
  TLabel *TabsLabel;
  TMaskEdit *TabsEdit;
  TRadioButton *TabsFlattenRB;
  TRadioButton *TabsStripRB;
  TMenuItem *PageBreak1;
  TStatusBar *StatusBar;
  TProgressBar *CpProgressBar;
  TMenuItem *MenuPadSpaces;
  TMenuItem *MenuFavoriteColors;
  TMenuItem *PopupTextState;
  void __fastcall ImportBlender(TObject *Sender);
  void __fastcall ExportBlender(TObject *Sender);
  void __fastcall ImportBlenderText(TObject *Sender);
  void __fastcall ExportBlenderText(TObject *Sender);
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall FileOpenUtf8Menu(TObject *Sender);
  void __fastcall FileExit(TObject *Sender);
  void __fastcall EditCut(TObject *Sender);
  void __fastcall EditCopy(TObject *Sender);
  void __fastcall EditPaste(TObject *Sender);
  void __fastcall HelpAbout(TObject *Sender);
  void __fastcall EditSelectAllClick(TObject *Sender);
  void __fastcall FileSaveAsItemClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall MenuWingEditClick(TObject *Sender);
  void __fastcall MenuSetFgColorClick(TObject *Sender);
  void __fastcall MenuSetBgColorClick(TObject *Sender);
  void __fastcall ProcessButtonClick(TObject *Sender);
  void __fastcall StartPlayClick(TObject *Sender);
  void __fastcall StopPlayClick(TObject *Sender);
  void __fastcall SetPlay1Click(TObject *Sender);
  void __fastcall EditClearItemClick(TObject *Sender);
  void __fastcall Original1Click(TObject *Sender);
  void __fastcall RTF1Click(TObject *Sender);
  void __fastcall IRC1Click(TObject *Sender);
  void __fastcall RTFSource1Click(TObject *Sender);
  void __fastcall TaeEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall Timer1Event(TObject *Sender);
  void __fastcall TextDemoClick(TObject *Sender);
  void __fastcall RestoreFactorySettings1Click(TObject *Sender);
  void __fastcall AutoWidthClick(TObject *Sender);
  void __fastcall AutoHeightClick(TObject *Sender);
  void __fastcall MonkeyMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall ASCIIArtDemoClick(TObject *Sender);
  void __fastcall ResumePlayClick(TObject *Sender);
  void __fastcall PausePlayClick(TObject *Sender);
  void __fastcall PauseButtonClick(TObject *Sender);
  void __fastcall MenuShowHexClick(TObject *Sender);
  void __fastcall MenuSetFontClick(TObject *Sender);
  void __fastcall EffectSetFgBgClick(TObject *Sender);
  void __fastcall Help1Click(TObject *Sender);
  void __fastcall FilePrintClick(TObject *Sender);
  void __fastcall BordersClick(TObject *Sender);
  void __fastcall FormActivate(TObject *Sender);
  void __fastcall IncrementForegroundBackgroundClick(TObject *Sender);
  void __fastcall EffectAlternateClick(TObject *Sender);
  void __fastcall EffectRandomClick(TObject *Sender);
  void __fastcall MenuStripCodesClick(TObject *Sender);
  void __fastcall MenuColorFractalClick(TObject *Sender);
  void __fastcall MenuSetWingColorClick(TObject *Sender);
  void __fastcall MenuRandomNumClick(TObject *Sender);
  void __fastcall MenuChangeColorPaletteClick(TObject *Sender);
  void __fastcall HTML1Click(TObject *Sender);
  void __fastcall Bold3Click(TObject *Sender);
  void __fastcall Underline3Click(TObject *Sender);
  void __fastcall Italics2Click(TObject *Sender);
  void __fastcall CtrlPushClick(TObject *Sender);
  void __fastcall CtrlPopClick(TObject *Sender);
  void __fastcall PlainText1Click(TObject *Sender);
  void __fastcall MenuUnderlineClick(TObject *Sender);
  void __fastcall MenuEmoticonsClick(TObject *Sender);
  void __fastcall SelectBlendPreset(String Topic);
  bool __fastcall SelectFontSize(String Topic, int &Size);
  bool __fastcall SelectFontType(String Topic, int &Type);
  void __fastcall DisplaySecurityCode1Click(TObject *Sender);
  DWORD __fastcall ComputeSecurityCode(void);
  bool __fastcall CompareSecurityCode(void);
  void __fastcall wwwYahCoLoRiZecom1Click(TObject *Sender);
  void __fastcall PageControl1Change(TObject *Sender);
  void __fastcall BlendDirectionRadioButtonsClick(TObject *Sender);
  void __fastcall BlendScopeRadioButtonsClick(TObject *Sender);
  void __fastcall BlendMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall BlendFGMenu(TObject *Sender);
  void __fastcall BlendBGMenu(TObject *Sender);
  void __fastcall BlendPresetMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall ImportButtonMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall EditUndoItemClick(TObject *Sender);
  void __fastcall WingdingsClick(TObject *Sender);
  void __fastcall ImportTextButtonMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall FontSawtoothClick(TObject *Sender);
  void __fastcall FontRandomClick(TObject *Sender);
  void __fastcall FontMountainClick(TObject *Sender);
  void __fastcall FontValleyClick(TObject *Sender);
  void __fastcall FontRise1Click(TObject *Sender);
  void __fastcall FontDipClick(TObject *Sender);
  void __fastcall FontDoubleMountainClick(TObject *Sender);
  void __fastcall FgPanelMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall BgPanelMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall WingColorPanelMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall FormDestroy(TObject *Sender);
  void __fastcall WordWrapClick(TObject *Sender);
  void __fastcall FormMouseWheel(TObject *Sender, TShiftState Shift,
          int WheelDelta, TPoint &MousePos, bool &Handled);
  void __fastcall InfoPanelMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall ExportAsWebPage1Click(TObject *Sender);
  void __fastcall Timer2Timer(TObject *Sender);
  void __fastcall TaeEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall RepaintTimeout(TObject *Sender);
  void __fastcall MenuCharacterMapClick(TObject *Sender);
  void __fastcall TechDemoClick(TObject *Sender);
  void __fastcall MenuOptimizeClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall RGBThresholdUpDownClick(TObject *Sender,
          TUDBtnType Button);
  void __fastcall RGBThresholdEditChange(TObject *Sender);
  void __fastcall ProcessSongTitleClick(TObject *Sender);
  void __fastcall ddetextPokeData(TObject *Sender);
  void __fastcall ddechanPokeData(TObject *Sender);
  void __fastcall PopupDeleteClick(TObject *Sender);
//  void __fastcall TaeEditMouseMove(TObject *Sender, TShiftState Shift,
//          int X, int Y);
  void __fastcall FontSize1Click(TObject *Sender);
  void __fastcall EditPasteColor(TObject *Sender);
  void __fastcall PackTextClick(TObject *Sender);
  void __fastcall BlendColorPanelMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall EffectsMenuClick(TObject *Sender);
  void __fastcall ViewInBrowser1Click(TObject *Sender);
  void __fastcall ShowButtonClick(TObject *Sender);
  void __fastcall FileSaveItemClick(TObject *Sender);
  void __fastcall MaskEditEnter(TObject *Sender);
  void __fastcall WidthEditEnter(TObject *Sender);
  void __fastcall WidthEditClick(TObject *Sender);
  void __fastcall PrinterPageSetupClick(TObject *Sender);
  void __fastcall MenuStripSpacesClick(TObject *Sender);
  void __fastcall MenuJoinLinesClick(TObject *Sender);
  void __fastcall MaskEditDblClick(TObject *Sender);
  void __fastcall WidthEditDblClick(TObject *Sender);
  void __fastcall EffectMorphClick(TObject *Sender);
  void __fastcall MarginsClick(TObject *Sender);
  void __fastcall LeftJustifyClick(TObject *Sender);
  void __fastcall TaeEditKeyPress(TObject *Sender, char &Key);
  void __fastcall FgRgbStatLabelClick(TObject *Sender);
  void __fastcall BgRgbStatLabelClick(TObject *Sender);
  void __fastcall WingRgbStatLabelClick(TObject *Sender);
  void __fastcall MenuClientClick(TObject *Sender);
  void __fastcall ClientStatusClick(TObject *Sender);
  void __fastcall ddeplayPokeData(TObject *Sender);
  void __fastcall EffectReplaceColorsClick(TObject *Sender);
  void __fastcall CopyColorMenuItemClick(TObject *Sender);
  void __fastcall PasteFgColorMenuItemClick(TObject *Sender);
  void __fastcall ShowColorMenuItemClick(TObject *Sender);
  void __fastcall ColorCopyPasteMenuPopup(TObject *Sender);
  void __fastcall MenuSetWindowColorClick(TObject *Sender);
  void __fastcall PasteBgColorMenuItemClick(TObject *Sender);
  void __fastcall EditFindItemClick(TObject *Sender);
  void __fastcall EditReplaceItemClick(TObject *Sender);
  void __fastcall TaeEditKeyUp(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall TaeEditMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall TextWingsDemoClick(TObject *Sender);
  void __fastcall FileNewItemClick(TObject *Sender);
  void __fastcall EditSpellCheckItemClick(TObject *Sender);
  void __fastcall FontType1Click(TObject *Sender);
  void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
  void __fastcall ExtendPaletteClick(TObject *Sender);
  void __fastcall FileOpenAnsiMenu(TObject *Sender);
  void __fastcall PrintPreview1Click(TObject *Sender);
  void __fastcall PageBreak1Click(TObject *Sender);
  void __fastcall TabsFlattenRBClick(TObject *Sender);
  void __fastcall TabsStripRBClick(TObject *Sender);
  void __fastcall TaeEditChange(TObject *Sender);
  void __fastcall TaeEditInsertModeChange(TObject *Sender);
  void __fastcall MenuPadSpacesClick(TObject *Sender);
  void __fastcall FavoriteColorsClick(TObject *Sender);
  void __fastcall PopupTextStateClick(TObject *Sender);
  void __fastcall PopupMenu1Popup(TObject *Sender);
//  void __fastcall TaeEditChange(TObject *Sender, DWORD dwChangeType,
//          void *pvCookieData, bool &AllowChange);

private:        // private user declarations
#if DEBUG_ON
  void __fastcall CInit(void);
#endif

#if DIAG_SHOWHEX_IS_UTF8
  // diagnostic to paste raw ANSI text after clicking Tools->ShowHex
  // to convert a block of UTF-8 text to ANSI codes for DefaultStrings.cpp!
  void __fastcall DisplayAnsiToUtf8(void);
#endif

  void __fastcall EffectMorphOrReplace(bool bUseDialog, bool bMorph);
  bool __fastcall IsRawCodeMode(void);
  bool __fastcall IsWinVistaOrHigher(void);
  void __fastcall PasteColor(int C);
  void __fastcall SetLine(int idx);
  void __fastcall PrintLineCount(void);
  void __fastcall PrintShortLineCount(void);
  void __fastcall SetBlendColor(TPanel* p, TColor c);
  void __fastcall SetNewClient(int Client, bool bAskSetupPaltalk=false);
  int __fastcall SetClientStatus(int Client);
  int __fastcall GetColorizeNetClient(int Client);
  int __fastcall GetInternalClient(int Client);
  void __fastcall SafeEditAdd(WideString S);
  void __fastcall IncDecColor(BlendColor &bc,
                           bool &bRedUp, bool &bGreenUp, bool &bBlueUp,
                             int redDelta, int greenDelta, int blueDelta);
  void __fastcall AppException(TObject *Sender, Exception *E);
  bool __fastcall InitLocalPlayback(int Time);
  void __fastcall LoadBlendMemoryFromDefaultPreset(const int Init[],
                                              BlendColor* BC);
  int __fastcall ConvertIrcToRtf(TStringsW* sl,
                      bool bBlackAndWhite, bool bShowStatus);
  int __fastcall ConvertIrcToYahoo(bool bShowStatus);
  int __fastcall ConvertIrcToHtml(bool bShowStatus);
  int __fastcall ConvertTextToIrc(bool bShowStatus);

  void __fastcall GetRegInfo(bool RestoreFactory);
  void __fastcall InitMainMenu(void);
  void __fastcall InitAllSettings(bool RestoreFactory);
  void __fastcall SaveBlendPresets(void);
  void __fastcall SaveToRegistry(void);
  int __fastcall RegisterInAddRemovePrograms(void);
  bool __fastcall UnegisterFromAddRemovePrograms(void);
  void __fastcall WriteInstallKeys(TRegistry* r);

  int __fastcall LoadBlendPresetsFromRegistry(void);
  bool __fastcall ReadFromRegistry(void);
  bool __fastcall DoBlend(int Type, bool bUndo);
  void __fastcall DoShowRtf(bool bShowStatus);
  void __fastcall DoProcess(bool bShowStatus);
  void __fastcall DoP(bool bShowStatus);
  void __fastcall Boxify(void);
  void __fastcall AddEffect(int Effect, bool bUndo=true,
              bool bShowRTF=true, bool bShowStatus=true);
  int __fastcall Outside(double rec, double imc, int k);
  void __fastcall GenColorFract(void);
  void __fastcall SaveFileViaStream(WideString wFile, bool SavePlain);
  void __fastcall DoSaveFile(WideString wFile);
  void __fastcall DoDemo(WideString wFile, bool bAnsi = false);
  TFontStyles __fastcall IntToStyle(int Value);
  int __fastcall StyleToInt(TFontStyles Value);

  void __fastcall BlendPanelColorClick(TPanel *P);
  void __fastcall BlendPresetLoadClick(TObject *Sender);
  void __fastcall BlendPresetStoreClick(TObject *Sender);

  void __fastcall BlendPanelClick(TObject *Sender);
  void __fastcall BlendPanelSelectClick(TObject *Sender);
  void __fastcall BlendPanelEnableClick(TObject *Sender);
  void __fastcall SetBgColor(void);
  void __fastcall SetFgColor(void);
  void __fastcall SetBlendBackgroundColor(void);
  void __fastcall SetWingColor(void);

  bool __fastcall PaintBlendColorButtons(void);
  TPanel* __fastcall GetPointerToColorPanel(int idx);
  bool __fastcall PaintPresetButtons(void);
  void __fastcall ClearAndFocus(bool bClearSL, bool bLeaveOrg=false);
  void __fastcall DoClear(bool bLeaveOrg=false);
  void __fastcall EmbraceSelection(char c);

  // Overloaded function
  bool __fastcall LoadLines(TMemoryStream* ms,
                               bool bClearSL, bool bPlainText);
  bool __fastcall LoadLines(TStringsW* sl,
                                bool bClearSL, bool bPlainText);
  bool __fastcall LoadLines(WideString Filename,
                    bool bClearSL, bool bPlainText);

  void __fastcall UpdateProcessButtons(int View);

  String __fastcall YahELiteColor(int Color, bool StrType);
  String __fastcall YahELiteRandomColor(bool StrType, bool bRgbRand);
  void __fastcall UpdateRgbRandLabels(void);
  String __fastcall DisplaySecurityCode(void);
  void __fastcall WMNCPaint(TColor fg, TColor bg);
  void __fastcall WMNCLDblClk(void);
  void __fastcall WMNCRDownClk(void);

  bool __fastcall SaveBlendPresetToRegistry(BlendColor* BC, int Idx);
  bool __fastcall BlendColorEqual(BlendColor BC1, BlendColor BC2);
  bool __fastcall BlendColorsEqual(BlendColor P1[], BlendColor P2[]);
  void __fastcall SaveWorkingBlendPreset(void);
  void __fastcall RangeCheckColorPanels(void);
  bool __fastcall ColorInRangeIRC(int C); // Good IRC Color?

  void __fastcall Blend(bool bFirstChar, int RealLength,
                       String &TempStr, char* buf, int idx);
  bool __fastcall CreateBlendEngine(int Type);
  void __fastcall DeleteBlendEngine(void);
  bool __fastcall DeleteBlenderObjects(void);
  void __fastcall ProcessError(int Error, int DefaultExtendedError = -1000);
  void __fastcall DoFontEffect(int type, int p1, int p2, double p5);

  bool __fastcall PopulateEnabledBlendStructs(BlendColor* BC,
                                       int EnabledCount, int Type);
  void __fastcall SetWordWrap(bool bOn);
  void __fastcall ConfigureForIRC(int Client);
  void __fastcall ConfigureForPaltalk(int Client);
  bool __fastcall TempAutoWidth(bool bOn);
  bool __fastcall OpenFileSetDefaultDirs(WideString wFile);
  bool __fastcall OpenYahcolorizeFile(WideString wFile,
                                             bool bShowStatus);
  void __fastcall WMDropFile(TWMDropFiles &Msg);
  void __fastcall HintTimerEvent(TObject *Sender);
  bool __fastcall AddRoom(String S);
  void __fastcall DoFindReplace(bool bReplaceMode);

  bool __fastcall GenerateHTML(void);
  WideString __fastcall GenerateHTML(bool bCreateDialog,
                                              WideString FileName = "");
  void __fastcall SetViewMenuHandlers(bool bEnable);
  void __fastcall UpdateViewMenuState(void);
  int __fastcall SetView(int NewView);

  // This reads an ANSI string in UTF-8 format in the registry and returns UTF-16
  WideString __fastcall RegReadStringW(TRegistry* pReg, char* pVal, char* pDef, bool* bErr = NULL);
  // This reads an ANSI string in UTF-8 format in the registry and returns either ANSI or UTF-8
  String __fastcall RegReadString(TRegistry* pReg, char* pVal, char* pDef, bool* bErr = NULL);
  bool __fastcall RegWriteStringW(TRegistry* pReg, String sVal, WideString sIn);

  int __fastcall RegReadInt(TRegistry* pReg, char* pVal, int Def);
  bool __fastcall RegReadBool(TRegistry* pReg, char* pVal, bool Def);
  PUSHSTRUCT __fastcall GetState(void);
  String __fastcall ReadColorsFromIni(TIniFile* pIni, int &count);
  bool __fastcall ReadPluginClassAndNameFromIni(TIniFile* pIni, int &count);
  void __fastcall CopyIniFile(void);
  bool __fastcall ReadIniFile(void);
  bool __fastcall WriteDictToIniFile(void);
  void __fastcall YcDestroyForm(TForm* f);
//  char* __fastcall GetMappedMemory(String MemName, int MemSize);

  ////////////////////// PRIVATE VARS ////////////////////////

  // Added to intercept a WM_COPYDATA from YahELite
  Controls::TWndMethod OldWinProc;

  // Needed for WM_COPYDATA with YahELite chat client
  //const char* const classname = "YahELite:class";
  UINT RWM_YahELiteExt;

  unsigned short cbHTML, cbRTF; // Registered clipboard format

  // XiRCON
  UINT RWM_ChanCoLoRiZe;
  UINT RWM_DataCoLoRiZe;
  UINT RWM_PlayCoLoRiZe;

  // IceChat, HydraIRC, LeafChat and Trinity
  UINT RWM_ColorizeNet;

  // SwiftMiX
  UINT RWM_SwiftMixTime, RWM_SwiftMixPlay, RWM_SwiftMixState;

  int BlendButtonSelectIndex;

  int Cli; // Client ID

  int UndoMode; // Present Undo available is 0=empty, 1=char, 2=cut, 3=paste

  TProcessEffect* ProcessEffect;

  // Fractal generator
  int F_Width, F_Height, F_Level;
  double F_Scale, F_Recen, F_Imcen;

  bool bIsColorFractal, bWordWrapWarningShown;
  bool bFreeze, bHaveSpeller;
  bool bIsVistaOrHigher, bSaveWork;
  bool bUnicodeKeySequence;

  // Flags to determine if we should set the TaeRichEdit FileName
  // before print or print-pewview (cleared when the document is cleared)
  bool bFileOpened, bFileSaved;

  // Used to restore the color when
  // changing it via the mouse-wheel
  BlendColor MouseBlendColor;
  int MouseBlendIndex;

  int RGBThreshold;

  TBlendEngine* pBE;

  // Private vars used by friend classes...

  // Playback to YahELite (Playback.cpp)
  // Friend classes: TPlayForm, TPlayback
  friend class TPlayForm;
  friend class TPlayback;
  friend class TPlay;
//  friend class TConvertToYahoo; // StripFont is only var...
  // License-Key properties
  // Friend classes: KeyClass
//  friend class KeyClass;
//
//  int today, installDay, keyDay, activationDays, forceKey, paidKey;
//  int remTimeBasedOnKeyDate;

  // Effects processing properties
  // Friend classes: TProcessEffect
  friend class TProcessEffect;

  // handle of window to reply to, set when a message arrives
  HWND GReplyTo; // Accessed in Playback.cpp

  // Vars for text-playback into chat-rooms via a plugin, via
  // WM_COPYDATA messages or via DDE (Dynamic Data Exchange)
  wchar_t* GPlayBuffer;
  unsigned PlayTime;
  unsigned int GInterval;
  int GPlayIndex, GPlaySize, GLineCount, GPlayer;
  int GLineBurstSize, GLineBurstCounter;
  int GMaxPlayXmitBytes; // Set by the user in DlgPlay.cpp

  // Vars used to relocate the cursor to the same line as we
  // traverse views...
  int linesInRtfHeader, orgLinePosition;

  // MorphDlg.cpp
  bool bFgChecked, bBgChecked;
  int FgToColor, FgFromColor;
  int BgToColor, BgFromColor;
  int DialogSaveMaxColors;

  int EParm0, EParm1, EParm2, EParm3, EParm4;
  double EParm5;

  bool bNewInstallation;
  bool bUseDLL, bUseFile, bPadSpaces;

  //////////////////////// Properties! ///////////////////////
  //////////////////////// Properties! ///////////////////////
  //////////////////////// Properties! ///////////////////////
  //////////////////////// Properties! ///////////////////////

  WideString cfont;

  TColor taeWindowColor;

  bool bIsPlaying, bTextWasProcessed, bMyDictChanged;
  bool bEffectWasAdded, bShiftHeldOnPlay;
  bool bStripFont, bSendUtf8;
  bool bIsAnsiFile;

  TStringList* g_rooms;

  TStringsW* sL_HTM;
  TStringsW* sL_IRC;
  TStringsW* sL_ORG;
  TMemoryStream* mS_RTF;

  TStringsW* leftWings;
  TStringsW* rightWings;

  TBlendEngine* fG_BlendEngine;
  TBlendEngine* bG_BlendEngine;

  BlendColor* fG_BlendColors;
  BlendColor* bG_BlendColors;

  // CancelPanel and progress-bars
  int FCpTotalSections, FCpCurrentSection, FCpMaxIterations;

  int captionFG, captionBG;

  int fG_BlendPreset, bG_BlendPreset;

  int regLmargin, regRmargin, regIndent;
  int regTmargin, regBmargin;
  int regWidth, regHeight, regSpacing;
  int regTabs, regTabMode;

  int regWysiwygPrintScaleX, regWysiwygPrintScaleY;

  int roomIndex, largestPlayLineWidth, largestPlayLineBytes;

  int* palette;
  int paletteSize, paletteExtent;

  int paltalkMaxColors;

  int wmCopyServer, wmCopyChannel, wmCopyClient;
  int wmCopyCommand, wmCopyData, wmCopyMode;

  // Used for the popup menu for copying/pasting colors
  int ColorCopyFg, ColorCopyBg;

  // Web-Export Dialog Reg Vars
  int webPageLineHeight, webPageBgColor;
  int webPageLeftMargin, webPageTopMargin;
  int webPageWhiteSpaceStyle, webPageBgImageStyle;
  String webPageBgImageUrl, webPageHomeButtonUrl;
  String webPageTitle, webPageAuthor;
  bool webPage_bNonBreakingSpaces, webPage_bBgImage, webPage_bBgFixed;
  bool webPage_bAuthor, webPage_bTitle, webPage_bHome;

  WideString wDeskDir;

  // Lists for the clients with a custom plugin (class/name)
  TStringList* pluginClass;
  TStringList* pluginName;

protected:
  // Added to intercept a WM_COPYDATA from YahELite
  void __fastcall CustomMessageHandler(TMessage &msg);
  void __fastcall ThreadTerminated(TObject *Sender);

BEGIN_MESSAGE_MAP
  //add message handler for WM_DROPFILES
  // NOTE: All of the TWM* types are in messages.hpp!
  VCL_MESSAGE_HANDLER(WM_DROPFILES, TWMDropFiles, WMDropFile)
END_MESSAGE_MAP(TComponent)

public:         // public user declarations
  __fastcall TDTSColor(TComponent* Owner);

#if DEBUG_ON
  void __fastcall CWrite(String S);
#endif

  // function to convert utf-8 strings in DefStrings into UTF-16 strings
  WideString __fastcall TDTSColor::U16(const char* p);

  // Handle progress bars and CancelPanel
  void __fastcall CpInit(int progressSections);
  void __fastcall CpSetMaxIterations(int maxIterations);
  void __fastcall CpShow(String sMsg, String sAuxMsg = "");
  void __fastcall CpReset(void);
  void __fastcall CpHide(bool bForceHide = false);
  void __fastcall CpUpdate(int Value);
  void __fastcall ResetProgress(void);

  void __fastcall ComputeAllColors(int ActualLineCount, int IRCLineCount);
  void __fastcall UpdateColorButtons(void);
  void __fastcall LoadPaletteFromString(String S, int count);
  bool __fastcall ReadSmileys(void);
  int __fastcall LoadView(int NewView);
  bool __fastcall LoadBlendPresetFromRegistry(BlendColor* BC, int Idx);
  bool __fastcall IsCli(tCli isCli);
  bool __fastcall IsPlugin(void);
  bool __fastcall IsPlugin(int c);
  bool __fastcall IsYahTrinPal(void);
  bool __fastcall IsWmCopyData(void);
  int __fastcall GetPrevEnabledBlendIndex(int idx, BlendColor* BC);
  int __fastcall GetNextEnabledBlendIndex(int idx, BlendColor* BC);
  int __fastcall GetEnabledBlendButtonCount(BlendColor* BC);
  bool __fastcall SetBlendButtonColor(TPanel* p, BlendColor* BC);
  void __fastcall ShowColor(int Color);
  void __fastcall ShowColor(TColor Color);
  void __fastcall SetAfgAbgAwcAbl(void);

  // Replace Dialog needs access to these...
  PASTESTRUCT  __fastcall TaeEditCutCopy(bool bCut, bool bCopy);
  PASTESTRUCT __fastcall TaeEditPaste(bool bColor);
  PASTESTRUCT  __fastcall TaeEditPaste(bool bColor, WideString wTemp);
  void __fastcall SetUndoEnabled(bool bValue);
  bool __fastcall GetUndoEnabled(void);

  ////////////////////// PUBLIC VARS ////////////////////////

  // WM_COPYDATA vars (UTF-8)
  String WmCopyStr, WmChanNickStr;
  String WmPath, WmName, WmArtist, WmAlbum;
  String PlayChan;

  WideString OpenD, OpenF, SaveD, SaveF; // Directory paths
  WideString TopBorder, LeftSideBorder, RightSideBorder;
  WideString Default_Playfile;
  WideString PluginVersion, GDictionary;
  WideString SaveFindText, SaveReplaceText;

  TStringList* GDictList; // this is kept in UTF-8 format!

  // Public properties
  int LineWidth, LineHeight;

  // Client Width/Height
  int ClientWidth, ClientHeight;

  unsigned BurstInterval;

  // Table used for RTF color-table
  int* RGB_Palette;
  int RGB_PaletteSize;

  // Color vars and flags
  bool bRgbRandFG, bRgbRandBG, bRgbRandWing;

  int Foreground, Afg, WingColor, Awc, Background, Abg;
  int BlendBackground, Abl; // Blender default background color
  int BorderBackground; // Wing-editor border background color
  int GlobalColor; // Color set in the color-dialog (Color.cpp)
  int PrevFG, PrevBG, PrevAwc; // Previous colors

  int GBlendPreset;
  int ProgressPosition;

  TStringsW* Emotes;
  TStringList* Iftf;

  bool bTopEnabled, bSideEnabled;

  int TotalWings;

  char NSChar; // "invisible spaces" char

  HINSTANCE hDll;

  // Font information (must declare these here
  // something with the VCL, who knows what...)
  TFontCharset cCharset;
  TColor cColor;
  TFontPitch cPitch;
  int cSize, cType;
  TFontStyles cStyle;

  TTimer* HintTimer;
  THintWindow* HintWindow;

  // Global vars for various remotely sent
  // data via WM_COPY...
  int WmPlayer;
  int WmDuration;
  int WmCurrent;
  int WmColor;
  int WmRedtime;
  int WmYellowtime;
  int WmState;

  String CmdArgs[MAXARGS];
  int CmdIdx; // Holds # of arguements in command-line up to MAXARGS

  //////////////////// Properties ////////////////////
  __property TColor TaeWindowColor = {read = taeWindowColor};

  __property bool EffectWasAdded = {read = bEffectWasAdded};

  __property WideString cFont = {read = cfont, write = cfont};
  __property TStringList* PluginClass = {read = pluginClass};
  __property TStringList* GRooms = {read = g_rooms, write = g_rooms};
  __property TStringList* PluginName = {read = pluginName};
  __property TStringsW* SL_IRC = {read = sL_IRC, write = sL_IRC};
  __property TStringsW* SL_ORG = {read = sL_ORG, write = sL_ORG};
  __property TStringsW* SL_HTM = {read = sL_HTM, write = sL_HTM};
  __property TStringsW* LeftWings = {read = leftWings, write = leftWings};
  __property TStringsW* RightWings = {read = rightWings, write = rightWings};
  __property TMemoryStream* MS_RTF = {read = mS_RTF, write = mS_RTF};

  // Blender Vars ************************************
  __property TBlendEngine* FG_BlendEngine = {read = fG_BlendEngine};
  __property TBlendEngine* BG_BlendEngine = {read = bG_BlendEngine};

  __property BlendColor* FG_BlendColors = {read = fG_BlendColors};
  __property BlendColor* BG_BlendColors = {read = bG_BlendColors};

  __property int FG_BlendPreset = {read = fG_BlendPreset};
  __property int BG_BlendPreset = {read = bG_BlendPreset};

  __property int CaptionFG = {read = captionFG};
  __property int CaptionBG = {read = captionBG};

  __property int GRoomIndex = {read = roomIndex, write = roomIndex};
  __property int LargestPlayLineWidth = {read = largestPlayLineWidth, write = largestPlayLineWidth};
  __property int LargestPlayLineBytes = {read = largestPlayLineBytes, write = largestPlayLineBytes};
  __property int RegTabs = {read = regTabs};
  __property int RegTabMode = {read = regTabMode};
  __property int RegLmargin = {read = regLmargin};
  __property int RegRmargin = {read = regRmargin};
  __property int RegIndent = {read = regIndent};
  __property int RegBmargin = {read = regBmargin};
  __property int RegTmargin = {read = regTmargin};
  __property int RegHeight = {read = regHeight};
  __property int RegSpacing = {read = regSpacing};
  __property int RegWidth = {read = regWidth};

  __property int WmCopyServer = {read = wmCopyServer, write = wmCopyServer};
  __property int WmCopyChannel = {read = wmCopyChannel, write = wmCopyChannel};
  __property int WmCopyCommand = {read = wmCopyCommand};
  __property int WmCopyData = {read = wmCopyData};

  __property int PaletteSize = {read = paletteSize};
  __property int PaltalkMaxColors = {read = paltalkMaxColors};
  __property int* Palette = {read = palette};

  __property bool ShiftHeldOnPlay = {read = bShiftHeldOnPlay, write = bShiftHeldOnPlay};
  __property bool UndoEnabled = {read = GetUndoEnabled, write = SetUndoEnabled};
  __property bool TextWasProcessed = {read = bTextWasProcessed, write = bTextWasProcessed};
  __property bool IsPlaying = {read = bIsPlaying};
  __property bool IsVistaOrHigher = {read = bIsVistaOrHigher};
  __property bool Freeze = {read = bFreeze, write = bFreeze};
  __property bool MyDictChanged = {read = bMyDictChanged, write = bMyDictChanged};
  __property bool StripFont = {read = bStripFont};
  __property bool SendUtf8 = {read = bSendUtf8};
  __property bool IsAnsiFile = {read = bIsAnsiFile};

  __property WideString DeskDir = {read = wDeskDir};

  __property int CpMaxIterations = {read = FCpMaxIterations, write = FCpMaxIterations};
  __property int CpCurrentSection = {read = FCpCurrentSection, write = FCpCurrentSection};
  __property int CpTotalSections = {read = FCpTotalSections, write = FCpTotalSections};
  
  __property int WebPageLineHeight = {read = webPageLineHeight, write = webPageLineHeight};
  __property int WebPageBgColor = {read = webPageBgColor, write = webPageBgColor};
  __property int WebPageLeftMargin = {read = webPageLeftMargin, write = webPageLeftMargin};
  __property int WebPageTopMargin = {read = webPageTopMargin, write = webPageTopMargin};
  __property int WebPageWhiteSpaceStyle = {read = webPageWhiteSpaceStyle, write = webPageWhiteSpaceStyle};
  __property int WebPageBgImageStyle = {read = webPageBgImageStyle, write = webPageBgImageStyle};
  __property String WebPageBgImageUrl = {read = webPageBgImageUrl, write = webPageBgImageUrl};
  __property String WebPageHomeButtonUrl = {read = webPageHomeButtonUrl, write = webPageHomeButtonUrl};
  __property String WebPageTitle = {read = webPageTitle, write = webPageTitle};
  __property String WebPageAuthor = {read = webPageAuthor, write = webPageAuthor};
  __property bool WebPage_bNonBreakingSpaces =
        {read = webPage_bNonBreakingSpaces, write = webPage_bNonBreakingSpaces};
  __property bool WebPage_bBgImage =
        {read = webPage_bBgImage, write = webPage_bBgImage};
  __property bool WebPage_bBgFixed =
        {read = webPage_bBgFixed, write = webPage_bBgFixed};
  __property bool WebPage_bAuthor =
        {read = webPage_bAuthor, write = webPage_bAuthor};
  __property bool WebPage_bTitle =
        {read = webPage_bTitle, write = webPage_bTitle};
  __property bool WebPage_bHome =
        {read = webPage_bHome, write = webPage_bHome};
};
//---------------------------------------------------------------------------
// I'm redefining this to be the same all the time for all of my
// Plugins, SwiftMiX, etc... for 64-bit compatibility! 1/2/2014
struct MYCOPYDATASTRUCT
{
  unsigned __int32 dwData; // IntPtr
  unsigned __int32 cbData; // int
  unsigned __int64 lpData; // IntPtr
};

// Struct to xfer text between clients and YahCoLoRiZe
//
// DONT'T CHANGE THIS!!!! The LeafChat, IceChat and HydraIRC plugins
// all depend on this never changing!!!!! (2596 bytes)
struct COLORIZENET_STRUCT
{
  unsigned __int64 lspare;
  __int32 clientID;
  __int32 commandID;
  __int32 commandData;
  __int32 serverID;
  __int32 channelID;
  __int32 lenChanNick; // length of channel or nickname string in characters, without the null
  __int32 lenData; // length of data string in characters, without the null
  BYTE chanNick[CNS_CHANNICKLEN];
  BYTE data[CNS_DATALEN];

  COLORIZENET_STRUCT()
  {
    lspare = 0;
    // TODO: Implement clientID on WM_COPYDATA in!
    clientID = 0; // 0=anyclient,1=yahelite,2=mirc,3=icechat,4=hydrairc,
                  // 5=leafchat,6=vortec,7=xircon,8=PALTALK,9=trinity
    commandID = -1;
    commandData = -1;
    serverID = -1;
    lenChanNick = 0;
    lenData = 0;
    chanNick[0] = 0;
    data[0] = 0;
 }
};
//---------------------------------------------------------------------------
extern TDTSColor* DTSColor;
//---------------------------------------------------------------------------
#endif
