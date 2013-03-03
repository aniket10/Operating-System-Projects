/******************************************************************************************************
Name   : Linkers - OS Lab 1
Author : Aniket Tushar Ajagaonkar
Date   : 2/5/2013
Email  : aniket [at] nyu [dot] edu
This is a program that simulates the working of the linker. 
Run this file in the terminal as gcc linker.c. It will prompt you to enter the filename. Enter the filename and hit enter. The output including the errors will be displayed on the output console.
*******************************************************************************************************/

#include<stdio.h>
#include<string.h>

//Flag is used to identify of any error check has detected an error in the input.
int error_flag=0;

//Struct is used to represent the symbol table. Mod indicates the module in which thw symbol is defined. Symbol is the actual symbol  and loc is the relocated address.

struct sym_table
{
	int mod;
	char symbol[10];
	int loc;
};
typedef struct sym_table SYM_TABLE;

//Struct is used to represent the symbols that are in the use list of each module.

struct use_list
{
	char symbol[10][10];
};
typedef struct use_list USE_LIST;

//Struct keeps the track of the code. mod indicates the module that the code belongs to. Symbol is one of A,I,E,R. address is the address specified in the input file. Later it is modified to represent relocation.

struct code_table
{
	int mod;
	char symbol;
	int address;
};
typedef struct code_table CODE_TABLE;

//Function to be used for removing additional spaces in the input line. This ensures that there is exactly 1 space between any 2 words.

void trim_input(char *str)
{
  int r; /* next character to be read */
  int w; /* next character to be written */

  r=w=0;
  while (str[r])
  {
    if (isspace(str[r]) || iscntrl(str[r]))
    {
      if (w > 0 && !isspace(str[w-1]))
        str[w++] = ' ';
    }
    else
      str[w++] = str[r];
    r++;
  }
  str[w] = 0;
}
		
//Function to extract a number from the input char string. The number is represented in int form for further calculations.		
			
int getNumber(char s[], int offset,int *num)
{
	int i=offset;
	int n=strlen(s);
	*num=0;
	while(i<n && s[i]!=' ' && s[i]!='\n') 
	{
		*num=(*num)*10 + (s[i]-'0');
		i++;
	}
	return i+1;
}

//Function to format the input file in the way that will make it easy to parse by the functions below. It generates a temp file and later deletes it. The formatting of the input file remains unaffected.

void correctIndentation(char filename[])
{
	FILE *fr=fopen(filename,"r");
	FILE *fw=fopen("temp.txt","w");
	int count=0;
	char s[100];
	int i;
	int line=0;
	int c;
	int endline=1,readline=1;
	int len;
	
	if(fr==NULL)
	{
		printf("File not found");
		return;
	}
	
	s[0]='\0';

	while(!feof(fr))
	{
		if(readline==1)
		{
			fgets(s,100,fr);
			trim_input(s);
			len=strlen(s);
			if(len==0)
				continue;
			i=0;
			readline=0;
		}
		
		
		if(endline==1)
		{
			line++;
			getNumber(s,i,&c);
			count=0;
			if(line%3==1 || line%3==0)
				c=2*c;
			endline=0;
		}
			
		while(s[i] && count<=c)
		{
			fputc(s[i],fw);
			if(s[i]==' ')
			{
				count++;
			}
			i++;
		}
		if(!s[i])
		{
			readline=1;
		}
		if(count>c)
		{
			fputs("\n",fw);
			endline=1;
		}
			
	}
	fputs("\n",fw);
	fclose(fr);
	fclose(fw);
		

}

//Function to check if a variable is previously defined. It is used for performing 2 error checks :- for checking multiple declarations of the symbol and if a variable is used but not defined.
		
int alreadyDefined(SYM_TABLE ins[], int sym_count,char s[])
{
	int i=0;
	for(i=0;i<sym_count;i++)
	{
		if(strcmp(ins[i].symbol,s)==0)
			return 1;
	}
	return 0;
}

//Function is used to insert the symbol in the symbol table. It is fired on the reading the def line of the module. It also performs error checking : If a variable is declared multiple times.

int insertSymbolTable(SYM_TABLE ins[10],char s[], int sym_count,int cur_offset,int m)
{	
	int i,j=0;
	int n =strlen(s);
	char temp[10]="";
	int val=0;
	i=getNumber(s,0,&val);

	if(val==0)
		return sym_count;

	for(;i<n;i++)
	{
		if(s[i]==' ')
		{
			strcpy(ins[sym_count].symbol,temp);
			
			if(alreadyDefined(ins,sym_count,ins[sym_count].symbol))
			{
				printf("ERROR : Multiple Declarations found for Symbol %s \n",ins[sym_count].symbol);
				error_flag=1;
			}

			i=getNumber(s,i+1,&val)-1;
			ins[sym_count].loc=val+cur_offset;
			ins[sym_count].mod=m;
			j=0;
			sym_count++;
			
		}
		else
		{
			temp[j]=s[i];
			j++;
			
		}
	}
	return sym_count;
}

