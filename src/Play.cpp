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

#include "Play.h"
#include "Playback.h"
#include "Paltalk.h"
#include "Utils.h"
//#include "TaeRichEdit.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// Playback Functions
//---------------------------------------------------------------------------
__fastcall TPlay::TPlay(TComponent* Owner)
// constructor
{
  this->dts = static_cast<TDTSColor*>(Owner);

  SetClassAndWindName();

  // Properties Playback.cpp can read to get a line's attributes
  lineWidthInChars = 0;
  lineWidthInBytes = 0;
}
//---------------------------------------------------------------------------
void __fastcall TPlay::SetClassAndWindName(void)
{
  // Check first to see if we have the class and name for the plugin as read from
  // out .ini file - if not, go with the defaults...
  if (DTSColor->PluginClass->Count < NUMBER_OF_PLUGINS_IN_INI_FILE ||
                  DTSColor->PluginName->Count < NUMBER_OF_PLUGINS_IN_INI_FILE)
  {
    if (DTSColor->IsCli(C_YAHELITE))
    {
      Class = LowerCase(FN[2]); // "YahELite:class", FN[2]
      Name = "YahELite";
    }
    else if (dts->IsCli(C_ICECHAT))
    {
//      Class = "WindowsForms10.Window.8.app.0"; // This works (used ws32.exe!)

      // Run spyxx.exe in "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools"
      // Choose "Search->Find Window" then drag the tool onto IceChat's upper border.
//      Class = "WindowsForms10.Window.8.app.0.378734a";
      Class = "WindowsForms10.Window.8.app.*";

//      Class = "FormMsgPump"; // This won't work finding a .NET app!
      Name = "IceChatMsgPump";
    }
    else if (dts->IsCli(C_HYDRAIRC))
    {
      Class = "CHydraColorizeMsgPump";
      Name = "HydraMsgPump";
    }
    else if (dts->IsCli(C_LEAFCHAT))
    {
      Class = "SunAwtFrame"; // Fixed class name used by Java 6 on...
      Name = "ColorizeMsgPump";
    }
    else // Trinity
    {
      Class = String(DS[181]); // "TDTSTrinity", DS[181]
      Name = "Trinity";
    }
  }
  else if (dts->IsCli(C_YAHELITE))
  {
    Class = dts->PluginClass->Strings[0]; // LowerCase(FN[2]); // "YahELite:class", FN[2]
    Name = dts->PluginName->Strings[0]; // "YahELite";
  }
  else if (dts->IsCli(C_ICECHAT))
  {
    Class = dts->PluginClass->Strings[1]; // "WindowsForms10.Window.8.app.0"; // This works (used ws32.exe!)
    Name = dts->PluginName->Strings[1]; //"IceChatMsgPump";
  }
  else if (dts->IsCli(C_HYDRAIRC))
  {
    Class = dts->PluginClass->Strings[2]; // "CHydraColorizeMsgPump";
    Name = dts->PluginName->Strings[2]; //"HydraMsgPump";
  }
  else if (dts->IsCli(C_LEAFCHAT))
  {
    Class = dts->PluginClass->Strings[3]; // "SunAwtFrame"; // Fixed class name used by Java 6 on...
    Name = dts->PluginName->Strings[3]; //"ColorizeMsgPump";
  }
  else // Trinity
  {
    Class = dts->PluginClass->Strings[4]; // String(DS[181]); // "TDTSTrinity", DS[181]
    Name = dts->PluginName->Strings[4]; //"Trinity";
  }
}
//---------------------------------------------------------------------------
// Way to find the HydraIRC chat-client (from its author's source-code...)
// (for now I just will use FindWindow(NULL, "HydraIRC")
// HydraIRC's plugin DLLs are just loaded to call a particular procedure
// then unload, so there's not a way to run a message-loop to look for
// our text-sends... and he is already using WM_COPYDATA in his main window's
// message-pump to remotely open a new IRC server. It looks like he not only
// finds the handle of the other instance of his app, but also SETS the
// data pointed to by lParam of the other app's GetMessage() to OUR handle...

//static const UINT WMU_WHERE_ARE_YOU =
//         ::RegisterWindowMessage(_T("WMU_HYDRAIRC_WHERE_ARE_YOU_MSG"));

// Enumerate windows, sending each one a custom message "where are you"
// until one responds... then we have its handle.
//HWND hOther = NULL;
//EnumWindows(searchOtherInstance, (LPARAM)&hOther);

