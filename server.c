#include "utils.h"


void main_server(char* PORT){
    
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    enum STATE state[100] = {0};
    char ds_ports[100][6];
    
    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    
    char buf[1024];    // buffer for client data
    int buf_len;
    char *res_buf;

    char remoteIP[INET6_ADDRSTRLEN];
    
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;
    DS_Struct *data_servers = NULL;
    DS_Struct *cursor = NULL;
    list *fl_cursor = NULL;

    struct Packet packet;

    
    struct addrinfo hints, *ai, *p;
    
    
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
    
    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    // fill this null input for ip.
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        print("ERR: getaddrinfo failed!\n");
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        
        break;
    }
    
    // if we got here, it means we didn't get bound
    if (p == NULL) {
        print("ERR: bind failed!\n");
        exit(2);
    }
    
    freeaddrinfo(ai); // all done with this
    
    // listen
    if (listen(listener, 10) == -1) {
        print("ERR: listen failed!\n");
        exit(3);
    }
    
    // add the listener to the master set
    FD_SET(listener, &master);
    
    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one
    
    print("Server is Up!\n");    

    // main loop
    while(1) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            print("ERR: select failed!\n");
            exit(4);
        }
        
        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);
                        
                        if (newfd == -1) {
                            print("ERR: failed to accept!\n");
                        } else {
                            FD_SET(newfd, &master); // add to master set
                            if (newfd > fdmax) {    // keep track of the max
                                fdmax = newfd;
                            }
                            print("New connection from: ");
                            print((char *)inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN));
                            print(" on socket ");printInt(newfd);print("\n");
                        }
                } else {
                    // handle data from a client
                    packet = myrecv(i, buf, sizeof buf);
                    if (packet.len <= 0) {
                        // got error or connection closed by client
                        if (packet.len == 0) {
                            // connection closed
                            print("socket ");printInt(i);print(" hung up\n");
                        } else {
                            print("ERR: receive!\n");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                        state[i] = 0;
                    } else {
                        // we got some data from a client
                        //change the structure of recv, so that it returns a packet. like myread in client
                        print("log: new message: ");printl(packet.data, packet.len); print("\n");
                        if (FD_ISSET(i, &master)) {
                            switch (state[i]){
                                case Idle:
                                if(strncmp(packet.data, "__DATA__", packet.len) == 0){
                                    print("log: A Data Server connected!\n");
                                    state[i] = Data;
                                    mysend(i, "Ok", 2);
                                }else if (strncmp(packet.data, "__CLIENT__", packet.len) == 0){
                                    print("log: A Client Connected!\n");
                                    state[i] = Client;
                                    mysend(i, "commands:\nls: list data_servers and files\ndc: disconnect\n", 57);                                        
                                }else{
                                    print("log: Some unknown device connected!\n");
                                    mysend(i, "Who the hell are you??", 22);
                                }
                                break;
                                case Data:
                                if(4 <= packet.len || packet.len <= 5 ){
                                    
                                        print("log: Data server sent port\n");
                                        if(search_port(data_servers, packet.data) == NULL){
                                            if(data_servers == NULL)
                                                data_servers = DS_create("127.0.0.1", packet.data, data_servers);
                                            else
                                                data_servers = DS_prepend(data_servers, "127.0.0.1", packet.data);
                                            strcpy(ds_ports[i], packet.data);
                                        }
                                        state[i] = Port;
                                        mysend(i, "Ok", 2);
                                    } else{
                                        mysend(i, "you were supposed to send port!", 31);
                                    }
                                    break;
                                case Port:
                                    print("log: Data server shared directory.\n");
                                    if(add_path(data_servers, ds_ports[i], packet.data)){
                                        state[i] = Dir;
                                        mysend(i, "Ok", 2);
                                    } else{
                                        mysend(i, "Not Ok", 6);
                                        state[i] = Dir;                                      
                                    }
                                    break;
                                case Dir:
                                    if(strncmp(packet.data, "__END__", packet.len) == 0){
                                        print("log: Data server sent information of all files, now closing connection.\n");
                                        mysend(i, "Ok", 2);
                                        close(i); // bye!
                                        FD_CLR(i, &master); // remove from master set
                                        state[i] = Idle;
                                    } else{
                                        if(add_to_files(data_servers, ds_ports[i], packet.data)){
                                            print("log: added to files: ");printl(packet.data, packet.len);print("\n");                                    
                                            mysend(i, "Ok", 2);
                                        } else{
                                            mysend(i, "Not Ok", 6);                                            
                                        }
                                    }
                                    break;
                                case Client:
                                    if(strcmp(packet.data, "ls") == 0){
                                        cursor = data_servers;
                                        packet.data[0] = '\0';
                                        strcat(packet.data, "\n");
                                        while(cursor != NULL)
                                        {
                                            strcat(packet.data, cursor->port);
                                            strcat(packet.data, "@");
                                            strcat(packet.data, cursor->path);
                                            strcat(packet.data, ":\n");
                                            // mysend(i, packet.data, strlen(packet.data));
                                            fl_cursor = cursor->files;
                                            while(fl_cursor != NULL)
                                            {
                                                strcat(packet.data, "--");
                                                strcat(packet.data, fl_cursor->data);
                                                //  print(packet.data);print("\n");
                                                strcat(packet.data, "\n");
                                                // mysend(i, packet.data, strlen(packet.data));
                                                fl_cursor = fl_cursor->next;
                                            }
                                            cursor = cursor->next;
                                        }
                                        mysend(i, packet.data, strlen(packet.data));
                                    } else if(strcmp(packet.data, "dc") == 0){
                                        close(i); // bye!
                                        FD_CLR(i, &master); // remove from master set
                                        state[i] = Idle;
                                    } else
                                        mysend(i, "unexpected behavior!", 20);
                                    break;
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END while(1)--and you thought it would never end!    
}
















void data_server(char* PORT, char *shared_dir){
    
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    
    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    int filefd;
    char *file_name = (char *) malloc(100);
    char chunk[1024] = "";
    int chunklen = 1024;
    
    char buf[1024];    // buffer for client data
    int buf_len;
    enum STATE state = Idle;
    
    struct Packet packet;

    char remoteIP[INET6_ADDRSTRLEN];
    
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;
    
    struct addrinfo hints, *ai, *p;
    
    
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
    
    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    // fill this null input for ip.
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        print("ERR: getaddrinfo failed!\n");
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        
        break;
    }
    
    // if we got here, it means we didn't get bound
    if (p == NULL) {
        print("ERR: bind failed!\n");
        exit(2);
    }
    
    freeaddrinfo(ai); // all done with this
    
    // listen
    if (listen(listener, 10) == -1) {
        print("ERR: listen failed!\n");
        exit(3);
    }
    
    // add the listener to the master set
    FD_SET(listener, &master);
    
    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one
    state = Filename;
    print("Server is Up!\n");

    // main loop
    while(1) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            print("ERR: select failed!\n");
            exit(4);
        }
        
        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);
                        
                        if (newfd == -1) {
                            print("ERR: failed to accept!\n");
                        } else {
                            FD_SET(newfd, &master); // add to master set
                            if (newfd > fdmax) {    // keep track of the max
                                fdmax = newfd;
                            }
                            print("New connection from: ");
                            print((char *)inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN));
                            print(" on socket ");printInt(newfd);print("\n");
                        }
                } else {
                    // handle data from a client
                    packet = myrecv(i, buf, sizeof buf);
                    if (packet.len <= 0) {
                        // got error or connection closed by client
                        if (packet.len == 0) {
                            // connection closed
                            print("socket ");printInt(i);print(" hung up\n");
                        } else {
                            print("ERR: recieve!\n");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        switch(state){
                            case Filename:
                                strcpy(file_name, "");
                                strcat(file_name, shared_dir);
                                strcat(file_name, "/");
                                strcat(file_name, packet.data);
                                print(file_name);print("\n");
                                if((filefd = open(file_name, O_RDONLY)) < 0){
                                    print("ERR: could not open file\n");
                                    mysend(i, "File not found!", 15);
                                    close(i); // bye!
                                    FD_CLR(i, &master); // remove from master set
                                } else{
                                    state = Read;
                                    mysend(i, "Ready for sending file.", 23);
                                }
                                break;
                            case Read:
                                if(chunklen == 1024){
                                    chunklen = read(filefd, chunk, 1024);
                                    print(chunk);
                                    mysend(i, chunk, chunklen);
                                } else{
                                    print("CLOSE\n");
                                    close(filefd);
                                    mysend(i, "__EOF__", 7);
                                    chunklen = 1024;
                                    state = Filename;
                                }
                                break;
                        }
                        
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END while(1)--and you thought it would never end!    
}