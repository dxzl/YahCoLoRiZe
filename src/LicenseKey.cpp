//
// TODO - need to port to Unicode... Application->ExeName to utils->GetExeNamwW() Etc.
//---------------------------------------------------------------------------
// All Routines for getting/decoding/entering a license key
// from either the registry/user-input/or a pre-attached Stub.
// Has code to store the key in the registry.
//
// Populates these properties:
// int Today, KeyInstallDay, KeyCreateDay, ActivationDays, ForceKey, PaidKey;
// int ExpireKey
//
// Refer to KeyGen.doc in the KeyGen project directory
//
// Dates earlier than 1980 are not in Windows XP Calendar
// so we shave down today's date to fit in 4-digit, Base36
// (range is 0-1,679,615)
//
// Jan 1, 1980 is 2,444,588 - 2,000,000 = 444,588
// Dec 31, 2099 is 2,488,082 - 2,000,000 = 488,082
//
// So we can handle 1,679,615 - 488,082 = 1,191,533
// days beyond the end of the year 2099!
//
// A standard 32-bit integer handles this fine also.
//
// So an expire day for "unlimited" key we will set
// to 1,679,615 (LK_MAX_DAY).
//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

#include "DlgLicense.h"
#include "LicenseKey.h"

#include <math.h>

#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TLicenseKey::TLicenseKey(void)
{
}
//---------------------------------------------------------------------------
__fastcall KeyClass::KeyClass(void)
{
  Clear();
}
//---------------------------------------------------------------------------
void __fastcall KeyClass::Clear(void)
{
  today=0;
  keyInstallDay=0;
  keyCreateDay=0;
  activationDays=0;
  regKeyIsExpired = false;

  forceKey=false;
  paidKey=false;
  expireKey=false;
}
//---------------------------------------------------------------------------
int __fastcall KeyClass::ComputeDaysRemaining(void)
{
  try
  {
#if ERROR_MESSAGES
    String S = RegKeyIsExpired ? "true" : "false";
    utils->ShowMessageU("ActivationDays: " + String(ActivationDays) +
                 "\nToday:" + String(Today) +
                 "\nRegKeyIsExpired:" + S +
                 "\nKeyCreateDay:" + String(KeyCreateDay) +
                 "\nKeyInstallDay:" + String(KeyInstallDay));
#endif

    // All fields populated?
    if (today <= 0 || activationDays <= 0 || keyInstallDay <= 0 || keyCreateDay <= 0) return 0;
    if (keyInstallDay < keyCreateDay || today < keyInstallDay || today < keyCreateDay) return 0;

    // This flag is only set in a registry key that
    // was detected as "expired" in ValidateLicenseKey()
    if (regKeyIsExpired) return 0; // no days remaining

    int temp;

    // Compute days elapsed since key issued or installed
    if (expireKey) temp = today-keyCreateDay;
    else temp = today-keyInstallDay;

    // Compute days remaining on key
    if (temp >= 0 && activationDays-temp >= 0)
    {
#if ERROR_MESSAGES
      ShowMessage("Remaining days: " + String(activationDays-temp));
#endif
      return(activationDays-temp);
    }

    return -100; // error -- shut it down
  }
  catch(...) { return -100; } // error -- shut it down
}
//---------------------------------------------------------------------------

