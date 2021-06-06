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

	template<typename TLocked, concepts::mutex TMutex, concepts::mutex_level Level>
	class locked_ptr_base;

	template<typename TLocked, concepts::mutex TMutex, concepts::mutex_level Level>
	class scoped_unlock;
	
	template<typename TLocked>
	class locked_ptr_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>
	{
	public:
		friend class scoped_unlock<TLocked, std::mutex, concepts::mutex_level::std_mutex>;
		using scoped_unlock_t = scoped_unlock<TLocked, std::mutex, concepts::mutex_level::std_mutex>;
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

		[[nodiscard]] scoped_unlock_t scoped_unlock_impl();

		void wait_impl(condition_variable_t& cv)
		{
			assert(is_locked_impl());
			cv.wait(m_lock);
		}

		template<std::predicate<bool()> Predicate>
		void wait_impl(condition_variable_t& cv, Predicate p)
		{
			assert(is_locked_impl());
			cv.wait(m_lock, p);
		}
		
		[[nodiscard]] std::add_lvalue_reference<locked_t> locked_value() const
		{
			assert(is_locked_impl() && m_locked != nullptr);
			return *m_locked;
		}

				
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
			m_locked = nullptr;
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

	

	template <typename TLocked>
	class scoped_unlock<TLocked, std::mutex, concepts::mutex_level::std_mutex>
	{
	public:
		using locked_ptr_base_t = locked_ptr_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>;
		using pointer_t = typename locked_ptr_base_t::locked_ptr_t;
		scoped_unlock& operator=(scoped_unlock&& other) noexcept = delete;
		scoped_unlock& operator=(const scoped_unlock& other) = delete;
		scoped_unlock() = delete;
		scoped_unlock(scoped_unlock&& other) noexcept = delete;
		scoped_unlock(const scoped_unlock& other) = delete;
		scoped_unlock(locked_ptr_base_t& locked_ptr) : m_ptr(&locked_ptr)
		{
			assert(locked_ptr.is_locked_impl());
			std::swap(locked_ptr.m_locked, m_locked_val);
			locked_ptr.m_lock.unlock();
			assert(!m_ptr->is_locked_impl() && m_ptr->m_locked == nullptr && m_locked_val != nullptr);
		}
		~scoped_unlock()
		{
			assert(static_cast<bool>(m_ptr) && !m_ptr->is_locked_impl());
			m_ptr->lock_impl();
			std::swap(m_locked_val, m_ptr->m_locked);
			assert(m_ptr->is_locked_impl());
		}

	private:
		pointer_t m_locked_val;
		locked_ptr_base_t* m_ptr;
	};

	template <typename TLocked>
	auto locked_ptr_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>::scoped_unlock_impl() -> scoped_unlock_t
	{
		return scoped_unlock_t{ *this };
	}
	
}

#endif // !CJM_SYNCHRO_SYNCBASE_
