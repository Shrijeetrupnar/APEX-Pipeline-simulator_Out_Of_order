/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  shrijeet rupnar (srupnar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include"iq.c"
#include "register_renaming.c"
#include "lsq.c"
#include "rob_driver.c"

/* Set this flag to 1 to enable debug messages */
int ENABLE_DEBUG_MESSAGES=1;
//int mem_address[1] = 200;
/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }
  for (int i=0; i<PRF_ENTRIES; i++) {
    cpu->prf[i].value = 0; // initial value for registers
    cpu->prf[i].free = 1; // all phyisical registers are FREE
    cpu->prf[i].valid = 0; // all values are NOT valid
  }

  for (int i=0; i<RAT_ENTRIES; i++) {
    cpu->rat[i].phys_reg = -1; // initially architectural registers point to nowhere
  }



  cpu->rob.tail = 0;
  cpu->rob.head = 0;
  for (int i=0; i<ROB_ENTRIES; i++) {
    cpu->rob.rob_entry[i].free = 1;
    cpu->rob.rob_entry[i].pc = -1;
    cpu->rob.rob_entry[i].rd = -1;
    cpu->rob.rob_entry[i].phy_rd = -1;
    cpu->rob.rob_entry[i].status = 0;
  }

  for (int i=0; i<RRAT_ENTRIES_NUMBER; i++) {
    cpu->rrat[i].commited_phy_reg = -1; // initially commited architectural registers point to nowhere
  }


  cpu->lsq.head = 0;
  cpu->lsq.tail = 0;
  for (int i = 0; i < LSQ_ENTRIES; i++) {
    cpu->lsq.LSQ_entry[i].free = 1;
    strcpy(cpu->lsq.LSQ_entry[i].opcode, "");
    cpu->lsq.LSQ_entry[i].pc = -1;
    cpu->lsq.LSQ_entry[i].mem_address_valid = 0;
    cpu->lsq.LSQ_entry[i].mem_address = 0;
    //cpu->lsq.lsq_entry[i].branch_id = -1;
    cpu->lsq.LSQ_entry[i].ROB_entry_id = -1;
    cpu->lsq.LSQ_entry[i].rs1_ready = 0;
    cpu->lsq.LSQ_entry[i].phy_rs1 = -1;
    cpu->lsq.LSQ_entry[i].rs1_value = 0;
    cpu->lsq.LSQ_entry[i].phy_rs2 = -1;
    cpu->lsq.LSQ_entry[i].imm = 0;
    cpu->lsq.LSQ_entry[i].rd = -1;
    cpu->lsq.LSQ_entry[i].rd = -1;
  }

  cpu->iq.free_entry = -1;
  for (int i=0; i<IQ_ENTRIES; i++) {
    cpu->iq.iq_entry[i].pc = -1;
    strcpy(cpu->iq.iq_entry[i].opcode, "");
    cpu->iq.iq_entry[i].counter = 0;
    cpu->iq.iq_entry[i].free = 1;
    cpu->iq.iq_entry[i].FU_type = -1;
    cpu->iq.iq_entry[i].imm = -1;
    cpu->iq.iq_entry[i].rs1_ready = 0;
    cpu->iq.iq_entry[i].phy_rs1 = -1;
    cpu->iq.iq_entry[i].rs1_value = 0;
    cpu->iq.iq_entry[i].rs2_ready = 0;
    cpu->iq.iq_entry[i].phy_rs2 = -1;
    cpu->iq.iq_entry[i].rs2_value = 0;
    cpu->iq.iq_entry[i].phy_rd = -1;
    cpu->iq.iq_entry[i].LSQ_index = -1;
  }

//  cpu->clock = 1;
//  cpu->fill_in_rob = 1;

  //cpu->simulation_completed = 0;
  cpu->commitments = 0;

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  // for (int i = 1; i < NUM_STAGES; ++i) {
  //   cpu->stage[i].busy = 1;
  // }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

