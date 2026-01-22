#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
	TRUE = (0 == 0),
	FALSE = (0 != 0),
	NUM_RAW_IMMS = 76,
	NUM_ROR_IMMS = 1201,
};

enum op {
	OP_ADC,
	OP_SBC,
	/*OP_RSC,
	OP_MOV,
	OP_BIC,
	OP_MVN,*/
};

union entry {
	struct {
		uint16_t index : 11;
		uint16_t operation : 2;
		uint16_t card : 3;
	};
	uint16_t raw;
};

static const uint32_t imms_raw[NUM_RAW_IMMS] = {
	0x00,
	0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,
	0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb8,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
	0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
	0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,
};
static uint32_t imms[NUM_ROR_IMMS];

int main(void)
{
	unsigned idx = 1;
	imms[0] = 0;
	for (int i = 0; i < 16; i += 1) {
		for (unsigned j = 1; j < NUM_RAW_IMMS; j += 1) {
			uint32_t imm = imms_raw[j];
			uint32_t imm_rored = (imm >> (i * 2)) | (imm << (32 - (i * 2)));
			imms[idx++] = imm_rored;
		}
	}
	FILE *file = fopen("database.raw", "r");
	union entry *array = calloc(1ull << 32, sizeof(union entry));
	fread(array, sizeof(union entry), 1ull << 32, file);
	fclose(file);
	for (uint32_t u = 1; u != 0; u += 1) {
		if ((u & 0xfff) == 0) {
			printf("Checking %08x...\r", u);
		}
		uint32_t val = u;
		union entry ent = array[val];
		while (ent.card > 1) {
			if (ent.operation == OP_ADC) {
				val -= imms[ent.index];
			} else {
				val += imms[ent.index] + 1;
			}
			union entry new = array[val];
			if (ent.card - new.card != 1) {
				printf("Warning! Value %08x has bad cardinality.\n", u);
			}
			ent = new;
		}
	}
	free(array);
}
