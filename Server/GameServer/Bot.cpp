#include "Bot.h"

void Monster::init()
{

}

void Monster::SetPosition(float x, float y, float z)
{
	f3Position.store(XMFLOAT3(x, y, z));
}

void Monster::Move(const XMFLOAT3& vDirection, float fSpeed)
{
	SetPosition(f3Position.load().x + vDirection.x * fSpeed,
		f3Position.load().y + vDirection.y * fSpeed, f3Position.load().z +
		vDirection.z * fSpeed);
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
			if (Vector3::Length(subtract) <= 300)
			{
				subtract = Vector3::Normalize(subtract);
				subtract.y = 0.f;
				//printf("%f\n", subtract.y);
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

				m_pServer->send_monster_pos(mon);
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
		m_pTimer->push_event(roomID, OE_gEvent, 16, reinterpret_cast<char*>(&e));
	}
}

