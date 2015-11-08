// done_states.cpp

#include "hsm/statemachine.h"

using namespace hsm;

class Character
{
public:
	Character();
	void Update();

	// Public to simplify sample
	bool mOpenDoor;

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
			if (Owner().mOpenDoor)
			{
				Owner().mOpenDoor = false;
				return SiblingTransition<OpenDoor>();
			}
			
			return NoTransition();
		}
	};

	struct OpenDoor : BaseState
	{
		virtual Transition GetTransition()
		{
			if (IsInInnerState<OpenDoor_Done>())
				return SiblingTransition<Stand>();

			return InnerEntryTransition<OpenDoor_GetIntoPosition>();
		}
	};

	struct OpenDoor_GetIntoPosition : BaseState
	{
		bool IsInPosition() const { return true; } // Stub

		virtual Transition GetTransition()
		{
			if (IsInPosition())
				return SiblingTransition<OpenDoor_PlayOpenAnim>();

			return NoTransition();
		}
	};

	struct OpenDoor_PlayOpenAnim : BaseState
	{
		bool IsAnimDone() const { return true; } // Stub

		virtual Transition GetTransition()
		{
			if (IsAnimDone())
				return SiblingTransition<OpenDoor_Done>();

			return NoTransition();
		}
	};

	struct OpenDoor_Done : BaseState
	{
	};
};

Character::Character()
	: mOpenDoor(false)
{
	mStateMachine.Initialize<CharacterStates::Alive>(this);
	mStateMachine.SetDebugInfo("TestHsm", TraceLevel::Basic);
}

void Character::Update()
{
	printf(">>> Character::Update\n");

	// Update state machine
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();
}

int main()
{
	Character character;

	character.Update();

	character.mOpenDoor = true;
	character.Update();
}
