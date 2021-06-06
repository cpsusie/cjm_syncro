#ifndef CJM_SYNCHRO_CONCEPTS
#define CJM_SYNCHRO_CONCEPTS
#include <chrono>
#include <concepts>
#include <type_traits>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#ifdef CJM_SYNCHRO_USE_BOOST_FEATURE
#include <boost/chrono.hpp>
#include <boost/thread/shared_mutex.hpp>
#endif
namespace cjm::synchro::concepts
{
	namespace detail
	{
		template<typename TRatio>
		concept ratio = std::is_nothrow_convertible_v<decltype(TRatio::num),
			std::intmax_t> && std::is_nothrow_convertible_v<decltype(TRatio::den),
				std::intmax_t>;

		template <typename T>
		concept is_bool = std::is_same_v<std::remove_cvref_t<std::remove_const_t<T>>, bool>;
		
		template<typename TFrom, typename TTo>
		concept nothrow_convertible_to = std::is_nothrow_convertible_v<TFrom, TTo>;

		template <typename TNumber>
		concept is_number = std::is_arithmetic_v<TNumber> && ((std::integral<TNumber> && !is_bool<TNumber>) || std::floating_point<TNumber>);

		template<typename TDuration>
		concept base_duration = std::is_arithmetic_v<typename TDuration::rep> && detail::ratio<typename TDuration::period>
			&& requires (const TDuration & d)
		{
			{d.count()} -> std::convertible_to<typename TDuration::rep>;
		};
		
		template<typename TDuration>
		concept std_duration = base_duration<TDuration> &&
			detail::nothrow_convertible_to<TDuration, std::chrono::duration<typename TDuration::rep, typename TDuration::period>>;

		template<typename TDuration>
		concept boost_duration =
#ifdef CJM_SYNCHRO_USE_BOOST_FEATURE
			base_duration<TDuration> && detail::nothrow_convertible_to<TDuration, boost::chrono::duration<typename TDuration::rep, typename TDuration::period>>;
#else
			false;
#endif

		template<typename TTimePoint>
		concept base_time_point = base_duration<typename TTimePoint::duration>
			&& detail::ratio<typename TTimePoint::period>
			&& requires (const TTimePoint & tp)
		{
			{tp.time_since_epoch()} -> detail::nothrow_convertible_to<typename TTimePoint::duration>;
		};

		template<typename TTimePoint>
		concept std_time_point = base_time_point<TTimePoint> && std_duration<typename TTimePoint::duration> && detail::nothrow_convertible_to<TTimePoint, std::chrono::time_point<typename TTimePoint::clock, typename TTimePoint::duration>>;

		template<typename TTimePoint>
		concept boost_time_point = base_time_point<TTimePoint> && boost_duration<typename TTimePoint::duration> && detail::nothrow_convertible_to<TTimePoint, boost::chrono::time_point<typename TTimePoint::clock, typename TTimePoint::duration>>;
	}
		
	template<typename TDuration>
	concept duration = detail::std_duration<TDuration> || detail::boost_duration<TDuration>;
	
	template<typename TTimePoint>
	concept time_point = detail::std_time_point<TTimePoint> || detail::boost_time_point<TTimePoint>;

	template<typename TDuration>
	concept other_duration = !duration<TDuration> && detail::base_duration<TDuration>;

	template<typename TTimePoint>
	concept other_timepoint = !time_point<TTimePoint> && detail::base_time_point<TTimePoint>;

	template<typename TLockable>
	concept basic_lockable = requires (TLockable& lockable)
	{
		{lockable.lock() };
		{lockable.unlock()};
	};

	template<typename TLockable>
	concept lockable = basic_lockable<TLockable>
		&& requires (TLockable& l)
	{
		{l.try_lock()} -> detail::nothrow_convertible_to<bool>;
	};

	template<typename TSharedLockable>
	concept shared_lockable = requires (TSharedLockable& sl)
	{
		{sl.lock_shared() };
		{sl.try_lock_shared()} -> detail::nothrow_convertible_to<bool>;
		{sl.unlock_shared()};
	};

