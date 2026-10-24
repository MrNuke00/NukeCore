#pragma once

#include <cstdint>
#include <cstddef>
#include <climits>
#include <type_traits>


// MSVC historically reports __cplusplus as 199711L unless /Zc:__cplusplus is
// passed. _MSVC_LANG is always accurate.
#if defined(_MSVC_LANG)
#  define CXX_STANDARD _MSVC_LANG
#elif defined(__cplusplus)
#  define CXX_STANDARD __cplusplus
#endif

#define CXX_STANDARD_11 (201103L)
#define CXX_STANDARD_14 (201402L)
#define CXX_STANDARD_17 (201703L)
#define CXX_STANDARD_20 (202002L)
#define CXX_STANDARD_23 (202302L)

// This header relies on C++-only facilities (namespaces, `using`,
// `static_assert`, `std::byte`, type traits). Refuse to be included from C.
#if !defined(CXX_STANDARD) || (CXX_STANDARD < CXX_STANDARD_17)
#  error "PlatformCoreMacro.h requires a C++ compiler (C++17 or later)."
#endif

#if defined(NDEBUG)
#	define BUILD_DEBUG		(0)
#	define BUILD_RELEASE	(1)
#else
#	define BUILD_DEBUG		(1)
#	define BUILD_RELEASE	(0)
#endif

// =============================================================================
// 1. Platform detection
// =============================================================================
//
// Order matters. Emscripten defines __linux__ on some configurations, so it
// must be tested before the generic Linux branch. Windows is checked before
// everything else because _WIN32 is unambiguous.
//
// Apple: __APPLE__ is true for every Darwin target (macOS, iOS, tvOS,
// watchOS, visionOS). TargetConditionals.h refines which one.
#if defined(__EMSCRIPTEN__)
#  define PLATFORM_EMSCRIPTEN 1
#elif defined(_WIN32) || defined(_WIN64)
#  define PLATFORM_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
#  include <TargetConditionals.h>
#  if TARGET_OS_OSX
#    define PLATFORM_MACOS 1
#  elif TARGET_OS_TV
#    define PLATFORM_TVOS 1
#  elif TARGET_OS_WATCH
#    define PLATFORM_WATCHOS 1
#  elif TARGET_OS_VISION
#    define PLATFORM_VISIONOS 1
#  elif TARGET_OS_IPHONE
#    define PLATFORM_IOS 1
#  else
#    error "PlatformCoreMacro.h: unknown Apple platform."
#  endif
#elif defined(__linux__)
#  if defined(__ANDROID__)
#    define PLATFORM_ANDROID 1
#  else
#    define PLATFORM_LINUX 1
#  endif
#else
#  error "PlatformCoreMacro.h: Unsupported or undetected target operating system."
#endif

// Zero-init unset platform macros so they can be used in #if without -Wundef.
#ifndef PLATFORM_WINDOWS
#  define PLATFORM_WINDOWS 0
#endif
#ifndef PLATFORM_MACOS
#  define PLATFORM_MACOS 0
#endif
#ifndef PLATFORM_IOS
#  define PLATFORM_IOS 0
#endif
#ifndef PLATFORM_TVOS
#  define PLATFORM_TVOS 0
#endif
#ifndef PLATFORM_WATCHOS
#  define PLATFORM_WATCHOS 0
#endif
#ifndef PLATFORM_VISIONOS
#  define PLATFORM_VISIONOS 0
#endif
#ifndef PLATFORM_LINUX
#  define PLATFORM_LINUX 0
#endif
#ifndef PLATFORM_ANDROID
#  define PLATFORM_ANDROID 0
#endif
#ifndef PLATFORM_EMSCRIPTEN
#  define PLATFORM_EMSCRIPTEN 0
#endif

// Sanity check — the outer #else path plus the Apple refinement mean that at
// least one platform flag must be set here.
#if !PLATFORM_WINDOWS && !PLATFORM_MACOS && !PLATFORM_IOS && \
	!PLATFORM_TVOS && !PLATFORM_WATCHOS && !PLATFORM_VISIONOS && \
	!PLATFORM_LINUX && !PLATFORM_ANDROID && !PLATFORM_EMSCRIPTEN
