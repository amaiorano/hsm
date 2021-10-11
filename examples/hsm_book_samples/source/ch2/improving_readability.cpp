// improving_readability.cpp

#define HSM_USE_CPP_RTTI_IF_ENABLED 1

#include "hsm.h"

using namespace hsm;

struct MyStates
{
	struct First : State
	{
		DEFINE_HSM_STATE(First);

		virtual Transition GetTransition()
		{
			return SiblingTransition<Second>();
		}
	};

	struct Second : State
	{
		DEFINE_HSM_STATE(Second);

		virtual Transition GetTransition()
		{
			return SiblingTransition<Third>();
		}
	};

	struct Third : State
	{
		DEFINE_HSM_STATE(Third);

		virtual Transition GetTransition()
		{
			return NoTransition();
		}
	};
};

int main()
{
	StateMachine stateMachine;
	stateMachine.Initialize<MyStates::First>();
	stateMachine.SetDebugInfo("TestHsm", TraceLevel::Basic);
	stateMachine.ProcessStateTransitions();
}
