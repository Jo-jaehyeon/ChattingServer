#include "pch.h"
#include "Service.h"
#include "Session.h"

Service::Service(ServiceType type, NetAddress netAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	:_type(type), _netAddress(netAddress), _iocpCore(core), _sessionFactory(factory), _maxSessionCount(maxSessionCount)
{
}

Service::~Service()
{
}

void Service::CloseService()
{
	// TODO
}

void Service::SetSessionFactory()
{
}

void Service::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (const auto& session : _sessions)
	{
		session->Send(sendBuffer);
	}
}

SessionRef Service::CreateSession()
{
	SessionRef Session = _sessionFactory();
	Session->SetService(shared_from_this());

	if (_iocpCore->Register(Session) == false)
		return nullptr;

	return Session;
}

void Service::AddSession(SessionRef session)
{
	WRITE_LOCK;
	_sessionCount++;
	_sessions.insert(session);
}

void Service::ReleaSession(SessionRef session)
{
	WRITE_LOCK;
	ASSERT_CRASH(_sessions.erase(session) != 0);
	_sessionCount--;
}

/*----------------------
	  ClientService
----------------------*/
ClientService::ClientService(NetAddress netAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	: Service(ServiceType::Client, netAddress, core, factory, maxSessionCount)
{
}

bool ClientService::Start()
{
	if (CanStart() == false)
		return false;

	const int32 sessionCount = GetMaxSessionCount();

	for (int32 i = 0; i < sessionCount; i++)
	{
		SessionRef session = CreateSession();
		if (session->Connect() == false)
			return false;
	}
	
	return true;
}

/*----------------------
	  ServerService
----------------------*/
ServerService::ServerService(NetAddress netAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	: Service(ServiceType::Server, netAddress, core, factory, maxSessionCount)
{
}

bool ServerService::Start()
{
	if (CanStart() == false)
		return false;
	
	_listener = MakeShared<Listener>();
	if (_listener == nullptr)
		return false;

	ServerServiceRef service = static_pointer_cast<ServerService>(shared_from_this());
	if (_listener->StartAccept(service) == false)
		return false;
	
	return true;
}

void ServerService::CloseService()
{
}
