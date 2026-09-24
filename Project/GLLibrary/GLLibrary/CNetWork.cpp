#include "CNetWork.h"

std::map<std::string, CNetWork*> CNetWork::m_instances;
CSocket::CSocket(SOCKET &s, sockaddr_in &client, unsigned long bufferSize, std::function<void(CSocket*, char*, int)> recvCallback, std::function<void(CSocket*)> connectCallback, std::function<void(CSocket*)> disconnectCallback,bool async,bool udp) :m_isDestroy(false),m_thread(nullptr){
	m_sock = s;
	m_bufferSize = bufferSize;
	m_recvbuffer = new char[m_bufferSize];
	m_recvDataSize = 0;
	m_sendbuffer = new char[m_bufferSize];
	m_recvCallback = recvCallback;
	m_connectCallback = connectCallback;
	m_disconnectCallback = disconnectCallback;
	m_sockaddr = m_sendaddr = client;
	m_errorState = eNone_Error;
	m_async = async;
	m_udp = udp;
	if (m_connectCallback) m_connectCallback(this);
	if (m_async) {
		m_thread = new std::thread(std::bind(&CSocket::RecvProc, this));
		SetThreadPriority(m_thread->native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
	}
}
CSocket::~CSocket() {
	m_isDestroy = true;
	if (m_disconnectCallback) m_disconnectCallback(this);
	if (m_thread) {
		m_thread->join();
		delete m_thread;
	}
	delete m_recvbuffer;
	delete m_sendbuffer;
}
void CSocket::RecvProc() {
	while (!m_isDestroy) {
		// サーバからデータを受信
		memset(m_recvbuffer + m_recvDataSize, 0, m_bufferSize - m_recvDataSize);
		int n = 0;
		if (m_udp) {
			int addrlen;
			addrlen = sizeof(m_sendaddr);
			n = recvfrom(m_sock, m_recvbuffer + m_recvDataSize, m_bufferSize - m_recvDataSize, 0, (struct sockaddr *)&m_sendaddr, &addrlen);
		} else {
			n = recv(m_sock, m_recvbuffer + m_recvDataSize, m_bufferSize - m_recvDataSize, 0);
		}
		if (n != SOCKET_ERROR) {
			if (m_recvDataSize == 0) {
				memcpy(&m_recvHeader, m_recvbuffer, sizeof(SBufferHeader));
				if (m_recvHeader.id[0] == 'N' && m_recvHeader.id[1] == 'T') {

				} else {
					printf("\n---------------\nrecv heder err\nー-----------\n");
				}
			}
			m_recvDataSize += n;
			if (m_recvDataSize >= m_recvHeader.size) {
				m_recvCallback(this, m_recvbuffer+ sizeof(SBufferHeader), m_recvHeader.size- sizeof(SBufferHeader));
				m_recvDataSize = 0;
			}
			else {
				printf("分割受信中 %d/%d\n", m_recvDataSize, m_recvHeader.size);

			}
		}
		else {
			printf("\n---------------\nrecv error\nー-----------\n");
			//m_errorState = eRecv_Error;
			//m_isDestroy = true;
		}
		if (!m_async) break;
	}
}
bool CSocket::Send(const char *buf, int len) {
	m_sendHeader.id[0] = 'N'; m_sendHeader.id[1] = 'T';
	m_sendHeader.ver = 0x100;
	m_sendHeader.size = len + sizeof(SBufferHeader);
	memcpy(m_sendbuffer,&m_sendHeader,sizeof(SBufferHeader));
	memcpy(m_sendbuffer + sizeof(SBufferHeader),buf, len);
	int s = 0;
	while (s<len) {
		if (m_udp) {
			s += sendto(m_sock, m_sendbuffer, len + sizeof(SBufferHeader), 0, (struct sockaddr *)&m_sendaddr, sizeof(sockaddr_in));
		} else {
			s += send(m_sock, m_sendbuffer, len + sizeof(SBufferHeader), 0);
		}
		if (s == SOCKET_ERROR) {
			printf("send err\n");
			//m_isDestroy = true;
			return false;
		}
		if (s<len) printf("分割送信 %d/%d\n", s, m_sendHeader.size);
	}
	return true;
}
CNetWork::CNetWork(unsigned long bufferSize, std::function<void(CSocket*, char*, int)> recvCallback, std::function<void(CSocket*)> connectCallback, std::function<void(CSocket*)> disconnectCallback, bool async) : m_thread(nullptr){
	m_bufferSize = bufferSize;
	m_buffer = new char[m_bufferSize];
	m_recvCallback = recvCallback;
	m_connectCallback = connectCallback;
	m_disconnectCallback = disconnectCallback;
	m_state = eNetState_None;
	m_async = async;
	FD_ZERO(&m_readfds);
}
CNetWork::~CNetWork() {
	if (m_state != eNetState_None) {
		End();
	}
	delete m_buffer;
}

void CNetWork::AcceptProc() {
	while (!m_isDestroy) {
		/*		fd_set mask; FD_ZERO(&mask); FD_SET(m_sock, &mask);
		struct timeval tv = { 10,0 };
		int rc = select((int)m_sock + 1, &mask, NULL, NULL, &tv);
		if (rc == SOCKET_ERROR) fprintf(stderr, "accept-select error:%d\n", WSAGetLastError());
		*/
		struct sockaddr_in client = { 0, };
		int len;
		// TCPクライアントからの接続要求を受け付ける
		len = sizeof(sockaddr_in);
		SOCKET clientSock;
		clientSock = accept(m_sock, (struct sockaddr *)&client, &len);
		if (clientSock != INVALID_SOCKET) {

			FD_SET(clientSock, &m_readfds);
			m_sockets.push_back(new CSocket(clientSock, client, m_bufferSize, m_recvCallback, m_connectCallback, m_disconnectCallback, m_async,m_udp));
			printf("accepted connection from %s, port=%d\n",
				inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		}
		if (!m_async) break;
	}


}
CNetWork* CNetWork::GetInstance(std::string name)
{
	return m_instances[name];
}
void CNetWork::ClearInstance(std::string name)
{
	if (!m_instances[name]) return;
	delete m_instances[name];
	m_instances[name] = nullptr;
}
void CNetWork::CreateInstance(std::string name, unsigned long bufferSize, std::function<void(CSocket*, char*, int)> recvCallback, std::function<void(CSocket*)> connectCallback, std::function<void(CSocket*)> disconnectCallback, bool async)
{
	if (!m_instances[name]) m_instances[name] = new CNetWork(bufferSize, recvCallback, connectCallback, disconnectCallback, async);
}

void CNetWork::StartServer(unsigned short port, bool udp) {
	m_udp = udp;
	// winsock2の初期化
	WSAStartup(MAKEWORD(2, 0), &m_wsaData);

	// ソケットの作成
	int type=SOCK_STREAM;
	if (udp) type = SOCK_DGRAM;
	m_sock = socket(AF_INET, type, 0);

	// ソケットの設定
	m_server.sin_family = AF_INET;
	m_server.sin_port = htons(port);
	m_server.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(m_sock, (struct sockaddr *)&m_server, sizeof(m_server));
	m_isDestroy = false;
	m_state = eNetState_Server;
	if (udp) {
		if (m_async)
			m_sockets.push_back(new CSocket(m_sock, m_server, m_bufferSize, m_recvCallback, m_connectCallback, m_disconnectCallback, m_async, udp));

	} else {


		/*
		// 10秒でタイムアウトするようにします
		m_tv.tv_sec = 10;
		m_tv.tv_usec = 0;

		memcpy(&m_fds, &m_readfds, sizeof(fd_set));
		select(0, &m_fds, NULL, NULL, &m_tv);
		*/

		// TCPクライアントからの接続要求を待てる状態にする
		listen(m_sock, 5);

		if (m_async) m_thread = new std::thread(std::bind(&CNetWork::AcceptProc, this));
	}
	FD_SET(m_sock, &m_readfds);
	printf("server start..\n");

}
bool CNetWork::StartClient(unsigned short port, const char *ipAddr, bool udp) {
	m_udp = udp;
	// winsock2の初期化
	WSAStartup(MAKEWORD(2, 0), &m_wsaData);

	// ソケットの作成
	int type = SOCK_STREAM;
	if (udp) type = SOCK_DGRAM;
	m_sock = socket(AF_INET, type, 0);

	// 接続先指定用構造体の準備
	m_server.sin_family = AF_INET;
	m_server.sin_port = htons(port);
	m_server.sin_addr.S_un.S_addr = inet_addr(ipAddr);
	m_state = eNetState_Cilent;
	if (udp) {
		//UDPの場合はソケット作成のみ
		m_sockets.push_back(new CSocket(m_sock, m_server, m_bufferSize, m_recvCallback, m_connectCallback, m_disconnectCallback, m_async, m_udp));
		return true;
	}
	// サーバに接続
	int st = connect(m_sock, (struct sockaddr *)&m_server, sizeof(m_server));
	if (st == SOCKET_ERROR) {
		printf("connect error..[%s]\n", ipAddr);
		m_state = eNetState_None;
		return false;
	}
	else {
		m_sockets.push_back(new CSocket(m_sock, m_server, m_bufferSize, m_recvCallback, m_connectCallback, m_disconnectCallback, m_async,m_udp));
	
		printf("client start..\n");
	}
	return true;
	
}

void CNetWork::SelectProc() {
	if (m_async) return;

	m_tv.tv_sec = 5;
	m_tv.tv_usec = 0;

	memcpy(&m_fds, &m_readfds, sizeof(fd_set));

	int n = select((int)m_sockets.size()+1, &m_fds, NULL, NULL, &m_tv);
	if (n > 0) {
		if (m_udp) {

		} else {
			if (FD_ISSET(m_sock, &m_fds)) {
				AcceptProc();
			}
			for (auto it : m_sockets) {
				if (FD_ISSET(*(it->GetSocket()), &m_fds)) {
					it->RecvProc();
				}
			}
		}
	}
}
void CNetWork::DestroyCheck() {
	auto it = m_sockets.begin();
	while (it != m_sockets.end()) {
		if ((*it)->m_isDestroy) {
			delete *it;
			it = m_sockets.erase(it);
		} else {
			it++;
		}
	}
}
void CNetWork::End() {
	// TCPセッションの終了
	closesocket(m_sock);

	auto it = m_sockets.begin();
	while (it != m_sockets.end()) {
		delete (*it);
		it++;
	}
	m_isDestroy = true;
	if (m_thread) {
		m_thread->join();
		delete m_thread;
	}

	// winsock2の終了処理
	WSACleanup();
	m_state = eNetState_None;
}
CSocket* CNetWork::GetClient(unsigned int idx) {
	if (m_sockets.size() <= idx) return nullptr;
	return m_sockets[idx];
}