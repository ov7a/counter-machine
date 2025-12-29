#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_INSTRUCTIONS 1024
typedef unsigned short INumber;

#define MAX_EXECUTION 1000000

#define MAX_MEMORY 1024
typedef unsigned short Address;

typedef unsigned short Counter;
#define MAX_COUNTER_VALUE USHRT_MAX

typedef enum {
	INC,
	JZDEC
} __attribute__ ((__packed__)) Command;

typedef struct {
	Command command;
	Address address;
	INumber jump;
} Instruction;

typedef struct {
	Instruction instructions[MAX_INSTRUCTIONS];
	INumber current;
	INumber loaded;
	unsigned int executed;

	Counter counters[MAX_MEMORY];
	Address used;
} State;

#ifdef DEBUG
    #define LOGDEBUG(fmt, ...) fprintf(stderr, "DEBUG: " fmt, ##__VA_ARGS__)
#else
    #define LOGDEBUG(fmt, ...)
#endif

void validate_address(INumber current, unsigned int address){
	if (address >= MAX_MEMORY){
		fprintf(stderr, "Instruction %u is trying to address counter #%u, but the number of counters is limited by %u.\n", current, address, MAX_MEMORY);
		exit(101);
	}
}

void validate_inc(Counter value, INumber current, Address address){
	if (value == MAX_COUNTER_VALUE){
		fprintf(stderr, "Instruction %u is trying to increment counter #%u, but it reached the max value of %u already.\n", current, address, MAX_COUNTER_VALUE);
		exit(102);
	}
}

void validate_jump(INumber current, unsigned int jumpTo){
	if (jumpTo > MAX_INSTRUCTIONS){
		fprintf(stderr, "Instruction %u is trying to jump to #%u, but the number of instructions is limited by %u.\n", current, jumpTo, MAX_INSTRUCTIONS);
		exit(103);
	}
	if (jumpTo == 0){
		fprintf(stderr, "Instruction %u is trying to jump to #%u, instruction numbers start from 1.\n", current, jumpTo);
		exit(104);
	}
}

void execute(State* state){
	Instruction instruction = state->instructions[state->current - 1];
	Address address = instruction.address;
	Counter* counter = &(state->counters[address]);
	switch(instruction.command){
		case INC:
			validate_inc(*counter, state->current, address);
			LOGDEBUG("%u: increment %u, current value %u\n", state->current, address, *counter);
			(*counter)++;
			state->current++;
			break;
		case JZDEC:
			LOGDEBUG("%u: decrement %u, current value %u, maybe jump to %u\n", state->current, address, *counter, instruction.jump);
			if (*counter > 0){
				(*counter)--;
				state->current++;
			} else {
				state->current = instruction.jump;
			}
			break;
	}
	state->executed++;
	if (state->executed > MAX_EXECUTION){
		fprintf(stderr, "Executed too many operations: %u. Most likely, you're in infinite loop at instruction %u.\n", state->executed, state->current);
		exit(1);
	}
}

// because scanf consumes trailing + and -
bool read_number(unsigned int *result){
	unsigned int n = 0;
	int c = getchar();
	if (c < '0' || '9' < c){
		return false;
	}
	while ('0' <= c && c <= '9'){
		n = n*10 + c - '0';
		c = getchar();
	}
	ungetc(c,stdin);
	*result = n;
	return true;
}

Address read_address(INumber current){
	unsigned int address;
	if (!read_number(&address)){
		fprintf(stderr, "Error during parsing address for instruction %u.\n", current);
		exit(104);
	}
	validate_address(current, address);
	return address;
}

INumber read_jump(INumber current){
	unsigned int inumber;
	int c = getchar();
	if (c != '?'){
		fprintf(stderr, "Error during parsing JZDEC (instruction %u): expected '?', got '%c'.\n", current, c);
		exit(105);
	}
	if (!read_number(&inumber)){
		fprintf(stderr, "Error during parsing jump direction for instruction %u.\n", current);
		exit(106);
	}
	validate_jump(current, inumber);
	return inumber;
}

bool read_next(INumber current, Instruction* instruction, Address* used){
	bool read = false;
	bool finish = false;
	while (!read && !finish){
		int c = getchar();
		switch (c){
			case EOF:
				finish = true;
				break;
			case '+':
				instruction->command = INC;
				instruction->address = read_address(current);
				if (instruction->address > *used) *used = instruction->address;
				read = true;
				break;
			case '-':
				instruction->command = JZDEC;
				instruction->address = read_address(current);
				instruction->jump = read_jump(current);
				if (instruction->address > *used) *used = instruction->address;
				read = true;
				break;
			case '\n':
				int next = getchar();
				if (next == '\n'){
					finish = true;
					break;
				} else {
					ungetc(next, stdin);
				}
				break;
			case ' ':
				break;
			default:
				fprintf(stderr, "Unexpected character during reading instruction %u: '%c'. The instruction should be either INC ('+123') or JZDEC ('-246?123'). Spaces and newlines are ignored. Two new lines abort the execution.\n", current, c);
				exit(100);
		}
	}
	return finish;
}

void read_and_execute(State* state){
    state->current = 1;
	Instruction instruction;

	while(true){
		LOGDEBUG("current: %u, loaded: %u\n", state->current, state->loaded);
		while (state->current > state->loaded){
		    if (state->current > MAX_INSTRUCTIONS){
				fprintf(stderr, "The maximum number of instructions is reached: %u. Aborting execution.\n", MAX_INSTRUCTIONS);
				exit(107);
			}
			bool finish = read_next(state->loaded + 1, &instruction, &(state->used));
			LOGDEBUG("Read instruction: %s, address %u, jump %u\n", instruction.command == INC ? "INC" : "JZDEC", instruction.address, instruction.jump);
			if (finish){
				if (state->current != state->loaded + 1){
					fprintf(stderr, "Warning: current instruction number is %u, but there are only %u instructions. Note that the last addressable instruction is %u (end of the program).\n", state->current, state->loaded, state->loaded + 1);
				}
				return;
			}
			state->instructions[state->loaded++] = instruction;
		}
		execute(state);
	}
}

void print_resulting_state(State* state){
	printf("Total instructions: %u, executed: %u. Total %u counters were used.\n", state->loaded, state->executed, state->used + 1);
	Address last_address = state->used;
	while (state->counters[last_address] == 0 && last_address != 0){
		last_address--;
	}
	if (last_address == 0 && state->counters[last_address] == 0){
		puts("All counters are zero.");
	} else {
		for (Address i = 0; i <= last_address; i++){
			printf("%04d: %u\n", i, state->counters[i]);
		}
	}
}

int main(void) {
    State state = {0};
	read_and_execute(&state);
	print_resulting_state(&state);
	return 0;
}
