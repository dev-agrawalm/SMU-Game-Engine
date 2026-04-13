#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"

EventSystem* g_eventSystem = nullptr;

EventSystem::EventSystem(EventSystemConfig const& config)
	: m_config(config)
{

}


EventSystem::~EventSystem()
{

}


void EventSystem::Startup()
{
}


void EventSystem::Shutdown()
{

}


void EventSystem::BeginFrame()
{

}


void EventSystem::EndFrame()
{

}


void EventSystem::SubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction funcPtr)
{
	GUARANTEE_OR_DIE(funcPtr != nullptr, Stringf("Trying to subscribe a null function to event : %s", eventName.c_str()));

	EventFunctionSubscription* funcSubscription = new EventFunctionSubscription();
	funcSubscription->m_callbackFunctionPtr = funcPtr;
	funcSubscription->m_numTimesTriggered = 0;

	m_eventsMutex.lock();
	m_subscriptionsListByEventName[eventName].push_back(funcSubscription);
	m_eventsMutex.unlock();
}


void EventSystem::UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallbackFunction funcPtr)
{
	m_eventsMutex.lock();
	EventSubscriptions& eventSubscriptions = m_subscriptionsListByEventName[eventName];
	m_eventsMutex.unlock();
	for (int subscriptionIndex = 0; subscriptionIndex < (int) eventSubscriptions.size(); subscriptionIndex++)
	{
		EventSubscriptionBase*& eventSubscription = eventSubscriptions[subscriptionIndex];
		EventFunctionSubscription* eventSubscription_asFuncSubscription = dynamic_cast<EventFunctionSubscription*>(eventSubscription);
		if (eventSubscription_asFuncSubscription && eventSubscription_asFuncSubscription->m_callbackFunctionPtr)
		{
			if (eventSubscription_asFuncSubscription->m_callbackFunctionPtr == funcPtr)
			{
				eventSubscription_asFuncSubscription->m_callbackFunctionPtr = nullptr;
				delete eventSubscription;
				eventSubscription = nullptr;
			}
		}
	}
}


void EventSystem::FireEvent(std::string const& eventName)
{
	EventArgs emptyArgs;
	FireEvent(eventName, emptyArgs);
}


void EventSystem::FireEvent(std::string const& eventName, EventArgs& args)
{
	m_eventsMutex.lock();
	EventSubscriptions& eventSubscriptions = m_subscriptionsListByEventName[eventName];
	m_eventsMutex.unlock();
	for (int subscriptionIndex = 0; subscriptionIndex < (int) eventSubscriptions.size(); subscriptionIndex++)
	{
		EventSubscriptionBase* eventSubscription = eventSubscriptions[subscriptionIndex];
		if (eventSubscription) 
		{
			bool wasConsumed = eventSubscription->Execute(args);
			if (wasConsumed) 
			{
				break;
			}
		}
	}
}


bool EventSystem::DoesEventExist(std::string const& eventName)
{
	m_eventsMutex.lock();
	auto it = m_subscriptionsListByEventName.find(eventName);

	if (it != m_subscriptionsListByEventName.end())
	{
		m_eventsMutex.unlock();
		return true;
	}

	m_eventsMutex.unlock();
	return false;
}


std::vector<std::string> EventSystem::GetAllEventNames()
{
	std::vector<std::string> eventNames;

	m_eventsMutex.lock();
	auto it = m_subscriptionsListByEventName.begin();
	while (it != m_subscriptionsListByEventName.end())
	{
		eventNames.push_back(it->first);
		it++;
	}

	m_eventsMutex.unlock();
	return eventNames;
}


EventSubscriptionBase::~EventSubscriptionBase()
{

}


bool EventFunctionSubscription::Execute(EventArgs& args)
{
	m_numTimesTriggered++;
	return m_callbackFunctionPtr(args);
}

