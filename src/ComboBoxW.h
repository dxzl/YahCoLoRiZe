/*****************************************************************************\
*                                                                             *
* ComboBoxW.h - Macro APIs, window message crackers, and control APIs         *
*                                                                             *
*               Version Win32 / Windows NT                                    *
*                                                                             *
*               Mr. Swift                                                     *
*                                                                             *
\*****************************************************************************/

#ifndef _INC_COMBOBOXW
#define _INC_COMBOBOXW

#define SNDMSGW ::SendMessageW

/****** ComboBox control message APIs ****************************************/

#define ComboBoxW_Focused(hwndCtl)       ((GetFocus()) == (hwndCtl) ? TRUE : FALSE)
#define ComboBoxW_Show(hwndCtl)       ShowWindow((hwndCtl), (SW_SHOW))
#define ComboBoxW_Hide(hwndCtl)       ShowWindow((hwndCtl), (SW_HIDE))
#define ComboBoxW_Enable(hwndCtl, fEnable)       EnableWindowW((hwndCtl), (fEnable))

#define ComboBoxW_GetText(hwndCtl, lpch, cchMax) GetWindowTextW((hwndCtl), (lpch), (cchMax))
#define ComboBoxW_GetTextLength(hwndCtl)         GetWindowTextLength(hwndCtl)
#define ComboBoxW_SetText(hwndCtl, lpsz)         SetWindowTextW((hwndCtl), (lpsz))

#define ComboBoxW_LimitText(hwndCtl, cchLimit)   ((int)(DWORD)SNDMSGW((hwndCtl), CB_LIMITTEXT, (WPARAM)(int)(cchLimit), 0L))

// LOWORD has start of selected HIWORD has first char after end
#define ComboBoxW_GetEditSel(hwndCtl)            ((DWORD)SNDMSGW((hwndCtl), CB_GETEDITSEL, 0L, 0L))
#define ComboBoxW_SetEditSel(hwndCtl, ichStart, ichEnd) ((int)(DWORD)SNDMSGW((hwndCtl), CB_SETEDITSEL, 0L, MAKELPARAM((ichStart), (ichEnd))))
#define ComboBoxW_SelStart(hwndCtl)             (LOWORD(ComboBoxW_GetEditSel(hwndCtl)))
#define ComboBoxW_SelEnd(hwndCtl)               (HIWORD(ComboBoxW_GetEditSel(hwndCtl)))

#define ComboBoxW_GetCount(hwndCtl)              ((int)(DWORD)SNDMSGW((hwndCtl), CB_GETCOUNT, 0L, 0L))
#define ComboBoxW_ResetContent(hwndCtl)          ((int)(DWORD)SNDMSGW((hwndCtl), CB_RESETCONTENT, 0L, 0L))

#define ComboBoxW_AddString(hwndCtl, lpsz)       ((int)(DWORD)SNDMSGW((hwndCtl), CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)(lpsz)))
#define ComboBoxW_InsertString(hwndCtl, index, lpsz) ((int)(DWORD)SNDMSGW((hwndCtl), CB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(LPCTSTR)(lpsz)))

#define ComboBoxW_AddItemData(hwndCtl, data)     ((int)(DWORD)SNDMSGW((hwndCtl), CB_ADDSTRING, 0L, (LPARAM)(data)))
#define ComboBoxW_InsertItemData(hwndCtl, index, data) ((int)(DWORD)SNDMSGW((hwndCtl), CB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(data)))

#define ComboBoxW_DeleteString(hwndCtl, index)   ((int)(DWORD)SNDMSGW((hwndCtl), CB_DELETESTRING, (WPARAM)(int)(index), 0L))

#define ComboBoxW_GetLBTextLen(hwndCtl, index)           ((int)(DWORD)SNDMSGW((hwndCtl), CB_GETLBTEXTLEN, (WPARAM)(int)(index), 0L))
#define ComboBoxW_GetLBText(hwndCtl, index, lpszBuffer)  ((int)(DWORD)SNDMSGW((hwndCtl), CB_GETLBTEXT, (WPARAM)(int)(index), (LPARAM)(LPCTSTR)(lpszBuffer)))

#define ComboBoxW_GetItemData(hwndCtl, index)        ((LRESULT)(ULONG_PTR)SNDMSGW((hwndCtl), CB_GETITEMDATA, (WPARAM)(int)(index), 0L))
#define ComboBoxW_SetItemData(hwndCtl, index, data)  ((int)(DWORD)SNDMSGW((hwndCtl), CB_SETITEMDATA, (WPARAM)(int)(index), (LPARAM)(data)))

#define ComboBoxW_FindString(hwndCtl, indexStart, lpszFind)  ((int)(DWORD)SNDMSGW((hwndCtl), CB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszFind)))
#define ComboBoxW_FindItemData(hwndCtl, indexStart, data)    ((int)(DWORD)SNDMSGW((hwndCtl), CB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data)))

