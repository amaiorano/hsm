// state_stack_visitors.cpp

#include "hsm.h"
#include <string>

using namespace hsm;

class Character
{
public:
	Character();
	void Update();

private:
	friend struct CharacterStates;
	StateMachine mStateMachine;
};

struct Event
{
	std::string name;
};

struct CharacterStates
{
	struct BaseState : StateWithOwner<Character>
	{
		virtual void Foo(int /*a*/, float /*b*/)
		{
			printf("Not implemented on %s\n", GetStateDebugName());
		}

		virtual VisitResult Bar(int /*a*/) const
		{
			printf("Not implemented on %s\n", GetStateDebugName());
			return VisitResult::Continue;
		}

		virtual VisitResult HandleEvent(const Event& /*event*/)
		{
			printf("Not implemented on %s\n", GetStateDebugName());
			return VisitResult::Continue;
		}

		virtual VisitResult GetSomeValue(std::string& /*value*/)
		{
			printf("Not implemented on %s\n", GetStateDebugName());
			return VisitResult::Continue;
		}
	};

	struct A : BaseState
	{
		Transition GetTransition() override
		{
			return InnerEntryTransition<B>();
		}

		void Foo(int /*a*/, float /*b*/) override
		{
			printf("Implemented on %s\n", GetStateDebugName());
		}
	};

	struct B : BaseState
	{
		Transition GetTransition() override
		{
			return InnerEntryTransition<C>();
		}

		VisitResult Bar(int /*a*/) const override
		{
			printf("Implemented on %s\n", GetStateDebugName());
			return VisitResult::Stop;
		}
	};

	struct C : BaseState
	{
		void Foo(int /*a*/, float /*b*/) override
		{
			printf("Implemented on %s\n", GetStateDebugName());
		}

		virtual VisitResult HandleEvent(const Event& event)
		{
			printf("Implemented on %s - handled event: %s\n", GetStateDebugName(), event.name.c_str());
			return VisitResult::Stop;
		}

		VisitResult GetSomeValue(std::string& value)
		{
			printf("Implemented on %s\n", GetStateDebugName());
			value = "Yay bacon!";
			return VisitResult::Stop;
		}
	};
};

Character::Character()
{
	mStateMachine.Initialize<CharacterStates::A>(this);
	mStateMachine.SetDebugInfo("TestHsm", TraceLevel::Basic);
}

void Character::Update()
{
	printf(">>> Character::Update\n");

	// Update state machine
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();

	// Visit state stack tests
	auto PrintSeparator = [] {printf("**********************************\n"); };

	// Test calling a simple function, Foo, on each state
	PrintSeparator();
	mStateMachine.VisitOuterToInner([](CharacterStates::BaseState& state) { state.Foo(42, 24.0f); });

	PrintSeparator();
	mStateMachine.VisitInnerToOuter([](CharacterStates::BaseState& state) { state.Foo(42, 24.0f); });

	// Test calling const function, Bar, which returns VisitResult. Implemented only in B, which stops
	// the visiting.
	PrintSeparator();
	mStateMachine.VisitOuterToInner([](const CharacterStates::BaseState& state) { return state.Bar(42); });

	PrintSeparator();
	mStateMachine.VisitInnerToOuter([](const CharacterStates::BaseState& state) { return state.Bar(42); });

	// More realistic example: we want to send some kind of event to our state stack, in case a state cares.
	// Usually we send this from inner to outer, which mimics virtual dispatch.
	// We can know if a state handled the event by having the default implementation in BaseState return
	// VisitResult::Continue, and having states that handle the event return VisitResult::Stop.
	PrintSeparator();
	const Event event{"test event"};
	VisitResult vr = mStateMachine.VisitInnerToOuter([&event](CharacterStates::BaseState& state) { return state.HandleEvent(event); });
	printf("Event was %s\n", vr == VisitResult::Stop ? "handled" : "not handled");

	// We can have the state stack return values via a reference parameter, using VisitResult once again to
	// communicate if a state actually did or not.
	PrintSeparator();
	std::string value;
	vr = mStateMachine.VisitOuterToInner([&value](CharacterStates::BaseState& state) { return state.GetSomeValue(value); });
	if (vr == VisitResult::Stop)
	{
		printf("Value was returned: %s\n", value.c_str());
	}
}

int main()
{
	Character character;
	character.Update();
}
