// Resourceless Dialog
//
// Dialog Template by Max McGuire
// http://www.flipcode.com/archives/Dialog_Template.shtml
//
// DLGTEMPLATE structure
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms645394.aspx
//
// DLGITEMTEMPLATE structure
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms644997.aspx
//
// Window Classes - This section lists the window class names provided by the common control library.
// http://msdn.microsoft.com/en-us/library/windows/desktop/bb775491.aspx
//
// The dialog manager, part 1: Warm-ups
// http://blogs.msdn.com/b/oldnewthing/archive/2005/03/29/403298.aspx

#ifndef RESOURCELESS_DIALOG_H
#define RESOURCELESS_DIALOG_H

#include <commctrl.h>
#include <functional>
#include <vector>

namespace ResourcelessDialogTypes {
    enum : int {
        TYPE_UNKNOWN = 0,
        TYPE_PUSHBUTTON,
        TYPE_EDITTEXT,
        TYPE_TEXT,
        TYPE_LISTBOX,
        TYPE_SCROLLBAR,
        TYPE_COMBOBOX,
        TYPE_CTRL_BUTTON,
        TYPE_CTRL_UPDOWN,
        TYPE_CTRL_PROGRESS,
        TYPE_GROUPBOX,
    };
}


class ResourcelessDialog {
public:
    ResourcelessDialog() {}

    ResourcelessDialog(
          const DLGTEMPLATE& dt
        , LPCTSTR caption = nullptr
        , LPCTSTR font = nullptr
        , int fontSize = 8
    ) {
        set(dt, caption, font, fontSize);
    }

    virtual ~ResourcelessDialog() {
        reset();
    }

    operator const DLGTEMPLATE*() const {
        return get();
    }

    size_t size() const {
        return buffer.size();
    }

    DLGTEMPLATE* get() const {
        return reinterpret_cast<DLGTEMPLATE*>(buffer.get());
    }

    bool good() const {
        return buffer.get() != nullptr;
    }

    void setDlgProc(DLGPROC dlgProc) {
        this->dlgProc = dlgProc;
    }

    DLGPROC getDlgProc() const {
        return dlgProc;
    }

    void set(
          LPCTSTR caption = nullptr
        , LPCTSTR font = nullptr
        , int fontSize = 8
    ) {
        DLGTEMPLATE dt {};
        dt.style    = WS_CAPTION;
        dt.cx       = 640;
        dt.cy       = 480;
        set(dt, caption, font, fontSize);
    }

    void set(
          const DLGTEMPLATE& dt
        , LPCTSTR caption = nullptr
        , LPCTSTR font = nullptr
        , int fontSize = 8
    ) {
        reset();

        buffer.append(dt);
        maskDlgTemplateStyle(WS_SYSMENU | DS_CENTER);

        buffer.append(static_cast<WORD>(0));
        buffer.append(static_cast<WORD>(0));

        if(caption) {
            maskDlgTemplateStyle(WS_CAPTION);
            buffer.appendString(caption);
        } else {
            buffer.appendString(_T(""));
        }

        if(font) {
            maskDlgTemplateStyle(DS_SETFONT);
            buffer.append(static_cast<WORD>(fontSize));
            buffer.appendString(font);
        }
    }

    void reset() {
        buffer.reset();
    }

    void add(int type, const DLGITEMTEMPLATE& t, LPCTSTR caption = _T("")) {
        using namespace ResourcelessDialogTypes;

        if(const auto* s = componentTypeToName(type)) {
            addComponent(s, t, caption);
        } else {
            switch(type) {
            default:
            case TYPE_UNKNOWN:                                              break;
            case TYPE_GROUPBOX:
            case TYPE_PUSHBUTTON:   addStdComponent0(0x0080, t, caption);   break;
            case TYPE_EDITTEXT:     addStdComponent0(0x0081, t, caption);   break;
            case TYPE_TEXT:         addStdComponent0(0x0082, t, caption);   break;
            case TYPE_LISTBOX:      addStdComponent0(0x0083, t, caption);   break;
            case TYPE_SCROLLBAR:    addStdComponent0(0x0084, t, caption);   break;
            case TYPE_COMBOBOX:     addStdComponent0(0x0085, t, caption);   break;
            }
        }
    }

