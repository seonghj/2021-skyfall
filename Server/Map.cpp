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
		Sleep(1000);
	}
}