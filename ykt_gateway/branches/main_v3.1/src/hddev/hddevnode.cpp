/**
 * V1.2 �޸����豸�ڵ� toplogic ��ʼ������
 */

#include "hddev/hddevnode.h"
#include "hddev/hdsvr.h"
#include "ksgateway.h"
#include "netutil.h"
#include "xutils/xstring.h"
#include "task_scheduler.h"
#include "impl/schdimpl.h"
#include "osutil.h"
#include "execimpl.h"
#include <ace/Test_and_Set.h>
#include <ace/Null_Mutex.h>
#include <ace/SOCK_Connector.h>
#include <ace/OS.h>
#include "pubfunc.h"
#include <curl/curl.h>

using namespace HDDEV;


// �Ի���豸��֧��

#ifdef KSG_HD_DEV_SUPPORT

// ע��CCU���������߳�
//KSG_ADD_SCHD_CLASS(KSG_SCHD_HD_TCP_SVR,HDCCUListenScheduler);
// ע�����豸���͹���
KSG_REG_FACTORY_INTERFACE(KSG_HD_DEV,KSGHDDevInterfaceFactory);

// ע�����豸����
KSG_REG_DEVICE_OBJECT(KSG_HD_DEV,KSG_HD_POS_DEV,HDPosDevice);
KSG_REG_DEVICE_OBJECT(KSG_HD_DEV,KSG_HD_SERVER_DEV,HDCCUDevice);
KSG_REG_DEVICE_OBJECT(KSG_HD_DEV,KSG_HD_ADD_POS_DEV,HDAddPosDevice);
KSG_REG_DEVICE_OBJECT(KSG_HD_DEV,KSG_HD_PENSTOCK,HDPenStockDevice);
KSG_REG_DEVICE_OBJECT(KSG_HD_DEV,KSG_JSB_DEV,HDKQDevice);
KSG_REG_DEVICE_OBJECT(KSG_HD_DEV,KSG_READER_DEV,HDReaderDevice);
KSG_REG_DEVICE_OBJECT(KSG_HD_DEV,KSG_HD_KQ_CCU,HDKQCCUDevice);
KSG_REG_DEVICE_OBJECT(KSG_HD_DEV,"9003",HDGCUDevice);


/// ע�����豸�ӿ�
/*
KSG_REG_DEV_INTERFACE(KSG_HD_DEV,TK_COLLSERIAL_TASK,IHDCollectSerial);
KSG_REG_DEV_INTERFACE(KSG_HD_DEV,TK_HEARTBEAT_TASK,IHDCollectHeardbeat);
KSG_REG_DEV_INTERFACE(KSG_HD_DEV,TK_ADD_BLACKCARD,IHDAddBlackCard);
KSG_REG_DEV_INTERFACE(KSG_HD_DEV,TK_DEL_BLACKCARD,IHDDelBlackCard);
KSG_REG_DEV_INTERFACE(KSG_HD_DEV,TK_SET_CARD_PRIVILEGE,IHDSetCardPrivileges);
KSG_REG_DEV_INTERFACE(KSG_HD_DEV,TK_SET_FEE_RATE,IHDSetFeeRate);
KSG_REG_DEV_INTERFACE(KSG_HD_DEV,TK_COLL_HIS_SERIAL,IHDCollPosHisSerial);
*/

#endif // KSG_HD_DEV_SUPPORT

namespace HDDEV
{
	std::string hd_ccu_ftp_user_name = "hdhdhdhd";
	std::string hd_ccu_ftp_pswd = "11111111";
	std::string hd_ccu_ftp_port = "21";
};

bool HDCCUDevice::Accept(BaseVisitor& guest,KSGDeviceNode* visitor)
{
	if(typeid(guest) == typeid(KSGTaskCollHeartBeat::CollHeartBeatVisitor))
		return true;
	if(typeid(guest) == typeid(KSGTaskCollDeviceState::CollDeviceState))
		return true;
	return false;
}

int HDCCUDevice::make_handler(KSGDeviceNode* node,ACE_HANDLE* handler)
{
	if(!node)
		return -1;
	std::string ip = node->GetDevAddr().GetConnect();
	int port = node->GetDevAddr().GetPort();
	ACE_INET_Addr addr(port,ip.c_str());
	ACE_SOCK_Connector conn;
	ACE_SOCK_Stream stream;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	int err_code;
	ACE_DEBUG((LM_TRACE,"��ʼ���ӻ��CCU��[%s][%s]",node->get_name().c_str(),ip.c_str()));
	if(conn.connect(stream,addr,&tv))
	{
		err_code = ACE_OS::last_error();
		// TODO: �������ӵĴ�����
		if(EWOULDBLOCK == err_code)
		{
			ACE_DEBUG((LM_ERROR,"����CCUʧ��"));
		}
		else if(EHOSTUNREACH == err_code || ENETUNREACH == err_code)
		{
			ACE_DEBUG((LM_ERROR,"�޷������豸����"));
			node->SetState(KSGDeviceNode::dsError);
		}
		else
		{
			ACE_DEBUG((LM_ERROR,"��������δ֪����![%d][%s]ip[%s]"
				,err_code,ACE_OS::strerror(err_code),ip.c_str()));
		}
		// add by cash �ͷ� SOCKET 
		// 2007-01-29
		stream.close();
		return -1;
	}
	// ���� handler Ϊ BLOCK ��
	// stream.disable(ACE_NONBLOCK);
	// ���� linger ����
	struct linger lg;
	ACE_OS::memset(&lg,0,sizeof lg);
	lg.l_onoff = 1;
	// 3s 
	lg.l_linger = 3;
	stream.set_option(SOL_SOCKET,SO_LINGER,&lg,sizeof lg);
	node->SetState(KSGDeviceNode::dsOnline);
	*handler = stream.get_handle();
	return 0;
}