    void addComponent(
          LPCTSTR componentName
        , const DLGITEMTEMPLATE& t
        , LPCTSTR caption = _T("")
    ) {
        if(!good()) {
            return;
        }

        buffer.align(sizeof(DWORD));
        buffer.append(t);
        buffer.appendString(componentName);
        buffer.appendString(caption);
        buffer.append(static_cast<WORD>(0));
        incDlgTemplateCdit();
    }

    template <
        class T0, class T1, class T2, class T3, class T4, class T5, class T6
    > static DLGITEMTEMPLATE makeItem(
        T0 style, T1 dwExtendedStyle, T2 x, T3 y, T4 w, T5 h, T6 id
    ) {
        return {
              static_cast<decltype(DLGITEMTEMPLATE::style)>(style)
            , static_cast<decltype(DLGITEMTEMPLATE::dwExtendedStyle)>(dwExtendedStyle)
            , static_cast<decltype(DLGITEMTEMPLATE::x)>(x)
            , static_cast<decltype(DLGITEMTEMPLATE::y)>(y)
            , static_cast<decltype(DLGITEMTEMPLATE::cx)>(w)
            , static_cast<decltype(DLGITEMTEMPLATE::cy)>(h)
            , static_cast<decltype(DLGITEMTEMPLATE::id)>(id)
        };
    }

    static const wchar_t* componentTypeToName(int type) {
        using namespace ResourcelessDialogTypes;

        const wchar_t* s = nullptr;
        switch(type) {
        default:                    s = nullptr;        break;
        case TYPE_CTRL_BUTTON:      s = WC_BUTTON;      break;
        case TYPE_CTRL_UPDOWN:      s = UPDOWN_CLASS;   break;
        case TYPE_CTRL_PROGRESS:    s = PROGRESS_CLASS; break;
        }
        return s;
    }

    static bool typeHasTabStop(int type) {
        using namespace ResourcelessDialogTypes;

        bool r = true;

        switch(type) {
        default:
            r = true;
            break;

        case TYPE_TEXT:
        case TYPE_GROUPBOX:
        case TYPE_CTRL_UPDOWN:
            r = false;
            break;
        }

        return r;
    }

    static DWORD typeDefaultStyle(int type) {
        using namespace ResourcelessDialogTypes;

        DWORD r = 0;

        switch(type) {
        default:
            break;

        case TYPE_EDITTEXT:
            r = WS_BORDER;
            break;

        case TYPE_GROUPBOX:
            r = BS_GROUPBOX;
            break;

        case TYPE_COMBOBOX:
            r = CBS_DROPDOWNLIST | WS_VSCROLL;
            break;

        case TYPE_CTRL_UPDOWN:
            r =   UDS_WRAP
                | UDS_ALIGNRIGHT
                | UDS_SETBUDDYINT
                | UDS_AUTOBUDDY
                | UDS_ARROWKEYS;
            break;
        }

        return r;
    }

    void maskDlgTemplateStyle(DWORD orValue, DWORD andValue = ~0) {
        if(auto* p = get()) {
            p->style &= andValue;
            p->style |= orValue;
        }
    }

    struct Data {
        int             type;
        const TCHAR*    text;
        WORD            id;
        WORD            x;
        WORD            y;
        WORD            w;
        WORD            h;
        int             style;
        int             exStyle;
    };

    template<class T>
    static ResourcelessDialog create(
          const T& datas
        , DLGPROC dlgProc
        , const TCHAR* caption = nullptr
        , const TCHAR* font = nullptr
        , int fontSize = 8
        , int marginX = 8
        , int marginY = 8
    ) {
        using namespace ResourcelessDialogTypes;

        ResourcelessDialog rd {};

        DLGTEMPLATE dt {};
        dt.style = DS_SETFONT | WS_CHILD | WS_DISABLED | WS_CAPTION;

        int ofsX {};
        int ofsY {};
        {
            int minX = SHRT_MAX;
            int minY = SHRT_MAX;
            int maxX = 0;
            int maxY = 0;
            for(const auto& d : datas) {
                minX = std::min<int>(minX, d.x);
                minY = std::min<int>(minY, d.y);
                maxX = std::max<int>(maxX, d.x + d.w);
                maxY = std::max<int>(maxY, d.y + d.h);
            }

            ofsX    = -minX + marginX;
            ofsY    = -minY + marginY;
            dt.cx   = static_cast<decltype(dt.cx)>(maxX - minX + marginX * 2);
            dt.cy   = static_cast<decltype(dt.cy)>(maxY - minY + marginY * 2);
        }

        rd.set(dt, caption, font, fontSize);
        rd.setDlgProc(dlgProc);

        for(const auto& d : datas) {
            auto item = rd.makeItem(d.style, d.exStyle, d.x, d.y, d.w, d.h, d.id);

            item.x += static_cast<decltype(item.x)>(ofsX);
            item.y += static_cast<decltype(item.y)>(ofsY);

            item.style |=
                  WS_VISIBLE
                | typeDefaultStyle(d.type)
                | (typeHasTabStop(d.type) ? WS_TABSTOP : 0);

            rd.add(d.type, item, d.text);
        }

        return rd;
    }