#  error "PlatformCoreMacro.h: Unsupported or undetected target operating system."
#endif

#if PLATFORM_WINDOWS || PLATFORM_MACOS || PLATFORM_LINUX
#  define PLATFORM_DESKTOP 1
#else
#  define PLATFORM_DESKTOP 0
#endif

#if PLATFORM_IOS || PLATFORM_ANDROID || PLATFORM_TVOS || \
	PLATFORM_WATCHOS || PLATFORM_VISIONOS
#  define PLATFORM_MOBILE 1
#else
#  define PLATFORM_MOBILE 0
#endif

#if PLATFORM_WINDOWS
#  define PLATFORM_POSIX 0
#elif PLATFORM_EMSCRIPTEN
#  define PLATFORM_POSIX 0
#else
#  define PLATFORM_POSIX 1
#endif


// =============================================================================
// 2. Compiler detection
// =============================================================================

#if defined(__clang__)
#  define COMPILER_CLANG 1
#elif defined(_MSC_VER)
#  define COMPILER_MSVC 1
#elif defined(__GNUC__)
#  define COMPILER_GCC 1
#else
#  error "PlatformCoreMacro.h: Unsupported or undetected compiler."
#endif

#ifndef COMPILER_CLANG
#  define COMPILER_CLANG 0
#endif
#ifndef COMPILER_GCC
#  define COMPILER_GCC 0
#endif
#ifndef COMPILER_MSVC
#  define COMPILER_MSVC 0
#endif


// =============================================================================
// 3. Language standard
// =============================================================================


#if CXX_STANDARD >= CXX_STANDARD_17
#  define HAS_CPP17 1
#else
#  define HAS_CPP17 0
#endif
#if CXX_STANDARD >= CXX_STANDARD_20
#  define HAS_CPP20 1
#else
#  define HAS_CPP20 0
#endif
#if CXX_STANDARD >= CXX_STANDARD_23
#  define HAS_CPP23 1
#else
#  define HAS_CPP23 0
#endif


// =============================================================================
// 4. Feature-test helpers
// =============================================================================

#if defined(__has_include)
#  define HAS_INCLUDE(x) __has_include(x)
#else
#  define HAS_INCLUDE(x) 0
#endif

#if defined(__has_feature)
#  define HAS_FEATURE(x) __has_feature(x)
#else
#  define HAS_FEATURE(x) 0
#endif

#if defined(__has_builtin)
#  define HAS_BUILTIN(x) __has_builtin(x)
#else
#  define HAS_BUILTIN(x) 0
#endif

#if defined(__has_cpp_attribute)
#  define HAS_CPP_ATTR(x) __has_cpp_attribute(x)
#else
#  define HAS_CPP_ATTR(x) 0
#endif


// =============================================================================
// 5. Architecture
// =============================================================================

#if defined(_M_X64) || defined(__x86_64__)
#  define ARCH_X86_64 1
#elif defined(_M_IX86) || defined(__i386__)
#  define ARCH_X86_32 1
#elif defined(_M_ARM64) || defined(_M_ARM64EC) || defined(__aarch64__)
#  define ARCH_ARM64 1
#elif defined(_M_ARM) || defined(__arm__)
#  define ARCH_ARM32 1
#elif defined(__wasm__) || defined(__EMSCRIPTEN__)
#  define ARCH_WASM 1
#elif defined(__riscv) && (__riscv_xlen == 64)
#  define ARCH_RISCV64 1
#elif defined(__riscv) && (__riscv_xlen == 32)
#  define ARCH_RISCV32 1
#else
#  error "PlatformCoreMacro.h: Unsupported or undetected target architecture."
#endif

#ifndef ARCH_X86_64
#  define ARCH_X86_64 0
#endif
#ifndef ARCH_X86_32
#  define ARCH_X86_32 0
#endif
#ifndef ARCH_ARM64
#  define ARCH_ARM64 0
#endif
#ifndef ARCH_ARM32
#  define ARCH_ARM32 0
#endif
#ifndef ARCH_WASM
#  define ARCH_WASM 0
#endif
#ifndef ARCH_RISCV64
#  define ARCH_RISCV64 0
#endif
#ifndef ARCH_RISCV32
#  define ARCH_RISCV32 0
#endif

