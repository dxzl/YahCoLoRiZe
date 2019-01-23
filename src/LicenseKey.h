//---------------------------------------------------------------------------
#ifndef LicenseKeyH
#define LicenseKeyH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------

#define DATA_STUB_EXPECTED true

// Product ID is unique to each product from DTS
// Super Rev. is bumped up (hidden from user) to draw a
// line between possibly incompatible sub-versions
// of the Product. So THIS program will run ONLY with
// a key with the proper Product ID and Super-Rev #.

// Product ID... (1 = SwiftMiX, 2 = YahCoLoRiZe, 3 = Trinity)
#define DEF_PRODUCT_ID 2
#define DEF_SUPER_REV 3 // bumped from 2 to 3 for Unicode Version 7.16

#define ERROR_MESSAGES false

#define TRIAL_EMAIL "yahcolorize@trial.com"

// License Key
#define L_C 18
#define L_D 22
#define DATA_STUB_SIZE 56

// Max days in a 3-digit Base36 number
#define LK_MAX_DAYS 1679615

// Days to subtract from current julian day (to make it fit)
#define LK_CORRECTION_DAYS 2000000

// Days over which we display "unlimited" in About dialog
// Dec 31, 2099 - Jan 1, 1980 (around 120 years)
#define LK_DISPLAY_UNLIMITTED_DAYS (2488082-2444588)
//---------------------------------------------------------------------------

class KeyClass
{
friend class TDTSColor;

private:

  int today;
  int keyInstallDay;
  int keyCreateDay;
  int activationDays;
  bool regKeyIsExpired;

  // Flags set by KeyGen
  bool forceKey;
  bool paidKey;
  bool expireKey;

public:

  __fastcall KeyClass(void); // Constructor

  int __fastcall ComputeDaysRemaining( void);
  void __fastcall Clear( void);

  // License-Key properties
  __property int Today = { read = today, write = today };
  __property int KeyInstallDay = { read = keyInstallDay, write = keyInstallDay };
  __property int KeyCreateDay = { read = keyCreateDay, write = keyCreateDay };
  __property int ActivationDays = { read = activationDays, write = activationDays };
  __property bool RegKeyIsExpired = { read = regKeyIsExpired, write = regKeyIsExpired };

  __property bool ForceKey = { read = forceKey, write = forceKey };
  __property bool PaidKey = { read = paidKey, write = paidKey };
  __property bool ExpireKey = { read = expireKey, write = expireKey };
};
//---------------------------------------------------------------------------

class TLicenseKey
{
private:

  // License Key Validation
  bool __fastcall GetStub(String &Key, String &Email);
  int __fastcall StoreProductKey(String Key);
  int __fastcall DecodeProductKey(String S, String Email);
  String __fastcall GetTitle(TObject * Sender);
  String __fastcall ReadKey(void);
  int __fastcall WriteKey(String Key);
  void __fastcall ErrorCode(int Code);
  int __fastcall DecodeBase36(String S);
  String __fastcall EncodeBase36(int N, int Len);
  String __fastcall MixUp(String S);
  String __fastcall UnMixUp(String S);

  int __fastcall JulianDayFromDate(int Y, int M, int D);
  bool __fastcall IsTrialExpired(int InstallDate);
  bool __fastcall ReadInstallDate(int &InstallDate);
  int __fastcall ActModeToActDays(int ActMode);
  int __fastcall GetActivationDays(int ActMode);
  bool __fastcall DecodeAndWriteKey(String Key, String Email);
  void __fastcall WriteStub( String StubKey, String StubEmail,
                                                       KeyClass * StubPK);

public:

  __fastcall TLicenseKey(void);

  bool __fastcall ValidateLicenseKey(bool bPrompt);
  bool __fastcall DoKey(bool bMessageOnCancel = true);
};
//---------------------------------------------------------------------------

extern KeyClass * PK;
#endif
