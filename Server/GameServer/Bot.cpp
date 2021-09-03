#include "Bot.h"

void Monster::init()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	state = 1;
	type = 0;
	f3Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fPitch = 0;
	m_fYaw = 0;
	m_fRoll = 0;

	hp = 100;
	def = 0;
	lv = 0;
	att = 10;
	speed = 20;

	CanAttack = TRUE;
}

void Monster::SetPosition(float x, float y, float z)
{
	f3Position.store(XMFLOAT3(x, y, z));
	UpdateTransform(NULL);
}

void Monster::Move(const XMFLOAT3& vDirection, float fSpeed)
{
	SetPosition(f3Position.load().x + vDirection.x * fSpeed,
		f3Position.load().y + vDirection.y * fSpeed, f3Position.load().z +
		vDirection.z * fSpeed);
}

void Monster::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParent = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParent);

	m_fPitch = fmodf(m_fPitch.load() + fPitch, 360.f);
	m_fYaw = fmodf(m_fYaw.load() + fYaw, 360.f);
	m_fRoll = fmodf(m_fRoll.load() + fRoll, 360.f);

	UpdateTransform(NULL);
}

void Monster::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParent, *pxmf4x4Parent) : m_xmf4x4ToParent;
}

DirectX::XMFLOAT3 Monster::GetUp()
{
	return(Vector3::Normalize((XMFLOAT3&)XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

void Bot::Init(int roomID)
{
	for (int i = 0; i < MAX_MONSTER; i++) {
		monsters[roomID][i].init();
		monsters[roomID][i].key = i;
		monsters[roomID][i].roomID = roomID;
	}
	{
		// dragon
		monsters[roomID][0].type = MonsterType::Dragon;
		monsters[roomID][0].SpawnPos = XMFLOAT3{ 2000, 125, 5400 };
		monsters[roomID][0].f3Position = monsters[roomID][0].SpawnPos.load();
		monsters[roomID][0].Rotate(-90.0f, 20.0f, 0.0f);
		monsters[roomID][0].state = 1;
		monsters[roomID][0].speed = 7.f;

		// wolf
		monsters[roomID][1].type = MonsterType::Wolf;
		monsters[roomID][1].SpawnPos = XMFLOAT3{ 1116, 124, 2216 };
		monsters[roomID][1].f3Position = monsters[roomID][1].SpawnPos.load();
		monsters[roomID][1].Rotate(-90.0f, -40.0f, 0.0f);
		monsters[roomID][1].state = 1;
		monsters[roomID][1].speed = 5.f;

		monsters[roomID][2].type = MonsterType::Wolf;
		monsters[roomID][2].SpawnPos = XMFLOAT3{ 1161, 124, 3300 };
		monsters[roomID][2].f3Position = monsters[roomID][2].SpawnPos.load();
		monsters[roomID][2].Rotate(-90.0f, -40.0f, 0.0f);
		monsters[roomID][2].state = 1;
		monsters[roomID][2].speed = 5.f;

		monsters[roomID][3].type = MonsterType::Wolf;
		monsters[roomID][3].SpawnPos = XMFLOAT3{ 2228, 124, 2882 };
		monsters[roomID][3].f3Position = monsters[roomID][3].SpawnPos.load();
		monsters[roomID][3].Rotate(-90.0f, -40.0f, 0.0f);
		monsters[roomID][3].state = 1;
		monsters[roomID][3].speed = 5.f;

		monsters[roomID][4].type = MonsterType::Wolf;
		monsters[roomID][4].SpawnPos = XMFLOAT3{ 2148, 124, 3888 };
		monsters[roomID][4].f3Position = monsters[roomID][4].SpawnPos.load();
		monsters[roomID][4].Rotate(-90.0f, -40.0f, 0.0f);
		monsters[roomID][4].state = 1;
		monsters[roomID][4].speed = 5.f;

		monsters[roomID][5].type = MonsterType::Wolf;
		monsters[roomID][5].SpawnPos = XMFLOAT3{ 2960, 124, 3837 };
		monsters[roomID][5].f3Position = monsters[roomID][5].SpawnPos.load();
		monsters[roomID][5].Rotate(-90.0f, -40.0f, 0.0f);
		monsters[roomID][5].state = 1;
		monsters[roomID][5].speed = 5.f;

		monsters[roomID][6].type = MonsterType::Wolf;
		monsters[roomID][6].SpawnPos = XMFLOAT3{ 3548, 124, 4500};
		monsters[roomID][6].f3Position = monsters[roomID][6].SpawnPos.load();
		monsters[roomID][6].Rotate(-90.0f, -40.0f, 0.0f);
		monsters[roomID][6].state = 1;
		monsters[roomID][6].speed = 5.f;

		monsters[roomID][7].type = MonsterType::Wolf;
		monsters[roomID][7].SpawnPos = XMFLOAT3{ 5368, 124, 3763 };
		monsters[roomID][7].f3Position = monsters[roomID][7].SpawnPos.load();
		monsters[roomID][7].Rotate(-90.0f, -40.0f, 0.0f);
		monsters[roomID][7].state = 1;
		monsters[roomID][7].speed = 5.f;

		//Metalon
		monsters[roomID][8].type = MonsterType::Metalon;
		monsters[roomID][8].SpawnPos = XMFLOAT3{ 1202, 124, 615 };
		monsters[roomID][8].f3Position = monsters[roomID][8].SpawnPos.load();
		monsters[roomID][8].Rotate(-90.0f, 0.0f, 0.0f);
		monsters[roomID][8].state = 1;
		monsters[roomID][8].speed = 3.f;

		monsters[roomID][9].type = MonsterType::Metalon;
		monsters[roomID][9].SpawnPos = XMFLOAT3{ 2798, 124, 835 };
		monsters[roomID][9].f3Position = monsters[roomID][9].SpawnPos.load();
		monsters[roomID][9].Rotate(-90.0f, 0.0f, 0.0f);
		monsters[roomID][9].state = 1;
		monsters[roomID][9].speed = 3.f;

		monsters[roomID][10].type = MonsterType::Metalon;
		monsters[roomID][10].SpawnPos = XMFLOAT3{ 3188, 128, 1260 };
		monsters[roomID][10].f3Position = monsters[roomID][10].SpawnPos.load();
		monsters[roomID][10].Rotate(-90.0f, 0.0f, 0.0f);
		monsters[roomID][10].state = 1;
		monsters[roomID][10].speed = 3.f;


		monsters[roomID][11].type = MonsterType::Metalon;
		monsters[roomID][11].SpawnPos = XMFLOAT3{ 3243, 240,  490 };
		monsters[roomID][11].f3Position = monsters[roomID][11].SpawnPos.load();
		monsters[roomID][11].Rotate(-90.0f, 0.0f, 0.0f);
		monsters[roomID][11].state = 1;
		monsters[roomID][11].speed = 3.f;


		monsters[roomID][12].type = MonsterType::Metalon;
		monsters[roomID][12].SpawnPos = XMFLOAT3{ 3708, 124, 1925 };
		monsters[roomID][12].f3Position = monsters[roomID][12].SpawnPos.load();
		monsters[roomID][12].Rotate(-90.0f, 0.0f, 0.0f);
		monsters[roomID][12].state = 1;
		monsters[roomID][12].speed = 3.f;


		monsters[roomID][13].type = MonsterType::Metalon;
		monsters[roomID][13].SpawnPos = XMFLOAT3{ 4288, 124, 1245 };
		monsters[roomID][13].f3Position = monsters[roomID][13].SpawnPos.load();
		monsters[roomID][13].Rotate(-90.0f, 0.0f, 0.0f);
		monsters[roomID][13].state = 1;
		monsters[roomID][13].speed = 3.f;

		monsters[roomID][14].type = MonsterType::Metalon;
		monsters[roomID][14].SpawnPos = XMFLOAT3{ 5375, 124, 875 };
		monsters[roomID][14].f3Position = monsters[roomID][14].SpawnPos.load();
		monsters[roomID][14].Rotate(-90.0f, 0.0f, 0.0f);
		monsters[roomID][14].state = 1;
		monsters[roomID][14].speed = 3.f;
	}
}

void Bot::CheckBehavior(int roomID)
{
	for (int& player : m_pServer->GameRooms[roomID].pkeys) {
		if (player == INVALIDID) continue;
		if (m_pServer->sessions[player].connected.load() == false) continue;
		if (m_pServer->sessions[player].state.load() == Death) continue;
		for (Monster& mon : monsters[roomID]) {
			if (mon.state.load() == Death) continue;
			XMFLOAT3 subtract;
			float rotation;
			float range;
			float distance;

			XMFLOAT3 player_pos = (XMFLOAT3&)m_pServer->sessions[player].f3Position.load();
			XMFLOAT3 mon_pos = (XMFLOAT3&)mon.GetPosition();

			player_pos.y = 0;
			//mon_pos.y = 0;

			subtract = Vector3::Subtract(player_pos, mon_pos);
			subtract.y = 0;
			range = Vector3::Length(subtract);

			switch (mon.type) {
			case MonsterType::Dragon:
				distance = Atack_Distance_Dragon;
				break;
			case MonsterType::Wolf:
				distance = Atack_Distance_Wolf;
				break;
			case MonsterType::Metalon:
				distance = Atack_Distance_Metalon/2;
				break;
			}

			if (range < 300.0f)
			{
				//printf("%d dis %f\n", mon.key, range);
				if (mon.TraceTarget == INVALIDID)
					mon.TraceTarget = player;
				if (mon.TraceTarget == player) {
					subtract = Vector3::Normalize(subtract);
					/*mon.SetPosition(floor(mon.GetPosition().x)
						, floor(mon.GetPosition().y), floor(mon.GetPosition().z));*/

						// 실제 몬스터의 look 벡터
					XMFLOAT3 look = Vector3::ScalarProduct((XMFLOAT3&)mon.GetUp(), -1);

					rotation = acosf(Vector3::DotProduct(subtract, look)) * 180 / PI;
					//printf("rotation : %f\n", rotation);

					// 외적에 따라 가까운 방향으로 회전하도록
					XMFLOAT3 cross = Vector3::CrossProduct(subtract, look);

					float rotate_degree = -cross.y * rotation / 10;
					if (EPSILON <= rotation)
						mon.Rotate(0.0f, 0.0f, rotate_degree);
					else
						rotate_degree = 0;
					mon.recv_pos = false;

					// 플레이어 쪽으로 이동, 일정 거리 안까지 들어가면 공격, 이동 종료
					if (range <= distance && rotation <= 5) {
						if (mon.CanAttack == TRUE) {
							mon_attack_event ae;
							ae.size = sizeof(ae);
							ae.type = EventType::Mon_attack;
							ae.key = mon.key;
							ae.roomid = roomID;
							ae.GameStartTime = StartTime[roomID];
							ae.direction = cross;
							ae.target = m_pServer->sessions[player].InGamekey;
							m_pTimer->push_event(roomID, OE_gEvent, 60, reinterpret_cast<char*>(&ae));

							//m_pServer->send_monster_attack(mon, cross, m_pServer->sessions[player].InGamekey);

							mon.CanAttack = false;

							mon_attack_cooltime_event e;
							e.size = sizeof(e);
							e.type = EventType::Mon_attack_cooltime;
							e.key = mon.key;
							e.roomid = roomID;
							e.GameStartTime = StartTime[roomID];

							int cooltime = 0;

							switch (mon.type) {
							case MonsterType::Dragon:
								cooltime = 1000;
								break;
							case MonsterType::Wolf:
								cooltime = 2000;
								break;
							case MonsterType::Metalon:
								cooltime = 3000;
								break;
							}
							m_pTimer->push_event(roomID, OE_gEvent, cooltime, reinterpret_cast<char*>(&e));
						}
					}
					else if (range > distance) {
						/*mon.Move(subtract, mon.speed);
						m_pServer->send_monster_pos(mon, cross, player);*/
						mon_move_event me;
						me.size = sizeof(me);
						me.type = EventType::Mon_move_to_player;
						me.key = mon.key;
						me.roomid = roomID;
						me.GameStartTime = StartTime[roomID];
						me.subtract = subtract;
						me.direction = cross;
						me.target = player;
						m_pTimer->push_event(roomID, OE_gEvent, 60, reinterpret_cast<char*>(&me));
					}
				}
			}
			else {
				if (mon.TraceTarget == player) {
					//m_pServer->send_monster_stop(mon.key, roomID);
					mon_stop_event se;
					se.size = sizeof(se);
					se.type = EventType::Mon_stop;
					se.key = mon.key;
					se.roomid = roomID;
					se.GameStartTime = StartTime[roomID];
					m_pTimer->push_event(roomID, OE_gEvent, 60, reinterpret_cast<char*>(&se));
					//printf("Room: %d monster %d stop\n", roomID, mon.key);
					mon.TraceTarget = INVALIDID;
				}
			}
		}
	}

	mon_behavior_event e;
	e.size = sizeof(e);
	e.type = EventType::Mon_behavior;
	e.key = 0;
	e.roomid = roomID;
	e.GameStartTime = StartTime[roomID];
	m_pTimer->push_event(roomID, OE_gEvent, 60, reinterpret_cast<char*>(&e));
}

void Bot::RunBot(int roomID)
{
	if (monsterRun[roomID]) {
		mon_behavior_event e;
		e.size = sizeof(e);
		e.type = EventType::Mon_behavior;
		e.key = 0;
		e.roomid = roomID;
		e.GameStartTime = StartTime[roomID];
		m_pTimer->push_event(roomID, OE_gEvent, 60, reinterpret_cast<char*>(&e));
		monsterRun[roomID] = true;
	}
}

