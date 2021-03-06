#include<glib.h>
#include<stdio.h>

    static void
cb_child_watch( GPid  pid,
        gint  status)
{
    /* Close pid */
    g_spawn_close_pid( pid );
}

    static gboolean
cb_out_watch( GIOChannel   *channel,
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
    g_print("%s", string);
    printf("%s", string);
    fflush(stdout);
    //gtk_text_buffer_insert_at_cursor( data->out, string, -1 );
    g_free( string );

    return( TRUE );
}

    static gboolean
cb_err_watch( GIOChannel   *channel,
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
    g_print("%s", string);
    printf("%s", string);
    fflush(stdout);
    //gtk_text_buffer_insert_at_cursor( data->err, string, -1 );
    g_free( string );

    return( TRUE );
}

int
main()
{
    GPid        pid;
    gchar      *argv[] = { "./helper", NULL };
    gint        out,
                err;
    GIOChannel *out_ch,
               *err_ch;
    gboolean    ret;

    /* Spawn child process */
    ret = g_spawn_async_with_pipes( NULL, argv, NULL,
            G_SPAWN_DO_NOT_REAP_CHILD, NULL,
            NULL, &pid, NULL, &out, &err, NULL );
    if( ! ret )
    {
        g_error( "SPAWN FAILED" );
        return 0;
    }

    g_child_watch_add( pid, (GChildWatchFunc)cb_child_watch, NULL);

    out_ch = g_io_channel_unix_new( out );
    err_ch = g_io_channel_unix_new( err );

    /* Add watches to channels */
    g_io_add_watch( out_ch,G_IO_ERR | G_IO_IN | G_IO_OUT | G_IO_HUP, (GIOFunc)cb_out_watch, NULL);
    g_io_add_watch( err_ch,G_IO_ERR| G_IO_IN | G_IO_OUT | G_IO_HUP, (GIOFunc)cb_err_watch, NULL);

    return 0;
}
