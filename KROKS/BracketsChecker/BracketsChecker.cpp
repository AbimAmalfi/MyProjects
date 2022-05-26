#include <stdio.h>
#define SIZE 256

int main(void)
{
	char message[SIZE];
	fgets(message, SIZE, stdin);
	unsigned short i = 0;
	short bracketcheck = 0;
	do
	{
		if ((message[i] == '(')) bracketcheck++;
		
		if (message[i] == ')') bracketcheck--;

		if (bracketcheck < 0)
		{
			printf("false");
			return 0;
		}
		i++;
	} while (message[i] != '\n');

	if (bracketcheck == 0) printf("true");
	else printf("false");

	return 0;
}