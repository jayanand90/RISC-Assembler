/* The RiSC - 16 Assembler */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINELENGTH 1000

/* Splitting an instruction into individual tokens : labelPtr, opcpdePtr, arg0Ptr, arg1Ptr, arg2Ptr */

char * readAndParse(FILE *inFilePtr, char *lineString, 	char **labelPtr, char **opcodePtr, char **arg0Ptr, 
	char **arg1Ptr, char **arg2Ptr)
{
	char *statusString, *firsttoken;
	statusString = fgets(lineString, MAXLINELENGTH, inFilePtr); // statusString stores the first instruction.
	if (statusString != NULL) 
    {
	
		firsttoken = strtok(lineString, " \t\n");
		
		if (firsttoken == NULL || firsttoken[0] == '#')
         {
			return readAndParse(inFilePtr, lineString, labelPtr, opcodePtr, arg0Ptr, arg1Ptr, arg2Ptr);
	     } 
        else if (firsttoken[strlen(firsttoken) - 1] == ':') // chekcking and handling a label.
         {
			*labelPtr = firsttoken;                         //assigning label to firsttoken
			*opcodePtr = strtok(NULL, " \t\n");             //and then searching for opcode 
			firsttoken[strlen(firsttoken) - 1] = '\0';
		 }  
        else
         {
			*labelPtr = NULL;                              // when the instruction starts off with an opcode
			*opcodePtr = firsttoken;
		 }
		
		*arg0Ptr = strtok(NULL, ", \t\n");                 // in either case (label or no),store the operands 
		*arg1Ptr = strtok(NULL, ", \t\n");
		*arg2Ptr = strtok(NULL, ", \t\n");
		
	}
	return(statusString);
}

/* This function returns  1 if string is a number. Used to know whether argument is a label or an imm value*/
int isNumber(char *string) 
{
	int i;
	return( (sscanf(string, "%d", &i)) == 1);
}


/* labels and their locations are stored in a linked list */
struct label
{
     char name[MAXLINELENGTH];
     int index;
     struct label *next;
};


/* main() begins here */

