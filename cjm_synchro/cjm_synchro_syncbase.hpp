#ifndef CJM_SYNCHRO_SYNCBASE_
#define CJM_SYNCHRO_SYNCBASE_
#include "cjm_synchro_concepts.hpp"
#include <type_traits>
#include <concepts>
#include <mutex>
#include <cassert>
#include <utility>
#ifdef CJM_SYNCHRO_USE_BOOST_FEATURE
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/lock_types.hpp>
#endif

namespace cjm::synchro::detail
{
	static constexpr bool using_boost =
#ifdef CJM_SYNCHRO_USE_BOOST_FEATURE
		true;
#else
		false;
#endif
	enum class lock_release_notify
	{
		none = 0,
		one,
		all
	};

	template<typename TLocked, concepts::mutex TMutex, concepts::mutex_level Level>
	class ctrl_block;


	template<typename TLocked, concepts::mutex TMutex, concepts::mutex_level Level>
	class locked_ptr_base;

	template<typename TLocked, concepts::mutex TMutex, concepts::mutex_level Level>
	class scoped_unlock;

	template<typename TLocked, concepts::mutex TMutex, concepts::mutex_level Level>
	class synchro_vault_base;

	template<typename TLocked, concepts::mutex TMutex, concepts::mutex_level Level>
	class ctrl_block
	{
	protected:
		static constexpr concepts::mutex_level level = Level;
	
		using synchro_vault_t = synchro_vault_base<TLocked, TMutex, Level>;
		using scoped_unlock_t = scoped_unlock<TLocked, TMutex, Level>;
		using locked_datum_t = std::remove_reference_t<TLocked>;
		using const_locked_datum_t = std::add_const_t<locked_datum_t>;
		using locked_datum_ref_t = std::add_lvalue_reference_t<locked_datum_t>;
		using const_locked_datum_ref_t = std::add_lvalue_reference_t<const_locked_datum_t>;
		using mutex_t = TMutex;
		using lock_t = std::unique_lock<TMutex>;
		using shared_lock_t = std::conditional_t<level == concepts::mutex_level::shared || level == concepts::mutex_level::upgrade, std::shared_lock<mutex_t>, void>;
		using upgrade_lock_t = std::conditional_t<using_boost&& level == concepts::mutex_level::upgrade, boost::upgrade_lock<TMutex>, void>;
		using condition_variable_t = std::conditional_t<Level == concepts::mutex_level::std_mutex, std::condition_variable, std::condition_variable_any>;
		using ptr_to_locked_datum = std::add_pointer_t<locked_datum_t>;
		using ptr_to_const_locked_datum = std::add_pointer_t<const_locked_datum_t>;
		using const_ptr_to_locked_datum = std::add_const_t<ptr_to_const_locked_datum>;
		using const_ptr_to_const_locked_datum = std::add_const_t<ptr_to_const_locked_datum>;
		using unlocker_data_t = std::pair<std::unique_lock<mutex_t>, ptr_to_locked_datum>;
		using vault_owner_t = synchro_vault_base<TLocked, TMutex, Level>;
		using locked_ptr_t = locked_ptr_base<TLocked, TMutex, Level>;
		static constexpr concepts::time_type condition_variable_time = concepts::time_type::std;
		friend class locked_ptr_t;
		friend class scoped_unlock_t;
		friend class synchro_vault_t;

		std::pair<lock_t, ctrl_block*> lock_impl() const
		{
			return std::make_pair(lock_t{ m_mutex }, this);
		}
	public:
		ctrl_block(const ctrl_block& cb) = delete;
		ctrl_block(ctrl_block&& cb) noexcept = delete;
		ctrl_block& operator=(const ctrl_block& cb) = delete;
		ctrl_block& operator=(ctrl_block&& cb) noexcept = delete;
		~ctrl_block() = default;
	protected:
		ctrl_block() noexcept(std::is_nothrow_default_constructible_v<locked_datum_t>)
			requires (std::is_default_constructible_v<locked_datum_t>)
			: m_mutex{}, m_condition_variable{}, m_locked{} {}
		explicit ctrl_block(const locked_datum_t & locked)
			noexcept(std::is_nothrow_copy_constructible_v<locked_datum_t>)
			requires std::copy_constructible<locked_datum_t> : m_mutex{}, m_condition_variable{}, m_locked{ locked } {}


