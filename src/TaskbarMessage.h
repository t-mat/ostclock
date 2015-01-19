#ifndef TASKBAR_MESSAGE_H
#define TASKBAR_MESSAGE_H

// Manipulating The Windows Taskbar
// http://www.codeproject.com/Articles/14380/Manipulating-The-Windows-Taskbar

enum : unsigned short {
    TASKBAR_MSG_START_MENU              = 305,      // Displays the Start menu
    TASKBAR_MSG_RUN                     = 401,      // Displays Run Dialog
    TASKBAR_MSG_LOGOFF                  = 402,      // Displays Logoff Dialog
    TASKBAR_MSG_WND_CASCADE_ALL         = 403,      // Command to cascade all toplevel windows
    TASKBAR_MSG_WND_TILE_HORZ           = 404,      // Command to Tile Horizontally all top level windows
    TASKBAR_MSG_WND_TILE_VERT           = 405,      // Command to Tile Vertically all top level windows
    TASKBAR_MSG_SHOW_DESKTOP            = 407,      // Shows the desktop. Do look at message number 419.
    TASKBAR_MSG_DATE_AND_TIME           = 408,      // Shows the Date and Time Dialog
    TASKBAR_MSG_TASKBAR_PROPERTIES      = 413,      // Shows taskbar properties
    TASKBAR_MSG_WND_MINIMIZE_ALL        = 415,      // Minimize all windows
    TASKBAR_MSG_WND_MAXIMIZE_ALL        = 416,      // Maximize all windows. To see the effect of this command first do Minimize and then Maximize all.
    TASKBAR_MSG_SHOW_DESKTOP2           = 419,      // Well I am a bit confused about this message. This also shows the desktop. Maybe somebody can notice the difference.
    TASKBAR_MSG_TASK_MANAGER            = 420,      // Shows task manager
    TASKBAR_MSG_TASKBAR_NOTIFICATION    = 421,      // Opens Customize Taskbar Notification Area
    TASKBAR_MSG_TASKBAR_LOCK            = 424,      // Locks the taskbar
    TASKBAR_MSG_HELP_AND_SUPPORT_CENTER = 503,      // Opens Help and Support Center Dialog
    TASKBAR_MSG_CONTROL_PANEL           = 505,      // Opens Control panel
    TASKBAR_MSG_SHUTDOWN                = 506,      // Shows the Shutdown computer dialog
    TASKBAR_MSG_DISPLAYS_AND_PRINTERS   = 510,      // Displays the Printers and Faxes dialog
    TASKBAR_MSG_LOCK_DESKTOP            = 517,      //
    TASKBAR_MSG_SWITCH_USER             = 5000,     //
    TASKBAR_MSG_FIND_FILES              = 41093,    // Displays Find Files Dialog
    TASKBAR_MSG_FIND_COMPUTERS          = 41094,    // Displays Find Computers Dialog
};

enum : unsigned short {
    TRAYCLOCK_MSG_CALC_RECT             = WM_USER + 100,    // WndProc must return clock's rectangle by MAKELONG(w,h)
    TRAYCLOCK_MSG_CALENDAR_AND_CLOCK    = WM_USER + 102,    // Show calendar & clock
    TRAYCLOCK_MSG_TOOLTIP               = WM_USER + 103,    // Show tooltip
};

#endif
