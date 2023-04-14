#include "syscall.h"
#define MAX_LENGTH 32

int main()
{
	int stdout;
	int openFileId;
	int fileSize;
	char buffer[MAX_LENGTH];
	char c; //Ky tu de in ra
	char fileName[MAX_LENGTH];
	int i; //Index for loop
	PrintString("\n\t\t\t-----PROGRAM CAT-----\n\n");
	PrintString("Input file name you want to read: ");

	// 0 la doc va ghi 
	// 1 chi doc >< -1 ko doc dc
	// 2 nhap tu console  >< -2 khong nhap dc
	// 3 xuat ra console
	
	//Goi ham ReadString de doc vao ten file
	//Co the su dung Open(stdin), nhung de tiet kiem thoi gian test ta dung ReadString
	ReadString(fileName, MAX_LENGTH);
	
	openFileId = Open(fileName, 1); // Goi ham Open de mo file 
	fileSize = Seek(-1,openFileId);	// seek den cuoi file de lay do dai noi dung
	Seek(0,openFileId);		// tra ve vi tri ban dau de read
	if (openFileId != -1) 
	{
		
		int len = Read(buffer, fileSize, openFileId);
		
		if (len != -1 && len != -2) //Kiem tra co bi loi, hay co EOF hay khong
		{	
			stdout = Open("stdout", 3); // Goi ham Open voi type = 3 de xuat ra console
			if (stdout != -1) // Kiem tra co doc dc khong
			{
			PrintString("File's content: \n");
			Write(buffer, len, stdout);
			}
			Close(stdout);
		}else{
			PrintString("Error \n");
		}
		Close(openFileId); // Goi ham Close de dong file
	}
	else
	{
		PrintString("Can't open file!!\n\n");
	}
	PrintString("\n\nPROGRAM's DONE\n\n");
	Halt();
	return 0;

}
