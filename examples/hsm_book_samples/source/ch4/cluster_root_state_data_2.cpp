// cluster_root_state_data_2.cpp

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

	struct JumpBaseState : BaseState
	{
		JumpBaseState() : mJumpState(NULL) {}

		virtual void OnEnter()
		{
			mJumpState = GetOuterState<Jump>();
			assert(mJumpState);
		}

		Jump& JumpState()
		{
			return *mJumpState;
		}

	private:
		Jump* mJumpState;
	};

	struct Jump_Up : JumpBaseState
	{
		virtual void OnEnter()
		{
			JumpBaseState::OnEnter();
			JumpState().mJumpValue1 = 1;
			JumpState().mJumpValue2 = 2.0f;
			JumpState().mJumpValue3 = true;
		}

		virtual Transition GetTransition()
		{
			return SiblingTransition<Jump_Down>();
		}
	};

	struct Jump_Down : JumpBaseState
	{
		virtual void OnEnter()
		{
			JumpBaseState::OnEnter();
			JumpState().mJumpValue1 = 2;
			JumpState().mJumpValue2 = 4.0f;
			JumpState().mJumpValue3 = false;
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
