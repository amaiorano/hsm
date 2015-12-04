// inner_transition.cpp

#include "hsm.h"

using namespace hsm;

class MyOwner
{
public:
	MyOwner();
	void UpdateStateMachine();

	void Die() { mDead = true; }
	void SetMove(bool enable) { mMove = enable; }

private:
	bool IsDead() const { return mDead; }
	bool PressedMove() const { return mMove; }

	bool mDead;
	bool mMove;

	friend struct MyStates;
	StateMachine mStateMachine;
};

struct MyStates
{
	struct BaseState : StateWithOwner<MyOwner>
	{
	};

	struct Alive : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().IsDead())
				return SiblingTransition<Dead>();

			return InnerEntryTransition<Locomotion>();
		}
	};

	struct Dead : BaseState
	{
		virtual Transition GetTransition()
		{
			return NoTransition();
		}
	};

	struct Locomotion : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().PressedMove())
				return InnerTransition<Move>();
			else
				return InnerTransition<Stand>();
		}
	};

	struct Stand : BaseState
	{
	};

	struct Move : BaseState
	{
	};
};

MyOwner::MyOwner()
	: mDead(false)
	, mMove(false)
{
	mStateMachine.Initialize<MyStates::Alive>(this);
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

	printf("Set Move = true\n");
	myOwner.SetMove(true);
	myOwner.UpdateStateMachine();

	printf("Set Move = false\n");
	myOwner.SetMove(false);
	myOwner.UpdateStateMachine();
}
