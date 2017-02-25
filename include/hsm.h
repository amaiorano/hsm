// Hierarchical State Machine (HSM)
//
// Copyright (c) 2015 Antonio Maiorano
//
// Distributed under the MIT License (MIT)
// (See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT)

/// \file hsm.h
/// \brief Single header for HSM library

// Required includes
#include <cstdarg>
#include <functional>
#include <memory>

#pragma once
#ifndef __HSM_H__
#define __HSM_H__

#ifdef _MSC_VER
#pragma region "Config"
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
// Config
///////////////////////////////////////////////////////////////////////////////////////////////////

// Includes required for macros defined below. You can remove/replace them if you modify the macros.
#include <vector>   // for HSM_STD_VECTOR
#include <map>      // for HSM_STD_MAP
#include <cassert>  // for HSM_ASSERT
#include <cstdio>   // for SNPRINTF
#include <cstring>  // for STRNCPY

// Define HSM_DEBUG to 0 or 1 explicitly, otherwise it will be 1 if _DEBUG is defined
#if !defined(HSM_DEBUG)
#ifdef _DEBUG // MSVC defines this for Debug configurations
#define HSM_DEBUG 1
#else
#define HSM_DEBUG 0
#endif
#endif

// If set, even if the compiler has RTTI enabled, this will force the usage of the custom HSM RTTI system
#define HSM_FORCE_CUSTOM_RTTI 0

// HSM_CPP_RTTI is set if the compiler has standard C++ RTTI support enabled
#if !HSM_FORCE_CUSTOM_RTTI && ( (defined(_MSC_VER) && defined(_CPPRTTI)) || (defined(__GNUC__) && defined(__GXX_RTTI)) )
#define HSM_CPP_RTTI 1
#endif

#define HSM_STD_VECTOR std::vector
#define HSM_STD_MAP std::map
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
#define SNPRINTF ::_snprintf_s
#else
#define SNPRINTF ::snprintf
#endif

#ifdef _MSC_VER
#define STRNCPY ::strncpy_s
#else
#define STRNCPY ::strncpy
#endif

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

#ifdef _MSC_VER
#pragma endregion "Config"
#endif

#ifdef _MSC_VER
#pragma region "RTTI"
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
// RTTI
///////////////////////////////////////////////////////////////////////////////////////////////////

#if HSM_CPP_RTTI

#include <typeinfo>

namespace hsm {

// We use standard C++ RTTI

// We need a copyable wrapper around std::type_info since std::type_info is non-copyable
struct StateTypeIdStorage
{
	StateTypeIdStorage() : m_typeInfo(0) { }
	StateTypeIdStorage(const std::type_info& typeInfo) { m_typeInfo = &typeInfo; }
	hsm_bool operator==(const StateTypeIdStorage& rhs) const
	{
		HSM_ASSERT_MSG(m_typeInfo != 0, "m_typeInfo was not properly initialized");
		return *m_typeInfo == *rhs.m_typeInfo;
	}
	const std::type_info* m_typeInfo;
};

typedef const StateTypeIdStorage& StateTypeId;

template <typename StateType>
StateTypeId GetStateType()
{
	static StateTypeIdStorage stateTypeId(typeid(StateType));
	return stateTypeId;
}

} // namespace hsm

// DEFINE_HSM_STATE is NOT necessary; however, we define it here to nothing to make it easier to
// test switching between compiler RTTI enabled or disabled.
#define DEFINE_HSM_STATE(__StateName__)


#else // !HSM_CPP_RTTI

namespace hsm {

// Standard C++ RTTI is not available, so we roll our own custom RTTI. All states are required to use the
// DEFINE_HSM_STATE macro, which makes use of the input name of the state as the unique identifier. String
// compares are used to determine equality.

// Like std::type_info, we need to be able test equality and get a unique name
struct StateTypeIdStorage
{
	StateTypeIdStorage(const hsm_char* aStateName = 0) : mStateName(aStateName) {}
	hsm_bool operator==(const StateTypeIdStorage& rhs) const
	{
		HSM_ASSERT_MSG(mStateName != 0, "StateTypeId was not properly initialized");
		return STRCMP(mStateName, rhs.mStateName) == 0;
	}
	const hsm_char* mStateName;
};

typedef const StateTypeIdStorage& StateTypeId;

template <typename StateType>
StateTypeId GetStateType()
{
	return StateType::GetStaticStateType();
}

} // namespace hsm

