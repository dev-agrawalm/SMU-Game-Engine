#pragma once
#include<map>
#include<string>
#include<vector>
#include <mutex>
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

typedef NamedProperties EventArgs;
typedef bool (*EventCallbackFunction) (EventArgs& args);

struct EventSubscriptionBase 
{
public:
	virtual ~EventSubscriptionBase();
	virtual bool Execute(EventArgs& args) = 0;
	
public:
	int	m_numTimesTriggered	= 0;
};


struct EventFunctionSubscription : public EventSubscriptionBase
{
public:
	virtual bool Execute(EventArgs& args) override;

public:
	EventCallbackFunction m_callbackFunctionPtr = nullptr;
};


template <typename T_ObjectType>
struct EventMethodSubscription : public EventSubscriptionBase 
{
public:
	typedef bool (T_ObjectType::*EventCallbackMethod) (EventArgs& args);
	EventMethodSubscription(T_ObjectType& object, EventCallbackMethod callbackMethod)
		: m_object(object)
		, m_callbackMethodPtr(callbackMethod)
	{
		//does nothing
	}

	virtual bool Execute(EventArgs& args) override;

public:
	T_ObjectType& m_object;
	EventCallbackMethod m_callbackMethodPtr = nullptr;
};


template <typename T_ObjectType>
bool EventMethodSubscription<T_ObjectType>::Execute(EventArgs& args)
{
	m_numTimesTriggered++;
	return (m_object.*m_callbackMethodPtr)(args);
}


typedef std::vector<EventSubscriptionBase*> EventSubscriptions;

struct EventSystemConfig
{
};

class EventSystem
{
public:
	EventSystem(EventSystemConfig const& config);
	~EventSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	template <typename T_ObjectType, typename T_MethodType>
	void SubscribeEventCallbackObjectMethod(std::string const& eventName, T_ObjectType& object, T_MethodType methodPtr);

	template <typename T_ObjectType, typename T_MethodType>
	void UnsubscribeEventCallbackObjectMethod(std::string const& eventName, T_ObjectType& object, T_MethodType methodPtr);
	
	void SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction funcPtr);
	void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction funcPtr);
	void FireEvent(std::string const& eventName);
	void FireEvent(std::string const& eventName, EventArgs& args);

	bool DoesEventExist(std::string const& eventName);
	std::vector<std::string> GetAllEventNames();

private:
	EventSystemConfig m_config;
	std::mutex m_eventsMutex;
	std::map<std::string, EventSubscriptions> m_subscriptionsListByEventName;
};


template <typename T_ObjectType, typename T_MethodType>
void EventSystem::SubscribeEventCallbackObjectMethod(std::string const& eventName, T_ObjectType& object, T_MethodType methodPtr)
{
	GUARANTEE_OR_DIE(methodPtr != nullptr, Stringf("Trying to subscribe a null function to event : %s", eventName.c_str()))
	EventMethodSubscription<T_ObjectType>* methodSubcription = new EventMethodSubscription<T_ObjectType>(object, methodPtr);

	m_eventsMutex.lock();
	m_subscriptionsListByEventName[eventName].push_back(methodSubcription);
	m_eventsMutex.unlock();
}


template <typename T_ObjectType, typename T_MethodType>
void EventSystem::UnsubscribeEventCallbackObjectMethod(std::string const& eventName, T_ObjectType& object, T_MethodType methodPtr)
{
	m_eventsMutex.lock();
	EventSubscriptions& eventSubscriptions = m_subscriptionsListByEventName[eventName];
	m_eventsMutex.unlock();
	for (int subscriptionIndex = 0; subscriptionIndex < (int) eventSubscriptions.size(); subscriptionIndex++)
	{
		EventSubscriptionBase*& eventSubscription = eventSubscriptions[subscriptionIndex];
		EventMethodSubscription<T_ObjectType>* eventSubscription_asMethodSubscription = dynamic_cast<EventMethodSubscription<T_ObjectType>*>(eventSubscription);
		if (eventSubscription_asMethodSubscription && eventSubscription_asMethodSubscription->m_callbackMethodPtr)
		{
			if (&eventSubscription_asMethodSubscription->m_object == &object && eventSubscription_asMethodSubscription->m_callbackMethodPtr == methodPtr)
			{
				eventSubscription_asMethodSubscription->m_callbackMethodPtr = nullptr;
				delete eventSubscription;
				eventSubscription = nullptr;
			}
		}
	}
}
