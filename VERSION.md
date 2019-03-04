= Version: 1.5.3 =

== 1.5 ==
- Improve StateArgs mechanism to make use of C++11 variadic template arguments

== 1.4 ==
- Improve state debugging by storing an mOwner member in StateWithOwner who's type is OwnerType* rather than void*
- Improve HSM_DEBUG handling so that it can be defined explicitly to 0 or 1 externally (e.g. preprocessor define)
- Add bash script to generate and build makefiles for clang and gcc on MSYS2
- hsm_book_samples: add 'DHSM_DEBUG=ON/OFF' argument to CMake so it can be enabled more easily for non-MSVC builds
- Improve .gitignore for book samples
- Remove unnecessary DEFINE_HSM_STATE macros in chapter 5 sample
- Merge branch 'single-include' into dev
- Add section header comments and general clean up
- Fix hsm_sample_basic_1 to build with single header hsm.h
- Fix hsm_book_samples to build with single header hsm.h
- Replace all headers and one cpp with single "hsm.h" header
- Ignore .gitattributes when archiving
- Use secure versions of snprintf and strncpy for MSVC
- Rename intrusive_ptr to IntrusivePtr to make it clear that it's not a copy of boost::intrusive_ptr
- Fix clang build by moving implementation of ConcreteStateFactory::InvokeStateOnEnter beneath State class

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
