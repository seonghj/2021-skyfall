﻿#pragma once
#pragma warning(disable : 4996)
#include "Server.h"

//#define Run_DB
//#define Run_Lobby

void SESSION::init() 
{
    ZeroMemory(&over.overlapped, sizeof(over.overlapped));

    roomID = INVALUED_ID;
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

    state = ObjectState::Death;
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

    over.dataBuffer.len = BUFSIZE;
    over.dataBuffer.buf = over.messageBuffer;
    over.roomID = INVALUED_ID;
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

int Server::CreateRoom(int* key, char* name)
{
    int cnt = 0;

    while (true) {
        if (cnt == MAX_ROOM) return 1;
        GameRooms[cnt].r_lock.lock();
        if (FALSE == GameRooms[cnt].isMade) {
            GameRooms[cnt].r_lock.unlock();
            break;
        }
        GameRooms[cnt].r_lock.unlock();
        cnt++;
    }

    for (auto& g : GameRooms) {
        if (strcmp(g.name, name) == 0) {
            return 2;
        }
    }

    for (int i = 0; i < MAX_PLAYER; ++i)
        GameRooms[cnt].pkeys[i] = INVALIDID;

    strcpy_s(GameRooms[cnt].name, name);

    GameRooms[cnt].m_pMap = new Map;
    GameRooms[cnt].m_pMap->SetNum(*key);

    m_pBot->monsters.emplace(cnt, std::array<Monster, MAX_MONSTER>{});
    m_pBot->Init(cnt);

    GameRooms[cnt].isMade = true;
    GameRooms[cnt].CanJoin = true;
    GameRooms[cnt].TotalPlayer = 0;
    *key = cnt;

    return 0;
}

void Server::Disconnect(int key)
{
    printf("client_end: IP =%s, port=%d Lobby key = %d\n",
        inet_ntoa(sessions[key].clientaddr.sin_addr)
        , ntohs(sessions[key].clientaddr.sin_port), key);
    closesocket(sessions[key].sock);
    int roomid = sessions[key].roomID;

    sessions[key].connected = FALSE;
    sessions[key].key = INVALIDID;
    if (sessions[key].roomID != INVALIDID) {
        if (sessions[key].InGamekey == INVALIDID) return;
        GameRooms[sessions[key].roomID].pkeys[sessions[key].InGamekey] = INVALIDID;
        sessions[key].roomID = INVALIDID;
        GameRooms[roomid].r_lock.lock();
        if (GameRooms[roomid].isMade == true) {
            bool deleteRoom = false;
            for (int i = 0; i < MAX_PLAYER; i++) {
                if (GameRooms[roomid].pkeys[i] == -1)
                    deleteRoom = true;
                else {
                    deleteRoom = false;
                    break;
                }
            }
            if (deleteRoom == true) {
                GameRooms[roomid].CanJoin = false;
                delete GameRooms[roomid].m_pMap;
                ZeroMemory(GameRooms[roomid].name, sizeof(GameRooms[roomid].name));
                GameRooms[roomid].isMade = false;
                m_pBot->monsters_lock.lock();
                m_pBot->monsters.erase(roomid);
                m_pBot->monsterRun.erase(roomid);
                m_pBot->monsters_lock.unlock();

                printf("delete Room: %d\n", roomid);
            }
        }
        GameRooms[roomid].r_lock.unlock();
    }

#ifdef Run_DB
    if(m_pDB->isRun)
        m_pDB->Logout_player(sessions[key].id);
#endif
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
            printf("key: %d room: %d recv error: %d / packet: %d\n", key, roomID, err_no
                , over->dataBuffer.buf[1]);
            Disconnect(key);
        }
    }
}

void Server::send_packet(int to, char* packet)
{
    if (SC_NONE >= packet[1] || packet[1] >= CS_NONE) return;
    SOCKET client_s = sessions[to].sock;

    OVER_EX* over = new OVER_EX;
    ZeroMemory(over, sizeof(OVER_EX));
    over->dataBuffer.buf = packet;
    over->dataBuffer.len = packet[0];

    memcpy(over->messageBuffer, packet, packet[0]);
    over->type = OE_send;
    ZeroMemory(&over->overlapped, sizeof(over->overlapped));

    if (WSASend(client_s, &over->dataBuffer, 1, NULL,
        0, &(over->overlapped), NULL)) {
        int err_no = WSAGetLastError();
        if (err_no != WSA_IO_PENDING){
            printf("to: %d packet: %d send error: %d\n", to, packet[1], err_no);
            Disconnect(to);
        }
    }
}

void Server::send_player_InGamekey_packet(int key, int roomID)
{
    player_key_packet p;

    p.key = sessions[key].InGamekey;
    p.size = sizeof(player_key_packet);
    p.type = PacketType::SC_player_InGamekey;
    p.roomid = roomID;
    send_packet(key, reinterpret_cast<char*>(&p));
}

void Server::send_Lobby_key_packet(int key)
{
    player_key_packet p;

    p.key = key;
    p.size = sizeof(player_key_packet);
    p.type = PacketType::SC_player_Lobbykey;
    p.roomid = INVALIDID;

    send_packet(key, reinterpret_cast<char*>(&p));
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
    send_packet(key, reinterpret_cast<char*>(&p));
}

