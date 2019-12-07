#include <stdio.h> 
#include <iostream>
#include <fstream>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <termios.h>
#include <stack>
#include <vector>
#include <string>
#include <iostream>
#include <ftw.h>
#include <sys/ioctl.h>
#include <openssl/sha.h>
#include <thread> 
#include<map>
#include <algorithm>
#include <mutex> 
#include <signal.h>
#include <time.h>
using namespace std;
#define TRUE   1

struct stat stat_buf;
string clientIP_Port,tracker1IP_Port,tracker2IP_Port,client_IP,client_Port,tracker1_IP,tracker1_Port,tracker2_IP,tracker2_Port,filesize,mtorrentfile="./mtorrentsclient.txt";
char* current_Path = getenv ("PWD");
string currentpath=current_Path;
int curser=0,commandmodeline=1,curser_column=15,flag=1;
map <string,string> file_path;
map <string,string> mtorrent_filepath;
map <string,string> chunks;
map <string,string> show_download_list;
map<string,string>::iterator it;

std::mutex mtx;
ofstream fotorrent;
ifstream fitorrent;
int pflag=0;


string file_original_path="./original_file_path";
string file_mtorrent_path="./mtorrent_file_path";
string file_chunks="./chunks";
string file_download_lists="./download_lists";
string log_file="./";

void write_in_log(string message)
{

    time_t now = time(0);

    string time = ctime(&now);
    time.substr (0,time.length()-1);
    std::fstream fout;
    fout.open(log_file,ios_base::out|ios_base::app);
    

    fout << message << " : " << time ;
    fout.close();
}


void update_from_file()
{
    string tempstr;
    std::fstream fin;

    if(!(stat(file_original_path.c_str(),&stat_buf)))
    {
        fin.open(file_original_path,ios_base::in|ios_base::binary);

        while(getline(fin,tempstr))
        {
            size_t found = tempstr.find('$');
            // cout<< tempstr.substr(0,found) << "$"<<tempstr.substr(found+1) << endl;
            file_path[tempstr.substr(0,found)]=tempstr.substr(found+1);
        }
        fin.close();            
    }

    if(!(stat(file_mtorrent_path.c_str(),&stat_buf)))
    {
        fin.open(file_mtorrent_path,ios_base::in|ios_base::binary);

        while(getline(fin,tempstr))
        {
            size_t found = tempstr.find('$');
            // cout<< tempstr.substr(0,found) << "$"<<tempstr.substr(found+1) << endl;
            mtorrent_filepath[tempstr.substr(0,found)]=tempstr.substr(found+1);
        }
        fin.close();            
    }
    if(!(stat(file_chunks.c_str(),&stat_buf)))
    {
        fin.open(file_chunks,ios_base::in|ios_base::binary);

        while(getline(fin,tempstr))
        {
            size_t found = tempstr.find('$');
            // cout<< tempstr.substr(0,found) << "$"<<tempstr.substr(found+1) << endl;
            chunks[tempstr.substr(0,found)]=tempstr.substr(found+1);
        }
        fin.close();            
    }
    if(!(stat(file_download_lists.c_str(),&stat_buf)))
    {
        fin.open(file_download_lists,ios_base::in|ios_base::binary);

        while(getline(fin,tempstr))
        {
            size_t found = tempstr.find('$');
            // cout<< tempstr.substr(0,found) << "$"<<tempstr.substr(found+1) << endl;
            show_download_list[tempstr.substr(0,found)]=tempstr.substr(found+1);
        }
        fin.close();            
    }
} 

