#ifndef COMBO_BOX_H
#define COMBO_BOX_H

template<typename T, typename U>
T cbFindStringExact(HWND hDlg, int id, U str) {
    return (T) SendDlgItemMessage(hDlg, id, CB_FINDSTRINGEXACT, 0, (LPARAM) str);
}


template<typename T, unsigned N = 256>
std::array<TCHAR,N> cbGetLbText(HWND hDlg, int id, T i) {
    std::array<TCHAR,N> buf {};
    SendDlgItemMessage(hDlg, id, CB_GETLBTEXT, (WPARAM) i, (LPARAM)buf.data());
    return buf;
}


template<typename T>
LRESULT cbAddString(HWND hDlg, int id, T str) {
    return SendDlgItemMessage(hDlg, id, CB_ADDSTRING, 0 , (LPARAM) str);
}


template<typename T, typename U>
T cbGetItemData(HWND hDlg, int id, U index) {
    return (T) SendDlgItemMessage(hDlg, id, CB_GETITEMDATA, (WPARAM) index, 0);
}


inline void cbResetContext(HWND hDlg, int id) {
    SendDlgItemMessage(hDlg, id, CB_RESETCONTENT, 0, 0);
}


template<typename T>
void cbSetCurSel(HWND hDlg, int id, T index) {
    SendDlgItemMessage(hDlg, id, CB_SETCURSEL, (WPARAM) index, 0);
}


template<typename T = int>
T cbGetCurSel(HWND hDlg, int id) {
    return (T) SendDlgItemMessage(hDlg, id, CB_GETCURSEL, 0, 0);
}


template<typename T = int>
T cbGetCount(HWND hDlg, int id) {
    return (T) SendDlgItemMessage(hDlg, id, CB_GETCOUNT, 0, 0);
}

#endif
