/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>

#include <net/if.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <directfb_version.h>

#include <direct/clock.h>
#include <direct/debug.h>
#include <direct/interface.h>
#include <direct/list.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/thread.h>
#include <direct/util.h>

#include <voodoo/conf.h>
#include <voodoo/internal.h>
#include <voodoo/message.h>
#include <voodoo/play.h>
#include <voodoo/play_internal.h>

#ifdef MACOS
#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))
#endif

D_DEBUG_DOMAIN( Voodoo_Play, "Voodoo/Play", "Voodoo Play" );

/**********************************************************************************************************************/

typedef struct {
     DirectLink          link;

     VoodooPlayVersion   version;
     VoodooPlayInfo      info;

     long long           last_seen;

     char                addr[64];
} PlayerNode;

/**********************************************************************************************************************/

static void  player_send_info( VoodooPlayer    *player,
                               const in_addr_t *in_addr,
                               bool             discover );

static void *player_main_loop( DirectThread    *thread,
                               void            *arg );

/**********************************************************************************************************************/

static const int one = 1;

VoodooPlayVersion g_VoodooPlay_version;
VoodooPlayInfo    g_VoodooPlay_info;

/**********************************************************************************************************************/

/*
 * FIXME
 */
static void
generate_uuid( u8 *buf )
{
     int i;

     srand( direct_clock_get_abs_micros() );

     for (i=0; i<16; i++) {
          buf[i] = rand();
     }
}

/**********************************************************************************************************************/

DirectResult
voodoo_player_create( const VoodooPlayInfo  *info,
                      VoodooPlayer         **ret_player )
{
     DirectResult        ret;
     int                 fd;
     struct sockaddr_in  addr;
     VoodooPlayer       *player;

     D_ASSERT( ret_player != NULL );

     /* Create the player socket. */
     fd = socket( PF_INET, SOCK_DGRAM, 0 );
     if (fd < 0) {
          ret = errno2result( errno );
          D_PERROR( "Voodoo/Player: Could not create the socket via socket()!\n" );
          return ret;
     }

     /* Allow reuse of local address. */
     if (setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one) ) < 0)
          D_PERROR( "Voodoo/Player: Could not set SO_REUSEADDR!\n" );

     if (setsockopt( fd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one) ) < 0)
          D_PERROR( "Voodoo/Player: Could not set SO_BROADCAST!\n" );

     /* Bind the socket to the local port. */
     addr.sin_family      = AF_INET;
     addr.sin_addr.s_addr = inet_addr( "0.0.0.0" );
     addr.sin_port        = htons( 2323 );

     if (bind( fd, (struct sockaddr*) &addr, sizeof(addr) )) {
          ret = errno2result( errno );
          D_PERROR( "Voodoo/Player: Could not bind() the socket!\n" );
          close( fd );
          return ret;
     }

     /* Allocate player structure. */
     player = D_CALLOC( 1, sizeof(VoodooPlayer) );
     if (!player) {
          D_WARN( "out of memory" );
          close( fd );
          return DR_NOLOCALMEMORY;
     }

     pthread_mutex_init( &player->lock, NULL );

     /* Initialize player structure. */
     player->fd = fd;

     /* Fill version struct */
     player->version.v[0] = VPVF_LITTLE_ENDIAN | VPVF_32BIT_SERIALS;
     player->version.v[1] = DIRECTFB_MAJOR_VERSION;
     player->version.v[2] = DIRECTFB_MINOR_VERSION;
     player->version.v[3] = DIRECTFB_MICRO_VERSION;

     /* Fill info struct */
     direct_snputs( player->info.name,   voodoo_config->play_info.name,    VOODOO_PLAYER_NAME_LENGTH );
     direct_snputs( player->info.vendor, voodoo_config->play_info.vendor,  VOODOO_PLAYER_VENDOR_LENGTH );
     direct_snputs( player->info.model,  voodoo_config->play_info.model,   VOODOO_PLAYER_MODEL_LENGTH );
     direct_memcpy( player->info.uuid,   voodoo_config->play_info.uuid,    16 );

     if (info)
          player->info = *info;

     if (!player->info.name[0])
          direct_snputs( player->info.name, "Unnamed Player", VOODOO_PLAYER_NAME_LENGTH );

     if (!player->info.vendor[0])
          direct_snputs( player->info.vendor, "Unknown Vendor", VOODOO_PLAYER_VENDOR_LENGTH );

     if (!player->info.model[0])
          direct_snputs( player->info.model, "Unknown Model", VOODOO_PLAYER_MODEL_LENGTH );

     if (!player->info.uuid[0])
          generate_uuid( player->info.uuid );

     D_MAGIC_SET( player, VoodooPlayer );


     g_VoodooPlay_version = player->version;
     g_VoodooPlay_info    = player->info;


     char buf[33];

     snprintf( buf, sizeof(buf), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
               player->info.uuid[0], player->info.uuid[1], player->info.uuid[2], player->info.uuid[3], player->info.uuid[4],
               player->info.uuid[5], player->info.uuid[6], player->info.uuid[7], player->info.uuid[8], player->info.uuid[9],
               player->info.uuid[10], player->info.uuid[11], player->info.uuid[12], player->info.uuid[13], player->info.uuid[14],
               player->info.uuid[15] );

     D_INFO( "Running player '%s' with UUID %s!\n", player->info.name, buf );

     /* Start messaging thread */
     player->thread = direct_thread_create( DTT_DEFAULT, player_main_loop, player, "Voodoo/Player" );

     /* Return the new player. */
     *ret_player = player;

     return DR_OK;
}

