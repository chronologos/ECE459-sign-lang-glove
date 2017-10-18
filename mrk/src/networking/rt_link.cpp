/******************************************************************************
*  Nano-RK, a real-time operating system for sensor networks.
*  Copyright (C) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
*  All rights reserved.
*
*  This is the Open Source Version of Nano-RK included as part of a Dual
*  Licensing Model. If you are unsure which license to use please refer to:
*  http://www.nanork.org/nano-RK/wiki/Licensing
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, version 2.0 of the License.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*  Contributing Authors (specific to this file):
*  Anthony Rowe
*******************************************************************************/

#include <rt_link.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <nrk.h>
#include <nrk_events.h>
#include <nrk_timer.h>
#include <nrk_error.h>
#include <rtl_defs.h>
#include <stdlib.h>

//#define TXT_DEBUG


static uint16_t global_slot;
static uint16_t global_cycle;

static uint16_t slot_start_time;
static uint16_t last_sync_slot;
static uint16_t current_global_slot;

static uint8_t _rtl_sync_ok;
static uint8_t _rtl_time_token; 
static uint8_t _rtl_time_token_status;

static RF_TX_INFO rtl_tsync_tx;
static char rtl_tsync_buf[PKT_DATA_START];
static uint8_t _rtl_contention_slots;


RF_RX_INFO rtl_rfRxInfo;
RF_TX_INFO rtl_rfTxInfo;	
nrk_sig_t rtl_tx_done_signal;
nrk_sig_t rtl_rx_pkt_signal;
	
static rtl_param_t rtl_param;
static rtl_node_mode_t rtl_node_mode;	

static uint8_t rtl_rx_data_ready;

static volatile uint8_t rtl_rx_slot;
static volatile char rtl_rx_buf[RF_MAX_PAYLOAD_SIZE];

static RTL_TX_INFO	rtl_tx_info[TDMA_FRAME_SLOTS+1];

static uint16_t rtl_abs_tx_slot;
static uint8_t rtl_abs_tx_ready; 
static uint32_t rtl_tx_data_ready;

static uint32_t rtl_tdma_rx_mask;
static uint32_t rtl_tdma_tx_mask;

void (*rx_callback)(uint8_t slot);
void (*tx_callback)(uint8_t slot); //Declared Extern
void (*abs_callback)(uint16_t global_slot); //Declared Extern
void (*slot_callback)(uint16_t global_slot); //Declared Extern
void (*cycle_callback)(uint16_t global_cycle); // cycles since the epoch,Declared Extern

static nrk_task_type rtl_task;   //Declared static by tharun on 11/22
static NRK_STK rtl_task_stack[RT_LINK_STACK_SIZE];//declared static by tharun on 11/22

static uint8_t _rtl_contention_pending;
static volatile uint8_t _rtl_ready;

// some handy book keeping values
static uint16_t last_slot; 	//Declared Extern
static nrk_time_t last_slot_time; //Declared Extern

/************************************rt_scheduler.h**************************************
 **************************************Declarations**************************************/
static uint8_t rtl_sched[16];            // only one since you can TX and RX on the same slot,Decleared Extern
static uint8_t rtl_sched_cache[32];				//Declared Extern
static uint16_t rtl_abs_wakeup[MAX_ABS_WAKEUP];  // MSB is the repeat flag

uint16_t prev_offset; //Declared Extern
uint16_t rtl_drift_rate;//Declared Extern

uint8_t _tsync_index;	//Declared Extern
uint8_t _dbg_index;		//Declared Extern
uint8_t pkt_log[MAX_PKT_LOG/8];//Declared Extern
uint8_t tsync_log[MAX_TSYNC_LOG]; //Declared Extern
uint8_t tsync_delay[MAX_TSYNC_LOG]; //Declared Extern

/*****************************************************************************************/

/*****************************************************************************************
 **********************************rtl_scheduler.h****************************************/
/**
 * rtl_set_schedule ( rx_tx, slot, sched )
 *
 * Arguments: rtl_rx_tx_t rx_tx takes either RTL_RX or RTL_TX to set if you
 *            are setting an RX or a TX schedule.
 *            uint8_t slot sets which RX or TX slot you wish to schedule
 *            uint8_t sched takes in the log based schedule you wish to set
 *                      sched   counts
 *                      0       reserved
 *                      1       1
 *                      2       2
 *                      3       4
 *                      4       8
 *                      5       16
 *                      6       32
 *                      7       reserve
 *                      8       reserve
 *
 * example: rtl_set_schedule( RTL_RX, 5, 3 );
 *
 * Return: NRK_OK if good, NRK_ERROR if failed
 */
int8_t rtl_set_schedule (rtl_rx_tx_t rx_tx, uint8_t slot, uint8_t sched)
{
    uint32_t t_mask;
    uint8_t dslot;
    if (sched > 15 || slot > 31)
        return NRK_ERROR;
    t_mask = 0;
    t_mask = ((uint32_t) 1) << slot;
    if (rx_tx == RTL_RX)
        rtl_tdma_rx_mask |= t_mask;
    else
        rtl_tdma_tx_mask |= t_mask;
    dslot = slot >> 1;
    if (slot % 2 == 0) {
        rtl_sched[dslot] = rtl_sched[dslot] & 0xF0;
        rtl_sched[dslot] = rtl_sched[dslot] | sched;
    }
    else {
        rtl_sched[dslot] = rtl_sched[dslot] & 0x0F;
        rtl_sched[dslot] = rtl_sched[dslot] | ((sched << 4) & 0xF0);
    }
return NRK_OK;
/*
   printf( "slot = %d sched = %d\n", slot,sched );
   printf( "index = %d\n", dslot);
   printf( "value = %d\n", rtl_rx_sched[dslot]);
   printf( "mask = %x\n", rtl_tdma_rx_mask );
   printf( "--------------------------\n");
*/
}





/**
 * rtl_get_schedule()
 *
 * This function returns the stored schedule for a particular slot.
 * Since you can't have RX and TX on the same slot, it assumes you know
 * if it is an RX or TX slot.
 *
 * Return: schedule value in its original form 1-6
 */
int8_t rtl_get_schedule (uint8_t slot)
{
    uint8_t dslot;

    if (slot > 31)
        return NRK_ERROR;

    dslot = slot >> 1;
    if (slot % 2 == 0)
        return (rtl_sched[dslot] & 0x0F);
    else
        return (rtl_sched[dslot] >> 4);
}

/**
 * rtl_clr_schedule()
 *
 * This function clears an already scheduled slot so that it
 * is not called by the scheduler anymore.  Use this to delete
 * slots when they are not wanted anymore.
 *
 * Arguments: rtl_rx_tx_t rx_tx is either RTL_RX or RTL_TX depending on if it
 *            is a TX or RX slot
 *            uint8_t slot is the slot number starting from 0
 *
 * Return: NRK_OK upon success, NRK_ERROR on failure
 */
