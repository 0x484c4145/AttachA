// Copyright Danyil Melnytskyi 2022-2023
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef SRC_RUN_TIME_EXCEPTIONS
#define SRC_RUN_TIME_EXCEPTIONS

#include <string>
#include <exception>
#include "../library/list_array.hpp"
#include "../library/string_help.hpp"
namespace art {
#pragma region AnyTimeExceptions
	class AttachARuntimeException {
		std::string message;
		std::exception_ptr inner_exception;
	public:
		AttachARuntimeException() { message = ""; }
		AttachARuntimeException(std::exception_ptr inner_exception) : inner_exception(inner_exception) { message = ""; }
		AttachARuntimeException(const std::string& msq) : message(msq) {}
		AttachARuntimeException(const std::string& msq, std::exception_ptr inner_exception) : inner_exception(inner_exception), message(msq) {}
		virtual ~AttachARuntimeException() noexcept(false) {}
		const std::string& what() const {
			return message;
		}
		virtual std::string full_info() const;
		virtual const char* name() const {
			return "attach_a_runtime_exception";
		}
		void throw_iner_exception() const  {
			std::rethrow_exception(inner_exception);
		}
		std::exception_ptr get_iner_exception() const  {
			return inner_exception;
		}
	};
	class InvalidCast : public virtual AttachARuntimeException {
	public:
		InvalidCast(const std::string& msq) : AttachARuntimeException(msq) {}
		InvalidCast(const std::string& msq, std::exception_ptr inner_exception) : AttachARuntimeException(msq, inner_exception) {}
		const char* name() const override {
			return "invalid_cast";
		}
	};
	class UnmodifabeValue : public AttachARuntimeException {
	public:
		UnmodifabeValue() : AttachARuntimeException() {}
		UnmodifabeValue(std::exception_ptr inner_exception) : AttachARuntimeException(inner_exception) {}
		const char* name() const override {
			return "unmodifabe_value";
		}
	};
	class InvalidOperation : public virtual AttachARuntimeException {
	public:
		InvalidOperation(const std::string& msq) : AttachARuntimeException(msq) {}
		InvalidOperation(const std::string& msq, std::exception_ptr inner_exception) : AttachARuntimeException(msq, inner_exception) {}
		const char* name() const override {
			return "invalid_operation";
		}
	};
	class InvalidArguments : public AttachARuntimeException {
	public:
		InvalidArguments(const std::string& msq) : AttachARuntimeException(msq) {}
		InvalidArguments(const std::string& msq, std::exception_ptr inner_exception) : AttachARuntimeException(msq, inner_exception) {}
		const char* name() const override {
			return "invalid_arguments";
		}
	};
	class InvalidLock : public InvalidOperation {
	public:
		InvalidLock(const std::string& msq) : InvalidOperation(msq) {}
		InvalidLock(const std::string& msq, std::exception_ptr inner_exception) : InvalidOperation(msq, inner_exception) {}
		const char* name() const override {
			return "invalid_lock";
		}
	};
	class InvalidUnlock : public InvalidOperation {
	public:
		InvalidUnlock(const std::string& msq) : InvalidOperation(msq) {}
		InvalidUnlock(const std::string& msq, std::exception_ptr inner_exception) : InvalidOperation(msq, inner_exception) {}
		const char* name() const override {
			return "invalid_unlock";
		}
	};
	class InvalidInput : public AttachARuntimeException {
	public:
		InvalidInput(const std::string& msq) : AttachARuntimeException(msq) {}
		InvalidInput(const std::string& msq, std::exception_ptr inner_exception) : AttachARuntimeException(msq, inner_exception) {}
		const char* name() const override {
			return "invalid_input";
		}
	};
	class NotImplementedException : public virtual AttachARuntimeException {
	public:
		NotImplementedException() : AttachARuntimeException("Entered to non implemented region") {}
		NotImplementedException(std::exception_ptr inner_exception) : AttachARuntimeException("Entered to non implemented region", inner_exception) {}
		const char* name() const override {
			return "attach_a_runtime_exception";
		}
	};
	class UnsupportedOperation : public NotImplementedException, public InvalidOperation {
	public:
		UnsupportedOperation() : InvalidOperation("Caught unsupported Operation") {}
		UnsupportedOperation(std::exception_ptr inner_exception) : InvalidOperation("Caught unsupported Operation", inner_exception) {}
		UnsupportedOperation(const std::string& msq) : InvalidOperation(msq) {}
		UnsupportedOperation(const std::string& msq, std::exception_ptr inner_exception) : InvalidOperation(msq, inner_exception) {}
		const char* name() const override {
			return "unsupported_operation";
		}
	};
	class OutOfRange : public AttachARuntimeException {
	public:
		OutOfRange() : AttachARuntimeException("Out of range") {}
		OutOfRange(std::exception_ptr inner_exception) : AttachARuntimeException("Out of range", inner_exception) {}
		OutOfRange(const std::string& str) : AttachARuntimeException(str) {}
		OutOfRange(const std::string& str, std::exception_ptr inner_exception) : AttachARuntimeException(str, inner_exception) {}
		const char* name() const override {
			return "out_of_range";
		}
	};
	class InvalidClassDeclarationException : public AttachARuntimeException {
	public:
		InvalidClassDeclarationException() : AttachARuntimeException("Invalid Class Declaration Exception") {}
		InvalidClassDeclarationException(const std::string& desc) : AttachARuntimeException("Invalid Class Declaration Exception: " + desc) {}
		const char* name() const override {
			return "invalid_class_declaration_exception";
		}
	};
	class LibrayNotFoundException : public AttachARuntimeException {
	public:
		LibrayNotFoundException() : AttachARuntimeException("Libray not found") {}
		LibrayNotFoundException(std::exception_ptr inner_exception) : AttachARuntimeException("Libray not found", inner_exception) {}
		LibrayNotFoundException(const std::string& desc) : AttachARuntimeException(desc) {}
		LibrayNotFoundException(const std::string& desc, std::exception_ptr inner_exception) : AttachARuntimeException(desc, inner_exception) {}
		const char* name() const override {
			return "libray_not_found_exception";
		}
	};
	class LibrayFunctionNotFoundException : public AttachARuntimeException {
	public:
		LibrayFunctionNotFoundException() : AttachARuntimeException("Libray function not found") {}
		LibrayFunctionNotFoundException(std::exception_ptr inner_exception) : AttachARuntimeException("Libray function not found", inner_exception) {}
		LibrayFunctionNotFoundException(const std::string& desc) : AttachARuntimeException(desc) {}
		LibrayFunctionNotFoundException(const std::string& desc, std::exception_ptr inner_exception) : AttachARuntimeException(desc, inner_exception) {}
		const char* name() const override {
			return "libray_function_not_found_exception";
		}
	};
	class EnviropmentRuinException : public AttachARuntimeException {
	public:
		EnviropmentRuinException() : AttachARuntimeException("Enviropment ruin exception") {}
		EnviropmentRuinException(std::exception_ptr inner_exception) : AttachARuntimeException("Enviropment ruin exception", inner_exception) {}
		EnviropmentRuinException(const std::string& desc) : AttachARuntimeException("EnviropmentRuinException: " + desc) {}
		EnviropmentRuinException(const std::string& desc, std::exception_ptr inner_exception) : AttachARuntimeException(desc, inner_exception) {}
		const char* name() const override {
			return "enviropment_ruin_exception";
		}
	};
	class InvalidArchitectureException : public AttachARuntimeException {
	public:
		InvalidArchitectureException() : AttachARuntimeException("Invalid archetecture") {}
		InvalidArchitectureException(std::exception_ptr inner_exception) : AttachARuntimeException("Invalid archetecture", inner_exception) {}
		const char* name() const override {
			return "invalid_architecture_exception";
		}
	};
	class StackOverflowException : public AttachARuntimeException {
	public:
		StackOverflowException() {}
		StackOverflowException(std::exception_ptr inner_exception) : AttachARuntimeException("Stack overflow", inner_exception) {}
		const char* name() const override {
			return "stack_overflow_exception";
		}
	};
	class UnusedDebugPointException : public AttachARuntimeException {
	public:
		UnusedDebugPointException() : AttachARuntimeException("Unused debug breakpoint") {}
		UnusedDebugPointException(std::exception_ptr inner_exception) : AttachARuntimeException("Unused debug breakpoint",inner_exception) {}
		const char* name() const override {
			return "unused_debug_point_exception";
		}
	};
	class DevideByZeroException : public AttachARuntimeException {
	public:
		DevideByZeroException() : AttachARuntimeException("Number devided by zero") {}
		DevideByZeroException(std::exception_ptr inner_exception) : AttachARuntimeException("Number devided by zero", inner_exception) {}
		const char* name() const override {
			return "devide_by_zero_exception";
		}
	};
	class BadInstructionException : public AttachARuntimeException {
	public:
		BadInstructionException() : AttachARuntimeException("This instruction undefined") {}
		BadInstructionException(std::exception_ptr inner_exception) : AttachARuntimeException("This instruction undefined", inner_exception) {}
		BadInstructionException(const std::string& msq) : AttachARuntimeException(msq) {}
		BadInstructionException(const std::string& msq, std::exception_ptr inner_exception) : AttachARuntimeException(msq, inner_exception) {}
		const char* name() const override {
			return "bad_instruction_exception";
		}
	};
	class NumericOverflowException : public AttachARuntimeException {
	public:
		NumericOverflowException() : AttachARuntimeException("Caught numeric overflow") {}
		NumericOverflowException(std::exception_ptr inner_exception) : AttachARuntimeException("Caught numeric overflow", inner_exception) {}
		const char* name() const override {
			return "numeric_overflow_exception";
		}
	};
	class NumericUndererflowException : public AttachARuntimeException {
	public:
		NumericUndererflowException() : AttachARuntimeException("Caught numeric underflow") {}
		NumericUndererflowException(std::exception_ptr inner_exception) : AttachARuntimeException("Caught numeric underflow", inner_exception) {}
		const char* name() const override {
			return "numeric_undererflow_exception";
		}
	};
	class SegmentationFaultException : public AttachARuntimeException {
	public:
		SegmentationFaultException() : AttachARuntimeException("Thread try get access to non mapped region") {}
		SegmentationFaultException(std::exception_ptr inner_exception) : AttachARuntimeException("Thread try get access to non mapped region", inner_exception) {}
		SegmentationFaultException(const std::string& text) : AttachARuntimeException(text) {}
		SegmentationFaultException(const std::string& text, std::exception_ptr inner_exception) : AttachARuntimeException(text, inner_exception) {}
		const char* name() const override {
			return "segmentation_fault_exception";
		}
	};
	class NullPointerException : public SegmentationFaultException {
	public:
		NullPointerException() : SegmentationFaultException("Thread try get access to null pointer region") {}
		NullPointerException(std::exception_ptr inner_exception) : SegmentationFaultException("Thread try get access to null pointer region", inner_exception) {}
		NullPointerException(const std::string& text) : SegmentationFaultException(text) {}
		NullPointerException(const std::string& text, std::exception_ptr inner_exception) : SegmentationFaultException(text, inner_exception) {}
		const char* name() const override {
			return "null_pointer_exception";
		}
	};
	class NoMemoryException : public AttachARuntimeException {
	public:
		NoMemoryException() : AttachARuntimeException("No memory") {}
		NoMemoryException(std::exception_ptr inner_exception) : AttachARuntimeException("No memory", inner_exception) {}
		const char* name() const override {
			return "no_memory_exception";
		}
	};

