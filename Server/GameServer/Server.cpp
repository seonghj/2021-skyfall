#pragma once
#pragma warning(disable : 4996)
#include "Server.h"

//#define Run_DB
//#define Run_Lobby

void SESSION::init() 
{
   //memset(this, 0x00, sizeof(SESSION));
    addrlen = 0;
    memset(packet_buf, 0, sizeof(packet_buf));
    prev_size = 0;
    packet_start = nullptr;
    recv_start = nullptr;

    connected = false;
    isready = false;
    playing = false;
    key = -1;

    state = Death;
    f3Position = DirectX::XMFLOAT3(0.0f, 124.0f, 0.0f);
    m_fPitch = 60;
    m_fYaw = -90;

    weapon1 = PlayerType::PT_BASIC;
    weapon2 = PlayerType::PT_BASIC;
    helmet = 0;
    shoes = 0;
    armor = 0;

    hp = 100;
    def = 0;
    lv = 0;
    att = 10;
    speed = 20;

    for (int i = 0; i < INVENTORY_MAX; i++)
        inventory[i] = 0;
}

Server::Server()
{

}

Server::~Server()
{

}

void Server::display_error(const char* msg, int err_no)
{
    WCHAR* lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    std::cout << msg;
    std::wcout << L"���� " << lpMsgBuf << std::endl;
    LocalFree(lpMsgBuf);
}

int Server::SetClientKey(int roomID)
{
    int cnt = 0;
    while (true){
        if (cnt == 20)
            return -1;
        if (FALSE == sessions[roomID][cnt].connected){
            return cnt;
        }
        else 
            ++cnt;
    }
}

int Server::SetLobbyKey()
{
    int cnt = 100;
    while (true) {
        if (FALSE == Lobby_sessions[cnt].connected) {
            return cnt;
        }
        else
            ++cnt;
    }
}

int Server::SetroomID()
{
    int cnt = 1;
    while (1) {
        if (sessions.find(cnt) == sessions.end()) {
            sessions_lock.lock();
            //std::array<SESSION, 20> s{};
            sessions.emplace(cnt, std::array<SESSION, 20>{});
            sessions_lock.unlock();

            maps_lock.lock();
            maps.emplace(cnt, Map(cnt));
            maps[cnt].SetNum(cnt);
            //maps[cnt].init_Map(this, m_pTimer);
            maps_lock.unlock();

            m_pBot->monsters.emplace(cnt, std::array<Monster, 15>{});
            //m_pBot->monsterRun = TRUE;
            m_pBot->Init(cnt);

            /*m_pBot->monsters[cnt][0].SetPosition(2000, 197.757935, 5000);
            m_pBot->monsters[cnt][0].state = 1;
            m_pBot->monsters[cnt][0].type = MonsterType::Dragon;
            m_pBot->monsters[cnt][0].Rotate(-90.0f, 20.0f, 0.0f);
            m_pBot->monsters[cnt][0].key = 0;*/

            m_pBot->RunBot(cnt);

            printf("create game room - %d\n", cnt);
            return cnt;
        }
        if (FALSE == sessions[cnt][MAX_PLAYER - 1].connected)
            return cnt;
        else {
           ++cnt;
        }
    }
}

bool Server::CreateRoom(int key)
{
    if (sessions.find(key) == sessions.end()) {
        sessions_lock.lock();
        //std::array<SESSION, 20> s{};
        sessions.emplace(key, std::array<SESSION, 20>{});
        sessions_lock.unlock();

        maps_lock.lock();
        maps.emplace(key, Map(key));
        maps[key].SetNum(key);
        //maps[key].init_Map(this, m_pTimer);
        maps_lock.unlock();

        m_pBot->monsters.emplace(key, std::array<Monster, 15>{});
        //m_pBot->monsterRun = TRUE;
        m_pBot->Init(key);
        //m_pBot->RunBot(key);

        printf("create game room - %d\n", key);
        return true;
    }
    else
        return false;
}

void Server::ConnectLobby()
{
    std::array<SESSION, 20> s{};
    sessions.emplace(LOBBY_ID, s);
    sessions[LOBBY_ID][0].key = LOBBY_ID;
    sessions[LOBBY_ID][0].connected = TRUE;
    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return;

    // socket()
    sessions[LOBBY_ID][0].sock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    // connect()
    SOCKADDR_IN lobbyaddr;
    ZeroMemory(&lobbyaddr, sizeof(lobbyaddr));
    lobbyaddr.sin_family = AF_INET;
    lobbyaddr.sin_addr.s_addr = inet_addr(SERVERIP);
    lobbyaddr.sin_port = htons(LOBBYPORT);
    connect(sessions[LOBBY_ID][0].sock, (SOCKADDR*)&lobbyaddr, sizeof(lobbyaddr));

    memset(&sessions[LOBBY_ID][0].over.overlapped, 0, sizeof(sessions[LOBBY_ID][0].over.overlapped));
    sessions[LOBBY_ID][0].over.is_recv = true;
    sessions[LOBBY_ID][0].over.dataBuffer.len = BUFSIZE;
    sessions[LOBBY_ID][0].over.dataBuffer.buf =
        sessions[LOBBY_ID][0].over.messageBuffer;

    std::cout << "Lobby connected\n";

    CreateIoCompletionPort((HANDLE)sessions[LOBBY_ID][0].sock, hcp, LOBBY_ID, 0);

    do_recv(LOBBY_ID, 0);
}

