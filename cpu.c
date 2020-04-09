/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename)
   {
    return NULL;
   }

  APEX_CPU* cpu = malloc(sizeof(*cpu));

  if (!cpu)
   {
    return NULL;
    }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);
//setting all the values in the memory to 777
  for(int v=0;v<4000;v++)
  {
    cpu->data_memory[v]=777;
  }
  //making all register as valid at the beginning
  for (int i = 0; i <=31; i++)
  {
    /* code */
    cpu->regs_valid[i]=1;
  }

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory)
   {
    free(cpu);
    return NULL;
   }

  if (ENABLE_DEBUG_MESSAGES)
    {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2","rs3", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].rs3,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i)
  	{
    cpu->stage[i].busy = 1;
  }

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

//code for printing all the instructions
static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "LOAD") == 0)
   {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
   }
   if (strcmp(stage->opcode, "LDR") == 0)
    {
     printf(
       "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
/*  if (strcmp(stage->opcode, "") == 0)
    {
     printf("NOP");
    }

*/
  if (strcmp(stage->opcode, "STORE") == 0)
   {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
   }


   if (strcmp(stage->opcode, "STR") == 0)
    {
     printf(
       "%s,R%d,R%d,R%d ", stage->opcode, stage->rs1, stage->rs2, stage->rs3);
    }

  if (strcmp(stage->opcode, "MOVC") == 0)
   {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);

   }

   if (strcmp(stage->opcode, "ADD") == 0)
    {
     printf("%s,R%d,R%d,R%d", stage->opcode, stage->rd, stage->rs1, stage->rs2);

    }

    if (strcmp(stage->opcode, "ADDL") == 0)
     {
      printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd,stage->rs1, stage->imm);

     }

     if (strcmp(stage->opcode, "SUB") == 0)
      {
       printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);

      }

      if (strcmp(stage->opcode, "SUBL") == 0)
       {
        printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1,stage->imm);

       }

      if (strcmp(stage->opcode, "MUL") == 0)
      {
         printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1,stage->rs2);

      }

      if (strcmp(stage->opcode, "AND") == 0)
      {
         printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1,stage->rs2);

      }

      if (strcmp(stage->opcode, "OR") == 0)
      {
         printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1,stage->rs2);

      }

      if (strcmp(stage->opcode, "EX-OR") == 0)
      {
         printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1,stage->rs2);

      }


       if (strcmp(stage->opcode, "JUMP") == 0)
      {
       printf("%s,R%d,#%d ", stage->opcode, stage->rs1, stage->imm);

      }


      if (strcmp(stage->opcode, "HALT") == 0)
      {
        printf("%s ", stage->opcode);
       }

       if (strcmp(stage->opcode, "BZ") == 0)
      {
       printf("%s,#%d ", stage->opcode, stage->imm);

      }

      if (strcmp(stage->opcode, "BNZ") == 0)
     {
      printf("%s,#%d ", stage->opcode, stage->imm);

     }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
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
  if (!stage->busy && !stage->stalled)
  {
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
    stage->rs3 = current_ins->rs3;
    stage->imm = current_ins->imm;
   // stage->rd = current_ins->rd;

    /* Update PC for next instruction and  logic for stalling fetch stage*/
    if(cpu->stage[DRF].stalled==0)
    {
          cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
          cpu->stage[DRF] = cpu->stage[F];

    }

    if(cpu->display_flag==1)
    {
      if (ENABLE_DEBUG_MESSAGES)
      {
        print_stage_content("Fetch", stage);
      }
    }


  }

  else
  {
   cpu->stage[DRF] = cpu->stage[F];

      if(cpu->display_flag==1)
      {
        if(ENABLE_DEBUG_MESSAGES)
         {
            printf("Fetch    : EMPTY\n");
          }


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

   if(stage->stalled)
   {
     stage->stalled = 0;
   }


   /*
      logic for implementing HALT instruction
      once halt is fetched no instruction is allowed to fetched
      till all prevoious instructions get till WB and in the end program terminate

   */

   if(strcmp(stage->opcode, "HALT") == 0)
   {
     CPU_Stage*  stage_halt = &cpu->stage[F];
     stage_halt->stalled=1;
     cpu->pc+=0;

   }

   if (!stage->busy && !stage->stalled)
  {


     /* Read data from register file for store */
     if (strcmp(stage->opcode, "STORE") == 0)
     {
                   if( cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])
                   {


                           stage->rs1_value = cpu->regs[stage->rs1];
                           stage->rs2_value = cpu->regs[stage->rs2];

                         //  cpu->stage[F].stalled=0;
                           cpu->stage[DRF].stalled=0;



                            //make register invalid
                           // cpu->regs_valid[stage->rs1]=0;
                            //cpu->regs_valid[stage->rs2]=0;



                   }
                   else
                   {


                         //   cpu->stage[F].stalled=1;
                            cpu->stage[DRF].stalled=1;



                   }


     }//STORE



         /* Read data from register file for store */
         if (strcmp(stage->opcode, "STR") == 0)
         {
                       if( cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2] && cpu->regs_valid[stage->rs3])
                       {


                               stage->rs1_value = cpu->regs[stage->rs1];
                               stage->rs2_value = cpu->regs[stage->rs2];
                               stage->rs3_value = cpu->regs[stage->rs3];

                           //    cpu->stage[F].stalled=0;
                               cpu->stage[DRF].stalled=0;



                               //make register invalid
                              //   cpu->regs_valid[stage->rs1]=0;
                             //   cpu->regs_valid[stage->rs2]=0;
                             //   cpu->regs_valid[stage->rs3]=0;



                        }
                       else
                       {

                         //strcpy(cpu->stage[EX1].opcode,"");
                       //  cpu->stage[F].pc=0;

                             //    cpu->stage[F].stalled=1;
                                 cpu->stage[DRF].stalled=1;



                       }


         }//STR


     /* No Register file read needed for MOVC */
     if (strcmp(stage->opcode, "MOVC") == 0)
     {
             if(cpu->regs_valid[stage->rd])
             {
               //Make Desination register invalid
               cpu->regs_valid[stage->rd]=0;
               cpu->stage[DRF].stalled=0;

             }
             else
             {

             //  cpu->stage[F].stalled=1;
               cpu->stage[DRF].stalled=1;
             //  cpu->stage[DRF].busy=1;


             }


     }


     if (strcmp(stage->opcode, "ADD") == 0)
     {

             if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])
             {

               //    cpu->flag1=1;

               stage->rs1_value = cpu->regs[stage->rs1];
               stage->rs2_value = cpu->regs[stage->rs2];
           //   stage->rdd       = cpu->regs[stage->rd];

               //  cpu->stage[F].stalled=0;
                cpu->stage[DRF].stalled=0;
               //    cpu->stage[DRF].busy=0;

               //make register invalid
               //cpu->regs_valid[stage->rs1]=0;
               //cpu->regs_valid[stage->rs2]=0;
               cpu->regs_valid[stage->rd]=0;

             }
             else
             {

                 //  strcpy(cpu->stage[EX1].opcode,"");
                 //  cpu->stage[F].pc=0;
                 //  cpu->stage[EX1].pc=0;

               //  cpu->stage[F].stalled=1;
                 cpu->stage[DRF].stalled=1;
               //cpu->stage[DRF].busy=1;


             }




     }//ADD


     /* ADDL  */
     if (strcmp(stage->opcode, "ADDL") == 0)
     {
             if(cpu->regs_valid[stage->rs1])
             {

                stage->rs1_value = cpu->regs[stage->rs1];
               // stage->rdd    = cpu->regs[stage->rd];

           //     cpu->stage[F].stalled=0;
                cpu->stage[DRF].stalled=0;



                //make register invalid
                //cpu->regs_valid[stage->rs1]=0;
                cpu->regs_valid[stage->rd]=0;

             }

             else
             {

           //     cpu->stage[F].stalled=1;
                cpu->stage[DRF].stalled=1;

             }

     }//ADDL



     /* SUB */

     if (strcmp(stage->opcode, "SUB") == 0)
     {
               if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2])
               {

                 stage->rs1_value = cpu->regs[stage->rs1];
                 stage->rs2_value = cpu->regs[stage->rs2];
                 stage->rdd       = cpu->regs[stage->rd];


             //    cpu->stage[F].stalled=0;
                 cpu->stage[DRF].stalled=0;


                 //make register invalid
               //  cpu->regs_valid[stage->rs1]=0;
               //  cpu->regs_valid[stage->rs2]=0;
                 cpu->regs_valid[stage->rd]=0;


               }
               else
               {

             //    cpu->stage[F].stalled=1;
                 cpu->stage[DRF].stalled=1;


               }




     }//SUB

     /* SUBL*/

     if (strcmp(stage->opcode, "SUBL") == 0)
     {

             if(cpu->regs_valid[stage->rs1])
             {

               stage->rs1_value = cpu->regs[stage->rs1];
               stage->rdd    = cpu->regs[stage->rd];

           //    cpu->stage[F].stalled=0;
               cpu->stage[DRF].stalled=0;


               //make register invalid
               //cpu->regs_valid[stage->rs1]=0;
               cpu->regs_valid[stage->rd]=0;

             }
             else
             {
         //      cpu->stage[F].stalled=1;
               cpu->stage[DRF].stalled=1;


             }

     }//SUBL

      /*MUL */

     if (strcmp(stage->opcode, "MUL") == 0)
     {

           if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2] && cpu->regs_valid[stage->rd])
           {

             stage->rs1_value = cpu->regs[stage->rs1];
             stage->rs2_value = cpu->regs[stage->rs2];
             stage->rdd       = cpu->regs[stage->rd];

             cpu->stage[DRF].stalled=0;
           //  cpu->stage[F].stalled=0;

             //make register invalid
             //cpu->regs_valid[stage->rs1]=0;
             //cpu->regs_valid[stage->rs2]=0;
             cpu->regs_valid[stage->rd]=0;

           }

           else
           {
           //  cpu->stage[F].stalled=1;
             cpu->stage[DRF].stalled=1;


           }


      }//MUL

      /*AND */
      if (strcmp(stage->opcode, "AND") == 0)
      {
               if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2] && cpu->regs_valid[stage->rd])
               {

                 stage->rs1_value = cpu->regs[stage->rs1];
                 stage->rs2_value = cpu->regs[stage->rs2];
                 stage->rdd       = cpu->regs[stage->rd];

                 cpu->stage[DRF].stalled=0;
             //    cpu->stage[F].stalled=0;

                 //make register invalid
              //   cpu->regs_valid[stage->rs1]=0;
              //   cpu->regs_valid[stage->rs2]=0;
                 cpu->regs_valid[stage->rd]=0;

               }

               else
               {
                 cpu->stage[DRF].stalled=1;
             //    cpu->stage[F].stalled=1;
               }


      }//AND

      /*OR */
       if (strcmp(stage->opcode, "OR") == 0)
       {

                   if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2] && cpu->regs_valid[stage->rd])
                   {
                     stage->rs1_value = cpu->regs[stage->rs1];
                     stage->rs2_value = cpu->regs[stage->rs2];
                     stage->rdd       = cpu->regs[stage->rd];

                     cpu->stage[DRF].stalled=0;
                   //  cpu->stage[F].stalled=0;

                     //make register invalid
                     //cpu->regs_valid[stage->rs1]=0;
                     //cpu->regs_valid[stage->rs2]=0;
                     cpu->regs_valid[stage->rd]=0;

                   }
                   else
                   {

                     cpu->stage[DRF].stalled=1;
                   //  cpu->stage[F].stalled=1;

                   }


       }//OR


       /*EX-OR */
        if (strcmp(stage->opcode, "EX-OR") == 0)
        {

                if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2] && cpu->regs_valid[stage->rd])
                {

                  stage->rs1_value = cpu->regs[stage->rs1];
                  stage->rs2_value = cpu->regs[stage->rs2];
                  stage->rdd       = cpu->regs[stage->rd];

                  cpu->stage[DRF].stalled=0;
             //     cpu->stage[F].stalled=0;

                  //make register invalid
                  //cpu->regs_valid[stage->rs1]=0;
                  //cpu->regs_valid[stage->rs2]=0;
                  cpu->regs_valid[stage->rd]=0;

                }
                else
                {
                  cpu->stage[DRF].stalled=1;
           //       cpu->stage[F].stalled=1;

                }


        }//EX-OR


     /*LOAD*/
     if (strcmp(stage->opcode, "LOAD") == 0)
     {
             if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rd])
             {
                 stage->rs1_value = cpu->regs[stage->rs1];
             //    stage->rd= cpu->regs[stage->rd];


                 cpu->stage[DRF].stalled=0;
           //      cpu->stage[F].stalled=0;

                 //make register invalid
                 //cpu->regs_valid[stage->rs1]=0;
                 cpu->regs_valid[stage->rd]=0;


             }
             else
             {
               cpu->stage[DRF].stalled=1;
           //    cpu->stage[F].stalled=1;

             }


     }//LOAD

     /*LDR*/
     if (strcmp(stage->opcode, "LDR") == 0)
     {
             if(cpu->regs_valid[stage->rs1] && cpu->regs_valid[stage->rs2] && cpu->regs_valid[stage->rd])
             {
                 stage->rs1_value = cpu->regs[stage->rs1];
                 stage->rs2_value = cpu->regs[stage->rs2];

                 cpu->stage[DRF].stalled=0;
           //      cpu->stage[F].stalled=0;

                 //make register invalid
               //  cpu->regs_valid[stage->rs1]=0;
               //  cpu->regs_valid[stage->rs2]=0;
                 cpu->regs_valid[stage->rd]=0;


             }
             else
             {
               cpu->stage[DRF].stalled=1;
         //      cpu->stage[F].stalled=1;

             }


     }//LDR


     /*JUMP*/
     if (strcmp(stage->opcode, "JUMP") == 0)
     {
       if(cpu->regs_valid[stage->rs1])
       {
         stage->rs1_value = cpu->regs[stage->rs1];

         cpu->stage[DRF].stalled=0;



       }
       else
       {
         cpu->stage[DRF].stalled=1;

       }



     }//JUMP



     /* Copy data from decode latch to execute latch*/
     cpu->stage[EX1] = cpu->stage[DRF];

      if(cpu->display_flag==1)
     {
       if (ENABLE_DEBUG_MESSAGES)
       {
         print_stage_content("Decode/RF", stage);


       }

     }


   }
 /*  else
   {

         if (ENABLE_DEBUG_MESSAGES)
         {
           print_stage_content("Decode/RF", stage);


         }

   }
   */

   else
   {
    cpu->stage[EX1] = cpu->stage[DRF];

      if(cpu->display_flag==1)
      {
        if(ENABLE_DEBUG_MESSAGES)
         {
            printf("Decode/RF     : EMPTY\n");
          }

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
execute(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX1];
  if (!stage->busy && !stage->stalled)
{

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {


        stage->mem_address =  stage->rs2_value + stage->imm ;


    }

    /* STR */
    if (strcmp(stage->opcode, "STR") == 0)
    {


        stage->mem_address =  stage->rs2_value + stage->rs3_value ;


    }


    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {    //printf("%d\n",stage->rd );
         stage->buffer = 0 + stage->imm;
      //   printf("MOVE from execute...%d",stage->buffer);
        // printf("\n Value from MOVC for rd= %d\n", stage->buffer);


    }

    if (strcmp(stage->opcode, "ADD") == 0)
    {



        stage->rdd =  stage->rs1_value + stage->rs2_value;

    //  printf("\n value in the Destination register after ADD : %d", stage->rdd);

    }

    if (strcmp(stage->opcode, "ADDL") == 0)
    {

        stage->rdd =  stage->rs1_value + stage->imm;

    //  printf("\n value in the Destination register after ADD : %d", stage->rdd);

    }

    if (strcmp(stage->opcode, "SUB") == 0)
    {

        stage->rdd =  stage->rs1_value - stage->rs2_value;

      // printf("\n value in the Destination register after ADD : %d", stage->buffer);

    }



    if (strcmp(stage->opcode, "SUBL") == 0)
    {

      stage->rdd =  stage->rs1_value - stage->imm;

    }

    /*MUL*/

    if (strcmp(stage->opcode, "MUL") == 0)
    {

        stage->rdd =  stage->rs1_value * stage->rs2_value;

    }

    /* AND*/
    if (strcmp(stage->opcode, "AND") == 0)
    {

        stage->rdd =  stage->rs1_value & stage->rs2_value;

    }

    /* OR*/
    if (strcmp(stage->opcode, "OR") == 0)
    {

        stage->rdd =  stage->rs1_value | stage->rs2_value;

    }

    /* EX-OR*/
    if (strcmp(stage->opcode, "EX-OR") == 0)
    {

        stage->rdd =  stage->rs1_value ^ stage->rs2_value;

    }


    if (strcmp(stage->opcode, "LOAD") == 0)
    {

        stage->mem_address =  stage->rs1_value + stage->imm;



    }

    if (strcmp(stage->opcode, "LDR") == 0)
    {

        stage->mem_address =  stage->rs1_value + stage->rs2_value;



    }

    if (strcmp(stage->opcode, "JUMP") == 0)
    {

      stage->rdd= stage->rs1_value + stage->imm;
      cpu->pc = stage->rdd;


    }


    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[EX2] = cpu->stage[EX1];

    if(cpu->display_flag==1)
    {
      if (ENABLE_DEBUG_MESSAGES)
      {
        print_stage_content("Execute_One", stage);
      }

    }


  }

  else
  {
   cpu->stage[EX2] = cpu->stage[EX1];

       if(cpu->display_flag==1)
       {
         if(ENABLE_DEBUG_MESSAGES)
          {
             printf("Execute        : EMPTY\n");
           }


       }

  }
  return 0;


}




int
execute_second(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX2];
  if (!stage->busy && !stage->stalled)
{

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {



    }

    /* STR */
    if (strcmp(stage->opcode, "STR") == 0)
    {



    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
      //  stage->buffer = 0 + stage->imm;
        cpu->regs[stage->rd] = stage->buffer;
      //  printf("MOVE in execute two...%d",cpu->regs[stage->rd]  );
        cpu->regs_valid[stage->rd]=1;

    }


    /* ADD */
    if (strcmp(stage->opcode, "ADD") == 0)
    {

      stage->buffer= stage->rdd;
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=1;


    }


    /* ADDL */
    if (strcmp(stage->opcode, "ADDL") == 0)
    {

        stage->buffer= stage->rdd;
        cpu->regs[stage->rd] = stage->buffer;

        cpu->regs_valid[stage->rd]=1;

    }


    /* SUB */
    if (strcmp(stage->opcode, "SUB") == 0)
    {

       stage->buffer= stage->rdd;
       cpu->regs[stage->rd] = stage->buffer;

      cpu->regs_valid[stage->rd]=1;



    }


    /* SUBL */
    if (strcmp(stage->opcode, "SUBL") == 0)
    {

        stage->buffer= stage->rdd;
        cpu->regs[stage->rd] = stage->buffer;

        cpu->regs_valid[stage->rd]=1;

    }


    /* MUL */
    if (strcmp(stage->opcode, "MUL") == 0)
    {

      stage->buffer=stage->rdd;
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=1;


    }


    /* AND */
    if (strcmp(stage->opcode, "AND") == 0)
    {
      stage->buffer= stage->rdd;
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=1;


    }


    /* OR */
    if (strcmp(stage->opcode, "OR") == 0)
    {
      stage->buffer= stage->rdd;
      cpu->regs[stage->rd] = stage->buffer;

      cpu->regs_valid[stage->rd]=1;

    }

    /* EX-OR */
    if (strcmp(stage->opcode, "EX-OR") == 0)
    {
      stage->buffer= stage->rdd;
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=1;

    }


    /* LOAD */
    if (strcmp(stage->opcode, "LOAD") == 0)
    {

    }


    /* LDR */
      if (strcmp(stage->opcode, "LDR") == 0)
      {

      }


    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM1] = cpu->stage[EX2];

    if(cpu->display_flag==1)
    {
      if (ENABLE_DEBUG_MESSAGES)
      {
        print_stage_content("Execute_second", stage);
      }

    }


  }

  else
  {
   cpu->stage[MEM1] = cpu->stage[EX2];
   if(cpu->display_flag==1)
   {
     if(ENABLE_DEBUG_MESSAGES)
      {
         printf("Execute_second : EMPTY\n");
       }

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
memory(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM1];
  if (!stage->busy && !stage->stalled)
 {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {

      cpu->data_memory[stage->mem_address]=stage->rs1_value;


    }

    if (strcmp(stage->opcode, "STR") == 0)
    {

      cpu->data_memory[stage->mem_address]=stage->rs1_value;


    }

    if (strcmp(stage->opcode, "LDR") == 0)
    {
      cpu->regs[stage->rd]=cpu->data_memory[stage->mem_address];


    }


    if (strcmp(stage->opcode, "LOAD") == 0)
    {
      cpu->regs[stage->rd]=cpu->data_memory[stage->mem_address];

    }


    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {


    }

    if (strcmp(stage->opcode, "ADD") == 0)
    {


    }

    if (strcmp(stage->opcode, "ADDL") == 0)
    {


    }


    if (strcmp(stage->opcode, "SUB") == 0)
    {


    }

    if (strcmp(stage->opcode, "SUBL") == 0)
    {


    }


    if (strcmp(stage->opcode, "MUL") == 0)
    {


    }

    /*AND */
    if (strcmp(stage->opcode, "AND") == 0)
    {


    }

    /*OR */
    if (strcmp(stage->opcode, "OR") == 0)
    {


    }

    /*EX-OR */
    if (strcmp(stage->opcode, "EX-OR") == 0)
    {


    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[MEM2] = cpu->stage[MEM1];

    if(cpu->display_flag==1)
    {
      if (ENABLE_DEBUG_MESSAGES)
      {
        print_stage_content("Memory_One", stage);
      }

    }


  }

  else
  {
   cpu->stage[MEM2] = cpu->stage[MEM1];
   if(cpu->display_flag==1)
   {
     if(ENABLE_DEBUG_MESSAGES)
      {
         printf("Memory_One     : EMPTY\n");
       }
   }

  }


  return 0;
}





int
memory_second(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM2];
  if (!stage->busy && !stage->stalled)
 {


       /* MOVC */
       if (strcmp(stage->opcode, "MOVC") == 0)
       {


       }

       /* ADD */

       if (strcmp(stage->opcode, "ADD") == 0)
       {


       }


       /* ADDL */
       if (strcmp(stage->opcode, "ADDL") == 0)
       {


       }


       /* SUb */

       if (strcmp(stage->opcode, "SUB") == 0)
       {


       }

       /* SUBL */
       if (strcmp(stage->opcode, "SUBL") == 0)
       {


       }

       /*MUL */

       if (strcmp(stage->opcode, "MUL") == 0)
       {

       }


       /* AND */

       if (strcmp(stage->opcode, "AND") == 0)
       {


       }

       /* OR */

       if (strcmp(stage->opcode, "OR") == 0)
       {


       }

       /* EX-OR */

       if (strcmp(stage->opcode, "EX-OR") == 0)
       {


       }


       if (strcmp(stage->opcode, "LDR") == 0)
       {
               //Make Register Valid
               //cpu->regs_valid[stage->rs1]=1;
               //cpu->regs_valid[stage->rs2]=1;

               cpu->regs_valid[stage->rd]=1;



       }


       if (strcmp(stage->opcode, "LOAD") == 0)
       {
               //Make Register Valid
               //cpu->regs_valid[stage->rs1]=1;
               cpu->regs_valid[stage->rd]=1;


       }
       /* Store */
       if (strcmp(stage->opcode, "STORE") == 0)
       {

          //make register valid
          //cpu->regs_valid[stage->rs1]=1;
          //cpu->regs_valid[stage->rs2]=1;


       }

       if (strcmp(stage->opcode, "STR") == 0)
       {

         //make register valid
        // cpu->regs_valid[stage->rs1]=1;
        // cpu->regs_valid[stage->rs2]=1;
        // cpu->regs_valid[stage->rs3]=1;



       }


    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM2];

    if(cpu->display_flag==1)
    {
      if (ENABLE_DEBUG_MESSAGES)
      {
        print_stage_content("Memory_second", stage);
      }

    }


  }
  else
  {
       cpu->stage[WB] = cpu->stage[MEM2];

       if(cpu->display_flag==1)
       {
         if(ENABLE_DEBUG_MESSAGES)
          {
             printf("Memory_second  : EMPTY\n");

           }
       }

  }

  return 0;
}


