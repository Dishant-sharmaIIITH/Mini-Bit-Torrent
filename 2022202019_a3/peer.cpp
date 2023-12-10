#include <sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<pthread.h>
#include<sys/types.h>
#include<signal.h>
#include<iostream>
#include<vector>
#include<string>
#include <fcntl.h>
#include <bits/stdc++.h>

using namespace std;

char* peer_ip;
int peer_port;

int chunk_size=16384;

//int max_fsize=1mb
map<int,vector<string>> avalchunk;

map<string,pair<vector<int>,string>>bitv;
map<string,int> port_sockfd;
map<string,vector<string>> port_chunkv;
map<string,int> uploadfsize;

string findfilename(string filepath){
    int ind=-1;
    for(int i=filepath.size()-1;i>=0;i--){
        if(filepath[i]=='/'){
            ind=i;
            break;
        }
    }
    return filepath.substr(ind+1,filepath.size()-ind-1);
}

vector<string> commandvector(string input){
    vector<string> res;
    string temp="";
    for(int i=0;i<input.size();i++){
        if(input[i]==' '&&(i!=input.size()-1)){
            res.push_back(temp);
            temp="";


        }else{
            temp+=input[i];
        }
    }
    res.push_back(temp);
    return res;
} 
static bool ter=false;

int get_file_size(string file_name)
{
    ifstream input_file(file_name, ios::binary);
    input_file.seekg(0, ios::end);
    int file_size = input_file.tellg();
    // file_size /= 1024;
    return file_size;
}
void updatebitv(string path){
    string fname=findfilename(path);
    int fsize=get_file_size(path);
    int no_ofchunk=ceil(((double)fsize)/chunk_size);
    // cout<<no_ofchunk<<"line no 74"<<endl;
    for(int i=0;i<no_ofchunk;i++){
        bitv[fname].first.push_back(1);
        bitv[fname].second=path;
    }
// for(auto pai:bitv){
//     cout<<pai.first<<" "<<" "<<pai.second.second<<endl;
//     for(int i:pai.second.first){
//     cout<<i<<" ";
//     }
// }
}



void * nextpeerfun(void * nextclient){
    int* sockfdpointer=(int*)nextclient;
    int sockfd=*sockfdpointer;
    // cout<<"hello new taker of file come"<<endl;

    char buff[chunk_size];
    
        
    while(1){
        bzero(buff,sizeof(buff));
        memset(&buff, 0, sizeof(buff));
        // cout<<"line no 91"<<endl;

        recv(sockfd,buff,sizeof(buff),0);
        string response=string(buff);
        if(response.size()==0){
            cout<<"no response"<<endl;
            break;
        }
        // cout<<response<<" comming from taker"<<endl;

        vector<string> rv=commandvector(response);
        string mess="";
        if(rv[0]=="tellchunks"){

            // cout<<"Inside tellchunks"<<endl;
            // cout<< "argument of tellchunks"<<rv[1]<<endl; 
            if(bitv.find(rv[1])==bitv.end()){
                // cout<<"pappu"<<endl;
                
            }else{
                for(int i=0;i<bitv[rv[1]].first.size();i++){
                    mess+=to_string(bitv[rv[1]].first[i])+" ";
                }
                mess+=to_string(uploadfsize[rv[1]]);
            }
            // cout<<"line no 110"<<mess<<endl;
            bzero(buff,sizeof(buff));
            memset(&buff, 0, sizeof(buff));
            strcpy(buff,mess.c_str());
            send(sockfd,buff,sizeof(buff),0);
        }else if(rv[0]=="transfer"){
            // cout<<"Inside transfer"<<endl;


            int chunkno=stoi(rv[1]);
            string fname=rv[2];
            string fullpath;
            for(auto path:bitv){
                if(findfilename(path.first)==fname){
                    // cout<<"file path 124"<<path.second.second<<endl;
                    fullpath=path.second.second;
                }
            }
            FILE * sfp=fopen(fullpath.c_str(),"rb");
            
            if(sfp==NULL){
                cout<<"Error in opening file at sender end"<<endl;
            }else{
                int fs = fseek(sfp, chunkno * chunk_size, SEEK_SET);
                if (fs != 0)
                {
                    perror("seek negative: ");
                    // cout<<"seek error 125"<<endl;
                    //mess = "seek nonzero ";
                }else{
                    char buff1[chunk_size];
                     bzero(buff1,sizeof(buff1));
                     memset(&buff1, 0, sizeof(buff1));
                    int readsize = fread(buff1, sizeof(char), chunk_size, sfp);
                    if (readsize > 0)
                    {
                        string str(buff1);
                        cout << "readsize " << readsize << endl;
                        // cout<<"read successfull";
                        mess = to_string(readsize)+ " " + str ;
                        //cout<<mess<<endl; 
                        
                    }
                    else
                    {
                        
                        cout<<"Not read done"<<endl;
                        //mess = "not read";
                    }
                }
            }
            //cout<<mess<<endl; 
            bzero(buff,sizeof(buff));
            memset(&buff, 0, sizeof(buff));
            strcpy(buff,mess.c_str());
            send(sockfd,buff,sizeof(buff),0);
            fclose(sfp);

        }

    }    
    
    
    
    
        
        
        
        
        
        // if(response.size()==0){
        //     cout<<"No response"<<endl;
        // }else{
        //     cout<<"from peer : "<<response<<endl;
        // }
        // string inp="";
        // cout<<"Enter message : ";
        // getline(cin,inp);
        // if(inp=="quit"){
        //     exit(0);
        // }
        // send(sockfd,buff,sizeof(buff),0);
        
       
    
    

}
vector<string> getsizeandmess(string mess){
    vector<string> v;
    v.push_back("");
    v.push_back("");
    long long int i=0;
    for(i=0;i<mess.size();i++){
        if(mess[i]==' '){
            break;
        }else{
            v[0]+=mess[i];
        }
    }
    v[1]+=mess.substr(i+1,mess.size()-1-i);
    return v;

}
struct sender_info{
    int sockfd;
    string ip;
    string port;
    string filename;
    string despath;
    int fsize;
};

