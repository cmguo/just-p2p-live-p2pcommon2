
#include "StdAfx.h"

//StunClient.cpp
#include <cassert>
#include <cstring>
//#include <iostream>
#include <cstdlib>   

#ifdef WIN32
#include <time.h>
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <ppl/mswin/shell.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#endif



#include "StunClient.h"
//#include "func.h"





using namespace std;
#define MAX_NIC 3




CStunClient::CStunClient():
m_Socket1((Socket)-1),
m_Socket2((Socket)-1)
{

}

void CStunClient::Stop()
{
    if (m_Socket1 != -1)
    {
        closesocket(m_Socket1);
        m_Socket1 = (Socket)-1;
    }

    if (m_Socket2 != -1)
    {
        closesocket(m_Socket2);
        m_Socket2 = (Socket)-1;
    }
    
}
MY_STUN_NAT_TYPE CStunClient::getNatType(char *pcServer)
{
    assert( sizeof(UInt8 ) == 1 );
    assert( sizeof(UInt16) == 2 );
    assert( sizeof(UInt32) == 4 );

    MY_STUN_NAT_TYPE snt_tpye = STUN_TYPE_ERROR;

    initNetwork();

    bool verbose = false;

    StunAddress4 stunServerAddr;
    stunServerAddr.addr=0;

    int srcPort=0;
    StunAddress4 sAddr[MAX_NIC];
    
    int numNic=0;

    for ( int i=0; i<MAX_NIC; i++ )
    {
        sAddr[i].addr=0; 
        sAddr[i].port=0; 
        
    }


    if ( stunParseServerName( pcServer, stunServerAddr) != true )
    {        
        return snt_tpye;
    }
    if ( srcPort == 0 )
    {
        srcPort = stunRandomPort();
    }

    if ( numNic == 0 )
    {
        numNic = 1;
    }

    for ( int nic=0; nic<numNic; nic++ )
    {
        sAddr[nic].port=(UInt16)srcPort;
        if ( stunServerAddr.addr == 0 )
        {

            return snt_tpye;
        }

        bool presPort=false;
        bool hairpin=false;

        NatType stype = stunNatType( stunServerAddr, verbose, &presPort, &hairpin, srcPort, &sAddr[nic], &m_Socket1, &m_Socket2);          

        switch (stype)
        {
        case StunTypeFailure:            
            snt_tpye = STUN_TYPE_ERROR;
            break;
        case StunTypeUnknown:            
            snt_tpye = STUN_TYPE_ERROR;
            break;
        case StunTypeOpen:
            //cout << "Open";
            snt_tpye = STUN_TYPE_PUBLIC;
            break;        
        case StunTypeFirewall:
            //cout << "Firewall";
            snt_tpye = STUN_TYPE_ERROR;
            break;
        case StunTypeBlocked:
            snt_tpye = STUN_TYPE_ERROR;
            break;

        case StunTypeConeNat:
//            cout << "StunTypeConeNat";
            snt_tpye = STUN_TYPE_FULLCONENAT;
            break;

        case StunTypeRestrictedNat:
//            cout << "StunTypeRestrictedNat";
            snt_tpye = STUN_TYPE_IP_RESTRICTEDNAT;
            break;

        case StunTypePortRestrictedNat:
//            cout << "StunTypePortRestrictedNat";
            snt_tpye = STUN_TYPE_IP_PORT_RESTRICTEDNAT;
            break;

        case StunTypeSymNat:
//            cout << "StunTypeSymNat";
            snt_tpye = STUN_TYPE_SYMNAT;
            break;

        default:            
//			cout << "Unkown NAT type";
            snt_tpye = STUN_TYPE_ERROR;;
            break;
        }
//        cout << stype << endl;;

    } // end of for loop 


    return snt_tpye;
}




