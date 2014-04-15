// Hierarchical State Machine (HSM)
//
// Copyright (c) 2013 Antonio Maiorano
//
// Distributed under the MIT License (MIT)
// (See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT)

/// \file transition.h
/// \brief Transition and related interfaces.

#ifndef HSM_TRANSITION_H
#define HSM_TRANSITION_H

#include "rtti.h"
#include "utils.h"

namespace hsm {

struct State;
struct StateFactory;

// StateArgs: For states that wish to receive arguments via OnEnter, implement an inner struct named 'Args' that derives
// from StateArgs, and implement State::OnEnter(const Args& args).
struct StateArgs : util::intrusive_ptr_client
{
	virtual ~StateArgs() {} // Make sure destructors get called in derived types
};

// Returns the one StateFactory instance for the input state. Note that this type can be used to effectively store
// a state in a variable at runtime, which can subsequently be passed to a Transition function.
template <typename TargetState>
const StateFactory& GetStateFactory();

// State creation interface
struct StateFactory
{
	virtual StateTypeId GetStateType() const = 0;
	virtual State* AllocateState() const = 0;
	virtual void InvokeStateOnEnter(State* state, const StateArgs* stateArgs) const = 0;
};

namespace detail
{
	template <bool condition, typename TrueType, typename FalseType>
	struct Select
	{
		typedef TrueType Type;
	};

	template <typename TrueType, typename FalseType>
	struct Select<false, TrueType, FalseType>
	{
		typedef FalseType Type;
	};
}

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

	virtual void InvokeStateOnEnter(State* state, const StateArgs* stateArgs) const
	{
		// We select which functor to call at compile-time so that only states that expect StateArgs are required to implement
		// an OnEnter(const Args& args) where Args is a struct derived from StateArgs defined within TargetState.
		const bool expectsStateArgs = sizeof(typename TargetState::Args) > sizeof(StateArgs);
		typedef typename detail::Select<expectsStateArgs, InvokeStateOnEnterWithArgsFunctor, InvokeStateOnEnterNoArgsFunctor>::Type Functor;
		Functor::Execute(state, stateArgs);
	}

private:
	// Only GetStateFactory can create this type
	friend const StateFactory& GetStateFactory<TargetState>();
	ConcreteStateFactory() {}

	struct InvokeStateOnEnterNoArgsFunctor
	{
		static void Execute(State* state, const StateArgs* stateArgs)
		{
			 HSM_ASSERT_MSG(stateArgs == 0, "Target state does not expect args, yet args were passed in via the transition");

			 //@NOTE: Compiler will fail here if TargetState defines OnEnter(const Args&)
			 static_cast<TargetState*>(state)->OnEnter();
		}
	};

	struct InvokeStateOnEnterWithArgsFunctor
	{
		static void Execute(State* state, const StateArgs* stateArgs)
		{
			HSM_ASSERT_MSG(stateArgs != 0, "Target state expects args, make sure to pass them in in via the transition");

			//@NOTE: Compiler will fail here if TargetState does not define OnEnter(const Args&)
			static_cast<TargetState*>(state)->OnEnter( static_cast<const typename TargetState::Args&>(*stateArgs) );
		}
	};
};

template <typename TargetState>
const StateFactory& GetStateFactory()
{
	static ConcreteStateFactory<TargetState> instance;
	return instance;
}


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
	template <typename StateArgsType>
	Transition(Transition::Type transitionType, const StateFactory& stateFactory, const StateArgsType& stateArgs)
		: mTransitionType(transitionType)
		, mStateFactory(&stateFactory)
	{
		// Copy-construct new instance of state args, stored in intrusive_ptr for ref counting
		mStateArgs.reset( HSM_NEW StateArgsType(stateArgs) );
	}

	Transition::Type GetTransitionType() const { return mTransitionType; }
	StateTypeId GetTargetStateType() const { HSM_ASSERT(mStateFactory != 0); return mStateFactory->GetStateType(); }	
	const StateFactory& GetStateFactory() const { HSM_ASSERT(mStateFactory != 0); return *mStateFactory; }

	hsm_bool IsSibling() const { return mTransitionType == Sibling; }
	hsm_bool IsInner() const { return mTransitionType == Inner; }
	hsm_bool IsInnerEntry() const { return mTransitionType == InnerEntry; }
	hsm_bool IsNo() const { return mTransitionType == No; }

	//@NOTE: Do not cache returned pointer
	const StateArgs* GetStateArgs() const { return mStateArgs.get(); }

private:
	Transition::Type mTransitionType;
	const StateFactory* mStateFactory; // Bald pointer is safe for shallow copying because StateFactory instances are always statically allocated
	util::intrusive_ptr<const StateArgs> mStateArgs; // Reference counted pointer so we can safely copy Transitions without leaking
};


// Transition generators - use these to return from State::GetTransition()

// SiblingTransition

inline Transition SiblingTransition(const StateFactory& stateFactory)
{
	return Transition(Transition::Sibling, stateFactory);
}

template <typename StateArgsType>
Transition SiblingTransition(const StateFactory& stateFactory, const StateArgsType& stateArgs)
{
	return Transition(Transition::Sibling, stateFactory, stateArgs);
}

template <typename TargetState>
Transition SiblingTransition()
{
	return Transition(Transition::Sibling, GetStateFactory<TargetState>());
}

template <typename TargetState, typename StateArgsType>
Transition SiblingTransition(const StateArgsType& stateArgs)
{
	return Transition(Transition::Sibling, GetStateFactory<TargetState>(), stateArgs);
}


// InnerTransition

inline Transition InnerTransition(const StateFactory& stateFactory)
{
	return Transition(Transition::Inner, stateFactory);
}

template <typename StateArgsType>
Transition InnerTransition(const StateFactory& stateFactory, const StateArgsType& stateArgs)
{
	return Transition(Transition::Inner, stateFactory, stateArgs);
}

template <typename TargetState>
Transition InnerTransition()
{
	return Transition(Transition::Inner, GetStateFactory<TargetState>());
}

template <typename TargetState, typename StateArgsType>
Transition InnerTransition(const StateArgsType& stateArgs)
{
	return Transition(Transition::Inner, GetStateFactory<TargetState>(), stateArgs);
}


// InnerEntryTransition

inline Transition InnerEntryTransition(const StateFactory& stateFactory)
{
	return Transition(Transition::InnerEntry, stateFactory);
}

template <typename StateArgsType>
Transition InnerEntryTransition(const StateFactory& stateFactory, const StateArgsType& stateArgs)
{
	return Transition(Transition::InnerEntry, stateFactory, stateArgs);
}

template <typename TargetState>
Transition InnerEntryTransition()
{
	return Transition(Transition::InnerEntry, GetStateFactory<TargetState>());
}

template <typename TargetState, typename StateArgsType>
Transition InnerEntryTransition(const StateArgsType& stateArgs)
{
	return Transition(Transition::InnerEntry, GetStateFactory<TargetState>(), stateArgs);
}


// NoTransition

inline Transition NoTransition()
{
	return Transition();
}

} // namespace hsm

#endif // HSM_TRANSITION_H
