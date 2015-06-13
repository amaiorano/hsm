// state_value_with.cpp

#include "hsm/statemachine.h"

using namespace hsm;

class PhysicsComponent
{
public:
	void SetSpeed(float speed) {} // Stub
	void Move() {} // Stub
};

class Character
{
public:
	Character();
	void Update();

	// Public to simplify sample
	bool m_inWater;
	bool m_move;
	bool m_crawl;

private:
	friend struct CharacterStates;
	StateMachine mStateMachine;

	PhysicsComponent mPhysicsComponent;
	hsm::StateValue<float> mSpeedScale; // [0,1]
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
			return InnerEntryTransition<OnGround>();
		}
	};

	struct OnGround : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().m_inWater)
				return SiblingTransition<Swim>();

			return InnerEntryTransition<Stand>();
		}
	};

	struct Stand : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().m_move)
				return SiblingTransition<Move>();

			return NoTransition();
		}
	};

	struct Move : BaseState
	{
		virtual Transition GetTransition()
		{
			if (!Owner().m_move)
				return SiblingTransition<Stand>();

			return InnerEntryTransition<Move_Walk>();
		}
	};

	struct Move_Walk : BaseState
	{
		virtual void OnEnter()
		{
			SetStateValue(Owner().mSpeedScale) = 1.0f; // Full speed when moving normally
		}

		virtual Transition GetTransition()
		{
			if (Owner().m_crawl)
				return SiblingTransition<Move_Crawl>();
			
			return NoTransition();
		}
	};

	struct Move_Crawl : BaseState
	{
		virtual void OnEnter()
		{
			SetStateValue(Owner().mSpeedScale) = 0.5f; // Half speed when crawling
		}

		virtual Transition GetTransition()
		{
			if (!Owner().m_crawl)
				return SiblingTransition<Move_Walk>();

			return NoTransition();
		}
	};

	struct Swim : BaseState
	{
		virtual void OnEnter()
		{
			SetStateValue(Owner().mSpeedScale) = 0.3f; // ~1/3 speed when swimming
		}

		virtual Transition GetTransition()
		{
			if (!Owner().m_inWater)
				return SiblingTransition<OnGround>();

			return NoTransition();
		}
	};
};

Character::Character()
	: m_inWater(false)
	, m_move(false)
	, m_crawl(false)
	, mSpeedScale(0.0f) // By default we don't move
{
	mStateMachine.Initialize<CharacterStates::Alive>(this);
	mStateMachine.SetDebugInfo("TestHsm", TraceLevel::Basic);
}

void Character::Update()
{
	// Update state machine
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();

	// Move character
	const float MAX_SPEED = 100.0f;
	float currSpeed = mSpeedScale * MAX_SPEED;
	mPhysicsComponent.SetSpeed(currSpeed);
	mPhysicsComponent.Move();

	printf("Current speed: %f\n", currSpeed);
}

int main()
{
	Character character;
	character.Update();

	character.m_move = true;
	character.Update();

	character.m_crawl = true;
	character.Update();

	character.m_inWater = true;
	character.Update();

	character.m_inWater = false;
	character.m_move = false;
	character.m_crawl = false;
	character.Update();
}
