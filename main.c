/*
 *  main.c
 *
 *  Author :
 *  Shrijeet Rupnar (srupnar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

int
main(int argc, char const* argv[])
{

 // fprintf("Welcome to APEX & stage Pipeline Programme.....!");
 /*printf("%s\n",argv[0] );
  printf("%s\n",argv[1] );
   printf("%s\n",argv[2] );
    printf("%s\n",argv[3] );*/

  if (argc != 4) {
    fprintf(stderr, "APEX_Help : Usage %s <input_file>\n", argv[0]);
    exit(1);
  }


  if(strcmp(argv[2],"display")==0)
  {
    APEX_CPU* cpu = APEX_cpu_init(argv[1]);
    if (!cpu) {
      fprintf(stderr, "APEX_Error : Unable to initialize CPU\n");
      exit(1);

    }
    cpu->clock_cycles = atoi(argv[3]);
    //printf("%d\n",cpu->d );

    display(cpu);



  }
  else if(strcmp(argv[2],"simulate")==0)
  {
    APEX_CPU* cpu = APEX_cpu_init(argv[1]);
    if (!cpu)
    {
      fprintf(stderr, "APEX_Error : Unable to initialize CPU\n");
      exit(1);
    }

    cpu->clock_cycles= atoi(argv[3]);

    simulate(cpu);

  }
  else
{
  fprintf(stderr, "APEX_Help : Usage %s <input_file>\n", argv[0]);
  exit(1);
}


  /*printf("%s\n",argv[2] );
  printf("%s\n",argv[3] );

  if(argv[2]=="simulate")
  {
    cpu->clock_cycles = argv[3];
    printf("%d\n", cpu->clock_cycles);
  } */

//  APEX_cpu_run(cpu);
  //APEX_cpu_stop(cpu);
  return 0;
}

void
display(APEX_CPU* cpu)
{
  cpu->display_flag=1;
  //char status[10];

//calling the run method
    APEX_cpu_run(cpu);

    //stop the cpu
    APEX_cpu_stop(cpu);

}



void
simulate(APEX_CPU* cpu)
{
  //char status[10];


  //calling the run method
  APEX_cpu_run(cpu);

  //stop the cpu
  APEX_cpu_stop(cpu);



}
