#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

struct node
{
	long src_node;
	long *dst_nodes;
	int dst_size;
	float pagerank;
	float pagerank_boost;
};

struct t_params
{
	int start;
	int end;
	int thread_number;
};

struct t_params par[4];

pthread_t threads[4];
struct node *srcs;
int srcs_size;
int num_of_threads;
pthread_mutex_t lock[4];

int find_struct_node(long val, struct node *arr, int arr_size)
{
	if(arr_size <= 0)	return -1;
	if(arr_size == 1)	return (arr[0].src_node == val) ? 0 : -1;
	int middle = arr_size / 2;
	if(arr[middle].src_node == val)	return middle;

	int ret;
	if(arr[middle].src_node > val)
	{
		ret = find_struct_node(val, arr, middle);
		if(ret != -1)	return ret;
	}
	else
	{
		ret = find_struct_node(val, arr + middle, arr_size - middle);
		if(ret != -1)	return middle + ret;
	}
	return -1;
}

void insert_dest_link(long val, int index, int start, int end)
{
	if(srcs[index].dst_size == 0)
	{
		srcs[index].dst_nodes = realloc(srcs[index].dst_nodes, ++srcs[index].dst_size * sizeof(long));
		srcs[index].dst_nodes[0] = val;
		return;
	}

	int middle = (start + end) / 2;
	if(srcs[index].dst_nodes[middle] == val)	return;
	if(start == end)
	{
		int direction;
		if(srcs[index].dst_nodes[middle] > val)	direction = 0;
		else	direction = 1;
		srcs[index].dst_nodes = realloc(srcs[index].dst_nodes, ++srcs[index].dst_size * sizeof(long));
		if(start < srcs[index].dst_size - 1)	memmove(srcs[index].dst_nodes + start + direction + 1, srcs[index].dst_nodes + start + direction, (srcs[index].dst_size - start - direction - 1) * sizeof(long));
		srcs[index].dst_nodes[start + direction] = val;
		return;
	}

	if(srcs[index].dst_nodes[middle] > val)
	{
		insert_dest_link(val, index, start, middle);
		return;
	}
	else
	{
		insert_dest_link(val, index, middle + 1, end);
		return;
	}
	//return (srcs[index].dst_nodes[middle] > val) ? insert_dest_link(val, index, start, middle) : insert_dest_link(val, index, middle + 1, end);
}

void insert_struct_node(long src, long dst, int start, int end)
{
	if(src < 0 || start < 0 || end > srcs_size - 1)	return;
	if(srcs_size == 0)
	{
		srcs = realloc(srcs, ++srcs_size * sizeof(struct node));
		srcs[0].src_node = src;
		if(dst >= 0)
		{
			srcs[0].dst_nodes = malloc(sizeof(long));
			srcs[0].dst_nodes[0] = dst;
			srcs[0].dst_size = 1;
		}
		else
		{
			srcs[0].dst_nodes = NULL;
			srcs[0].dst_size = 0;
		}
		srcs[0].pagerank = 1;
		srcs[0].pagerank_boost = 0;
		insert_struct_node(dst, -1, 0, srcs_size - 1);
		return;
	}

	int middle = (start + end) / 2;

	if(srcs[middle].src_node == src)
	{
		if(dst != -1)	
		{
			insert_struct_node(dst, -1, 0, srcs_size - 1);
			insert_dest_link(dst, middle, 0, srcs[middle].dst_size - 1);
		}
		return;
	}

	if(start == end)
	{
		int direction;
		if(srcs[middle].src_node > src)	direction = 0;
		else	direction = 1;
		srcs = realloc(srcs, ++srcs_size * sizeof(struct node));
		if(start < srcs_size - 1)	memmove(srcs + start + direction + 1, srcs + start + direction, (srcs_size - start - direction - 1) * sizeof(/*struct node*/*(srcs + start + direction)));
		srcs[start + direction].src_node = src;
		if(dst >= 0)
		{
			srcs[start + direction].dst_nodes = malloc(sizeof(long));
			srcs[start + direction].dst_nodes[0] = dst;
			srcs[start + direction].dst_size = 1;
		}
		else
		{
			srcs[start + direction].dst_nodes = NULL;
			srcs[start + direction].dst_size = 0;
		}
		srcs[start + direction].pagerank = 1;
		srcs[start + direction].pagerank_boost = 0;
		insert_struct_node(dst, -1, 0, srcs_size - 1);
		return;
	}

	if(srcs[middle].src_node > src)
	{
		insert_struct_node(src, dst, start, middle);
		return;
	}
	else
       	{
		insert_struct_node(src, dst, middle + 1, end);
		return;
	}
	//return (srcs[middle].src_node > src) ? insert_struct_node(src, dst, start, middle) : insert_struct_node(src, dst, middle + 1, end);
}

