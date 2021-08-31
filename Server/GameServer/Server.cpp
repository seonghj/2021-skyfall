#pragma once
#pragma warning(disable : 4996)
#include "Server.h"

#define Run_DB
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
    InGamekey = -1;

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
    moveframe = -1;

    for (int i = 0; i < INVENTORY_MAX; i++)
        inventory[i] = 0;
}

void SESSION::Reset()
{
    state = 0;
    f3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_fPitch = 0;
    m_fYaw = 0;

    hp = 100;
    def = 0;
    lv = 0;
    att = 10;
    speed = 20;
    proficiency = 0.0f;
    using_weapon = PlayerType::PT_BASIC;
}

void SESSION::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity, bool isRun)
{
    if (dwDirection)
    {
        XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
        if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, +fDistance);
        if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
        if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, +fDistance);
        if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);

        Move(xmf3Shift, bUpdateVelocity, isRun);
    }
}

void SESSION::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity, bool isRun)
{
    /*if (bUpdateVelocity)
    {
        m_xmf3Velocity.x = 0;
        m_xmf3Velocity.z = 0;
        m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
        if (m_isRunning)
        {
            m_xmf3Velocity.x *= 3.3;
            m_xmf3Velocity.z *= 3.3;
        }
    }
    else
    {
        m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
        m_pCamera->Move(xmf3Shift);
    }*/
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

int Server::SetInGameKey(int roomID)
{
    int cnt = 0;
    while (true){
        if (cnt == MAX_PLAYER)
            return -1;
        if (INVALIDID == GameRooms[roomID].pkeys[cnt]){
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
        if (cnt == 1000)
            return -1;
        if (FALSE == sessions[cnt].connected) {
            return cnt;
        }
        else
            ++cnt;
    }
}

int Server::SetroomID()
{
    //int cnt = 1;
    //while (1) {
    //    if (sessions.find(cnt) == sessions.end()) {
    //        sessions_lock.lock();
    //        //std::array<SESSION, 20> s{};
    //        sessions.emplace(cnt, std::array<SESSION, 20>{});
    //        sessions_lock.unlock();

    //        maps_lock.lock();
    //        maps.emplace(cnt, Map(cnt));
    //        maps[cnt].SetNum(cnt);
    //        //maps[cnt].init_Map(this, m_pTimer);
    //        maps_lock.unlock();

    //        m_pBot->monsters.emplace(cnt, std::array<Monster, MAX_MONSTER>{});
    //        m_pBot->monsterRun.emplace(cnt, false);
    //        m_pBot->Init(cnt);

    //        /*m_pBot->monsters[cnt][0].SetPosition(2000, 197.757935, 5000);
    //        m_pBot->monsters[cnt][0].state = 1;
    //        m_pBot->monsters[cnt][0].type = MonsterType::Dragon;
    //        m_pBot->monsters[cnt][0].Rotate(-90.0f, 20.0f, 0.0f);
    //        m_pBot->monsters[cnt][0].key = 0;*/

    //        m_pBot->RunBot(cnt);

    //        printf("create game room - %d\n", cnt);
    //        return cnt;
    //    }
    //    if (FALSE == sessions[cnt][MAX_PLAYER - 1].connected)
    //        return cnt;
    //    else {
    //       ++cnt;
    //    }
    //}
}

int Server::CreateRoom(int key, char* name)
{
    if (GameRooms.count(key) != 0) return 1;

    for (auto& g : GameRooms) {
        if (strcmp(g.second.name, name) == 0) {
            return 2;
        }
    }

    GameRooms.emplace(key, GameRoom{});

    for (int i = 0; i < MAX_PLAYER; ++i)
        GameRooms[key].pkeys[i] = INVALIDID;

    strcpy_s(GameRooms[key].name, name);

    maps_lock.lock();
    GameRooms[key].m_pMap = new Map;
    GameRooms[key].m_pMap->SetNum(key);
    //maps[key].init_Map(this, m_pTimer);
    maps_lock.unlock();

    m_pBot->monsters.emplace(key, std::array<Monster, MAX_MONSTER>{});
    //m_pBot->monsterRun = TRUE;
    m_pBot->Init(key);
    //m_pBot->RunBot(key);
    GameRooms[key].CanJoin = true;
    GameRooms[key].TotalPlayer = 0;

    return 0;
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

        int client_key = SetLobbyKey();
        if (client_key == -1) {
            closesocket(client_sock);
            break;
        }

        sessions[client_key].init();
        sessions[client_key].connected = TRUE;
        sessions[client_key].key = client_key;
        sessions[client_key].roomID = INVALUED_ID;
        sessions[client_key].sock = client_sock;
        sessions[client_key].clientaddr = clientaddr;
        getpeername(client_sock, (SOCKADDR*)&sessions[client_key].clientaddr
            , &sessions[client_key].addrlen);

        ZeroMemory(&sessions[client_key].over.overlapped
            , sizeof(sessions[client_key].over.overlapped));
        sessions[client_key].over.type = 0;
        sessions[client_key].over.dataBuffer.len = BUFSIZE;
        sessions[client_key].over.dataBuffer.buf =
            sessions[client_key].over.messageBuffer;
        sessions[client_key].over.is_recv = true;
        sessions[client_key].over.roomID = INVALUED_ID;


        /*for (int i = 0; i < 20; i++)
            Lobby_sessions[client_key].near_monster.insert(i);*/

        accept_lock.unlock();
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


void Server::Disconnected(int key)
{
    printf("client_end: IP =%s, port=%d Lobby key = %d\n",
        inet_ntoa(sessions[key].clientaddr.sin_addr)
        , ntohs(sessions[key].clientaddr.sin_port), key);
    //send_disconnect_player_packet(key);
    closesocket(sessions[key].sock);
    sessions[key].connected = FALSE;
    sessions[key].key = INVALIDID;
    std::lock_guard<std::mutex> lock_guard(GameRooms_lock);
    if (sessions[key].roomID != INVALIDID) {
        if (sessions[key].InGamekey == INVALIDID) return;
        --GameRooms[key].TotalPlayer;
        GameRooms[sessions[key].roomID].pkeys[sessions[key].InGamekey] = INVALIDID;
        int roomid = sessions[key].roomID;

        if (GameRooms[key].TotalPlayer < -0){
            GameRooms[roomid].CanJoin = false;
            delete GameRooms[roomid].m_pMap;  
            ZeroMemory(GameRooms[roomid].name, sizeof(GameRooms[roomid].name));
            GameRooms.erase(roomid);
            m_pBot->monsters_lock.lock();
            m_pBot->monsters.erase(roomid);
            m_pBot->monsterRun.erase(roomid);
            m_pBot->monsters_lock.unlock();

            printf("delete Room: %d\n", roomid);
        }
    }
    //printf("disconnected %d\n", gameroom[roomID].ID[i]);

#ifdef Run_DB
    if(m_pDB->isRun)
        m_pDB->Logout_player(sessions[key].id);
#endif
    //sessions.clear();
}

void Server::do_recv(int key, int roomID)
{
    DWORD flags = 0;

    SOCKET client_s = sessions[key].sock;
    OVER_EX* over = &sessions[key].over;

    over->dataBuffer.len = BUFSIZE;
    over->dataBuffer.buf = over->messageBuffer;
    ZeroMemory(&over->overlapped, sizeof(over->overlapped));

    if (WSARecv(client_s, &over->dataBuffer, 1, (LPDWORD)sessions[key].prev_size,
        &flags, &(over->overlapped), NULL)) {
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING) {
            /*if (over->dataBuffer.buf[1] != 0) {
                printf("key: %d recv error: %d / packet: %d\n", key, err_no
                    , over->dataBuffer.buf[1]);
                Disconnected(key, sessions[key].over.roomID);
            }*/
            printf("key: %d room: %d recv error: %d / packet: %d\n", key, roomID, err_no
                , over->dataBuffer.buf[1]);
            Disconnected(key);
        }
    }
}

void Server::send_packet(int to, char* packet, int roomID)
{
    //if (to <= INVALIDID) return;
    if (SC_NONE >= packet[1] || packet[1] >= CS_NONE) return;
    //printf("packet num = %d\n", packet[1]);
    SOCKET client_s = sessions[to].sock;

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
            //Disconnected(to, sessions[to].over.roomID);
        }
    }
    //printf("to: %d packet: %d send\n", to, packet[1]);
}

