/***************************************************************************
 *            dpopt.c
 *
 *  Fri Mar 23 15:49:34 2012
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
#include <math.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "dploop.h"
#include "dpopt.h"
#include "dpdeep.h"
#include "dposda.h"

#ifdef GIO_STANDALONE_SOURCE
#define g_strcmp0(str1, str2) strcmp(str1, str2)
#endif

DpOpt *dp_opt_init(DpEvaluation*heval, DpTarget*htarget, int world_id, int world_count,char*filename, DpOptStopType stop_type, double criterion, int stop_count, int pareto_all, int precision)
{
	DpOpt *hopt;
	GList *funcs = NULL;
	GList *run_once_before = NULL;
	GList *run_once_after = NULL;
	hopt = (DpOpt*)malloc(sizeof(DpOpt));
	hopt->world_id = world_id;
	hopt->world_count = world_count;
	hopt->filename = g_strdup (filename);
	hopt->stop_type = stop_type;
	hopt->criterion = criterion;
	hopt->stop_count = stop_count;
	hopt->logname = g_strdup_printf( "%s.hopt_log_%d", hopt->filename, hopt->world_id);
	hopt->chkname = g_strdup_printf( "%s.hopt_chk_%d", hopt->filename, hopt->world_id);
	hopt->tstname = g_strdup_printf( "%s.hopt_time", hopt->filename);
	hopt->htarget = htarget;
	hopt->heval = heval;
	hopt->hloop = dp_loop_new(NULL, NULL, NULL);
	hopt->opt_type = H_OPT_NONE;
	hopt->method_info = NULL;
	hopt->pareto_all = pareto_all;
	hopt->precision = precision;
	hopt->debug = 0;
	return hopt;
}

void dp_opt_add_func(DpOpt *hopt, DpLoopFunc func, int tau_flag, DpOptType opt_type, int order, gpointer method_info)
{
	DpLoopRunFunc *run_func;
	DpLoop *hloop = hopt->hloop;
	run_func = dp_loop_run_func_new(tau_flag, func, (gpointer) hopt);
	if ( hopt->opt_type == H_OPT_NONE ) {
		hopt->opt_type = opt_type;
	}
	if ( hopt->method_info == NULL ) {
		hopt->method_info = method_info;
	}
	if ( order < 0 ) {
		dp_loop_add_func_before(hloop, run_func);
	} else if ( order == 0 ) {
		dp_loop_add_func(hloop, run_func);
	} else {
		dp_loop_add_func_after(hloop, run_func);
	}
}

void dp_opt_add_func_from_list(gchar**list, DpOpt *hopt, int tau_flag, DpOptType opt_type, int order, gpointer method_info)
{
	int i;
	for ( i = 0; list[i]; i++ ) {
		if (hopt->world_id == 0) {
			if ( !g_strcmp0(list[i], "writelog") ) {
				dp_opt_add_func(hopt, dp_write_log, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "writestate") ) {
					dp_opt_add_func(hopt, dp_write_state, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "readstate") ) {
				dp_opt_add_func(hopt, dp_read_state, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "writetst") ) {
				dp_opt_add_func(hopt, dp_write_tst, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "duplicate") ) {
				dp_opt_add_func(hopt, dp_opt_duplicate, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "substitute") ) {
				dp_opt_add_func(hopt, dp_opt_substitute, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "substold") ) {
				dp_opt_add_func(hopt, dp_opt_substold, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "substfailed") ) {
				dp_opt_add_func(hopt, dp_opt_substfailed, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "optpost") ) {
				dp_opt_add_func(hopt, dp_opt_post, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "optposteval") ) {
				dp_opt_add_func(hopt, dp_opt_post_evaluate, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "rotatetarget") ) {
				dp_opt_add_func(hopt, dp_rotate_target, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "initstop") ) {
				dp_opt_add_func(hopt, dp_opt_init_stop, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "writepareto") ) {
				dp_opt_add_func(hopt, dp_write_pareto, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "cr2cost") ) {
				dp_opt_add_func(hopt, dp_opt_cr2cost, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "evalpareto") ) {
				dp_opt_add_func(hopt, dp_opt_evaluate_pareto_front, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "selpareto") ) {
				dp_opt_add_func(hopt, dp_select_pareto_front, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "sortpareto") ) {
				dp_opt_add_func(hopt, dp_sort_pareto_front, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "permutepop") ) {
				dp_opt_add_func(hopt, dp_opt_permute, tau_flag, opt_type, order, method_info);
			}
		}
		if ( !g_strcmp0(list[i], "mpidistribute") ) {
				dp_opt_add_func(hopt, dp_opt_mpi_distribute, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "mpigather") ) {
			dp_opt_add_func(hopt, dp_opt_mpi_gather, tau_flag, opt_type, order, method_info);

		}
	}
}

void dp_opt_run(DpOpt *hopt)
{
	DpLoop *hloop = hopt->hloop;
	dp_loop_zero_counters(hopt->hloop);
	dp_loop_run (hloop);
}

void dp_opt_monitor(DpOpt *hopt, int monitor, GError**gerror)
{
	DpLoop *hloop;
	GList *funcs = NULL;
	GList *run_once_before = NULL;
	GList *run_once_after = NULL;
	DpLoopRunFunc *func;

	func = (DpLoopRunFunc *)malloc(sizeof(DpLoopRunFunc));
	func->tau_flag = 1;
	func->user_data = (void*)hopt;
	func->func = (DpLoopFunc)dp_read_log;
	funcs = g_list_append (funcs, (void*)func);
/*
	func = (DpLoopRunFunc *)malloc(sizeof(DpLoopRunFunc));
	func->tau_flag = 1;
	func->user_data = (void*)hopt;
	func->func = (DpLoopFunc)dp_opt_post;
	funcs = g_list_prepend (funcs, (void*)func);

	func = (DpLoopRunFunc *)malloc(sizeof(DpLoopRunFunc));
	func->tau_flag = 1;
	func->user_data = (void*)hopt;
	func->func = (DpLoopFunc)dp_opt_post_evaluate;
	funcs = g_list_prepend (funcs, (void*)func);
*/
	func = (DpLoopRunFunc *)malloc(sizeof(DpLoopRunFunc));
	func->tau_flag = 1;
	func->user_data = (void*)hopt;
	func->func = (DpLoopFunc)dp_write_tst;
	funcs = g_list_append (funcs, (void*)func);

	hloop = dp_loop_new(run_once_before, funcs, run_once_after);
	dp_loop_zero_counters(hloop);
	hopt->hloop = hloop;
	hopt->monitor = monitor;
	dp_loop_run (hloop);
}