    static HWND propertySheet(
          const std::vector<ResourcelessDialog>& resourcelessDialogs
        , const PROPSHEETHEADER& propSheetHeader
    ) {
        std::vector<PROPSHEETPAGE> propSheetPages;
        for(const auto& rd : resourcelessDialogs) {
            PROPSHEETPAGE psp {};
            psp.dwSize      = sizeof(psp);
            psp.pfnDlgProc  = rd.getDlgProc();
            psp.pResource   = rd.get();
            psp.dwFlags     |= PSP_DLGINDIRECT;
            propSheetPages.push_back(psp);
        };

        PROPSHEETHEADER psh = propSheetHeader;
        psh.nPages  = static_cast<UINT>(propSheetPages.size());
        psh.ppsp    = propSheetPages.data();

        return reinterpret_cast<HWND>(PropertySheet(&psh));
    }

    static HWND propertySheet(
          const std::vector<ResourcelessDialog>& resourcelessDialogs
        , const TCHAR* caption
        , PFNPROPSHEETCALLBACK psc
    ) {
        PROPSHEETHEADER psh {};
        psh.dwSize      = sizeof(PROPSHEETHEADER);
        psh.dwFlags     =
              PSH_PROPSHEETPAGE
            | PSH_USECALLBACK
            | PSH_MODELESS
            | PSH_NOCONTEXTHELP
            | PSH_PROPTITLE
        ;
        psh.hInstance   = GetModuleHandle(nullptr);
        psh.hIcon       = LoadIcon(nullptr, IDI_APPLICATION);
        psh.pszCaption  = caption;
        psh.pfnCallback = psc;

        return propertySheet(resourcelessDialogs, psh);
    }

protected:
    void addStdComponent0(WORD type, const DLGITEMTEMPLATE& t, LPCTSTR caption) {
        if(!good()) {
            return;
        }

        buffer.align(sizeof(DWORD));
        buffer.append(t);
        buffer.append(static_cast<WORD>(0xffff));
        buffer.append(type);
        buffer.appendString(caption);
        buffer.append(static_cast<WORD>(0));
        incDlgTemplateCdit();
    }

    void incDlgTemplateCdit() {
        if(auto* p = get()) {
            p->cdit += 1;
        }
    }

    class Buffer {
    public:
        void reset() {
            memory.clear();
        }

        char* get() const {
            if(memory.empty()) {
                return nullptr;
            } else {
                return const_cast<char*>(memory.data());
            }
        }

        size_t size() const {
            return memory.size();
        }

        void append(const void* data, size_t dataBytes) {
            if(auto* p = allocBuffer(dataBytes)) {
                memcpy(p, data, dataBytes);
            }
        }

        template<class T>
        void append(const T& data) {
            append(&data, sizeof(data));
        }

        void appendString(const char* mbStr) {
            // note : "wcLen" includes tail '\0'
            size_t wcLen = 0;
            mbstowcs_s(&wcLen, nullptr, 0, mbStr, 0);
            const auto wcBytes = wcLen * sizeof(wchar_t);
            if(auto* p = reinterpret_cast<wchar_t*>(allocBuffer(wcBytes))) {
                mbstowcs_s(nullptr, p, wcLen, mbStr, wcLen);
            }
        }

        void appendString(const wchar_t* wcStr) {
            // note : "+1" for tail L'\0'.
            const auto wcLen = wcslen(wcStr) + 1;
            const auto wcBytes = wcLen * sizeof(wchar_t);
            append(wcStr, wcBytes);
        }

        void align(size_t bytes) {
            if(const auto paddingSize = size() % bytes) {
                allocBuffer(paddingSize);
            }
        }

    protected:
        char* allocBuffer(size_t bytes) {
            const auto s = size();
            memory.resize(s + bytes);
            return get() + s;
        }

        std::vector<char> memory;
    };

    Buffer buffer;
    DLGPROC dlgProc {};
};

#endif