void * communicate_peer(void * info){

    sender_info* sinfo=(sender_info*) info;
    
    int sockfd=sinfo->sockfd;
    
    // cout<<"hello connection established to get file from peer"<<endl;

    string despath=sinfo->despath;
    string filename=sinfo->filename;
    int no_of_chunk=ceil(double(sinfo->fsize)/chunk_size);
    cout<<"des path "<<despath<<" filename "<<filename<<" filesize "<<" no_of_chunk "<<no_of_chunk<<endl;
    FILE* fp;
    fp=fopen(despath.c_str(),"w+");
    if(fp==NULL){
        cout<<"error in openning file at reaciever end"<<endl;


    }else{
        if(fallocate(fileno(fp),0,0,sinfo->fsize)==0){
            cout<<"file size allocated"<<endl;

            
            for(int i=0;i<no_of_chunk;i++){
                string response="";
                char buff[chunk_size];
                bzero(buff, sizeof(buff));
                memset(&buff, 0, sizeof(buff));
                string request="transfer "+to_string(i)+" "+filename;
                // cout<<" request of ith chunk "<<request<<endl;
                strcpy(buff,request.c_str());
               
                send(sockfd,buff,sizeof(buff),0);
                
                bzero(buff,sizeof(buff));
                memset(&buff, 0, sizeof(buff));
                recv(sockfd,buff,sizeof(buff),0);
                response=string(buff);
                vector<string> rv=getsizeandmess(response);
                // cout<<rv[0]<<endl;
                // cout<<rv[1]<<endl;
                bzero(buff,sizeof(buff));
                memset(&buff, 0, sizeof(buff));
                strcpy(buff,rv[1].c_str());

                //cout<<" response of ith chunk "<<response<<endl;

                fseek(fp, i*chunk_size, SEEK_SET);
                
                // cout << out[1].c_str() << endl;
                // cout<<"size "<<rv[0]<<endl;
                
                fwrite(buff, sizeof(char), stoi(rv[0]), fp);
            }
            



        }else{
            cout<<"error in allocation of file"<<endl;
        }

    }
    fclose(fp);

    // char buff[16384];
    
    //     string inp="tellchunks "+sinfo->filename;
        
    //     if(inp=="quit"){
    //         exit(0);
    //     }
    //     bzero(buff,sizeof(buff));
    //     strcpy(buff,inp.c_str());
    //     send(sockfd,buff,sizeof(buff),0);
        
    //     bzero(buff,sizeof(buff));
    //     recv(sockfd,buff,sizeof(buff),0);
    //     string response=string(buff);
    //     vector<string> rv=commandvector(response);
    //     if(response.size()==0){
    //         cout<<"No response"<<endl;
    //     }else{
    //         cout<<"from peer : "<<response<<endl;
    //         port_sockfd[sinfo->port]=sockfd;
    //         port_chunkv[sinfo->port]=rv;


    //     }

        

    
    

}




