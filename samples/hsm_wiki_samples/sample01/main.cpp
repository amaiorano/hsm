#define ENABLED_SECTION 6

#if ENABLED_SECTION == 1

// main.cpp

#include "hsm/statemachine.h"

struct First : hsm::State
{
};

int main()
{
	hsm::StateMachine stateMachine;
	stateMachine.Initialize<First>();
	stateMachine.ProcessStateTransitions();
}

#elif ENABLED_SECTION == 2

// main.cpp

#include "hsm/statemachine.h"

using namespace hsm;

struct Third : State
{
	virtual Transition GetTransition()
	{
		return NoTransition();
	}
};

struct Second : State
{
	virtual Transition GetTransition()
	{
		return SiblingTransition<Third>();
	}
};

struct First : State
{
	virtual Transition GetTransition()
	{
		return SiblingTransition<Second>();
	}
};

int main()
{
	StateMachine stateMachine;
	stateMachine.Initialize<First>();
	stateMachine.SetDebugInfo("TestHsm", 1);
	stateMachine.ProcessStateTransitions();
}

#elif ENABLED_SECTION == 3

// main.cpp

#include "hsm/statemachine.h"

using namespace hsm;

struct MyStates
{
	struct First : State
	{
		virtual Transition GetTransition()
		{
			return SiblingTransition<Second>();
		}
	};

	struct Second : State
	{
		virtual Transition GetTransition()
		{
			return SiblingTransition<Third>();
		}
	};

	struct Third : State
	{
		virtual Transition GetTransition()
		{
			return NoTransition();
		}
	};
};

int main()
{
	StateMachine stateMachine;
	stateMachine.Initialize<MyStates::First>();
	stateMachine.SetDebugInfo("TestHsm", 1);
	stateMachine.ProcessStateTransitions();
}

#elif ENABLED_SECTION == 4

// main.cpp
#include <cstdio>
#include "hsm/statemachine.h"
using namespace hsm;

struct MyStates
{
	struct First : State
	{
		virtual void OnEnter()
		{
			printf("First::OnEnter\n");
		}

		virtual void OnExit()
		{
			printf("First::OnExit\n");
		}

		virtual Transition GetTransition()
		{
			return SiblingTransition<Second>();
		}
	};

	struct Second : State
	{
		virtual void OnEnter()
		{
			printf("Second::OnEnter\n");
		}

		virtual void OnExit()
		{
			printf("Second::OnExit\n");
		}

		virtual Transition GetTransition()
		{
			return SiblingTransition<Third>();
		}
	};

	struct Third : State
	{
		virtual void OnEnter()
		{
			printf("Third::OnEnter\n");
		}

		virtual void OnExit()
		{
			printf("Third::OnExit\n");
		}

		virtual Transition GetTransition()
		{
			return NoTransition();
		}
	};
};

int main()
{
	StateMachine stateMachine;
	stateMachine.Initialize<MyStates::First>();
	stateMachine.SetDebugInfo("TestHsm", 1);
	stateMachine.ProcessStateTransitions();
}

#elif ENABLED_SECTION == 5

// main.cpp
#include <cstdio>
#include "hsm/statemachine.h"
using namespace hsm;

bool gStartOver = false;

struct MyStates
{
	struct First : State
	{
		virtual void OnEnter()
		{
			gStartOver = false;
		}

		virtual Transition GetTransition()
		{
			return SiblingTransition<Second>();
		}
	};

	struct Second : State
	{
		virtual Transition GetTransition()
		{
			return SiblingTransition<Third>();
		}
	};

	struct Third : State
	{
		virtual Transition GetTransition()
		{
			if (gStartOver)
				return SiblingTransition<First>();

			return NoTransition();
		}
	};
};

int main()
{
	StateMachine stateMachine;
	stateMachine.Initialize<MyStates::First>();
	stateMachine.SetDebugInfo("TestHsm", 1);
	
	printf(">>> First ProcessStateTransitions\n");
	stateMachine.ProcessStateTransitions();

	printf(">>> Second ProcessStateTransitions\n");
	stateMachine.ProcessStateTransitions();

	gStartOver = true;
	printf(">>> Third ProcessStateTransitions\n");
	stateMachine.ProcessStateTransitions();

	printf(">>> Fourth ProcessStateTransitions\n");
	stateMachine.ProcessStateTransitions();
}

#elif ENABLED_SECTION == 6

// main.cpp
#include <cstdio>
#include "hsm/statemachine.h"
using namespace hsm;

bool gPlaySequence = false;

struct MyStates
{
	struct First : State
	{
		virtual Transition GetTransition()
		{
			if (gPlaySequence)
				return SiblingTransition<Second>();
			
			return NoTransition();
		}

		virtual void Update()
		{
			printf("First::Update\n");
		}
	};

	struct Second : State
	{
		virtual Transition GetTransition()
		{
			if (gPlaySequence)
				return SiblingTransition<Third>();
			
			return NoTransition();
		}

		virtual void Update()
		{
			printf("Second::Update\n");
		}
	};

	struct Third : State
	{
		virtual Transition GetTransition()
		{
			return NoTransition();
		}

		virtual void Update()
		{
			printf("Third::Update\n");
		}
	};
};

int main()
{
	StateMachine stateMachine;
	stateMachine.Initialize<MyStates::First>();
	stateMachine.SetDebugInfo("TestHsm", 1);

	stateMachine.ProcessStateTransitions();
	stateMachine.UpdateStates();

	stateMachine.ProcessStateTransitions();
	stateMachine.UpdateStates();

	gPlaySequence = true;

	stateMachine.ProcessStateTransitions();
	stateMachine.UpdateStates();

	stateMachine.ProcessStateTransitions();
	stateMachine.UpdateStates();
}

#endif
