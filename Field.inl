template<class TType>
Field<TType>::Field() : m_tValue(TType())
{
}

template<class TType>
Field<TType>::Field( TType &tValue ) : m_tValue(tValue)
{
}

template<class TType>
Field<TType>::~Field()
{
}

template<class TType>
TType Field<TType>::operator()()
{
	return m_tValue;
}

template<class TType>
void Field<TType>::Set(TType tValue)
{
	
	
	if (m_bLock == true || m_tValue == tValue)
	{
		return;
	}

	m_bLock = true;
	TType tOldValue = m_tValue;
	m_tValue = tValue;	
	for (size_t i = 0; i < m_mMethodContainer.size(); i++)
	{
		m_mMethodContainer[i]->Process(tOldValue);
	}

	m_bLock = false;
}

template<class TType>
template<class TListener>
void Field<TType>::AddSlot(TListener* const pClass, void(TListener::* const pMethod)(TType &tOldValue))
{
	std::shared_ptr<Slot<TType, TListener>> oSlot(std::make_shared<Slot<TType, TListener>>(pClass, pMethod));
	m_mMethodContainer.push_back(oSlot);
}

template<class TType>
void Field<TType>::SetLock(bool bLock)
{
	m_bLock = bLock;
}