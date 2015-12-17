// shared_states_unrelated_owners.cpp

#include "hsm.h"

using namespace hsm;

struct SharedStates
{
	template <typename BaseState>
	struct PlayAnim_Done : BaseState
	{
	};

	template <typename BaseState>
	struct PlayAnim : BaseState
	{
		using BaseState::Owner;

		virtual void OnEnter(const char* animName
			, bool loop = true
			, const Transition& doneTransition = SiblingTransition< PlayAnim_Done<BaseState> >()
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


////////////////////// Hero //////////////////////

class Hero
{
public:
	Hero();
	void Update();

	// Public to simplify sample
	bool mAttack;

private:
	friend struct SharedStates;
	friend struct HeroStates;
	StateMachine mStateMachine;

	AnimComponent mAnimComponent;
};

struct HeroStates
{
	struct BaseState : StateWithOwner<Hero>
	{
	};

	typedef SharedStates::PlayAnim_Done<BaseState> PlayAnim_Done;
	typedef SharedStates::PlayAnim<BaseState> PlayAnim;

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

void Hero::Update()
{
	// Update state machine
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();
}

////////////////////// Enemy //////////////////////

class Enemy
{
public:
	Enemy();
	void Update();

	// Public to simplify sample
	bool mAttack;

private:
	friend struct SharedStates;
	friend struct EnemyStates;
	StateMachine mStateMachine;

	AnimComponent mAnimComponent;
};

struct EnemyStates
{
	struct BaseState : StateWithOwner<Enemy>
	{
	};

	typedef SharedStates::PlayAnim_Done<BaseState> PlayAnim_Done;
	typedef SharedStates::PlayAnim<BaseState> PlayAnim;

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

void Enemy::Update()
{
	// Update state machine
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();
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