//Function is used to create a list of all the symbols that are used in the module.

void makeUseList(USE_LIST use[],char s[],int mod)
{
	int num,i=0,n,j=0,k=0;
	char temp[10];
	k=getNumber(s,0,&num);
	n=strlen(s);
	if(num==0)
	{
		use[mod].symbol[0][0]='\0';
		return;
	}
	
	for(;k<=n;k++)
	{
		if(s[k]==' ' || s[k]=='\0')
		{
			strcpy(use[mod].symbol[i],temp);
			use[mod].symbol[i][j]='\0';
			i++;
			j=0;
		}
		else
		{
			temp[j]=s[k];
			j++;
		}
	}
	use[mod].symbol[i][0]='\0';
}

//Function for error checking. It checks if a symbol is used but not defined. It makes use of alreadyDefined(..) written above.

void checkDeclaration(SYM_TABLE ins[],USE_LIST use[],int mod,int sym_count)
{
	int i,j;
	for(i=0;i<mod;i++)
	{
		j=0;
		while(use[i].symbol[j][0]!='\0')
		{
			if(alreadyDefined(ins,sym_count,use[i].symbol[j])==0)
			{
				printf("ERROR : Variable %s is used but not defined\n",use[i].symbol[j]);
				error_flag=1;
			}
			j++;
		}
	}
}

//Function for error checking. It is used to ensure that the address appearing in the definition list doesnot exceed the size of the module.

void checkDefAddress(SYM_TABLE ins[],int off,int m,int sym_count)
{
	int i=0;
	for(i=0;i<sym_count;i++)
	{
		if(ins[i].mod==m && ins[i].loc-off > m)
		{
			printf("ERROR : Address %d appearing in the definition of %s exceeds the size %d of the module\n",(ins[i].loc-off),ins[i].symbol,m);
			error_flag=1;
		}
	}
}

//Fucntion used for error checking. It checks if the absolute address in the code block doesnot exceed the machine address of 600.

void checkForMacSize(CODE_TABLE code[],int n)
{
	int i;
	for(i=0;i<n;i++)
	{
		if((code[i].symbol=='A' || code[i].symbol=='R' || code[i].symbol=='E') && (code[i].address)%1000 > 600)
		{
			printf("ERROR : Absolute Address %d exceeds the machine address\n",code[i].address);
			error_flag=1;
		}
	}
}

//Function used to make the code table. It keeps a track of module, the symbol and the address for pass 1. It also performs error checking for 2 cases :- If the external address is too large to reference an entry in the uselist and relative address exceeds the size of the module.

int makeCodeTable(CODE_TABLE code[],char s[],int code_count,int lev,int use_num)
{
	int num,c;
	int i= getNumber(s,0,&c);
	int n=strlen(s);
	for(;i<n;)
	{
		code[code_count].mod=lev;
		code[code_count].symbol=s[i];
		i=getNumber(s,i+2,&num);
		code[code_count].address=num;
		
		if(code[code_count].symbol == 'E' && num%1000 > use_num-1)
		{
			printf("ERROR : External Address %d is too large to reference an entry in the use list\n",num);
			error_flag=1;
		}
		if(code[code_count].symbol == 'R' && num%1000 > c)
		{
			printf("ERROR : Relative address %d exceeds the size of module %d\n",num,lev);
			error_flag=1;
		}
		code_count++;
	}
	return code_count;
}

//Function to check if the variable declared in the uselist of the module is actually used in the module.

void inUseListNotUsed(USE_LIST use[],CODE_TABLE code[],int count,int code_count)
{
	int i,j,k;
	char temp[10];
	int l,flag=0;
	for(i=0;i<count;i++)
	{
		j=0;
		while(use[i].symbol[j][0]!='\0')
		{
			flag=0;
			for(k=0;k<code_count;k++)
			{		
				if(code[k].mod==i+1)
				{
					if(code[k].symbol=='E' && code[k].address%1000==j)
					{
						flag=1;					
						break;
					}
				}		
			}
			if(flag==0)
				{
					printf("WARNING : Symbol %s is declared in the use list but used not in the module %d\n",use[i].symbol[j],i+1);
				}
			j++;
		}
	}
}

//Function is used in pass 2 of the linker. It computes the absolute address for code address represented by R and E.

