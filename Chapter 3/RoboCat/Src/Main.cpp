
#include "RoboCatPCH.h"
#include <iostream>
#define GOOD_BUF_SIZE 128

using namespace std;

void remove_tcp(vector<TCPSocketPtr> &vec, TCPSocketPtr ptr){
    for (auto iter = vec.begin(); iter != vec.end(); iter ++){
        if (*iter == ptr){
            vec.erase(iter);
            break;
        }
    }
}

void run_tcp_server(){
    SocketAddressPtr addrPtr = std::make_shared<SocketAddress>(INADDR_ANY, 7777);
    cout<<"run tcp server at "<< addrPtr->ToString()<<endl;
    TCPSocketPtr listenSocket= SocketUtil::CreateTCPSocket(INET);
    listenSocket->Bind(*addrPtr);
    listenSocket->Listen(128);
    vector<TCPSocketPtr> inReadSet;
    vector<TCPSocketPtr> outReadSet;
    vector<TCPSocketPtr> inWriteSet;
    vector<TCPSocketPtr> outWriteSet;
    vector<TCPSocketPtr> inErrorSet;
    vector<TCPSocketPtr> outErrorSet;

    inReadSet.push_back(listenSocket);
    inErrorSet.push_back(listenSocket);

    int ret;
    while (true){
        ret = SocketUtil::Select(&inReadSet, &outReadSet, &inWriteSet, &outWriteSet, &inErrorSet, &outErrorSet);
        if (ret <= 0){
            continue;
        }
        cout<<" size of sockets"<<inReadSet.size()<<endl;
        for(auto sockPtr: outReadSet){
            if(sockPtr == listenSocket){
                SocketAddress addr;
                TCPSocketPtr ptr = sockPtr->Accept(addr);
                inReadSet.push_back(ptr);
                // process new client
                cout<<"New client connect, address is:"<<addr.ToString()<<endl;
            }else{
                char buf[GOOD_BUF_SIZE];
                int readedBytes = sockPtr->Receive(buf, GOOD_BUF_SIZE);
                if (readedBytes > 0){
                    buf[readedBytes] = '\0';
                    cout<<"client "<<sockPtr->GetSocket()<<" say:"<<buf<<endl;
                    sockPtr->Send(buf, readedBytes);
                } else if (readedBytes == 0){
                    cout<<"socket disconnect:"<<sockPtr->GetSocket()<<endl;
                    remove_tcp(inReadSet, sockPtr);
                    remove_tcp(inErrorSet, sockPtr);
                }
            }
        }
        for(auto sockPtr: outErrorSet){
            cout<<"socket error :"<<sockPtr->GetSocket()<<endl;
            remove_tcp(inReadSet, sockPtr);
            remove_tcp(inErrorSet, sockPtr);
        }
    }
}

void run_tcp_client(){
    SocketAddressPtr addrPtr = SocketAddressFactory::CreateIPv4FromString("127.0.0.1:7777");
    TCPSocketPtr socket= SocketUtil::CreateTCPSocket(INET);
    if (!socket->Connect(*addrPtr)){
        cout<<"cant connect to "<<addrPtr->ToString()<<endl;
        return;
    }
    char buf[GOOD_BUF_SIZE];
    while(true){
        cout<<">";
        cin.getline(buf, GOOD_BUF_SIZE);
        socket->Send(buf, strlen(buf));
        int bytes = socket->Receive(buf, GOOD_BUF_SIZE);
        if (bytes > 0){
            buf[bytes] = '\0';
            cout<<"  server say:"<<buf<<endl;
        }else{
            cout<<"  server disconnect!"<<endl;
            break;
        }
    }
}

void run_udp_server(){
    // SocketAddressPtr addrPtr = std::make_shared<SocketAddress>(INADDR_ANY, 7777);
    SocketAddressPtr addrPtr = SocketAddressFactory::CreateIPv4FromString("127.0.0.1:48000");
    cout<<"run udp server at "<< addrPtr->ToString()<<endl;
    UDPSocketPtr listenSocket= SocketUtil::CreateUDPSocket(INET);
    listenSocket->SetNonBlockingMode(true);
    if (listenSocket->Bind(*addrPtr)){
        cerr<<"bind udp error"<<endl;
        return;
    }
    SocketAddress clientAddr;
    int ret;
    char buf[GOOD_BUF_SIZE];
    while (true){
        ret = listenSocket->ReceiveFrom(buf, GOOD_BUF_SIZE, clientAddr);
        if (ret <= 0){
            continue;
        }
        cout<<"client:"<< clientAddr.ToString()<<" say "<<buf<<endl;
        listenSocket->SendTo(buf, ret, clientAddr);
    }
}

void run_udp_client(){
    SocketAddressPtr addrPtr = SocketAddressFactory::CreateIPv4FromString("localhost:48000");
    UDPSocketPtr socket= SocketUtil::CreateUDPSocket(INET);
    char buf[GOOD_BUF_SIZE];
    SocketAddress srvAddr;
    while(true){
        cout<<"udp>";
        cin.getline(buf, GOOD_BUF_SIZE);
        cout<<buf<<endl;
        int err = socket->SendTo(buf, strlen(buf) , *addrPtr);
        if (err < 0){
            cout<<"error"<<err<<endl;
            return;
        }
        int bytes = socket->ReceiveFrom(buf, GOOD_BUF_SIZE, srvAddr);
        if (bytes > 0){
            cout<<"  server say:"<<buf<<endl;
        }else{
            cout<<"  server disconnect!"<<endl;
            break;
        }
    }
}

#if _WIN32
int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );
	
}
#else
const char** __argv;
int __argc;
int main(int argc, const char** argv)
{
	__argc = argc;
	__argv = argv;
    string arg1(argv[1]);
    if (arg1 ==  "tcp_server"){
        run_tcp_server();
    }
    else if (arg1 == "tcp_client"){
        run_tcp_client();
    }
    else if (arg1 ==  "udp_server"){
        run_udp_server();
    }
    else if (arg1 == "udp_client"){
        run_udp_client();
    }
}
#endif

