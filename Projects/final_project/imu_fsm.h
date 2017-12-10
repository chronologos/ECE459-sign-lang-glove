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
		STATE_WAIT_GDZQ,
		STATE_WAIT_IJ,
		FN_LAYER_ACTIVE, // When shift/function layer of glove is active
		// analogous to when you hold down "shift" on a keyboard.
		STATE_WAIT_5,
		STATE_WAIT_A,
		STATE_WAIT_SXT,
		STATE_WAIT_CO,
		NUM_STATES
	} state_t;

	typedef enum {
		GESTURE_IN, // instance_data_t->key is interpreted as a character in ASL
		MOTION_IN, // instance_data_t->key is interpreted a motion
		TIMEOUT_IN, // instance_data_t->key is ignored
		KEY_OUT, // instance_data_t->key is used to output a character in ASL
		NOOP // instance_data_t->key is ignored
	} event_t;

	/* instance_data_t is used to pass input into fsm and return output to client.
	* We do this by mutating the struct inside do_state_x functions.
	*/
	typedef struct {
		event_t event;
		char key; // populated with gesture name if event=GESTURE,
		// populated with key pressed if event=RETURN_KEY
		// otherwise empty.
		// Gesture name notation: x,y,z (positive XM), X,Y,Z (negative XM),
		// a,b,c and A,B,C for gyro
		char aux; // For m and n disambiguation.
	}	instance_data_t;

	// STATE TXN FUNCTIONS:
	typedef state_t state_func_t(instance_data_t *data);
	// Dispatches keys in default layer.
	state_t do_state_default(instance_data_t *data);
	// Distinguishes between g, d, z, q
	state_t do_state_wait_gdzq(instance_data_t *data);
	// Distinguishes between i, j
	state_t do_state_wait_ij(instance_data_t *data);
	// Analogous to do_state_default, but dispatches keys in function layer.
	state_t do_state_fn(instance_data_t *data);
	// Distinguishes 5, <backspace>, <space>, <repeat>
	state_t do_state_wait_5(instance_data_t *data);
	// Distinguishes <shift> and a
	state_t do_state_wait_a(instance_data_t *data);
	// Distinguishes s, x and t using IMU
	state_t do_state_wait_sxt(instance_data_t *data);
	// Distinguishes c and o using IMU
	state_t do_state_wait_co(instance_data_t *data);
	
	// STATE TXN TABLE:
	// MUST BE ORDERED IN CORRESPONDENCE WITH enum state_t
	state_func_t* const state_table[ NUM_STATES ] = {
		do_state_default,
		do_state_wait_gdzq, do_state_wait_ij,
		do_state_fn,
		do_state_wait_5, do_state_wait_a,
		do_state_wait_sxt, do_state_wait_co
	};

	// Transitions mealy machine state:
	// data - input to fsm and outputs if any
	// cur_state - current fsm state
	// returns new fsm state
	state_t run_state( state_t cur_state, instance_data_t *data );
}

#endif /* IMU_FSM_H */