	class AttachedLangException : public AttachARuntimeException {
	public:
		AttachedLangException() : AttachARuntimeException("Caught unconvertable external attached langue exception") {}
		AttachedLangException(std::exception_ptr inner_exception) : AttachARuntimeException("Caught unconvertable external attached langue exception", inner_exception) {}
		const char* name() const override {
			return "attached_lang_exception";
		}
	};
	class DeprecatedException : public AttachARuntimeException {
	public:
		DeprecatedException() : AttachARuntimeException("This function deprecated") {}
		DeprecatedException(std::exception_ptr inner_exception) : AttachARuntimeException("This function deprecated", inner_exception) {}
		const char* name() const override {
			return "deprecated_exception";
		}
	};
	class SystemException : public AttachARuntimeException {
	public:
		SystemException(uint32_t error_code) : AttachARuntimeException("System error: " + std::to_string(error_code)) {}
		SystemException(uint32_t error_code, std::exception_ptr inner_exception) : AttachARuntimeException("System error: "+ std::to_string(error_code), inner_exception) {}
		const char* name() const override {
			return "system_exception";
		}
	};
	class AllocationException : public AttachARuntimeException {
	public:
		AllocationException(const std::string& msq) : AttachARuntimeException(msq) {}
		AllocationException(const std::string& msq, std::exception_ptr inner_exception) : AttachARuntimeException(msq, inner_exception) {}
		const char* name() const override {
			return "allocation_exception";
		}
	};