int8_t rtl_clr_schedule (rtl_rx_tx_t rx_tx, uint8_t slot)
{
    uint32_t t_mask;
    uint8_t dslot;
    if (slot > 31)
        return NRK_ERROR;
    t_mask = 0;
    t_mask = 1 << slot;
    if (rx_tx == RTL_RX)
        rtl_tdma_rx_mask &= ~t_mask;
    else
        rtl_tdma_tx_mask &= ~t_mask;
    dslot = slot >> 1;
    if (slot % 2 == 0)
        rtl_sched[dslot] = rtl_sched[dslot] & 0xF0;
    else
        rtl_sched[dslot] = rtl_sched[dslot] & 0x0F;
		
		return 1;
}

/**
 * _rtl_clr_abs_all_wakeup()
 *
 * This function clears all absolute wakeups. 
 */
void _rtl_clr_abs_all_wakeup ()
{
    uint8_t i;
    for (i = 0; i < MAX_ABS_WAKEUP; i++)
        rtl_abs_wakeup[i] = MAX_SLOTS + 1;
}

/**
 * rtl_set_abs_wakeup()
 *
 * This function sets an absolute wakeup.  An absolute wakeup
 * is a 16 bit slot value 0-1024 that signifies that the interrupt
 * will be called at this point of each cycle.  This function does
 * not check if a duplicate wakeup exists, so try to avoid them.  
 * There is only room for MAX_ABS_WAKEUP number of scheduled events.
 *
 * Argument: uint16_t slot is the value of a slot between 0 and 1024
 *           uint8_t repeat is 1 if this should always trigger each cycle
 *                 if repeat is 0, it is cleared after it triggers once.
 *
 * Return: 1 on success, 0 if no slots are available
 *
 */
int8_t rtl_set_abs_wakeup (uint16_t slot, uint8_t repeat)
{
    uint8_t i;
    uint16_t tmp;
    for (i = 0; i < MAX_ABS_WAKEUP; i++) {
        tmp=rtl_abs_wakeup[i]&0x7FFF;
        if (tmp > MAX_SLOTS) {
            rtl_abs_wakeup[i] = slot;
            if(repeat) rtl_abs_wakeup[i]|=0x8000;
            return NRK_OK;
        }
    }
return NRK_ERROR;
}

/**
 * _rtl_clr_abs_wakeup()
 *
 * This function clears an already set absolute wakeup.
 */
void _rtl_clr_abs_wakeup (uint16_t slot)
{
    uint8_t i;
    
    for (i = 0; i < MAX_ABS_WAKEUP; i++) {
        uint16_t tmp;
        tmp=rtl_abs_wakeup[i]&0x7FFF;
        if (tmp == slot) {
            rtl_abs_wakeup[i] = MAX_SLOTS + 1;
            return;
        }
    }

}

/**
 * _rtl_match_abs_wakeup()
 *
 * This function is called by the interrupt timer to check to
 * see if there is a scheduled slot at the current time.
 *
 * Return: 1 if this is a scheduled slot, 0 otherwise
 */
uint8_t _rtl_match_abs_wakeup (uint16_t global_slot)
{
    uint8_t i;
    for (i = 0; i < MAX_ABS_WAKEUP; i++) {
        uint16_t tmp;
        tmp=rtl_abs_wakeup[i]&0x7FFF;
        if (tmp == global_slot)
        {
            //if( (rtl_abs_wakeup[i]&0x8000)==0 )
            if( rtl_abs_wakeup[i]<=0x8000 )
                {
                // If it is not a repeat slot, clear it
                _rtl_clr_abs_wakeup(global_slot);
                }
            return 1;
        }
    }
    return 0;
}

/**
 * _rtl_get_next_abs_wakeup()
 *
 * This function returns the number of slots between the given
 * global_slot and the next already scheduled absolute wakeup.
 * This is used by the timer interrupt to help schedule when it needs 
 * to wakeup again.
 * 
 * Arguments: global_slot is the current slot
 *
 * Return: uint16_t with the offset until the next absolute wakeup.  If the
 *         next wakeup is greater than 1024, then 0 is returned.
 */
uint16_t _rtl_get_next_abs_wakeup (uint16_t global_slot)
{
    uint8_t i;
    int16_t min;
    int16_t tmp;
    min = MAX_SLOTS +1;

    for (i = 0; i < MAX_ABS_WAKEUP; i++) {
        if((rtl_abs_wakeup[i]&0x7FFF)<MAX_SLOTS)
        {
        tmp = (rtl_abs_wakeup[i]&0x7FFF) - global_slot;
        if (tmp > 0 && tmp < min)
            min = tmp;
        }
    }
    if (min == MAX_SLOTS +1)
        min = 0;
    return min;
}

/**
 * _rtl_clear_sched_cache()
 *
 * This function is called by the timer interrupt at the
 * start of each TDMA cycle to remove any cached scheduling
 * values.  Only call this if you are reseting the TDMA frames.
 */
void _rtl_clear_sched_cache ()
{
    uint8_t i;
// FIXME compress this shit later...
    for (i = 0; i < 32; i++) {
        rtl_sched_cache[i] = 0;
    }
}


/**
 * rtl_get_slots_until_next_wakeup()
 *
 * This function returns the absolute number of slots between the current_slot
 * and the next RX/TX related wakeup.  It uses an internal cache to allow for
 * faster computation.
 *
 * Argument: current_slot is the current slot
 * Return: uint16_t number of slots until the next wakeup
 */
uint16_t rtl_get_slots_until_next_wakeup (uint16_t current_slot)
{
    uint16_t abs_slot;
    uint16_t min_slot;
    uint8_t test_slot;
    uint8_t test_frame;
    uint8_t frame_inc;

//total_slot = (((uint16_t)current_frame)<<5) + current_slot; 
    min_slot = MAX_SLOTS + 1;
    for (test_slot = 0; test_slot < 32; test_slot++) {
        uint8_t s;
        s = rtl_get_schedule (test_slot);
        if (s == 0)
            continue;
        s--;
        //test_frame=0;   
        test_frame = rtl_sched_cache[test_slot];
        frame_inc = _rtl_pow (2, s);
        do {
            abs_slot = (((uint16_t) test_frame) << 5) + test_slot;
            if (abs_slot <= current_slot)
                test_frame += frame_inc;
            //printf_u( "." );
        } while (abs_slot <= current_slot);
        rtl_sched_cache[test_slot] = test_frame;
         //printf( "current_slot = %d,  test_slot = %d, abs_slot=%d\n",current_slot, test_slot, abs_slot );
        if (abs_slot - current_slot < min_slot && abs_slot < MAX_SLOTS + 1)
            min_slot = abs_slot - current_slot;
    }
// If next slot is in the next TDMA cycle, return 0 to wakeup at the start if the frame.
    if (min_slot > MAX_SLOTS)
        return 0;
    return min_slot;
}

