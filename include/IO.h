#ifndef IO_H
#define IO_H

#include <queue>
#include <string>
#include <mutex>

class IORequest {
public:
    int processId;
    std::string data;
    int priority;
    int timestamp;
    
    IORequest(int pid, std::string d, int prio, int time);
    bool operator<(const IORequest& other) const;
};

class IODevice {
protected:
    std::string name;
    bool busy;
    std::priority_queue<IORequest> requestQueue;
    std::mutex mtx;
    
public:
    IODevice(std::string n);
    virtual ~IODevice() {}
    
    void addRequest(int processId, std::string data, int priority, int timestamp);
    virtual void processRequest();
    bool isBusy() { return busy; }
    int getQueueSize();
};

class Printer : public IODevice {
public:
    Printer();
    void processRequest() override;
};

class SharedBuffer : public IODevice {
private:
    static const int BUFFER_SIZE = 100;
    std::string buffer[BUFFER_SIZE];
    int count;
    
public:
    SharedBuffer();
    void write(std::string data);
    std::string read();
};

#endif
