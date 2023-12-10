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
#include<set>
#include<map>
#include<string>
#include<cmath>


using namespace std;

struct peer{
    
    string peer_id;
    string p_password;
    bool logged_in;
    string peer_ip;
    string peer_port;
    set<string> files;
    // peer(int pid,string ppass,bool login,string pip,string pport){
    //     peer_id=pid;
    //     p_password=ppass;
    //     logged_in=login;
    //     peer_ip=pip;
    //     peer_port=pport;
    //     // files=filelist;
        
    // }
    
};


map<string,peer> peer_info;

string create_peer(string uid,string pass,string ip,string port){
    string response="";
    if(peer_info.find(uid)!=peer_info.end()){
        response+="user with same id already exist";
        return response;
    }else{
        peer_info[uid].peer_id=uid;
        peer_info[uid].p_password=pass;
        
        peer_info[uid].logged_in=false;
        response="done "+uid;

        return response;
        


    }
}
string login_peer(string uid,string pass,string ip,string port){
    string response;
    if(peer_info.find(uid)==peer_info.end()){
        response="please register first";
        return response;
    }else if(peer_info[uid].logged_in==true){
        response="user is already logged in";
        return response;
    }
    else if(peer_info[uid].p_password!=pass){
        response="Password not matching";
        return response;
        
    }else if(peer_info[uid].p_password==pass){
        peer_info[uid].logged_in=true;
        peer_info[uid].peer_ip=ip;
        peer_info[uid].peer_port=port;
        response="done "+uid;
        return response;
    }
    else{
        return response;
    }
}


struct group{
    int member_count;
    set<string> members;
    string admin;
    set<string> group_files;
    set<string> g_requests;
};

map<string,group> group_details;


string creategroup(string gid,string uid){
     string response;
    if(peer_info.find(uid)==peer_info.end()){
        response="please register first";
        return response;
    }else if(peer_info[uid].logged_in==false){
        response="Plaese Log in first";
        return response;
    }else{
        group_details[gid].admin=uid;
        group_details[gid].member_count++;
        group_details[gid].members.insert(uid);
        response="Group Created succefully";
        return response;
        

    }
}
string joingroup(string gid,string uid){
     string response;
    if(peer_info.find(uid)==peer_info.end()){
        response="please register first";
        return response;
    }else if(peer_info[uid].logged_in==false){
        response="Plaese Log in first";
        return response;
    }else{
        group_details[gid].g_requests.insert(uid);
       
        response="Request Created succefully";
        return response;
        

    }
}
string leavegroup(string gid,string uid){
     string response;
    if(peer_info.find(uid)==peer_info.end()){
        response="please register first";
        return response;
    }else if(peer_info[uid].logged_in==false){
        response="Plaese Log in first";
        return response;
    }else{
        if(group_details[gid].admin==uid){
            group_details.erase(gid);
            response="You was the admin\n Now this group do not exist";
            return response;

        }else if(group_details[gid].members.find(uid)==group_details[gid].members.end()){
            response="You are not a meber of this group";
            // group_details[gid].g_requests.erase(uid);
            return response;
        }else{
           group_details[gid].members.erase(uid);
           group_details[gid].member_count--;
           //not removing files of this member
           response="Exit succefully";
            return response;

        }
        
       
        
        

    }
}

string pendingrequest(string gid,string uid){
     string response;
    if(peer_info.find(uid)==peer_info.end()){
        response="please register first";
        return response;
    }else if(peer_info[uid].logged_in==false){
        response="Plaese Log in first";
        return response;
    }else{
        response="pending requests\n";
        for(auto request:group_details[gid].g_requests){
            response+=request+"\n";

        }
        return response;
    }
}


string acceptrequest(string gid,string uid){
     string response;
    if(peer_info.find(uid)==peer_info.end()){
        response="please register first";
        return response;
    }else if(peer_info[uid].logged_in==false){
        response="Plaese Log in first";
        return response;
    }else{
         if(group_details[gid].admin==uid){
            for(string request:group_details[gid].g_requests){
             group_details[gid].members.insert(request);
             //group_details[gid].g_requests.erase(request);


            } 
            group_details[gid].g_requests.clear();
            response="Accepted all requests";
            return response;

        }else{
            response="You are not an admin";
            return response;
        }
    }
}
string listgroup(string uid){
    string response;
    response="Avialable groups\n";
        for(auto group:group_details){
            response+=group.first+"\n";

        }
       return response;
}
string listfiles(string gid,string uid){
     string response;
    if(peer_info.find(uid)==peer_info.end()){
        response="please register first";
        return response;
    }else if(peer_info[uid].logged_in==false){
        response="Plaese Log in first";
        return response;
    }else{
        response="Avialable files\n";
        for(auto file:group_details[gid].group_files){
            response+=file+"\n";

        }
        return response;
    }
}
string getfname(string filepath){
    string fname="";
    if(filepath[0]=='/'){
        for(int i=0;i<filepath.size();i++){
            if(filepath[i]=='/'){
                fname="";
            }else{
                fname+=filepath[i];
            }
        }
        return fname;

    }
    if(filepath[0]=='.'){
        int len=filepath.size();
        fname=filepath.substr(1,len-1);
        return fname;
    }
    else{
        return filepath;
    }
}