void connectwithpeer(vector<string>peerip,vector<string>peerport,string gid,string fname,string despath){
    int numpeers=peerip.size();
    
    // cout<<"namaste3"<<endl;
     int filesize;
    for(int i=0;i<numpeers;i++){

        const char* ip=peerip[i].c_str();
        int port=stoi(peerport[i]);
        struct sockaddr_in peeraddr;
        char buff[chunk_size];
        int sockfd,connfd;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
         bzero(&peeraddr, sizeof(peeraddr));

        peeraddr.sin_family = AF_INET;
        
        peeraddr.sin_port = htons(port);
         if (inet_pton(AF_INET, ip, &peeraddr.sin_addr) <= 0)
        {
            cout << "Error in inet_pton for " << ip;
            exit(-1);
        }
        socklen_t peerlen=sizeof(peeraddr);
        
        connfd=connect(sockfd,(struct sockaddr* )&peeraddr,peerlen);
        
        if(connfd!=0){
            printf("connection not  established\n");
            exit(0);

            
            
        }
        // cout<<ntohs(peeraddr.sin_port)<<"gande"<<endl;
        // cout<<inet_ntoa(peeraddr.sin_addr)<<"bache"<<endl;
        
        // int * pclient=(int*)malloc(sizeof(int));
        // *pclient=sockfd;
        // sender_info info;
        
        // info.sockfd=sockfd;
        // info.filename=fname;
        // info.ip=peerip[i];
        // info.port=peerport[i];
        // info.despath=despath;

    
        cout<<"hello connection established to get file from peer"<<endl;

         
    
        string inp="tellchunks "+fname;
        
        if(inp=="quit"){
            exit(0);
        }
        bzero(buff,sizeof(buff));
        memset(&buff, 0, sizeof(buff));
        strcpy(buff,inp.c_str());
        // cout<<"tellchunk string "<<inp;
        send(sockfd,buff,sizeof(buff),0);
        
        bzero(buff,sizeof(buff));
        memset(&buff, 0, sizeof(buff));
        recv(sockfd,buff,sizeof(buff),0);
        string response=string(buff);
        vector<string> rv=commandvector(response);
        if(response.size()==0){
            cout<<"No response"<<endl;
        }else{
            cout<<"from peer : "<<response<<endl;
            port_sockfd[peerport[i]]=sockfd;
           for(int j=0;j<rv.size()-1;j++){
                port_chunkv[peerport[i]].push_back(rv[j]);
           }
        //    cout<<rv[rv.size()-1]<<"hell"<<endl;
           filesize=stoi(rv[rv.size()-1]);


        }

        
        

    }
    pthread_t peers[numpeers];
    int temp=0;
    for(auto port:port_sockfd){
        sender_info info;
        
        info.sockfd=port.second;
        info.filename=fname;
        info.fsize=filesize;
        
        info.port=port.first;
        info.despath=despath;
        pthread_create(&peers[temp++],NULL,communicate_peer,&info);
        pthread_join(peers[temp-1], NULL);

    }
    
    
}