void
print_instruction(int fetch_decode, CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0) {
    if (fetch_decode) {
      printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
    }
    else {
      printf("%s,R%d,R%d,#%d  [%s,P%d,P%d,#%d]",
              stage->opcode, stage->rs1, stage->rs2, stage->imm,
              stage->opcode, stage->phy_rs1, stage->phy_rs2, stage->imm);
    }
  }

  if (strcmp(stage->opcode, "LOAD") == 0) {
    if (fetch_decode) {
      printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
    }
    else {
      printf("%s,R%d,R%d,R%d  [%s,P%d,P%d,#%d]",
              stage->opcode, stage->rd, stage->rs1, stage->rs2,
              stage->opcode, stage->phy_rd, stage->phy_rs1, stage->phy_rs2);
    }
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    if (fetch_decode) {
      printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
    }
    else {
      printf("%s,R%d,#%d  [%s,P%d,#%d]",
              stage->opcode, stage->rd, stage->imm,
              stage->opcode, stage->phy_rd, stage->imm);
    }
  }

  if (strcmp(stage->opcode, "ADD") == 0 ||
      strcmp(stage->opcode, "SUB") == 0 ||
      strcmp(stage->opcode, "AND") == 0 ||
      strcmp(stage->opcode, "OR") == 0 ||
      strcmp(stage->opcode, "EX-OR") == 0 ||
      strcmp(stage->opcode, "MUL") == 0) {

    if (fetch_decode) {
      printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
    else {
      printf("%s,R%d,R%d,R%d  [%s,P%d,P%d,P%d]",
              stage->opcode, stage->rd, stage->rs1, stage->rs2,
              stage->opcode, stage->phy_rd, stage->phy_rs1, stage->phy_rs2);
    }
  }

  if (strcmp(stage->opcode, "ADDL") == 0 ||
      strcmp(stage->opcode, "SUBL") == 0) {

    if (fetch_decode) {
      printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
    }
    else {
      printf("%s,R%d,R%d,#%d  [%s,P%d,P%d,#%d]",
              stage->opcode, stage->rd, stage->rs1, stage->imm,
              stage->opcode, stage->phy_rd, stage->phy_rs1, stage->imm);
    }
  }

  if (strcmp(stage->opcode, "HALT") == 0) {
    printf("%s", stage->opcode);
  }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */

static void
print_stage_content(char* name, APEX_CPU* cpu, enum Stages FU_type)
{
  CPU_Stage* stage = &cpu->stage[FU_type];
  if (strcmp(stage->opcode, "") != 0 && strlen(stage->opcode) <= 50) {
    if (strcmp(name, "ROB Retired Inst") == 0) {
      printf("%-15s: ", name);
      //printf("(cycle-%d) pc(%d) ", cpu->mem_cycle, stage->pc);
      print_instruction(0, stage);
    }
    if (strcmp(name, "Mem FU") == 0) {
      printf("%-15s: ", name);
      //printf("(cycle-%d) pc(%d) ", cpu->mem_cycle, stage->pc);
      print_instruction(0, stage);
    }
    if (strcmp(name, "Mul FU-1") == 0) {
      printf("%-15s: pc(%d) ", name, stage->pc);
      print_instruction(0, stage);
    }
    if (strcmp(name, "Mul FU-2") == 0) {
      printf("%-15s: pc(%d) ", name, stage->pc);
      print_instruction(0, stage);
    }
    if (strcmp(name, "Mul FU-3") == 0) {
      printf("%-15s: pc(%d) ", name, stage->pc);
      print_instruction(0, stage);
    }
    if (strcmp(name, "Int FU-2") == 0) {
      printf("%-15s: pc(%d) ", name, stage->pc);
      print_instruction(0, stage);
    }
    if (strcmp(name, "Int FU-1") == 0) {
      printf("%-15s: pc(%d) ", name, stage->pc);
      print_instruction(0, stage);
    }
    if (strcmp(name, "Fetch") == 0 || strcmp(name, "Decode/RF") == 0) {
      printf("%-15s: pc(%d) ", name, stage->pc);
      print_instruction(1, stage);
    }
  }
  else {
    printf("%-15s:", name);
  }
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];
  if (!stage->busy && !stage->stalled) {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    //printf("in fetch %d", stage->imm);
    stage->rd = current_ins->rd;

    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    cpu->stage[DRF] = cpu->stage[F];

  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Fetch", cpu , F);
}
  }
  else if(stage->stalled){
    stage->pc=cpu->pc; // to store non-updated pc
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch", cpu,F);
  }
  }


  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];
  if (strcmp(stage->opcode, "HALT") == 0) {
    CPU_Stage* stage1 = &cpu->stage[F];//object creation for[F]
    stage1->stalled=1;
    cpu->pc +=0;

}
if(stage->stalled){
  stage->stalled=0;
  cpu->stage[F].stalled=0;
  //cpu->stage[EX1] = cpu->stage[DRF];

}

  if (!stage->busy && !stage->stalled) {

    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {
    //  if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2]){
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
  //  }
    // else{
    //   cpu->stage[F].stalled=1;
    //   cpu->stage[DRF].stalled=1;
    // }

//////////////////////////////////////////////////////STORE//////////////////////////////////////////////
              int iq=1;
              int dest=1;
          if (iq && dest) {

            CPU_Stage* stage = &cpu->stage[DRF];

            int dest = 0;
            int src1 = 1;
            int src2 = 1;

            if(src1){

            if(cpu->rat[stage->rs1].phys_reg==-1){
            for(int i=0;i<PRF_ENTRIES;i++){
              if(cpu->prf[i].free)
              {
                cpu->rat[stage->rs1].phys_reg = i;

                stage->phy_rs1 = cpu->rat[stage->rs1].phys_reg;
              cpu->prf[i].free=0;
              break;
              }
            }
          }
          else{
            stage->phy_rs1 = cpu->rat[stage->rs1].phys_reg;
          }

        }
        if(src2){

        if(cpu->rat[stage->rs2].phys_reg==-1){
        for(int i=0;i<PRF_ENTRIES;i++){
          if(cpu->prf[i].free)
          {
            cpu->rat[stage->rs2].phys_reg = i;

            stage->phy_rs2 = cpu->rat[stage->rs2].phys_reg;
          cpu->prf[i].free=0;
          break;
          }
        }
      }
      else{
        stage->phy_rs2 = cpu->rat[stage->rs2].phys_reg;
      }

    }
    if(dest){

    if(cpu->rat[stage->rd].phys_reg==-1){
    for(int i=0;i<PRF_ENTRIES;i++){
      if(cpu->prf[i].free)
      {
        cpu->rat[stage->rd].phys_reg = i;

        stage->phy_rd = cpu->rat[stage->rd].phys_reg;

      cpu->prf[i].free=0;
      break;
      }
      }
    }
    else{
      stage->phy_rd = cpu->rat[stage->rd].phys_reg;
    }

    }
    /////////////////////////////////ROB/////////////////////////////
    ROB_entry* new_rob_entry = malloc(sizeof(*new_rob_entry));
    new_rob_entry->free = 0;
    strcpy(new_rob_entry->opcode, stage->opcode);
    new_rob_entry->pc = stage->pc;
    new_rob_entry->rd = stage->rd;
    new_rob_entry->phy_rd = stage->phy_rd;
    new_rob_entry->rs1 = stage->rs1;
    new_rob_entry->phy_rs1 = stage->phy_rs1;
    new_rob_entry->rs2 = stage->rs2;
    new_rob_entry->phy_rs2 = stage->phy_rs2;
    new_rob_entry->imm = stage->imm;
    if (strcmp(stage->opcode, "HALT") == 0) { new_rob_entry->status = 1; }
    else { new_rob_entry->status = 0; }
    //  new_rob_entry->branch_id = cpu->last_branch_id;
    stage->ROB_entry_id = push_rob_entry(cpu, new_rob_entry);
///////////////////////////////////////////IQ////////////////////////////////

LSQ_Entry* new_lsq_entry = malloc(sizeof(*new_lsq_entry));
new_lsq_entry->free = 0;
strcpy(new_lsq_entry->opcode, stage->opcode);
new_lsq_entry->pc = stage->pc;
new_lsq_entry->mem_address_valid = 0;
new_lsq_entry->mem_address = 0;
//new_lsq_entry->branch_id = cpu->last_branch_id;
new_lsq_entry->ROB_entry_id = stage->ROB_entry_id;
new_lsq_entry->rs1_ready = stage->rs1_valid;
new_lsq_entry->phy_rs1 = stage->phy_rs1;
new_lsq_entry->rs1 = stage->rs1;
new_lsq_entry->rs1_value = stage->rs1_value;
new_lsq_entry->phy_rs2 = stage->phy_rs2;
new_lsq_entry->rs2 = stage->rs2;
new_lsq_entry->imm = stage->imm;
new_lsq_entry->rd = stage->rd;
new_lsq_entry->phy_rd = stage->phy_rd;
stage->LSQ_index = pushLSQentry(cpu, new_lsq_entry);

              IQ_entry* new_iq_entry = malloc(sizeof(*new_iq_entry));
              new_iq_entry->pc = stage->pc;
              strcpy(new_iq_entry->opcode, stage->opcode);
              new_iq_entry->counter = 1;
              new_iq_entry->free = 0;
              new_iq_entry->FU_type = INT_FU1;
              new_iq_entry->imm = stage->imm;
              new_iq_entry->rs1 = stage->rs1;
              if (!src1) { new_iq_entry->rs1_ready = 1; }
              else { new_iq_entry->rs1_ready = stage->rs1_valid; }
              new_iq_entry->phy_rs1 = stage->phy_rs1;
              new_iq_entry->rs1_value = stage->rs1_value;
              new_iq_entry->rs2 = stage->rs2;
              if (!src2) { new_iq_entry->rs2_ready = 1; }
              else { new_iq_entry->rs2_ready = stage->rs2_valid; }
              new_iq_entry->phy_rs2 = stage->phy_rs2;
              new_iq_entry->rs2_value = stage->rs2_value;
              new_iq_entry->rd = stage->rd;
              new_iq_entry->phy_rd = stage->phy_rd;
              new_iq_entry->LSQ_index = stage->LSQ_index;
              new_iq_entry->ROB_entry_id = stage->ROB_entry_id;
              push_in_iq(cpu, new_iq_entry);
              cpu->stage[INT_FU1] = cpu->stage[DRF];
          }
          else {
            stage->stalled = 1;
          }
    }


///////////////////////////////////////////////////////////////////STR////////////////////////////////////////////////////////////////////
    if (strcmp(stage->opcode, "STR") == 0) {
      //if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2]){
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];

      stage->rdd = cpu->regs[stage->rd];
//}
// else{
//   cpu->stage[F].stalled=1;
//   cpu->stage[DRF].stalled=1;
// }



          int iq=1;
          int dest=1;
      if (iq && dest) {

        CPU_Stage* stage = &cpu->stage[DRF];

        int dest = 1;
        int src1 = 1;
        int src2 = 1;

        if(src1){

        if(cpu->rat[stage->rs1].phys_reg==-1){
        for(int i=0;i<PRF_ENTRIES;i++){
          if(cpu->prf[i].free)
          {
            cpu->rat[stage->rs1].phys_reg = i;

            stage->phy_rs1 = cpu->rat[stage->rs1].phys_reg;
          cpu->prf[i].free=0;
          break;
          }
        }
      }
      else{
        stage->phy_rs1 = cpu->rat[stage->rs1].phys_reg;
      }

    }
    if(src2){

    if(cpu->rat[stage->rs2].phys_reg==-1){
    for(int i=0;i<PRF_ENTRIES;i++){
      if(cpu->prf[i].free)
      {
        cpu->rat[stage->rs2].phys_reg = i;

        stage->phy_rs2 = cpu->rat[stage->rs2].phys_reg;
      cpu->prf[i].free=0;
      break;
      }
    }
  }
  else{
    stage->phy_rs2 = cpu->rat[stage->rs2].phys_reg;
  }

}
if(dest){

if(cpu->rat[stage->rd].phys_reg==-1){
for(int i=0;i<PRF_ENTRIES;i++){
  if(cpu->prf[i].free)
  {
    cpu->rat[stage->rd].phys_reg = i;

    stage->phy_rd = cpu->rat[stage->rd].phys_reg;

  cpu->prf[i].free=0;
  break;
  }
  }
}
else{
  stage->phy_rd = cpu->rat[stage->rd].phys_reg;
}

}
///////////////////////////ROB///////////////////////////////////////////
ROB_entry* new_rob_entry = malloc(sizeof(*new_rob_entry));
new_rob_entry->free = 0;
strcpy(new_rob_entry->opcode, stage->opcode);
new_rob_entry->pc = stage->pc;
new_rob_entry->rd = stage->rd;
new_rob_entry->phy_rd = stage->phy_rd;
new_rob_entry->rs1 = stage->rs1;
new_rob_entry->phy_rs1 = stage->phy_rs1;
new_rob_entry->rs2 = stage->rs2;
new_rob_entry->phy_rs2 = stage->phy_rs2;
new_rob_entry->imm = stage->imm;
if (strcmp(stage->opcode, "HALT") == 0) { new_rob_entry->status = 1; }
else { new_rob_entry->status = 0; }
//  new_rob_entry->branch_id = cpu->last_branch_id;
stage->ROB_entry_id = push_rob_entry(cpu, new_rob_entry);

LSQ_Entry* new_lsq_entry = malloc(sizeof(*new_lsq_entry));
new_lsq_entry->free = 0;
strcpy(new_lsq_entry->opcode, stage->opcode);
new_lsq_entry->pc = stage->pc;
new_lsq_entry->mem_address_valid = 0;
new_lsq_entry->mem_address = 0;
//new_lsq_entry->branch_id = cpu->last_branch_id;
new_lsq_entry->ROB_entry_id = stage->ROB_entry_id;
new_lsq_entry->rs1_ready = stage->rs1_valid;
new_lsq_entry->phy_rs1 = stage->phy_rs1;
new_lsq_entry->rs1 = stage->rs1;
new_lsq_entry->rs1_value = stage->rs1_value;
new_lsq_entry->phy_rs2 = stage->phy_rs2;
new_lsq_entry->rs2 = stage->rs2;
new_lsq_entry->imm = stage->imm;
new_lsq_entry->rd = stage->rd;
new_lsq_entry->phy_rd = stage->phy_rd;
stage->LSQ_index = pushLSQentry(cpu, new_lsq_entry);

          IQ_entry* new_iq_entry = malloc(sizeof(*new_iq_entry));
          new_iq_entry->pc = stage->pc;
          strcpy(new_iq_entry->opcode, stage->opcode);
          new_iq_entry->counter = 1;
          new_iq_entry->free = 0;
          new_iq_entry->FU_type = INT_FU1;
          new_iq_entry->imm = stage->imm;
          new_iq_entry->rs1 = stage->rs1;
          if (!src1) { new_iq_entry->rs1_ready = 1; }
          else { new_iq_entry->rs1_ready = stage->rs1_valid; }
          new_iq_entry->phy_rs1 = stage->phy_rs1;
          new_iq_entry->rs1_value = stage->rs1_value;
          new_iq_entry->rs2 = stage->rs2;
          if (!src2) { new_iq_entry->rs2_ready = 1; }
          else { new_iq_entry->rs2_ready = stage->rs2_valid; }
          new_iq_entry->phy_rs2 = stage->phy_rs2;
          new_iq_entry->rs2_value = stage->rs2_value;
          new_iq_entry->rd = stage->rd;
          new_iq_entry->phy_rd = stage->phy_rd;
          new_iq_entry->LSQ_index = stage->LSQ_index;
          new_iq_entry->ROB_entry_id = stage->ROB_entry_id;
          push_in_iq(cpu, new_iq_entry);
          cpu->stage[INT_FU1] = cpu->stage[DRF];
      }
      else {
        stage->stalled = 1;
      }
    }




