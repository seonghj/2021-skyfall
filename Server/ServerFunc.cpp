#pragma once
#pragma warning(disable : 4996)
#include "ServerFunc.h"
#include "CPacket.h"

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char* buf, int len, int flags)
{
    int received;
    char* ptr = buf;
    int left = len;

    while (left > 0) {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        else if (received == 0)
            break;
        left -= received;
        ptr += received;
    }

    return (len - left);
}

void err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// ���� �Լ� ���� ���
void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

IOCPServer::IOCPServer()
{

}

IOCPServer::~IOCPServer()
{

}

int IOCPServer::get_new_id()
{
    // ������ Ŭ���̾�Ʈ�� id �ο�
    while (true)
        for (int i = 0; i < MAX_CLIENT; ++i)
            if (clients[i].connected == false) {
                clients[i].connected = true;
                return i;
            }
}

void IOCPServer::do_accept()
{
    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    // socket()
    SOCKET listen_sock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    int retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, MAX_CLIENT);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    // ������ ��ſ� ����� ����
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen = sizeof(SOCKADDR_IN);
    DWORD recvbytes, flags;

    while (1) {
        client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        int new_id = get_new_id();
        memset(&clients[new_id], 0x00, sizeof(struct SOCKETINFO));
        getpeername(client_sock, (SOCKADDR*)&clients[new_id].clientaddr
            , &clients[new_id].addrlen);

        printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d, key=%d\n",
            inet_ntoa(clients[new_id].clientaddr.sin_addr)
            , ntohs(clients[new_id].clientaddr.sin_port), new_id);

        clients[new_id].sock = client_sock;
        clients[new_id].over.dataBuffer.len = BUFSIZE;
        clients[new_id].over.dataBuffer.buf =
            clients[client_sock].over.messageBuffer;
        clients[new_id].over.is_recv = true;
        flags = 0;

        // ���ϰ� ����� �Ϸ� ��Ʈ ����
        CreateIoCompletionPort((HANDLE)client_sock, hcp, new_id, 0);
        clients[new_id].connected = true;

        do_recv(new_id);
    }

    // closesocket()
    closesocket(listen_sock);

    // ���� ����
    WSACleanup();
}

void IOCPServer::Disconnect(int id)
{
    closesocket(clients[id].sock);
    clients[id].connected = false;
    printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
        inet_ntoa(clients[id].clientaddr.sin_addr)
        , ntohs(clients[id].clientaddr.sin_port));
}

void IOCPServer::process_packet(char id, char* buf)
{
    // InputPacket* Packet = reinterpret_cast<InputPacket*>(buf);
    // �Է¹��� ��Ŷ ó��
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (clients[i].connected)
            do_send(i, buf);
    }
}

void IOCPServer::WorkerFunc()
{
    int retval = 0;

    while (1) {
        DWORD cbTransferred;
        SOCKET client_sock;
        ULONG id;
        SOCKETINFO* ptr;

        OVER_EX* lpover_ex;

        retval = GetQueuedCompletionStatus(hcp, &cbTransferred,
            (PULONG_PTR)&id, (LPOVERLAPPED*)&lpover_ex, INFINITE);

        std::thread::id Thread_id = std::this_thread::get_id();

        printf("thread id: %d\n", Thread_id);

        // �񵿱� ����� ��� Ȯ��
        if (FALSE == retval)
            err_display("WSAGetOverlappedResult()");
        if (0 == cbTransferred)
            Disconnect(id);

        if (lpover_ex->is_recv) {
            do_recv(id);
            int rest_size = cbTransferred;
            char* buf_ptr = lpover_ex->messageBuffer;
            char packet_size = 0;
            if (0 < clients[id].prev_size)
                packet_size = sizeof(clients[id].packet_buf);
            /*while (rest_size > 0) {
                if (0 == packet_size) packet_size = sizeof(buf_ptr);
                int required = packet_size - clients[id].prev_size;
                printf("required: %d\n", required);
                if (rest_size >= required) {
                    memcpy(clients[id].packet_buf + clients[id].
                        prev_size, buf_ptr, required);
                    process_packet(id, clients[id].packet_buf);
                    rest_size -= required;
                    buf_ptr += required;
                    packet_size = 0;
                }
                else {
                    memcpy(clients[id].packet_buf + clients[id].prev_size,
                        buf_ptr, rest_size);
                    rest_size = 0;
                }
            }*/
            process_packet(id, clients[id].packet_buf);
        }
        else {
            delete lpover_ex;
        }
    }
}

bool IOCPServer::Init()
{
    //std::vector <std::thread> working_threads;

    for (int i = 0; i < MAX_CLIENT; ++i)
        clients[i].connected = false;

    // ����� �Ϸ� ��Ʈ ����
    hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (hcp == NULL) return 0;

    // CPU ���� Ȯ��
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    // (CPU ���� * 2)���� �۾��� ������ ����
    for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; i++)
        working_threads.emplace_back(std::thread(&IOCPServer::WorkerFunc, this));

    accept_thread = std::thread(&IOCPServer::do_accept, this);

    return 1;
}

void IOCPServer::Run()
{
    accept_thread.join();
    for (auto& t : working_threads)
        t.join();

    CloseHandle(hcp);
}

void IOCPServer::do_recv(char id)
{
    DWORD flags = 0;

    SOCKET client_s = clients[id].sock;
    OVER_EX* over = &clients[id].over;

    over->dataBuffer.len = BUFSIZE;
    over->dataBuffer.buf = over->messageBuffer;
    ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));

    //InputPacket* Packet = reinterpret_cast<InputPacket*>(over->dataBuffer.buf);

    int retval = WSARecv(client_s, &over->dataBuffer, 1, (LPDWORD)clients[id].prev_size,
        &flags, &(over->overlapped), NULL);

    printf("Recv %d: %s\n", id, over->dataBuffer.buf);
    if (retval == SOCKET_ERROR)
    {
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING)
        {
            err_display("WSARecv()");
        }
    }
    memcpy(clients[id].packet_buf, over->dataBuffer.buf, over->dataBuffer.len);
    //memcpy(clients[id].packet_buf, reinterpret_cast<char*>(Packet), sizeof(InputPacket));
    //clients[id].prev_size = retval;
}

void IOCPServer::do_send(int to, char* packet)
{
    SOCKET client_s = clients[to].sock;

    int retval = 0;

    OVER_EX* over = reinterpret_cast<OVER_EX*>(malloc(sizeof(OVER_EX)));

    over->dataBuffer.len = strlen(packet);
    over->dataBuffer.buf = packet;

    memcpy(over->messageBuffer, packet, sizeof(packet));

    ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
    over->is_recv = false;

    //InputPacket* Packet = reinterpret_cast<InputPacket*>(over->dataBuffer.buf);
    //printf("Send -> type: %d, x: %d, y: %d\n", Packet->type, Packet->x, Packet->y);

    retval = WSASend(client_s, &over->dataBuffer, 1, NULL,
        0, &(over->overlapped), NULL);
    printf("Send %d: %s/%d\n", to, over->dataBuffer.buf, over->dataBuffer.len);

    if (retval == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            std::cout << "Error - Fail WSASend(error_code : ";
            std::cout << WSAGetLastError() << ")\n";
        }
    }
}