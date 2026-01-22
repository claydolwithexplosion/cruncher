#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
	NUM_IMMS_RAW = 76,
	NUM_IMMS_RORED = 1201,
	TRUE = (0 == 0),
	FALSE = (0 != 0),
};

enum op {
	OP_ADC,
	OP_SBC,
	/*OP_BIC,
	OP_RSC,
	OP_MOV,
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

static int is_done(union entry *array)
{
	for (uint32_t u = 1; u != 0; u += 1)
		if (array[u].raw == 0)
			return FALSE;
	return TRUE;
}

uint32_t imms_raw[NUM_IMMS_RAW] = {
	0x00,
	0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,
	0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb8,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
	0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
	0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,
};

int main(void)
{
	uint32_t *imms = calloc(NUM_IMMS_RORED, sizeof(uint32_t));
	union entry *array = calloc(1ull << 32, sizeof(union entry));
	array[0] = (union entry) {
		.index = 0,
		.operation = OP_ADC,
		.card = 1,
	};
	unsigned idx = 1;
	for (int i = 0; i < 16; i += 1) {
		for (unsigned j = 1; j < NUM_IMMS_RAW; j += 1) {
			uint32_t imm = imms_raw[j];
			uint32_t imm_rored = (imm >> (i * 2)) | (imm << (32 - (i * 2)));
			imms[idx++] = imm_rored;
		}
	}

	unsigned card = 2;
	while (!is_done(array)) {
		unsigned prog = 0;
		/* For cardinalities 1-4 (2-5 in this program), expand from existing nodes. */
		if (card <= 5) {
			for (uint64_t cnt = 0; cnt < 1ull << 32; cnt += 1) {
				union entry ent = array[(uint32_t)(cnt)];
				if (ent.raw != 0 && ent.card == card - 1) {
					if ((prog++ & 0x3fff) == 0) {
						printf("%u\r", prog);
						fflush(stdout);
					}
					for (int i = 0; i < NUM_IMMS_RORED; i += 1) {
						uint32_t imm = imms[i];
						uint32_t new_val = cnt + imm;
						if (array[new_val].raw == 0) {
							array[new_val] = (union entry) {
								.index = i,
								.operation = OP_ADC,
								.card = card,
							};
						}
					}
					for (int i = 0; i < NUM_IMMS_RORED; i += 1) {
						uint32_t imm = imms[i];
						uint32_t new_val = cnt - imm - 1;
						if (array[new_val].raw == 0) {
								array[new_val] = (union entry) {
								.index = i,
								.operation = OP_SBC,
								.card = card,
							};
						}
					}
				}
			}
		}
		/* For cardinalities above this, (so 5+, 6+ in this program) instead search through all empty nodes (much faster). */
		else {
			for (uint32_t cnt = 1; cnt != 0; cnt += 1) {
				union entry ent = array[(uint32_t)(cnt)];
				if (ent.raw == 0) {
					if ((prog++ & 0x3fff) == 0) {
						printf("%u\r", prog);
						fflush(stdout);
					}
					for (int i = 0; i < NUM_IMMS_RORED; i += 1) {
						uint32_t imm = imms[i];
						uint32_t new_val = cnt - imm;
						if (array[new_val].card == card - 1) {
							array[cnt] = (union entry) {
								.index = i,
								.operation = OP_ADC,
								.card = card,
							};
							break;
						}
					}
					for (int i = 0; i < NUM_IMMS_RORED; i += 1) {
						uint32_t imm = imms[i];
						uint32_t new_val = cnt + imm + 1;
						if (array[new_val].card == card - 1) {
								array[cnt] = (union entry) {
								.index = i,
								.operation = OP_SBC,
								.card = card,
							};
							break;
						}
					}
				}
			}
		}
		printf("Done with cardinality %d in %d steps\n", card - 1, prog);
		/*printf("Exit? ");
		char c;
		scanf("%c", &c);
		if (c == 'y')
			goto end;*/
		card += 1;
	}
	FILE *file = fopen("database.raw", "w");
	fwrite(array, sizeof(union entry), 1ull << 32, file);
	fclose(file);

/*end:*/
	uint32_t query;
	while (0 == 0) {
		printf("Please enter the number in hex: ");
		scanf("%x", &query);
		union entry ent = array[query];
		printf("Operation: %s, Number: %08x, Card: %d\n", (const char*[4]){"ADC","SBC","undefined","undefined"}[ent.operation], imms[ent.index], ent.card - 1);
	}
	return 0;
}
