/*
 * status.hpp
 *
 *  Created on: Sep 23, 2024
 *      Author: matt
 */

#ifndef PLATFORM_INC_STATUS_HPP_
#define PLATFORM_INC_STATUS_HPP_


#include <cstdint>
#include <mutex>

#include "lua.hpp"


constexpr uint16_t PON_BIT = 7;
constexpr uint16_t URQ_BIT = 6;
constexpr uint16_t CME_BIT = 5;
constexpr uint16_t ERR_BIT = 4;
constexpr uint16_t DDE_BIT = 3;
constexpr uint16_t QYE_BIT = 2;
constexpr uint16_t RQC_BIT = 1;
constexpr uint16_t OPC_BIT = 0;

constexpr uint16_t RQS_BIT = 6;
constexpr uint16_t MSS_BIT = 6;
constexpr uint16_t ESB_BIT = 5;
constexpr uint16_t MAV_BIT = 4;


void StatusInstall(lua_State *lState);


enum StatusFunctions {
	FUNC_GET_CONDITION_REGISTER = 0,
	FUNC_GET_EVENT_REGISTER,
	FUNC_SET_EVENT_REGISTER,
	FUNC_GET_EVENT_ENABLE_REGISTER,
	FUNC_SET_EVENT_ENABLE_REGISTER,
	FUNC_GET_POSITIVE_TRANS_REGISTER,
	FUNC_SET_POSITIVE_TRANS_REGISTER,
	FUNC_GET_NEGATIVE_TRANS_REGISTER,
	FUNC_SET_NEGATIVE_TRANS_REGISTER,
	FUNC_GET_SERVICE_REQUEST_ENABLE_REGISTER,
	FUNC_SET_SERVICE_REQUEST_ENABLE_REGISTER,
};

/*
 * Status Data Structure - Register Model
 * IEEE488.2 - 11.4
 * The Status Data Structure provides a common way to perform status reporting.
 * The status data structure has a single “output,” a summary message that summarizes the structure's state.
 */
class StatusRegister {
public:
	StatusRegister(void) : StatusRegister{0} { }
	explicit StatusRegister(uint16_t lValue) : mValue{lValue}, mLock{} { }
	explicit StatusRegister(StatusRegister& lOther) : StatusRegister{lOther.Get()} { }

	inline void Lock(void) { mLock.lock(); }
	inline bool TryLock(void) { return mLock.try_lock(); }
	inline void Unlock(void) { mLock.unlock(); }

	inline uint16_t Get(void) const { return mValue; }
	inline void Set(uint16_t lValue) { Lock(); mValue = lValue; Unlock(); }
	inline void Clear(void) { Lock(); mValue = 0; Unlock(); }
	inline void ClearBits(uint16_t lBitMask) { Lock(); mValue &= (~lBitMask); Unlock(); }
	inline void ClearBits(const StatusRegister& lOther) { ClearBits(lOther.Get()); }
	inline void SetBits(uint16_t lBitMask) { Lock(); mValue |= lBitMask; Unlock(); }
	inline void SetBits(const StatusRegister& lOther) { SetBits(lOther.Get()); }

	inline StatusRegister operator&(const StatusRegister& lOther) const { return StatusRegister(lOther.mValue & this->mValue); }
	inline StatusRegister operator|(const StatusRegister& lOther) const { return StatusRegister(lOther.mValue | this->mValue); }
	inline StatusRegister& operator&=(const StatusRegister& lOther) { this->Set(this->Get() & lOther.Get()); return *this; }
	inline StatusRegister& operator|=(const StatusRegister& lOther) { this->Set(this->Get() | lOther.Get()); return *this; }

private:
	uint16_t mValue = 0;
	std::mutex mLock;
};

class ConditionRegister : public StatusRegister {
public:
	ConditionRegister(void) : StatusRegister() { }
	explicit ConditionRegister(uint16_t lValue) : StatusRegister(lValue) { }
};

class EventRegister : public StatusRegister {
public:
	EventRegister(void) : StatusRegister() { }
	explicit EventRegister(uint16_t lValue) : StatusRegister(lValue) { }
};

class EventEnableRegister : public StatusRegister {
public:
	EventEnableRegister(void) : StatusRegister() { }
	explicit EventEnableRegister(uint16_t lValue) : StatusRegister(lValue) { }
};

class TransitionRegister : public StatusRegister {
public:
	TransitionRegister(void) : StatusRegister() { }
	explicit TransitionRegister(uint16_t lValue) : StatusRegister(lValue) { }
};

class ServiceRequestEnableRegister : public StatusRegister {
public:
	ServiceRequestEnableRegister(void) : StatusRegister() { }
	explicit ServiceRequestEnableRegister(uint16_t lValue) : StatusRegister(lValue) { }
};

class StatusByteRegister : public StatusRegister {
public:
	StatusByteRegister(void) : StatusRegister() { }
	explicit StatusByteRegister(uint16_t lValue) : StatusRegister(lValue) { }
};


/*
 * StatusDataStructure - IEEE 488.2 Section 11.5
 * Based on the Standard Event Status Register Model (11.5.1)
 */
class StatusDataStructure {
private:
	ConditionRegister mConditionRegister;
	EventRegister mEventRegister;
	EventEnableRegister mEventEnableRegister;
	TransitionRegister mPositiveTransitionRegister;
	TransitionRegister mNegativeTransitionRegister;
	ServiceRequestEnableRegister mServiceRequestEnableRegister;

