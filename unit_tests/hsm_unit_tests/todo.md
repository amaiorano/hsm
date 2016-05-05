= HSM Unit Tests TODO =

-> Add option to turn asserts into exceptions to hsm; this would allow to test that failures are detected using catch.

== Tests to write ==

- transitions
  - inner: make sure it forces a transition from outer
    - also: test inner from an inner state (make sure outers don't change)
  - innerentry: make sure it cannot force a transition from outer
  - sibling: make sure it transitions
  - test state stack settling using ephemeral states

- ProcessStateTransition
  - make sure it processes all transitions until state stack settles (validate that state stack remains the same after multiple calls; use randoms to make all kinds of tranitions occur between runs)


- state stack query functions

- StateValue

- StateWithOwner
	- const and non-const access to Owner

- StateOverride

- Failure detection:
  - infinite transition loop
  - accessing null owner
	