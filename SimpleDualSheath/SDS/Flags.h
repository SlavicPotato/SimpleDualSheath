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
            kImmediate = 1u << 5,
            kUpdateNodeOnAttach = 1u << 6,

            kEnabled = (kPlayer | kNPC)
        };

        DEFINE_ENUM_CLASS_BITWISE(Flags);
    }
}