void update_to_file()
{
    //string tempstr;
    std::fstream fout;

    fout.open(file_original_path,ios_base::out|ios_base::trunc);
    
    for(auto it = file_path.cbegin(); it != file_path.cend(); ++it)
    {
        fout << it->first << "$" << it->second<< "\n";
    }
    fout.close();

    fout.open(file_mtorrent_path,ios_base::out|ios_base::trunc);
    
    for(auto it = mtorrent_filepath.cbegin(); it != mtorrent_filepath.cend(); ++it)
    {
        fout << it->first << "$" << it->second<< "\n";
    }
    fout.close();

    fout.open(file_chunks,ios_base::out|ios_base::trunc);
    
    for(auto it = chunks.cbegin(); it != chunks.cend(); ++it)
    {
        fout << it->first << "$" << it->second<< "\n";
    }
    fout.close();

    fout.open(file_download_lists,ios_base::out|ios_base::trunc);
    
    for(auto it = show_download_list.cbegin(); it != show_download_list.cend(); ++it)
    {
        fout << it->first << "$" << it->second<< "\n";
    }
    fout.close();            
}


void sig_handler(int sig)
{
    ;
}
                        
string findname(string str1) //finding current directory
{
    int i=str1.length()-1;
    string str2;

    while(str1[i]!='/')
        --i;

    str2=str1.substr(i+1,str1.length()-i-1); 

    return str2;
}

string findpath(string tempstr) //finding parent path
{
    int i=tempstr.length()-1;

    while(tempstr[i]!='/')
        i--;

    return tempstr.substr(0,i);
}

string makefullpath(string str1,string currentpath) //making absolute path
{
    string str2;
    if(str1[0]=='~')
        return str1.substr(1);
    else if(str1[0]=='/')
    {
        return str1;
    }
    else if(str1[0]=='.' && str1[1]!='.')
    {
        str2.append(currentpath);
        str2.append(str1.begin()+1,str1.end());
        return str2;   
    }
    else if(str1[0]=='.' && str1[1]=='.')
    {
        currentpath=findpath(currentpath);
        return makefullpath(str1.substr(3),currentpath);
    }
    else
    {
        str2.append(currentpath);
        str2.append("/");
        str2.append(str1);
        return str2;
    }
}
void statusbar(string tempstr) //printing status bar and path
{   
    cout<<"\033[1;1m"<<tempstr<<"\033[0m";
    cout<<"\n";
}
int connection(string port,string ip_address)
{
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char hello[50] = "Hello from client"; 
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
            
    memset(&serv_addr, '0', sizeof(serv_addr)); 
                   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(stoi(port)); 
                       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, ip_address.c_str(), &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
                   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

   return sock;
}
void getting_chunks(string SH2,vector <int> &final_chunks_getting,string seeder_port,string seeder_ip)
{
    string tempstr=SH2;                                          
    string temp;
    int sock=connection(seeder_port,seeder_ip);              
    tempstr.insert(0,"chunks:");
          
    send(sock,tempstr.c_str(),1024,0);
        
    int size,read_bytes=0;
    read(sock,&size,sizeof(size));
    char chn[size];

    while(read_bytes<size)
        read_bytes+=read( sock , chn, size);
    
    temp=chn;
    
    int k;
                            
    while(temp.length()!=0)
    {

        size_t found = temp.find(':');
        if(found!=std::string::npos)
        {
            final_chunks_getting.push_back(stoi(temp.substr(0,found)));
            temp=temp.substr(found+1);
        }
        else
        {
            final_chunks_getting.push_back(stoi(temp));
            break;
        }
    }

    sort(final_chunks_getting.begin(),final_chunks_getting.end());
    close(sock);
}

