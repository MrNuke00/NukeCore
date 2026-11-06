// NukeCore\include\Core\Assert.h
#pragma once
#include <Core/Core.h>
#include <assert.h>

// Cross-platform debugger break intrinsic

#if defined(NDEBUG)
#define Assert(Expr)          ((void)0)
#define AssertMsg(Expr, Msg)  ((void)0)

// In release/shipping, Ensure still evaluates the expression so your logic runs, 
// but strips out the assertion check entirely for performance.
#define Ensure(Expr)          (Expr)
#define EnsureMsg(Expr, Msg)  (Expr)

#else
#define AssertMsg(Expr) \
        do { \
            if (!(Expr)) { \
                DEBUG_BREAK(); \
            } \
        } while(0)

#define Assert(Expr, Msg) AssertMsg(Expr, #Expr)

// Ensure stays active in Debug AND Release (or whatever config you choose),
// evaluates the expression, triggers a breakpoint if false, and returns the result.

#define Ensure(Expr, Msg) \
        [&]() { \
            auto&& _eval_result = (Expr); \
            if (!static_cast<bool>(_eval_result)) { \
                DEBUG_BREAK(); \
            } \
            return _eval_result; \
        }()

#define EnsureMsg(Expr) EnsureMsg(Expr, #Expr)

#endif