DirectResult
voodoo_player_destroy( VoodooPlayer *player )
{
     D_MAGIC_ASSERT( player, VoodooPlayer );

     player->quit = true;

     direct_thread_join( player->thread );
     direct_thread_destroy( player->thread );

     close( player->fd );

     pthread_mutex_destroy( &player->lock );

     D_MAGIC_CLEAR( player );

     D_FREE( player );

     return DR_OK;
}

DirectResult
voodoo_player_broadcast( VoodooPlayer *player )
{
#if !VOODOO_PLAY_FAKE
     int           ret;
#ifdef MACOS
     char          *ptr, lastname[IFNAMSIZ];
#else
     int           i;
#endif
     struct ifreq  req[16];
     struct ifconf conf;

     D_MAGIC_ASSERT( player, VoodooPlayer );

     conf.ifc_buf = (char*) req;
     conf.ifc_len = sizeof(req);

     ret = ioctl( player->fd, SIOCGIFCONF, &conf );
     if (ret) {
          D_PERROR( "Voodoo/Player: ioctl( SIOCGIFCONF ) failed!\n" );
          return DR_FAILURE;
     }

#ifdef MACOS
     // TIV: On iPhone (and I believe in general on BSD, you can't just plainly iterate on struct size)
     
     lastname[0] = 0;
     
     for (ptr = conf.ifc_buf; ptr < conf.ifc_buf + conf.ifc_len; )
     {
         char                 buf[100];
         int                  len, flags;
         struct ifreq         ifrcopy, *ifr  = (struct ifreq *)ptr;
         struct sockaddr_in  *addr = (struct sockaddr_in*) &ifr->ifr_broadaddr;

         len = max(sizeof(struct sockaddr), ifr->ifr_addr.sa_len);
         ptr += sizeof(ifr->ifr_name) + len; // for next one in buffer
         
         if (strncmp(lastname, ifr->ifr_name, IFNAMSIZ) == 0)
         {
             continue; /* already processed this interface */
         }

         memcpy(lastname, ifr->ifr_name, IFNAMSIZ);

         ifrcopy = *ifr;
         ioctl( player->fd, SIOCGIFFLAGS, &ifrcopy);
         flags = ifrcopy.ifr_flags;
         if ((flags & IFF_UP) == 0)
         {
             D_INFO( "Voodoo/Player:   %-16s is not up.\n", ifrcopy.ifr_name );
             continue;	// ignore if interface not up
         }

         ret = ioctl( player->fd, SIOCGIFBRDADDR, ifr );
         if (ret) {
             D_PERROR( "Voodoo/Player: ioctl( SIOCGIFBRDADDR ) %-16s failed!\n", ifr->ifr_name );
             continue;
         }
         
         if (addr->sin_addr.s_addr) {
              inet_ntop( AF_INET, &addr->sin_addr, buf, sizeof(buf) );

              D_INFO( "Voodoo/Player:   %-16s (%s)\n", ifr->ifr_name, buf );
         }
         else {
              ret = ioctl( player->fd, SIOCGIFDSTADDR, ifr );
              if (ret) {
                   D_PERROR( "Voodoo/Player: ioctl( SIOCGIFDSTADDR ) failed!\n" );
                   continue;
              }

              inet_ntop( AF_INET, &addr->sin_addr, buf, sizeof(buf) );

              D_INFO( "Voodoo/Player:   %-16s (%s) (P-t-P)\n", ifr->ifr_name, buf );
         }
         
         player_send_info( player, &addr->sin_addr.s_addr, true );
     }
#else
     D_INFO( "Voodoo/Player: Detected %d interfaces\n", conf.ifc_len/sizeof(req[0]) );

     for (i=0; i<conf.ifc_len/sizeof(req[0]); i++) {
          struct sockaddr_in *addr = (struct sockaddr_in*) &req[i].ifr_broadaddr;
          char                buf[100];

          ret = ioctl( player->fd, SIOCGIFBRDADDR, &req[i] );
          if (ret) {
               D_PERROR( "Voodoo/Player: ioctl( SIOCGIFBRDADDR ) failed!\n" );
               continue;
          }

          if (addr->sin_addr.s_addr) {
               inet_ntop( AF_INET, &addr->sin_addr, buf, sizeof(buf) );

               D_INFO( "Voodoo/Player:   %-16s (%s)\n", req[i].ifr_name, buf );
          }
          else {
               ret = ioctl( player->fd, SIOCGIFDSTADDR, &req[i] );
               if (ret) {
                    D_PERROR( "Voodoo/Player: ioctl( SIOCGIFDSTADDR ) failed!\n" );
                    continue;
               }

               inet_ntop( AF_INET, &addr->sin_addr, buf, sizeof(buf) );

               D_INFO( "Voodoo/Player:   %-16s (%s) (P-t-P)\n", req[i].ifr_name, buf );
          }

          //addr->sin_addr.s_addr = inet_addr( "192.168.1.150" );
          //addr->sin_addr.s_addr = inet_addr( "192.168.255.255" );

          player_send_info( player, &addr->sin_addr.s_addr, true );
     }
#endif
#endif

     return DR_OK;
}

