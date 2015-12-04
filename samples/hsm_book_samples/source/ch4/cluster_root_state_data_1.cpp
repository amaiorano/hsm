// cluster_root_state_data_1.cpp

#include "hsm.h"

using namespace hsm;

class Character
{
public:
	Character();
	void Update();

	bool mJump;

private:
	friend struct CharacterStates;
	StateMachine mStateMachine;
};

struct CharacterStates
{
	struct BaseState : StateWithOwner<Character>
	{
	};

	struct Alive : BaseState
	{
		virtual Transition GetTransition()
		{
			return InnerEntryTransition<Stand>();
		}
	};

	struct Stand : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().mJump)
			{
				Owner().mJump = false;
				return SiblingTransition<Jump>();
			}
			
			return NoTransition();
		}
	};

	struct Jump : BaseState
	{
		int mJumpValue1;
		float mJumpValue2;
		bool mJumpValue3;

		Jump() : mJumpValue1(0), mJumpValue2(0.0f), mJumpValue3(false) { }

		virtual Transition GetTransition()
		{
			if (IsInInnerState<Jump_Done>())
				return SiblingTransition<Stand>();

			return InnerEntryTransition<Jump_Up>();
		}
	};

	struct Jump_Up : BaseState
	{
		virtual void OnEnter()
		{
			GetOuterState<Jump>()->mJumpValue1 = 1;
			GetOuterState<Jump>()->mJumpValue2 = 2.0f;
			GetOuterState<Jump>()->mJumpValue3 = true;
		}

		virtual Transition GetTransition()
		{
			return SiblingTransition<Jump_Down>();
		}
	};

	struct Jump_Down : BaseState
	{
		virtual void OnEnter()
		{
			GetOuterState<Jump>()->mJumpValue1 = 2;
			GetOuterState<Jump>()->mJumpValue2 = 4.0f;
			GetOuterState<Jump>()->mJumpValue3 = false;
		}

		virtual Transition GetTransition()
		{
			return SiblingTransition<Jump_Done>();
		}
	};

	struct Jump_Done : BaseState
	{
	};
};

Character::Character()
	: mJump(false)
{
	mStateMachine.Initialize<CharacterStates::Alive>(this);
	mStateMachine.SetDebugInfo("TestHsm", TraceLevel::Basic);
}

void Character::Update()
{
	// Update state machine
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();
}

int main()
{
	Character character;
	character.Update();
	character.mJump = true;
	character.Update();
}