	class InternalException : public AttachARuntimeException {
		list_array<void*> stack_trace;
	public:
		InternalException(const std::string& msq);
		InternalException(const std::string& msq, std::exception_ptr inner_exception);
		const char* name() const override {
			return "internal_exception";
		}
		std::string full_info() const override;
	};
	class RoutineHandleExceptions : public AttachARuntimeException {
		std::exception_ptr second_exception;
	public:
		RoutineHandleExceptions(std::exception_ptr first_exception, std::exception_ptr second_exception) : AttachARuntimeException(first_exception), second_exception(second_exception) {}
		const char* name() const override {
			return "multiple_exceptions";
		}
		std::string full_info() const override;
	};

	class MissingDependencyException : public AttachARuntimeException {
	public:
		MissingDependencyException(const std::string& msq) : AttachARuntimeException(msq) {}
		MissingDependencyException(const std::string& msq, std::exception_ptr inner_exception) : AttachARuntimeException(msq, inner_exception) {}
		const char* name() const override {
			return "missing_dependency_exception";
		}
	};
#pragma endregion




#pragma region CompileTimeExceptions
	class CompileTimeException : public AttachARuntimeException {
	public:
		CompileTimeException(const std::string& msq) : AttachARuntimeException(msq) {}
		CompileTimeException(const std::string& msq, std::exception_ptr inner_exception) : AttachARuntimeException(msq, inner_exception) {}
		const char* name() const override {
			return "CompileTimeException";
		}
	};
	class HotPathException : public CompileTimeException {
	public:
		HotPathException(const std::string& msq) : CompileTimeException(msq) {}
		HotPathException(const std::string& msq, std::exception_ptr inner_exception) : CompileTimeException(msq, inner_exception) {}
		const char* name() const override {
			return "HotPathException";
		}
	};
	class SymbolException : public CompileTimeException {
	public:
		SymbolException(const std::string& msq) : CompileTimeException(msq) {}
		SymbolException(const std::string& msq, std::exception_ptr inner_exception) : CompileTimeException(msq, inner_exception) {}
		const char* name() const override {
			return "SymbolException";
		}
	};
	class InvalidFunction : public CompileTimeException {
	public:
		InvalidFunction(const std::string& msq) : CompileTimeException(msq) {}
		InvalidFunction(const std::string& msq, std::exception_ptr inner_exception) : CompileTimeException(msq, inner_exception) {}
		const char* name() const override {
			return "InvalidFunction";
		}
	};
	class InvalidIL : public InvalidFunction {
	public:
		InvalidIL(const std::string& msq) : InvalidFunction(msq) {}
		InvalidIL(const std::string& msq, std::exception_ptr inner_exception) : InvalidFunction(msq, inner_exception) {}
		const char* name() const override {
			return "InvalidIL";
		}
	};
	class InvalidType : public CompileTimeException {
	public:
		InvalidType(const std::string& msq) : CompileTimeException(msq) {}
		InvalidType(const std::string& msq, std::exception_ptr inner_exception) : CompileTimeException(msq, inner_exception) {}
		const char* name() const override {
			return "InvalidType";
		}
	};
	class BadOperationException : public CompileTimeException {
	public:
		BadOperationException() : CompileTimeException("Bad Operation") {}
		BadOperationException(std::exception_ptr inner_exception) : CompileTimeException("Bad Operation", inner_exception) {}
		const char* name() const override {
			return "BadOperatinException";
		}
	};
#pragma endregion


	//this exception can be throw from attacha runtime
	class AException : public AttachARuntimeException {
		std::string _name;
	public:
		AException(const std::string& ex_name, const std::string& description, void* va = nullptr, size_t ty = 0) : _name(string_help::replace_space(ex_name)), AttachARuntimeException(description), v(va), t(ty) {}
		const char* name() const override {
			return _name.c_str();
		}
		void* v;
		size_t t;
	};
}


#endif /* SRC_RUN_TIME_EXCEPTIONS */
