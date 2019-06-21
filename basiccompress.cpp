/*
BASIC COMPRESSION
READ FROM FILE 'data.txt'
*/
#include<stdio.h>
#include<math.h>
#include<string.h>
#include<stdlib.h>

//Use this to define maximum number of unique characters
#define MAX_UNIQ_CHARS 40

//Use this to print a byte in bit form

#define BYTE(byte)  printf("%c%c%c%c%c%c%c%c",\
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0'))

char a[MAX_UNIQ_CHARS];
int count=0;
int bit_count=0;
char *code[MAX_UNIQ_CHARS];
unsigned char byte = 0;

//Header Structures
struct map{
	char ch;
	char *code;
};
struct header{
	int size, no_of_char;
};

//Converts int to binary(int)
int tobinary(int n)
{
    int bin = 0;
    int rem, i = 1, step = 1;

    while (n!=0)
    {
        rem = n%2;
        n /= 2;
        bin += rem*i;
        i *= 10;
    }
    return bin;
}

//Appends unique characters
void inlist(char x)
{
    int flag=0,i=0;
    for(i=0;i<count;i++)
        if(a[i]==x)
        {
            flag=1;
            return;
        }
    if(flag==0)
        count++;
    a[count-1]=x;
}

//Mapping functions
//char -> code
char * map(char c)
{
	int i;
	for(i=0;i<count;i++)
		if(a[i]==c)
			return code[i];
}
//code -> char
char inmap(char *c,struct map* M)
{
	int i;
	for(i=0;i<count;i++)
		if(strcmp(M[i].code,c)==0)
			return M[i].ch;
	return '\0';
}

//write a bit to file (accumulates bits to form a byte)
void write_bit(FILE* p,char ch,int n)
{
	int x,i;
	char *text=map(ch);
	//printf("\nEncoding %c as %s\n",ch,text);
	for (i=0;i<strlen(text);i++)
	{
		if(text[i]=='1')
			x=1;
		else
			x=0;
		byte=byte<<1;
		byte|=x;
		bit_count++;
		if(bit_count==8)
		{
			//BYTE(byte);
			fwrite(&byte,sizeof(byte),1,p);
			bit_count=0;
			byte=0;
		}
	}
}

//Assigns codes for unique characters
void assign_code(int n)
{
    int fill,i;
	for(i=0;i<count;i++)
    {
    	if(i==0)
    		fill=n-1;
    	else
			fill=n-(floor(log2f(i))+1);
    	//printf("log val %d %d",i,floor(log2f(i)));
		code[i]=(char*)malloc(n*sizeof(char));
        memset(&code[i][0],'0',fill);
		sprintf(code[i]+fill, "%d", tobinary(i))		;
		//printf("\nfill = %d Code Mapping = %c - %s\n",fill,a[i],code[i]);
    }

}

main()
{

/*ENCODING PART*/

    FILE *f=fopen("data.txt","rb");
	FILE *g=fopen("entry.txt","wb");

	int n,s=0;
	char temp;
	struct header H;
	struct map* M;

    while((temp=fgetc(f))!=EOF)
    {
        //printf("%c",temp);
        s++;
        inlist(temp);
    }
    rewind(f);
    printf("\nUnique characters:\t%d\n",count);

	//No. of bits required is calculated
	n=(floor(log2f(count))+1);
	printf("\nNo.of bits required:\t%d",n);

    assign_code(n);

    H.no_of_char=count;
	H.size=s;
	M=(struct map*)malloc(count*sizeof(struct map));

	int i;
	for(i=0;i<count;i++)
 	{
 		M[i].ch=a[i];
		M[i].code=code[i];
	}

	//header is written first to output file
	fwrite(&H,sizeof(struct header),1,g);
	fwrite(M,sizeof(struct map),count,g);

	//encode the file content and write onto output file
	while((temp=fgetc(f))!=EOF)
        write_bit(g,temp,n);

	//remaining bits are written
	if(bit_count!=0)
	{
		int i;
		for(i=0;i<8-bit_count;i++)
		{
			byte=byte<<1;
			byte|=0;
		}
//		BYTE(byte);
		fwrite(&byte,sizeof(byte),1,g);
	}

	fclose(f);
	fclose(g);
	printf("\nFile is compressed..\n");

/*DECODING PART*/

	printf("\nReading from compressed file..\n");

	FILE *e=fopen("entry.txt","rb");
	while((temp=fgetc(f))!=EOF)
        printf("%c",temp);
    rewind(e);

	unsigned char read;
	unsigned char mask=0x80;
	int S=0;

	//Headers are read from file
	fread(&H,sizeof(struct header),1,e);
	int ct=H.no_of_char;
	int bit_size=(floor(log2f(ct))+1);
	int size=H.size;

	M=(struct map*)malloc(ct*sizeof(struct map));
	fread(M,sizeof(struct map),ct,e);

	printf("\nData obtained:\n no. of uniques:\t %d\nSize:\t%d\n",H.no_of_char,H.size);

	int bits=0;
	char *trans=(char*)malloc(bit_size*sizeof(char));


	printf("\n\nDecompressd file content:\n");

	//Data content is read and decoded
	while(!feof(e))
    {
    	read=0;
        fread(&read,sizeof(read),1,e);
		//BYTE(read);
		for(int i=0;i<8;i++)
        {
        	if(bits>=size*bit_size)
				break;
        	trans[bits%bit_size]=(char)((read&mask)>>7)+'0';
        	bits++;
			read=read<<1;
			if(bits%bit_size==0)
        	{
				trans[bit_size]='\0';
				temp=inmap(trans,M);
				printf("%c",temp);
			}
		}
    	S++;
	}

    int calc = 2*sizeof(int)+count*(sizeof(char)+n*sizeof(char))+ceil((bit_count*size)/8.0)+1;
	//+1 due to ending with null character

    int org = size*sizeof(char);

    //extra bits for struct is alloacted in the file, idk why.

	printf("\nNo.of bytes,\n\tIn compressed file:\t%d\n\tIn Original file:\t%d.",calc,org);
    printf("\nPercentage Compressed:\t%f%%",((org-calc*1.0)/org)*100);
    fclose(e);
}
