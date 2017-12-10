#include "imu_fsm.h"

namespace ImuFsm {
	char digits[] = "0123456789";
	bool repeated = false;
	void checkPrevKeyAndSetAnother(instance_data_t *data, char *prev_key, 
		                             char set, char prev) {
			if (*prev_key == prev){
			data->event = NOOP;
		} else {
			data->event = KEY_OUT;
			data->key = set;
			*prev_key = prev;
		}
	}
	void checkPrevKeyAndSet(instance_data_t *data, char *prev_key, char key) {
		checkPrevKeyAndSetAnother(data, prev_key, key, key);
	}

	char prev_key = 0x00;
	state_t do_state_default(instance_data_t *data) {
		// printf("prev key: %c\n", prev_key);
		if (data->event == GESTURE_IN) {
			char gesture = data->key;
			data->event=NOOP;
			switch (gesture) {
				case 'a':
					return STATE_WAIT_A;
				case '5':
					return STATE_WAIT_5;
				case 'g':
					return STATE_WAIT_GQ;
				case 'd':
					return STATE_WAIT_DZ;
				case 'z':
					return STATE_WAIT_DZ;
				case 'q':
					return STATE_WAIT_GQ;
				case 'i':
					return STATE_WAIT_IJ;
				case 'j':
					return STATE_WAIT_IJ;
				case 'm':
					data->aux = 'm';
					return STATE_WAIT_SXT;
				case 'n':
					data->aux = 'n';
					return STATE_WAIT_SXT;
				case 's':
					data->aux = 'n';
					return STATE_WAIT_SXT;
				case 'x':
					data->aux = 'n';
					return STATE_WAIT_SXT;
				case 't':
					data->aux = 'n';
					return STATE_WAIT_SXT;
				case 'c':
					return STATE_WAIT_CO;
				case 'o': 
					return STATE_WAIT_CO;
				case 'h': 
					return STATE_WAIT_HU;
				case 'u': 
					return STATE_WAIT_HU;
				case 'p': 
					return STATE_WAIT_PK;
				case 'k': 
					return STATE_WAIT_PK;
				default:
				// Ignore digits, they are in function layer.
				for (int i = 0; i < 10; ++i){
					if (digits[i] == gesture){
						return STATE_DEFAULT;
					}
				}
				checkPrevKeyAndSet(data, &prev_key, data->key);
				return STATE_DEFAULT;
			}
		}
		return STATE_DEFAULT;
	}

	state_t do_state_wait_gq(instance_data_t *data){
		if (data->event == MOTION_IN) {
			char motion = data->key;
			if (motion == 'y') {
				checkPrevKeyAndSet(data, &prev_key, 'g');
			} else if (motion == 'Z'){
				checkPrevKeyAndSet(data, &prev_key, 'q');
			}
		}
		return STATE_DEFAULT;
	}
	
	state_t do_state_wait_dz(instance_data_t *data){
		if (data->event == MOTION_IN) {
			char motion = data->key;
			if (motion == 'X') {
				checkPrevKeyAndSet(data, &prev_key, 'd');
			} else if (motion == 'y'){
				checkPrevKeyAndSet(data, &prev_key, 'z');
			}
		}
		return STATE_DEFAULT;
	}

	state_t do_state_wait_ij(instance_data_t *data){
		if (data->event == MOTION_IN) {
			char motion = data->key;
			if (motion == 'g') {
				checkPrevKeyAndSet(data, &prev_key, 'j');
			} else if (motion == 'X') { 
				checkPrevKeyAndSet(data, &prev_key, 'i');
			}
		}
		return STATE_DEFAULT;
	}

