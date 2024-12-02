#include "pcbmanager.h"

PCBManager::PCBManager(int maxProcesses) {

    bitmap = new BitMap(maxProcesses);
    pcbs = new PCB*[maxProcesses];
    pcbManagerLock = new Lock("pcbManagerLock");

    for(int i = 0; i < maxProcesses; i++) {
        pcbs[i] = NULL;
    }
}


PCBManager::~PCBManager() {
    delete bitmap;
    delete pcbs;
    delete pcbManagerLock;
}


PCB* PCBManager::AllocatePCB() {

    // Aquire pcbManagerLock
    pcbManagerLock->Acquire();

    int pid = bitmap->Find();

    // Release pcbManagerLock

    ASSERT(pid != -1);

    pcbs[pid] = new PCB(pid);
    pcbManagerLock->Release();

    return pcbs[pid];

}


int PCBManager::DeallocatePCB(PCB* pcb) {

    // Acquire pcbManagerLock
    pcbManagerLock->Acquire();

    // Check is pcb is valid -- check pcbs for pcb->pid
    if (GetPCB(pcb->pid) == NULL) {
        return -1;
    }

    bitmap->Clear(pcb->pid);

    int pid = pcb->pid;

    delete pcbs[pid];

    pcbs[pid] = NULL;

    // Release pcbManagerLock
    pcbManagerLock->Release();

    return 0;
}

PCB* PCBManager::GetPCB(int pid) {
    return pcbs[pid];
}