int HDCCUDevice::close_handler(KSGDeviceNode* node,ACE_HANDLE handler) throw ()
{
	try
	{
		ACE_Time_Value tv(0,500);
		// һ��Ҫ�ȵ��� close_handler ����ᵼ������
		//KSGDevice::close_handler(node,handler);
		/*
		int ret = ACE::handle_ready(handler,&tv,1,1,1);
		if( ret > 0)
		{
			ACE_OS::closesocket(handler);
		}
		else
		{
			// ��������쳣,Ҫ��ιر�??
			ACE_DEBUG((LM_ERROR,"���ӳ����쳣�޷����ر�!..."));
		}
		*/
		ACE_OS::closesocket(handler);
		return 0;
	}
	catch(...)
	{
		// catch all
		ACE_DEBUG((LM_ERROR,"�ر�CCU Handler ʧ��"));
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////////
// HDGCUDevice
int HDGCUDevice::make_handler(KSGDeviceNode* node,ACE_HANDLE* handler)
{
	if(!node)
		return -1;
	std::string ip = node->GetDevAddr().GetConnect();
	int port = node->GetDevAddr().GetPort();
	ACE_INET_Addr addr(port,ip.c_str());
	ACE_SOCK_Connector conn;
	ACE_SOCK_Stream stream;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	int err_code;
	ACE_DEBUG((LM_TRACE,"��ʼ���ӻ��GCU��[%s][%s]",node->get_name().c_str(),ip.c_str()));
	if(conn.connect(stream,addr,&tv))
	{
		err_code = ACE_OS::last_error();
		// TODO: �������ӵĴ�����
		if(EWOULDBLOCK == err_code)
		{
			ACE_DEBUG((LM_ERROR,"����GCUʧ��"));
		}
		else if(EHOSTUNREACH == err_code || ENETUNREACH == err_code)
		{
			ACE_DEBUG((LM_ERROR,"�޷������豸����"));
			node->SetState(KSGDeviceNode::dsError);
		}
		else
		{
			ACE_DEBUG((LM_ERROR,"���� GCU δ֪����![%d][%s]ip[%s]"
				,err_code,ACE_OS::strerror(err_code),ip.c_str()));
		}
		// add by cash �ͷ� SOCKET 
		// 2007-01-29
		stream.close();
		return -1;
	}
	// ���� handler Ϊ BLOCK ��
	// stream.disable(ACE_NONBLOCK);
	// ���� linger ����
	struct linger lg;
	ACE_OS::memset(&lg,0,sizeof lg);
	lg.l_onoff = 1;
	// 3s 
	lg.l_linger = 3;
	stream.set_option(SOL_SOCKET,SO_LINGER,&lg,sizeof lg);
	node->SetState(KSGDeviceNode::dsOnline);
	*handler = stream.get_handle();
	return 0;
}

int HDGCUDevice::close_handler(KSGDeviceNode* node,ACE_HANDLE handler)
{
	try
	{
		ACE_OS::closesocket(handler);
		return 0;
	}
	catch(...)
	{
		// catch all
		ACE_DEBUG((LM_ERROR,"�ر�GCU Handler �쳣"));
		return -1;
	}
}
bool HDGCUDevice::Accept(BaseVisitor& guest,KSGDeviceNode* visitor)
{
	if(typeid(guest) == typeid(KSGTaskDoorBatchDlCard::BatchDownloadCardVisitor))
		return true;
	if(typeid(guest) == typeid(KSGTaskCollectSerial::CollectSerialVisitor))
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// HDKQCCUDevice

bool HDKQCCUDevice::Accept(BaseVisitor& guest,KSGDeviceNode* visitor)
{
	//if(typeid(guest) == typeid(KSGTaskCollHeartBeat::CollHeartBeatVisitor))
	//	return true;
	return false;
}

int HDKQCCUDevice::make_handler(KSGDeviceNode* node,ACE_HANDLE* handler)
{
	if(!node)
		return -1;
	std::string ip = node->GetDevAddr().GetConnect();
	int port = node->GetDevAddr().GetPort();
	ACE_INET_Addr addr(port,ip.c_str());
	ACE_SOCK_Connector conn;
	ACE_SOCK_Stream stream;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	int err_code;
	if(conn.connect(stream,addr,&tv))
	{
		err_code = ACE_OS::last_error();
		// TODO: �������ӵĴ�����
		if(EWOULDBLOCK == err_code)
		{
			ACE_DEBUG((LM_ERROR,"����CCUʧ��"));
		}
		else if(EHOSTUNREACH == err_code || ENETUNREACH == err_code)
		{
			ACE_DEBUG((LM_ERROR,"�޷������豸����"));
			node->SetState(KSGDeviceNode::dsError);
		}
		else
		{
			ACE_DEBUG((LM_ERROR,"��������δ֪����![%d][%s]ip[%s]"
				,err_code,ACE_OS::strerror(err_code),ip.c_str()));
		}
		// add by cash �ͷ� SOCKET 
		// 2007-01-29
		stream.close();
		return -1;
	}
	// ���� handler Ϊ BLOCK ��
	// stream.disable(ACE_NONBLOCK);
	// ���� linger ����
	struct linger lg;
	ACE_OS::memset(&lg,0,sizeof lg);
	lg.l_onoff = 1;
	// 3s 
	lg.l_linger = 3;
	stream.set_option(SOL_SOCKET,SO_LINGER,&lg,sizeof lg);
	node->SetState(KSGDeviceNode::dsOnline);
	*handler = stream.get_handle();
	return 0;
}

int HDKQCCUDevice::close_handler(KSGDeviceNode* node,ACE_HANDLE handler) throw ()
{
	try
	{
		ACE_Time_Value tv(0,500);
		// һ��Ҫ�ȵ��� close_handler ����ᵼ������
		ACE_OS::closesocket(handler);
		return 0;
	}
	catch(...)
	{
		// catch all
		ACE_DEBUG((LM_ERROR,"�ر�CCU Handler ʧ��"));
		return -1;
	}
}

bool HDPosDevice::Accept(BaseVisitor& guest,KSGDeviceNode* visitor)
{
	/*
	if(typeid(guest) == typeid(KSGTaskCollHeartBeat::CollHeartBeatVisitor))
		return true;
	*/
	if(typeid(guest) == typeid(KSGTaskDownloadBlackCard::DownloadBlackCardVisitor))
	{
		if(visitor->owner_queue() && visitor->GetConnType() != KSGDeviceURL::dctModem)
			return true;
	}
	if(typeid(guest) == typeid(KSGTaskCollDeviceState::CollDeviceState))
		return true;
	//if(typeid(guest) == typeid(KSGTaskCollectSerial::CollectSerialVisitor))
	//	return true;
	return false;
}

int HDPosDevice::make_handler(KSGDeviceNode* node,ACE_HANDLE* handler)
{
	if(!node)
		return -1;
	if(node->GetConnType() != KSGDeviceURL::dctTCP)
		return 1;
	std::string ip = node->GetDevAddr().GetConnect();
	int port = node->GetDevAddr().GetPort();
	ACE_INET_Addr addr(port,ip.c_str());
	ACE_SOCK_Connector conn;
	ACE_SOCK_Stream stream;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	int err_code;
	if(conn.connect(stream,addr,&tv))
	{
		err_code = ACE_OS::last_error();
		// TODO: �������ӵĴ�����
		if(EWOULDBLOCK == err_code)
		{
			ACE_DEBUG((LM_ERROR,"����CCUʧ��"));
		}
		else if(EHOSTUNREACH == err_code || ENETUNREACH == err_code)
		{
			ACE_DEBUG((LM_ERROR,"�޷������豸����"));
			node->SetState(KSGDeviceNode::dsError);
		}
		else
		{
			ACE_DEBUG((LM_ERROR,"��������δ֪����![%d][%s]ip[%s]"
				,err_code,ACE_OS::strerror(err_code),ip.c_str()));
		}
		// add by cash �ͷ� SOCKET 
		// 2007-01-29
		stream.close();
		return -1;
	}
	// ���� handler Ϊ BLOCK ��
	// stream.disable(ACE_NONBLOCK);
	// ���� linger ����
	struct linger lg;
	ACE_OS::memset(&lg,0,sizeof lg);
	lg.l_onoff = 1;
	// 3s 
	lg.l_linger = 3;
	stream.set_option(SOL_SOCKET,SO_LINGER,&lg,sizeof lg);
	node->SetState(KSGDeviceNode::dsOnline);
	*handler = stream.get_handle();
	return 0;
}

int HDPosDevice::close_handler(KSGDeviceNode* node,ACE_HANDLE handler) throw ()
{
	try
	{
		ACE_Time_Value tv(0,500);
		ACE_OS::closesocket(handler);
		return 0;
	}
	catch(...)
	{
		// catch all
		ACE_DEBUG((LM_ERROR,"�ر�CCU Handler ʧ��"));
		return -1;
	}
}

bool HDAddPosDevice::Accept(BaseVisitor& guest,KSGDeviceNode* visitor)
{
	/*
	if(typeid(guest) == typeid(KSGTaskCollHeartBeat::CollHeartBeatVisitor))
		return true;
	if(typeid(guest) == typeid(KSGTaskCollectSerial::CollectSerialVisitor))
		return true;
	*/
	if(typeid(guest) == typeid(KSGTaskCollDeviceState::CollDeviceState))
		return true;
	return false;
}

int HDAddPosDevice::make_handler(KSGDeviceNode* node,ACE_HANDLE* handler)
{
	if(!node)
		return -1;
	// �����TCPЭ��
	if(node->GetConnType() != KSGDeviceURL::dctTCP)
		return 1;
	std::string ip = node->GetDevAddr().GetConnect();
	int port = node->GetDevAddr().GetPort();
	ACE_INET_Addr addr(port,ip.c_str());
	ACE_SOCK_Connector conn;
	ACE_SOCK_Stream stream;
	ACE_Time_Value tv(1);
	int err_code;
	if(conn.connect(stream,addr,&tv))
	{
		err_code = ACE_OS::last_error();
		// TODO: �������ӵĴ�����
		if(EWOULDBLOCK == err_code)
		{
			ACE_DEBUG((LM_ERROR,"����CCUʧ��"));
		}
		else if(EHOSTUNREACH == err_code || ENETUNREACH == err_code)
		{
			ACE_DEBUG((LM_ERROR,"�޷������豸����"));
			node->SetState(KSGDeviceNode::dsError);
		}
		else
		{
			ACE_DEBUG((LM_ERROR,"��������δ֪����![%d][%s]"
				,err_code,ACE_OS::strerror(err_code)));
		}
		// add by cash �ͷ� SOCKET 
		// 2007-01-29
		stream.close();
		return -1;
	}
	// ���� handler Ϊ BLOCK ��
	// stream.disable(ACE_NONBLOCK);
	// ���� linger ����
	struct linger lg;
	ACE_OS::memset(&lg,0,sizeof lg);
	lg.l_onoff = 1;
	// 3s 
	lg.l_linger = 3;
	stream.set_option(SOL_SOCKET,SO_LINGER,&lg,sizeof lg);
	node->SetState(KSGDeviceNode::dsOnline);
	*handler = stream.get_handle();
	return 0;
}
int HDAddPosDevice::close_handler(KSGDeviceNode* node,ACE_HANDLE handler) throw()
{
	if(node->GetConnType() != KSGDeviceURL::dctTCP)
		return 1;
	try
	{
		ACE_OS::closesocket(handler);
		return 0;
	}
	catch(...)
	{
		// catch all
		ACE_DEBUG((LM_ERROR,"�ر�CCU Handler ʧ��"));
		return -1;
	}
}

int HDPenStockDevice::make_handler(KSGDeviceNode* node,ACE_HANDLE* handler)
{
	return 1;
}
int HDPenStockDevice::close_handler(KSGDeviceNode* node,ACE_HANDLE handler) throw()
{
	return 1;
}
bool HDPenStockDevice::Accept(BaseVisitor& guest,KSGDeviceNode* visitor)
{
	if(typeid(guest) == typeid(KSGTaskCollDeviceState::CollDeviceState))
		return true;
	return false;
}

int HDKQDevice::make_handler(KSGDeviceNode* node,ACE_HANDLE* handler)
{
	if(!node)
		return -1;
	// �����TCPЭ��
	if(node->GetConnType() != KSGDeviceURL::dctTCP)
		return 1;
	std::string ip = node->GetDevAddr().GetConnect();
	int port = node->GetDevAddr().GetPort();
	ACE_INET_Addr addr(port,ip.c_str());
	ACE_SOCK_Connector conn;
	ACE_SOCK_Stream stream;
	ACE_Time_Value tv(1);
	int err_code;
	if(conn.connect(stream,addr,&tv))
	{
		err_code = ACE_OS::last_error();
		// TODO: �������ӵĴ�����
		if(EWOULDBLOCK == err_code)
		{
			ACE_DEBUG((LM_ERROR,"���ӻ�࿼�ڻ�ʧ��"));
		}
		else if(EHOSTUNREACH == err_code || ENETUNREACH == err_code)
		{
			ACE_DEBUG((LM_ERROR,"�޷������豸����"));
			node->SetState(KSGDeviceNode::dsError);
		}
		else
		{
			ACE_DEBUG((LM_ERROR,"��������δ֪����![%d][%s]"
				,err_code,ACE_OS::strerror(err_code)));
		}
		// add by cash �ͷ� SOCKET 
		// 2007-01-29
		stream.close();
		return -1;
	}
	// ���� handler Ϊ BLOCK ��
	// stream.disable(ACE_NONBLOCK);
	// ���� linger ����
	struct linger lg;
	ACE_OS::memset(&lg,0,sizeof lg);
	lg.l_onoff = 1;
	// 3s 
	lg.l_linger = 3;
	stream.set_option(SOL_SOCKET,SO_LINGER,&lg,sizeof lg);
	node->SetState(KSGDeviceNode::dsOnline);
	*handler = stream.get_handle();
	return 0;
}
int HDKQDevice::close_handler(KSGDeviceNode* node,ACE_HANDLE handler) throw()
{
	if(node->GetConnType() != KSGDeviceURL::dctTCP)
		return 1;
	try
	{
		ACE_OS::closesocket(handler);
		return 0;
	}
	catch(...)
	{
		// catch all
		ACE_DEBUG((LM_ERROR,"�رջ�࿼�� Handler ʧ��"));
		return -1;
	}
}
bool HDKQDevice::Accept(BaseVisitor& guest,KSGDeviceNode* visitor)
{
	if(typeid(guest) == typeid(KSGTaskCollDeviceState::CollDeviceState)
		||typeid(guest) == typeid(KSGTaskCollectSerial::CollectSerialVisitor)
		||typeid(guest) == typeid(KSGTaskDoorBatchDlCard::BatchDownloadCardVisitor)
		)
		return true;
	return false;
}

int HDReaderDevice::make_handler(KSGDeviceNode* node,ACE_HANDLE* handler)
{
	return 1;
}
int HDReaderDevice::close_handler(KSGDeviceNode* node,ACE_HANDLE handler) throw()
{
	return 1;
}
bool HDReaderDevice::Accept(BaseVisitor& guest,KSGDeviceNode* visitor)
{
	return false;
}

//////////////////////////////////////////////////////////////////////
// HDDeviceLoader
int HDDeviceLoader::Finish(KSGDeviceManager* manager)
{
	KSGDeviceManager::DeviceList * devs = manager->GetDevices();
	//std::for_each(devs->begin(),devs->end(),
	//	boost::bind(&HDDeviceLoader::SetupDeviceNoticeTask,this,_1));
	// ����ָ����еķ���
	try
	{
		manager->Traseval(boost::bind(&HDDeviceLoader::SetupDeviceGroup,this,_1));
	}
	catch(KSGException&)
	{
		return -1;
	}
	return 0;
}

int HDDeviceLoader::LoadDevice(KSGDeviceManager* manager)
{
	ACE_DEBUG((LM_DEBUG,"��ʼ���ػ���豸..."));
	try
	{
		// modified by ����  2006-12-31
		// �޸� : ���豸�ڵ� toplogic �ĳ�ʼ��������������������
		// �����豸�������ṹ��ϵ
		/*
		std::for_each(devs->begin(),devs->end(),
			boost::bind(&HDDeviceLoader::SetupDevice,this,manager,_1));
		*/
	}
	catch(...)
	{
	}
	return 0;
}

void HDDeviceLoader::SetupDeviceGroup(KSGDeviceNode* node)
{
	if(node->get_vendor() != KSG_HD_DEV)
		return;
	node->connect_module(KSGDeviceNode::cm_long_conn);
	// modify by Cash , TCP ��POS������Ϊ������ģʽ
	if(node->GetConnType() == KSGDeviceURL::dctTCP
		&& node->GetDeviceType() != KSG_HD_SERVER_DEV)
	{
		node->connect_module(KSGDeviceNode::cm_short_conn);
		ACE_DEBUG((LM_TRACE,"����豸[%d]��TCP�豸",node->GetDevId()));
	}
	// ����Ҫ���� loop device ,
	if(node->GetDeviceType() == KSG_HD_SERVER_DEV
		|| node->GetConnType() == KSGDeviceURL::dctTCP)
	{
		KSG_Task_Queue *queue = Task_Queue_Pool::instance()->add_initial_queue(KSG_SCHEDULER_STATUS);
		if(queue)
		{
			// ���������豸
			if(queue->load_all_device(node))
				throw KSGException();
		}
		else
		{
			ACE_DEBUG((LM_WARNING,"�豸�޷������������,dev[%d]",node->GetDevId()));
		}
	}
}
//////////////////////////////////////////////////////////////////////
// HDDevInterfaceLoader
int HDDevInterfaceLoader::LoadInterface(KSGDeviceManager *manager)
{
	try
	{
		// Ϊ�豸�������ӽӿ�
		// ע��POS���ӿ�
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_POS_DEV,IHDAddBlackCard);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_POS_DEV,IHDDelBlackCard);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_POS_DEV,IHDSetCardPrivileges);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_POS_DEV,IHDSetFeeRate);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_POS_DEV,IHDSetFeeRate2);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_POS_DEV,IHDCollPosHisSerial);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_POS_DEV,IHDDownloadConCode);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_POS_DEV,IHDBatchDownloadBlkCard);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_POS_DEV,IHDCollectHeardbeat);
		
		// ע���ֵ���ӿ�
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_ADD_POS_DEV,IHDAddBlackCard);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_ADD_POS_DEV,IHDDelBlackCard);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_ADD_POS_DEV,IHDSetCardPrivileges);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_ADD_POS_DEV,IHDCollPosHisSerial);
		// ע��CCU�ӿ�
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_SERVER_DEV,IHDDLSubsidyFile);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_SERVER_DEV,IHDCCUOnlineNotice);
		
		//ע��ˮ�ػ�
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_PENSTOCK,IHDCtlPenStock);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_PENSTOCK,IHDCollPosHisSerial);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_PENSTOCK,IHDDLWaterFeeCfg);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_HD_PENSTOCK,IHDSetCardPrivileges);
		
		//ע��GCU
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,"9003",IHDBatchDownloadWhiteCard);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,"9003",ID_GCU_DL_Timesect);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,"9003",ID_GCU_DL_Week);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_READER_DEV,ID_CTRL_DOOR);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,"9003",IHDCollectSerial);

		// ע�ῼ�ڻ�
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_JSB_DEV,IHDBatchDownloadKQCard);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_JSB_DEV,IHDCollectSerial);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_JSB_DEV,IHDDLManageCard);
		KSG_ADD_DEVICE_INTERFACE(KSG_HD_DEV,KSG_JSB_DEV,IHDDLKQPARAM);

		return 0;
	}
	catch(...)
	{
		ACE_DEBUG((LM_ERROR,"���ػ���豸�ӿ�ʧ��.........."));
		return -1;
	}
}

int HDTaskExecutorLoader::LoadExecutor()
{
	// ע��ָ��ִ�д����ӿ�
	return 0;
}
//////////////////////////////////////////////////////////////////////
//
int IHDCollectSerial::DeleteRecord(KSGDeviceNode* node,Task* task)
{
	ACE_HANDLE handler = task->_handle;
	HD8583STRUCT req;

	KSGDeviceNode* task_node = task->GetDeviceNode();
	unsigned long termsn = DecodeTermSN(task_node->GetPhyId().c_str());
	unsigned short address = GetDeviceAddress(task_node);
	unsigned short termid = task_node->GetTermId();
	req.Init();
	req.SetFieldValue(FIELD_TERMINALSN,termsn);
	req.SetFieldValue(FIELD_ADDRESS,address);
	req.SetFieldValue(FIELD_TERMINALID,termid);
	char rec_cnt = (char)10;
	req.SetFieldValue(FIELD_ADDITIONALDATA1,&rec_cnt,1);
	
	char buf[8184] = "";
	size_t32 packlen = PackRequestStruct(req,MT_DELETERECORD2,buf,sizeof buf,true);
	if( packlen <= 0)
	{
		ACE_DEBUG((LM_ERROR,"PackRequestStruct err"));
		return TASK_ERR_COMMON;
	}
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	if(HDSendBuffer(handler,buf,packlen,&tv) != 0)
	{
		ACE_DEBUG((LM_ERROR,"HDSendBuffer err"));
		return TASK_ERR_TIMEOUT;
	}
	/*    ɾ����ˮ����Ӧ����
	packlen = HDRecvBuffer(handler,buf,sizeof buf,&tv);
	if( packlen <= 0)
	{
		ACE_DEBUG((LM_ERROR,"d HDRecvBuffer err"));
		return TASK_ERR_COMMON;
	}
	MESSAGETYPE msg_type;
	if(UnPackResponseStruct(req,&msg_type,buf,packlen)!=0)
	{
		//
		ACE_DEBUG((LM_ERROR,"d UnPackResponseStruct err"));
		return TASK_ERR_COMMON;
	}
	if(req->ResponseCode != RC_SUCCESS)
	{
		ACE_DEBUG((LM_ERROR,"d ResponseCode err"));
		return TASK_ERR_COMMON;
	}
	*/
	return 0;

}
int IHDCollectSerial::ExecuteTask(KSGDeviceNode* node,Task* task)
{

	KSGDevice* device = node->GetDevice();
	if(!device)
		return TASK_ERR_COMMON;
	ACE_HANDLE handler = task->_handle;
	unsigned long termsn = 0;
	ACE_SOCK_Stream stream(handler);

	HD8583STRUCT req;

	KSGDeviceNode* task_node = task->GetDeviceNode();
	termsn = DecodeTermSN(task_node->GetPhyId().c_str());
	unsigned short address = GetDeviceAddress(task_node);
	unsigned short termid = task_node->GetTermId();
	req.Init();
	req.SetFieldValue(FIELD_TERMINALSN,termsn);
	req.SetFieldValue(FIELD_ADDRESS,address);
	req.SetFieldValue(FIELD_TERMINALID,termid);
	char rec_cnt = (char)10;
	req.SetFieldValue(FIELD_ADDITIONALDATA1,&rec_cnt,1);

	char buf[8184] = "";
	size_t32 packlen = PackRequestStruct(req,MT_BATCHSENDRECORD2,buf,sizeof buf,true);
	if( packlen <= 0)
	{
		ACE_DEBUG((LM_ERROR,"PackRequestStruct err"));
		return TASK_ERR_COMMON;
	}
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	if(HDSendBuffer(handler,buf,packlen,&tv) != 0)
	{
		ACE_DEBUG((LM_ERROR,"HDSendBuffer err"));
		return TASK_ERR_TIMEOUT;
	}
	packlen = HDRecvBuffer(handler,buf,sizeof buf,&tv);
	if( packlen <= 0)
	{
		ACE_DEBUG((LM_ERROR,"HDRecvBuffer err"));
		return TASK_ERR_COMMON;
	}
	MESSAGETYPE msg_type;
	if(UnPackResponseStruct(req,&msg_type,buf,packlen)!=0)
	{
		//
		ACE_DEBUG((LM_ERROR,"UnPackResponseStruct err"));
		return TASK_ERR_COMMON;
	}
	if(req->ResponseCode != RC_SUCCESS)
	{
		ACE_DEBUG((LM_ERROR,"ResponseCode err"));
		return TASK_ERR_COMMON;
	}
	if(CollectRecord(node,req) == 0)
	{
		return DeleteRecord(node,task);
	}

	return TASK_ERR_COMMON;
}

