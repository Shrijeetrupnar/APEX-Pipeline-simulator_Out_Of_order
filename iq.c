#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

int
is_iq_entry_free(APEX_CPU* cpu)
{
  for (int i = 0; i < IQ_ENTRIES; i++) {
    if (cpu->iq.iq_entry[i].free) {
      cpu->iq.free_entry = i;
      return 1;
    }
  }
  return 0;
}

int
push_in_iq(APEX_CPU* cpu, IQ_entry* new_iq_entry)
{
  int free_entry = cpu->iq.free_entry;
  cpu->iq.iq_entry[free_entry].pc = new_iq_entry->pc;
  strcpy(cpu->iq.iq_entry[free_entry].opcode, new_iq_entry->opcode);
  cpu->iq.iq_entry[free_entry].counter  = new_iq_entry->counter;
  cpu->iq.iq_entry[free_entry].free  = new_iq_entry->free;
  cpu->iq.iq_entry[free_entry].FU_type  = new_iq_entry->FU_type;
  cpu->iq.iq_entry[free_entry].imm  = new_iq_entry->imm;
  cpu->iq.iq_entry[free_entry].rs1_ready  = new_iq_entry->rs1_ready;
  cpu->iq.iq_entry[free_entry].phy_rs1  = new_iq_entry->phy_rs1;
  cpu->iq.iq_entry[free_entry].rs1  = new_iq_entry->rs1;
  cpu->source1_flag = new_iq_entry->rs1;
  cpu->iq.iq_entry[free_entry].rs1_value  = new_iq_entry->rs1_value;
  cpu->iq.iq_entry[free_entry].rs2_ready  = new_iq_entry->rs2_ready;
  cpu->iq.iq_entry[free_entry].phy_rs2  = new_iq_entry->phy_rs2;
  cpu->iq.iq_entry[free_entry].rs2  = new_iq_entry->rs2;
  cpu->source2_flag = new_iq_entry->rs2;
  cpu->iq.iq_entry[free_entry].rs2_value  = new_iq_entry->rs2_value;
  cpu->iq.iq_entry[free_entry].phy_rd  = new_iq_entry->phy_rd;
  cpu->iq.iq_entry[free_entry].rd  = new_iq_entry->rd;
  cpu->iq.iq_entry[free_entry].LSQ_index  = new_iq_entry->LSQ_index;
  cpu->iq.iq_entry[free_entry].ROB_entry_id  = new_iq_entry->ROB_entry_id;
  //cpu->iq.iq_entry[free_entry].branch_id  = new_iq_entry->branch_id;
  printf("IQ entry%d\n",cpu->iq.iq_entry[free_entry].counter );
  return 0;
}





int
update_counters(APEX_CPU* cpu)
{
  for (int i = 0; i < IQ_ENTRIES; i++) {
    if (!cpu->iq.iq_entry[i].free) {
      cpu->iq.iq_entry[i].counter++;
    }
  }
  return 0;
}




void
display_iq(APEX_CPU* cpu)
{
  int iq_empty = 1;
  printf("\n--------------------------------- Issue Queue -----------------------------------\n");
  for (int i = 0; i < IQ_ENTRIES; i++) {
    if (!cpu->iq.iq_entry[i].free) {
      iq_empty = 0;
      printf("| Counter = %d |\tpc(%d)  ", cpu->iq.iq_entry[i].counter, cpu->iq.iq_entry[i].pc);
      CPU_Stage* instruction_to_print = malloc(sizeof(*instruction_to_print));
      strcpy(instruction_to_print->opcode, cpu->iq.iq_entry[i].opcode);
      instruction_to_print->rs1 = cpu->iq.iq_entry[i].rs1;
      instruction_to_print->phy_rs1 = cpu->iq.iq_entry[i].phy_rs1;
      instruction_to_print->rs2 = cpu->iq.iq_entry[i].rs2;
      instruction_to_print->phy_rs2 = cpu->iq.iq_entry[i].phy_rs2;
      instruction_to_print->rd = cpu->iq.iq_entry[i].rd;
      instruction_to_print->phy_rd = cpu->iq.iq_entry[i].phy_rd;
      instruction_to_print->imm = cpu->iq.iq_entry[i].imm;
      printf("%s,R%d,R%d,R%d  [%s,P%d,P%d,P%d]",
            cpu->iq.iq_entry[i].opcode  , cpu->iq.iq_entry[i].rd,cpu->iq.iq_entry[i].rs1 , cpu->iq.iq_entry[i].rs2,
              cpu->iq.iq_entry[i].opcode, cpu->iq.iq_entry[i].phy_rd, cpu->iq.iq_entry[i].phy_rs1, cpu->iq.iq_entry[i].phy_rs2);
      printf("\t|\n");
    }
  }
  if (iq_empty) {
    printf("Empty\n");
  }
  printf("---------------------------------------------------------------------------------\n\n");
}

int
process_iq(APEX_CPU* cpu)
{
  //print_iq_for_debug(cpu);
//  get_inst_FU(cpu, INT_FU1);
  //get_inst_FU(cpu, MUL_FU1);
  update_counters(cpu);
  return 0;
}
