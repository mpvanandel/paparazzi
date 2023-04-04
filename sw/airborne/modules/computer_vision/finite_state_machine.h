#ifndef FINITE_STATE_MACHINE_H
#define FINITE_STATE_MACHINE_H

void reset_turn_counters(void);
void state_just_turned(void);
void state_out_of_bounds(void);
void state_obs_middle(void);
void state_obs_left(void);
void state_obs_right(void);
void state_safe(void);
void run_fsm(float flow_left, float flow_middle, float flow_right);

int counter_turn_1 = 0;
int counter_turn_2 = 0;

#endif // FINITE_STATE_MACHINE_H