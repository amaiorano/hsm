#include "Player.h"

namespace
{
	struct CombatInfo
	{
		// Pretend there's super useful data here
	};

	// Stub
	void SetPlayerControlsLocked(bool bLocked) {}
}

// To reduce verbosity, it's recommended to bring in the hsm namespace - but only in the cpp and never in the header!
using namespace hsm;

// Define the inner struct States - this is where we put all our states
struct Player::States
{
	// Declare a custom base State class that effectively derives from hsm::State and sets Player as
	// the Owner type. This basically adds the member function Player& Owner() to your states. Internally,
	// it's just casting the state machine's void* pointer to your owner to Player for you. Note that you
	// can just use typedef or declare a StateBase struct that derives from StateWithOwner in which
	// you could add utility functions for all states.
	struct StateBase : StateWithOwner<Player>
	{
		// An example state message/event. @TODO: Add support for generic state messages.
		virtual bool OnCombatStarted(const CombatInfo& combatInfo) { return false; }
	};

	// Declare a root state. This will be the initial state we pass to the state machine. Note that you don't
	// actually need a single root state - if you want to have a simple flat state machine, just pass whatever
	// initial state you want and perform only sibling transitions.
	struct Root : StateBase
	{
		// This macro is necessary because we set the HSM to NOT use the standard C++ RTTI.
		DEFINE_HSM_STATE(Root);

		virtual Transition GetTransition()
		{
			return InnerEntryTransition<Alive>();
		}
	};

	// Alive and Dead are both inner states of Root, and sibling states of each other
	struct Alive : StateBase
	{
		DEFINE_HSM_STATE(Alive);

		virtual Transition GetTransition()
		{
			if ( Owner().m_health <= 0.0f )
			{
				return SiblingTransition<Dead>();
			}

			// Start in Exploring inner state
			return InnerEntryTransition<Exploring>();
		}
	};

	struct Dead : StateBase
	{
		DEFINE_HSM_STATE(Dead);

		virtual void OnEnter()
		{
			// Tell some manager that we died!
		}
	};

	struct Exploring : StateBase
	{
		DEFINE_HSM_STATE(Exploring);

		// This state is a good example of how to handle events. We store a transition object default initialized
		// to NoTransition, which we keep returning in GetTransition, and once we get the event we're waiting for,
		// we set the transition object to whatever transition we want.

		virtual void OnEnter()
		{
			m_transition = NoTransition();
		}		

		virtual Transition GetTransition()
		{
			return m_transition;
		}

		virtual void Update(/*float deltaTime*/)
		{
		}

		virtual bool OnCombatStarted(const CombatInfo& combatInfo)
		{
			// Ok, we received a combat started message, so let's set up the transition we want
			// to return on the next HSM update. This is also an example of how to pass arguments
			// to a state via the StateArgs mechanism.
			m_transition = SiblingTransition<Fighting>( Fighting::Args(combatInfo) );

			return true; // We handled this message, no need to keep dispatching up the stack
		}

		Transition m_transition;
	};


	struct Fighting : StateBase
	{
		DEFINE_HSM_STATE(Fighting);

		// This is an example of how to declare arguments for a state. The way it works is that
		// you must declare a type named Args that derives from StateArgs, and override the
		// version of OnEnter that takes 'const Args&'.
		struct Args : StateArgs
		{
			// Having a constructor like this is convenient but not necessary. You could simply
			// declare and fill up a struct instance at the call site and pass it in to the transition
			// function.
			Args(const CombatInfo& combatInfo) : m_combatInfo(combatInfo) {}
			Args() {};

			CombatInfo m_combatInfo;
		};

		virtual void OnEnter(const Args& args)
		{
			// Sometimes it's useful to cache the args like this
			m_args = args;
		}

		virtual Transition GetTransition()
		{
			// We start in Fighting_Main, and wait until our inner siblings to Fighting_Done
			if ( IsInState<Fighting_Done>() )
			{
				return SiblingTransition<Exploring>();
			}

			return InnerEntryTransition<Fighting_Main>();
		}