//////////////////////////////////////////////////////////////LDR//////////////////////////////////////////////////////
    if (strcmp(stage->opcode, "LDR") == 0) {
    //  if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2]){
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      stage->rdd = cpu->regs[stage->rd];
//cpu->regs_valid[stage->rd]=0;
  //  }
    // else{
    //   cpu->stage[F].stalled=1;
    //   cpu->stage[DRF].stalled=1;
    // }

    int iq=1;
    int dest=1;
if (iq && dest) {

  CPU_Stage* stage = &cpu->stage[DRF];

  int dest = 1;
  int src1 = 1;
  int src2 = 1;

  if(src1){

  if(cpu->rat[stage->rs1].phys_reg==-1){
  for(int i=0;i<PRF_ENTRIES;i++){
    if(cpu->prf[i].free)
    {
      cpu->rat[stage->rs1].phys_reg = i;

      stage->phy_rs1 = cpu->rat[stage->rs1].phys_reg;
    cpu->prf[i].free=0;
    break;
    }
  }
}
else{
  stage->phy_rs1 = cpu->rat[stage->rs1].phys_reg;
}

}
if(src2){

if(cpu->rat[stage->rs2].phys_reg==-1){
for(int i=0;i<PRF_ENTRIES;i++){
if(cpu->prf[i].free)
{
  cpu->rat[stage->rs2].phys_reg = i;

  stage->phy_rs2 = cpu->rat[stage->rs2].phys_reg;
cpu->prf[i].free=0;
break;
}
}
}
else{
stage->phy_rs2 = cpu->rat[stage->rs2].phys_reg;
}

}
if(dest){

if(cpu->rat[stage->rd].phys_reg==-1){
for(int i=0;i<PRF_ENTRIES;i++){
if(cpu->prf[i].free)
{
cpu->rat[stage->rd].phys_reg = i;

stage->phy_rd = cpu->rat[stage->rd].phys_reg;

cpu->prf[i].free=0;
break;
}
}
}
else{
stage->phy_rd = cpu->rat[stage->rd].phys_reg;
}

}
//////////////////////STALLING//////////////


//////////////////////////////////////
ROB_entry* new_rob_entry = malloc(sizeof(*new_rob_entry));
new_rob_entry->free = 0;
strcpy(new_rob_entry->opcode, stage->opcode);
new_rob_entry->pc = stage->pc;
new_rob_entry->rd = stage->rd;
new_rob_entry->phy_rd = stage->phy_rd;
new_rob_entry->rs1 = stage->rs1;
new_rob_entry->phy_rs1 = stage->phy_rs1;
new_rob_entry->rs2 = stage->rs2;
new_rob_entry->phy_rs2 = stage->phy_rs2;
new_rob_entry->imm = stage->imm;
if (strcmp(stage->opcode, "HALT") == 0) { new_rob_entry->status = 1; }
else { new_rob_entry->status = 0; }
//  new_rob_entry->branch_id = cpu->last_branch_id;
stage->ROB_entry_id = push_rob_entry(cpu, new_rob_entry);

LSQ_Entry* new_lsq_entry = malloc(sizeof(*new_lsq_entry));
new_lsq_entry->free = 0;
strcpy(new_lsq_entry->opcode, stage->opcode);
new_lsq_entry->pc = stage->pc;
new_lsq_entry->mem_address_valid = 0;
new_lsq_entry->mem_address = 0;
//new_lsq_entry->branch_id = cpu->last_branch_id;
new_lsq_entry->ROB_entry_id = stage->ROB_entry_id;
new_lsq_entry->rs1_ready = stage->rs1_valid;
new_lsq_entry->phy_rs1 = stage->phy_rs1;
new_lsq_entry->rs1 = stage->rs1;
new_lsq_entry->rs1_value = stage->rs1_value;
new_lsq_entry->phy_rs2 = stage->phy_rs2;
new_lsq_entry->rs2 = stage->rs2;
new_lsq_entry->imm = stage->imm;
new_lsq_entry->rd = stage->rd;
new_lsq_entry->phy_rd = stage->phy_rd;
stage->LSQ_index = pushLSQentry(cpu, new_lsq_entry);

    IQ_entry* new_iq_entry = malloc(sizeof(*new_iq_entry));
    new_iq_entry->pc = stage->pc;
    strcpy(new_iq_entry->opcode, stage->opcode);
    new_iq_entry->counter = 1;
    new_iq_entry->free = 0;
    new_iq_entry->FU_type = INT_FU1;
    new_iq_entry->imm = stage->imm;
    new_iq_entry->rs1 = stage->rs1;
    if (!src1) { new_iq_entry->rs1_ready = 1; }
    else { new_iq_entry->rs1_ready = stage->rs1_valid; }
    new_iq_entry->phy_rs1 = stage->phy_rs1;
    new_iq_entry->rs1_value = stage->rs1_value;
    new_iq_entry->rs2 = stage->rs2;
    if (!src2) { new_iq_entry->rs2_ready = 1; }
    else { new_iq_entry->rs2_ready = stage->rs2_valid; }
    new_iq_entry->phy_rs2 = stage->phy_rs2;
    new_iq_entry->rs2_value = stage->rs2_value;
    new_iq_entry->rd = stage->rd;
    new_iq_entry->phy_rd = stage->phy_rd;
    new_iq_entry->LSQ_index = stage->LSQ_index;
    new_iq_entry->ROB_entry_id = stage->ROB_entry_id;
    push_in_iq(cpu, new_iq_entry);
    cpu->stage[INT_FU1] = cpu->stage[DRF];
}
else {
  stage->stalled = 1;
}
}