/*
//判断是否需要使用stun协议检测NAT信息，同时将保存的nat信息输出到snt_result
//需要更新NAT信息则返回true；否则返回false
需要更新NAT的条件为：
1.保存的nattype为error
2.获取的本地ip地址与保存的地址不同
3.保存信息已经过期(超过3天)
*/
//bool CStunClient::IsNeedToUpdateNat( MY_STUN_NAT_TYPE &snt_result, IniFile& ini)
//{
//    //初始化MeidaEngine的配置文件路径
//    int LastTime = -1;
//    UInt32 LastLocalIP = -1;
//
//    snt_result = (MY_STUN_NAT_TYPE)ini.GetInteger(nat_ini_key_type, STUN_TYPE_ERROR);
//	// if ( snt_result < TYPE_FULLCONENAT || snt_result > TYPE_PUBLIC)
//    if ( snt_result < STUN_TYPE_PUBLIC || snt_result > STUN_TYPE_SYMNAT)
//    {
//        return true;
//    }
//    
//    LastLocalIP = ini.GetInteger(nat_ini_key_ip, -1);
//	if ( -1 == LastLocalIP )
//		return true;
//    if ( LastLocalIP != GetLocalFirstIP() )
//    {
//        return true;
//    }
//
//    LastTime = ini.GetInteger(nat_ini_key_time, -1);
//	if ( -1 == LastTime )
//		return true;
//    SYSTEMTIME st;
//    GetLocalTime(&st);
//    //如果日期相差三天，则判断已经过期
//    if ( abs(st.wDay - LastTime) >= 3)
//    {
//        return true;
//    }
//    
//    return false;
//}

std::string g_strStunServer[] = 
{ "stun.ekiga.net",
"stun.fwdnet.net",
"stun.ideasip.com",
"stun01.sipphone.com",
"stun.xten.com",
"stunserver.org",
"stun.sipgate.net",
"60.28.216.165"
};

MY_STUN_NAT_TYPE CStunClient::StartGetNatTpye()
{
    MY_STUN_NAT_TYPE snt_ret = STUN_TYPE_ERROR;    

 //   if ( IsNeedToUpdateNat(snt_ret, ini) )
    {
        //默认的stunserver服务器地址
        string strStunServer = "60.28.216.165" ;

        //从设置的多个服务器中随机选择一个
        int ServerCounts = sizeof(g_strStunServer) / sizeof(std::string);
        srand( (unsigned)time(NULL) );
        if( ServerCounts > 0)
        {
            strStunServer = g_strStunServer[ rand() % ServerCounts];
        }

        //OutputDebugString(strStunServer.c_str());
        snt_ret = getNatType( (char*)strStunServer.c_str() );
		
        //TCHAR str[64];
        //_stprintf_s(str, _T("%d"), snt_ret);
        //////保存获取到的NAT信息
        //ini.SetInt(nat_ini_key_type, snt_ret);

        //SYSTEMTIME st;
        //GetLocalTime(&st);
        //////保存当天的时间
        ////_stprintf_s(str, _T("%d"), st.wDay);
        //ini.SetInt(nat_ini_key_time, st.wDay);

        //////保存获取的本地IP        
        //_stprintf_s(str, _T("%u"), GetLocalFirstIP());        
        //ini.SetString(nat_ini_key_ip, str);
    }
    
    return snt_ret;
}


UINT32 CStunClient::GetLocalFirstIP(void)
{

    //使用 ip helper函数
    DWORD nip = 0;    
#ifdef WIN32
    PMIB_IPADDRTABLE pIPAddrTable;
    DWORD dwSize = 0 , dwRetVal;

    pIPAddrTable = (MIB_IPADDRTABLE*) malloc( sizeof( MIB_IPADDRTABLE) );

    // Make an initial call to GetIpAddrTable to get the
    // necessary size into the dwSize variable
    if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) 
    {
        free( pIPAddrTable );
        pIPAddrTable = (MIB_IPADDRTABLE *) malloc ( dwSize );
    }

    // Make a second call to GetIpAddrTable to get the
    // actual data we want
    if ( (dwRetVal = GetIpAddrTable( pIPAddrTable, &dwSize, 0 )) == NO_ERROR ) 
    { 
        for( UINT32 i = 0; i < pIPAddrTable->dwNumEntries; i++)
        {
            if(pIPAddrTable->table[i].dwAddr != inet_addr("127.0.0.1") && pIPAddrTable->table[i].dwAddr != 0 )
            {
                nip = pIPAddrTable->table[i].dwAddr;
                break;
            }
        }        

    }

    free(pIPAddrTable);
#endif
    return nip;
}