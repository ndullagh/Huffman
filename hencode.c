#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include "htable.h"
	
#define BYTE_MAX 256
#define PATH_SIZE 50
#define BYTE_SIZE 8
#define INT_SIZE 4

struct man_byte
{
	char byte;
	int pos;

};

extern int count_uniq(struct fchar *unsort);
struct man_byte out_byte(char *path, struct man_byte in_byte, int w_fd);


/*prints out the header of the file*/
void print_header(struct fchar *unsort, char *outfile)
{
	int fd;
	uint8_t *charbuff;
	
	uint32_t *intbuff;

	int i;
	charbuff = malloc(1 * sizeof(uint8_t));
	intbuff = malloc(1 * sizeof(uint32_t));
	if(outfile == NULL)
	{
		fd = 1; /*stdout*/ 
		/*This was written before I knew about STDOUT_FILENO*/
	}
	else
	{
		fd = open(outfile, O_WRONLY | O_TRUNC 
				| O_CREAT, S_IRUSR | S_IWUSR);
		if(fd == -1)
		{
			perror("Open failed.");
			exit(EXIT_FAILURE);
		}
	}
		
	*charbuff = (uint8_t)count_uniq(unsort);

	
	if(write(fd, charbuff, 1) == -1)
	{
		perror("Write failed.");
		exit(EXIT_FAILURE);
	}
	/*write number of uniq characters*/
	
	/*write each char then 4 byte frequency*/
	for(i = 0; i < BYTE_MAX; i++)
	{
		if(unsort[i].freq > 0)
		{
			*charbuff = (uint8_t)unsort[i].c;
			*intbuff = (uint32_t)unsort[i].freq;
			*intbuff = htonl(*intbuff);
			write(fd, charbuff, 1);
			write(fd, intbuff, INT_SIZE);
		}
	}
	
	free(charbuff);
	free(intbuff);	
	
}



/*prints out the body of the file*/
void print_body(struct fchar *unsort, char *arg, char *arg2)
{
	char cur_read;
	int was_read;
	char *cur_path;
	struct man_byte cur_byte;
	int r_fd = open(arg, O_RDONLY);
	int w_fd;
	
	cur_byte.byte = 0;
	cur_byte.pos = 0;
	if(arg2 == NULL)
	{
		/*stdout*/
		w_fd = 1;
	}
	else
	{
		w_fd = open(arg2, O_WRONLY | O_APPEND);
		if(w_fd == -1 || r_fd == -1)
		{
			perror("Open failed.");
			exit(EXIT_FAILURE);
		}
	}
	
	was_read = read(r_fd, &cur_read, 1);
	if(was_read == -1)
	{
		perror("Read failed.");
		exit(EXIT_FAILURE);
	}
	while(was_read == 1)
	{
		/*essentially just a runner for out_byte*/
		cur_path = unsort[(uint8_t)cur_read].path;
			
		cur_byte = out_byte(cur_path, cur_byte, w_fd); 

		was_read = read(r_fd, &cur_read, 1);
		if(was_read == -1)
		{
			perror("Read failed.");
			exit(EXIT_FAILURE);
		}
		
	}

	if(cur_byte.pos != 0)
	{
		if(write(w_fd, &(cur_byte.byte), 1) == -1)
		{
			perror("Write failed.");
			exit(EXIT_FAILURE);
		}
	}	

	
}

/*returns new current output byte*/
/*if still in same byte, return that byte*/
/*if overflowed into new byte, prints the finished
 * byte and returns the new unfinished one*/
/*if it finishes a byte, writes it to the output*/
struct man_byte out_byte(char *path, struct man_byte in_byte, int w_fd)
{
	int cur = 0; /*where we are in the path*/
	int pathlen = strlen(path);
	struct man_byte output;

	output = in_byte;
	
	while(output.pos < BYTE_SIZE && cur < pathlen)
	{
		if(path[cur] == '1')
		{
			output.byte = output.byte |
			 (1 << (BYTE_SIZE - output.pos - 1));
		}
		output.pos++;
		cur++;	
	
		if(output.pos == BYTE_SIZE)
		{
			if(write(w_fd, &(output.byte), 1) == -1)
			{
				perror("Write failed.");
				exit(EXIT_FAILURE);
			}
			output.byte = 0;
			output.pos = 0;
		}
		if(cur == pathlen)
		{
			  return output;
		}

		
	}
	return output;
}
	

int main(int argc, char *argv[])
{		
	struct fchar *linked_list;
	int i;
	char * arg;
	char *arg2;
	FILE *file;
	struct fchar *treeptr;
	char *temppath;
	struct fchar *freq_list;
	struct fchar *unsort;
	
	arg2 = NULL;
	
	arg = argv[1];
	file = fopen(arg, "r");
	if(file == NULL)
	{
		perror(arg);
		exit(EXIT_FAILURE);
	}
	fclose(file);
	if(argc > 2)
	{
		arg2 = argv[2];
	}

	freq_list = freqs(arg);
	
	qsort(freq_list, BYTE_MAX, sizeof(struct fchar), cmpfnc);
	
	linked_list = build_list(freq_list);
		
	treeptr = build_tree(linked_list);

	unsort = freqs(arg);

	temppath = calloc(PATH_SIZE, sizeof(char));
	
	DFS(treeptr, temppath, 0, unsort);
	
	
	free(freq_list);
	free(temppath);

	print_header(unsort, arg2);

	print_body(unsort, arg, arg2);
	for(i = 0; i < BYTE_MAX; i++)
	{
		free(unsort[i].path);
	}
	free(unsort);
	free_tree(treeptr);
	return 0;
}
