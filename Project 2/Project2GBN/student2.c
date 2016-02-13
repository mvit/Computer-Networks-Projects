#include <stdio.h>
#include <stdlib.h>
#include "project2.h"
 
/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for Project 2, unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/



/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

int get_checksum(struct pkt packet) {
	int checksum = packet.seqnum & 0xFF;
	int i;

	for (i = 0; i<= MESSAGE_LENGTH; i++) {
			checksum = (checksum >> 1) + ((checksum & 0x1) << 8);
			if (i==0) checksum = (checksum + packet.acknum) &0xFF;
			else checksum = (checksum + packet.payload[i-1]) & 0xFF;
	}
	//printf("checksum generated, value of %i\n", checksum);
	
	return checksum;
}
