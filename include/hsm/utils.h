// Hierarchical State Machine (HSM)
//
// Copyright (c) 2013 Antonio Maiorano
//
// Distributed under the MIT License (MIT)
// (See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT)

/// \file utils.h
/// \brief Internal library utilities.

#ifndef HSM_UTILS_H
#define HSM_UTILS_H

#include "config.h"

namespace hsm {
namespace util {

// IntrusivePtr is a ref-counting smart pointer that deletes the pointed-to object when its
// ref count reaches 0. The pointed-to object is expected to implement AddRef/RemoveRef
// (see IntrusivePtrClient below for a default implementation that can be used).
// @NOTE: object is required to be allocated using HSM_NEW.
template <typename T>
class IntrusivePtr
{
public:
	typedef IntrusivePtr<T> ThisType;

	IntrusivePtr(T* object = 0) : mObject(object)
	{
		InvokeAddRef();
	}

	~IntrusivePtr()
	{
		InvokeRemoveRef();
	}

	IntrusivePtr(const ThisType& rhs) : mObject(rhs.Get())
	{
		InvokeAddRef();
	}

	template <typename U>
	IntrusivePtr(const IntrusivePtr<U>& rhs) : mObject(rhs.Get())
	{
		InvokeAddRef();
	}

	ThisType& operator=(const ThisType& rhs)
	{
		// This neat "swap" trick uses the constructor/destructor to do the work.
		ThisType(rhs).swap(*this);
		return *this;
	}

	template <typename U>
	ThisType& operator=(const IntrusivePtr<U>& rhs)
	{
		ThisType(rhs).swap(*this);
		return *this;
	}

	// Since the object holds its own ref count, we can safely assign an object
	// directly to an IntrusivePtr, even if another IntrusivePtr already points
	// to it. Note that this is not possible with shared_ptr, so this assignment
	// operator is not made available for shared_ptr.
	ThisType& operator=(T* object)
	{
		Reset(object);
		return *this;
	}

	void Reset(T* object = 0)
	{
		ThisType(object).swap(*this);
	}

	T* Get() const { return mObject; }

	T* operator->()
	{
		return mObject;
	}
	
	const T* operator->() const
	{
		return mObject;
	}

	T& operator*()
	{
		return *mObject;
	}
	
	const T& operator*() const
	{
		return *mObject;
	}

	void swap(ThisType& rhs)
	{
		T* lhsObject = mObject;
		mObject = rhs.mObject;
		rhs.mObject = lhsObject;
	}

private:
	void InvokeAddRef()
	{
		if (mObject)
		{
			mObject->AddRef();
		}
	}

	void InvokeRemoveRef()
	{
		if (mObject && mObject->RemoveRef() == 0)
		{
			HSM_DELETE(mObject);
			mObject = 0;
		}
	}

	T* mObject;
};

// Optional class that T can derive from to add required functionality for IntrusivePtr<T>
class IntrusivePtrClient
{
public:
	IntrusivePtrClient() : mRefCount(0) {}
	virtual ~IntrusivePtrClient() {}

	void AddRef() const
	{
		++mRefCount;
	}

	int RemoveRef() const
	{
		HSM_ASSERT(mRefCount > 0);
		--mRefCount;
		return mRefCount;
	}

private:
	mutable int mRefCount;
};

} // namespace util
} // namespace hsm

#endif // HSM_UTILS_H