		explicit ctrl_block(locked_datum_t && locked)
			noexcept(std::is_nothrow_move_constructible_v<locked_datum_t>)
			requires (std::move_constructible<locked_datum_t>) : m_mutex{}, m_condition_variable{}, m_locked{ std::move(locked) } {}
		template<typename...TArgs>
		requires (std::constructible_from<locked_datum_t, TArgs...>)
			ctrl_block(TArgs&&... args)
			noexcept(cjm::concepts::nothrow_constructible_from<locked_datum_t,
				TArgs...>)
			: m_mutex{}, m_condition_variable{},
			m_locked{ std::forward<TArgs>(args)... } {}
		mutable mutex_t m_mutex;
		mutable condition_variable_t m_condition_variable;
		locked_datum_t m_locked;
	};
	
	template<typename TLocked>
	class ctrl_block<TLocked, std::mutex, concepts::mutex_level::std_mutex>
	{
	protected:
		using synchro_vault_t = synchro_vault_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>;
		using scoped_unlock_t = scoped_unlock<TLocked, std::mutex, concepts::mutex_level::std_mutex>;
		using locked_datum_t = std::remove_reference_t<TLocked>;
		using mutex_t = std::mutex;
		using lock_t = std::unique_lock<std::mutex>;
		using condition_variable_t = std::condition_variable;
		using ptr_to_locked = locked_datum_t*;
		using unlocker_data_t = std::pair<std::unique_lock<mutex_t>, ptr_to_locked>;
		using vault_owner_t = synchro_vault_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>;
		using locked_ptr_t = locked_ptr_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>;
		static constexpr concepts::time_type condition_variable_time = concepts::time_type::std;
		static constexpr concepts::mutex_level level = concepts::mutex_level::std_mutex;
		friend class locked_ptr_t;
		friend class scoped_unlock_t;
		friend class synchro_vault_t;

		std::pair<lock_t, ctrl_block*> lock_impl() const
		{
			return std::make_pair(lock_t{ m_mutex }, this);
		}

	public:
		ctrl_block(const ctrl_block& cb) = delete;
		ctrl_block(ctrl_block&& cb) noexcept = delete;
		ctrl_block& operator=(const ctrl_block& cb) = delete;
		ctrl_block& operator=(ctrl_block&& cb) noexcept = delete;
		~ctrl_block() = default;
	protected:
		ctrl_block() noexcept(std::is_nothrow_default_constructible_v<locked_datum_t>)
			requires (std::is_default_constructible_v<locked_datum_t>)
			: m_mutex{}, m_condition_variable{}, m_locked{} {}
		explicit ctrl_block(const locked_datum_t& locked)
			noexcept(std::is_nothrow_copy_constructible_v<locked_datum_t>)
			requires std::copy_constructible<locked_datum_t> : m_mutex{}, m_condition_variable{}, m_locked{locked} {}

		
		explicit ctrl_block(locked_datum_t&& locked)
			noexcept(std::is_nothrow_move_constructible_v<locked_datum_t>)
			requires (std::move_constructible<locked_datum_t>) : m_mutex{}, m_condition_variable{}, m_locked{std::move(locked)} {}
		template<typename...TArgs>
			requires (std::constructible_from<locked_datum_t, TArgs...>)
		ctrl_block(TArgs&&... args)
			noexcept(cjm::concepts::nothrow_constructible_from<locked_datum_t, 
				TArgs...>)			
				: m_mutex{}, m_condition_variable{},
					m_locked{ std::forward<TArgs>(args)... } {}

