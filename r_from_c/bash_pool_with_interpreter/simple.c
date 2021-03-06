#include<glib.h>
#include<unistd.h>
#include<stdio.h>

struct interpreter{
    GIOChannel *in,
               *out,
               *err;
};

void write_to_interpreter(GIOChannel* in, GString* msg){
    g_io_channel_write_chars( in,
            msg->str,
            msg->len,
            NULL,
            NULL);
    g_io_channel_flush(in, NULL);
}

GString * create_command(int n){
    GString * cmd = g_string_new("echo \"$((");
    char number[30];
    sprintf(number, "%d", n);
    cmd = g_string_append(cmd, number);
    cmd = g_string_append(cmd, "+1))\"");
    g_print("VALUE: %s\n", cmd->str);
    return cmd;
}

gpointer pool_func(gpointer data, gpointer user_data){
    GAsyncQueue * queue = (GAsyncQueue*)user_data;

    struct interpreter* intprt = (struct interpreter*)g_async_queue_pop(queue); 
    int val = *((int*)data);

    g_print("interpreter = %d, data = %d\n", intprt, val);
    GString * cmd = create_command(val);
    write_to_interpreter(intprt->in, cmd);

    g_async_queue_push(queue, (gpointer)intprt);
    return 0;
}

static gboolean out_watch( GIOChannel   *channel,
        GIOCondition  cond, gpointer data)
{
    gchar *string;
    gsize  size;

    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }

    g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    g_print("Out: %s", string);
    g_io_channel_flush(channel, NULL);
    g_free( string );

    return TRUE;
}

static gboolean err_watch( GIOChannel   *channel,
        GIOCondition  cond, gpointer data)
{
    gchar *string;
    gsize  size;

    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }

    g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    g_print("Err: %s", string);
    g_io_channel_flush(channel, NULL);
    g_free( string );

    return TRUE;
}

struct interpreter* init_interpreter(void){
    gchar      *argv[] = { "bash", "+x", NULL };
    gint        in,
                out,
                err;
    gboolean    ret;

    /* Spawn child process */
    ret = g_spawn_async_with_pipes( NULL, argv, NULL,
            G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD, NULL,
            NULL, NULL,
            &in, &out, &err, NULL );
    if( ! ret )
    {
        g_error( "SPAWN FAILED" );
        return NULL;
    }

    struct interpreter* intprt = g_new(struct interpreter, 1);
    intprt->in = g_io_channel_unix_new( in );
    intprt->out = g_io_channel_unix_new( out );
    intprt->err = g_io_channel_unix_new( err );

    /* Add watches to channels */
    g_io_add_watch( intprt->out, G_IO_IN | G_IO_HUP, (GIOFunc)out_watch, NULL);
    g_io_add_watch( intprt->err, G_IO_IN | G_IO_HUP, (GIOFunc)err_watch, NULL);

    return intprt;
}


int main(){
    int i;
    int num_processors = g_get_num_processors();
    g_print("Number processors: %d\n", num_processors);

    g_print("Create queue\n");
    GAsyncQueue *q = g_async_queue_new();

    g_print("Push interpreters to queue\n");
    struct interpreter* intprt;
    for(i = 0; i < num_processors; i++){
        intprt = init_interpreter();
        g_async_queue_push(q, (gpointer)intprt);
    }
    g_print("Number tasks in queue: %d\n", g_async_queue_length(q));

    g_print("Create pool\n");
    GThreadPool *pool = g_thread_pool_new(
            (GFunc)pool_func,
            (gpointer)q,
            g_get_num_processors(),
            TRUE, NULL);

    g_print("Push tasks to pool\n");
    int num_tasks = 2 * num_processors;
    int* tasks = g_new(int, num_tasks);
    for(i = 0; i < num_tasks; i++){
        tasks[i] = i;
        g_thread_pool_push (pool, (gpointer)(tasks+i), NULL);
    }

    GMainLoop *loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);

    g_print("End main\n");
    return 0;
}
