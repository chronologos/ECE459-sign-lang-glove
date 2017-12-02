#ifndef IMU_FSM_H
#define IMU_FSM_H

#include "mbed.h"                    
#include <string>

namespace ImuFsm {
/* This module contains code implementing a mealy machine that we use to synthesize 
 * our different sensor inputs.
 */
typedef enum {
	STATE_DEFAULT,
	STATE_WAIT_AY_Q,
	STATE_WAIT_AY_G,
	STATE_WAIT_AX,
	STATE_WAIT_GZ,
	FN_LAYER_ACTIVE, // When shift/function layer of glove is active
	                 // analogous to when you hold down "shift" on a keyboard.
	NUM_STATES 
} state_t;

typedef enum {
	GESTURE_IN, // instance_data_t->key is interpreted as a character in ASL
	MOTION_IN, // instance_data_t->key is interpreted a motion
	TIMEOUT_IN, // instance_data_t->key is ignored
	KEY_OUT, // instance_data_t->key is used to output a character in ASL
	FN_TOGGLE_IN // instance_data_t->key is ignored
} event_t;

/* instance_data_t is used to pass input into fsm and return output to client.
 * We do this by mutating the struct inside do_state_x functions. 
 */ 
typedef struct {
	event_t event;
	string key; // populated with gesture name if event=GESTURE,
	            // populated with key pressed if event=RETURN_KEY
		          // otherwise empty.
	string prev_key;
}	instance_data_t;

typedef state_t state_func_t(instance_data_t *data);
state_t do_state_default(instance_data_t *data);
state_t do_state_wait_ayq(instance_data_t *data);
state_t do_state_wait_ayg(instance_data_t *data);
state_t do_state_wait_ax(instance_data_t *data);
state_t do_state_wait_gz(instance_data_t *data);
state_t do_state_fn(instance_data_t *data);

state_func_t* const state_table[ NUM_STATES ] = {
    do_state_default, do_state_wait_ayq, do_state_wait_ayg, do_state_wait_ax, do_state_wait_gz, do_state_fn
};

state_t run_state( state_t cur_state, instance_data_t *data );
}

#endif /* IMU_FSM_H */
