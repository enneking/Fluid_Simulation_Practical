#pragma once

template< class TType >
class SlotBase {
public:
	SlotBase() 
	{};
	virtual ~SlotBase() 
	{};

	virtual void Process(TType &tOldValue) = 0;

};

template<class TType, class TListener>
class Slot : public SlotBase<TType>{
public:

	explicit Slot(TListener* const pClass, void(TListener::* const pMethod)(TType &tOldValue));
	~Slot();

	void Process(TType &tOldValue);

public:
	TListener* const m_pClass;
	void(TListener::* const m_pMethod)(TType &);
};



#include"Slot.inl"