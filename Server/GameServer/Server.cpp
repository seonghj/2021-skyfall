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

    state = 0;
    f3Position = DirectX::XMFLOAT3(0.0f, 124.0f, 0.0f);
    m_fPitch = 0;
    m_fYaw = 0;

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

int Server::SetroomID()
{
    int cnt = 1;
    while (1) {
        if (sessions.find(cnt) == sessions.end()) {
            sessions_lock.lock();
            //std::array<SESSION, 20> s{};
            sessions.emplace(cnt, std::array<SESSION, 20>{});
            sessions_lock.unlock();
            
            int i = 0;
            for (auto& m : m_pBot->monsters[cnt]) {
                m.f3Position.store(DirectX::XMFLOAT3(0, 0, 0));
                m.key = i;
                m.roomID = cnt;
                ++i;
            }

            maps_lock.lock();
            maps.emplace(cnt, Map(cnt));
            maps[cnt].SetNum(cnt);
            maps[cnt].init_Map(this, m_pTimer);
            maps_lock.unlock();

            m_pBot->monsters.emplace(cnt, std::array<Monster, 50>{});
            m_pBot->monsterRun = TRUE;
            m_pBot->monsters[cnt][0].SetPosition(300, 197.757935, 300);
            m_pBot->monsters[cnt][0].state = 1;
            m_pBot->monsters[cnt][0].type = MonsterType::Dragon;
            m_pBot->monsters[cnt][0].Rotate(-90.0f, 20.0f, 0.0f);

            /*m_pBot->monsters[cnt][1].SetPosition(400, 197.757935, 400);
            m_pBot->monsters[cnt][1].state = 1;
            m_pBot->monsters[cnt][1].type = MonsterType::Wolf;
            m_pBot->monsters[cnt][1].Rotate(-90.0f, -40.0f, 0.0f);*/

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
        accept_lock.lock();
        client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            printf("accept error: %d", WSAGetLastError());
            break;
        }

        int roomID = SetroomID();
        int client_key = SetClientKey(roomID);
        if (client_key == -1) {
            closesocket(client_sock);
            break;
        }

        sessions[roomID][client_key].init();
        sessions[roomID][client_key].connected = TRUE;
        sessions[roomID][client_key].key = client_key;
        sessions[roomID][client_key].roomID = roomID;
        sessions[roomID][client_key].sock = client_sock;
        sessions[roomID][client_key].clientaddr = clientaddr;
        getpeername(client_sock, (SOCKADDR*)&sessions[roomID][client_key].clientaddr
            , &sessions[roomID][client_key].addrlen);

        sessions[roomID][client_key].over.type = 0;
        sessions[roomID][client_key].over.dataBuffer.len = BUFSIZE;
        sessions[roomID][client_key].over.dataBuffer.buf =
            sessions[roomID][client_key].over.messageBuffer;
        sessions[roomID][client_key].over.is_recv = true;
        sessions[roomID][client_key].over.roomID = roomID;

        sessions[roomID][client_key].near_monster;

        /*for (int i = 0; i < 20; i++)
            sessions[roomID][client_key].near_monster.insert(i);*/

        accept_lock.unlock();
        // ���ϰ� ����� �Ϸ� ��Ʈ ����

        CreateIoCompletionPort((HANDLE)client_sock, hcp, client_key, 0);

        //printf("client_key: %d\n", client_key);
        send_player_key_packet(client_key, roomID);

        do_recv(client_key, roomID);
    }

    // closesocket()
    closesocket(listen_sock); 

    // ���� ����
    WSACleanup();
}

void Server::Disconnected(int key, int roomID)
{
    std::lock_guard<std::mutex> lock_guard(sessions_lock);
    for (int i = 0; i < MAX_PLAYER; ++i) {
        //printf("%d\n", i->second);
        if (sessions[roomID][i].key == key) {
            printf("client_end: IP =%s, port=%d key = %d, Room = %d\n",
                inet_ntoa(sessions[roomID][i].clientaddr.sin_addr)
                , ntohs(sessions[roomID][i].clientaddr.sin_port), key, roomID);
            //send_disconnect_player_packet(key);
            closesocket(sessions[roomID][i].sock);
            sessions[roomID][i].connected = FALSE;
            sessions[roomID][i].key = -1;
            //printf("disconnected %d\n", gameroom[roomID].ID[i]);
            break;
        }
    }

#ifdef Run_DB
    m_pDB->Logout_player(sessions[roomID][key].id);
#endif
    //sessions[roomID].clear();
}