bool __fastcall TLicenseKey::ValidateLicenseKey(bool bPrompt)
// The first function called from the "Show" Event
{
  if (!PK) return false;

  try
  {
    // First see if we have a Key and E-Mail sum attached as a stub...
    //
    // If the "stub" key is present and the registry-key is absent
    // or corrupt in any way install the stub-key.
    //
    // If the stub is present and the registry key is valid but has
    // expired, 1) If the PaidKey flag is set in the REGISTRY KEY,
    // prompt for a manual key, 2) If the PaidKey flag is NOT set
    // in the REGISTRY KEY, install a new trial key only if the #
    // days in the two keys do not match. (else prompt for a user to
    // enter a key).
    //
    // The ExpireKey flag, if set, tells the program to compute the
    // # days left on the key relative to the key's creation-date
    // rather than the key's installation date.
    //
    // The ForceKey flag tells the program to write the key with
    // the only condition being that its creation date is newer than
    // the key in the registry.
    //
    // PaidKey, ForceKey and ExpireKey are all permitted in a trial
    // or "stub" key.
    //
    // We want the user to be able to re-install their old key if PaidKey
    // is set, regardless of the Key's creation date.
    String StubKey, StubEmail;

#if DATA_STUB_EXPECTED
    GetStub(StubKey, StubEmail);
#else
    StubKey = String(Trial_Key);
    StubEmail = String(TRIAL_EMAIL);
#endif

    // Init License-Key Properties
    PK->Clear();

    KeyClass StubPK;

    int StubCode;

    // Check Stub-Key, if any...
    if (!StubKey.IsEmpty())
      StubCode = DecodeProductKey(StubKey, StubEmail);
    else
      StubCode = -100;

    StubPK = *PK; // Save Stub-key properties

    // Init License-Key Properties
    PK->Clear();

    // Check user's existing registry Product-Key and get properties
    int RegCode = DecodeProductKey("","");

    // No key found...
    // Trial version, use a 15-day key with PaidKey, ForceKey and ExpireKey
    // clear. Require e-mail validation to "yahcolorize@trial.com"
    if (RegCode < 0 && RegCode != -7) // Corrupt key or no key in registry?
    {
      if (!StubKey.IsEmpty() && DecodeAndWriteKey(StubKey, StubEmail)) // Good Stub?
        // "SwiftMiX has been successfully activated!\n"
        // "To enter a purchased license key, click\n\n"
        // "     Options->About->Install License Key\n\n"
        // "Thank You for trying my software!", //16
        utils->ShowMessageU(KEYSTRINGS[16]); // Key stored + activated...
      else if (bPrompt)
        DoKey(); // bad stub-key
      else if (RegCode != -100)
        ErrorCode(RegCode); // Error in existing registry-key
    }
    // Good registry key (but may be expired) + good stub key also?
    else if (StubCode >= 0)
    {
      // Good stub-key...
      if (RegCode == -7) // Reg key expired?
      {
        // Expired...
        if (PK->ActivationDays != StubPK.ActivationDays)
          WriteStub(StubKey, StubEmail, &StubPK);
      }
      else
      {
        // Not expired...
        if (!StubPK.ForceKey)
        {
          if (!StubPK.PaidKey)
          {
            if (!PK->PaidKey && PK->ActivationDays != StubPK.ActivationDays)
              WriteStub(StubKey, StubEmail, &StubPK);
          }
          else
            WriteStub(StubKey, StubEmail, &StubPK);
        }
        else if (!PK->PaidKey)
          WriteStub(StubKey, StubEmail, &StubPK);
      }
    }
    // Bad stub-key and reg-key exists but may be expired...
    else if (RegCode == -7) // Reg key expired?
    {
      PK->Clear();
#if ERROR_MESSAGES
      utils->ShowMessageU("Calling DoKey() with expired reg-key\n"
                      "and bad or missing stub-key...");
#endif
      DoKey();
    }
  }
  catch(...)
  {
    utils->ShowMessageU("There was a problem validating the product key!");
    return(false);
  }

  return true;
}
//---------------------------------------------------------------------------

void __fastcall TLicenseKey::WriteStub( String StubKey,
                              String StubEmail, KeyClass * StubPK)
{
  // Don't write stub if older or same create date as reg-key.
  if (StubPK->KeyCreateDay <= PK->KeyCreateDay)
    return;

  PK->Clear();

  if (DecodeAndWriteKey(StubKey, StubEmail)) // Good Write?
    // "SwiftMiX has been successfully activated!\n"
    // "To enter a purchased license key, click\n\n"
    // "     Options->About->Install License Key\n\n"
    // "Thank You for trying my software!", //16
    utils->ShowMessageU(KEYSTRINGS[16]); // Key stored + activated...
  else // bad stub-key write...
    // "Invalid License Key...\n"
    // "Operating in Restricted Mode, Some Features Unavailable...", //17
    utils->ShowMessageU(KEYSTRINGS[17]);
}
//---------------------------------------------------------------------------