//////////////////////////////////////////////////////////////////////////LOAD///////////////////////////////////////////////

    if (strcmp(stage->opcode, "LOAD") == 0) {
    //  if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2]){
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->imm = cpu->regs[stage->imm];
      stage->rdd = cpu->regs[stage->rd];
  //    cpu->regs_valid[stage->rd]=0;
//}

    // else{
    //   cpu->stage[F].stalled=1;
    //   cpu->stage[DRF].stalled=1;
    // }
      int lsq=1;
      int dest=1;
  if (lsq && dest) {

    CPU_Stage* stage = &cpu->stage[DRF];

    int dest = 1;
    int src1 = 1;
    int src2 = 0;

    if(src1){

    if(cpu->rat[stage->rs1].phys_reg==-1){
    for(int i=0;i<PRF_ENTRIES;i++){
      if(cpu->prf[i].free)
      {
        cpu->rat[stage->rs1].phys_reg = i;

        stage->phy_rs1 = cpu->rat[stage->rs1].phys_reg;
      cpu->prf[i].free=0;
      break;
      }
    }
  }
  else{
    stage->phy_rs1 = cpu->rat[stage->rs1].phys_reg;
  }

}
if(src2){

if(cpu->rat[stage->rs2].phys_reg==-1){
for(int i=0;i<PRF_ENTRIES;i++){
  if(cpu->prf[i].free)
  {
    cpu->rat[stage->rs2].phys_reg = i;

    stage->phy_rs2 = cpu->rat[stage->rs2].phys_reg;
  cpu->prf[i].free=0;
  break;
  }
}
}
else{
stage->phy_rs2 = cpu->rat[stage->rs2].phys_reg;
}

}
if(dest){

if(cpu->rat[stage->rd].phys_reg==-1){
for(int i=0;i<PRF_ENTRIES;i++){
if(cpu->prf[i].free)
{
cpu->rat[stage->rd].phys_reg = i;

stage->phy_rd = cpu->rat[stage->rd].phys_reg;

cpu->prf[i].free=0;
break;
}
}
}
else{
stage->phy_rd = cpu->rat[stage->rd].phys_reg;
}

}

ROB_entry* new_rob_entry = malloc(sizeof(*new_rob_entry));
new_rob_entry->free = 0;
strcpy(new_rob_entry->opcode, stage->opcode);
new_rob_entry->pc = stage->pc;
new_rob_entry->rd = stage->rd;
new_rob_entry->phy_rd = stage->phy_rd;
new_rob_entry->rs1 = stage->rs1;
new_rob_entry->phy_rs1 = stage->phy_rs1;
new_rob_entry->rs2 = stage->rs2;
new_rob_entry->phy_rs2 = stage->phy_rs2;
new_rob_entry->imm = stage->imm;
if (strcmp(stage->opcode, "HALT") == 0) { new_rob_entry->status = 1; }
else { new_rob_entry->status = 0; }
//  new_rob_entry->branch_id = cpu->last_branch_id;
stage->ROB_entry_id = push_rob_entry(cpu, new_rob_entry);


LSQ_Entry* new_lsq_entry = malloc(sizeof(*new_lsq_entry));
new_lsq_entry->free = 0;
strcpy(new_lsq_entry->opcode, stage->opcode);
new_lsq_entry->pc = stage->pc;
new_lsq_entry->mem_address_valid = 0;
new_lsq_entry->mem_address = 0;
//new_lsq_entry->branch_id = cpu->last_branch_id;
new_lsq_entry->ROB_entry_id = stage->ROB_entry_id;
new_lsq_entry->rs1_ready = stage->rs1_valid;
new_lsq_entry->phy_rs1 = stage->phy_rs1;
new_lsq_entry->rs1 = stage->rs1;
new_lsq_entry->rs1_value = stage->rs1_value;
new_lsq_entry->phy_rs2 = stage->phy_rs2;
new_lsq_entry->rs2 = stage->rs2;
new_lsq_entry->imm = stage->imm;
new_lsq_entry->rd = stage->rd;
new_lsq_entry->phy_rd = stage->phy_rd;
stage->LSQ_index = pushLSQentry(cpu, new_lsq_entry);

IQ_entry* new_iq_entry = malloc(sizeof(*new_iq_entry));
new_iq_entry->pc = stage->pc;
strcpy(new_iq_entry->opcode, stage->opcode);
new_iq_entry->counter = 1;
new_iq_entry->free = 0;
new_iq_entry->FU_type = INT_FU1;
new_iq_entry->imm = stage->imm;
new_iq_entry->rs1 = stage->rs1;
if (!src1) { new_iq_entry->rs1_ready = 1; }
else { new_iq_entry->rs1_ready = stage->rs1_valid; }
new_iq_entry->phy_rs1 = stage->phy_rs1;
new_iq_entry->rs1_value = stage->rs1_value;
new_iq_entry->rs2 = stage->rs2;
if (!src2) { new_iq_entry->rs2_ready = 1; }
else { new_iq_entry->rs2_ready = stage->rs2_valid; }
new_iq_entry->phy_rs2 = stage->phy_rs2;
new_iq_entry->rs2_value = stage->rs2_value;
new_iq_entry->rd = stage->rd;
new_iq_entry->phy_rd = stage->phy_rd;
new_iq_entry->LSQ_index = stage->LSQ_index;
new_iq_entry->ROB_entry_id = stage->ROB_entry_id;
push_in_iq(cpu, new_iq_entry);

cpu->stage[INT_FU1] = cpu->stage[DRF];

  }
  else {
    stage->stalled = 1;
  }
    }