void Server::do_recv(int key, int roomID)
{
    DWORD flags = 0;
    SOCKET client_s = sessions[roomID][key].sock;
    OVER_EX* over = &sessions[roomID][key].over;

    over->dataBuffer.len = BUFSIZE;
    over->dataBuffer.buf = over->messageBuffer;
    ZeroMemory(&over->overlapped, sizeof(over->overlapped));

    if (WSARecv(client_s, &over->dataBuffer, 1, (LPDWORD)sessions[roomID][key].prev_size,
        &flags, &(over->overlapped), NULL)){
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING){
            printf("key: %d recv error: %d\n", key, err_no);
            Disconnected(key, sessions[roomID][key].over.roomID);
        }
    }
}

void Server::send_packet(int to, char* packet, int roomID)
{
    if (SC_NONE >= packet[1] || packet[1] >= CS_NONE) return;

    SOCKET client_s = sessions[roomID][to].sock;
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
        if (sessions[roomID][i].connected.load(std::memory_order_seq_cst))
            send_packet(i, buf, roomID);
    }
   //sessions_lock.unlock();
}

void Server::send_map_collapse_packet(int num, int map_num)
{
    map_collapse_packet p;
    p.size = sizeof(map_collapse_packet);
    p.type = PacketType::SC_map_collapse;
    p.block_num = num;
    p.key = 0;
    p.roomid = map_num;

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

void Server::send_monster_pos(const Monster& mon, XMFLOAT3 pos, XMFLOAT3 direction, float degree)
{
    int roomID = mon.roomID;

    mon_pos_packet p;
    p.size = sizeof(mon_pos_packet);
    p.type = PacketType::SC_monster_pos;
    p.key = mon.key.load();
    p.roomid = mon.roomID.load();
    p.Position = pos;
    p.direction = direction;
    p.degree = degree;
    p.MoveType = 0;
    p.state = 0;
    
    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (sessions[roomID][i].connected == FALSE) continue;
        if (true == in_VisualField(mon, sessions[roomID][i], roomID)) {
            send_packet(sessions[roomID][i].key.load(), reinterpret_cast<char*>(&p), roomID);
            //printf("send to %d \n", sessions[roomID][i].key.load());
        }
    }
}

