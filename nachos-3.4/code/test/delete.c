#include "syscall.h"
#define MAX_LENGTH 32

int main()
{
	int result;
	char fileName[MAX_LENGTH];
	PrintString("\n\t\t\t\t\t-----PROGRAM DELETE-----\n\n");
	PrintString("Input file name you want to delete: ");
	result = Open("stdin", 2);


	if(result != -1)
	{	
		int read = Read(fileName, MAX_LENGTH, result);
		if (read < 1)
			PrintString("Invalid file name!\n");
		else
		{
			OpenFileId openf;
			int check;
			check = Delete(fileName);
			/*openf = Open("text", 0);
			if(openf != -1){
				check = Delete("text");
			}	*/
		}
		Close(result); // Goi ham Close de dong file
	}
	PrintString("\n\nPROGRAM's DONE\n\n");
	
	Halt();
	return 0;
}
