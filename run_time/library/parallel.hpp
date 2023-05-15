// Copyright Danyil Melnytskyi 2022-2023
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "../attacha_abi_structs.hpp"

namespace parallel {
	void init();
	namespace constructor {
		ValueItem* createProxy_ConditionVariable(ValueItem*, uint32_t);
		ValueItem* createProxy_Mutex(ValueItem*, uint32_t);
		ValueItem* createProxy_Semaphore(ValueItem*, uint32_t);

		ValueItem* createProxy_EventSystem(ValueItem*, uint32_t);
		ValueItem* createProxy_TaskLimiter(ValueItem*, uint32_t);

		//args: [func, (fault handler), (timeout), (used_task_local)], sarr,farr[args....]
		ValueItem* createProxy_TaskQuery(ValueItem*, uint32_t);

		//ValueItem* createProxy_ValueMonitor(ValueItem*, uint32_t);
		//ValueItem* createProxy_ValueChangeMonitor(ValueItem*, uint32_t);
	}

	//typed_lgr<FuncEnviropment>*, any...
	ValueItem* createThread(ValueItem*, uint32_t);

	//returns function result that what reached by native thread
	ValueItem* createThreadAndWait(ValueItem*, uint32_t);

	//returns task that wait native thread and return function result
	ValueItem* createAsyncThread(ValueItem*, uint32_t);

	//returns task, args: [func, (fault handler), (timeout), (used_task_local)], sarr,farr[args....]
	ValueItem* createTask(ValueItem*, uint32_t);
}