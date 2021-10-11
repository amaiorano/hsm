// deferred_transitions.cpp

#include "hsm.h"

using namespace hsm;

class Character
{
public:
	Character();
	void Update();

	// Public to simplify sample
	bool mCrouchInputPressed;

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


// Uncomment this macro to compile the "broken version" of this sample.
// The problem is that it results in an infinite loop between the two states.
// The non-broken version works by deferring the transition to the next "frame".
//#define BROKEN_VERSION

#ifdef BROKEN_VERSION

	struct Stand : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().mCrouchInputPressed)
			{
				return SiblingTransition<Crouch>();
			}

			return NoTransition();
		}
	};

	struct Crouch : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().mCrouchInputPressed)
				return SiblingTransition<Stand>();

			return NoTransition();
		}
	};

#else

	struct Stand : BaseState
	{
		virtual Transition GetTransition()
		{
			return mTransition;
		}

		virtual void Update()
		{
			if (Owner().mCrouchInputPressed)
				mTransition = SiblingTransition<Crouch>();
		}

		Transition mTransition;
	};

	struct Crouch : BaseState
	{
		virtual Transition GetTransition()
		{
			return mTransition;
		}

		virtual void Update()
		{
			if (Owner().mCrouchInputPressed)
				mTransition = SiblingTransition<Stand>();
		}

		Transition mTransition;
	};
#endif
};

Character::Character()
	: mCrouchInputPressed(false)
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

	printf(">>> Crouch!\n");
	character.mCrouchInputPressed = true;
	character.Update();
	character.Update();
	character.Update();
	character.Update();
	character.mCrouchInputPressed = false;
	character.Update();

	printf(">>> Stand!\n");
	character.mCrouchInputPressed = true;
	character.Update();
	character.mCrouchInputPressed = false;
	character.Update();

	printf(">>> Crouch!\n");
	character.mCrouchInputPressed = true;
	character.Update();
	character.mCrouchInputPressed = false;
	character.Update();
}