DirectResult
voodoo_player_lookup( VoodooPlayer   *player,
                      const u8        uuid[16],
                      VoodooPlayInfo *ret_info,
                      char           *ret_addr,
                      int             max_addr )
{
     PlayerNode *node;

     D_MAGIC_ASSERT( player, VoodooPlayer );

     pthread_mutex_lock( &player->lock );

     direct_list_foreach (node, player->nodes) {
          if (!uuid || !memcmp( node->info.uuid, uuid, 16 )) {
               if (ret_info)
                    direct_memcpy( ret_info, &node->info, sizeof(VoodooPlayInfo) );

               if (ret_addr)
                    direct_snputs( ret_addr, node->addr, max_addr );

               pthread_mutex_unlock( &player->lock );
               return DR_OK;
          }
     }

     if (uuid && !memcmp( player->info.uuid, uuid, 16 )) {
          if (ret_info)
               direct_memcpy( ret_info, &player->info, sizeof(VoodooPlayInfo) );

          if (ret_addr)
               direct_snputs( ret_addr, "127.0.0.1", max_addr );

          pthread_mutex_unlock( &player->lock );
          return DR_OK;
     }

     pthread_mutex_unlock( &player->lock );

     return DR_ITEMNOTFOUND;
}

DirectResult
voodoo_player_enumerate( VoodooPlayer          *player,
                         VoodooPlayerCallback   callback,
                         void                  *ctx )
{
     PlayerNode *node;
     long long   now = direct_clock_get_abs_millis();


     D_MAGIC_ASSERT( player, VoodooPlayer );

     pthread_mutex_lock( &player->lock );

     direct_list_foreach (node, player->nodes) {
          if (callback( ctx, &node->info, &node->version,
                        node->addr, now - node->last_seen ) == DENUM_CANCEL)
               break;
     }

     pthread_mutex_unlock( &player->lock );

     return DR_OK;
}

/**********************************************************************************************************************/

