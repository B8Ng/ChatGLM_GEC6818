#ifndef __SOCKET_SERVER_H__
#define __SOCKET_SERVER_H__

int Server_Init(int Port,char *IPAddress);
int Server_Accept(int tcp_socket);

#endif