		Args m_args;
	};

	struct Fighting_Main : StateBase
	{
		DEFINE_HSM_STATE(Fighting_Main);

		virtual Transition GetTransition()
		{
			if ( IsCombatFinished() )
			{
				return SiblingTransition<Fighting_Done>();
			}

			if ( DoCinematicAttack() )
			{
				return SiblingTransition<Fighting_CinematicAttack>();
			}

			return NoTransition();
		}

		bool IsCombatFinished() // Stub
		{
			static int frame = 0;
			++frame;
			return frame >= 10;
		}

		bool DoCinematicAttack() // Stub
		{
			return true;
		}
	};

	struct Fighting_CinematicAttack : StateBase
	{
		DEFINE_HSM_STATE(Fighting_CinematicAttack);

		virtual void OnEnter()
		{
			// An example of setting a StateValue. When this state gets popped off the stack, the value
			// of m_bLockControls will go back to whatever it was before we set it in this state. Note that
			// we can set it as often as we like in this state, it will go back to whatever it was before this
			// state was pushed.
			SetStateValue(Owner().m_bLockControls) = true; // Lock controls during cinematics
		}

		virtual Transition GetTransition()
		{
			if ( IsCinematicAttackFinished() )
			{
				return SiblingTransition<Fighting_Main>();
			}

			return NoTransition();
		}

		bool IsCinematicAttackFinished() // Stub
		{
			return true;
		}
	};

	// This state doesn't do anything - an outer state checks for its existence in GetTransition()
	// and performs a transition when found. In other words, this is a transient state that acts like
	// a bool, and should never remain on the stack once it has settled.
	struct Fighting_Done : StateBase
	{
		DEFINE_HSM_STATE(Fighting_Done);
	};
};


Player::Player() :
	m_health(100.0f),
	m_bLockControls(false)
{
}

void Player::Init()
{
	// Initialize the state machine, passing in the root state type, the owner (this), and debug info.
	int debugLevel = 2; // 0 is no logging, 1 is basic logging, and 2 is verbose (for now, these may change)
	m_stateMachine.Initialize<States::Root>(this, HSM_TEXT("Player"), debugLevel);
}

void Player::Shutdown()
{
	m_stateMachine.Shutdown();
}

void Player::FrameUpdate(float deltaTime)
{
	// Update the state machine once per frame. This will first process all state transitions until
	// the state stack has settled.
	m_stateMachine.ProcessStateTransitions();

	// Update all states from outermost to innermost.
	// To pass in deltaTime here, modify the HSM_STATE_UPDATE_ARGS macros in config.h
	m_stateMachine.UpdateStates(/*deltaTime*/);

	// This is how to invoke a custom function on our states
	//InnerToOuterIterator iter = m_stateMachine.BeginInnerToOuter();
	//InnerToOuterIterator end = m_stateMachine.EndInnerToOuter();
	//for ( ; iter != end; ++iter)
	//{
	//	static_cast<States::StateBase*>(*iter)->CustomFunc();
	//}

	// Here we read a StateValue that may have been modified by the state machine
	SetPlayerControlsLocked(m_bLockControls);

	// Wait a few frames to send a CombatStarted message to the HSM
	static int frame = 0;
	++frame;
	if (frame == 5)
	{
		CombatInfo combatInfo;

		// This is one way to send a message to the HSM. We decide to invoke from innermost to outermost
		// because this emulates virtual dispatch.
		InnerToOuterIterator iter = m_stateMachine.BeginInnerToOuter();
		InnerToOuterIterator end = m_stateMachine.EndInnerToOuter();
		for ( ; iter != end; ++iter)
		{
			bool bHandled = static_cast<States::StateBase*>(*iter)->OnCombatStarted(combatInfo);
			if (bHandled)
				break;
		}
	}
}
