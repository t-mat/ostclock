#ifndef DATE_TIME_FORMAT_H
#define DATE_TIME_FORMAT_H

using FormatOutput = std::array<TCHAR, 256>;

void initDateTimeFormat();
FormatOutput makeDateTimeString(const SYSTEMTIME& pt, const TCHAR* fmt, WORD iLang);

#endif
