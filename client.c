#include "utils.h"
#include "client_connect.h"  

char * itoa(int i) {
    char * res = malloc(8*sizeof(int));
    sprintf(res, "%d", i);
    return res;
}
  

DS_Struct *parse_data(struct Packet packet){
    DS_Struct * data_servers = NULL;
    char * curLine = packet.data;
    char * nextLine = "";
    char * new_port = "", *new_path = "", *at = "", *new_file = "";
    while(curLine)
    {
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line
        
        
        //do something with curLine
        if((at = strchr(curLine, '@')) != NULL){
            new_port = strtok(curLine, "@:");
            if(data_servers == NULL)
                data_servers = DS_create("127.0.0.1", new_port, data_servers);
            else
                data_servers = DS_prepend(data_servers, "127.0.0.1", new_port);
            curLine = at+1;
            new_path = strtok(curLine, "@:");
            add_path(data_servers, new_port, new_path);
        } else if(strlen(curLine) > 1){
            new_file = strtok(curLine, "-");
            add_to_files(data_servers, new_port, new_file);
        }
       
        if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy    
        curLine = nextLine ? (nextLine+1) : NULL;
    }
    return data_servers;
 
}

int main(int argc, char *argv[])
{
    int sock = 0;
    char buffer[1024] = {0};
    char* ip, *port;
    int status = 0;
    int response_len;
    struct Packet packet;    
    int servers_count = 0, num = 1;
    DS_Struct * data_servers = NULL, *res_server;
    char *filename = (char *) malloc(100);    
    char output_file[1024] = "";
    char *ext_num = "";
    int filefd;
    char chunk[1024] = "";
    int chunklen = 1024;

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
    mysend(sock, "__CLIENT__", 10);
    packet = myrecv(sock, buffer, 1024);
    print(packet.data);

    while(packet.len > 0){
        packet = myread(STDIN, buffer, sizeof(buffer));
        mysend(sock , packet.data , packet.len);
        print("Request: ");print(packet.data);print("\n");
        if(strcmp(packet.data, "ls") == 0 ){
            packet = myrecv(sock , buffer, 1024);
            print("Response: ");print(packet.data);print("\n");            
            data_servers = parse_data(packet);
        } else{
            packet = myrecv(sock , buffer, 1024);        
            // here I should handle responses with case or if else probably!
            print("Response: ");print(packet.data);print("\n");
        }
    }
    print("Enter the name of file you want to download (without part number!): ");
    packet = myread(STDIN, buffer, sizeof(buffer));
    servers_count = DS_count(data_servers);
    strcpy(output_file, packet.data);
    print(output_file);print("\n");
    filefd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(filefd > 0){
        while(num <= servers_count){
            ext_num = itoa(num);
            strcpy(filename, "");
            strcat(filename, output_file);
            strcat(filename, ".");
            strcat(filename, ext_num);
            num++;
            print("Next file: ");print(filename);print("\n");
            res_server = find_file(data_servers, filename);
            print(res_server->port);print("\n");
            status = client_connect("127.0.0.1", res_server->port, &sock);
            if(status != 0){
                print("ERR: failed to connect to data server!\n");
                return -1;
            }
            mysend(sock , filename , strlen(filename));
            packet = myrecv(sock, buffer, 1024);
            if(packet.len == 23){
                print("downloading ");print(filename);print("\n");
                while(strncmp(chunk, "__EOF__", 7) != 0){
                    mysend(sock , "OK" , 2);
                    chunklen = recv(sock, chunk, 1024, 0);
                    printl(chunk, chunklen);print("\n");
                    if(strncmp(chunk, "__EOF__", 7) != 0)
                        write(filefd, chunk, chunklen);
                }
            } else{
                printl(packet.data, packet.len);print("\n");
            }
            strcpy(chunk, "");
            // chunklen = 1024;
            
        }
        close(filefd);
    } else{
        print("ERR: Unable to Open file!\n");
    }
    
    return 0;
}

