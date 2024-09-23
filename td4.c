#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#define CLOCK_GENERATOR 1
const char* program_file = "bins\\binary.bin";
const char* input_file = "bins\\input.bin";

typedef struct
{
	char command;
	char operand;
} instruction;


typedef struct program {
	instruction instruction_set[16];    
	char reg_A;                  
	char reg_B;                  
	char flag_C;                
	char out_port;                
	char in_port;                
	char prog_counter;              
} program;

program Program = {
	.instruction_set = {
		{0, 0}, {0, 0}, {0, 0}, {0, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}
	},
	.reg_A = 0,
	.reg_B = 0,
	.flag_C = 0,
	.out_port = 0,
	.in_port = 0,
	.prog_counter = 0
};


void inputfile_check()
{
	FILE *fp;
	fp = fopen(input_file, "rb");

	if (fp == NULL)
	{
		printf("Error opening file\n");
		exit(-1);
	}

	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);
	if (file_size > 1)
	{
		printf("Too big for input file\n");
		exit(-1);
	}
	
	fclose(fp);
}

void programfile_check()
{
	FILE *fp;
	fp = fopen(program_file, "rb");

	if (fp == NULL)
	{
		printf("Error opening file\n");
		exit(-1);
	}

	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);
	if (file_size > 16)
	{
		printf("Too big for program file\n");
		exit(-1);
	}
	
	fclose(fp);
}

void write_instruction(char octet, int location)
{
	char first_quartet = (octet >> 4) & 0x0F;
	char second_quartet = octet & 0x0F;
	Program.instruction_set[location].command = first_quartet;
	Program.instruction_set[location].operand = second_quartet;
}

void fill_program()
{
	FILE *fp;
	fp = fopen(program_file, "rb");

	if (fp == NULL)
	{
		printf("Error opening file\n");
		exit(-1);
	}

	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp);
	rewind(fp);

	unsigned char *buffer = (unsigned char *)malloc(file_size);
	fread(buffer, sizeof(unsigned char), file_size, fp);

	for (int i = 0; i < file_size; i++)
		write_instruction(buffer[i], i);

	fclose(fp);
	free(buffer);
}

unsigned char read_input()
{
	FILE *fp;
	fp = fopen(input_file, "rb");
	unsigned char input;
	fread(&input, sizeof(unsigned char), 1, fp);
	input = input & 0x0F;
	fclose(fp);
	return input;
}

void print_binary(int num)
{
	if (num == 0) {
		printf("0000");
		return;
	}

	int binary[4];
	int i = 3; 

	while (num > 0) {
		binary[i--] = num % 2;
		num /= 2;
	}

	for (; i >= 0; i--)
		binary[i] = 0; 

	for (int j = 0; j < 4; j++)
		printf("%d", binary[j]);
}

void update_screen()
{
	system("cls");
	printf("=========================================================\n");
	printf("\t\t\t TD4 Emulator\n");
	printf("=========================================================\n\n");
	printf("Register A: "); print_binary((int)Program.reg_A); printf(" \tRegister B: "); print_binary((int)Program.reg_B); printf(" \tFlag C: %d\n", Program.flag_C);
	printf("Input Port: "); print_binary((int)Program.in_port); printf(" \tOutput Port: "); print_binary((int)Program.out_port);
	printf("\nProgram Counter: "); print_binary((int)Program.prog_counter);
}



void exec()
{
	int next_instruction = 0;
	int res = 0;
	
	Program.in_port = read_input();
	
	
	while (next_instruction < 16)
	{
		Program.prog_counter = next_instruction;
		update_screen();
		switch (Program.instruction_set[next_instruction].command)
		{
		case 0b0000:							// ADD A,Im
			res = (int)Program.reg_A + (int)Program.instruction_set[next_instruction].operand;
			if (res > 16)
			{
				Program.reg_A = res - 16;
				Program.flag_C = 1;
			}
			else
			{
				Program.reg_A = res;
				Program.flag_C = 0;
			}
			next_instruction++;
			break;	
		case 0b0101:							// ADD B,Im
			res = (int)Program.reg_B + (int)Program.instruction_set[next_instruction].operand;
			if (res > 16)
			{
				Program.reg_B = res - 16;
				Program.flag_C = 1;
			}
			else
			{
				Program.reg_B = res;
				Program.flag_C = 0;
			}
			next_instruction++;
			break;
		case 0b0011:							// MOV A,Im
			Program.reg_A = Program.instruction_set[next_instruction].operand;
			next_instruction++;
			break;
		case 0b0111:							// MOV B,Im
			Program.reg_B = Program.instruction_set[next_instruction].operand;
			next_instruction++;
			break;
		case 0b0001:							// MOV A,B
			Program.reg_A = Program.reg_B;
			next_instruction++;
			break;
		case 0b0100:							// MOV B,A
			Program.reg_B = Program.reg_A;
			next_instruction++;
			break;
		case 0b1111:							// JMP Im
			next_instruction = (int)Program.instruction_set[next_instruction].operand;
			break;
		case 0b1110:							// JNC Im
			if (Program.flag_C == 0)
				next_instruction = (int)Program.instruction_set[next_instruction].operand;
			else
				next_instruction++;
			break;
		case 0b0010:							// IN A
			Program.reg_A = read_input();
			next_instruction++;
			break;
		case 0b0110:							// IN B
			Program.reg_B = read_input();
			next_instruction++;
			break;
		case 0b1001:							// OUT B
			Program.out_port = Program.reg_B;
			next_instruction++;
			break;
		case 0b1011:							// OUT Im
			Program.out_port = Program.instruction_set[next_instruction].operand;
			next_instruction++;
			break;
		default:
			printf("error");
			exit(-1);
		}
		Sleep(1000/CLOCK_GENERATOR);
	}
	
	printf("\n\nProgram stopped\n");
}


int main()
{
	inputfile_check();
	programfile_check();
	fill_program();
	exec();
		
	return 0;
}