// Lobby
void Server::send_player_InGamekey_packet(int key, int roomID)
{
    player_key_packet p;

    p.key = sessions[key].InGamekey;
    p.size = sizeof(player_key_packet);
    p.type = PacketType::SC_player_InGamekey;
    p.roomid = roomID;
    send_packet(key, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_Lobby_key_packet(int key)
{
    player_key_packet p;

    p.key = key;
    p.size = sizeof(player_key_packet);
    p.type = PacketType::SC_player_Lobbykey;
    p.roomid = INVALIDID;

    send_packet(key, reinterpret_cast<char*>(&p), INVALIDID);
}

void Server::send_Lobby_loginOK_packet(int key)
{
    player_loginOK_packet p;

    p.key = key;
    p.size = sizeof(player_loginOK_packet);
    p.type = PacketType::SC_player_loginOK;
    p.roomid = INVALIDID;
    p.Position = sessions[key].f3Position.load();
    p.dx = sessions[key].m_fPitch.load();
    p.dy = sessions[key].m_fYaw.load();
    send_packet(key, reinterpret_cast<char*>(&p), INVALIDID);
}

void Server::send_room_list_packet(int key)
{
    for (int i = 0; i < MAX_ROOM; ++i) {
        if (sessions[key].playing == true) break;
        if (GameRooms.count(i) == 0) continue;
        if (GameRooms[i].CanJoin == false) continue;
        if (GameRooms[i].name == NULL) continue;
        room_list_packet p;
        p.key = key;
        p.size = sizeof(p);
        p.type = PacketType::SC_room_list;
        p.roomid = INVALIDID;
        p.idx = i;
        strcpy_s(p.name, GameRooms[i].name);
        printf("send room %d %s\n", i, GameRooms[i].name);
        send_packet(key, reinterpret_cast<char*>(&p), INVALIDID);
    }
}

void Server::send_player_loginOK_packet(int key, int roomID)
{
    player_loginOK_packet p;

    p.key = key;
    p.size = sizeof(player_loginOK_packet);
    p.type = PacketType::SC_player_loginOK;
    p.roomid = roomID;
    p.Position = sessions[key].f3Position.load();
    p.dx = sessions[key].m_fPitch.load();
    p.dy = sessions[key].m_fYaw.load();
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

void Server::send_start_packet(int to, int roomID)
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dis(100, MAP_SIZE - 100);

    game_start_packet p;
    p.type = PacketType::SC_start_ok;
    p.size = sizeof(p);
    p.key = to;
    p.roomid = roomID;
    p.weaponType = sessions[to].using_weapon;
    p.ingamekey = sessions[to].InGamekey;
    p.pos.x = dis(gen);
    p.pos.y = 500;
    p.pos.z = dis(gen);
    sessions[to].f3Position = p.pos;

    send_packet(to, reinterpret_cast<char*>(&p), roomID);

    if (GameRooms[roomID].m_pMap->game_start == false) {
        GameRooms[roomID].m_pMap->game_start = true;
        GameRooms[roomID].m_pMap->init_Map(this, m_pTimer);
        printf("Room %d Game start\n", roomID);
        Mapbreak_event e;
        e.roomid = roomID;
        e.size = sizeof(e);
        e.type = EventType::MapBreak;
        m_pTimer->push_event(roomID, OE_gEvent, MAP_BREAK_TIME, reinterpret_cast<char*>(&e));
    }

    if (m_pBot->monsterRun[roomID] == false) {
        m_pBot->monsterRun[roomID] = true;
        m_pBot->RunBot(roomID);
    }
}

// In Game
void Server::send_add_player_packet(int key, int to, int roomID)
{
    player_add_packet p;

    p.key = sessions[key].InGamekey;
    p.size = sizeof(player_add_packet);
    p.type = PacketType::SC_player_add;
    p.roomid = roomID;
    p.Position = sessions[key].f3Position.load();
    p.dx = sessions[key].m_fPitch.load();
    p.dy = sessions[key].m_fYaw.load();
    p.WeaponType = sessions[key].using_weapon;

    //printf("%d send login to %d\n",key, to);

    send_packet(to, reinterpret_cast<char*>(&p), roomID);
}

void Server::send_remove_player_packet(int ingamekey, int roomID)
{
    if (ingamekey >= MAX_PLAYER || ingamekey < 0) return;
    player_remove_packet p;
    p.key = ingamekey;
    p.size = sizeof(player_remove_packet);
    p.type = PacketType::SC_player_remove;
    p.roomid = roomID;

    for (auto& k : GameRooms[roomID].pkeys) {
        if (sessions[k].connected == TRUE)
            send_packet(sessions[k].key, reinterpret_cast<char*>(&p), roomID);
    }
    Disconnected(GameRooms[roomID].pkeys[ingamekey]);
}

void Server::send_disconnect_player_packet(int ingamekey, int roomID)
{
    if (ingamekey >= MAX_PLAYER || ingamekey < 0) return;
    player_disconnect_packet p;
    p.key = ingamekey;
    p.size = sizeof(player_disconnect_packet);
    p.type = PacketType::SC_player_disconnect;
    p.roomid = roomID;

    for (auto& k : GameRooms[roomID].pkeys) {
        if (sessions[k].connected == TRUE)
            send_packet(sessions[k].key, reinterpret_cast<char*>(&p), roomID);
    }
    Disconnected(GameRooms[roomID].pkeys[ingamekey]);
}

void Server::send_packet_to_players(int ingamekey, char* buf, int roomID)
{
    if (ingamekey >= MAX_PLAYER || ingamekey < 0) return;
   //sessions_lock.lock();
    for (int k : GameRooms[roomID].pkeys) {
        if (k == INVALIDID) continue;
        if (sessions[k].connected == FALSE) continue;
        if (sessions[k].playing == FALSE) continue;
        if (in_VisualField(sessions[k]
            , sessions[GameRooms[roomID].pkeys[ingamekey]], roomID)) {
            send_packet(k, buf, roomID);
        }
    }
    //sessions_lock.unlock();
}

void Server::send_packet_to_allplayers(int roomID, char* buf)
{
    if (MAX_ROOM <= roomID) return;
   //sessions_lock.lock();
    for (int k : GameRooms[roomID].pkeys) {
        if (k == INVALIDID) continue;
        if (sessions[k].connected == FALSE) continue;
        if (sessions[k].playing == FALSE) continue;
        send_packet(k, buf, roomID);
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
    if (key >= MAX_MONSTER) return;
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

void Server::send_monster_pos(const Monster& mon, XMFLOAT3 direction, int target)
{
    if (mon.key >= MAX_MONSTER) return;
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
    p.target = target;

    //printf("%d\n", mon.key);

    for (int k : GameRooms[roomID].pkeys) {
        if (k == INVALIDID) continue;
        if (sessions[k].connected == FALSE) continue;
        if (sessions[k].playing == FALSE) continue;
        if (in_VisualField(mon, sessions[k], roomID)) {
            send_packet(k, reinterpret_cast<char*>(&p), roomID);
        }
    }
}

void Server::send_monster_attack(const Monster& mon, XMFLOAT3 direction, int target)
{
    if (mon.key >= MAX_MONSTER) return;
    int roomID = mon.roomID.load();

    mon_attack_packet p;
    p.size = sizeof(p);
    p.type = PacketType::SC_monster_attack;
    p.key = mon.key;
    p.roomid = mon.roomID.load();
    p.direction = direction;
    p.degree = mon.m_fRoll.load();
    p.target = target;
    p.PlayerLeftHp = sessions[GameRooms[roomID].pkeys[target]].hp.load()
        - CalcDamageToMon(sessions[GameRooms[roomID].pkeys[target]].att,
            sessions[GameRooms[roomID].pkeys[target]].def);

    if (mon.type == MonsterType::Dragon)
        p.attack_dis = Atack_Distance_Dragon;
    else if (mon.type == MonsterType::Wolf)
        p.attack_dis = Atack_Distance_Wolf;
    else if (mon.type == MonsterType::Metalon)
        p.attack_dis = Atack_Distance_Metalon;

    for (auto& k : GameRooms[roomID].pkeys) {
        if (sessions[k].connected == FALSE) continue;
        if (sessions[k].playing == FALSE) continue;
        if (in_VisualField(mon, sessions[k], roomID)) {
            send_packet(k, reinterpret_cast<char*>(&p), roomID);
        }
    }
}

void Server::send_monster_stop(int key, int roomID)
{
    if (key >= MAX_MONSTER) return;
    mon_stop_packet p;
    p.size = sizeof(p);
    p.type = PacketType::SC_monster_stop;
    p.key = key;
    p.roomid = roomID;
    p.Position = m_pBot->monsters[roomID][key].f3Position.load();

    for (auto& k : GameRooms[roomID].pkeys) {
        if (sessions[k].connected == FALSE) continue;
        if (sessions[k].playing == FALSE) continue;
        if (in_VisualField(m_pBot->monsters[roomID][key], sessions[k], roomID)) {
            send_packet(k, reinterpret_cast<char*>(&p), roomID);
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
        p.block_type[i] = GameRooms[roomID].m_pMap->Map_type[i];
    }

    send_packet(to, reinterpret_cast<char*>(&p), roomID);
}


void Server::send_game_end_packet(int key, int roomID)
{
    game_end_packet p;
    p.key = key;
    p.size = sizeof(p);
    p.type = PacketType::SC_game_end;
    p.roomid = roomID;
    send_packet(key, reinterpret_cast<char*>(&p), roomID);
    printf("send end game to %d\n", key);
}

void Server::send_player_dead_packet(int ingamekey, int roomID)
{
    if (ingamekey >= MAX_PLAYER || ingamekey < 0) return;
    player_dead_packet p;
    p.key = GameRooms[roomID].pkeys[ingamekey];
    p.ingamekey = ingamekey;
    p.size = sizeof(p);
    p.type = SC_player_dead;
    p.roomid = roomID;

    send_packet_to_allplayers(roomID, reinterpret_cast<char*>(&p));

    //send_packet(ingamekey, reinterpret_cast<char*>(&p), roomID);

    sessions[GameRooms[roomID].pkeys[ingamekey]].playing == FALSE;

#ifdef Run_DB
    if (m_pDB->isRun)
        m_pDB->Send_player_record(sessions[GameRooms[roomID].pkeys[ingamekey]]
            , 0, GameRooms[roomID].TotalPlayer);
#endif 
}

void Server::game_end(int roomnum)
{
    if (GameRooms.count(roomnum) == 0) return;
    GameRooms[roomnum].CanJoin = false;
    GameRooms[roomnum].TotalPlayer = 0;
    for (int key : GameRooms[roomnum].pkeys) {
        if (key == INVALIDID) continue;
        //if (sessions[key].connected == false || sessions[key].playing == false) continue;
        sessions[key].playing = false;
        send_game_end_packet(key, roomnum);
        sessions[key].InGamekey = INVALIDID;
#ifdef Run_DB
        if (m_pDB->isRun)
            m_pDB->Send_player_record(sessions[key], 0
                , GameRooms[roomnum].TotalPlayer);
#endif 
    }
    delete GameRooms[roomnum].m_pMap;
    ZeroMemory(GameRooms[roomnum].name, sizeof(GameRooms[roomnum].name));
    GameRooms_lock.lock();
    GameRooms[roomnum].master = INVALIDID;
    GameRooms.erase(roomnum);
    GameRooms_lock.unlock();

    /*maps_lock.lock();
    maps.erase(roomnum);
    maps_lock.unlock();*/

    m_pBot->monsters_lock.lock();
    m_pBot->monsters.erase(roomnum);
    m_pBot->monsterRun.erase(roomnum);
    m_pBot->monsters_lock.unlock();

    printf("Room %d Game End Left room %d\n", roomnum, (int)GameRooms.size());
}

void Server::Delete_room(int roomID)
{
    GameRooms[roomID].CanJoin = false;
    GameRooms[roomID].TotalPlayer = 0;
    GameRooms_lock.lock();
    delete GameRooms[roomID].m_pMap;
    ZeroMemory(GameRooms[roomID].name, sizeof(GameRooms[roomID].name));
    for (auto& key : GameRooms[roomID].pkeys)
        if (key != -1)
            send_room_list_packet(key);
    GameRooms.erase(roomID);

    GameRooms_lock.unlock();

    /*maps_lock.lock();
    maps.erase(roomnum);
    maps_lock.unlock();*/

    m_pBot->monsters_lock.lock();
    m_pBot->monsters.erase(roomID);
    m_pBot->monsterRun.erase(roomID);
    m_pBot->monsters_lock.unlock();

    printf("delete room %d\n", roomID);
}

void Server::player_go_lobby(int key, int roomID)
{
    //send_game_end_packet(key, roomID);
    if (GameRooms.count(roomID) != 0)
        GameRooms[roomID].pkeys[sessions[key].InGamekey] = INVALIDID;

    sessions[key].Reset();
    sessions[key].roomID = INVALIDID;
    sessions[key].over.roomID = INVALUED_ID;
    sessions[key].playing = false;
    sessions[key].InGamekey = INVALIDID;


    return_lobby_packet p;
    p.key = key;
    p.roomid = roomID;
    p.size = sizeof(p);
    p.type = SC_return_lobby;
    send_packet(key, reinterpret_cast<char*>(&p), roomID);
    
    //send_room_list_packet(key);
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
    sessions[key].m_fPitch.store(dx);
    sessions[key].m_fYaw.store(dy);
    sessions[key].f3Position = pos;

    /*std::lock_guard <std::mutex> lg(sessions[client_key].nm_lock);
    std::unordered_set<int> old_nm;
    std::unordered_set<int> new_nm;

    old_nm = sessions[client_key].near_monster;

    for (auto& m : m_pBot->monsters[roomID]) {
        if (m.state == 0) continue;
        if (in_VisualField(m, sessions[client_key], roomID)) {
            new_nm.insert(m.key.load());
        }
    }

    for (auto m : new_nm) {
        if (m_pBot->monsters[roomID][m].state == 0) continue;
        if (old_nm.find(m) == old_nm.end()) {
            sessions[client_key].near_monster.insert(m);
            send_add_monster(m, roomID, key);
        }
    }

    for (auto m : old_nm) {
        if (m_pBot->monsters[roomID][m].state == 0) continue;
        if (new_nm.find(m) == new_nm.end()) {
            sessions[client_key].near_monster.erase(m);
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
        if (m_pDB->isRun) {
            if (strcmp(p->id, "test") != 0) {
                b = m_pDB->Search_ID(p->id, p->pw);

                if (!b) {
                    send_player_loginFail_packet(client_key, INVALIDID);
                    break;
                }
            }
        }
#endif
        send_room_list_packet(client_key);

        strcpy_s(sessions[client_key].id, p->id);

        send_Lobby_loginOK_packet(client_key);

        printf("client_connected to Lobby: IP =%s, port=%d key = %d\n",
            inet_ntoa(sessions[client_key].clientaddr.sin_addr)
            , ntohs(sessions[client_key].clientaddr.sin_port), client_key);

        break;
    }
    case PacketType::CS_create_account: {
        create_account_packet* p = reinterpret_cast<create_account_packet*>(buf);
        int key = p->key;
        bool b = true;
        bool is_Login = false;
        if (sessions[key].playing == true) break;
#ifdef Run_DB
        if (m_pDB->isRun) {
            b = m_pDB->Insert_ID(p->id, p->pw);

            if (!b)
                p->canmake = false;
            p->type = SC_create_account;
            send_packet(key, reinterpret_cast<char*>(p), INVALIDID);
        }
#endif
        break;
    }
    case PacketType::CS_game_start: {
        game_start_packet* p = reinterpret_cast<game_start_packet*>(buf);
        if (p->key == GameRooms[p->roomid].master) {

            bool ready = false;

            GameRooms[p->roomid].CanJoin = false;
            for (int client_key : GameRooms[p->roomid].pkeys) {
                if (client_key == INVALIDID) continue;
                if (sessions[client_key].playing == true) continue;

                sessions[client_key].roomID = p->roomid;
                sessions[client_key].over.roomID = p->roomid;

                //sessions[client_key].using_weapon = p->weaponType;

                printf("connected to Game: IP =%s, port=%d key = %d Room = %d / nkey: %d\n",
                    inet_ntoa(sessions[client_key].clientaddr.sin_addr)
                    , ntohs(sessions[client_key].clientaddr.sin_port), key, p->roomid
                    , sessions[client_key].InGamekey);

                sessions[client_key].state = Alive;

                send_start_packet(client_key, p->roomid);

                //send_map_packet(client_key, roomID);

                sessions[client_key].isready = true;
                sessions[client_key].playing = true;

                sessions[client_key].hp = 100;
                sessions[client_key].def = 0;
                sessions[client_key].lv = 0;
                sessions[client_key].att = 10;
                sessions[client_key].speed = 20;
                sessions[client_key].proficiency = 0.0f;


                for (auto& k : GameRooms[p->roomid].pkeys) {
                    if ((TRUE == sessions[k].connected) && (k != client_key)) {
                        send_add_player_packet(client_key, k, p->roomid);
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                }

                for (auto& k : GameRooms[p->roomid].pkeys) {
                    if ((TRUE == sessions[k].connected) && (k != client_key)) {
                        send_add_player_packet(k, client_key, p->roomid);
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                }

                for (int i = 0; i < MAX_MONSTER; ++i) {
                    if (m_pBot->monsters[p->roomid][i].state == 1) {
                        //printf("send monster %d\n", i);
                        send_add_monster(i, p->roomid, client_key);
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                }
            }
        }
        break;
    }
    case PacketType::CS_create_room: {
        room_create_packet* p = reinterpret_cast<room_create_packet*>(buf);
        int key = p->key;
        //if (sizeof(p->name) <= 0) break;
        if (GameRooms.size() >= MAX_ROOM) break;
        GameRooms_lock.lock();
        if (GameRooms.size() > 0) {
            for (int i = 1; i <= MAX_ROOM; i++) {
                if (GameRooms.count(i) > 0) {
                    if (GameRooms[i].TotalPlayer <= 0)
                        GameRooms.erase(i);
               }
            }
        }
        GameRooms_lock.unlock();
        int cnt = 0;
        // 0 = 정상 1 = 꽉참 2 = 방이름 같음
        int error = 0;
        while (cnt <= MAX_ROOM) {
            GameRooms_lock.lock();
            error = CreateRoom(cnt, p->name);
            GameRooms_lock.unlock();
            if (error == 0) break;
            if (error == 2) {
                room_list_packet r;
                r.key = key;
                r.size = sizeof(r);
                r.type = PacketType::SC_room_list;
                r.roomid = INVALIDID;
                r.idx = INVALIDID;
                send_packet(key, reinterpret_cast<char*>(&r), INVALIDID);
                break;
            }

            ++cnt;
        }
        if (error == 2) break;

        GameRooms[cnt].master = key;
        ++GameRooms[cnt].TotalPlayer;

        int nkey = SetInGameKey(cnt);
        if (nkey == INVALIDID) {
            printf("Room %d no empty nkey\n", cnt);
            Disconnected(key);
            break;
        }
        GameRooms[cnt].pkeys[nkey] = key;
        sessions[key].InGamekey = nkey;
        sessions[key].roomID = cnt;
        sessions[key].over.roomID = cnt;
        printf("player key: %d create game room - %d nkey %d\n", key, cnt, nkey);
        //printf("Left room %d\n", (int)GameRooms.size());
        room_select_packet s;
        s.key = nkey;
        s.room = cnt;
        s.roomid = cnt;
        s.size = sizeof(s);
        s.type = SC_select_room;
        s.ingamekey = nkey;
        send_packet(p->key, reinterpret_cast<char*>(&s), -1);
        break;
    }
    case PacketType::CS_room_select: {
        room_select_packet* p = reinterpret_cast<room_select_packet*>(buf);
        std::lock_guard<std::mutex> lock_guard(GameRooms_lock);
        if (GameRooms.find(p->room) == GameRooms.end()) {
            break;
        }

        if (GameRooms[p->room].CanJoin == false) break;
        if (GameRooms[key].TotalPlayer >= MAX_PLAYER) break;

        p->type = SC_select_room;

        int nkey = SetInGameKey(p->room);
        if (nkey == INVALIDID) {
            printf("Room %d no empty nkey\n", p->room);
            Disconnected(key);
            break;
        }
        printf("player key: %d in game room - %d nkey %d\n", key, p->room, nkey);
        GameRooms[p->room].pkeys[nkey] = p->key;
        sessions[p->key].InGamekey = nkey;
        sessions[p->key].roomID = p->room;
        sessions[p->key].over.roomID = p->room;
        p->ingamekey = nkey;
        ++GameRooms[p->room].TotalPlayer;

        send_packet(p->key, reinterpret_cast<char*>(p), -1);
        break;
    }
    case PacketType::CS_return_lobby: {
        return_lobby_packet* p = reinterpret_cast<return_lobby_packet*>(buf);
        //std::lock_guard<std::mutex> lock_guard(GameRooms_lock);
        if (p->key == GameRooms[p->roomid].master
            && sessions[p->key].playing == false) {
            GameRooms[p->roomid].CanJoin = false;
            for (int k : GameRooms[p->roomid].pkeys) {
                if (k != INVALIDID) {
                    player_go_lobby(k, p->roomid);
                }
            }
            Delete_room(p->roomid);
        }
        else {
            GameRooms[p->roomid].pkeys[p->key] = INVALIDID;
            --GameRooms[p->roomid].TotalPlayer;
            if (GameRooms[p->roomid].TotalPlayer == 0)
                Delete_room(p->roomid);
            player_go_lobby(p->key, p->roomid);
        }
        /*if (GameRooms.count(p->roomid) != 0) {
            if (GameRooms[p->roomid].master != INVALIDID) {
                GameRooms[p->roomid].pkeys[sessions[p->key].InGamekey] = INVALIDID;
                --GameRooms[p->roomid].TotalPlayer;
                if (GameRooms[p->roomid].TotalPlayer == 0)
                    Delete_room(p->roomid);
            }
        }*/
        //player_go_lobby(p->key, p->roomid);
        break;
    }
    case PacketType::CS_refresh_lobby: {
        refresh_lobby_packet* p = reinterpret_cast<refresh_lobby_packet*>(buf);
        send_room_list_packet(p->key);
        break;
    }
    case PacketType::CS_player_info:{
        player_info_packet* p = reinterpret_cast<player_info_packet*>(buf);
        p->type = SC_player_info;
        break;
    }
    case PacketType::CS_player_pos: {
        player_pos_packet* p = reinterpret_cast<player_pos_packet*>(buf);
        if (0 > p->ingamekey || p->ingamekey >= 20) break;
        //printf("frame: %d dx = %f dy = %f\n", p->frame, p->dx, p->dy);
        player_move(p->key, p->roomid, p->Position, p->dx, p->dy);
        player_pos_packet packet;
        packet.type = SC_player_pos;
        packet.key = p->key;
        packet.ingamekey = p->ingamekey;
        packet.roomid = roomID;
        packet.state = p->state;
        packet.size = sizeof(player_pos_packet);
        packet.Position = sessions[p->key].f3Position;
        packet.dx = sessions[p->key].m_fPitch;
        packet.dy = sessions[p->key].m_fYaw;
        packet.MoveType = p->MoveType;
        packet.dir = p->dir;
        packet.playertype = sessions[p->key].using_weapon;

        /*if (sessions[p->key].playing == FALSE) {
            send_packet(p->key, reinterpret_cast<char*>(&packet), roomID);
            break;
        }*/

        send_packet_to_players(p->ingamekey, reinterpret_cast<char*>(&packet), roomID);

        if (sessions[p->key].f3Position.load().y <= -10) {
            sessions[p->key].state = Death;
            sessions[p->key].playing = false;
            send_player_dead_packet(p->ingamekey, p->roomid);
        }

        break;
    }
    case PacketType::CS_start_pos: {
        player_start_pos* p = reinterpret_cast<player_start_pos*>(buf);
        sessions[GameRooms[p->roomid].pkeys[p->key]].f3Position = p->Position;
        send_packet(key, reinterpret_cast<char*>(p), roomID);
        break;
    }
    case PacketType::CS_weapon_swap :{
         Weapon_swap_packet* p = reinterpret_cast<Weapon_swap_packet*>(buf);
         if (0 > p->ingamekey || p->ingamekey >= 20) break;
         p->type = SC_weapon_swap;
         sessions[p->key].using_weapon = p->weapon;
         //printf("player %d swap to %d\n", p->key, p->weapon);
         send_packet_to_allplayers(roomID, reinterpret_cast<char*>(p));
         break;
    }
    case PacketType::CS_player_move: {
        player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);
        if (0 > p->ingamekey || p->ingamekey >= 20) break;
        p->type = SC_player_move;
        send_packet_to_players(p->ingamekey, reinterpret_cast<char*>(p), roomID);
        break;
    }
    case PacketType::CS_player_attack:{
        player_attack_packet* p = reinterpret_cast<player_attack_packet*>(buf);
        if (0 > p->ingamekey || p->ingamekey >= 20) break;
        p->type = SC_player_attack;
        send_packet_to_players(p->ingamekey, reinterpret_cast<char*>(p), roomID);
        break;

    }
    case PacketType::CS_allow_shot: {
        player_shot_packet* p = reinterpret_cast<player_shot_packet*>(buf);
        if (0 > p->ingamekey || p->ingamekey >= 20) break;
        p->type = SC_allow_shot;
        send_packet_to_players(p->ingamekey, reinterpret_cast<char*>(p), roomID);
        break;
    }
    case PacketType::CS_player_stop: {
        player_stop_packet* p = reinterpret_cast<player_stop_packet*>(buf);
        if (0 > p->ingamekey || p->ingamekey >= 20) break;
        p->type = SC_player_stop;
        //p->Position = sessions[GameRooms[p->roomid].pkeys[p->key]].f3Position;
        send_packet_to_players(p->ingamekey, reinterpret_cast<char*>(p), roomID);
        break;
    }
    case PacketType::CS_player_getitem: {
        player_getitem_packet* p = reinterpret_cast<player_getitem_packet*>(buf);
        int key = p->key;
        int room = p->roomid;
        for (auto& i : sessions[GameRooms[p->roomid].pkeys[p->key]].inventory) {
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
        sessions[GameRooms[p->roomid].pkeys[p->target]].hp
            = p->PlayerLeftHp;

        if (sessions[GameRooms[roomID].pkeys[p->target]].hp.load() <= 0) {
            sessions[GameRooms[roomID].pkeys[p->target]].state = Death;
            send_player_dead_packet(p->target, roomID);
            --GameRooms[roomID].TotalPlayer;
            if (GameRooms[roomID].TotalPlayer <= 1)
                game_end(roomID);
        }
        break;
    }
    case PacketType::CS_monster_damaged: {
        mon_damaged_packet* p = reinterpret_cast<mon_damaged_packet*>(buf);
        if (0 > p->ingamekey || p->ingamekey >= 20) break;
        int key = p->key;
        int target = p->target;
        if (m_pBot->monsters[p->roomid][target].state == 0) break;

        p->type = SC_monster_damaged;
        p->damage = (sessions[key].att * (1.f + p->nAttack / 4.f))
            * (100 - m_pBot->monsters[p->roomid][target].def) / 100;
        sessions[key].AddProficiency();
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

        send_packet_to_players(p->ingamekey, reinterpret_cast<char*>(p), p->roomid);

        //printf("%f\n", m_pBot->monsters[p->roomid][target].hp.load());

        break;
    }
    case PacketType::CS_player_damage: {
        player_damage_packet* p = reinterpret_cast<player_damage_packet*>(buf);
        if (0 > p->ingamekey || p->ingamekey >= 20) break;
        int key = p->key;
        int ingamekey = p->ingamekey;
        int target = p->target;
        if (sessions[key].roomID
            != sessions[GameRooms[p->roomid].pkeys[target]].roomID) break;

        p->type = SC_player_damage;
        p->damage = (sessions[key].GetAtkDamage() 
            * (1.f + p->nAttack / 4.f))
            * (100 - sessions[GameRooms[p->roomid].pkeys[target]].def) / 100;
        sessions[GameRooms[p->roomid].pkeys[target]].hp 
            = sessions[GameRooms[p->roomid].pkeys[target]].hp - p->damage;
        p->leftHp = sessions[GameRooms[p->roomid].pkeys[target]].hp;
        sessions[key].AddProficiency();
        send_packet_to_players(ingamekey, reinterpret_cast<char*>(p), p->roomid);

        if (sessions[GameRooms[p->roomid].pkeys[target]].hp.load() <= 0) {
            sessions[GameRooms[p->roomid].pkeys[target]].state = Death;
            //printf("Room: %d player: %d Dead\n", p->roomid, target);
            send_player_dead_packet(target, roomID);
            --GameRooms[p->roomid].TotalPlayer;
            //printf("left %d\n", GameRooms[p->roomid].TotalPlayer);
            if (GameRooms[p->roomid].TotalPlayer <= 1)
                game_end(p->roomid);
        }

        break;
    }
    case PacketType::CS_player_dead: {
        player_dead_packet* p = reinterpret_cast<player_dead_packet*>(buf);
        if (0 > p->ingamekey || p->ingamekey >= 20) break;
        p->type = PacketType::SC_player_dead;
        sessions[p->key].state = Death;
        send_packet_to_allplayers(p->roomid, reinterpret_cast<char*>(p));
        --GameRooms[p->roomid].TotalPlayer;
        if (GameRooms[p->roomid].TotalPlayer <= 1)
            game_end(p->roomid);
        //printf("Room: %d player: %d Dead\n", p->roomid, p->ingamekey);
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
            // key = 유저번호
            if (FALSE == retval)
            {
                //printf("error = %d\n", WSAGetLastError());
                display_error("GQCS", WSAGetLastError());
                Disconnected(key);
                continue;
            }

            if ((Transferred == 0)) {
                display_error("GQCS", WSAGetLastError());
                Disconnected(key);
                continue;
            }
            //printf("%d\n", true);
            if (over_ex->is_recv) {
                char* packet_ptr = over_ex->messageBuffer;
                int num_data = Transferred + sessions[key].prev_size;
                int packet_size = packet_ptr[0];
                while (num_data >= packet_size) {
                    if (num_data >= BUFSIZE) break;
                    if (packet_size <= 0) break;
                    //printf("num_data: %d, packet_size: %d prev_size: %d\n", num_data, packet_size, sessions[key].prev_size);
                    process_packet(key, packet_ptr, roomID);
                    num_data -= packet_size;
                    packet_ptr += packet_size;
                    packet_size = packet_ptr[0];
                    if (0 >= num_data) {
                        //ZeroMemory(packet_ptr, sizeof(packet_ptr));
                        packet_size = 0;
                        break;
                    }
                }
                sessions[key].prev_size = 0;
                if (0 != num_data)
                    memcpy(over_ex->messageBuffer, packet_ptr, num_data);
                do_recv(key, roomID);
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
                std::lock_guard<std::mutex> lock_guard(GameRooms_lock);
                if (GameRooms[roomID].m_pMap == NULL) break;
                if (GameRooms[roomID].m_pMap->game_start == false) break;
                map_block_set* p = reinterpret_cast<map_block_set*>(over_ex->messageBuffer);
                GameRooms[roomID].m_pMap->Set_map();
                
                game_end_packet ep;
                ep.roomid = key;
                ep.size = sizeof(ep);
                ep.type = EventType::game_end;
                m_pTimer->push_event(ep.roomid, OE_gEvent, (MAP_BREAK_TIME*9), reinterpret_cast<char*>(&ep));
                //delete over_ex;
                break;
            }
            case EventType::Cloud_move: {
                std::lock_guard<std::mutex> lock_guard(GameRooms_lock);
                if (GameRooms[roomID].m_pMap == NULL) break;
                if (GameRooms[roomID].m_pMap->game_start == false) break;
                GameRooms[roomID].m_pMap->ismove = true;
                cloud_move_packet* p = reinterpret_cast<cloud_move_packet*>(over_ex->messageBuffer);
                send_cloud_move_packet(p->x, p->z, p->roomid);
                //printf("room: %d cloud x: %f | y: %f\n\n", p->roomid, p->x, p->z);
                GameRooms[roomID].m_pMap->cloud_move();
                delete over_ex;
                break;
            }
            case EventType::Mon_move_to_player: {
                if (m_pBot->monsterRun[roomID] == false) break;
                mon_move_to_player_event* e = reinterpret_cast<mon_move_to_player_event*>(over_ex->messageBuffer);
                //m_pBot->CheckTarget(e->roomid);
                m_pBot->CheckBehavior(e->roomid);
                m_pBot->RunBot(e->roomid);
                delete over_ex;
                break;
            }
            case EventType::Mon_attack_cooltime: {
                if (m_pBot->monsterRun[roomID] == false) break;
                mon_attack_cooltime_event* e = reinterpret_cast<mon_attack_cooltime_event*>(over_ex->messageBuffer);
                m_pBot->monsters[e->roomid][e->key].CanAttack = TRUE;
                delete over_ex;
                break;
            }
            case EventType::MapBreak: {
                std::lock_guard<std::mutex> lock_guard(GameRooms_lock);
                if (GameRooms[roomID].m_pMap == NULL) break;
                if (GameRooms[roomID].m_pMap->game_start == false) break;
                Mapbreak_event* p = reinterpret_cast<Mapbreak_event*>(over_ex->messageBuffer);
                GameRooms[roomID].m_pMap->Map_collapse();

                Mapbreak_event e;
                e.roomid = key;
                e.size = sizeof(e);
                e.type = EventType::MapBreak;
                m_pTimer->push_event(key, OE_gEvent, MAP_BREAK_TIME, reinterpret_cast<char*>(&e));
                delete over_ex;
                break;
            }
            case EventType::Mon_respawn: {
                if (m_pBot->monsterRun[roomID] == false) break;
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
                delete over_ex;
                break;
            }
            case EventType::game_end: {
                //std::lock_guard<std::mutex> lock_guard(GameRooms_lock);
                if (GameRooms[roomID].m_pMap == NULL) break;
                if (GameRooms[roomID].m_pMap->game_start == false) break;
                game_end_event* e = reinterpret_cast<game_end_event*>(over_ex->messageBuffer);
                int roomID = e->roomid;
                printf("room: %d gameover\n", roomID);
                game_end(roomID);
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

#ifdef Run_Lobby
    ConnectLobby();
#endif

    for (auto& s : sessions) {
        s.init();
    }

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

    delete m_pTimer;
    CloseHandle(hcp);
}