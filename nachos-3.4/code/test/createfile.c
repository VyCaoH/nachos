#include "syscall.h"
#define MAX_LENGTH 32

int main()
{
	int stdin;
	char fileName[MAX_LENGTH ];
	PrintString("\n\t\t\t\t\t-----PROGRAM CREATE FILE-----\n\n");
	PrintString("Input file name to create: ");
	

	// 0 la doc va ghi 
	// 1 chi doc >< -1 ko doc dc
	// 2 nhap tu console  >< -2 khong nhap dc
	// 3 xuat ra console 

	stdin = Open("stdin", 2); // Goi ham Open mo file stdin nhap vao ten file
	if (stdin != -2)
	{
		int len = Read(fileName, MAX_LENGTH, stdin); // Goi ham Read doc ten file vua nhap
		
		if(len < 1)
			PrintString("Invalid file name!\n\n");
		else
		{
			if (CreateFile(fileName) != -1) // Goi ham CreateFile
			{
				PrintString("Create successfully!\n\n");
			}else{
				PrintString("Create failed!\n\n");			
			}

		}
		Close(stdin); // Goi ham Close de dong stdin
	}
	Halt();
	return 0;
}
