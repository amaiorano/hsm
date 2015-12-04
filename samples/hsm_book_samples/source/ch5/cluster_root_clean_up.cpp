// cluster_root_clean_up.cpp

#include "hsm.h"

using namespace hsm;

class Character
{
public:
	Character();
	void Update();

private:
	bool IsHurt() const { return false; }
	bool ShouldGetOnLadder() const { return true; }
	bool ShouldGetOffLadder() const { return false; }
	void AttachToLadder() {}
	void DetachFromLadder() {}

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
		DEFINE_HSM_STATE(Alive);

		virtual Transition GetTransition()
		{
			if (Owner().IsHurt())
				return SiblingTransition<Hurt>();

			return InnerEntryTransition<Stand>();
		}
	};

	struct Hurt : BaseState
	{
		DEFINE_HSM_STATE(Hurt);
	};

	struct Stand : BaseState
	{
		DEFINE_HSM_STATE(Stand);

		virtual Transition GetTransition()
		{
			if (Owner().ShouldGetOnLadder())
				return SiblingTransition<Ladder>();

			return NoTransition();
		}
	};

	struct Ladder : BaseState
	{
		DEFINE_HSM_STATE(Ladder);

		virtual Transition GetTransition()
		{
			return InnerEntryTransition<Ladder_GetOn>();
		}
	};

	struct Ladder_GetOn : BaseState
	{
		DEFINE_HSM_STATE(Ladder_GetOn);

		virtual void OnEnter()
		{
			Owner().AttachToLadder();
		}

		virtual Transition GetTransition()
		{
			return SiblingTransition<Ladder_OnLadder>();
		}
	};

	struct Ladder_OnLadder : BaseState
	{
		DEFINE_HSM_STATE(Ladder_OnLadder);

		virtual Transition GetTransition()
		{
			if (Owner().ShouldGetOffLadder())
				return SiblingTransition<Ladder_GetOff>();
			
			return NoTransition();
		}
	};

	struct Ladder_GetOff : BaseState
	{
		DEFINE_HSM_STATE(Ladder_GetOff);

		virtual void OnEnter()
		{
			Owner().DetachFromLadder();
		}
	};
};

Character::Character()
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
}