void *calculate_pagerank(void *parameter)
{
	int i, j, k;
	struct t_params *param = (struct t_params *) parameter;
	for(i = param->start; i < param->end; i++)
	{
		float dist_val = srcs[i].pagerank * 0.85 / srcs[i].dst_size;

		for(j = 0; j < srcs[i].dst_size; j++)
		{
			long curr = srcs[i].dst_nodes[j];
			int index = find_struct_node(curr, srcs, srcs_size);

			for(k = 0; k < num_of_threads; k++)
			{
				if(index >= par[k].start && index < par[k].end)
				{
					pthread_mutex_lock(&lock[k]);
					srcs[index].pagerank_boost += dist_val;
					pthread_mutex_unlock(&lock[k]);
					break;
				}
			}
		}
	}

	return NULL;
}

void *update_pagerank(void *parameter)
{
	int i;
	struct t_params *param = (struct t_params *) parameter;
	for(i = param->start; i < param->end; i++)
	{
		if(srcs[i].dst_size > 0)	srcs[i].pagerank *= 0.15; 
		srcs[i].pagerank += srcs[i].pagerank_boost;
		srcs[i].pagerank_boost = 0;
	}

	return NULL;
}

int main(int argc, char ** argv)
{
	if(argc != 3)	
	{
		printf("Provide a file name and the number of threads.\n");
		exit(EXIT_FAILURE);
	}

	FILE * fp = fopen(argv[1], "r");
	if(fp == NULL)
	{
		printf("Invalid file name.\n");
		exit(EXIT_FAILURE);
	}

	num_of_threads = atoi(argv[2]);
	if(num_of_threads < 1 || num_of_threads > 4)
	{
		printf("Not a valid number of threads.\n");
		exit(EXIT_FAILURE);
	}
	srcs = NULL;
	srcs_size = 0;
	size_t max_line_size = 256, ret;
	char *line = NULL;
	int c = 0;
	clock_t begin = clock();
	while((ret = getline(&line, &max_line_size, fp)) != -1)
	{
		if(line[0] == '#' || line[0] == '\n')	continue;

		char *line_copy = strdup(line);
		long curr_src = atol(strtok(line, "\t"));
		if(strcmp(line, line_copy) == 0)	curr_src = atol(strtok(line_copy, " "));
		long curr_dst = atol(strtok(NULL, "\t"));
		insert_struct_node(curr_src, curr_dst, 0, srcs_size - 1);

	}
	fclose(fp);
	clock_t end = clock();
	printf("Time spent creating the graph: %f sec", (double) (end - begin) / CLOCKS_PER_SEC);

	int i, j;
	
	for(j = 0; j < num_of_threads; j++)
	{
		par[j].start = srcs_size * j / num_of_threads;
		par[j].end = srcs_size * (j + 1) / num_of_threads;
		par[j].thread_number = j + 1;
		pthread_mutex_init(&lock[j], NULL);
	}
	for(i = 1; i <= 50; i++)
	{
		for(j = 0; j < num_of_threads; j++)
		{
			pthread_create(&threads[j], NULL, calculate_pagerank, &(par[j]));
		}
		for(j = 0; j < num_of_threads; j++)
		{
			pthread_join(threads[j], NULL);
		}
		for(j = 0; j < num_of_threads; j++)
		{
			pthread_create(&threads[j], NULL, update_pagerank, &(par[j]));
		}
		for(j = 0; j < num_of_threads; j++)
		{
			pthread_join(threads[j], NULL);
		}
	}

	FILE *fp2 = fopen("pagerank.csv", "w");
	for(i = 0; i < srcs_size; i++)
	{
		fprintf(fp2, "%ld, %.3f\n", srcs[i].src_node, srcs[i].pagerank);
	}
	fclose(fp2);

	for(j = 0; j < num_of_threads; j++)	pthread_mutex_destroy(&lock[j]);
	free(srcs);
	return 0;
}
