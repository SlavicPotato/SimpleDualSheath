#pragma once

namespace SDS
{
	namespace Data
	{
		enum class Flags : std::uint32_t
		{
			kNone = 0,

			kNPC = 1u << 0,
			kPlayer = 1u << 1,
			kRight = 1u << 2,
			kSwap = 1u << 3,
			kFirstPerson = 1u << 4,
			kMountOnly = 1u << 7,

			kEnabled = (kPlayer | kNPC)
		};

		DEFINE_ENUM_CLASS_BITWISE(Flags);
	}
}