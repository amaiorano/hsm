// ownership_access_owner_privates.cpp

#include <cstdio>
#include "hsm.h"
using namespace hsm;

class MyOwner
{
public:
	MyOwner();
	void UpdateStateMachine();

	void PlaySequence();

private:
	friend struct MyStates; //*** All states can access MyOwner's private members

	StateMachine mStateMachine;
	bool mPlaySequence;
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
			if (Owner().mPlaySequence) //*** Access one of owner's private members
				return SiblingTransition<Second>();

			return NoTransition();
		}

		virtual void Update()
		{
			printf("First::Update\n");
		}
	};

	struct Second : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().mPlaySequence) //*** Access one of owner's private members
				return SiblingTransition<Third>();

			return NoTransition();
		}

		virtual void Update()
		{
			printf("Second::Update\n");
		}
	};

	struct Third : BaseState
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

MyOwner::MyOwner()
{
	mPlaySequence = false;
	mStateMachine.Initialize<MyStates::First>(this);
	mStateMachine.SetDebugInfo("TestHsm", TraceLevel::Basic);
}

void MyOwner::UpdateStateMachine()
{
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();
}

void MyOwner::PlaySequence()
{
	mPlaySequence = true;
}

int main()
{
	MyOwner myOwner;

	myOwner.UpdateStateMachine();
	myOwner.UpdateStateMachine();

	myOwner.PlaySequence();

	myOwner.UpdateStateMachine();
	myOwner.UpdateStateMachine();
}
