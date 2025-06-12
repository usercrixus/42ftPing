#pragma once

/**
 * handle_sigint — SIGINT (Ctrl-C) handler for ping()
 *
 *   printf("\n--- ping statistics ---\n");
 *     — Prints a header labeling the summary block.
 *
 *   first print:
 *     — transmitted: total ICMP ECHO_REQUESTs sent.
 *     — received: total ICMP ECHO_REPLYs received.
 *     — packet loss: percentage of lost packets: (transmitted − received) / transmitted × 100.
 *     — elapsed: total run time of the ping session in milliseconds.
 *
 *   second print:
 *     — avg: average RTT over all replies.
 *     — mdev: standard deviation of RTTs.
 *     — rtt_min, rtt_max: minimum and maximum observed RTT.
 * 
 *     RTT stands for Round-Trip Time—the elapsed time between when you send an ICMP “echo request”
 *     packet and when you receive the corresponding “echo reply.”
 */
void handle_sigint(int signo);