DpLoopExitCode dp_opt_init_stop(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	DpOsdaInfo*hosdainfo;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			dp_deep_accept_step(hdeepinfo, &(hopt->cost));
			hopt->old_cost = hopt->cost;
			hopt->stop_counter = 0;
		break;
		case H_OPT_OSDA:
			hosdainfo = (DpOsdaInfo*)(hopt->method_info);
			dp_osda_accept_step(hosdainfo, &(hopt->cost));
			hopt->old_cost = hopt->cost;
			hopt->stop_counter = 0;
		break;
	}
	return ret_val;
}

DpLoopExitCode dp_opt_check_stop(DpLoop*hloop, gpointer user_data)
{
	DpOpt*hopt = (DpOpt*)user_data;
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	double delta;
	if ( hopt->stop_type != H_OPT_ABSOLUTE_TIME && hopt->stop_type != H_OPT_ABSOLUTE_ITER ) {
		switch( hopt->stop_type ) {
			case H_OPT_PROPORTIONAL_STOP:
				delta = ( hopt->old_cost - hopt->cost );
				if ( hopt->old_cost != 0 ) {
					delta /= hopt->old_cost;
				}
			break;
			case H_OPT_ABSOLUTE_STOP:
				delta = hopt->old_cost - hopt->cost;
			break;
			case H_OPT_ABSOLUTE_SCORE:
				delta = hopt->cost;
			break;
			default:
				hloop->exit_str = g_strdup_printf ( "dp_opt_checkstop: unknown %d", (int)(hopt->stop_type));
				return DP_LOOP_EXIT_ERROR;
		}
		if ( fabs(delta) < hopt->criterion ) {
			hopt->stop_counter++;
		} else {
			hopt->stop_counter = 0;
			hopt->old_cost = hopt->cost;
		}
		if ( hopt->stop_counter == hopt->stop_count) {
			hopt->stop_counter = 0;
			hopt->old_cost = hopt->cost;
			ret_val = DP_LOOP_EXIT_SUCCESS;
		}
	} else if ( hopt->stop_type == H_OPT_ABSOLUTE_TIME && hloop->w_time >= hopt->criterion ) {
		ret_val = DP_LOOP_EXIT_SUCCESS;
	} else if ( hopt->stop_type == H_OPT_ABSOLUTE_ITER && hloop->tau_counter >= hopt->criterion ) {
		ret_val = DP_LOOP_EXIT_SUCCESS;
	}
	return ret_val;
}

