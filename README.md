# hsm

HSM, short for Hierarchical State Machine, is an open-source C++ framework library that can be used to simplify the organization of state-driven code. The implementation is partially inspired by the UML State Machine or UML Statechart as well as by the UnrealScript State system in Unreal Engine 3.

## Why use it?

Most state-driven code can benefit from using any type of state machine library; however, HSM provides the following specific benefits:

* *Simple to use:* making use of HSM is fairly straightforward;
* *Easy to understand:* reading HSM code and understanding how it works is often much simpler than in similar systems mainly due to how transition logic is implemented inline rather than in a table;
* *Easy to debug:* similarly, since there are no transition table lookups, debugging the state machine is straightforward;
* *Scalable:* although its main purpose is to provide a method for nesting states, HSM can also simply be used for single-layer, or flat, state machines with very little overhead;
* *Performance:* HSM has been written with performance in mind.

HSM is particularly suited to real-time systems, games, or any environment where state changes are made frequently based on complex input-related conditions. However, this does not preclude its use in any other type of software.

## Developer Setup

The library is made up of a few header files and a single cpp file (statemachine.cpp). To make use of it, you are expected to build the cpp file as part of your compilation step, and make the headers available for inclusion.

1. Clone or download a zip of the source code
2. Assuming the root directory is named "hsm", add "hsm/include" to your compiler's INCLUDE path. This is recommended so that including hsm headers with take the form: ```#include "hsm/statemachine.h"```
3. Modify hsm/include/hsm/config.h to match your environment and project-specific settings (more on this below)
4. Build hsm/src/statemachine.cpp

### Configuration

By default, HSM is configured to be compatible with standard C++ projects, making use of certain STL types, default operator new and operator delete, etc. However, many projects do not make use of the standard types, or provide their own types optimized for their target platform. Thus, the library provides a single header file, hsm/config.h, which contains macros and typedefs used by the library. This file can be modified to configure HSM to better fit the code environment it's being used in.

Most of the contents of config.h are self-explanatory; here are a couple of notes for some of the configurable types and definitions:

* HSM_STD_VECTOR and HSM_STD_STRING are expected to define types that provide a subset of the std::vector and std::string functionality, respectively. Some code bases avoid using the STL, and define their own types, which are not necessarily compatible with the STL in terms of interface. In this case, one solution would be to provide a wrapper that implements the functionality subset of the STL types to your own types. If this proves too difficult, it may be simpler to replace HSM code that makes use of these types with your own. 
