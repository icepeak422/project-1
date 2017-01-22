#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "thread.h"
#include "cpu.h"
#include "cv.h"
#include "mutex.h"

using namespace std;
mutex mutex1;
cv requestCV;
cv serverCV;
int track=0;
int head=0;
int max_disk_queue=0;
int requester=0;

struct disk
 {
     int disknum;
     vector<int> tracklist;
 }; 
vector<disk> disklist;
vector<disk> requestlist;

 void request(int i)
{ 
    disk d;
    d=disklist.at(i);
    mutex1.lock();
    while(requestlist.size()>=max_disk_queue){
        requestCV.wait(mutex1);
    }
    requestlist.push_back(d);
    requester=d.disknum;
    track=d.tracklist.back();
    cout << "requester " << requester << " track " << track << endl;
    d.tracklist.pop_back();
    if (d.tracklist.size()==0){ 
        for(int i=0;i<disklist.size();i++){
            if(disklist.at(i)==d){
                disklist.erase(i);
            }   
            break; 
        } 
    }
    mutex1.lock();
}


void server(void *a)
{
    vector<disk> *disklist = (vector<disk> *) a;

    for(int i=0;i<disklist.size();i++){
       thread t1 ((thread_startfunc_t) request, (int) i); 
    }

    mutex1.lock();
    //seek the cloest postion
    while(requestlist.size()<max_disk_queue && disklist.size()!=0){
         serverCV.wait(mutex1);
    }
    int temp,dif=0;
    temp=abs(((requestlist.at(0)).track)-head);
    for(int i=0;i<requestlist.size();i++){
        dif=abs(((requestlist.at(i)).track-head));   
        if(dif<temp){
            temp=dif;
            track=(requestlist.at(i)).disknum;
            requester=(requestlist.at(i)).track;
        }
    }
    head= track;
    //serve the closest track
    cout << "service requester " << requester << " track " << head << endl;
    requestCV.signal();
    mutex1.unlock();
}


int main(int argc,char *argv[])
{   
    string lineinfile;
    max_disk_queue= stoi(argv[1]);
    for (int n=0;n<argc-2;n++){
        disk d;
        d.disknum=n;
        string fname(argv[n+2]);
        fstream inpf;
        inpf.open(fname);
        while(!inpf.eof()){
            getline(inpf,lineinfile);
            track=stoi(lineinfile);
            d.tracklist.push_back(track);
        }
        reverse(d.tracklist.begin(),d.tracklist.end());
        disklist.push_back(d);
    };
    if (disklist.size()<max_disk_queue){
        max_disk_queue = disklist.size();
    };
    cpu::boot((thread_startfunc_t) server, (void *) 100, 0);
}