///////////////////////////////////////////////////////////////MUL////////////////////////////////////////////////////////
    if (strcmp(stage->opcode, "MUL") == 0) {
      cpu->mul_display=1;
    //  if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2]){
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      stage->rdd = cpu->regs[stage->rd];
    //  cpu->regs_valid[stage->rd]=0;
    //  cpu->stage[MUL_FU1] = cpu->stage[DRF];
//}
// else{
//   cpu->stage[F].stalled=1;
//   cpu->stage[DRF].stalled=1;
// }
      int iq=1;
      int dest=1;
  if (iq && dest) {

    CPU_Stage* stage = &cpu->stage[DRF];

    int dest = 1;
    int src1 = 1;
    int src2 = 1;

    if(src1){

    if(cpu->rat[stage->rs1].phys_reg==-1){
    for(int i=0;i<PRF_ENTRIES;i++){
      if(cpu->prf[i].free)
      {
        cpu->rat[stage->rs1].phys_reg = i;

        stage->phy_rs1 = cpu->rat[stage->rs1].phys_reg;
      cpu->prf[i].free=0;
      break;
      }
    }
  }
  else{
    stage->phy_rs1 = cpu->rat[stage->rs1].phys_reg;
  }

}
if(src2){

if(cpu->rat[stage->rs2].phys_reg==-1){
for(int i=0;i<PRF_ENTRIES;i++){
  if(cpu->prf[i].free)
  {
    cpu->rat[stage->rs2].phys_reg = i;

    stage->phy_rs2 = cpu->rat[stage->rs2].phys_reg;
  cpu->prf[i].free=0;
  break;
  }
}
}
else{
stage->phy_rs2 = cpu->rat[stage->rs2].phys_reg;
}

}
if(dest){

if(cpu->rat[stage->rd].phys_reg==-1){
for(int i=0;i<PRF_ENTRIES;i++){
if(cpu->prf[i].free)
{
cpu->rat[stage->rd].phys_reg = i;

stage->phy_rd = cpu->rat[stage->rd].phys_reg;

cpu->prf[i].free=0;
break;
}
}
}
else{
stage->phy_rd = cpu->rat[stage->rd].phys_reg;
}

}
//////////////////////////////////////ROB///////////////////////////////////
ROB_entry* new_rob_entry = malloc(sizeof(*new_rob_entry));
new_rob_entry->free = 0;
strcpy(new_rob_entry->opcode, stage->opcode);
new_rob_entry->pc = stage->pc;
new_rob_entry->rd = stage->rd;
new_rob_entry->phy_rd = stage->phy_rd;
new_rob_entry->rs1 = stage->rs1;
new_rob_entry->phy_rs1 = stage->phy_rs1;
new_rob_entry->rs2 = stage->rs2;
new_rob_entry->phy_rs2 = stage->phy_rs2;
new_rob_entry->imm = stage->imm;
if (strcmp(stage->opcode, "HALT") == 0) { new_rob_entry->status = 1; }
else { new_rob_entry->status = 0; }
//  new_rob_entry->branch_id = cpu->last_branch_id;
stage->ROB_entry_id = push_rob_entry(cpu, new_rob_entry);


      IQ_entry* new_iq_entry = malloc(sizeof(*new_iq_entry));
      new_iq_entry->pc = stage->pc;
      strcpy(new_iq_entry->opcode, stage->opcode);
      new_iq_entry->counter = 1;
      new_iq_entry->free = 0;
      new_iq_entry->FU_type = MUL_FU1;
      new_iq_entry->imm = stage->imm;
      new_iq_entry->rs1 = stage->rs1;
      if (!src1) { new_iq_entry->rs1_ready = 1; }
      else { new_iq_entry->rs1_ready = stage->rs1_valid; }
      new_iq_entry->phy_rs1 = stage->phy_rs1;
      new_iq_entry->rs1_value = stage->rs1_value;
      new_iq_entry->rs2 = stage->rs2;
      if (!src2) { new_iq_entry->rs2_ready = 1; }
      else { new_iq_entry->rs2_ready = stage->rs2_valid; }
      new_iq_entry->phy_rs2 = stage->phy_rs2;
      new_iq_entry->rs2_value = stage->rs2_value;
      new_iq_entry->rd = stage->rd;
      new_iq_entry->phy_rd = stage->phy_rd;
      new_iq_entry->LSQ_index = stage->LSQ_index;
      new_iq_entry->ROB_entry_id = stage->ROB_entry_id;
      push_in_iq(cpu, new_iq_entry);

      cpu->stage[MUL_FU1] = cpu->stage[DRF];

  }
  else {
    stage->stalled = 1;
  }

    }
    ////////////////////////////////////////////////////ADD and all/////////////////////////////////////////////
    if (strcmp(stage->opcode, "ADD") == 0 ||
        strcmp(stage->opcode, "SUB") == 0 ||
        strcmp(stage->opcode, "AND") == 0 ||
        strcmp(stage->opcode, "OR") == 0 ||
        strcmp(stage->opcode, "EX-OR") == 0) {
          //  if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2]){
              cpu->stage[F].stalled=0;
              cpu->stage[DRF].stalled=0;

          stage->rdd = cpu->regs[stage->rd];
     stage->rs1_value = cpu->regs[stage->rs1];
     stage->rs2_value = cpu->regs[stage->rs2];
    // cpu->regs_valid[stage->rd]=0;
//}
// else{
//   cpu->stage[F].stalled=1;
//   cpu->stage[DRF].stalled=1;
// }
          int iq=1;
          int dest=1;
      if (iq && dest) {

        CPU_Stage* stage = &cpu->stage[DRF];

        int dest = 1;
        int src1 = 1;
        int src2 = 1;

        if(src1){

        if(cpu->rat[stage->rs1].phys_reg==-1){
        for(int i=0;i<PRF_ENTRIES;i++){
          if(cpu->prf[i].free)
          {
            cpu->rat[stage->rs1].phys_reg = i;

            stage->phy_rs1 = cpu->rat[stage->rs1].phys_reg;
          cpu->prf[i].free=0;
          break;
          }
        }
      }
      else{
        stage->phy_rs1 = cpu->rat[stage->rs1].phys_reg;
      }

    }
    if(src2){

    if(cpu->rat[stage->rs2].phys_reg==-1){
    for(int i=0;i<PRF_ENTRIES;i++){
      if(cpu->prf[i].free)
      {
        cpu->rat[stage->rs2].phys_reg = i;

        stage->phy_rs2 = cpu->rat[stage->rs2].phys_reg;
      cpu->prf[i].free=0;
      break;
      }
    }
  }
  else{
    stage->phy_rs2 = cpu->rat[stage->rs2].phys_reg;
  }

}
if(dest){

if(cpu->rat[stage->rd].phys_reg==-1){
for(int i=0;i<PRF_ENTRIES;i++){
  if(cpu->prf[i].free)
  {
    cpu->rat[stage->rd].phys_reg = i;

    stage->phy_rd = cpu->rat[stage->rd].phys_reg;

  cpu->prf[i].free=0;
  break;
  }
  }
}
else{
  stage->phy_rd = cpu->rat[stage->rd].phys_reg;
}

}
////////////////////////////////////////////ROB////////////////////////////////
ROB_entry* new_rob_entry = malloc(sizeof(*new_rob_entry));
new_rob_entry->free = 0;
strcpy(new_rob_entry->opcode, stage->opcode);
new_rob_entry->pc = stage->pc;
new_rob_entry->rd = stage->rd;
new_rob_entry->phy_rd = stage->phy_rd;
new_rob_entry->rs1 = stage->rs1;
new_rob_entry->phy_rs1 = stage->phy_rs1;
new_rob_entry->rs2 = stage->rs2;
new_rob_entry->phy_rs2 = stage->phy_rs2;
new_rob_entry->imm = stage->imm;
if (strcmp(stage->opcode, "HALT") == 0) { new_rob_entry->status = 1; }
else { new_rob_entry->status = 0; }
//  new_rob_entry->branch_id = cpu->last_branch_id;
stage->ROB_entry_id = push_rob_entry(cpu, new_rob_entry);

          IQ_entry* new_iq_entry = malloc(sizeof(*new_iq_entry));
          new_iq_entry->pc = stage->pc;
          strcpy(new_iq_entry->opcode, stage->opcode);
          new_iq_entry->counter = 1;
          new_iq_entry->free = 0;
          new_iq_entry->FU_type = INT_FU1;
          new_iq_entry->imm = stage->imm;
          new_iq_entry->rs1 = stage->rs1;
          if (!src1) { new_iq_entry->rs1_ready = 1; }
          else { new_iq_entry->rs1_ready = stage->rs1_valid; }
          new_iq_entry->phy_rs1 = stage->phy_rs1;
          new_iq_entry->rs1_value = stage->rs1_value;
          new_iq_entry->rs2 = stage->rs2;
          if (!src2) { new_iq_entry->rs2_ready = 1; }
          else { new_iq_entry->rs2_ready = stage->rs2_valid; }
          new_iq_entry->phy_rs2 = stage->phy_rs2;
          new_iq_entry->rs2_value = stage->rs2_value;
          new_iq_entry->rd = stage->rd;
          new_iq_entry->phy_rd = stage->phy_rd;
          new_iq_entry->LSQ_index = stage->LSQ_index;
          new_iq_entry->ROB_entry_id = stage->ROB_entry_id;
          push_in_iq(cpu, new_iq_entry);
           cpu->stage[INT_FU1] = cpu->stage[DRF];
      }
      else {
        stage->stalled = 1;
      }
    }
    ////////////////////////////////////////////////// EDIT UPTO THIS /////////////////////////////////////
    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {

      stage->buffer = stage->imm;
      //cpu->regs_valid[stage->rd]=0;

    }

    /* Copy data from decode latch to execute latch*/