bool __fastcall TLicenseKey::DoKey(bool bMessageOnCancel)
// if bMessageOnCancel, show message when Cancel is pressed
// warning that no valid license key is present...
//
// (only place we don't show message on Cancel is if DoKey()
// is called from the About-dialog...)
{
  if (!PK)
    return(false);

  bool bOK = false;

  try
{
    Application->CreateForm(__classid(TLicenseForm), &LicenseForm);

ShowForm:

    LicenseForm->ShowModal();

    if (LicenseForm->ModalResult == mrOk)
  {
      String S1 = LicenseForm->Edit2->Text.Trim(); // Key
      String S2 = LicenseForm->Edit1->Text.Trim(); //Email

      if (S1.IsEmpty())
    {
        //"Your need to obtain a License Key and enter\n"
        //"it in order to validate this product.");
        utils->ShowMessageU(KEYSTRINGS[6]);
        goto ShowForm;
    }

      if (DecodeAndWriteKey(S1, S2))
    {
        bOK = true;

        // "New License Key Entered Successfully!", //15
        utils->ShowMessageU(KEYSTRINGS[15]);
    }
      else
        // "Key you entered was not valid!" //18
        utils->ShowMessageU(KEYSTRINGS[18]); // Invalid key
  }
    else if (bMessageOnCancel)
      // "Key you entered was not valid!" //18
      utils->ShowMessageU(KEYSTRINGS[18]);

    LicenseForm->Release();
}
  catch(...)
{
    return(false);
}

  return(bOK);
}
//---------------------------------------------------------------------------

bool __fastcall TLicenseKey::DecodeAndWriteKey(String Key, String Email )
// return true if success
//
// decode the Key provided, if it is good, store it
// and return true if stored successfully, otherwise,
// restore the previous key-vars.
{
  try
  {
    // Here we have a key user has entered manually.

    // Init License-Key Properties
    PK->Clear();

    // Set values for the old registry key
#if ERROR_MESSAGES
    int OldKeyCode = DecodeProductKey("", "");
#else
    DecodeProductKey("", "");
#endif

    // Save old key values, if any
    KeyClass SavePK = *PK;

    bool bOK = false;

    // Init License-Key Properties
    PK->Clear();

    // Check for a good key
    int NewKeyCode = DecodeProductKey(Key.Trim(), Email.Trim());

    // All fields populated?
    if (PK->Today <= 0 || PK->ActivationDays <= 0 ||
             PK->KeyInstallDay <= 0 || PK->KeyCreateDay <= 0)
    {
      // Restore values from registry-key
      *PK = SavePK;
      return(false);
    }

    if (NewKeyCode >= 0)
    {
      // Store a key the user entered if ForceKey is set, or if
      // it is a purchaced key, or if it is a trial-key that is
      // newer than the old key.
      if (PK->ForceKey || PK->PaidKey ||
            (!PK->PaidKey && PK->KeyCreateDay > SavePK.KeyCreateDay))
      {
        // Write the new key if it is valid...
        int StoreCode = StoreProductKey(Key);

        if (StoreCode >= 0)
          bOK = true;
        else if (StoreCode == -203)
          // "Hint: The first time you run this program,\n"
          // "you must have Administrator access rights!");
          utils->ShowMessageU(KEYSTRINGS[7]);
#if ERROR_MESSAGES
        else
          ShowMessage("License Key Storage Failed: " + String(StoreCode));
#endif
      }
    }
#if ERROR_MESSAGES
    else
      ShowMessage("DecodeProductKey(): Old key: " + String(OldKeyCode) +
                  "\nNew key: " + String(NewKeyCode));
#endif

    // bOK false if we could not write key...
    if (!bOK)
    {
      // Restore values from registry-key
      *PK = SavePK;
      return(false);
    }
  }
  catch(...)
  {
    return(false);
  }

  return(true);
}
//---------------------------------------------------------------------------

