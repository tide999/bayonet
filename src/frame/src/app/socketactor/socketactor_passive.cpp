/*=============================================================================
#  Author:          dantezhu - http://www.vimer.cn
#  Email:           zny2008@gmail.com
#  FileName:        socketactor_passive.cpp
#  Description:     
#  Version:         1.0
#  LastChange:      2011-01-28 17:08:34
#  History:         
=============================================================================*/
#include "socketactor_passive.h"
#include "appactor_base.h"
namespace bayonet {

CSocketActorPassive::~CSocketActorPassive ()
{}

int CSocketActorPassive::OnWaitRecv()
{
    ClearFsmNodes();
    return CSocketActorData::OnWaitRecv();
}
int CSocketActorPassive::OnSendOver()
{
    NotifyAppActor();//通知上层状态机可以FINI了

    ResetStatusData();
    if (m_bKeepcnt && m_ProtoType == PROTO_TYPE_TCP)
    {
        return SOCKET_FSM_WAITRECV;
    }
    else
    {
        return SOCKET_FSM_CLOSING;
    }
}
void CSocketActorPassive::NotifyAppActor()
{
    if (m_pAppActorProxy.true_ptr())
    {
        m_pAppActorProxy.true_ptr()->ChangeState(APP_FSM_FINI);
    }
    /*
    //这种情况是有可能的，当是长连接的时候
    else
    {
        error_log("[class:%s]m_pAppActor is NULL",Name().c_str());
    }
    */
}
int CSocketActorPassive::OnCloseOver()
{
    NotifyAppActor();//通知上层状态机可以FINI了
    return CSocketActorData::OnCloseOver();
}
//=============================================================================
CSocketActorPassiveTcp::~CSocketActorPassiveTcp () {}
bool CSocketActorPassiveTcp::IsTimeOut()
{
    if (m_bKeepcnt)
    {
        //只要收到数据，那就不算超时
        if (m_recvFlag != 0)
        {
            return false;
        }

        if (m_TimeoutMs < 0)
        {
            //永不超时
            return false;
        }

        //是说空闲时间的多少
        int pastTime = m_idleTimer.GetPastTime();

        if (pastTime > m_TimeoutMs)
        {
            return true;
        }
        else
        {
            return false;
        }
    }   
    else
    {
        return CSocketActorPassive::IsTimeOut();
    }
}
int CSocketActorPassiveTcp::OnInitOver()
{
    return SOCKET_FSM_WAITRECV;
}
int CSocketActorPassiveTcp::OnWaitRecv()
{
    //空闲即时开始
    m_idleTimer.Start();
    return CSocketActorPassive::OnWaitRecv();
}
int CSocketActorPassiveTcp::OnRecvOver()
{
    //TODO
    //这个时候,应该是要等数据处理的阶段
    return SOCKET_FSM_WAITCLOSE;
}
//=============================================================================
CSocketActorPassiveUdp::~CSocketActorPassiveUdp () {}
int CSocketActorPassiveUdp::OnInitOver()
{
    //按理说应该是返回close，等待上层唤醒
    //return SOCKET_FSM_WAITSEND;
    return SOCKET_FSM_WAITCLOSE;
}
int CSocketActorPassiveUdp::OnRecvOver()
{
    //不会走到这一步
    return SOCKET_FSM_WAITSEND;
}
}
