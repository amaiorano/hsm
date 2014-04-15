// Hierarchical State Machine (HSM)
//
// Copyright (c) 2013 Antonio Maiorano
//
// Distributed under the MIT License (MIT)
// (See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT)

/// \file state.h
/// \brief State and related interfaces.

#ifndef HSM_STATE_H
#define HSM_STATE_H

#include "transition.h"

namespace hsm {

class StateMachine;

///////////////////////////////////////////////////////////////////////////////
// StateValues
///////////////////////////////////////////////////////////////////////////////

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

namespace detail
{
	void InitState(State* state, StateMachine* ownerStateMachine, size_t stackDepth);
}


///////////////////////////////////////////////////////////////////////////////
// State
///////////////////////////////////////////////////////////////////////////////

struct State
{
	State()
		: mOwnerStateMachine(0)
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

	template <typename StateType>
	hsm_bool IsInState() const;

	template <typename StateType>
	hsm_bool IsInOuterState() const { return GetOuterState<StateType>() != 0; }

	template <typename StateType>
	hsm_bool IsInInnerState() const { return GetInnerState<StateType>() != 0; }

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

	// Child states are expected to hide this type with their own struct named Args that derives from StateArgs
	typedef StateArgs Args;

	// Overridable functions

	// OnEnter is invoked when a State is created; Note that GetStateMachine() is valid in OnEnter.
	// Also note that the function does not need to be virtual as the state machine invokes it
	// directly on the most-derived type (not polymorphically); however, we make it virtual for consistency.
	virtual void OnEnter() {}
	
	// If state expects StateArgs, the overridden version should look like this
	//virtual void OnEnter(const Args& args);

	// OnExit is invoked just before a State is destroyed
	virtual void OnExit() {}
	
	// Called by StateMachine::ProcessStateTransitions from outermost to innermost state, repeatedly until
	// the state stack has settled (i.e. all states return NoTransition). Override this function to return
	// a state to transition to, or NoTransition to remain in this state. Generally, this function should avoid
	// side-effects (updating state) as it may be called several times on the same state per ProcessStateTransitions.
	// Instead, it should read state to determine whether a transition should be made. For udpating, override
	// the Update function.
	virtual Transition GetTransition()
	{
		return NoTransition();
	}

	// Called by StateMachine::UpdateStates from outermost to innermost state. Usually invoked after the state
	// stack has settled, and is where a state can do it's work.
	virtual void Update(HSM_STATE_UPDATE_ARGS) {}

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
	size_t mStackDepth; // Depth of this state instance on the stack
	StateValueResetterList mStateValueResetters;
	
	// Values cached to avoid virtual call, especially since the values are constant
	StateTypeIdStorage mStateTypeId;
	const hsm_char* mStateDebugName;
};


///////////////////////////////////////////////////////////////////////////////
// StateWithOwner
///////////////////////////////////////////////////////////////////////////////

// Class that clients can use instead of deriving directly from State that provides convenient
// typed access to the Owner. This class can also be chained via the StateBaseType parameter,
// which is useful when inheriting state machines.

template <typename OwnerType, typename StateBaseType = State>
struct StateWithOwner : StateBaseType
{
	using StateBaseType::GetStateMachine;
	typedef StateWithOwner<OwnerType, StateBaseType> ThisType;

	const OwnerType& Owner() const
	{
		HSM_ASSERT(GetStateMachine().GetOwner() != 0);
		return *static_cast<const OwnerType*>(GetStateMachine().GetOwner());
	}

	OwnerType& Owner()
	{
		return const_cast<OwnerType&>( const_cast<const ThisType*>(this)->Owner() );
	}
};

} // namespace hsm

#endif // HSM_STATE_H