void Server::Accept()
{
    // socket()
    SOCKET listen_sock = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(GAMESERVERPORT);
    int retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
  
    // listen()
    retval = listen(listen_sock, MAX_CLIENT);

    // ������ ��ſ� ����� ����
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen = sizeof(SOCKADDR_IN);
    DWORD flags;

    printf("ready\n");

    while (1) {
        //accept_lock.lock();
        client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            printf("accept error: %d", WSAGetLastError());
            break;
        }

        int client_key = SetLobbyKey();
        if (client_key == -1) {
            closesocket(client_sock);
            break;
        }

        std::lock_guard<std::mutex> lock_guard{ Lobby_sessions_lock };
        Lobby_sessions[client_key].init();
        Lobby_sessions[client_key].connected = TRUE;
        Lobby_sessions[client_key].key = client_key;
        Lobby_sessions[client_key].roomID = INVALUED_ID;
        Lobby_sessions[client_key].sock = client_sock;
        Lobby_sessions[client_key].clientaddr = clientaddr;
        getpeername(client_sock, (SOCKADDR*)&Lobby_sessions[client_key].clientaddr
            , &Lobby_sessions[client_key].addrlen);

        Lobby_sessions[client_key].over.type = 0;
        Lobby_sessions[client_key].over.dataBuffer.len = BUFSIZE;
        Lobby_sessions[client_key].over.dataBuffer.buf =
            Lobby_sessions[client_key].over.messageBuffer;
        Lobby_sessions[client_key].over.is_recv = true;
        Lobby_sessions[client_key].over.key = client_key;
        Lobby_sessions[client_key].over.roomID = INVALUED_ID;


        /*for (int i = 0; i < 20; i++)
            Lobby_sessions[client_key].near_monster.insert(i);*/

        //accept_lock.unlock();
        // ���ϰ� ����� �Ϸ� ��Ʈ ����

        CreateIoCompletionPort((HANDLE)client_sock, hcp, client_key, 0);

        //printf("client_key: %d\n", client_key);
        send_Lobby_key_packet(client_key);

        do_recv(client_key, -1);
    }

    // closesocket()
    closesocket(listen_sock); 

    // ���� ����
    WSACleanup();
}

void Server::Disconnected(int key, int roomID)
{
    std::lock_guard<std::mutex> lock_guard(sessions_lock);
    if (sessions[roomID][key].key == key) {
        printf("client_end: IP =%s, port=%d key = %d, Room = %d\n",
            inet_ntoa(sessions[roomID][key].clientaddr.sin_addr)
            , ntohs(sessions[roomID][key].clientaddr.sin_port), key, roomID);
        //send_disconnect_player_packet(key);
        closesocket(sessions[roomID][key].sock);
        sessions[roomID][key].connected = FALSE;
        sessions[roomID][key].key = -1;
        //printf("disconnected %d\n", gameroom[roomID].ID[i]);
        
    }

#ifdef Run_DB
    m_pDB->Logout_player(sessions[roomID][key].id);
#endif
    //sessions[roomID].clear();
}

void Server::Disconnected(int key)
{
    std::lock_guard<std::mutex> lock_guard(Lobby_sessions_lock);
    printf("client_end: IP =%s, port=%d Lobby key = %d\n",
        inet_ntoa(Lobby_sessions[key].clientaddr.sin_addr)
        , ntohs(Lobby_sessions[key].clientaddr.sin_port), key);
    //send_disconnect_player_packet(key);
    closesocket(Lobby_sessions[key].sock);
    Lobby_sessions[key].connected = FALSE;
    Lobby_sessions[key].key = -1;
    //printf("disconnected %d\n", gameroom[roomID].ID[i]);

#ifdef Run_DB
    m_pDB->Logout_player(Lobby_sessions[key].id);
#endif
    //sessions[roomID].clear();
}

void Server::do_recv(int key, int roomID)
{
    DWORD flags = 0;
    SOCKET client_s;
    OVER_EX* over;
    if (roomID == -1) {
        client_s = Lobby_sessions[key].sock;
        over = &Lobby_sessions[key].over;

        over->dataBuffer.len = BUFSIZE;
        over->dataBuffer.buf = over->messageBuffer;
        ZeroMemory(&over->overlapped, sizeof(over->overlapped));

        if (WSARecv(client_s, &over->dataBuffer, 1, (LPDWORD)Lobby_sessions[key].prev_size,
            &flags, &(over->overlapped), NULL)) {
            int err_no = WSAGetLastError();
            if (err_no != WSA_IO_PENDING) {
                printf("key: %d recv error: %d\n", key, err_no);
                Disconnected(key, Lobby_sessions[key].over.roomID);
            }
        }
    }
    else {
        client_s = sessions[roomID][key].sock;
        over = &sessions[roomID][key].over;

        over->dataBuffer.len = BUFSIZE;
        over->dataBuffer.buf = over->messageBuffer;
        ZeroMemory(&over->overlapped, sizeof(over->overlapped));

        if (WSARecv(client_s, &over->dataBuffer, 1, (LPDWORD)sessions[roomID][key].prev_size,
            &flags, &(over->overlapped), NULL)) {
            int err_no = WSAGetLastError();
            if (err_no != WSA_IO_PENDING) {
                printf("key: %d recv error: %d\n", key, err_no);
                Disconnected(key, sessions[roomID][key].over.roomID);
            }
        }
    }
}

