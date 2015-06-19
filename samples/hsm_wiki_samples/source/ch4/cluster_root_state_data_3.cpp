// cluster_root_state_data_3.cpp

#include "hsm/statemachine.h"

using namespace hsm;

class Character
{
public:
	Character();
	void Update();

	bool mJump;

private:
	friend struct CharacterStates;
	StateMachine mStateMachine;
};

struct CharacterStates
{
	template <typename RootStateType, typename BaseStateType>
	struct ClusterBaseState : BaseStateType
	{
		ClusterBaseState() : mRootState(NULL) {}

		virtual void OnEnter()
		{
			mRootState = GetOuterState<RootStateType>();
			assert(mRootState);
		}

		RootStateType& ClusterRootState()
		{
			return *mRootState;
		}

	private:
		RootStateType* mRootState;
	};

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
		int mJumpValue1;
		float mJumpValue2;
		bool mJumpValue3;

		Jump() : mJumpValue1(0), mJumpValue2(0.0f), mJumpValue3(false) { }

		virtual Transition GetTransition()
		{
			if (IsInInnerState<Jump_Done>())
				return SiblingTransition<Stand>();

			return InnerEntryTransition<Jump_Up>();
		}
	};

	typedef ClusterBaseState<Jump, BaseState> JumpBaseState;

	struct Jump_Up : JumpBaseState
	{
		virtual void OnEnter()
		{
			JumpBaseState::OnEnter();
			ClusterRootState().mJumpValue1 = 1;
			ClusterRootState().mJumpValue2 = 2.0f;
			ClusterRootState().mJumpValue3 = true;
		}

		virtual Transition GetTransition()
		{
			return SiblingTransition<Jump_Down>();
		}
	};

	struct Jump_Down : JumpBaseState
	{
		virtual void OnEnter()
		{
			JumpBaseState::OnEnter();
			ClusterRootState().mJumpValue1 = 2;
			ClusterRootState().mJumpValue2 = 4.0f;
			ClusterRootState().mJumpValue3 = false;
		}

		virtual Transition GetTransition()
		{
			return SiblingTransition<Jump_Done>();
		}
	};

	struct Jump_Done : BaseState
	{
	};
};

Character::Character()
	: mJump(false)
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
	character.mJump = true;
	character.Update();
}
