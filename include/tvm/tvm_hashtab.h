#ifndef TVM_HASHTAB_H_
#define TVM_HASHTAB_H_

#define KEY_LENGTH 64
#define HTAB_SIZE 4096

typedef struct tvm_htable_node_s
{
	char* key;
	int value;
	void * complexValue;
	int complexValueLen;
	unsigned char complexValueType;
} tvm_htable_node_t;

typedef struct tvm_htab_s
{
	int num_nodes;
	tvm_htable_node_t* nodes[HTAB_SIZE];
} tvm_htab_t;

tvm_htab_t* create_htab();
void destroy_htab(tvm_htab_t* htab);
int htab_add(tvm_htab_t* htab, const char* k, int v);
int htab_add_complex_value(tvm_htab_t* htab, const char* k, int v, void * cv, int cvLen, unsigned char cvType);
int htab_find(tvm_htab_t* htab, const char* key);
int * htab_find_pointer(tvm_htab_t* htab, const char* key);
void * htab_find_complex_value(tvm_htab_t* htab, const char* key, int * cvLen, unsigned char * cvType);
unsigned int htab_hash(const char* key);

#endif