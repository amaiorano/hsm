// drawing_hsms_clusters.cpp

#include "hsm.h"

using namespace hsm;

class MyOwner
{
public:
	MyOwner();
	void UpdateStateMachine();

private:
	bool IsDead() const { return false; }
	bool PressedJump() const { return false; }
	bool PressedShoot() const { return false; }
	bool PressedMove() const { return false; }
	bool PressedCrouch() const { return false; }

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
			if (Owner().PressedJump())
				return SiblingTransition<Jump>();

			if (Owner().PressedShoot())
				return SiblingTransition<Shoot>();

			return InnerEntryTransition<Locomotion_Stand>();
		}
	};

	struct Jump : BaseState
	{
		bool FinishedJumping() const { return false; }

		virtual Transition GetTransition()
		{
			if (FinishedJumping())
				return SiblingTransition<Locomotion>();

			return NoTransition();
		}
	};

	struct Shoot : BaseState
	{
		bool FinishedShooting() const { return false; }

		virtual Transition GetTransition()
		{
			if (FinishedShooting())
				return SiblingTransition<Locomotion>();

			return NoTransition();
		}
	};

	struct Locomotion_Stand : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().PressedMove())
				return SiblingTransition<Locomotion_Move>();

			if (Owner().PressedCrouch())
				return SiblingTransition<Locomotion_Crouch>();

			return NoTransition();
		}
	};

	struct Locomotion_Move : BaseState
	{
		virtual Transition GetTransition()
		{
			if (!Owner().PressedMove())
				return SiblingTransition<Locomotion_Stand>();

			if (Owner().PressedCrouch())
				return SiblingTransition<Locomotion_Crouch>();

			return NoTransition();
		}
	};

	struct Locomotion_Crouch : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().PressedMove())
				return SiblingTransition<Locomotion_Move>();

			if (Owner().PressedCrouch())
				return SiblingTransition<Locomotion_Stand>();

			return NoTransition();
		}
	};
};

MyOwner::MyOwner()
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
}
