/******************************************************************************************************
Name   : Scheduler - OS Lab 2
Author : Aniket Tushar Ajagaonkar
Date   : 2/24/2013
Email  : aniket [at] nyu [dot] edu
This is a program that simulates the working of the scheduler. 
Run the file scheduler.c providing 2 parameters at the input. First is the name of the input file and second is the type of scheduling algorithm to be used.
0 = First Come First Serve
1 = Round Robin
2 = Shortest Remaining Job First
*******************************************************************************************************/

#include<stdio.h>
#include<string.h>
#include<math.h>
#define N 10 		//Max number of processes that can be allowed at a time.

//Map entry at position 0 indicates the process id. 
//Map entry at position 1 indicates the CPU time needed by the process.
//Map entry at position 2 indicates the io time needed by the process.
//Map entry at position 3 indicates the arrival time of the process.
//Map entry at position 4 indicates the number of CPU cycles completed by the process or the number of CPU cycles left by the process.


//Structure of the queue.
struct queue
{
	int arr[10];
	int rear,front;
};

typedef struct queue QUEUE;

//Function for inserting into the queue.
void insertQ(QUEUE *q,int p)
{
	if(q->rear==N)
	{
		printf("Queue limit exceeded. Reduce the number of processes and restart\n");
		return;			
	}
	else
	{
		q->rear=(q->rear+1)%N;
		q->arr[q->rear]=p;
	}
}

//Function for removing an element from the queue.
int removeQ(QUEUE *q)
{
	if(q->rear==q->front)
	{
		printf("ERROR : No Process in the queue\n");
		return -1;
	}
	else
	{
		q->front=(q->front+1)%N;
		return q->arr[q->front];
	}
}

//Function for returning the element at the front end of the list.

int peekFront(QUEUE q)
{
	if(q.front==q.rear)
	{
		return -1;
	}
	else
	{
		return q.arr[q.front+1];
	}
}

void printQ(QUEUE q)
{
	printf("\nQueue\t");
	while(q.front!=q.rear)
	{	
		q.front=(q.front+1)%N;
		printf("%d\t",q.arr[q.front]);
	}
	printf("\n");
}

//Function to extract a number from the input char string. The number is represented in int form for further calculations.
	
int getNumber(char s[], int offset,int *num)
{
	int i=offset;
	int n=strlen(s);
	*num=0;

	while(i<n && s[i]!=' ' && s[i]!='\n' && s[i]>='0' && s[i]<='9') 
	{

		*num=(*num)*10 + (s[i]-'0');
		i++;
	}
	return i+1;
}

//Function that for setting the format of the input file into the expected file format.

void reformatFile(char filename[])
{
	FILE *fw = fopen("temp.txt","w");
	FILE *fr = fopen(filename,"r");
	char str[100],tgt_str[50]="";
	int i,num,count=0,n,j;
	
	while(!feof(fr))
	{
		fgets(str,100,fr);
		n=strlen(str);
		
		for(i=0;i<n;)
		{	
			j=getNumber(str,i,&num);

			if(j!=i+1)
			{
				count++;
				fprintf(fw,"%d ",num);
		
				if(count==4)
				{
					fprintf(fw,"\n");
					count=0;
					strcpy(str,"");
				}
			}
			i=j;
		}
	}
	fclose(fr);
	fclose(fw);
	
	remove(filename);
	rename("temp.txt",filename);
}
	
//Function used for printing the current state of each process in the system.

void printMap(int map[N][5],int proc_no)
{
	int i;
	for(i=0;i<proc_no;i++)
		printf("%d\t%d\t%d\t%d\t%d\n",map[i][0],map[i][1],map[i][2],map[i][3],map[i][4]);
}

//Function to process the input file. It processes the input string to identify the Process id, CPU cycles, IO time and arrival time.

void generateMap(int map[N][5],char proc[],int proc_no)
{
	int i=0;
	int n=strlen(proc);
	int num,count=0,temp;
	for(i=0;i<n;)
	{
		i=getNumber(proc,i,&num);
		if(count==1)
		{
			map[proc_no][count]=num+num%2;
			temp=num;
		}
		else map[proc_no][count]=num;
		if(count==2 && num==0)
			map[proc_no][count-1]=temp;
		count++;
	}
	map[proc_no][4]=0;
	
}

//Function to check if any process exists in the block state.

int checkBlocked(int blocked[],int proc_count)
{
	int i=0;
	for(i=0;i<proc_count;i++)
	{
		if(blocked[i]!=-1)
			return 0;
	}
	return 1;
}

//Function used to sort the process so that the process with lower id is ahead in the queue.

void sort_proc(int new_proc[],int k)
{
	int i=0,j=0,t;
	for(i=0;i<k;i++)
	{
		for(j=0;j<k-i-1;j++)
		{
			if(new_proc[j]>new_proc[j+1])
			{
				t=new_proc[j];
				new_proc[j]=new_proc[j+1];
				new_proc[j+1]=t;
			}
		}
	}
}

//Function to print the output to the file.