DpLoopExitCode dp_opt_deep(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo = (DpDeepInfo*)(hopt->method_info);
	double energy_start;
	energy_start = hopt->cost;
	dp_deep_step(hdeepinfo);
	dp_deep_accept_step(hdeepinfo, &(hopt->cost));
	return ret_val;
}

DpLoopExitCode dp_opt_deep_generate(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo = (DpDeepInfo*)(hopt->method_info);
	dp_deep_generate_step(hdeepinfo);
	hdeepinfo->selection_done = 0;
	return ret_val;
}

DpLoopExitCode dp_opt_deep_evaluate(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo = (DpDeepInfo*)(hopt->method_info);
	dp_deep_evaluate_step(hdeepinfo);
	return ret_val;
}

DpLoopExitCode dp_opt_deep_select(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo = (DpDeepInfo*)(hopt->method_info);
	if ( hdeepinfo->selection_done == 1 ) {
        return ret_val;
	}
	dp_deep_select_step(hdeepinfo);
	dp_deep_accept_step(hdeepinfo, &(hopt->cost));
	hdeepinfo->selection_done = 1;
	return ret_val;
}

DpLoopExitCode dp_opt_deep_update(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo = (DpDeepInfo*)(hopt->method_info);
	dp_deep_update_step(hdeepinfo);
	return ret_val;
}

DpLoopExitCode dp_write_log(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpEvaluation*heval = (DpEvaluation*)(hopt->heval);
	DpTarget*htarget = (DpTarget*)(hopt->htarget);
	DpDeepInfo*hdeepinfo;
	DpOsdaInfo*hosdainfo;
	FILE*fp;
	int i;
	int precision = hopt->precision;
	fp = fopen(hopt->logname, "a");
	if ( !fp ) {
		hloop->exit_str = g_strdup_printf ( "dp_write_log: can't open %s", hopt->logname);
		return DP_LOOP_EXIT_ERROR;
	}
	fprintf(fp, "wtime:%e tau:%d", hloop->w_time, hloop->tau_counter);
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
            DpIndivid* individ = hdeepinfo->population->individ[hdeepinfo->population->imin];
            fprintf(fp, " cost:%.*f", precision, individ->cost);
            for( i = 0; i < individ->ntargets; i++) {
                fprintf(fp, " target[%i]:%.*f", i, precision, individ->targets[i]);
            }
		break;
		case H_OPT_OSDA:
			hosdainfo = (DpOsdaInfo*)(hopt->method_info);
		break;
	}
	for ( i = 0; i < heval->size; i++) {
		fprintf(fp, " p[%d]:%.*f", heval->points[i]->index, precision, *(heval->points[i]->param));
	}
	fprintf(fp, "\n");
	fclose(fp);
	return ret_val;
}

DpLoopExitCode dp_read_log(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpEvaluation*heval = (DpEvaluation*)(hopt->heval);
	DpTarget*htarget = (DpTarget*)(hopt->htarget);
	char              *base;
	gchar **base_tokens;
	int n_base_tokens, n_extra_tokens;
	base = (char *)calloc(MAX_RECORD, sizeof(char *));
	int monitor = hopt->monitor;
	FILE*fp;
	int i;
	fp = fopen(hopt->logname, "r");
	if ( !fp ) {
		hloop->exit_str = g_strdup_printf ( "dp_read_log: can't open %s", hopt->logname);
		return DP_LOOP_EXIT_ERROR;
	}
    base = fgets(base, MAX_RECORD, fp);
    base_tokens = g_strsplit(base, ":", -1);
    n_base_tokens = g_strv_length(base_tokens) - 1;
    n_extra_tokens = n_base_tokens - ( heval->size + 3 );
    g_strfreev(base_tokens);
    free(base);
	do {
		fscanf(fp, "wtime:%lf tau:%d cost:%lf", &(hloop->w_time), &(hloop->tau_counter), &(hopt->cost));
        for ( i = 0; i < n_extra_tokens; i++) {
            fscanf(fp, " target[%*i]:%*f");
		}
		for ( i = 0; i < heval->size; i++) {
			fscanf(fp, " p[%d]:%lf", &(heval->points[i]->index), heval->points[i]->param);
		}
	} while ( !feof(fp) && monitor != hloop->tau_counter );
	fclose(fp);
	return ret_val;
}