/**
 * _rtl_pow()
 *
 * This is a little helper function to do powers in order to 
 * resolve the schedules.  This is all 8 bit and should not
 * be forced to overflow.
 *
 * Arguments: x and y to compute x^y
 * Return: x^y
 */
uint8_t _rtl_pow (uint8_t x, uint8_t y)
{
    uint8_t acc, i;
    if (y == 0)
        return 1;
    acc = x;
    for (i = 0; i < y - 1; i++)
        acc = acc * x;
    return acc;
}
	
/******************************************************************************************/

/******************************************************************************************
 ***************************************rtl_debug.h****************************************/
  void rtl_debug_time_update(uint16_t offset)
{
    rtl_drift_rate=offset-prev_offset;
    prev_offset=offset; 
}

int16_t rtl_debug_time_get_drift()
{
return rtl_drift_rate;
}

void rtl_debug_init()
{
uint8_t i;
for(i=0; i<MAX_PKT_LOG/8; i++ )
        pkt_log[i]=0xFF;
for(i=0; i<MAX_TSYNC_LOG/8; i++ )
        tsync_log[i]=0xFF;
_dbg_index=0;
prev_offset=0;
}

uint8_t bit_count(uint8_t n)
{
n = (n & 0x55) + ((n>>1) & 0x55);
n = (n & 0x33) + ((n>>2) & 0x33);
n = (n & 0x0F) + ((n>>4) & 0x0F);
return n;
}

uint8_t rtl_debug_get_pkt_loss()
{
uint8_t i,j,cnt,tmp;
cnt=0;
for(i=0; i<MAX_PKT_LOG/8; i++ )
        cnt+=bit_count(pkt_log[i]);
return cnt;
}

uint8_t rtl_debug_get_tsync_loss()
{
uint8_t i,j,cnt,tmp;
cnt=0;
for(i=0; i<MAX_TSYNC_LOG/8; i++ )
        cnt+=bit_count(tsync_log[i]);
return cnt;
}



void rtl_debug_rx_pkt()
{
uint8_t offset;
offset=_dbg_index/8;
pkt_log[offset]|=(1<<(7-(_dbg_index%8)));
_dbg_index++;
if(_dbg_index>MAX_PKT_LOG) _dbg_index=0;
}

void rtl_debug_dropped_pkt()
{
uint8_t offset;
offset=_dbg_index/8;
pkt_log[offset]&=~(1<<(7-(_dbg_index%8)));
_dbg_index++;
if(_dbg_index>MAX_PKT_LOG) _dbg_index=0;
}

uint8_t rtl_debug_get_tsync_delay(uint8_t index)
{

if(index<MAX_TSYNC_LOG) return tsync_delay[index];
return 0;
}


void rtl_debug_rx_tsync()
{
uint8_t offset;
offset=_tsync_index/8;
tsync_log[offset]|=(1<<(7-(_tsync_index%8)));
tsync_delay[_tsync_index]=0;
_tsync_index++;
if(_tsync_index>MAX_TSYNC_LOG) _tsync_index=0;
}

void rtl_debug_dropped_tsync(uint8_t delay)
{
uint8_t offset;
offset=_tsync_index/8;
tsync_log[offset]&=~(1<<(7-(_tsync_index%8)));
tsync_delay[_tsync_index]=delay;
_tsync_index++;
if(_tsync_index>MAX_TSYNC_LOG) _tsync_index=0;
}
/*****************************************************************************************/

/************ start of main rt_link functions **************************/

nrk_sig_t rtl_get_rx_pkt_signal()
{
  if(rtl_ready()==NRK_ERROR) return NRK_ERROR;
  return rtl_rx_pkt_signal;
}

int8_t rtl_wait_until_tx_done (uint8_t slot)
{
   nrk_signal_register(rtl_tx_done_signal);
   while (rtl_tx_pkt_check(slot) != 0)
       nrk_event_wait (SIG(rtl_tx_done_signal));
   return NRK_OK;
}

int8_t rtl_wait_until_rx_pkt ()
{
    nrk_signal_register(rtl_rx_pkt_signal);
    if (rtl_rx_pkt_check() != 0)
        return NRK_OK;
    nrk_event_wait (SIG(rtl_rx_pkt_signal));
    return NRK_OK;
}

int8_t rtl_wait_until_rx_or_tx ()
{
    nrk_signal_register(rtl_rx_pkt_signal);
    nrk_signal_register(rtl_tx_done_signal);
    nrk_event_wait (SIG(rtl_rx_pkt_signal) | SIG(rtl_tx_done_signal));
    return NRK_OK;
}

/**
 * rtl_tx_packet()
 *
 * This function associates a slot with a particular packet that needs
 * to be sent. 
 * 
 * Arguments: RF_TX_INFO *tx is a pointer to a transmit structure, this structure
 *            must have a valid pPayload pointer to the real packet.
 *            uint8_t slot is the value of the tx slot (starting from 0)
 *
 * Return:  currently always returns 1
 */
int8_t rtl_tx_pkt (char *tx, uint8_t len, uint8_t slot)
{
    if (slot == RTL_CONTENTION) {
        slot = rand () % _rtl_contention_slots;
        slot = (TDMA_FRAME_SLOTS - slot - 1);
        _rtl_contention_pending = 1;
    }
    rtl_tx_info[slot].pPayload = tx;    // pass le pointer
    rtl_tx_info[slot].length = len;    // pass le pointer
    rtl_tx_data_ready |= ((uint32_t) 1 << slot);        // set the flag    
		return 1;		
}

