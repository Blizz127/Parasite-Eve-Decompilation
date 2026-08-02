/* Phase 6C — Game port shared declarations */
#ifndef GAME_PORT_H
#define GAME_PORT_H

/* Host-owned stop flags for main loop bounding */
extern int g_port_stop_requested;
extern int g_port_main_iterations;

/* Trace helper available to game code */
void Trace_Direct(const char *event);

#endif
