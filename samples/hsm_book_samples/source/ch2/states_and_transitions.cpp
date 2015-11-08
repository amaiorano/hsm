// states_and_transitions.cpp

#include "hsm/statemachine.h"

using namespace hsm;

struct Third : State
{
	virtual Transition GetTransition()
	{
		return NoTransition();
	}
};

struct Second : State
{
	virtual Transition GetTransition()
	{
		return SiblingTransition<Third>();
	}
};

struct First : State
{
	virtual Transition GetTransition()
	{
		return SiblingTransition<Second>();
	}
};

int main()
{
	StateMachine stateMachine;
	stateMachine.Initialize<First>();
	stateMachine.SetDebugInfo("TestHsm", TraceLevel::Basic);
	stateMachine.ProcessStateTransitions();
	return 0;
}