void getAbsoluteAddress(SYM_TABLE ins[], USE_LIST use[], CODE_TABLE code[], int code_count, int offset[],int sym_count)
{
	int i=0;
	for(i=0;i<code_count;i++)
	{
		if(code[i].symbol=='R')
		{
			code[i].address=code[i].address+offset[(code[i].mod)-1];
		}
		
		if(code[i].symbol=='E')
		{	
			int d=(code[i].address)%100;
			char *s=use[(code[i].mod)-1].symbol[d];
			int j,o;
			for(j=0;j<sym_count;j++)
			{
				if(strcmp(s,ins[j].symbol)==0)
				{
					o=ins[j].loc;
				}
			}
			code[i].address=code[i].address-(code[i].address%100)+o;
		}
	}
}

//Function used for checking if the variable that is defined is actually used or not.

void isUsed(SYM_TABLE ins[],USE_LIST use[],int sym_count,int use_count)
{
	int i=0,j,k=0;
	char s[10];
	int flag=0;
	for(i=0;i<sym_count;i++)
	{
		strcpy(s,ins[i].symbol);
		flag=0;
		for(j=0;j<use_count;j++)
		{
			k=0;
			while(use[j].symbol[k][0]!='\0')
			{
				if(strcmp(s,use[j].symbol[k])==0)
				{
					flag=1;
					break; 
				}
				k++;
			}
			if(flag==1)
				break;
		}
		if(flag==0)
		{
			printf("WARNING : Symbol %s defined but never used\n",s);		
		}
	}
}								
	
//Function used to print the symbol table.		

void printSymbolTable(SYM_TABLE ins[], int sym_count)
{
	int i;
	printf("Symbol Table\n");
	for(i=0;i<sym_count;i++)
	{
		printf("%s\t",ins[i].symbol);
		printf("%d\n",ins[i].loc);
	}
}

//NOT USED. Function used to print the symbols in the use list of each module.

void printUseList(USE_LIST use[],int n)
{
	printf("Printing the use list\n");
	int i,j;
	for(i=0;i<n;i++)
	{
		j=0;
		printf("%d\t",i+1);
		while(use[i].symbol[j][0]!='\0')
		{
			printf("%s",use[i].symbol[j]);
			j++;
		}
		printf("\n");
	}
}

//Function used to print the code table.

void printCodeTable(CODE_TABLE code[],int code_count)
{
	int i;
	printf("\nMemory Map\n");
	for(i=0;i<code_count;i++)
	{
		printf("%d : %d\n",i,code[i].address);
	}
}

int main()
{
	FILE * fp,*ftemp;
	char s[100];
	char filename[10];
	int offset[20]={0};		//Storing the offset for each module.
	int line=1;				//Represents the line number.
	SYM_TABLE ins[10];		//Storing the symbols defined. Program allows max of 10 symbols to be defined. Increase the array size 									for more symbols.
	USE_LIST use[10];		//Storing the symbols in the uselist. Program allows maximum of 10 modules in each input file. Increase 								the array size for more modules.
	CODE_TABLE code[50];	//Storing the code symbols and address. Program allows maximum of 50 address in the file. Increase the 									array size for more addresses.
	int code_count=0;		//Maintains the count of the code addresses defined.
	int sym_count=0;		//Maintains the count of the symbols defined.
	int num=0,use_num=0;
	int i;
	
	printf("Enter the filename : ");
	scanf("%s",filename);
	
	ftemp=fopen(filename,"a");
	fputs("\n",ftemp);
	fclose(ftemp);
	correctIndentation(filename);		//Correct the file indentation.
	
	fp = fopen("temp.txt","r");
	
	offset[0]=0;
	if(fp==NULL)						//In case of invalid file.
	{
		printf("Invalid file");
		return 0;
	}

	while(!feof(fp))
	{
		int len;
		fgets(s,100,fp);
		trim_input(s);
		len=strlen(s);
		if(len==0)
		{
			continue;
		}
	
		if((line%3)==1)						//If the line read represents a definition list 
		{	
			sym_count=insertSymbolTable(ins,s,sym_count,offset[line/3],line/3);
		}
		if(line%3==2)						//If the line read represents the use list
		{
			getNumber(s,0,&use_num);
			makeUseList(use,s,line/3);			
		}
		if(line%3==0)						//If the line read represents code.
		{
			getNumber(s,0,&num);
			offset[line/3]=offset[line/3-1]+num;
			code_count=makeCodeTable(code,s,code_count,line/3,use_num);
			checkDefAddress(ins,offset[line/3-1],num,sym_count);
 
		}
		line++;
	}
	
//Performing error checks.

	checkDeclaration(ins,use,line/3,sym_count);
	inUseListNotUsed(use,code,line/3,code_count);
	getAbsoluteAddress(ins,use,code,code_count,offset,sym_count);	
	checkForMacSize(code,offset[line/3]);

	
		
	if(error_flag==0)	//No errors found by the error checks.
	{
		printSymbolTable(ins,sym_count);
		printCodeTable(code,code_count);
	}
	isUsed(ins,use,sym_count,line/3);
	fclose(fp);
	remove("temp.txt");
	

	return 0;
}
