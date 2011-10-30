#include <tvm/tvm_memory.h>

#include <stdlib.h>
#include <string.h>

#define NUM_REGISTERS 17

tvm_memory_t* create_memory(size_t size)
{
	tvm_memory_t* m =  (tvm_memory_t*)calloc(1, sizeof(tvm_memory_t));

	m->registers = calloc(NUM_REGISTERS, sizeof(tvm_register_t));

	m->mem_space_size = size;
	m->mem_space = (int*)calloc(size, 1);
	m->address_type_htab = create_htab();
	m->int_type = 0;
	
	return m;
}

void destroy_memory(tvm_memory_t* m)
{
	free(m->mem_space);
	free(m->registers);
	free(m);
}
