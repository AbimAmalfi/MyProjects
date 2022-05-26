
#include <iostream>
#define SIZE 256
int search_index(int* array, int size, int value);
int main()
{
	int array[SIZE];	
	int value;
	for (int i = 0; i < SIZE; ++i)
	{
		array[i] = rand() % 99;
	}
	for (int i = 0; i < SIZE - 1; i++) 
	{
		for (int j = 0; j < SIZE - i - 1; j++) 
		{
			if (array[j] > array[j + 1]) 
			{
				value = array[j];
				array[j] = array[j + 1];
				array[j + 1] = value;
			}
		}
	}
	char c;
	value = getchar() - '0';
	if ((c = getchar()) != '\n') value = (value*10) + (c - '0');

	printf("index: %d\n",search_index(array, SIZE, value));
	return 0;
}

int search_index(int* array, int size, int value)
{
	int left_boundary = 0; 
	int right_boundary = size; 
	int mid;

	while (left_boundary <= right_boundary) 
	{
		mid = (left_boundary + right_boundary) / 2;
		if (array[mid] == value) return mid;
		if (array[mid] > value) right_boundary = mid - 1;
		else left_boundary = mid + 1;
	}
	return -1;
}