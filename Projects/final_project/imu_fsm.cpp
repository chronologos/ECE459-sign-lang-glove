#include "imu_fsm.h"

namespace ImuFsm {
	state_t do_state_default(instance_data_t *data) {
		if (data->event == GESTURE_IN) {
			char gesture = data->key.c_str()[0];
			switch (gesture) {
				case 'q':
					return STATE_WAIT_AY_Q;
				case 'g':
					return STATE_WAIT_AY_G;
				case 'z':
					return STATE_WAIT_AX;
				case 'j':
					return STATE_WAIT_GZ;
				default:
					if (data->prev_key != data->key){
						data->event = KEY_OUT;
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
			if (motion == mtn) {
					data->event = KEY_OUT;
					data->key = out;
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
	  return do_state_wait (data, 'z', 'x');
	}

	state_t do_state_wait_gz(instance_data_t *data){
		return do_state_wait (data, 'j', 'z');		
	}
	
	state_t run_state( state_t cur_state, instance_data_t *data ) {
    return state_table[ cur_state ]( data );
};
	state_t do_state_fn(instance_data_t *data) {
		return STATE_DEFAULT;
	}

}