#include <tvm/tvm.h>

tvm_t* tvm_create(char* filename)
{
	tvm_t* vm = (tvm_t*)malloc(sizeof(tvm_t));

	vm->pMemory = create_memory(MIN_MEMORY_SIZE);
	vm->pProgram = create_program();

	create_stack(vm->pMemory, MIN_STACK_SIZE);

	if(!vm || !vm->pMemory || !vm->pProgram) return NULL;

	return vm;
}

int tvm_interpret(tvm_t* vm, char* filename)
{
	if(interpret_program(vm->pProgram, filename, vm->pMemory) != 0) return 1;
	return 0;
}

void tvm_destroy(tvm_t* vm)
{
	if(vm && vm->pMemory)destroy_memory(vm->pMemory);
	if(vm && vm->pProgram)destroy_program(vm->pProgram);
	if(vm) free(vm);
}

void tvm_run(tvm_t* vm)
{
	int* instr_idx = &vm->pMemory->registers[0x8].i32; *instr_idx = vm->pProgram->start;

	for(;vm->pProgram->instr[*instr_idx] != -0x1; ++(*instr_idx)) tvm_step(vm, instr_idx);
}

int * tvm_lookup_arg_type(tvm_t* vm, int * arg)
{
	char pointer_address[50];
	int * arg_type;
		
	memset(pointer_address,0,sizeof(pointer_address));
	sprintf(pointer_address,"%p", arg);
	arg_type = htab_find_pointer(vm->pMemory->address_type_htab, pointer_address);
	if (arg_type == NULL) 
	{
		htab_add(vm->pMemory->address_type_htab, pointer_address, 0);
		arg_type = htab_find_pointer(vm->pMemory->address_type_htab, pointer_address);
	}
		
	return arg_type;
}

void tvm_set_arg_type(tvm_t* vm, int * arg, int type)
{
	char pointer_address[50];
	
	memset(pointer_address,0,sizeof(pointer_address));
	sprintf(pointer_address,"%p",arg);
	htab_add(vm->pMemory->address_type_htab, pointer_address, type);
}

void tvm_prn(tvm_t* vm, int * arg)
{
	char prn_buf[1024];
	int * type;
	void * complexValue;
	int complexValueLen;

	type = tvm_lookup_arg_type(vm, arg);
	// is not an address to the hashtable. It is a integer
	if(*type == 0) 
	{
		printf("%i\n", *arg); 
	}
	else 
	{
		memset(prn_buf,0,sizeof(prn_buf));
		complexValue = vm->pProgram->label_htab->nodes[*arg]->complexValue;
		complexValueLen = vm->pProgram->label_htab->nodes[*arg]->complexValueLen;
		strncpy(prn_buf,(const char *)complexValue, complexValueLen);
		//is an address in the hashtable
		printf("%s\n", prn_buf);
	}
}

void tvm_add(tvm_t* vm, int * arg0, int * arg1)
{
	int * arg_type0;
	int * arg_type1;
	void * complexValue;
	int complexValueLen;
	char add_buf1[1024];
	char add_buf2[1024];
	
	arg_type0 = tvm_lookup_arg_type(vm, arg0);
	arg_type1 = tvm_lookup_arg_type(vm, arg1);
	
	// parse complex values
	if (*arg_type0 == 1)
	{
		memset(add_buf1,0,sizeof(add_buf1));
		complexValue = vm->pProgram->label_htab->nodes[*arg0]->complexValue;
		complexValueLen = vm->pProgram->label_htab->nodes[*arg0]->complexValueLen;
		strncpy(add_buf1,(const char *)complexValue, complexValueLen);
	}
	
	if (*arg_type1 == 1)
	{
		memset(add_buf2,0,sizeof(add_buf2));
		complexValue = vm->pProgram->label_htab->nodes[*arg1]->complexValue;
		complexValueLen = vm->pProgram->label_htab->nodes[*arg1]->complexValueLen;
		strncpy(add_buf2,(const char *)complexValue, complexValueLen);
	}
	
	//first and second arg are int
	if (*arg_type0 == 0 && *arg_type1 == 0)
	{
		*arg0 += *arg1; 
	}
	
	//first arg is int and second arg is a complex value
	if(*arg_type0 == 0 && *arg_type1 == 1)
	{
		*arg0 += atoi(add_buf2);
	}
	
	//first and second arg are complex value
	if (*arg_type0 == 1 && *arg_type1 == 1)
	{
		strcat(add_buf1,add_buf2);
		
		//clean the old memory in the hash table
		free(vm->pProgram->label_htab->nodes[*arg0]->complexValue);
		vm->pProgram->label_htab->nodes[*arg0]->complexValue = malloc(strlen(add_buf1));
		vm->pProgram->label_htab->nodes[*arg0]->complexValueLen = strlen(add_buf1);
		memcpy(vm->pProgram->label_htab->nodes[*arg0]->complexValue, (void*)add_buf1, strlen(add_buf1));	
	}
	
	//first arg is a complex value and second arg is a int
	if (*arg_type0 == 1 && *arg_type1 == 0)
	{
		memset(add_buf2,0,sizeof(add_buf2));
		sprintf(add_buf2,"%i", *arg1);
		strcat(add_buf1,add_buf2);
		
		//clean the old memory in the hash table
		free(vm->pProgram->label_htab->nodes[*arg0]->complexValue);
		vm->pProgram->label_htab->nodes[*arg0]->complexValue = malloc(strlen(add_buf1));
		vm->pProgram->label_htab->nodes[*arg0]->complexValueLen = strlen(add_buf1);
		memcpy(vm->pProgram->label_htab->nodes[*arg0]->complexValue, (void*)add_buf1, strlen(add_buf1));
	}
}

