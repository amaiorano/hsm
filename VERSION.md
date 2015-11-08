= Version: 1.3 =

== 1.3 ==
- Add hsm_book_samples to go with [HSM Book](https://github.com/amaiorano/hsm/wiki)
- Fix hsmToDot (plotHsm) to support state machines with multiple roots
- Implement [state override feature](https://github.com/amaiorano/hsm/wiki/Chapter-4.-Advanced-Techniques#sharing-state-machines-state-overrides)
- Make HSM and hsm_book_samples compile under gcc

== 1.2 ==
- Add State::GetImmediateInnerState<StateType> and State::IsInImmediateInnerState<StateType>
- Add TraceLevel enum to replace using hard-coded numbers
- Remove StateMachine::Initialize that takes StateArgs and deprecate Initialize that takes debug info
- Add infinite transition loop detection
- Fix unused arg warning when asserts are disabled (non-debug)
- Add plotHsm.py tool to parse cpp files and render state machine image

== 1.1 ==
- Fix UpdateStates to actually invoke State::Update from outer to inner, rather than from inner to outer

== 1.0 ==
- Initial release
