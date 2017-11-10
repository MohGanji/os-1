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
    char shared_dir[1024] = "";
    list *list_of_files = NULL;
    list* cursor;
    int response_len = 2;
    enum STATE state = Idle;
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
    } else{
        state = Data;
    }
    // do stuff with ms_sock;
    while(response_len > 0){
        switch(state){
            case Data:
                mysend(ms_sock, "__DATA__", 8);
                state = Port;
                break;
            case Port:
                mysend(ms_sock, ds_port, 4);
                state = Dir;
                break;
            case Dir:
                print("Enter the directory you want to share: ");
                packet = myread(STDIN, buffer, sizeof(buffer));
                strcpy(shared_dir, packet.data);
                list_of_files = ls(packet.data);
                cursor = list_of_files;
                mysend(ms_sock, packet.data, packet.len);
                state = Files;
                break;
            case Files:
                if(cursor != NULL){
                    mysend(ms_sock, cursor->data, strlen(cursor->data));
                    cursor = cursor->next;
                } else{
                    mysend(ms_sock, "__END__", 7);
                }
                break;
        }
        response_len = recv( ms_sock , buffer, 1024, 0);    
        print("Response: ");printl(buffer, response_len);print("\n");    
    }
    print("Connection closed by server!\n");

    data_server(ds_port, shared_dir);
    return 0;
}