DpLoopExitCode dp_write_state(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	DpOsdaInfo*hosdainfo;
	FILE*fp;
	fp = fopen(hopt->chkname, "w");
	if ( !fp ) {
		hloop->exit_str = g_strdup_printf ( "dp_write_state: can't open %s", hopt->chkname);
		return DP_LOOP_EXIT_ERROR;
	}
	fprintf(fp, "%e %e %e %e %d %e %d %e\n", hloop->w_time, hloop->u_time, hloop->start, hloop->finish, hloop->tau_counter, hopt->cost, hopt->stop_counter, hopt->old_cost);
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			dp_deep_info_save(fp, hdeepinfo);
		break;
		case H_OPT_OSDA:
			hosdainfo = (DpOsdaInfo*)(hopt->method_info);
			dp_osda_info_save(fp, hosdainfo);
		break;
	}
	fclose(fp);
	return ret_val;
}

DpLoopExitCode dp_rotate_target(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpTarget*htarget = (DpTarget*)(hopt->htarget);
	DpDeepInfo*hdeepinfo;
	DpOsdaInfo*hosdainfo;
	int *indices_of_targets;
	int n_indices;
	int offset;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			dp_target_shift_penalty_ranks (htarget);
		break;
		case H_OPT_OSDA:
			hosdainfo = (DpOsdaInfo*)(hopt->method_info);
		break;
	}
	return ret_val;
}

DpLoopExitCode dp_read_state(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	DpOsdaInfo*hosdainfo;
	FILE*fp;
	if ( g_file_test (hopt->chkname, G_FILE_TEST_EXISTS) ) {
	  fp = fopen(hopt->chkname, "r");
	  if ( !fp ) {
	    hloop->exit_str = g_strdup_printf ( "dp_read_state: can't open %s", hopt->chkname);
	    return DP_LOOP_EXIT_ERROR;
	  }
	  fscanf(fp, "%e %e %e %e %d %e %d %e", &(hloop->w_time), &(hloop->u_time), &(hloop->start), &(hloop->finish), &(hloop->tau_counter), &(hopt->cost), &(hopt->stop_counter), &(hopt->old_cost));
	  switch (hopt->opt_type) {
	  case H_OPT_DEEP:
	    hdeepinfo = (DpDeepInfo*)(hopt->method_info);
	    dp_deep_info_load(fp, hdeepinfo);
	    break;
	  case H_OPT_OSDA:
	    hosdainfo = (DpOsdaInfo*)(hopt->method_info);
	    dp_osda_info_load(fp, hosdainfo);
	    break;
	  }
	  fclose(fp);
	}
	/*	dp_loop_del_func(hloop, dp_read_state);*/
	return ret_val;
}

DpLoopExitCode dp_write_tst(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_SUCCESS;
	DpOpt*hopt = (DpOpt*)user_data;
	DpEvaluation*heval = (DpEvaluation*)(hopt->heval);
	DpTarget*htarget = (DpTarget*)(hopt->htarget);
	FILE*fp;
	int i;
	fp = fopen(hopt->tstname, "w");
	if ( !fp ) {
		hloop->exit_str = g_strdup_printf ( "dp_write_tst: can't open %s", hopt->tstname);
		return DP_LOOP_EXIT_ERROR;
	}
	fprintf(fp, "user       time:%e\n", hloop->u_time);
	fprintf(fp, "wall clock time:%e\n", hloop->w_time);
	fprintf(fp, "tau     counter:%d\n", hloop->tau_counter);
	fprintf(fp, "final      cost:%e\n", hopt->cost);
//	fprintf(fp, "cost       runs:%d\n", hopt->htarget->target->kount);
	for ( i = 0; i < hopt->htarget->size; i++ ) {
//		fprintf(fp, "penalty[%d]cost:%e\n", i, hopt->htarget->penalty[i]->retval);
//		fprintf(fp, "penalty[%d]runs:%d\n", i, hopt->htarget->penalty[i]->kount);
	}
	fprintf(fp, "\n");
	fclose(fp);
	return ret_val;
}

DpLoopExitCode dp_opt_osda(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpOsdaInfo*hosdainfo = (DpOsdaInfo*)(hopt->method_info);
	double energy_start;
	energy_start = hopt->cost;
	dp_osda_step(hosdainfo);
	dp_osda_accept_step(hosdainfo, &(hopt->cost));
	dp_osda_update_step(hosdainfo);
	return ret_val;
}

DpLoopExitCode dp_opt_mpi_distribute(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	int stop_flag = ( hloop->stop_flag == DP_LOOP_EXIT_SUCCESS ) ? 1 : 0;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			dp_population_mpi_distribute(hdeepinfo->trial, hopt->world_id, hopt->world_count);
		break;
	}
	hloop->stop_flag = ( stop_flag == 1 ) ? DP_LOOP_EXIT_SUCCESS : DP_LOOP_EXIT_NOEXIT;
	return ret_val;
}

