#include "utils.h"
#include "server.h"
#include "client_connect.h"


int main(int argc, char *argv[]){
    // ms: main_server, ds: data_server
    char * ms_ip, *ms_port, *ds_port;
    int ms_sock = 0;
    int client_connect_status = 0;
    struct Packet packet;
    char buffer[1024] = {0};
    list *list_of_files = NULL;
    list* cursor;
    int response_len;
    if(argc < 4){
        print("ERR: No ip and port provided!\n");
        print("you should provide ip and port of main_server and port to listen on!\n");
        exit(1);
    }
    ms_ip = (char *)argv[1];
    ms_port = (char *)argv[2];
    ds_port = (char *)argv[3];

    client_connect_status = client_connect(ms_ip, ms_port, &ms_sock);
    if(client_connect_status != 0){
        print("Connection to server failed!\n");
        exit(2);
    }
    // do stuff with ms_sock;
    mysend(ms_sock, "__DATA__", 8);
     response_len = recv( ms_sock , buffer, 1024, 0);    
      print("Response: ");print(buffer);print("\n");    
    mysend(ms_sock, ds_port, 4);
     response_len = recv( ms_sock , buffer, 1024, 0);
      print("Response: ");print(buffer);print("\n");
    print("Enter the directory you want to share: ");
     packet = myread(STDIN, buffer, sizeof(buffer));
     list_of_files = ls(packet.data);
      mysend(ms_sock, packet.data, packet.len);
       response_len = recv( ms_sock , buffer, 1024, 0);
        print("Response: ");printl(buffer, response_len);print("\n");    

    cursor = list_of_files;
    while(cursor != NULL)
    {
        mysend(ms_sock, cursor->data, strlen(cursor->data));
        response_len = recv( ms_sock , buffer, 1024, 0);
        cursor = cursor->next;
    }
    mysend(ms_sock, "__END__", 7);
    data_server(ds_port);
    return 0;
}