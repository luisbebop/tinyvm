#include <stdlib.h>
#include <string.h>

#include <tvm/tvm_hashtab.h>

tvm_htab_t* create_htab()
{
	return (tvm_htab_t*)calloc(1, sizeof(tvm_htab_t));
}

void destroy_htab(tvm_htab_t* htab)
{
	int i;
	for(i = 0; i < HTAB_SIZE; i++)
	{
		if(htab->nodes[i])
		{
			if (htab->nodes[i]->complexValue != NULL ) free(htab->nodes[i]->complexValue);
			free(htab->nodes[i]->key);
			free(htab->nodes[i]);
		}
	}

	free(htab);
}

int htab_add(tvm_htab_t* htab, const char* k, int v)
{
	return htab_add_complex_value(htab, k, v, NULL, 0, 0);
}

int htab_add_complex_value(tvm_htab_t* htab, const char* k, int v, void * cv, int cvLen, unsigned char cvType)
{
	int hash = htab_hash(k);

	/* If the node is not already occupied, allocate space, and copy the key/value pair. */
	if(htab->nodes[hash] == NULL) htab->nodes[hash] = calloc(1, sizeof(tvm_htable_node_t));
	else return 1;

	htab->nodes[hash]->key = (char*)malloc(sizeof(char) * (strlen(k) + 1));
	
	if (cv != NULL)
	{
		htab->nodes[hash]->complexValue = malloc(cvLen);
		htab->nodes[hash]->complexValueType = cvType;
		htab->nodes[hash]->complexValueLen = cvLen;
		memcpy(htab->nodes[hash]->complexValue, cv, cvLen);
	}
	else
	{
		htab->nodes[hash]->complexValue = NULL;
		htab->nodes[hash]->complexValueType = 0;
		htab->nodes[hash]->complexValueLen = 0;
	}

	strcpy(htab->nodes[hash]->key, k);
	htab->nodes[hash]->value = v;

	return 0;
}

int htab_find(tvm_htab_t* htab, const char* key)
{
	int hash = htab_hash(key);

	if(htab->nodes[hash] != NULL) return htab->nodes[hash]->value;
	else return -1;
}

void * htab_find_complex_value(tvm_htab_t* htab, const char* key, int * cvLen, unsigned char * cvType)
{
	int hash = htab_hash(key);

	if(htab->nodes[hash] != NULL)
	{
		*cvLen = htab->nodes[hash]->complexValueLen;
		*cvType = htab->nodes[hash]->complexValueType;
		return htab->nodes[hash]->complexValue;
	}
	else return NULL;
}

unsigned int htab_hash(const char* k)
{
	unsigned int hash = 1;

	char* c; for(c = (char*)k; *c; c++)
		hash += (hash << *c) - *c;

	return hash % HTAB_SIZE;
}