void download_chunks(string IP,string Port,vector <int> &client_chunk,string destination_filepath,string hash,fstream& fout,string mtorrent_path)
{
    for(int i=0;i<client_chunk.size();i++)
    {   
        
        int sock=connection(Port,IP);
        string message="download:";
        message.append(hash);
        message.append(":");
        message.append(to_string(client_chunk[i]));
       
        send(sock,message.c_str(),1024,0);
        
        int size,no_of_byes_read=0;
        int read_bytes=read(sock,&size,sizeof(size));
        
        mtx.lock();        
        fout.seekp (client_chunk[i]*512*1024, fout.beg);
        
        while (no_of_byes_read<size) 
        {
            char buffer[512*1024];
            int val=read(sock,buffer,512*1024);
            no_of_byes_read+=val;
            fout.write(buffer,val);
        }

        it =chunks.find(hash);
        if (it == chunks.end())
        {
            file_path[hash]=destination_filepath;
            chunks[hash]=to_string(client_chunk[i]);
            string str="share";
            str=str+"|"+destination_filepath+"|"+hash+"|"+client_IP+"|"+client_Port+"\0";
            int new_sock=connection(tracker1_Port,tracker1_IP);
            int size=strlen(str.c_str());
            send(new_sock,&size,sizeof(size),0);
            send(new_sock,str.c_str(),size,0);
            close(new_sock);         
        }
        else
        {
            string str=chunks[hash];
            str.append(":");
            str.append(to_string(client_chunk[i]));
            chunks[hash]=str;
        }
        
        it =show_download_list.find(hash);
        if (it == show_download_list.end())
        {
            show_download_list[hash]="[D] "+destination_filepath;
        }
        
        it =mtorrent_filepath.find(hash);
        if (it == mtorrent_filepath.end())
        {
            mtorrent_filepath[hash]=mtorrent_path;
        }
        
        mtx.unlock();
        close(sock);
        
    }
}
void upload_chunks(string SHA_chunk_string,int socket)
{
    string path;
    size_t found = SHA_chunk_string.find(':');
    path=SHA_chunk_string.substr(0,found);

    SHA_chunk_string=SHA_chunk_string.substr(found+1); //
    
    string path_of_file=file_path[path];
    
    std::fstream fin;
    fin.open(path_of_file, ios_base::in|ios_base::binary);


    char buffer1[512*1024];
    int size;
    fin.seekg (stoi(SHA_chunk_string)*512*1024, fin.beg);
                                    
    fin.read(buffer1,1024*512);
                                    
    size=fin.gcount();
                                    
                                    
    send(socket,&size,sizeof(size),0);
    
    send(socket,buffer1,size,0);
    fin.close();
}
void sharing(vector <string> v)
{

    string local_file_path=makefullpath(v[1],currentpath);
    string filename_with_extension=makefullpath(v[2],currentpath);

    stat(local_file_path.c_str(),&stat_buf);
                        
    int size=stat_buf.st_size;
    int tempsize=size;
    
    filesize=to_string(size);
    unsigned char digest[20];
    char mdString[SHA_DIGEST_LENGTH*2];
                        
    string SH1, SH2;
    std::fstream fin,fout;
                        
    fin.open(local_file_path, ios_base::in|ios_base::binary);
    fout.open(filename_with_extension, ios_base::out);

    fout << tracker1_IP.c_str() << ":";
    fout << tracker1_Port.c_str() << "\n";
    fout << tracker2_IP.c_str() << ":";
    fout << tracker2_Port.c_str() << "\n";
    fout << local_file_path.c_str() << "\n";
    fout << filesize.c_str() << "\n";


    while (tempsize>0) 
    {   

        if(tempsize>1024*512)
        {
            char buffer[1024*512];
            fin.read(buffer,1024*512);
            SHA1((unsigned char*)&buffer, strlen(buffer), (unsigned char*)&digest);

            for (int i = 0; i <20; i++)
                sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);

            SH1=mdString;
            SH2.append(SH1.substr(0,20));                  
        }
        else
        {
            char buffer[tempsize];
            fin.read(buffer,tempsize);
            SHA1((unsigned char*)&buffer, strlen(buffer), (unsigned char*)&digest);

            for (int i = 0; i <20; i++)
                sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);

            SH1=mdString;
            SH2.append(SH1.substr(0,20)); 
        }
        tempsize-=1024*512;
    }

    fout << SH2.c_str();
    
    SHA1((unsigned char*)SH2.c_str(), strlen(SH2.c_str()), (unsigned char*)&digest);

    for (int i = 0; i <20; i++)
        sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);

    SH1=mdString;
    SH1=SH1.substr(0,20);



    file_path[SH1]=local_file_path;
    mtorrent_filepath[SH1]=filename_with_extension;
    tempsize=size/(1024*512);
    if(size%(1024*512)!=0)
        tempsize++;

    string temp;
    for(int i=0;i<tempsize;i++)
    {
        temp.append(to_string(i));
        if(i<tempsize-1)
            temp.append(":");
    }

    chunks[SH1]=temp;
                        

    SH2.clear();
    SH2="share|";
    SH2.append(findname(local_file_path));
    SH2.append("|");
    SH2.append(SH1);
    SH2.append("|");
    SH2.append(client_IP);
    SH2.append("|");
    SH2.append(client_Port);
    SH2.append("\0");
    int size1=strlen(SH2.c_str());

    int sock=connection(tracker1_Port,tracker1_IP);
    send(sock,&size1,sizeof(size1),0);
    send(sock,SH2.c_str(),size1,0);
    printf("Message sent\n"); 
    
    fin.close();
    fout.close();
    close(sock);
    v.clear();
    cout<<"\033["<<commandmodeline<<";"<<1<<"H"<<flush;   
    cout<<"\e[2K"<<flush;
    statusbar("Enter Command:");
    curser=0,commandmodeline=1,curser_column=15;
    cout<<"\033["<<commandmodeline<<";"<<curser_column<<"H"<<flush;

    update_to_file();
    write_in_log("Sharing mtorrentfile "+filename_with_extension);
}