#if ARCH_X86_64 || ARCH_ARM64 || ARCH_RISCV64 || \
	(defined(__LP64__) || defined(_LP64) || defined(_WIN64))
#  define ARCH_64BIT 1
#else
#  define ARCH_64BIT 0
#endif

#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#  define ENDIAN_BIG 1
#  define ENDIAN_LITTLE 0
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#  define ENDIAN_BIG 0
#  define ENDIAN_LITTLE 1
#elif COMPILER_MSVC
#  define ENDIAN_BIG 0
#  define ENDIAN_LITTLE 1
#elif ARCH_X86_64 || ARCH_X86_32 || ARCH_ARM64 || ARCH_ARM32 || ARCH_WASM || \
	  ARCH_RISCV64 || ARCH_RISCV32
#  define ENDIAN_BIG 0
#  define ENDIAN_LITTLE 1
#else
#  error "PlatformCoreMacro.h: cannot determine endianness."
#endif


// =============================================================================
// 6. SIMD detection
// =============================================================================

#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#  define SIMD_SSE2 1
#else
#  define SIMD_SSE2 0
#endif
#if defined(__SSE3__)
#  define SIMD_SSE3 1
#else
#  define SIMD_SSE3 0
#endif
#if defined(__SSSE3__)
#  define SIMD_SSSE3 1
#else
#  define SIMD_SSSE3 0
#endif
#if defined(__SSE4_1__)
#  define SIMD_SSE41 1
#else
#  define SIMD_SSE41 0
#endif
#if defined(__SSE4_2__)
#  define SIMD_SSE42 1
#else
#  define SIMD_SSE42 0
#endif
#if defined(__AVX__)
#  define SIMD_AVX 1
#else
#  define SIMD_AVX 0
#endif
#if defined(__AVX2__)
#  define SIMD_AVX2 1
#else
#  define SIMD_AVX2 0
#endif
#if defined(__AVX512F__)
#  define SIMD_AVX512 1
#else
#  define SIMD_AVX512 0
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__) || \
	defined(_M_ARM) || defined(_M_ARM64) || defined(_M_ARM64EC)
#  define SIMD_NEON 1
#else
#  define SIMD_NEON 0
#endif
#if defined(__ARM_FEATURE_SVE)
#  define SIMD_SVE 1
#else
#  define SIMD_SVE 0
#endif

#if defined(__wasm_simd128__)
#  define SIMD_WASM128 1
#else
#  define SIMD_WASM128 0
#endif


// =============================================================================
// 7. Symbol visibility / export
// =============================================================================

#if PLATFORM_WINDOWS
#  define EXPORT_DLL __declspec(dllexport)
#  define IMPORT_DLL __declspec(dllimport)
#  define LOCAL_DLL
#else
#  define EXPORT_DLL __attribute__((visibility("default")))
#  define IMPORT_DLL
#  define LOCAL_DLL  __attribute__((visibility("hidden")))
#endif

// API_EXPORT / API_LOCAL are the user-facing decoration pair.
//   BUILD_SHARED  — the project is built as a dynamic library.
//   BUILD_EXPORTS — we are inside the library (vs. consuming it).
#if defined(BUILD_SHARED)
#  if PLATFORM_WINDOWS
#    if defined(BUILD_EXPORTS)
#      define API_EXPORT __declspec(dllexport)
#    else
#      define API_EXPORT __declspec(dllimport)
#    endif
#    define API_LOCAL
#  else
#    if defined(BUILD_EXPORTS)
#      define API_EXPORT __attribute__((visibility("default")))
#    else
#      define API_EXPORT
#    endif
#    define API_LOCAL __attribute__((visibility("hidden")))
#  endif
#else
#  define API_EXPORT
#  define API_LOCAL
#endif


// =============================================================================
// 8. Inlining
// =============================================================================

#if COMPILER_MSVC
#  define FORCE_INLINE __forceinline
#  define NO_INLINE    __declspec(noinline)
#elif COMPILER_GCC || COMPILER_CLANG
#  define FORCE_INLINE inline __attribute__((always_inline))
#  define NO_INLINE    __attribute__((noinline))
#else
#  define FORCE_INLINE inline
#  define NO_INLINE
#endif


