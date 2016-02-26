#include "test_util.h"
using namespace hsm;

namespace UNIQUE_NAMESPACE_NAME {
struct MyStates
{
	struct A : State
	{
		virtual Transition GetTransition() { return InnerEntryTransition<B>(); }
	};

	struct B : State
	{
		virtual Transition GetTransition() { return InnerEntryTransition<C>(); }
	};

	struct C : State
	{
	};
};

TEST_CASE("statemachine/initializeshutdown", "[statemachine]")
{
	StateMachine stateMachine;
	REQUIRE_FALSE(stateMachine.IsInitialized());

	stateMachine.Initialize<MyStates::A>();
	REQUIRE(stateMachine.IsInitialized());
	REQUIRE_FALSE(stateMachine.IsStarted());
	REQUIRE(IsStateStackEmpty(stateMachine));
	REQUIRE_FALSE(stateMachine.GetOwner());

	stateMachine.ProcessStateTransitions();
	REQUIRE(stateMachine.IsStarted());

	INFO("Actual state stack is: " << GetStateStackAsString(stateMachine));
	REQUIRE((EqualsStateStack<MyStates::A, MyStates::B, MyStates::C>(stateMachine)));

	stateMachine.Stop();
	REQUIRE_FALSE(stateMachine.IsStarted());
	REQUIRE(stateMachine.IsInitialized());
	REQUIRE(IsStateStackEmpty(stateMachine));

	stateMachine.Shutdown();
	REQUIRE_FALSE(stateMachine.IsInitialized());
}
} // namespace

namespace UNIQUE_NAMESPACE_NAME {
int count = 0;
struct MyStates
{
	struct A : State
	{
		void OnEnter() { ++count; }
		void OnExit() { --count; }
		virtual Transition GetTransition() { return InnerEntryTransition<B>(); }
	};

	struct B : State
	{
		void OnEnter() { ++count; }
		void OnExit() { --count; }
		virtual Transition GetTransition() { return InnerEntryTransition<C>(); }
	};

	struct C : State
	{
		void OnEnter() { ++count; }
		void OnExit() { --count; }
	};
};

TEST_CASE("statemachine/shutdown", "[statemachine]")
{
	using namespace hsm;
	StateMachine stateMachine;
	stateMachine.Initialize<MyStates::A>();
	REQUIRE(count == 0);
	stateMachine.ProcessStateTransitions();
	REQUIRE(count == 3);
	stateMachine.ProcessStateTransitions();
	REQUIRE(count == 3);
	stateMachine.Shutdown(); // stop arg is true by default, OnExit will get called on all states
	REQUIRE(count == 0);

	stateMachine.Initialize<MyStates::A>();
	stateMachine.ProcessStateTransitions();
	REQUIRE(count == 3);
	stateMachine.Shutdown(false); // OnExit will not be called on state stack
	REQUIRE(count == 3);
}
} // namespace


namespace UNIQUE_NAMESPACE_NAME {

int count = 0;

struct MyStates
{
	struct A : State
	{
		virtual Transition GetTransition() { return InnerEntryTransition<B>(); }
		virtual void Update() { ++count; }
	};

	struct B : State
	{
		virtual Transition GetTransition() { return InnerEntryTransition<C>(); }
		virtual void Update() { ++count; }
	};

	struct C : State
	{
		virtual void Update() { ++count; }
	};
};

TEST_CASE("statemachine/updatestates", "[statemachine]")
{
	StateMachine stateMachine;
	stateMachine.Initialize<MyStates::A>();
	REQUIRE(count == 0);
	stateMachine.ProcessStateTransitions();
	REQUIRE(count == 0);
	stateMachine.UpdateStates();
	REQUIRE(count == 3);
	stateMachine.UpdateStates();
	REQUIRE(count == 3*2);
	stateMachine.Shutdown();
	REQUIRE(count == 3 * 2);
}
}

namespace UNIQUE_NAMESPACE_NAME {

int whichInner = 0;
bool gotoSiblingC1 = false;

struct MyStates
{
	struct Root : State
	{
		virtual Transition GetTransition()
		{
			switch (whichInner)
			{
			case 0: return InnerTransition<A1>();
			case 1: return InnerTransition<B1>();
			default: FAIL();
			}
			return NoTransition();
		}
	};

	struct A1 : State
	{
		virtual Transition GetTransition()
		{
			if (gotoSiblingC1)
			{
				gotoSiblingC1 = false;
				return SiblingTransition<C1>();
			}
			return InnerEntryTransition<A2>();
		}
	};
	struct A2 : State
	{
		virtual Transition GetTransition() { return InnerEntryTransition<A3>(); }
	};
	struct A3 : State
	{
	};

	struct B1 : State
	{
		virtual Transition GetTransition() { return InnerEntryTransition<B2>(); }
	};
	struct B2 : State
	{
		virtual Transition GetTransition() { return InnerEntryTransition<B3>(); }
	};
	struct B3 : State
	{
	};

	struct C1 : State
	{
	};
};

TEST_CASE("statemachine/innertransition", "[transitions]")
{
	StateMachine stateMachine;
	stateMachine.Initialize<MyStates::Root>();
	REQUIRE(whichInner == 0);

	stateMachine.ProcessStateTransitions();
	REQUIRE((EqualsStateStack<MyStates::Root, MyStates::A1, MyStates::A2, MyStates::A3>(stateMachine)));
	REQUIRE(whichInner == 0);

	whichInner = 1;
	stateMachine.ProcessStateTransitions();
	REQUIRE((EqualsStateStack<MyStates::Root, MyStates::B1, MyStates::B2, MyStates::B3>(stateMachine)));

	whichInner = 0;
	stateMachine.ProcessStateTransitions();
	REQUIRE((EqualsStateStack<MyStates::Root, MyStates::A1, MyStates::A2, MyStates::A3>(stateMachine)));

	// Test that despite A1 sibling to C1, Root will force A1 back via InnerTransition
	gotoSiblingC1 = true;
	stateMachine.ProcessStateTransitions();
	REQUIRE_FALSE((EqualsStateStack<MyStates::Root, MyStates::C1>(stateMachine)));
	REQUIRE((EqualsStateStack<MyStates::Root, MyStates::A1, MyStates::A2, MyStates::A3>(stateMachine)));

	stateMachine.Shutdown();
	REQUIRE((EqualsStateStack<NullType>(stateMachine)));
}
}
