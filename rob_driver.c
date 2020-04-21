#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"




int
push_rob_entry(APEX_CPU* cpu, ROB_entry* new_rob_entry)
{
  int free_entry = cpu->rob.tail;
  cpu->rob.rob_entry[free_entry].free = new_rob_entry->free; //why
  strcpy(cpu->rob.rob_entry[free_entry].opcode, new_rob_entry->opcode); //why
  cpu->rob.rob_entry[free_entry].pc = new_rob_entry->pc;
  cpu->rob.rob_entry[free_entry].rd = new_rob_entry->rd;
  cpu->rob.rob_entry[free_entry].phy_rd = new_rob_entry->phy_rd;
  cpu->rob.rob_entry[free_entry].status = new_rob_entry->status;
  cpu->rob.rob_entry[free_entry].rs1 = new_rob_entry->rs1;
  cpu->rob.rob_entry[free_entry].phy_rs1 = new_rob_entry->phy_rs1;
  cpu->rob.rob_entry[free_entry].rs2 = new_rob_entry->rs2;
  cpu->rob.rob_entry[free_entry].phy_rs2 = new_rob_entry->phy_rs2;
  cpu->rob.rob_entry[free_entry].imm = new_rob_entry->imm;
  cpu->rob.tail++;
  if (cpu->rob.tail == ROB_ENTRIES) {
    cpu->rob.tail = 0;
  }
  return free_entry;
}

int
commit_rob_entry(APEX_CPU* cpu)
{
  if (cpu->rob.rob_entry[cpu->rob.head].status && !cpu->rob.rob_entry[cpu->rob.head].free) {
      // Do not commit instructions that do not have phyical destination address - BNZ, BZ, STORE
     // for these instructions simly remove entry from ROB
    // if (cpu->rob.rob_entry[cpu->rob.head].phy_rd != -1 ) {
      cpu->arch_reg = cpu->rob.rob_entry[cpu->rob.head].rd;
      cpu->phy_reg = cpu->rob.rob_entry[cpu->rob.head].phy_rd;
  //commit_register(cpu, rrat_index, phy_reg_to_be_commit); // commits in R-RAT and deallocates phy reg in URF
     if (cpu->rrat[cpu->arch_reg].commited_phy_reg != -1) {
 	cpu->prf[cpu->phy_reg].free = 1;
 	}
 	cpu->rrat[cpu->arch_reg].commited_phy_reg = cpu->phy_reg;

      //}
    }

    if (strcmp(cpu->rob.rob_entry[cpu->rob.head].opcode, "HALT") == 0) {
     // if (cpu->mem_cycle == 1 && strcmp(cpu->stage[MEM].opcode, "") == 0) {
        cpu->rob.rob_entry[cpu->rob.head].free = 1;    // making free ROB entry after commitment
        cpu->rob.head++;
        if (cpu->rob.head == ROB_ENTRIES) {
          cpu->rob.head = 0;
        }
      }

    else {
      cpu->rob.rob_entry[cpu->rob.head].free = 1;    // making free ROB entry after commitment
      cpu->rob.head++;
      if (cpu->rob.head == ROB_ENTRIES) {
        cpu->rob.head = 0;
      }
      cpu->commitments++;
    }

   // if (cpu->fill_in_rob > 2 && robcheck(cpu)) {
     // cpu->simulation_completed = 1;
    //}
    return 1;
  }

  void
  display_rob(APEX_CPU* cpu)
  {
    printf("-------------------------------------- ROB --------------------------------------\n");
    for (int i = 0; i < ROB_ENTRIES; i++) {
      if (!cpu->rob.rob_entry[i].free || i == cpu->rob.tail) {

        printf("| Index = %d | ", i);

        if (i == cpu->rob.tail) {
          printf("t |");
        }
        else {
          printf("  |");
        }

        if (i == cpu->rob.head) {
          printf(" h |");
        }
        else {
          printf("   |");
        }

        printf("\t");
        if (!cpu->rob.rob_entry[i].free) {
        printf("pc(%d)  ", cpu->rob.rob_entry[i].pc);
          CPU_Stage* instruction_to_print = malloc(sizeof(*instruction_to_print));
          strcpy(instruction_to_print->opcode, cpu->rob.rob_entry[i].opcode);
          instruction_to_print->rs1 = cpu->rob.rob_entry[i].rs1;
          instruction_to_print->phy_rs1 = cpu->rob.rob_entry[i].phy_rs1;
          instruction_to_print->rs2 = cpu->rob.rob_entry[i].rs2;
          instruction_to_print->phy_rs2 = cpu->rob.rob_entry[i].phy_rs2;
          instruction_to_print->rd = cpu->rob.rob_entry[i].rd;
          instruction_to_print->phy_rd = cpu->rob.rob_entry[i].phy_rd;
          instruction_to_print->imm = cpu->rob.rob_entry[i].imm;
          printf("%s,R%d,R%d,R%d  [%s,P%d,P%d,P%d]",
                cpu->rob.rob_entry[i].opcode  , cpu->rob.rob_entry[i].rd,cpu->rob.rob_entry[i].rs1 , cpu->rob.rob_entry[i].rs2,
                cpu->rob.rob_entry[i].opcode, cpu->rob.rob_entry[i].phy_rd, cpu->rob.rob_entry[i].phy_rs1, cpu->rob.rob_entry[i].phy_rs2);
          printf("\t|");
        }
        printf("\n");
      }
    }
    printf("---------------------------------------------------------------------------------\n\n");
  }



/*
 *  Checks whether there is free entry in ROB
 */
int
robchecks(APEX_CPU* cpu)
{
  if (cpu->rob.rob_entry[cpu->rob.tail].free) {
    return 1;
  }
  return 0;
}