		mutable mutex_t m_mutex;
		mutable condition_variable_t m_condition_variable;
		locked_datum_t m_locked;
	};
	template<typename TLocked>
	class locked_ptr_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>
	{
	public:
		using scoped_unlock_t = scoped_unlock<TLocked, std::mutex, concepts::mutex_level::std_mutex>;
		friend class scoped_unlock<TLocked, std::mutex, concepts::mutex_level::std_mutex>;
		using locked_t = std::remove_reference_t<TLocked>;
		using mutex_t = std::mutex;
		using lock_t = std::unique_lock<std::mutex>;
		using condition_variable_t = std::condition_variable;
		using locked_ptr_t = std::add_pointer_t<lock_t>;
		using ctrl_blck_t = ctrl_block<TLocked, std::mutex, concepts::mutex_level::std_mutex>;
		friend class ctrl_blck_t;
		using ctrl_blck_ptr_t = std::add_pointer_t<ctrl_blck_t>;
		static constexpr concepts::time_type condition_variable_time = ctrl_blck_t::condition_variable_time;

		using unlocker_data_t = std::pair<std::unique_lock<mutex_t>, ctrl_blck_ptr_t>;
		~locked_ptr_base();
		locked_ptr_base(const locked_ptr_base& other) = delete;
		locked_ptr_base(locked_ptr_base&& other) noexcept = delete;
		locked_ptr_base& operator=(const locked_ptr_base& other) = delete;
		locked_ptr_base& operator=(locked_ptr_base&& other) noexcept = delete;

	protected:

		lock_release_notify lock_release_setting() const noexcept
		{
			return m_cv_notify_on_destruct.load();
		}

		[[nodiscard]] scoped_unlock_t scoped_unlock_impl();

		void notify_one_impl()
		{
			m_ctrl_blck->m_condition_variable.notify_one();
		}

		void notify_all_impl()
		{
			m_ctrl_blck->m_condition_variable.notify_all();
		}

		void set_cv_release_notification(lock_release_notify lrn)
		{
			assert(lrn >= lock_release_notify::none && lrn <= lock_release_notify::all);
			m_cv_notify_on_destruct.store(lrn);
		}

		template<concepts::duration Duration>
		void wait_for_impl(const Duration& d)
		{
			assert(is_locked_impl());
			static_assert(
				(concepts::detail::std_duration<Duration> ||
					concepts::detail::boost_duration<Duration>)
				&&
				!(concepts::detail::std_duration<Duration> &&
					concepts::detail::boost_duration<Duration>));
			m_ctrl_blck->m_condition_variable.wait_for(d);
		}

		template<concepts::duration Duration, std::predicate<bool()> Predicate>
		void wait_for_impl(const Duration& d, Predicate p)
		{
			assert(is_locked_impl());
			m_ctrl_blck->m_condition_variable.wait_for(d, p);
		}

		template<concepts::time_point TimePoint>
		void wait_until_impl(const TimePoint& tp)
		{
			assert(is_locked_impl());
			static_assert(
				(concepts::detail::boost_time_point<TimePoint> ||
					concepts::detail::std_time_point<TimePoint>)
				&&
				!(concepts::detail::boost_time_point<TimePoint> &&
					concepts::detail::std_time_point<TimePoint>));
			m_ctrl_blck->m_condition_variable.wait_until(tp);
		}

		template<concepts::time_point TimePoint, std::predicate<bool()> Predicate>
		void wait_until_impl(const TimePoint& tp, Predicate p)
		{
			assert(is_locked_impl());
			static_assert(
				(concepts::detail::boost_time_point<TimePoint> ||
					concepts::detail::std_time_point<TimePoint>)
				&&
				!(concepts::detail::boost_time_point<TimePoint> &&
					concepts::detail::std_time_point<TimePoint>));
			m_ctrl_blck->m_condition_variable.wait_until(tp, p);
		}

		void wait_impl()
		{
			assert(is_locked_impl());
			m_ctrl_blck->m_condition_variable.wait();
		}