void* peer_fun(void * pclient){
    int* sockfdpointer=(int*)pclient;
    int sockfd=*sockfdpointer;
    ter=true;

    // cout<<"client thread come"<<endl;
    string clientuid="";

   
   
    while(1){
        cout<<"Enter commands : ";
        string input="";
        vector<string> commandv;
        vector<string> v_response;
        getline(cin,input);
        // cout<<input<<" Hello hjsbddfhbhjdsfbv"<<endl;
        string request="";
        if(input.size()==0){
            
            cout<<"Please enter valid command"<<endl;
            continue;
        }
        char buffer[1024];
        commandv=commandvector(input);
        if(commandv[0]=="Create_user"||commandv[0]=="create_user"){
            if(commandv.size()!=3){
                cout<<"User_id and Password are required to create new User"<<endl;
                continue;
            }
            string peerip=string(peer_ip);
            request+="1 "+commandv[1]+" "+commandv[2]+" "+peerip+" "+to_string(peer_port);

        }else if(commandv[0]=="Login"||commandv[0]=="login"){
             if(commandv.size()!=3){
                cout<<"User_id and Password are required to Login"<<endl;
                continue;
            }else{
                string peerip=string(peer_ip);
                request+="2 "+commandv[1]+" "+commandv[2]+" "+peerip+" "+to_string(peer_port);

            }
        }
        else if(commandv[0]=="Create_group"||commandv[0]=="create_group"){
             if(commandv.size()!=2){
                cout<<"groupid required to create group"<<endl;
                continue;
            }
            request+="3 "+commandv[1]+" "+clientuid;
        }
        else if(commandv[0]=="Join_group"||commandv[0]=="join_group"){
             if(commandv.size()!=2){
                cout<<"groupid required to join group"<<endl;
                continue;
            }
            request+="4 "+commandv[1]+" "+clientuid;
        }
        else if(commandv[0]=="Leave_group"||commandv[0]=="leave_group"){
             if(commandv.size()!=2){
                cout<<"groupid required to leave group"<<endl;
                continue;
            }
            request+="5 "+commandv[1]+" "+clientuid;
        }
        else if(commandv[0]=="List_requests"||commandv[0]=="list_requests"){
             if(commandv.size()!=2){
                cout<<"group_id required to list pending request of group"<<endl;
                continue;
            }
            request+="6 "+commandv[1]+" "+clientuid;
        }
        else if(commandv[0]=="List_groups"||commandv[0]=="list_groups"){
             if(commandv.size()!=1){
                cout<<"No argument required for listing all groups "<<endl;
                continue;
            }
            request+="8 "+clientuid;
        }else if(commandv[0]=="List_files"||commandv[0]=="list_files"){
             if(commandv.size()!=2){
                cout<<"group_id required to list files available in the group"<<endl;
                continue;
            }
            request+="9 "+commandv[1]+" "+clientuid;
        }
        else if(commandv[0]=="Accept_request"||commandv[0]=="accept_request"){
             if(commandv.size()!=3){
                cout<<"group_id and user_id are required to accept the request"<<endl;
                continue;
            }
            request+="7 "+commandv[1]+" "+clientuid;
        }
        else if(commandv[0]=="Upload_file"||commandv[0]=="upload_file"){
             if(commandv.size()!=3){
                cout<<"file path and groupid required to upload file"<<endl;
                continue;
            }
            int fsize=get_file_size(commandv[1]);
            string fname=findfilename(commandv[1]);
            request+="10 "+fname+" "+commandv[2]+" "+clientuid+" "+to_string(fsize);
        }
        else if(commandv[0]=="Download_file"||commandv[0]=="download_file"){
             if(commandv.size()!=4){
                cout<<"file path and groupid required to download file"<<endl;
                continue;
            }
            request+="11 "+commandv[1]+" "+commandv[2]+" "+commandv[3]+" "+clientuid;
        }
        else if(commandv[0]=="Logout"||commandv[0]=="logout"){
             if(commandv.size()!=1){
                cout<<"No argument required"<<endl;
                continue;
            }
            request+="12 ";
        }
        else if(commandv[0]=="Stop_sharing"||commandv[0]=="stop_sharing"){
             if(commandv.size()!=1){
                cout<<"No argument required"<<endl;
                continue;
            }
            request+="13 ";
        }
        else if(commandv[0]=="Show_download"||commandv[0]=="show_download"){
             if(commandv.size()!=1){
                cout<<"No argument required"<<endl;
                continue;
            }
            request+="14 ";
        }
        else if(commandv[0]=="quit"){
            
            
            break;
        }
        else{
            cout<<"Invalid operation"<<endl;
            continue;
        }
        bzero(buffer,sizeof(buffer));
        strcpy(buffer,request.c_str());
        send(sockfd,buffer,sizeof(buffer),0);
        
        bzero(buffer,sizeof(buffer));
        recv(sockfd,buffer,sizeof(buffer),0);
        
        string response=string(buffer);
        if(response.size()==0){
            cout<<"No response"<<endl;
        }else{
            v_response=commandvector(response);
            if(v_response[0]=="done"&&int(request[0])==50){
                clientuid=v_response[1];

            }
            else if(v_response[0]=="done1"){
                vector<string> peerrequestip;
                vector<string> peerrequestport;
                // cout<<"namaste"<<endl;

                for(int i=2;i<v_response.size();i=i+4){
                    peerrequestip.push_back(v_response[i]);
                    // cout<<peerrequestip[0];
                    // cout<<"namaste1"<<endl;
                    
                }
                for(int i=3;i<v_response.size();i=i+4){
                    peerrequestport.push_back(v_response[i]);
                    // cout<<peerrequestport[0];
                    // cout<<"namaste2"<<endl;
                    
                }
                
                connectwithpeer(peerrequestip,peerrequestport,commandv[1],commandv[2],commandv[3]);
            }else if(v_response[0]=="done2"){
                updatebitv(commandv[1]);
                // cout<<commandv[1]<<endl;
                // cout<<findfilename(commandv[1])<<endl;
                // cout<<get_file_size(commandv[1])<<endl;

                uploadfsize[findfilename(commandv[1])]=get_file_size(commandv[1]);


            }
            
            cout<<response<<endl;
        }
       

    }
    // close(sockfd);
    // cout<<"loop ends"<<endl;
    
    return NULL;

}

