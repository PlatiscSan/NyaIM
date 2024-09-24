
#if _MSC_VER > 1928
#include <__msvc_all_public_headers.hpp>
#endif

#if __GNUC__
#include <bits/stdc++.h>
#endif

#if _WIN32

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif // !WIN32_LEAN_AND_MEAN

	#include <Windows.h>
	#include <WinSock2.h>
	#include <WS2tcpip.h>
	#include <iphlpapi.h>

#elif __linux__


#endif // _WIN32

#include "Utility/common_developement.h"

#include <sql.h>
#include <sqlext.h>

#include <nmmintrin.h>