int __fastcall TLicenseKey::DecodeProductKey(String Key, String Email)
{
  bool bIgnoreForceKey = false;

  if (Key.IsEmpty())
  {
    Key = ReadKey(); // read from registry

    if (Key.Length() != L_D)
      return(-100);

    bIgnoreForceKey = true;
  }

  Key = UnMixUp(Key);

  // OK we have a string 18-chars long to check against
  // Target.
  if (Key.Length() == L_C)
  {
    // Don't do this for long key because the user's
    // computer-name chars are stored in it!!!!!!!!!!
    for (int ii = 1 ; ii <= L_C ; ii++) // use 18 NOT length!
    {
      if (Key[ii] >= 97 && Key[ii] <= 122) // lower case
        Key[ii] -= (char)(97-65);
      else if (Key[ii] < 48 || (Key[ii] > 57 && Key[ii] < 65) || Key[ii] > 90)
        return(-2);
    }
  }
  else if (Key.Length() != L_D)
    return(-1);

  int Sum = 0;
  for (int ii = 1 ; ii <= L_C ; ii++) // use 18 NOT Key.Length()!
    if (ii != 10)
      Sum += (char)Key[ii];

  Sum += DecodeBase36(String(Key[10]));
  Sum &= 0x1f; // only last 5 bits will fit in 0-35 range

  if (Sum)
    return(-3);

  String S1 = Key.SubString(1,2);
  String S2 = Key.SubString(3,3);
  String S3 = Key.SubString(6,4);

  int Temp = DecodeBase36(S1)+DecodeBase36(S2)+DecodeBase36(S3);

  // Divide key into fields
  int t1 = Temp & 0x1f; // Product ID
  int t2 = (Temp >> 5) & 0x1f; // Super Revision
  int t3 = (Temp >> 10) & 0x1f; // Flags (5-bits)
  int t4 = (Temp >> 15) & 0x1f; // Activation Mode (24-27 reserved)

  if (t1 != DEF_PRODUCT_ID)
    return(-4);

  // SuperRev of 31 means ignore it!
  if (t2 != 31 && t2 != DEF_SUPER_REV)
    return(-10);

  // Flags (Bit 0 = ForceKey, Bit 1 = PaidKey, Bit 2 = ExpireKey)
  int Flags = t3;

  // don't want forcekey if key from registry!
  if (bIgnoreForceKey)
    PK->ForceKey = false;
  else
    PK->ForceKey = (Flags & 1);

  PK->PaidKey = (Flags & 2);
  PK->ExpireKey = (Flags & 4);

  // ActivationMode
  int TempActMode = t4;

  // IF THE LSB IS SET, IGNORE E-MAIL ADDRESS!
  bool bIgnoreEmail;

  if ((TempActMode & 1) != 0)
  {
    bIgnoreEmail = true;
    TempActMode -= 1;
  }
  else
    bIgnoreEmail = false;

  // MODES MUST BE EVEN UNLESS EMAIL IS DISABLED
  int TempActDays = GetActivationDays(TempActMode);

  if (TempActDays == 0)
    return(-5);

  PK->ActivationDays = TempActDays;

  // Get Today's date
  unsigned short cy, cm, cd;
  Date().DecodeDate(&cy,&cm,&cd);

  // Get Today's date in Julian
  int cjd = JulianDayFromDate(cy, cm, cd);
  PK->Today = cjd;

  // Extract the key-date
  int TempCreateDay = DecodeBase36(Key.SubString(15,4)) + LK_CORRECTION_DAYS;
  PK->KeyCreateDay = TempCreateDay;

  // Extract the install-date
  int TempInstallDay;
  if (Key.Length() == L_D)
    TempInstallDay = DecodeBase36(Key.SubString(19,4)) + LK_CORRECTION_DAYS;
  else
    TempInstallDay = cjd; // For short-key set install-day = "today"

  PK->KeyInstallDay = TempInstallDay;

  // Get Spare (10 bits, bit 9 is "expired" flag,
  // bits 0-8 = 0-511 unsigned integer)
  int TempInt = DecodeBase36(Key.SubString(13,2));

//  int Spare = (TempInt & 0x1ff);

  PK->RegKeyIsExpired = (TempInt & 0x200) ? true : false;

#if ERROR_MESSAGES
  if (PK->RegKeyIsExpired)
    ShowMessage("Expired flag is set.");
  else
    ShowMessage("Expired flag is clear.");
#endif

  if (PK->RegKeyIsExpired)
    return(-7);

  // Skip further checks if forceing a key
  // (emergencies only!)
  if (PK->ForceKey)
    return(0);

  if (TempActDays != LK_MAX_DAYS) // Unlimited key?
  {
    // Today's date can't be earlier than key creation day!
    // Give the user 2 days to account for time-zone
    // diffs & up to a 24-hour slop...
    if (cjd < TempCreateDay-2)
      return(-6); // Message... user needs to set calendar

    // ok - now see if we have a long key with the actual
    // installation date... (as in a user's registry key,
    // not in a stub-key or new key the user is entering)
    if (Key.Length() == L_D)
    {
      // Install date can't be earlier than Key's create
      // date. Tampered key...
      if (TempInstallDay < TempCreateDay-2)
        return(-8);

      // User's time off?
      if (cjd < TempInstallDay-2)
        return(-6);

      // User's date expired...
      if (PK->ComputeDaysRemaining() <= 0)
      {
        // Is this key from the Registry?
        if (bIgnoreForceKey)
        {
          // Re-Write the old key with RegKeyIsExpired set...
          String NewS = EncodeBase36((TempInt | 0x200), 2);
          Key[13] = NewS[1];
          Key[14] = NewS[2];

          // Have to re-encode the checksum
          int Sum = 0;
          for (int ii = 1 ; ii <= L_C ; ii++) // use 18 NOT Key.Length()!
            if (ii != 10) // don't add the checksum char [10]
              Sum += (char)Key[ii];

          Sum = ~Sum + 1;
          Sum &= 0x1f; // only last 5 bits will fit in 0-35 range

          String T = EncodeBase36(Sum,1);
          Key[10] = T[1]; //10

          WriteKey(MixUp(Key));
        }

        PK->RegKeyIsExpired = true;
        return(-7);
      }
    }
  }

  if (!bIgnoreEmail && Key.Length() == L_C)
  {
    String EmailUpper = Email.UpperCase();
    EmailUpper = EmailUpper.Trim(); // Trim Spaces

    if (EmailUpper.Length() == 0)
      return(-8);

    // Do this ONLY when user is entering a new key!
    int EmailSum = DecodeBase36(Key.SubString(11,2));

    int Temp = 0;
    for (int ii = 1 ; ii <= EmailUpper.Length() ; ii++)
      Temp += (char)EmailUpper[ii];

    Temp &= 0x3ff; // 10-bits

    if (Temp != EmailSum)
      return(-9);
  }
  else if (Key.Length() == L_D)
  {
    // long 22 char key from registry we
    // have replaced the e-mail sum with the computer-name
    // first and last char.
    char* buf = new char[MAX_COMPUTERNAME_LENGTH+1];
    DWORD s = MAX_COMPUTERNAME_LENGTH+1;
    if (GetComputerName((LPTSTR)buf,&s))
    {
      String TS = String(buf);

      // Don't check if the field is 0
      if ((TS.Length() >= 1 && Key[12] != '0' && TS[1] != Key[12]) ||
            (TS.Length() >= 2 && Key[11] != '0' &&
                            TS[TS.Length()] != Key[11]))
      {
        delete [] buf;
        return(-10);
      }
    }
    delete [] buf;
  }

  return(0);
}
//---------------------------------------------------------------------------

