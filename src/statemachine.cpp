// Hierarchical State Machine (HSM)
//
// Copyright (c) 2013 Antonio Maiorano
//
// Distributed under the MIT License (MIT)
// (See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT)

/// \file statemachine.cpp
/// \brief StateMachine implementation.

#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif // _MSC_VER

#include "../include/hsm/statemachine.h"
#include <stdarg.h>

namespace hsm {

#ifndef HSM_DEBUG
	#define HSM_LOG(minLevel, numSpaces, printfArgs)
	#define HSM_LOG_TRANSITION(minLevel, depth, transTypeStr, state)
#else
	#define HSM_LOG Log
	#define HSM_LOG_TRANSITION LogTransition
#endif

namespace detail
{
	void InitState(State* state, StateMachine* ownerStateMachine, size_t stackDepth)
	{
		HSM_ASSERT(ownerStateMachine != 0);
		state->mOwnerStateMachine = ownerStateMachine;
		state->mStackDepth = stackDepth;
		state->mStateTypeId = state->DoGetStateType();
		state->mStateDebugName = state->DoGetStateDebugName();
	}

	State* CreateState(const Transition& transition, StateMachine* ownerStateMachine, size_t stackDepth)
	{
		State* state = transition.GetStateFactory().AllocateState();
		InitState(state, ownerStateMachine, stackDepth);
		return state;
	}

	void DestroyState(State* state)
	{
		HSM_DELETE(state);
	}

	void InvokeStateOnEnter(const Transition& transition, State* state)
	{
		transition.GetStateFactory().InvokeStateOnEnter(state, transition.GetStateArgs());
	}

	void InvokeStateOnExit(State* state)
	{
		state->OnExit();
	}
}

StateMachine::StateMachine()
	: mOwner(0)
	, mDebugLevel(0)
{
}

StateMachine::~StateMachine()
{
	Shutdown(hsm_false);
}

void StateMachine::Shutdown(hsm_bool stop)
{
	if (stop)
		Stop();

	// Free any allocated states
	PopStatesToDepth(0, hsm_false);

	mOwner = 0;
	mInitialTransition = NoTransition();
	mDebugLevel = 0;
	mDebugName[0] = '\0';
}

void StateMachine::Stop()
{
	PopStatesToDepth(0);
	HSM_ASSERT(mStateStack.empty());
}

void StateMachine::SetDebugInfo(const hsm_char* debugName, size_t debugLevel)
{
	STRNCPY(mDebugName, debugName, HSM_DEBUG_NAME_MAXLEN);
	mDebugName[HSM_DEBUG_NAME_MAXLEN - 1] = '\0';
	SetDebugLevel(debugLevel);
}

void StateMachine::ProcessStateTransitions()
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
	while (keepProcessing)
	{
		keepProcessing = ProcessStateTransitionsOnce();
	}
}

void StateMachine::UpdateStates(HSM_STATE_UPDATE_ARGS)
{
	InnerToOuterIterator iter = BeginInnerToOuter();
	InnerToOuterIterator end = EndInnerToOuter();
	for ( ; iter != end; ++iter)
	{
		(*iter)->Update(HSM_STATE_UPDATE_ARGS_FORWARD);
	}
}

State* StateMachine::GetState(StateTypeId stateType)
{	
	for (size_t i = 0; i < mStateStack.size(); ++i)
	{
		State* state = mStateStack[i];
		if (state->GetStateType() == stateType)
			return state;
	}
	return 0;
}

State* StateMachine::GetStateAtDepth(size_t depth)
{
	if (depth >= mStateStack.size())
	{
		return 0;
	}

	return mStateStack[depth];
}

State* StateMachine::GetOuterState(StateTypeId stateType, size_t startDepth)
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

const State* StateMachine::GetOuterState(StateTypeId stateType, size_t startDepth) const
{
	return const_cast<StateMachine*>(this)->GetOuterState(stateType, startDepth);
}

State* StateMachine::GetInnerState(StateTypeId stateType, size_t startDepth)
{
	for (size_t i = startDepth; i < mStateStack.size(); ++i)
	{
		State* state = mStateStack[i];
		if (state->GetStateType() == stateType)
			return state;
	}
	return 0;
}

const State* StateMachine::GetInnerState(StateTypeId stateType, size_t startDepth) const
{
	return const_cast<StateMachine*>(this)->GetInnerState(stateType, startDepth);
}

void StateMachine::CreateAndPushInitialState(const Transition& transition)
{
	HSM_ASSERT(mStateStack.empty());
	State* initialState = detail::CreateState(transition, this, 0);
	HSM_LOG_TRANSITION(1, 0, HSM_TEXT("Init"), initialState);
	PushState(initialState);
	detail::InvokeStateOnEnter(transition, initialState);
}

void StateMachine::PopStatesToDepth(size_t depth, hsm_bool invokeOnExit)
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

hsm_bool StateMachine::ProcessStateTransitionsOnce()
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

void StateMachine::PushState(State* state)
{
	mStateStack.push_back(state);
}

void StateMachine::PopState()
{
	mStateStack.pop_back();
}

void StateMachine::Log(size_t minLevel, size_t numSpaces, const hsm_char* format, ...)
{
	if (mDebugLevel >= minLevel)
	{
		static hsm_char buffer[4096];
		int offset = SNPRINTF(buffer, sizeof(buffer), HSM_TEXT("HSM_%d_%s:%*s "), minLevel, mDebugName, numSpaces, "");

		va_list args;
		va_start(args, format);
		VSNPRINTF(buffer + offset, sizeof(buffer) - offset - 1, format, args);

		// Print to stdout
		HSM_PRINTF(HSM_TEXT("%s"), buffer);
	}
}

void StateMachine::LogTransition(size_t minLevel, size_t depth, const hsm_char* transType, State* state)
{
	Log(minLevel, depth, HSM_TEXT("%-8s: %s\n"), transType, state->GetStateDebugName());
}

} // namespace hsm
