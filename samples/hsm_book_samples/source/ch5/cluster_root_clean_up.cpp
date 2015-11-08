// cluster_root_clean_up.cpp

#include "hsm/statemachine.h"

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
		virtual Transition GetTransition()
		{
			if (Owner().IsHurt())
				return SiblingTransition<Hurt>();

			return InnerEntryTransition<Stand>();
		}
	};

	struct Hurt : BaseState
	{
	};

	struct Stand : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().ShouldGetOnLadder())
				return SiblingTransition<Ladder>();

			return NoTransition();
		}
	};

	struct Ladder : BaseState
	{
		virtual Transition GetTransition()
		{
			return InnerEntryTransition<Ladder_GetOn>();
		}
	};

	struct Ladder_GetOn : BaseState
	{
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
		virtual Transition GetTransition()
		{
			if (Owner().ShouldGetOffLadder())
				return SiblingTransition<Ladder_GetOff>();
			
			return NoTransition();
		}
	};

	struct Ladder_GetOff : BaseState
	{
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