__attribute__((unused))
static void
player_send_info( VoodooPlayer    *player,
                  const in_addr_t *in_addr,
                  bool             discover )
{
     int                 ret;
     struct sockaddr_in  addr;
     VoodooPlayMessage   msg;
     PlayerNode         *node;

     D_MAGIC_ASSERT( player, VoodooPlayer );

     msg.version = player->version;
     msg.type    = discover ? VPMT_DISCOVER : VPMT_SENDINFO;
     msg.info    = player->info;

     addr.sin_family      = AF_INET;
     addr.sin_addr.s_addr = *in_addr;
     addr.sin_port        = htons( 2323 );

     ret = sendto( player->fd, &msg, sizeof(msg), 0, (struct sockaddr*) &addr, sizeof(addr) );
     if (ret < 0) {
          D_PERROR( "Voodoo/Player: sendto() failed!\n" );
          return;
     }

     if (!discover && voodoo_config->forward_nodes) {
          direct_list_foreach (node, player->nodes) {
               VoodooPlayInfo info = node->info;
     
               info.flags |= VPIF_LEVEL2;
     
               msg.version = node->version;
               msg.type    = discover ? VPMT_DISCOVER : VPMT_SENDINFO;
               msg.info    = info;
     
               ret = sendto( player->fd, &msg, sizeof(msg), 0, (struct sockaddr*) &addr, sizeof(addr) );
               if (ret < 0) {
                    D_PERROR( "Voodoo/Player: sendto() failed!\n" );
                    return;
               }
          }
     }
}

static void
player_save_info( VoodooPlayer            *player,
                  const VoodooPlayMessage *msg,
                  const char              *addr )
{
     PlayerNode *node;

     D_MAGIC_ASSERT( player, VoodooPlayer );

     direct_list_foreach (node, player->nodes) {
          if (!memcmp( node->info.uuid, msg->info.uuid, 16 )) {
               if (msg->info.flags & VPIF_LEVEL2 && !(node->info.flags & VPIF_LEVEL2)) {
                    node->version = msg->version;
                    node->info    = msg->info;

                    direct_snputs( node->addr, addr, sizeof(node->addr) );
               }

               node->last_seen = direct_clock_get_abs_millis();

               return;
          }
     }

     node = D_CALLOC( 1, sizeof(PlayerNode) );
     if (!node) {
          D_OOM();
          return;
     }

     node->version   = msg->version;
     node->info      = msg->info;

     node->last_seen = direct_clock_get_abs_millis();

     direct_snputs( node->addr, addr, sizeof(node->addr) );


     direct_list_append( &player->nodes, &node->link );
}

#if !VOODOO_PLAY_FAKE
static void *
player_main_loop( DirectThread *thread, void *arg )
{
     VoodooPlayer       *player = arg;
     int                 ret;
     struct sockaddr_in  addr;
     socklen_t           addr_len = sizeof(addr);
     VoodooPlayMessage   msg;
     char                buf[100];

     D_MAGIC_ASSERT( player, VoodooPlayer );

     while (!player->quit) {
          struct pollfd  pf;

          pf.fd     = player->fd;
          pf.events = POLLIN;

          switch (poll( &pf, 1, 100 )) {
               default:
                    ret = recvfrom( player->fd, &msg, sizeof(msg), 0, (struct sockaddr*) &addr, &addr_len );
                    if (ret < 0) {
                         D_PERROR( "Voodoo/Player: recvfrom() failed!\n" );
                         usleep( 500000 );
                         continue;
                    }

                    inet_ntop( AF_INET, &addr.sin_addr, buf, sizeof(buf) );

                    pthread_mutex_lock( &player->lock );

                    /* Send reply if message is not from ourself */
                    if (memcmp( msg.info.uuid, player->info.uuid, 16 )) {
                         switch (msg.type) {
                              case VPMT_DISCOVER:
                                   D_INFO( "Voodoo/Player: Received DISCOVER from '%s' (%s)\n", msg.info.name, buf );
                                   player_send_info( player, &addr.sin_addr.s_addr, false );
                                   break;

                              case VPMT_SENDINFO:
                                   D_INFO( "Voodoo/Player: Received SENDINFO from '%s' (%s)\n", msg.info.name, buf );
                                   player_save_info( player, &msg, buf );
                                   break;

                              default:
                                   D_ERROR( "Voodoo/Player: Received unknown message (%s)\n", buf );
                                   break;
                         }
                    }
                    else
                         D_INFO( "Voodoo/Player: Received message from ourself (%s)\n", buf );

                    pthread_mutex_unlock( &player->lock );
                    break;

               case 0:
                    D_DEBUG( "Voodoo/Player: Timeout during poll()\n" );
                    break;

               case -1:
                    if (errno != EINTR) {
                         D_PERROR( "Voodoo/Player: Could not poll() the socket!\n" );
                         player->quit = true;
                    }
                    break;
          }
     }

     return DR_OK;
}
#else

