#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

//************************************************************************
// This table allows us to maintain a constant index across various
// platforms over the IRC network, independant of the fonts on a given machine.
const char* FONTS[] =
{
// Internal fonts for which we embed a code-index to represent it in
// the IRC text... 1-99 (0 is reserved to indicate the default font)
// ok to add as many as you want... but change FONTITEMS in the header file!
//
// Completely case independant...
"Courier New", // 1 (do not change) USER_DEF_TYPE
"Tahoma", // 2 (do not change) PALTALK_DEF_TYPE
"Arial",
"Arial Black",
"Arial Narrow",
"Arial Unicode MS",
"Batang",
"Batangche",
"Book Antiqua",
"Bookman Old Style", // 10
"Bookshelf Symbol 7",
"Browallia New",
"Browalliaupc",
"Buxton Sketch",
"Calibri",
"Calibri Light",
"Cambria",
"Cambria Math",
"Candara",
"Century", // 20
"Century Gothic",
"Comic Sans MS",
"Consolas",
"Constantia",
"Corbel",
"Cordia New",
"Cordiaupc",
"Courier",
"Daunpenh",
"David", // 30
"Default",
"Dilleniaupc",
"Dokchampa",
"Euphemia",
"Fangsong",
"Fixedsys",
"Franklin Gothic Medium",
"Freesiaupc",
"Gabriola",
"Garamond", // 40
"Gautami",
"Georgia",
"Gelvetica",
"Impact",
"Irisupc",
"Iskoola Pota",
"Jasmineupc",
"Javanese Text",
"Kaiti",
"Kalinga", // 50
"Kartika",
"Khmer UI",
"Kodchiangupc",
"Kokila",
"Lao UI",
"Latha",
"Leelawadee",
"Lucida Console",
"Lucida Sans Unicode",
"Malgun Gothic", // 60
"Mangal",
"Marlett",
"Meiryo",
"Meiryo UI",
"Miriam",
"Miriam Fixed",
"Modern",
"Mongolian Baiti",
"Moolboran",
"Mt Extra", // 70
"Mv Boli",
"Myanmar Text",
"Nyala",
"Palatino Linotype",
"Plantagenet Cherokee",
"Raavi",
"Roman",
"Script",
"Sketchflow Print",
"Small Fonts", // 80
"Symbol",
"System",
"Terminal",
"Times New Roman",
"Traditional Arabic",
"Trebuchet MS",
"Verdana",
"Webdings",
"Wingdings",
"Wingdings 2", // 90
"Wingdings 3",
"@Arial Unicode MS",
"@Batang",
"@Dotum",
"@Kaiti",
"@Malgun Gothic",
"@Meiryo UI",
"@Mingliu",
"@Mingliu_hkscs", // 99
};
//************************************************************************
const char* VIEWS[] =
{
  // ViewStatus Text Display
  "View: Original", // 0
  "View: Processed", // 1
  "View: IRC Codes", // 2
  "View: HTML", // 3
  "View: RTF Source", // 4
  "View: Off", // 5
  "View: Unknown", // 6
};
//************************************************************************
// To add a new client, add it to the enumerated types,
// tCli and tCliID, add it here and in the MS[] menu-array below
// and in Main.cpp add it to GetColorizeNetClient()
// and GetInternalClient()
const char* CLIENTS[] =
{
  // ClientStatus Text Display
  "YahELite",
  "mIRC",
  "IceChat",
  "Hydra",
  "LeafChat",
  "Vortec",
  "XiRCON",
  "Paltalk",
  "Trinity",
};
//************************************************************************
const char* WINGEDITMAINMENU[] =
{
  "Options",

  // These Wings are in "ANSI UTF-8" a backslash
  // escape is in front of the " or \ char.
  "Insert Wingding",
  "<~*-,._.,-*~'^'~*-,",
  ",-*~'^'~*-,._.,-*~>",
  "Â´Â¯`~Â°Â³Â²Â¹ÂºÂªÂ¤", // "´¯`~°³²¹ºª¤"
  "Â¤ÂªÂºÂ¹Â²Â³Â°~Â´Â¯`", // "¤ªº¹²³°~´¯`"
  "\"Â°ÂºÂ©o., ", // "\"°º©o., "
  " ,.oÂ©ÂºÂ°\"", // " ,.o©º°\""
  "Â«Â«", // "««"
  "Â»Â»", // "»»"
  "<-.Â¸Â¸.Â·Â´Â¯", // "<-.¸¸.·´¯"
  "Â¯`Â·.Â¸Â¸.->", // "¯`·.¸¸.->"
  "03.;'``~<04@",
  "04@03>~``';.",
  "04Â¶09Â¦10+11) ", // "04¶09¦10+11) "
  " 11Â§10:09Â¬04>", // " 11§10:09¬04>"
  "`Â·.<Âº)))><.Â·Â´", // "`·.<º)))><.·´"
  "`Â·.><(((Âº>.Â·Â´", // "`·.><(((º>.·´"
  "3Â¤~Â»}4@3{Â«~Â¤", // "3¤~»}4@3{«~¤"
  "Â«Â¤Â´Â¤Â»Â¥Â«Â¤Â´Â¤Â»Â¥Â«Â¤Â´Â¤Â»", // "«¤´¤»¥«¤´¤»¥«¤´¤»"
  " KÃ«wÂ£ ", // " Këw£ "
  "Â»4Â¡Â«Â»10Â¡Â«Â»12Â¡Â«", // "»4¡«»10¡«»12¡«"
  "02,01 04\\01,04\\08\\04,08\\/08,04/01/04,01/01 ",

  "-",
  "Insert From Main Design-Window",
  "-",
  "(C) Color",
  "(F) Font Type",
  "(S) Font Size",
  "(B) Bold",
  "(I) Italics",
  "(U) Underline",
  "(O) Plain Text",
  "(P) Protected",
  "(p) Protected End",

  "-",
  "Optimize",

  "-",
  "Export To Text-File",
  "Import From Text-File",
  "-",
  "Save and Exit",
  "Quit",

  "View",

  "RTF (Color Text Edit)",
  "IRC (Codes Text Edit)",

  "Help",
};
//************************************************************************
const char* EDITPOPUPMENU1[TOTALEDITPOPUPMENU1] =
{
  "Undo All Wing-Edits",
  "-",
  "Add New Wing-Pair",
  "Import From Main Design Window",
  "-",
  "Delete Selected Wing-Pair",
  "Edit Selected Wing",
  "-",
  "Copy",
  "Paste",
  "-",
  "Clear Entire List",
};
//************************************************************************
const char* EDITPOPUPMENU2[] =
{
  "Clear",
  "Undo",
  "-",
  "Cut",
  "Copy",
  "Paste",
  "Delete",
  "-",
  "Select All",
  "-",
  "Toggle View Mode",
  "-",

  "Add Text Effect",

  "Color (Ctrl+K)",
  "Font Type (Ctrl+F)",
  "Font Size (Ctrl+S)",
  "Bold (Ctrl+B)",
  "Italics (Ctrl+R)",
  "Underline (Ctrl+U)",

  "-",
  "Save And Exit",
  "Quit Without Saving",
};
//************************************************************************
const char* COLORDIALOGMSG[] =
{
  "Vert Blend", //0
  "RGB Color", //1
  "Horiz Blend", //2
  "Random", //3
  "Transparent", //4
  "Cancel", //5
  "", //6
  "", //7
  "", //8
  "", //9
  "Foreground Color", // 10
  "Background Color", // 11
  "Effect: Alternating Foreground", // 12
  "Morph Colors", // 13
  "Replace Colors", // 14
  "", // 15
  "", // 16
  "Effect: Underline Color A", // 17
  "Effect: Underline Color B", // 18
};
//************************************************************************
const char* WEBEXPORTMSG[] =
{
  "", //0
  "Web-Page Top Margin (pixels)", // 1
  "Web-Page Left Margin (pixels)", // 2
  "Background Color", // 3

  "Click to select the background color\n"
  "for your new web-page...", // 4

  "Uncheck this if your are generating a full web-page.\n"
  "Check this to copy HTML to the Windows Clipboard.\n"
  "You can then paste HTML into other programs such as E-Mail.", // 5

  "Check this to use a web-link to a picture\n"
  "for the web-page we generate.", // 6

  "Check this to prevent the background image\n"
  "from scrolling along with the text.", // 7

  "Check this to include the author META tag!", // 8

  "Check this to include the title META tag!", // 9

  "Check this to generate a HOME page button!", // 10

  "Repeat: repeats image horizontally and vertically.\n"
  "Repeat-X: repeats image horizontally only.\n"
  "Repeat-Y: repeats image veritcally only.\n"
  "No-Repeat: displays the image as-is.\n\n"
  "(Note: If No-Repeat is set, the image pisition is\n"
  "set by the sliders at the top of this dialog 0%-100%)", // 11

  "Check this to force insertion of non-breaking spaces at the\n"
  "end of lines. This should only be needed for compatibility\n"
  "with older browsers that ignore the white-space style.", // 12

  "", // 13
  "", // 14
  "", // 15
  "", // 16

  "Use Title", // 17
  "Use Author's Name", // 18
  "Make Home Button", // 19
  "Insert Non-Breaking Spaces", // 20
  "White-Space Style", // 21

  "", // 22
  "", // 23
  "", // 24
  "", // 25

  "Line-Height: ", // 26
};
//************************************************************************
const char* EDITMSG[] =
{
  // Caption
  "Edit Wingdings", //0

  // Label5
  "Left", //1

  // Label6
  "Right", //2

  // Label7
  "Top and Bottom Borders (pattern auto-repeats)", //3

  // Label8
  "Left Side Border", //4

  // Label9
  "Right Side Border", //5

  // Label10
  "Border Background Color:", //6

  // TopBottomBorders
  "Enabled", //7

  // SideBorders
  "Sides Enabled", //8

  // Button1
  "&Save and Exit", //9

  // Button2
  "Character Map", //10

  // Button3
  "Font Editor", //11

  // Edit1->Hint
  "Press ENTER to exit and save.\n"
  "Press ESC to exit without saving.\n"
  "CTRL+T Toggles the View.\n"
  "Right-click for the pop-up menu.", // 12

  // LeftWingsListBox->Hint, RightWingsListBox->Hint (not in edit-mode)
  "Double-click an item to edit it.\n"
  "Right-click for the popup menu.\n"
  "Check the box to enable the wing-pair.\n"
  "Press the ESC key to abort.", //13

  // TopBottomEdit->Hint, LeftSideEdit->Hint, RightSideEdit->Hint
  "Left-click to Edit.", // 14

  // LeftWingsListBox->Hint, RightWingsListBox->Hint (in Edit-mode only)
  "Left-click to Edit.\n"
  "Right-click for the pop-up menu.", // 15

  // Spares
  "", //16
  "", //17
  "", //18
  "", //19

  // General messages...

  // Import instructions
  "To import, design a ONE-LINE wing in the MAIN WINDOW. Press\n"
  "PROCESS and add any effects you may want. In the wing-editor,\n"
  "click the box you want to import into (Left-Wings or Right-Wings),\n"
  "Click-right to pop up this menu and click this menu selection again.", //20

  "Error Importing Wings...", //21

  "Export Wings As Text", //22

  "Error Exporting Wings...", //23

  "Operation not allowed in this View!", //24
  "You must select text to perform this operation!", //25

  "Click on a border to edit it.\n\n"
  "Double-click on a wing to edit it.\n\n"
  "Right-click in the edit window to see\n"
  "the pop-up menu.\n\n"
  "Click the check-boxes to enable/disable\n"
  "individual wing-pairs.\n\n"
  "Click \"View\" to edit the raw IRC codes.\n\n"
  "Click \"Options\" to export/import wings,\n"
  "or to change the selected part of the\n"
  "wing's color, etc.\n\n"
  "You can also design a wing in the main\n"
  "YahCoLoRiZe window and then import it via\n"
  "\"Options->Insert From Main Design Window\".", //26

  "Wing-list is corrupt... repaired it", //27
  "Click or double-click an item to edit it...", //28

  // Spares
  "", //29

  // DO NOT TRANSLATE THE FOLLOWING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  "charmap.exe", //30
  "eudcedit.exe", //31

  // Spares
  "", //32
  "", //33
  "", //34
};
//************************************************************************
const char* REPLACEDLGMAINMENU[] =
{
  "Insert",

  "(C) Color",
  "(F) Font Type",
  "(S) Font Size",
  "(B) Bold",
  "(I) Italics",
  "(U) Underline",
  "(O) Plain Text",
  "(P) Protected",
  "(p) Protected End",
  "(N) Page Break",

  "Edit",

  "Clear",
  "Undo",
  "-",
  "Cut",
  "Copy",
  "Paste Color",
  "Paste Plain",
  "-",
  "Select All",
  "-",
  "Toggle View Mode",
  "-",
  "Optimize",

  "Help",
};
//************************************************************************
const char* REPLACEDLGPOPUPMENU[] =
{
"Clear",
"Undo",
"-",
"Cut",
"Copy",
"Paste Color",
"Paste Plain",
"-",
"Select All",
"-",
"Toggle View Mode",
"-",
"Optimize",
};
//************************************************************************
const char* REPLACEMSG[] =
{
"Find/Replace (selected text only)", // Window Titles 0
"Find (selected text only)",
"Find/Replace (entire document)",
"Find (entire document)",
"",
"Wrap Around", // Check Boxes 5
"Case Sensitive",
"Match Whole Word",
"",
"Find &Up", // Buttons 9
"Find &Down",
"&Replace",
"Replace &All",
"&Cancel",
"",
"Find", // Labels 15
"Replace With",
"Press the F3/F4 Keys to search Down/Up",
"",
"Text not found!", // Find return status 19
"Search complete!",
"Error in search!",
"",
"Replacing text...", // Misc 23
"Finished searching text.\n", // 24
" replacements made.", // 25
"Replace string has only format codes!", // 26
"", // 27
"Click the find or replace box to edit it...", // 28
//------------------------
// Help text (29)
"Press Find Down to search forward for text in the Find box.\n"
"Press Find Up to search backward for text in the Find box.\n"
"Press Replace to replace the text that was found with text "
"in the \"Replace With\" box.\n"
"Press \"Replace All\" to replace all occurences.\n\n"
"To find/replace over a particular area of a document, drag the "
"mouse over it to highlight it (it must be one or more complete "
"lines of text!). The Title-bar will now say \"(selected text "
"only)\"\n\n"
"1) Use the Insert menu to add colors and text-formatting. (First "
"click the box you want to insert into to select it!)\n"
"2) Right-click for the popup menu to copy/paste text, etc.\n"
"3) You can use (Shift+Ins)/(Shift+Del) to insert/delete text.\n"
"4) Left-click drag the mouse to select zones of text (or hold "
"the shift-key and press the arrow keys).\n\n"
"Example1: To Replace all occurences of a particular color:\n"
"a. In the main program click \"View\" and select \"IRC Codes\".\n"
"b. In the main program, drag the mouse over the color you want "
"to change (\"C#FF00FF\" for example), right-click Copy it "
"then right-click and \"Paste (Color)\" it into the Find box.\n"
"c. Now copy/paste a new color into the \"Replace With\" box or "
"use the Insert menu to add a color-code.\n"
"d. Edit the replace color-code(s) to your taste, (Hint: type a "
"dummy character after the color-code(s) and click View->Color "
"for a preview of its apperance, then you must go back to "
"View->Codes to remove the dummy character.)\n"
"e. Press \"Replace All\" and all C#FF00FF RGB colors will be "
"replaced with the new color.\n"
"f. In the main program click \"Show\" to see the new colors.\n\n"
"Example2: To search/replace multi-line text:\n"
"a. Select, say a paragraph of text in the main document and "
"right-click Copy it to the clipboard.\n"
"b. Deselect the text by clicking anywhere in the document so it "
"won't be mistaken as a search-zone.\n"
"c. Click Edit->Find to bring up this dialog and right-click "
"\"Paste Plain\" to add the paragraph's text.\n"
"d. Now you can click \"Find Up\" or \"Find Down\" to see if the "
"paragraph appears more than once.\n\n"
"NOTE: When the main view mode is \"Processed\", the Find text is "
"always stripped of any format-codes to perform the search, but "
"the Replace text is allowed to have format-codes. When the main "
"view mode is \"IRC Codes\" or \"Original\", both the Find and "
"the Replace text-boxes can have raw text-format codes.", // 29
//------------------------
// Toggle View Button
"View", // 30
"Toggle View", // 31
};
//************************************************************************
const char* SPELLINGMSG[] =
{
"Spelling Checker", // Window Title 0
"My Dictionary", // DlgShowDict.cpp
"Choose Dictionary", // DlgChooseDict.cpp
"",
"",
"",
"Language", // Buttons 6
"Replace",
"Replace All",
"Help",
"Ignore",
"Ignore All",
"Close",
"Add Word",
"Del Word(s)",
"",
"My Dictionary", // (custom dictionary) Sub-dialog caption 16
"Language", // (choose dictionary) Sub-dialog caption 17
"Delete Selected Items", // Sub-dialog buttons 18
"Help",
"Close",
"OK",
"",
"",
"",
"Max custom dictionary entries is: ", // Strings 25
"No misspellings found, edit Custom Dictionary?", // 26
"Unable to check spelling on this system...", // 27

"Unable to find dictionary. Do a web-search for \"OpenOffice Dictionaries\"\n"
"and unzip the .aff and .dic files for the dictionary you download into your\n"
"application data folder, typically \"C:\\Program Files (x86)\\\n"
"Discrete-Time Systems\\YahCoLoRiZe\\Dict\"", // 28

"", // 29

"No Spell-checker: Unable to locate hunspell.dll!\n"
"Try uninstalling then reinstalling YahCoLoRiZe...", // 30

"Add the current word to My Dictionary...", // 31
"Delete word(s) from My Dictionary...",
"Choose a HunSpell (Open Office) dictionary...",
"",
"Hunspell.dll", // HunSpell spell-checker DLL 35
"",
"",
"",
"",
"Hold the Ctrl Key down and click the items you want to delete...", // Hints 40
"",
"",
"",
"",
"",
// --------- Help text for DlgShowDict.cpp 46
"Select the word(s) you want to remove by holding the Ctrl key\n"
"and clicking them (or hold Shift and click to select a range\n"
"of words). Press the Del key or click \"Delete Selected Items\"\n"
"to remove the words from the list... Click \"Close\".",

// --------- Help text for DlgSpellCheck.cpp 47
"A misspelled word appears in the top edit-box with suggestions\n"
"appearing in the list-box to the left. Click one of the suggestions\n"
"or type in a new word.\n\n"
"Click \"Replace\" to replace one word or \"Replace All\" to replace all\n"
"of the same misspellings of that word throughout the entire\n"
"document. Likewise, you can click \"Ignore\" to continue the spell\n"
"check without replacing the word or \"Ignore All\" if the word is\n"
"fine as-is.\n\n"
"Click \"Language\" to bring up a list of available dictionaries.\n"
"Dictionaries each consist of two files, en_US.aff and en_US.dic\n"
"for the English dictionary, for example. You can place more\n"
"dictionaries in the following directory:\n\n"
"   \\Program Files\\Discrete-Time Systems\\YahCoLoRiZe\\Dict\n\n"
"Make sure you copy both files (.aff/.dic). Zip-files of available\n"
"dictionaries: http://archive.services.openoffice.org/pub/\n"
"mirror/OpenOffice.org/contrib/dictionaries/\n\n"
"Click \"Add Word\" to add a word to your own custom \"My Dictionary\".\n"
"Clicking \"Del Word\" will bring up a list-box which allows you to\n"
"select words to remove from \"My Dictionary\". Press and hold the\n"
"Ctrl key and click multiple words then press the Del key to delete\n"
"them.\n\n"
"Credits: This spelling-checker is made possible by the creators\n"
"of Hunspell as well as the Open Office Dictionaries. The DLL is\n"
"compiled by Chris from www.progui.co.uk. The dictionaries are from\n"
"openoffice.org. Hunspell is avalible at hunspell.sourceforge.net.",
};
//************************************************************************
const char* PLAYMSG[] =
{
  // UseDLL->Hint
  "Check this for mIRC/IceChat/Vortec to permit use of burst-\n"
  "interval without stripping out extra spaces (as in ASCII-Art).\n"
  "Also permits Pause/Resume playback in mIRC/IceChat/Vortec.", // 0

  // StripFont->Hint
  "Check this to strip font-tags for Yahoo. Lets you send\n"
  "a longer text-line, but you have to depend on the default\n"
  "font.", // 1

  // UseFile->Hint (for YahELite)
  "Check this to send text via YahELite's /textfile command.", // 2

  // UseFile->Hint (for mIRC/IceChat/Vortec)
  " UseFile  UseDLL                  Problem                         \n"
  "------------------------------------------------------------------\n"
  "   NO       NO    mIRC strips spaces, Don't use for ASCII Art     \n"
  "   NO      YES    GOOD. ALLOWS ASCII-ART, PAUSE/RESUME/BURST      \n"
  "  YES       NO    Good for ASCII-art but can't pause-play in mIRC \n"
  "  YES      YES    Problem with this, you can't use burst-play...  ", // 3

  // SendUtf8CheckBox->Hint
  "Check this to cause all text sent to your chat-client to be\n"
  "converted from ANSI to UTF-8", // 4

  // Spares
  "", // 5
  "", // 6
  "", // 7

  "Play channel was remotely changed to ", // 8
  "setting it back to ", // 9

  // AddButton->Hint
  "Add a new room using text in the edit-box...\n\n"
  "To enter a Unicode character, type its\n"
  "hex digits and press Alt+X", // 10

  "All rooms deleted... Press Cancel to Undo!", // 11

  // ChanEditKeyDown() - add
  "Added ", // 12
  " to room-list", // 13

  // NoSpaces->Hint
  "On Play, replace spaces with a unique\n"
  "character and make them invisible. This\n"
  "is for ASCII art when using mIRC.", // 14

  // NSEdit->Hint
  "This character will be used\n"
  "to form an \"invisible space\".", // 15

  // Rooms->Hint
  "Type in a room name (Ex: #Chatters) and press \"Enter\" to add\n"
  "it to the list. Click on the down-arrow to view the list.", // 16

  // DeleteButton->Hint
  "Delete a room. Shift-click\n"
  "to delete ALL rooms...", // 17

  "Deleted ", // 18
  " from room-list", // 19

  "You can't delete \"status\"!", // 20

  // Form's captions and lables...

  // Caption
  "Text-Playback Options", // 21

  // Label1
  "Time between line-bursts (in ms) =>", // 22

  // Label11
  "<= Burst-Interval (in ms):", // 23

  // Label2
  "Chat Rooms (Example \"#myroom\")\n"
  "(for most clients use \"status\" to test)", // 24

  // SendUtf8
  "Send As Utf-8", // 25

  // Label5
  "Temp file to write to for playback:", // 26

  // Label6
  "Max bytes to send", // 27

  // StripFont
  "Strip Font Tags", // 28

  // UseDLL
  "Use DLL", // 29

  // UseFile
  "Use File", // 30

  // LineBurst
  "Lines per burst", // 31

  // NoSpaces
  "Pad Spaces", // 32

  // ClearRoomsButton
  "Delete", // 33

  // ResetPathButton
  "Reset", // 34

  // Cancel Button
  "Cancel", // 35

  // PaddingLabel
  "Padding Character", // 36

    // AddButton
  "Add", // 37

  // Spares
  "", // 38
  "", // 39

  // SPECIAL!!!!!!!! DO NOT TRANSLATE!!!!!!!!!!!!!!!
  "status", // 40

  // Spares
  "", // 41
  "", // 42
  "", // 43
};
//************************************************************************
const char* FAVCOLORSMAINMENU[] =
{
  "File",

  "Import",
  "Export",
};
//************************************************************************
const char* FAVCOLORSDLG[] =
{
"Favorite Colors", // Window Title
"Select Color And Press OK", // Label
"Example", // List-box text
"Add Current",
"Add New",
"Delete",
"OK",
"Cancel",
};
//************************************************************************
const char* STRIPDLG[] =
{
"Strip", // Window Title
"Strip Codes:", // Radiogroup caption
"", // Spares
"",
"All Codes", // MUST REMAIN AT INDEX 4!!!!
"All  Codes Outside Push/Pop",
"Bold Codes",
"Underline Codes",
"Italics (or Reverse Video)",
"FG Colors",
"BG Colors",
"All Colors",
"All Font Codes",
"Push/Pop Codes",
"Ctrl-O",
"Tabs",
"Page Breaks",
};
//************************************************************************
const char* MAINPOPUPMENU[] =
// Main popup menu
{
"Clear",
"Original",
"Process",
"Play",
"-",
"Text state at Caret",
"Process SwiftMiX Song Title",
"-",
"Cut",
"Copy",
"Paste Color",
"Paste Plain",
"Delete",
"-",
"Select All",
};
//************************************************************************
const char* COLORCOPYPASTEMENU[] =
// Color paste menu
{
"Show Color",
"-",
"Copy Color",
"Paste Color",
"Paste Background Color",
};
//************************************************************************
const char* MAINMENU[] =
// Main Menu
{
"File", // FileMenu
"New...", // FileNewItem            "Clear the edit control"
"Open (UTF-8)", // FileOpenUtf8Menu "Open existing file as UTF-8"
"Open (ANSI)", // FileOpenAnsiMenu      "Open existing file as ANSI"
"Save...", // FileSaveItem          "Save file with no prompt"
"Save As...", // FileSaveAsItem     "Save current file under a new name"
"-",
"Import Blend Colors", // ImportBlendColors1
"Export Blend Colors", // ExportBlendColors1
"-",
"Export As Web-Page", // ExportAsWebPage1
"-",
"Page Setup...", // FilePageSetup
"Print Preview...", // FilePrintPreview
"Print...", // FilePrintItem
"-",
"Exit", // FileExitItem

"Tools", // ToolsMenu 17
"Emoticons", // InsertEmoteicon1
"Wing-Editor", // EditWingdings1
"-",
"Set Foreground Color", // MenuSetForegroundColor
"Set Background Color", // MenuSetBackgroundColor
"Set Wing Color", // MenuSetWingColor
"Set Window Color", // MenuSetWindowColor
"Change Color Palette", // MenuChangeColorPalette
"Favorite Colors", // MenuFavoriteColors
"-",
"Font", // Font1
"Character Map",
"Show Hex", // ShowHex1
"Reseed Random Num Gen", // RandomNumberGenerator1
"-",
"Strip Codes", // MenuStripCodes
"Trim Leading/Trailing Spaces", // Strip Leading/Trailing Spaces
"Adjust Trailing Spaces", // Adjust Trailing Spaces
"Join Lines In Paragraphs", // Join Lines In Paragraphs
"Optimize", // Optimize1

"Edit", // EditMenu 11
"Clear", // EditClearItem
"Undo", // Undo1
"-",
"Cut", // EditCutItem
"Copy", // EditCopyItem
"Paste Color", // EditPasteColor
"Paste Plain", // EditPaste
"Select All", // SelectAll1
"-",
"Find", // FindText1
"Replace", // ReplaceText1
"-",
"Spell Check", // EditSpellCheckItem

"View", // ViewMenu 10
"Original", // Original1
"Processed", // RTF1
"IRC Codes", // IRC1
"HTML", // HTML1
"RTF Source", // RTFSource1
"-",
"View In Browser", // Call ViewInBrowser1Click()
"-",
"Word Wrap", // WordWrap

"Play", // PlayMenu 7
"Start", // StartPlay1
"Stop", // StopPlay1
"Pause", // PausePlay1
"Resume", // Resume1
"-",
"Options", // SetPlay1

"Effects", // EffectsMenu 35

"Format Codes",
"Bold (B)",
"Underline (U)",
"Italics (I)",
"Protected (P)",
"Protected End (p)",
"Plain Text (O)",
"Color Code (C)",
"Font-Type (F)",
"Font-Size (S)",
"Page-Break (N)",

"Font",
"Type",
"Size",
"Mountain",
"Valley",
"Dip",
"Rise",
"Double Mountain",
"Huge Mountain",
"Random",

"Set New Colors", // ForegroundAndBackground1
"Replace Color",
"Morph Colors", // MorphColors1
"Alternate Colors", // AlternateForegroundBackground1
"Alternate Character", // Alternate Character
"Increment Colors", // IncrementFGBG
"Random Colors", // RandomForegroundBackground1
"FG Blend", // FgBlend
"BG Blend", // BgBlend
"-",
"Color Fractal", // GenerateColorFractal1

// To add a new client, add it to the enumerated types,
// tCli and tCliID, add it here (another entry below)
// and in Main.cpp add it to GetColorizeNetClient()
// and GetInternalClient()
"Client",
CLIENTS[0],
CLIENTS[1],
CLIENTS[2],
CLIENTS[3],
CLIENTS[4],
CLIENTS[5],
CLIENTS[6],
CLIENTS[7],
CLIENTS[8],

"Help", // HelpMenu 14
"Help", // Help1
"Web-Site", // wwwYahCoLoRiZecom1
"-", // N3
"ASCII Art Demo", // ASCIIArtDemo
"Text Demo", // TextDemo
"Text Demo (Wings)", // TextDemoWings
"Technical Doc.", // TechDemo
"-", // N12
"Restore Settings", // RestoreFactorySettings1
"-", // N11
"About...", // HelpAboutItem
};
//************************************************************************
const char* KEYSTRINGS[TOTAL_KEY_STRINGS] =
{
"License Days: ", //0
"(Invalid License)", //1

"Your trial-period has begun!\n"
"To enter a purchased license key, click\n\n"
"     Help->About->Install New Key\n\n"
"Thank You for trying my software!", //2

"There was a problem validating the license key!", //3
"Visit ", //4
" to buy this product!", //5

"Your need to obtain a License Key and enter\n"
"it in order to validate this product.", //6

"Hint: The first time you run this program,\n"
"you must have Administrator access rights!", //7

"Please set your calendar to the proper date!", //8

"Your activation has expired.\n Visit ", //9
" to renew!", //10

"You must enter the E-mail address\n"
"you used when requesting your License Key!", //11

"The E-mail address you entered is not the\n"
"same as that used to request your License Key!", //12

"Your computer's name may have changed since\n"
"the last time you used YahCoLoRiZe.  You will need to\n"
"re-install the license key! For information visit:\n\n", //13

"Discrete-Time Systems will not be held liable\n"
"for any damages whatsoever including indirect,\n"
"direct, consequential, incidental, special damages\n"
"or loss of business profits, even if Discrete-Time Systems\n"
"(Mister Swift) has been apprised of the possibility\n"
"of said damages.\n\n"
"If you AGREE, press Yes, otherwise, press No.", //14

"New License Key Entered Successfully!", //15

"YahCoLoRiZe has been successfully activated!\n"
"To enter a purchased license key, click\n\n"
"     Options->About->Install License Key\n\n"
"Thank You for trying my software!", //16

"Invalid Licence Key...\n"
"Operating in Restricted Mode, Some Features Unavailable...", //17

"Key you entered was not valid!", //18

"(Unlimited License)", //19
};
//************************************************************************
// Status messages displayed while busy processing
const char* STATUS[] =
{
  "Merging buffers...", // 0 Merging buffers
  "Loading string-list...",  // 1 Loading string-list
  "Processing to RTF format...", // 2 Processing to RTF format
  "Processing to IRC format...", // 3 Processing to IRC format
  "Adding text effect...", // 4 Adding text effect
  "Initializing text-effect...", // 5 Initializing text-effect
  "Stripping old formatting...", // 6 Stripping old formatting
  "Optimizing text...", // 7 Optimizing text
  "Processing to Yahoo format...", // 8 Processing to Yahoo format
  "Processing text-highlight chars...", // 9 Processing text-highlight chars
  "Processing to HTML format...", // 10 Processing to HTML format
  "Pasting new text...", // 11 Pasting new text
  "Creating font-list...", // 12 Creating font-list
  "Converting from HTML to IRC...", // 13 Converting from HTML to IRC
};
//************************************************************************
// Color blender tab
const char* BLENDSCOPEITEMS[] =
{
// BlendScopeRadioButtons
"Word", // 0
"Line", // 1
"Sentence", // 2
"Paragraph", // 3
"Entire Selection", // 4
};
//************************************************************************
const char* BLENDDIRECTIONITEMS[] =
{
// BlendDirectionRadioButtons
"====>>>>", // 0
"==>><<==", // 1
"<<====>>", // 2
"<<<<====", // 3
};
//************************************************************************
const char* TABITEMS[] =
{
// Tab-control
// If you change order, also change:
// TAB_PROCESS   0
// TAB_WIDTH     1
// TAB_MARGINS   2
// TAB_FORMAT    3
// TAB_BLENDER   4
// TAB_OPTIONS   5
// TAB_TABS      6
// in Main.h and change the TTabSheet PageIndex
"Process",
"Width",
"Margins",
"Format",
"Blender",
"Options",
"Tabs",
};
//************************************************************************
const char* FN[] =
{
"WM_YahELiteExt", // 0
"&Process", // 1
"YahELite:class", // 2
"Colorize.chm", // 3
"Discrete-Time Systems", // 4
"TDTSColor", // 5
"colorize.tmp", // 6
"Colorize.dll", // 7
"xircon", // 8
"command", // 9
"/play -eps", // 10
"/play -p", // 11
"/play stop", // 12
"BlendColors1.bin", // 13
"TDTSTrinity", // 14
"Colorize.ini", // 15
"smileys.txt", // 16
"Smileys\\", // 17
"Dict\\", // 18
"manual.txt", // 19
"asciiart.txt", // 20
"tech.txt", // 21
"Colorize.ico", // 22
"YahCoLoRiZe", // 23 OUR_NAME
"YahCoLoRiZe", // 24 OUR_TITLE
"colorize.htm", // 25
"BlendColors1.txt", // 26
"FavColors.txt", // 27
};
//************************************************************************
const char* DS[TOTALSTRINGS] =
{
// XX->Strings[0];
"",

// XX->Strings[1];
"RegisterWindowMessage failed!",

// ImportAsciiButton->Hint = XX->Strings[2];
"Left-click to IMPORT, right-click to EXPORT\n"
"plain ASCII text to selected preset (1-10).\n\n"
"Format is: \"BLEND:rrggbb rrggbb rrggbb...\"\n"
"(up to 10 RGB colors in hex)",

// XX->Strings[3];
"",

// CheckBox2->Hint = XX->Strings[4];
"Uncheck this to process text with \"effects\"\n"
"Then re-check it, switch View to IRC and now\n"
"re-process the text to add borders/wings.",

// PackLines->Hint = XX->Strings[5];
"Check to limit paragraph separation to 1 line.",

// CheckBox1->Hint = XX->Strings[6];
"Uncheck this to keep the original line-breaks.\n"
"Check this to squeeze the text together.",

// LeftJustify->Hint = XX->Strings[7];
"Check to align text to left-margin.",

// ExtendPalette->Hint = XX->Strings[8];
"Check this to extend the number of default colors beyond 16\n"
"for future IRC networks and clients. You can edit the\n"
"extended colors in the Colorize.ini file.",

// Process->Hint = XX->Strings[9];
"Click-Right on top web-site\n"
"To Set the Colors you see at the top.",

// BlendScopeRadioButtons->Hint = XX->Strings[10];
"Select the scope of the blend within a selected block of text.",

// BlendDirectionRadioButtons->Hint = XX->Strings[11];
"Select the direction of blending.",

// RGBThresholdEdit->Hint = XX->Strings[12];
"Set higher to avoid sending too much color-data.",

// XX->Strings[13];
"Delete all installed files?",

// ResolveToPaletteCheckBox->Hint = XX->Strings[14];
"Check this to keep WYSIWYG view accurate.",

// ColorBlenderPresetPanel->Hint = XX->Strings[15];
"Left-Click to Load\n"
"Right-Click to save Preset.",

// ImportButton = XX->Strings[16];
"Left-click to IMPORT all presets.\n"
"Right-click to EXPORT all presets.",

// Borders = XX->Strings[17];
"Sets top/bottom borders on if checked.",

// Wingdings = XX->Strings[18];
"Sets all wingdings on if checked.",

// AutoDetectEmotes = XX->Strings[19];
"Detects emotion-codes if checked.",

// PlayOnCR->Hint = XX->Strings[20];
"Check to cause an auto-press of\n"
"Process and auto-play into a room\n"
"when you hit the Enter key.",

"RGB colors are not allowed for IRC chat-clients.\n"
"Try selecting Client->Trinity to enable RGB colors...", // 21

// AddColorCheckBox->Hint = XX->Strings[22];
"Check to add color to text on Process.",

"", // 23

// TAlternateForm Caption (used for random and alternate text effects)
"Alternate Colors", // 24
"Randomize Colors", // 25

"", // 26

// ProcessButton->Hint = XX->Strings[27];
"Convert the highlighted text to a\n"
"color format used in chat-clients.",

// PlayButton->Hint = XX->Strings[28];
"Press to play text into the chat-room.",

// StopButton->Hint = XX->Strings[29];
"Press to stop text-play.",

// ResumeButton->Hint = XX->Strings[30];
"Press to resume text-play.",

// ForegroundColorPanel->Hint = XX->Strings[31];
"Left-Click to Set Foreground Color\n"
"Right-Click to display RGB color-codes.",

// BackgroundColorPanel->Hint = XX->Strings[32];
"Left-Click to Set Background Color\n"
"Right-Click to display RGB color-codes.",

// WingColorPanel->Hint = XX->Strings[33];
"Left-Click to Set Wing Color\n"
"Right-Click to display RGB color-codes.",

// PlayOnCX->Hint [34]
"Check to receive and process remote text\n"
"(as with IceChat's /CX command).",

"Thanks for trying my software!", // 35
"Open File", // 36
"Conversion failed", // 37
"Can't load file...", // 38
"Save File", // 39
"No filename specified...", // 40
"Conversion failed", // 41
"Can't save file...", // 42

"Select: Tools->Edit Wingdings, then click \"Sides\"\n"
"or \"Top and Bottom Borders\".", // 43

"Select: Tools->Edit Wingdings, then\n"
"click the wingding checkboxes you prefer.", // 44

"There is no text to process.", // 45

"Auto-set everything for Paltalk?\n"
"(Left-justify, auto-linesize, no-margins,\n"
"Tahoma font, no-borders, BG-Color is white)", // 46

"Press to play text into the chat-room.\n"
"(hold SHIFT down and click to send text\n"
"to Paltalk without posting into room.)", // 47

// Margins On/Off Checkbox Hint
"Check this to turn all text margins ON.\n"
"Uncheck to turn all text margins OFF.", // 48

"WARNING: You cannot edit any text in word-wrap mode!", // 49

"After Optimizing you may not be able to Undo previous\n"
"changes. Optimize?", // 50

"", // 51
"Unable to process:", // 52
"", // 53
"Text processing aborted by user", // 54
"Could not replace RTF Color-table!", // 55
"Could not locate RTF Color-table!", // 56
"Text processing error in RTF conversion", // 57
"", // 58
"Unknown thread return-code", // 59
"exception in SkipSmileys", // 60
"Auto-line width. Increased Width to: ", // 61
"Auto-line width. Changed Margin to: ", // 62
"Foreground Color", // 63

"Warning: you have set foreground color\n"
"the same as the background color!", // 64

"Background Color", // 65

"Warning: you have set background color\n"
"the same as the foreground color!", // 66

"Warning: you have set background color\n"
"the same as the wing color!", // 67

"Wing Color", // 68

"Warning: you have set wing color\n"
"the same as the background color!", // 69

"Press ESC to quit...", // 70

"Warning: IRC chat can't handle the custom\n"
"codes YahCoLoRiZe uses. Turned off Blend Before\n"
"Process and Blender controls...", // 71

// TUnderlineForm Caption (used for alternating character text effects)
"Alternate Character", // 72

"Changing the font requires changing to the\n"
"Original text view and reprocessing. Any changes\n"
"you have made will be lost. Continue?", // 73

// Help for Replace Color
"Replace Color is handy if you have existing text with effects already\n"
"added but you just need to change one color to another in the selection\n"
"or in the entire document. Unlike Set New Colors, Replace Color leaves\n"
"all other pre-existing colors intact.\n\n"
"On entry, the initial From and To colors are set the same as the colors\n"
"at the start of the selection. You can right-click a color to copy/paste\n"
"it from one box to the other. Left-click a color box to change its color.\n\n"
"*Hint: Before you select the text you want to change the color in, you can\n"
"copy colors to use from anywhere in the document by selecting some of its\n"
"text and chosing right-click Copy. Then you can select your zone to replace\n"
"colors in, choose Effects->Replace Color and right click Paste the copied\n"
"foreground or background color into the To or From color-boxs. :-)", // 74

"Go to the Client Tab and choose Trinity or\n"
"YahELite to access color blending!", // 75

"In IRC chat you can only set the Bold, Underline, Italics\n"
"font attributes. Set the font color by clicking the color-panels\n"
"in the Process tab... Italics are REVERSE-VIDEO in IRC!", // 76

"Largest number of chars to select is 160!", // 77
"You must select text to use this function!", // 78

"You are pasting text that contains background color\n"
"into text with no background color defined.\n\n"
"Remove background colors from the text you are pasting?", // 79

"There is nothing to print!", // 80

"Printer Not Found... Check to be sure a\n"
"default printer is set for your computer", // 81

"Random Number Generator re-seeded!", // 82

// Classname for YahELite
"", // 83

"Text too long. Use clipboard?", // 84

"", // 85

"Replaced ", // 86
" misspelled word(s)", // 87

"Deleted local settings but you must be an administrator\n"
"to delete some settings", // 88

"YahCoLoRiZe settings deleted!...Exiting", // 89

"Uncheck \"Auto-Blend\"\n"
"(Format Tab) to Enable", // 90

"Width", // 91

"Default Settings Were Restored!", // 92

"I set this version of my program to reset all of\n"
"the YahCoLoRiZe settings. Sorry, but this time it\n"
"could not be helped ;) - dxzl", // 93

"Old Version Detected! Save old settings?", // 94
"Error in : ", // 95
"Could not save registry values", // 96
"Line Count", // 97
"Play Width", // 98

"RegOpenKeyEx: Error reading security code from registry!\n"
"You need to re-install YahCoLoRiZe...", // 99

"Left-click for line-count...", // 100

"WARNING!!! This program (colorize.exe) is not the version\n"
"originally installed.  Cannot Verify authenticity of .EXE file!", // 101

"Unable to validate security-code,\n"
"are you using an old version of windows?", // 102

"Unable to compute security code!", // 103
"Security Code: ", // 104

"(Compare this code to the security-code on my web-site.\n"
"The codes should be identical.  If not, the version you\n"
"have may have been altered, or the version on the web-site\n"
"was changed by someone without authority to do so!)", // 105

"Unable to open: ", // 106

"", // 107
"", // 108
"", // 109
"", // 110

"Restart your chat-client, then try Play again...or you may\n"
"need to check \"PlayOnCX\" in the Options tab ;)", // 111

"You need to type a command into YahELite to play your\n"
"text into the room.  In YahELite type the following\n"
"command, and then hit Enter:\n\n"
"/extend TDTSColor YahCoLoRiZe\n\n"
"After this, the message \"Extension Active\"\n"
"should show in the chat-window.  Then Press the green\n"
"play button again.", // 112

"Warning: playing this into room will send"
"raw characters. You may want to press\n"
"\"PROCESS\" first. Cancel play?", // 113

"is over the limit set in Play->Options!\n\n"
"Click View->Original and try re-processing...\n"
"or set \"Width\" to a smaller number...\n"
"or INCREASE the RGB Threshold number in the Blend Tab.", // 114

"Line", // 115

"There is nothing to play... ^_^", // 116
"Error preparing text for playback...", // 117

"", // 118

" Make sure both of the files Colorize.dll and Xtcl.dll\n"
"are in the same directory as YahCoLoRiZe.exe", // 119

"", // 120
"", // 121
"", // 122
"", // 123
"", // 124

"Error sending play command to client", // 125
"Cannot find: ", // 126
"No pause function: ", // 127

"Change of client reguires clearing all text.\n"
"Press OK to clear.", // 128

"No resume function: ", // 129

"Rendering this much text will take time.\n"
"Keep going?", // 130

"Cannot load: ", // 131
"Cannot find ColorStop()", // 132

"", // 133

"Adding an effect is not allowed in this View!", // 134

"Thank you for trying YahCoLoRiZe!\n\n"
"To delete the YahCoLoRiZe icons,\n"
"right-click them and choose Delete!\n\n"
"Visit: www.yahcolorize.com!", // 135

"Uninstall YahCoLoRiZe?", // 136
"User-cancelled operation!", // 137

"An error occured in text-processing", // 138
"No text was selected, select some!", // 139

"Selection overlaps other effects...\n"
"This message happens when you have a\n"
"protect-zone with unmached PUSH/POP\n"
"codes embedded in a text-selection.", // 140

"Unknown processing return-code", // 141

"Print Setup", // 142
"Print", // 143

"", // 144
"", // 145
"", // 146

"Border edges will not be even\n"
"after Processing. Turn off Borders?", // 147

"Border edges will not be even"
" after Processing. Abort?", // 148

"BlendColors object is missing...", // 149

"Error code: ", // 150
"Error reading file...", // 151

"You must have a minimum of TWO blend-colors enabled! To enable,\n"
"click the Blender tab and left-click an X color panel to select it.\n"
"Then right-click on the panel to enable it.", // 152

"Problem occured in \"CreateBlendEngine()\"", // 153

"Preset colors were changed\n"
"without saving. Save them now?", // 154

"", // 155

// SaveDialog->Title = XX->Strings[156];
"Export Blender Colors", // 156

"", // 157

"Can't save file...", // 158

// SaveDialog->Title = XX->Strings[159];
"Export ASCII to Current Preset", // 159

"File exists, overwrite?", // 160
"Unable to open file for color-export", // 161
"Unable to export colors", // 162

// OpenDialog->Title = XX->Strings[163];
"Import ASCII to Current Preset", // 163

"Unable to open file for color-import", // 164
"Unable to import colors", // 165

// OpenDialog->Title = XX->Strings[166];
"Import Blender Colors", // 166

"Sorry, file not readable...", // 167

"Restore Defaults?\n\n"
"Before restoring defaults you may want to export\n"
"your custom wings and/or blend-colors... Are you\n"
"sure you want to restore defaults?", // 168

"This preset is empty...\n"
"click right to save displayed colors.", // 169

"YahCoLoRiZe can display a .rtf file but cannot convert\n"
"it to the native IRC code format. YahCoLoRiZe works with\n"
"normally. There is no converter at this time!", // 170

"Saved to preset ", // 171

"Warning: un-checking this will give you pure RGB; however,\n"
"you will lose the WYSIWYG accuracy! Are you Sure?", // 172

"Setting RGB threshold values lower than ", // 173
" may\ncause text-length to be too great for chat-room usage...", // 174

// Border-color dialog
"Border Background Color", // 175
"Use full RGB spectrum?", // 176
"Revision: ", // 177
"Forground color is illegal: ", // 178
"Background color is illegal: ", // 179
"Undo last \"Process\"?", // 180
"", // 181

"Insert Emoticon Shortcut...", // 182

"", // 183
"", // 184
"", // 185
"", // 186
"", // 187
"", // 188
"", // 189
"", // 190
"", // 191
"", // 192
"", // 193
"", // 194
"", // 195

// SwiftMixSync Hint
"Check this to receive SwiftMix playback timing information.", // 196

// Blender-Tab Hint "Blender->Hint" (Used for color-buttons)
"Left-Click: Select a  color-panel\n"
"Left-Double-Click: Choose the panel's color\n"
"Right-Click: Toggle the selected color-panel ON/OFF\n" // 197
"Wheel-Up/Down: Dim or Brighten selected color\n"
"Wheel-Click: Restore original brightness",

"Right-click in title-bar to set the colors you see!", // 198

// Checkbox-Captions...
// LeftJustify->Caption
"Left Justify", // 199
// PackText->Caption
"Pack Text", // 200
// not used
"", // 201
// PackLines->Caption
"Pack Lines", // 202
// Borders->Caption
"Borders", // 203
// Wingdings->Caption
"Wings", // 204
// DetectEmotes->Caption
"Detect Emotes", // 205
// PlayOnCR->Caption
"PlayOnCR", // 206
// SwiftMixSync->Caption
"SwiftMiX Sync", // 207
// ExtendPalette->Caption
"Extend Palette", // 208
// ImportBlenderLabel->Caption
"Import/Export Blender", // 209
// RGBThresholdLabel->Caption
"RGB Threshold", // 210
// Text for the random color rectangle that appears in the main menu-bar
"Right-Click Me!", // 211

"OK to clear the text and a change to a new\n"
"client-mode?", // 212

"Select Font-Type", // 213
"Select Font-Size", // 214

"", // 215
"", // 216
"", // 217
"", // 218
"", // 219

// WingColorLabel->Caption
"Wings", // 220

"This preset is empty...", // 221

"Cannot find...", // 222
"Please reinstall YahCoLoRiZe...", // 223
"Client: ", // 224

// PlayOnCX->Caption
"PlayOnCX", // 225

"Corrupt file...", // 226

"Preparing to uninstall YahCoLoRiZe, If a Windows prompt\n"
"requests permission to run the Microsoft Command Shell\n"
"click \"Yes\" and grant it permission to run; otherwise,\n"
"the Uninstall will not be compleated...", // 227

"Unsaved work, close anyway?", // 228

"Transparent foreground text won't properly render to HTML, allow anyway?", // 229

"", // 230
"", // 231

"Tab Width", // 232
"Convert Tabs To Spaces", // 233

"Checking this will insert spaces rather than tab"
"characters when you press Tab", // 234

"Flatten Tabs On Process", // 235

"Select this to replace tabs with equivalent spaces\n"
"when processing text.", // 236

"Strip Tabs On Process", // 237

"Select this to replace tabs with a single space\n"
"when processing text.", // 238

"", // 239
"", // 240
"", // 241
"", // 242

// AsIsButton Hint and Label
"&Show", // 243 Button Label

"Renders the text \"As Is\" with no borders, wings\n"
"or additional text-formatting of any kind.\n\n"
"(Protected regions are shown in STRIKETHROUGH!)", // 244 Hint

// Auto-linewidth (Width tab)
"Auto", // 245

"Sets line-length to length of longest\n"
"line of text", // 246

// Width of text plus wings (Width tab)
"Line Width", // 247
"Length of a text-line. This includes wings,\n"
"but does not include the margin or side-borders", // 248

// Left margin (Width tab)
"Left", // 249
"Amount of space between left side border\n"
"and text", // 250

// Indent (Width tab)
"Indent (+/-)", // 251
"Paragraph indent (set negative to make\n"
"a numbered-list)", // 252

// Right margin (Width tab)
"Right", // 253
"Amount of space between right side border\n"
"and text", // 254

// Auto-lineheight (Height tab)
"Auto", // 255
"Height of boxes is by paragraphs", // 256

// Lines per box (Height Tab)
"Height", // 257
"Lines of text per box: 0 = OFF", // 258

// Spacing between boxes (Height Tab)
"Spacing", // 259
"Spacing between boxes: 0 = single-divider", // 260

// # blank lines at top (Height Tab)
"Top", // 261
"Number of blank lines between top border\n"
"and text", // 262

// # blank lines at bottom (Height Tab)
"Bottom", // 263

"Number of blank lines between bottom\n"
"border and text", // 264

// Margins On/Off Checkbox
"On/Off", // 265
};
//---------------------------------------------------------------------------
const wchar_t* HTMLCODES[] =
{
  L"&amp;",
  L"&#39;",
  L"&quot;",
  L"&nbsp;",
  L"&gt;",
  L"&lt;",
  L"&cent;",
  L"&curren;",
  L"&divide;",
  L"&copy;",
  L"&micro;",
  L"&pound;",
  L"&euro;",
  L"&yen;",
  L"&trade;",
  L"&sect;",
  L"&reg;",
  L"&para;",
  L"&middot;",
  L"&plusmn;",
  L"&iexcl;",
  L"&sect;",
  L"&brvbar;",
  L"&uml;",
  L"&ordf;",
  L"&laquo;",
  L"&not;",
  L"&macr;",
  L"&deg;",
  L"&sup2;",
  L"&sup3;",
  L"&acute;",
  L"&cedil;",
  L"&sup1;",
  L"&ordm;",
  L"&raquo;",
  L"&frac14;",
  L"&frac12;",
  L"&frac34;",
  L"&iquest;",
  L"&times;",
};
//---------------------------------------------------------------------------
// The index of the char in this string looks up its HTML replacement
// string in the table above.
const wchar_t HTMLCHARS[] = L"&\'\",><¢¤÷©µ£€¥™§®¶·±¡§¦¨ª«¬¯°²³´¸¹º»¼½¾¿×";
//---------------------------------------------------------------------------
