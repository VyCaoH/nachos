// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#include "copyright.h"
#include "system.h"
#include "syscall.h"

#define MaxFileLength 32
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
//Function to increase programming counter to make next step. Save value of current PC for previous PC, 
//upload next value for current PC, upload next value for next PC.
void IncreasePC()
{
	
	int counter = machine->ReadRegister(PCReg);
   	machine->WriteRegister(PrevPCReg, counter);
    	counter = machine->ReadRegister(NextPCReg);
    	machine->WriteRegister(PCReg, counter);
   	machine->WriteRegister(NextPCReg, counter + 4);
}
//Use Nachos FileSystem Object to create an empty file
char* User2System(int virtAddr,int limit)
{
    //Input: Address space of user (int) - limit of buffer(int)
    //Output: count buffer(char*)
    //Purpose: copy memory space from User to System
    int i;// index
    int oneChar;
    char* kernelBuf = NULL;
    kernelBuf = new char[limit +1];//need for terminal string
    if (kernelBuf == NULL)
        return kernelBuf;
        
    memset(kernelBuf,0,limit+1);
    //printf("\n Filename u2s:");
    for (i = 0 ; i < limit ;i++)
    {
        machine->ReadMem(virtAddr+i,1,&oneChar);
        kernelBuf[i] = (char)oneChar;
        //printf("%c",kernelBuf[i]);
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}

int System2User(int virtAddr,int len,char* buffer)
{
    //Input: memory space of User(int) - limit of buffer(int) - memory count buffer(char*)
    //Output: number of byte copied
    //Purpose: copy memory space from System to User
    if (len < 0) return -1;
    if (len == 0)return len;
    int i = 0;
    int oneChar = 0 ;
    do{
        oneChar= (int) buffer[i];
        machine->WriteMem(virtAddr+i,1,oneChar);
        i ++;
    }while(i < len && oneChar != 0);
    return i;
}
//Function to solve runtime Exception and system call
void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch (which) {
	case NoException: //only this case is return; 
		return;
	//other cases will shut down the PC
	case PageFaultException:
	    DEBUG('a', "\n No valid translation found.");
	    printf ("\n\n No valid translation found.");
	    interrupt->Halt();
		break;
	case ReadOnlyException:
	    DEBUG('a', "\n Write attemped to page marked read-only.");
	    printf ("\n\n Write attemped to page marked read-only.");
	    interrupt->Halt();
		break;
	case BusErrorException:
	    DEBUG('a', "\n Translte resulted invalid physical address.");
	    printf ("\n\n Translte resulted invalid physical address.");
	    interrupt->Halt();
		break;
	case AddressErrorException:
		DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space");
		printf("\n\n Unaligned reference or one that was beyond the end of the address space");
		interrupt->Halt();
		break;

	case OverflowException:
		DEBUG('a', "\nInteger overflow in add or sub.");
		printf("\n\n Integer overflow in add or sub.");
		interrupt->Halt();
		break;

	case IllegalInstrException:
		DEBUG('a', "\n Unimplemented or reserved instr.");
		printf("\n\n Unimplemented or reserved instr.");
		interrupt->Halt();
		break;

	case NumExceptionTypes:
		DEBUG('a', "\n Number exception types");
		printf("\n\n Number exception types");
		interrupt->Halt();
        	break;
	case SyscallException:
		switch (type){
			case SC_Halt:
				//Input: none
				//Output: notify to shut down the computer
				//Purpose: Turn off OS
				DEBUG('a', "\n Shutdown, initiated by user program. ");
				printf ("\nShutdown, initiated by user program. ");
				interrupt->Halt();
				break;
			case SC_CreateFile:
			{
				//Input: Address from User's space of file name
				//Output: -1: fail, 0: successful
				//Purpose: Create file with parameter is filename
				int virtAddr;
				char* filename;
				DEBUG('a',"\n SC_Create call ...");
				DEBUG('a',"\n Reading virtual address of filename");
				//get file name from register 4
				virtAddr = machine->ReadRegister(4);
				DEBUG ('a',"\n Reading filename.");
				// MaxFileLength is 32
				filename = User2System(virtAddr,MaxFileLength+1);
				if (filename == NULL){
					printf("\n Not enough memory in system");
					DEBUG('a',"\n Not enough memory in system");
					machine->WriteRegister(2,-1);//return error to user 
					delete filename;
					break;
				}
				DEBUG('a',"\n Finish reading filename.");
				// Create file with size = 0
				if (!fileSystem->Create(filename,0)){
					printf("\n Error create file '%s'",filename);
					machine->WriteRegister(2,-1);
					delete filename;
					break;
				}
				machine->WriteRegister(2,0); // return to program
				// user success
				delete filename;
				IncreasePC();
				break;
			}
			case SC_Open:
			{
			// Input: arg1: address of string name, arg2: type
			// Output: return OpenFileID if success, -1 if fail
			// Purpose:: return ID of a file
			int virtAddr = machine->ReadRegister(4); //take address of file name from register 4
			int type = machine->ReadRegister(5); // take type from register 5
			char* filename;
			//copy string from UserSpace to SystemSpace with counter is MAXLENGTH
			filename = User2System(virtAddr, MaxFileLength);
			//Chcek if there's any free slot to open file
			int freeSlot = fileSystem->FindFreeSlot();
			if (freeSlot != -1) //Only continue if there's still free slot
			{
				if (type == 0 || type == 1) //handle when type is 0 or 1
				{
				
					if ((fileSystem->openf[freeSlot] = fileSystem->Open(filename, type)) != NULL)//Open file successfully
					{
						machine->WriteRegister(2, freeSlot); //return FileID
					}
				}
				else if (type == 2) //handle with type = 2 (sdtin)
				{
					machine->WriteRegister(2, 0); //return FileID
				}
				else //handle with type = 3 (sdtout)
				{
					machine->WriteRegister(2, 1); //return FileID
				}
				delete[] filename;
				break;
			}
			machine->WriteRegister(2, -1); //Cant open file
		
			delete[] filename;
			break;
			}

			case SC_Close:
			{
			//Input: id of File(OpenFileID)
			//Output: 0: success, -1: fail
			//Purpose: Close file
			int fid = machine->ReadRegister(4); //Take file id from Register 4
			if (fid >= 0 && fid <= 10) //only handle when limit in [0, 14]
			{
				if (fileSystem->openf[fid]) //if open file successfully
				{
					delete fileSystem->openf[fid]; //delete memory space stored file
					fileSystem->openf[fid] = NULL; //assign memory space = NULL
					machine->WriteRegister(2, 0);
					break;
				}
			}
				machine->WriteRegister(2, -1);
				break;
			}
			case SC_Read:
			{
				// Input: buffer(char*), number of char(int), id of file(OpenFileID)
				// Output: -1: fail, true number of byte read: success, -2: success
				// Purpose: Read file with parameter is buffer, number of character and ID of file
				int virtAddr = machine->ReadRegister(4); // Take address of buffer from Register 4
				int charcount = machine->ReadRegister(5); //Take char count from Register 5
				int id = machine->ReadRegister(6); //Take file id from Register 6
				int OldPos;
				int NewPos;
				char *buf;
				//Check if id of file is out of range of FileDiscriptor
				if (id < 0 || id > 10)
				{
					printf("\nIt's out of range, can't read.");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				//Check if file is exist
				if (fileSystem->openf[id] == NULL)
				{
					printf("\nIt's doesn't exist, can't read");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				//if file type = 3 -> read file stdout -> return -1
				if (fileSystem->openf[id]->type == 3) 
				{
					printf("\nCan't read file stdout.");
					machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
				//Check if success -> take old position
				OldPos = fileSystem->openf[id]->GetCurrentPos(); 
				//Copy string from User space to System space with buffer counter long charcount
				buf = User2System(virtAddr, charcount); 
				///if file type = 2 -> read file stdin
				if (fileSystem->openf[id]->type == 2)
				{
					//Use Read function of class SynchConsole to return true bytes that read.
					int size = gSynchConsole->Read(buf, charcount); 
					//Copy string from System space to User space with buffer counter long true byte
					System2User(virtAddr, size, buf); 
					machine->WriteRegister(2, size); //Return true number of bytes read
					delete buf;
					IncreasePC();
					return;
				}
				//if it's normal file -> return true byte
				if ((fileSystem->openf[id]->Read(buf, charcount)) > 0)
				{
				//true byte = newPos - oldPos
					NewPos = fileSystem->openf[id]->GetCurrentPos();
				//Copy string from System space to User space with buffer counter long true byte
					System2User(virtAddr, NewPos - OldPos, buf); 
					machine->WriteRegister(2, NewPos - OldPos);
				}
				//Read file with content = NULL -> return - 2
				else
				{
					machine->WriteRegister(2, -2);
				}
				delete buf;
				IncreasePC();
				return;
			}

				case SC_Write:
				{
					// Input: buffer(char*), number of char(int), id of file(OpenFileID)
					// Output: -1: fail, true number of byte write: success, -2: success
					// Purpose: Write file with parameter is buffer, number of character and ID of file
					int virtAddr = machine->ReadRegister(4); // Take address of buffer from Register 4
					int charcount = machine->ReadRegister(5); // Take char count from Register 5
					int id = machine->ReadRegister(6); // Take file id from Register 6
					int OldPos;
					int NewPos;
					char *buf;
					//Check if id of file is out of range of FileDiscriptor
					if (id < 0 || id > 10)
					{
						printf("\nIt's out of range, can't write.");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					//Check if file is exist
					if (fileSystem->openf[id] == NULL)
					{
						printf("\nItsn't exist, can't write.");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					//if file type is 1 (only read) of 2 (stdin) -> return -1
					if (fileSystem->openf[id]->type == 1 || fileSystem->openf[id]->type == 2)
					{
						printf("\nCan't write file stdin and file only read.");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					OldPos = fileSystem->openf[id]->GetCurrentPos(); //check if success -> take old position
					buf = User2System(virtAddr, charcount);//Copy string from Userspace to Systemspace with buffer counter long true byte
					// if file type is 0 (write and read) -> return true byte
					if (fileSystem->openf[id]->type == 0)
					{
						if ((fileSystem->openf[id]->Write(buf, charcount)) > 0)
						{
							// true byte = NewPos - OldPos
							NewPos = fileSystem->openf[id]->GetCurrentPos();
							machine->WriteRegister(2, NewPos - OldPos);
							delete buf;
							IncreasePC();
							return;
						}
					}
					// if file type is 3 (stdout) 
					if (fileSystem->openf[id]->type == 3) 
					{
						int i = 0;
						while (buf[i] != 0 && buf[i] != '\n') //loop for write until meet '\n'
						{
							gSynchConsole->Write(buf + i, 1); //use Write function of SynchConSole
							i++;
						}
						buf[i] = '\n';
						gSynchConsole->Write(buf + i, 1); //write character '\n'
						machine->WriteRegister(2, i - 1); //return true byte that wrote
						delete buf;
						IncreasePC();
						return;
					}
				}
				case SC_Seek:
				{
					// Input: position(int), File ID(OpenFileID)
					// Output: -1: Fail, True postion: success
					//Purpose: move pointer to request in file with parameter is destination and file ID
					int pos = machine->ReadRegister(4); //Take address of pointer from register 4
					int id = machine->ReadRegister(5); //Take ID of file from register 5
					//Check if id of file is out of range of FileDiscriptor
					if (id < 0 || id > 10)
					{
						printf("\nIt's out of range. Can't seek");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					//Check if file is exist
					if (fileSystem->openf[id] == NULL)
					{
						printf("\nItsn't exist. Can't seek");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					//Check if there's call from console
					if (id == 0 || id == 1)
					{
						printf("\nCan't seek in file console");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					//If pos = -1 then assign pos = length, if not maintain pos
					pos = (pos == -1) ? fileSystem->openf[id]->Length() : pos;
					//Check valid postion for pos
					if (pos > fileSystem->openf[id]->Length() || pos < 0) 
					{
						printf("\nCan't seek to this postion");
						machine->WriteRegister(2, -1);
					}
					//if valid -> return true postion in file
					else
					{
						fileSystem->openf[id]->Seek(pos);
						machine->WriteRegister(2, pos);
					}
					IncreasePC();
					return;
				}
				case SC_Delete:
				{
					// Input: buffer(char*), File ID(OpenFileID)
					// Output: -1: Fail, 0: success
					//Purpose: delete a file
					int virtAddr = machine->ReadRegister(4);
					char* filename;
					filename = User2System(virtAddr ,MaxFileLength + 1); //Copy file name from User Space to System Space
					//Check if file is exist
					if(fileSystem->isExist(filename)==FALSE){
						printf("\nFile %s isn't exist!\n.",filename);
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					//Check if file is opening
					if(fileSystem->isOpen(filename) == TRUE){
						printf("\nCan not delete file %s, it's opening\n.",filename);
						//delete filename;
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					//Check if can remove or not
					if(fileSystem->Remove(filename)==FALSE){
						printf("\nSuccessful deleted file %s\n.",filename);
						//printf("\nFail attemp to delete file %s\n.",filename);
						//delete filename;
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					printf("\nSuccessful deleted file %s\n.",filename);
					//delete filename;
					machine->WriteRegister(2, 0);
					IncreasePC();
					return;
				}

				case SC_ReadString:
				{
					// Input: Buffer(char*), max length of input string(int)
					//Output: none
					//Purpose: read a string with paramerter is buffer and max length
					int virtAddr, length;
					char* buffer;
					virtAddr = machine->ReadRegister(4); //Take value of buffer from register 4
					length = machine->ReadRegister(5); // Take value of input string from register 5
					buffer = User2System(virtAddr, length); //Copy string from User Space to System Space
					gSynchConsole->Read(buffer, length); //Call function Read of SynchConsole to read string
					System2User(virtAddr, length, buffer); ///Copy string from System Space to User Space
					delete buffer; 
					IncreasePC();
					return;
					//break;
				}

				case SC_PrintString:
				{
					// Input: Buffer(char*)
					// Output: String read from buffer
					// Purpose: output a string to screen
					int virtAddr;
					char* buffer;
					virtAddr = machine->ReadRegister(4); //Take value of buffer from register 4
					buffer = User2System(virtAddr, 255); //Copy string from User Space to System Space with buffer have a length 55 characters
					int length = 0;
					while (buffer[length] != 0) length++; //count true length of string
					gSynchConsole->Write(buffer, length + 1); //Call function Write of SynchConsole to print string
					delete buffer; 
				
					break;
				}
				default:
					printf("\n Unexpected user mode exception (%d %d)", which,type);
					interrupt->Halt();
				}
				IncreasePC();
	   	 }
}
