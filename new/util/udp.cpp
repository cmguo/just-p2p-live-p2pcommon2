
#include "StdAfx.h"

#include "udp.h"
#include <assert.h>

//using namespace std;


// #ifndef _PPL_USE_ASIO
// 
// #define EWOULDBLOCK             WSAEWOULDBLOCK
// #define EINPROGRESS             WSAEINPROGRESS
// #define EALREADY                WSAEALREADY
// #define ENOTSOCK                WSAENOTSOCK
// #define EDESTADDRREQ            WSAEDESTADDRREQ
// #define EMSGSIZE                WSAEMSGSIZE
// #define EPROTOTYPE              WSAEPROTOTYPE
// #define ENOPROTOOPT             WSAENOPROTOOPT
// #define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
// #define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
// #define EOPNOTSUPP              WSAEOPNOTSUPP
// #define EPFNOSUPPORT            WSAEPFNOSUPPORT
// #define EAFNOSUPPORT            WSAEAFNOSUPPORT
// #define EADDRINUSE              WSAEADDRINUSE
// #define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
// #define ENETDOWN                WSAENETDOWN
// #define ENETUNREACH             WSAENETUNREACH
// #define ENETRESET               WSAENETRESET
// #define ECONNABORTED            WSAECONNABORTED
// #define ECONNRESET              WSAECONNRESET
// #define ENOBUFS                 WSAENOBUFS
// #define EISCONN                 WSAEISCONN
// #define ENOTCONN                WSAENOTCONN
// #define ESHUTDOWN               WSAESHUTDOWN
// #define ETOOMANYREFS            WSAETOOMANYREFS
// #define ETIMEDOUT               WSAETIMEDOUT
// #define ECONNREFUSED            WSAECONNREFUSED
// #define ELOOP                   WSAELOOP
// #define EHOSTUNREACH            WSAEHOSTUNREACH
// #define EPROCLIM                WSAEPROCLIM
// #define EUSERS                  WSAEUSERS
// #define EDQUOT                  WSAEDQUOT
// #define ESTALE                  WSAESTALE
// #define EREMOTE                 WSAEREMOTE
// 


//#define EHOSTDOWN               WSAEHOSTDOWN
// #endif

Socket
openPort( unsigned short port, unsigned int interfaceIp, bool verbose )
{
   Socket fd;
    
   fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if ( fd == INVALID_SOCKET )
   {
      //int err = getErrno();
      //cerr << "Could not create a UDP socket:" << err << endl;
      return INVALID_SOCKET;
   }
    
   struct sockaddr_in addr;
   memset((char*) &(addr),0, sizeof((addr)));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   addr.sin_port = htons(port);
    
   if ( (interfaceIp != 0) && 
        ( interfaceIp != 0x100007f ) )
   {
      addr.sin_addr.s_addr = htonl(interfaceIp);
      if (verbose )
      {
         //Myclog << "Binding to interface "               << hex << "0x" << htonl(interfaceIp) << dec << endl;
      }
   }
	
   if ( ::bind( fd,(struct sockaddr*)&addr, sizeof(addr)) != 0 )
   {
      int e = getErrno();
        
      switch (e)
      {
         case 0:
         {
            //cerr << "Could not bind socket" << endl;
            return INVALID_SOCKET;
         }
         case EADDRINUSE:
         {
            //cerr << "Port " << port << " for receiving UDP is in use" << endl;
            return INVALID_SOCKET;
         }
         break;
         case EADDRNOTAVAIL:
         {
            if ( verbose ) 
            {
               //cerr << "Cannot assign requested address" << endl;
            }
            return INVALID_SOCKET;
         }
         break;
         default:
         {
            //cerr << "Could not bind UDP receive port"                 << "Error=" << e << " " << strerror(e) << endl;
            return INVALID_SOCKET;
         }
         break;
      }
   }
   if ( verbose )
   {
      //Myclog << "Opened port " << port << " with fd " << fd << endl;
   }
   
   LIVE_ASSERT( fd != INVALID_SOCKET  );
	
   return fd;
}