uint8_t _rtl_rx_sync ()
{
    int8_t n;
    uint8_t tmp_token;
    uint16_t timeout,start_lat;
    uint16_t timer;
    uint8_t tdma_start_tick;
    uint8_t battery_save_cnt;
    uint8_t last_nrk_tick;
    uint16_t elapsed_time;
    int16_t tmp_offset;
    volatile uint16_t sfd_start_time;

  //  DISABLE_GLOBAL_INT ();
    timer=0;
    battery_save_cnt=0;

while(1)
{
	timer = 0;
rtl_rfRxInfo.pPayload[GLOBAL_SLOT]=0xFF;
rtl_rfRxInfo.pPayload[GLOBAL_SLOT+1]=0xFF;
#ifdef LED_DEBUG
    nrk_led_set(1);
#endif
#ifdef GPIO_RX_DEBUG
    nrk_gpio_set(NRK_DEBUG_1);
#endif

/*
    // prints out a measurement of OS Tick
    nrk_int_disable();
    last_nrk_tick=_nrk_get_raw_timer();
    while(1)
	{
	last_nrk_tick=_nrk_get_raw_timer();
	while(last_nrk_tick==_nrk_get_raw_timer()); 
	_nrk_stop_high_speed_timer();
	_nrk_reset_high_speed_timer();
	_nrk_start_high_speed_timer();
	last_nrk_tick=_nrk_get_raw_timer();
	while(last_nrk_tick==_nrk_get_raw_timer()); 
		_nrk_stop_high_speed_timer();
    		tick_to_slot_offset=_nrk_get_high_speed_timer();
		printf( "time = %d\r\n",tick_to_slot_offset );
}
*/

    //rf_set_rx (&rtl_rfRxInfo, rtl_param.channel);       // sets rx buffer and channel 
    rf_polling_rx_on ();
    n = 0;
    _rtl_sync_ok = 0;
    last_nrk_tick=0;  // should be 0 going in
    //_nrk_prev_timer_val=250;
    _nrk_set_next_wakeup(250);
    //timeout=200;
    while ((n = rf_rx_check_fifop()) == 0) {
	// every OS tick 
	if(last_nrk_tick!=_nrk_os_timer_get()) 
		{
			
		//_nrk_stop_high_speed_timer();
		//_nrk_reset_high_speed_timer();
		//_nrk_start_high_speed_timer();
		last_nrk_tick=_nrk_os_timer_get();
	   	timer++;
	   	if(timer>RTL_TOKEN_TIMEOUT) 
			{
			timer=0;
			rf_polling_rx_on ();
			//printf( "token timeout! %d\r\n",battery_save_cnt );
			_rtl_time_token=0;
			//printf("rttoken %d \r\n", _rtl_time_token);
			battery_save_cnt++;
			if(battery_save_cnt>RTL_BATTERY_SAVE_TIMEOUT)
				{
				battery_save_cnt=0;
				nrk_battery_save();
				}
           		break;
			} 
		}

    }
		

    // nrk_kprintf( PSTR("got sfd\r\n"));
    // Wait measured time until next slot
    // reset OS timer

    _nrk_high_speed_timer_reset();
    // capture SFD transition with high speed timer
    sfd_start_time=_nrk_high_speed_timer_get();
    tdma_start_tick=_nrk_os_timer_get();
    //nrk_gpio_set(NRK_DEBUG_1);

    timeout = tdma_start_tick+10;
    // an interrupt could happen in here and mess things up
		
    if (n != 0) {			
        n = 0;
        // Packet on its way
        while ((n = rf_polling_rx_packet ()) == 0) {
            if (_nrk_os_timer_get () > timeout)
		{
		//nrk_kprintf( PSTR("Pkt timed out\r\n") );
                break;          // huge timeout as failsafe
		}
        }
    }
			
    
    rf_rx_off ();
    if (n == 1 && rtl_rfRxInfo.length>0) {
				
        // CRC and checksum passed
        uint8_t explicit_sync_pkt;
				rtl_rx_data_ready = 1;
        //rtl_rx_slot = 0;
        global_slot = rtl_rfRxInfo.pPayload[GLOBAL_SLOT];
        global_slot <<= 8;
        global_slot |= rtl_rfRxInfo.pPayload[GLOBAL_SLOT + 1];
	
        //global_slot++;
        tmp_token= 0x7F & (rtl_rfRxInfo.pPayload[TIME_SYNC_TOKEN]);
	//printf( "%d: ",global_slot );
        //printf ("sync slot %d %d\r\n", global_slot, rtl_rfRxInfo.length);
	// Time Sync Token goes to 127 since MSB is the explicit sync flag
	// After 110, assume packets were dropped and try to catch the wrap
        // around case.
	//if((global_slot%32)!=1)
	//printf( "token %d %d\r\n",tmp_token,_rtl_time_token);
      
        // check if this should be just greater than! 
				//printf("%d %d \r\n", tmp_token, _rtl_time_token);
		//if(tmp_token >=_rtl_time_token || (_rtl_time_token>110 && tmp_token<10))
		//{						
    		rtl_rx_slot = (global_slot ) % 32;
		// only acccept sync if the token is greater than yours
		if((rtl_rfRxInfo.pPayload[TIME_SYNC_TOKEN]&0x80)==0 )
			{
			// Got normal packet 
			// if we got a good packet, send the signal to
			// the application

			// Check if we are supposed to RX on this slot, or if it was an overheard sync slot with data
			// If it had data but not for me, ignore it
			if((((uint32_t)1<<rtl_rx_slot) & rtl_tdma_rx_mask)==(uint32_t)0)
				rtl_rx_pkt_release(); 
			else
				nrk_event_signal (rtl_rx_pkt_signal);
			}
		// If it is a time sync packet, then release it
		// so it doesn't block a buffer...
		else { 
			// Explicit Sync
			rtl_rx_pkt_release(); 
		//	nrk_event_signal (SIG(RTL_RX_PKT_EVENT));

		}
		// Got a good signal...
		break;
		} 
		/*else 
		{
			nrk_kprintf( PSTR("Bad token\r\n") ); 
			printf( "tmp %d rtl %d\r\n",tmp_token,_rtl_time_token );
		}*/

    //} //else printf( "Error n=%d %d\r\n", n, rtl_rfRxInfo.length);
}
#ifdef LED_DEBUG
    nrk_led_clr(1);
#endif
	 
    rtl_rx_slot = (global_slot) % 32;
    current_global_slot = global_slot-1;
    //printf( "Got sync token %d\r\n",tmp_token ); 
    //printf( "sync slot %u\r\n",global_slot); 
    // Start new time token cycle
    // Just in case _rtl_abs_match was not called
    _rtl_time_token_status=RTL_TOKEN_NOT_SENT;
	
    _rtl_time_token=tmp_token;
    _rtl_sync_ok = 1;
    _rtl_clr_abs_wakeup (last_sync_slot);



    if(global_slot>=MAX_SLOTS || global_slot<2)
	last_sync_slot=MAX_SLOTS-2;
    else
    	last_sync_slot = global_slot-2;  // slot before real last sync slot
	// Must wake up slot before sync slot so that you can schedule
	// a wakeup 1 OS tick earlier for the actual sync slot.
  /* 
    if(last_sync_slot%32>(TDMA_FRAME_SLOTS-_rtl_contention_slots))
	{
	// Got sync in contention slot, so set next wakeup before 
	// that contention period
	printf( "last_sync %d %d->",last_sync_slot,last_sync_slot%32 );	
	last_sync_slot=last_sync_slot-(last_sync_slot%32-(TDMA_FRAME_SLOTS-_rtl_contention_slots))-2;
	printf( " %d\r\n",last_sync_slot );	
	} 
*/
     rtl_set_abs_wakeup (last_sync_slot, 0);
    _rtl_clear_sched_cache ();
#ifdef GPIO_RX_DEBUG
    nrk_gpio_clr(NRK_DEBUG_1);
#endif
   // DISABLE_GLOBAL_INT ();


   // ENABLE_GLOBAL_INT ();
    //tmp_token=_nrk_get_raw_timer();
//    printf( "tt=%d et=%d %d\r\n",tmp_token,elapsed_time,tick_to_slot_offset );	
    //printf( "%d %d\r\n",tick_to_slot_offset,prev_offset-tick_to_slot_offset );
    #ifdef RTL_DEBUG
    //rtl_debug_time_update(tick_to_slot_offset);
    #endif
    
    //nrk_spin_wait_us(3600);
    //printf( "sfd-start = %d %d\r\n",sfd_start_time,_nrk_high_speed_timer_get() );
     _nrk_os_timer_stop();
     _nrk_os_timer_reset();
     _nrk_os_timer_set(7);
    nrk_high_speed_timer_wait(0,SFD_TO_NEXT_SLOT_TIME);
     _nrk_os_timer_reset();
     _nrk_os_timer_start();
    _nrk_high_speed_timer_reset();
    
    //SFIOR |= BM(PSR0);              // reset prescaler
    //TCNT0 = 0; 
    slot_start_time=_nrk_high_speed_timer_get();
    //nrk_gpio_clr(NRK_DEBUG_1);
    //printf( "%u %u\r\n",sfd_start_time, SFD_TO_NEXT_SLOT_TIME );
    //_nrk_set_next_wakeup(6);
    //nrk_wait_until_next_period();
    //_nrk_prev_timer_val=elapsed_time;
    //nrk_cur_task_TCB->suspend_flag=1;
    //_nrk_set_next_wakeup(elapsed_time);
    //_nrk_wait_for_scheduler();
   //printf( "sync %d\r\n",_rtl_sync_ok ); 
   //nrk_clr_led(2);
   //printf( "%d %d %d %d\r\n",tmp_token,tdma_start_tick,elapsed_time,tmp_offset);
    return _rtl_sync_ok;
}