void __fastcall TLicenseKey::ErrorCode(int Code)
{
  if (Code == -6)
    // "Please set your calendar to the proper date!"
    utils->ShowMessageU(KEYSTRINGS[8]);
  else if (Code == -7)
    // "Your activation has expired.\n"
    utils->ShowMessageU(KEYSTRINGS[9] +
        DTSColor->Iftf->Strings[INFO_WEB_SITE] + KEYSTRINGS[10]);
  else if (Code == -8)
    // "You must enter the E-mail address\n"
    // "you used when requesting your License Key!"
    utils->ShowMessageU(KEYSTRINGS[11]);
  else if (Code == -9)
    // "The E-mail address you entered is not the\n"
    // "same as that used to request your License Key!"
    utils->ShowMessageU(KEYSTRINGS[12]);
  else if (Code == -10)
    // "Your computer's name may have changed since\n"
    // "the last time you used YahCoLoRiZe.  You will need to\n"
    // "re-install the license key! For information visit:\n\n"
    utils->ShowMessageU(KEYSTRINGS[13] +
        DTSColor->Iftf->Strings[INFO_WEB_SITE]);
#if ERROR_MESSAGES
  else
    ShowMessage("License Key Validation Failed: " + String(Code));
#endif
}
//---------------------------------------------------------------------------

