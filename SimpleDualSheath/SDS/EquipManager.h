#pragma once

namespace SDS
{
	struct EquipCandidateCollector
	{
	public:
		EquipCandidateCollector(TESObjectWEAP* a_ignore);

		bool Accept(TESContainer::Entry* entry);
		bool Accept(InventoryEntryData* a_entryData);

		stl::flat_map<TESObjectWEAP*, std::int32_t> m_results;

	private:
		TESObjectWEAP* m_ignore;

		template <class T>
		void Process(TESForm* a_item, T a_count);
	};

	class EquipExtensions :
		public BSTEventSink<TESContainerChangedEvent>
	{
	public:
	protected:
		static bool CheckDualWield(Actor* a_actor);
		static bool ActorQualifiesForEquip(Actor* a_actor);

		void QueueEvaluateEquip(TESObjectREFR* a_actor) const;

		void EvaluateEquip(Actor* a_actor) const;

		virtual EventResult ReceiveEvent(
			const TESContainerChangedEvent*           a_evn,
			BSTEventSource<TESContainerChangedEvent>* a_dispatcher) override;
	};
}