int IHDCollectSerial::CollectRecord(KSGDeviceNode* node,HD8583STRUCT& record)
{
	int ctrl_id=0;
	int reader_id = 0;
	int event_code =0;
	int card_no=0;
	int cnt=0;
	char date_str[9]="";
	char time_str[7]="";
	unsigned char tmp[10]="";
	char phyno[12]="";
	KSGVendorConfig *vendor;

	DRTPPoolType::SmartObject drtp = KsgGetDrtpPool()->Alloc();
	if(!drtp)
		return TASK_ERR_EXECUTE;
	if(drtp->Connect())
	{
		// ����ʧ��
		return 1;
	}
	drtp->SetRequestHeader(930203);
	
	if(node->GetDeviceType() == KSG_JSB_DEV)				// ���ڻ�
	{
		cnt = record->LenOfAdditionalData1/14;				// ��¼����Ϊ14�ֽ�
		for(int i=0;i<cnt;i++)
		{
			memset(tmp,0,sizeof tmp);
			memcpy(tmp,record->AdditionalData1+i*14,4);
			//if(strlen((char*)tmp)==0)								// ��ˮ�ɼ�����
			//	break;
			sprintf(phyno,"%02X%02X%02X%02X",tmp[3],tmp[2],tmp[1],tmp[0]);
			drtp->AddField(F_SDATE1,phyno);
			/*card_no = atoi(tmp);
			if(!card_no)
				break;
			
			drtp->AddField(F_LVOL0,card_no);
			*/
			memset(tmp,0,sizeof tmp);
			memcpy(tmp,record->AdditionalData1+i*14+4,3);		// ����
			sprintf(date_str,"20%02d%02d%02d",tmp[0],tmp[1],tmp[2]);
			drtp->AddField(F_SDATE0,date_str);	

			memset(tmp,0,sizeof tmp);
			memcpy(tmp,record->AdditionalData1+i*14+7,3);		// ʱ��
			sprintf(time_str,"%02d%02d%02d",tmp[0],tmp[1],tmp[2]);		
			drtp->AddField(F_STIME0,time_str);

			memset(tmp,0,sizeof tmp);
			BUF_2_SHORT_LE(ctrl_id,record->AdditionalData1+i*14+10);  // ������id��

			reader_id = record->AdditionalData1[i*14+12];		// ��ͷ��
			if(reader_id == 0)
				drtp->AddField(F_LVOL1,ctrl_id);
			else
			{
				// ���ڻ� ����
				drtp->AddField(F_LVOL3,ctrl_id);
				// ��ͷ����
				drtp->AddField(F_LVOL1,reader_id);
			}
			// ����
			drtp->AddField(F_SBANK_CODE,KSG_JSB_DEV);
			// ����豸
			vendor = KsgGetGateway()->get_vendor_config(KSG_HD_DEV);
			if(vendor)
			{
				drtp->AddField(F_LSERIAL0,vendor->_vendor_id);
			}
			else
				ACE_DEBUG((LM_ERROR,"���Ӳ������δ֪"));
			/*
			memcpy(tmp,record->AdditionalData1+i*14+10,2);	
			ctrl_id = atoi(tmp);
			drtp->AddField(F_LVOL2,ctrl_id);					// ������id��

			reader_id = record->AdditionalData1[i*14+12];		// ��ͷ��
			drtp->AddField(F_LVOL1,reader_id);					
			*/
			event_code = record->AdditionalData1[i*14+13];		// ���ڼ�¼��
			drtp->AddField(F_SEMP_NO,(event_code & 0x03) + HDA_SYSTEMIDOFATT1_1);
			drtp->AddField(F_LCERT_CODE,KsgGetGateway()->GetConfig()->_gwId);	//sys_id
			if(drtp->SendRequest(5000))
			{
				if(drtp->GetReturnCode())
					ACE_DEBUG((LM_ERROR,"�ϴ�ˢ����Ϣʧ��,�豸[%d]������[%d]"
					,ctrl_id,drtp->GetReturnCode()));
				return TASK_ERR_EXECUTE;
			}		
		}
	}
	else if(node->GetDeviceType() == "9003")				// �Ž�������GCU
	{
		cnt = record->LenOfAdditionalData1/18;				// ��¼����Ϊ18�ֽ�
		for(int i=0;i<cnt;i++)
		{
			memset(tmp,0,sizeof tmp);
			memcpy(tmp,record->AdditionalData1+i*18,4);
			//if(strlen((char*)tmp)==0)								// ��ˮ�ɼ�����
			//	break;
			BUF_2_INT(card_no,tmp);
			drtp->AddField(F_LVOL0,card_no);

			memset(tmp,0,sizeof tmp);
			memcpy(tmp,record->AdditionalData1+i*18+4,3);		// ����
			sprintf(date_str,"20%02d%02d%02d",tmp[0],tmp[1],tmp[2]);
			drtp->AddField(F_SDATE0,date_str);	

			memset(tmp,0,sizeof tmp);
			memcpy(tmp,record->AdditionalData1+i*18+7,3);		// ʱ��
			sprintf(time_str,"%02d%02d%02d",tmp[0],tmp[1],tmp[2]);		
			drtp->AddField(F_STIME0,time_str);

			memset(tmp,0,sizeof tmp);
			BUF_2_SHORT_LE(ctrl_id,record->AdditionalData1+i*18+10);  // ������id��

			reader_id = record->AdditionalData1[i*18+12];		// ��ͷ��
			if(reader_id == 0)
				drtp->AddField(F_LVOL1,ctrl_id);
			else
			{
				// ����������
				drtp->AddField(F_LVOL3,ctrl_id);
				// ��ͷ����
				drtp->AddField(F_LVOL1,reader_id);
			}
			// ����豸
			vendor = KsgGetGateway()->get_vendor_config(KSG_HD_DEV);
			if(vendor)
			{
				drtp->AddField(F_LSERIAL0,vendor->_vendor_id);
			}
			else
				ACE_DEBUG((LM_ERROR,"���Ӳ������δ֪"));
			
			event_code = record->AdditionalData1[i*14+15];		// GCU�������
			drtp->AddField(F_SEMP_NO,event_code);
			drtp->AddField(F_LCERT_CODE,KsgGetGateway()->GetConfig()->_gwId);	//sys_id
			if(drtp->SendRequest(5000))
			{
				if(drtp->GetReturnCode())
					ACE_DEBUG((LM_ERROR,"�ϴ�ˢ����Ϣʧ��,�豸[%d]������[%d]"
					,ctrl_id,drtp->GetReturnCode()));
				return TASK_ERR_EXECUTE;
			}		
		}
	}
	else
	{
		ACE_DEBUG((LM_ERROR,"�����ɼ���ˮ���ԣ��豸����ID[%s]�豸����[%s]����",
			node->GetPhyId().c_str(),node->GetDeviceType().c_str()));
	}

	return 0;
}

int IHDCollectHeardbeat::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	KSGDevice * device = node->GetDevice();
	if(!device)
		return TASK_ERR_COMMON;
	ACE_HANDLE handler = task->_handle;
//	int ret ;
	HD8583STRUCT req;
	KSGDeviceNode* task_node = task->GetDeviceNode();
	unsigned long termid = ACE_OS::strtoul(task_node->GetPhyId().c_str(),NULL,10);
	req.SetFieldValue(FIELD_TERMINALSN,termid);
	req.SetFieldValue(FIELD_ADDRESS,GetDeviceAddress(task_node));
	char buf[4096] = "";
	size_t32 packlen = PackRequestStruct(req,MT_REECHO2,buf,sizeof buf,false);
	if( packlen <= 0)
	{
		return TASK_ERR_COMMON;
	}
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	if(HDSendBuffer(handler,buf,packlen,&tv) != 0)
	{
		return TASK_ERR_TIMEOUT;
	}
	
	memset(buf,0,sizeof buf);
	packlen = HDRecvBuffer(handler,buf,sizeof buf,&tv);
	if(packlen <= 0)
	{
		return TASK_ERR_CONNECT;
	}
	MESSAGETYPE msg_type;
	if(UnPackResponseStruct(req,&msg_type,buf,packlen)!=0)
	{
		ACE_DEBUG((LM_DEBUG,"�������ʧ��"));
		return TASK_ERR_CONNECT;
	}
	task_node->SetState(KSGDeviceNode::dsOnline);
	ACE_DEBUG((LM_DEBUG,"������Գɹ�![%u][%x]",termid,req->ResponseCode));
	return TASK_SUCCESS;
}
int IHDCCUOnlineNotice::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	MYDATETIMESTRUCT now;
	HD8583STRUCT req;
	char data[300];
	size_t packlen;
	int ret;
	MESSAGETYPE msg_type;
	ACE_HANDLE handle = task->_handle;
	KSGDeviceNode* task_node = task->GetDeviceNode();
	unsigned long termid = ACE_OS::strtoul(task_node->GetPhyId().c_str(),NULL,10);
	req.SetFieldValue(FIELD_TERMINALSN,termid);
	now = HDGetDataTime();
	req.SetFieldValue(FIELD_DATEANDTIME,(char*)&now,7);

	ACE_DEBUG((LM_DEBUG,"�ɼ�CCU����,dev[%s]",task_node->GetPhyId().c_str()));
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	if( (packlen = PackRequestStruct(req,MT_CCUONLINENOTICE2,data,sizeof data,false)) <= 0)
	{
		ret = TASK_ERR_TIMEOUT;
	}
	else if( HDSendBuffer(handle,data,packlen,&tv) != 0)
	{
		task_node->SetState(KSGDeviceNode::dsOffline);
		ret = TASK_ERR_TIMEOUT;
	}
	else 
	{
		tv = KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handle,data,sizeof data,&tv)) <= 0)
		{
			task_node->SetState(KSGDeviceNode::dsOffline);
			ret = TASK_INVALID_CONN;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			task_node->SetState(KSGDeviceNode::dsOffline);
			ret = TASK_INVALID_CONN;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ret = TASK_ERR_EXECUTE;
			// ��Ϊ�ѻ�
			task_node->SetState(KSGDeviceNode::dsOffline);
		}
		else
		{
			task_node->SetState(KSGDeviceNode::dsOnline);
			task_node->update_time();
			ret = TASK_SUCCESS;
		}
	}
	task->SetNeedResponse(false);
	return ret;
}
/////////////////////////////////////////////////////////////////////////
// IHDAddBlackCard
int IHDAddBlackCard::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	HDDownloadBlkCard dl;
	return dl.DownloadBlackCard(ADD_BLK_LIST,node,task);
}

int IHDDelBlackCard::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	HDDownloadBlkCard dl;
	return dl.DownloadBlackCard(DEL_BLK_LIST,node,task);
}

