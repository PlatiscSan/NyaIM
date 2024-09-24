#ifndef AES128_H
#define AES128_H

#include <array>
#include <span>
#include <vector>

namespace common_dev {

	class AES128 final {

	public:

		AES128(std::span<std::uint8_t const, 16> const& key);

		struct EncryptRet {

			std::uint8_t* encrpyted_data = nullptr;
			std::size_t size_in_bytes;

			EncryptRet(std::size_t size, std::size_t alignment) {

			#if _MSC_VER
				encrpyted_data = static_cast<std::uint8_t*>(_aligned_malloc(size, alignment));
			#endif // _MSC_VER
				if (!encrpyted_data) {
					throw std::bad_alloc();
				}
				size_in_bytes = size;

			}

			~EncryptRet() {
			#if _MSC_VER
				_aligned_free(encrpyted_data);
			#endif // _MSC_VER
				encrpyted_data = nullptr;
				size_in_bytes = 0;
			}

		};

		EncryptRet Encrypt(void const* src, std::size_t src_size);
		void Decrypt(EncryptRet& encrypted_data);


	private:
			
		std::uint8_t alignas(16) m_round_key[11][4][4];

		void KeyExpansion(std::uint8_t* key);

		void Encrypt(std::uint8_t* data);
		void AddRoundKey(std::uint8_t data[][4], std::uint8_t key[][4]);
		void EncryptSubBytes(std::uint8_t data[][4]);
		void EncryptShiftRows(std::uint8_t data[][4]);
		void EncryptMixColumns(std::uint8_t data[][4]);

		void Decrypt(std::uint8_t* data);
		void DecryptSubBytes(std::uint8_t data[][4]);
		void DecryptShiftRows(std::uint8_t data[][4]);
		void DecryptMixColumns(std::uint8_t data[][4]);

	};

}

#endif // !AES128_H
