#include "Bot.h"

void Monster::init()
{

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
	m_fPitch = fmodf(m_fRoll.load() + fRoll, 360.f);

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
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<int> dis1(0, 99999);

	/*for (auto& mon : monsters[roomID]) {
		mon.f3Position()
	}*/
}

void Bot::CheckTarget(int roomID)
{
	XMFLOAT3 subtract;
	for (SESSION& player : m_pServer->sessions[roomID]) {
		if (player.state.load() == false) continue;
		for (Monster& mon : monsters[roomID]) {
			if (mon.state.load() == 0) continue;
			subtract = Vector3::Subtract((XMFLOAT3&)player.GetPosition(), (XMFLOAT3&)mon.GetPosition());
			if ( 30 < Vector3::Length(subtract) || Vector3::Length(subtract) <= 300){
				subtract = Vector3::Normalize(subtract);
				subtract.y = 0.f;
				mon.Move(subtract, 0.5f);
				//printf("%f, %f, %f\n", mon.GetPosition().x, mon.GetPosition().y, player.GetPosition().z);
				
				/*for (SESSION& p : m_pServer->sessions[roomID]) {
					if (player.key == p.key) continue;
					if (player.connected == FALSE) continue;

					std::lock_guard <std::mutex> lg(p.nm_lock);
					std::unordered_set<int> old_nm;
					std::unordered_set<int> new_nm;

					old_nm = p.near_monster;

					if (m_pServer->in_VisualField(mon, p, roomID)) {
						new_nm.insert(mon.key.load());
					}

					if (old_nm.find(mon.key.load()) == old_nm.end()) {
						p.near_monster.insert(mon.key.load());
						m_pServer->send_add_monster(mon.key.load(), roomID, p.key.load());
					}

					if (new_nm.find(mon.key.load()) == new_nm.end()) {
						p.near_monster.erase(mon.key.load());
						m_pServer->send_remove_monster(mon.key.load(), roomID, p.key.load());
					}
				}*/

				m_pServer->send_monster_pos(mon, XMFLOAT3(0, 0, 0), 0);
			}
		}
	}
}

void Bot::CheckBehavior(int roomID)
{
	for (SESSION& player : m_pServer->sessions[roomID]) {
		if (player.state.load() == false) continue;
		for (Monster& mon : monsters[roomID]) {
			if (mon.state.load() == 0) continue;
			XMFLOAT3 subtract;
			float rotation;
			float range;
			subtract = Vector3::Subtract((XMFLOAT3&)player.f3Position.load(), (XMFLOAT3&)mon.GetPosition());
			range = Vector3::Length(subtract);

			//float distance = (pMonster->FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBExtents.z - pMonster->FindFrame("BoundingBox")->m_pMesh->m_xmf3AABBCenter.z) / 2.0f;
			if (range < 300.0f)
			{
				subtract = Vector3::Normalize(subtract);
				// 실제 몬스터의 look 벡터
				XMFLOAT3 look = Vector3::ScalarProduct((XMFLOAT3&)mon.GetUp(), -1);

				rotation = acosf(Vector3::DotProduct(subtract, look)) * 180 / PI;
				//printf("rotation : %f\n", rotation);

				// 플레이어 쪽으로 이동, 일정 거리 안까지 들어가면 공격, 이동 종료
				/*if (range <= distance && rotation <= 5) {
					pMonster->Attack();
					return;
				}
				else if (range > distance) {
					mon.Move(subtract, 0.5f);
				}*/

				mon.Move(subtract, 0.5f);
				// 외적에 따라 가까운 방향으로 회전하도록
				XMFLOAT3 cross = Vector3::CrossProduct(subtract, look);

				if (EPSILON <= rotation)
					mon.Rotate(0.0f, 0.0f, -cross.y * rotation / 10);
				//printf("%f, %f, %f\n", cross.y, rotation, -cross.y * rotation / 10);

				m_pServer->send_monster_pos(mon, cross, -cross.y * rotation / 10);


			}
		}
	}
}

void Bot::RunBot(int roomID)
{
	if (monsterRun) {
		mon_move_to_player_event e;
		e.size = sizeof(e);
		e.type = EventType::Mon_move_to_player;
		e.key = 0;
		e.roomid = roomID;
		m_pTimer->push_event(roomID, OE_gEvent, 1, reinterpret_cast<char*>(&e));
	}
}