/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
 int
 writeback(APEX_CPU* cpu)
 {
   CPU_Stage* stage = &cpu->stage[WB];
   char status[10];
   if(strcmp(stage->opcode, "HALT") == 0)
   {

     printf("\n");
     printf("\n--------------------------------------------------------------------------------------\n");
     printf("\n====================================REGISTER VALUE====================================\n");
     printf("\n--------------------------------------------------------------------------------------\n");

     for(int i=0;i<32;i++)
     {
           printf("\n");
           if(cpu->regs_valid[i]==1)
            {
              strcpy(status, "Valid");
            }
            else
            {
              strcpy(status, "Invalid");
            }
          printf(" \t | Register[%2d] |\t Value=%4d |\t status=%s | \n",i,cpu->regs[i], status );
     }


     printf("\n--------------------------------------------------------------------------------------\n");

     printf("\n=========================DATA MEMORY===============================\n");
     printf("\n--------------------------------------------------------------------------------------\n");
     for(int i=0;i<=99;i++)
     {
     printf(" \t|\t MEM[%d] \t|\t Value=%d \t| \n",i,cpu->data_memory[i]);
     }
     printf("\n--------------------------------------------------------------------------------------\n");

     APEX_cpu_stop(cpu);


   }

   if (!stage->busy && !stage->stalled)

   {

     /* Update register file */
     if (strcmp(stage->opcode, "MOVC") == 0)
     {
       cpu->regs[stage->rd] = stage->buffer;
       cpu->regs_valid[stage->rd]=1;

     }

     if (strcmp(stage->opcode, "ADD") == 0)
     {
       cpu->regs[stage->rd] = stage->buffer;
       cpu->regs_valid[stage->rd]=1;


     }


     if (strcmp(stage->opcode, "ADDL") == 0)
     {
       cpu->regs[stage->rd] = stage->buffer;

       cpu->regs_valid[stage->rd]=1;
     }



     if (strcmp(stage->opcode, "SUB") == 0)
     {
       cpu->regs[stage->rd] = stage->buffer;
       cpu->regs_valid[stage->rd]=1;

     }

     if (strcmp(stage->opcode, "SUBL") == 0)
     {
       cpu->regs[stage->rd] = stage->buffer;
       cpu->regs_valid[stage->rd]=1;

     }

     if (strcmp(stage->opcode, "MUL") == 0)
     {
       cpu->regs[stage->rd] = stage->buffer;
       cpu->regs_valid[stage->rd]=1;

     }


     if (strcmp(stage->opcode, "AND") == 0)
     {
       cpu->regs[stage->rd] = stage->buffer;
       cpu->regs_valid[stage->rd]=1;

     }

     if (strcmp(stage->opcode, "OR") == 0)
     {
       cpu->regs[stage->rd] = stage->buffer;
       cpu->regs_valid[stage->rd]=1;

     }

     if (strcmp(stage->opcode, "EX-OR") == 0)
     {
       cpu->regs[stage->rd] = stage->buffer;
       cpu->regs_valid[stage->rd]=1;

     }

     cpu->ins_completed++;

    if(cpu->display_flag==1)
     {  if (ENABLE_DEBUG_MESSAGES)
       {
         print_stage_content("Writeback", stage);
       }

     }


   }

   else
   {
    cpu->stage[WB] = cpu->stage[MEM2];

    if(cpu->display_flag==1)
    {
      if(ENABLE_DEBUG_MESSAGES)
       {
          printf("Writeback      : EMPTY\n");
        }

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
  char status[10];
  while (1)
 {

    /* All the instructions committed, so exit */
    if (cpu->ins_completed == cpu->code_memory_size)
    {
      printf("---------------------(apex) >> --------------------Simulation Complete------------------------------------------");

     /*printf("    Register :  Value");
     for(int i=0;i<31;i++)
     {

       printf("\n  %2d   :   %2d",i,cpu->regs[i]);
     } */

      break;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
      printf("-----------------------------------------------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("-----------------------------------------------------------------------\n");
    }

//for(int i = 0; i<cpu->clock_cycles;i++){
  writeback(cpu);
  memory_second(cpu);
  memory(cpu);
  execute_second(cpu);
  execute(cpu);
  decode(cpu);
  fetch(cpu);
  cpu->clock++;
//}

}


 printf("\n");
 printf("\n--------------------------------------------------------------------------------------\n");
 printf("\n====================================REGISTER VALUE====================================\n");
 printf("\n--------------------------------------------------------------------------------------\n");

 for(int i=0;i<32;i++)
 {
       printf("\n");
       if(cpu->regs_valid[i]==1)
        {
          strcpy(status, "Valid");
        }
        else
        {
          strcpy(status, "Invalid");
        }
      printf(" \t | Register[%2d] |\t Value=%4d |\t status=%s | \n",i,cpu->regs[i], status );
 }


printf("\n--------------------------------------------------------------------------------------\n");

printf("\n==========================DATA MEMORY====================================\n");
printf("\n--------------------------------------------------------------------------------------\n");
 for(int i=0;i<=99;i++)
 {
 printf(" \t|\t MEM[%d] \t|\t Value=%d \t| \n",i,cpu->data_memory[i]);
 }
 printf("\n--------------------------------------------------------------------------------------\n");
 return 0;
 }