int IHDBatchDownloadBlkCard::dowload_blkcard(KSGDeviceNode* node,Task* task,std::string &ret_ver)
{
	// ���ӣ�ɾ��������
	int ret = TASK_ERR_COMMON;
	try
	{
		int result,i;
		task->SetNeedResponse(false);
		char data[300];
		std::string cardid_str = task->GetParams().GetParam(XML_KEY_CARDID);
		std::string version = task->GetParams().GetParam(XML_KEY_VERNUM);
		int count = task->GetParams().GetParamIntVal(XML_KEY_FTFLAG);
		KSGDeviceNode* task_node = task->GetDeviceNode();
		if(task_node == NULL)
			return TASK_ERR_EXECUTE;
		unsigned short addr;
		if(count <= 0 || count > 50)
			return -1;
		addr = GetDeviceAddress(task->GetDeviceNode());
		// ׼��������������
		HD8583STRUCT req;
		req.Init();
		req.SetFieldValue(FIELD_ADDRESS,addr); // �ն˵�ַ
		unsigned long termid = ACE_OS::strtoul(task_node->GetPhyId().c_str(),NULL,10);
		req.SetFieldValue(FIELD_TERMINALSN,termid);
		//req.SetFieldValue(FIELD_TERMINALID,task_node->GetTermId()); // �ն˻���
		unsigned long encode_ver = EncodeVersionNum(version.c_str()); // ѹ���汾��
		req.SetFieldValue(FIELD_VERSIONOFLIST,encode_ver);	// �������汾��
		for(i = 0;i < count; ++i)
		{
			xutil::StringUtil::Str2Hex(cardid_str.c_str() + i*10,(unsigned char*)data+i*5,10);
			data[i*5] = (data[i*5] == 0) ? ADD_BLK_LIST : DEL_BLK_LIST ;
		}
		ACE_HEX_DUMP((LM_TRACE,data,count*5));
		req.SetFieldValue(FIELD_ADDITIONALDATA2,data,count*5);  //  ���׿���
		data[0] = count; 
		req.SetFieldValue(FIELD_ADDITIONALDATA3,data,1); // ��������
		ACE_OS::memset(data,0,sizeof data);
		ACE_HANDLE handler = ACE_INVALID_HANDLE;
		ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
		int recvlen = -1;
		size_t packlen;
		handler = task->_handle;
		ACE_DEBUG((LM_DEBUG,"�����豸[%s]���������汾[%s]",node->get_name().c_str(),version.c_str()));
		// ѹ�����ݰ�
		if((packlen = PackRequestStruct(req,MT_UPDATELIST2,data,sizeof data,true)) == 0)
		{
			// ��������ݰ�
			ret = TASK_ERR_COMMON;
			return ret;
		}
		ACE_HEX_DUMP((LM_DEBUG,data,packlen));
		// �ȴ�ʱ��

		if((result=HDSendBuffer(handler,data,packlen,&tv))) // �������ݰ�
		{
			ACE_DEBUG((LM_ERROR,"���ͺ���������ʧ�ܣ�������[%d]",result));
			ret = TASK_ERR_TIMEOUT;
			return ret;
		}
		ACE_DEBUG((LM_DEBUG,"�����豸[%s]���������ȴ��豸Ӧ��",node->get_name().c_str()));
		int wait_time = 150;
		KSGThreadUtil::Sleep(wait_time);
		//ACE_DEBUG((LM_DEBUG,"�����豸[%s]���������ȴ��豸Ӧ��2",node->get_name().c_str()));
		tv = ACE_Time_Value(1,0);
		if((recvlen = HDRecvBuffer(handler,data,sizeof data,&tv)) <= 0) // �������ݰ�
		{
			if(recvlen == -2)
				ret = TASK_INVALID_CONN;
			else
				ret = TASK_ERR_TIMEOUT;
		}
		else
		{
			HD8583STRUCT resp;
			MESSAGETYPE msg_type;
			if(recvlen < 17)
			{
				ACE_DEBUG((LM_ERROR,"�´��������豸Ӧ�����dev[%s]",task_node->GetPhyId().c_str()));
				ret = TASK_ERR_TIMEOUT;
			}
			else if(UnPackResponseStruct(resp,&msg_type,data,recvlen)) // ��ѹ���ݰ�
			{
				ACE_DEBUG((LM_ERROR,"�´��������豸���ذ����Ϸ�dev[%s]",task_node->GetPhyId().c_str()));
				ret = TASK_ERR_EXECUTE;
			}
			else if( (req->Address != resp->Address)
				|| (req->VerOfList != resp->VerOfList)) // �Ƚ�POS��Ӧ����ҵĺ������汾��
			{
				char ver[15] = "";
				DecodeVersionNum(resp->VerOfList,ver);
				ACE_DEBUG((LM_ERROR,"���ͺ���������ʧ�ܣ�������[%d]�豸[%d][%s]�汾��[%s]"
					,resp->ResponseCode,task_node->GetDevId(),task_node->get_name().c_str(),ver));
				if(resp->ResponseCode != 39)
				{

					ret = TASK_ERR_EXECUTE;
					return ret;
				}
				else
				{
					// ���ܰ汾���Ѿ�������
					ACE_DEBUG((LM_INFO,"�豸[%d][%s]�豸�汾��[%s],ϵͳ�汾��[%s]"
						,task_node->GetDevId(),task_node->get_name().c_str(),ver,version.c_str()));
					ret = TASK_SUCCESS;
				}
			}
			else
				ret = TASK_SUCCESS;
			if(TASK_SUCCESS == ret)
			{
				// �ɹ�����Ӧ�����̨
				ACE_DEBUG((LM_DEBUG,"�����豸[%s]���������豸Ӧ��ɹ�",node->get_name().c_str()));
				//KSGTaskResponse &tresp = task->GetResponse();
				//tresp.AddField(XML_KEY_CARDID,cardid);
				char version_str[14] = "";
				DecodeVersionNum(resp->VerOfList,version_str);
				//tresp.AddField(XML_KEY_VERNUM,version_str);
				//ACE_DEBUG((LM_INFO,"�����豸�������汾�ɹ�id[%d][%s]"
				//	,task_node->GetDevId(),version_str));
				ret_ver = version;
			}

		}
		return ret;
	}
	catch(KeyNotFoundException& )
	{
		return TASK_ERR_COMMON;
	}
	catch(...)
	{
		return TASK_ERR_COMMON;
	}
}
int IHDBatchDownloadBlkCard::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	std::string version;
	int ret = dowload_blkcard(node,task,version);
	if(ret == TASK_SUCCESS)
	{
		// Ӧ��
		DRTPPoolType::SmartObject obj;
		try
		{
			obj = KsgGetDrtpPool()->Alloc();
		}
		catch (NoneResourceException& )
		{
			// û����Դ	
			ACE_DEBUG((LM_ERROR,"����DRTP����ʧ�ܣ�"));
			return TASK_ERR_EXECUTE;
		}
		if(obj->Connect())
		{
			// ����ʧ��
			return TASK_ERR_EXECUTE;
		}
		obj->SetRequestHeader(930046);
		obj->AddField(F_LCERT_CODE,KsgGetGateway()->GetConfig()->_gwId);
		obj->AddField(F_SCUST_LIMIT2,KsgGetGateway()->GetConfig()->_dynaKey.c_str());
		obj->AddField(F_LVOL5,node->GetDevId());
		obj->AddField(F_SSERIAL0,version.c_str());
		int retries = 3;
		ret = TASK_ERR_EXECUTE;
		while(retries-- > 0)
		{
			if(obj->SendRequest(5000))
			{
				// ����ָ��ʧ��
				ACE_DEBUG((LM_DEBUG,"Ӧ����½��ʧ��"));
			}
			else if(obj->GetReturnCode())
			{
				ACE_DEBUG((LM_ERROR,"���º�������汾ʧ��,dev[%s],ret[%d][%s]"
					,node->GetPhyId().c_str(),obj->GetReturnCode(),obj->GetReturnMsg().c_str()));
			}
			else
			{
				ACE_DEBUG((LM_INFO,"�����豸�������汾�ɹ�id[%d][%s]"
					,node->GetDevId(),version.c_str()));
				ret = TASK_SUCCESS;
				break;
			}
		}
	}
	return ret;
}
int IHDSetCardPrivileges::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	std::string rights = task->GetParams().GetParam(XML_KEY_CARDRIGHTTYPE);
	int i,j;
	char right_buf[257] = "";
	/*
	for(j=i=0;i < rights.length() && i < sizeof(right_buf) - 1;++i)
	{
		if(rights.at(i) == '1')
			right_buf[j++] = i;
	}
	*/
	//ACE_DEBUG((LM_INFO,"right[%s]",rights.c_str()));
	for(i=0; i < rights.length(); i+=2)
	{
		char temp[3] = "";
		temp[0] = rights.at(i);
		temp[1] = rights.at(i+1);
		unsigned char t = (unsigned char)strtoul(temp,NULL,16);
		//ACE_DEBUG((LM_INFO,"parse data[%d]",t));
		for(j = 0;j < 8; ++j)
		{
			right_buf[1+i*4+j] = (((t >> (7 - j)) & 0x01) == 1) ? (1+i*4+j) : 0;
		}
	}
	//ACE_HEX_DUMP((LM_INFO,right_buf,256));
	KSGDeviceNode* task_node = task->GetDeviceNode();
	unsigned short addr = GetDeviceAddress(task_node);
	HD8583STRUCT req;
	req.SetFieldValue(FIELD_ADDRESS,addr);
	req.SetFieldValue(FIELD_ADDITIONALDATA2,right_buf,256);
	//req.SetFieldValue(FIELD_TERMINALID,(short)task_node->GetTermId());
	char data[1024] = "";
	int packlen;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	MESSAGETYPE msg_type;
	ACE_HANDLE handler = task->_handle;
	int ret ;
	if( (packlen = PackRequestStruct(req,MT_AUTHGROUP2,data,sizeof data,true)) <= 0)
	{
		ret = TASK_ERR_TIMEOUT;
	}
	else if( HDSendBuffer(handler,data,packlen,&tv) != 0)
	{
		ret = TASK_ERR_TIMEOUT;
	}
	else 
	{
		//ACE_HEX_DUMP((LM_INFO,data,packlen));
		// �ȴ� 50 ms Ӧ��
		//KSGThreadUtil::Sleep(50);
		tv = ACE_Time_Value(3);	//KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handler,data,sizeof data,&tv)) <= 0)
		{
			ret = TASK_ERR_TIMEOUT;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			ret = TASK_ERR_EXECUTE;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ret = TASK_ERR_EXECUTE;
			ACE_DEBUG((LM_INFO,"�����豸����Ȩ��ʧ��!! id[%d]dev[%s]"
				,task_node->GetDevId(),task_node->GetPhyId().c_str()));
		}
		else
		{
			ACE_DEBUG((LM_INFO,"�����豸����Ȩ�޳ɹ�id[%d]dev[%s]"
				,task_node->GetDevId(),task_node->GetPhyId().c_str()));
			ret = TASK_SUCCESS;
		}
	}
	task->SetNeedResponse();
	return ret;
}

int HDSetFeeRate::do_set_fee_rate(KSGDeviceNode *node,KSGDeviceNode::Task *task,unsigned char *rate_buf,int len)
{
	int ret = 0;
	KSGDeviceNode* task_node = task->GetDeviceNode();
	unsigned short addr = GetDeviceAddress(task_node);
	HD8583STRUCT req;
	req.SetFieldValue(FIELD_ADDRESS,addr);
	req.SetFieldValue(FIELD_ADDITIONALDATA2,rate_buf,len);
	//req.SetFieldValue(FIELD_TERMINALID,task_node->GetTermId());
	char data[1024] = "";
	int packlen;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	MESSAGETYPE msg_type;
	ACE_HANDLE handler = task->_handle;
	if( (packlen = PackRequestStruct(req,MT_SET_FEE_RATE2,data,sizeof data,true)) <= 0)
	{
		ACE_DEBUG((LM_ERROR,"�������ݰ�����!"));
		ret = TASK_ERR_EXECUTE;
	}
	else if( HDSendBuffer(handler,data,packlen,&tv) != 0)
	{
		ACE_DEBUG((LM_ERROR,"���ʹ�������ʧ��!"));
		ret = TASK_ERR_TIMEOUT; 
	}
	else 
	{
		ACE_HEX_DUMP((LM_DEBUG,data,packlen));
		KSGThreadUtil::Sleep(200);
		tv = KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handler,data,sizeof data,&tv)) <= 0)
		{
			ret = TASK_ERR_TIMEOUT;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			ACE_DEBUG((LM_ERROR,"���մ������ݲ���ȷ!"));
			ret = TASK_ERR_EXECUTE;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ACE_DEBUG((LM_INFO,"�����豸���ѱ���ʧ��id[%d]dev[%s],������[%d]"
				,task_node->GetDevId(),task_node->GetPhyId().c_str(),req->ResponseCode));
			ret = TASK_ERR_EXECUTE;
		}
		else
		{
			ACE_DEBUG((LM_INFO,"�����豸���ѱ��ʳɹ�id[%d]dev[%s]"
				,task_node->GetDevId(),task_node->GetPhyId().c_str()));
			ret = TASK_SUCCESS;
		}
	}
	return ret;
}
int IHDSetFeeRate::ExecuteTask(KSGDeviceNode* node,Task* task)
{

	int ret = 0;
	HDSetFeeRate feerate;
	unsigned char right_buf[256];
	memset(right_buf,0,sizeof right_buf);
	KSGDeviceNode* task_node = task->GetDeviceNode();
	DRTPPoolType::SmartObject obj = KsgGetDrtpPool()->Alloc();
	ACE_DEBUG((LM_DEBUG,"�����豸����..."));
	if(obj)
	{
		// ����ǳ�ֵ����
		obj->SetRequestHeader(950043);
		obj->AddField(F_LCERT_CODE,KsgGetGateway()->GetConfig()->_gwId);
		obj->AddField(F_SCUST_LIMIT2,KsgGetGateway()->GetConfig()->_dynaKey.c_str());
		obj->AddField(F_LVOL0,task_node->GetDevId());
		if(obj->Connect())
		{
			// ����ʧ��
			ACE_DEBUG((LM_INFO,"��ֵ��������ʧ��,����DRTPʧ�ܣ�����!!!"));
			return 1;
		}
		// ��������
		if(!obj->SendRequest(3000))
		{
			if(obj->GetReturnCode())
			{
				ACE_DEBUG((LM_ERROR,"�����豸���Ѳ���ʧ��,dev[%d]ret[%d]",
					task_node->GetDevId(),obj->GetReturnCode()));
				return TASK_ERR_EXECUTE;
			}
			while(obj->HasMoreRecord())
			{
				ST_PACK *pack = obj->GetNextRecord();
				int feetype = pack->lvol1;
				int rate = pack->lvol2;
				if( feetype > 0 && feetype < sizeof right_buf)
					right_buf[feetype-1] = rate;
			}
			ACE_DEBUG((LM_DEBUG,"��ʼ���ô���..."));
			return feerate.do_set_fee_rate(node,task,right_buf,sizeof right_buf);
		}
		else
		{
			ACE_DEBUG((LM_ERROR,"�Ӻ�̨��ȡ����ʧ��"));
			return TASK_ERR_TIMEOUT;
		}
	}
	ACE_DEBUG((LM_ERROR,"�����̨����ʧ��"));
	return TASK_ERR_EXECUTE;
}

int IHDSetFeeRate2::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	HDSetFeeRate feerate;
	unsigned char rate_buf[256];
	char temp[3] = "";
	int i;
	std::string cardbuf = task->GetParams().GetParam(XML_KEY_CARDRIGHTTYPE);
	if(cardbuf.length()!=256*2)
		return TASK_ERR_PARAM;

	for(i=0;i<256;++i)
	{
		memcpy(temp,cardbuf.c_str()+i*2,2);
		rate_buf[i] = (unsigned char)strtoul(temp,NULL,16);
	}

	return feerate.do_set_fee_rate(node,task,rate_buf,sizeof rate_buf);
}

int IHDCtlPenStock::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	unsigned char ctlCmd = task->GetParams().GetParamIntVal(XML_KEY_PENSTOCKCMD);

	KSGDeviceNode* task_node = task->GetDeviceNode();
	unsigned long termsn  = ACE_OS::strtoul(task_node->GetPhyId().c_str(),NULL,10);
	unsigned short addr = GetDeviceAddress(task_node);
	HD8583STRUCT req;
	req.SetFieldValue(FIELD_TERMINALSN,termsn);
	req.SetFieldValue(FIELD_ADDRESS,addr);
	req.SetFieldValue(FIELD_TERMINALID,task_node->GetTermId());
	req.SetFieldValue(FIELD_ADDITIONALDATA2,&ctlCmd,1);
	char data[1024] = "";
	int packlen;
	int ret = 0;
	MESSAGETYPE msg_type;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	ACE_HANDLE handler = task->_handle;
	if( (packlen = PackRequestStruct(req,MT_CTRLPENSTOCK,data,sizeof data,true)) <= 0)
	{
		ret = TASK_ERR_EXECUTE;
	}
	else if( HDSendBuffer(handler,data,packlen,&tv) != 0)
	{
		ret = TASK_ERR_TIMEOUT; 
	}
	else 
	{
		KSGThreadUtil::Sleep(50);
		tv = KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handler,data,sizeof data,&tv)) <= 0)
		{
			ret = TASK_ERR_TIMEOUT;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			ret = TASK_ERR_EXECUTE;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ACE_DEBUG((LM_INFO,"Զ��ˮ������ʧ��id[%d]dev[%s],������[%d]"
				,task_node->GetDevId(),task_node->GetPhyId().c_str(),req->ResponseCode));
			ret = TASK_ERR_EXECUTE;
		}
		else
		{
			ACE_DEBUG((LM_INFO,"Զ��ˮ�����Ƴɹ�id[%d]dev[%s]"
				,task_node->GetDevId(),task_node->GetPhyId().c_str()));
			ret = TASK_SUCCESS;
		}
	}
	return ret;

}

