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

void Bot::CheckTarget(std::array<SESSION, 20> sessions, int roomID)
{
	XMFLOAT3 subtract;
	for (auto& player : sessions) {
		for (auto& mon : monsters[roomID]) {
			if (mon.state == 0) continue;
			subtract = Vector3::Subtract((XMFLOAT3&)player.GetPosition(), (XMFLOAT3&)mon.GetPosition());
			if (Vector3::Length(subtract) <= 300)
			{
				subtract = Vector3::Normalize(subtract);
				subtract.y = 0;
				mon.Move(subtract, 0.5f);

				for (SESSION& p : sessions) {
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
				}

				m_pServer->send_monster_pos(mon);
			}
		}
	}
}

void Bot::RunBot(std::array<SESSION, 20> sessions, int roomID)
{
	if (monsterRun) {
		mon_move_to_player_event e;
		e.size = sizeof(e);
		e.type = EventType::Mon_move_to_player;
		e.key = 0;
		e.roomid = roomID;
		m_pTimer->push_event(roomID, OE_gEvent, 166, reinterpret_cast<char*>(&e));
	}
}