static DirectResult
send_discover_and_receive_info( int                fd,
                                VoodooPlayVersion *ret_version,
                                VoodooPlayInfo    *ret_info )
{
     int                 ret;
     VoodooMessageHeader header;

     D_INFO( "Voodoo/Player: Sending VMSG_DISCOVER message via Voodoo TCP port...\n" );

     header.size   = sizeof(VoodooMessageHeader);
     header.serial = 0;
     header.type   = VMSG_DISCOVER;

     ret = write( fd, &header, sizeof(header) );
     if (ret < 0) {
          ret = errno2result( errno );
          D_PERROR( "Voodoo/Player: Failed to send VMSG_DISCOVER message via Voodoo TCP port!\n" );
          return ret;
     }



     struct pollfd pfd;

     pfd.events = POLL_IN;
     pfd.fd     = fd;

     // wait for up to one second (old server will not reply anything, so we have to timeout)
     ret = poll( &pfd, 1, 1000 );
     if (ret < 0) {
          ret = errno2result( errno );
          D_PERROR( "Voodoo/Player: Failed to wait for reply after sending VMSG_DISCOVER message via Voodoo TCP port!\n" );
          return ret;
     }

     if (ret == 0) {
          D_INFO( "Voodoo/Player: Old Voodoo Server without VMSG_DISCOVER support (timeout waiting for reply)\n" );
          return DR_UNSUPPORTED;
     }

     D_INFO( "Voodoo/Player: New Voodoo Server with VMSG_DISCOVER support, reading version/info (SENDINFO) reply...\n" );


     struct {
          VoodooMessageHeader header;
          VoodooPlayVersion   version;
          VoodooPlayInfo      info;
     } msg;

     size_t got = 0;

     while (got < sizeof(msg)) {
          ret = read( fd, (void*) &msg + got, sizeof(msg) - got );
          if (ret < 0) {
               ret = errno2result( errno );
               D_PERROR( "Voodoo/Player: Failed to read after sending VMSG_DISCOVER message via Voodoo TCP port!\n" );
               return ret;
          }

          got += ret;
     }


     if (msg.header.type != VMSG_SENDINFO) {
          D_ERROR( "Voodoo/Player: Received message after sending VMSG_DISCOVER message via Voodoo TCP port is no VMSG_SENDINFO!\n");
          return DR_INVARG;
     }

     *ret_version = msg.version;
     *ret_info    = msg.info;

     D_INFO( "Voodoo/Player: Voodoo Server sent name '%s', version %d.%d.%d\n",
             msg.info.name, msg.version.v[1], msg.version.v[2], msg.version.v[3] );

     return DR_OK;
}