int main(int argc,char** argv){
    if(argc!=5){
        printf("Invalid Number of arguments");
        return 0;
    }


    int sockfd,connfd;
    //struct sockaddr_in peeraddr;
    struct sockaddr_in trackeraddr;
    const char* ip=argv[3];
    int port=atoi(argv[4]);
    int opt=1;

    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd==-1){
        printf("Peer socket not created\n");
        return EXIT_FAILURE;
    }
    printf("Peer socket created\n");
    bzero(&trackeraddr, sizeof(trackeraddr));

     trackeraddr.sin_family = AF_INET;
    trackeraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    trackeraddr.sin_port = htons(port);

     
   
    // cout<<ntohs(trackeraddr.sin_port)<<endl;
    // cout<<inet_ntoa(trackeraddr.sin_addr)<<endl;

    //socklen_t cli_size=sizeof(addr);

    if (inet_pton(AF_INET, ip, &trackeraddr.sin_addr) <= 0)
    {
        cout << "Error in inet_pton for " << ip;
        exit(-1);
    }
    socklen_t trackerlen=sizeof(trackeraddr);
       
    connfd=connect(sockfd,(struct sockaddr* )&trackeraddr,trackerlen);
    
    if(connfd!=0){
        printf("connection not  established\n");
         return EXIT_FAILURE;

        
        
    }
    pthread_t ct;
    int * pclient=(int*)malloc(sizeof(int));
    *pclient=sockfd;
    
    pthread_create(&ct,NULL,peer_fun,pclient);

   

    

   // cout<<"after thread"<<endl;
    int peerservsockfd,peerservconnfd;
    struct sockaddr_in peeraddr;
    struct sockaddr_in nextpeeraddr;
     peer_ip=argv[1];
     peer_port=atoi(argv[2]);
    int peeropt=1;
    peerservsockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        printf("Peer srever socket not created\n");
        return EXIT_FAILURE;
    }
    //printf("Peer  server socket created\n");
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_addr.s_addr = inet_addr(peer_ip);
    peeraddr.sin_port = htons(peer_port);
    

     
    //cin>>temp;

    if (bind(peerservsockfd, (struct sockaddr *)&peeraddr, sizeof(peeraddr)) < 0)
    {
        printf("Error in binding server\n");
       
        return EXIT_FAILURE;
    }
   // printf("Binding done\n");
    if (listen(peerservsockfd, 5) != 0)
    {
        printf("error in listening server\n");
        
    }
    printf("listening.......\n");
    //cin>>temp;
    
    
    while(1){

        socklen_t nextpeerlen=sizeof(nextpeeraddr);
        peerservconnfd=accept(peerservsockfd,(struct sockaddr* )&nextpeeraddr,&nextpeerlen);
        if(peerservconnfd<0){
            printf("connection established\n");
            
        }
        //cin>>temp;
        // cout<<ntohs(nextpeeraddr.sin_port)<<"bhut"<<endl;
        // cout<<inet_ntoa(nextpeeraddr.sin_addr)<<"badiya"<<endl;

        int * pclient=(int*)malloc(sizeof(int));
        *pclient=peerservconnfd;
        pthread_t ctserver;
        pthread_create(&ctserver,NULL,nextpeerfun,pclient);

        // cout<<"second thread tracker";

    } 
    pthread_join(ct, NULL);

    return 0;
}
