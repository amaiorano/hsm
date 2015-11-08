// update_states.cpp

#include <cstdio>
#include "hsm/statemachine.h"
using namespace hsm;

bool gPlaySequence = false;

struct MyStates
{
	struct First : State
	{
		virtual Transition GetTransition()
		{
			if (gPlaySequence)
				return SiblingTransition<Second>();
			
			return NoTransition();
		}

		virtual void Update()
		{
			printf("First::Update\n");
		}
	};

	struct Second : State
	{
		virtual Transition GetTransition()
		{
			if (gPlaySequence)
				return SiblingTransition<Third>();
			
			return NoTransition();
		}

		virtual void Update()
		{
			printf("Second::Update\n");
		}
	};

	struct Third : State
	{
		virtual Transition GetTransition()
		{
			return NoTransition();
		}

		virtual void Update()
		{
			printf("Third::Update\n");
		}
	};
};

int main()
{
	StateMachine stateMachine;
	stateMachine.Initialize<MyStates::First>();
	stateMachine.SetDebugInfo("TestHsm", TraceLevel::Basic);

	stateMachine.ProcessStateTransitions();
	stateMachine.UpdateStates();

	stateMachine.ProcessStateTransitions();
	stateMachine.UpdateStates();

	gPlaySequence = true;

	stateMachine.ProcessStateTransitions();
	stateMachine.UpdateStates();

	stateMachine.ProcessStateTransitions();
	stateMachine.UpdateStates();
}
