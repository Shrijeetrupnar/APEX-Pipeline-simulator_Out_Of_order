/*
 *  LSQ.c
 *
 *  Author :
 *  shrijeet rupnar (srupnar1@binghamton.edu)
 *  State University of New York, Binghamton
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"


int
isLSQentryfree(APEX_CPU* cpu)
{
  if (cpu->lsq.LSQ_entry[cpu->lsq.tail].free) {
    return 1;
  }
  return 0;
}

int
pushLSQentry(APEX_CPU* cpu, LSQ_Entry* new_LSQ_entry)
{
  int free_entry = cpu->lsq.tail;
  cpu->lsq.LSQ_entry[free_entry].free = new_LSQ_entry->free;
  strcpy(cpu->lsq.LSQ_entry[free_entry].opcode, new_LSQ_entry->opcode);
  cpu->lsq.LSQ_entry[free_entry].pc = new_LSQ_entry->pc;
  cpu->lsq.LSQ_entry[free_entry].mem_address_valid = new_LSQ_entry->mem_address_valid;
  cpu->lsq.LSQ_entry[free_entry].mem_address = new_LSQ_entry->mem_address;
//cpu->lsq.LSQ_entry[free_entry].branch_id = new_LSQ_entry->branch_id;
  cpu->lsq.LSQ_entry[free_entry].ROB_entry_id = new_LSQ_entry->ROB_entry_id;
  cpu->lsq.LSQ_entry[free_entry].rs1_ready = new_LSQ_entry->rs1_ready;
  cpu->lsq.LSQ_entry[free_entry].phy_rs1 = new_LSQ_entry->phy_rs1;
  cpu->lsq.LSQ_entry[free_entry].rs1 = new_LSQ_entry->rs1;
  cpu->lsq.LSQ_entry[free_entry].rs1_value = new_LSQ_entry->rs1_value;
  cpu->lsq.LSQ_entry[free_entry].phy_rs2 = new_LSQ_entry->phy_rs2;
  cpu->lsq.LSQ_entry[free_entry].rs2 = new_LSQ_entry->rs2;
  cpu->lsq.LSQ_entry[free_entry].imm = new_LSQ_entry->imm;
  cpu->lsq.LSQ_entry[free_entry].rd = new_LSQ_entry->rd;
  cpu->lsq.LSQ_entry[free_entry].phy_rd = new_LSQ_entry->phy_rd;

  cpu->lsq.tail ++;
  if (cpu->lsq.tail == LSQ_ENTRIES)
  {
    cpu->lsq.tail = 0;
  }
  return free_entry;
}

void
getinstructiontoMEM(APEX_CPU* cpu)
{
  int push_to_MEM = 0;
  int entry = cpu->lsq.head;
  if (!cpu->lsq.LSQ_entry[entry].free &&  cpu->lsq.LSQ_entry[entry].mem_address_valid && !cpu->stage[MEM_FU].stalled)
      {

    if (strcmp(cpu->lsq.LSQ_entry[entry].opcode, "STORE") == 0)
     {
      if (cpu->lsq.LSQ_entry[entry].rs1_ready && cpu->commitments != 2)
       {
        int rob_head = cpu->rob.head;
        if (strcmp(cpu->rob.rob_entry[rob_head].opcode, "STORE") == 0)
        {
          push_to_MEM = 1;
          // delete from ROB
          //remove_store_from_rob(cpu);
        }
      }
    }
    else {
      push_to_MEM = 1;
    }
  }

  if (push_to_MEM)
  {
    cpu->stage[MEM_FU].pc = cpu->lsq.LSQ_entry[entry].pc;
    strcpy(cpu->stage[MEM_FU].opcode, cpu->lsq.LSQ_entry[entry].opcode);
    cpu->stage[MEM_FU].rd = cpu->lsq.LSQ_entry[entry].rd;
    cpu->stage[MEM_FU].phy_rd = cpu->lsq.LSQ_entry[entry].phy_rd;
    cpu->stage[MEM_FU].phy_rs1 = cpu->lsq.LSQ_entry[entry].phy_rs1;
    cpu->stage[MEM_FU].rs1 = cpu->lsq.LSQ_entry[entry].rs1;
    cpu->stage[MEM_FU].phy_rs2 = cpu->lsq.LSQ_entry[entry].phy_rs2;
    cpu->stage[MEM_FU].rs2 = cpu->lsq.LSQ_entry[entry].rs2;
    cpu->stage[MEM_FU].imm = cpu->lsq.LSQ_entry[entry].imm;
    cpu->stage[MEM_FU].rs1_value = cpu->lsq.LSQ_entry[entry].rs1_value;
    cpu->stage[MEM_FU].mem_address = cpu->lsq.LSQ_entry[entry].mem_address;
    cpu->stage[MEM_FU].ROB_entry_id = cpu->lsq.LSQ_entry[entry].ROB_entry_id;
    //cpu->stage[MEM_FU].branch_id = cpu->lsq.LSQ_entry[entry].branch_id;
    cpu->stage[MEM_FU].busy = 0;
    cpu->stage[MEM_FU].stalled = 0;

    cpu->lsq.LSQ_entry[entry].free = 1;
    cpu->lsq.head++;
    if (cpu->lsq.head == LSQ_ENTRIES) {
      cpu->lsq.head = 0;
    }
  }
}

void
updateLSQentry(APEX_CPU* cpu, enum Stages FU_type)
{
  int LSQ_index = cpu->stage[FU_type].LSQ_index;
  cpu->lsq.LSQ_entry[LSQ_index].mem_address = cpu->stage[FU_type].buffer;
  cpu->lsq.LSQ_entry[LSQ_index].mem_address_valid = 1;
}

// void
// broadcastresultintoLSQ(APEX_CPU* cpu, enum Stages FU_type)
// {
//   for (int i = 0; i < LSQ_ENTRIES; i++) {
//     if (!cpu->lsq.LSQ_entry[i].free &&
//         cpu->lsq.LSQ_entry[i].phy_rs1 == cpu->stage[FU_type].phy_rd) {
//       cpu->lsq.LSQ_entry[i].rs1_value = cpu->stage[FU_type].buffer;
//       cpu->lsq.LSQ_entry[i].rs1_ready = 1;
//     }
//   }
// }



void
displayLSQ(APEX_CPU* cpu)
{
  printf("------------------------------- Load Store Queue --------------------------------\n");
  for (int i = 0; i < LSQ_ENTRIES; i++) {
    if (!cpu->lsq.LSQ_entry[i].free || i == cpu->lsq.tail) {

      printf("| Index = %d | ", i);

      if (i == cpu->lsq.tail) {
        printf("t |");
      }
      else {
        printf("  |");
      }

      if (i == cpu->lsq.head) {
        printf(" h |\t");
      }
      else {
        printf("   |\t");
      }

      //printf("\t");
      if (!cpu->lsq.LSQ_entry[i].free) {
      printf("pc(%d)  ", cpu->lsq.LSQ_entry[i].pc);
        CPU_Stage* instruction_to_print = malloc(sizeof(*instruction_to_print));
        strcpy(instruction_to_print->opcode, cpu->lsq.LSQ_entry[i].opcode);
        instruction_to_print->rs1 = cpu->lsq.LSQ_entry[i].rs1;
        instruction_to_print->phy_rs1 = cpu->lsq.LSQ_entry[i].phy_rs1;
        instruction_to_print->rs2 = cpu->lsq.LSQ_entry[i].rs2;
        instruction_to_print->phy_rs2 = cpu->lsq.LSQ_entry[i].phy_rs2;
        instruction_to_print->rd = cpu->lsq.LSQ_entry[i].rd;
        instruction_to_print->phy_rd = cpu->lsq.LSQ_entry[i].phy_rd;
        instruction_to_print->imm = cpu->lsq.LSQ_entry[i].imm;
      //  print_instruction(0, instruction_to_print);
      printf("%s,R%d,R%d,R%d  [%s,P%d,P%d,P%d]",
            cpu->lsq.LSQ_entry[i].opcode  , cpu->lsq.LSQ_entry[i].rd,cpu->lsq.LSQ_entry[i].rs1 , cpu->lsq.LSQ_entry[i].rs2,
              cpu->lsq.LSQ_entry[i].opcode, cpu->lsq.LSQ_entry[i].phy_rd, cpu->lsq.LSQ_entry[i].phy_rs1, cpu->lsq.LSQ_entry[i].phy_rs2);
        printf("\t|");
      }
      printf("\n");
    }
  }
  printf("---------------------------------------------------------------------------------\n\n");
}

int
isLSQempty(APEX_CPU* cpu)
{
  for (int i = 0; i < LSQ_ENTRIES; i++) {
    if (!cpu->lsq.LSQ_entry[i].free){
      return 0;
    }
  }
  return 1;
}



void
processLSQ(APEX_CPU* cpu)
{
  getinstructiontoMEM(cpu);
}