int IHDCollPosHisSerial::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	static const int record_per_req = 5;
	int ret = 0;
	unsigned char buf[5] = "";
	unsigned char recordbuf[9] = "";
	int start_no = task->GetParams().GetParamIntVal(XML_KEY_STARTNUM);
	int end_no = task->GetParams().GetParamIntVal(XML_KEY_ENDNUM);
	if(end_no < start_no || start_no < 0)
		return TASK_ERR_EXECUTE;
	int i,j;		
	KSGDeviceNode* task_node = task->GetDeviceNode();
	unsigned short addr = GetDeviceAddress(task_node);
	unsigned long termsn = ACE_OS::strtoul(task_node->GetPhyId().c_str(),NULL,10);
	HD8583STRUCT req;
	char data[1024] = "";
	int packlen;
	int index;
	for(index = 1,i = start_no;i <= end_no;i+=record_per_req,++index)
	{
		if(index > 1)
			KSGThreadUtil::Sleep(100);
		j = i + record_per_req;
		j = (j > end_no) ? (end_no + 1) : j;
		req.Init();
		req.SetFieldValue(FIELD_TERMINALSN,termsn);
		req.SetFieldValue(FIELD_ADDRESS,addr);
		//req.SetFieldValue(FIELD_TERMINALID,task_node->GetTermId());
		buf[0] = j - i;
		req.SetFieldValue(FIELD_ADDITIONALDATA1,buf,1);
		//index = 1;  // �ɼ����
		//INT_2_BUF_LE(index,buf);
		buf[0] = 1;
		req.SetFieldValue(FIELD_ADDITIONALDATA2,buf,sizeof(int));
		INT_2_BUF_LE(i,buf);
		ACE_OS::memcpy(recordbuf,buf,4);
		INT_2_BUF_LE(j,buf);
		ACE_OS::memcpy(recordbuf+4,buf,4);
		req.SetFieldValue(FIELD_ADDITIONALDATA3,recordbuf,8);
		ACE_OS::memset(data,0,sizeof data);
		ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
		tv += ACE_Time_Value(2);
		ACE_HANDLE handler = task->_handle;
		MESSAGETYPE msg_type;
		if( (packlen = PackRequestStruct(req,MT_COLLECT_SERIAL1,data,sizeof data,true)) <= 0)
		{
			ACE_DEBUG((LM_ERROR,"PackRequestStruct err��"));
			ret = TASK_ERR_TIMEOUT;
			return TASK_ERR_EXECUTE;
		}
		else
		{
			//ACE_HEX_DUMP(("������ˮ���ͱ���[%s],����[%d]",data,packlen));
			ACE_HEX_DUMP((LM_DEBUG,data,packlen));
			if( HDSendBuffer(handler,data,packlen,&tv) != 0)
			{
				ACE_DEBUG((LM_ERROR,"������ˮ���ͳ�ʱ��"));
				ret = TASK_ERR_TIMEOUT;
			}		
			else
			{
				KSGThreadUtil::Sleep(60 + 30 * (j-i));
				tv = ACE_Time_Value(5);
				packlen = HDRecvBuffer(handler,data,sizeof data,&tv);
				//ACE_HEX_DUMP((LM_DEBUG,data,packlen));
				ACE_DEBUG((LM_DEBUG,"������ˮ�������ݳ���[%d]",packlen));			
				if(packlen <= 0)
				{
					ACE_DEBUG((LM_ERROR,"������ˮ���ճ�ʱ��"));
					ret = TASK_INVALID_CONN;
				}
				else if(UnPackResponseStruct(req,&msg_type,data,packlen))
				{
					ACE_DEBUG((LM_ERROR,"������ˮ���ݰ�����"));
					ret = TASK_ERR_EXECUTE;
				}
				else if(req->ResponseCode != RC_SUCCESS
					//||ACE_OS::memcmp(req->AdditionalData2,buf,sizeof(int))
					)
				{
					// ������ɹ����߷�����ˮ�Ų�һ��,��Ϊʧ��
					ACE_DEBUG((LM_NOTICE,"����POS����ˮʧ��,dev[%s],serialno[%d]return[%d]"
						,task_node->GetPhyId().c_str(),i,req->ResponseCode));
					ret = TASK_ERR_EXECUTE;
				}
				else
				{
					unsigned char serial_buf[100] = "";
					int offset;
					int teplen=HDRecordFileReader::RECORD_BUF_LEN;
					HDRecordFileReader reader("",0);
					reader.bColHisSeri = true;
					int datalen = req->LenOfAdditionalData1;
					for(offset = 0;offset < datalen;
						offset+=teplen)
					{
						if(offset + teplen > datalen)
							break;
						ACE_DEBUG((LM_DEBUG,"������ˮ�ɹ�:��ʼ��ˮ��[%d],������ˮ��[%d]",i,j));
						ACE_OS::memcpy(serial_buf,req->AdditionalData1 + offset,teplen);
						if(reader.SavePosRecord(serial_buf))
						{
							// ������ˮʧ��
							ACE_DEBUG((LM_ERROR,"�ϴ���ˮʧ��:��ʼ��ˮ��[%d],������ˮ��[%d]",i,j));
							ret = TASK_ERR_EXECUTE;
							break;
						}
					}
					// �ϴ���ˮ
					if(offset >= req->LenOfAdditionalData1)
					{
						ret = TASK_SUCCESS;
					}
				}
			}
		}
		if(ret != TASK_SUCCESS)
			break;
	}
	return ret;
}

int IHDDownloadConCode::parse_time_buf(const char* time_str,int str_len,unsigned char *buf)
{
	int i;
	short hour,minute,total_minute;
	char tmp[3] = "";
	int len = str_len/4*4;
	if(len != str_len)
		return -1;
	for(i=0;i<len/4;++i)
	{
		strncpy(tmp,time_str+i*4,2);
		hour = atoi(tmp);
		strncpy(tmp,time_str+i*4+2,2);
		minute = atoi(tmp);
		total_minute = hour * 60 + minute;
		SHORT_2_BUF_LE(total_minute,buf+i*2);
	}
	return 0;
}
int IHDDownloadConCode::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	KSGDeviceNode *task_node = node;
	if(task->GetDeviceNode() != node)
	{
		task_node = task->GetDeviceNode();
	}
	std::string concode;
	try
	{
		concode = task->GetParams().GetParam(XML_KEY_CONCODE);
	}
	catch (...)
	{
		return TASK_ERR_PARAM;
	}
	int ret;
	unsigned char concode_buf[32]="";
	unsigned char tmp[2];
	int len = concode.length() - concode.length() % 2;
	len = (len > 32) ? 32 : len;

	ret = parse_time_buf(concode.c_str(),len,concode_buf);
	if(ret)
	{
		ACE_DEBUG((LM_ERROR,"�ʹ�ʱ��δ���!"));
		return TASK_ERR_PARAM;
	}
	
	unsigned short addr = GetDeviceAddress(task_node);
	HD8583STRUCT req;
	unsigned long sn = ACE_OS::strtoul(task_node->GetPhyId().c_str(),NULL,10);
	tmp[0] = 0;
	req.SetFieldValue(FIELD_TERMINALSN,sn);
	req.SetFieldValue(FIELD_ADDRESS,addr);
	req.SetFieldValue(FIELD_ADDITIONALDATA1,tmp,1);
	req.SetFieldValue(FIELD_ADDITIONALDATA2,concode_buf,sizeof concode_buf);
	req.SetFieldValue(FIELD_TERMINALID,task_node->GetTermId());
	char data[1024] = "";
	int packlen;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	MESSAGETYPE msg_type;
	ACE_HANDLE handler = task->_handle;
	if( (packlen = PackRequestStruct(req,MT_SETCARDTIME2,data,sizeof data,true)) <= 0)
	{
		ret = TASK_ERR_EXECUTE;
	}
	else if( HDSendBuffer(handler,data,packlen,&tv) != 0)
	{
		ret = TASK_ERR_TIMEOUT;
	}
	else 
	{
		//ACE_HEX_DUMP((LM_INFO,data,packlen));
		KSGThreadUtil::Sleep(100);
		tv = KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handler,data,sizeof data,&tv)) <= 0)
		{
			ret = TASK_ERR_TIMEOUT;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			ret = TASK_ERR_EXECUTE;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ret = TASK_ERR_EXECUTE;
		}
		else
			ret = TASK_SUCCESS;
	}
	return ret;
}
// ���ղ���930077���ܺ�ʱ����
int IHDDLSubsidyFile::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	KSGDeviceNode *task_node = node;
	if(task->GetDeviceNode() != node)
	{
		task_node = task->GetDeviceNode();
	}
	DRTPPoolType::SmartObject obj = KsgGetDrtpPool()->Alloc();
	if(obj)
	{
		// ���̨����������
		obj->SetRequestHeader(849007);
		obj->AddField(F_LCERT_CODE,KsgGetGateway()->GetConfig()->_gwId);
		obj->AddField(F_LVOL0,task_node->GetDevId());
		if(obj->Connect())
		{
			// ����ʧ��
			ACE_DEBUG((LM_INFO,"����DRTPʧ�ܣ�����!!!"));
			return TASK_ERR_EXECUTE;
		}
		// ������ˮ
		if(!obj->SendRequest(3000))
		{
			if(obj->GetReturnCode())
			{
				ACE_DEBUG((LM_ERROR,"��ѯ��������ʧ��[%d]������[%d]"
					,task_node->GetDevId(),obj->GetReturnCode()));
				return TASK_ERR_EXECUTE;
			}
			else
			{
				// ���÷���ֵ
				HD_Subsidy_File_Gen gen;
				std::string data_file;
				ST_PACK *data = NULL;
				// ��鲹���ļ�·��
				if(check_subsidy_dir(task_node->GetPhyId().c_str(),data_file))
				{
					ACE_DEBUG((LM_ERROR,"��鲹���ļ�·��ʧ��"));
					return TASK_ERR_EXECUTE;
				}
				data_file += "\\AllowanceFile.dat";
				if(obj->HasMoreRecord())
				{
					data = obj->GetNextRecord();
					// ���������ļ�
					if(gen.open_data_file(data_file.c_str(),data->lvol1))
					{
						ACE_DEBUG((LM_ERROR,"���������ļ�ʧ��[%s]!",data_file.c_str()));
						return TASK_ERR_EXECUTE;
					}
				}
				// д��һ����
				if(gen.put_one_pack(data))
				{
					ACE_DEBUG((LM_ERROR,"д�벹�������ļ�ʧ��![%s]",data_file.c_str()));
					return TASK_ERR_EXECUTE;
				}
				// ȡ��������д�ļ�
				while(obj->HasMoreRecord())
				{
					data = obj->GetNextRecord();
					// ���ɲ����ļ�
					if(gen.put_one_pack(data))
					{
						ACE_DEBUG((LM_ERROR,"д�벹�������ļ�ʧ��![%s]",data_file.c_str()));
						return TASK_ERR_EXECUTE;
					}
				}
				// ��ȫ�ļ�
				gen.finish();
				// ׼�� ftp �ϴ�
				std::string ipstr = "ftp://" + node->GetDevAddr().GetConnect() + ":";
				ipstr += hd_ccu_ftp_port;
				//ipstr += "/AllowanceFile.dat";
				//std::string to_file(HDA_CCUPATH_RECORD);
				//to_file += "AllowanceFile.dat";
				ipstr += "/AllowanceFile.dat";
				ftp_upload_conf_t conf = {ipstr.c_str(),
				hd_ccu_ftp_user_name.c_str(),
				hd_ccu_ftp_pswd.c_str(),
				data_file.c_str(),5};
				if(KSGNetUtil::ftp_upload_file(&conf)) // FTP �ϴ��ļ�
				{
					ACE_DEBUG((LM_ERROR,"�ϴ���������ʧ��!CCU[%s]",task_node->GetPhyId().c_str()));
					return TASK_ERR_EXECUTE;
				}
				ACE_DEBUG((LM_INFO,"�·������ɹ�!CCU[%s]",task_node->GetPhyId().c_str()));
				return TASK_SUCCESS;
			}
		}
	}
	return TASK_ERR_EXECUTE;
}

int IHDDLSubsidyFile::check_subsidy_dir(const char *phyno,std::string &out_path)
{
	std::string basepath = KsgGetGateway()->GetConfig()->_basedir;
	basepath = KSGOSUtil::JoinPath(basepath,"subsidy/");
	ACE_DIR* dir = ACE_OS::opendir(basepath.c_str());
	if(!dir)
	{
		if(ACE_OS::mkdir(basepath.c_str()))
		{
			ACE_DEBUG((LM_ERROR,"����Ŀ¼ʧ��[%s]",basepath.c_str()));
			return -1;
		}
	}
	else
		ACE_OS::closedir(dir);
	basepath = KSGOSUtil::JoinPath(basepath,phyno);
	dir = ACE_OS::opendir(basepath.c_str());
	if(!dir)
	{
		if(ACE_OS::mkdir(basepath.c_str()))
		{
			ACE_DEBUG((LM_ERROR,"����Ŀ¼ʧ��[%s]",basepath.c_str()));
			return -1;
		}
	}
	else 
		ACE_OS::closedir(dir);
	out_path = basepath;
	return 0;
}

ST_PACK HD_Subsidy_File_Gen::g_out_pack;
int HD_Subsidy_File_Gen::g_card_idx_range[] = {
	sizeof(HD_Subsidy_File_Gen::g_out_pack.usset0)-2,
	sizeof(HD_Subsidy_File_Gen::g_out_pack.usset0)-2,
	sizeof(HD_Subsidy_File_Gen::g_out_pack.usset0)-2,
	sizeof(HD_Subsidy_File_Gen::g_out_pack.usset0)-2,
	sizeof(HD_Subsidy_File_Gen::g_out_pack.usset0)-2,
	sizeof(HD_Subsidy_File_Gen::g_out_pack.usset0)-2,
	sizeof(HD_Subsidy_File_Gen::g_out_pack.usset0)-2,
	(sizeof(HD_Subsidy_File_Gen::g_out_pack.scusttypes) - 1),
	(sizeof(HD_Subsidy_File_Gen::g_out_pack.ssectypes) - 1),
	(sizeof(HD_Subsidy_File_Gen::g_out_pack.vsmess) - 2),
	(sizeof(HD_Subsidy_File_Gen::g_out_pack.vsmess) - 2),
	(sizeof(HD_Subsidy_File_Gen::g_out_pack.vsmess) - 2),
	(sizeof(HD_Subsidy_File_Gen::g_out_pack.vsmess) - 2),
	(sizeof(HD_Subsidy_File_Gen::g_out_pack.vsmess) - 2),
};

int HD_Subsidy_File_Gen::get_pack_index(int card_idx)
{
	int idx = (card_idx-1) / get_card_count_per_pack();
	return idx+1;
}

int HD_Subsidy_File_Gen::get_card_count_per_pack()
{
	int count,i;
	count = 0;
	for(i = 0;i < sizeof(g_card_idx_range)/sizeof(int);++i)
	{
		count += g_card_idx_range[i];
	}
	return count*4;
}



HD_Subsidy_File_Gen::~HD_Subsidy_File_Gen()
{
	if(_fp)
	{
		fclose(_fp);
		_fp = NULL;
	}
}

int HD_Subsidy_File_Gen::open_data_file(const char *file_path,int seqno)
{
	if(_fp)
	{
		ACE_OS::fclose(_fp);
		_fp = NULL;
		if(ACE_OS::unlink(file_path))
			return -1;
	}
	if((_fp = ACE_OS::fopen(file_path,"wb+")) == NULL)
		return -1;
	_pack_index = 0;
	ACE_OS::fseek(_fp,0L,SEEK_SET);
	unsigned short seq = static_cast<unsigned short>(seqno);
	if(ACE_OS::fwrite(&seq,sizeof(seq),1,_fp) != 1)
		return -1;
	return 0;
}

int HD_Subsidy_File_Gen::put_one_pack(ST_PACK * data)
{
	if(!_fp)
		return -1;
	if(write_buffer((const char*)data->usset0,98))
		return -1;
	if(write_buffer((const char*)data->usset1,98))
		return -1;
	if(write_buffer((const char*)data->usset2,98))
		return -1;
	if(write_buffer((const char*)data->usset3,98))
		return -1;
	if(write_buffer((const char*)data->usset4,98))
		return -1;
	if(write_buffer((const char*)data->usset5,98))
		return -1;
	if(write_buffer((const char*)data->usset6,98))
		return -1;
	if(write_buffer(data->scusttypes,200))
		return -1;
	if(write_buffer(data->ssectypes,200))
		return -1;
	if(write_buffer(data->vsmess,254))
		return -1;
	if(write_buffer(data->vsvarstr0,254))
		return -1;
	if(write_buffer(data->vsvarstr1,254))
		return -1;
	if(write_buffer(data->vsvarstr2,254))
		return -1;
	if(write_buffer(data->vsvarstr3,254))
		return -1;
	_pack_index++;
	return 0;
}

