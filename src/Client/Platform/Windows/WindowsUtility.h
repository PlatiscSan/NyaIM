#ifndef WINDOWS_UTILITY_H
#define WINDOWS_UTILITY_H

#include <string>

namespace NyaIM::utility::windows {

	void InitializeWSA();
	void CleanUpWSA();
	std::string GetLastErrorMessageFromWinAPI();

}

#endif // !WINDOWS_UTILITY_H
