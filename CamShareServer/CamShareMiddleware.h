/*
 * CamShareMiddleware.h
 *
 *  Created on: 2015-1-13
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef CamShareMiddleware_H_
#define CamShareMiddleware_H_

#include "Session.h"
#include "Client.h"
#include "MessageList.h"
#include "TcpServer.h"
#include "FreeswitchClient.h"

#include <common/ConfFile.hpp>
#include <common/KSafeMap.h>
#include <common/TimeProc.hpp>
#include <common/StringHandle.h>

#include <request/IRequest.h>
#include <respond/IRespond.h>

#include <livechat/ILiveChatClient.h>

#include <map>
#include <list>
using namespace std;

// socket -> client
typedef KSafeMap<int, Client*> ClientMap;

// type -> livechat client
typedef KSafeMap<SITE_TYPE, ILiveChatClient*> LiveChatClientMap;

// client -> session
typedef KSafeMap<Client*, Session*> Client2SessionMap;

// livechat client -> session
typedef KSafeMap<ILiveChatClient*, Session*> LiveChat2SessionMap;

class StateRunnable;
class ConnectLiveChatRunnable;
class ConnectFreeswitchRunnable;
class CamShareMiddleware : public TcpServerObserver, ClientCallback, ILiveChatClientListener, FreeswitchClientListener {
public:
	CamShareMiddleware();
	virtual ~CamShareMiddleware();

	void Run(const string& config);
	void Run();
	bool Reload();
	bool IsRunning();

	/**
	 * TcpServer回调
	 */
	bool OnAccept(TcpServer *ts, int fd, char* ip);
	void OnRecvMessage(TcpServer *ts, Message *m);
	void OnSendMessage(TcpServer *ts, Message *m);
	void OnDisconnect(TcpServer *ts, int fd);
	void OnClose(TcpServer *ts, int fd);
	void OnTimeoutMessage(TcpServer *ts, Message *m);

	/**
	 * 检测状态线程处理
	 */
	void StateHandle();

	/**
	 * 连接LiveChat线程处理
	 */
	void ConnectLiveChatHandle();

	/**
	 * 连接Freeswitch
	 */
	void ConnectFreeswitchHandle();

	/**
	 * 内部服务(HTTP), 命令回调
	 */
	void OnClientGetUserBySession(Client* client, const string& session);
	void OnClientUndefinedCommand(Client* client);

	/**
	 * 内部服务(Freeswitch), 命令回调
	 */
	 void OnFreeswitchEventConferenceAddMember(FreeswitchClient* freeswitch, const string& user, const string& conference, MemberType type);

	/**
	 * 外部服务(LiveChat), 任务回调
	 */
	void OnConnect(ILiveChatClient* livechat, LCC_ERR_TYPE err, const string& errmsg);
	void OnDisconnect(ILiveChatClient* livechat, LCC_ERR_TYPE err, const string& errmsg);
	void OnSendEnterConference(ILiveChatClient* livechat, int seq, const string& fromId, const string& toId, LCC_ERR_TYPE err, const string& errmsg);
	void OnRecvDisconnectUserVideo(ILiveChatClient* livechat, int seq, const string& userId1, const string& userId2, LCC_ERR_TYPE err, const string& errmsg);

