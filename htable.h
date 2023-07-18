#ifndef HTABLE_H
#define HTABLE_H

struct fchar
{
	int c;
	int freq;
	struct fchar *next;
	struct fchar *left;
	struct fchar *right;
	char *path;
};


struct fchar *ordered_add(struct fchar *start, struct fchar *parent);

int cmpfnc(const void *ch1, const void *ch2);

struct fchar *freqs(char * filename);

struct fchar *build_list(struct fchar *arr);

struct fchar *build_tree(struct fchar *list);

void DFS(struct fchar *root, char *pth, int i, struct fchar *freq_list);

void free_tree(struct fchar *root);

#endif