void Server::send_room_list_packet(int key)
{
    for (int i = 0; i < MAX_ROOM; ++i) {
        if (sessions[key].playing == true) break;
        if (GameRooms[i].isMade == false) continue;
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
        send_packet(key, reinterpret_cast<char*>(&p));
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
    send_packet(key, reinterpret_cast<char*>(&p));
}

void Server::send_player_loginFail_packet(int key, int roomID)
{
    player_loginFail_packet p;
    p.key = key;
    p.size = sizeof(player_loginFail_packet);
    p.type = PacketType::SC_player_loginFail;
    p.roomid = roomID;
    send_packet(key, reinterpret_cast<char*>(&p));
}

void Server::send_start_packet(int to, int roomID)
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dis(100, MAP_SIZE - 100);
    std::uniform_int_distribution<int> weapondis(1, 4);

    game_start_packet p;
    p.type = PacketType::SC_start_ok;
    p.size = sizeof(p);
    p.key = to;
    p.roomid = roomID;
    p.ingamekey = sessions[to].InGamekey;
    p.pos.x = dis(gen);
    p.pos.y = 500;
    p.pos.z = dis(gen);
    p.leftplayer = GameRooms[roomID].TotalPlayer;
    sessions[to].f3Position = p.pos;

    if (sessions[to].using_weapon == PT_BASIC) {
        p.weaponType = (PlayerType)weapondis(gen);
        sessions[to].using_weapon = p.weaponType;
    }
    else
        p.weaponType = sessions[to].using_weapon;

    send_packet(to, reinterpret_cast<char*>(&p));

    if (GameRooms[roomID].m_pMap->game_start == false) {
        GameRooms[roomID].m_pMap->game_start = true;
        GameRooms[roomID].m_pMap->init_Map(this, m_pTimer);
        printf("Room %d Game start\n", roomID);
        m_pBot->StartTime[roomID] = GameRooms[roomID].m_pMap->StartTime;
        Mapbreak_event e;
        e.roomid = roomID;
        e.size = sizeof(e);
        e.type = EventType::MapBreak;
        e.GameStartTime = GameRooms[roomID].m_pMap->StartTime;
        m_pTimer->push_event(roomID, OE_gEvent, MAP_BREAK_TIME, reinterpret_cast<char*>(&e));
    }

    if (m_pBot->monsterRun[roomID] == false) {
        m_pBot->monsterRun[roomID] = true;
        m_pBot->RunBot(roomID);
    }
}

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

    send_packet(to, reinterpret_cast<char*>(&p));
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
            send_packet(sessions[k].key, reinterpret_cast<char*>(&p));
    }
    Disconnect(GameRooms[roomID].pkeys[ingamekey]);
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
            send_packet(sessions[k].key, reinterpret_cast<char*>(&p));
    }
    Disconnect(GameRooms[roomID].pkeys[ingamekey]);
}

void Server::send_packet_to_players(int ingamekey, char* buf, int roomID)
{
    if (roomID > 999) {
        Packet* p = reinterpret_cast<Packet*>(buf);
        std::cout << "packet: " << p->size << " | " << "room: " << roomID << std::endl;
    }
    if (ingamekey >= MAX_PLAYER || ingamekey < 0) return;
    for (int k : GameRooms[roomID].pkeys) {
        if (k == INVALIDID) continue;
        if (sessions[k].connected == FALSE) continue;
        if (sessions[k].playing == FALSE) continue;
        if (in_VisualField(sessions[k]
            , sessions[GameRooms[roomID].pkeys[ingamekey]], roomID)) {
            send_packet(k, buf);
        }
    }
}

void Server::send_packet_to_allplayers(int roomID, char* buf)
{
    if (MAX_ROOM <= roomID) return;
    for (int k : GameRooms[roomID].pkeys) {
        if (k == INVALIDID) continue;
        if (sessions[k].connected == FALSE) continue;
        if (sessions[k].playing == FALSE) continue;
        send_packet(k, buf);
    }
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

    send_packet(to, reinterpret_cast<char*>(&p));
}

void Server::send_remove_monster(int key, int roomID, int to)
{
    mon_remove_packet p;
    p.size = sizeof(mon_remove_packet);
    p.type = PacketType::SC_monster_remove;
    p.key = key;
    p.roomid = roomID;

    send_packet(to, reinterpret_cast<char*>(&p));
}

void Server::send_monster_pos(Monster& mon, XMFLOAT3 direction, int target)
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
    p.MonsterType = mon.type;
    p.target = target;

    for (int k : GameRooms[roomID].pkeys) {
        if (k == INVALIDID) continue;
        if (sessions[k].connected == FALSE) continue;
        if (sessions[k].playing == FALSE) continue;
        if (in_VisualField(mon, sessions[k], roomID)) {
            send_packet(k, reinterpret_cast<char*>(&p));
        }
    }
}

void Server::send_monster_move(Monster& mon, XMFLOAT3 direction, int target)
{
    if (mon.key >= MAX_MONSTER) return;
    int roomID = mon.roomID;

    mon_move_packet p;
    p.size = sizeof(mon_move_packet);
    p.type = PacketType::SC_monster_move;
    p.key = mon.key;
    p.roomid = mon.roomID.load();
    p.Position = mon.f3Position.load();
    p.direction = direction;
    p.degree = mon.m_fRoll.load();
    p.MoveType = 0;
    p.state = 0;
    p.MonsterType = mon.type;
    p.target = target;

    for (int k : GameRooms[roomID].pkeys) {
        if (k == INVALIDID) continue;
        if (sessions[k].connected == FALSE) continue;
        if (sessions[k].playing == FALSE) continue;
        if (in_VisualField(mon, sessions[k], roomID)) {
            send_packet(k, reinterpret_cast<char*>(&p));
        }
    }
}

