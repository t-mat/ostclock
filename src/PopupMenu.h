#ifndef POPUP_MENU_H
#define POPUP_MENU_H

struct PopupMenu {
	PopupMenu() {
		create();
	}

	~PopupMenu() {
		destroy();
	}

	void create() {
		if(!hMenu) {
			hMenu = CreatePopupMenu();
		}
	}

	void destroy() {
		if(hMenu) {
			DestroyMenu(hMenu);
			hMenu = nullptr;
		}
	}

	template<typename T_NEWITEMID = UINT_PTR, typename T_NEWITEMPTR = LPCTSTR>
	BOOL append(UINT uFlags, T_NEWITEMID uIDNewItem = 0, T_NEWITEMPTR lpNewItem = nullptr) {
		if(!hMenu) {
			return FALSE;
		}
		return AppendMenu(hMenu, uFlags, (UINT_PTR) uIDNewItem, (LPCTSTR) lpNewItem);
	}

	BOOL track(HWND hWnd, int x, int y, UINT uFlags, const RECT* rc = nullptr) {
		if(!hMenu) {
			return FALSE;
		}
		return TrackPopupMenu(hMenu, uFlags, x, y, 0, hWnd, rc);
	}

	HMENU hMenu { nullptr };
};

#endif
