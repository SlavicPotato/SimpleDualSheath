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
            kNoFirstPerson = 1u << 4,

            kEnabled = (kPlayer | kNPC)
        };

        class Weapon
        {
        public:
            Weapon(
                const char* a_nodeName,
                const char* a_nodeNameLeft,
                Flags a_flags);

            Weapon(Flags a_flags);

            [[nodiscard]] const BSFixedString& GetNodeName(bool a_left) const;
            [[nodiscard]] NiNode* GetNode(NiNode* a_root, bool a_left) const;

            BSFixedString m_nodeName;
            BSFixedString m_nodeNameLeft;
            Flags m_flags;
        };

        class WeaponData
        {
        public:

            template <typename... Args>
            void Create(std::uint32_t a_type, Args&&... a_args)
            {
                if (a_type < std::size(m_entries)) {
                    m_entries[a_type] = std::make_unique<Weapon>(std::forward<Args>(a_args)...);
                }
            }

            void SetStrings(std::uint32_t a_type, const char* a_nodeName, const char* a_nodeNameLeft);

            [[nodiscard]] const Weapon* Get(Actor* a_actor, TESObjectWEAP* a_weapon, bool a_left) const;

        private:

            std::unique_ptr<Weapon> m_entries[10];

        };

        DEFINE_ENUM_CLASS_BITWISE(Flags);
    }
}