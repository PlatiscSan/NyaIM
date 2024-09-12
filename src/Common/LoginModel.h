#ifndef LOGIN_MODEL_H
#define LOGIN_MODEL_H

#include <cstdint>

namespace NyaIM::model {

	constexpr auto MaxNameLength = 32u;
	constexpr auto MaxPasswdLength = 32u;

	struct LoginModel {

		char name[MaxNameLength];
		char passwd[MaxPasswdLength];
		std::uint64_t uid;

	};

}

#endif // !LOGIN_MODEL_H