struct files{
    int files_size;
    string u_name;
    int no_of_chunks;
    string path;
    string hash;
    vector<string> hashofchunk;
};

static int user=1;
map<string,files> files_details;
map<string,vector<string>> fileuname;
// 
int chunk_size=5*1024;
string uploadfile(string filepath,string gid,string uid,int filesize){
    string response;
    if(peer_info.find(uid)==peer_info.end()){
        response="please register first";
        return response;
    }else if(peer_info[uid].logged_in==false){
        response="Plaese Log in first";
        return response;
    }else if(group_details[gid].members.find(uid)==group_details[gid].members.end()){
        response ="You are not a active member of this group";
        return response;
    }else{
        string fname=getfname(filepath);
        if(group_details[gid].group_files.find(fname)==group_details[gid].group_files.end()){
            //first time
            group_details[gid].group_files.insert(fname);
            files_details[fname].files_size=filesize;
            files_details[fname].no_of_chunks=ceil(files_details[fname].files_size/chunk_size);
            files_details[fname].u_name=uid;
            files_details[fname].path=filepath;

            fileuname[fname].push_back(uid);
            response ="done2 file Uploaded successfully "+fname;
            return response;


        }else{
            fileuname[fname].push_back(uid);
            response ="done2 file Uploaded successfully "+fname;
            return response;
        }
        

    }
    
}
string downloadfile(string gid,string filename,string despath,string uid){
    cout<<gid<<" "<<filename<<" "<<despath<<" "<<uid;
    string response;
    if(peer_info.find(uid)==peer_info.end()){
        response="please register first";
        return response;
    }else if(peer_info[uid].logged_in==false){
        response="Plaese Log in first";
        return response;
    }else if(group_details[gid].members.find(uid)==group_details[gid].members.end()){
        response ="You are not a active member of this group";
        return response;
    }else if(group_details[gid].group_files.find(filename)==group_details[gid].group_files.end()){
         response ="file does not exist";
        return response;
    }else{
        for(auto userid:fileuname[filename]){
            response+="done1 "+uid+" "+peer_info[userid].peer_ip+" "+peer_info[userid].peer_port+" ";
        }
        return response;
    }



}

vector<string> commandvector(string input){
    vector<string> res;
    string temp="";
    for(int i=0;i<input.size();i++){
        if(input[i]==' '){
            res.push_back(temp);
            temp="";


        }else{
            temp+=input[i];
        }
    }
    res.push_back(temp);
    
    return res;
} 
string logout(string uid){
    string res;
    if(peer_info[uid].logged_in==true){
        peer_info[uid].logged_in=false;
        res="Logged Out successfully";
    }else{
        res="You are not logged in";
    }
    return res;
}