// =============================================================================
// 9. Alignment / packing
// =============================================================================

#define ALIGN_OF(x) alignof(x)
#define ALIGN_AS(x) alignas(x)

#if COMPILER_MSVC
#  define PACK_STRUCT     __pragma(pack(push, 1))
#  define PACK_STRUCT_END __pragma(pack(pop))
#elif COMPILER_GCC || COMPILER_CLANG
#  define PACK_STRUCT     _Pragma("pack(push, 1)")
#  define PACK_STRUCT_END _Pragma("pack(pop)")
#else
#  define PACK_STRUCT
#  define PACK_STRUCT_END
#endif


// =============================================================================
// 10. Debug break
// =============================================================================
//
// Order of preference:
//   MSVC    : __debugbreak()      — works on x86/x64/ARM/ARM64.
//   Clang   : __builtin_debugtrap() when available — intentionally a
//             debugger break, unlike __builtin_trap() which is UB.
//   GCC x86 : int3                — the canonical breakpoint instruction.
//   Others  : __builtin_trap()    — last-resort (SIGILL / ud2 / brk).
#if COMPILER_MSVC
#  include <intrin.h>
#  define DEBUG_BREAK() __debugbreak()
#elif COMPILER_CLANG
#  if HAS_BUILTIN(__builtin_debugtrap)
#    define DEBUG_BREAK() __builtin_debugtrap()
#  elif HAS_BUILTIN(__builtin_trap)
#    define DEBUG_BREAK() __builtin_trap()
#  else
#    define DEBUG_BREAK() ((void)0)
#  endif
#elif COMPILER_GCC
#  if ARCH_X86_64 || ARCH_X86_32
#    define DEBUG_BREAK() __asm__ __volatile__("int3")
#  else
#    define DEBUG_BREAK() __builtin_trap()
#  endif
#else
#  define DEBUG_BREAK() ((void)0)
#endif


// =============================================================================
// 11. Branch prediction
// =============================================================================

#if COMPILER_GCC || COMPILER_CLANG
#  define LIKELY(x)   __builtin_expect(!!(x), 1)
#  define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#  define LIKELY(x)   (x)
#  define UNLIKELY(x) (x)
#endif


// =============================================================================
// 12. Assumptions / unreachable
// =============================================================================

#if COMPILER_MSVC
#  define UNREACHABLE() __assume(0)
#  define ASSUME(x)     __assume(x)
#elif COMPILER_CLANG
#  if HAS_BUILTIN(__builtin_assume)
#    define ASSUME(x) __builtin_assume(x)
#  else
#    define ASSUME(x) do { if (!(x)) __builtin_unreachable(); } while (0)
#  endif
#  define UNREACHABLE() __builtin_unreachable()
#elif COMPILER_GCC
#  define UNREACHABLE() __builtin_unreachable()
#  define ASSUME(x)     do { if (!(x)) __builtin_unreachable(); } while (0)
#else
#  define UNREACHABLE() ((void)0)
#  define ASSUME(x)     ((void)0)
#endif

#define UNIMPLEMENTED() do { DEBUG_BREAK(); UNREACHABLE(); } while (0)


// =============================================================================
// 13. Miscellaneous
// =============================================================================

#define UNUSED(x) ((void)(x))


// =============================================================================
// 14. Token-level helpers
// =============================================================================

#define STRINGIZE_IMPL(x) #x
#define STRINGIZE(x)      STRINGIZE_IMPL(x)

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b)      CONCAT_IMPL(a, b)


// =============================================================================
// 15. Pragma emission — compile-time diagnostics
// =============================================================================
//
// PRAGMA_MESSAGE prints an informational diagnostic at the call site.
// COMPILE_NOTE/WARN/ERROR wrap it with severity prefixes.
//
// IMPORTANT: these fire at COMPILE TIME and are NOT conditional. Never place
// them inside a runtime `if`. Use STATIC_ASSERT for conditional compile-time
// checks.
#if COMPILER_MSVC
#  define DO_PRAGMA_IMPL(x)		__pragma(x)
#elif COMPILER_GCC || COMPILER_CLANG
#  define DO_PRAGMA_IMPL(x)		_Pragma(#x)
#else
#  define DO_PRAGMA_IMPL(x)
#endif

