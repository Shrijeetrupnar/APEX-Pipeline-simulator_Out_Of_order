#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#define IQ_ENTRIES 8
#define LSQ_ENTRIES 6
#define PHYREG_ENTRIES 24
#define ROB_ENTRIES 12
#define ARF_ENTRIES 24
#define PRF_ENTRIES 24
#define RAT_ENTRIES 24
#define RRAT_ENTRIES_NUMBER 24



/**
 *  cpu.h
 *  Contains various CPU and Pipeline Data structures
 *
 *  Author :
 *  shrijeet rupnar (srupnar1@binghamton.edu)
 *  State University of New York, Binghamton
 */

enum Stages
{
  F,
  DRF,
  INT_FU1,
  INT_FU2,
  MUL_FU1,
  MUL_FU2,
  MUL_FU3,
  MEM_FU,
  NUM_STAGES
};

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
  char opcode[128];	// Operation Code
  int rd;		    // Destination Register Address
  int rs1;		    // Source-1 Register Address
  int rs2;		    // Source-2 Register Address
  int imm;		    // Literal Value
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
  int pc;		    // Program Counter
  char opcode[128];	// Operation Code
  int phy_rs1;   // physical register
  int phy_rs2;
  int phy_rd;
  int rs1;		    // Architectural Source-1 Register Address
  int rs2;		    // Source-2 Register Address
  int rd;		    // Destination Register Address
  int rdd;      //extra created for printing so that it doesnt update rd
  int imm;		    // Literal Value
  int rs1_value;	// Source-1 Register Value
  int rs2_value;	// Source-2 Register Value
  int rs1_valid;
  int rs2_valid;
  int buffer;		// Latch to hold some value
  int mem_address;	// Computed Memory Address
  int busy;		    // Flag to indicate, stage is performing some action
  int stalled;		// Flag to indicate, stage is stalled
  int ROB_entry_id;
  int LSQ_index;

} CPU_Stage;

// ISSUE QUEUE  entry
typedef struct IQ_entry{

int pc;
char opcode[128];
int counter;
int free;
enum Stages FU_type;
int imm;
// source1
int phy_rs1;
int rs1;
int rs1_value;
int rs1_ready;
// source2
int phy_rs2;
int rs2;
int rs2_value;
int rs2_ready;

int phy_rd;
int rd;
int ROB_entry_id;
int LSQ_index;
} IQ_entry;
// issue queue
typedef struct IQ
{
int free_entry; //for pointing
IQ_entry iq_entry[IQ_ENTRIES];
} IQ;

typedef struct ROB_entry
{
int free;
char opcode[128];
int pc;
int imm;
// source1
int phy_rs1;
int rs1;
int rs1_value;
// source2
int phy_rs2;
int rs2;
int rs2_value;

int phy_rd;
int rd;
int status;
} ROB_entry;

typedef struct ROB
{
int tail;
int head;
ROB_entry rob_entry[ROB_ENTRIES];
} ROB;

typedef struct ARF_Entry
{
  int value;    // Value of arch register
  int free;    // Status bit indicating whether physical register is free or allocated
  int valid;    // Valid bit indicating whether physical register holds valid value or not
} ARF_Entry;

typedef struct PRF_Entry
{
  int value;    // Value of physical register
  int free;    // Status bit indicating whether physical register is free or allocated
  int valid;    // Valid bit indicating whether physical register holds valid value or not
} PRF_Entry;

typedef struct RAT_Entry
{
  int phys_reg;   // The most recent physical register for an architectural register in URF
} RAT_Entry;

typedef struct RRAT_Entry
{
  int commited_phy_reg; // Commited physical register for an architectural register in URF
} RRAT_Entry;

typedef struct LSQ_Entry
{
  int free;
  char opcode[128];
  int pc;
  int mem_address_valid;
  int mem_address;
  int ROB_entry_id;

  /* Source-1 fields */
  int rs1;
  int rs1_ready;    // source-1 ready bit
  int phy_rs1;    // source-1 physical address
  int rs1_value;    // source-1 value

  /* Source-2 fields */
  int rs2;
  int phy_rs2;    // source-2 physical address

  int imm;

  int rd;
  int phy_rd;
} LSQ_Entry;

typedef struct LSQ
{
  int tail;
  int head;
  LSQ_Entry LSQ_entry[LSQ_ENTRIES];
} LSQ;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
  /* Clock cycles elasped */
  int clock;

  /* Current program counter */
  int pc;
  int flag_mul;
  int source1_flag;
  int source2_flag;
  int mul_display;
  int int_display;
  /* Integer register file */
  int regs[24];
  int regs_valid[24];
  int phy_reg;
  int phy_regs[24];
  int last_arith_phys_rd;
  int commitments;
  int arch_reg;
  /* Array of 9 CPU_stage */
  CPU_Stage stage[8];

  /* Code Memory where instructions are stored */
  APEX_Instruction* code_memory;
  int code_memory_size;

  /* Data Memory */
  int data_memory[4096];

  /* Some stats */
  int ins_completed;
  ARF_Entry arf[ARF_ENTRIES];
  PRF_Entry prf[PRF_ENTRIES];
  RAT_Entry rat[RAT_ENTRIES];
  RRAT_Entry rrat[RRAT_ENTRIES_NUMBER];
  IQ iq;
  ROB rob;
  LSQ lsq;

} APEX_CPU;

ARF_Entry arf[ARF_ENTRIES];
RAT_Entry rat[RAT_ENTRIES];
IQ iq;
ROB rob;
LSQ lsq;

APEX_Instruction*
create_code_memory(const char* filename, int* size);

APEX_CPU*
APEX_cpu_init(const char* filename);

int
APEX_cpu_run(APEX_CPU* cpu);

void
APEX_cpu_stop(APEX_CPU* cpu);

int
fetch(APEX_CPU* cpu);

int
decode(APEX_CPU* cpu);

int
int_FU1(APEX_CPU* cpu);

int
int_FU2(APEX_CPU* cpu);

int
mul_FU1(APEX_CPU* cpu);

int
mul_FU2(APEX_CPU* cpu);

int
mul_FU3(APEX_CPU* cpu);

int
rob_retire(APEX_CPU* cpu);

int
mem_FU(APEX_CPU* cpu);
#endif
