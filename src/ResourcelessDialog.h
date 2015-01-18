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
	using WalkerFunc = std::function<void(int type, DLGITEMTEMPLATE& t, const wchar_t* caption, const wchar_t* componentName)>;

	ResourcelessDialog() {}

	ResourcelessDialog(const DLGTEMPLATE& dt, LPCTSTR caption = nullptr, LPCTSTR font = nullptr, int fontSize = 8) {
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

	void set(LPCTSTR caption = nullptr, LPCTSTR font = nullptr, int fontSize = 8) {
		DLGTEMPLATE dt {};
		dt.style	= WS_CAPTION;
		dt.cx		= 640;
		dt.cy		= 480;
		set(dt, caption, font, fontSize);
	}

	void set(const DLGTEMPLATE& dt, LPCTSTR caption = nullptr, LPCTSTR font = nullptr, int fontSize = 8) {
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
			case TYPE_UNKNOWN:												break;
			case TYPE_PUSHBUTTON:	addStdComponent0(0x0080, t, caption);	break;
			case TYPE_EDITTEXT:		addStdComponent0(0x0081, t, caption);	break;
			case TYPE_TEXT:			addStdComponent0(0x0082, t, caption);	break;
			case TYPE_LISTBOX:		addStdComponent0(0x0083, t, caption);	break;
			case TYPE_SCROLLBAR:	addStdComponent0(0x0084, t, caption);	break;
			case TYPE_COMBOBOX:		addStdComponent0(0x0085, t, caption);	break;
			}
		}
	}

	void addComponent(LPCTSTR componentName, const DLGITEMTEMPLATE& t, LPCTSTR caption = _T("")) {
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

	static DLGITEMTEMPLATE makeItem(DWORD style, DWORD exStyle, int x, int y, int w, int h, int id) {
		return {
			  style
			, exStyle
			, static_cast<decltype(DLGITEMTEMPLATE::x)>(x)
			, static_cast<decltype(DLGITEMTEMPLATE::y)>(y)
			, static_cast<decltype(DLGITEMTEMPLATE::cx)>(w)
			, static_cast<decltype(DLGITEMTEMPLATE::cy)>(h)
			, static_cast<decltype(DLGITEMTEMPLATE::id)>(id)
		};
	}

	static int componentNameToType(const wchar_t* componentName) {
		using namespace ResourcelessDialogTypes;

		int type = TYPE_UNKNOWN;
		const auto& cmp = [componentName](const wchar_t* s) {
			return 0 == wcscmp(componentName, s);
		};
		if(cmp(WC_BUTTON)) {
			type = TYPE_CTRL_BUTTON;
		} else if(cmp(UPDOWN_CLASS)) {
			type = TYPE_CTRL_UPDOWN;
		} else if(cmp(PROGRESS_CLASS)) {
			type = TYPE_CTRL_PROGRESS;
		}
		return type;
	}

	static const wchar_t* componentTypeToName(int type) {
		using namespace ResourcelessDialogTypes;

		const wchar_t* s = nullptr;
		switch(type) {
		default:					s = nullptr;		break;
		case TYPE_CTRL_BUTTON:		s = WC_BUTTON;		break;
		case TYPE_CTRL_UPDOWN:		s = UPDOWN_CLASS;	break;
		case TYPE_CTRL_PROGRESS:	s = PROGRESS_CLASS;	break;
		}
		return s;
	}

	void maskDlgTemplateStyle(DWORD orValue, DWORD andValue = ~0) {
		if(auto* p = get()) {
			p->style &= andValue;
			p->style |= orValue;
		}
	}

	void foreachComponent(const WalkerFunc& func) {
		foreachComponent(get(), func);
	}

	struct Data {
		int				type;
	//	const TCHAR*	controlName;
		const TCHAR*	text;
		WORD			id;
		WORD			x;
		WORD			y;
		WORD			w;
		WORD			h;
		int				style;
		int				exStyle;
	};

	template<class T>
//	static ResourcelessDialog create(const std::vector<Data>& datas, const TCHAR* caption, const TCHAR* font, int fontSize, DLGPROC dlgProc) {
	static ResourcelessDialog create(const T& datas, const TCHAR* caption, const TCHAR* font, int fontSize, DLGPROC dlgProc) {
		using namespace ResourcelessDialogTypes;

		ResourcelessDialog rd {};

		DLGTEMPLATE dt {};
		dt.style = DS_SETFONT | WS_CHILD | WS_DISABLED | WS_CAPTION;

		WORD ofsX {};
		WORD ofsY {};
		{
			WORD minX = 32767;
			WORD minY = 32767;
			WORD maxX = 0;
			WORD maxY = 0;
			for(const auto& d : datas) {
				minX = std::min<WORD>(minX, d.x);
				minY = std::min<WORD>(minY, d.y);
				maxX = std::max<WORD>(maxX, d.x + d.w);
				maxY = std::max<WORD>(maxY, d.y + d.h);
			}

			const WORD marginLeft	= 8;
			const WORD marginRight	= 8;
			const WORD marginTop	= 8;
			const WORD marginBottom	= 8;

			ofsX	= -minX + marginLeft;
			ofsY	= -minY + marginTop;
			dt.cx	= maxX - minX + marginLeft + marginRight;
			dt.cy	= maxY - minY + marginTop  + marginBottom;
		}

//		const TCHAR*	caption		= _T("Clock Text");
//		const TCHAR*	font		= _T("MS Sans Serif");
//		const int		fontSize	= 8;
		rd.set(dt, caption, font, fontSize);
		rd.setDlgProc(dlgProc);

		for(auto d : datas) {
			auto st = d.style | WS_VISIBLE;
			auto es = d.exStyle;

			d.x += ofsX;
			d.y += ofsY;

			const bool tabStop = [&]() {
				switch(d.type) {
				default:
					break;
				case TYPE_TEXT:
				case TYPE_GROUPBOX:
				case TYPE_CTRL_UPDOWN:
//				case TYPE_CONTROL:
					return false;
				}
				return true;
			}();
			if(tabStop) {
				st |= WS_TABSTOP;
			}

			switch(d.type) {
			default:
				break;
			case TYPE_TEXT:
				rd.add(TYPE_TEXT, rd.makeItem(st, es, d.x, d.y, d.w, d.h, d.id), d.text);
				break;
			case TYPE_PUSHBUTTON:
				rd.add(TYPE_PUSHBUTTON, rd.makeItem(st, es, d.x, d.y, d.w, d.h, d.id), d.text);
				break;
			case TYPE_COMBOBOX:
				st |= CBS_DROPDOWNLIST | WS_VSCROLL;
				rd.add(TYPE_COMBOBOX, rd.makeItem(st, es, d.x, d.y, d.w, d.h, d.id), d.text);
				break;
			case TYPE_EDITTEXT:
				st |= WS_BORDER;
				rd.add(TYPE_EDITTEXT, rd.makeItem(st, es, d.x, d.y, d.w, d.h, d.id), d.text);
				break;
	//		case TYPE_CONTROL:
	//			rd.addComponent(d.controlName, rd.makeItem(st, es, d.x, d.y, d.w, d.h, d.id), d.text);
	//			break;
			case TYPE_GROUPBOX:
				st |= BS_GROUPBOX;
				rd.add(TYPE_PUSHBUTTON, rd.makeItem(st, es, d.x, d.y, d.w, d.h, d.id), d.text);
				break;
			case TYPE_CTRL_BUTTON:
				rd.add(TYPE_CTRL_BUTTON, rd.makeItem(st, es, d.x, d.y, d.w, d.h, d.id), d.text);
				break;
			case TYPE_CTRL_UPDOWN:
				st |= UDS_WRAP | UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_AUTOBUDDY | UDS_ARROWKEYS;
				rd.add(TYPE_CTRL_UPDOWN, rd.makeItem(st, es, d.x, d.y, d.w, d.h, d.id), d.text);
				break;
			case TYPE_CTRL_PROGRESS:
				rd.add(TYPE_CTRL_PROGRESS, rd.makeItem(st, es, d.x, d.y, d.w, d.h, d.id), d.text);
				break;
			}
		}

		return rd;
	}

	static void foreachComponent(DLGTEMPLATE* dlgTemplate, const WalkerFunc& func) {
		using namespace ResourcelessDialogTypes;

		if(! dlgTemplate) {
			return;
		}

		auto* top = reinterpret_cast<char*>(dlgTemplate);
		const auto nComponent = dlgTemplate->cdit;
		size_t ofs = sizeof(*dlgTemplate);
		for(int iComponent = 0; iComponent < nComponent; ++iComponent) {
			if(const auto a = ofs % sizeof(DWORD)) {
				ofs += a;
			}

			int type = TYPE_UNKNOWN;
			auto* dlgItemTemplate = reinterpret_cast<DLGITEMTEMPLATE*>(top + ofs);
			const wchar_t* caption = nullptr;
			const wchar_t* componentName = nullptr;

			{
				ofs += sizeof(DLGITEMTEMPLATE);
				auto* p0 = reinterpret_cast<WORD*>(top + ofs);
				if(*p0 == 0xffff) {
					// Standard component
					switch(p0[1]) {
					default:	break;
					case 0x0080: type = TYPE_PUSHBUTTON;	break;
					case 0x0081: type = TYPE_EDITTEXT;		break;
					case 0x0082: type = TYPE_TEXT;			break;
					case 0x0083: type = TYPE_LISTBOX;		break;
					case 0x0084: type = TYPE_SCROLLBAR;		break;
					case 0x0085: type = TYPE_COMBOBOX;		break;
					}
					ofs += sizeof(WORD) * 2;
				} else {
					componentName = reinterpret_cast<wchar_t*>(top + ofs);
					const auto componentNameBytes = (wcslen(componentName) + 1) * sizeof(wchar_t);
					ofs += componentNameBytes;
					type = componentNameToType(componentName);
				}

				caption = reinterpret_cast<wchar_t*>(top + ofs);
				const auto captionBytes = (wcslen(caption) + 1) * sizeof(wchar_t);
				ofs += captionBytes;
				ofs += sizeof(WORD);	// tail 0
			}
			func(type, *dlgItemTemplate, caption, componentName);
		}
	}


	static HWND propertySheet(
		  const std::vector<ResourcelessDialog>& resourcelessDialogs
		, const PROPSHEETHEADER& propSheetHeader
	) {
		std::vector<PROPSHEETPAGE> propSheetPages;
		for(const auto& rd : resourcelessDialogs) {
			PROPSHEETPAGE psp {};
			psp.dwSize		= sizeof(psp);
			psp.pfnDlgProc	= rd.getDlgProc();
			psp.pResource	= rd.get();
			psp.dwFlags		|= PSP_DLGINDIRECT;
			propSheetPages.push_back(psp);
		};

		PROPSHEETHEADER psh = propSheetHeader;
		psh.nPages	= static_cast<UINT>(propSheetPages.size());
		psh.ppsp	= propSheetPages.data();

		return reinterpret_cast<HWND>(PropertySheet(&psh));
	}


	static HWND propertySheet(
		  const std::vector<ResourcelessDialog>& resourcelessDialogs
		, const TCHAR* caption
		, PFNPROPSHEETCALLBACK psc
	) {
		PROPSHEETHEADER psh {};
		psh.dwSize		= sizeof(PROPSHEETHEADER);
		psh.dwFlags		=
			  PSH_PROPSHEETPAGE
			| PSH_USECALLBACK
			| PSH_MODELESS
			| PSH_NOCONTEXTHELP
			| PSH_PROPTITLE
		;
		psh.hInstance	= GetModuleHandle(nullptr);
		psh.hIcon		= LoadIcon(nullptr, IDI_APPLICATION);
		psh.pszCaption	= caption;
		psh.pfnCallback	= psc;

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
	DLGPROC	dlgProc {};
};

#endif
