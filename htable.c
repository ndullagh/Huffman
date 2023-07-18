#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "htable.h"

#define BUFF_SIZE 100
#define BYTE_MAX 256
#define PAR -257
#define PATH_SIZE 50

/*compare by freq, break ties with char value*/
int cmpfnc(const void *ch1, const void *ch2)
{
	int ret;
	ret = ((struct fchar*)ch1) -> freq - ((struct fchar*)ch2) -> freq;
	if(ret == 0)
	{
		ret = (uint8_t)(((struct fchar*)ch1) -> c)
			 - (uint8_t)(((struct fchar*)ch2) -> c);
		/*ret = -1;*/
	}
	return ret;
}

/*int cmpfnc_ins(const void *ch1, const void *ch2)
{
	int ret;
	ret = ((struct fchar*)ch1) -> freq - ((struct fchar*)ch2) -> freq;
	if(ret == 0)
	{
		ret = ((struct fchar*)ch1) -> c - ((struct fchar*)ch2) -> c;
		ret = -1;
	}
	return ret;
}*/


/*reads from stdin*/
/*puts chars into a linked
 *list until no longer able
 *to read, returns pointer to the 
location the linked list starts at*/
struct fchar *freqs(char * filename)
{
	/*FILE *fptr;*/
	struct fchar *freq_arr;
	char *cur;
	char *readbuff;
	int count;
	int fd;
	int numread;
	
	freq_arr = calloc(BYTE_MAX, sizeof(struct fchar));

	fd = open(filename, O_RDONLY);

	readbuff = malloc(BUFF_SIZE * sizeof(char));

	numread = read(fd, readbuff, BUFF_SIZE);
	
	count = 0;
	
	for(cur = readbuff; count != numread; cur++)
	{
		freq_arr[(uint8_t)(*cur)].c = *cur;
		freq_arr[(uint8_t)(*cur)].freq++;	
		count++;		
	}
	while(numread == BUFF_SIZE)
	{
		numread = read(fd, readbuff, BUFF_SIZE);
		count = 0;
		for(cur = readbuff; count != numread; cur++)
		{
			freq_arr[(uint8_t)(*cur)].c = *cur;
			freq_arr[(uint8_t)(*cur)].freq++;
			count++;
		}
	}
	free(readbuff);	
	close(fd);
	return freq_arr;	

}

/*take anything with a freq >0 and append to linked list*/
/*return head of list*/
struct fchar *build_list(struct fchar *arr)
{
	int i;
	int j;
	struct fchar *start;
	struct fchar *cur;
	start = calloc(1, sizeof(struct fchar));
	
	cur = start;

	for(i = 0; i < BYTE_MAX; i++)
	{
		if(arr[i].freq > 0)
		{
			start -> freq = arr[i].freq;
			start -> c = arr[i].c;
			/*start -> next = NULL;
			start -> left = NULL;
			start -> right = NULL;*/
			break;
		}
	}
	for(j = i + 1; j < BYTE_MAX; j++)
	{
		cur -> next = calloc(1, sizeof(struct fchar));
		cur -> next -> c = arr[j].c;
		cur -> next -> freq = arr[j].freq;
		/*cur -> next -> next = NULL;
		cur -> next -> left = NULL;
		cur -> next -> right = NULL;*/
		cur = cur -> next;
	}
	return start;
}

/*pull out 2 nodes at a time, combine, and sort back into list*/
/*repeat until only one top node remaining*/
/*return root*/
struct fchar *build_tree(struct fchar *list)
{
	struct fchar *start; /*1st node in list*/
	struct fchar *rem; /*holds 2 removed nodes*/
	struct fchar *parent;

	start = list;
	while(start -> next != NULL)
	{
		rem = start;
		start = start ->next -> next;	
		
		parent = calloc(1, sizeof(struct fchar));
		
		parent -> c = PAR;
		parent -> left = rem;
		parent -> right = rem -> next;
		parent -> freq = (rem -> freq) + (rem -> next -> freq);
		
		rem -> next -> next = NULL;
		rem -> next = NULL;
		
		start = ordered_add(start, parent);
		
	}
	return start;
}

/*sort parent back into list (start = head node)*/
struct fchar *ordered_add(struct fchar *start, struct fchar *parent)
{
	struct fchar *cur; /*points at the node we're checking*/


	if(start == NULL || start -> freq >= parent -> freq)
	{
		parent -> next = start;
		start = parent; /*reassign start pointer to the parent*/
	}
	else
	{
		cur = start;
		while(cur -> next != NULL 
			&& cur -> next -> freq < parent -> freq)
		{
			cur = cur -> next;
		}
		parent -> next = cur -> next;
		cur -> next = parent;
		

	}
	return start;
	
}

/*recursive tree traversal, gives each struct in the freq_list a path*/
void DFS(struct fchar *root, char *pth, int i, struct fchar *freq_list)
{
	if(root == NULL)
	{
		return;
	}

	if(root -> left)
	{
      		pth[i] = '0';
		DFS(root -> left, pth, i + 1, freq_list);
	}

	if(root -> right)
	{
		pth[i] = '1';
		DFS(root -> right, pth, i + 1, freq_list);
		
	}

	if(!(root -> left) && !(root -> right))
	{
		pth[i] = '\0';
		freq_list[(uint8_t)(root -> c)].path 
			= calloc(PATH_SIZE, sizeof(char));
		strcpy(freq_list[(uint8_t)(root -> c)].path, pth);
		/*for(j = 0; j < strlen(pth) */
	}

}

void free_tree(struct fchar *root)
{
	if(root == NULL)
	{
		return;
	}
	free_tree(root -> left);
	free_tree(root -> right);
	free(root);
}

int count_uniq(struct fchar *unsort)
{
	int ret;
	struct fchar cur;
	int i;
	ret = 0;
	
	i = 0;
	while(i < BYTE_MAX)
	{
		cur = unsort[i];
		if(cur.freq > 0)
		{
			ret++;
		}
		i++;
	}
	
	if(ret == 0)
	{
		exit(0);
	}
	return ret - 1;
	
}