DpLoopExitCode dp_opt_mpi_gather(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	int stop_flag = ( hloop->stop_flag == DP_LOOP_EXIT_SUCCESS ) ? 1 : 0;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			dp_population_mpi_gather(hdeepinfo->trial, hopt->world_id, hopt->world_count);
		break;
	}
	hloop->stop_flag = ( stop_flag == 1 ) ? DP_LOOP_EXIT_SUCCESS : DP_LOOP_EXIT_NOEXIT;
	return ret_val;
}

DpLoopExitCode dp_opt_duplicate(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	int stop_flag = ( hloop->stop_flag == DP_LOOP_EXIT_SUCCESS ) ? 1 : 0;
	int src_ind, dst_ind, i;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			g_qsort_with_data(hdeepinfo->population->ages_descending, hdeepinfo->population->size, sizeof(hdeepinfo->population->ages_descending[0]), (GCompareDataFunc)dp_individ_ages_descending, hdeepinfo->population);
			g_qsort_with_data(hdeepinfo->population->cost_ascending, hdeepinfo->population->size, sizeof(hdeepinfo->population->cost_ascending[0]), (GCompareDataFunc)dp_individ_cost_ascending, hdeepinfo->population);
            for ( i = 0; i < hdeepinfo->es_lambda; i++ ) {
                src_ind = hdeepinfo->population->cost_ascending[i];
                dst_ind = hdeepinfo->population->ages_descending[i];
                if ( dst_ind != hdeepinfo->population->imin ) {
                    dp_individ_copy_values(hdeepinfo->population->individ[dst_ind], hdeepinfo->population->individ[src_ind]);
                }
			}
			dp_population_update(hdeepinfo->population, 0, hdeepinfo->population->size);
		break;
	}
	hloop->stop_flag = ( stop_flag == 1 ) ? DP_LOOP_EXIT_SUCCESS : DP_LOOP_EXIT_NOEXIT;
	return ret_val;
}

DpLoopExitCode dp_opt_substold(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	int stop_flag = ( hloop->stop_flag == DP_LOOP_EXIT_SUCCESS ) ? 1 : 0;
	int src_ind, dst_ind, i;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			g_qsort_with_data(hdeepinfo->population->ages_descending, hdeepinfo->population->size, sizeof(hdeepinfo->population->ages_descending[0]), (GCompareDataFunc)dp_individ_ages_descending, hdeepinfo->population);
			g_qsort_with_data(hdeepinfo->population->cost_ascending, hdeepinfo->population->size, sizeof(hdeepinfo->population->cost_ascending[0]), (GCompareDataFunc)dp_individ_cost_ascending, hdeepinfo->population);
			for ( i = 0; i < hdeepinfo->es_lambda; i++ ) {
                src_ind = hdeepinfo->population->cost_ascending[i];
                dst_ind = hdeepinfo->population->ages_descending[i];
				if (hopt->debug > 1) {
					g_message ("dp_opt_substold: trying count=%d; iter=%d; src=%d; age=%d; failures=%d; cost=%f; dst=%d; age=%d; failures=%d; cost=%f;", i, hdeepinfo->population->iter, src_ind, hdeepinfo->population->individ[src_ind]->age, hdeepinfo->population->individ[src_ind]->failures, hdeepinfo->population->individ[src_ind]->cost, dst_ind, hdeepinfo->population->individ[dst_ind]->age, hdeepinfo->population->individ[dst_ind]->failures, hdeepinfo->population->individ[dst_ind]->cost);
				}
				if ( hdeepinfo->population->individ[dst_ind]->age < hdeepinfo->es_cutoff ) break;
                if ( dst_ind != hdeepinfo->population->imin ) {
					if (hopt->debug > 0) {
						g_message ("dp_opt_substold: changed count=%d; iter=%d; src=%d; age=%d; failures=%d; cost=%f; dst=%d; age=%d; failures=%d; cost=%f;", i, hdeepinfo->population->iter, src_ind, hdeepinfo->population->individ[src_ind]->age, hdeepinfo->population->individ[src_ind]->failures, hdeepinfo->population->individ[src_ind]->cost, dst_ind, hdeepinfo->population->individ[dst_ind]->age, hdeepinfo->population->individ[dst_ind]->failures, hdeepinfo->population->individ[dst_ind]->cost);
					}
                    dp_individ_copy_values(hdeepinfo->population->individ[dst_ind], hdeepinfo->population->individ[src_ind]);
                }
			}
			dp_population_update(hdeepinfo->population, 0, hdeepinfo->population->size);
		break;
	}
	hloop->stop_flag = ( stop_flag == 1 ) ? DP_LOOP_EXIT_SUCCESS : DP_LOOP_EXIT_NOEXIT;
	return ret_val;
}

DpLoopExitCode dp_opt_substfailed(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	int stop_flag = ( hloop->stop_flag == DP_LOOP_EXIT_SUCCESS ) ? 1 : 0;
	int src_ind, dst_ind, i;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			g_qsort_with_data(hdeepinfo->population->ages_descending, hdeepinfo->population->size, sizeof(hdeepinfo->population->ages_descending[0]), (GCompareDataFunc)dp_individ_failures_descending, hdeepinfo->population);
			g_qsort_with_data(hdeepinfo->population->cost_ascending, hdeepinfo->population->size, sizeof(hdeepinfo->population->cost_ascending[0]), (GCompareDataFunc)dp_individ_cost_ascending, hdeepinfo->population);
			for ( i = 0; i < hdeepinfo->es_lambda; i++ ) {
                src_ind = hdeepinfo->population->cost_ascending[i];
                dst_ind = hdeepinfo->population->ages_descending[i];
				if (hopt->debug > 1) {
					g_message ("dp_opt_subsfailed: trying count=%d; iter=%d; src=%d; age=%d; failures=%d; cost=%f; dst=%d; age=%d; failures=%d; cost=%f;", i, hdeepinfo->population->iter, src_ind, hdeepinfo->population->individ[src_ind]->age, hdeepinfo->population->individ[src_ind]->failures, hdeepinfo->population->individ[src_ind]->cost, dst_ind, hdeepinfo->population->individ[dst_ind]->age, hdeepinfo->population->individ[dst_ind]->failures, hdeepinfo->population->individ[dst_ind]->cost);
				}
				if ( hdeepinfo->population->individ[dst_ind]->failures < hdeepinfo->es_cutoff ) break;
                if ( dst_ind != hdeepinfo->population->imin ) {
					if (hopt->debug > 0) {
						g_message ("dp_opt_subsfailed: changed count=%d; iter=%d; src=%d; age=%d; failures=%d; cost=%f; dst=%d; age=%d; failures=%d; cost=%f;", i, hdeepinfo->population->iter, src_ind, hdeepinfo->population->individ[src_ind]->age, hdeepinfo->population->individ[src_ind]->failures, hdeepinfo->population->individ[src_ind]->cost, dst_ind, hdeepinfo->population->individ[dst_ind]->age, hdeepinfo->population->individ[dst_ind]->failures, hdeepinfo->population->individ[dst_ind]->cost);
					}
                    dp_individ_copy_values(hdeepinfo->population->individ[dst_ind], hdeepinfo->population->individ[src_ind]);
                }
			}
			dp_population_update(hdeepinfo->population, 0, hdeepinfo->population->size);
		break;
	}
	hloop->stop_flag = ( stop_flag == 1 ) ? DP_LOOP_EXIT_SUCCESS : DP_LOOP_EXIT_NOEXIT;
	return ret_val;
}
				
DpLoopExitCode dp_opt_substitute(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	int stop_flag = ( hloop->stop_flag == DP_LOOP_EXIT_SUCCESS ) ? 1 : 0;
	int src_ind, dst_ind, i;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			g_qsort_with_data(hdeepinfo->population->ages_descending, hdeepinfo->population->size, sizeof(hdeepinfo->population->ages_descending[0]), (GCompareDataFunc)dp_individ_ages_descending, hdeepinfo->population);
			g_qsort_with_data(hdeepinfo->population->cost_ascending, hdeepinfo->population->size, sizeof(hdeepinfo->population->cost_ascending[0]), (GCompareDataFunc)dp_individ_cost_ascending, hdeepinfo->population);
            src_ind = hdeepinfo->population->cost_ascending[0];
            for ( i = 0; i < hdeepinfo->es_lambda; i++ ) {
                dst_ind = hdeepinfo->population->ages_descending[i];
                if ( dst_ind != hdeepinfo->population->imin ) {
                    dp_individ_copy_values(hdeepinfo->population->individ[dst_ind], hdeepinfo->population->individ[src_ind]);
                }
			}
			dp_population_update(hdeepinfo->population, 0, hdeepinfo->population->size);
		break;
	}
	hloop->stop_flag = ( stop_flag == 1 ) ? DP_LOOP_EXIT_SUCCESS : DP_LOOP_EXIT_NOEXIT;
	return ret_val;
}

