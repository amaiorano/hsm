// state_stack_visitors.cpp

#include "hsm.h"
#include <string>
#include <memory>

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
			printf("Foo: Not implemented on %s\n", GetStateDebugName());
		}

		virtual void Bar(bool& /*keepVisiting*/, int /*a*/) const
		{
			printf("Bar: Not implemented on %s\n", GetStateDebugName());
		}

		virtual void HandleEvent(bool& /*handled*/, const Event& /*event*/)
		{
			printf("HandleEvent: Not implemented on %s\n", GetStateDebugName());
		}

		virtual void GetSomeValue(std::unique_ptr<std::string>& /*value*/)
		{
			printf("Not implemented on %s\n", GetStateDebugName());
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
			printf("Foo: Implemented on %s\n", GetStateDebugName());
		}
	};

	struct B : BaseState
	{
		Transition GetTransition() override
		{
			return InnerEntryTransition<C>();
		}

		void Bar(bool& keepVisiting, int /*a*/) const override
		{
			printf("Bar: Implemented on %s\n", GetStateDebugName());
			keepVisiting = false;
		}

		void HandleEvent(bool& handled, const Event& event) override
		{
			printf("HandleEvent: Implemented on %s - handled event: %s\n", GetStateDebugName(), event.name.c_str());
			handled = true;
		}
	};

	struct C : BaseState
	{
		void Foo(int /*a*/, float /*b*/) override
		{
			printf("Foo: Implemented on %s\n", GetStateDebugName());
		}

		void GetSomeValue(std::unique_ptr<std::string>& value) override
		{
			printf("Implemented on %s\n", GetStateDebugName());
			value = std::make_unique<std::string>("C's value");
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

	auto PrintSeparator = [] {printf("**********************************\n"); };

	// Invoke member function on state stack tests

	// Invoke virtual member function Foo on each state, passing in extra arguments
	PrintSeparator();
	mStateMachine.InvokeOuterToInner(&CharacterStates::BaseState::Foo, 10, 2.4f);

	PrintSeparator();
	mStateMachine.InvokeInnerToOuter(&CharacterStates::BaseState::Foo, 10, 2.4f);

	// Visit state stack tests

	// Call a simple function, Foo, on each state
	PrintSeparator();
	mStateMachine.VisitOuterToInner(
		[](CharacterStates::BaseState& state) { state.Foo(42, 24.0f); });

	PrintSeparator();
	mStateMachine.VisitInnerToOuter(
		[](CharacterStates::BaseState& state) { state.Foo(42, 24.0f); });

	// Call Bar to which we pass an argument to know whether to keep visiting
	PrintSeparator();
	mStateMachine.VisitOuterToInner(
		[keepVisiting = true](const CharacterStates::BaseState& state) mutable
		{ if (keepVisiting) state.Bar(keepVisiting, 42); });

	PrintSeparator();
	mStateMachine.VisitInnerToOuter(
		[keepVisiting = true](const CharacterStates::BaseState& state) mutable
		{ if (keepVisiting) state.Bar(keepVisiting, 42); });

	// More realistic example: we want to send some kind of event to our state stack, in case a state cares.
	// Usually we send this from inner to outer, which mimics virtual dispatch.
	PrintSeparator();
	const Event event{"test event"};
	bool handled = false;
	mStateMachine.VisitInnerToOuter(
		[&handled, &event](CharacterStates::BaseState& state)
		{ if (!handled) return state.HandleEvent(handled, event); });
	printf("Event was %s\n", handled ? "handled" : "not handled");

	// We can have the state stack return values via a reference parameter. Here we use an unset unique_ptr
	// that is optionally set to a value by a state. Note that with C++17 support, std::optional would be a
	// better choice.
	PrintSeparator();
	std::unique_ptr<std::string> value;
	mStateMachine.VisitOuterToInner(
		[&value](CharacterStates::BaseState& state)
		{ if (!value) state.GetSomeValue(value); });
	if (value)
	{
		printf("Value was returned: %s\n", value->c_str());
	}
}

int main()
{
	Character character;
	character.Update();
}