	template<typename TUpgradeLockable>
	concept upgrade_lockable = shared_lockable<TUpgradeLockable> && requires (TUpgradeLockable & up)
	{
		{up.lock_upgrade()};
		{up.unlock_upgrade()};
		{up.try_lock_upgrade()} -> detail::nothrow_convertible_to<bool>;
		{up.unlock_and_lock_shared()};
		{up.unlock_and_lock_upgrade()};
		{up.unlock_upgrade_and_lock()};
		{up.unlock_upgrade_and_lock_shared()};		
	};

	template<typename TTimedLockable>
	concept std_timed_lockable = lockable<TTimedLockable> && requires (TTimedLockable & tl, const std::chrono::microseconds & dur, const std::chrono::steady_clock::time_point & tp)
	{
		{tl.try_lock_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tl.try_lock_until(tp)} -> detail::nothrow_convertible_to<bool>;
	};
	
	template<typename TTimedLockable>
	concept boost_timed_lockable =
#ifdef CJM_SYNCHRO_USE_BOOST_FEATURE
		lockable<TTimedLockable> && requires (TTimedLockable & tl, const boost::chrono::microseconds & dur, const boost::chrono::steady_clock::time_point & tp)
	{
		{tl.try_lock_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tl.try_lock_until(tp)} -> detail::nothrow_convertible_to<bool>;
	};
#else
		false;
#endif
	
	template<typename TTimedLockable>
	concept timed_lockable = boost_timed_lockable<TTimedLockable> || std_timed_lockable<TTimedLockable>;

