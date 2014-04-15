// Hierarchical State Machine (HSM)
//
// Copyright (c) 2013 Antonio Maiorano
//
// Distributed under the MIT License (MIT)
// (See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT)

/// \file statemachine.h
/// \brief StateMachine interface and main library include file.

#ifndef HSM_STATEMACHINE_H
#define HSM_STATEMACHINE_H

#include "state.h"

namespace hsm {

struct State;

// State stack types
typedef HSM_STD_VECTOR<State*> StackType;
typedef StackType::iterator OuterToInnerIterator;
typedef StackType::reverse_iterator InnerToOuterIterator;

// The main interface to the hierarchical state machine; a single state machine
// manages a stack of states.
class StateMachine
{
public:
	StateMachine();
	~StateMachine();

	// Initializes the state machine
	template <typename InitialStateType>
	void Initialize(Owner* owner = 0, const hsm_char* debugName = HSM_TEXT("Unnamed"), size_t debugLevel = 0)
	{
		HSM_ASSERT(mInitialTransition.IsNo());
		mInitialTransition = SiblingTransition(GetStateFactory<InitialStateType>());
		mOwner = owner;
		SetDebugInfo(debugName, debugLevel);
	}

	// Overload that takes StateArgs for the initial state
	template <typename InitialStateType, typename StateArgsType>
	void Initialize(const StateArgsType& initialStateArgs, Owner* owner = 0, const hsm_char* debugName = HSM_TEXT("Unnamed"), size_t debugLevel = 0)
	{
		HSM_ASSERT(mInitialTransition.IsNo());
		mInitialTransition = SiblingTransition(GetStateFactory<InitialStateType>(), initialStateArgs);
		mOwner = owner;
		SetDebugInfo(debugName, debugLevel);
	}

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

	// Debug
	void SetDebugInfo(const hsm_char* debugName, size_t debugLevel);
	const hsm_char* GetDebugName() const { return mDebugName; }
	void SetDebugLevel(size_t debugLevel) { mDebugLevel = debugLevel; }
	size_t GetDebugLevel() { return mDebugLevel; }

	// Call to update the state stack (usually once per frame). This function will iterate over the state stack,
	// calling GetTransition() on each state, and will perform transitions until all states return NoTransition.
	void ProcessStateTransitions();

	// Call after ProcessStateTransitions (once the state stack has settled) to allow each state to perform its
	// work. Will invoke Update() on each state, from outermost to innermost.
	void UpdateStates(HSM_STATE_UPDATE_ARGS);

	// Owner accessors
	Owner* GetOwner() { return mOwner; }
	const Owner* GetOwner() const { return mOwner; }

	// State stack iterators
	OuterToInnerIterator BeginOuterToInner() { return mStateStack.begin(); }
	OuterToInnerIterator EndOuterToInner() { return mStateStack.end(); }
	InnerToOuterIterator BeginInnerToOuter() { return mStateStack.rbegin(); }
	InnerToOuterIterator EndInnerToOuter() { return mStateStack.rend(); }

	// State stack query functions

	// Returns NULL if state is not found on the stack
	State* GetState(StateTypeId stateType);
	const State* GetState(StateTypeId stateType) const { return const_cast<const State*>( const_cast<StateMachine*>(this)->GetState(stateType) ); }

	hsm_bool IsInState(StateTypeId stateType) const { return GetState(stateType) != 0; }

	template <typename StateType>
	StateType* GetState() { return static_cast<StateType*>(GetState(hsm::GetStateType<StateType>())); }

	template <typename StateType>
	hsm_bool IsInState() const { return IsInState(hsm::GetStateType<StateType>()); }

private:
	friend struct State;

	void CreateAndPushInitialState(const Transition& transition);

	// Returns state at input depth, or NULL if depth is invalid
	State* GetStateAtDepth(size_t depth);

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
	size_t mDebugLevel;
	hsm_char mDebugName[HSM_DEBUG_NAME_MAXLEN];
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

} // namespace hsm

#endif // HSM_STATEMACHINE_H
