// state_overrides.cpp

#include "hsm/statemachine.h"

using namespace hsm;

////////////////////// Character //////////////////////

class Character
{
public:
	Character();
	void Update();

	// Public to simplify sample
	bool mAttack;
	bool mJump;

protected:
	friend struct CharacterStates;
	StateMachine mStateMachine;
};

struct CharacterStates
{
	struct BaseState : StateWithOwner<Character>
	{
	};

	struct PlayAnim_Done : BaseState
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
			if (Owner().mAttack)
			{
				Owner().mAttack = false;
				return SiblingTransition(GetStateOverride<Attack>());
			}

			if (Owner().mJump)
			{
				Owner().mJump = false;
				return SiblingTransition(GetStateOverride<Jump>());
			}

			return NoTransition();
		}
	};

	struct Attack : BaseState
	{
		virtual Transition GetTransition()
		{
			return SiblingTransition<Stand>();
		}
	};

	struct Jump : BaseState
	{
		virtual Transition GetTransition()
		{
			return SiblingTransition<Stand>();
		}
	};
};

Character::Character()
	: mAttack(false)
	, mJump(false)

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


////////////////////// Hero //////////////////////

class Hero : public Character
{
public:
	Hero();

private:
	friend struct HeroStates;
};

struct HeroStates
{
	struct BaseState : StateWithOwner<Hero, CharacterStates::BaseState>
	{
	};

	struct Attack : BaseState
	{
		virtual Transition GetTransition()
		{
			return SiblingTransition<CharacterStates::Stand>();
		}
	};

	struct Jump : BaseState
	{
		virtual Transition GetTransition()
		{
			return SiblingTransition<CharacterStates::Stand>();
		}
	};
};

Hero::Hero()
{
	mStateMachine.AddStateOverride<CharacterStates::Attack, HeroStates::Attack>();
	mStateMachine.AddStateOverride<CharacterStates::Jump, HeroStates::Jump>();
}


////////////////////// Enemy //////////////////////

class Enemy : public Character
{
public:
	Enemy();

private:
	friend struct EnemyStates;
};


struct EnemyStates
{
	struct BaseState : StateWithOwner<Enemy, CharacterStates::BaseState>
	{
	};

	struct Attack : BaseState
	{
		virtual Transition GetTransition()
		{
			return SiblingTransition<CharacterStates::Stand>();
		}
	};
};

Enemy::Enemy()
{
	mStateMachine.AddStateOverride<CharacterStates::Attack, EnemyStates::Attack>();
}


////////////////////// main //////////////////////

int main()
{
	Hero hero;
	hero.Update();
	hero.mAttack = true;
	hero.Update();
	hero.mJump = true;
	hero.Update();

	printf("\n");

	Enemy enemy;
	enemy.Update();
	enemy.mAttack = true;
	enemy.Update();
	enemy.mJump = true;
	enemy.Update();
}
