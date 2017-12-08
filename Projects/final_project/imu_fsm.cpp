#include "imu_fsm.h"

namespace ImuFsm {
	char prev_key = '!';
	state_t do_state_default(instance_data_t *data) {
		// printf("prev key: %c\n", prev_key);
		if (data->event == GESTURE_IN) {
			char gesture = data->key.c_str()[0];
			switch (gesture) {
				case 'a':
					return STATE_WAIT_A;
				case '5':
					return STATE_WAIT_5;
				case 'q':
					return STATE_WAIT_AY_Q;
				case 'g':
					return STATE_WAIT_AY_G;
				case 'z':
					return STATE_WAIT_AX;
				case 'j':
					return STATE_WAIT_GZ;
				default:
					if (prev_key != data->key.c_str()[0]){
						data->event = KEY_OUT;
						prev_key = data->key.c_str()[0];
					} else {
						// printf("repeat\n");
					} // So if key is repeated, nothing happens and txn back to STATE_DEFAULT
					// data->key = gesture;
					return STATE_DEFAULT;
			 }
	  }
		return STATE_DEFAULT;
	}
	
	state_t do_state_wait(instance_data_t *data, char out, char mtn){
		if (data->event == MOTION_IN) {
			char motion = data->key.c_str()[0];
			if (prev_key == out) {
				// printf("repeat\n");
				return STATE_DEFAULT;
			}
			else if (motion == mtn) {
				  // printf("prev key is %c\n", prev_key);
					// printf("out is %c\n", out);
					data->event = KEY_OUT;
				  std::string s(1, out);
				  data->key = s;
					prev_key = out;
			 }
	  }
		return STATE_DEFAULT;
	}
	
	state_t do_state_wait_ayg(instance_data_t *data){
		return do_state_wait (data, 'g', 'y');
	}
	
	state_t do_state_wait_ayq(instance_data_t *data){
		return do_state_wait (data, 'q', 'y');
	}
	
	state_t do_state_wait_ax(instance_data_t *data){
	  return do_state_wait (data, 'z', 'X');
	}

	state_t do_state_wait_gz(instance_data_t *data){
		return do_state_wait (data, 'j', 'z');		
	}
	
	state_t do_state_fn(instance_data_t *data) {
		if (data->event == GESTURE_IN) {
			char gesture = data->key.c_str()[0];
			if (prev_key == data->key.c_str()[0]){
				data->event=NOOP;
				return STATE_DEFAULT;
			}
			data->event = KEY_OUT;
			char real_key;
			switch (gesture) {
				case 'd':
					real_key = '1';
					prev_key = real_key;
					break;
				case 'k':
				  real_key = '2';
					prev_key = real_key;
					break;
				case 'b':
				  real_key = '4';
					prev_key = real_key;
					break;
				case 'w':
				  real_key = '6';
					prev_key = real_key;
					break;
				case 'f':
				  real_key = '9';
					prev_key = real_key;
					break;
				default:
					data->event=NOOP;
			}
		
		string s(1, real_key);
		data->key = s;
		}
		return STATE_DEFAULT;
	}
	
	state_t do_state_wait_5(instance_data_t *data){
		if (data->event == MOTION_IN) {
			char motion = data->key.c_str()[0];
			// this is REPEAT -> replay prev_key.
			if (motion == 'y') {
				  // printf("prev key is %c\n", prev_key);
					// printf("out is %c\n", out);
					data->event = KEY_OUT;
				  std::string s(1, prev_key);
				  data->key = s;
			 } else if (motion == 'X'){
				 if (prev_key == '5') {
					 return STATE_DEFAULT;
				 } else {
					data->event = KEY_OUT;
				  data->key = "5";
					prev_key = '5';
				 }
			 } else if (motion == 'z'){
					data->event = KEY_OUT;
				  data->key = "SPACE";

			 } else if (motion == 'Z'){
					data->event = KEY_OUT;
				  data->key = "BS";
			 }
	  }
		return STATE_DEFAULT;
	}

	state_t do_state_wait_a(instance_data_t *data){
		if (data->event == MOTION_IN) {
			char motion = data->key.c_str()[0];
			if (motion == 'Z') {
					data->event = NOOP;
					printf("SHIFT ACTIVATED\n");
				  return FN_LAYER_ACTIVE;
			 } else if (motion == 'X'){
				 if (prev_key == 'a') {
					 return STATE_DEFAULT;
				 } else {
					data->event = KEY_OUT;
				  data->key = "a";
					prev_key = 'a';
				 }
			 }
	  }
		return STATE_DEFAULT;
	}

	state_t run_state( state_t cur_state, instance_data_t *data ) {
    return state_table[ cur_state ]( data );
	}

}