#if COMPILER_MSVC || COMPILER_GCC || COMPILER_CLANG
#  define PRAGMA_MESSAGE(msg)	DO_PRAGMA_IMPL(message(__FILE__ "(" STRINGIZE(__LINE__) "): " msg))
#  define PRAGMA_ERROR(msg)		DO_PRAGMA_IMPL(message(__FILE__ "(" STRINGIZE(__LINE__) "): error: " msg))
#else
#  define PRAGMA_MESSAGE(msg)
#  define PRAGMA_ERROR(msg)
#endif



// =============================================================================
// 16. Container/type helpers
// =============================================================================

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))
#define OFFSET_OF(type, member) offsetof(type, member)

#define CONTAINER_OF(ptr, type, member) \
	((type*)((char*)(ptr) - OFFSET_OF(type, member)))

#define TYPE_OF(e)      decltype(e)
#define SAME_TYPE(a, b) std::is_same< \
							typename std::remove_cv<a>::type, \
							typename std::remove_cv<b>::type>::value


// =============================================================================
// 17. Attributes
// =============================================================================

#if COMPILER_MSVC
#  define DEPRECATED(msg) __declspec(deprecated(msg))
#elif COMPILER_GCC || COMPILER_CLANG
#  define DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#  define DEPRECATED(msg)
#endif


// =============================================================================
// 18. Calling conventions
// =============================================================================

#if COMPILER_MSVC
#  define CDECL    __cdecl
#  define STDCALL  __stdcall
#  define FASTCALL __fastcall
#else
#  define CDECL
#  define STDCALL
#  define FASTCALL
#endif

#if ARCH_X86_32 && COMPILER_MSVC
#  define THISCALL __thiscall
#elif ARCH_X86_32 && PLATFORM_WINDOWS && (COMPILER_GCC || COMPILER_CLANG)
#  define THISCALL __attribute__((thiscall))
#else
#  define THISCALL
#endif

#if defined(__cplusplus)
#  define EXTERN_C extern "C"
#else
#  define EXTERN_C
#endif


// =============================================================================
// 19. Standard-version shorthands
// =============================================================================
//
// These degrade to the closest compiling equivalent on older standards. When
// the semantic difference matters (e.g. consteval vs. constexpr), use the
// capability queries in section 20 instead.
#if CXX_STANDARD >= CXX_STANDARD_20
#  define CONSTEXPR20 constexpr
#  define CONSTEVAL   consteval
#  define CONSTINIT   constinit
#elif CXX_STANDARD >= CXX_STANDARD_17
#  define CONSTEXPR20 constexpr
#  define CONSTEVAL   constexpr
#  define CONSTINIT   constexpr
#elif CXX_STANDARD >= CXX_STANDARD_14
#  define CONSTEXPR20 inline
#  define CONSTEVAL   inline
#  define CONSTINIT
#else
#  define CONSTEXPR20 constexpr
#  define CONSTEVAL   inline
#  define CONSTINIT
#endif

#if CXX_STANDARD >= CXX_STANDARD_17
#  define IF_CONSTEXPR if constexpr
#else
#  define IF_CONSTEXPR if
#endif

#if CXX_STANDARD >= CXX_STANDARD_17
#  define NODISCARD     [[nodiscard]]
#  define MAYBE_UNUSED  [[maybe_unused]]
#  define FALLTHROUGH   [[fallthrough]]
#  define NOEXCEPT      noexcept
#else
#  define NODISCARD
#  define MAYBE_UNUSED
#  define FALLTHROUGH
#  define NOEXCEPT      throw()
#endif

#if CXX_STANDARD >= CXX_STANDARD_20
#  define LIKELY_ATTR   [[likely]]
#  define UNLIKELY_ATTR [[unlikely]]
#else
#  define LIKELY_ATTR
#  define UNLIKELY_ATTR
#endif


// =============================================================================
// 20. Feature queries (per-feature, not per-standard)
// =============================================================================

#if defined(__cpp_consteval)
#  define HAS_CONSTEVAL 1
#else
#  define HAS_CONSTEVAL 0
#endif

