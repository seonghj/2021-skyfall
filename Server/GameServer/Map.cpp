#include "Map.h"

void CALLBACK Map::game_timer(HWND hWnd, UINT nMsg, UINT_PTR nID, DWORD dwTime)
{
	++game_time;
}

void Map::init_Map(Server* s)
{
	m_pServer = s;

	int num_count[3] = { 0 };
	int n;

	for (int i = 0; i < MAX_MAP_BLOCK; i++)
	{
		
		while (1)
		{
			int n = rand() % 3;
			if (num_count[n] < 3)
			{
				num_count[n]++;
				Map_num[i] = n;
				break;
			}
		}
	}

	for (int i = 0; i < 9; i++)
	{
		atm[i] = 1000 + rand() % 100;
		if (Map_num[i] == terrain::Desert)
			atm[i] -= 100;
		else if (Map_num[i] == terrain::Snowy_field)
			atm[i] += 100;
		isMap_block[i] = TRUE;
	}

	map_block_set p;
	p.type = PacketType::Type_map_set;
	p.size = sizeof(p);
	for (int i = 0; i < MAX_MAP_BLOCK; ++i)
		p.block_num[i] = Map_num[i];
	m_pServer->send_packet_to_players(game_num, reinterpret_cast<char*>(&p));

	game_time = 0;

	Set_wind();
	Set_cloudpos();
	print_Map();
	cloud_move();
}

void Map::Set_wind()
{
	for (int i = 0; i < 12; i++)
	{
		if (i < 6)
		{
			if (i == 0 || i == 1)
				wind[i] = calc_windpower(atm[i], atm[i + 1]);
			else if (i == 2 || i == 3)
				wind[i] = calc_windpower(atm[i + 1], atm[i + 2]);
			else if (i == 4 || i == 5)
				wind[i] = calc_windpower(atm[i + 2], atm[i + 3]);
		}
		else
		{
			wind[i] = -calc_windpower(atm[i - 6], atm[i - 3]);
		}
	}
}

void Map::Set_cloudpos()
{
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<int> dis(10, 2990);
	Cloud.x = dis(gen);
	Cloud.y = dis(gen);
}

void Map::print_Map()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			printf("%d: %f | ", Map_num[(3*i)+j], atm[(3 * i) + j]);
		}
		printf("\n");
	}
	printf("\n");
	for (int i = 0; i < 12; i++)
		printf("%d | %f\n", i, wind[i]);

	printf("\n");
	printf("x: %f | y: %f\n", Cloud.x, Cloud.y);
}

float Map::calc_windpower(float a, float b)
{
	return (a - b) / 5;
}

void Map::cloud_move()
{
	while (1)
	{
		for (int i = 0; i < 3; i++)
		{
			if (MAP_BLOCK_SIZE * i < Cloud.y && Cloud.y <= MAP_BLOCK_SIZE * (i+1))
			{
				if (0 < Cloud.x && Cloud.x <= MAP_BLOCK_SIZE * 1.5f)
				{
					Cloud.x += wind[i * 2];
					//printf("%d", i * 2);
					
				}
				else if (MAP_BLOCK_SIZE * 1.5f < Cloud.x && Cloud.x < MAP_BLOCK_SIZE * 3)
				{
					Cloud.x += wind[i * 2 + 1];
					//printf("%d", i * 2 + 1);
					
				}
			}
		}

		for (int i = 0; i < 3; i++)
		{
			if (MAP_BLOCK_SIZE * i < Cloud.x && Cloud.x <= MAP_BLOCK_SIZE * (i + 1))
			{
				if (0 < Cloud.y && Cloud.y <= MAP_BLOCK_SIZE * 1.5f)
				{
					Cloud.y += wind[i + 6];
					//printf("%d", i + 6);

				}
				else if (MAP_BLOCK_SIZE * 1.5f < Cloud.y && Cloud.y < MAP_BLOCK_SIZE * 3)
				{
					Cloud.y += wind[i + 9];
					//printf("%d", i + 9);

				}
			}
		}

		if(Cloud.x < 0 || Cloud.y < 0){ 
			Set_cloudpos();
		}

		//printf("cloud x: %f | y: %f\n\n", Cloud.x, Cloud.y);

		m_pServer->send_cloud_move_packet(Cloud.x, Cloud.y, game_num);

		++game_time;
		if (game_time % MAP_BREAK_TIME == 0)
			Map_collapse();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		//printf("%d\n", collapse_count);
	}
}

void Map::Map_collapse()
{
	srand((unsigned int)time(NULL));
	int num = 0;
	while (1)
	{
		num = rand() % 9;

		if (isMap_block[num] == TRUE)
		{
			isMap_block[num] = FALSE;
			break;
		}
	}

	printf("collapse block: %d\n", num);
	//printf("cloud x: %f | y: %f\n\n", Cloud.x, Cloud.y);
	atm[num] = 1000;
	Set_wind();

	/*if (num%2 == 0)
	{
		if (num == 0)
			wind[0] = 0, wind[6] = 0;
		else if (num == 2)
			wind[1] = 0, wind[8] = 0;
		else if (num == 4)
			wind[2] = 0, wind[3] = 0, wind[7] = 0, wind[10] = 0;
		else if (num == 6)
			wind[4] = 0, wind[9] = 0;
		else if (num == 8)
			wind[5] = 0, wind[11] = 0;
	}
	else
	{
		if (num == 1)
			wind[0] = 0, wind[1] = 0, wind[7] = 0;
		else if (num == 3)
			wind[2] = 0, wind[6] = 0, wind[9] = 0;
		else if (num == 5)
			wind[3] = 0, wind[8] = 0, wind[11] = 0;
		else if (num == 7)
			wind[4] = 0, wind[5] = 0, wind[10] = 0;
	}*/
	m_pServer->send_map_collapse_packet(num, game_num);
	print_Map();

	if (num == 9) {
		m_pServer->game_end(game_num);
	}
}