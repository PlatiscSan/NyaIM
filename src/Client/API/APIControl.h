#ifndef API_CONTROL_H
#define API_CONTROL_H

#if defined(_WIN32)

	#ifdef DLLEXPORT
		#define NYAIM_API __declspec(dllexport)
	#else	
		#define NYAIM_API __declspec(dllimport)
	#endif // DLLEXPORT

	#define NYAIM_CALL __cdecl

#elif defined(__GNUC__)

	#ifdef SOEXPORT
		#define NYAIM_API __attribute__((visibility("default")))
	#else
		#define NYAIM_API
	#endif // SOEXPORT

	#define NYAIM_CALL  __attribute__((__cdecl__))

#endif // defined(_WIN32)

#endif // !API_CONTROL_H