int HD_Subsidy_File_Gen::finish()
{
	if(!_fp)
		return -1;
	int last_card_count = MAX_CARD_COUNT - (_pack_index * get_card_count_per_pack());
	ssize_t last_pack = last_card_count / 8;
	ssize_t write_len = 0;
	ssize_t written_len = 0;
	ssize_t ret_len;
	unsigned char buf[1024];
	ACE_OS::memset(buf,0,sizeof buf);
	do 
	{
		write_len = ((last_pack - written_len) > 1024) ? 1024 : (last_pack - written_len);
		ret_len = ACE_OS::fwrite(buf,write_len,1,_fp);
		if(ret_len < 1)
		{
			if(ferror(_fp))
				return -1;
		}
		written_len += write_len;
	} while(written_len < last_pack);
	ACE_OS::fclose(_fp);
	_fp = NULL;
	return 0;
}

unsigned char HD_Subsidy_File_Gen::char_to_hex(char c)
{
	if(c >='0' && c <='9')
		return (c - '0');
	else if(c >='A' && c <= 'F')
		return (c - 'A' + 10);
	else if(c >= 'a' && c <= 'f' )
		return (c - 'a' + 10);
	else
		return 0;
}
int HD_Subsidy_File_Gen::write_buffer(const char *buffer,int buf_len)
{
	unsigned char tmp[512];
	size_t write_len;
	int i,len,count;
	len = buf_len/2;
	count = 0;
	do 
	{
		// add 2007-8-16
		// ��Ҫ��ʼ��һ��
		ACE_OS::memset(tmp,0,sizeof tmp);
		for(i = 0;count < len && i < sizeof(tmp) ;++i,++count)
		{
			tmp[i] = (char_to_hex(buffer[i*2]) << 4) |
				(char_to_hex(buffer[i*2+1]));
		}
		if(i > 0)
		{
			write_len = ACE_OS::fwrite(tmp,i,1,_fp);
			if(write_len < 1)
				return -1;
		}
	} while(count < len);
	return 0;
}
//////////////////////////////////////////////////////////////////////
// HDCCUListenScheduler
HDCCUListenScheduler::HDCCUListenScheduler():KSGScheduler(KSG_SCHD_HD_TCP_SVR)
{
}

HDCCUListenScheduler::~HDCCUListenScheduler()
{

}

void HDCCUListenScheduler::StartListen()
{
	std::string value;
	if(!KsgGetSystemParam(HD_CCU_FTP_USER,value))
	{
		hd_ccu_ftp_user_name = value;
	}
	
	if(!KsgGetSystemParam(HD_CCU_FTP_PSWD,value))
	{
		hd_ccu_ftp_pswd = value;
	}
	if(!KsgGetSystemParam(HD_CCU_FTP_PORT,value))
	{
		hd_ccu_ftp_port = value;
	}
	//ACE_DEBUG((LM_DEBUG,"FTP user[%s],pwd[%s],port[%s]"
	//	,hd_ccu_ftp_user_name.c_str(),hd_ccu_ftp_pswd.c_str()
	//	,hd_ccu_ftp_port.c_str()));
}
void HDCCUListenScheduler::Run()
{
	CCUSvrAcceptor acceptor;
	REACTOR::instance()->owner(ACE_OS::thr_self());
	// �˿ڴӺ�̨�ж�ȡ
	std::string value;
	int port = 6001;
	if(KsgGetSystemParam(HD_SYSPARAM_LISTEN_PORT,value))
	{
		ACE_DEBUG((LM_INFO,"��ȡCCU�����˿ں�ʧ��,ʹ��Ĭ�϶˿�[%d]",port));
	}
	else
	{
		port = ACE_OS::atoi(value.c_str());
	}
	StartListen();
	if(acceptor.open(ACE_INET_Addr(port),REACTOR::instance()
		,0,1,0) == -1 )
	{
		int err = ACE_OS::last_error();
		ACE_DEBUG((LM_ERROR,"���CCU������������ʧ��[%d][%s]",err,
				   ACE_OS::strerror(err)));
		return;
	}
	// FIXME : �������ʼ��  curl ��̫����,��Ҫ�Ľ�
	curl_global_init(CURL_GLOBAL_DEFAULT);
	ACE_DEBUG((LM_INFO,"CCU�����߳�ThreadId[%d]�˿�[%d]\n",ACE_OS::thr_self(),port));
	int ret = 0;
	try
	{
		while(true)
		{
			ACE_Time_Value t(5,0);
			REACTOR::instance()->handle_events(t);
			Notify();
			if(IsTerminated())
			{
				// �����ڹر�����ʱ��ϵͳ���ڴ���
				while(!REACTOR::instance()->reactor_event_loop_done())
					REACTOR::instance()->end_reactor_event_loop();
				break;
			}
		}
	}
	catch(...)
	{
		// ignore all exception
	}
	curl_global_cleanup();
	acceptor.close();
}

// �����Ž�Ȩ�ޱ�
int IHDBatchDownloadWhiteCard::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	ACE_DEBUG((LM_DEBUG,"BatchDownloadWhiteCard"));

	KSGDeviceNode *task_node = node;
	static const std::string node_param = "whitecardver";
	static const int card_length = 12;
	std::string v;
	int maxid = 0,card_idx,i;
	if(task->GetDeviceNode() != node)
	{
		task_node = task->GetDeviceNode();
	}
	if(task_node == NULL)
		return TASK_ERR_EXECUTE;

	unsigned short addr = GetDeviceAddress(task_node);

	int ret,card_ver=0;
	unsigned char whitebuf[512]="";
	char tmp[20];

	// 1. �ȴӺ�̨��ѯ�����صĶ���
	DRTPPoolType::SmartObject obj = KsgGetDrtpPool()->Alloc();
	if(!obj)
	{
		return TASK_ERR_EXECUTE;
	}
	/*
	if(task_node->get_param(node_param,v))
		card_ver = 0;
	else
		card_ver = atoi(v.c_str());
	*/
	obj->SetRequestHeader(950105);
	obj->AddField(F_LCERT_CODE,KsgGetGateway()->GetConfig()->_gwId);
	obj->AddField(F_LVOL0,task_node->GetDevId());
	obj->AddField(F_LVOL1,10);
	//obj->AddField(F_LVOL2,card_ver);
/*
	if(task_node->get_param("doordlid",v)==0)
	{
		maxid = ACE_OS::atoi(v.c_str());
	}
	obj->AddField(F_LVOL2,maxid);
*/
	ACE_DEBUG((LM_DEBUG,"sysid[%d]",KsgGetGateway()->GetConfig()->_gwId));
	ACE_DEBUG((LM_DEBUG,"devid[%d]",task_node->GetDevId()));
	ACE_DEBUG((LM_DEBUG,"card_ver[%d]",card_ver));
	ACE_DEBUG((LM_DEBUG,"maxid[%d]",maxid));

	if(obj->Connect())
	{
		// ����ʧ��
		ACE_DEBUG((LM_INFO,"����DRTPʧ�ܣ�����!!!"));
		return TASK_ERR_EXECUTE;
	}
	if(!obj->SendRequest(5000))
	{
		if(!obj->GetReturnCode())
		{
			
		}
		else
		{
			ACE_DEBUG((LM_ERROR,"���ػ���Ž�����ʧ�ܣ�����[%d][%s]",
				obj->GetReturnCode(),obj->GetReturnMsg().c_str()));
			return TASK_ERR_EXECUTE;
		}
	}
	else
	{
		ACE_DEBUG((LM_ERROR,"���ػ���Ž��������������󵽺�̨ʧ��"));
		return TASK_ERR_EXECUTE;
	}
	card_ver = 0;
	card_idx = 0;
	memset(tmp,0,sizeof tmp);
	while(obj->HasMoreRecord() && card_idx < 10)
	{
		ST_PACK *data = obj->GetNextRecord();
		int doorcount = data->lvol10;
		//xutil::StringUtil::Str2Hex(data->sdate0,whitebuf+card_idx*card_length,8);
		card_ver = data->lvol0;

		unsigned char temp[4];
		INT_2_BUF_LE(data->lvol3,temp);								// ���׿���
		memcpy(whitebuf+card_idx*card_length,temp,4);

		if(doorcount >0)
		{
			// GCU ���֧�� 4 ��
			for(i =0;i < 4;++i)
			{
				unsigned char b=0;
				memcpy(tmp,data->vsvarstr0+(i+1)*2,2);
				int t = (int)strtoul(tmp,NULL,16);
				if(t == 0xFF )
					b = 0x00;
				else if(t > 127)
				{
					ACE_DEBUG((LM_ERROR,"����Ž�GCU[%s],����ʱ�����������127",task_node->get_name().c_str()));
				}
				else
				{
					//b = t & 0xFF;
					b = (t+1) | 0x80;
				}
				whitebuf[card_idx*card_length+4+i] = b;
			}
			whitebuf[card_idx*card_length+11] = 1;		// ����
		}
		else
		{
			whitebuf[card_idx*card_length+11] = 0;		// ɾ��
		}
		card_idx++;

		ACE_DEBUG((LM_DEBUG,"doorcount[%d]",doorcount));
		ACE_DEBUG((LM_DEBUG,"card_ver[%d]",card_ver));
		ACE_DEBUG((LM_DEBUG,"card_id[%d]",data->lvol3));
		ACE_DEBUG((LM_DEBUG,"rightbit[%s]",data->vsvarstr0));
		ACE_HEX_DUMP((LM_DEBUG,(char*)whitebuf,card_idx * card_length));
	}
	HD8583STRUCT req;
	memset(tmp,0,sizeof tmp);
	unsigned long sn = DecodeTermSN(task_node->GetPhyId().c_str());
	tmp[0] = 0;
	req.SetFieldValue(FIELD_TERMINALSN,sn);
	req.SetFieldValue(FIELD_ADDRESS,addr);
	req.SetFieldValue(FIELD_TERMINALID,task_node->GetTermId());
	req.SetFieldValue(FIELD_VERSIONOFLIST,card_ver);
	tmp[0] = 0x01;
	//req.SetFieldValue(FIELD_ADDITIONALDATA1,tmp,4);
	req.SetFieldValue(FIELD_ADDITIONALDATA2,whitebuf,card_idx * card_length);
	req.SetFieldValue(FIELD_ADDITIONALDATA3,tmp,1);
	char data[1024] = "";
	int packlen;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	MESSAGETYPE msg_type;
	ACE_HANDLE handler = task->_handle;
	if( (packlen = PackRequestStruct(req,MT_UPDATEACCESSTABLE2,data,sizeof data,true)) <= 0)
	{
		ACE_DEBUG((LM_ERROR,"PackRequestStruct err"));
		ret = TASK_ERR_EXECUTE;
	}
	else if( HDSendBuffer(handler,data,packlen,&tv) != 0)
	{
		ACE_DEBUG((LM_ERROR,"HDSendBuffer err"));
		ret = TASK_ERR_TIMEOUT;
	}
	else 
	{
		//ACE_HEX_DUMP((LM_INFO,data,packlen));
		KSGThreadUtil::Sleep(100);
		tv = KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handler,data,sizeof data,&tv)) <= 0)
		{
			ACE_DEBUG((LM_ERROR,"HDRecvBuffer err"));
			ret = TASK_ERR_TIMEOUT;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			ACE_DEBUG((LM_ERROR,"UnPackResponseStruct err"));
			ret = TASK_ERR_EXECUTE;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ACE_DEBUG((LM_ERROR,"ResponseCode [%d]",req->ResponseCode));
			ret = TASK_ERR_EXECUTE;
		}
		else
		{
			ret = TASK_SUCCESS;
			/*
			sprintf(tmp,"%d",card_ver);
			task_node->set_param(node_param,tmp);
			ACE_DEBUG((LM_INFO,"����Ž�GCU[%s],�����Ž������ɹ�ver[%d]",
				task_node->get_name().c_str(),card_ver));
			*/
			// �������سɹ���֪ͨ��̨��������״̬
			obj->SetRequestHeader(950105);
			obj->AddField(F_LCERT_CODE,KsgGetGateway()->GetConfig()->_gwId);
			obj->AddField(F_LVOL0,task_node->GetDevId());
			//obj->AddField(F_LVOL1,10);
			obj->AddField(F_LVOL2,card_ver);

			if(!obj->SendRequest(5000))
			{				
				ACE_DEBUG((LM_DEBUG,"���ػ���Ž������ɹ�,��̨����״̬ver[%d]",card_ver));				
			}
		}
	}
	return ret;
}

