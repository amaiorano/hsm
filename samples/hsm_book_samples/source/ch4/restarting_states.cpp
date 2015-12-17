// restarting_states.cpp

#include "hsm.h"

using namespace hsm;

class AnimComponent
{
public:
	AnimComponent() {}
	void PlayAnim(const char* name)
	{
		printf(">>> PlayAnim: %s\n", name);
	}

	bool IsFinished() const { return false; } // Stub

	// Return true if input event was processed in animation
	bool PollEvent(const char*) { return true; } // Stub
};

class Character
{
public:
	Character();
	void Update();

	// Public to simplify sample
	bool mMove;
	bool mAttack;

private:
	friend struct CharacterStates;
	StateMachine mStateMachine;

	AnimComponent mAnimComponent;
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
			return InnerEntryTransition<Locomotion>();
		}
	};

	struct Locomotion : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().mAttack)
			{
				// Start attack sequence with combo index 0
				return SiblingTransition<Attack>(0);
			}

			return InnerEntryTransition<Stand>();
		}
	};

	struct Stand : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().mMove)
				return SiblingTransition<Move>();

			return NoTransition();
		}
	};

	struct Move : BaseState
	{
		virtual Transition GetTransition()
		{
			if (!Owner().mMove)
				return SiblingTransition<Stand>();

			return NoTransition();
		}
	};

	struct Attack : BaseState
	{
		virtual void OnEnter(int comboIndex)
		{
			Owner().mAttack = false;

			mComboIndex = comboIndex;

			static const char* AttackAnim[] =
			{
				"Attack_1",
				"Attack_2",
				"Attack_3"
			};
			assert(mComboIndex < sizeof(AttackAnim) / sizeof(AttackAnim[0]));

			Owner().mAnimComponent.PlayAnim(AttackAnim[mComboIndex]);
		}

		virtual Transition GetTransition()
		{
			// Check if player can chain next attack
			if (Owner().mAttack
				&& mComboIndex < 2
				&& Owner().mAnimComponent.PollEvent("CanChainCombo"))
			{
				// Restart state with next combo index
				return SiblingTransition<Attack>(mComboIndex + 1);
			}

			if (Owner().mAnimComponent.IsFinished())
				return SiblingTransition<Locomotion>();

			return NoTransition();
		}

		virtual void Update()
		{
			printf(">>> Attacking: %d\n", mComboIndex);
		}

		int mComboIndex;
	};
};

Character::Character()
	: mMove(false)
	, mAttack(false)
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

	character.mMove = true;
	character.Update();

	// First attack
	character.mAttack = true;
	character.Update();
	character.Update();
	character.Update();

	// Second attack combo
	character.mAttack = true;
	character.Update();
	character.Update();
	character.Update();

	// Third attack combo
	character.mAttack = true;
	character.Update();
	character.Update();
	character.Update();

	// Another attack has no effect (reached our max combo of 3)
	character.mAttack = true;
	character.Update();
}
