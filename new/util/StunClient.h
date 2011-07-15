
#ifndef _LIVE_P2PCOMMON2_NEW_UTIL_STUN_CLIENT_H_
#define _LIVE_P2PCOMMON2_NEW_UTIL_STUN_CLIENT_H_

//StunClient.h

#include "udp.h"
#include "stun.h"
#include <synacast/protocol/nat.h>
#include <string>


class CStunClient
{
public:
    CStunClient();

public:
    // 获取nat的类型
    
    MY_STUN_NAT_TYPE    StartGetNatTpye();
    void Stop();

	static UINT32 GetLocalFirstIP(void);

private:
    MY_STUN_NAT_TYPE	getNatType(char *pcServer);
    //判断是否需要使用stun协议检测NAT信息，同时将保存的nat信息输出到参数中
    //bool                IsNeedToUpdateNat(MY_STUN_NAT_TYPE &snt_result, IniFile& ini);


private:
    Socket m_Socket1;
    Socket m_Socket2;
	std::string m_strConfig;
};

#endif