/**
 * rtl_check_tx_status()
 *
 * This function allows an upper layer to check if a TX packet
 * has been sent.  For instance, you would first send a packet with
 * rtl_tx_packet(&myTX, 8 );
 * and then you could sleep or do more work, and eventually check
 * done=rtl_check_tx_status(8);
 * to see if the packet had been sent by the interrupt routine.
 *
 * Returns: 1 if the packet was sent, 0 otherwise
 */
int8_t rtl_tx_pkt_check(uint8_t slot)
{
    if (slot == RTL_CONTENTION) {
        return _rtl_contention_pending;
    }
    if ((rtl_tx_data_ready & ((uint32_t) 1 << slot)) != 0)
        return 1;
    return 0;
}

/**
 * rtl_check_rx_status()
 *
 * This function returns if there is a packet in the link layer
 * rx buffer.  Once a packet has been received, it should be quickly
 * processed or moved and then rtl_release_rx_packet() should be called. 
 * rtl_release_rx_packet() then resets the value of rtl_check_rx_status()
 *
 * Returns: 1 if a new packet was received, 0 otherwise
 */
int8_t rtl_rx_pkt_check()
{
    return rtl_rx_data_ready;
}

void rtl_set_channel (uint8_t chan)
{
    rtl_param.channel = chan;
    rf_set_channel (chan);
}

/**
 * rtl_release_rx_packet()
 *
 * This function releases the link layer's hold on the rx buffer.
 * This must be called after a packet is received before a new
 * packet can be buffered!  This should ideally be done by the
 * network layer.
 */
void rtl_rx_pkt_release()
{
    rtl_rx_data_ready = 0;
}

/**
 * _rtl_tx()
 *
 * This function is the low level TX function.
 * It is only called from the timer interrupt and fetches any
 * packets that were set for a particular slot by rtl_tx_packet().
 *
 * Arguments: slot is the active slot set by the interrupt timer.
 */
void _rtl_tx (uint8_t slot)
{
	nrk_gpio_set(DEBUG_0);
	int8_t explicit_tsync;    
  
	if (rtl_sync_status () == 0)
     return;                 // don't tx if you aren't sure you are in sync

	if (tx_callback != NULL)
      tx_callback (slot);
			
  
	// Copy the element from the smaller vector of TX packets
  // to the main TX packet
  rtl_rfTxInfo.pPayload=rtl_tx_info[slot].pPayload;
  rtl_rfTxInfo.length=rtl_tx_info[slot].length;
	rtl_rfTxInfo.ackRequest = 0;

  rtl_rfTxInfo.pPayload[GLOBAL_SLOT] = (global_slot >> 8);
  rtl_rfTxInfo.pPayload[GLOBAL_SLOT + 1] = (global_slot & 0xFF);
  // or in so that you don't kill 
  //rtl_rfTxInfo.pPayload[TIME_SYNC_TOKEN]|= _rtl_time_token;  

  // This clears the explicit sync bit
  rtl_rfTxInfo.pPayload[TIME_SYNC_TOKEN]= _rtl_time_token; 
  explicit_tsync=0;
  // If it is an empty packet set explicit sync bit
  if(rtl_rfTxInfo.length==PKT_DATA_START )
	{
		explicit_tsync=1;
    rtl_rfTxInfo.pPayload[TIME_SYNC_TOKEN]|= 0x80;
	}
	
	 
									// MSB (explicit time slot flag)

#ifdef GPIO_TX_DEBUG
    nrk_gpio_set (NRK_DEBUG_1);
#endif
#ifdef LED_DEBUG_RTL
    nrk_led_toggle(RED_LED);
#endif
    //rf_tx_packet (rtl_rfTxInfo[slot]);
	
		//rf_tx_packet(&rtl_rfTxInfo);
    rf_tx_tdma_packet (&rtl_rfTxInfo , slot_start_time, rtl_param.tx_guard_time);
    rtl_tx_data_ready &= ~((uint32_t) 1 << slot);       // clear the flag
    if (slot >= (TDMA_FRAME_SLOTS - _rtl_contention_slots))
        _rtl_contention_pending = 0;
    // clear time sync token so that explicit time slot flag is cleared 
    //rtl_rfTxInfo.pPayload[TIME_SYNC_TOKEN]=0;  
    if(explicit_tsync==0)
    	nrk_event_signal (rtl_tx_done_signal);

#ifdef LED_DEBUG
    nrk_led_clr (0);
#endif
#ifdef GPIO_TX_DEBUG
    nrk_gpio_clr (NRK_DEBUG_1);
#endif

	nrk_gpio_clr(DEBUG_0);
}	

void rtl_set_slot_callback (void (*fp)(uint16_t))
{
    slot_callback = fp;
}

void rtl_set_rx_callback (void (*fp)(uint8_t))
{
   rx_callback = fp;
}

void rtl_set_tx_callback (void (*fp)(uint8_t))
{
   tx_callback = fp;
}

void rtl_set_abs_callback (void (*fp)(uint16_t))
{
   abs_callback = fp;
}

void rtl_set_tx_power (uint8_t pwr)
{
    rf_tx_power (pwr);
}

void rtl_set_contention (uint8_t slots, uint8_t rate)
{
    uint8_t i;

    _rtl_contention_slots = slots;
    if (slots != 0) {
        for (i = 31; i > (31 - slots); i--) {
            //printf( "Adding Contention slot %d %d\n",i, rate );   
            rtl_set_schedule (RTL_RX, i, rate);
            rtl_set_schedule (RTL_TX, i, rate);
        }
    }
}

