#include "mainwindow.h"
#include <QApplication>
#include <winsock.h>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <ftpsock/login.h>
#include <ftpsock/ftpdata.h>
#include <utils/utils.h>
#define clength 400
#define dlength 1000
using namespace std;
/**
 * @deprecated 测试时写的，已经弃用 2020.2.23
 * @brief 连接到本地ftp服务器    2020.2.21
 * @authors 许敏章 、叶茂鑫
 * @details 连接到本机的127.0.0.1的21端口的ftp
 *          该服务端口暂时设置为匿名可访问
 *          根据由IBM教程中获得的关于ftp协议的规定内容设置，只要实现对应的即可
 *          此处先进行ftp连接和认证
 * */
int Connect_to_Server(){
    WSADATA wsadata;
    WORD minVersionRequired;
    minVersionRequired=MAKEWORD(1,1);         //生成版本号
    int err=WSAStartup(minVersionRequired,&wsadata);   //启动wsa的服务
    if(err!=0)
    {
        return -1 ;
    }
    //对版本要求的低字节和高字节进行要求
    if(LOBYTE(wsadata.wVersion)!=1||HIBYTE(wsadata.wVersion)!=1)
    {
        WSACleanup();
        return -1;
    }

    SOCKET sock=socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in ip_mem;
    int error;
    int slen= sizeof(ip_mem);
    memset(&ip_mem, 0,sizeof(ip_mem)); //设置sockaddr的变量的指针为原零
    ip_mem.sin_family=AF_INET;
    ip_mem.sin_port=htons(21);
    ip_mem.sin_addr.s_addr=inet_addr("127.0.0.1");
    //为何将sockaddr_in转sockaddr，尽管有说明就是了
    int err_connect=connect(sock,(struct sockaddr *)&ip_mem, sizeof(ip_mem));
    if(err_connect==0)
    {
        char* message=(char*)malloc(sizeof(int)*100);
        recv(sock,message,400,0);
        std::string sm=message;
        std::cout<<sm;
    } else{
        std::cout<<"连接失败";
    }
    std::cout.flush();
    //首次握手完毕，获得ftp服务器的欢迎信息
     char *command=(char *)malloc(clength);
     char *message=(char *)malloc(clength);
     memset(command,0, clength);
     string Message;
     sprintf(command,"USER jh\r\n");
     send(sock,command,clength,0);
     recv(sock,message,clength,0);
     Message=message;
     cout<<Message;
     sprintf(command,"PASS 123456\r\n");
     send(sock,command,clength,0);
     recv(sock,message,clength,0);
     Message=message;
     cout<<Message;

     




}
int main(int argc, char *argv[])
{

    char *username=(char*)malloc(clength);
    char *password=(char*)malloc(clength);
    char *command=(char*)malloc(clength);
    char *message=(CHAR*)malloc(clength);
    string Message;
    //malloc和memset还有sprintf一定要配套使用。才能保证数据的干净正确
    memset(username,0,clength);
    memset(password,0,clength);
    memset(command,0,clength);
    memset(message,0,clength);
    sprintf(username,"jh");
    sprintf(password,"123456");
    //登录ftp服务器的同时，获取socket
    SOCKET sock=login(username,password);
    //进入被动模式,同时获取服务器端的端口信息   被动模式下的服务器打开一个新的数据端口，服务器将数据放到自己的端口处，客户端自己来取
    sprintf(command,"PASV\r\n");
    send(sock,command,clength,0);
    recv(sock,message,clength,0);
    Message=message;
    cout<<Message;
    string port=message;
    int newport=getNewPort(port);
    cout<<newport<<endl;
    memset(command,0,clength);
    memset(message,0,clength);

    //新建的数据端口
    SOCKET  DataSock=getNewSocket("127.0.0.1",newport);
    //向命令端口发送命令，此处为列出当前目录列表
    SendCommand(sock,LISTCUR);
    //从数据端口取数据。次数信息为当前目录列表
    recv(DataSock,message,Dlength,0);
    Message=message;
    cout<<Message;

}
