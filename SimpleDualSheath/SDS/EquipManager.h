#pragma once

namespace SDS
{
    struct EquipCandidateCollector
    {
    public:
        EquipCandidateCollector(TESObjectWEAP* a_ignore);

        bool Accept(TESContainer::ConfigEntry* entry);
        bool Accept(InventoryEntryData* a_entryData);

        std::unordered_set<TESObjectWEAP*> m_results;
        

    private:
        TESObjectWEAP* m_ignore;

        void Process(TESForm *a_item);
    };


    class EquipExtensions :
        public BSTEventSink <TESContainerChangedEvent>
    {

    public:

    protected:

        static bool CheckDualWield(Actor* a_actor);
        static bool ActorQualifiesForEquip(Actor* a_actor);

        void QueueEvaluateEquip(TESObjectREFR* a_actor) const;

        void EvaluateEquip(Actor* a_actor) const;

        virtual EventResult	ReceiveEvent(TESContainerChangedEvent* a_evn, EventDispatcher<TESContainerChangedEvent>* a_dispatcher) override;
    };
}