void Server::send_monster_attack(Monster& mon, XMFLOAT3 direction, int target)
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
        p.attack_dis = Attack_Distance_Dragon;
    else if (mon.type == MonsterType::Wolf)
        p.attack_dis = Attack_Distance_Wolf;
    else if (mon.type == MonsterType::Metalon)
        p.attack_dis = Attack_Distance_Metalon;

    for (int k : GameRooms[roomID].pkeys) {
        if (k <= INVALIDID) continue;
        if (sessions[k].connected == FALSE) continue;
        if (sessions[k].playing == FALSE) continue;
        if (in_VisualField(mon, sessions[k], roomID)) {
            send_packet(k, reinterpret_cast<char*>(&p));
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

    for (int k : GameRooms[roomID].pkeys) {
        if (k <= INVALIDID) continue;
        if (sessions[k].connected == FALSE) continue;
        if (sessions[k].playing == FALSE) continue;
        if (in_VisualField(m_pBot->monsters[roomID][key], sessions[k], roomID)) {
            send_packet(k, reinterpret_cast<char*>(&p));
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

    send_packet(key, reinterpret_cast<char*>(&p));
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

    send_packet(to, reinterpret_cast<char*>(&p));
}


void Server::send_game_end_packet(int key, int roomID)
{
    game_end_packet p;
    p.key = key;
    p.size = sizeof(p);
    p.type = PacketType::SC_game_end;
    p.roomid = roomID;
    send_packet(key, reinterpret_cast<char*>(&p));
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

    sessions[p.key].playing == FALSE;
    GameRooms[roomID].pkeys[ingamekey] = INVALIDID;

    printf("player %d dead\n", p.key);

#ifdef Run_DB
    if (m_pDB->isRun)
        m_pDB->Send_player_record(sessions[GameRooms[roomID].pkeys[ingamekey]]
            , 0, GameRooms[roomID].TotalPlayer);
#endif 
}

void Server::game_end(int roomnum)
{
    if (GameRooms[roomnum].isMade == false) return;
    GameRooms[roomnum].r_lock.lock();
    GameRooms[roomnum].isMade = false;
    GameRooms[roomnum].CanJoin = false;
    GameRooms[roomnum].TotalPlayer = 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    for (int key : GameRooms[roomnum].pkeys) {
        if (key == INVALIDID) continue;

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

    GameRooms[roomnum].master = INVALIDID;
    GameRooms[roomnum].r_lock.unlock();

    m_pBot->monsters_lock.lock();
    m_pBot->monsters.erase(roomnum);
    m_pBot->monsterRun.erase(roomnum);
    m_pBot->StartTime.erase(roomnum);
    m_pBot->monsters_lock.unlock();

    printf("Room %d Game End\n", roomnum);
}

void Server::Delete_room(int roomID)
{
    GameRooms[roomID].r_lock.lock();
    GameRooms[roomID].CanJoin = false;
    GameRooms[roomID].TotalPlayer = 0;
    delete GameRooms[roomID].m_pMap;
    ZeroMemory(GameRooms[roomID].name, sizeof(GameRooms[roomID].name));
    for (auto& key : GameRooms[roomID].pkeys)
        if (key != -1)
            send_room_list_packet(key);
    GameRooms[roomID].isMade = false;
    GameRooms[roomID].r_lock.unlock();

    m_pBot->monsters_lock.lock();
    m_pBot->monsters.erase(roomID);
    m_pBot->monsterRun.erase(roomID);
    m_pBot->StartTime.erase(roomID);
    m_pBot->monsters_lock.unlock();

    printf("delete room %d\n", roomID);
}

void Server::player_go_lobby(int key, int roomID)
{
    if (GameRooms[roomID].isMade == true)
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
    send_packet(key, reinterpret_cast<char*>(&p));
}

bool Server::in_VisualField(SESSION& a, SESSION& b, int roomID)
{
    float value = pow((a.f3Position.load().x - b.f3Position.load().x), 2)
        + pow((a.f3Position.load().z - b.f3Position.load().z), 2);

    if (value < 0) {
        if (sqrt(-value) <= VIEWING_DISTANCE) return true;
    }
    else {
        if (sqrt(value) <= VIEWING_DISTANCE) return true;
    }
    return false;
}

bool Server::in_VisualField(Monster& a, SESSION& b, int roomID)
{
    float value = pow((a.f3Position.load().x - b.f3Position.load().x), 2)
        + pow((a.f3Position.load().z - b.f3Position.load().z), 2);
    if (value < 0) {
        if (sqrt(-value) <= VIEWING_DISTANCE) {
            return true;
        }
    }
    else {
        if (sqrt(value) <= VIEWING_DISTANCE) {
            return true;
        }
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

    if (sessions[key].f3Position.load().x == pos.x 
        && sessions[key].f3Position.load().z == pos.z
        && sessions[key].f3Position.load().y == pos.y) return;

    sessions[key].f3Position = pos;

    std::lock_guard <std::mutex> lg(sessions[key].nm_lock);
    std::unordered_set<int> old_nm;
    std::unordered_set<int> new_nm;

    old_nm = sessions[key].near_monster;

    for (auto& m : m_pBot->monsters[roomID]) {
        if (m.state == 0) continue;
        if (in_VisualField(m, sessions[key], roomID)) {
            new_nm.insert(m.key);
        }
    }

    for (auto m : new_nm) {
        if (m_pBot->monsters[roomID][m].state == 0) continue;
        if (old_nm.count(m) == 0) {
            sessions[key].near_monster.insert(m);
            send_add_monster(m, roomID, key);
        }
    }

    for (auto m : old_nm) {
        if (m_pBot->monsters[roomID][m].state == 0) continue;
        if (new_nm.count(m) == 0) {
            sessions[key].near_monster.erase(m);
        }
    }
}

void Server::process_packet(int key, char* buf, int roomID)
{
    switch (buf[1]) {
    case PacketType::CS_player_Lobbylogin: {
        CS_PlayerLobbylogin(buf);
        break;
    }
    case PacketType::CS_create_account: {
        if (sessions[key].playing == true) break;
        CS_CreateAccount(buf);
        break;
    }
    case PacketType::CS_game_start: {
        game_start_packet* p = reinterpret_cast<game_start_packet*>(buf);
        CS_GameStart(buf);
        break;
    }
    case PacketType::CS_create_room: {
        CS_CreateRoom(buf);
        break;
    }
    case PacketType::CS_room_select: {
        CS_RoomSelect(buf);
        break;
    }
    case PacketType::CS_return_lobby: {
        CS_ReturnLobby(buf);
        break;
    }
    case PacketType::CS_refresh_lobby: {
        CS_RefreshLobby(buf);
        break;
    }
    case PacketType::CS_player_info:{
        player_info_packet* p = reinterpret_cast<player_info_packet*>(buf);
        p->type = SC_player_info;
        break;
    }
    case PacketType::CS_player_pos: {
        CS_PlayerPos(buf);
        break;
    }
    case PacketType::CS_start_pos: {
        CS_StartPos(buf);
        break;
    }
    case PacketType::CS_weapon_swap :{
        CS_StartPos(buf);
        break;
    }
    case PacketType::CS_weapon_select: {
        CS_WeaponSelect(buf);
        break;
    }
    case PacketType::CS_player_move: {
        CS_PlayerMove(buf);
        break;
    }
    case PacketType::CS_player_attack:{
        CS_PlayerAttack(buf);
        break;

    }
    case PacketType::CS_allow_shot: {
        CS_AllowShot(buf);
        break;
    }
    case PacketType::CS_player_stop: {
        CS_PlayerStop(buf);
        break;
    }
    case PacketType::CS_player_getitem: {
        CS_PlayerGetItem(buf);
        break;
    }
    case PacketType::CS_monster_pos: {
        CS_MonsterPos(buf);
        break;
    }
    case PacketType::CS_monster_attack: {
        CS_MonsterAttack(buf);
        break;
    }
    case PacketType::CS_monster_damaged: {
        CS_MonsterDamaged(buf);
        break;
    }
    case PacketType::CS_player_damage: {
        CS_PlayerDamaged(buf);
        break;
    }
    case PacketType::CS_player_dead: {
        CS_PlayerDead(buf);
        break;
    }
    }
}

void Server::ProcessEvent(OVER_EX* over_ex, int roomID, int key)
{
    switch (over_ex->messageBuffer[1]) {
    case EventType::Mapset: {
        Event_MapSet(roomID, over_ex->messageBuffer);
        delete over_ex;
        break;
    }
    case EventType::Cloud_move: {
        Event_CloudMove(roomID, over_ex->messageBuffer);
        delete over_ex;
        break;
    }
    case EventType::Mon_behavior: {
        Event_MonBehavior(roomID, over_ex->messageBuffer);
        delete over_ex;
        break;
    }
    case EventType::Mon_move_to_player: {
        Event_MonMoveToPlayer(roomID, over_ex->messageBuffer);
        delete over_ex;
        break;
    }
    case EventType::Mon_attack_cooltime: {
        Event_MonAttackCooltime(roomID, over_ex->messageBuffer);
        delete over_ex;
        break;
    }
    case EventType::Mon_attack: {
        Event_MonAttack(roomID, over_ex->messageBuffer);
        delete over_ex;
        break;
    }
    case EventType::Mon_stop: {
        Event_MonStop(roomID, over_ex->messageBuffer);
        delete over_ex;
        break;
    }
    case EventType::MapBreak: {
        Event_MapBreak(roomID, over_ex->messageBuffer);
        delete over_ex;
        break;
    }
    case EventType::Mon_respawn: {
        Event_MonRespawn(roomID, over_ex->messageBuffer);
        delete over_ex;
        break;
    }
    case EventType::game_end: {
        Event_GameEnd(roomID, over_ex->messageBuffer);
        delete over_ex;
        break;
    }
    }
}

void Server::WorkerFunc()
{
    DWORD Transferred;
    ULONG key;
    WSAOVERLAPPED* over;

    while (1) {
        BOOL retval = GetQueuedCompletionStatus(hcp, &Transferred,
            (PULONG_PTR)&key, &over, INFINITE);

        OVER_EX* over_ex = reinterpret_cast<OVER_EX*>(over);
        int roomID = over_ex->roomID;

        if (false == retval) {
            display_error("GQCS: ", WSAGetLastError());
            Disconnect(key);
        }
        if (SERVER_ID != key && Transferred == 0) {
            Disconnect(key);
        }

        switch (over_ex->type) {
        case OE_accept: {
            int client_key = SetLobbyKey();
            if (client_key != -1) {
                sessions[client_key].sock = over_ex->csocket;
                sessions[client_key].over.type = OE_recv;
                sessions[client_key].prev_size = 0;
                sessions[client_key].init();
                sessions[client_key].connected = TRUE;
                sessions[client_key].key = client_key;
                CreateIoCompletionPort((HANDLE)sessions[client_key].sock, hcp, client_key, 0);
                send_Lobby_key_packet(client_key);
                do_recv(client_key, INVALUED_ID);
            }
            else
                closesocket(over_ex->csocket);

            ZeroMemory(&over_ex->overlapped, sizeof(over_ex->overlapped));
            SOCKET c_socket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
            over_ex->csocket = c_socket;
            BOOL ret = AcceptEx(listenSocket, c_socket
                , over_ex->messageBuffer
                , 0, 32, 32, NULL, &over_ex->overlapped);
            if (false == ret) {
                int err_num = WSAGetLastError();
                if (err_num != WSA_IO_PENDING)
                {
                    display_error("AcceptEx_error ", err_num);
                    printf("%d\n", err_num);
                }
            }
            /*getpeername(sessions[client_key].sock, (SOCKADDR*)&sessions[client_key].clientaddr
                , &sessions[client_key].addrlen);*/
            break;
        }
        case OE_recv: {
            char* packet_ptr = over_ex->messageBuffer;
            int required_data = Transferred + sessions[key].prev_size;
            int packet_size = packet_ptr[0];
            while (required_data >= packet_size) {
                if (required_data >= BUFSIZE) break;
                if (packet_size <= 0) break;
                process_packet(key, packet_ptr, roomID);
                required_data -= packet_size;
                packet_ptr += packet_size;
                packet_size = packet_ptr[0];
            }
            packet_size = 0;
            sessions[key].prev_size = 0;
            if (0 != required_data)
                memcpy(over_ex->messageBuffer, packet_ptr, required_data);
            do_recv(key, roomID);
            break;
        }
        case OE_send: {
            delete over_ex;
            break;
        }
        case OE_gEvent: {
            if (FALSE == retval)
                continue;
            ProcessEvent(over_ex, roomID, key);
            break;
        }
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

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    if (hcp == NULL) return 0;

    listenSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), hcp, 0, 0);
    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(GAMESERVERPORT);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    ::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
    listen(listenSocket, SOMAXCONN);

    OVER_EX accept_over;
    accept_over.type = OE_accept;
    memset(&accept_over.overlapped, 0, sizeof(accept_over.overlapped));
    SOCKET c_socket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    accept_over.csocket = c_socket;
    BOOL ret = AcceptEx(listenSocket, c_socket, accept_over.messageBuffer
        , SERVER_ID, 32, 32, NULL, &accept_over.overlapped);
    if (false == ret) {
        int err_num = WSAGetLastError();
        if (err_num != WSA_IO_PENDING)
            display_error("AcceptEx_error", err_num);
    }
    printf("ready\n");

    SYSTEM_INFO si;
    GetSystemInfo(&si);

    for (int i = 0; i <(int)si.dwNumberOfProcessors; i++)
        working_threads.emplace_back(std::thread(&Server::WorkerFunc, this));

    timer_thread = std::thread(&Timer::init, m_pTimer, Gethcp());

    return 1;
}

void Server::Thread_join()
{
    timer_thread.join();

    for (auto& t : working_threads)
        t.join();

    delete m_pTimer;
    CloseHandle(hcp);
}


void Server::CS_PlayerLobbylogin(char* buf) 
{
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
}

void Server::CS_CreateAccount(char* buf)
{
    create_account_packet* p = reinterpret_cast<create_account_packet*>(buf);
    int key = p->key;
    bool b = true;
    bool is_Login = false;
#ifdef Run_DB
    if (m_pDB->isRun) {
        b = m_pDB->Insert_ID(p->id, p->pw);

        if (!b)
            p->canmake = false;
        else
            p->canmake = true;
        p->type = SC_create_account;
        send_packet(key, reinterpret_cast<char*>(p), INVALIDID);
    }
#endif
}

void Server::CS_GameStart(char* buf)
{
    game_start_packet* p = reinterpret_cast<game_start_packet*>(buf);
    if (p->key == GameRooms[p->roomid].master) {

        bool ready = false;

        GameRooms[p->roomid].CanJoin = false;
        for (int client_key : GameRooms[p->roomid].pkeys) {
            if (client_key == INVALIDID) continue;
            if (sessions[client_key].playing == true) continue;

            sessions[client_key].roomID = p->roomid;
            sessions[client_key].over.roomID = p->roomid;

            sessions[client_key].state = ObjectState::Alive;

            send_start_packet(client_key, p->roomid);

            sessions[client_key].isready = true;
            sessions[client_key].playing = true;

            sessions[client_key].hp = 100;
            sessions[client_key].def = 0;
            sessions[client_key].lv = 0;
            sessions[client_key].att = 10;
            sessions[client_key].speed = 20;
            sessions[client_key].proficiency = 0.0f;
            sessions[client_key].near_monster.clear();

            for (int k : GameRooms[p->roomid].pkeys) {
                if (k == INVALIDID) continue;
                if ((TRUE == sessions[k].connected) && (k != client_key)) {
                    send_add_player_packet(client_key, k, p->roomid);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }

            for (int k : GameRooms[p->roomid].pkeys) {
                if (k == INVALIDID) continue;
                if ((TRUE == sessions[k].connected) && (k != client_key)) {
                    send_add_player_packet(k, client_key, p->roomid);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }
    }
}

void Server::CS_CreateRoom(char* buf)
{
    room_create_packet* p = reinterpret_cast<room_create_packet*>(buf);
    int key = p->key;
    if (sizeof(p->name) <= 0) return;
    GameRooms_lock.lock();
    if (GameRooms[MAX_ROOM - 1].isMade == true) {
        GameRooms_lock.unlock();
        return;
    }
    int roomid = 0;
    // 0 = 정상 1 = 꽉참 2 = 방이름 같음
    int error = CreateRoom(&roomid, p->name);
    if (error == ErrorCode::FullRoom) {
        GameRooms_lock.unlock();
        return;
    }
    if (error == ErrorCode::AlreayExistRoom) {
        room_list_packet r;
        r.key = key;
        r.size = sizeof(r);
        r.type = PacketType::SC_room_list;
        r.roomid = INVALIDID;
        r.idx = INVALIDID;
        send_packet(key, reinterpret_cast<char*>(&r));
        GameRooms_lock.unlock();
        return;
    }

    GameRooms[roomid].master = key;
    ++GameRooms[roomid].TotalPlayer;

    int nkey = SetInGameKey(roomid);
    if (nkey == INVALIDID) {
        Disconnect(key);
        GameRooms_lock.unlock();
        return;
    }
    GameRooms[roomid].pkeys[nkey] = key;
    sessions[key].InGamekey = nkey;
    sessions[key].roomID = roomid;
    sessions[key].over.roomID = roomid;
    room_select_packet s;
    s.key = nkey;
    s.room = roomid;
    s.roomid = roomid;
    s.size = sizeof(s);
    s.type = SC_select_room;
    s.ingamekey = nkey;
    s.isMaster = true;
    send_packet(p->key, reinterpret_cast<char*>(&s));
    GameRooms_lock.unlock();
}

void Server::CS_RoomSelect(char* buf)
{
    room_select_packet* p = reinterpret_cast<room_select_packet*>(buf);
    GameRooms_lock.lock();
    if (p->room < 0) { GameRooms_lock.unlock(); return; }
    if (GameRooms[p->room].isMade == false) { GameRooms_lock.unlock(); return; }
    if (GameRooms[p->room].CanJoin == false) { GameRooms_lock.unlock(); return; }
    if (GameRooms[p->room].TotalPlayer >= MAX_PLAYER) { GameRooms_lock.unlock(); return; }

    p->type = SC_select_room;
    p->isMaster = false;
    int nkey = SetInGameKey(p->room);
    if (nkey == INVALIDID) {
        Disconnect(p->key);
        GameRooms_lock.unlock();
        return;
    }
    GameRooms[p->room].pkeys[nkey] = p->key;
    sessions[p->key].InGamekey = nkey;
    sessions[p->key].roomID = p->room;
    sessions[p->key].over.roomID = p->room;
    p->ingamekey = nkey;
    ++GameRooms[p->room].TotalPlayer;
    GameRooms_lock.unlock();
    send_packet(p->key, reinterpret_cast<char*>(p));
}

void Server::CS_ReturnLobby(char* buf)
{
    return_lobby_packet* p = reinterpret_cast<return_lobby_packet*>(buf);
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
}

void Server::CS_RefreshLobby(char* buf)
{
    refresh_lobby_packet* p = reinterpret_cast<refresh_lobby_packet*>(buf);
    send_room_list_packet(p->key);
}

void Server::CS_PlayerInfo(char* buf)
{

}

void Server::CS_PlayerPos(char* buf)
{
    player_pos_packet* p = reinterpret_cast<player_pos_packet*>(buf);
    if (0 > p->ingamekey || p->ingamekey >= 20) return;
    player_move(p->key, p->roomid, p->Position, p->dx, p->dy);
    player_pos_packet packet;
    packet.type = SC_player_pos;
    packet.key = p->key;
    packet.ingamekey = p->ingamekey;
    packet.roomid = p->roomid;
    packet.state = p->state;
    packet.size = sizeof(player_pos_packet);
    packet.Position = sessions[p->key].f3Position;
    packet.dx = sessions[p->key].m_fPitch;
    packet.dy = sessions[p->key].m_fYaw;
    packet.MoveType = p->MoveType;
    packet.dir = p->dir;
    packet.playertype = sessions[p->key].using_weapon;

    send_packet_to_players(p->ingamekey, reinterpret_cast<char*>(&packet), p->roomid);

    if (sessions[p->key].f3Position.load().y <= -10) {
        sessions[p->key].state = ObjectState::Death;
        send_player_dead_packet(p->ingamekey, p->roomid);
    }
}

void Server::CS_StartPos(char* buf)
{
    player_start_pos* p = reinterpret_cast<player_start_pos*>(buf);
    sessions[GameRooms[p->roomid].pkeys[p->key]].f3Position = p->Position;
    send_packet(p->key, reinterpret_cast<char*>(p));
}

void Server::CS_WeaponSwap(char* buf)
{
    Weapon_swap_packet* p = reinterpret_cast<Weapon_swap_packet*>(buf);
    if (0 > p->ingamekey || p->ingamekey >= 20) return;
    p->type = SC_weapon_swap;
    if (p->weapon != 0)
        sessions[p->key].using_weapon = p->weapon;
    send_packet_to_allplayers(p->roomid, reinterpret_cast<char*>(p));
}

void Server::CS_WeaponSelect(char* buf)
{
    Weapon_select_packet* p = reinterpret_cast<Weapon_select_packet*>(buf);
    p->type = SC_weapon_select;
    if (p->weapon != 0)
        sessions[p->key].using_weapon = p->weapon;
}

void Server::CS_PlayerMove(char* buf)
{
    player_move_packet* p = reinterpret_cast<player_move_packet*>(buf);
    if (0 > p->ingamekey || p->ingamekey >= 20) return;
    p->type = SC_player_move;
    sessions[p->ingamekey].m_fPitch.store(p->dx);
    sessions[p->ingamekey].m_fYaw.store(p->dy);
    send_packet_to_players(p->ingamekey, reinterpret_cast<char*>(p), p->roomid);
}

void Server::CS_PlayerAttack(char* buf)
{
    player_attack_packet* p = reinterpret_cast<player_attack_packet*>(buf);
    if (0 > p->ingamekey || p->ingamekey >= 20) return;
    p->type = SC_player_attack;
    send_packet_to_players(p->ingamekey, reinterpret_cast<char*>(p), p->roomid);
}

void Server::CS_PlayerStop(char* buf)
{
    player_stop_packet* p = reinterpret_cast<player_stop_packet*>(buf);
    if (0 > p->ingamekey || p->ingamekey >= 20) return;
    p->type = SC_player_stop;
    sessions[p->key].f3Position = p->Position;
    p->playertype = sessions[p->key].using_weapon;
    send_packet_to_allplayers(p->roomid, reinterpret_cast<char*>(p));
}

void Server::CS_AllowShot(char* buf)
{
    player_shot_packet* p = reinterpret_cast<player_shot_packet*>(buf);
    if (0 > p->ingamekey || p->ingamekey >= 20) return;
    p->type = SC_allow_shot;
    send_packet_to_players(p->ingamekey, reinterpret_cast<char*>(p), p->roomid);
}

void Server::CS_PlayerGetItem(char* buf)
{
    player_getitem_packet* p = reinterpret_cast<player_getitem_packet*>(buf);
    int key = p->key;
    int room = p->roomid;
    for (auto& i : sessions[GameRooms[p->roomid].pkeys[p->key]].inventory) {
        if (i == 0) {
            i = p->item;
            return;
        }
    }
    p->type = SC_player_getitem;
    send_packet_to_players(p->key, reinterpret_cast<char*>(p), p->roomid);
}

void Server::CS_MonsterPos(char* buf)
{
    mon_pos_packet* p = reinterpret_cast<mon_pos_packet*>(buf);
    if (m_pBot->monsters[p->roomid][p->key].recv_pos == TRUE) return;
    if (_isnanf(p->Position.x) || _isnanf(p->Position.y) || _isnanf(p->Position.z)) {
        m_pBot->monsters[p->roomid][p->key].recv_pos = TRUE;
        return;
    }
    m_pBot->monsters[p->roomid][p->key].recv_pos = TRUE;
    m_pBot->monsters[p->roomid][p->key].SetPosition(p->Position.x, p->Position.y, p->Position.z);
}

void Server::CS_MonsterAttack(char* buf)
{
    mon_attack_packet* p = reinterpret_cast<mon_attack_packet*>(buf);

    if (sessions[GameRooms[p->roomid].pkeys[p->target]].hp > 0) {
        sessions[GameRooms[p->roomid].pkeys[p->target]].hp
            = p->PlayerLeftHp;

        if (sessions[GameRooms[p->roomid].pkeys[p->target]].hp.load() <= 0) {
            sessions[GameRooms[p->roomid].pkeys[p->target]].state = ObjectState::Death;
            send_player_dead_packet(p->target, p->roomid);
            --GameRooms[p->roomid].TotalPlayer;
            if (GameRooms[p->roomid].TotalPlayer <= 1)
                game_end(p->roomid);
        }
    }
}

void Server::CS_MonsterDamaged(char* buf)
{
    mon_damaged_packet* p = reinterpret_cast<mon_damaged_packet*>(buf);
    if (0 > p->ingamekey || p->ingamekey >= 20) return;
    if (0 > p->target || p->target >= 20) return;
    if (GameRooms[p->roomid].pkeys[p->target] == INVALIDID) return;
    int key = p->key;
    int target = p->target;
    int roomID = p->roomid;
    if (m_pBot->monsters[p->roomid][target].state == 0) return;

    p->type = SC_monster_damaged;
    p->damage = (sessions[key].att * (1.f + p->nAttack / 4.f))
        * (100 - m_pBot->monsters[p->roomid][target].def) / 100;
    sessions[key].AddProficiency();
    m_pBot->monsters[p->roomid][target].hp = m_pBot->monsters[p->roomid][target].hp - p->damage;
    p->leftHp = m_pBot->monsters[p->roomid][target].hp;

    m_pBot->monsters[p->roomid][target].TraceTarget = key;

    send_packet_to_players(p->ingamekey, reinterpret_cast<char*>(p), p->roomid);

    if (m_pBot->monsters[p->roomid][target].hp <= 0) {
        m_pBot->monsters[p->roomid][target].state = 0;

        mon_respawn_event e;
        e.key = target;
        e.roomid = p->roomid;
        e.size = sizeof(e);
        e.type = EventType::Mon_respawn;
        e.GameStartTime = GameRooms[roomID].m_pMap->StartTime;
        m_pTimer->push_event(roomID, OE_gEvent, MON_SPAWN_TIME, reinterpret_cast<char*>(&e));
    }
}

void Server::CS_PlayerDamaged(char* buf)
{
    player_damage_packet* p = reinterpret_cast<player_damage_packet*>(buf);
    if (0 > p->ingamekey || p->ingamekey >= 20) return;
    if (0 > p->target || p->target >= 20) return;
    if (GameRooms[p->roomid].pkeys[p->target] == INVALIDID) return;
    int key = p->key;
    int ingamekey = p->ingamekey;
    int target = p->target;
    int roomID = p->roomid;
    if (sessions[key].roomID
        != sessions[GameRooms[p->roomid].pkeys[target]].roomID) return;

    p->type = SC_player_damage;
    p->damage = (sessions[key].GetAtkDamage()
        * (1.f + p->nAttack / 4.f))
        * (100 - sessions[GameRooms[p->roomid].pkeys[target]].def) / 100;

    if (sessions[GameRooms[p->roomid].pkeys[target]].hp > 0) {
        sessions[GameRooms[p->roomid].pkeys[target]].hp
            = sessions[GameRooms[p->roomid].pkeys[target]].hp - p->damage;
        p->leftHp = sessions[GameRooms[p->roomid].pkeys[target]].hp;
        sessions[key].AddProficiency();
        send_packet_to_players(ingamekey, reinterpret_cast<char*>(p), p->roomid);
    }

    if (sessions[GameRooms[p->roomid].pkeys[target]].hp.load() <= 0) {
        sessions[GameRooms[p->roomid].pkeys[target]].state = ObjectState::Death;
        send_player_dead_packet(target, roomID);
        --GameRooms[p->roomid].TotalPlayer;
        if (GameRooms[p->roomid].TotalPlayer <= 1)
            game_end(p->roomid);
    }
}

void Server::CS_PlayerDead(char* buf)
{
    player_dead_packet* p = reinterpret_cast<player_dead_packet*>(buf);
    if (0 > p->ingamekey || p->ingamekey >= 20) return;
    p->type = PacketType::SC_player_dead;
    sessions[p->key].state = ObjectState::Death;
    send_packet_to_allplayers(p->roomid, reinterpret_cast<char*>(p));
    --GameRooms[p->roomid].TotalPlayer;
    if (GameRooms[p->roomid].TotalPlayer <= 1)
        game_end(p->roomid);
}

void Server::Event_MapSet(int roomID, char* buf)
{
    std::lock_guard<std::mutex> lock_guard(GameRooms_lock);
    if (GameRooms[roomID].m_pMap == NULL) return;
    if (GameRooms[roomID].m_pMap->game_start == false) return;
    map_block_set* p = reinterpret_cast<map_block_set*>(buf);
    if (p->GameStartTime != GameRooms[roomID].m_pMap->StartTime) return;
    GameRooms[roomID].m_pMap->Set_map();
}

void Server::Event_CloudMove(int roomID, char* buf)
{
    std::lock_guard<std::mutex> lock_guard(GameRooms_lock);
    if (GameRooms[roomID].m_pMap == NULL) return;
    if (GameRooms[roomID].m_pMap->game_start == false) return;
    GameRooms[roomID].m_pMap->ismove = true;
    cloud_move_packet* p = reinterpret_cast<cloud_move_packet*>(buf);
    if (p->GameStartTime != GameRooms[roomID].m_pMap->StartTime) return;
    send_cloud_move_packet(p->x, p->z, p->roomid);
    //printf("room: %d cloud x: %f | y: %f\n\n", p->roomid, p->x, p->z);
    GameRooms[roomID].m_pMap->cloud_move();
}

void Server::Event_MonBehavior(int roomID, char* buf)
{
    mon_behavior_event* e = reinterpret_cast<mon_behavior_event*>(buf);
    if (m_pBot->monsterRun[roomID] == false) return;
    if (e->GameStartTime != m_pBot->StartTime[roomID]) return;
    //m_pBot->CheckTarget(e->roomid);
    m_pBot->CheckBehavior(e->roomid);
}

void Server::Event_MonMoveToPlayer(int roomID, char* buf)
{
    mon_move_event* e = reinterpret_cast<mon_move_event*>(buf);
    if (m_pBot->monsterRun[roomID] == false) return;
    if (e->GameStartTime != m_pBot->StartTime[roomID]) return;
    m_pBot->monsters[e->roomid][e->key].Move(e->subtract
        , m_pBot->monsters[e->roomid][e->key].speed);
    send_monster_pos(m_pBot->monsters[e->roomid][e->key]
        , e->subtract, e->target);
}

void Server::Event_MonAttack(int roomID, char* buf)
{
    mon_attack_event* e = reinterpret_cast<mon_attack_event*>(buf);
    if (m_pBot->monsterRun[roomID] == false) return;
    if (e->GameStartTime != m_pBot->StartTime[roomID]) return;
    send_monster_attack(m_pBot->monsters[e->roomid][e->key]
        , e->direction, e->target);
}

void Server::Event_MonAttackCooltime(int roomID, char* buf)
{
    if (m_pBot->monsterRun[roomID] == false) return;
    mon_attack_cooltime_event* e = reinterpret_cast<mon_attack_cooltime_event*>(buf);
    if (e->GameStartTime != m_pBot->StartTime[roomID]) return;
    m_pBot->monsters[e->roomid][e->key].CanAttack = TRUE;
}

void Server::Event_MonStop(int roomID, char* buf)
{
    if (m_pBot->monsterRun[roomID] == false) return;
    mon_stop_event* e = reinterpret_cast<mon_stop_event*>(buf);
    if (e->GameStartTime != m_pBot->StartTime[roomID]) return;
    send_monster_stop(e->key, e->roomid);
}

void Server::Event_MapBreak(int roomID, char* buf)
{
    std::lock_guard<std::mutex> lock_guard(GameRooms_lock);
    if (GameRooms[roomID].m_pMap == NULL) return;
    if (GameRooms[roomID].m_pMap->game_start == false) return;
    Mapbreak_event* p = reinterpret_cast<Mapbreak_event*>(buf);
    if (p->GameStartTime != GameRooms[roomID].m_pMap->StartTime) return;
    GameRooms[roomID].m_pMap->Map_collapse();

    Mapbreak_event e;
    e.roomid = roomID;
    e.size = sizeof(e);
    e.type = EventType::MapBreak;
    e.GameStartTime = p->GameStartTime;
    m_pTimer->push_event(roomID, OE_gEvent, MAP_BREAK_TIME, reinterpret_cast<char*>(&e));
}

void Server::Event_MonRespawn(int roomID, char* buf)
{
    if (m_pBot->monsterRun[roomID] == false) return;
    mon_respawn_event* e = reinterpret_cast<mon_respawn_event*>(buf);
    if (e->GameStartTime != m_pBot->StartTime[roomID]) return;
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
}

void Server::Event_GameEnd(int roomID, char* buf)
{
    //std::lock_guard<std::mutex> lock_guard(GameRooms_lock);
    if (GameRooms[roomID].m_pMap == NULL) return;
    if (GameRooms[roomID].m_pMap->game_start == false) return;
    game_end_event* e = reinterpret_cast<game_end_event*>(buf);
    if (e->GameStartTime != GameRooms[roomID].m_pMap->StartTime) return;
    game_end(roomID);
}