void printProcessStateToFile(char filename[],int process_state[],int proc_count,int cur_time)
{
	FILE *fp=fopen(filename,"a");
	int i;
	fprintf(fp,"%d ",cur_time);
	for(i=0;i<proc_count;i++)
	{	
		switch(process_state[i])
		{
			case 1:fprintf(fp,"%d:running ",i);break;
			case 2:fprintf(fp,"%d:ready ",i);break;
			case 3:fprintf(fp,"%d:blocked ",i);break;
		}
	}
	fprintf(fp,"\n");
	fclose(fp);
}

//Function to schedule the processes using first come first serve algorithm.

void firstComeFirstServe(int map[N][5],int proc_count,char filename[])
{
	QUEUE q;
	int i,p,cur_time=0,temp,c,k=0;
	int blocked[N]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	int new_proc[10];
	int cpu_util=0;
	int turn_around[N];
	int proc_added=0;

	FILE *fp=fopen(filename,"w");
	fclose(fp);
	
	q.rear=q.front=-1;	
	
	while(1)
	{	
		int process_state[N]={0};
		k=0;
		for(i=0;i<proc_count;i++)
		{
			if(cur_time==map[i][3])
			{
				new_proc[k++]=i;
				proc_added++;
			}
		}
		for(i=0;i<proc_count;i++)
		{
			if(blocked[i]==cur_time)
			{
				new_proc[k++]=i;
				blocked[i]=-1;
			}
		}
		sort_proc(new_proc,k);
		for(i=0;i<k;i++)
			insertQ(&q,new_proc[i]);
		if(q.rear!=q.front)
		{
			p=peekFront(q);
			process_state[p]=1;		
			cpu_util++;
			map[p][4]++;
			if(map[p][4]==(map[p][1])/2 && map[p][2]!=0)
			{
				p=removeQ(&q);
				blocked[p]=map[p][2]+cur_time+1;
			}
			else if(map[p][4]==map[p][1])
			{	
				removeQ(&q);
				turn_around[p]=cur_time;
			}
				
		}
		temp=q.front;
		while(temp!=q.rear)
		{
			temp=(temp+1)%N;
			if(q.arr[temp]!=p)
				process_state[q.arr[temp]]=2;
			
		}
		for(i=0;i<proc_count;i++)	
		{
			if(blocked[i]!=-1 && i!=p)
			{
				process_state[i]=3;
			}
		}

		c=checkBlocked(blocked,proc_count);

		printProcessStateToFile(filename,process_state,proc_count,cur_time);
				
		if(q.rear==q.front && c && proc_added==proc_count)
			break;
			
		cur_time++;
		p=-1;
	}
	
	fp=fopen(filename,"a");
	fprintf(fp,"\n");
	fprintf(fp,"Finishing Time : %d\n",cur_time);
	fprintf(fp,"CPU utilization : %3.2f\n",(float)cpu_util/(cur_time+1));
	for(i=0;i<proc_count;i++)
	{
		temp=turn_around[i]-map[i][3]+1;
		fprintf(fp,"Turnaround process %d : %d\n",i,temp);
	}
	fclose(fp);
}

//Function to schedule the processes using round robin algorithm.

void roundRobin(int map[N][5],int proc_count,char filename[])
{
	QUEUE q;
	int cur_time=0;
	int new_proc[N];
	int i,k,quantum_used,p,temp,c,cpu_util=0;
	int blocked[N]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	int turn_around[N];
	int proc_added=0;
	q.rear=q.front=-1;
	
	FILE *fp=fopen(filename,"w");
	fclose(fp);
	
	while(1)
	{
		int process_state[N]={0};		
		k=0;
		for(i=0;i<proc_count;i++)
		{
			if(cur_time==map[i][3])
			{
				new_proc[k++]=i;
				proc_added++;
			}
		}
		
		for(i=0;i<proc_count;i++)
		{
			if(blocked[i]==cur_time)
			{
				new_proc[k++]=i;
				blocked[i]=-1;
			}
		}
		
		sort_proc(new_proc,k);
		
		for(i=0;i<k;i++)
			insertQ(&q,new_proc[i]);
		
		if(q.rear!=q.front)
		{
			p=peekFront(q);
			process_state[p]=1;
			cpu_util++;
			quantum_used=(quantum_used+1)%2;
			map[p][4]++;
			
			if(map[p][4]==map[p][1])
			{
				removeQ(&q);
				quantum_used=0;
				turn_around[p]=cur_time;
			}
			if(quantum_used==0 && map[p][4]!=map[p][1] && map[p][4]!=(map[p][1])/2)
			{
				p=removeQ(&q);
				quantum_used=0;
				insertQ(&q,p);
			}
			
			if(map[p][4]==(map[p][1])/2)
			{
				p=removeQ(&q);
				quantum_used=0;
				blocked[p]=map[p][2]+cur_time+1;
			}
		}
			
		temp=q.front;
		while(temp!=q.rear)
		{
			temp=(temp+1)%N;
			if(q.arr[temp]!=p)
				process_state[q.arr[temp]]=2;
		}
		for(i=0;i<proc_count;i++)	
		{
			if(blocked[i]!=-1 && i!=p)
			{
				process_state[i]=3;
			}
		}	
		
		printProcessStateToFile(filename,process_state,proc_count,cur_time);
		
		c=checkBlocked(blocked,proc_count);
			
		if(q.front==q.rear && c && proc_added==proc_count)
			break;
		
		cur_time++;
		p=-1;
	}
	fp=fopen(filename,"a");
	fprintf(fp,"\n");
	fprintf(fp,"Finishing Time : %d\n",cur_time);
	fprintf(fp,"CPU utilization : %3.2f\n",(float)cpu_util/(cur_time+1));
	for(i=0;i<proc_count;i++)
	{
		temp=turn_around[i]-map[i][3]+1;
		fprintf(fp,"Turnaround process %d : %d\n",i,temp);
	}			
	fclose(fp);
}

