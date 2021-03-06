/*=============================================================================
#  Author:          dantezhu - http://www.vimer.cn
#  Email:           zny2008@gmail.com
#  FileName:        socketactor_base.cpp
#  Description:     
#  Version:         1.0
#  LastChange:      2011-01-24 20:34:16
#  History:         
=============================================================================*/
#include "socketactor_base.h"
#include "byt_frame.h"
namespace bayonet {
CSocketActorBase::CSocketActorBase () 
{
    m_SocketFd = -1;
    m_Port = 0;
    m_TimeoutMs = -1;
    m_ProtoType = 0;
    m_pAction = NULL;
    m_bKeepcnt = false;
}
CSocketActorBase::~CSocketActorBase () {}

int CSocketActorBase::HandleEvent(IEvent* pEvent)
{
    CEpollEvent* pEpollEvent = (CEpollEvent*)pEvent;
    unsigned evt = pEpollEvent->evt;
    int ret = 0;
    if ( evt&EPOLLIN )
    {
        ret = ChangeState(SOCKET_FSM_RECVING);                         
    }
    else if ( evt&EPOLLOUT )
    {
        ret = ChangeState(SOCKET_FSM_SENDING);
    }
    else if ( evt&EPOLLHUP )
    {
        ret = ChangeState(SOCKET_FSM_CLOSING);
    }
    else if ( evt&EPOLLERR )
    {
        ret = ChangeState(SOCKET_FSM_ERROR);
    }
    else
    {
        ret = ChangeState(SOCKET_FSM_ERROR);
    }
    return ret;
}
void CSocketActorBase::SetAppActor(CActorBase* pActor)
{
    if (pActor)
    {
        m_pAppActorProxy = pActor->get_ptr_proxy();
    }
}

int CSocketActorBase::Init(string ip,int port,int timeout_ms,int protoType)
{
    m_IP = ip;
    m_Port = port;
    m_TimeoutMs = timeout_ms;
    m_ProtoType = protoType;
    return 0;
}
int CSocketActorBase::Init(int socketFd,int timeout_ms,int protoType)
{
    m_SocketFd = socketFd;
    m_TimeoutMs = timeout_ms;
    m_ProtoType = protoType;
    return 0;
}
void CSocketActorBase::SetIActionPtr(IAction *pAction)
{
    m_pAction = pAction;
}

int CSocketActorBase::SetEvent(unsigned event)
{
    CEPoller * pEpoller = GetEpoller();
    if (!pEpoller) 
    {
        byt_error_log("[class:%s]pEpoller is NULL",Name().c_str());
        return -1;
    }

    /*if ( pEpoller->ModEpollIO(m_SocketFd,event) < 0 )
    {
        return pEpoller->AddEpollIO(m_SocketFd,event);
    }*/

    return pEpoller->SetEpollIO(m_SocketFd,event);
}
int CSocketActorBase::GetSocketFd()
{
    return m_SocketFd;
}
void CSocketActorBase::SetKeepcnt(bool bKeepcnt)
{
    m_bKeepcnt = bKeepcnt;
}

void CSocketActorBase::SetTimeOutMs(int timeout_ms)
{
    m_TimeoutMs = timeout_ms;
}

int CSocketActorBase::CheckTimeOut()
{
    if (GetGCMark())
    {
        return 0;
    }
    //默认是永不超时的
    if (IsTimeOut())
    {
        ChangeState(SOCKET_FSM_TIMEOUT);
    }
    return 0;
}
bool CSocketActorBase::IsTimeOut()
{
    if (m_TimeoutMs < 0)
    {
        //永不超时
        return false;
    }

    /*
    //这个只是针对passive，所以不需要写在这里了
    if (m_ProtoType == PROTO_TYPE_TCP && m_bKeepcnt)
    {
        //长连接
        return false;
    }
    */

    int pastTime = m_aliveTimer.GetPastTime();

    if (pastTime > m_TimeoutMs)
    {
        return true;
    }
    else
    {
        return false;
    }
}
int CSocketActorBase::OnTimeout()
{
    return SOCKET_FSM_CLOSING;
}

int CSocketActorBase::OnError()
{
    return SOCKET_FSM_CLOSING;
}

int CSocketActorBase::OnWaitRecv() {
    SetEvent(EPOLLIN|EPOLLHUP|EPOLLERR);
    return 0;
}

int CSocketActorBase::OnWaitSend() {
    SetEvent(EPOLLOUT|EPOLLHUP|EPOLLERR);
    return 0;
}

int CSocketActorBase::OnWaitClose() {
    SetEvent(EPOLLHUP|EPOLLERR);
    return 0;
}

int CSocketActorBase::OnFini()
{
    //标记为GC
    SetGCMark();
    return SOCKET_FSM_ALLOVER;
}

CEPoller* CSocketActorBase::GetEpoller()
{
    IFrame* pFrame = GetFrame();
    if (!pFrame)
    {
        return NULL;
    }
    CBytFrame* pBytFrame = (CBytFrame*)pFrame;
    CEPoller * pEpoller = pBytFrame->GetEpoller();
    return pEpoller;
}
int CSocketActorBase::DetachFromEpoller()
{
    CEPoller* pEpoller = GetEpoller();
    if (pEpoller)
    {
        pEpoller->DetachSocket(this);
        return 0;
    }
    return 1;
}
}
