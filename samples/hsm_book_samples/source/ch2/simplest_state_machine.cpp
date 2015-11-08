// simplest_state_machine.cpp

#include "hsm/statemachine.h"

struct First : hsm::State
{
};

int main()
{
	hsm::StateMachine stateMachine;
	stateMachine.Initialize<First>();
	stateMachine.ProcessStateTransitions();
	return 0;
}
