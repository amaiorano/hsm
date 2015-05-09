#define ENABLED_SECTION 10

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

#elif ENABLED_SECTION == 7

/// Ownership.BasicUsage

// main.cpp
#include <cstdio>
#include "hsm/statemachine.h"
using namespace hsm;

class MyOwner
{
public:
	MyOwner();
	void UpdateStateMachine();
	void PlaySequence();
	bool GetPlaySequence() const;

private:
	StateMachine mStateMachine;
	bool mPlaySequence;
};

struct MyStates
{
	struct First : State
	{
		virtual Transition GetTransition()
		{
			MyOwner* owner = reinterpret_cast<MyOwner*>(GetStateMachine().GetOwner());

			if (owner->GetPlaySequence())
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
			MyOwner* owner = reinterpret_cast<MyOwner*>(GetStateMachine().GetOwner());

			if (owner->GetPlaySequence())
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

MyOwner::MyOwner()
{
	mPlaySequence = false;
	mStateMachine.Initialize<MyStates::First>(this); //*** Note that we pass 'this' as our owner
	mStateMachine.SetDebugInfo("TestHsm", 1);
}

void MyOwner::UpdateStateMachine()
{
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();
}

void MyOwner::PlaySequence()
{
	mPlaySequence = true;
}

bool MyOwner::GetPlaySequence() const
{
	return mPlaySequence;
}

int main()
{
	MyOwner myOwner;

	myOwner.UpdateStateMachine();
	myOwner.UpdateStateMachine();

	myOwner.PlaySequence();

	myOwner.UpdateStateMachine();
	myOwner.UpdateStateMachine();
}

#elif ENABLED_SECTION == 8

/// Ownership.EasierOwnerAccess

// main.cpp
#include <cstdio>
#include "hsm/statemachine.h"
using namespace hsm;

class MyOwner
{
public:
	MyOwner();
	void UpdateStateMachine();
	void PlaySequence();
	bool GetPlaySequence() const;

private:
	StateMachine mStateMachine;
	bool mPlaySequence;
};

struct MyStates
{
	struct BaseState : StateWithOwner<MyOwner>
	{
	};

	struct First : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().GetPlaySequence())
				return SiblingTransition<Second>();

			return NoTransition();
		}

		virtual void Update()
		{
			printf("First::Update\n");
		}
	};

	struct Second : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().GetPlaySequence())
				return SiblingTransition<Third>();

			return NoTransition();
		}

		virtual void Update()
		{
			printf("Second::Update\n");
		}
	};

	struct Third : BaseState
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

MyOwner::MyOwner()
{
	mPlaySequence = false;
	mStateMachine.Initialize<MyStates::First>(this);
	mStateMachine.SetDebugInfo("TestHsm", 1);
}

void MyOwner::UpdateStateMachine()
{
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();
}

void MyOwner::PlaySequence()
{
	mPlaySequence = true;
}

bool MyOwner::GetPlaySequence() const
{
	return mPlaySequence;
}

int main()
{
	MyOwner myOwner;

	myOwner.UpdateStateMachine();
	myOwner.UpdateStateMachine();

	myOwner.PlaySequence();

	myOwner.UpdateStateMachine();
	myOwner.UpdateStateMachine();
}

#elif ENABLED_SECTION == 9

/// Ownership.AccessingOwnersPrivateMembers

// main.cpp
#include <cstdio>
#include "hsm/statemachine.h"
using namespace hsm;

class MyOwner
{
public:
	MyOwner();
	void UpdateStateMachine();

	void PlaySequence();

private:
	friend struct MyStates; //*** All states can access MyOwner's private members

	StateMachine mStateMachine;
	bool mPlaySequence;
};

struct MyStates
{
	struct BaseState : StateWithOwner<MyOwner>
	{
	};

	struct First : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().mPlaySequence) //*** Access one of owner's private members
				return SiblingTransition<Second>();

			return NoTransition();
		}

		virtual void Update()
		{
			printf("First::Update\n");
		}
	};

	struct Second : BaseState
	{
		virtual Transition GetTransition()
		{
			if (Owner().mPlaySequence) //*** Access one of owner's private members
				return SiblingTransition<Third>();

			return NoTransition();
		}

		virtual void Update()
		{
			printf("Second::Update\n");
		}
	};

	struct Third : BaseState
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

MyOwner::MyOwner()
{
	mPlaySequence = false;
	mStateMachine.Initialize<MyStates::First>(this);
	mStateMachine.SetDebugInfo("TestHsm", 1);
}

void MyOwner::UpdateStateMachine()
{
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();
}

void MyOwner::PlaySequence()
{
	mPlaySequence = true;
}

int main()
{
	MyOwner myOwner;

	myOwner.UpdateStateMachine();
	myOwner.UpdateStateMachine();

	myOwner.PlaySequence();

	myOwner.UpdateStateMachine();
	myOwner.UpdateStateMachine();
}

#elif ENABLED_SECTION == 10

/// StoringData

// main.cpp
#include <cstdio>
#include <string>
#include "hsm/statemachine.h"
using namespace hsm;

class MyOwner
{
public:
	MyOwner();
	void UpdateStateMachine();

private:
	friend struct MyStates; //*** All states can access MyOwner's private members
	StateMachine mStateMachine;
};

struct Foo
{
	Foo() { printf(">>> Foo created\n"); }
	~Foo() { printf(">>>Foo destroyed\n"); }
};

struct MyStates
{
	struct BaseState : StateWithOwner<MyOwner>
	{
	};

	struct First : BaseState
	{
		virtual Transition GetTransition()
		{
			return SiblingTransition<Second>();
		}

		Foo mFoo; //*** State data member
	};

	struct Second : BaseState
	{
		virtual Transition GetTransition()
		{
			return NoTransition();
		}
	};
};

MyOwner::MyOwner()
{
	mStateMachine.Initialize<MyStates::First>(this);
	mStateMachine.SetDebugInfo("TestHsm", 1);
}

void MyOwner::UpdateStateMachine()
{
	mStateMachine.ProcessStateTransitions();
	mStateMachine.UpdateStates();
}

int main()
{
	MyOwner myOwner;
	myOwner.UpdateStateMachine();
}

#endif
