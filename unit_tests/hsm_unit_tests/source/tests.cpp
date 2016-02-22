#include "test_util.h"
using namespace hsm;

namespace UNIQUE_NAMESPACE_NAME {
struct MyStates
{
	struct A : State
	{
		Transition GetTransition() { return InnerEntryTransition<B>(); }
	};

	struct B : State
	{
		Transition GetTransition() { return InnerEntryTransition<C>(); }
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
struct MyStates
{
	static int count;

	struct A : State
	{
		void OnEnter() { ++count; }
		void OnExit() { --count; }
		Transition GetTransition() { return InnerEntryTransition<B>(); }
	};

	struct B : State
	{
		void OnEnter() { ++count; }
		void OnExit() { --count; }
		Transition GetTransition() { return InnerEntryTransition<C>(); }
	};

	struct C : State
	{
		void OnEnter() { ++count; }
		void OnExit() { --count; }
	};
};

int MyStates::count = 0;

TEST_CASE("statemachine/shutdown", "[statemachine]")
{
	using namespace hsm;
	StateMachine stateMachine;
	stateMachine.Initialize<MyStates::A>();
	REQUIRE(MyStates::count == 0);
	stateMachine.ProcessStateTransitions();
	REQUIRE(MyStates::count == 3);
	stateMachine.ProcessStateTransitions();
	REQUIRE(MyStates::count == 3);
	stateMachine.Shutdown(); // stop arg is true by default, OnExit will get called on all states
	REQUIRE(MyStates::count == 0);

	stateMachine.Initialize<MyStates::A>();
	stateMachine.ProcessStateTransitions();
	REQUIRE(MyStates::count == 3);
	stateMachine.Shutdown(false); // OnExit will not be called on state stack
	REQUIRE(MyStates::count == 3);
}
} // namespace


namespace UNIQUE_NAMESPACE_NAME {
struct MyStates
{
	static int count;

	struct A : State
	{
		Transition GetTransition() { return InnerEntryTransition<B>(); }
		void Update() { ++count; }
	};

	struct B : State
	{
		Transition GetTransition() { return InnerEntryTransition<C>(); }
		void Update() { ++count; }
	};

	struct C : State
	{
		void Update() { ++count; }
	};
};

int MyStates::count = 0;

TEST_CASE("statemachine/updatestates", "[statemachine]")
{
	StateMachine stateMachine;
	stateMachine.Initialize<MyStates::A>();
	REQUIRE(MyStates::count == 0);
	stateMachine.ProcessStateTransitions();
	REQUIRE(MyStates::count == 0);
	stateMachine.UpdateStates();
	REQUIRE(MyStates::count == 3);
	stateMachine.UpdateStates();
	REQUIRE(MyStates::count == 3*2);
	stateMachine.Shutdown();
	REQUIRE(MyStates::count == 3 * 2);
}
}
