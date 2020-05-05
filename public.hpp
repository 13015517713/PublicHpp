#pragma once

#include <set>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <vector>
#include <math.h>

/*
package read
其实没啥用
*/
int readn(int fd,char *buf,int n){
    int len=n;
    int aread=0;
    char *ptr=buf;
    while(len>0){
        int tlen=0;
        if((tlen=read(fd,ptr,len))<0) return -1;
        if(tlen==0) break;
        len-=tlen;
        aread+=tlen;
        ptr+=tlen;
    }
    return aread;
}
/*
package write
其实没啥用
*/
int writen(int fd,char *buf,int n){
    int len=n;
    int awrite=0;
    char *ptr=buf;
    while(len>0){
        int tlen=0;
        if((tlen=write(fd,ptr,len))<0){
            if(errno&EINTR){
                tlen=0; 
                continue;
            }
            else return -1;
        }
        if(tlen==0) break;
        len-=tlen;
        awrite+=tlen;
        ptr+=tlen;
    }
    return awrite;
}

/*
setSock to kill time_wait
*/
bool setSock(int fd){
    int tlen=1;
    if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&tlen,sizeof(tlen))<0){
        perror("setsocket error"); exit(-1);
    }
    return true;
}

/*
how to use select
*/
void howToUseSelect(){
    fd_set t;
    int tfd=1;
    int maxfd=tfd+1;
    FD_ZERO(&t);
    FD_SET(tfd,&t);
    timeval time{3,0};
    select(maxfd,&t,NULL,NULL,&time);
}

/*
package select
tips:for read,write,error return vector
inner:use set and vector
*/
class M_select{
public:
    M_select(){}
    void addReadFd(int &fd){
        readFdSet.insert(fd);
    }
    void addWriteFd(int &fd){
        writeFdSet.insert(fd);
    }
    void getErrorFd(int &fd){
        errorFdSet.insert(fd);
    }
    void delReadFd(int &fd){
        readFdSet.erase(fd);
    }
    void delWriteFd(int &fd){
        writeFdSet.erase(fd);
    }
    void delErrorFd(int &fd){
        errorFdSet.erase(fd);
    }
    std::vector<int> getRead(){
        canRead.clear();
        for(auto i:readFdSet){
            if(FD_ISSET(i,&readFd)) canRead.push_back(i);
        }
        return canRead;
    }
    std::vector<int> getWrite(){
        canWrite.clear();
        for(auto i:writeFdSet){
            if(FD_ISSET(i,&writeFd)) canWrite.push_back(i);
        }
        return canWrite;
    }
    std::vector<int> getError(){
        canError.clear();
        for(auto i:errorFdSet){
            if(FD_ISSET(i,&errorFd)) canError.push_back(i);
        }
        return canError;
    }
    void setTime(int &stime,int &utime){
        tim.tv_sec=stime;
        tim.tv_usec=utime;
    }
    int run(){  //最后把set的所有东西加入fd中即可
        FD_ZERO(&readFd);
        FD_ZERO(&writeFd);
        FD_ZERO(&errorFd);
        for(auto i:readFdSet) {FD_SET(i,&readFd);maxfd=std::max(maxfd,i+1);}
        for(auto i:writeFdSet) {FD_SET(i,&writeFd);maxfd=std::max(maxfd,i+1);}
        for(auto i:errorFdSet) {FD_SET(i,&errorFd);maxfd=std::max(maxfd,i+1);}
        if(tim.tv_sec==-1) return select(maxfd,&readFd,&writeFd,&errorFd,NULL);
        return select(maxfd,&readFd,&writeFd,&errorFd,&tim);
    }
private:
    timeval tim{-1,-1};
    int maxfd=0;
    fd_set readFd;
    fd_set writeFd;
    fd_set errorFd;
    std::set<int> readFdSet;
    std::set<int> writeFdSet;
    std::set<int> errorFdSet;
    std::vector<int> canRead;
    std::vector<int> canWrite;
    std::vector<int> canError;
};