int IHDBatchDownloadKQCard::GetBlkVer(KSGDeviceNode *task_node,Task* task,int &blkver)
{
	int ret = 0;
	unsigned char tmp[4]="";
	unsigned char datetime[8]="";
	HD8583STRUCT req;

	time_t now = ACE_OS::gettimeofday().sec();
	struct tm* tv = ACE_OS::localtime(&now);
	datetime[0]=tv->tm_year-100;
	datetime[1]=tv->tm_mon+1;
	datetime[2]=tv->tm_mday;
	datetime[3]=tv->tm_hour;
	datetime[4]=tv->tm_min;
	datetime[5]=tv->tm_sec;
	datetime[6]=(tv->tm_wday+7)%7;

	tmp[0]=1;
	unsigned short addr = GetDeviceAddress(task_node);	
	unsigned long sn = DecodeTermSN(task_node->GetPhyId().c_str());
	req.SetFieldValue(FIELD_TERMINALSN,sn);
	req.SetFieldValue(FIELD_ADDRESS,addr);
	req.SetFieldValue(FIELD_DATEANDTIME,(char*)datetime);
	req.SetFieldValue(FIELD_VERSIONOFLIST,(char*)tmp);
	char data[1024] = "";
	int packlen;
	ACE_Time_Value atv = KSGGetTaskTimeoutIntval();
	MESSAGETYPE msg_type;
	ACE_HANDLE handler = task->_handle;
	if( (packlen = PackRequestStruct(req,MT_ONLINENOTICE2,data,sizeof data,true)) <= 0)
	{
		ret = TASK_ERR_EXECUTE;
	}
	else if( HDSendBuffer(handler,data,packlen,&atv) != 0)
	{
		ret = TASK_ERR_TIMEOUT;
	}
	else 
	{
		//ACE_HEX_DUMP((LM_INFO,data,packlen));
		KSGThreadUtil::Sleep(100);
		atv = KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handler,data,sizeof data,&atv)) <= 0)
		{
			ret = TASK_ERR_TIMEOUT;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			ret = TASK_ERR_EXECUTE;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ret = TASK_ERR_EXECUTE;
		}
		else
		{
			ret = TASK_SUCCESS;
			blkver = req->VerOfList;
		}
	}
	return ret;
	
}
// ���ؿ�����������������
int IHDBatchDownloadKQCard::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	KSGDeviceNode *task_node = node;
	static const std::string node_param = "whitecardver";
	static const int card_length = 5;
	std::string v;
	int maxid = 0,card_idx;
	if(task->GetDeviceNode() != node)
	{
		task_node = task->GetDeviceNode();
	}
	if(task_node == NULL)
		return TASK_ERR_EXECUTE;

	unsigned short addr = GetDeviceAddress(task_node);

	int ret,card_ver=0;
	unsigned char whitebuf[512]="";
	char tmp[20];

	// 1. �ȴӺ�̨��ѯ�����صĶ���
	DRTPPoolType::SmartObject obj = KsgGetDrtpPool()->Alloc();
	if(!obj)
	{
		return TASK_ERR_EXECUTE;
	}

	if(task_node->get_param(node_param,v))
	{
		if((ret = GetBlkVer(task_node,task,card_ver)) !=0)
			return ret;
	}
	else
		card_ver = atoi(v.c_str());

	card_ver++;

	obj->SetRequestHeader(950105);
	obj->AddField(F_LCERT_CODE,KsgGetGateway()->GetConfig()->_gwId);
	obj->AddField(F_LVOL0,task_node->GetDevId());
	obj->AddField(F_LVOL1,10);
	/*	
	obj->AddField(F_LVOL2,card_ver);
	if(task_node->get_param("doordlid",v)==0)
	{
		maxid = ACE_OS::atoi(v.c_str());
	}
	obj->AddField(F_LVOL2,maxid);
	*/
	if(obj->Connect())
	{
		// ����ʧ��
		ACE_DEBUG((LM_INFO,"����DRTPʧ�ܣ�����!!!"));
		return TASK_ERR_EXECUTE;
	}
	if(!obj->SendRequest(5000))
	{
		if(!obj->GetReturnCode())
		{

		}
		else
		{
			ACE_DEBUG((LM_ERROR,"���ػ�࿼�ڻ�����ʧ�ܣ�����[%d][%s]",
				obj->GetReturnCode(),obj->GetReturnMsg().c_str()));
			return TASK_ERR_EXECUTE;
		}
	}

	card_idx = 0;
	memset(tmp,0,sizeof tmp);
	while(obj->HasMoreRecord() && card_idx < 10)
	{
		ST_PACK *data = obj->GetNextRecord();
		int doorcount = data->lvol10;
	//	xutil::StringUtil::Str2Hex(data->sdate0,whitebuf+card_idx*card_length,8);
		//card_ver = data->lvol0;
		if(doorcount >0)
		{
			whitebuf[card_idx*card_length] = 0x01;
		}
		else
		{
			whitebuf[card_idx*card_length] = 0x00;
		}
		unsigned char temp[4];
		INT_2_BUF_LE(data->lvol3,temp);								// ���׿���
		memcpy(whitebuf+card_idx*card_length+1,temp,4);
		card_idx++;
	}
	HD8583STRUCT req;
	memset(tmp,0,sizeof tmp);
	unsigned long sn = DecodeTermSN(task_node->GetPhyId().c_str());
	
	req.SetFieldValue(FIELD_TERMINALSN,sn);
	req.SetFieldValue(FIELD_ADDRESS,addr);
	req.SetFieldValue(FIELD_TERMINALID,task_node->GetTermId());
	req.SetFieldValue(FIELD_VERSIONOFLIST,card_ver);
	req.SetFieldValue(FIELD_ADDITIONALDATA2,whitebuf,card_idx * card_length);
	tmp[0] = 1;
	req.SetFieldValue(FIELD_ADDITIONALDATA3,tmp,1);
	char data[1024] = "";
	int packlen;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	MESSAGETYPE msg_type;
	ACE_HANDLE handler = task->_handle;
	if( (packlen = PackRequestStruct(req,MT_UPDATELIST2,data,sizeof data,true)) <= 0)
	{
		ret = TASK_ERR_EXECUTE;
	}
	else if( HDSendBuffer(handler,data,packlen,&tv) != 0)
	{
		ret = TASK_ERR_TIMEOUT;
	}
	else 
	{
		//ACE_HEX_DUMP((LM_INFO,data,packlen));
		KSGThreadUtil::Sleep(100);
		tv = KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handler,data,sizeof data,&tv)) <= 0)
		{
			ret = TASK_ERR_TIMEOUT;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			ret = TASK_ERR_EXECUTE;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ret = TASK_ERR_EXECUTE;
		}
		else
		{
			ret = TASK_SUCCESS;
			
			sprintf(tmp,"%d",card_ver);
			task_node->set_param(node_param,tmp);

			obj->SetRequestHeader(950105);
			obj->AddField(F_LCERT_CODE,KsgGetGateway()->GetConfig()->_gwId);
			obj->AddField(F_LVOL0,task_node->GetDevId());
			//obj->AddField(F_LVOL1,10);
			obj->AddField(F_LVOL2,card_ver);

			if(!obj->SendRequest(5000))
			{				
				ACE_DEBUG((LM_DEBUG,"���ػ�࿼�������ɹ�,��̨����״̬ver[%d]",card_ver));				
			}
		}
	}
	return ret;
}


int ID_GCU_DL_Timesect::get_day_time(unsigned char *zoneinfo,Task* task,const char *param_name)
{
	// ����Ϊ��λ
	std::string day_time("");
	char str_hh[3]="";
	char str_mm[3]="";
	BYTE str_begin[3]="";
	BYTE str_end[3]="";
	short begin_time=0,end_time=0;
	try
	{
		day_time = task->GetParams().GetParam(param_name);
		if(day_time.length()==8)
		{	
			day_time.copy(str_hh,2);			// ��ʼʱ��
			day_time.copy(str_mm,2,2);
			begin_time = atoi(str_hh)*60 +atoi(str_mm);

			day_time.copy(str_hh,2,4);			// ����ʱ��
			day_time.copy(str_mm,2,6);
			end_time = atoi(str_hh)*60 + atoi(str_mm);
		}

		SHORT_2_BUF_LE(begin_time,str_begin);	
		SHORT_2_BUF_LE(end_time,str_end);	
		memcpy(zoneinfo,str_begin,2);
		memcpy(zoneinfo+2,str_end,2);
	}
	catch (...)
	{
		return TASK_ERR_PARAM;
	}
	return 0;
}

int ID_GCU_DL_Timesect::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	int ret = 0,seqno,timesect_no=0;	
	unsigned char zoneinfo[34]="";
	char temp[8]="";
	std::string v;
	static const std::string param_timesect = "param_timesect_no";		
	KSGDeviceNode *task_node = node;
	if(task->GetDeviceNode() != node)
	{
		task_node = task->GetDeviceNode();
	}
	if(task_node == NULL)
		return TASK_ERR_EXECUTE;

	ret=task_node->get_param(param_timesect,v);
	if(ret)
		timesect_no=1;
	else
		timesect_no=atoi(v.c_str())+1;

	sprintf(temp,"%d",timesect_no);
	task_node->set_param(param_timesect,temp);

	unsigned short addr = GetDeviceAddress(task_node);


	seqno = task->GetParams().GetParamIntVal(XML_KEY_SEQNO);
	zoneinfo[0] = seqno+1;				//255��ʱ���飨1-255��
	get_day_time(zoneinfo+1,task,XML_KEY_DOORTIME1);
	get_day_time(zoneinfo+5,task,XML_KEY_DOORTIME2);
	get_day_time(zoneinfo+9,task,XML_KEY_DOORTIME3);
	get_day_time(zoneinfo+13,task,XML_KEY_DOORTIME4);
	get_day_time(zoneinfo+17,task,XML_KEY_DOORTIME5);
	get_day_time(zoneinfo+21,task,XML_KEY_DOORTIME6);
	get_day_time(zoneinfo+25,task,XML_KEY_DOORTIME7);
	get_day_time(zoneinfo+29,task,XML_KEY_DOORTIME8);

	
	HD8583STRUCT req;
	char tmp[5]="";
	unsigned long sn = DecodeTermSN(task_node->GetPhyId().c_str());

	req.SetFieldValue(FIELD_TERMINALSN,sn);
	req.SetFieldValue(FIELD_ADDRESS,addr);
	req.SetFieldValue(FIELD_TERMINALID,task_node->GetTermId());
	req.SetFieldValue(FIELD_VERSIONOFLIST,timesect_no);
	tmp[0]=1;
	req.SetFieldValue(FIELD_ADDITIONALDATA1,tmp,4);
	req.SetFieldValue(FIELD_ADDITIONALDATA2,zoneinfo,33);	
	req.SetFieldValue(FIELD_ADDITIONALDATA3,tmp,1);
	char data[1024] = "";
	int packlen;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	MESSAGETYPE msg_type;
	ACE_HANDLE handler = task->_handle;
	if( (packlen = PackRequestStruct(req,MT_SETACCESSPERIOD2,data,sizeof data,true)) <= 0)
	{
		ret = TASK_ERR_EXECUTE;
	}
	else if( HDSendBuffer(handler,data,packlen,&tv) != 0)
	{
		ret = TASK_ERR_TIMEOUT;
	}
	else 
	{
		//ACE_HEX_DUMP((LM_INFO,data,packlen));
		KSGThreadUtil::Sleep(100);
		tv = KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handler,data,sizeof data,&tv)) <= 0)
		{
			ret = TASK_ERR_TIMEOUT;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			ret = TASK_ERR_EXECUTE;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ret = TASK_ERR_EXECUTE;
		}
		else
		{
			ret = TASK_SUCCESS;
			ACE_DEBUG((LM_INFO,"����Ž�GCU[%s],�����Ž�ʱ���[%d]�ɹ�",
				task_node->get_name().c_str(),seqno));
		}
	}
 	return ret;
}

int ID_GCU_DL_Week::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	int ret,seqno,day_id,week_no=0;
	unsigned char zoneinfo[9]="";
	char temp[8]="";
	std::string v;
	static const std::string param_weekno = "param_week_no";		

	KSGDeviceNode *task_node = node;
	if(task->GetDeviceNode() != node)
	{
		task_node = task->GetDeviceNode();
	}
	if(task_node == NULL)
		return TASK_ERR_EXECUTE;

	ret=task_node->get_param(param_weekno,v);
	if(ret)
		week_no=1;
	else
		week_no=atoi(v.c_str())+1;

	sprintf(temp,"%d",week_no);
	task_node->set_param(param_weekno,temp);

	unsigned short addr = GetDeviceAddress(task_node);

	try
	{
		seqno = task->GetParams().GetParamIntVal(XML_KEY_SEQNO);
		zoneinfo[0] = seqno+1;		// 127���Ž�����1-127��
		zoneinfo[1] =task->GetParams().GetParamIntVal(XML_KEY_WEEK_DAY7)+1;
		zoneinfo[2] =task->GetParams().GetParamIntVal(XML_KEY_WEEK_DAY1)+1;
		zoneinfo[3] =task->GetParams().GetParamIntVal(XML_KEY_WEEK_DAY2)+1;
		zoneinfo[4] =task->GetParams().GetParamIntVal(XML_KEY_WEEK_DAY3)+1;
		zoneinfo[5] =task->GetParams().GetParamIntVal(XML_KEY_WEEK_DAY4)+1;
		zoneinfo[6] =task->GetParams().GetParamIntVal(XML_KEY_WEEK_DAY5)+1;
		zoneinfo[7] =task->GetParams().GetParamIntVal(XML_KEY_WEEK_DAY6)+1;
	}
	catch (...)
	{
		return TASK_ERR_PARAM;
	}

	HD8583STRUCT req;
	char tmp[5]="";
	unsigned long sn = DecodeTermSN(task_node->GetPhyId().c_str());

	req.SetFieldValue(FIELD_TERMINALSN,sn);
	req.SetFieldValue(FIELD_ADDRESS,addr);
	req.SetFieldValue(FIELD_TERMINALID,task_node->GetTermId());
	req.SetFieldValue(FIELD_VERSIONOFLIST,week_no);
	tmp[0]=1;
	req.SetFieldValue(FIELD_ADDITIONALDATA1,tmp,4);
	req.SetFieldValue(FIELD_ADDITIONALDATA2,zoneinfo,8);	
	req.SetFieldValue(FIELD_ADDITIONALDATA3,tmp,1);
	char data[1024] = "";
	int packlen;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	MESSAGETYPE msg_type;
	ACE_HANDLE handler = task->_handle;
	if( (packlen = PackRequestStruct(req,MT_SETWORKADAYACCESSRULE2,data,sizeof data,true)) <= 0)
	{
		ret = TASK_ERR_EXECUTE;
	}
	else if( HDSendBuffer(handler,data,packlen,&tv) != 0)
	{
		ret = TASK_ERR_TIMEOUT;
	}
	else 
	{
		//ACE_HEX_DUMP((LM_INFO,data,packlen));
		KSGThreadUtil::Sleep(100);
		tv = KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handler,data,sizeof data,&tv)) <= 0)
		{
			ret = TASK_ERR_TIMEOUT;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			ret = TASK_ERR_EXECUTE;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ret = TASK_ERR_EXECUTE;
		}
		else
		{
			ret = TASK_SUCCESS;
			ACE_DEBUG((LM_INFO,"����Ž�GCU[%s],���ù������Ž�����[%d]�ɹ�",
				task_node->get_name().c_str(),seqno));
		}
	}
	return ret;

}

int ID_CTRL_DOOR::ExecuteTask(KSGDeviceNode *node, Task *task)
{
	int ret=0,flag;
	/*
	KSGDeviceNode *task_node = node;
	if(task->GetDeviceNode() != node)
	{
		task_node = task->GetDeviceNode();
	}
	if(task_node == NULL)
		return TASK_ERR_EXECUTE;
	
	int reader = task_node->GetTermId();								// ��ͷ (1--4)

	KSGDeviceNode *task_parent = node->GetParent();
	ACE_ASSERT(task_parent != NULL);

	unsigned short addr = GetDeviceAddress(task_parent);

	try
	{
		// 0:���գ�1��������2����һ��
		flag = task->GetParams().GetParamIntVal(XML_KEY_FTFLAG);
	}
	catch (...)
	{
		return TASK_ERR_PARAM;
	}
	*/
	return ret;
}

