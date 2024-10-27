// Copyright Danyil Melnytskyi 2022-Present
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef SRC_RUN_TIME_ASM_ATTACHA_ENVIRONMENT
#define SRC_RUN_TIME_ASM_ATTACHA_ENVIRONMENT
#include <asmjit/asmjit.h>
#include <run_time/tasks.hpp>
#include <run_time/types_global.hpp>
#include <run_time/values_global.hpp>
namespace art {


    struct frame_info {
        art::ustring name;
        art::ustring file;
        size_t fun_size = 0;

        struct line_info {
            uint64_t offset_begin;
            uint64_t offset_end;
            art::line_info abstracted;
        };

        list_array<line_info> line_infos;
    };

    struct FrameSymbols {
        std::unordered_map<uint8_t*, frame_info, art::hash<uint8_t*>> map;
        bool destroyed = false;

        FrameSymbols() = default;

        ~FrameSymbols() {
            map.clear();
            destroyed = true;
        }
    };

    class attacha_environment {
        struct function_globals_handle;

        struct code_gen_handle {
            TaskMutex frame_symbols_lock;
            FrameSymbols frame_symbols;
            asmjit::JitRuntime run_time;
#if PLATFORM_WINDOWS
            art::mutex DbgHelp_lock;
#endif
        };

        TaskRecursiveMutex mutex;
        values_global* _value_global;
        types_global* _types_global;
        function_globals_handle* function_globals;
        code_gen_handle* code_gen;
        static attacha_environment self;
        attacha_environment();

        static function_globals_handle* create_function_globals();

        static void remove_function_globals(function_globals_handle*);


    public:
        ~attacha_environment();
        static function_globals_handle& get_function_globals();
        static values_global& get_value_globals();
        static types_global& get_types_global();
        static code_gen_handle& get_code_gen();

        static art::shared_ptr<class FuncEnvironment>& create_fun_env(class FuncEnvironment* ptr);
    };
} // namespace art

#endif /* SRC_RUN_TIME_ASM_ATTACHA_ENVIRONMENT */
