#include <stdio.h>
#define SIZE 256

bool Check(char* message);
int main(void)
{
	char message[SIZE];
	fgets(message, SIZE, stdin);
	printf("%d",Check(message));
	return 0;
}
bool Check(char* message)
{
	unsigned short i = 0;
	short bracketcheck = 0;
	do
	{
		if ((message[i] == '(')) bracketcheck++;
		if (message[i] == ')') bracketcheck--;
		if (bracketcheck < 0) return false;		
		i++;
	} while (message[i] != '\n');
	if (bracketcheck == 0) return true;
	else return false;
}