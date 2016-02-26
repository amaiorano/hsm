#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "hsm.h"
#include "catch.hpp"
#include <cstdio>

#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)
#define UNIQUE_NAMESPACE_NAME CONCAT(NS_, __COUNTER__)

struct NullType {};

bool IsStateStackEmpty(hsm::StateMachine& sm)
{
	return sm.BeginOuterToInner() == sm.EndOuterToInner();
}

namespace util_detail
{
	using namespace hsm;

	//@NOTE: This should be moved to hsm.h, but I know StateTypeIdStorage is removed in the cpp11
	// branch, so don't bother.
	bool operator!=(const hsm::StateTypeIdStorage& lhs, const hsm::StateTypeIdStorage& rhs)
	{
		return !(lhs == rhs);
	}

	template <typename StateT>
	bool EqualsStateStack(OuterToInnerIterator iter, OuterToInnerIterator end)
	{
		if (iter == end)
			return false;

		if ((*iter)->GetStateType() != GetStateType<StateT>())
			return false;

		++iter;

		// Ok, last templated state is equal, now make sure our state stack is also on its last entry
		return iter == end;
	}

	template <typename First, typename Second, typename... Rest>
	bool EqualsStateStack(OuterToInnerIterator iter, OuterToInnerIterator end)
	{
		if (iter == end)
			return false;

		if ((*iter)->GetStateType() != GetStateType<First>())
			return false;

		++iter;

		return EqualsStateStack<Second, Rest...>(iter, end);
	}

	template <typename Only>
	struct EqualsStateStackSelector
	{
		static bool Exec(hsm::StateMachine& sm)
		{
			return util_detail::EqualsStateStack<Only>(sm.BeginOuterToInner(), sm.EndOuterToInner());
		}
	};

	template <>
	struct EqualsStateStackSelector<NullType>
	{
		static bool Exec(hsm::StateMachine& sm)
		{
			return IsStateStackEmpty(sm);
		}
	};

	template <typename Only>
	bool EqualsStateStack(hsm::StateMachine& sm)
	{
		return EqualsStateStackSelector<Only>::Exec(sm);
	}
}

//@NOTE: With C++11 we can use default template args on function templates. For now,
// call-sites must pass NullType explicitly to test against empty state stack.
template <typename Only /*= NullType*/>
bool EqualsStateStack(hsm::StateMachine& sm)
{
	return util_detail::EqualsStateStack<Only>(sm);
}

template <typename First, typename Second, typename... Rest>
bool EqualsStateStack(hsm::StateMachine& sm)
{
	return util_detail::EqualsStateStack<First, Second, Rest...>(sm.BeginOuterToInner(), sm.EndOuterToInner());
}

std::string GetStateStackAsString(hsm::StateMachine& sm)
{
	std::string result = "[";
	hsm::OuterToInnerIterator begin = sm.BeginOuterToInner();
	for (hsm::OuterToInnerIterator iter = begin; iter != sm.EndOuterToInner(); ++iter)
	{
		if (iter != begin) result += ", ";
		result += (*iter)->GetStateDebugName();
	}
	result += "]";
	return result;
}
