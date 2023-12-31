/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <direct/build.h>
#include <direct/log.h>
#include <direct/messages.h>
#include <direct/print.h>
#include <direct/result.h>
#include <direct/trace.h>
#include <direct/util.h>

/**********************************************************************************************************************/

#if DIRECT_BUILD_TEXT

__no_instrument_function__
void
direct_messages_info( const char *func, const int line, const int pid, const char *format, ... )
{
     char buf[512];

     va_list ap;

     va_start( ap, format );

     direct_vsnprintf( buf, sizeof(buf), format, ap );

     va_end( ap );

     if(direct_config->debug_file) {
          FILE *fp = fopen(direct_config->debug_file_dir, "a");
          int ret = 0;
          if(fp != NULL) {
               ret = fprintf(fp, "[%s %d] pid=%d %s", func, line, pid, buf);
               if(ret < 0)
                    direct_log_printf( NULL, "fprintf fail, ret=%d\n", ret );

               ret = fclose(fp);
               if(ret != 0)
                    direct_log_printf( NULL, "fclose fail, ret=%d\n", ret );
          }
     } else {
          direct_log_printf( NULL, "(*) [%s %d] pid=%d %s",  func, line, pid, buf);
     }
}

__no_instrument_function__
void
direct_messages_error( const char *func, const int line, const int pid, const char *format, ... )
{
     char buf[512];

     va_list ap;

     va_start( ap, format );

     direct_vsnprintf( buf, sizeof(buf), format, ap );

     va_end( ap );

     if(direct_config->debug_file) {
          FILE *fp = fopen(direct_config->debug_file_dir, "a");
          int ret = 0;
          if(fp != NULL) {
               ret = fprintf(fp, "(!) [%s %d] pid=%d %s", func, line, pid, buf);
               if(ret < 0)
                    direct_log_printf( NULL, "fprintf fail, ret=%d\n", ret );

               ret = fclose(fp);
               if(ret != 0)
                    direct_log_printf( NULL, "fclose fail, ret=%d\n", ret );
          }
     } else {
          direct_log_printf( NULL, "(!) [%s %d] pid=%d %s",  func, line, pid, buf );
     }

     direct_trace_print_stack( NULL );
}

__no_instrument_function__
void
direct_messages_derror( DirectResult result, const char *format, ... )
{
     char buf[512];

     va_list ap;

     va_start( ap, format );

     direct_vsnprintf( buf, sizeof(buf), format, ap );

     va_end( ap );

     direct_log_printf( NULL, "(!) %s    --> %s\n", buf, DirectResultString( result ) );

     direct_trace_print_stack( NULL );
}

__no_instrument_function__
void
direct_messages_perror( int erno, const char *format, ... )
{
     char buf[512];

     va_list ap;

     va_start( ap, format );

     direct_vsnprintf( buf, sizeof(buf), format, ap );

     va_end( ap );

     direct_log_printf( NULL, "(!) %s    --> %s\n", buf, direct_strerror( erno ) );

     direct_trace_print_stack( NULL );
}

__no_instrument_function__
void
direct_messages_dlerror( const char *dlerr, const char *format, ... )
{
     char buf[512];

     va_list ap;

     va_start( ap, format );

     direct_vsnprintf( buf, sizeof(buf), format, ap );

     va_end( ap );

     direct_log_printf( NULL, "(!) %s    --> %s\n", buf, dlerr );

     direct_trace_print_stack( NULL );
}

__no_instrument_function__
void
direct_messages_once( const char *func,
                      const char *file,
                      int         line,
                      const char *format, ... )
{
     char buf[512];

     va_list ap;

     va_start( ap, format );

     direct_vsnprintf( buf, sizeof(buf), format, ap );

     va_end( ap );

     direct_log_printf( NULL, " (!!!)  *** ONCE [%s] *** [%s:%d in %s()]\n", buf, file, line, func );

     direct_trace_print_stack( NULL );
}

__no_instrument_function__
void
direct_messages_unimplemented( const char *func,
                               const char *file,
                               int         line )
{
     direct_log_printf( NULL, " (!!!)  *** UNIMPLEMENTED [%s] *** [%s:%d]\n", func, file, line );

     direct_trace_print_stack( NULL );
}

__no_instrument_function__
void
direct_messages_bug( const char *func,
                     const char *file,
                     int         line,
                     const char *format, ... )
{
     char buf[512];

     va_list ap;

     va_start( ap, format );

     direct_vsnprintf( buf, sizeof(buf), format, ap );

     va_end( ap );

     direct_log_printf( NULL, " (!?!)  *** BUG [%s] *** [%s:%d in %s()]\n", buf, file, line, func );

     direct_trace_print_stack( NULL );
}

__no_instrument_function__
void
direct_messages_warn( const char *func,
                      const char *file,
                      int         line,
                      const char *format, ... )
{
     char buf[512];

     va_list ap;

     va_start( ap, format );

     direct_vsnprintf( buf, sizeof(buf), format, ap );

     va_end( ap );

     direct_log_printf( NULL, " (!!!)  *** WARNING [%s] *** [%s:%d in %s()]\n", buf, file, line, func );

     direct_trace_print_stack( NULL );
}

#endif

