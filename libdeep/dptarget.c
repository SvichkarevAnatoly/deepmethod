/***************************************************************************
 *            dptarget.c
 *
 *  Fri Mar 23 15:59:35 2012
 *  Copyright  2012  Konstantin Kozlov
 *  <kozlov@spbcas.ru>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <glib.h>
#include "dptarget.h"

DpTargetFunc*dp_target_func_new(int index, double weight, double rank, char*sname)
{
	DpTargetFunc*htargetfunc;
	htargetfunc = (DpTargetFunc*)malloc(sizeof(DpTargetFunc));
	htargetfunc->index = 0;
	htargetfunc->weight = weight;
	htargetfunc->rank = rank;
	htargetfunc->name = g_strdup (sname);
	return htargetfunc;
}

DpTarget*dp_target_new()
{
	DpTarget*htarget;
	htarget = (DpTarget*)malloc(sizeof(DpTarget));
	htarget->size = 0;
	htarget->target = NULL;
	htarget->penalty = NULL;
	htarget->precond_size = 0;
	htarget->precond = NULL;
	htarget->prime = NULL;
	htarget->debug = 0;
	htarget->ignore_cost = 0;
	htarget->use_crdist = 0;
	return htarget;
}

void dp_target_add_func (DpTarget*htarget, int index, double weight, double rank, char *sname)
{
	DpTargetFunc*htargetfunc;
	htargetfunc = dp_target_func_new(index, weight, rank, sname);
	if ( index == 0 ) {
		htarget->target = htargetfunc;
	} else if ( index > 0 ) {
		htarget->penalty = (DpTargetFunc**)realloc(htarget->penalty, ( htarget->size + 1 ) * sizeof (DpTargetFunc*) );
		htarget->penalty[ htarget->size ] = htargetfunc;
		htarget->size++;
	} else if ( index < 0 ) {
		htarget->precond = (DpTargetFunc**)realloc(htarget->precond, ( htarget->precond_size + 1 ) * sizeof (DpTargetFunc*) );
		htarget->precond[ htarget->precond_size ] = htargetfunc;
		htarget->precond_size++;
	}
}

void dp_target_insert_prime_func (DpTarget*htarget, int index, double weight, double rank, char *sname)
{
	DpTargetFunc*hprimefunc;
	hprimefunc = dp_target_func_new(index, weight, rank, sname);
	htarget->prime = hprimefunc;
}

int dp_target_eval (DpTarget*htarget, double*x, int*invalid, double*cost, double*penalty, double*precond, gpointer user_data, int index, double cost0)
{
	int max_value_flag = 0, i;
	double value, f, retval, max_value = G_MAXDOUBLE, target_value;
	for ( i = 0; i < htarget->precond_size; i++ ) {
		f = htarget->precond[i]->f(user_data, x);
		if ( f < max_value ) {
			f *= htarget->precond[i]->weight;
			precond[i] = f;
		} else {
			max_value_flag = 1;
			break;
		}
	}
	if ( max_value_flag == 1 ) {
		(*cost) = max_value;
		(*invalid) = 1;
		return 1;
	}
	retval = max_value;
	f = htarget->target->f(user_data, x);
	target_value = f;
	if ( f < max_value ) {
		f *= htarget->target->weight;
		retval = f;
		value = 0;
		for ( i = 0; i < htarget->size; i++ ) {
			f = htarget->penalty[i]->f(user_data, x);
			if ( f < max_value ) {
				penalty[i] = f;
				f *= htarget->penalty[i]->weight;
				value += f;
			} else {
				max_value_flag = 1;
				penalty[i] = max_value;
			}
		}
		retval += value;
	} else {
		max_value_flag = 1;
	}
	(*cost) = retval;
	(*invalid) = max_value_flag;
	if ( htarget->debug == 1 ) {
		fprintf(stdout, "target_value=%13.9f weight=%13.9f\n", target_value, htarget->target->weight);
		fprintf(stdout, "N precond weight:\n");
		for ( i = 0; i < htarget->precond_size; i++ ) {
			fprintf(stdout, "%d %13.9f %13.9f\n", i, precond[i], htarget->precond[i]->weight);
		}
		fprintf(stdout, "N penalty weight:\n");
		for ( i = 0; i < htarget->size; i++ ) {
			fprintf(stdout, "%d %13.9f %13.9f\n", i, penalty[i], htarget->penalty[i]->weight);
		}
		fprintf(stdout, "max_value_flag=%d value=%13.9f retval=%13.9f\n", max_value_flag, value, retval);
	}
	return max_value_flag;
}

int dp_target_eval_precond (DpTarget*htarget, double*x, int*invalid, double*precond, gpointer user_data, int index, double cost0)
{
	int max_value_flag = 0, i;
	double value, f, retval, max_value = G_MAXDOUBLE;
	for ( i = 0; i < htarget->precond_size; i++ ) {
		f = htarget->precond[i]->f(user_data, x);
		if ( f < max_value ) {
			f *= htarget->precond[i]->weight;
			precond[i] = f;
		} else {
			max_value_flag = 1;
			break;
		}
	}
	(*invalid) = max_value_flag;
	return max_value_flag;
}

int dp_target_eval_prime (DpTarget*htarget, double*x, int*invalid, double*prime, gpointer user_data, int index, double cost0)
{
	int max_value_flag = 0;
	double value, f, retval, max_value = G_MAXDOUBLE;
	f = htarget->prime->f(user_data, x);
	if ( f < max_value ) {
		f *= htarget->prime->weight;
		prime[index] = f;
	} else {
		max_value_flag = 1;
	}
	(*invalid) = max_value_flag;
	return max_value_flag;
}

gpointer dp_target_eval_get_user_data(DpTarget*htarget)
{
	return htarget->copy_model(htarget->user_data);
}

void dp_target_eval_update_user_data(DpTarget*htarget, gpointer user_data, double*buffer, int index, double cost0)
{
	htarget->update_model(user_data, buffer, index, cost0);
}

void dp_target_shift_penalty_weights (DpTarget*htarget)
{
	int i;
	double weight;
	weight = htarget->penalty[ htarget->size - 1 ]->weight;
	for ( i = htarget->size - 1; i > 0; i-- ) {
		htarget->penalty[i]->weight = htarget->penalty[i - 1]->weight;
	}
	htarget->penalty[0]->weight = weight;
	if ( htarget->debug == 1 ) {
		fprintf(stdout, "dp_target_shift_penalty_weights: # rank weight\n");
		for ( i = 0; i < htarget->size; i++ ) {
			fprintf(stdout, "%d %13.9f %13.9f\n", i, htarget->penalty[i]->rank, htarget->penalty[i]->weight);
		}
	}
}

void dp_target_shift_penalty_ranks (DpTarget*htarget)
{
	int i;
	double rank;
	rank = htarget->penalty[ htarget->size - 1 ]->rank;
	for ( i = htarget->size - 1; i > 0; i-- ) {
		htarget->penalty[i]->rank = htarget->penalty[i - 1]->rank;
	}
	htarget->penalty[0]->rank = rank;
	if ( htarget->debug == 1 ) {
		fprintf(stdout, "dp_target_shift_penalty_ranks: # rank weight\n");
		for ( i = 0; i < htarget->size; i++ ) {
			fprintf(stdout, "%d %13.9f %13.9f\n", i, htarget->penalty[i]->rank, htarget->penalty[i]->weight);
		}
	}
}
