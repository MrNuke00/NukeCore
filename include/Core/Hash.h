// NukeCore\include\Core\Hash.h
#pragma once
#include <Core/Core.h>

namespace NK::Core
{
	static uint32 HashInteger(uint32 Val)
	{
		Val = (Val ^ (Val >> 16)) * 0x7feb352dU;
		Val = (Val ^ (Val >> 15)) * 0x846ca68bU;
		return Val ^ (Val >> 16);
	}

	static uint64 HashInteger(uint64 Val)
	{
		Val = (Val ^ (Val >> 30)) * 0xbf58476d1ce4e5b9ULL;
		Val = (Val ^ (Val >> 27)) * 0x94d049bb133111ebULL;
		return Val ^ (Val >> 31);
	}

	inline void HashCombine(uint64& Seed, uint64 HashVal)
	{
		Seed ^= HashVal + 0x9e3779b97f4a7c15ULL + (Seed << 6) + (Seed >> 2);
	}

	template<typename Key>
	struct Hash
	{
	private:
		
		static uint64 HashBuffer(const void* Data, uint64 Size)
		{
			if (!Data || Size == 0) return 0;
			const uint8* Bytes = static_cast<const uint8*>(Data);
			uint64 HashValue = 0xcbf29ce484222325ULL;
			for (uint64 i = 0; i < Size; ++i)
			{
				HashValue ^= Bytes[i];
				HashValue *= 0x100000001b3ULL;
			}
			return HashValue;
		}

	public:
		uint64 operator()(const Key& Value) const
		{
			return GetHash(Value);
		}

		static uint64 GetHash(const Key& Value)
		{
			if constexpr (std::is_integral_v<Key> || std::is_enum_v<Key>)
			{
				using UnsignedT = std::make_unsigned_t<Key>;
				if constexpr (sizeof(Key) <= 4)
				{
					return static_cast<uint64>(HashInteger(static_cast<uint32>(static_cast<UnsignedT>(Value))));
				}
				else
				{
					return HashInteger(static_cast<uint64>(static_cast<UnsignedT>(Value)));
				}
			}
			else if constexpr (std::is_pointer_v<Key>)
			{
				return HashInteger(reinterpret_cast<uintptr_t>(Value));
			}
			else if constexpr (std::is_floating_point_v<Key>)
			{
				if constexpr (sizeof(Key) == 4)
				{
					uint32 Temp;
					std::memcpy(&Temp, &Value, sizeof(uint32));
					return static_cast<uint64>(HashInteger(Temp));
				}
				else
				{
					uint64 Temp;
					std::memcpy(&Temp, &Value, sizeof(uint64));
					return HashInteger(Temp);
				}
			}
			else if constexpr (std::is_same_v<Key, std::string> || std::is_same_v<Key, std::string_view>)
			{
				return HashBuffer(Value.data(), static_cast<uint64>(Value.size() * sizeof(typename Key::value_type)));
			}
			else if constexpr (std::is_same_v<Key, const char*> || std::is_same_v<Key, char*>)
			{
				return Value ? HashBuffer(Value, static_cast<uint64>(std::strlen(Value))) : 0;
			}
			else
			{
				return Value.GetHash();
			}
		}

		uint64 operator()(const Key* Data, uint64 Size) const
		{
			return GetHash(Data, Size);
		}

		static uint64 GetHash(const Key* Data, uint64 Size)
		{
			if (!Data || Size == 0) return 0;
			return HashBuffer(Data, Size * sizeof(Key));
		}
		
	};
}