#include "Map.h"

void Map::init_Map()
{
	int num_count[3] = { 0 };
	int n;

	srand((unsigned int)time(NULL));

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


	collapse_count = 0;
	Cloud.x = rand() % 3000;
	Cloud.y = rand() % 3000;
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
	return (a - b) / 5.f;
}

void Map::cloud_move()
{
	while (1)
	{
		for (int i = 0; i < 3; i++)
		{
			if (MAP_BLOCK_SIZE * i < Cloud.y && Cloud.y <= MAP_BLOCK_SIZE * (i+1))
			{
				if (MAP_BLOCK_SIZE < Cloud.x && Cloud.x <= MAP_BLOCK_SIZE * 1.5f)
				{
					Cloud.x += wind[i * 2];
					printf("%d", i * 2);
					break;
				}
				else if (MAP_BLOCK_SIZE * 1.5f < Cloud.x && Cloud.x < MAP_BLOCK_SIZE * 3)
				{
					Cloud.x += wind[i * 2 + 1];
					printf("%d", i * 2 + 1);
					break;
				}
			}
		}

		for (int i = 0; i < 3; i++)
		{
			if (MAP_BLOCK_SIZE * i < Cloud.x && Cloud.x <= MAP_BLOCK_SIZE * (i + 1))
			{
				if (MAP_BLOCK_SIZE < Cloud.y && Cloud.y <= MAP_BLOCK_SIZE * 1.5f)
				{
					Cloud.y += wind[i + 6];
					printf("%d", i + 6);
					break;
				}
				else if (MAP_BLOCK_SIZE * 1.5f < Cloud.y && Cloud.y < MAP_BLOCK_SIZE * 3)
				{
					Cloud.y += wind[i + 9];
					printf("%d", i + 9);
					break;
				}
			}
		}
		printf("x: %f | y: %f\n", Cloud.x, Cloud.y);
		collapse_count++;
		if (collapse_count % MAP_BREAK_TIME == 0)
			Map_collapse();
		Sleep(1000);
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

	if (num%2 == 0)
	{
		if (num == 0)
			wind[0] = 0.f, wind[6] = 0.f;
		else if (num == 2)
			wind[1] = 0.f, wind[8] = 0.f;
		else if (num == 4)
			wind[2] = 0.f, wind[3] = 0.f, wind[7] = 0.f, wind[10] = 0.f;
		else if (num == 6)
			wind[4] = 0.f, wind[9] = 0.f;
		else if (num == 8)
			wind[5] = 0.f, wind[11] = 0.f;
	}
	else
	{
		if (num == 1)
			wind[0] = 0.f, wind[1] = 0.f, wind[7] = 0.f;
		else if (num == 3)
			wind[2] = 0.f, wind[6] = 0.f, wind[9] = 0.f;
		else if (num == 5)
			wind[3] = 0.f, wind[8] = 0.f, wind[11] = 0.f;
		else if (num == 7)
			wind[4] = 0.f, wind[5] = 0.f, wind[10] = 0.f;
	}
	print_Map();
}