DpLoopExitCode dp_opt_permute(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
    int i, src_ind, dst_ind, seed_ind;
    DpIndivid*tmp_ind;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			DpPopulation*pop = hdeepinfo->population;
			seed_ind = hdeepinfo->es_lambda % pop->size;
			for ( i = 0; i < hdeepinfo->es_lambda; i++ ) {
                src_ind = g_rand_int_range (pop->individ[seed_ind]->hrand, 0, pop->size);
                dst_ind = g_rand_int_range (pop->individ[seed_ind]->hrand, 0, pop->size);
                tmp_ind = pop->individ[dst_ind];
                pop->individ[dst_ind] = pop->individ[src_ind];
                pop->individ[src_ind] = tmp_ind;
			}
		break;
	}
	return ret_val;
}

DpLoopExitCode dp_opt_post(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	DpOsdaInfo*hosdainfo;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			dp_deep_post(hdeepinfo);
		break;
		case H_OPT_OSDA:
			hosdainfo = (DpOsdaInfo*)(hopt->method_info);
			dp_osda_post(hosdainfo);
		break;
	}
	return ret_val;
}

DpLoopExitCode dp_opt_post_evaluate(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	DpOsdaInfo*hosdainfo;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			dp_deep_post_evaluate(hdeepinfo);
		break;
		case H_OPT_OSDA:
			hosdainfo = (DpOsdaInfo*)(hopt->method_info);
			dp_osda_post_evaluate(hosdainfo);
		break;
	}
	return ret_val;
}

DpLoopExitCode dp_opt_evaluate_pareto_front(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	DpPopulation*pop;
	GArray*fronts;
	GArray*curr_front, *next_front, *tmp_front;
	int i, curr_front_index, curr_ind, j, flag, k;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			pop = hdeepinfo->popunion;
			for ( i = 0; i < pop->size; i++ ) {
                pop->individ[i]->pareto_front = -1;
                pop->individ[i]->dom_count = 0;
                if ( pop->individ[i]->dominated != NULL ) {
                    g_array_free (pop->individ[i]->dominated, TRUE);
                }
                pop->individ[i]->dominated = g_array_new (FALSE, FALSE, sizeof (int));
			}
			fronts = g_array_new (FALSE, FALSE, sizeof (GArray*));
			curr_front = g_array_new (FALSE, FALSE, sizeof (int));
            for ( i = 0; i < pop->size; i++ ) {
                for ( j = 0; j < pop->size; j++ ) {
                    if ( 1 == dp_evaluation_individ_dominates_compare((const void *)(&(pop->individ[i])), (const void *)(&(pop->individ[j])), (void*)(hdeepinfo->hevalctrl)) ) {
                        g_array_append_val (pop->individ[i]->dominated, j);
                    } else if ( 1 == dp_evaluation_individ_dominates_compare((const void *)(&(pop->individ[j])), (const void *)(&(pop->individ[i])), (void*)(hdeepinfo->hevalctrl)) ) {
                        pop->individ[i]->dom_count++;
                    }
                }
                if ( pop->individ[i]->dom_count == 0 ) {
                    pop->individ[i]->pareto_front = 0;
                    g_array_append_val (curr_front, i);
                }
            }
            g_array_append_val (fronts, curr_front);
            curr_front_index = 0;
            do {
                next_front = g_array_new (FALSE, FALSE, sizeof (int));
                flag = 0;
                for ( k = 0; k < curr_front->len; k++ ) {
                    i = g_array_index (curr_front, int, k);
                    for ( j = 0; j < pop->individ[i]->dominated->len; j++ ) {
                        curr_ind = g_array_index (pop->individ[i]->dominated, int, j);
                        pop->individ[curr_ind]->dom_count--;
                        if ( pop->individ[curr_ind]->dom_count == 0 ) {
                            pop->individ[curr_ind]->pareto_front = curr_front_index + 1;
                            g_array_append_val (next_front, curr_ind);
                            flag = 1;
                        }
                    }
                }
                if ( flag == 1 ) {
                    curr_front_index++;
                    g_array_append_val (fronts, next_front);
                } else {
                    g_array_free ( next_front, FALSE );
                }
                tmp_front = curr_front;
                curr_front = next_front;
            } while ( flag == 1 );
            pop->nfronts = curr_front_index;
            if ( pop->fronts != NULL ) {
                for ( k = 0; k < pop->fronts->len; k++ ) {
                    tmp_front = g_array_index (pop->fronts, GArray*, k);
                    g_array_free ( tmp_front, TRUE );
                }
                g_array_free ( pop->fronts, TRUE );
            }
            pop->fronts = fronts;
		break;
	}
	return ret_val;
}