//     if(!cpu->mul_display){
//     cpu->stage[INT_FU1] = cpu->stage[DRF];
// }
// else{
//   cpu->stage[MUL_FU1] = cpu->stage[DRF];
// //  cpu->mul_display = 0;
// }
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode/RF", cpu,DRF);
    }
  }
  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
int_FU1(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[INT_FU1];

    if (!stage->busy && !stage->stalled){
    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs[stage->rd] = stage->imm;
    printf("REGISTER : %d\n",cpu->regs[stage->rd]);
      cpu->regs_valid[stage->rd]=1;

   }
   if (strcmp(stage->opcode, "ADD") == 0) {
     printf("Value passed= %d\n",stage->rs1_value);
     stage->rdd= stage->rs1_value + stage->rs2_value;
     cpu->regs[stage->rd] = stage->rdd;
    printf("REGISTER : %d\n",cpu->regs[stage->rd]);


   }
   if (strcmp(stage->opcode, "SUB") == 0) {
     stage->rdd=stage->rs1_value - stage->rs2_value;
     cpu->regs[stage->rd] = stage->rdd;

   }

   if (strcmp(stage->opcode, "AND") == 0) {
      stage->rdd = stage->rs1_value & stage->rs2_value;
      cpu->regs[stage->rd] = stage->rdd;

   }

       if (strcmp(stage->opcode, "OR") == 0) {
       stage->rdd = stage->rs1_value | stage->rs2_value;
       cpu->regs[stage->rd] = stage->rdd;

   }

       if (strcmp(stage->opcode, "EX-OR") == 0) {
         stage->rdd = stage->rs1_value ^ stage->rs2_value;
         cpu->regs[stage->rd] = stage->rdd;
       }

       if (strcmp(stage->opcode, "ADDL") == 0) {
         stage->rdd = stage->rs1_value + stage->imm;
         cpu->regs[stage->rd] = stage->rdd;
       }

       if (strcmp(stage->opcode, "SUBL") == 0) {
         stage->rdd = stage->rs1_value - stage->imm;
         cpu->regs[stage->rd] = stage->rdd;
       }

       if (strcmp(stage->opcode, "LOAD") == 0) {
         stage->buffer = stage->rs1_value + stage->imm;
         cpu->regs[stage->rd] = cpu->data_memory[stage->buffer];
       }
       if (strcmp(stage->opcode, "LDR") == 0) {
         stage->buffer = stage->rs1_value + stage->rs2_value;
         cpu->regs[stage->rd] = cpu->data_memory[stage->buffer];
       }
       if (strcmp(stage->opcode, "STORE") == 0) {
         stage->buffer = stage->rs2_value + stage->imm;
         cpu->data_memory[stage->buffer] =  stage->rs1_value;
       }
       if (strcmp(stage->opcode, "STR") == 0) {
         stage->buffer = stage->rs1_value + stage->rs2_value;
         cpu->data_memory[stage->buffer] =  stage->rd;
       }


    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[INT_FU2] = cpu->stage[INT_FU1];
//  stage->pc = stage->pc+4;
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Int FU-1", cpu,INT_FU1);
    }
  }
  return 0;
}