#define ComboBoxW_GetCurSel(hwndCtl)                 ((int)(DWORD)SNDMSGW((hwndCtl), CB_GETCURSEL, 0L, 0L))
#define ComboBoxW_SetCurSel(hwndCtl, index)          ((int)(DWORD)SNDMSGW((hwndCtl), CB_SETCURSEL, (WPARAM)(int)(index), 0L))

// Searches for item (starting at index in the list) that begins with "some text" and if found,
// selects that item and displays it in the edit-box
#define ComboBoxW_SelectString(hwndCtl, indexStart, lpszSelect)  ((int)(DWORD)SNDMSGW((hwndCtl), CB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszSelect)))
#define ComboBoxW_SelectItemData(hwndCtl, indexStart, data)      ((int)(DWORD)SNDMSGW((hwndCtl), CB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data)))

#define ComboBoxW_Dir(hwndCtl, attrs, lpszFileSpec)  ((int)(DWORD)SNDMSGW((hwndCtl), CB_DIR, (WPARAM)(UINT)(attrs), (LPARAM)(LPCTSTR)(lpszFileSpec)))

#define ComboBoxW_ShowDropdown(hwndCtl, fShow)       ((BOOL)(DWORD)SNDMSGW((hwndCtl), CB_SHOWDROPDOWN, (WPARAM)(BOOL)(fShow), 0L))

#if (WINVER >= 0x030a)
#define ComboBoxW_FindStringExact(hwndCtl, indexStart, lpszFind)  ((int)(DWORD)SNDMSGW((hwndCtl), CB_FINDSTRINGEXACT, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszFind)))

#define ComboBoxW_GetDroppedState(hwndCtl)           ((BOOL)(DWORD)SNDMSGW((hwndCtl), CB_GETDROPPEDSTATE, 0L, 0L))
#define ComboBoxW_GetDroppedControlRect(hwndCtl, lprc) ((void)SNDMSGW((hwndCtl), CB_GETDROPPEDCONTROLRECT, 0L, (LPARAM)(RECT *)(lprc)))

#define ComboBoxW_GetItemHeight(hwndCtl)             ((int)(DWORD)SNDMSGW((hwndCtl), CB_GETITEMHEIGHT, 0L, 0L))
#define ComboBoxW_SetItemHeight(hwndCtl, index, cyItem) ((int)(DWORD)SNDMSGW((hwndCtl), CB_SETITEMHEIGHT, (WPARAM)(int)(index), (LPARAM)(int)cyItem))

#define ComboBoxW_GetExtendedUI(hwndCtl)             ((UINT)(DWORD)SNDMSGW((hwndCtl), CB_GETEXTENDEDUI, 0L, 0L))
#define ComboBoxW_SetExtendedUI(hwndCtl, flags)      ((int)(DWORD)SNDMSGW((hwndCtl), CB_SETEXTENDEDUI, (WPARAM)(UINT)(flags), 0L))
#endif  /* WINVER >= 0x030a */

/****** Edit control message APIs ********************************************/

#define EditW_Enable(hwndCtl, fEnable)           EnableWindow((hwndCtl), (fEnable))

#define EditW_GetText(hwndCtl, lpch, cchMax)     GetWindowTextW((hwndCtl), (lpch), (cchMax))
#define EditW_GetTextLength(hwndCtl)             GetWindowTextLength(hwndCtl)
#define EditW_SetText(hwndCtl, lpsz)             SetWindowTextW((hwndCtl), (lpsz))

#define EditW_LimitText(hwndCtl, cchMax)         ((void)SNDMSGW((hwndCtl), EM_LIMITTEXT, (WPARAM)(cchMax), 0L))

#define EditW_GetLineCount(hwndCtl)              ((int)(DWORD)SNDMSGW((hwndCtl), EM_GETLINECOUNT, 0L, 0L))
#ifndef _MAC
#define EditW_GetLine(hwndCtl, line, lpch, cchMax) ((*((int *)(lpch)) = (cchMax)), ((int)(DWORD)SNDMSGW((hwndCtl), EM_GETLINE, (WPARAM)(int)(line), (LPARAM)(LPTSTR)(lpch))))
#else
#define EditW_GetLine(hwndCtl, line, lpch, cchMax) ((*((WORD *)(lpch)) = (cchMax)), ((int)(DWORD)SNDMSGW((hwndCtl), EM_GETLINE, (WPARAM)(int)(line), (LPARAM)(LPTSTR)(lpch))))
#endif

#define EditW_GetRect(hwndCtl, lprc)             ((void)SNDMSGW((hwndCtl), EM_GETRECT, 0L, (LPARAM)(RECT *)(lprc)))
#define EditW_SetRect(hwndCtl, lprc)             ((void)SNDMSGW((hwndCtl), EM_SETRECT, 0L, (LPARAM)(const RECT *)(lprc)))
#define EditW_SetRectNoPaint(hwndCtl, lprc)      ((void)SNDMSGW((hwndCtl), EM_SETRECTNP, 0L, (LPARAM)(const RECT *)(lprc)))