int8_t rtl_rx_pkt_set_buffer(char *buf, uint8_t size)
{

    if(size==0 || buf==NULL) return NRK_ERROR;
    rtl_rfRxInfo.pPayload = buf;
    rtl_rfRxInfo.max_length = size;

return NRK_OK;
}

/**
 * rtl_sync_status()
 *
 * Return: 1 if sync is good, 0 if sync has failed
 *
 * This function returns if the synchronization is good.  _rtl_sync_ok
 * is set inside the interrupt based upon the time between the last cycle
 * and the sync pulse. 
 */
uint8_t rtl_sync_status ()
{
    return _rtl_sync_ok;
}

/**
 * rtl_check_abs_tx_status()
 *
 * This function allows an upper layer to check if the abs TX packet
 * has been sent.  This is only for the single absolute packet. 
 *
 * Returns: 1 if the packet was sent, 0 otherwise
 */
uint8_t rtl_check_abs_tx_status ()
{
    return rtl_abs_tx_ready;
}

/* shouldn't change */
/**
 * rtl_tx_abs_packet()
 *
 * This function sends a single packet at an arbitrary slot
 * over the entire TDMA frame. 
 *
 * Arguments: RF_TX_INFO *tx is a pointer to a transmit structure, this structure
 *            must have a valid pPayload pointer to the real packet.
 *            uint16_t abs_slot is the number of the absolute slot that will tx.
 * 
 * Return: returns 1 upon success, and 0 on failure due to overlaping TX slot
 *
 */
int8_t rtl_tx_abs_pkt (char *buf, uint8_t len, uint16_t abs_slot)
{
    uint8_t slot;
    uint32_t slot_mask;

    // Make sure this isn't already a TX slot
    slot = abs_slot % 32;
    slot_mask = ((uint32_t) 1) << slot;
    if (slot_mask & rtl_tdma_tx_mask)
        return 0;

    rtl_tx_info[TDMA_FRAME_SLOTS+1].pPayload= buf; 
    rtl_tx_info[TDMA_FRAME_SLOTS+1].length= len; 
    //rtl_rfTxInfo[TDMA_FRAME_SLOTS + 1] = tx;    // last element for abs slot 
    rtl_abs_tx_slot = abs_slot;
    rtl_set_abs_wakeup (abs_slot, 0);   // schedule once, but don't repeat
    rtl_abs_tx_ready = 1;
    return 1;
}

/* _rtl_rx()
 *
 * This is the low level RX packet function.  It will read in
 * a packet and buffer it in the link layer's single RX buffer.
 * This buffer can be checked with rtl_check_rx_status() and 
 * released with rtl_release_rx_packet().  If the buffer has not
 * been released and a new packet arrives, the packet will be lost.
 * This function is only called from the timer interrupt routine.
 *
 * Arguments: slot is the current slot that is actively in RX mode.
 */
void _rtl_rx (uint8_t slot)
{
	
	int8_t n;
	uint8_t cnt;
	uint16_t timeout;
	uint16_t tmp;
	
	
	rf_rx_on();
	
	cnt = 0;
	timeout = _nrk_os_timer_get();
	timeout += 15;
	while((n = rf_rx_check_fifop()) == 0) {
		 
		 if (_nrk_os_timer_get() > timeout) {
							if ((rtl_node_mode != RTL_COORDINATOR) && (global_slot == 0))
								_rtl_sync_ok = 0;						
            	rf_rx_off ();
							return;
		 }
	}
	
	timeout = _nrk_os_timer_get();
	timeout += 10;
	while((n = rf_rx_packet()) == 0) {
		if (_nrk_os_timer_get() > timeout) {
			rf_rx_off();
			return;
		}
	}
	//printf("failure3=%d\r\n",rx_failure_cnt);
	rf_rx_off();
  if (n == 1) 
		{
			uint8_t explicit_sync_pkt;
        // CRC and checksum passed
				// FIXME: DEBUG add back
				//rtl_debug_rx_pkt(1);
      rtl_rx_data_ready = 1;
      rtl_rx_slot = slot;
      tmp = rtl_rfRxInfo.pPayload[GLOBAL_SLOT];
      tmp <<= 8;
      tmp |= rtl_rfRxInfo.pPayload[GLOBAL_SLOT + 1];
			if(tmp!=global_slot)
			{
			// XXX HUGE HACK!
			// This shouldn't happen, but it does.  This should
			// be fixed soon.
			//printf( "mismatch coord: %d %d\r\n",global_slot,tmp );			
			global_slot=tmp;
			}	
      //printf ("my slot = %d  rx slot = %d\r\n", global_slot, tmp);
	    if (rx_callback != NULL)
          rx_callback (slot);
      // check if packet is an explicit time sync packet
			if((rtl_rfRxInfo.pPayload[TIME_SYNC_TOKEN]&0x80)==0)
			{
				// if we got a good packet, send the signal to
				// the application.  Shouldn't need to check rx
				// mask here since this should only get called by real
				// rx slot.
				nrk_event_signal (rtl_rx_pkt_signal);
			}
			// If it is an explicit time sync packet, then release it
			// so it doesn't block a buffer...
			else 
			{ 
				//nrk_kprintf( PSTR("got explicit sync\r\n") );
				rtl_rx_pkt_release(); 
			}
		} // else printf( "Error = %d\r\n",(int8_t)n );           
    #ifdef RTL_DEBUG
    else  rtl_debug_dropped_pkt();
    #endif                
    // else printf( "CRC Failed!\r" );

#ifdef LED_DEBUG
    nrk_led_clr (1);
#endif
#ifdef GPIO_RX_DEBUG
	nrk_gpio_clr(NRK_DEBUG_1);
#endif
}
/* checks to see if rtl is ready or not */
int8_t rtl_ready ()
{
    if (_rtl_ready ==  1)
        return NRK_OK;
    else
        return NRK_ERROR;
}

