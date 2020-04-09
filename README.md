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
1) This code is a simple implementation of 7 Stage APEX Pipeline out of order.

	 Fetch -> Decode -> Execute ->Execute_One-> Memory -> Memory_One-> Writeback



2) All the stages have latency of one cycle. There is a single functional unit in
	 EX stage which perform all the arithmetic and logic operations.

3) Logic to check data dependencies has been included in Decode/RF stage."# APEX-Pipeline-Out_of_order_simulator" 
