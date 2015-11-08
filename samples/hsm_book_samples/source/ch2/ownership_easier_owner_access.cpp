// ownership_easier_owner_access.cpp

#include <cstdio>
#include "hsm/statemachine.h"
using namespace hsm;

class MyOwner
{
public:
	MyOwner();
	void UpdateStateMachine();
	void PlaySequence();
	bool GetPlaySequence() const;

private:
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
			if (Owner().GetPlaySequence())
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
			if (Owner().GetPlaySequence())
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

bool MyOwner::GetPlaySequence() const
{
	return mPlaySequence;
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
