#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
#include <iostream>
#include <string>
#include<map>
#include <fstream>
#include <sys/stat.h>
using namespace std;



#define TRUE   1  
#define FALSE  0  
//#define PORT 1313
     

multimap <string, string> seeder_list;
multimap <string, string> :: iterator itr;
string SHA_string,client_IP,client_Port,filename,tracker1_IP,tracker1_Port,tracker2_IP,tracker2_Port,tracker1IP_Port,tracker2IP_Port;
struct stat stat_buf;

string seeder_file="./";
string log_file="./";

void write_in_log(string message)
{

    time_t now = time(0);

    string time = ctime(&now);
    std::fstream fout;
    fout.open(log_file,ios_base::out|ios_base::app);
    
    fout << message << " : " << time ;
    fout.close();
}


void findIP(string tracker1IP_Port,string tracker2IP_Port)
{

    size_t found1 = tracker1IP_Port.find(':');
    tracker1_IP=tracker1IP_Port.substr(0,found1);
    tracker1_Port=tracker1IP_Port.substr(found1+1);
    
    size_t found2 = tracker2IP_Port.find(':');
    tracker2_IP=tracker2IP_Port.substr(0,found2);
    tracker2_Port=tracker2IP_Port.substr(found2+1);
}

void update_seeder_list()
{
    string tempstr;
    std::fstream fin;

    if(!(stat(seeder_file.c_str(),&stat_buf)))
    {
        fin.open(seeder_file,ios_base::in|ios_base::binary);

      //  seeder_list.insert({SHA_string,client_IP});
        while(getline(fin,tempstr))
        {
            //////cout<<"hello:"<<tempstr<<"\n";
            size_t found = tempstr.find(':');
            seeder_list.insert({tempstr.substr(0,found),tempstr.substr(found+1)});
         //   ////cout<<tempstr.substr(0,found)<<" "<<tempstr.substr(found+1)<<"\n";
        }
        // for(auto itr = seeder_list.cbegin(); itr != seeder_list.cend(); ++itr)
        // {
        //    // std::////cout << itr->first << " " << itr->second<< "\n";
        // }
        fin.close();            
    }
}
void update_seeder_file()
{
    //string tempstr;
    std::fstream fout;

    if(!(stat(seeder_file.c_str(),&stat_buf)))
    {
        fout.open(seeder_file,ios_base::out|ios_base::trunc);

      //  seeder_list.insert({SHA_string,client_IP});
        // while(getline(fin,tempstr))
        // {
        //     //////cout<<"hello:"<<tempstr<<"\n";
        //     size_t found = tempstr.find(':');
        //     seeder_list.insert({tempstr.substr(0,found),tempstr.substr(found+1)});
        //  //   ////cout<<tempstr.substr(0,found)<<" "<<tempstr.substr(found+1)<<"\n";
        // }
        for(auto itr = seeder_list.cbegin(); itr != seeder_list.cend(); ++itr)
        {
            fout << itr->first << " " << itr->second<< "\n";
        }
        fout.close();            
    }
} 
int main(int argc , char *argv[])   
{   

    tracker1IP_Port=argv[1];
    tracker2IP_Port=argv[2];
    seeder_file.append(argv[3]);
    log_file.append(argv[4]);
    
    findIP(tracker1IP_Port,tracker2IP_Port);

    update_seeder_list();

    int opt = TRUE; 
    int master_socket , addrlen , new_socket , client_socket[100] , max_clients = 100 , activity, i , valread , sd;   
    int max_sd;   
    struct sockaddr_in address;   
         
    char buffer[1025];  //data buffer of 1K  
         
    //set of socket descriptors  
    fd_set readfds;   
         
    //a message  
  //  char *message = "ECHO Daemon v1.0 \r\n";   
     
    //initialise all client_socket[] to 0 so not checked  
    for (i = 0; i < max_clients; i++)   
    {   
        client_socket[i] = 0;   
    }   
         
    //create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }
     
    //type of socket created  
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons(stoi(tracker1_Port));   
         
    //bind the socket to localhost port 1313  
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
   printf("Listener on port %d \n", stoi(tracker1_Port));   
         
    //try to specify maximum of 100 pending connections for the master socket  
    if (listen(master_socket, 100) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    addrlen = sizeof(address);   
  //  puts("Waiting for connections ...");   
         
    while(TRUE)   
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(master_socket, &readfds);   
        max_sd = master_socket;   
             
        //add child sockets to set  
        for ( i = 0 ; i < max_clients ; i++)   
        {   
            //socket descriptor  
            sd = client_socket[i];   
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(master_socket, &readfds))   
        {   
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             
            //inform user of socket number - used in send and receive commands  
       //     printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
           
            //send new connection greeting message  
            int size;
            read(new_socket,&size,sizeof(size));
            char buff[size+1];
            ////cout<<size<<"\n";
            read(new_socket,buff,size);
            buff[size]='\0';
            ////cout<<"message from client:"<<buff<<"\n";

            string command;
            string message=buff;
            //std::string delimiter = "|";

            size_t found = message.find('|');
            command=message.substr(0,found);
            ////cout<<command;
            if(command=="share")
            {
                ////cout<<"share command arrived\n";
                
                message=message.substr(found+1);
                found = message.find('|');
                filename=message.substr(0,found);

                message=message.substr(found+1);
                found = message.find('|');
                SHA_string=message.substr(0,found);
                
                message=message.substr(found+1);
                found = message.find('|');
                client_IP=message.substr(0,found);

                message=message.substr(found+1);
                found = message.find('|');
                client_Port=message.substr(0,found);

                //////cout<<SHA_string<<":"<<client_IP<<":"<<client_Port<<":";

                client_IP.append(":");
                client_IP.append(client_Port);


                seeder_list.insert({SHA_string,client_IP});

                std::fstream fout;

                fout.open(seeder_file,ios_base::out|ios_base::app);
                fout <<  SHA_string << ":" << client_IP << "\n";
                
                fout.close();
                write_in_log("Client "+client_IP+" share SHA of mtorrent file");
                // ////cout<<"going inside\n";
                // for (itr = seeder_list.lower_bound(SHA_string); itr != seeder_list.upper_bound(SHA_string); ++itr) 
                // { 
                //     ////cout << '\t' << itr->first<< '\t' << itr->second << '\n'; 
                // }


            }
            else if(command=="get")
            {
                string tempstr;
                int counter=0;
                ////cout<<"share command arrived\n";
                
                message=message.substr(found+1);
                found = message.find('|');
                SHA_string=message.substr(0,found);


                for (itr = seeder_list.lower_bound(SHA_string); itr != seeder_list.upper_bound(SHA_string); ++itr) 
                { 
                    counter++;
                    tempstr.append(itr->second);
                    tempstr.append(":"); 
                }

                tempstr=tempstr.substr(0,tempstr.length()-1);
                string temp=to_string(counter);
                temp=temp+":"+tempstr;
                // tempstr.insert(0,to_string(counter));
                // tempstr.insert(1,1,':');

                send(new_socket, temp.c_str(), strlen(temp.c_str()), 0);
                write_in_log("Client ask for seeder_list");

            }
            else if(command=="remove")
            {
                //////cout<<"in remove command\n";
                message=message.substr(found+1);
                found = message.find('|');
                SHA_string=message.substr(0,found);
                
                message=message.substr(found+1);
                found = message.find('|');
                client_IP=message.substr(0,found);

                message=message.substr(found+1);
                found = message.find('|');
                client_Port=message.substr(0,found);

                //////cout<<SHA_string<<":"<<client_IP<<":"<<client_Port<<":";

                client_IP.append(":");
                client_IP.append(client_Port);

                //////cout<<SHA_string<<"\n";
                ////cout<<client_IP<<"\n";

                for (itr = seeder_list.lower_bound(SHA_string); itr != seeder_list.upper_bound(SHA_string); ++itr) 
                { 
                    if(itr->second==client_IP)
                    {
                        ////cout<<"got matching string";
                        seeder_list.erase(itr);
                        break;
                    } 
                }
                update_seeder_file();
                write_in_log("Client requests to remove it as seeder_list for given mtorrentfile");
                // for (itr = seeder_list.lower_bound(SHA_string); itr != seeder_list.upper_bound(SHA_string); ++itr) 
                // { 
                //     if(itr->second==client_IP)
                //     {
                //         ////cout<<itr->second<<"\n";
                //     } 
                // }
            }
            else if(command=="delete")
            {
                ////cout<<"in delete";
                message=message.substr(found+1);
                ////cout<<message;
                for(auto itr = seeder_list.cbegin(); itr != seeder_list.cend(); ++itr)
                {
                    if(itr->second==message)
                        seeder_list.erase(itr);      
                }

                update_seeder_file();
                write_in_log("Client "+message+" is closing off");
            }
             


            //add new socket to array of sockets  
            for (i = 0; i < max_clients; i++)   
            {   
                //if position is empty  
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = new_socket;   
                  //  printf("Adding to list of sockets as %d\n" , i);   
                         
                    break;   
                }   
            }   
            update_seeder_file();
        }   
             
        //else its some IO operation on some other socket
        else
        { 
        for (i = 0; i < max_clients; i++)   
        {   
            sd = client_socket[i];   
                 
            if (FD_ISSET( sd , &readfds))   
            {   
                //Check if it was for closing , and also read the  
                //incoming message  
                if ((valread = read( sd , buffer, 1024)) == 0)   
                {   
                    //Somebody disconnected , get his details and print  
                    getpeername(sd , (struct sockaddr*)&address ,(socklen_t*)&addrlen); 
                //    printf("Host disconnected , ip %s , port %d \n" ,inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close( sd );   
                    client_socket[i] = 0;   
                }   
                     
                //Echo back the message that came in  
                else 
                {   
                    //set the string terminating NULL byte on the end  
                    //of the data read  
                    buffer[valread] = '\0';   
                    send(sd , buffer , strlen(buffer) , 0 );   
                }   
            }   
        } 
        }  
    }   
         
    return 0;   
}       