bool 
getMessage( Socket fd, char* buf, int* len,
            unsigned int* srcIp, unsigned short* srcPort,
            bool verbose)
{
   LIVE_ASSERT( fd != INVALID_SOCKET );
	
   int originalSize = *len;
   LIVE_ASSERT( originalSize > 0 );
   
   struct sockaddr_in from;
   int fromLen = sizeof(from);
	
   *len = recvfrom(fd,
                   buf,
                   originalSize,
                   0,
                   (struct sockaddr *)&from,
                   (socklen_t*)&fromLen);
	
   if ( *len == SOCKET_ERROR )
   {
      int err = getErrno();
		
      switch (err)
      {
         case ENOTSOCK:
            //cerr << "Error fd not a socket" <<   endl;
            break;
         case ECONNRESET:
            //cerr << "Error connection reset - host not reachable" <<   endl;
            break;
				
         default:
            //cerr << "Socket Error=" << err << endl;
             break;
      }
		
      return false;
   }
	
   if ( *len < 0 )
   {
      //Myclog << "socket closed? negative len" << endl;
      return false;
   }
    
   if ( *len == 0 )
   {
      //Myclog << "socket closed? zero len" << endl;
      return false;
   }
    
   *srcPort = ntohs(from.sin_port);
   *srcIp = ntohl(from.sin_addr.s_addr);
	
   if ( (*len)+1 >= originalSize )
   {
      if (verbose)
      {
         //Myclog << "Received a message that was too large" << endl;
      }
      return false;
   }
   buf[*len]=0;
    
   return true;
}


bool 
sendMessage( Socket fd, char* buf, int l, 
             unsigned int dstIp, unsigned short dstPort,
             bool verbose)
{
   LIVE_ASSERT( fd != INVALID_SOCKET );
    
   int s;
   if ( dstPort == 0 )
   {
      // sending on a connected port 
      LIVE_ASSERT( dstIp == 0 );
		
      s = send(fd,buf,l,0);
   }
   else
   {
      LIVE_ASSERT( dstIp != 0 );
      LIVE_ASSERT( dstPort != 0 );
        
      struct sockaddr_in to;
      int toLen = sizeof(to);
      memset(&to,0,toLen);
        
      to.sin_family = AF_INET;
      to.sin_port = htons(dstPort);
      to.sin_addr.s_addr = htonl(dstIp);
        
      s = sendto(fd, buf, l, 0,(sockaddr*)&to, toLen);
   }
    
   if ( s == SOCKET_ERROR )
   {
      int e = getErrno();
      switch (e)
      {
         case ECONNREFUSED:
//         case EHOSTDOWN:
         case EHOSTUNREACH:
         {
            // quietly ignore this 
         }
         break;
         case EAFNOSUPPORT:
         {
            //cerr << "err EAFNOSUPPORT in send" << endl;
         }
         break;
         default:
         {
            //cerr << "err " << e << " "  << strerror(e) << " in send" << endl;
         }
      }
      return false;
   }
    
   if ( s == 0 )
   {
      //cerr << "no data sent in send" << endl;
      return false;
   }
    
   if ( s != l )
   {
      if (verbose)
      {
         //cerr << "only " << s << " out of " << l << " bytes sent" << endl;
      }
      return false;
   }
    
   return true;
}


void
initNetwork()
{
#ifdef WIN32 
   WORD wVersionRequested = MAKEWORD( 2, 2 );
   WSADATA wsaData;
   int err;
	
   err = WSAStartup( wVersionRequested, &wsaData );
   if ( err != 0 ) 
   {
      // could not find a usable WinSock DLL
      //cerr << "Could not load winsock" << endl;
      LIVE_ASSERT(0); // is this is failing, try a different version that 2.2, 1.0 or later will likely work 
      //exit(1);
   }
    
   /* Confirm that the WinSock DLL supports 2.2.*/
   /* Note that if the DLL supports versions greater    */
   /* than 2.2 in addition to 2.2, it will still return */
   /* 2.2 in wVersion since that is the version we      */
   /* requested.                                        */
    
   if ( LOBYTE( wsaData.wVersion ) != 2 ||
        HIBYTE( wsaData.wVersion ) != 2 ) 
   {
      /* Tell the user that we could not find a usable */
      /* WinSock DLL.                                  */
      WSACleanup( );
      //cerr << "Bad winsock verion" << endl;
      LIVE_ASSERT(0); // is this is failing, try a different version that 2.2, 1.0 or later will likely work 
      //exit(1);
   }    
#endif
}


/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */

// Local Variables:
// mode:c++
// c-file-style:"ellemtel"
// c-file-offsets:((case-label . +))
// indent-tabs-mode:nil
// End:
