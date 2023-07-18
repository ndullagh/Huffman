#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include "htable.h"

#define BUFF_SIZE 1000
#define BYTE_MAX 256
#define BYTE_LEN 8

struct pos
{
	int bytepos;
	int bitpos;
};

struct pos out_char(struct fchar *treepos, 
	struct fchar *treeptr, struct pos buffpos, 
	int w_fd, char *bytebuff, int *numreadptr, int r_fd);
extern int count_uniq(struct fchar *unsort);
int count_total(struct fchar *freq_list);
/*if empty file, returns 1*/
int decode_header(int r_fd, struct fchar *freq_list)
{
	int uniq;
	uint8_t *bytebuff;
	int numread;
	int i;
	int curc;
	uint32_t curfreq;
	uint32_t freqbyte;

	uniq = 0;
	curfreq = 0;
	numread = read(r_fd, &uniq, 1);
	if(numread == 0)
	{
		return 1;
	}
	if(numread == -1)
	{
		perror("Read Failed.");
		exit(EXIT_FAILURE);
	}

	bytebuff = calloc((uniq + 1) * 5, sizeof(uint8_t));
	
	if(read(r_fd, bytebuff, (uniq + 1) * 5) == -1)
	{
		perror("Read failed.");
		exit(EXIT_FAILURE);
	}
	i = 0;
	while(i < (uniq + 1) * 5)
	{
		curfreq = 0;
		curc = bytebuff[i];
		freqbyte = bytebuff[i + 1];
		/*shift the 1 to the correct position*/
		curfreq = curfreq | (freqbyte << BYTE_LEN * 3);
		freqbyte = bytebuff[i + 2];
		curfreq = curfreq | (freqbyte << BYTE_LEN * 2);
		freqbyte = bytebuff[i + 3];
		curfreq = curfreq | (freqbyte << BYTE_LEN);
		freqbyte = bytebuff[i + 4];
		curfreq = curfreq | (freqbyte);
		
		freq_list[curc].c = curc;
		freq_list[curc].freq = curfreq;
		i+=5;
	}
	free(bytebuff);	
	return 0;
}

/*output the actual contents of the file*/
int print_contents(int r_fd, int w_fd, 
	struct fchar *treeptr, struct fchar* freq_list)
{
	char *bytebuff = malloc(BUFF_SIZE *sizeof(char));
	int numread;
	int cur = 0;
	struct fchar *treepos = treeptr;
	struct pos buffpos;
	int ttl;
	int *numreadptr = &numread;
	int i;
	int j;

	/*set read pos to after the header*/
	lseek(r_fd, ((count_uniq(freq_list) + 1) * 5) + 1, SEEK_SET);
	numread = read(r_fd, bytebuff, BUFF_SIZE);
	if(numread == -1)
	{
		perror("Read Failed.");
		exit(EXIT_FAILURE);
	}
	ttl = count_total(freq_list);
	/*case for only one unique char*/
	if(count_uniq(freq_list) == 0)
	{
		for(i = 0; i < BYTE_MAX; i++)
		{
			if(freq_list[i].freq > 0)
			{
				free(bytebuff);
				bytebuff = malloc(ttl * sizeof(char));
				/*output the character totalcount times*/
				for(j = 0; j < ttl; j++)
				{
					bytebuff[j] = freq_list[i].c;
				}
				if(write(w_fd, bytebuff, ttl) == -1)
				{
					perror("Write failed.");
					exit(EXIT_FAILURE);
				}
				free(bytebuff);
				return 0;
			}
		}
	}
	/*otherwise...*/
	while(1)
	{
		buffpos.bytepos = 0;
		buffpos.bitpos = 0;
		/*continue end of buff or out of chars to write*/
		while(buffpos.bytepos < numread && cur < ttl)
		{
		
			buffpos = out_char(treepos, treeptr, 
				buffpos, w_fd, bytebuff, numreadptr, r_fd);
			cur++;
			
		}
		numread = read(r_fd, bytebuff, BUFF_SIZE);
		if(numread == 0 || cur == ttl)
		{
			break;
		}
		if(numread == -1)
		{
			perror("Read fail");
			exit(EXIT_FAILURE);
		}
		
	}
	free(bytebuff);
	return 0;	
}


