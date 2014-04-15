// Hierarchical State Machine (HSM)
//
// Copyright (c) 2013 Antonio Maiorano
//
// Distributed under the MIT License (MIT)
// (See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT)

/// \file rtti.h
/// \brief Utilities to uniquely identify States via standard or custom RTTI.

#include "config.h"

#ifndef HSM_RTTI_H
#define HSM_RTTI_H


#if HSM_CPP_RTTI

#include <typeinfo>

namespace hsm {

// We use standard C++ RTTI
	
// We need a copyable wrapper around std::type_info since std::type_info is non-copyable
struct StateTypeIdStorage
{
	StateTypeIdStorage() : m_typeInfo(0) { }
	StateTypeIdStorage(const std::type_info& typeInfo) { m_typeInfo = &typeInfo; }
	hsm_bool operator==(const StateTypeIdStorage& rhs) const
	{
		HSM_ASSERT_MSG(m_typeInfo != 0, "m_typeInfo was not properly initialized");
		return *m_typeInfo == *rhs.m_typeInfo;
	}
	const std::type_info* m_typeInfo;
};

typedef const StateTypeIdStorage& StateTypeId;

template <typename StateType>
StateTypeId GetStateType()
{
	static StateTypeIdStorage stateTypeId(typeid(StateType));
	return stateTypeId;
}

} // namespace hsm

// DEFINE_HSM_STATE is NOT necessary; however, we define it here to nothing to make it easier to
// test switching between compiler RTTI enabled or disabled.
#define DEFINE_HSM_STATE(__StateName__)


#else // !HSM_CPP_RTTI

namespace hsm {

// Standard C++ RTTI is not available, so we roll our own custom RTTI. All states are required to use the
// DEFINE_HSM_STATE macro, which makes use of the input name of the state as the unique identifier. String
// compares are used to determine equality.

// Like std::type_info, we need to be able test equality and get a unique name
struct StateTypeIdStorage
{
	StateTypeIdStorage(const hsm_char* aStateName = 0) : mStateName(aStateName) {}
	hsm_bool operator==(const StateTypeIdStorage& rhs) const
	{
		HSM_ASSERT_MSG(mStateName != 0, "StateTypeId was not properly initialized");
		return STRCMP(mStateName, rhs.mStateName) == 0;
	}
	const hsm_char* mStateName;
};

typedef const StateTypeIdStorage& StateTypeId;

template <typename StateType>
StateTypeId GetStateType()
{
	return StateType::GetStaticStateType();
}

} // namespace hsm

// Must use this macro in every State to add RTTI support.
#define DEFINE_HSM_STATE(__StateName__) \
	static hsm::StateTypeId GetStaticStateType() { static hsm::StateTypeIdStorage sStateTypeId(HSM_TEXT(#__StateName__)); return sStateTypeId; } \
	virtual hsm::StateTypeId DoGetStateType() const { return GetStaticStateType(); } \
	virtual const hsm_char* DoGetStateDebugName() const { return HSM_TEXT(#__StateName__); }

#endif // !HSM_CPP_RTTI

#endif // HSM_RTTI_H
