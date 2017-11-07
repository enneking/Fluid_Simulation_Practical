#pragma once
#include "Slot.h"
#include <vector>
#include <memory>


template<class TType>
class Field {
public:
	Field();
	explicit Field( TType &tValue );
	~Field();

	TType operator()();
	void Set(TType tValue);

	template<class TListener>
	void AddSlot(TListener* const pClass, void(TListener::* const pMethod)(TType &tOldValue));

	

	void SetLock(bool bLock);


private:
	void Raise();
	std::vector<std::shared_ptr<SlotBase<TType>>> m_mMethodContainer;
	TType m_tValue;
	bool m_bLock = false;
};


#include "Field.inl"