//if(hOther != NULL)
//{
//}

//BOOL CALLBACK searchOtherInstance(HWND hWnd, LPARAM lParam)
//{
//  DWORD result;
//  LRESULT ok = ::SendMessageTimeout(hWnd, WMU_WHERE_ARE_YOU, 0, 0,
//    SMTO_BLOCK | SMTO_ABORTIFHUNG, 5000, &result);
//  if(ok == 0)
//    return TRUE;
//  if(result == WMU_WHERE_ARE_YOU) {
//    // found it
//    HWND *target = (HWND *)lParam;
//    *target = hWnd;
//    return FALSE;
//  }
//  return TRUE;
//}
//---------------------------------------------------------------------------

// This was tricky to get working for Borland... header file flukes:
// 1) Have to put the callback prototype here. 2) Have to
// cast (WNDENUMPROC) when calling EnumWindows...
//BOOL CALLBACK enumWinProc(HWND hwnd, LPARAM lParam);
//
///* The EnumChildProc callback */
//BOOL CALLBACK enumWinProc(HWND hwnd, LPARAM lParam)
//{
//  char class_name[80];
//  char title[80];
//  GetClassName(hwnd,class_name, sizeof(class_name));
//  GetWindowText(hwnd,title,sizeof(title));
//
//  dts->TaeEdit->Lines->Add(String(title) + "," + String(class_name));
//  return TRUE; // must return TRUE; If return is FALSE it stops the recursion
//}
//
// Put this in some function...
// EnumWindows((WNDENUMPROC)enumWinProc, 0);    // Call EnumWindows!
//---------------------------------------------------------------------------
HWND __fastcall TPlay::FindClient(HWND h)
// Return handle if one exists and the one we pass in is no good
{
  // Look for the class first then try the window-name...
  // FindWindow looks for the class name or atom created with RegisterClass
  // or RegisterClassEx. If it can't find that it looks for the window's title
  // text... (Hydra is a special case - we have the thread ID in GReplyTo)
  //ShowMessage(String(h));

  try
  {
    if(!h || !IsWindow(h))
    {
      HWND out;

      if ((out = FindWindow(Class.c_str(), Name.c_str())) != NULL)
        return out;

      if ((out = FindWindow(Class.c_str(), 0)) != NULL)
        return out;

      if ((out = FindWindow(0, Name.c_str())) != NULL)
        return out;

      return 0; // Can't find client
    }

    return h;
  }
  catch(...)
  {
    return h;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TPlay::SendToClient(bool bFindLineLength)
// if bFindLineLength true returns length of play-line
// else returns 0
//
// Playback object is instantiated each time timer1 times out
// in Main.cpp.  This method is called.
//
// This is also called in a continuous loop to get the max line-width
{
  try
  {
    // init properties...
    lineWidthInBytes = 0;
    lineWidthInChars = 0;

    if(dts->GPlaySize <= 0 || !dts->GPlayBuffer ||
                              dts->GPlayIndex >= dts->GPlaySize)
    {
      DoStopPlay();
      return true;
    }

    if (dts->IsWmCopyData()) // WM_COPYDATA Client?
    {
      // FindWindow looks for the class name or atom created with RegisterClass
      // or RegisterClassEx. If it can't find that it looks for the window's title
      // text...
      if(!dts->GReplyTo || !IsWindow((HWND)dts->GReplyTo))
      {
        // Look for the class first then try the window-name...
        if (((HWND)dts->GReplyTo = FindWindow(Class.c_str(), 0)) == NULL)
          if (((HWND)dts->GReplyTo = FindWindow(0, Name.c_str())) == NULL)
          {
            DoStopPlay();
            return true;
          }
      }
    }

    WideString OutStrW = "";

    // build new reply string
    while(dts->GPlayIndex < dts->GPlaySize)
    {
      if (dts->GPlayBuffer[dts->GPlayIndex] == C_CR ||
         dts->GPlayBuffer[dts->GPlayIndex] == C_LF)
      {
        // skip CR/LF
        while (dts->GPlayIndex < dts->GPlaySize &&
              (dts->GPlayBuffer[dts->GPlayIndex] == C_CR ||
                        dts->GPlayBuffer[dts->GPlayIndex] == C_LF))
          dts->GPlayIndex++;

        break;
      }

      // Don't add a NULL
      if (dts->GPlayBuffer[dts->GPlayIndex] == C_NULL)
      {
        // force index to end to force stopplay
        dts->GPlayIndex = dts->GPlaySize;
        break;
      }

      if (dts->IsCli(C_XIRCON))
      {
        // XiRCON thinks braces are a tcl command :-(
        if (dts->GPlayBuffer[dts->GPlayIndex] == '[' ||
                                  dts->GPlayBuffer[dts->GPlayIndex] == ']')
          OutStrW += WideString(L"*"); // Substitute...
        // XiRCON thinks $# is a tcl command :-(
        else if (dts->GPlayIndex+1 < dts->GPlaySize &&
                      dts->GPlayBuffer[dts->GPlayIndex] == '$' &&
                                 iswdigit(dts->GPlayBuffer[dts->GPlayIndex+1]))
          OutStrW += WideString("($)"); // Substitute...
        else
          OutStrW += dts->GPlayBuffer[dts->GPlayIndex];

        dts->GPlayIndex++;
      }
      else
        OutStrW += dts->GPlayBuffer[dts->GPlayIndex++];
    }

    // Getting extra line sent to YahCoLoRiZe if at end of
    // buffer and empty line so stop play 1/2008
    if (dts->GPlayIndex >= dts->GPlaySize && OutStrW.Length() == 0)
    {
      if (!bFindLineLength)
        DoStopPlay(); // don't call if timer not even running yet

      return true;
    }

    // Set useful property - width in chars, not bytes
    // (With PalTalk, the final text output to the client is RTF and font codes
    // and background colors will be stripped and foreground colors are
    // totally different than the raw IRC-format we have at this point,
    // so just report the stripped width of each line in characters...)
    if (dts->IsCli(C_PALTALK) || dts->IsCli(C_YAHELITE))
      lineWidthInChars = utils->GetRealLength(OutStrW);
    else
      lineWidthInChars = OutStrW.Length();

    String OutStr; // ANSI/UTF-8 string...

    // Convert to UTF-8 if enabled...
    if (lineWidthInChars > 0 && dts->SendUtf8)
      OutStr = utils->WideToUtf8(OutStrW); // Convert UTF-16 to UTF-8
    else
      OutStr = String(OutStrW); // Convert UTF-16 to ANSI

    // set property with the width in bytes...
    if (dts->IsCli(C_PALTALK) || dts->IsCli(C_YAHELITE))
      lineWidthInBytes = lineWidthInChars;
    else
      lineWidthInBytes = OutStr.Length();

    // Can the chat-network handle this much text in one post?

    // NOTE: In Playback.cpp we do a "fake playback" of all the text
    // in the buffer, keeping track of the largest playback line
    // and printing a warning to the user about the first line that
    // exceeds the limit set in DlgPlay.cpp. Just above you see where we
    // abort prior to actual playback and return the length of each line...
    // Read the lengths as properties of this class...

    // Return line-length ONLY?
    // don't do stop-play because if this flag is set the timer
    // is not even being used as yet.
    if (bFindLineLength)
      return true;

    // Still need to check for buffer overrun in case we just call this
    // function without any pre-analysis via setting bFindLineLength...
    if (dts->GMaxPlayXmitBytes >= 0 && lineWidthInBytes > dts->GMaxPlayXmitBytes)
    {
      DoStopPlay();
      utils->ShowMessageU(DS[84]); // text too long
      return true;
    }

    if (lineWidthInBytes > 0)
    {
      if (dts->IsCli(C_YAHELITE))
      {
        OutStr = OutStr.Insert("`", 1); // Backtick tells YahELite to use only our colors
        OutStr += String(CTRL_Z); // Add CTRL Z to send into yahoo chat-room
      }
      else // NOTE does not work for mIRC or Vortec, need PadSpaces algorithm
      {
        // Add a CTRL_K if the last char in the string is a space.
        // This will force the client to post the trailing spaces
        // in a text-line. (works for IceChat at least!)
        if (OutStr[OutStr.Length()] == C_SPACE)
          OutStr += CTRL_K;
      }

      bool bIsStatusRoom;

      // Go through some conversion shinanigans to see if this room is the
      // "status" window...
      if (dts->PlayChan.LowerCase() == STATUSCHAN)
        bIsStatusRoom = true;
      else
        bIsStatusRoom = false;

      String sTempPlayChan;

      if (dts->bSendUtf8)
        sTempPlayChan = dts->PlayChan;
      else
        sTempPlayChan = utils->Utf8ToAnsi(dts->PlayChan); // ANSI

      // Send the text to the chat-client
      if (dts->IsPlugin())
      {
        // Insert /msg channel/nick at beginning
        if (bIsStatusRoom) // "status" Window play
          OutStr = OutStr.Insert("/echo ", 1);
        else
          OutStr = OutStr.Insert("/msg " + sTempPlayChan + " ", 1);

        SendColorizeNet(sTempPlayChan, OutStr, dts->RWM_ColorizeNet);
      }
      else if (dts->IsCli(C_YAHELITE))
        SendYahELite(OutStr);
      else if (dts->IsCli(C_PALTALK))
        SendPaltalk(sTempPlayChan, OutStr);
      else
      {
        // Can use DLL to send for Vortec, mIRC and XiRCON
        tColorStart pColorStart = NULL;

        if (dts->bUseDLL)
        {
          // Get custom dll's entry point
          pColorStart = LoadDll(dts->hDll);

          if (!pColorStart)
          {
            DoStopPlay();
            return true;
          }
        }

        if (dts->IsCli(C_MIRC))
        {
          if (!dts->bUseDLL)
          {
            if (bIsStatusRoom) // "status" Window play
              OutStr = String("/echo -s ") + OutStr;
            else
              OutStr = "/msg " + sTempPlayChan + " " + OutStr;

            // below is a way to send it "raw" but we would need to
            // /echo it locally too - might not need "PadSpaces" then...
            //TempStr = "/raw PRIVMSG " + PlayChan + " :" + OutStr;

            SendDDE("mirc", FN[9], OutStr); // "command"
#if DEBUG_ON
            dts->CWrite("\r\nSendToClient(): \"" + OutStr + "\"\r\n");
#endif
          }
          else
            (*pColorStart)("mirc", sTempPlayChan.c_str(),
                   OutStr.c_str(), -1, dts->bUseFile);
        }
  //      else if (dts->IsPirch())
  //      {
  //        if (!dts->bUseDLL)
  //        {
  //          if (bIsStatusRoom) // "status" Window play
  //            OutStr = String("/echo ") + OutStr;
  //          else
  //            OutStr = "/msg " + sTempPlayChan + " " + OutStr;
  //
  //          SendStringToClient(sTempPlayChan, OutStr);
  //        }
  //        else
  //          (*pColorStart)("PIRCH", sTempPlayChan.c_str(),
  //                 OutStr.c_str(), -1, dts->bUseFile);
  //      }
        else if (dts->IsCli(C_VORTEC))
        {
          if (!dts->bUseDLL)
          {
            if (bIsStatusRoom) // "status" Window play
              OutStr = String("/display ") + OutStr;
            else
              OutStr = "/msg " + sTempPlayChan + " " + OutStr;

            // below is a way to send it "raw" but we would need to
            // /echo it locally too - might not need "PadSpaces" then...
            //TempStr = "/quote PRIVMSG " + sTempPlayChan + " :" + OutStr;

            SendDDE("VORTEC", "IRC_COMMAND", OutStr);
          }
          else
            (*pColorStart)("VORTEC", sTempPlayChan.c_str(),
                   OutStr.c_str(), -1, dts->bUseFile);
        }
        else if (dts->IsCli(C_XIRCON))
          (*pColorStart)(NULL, sTempPlayChan.c_str(),
                 OutStr.c_str(), -1, dts->bUseFile);
      }

      // Advance progress-bar
      // NOTE: we can init and show a progress bar ok but if text is processed
      // during playback our bar gets messed up - and it really needs to have
      // priority... so instead of updating the bar via CpUpdate, drive the bar
      // directly with:
      if (!dts->MainProgressBar->Visible)
        dts->MainProgressBar->Visible = true;
      dts->MainProgressBar->Position = dts->GPlayIndex*100/dts->GPlaySize;
      //dts->CpUpdate(dts->GPlayIndex);
    }

    if (dts->GPlayIndex >= dts->GPlaySize)
    {
      DoStopPlay();
      return true;
    }

  //  dts->Timer1->Enabled = true; // start new interval

    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPlay::SendStringToClient(String sPlayChan, String S)
// Mainly for sending specialized commands to YahELite...
// sPlayChan is empty for YahELite
{
  try
  {
    // FindWindow looks for the class name or atom created with RegisterClass
    // or RegisterClassEx. If it can't find that it looks for the window's title
    // text...
    HWND temp = FindClient(dts->GReplyTo);

    if (!temp)
    {
      DoStopPlay();
      return;
    }

    dts->GReplyTo = temp;

    if (dts->IsCli(C_YAHELITE))
      SendYahELite(S);
    else
      SendColorizeNet(sPlayChan, S, dts->RWM_ColorizeNet);
  }
  catch(...){}
}
//---------------------------------------------------------------------------
bool __fastcall TPlay::SendColorizeNet(String sPlayChan, String S, UINT winMsg)
{
  try
  {
    int size = sizeof(COLORIZENET_STRUCT);

    HGLOBAL hMem = GlobalAlloc(GMEM_FIXED, size);
    COLORIZENET_STRUCT* cns = (COLORIZENET_STRUCT *)GlobalLock(hMem);

    // Limit string length...
    if (S.Length() > CNS_DATALEN-1)
      S.SetLength(CNS_DATALEN-1);

    // Set the string length in chars and copy the text to our struct
    cns->lenData = S.Length();
    StrCopy((char*)cns->data, S.c_str());

    // Current play-channel...
    cns->lenChanNick = dts->PlayChan.Length();
    StrCopy((char*)cns->chanNick, sPlayChan.c_str());

    cns->commandID = REMOTE_COMMAND_TEXT;
    cns->serverID = dts->WmCopyServer;
    cns->channelID = dts->WmCopyChannel;

    cns->commandData = dts->SendUtf8 ? 1 : -1;

    cns->clientID = dts->GetColorizeNetClient(dts->Cli);

    cns->lspare = 0; // not used

    HGLOBAL hCds = GlobalAlloc(GMEM_FIXED, sizeof(MYCOPYDATASTRUCT));
    MYCOPYDATASTRUCT * cds = (MYCOPYDATASTRUCT *)GlobalLock(hCds);

    // Set COPYDATASTRUCT lpData to address of struct
    cds->lpData = (unsigned __int64)cns;

    // Set COPYDATASTRUCT dwData to registered windows message
    cds->dwData = winMsg;

    // the size of COLORIZENET_STRUCT
    cds->cbData = size;

    DWORD result;
    SendMessageTimeout((HWND)dts->GReplyTo, WM_COPYDATA,
             (WPARAM)Application->Handle, (LPARAM)cds, SMTO_NORMAL,
                  SMTO_TIMEOUT, &result);

    GlobalFree(hCds);
    GlobalFree(hMem);

    return true;
  }
  catch(...){return false;}
}
//---------------------------------------------------------------------------
bool __fastcall TPlay::SendYahELite(String S)
{
  try
  {
    char buf[64];

    // get the class name to ensure it is a YahELite chat window
    GetClassName((HWND)dts->GReplyTo, buf, sizeof(buf)-1);

    if(LowerCase(String(buf)) == LowerCase(Class))
    {
      MYCOPYDATASTRUCT cds;

      cds.dwData = dts->RWM_YahELiteExt ; // Windows message "signiture"

      cds.lpData = (unsigned long)S.c_str();

      // the size of the string includes the terminating NULL
      cds.cbData = S.Length() + 1;

      // Send to client with 5 second timeout
      DWORD result;
      SendMessageTimeout((HWND)dts->GReplyTo, WM_COPYDATA,
               (WPARAM)Application->Handle, (LPARAM)&cds, SMTO_NORMAL,
                                               SMTO_TIMEOUT, &result);
    }

    return true;
  }
  catch(...)
  {
    return false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPlay::SendPaltalk(String chan, String S)
// chan is the roomName (such as PlayChan)
// S is the paltalk formatted text to post
// (in UTF-8!)
{
  if (GPaltalk)
  {
    // The compiler converts to WideString and back in the following...
    // (what goes in is <= 255 so it should work ok!)
    S = utils->StripBgCodes(S);
    S = utils->StripTrailingSpaces(S);

    if (!S.IsEmpty())
    {
      // Max colors is 10 for PalTalk, don't write any default-colors
      // Set bStripFontType true - Paltalk has no RTF font-table that is
      // documented... so embedded f3, f10, etc would not be predictable...

      // NOTE - I'm setting this to always strip RTF font-indexes because
      // there is no info about any Paltalk server-side RTF font-table and even
      // if there is one it won't match our own font-type-table anyway!
      S = utils->ConvertToRtf(utils->Utf8ToWide(S), dts->PaltalkMaxColors,
                                                  NO_COLOR, NO_COLOR, true);
//      S = utils->ConvertToRtf(utils->Utf8ToWide(S), dts->PaltalkMaxColors,
//                                      NO_COLOR, NO_COLOR, dts->bStripFont);

      if (!GPaltalk->sendTextToRoomOrPm(chan, S, true))
        DoStopPlay();
    }
  }
  else
    DoStopPlay();
}
//---------------------------------------------------------------------------
void __fastcall TPlay::SendDDE(String client, String command, String data)
// Send via dynamic-data-exchange (used in mIRC and Vortec)
{
  TDdeClientConv* dde = new TDdeClientConv(DTSColor);
  dde->SetLink(client.c_str(), command.c_str());
  dde->PokeData("active", data.c_str());
  dde->CloseLink();
  delete dde;
}
//---------------------------------------------------------------------------
void __fastcall TPlay::DoStopPlay(void)
{
  // if UseFile and UseDLL are unchecked, we send the text
  // to mIRC or Vortec locally...

  if (dts->bIsPlaying)
  {
    dts->Timer1->Enabled = false;
    dts->bIsPlaying = false;

    if (dts->GPlayBuffer)
    {
      delete [] dts->GPlayBuffer;
      dts->GPlayBuffer = NULL;
    }

    dts->GPlaySize = 0;
    dts->GPlayIndex = 0;
    dts->GPlayer = -1;

    // Yahoo playback via /ex command may change the play-speed
    // so restore it after play ends.
    dts->Timer1->Interval = dts->PlayTime;

    dts->CpReset(); // force status-panel reset
  }
  else if (dts->IsCli(C_YAHELITE) && dts->bUseFile)
    // YahELite's stop-command
    SendStringToClient("", "/notextfile" + String(CTRL_Z));

  if (dts->bUseDLL)
  {
    // Get custom dll's entry point
    tColor pColor = LoadDLL(dts->hDll, "_ColorStop");

    if (pColor != NULL)
      (*pColor)(); // stop

    FreeLibrary(dts->hDll);
    dts->hDll = NULL;
  }
  // mIRC with UseDLL unchecked and UseFile checked
  else if (dts->IsCli(C_MIRC) && dts->bUseFile)
    // "command"
    SendDDE("mirc", FN[9], String(FN[12])); // "/play stop"
  // Vortec with UseDLL unchecked and UseFile checked
  else if (dts->IsCli(C_VORTEC) && dts->bUseFile)
    SendDDE("VORTEC", "IRC_COMMAND", "/PLAYSTOP"); // "/PLAYSTOP"
  // PIRCH?
//  else if (dts->IsPirch())
//    SendDDE("PIRCH", "IRC_COMMAND", "/PLAYSTOP");

  // Clear the remote paltalk edit-control
  if (dts->IsCli(C_PALTALK) && GPaltalk != NULL)
  {
    if (!dts->ShiftHeldOnPlay)
      GPaltalk->clearText(); // also clears InhibitRemoteEnterKeypress!

    if (OpenClipboard(dts->Handle))
      utils->ClearClipboard(); // Clean up the clipboard
  }

  dts->StopButton->Enabled = false;
  dts->PauseButton->Enabled = false;
  dts->PauseButton->Visible = true;
  dts->ResumeButton->Enabled = false;
  dts->ResumeButton->Visible = false;

  // Clear the target server and channel globals (they are set when the
  // user has typed "cx sometext" in some channel... we get the ID and
  // post back to that same channel, but after we stop and the user
  // presses our play button, we want to use the room selected in
  // Play->Options.
  dts->WmCopyChannel = -1;
  dts->WmCopyServer = -1;

  // Set to 0 if client is gone...
  if (!IsWindow((HWND)dts->GReplyTo))
    dts->GReplyTo = 0;
}
//---------------------------------------------------------------------------
void __fastcall TPlay::DoPausePlay(void)
{
  if (dts->bIsPlaying)
  {
    dts->Timer1->Enabled = false;

    dts->PauseButton->Enabled = false;
    dts->PauseButton->Visible = false;
    dts->ResumeButton->Enabled = true;
    dts->ResumeButton->Visible = true;
  }
  else if (dts->bUseDLL)
  {
    // Get custom dll's entry point
    tColor pColor = LoadDLL(dts->hDll, "_ColorPause");

    if (pColor != NULL)
    {
      (*pColor)(); // pause

      dts->PauseButton->Enabled = false;
      dts->PauseButton->Visible = false;
      dts->ResumeButton->Enabled = true;
      dts->ResumeButton->Visible = true;
    }
  }
  // mIRC?
  else if (dts->IsCli(C_MIRC) && dts->bUseFile)
    // mIRC with UseDLL unchecked and UseFile checked
    utils->ShowMessageU(String(DS[127]) + "mIRC");
  // Vortec?
  else if (dts->IsCli(C_VORTEC) && dts->bUseFile)
  {
    SendDDE("VORTEC", "IRC_COMMAND", "/PLAYPAUSE");

    dts->PauseButton->Enabled = false;
    dts->PauseButton->Visible = false;
    dts->ResumeButton->Enabled = true;
    dts->ResumeButton->Visible = true;
  }
  // PIRCH?
//  else if (dts->IsPirch() && dts->bUseFile)
//  {
//    SendDDE("PIRCH", "IRC_COMMAND", "/PLAYPAUSE");
//
//    dts->PauseButton->Enabled = false;
//    dts->PauseButton->Visible = false;
//    dts->ResumeButton->Enabled = true;
//    dts->ResumeButton->Visible = true;
//  }
}
//---------------------------------------------------------------------------
void __fastcall TPlay::DoResumePlay(void)
{
  if (dts->bIsPlaying)
  {
    dts->Timer1->Enabled = true;

    dts->PauseButton->Enabled = true;
    dts->PauseButton->Visible = true;
    dts->ResumeButton->Enabled = false;
    dts->ResumeButton->Visible = false;
  }
  else if (dts->bUseDLL)
  {
    // Get custom dll's entry point
    tColor pColor = LoadDLL(dts->hDll, "_ColorResume");

    if (pColor != NULL)
    {
      (*pColor)(); // resume

      dts->PauseButton->Enabled = true;
      dts->PauseButton->Visible = true;
      dts->ResumeButton->Enabled = false;
      dts->ResumeButton->Visible = false;
    }
  }
  // mIRC?
  else if (dts->IsCli(C_MIRC) && dts->bUseFile)
    // mIRC with UseDLL unchecked and UseFile checked (can't do resume)
    utils->ShowMessageU(String(DS[129]) + "mIRC");
  // Vortec?
  else if (dts->IsCli(C_VORTEC) && dts->bUseFile)
  {
    SendDDE("VORTEC", "IRC_COMMAND", "/PLAYRESUME");

    dts->PauseButton->Enabled = true;
    dts->PauseButton->Visible = true;
    dts->ResumeButton->Enabled = false;
    dts->ResumeButton->Visible = false;
  }
  // PIRCH?
//  else if (dts->IsPirch() && dts->bUseFile)
//  {
//    SendDDE("PIRCH", "IRC_COMMAND", "/PLAYRESUME");
//
//    dts->PauseButton->Enabled = true;
//    dts->PauseButton->Visible = true;
//    dts->ResumeButton->Enabled = false;
//    dts->ResumeButton->Visible = false;
//  }
}
//---------------------------------------------------------------------------
tColor __fastcall TPlay::LoadDLL(HINSTANCE &handle, String entry)
{
  handle = LoadLibrary(FN[7]); // "Colorize.dll"

  if (handle == NULL || !entry.Length())
    return NULL;

  tColor addr;

  try
  {
    addr = (tColor)GetProcAddress(handle, entry.c_str());
  }
  catch(...)
  {
    return NULL;
  }

  return addr;
}

tColorStart __fastcall TPlay::LoadDll(HINSTANCE &handle)
{
  handle = LoadLibrary(FN[7]); // "Colorize.dll"

  if (handle == NULL)
    return(NULL);

  tColorStart addr;

  try
  {
    addr = (tColorStart)GetProcAddress(handle, "_ColorStart");
  }
  catch(...)
  {
    return NULL;
  }

  return addr;
}
//---------------------------------------------------------------------------
// End Play Functions
//---------------------------------------------------------------------------