//Function to schedule the processes using shortest remaining job first algorithm.

void shortestRemainingJobFirst(int map[N][5],int proc_count,char filename[])
{
	int i,cur_time=0,k=0,j,min,temp;
	int ready[N],c;
	int blocked[N]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	int cpu_util=0;
	int turn_around[N];
	int proc_added=0;
	
	FILE *fp=fopen(filename,"w");
	fclose(fp);

	for(i=0;i<proc_count;i++)
	{
		map[i][4]=map[i][1];
	}
	
	while(1)
	{
		int process_state[N]={0};
		for(i=0;i<proc_count;i++)
		{
			if(cur_time==map[i][3])
			{
				ready[k++]=i;
				proc_added++;
			}
		}
		
		for(i=0;i<proc_count;i++)
		{
			if(blocked[i]==cur_time)
			{
				ready[k++]=i;
				blocked[i]=-1;
			}
		}
		
		if(k>0)
		{
			min=ready[0];
			j=0;
			for(i=0;i<k;i++)
			{
				if(map[ready[i]][4]<map[min][4] || (map[ready[i]][4]==map[min][4] && ready[i]<min))
				{
					min=ready[i];
					j=i;
				}
			}
			
			temp=ready[k-1];
			ready[k-1]=ready[j];
			ready[j]=temp;
			
			process_state[min]=1;
			map[min][4]--;
			cpu_util++;
			
			if(map[min][4]==0)
			{
				k--;
				turn_around[min]=cur_time;
			}
			if(map[min][4]==(map[min][1])/2)
			{
				blocked[min]=map[min][2]+cur_time+1;
				k--;
			}
		}
			
		for(i=0;i<k;i++)
		{
			if(ready[i]!=min)
				process_state[ready[i]]=2;
		}
		for(i=0;i<proc_count;i++)
		{
			if(blocked[i]!=-1 && i!=min)
				process_state[i]=3;
		}
			
		printProcessStateToFile(filename,process_state,proc_count,cur_time);
		
		c=checkBlocked(blocked,proc_count);
		
		if(k==0 && c && proc_added==proc_count)
			break;
				
		cur_time++;
		min=-1;
		
	}
	fp=fopen(filename,"a");	
	fprintf(fp,"\n");
	fprintf(fp,"Finishing Time : %d\n",cur_time);
	fprintf(fp,"CPU utilization : %3.2f\n",(float)cpu_util/(cur_time+1));
	for(i=0;i<proc_count;i++)
	{
		temp=turn_around[i]-map[i][3]+1;
		fprintf(fp,"Turnaround process %d : %d\n",i,temp);
	}
	fclose(fp);			
}
		
		
int main(int argc,char *argv[])
{
	char filename[20],output_file[20];
	FILE *fp;
	char proc[20];   
	int map[10][5]={{0}};	
	int proc_no=0,n;
	int sch_type,i=0;

	if(argc==3)
	{
		strcpy(filename,argv[1]);
		sch_type=*argv[2]-'0';
	}
	else
	{
		printf("Invalid number of arguments\n");
		return 0;
	}
	
	fp=fopen(filename,"r");
	if(fp==NULL)
	{
		printf("File not found\n");
		return 0;
	}
	n=strlen(filename);
	while(i<n && filename[i]!='.')
	{
		output_file[i]=filename[i];
		i++;
	}
	strcat(output_file,"-");
	strcat(output_file,argv[2]);
	strcat(output_file,".txt");
	
	fclose(fp);
	reformatFile(filename);
	fp=fopen(filename,"r");
	while(!(feof(fp)))
	{
		strcpy(proc,"");
		fgets(proc,20,fp);
		n=strlen(proc);

		if(n!=0)
		{
			generateMap(map,proc,proc_no);
			proc_no++;
		}

	}
	fclose(fp);
	
	switch(sch_type)
	{
		case 0 : firstComeFirstServe(map,proc_no,output_file); break;
		case 1 : roundRobin(map,proc_no,output_file);break;
	 	case 2 : shortestRemainingJobFirst(map,proc_no,output_file); break;
	 	default : printf("Invalid type of scheduling selection");
	 }

	printf("Output written to the file %s",output_file);
	printf("\n");
	return 0;
}