// Must use this macro in every State to add RTTI support.
#define DEFINE_HSM_STATE(__StateName__) \
	static hsm::StateTypeId GetStaticStateType() { static hsm::StateTypeIdStorage sStateTypeId(HSM_TEXT(#__StateName__)); return sStateTypeId; } \
	virtual hsm::StateTypeId DoGetStateType() const { return GetStaticStateType(); } \
	virtual const hsm_char* DoGetStateDebugName() const { return HSM_TEXT(#__StateName__); }

#endif // !HSM_CPP_RTTI

#ifdef _MSC_VER
#pragma endregion "RTTI"
#endif

#ifdef _MSC_VER
#pragma region "Transition"
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
// Transition
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace hsm {

struct State;
struct StateFactory;

// Returns the one StateFactory instance for the input state. Note that this type can be used to effectively store
// a state in a variable at runtime, which can subsequently be passed to a Transition function.
template <typename TargetState>
const StateFactory& GetStateFactory();

// State creation interface
struct StateFactory
{
	virtual StateTypeId GetStateType() const = 0;
	virtual State* AllocateState() const = 0;
};

inline bool operator==(const StateFactory& lhs, const StateFactory& rhs) { return lhs.GetStateType() == rhs.GetStateType(); }
inline bool operator!=(const StateFactory& lhs, const StateFactory& rhs) { return !(lhs == rhs); }

// ConcreteStateFactory is the actual state creator; these are allocated statically in the transition
// functions (below) and stored within Transition instances.
template <typename TargetState>
struct ConcreteStateFactory : StateFactory
{
	virtual StateTypeId GetStateType() const
	{
		return hsm::GetStateType<TargetState>();
	}

	virtual State* AllocateState() const
	{
		return HSM_NEW TargetState();
	}

private:
	// Only GetStateFactory can create this type
	friend const StateFactory& GetStateFactory<TargetState>();
	ConcreteStateFactory() {}
};

template <typename TargetState>
const StateFactory& GetStateFactory()
{
	static_assert(std::is_convertible<TargetState, State>::value, "TargetState must derive from hsm::State");
	static ConcreteStateFactory<TargetState> instance;
	return instance;
}

// Small wrapper used to carry the SourceState along with the StateFactory for a state override.
// This allows us to provide an overload of transition functions that accept a state override with args.
template <typename SourceState>
struct StateOverride
{
	explicit StateOverride(const StateFactory& stateFactory) : mStateFactory(stateFactory) {}
	operator const StateFactory& () const { return mStateFactory; }
	const StateFactory& mStateFactory;
};

typedef std::function<void (State*)> OnEnterArgsFunc;

// MSVC 14 (VS 2015) doesn't handle generating lambdas that capture C-style arrays ("const T(&)[n]")
// by value, emitting error "array initialization requires a brace-enclosed initializer list".
// The most common case that this affects are immediate strings, so we work around this issue by
// detecting this case and forwarding them as "const char*", which is safe since immediate strings
// have global lifetime.
#ifdef _MSC_VER
#define DECAY_IMMEDIATE_STRINGS_WHEN_FORWARDING
#endif

namespace detail
{
	// Generates a lambda that will invoke TargetState::OnEnter with matching args.
	// We get a compiler-time error if a matching OnEnter is not found.
	template <typename TargetState, typename... Args>
	OnEnterArgsFunc DoGenerateOnEnterArgsFunc(Args&&... args)
	{
		static_assert(std::is_convertible<TargetState, State>::value, "TargetState must derive from hsm::State");

		// Purposely capture args by copy rather than by reference in case args are
		// created on the stack. Use std::ref() to wrap args that do not need to be copied.
		return [args...](State* state) { (static_cast<TargetState*>(state))->OnEnter(std::move(args)...); };
	}

	// Base case: do nothing
	template <typename T>
	T&& DecayIfImmediateString(T&& arg1)
	{
		return std::forward<T>(arg1);
	}

	// If immediate string, decay to const char*
	template <size_t _Nx>
	const char* DecayIfImmediateString(const char(&arr)[_Nx])
	{
		return static_cast<const char*>(arr);
	}

	template <typename TargetState, typename... Args>
	OnEnterArgsFunc GenerateOnEnterArgsFunc(Args&&... args)
	{
#ifdef DECAY_IMMEDIATE_STRINGS_WHEN_FORWARDING
		return DoGenerateOnEnterArgsFunc<TargetState>(DecayIfImmediateString(args)...);
#else
		return DoGenerateOnEnterArgsFunc<TargetState>(std::forward<Args>(args)...);
#endif // DECAY_IMMEDIATE_STRINGS_WHEN_FORWARDING
	}
} // namespace detail


// Transition objects are created via the free-standing transition functions below, and typically returned by
// GetTransition. They can also be stored as data members, passed around, and returned later. They are meant
// to be copyable and lightweight.
struct Transition
{
	enum Type { Sibling, Inner, InnerEntry, No };

	// Default is no transition
	Transition()
		: mTransitionType(Transition::No)
		, mStateFactory(0)
	{
	}

	// Transition without state args
	Transition(Transition::Type transitionType, const StateFactory& stateFactory)
		: mTransitionType(transitionType)
		, mStateFactory(&stateFactory)
	{
	}

	// Transition with state args
	Transition(Transition::Type transitionType, const StateFactory& stateFactory, OnEnterArgsFunc&& onEnterArgsFunc)
		: mTransitionType(transitionType)
		, mStateFactory(&stateFactory)
		, mOnEnterArgsFunc(std::move(onEnterArgsFunc))
	{
	}

	Transition::Type GetTransitionType() const { return mTransitionType; }
	StateTypeId GetTargetStateType() const { HSM_ASSERT(mStateFactory != 0); return mStateFactory->GetStateType(); }
	const StateFactory& GetStateFactory() const { HSM_ASSERT(mStateFactory != 0); return *mStateFactory; }
	const OnEnterArgsFunc& GetOnEnterArgsFunc() const { return mOnEnterArgsFunc; }

	hsm_bool IsSibling() const { return mTransitionType == Sibling; }
	hsm_bool IsInner() const { return mTransitionType == Inner; }
	hsm_bool IsInnerEntry() const { return mTransitionType == InnerEntry; }
	hsm_bool IsNo() const { return mTransitionType == No; }

private:
	Transition::Type mTransitionType;
	const StateFactory* mStateFactory; // Bald pointer is safe for shallow copying because StateFactory instances are always statically allocated
	OnEnterArgsFunc mOnEnterArgsFunc; // Optional: set if transition specifies arguments
};


// Transition generators - use these to return from State::GetTransition()

// SiblingTransition

inline Transition SiblingTransition(const StateFactory& stateFactory)
{
	return Transition(Transition::Sibling, stateFactory);
}

template <typename TargetState>
Transition SiblingTransition()
{
	return Transition(Transition::Sibling, GetStateFactory<TargetState>());
}

template <typename TargetState, typename... Args>
Transition SiblingTransition(Args&&... args)
{
	return Transition(Transition::Sibling, GetStateFactory<TargetState>(), detail::GenerateOnEnterArgsFunc<TargetState>(std::forward<Args>(args)...));
}

template <typename TargetState, typename T1, typename... Args>
Transition SiblingTransition(const StateOverride<TargetState>& stateOverride, T1&& arg1, Args&&... args)
{
	return Transition(Transition::Sibling, stateOverride.mStateFactory, detail::GenerateOnEnterArgsFunc<TargetState>(std::forward<T1>(arg1), std::forward<Args>(args)...));
}

// InnerTransition

inline Transition InnerTransition(const StateFactory& stateFactory)
{
	return Transition(Transition::Inner, stateFactory);
}

template <typename TargetState>
Transition InnerTransition()
{
	return Transition(Transition::Inner, GetStateFactory<TargetState>());
}

template <typename TargetState, typename... Args>
Transition InnerTransition(Args&&... args)
{
	return Transition(Transition::Inner, GetStateFactory<TargetState>(), detail::GenerateOnEnterArgsFunc<TargetState>(std::forward<Args>(args)...));
}

template <typename TargetState, typename T1, typename... Args>
Transition InnerTransition(const StateOverride<TargetState>& stateOverride, T1&& arg1, Args&&... args)
{
	return Transition(Transition::Inner, stateOverride.mStateFactory, detail::GenerateOnEnterArgsFunc<TargetState>(std::forward<T1>(arg1), std::forward<Args>(args)...));
}

// InnerEntryTransition

inline Transition InnerEntryTransition(const StateFactory& stateFactory)
{
	return Transition(Transition::InnerEntry, stateFactory);
}

template <typename TargetState>
Transition InnerEntryTransition()
{
	return Transition(Transition::InnerEntry, GetStateFactory<TargetState>());
}

template <typename TargetState, typename... Args>
Transition InnerEntryTransition(Args&&... args)
{
	return Transition(Transition::InnerEntry, GetStateFactory<TargetState>(), detail::GenerateOnEnterArgsFunc<TargetState>(std::forward<Args>(args)...));
}

template <typename TargetState, typename T1, typename... Args>
Transition InnerEntryTransition(const StateOverride<TargetState>& stateOverride, T1&& arg1, Args&&... args)
{
	return Transition(Transition::InnerEntry, stateOverride.mStateFactory, detail::GenerateOnEnterArgsFunc<TargetState>(std::forward<T1>(arg1), std::forward<Args>(args)...));
}


// NoTransition

inline Transition NoTransition()
{
	return Transition();
}

} // namespace hsm

#ifdef _MSC_VER
#pragma endregion "Transition"
#endif

#ifdef _MSC_VER
#pragma region "State"
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
// State
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace hsm {

class StateMachine;

// StateValue

template <typename T>
struct ConcreteStateValueResetter;

template <typename T>
struct StateValue
{
	// Constructor - allows client to initialize StateValue; afterward,
	// can only assign via State::SetStateValue().
	// Note that T must be default constructable.
	explicit StateValue(const T& initValue = T()) { mValue = initValue; }

	// Sometimes value cannot be set only from constructor, so we provide this setter;
	// however it should only be used to initialize the value before states manipulate it.
	void SetInitialValue(const T& initValue) { mValue = initValue; }

	// Implicit conversion operator to const T& (but not T&)
	operator const T&() const { return mValue; }

	// Explicitly returns value
	const T& Value() const { return mValue; }

private:
	// Disable copy
	StateValue(const StateValue& rhs);
	StateValue& operator=(StateValue& rhs);

	friend struct ConcreteStateValueResetter<T>;
	friend struct State;
	T mValue;
};

struct StateValueResetter
{
	virtual ~StateValueResetter() {}
};

template <typename T>
struct ConcreteStateValueResetter : StateValueResetter
{
	ConcreteStateValueResetter(StateValue<T>& stateValue)
	{
		mStateValue = &stateValue;
		mOrigValue = stateValue.mValue;
	}

	virtual ~ConcreteStateValueResetter()
	{
		mStateValue->mValue = mOrigValue;
	}

	StateValue<T>* mStateValue;
	T mOrigValue;
};


// State

namespace detail
{
	void InitState(State* state, StateMachine* ownerStateMachine, size_t stackDepth);
}

struct State
{
	State()
		: mOwnerStateMachine(0)
		, mStackDepth(0)
		, mStateValueResetters(0)
		, mStateDebugName(0)
	{
	}

	virtual ~State()
	{
		ResetStateValues();
	}

	// RTTI interface
	StateTypeId GetStateType() const { return mStateTypeId; }
	const hsm_char* GetStateDebugName() const { return mStateDebugName; }

	// Accessors
	StateMachine& GetStateMachine() { HSM_ASSERT(mOwnerStateMachine != 0); return *mOwnerStateMachine; }
	const StateMachine& GetStateMachine() const { HSM_ASSERT(mOwnerStateMachine != 0); return *mOwnerStateMachine; }
	
	Owner*& GetOwner() { return mOwner; }
	Owner*const& GetOwner() const { return mOwner; }

	// Searches for state on stack from outermost to innermost, returns NULL if not found
	template <typename StateType>
	StateType* GetState();

	template <typename StateType>
	const StateType* GetState() const;

	// Searches for state on stack starting from immediate outer to outermost, returns NULL if not found
	template <typename StateType>
	StateType* GetOuterState();

	template <typename StateType>
	const StateType* GetOuterState() const;

	// Searches for state on stack starting from immediate inner to innermost, returns NULL if not found
	template <typename StateType>
	StateType* GetInnerState();

	template <typename StateType>
	const StateType* GetInnerState() const;

	// Returns state on the stack immediately below us (our inner) if one exists
	State* GetImmediateInnerState();
	const State* GetImmediateInnerState() const;

	// Returns state on the stack immediately below us (our inner) if one exists AND is type StateType
	template <typename StateType>
	StateType* GetImmediateInnerState();

	template <typename StateType>
	const StateType* GetImmediateInnerState() const;

	// Boolean query functions

	template <typename StateType>
	hsm_bool IsInState() const;

	template <typename StateType>
	hsm_bool IsInOuterState() const { return GetOuterState<StateType>() != 0; }

	template <typename StateType>
	hsm_bool IsInInnerState() const { return GetInnerState<StateType>() != 0; }

	template <typename StateType>
	hsm_bool IsInImmediateInnerState() const { return GetImmediateInnerState<StateType>() != 0; }

	// Called from state functions (usually OnEnter()) to bind a StateValue to current state. Rather than
	// passing in the new value, we return a writable reference to the StateValue's internal value to support
	// modifying data members of structs/classes.
	template <typename T>
	T& SetStateValue(StateValue<T>& stateValue)
	{
		// Lazily add a resetter for this StateValue
		if (!FindStateValueInResetterList(stateValue))
		{
			mStateValueResetters.push_back( HSM_NEW ConcreteStateValueResetter<T>(stateValue) );
		}

		// Return its value so it can be modified
		return stateValue.mValue;
	}

	// Overridable functions

	// OnEnter is invoked when a State is created; Note that GetStateMachine() is valid in OnEnter.
	// Also note that the function does not need to be virtual as the state machine invokes it
	// directly on the most-derived type (not polymorphically); however, we make it virtual to avoid
	// compiler warnings about hiding base class functions.
	virtual void OnEnter() {}

	// You can also pass args to OnEnter via any of the transition functions. Just make sure to declare an 
	// OnEnter that matches the argument types. Since the OnEnter call is delayed, the arguments will be copied.
	// Use std::ref() to avoid the copy (but make sure the object is still valid by the time OnEnter is called!).
	// 
	//void OnEnter(...)

	// OnExit is invoked just before a State is destroyed
	virtual void OnExit() {}

	// Called by StateMachine::ProcessStateTransitions from outermost to innermost state, repeatedly until
	// the state stack has settled (i.e. all states return NoTransition). Override this function to return
	// a state to transition to, or NoTransition to remain in this state. Generally, this function should avoid
	// side-effects (updating state) as it may be called several times on the same state per ProcessStateTransitions.
	// Instead, it should read state to determine whether a transition should be made. Override Update instead
	// to update the current state.
	virtual Transition GetTransition()
	{
		return NoTransition();
	}

	// Called by StateMachine::UpdateStates from outermost to innermost state. Usually invoked after the state
	// stack has settled, and is where a state can do it's work.
	virtual void Update(HSM_STATE_UPDATE_ARGS) {}

	template <typename SourceState>
	StateOverride<SourceState> GetStateOverride();

private:

#if HSM_CPP_RTTI
	StateTypeIdStorage DoGetStateType() const { return typeid(*this); }
	const hsm_char* DoGetStateDebugName() const { return typeid(*this).name(); }
#else
	// These are implemented in each state via the DEFINE_HSM_STATE macro
	virtual StateTypeId DoGetStateType() const = 0;
	virtual const hsm_char* DoGetStateDebugName() const = 0;
#endif

	friend void detail::InitState(State* state, StateMachine* ownerStateMachine, size_t stackDepth);

	template <typename T>
	StateValue<T>* FindStateValueInResetterList(StateValue<T>& stateValue)
	{
		StateValueResetterList::iterator iter = mStateValueResetters.begin();
		const StateValueResetterList::iterator& iterEnd = mStateValueResetters.end();
		for ( ; iter != iterEnd; ++iter)
		{
			if (&stateValue == static_cast<ConcreteStateValueResetter<T>*>(*iter)->mStateValue)
			{
				return &stateValue;
			}
		}
		return 0;
	}

	void ResetStateValues()
	{
		// Destroy StateValues (will reset to old value)
		StateValueResetterList::iterator iter = mStateValueResetters.begin();
		const StateValueResetterList::iterator& iterEnd = mStateValueResetters.end();
		for ( ; iter != iterEnd; ++iter)
		{
			HSM_DELETE(*iter);
		}
		mStateValueResetters.clear();
	}

	typedef HSM_STD_VECTOR<StateValueResetter*> StateValueResetterList;

	StateMachine* mOwnerStateMachine;
	Owner* mOwner; // Cached for performance and easier debugging
	size_t mStackDepth; // Depth of this state instance on the stack
	StateValueResetterList mStateValueResetters;

	// Values cached to avoid virtual call, especially since the values are constant
	StateTypeIdStorage mStateTypeId;
	const hsm_char* mStateDebugName;
};


// StateWithOwner

// Class that clients can use instead of deriving directly from State that provides convenient
// typed access to the Owner. This class can also be chained via the StateBaseType parameter,
// which is useful when inheriting state machines.

template <typename OwnerType, typename StateBaseType = State>
struct StateWithOwner : StateBaseType
{
	using State::GetStateMachine;
	using State::GetOwner;

	StateWithOwner() : mOwner(reinterpret_cast<OwnerType*&>(GetOwner())) {}

	const OwnerType& Owner() const
	{
		HSM_ASSERT(mOwner);
		return *mOwner;
	}

	OwnerType& Owner()
	{
		HSM_ASSERT(mOwner);
		return *mOwner;
	}

private:
	// Hide base class member with one who's type is OwnerType*, rather than void*. Also helpful when debugging.
	OwnerType* const& mOwner;
};

} // namespace hsm

#ifdef _MSC_VER
#pragma endregion "State"
#endif

#ifdef _MSC_VER
#pragma region "StateMachine"
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
// StateMachine
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace hsm {

// State stack types
typedef HSM_STD_VECTOR<State*> StackType;
typedef StackType::iterator OuterToInnerIterator;
typedef StackType::reverse_iterator InnerToOuterIterator;

namespace TraceLevel
{
	enum Type
	{
		None = 0,
		Basic = 1,
		Diagnostic = 2
	};
};

// The main interface to the hierarchical state machine; a single state machine
// manages a stack of states.
class StateMachine
{
public:
	StateMachine();
	~StateMachine();

	// Initializes the state machine
	template <typename InitialStateType>
	void Initialize(Owner* owner = 0)
	{
		HSM_ASSERT(mInitialTransition.IsNo());
		mInitialTransition = SiblingTransition(GetStateFactory<InitialStateType>());
		mOwner = owner;
	}

	//@NOTE: Removing this overload as it causes ambiguity when owner is not specified.
	// Can make this work using SFINAE to enable this overload when StateArgsType derives
	// from hsm::StateArgs. This would be simpler in C++11.
	//
	// Initializes the state machie with StateArgs for the initial state.
	//template <typename InitialStateType, typename StateArgsType>
	//void Initialize(const StateArgsType& initialStateArgs, Owner* owner = 0)
	//{
	//	HSM_ASSERT(mInitialTransition.IsNo());
	//	mInitialTransition = SiblingTransition(GetStateFactory<InitialStateType>(), initialStateArgs);
	//	mOwner = owner;
	//}

	// Shuts down the state machine, after which Initialize() must be called to use the state machine again.
	// If stop is true, invokes Stop(). Destructor calls Shutdown(false).
	void Shutdown(hsm_bool stop = hsm_true);

	// Returns true after Initialize and before Shutdown are invoked
	hsm_bool IsInitialized() const { return !mInitialTransition.IsNo(); }

	// Pops all states off the state stack, including initial state, invoking OnExit on each one in inner-to-outer order.
	// A subsequent call to ProcessStateTransitions will re-populate the state stack.
	// After invoking Stop and before ProcessStateTransitions, IsStarted returns false.
	void Stop();

	// Started means the state stack is not empty
	hsm_bool IsStarted() { return !mStateStack.empty(); }

	// Debug tracing
	void SetDebugInfo(const hsm_char* name, TraceLevel::Type traceLevel);
	void SetDebugName(const hsm_char* name);
	const hsm_char* GetDebugName() const { return mDebugName; }
	void SetDebugTraceLevel(TraceLevel::Type trace) { mDebugTraceLevel = trace; }
	TraceLevel::Type GetDebugTraceLevel() const { return mDebugTraceLevel; }

	// Call to update the state stack (usually once per frame). This function will iterate over the state stack,
	// calling GetTransition() on each state, and will perform transitions until all states return NoTransition.
	void ProcessStateTransitions();

	// Call after ProcessStateTransitions (once the state stack has settled) to allow each state to perform its
	// work. Will invoke Update() on each state, from outermost to innermost.
	void UpdateStates(HSM_STATE_UPDATE_ARGS);

	// Owner accessors (may return NULL)
	Owner* GetOwner() { return mOwner; }
	const Owner* GetOwner() const { return mOwner; }

	// State stack iterators
	OuterToInnerIterator BeginOuterToInner() { return mStateStack.begin(); }
	OuterToInnerIterator EndOuterToInner() { return mStateStack.end(); }
	InnerToOuterIterator BeginInnerToOuter() { return mStateStack.rbegin(); }
	InnerToOuterIterator EndInnerToOuter() { return mStateStack.rend(); }

	// State stack visitor functions
	// Func should be "void (State&)" or "void (DerivedState&)"
	template <typename StateMemFun, typename... Args>
	void InvokeOuterToInner(StateMemFun&& memFunc, Args&&... args);

	template <typename StateMemFun, typename... Args>
	void InvokeInnerToOuter(StateMemFun&& memFunc, Args&&... args);

	// State stack visitor functions
	// Func should be "void (State&)" or "void (DerivedState&)"
	template <typename Func>
	void VisitOuterToInner(Func&& func);

	template <typename Func>
	void VisitInnerToOuter(Func&& func);

	// State stack query functions

	// Returns NULL if state is not found on the stack
	State* GetState(StateTypeId stateType);
	const State* GetState(StateTypeId stateType) const { return const_cast<const State*>( const_cast<StateMachine*>(this)->GetState(stateType) ); }

	hsm_bool IsInState(StateTypeId stateType) const { return GetState(stateType) != 0; }

	template <typename StateType>
	StateType* GetState() { return static_cast<StateType*>(GetState(hsm::GetStateType<StateType>())); }

	template <typename StateType>
	hsm_bool IsInState() const { return IsInState(hsm::GetStateType<StateType>()); }

	// State override functions

	template <typename SourceState, typename TargetState>
	void AddStateOverride();

	template <typename SourceState>
	void RemoveStateOverride();

	template <typename SourceState>
	const StateFactory& GetStateOverride();


	//@DEPRECATED: Initialize should no longer accept debug info. Use SetDebugInfo instead.
	template <typename InitialStateType>
	void Initialize(Owner* owner, const hsm_char* debugName, size_t debugLevel)
	{
		HSM_ASSERT(mInitialTransition.IsNo());
		mInitialTransition = SiblingTransition(GetStateFactory<InitialStateType>());
		mOwner = owner;
		SetDebugInfo(debugName, debugLevel);
	}

	//@DEPRECATED: Use SetDebugInfo(const hsm_char*, TraceLevel::Type)
	void SetDebugInfo(const hsm_char* name, size_t level) { SetDebugName(name); SetDebugLevel(level); }

	//@DEPRECATED: Use SetDebugTraceLevel
	void SetDebugLevel(size_t level) { SetDebugTraceLevel(static_cast<TraceLevel::Type>(level)); }

	//@DEPRECATED: Use GetDebugTraceLevel
	size_t GetDebugLevel() { return static_cast<size_t>(GetDebugTraceLevel()); }

private:
	friend struct State;

	void CreateAndPushInitialState(const Transition& transition);

	// Returns state at input depth, or NULL if depth is invalid
	State* GetStateAtDepth(size_t depth);

	// Overload returns state at input depth if it matches input type
	State* GetStateAtDepth(size_t depth, StateTypeId stateType);

	State* GetOuterState(StateTypeId stateType, size_t startDepth);
	const State* GetOuterState(StateTypeId stateType, size_t startDepth) const;
	State* GetInnerState(StateTypeId stateType, size_t startDepth);
	const State* GetInnerState(StateTypeId stateType, size_t startDepth) const;

	// Pops states from most inner up to and including depth
	void PopStatesToDepth(size_t depth, hsm_bool invokeOnExit = hsm_true);

	// Returns true if a transition was made, meaning we must keep processing
	hsm_bool ProcessStateTransitionsOnce();

	void PushState(State* state);
	void PopState();

	void Log(size_t minLevel, size_t numSpaces, const hsm_char* format, ...);
	void LogTransition(size_t minLevel, size_t depth, const hsm_char* transType, State* state);

	Owner* mOwner; // Provided by client, accessed within states via StateWithOwner<>::Owner()
	Transition mInitialTransition;
	StackType mStateStack;

	typedef std::map<const StateFactory*, const StateFactory*> OverrideMap;
	OverrideMap mStateOverrides;

	hsm_char mDebugName[HSM_DEBUG_NAME_MAXLEN];
	TraceLevel::Type mDebugTraceLevel;
};


// Inline State member function implementations - implemented here because they depend StateMachine being defined

template <typename StateType>
StateType* State::GetState()
{
	return GetStateMachine().GetState<StateType>();
}

template <typename StateType>
const StateType* State::GetState() const
{
	return const_cast<State*>(this)->GetState<StateType>();
}

template <typename StateType>
StateType* State::GetOuterState()
{
	return static_cast<StateType*>(GetStateMachine().GetOuterState(hsm::GetStateType<StateType>(), mStackDepth - 1));
}

template <typename StateType>
const StateType* State::GetOuterState() const
{
	return const_cast<State*>(this)->GetOuterState<StateType>();
}

template <typename StateType>
StateType* State::GetInnerState()
{
	return static_cast<StateType*>(GetStateMachine().GetInnerState(hsm::GetStateType<StateType>(), mStackDepth + 1));
}

template <typename StateType>
const StateType* State::GetInnerState() const
{
	return const_cast<State*>(this)->GetInnerState<StateType>();
}

template <typename StateType>
hsm_bool State::IsInState() const
{
	return GetStateMachine().IsInState<StateType>();
}

inline State* State::GetImmediateInnerState()
{
	return GetStateMachine().GetStateAtDepth(mStackDepth + 1);
}

inline const State* State::GetImmediateInnerState() const
{
	return const_cast<State*>(this)->GetImmediateInnerState();
}

template <typename StateType>
inline StateType* State::GetImmediateInnerState()
{
	return static_cast<StateType*>(GetStateMachine().GetStateAtDepth(mStackDepth + 1, hsm::GetStateType<StateType>()));
}

template <typename StateType>
inline const StateType* State::GetImmediateInnerState() const
{
	return const_cast<State*>(this)->GetImmediateInnerState<StateType>();
}

template <typename SourceState>
inline StateOverride<SourceState> State::GetStateOverride()
{
	return StateOverride<SourceState>(GetStateMachine().GetStateOverride<SourceState>());
}

// Inline StateMachine function implementations

namespace detail
{
	// function_traits to get result and argument types of a function
	// http://stackoverflow.com/questions/27879815/c11-get-type-of-first-second-etc-argument-similar-to-result-of

	// primary template.
	template<typename T>
	struct function_traits
	{
		using class_type = T;
		using result_type = typename function_traits<decltype(&T::operator())>::result_type;
		using argument_types = typename function_traits<decltype(&T::operator())>::argument_types;
	};

	// partial specialization for function type
	template<typename R, typename... Args>
	struct function_traits<R(Args...)>
	{
		using result_type = R;
		using argument_types = std::tuple<Args...>;
	};

	// partial specialization for function pointer
	template<typename R, typename... Args>
	struct function_traits<R(*)(Args...)>
	{
		using result_type = R;
		using argument_types = std::tuple<Args...>;
	};

	// partial specialization for std::function
	template<typename R, typename... Args>
	struct function_traits<std::function<R(Args...)>>
	{
		using result_type = R;
		using argument_types = std::tuple<Args...>;
	};

	// partial specialization for pointer-to-member-function (i.e., operator()'s)
	template<typename T, typename R, typename... Args>
	struct function_traits<R(T::*)(Args...)>
	{
		using class_type = T;
		using result_type = R;
		using argument_types = std::tuple<Args...>;
	};

	template<typename T, typename R, typename... Args>
	struct function_traits<R(T::*)(Args...) const>
	{
		using class_type = T;
		using result_type = R;
		using argument_types = std::tuple<Args...>;
	};

	template<typename T>
	using function_first_argument_t = typename std::tuple_element<0, typename function_traits<T>::argument_types>::type;

	template <typename IterType, typename StateMemFun, typename... Args>
	inline void InvokeImpl(IterType first, IterType last, StateMemFun memFunc, Args&&... args)
	{
		using DerivedStateType = typename function_traits<StateMemFun>::class_type;
		static_assert(std::is_convertible<DerivedStateType, const hsm::State&>::value, "memFunc must be a member function of a State-derived type");
		for (auto iter = first; iter != last; ++iter)
		{
			DerivedStateType* state = static_cast<DerivedStateType*>(*iter);
			(state->*memFunc)(std::forward<Args>(args)...);
		}
	}

	template <typename IterType, typename Func>
	void VisitImpl(IterType first, IterType last, Func&& func)
	{
		static_assert(std::is_convertible<function_first_argument_t<Func>, const hsm::State&>::value, "First argument must be State* or derived type");
		for (auto iter = first; iter != last; ++iter)
		{
			func(static_cast<function_first_argument_t<Func>>(**iter));
		}
	}
}

template <typename StateMemFun, typename... Args>
inline void StateMachine::InvokeOuterToInner(StateMemFun&& memFunc, Args&&... args)
{
	detail::InvokeImpl(BeginOuterToInner(), EndOuterToInner(), std::forward<StateMemFun>(memFunc), std::forward<Args>(args)...);
}

template <typename StateMemFun, typename... Args>
inline void StateMachine::InvokeInnerToOuter(StateMemFun&& memFunc, Args&&... args)
{
	detail::InvokeImpl(BeginInnerToOuter(), EndInnerToOuter(), std::forward<StateMemFun>(memFunc), std::forward<Args>(args)...);
}

template <typename Func>
inline void StateMachine::VisitOuterToInner(Func&& func)
{
	detail::VisitImpl(BeginOuterToInner(), EndOuterToInner(), std::forward<Func>(func));
}

template <typename Func>
inline void StateMachine::VisitInnerToOuter(Func&& func)
{
	detail::VisitImpl(BeginInnerToOuter(), EndInnerToOuter(), std::forward<Func>(func));
}

template <typename SourceState, typename TargetState>
inline void StateMachine::AddStateOverride()
{
	mStateOverrides[&hsm::GetStateFactory<SourceState>()] = &hsm::GetStateFactory<TargetState>();
}

template <typename SourceState>
inline void StateMachine::RemoveStateOverride()
{
	const hsm::StateFactory& sourceStateFactory = hsm::GetStateFactory<SourceState>();
	mStateOverrides.erase(mStateOverrides.find(&sourceStateFactory));
}

template <typename SourceState>
inline const StateFactory& StateMachine::GetStateOverride()
{
	const StateFactory& sourceStateFactory = GetStateFactory<SourceState>();
	OverrideMap::iterator iter = mStateOverrides.find(&sourceStateFactory);
	return iter == mStateOverrides.end() ? sourceStateFactory : *iter->second;
}

#if !HSM_DEBUG
	#define HSM_LOG(minLevel, numSpaces, printfArgs)
	#define HSM_LOG_TRANSITION(minLevel, depth, transTypeStr, state)
#else
	#define HSM_LOG Log
	#define HSM_LOG_TRANSITION LogTransition
#endif

namespace detail
{
	inline void InitState(State* state, StateMachine* ownerStateMachine, size_t stackDepth)
	{
		HSM_ASSERT(ownerStateMachine != 0);
		state->mOwnerStateMachine = ownerStateMachine;
		state->mOwner = ownerStateMachine->GetOwner();
		state->mStackDepth = stackDepth;
		state->mStateTypeId = state->DoGetStateType();
		state->mStateDebugName = state->DoGetStateDebugName();
	}

	inline State* CreateState(const Transition& transition, StateMachine* ownerStateMachine, size_t stackDepth)
	{
		State* state = transition.GetStateFactory().AllocateState();
		InitState(state, ownerStateMachine, stackDepth);
		return state;
	}

	inline void DestroyState(State* state)
	{
		HSM_DELETE(state);
	}

	inline void InvokeStateOnEnter(const Transition& transition, State* state)
	{
		if (const auto& onEnterArgsFunc = transition.GetOnEnterArgsFunc())
		{
			onEnterArgsFunc(state);
		}
		else
		{
			state->OnEnter();
		}
	}

	inline void InvokeStateOnExit(State* state)
	{
		state->OnExit();
	}
}

inline StateMachine::StateMachine()
	: mOwner(0)
	, mDebugTraceLevel(TraceLevel::None)
{
	mDebugName[0] = '\0';
}

inline StateMachine::~StateMachine()
{
	Shutdown(hsm_false);
}

inline void StateMachine::Shutdown(hsm_bool stop)
{
	if (stop)
		Stop();

	// Free any allocated states
	PopStatesToDepth(0, hsm_false);

	mOwner = 0;
	mInitialTransition = NoTransition();
}

inline void StateMachine::Stop()
{
	PopStatesToDepth(0);
	HSM_ASSERT(mStateStack.empty());
}

inline void StateMachine::SetDebugInfo(const hsm_char* name, TraceLevel::Type traceLevel)
{
	SetDebugName(name);
	SetDebugTraceLevel(traceLevel);
}

inline void StateMachine::SetDebugName(const hsm_char* name)
{
	STRNCPY(mDebugName, name, HSM_DEBUG_NAME_MAXLEN);
	mDebugName[HSM_DEBUG_NAME_MAXLEN - 1] = '\0';
}

inline void StateMachine::ProcessStateTransitions()
{
	// If the state stack is empty, push the initial state
	if (mStateStack.empty())
	{
		HSM_ASSERT_MSG(!mInitialTransition.IsNo(), "Must call Initialize()");
		CreateAndPushInitialState(mInitialTransition);
	}

	// After we make a transition, we must process all transitions again until we get no transitions
	// from all states on the stack.
	hsm_bool keepProcessing = hsm_true;
	int numTransitionsProcessed = 0;
	while (keepProcessing)
	{
		keepProcessing = ProcessStateTransitionsOnce();

		if (++numTransitionsProcessed >= 1000)
		{
			HSM_ASSERT_MSG(hsm_false, "ProcessStateTransitions: detected infinite transition loop");
		}
	}
}

inline void StateMachine::UpdateStates(HSM_STATE_UPDATE_ARGS)
{
	OuterToInnerIterator iter = BeginOuterToInner();
	OuterToInnerIterator end = EndOuterToInner();
	for ( ; iter != end; ++iter)
	{
		(*iter)->Update(HSM_STATE_UPDATE_ARGS_FORWARD);
	}
}

inline State* StateMachine::GetState(StateTypeId stateType)
{
	for (size_t i = 0; i < mStateStack.size(); ++i)
	{
		State* state = mStateStack[i];
		if (state->GetStateType() == stateType)
			return state;
	}
	return 0;
}

inline State* StateMachine::GetStateAtDepth(size_t depth)
{
	if (depth >= mStateStack.size())
	{
		return 0;
	}

	return mStateStack[depth];
}

inline State* StateMachine::GetStateAtDepth(size_t depth, StateTypeId stateType)
{
	State* state = GetStateAtDepth(depth);
	return (state && state->GetStateType() == stateType) ? state : 0;
}

inline State* StateMachine::GetOuterState(StateTypeId stateType, size_t startDepth)
{
	const size_t numStatesToCompare = startDepth + 1;
	size_t currDepth = startDepth;

	for (size_t i = 0; i < numStatesToCompare; ++i, --currDepth)
	{
		State* state = mStateStack[currDepth];
		if (state->GetStateType() == stateType)
			return state;
	}
	return 0;
}

inline const State* StateMachine::GetOuterState(StateTypeId stateType, size_t startDepth) const
{
	return const_cast<StateMachine*>(this)->GetOuterState(stateType, startDepth);
}

inline State* StateMachine::GetInnerState(StateTypeId stateType, size_t startDepth)
{
	for (size_t i = startDepth; i < mStateStack.size(); ++i)
	{
		State* state = mStateStack[i];
		if (state->GetStateType() == stateType)
			return state;
	}
	return 0;
}

inline const State* StateMachine::GetInnerState(StateTypeId stateType, size_t startDepth) const
{
	return const_cast<StateMachine*>(this)->GetInnerState(stateType, startDepth);
}

inline void StateMachine::CreateAndPushInitialState(const Transition& transition)
{
	HSM_ASSERT(mStateStack.empty());
	State* initialState = detail::CreateState(transition, this, 0);
	HSM_LOG_TRANSITION(1, 0, HSM_TEXT("Init"), initialState);
	PushState(initialState);
	detail::InvokeStateOnEnter(transition, initialState);
}

inline void StateMachine::PopStatesToDepth(size_t depth, hsm_bool invokeOnExit)
{
	const size_t numStatesToPop = mStateStack.size() - depth;
	size_t currDepth = mStateStack.size() - 1;

	for (size_t i = 0; i < numStatesToPop; ++i, --currDepth)
	{
		State* state = mStateStack.back();
		HSM_ASSERT(state == mStateStack.at(currDepth));

		if (invokeOnExit)
		{
			HSM_LOG_TRANSITION(2, currDepth, HSM_TEXT("Pop"), state);
			detail::InvokeStateOnExit(state);
		}
		PopState();
		detail::DestroyState(state);
	}
}

inline hsm_bool StateMachine::ProcessStateTransitionsOnce()
{
	// Process transitions from outermost to innermost states; if a valid sibling transition
	// is returned, we must pop inners up to and including the state that returned the transition,
	// then push the new inner. If an inner transition is returned, we must pop inners up to but
	// not including the state that returned the transition (if any), then push the new inner.

	for (size_t depth = 0; depth < mStateStack.size(); ++depth)
	{
		State* currState = GetStateAtDepth(depth);
		const Transition& transition = currState->GetTransition();

		switch (transition.GetTransitionType())
		{
			case Transition::No:
			{
				// Move on to next inner
				continue;
			}
			break;

			case Transition::Inner:
			{
				if (State* innerState = GetStateAtDepth(depth + 1))
				{
					if ( transition.GetTargetStateType() == innerState->GetStateType() )
					{
						// Inner is already target state so keep going to next inner
						continue;
					}
					else
					{
						// Pop all states under us and push target
						PopStatesToDepth(depth + 1);

						State* targetState = detail::CreateState(transition, this, depth + 1);
						HSM_LOG_TRANSITION(1, depth + 1, HSM_TEXT("Inner"), targetState);
						PushState(targetState);
						detail::InvokeStateOnEnter(transition, targetState);
						return hsm_true;
					}
				}
				else
				{
					// No state under us so just push target
					State* targetState = detail::CreateState(transition, this, depth + 1);
					HSM_LOG_TRANSITION(1, depth + 1, HSM_TEXT("Inner"), targetState);
					PushState(targetState);
					detail::InvokeStateOnEnter(transition, targetState);
					return hsm_true;
				}
			}
			break;

			case Transition::InnerEntry:
			{
				// If current state has no inner (is currently the innermost), then push the entry state
				if ( !GetStateAtDepth(depth + 1) )
				{
					State* targetState = detail::CreateState(transition, this, depth + 1);
					HSM_LOG_TRANSITION(1, depth + 1, HSM_TEXT("Entry"), targetState);
					PushState(targetState);
					detail::InvokeStateOnEnter(transition, targetState);
					return hsm_true;
				}
			}
			break;

			case Transition::Sibling:
			{
				PopStatesToDepth(depth);

				State* targetState = detail::CreateState(transition, this, depth);
				HSM_LOG_TRANSITION(1, depth, HSM_TEXT("Sibling"), targetState);
				PushState(targetState);
				detail::InvokeStateOnEnter(transition, targetState);
				return hsm_true;
			}
			break;

		} // end switch on transition type
	} // end for each depth

	return hsm_false;
}

inline void StateMachine::PushState(State* state)
{
	mStateStack.push_back(state);
}

inline void StateMachine::PopState()
{
	mStateStack.pop_back();
}

inline void StateMachine::Log(size_t minLevel, size_t numSpaces, const hsm_char* format, ...)
{
	if (static_cast<size_t>(mDebugTraceLevel) >= minLevel)
	{
		static hsm_char buffer[4096];
 		int offset = SNPRINTF(buffer, sizeof(buffer), HSM_TEXT("HSM_%lu_%s:%*s "), static_cast<unsigned long>(minLevel), mDebugName, static_cast<int>(numSpaces), "");

		va_list args;
		va_start(args, format);
		VSNPRINTF(buffer + offset, sizeof(buffer) - offset - 1, format, args);

		// Print to stdout
		HSM_PRINTF(HSM_TEXT("%s"), buffer);
		va_end(args);
	}
}

inline void StateMachine::LogTransition(size_t minLevel, size_t depth, const hsm_char* transType, State* state)
{
	Log(minLevel, depth, HSM_TEXT("%-8s: %s\n"), transType, state->GetStateDebugName());
}

#undef HSM_LOG
#undef HSM_LOG_TRANSITION

} // namespace hsm

#ifdef _MSC_VER
#pragma endregion "StateMachine"
#endif

#endif // __HSM_H__