int main()
{
    
struct label *templabel, *ptr, *y, *front = NULL;
	char *inFileString, *outFileString;
	FILE *inFilePtr, *outFilePtr;
	char *label, *opcode, *arg0, *arg1, *arg2;
	char lineString[MAXLINELENGTH+1];
	int labelcount=0;

	inFilePtr = fopen("assemblycode.txt", "r");

	outFilePtr = fopen("hexcode.txt", "w");

	int pc_count=0; /*This is for the number of times ReadAndParse has been called and thus keep track of PC */
	
	int k;

	
	/* To create a linked list to store label names and their locations*/
	while(readAndParse(inFilePtr, lineString, &label, &opcode, &arg0, &arg1, &arg2) != NULL) 
	{
	if (label!=NULL) // if there is a label, it will go in
	{
                     if (front == NULL)
                     {
                               front = malloc (sizeof (struct label));
                               strcpy(front->name,label);
                               front->index = pc_count;
                               front->next=NULL;
                               ptr = front;
                     }
                     else
                     {
                         y = malloc (sizeof (struct label));
                         strcpy(y->name,label);
                         y->index = pc_count;
                         y->next = NULL;
                         ptr->next = y;
                         ptr = y;
                     }
    }
    pc_count++;
	}
	
	// To count number of labels.
	templabel = front;
	while(templabel != NULL)
    {
               labelcount++;
               templabel = templabel->next;
    }

	rewind(inFilePtr);
	pc_count=0;
	int mc[4];/* Array to store values after bit shifting from arg0, arg1, arg2 and to hold final value */
	int machine_opcode;
	int check;
	
	while(readAndParse(inFilePtr, lineString, &label, &opcode, &arg0, &arg1, &arg2) != NULL)
	{
	
	// ADD  
	
	if (strcmp(opcode,"add")==0)
	{
	if((arg2==NULL)||(strcmp(arg2,"#")==0))
    printf("There is no argument for the opcode: %s \n",opcode);
	else
	{
	mc[0]=((atoi(arg0))<<10);     
	mc[1]=((atoi(arg1))<<7);
	mc[2]=((atoi(arg2))<<0);
	mc[3]=(mc[2]|mc[1]|mc[0]);    //to get the final 16-bit number
	fprintf(outFilePtr,"%04x \n",mc[3]);	                     
	}
	}
	
	
	// ADDI 

	else if (strcmp(opcode,"addi")==0)
	{
	if((arg2==NULL)||(strcmp(arg2,"#")==0))
    printf("There is no argument for the opcode: %s \n",opcode);
	else
	{
	machine_opcode=0x2000;            // This is to set 001 as the opcode.
	mc[0]=((atoi(arg0))<<10);
	mc[1]=((atoi(arg1))<<7);

	if((check=isNumber(arg2))==1)
	{
	if(((atoi(arg2))<=63)&&((atoi(arg2))>=-64))
	{
	mc[2]=((atoi(arg2))&0x7f);    
	mc[3]=(mc[2]|mc[1]|mc[0]|machine_opcode);
	fprintf(outFilePtr,"%04x \n",mc[3]);
	}
	else 
    printf("Oops! Immediate is not within range %s\n",opcode);
	}
	else // only when the arg 2 is a label and not imm number
	{
         templabel = front;
         k = 0;
         while(templabel != NULL)    // compare arg2 with every name in the label array. If arg2 matches a label in the label array, exit for loop. k increments only if 1 && 1.
         {
                    if (strcmp(arg2,(templabel->name))!=0)
                    {
                    k++;
                    }
                    else
                    {
                        break;
                    }
                    templabel = templabel->next;
         }
                                                     

	if(k==labelcount)
    printf("label \"%s\" does not exist!\n",arg2);
	else
	{
	mc[2]=templabel->index;
	mc[3]=(mc[2]|mc[1]|mc[0]|machine_opcode);
	fprintf(outFilePtr,"%04x \n",mc[3]);
	}
	}
	}
	}

	//NAND 

	else if (strcmp(opcode,"nand")==0)
	{
	if((arg2==NULL)||(strcmp(arg2,"#")==0))
    printf("There is no argument for the opcode: %s \n",opcode);
	else
	{
	machine_opcode=0x4000;
	mc[0]=((atoi(arg0))<<10);
	mc[1]=((atoi(arg1))<<7);
	mc[2]=((atoi(arg2))<<0);
	mc[3]=(mc[2]|mc[1]|mc[0]|machine_opcode);
	fprintf(outFilePtr,"%04x \n",mc[3]);	
	}
	}

	// LUI 

	else if (strcmp(opcode,"lui")==0)	
	{
	if((arg1==NULL)||(strcmp(arg1,"#")==0))
    printf("There is no argument for the opcode: %s \n",opcode);
	else
	{
	if((check=isNumber(arg1))==1)
	{
	if(((atoi(arg2))<=1023)&&((atoi(arg2))>=0))
	{
	machine_opcode=0x6000;
	mc[0]=((atoi(arg0))<<10);
	mc[1]=((atoi(arg1))&0x3ff);
	mc[2]=(mc[1]|mc[0]|machine_opcode);
	fprintf(outFilePtr,"%04x \n",mc[2]);	
	}
	else 
    printf("Oops! Immediate is not within range %s\n",opcode);
	}
	else
	{
	machine_opcode=0x6000;
	mc[0]=((atoi(arg0))<<10);
	
         templabel = front;
         k = 0;
         while(templabel != NULL)                   
         {
                    if (strcmp(arg1,(templabel->name))!=0)
                    {
                    k++;
                    }
                    else
                    {
                        break;
                    }
                    templabel = templabel->next;
         }  
	if(k==labelcount)
    printf("label \"%s\" does not exist!\n",arg1);
	else
	{
	mc[1]=templabel->index;
	mc[2]=(mc[1]|mc[0]|machine_opcode);
	fprintf(outFilePtr,"%04x \n",mc[2]);	
	}
	}
	}
	}

	// SW 

	else if (strcmp(opcode,"sw")==0)
	{
	if((arg2==NULL)||(strcmp(arg2,"#")==0))
    printf("There is no argument for the opcode: %s \n",opcode);
	else
	{
	machine_opcode=0x8000;
	mc[0]=((atoi(arg0))<<10);
	mc[1]=((atoi(arg1))<<7);
	if((check=isNumber(arg2))==1)
	{
	if(((atoi(arg2))<=64)&&((atoi(arg2))>=-63))mc[2]=((atoi(arg2))<<0);
	else 
    printf("Oops! Immediate is not within range %s\n",opcode);
	}
	else
	{
	     templabel = front;
         k = 0;
         while(templabel != NULL)                   
         {
                    if (strcmp(arg2,(templabel->name))!=0)
                    {
                    k++;
                    }
                    else
                    {
                        break;
                    }
                    templabel = templabel->next;
         }
	if(k==labelcount)
    printf("label \"%s\" does not exist!\n",arg2);
	else
	{
	mc[2]=templabel->index;
	}
	}
	mc[3]=(mc[2]|mc[1]|mc[0]|machine_opcode);
	fprintf(outFilePtr,"%04x \n",mc[3]);	
		
	}
	}
	
	

	// LW 

	else if (strcmp(opcode,"lw")==0)
	{
	if((arg2==NULL)||(strcmp(arg2,"#")==0))
    printf("There is no argument for the opcode: %s \n",opcode);
	else
	{
	machine_opcode=0xa000;
	mc[0]=((atoi(arg0))<<10);
	mc[1]=((atoi(arg1))<<7);
	
	if((check=isNumber(arg2))==1) 
	{
	if(((atoi(arg2))<=64)&&((atoi(arg2))>=-63))mc[2]=((atoi(arg2))<<0);
	else 
    printf("Oops! Immediate is not within range %s\n",opcode);
	}
	else
	{
	
	     templabel = front;
         k = 0;
         while(templabel != NULL)                  
         {
                    if (strcmp(arg2,(templabel->name))!=0)
                    {
                    k++;
                    }
                    else
                    {
                        break;
                    }
                    templabel = templabel->next;
         }
	if(k==labelcount)
    printf("label \"%s\" does not exist!\n",arg2);
	else
	{
	mc[2]=templabel->index;
	
    }
    }	
	mc[3]=(mc[2]|mc[1]|mc[0]|machine_opcode);
	fprintf(outFilePtr,"%04x \n",mc[3]);
	
	}
	}
  
	// BNE 

	else if (strcmp(opcode,"bne")==0)
	{
	if((arg2==NULL)||(strcmp(arg2,"#")==0))
    printf("There is no argument for the opcode: %s \n",opcode);
	else
	{
	machine_opcode=0xc000;
	mc[0]=((atoi(arg0))<<10);
	mc[1]=((atoi(arg1))<<7);
	if((check=isNumber(arg2))==1) 
	{
	if(((atoi(arg2))<=64)&&((atoi(arg2))>=-63))mc[2]=((atoi(arg2))<<0);
	else 
    printf("Oops! Immediate is not within range %s\n",opcode);
	}
	else
	{
	     templabel = front;
         k = 0;
         while(templabel != NULL)                   
         {
                    if (strcmp(arg2,(templabel->name))!=0)
                    {
                    k++;
                    }
                    else
                    {
                        break;
                    }
                    templabel = templabel->next;
         }
	if(k==labelcount)
    printf("label \"%s\" does not exist!\n",arg2);
	else
	{
	mc[2]=(((templabel->index)-1-pc_count)& 0x7f);
	}
	}
	mc[3]=(mc[2]|mc[1]|mc[0]|machine_opcode);
	fprintf(outFilePtr,"%04x \n",mc[3]);	
	
	}
	}

	// JALR

	else if (strcmp(opcode,"jalr")==0)
	{
	if((arg1==NULL)||(strcmp(arg1,"#")==0))
    printf("There is no argument for the opcode: %s \n",opcode);
	else
	{
	machine_opcode=0xe000;
	mc[0]=((atoi(arg0))<<10);
	mc[1]=((atoi(arg1))<<7);
	mc[2]=((atoi(arg2))<<0);
	mc[3]=(mc[2]|mc[1]|mc[0]|machine_opcode);
	fprintf(outFilePtr,"%04x \n",mc[3]);	
	}
	}

	// HALT 


	else if(strcmp(opcode,"halt")==0)fprintf(outFilePtr,"%04x \n",0xe071);

	// . FILL 
	
	else if(strcmp(opcode,".fill")==0)
	{
	if((arg0==NULL)||(strcmp(arg0,"#")==0))
    printf("There is no argument for the opcode: %s \n",opcode);
	else
	{
	if((check=isNumber(arg0))==1)fprintf(outFilePtr,"%04x \n",(atoi(arg0)&0xffff));
	else
	{
	     templabel = front;
         k = 0;
         while(templabel != NULL)                   
         {
                    if (strcmp(arg0,(templabel->name))!=0)
                    {
                    k++;
                    }
                    else
                    {
                        break;
                    }
                    templabel = templabel->next;
         }
	if(k==labelcount)
    printf("label \"%s\" does not exist!\n",arg0);
	else
	{
	fprintf(outFilePtr,"%04x \n",((templabel->index)&0xffff));
	}
	}
	}	
	}

	// NOP

	else if(strcmp(opcode,"nop")==0) 
    fprintf(outFilePtr,"%04x \n",0x0000);
	else if(opcode!=NULL)printf(" \"%s\":not a valid opcode\n",opcode);

	pc_count++;
	}

	fclose(inFilePtr);
	fclose(outFilePtr);
	system("pause");
	return 0;
}
