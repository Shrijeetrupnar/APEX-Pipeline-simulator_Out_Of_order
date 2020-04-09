---------------------------------------------------------------------------------
APEX Pipeline Simulator with 7 stages
---------------------------------------------------------------------------------
A simple implementation of 7 Stage APEX Pipeline

Author :
---------------------------------------------------------------------------------
Shrijeet Rupnar (srupnar1@binghamton.edu)
B-N0 : B00808280
State University of New York, Binghamton

Notes:
----------------------------------------------------------------------------------
1) This code is a simple implementation of 7 Stage APEX Pipeline.

	 Fetch -> Decode -> Execute ->Execute_One-> Memory -> Memory_One-> Writeback



2) All the stages have latency of one cycle. There is a single functional unit in
	 EX stage which perform all the arithmetic and logic operations.

3) Logic to check data dependencies has been included in Decode/RF stage.

File-Info
----------------------------------------------------------------------------------
1) Makefile 			- You can edit as needed
2) file_parser.c 	- Contains Functions to parse input file. No need to change this file
3) cpu.c          - Contains Implementation of APEX cpu.
4) cpu.h          - Contains various data structures declarations needed by 'cpu.c'.


How to compile and run
----------------------------------------------------------------------------------
1) go to terminal, cd into project directory and type 'make' to compile project
2) Run using ./apex_sim <input file name> <simulate> no_of_clock_cycles

Ex: ./apex_sim input.asm simulate 10

3) Run using ./apex_sim <input file name> <display> no_of_clock_cycles

Ex: ./apex_sim input.asm display 10

----------------------------------------------------------------------------------

NOTE:
I have implemented ADD, ADDL, SUB, SUBL, LOAD, LDR, STORE, STR,AND, OR, EX-OR,JUMP,HALT
Stalling is working
JUMP is working
HALT is working
display and simulate is working

I have tried but could not implement  below instructions successfully
1. BZ 2. BNZ

----------------------------------------------------------------------------------
