#include "imu_fsm.h"

namespace ImuFsm {
	
	void checkPrevKeyAndSet(instance_data_t *data, char *prev_key, char key) {
		if (*prev_key == key){
			data->event = NOOP;
		} else {
			data->event = KEY_OUT;
			string s(1, key);
			data->key = s;
			*prev_key = key;
		}
	}
		
	char prev_key = '!';
	state_t do_state_default(instance_data_t *data) {
		// printf("prev key: %c\n", prev_key);
		if (data->event == GESTURE_IN) {
			char gesture = data->key.c_str()[0];
			data->event=NOOP;
			switch (gesture) {
				case 'a':
					return STATE_WAIT_A;
				case '5':
					return STATE_WAIT_5;
				case 'g':
					return STATE_WAIT_GDZQ;
				case 'd':
					return STATE_WAIT_GDZQ;
				case 'z':
					return STATE_WAIT_GDZQ;
				case 'q':
					return STATE_WAIT_GDZQ;
				case 'i':
					return STATE_WAIT_IJ;
				case 'j':
					return STATE_WAIT_IJ;
				case 's':
					return STATE_WAIT_MNSXT;
				case 'x':
					return STATE_WAIT_MNSXT;
				case 't':
					return STATE_WAIT_MNSXT;
				default:
					checkPrevKeyAndSet(data, &prev_key, data->key.c_str()[0]);
					return STATE_DEFAULT;
			 }
	  }
		return STATE_DEFAULT;
	}
		
	state_t do_state_wait_gdzq(instance_data_t *data){
		if (data->event == MOTION_IN) {
			char motion = data->key.c_str()[0];
			// this is REPEAT -> replay prev_key.
			if (motion == 'y') {
				  // printf("prev key is %c\n", prev_key);
					// printf("out is %c\n", out);
					checkPrevKeyAndSet(data, &prev_key, 'g');      
			 } else if (motion == 'X'){
				  checkPrevKeyAndSet(data, &prev_key, 'd');
			 } else if (motion == 'Y'){
				  checkPrevKeyAndSet(data, &prev_key, 'z');
			 } else if (motion == 'x'){
				  checkPrevKeyAndSet(data, &prev_key, 'q');
			 }
	  }
		return STATE_DEFAULT;
	}
	
	state_t do_state_wait_ij(instance_data_t *data){
		if (data->event == MOTION_IN) {
			char motion = data->key.c_str()[0];
			// this is REPEAT -> replay prev_key.
			if (motion == 'X') {
				checkPrevKeyAndSet(data, &prev_key, 'i');
			 } else if (motion == 'c'){
					checkPrevKeyAndSet(data, &prev_key, 'j');
			 }
	  }
		return STATE_DEFAULT;
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
				case 'g':
					real_key = '1';
					prev_key = real_key;
					break;
				case 'd':
					real_key = '1';
					prev_key = real_key;
					break;
				case 'z':
					real_key = '1';
					prev_key = real_key;
					break;
				case 'u':
				  real_key = '2';
					prev_key = real_key;
					break;
				case 'v':
				  real_key = '2';
					prev_key = real_key;
					break;
				case 'k':
				  real_key = '2';
					prev_key = real_key;
					break;
				case 'h':
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
					printf("repeat triggered\n");
					data->event = KEY_OUT;
				  std::string s(1, prev_key);
				  data->key = s;
			 } else if (motion == 'X'){
					checkPrevKeyAndSet(data, &prev_key, '5');
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

	state_t do_state_wait_mnsxt(instance_data_t *data) {
		if (data->event == MOTION_IN) {
			char motion = data->key.c_str()[0];
			if (motion == 'Z') {
					data->event = KEY_OUT;
				  data->key = "x";
		  } else if (motion == 'y'){
					data->event = KEY_OUT;
				  data->key = "t";

			} else if (motion == 'Y'){
					data->event = KEY_OUT;
				  data->key = "s";
			}
		}
	return STATE_DEFAULT;
	}

	state_t run_state( state_t cur_state, instance_data_t *data ) {
    return state_table[ cur_state ]( data );
	}

}