void getting(vector <string> v)
{
    unsigned char digest[20];
    char mdString[SHA_DIGEST_LENGTH*2];
    char tempbuffer[1024*1024];
    string tempstr;
    string SH1,SH2;
    
    std::fstream fin,fout;

    string mtorrent_file_path=makefullpath(v[1],currentpath);
    string destinstion_path=makefullpath(v[2],currentpath);


    fin.open(mtorrent_file_path,ios_base::in|ios_base::binary);

    for (int i=0; i<5;i++)
    {
        getline(fin,SH1);
    }
    
    fin.close();
    SHA1((unsigned char*)SH1.c_str(), strlen(SH1.c_str()), (unsigned char*)&digest); 

    for (int i = 0; i <20; i++)
        sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
                        
                        
    SH1=mdString;
    SH1=SH1.substr(0,20);
    SH2=SH1;
                       

    SH1.insert(0,"get|");
    SH1.append("\0");

    int sock=connection(tracker1_Port,tracker1_IP);
    int size1=strlen(SH1.c_str());
    send(sock,&size1,sizeof(size1),0);        
    send(sock,SH1.c_str(),size1,0);

    read( sock , tempbuffer, 1024*1024);
    close(sock);

    int no_of_seeder=0;
    vector <string> seeder_ip;
    vector <string> seeder_port;
    string seeder;
    seeder=tempbuffer;

    no_of_seeder=seeder[0]-'0'; 
    seeder=seeder.substr(2);
    for(int i=0;i<no_of_seeder*2;i++)
    {
        size_t found = seeder.find(':');
        if(i%2==0)
            seeder_ip.push_back(seeder.substr(0,found));
        else
            seeder_port.push_back(seeder.substr(0,found));
        if(i!=no_of_seeder*2-1)
            seeder=seeder.substr(found+1);
    }

    std::vector<int> final_chunks_getting[no_of_seeder];           //ggggggggggggggg
    std::vector<int> final_chunks_downloading[no_of_seeder];           
    std::thread mythread[no_of_seeder];  
    for(int i=0;i<no_of_seeder;i++)                                //i
    {
        mythread[i]=std::thread(getting_chunks,SH2,std::ref(final_chunks_getting[i]),seeder_port[i],seeder_ip[i]); //std::thread t1(sharing, std::ref(v));
    }
    for(int i=0;i<no_of_seeder;i++)                                //i
    {
        mythread[i].join();
    }

    string size;

    fin.open(mtorrent_file_path,ios_base::in|ios_base::binary);

    for (int i=0; i<4;i++)
    {
        getline(fin,size);
    }
    fin.close();

    fout.open(destinstion_path,ios_base::out);

                        
    int no_of_chunks=stoi(size)/(512*1024);
    if(stoi(size)%(512*1024)!=0)
        no_of_chunks++;

    int visited_chunks[no_of_chunks]={0,0};
    int temp_num_of_chunks=no_of_chunks;
    int seeder_pointer[no_of_seeder]={0,0};

                     
    while(temp_num_of_chunks>0)
    {
        for(int i=0;i<no_of_seeder;i++)
        {
            int j=seeder_pointer[i];
            while(j<=final_chunks_getting[i].size()-1 && visited_chunks[final_chunks_getting[i][j]]==1)
            {
                seeder_pointer[i]++;
                j=seeder_pointer[i];
            }
            
            if(j<=final_chunks_getting[i].size()-1)
            {
                visited_chunks[final_chunks_getting[i][j]]=1;   
                final_chunks_downloading[i].push_back(final_chunks_getting[i][j]);
                seeder_pointer[i]++;
                temp_num_of_chunks--;
            }
            
            if(temp_num_of_chunks==0)
                i=no_of_seeder;
        }    
    }

    std::thread my1thread [no_of_seeder];
    for(int i=0;i<no_of_seeder;i++)
    {
        my1thread[i]=std::thread(download_chunks,seeder_ip[i],seeder_port[i],std::ref(final_chunks_downloading[i]),destinstion_path,SH2,std::ref(fout),mtorrent_file_path);
    }

    for(int i=0;i<no_of_seeder;i++)
    {
        my1thread[i].join();
    }
    show_download_list[SH2]="[S] "+destinstion_path;   
    fout.close();
    v.clear();
    cout<<"\033["<<commandmodeline<<";"<<1<<"H"<<flush;   
    cout<<"\e[2K"<<flush;
    statusbar("Enter Command:");
    curser=0,commandmodeline=1,curser_column=15;
    cout<<"\033["<<commandmodeline<<";"<<curser_column<<"H"<<flush;


    update_to_file();
    write_in_log("Downloaded a file "+destinstion_path);
}


