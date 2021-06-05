#ifndef CJM_SYNCHRO_SYNCBASE_
#define CJM_SYNCHRO_SYNCBASE_
#include "cjm_synchro_concepts.hpp"
#include <type_traits>
#include <concepts>
#include <mutex>
#include <cassert>
#include <utility>

namespace cjm::synchro::detail
{

	template<typename TLocked, concepts::mutex TMutex, concepts::mutex_level Level, bool Timed>
	class locked_ptr_base;

	
	template<typename TLocked>
	class locked_ptr_base<TLocked, std::mutex, concepts::mutex_level::std_mutex, false>
	{
	public:	
		using locked_t = std::remove_reference_t<TLocked>;
		using mutex_t = std::mutex;
		using lock_t = std::unique_lock<std::mutex>;
		using condition_variable_t = std::condition_variable;
		using locked_ptr_t = locked_t*;
		using unlocker_data_t = std::pair<std::unique_lock<mutex_t>, locked_ptr_t>;
		~locked_ptr_base() = default;
		locked_ptr_base(const locked_ptr_base& other) = delete;
		locked_ptr_base(locked_ptr_base&& other) noexcept = delete;
		locked_ptr_base& operator=(const locked_ptr_base& other) = delete;
		locked_ptr_base& operator=(locked_ptr_base&& other) noexcept = delete;
	
	protected:

		[[nodiscard]] bool is_empty_impl() const noexcept
		{
			return get_mutex_impl() == nullptr && m_locked == nullptr;
		}
		
		void lock_impl(unlocker_data_t unlocker_data)
		{
			assert(!is_locked_impl());
			m_lock = std::move(unlocker_data.first);
			m_locked = unlocker_data.second;
			assert(m_locked != nullptr && m_lock.mutex() != nullptr && !m_lock.owns_lock());
			m_lock.lock();
			assert(is_locked_impl());
		}

		unlocker_data_t unlock_impl()
		{
			assert(is_locked_impl());
			unlocker_data_t ret = std::make_pair(std::move(m_lock), m_locked);
			ret.first.unlock();
			assert(ret.second != nullptr && !is_locked_impl() && is_empty_impl());
			return ret;
		}
		
		[[nodiscard]] bool is_locked_impl() const noexcept
		{
			assert(m_lock.mutex() != nullptr && (!m_lock.owns_lock || m_locked != nullptr));
			return m_lock.owns_lock();
		}

		[[nodiscard]] mutex_t* get_mutex_impl() const noexcept
		{
			return m_lock.mutex();
		}
		
		explicit locked_ptr_base(std::unique_lock<mutex_t> lock, TLocked* locked) : m_lock{std::move(lock)}, m_locked{locked}
		{
			assert(!m_lock.owns_lock || m_locked != nullptr);
		}
		locked_ptr_base() = default;
	private:
		std::unique_lock<mutex_t> m_lock;
		TLocked* m_locked;
	};

}

#endif // !CJM_SYNCHRO_SYNCBASE_
