// ejc

#ifndef Dispatch_hh
#define Dispatch_hh

#include <vector>
#include <H5Cpp.h>
#include <Buffer.hh>
#include <Digitizer.hh>
#include <Dispatcher.hh>
#include <RunType.hh>
//#include <Decode.hh>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

using namespace std;
using namespace H5;

extern bool stop;
bool dispatch_running;

void int_handler(int x) {
    if (stop) exit(1);
    stop = true;
}

typedef struct {
    vector<Buffer*> *buffers;
    Dispatcher* dispatcher;
    pthread_mutex_t *iomutex;
    pthread_cond_t *newdata;
    string config;
    RunType *runtype;
} dispatch_thread_data;

void *dispatch_thread(void *_data) {
    signal(SIGINT,int_handler);
    dispatch_thread_data* data = (dispatch_thread_data*)_data;

    /* FIXME
    string path = "/home/eos/ejc/socket/socket";
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    char _path[256];
    sprintf(_path, "%s", path.c_str());
    struct sockaddr_un saddr;
    saddr.sun_family = AF_UNIX;
    sprintf(saddr.sun_path, "%s", path.c_str());
    if (connect(sockfd, (struct sockaddr*) &saddr, sizeof(saddr)) < 0){
         cerr << "error during socket connection";
         pthread_exit(NULL);
    }
    */

    vector<size_t> evtsReady(data->buffers->size());
    data->runtype->begin();
    try {
        dispatch_running = true;
        while (dispatch_running) {
            pthread_mutex_lock(data->iomutex);
            for (;;) {
                bool found = stop;
                for (size_t i = 0; i < data->buffers->size(); i++) {
                    found |= (*data->buffers)[i]->fill() > 0;
                }
                if (found) break;
                pthread_cond_wait(data->newdata,data->iomutex);
            }
            
            size_t total = data->dispatcher->Digest(*data->buffers);
            if (stop && total == 0) {
                dispatch_running = false;
            } else if (stop || data->dispatcher->Ready()) {
                Exception::dontPrint();

                string path = data->dispatcher->NextPath();
                cout << "Streaming data to " << path << endl;
                data->dispatcher->Dispatch(*data->buffers);
                dispatch_running = data->runtype->keepgoing();
            }
            pthread_mutex_unlock(data->iomutex);
        }
        stop = true;
    } catch (runtime_error &e) {
        pthread_mutex_unlock(data->iomutex);
        stop = true;
        pthread_mutex_lock(data->iomutex);
        cout << "Dispatch thread aborted: " << e.what() << endl;
        pthread_mutex_unlock(data->iomutex);
    }
    pthread_exit(NULL);
}

#endif

