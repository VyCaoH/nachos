#include "syscall.h"
#include "copyright.h"
#define MAX_LENGTH 255
int main()
{

	int stdin;
	int stdout;
	char buffer[MAX_LENGTH];
	int len; 
	PrintString("\n\t\t\t\t\t-----PROGRAM ECHO-----\n\n");
	PrintString("Input information: ");
	
	// Goi ham Open de mo file stdin 
	// 0 la doc va ghi 
	// 1 chi doc >< -1 ko doc dc
	// 2 nhap tu console  >< -2 khong nhap dc
	// 3 xuat ra console
	stdin = Open("stdin", 2);
	if (stdin != -1)
	{
		// Goi ham Read de doc noi dung nhap vao stdin
		//Bay gio len vua co the la do dai, vua co the la ket qua (thanh cong/that bai) cua ham Read()
		len = Read(buffer, MAX_LENGTH, stdin);
		
		if (len != -1 && len != -2) //Kiem tra co bi loi, hay co EOF hay khong
		{
			stdout = Open("stdout", 3); // Goi ham Open voi type = 3 de su dung stdout
			if (stdout != -1) // Kiem tra co doc dc khong
			{
				PrintString("Output information: ");
				Write(buffer, len, stdout); // Goi ham Write de ghi noi dung doc duoc vao stdout
			}
			Close(stdout); // Goi ham Close de dong stdout
		}
	}
	Close(stdin); // Goi ham Close de dong stdin
	PrintString("\n\nPROGRAM'S DONE\n\n");
	Halt();
	return 0;
}