	StatusByteRegister mStatusByteRegister;
public:
	StatusDataStructure(void);

	static int HandleMsg(lua_State *lState);

	inline void ClearConditionRegister(void) { mConditionRegister.Clear(); }
	inline void SetConditionRegister(uint16_t lConditionRegister) {  mConditionRegister.Set(lConditionRegister); }
	inline void ClearConditionRegisterBits(uint16_t lBitMask) { mConditionRegister.ClearBits(lBitMask); }
	inline void SetConditionRegisterBits(uint16_t lBitMask) { mConditionRegister.SetBits(lBitMask); }
	inline uint16_t GetConditionRegister(void) { return mConditionRegister.Get(); }

	inline void ClearEventRegister(void) { mEventRegister.Clear(); }
	inline void SetEventRegister(uint16_t lEventRegister) { mEventRegister.Set(lEventRegister); }
	inline void ClearEventRegisterBits(uint16_t lBitMask) { mEventRegister.ClearBits(lBitMask); }
	inline void SetEventRegisterBits(uint16_t lBitMask) { mEventRegister.SetBits(lBitMask); }
	inline uint16_t GetEventRegister(void) { return mEventRegister.Get(); }

	inline void ClearEventEnableRegister(void) { mEventEnableRegister.Clear(); }
	inline void SetEventEnableRegister(uint16_t lEventEnableRegister) { mEventEnableRegister.Set(lEventEnableRegister); }
	inline void ClearEventEnableRegisterBits(uint16_t lBitMask) { mEventEnableRegister.ClearBits(lBitMask); }
	inline void SetEventEnableRegisterBits(uint16_t lBitMask) { mEventEnableRegister.SetBits(lBitMask); }
	inline uint16_t GetEventEnableRegister(void) { return mEventEnableRegister.Get(); }

	inline void ClearPositiveTransitionRegister(void) { mPositiveTransitionRegister.Clear(); }
	inline void SetPositiveTransitionRegister(uint16_t lPositiveTransitionRegister) { mPositiveTransitionRegister.Set(lPositiveTransitionRegister); }
	inline void ClearPositiveTransitionRegisterBits(uint16_t lBitMask) { mPositiveTransitionRegister.ClearBits(lBitMask); }
	inline void SetPositiveTransitionRegisterBits(uint16_t lBitMask) { mPositiveTransitionRegister.SetBits(lBitMask); }
	inline uint16_t GetPositiveTransitionRegister(void) { return mPositiveTransitionRegister.Get(); }

	inline void ClearNegativeTransitionRegister(void) { mNegativeTransitionRegister.Clear(); }
	inline void SetNegativeTransitionRegister(uint16_t lNegativeTransitionRegister) { mNegativeTransitionRegister.Set(lNegativeTransitionRegister); }
	inline void ClearNegativeTransitionRegisterBits(uint16_t lBitMask) { mNegativeTransitionRegister.ClearBits(lBitMask); }
	inline void SetNegativeTransitionRegisterBits(uint16_t lBitMask) { mNegativeTransitionRegister.SetBits(lBitMask); }
	inline uint16_t GetNegativeTransitionRegister(void) { return mNegativeTransitionRegister.Get(); }

	inline void ClearServiceRequestEnableRegister(void) { mServiceRequestEnableRegister.Clear(); }
	inline void SetServiceRequestEnableRegister(uint16_t lServiceRequestEnableRegister) { mServiceRequestEnableRegister.Set(lServiceRequestEnableRegister); }
	inline void ClearServiceRequestEnableRegisterBits(uint16_t lBitMask) { mServiceRequestEnableRegister.ClearBits(lBitMask); }
	inline void SetServiceRequestEnableRegisterBits(uint16_t lBitMask) { mServiceRequestEnableRegister.SetBits(lBitMask); }
	inline uint16_t GetServiceRequestEnableRegister(void) { return mServiceRequestEnableRegister.Get(); }

	inline void ClearStatusByteRegister(void) { mStatusByteRegister.Clear(); }
	inline void SetStatusByteRegister(uint16_t lStatusByteRegister) { mStatusByteRegister.Set(lStatusByteRegister); }
	inline void ClearStatusByteRegisterBits(uint16_t lBitMask) { mStatusByteRegister.ClearBits(lBitMask); }
	inline void SetStatusByteRegisterBits(uint16_t lBitMask) { mStatusByteRegister.SetBits(lBitMask); }
	inline uint16_t GetStatusByteRegister(void) { return mStatusByteRegister.Get(); }
};

extern StatusDataStructure gPlatformStatus;

namespace StatusModelApi
{
	void SetConditionRegister(StatusDataStructure& lStatus, uint16_t lValue);
	void ClearConditionRegister(StatusDataStructure& lStatus);
	uint16_t GetConditionRegister(StatusDataStructure& lStatus);

	void SetEventRegister(StatusDataStructure& lStatus, uint16_t lValue);
	void ClearEventRegister(StatusDataStructure& lStatus);
	uint16_t GetEventRegister(StatusDataStructure& lStatus);
	void UpdateEventRegister(StatusDataStructure& lStatus);

	void SetEventEnableRegister(StatusDataStructure& lStatus, uint16_t lValue);
	void ClearEventEnableRegister(StatusDataStructure& lStatus);
	uint16_t GetEventEnableRegister(StatusDataStructure& lStatus);

	uint16_t Summarize(StatusDataStructure& lStatus);
}

#endif /* PLATFORM_INC_STATUS_HPP_ */
