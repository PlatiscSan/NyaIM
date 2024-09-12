#ifndef WINDOWS_UTILITY_H
#define WINDOWS_UTILITY_H

#include <string>

namespace NyaIM::utility::windows {

	std::string GetLastErrorMessageFromWSA();

	void InitializeWSA();
	void CleanUpWSA();

}

#endif // !WINDOWS_UTILITY_H