// ��ȡ�������汾�ţ����8���ֽڣ�ǰ4����ʾ�������汾�ţ���4����ʾ�������汾��
int IHDDLManageCard::GetBlkVer(KSGDeviceNode *task_node,Task* task,int &blkver1,int &blkver2)
{
	int ret = 0;
	char tmp[9]="";
	unsigned char datetime[8]="";
	HD8583STRUCT req;

	time_t now = ACE_OS::gettimeofday().sec();
	struct tm* tv = ACE_OS::localtime(&now);
	datetime[0]=tv->tm_year-100;
	datetime[1]=tv->tm_mon+1;
	datetime[2]=tv->tm_mday;
	datetime[3]=tv->tm_hour;
	datetime[4]=tv->tm_min;
	datetime[5]=tv->tm_sec;
	datetime[6]=(tv->tm_wday+7)%7;

	tmp[0]=1;
	unsigned short addr = GetDeviceAddress(task_node);	
	unsigned long sn = DecodeTermSN(task_node->GetPhyId().c_str());
	req.SetFieldValue(FIELD_TERMINALSN,sn);
	req.SetFieldValue(FIELD_ADDRESS,addr);
	req.SetFieldValue(FIELD_DATEANDTIME,(char*)datetime);
	req.SetFieldValue(FIELD_VERSIONOFLIST,tmp);
	char data[1024] = "";
	int packlen;
	ACE_Time_Value atv = KSGGetTaskTimeoutIntval();
	MESSAGETYPE msg_type;
	ACE_HANDLE handler = task->_handle;
	if( (packlen = PackRequestStruct(req,MT_ONLINENOTICE2,data,sizeof data,true)) <= 0)
	{
		ret = TASK_ERR_EXECUTE;
	}
	else if( HDSendBuffer(handler,data,packlen,&atv) != 0)
	{
		ret = TASK_ERR_TIMEOUT;
	}
	else 
	{
		//ACE_HEX_DUMP((LM_INFO,data,packlen));
		KSGThreadUtil::Sleep(100);
		atv = KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handler,data,sizeof data,&atv)) <= 0)
		{
			ret = TASK_ERR_TIMEOUT;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			ret = TASK_ERR_EXECUTE;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ret = TASK_ERR_EXECUTE;
		}
		else
		{
			ret = TASK_SUCCESS;
			memcpy(tmp,req->AdditionalData3+27,4);
			BUF_2_INT(blkver1,tmp);
			memcpy(tmp,req->AdditionalData3+31,4);
			BUF_2_INT(blkver2,tmp);
		}
	}
	return ret;

}
int IHDDLManageCard::ExecuteTask(KSGDeviceNode *node, Task *task)
{
	int ret = 0;
	int blkver1 = 0,blkver2 = 0;
	BYTE tmp[8]="";
	char temp[12]="";
	KSGDeviceNode *task_node = node;
	static const std::string node_param1 = "managecardver";			// ������
	static const std::string node_param2 = "oncardver";				// ������
	std::string v;
	if(task->GetDeviceNode() != node)
	{
		task_node = task->GetDeviceNode();
	}
	if(task_node == NULL)
		return TASK_ERR_EXECUTE;

	int devcardtype = task_node->card_type();

	HD8583STRUCT req;
	unsigned short addr = GetDeviceAddress(task_node);
	unsigned long sn = DecodeTermSN(task_node->GetPhyId().c_str());

	req.SetFieldValue(FIELD_TERMINALSN,sn);
	req.SetFieldValue(FIELD_ADDRESS,addr);
	req.SetFieldValue(FIELD_TERMINALID,task_node->GetTermId());

	int usetype = task->GetParams().GetParamIntVal(XML_KEY_USETYPE);
	int adddelflag = task->GetParams().GetParamIntVal(XML_KEY_ADDDELSIGN);
	std::string cardphyid = task->GetParams().GetParam(XML_KEY_CARDPHY);
	tmp[0] = adddelflag;
	
	int cardno = ACE_OS::strtoul(cardphyid.c_str(),NULL,16);
	INT_2_BUF(cardno,tmp+1);
	
	req.SetFieldValue(FIELD_ADDITIONALDATA2,tmp,5);					//����������

	memset(tmp,0,sizeof tmp);
	tmp[0] = usetype;
	req.SetFieldValue(FIELD_ADDITIONALDATA1,tmp,1);					//����ѡ��
	if(!usetype)													// ������
	{
		if(task_node->get_param(node_param1,v))
		{
			if((ret = GetBlkVer(task_node,task,blkver1,blkver2)) !=0)
				return ret;
		}
		else
			blkver1 = atoi(v.c_str());

		blkver1++;
		// �������汾
		req.SetFieldValue(FIELD_VERSIONOFLIST,blkver1);
	}
	else
	{
		if(task_node->get_param(node_param2,v))
		{
			if((ret = GetBlkVer(task_node,task,blkver1,blkver2)) !=0)
				return ret;
		}
		else
			blkver2 = atoi(v.c_str());

		blkver2++;
		// �������汾
		req.SetFieldValue(FIELD_VERSIONOFLIST,blkver2);
	}
	

	tmp[0] = 1;
	req.SetFieldValue(FIELD_ADDITIONALDATA3,tmp,1);					//������־

	char data[1024] = "";
	int packlen;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	MESSAGETYPE msg_type;
	ACE_HANDLE handler = task->_handle;
	if( (packlen = PackRequestStruct(req,MT_DLMANAGECARD,data,sizeof data,true)) <= 0)
	{
		ret = TASK_ERR_EXECUTE;
	}
	else if( HDSendBuffer(handler,data,packlen,&tv) != 0)
	{
		ret = TASK_ERR_TIMEOUT;
	}
	else 
	{
		//ACE_HEX_DUMP((LM_INFO,data,packlen));
		KSGThreadUtil::Sleep(100);
		tv = KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handler,data,sizeof data,&tv)) <= 0)
		{
			ret = TASK_ERR_TIMEOUT;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			ret = TASK_ERR_EXECUTE;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ret = TASK_ERR_EXECUTE;
		}
		else
		{
			ret = TASK_SUCCESS;
			if(!usetype)
			{
				sprintf(temp,"%d",blkver1);
				task_node->set_param(node_param1,temp);
				ACE_DEBUG((LM_INFO,"�����豸[%s],���ù�����[%s]�ɹ�",
					task_node->get_name().c_str(),cardphyid.c_str()));
			}
			else
			{
				sprintf(temp,"%d",blkver2);
				task_node->set_param(node_param2,temp);
				ACE_DEBUG((LM_INFO,"�����豸[%s],���ó�����[%s]�ɹ�",
					task_node->get_name().c_str(),cardphyid.c_str()));
			}
		}
	}

	return ret;
}

// ���ؿ������ò���(����ý��ȡ��)
int IHDDLKQPARAM::ExecuteTask(KSGDeviceNode *node, Task *task)
{
	int ret = 0;
	/*
	BYTE tmp[8]="";
	KSGDeviceNode *task_node = node;
	if(task->GetDeviceNode() != node)
	{
		task_node = task->GetDeviceNode();
	}
	if(task_node == NULL)
		return TASK_ERR_EXECUTE;

	HD8583STRUCT req;
	unsigned short addr = GetDeviceAddress(task_node);
	unsigned long sn = DecodeTermSN(task_node->GetPhyId().c_str());

	req.SetFieldValue(FIELD_TERMINALSN,sn);
	req.SetFieldValue(FIELD_ADDRESS,addr);
	req.SetFieldValue(FIELD_TERMINALID,task_node->GetTermId());

	BYTE onoffflag = 0x00;				// ���ص��־(1byte):0x01��ʾ����״̬, 0x00��ʾ�ر�״̬; 
	BYTE limittime = 45;				// ����ʱ��(1byte):0~240��;���Ƶ������
	BYTE kqmode = 0x00;					// ���ڻ�ģʽ(1byte):0x01��ʾ��ͨ����ģʽ,0x00��ʾ����ģʽ, 0x02��ʾCPU��ģʽ
	BYTE psammode = 0x00;				// PSAM��ģʽ(1byte):0x01��ʾ����PSAM,0x00��ʾ������
	BYTE autoofftime = 180;				// �Զ��رյ�Դʱ��(1byte): 0~240��;

	tmp[0] = onoffflag;
	tmp[1] = limittime;
	tmp[2] = kqmode;
	tmp[3] = psammode;
	tmp[4] = autoofftime;	
	req.SetFieldValue(FIELD_ADDITIONALDATA1,tmp,5);	

	char data[1024] = "";
	int packlen;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	MESSAGETYPE msg_type;
	ACE_HANDLE handler = task->_handle;
	if( (packlen = PackRequestStruct(req,MT_DLKQPARAM,data,sizeof data,true)) <= 0)
	{
		ret = TASK_ERR_EXECUTE;
	}
	else if( HDSendBuffer(handler,data,packlen,&tv) != 0)
	{
		ret = TASK_ERR_TIMEOUT;
	}
	else 
	{
		//ACE_HEX_DUMP((LM_INFO,data,packlen));
		KSGThreadUtil::Sleep(100);
		tv = KSGGetTaskTimeoutIntval();
		if((packlen = HDRecvBuffer(handler,data,sizeof data,&tv)) <= 0)
		{
			ret = TASK_ERR_TIMEOUT;
		}
		else if(UnPackResponseStruct(req,&msg_type,data,packlen))
		{
			ret = TASK_ERR_EXECUTE;
		}
		else if(req->ResponseCode != RC_SUCCESS)
		{
			ret = TASK_ERR_EXECUTE;
		}
		else
		{
			ret = TASK_SUCCESS;
			ACE_DEBUG((LM_INFO,"�����豸[%s],�������ò����ɹ�",
				task_node->get_name().c_str()));
		}
	}
	*/
	return ret;
}


int IHDDLWaterFeeCfg::ExecuteTask(KSGDeviceNode* node,Task* task)
{
	static const int pageSize = 15 * 17;
	static const int feeGroupCnt = 17;
	static const int feeSize = 15;
	int startFee = 0;
	unsigned char feeIndex = 0;
	unsigned char feePage[pageSize];
	int ret;

	KSGDeviceNode *task_node = node;
	if(task->GetDeviceNode() != node)
	{
		task_node = task->GetDeviceNode();
	}
	if(task_node == NULL)
		return TASK_ERR_EXECUTE;

	int feeID = 0;
	try
	{
		feeID = task->GetParams().GetParamIntVal(XML_KEY_FEE);
	}
	catch (...)
	{
		return TASK_ERR_PARAM;
	}


	HD8583STRUCT req;
	unsigned short addr = GetDeviceAddress(task_node);
	unsigned long sn = DecodeTermSN(task_node->GetPhyId().c_str());

	req.SetFieldValue(FIELD_TERMINALSN,sn);
	req.SetFieldValue(FIELD_ADDRESS,addr);
	//req.SetFieldValue(FIELD_TERMINALID,task_node->GetTermId());


	DRTPPoolType::SmartObject obj = KsgGetDrtpPool()->Alloc();
	if(!obj)
	{
		return 1;
	}
	if(obj->Connect())
	{
		// ����ʧ��
		ACE_DEBUG((LM_INFO,"����DRTPʧ�ܣ�����!!!"));
		return 1;
	}
	obj->SetRequestHeader(846405);
	obj->AddField(F_LCERT_CODE,feeID);
	if(obj->SendRequest(5000))
	{
		// ����ָ��ʧ��
		ACE_DEBUG((LM_DEBUG,"����ָ�ʱ�ȴ�����!"));
		return -1;
	}
	if(obj->GetReturnCode())
	{
		ACE_DEBUG((LM_ERROR,"���غ�����ʧ�ܣ�����[%d][%s]",
			obj->GetReturnCode(),obj->GetReturnMsg().c_str()));
		return -1;
	}
	memset(feePage,0xFF,sizeof feePage);
	feeIndex = 0;
	int used = 0;
	int fee = 0;
	while(obj->HasMoreRecord())
	{
		ST_PACK *data = obj->GetNextRecord();
		fee = data->lvol11;
		int feeCnt = data->lvol12;
		if(startFee <= fee && startFee + feeGroupCnt > fee)
		{
			// �ڷ�Χ��
			feePage[feeSize*(fee-startFee)] = data->lvol0;
			feePage[feeSize*(fee-startFee)+1] = data->lvol1;
			feePage[feeSize*(fee-startFee)+2] = 0;
			feePage[feeSize*(fee-startFee)+3] = data->lvol2;
			feePage[feeSize*(fee-startFee)+4] = 0;

			if(feeCnt > 1)
			{
				feePage[feeSize*(fee-startFee)+5] = data->lvol3;
				feePage[feeSize*(fee-startFee)+6] = data->lvol4;
				feePage[feeSize*(fee-startFee)+7] = 0;
				feePage[feeSize*(fee-startFee)+8] = data->lvol5;
				feePage[feeSize*(fee-startFee)+9] = 0;
			}
			if(feeCnt > 2)
			{
				feePage[feeSize*(fee-startFee)+10] = data->lvol6;
				feePage[feeSize*(fee-startFee)+11] = data->lvol7;
				feePage[feeSize*(fee-startFee)+12] = 0;
				feePage[feeSize*(fee-startFee)+13] = data->lvol8;
				feePage[feeSize*(fee-startFee)+14] = 0;
			}
			used = 1;
		}
		else if(fee < startFee)
		{
			ACE_DEBUG((LM_ERROR,"����ϵͳ�еķ��ʷ���δ����!"));
			return TASK_ERR_EXECUTE;
		}
		if(fee + 1 == startFee + feeGroupCnt)
		{
			// ���ط���
			if(used == 1)
			{
				req.SetFieldValue(FIELD_ADDITIONALDATA1,&feeIndex,1);
				req.SetFieldValue(FIELD_ADDITIONALDATA2,feePage,pageSize);
				ret = SendRequest(req,task->_handle);
				if(ret == TASK_SUCCESS)
				{
					used = 0;
				}
				else
				{
					ACE_DEBUG((LM_ERROR,"����ˮ�ط���ʧ��,dev[%s]ret[%d]",task_node->GetPhyId().c_str(),ret));
					return ret;
				}
			}
			feeIndex++;
			startFee += feeGroupCnt;
		}
	}
	if(fee >= startFee)
	{
		// ������һ��
		if(used == 1)
		{
			req.SetFieldValue(FIELD_ADDITIONALDATA1,&feeIndex,1);
			req.SetFieldValue(FIELD_ADDITIONALDATA2,feePage,pageSize);
			ret = SendRequest(req,task->_handle);
			if(ret == TASK_SUCCESS)
			{
				feeIndex++;
				used = 0;
				startFee += feeGroupCnt;
			}
			else
			{
				ACE_DEBUG((LM_ERROR,"����ˮ�ط���ʧ��,dev[%s]ret[%d]",task_node->GetPhyId().c_str(),ret));
				return ret;
			}
		}
	}
	ACE_DEBUG((LM_INFO,"����ˮ�ط��ʳɹ�,dev[%s],fee[%d]",task_node->GetPhyId().c_str(),feeID));
	return 0;
}
int IHDDLWaterFeeCfg::SendRequest(HD8583STRUCT &req,ACE_HANDLE handler)
{
	char data[1024] = "";
	int packlen;
	int ret;
	ACE_Time_Value tv = KSGGetTaskTimeoutIntval();
	MESSAGETYPE msg_type;
	if( (packlen = PackRequestStruct(req,MT_SETWATERFEE,data,sizeof data,true)) <= 0)
	{
		ret = TASK_ERR_EXECUTE;
	}
	else
	{
		ACE_HEX_DUMP((LM_ERROR,data,packlen));
		KSG_SLEEP(200);
		ACE_DEBUG((LM_ERROR,"time[%d][%d]",tv.sec(),tv.msec()));
		if( (ret=HDSendBuffer(handler,data,packlen,&tv) )!= 0)
		{
			ACE_DEBUG((LM_ERROR,"����������ʧ��[%d]",ret));
			if(ret==-2)
				ret = TASK_INVALID_CONN;
			else
				ret = TASK_ERR_TIMEOUT;
		}
		else 
		{
			//KSGThreadUtil::Sleep(100);
			tv = KSGGetTaskTimeoutIntval();
			if((packlen = HDRecvBuffer(handler,data,sizeof data,&tv)) <= 0)
			{
				ACE_DEBUG((LM_ERROR,"����������ʧ��,[%d]",packlen));
				ret = TASK_ERR_TIMEOUT;
			}
			else if(UnPackResponseStruct(req,&msg_type,data,packlen))
			{
				ret = TASK_ERR_EXECUTE;
			}
			else if(req->ResponseCode != RC_SUCCESS)
			{
				ret = req->ResponseCode;
			}
			else
			{
				ret = TASK_SUCCESS;
			}
		}
	}
	return ret;
}