#if defined(__cpp_constinit)
#  define HAS_CONSTINIT 1
#else
#  define HAS_CONSTINIT 0
#endif

#if defined(__cpp_if_constexpr)
#  define HAS_IF_CONSTEXPR 1
#else
#  define HAS_IF_CONSTEXPR 0
#endif

#if defined(__cpp_constexpr) && __cpp_constexpr >= 201907L
#  define HAS_RELAXED_CONSTEXPR 1
#else
#  define HAS_RELAXED_CONSTEXPR 0
#endif

#if defined(__cpp_concepts)
#  define HAS_CONCEPTS 1
#else
#  define HAS_CONCEPTS 0
#endif

#if defined(__cpp_coroutines) && __cpp_coroutines >= CXX_STANDARD_17
#  define HAS_COROUTINES 1
#else
#  define HAS_COROUTINES 0
#endif

#if CXX_STANDARD >= CXX_STANDARD_17
#  define HAS_STD_ATTRIBUTES 1
#else
#  define HAS_STD_ATTRIBUTES 0
#endif

#if HAS_CPP_ATTR(likely) && HAS_CPP_ATTR(unlikely)
#  define HAS_LIKELY_ATTR 1
#else
#  define HAS_LIKELY_ATTR 0
#endif

#if defined(__cpp_if_consteval)
#  define HAS_IF_CONSTEVAL 1
#else
#  define HAS_IF_CONSTEVAL 0
#endif


// =============================================================================
// 21. Fixed-width aliases
// =============================================================================

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

using float32 = float;
using float64 = double;

using usize = std::size_t;
using isize = std::ptrdiff_t;
using uptr = std::uintptr_t;
using iptr = std::intptr_t;

#if HAS_CPP17
using byte = unsigned char;
#else
enum class byte : unsigned char {};
#endif

static_assert(sizeof(float32) == 4, "f32 must be exactly 32 bits.");
static_assert(sizeof(float64) == 8, "f64 must be exactly 64 bits.");
static_assert(sizeof(int8)  == 1 && sizeof(uint8)  == 1, "8-bit type size mismatch.");
static_assert(sizeof(int16) == 2 && sizeof(uint16) == 2, "16-bit type size mismatch.");
static_assert(sizeof(int32) == 4 && sizeof(uint32) == 4, "32-bit type size mismatch.");
static_assert(sizeof(int64) == 8 && sizeof(uint64) == 8, "64-bit type size mismatch.");

#if defined(__cpp_char8_t)
using char8 = char8_t;
#else
using char8 = char;
#endif

using char16 = char16_t;
using char32 = char32_t;
using wchar = wchar_t;

#define WIDE_STR_IMPL(TEXT)  L##TEXT
#define WIDE_STR(TEXT)       WIDE_STR_IMPL(TEXT)

#if defined(__cpp_char8_t)
#  define UTF8_STR_IMPL(TEXT)  u8##TEXT
#else
#  define UTF8_STR_IMPL(TEXT)  TEXT
#endif
#define UTF8_STR(TEXT)       UTF8_STR_IMPL(TEXT)

#define UTF16_STR_IMPL(TEXT) u##TEXT
#define UTF16_STR(TEXT)      UTF16_STR_IMPL(TEXT)

#define UTF32_STR_IMPL(TEXT) U##TEXT
#define UTF32_STR(TEXT)      UTF32_STR_IMPL(TEXT)


// =============================================================================
// 22. Build configuration
// =============================================================================

#if defined(NDEBUG)
#  define BUILD_DEBUG   0
#  define BUILD_RELEASE 1
#else
#  define BUILD_DEBUG   1
#  define BUILD_RELEASE 0
#endif

// =============================================================================
// 23. Compile-time assertion
// =============================================================================

#if defined(__cpp_static_assert) && __cpp_static_assert >= 200410L
#  define STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif COMPILER_MSVC
   // Fallback for very old MSVC without /std:c++11 or later.
#  define STATIC_ASSERT(cond, msg) \
	   typedef char CONCAT(static_assert_, __LINE__)[(cond) ? 1 : -1]
#else
#  define STATIC_ASSERT(cond, msg) \
	   typedef char CONCAT(static_assert_, __LINE__)[(cond) ? 1 : -1]
#endif

