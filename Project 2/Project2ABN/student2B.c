#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
extern int TraceLevel;
int ack_expected = 0;
/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {
	
}

/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
	struct msg message;
	struct pkt ack;

	if (TraceLevel == 2) printf("------------B received %s, %i, %i, %i\n", packet.payload, packet.acknum, packet.seqnum, packet.checksum);
	if (TraceLevel == 2) printf("Expected ACK is %i\n", ack_expected);
	if (TraceLevel == 2) printf("B Generating Checksum\n");
	int check = get_checksum(packet);
	memset(ack.payload, '\0', sizeof(20));
	
	if ((ack_expected == packet.seqnum) && (check == packet.checksum)) {
		if (TraceLevel == 2) printf("Got Correct Packet in B\n");

		memset(message.data, '\0', sizeof(20));
		strncpy(message.data, packet.payload, 20);
		tolayer5(BEntity, message);
		
		ack.acknum = packet.seqnum;
		ack.seqnum = packet.seqnum;

		strncpy(ack.payload, packet.payload, 20);
		ack.checksum = get_checksum(packet);
		
		ack_expected = 1 - ack_expected;
	}
	
	else {
		if (TraceLevel == 2) printf("Got incorrect Packet in B\n");
		if (TraceLevel == 2) printf("Generated ACK%i from B\n", 1 - ack_expected);	
		ack.acknum = 1 - ack_expected;
		ack.seqnum = packet.seqnum;
		strncpy(ack.payload, packet.payload, 20);
		
		ack.checksum = get_checksum(packet);
	}
	
	if (TraceLevel == 2) printf("Sending ACK%i from B\n", ack.acknum);	
	tolayer3(BEntity, ack);
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
}

