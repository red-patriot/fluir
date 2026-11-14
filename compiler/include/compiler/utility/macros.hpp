#ifndef FLUIR_COMPILER_UTILITY_MACROS_HPP
#define FLUIR_COMPILER_UTILITY_MACROS_HPP

#define FLUIR_CONCAT_IMPL(a, b) a##b
#define FLUIR_CONCAT(a, b) FLUIR_CONCAT_IMPL(a, b)
#define FLUIR_ANONYMOUS_VARIABLE(BaseName) FLUIR_CONCAT(BaseName, __LINE__)

#ifdef _MSC_VER
// Workaround MSVC non-compliance...
#define FLUIR_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define FLUIR_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#endif
