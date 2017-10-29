#include "utils.h"
#include "client_connect.h"  

int main(int argc, char *argv[])
{
    int sock = 0;
    char buffer[1024] = {0};
    char* ip, *port;
    int status = 0;
    int response_len;
    struct Packet packet;    

    if(argc < 3){
        print("ERR: no ip or port provided!\n");
        exit(1);
    }
    ip =  (char *) argv[1];
    port = (char *) argv[2];
    status = client_connect(ip, port, &sock);
    if(status != 0){
        print("ERR: failed to connect to server!\n");
        return -1;
    }

    while(1){
        packet = myread(STDIN, buffer, sizeof(buffer));
        send(sock , packet.data , packet.len , 0 );
        print("Request: ");print(packet.data);print("\n");
        response_len = recv( sock , buffer, 1024, 0);
        // here I should handle responses with case or if else probably!
        print("Response: ");print(buffer);print("\n");
    }
    return 0;
}