		template<std::predicate<bool()> Predicate>
		void wait_impl(Predicate p)
		{
			assert(is_locked_impl());
			m_ctrl_blck->m_condition_variable.wait(m_lock, p);
		}

		[[nodiscard]] std::add_lvalue_reference<locked_t> locked_value() const;


		[[nodiscard]] bool is_empty_impl() const noexcept
		{
			return get_mutex_impl() == nullptr && m_ctrl_blck == nullptr;
		}

		void lock_impl(unlocker_data_t unlocker_data)
		{
			assert(!is_locked_impl());
			m_lock = std::move(unlocker_data.first);
			m_ctrl_blck = unlocker_data.second;
			assert(m_ctrl_blck != nullptr && m_lock.mutex() != nullptr && !m_lock.owns_lock());
			m_lock.lock();
			assert(is_locked_impl());
		}

		unlocker_data_t unlock_impl()
		{
			assert(is_locked_impl());
			unlocker_data_t ret = std::make_pair(std::move(m_lock), m_ctrl_blck);
			m_ctrl_blck = nullptr;
			ret.first.unlock();
			assert(ret.second != nullptr && !is_locked_impl() && is_empty_impl());
			return ret;
		}

		[[nodiscard]] bool is_locked_impl() const noexcept
		{
			assert(m_lock.mutex() != nullptr && (!m_lock.owns_lock || m_ctrl_blck != nullptr));
			return m_lock.owns_lock();
		}

		[[nodiscard]] mutex_t* get_mutex_impl() const noexcept
		{
			return m_lock.mutex();
		}

		explicit locked_ptr_base(std::unique_lock<mutex_t> lock, ctrl_blck_ptr_t locked) : m_lock{ std::move(lock) }, m_ctrl_blck{ locked }
		{
			assert(!m_lock.owns_lock || m_ctrl_blck != nullptr);
		}
		locked_ptr_base() = default;
	private:

		std::atomic<lock_release_notify> m_cv_notify_on_destruct;
		lock_t m_lock;
		ctrl_blck_ptr_t m_ctrl_blck;
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
			m_locked_val = &(m_ptr->m_ctrl_blck->m_locked);
			locked_ptr.m_lock.unlock();
			assert(!m_ptr->is_locked_impl() && m_ptr->m_locked == nullptr && m_locked_val != nullptr);
		}
		~scoped_unlock()
		{
			assert(static_cast<bool>(m_ptr) && !m_ptr->is_locked_impl());
			m_ptr->lock_impl();
			std::swap(m_locked_val, m_ptr->m_ctrl_blck);
			assert(m_ptr->is_locked_impl());
		}