/* there is a change here
* returns a character pointer as opposed to a uint8_t
*/
char* rtl_rx_pkt_get (uint8_t *len, int8_t *rssi,uint8_t *slot)
{
if(rtl_rx_pkt_check()==0)
	{
	*len=0;
	*rssi=0;
	*slot=0;
	return NULL;
	}
  *len=rtl_rfRxInfo.length;
  *rssi=rtl_rfRxInfo.rssi;
  *slot=rtl_rx_slot;

return rtl_rfRxInfo.pPayload;
}

	/**
	 * rtl_init()
	 *
	 * This function sets up the low level link layer parameters.
	 * This starts the main timer routine that will then automatically
	 * trigger whenever a packet might be sent or received.
	 * This should be called before ANY scheduling information is set
	 * since it will clear some default values.
	 *
	 */
	void rtl_init (rtl_node_mode_t mode)
	{
		uint8_t i;
		uint16_t a,b;

	 rtl_rx_pkt_signal=nrk_signal_create();
		 if(rtl_rx_pkt_signal==NRK_ERROR)
		{
		printf("RT-Link ERROR: creating rx signal failed\r\n");
		nrk_kernel_error_add(NRK_SIGNAL_CREATE_ERROR,nrk_cur_task_TCB->task_ID);
		//return NRK_ERROR;
		}
	  rtl_tx_done_signal=nrk_signal_create();
		 if(rtl_tx_done_signal==NRK_ERROR)
		{
		printf("RT-Link ERROR: creating tx signal failed\r\n");
		nrk_kernel_error_add(NRK_SIGNAL_CREATE_ERROR,nrk_cur_task_TCB->task_ID);
		//return NRK_ERROR;
		}

		// No buffer to start with
		rtl_rfRxInfo.pPayload = NULL;
		rtl_rfRxInfo.max_length = 0;

		// FIXME: Move into kernel later...
		// Need to decide if this should always be running or not...
		_nrk_high_speed_timer_start();  
		//printf( "slot to sfd = %d\r\n",SLOT_TO_SFD);
		//printf( "guard low = %d\r\n",GUARD_LOW);
		//printf( "guard high = %d\r\n",GUARD_HIGH);
		#ifdef RTL_DEBUG
		rtl_debug_init();
		#endif
		// clear everything out
		_rtl_time_token_status=RTL_TOKEN_NOT_SENT;
		_rtl_time_token=0;
		last_sync_slot=0;
		global_cycle = 0;
		global_slot = 1025;
		_rtl_sync_ok = 0;
		rtl_node_mode = mode;
		rtl_rx_data_ready = 0;
		rtl_tx_data_ready = 0;
		_rtl_contention_slots = 0;
		_rtl_contention_pending = 0;


		rx_callback = NULL;
		tx_callback = NULL;
		abs_callback = NULL;
		slot_callback = NULL;
		cycle_callback = NULL;

		rtl_param.mobile_sync_timeout = 100;
		rtl_param.rx_timeout = 8000;   // 8000 *.125us = 1ms

		// for nano-RK this value is +1 due to timing
		// 1 -> 2ms
		// 2 -> 3ms
		// 3 -> 4ms 
		//rtl_param.rx_timeout = 5;  // each tick is about 120 uS (not true in nano-rk)
		//rtl_param.tx_guard_time = 120;
		//rtl_param.tx_guard_time = 300;
		rtl_param.tx_guard_time = TX_GUARD_TIME;  // 144uS  410-266 // Putting 0.5 ms as the guard time /* Tharun */
		//rtl_param.tx_guard_time = 1600;  // 144uS  410-266
		rtl_param.channel = RADIO_CHANNEL;
		rtl_param.mac_addr = 0x0000;

	for (i = 0; i < 16; i++) {
			rtl_sched[i] = 0;
			rtl_sched[i] = 0;
		}
		rtl_tdma_rx_mask = 0;
		rtl_tdma_tx_mask = 0;
		rtl_rx_data_ready = 0;
		rtl_tx_data_ready = 0;
		//rtl_abs_tx_slot=MAX_SLOTS+1;
	  rtl_abs_tx_slot=-1;
		_rtl_clr_abs_all_wakeup ();

		// Setup the AM sync ports etc
		//AM_INIT ();   // FIXME
		// Setup the cc2420 chip
		//set up the mrf24j20 chip in our case
		rf_init (&rtl_rfRxInfo, rtl_param.channel, 0xffff, 0);
		//rf_rx_on();
		rf_set_cca_thresh(-45);
		rf_addr_decode_disable();
		rf_auto_ack_disable();		
}
	
void myFunction(uint16_t slot)
{
				if (LPC_GPIO0->FIOPIN & 1 << 10)
					LPC_GPIO0->FIOCLR |= 1 << 10;
				else
					LPC_GPIO0->FIOSET |= 1 << 10;		
}

void (*fcnPtr)(uint16_t) = myFunction;	

	void rtl_start ()
{

    // Setup one absolute wakeup for the AM sync warmup repeated
    //if (rtl_node_mode != RTL_MOBILE || rtl_node_mode != RTL_COORDINATOR)
        rtl_set_abs_wakeup (_RTL_SYNC_WAKEUP_SLOT, 1);
		rtl_set_abs_callback(fcnPtr);
    
    // If you are the coordinator then set to transmit at rate 4
    // on every 0 slot if not already using slot 0. 
    if(rtl_node_mode==RTL_COORDINATOR)
	{
	// Check if slot 0 is set to be used, and make sure it is
	// at least rate 4 or faster.
	if((rtl_tdma_tx_mask & 0x1)==0 || (rtl_sched[0]&0x0F)>4)
    		{
			rtl_set_schedule( RTL_TX, 0, 4 );
		}
	}

    //_nrk_reset_os_timer ();
    _rtl_clear_sched_cache ();
    _rtl_ready = 2;

}
	