struct pos out_char(struct fchar *treepos, 
	struct fchar *treeptr, struct pos buffpos, int w_fd, char *bytebuff,
	int *numreadptr, int r_fd)
{
	char *writebuff = malloc(1 * sizeof(char));
	while(1)
	{
		if(treepos -> right == NULL && treepos -> left == NULL)
		{
			*writebuff = treepos -> c;
			if(write(w_fd, writebuff, 1) == -1)
			{
				perror("Write failed.");
				exit(EXIT_FAILURE);
			}
			free(writebuff);
			return buffpos;
		}
		/*mask to check the current bit with bitwise and*/
		if((bytebuff[buffpos.bytepos] 
				& (1 << (BYTE_LEN - (buffpos.bitpos) - 1)))
				== (1 << (BYTE_LEN - (buffpos.bitpos) - 1)))
		{
			treepos = treepos -> right;
		}
		else
		{
			treepos = treepos -> left;
		}
		buffpos.bitpos++;
		/*if end of byte reached, increment byte*/
		if(buffpos.bitpos == 8)
		{
			(buffpos.bytepos)++;
			/*if end of buff reached, new buff*/
			if(buffpos.bytepos == BUFF_SIZE)
			{
				*numreadptr = read(r_fd, bytebuff, BUFF_SIZE);
				if(*numreadptr == 0)
				{
					free(writebuff);
					return buffpos;
				}
				if(*numreadptr == -1)
				{
					perror("Read Failed.");
					exit(EXIT_FAILURE);
				}
				buffpos.bytepos = 0;
			}

			buffpos.bitpos = 0;
		}
	}
	free(writebuff);
	return buffpos;
}

/*count total number of characters in the freq list*/
int count_total(struct fchar *freq_list)
{
	int i;
	int sum = 0;
	for(i = 0; i < BYTE_MAX; i++)
	{
		if(freq_list[i].freq > 0)
		{
			sum += freq_list[i].freq;
		}
	}
	return sum;
}

int main(int argc, char *argv[])
{
	int r_fd = 0;
	int w_fd = 1;
	struct fchar *freq_list = calloc(BYTE_MAX, sizeof(struct fchar));
	FILE *file;
	struct fchar *unsort;
	struct fchar *treeptr;
	struct fchar *linked_list;
	/*char *temppath;*/

	if(argc >= 2)
	{
		if((file = fopen(argv[1], "r")) != NULL 
					&& strcmp(argv[1], "-") != 0)
		{
			fclose(file);
			r_fd = open(argv[1], O_RDONLY);
			if(r_fd == -1)
			{
				perror("Open failed.");
				exit(EXIT_FAILURE);
			}
		}
		else if(fopen(argv[1], "r") == NULL 
					&& strcmp(argv[1], "-") != 0)
		{
			perror(argv[1]);
			exit(EXIT_FAILURE);
		}
	}
	if(argc >= 3)
	{
		
		w_fd = open(argv[2], O_WRONLY | O_TRUNC| 
				O_CREAT, S_IRUSR | S_IWUSR);
		if( w_fd == -1)
		{
			perror("Open failed.");
			exit(EXIT_FAILURE);
		}
	
	}

	if(decode_header(r_fd, freq_list) == 1)
	{
		free(freq_list);	
		exit(0);
	}
	
	unsort = malloc(BYTE_MAX * sizeof(struct fchar));
	memcpy(unsort, freq_list, BYTE_MAX * sizeof(struct fchar));

	qsort(freq_list, BYTE_MAX, sizeof(struct fchar), cmpfnc);
	linked_list = build_list(freq_list);
	treeptr = build_tree(linked_list);

	print_contents(r_fd, w_fd, treeptr, unsort);
	free(unsort);
	free(freq_list);
	free_tree(treeptr);	
	return 0;
}
