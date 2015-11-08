// process_state_transitions.cpp

#include <cstdio>
#include "hsm/statemachine.h"
using namespace hsm;

bool gStartOver = false;

struct MyStates
{
	struct First : State
	{
		virtual void OnEnter()
		{
			gStartOver = false;
		}

		virtual Transition GetTransition()
		{
			return SiblingTransition<Second>();
		}
	};

	struct Second : State
	{
		virtual Transition GetTransition()
		{
			return SiblingTransition<Third>();
		}
	};

	struct Third : State
	{
		virtual Transition GetTransition()
		{
			if (gStartOver)
				return SiblingTransition<First>();

			return NoTransition();
		}
	};
};

int main()
{
	StateMachine stateMachine;
	stateMachine.Initialize<MyStates::First>();
	stateMachine.SetDebugInfo("TestHsm", TraceLevel::Basic);
	
	printf(">>> First ProcessStateTransitions\n");
	stateMachine.ProcessStateTransitions();

	printf(">>> Second ProcessStateTransitions\n");
	stateMachine.ProcessStateTransitions();

	gStartOver = true;
	printf(">>> Third ProcessStateTransitions\n");
	stateMachine.ProcessStateTransitions();

	printf(">>> Fourth ProcessStateTransitions\n");
	stateMachine.ProcessStateTransitions();
}
