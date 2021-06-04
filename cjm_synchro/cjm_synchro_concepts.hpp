#ifndef CJM_SYNCHRO_CONCEPTS
#define CJM_SYNCHRO_CONCEPTS
#include <chrono>
#include <concepts>
#include <type_traits>
#include <mutex>
#include <condition_variable>
namespace cjm::synchro::concepts
{
	namespace detail
	{
		template<typename TRatio>
		concept ratio = std::is_nothrow_convertible_v<decltype(TRatio::num),
			std::intmax_t> && std::is_nothrow_convertible_v<decltype(TRatio::den),
				std::intmax_t>;

		template<typename TFrom, typename TTo>
		concept nothrow_convertible_to = std::is_nothrow_convertible_v<TFrom, TTo>;
	}

	template<typename TDuration>
	concept duration = std::is_arithmetic_v<typename TDuration::rep> && detail::ratio<typename TDuration::period>
		&& requires (const TDuration & d)
	{
		{d.count()} -> std::convertible_to<typename TDuration::rep>;
	};

	template<typename TTimePoint>
	concept time_point = duration<typename TTimePoint::duration>
		&& detail::ratio<typename TTimePoint::period>
		&& requires (const TTimePoint & tp)
	{
		{tp.time_since_epoch()} -> detail::nothrow_convertible_to<typename TTimePoint::duration>;
	};

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
		{up.try_unlock_upgrade_and_lock()} -> detail::nothrow_convertible_to<bool>;
	};

	template<typename TTimedLockable, typename TDuration, typename TTimePoint>
	concept timed_lockable = lockable<TTimedLockable> &&  duration<TDuration> && time_point<TTimePoint> &&
		requires (TTimedLockable& tl, const TDuration & dur, const TTimePoint & tp)
	{
		{tl.try_lock_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tl.try_lock_until(tp)} -> detail::nothrow_convertible_to<bool>;
	};

	template<typename TTimedSharedLockable, typename TDuration, typename TTimePoint>
	concept shared_timed_lockable = lockable<TTimedSharedLockable> && timed_lockable<TTimedSharedLockable, TDuration, TTimePoint> && 
		shared_lockable<TTimedSharedLockable>
			&& requires (TTimedSharedLockable& tl, const TDuration& dur, const TTimePoint& tp)
	{
		{tl.try_lock_shared_for(dur)} -> detail::nothrow_convertible_to<bool>;
		{tl.try_lock_shared_until(tp)} -> detail::nothrow_convertible_to<bool>;
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

	template<typename TTimedMutex, typename TDuration, typename TTimePoint>
	concept timed_mutex = mutex<TTimedMutex> && timed_lockable<TTimedMutex, TDuration, TTimePoint>;

	template<typename TTimedSharedMutex, typename TDuration, typename TTimePoint>
	concept timed_shared_mutex = 
		shared_mutex<TTimedSharedMutex> && timed_mutex<TTimedSharedMutex, TDuration, TTimePoint>;

	
	
	template<mutex TMutex>
	struct mutex_traits
	{
		template<duration TDuration, time_point TTime>
		static constexpr bool supports_exclusive_timed_locks_with = timed_mutex<TMutex, TDuration, TTime>;
		static constexpr bool is_shared = shared_mutex<TMutex>;
		static constexpr bool is_timed = timed_mutex<TMutex>;
		static constexpr bool is_upgrade = upgrade_mutex<TMutex>;
		static constexpr bool supports_condition_variable = std::is_same_v<std::remove_cvref_t<TMutex>, std::mutex>;

		
		
		
	};
};
#endif