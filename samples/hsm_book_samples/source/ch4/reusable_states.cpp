// reusable_states.cpp

#include "hsm.h"

using namespace hsm;

class AnimComponent
{
public:
	AnimComponent() : mLoop(false) {}
	void PlayAnim(const char* name, bool loop)
	{
		printf(">>> PlayAnim: %s, looping: %s\n", name, loop ? "true" : "false");
		mLoop = loop;
	}

	bool IsFinished() const { return !mLoop; }

private:
	bool mLoop;
};

class Character
{
public:
	Character();
	void Update();

	// Public to simplify sample
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

	struct PlayAnim_Done : BaseState
	{
	};

	struct PlayAnim : BaseState
	{
		virtual void OnEnter(const char* animName
			, bool loop = true
			, const Transition& doneTransition = SiblingTransition<PlayAnim_Done>()
			)
		{
			Owner().mAnimComponent.PlayAnim(animName, loop);
			mDoneTransition = doneTransition;
		}

		virtual Transition GetTransition()
		{
			if (Owner().mAnimComponent.IsFinished())
				return mDoneTransition;
			
			return NoTransition();
		}

		Transition mDoneTransition;
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
			if (Owner().mAttack)
			{
				Owner().mAttack = false;
				return SiblingTransition<Attack>();
			}

			return NoTransition();
		}
	};

	template <typename T>
	struct PrintType;

	struct Attack : BaseState
	{
		virtual Transition GetTransition()
		{
			if (IsInInnerState<PlayAnim_Done>())
				return SiblingTransition<Stand>();

			return InnerEntryTransition<PlayAnim>(
				std::ref("Attack_1"), false, SiblingTransition<PlayAnim>(
					std::ref("Attack_2"), false, SiblingTransition<PlayAnim>(
						std::ref("Attack_3"), false)));
		}
	};
};

Character::Character()
	: mAttack(false)
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

	character.mAttack = true;
	character.Update();
}
