#pragma once

namespace SDS
{
    struct EquipCandidateCollector
    {
        EquipCandidateCollector(TESObjectWEAP* a_ignore);

        bool Accept(InventoryEntryData* a_entryData);

        std::vector<TESObjectWEAP*> m_results;
        TESObjectWEAP* m_ignore;
    };


    class EquipExtensions :
        public BSTEventSink <TESContainerChangedEvent>
    {

    public:

    protected:

        static bool CheckDualWield(Actor* a_actor);
        static bool ActorQualifiesForEquip(Actor* a_actor);

        void QueueEvaluateEquip(TESObjectREFR* a_actor);

        void EvaluateEquip(Actor* a_actor);

        virtual EventResult	ReceiveEvent(TESContainerChangedEvent* a_evn, EventDispatcher<TESContainerChangedEvent>* a_dispatcher) override;
    };
}