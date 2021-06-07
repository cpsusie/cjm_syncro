#ifndef CJM_SYNCHRO_HPP_
#define CJM_SYNCHRO_HPP_
#include "cjm_synchro_syncbase.hpp"

namespace cjm::synchro
{
	template<typename TLocked, concepts::mutex TMutex, concepts::mutex_level Level>
	class locked_ptr;

	template<typename TLocked, concepts::mutex TMutex, concepts::mutex_level Level>
	class synchro_vault;

	template<typename TLocked, concepts::mutex TMutex, concepts::mutex_level Level>
	class locked_ptr : public detail::locked_ptr_base<TLocked, TMutex, Level>
	{
		using synchro_vault_t = synchro_vault<TLocked, TMutex, Level>;
		friend class synchro_vault_t;
		using ctrl_block
	public:
		locked_ptr() = default;
	private:
		explicit locked_ptr(ctrl_blck_ptr_t ptr)
	};
}


#endif