static void
player_try_connect( VoodooPlayer *player,
                    u32           addr )
{
     DirectResult   ret;
     int            fd, err;
     struct in_addr sin_addr = { addr };

     char buf[100];

     inet_ntop( AF_INET, &sin_addr, buf, sizeof(buf) );


     /* Create the client socket. */
     fd = socket( AF_INET, SOCK_STREAM, 0 );
     if (fd < 0) {
          D_PERROR( "Voodoo/Player: Could not create the socket via socket( %d )!\n", AF_INET );
          return;
     }

     struct sockaddr_in sock_addr;

     sock_addr.sin_family = AF_INET;
     sock_addr.sin_port   = htons( 2323 );
     sock_addr.sin_addr   = sin_addr;

     /* Connect to the server. */
     err = connect( fd, (struct sockaddr*) &sock_addr, sizeof(sock_addr) );
     if (err) {
          D_PERROR( "Voodoo/Player: No Voodoo at '%s:2323'", buf );
          close( fd );
          return;
     }

     D_INFO( "Voodoo/Player: Found Voodoo at '%s'!\n", buf );


     VoodooPlayMessage msg;


     ret = send_discover_and_receive_info( fd, &msg.version, &msg.info );
     if (ret) {
          /* Fill version struct */
          msg.version.v[0] = VPVF_LITTLE_ENDIAN | VPVF_32BIT_SERIALS;
          msg.version.v[1] = DIRECTFB_MAJOR_VERSION;
          msg.version.v[2] = DIRECTFB_MINOR_VERSION;
          msg.version.v[3] = DIRECTFB_MICRO_VERSION;

          msg.type = VPMT_SENDINFO;

          /* Fill info struct */
          direct_snputs( msg.info.name,   "Unknown", VOODOO_PLAYER_NAME_LENGTH );
          direct_snputs( msg.info.vendor, "Unknown", VOODOO_PLAYER_VENDOR_LENGTH );
          direct_snputs( msg.info.model,  "Unknown", VOODOO_PLAYER_MODEL_LENGTH );
          generate_uuid( msg.info.uuid );
     }


     close( fd );


     pthread_mutex_lock( &player->lock );

     player_save_info( player, &msg, buf );

     pthread_mutex_unlock( &player->lock );
}

typedef struct {
     VoodooPlayer *player;
     u32           addr;
} PlayerTryContext;

static void *
player_try_thread( void *arg )
{
     PlayerTryContext *context = arg;

     player_try_connect( context->player, context->addr );

     D_FREE( context );

     return NULL;
}

static void *
player_main_loop( DirectThread *thread, void *arg )
{
     VoodooPlayer  *player = arg;
     int            ret;
     int            i;
     struct ifreq   req[16];
     struct ifconf  conf;

     D_MAGIC_ASSERT( player, VoodooPlayer );

//     while (!player->quit) {
          conf.ifc_buf = (char*) req;
          conf.ifc_len = sizeof(req);

          ret = ioctl( player->fd, SIOCGIFCONF, &conf );
          if (ret) {
               D_PERROR( "Voodoo/Player: ioctl( SIOCGIFCONF ) failed!\n" );
               return NULL;
          }

          D_INFO( "Voodoo/Player: Detected %d interfaces\n", conf.ifc_len/sizeof(req[0]) );

          for (i=0; i<conf.ifc_len/sizeof(req[0]); i++) {
               struct sockaddr_in *addr = (struct sockaddr_in*) &req[i].ifr_broadaddr;
               char                buf[100];

               ret = ioctl( player->fd, SIOCGIFBRDADDR, &req[i] );
               if (ret) {
                    D_PERROR( "Voodoo/Player: ioctl( SIOCGIFBRDADDR ) failed!\n" );
                    continue;
               }

               if (addr->sin_addr.s_addr) {
                    inet_ntop( AF_INET, &addr->sin_addr, buf, sizeof(buf) );

                    D_INFO( "Voodoo/Player:   %-16s (%s)  [0x%08x]\n", req[i].ifr_name, buf, addr->sin_addr.s_addr );

                    u32 _addr = htonl( addr->sin_addr.s_addr );
                    u32 a;

                    for (a = (_addr & ~0xff) + 1; a < (_addr | 0xff); a++) {
                         if (a == _addr)
                              continue;

                         PlayerTryContext *context = D_CALLOC( 1, sizeof(PlayerTryContext) );

                         context->player = player;
                         context->addr   = ntohl(a);


                         pthread_t t;

                         pthread_create( &t, NULL, player_try_thread, context );
                    }
               }
               else {
                    ret = ioctl( player->fd, SIOCGIFDSTADDR, &req[i] );
                    if (ret) {
                         D_PERROR( "Voodoo/Player: ioctl( SIOCGIFDSTADDR ) failed!\n" );
                         continue;
                    }

                    inet_ntop( AF_INET, &addr->sin_addr, buf, sizeof(buf) );

                    D_INFO( "Voodoo/Player:   %-16s (%s) (P-t-P)\n", req[i].ifr_name, buf );
               }
          }
//     }

     return DR_OK;
}
#endif

