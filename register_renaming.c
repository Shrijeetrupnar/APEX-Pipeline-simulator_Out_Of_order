

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

int
is_phys_reg_free(APEX_CPU* cpu)
{
  for (int i = 0; i < PRF_ENTRIES; i++) {
    if (cpu->prf[i].free) {
      return 1;
    }
  }
  return 0; // there is NO free physical register
}

int
get_phys_reg(APEX_CPU* cpu)
{
  for (int i = 0; i < PRF_ENTRIES; i++) {
    if (cpu->prf[i].free) {
      return i;
    }
  }
return -1;
}

// Before calling this function, make sure you first call
// is_phys_reg_free function explicitly
int
allocate_phys_reg(APEX_CPU* cpu, int arch_reg)
{
  int free_phys_reg = get_phys_reg(cpu);
  cpu->prf[free_phys_reg].free = 0;   // this phys reg is not free now
  cpu->prf[free_phys_reg].valid = 0;  // this phys reg is not valid now
  cpu->rat[arch_reg].phys_reg = free_phys_reg;
  return free_phys_reg;
}

void
deallocate_phys_reg(APEX_CPU* cpu, int phys_reg)
{
  //int phys_reg = cpu->rrat[arch_reg].commited_phys_reg;
  if (phys_reg != -1) {
    //printf("Releasing phys_reg: %d, arch_reg: %d\n", phys_reg, arch_reg);
    cpu->prf[phys_reg].free = 1;
  }
}

void
commit_register(APEX_CPU* cpu, int arch_reg, int phys_reg)
{
  //printf("In commit_register #1: %d\n", cpu->clock);
  //int phys_reg_to_deallocate = cpu->rrat[arch_reg].commited_phys_reg;
  //deallocate_phys_reg(cpu, phys_reg_to_deallocate);
  //printf("Commiting phys_reg: %d\n", phys_reg);
  //cpu->rrat[arch_reg].commited_phys_reg = phys_reg;
}



void
write_prf(APEX_CPU* cpu, enum Stages FU_type)
{
  int phy_reg = cpu->stage[FU_type].phy_rd;
  int result = cpu->stage[FU_type].buffer;
  cpu->prf[phy_reg].value = result;
  cpu->prf[phy_reg].valid = 1;
}





void
display_rat(APEX_CPU* cpu)
{
  int rat_empty = 1;
  printf("-------------------------------------- RAT --------------------------------------\n");
  for (int i = 0; i < RAT_ENTRIES; i++) {
    if (cpu->rat[i].phys_reg != -1) {
      rat_empty = 0;
      printf("| RAT[%d] = U%d |",
              i, cpu->rat[i].phys_reg);
    }
  }
  if (rat_empty) {
    printf("Empty");
  }
  printf("\n");
  printf("---------------------------------------------------------------------------------\n\n");
}


void
display_registers(APEX_CPU* cpu)
{
  //display_rrat(cpu);
  display_rat(cpu);
  //print_prf_for_debug(cpu);
  //print_datamemory_for_debug(cpu);
}

void
display_prf(APEX_CPU* cpu)
{
  printf("\n======================== STATE OF UNIFIED REGISTER FILE ========================\n");
  for (int i = 0; i < PRF_ENTRIES; i++) {
    if (!cpu->prf[i].free) {
      printf("         |\tPRF[%d]\t|\tValue = %d\t|\tStatus = %d\t|\n",
            i, cpu->prf[i].value, cpu->prf[i].valid);
    }
  }
  printf("================================================================================\n");
}

void
display_data_mem(APEX_CPU* cpu)
{
  printf("\n============================= STATE OF DATA MEMORY =============================\n");
  for (int i = 0; i < 100; i++) {
    printf("                     |\tMEM[%d]\t|\tData Value = %d\t|\n",
            i, cpu->data_memory[i]);
  }
  printf("================================================================================\n\n");
}

void
display_regs_mem(APEX_CPU* cpu)
{
  display_prf(cpu);
  display_data_mem(cpu);
}