// LOWORD has start of selected HIWORD has first char after end
#define EditW_GetSel(hwndCtl)                    ((DWORD)SNDMSGW((hwndCtl), EM_GETSEL, 0L, 0L))
#define EditW_SetSel(hwndCtl, ichStart, ichEnd)  ((void)SNDMSGW((hwndCtl), EM_SETSEL, (ichStart), (ichEnd)))
#define EditW_ReplaceSel(hwndCtl, lpszReplace)   ((void)SNDMSGW((hwndCtl), EM_REPLACESEL, 0L, (LPARAM)(LPCTSTR)(lpszReplace)))

#define EditW_GetModify(hwndCtl)                 ((BOOL)(DWORD)SNDMSGW((hwndCtl), EM_GETMODIFY, 0L, 0L))
#define EditW_SetModify(hwndCtl, fModified)      ((void)SNDMSGW((hwndCtl), EM_SETMODIFY, (WPARAM)(UINT)(fModified), 0L))

#define EditW_ScrollCaret(hwndCtl)               ((BOOL)(DWORD)SNDMSGW((hwndCtl), EM_SCROLLCARET, 0, 0L))

#define EditW_LineFromChar(hwndCtl, ich)         ((int)(DWORD)SNDMSGW((hwndCtl), EM_LINEFROMCHAR, (WPARAM)(int)(ich), 0L))
#define EditW_LineIndex(hwndCtl, line)           ((int)(DWORD)SNDMSGW((hwndCtl), EM_LINEINDEX, (WPARAM)(int)(line), 0L))
#define EditW_LineLength(hwndCtl, line)          ((int)(DWORD)SNDMSGW((hwndCtl), EM_LINELENGTH, (WPARAM)(int)(line), 0L))

#define EditW_Scroll(hwndCtl, dv, dh)            ((void)SNDMSGW((hwndCtl), EM_LINESCROLL, (WPARAM)(dh), (LPARAM)(dv)))

#define EditW_CanUndo(hwndCtl)                   ((BOOL)(DWORD)SNDMSGW((hwndCtl), EM_CANUNDO, 0L, 0L))
#define EditW_Undo(hwndCtl)                      ((BOOL)(DWORD)SNDMSGW((hwndCtl), EM_UNDO, 0L, 0L))
#define EditW_EmptyUndoBuffer(hwndCtl)           ((void)SNDMSGW((hwndCtl), EM_EMPTYUNDOBUFFER, 0L, 0L))

#define EditW_SetPasswordChar(hwndCtl, ch)       ((void)SNDMSGW((hwndCtl), EM_SETPASSWORDCHAR, (WPARAM)(UINT)(ch), 0L))

#define EditW_SetTabStops(hwndCtl, cTabs, lpTabs) ((void)SNDMSGW((hwndCtl), EM_SETTABSTOPS, (WPARAM)(int)(cTabs), (LPARAM)(const int *)(lpTabs)))

#define EditW_FmtLines(hwndCtl, fAddEOL)         ((BOOL)(DWORD)SNDMSGW((hwndCtl), EM_FMTLINES, (WPARAM)(BOOL)(fAddEOL), 0L))

#define EditW_GetHandle(hwndCtl)                 ((HLOCAL)(UINT_PTR)SNDMSGW((hwndCtl), EM_GETHANDLE, 0L, 0L))
#define EditW_SetHandle(hwndCtl, h)              ((void)SNDMSGW((hwndCtl), EM_SETHANDLE, (WPARAM)(UINT_PTR)(HLOCAL)(h), 0L))

// Comclt32.dll version 6.0 only!
//#define EditW_SetCueBannerTextFocused(hwndCtl, lpcwText, fDrawFocused) ((void)SNDMSGW((hwndCtl), EM_SETCUEBANNER, (WPARAM)(fDrawFocused), (LPARAM)(lpcwText)))

#if (WINVER >= 0x030a)
#define EditW_GetFirstVisibleLine(hwndCtl)       ((int)(DWORD)SNDMSGW((hwndCtl), EM_GETFIRSTVISIBLELINE, 0L, 0L))

#define EditW_SetReadOnly(hwndCtl, fReadOnly)    ((BOOL)(DWORD)SNDMSGW((hwndCtl), EM_SETREADONLY, (WPARAM)(BOOL)(fReadOnly), 0L))

#define EditW_GetPasswordChar(hwndCtl)           ((TCHAR)(DWORD)SNDMSGW((hwndCtl), EM_GETPASSWORDCHAR, 0L, 0L))

#define EditW_SetWordBreakProc(hwndCtl, lpfnWordBreak) ((void)SNDMSGW((hwndCtl), EM_SETWORDBREAKPROC, 0L, (LPARAM)(EDITWORDBREAKPROC)(lpfnWordBreak)))
#define EditW_GetWordBreakProc(hwndCtl)          ((EDITWORDBREAKPROC)SNDMSGW((hwndCtl), EM_GETWORDBREAKPROC, 0L, 0L))
#endif /* WINVER >= 0x030a */

#endif /* _INC_COMBOBOXW */