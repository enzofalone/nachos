#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "elevator.h"


int nextPersonID = 1;
Lock *personIDLock = new Lock("PersonIDLock");


ELEVATOR *e;


void ELEVATOR::start() {

    while(1) {
        // A. Wait until hailed
        while (!activePersons()) {
            elevatorWaiting->Wait(elevatorLock);
        }
        // B. While there are active persons, loop doing the following
        while (activePersons()) {
            //      0. Acquire elevatorLock
            elevatorLock->Acquire();
            //      1. Signal persons inside elevator to get off (leaving->broadcast(elevatorLock))
            leaving[currentFloor]->Broadcast(elevatorLock);
            //      2. Signal persons atFloor to get in, one at a time, checking occupancyLimit each time
            while (personsWaiting[currentFloor - 1] > 0 && occupancy < maxOccupancy) {
                entering[currentFloor]->Signal(elevatorLock);
                occupancy++;
                personsWaiting[currentFloor - 1]--;
            }
            //      2.5 Release elevatorLock
            elevatorLock->Release();
            //      3. Spin for some time
            for(int j = 0 ; j < 100; j++) {
                currentThread->Yield();
            }
            //      4. Go to next floor
            currentFloor = (currentFloor % numberOfFloors) + 1;
            //  printf("Elevator arrives on floor %d", )
            printf("Elevator arrives on floor %d\n", currentFloor);

        }
    }
}

void ElevatorThread(int numberOfFloors) {

    printf("Elevator with %d floors was created!\n", numberOfFloors);

    e = new ELEVATOR(numberOfFloors);

    e->start();


}

bool ELEVATOR::activePersons() {
    for (int i = 0; i < numberOfFloors; i++) {
        if (personsWaiting[i] > 0 || occupancy > 0) {
            return true;
        }
    }
    return false;
}

ELEVATOR::ELEVATOR(int numberOfFloors) {
    this->numberOfFloors = numberOfFloors;
    currentFloor = 1;
    occupancy = 0;
    maxOccupancy = 5;
    entering = new Condition*[numberOfFloors];
    leaving = new Condition*[numberOfFloors];

    // Initialize entering
    for (int i = 0; i < numberOfFloors; i++) {
        entering[i] = new Condition("Entering " + i);
    }
    personsWaiting = new int[numberOfFloors];
    elevatorLock = new Lock("ElevatorLock");
    elevatorWaiting = new Condition("ElevatorWaiting");

    // Initialize leaving
    for (int i = 0; i < numberOfFloors; i++) {
        leaving[i] = new Condition("Leaving " + i);
        personsWaiting[i] = 0;
    }
}


void Elevator(int numberOfFloors) {
    // Create Elevator Thread
    Thread *t = new Thread("Elevator");
    t->Fork(ElevatorThread, numberOfFloors);
}


void ELEVATOR::hailElevator(Person *p) {
    // 1. Increment waiting persons atFloor
    personsWaiting[p->atFloor - 1]++;
    // 2. Hail Elevator
    elevatorWaiting->Signal(elevatorLock);
    // 2.5 Acquire elevatorLock;
    elevatorLock->Acquire();
    // 3. Wait for elevator to arrive atFloor [entering[p->atFloor]->wait(elevatorLock)]
    entering[p->atFloor]->Wait(elevatorLock);

    printf("Blahh");
    // 5. Get into elevator
    printf("Person %d got into the elevator.\n", p->id);
    // 6. Decrement persons waiting atFloor [personsWaiting[atFloor]++]
    personsWaiting[p->atFloor - 1]--;
    // 7. Increment persons inside elevator [occupancy++]
    occupancy++;
    // 8. Wait for elevator to reach toFloor [leaving[p->toFloor]->wait(elevatorLock)]
    leaving[p->toFloor]->Wait(elevatorLock);
    // 9. Get out of the elevator
    printf("Person %d got out of the elevator.\n", p->id);
    // 10. Decrement persons inside elevator
    occupancy--;
    // 11. Release elevatorLock;
    elevatorLock->Release();
}

void PersonThread(int person) {

    Person *p = (Person *)person;

    printf("Person %d wants to go from floor %d to %d\n", p->id, p->atFloor, p->toFloor);

    e->hailElevator(p);

}

int getNextPersonID() {
    int personID = nextPersonID;
    personIDLock->Acquire();
    nextPersonID = nextPersonID + 1;
    personIDLock->Release();
    return personID;
}


void ArrivingGoingFromTo(int atFloor, int toFloor) {


    // Create Person struct
    Person *p = new Person;
    p->id = getNextPersonID();
    p->atFloor = atFloor;
    p->toFloor = toFloor;

    // Creates Person Thread
    Thread *t = new Thread("Person " + p->id);
    t->Fork(PersonThread, (int)p);

}