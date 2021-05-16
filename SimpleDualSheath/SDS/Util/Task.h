#pragma once

namespace SDS
{
    namespace Util
    {
        namespace Task
        {
            void QueueActorTask(
                TESObjectREFR* a_actor,
                std::function<void(Actor*)> a_func);
        }
    }
}