	private:
		pointer_t m_locked_val;
		locked_ptr_base_t* m_ptr;
	};

	template<typename TLocked>
	class synchro_vault_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>
	{
	protected:
		using ctrl_blck_t = ctrl_block<TLocked, std::mutex, concepts::mutex_level::std_mutex>;
		using scoped_unlock_t = scoped_unlock<TLocked, std::mutex, concepts::mutex_level::std_mutex>;
		using locked_t = typename ctrl_blck_t::locked_datum_t;
		using mutex_t = typename ctrl_blck_t::mutex_t;
		using lock_t = typename ctrl_blck_t::lock_t;
		using condition_variable_t = typename ctrl_blck_t::condition_variable_t;
		using ptr_to_locked = typename ctrl_blck_t::ptr_to_locked_datum;
		using unlocker_data_t = typename ctrl_blck_t::unlocker_data_t;
		using vault_owner_t = typename ctrl_blck_t::vault_owner_t;
		static_assert(std::is_same_v<synchro_vault_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>, vault_owner_t>);
		using locked_ptr_t = typename ctrl_blck_t::locked_ptr_t;
		friend class ctrl_blck_t;
		friend class scoped_unlock_t;		

		synchro_vault_base() noexcept(std::is_nothrow_default_constructible_v<locked_t>)
			requires (std::is_default_constructible_v<locked_t>) : m_ctrl_blck{} {}
		synchro_vault_base(const locked_t& locked_datum) noexcept(std::is_nothrow_copy_constructible_v<locked_t>)
			requires (std::copy_constructible<locked_t>) : m_ctrl_blck{ locked_datum } {}
		synchro_vault_base(locked_t&& locked_datum) noexcept(std::is_nothrow_move_constructible_v<locked_t>)
			requires (std::move_constructible<locked_t>) : m_ctrl_blck{ std::move(locked_datum) } {}
		template<typename...TArgs>
		requires (std::constructible_from<locked_t, TArgs...>)
			synchro_vault_base(TArgs&&... args) noexcept(cjm::concepts::nothrow_constructible_from<locked_t,
				TArgs...>) : m_ctrl_blck{ std::forward<TArgs>(args)... } {}

		[[nodiscard]] locked_ptr_t lock_impl() 
		{
			return locked_ptr_t{ lock_t{m_ctrl_blck.m_mutex}, &m_ctrl_blck };
		}

		void notify_one_impl() 
		{
			m_ctrl_blck.m_condition_variable.notify_one();
		}

		void notify_all_impl() 
		{
			m_ctrl_blck.m_condition_variable.notify_all();
		}


		[[nodiscard]] auto copy_locked_datum() const
			noexcept(std::is_nothrow_copy_constructible_v<locked_t>)->locked_t
			requires (std::copy_constructible<locked_t>)
		{
			auto lock = locked_t{ m_ctrl_blck.m_mutex };
			return m_ctrl_blck.m_locked;
		}

		auto release_locked_datum() noexcept -> locked_t
			requires (std::is_nothrow_move_constructible_v<locked_t>&& std::is_nothrow_default_constructible_v<locked_t>)
		{
			locked_t def_val;
			auto lock = locked_t{ m_ctrl_blck.m_mutex };
			std::swap(m_ctrl_blck.m_locked, def_val);
			return def_val;
		}

		auto swap_locked_datum(locked_t&& swap_me) noexcept -> locked_t
			requires (std::is_nothrow_swappable_v<locked_t>)
		{
			std::swap(swap_me, m_ctrl_blck.m_locked);
			return swap_me;
		}

		void assign_locked_datum(const locked_t& new_datum)
			noexcept (std::is_nothrow_copy_assignable_v<locked_t>)
			requires(std::is_copy_assignable_v<locked_t>)
		{
			auto lck = lock_t{ m_ctrl_blck.m_mutex };
			m_ctrl_blck.m_locked = new_datum;
		}

		void assign_locked_datum(locked_t&& new_datum)
			noexcept(std::is_nothrow_move_assignable_v<locked_t>)
			requires(std::is_move_assignable_v<locked_t>)
		{
			auto lck = lock_t{ m_ctrl_blck.m_mutex };
			m_ctrl_blck.m_locked = std::move(new_datum);
		}
	
	private:
		ctrl_blck_t m_ctrl_blck;
	};

	template <typename TLocked>
	locked_ptr_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>::~locked_ptr_base()
	{
		const lock_release_notify notify =
			m_cv_notify_on_destruct.exchange(lock_release_notify::none);
		if (m_lock.owns_lock())
		{
			m_lock.unlock();
		}
		if (notify == lock_release_notify::one)
		{
			m_ctrl_blck->m_condition_variable.notify_one();
		}
		else if (notify == lock_release_notify::all)
		{
			m_ctrl_blck->m_condition_variable.notify_all();
		}
	}

	template <typename TLocked>
	auto locked_ptr_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>::scoped_unlock_impl() -> scoped_unlock_t
	{
		return scoped_unlock_t{ *this };
	}

	template <typename TLocked>
	auto locked_ptr_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>::locked_value() const -> std::add_lvalue_reference<typename locked_ptr_base<TLocked, std::mutex, concepts::mutex_level::std_mutex>::locked_t>
	{
		assert(is_locked_impl() && m_ctrl_blck != nullptr);
		return m_ctrl_blck->m_locked;
	}
}
#endif