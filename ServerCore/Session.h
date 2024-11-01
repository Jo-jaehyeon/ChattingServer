#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"

class Service;

/*-----------------
	  Session
-----------------*/
class Session : public IocpObject
{
	enum
	{
		BUFFER_SIZE=0x10000,
	};

	friend class Listener;
	friend class IocpCore;
	friend class Service;

public:
	Session();
	virtual ~Session();

public:	
						/* 외부에서 사용 */
	void				Send(SendBufferRef sendBuffer);
	void				Disconnect(const WCHAR* cause);
	bool				Connect();

	shared_ptr<Service> GetService() { return _service.lock(); }
	void				SetService(shared_ptr<Service> service) { _service = service; }
public:
						/* 정보 관련 */
	void				SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress			GetAddress() { return _netAddress; }
	SOCKET				GetSocket() { return _socket; }
	bool				IsConnected() { return _connected; }
	SessionRef			GetSessionRef() {
		return static_pointer_cast<Session>(shared_from_this());
	}

private:
						/* 전송 관련 */
	bool				RegisterConnect();
	bool				RegisterDisconnect();
	void				RegisterRecv();
	void				RegisterSend();

	void				ProcessConnect();
	void				ProcessDisconnect();
	void				ProcessRecv(int32 numOfBytes);
	void				ProcessSend(int32 numOfBytes);

	void				HandleError(int32 errorCode);
	
private:
						/* 인터페이스 구현 */
	virtual HANDLE		GetHandle() override;
	virtual void		Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

protected:
	virtual void		OnConnected();
	virtual int32		OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void		OnSend(int32 len) { }
	virtual void		OnDisconnected() { }

private:
	weak_ptr<Service>	_service;

private:
	USE_LOCK;

	/* susin guawnryun */
	RecvBuffer _recvBuffer;

	/**/
	Queue<SendBufferRef> _sendQueue;
	Atomic<bool> _sendRegisterd = false;

private:
						/* IocpEvent 재사용 */
	RecvEvent			_recvEvent;
	ConnectEvent		_connectEvent;
	DisconnectEvent		_disconnectEvent;
	SendEvent			_sendEvent;

private:
	SOCKET				_socket = INVALID_SOCKET;
	NetAddress			_netAddress = {};
	Atomic<bool>		_connected = false;
};

/*-----------------
	PacketSession
-----------------*/
struct PacketHeader
{
	uint16 size;
	uint16 id;
};
class PacketSession : public Session
{
public:
	PacketSession();
	virtual ~PacketSession();

	PacketSessionRef GetPacketSessionRef() { return static_pointer_cast<PacketSession>(shared_from_this()); }

protected:
	virtual int32 OnRecv(BYTE* buffer, int32 len) sealed;
	virtual void OnRecvPacket(BYTE* bufer, int32 len) abstract;
};