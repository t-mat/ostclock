#ifndef FONT_H
#define FONT_H

struct Font {
	Font() {}


	~Font() {
		close();
	}


	void open(
		  WORD langId
		, int fontQuality
		, const TCHAR* fontName
		, int fontSize
		, LONG weight
		, LONG italic
		, int angle
	) {
		close();
		hFont = createFont(
			  langId
			, fontQuality
			, fontName
			, fontSize
			, weight
			, italic
			, angle
		);
	}


	void close() {
		if(hFont) {
			DeleteObject(hFont);
			hFont = nullptr;
		}
	}


	void select(HDC hdc) const {
		if(hdc && hFont) {
			SelectObject(hdc, hFont);
		}
	}


	using EnumFontFamiliesExFunc = std::function<BOOL(
		  const ENUMLOGFONTEX*
		, const NEWTEXTMETRICEX*
		, DWORD
	)>;


	static int enumFontFamiliesEx(
		  HDC hdc
		, LOGFONT& lf
		, const EnumFontFamiliesExFunc& func
	) {
		struct Param {
			Param() = delete;
			const Param& operator=(const Param&) = delete;
			const EnumFontFamiliesExFunc& func;
		};

		const auto& cb = [](
			  const LOGFONT* lf
			, const TEXTMETRIC* tm
			, DWORD type
			, LPARAM lParam
		) -> BOOL {
			const auto* param = reinterpret_cast<Param*>(lParam);
			return param->func(
				  reinterpret_cast<const ENUMLOGFONTEX*>(lf)
				, reinterpret_cast<const NEWTEXTMETRICEX*>(tm)
				, type
			);
		};

		Param param { func };

		return EnumFontFamiliesEx(
			  hdc
			, &lf
			, cb
			, reinterpret_cast<LPARAM>(&param)
			, 0
		);
	}


protected:
	static HFONT createFont(
		  WORD langId
		, int fontQuality
		, const TCHAR* fontName
		, int fontSize
		, LONG weight
		, LONG italic
		, int angle
	) {
		const HDC hdc = GetDC(nullptr);

		const POINT pt = [hdc, fontSize]() {
			POINT pt {};
			pt.x = 0;
			pt.y = MulDiv(fontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			DPtoLP(hdc, &pt, 1);
			return pt;
		} ();

		const int cp = [langId]() {
			int cp = CP_ACP;
			TCHAR s[80] {};
			if(GetLocaleInfo(langId, LOCALE_IDEFAULTANSICODEPAGE, s, _countof(s))) {
				cp = _tstoi(s);
			}
			if(!IsValidCodePage(cp)) {
				cp = CP_ACP;
			}
			return cp;
		}();

		struct CodePageCharSet {
			int cp;
			BYTE charset;
		};

		static const CodePageCharSet codePageCharSets[] = {
			{ 932,  SHIFTJIS_CHARSET },
			{ 936,  GB2312_CHARSET },
			{ 949,  HANGEUL_CHARSET },
			{ 950,  CHINESEBIG5_CHARSET },
			{ 1250, EASTEUROPE_CHARSET },
			{ 1251, RUSSIAN_CHARSET },
			{ 1252, ANSI_CHARSET },
			{ 1253, GREEK_CHARSET },
			{ 1254, TURKISH_CHARSET },
			{ 1257, BALTIC_CHARSET },
		};

		const BYTE charSet = [hdc, cp]() -> BYTE {
			auto c = static_cast<BYTE>(GetTextCharset(hdc));
			for(const auto& cpcs : codePageCharSets) {
				if(cp == cpcs.cp) {
					c = cpcs.charset;
					break;
				}
			}
			return c;
		} ();

		// find a font named "fontName"
		const LOGFONT logFont = [&]() {
			LOGFONT lf {};
			lf.lfCharSet = charSet;
			for(;;) {
				const auto r = enumFontFamiliesEx(
					  hdc, lf
					, [&](
						  const ENUMLOGFONTEX* pelf
						, const NEWTEXTMETRICEX*
						, DWORD
					) -> BOOL {
						if(_tcscmp(fontName, pelf->elfLogFont.lfFaceName) == 0) {
							return FALSE;
						} else {
							return TRUE;
						}
					}
				);
				if(!r) {
					break;
				}

				auto& cs = lf.lfCharSet;
				if(cs == charSet) {
					cs = OEM_CHARSET;
				} else if(cs == OEM_CHARSET) {
					cs = ANSI_CHARSET;
				} else {
					break;
				}
			}

			lf.lfHeight			= -pt.y;
			lf.lfWeight			= weight;
			lf.lfItalic			= (BYTE)italic;
			lf.lfEscapement		= (angle > 0) ? angle : 0;
			lf.lfOutPrecision	= OUT_DEFAULT_PRECIS;
			lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
			lf.lfQuality		= static_cast<BYTE>(fontQuality);
			lf.lfPitchAndFamily	= DEFAULT_PITCH | FF_DONTCARE;
			_tcscpy_s(lf.lfFaceName, fontName);
			return lf;
		}();

		ReleaseDC(nullptr, hdc);
		return CreateFontIndirect(&logFont);
	}

	HFONT hFont { nullptr };
};

#endif