	state_t do_state_fn(instance_data_t *data) {
		if (data->event == GESTURE_IN) {
			char gesture = data->key;
			switch (gesture) {
				case 'g':
					checkPrevKeyAndSetAnother(data, &prev_key, '1', 'g');
					return STATE_DEFAULT;
				case '1':
					checkPrevKeyAndSetAnother(data, &prev_key, '1', '1');
					return STATE_DEFAULT;
				case 'd':
					checkPrevKeyAndSetAnother(data, &prev_key, '1', 'd');
					return STATE_DEFAULT;
				case 'z':
					checkPrevKeyAndSetAnother(data, &prev_key, '1', 'z');
					return STATE_DEFAULT;
				case 'q':
					checkPrevKeyAndSetAnother(data, &prev_key, '1', 'q');
					return STATE_DEFAULT;
				case 'u':
					checkPrevKeyAndSetAnother(data, &prev_key, '2', 'u');
					return STATE_DEFAULT;
				case 'v':
					checkPrevKeyAndSetAnother(data, &prev_key, '2', 'v');
					return STATE_DEFAULT;
				case 'k':
					checkPrevKeyAndSetAnother(data, &prev_key, '2', 'k');
					return STATE_DEFAULT;
				case 'h':
					checkPrevKeyAndSetAnother(data, &prev_key, '2', 'h');
					return STATE_DEFAULT;
				case '3':
					checkPrevKeyAndSetAnother(data, &prev_key, '3', '3');
					return STATE_DEFAULT;
				case 'b':
					checkPrevKeyAndSetAnother(data, &prev_key, '4', 'b');
					return STATE_DEFAULT;
				case '5':
					checkPrevKeyAndSetAnother(data, &prev_key, '5', '5');
					return STATE_DEFAULT;
				case 'w':
					checkPrevKeyAndSetAnother(data, &prev_key, '6', 'w');
					return STATE_DEFAULT;
				case '7':
					checkPrevKeyAndSetAnother(data, &prev_key, '7', '7');
					return STATE_DEFAULT;
				case '8':
					checkPrevKeyAndSetAnother(data, &prev_key, '8', '8');
					return STATE_DEFAULT;
				case 'f':
					checkPrevKeyAndSetAnother(data, &prev_key, '9', 'f');
					return STATE_DEFAULT;
				default:
					data->event=NOOP;
					return FN_LAYER_ACTIVE;
			}
		}
		return STATE_DEFAULT;
	}

	state_t do_state_wait_5(instance_data_t *data){
		if (data->event == MOTION_IN) {
			char motion = data->key;
			// this is REPEAT -> replay prev_key.
			if (motion == 'y') {
				if (repeated){
					wait_ms(300);
					repeated = false;
					data->event = NOOP;
					return STATE_DEFAULT;
				}
				repeated = true;
				// printf("prev key is %c\n", prev_key);
				printf("REPEAT ACTIVATED\n");
				data->event = KEY_OUT;
				data->key = prev_key;
			} else if (motion == 'z'){
					checkPrevKeyAndSet(data, &prev_key, 0x20); // SPACE
			} else if (motion == 'Z'){ // BACKSPACE
				data->event = KEY_OUT;
				data->key = '!';
			} else {
				data->event = NOOP;
				prev_key = '.';
			}				
		}
		return STATE_DEFAULT;
	}

	state_t do_state_wait_a(instance_data_t *data){
		if (data->event == MOTION_IN) {
			char motion = data->key;
			if (motion == 'Z') {
				data->event = NOOP;
				printf("SHIFT ACTIVATED\n");
				return FN_LAYER_ACTIVE;
			} else if (motion == 'X'){
					checkPrevKeyAndSet(data, &prev_key, 'a');
			} else if (motion == 'y'){
					checkPrevKeyAndSet(data, &prev_key, 0x0F);
			}
		}
		return STATE_DEFAULT;
	}

	state_t do_state_wait_sxt(instance_data_t *data) {
		if (data->event == MOTION_IN) {
			char motion = data->key;
			if (motion == 'Z') {
				checkPrevKeyAndSet(data, &prev_key, 'x');
			}  else if (motion == 'y'){
				checkPrevKeyAndSet(data, &prev_key, 't');
			} else if (motion == 'Y'){
				checkPrevKeyAndSet(data, &prev_key, 's');
			} else {
				// m or n depending on what caused this state
				checkPrevKeyAndSet(data, &prev_key, data->aux);
			}
		}
		return STATE_DEFAULT;
	}
	
		state_t do_state_wait_co(instance_data_t *data) {
		if (data->event == MOTION_IN) {
			char motion = data->key;
			if (motion == 'Z') {
				checkPrevKeyAndSet(data, &prev_key, 'o');
			} else {
				// m or n depending on what caused this state
				checkPrevKeyAndSet(data, &prev_key, 'c');
			}
		}
		return STATE_DEFAULT;
	}
		
		state_t do_state_wait_hu(instance_data_t *data) {
		if (data->event == MOTION_IN) {
			char motion = data->key;
			if (motion == 'y') {
				checkPrevKeyAndSet(data, &prev_key, 'h');
			} else {
				checkPrevKeyAndSet(data, &prev_key, 'u');
			}
		}
		return STATE_DEFAULT;
	}
		
	state_t do_state_wait_pk(instance_data_t *data) {
		if (data->event == MOTION_IN) {
			char motion = data->key;
			if (motion == 'Z') {
				checkPrevKeyAndSet(data, &prev_key, 'p');
			} else {
				checkPrevKeyAndSet(data, &prev_key, 'k');
			}
		}
		return STATE_DEFAULT;
	}


	state_t run_state( state_t cur_state, instance_data_t *data ) {
		return state_table[ cur_state ]( data );
	}

}