int
int_FU2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[INT_FU2];

    if (!stage->busy && !stage->stalled){

      if (strcmp(stage->opcode, "MOVC") == 0) {
        cpu->regs[stage->rd] = stage->buffer;
        printf("REGISTER : %d\n",cpu->regs[stage->rd]);

     }
    //  if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 ||
    //  strcmp(stage->opcode, "AND") == 0 ||
    //  strcmp(stage->opcode, "OR") == 0 ||
    //  strcmp(stage->opcode, "EX-OR") == 0||
    // strcmp(stage->opcode, "ADDL") == 0
    //  strcmp(stage->opcode, "SUB") == 0
    //  strcmp(stage->opcode, "LOAD") == 0
    //  strcmp(stage->opcode, "STORE") == 0
    //  strcmp(stage->opcode, "LDR") == 0
    //  strcmp(stage->opcode, "STR") == 0
    //  strcmp(stage->opcode, "MOVC") == 0)
    if (strcmp(stage->opcode, "ADD") == 0 ||
        strcmp(stage->opcode, "SUB") == 0 ||
        strcmp(stage->opcode, "AND") == 0 ||
        strcmp(stage->opcode, "OR") == 0 ||
        strcmp(stage->opcode, "MOVC") == 0)
    {


        cpu->ins_completed++;
     }

         printf("INS - %d, CMEM - %d\n",cpu->ins_completed,cpu->code_memory_size );


    /* Copy data from Execute latch to Memory latch*/
  // cpu->stage[MEM_FU] = cpu->stage[INT_FU2];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Int FU-2",cpu,INT_FU2);
    }
  }
  return 0;

}
/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
 int
 mul_FU1(APEX_CPU* cpu)
 {
   CPU_Stage* stage = &cpu->stage[MUL_FU1];

     if (!stage->busy && !stage->stalled){

     /* MOVC */
     if (strcmp(stage->opcode, "MOVC") == 0) {
       stage->imm=stage->imm+0;

     }

     if (strcmp(stage->opcode, "MUL") == 0) {

       stage->rdd=stage->rs1_value * stage->rs2_value;
       cpu->regs[stage->rd] = stage->rdd;
       printf("MUL %d\n",cpu->regs[stage->rd] );

     }



     /* Copy data from Execute latch to Memory latch*/
     cpu->stage[MUL_FU2] = cpu->stage[MUL_FU1];


       print_stage_content("Mul FU-1",cpu,MUL_FU1);

   }
   return 0;
 }

 int
 mul_FU2(APEX_CPU* cpu)
 {
   CPU_Stage* stage = &cpu->stage[MUL_FU2];

     if (!stage->busy && !stage->stalled){
     /* MOVC */
     if (strcmp(stage->opcode, "MUL") == 0) {


     }

     /* Copy data from Execute latch to Memory latch*/
     cpu->stage[MUL_FU3] = cpu->stage[MUL_FU2];

     if (ENABLE_DEBUG_MESSAGES) {
       print_stage_content("Mul FU-2",cpu,MUL_FU2);
     }
   }
   return 0;
 }

 int
 mul_FU3(APEX_CPU* cpu)
 {
   CPU_Stage* stage = &cpu->stage[MUL_FU3];

     if (!stage->busy && !stage->stalled){
     /* MOVC */
     if (strcmp(stage->opcode, "MOVC") == 0) {
       stage->imm=stage->imm+0;

     }


          if (strcmp(stage->opcode, "MUL") == 0) {

          //cpu->ins_completed++;

          }

     /* Copy data from Execute latch to Memory latch*/
     //cpu->stage[BRANCH_FU] = cpu->stage[MUL_FU3];

     if (ENABLE_DEBUG_MESSAGES) {
       print_stage_content("Mul FU-3",cpu,MUL_FU3);
     }
   }
   return 0;
 }

 int
 mem_FU1(APEX_CPU* cpu)
 {
   CPU_Stage* stage = &cpu->stage[MEM_FU];

     if (!stage->busy && !stage->stalled){
     /* MOVC */
     if (strcmp(stage->opcode, "MOVC") == 0) {
       stage->imm=stage->imm+0;

     }

     /* Copy data from Execute latch to Memory latch*/
     //cpu->stage[EX2] = cpu->stage[EX1];

     if (ENABLE_DEBUG_MESSAGES) {
       print_stage_content("Mem FU",cpu,MEM_FU);
     }
   }
   return 0;
 }




