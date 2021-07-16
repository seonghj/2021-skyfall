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

void Bot::CheckTarget(SESSION& player, int roomID)
{
	XMFLOAT3 subtract;
	for (auto& mon : monsters[roomID]) {
		subtract = Vector3::Subtract((XMFLOAT3&)player.GetPosition(), (XMFLOAT3&)mon.GetPosition());
		if (Vector3::Length(subtract) <= 300)
		{
			subtract = Vector3::Normalize(subtract);
			subtract.y = 0;
			mon.Move(subtract, 0.5f);
			m_pServer->send_monster_pos(mon);
		}
	}
}

