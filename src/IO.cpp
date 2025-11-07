#include "../include/IO.h"
#include <iostream>
#include <thread>
#include <chrono>

IORequest::IORequest(int pid, std::string d, int prio, int time)
    : processId(pid), data(d), priority(prio), timestamp(time) {}

bool IORequest::operator<(const IORequest& other) const {
    // Mayor prioridad = menor valor numerico
    return priority > other.priority;
}

IODevice::IODevice(std::string n) : name(n), busy(false) {}

void IODevice::addRequest(int processId, std::string data, int priority, int timestamp) {
    std::lock_guard<std::mutex> lock(mtx);
    requestQueue.push(IORequest(processId, data, priority, timestamp));
}

void IODevice::processRequest() {
    while (!requestQueue.empty()) {
        mtx.lock();
        if (requestQueue.empty()) {
            mtx.unlock();
            break;
        }
        
        IORequest req = requestQueue.top();
        requestQueue.pop();
        busy = true;
        mtx.unlock();
        
        std::cout << name << " procesando solicitud de proceso " << req.processId 
                  << " (prioridad=" << req.priority << ")" << std::endl;
        
        // Simular procesamiento
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        busy = false;
    }
}

int IODevice::getQueueSize() {
    std::lock_guard<std::mutex> lock(mtx);
    return requestQueue.size();
}

// Printer Implementation
Printer::Printer() : IODevice("Impresora") {}

void Printer::processRequest() {
    while (!requestQueue.empty()) {
        mtx.lock();
        if (requestQueue.empty()) {
            mtx.unlock();
            break;
        }
        
        IORequest req = requestQueue.top();
        requestQueue.pop();
        busy = true;
        mtx.unlock();
        
        std::cout << "\n[IMPRESORA] Imprimiendo trabajo de proceso " << req.processId << std::endl;
        std::cout << "Prioridad: " << req.priority << std::endl;
        std::cout << "Datos: " << req.data << std::endl;
        
        // Simular impresion
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        std::cout << "[IMPRESORA] Trabajo completado\n" << std::endl;
        busy = false;
    }
}

// Shared Buffer Implementation
SharedBuffer::SharedBuffer() : IODevice("Buffer Compartido"), count(0) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = "";
    }
}

void SharedBuffer::write(std::string data) {
    std::lock_guard<std::mutex> lock(mtx);
    if (count < BUFFER_SIZE) {
        buffer[count++] = data;
        std::cout << "[BUFFER] Escrito: " << data << " (Ocupacion: " << count << "/" << BUFFER_SIZE << ")" << std::endl;
    } else {
        std::cout << "[BUFFER] Buffer lleno, no se puede escribir" << std::endl;
    }
}

std::string SharedBuffer::read() {
    std::lock_guard<std::mutex> lock(mtx);
    if (count > 0) {
        std::string data = buffer[--count];
        std::cout << "[BUFFER] Leido: " << data << " (Ocupacion: " << count << "/" << BUFFER_SIZE << ")" << std::endl;
        return data;
    }
    std::cout << "[BUFFER] Buffer vacio, no hay datos para leer" << std::endl;
    return "";
}