void act_as_server()
{
   
    int opt = TRUE;   
    int master_socket , addrlen , new_socket , client_socket[100] ,  max_clients = 100 , activity, i , valread , sd;   
    int max_sd;   
    struct sockaddr_in address;   
         
    char buffer[1025];  //data buffer of 1K  
         
    fd_set readfds;   
         
    for (i = 0; i < max_clients; i++)   
    {   
        client_socket[i] = 0;   
    }   
         
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons(stoi(client_Port));   
         
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
         
    if (listen(master_socket, 100) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    addrlen = sizeof(address);   
         
    while(TRUE)   
    {   
        FD_ZERO(&readfds);   
     
        FD_SET(master_socket, &readfds);   
        max_sd = master_socket;   
             
        for ( i = 0 ; i < max_clients ; i++)   
        {   
            sd = client_socket[i];   
                 
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            if(sd > max_sd)   
                max_sd = sd;   
        }   
     
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
       
        if(pflag)
            return;       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        if (FD_ISSET(master_socket, &readfds))   
        {   
            if ((new_socket = accept(master_socket,  
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             

            valread=read( new_socket , buffer, 1024);
            string command;
            string message=buffer;

            size_t found = message.find(':');
            command=message.substr(0,found);

            if(command=="chunks")
            {
                message=message.substr(found+1);
                int size=strlen(chunks[message].c_str());
                send(new_socket,&size,sizeof(size),0);
                send(new_socket, chunks[message].c_str(),size, 0);
                write_in_log("Client asked for chunk_no of "+file_path[message]);
            }
            else if(command=="download")
            {
                message=message.substr(found+1);
                upload_chunks(message,new_socket);
                write_in_log("Sending chunks of "+file_path[message]);
            }
           
            for (i = 0; i < max_clients; i++)   
            {   
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = new_socket;   
                    break;   
                }   
            }   
        }   
        else
        {     
        for (i = 0; i < max_clients; i++)   
        {   
            sd = client_socket[i];   
                 
            if (FD_ISSET( sd , &readfds))   
            {   
                if ((valread = read( sd , buffer, 1024)) == 0)   
                {   
                    getpeername(sd , (struct sockaddr*)&address ,(socklen_t*)&addrlen); 
                         
                    close( sd );   
                    client_socket[i] = 0;   
                }   
                     
                else 
                {   
                    buffer[valread] = '\0';   
                    send(sd , buffer , strlen(buffer) , 0 );   
                }   
            }   
        }
        }   
    }
}


void findIP(string clientIP_Port,string tracker1IP_Port,string tracker2IP_Port)
{
    size_t found = clientIP_Port.find(':');
    client_IP=clientIP_Port.substr(0,found);
    client_Port=clientIP_Port.substr(found+1);

    size_t found1 = tracker1IP_Port.find(':');
    tracker1_IP=tracker1IP_Port.substr(0,found1);
    tracker1_Port=tracker1IP_Port.substr(found1+1);
    
    size_t found2 = tracker2IP_Port.find(':');
    tracker2_IP=tracker2IP_Port.substr(0,found2);
    tracker2_Port=tracker2IP_Port.substr(found2+1);
}


void gotoNonCanon() //going to non canon
{
    struct termios initial_settings, new_settings;
    FILE *input;
    FILE *output;

    input = fopen("/dev/tty", "r");
    output = fopen("/dev/tty", "w");
    tcgetattr(fileno(input),&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;

    if(tcsetattr(fileno(input), TCSANOW, &new_settings) != 0) 
    {
        fprintf(stderr,"could not set attributes\n");
    }
  
//  tcsetattr(fileno(input),TCSANOW,&initial_settings);
}

void clearscreen()
{
    cout<<"\033[3J";
    cout<<"\033[H\033[J";
    statusbar("Enter Command: ");
}


int main(int argc, char *argv[])
{
    
    gotoNonCanon(); 
    //Going to Non Canon mode.
    
    cout<<"\033[3J";
    cout<<"\033[H\033[J";
   
    statusbar("Enter Command:");
    cout<<" ";
    
    clientIP_Port=argv[1];
    tracker1IP_Port=argv[2];
    tracker2IP_Port=argv[3];
    log_file.append(argv[4]);
    
    
    findIP(clientIP_Port,tracker1IP_Port,tracker2IP_Port);
    update_from_file();

   
    thread th1(act_as_server); 

    cout<<"\033["<<commandmodeline<<";"<<curser_column<<"H"<<flush;
    while(flag)
    { 
        char command[500];
        char buffer[500];
        read(0,buffer,128);
        if(buffer[0]==27 && buffer[1]!=91) // Esc key , Going back to normal mode
        {   
            clearscreen();
            flag=0;
        }

        else if(buffer[0]!=27 && buffer[0]!=127 && buffer[0]!=10)  //Entering Input or commands.
        {
            command[curser++]=buffer[0];
            cout<<buffer[0]<<flush;
            curser_column++;
        }
        else if(buffer[0]==127 || buffer[0]==8) // Pressing Back space to Erase text.
        {
            if(curser_column>15)
            {
                command[--curser]=' ';
                command[curser+1]='\0';
                cout<<"\033["<<commandmodeline<<";15H"<<flush;
                cout<<command<<flush;
                command[curser]='\0';
                cout<<"\033["<<commandmodeline<<";"<<--curser_column<<"H"<<flush;
            }
                  
        }
        else if(buffer[0]==10) // Pressing Enter key to execute commands
        {
            std:: vector<string> v;
            command[curser]='\0';
            char s[1000];
            int j=0,i;

            for(i=0;command[i]!='\0';i++) // Differentiate Operation and Arguments.
            {
                if(command[i]==' ')
                {
                    if(command[i-1]==92)
                        s[j++]=command[i];
                    else
                    {
                        s[j]='\0';
                        v.push_back(s); 
                        j=0;    
                    }
                }
                else if(command[i]!=92)
                {
                    s[j++]=command[i];
                }
            }

            s[j]=command[i];
            v.push_back(s);
            cout<<"\n"<<flush;
            cout<<"\e[2K"<<flush;
            if(v[0]=="share")
            {
                clearscreen();
                std::thread t1(sharing,v);
		        t1.detach();
            }
             
            if(v[0]=="get")
            {
                clearscreen();
                std::thread t2(getting,v);
                t2.detach();
            }
            if(v[0]=="show" && v[1]=="downloads")
            {
                clearscreen();

                for(map<string,string>::const_iterator itr =show_download_list.begin();itr != show_download_list.end(); ++itr)
                {
                    cout << itr->second<< "\n";
                }
            }
            if(v[0]=="show" && v[1]=="mtorrents")
            {
                clearscreen();

                cout<<"\n";
                for(map<string,string>::const_iterator itr =mtorrent_filepath.begin();itr != mtorrent_filepath.end(); ++itr)
                {
                    cout << itr->second<< "\n";
                }
            }
            if(v[0]=="remove")
            {
                clearscreen();
                unsigned char digest[20];
                char mdString[SHA_DIGEST_LENGTH*2];
                char tempbuffer[1024*1024];
                string tempstr;
                string SH1,SH2;
                
                std::fstream fin,fout;

                string mtorrent_file_path=makefullpath(v[1],currentpath);


                fin.open(mtorrent_file_path,ios_base::in|ios_base::binary);

                for (int i=0; i<5;i++)
                {
                    getline(fin,SH1);
                }
                
                fin.close();
                SHA1((unsigned char*)SH1.c_str(), strlen(SH1.c_str()), (unsigned char*)&digest); 

                for (int i = 0; i <20; i++)
                    sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
                                    
                                    
                SH1=mdString;
                SH1=SH1.substr(0,20);
                SH2=SH1;
                    
                while(show_download_list[SH2].substr(0,3)=="[D]");                               

                SH1.insert(0,"remove|");
                SH1=SH1+"|"+client_IP+"|"+client_Port;
                SH1.append("\0");


                int sock=connection(tracker1_Port,tracker1_IP);
                int size1=strlen(SH1.c_str());
                send(sock,&size1,sizeof(size1),0);        
                send(sock,SH1.c_str(),size1,0);
                close(sock);
               
                it = mtorrent_filepath.find(SH2);
                if (it != mtorrent_filepath.end())
                {
                    mtorrent_filepath.erase(it);
                }
                remove(mtorrent_file_path.c_str());

                write_in_log("Removing mtorrentfile "+mtorrent_file_path);
            }
            if(v[0]=="exit")
            {
                clearscreen();
                string message="delete|"+client_IP+":"+client_Port;
                int sock=connection(tracker1_Port,tracker1_IP);
                int size1=strlen(message.c_str());
                send(sock,&size1,sizeof(size1),0);        
                send(sock,message.c_str(),size1,0);
                close(sock);

                flag=0;

            }

            v.clear();
            cout<<"\e[3K"<<flush;
            cout<<"\033["<<commandmodeline<<";"<<1<<"H"<<flush;   
            statusbar("Enter Command:");

            curser=0,commandmodeline=1,curser_column=15;
            cout<<"\033["<<commandmodeline<<";"<<curser_column<<"H"<<flush;

        }

        buffer[0]=buffer[1]=buffer[2]=0;
    } 
    pflag=1;
    pthread_kill(th1.native_handle(), SIGUSR1);
    signal(SIGUSR1, sig_handler);
    return 0;
}

