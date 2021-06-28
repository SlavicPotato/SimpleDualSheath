#pragma once

namespace SDS
{

    class InputHandler :
        public BSTEventSink <InputEvent>
    {

    private:
        virtual void OnKeyDown(UInt32 a_keyCode) = 0;
        virtual void OnKeyUp(UInt32 a_keyCode) = 0;

        virtual EventResult ReceiveEvent(InputEvent** evns, InputEventDispatcher* dispatcher) override;

    };

    class ComboKeyPressHandler :
        public InputHandler
    {
    public:

        void SetComboKey(UInt32 a_key);
        void SetKey(UInt32 a_key);
        void SetKeys(UInt32 a_comboKey, UInt32 a_key);

    protected:

        virtual void OnKeyPressed() = 0;
    private:

        void OnKeyDown(UInt32 a_keyCode) override;
        void OnKeyUp(UInt32 a_keyCode) override;

        bool m_comboKeyDown{ false };

        UInt32 m_comboKey{ 0 };
        UInt32 m_key{ 0 };


    };

}