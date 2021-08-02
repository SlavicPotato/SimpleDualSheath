#pragma once

namespace SDS
{

    class InputHandler :
        public BSTEventSink <InputEvent>
    {

    private:
        virtual void OnKeyDown(std::uint32_t a_keyCode) = 0;
        virtual void OnKeyUp(std::uint32_t a_keyCode) = 0;

        virtual EventResult ReceiveEvent(InputEvent** evns, InputEventDispatcher* dispatcher) override;

    };

    class ComboKeyPressHandler :
        public InputHandler
    {
    public:

        void SetComboKey(std::uint32_t a_key);
        void SetKey(std::uint32_t a_key);
        void SetKeys(std::uint32_t a_comboKey, std::uint32_t a_key);

    protected:

        virtual void OnKeyPressed() = 0;
    private:

        void OnKeyDown(std::uint32_t a_keyCode) override;
        void OnKeyUp(std::uint32_t a_keyCode) override;

        bool m_comboKeyDown{ false };

        std::uint32_t m_comboKey{ 0 };
        std::uint32_t m_key{ 0 };


    };

}