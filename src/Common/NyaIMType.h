#ifndef NYAIM_TYPE_H
#define NYAIM_TYPE_H

namespace NyaIM::core {

#ifdef _WIN32

#ifdef _UNICODE
#define STRING(x) L##x
	using char_t = wchar_t;
#else
#define STRING(x) x
	using char_t = char;
#endif // _UNICODE



#elif defined(__linux__)

#define STRING(x) x
	using char_t = char;

#endif // _WIN32
}

#endif // !NYAIM_TYPE_H
