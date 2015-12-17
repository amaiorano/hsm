// shared_states_related_owners.cpp

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

////////////////////// Character //////////////////////

class Character
{
public:
	void Update();

protected:
	friend struct SharedStates;
	StateMachine mStateMachine;
	AnimComponent mAnimComponent;
};

void Character::Update()
{
	// Update state machine
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();
}

struct SharedStates
{
	// These states can be shared by state machines with Character-derived owners

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
};

////////////////////// Hero //////////////////////

class Hero : public Character
{
public:
	Hero();

	// Public to simplify sample
	bool mAttack;

private:
	friend struct HeroStates;
};


struct HeroStates
{
	struct BaseState : StateWithOwner<Hero, SharedStates::BaseState>
	{
	};

	typedef SharedStates::PlayAnim_Done PlayAnim_Done;
	typedef SharedStates::PlayAnim PlayAnim;

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

	struct Attack : BaseState
	{
		virtual Transition GetTransition()
		{
			if (IsInInnerState<PlayAnim_Done>())
				return SiblingTransition<Stand>();

			return InnerEntryTransition<PlayAnim>(std::ref("Attack_1"), false);
		}
	};
};

Hero::Hero()
	: mAttack(false)
{
	mStateMachine.Initialize<HeroStates::Alive>(this);
	mStateMachine.SetDebugInfo("TestHsm", TraceLevel::Basic);
}


////////////////////// Enemy //////////////////////

class Enemy : public Character
{
public:
	Enemy();

	// Public to simplify sample
	bool mAttack;

private:
	friend struct EnemyStates;
};

struct EnemyStates
{
	struct BaseState : StateWithOwner<Enemy, SharedStates::BaseState>
	{
	};

	typedef SharedStates::PlayAnim_Done PlayAnim_Done;
	typedef SharedStates::PlayAnim PlayAnim;

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

	struct Attack : BaseState
	{
		virtual Transition GetTransition()
		{
			if (IsInInnerState<PlayAnim_Done>())
				return SiblingTransition<Stand>();

			return InnerEntryTransition<PlayAnim>(std::ref("Attack_1"), false);
		}
	};
};

Enemy::Enemy()
	: mAttack(false)
{
	mStateMachine.Initialize<EnemyStates::Alive>(this);
	mStateMachine.SetDebugInfo("TestHsm", TraceLevel::Basic);
}


////////////////////// main //////////////////////

int main()
{
	Hero hero;
	hero.Update();
	hero.mAttack = true;
	hero.Update();

	Enemy enemy;
	enemy.Update();
	enemy.mAttack = true;
	enemy.Update();
}
