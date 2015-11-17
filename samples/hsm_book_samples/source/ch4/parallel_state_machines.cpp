// parallel_state_machines.cpp

#include "hsm.h"

using namespace hsm;

class Hero
{
public:
	Hero();
	void Update();

	// Public to simplify sample
	bool mMove;
	bool mJump;
	bool mReload;

private:

	// Functions that simulate playing an animation that always lasts 3 frames
	void PlayAnim(const char*) { mAnimFrame = 0; }
	bool IsAnimDone() const { return mAnimFrame >= 2; }

	void ReloadWeapon() { printf(">>> WEAPON RELOADED!\n"); }

	friend struct HeroFullBodyStates;
	friend struct HeroUpperBodyStates;
	StateMachine mStateMachines[2];

	hsm::StateValue<bool> mUpperBodyEnabled;

	int mAnimFrame;
};

struct HeroFullBodyStates
{
	struct BaseState : StateWithOwner<Hero>
	{
	};

	struct Alive : BaseState
	{
		virtual void OnEnter()
		{
			// By default, upper body is enabled
			SetStateValue(Owner().mUpperBodyEnabled) = true;
		}

		virtual Transition GetTransition()
		{
			return InnerEntryTransition<Stand>();
		}
	};

	struct Stand : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().mMove)
			{
				return SiblingTransition<Move>();
			}

			if (Owner().mJump)
			{
				Owner().mJump = false;
				return SiblingTransition<Jump>();
			}

			return NoTransition();
		}
	};

	struct Move : BaseState
	{
		virtual Transition GetTransition()
		{
			if (!Owner().mMove)
			{
				return SiblingTransition<Stand>();
			}

			if (Owner().mJump)
			{
				Owner().mJump = false;
				return SiblingTransition<Jump>();
			}
			
			return NoTransition();
		}
	};

	struct Jump : BaseState
	{
		virtual void OnEnter()
		{
			// Don't allow upper body to do anything while jumping
			SetStateValue(Owner().mUpperBodyEnabled) = false;
			Owner().PlayAnim("Jump");
		}

		virtual Transition GetTransition()
		{
			if (Owner().IsAnimDone())
			{
				if (!Owner().mMove)
				{
					return SiblingTransition<Stand>();
				}
				else
				{
					return SiblingTransition<Move>();
				}
			}

			return NoTransition();
		}
	};
};

struct HeroUpperBodyStates
{
	struct BaseState : StateWithOwner < Hero >
	{
	};

	struct Disabled : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().mUpperBodyEnabled)
				return SiblingTransition<Enabled>();

			return NoTransition();
		}
	};

	struct Enabled : BaseState
	{
		virtual Transition GetTransition()
		{
			if (!Owner().mUpperBodyEnabled)
				return SiblingTransition<Disabled>();

			return InnerEntryTransition<Idle>();
		}
	};

	struct Idle : BaseState
	{
		virtual void OnEnter()
		{
			Owner().PlayAnim("Idle");
		}

		virtual Transition GetTransition()
		{
			if (Owner().mReload)
			{
				Owner().mReload = false;
				return SiblingTransition<Reload>();
			}

			return NoTransition();
		}
	};

	struct Reload : BaseState
	{
		virtual Transition GetTransition()
		{
			if (IsInInnerState<Reload_Done>())
				return SiblingTransition<Idle>();

			return InnerEntryTransition<Reload_PlayAnim>();
		}
	};

	struct Reload_PlayAnim : BaseState
	{
		virtual void OnEnter()
		{
			Owner().PlayAnim("Reload");
		}

		virtual Transition GetTransition()
		{
			if (Owner().IsAnimDone())
				return SiblingTransition<Reload_Done>();

			return NoTransition();
		}
	};

	struct Reload_Done : BaseState
	{
		virtual void OnEnter()
		{
			// Only once we're done the anim to we actually reload our weapon
			Owner().ReloadWeapon();
		}
	};
};

Hero::Hero()
	: mMove(false)
	, mJump(false)
	, mReload(false)
	, mUpperBodyEnabled(false)

{
	mStateMachines[0].Initialize<HeroFullBodyStates::Alive>(this);
	mStateMachines[0].SetDebugInfo("FullBody ", TraceLevel::Basic);

	mStateMachines[1].Initialize<HeroUpperBodyStates::Disabled>(this);
	mStateMachines[1].SetDebugInfo("UpperBody", TraceLevel::Basic);
}

void Hero::Update()
{
	// Update state machines
	for (int i = 0; i < 2; ++i)
	{
		mStateMachines[i].ProcessStateTransitions();
		mStateMachines[i].UpdateStates();
	}

	++mAnimFrame;
}

////////////////////// main //////////////////////

int main()
{
	Hero hero;
	
	int whichUpdate = 0;

	printf(">>> Update %d\n", whichUpdate++);
	hero.Update();

	printf(">>> Input: Reload\n");
	hero.mReload = true;

	printf(">>> Update %d\n", whichUpdate++);
	hero.Update();

	printf(">>> Input: Move\n");
	hero.mMove = true;

	printf(">>> Update %d\n", whichUpdate++);
	hero.Update();

	printf(">>> Update %d\n", whichUpdate++);
	hero.Update();

	printf(">>> Input: Reload\n");
	hero.mReload = true;

	printf(">>> Update %d\n", whichUpdate++);
	hero.Update();

	printf(">>> Input: Jump\n");
	hero.mJump = true;

	printf(">>> Update %d\n", whichUpdate++);
	hero.Update();

	printf(">>> Update %d\n", whichUpdate++);
	hero.Update();

	printf(">>> Update %d\n", whichUpdate++);
	hero.Update();
}
