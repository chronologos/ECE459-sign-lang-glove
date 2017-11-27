#ifndef IMU_FSM_H
#define IMU_FSM_H

#include "mbed.h"                    
#include <string>

namespace ImuFsm {
	
typedef enum {
	STATE_DEFAULT,
	STATE_WAIT_AY_Q,
	STATE_WAIT_AY_G,
	STATE_WAIT_AX,
	STATE_WAIT_GZ,
	NUM_STATES 
} state_t;

typedef enum {
	GESTURE_IN,
	MOTION_IN,
	TIMEOUT_IN,
	KEY_OUT
} event_t;

/* instance_data_t is used to pass input into fsm and return output to client.
 * We do this by mutating the struct inside do_state_x functions. 
 */ 
typedef struct {
	event_t event;
	string key; // populated with gesture name if event=GESTURE,
	            // populated with key pressed if event=RETURN_KEY
		          // otherwise empty.
}	instance_data_t;

typedef state_t state_func_t(instance_data_t *data);
state_t do_state_default(instance_data_t *data);
state_t do_state_wait_ayq(instance_data_t *data);
state_t do_state_wait_ayg(instance_data_t *data);
state_t do_state_wait_ax(instance_data_t *data);
state_t do_state_wait_gz(instance_data_t *data);

state_func_t* const state_table[ NUM_STATES ] = {
    do_state_default, do_state_wait_ayq, do_state_wait_ayg, do_state_wait_ax, do_state_wait_gz
};

state_t run_state( state_t cur_state, instance_data_t *data );
}

#endif /* IMU_FSM_H */