/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu)
{
  while (1) {

    /* All the instructions committed, so exit */
    if (cpu->ins_completed == cpu->code_memory_size) {
      printf("(apex) >> Simulation Complete \n");
      {
        printf("\n======================== STATE OF REGISTER FILE ========================\n");
        for (int i = 0; i < PRF_ENTRIES; i++) {
          if (1) {
            printf("         |\tPRF[%d]\t|\tValue = %d\t|\tStatus = 1\t|\n",
                  i, cpu->regs[i]);
          }
        }
        printf("================================================================================\n");
        printf("\n============================= STATE OF DATA MEMORY =============================\n");
        for (int i = 0; i < 100; i++) {
          printf("                     |\tMEM[%d]\t|\tData Value = %d\t|\n",
                  i, cpu->data_memory[i]);
        }
        printf("================================================================================\n\n");
      }
  // for(i=0;i<6;i++)
  // {
  //   printf("M%d-%d\n",i,cpu->data_memory[i] );
  // }

      break;
    }

    if (ENABLE_DEBUG_MESSAGES) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }
    if (commit_rob_entry(cpu)) {
      commit_rob_entry(cpu);
    }
    mem_FU1(cpu);
  //  branch_FU(cpu);
    mul_FU3(cpu);
    mul_FU2(cpu);
    mul_FU1(cpu);
    int_FU2(cpu);
    int_FU1(cpu);
    for(int i=0;)
    if (ENABLE_DEBUG_MESSAGES) { display_iq(cpu); }
    process_iq(cpu);
    if (ENABLE_DEBUG_MESSAGES){
        display_rob(cpu);
    displayLSQ(cpu);}
  processLSQ(cpu);

    decode(cpu);
    fetch(cpu);


    cpu->clock++;
     cpu->commitments = 0;

  }

  return 0;
}
