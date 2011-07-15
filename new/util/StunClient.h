
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
    // ��ȡnat������
    
    MY_STUN_NAT_TYPE    StartGetNatTpye();
    void Stop();

	static UINT32 GetLocalFirstIP(void);

private:
    MY_STUN_NAT_TYPE	getNatType(char *pcServer);
    //�ж��Ƿ���Ҫʹ��stunЭ����NAT��Ϣ��ͬʱ�������nat��Ϣ�����������
    //bool                IsNeedToUpdateNat(MY_STUN_NAT_TYPE &snt_result, IniFile& ini);


private:
    Socket m_Socket1;
    Socket m_Socket2;
	std::string m_strConfig;
};

#endif