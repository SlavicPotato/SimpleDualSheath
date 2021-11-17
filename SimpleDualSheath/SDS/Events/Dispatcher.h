#pragma once

namespace SDS
{
	namespace Events
	{
		template <class T>
		class EventSink
		{
			using event_type = stl::strip_type<T>;

		public:
			virtual ~EventSink() noexcept = default;

			virtual void Receive(const event_type& a_evn) = 0;
		};

		template <class T>
		class EventDispatcher
		{
			using event_type = stl::strip_type<T>;
			using sink_type = EventSink<event_type>;
			using storage_type = std::vector<sink_type*>;

		public:
			virtual ~EventDispatcher() noexcept = default;

			void SendEvent(const event_type& a_evn) const
			{
				for (auto& e : m_sinks)
				{
					e->Receive(a_evn);
				}
			}

			void AddSink(sink_type* a_sink)
			{
				if (Find(a_sink) == m_sinks.end())
				{
					m_sinks.emplace_back(a_sink);
				}
			}

			void RemoveSink(sink_type* a_sink) noexcept
			{
				auto it = Find(a_sink);
				if (it != m_sinks.end())
				{
					m_sinks.erase(it);
				}
			}

		private:
			auto Find(sink_type* a_sink) const noexcept
			{
				return std::find(m_sinks.cbegin(), m_sinks.cend(), a_sink);
			}

			auto Find(sink_type* a_sink) noexcept
			{
				return std::find(m_sinks.begin(), m_sinks.end(), a_sink);
			}

			storage_type m_sinks;
		};
	}
}