void Server::send_monster_attack(const Monster& mon, XMFLOAT3 direction, float degree, int target)
{
    int roomID = mon.roomID;

    mon_attack_packet p;
    p.size = sizeof(mon_attack_packet);
    p.type = PacketType::SC_monster_attack;
    p.key = mon.key.load();
    p.roomid = mon.roomID.load();
    p.direction = direction;
    p.degree = degree;
    p.target = target;
    p.PlayerLeftHp = sessions[roomID][target].hp;

    for (int i = 0; i < MAX_PLAYER; ++i) {
        if (sessions[roomID][i].connected == FALSE) continue;
        if (true == in_VisualField(mon, sessions[roomID][i], roomID)) {
            send_packet(sessions[roomID][i].key.load(), reinterpret_cast<char*>(&p), roomID);
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

void Server::game_end(int roomnum)
{
    game_end_packet p;
    p.key = 0;
    p.size = sizeof(p);
    p.type = PacketType::SC_game_end;
    p.roomid = roomnum;

    send_packet_to_allplayers(roomnum, reinterpret_cast<char*>(&p));
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
    case PacketType::CS_player_login: {
        player_login_packet* p = reinterpret_cast<player_login_packet*>(buf);

        int client_key = p->key;
        bool is_Login = false;
        bool b;

#ifdef Run_DB
        if (strcmp(p->id, "test") != 0) {
            b = m_pDB->Search_ID(p->id, &is_Login);

            if (!b && !is_Login) b = m_pDB->Insert_ID(p->id);

            if (is_Login) {
                send_player_loginFail_packet(client_key, sessions[roomID][client_key].roomID);
                Disconnected(client_key, sessions[roomID][client_key].roomID);
                break;
            }
        }
#endif
        strcpy_s(sessions[roomID][client_key].id, p->id);
        send_player_loginOK_packet(client_key, sessions[roomID][client_key].roomID);

        printf("client_connected: IP =%s, port=%d key = %d Room = %d\n",
            inet_ntoa(sessions[roomID][client_key].clientaddr.sin_addr)
            , ntohs(sessions[roomID][client_key].clientaddr.sin_port), client_key, roomID);

        for (auto& s : sessions[roomID]) {
            if ((TRUE == s.connected) && (s.key.load() != client_key)){
                send_add_player_packet(s.key.load(), client_key, roomID);
            }
        }
        for (auto& s : sessions[roomID]) {
            if ((TRUE == s.connected) && (s.key.load() != client_key))
                send_add_player_packet(client_key, s.key.load(), roomID);
        }
        for (auto& m : m_pBot->monsters[roomID]) {
            if (m.state == 1)
                send_add_monster(m.key.load(), roomID, client_key);
        }

        sessions[roomID][client_key].state = 1;

        send_map_packet(client_key, roomID);

        //m_pBot->monsters[roomID][0].SetPosition()

        break;
    }
    case PacketType::CS_game_ready: {
        game_ready_packet* p = reinterpret_cast<game_ready_packet*>(buf);
        sessions[roomID][p->key].isready = true;
        break;
    }
    case PacketType::CS_game_start: {
        start_ok_packet* p = reinterpret_cast<start_ok_packet*>(buf);
        p->size = sizeof(p);
        p->type = SC_start_ok;
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
        packet.roomid = p->roomid;
        packet.state = p->state;
        packet.size = sizeof(player_pos_packet);
        packet.Position = sessions[roomID][p->key].f3Position;
        packet.dx = p->dx;
        packet.dy = p->dy;
        packet.MoveType = p->MoveType;
        packet.dir = p->dir;
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
         send_packet_to_players(p->key, reinterpret_cast<char*>(p), roomID);
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
        case SWORD1HL: {
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
        switch (over_ex->type) {
        case OE_session: {
            if (FALSE == retval)
            {
                //printf("error = %d\n", WSAGetLastError());
                display_error("GQCS", WSAGetLastError());
                Disconnected(key, roomID);
                continue;
            }

            if ((Transferred == 0)) {
                display_error("GQCS", WSAGetLastError());
                Disconnected(key, roomID);
                continue;
            }
            //printf("%d\n", true);
            if (over_ex->is_recv) {
                //printf("thread key: %d\n", Thread_key);
                int rest_size = Transferred;
                char* buf_ptr = over_ex->messageBuffer;
                char packet_size = 0;
                if (0 < sessions[roomID][key].prev_size)
                    packet_size = sessions[roomID][key].packet_buf[0];
                while (rest_size > 0) {
                    if (0 == packet_size) packet_size = buf_ptr[0];
                    int required = packet_size - sessions[roomID][key].prev_size;
                    if (rest_size >= required) {
                        memcpy(sessions[roomID][key].packet_buf + sessions[roomID][key].
                            prev_size, buf_ptr, required);
                        process_packet(key, sessions[roomID][key].packet_buf, roomID);
                        rest_size -= required;
                        buf_ptr += required;
                        packet_size = 0;
                    }
                    else {
                        memcpy(sessions[roomID][key].packet_buf + sessions[roomID][key].prev_size,
                            buf_ptr, rest_size);
                        rest_size = 0;
                    }
                }
                do_recv(key, roomID);
            }
            else {
                delete over_ex;
            }
            break;
        }
        case OE_gEvent: {
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
                m_pBot->monsters[roomID][e->key].CanAttack = TRUE;
                delete over_ex;
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