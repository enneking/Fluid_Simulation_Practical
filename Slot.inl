
template<class TType, class TListener>
Slot<TType, TListener>::Slot(TListener* const pClass, void(TListener::* const pMethod)(TType &tOldValue))
	: m_pClass(pClass), m_pMethod(pMethod)
{
}

template<class TType, class TListener>
Slot<TType, TListener>::~Slot()
{
}

template<class TType, class TListener>
void Slot<TType, TListener>::Process(TType &tOldValue)
{
	(m_pClass->*(m_pMethod))(tOldValue);
}