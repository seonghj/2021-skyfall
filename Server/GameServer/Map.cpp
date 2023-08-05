#include "Map.h"

void Map::init_Map(Server* s, Timer* t)
{
	StartTime = std::chrono::system_clock::now();
	m_pServer = s;
	m_pTimer = t;

	memset(&over, 0, sizeof(over));
	over.dataBuffer.len = BUFSIZE;
	over.dataBuffer.buf = over.messageBuffer;
	over.type = OE_gEvent;
	over.roomID = roomnum;
	game_start = true;

	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<int> dis1(0, 2);
	int num_count[3] = { 0 };

	for (int i = 0; i < MAX_MAP_BLOCK; i++)
	{
		while (1)
		{
			int n = rand() % 3;
			if (num_count[n] < 3)
			{
				num_count[n]++;
				Map_type[i] = dis1(gen);
				break;
			}
		}
	}

	push_map_set_event();

	game_time = 0;

	ismove = true;
	
}

void Map::Set_map()
{
	int num_count[3] = { 0 };
	int n;

	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<int> dis2(10, 20);

	for (int i = 0; i < 9; i++)
	{
		atm[i] = dis2(gen);
		if (Map_type[i] == terrain::Desert)
			atm[i] -= 100;
		else if (Map_type[i] == terrain::Snowy_field)
			atm[i] += 100;
		isMap_block[i] = TRUE;
	}

	ismove = true;
	Set_wind();
	Set_cloudpos();

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
	std::uniform_int_distribution<int> dis(10, MAP_SIZE - MAP_BLOCK_SIZE);
	Cloud.x = dis(gen);
	Cloud.y = dis(gen);
}

void Map::print_Map()
{
	printf("\n");
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			printf("%d: %f | ", Map_type[(3*i)+j], atm[(3 * i) + j]);
		}
		printf("\n");
	}
	printf("\n");
	for (int i = 0; i < 12; i++)
		printf("%d | %f\n", i, wind[i]);

	printf("\n");
}

float Map::calc_windpower(float a, float b)
{
	return (a - b) / 5;
}

void Map::cloud_move()
{;
	for (int i = 0; i < 3; i++)
	{
		if (MAP_BLOCK_SIZE * i < Cloud.y && Cloud.y <= MAP_BLOCK_SIZE * (i + 1))
		{
			if (0 < Cloud.x && Cloud.x <= MAP_BLOCK_SIZE * 1.5f)
			{
				Cloud.x += wind[i * 2];

			}
			else if (MAP_BLOCK_SIZE * 1.5f < Cloud.x && Cloud.x < MAP_BLOCK_SIZE * 3)
			{
				Cloud.x += wind[i * 2 + 1];

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

			}
			else if (MAP_BLOCK_SIZE * 1.5f < Cloud.y && Cloud.y < MAP_BLOCK_SIZE * 3)
			{
				Cloud.y += wind[i + 9];

			}
		}
	}

	if (Cloud.x < 0 || Cloud.y < 0) {
		Set_cloudpos();
	}

	push_cloud_move_event();
}

void Map::Map_collapse()
{
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<int> dis(0, 8);
	int num = 0;
	while (1)
	{
		num = dis(gen);

		if (isMap_block[num] == TRUE)
		{
			isMap_block[num] = FALSE;
			break;
		}
	}

	printf("Room: %d collapse block: %d\n", roomnum, num);
	atm[num] = 1000;
	Set_wind();

	
	m_pServer->send_map_collapse_packet(num, roomnum);

	if (num == 9) {
		m_pServer->game_end(roomnum);
	}
}

void Map::push_cloud_move_event()
{
	cloud_move_packet p;
	p.type = EventType::Cloud_move;
	p.size = sizeof(cloud_move_packet);
	p.key = roomnum;
	p.roomid = roomnum;
	p.x = Cloud.x;
	p.z = Cloud.y;
	p.GameStartTime = StartTime;
	m_pTimer->push_event(roomnum, OE_gEvent, 1000, reinterpret_cast<char*>(&p));
}

void Map::push_map_set_event()
{
	map_block_set p;
	p.type = EventType::Mapset;
	p.size = sizeof(p);
	p.key = roomnum;
	p.GameStartTime = StartTime;
	over.dataBuffer.len = sizeof(p);
	memcpy(over.dataBuffer.buf, reinterpret_cast<char*>(&p), sizeof(p));
	DWORD Transferred = 0;
	BOOL ret = PostQueuedCompletionStatus(m_pServer->Gethcp(), Transferred
		, roomnum, &over.overlapped);
}