void rtl_nw_task ()
{
    uint8_t slot;
    uint32_t slot_mask;
    uint8_t i, k, val, j, frames;
    int8_t n;
    uint8_t timeout;
    uint16_t next_slot_offset, tmp; 
    uint8_t blink,skip_rxtx;
		uint16_t sync_counter = 0;

    blink = 0;



    _rtl_ready = 0;
   
    do {
        nrk_wait_until_next_period ();
    }while (_rtl_ready == 0);
    _rtl_ready = 1;
    

    skip_rxtx=0;
    last_slot = 0;
    nrk_time_get (&last_slot_time);
    while (1) {
		  
			//printf("%d \r\n", rtl_abs_tx_slot);
			
	// Need to calculate offset into TDMA slot starting now	
	//_nrk_stop_high_speed_timer();  
	//_nrk_reset_high_speed_timer();  
	//_nrk_start_high_speed_timer();  	
        slot_start_time=_nrk_high_speed_timer_get();
				//printf("sst %d \r\n", slot_start_time);
	nrk_time_get (&last_slot_time);
        last_slot = global_slot;
        if (last_slot == 1025)
            last_slot = 0;
	
	current_global_slot = global_slot;
        if (global_slot >= MAX_SLOTS) {						
            global_slot = 0;						
            global_cycle++;
            if (cycle_callback != NULL)
                cycle_callback (global_cycle);
            _rtl_clear_sched_cache ();						
						if (rtl_node_mode == RTL_COORDINATOR) {
                _rtl_sync_ok = 1;
            }
        }


        if (slot_callback != NULL)						
            slot_callback (global_slot);

	
 	if (rtl_node_mode == RTL_MOBILE ){		
		if ( (global_slot==last_sync_slot+1) || (!_rtl_sync_ok) )		
				{				
					
					
            // Wait for packet
            // Sync on packet
            // reset timer at the start of next slot
            // set global_slot to slot of rx packet+1
            //while (rtl_rx_data_ready != 0)
            	//nrk_wait_until_next_period ();
            while (_rtl_rx_sync () == 0);
	   	//_nrk_stop_high_speed_timer();  
		//_nrk_reset_high_speed_timer();  
		//_nrk_start_high_speed_timer();  
	    }
   }
	
		sync_counter++;
	// This call is required to clear abs schedules
     
			if (_rtl_match_abs_wakeup (global_slot) == 1) {				 
            //printf( "Application Timer!\n" );
			
	    if (abs_callback != NULL)
                abs_callback (global_slot);
        } 

        next_slot_offset = rtl_get_slots_until_next_wakeup (global_slot);
				
// Check for earlier non-slot wakeups...
        tmp = _rtl_get_next_abs_wakeup (global_slot);
        if (tmp != 0) {
						
            if (next_slot_offset == 0)
                next_slot_offset = tmp;
            if (tmp < next_slot_offset)
                next_slot_offset = tmp;
        }
				
// Set next wakeup
        if (next_slot_offset == 0) {
            next_slot_offset = MAX_SLOTS - global_slot;       //Wake at end of TDMA cycle 
        }
#ifdef TXT_DEBUG
        printf ("%d,%d,%d\r\n", global_slot, global_slot % 32,
                next_slot_offset);
#endif
        // FIXME _rtl_set_next_wakeup (next_slot_offset);
				slot = global_slot % 32;
        slot_mask = ((uint32_t) 1) << slot;

			
       // Coordinator always TX on slot 0
       if (rtl_node_mode == RTL_COORDINATOR && global_slot==0) 
		{ 
		_rtl_time_token++;  // Coordinator increases sync token
		if(_rtl_time_token>127) _rtl_time_token=0;
		}
       if (rtl_node_mode == RTL_COORDINATOR && slot==0 )  
		_rtl_time_token_status=RTL_TOKEN_NOT_SENT; 

		
	// If no data needs to be sent but time token needs to be passed, generate
	// explicit time sync packet.
       if (_rtl_time_token_status==RTL_TOKEN_NOT_SENT)
		{
		//printf( "tns %d %d\r\n",slot,_rtl_contention_pending );
		if(slot_mask & rtl_tdma_tx_mask )  // Yes it is your TX slot
			{
			if((slot_mask & rtl_tx_data_ready) == 0)  // No app data to send
				{
				// generate explicit packet
				// When the link layer receives an explicit sync, it does not block buffers
				// and does not signal applications
				rtl_tsync_tx.pPayload=rtl_tsync_buf;
				// set explicit time sync flag 
				rtl_tsync_buf[TIME_SYNC_TOKEN]|=0x80;  
				rtl_tsync_tx.length=PKT_DATA_START;
				if(slot>(TDMA_FRAME_SLOTS-_rtl_contention_slots))
				{
					if(_rtl_contention_pending==0) 
					{
						//printf( "cs" );
						rtl_tx_pkt (rtl_tsync_buf,rtl_tsync_tx.length, RTL_CONTENTION);
					} 
					//else printf( "already pending\r\n" );
				} else
					{
			        	rtl_tx_pkt (rtl_tsync_buf,rtl_tsync_tx.length, slot);
					//printf( "ss" );
					}
				}
			//else printf( "piggy back\r\n" );
			_rtl_time_token_status=RTL_TOKEN_SENT;
			}
			//else printf( "no slot\r\n" );
		
		}

	//printf("gs %d \r\n", global_slot);
	
	
	if(global_slot!=last_sync_slot)
	{
		//nrk_high_speed_timer_wait(860, 2);
		//printf("%d %d \r\n", global_slot, _nrk_high_speed_timer_get());
	// if TX slot mask and ready flag, send a packet
        if (slot_mask & rtl_tx_data_ready & rtl_tdma_tx_mask)
	    {
				
					//printf("tx %d \r\n", _nrk_high_speed_timer_get());
						//nrk_high_speed_timer_wait(866,3);
            _rtl_tx (slot);
						if ((sync_counter > 10) && (rtl_node_mode != RTL_COORDINATOR)) {
							//printf("o \r\n");
							_rtl_sync_ok = 0;
							sync_counter = 0;
						}
	    //printf( "sent %d\r\n",slot );
	    }
	// if RX slot mask and RX buffer free, try to receive a packet
        else if ((slot_mask & rtl_tdma_rx_mask) && (rtl_rx_data_ready == 0))
            { 
							//printf("%d %d \r\n", global_slot, _nrk_high_speed_timer_get());
							//printf("rx %d \r\n", _nrk_high_speed_timer_get());
	      _rtl_rx (slot);
	    } 
        else if (global_slot == rtl_abs_tx_slot) {
					printf("gs %d \r\n", global_slot);
            // Make sure this isn't overlapping a normal TX slot!
					_rtl_tx (TDMA_FRAME_SLOTS + 1);
          rtl_abs_tx_ready = 0;
        }				
	}
			

//    printf( "%d\r\n",global_slot);
// Set correct slot for next wakeup
//	printf( "s %d nw %d ",global_slot,next_slot_offset );
	
       
	if(global_slot==last_sync_slot && rtl_node_mode!=RTL_COORDINATOR)
	{
        global_slot++;
        nrk_wait_until_ticks(5);
	}
	else
	{
        global_slot += next_slot_offset;
				nrk_wait_until_next_n_periods (next_slot_offset);
        //nrk_clr_led (1);
	#ifdef LED_SLOT_DEBUG
	nrk_led_clr(0);
	#endif

	#ifdef GPIO_SLOT_DEBUG
	nrk_gpio_clr(NRK_DEBUG_0);
	#endif
        
	#ifdef LED_SLOT_DEBUG
	nrk_led_set(0);
	#endif
	#ifdef GPIO_SLOT_DEBUG
	nrk_gpio_set(NRK_DEBUG_0);
	#endif
	}
	
	
        //nrk_set_led (1);
        // Set last_slot_time to the time of the start of the slot
        }

}



void rtl_task_config ()
{
    rtl_task.task = rtl_nw_task;
    nrk_task_set_stk( &rtl_task, rtl_task_stack, RT_LINK_STACK_SIZE);
    rtl_task.prio = 20;
    rtl_task.FirstActivation = TRUE;
    rtl_task.Type = BASIC_TASK;
    rtl_task.SchType = PREEMPTIVE;
    rtl_task.period.secs = 0;
    rtl_task.period.nano_secs = RTL_NRK_TICKS_PER_SLOT * NANOS_PER_TICK;
    //rtl_task.period.nano_secs = 100*NANOS_PER_MS;
    rtl_task.cpu_reserve.secs = 0;      // Way larger than period
    rtl_task.cpu_reserve.nano_secs = 0;
    rtl_task.offset.secs = 0;
    rtl_task.offset.nano_secs = 0;
    nrk_activate_task (&rtl_task);
}