DpLoopExitCode dp_sort_pareto_front(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	DpPopulation*pop, *trial, *population;
	GArray*curr_front;
	int i, curr_ind, j, k;
	DpEvaluation*heval = (DpEvaluation*)(hopt->heval);
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			pop = hdeepinfo->popunion;
            for ( k = 0; k < pop->fronts->len; k++ ) {
                curr_front = g_array_index (pop->fronts, GArray*, k);
                g_array_sort_with_data(curr_front, (GCompareDataFunc) dp_evaluation_cr_compare, (gpointer) pop);
            }
		break;
	}
	return ret_val;
}

DpLoopExitCode dp_select_pareto_front(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	DpPopulation*pop, *trial, *population;
	GArray*curr_front;
	int i, curr_ind, j, k;
	DpEvaluation*heval = (DpEvaluation*)(hopt->heval);
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
            if ( hdeepinfo->selection_done == 1 ) {
                return ret_val;
            }
			pop = hdeepinfo->popunion;
			population = hdeepinfo->population;
			trial = hdeepinfo->trial;
			i = 0;
            for ( k = 0; k < pop->fronts->len; k++ ) {
                curr_front = g_array_index (pop->fronts, GArray*, k);
                for ( j = 0; j < curr_front->len; j++ ) {
                    curr_ind = g_array_index (curr_front, int, j);
                    if ( i < population->size ) {
                        population->individ[i] = pop->individ[curr_ind];
                        population->individ[i]->age = 0;
                    } else {
                        trial->individ[i - population->size] = pop->individ[curr_ind];
                        trial->individ[i - population->size]->age++;
                    }
                    i++;
                }
            }
            trial->iter = population->iter;
            population->iter++;
            dp_population_update(population, 0, population->size);
            dp_deep_accept_step(hdeepinfo, &(hopt->cost));
            hdeepinfo->selection_done = 1;
		break;
	}
	return ret_val;
}

DpLoopExitCode dp_write_pareto(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	DpPopulation*pop;
	GArray*curr_front;
	GString *params;
	int i, curr_ind, j, k, lim_ind;
	DpEvaluation*heval = (DpEvaluation*)(hopt->heval);
	FILE*fp;
	gchar*pareto_logname;
	pareto_logname = g_strdup_printf("%s_pareto", hopt->logname);
	fp = fopen(pareto_logname, "a");
	if ( !fp ) {
		hloop->exit_str = g_strdup_printf ( "dp_write_pareto: can't open %s", pareto_logname);
		return DP_LOOP_EXIT_ERROR;
	}
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
		    hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			DpTarget*htarget = hopt->htarget;
			pop = hdeepinfo->popunion;
			lim_ind = ( hopt->pareto_all < 0 ) ? pop->fronts->len : hopt->pareto_all + 1;
            for ( k = 0; k < lim_ind; k++ ) {
                curr_front = g_array_index (pop->fronts, GArray*, k);
                for ( j = 0; j < curr_front->len; j++ ) {
                    curr_ind = g_array_index (curr_front, int, j);
                    DpIndivid* individ = pop->individ[curr_ind];
                    params = htarget->params_to_string(individ->user_data, individ->z);
                    fprintf(fp, "wtime:%e tau:%d cost:%f front:%d:%d", hloop->w_time, hloop->tau_counter, individ->cost, k, j);
                    for( i = 0; i < individ->ntargets; i++) {
                        fprintf(fp, " target[%i]:%f", i, individ->targets[i]);
                    }
/*                    for ( i = 0; i < heval->size; i++) {
                        fprintf(fp, " p[%d]:%f", heval->points[i]->index, individ->z[i]);
                    }*/
                    fprintf(fp, " %s", params->str);
                    fprintf(fp, "\n");
                    g_string_free(params, TRUE);
                }
                fprintf(fp, "\n\n");
            }
            fprintf(fp, "\n\n\n\n");
		break;
	}
	fclose(fp);
	g_free(pareto_logname);
	return ret_val;
}

DpLoopExitCode dp_opt_cr2cost(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	DpOsdaInfo*hosdainfo;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			dp_population_cr2cost(hdeepinfo->popunion);
		break;
		case H_OPT_OSDA:
			hosdainfo = (DpOsdaInfo*)(hopt->method_info);
		break;
	}
	return ret_val;
}