String __fastcall TLicenseKey::ReadKey(void)
{
  String Key = "";

  // Read key from registry
  try
  {
    const String RegKey = REGISTRY_KEY;
    TRegistry* MyRegistry = new TRegistry();
    MyRegistry->RootKey = HKEY_LOCAL_MACHINE;

    if(!MyRegistry->OpenKeyReadOnly(RegKey))
    {
      delete MyRegistry;
      return(-100);
    }

    // Try to Read product key
    try
    {
      Key = MyRegistry->ReadString("ProductKey");
    }
    catch(...) { }

    MyRegistry->CloseKey();
    delete MyRegistry;
  }
  catch(...) { }

  return Key.Trim();
}
//---------------------------------------------------------------------------
int __fastcall TLicenseKey::WriteKey(String Key)
{
  try
  {
    const String RegKey = REGISTRY_KEY;
    TRegistry* MyRegistry = new TRegistry();
    MyRegistry->RootKey = HKEY_LOCAL_MACHINE;

    // Open/Create key
    if(MyRegistry->OpenKey(RegKey, true))
    {
      MyRegistry->WriteString("ProductKey", Key);

      // Read back to be sure...
      String S = MyRegistry->ReadString("ProductKey");

      MyRegistry->CloseKey();

      if (S != Key) // Could not write...
      {
        delete MyRegistry;
        return -202;
      }
    }
    else // Could not open key for full-access, not an administrator
    {
      delete MyRegistry;
      return -203;
    }

    delete MyRegistry;
  }
  catch(...)
  {
    return -202;
  }

  return 0;
}
//---------------------------------------------------------------------------
int __fastcall TLicenseKey::GetActivationDays(int ActMode)
// ActMode range is 0-31
//
// The least significant two bits are flags...
// .. bit 0 if set means "ignore E-Mail"
// .. bit 1 is reserved and should be 0
{
  int ActDays;

  // filter out lsb flag-bits
  ActMode &= 0x1c;

  // 4,8,12,16,20,24,28 << available modes
  // 0 is illegal! (will cause algorithm error!)
  if (ActMode == 4) ActDays = 90; // 90 days
  else if (ActMode == 8) ActDays = 365 * 2; // 2 years
  else if (ActMode == 12) ActDays = 30; // 30-days
  else if (ActMode == 16) ActDays = 365 * 3; // 3 years
  else if (ActMode == 20) ActDays = LK_MAX_DAYS; // Unlimited
  else if (ActMode == 24) ActDays = 15; // 15-days
  else if (ActMode == 28) ActDays = 365; // 1 year
  else ActDays = 0; // error

  return ActDays;
}
//---------------------------------------------------------------------------
int __fastcall TLicenseKey::StoreProductKey(String Key)
{
  try
  {
    // Test for new installation
    String Temp = ReadKey();

    if (Temp.Length() != L_D)
    {
      // "Discrete-Time Systems will not be held liable\n"
      // "for any damages whatsoever including indirect,\n"
      // "direct, consequential, incidental, special damages\n"
      // "or loss of business profits, even if Discrete-Time Systems\n"
      // "(Mister Swift) has been apprised of the possibility\n"
      // "of said damages.\n\n"
      // "If you AGREE, press Yes, otherwise, press No.", //14
      int button =
        MessageBox(NULL, utils->Utf8ToWide(KEYSTRINGS[14]).c_bstr(),
            L"License", MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2|MB_APPLMODAL);

      if (button == IDNO) return -201;
    }

    Key = Key.Trim();

    if (Key.Length() != L_C) return -200;

    // Add first/last chars of computer-name
    // in place of e-mail checksum
    char * buf = new char[MAX_COMPUTERNAME_LENGTH+1];
    DWORD s = MAX_COMPUTERNAME_LENGTH+1;

    // REMEMBER the key at this point is
    // scrambled...so
    // Checksum [10] => 11
    // Email Sum [11] => 16
    // Email Sum [12] => 8
    // store 0's in the field meaning "do not check"
    Key[16] = '0';
    Key[8] = '0';

    if (GetComputerName((LPTSTR)buf,&s))
    {
      String TS = String(buf);
      if (TS.Length() >= 1)
        Key[8] = TS[1]; //12

      if (TS.Length() >= 2)
        Key[16] = TS[TS.Length()]; //11

      // Have to re-encode the checksum
      int Sum = 0;
      for (int ii = 1 ; ii <= Key.Length() ; ii++)
        if (ii != 11) // don't add the checksum char [10]
          Sum += (char)Key[ii];

      Sum = ~Sum + 1;
      Sum &= 0x1f; // only last 5 bits will fit in 0-35 range

      String T = EncodeBase36(Sum,1);
      Key[11] = T[1]; //10
    }

    delete [] buf;

    // Get Today's date
    unsigned short cy, cm, cd;
    Date().DecodeDate(&cy,&cm,&cd);

    // Get Today's date in Julian
    int cjd = JulianDayFromDate(cy, cm, cd);
    PK->Today = cjd;

    // Add into 1,5,10 and 15
    String X = EncodeBase36(cjd - LK_CORRECTION_DAYS, 4);

    Key = Key.Insert(String(X[1]),1);
    Key = Key.Insert(String(X[2]),5);
    Key = Key.Insert(String(X[3]),10);
    Key = Key.Insert(String(X[4]),15);

    int RetVal = WriteKey(Key);

    if (RetVal == 0) PK->KeyInstallDay = PK->Today;

    return RetVal;
  }
  catch(...)
  {
    return -204;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TLicenseKey::GetStub(String &Key, String &Email)
{
  String TempKey, TempEmail;

  // Init
  Key = "";
  Email = "";

  // Open our own .EXE in binary/read-only mode
  int iFileHandle = FileOpen(Application->ExeName, fmOpenRead);

  if (iFileHandle == NULL)
  {
    ShowMessage("Error opening: " + Application->ExeName);
    return false;
  }

  // Seek end
  #define SEEK_ITERATIONS 10

  int iFileLength = 0;
  int ii;

  // try several times - this has ocassionally been
  // producing an error...
  for (ii = 0 ; ii < SEEK_ITERATIONS ; ii++)
  {
    iFileLength = FileSeek(iFileHandle, 0, 2);

    if (iFileLength > DATA_STUB_SIZE)
      break;

// THIS MAY BE PROBLEMATIC IN WINDOWS 7 & 8!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//    Sleep(100); // delay
  }

  if (ii >= SEEK_ITERATIONS)
  {
    ShowMessage("Error seeking: " + Application->ExeName);
    FileClose(iFileHandle);
    return false;
  }

  // Seek back from end
  FileSeek(iFileHandle, -DATA_STUB_SIZE, 2);

  char* buf = new char[DATA_STUB_SIZE-8];
  if (FileRead(iFileHandle, buf, DATA_STUB_SIZE-8) != DATA_STUB_SIZE-8)
  {
    ShowMessage("Error reading: " + Application->ExeName);
    delete [] buf;
    FileClose(iFileHandle);
    return false;
  }

  int Checksum = 0;
  for (int ii = 0 ; ii < DATA_STUB_SIZE-8 ; ii++)
  {
    if (ii < 18) TempKey += (char)(buf[ii]+16); // key
    else TempEmail += (char)(buf[ii]+16); // e-mail

    Checksum += buf[ii];
  }

  delete [] buf;

  // Have to have something besides all 0s for key and e-mail!
  if (!Checksum)
  {
    FileClose(iFileHandle);
    return false;
  }

  // Read checksum
  int StoredChecksum;
  if (FileRead(iFileHandle, (char*)&StoredChecksum, 4) != 4)
  {
    ShowMessage("Error reading: " + Application->ExeName);
    FileClose(iFileHandle);
    return false;
  }

  Checksum += StoredChecksum;

  // Read the original length
  int OrigLen;
  if (FileRead(iFileHandle, (char*)&OrigLen, 4) != 4)
  {
    ShowMessage("Error reading: " + Application->ExeName);
    FileClose(iFileHandle);
    return false;
  }

  FileClose(iFileHandle);

  if ((Checksum & 0xffff) == 0 && iFileLength-OrigLen == DATA_STUB_SIZE)
  {
    // Trim spaces, return Key/Email
    Key = TempKey.Trim();
    Email = TempEmail.Trim();

    return true;
  }

  return false;
}
//---------------------------------------------------------------------------

String __fastcall TLicenseKey::EncodeBase36(int N, int Len)
{
  String S;

  int Out, Div;

  if (Len > 8)
    return("Bad    String ");

  for (int ii = Len-1 ; ii >= 0 ; ii--)
  {
    Div = pow((double)36,ii);

    Out = N/Div;

    if (Out)
      N %= Div;

    if (Out < 10)
      S += String(char(Out+48)); // 0-9
    else
      S += String(char((Out-10)+65)); // A-Z
  }

  return(S);
}
//---------------------------------------------------------------------------

int __fastcall TLicenseKey::DecodeBase36(String S)
{
  int Acc = 0;

  for (int ii = 1 ; ii <= S.Length() ; ii++)
  {
    // Convert A... to int 10...
    if (S[ii] > 57)
      S[ii] -= (char)55;
    else // 0-9
      S[ii] -= (char)48;

    Acc += S[ii] * pow((double)36,S.Length()-ii);
  }

  return(Acc);
}
//---------------------------------------------------------------------------

String __fastcall TLicenseKey::MixUp(String S)
{
  String InstallDay = "";

  if (S.Length() == L_D)
  {
    InstallDay = S.SubString(L_D-4+1,4);
    S = S.SubString(1,L_C);
  }
  else if (S.Length() != L_C)
    return("");

  String Temp = S; // Make Temp of equal length

  // Mix up the string-order
  Temp[1] = S[9];
  Temp[2] = S[17];
  Temp[3] = S[14];
  Temp[4] = S[3];
  Temp[5] = S[8];
  Temp[6] = S[4];
  Temp[7] = S[1];
  Temp[8] = S[12];
  Temp[9] = S[16];
  Temp[10] = S[15];
  Temp[11] = S[10];
  Temp[12] = S[18];
  Temp[13] = S[2];
  Temp[14] = S[7];
  Temp[15] = S[5];
  Temp[16] = S[11];
  Temp[17] = S[6];
  Temp[18] = S[13];

  if (InstallDay.Length() == 4)
  {
    Temp = Temp.Insert(String(InstallDay[1]),1);
    Temp = Temp.Insert(String(InstallDay[2]),5);
    Temp = Temp.Insert(String(InstallDay[3]),10);
    Temp = Temp.Insert(String(InstallDay[4]),15);
  }

  return(Temp);
}
//---------------------------------------------------------------------------

String __fastcall TLicenseKey::UnMixUp(String S)
// UnMix the string-order
{
  // long key pluck out 4 hidden chars
  if (S.Length() == L_D)
  {
    String X;
    X += String(S[1]);
    X += String(S[5]);
    X += String(S[10]);
    X += String(S[15]);
    S = S.Delete(15,1);
    S = S.Delete(10,1);
    S = S.Delete(5,1);
    S = S.Delete(1,1);
    S += X;
  }
  else if (S.Length() != L_C) return "";

  String Temp = S;

  Temp[9] = S[1];
  Temp[17] = S[2];
  Temp[14] = S[3];
  Temp[3] = S[4];
  Temp[8] = S[5];
  Temp[4] = S[6];
  Temp[1] = S[7];
  Temp[12] = S[8];
  Temp[16] = S[9];
  Temp[15] = S[10];
  Temp[10] = S[11];
  Temp[18] = S[12];
  Temp[2] = S[13];
  Temp[7] = S[14];
  Temp[5] = S[15];
  Temp[11] = S[16];
  Temp[6] = S[17];
  Temp[13] = S[18];

  return Temp;
}
//---------------------------------------------------------------------------
int __fastcall TLicenseKey::JulianDayFromDate(int Y, int M, int D)
{
  return 1461*(int)((Y+4800)/4) + 365*((Y+4800) % 4) +
                  (int)((7*(M-2))/12) + 30 * M + D - 32173;
}
//---------------------------------------------------------------------------

