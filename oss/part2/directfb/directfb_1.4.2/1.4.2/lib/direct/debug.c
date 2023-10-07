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


#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <direct/clock.h>
#include <direct/debug.h>
#include <direct/list.h>
#include <direct/log.h>
#include <direct/mem.h>
#include <direct/print.h>
#include <direct/system.h>
#include <direct/thread.h>
#include <direct/trace.h>
#include <direct/util.h>


/**********************************************************************************************************************/

#if DIRECT_BUILD_TEXT

#if DIRECT_BUILD_DEBUGS  /* Build with debug support? */

__no_instrument_function__
static inline void
debug_domain_vprintf( const char        *name,
                      int                name_len,
                      const char        *format,
                      va_list            ap )
{
     char        buf[512];
     long long   millis = direct_clock_get_millis();
     const char *thread = direct_thread_self_name();
     int         indent = direct_trace_debug_indent() * 4;

     /* Prepare user message. */
     if (vsnprintf( buf, sizeof(buf), format, ap ) < 0)
         direct_log_printf(NULL, "[DFB] vsnprintf failed with error [%s] (%s)\n", strerror(errno), __FUNCTION__);

     /* Fill up domain name column after the colon, prepending remaining space (excl. ': ') to indent. */
     indent += (name_len < 20 ? 20 : 36) - name_len - 2;

     /* Print full message. */
     direct_log_printf( NULL, "(-) [%-15s %3lld.%03lld] (pid=%5d, tid=%5d) %s: %*s%s", thread ? thread : "  NO NAME",
                        millis / 1000LL, millis % 1000LL, getpid(), direct_gettid(), name, indent, "", buf );
}

  
__no_instrument_function__
void
direct_debug_log( DirectLogDomain *domain,
              unsigned int     debug_level,  /* 1-9, 0 = info */
              const char      *format, ... )
{
     va_list ap;

     debug_level += DIRECT_LOG_DEBUG_0;

     va_start( ap, format );
     direct_log_domain_vprintf( domain, debug_level > DIRECT_LOG_DEBUG_9 ? DIRECT_LOG_DEBUG_9 : debug_level, format, ap );
     va_end( ap );
}

__no_instrument_function__
void
direct_debug_at( DirectLogDomain *domain,
                 const char      *format, ... )
{
     va_list ap;

     va_start( ap, format );
     direct_log_domain_vprintf( domain, DIRECT_LOG_DEBUG, format, ap );
     va_end( ap );
}

__no_instrument_function__
void
direct_debug( const char *format, ... )
{
     va_list ap;

     va_start( ap, format );

     debug_domain_vprintf( "- - ", 4, format, ap );

     va_end( ap );
}

#endif /* DIRECT_BUILD_DEBUGS */

__no_instrument_function__
void
direct_debug_at_always( DirectLogDomain *domain,
                        const char      *format, ... )
{
     if (direct_config->log_level >= DIRECT_LOG_DEBUG) {
          va_list ap;

          va_start( ap, format );
          direct_log_domain_vprintf( domain, DIRECT_LOG_NONE, format, ap );
          va_end( ap );
     }
}

#if DIRECT_BUILD_DEBUGS  /* Build with debug support? */

__no_instrument_function__
void
direct_break( const char *func,
              const char *file,
              int         line,
              const char *format, ... )
{
     char        buf[512];
     long long   millis = direct_clock_get_millis();
     const char *name   = direct_thread_self_name();

     va_list ap;

     va_start( ap, format );

     direct_vsnprintf( buf, sizeof(buf), format, ap );

     va_end( ap );

     direct_log_printf( NULL,
                        "(!) [%-15s %3lld.%03lld] (%5d) *** Break [%s] *** [%s:%d in %s()]\n",
                        name ? name : "  NO NAME  ", millis / 1000LL, millis % 1000LL,
                        direct_gettid(), buf, file, line, func );

     direct_trace_print_stack( NULL );

     if (direct_config->fatal_break)
          direct_trap( "Break", SIGABRT );
}

__no_instrument_function__
void
direct_assertion( const char *exp,
                  const char *func,
                  const char *file,
                  int         line )
{
     long long   millis = direct_clock_get_millis();
     const char *name   = direct_thread_self_name();

     direct_log_printf( NULL,
                        "(!) [%-15s %3lld.%03lld] (%5d) *** Assertion [%s] failed *** [%s:%d in %s()]\n",
                        name ? name : "  NO NAME  ", millis / 1000LL, millis % 1000LL,
                        direct_gettid(), exp, file, line, func );

     direct_trace_print_stack( NULL );

     if (direct_config->fatal >= DCFL_ASSERT)
          direct_trap( "Assertion", SIGTRAP );
}

__no_instrument_function__
void
direct_assumption( const char *exp,
                   const char *func,
                   const char *file,
                   int         line )
{
     long long   millis = direct_clock_get_millis();
     const char *name   = direct_thread_self_name();

     direct_log_printf( NULL,
                        "(!) [%-15s %3lld.%03lld] (%5d) *** Assumption [%s] failed *** [%s:%d in %s()]\n",
                        name ? name : "  NO NAME  ", millis / 1000LL, millis % 1000LL,
                        direct_gettid(), exp, file, line, func );

     direct_trace_print_stack( NULL );

     if (direct_config->fatal >= DCFL_ASSUME)
          direct_trap( "Assumption", SIGTRAP );
}

#endif /* DIRECT_BUILD_DEBUGS */

#endif /* DIRECT_BUILD_TEXT */

void
direct_debug_config_domain( const char *name, bool enable )
{
     direct_log_domain_config_level( name, enable ? DIRECT_LOG_ALL : DIRECT_LOG_NONE );
}

void direct_debug_redirect_to_tmpfile(const char  *format, ... )
{
/**
    At console, type the commands:

    tail -f <temp file name (i.e. /tmp/dfb_tmpfile_PID1234_XXXXXX) >

*/
    static int fd = -1;

    char buf[512] = {0};

    int pid = getpid();

    va_list ap;

    va_start( ap, format );

    /* Prepare user message. */
    if (vsnprintf( buf, sizeof(buf), format, ap ) < 0)
        printf("[DFB] vsnprintf failed with error [%s] (%s)\n", strerror(errno), __FUNCTION__);

    va_end( ap );

    if(fd <0)
    {
        char nameBuff[32] = {0};
        if (snprintf(nameBuff, 32,"/tmp/dfb_tmpfile_PID%d_XXXXXX", pid) < 0)
        {
            printf("[DFB] snprintf failed with error [%s] (%s)\n", strerror(errno), __FUNCTION__);
            return;
        }

        fd = mkstemp(nameBuff);

        if (fd < 0)
        {
            printf("[DFB] Creation of temp file failed with error [%s] (%s)\n", strerror(errno), __FUNCTION__);
            return;
        }

        printf("[DFB] Temp-Filename = %s\n", nameBuff);
    }

    if(-1 == write(fd, buf, sizeof(buf)))
    {
        printf("[DFB] Write to tmpfile failed with error [%s] (%s)\n", strerror(errno), __FUNCTION__);
        return;
    }

}


