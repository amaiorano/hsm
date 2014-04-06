#include "../../include/hsm/statemachine.h"

// Player is a class that _owns_ a state machine. The states defined in the cpp can access this
// owner and can access its privates.
class Player
{
public:
	Player();
	void Init();
	void Shutdown();
	void FrameUpdate(float deltaTime);

private:
	// The state machine instance that will maintain the state stack
	hsm::StateMachine m_stateMachine;

	// Declare an inner-struct here which we will define in the cpp file. In this struct, we will put all our
	// states. This serves two purposes:
	// 1) Because of C++ parsing rules for inner types, the states we add within the States struct can refer to other
	//    states defined below it - no need to forward declare them.
	// 2) By making this an inner type of Player, all states will have access to the privates of Player. This
	//    is because in C++, inner types are members and as such has the same access rights as any other member.
	struct States;

	// Regular data member
	float m_health;

	// StateValues - the values of these variables are bound to the state stack. What this means is that if a state
	// modifies the value, when that state gets popped, the value will revert to its previous value.
	hsm::StateValue<bool> m_bLockControls;
};