void Server::send_packet(int to, char* packet, int roomID)
{
    if (SC_NONE >= packet[1] || packet[1] >= CS_NONE) return;
    SOCKET client_s;
    if (roomID == -1)
        client_s = Lobby_sessions[to].sock;
    else
        client_s = sessions[roomID][to].sock;

    OVER_EX* over = new OVER_EX;
    ZeroMemory(over, sizeof(OVER_EX));
    over->dataBuffer.buf = packet;
    over->dataBuffer.len = packet[0];

    memcpy(over->messageBuffer, packet, packet[0]);
    over->is_recv = false;
    ZeroMemory(&over->overlapped, sizeof(over->overlapped));

    if (WSASend(client_s, &over->dataBuffer, 1, NULL,
        0, &(over->overlapped), NULL)) {
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING){
            //printf("to: %d packet: %d send error: %d\n", to, packet[1], err_no);
            //Disconnected(to, sessions[roomID][to].over.roomID);
        }
    }
    //printf("to: %d packet: %d send\n", to, packet[1]);
}

void Server::send_player_key_packet(int key, int roomID)
{
    player_key_packet p;

    p.key = key;
    p.size = sizeof(player_key_packet);
    p.type = PacketType::SC_player_key;
    p.roomid = roomID;
    send_packet(key, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_Lobby_key_packet(int key)
{
    player_key_packet p;

    p.key = key;
    p.size = sizeof(player_key_packet);
    p.type = PacketType::SC_player_Lobbykey;
    p.roomid = -1;
    send_packet(key, reinterpret_cast<char*>(&p),-1);
}

void Server::send_Lobby_loginOK_packet(int key)
{
    player_loginOK_packet p;

    p.key = key;
    p.size = sizeof(player_loginOK_packet);
    p.type = PacketType::SC_player_loginOK;
    p.roomid = INVALIDID;
    p.Position = Lobby_sessions[key].f3Position.load();
    p.dx = Lobby_sessions[key].m_fPitch.load();
    p.dy = Lobby_sessions[key].m_fYaw.load();
    send_packet(key, reinterpret_cast<char*>(&p), -1);
}

void Server::send_player_loginOK_packet(int key, int roomID)
{
    player_loginOK_packet p;

    p.key = key;
    p.size = sizeof(player_loginOK_packet);
    p.type = PacketType::SC_player_loginOK;
    p.roomid = roomID;
    p.Position = sessions[roomID][key].f3Position.load();
    p.dx = sessions[roomID][key].m_fPitch.load();
    p.dy = sessions[roomID][key].m_fYaw.load();
    send_packet(key, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_player_loginFail_packet(int key, int roomID)
{
    player_loginFail_packet p;
    p.key = key;
    p.size = sizeof(player_loginFail_packet);
    p.type = PacketType::SC_player_loginFail;
    p.roomid = roomID;
    send_packet(key, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_add_player_packet(int key, int to, int roomID)
{
    player_add_packet p;

    p.key = key;
    p.size = sizeof(player_add_packet);
    p.type = PacketType::SC_player_add;
    p.roomid = roomID;
    p.Position = sessions[roomID][key].f3Position.load();
    p.dx = sessions[roomID][key].m_fPitch.load();
    p.dy = sessions[roomID][key].m_fYaw.load();
    p.WeaponType = sessions[roomID][key].using_weapon;

    //printf("%d send login to %d\n",key, to);

    send_packet(to, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_remove_player_packet(int key, int roomID)
{
    player_remove_packet p;
    p.key = key;
    p.size = sizeof(player_remove_packet);
    p.type = PacketType::SC_player_remove;
    p.roomid = roomID;

    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (sessions[roomID][i].connected == TRUE)
            send_packet(i, reinterpret_cast<char*>(&p), roomID);
    }
    Disconnected(key, roomID);
}

void Server::send_disconnect_player_packet(int key, int roomID)
{
    player_disconnect_packet p;
    p.key = key;
    p.size = sizeof(player_disconnect_packet);
    p.type = PacketType::SC_player_disconnect;
    p.roomid = roomID;

    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (sessions[roomID][i].connected == TRUE)
            send_packet(i, reinterpret_cast<char*>(&p), roomID);
    }
    Disconnected(key, roomID);
}

void Server::send_packet_to_players(int key, char* buf, int roomID)
{
   //sessions_lock.lock();
    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (sessions[roomID][i].connected == FALSE) continue;
        if (sessions[roomID][i].playing == FALSE) continue;
        if (in_VisualField(sessions[roomID][key], sessions[roomID][i], roomID)) {
            send_packet(i, buf, roomID);
        }
    }
    //sessions_lock.unlock();
}

void Server::send_packet_to_allplayers(int roomID, char* buf)
{
   //sessions_lock.lock();
    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (sessions[roomID][i].connected == FALSE) continue;
        if (sessions[roomID][i].playing == FALSE) continue;
        send_packet(i, buf, roomID);
    }
   //sessions_lock.unlock();
}

void Server::send_map_collapse_packet(int num, int map_num)
{
    std::vector<std::vector<int>> m_vMapArrange = { { 0, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}, {0, 2}, {1, 2}, {2, 2} };

    map_collapse_packet p;
    p.size = sizeof(map_collapse_packet);
    p.type = PacketType::SC_map_collapse;
    p.block_num = num;
    p.key = 0;
    p.roomid = map_num;
    p.index[0] = m_vMapArrange[num][0];
    p.index[1] = m_vMapArrange[num][1];

    send_packet_to_allplayers(map_num, reinterpret_cast<char*>(&p));
}

void Server::send_cloud_move_packet(float x, float z, int map_num)
{
    cloud_move_packet p;
    p.size = sizeof(cloud_move_packet);
    p.type = PacketType::SC_cloud_move;
    p.x = x;
    p.z = z;
    p.key = 0;
    p.roomid = map_num;

    send_packet_to_allplayers(map_num, reinterpret_cast<char*>(&p));
}

void Server::send_add_monster(int key, int roomID, int to)
{
    mon_add_packet p;
    p.size = sizeof(mon_add_packet);
    p.type = PacketType::SC_monster_add;
    p.key = key;
    p.roomid = roomID;
    p.Position = m_pBot->monsters[roomID][key].f3Position;
    p.dx = m_pBot->monsters[roomID][key].m_fPitch;
    p.dy = m_pBot->monsters[roomID][key].m_fYaw;
    p.dz = m_pBot->monsters[roomID][key].m_fRoll;
    p.MonsterType = m_pBot->monsters[roomID][key].type;

    send_packet(to, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_remove_monster(int key, int roomID, int to)
{
    mon_remove_packet p;
    p.size = sizeof(mon_remove_packet);
    p.type = PacketType::SC_monster_remove;
    p.key = key;
    p.roomid = roomID;

    send_packet(to, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_monster_pos(const Monster& mon, XMFLOAT3 direction)
{
    int roomID = mon.roomID;

    mon_pos_packet p;
    p.size = sizeof(mon_pos_packet);
    p.type = PacketType::SC_monster_pos;
    p.key = mon.key;
    p.roomid = mon.roomID.load();
    p.Position = mon.f3Position.load();
    p.direction = direction;
    p.degree = mon.m_fRoll.load();
    p.MoveType = 0;
    p.state = 0;
    p.MonsterType = mon.type.load();

    //printf("%d\n", mon.key);

    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (sessions[roomID][i].connected == FALSE) continue;
        if (sessions[roomID][i].playing == FALSE) continue;
        if (true == in_VisualField(mon, sessions[roomID][i], roomID)) {
            send_packet(i, reinterpret_cast<char*>(&p), roomID);
            //printf("%d\n", mon.key);
            //printf("send to %d \n", sessions[roomID][i].key.load());
        }
    }
}

void Server::send_monster_attack(const Monster& mon, XMFLOAT3 direction, int target)
{
    int roomID = mon.roomID.load();

    mon_attack_packet p;
    p.size = sizeof(p);
    p.type = PacketType::SC_monster_attack;
    p.key = mon.key;
    p.roomid = mon.roomID.load();
    p.direction = direction;
    p.degree = mon.m_fRoll.load();
    p.target = target;
    p.PlayerLeftHp = sessions[roomID][target].hp.load();

    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (sessions[roomID][i].connected == FALSE) continue;
        if (true == in_VisualField(mon, sessions[roomID][i], roomID)) {
            send_packet(i, reinterpret_cast<char*>(&p), roomID);
            //printf("send to %d \n", sessions[roomID][i].key.load());
        }
    }
}

void Server::send_player_record(int key, int roomID
    , const SESSION& s, int time, int rank)
{
    player_record_packet p;
    p.size = sizeof(player_record_packet);
    p.type = PacketType::SC_player_record;
    p.key = key;
    p.roomid = roomID;
    strcpy(p.id, s.id);
    p.survivalTime = time;
    p.rank = rank;
    p.weapon1 = s.weapon1;
    p.weapon2 = s.weapon2;
    p.helmet = s.helmet;
    p.shoes = s.shoes;
    p.armor = s.armor;

    send_packet(key, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_map_packet(int to, int roomID)
{
    map_block_set p;
    p.type = PacketType::SC_map_set;
    p.size = sizeof(p);
    p.key = roomID;
    p.roomid = roomID;

    for (int i = 0; i < MAX_MAP_BLOCK; i++){
        p.block_type[i] = maps[roomID].Map_type[i];
    }

    send_packet(to, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_start_packet(int to, int roomID)
{
    game_start_packet p;
    p.type = PacketType::SC_start_ok;
    p.size = sizeof(p);
    p.key = to;
    p.roomid = roomID;
    p.weaponType = sessions[roomID][to].using_weapon;

    send_packet(to, reinterpret_cast<char*>(&p), roomID);

    if (maps[roomID].game_start == false) {
        Mapbreak_event e;
        e.roomid = roomID;
        e.size = sizeof(e);
        e.type = EventType::MapBreak;
        m_pTimer->push_event(roomID, OE_gEvent, MAP_BREAK_TIME, reinterpret_cast<char*>(&e));
        maps[roomID].game_start = true;
    }
}

void Server::send_game_end_packet(int key, int roomID)
{
    game_end_packet p;
    p.key = 0;
    p.size = sizeof(p);
    p.type = PacketType::SC_game_end;
    p.roomid = roomID;

    send_packet(key, reinterpret_cast<char*>(&p), roomID);
}

void Server::game_end(int roomnum)
{

    int nkey;
    Lobby_sessions_lock.lock();
    for (auto& s : sessions[roomnum]) {
        if (s.connected == false || s.playing == false) continue;

        send_game_end_packet(s.key.load(), s.roomID.load());
        nkey = SetLobbyKey();

        s.over.key = nkey;
        s.over.roomID = INVALUED_ID;
        Lobby_sessions[nkey].init();
        Lobby_sessions[nkey].connected = TRUE;
        Lobby_sessions[nkey].key = nkey;
        Lobby_sessions[nkey].roomID = INVALUED_ID;
        Lobby_sessions[nkey].sock = s.sock;
        Lobby_sessions[nkey].clientaddr = s.clientaddr;

        Lobby_sessions[nkey].over = s.over;
        Lobby_sessions[nkey].over.type = 0;
        Lobby_sessions[nkey].over.dataBuffer.len = BUFSIZE;
        Lobby_sessions[nkey].over.dataBuffer.buf =
            Lobby_sessions[nkey].over.messageBuffer;
        Lobby_sessions[nkey].over.is_recv = true;
        Lobby_sessions[nkey].over.key = nkey;
        Lobby_sessions[nkey].over.roomID = INVALUED_ID;
        strcpy_s(Lobby_sessions[nkey].id, s.id);

        send_Lobby_loginOK_packet(nkey);
        //send_Lobby_key_packet(nkey);
    }
    Lobby_sessions_lock.unlock();

    maps_lock.lock();
    maps.erase(roomnum);
    maps_lock.unlock();
    sessions_lock.lock();
    sessions.erase(roomnum);
    sessions_lock.unlock();
}

bool Server::in_VisualField(SESSION a, SESSION b, int roomID)
{
    float value = pow(((short)a.f3Position.load().x - (short)b.f3Position.load().x), 2)
        + pow(((short)a.f3Position.load().z - (short)b.f3Position.load().z), 2);

    if (value < 0) {
        if (sqrt(-value) <= VIEWING_DISTANCE) return true;
    }
    else {
        if (sqrt(value) <= VIEWING_DISTANCE) return true;
    }
    return false;
}

bool Server::in_VisualField(Monster a, SESSION b, int roomID)
{
    float value = pow(((short)a.f3Position.load().x - (short)b.f3Position.load().x), 2)
        + pow(((short)a.f3Position.load().z - (short)b.f3Position.load().z), 2);

    if (value < 0) {
        if (sqrt(-value) <= VIEWING_DISTANCE) return true;
    }
    else {
        if (sqrt(value) <= VIEWING_DISTANCE) return true;
    }
    return false;
}

unsigned short Server::calc_attack(int key, char attacktype)
{

    return 0;
}

void Server::player_move(int key, int roomID, DirectX::XMFLOAT3 pos, float dx, float dy)
{
    int client_key = key;

    sessions[roomID][client_key].m_fPitch.store(fmodf(sessions[roomID][client_key].m_fPitch.load() + dx, 360.f));
    sessions[roomID][client_key].m_fYaw.store(fmodf(sessions[roomID][client_key].m_fYaw.load() + dy, 360.f));
    
    if (sessions[roomID][client_key].f3Position.load().x == pos.x
        && sessions[roomID][client_key].f3Position.load().z == pos.z) {
        sessions[roomID][client_key].f3Position = pos;
        return;
    }
    else
        sessions[roomID][client_key].f3Position = pos;

    /*std::lock_guard <std::mutex> lg(sessions[roomID][client_key].nm_lock);
    std::unordered_set<int> old_nm;
    std::unordered_set<int> new_nm;

    old_nm = sessions[roomID][client_key].near_monster;

    for (auto& m : m_pBot->monsters[roomID]) {
        if (m.state == 0) continue;
        if (in_VisualField(m, sessions[roomID][client_key], roomID)) {
            new_nm.insert(m.key.load());
        }
    }

    for (auto m : new_nm) {
        if (m_pBot->monsters[roomID][m].state == 0) continue;
        if (old_nm.find(m) == old_nm.end()) {
            sessions[roomID][client_key].near_monster.insert(m);
            send_add_monster(m, roomID, key);
        }
    }

    for (auto m : old_nm) {
        if (m_pBot->monsters[roomID][m].state == 0) continue;
        if (new_nm.find(m) == new_nm.end()) {
            sessions[roomID][client_key].near_monster.erase(m);
            send_remove_monster(m, roomID, client_key);
        }
    }*/
}

void Server::process_packet(int key, char* buf, int roomID)
{
    // Ŭ���̾�Ʈ���� ���� ��Ŷ ó��
    switch (buf[1]) {
    case PacketType::CS_player_Lobbylogin: {
        player_login_packet* p = reinterpret_cast<player_login_packet*>(buf);
        int client_key = p->key;
        bool is_Login = false;

        bool b;
#ifdef Run_DB
        if (strcmp(p->id, "test") != 0) {
            b = m_pDB->Search_ID(p->id, p->pw, &is_Login);

            if (!b && !is_Login) b = m_pDB->Insert_ID(p->id);

            if (is_Login) {
                send_player_loginFail_packet(client_key);
                Disconnected(client_key);
                break;
            }
        }
#endif

        strcpy_s(Lobby_sessions[client_key].id, p->id);

        send_Lobby_loginOK_packet(client_key);

        printf("client_connected to Lobby: IP =%s, port=%d key = %d\n",
            inet_ntoa(Lobby_sessions[client_key].clientaddr.sin_addr)
            , ntohs(Lobby_sessions[client_key].clientaddr.sin_port), client_key);

        break;
    }
    case PacketType::CS_game_start: {
        game_start_packet* p = reinterpret_cast<game_start_packet*>(buf);

        int client_key = p->key;
        bool is_Login = false;

        int nkey = SetClientKey(p->roomid);

        if (nkey == -1) break;
        Lobby_sessions[client_key].over.roomID = p->roomid;
        Lobby_sessions[client_key].over.key = nkey;
        sessions[p->roomid][nkey].init();
        sessions[p->roomid][nkey].connected = TRUE;
        sessions[p->roomid][nkey].key = nkey;
        sessions[p->roomid][nkey].roomID = p->roomid;
        sessions[p->roomid][nkey].sock = Lobby_sessions[client_key].sock;
        sessions[p->roomid][nkey].clientaddr = Lobby_sessions[client_key].clientaddr;

        sessions[p->roomid][nkey].over = Lobby_sessions[client_key].over;
        sessions[p->roomid][nkey].over.type = 0;
        sessions[p->roomid][nkey].over.dataBuffer.len = BUFSIZE;
        sessions[p->roomid][nkey].over.dataBuffer.buf =
            sessions[p->roomid][nkey].over.messageBuffer;
        sessions[p->roomid][nkey].over.is_recv = true;
        sessions[p->roomid][nkey].over.key = nkey;
        sessions[p->roomid][nkey].over.roomID = p->roomid;

        sessions[p->roomid][nkey].using_weapon = p->weaponType;
        sessions[p->roomid][nkey].weapon1 = p->weaponType;
        strcpy_s(sessions[p->roomid][nkey].id, Lobby_sessions[client_key].id);
        send_player_key_packet(nkey, p->roomid);

        printf("client_connected to Game: IP =%s, port=%d key = %d Room = %d\n",
            inet_ntoa(sessions[p->roomid][nkey].clientaddr.sin_addr)
            , ntohs(sessions[p->roomid][nkey].clientaddr.sin_port), nkey, p->roomid);

        sessions[p->roomid][nkey].state = Alive;

        send_start_packet(nkey, p->roomid);

        //send_map_packet(client_key, roomID);

        sessions[p->roomid][nkey].isready = true;
        sessions[p->roomid][nkey].playing = true;

        if (m_pBot->monsterRun == false) {
            m_pBot->monsterRun = true;
            m_pBot->RunBot(p->roomid);
        }
        if (maps[p->roomid].game_start == false) {
            maps[p->roomid].init_Map(this, m_pTimer);
        }

        for (auto& s : sessions[p->roomid]) {
            if ((TRUE == s.connected) && (s.key.load() != nkey)) {
                send_add_player_packet(s.key.load(), nkey, p->roomid);
            }
        }
        for (auto& s : sessions[p->roomid]) {
            if ((TRUE == s.connected) && (s.key.load() != nkey))
                send_add_player_packet(nkey, s.key.load(), p->roomid);
        }

        for (int i = 0; i < 15; ++i) {
            if (m_pBot->monsters[p->roomid][i].state == 1) {
                //printf("send monster %d\n", i);
                send_add_monster(i, p->roomid, nkey);
            }
        }

        Lobby_sessions_lock.lock();
        Lobby_sessions.erase(client_key);
        Lobby_sessions_lock.unlock();

        break;
    }
    case PacketType::CS_player_info:{
        player_info_packet* p = reinterpret_cast<player_info_packet*>(buf);
        p->type = SC_player_info;
        break;
    }
    case PacketType::CS_player_pos: {
        player_pos_packet* p = reinterpret_cast<player_pos_packet*>(buf);
        if (0 > p->key || p->key >= 20) break;
        player_move(p->key, roomID, p->Position, p->dx, p->dy);
        player_pos_packet packet;
        packet.type = SC_player_pos;
        packet.key = p->key;
        packet.roomid = roomID;
        packet.state = p->state;
        packet.size = sizeof(player_pos_packet);
        packet.Position = sessions[roomID][p->key].f3Position;
        packet.dx = sessions[roomID][p->key].m_fPitch;
        packet.dy = sessions[roomID][p->key].m_fYaw;
        packet.MoveType = p->MoveType;
        packet.dir = p->dir;

        /*if (sessions[roomID][p->key].playing == FALSE) {
            send_packet(p->key, reinterpret_cast<char*>(&packet), roomID);
            break;
        }*/

        send_packet_to_players(key, reinterpret_cast<char*>(&packet), roomID);
        break;
    }
    case PacketType::CS_start_pos: {
        player_start_pos* p = reinterpret_cast<player_start_pos*>(buf);
        sessions[roomID][p->key].f3Position = p->Position;
        send_packet(key, reinterpret_cast<char*>(p), roomID);
        break;
    }
    case PacketType::CS_weapon_swap :{
         Weapon_swap_packet* p = reinterpret_cast<Weapon_swap_packet*>(buf);
         p->type = SC_weapon_swap;
         sessions[roomID][p->key].using_weapon = p->weapon;
         for (int i = 0; i < MAX_PLAYER; ++i) {
             if (sessions[roomID][i].connected == FALSE) continue;
             send_packet(i, buf, roomID);
         }
         break;
    }
    case PacketType::CS_player_move: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);
        p->type = SC_player_move;
        switch (p->MoveType) {
        case PlayerMove::JUMP:{
            send_packet_to_players(key, reinterpret_cast<char*>(p), roomID);
                break;
            }
        default: {
            break;
        }
        }
        break;
    }
    case PacketType::CS_player_attack:{
        player_attack_packet* p = reinterpret_cast<player_attack_packet*>(buf);
        switch (p->attack_type) {
        case BOWL: {
            // �浹ó��
            break;
        }
        case SWORD1HL1: {
            // �浹ó��

            break;
        }
        }
        p->type = SC_player_attack;
        send_packet_to_players(p->key, reinterpret_cast<char*>(p), roomID);
        break;

    }
    case PacketType::CS_allow_shot: {
        player_shot_packet* p = reinterpret_cast<player_shot_packet*>(buf);
        p->type = SC_allow_shot;
        send_packet_to_players(p->key, reinterpret_cast<char*>(p), roomID);
        break;
    }
    case PacketType::CS_player_stop: {
        player_stop_packet* p = reinterpret_cast<player_stop_packet*>(buf);
        if (roomID == -1) break;
        p->type = SC_player_stop;
        p->Position = sessions[roomID][p->key].f3Position;
        send_packet_to_players(p->key, reinterpret_cast<char*>(p), roomID);
        break;
    }
    case PacketType::CS_player_getitem: {
        player_getitem_packet* p = reinterpret_cast<player_getitem_packet*>(buf);
        int key = p->key;
        int room = p->roomid;
        for (auto& i : sessions[room][key].inventory) {
            if (i == 0) {
                i = p->item;
                break;
            }
        }
        p->type = SC_player_getitem;
        send_packet_to_players(p->key, reinterpret_cast<char*>(p), roomID);
        break;
    }
    case PacketType::CS_monster_pos: {
        mon_pos_packet* p = reinterpret_cast<mon_pos_packet*>(buf);
        if (m_pBot->monsters[roomID][p->key].recv_pos == TRUE) break;
        if (_isnanf(p->Position.x) || _isnanf(p->Position.y) || _isnanf(p->Position.z)) {
            m_pBot->monsters[roomID][p->key].recv_pos = TRUE;
            break;
        }
        m_pBot->monsters[roomID][p->key].recv_pos = TRUE;
        m_pBot->monsters[roomID][p->key].SetPosition(p->Position.x, p->Position.y, p->Position.z);
        break;
    }
    case PacketType::CS_monster_attack: {
        mon_attack_packet* p = reinterpret_cast<mon_attack_packet*>(buf);

        //std::cout << "target: " << p->target << " key: " << p->key << std::endl;
        break;
    }
    case PacketType::CS_monster_damaged: {
        mon_damaged_packet* p = reinterpret_cast<mon_damaged_packet*>(buf);
        int key = p->key;
        int target = p->target;
        if (m_pBot->monsters[p->roomid][target].state == 0) break;

        p->type = SC_monster_damaged;
        p->damage = (sessions[p->roomid][key].att * (1.f + p->nAttack / 4.f))
            * (100 - m_pBot->monsters[p->roomid][target].def) / 100;
        sessions[p->roomid][key].AddProficiency();
        m_pBot->monsters[p->roomid][target].hp = m_pBot->monsters[p->roomid][target].hp - p->damage;
        p->leftHp = m_pBot->monsters[p->roomid][target].hp;
        if (m_pBot->monsters[p->roomid][target].hp <= 0) {
            m_pBot->monsters[p->roomid][target].state = 0;

            mon_respawn_event e;
            e.key = target;
            e.roomid = p->roomid;
            e.size = sizeof(e);
            e.type = EventType::Mon_respawn;
            m_pTimer->push_event(roomID, OE_gEvent, MON_SPAWN_TIME, reinterpret_cast<char*>(&e));
        }

        send_packet_to_players(key, reinterpret_cast<char*>(p), p->roomid);

        //printf("%f\n", m_pBot->monsters[p->roomid][target].hp.load());

        break;
    }
    case PacketType::CS_player_damage: {
        player_damage_packet* p = reinterpret_cast<player_damage_packet*>(buf);

        int key = p->key;
        int target = p->target;

        p->type = SC_player_damage;
        p->damage = (sessions[roomID][key].GetAtkDamage() * (1.f + p->nAttack / 4.f))
            * (100 - sessions[roomID][target].def) / 100;
        sessions[roomID][target].hp = sessions[roomID][target].hp - p->damage;
        p->leftHp = sessions[roomID][target].hp;
        sessions[roomID][key].AddProficiency();
        if (sessions[roomID][key].hp <= 0) {
            sessions[roomID][key].state = Death;
        }
        send_packet_to_players(key, reinterpret_cast<char*>(p), p->roomid);
        break;
    }
     // Lobby
    case PacketType::CS_room_select: {
        room_select_packet* p = reinterpret_cast<room_select_packet*>(buf);
        CreateRoom(p->room);

        p->type = SC_select_room;
        send_packet(p->key, reinterpret_cast<char*>(p), -1);
        break;
    }
    }
}

void Server::WorkerFunc()
{
    DWORD Transferred;
    SOCKET client_sock;
    ULONG key;
    WSAOVERLAPPED* over;

    while (1) {
        BOOL retval = GetQueuedCompletionStatus(hcp, &Transferred,
            (PULONG_PTR)&key, (LPOVERLAPPED*)&over, INFINITE);

        //std::thread::key Thread_key = std::this_thread::get_key();
        OVER_EX* over_ex = reinterpret_cast<OVER_EX*>(over);
        int roomID = over_ex->roomID;
        int nkey = over_ex->key;
        switch (over_ex->type) {
        case OE_session: {
            // key = 유저번호
            if (FALSE == retval)
            {
                //printf("error = %d\n", WSAGetLastError());
                display_error("GQCS", WSAGetLastError());
                if (roomID == INVALUED_ID)
                    Disconnected(nkey);
                else
                    Disconnected(nkey, roomID);
                continue;
            }

            if ((Transferred == 0)) {
                display_error("GQCS", WSAGetLastError());
                if (roomID == INVALUED_ID)
                    Disconnected(nkey);
                else
                    Disconnected(nkey, roomID);
                continue;
            }
            //printf("%d\n", true);
            if (over_ex->is_recv) {
                //printf("thread key: %d\n", Thread_key);
                int rest_size = Transferred;
                char* buf_ptr = over_ex->messageBuffer;
                char packet_size = 0;
                if (roomID != INVALUED_ID) {
                    if (0 < sessions[roomID][nkey].prev_size)
                        packet_size = sessions[roomID][nkey].packet_buf[0];
                    while (rest_size > 0) {
                        if (0 == packet_size) packet_size = buf_ptr[0];
                        int required = packet_size - sessions[roomID][nkey].prev_size;
                        if (rest_size >= required) {
                            memcpy(sessions[roomID][nkey].packet_buf + sessions[roomID][nkey].
                                prev_size, buf_ptr, required);
                            process_packet(nkey, sessions[roomID][nkey].packet_buf, roomID);
                            rest_size -= required;
                            buf_ptr += required;
                            packet_size = 0;
                        }
                        else {
                            memcpy(sessions[roomID][nkey].packet_buf + sessions[roomID][nkey].prev_size,
                                buf_ptr, rest_size);
                            rest_size = 0;
                        }
                    }
                    do_recv(nkey, roomID);
                }
                else {
                    if (0 < Lobby_sessions[key].prev_size)
                        packet_size = Lobby_sessions[key].packet_buf[0];
                    while (rest_size > 0) {
                        if (0 == packet_size) packet_size = buf_ptr[0];
                        int required = packet_size - Lobby_sessions[key].prev_size;
                        if (rest_size >= required) {
                            memcpy(Lobby_sessions[key].packet_buf + Lobby_sessions[key].
                                prev_size, buf_ptr, required);
                            process_packet(key, Lobby_sessions[key].packet_buf, roomID);
                            rest_size -= required;
                            buf_ptr += required;
                            packet_size = 0;
                        }
                        else {
                            memcpy(Lobby_sessions[key].packet_buf + Lobby_sessions[key].prev_size,
                                buf_ptr, rest_size);
                            rest_size = 0;
                        }
                    }
                    do_recv(key, roomID);
                }
            }
            else {
                delete over_ex;
            }
            break;
        }
        case OE_gEvent: {
            // key = 방번호
            if (FALSE == retval)
                continue;
            switch (over_ex->messageBuffer[1]) {
            case EventType::Mapset:{
                map_block_set* p = reinterpret_cast<map_block_set*>(over_ex->messageBuffer);
                maps[key].Set_map();
                
                game_end_packet ep;
                ep.roomid = key;
                ep.size = sizeof(ep);
                ep.type = EventType::game_end;
                m_pTimer->push_event(key, OE_gEvent, 1000 * (MAP_BREAK_TIME*9), reinterpret_cast<char*>(&ep));

                break;
            }
            case EventType::Cloud_move: {
                cloud_move_packet* p = reinterpret_cast<cloud_move_packet*>(over_ex->messageBuffer);
                send_cloud_move_packet(p->x, p->z, p->roomid);
                //printf("room: %d cloud x: %f | y: %f\n\n", p->roomid, p->x, p->z);
                maps[key].ismove = true;
                maps[key].cloud_move();
                delete over_ex;
                break;
            }
            case EventType::Mon_move_to_player: {
                mon_move_to_player_event* e = reinterpret_cast<mon_move_to_player_event*>(over_ex->messageBuffer);
                //m_pBot->CheckTarget(e->roomid);
                m_pBot->CheckBehavior(e->roomid);
                m_pBot->RunBot(e->roomid);
                delete over_ex;
                break;
            }
            case EventType::Mon_attack_cooltime: {
                mon_attack_cooltime_event* e = reinterpret_cast<mon_attack_cooltime_event*>(over_ex->messageBuffer);
                m_pBot->monsters[e->roomid][e->key].CanAttack = TRUE;
                delete over_ex;
                break;
            }
            case EventType::MapBreak: {
                Mapbreak_event* p = reinterpret_cast<Mapbreak_event*>(over_ex->messageBuffer);
                maps[key].Map_collapse();

                Mapbreak_event e;
                e.roomid = key;
                e.size = sizeof(e);
                e.type = EventType::MapBreak;
                m_pTimer->push_event(key, OE_gEvent, MAP_BREAK_TIME, reinterpret_cast<char*>(&e));
                delete over_ex;
                break;
            }
            case EventType::Mon_respawn: {
                mon_respawn_event* e = reinterpret_cast<mon_respawn_event*>(over_ex->messageBuffer);
                m_pBot->monsters[e->roomid][e->key].hp = 100;
                m_pBot->monsters[e->roomid][e->key].f3Position = m_pBot->monsters[e->roomid][e->key].SpawnPos.load();
                
                mon_respawn_packet p;
                p.key = e->key;
                p.roomid = e->roomid;
                p.type = SC_monster_respawn;
                p.size = sizeof(p);
                p.Position = m_pBot->monsters[e->roomid][e->key].SpawnPos.load();
                p.dx = 0;
                p.dy = 0;
                p.dz = 0;
                p.MonsterType = m_pBot->monsters[e->roomid][e->key].type;

                send_packet_to_allplayers(e->roomid, reinterpret_cast<char*>(&p));

                m_pBot->monsters[e->roomid][e->key].state = 1;
                break;
            }
            }
        }
        break;
        }

    }
}

bool Server::Init()
{
    sessions.clear();

#ifdef Run_Lobby
    ConnectLobby();
#endif

    // ���� �ʱ�ȭ
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // ����� �Ϸ� ��Ʈ ����
    hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (hcp == NULL) return 0;

    // CPU ���� Ȯ��
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    for (int i = 0; i <(int)si.dwNumberOfProcessors * 2; i++)
        working_threads.emplace_back(std::thread(&Server::WorkerFunc, this));


    timer_thread = std::thread(&Timer::init, m_pTimer, Gethcp());
    accept_thread = std::thread(&Server::Accept, this);

    return 1;
}

void Server::Thread_join()
{
    accept_thread.join();
    timer_thread.join();

    for (auto& t : working_threads)
        t.join();

    for (auto& t : map_threads)
        t.join();

    delete m_pTimer;
    CloseHandle(hcp);
}