//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

#include "Playback.h"
#include "Play.h"
#include "Paltalk.h"
#include "Utils.h"
#include "DlgPlay.h"
#include "ConvertToYahoo.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// Playback Functions
//---------------------------------------------------------------------------
__fastcall TPlayback::TPlayback(TComponent* Owner)
// constructor
{
  this->dts = static_cast<TDTSColor*>(Owner);

  play = new TPlay(dts);
  play->SetClassAndWindName();
}
//---------------------------------------------------------------------------
__fastcall TPlayback::~TPlayback(void)
// destructor
{
  if (play)
    delete play;
}
//---------------------------------------------------------------------------
bool __fastcall TPlayback::DoStartPlay(int Time)
{
  if (dts->bIsPlaying)
    play->DoStopPlay();

  WideString FileStr = dts->Default_Playfile;

  dts->GInterval = Time;

  if (dts->IsWmCopyData()) // WM_COPYDATA Client?
  {
    // Had trouble finding a C# Window Title to use for FindWindow() when
    // we don't have the handle yet (Client has not sent us anything yet).
    // So this is a diagnostic function to get the name given a handle...
    //if (dts->GhwndReplyTo != NULL)
    //{
    //  int len;
    //
    //  // Window caption
    //  if ((len = GetWindowTextLength(dts->GhwndReplyTo)) > 0)
    //  {
    //    utils->ShowMessageU(String(len)); // Returns 14 chars "IceChatMsgPump"
    //    char * p = new char[len+1];
    //    if (GetWindowText(dts->GhwndReplyTo, p, len+1) != NULL)
    //      utils->ShowMessageU(String(p));
    //    delete [] p;
    //  }
    //}

    HWND temp = play->FindClient(dts->GReplyTo);

    if (temp == NULL)
    {
      // Client not found...
      utils->ShowMessageU(DS[111]);
      return false;
    }

    dts->GReplyTo = temp; // Set handle

    // Delete old buffer if necessary...
    if (dts->GPlayBuffer != NULL)
    {
      delete [] dts->GPlayBuffer;
      dts->GPlayBuffer = NULL;
    }

    if (dts->IsCli(C_YAHELITE))
    {
      // YahELite stores our Class and Name in yahelite.ini
      // in the WINDOWS directory.  Hackers can easily change this
      // and we lose registration.  Idea here is to read the ini file
      // and if it exists and the Class and Name for the extension
      // are not ours, tell the user how to re-set it.
      WideString YahIniFile = utils->FindFileW("yahelite.ini", "\\yahelite");

      if (!YahIniFile.IsEmpty())
      {
        // Open and read the file
        TIniFile* pIniFile = new TIniFile(utils->GetAnsiPathW(YahIniFile));
        String Class = pIniFile->ReadString("Ext", "Class", "");
        String Name = pIniFile->ReadString("Ext", "Name", "");
        delete pIniFile;

        if (Class.LowerCase() != LowerCase(String('T') + Application->Name) ||
              Name.LowerCase() != dts->Caption.LowerCase())
        {
          utils->ShowMessageU(DS[112]);
          return false;
        }
      }

    // Warn user against sending garbage to Yahoo
//    if (tae->View == V_OFF && dts->EffectWasAdded)
//    {
//      if (utils->ShowMessageU(Application->Handle, DS[113],
//                    MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1) == IDYES)
//        return(false);
//    }

      // Move SL_HTM or edit text to a buffer
      // GPlaySize will be 1 less than the actual buffer-size...
      if (!MoveTextToYahooBuffer(dts->GPlayBuffer, dts->GPlaySize))
        return false;

      // Copy to a temp file and tell YahELite to play it
      if (dts->bUseFile && dts->GPlayBuffer != NULL)
      {
        TStringsW* SelStrings = new TStringsW();
        SelStrings->SetTextBuf(dts->GPlayBuffer);
        delete [] dts->GPlayBuffer;
        SelStrings->SaveToFile(FileStr);
        delete SelStrings;

        // play the file by commands

        // /extend tdtscolor yahcolorize
        String InitStr = String(YAHEXTENDSTR) + Application->Name +
                        String(' ') + dts->Caption + String(CTRL_Z);

        play->SendStringToClient("", InitStr); // Initialize... Send /extend for YahELite
        play->SendStringToClient("", "/delay " + String(dts->PlayTime) + String(CTRL_Z));
        play->SendStringToClient("", "/textfile " + FileStr + String(CTRL_Z));
        dts->StopButton->Enabled = true;
        return true;
      }
    }
    // Move edit text to a buffer for all non-YahELite WM_COPYDATA clients...
    else
      MoveTextToGPlayBuffer();

    return InitLocalPlayback(Time);
  }
  else if (dts->IsCli(C_PALTALK))
  {
    if (!GPaltalk || !GPaltalk->getAllRoomsAndPms())
    {
      // Client not found...
      utils->ShowMessageU(DS[111]);
      return false;
    }

    // Is the room still here?
    GPaltalk->deleteServiceIfRoomGone();

    // If the user has never selected a Paltalk room, we pop up the play-options
    // dialog and if there are no rooms we print a message and return
    if (GPaltalk->CurrentRoomIndex == -1)
    {
      try
      {
        Application->CreateForm(__classid(TPlayForm), &PlayForm);
        PlayForm->ShowModal();

        if (!PlayForm->HaveRooms)
          // Client not found...
          utils->ShowMessageU(DS[111]);
      }
      __finally
      {
        if (PlayForm)
        {
          PlayForm->Release(); // Destroy the form
          PlayForm = NULL;
        }
      }

      return false; // return unless you want play to begin now!
    }

    GPaltalk->clearText(); // Clear the remote Paltalk edit-control

    // Inhibit remote press of "Enter" during playback if shift key down
    if (GetKeyState(VK_SHIFT)&0x8000)
      dts->ShiftHeldOnPlay = true;
    else
      dts->ShiftHeldOnPlay = false;

  }

  // Non- WM_COPYDATA clients!
  /////////////////////////////////////////////////////////////////////
  //                 mIRC/XiRCON/Vortec/PalTalk                      //
  /////////////////////////////////////////////////////////////////////

  try
  {
    if (tae->View == V_RTF || tae->View == V_IRC || tae->View == V_ORG)
    {
      if (tae->SelLength || dts->SL_IRC->Count > 0 || dts->SL_ORG->Count > 0)
      {
        MoveTextToGPlayBuffer();

        // Send using our own loop?
        if (!dts->bUseFile)
          return (InitLocalPlayback(Time)); // Kick off internal play

        // Here we write to temp file set in PlayPath of PlayDlg.cpp
        // then the dll will read that file and then use its own timer
        // to alternately write single text lines to two additional
        // files and send those files to mIRC with mIRC's /play <file>
        // command... original concept was to avoid the stripping of
        // spaces that mIRC does when you send it text using the /msg
        // command... but WHAT a PAIN! - the dll could just send mIRC
        // /play this file we are about to write... but then if you
        // do that we can just do that here now...
        //
        // I think the idea was that mIRC could not do a pause/resume
        // play of a file, but we can command the dll to pause/resume.
        // But that's NOT working now, and stop is not working either!
        TStringsW* SelStrings = new TStringsW();
        SelStrings->SetTextBuf(dts->GPlayBuffer);
        delete dts->GPlayBuffer;
        dts->GPlayBuffer = NULL;
        dts->GPlaySize = 0;
        SelStrings->SaveToFile(FileStr);
        delete SelStrings;
      }
      else // No text to play
      {
        utils->ShowMessageU(DS[116]);
        return false;
      }
    }
    else // ViewType = V_HTML or V_RTF_SOURCE
    {
      tae->PlainText = true;

      if (tae->SelLength > 0 || tae->TextLength > 0)
      {
        // Processed text strings
        TStringsW* SelStrings = new TStringsW();

        if (tae->SelLength)
          SelStrings->Text = tae->SelTextW;
        else
          SelStrings->Text = tae->TextW;

        // Send using our own loop?
        if (!dts->bUseFile)
        {
          if (dts->GPlayBuffer)
            delete [] dts->GPlayBuffer;

          dts->GPlayBuffer = SelStrings->GetTextBuf();
          dts->GPlaySize = SelStrings->TotalLength;
          delete SelStrings;

          // RETURN
          return InitLocalPlayback(Time); // Kick off internal play
        }

        SelStrings->SaveToFile(FileStr);
        delete SelStrings;
      }
      else // No text to play
      {
        utils->ShowMessageU(DS[116]);
        return false;
      }
    }
  }
  catch(...)
  {
    // "Error preparing text for playback...", // 117
    utils->ShowMessageU(DS[117]);
    return(false);
  }

  String sPath = utils->GetAnsiPathW(FileStr);

  // Talk to DLL, tell it to start playing the specified file
  String sChan;

  if (dts->bSendUtf8)
    sChan = dts->PlayChan; // normally in Utf8
  else
    sChan = utils->Utf8ToAnsi(dts->PlayChan);

  if (dts->bUseDLL)
  {
    if (dts->bUseFile)
    {
      // Get custom dll's entry point
      tColorStart pColorStart = play->LoadDll(dts->hDll);

      if (pColorStart == NULL)
      {
        // "Colorize.dll"
        utils->ShowMessageU("Cannot load: " + String(FN[7]) + ".\n" +
          "Make sure the file Colorize.dll is in the\n" +
            "same directory as colorizeu.exe. Also try\n" +
            "unchecking UseDLL in Play->Options");

        return false;
      }

      if (dts->IsCli(C_XIRCON))
        (*pColorStart)(NULL, sChan.c_str(), sPath.c_str(), Time, true);
      else if (dts->IsCli(C_MIRC))
        (*pColorStart)("mirc", sChan.c_str(), sPath.c_str(), Time, true); // mIRC
      else if (dts->IsCli(C_VORTEC))
        (*pColorStart)("vortec", sChan.c_str(), sPath.c_str(), Time, true); // Vortec
      else if (dts->IsCli(C_ICECHAT))
        (*pColorStart)("icechat", sChan.c_str(), sPath.c_str(), Time, true); // IceChat
      else
        (*pColorStart)("leafchat", sChan.c_str(), sPath.c_str(), Time, true); // LeafChat

      dts->StopButton->Enabled = true;
      dts->PauseButton->Enabled = true;
      dts->PauseButton->Visible = true;
      dts->ResumeButton->Enabled = false;
      dts->ResumeButton->Visible = false;
      dts->bIsPlaying = true;
    }
    //else
    // Case where UseFile not checked, our internal loop should
    // be running... the SendToClient routine will send to the DLL
    // in the new "one-line" mode...
  }
  // mIRC or Vortec using DDE and own /play ?
  else if (!dts->IsCli(C_XIRCON))
  {
    try
    {
      String TempStr;

      if (dts->IsCli(C_MIRC))
      {
        if (LowerCase(sChan) == STATUSCHAN) // "status" Window play
          // /play -eps
          TempStr = String(FN[10]) + " \"" + sPath + "\" " + String(Time);
        else
          // /play -p
          TempStr = String(FN[11]) + " " + sChan + " " + sPath + " " + String(Time);

        play->SendDDE("mirc", FN[9], TempStr); // "command"
      }
//      else if (dts->IsVortec()) // Vortec
      else // Vortec
      {
        if (LowerCase(sChan) == STATUSCHAN) // "status" Window play
          TempStr = String("/PLAYFILE -s ") + sPath;
        else
          TempStr = "/PLAYFILE " + sChan + " " + sPath;

        play->SendDDE("VORTEC", "IRC_COMMAND", TempStr);
      }
//      else // PIRCH
//      {
//        if (LowerCase(sChan) == STATUSCHAN) // "status" Window play
//          TempStr = String("/PLAYFILE -s ") + sPath;
//        else
//          TempStr = "/PLAYFILE " + sChan + " " + sPath;
//
//        SendDDE("PIRCH", "IRC_COMMAND", TempStr);
//      }

      dts->StopButton->Enabled = true;
      dts->PauseButton->Enabled = true;
      dts->PauseButton->Visible = true;
      dts->ResumeButton->Enabled = false;
      dts->ResumeButton->Visible = false;
      dts->bIsPlaying = true;
    }
    catch(...)
    {
      utils->ShowMessageU(DS[125]);
      return false;
    }
  }

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TPlayback::MoveTextToGPlayBuffer(void)
{
  if (dts->GPlayBuffer != NULL)
  {
    try {delete [] dts->GPlayBuffer;} catch(...) {}
    dts->GPlayBuffer = NULL;
  }

  dts->GPlaySize = 0;

  WideString sOut = utils->MoveMainTextToString();

  // Flatten tabs
  sOut = utils->FlattenTabs(sOut, dts->RegTabMode);

  // Convert \pagebreak strings into C_FF then strip them out
  sOut = utils->PageBreaksToFormFeed(sOut);
  sOut = utils->StripChar(sOut, C_FF);

  if (sOut.IsEmpty())
  {
    utils->ShowMessageU(DS[116]);
    return false;
  }

  // Strip trailing codes
  sOut = utils->StripTrailingCodes(sOut);

  if (sOut.IsEmpty())
    utils->ShowMessageU("Exception in utils->StripTrailingSpacesAndCodes()\n"
                                                "from Playback.cpp");

  // Strip out Push/Pop
  sOut = utils->FlattenPushPop(sOut);

  if (sOut.IsEmpty())
    utils->ShowMessageU("Exception in utils->StripPushPop()\nfrom Playback.cpp");

  // Strip out redundant codes
  sOut = utils->Optimize(sOut, false, NO_COLOR);

  if (dts->bPadSpaces && (dts->IsCli(C_XIRCON) || !dts->bUseFile))
  {
    sOut = utils->ReplaceSpaces(sOut);
    if (sOut.IsEmpty())
      utils->ShowMessageU("Exception in utils->ReplaceSpaces() from\nPlayback.cpp");
  }

  dts->GPlayBuffer = utils->StrNewW(sOut.c_bstr());

  if (dts->GPlayBuffer == NULL)
  {
    utils->ShowMessageU(DS[116]);
    dts->GPlaySize = 0;;
    return false;
  }

  dts->GPlaySize = sOut.Length();
  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TPlayback::MoveTextToYahooBuffer(wchar_t* &pBuf, int &iSize)
// Call with an undefined buffer!!!!
// Don't forget to delete it....
// Returns iSize as the size of the buffer without the NULL!
{
  iSize = 0;
  pBuf = NULL;

  if (!dts->IsCli(C_YAHELITE))
    return false;

  try
  {
    if (tae->View == V_RTF)
    {
      // If the Client is "YahELite", try to move the SL_HTM buffer
      if (dts->SL_HTM->Count)
      {
        // Read entire buffer
        iSize = dts->SL_HTM->TotalLength;
        pBuf = dts->SL_HTM->GetTextBuf(); // Alloc buffer

        // If selected-zone... re-write the buffer
        // with just the selected lines
        if (tae->SelLength)
        {
          // Make a new buffer big enough for the lines we
          // want to move
          wchar_t* pLineBuf = new wchar_t[iSize+1];

          int ii;
          int jj = 0;

          bool bDummy;
          int LinesToMove = utils->GetNumberOfLinesSelected(bDummy);

          if (!LinesToMove)
            return false;

          int Line = 1;

          // Read through the SL_HTM text to find the lines
          // corresponding to the RTF-View's selected lines
          for (ii = 0; ii < iSize; ii++)
          {
            int LineIdx = utils->GetLine(tae);
            if (Line >= LineIdx && Line < LineIdx+LinesToMove)
              pLineBuf[jj++] = pBuf[ii];

            if (utils->FoundCRLF(pBuf, iSize, ii))
              if (++Line >= LineIdx+LinesToMove)
                break;
          }

          // Re-allocate buffer
          try
          {
            delete [] pBuf;
          }
          catch(...)
          {
#if DEBUG_ON
            dts->CWrite("\r\nUnable to deallocate pBuf in: MoveYahooTextToBuffer()\r\n");
#endif
          }

          pBuf = new wchar_t[jj+1];

          for (ii = 0; ii < jj; ii++)
            pBuf[ii] = pLineBuf[ii];

          pBuf[ii] = C_NULL;
          iSize = ii;

          try
          {
            delete [] pLineBuf;
          }
          catch(...)
          {
#if DEBUG_ON
            dts->CWrite("\r\nUnable to deallocate pLineBuf in: MoveYahooTextToBuffer()\r\n");
#endif
          }
        }
      }
    }
    else
    {
      int textLen;

      if (tae->SelLength)
        textLen = tae->SelLength;
      else
        textLen = tae->TextLength - tae->LineCount + 1;

      if (textLen > 0)
      {
        int NumChars = textLen+1;

        pBuf = new wchar_t[NumChars];

        if (tae->SelLength > 0)
          tae->GetSelTextBufW(pBuf, NumChars*sizeof(wchar_t));
        else
          tae->GetTextBufW(pBuf, NumChars*sizeof(wchar_t));

        iSize = NumChars;
      }
    }

    return true;
  }
  catch(...)
  {
    utils->ShowMessageU(String(DS[95]) + "MoveYahooTextToBuffer()");
    return false;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TPlayback::InitLocalPlayback(int Time)
{
  if (dts->GPlayBuffer == NULL || dts->GPlaySize <= 0)
    return false;

  // Analyze all text to see if it goes over user's
  // text size-limit (500 bytes) (Yahoo network limit is 500 bytes)
  int LineCount = 0;

  dts->LargestPlayLineWidth = 0;
  dts->LargestPlayLineBytes = 0;

  dts->GPlayIndex = 0;
  dts->bIsPlaying = true;

  // Fake loop-through to compute width of longest line we will
  // transmit to the client...
  //
  // (Note: With Paltalk, we paste RTF text to the Paltalk text-entry
  // edit-control - so the IRC-coded line-length is utterly meaningless.)
  // (Same with the YahELite encoding - but that's a moot point these days...)
  while (dts->GPlayIndex < dts->GPlaySize && dts->bIsPlaying)
  {
    if (!play->SendToClient(true)) // Fake send to get length (in bytes!)
    {
#if DEBUG_ON
      dts->CWrite( "\r\nError in play->SendToClient(true)...\r\n");
#endif
      return false;
    }

    // Track largest line's width and number ofbytes...
    // ...for status display (lower right corner)
    if (play->LineWidthInChars > dts->LargestPlayLineWidth)
      dts->LargestPlayLineWidth = play->LineWidthInChars;

    int LineBytes = play->LineWidthInBytes;

    if (LineBytes > dts->LargestPlayLineBytes)
      dts->LargestPlayLineBytes = LineBytes;

    if (dts->GMaxPlayXmitBytes >= 0 && LineBytes > dts->GMaxPlayXmitBytes)
    {
      play->DoStopPlay();

      if (GetForegroundWindow() != Application->Handle)
      {
        SetForegroundWindow(Application->Handle);

        if (dts->WindowState == wsMinimized)
        {
          dts->WindowState = wsNormal;
          Application->ProcessMessages();

          if (dts->WindowState != wsMinimized)
            tae->SetFocus();
        }
      }

      // Print largest playback line width

      dts->StatusBar->Font->Color = clRed;
      dts->StatusBar->Panels->Items[0]->Text = String(DS[98]) + " " +
           String(dts->LargestPlayLineWidth) + " chars";

      String TempStr = "Client Line #" + String(LineCount) +
                    " (" + String(LineBytes) + " bytes...)\n" +
                                                  String(DS[114]);

      MessageDlg(TempStr, mtWarning, TMsgDlgButtons() << mbOK, NULL);

      return false;
    }

    LineCount++;
  }

  dts->GLineCount = LineCount;

  // /extend tdtscolor yahcolorize
  if (dts->IsCli(C_YAHELITE))
  {
    String InitStr = YAHEXTENDSTR + Application->Name + String(' ') +
                                           dts->Caption + String(CTRL_Z);

    play->SendStringToClient("", InitStr); // Initialize... Send /extend for YahELite
  }

  dts->StopButton->Enabled = true;
  dts->PauseButton->Enabled = true;
  dts->PauseButton->Visible = true;
  dts->ResumeButton->Enabled = false;
  dts->ResumeButton->Visible = false;

  // Print largest playback line width
  dts->StatusBar->Font->Color = clBlue;
  dts->StatusBar->Panels->Items[SB_COUNT]->Text = String(DS[98]) + " " +
       String(dts->LargestPlayLineWidth) + " chars";

  // Set up to play 1st line immediately
  dts->GLineBurstCounter = dts->GLineBurstSize;

  dts->GPlayIndex = 0;
  dts->bIsPlaying = true;

  // Initialize the progress-bar
  dts->CpReset(); // force status panel reset
  dts->CpInit(1); // init for one section in the progress bar
  dts->CpSetMaxIterations(dts->GPlaySize); // base it on total chars

  // Start timer AFTER setting bIsPlaying flag!
  dts->Timer1->Interval = 50;  // Don't wait around
  dts->Timer1->Enabled = true;

  return true;
}
//---------------------------------------------------------------------------
String __fastcall TPlayback::GetDLLVersion(void)
{
  String Version;

  try
  {
    tColor pColor = play->LoadDLL(dts->hDll, "_Colorize_Version");

    // Get custom dll's entry point
    if (pColor)
    {
      LPTSTR ver = (LPTSTR)(*pColor)(); // version
      Version = String(ver);
      FreeLibrary(dts->hDll); // won't release until reference-count is 0!
    }
  }
  catch(...) {}

  return Version;
}
//---------------------------------------------------------------------------
// End Playback Functions
//---------------------------------------------------------------------------

