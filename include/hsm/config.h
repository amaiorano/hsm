// Hierarchical State Machine (HSM)
//
// Copyright (c) 2013 Antonio Maiorano
//
// Distributed under the MIT License (MIT)
// (See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT)

/// \file config.h
/// \brief Types and macros used by library. Typically customized by client to fit project/code base.

#ifndef HSM_CONFIG_H
#define HSM_CONFIG_H

#include <vector>
#include <string>
#include <cassert>
#include <cstdio>
#include <cstring>

#if _DEBUG
#define HSM_DEBUG
#endif

// If set, even if the compiler has RTTI enabled, this will force the usage of the custom HSM RTTI system
#define HSM_FORCE_CUSTOM_RTTI 0

// HSM_CPP_RTTI is set if the compiler has standard C++ RTTI support enabled
#if !HSM_FORCE_CUSTOM_RTTI && ( (defined(_MSC_VER) && defined(_CPPRTTI)) || (defined(__GNUC__) && defined(__GXX_RTTI)) )
#define HSM_CPP_RTTI 1
#endif

#define HSM_STD_VECTOR std::vector
#define HSM_ASSERT assert
#define HSM_ASSERT_MSG(cond, msg) assert((cond) && msg)
#define HSM_NEW new
#define HSM_DELETE delete
#define HSM_DEBUG_NAME_MAXLEN 128

#define HSM_STATE_UPDATE_ARGS void
#define HSM_STATE_UPDATE_ARGS_FORWARD
//#define HSM_STATE_UPDATE_ARGS float deltaTime
//#define HSM_STATE_UPDATE_ARGS_FORWARD deltaTime

typedef bool hsm_bool;
#define hsm_true true
#define hsm_false false

typedef char hsm_char;
#define HSM_TEXT(x) x
#define HSM_PRINTF ::printf
#define STRCMP ::strcmp

#ifdef _MSC_VER
#define SNPRINTF ::_snprintf
#else
#define SNPRINTF ::snprintf
#endif

#define STRNCPY ::strncpy
#define VSNPRINTF ::vsnprintf

//typedef wchar_t hsm_char;
//#define HSM_TEXT(x) L##x
//#define HSM_PRINTF ::wprintf
//#define STRCMP ::wcscmp
//#define SNPRINTF ::swprintf
//#define STRNCPY ::wcsncpy
//#define VSNPRINTF _vsnwprintf

namespace hsm {

// By default, Owner is stored as a void* but it may be useful to override this to a client-specific
// type (or interface).
typedef void Owner;

} // namespace hsm

#endif // HSM_CONFIG_H
