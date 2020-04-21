// Pushing ROB Entry

  ROB_Entry* new_rob_entry = malloc(sizeof(*new_rob_entry));
  new_rob_entry->free = 0;
  strcpy(new_rob_entry->opcode, stage->opcode);
  new_rob_entry->pc = stage->pc;
  new_rob_entry->arch_rd = stage->arch_rd;
  new_rob_entry->phys_rd = stage->phys_rd;
  new_rob_entry->arch_rs1 = stage->arch_rs1;
  new_rob_entry->phys_rs1 = stage->phys_rs1;
  new_rob_entry->arch_rs2 = stage->arch_rs2;
  new_rob_entry->phys_rs2 = stage->phys_rs2;
  new_rob_entry->imm = stage->imm;
  if (strcmp(stage->opcode, "HALT") == 0) { new_rob_entry->status = 1; }
  else { new_rob_entry->status = 0; }
  stage->rob_entry_id = push_rob_entry(cpu, new_rob_entry); \\simply gets 0 from the rob func in rob.c





is_rob_entry_free ------- robchecks
