#include "stdafx.h"
#include "Config.h"
#include "Hdc.h"
#include "Font.h"
#include "ResourcelessDialog.h"
#include "ComboBox.h"
#include "WinMain.h"
#include "Util.h"
#include "PropertySheetPageColor.h"

namespace {

Config config;

using namespace ResourcelessDialogTypes;

enum : WORD {
    IDC_NULL    = (WORD) -1,
    IDC_DUMMY   = 0x0100,
    IDC_FONT,
    IDC_BTN_FOREGROUND,
    IDC_TEXTBACK,
    IDC_BTN_BACKGROUND,
    IDC_FONT_QUALITY,
    IDC_FONT_SIZE,
    IDC_BOLD,
    IDC_ITALIC,
    IDC_LINESPACING,
    IDC_SPIN_LINESPACING,
    IDC_BTN_EXIT,
    IDC_WIDTH,
    IDC_SPIN_WIDTH,
    IDC_SPIN_HEIGHT,
    IDC_XPOS,
    IDC_SPIN_XPOS,
    IDC_YPOS,
    IDC_SPIN_YPOS,
    IDC_FORMAT,
};

const DWORD spinSt      = UDS_WRAP | UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_AUTOBUDDY | UDS_ARROWKEYS;
const DWORD chkBoxSt    = BS_AUTOCHECKBOX | WS_TABSTOP | WS_CHILD;

const ResourcelessDialog::Data widgets[] = {
    { TYPE_GROUPBOX,    _T("Text Options"),         IDC_NULL,                 7,   3, 295,  78, },
    { TYPE_TEXT,        _T("Font:"),                IDC_NULL,                24,  20,  26,  10, },
    { TYPE_COMBOBOX,    _T(""),                     IDC_FONT,                55,  18,  86, 120, CBS_SORT },
    { TYPE_TEXT,        _T("Color:"),               IDC_NULL,               159,  20,  55,  10, },
    { TYPE_PUSHBUTTON,  _T("..."),                  IDC_BTN_FOREGROUND,     215,  17,  60,  13, BS_OWNERDRAW },
    { TYPE_TEXT,        _T("Quality:"),             IDC_NULL,                24,  40,  26,  10, },
    { TYPE_COMBOBOX,    _T(""),                     IDC_FONT_QUALITY,        55,  38,  86, 120, },
    { TYPE_TEXT,        _T("Background:"),          IDC_TEXTBACK,           159,  40,  55,  10, },
    { TYPE_PUSHBUTTON,  _T("..."),                  IDC_BTN_BACKGROUND,     215,  37,  60,  13, BS_OWNERDRAW },
    { TYPE_TEXT,        _T("Size:"),                IDC_NULL,                24,  60,  26,  10, },
    { TYPE_COMBOBOX,    _T(""),                     IDC_FONT_SIZE,           55,  58,  38, 120, },
    { TYPE_CTRL_BUTTON, _T("Bold"),                 IDC_BOLD,               106,  59,  33,  12, chkBoxSt },
    { TYPE_CTRL_BUTTON, _T("Italic"),               IDC_ITALIC,             152,  59,  33,  12, chkBoxSt },
    { TYPE_TEXT,        _T("Line Spacing:"),        IDC_NULL,               193,  60,  47,  11, },
    { TYPE_EDITTEXT,    _T("0"),                    IDC_LINESPACING,        247,  59,  28,  11, ES_AUTOHSCROLL },
    { TYPE_CTRL_UPDOWN, _T(""),                     IDC_SPIN_LINESPACING,   275,  59,   9,  13, spinSt },

    { TYPE_GROUPBOX,    _T("Position"),             IDC_NULL,                 7,  86, 295,  65, },
    { TYPE_TEXT,        _T("Width:"),               IDC_NULL,                16, 105,  40,  11, },
    { TYPE_EDITTEXT,    _T(""),                     IDC_NULL,                62, 103,  28,  13, },
    { TYPE_CTRL_UPDOWN, _T(""),                     IDC_SPIN_WIDTH,          79, 103,  11,  13, spinSt },

    { TYPE_TEXT,        _T("Height:"),              IDC_NULL,                97, 105,  40,  11, },
    { TYPE_EDITTEXT,    _T(""),                     IDC_NULL,               143, 103,  28,  13, },
    { TYPE_CTRL_UPDOWN, _T(""),                     IDC_SPIN_HEIGHT,        160, 103,  11,  13, spinSt },

    { TYPE_TEXT,        _T("Horizontal:"),          IDC_NULL,                16, 130,  40,  11, },
    { TYPE_EDITTEXT,    _T(""),                     IDC_XPOS,                62, 128,  28,  13, },
    { TYPE_CTRL_UPDOWN, _T(""),                     IDC_SPIN_XPOS,           78, 128,  11,  13, spinSt },

    { TYPE_TEXT,        _T("Vertical:"),            IDC_NULL,                97, 130,  40,  11, },
    { TYPE_EDITTEXT,    _T(""),                     IDC_YPOS,               143, 128,  28,  13, },
    { TYPE_CTRL_UPDOWN, _T(""),                     IDC_SPIN_YPOS,          160, 128,  11,  13, spinSt },

    { TYPE_TEXT,        _T("strftime() Format:"),   IDC_NULL,                 7, 157,  61,  11 },
    { TYPE_EDITTEXT,    _T(""),                     IDC_FORMAT,              90, 157, 212,  12, ES_AUTOHSCROLL },
};


void initComboFont(HWND hDlg, int iDlgItem) {
    {
        auto hdc = Hdc::getDc(nullptr);

        const BYTE cs[] = {
              static_cast<BYTE>(GetTextCharset(hdc))
            , OEM_CHARSET
            , DEFAULT_CHARSET
        };

        for(auto c : cs) {
            auto hCombo = GetDlgItem(hDlg, iDlgItem);

            LOGFONT lf {};
            lf.lfCharSet = c;

            Font::enumFontFamiliesEx(hdc, lf, [&](
                  const ENUMLOGFONTEX* pelf
                , const NEWTEXTMETRICEX*
                , DWORD
            ) -> BOOL {
                const auto& l = pelf->elfLogFont;
                const auto facename = l.lfFaceName;
                if(   facename[0] != _T('@')
                   && LB_ERR == sendMessage(
                         hCombo
                       , CB_FINDSTRINGEXACT
                       , 0
                       , facename
                    )
                ) {
                    const auto index = sendMessage<int>(
                          hCombo
                        , CB_ADDSTRING
                        , 0
                        , facename
                    );
                    if(index >= 0) {
                        sendMessage(
                              hCombo
                            , CB_SETITEMDATA
                            , index
                            , l.lfCharSet
                        );
                    }
                }
                return 1;
            });
        }
    }

    {
        const int i = [&]() {
            const auto& x = config.font.name;
            auto i = cbFindStringExact<int>(hDlg, iDlgItem, x.data());
            if(i == LB_ERR) {
                i = 0;
            }
            return i;
        }();

        cbSetCurSel(hDlg, iDlgItem, i);
    }
}


void setComboFontSize(
      HWND hDlg
    , int iDlgItem
    , int initValue
    , const TCHAR* faceName
    , int charSet
) {
    const DWORD oldSize = [&]() {
        DWORD x = 0;
        if(initValue > 0) {
            x = initValue;
        } else {
            const auto curSel = cbGetCurSel(hDlg, iDlgItem);
            x = _tstoi(cbGetLbText(hDlg, iDlgItem, curSel).data());
        }
        if(x == 0) {
            x = 9;
        }
        return x;
    }();

    // note : cbResetContent() must be called after retrieving the oldSize.
    cbResetContext(hDlg, iDlgItem);

    {
        auto hdc = Hdc::getDc(nullptr);

        LOGFONT lf {};
        _tcscpy_s(lf.lfFaceName, faceName);
        lf.lfCharSet = static_cast<BYTE>(charSet);

        const auto hCombo = GetDlgItem(hDlg, iDlgItem);
        const auto logpixelsy = GetDeviceCaps(hdc, LOGPIXELSY);

        Font::enumFontFamiliesEx(hdc, lf, [&](
              const ENUMLOGFONTEX*
            , const NEWTEXTMETRICEX* lpntm
            , DWORD fontType
        ) -> BOOL {
            const auto ftt = fontType & (TRUETYPE_FONTTYPE | RASTER_FONTTYPE);
            if(ftt != RASTER_FONTTYPE) {
                static const int nFontSizes[] = {
                       4,  5,  6,  7,  8,  9, 10, 11, 12, 14
                    , 16, 18, 20, 22, 24, 26, 28, 36, 48, 72
                };
                for(const auto c : nFontSizes) {
                    auto s = formatString(_T("%d"), c);
                    sendMessage(hCombo, CB_ADDSTRING, 0, s.data());
                }
                return FALSE;
            }

            const int num = (
                    lpntm->ntmTm.tmHeight - lpntm->ntmTm.tmInternalLeading
                ) * 72 / logpixelsy;
            const auto count = sendMessage<int>(hCombo, CB_GETCOUNT);
            for(int i = 0; i < count; ++i) {
                TCHAR s[80] {};
                sendMessage(hCombo, CB_GETLBTEXT, i, s);
                const auto si = _tstoi(s);
                if(num == si) {
                    return TRUE;
                } else if(num < si) {
                    auto x = formatString(_T("%d"), num);
                    sendMessage(hCombo, CB_INSERTSTRING, i, x.data());
                    return TRUE;
                }
            }
            auto x = formatString(_T("%d"), num);
            sendMessage(hCombo, CB_ADDSTRING, 0, x.data());
            return TRUE;
        });
    }

    auto size = oldSize;
    for(; size > 0; --size) {
        auto x = formatString(_T("%d"), size);
        const auto i = cbFindStringExact<int>(
              hDlg
            , iDlgItem
            , x.data()
        );
        if(i != LB_ERR) {
            cbSetCurSel(hDlg, iDlgItem, i);
            break;
        }
    }
    if(size == 0) {
        cbSetCurSel(hDlg, iDlgItem, 0);
    }
}


void onInit(HWND hDlg) {
    config = loadConfig();

    if(auto hfont = GetStockObject(DEFAULT_GUI_FONT)) {
        sendDlgItemMessage(hDlg, IDC_FONT, WM_SETFONT, hfont, 0);
    }

    initComboFont(hDlg, IDC_FONT);

    {
        const auto charSet = cbGetItemData<int>(
              hDlg
            , IDC_FONT
            , cbGetCurSel(hDlg, IDC_FONT)
        );
        const auto fontSize = std::max(6, config.font.size);

        setComboFontSize(
              hDlg
            , IDC_FONT_SIZE
            , fontSize
            , config.font.name.c_str()
            , charSet
        );
    }

    {
        const auto& setRangePos = [&](int id, int maxV, int minV, int pos) {
            sendDlgItemMessage(hDlg, id, UDM_SETRANGE, 0, MAKELONG(maxV, minV));
            sendDlgItemMessage(hDlg, id, UDM_SETPOS, 0, pos);
        };
        setRangePos(IDC_SPIN_HEIGHT     , 200 , -64, config.text.height);
        setRangePos(IDC_SPIN_WIDTH      ,  64 , -64, config.text.width);
        setRangePos(IDC_SPIN_LINESPACING,  64 , -64, config.text.lineSpacing);
        setRangePos(IDC_SPIN_YPOS       ,  64 , -64, config.text.yPos);
        setRangePos(IDC_SPIN_XPOS       ,  64 , -64, config.text.xPos);
    }

    {
        auto hwndTextBack = GetDlgItem(hDlg, IDC_TEXTBACK);
        if(hwndTextBack) {
            if(config.explore.isXpStyle) {
                auto style = getWindowLongPtr(hwndTextBack, GWL_STYLE);
                style |= WS_DISABLED;
                setWindowLongPtr(hwndTextBack, GWL_STYLE, style);
            }
        }
    }

    {
        static const TCHAR* quals[] = {
            _T("Default (Win2000)"),
            _T("Draft"),
            _T("Proof"),
            _T("NonAntiAliased"),
            _T("AntiAliased (Win7))"),
            _T("ClearType (WinXP)"),
            _T("ClearType Natural"),
        };
        for(const auto& e : quals) {
            cbAddString(hDlg, IDC_FONT_QUALITY, e);
        }
        cbSetCurSel(hDlg, IDC_FONT_QUALITY, config.font.quality);
    }

    {
        const auto& checkButton = [&](int id, int b) {
            CheckDlgButton(hDlg, id, b ? TRUE : FALSE);
        };

        checkButton(IDC_BOLD    , config.font.bold);
        checkButton(IDC_ITALIC  , config.font.italic);
        SetDlgItemText(hDlg, IDC_FORMAT      , config.text.format.c_str());
    }
}


void onApply(HWND hDlg) {
    config.font.name = cbGetLbText(
          hDlg
        , IDC_FONT
        , cbGetCurSel(hDlg, IDC_FONT)
    ).data();

    if(cbGetCount(hDlg, IDC_FONT_SIZE) > 0) {
        config.font.size = _tstoi(cbGetLbText(
              hDlg
            , IDC_FONT_SIZE
            , cbGetCurSel(hDlg, IDC_FONT_SIZE)
        ).data());
    } else {
        config.font.size = 9;
    }

    const auto getPos = [&](int id) {
        return sendDlgItemMessage<short>(hDlg, id, UDM_GETPOS);
    };

    config.font.bold        = IsDlgButtonChecked(hDlg, IDC_BOLD) != 0;
    config.font.italic      = IsDlgButtonChecked(hDlg, IDC_ITALIC) != 0;
    config.font.quality     = cbGetCurSel<int>(hDlg, IDC_FONT_QUALITY);
    config.text.height      = getPos(IDC_SPIN_HEIGHT);
    config.text.width       = getPos(IDC_SPIN_WIDTH);
    config.text.lineSpacing = getPos(IDC_SPIN_LINESPACING);
    config.text.yPos        = getPos(IDC_SPIN_YPOS);
    config.text.xPos        = getPos(IDC_SPIN_XPOS);

    {
        TCHAR s[1024];
        GetDlgItemText(hDlg, IDC_FORMAT, s, _countof(s));
        config.text.format  = s;
    }

    saveConfig(config);
}


void onChooseColor(HWND hDlg, WORD, int& color) {
    COLORREF colarray[16] {};
    for(auto& c : colarray) {
        const auto i = &c - &colarray[0];
        const auto k = (i / 8) * 128 + 127;
        c = RGB((i&1)*k, ((i>>1)&1)*k, ((i>>2)&1)*k);
    }
    colarray[0] = GetSysColor(COLOR_BTNTEXT);
    colarray[1] = GetSysColor(COLOR_3DFACE);

    CHOOSECOLOR cc {};
    cc.lStructSize  = sizeof(CHOOSECOLOR);
    cc.hwndOwner    = hDlg;
    cc.hInstance    = (HWND) GetModuleHandle(nullptr);
    cc.rgbResult    = color;
    cc.lpCustColors = colarray;
    cc.Flags        = CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;

    if(ChooseColor(&cc)) {
        color = cc.rgbResult;
        postMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
        sendMessage(GetParent(hDlg), PSM_CHANGED, hDlg, 0);
    }
}


void onCommand(HWND hDlg, WORD id, WORD code) {
    const bool changed = [&]() {
        const HWND item = GetDlgItem(hDlg, id);

        TCHAR className[256] {};
        GetClassName(item, className, _countof(className));

        if(_tcscmp(className, _T("ComboBox")) == 0) {
            return (code == CBN_SELCHANGE);
        } else if(_tcscmp(className, _T("Edit")) == 0) {
            return (code == EN_CHANGE);
        } else if(_tcscmp(className, _T("Button")) == 0) {
            return true;
        }

        return false;
    }();

    switch(id) {
    default:
        break;

    case IDC_FONT:
        {
            const auto curSel = cbGetCurSel(hDlg, IDC_FONT);
            const auto faceName = cbGetLbText(hDlg, IDC_FONT, curSel);
            int charSet = cbGetItemData<int>(hDlg, IDC_FONT, curSel);
            setComboFontSize(hDlg, IDC_FONT_SIZE, 0, faceName.data(), charSet);
        }
        break;

    case IDC_BTN_FOREGROUND:
        onChooseColor(hDlg, id, config.text.color.foreground);
        break;

    case IDC_BTN_BACKGROUND:
        onChooseColor(hDlg, id, config.text.color.background);
        break;
    }

    if(changed) {
        sendMessage(GetParent(hDlg), PSM_CHANGED, hDlg);
    }
}


void onDrawItem(HWND, WPARAM, LPARAM lParam) {
    const auto* dis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
    const auto id = dis->CtlID;
    switch(id) {
    default:
        break;

    case IDC_BTN_FOREGROUND:
    case IDC_BTN_BACKGROUND:
        {
            // How can I change the background color of a button WinAPI C++
            // http://stackoverflow.com/a/18839564/2132223
            const auto haveFocus = (dis->itemState & ODS_FOCUS) != 0;

            const COLORREF color = [&]() {
                switch(id) {
                default:
                case IDC_BTN_FOREGROUND:
                    return config.text.color.foreground;
                case IDC_BTN_BACKGROUND:
                    return config.text.color.background;
                }
            }();

            const COLORREF frameColor = [&]() {
                if(haveFocus) {
                    return GetSysColor(COLOR_HIGHLIGHT);
                } else {
                    return GetSysColor(COLOR_BTNSHADOW);
                }
            }();

            const auto hdc = dis->hDC;
            const auto oldPen = SelectObject(hdc, GetStockObject(DC_PEN));
            const auto oldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
            SetDCPenColor(hdc, frameColor);
            SetDCBrushColor(hdc, color);
            const int r = 16;
            RoundRect(
                  hdc
                , dis->rcItem.left
                , dis->rcItem.top
                , dis->rcItem.right
                , dis->rcItem.bottom
                , r
                , r
            );
            SelectObject(hdc, oldPen);
            SelectObject(hdc, oldBrush);
        }
        break;
    }
}


INT_PTR CALLBACK
colorDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
    default:
        break;

    case WM_INITDIALOG:
        onInit(hDlg);
        return TRUE;

    case WM_COMMAND:
        onCommand(hDlg, LOWORD(wParam), HIWORD(wParam));
        return TRUE;

    case WM_NOTIFY:
        if(((const NMHDR*)lParam)->code == PSN_APPLY) {
            onApply(hDlg);
        }
        return TRUE;

    case WM_DRAWITEM:
        onDrawItem(hDlg, wParam, lParam);
        return TRUE;

    case WM_DESTROY:
        DestroyWindow(hDlg);
        break;
    }
    return FALSE;
}

} // Anonymous namespace


ResourcelessDialog createColorDlg() {
    auto rd = ResourcelessDialog::create(
          widgets
        , colorDlgProc
        , _T("Clock Text")
        , _T("MS Sans Serif")
        , 8
    );
    rd.maskDlgTemplateStyle(DS_SHELLFONT | WS_CHILD);
    return rd;
}
