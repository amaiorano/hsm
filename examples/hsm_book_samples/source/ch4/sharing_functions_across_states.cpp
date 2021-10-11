// sharing_functions_across_states.cpp

#include "hsm.h"

using namespace hsm;

class Character
{
public:
	Character();
	void Update();

	// Public to simplify sample
	bool mMove;
	bool mJump;

private:
	friend struct CharacterStates;
	StateMachine mStateMachine;
};

struct CharacterStates
{
	// Example: utility functions in base-most state to share across all states
	struct BaseState : StateWithOwner<Character>
	{
		void ClearJump() { Owner().mJump = false; }
	};

	struct Alive : BaseState
	{
		virtual Transition GetTransition()
		{
			//return InnerEntryTransition<Locomotion>();
			return InnerEntryTransition<JumpAndMove>();
		}
	};

	// Example: utility functions for a subset of states
	struct LocomotionBaseState : BaseState
	{
		bool ShouldJump() const
		{
			return Owner().mJump;
		}

		bool ShouldMove() const
		{
			// Jumping has priority over moving
			return !ShouldJump() && Owner().mMove;
		}

		bool ShouldStand() const
		{
			return !ShouldJump() && !ShouldMove();
		}
	};
	
	struct Locomotion : LocomotionBaseState
	{
		virtual Transition GetTransition()
		{
			return InnerEntryTransition<Selector>();
		}
	};

	struct Selector : LocomotionBaseState
	{
		virtual Transition GetTransition()
		{
			if (ShouldJump())
				return SiblingTransition<Jump>();

			if (ShouldMove())
				return SiblingTransition<Move>();

			assert(ShouldStand());
			return SiblingTransition<Stand>();
		}
	};

	struct Stand : LocomotionBaseState
	{
		virtual Transition GetTransition()
		{
			if (!ShouldStand())
				return SiblingTransition<Selector>();

			return NoTransition();
		}
	};

	struct Move : LocomotionBaseState
	{
		virtual Transition GetTransition()
		{
			if (!ShouldMove())
				return SiblingTransition<Selector>();

			return NoTransition();
		}
	};

	struct Jump : LocomotionBaseState
	{
		virtual Transition GetTransition()
		{
			if (!ShouldJump())
				return SiblingTransition<Selector>();

			return NoTransition();
		}
	};

	// Example: sharing utility functions from multiple base states by
	// chaining template classes
	template <typename BaseType = BaseState>
	struct JumpBaseState : BaseType
	{
		using BaseType::Owner;

		void ClearJump() { Owner().mJump = false; }
	};

	template <typename BaseType = BaseState>
	struct MoveBaseState : BaseType
	{
		using BaseType::Owner;

		void ClearMove() { Owner().mMove = false; }
	};

	struct JumpAndMove : JumpBaseState< MoveBaseState<> >
	{
		virtual void OnEnter()
		{
			ClearJump();
			ClearMove();
		}
	};
};

Character::Character()
	: mMove(false)
	, mJump(false)
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

	character.mMove = true;
	character.Update();

	character.mJump = true;
	character.Update();

	character.mJump = false;
	character.Update();

	character.mMove = false;
	character.Update();
}
