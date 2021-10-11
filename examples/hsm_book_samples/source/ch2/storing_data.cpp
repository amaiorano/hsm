// storing_data.cpp
#include <cstdio>
#include <string>
#include "hsm.h"
using namespace hsm;

class MyOwner
{
public:
	MyOwner();
	void UpdateStateMachine();

private:
	friend struct MyStates; //*** All states can access MyOwner's private members
	StateMachine mStateMachine;
};

struct Foo
{
	Foo() { printf(">>> Foo created\n"); }
	~Foo() { printf(">>>Foo destroyed\n"); }
};

struct MyStates
{
	struct BaseState : StateWithOwner<MyOwner>
	{
	};

	struct First : BaseState
	{
		virtual Transition GetTransition()
		{
			return SiblingTransition<Second>();
		}

		Foo mFoo; //*** State data member
	};

	struct Second : BaseState
	{
		virtual Transition GetTransition()
		{
			return NoTransition();
		}
	};
};

MyOwner::MyOwner()
{
	mStateMachine.Initialize<MyStates::First>(this);
	mStateMachine.SetDebugInfo("TestHsm", TraceLevel::Basic);
}

void MyOwner::UpdateStateMachine()
{
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();
}

int main()
{
	MyOwner myOwner;
	myOwner.UpdateStateMachine();
}
