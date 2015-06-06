= Version: 1.2

== Changes since 1.1
- Add State::GetImmediateInnerState<StateType> and State::IsInImmediateInnerState<StateType>
- Add TraceLevel enum to replace using hard-coded numbers
- Remove StateMachine::Initialize that takes StateArgs and deprecate Initialize that takes debug info
- Add infinite transition loop detection
- Fix unused arg warning when asserts are disabled (non-debug)
- Add plotHsm.py tool to parse cpp files and render state machine image

== Changes since 1.0
- Fix UpdateStates to actually invoke State::Update from outer to inner, rather than from inner to outer
