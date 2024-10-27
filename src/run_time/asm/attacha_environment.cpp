#include <run_time/asm/attacha_environment.hpp>

namespace art {
    attacha_environment attacha_environment::self;

    attacha_environment::attacha_environment() {
        _value_global = nullptr;
        _types_global = nullptr;
        function_globals = nullptr;
        code_gen = nullptr;
    }

    attacha_environment::~attacha_environment() {
        art::lock_guard<art::TaskRecursiveMutex> lock(self.mutex);
        if (_value_global)
            delete _value_global;

        if (_types_global)
            delete _types_global;

        if (function_globals)
            remove_function_globals(function_globals);

        if (code_gen)
            delete code_gen;
    }

    attacha_environment::function_globals_handle& attacha_environment::get_function_globals() {
        art::lock_guard<art::TaskRecursiveMutex> lock(self.mutex);
        if (!self.function_globals)
            self.function_globals = create_function_globals();
        return *self.function_globals;
    }

    values_global& attacha_environment::get_value_globals() {
        art::lock_guard<art::TaskRecursiveMutex> lock(self.mutex);
        if (!self._value_global)
            self._value_global = new values_global();
        return *self._value_global;
    }

    types_global& attacha_environment::get_types_global() {
        art::lock_guard<art::TaskRecursiveMutex> lock(self.mutex);
        if (!self._types_global)
            self._types_global = new types_global();
        return *self._types_global;
    }

    attacha_environment::code_gen_handle& attacha_environment::get_code_gen() {
        art::lock_guard<art::TaskRecursiveMutex> lock(self.mutex);
        if (!self.code_gen)
            self.code_gen = new code_gen_handle();
        return *self.code_gen;
    }
} // namespace art