void* serv_fun(void * pclient){
    int lo=0;
    int* sockfdpointer=(int*)pclient;
    int sockfd=*sockfdpointer;
    int ntr;
    char buffer[1024];
    string uid;
    // cout<<"server thread aa gya"<<endl;

    
    while(1){
        bzero(buffer,sizeof(buffer));
        
        
        string request;
        string dummy;
        string response="";
        vector<string> req_v;
        recv(sockfd,buffer,sizeof(buffer),0);
        request=string(buffer);
        if(request.size()==0){
            dummy=logout(uid);
            uid="";
            lo=1;
                        
            break;
        }
        req_v=commandvector(request);
        cout<<req_v[0]<<endl;
        //cout<<req_v[1]<<endl;
        int var=stoi(req_v[0]);
        switch(char(var+97)){
            case 'b': //cout<<"create karwana he"<<endl;
                        
                        response=create_peer(req_v[1],req_v[2],req_v[3],req_v[4]);
                            
                        
                    break;
            case 'c': //cout<<"login karwana he"<<endl;
                        uid=req_v[1];
                        response=login_peer(uid,req_v[2],req_v[3],req_v[4]);
                    break;
            case 'd': //cout<<"create group karwana he"<<endl;
                        response=creategroup(req_v[1],uid);
                    break;

            case 'e': //cout<<"join group karwana he"<<endl;
                        response=joingroup(req_v[1],uid);
                    break;
            case 'f': //cout<<"Leave group karwana he"<<endl;
                        response=leavegroup(req_v[1],uid);
                    break;
            case 'g': //cout<<"list pending in group karwana he"<<endl;
                    response=pendingrequest(req_v[1],req_v[2]);
                    break;
            case 'h': //cout<<"accept request karwana he"<<endl;
                    response=acceptrequest(req_v[1],uid);
                    break;
            case 'i': //cout<<"list all group karwana he"<<endl;
                    response=listgroup(uid);
                    break;
            case 'j': //cout<<"list all files karwana he"<<endl;
                    response=listfiles(req_v[1],uid);
                    break;
            case 'k': //cout<<"upload files karwana he"<<endl;
                    response=uploadfile(req_v[1],req_v[2],uid,stoi(req_v[4]));
                    break;
            case 'l': //cout<<"download files karwana he"<<endl;
                    response=downloadfile(req_v[1],req_v[2],req_v[3],uid);
                    break;
            case 'm': cout<<"logout karwana he"<<endl;


                        response=logout(uid);
                        uid="";
                        lo=1;
                        
                    break;
            default://cout<<"pta nhi kya aa gya"<<endl;
                    break;
        

        }








        cout<<"From peer to tracker : "<<request<<endl;
        if(lo){
            break;
        }
        
        char buffer1[1024];
        bzero(buffer1,sizeof(buffer1));
        strcpy(buffer1,response.c_str());
        
        
        send(sockfd,buffer1,sizeof(buffer1),0);
        // bzero(buffer,sizeof(buffer));



        
    }
    return NULL;
    // cout<<"loop end"<<endl;
}

int main(int argc,char** argv){
    if(argc!=5){
        printf("Invalid Number of arguments");
        return 0;
    }

    // peer_info["dish"].peer_id="dish";
    // peer_info["dish"].p_password="12";
    // peer_info["dish"].logged_in=false;

    // peer_info["v"].peer_id="v";
    // peer_info["v"].p_password="21";
    // peer_info["v"].logged_in=false;

    // group_details["pg1"].admin="dish";
    // group_details["pg1"].member_count=2;
    // group_details["pg1"].members.insert("dish");
    // group_details["pg1"].members.insert("v");
    

   


    int sockfd,connfd;
    struct sockaddr_in peeraddr;
    struct sockaddr_in trackeraddr;
    char* ip=argv[3];
    int port=atoi(argv[4]);
    int opt=1;

    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        printf("Peer socket not created\n");
        return EXIT_FAILURE;
    }
    printf("Peer socket created\n");
    cout<<sockfd<<endl;

    trackeraddr.sin_family = AF_INET;
    trackeraddr.sin_addr.s_addr = inet_addr(ip);
    trackeraddr.sin_port = htons(port);

     signal(SIGPIPE,SIG_IGN);
    if(setsockopt(sockfd,SOL_SOCKET,(SO_REUSEPORT|SO_REUSEADDR),(char*)&opt,sizeof(opt))<0){
        printf("Error Setsocopt\n");
       
        return EXIT_FAILURE;
    }

    if (bind(sockfd, (struct sockaddr *)&trackeraddr, sizeof(trackeraddr)) != 0)
    {
        printf("Error in binding server\n");
       
        return EXIT_FAILURE;
    }
    printf("Binding done\n");
    if (listen(sockfd, 5) != 0)
    {
        printf("error in listening server\n");
        
    }
    printf("listening.......\n");

     //socklen_t cli_size=sizeof(peeraddr);
      while(1){
        socklen_t peerlen=sizeof(peeraddr);
        // cout<<"before accept nnnnnnnnn"<<endl;
        cout<<connfd<<endl;
        connfd=accept(sockfd,(struct sockaddr* )&peeraddr,&peerlen);
        cout<<connfd<<endl;
        if(connfd<0){
            printf("connection failled\n");
            
        }else{
            cout<<"connection established"<<endl;
        }
        // cout<<ntohs(peeraddr.sin_port)<<endl;
        // cout<<inet_ntoa(peeraddr.sin_addr)<<endl;

        int * pclient=(int*)malloc(sizeof(int));
        *pclient=connfd;
        pthread_t ct;
        pthread_create(&ct,NULL,serv_fun,pclient);

        // cout<<"second thread tracker";

      } 
      close(sockfd);
      return 0; 
    
}