void tvm_step(tvm_t* vm, int* instr_idx)
{
	int *arg0 = vm->pProgram->args[*instr_idx][0], *arg1 = vm->pProgram->args[*instr_idx][1];
	int arg_type0 = 0;
	
	switch(vm->pProgram->instr[*instr_idx])
	{
/* nop   */	case 0x0:  break;
/* int   */	case 0x1:  /* unimplemented */ break;
/* mov   */	case 0x2:  *arg0 = *arg1; break;
/* push  */	case 0x3:  		
				//push argument
				stack_push(vm->pMemory, arg0);
				printf("push arg0 [%i]\n",*arg0);
				//push argument type
				stack_push(vm->pMemory,tvm_lookup_arg_type(vm, arg0));
				break;
/* pop   */	case 0x4:
				//pop argument type
				stack_pop(vm->pMemory, &arg_type0);
				printf("pop arg_type [%i]\n",arg_type0);
				//pop argument
				stack_pop(vm->pMemory, arg0); 
				printf("pop arg0 [%i]\n",*arg0);
				tvm_set_arg_type(vm, arg0, arg_type0);
				break;
/* pushf */	case 0x5:  stack_push(vm->pMemory, &vm->pMemory->FLAGS); break;
/* popf  */	case 0x6:  stack_pop(vm->pMemory, arg0); break;
/* inc   */	case 0x7:  ++(*arg0); break;
/* dec   */	case 0x8:  --(*arg0); break;
/* add   */	case 0x9: tvm_add(vm, arg0, arg1); break;
/* sub   */	case 0xA:  *arg0 -= *arg1; break;
/* mul   */	case 0xB:  *arg0 *= *arg1; break;
/* div   */	case 0xC:  *arg0 /= *arg1; break;
/* mod   */	case 0xD:  vm->pMemory->remainder = *arg0 % *arg1; break;
/* rem   */	case 0xE:  *arg0 = vm->pMemory->remainder; break;
/* not   */	case 0xF:  *arg0 = ~(*arg0); break;
/* xor   */	case 0x10:  *arg0 ^= *arg1;  break;
/* or    */	case 0x11: *arg0 |= *arg1;   break;
/* and   */	case 0x12: *arg0 &= *arg1;   break;
/* shl   */	case 0x13: *arg0 <<= *arg1;  break;
/* shr   */	case 0x14: *arg0 >>= *arg1;  break;
/* cmp   */	case 0x15: vm->pMemory->FLAGS = ((*arg0 == *arg1) | (*arg0 > *arg1) << 1); break;
/* call	 */	case 0x17: stack_push(vm->pMemory, instr_idx);
/* jmp	 */	case 0x16: *instr_idx = *arg0 - 1; break;
/* ret   */	case 0x18: stack_pop(vm->pMemory, instr_idx); break;
/* je    */	case 0x19: if(vm->pMemory->FLAGS   & 0x1)  *instr_idx = *arg0 - 1; break;
/* jne   */	case 0x1A: if(!(vm->pMemory->FLAGS & 0x1)) *instr_idx = *arg0 - 1; break;
/* jg    */	case 0x1B: if(vm->pMemory->FLAGS   & 0x2)  *instr_idx = *arg0 - 1; break;
/* jge   */	case 0x1C: if(vm->pMemory->FLAGS   & 0x3)  *instr_idx = *arg0 - 1; break;
/* jl    */	case 0x1D: if(!(vm->pMemory->FLAGS & 0x3)) *instr_idx = *arg0 - 1; break;
/* jle   */	case 0x1E: if(!(vm->pMemory->FLAGS & 0x2)) *instr_idx = *arg0 - 1; break;
/* prn   */	case 0x1F: tvm_prn(vm, arg0);
	};
}