	template<typename TTimedLockableWith, typename TDuration, typename TTimepoint>
	concept timed_lockable_with = lockable<TTimedLockableWith> && detail::base_duration<TDuration> && detail::base_time_point<TTimepoint> &&
		requires (TTimedLockableWith& ttlw, const TDuration& dur, const TTimepoint& tp)
	{
		{ttlw.try_lock_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{ttlw.try_lock_until(tp)} -> detail::nothrow_convertible_to<bool>;
	};
	
	template<typename TTimedSharedLockable>
	concept std_shared_timed_locable = lockable<TTimedSharedLockable> && timed_lockable<TTimedSharedLockable> && shared_lockable<TTimedSharedLockable> &&
			requires (TTimedSharedLockable & tl, const std::chrono::microseconds & dur, const std::chrono::steady_clock::time_point& tp)
	{
		{tl.try_lock_shared_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tl.try_lock_shared_until(tp)} -> detail::nothrow_convertible_to<bool>;
	};

	template<typename TTimedSharedLockable>
	concept boost_shared_timed_lockable =
#ifdef CJM_SYNCHRO_USE_BOOST_FEATURE
		lockable<TTimedSharedLockable> && timed_lockable<TTimedSharedLockable> && shared_lockable<TTimedSharedLockable> &&
		requires (TTimedSharedLockable & tl, const boost::chrono::microseconds & dur, const boost::chrono::steady_clock::time_point & tp)
	{
		{tl.try_lock_shared_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tl.try_lock_shared_until(tp)} -> detail::nothrow_convertible_to<bool>;
	};
#else
	false;
#endif
	
	template<typename TTimedSharedLockable>
	concept shared_timed_lockable =
		shared_lockable<TTimedSharedLockable> && lockable<TTimedSharedLockable>
		&& (std_shared_timed_locable<TTimedSharedLockable> || boost_shared_timed_lockable<TTimedSharedLockable>);

	template<typename TTimedSharedLockable, typename TDuration, typename TTimePoint>
	concept shared_timed_lockable_with = lockable<TTimedSharedLockable> && timed_lockable_with<TTimedSharedLockable, TDuration, TTimePoint> && 
		shared_lockable<TTimedSharedLockable>
			&& requires (TTimedSharedLockable& tl, const TDuration& dur, const TTimePoint& tp)
	{
		{tl.try_lock_shared_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tl.try_lock_shared_until(tp)} -> detail::nothrow_convertible_to<bool>;
	};

	template<typename TTimedUpgradeLockable>
	concept std_upgrade_timed_lockable = shared_timed_lockable<TTimedUpgradeLockable> && upgrade_lockable<TTimedUpgradeLockable> &&
		requires (TTimedUpgradeLockable & tul, const std::chrono::microseconds& dur, const std::chrono::steady_clock::time_point& tp)
	{
		{tul.try_lock_upgrade_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tul.try_lock_upgrade_until(tp)} -> detail::nothrow_convertible_to<bool>;
		{tul.try_unlock_upgrade_and_lock_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tul.try_unlock_upgrade_and_lock_until(tp)} -> detail::nothrow_convertible_to<bool>;
	};

	template<typename TTimedUpgradeLockable>
	concept boost_upgrade_timed_lockable =
#ifdef CJM_SYNCHRO_USE_BOOST_FEATURE
		shared_timed_lockable<TTimedUpgradeLockable> && upgrade_lockable<TTimedUpgradeLockable> &&
		requires (TTimedUpgradeLockable & tul, const boost::chrono::microseconds & dur, const boost::chrono::steady_clock::time_point & tp)
	{
		{tul.try_lock_upgrade_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tul.try_lock_upgrade_until(tp)} -> detail::nothrow_convertible_to<bool>;
		{tul.try_unlock_upgrade_and_lock_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tul.try_unlock_upgrade_and_lock_until(tp)} -> detail::nothrow_convertible_to<bool>;
	};
#else
		false;
#endif

	template<typename TTimedUpgradeLockable>
	concept upgrade_timed_lockable = std_upgrade_timed_lockable<TTimedUpgradeLockable> || boost_upgrade_timed_lockable<TTimedUpgradeLockable>;

	template<typename TTimedUpgradeLockable, typename TDuration, typename TTimePoint>
	concept upgrade_timed_lockable_with = shared_timed_lockable_with<TTimedUpgradeLockable, TDuration, TTimePoint> &&
		requires (TTimedUpgradeLockable & tul, const TDuration & dur, const TTimePoint & tp)
	{
		{tul.try_lock_upgrade_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tul.try_lock_upgrade_until(tp)} -> detail::nothrow_convertible_to<bool>;
		{tul.try_unlock_upgrade_and_lock_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tul.try_unlock_upgrade_and_lock_until(tp)} -> detail::nothrow_convertible_to<bool>;
	};
	
	template<typename TMutex>
	concept mutex = lockable<TMutex>
					&& std::is_default_constructible_v<TMutex>
					&& std::is_destructible_v<TMutex>
					&& !std::copy_constructible<TMutex>
					&& !std::is_copy_assignable_v<TMutex> && 
					!std::move_constructible<TMutex> && 
					!std::is_move_assignable_v<TMutex>;

	template<typename TSharedMutex>
	concept shared_mutex = 
		mutex<TSharedMutex> && shared_lockable<TSharedMutex>;

	template<typename TTimedMutex>
	concept timed_mutex = mutex<TTimedMutex> && timed_lockable<TTimedMutex>;

	template<typename TTimedSharedMutex, typename TDuration, typename TTimePoint>
	concept timed_shared_mutex = 
		shared_mutex<TTimedSharedMutex> && timed_mutex<TTimedSharedMutex> && shared_timed_lockable<TTimedSharedMutex>;

	template<typename TUpgradeMutex>
	concept upgrade_mutex = shared_mutex<TUpgradeMutex> && upgrade_lockable<TUpgradeMutex>;
	
	enum class mutex_level
	{
		std_mutex = 0,
		basic,
		shared,
		upgrade
	};

	enum class lock_state
	{
		none=0,
		shared,
		upgrade,
		exclusive
	};

	enum class time_type
	{
		not_timed_or_unknown = 0,
		std,
		boost,
	};

	template<mutex TMutex>
	struct mutex_exclusive_lock_type
	{
		using exclusive_lock = std::unique_lock<TMutex>;
	};

	template<typename TMutex>
		requires (shared_mutex<TMutex>)
	struct mutex_shared_lock_type
	{
		using shared_lock = std::shared_lock<TMutex>;
	};

	template<upgrade_mutex TMutex, upgrade_lockable TUpgradeLock>
	struct mutex_upgrade_lock_type
	{
		using upgrade_lock = std::remove_cvref_t<std::remove_const_t<TUpgradeLock>>;
	};
	
	template<mutex TMutex>
	struct mutex_traits
	{
		template<mutex_level Level>
		static constexpr time_type get_time_type_for_level() noexcept;
		static constexpr mutex_level get_mutex_level() noexcept;
		
		template<duration TDuration, time_point TTime>
		static constexpr bool supports_exclusive_timed_locks_with = timed_mutex<TMutex, TDuration, TTime>;
		
		static constexpr bool is_shared = shared_mutex<TMutex>;
		static constexpr bool is_timed = timed_mutex<TMutex>;
		static constexpr bool is_upgrade = upgrade_mutex<TMutex>;
		static constexpr bool is_std_mutex = std::is_same_v<std::remove_cvref_t<std::remove_const_t<TMutex>>, std::mutex>;
			
	};

	template <mutex TMutex>
	template <mutex_level Level>
	constexpr time_type mutex_traits<TMutex>::get_time_type_for_level() noexcept
	{
		if constexpr (Level == mutex_level::upgrade)
		{
			if constexpr (upgrade_mutex<TMutex>)
			{
				if constexpr (upgrade_timed_lockable<TMutex>)
				{
					if constexpr (std_upgrade_timed_lockable<TMutex>)
						return time_type::std;
					else if constexpr (boost_upgrade_timed_lockable<TMutex>)
						return time_type::boost;
					else
						return time_type::not_timed_or_unknown;
				}
				return time_type::not_timed_or_unknown;
			}
			return time_type::not_timed_or_unknown;
		}
		else if constexpr (Level == mutex_level::shared)
		{
			if constexpr (upgrade_mutex<TMutex> || shared_mutex<TMutex>)
			{
				if constexpr (upgrade_timed_lockable<TMutex> || shared_timed_lockable<TMutex>)
				{
					if constexpr (std_shared_timed_locable<TMutex>)
						return time_type::std;
					else if constexpr (boost_shared_timed_lockable<TMutex>)
						return time_type::boost;
					else
						return time_type::not_timed_or_unknown;
				}
				return time_type::not_timed_or_unknown;
			}
			return time_type::not_timed_or_unknown;
		}
		else if constexpr (Level == mutex_level::basic)
		{
			if constexpr (upgrade_mutex<TMutex> || shared_mutex<TMutex> || timed_mutex<TMutex>)
			{
				if constexpr (timed_lockable<TMutex>)
				{
					if constexpr (std_timed_lockable<TMutex>)
						return time_type::std;
					else if constexpr (boost_timed_lockable<TMutex>)
						return time_type::boost;
					else
						return time_type::not_timed_or_unknown;
				}
			}
		}

		return time_type::not_timed_or_unknown;
	}

	template <mutex TMutex>
	constexpr mutex_level mutex_traits<TMutex>::get_mutex_level() noexcept
	{
		if constexpr (is_std_mutex)
			return mutex_level::std_mutex;
		else if constexpr (is_upgrade)
			return mutex_level::upgrade;
		else if constexpr (is_shared)
			return mutex_level::shared;
		else
			return mutex_level::basic;
	}
	
	
	template<mutex TMutex>
	static constexpr mutex_level level_v = mutex_traits<TMutex>::get_mutex_level();

	template<mutex TMutex, mutex_level Level>
	static constexpr time_type time_library_v = mutex_traits<TMutex>::template get_time_type_for_level<Level>();

	
	
	

	
}
#endif
