#include "pch.h"

#include "Task.h"
#include "Common.h"

namespace SDS
{
    namespace Util
    {
        namespace Task
        {
            void QueueActorTask(
                TESObjectREFR* a_actor,
                std::function<void(Actor*)> a_func)
            {
                if (!Common::IsREFRValid(a_actor)) {
                    return;
                }

                if (a_actor->formType != Actor::kTypeID) {
                    return;
                }

                auto handle = a_actor->GetHandle();
                if (!handle.IsValid()) {
                    return;
                }

                auto task = new TaskFunctor(
                    [handle, func = std::move(a_func)]
                {
                    NiPointer<TESObjectREFR> ref;
                    if (!handle.LookupREFR(ref)) {
                        return;
                    }

                    if (!Common::IsREFRValid(ref)) {
                        return;
                    }

                    if (ref->formType != Actor::kTypeID) {
                        return;
                    }

                    func(static_cast<Actor*>(ref.get()));
                }
                );

                ISKSE::GetSingleton().GetInterface<SKSETaskInterface>()->AddTask(task);
            }

        }

    }
}