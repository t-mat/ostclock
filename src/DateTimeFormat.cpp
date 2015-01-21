#include "stdafx.h"
#include "DateTimeFormat.h"

namespace {

// Process "\n".
void pass1(const SYSTEMTIME&, FormatOutput& dst, const TCHAR* src) {
    const auto* s = src;
    auto* d = dst.data();
    for(; *s; ++s) {
        if(s[0] == _T('\\') && s[1] == _T('n')) {
            *d++ = _T('\n');
            ++s;
        } else {
            *d++ = *s;
        }
    }
    *d = _T('\0');
}


// strftime()
// http://msdn.microsoft.com/en-us/library/fe06s4ak.aspx
void pass2(const SYSTEMTIME& pt, FormatOutput& dst, const FormatOutput& src) {
    // Windows FILETIME examples
    // http://www.programmingforums.org/post45492.html

    // 116444736000000000 nsecs
    static const int64_t EPOCH_DIFF = 0x019DB1DED53E8000LL;

    // 100 nsecs
    static const int RATE_DIFF = 10000000;

    SYSTEMTIME st {};
    TzSpecificLocalTimeToSystemTime(nullptr, &pt, &st);

    FILETIME ft {};
    SystemTimeToFileTime(&st, &ft);

    const auto ift = * reinterpret_cast<const int64_t*>(&ft);
    const auto unixTime = static_cast<time_t>((ift - EPOCH_DIFF) / RATE_DIFF);
    struct tm tm {};
    localtime_s(&tm, &unixTime);

    const auto currentLocale = _get_current_locale();
    _tcsftime_l(dst.data(), dst.size(), src.data(), &tm, currentLocale);
}

} // Anonymous namespace


void initDateTimeFormat() {
    _tsetlocale(LC_TIME, _T(""));
}


FormatOutput makeDateTimeString(const SYSTEMTIME& pt, const TCHAR* fmt) {
    std::array<FormatOutput,2> buf;
    pass1(pt, buf[0], fmt);
    pass2(pt, buf[1], buf[0]);
    return buf[1];
}
