#include "Bot.h"

void Monster::init()
{

}

void Bot::CheckTarget(SESSION& player, int roomID)
{
	XMFLOAT3 subtract;
	for (auto& mon : monsters[roomID]) {
		subtract = Vector3::Subtract(player.GetPosition(), mon.GetPosition());
		if (Vector3::Length(subtract) <= 300)
		{
			subtract = Vector3::Normalize(subtract);
			subtract.y = 0;
			m_ppGameObjects[i]->Move(subtract, 0.5f);
		}
		//printf("%d 번째 크기 : %f\n", i, Vector3::Length(Vector3::Subtract(m_ppGameObjects[i]->GetPosition(), m_pPlayer->GetPosition())));
	}
}

