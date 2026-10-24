#pragma once
#include <Core/Core.h>

/*
check / checkSlow (and checkf, checkCode, checkNoEntry, checkNoReentry, checkNoRecursion, unimplemented): Halts execution on failure and are typically stripped out in shipping builds (USE_CHECKS_IN_SHIPPING=0).

verify / verifySlow (and verifyf): Similar to check, but the underlying expression is always evaluated regardless of build configuration, making it essential when the expression contains side effects (like assignments or function calls that must run).

ensure / ensureAlways (and ensureMsgf, ensureAlwaysMsgf): Non-fatal checks that trigger the crash reporter and log the failure on the first occurrence (or every time with "Always") while allowing execution to continue.
*/





