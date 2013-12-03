/*
 * encoder.c - encoder interface
 * This file is part of the TinyG project
 *
 * Copyright (c) 2010 - 2013 Alden S. Hart, Jr.
 *
 * This file ("the software") is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 as published by the
 * Free Software Foundation. You should have received a copy of the GNU General Public
 * License, version 2 along with the software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, you may use this file as part of a software library without
 * restriction. Specifically, if other files instantiate templates or use macros or
 * inline functions from this file, or you compile this file and link it with  other
 * files to produce an executable, this file does not by itself cause the resulting
 * executable to be covered by the GNU General Public License. This exception does not
 * however invalidate any other reasons why the executable file might be covered by the
 * GNU General Public License.
 *
 * THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 * WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* 	This module provides the low-level stepper drivers and some related functions.
 *	See stepper.h for a detailed explanation of this module.
 */

#include "tinyg.h"
#include "config.h"
#include "stepper.h"
#include "encoder.h"
#include "canonical_machine.h"
#include "hardware.h"

/**** Allocate Structures ****/

enEncoders_t en;

/************************************************************************************
 **** CODE **************************************************************************
 ************************************************************************************/

/* 
 * encoder_init() - initialize encoders 
 */

void encoder_init()
{
	memset(&en, 0, sizeof(en));		// clear all values, pointers and status
	en.magic_end = MAGICNUM;
	en.magic_start = MAGICNUM;
	en_reset_encoders();
}

/*
 * en_assertions() - test assertions, return error code if violation exists
 */

stat_t en_assertions()
{
	if (en.magic_end   != MAGICNUM) return (STAT_STEPPER_ASSERTION_FAILURE);
	if (en.magic_start != MAGICNUM) return (STAT_STEPPER_ASSERTION_FAILURE);
	return (STAT_OK);
}

/* 
 * en_reset_encoder() 			- initialize encoder values and position
 * en_update_target()			- provide a new target
 * en_compute_position_error()	- process position, error term and stage next position measurement
 *
 *	Usage:
 *	
 *	This trio of routines works together to generate the error term.
 * 
 *	Reset the encoders after homing and/or at the start of a cycle. This establishes the
 *	"step grid" relative to the current machine position.
 *
 *	Update the target position when st_prep_line() sees a new target. This saves the
 *	target values so they can be used later for generating the error term.
 *
 *	Compute the error term in the segment interval following the last segment of a move.
 *	This is signalled by the steppers setting the "last_segment" flag. This function does
 *	not need to be called for every move end, but if it is called it can only be called once
 *	or the target staging will be messed up. Call this function from st_prep_line();
 */

void en_reset_encoders()
{
	for (uint8_t i=0; i<MOTORS; i++) {
		en.en[i].target_steps = 0;
		en.en[i].target_steps_next = 0;
		en.en[i].position_steps = cm.gmx.position[i] * st_cfg.mot[i].steps_per_unit;
		en.en[i].position_steps_float = en.en[i].position_steps;	// initial approximation
	}
}

void en_update_target(const float target[])
{
	for (uint8_t i=0; i<MOTORS; i++) {
		en.en[i].target_steps_next = target[i] * st_cfg.mot[i].steps_per_unit;	// convert target to steps
	}
}

void en_compute_position_error()
{
	for (uint8_t i=0; i<MOTORS; i++) {
		en.en[i].error_steps = en.en[i].position_steps - en.en[i].target_steps;
		en.en[i].error_distance = (float)en.en[i].error_steps * st_cfg.mot[i].units_per_step;
		en.en[i].target_steps = en.en[i].target_steps_next;	// transfer staged target to working target
	}
}

/*
 * en_update_incoming_steps() - add new incoming steps. Handy diagnostic. It's not used for anything else.
 */

void en_update_incoming_steps(const float steps[])
{
	for (uint8_t i=0; i<MOTORS; i++) {
		en.en[i].position_steps_float += steps[i];
	}
}

/*
 * en_print_encoder()
 * en_print_encoders()
 */

void en_print_encoder(const uint8_t motor)
{
//	en_compute_position_error();

	printf("{\"en%d\":{\"steps_flt\":%0.3f,\"pos_st\":%li,\"tgt_st\":%li,\"err_st\":%li,\"err_d\":%0.5f}}\n",
		motor+1,
		(double)en.en[motor].position_steps_float,
		en.en[motor].position_steps, 
		en.en[motor].target_steps,
		en.en[motor].error_steps,
		(double)en.en[motor].error_distance);
}

void en_print_encoders()
{
//	en_compute_position_error();

	for (uint8_t i=0; i<MOTORS; i++) {
		printf("{\"en%d\":{\"steps_flt\":%0.3f,\"pos_st\":%li,\"tgt_st\":%li,\"err_st\":%li,\"err_d\":%0.5f}}\n",
			i+1,
			(double)en.en[i].position_steps_float,
			en.en[i].position_steps, 
			en.en[i].target_steps,
			en.en[i].error_steps,
			(double)en.en[i].error_distance);
	}
}

/***********************************************************************************
 * CONFIGURATION AND INTERFACE FUNCTIONS
 * Functions to get and set variables from the cfgArray table
 ***********************************************************************************/

/***********************************************************************************
 * TEXT MODE SUPPORT
 * Functions to print variables from the cfgArray table
 ***********************************************************************************/

#ifdef __TEXT_MODE

#endif // __TEXT_MODE

