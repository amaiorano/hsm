// ownership_basic_usage.cpp

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
	struct First : State
	{
		virtual Transition GetTransition()
		{
			MyOwner* owner = reinterpret_cast<MyOwner*>(GetStateMachine().GetOwner());

			if (owner->GetPlaySequence())
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
			MyOwner* owner = reinterpret_cast<MyOwner*>(GetStateMachine().GetOwner());

			if (owner->GetPlaySequence())
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

MyOwner::MyOwner()
{
	mPlaySequence = false;
	mStateMachine.Initialize<MyStates::First>(this); //*** Note that we pass 'this' as our owner
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