private:
	/*
	 *	请求解析函数
	 *	return : -1:Send fail respond / 0:Continue recv, send no respond / 1:Send OK respond
	 */
	int TcpServerRecvMessageHandle(TcpServer *ts, Message *m);
	int TcpServerTimeoutMessageHandle(TcpServer *ts, Message *m);

	/**
	 * Freeswitch事件处理
	 */
	void FreeswitchEventHandle(esl_event_t *event);

	/***************************** 会话管理 **************************************/
	/**
	 * 会话交互(Session), 客户端发起会话
	 * @param client		客户端
	 * @param livechat		LiveChat client
	 * @param request		请求实例
	 * @param seq			LiveChat client seq
	 * @return true:需要Finish / false:不需要Finish
	 */
	bool StartSession(
			Client* client,
			ILiveChatClient* livechat,
			IRequest* request,
			int seq
			);

	/**
	 * 会话交互(Session), LiveChat返回会话
	 * @param livechat		LiveChat client
	 * @param seq			LiveChat client seq
	 * @param client		客户端(出参)
	 * @return 请求实例
	 */
	IRequest* FinishSession(
			ILiveChatClient* livechat,
			int seq,
			Client** client
			);

	/**
	 * 内部服务(HTTP), 关闭会话
	 * @param client	客户端
	 * @return true:发送成功/false:发送失败
	 */
	bool CloseSessionByClient(Client* client);

	/**
	 * 外部服务(LiveChat), 关闭会话
	 * @param livechat		LiveChat client
	 */
	bool CloseSessionByLiveChat(ILiveChatClient* livechat);
	/***************************** 会话管理 end **************************************/

	/***************************** 外部服务接口 **************************************/
	/**
	 * 外部服务(LiveChat), 发送进入聊天室命令
	 * @param 	fromId		用户Id
	 * @param 	toId		对方Id
	 * @param	type		会员类型
	 */
	bool SendEnterConference2LiveChat(
			const string& fromId,
			const string& toId,
			MemberType type,
			Client* client = NULL
			);
	/***************************** 外部服务接口 end **************************************/

	/***************************** 内部服务接口 **************************************/
	/**
	 * 内部服务(HTTP), 发送请求响应
	 * @param client		客户端
	 * @param respond		响应实例
	 * @return true:发送成功/false:发送失败
	 */
	bool SendRespond2Client(
			Client* client,
			IRespond* respond
			);
	/***************************** 内部服务接口 end **************************************/

	/***************************** 基本参数 **************************************/
	/**
	 * 监听端口
	 */
	short miPort;

	/**
	 * 最大连接数
	 */
	int miMaxClient;

	/**
	 * 处理线程数目
	 */
	int miMaxHandleThread;

	/**
	 * 每条线程处理任务速度(个/秒)
	 */
	int miMaxQueryPerThread;

	/**
	 * 请求超时(秒)
	 */
	unsigned int miTimeout;
	/***************************** 基本参数 end **************************************/

	/***************************** 日志参数 **************************************/
	/**
	 * 日志等级
	 */
	int miLogLevel;

	/**
	 * 日志路径
	 */
	string mLogDir;

	/**
	 * 是否debug模式
	 */
	int miDebugMode;
	/***************************** 日志参数 end **************************************/

	/***************************** 处理线程 **************************************/
	/**
	 * 状态监视线程
	 */
	StateRunnable* mpStateRunnable;
	KThread* mpStateThread;
	/**
	 * 连接Livechat线程
	 */
	ConnectLiveChatRunnable* mpConnectLiveChatRunnable;
	KThread* mpLiveChatConnectThread;
	/**
	 * 连接Freeswitch线程
	 */
	ConnectFreeswitchRunnable* mpConnectFreeswitchRunnable;
	KThread* mpConnectFreeswitchThread;
	/***************************** 处理线程 end **************************************/

	/***************************** 统计参数 **************************************/
	/**
	 * 统计请求总数
	 */
	unsigned int mTotal;
	unsigned int mResponed;
	KMutex mCountMutex;

	/**
	 * 监听线程输出间隔
	 */
	unsigned int miStateTime;
	/***************************** 统计参数 end **************************************/

	/**
	 * 是否运行
	 */
	bool mIsRunning;

	/**
	 * 配置文件锁
	 */
	KMutex mConfigMutex;
	/**
	 * 配置文件
	 */
	string mConfigFile;

	/**
	 * 内部服务(HTTP)
	 */
	TcpServer mClientTcpServer;

	/*
	 * 内部服务(HTTP), 在线客户端列表
	 */
	ClientMap mClientMap;

	/**
	 * 外部服务(LiveChat), 实例列表
	 */
	LiveChatClientMap mLiveChatClientMap;

	/**
	 * 内部服务(HTTP), 对应会话
	 */
	Client2SessionMap mClient2SessionMap;

	/**
	 * 外部服务(LiveChat), 对应会话
	 */
	LiveChat2SessionMap mLiveChat2SessionMap;

	/**
	 * 客户端缓存数据包buffer
	 */
	MessageList mIdleMessageList;

	/**
	 * Freeswitch实例
	 */
	FreeswitchClient mFreeswitch;
};

#endif /* CAMSHAREMIDDLEWARE_H_ */