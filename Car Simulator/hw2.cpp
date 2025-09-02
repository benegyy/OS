
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <ctime>
#include <queue>
#include <string>
#include <ctime>
#include <chrono>
#include <set>
#include <iostream>
#include <vector>
#include <sys/types.h> // Include for timer_t
#include <sys/time.h> // Include for timer_t
#include <ctime> // Include for time functions
#include <pthread.h>
#include <unistd.h>
#include <ctime>
#include <ctime>
#include "helper.h"
#include "WriteOutput.h"
using namespace std;


enum ConnectorType { NARROW_BRIDGE, FERRY, CROSSROAD };
enum Direction { DIRECTION_0_TO_1, DIRECTION_1_TO_0 ,DIRECTION_0_TO_2,
    DIRECTION_0_TO_3,DIRECTION_1_TO_2,DIRECTION_1_TO_3,DIRECTION_2_TO_3,
    DIRECTION_3_TO_0,DIRECTION_3_TO_1,DIRECTION_3_TO_2};
int numNarrowBridges, numFerries, numCrossroads, numCars;

class Timer {
private:
    std::chrono::steady_clock::time_point start_time;
public:
    Timer() {
        reset();
    }
    void reset() {
        start_time = std::chrono::steady_clock::now();
    }
    bool hasReachedDuration(std::chrono::milliseconds duration) const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
        return elapsed >= duration;
    }
};
struct PathObject {
    PathObject() : connectorType("basicString"), fromDirection(0), toDirection(0) {}
    string connectorType;
    int fromDirection;
    int toDirection;
};

class Car{
public:
    int id;
    int travel_time;
    int pathLength;
    vector<PathObject> pathObjects;
    bool same_car=false;
    pthread_t timerferryc;

};
vector<Car> cars;

class Narrow_bridge{
public:
    int id;
    int travel_time;
    int max_wait_limit;
    int numCarsPassing;
    int current_direction;
    int carpassing1;
    int carpassing0;
    pthread_mutex_t mutex;
    pthread_mutex_t mutex1;
    pthread_mutex_t mutex2;
    pthread_mutex_t mutex00;
    pthread_mutex_t mutex11;
    pthread_mutex_t mutex000;
    pthread_mutex_t mutex111;
    pthread_cond_t cond0;
    pthread_t timernarrow;
    pthread_cond_t condx;
    pthread_t timerferryc;
    pthread_cond_t cond1;
    int waitingCars[2];
    int passingcars[2];
    bool carhaspassed;
    int car_is_passing;
    bool direction_changed;

    Narrow_bridge() : id(id), travel_time(travel_time), max_wait_limit(max_wait_limit) {
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_init(&mutex2, NULL);

        pthread_mutex_init(&mutex1, NULL);

        pthread_mutex_init(&mutex00, NULL);
        pthread_mutex_init(&mutex11, NULL);

        pthread_mutex_init(&mutex000, NULL);
        pthread_mutex_init(&mutex111, NULL);

        pthread_cond_init(&cond0, NULL);
        pthread_cond_init(&condx, NULL);

        pthread_cond_init(&cond1, NULL);
        numCarsPassing = 0;
        current_direction = -1;
        waitingCars[0]=0;
        waitingCars[1]=0;
        passingcars[0]=0;
        passingcars[1]=0;
        carhaspassed = false;
        car_is_passing= 0;
        direction_changed= false;
        carpassing1= 0;
        carpassing0= 0;

    }
    Narrow_bridge(int id, int travel_time, int max_wait_limit) : id(id), travel_time(travel_time), max_wait_limit(max_wait_limit) {
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_init(&mutex1, NULL);
        pthread_mutex_init(&mutex2, NULL);
        pthread_mutex_init(&mutex00, NULL);
        pthread_mutex_init(&mutex11, NULL);

        pthread_mutex_init(&mutex000, NULL);
        pthread_mutex_init(&mutex111, NULL);

        pthread_cond_init(&condx, NULL);

        pthread_cond_init(&cond0, NULL);
        pthread_cond_init(&cond1, NULL);
        numCarsPassing = 0;
        current_direction = -1;
        waitingCars[0]=0;
        waitingCars[1]=0;
        passingcars[0]=0;
        passingcars[1]=0;
        carhaspassed = false;
        car_is_passing= 0;
        direction_changed= false;
        carpassing1= 0;
        carpassing0= 0;
    }
    ~Narrow_bridge() {
        pthread_mutex_destroy(&mutex);
        pthread_mutex_destroy(&mutex1);
        pthread_mutex_destroy(&mutex2);

        pthread_mutex_destroy(&mutex00);
        pthread_mutex_destroy(&mutex11);

        pthread_mutex_destroy(&mutex000);
        pthread_mutex_destroy(&mutex111);

        pthread_cond_destroy(&cond0);
        pthread_cond_destroy(&condx);

        pthread_cond_destroy(&cond1);
    }
} ;
vector<Narrow_bridge> narrowBridges;

class FerryMonitor {
public:
    int id;
    int travel_time;
    int max_wait_limit;
    int capacity;
    pthread_mutex_t mutex;

    pthread_mutex_t mutex1;
    pthread_cond_t cond0;
    pthread_cond_t cond1;
    pthread_t timerferryc;
    int num_carss0;
    int num_carss1;
    bool ferry_first;
    bool timerExpired;
    bool ferry_leaving;
    bool ferry_leaving1;
    bool reset_timer;
    FerryMonitor(): id(id), travel_time(travel_time), max_wait_limit(max_wait_limit) , capacity(capacity){
        num_carss0 = 0;
        num_carss1 = 0;
        ferry_first = true;
        ferry_leaving = false;
        ferry_leaving1 = false;
        reset_timer=false;
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_init(&mutex1, NULL);

        pthread_cond_init(&cond0, NULL);
        pthread_cond_init(&cond1, NULL);
    }
    FerryMonitor(int _id, int _travel_time,  int _max_wait_limit,int _capacity) : capacity(_capacity), travel_time(_travel_time),max_wait_limit(_max_wait_limit) {
        num_carss0 = 0;
        num_carss1 = 0;
        ferry_first = true;
        ferry_leaving = false;
        ferry_leaving1 = false;
        reset_timer=false;
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_init(&mutex1, NULL);

        pthread_cond_init(&cond1, NULL);
        pthread_cond_init(&cond0, NULL);
    }
    ~FerryMonitor() {
        pthread_mutex_destroy(&mutex);


        pthread_mutex_destroy(&mutex1);
        pthread_cond_destroy(&cond0);
        pthread_cond_destroy(&cond1);
    }
    int getMaxWaitLimit() const {
        return max_wait_limit;
    }

};
vector<FerryMonitor> ferries;
pthread_t timerThreads[50];
pthread_t timerferry;
class Cross_road {
public:
    int id;
    int travel_time;
    int max_wait_limit;
    pthread_mutex_t mutex;

    pthread_mutex_t mutex0;
    pthread_mutex_t mutex1;
    pthread_mutex_t mutex2;
    pthread_mutex_t mutex3;
    pthread_mutex_t mutex00;
    pthread_mutex_t mutex11;
    pthread_mutex_t mutex22;
    pthread_mutex_t mutex33;
    pthread_mutex_t mutex000;
    pthread_mutex_t mutex111;
    pthread_mutex_t mutex222;
    pthread_mutex_t mutex333;
    pthread_cond_t cond0;
    pthread_cond_t cond1;
    pthread_cond_t cond2;
    pthread_cond_t cond3;

    bool carhaspassed0;
    bool carhaspassed1;
    bool carhaspassed2;
    bool carhaspassed3;
    int car_is_passing0;
    int car_is_passing1;
    int car_is_passing2;
    int car_is_passing3;
    bool direction_changed;
    int current_direction;
    int waitingCars[4];


    Cross_road()
            : id(id), travel_time(travel_time), max_wait_limit(max_wait_limit) {
        pthread_mutex_init(&mutex, NULL);

        pthread_mutex_init(&mutex0, NULL);
        pthread_mutex_init(&mutex1, NULL);
        pthread_mutex_init(&mutex2, NULL);
        pthread_mutex_init(&mutex3, NULL);

        pthread_mutex_init(&mutex00, NULL);
        pthread_mutex_init(&mutex11, NULL);
        pthread_mutex_init(&mutex22, NULL);
        pthread_mutex_init(&mutex33, NULL);

        pthread_mutex_init(&mutex000, NULL);
        pthread_mutex_init(&mutex111, NULL);
        pthread_mutex_init(&mutex222, NULL);
        pthread_mutex_init(&mutex333, NULL);

        pthread_cond_init(&cond0, NULL);
        pthread_cond_init(&cond1, NULL);
        pthread_cond_init(&cond2, NULL);
        pthread_cond_init(&cond3, NULL);
        carhaspassed0 = false;
        carhaspassed1 = false;
        carhaspassed2 = false;
        carhaspassed3 = false;
        car_is_passing0 = 0;
        car_is_passing1 = 0;
        car_is_passing2 = 0;
        car_is_passing3 = 0;
        direction_changed = false;
        current_direction = -1;
        waitingCars[0] = 0;
        waitingCars[1] = 0;
        waitingCars[2] = 0;
        waitingCars[3] = 0;
    }

    Cross_road(int _id, int _travel_time, int _max_wait_limit)
            : id(_id), travel_time(_travel_time), max_wait_limit(_max_wait_limit) {
        pthread_mutex_init(&mutex, NULL);

        pthread_mutex_init(&mutex0, NULL);
        pthread_mutex_init(&mutex1, NULL);
        pthread_mutex_init(&mutex2, NULL);
        pthread_mutex_init(&mutex3, NULL);

        pthread_mutex_init(&mutex00, NULL);
        pthread_mutex_init(&mutex11, NULL);
        pthread_mutex_init(&mutex22, NULL);
        pthread_mutex_init(&mutex33, NULL);

        pthread_mutex_init(&mutex000, NULL);
        pthread_mutex_init(&mutex111, NULL);
        pthread_mutex_init(&mutex222, NULL);
        pthread_mutex_init(&mutex333, NULL);
        pthread_cond_init(&cond0, NULL);
        pthread_cond_init(&cond1, NULL);
        pthread_cond_init(&cond2, NULL);
        pthread_cond_init(&cond3, NULL);
        carhaspassed0 = false;
        carhaspassed1 = false;
        carhaspassed2 = false;
        carhaspassed3 = false;
        car_is_passing0 = 0;
        car_is_passing1 = 0;
        car_is_passing2 = 0;
        car_is_passing3 = 0;
        direction_changed = false;
        current_direction = -1;
        waitingCars[0] = 0;
        waitingCars[1] = 0;
        waitingCars[2] = 0;
        waitingCars[3] = 0;
    }

    ~Cross_road() {
        pthread_mutex_destroy(&mutex);

        pthread_mutex_destroy(&mutex0);
        pthread_mutex_destroy(&mutex1);
        pthread_mutex_destroy(&mutex2);
        pthread_mutex_destroy(&mutex3);

        pthread_mutex_destroy(&mutex00);
        pthread_mutex_destroy(&mutex11);
        pthread_mutex_destroy(&mutex22);
        pthread_mutex_destroy(&mutex33);

        pthread_mutex_destroy(&mutex000);
        pthread_mutex_destroy(&mutex111);
        pthread_mutex_destroy(&mutex222);
        pthread_mutex_destroy(&mutex333);


        pthread_cond_destroy(&cond0);
        pthread_cond_destroy(&cond1);
        pthread_cond_destroy(&cond2);
        pthread_cond_destroy(&cond3);
    }

    pthread_cond_t selectcond(int i) {
        switch (i) {
            case 0:
                return cond0;
                break;
            case 1:
                return cond1;
                break;
            case 2:
                return cond2;
                break;
            case 3:
                return cond3;
                break;

        }

    }

    bool get_car_has_passed(int i) {
        switch (i) {
            case 0:
                return carhaspassed0;
                break;
            case 1:
                return carhaspassed1;
                break;
            case 2:
                return carhaspassed2;
                break;
            case 3:
                return carhaspassed3;
                break;

        }
    };

    void set_car_has_passed(int i) {
        switch (i) {
            case 0:
                carhaspassed0 = true;
                break;
            case 1:
                carhaspassed1 = true;
                break;
            case 2:
                carhaspassed2 = true;
                break;
            case 3:
                carhaspassed3 = true;
                break;

        }
    };

    void clear_car_has_passed(int i) {
        switch (i) {
            case 0:
                carhaspassed0 = false;
                break;
            case 1:
                carhaspassed1 = false;
                break;
            case 2:
                carhaspassed2 = false;
                break;
            case 3:
                carhaspassed3 = false;
                break;

        }
    };

    int get_car_is_passing(int i) {
        switch (i) {
            case 0:
                return car_is_passing0;
                break;
            case 1:
                return car_is_passing1;
                break;
            case 2:
                return car_is_passing2;
                break;
            case 3:
                return car_is_passing3;
                break;
           default:
           	return 0;
           	break;

        }

    };

    void set_car_is_passing(int i) {
        switch (i) {
            case 0:
                car_is_passing0++;
                break;
            case 1:
                car_is_passing1++;
                break;
            case 2:
                car_is_passing2++;
                break;
            case 3:
                car_is_passing3++;
                break;

        }
    };

    void clear_car_is_passing(int i) {
        switch (i) {
            case 0:
                car_is_passing0--;
                break;
            case 1:
                car_is_passing1--;
                break;
            case 2:
                car_is_passing2--;
                break;
            case 3:
                car_is_passing3--;
                break;

        }
    };
};
vector<Cross_road> crossroads;


int findNextNonzeroIndex(int a[], int currentIndex, int size) {
    //cout << "current from direction " << currentIndex<< endl;
    //cout << "num of crossroads " << size  << endl;
    for (int i = currentIndex + 1; i < size; ++i) {
        //cout << "waiting from " << i << " is " <<a[i] << endl;
        if (a[i] != 0) {
            return i;
        }
    }
    for (int i = 0; i < currentIndex; ++i) {
        //cout << "waiting from " << i << " is " <<a[i] << endl;
        if (a[i] != 0) {
            return i;
        }
    }
    return -1; // If no nonzero element found
}



void narrowBridgeWaitRoutine(Car &car, Narrow_bridge &narrowBridge, int fromDirection, int toDirection) {
    if(fromDirection ==0){
        pthread_mutex_lock(&narrowBridge.mutex);

        if (!car.same_car) {
            narrowBridge.waitingCars[0]++;
        }
        if (narrowBridge.current_direction == -1) {
            narrowBridge.current_direction = 0;
            narrowBridge.carhaspassed = false;
        }
        pthread_mutex_unlock(&narrowBridge.mutex);

        if (narrowBridge.current_direction == 0) {
            //while(narrowBridge.carpassing1>0);

            if (!narrowBridge.carhaspassed) {
              //  cout << "pas no delay " <<car.id<< endl;

                pthread_mutex_lock(&narrowBridge.mutex);
                narrowBridge.carhaspassed = true;
                narrowBridge.car_is_passing++;
                narrowBridge.carpassing0++;
                pthread_mutex_unlock(&narrowBridge.mutex);

                WriteOutput(car.id, 'N', narrowBridge.id, START_PASSING);
                pthread_cond_signal(&narrowBridge.cond0);
                sleep_milli(narrowBridge.travel_time);
                WriteOutput(car.id, 'N', narrowBridge.id, FINISH_PASSING);

                pthread_mutex_lock(&narrowBridge.mutex);
                narrowBridge.waitingCars[0]--;
                narrowBridge.car_is_passing--;
                car.same_car = false;
                narrowBridge.carpassing0--;
                pthread_mutex_unlock(&narrowBridge.mutex);

                if(narrowBridge.waitingCars[fromDirection] <=0 ){
                    int next_direction = findNextNonzeroIndex(narrowBridge.waitingCars, 0, 2);
                    if (next_direction == 0) {
                      //  cout << "daaa " <<car.id<< endl;
                        pthread_mutex_lock(&narrowBridge.mutex);
                        narrowBridge.current_direction = 0;
                        narrowBridge.carhaspassed=false;
                        pthread_mutex_unlock(&narrowBridge.mutex);
                        pthread_cond_signal(&narrowBridge.cond0);
                        return;
                    } else if(next_direction ==1) {
                        pthread_mutex_lock(&narrowBridge.mutex);
                        narrowBridge.current_direction = 1;
                        narrowBridge.carhaspassed=false;
                        pthread_mutex_unlock(&narrowBridge.mutex);
                      //  cout << "aaaaaaaaaaaaaaaaa " <<car.id<< endl;
                        pthread_cond_signal(&narrowBridge.cond1);
                        return;
                    }
                    else{
                      //  cout << "ada " <<car.id<< endl;
                        pthread_mutex_lock(&narrowBridge.mutex);
                        narrowBridge.carhaspassed=0;
                        narrowBridge.current_direction = -1;
                        pthread_mutex_unlock(&narrowBridge.mutex);
                        return;
                    }
                }
                return;

            }
            else
            {
                pthread_mutex_lock(&narrowBridge.mutex);
                narrowBridge.car_is_passing++;

                narrowBridge.carpassing0++;
                pthread_mutex_unlock(&narrowBridge.mutex);


                pthread_mutex_lock(&narrowBridge.mutex2);
            //    cout << "pass delay " <<car.id << endl;
                sleep_milli(PASS_DELAY);
                pthread_mutex_unlock(&narrowBridge.mutex2);

                WriteOutput(car.id, 'N', narrowBridge.id, START_PASSING);

                pthread_cond_signal(&narrowBridge.cond0);


                sleep_milli(narrowBridge.travel_time);
                WriteOutput(car.id, 'N', narrowBridge.id, FINISH_PASSING);
                pthread_mutex_lock(&narrowBridge.mutex);
                narrowBridge.waitingCars[0]--;
                narrowBridge.car_is_passing--;
                car.same_car = false;
                narrowBridge.carpassing0--;
                pthread_mutex_unlock(&narrowBridge.mutex);

                if(narrowBridge.waitingCars[0] <=0 ){
                    int next_direction = findNextNonzeroIndex(narrowBridge.waitingCars, 0, 2);
                    if (next_direction == 0) {
                        pthread_mutex_lock(&narrowBridge.mutex);
                        narrowBridge.current_direction = 0;
                        narrowBridge.carhaspassed=0;
                        pthread_mutex_unlock(&narrowBridge.mutex);
                        pthread_cond_signal(&narrowBridge.cond0);
                        return;
                    } else if(next_direction ==1) {
                        pthread_mutex_lock(&narrowBridge.mutex);
                        narrowBridge.current_direction = 1;
                        narrowBridge.carhaspassed=0;
                        pthread_mutex_unlock(&narrowBridge.mutex);
                    //    cout << "bbbbbbbbbbbbbbbb " <<car.id<< endl;
                        pthread_cond_signal(&narrowBridge.cond1);
                        return;
                    }
                    else{
                        pthread_mutex_lock(&narrowBridge.mutex);
                        narrowBridge.carhaspassed=0;
                        narrowBridge.current_direction = -1;
                        pthread_mutex_unlock(&narrowBridge.mutex);
                        return;
                    }
                }
                return;

            }


        }


        else if ((narrowBridge.waitingCars[narrowBridge.current_direction] <= 0) && (narrowBridge.car_is_passing ==0) ) {
            // cout <<"no waiting at direction" << car.id <<endl;


            pthread_mutex_lock(&narrowBridge.mutex);
            car.same_car = true;
            narrowBridge.current_direction = fromDirection;
            pthread_mutex_unlock(&narrowBridge.mutex);
            pthread_cond_signal(&narrowBridge.cond0);
            narrowBridgeWaitRoutine(car, narrowBridge, fromDirection, toDirection);
            return;
        } else {
            if(narrowBridge.waitingCars[0] == 1){
            //  cout    << "timer0 : " << car.id<< endl;
                struct timeval tv;
                struct timespec ts;
                gettimeofday(&tv, NULL);

                ts.tv_sec = tv.tv_sec + narrowBridge.max_wait_limit / 1000;
                ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (narrowBridge.max_wait_limit % 1000);
                ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
                ts.tv_nsec %= (1000 * 1000 * 1000);
                pthread_mutex_lock(&narrowBridge.mutex);
                int timed_wait_result = pthread_cond_timedwait(&narrowBridge.cond0,&narrowBridge.mutex, &ts);

                pthread_mutex_unlock(&narrowBridge.mutex);


                if (timed_wait_result == ETIMEDOUT) {
                //    cout << "timeoutd  "<<car.id<<endl;

                    pthread_mutex_lock(&narrowBridge.mutex);
                    car.same_car = true;
                    narrowBridge.current_direction = 0;
                    narrowBridge.carhaspassed=false;
                    pthread_mutex_unlock(&narrowBridge.mutex);
                    //while(narrowBridge.carpassing1>0);
                    while(narrowBridge.carpassing1>0);
                    pthread_cond_signal(&narrowBridge.cond0);

                    pthread_mutex_unlock(&narrowBridge.mutex00);
                    narrowBridgeWaitRoutine(car, narrowBridge, fromDirection, toDirection);
                    return;
                }
                else {
                  //  cout << "signaled0  "<<car.id<<endl;


                    pthread_mutex_lock(&narrowBridge.mutex);
                    car.same_car = true;
                    pthread_mutex_unlock(&narrowBridge.mutex);
                    pthread_cond_signal(&narrowBridge.cond0);
                    pthread_mutex_unlock(&narrowBridge.mutex00);
                    narrowBridgeWaitRoutine(car, narrowBridge, fromDirection, toDirection);

                    return;
                }
            }
            else{
                // cout << "wait0  "<< car.id<<endl;
                pthread_mutex_lock(&narrowBridge.mutex);
                pthread_cond_wait(&narrowBridge.cond0, &narrowBridge.mutex);

                car.same_car = true;
                pthread_mutex_unlock(&narrowBridge.mutex);
                pthread_cond_signal(&narrowBridge.cond0);
                narrowBridgeWaitRoutine(car, narrowBridge, fromDirection, toDirection);
                return;
            }
        }


    }

    else{

        pthread_mutex_lock(&narrowBridge.mutex1);

        if (!car.same_car) {
            narrowBridge.waitingCars[1]++;
        }
        if (narrowBridge.current_direction == -1) {
            narrowBridge.current_direction = 1;
            narrowBridge.carhaspassed = false;
        }
        pthread_mutex_unlock(&narrowBridge.mutex1);
        if (narrowBridge.current_direction == 1) {
          //  cout << " carpassing0 " << narrowBridge.carpassing0 << endl;
            while(narrowBridge.carpassing0>0);
            if (!narrowBridge.carhaspassed) {
              //  cout << "pas no delayy " <<car.id<< endl;
                pthread_mutex_lock(&narrowBridge.mutex1);
                narrowBridge.carhaspassed = true;
                narrowBridge.car_is_passing++;
                narrowBridge.carpassing1++;
                pthread_mutex_unlock(&narrowBridge.mutex1);

                WriteOutput(car.id, 'N', narrowBridge.id, START_PASSING);
                pthread_cond_signal(&narrowBridge.cond1);
            //    cout << "c " <<car.id<< endl;
                sleep_milli(narrowBridge.travel_time);
                WriteOutput(car.id, 'N', narrowBridge.id, FINISH_PASSING);

                pthread_mutex_lock(&narrowBridge.mutex1);
                narrowBridge.waitingCars[1]--;
                narrowBridge.car_is_passing--;
                car.same_car = false;
                narrowBridge.carpassing1--;
                pthread_mutex_unlock(&narrowBridge.mutex1);

                if(narrowBridge.waitingCars[fromDirection] <=0 ){
                    int next_direction = findNextNonzeroIndex(narrowBridge.waitingCars, 1, 2);
                    if (next_direction == 0) {
                        pthread_mutex_lock(&narrowBridge.mutex1);
                        narrowBridge.current_direction = 0;
                        narrowBridge.carhaspassed=0;
                        pthread_mutex_unlock(&narrowBridge.mutex1);
                        pthread_cond_signal(&narrowBridge.cond0);
                        return;
                    } else if(next_direction ==1) {
                        pthread_mutex_lock(&narrowBridge.mutex1);
                        narrowBridge.current_direction = 1;
                        narrowBridge.carhaspassed=0;
                        pthread_mutex_unlock(&narrowBridge.mutex1);
                  //      cout << "ddddddddd " <<car.id<< endl;
                        pthread_cond_signal(&narrowBridge.cond1);
                        return;
                    }
                    else{
                        pthread_mutex_lock(&narrowBridge.mutex1);
                        narrowBridge.carhaspassed=0;
                        narrowBridge.current_direction = -1;
                        pthread_mutex_unlock(&narrowBridge.mutex1);
                        return;
                    }
                }
                return;

            }

            else
            {
                pthread_mutex_lock(&narrowBridge.mutex1);
                narrowBridge.car_is_passing++;
                narrowBridge.carpassing1++;
                pthread_mutex_unlock(&narrowBridge.mutex1);

                pthread_mutex_lock(&narrowBridge.mutex2);
                //cout << "pass delayy " <<car.id << endl;
                sleep_milli(PASS_DELAY);
                pthread_mutex_unlock(&narrowBridge.mutex2);

                WriteOutput(car.id, 'N', narrowBridge.id, START_PASSING);
                //cout << "eeeeeeee " <<car.id<< endl;
                pthread_cond_signal(&narrowBridge.cond1);


                sleep_milli(narrowBridge.travel_time);
                WriteOutput(car.id, 'N', narrowBridge.id, FINISH_PASSING);
                pthread_mutex_lock(&narrowBridge.mutex1);
                narrowBridge.waitingCars[1]--;
                narrowBridge.car_is_passing--;
                narrowBridge.carpassing1--;
                car.same_car = false;
                pthread_mutex_unlock(&narrowBridge.mutex1);

                if(narrowBridge.waitingCars[1] <=0 ){
                    int next_direction = findNextNonzeroIndex(narrowBridge.waitingCars, 1, 2);
                    if (next_direction == 0) {
                        pthread_mutex_lock(&narrowBridge.mutex1);
                        narrowBridge.current_direction = 0;
                        narrowBridge.carhaspassed=0;
                        pthread_mutex_unlock(&narrowBridge.mutex1);
                        pthread_cond_signal(&narrowBridge.cond0);
                        return;
                    } else if(next_direction ==1) {
                        pthread_mutex_lock(&narrowBridge.mutex1);
                        narrowBridge.current_direction = 1;
                        narrowBridge.carhaspassed=0;
                        pthread_mutex_unlock(&narrowBridge.mutex1);
                      //  cout << "fffffffffffff " <<car.id<< endl;
                        pthread_cond_signal(&narrowBridge.cond1);
                        return;
                    }
                    else{
                        pthread_mutex_lock(&narrowBridge.mutex1);
                        narrowBridge.carhaspassed=0;
                        narrowBridge.current_direction = -1;
                        pthread_mutex_unlock(&narrowBridge.mutex1);
                        return;
                    }
                }
                return;

            }


            return;
        }


        else if ((narrowBridge.waitingCars[narrowBridge.current_direction] <= 0) && (narrowBridge.car_is_passing ==0) ) {
          //  cout <<"no waiting at directionn" << car.id <<endl;

            pthread_mutex_lock(&narrowBridge.mutex1);
            car.same_car = true;
            narrowBridge.current_direction = fromDirection;
            pthread_mutex_unlock(&narrowBridge.mutex1);
        //    cout << "ggggggggggggggg " <<car.id<< endl;
            pthread_cond_signal(&narrowBridge.cond1);

            narrowBridgeWaitRoutine(car, narrowBridge, fromDirection, toDirection);
            return;
        }
        else {

            if(narrowBridge.waitingCars[1] == 1){
        //      cout << "timer1 : " << car.id<< endl;
                struct timeval tv;
                struct timespec ts;
                gettimeofday(&tv, NULL);

                ts.tv_sec = tv.tv_sec + narrowBridge.max_wait_limit / 1000;
                ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (narrowBridge.max_wait_limit % 1000);
                ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
                ts.tv_nsec %= (1000 * 1000 * 1000);

                pthread_mutex_lock(&narrowBridge.mutex1);


                int timed_wait_result = pthread_cond_timedwait(&narrowBridge.cond1,&narrowBridge.mutex1, &ts);
                pthread_mutex_unlock(&narrowBridge.mutex1);

                //pthread_mutex_unlock(&narrowBridge.mutex11);



                if (timed_wait_result == ETIMEDOUT) {
                  //  cout << "Timed out1: " << car.id << endl;
                    pthread_mutex_lock(&narrowBridge.mutex1);
                    car.same_car = true;
                    narrowBridge.current_direction = 1;
                    narrowBridge.carhaspassed=false;
                    pthread_mutex_unlock(&narrowBridge.mutex1);
                  //  cout << "hhhhhhhhhhhhh " <<car.id<< endl;
                  while(narrowBridge.carpassing0>0);
                    pthread_cond_signal(&narrowBridge.cond1);
                    //pthread_mutex_unlock(&narrowBridge.mutex11);

                    narrowBridgeWaitRoutine(car, narrowBridge, fromDirection, toDirection);
                    //pthread_mutex_unlock(&narrowBridge.mutex11);
                    return;
                }
                else{
                //    cout << "Received signal1: " << car.id << endl;


                    pthread_mutex_lock(&narrowBridge.mutex1);
                    car.same_car = true;
                    narrowBridge.current_direction = 1;
                    pthread_mutex_unlock(&narrowBridge.mutex1);
                  //  cout << "mmmmmmmmmmmmmmmmm" <<car.id<< endl;
                    pthread_cond_signal(&narrowBridge.cond1);
                    //pthread_mutex_unlock(&narrowBridge.mutex11);

                    narrowBridgeWaitRoutine(car, narrowBridge, fromDirection, toDirection);
                    //pthread_mutex_unlock(&narrowBridge.mutex11);
                    return;
                }
            }
            else{
              //  cout << "Waiting:1 " << car.id << endl;
                pthread_mutex_lock(&narrowBridge.mutex1);
                pthread_cond_wait(&narrowBridge.cond1, &narrowBridge.mutex1);
                car.same_car = true;
                pthread_mutex_unlock(&narrowBridge.mutex1);
              //  cout << "kkkkkkkkkkkkkkkk" <<car.id<< endl;
                pthread_cond_signal(&narrowBridge.cond1);
                narrowBridgeWaitRoutine(car, narrowBridge, fromDirection, toDirection);
                return;
            }
        }

    }



}




void FerryWaitingRoutine( Car  &car, FerryMonitor &ferry, int FerryID ,int fromDirection,int toDirection) {
    if(fromDirection == 0){
        pthread_mutex_lock(&ferry.mutex);
        ferry.num_carss0++;
        pthread_mutex_unlock(&ferry.mutex);
    //    cout << "numcars " << ferry.num_carss0 << endl;
        if(ferry.num_carss0==ferry.capacity){
      //      cout <<"full with car " <<car.id <<endl;
            WriteOutput(car.id, 'F', FerryID, START_PASSING);
            for (int i=1;i<ferry.capacity;i++){
                pthread_cond_signal(&ferry.cond0);
            }
            sleep_milli(ferry.travel_time);
            WriteOutput(car.id, 'F', FerryID, FINISH_PASSING);
            pthread_mutex_lock(&ferry.mutex);
            ferry.num_carss0--;
            ferry.reset_timer = true;
            pthread_mutex_unlock(&ferry.mutex);
            return;
        }
        else{
          struct timeval tv;
          struct timespec ts;
          gettimeofday(&tv, NULL);

          ts.tv_sec = tv.tv_sec + ferry.max_wait_limit / 1000;
          ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (ferry.max_wait_limit % 1000);
          ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
          ts.tv_nsec %= (1000 * 1000 * 1000);

          pthread_mutex_lock(&ferry.mutex);
          int timed_wait_result = pthread_cond_timedwait(&ferry.cond0,&ferry.mutex, &ts);
          pthread_mutex_unlock(&ferry.mutex);


            if (timed_wait_result == ETIMEDOUT) {
            //    cout << "jjjj  "<<endl;
                pthread_cond_broadcast(&ferry.cond0);

                WriteOutput(car.id, 'F', FerryID, START_PASSING);
                sleep_milli(ferry.travel_time);
                WriteOutput(car.id, 'F', FerryID, FINISH_PASSING);

                pthread_mutex_lock(&ferry.mutex);
                ferry.num_carss0--;
                pthread_mutex_unlock(&ferry.mutex);
                return;
            }

            WriteOutput(car.id, 'F', FerryID, START_PASSING);
            //pthread_cond_signal(&ferry.cond0);
            sleep_milli(ferry.travel_time);
            WriteOutput(car.id, 'F', FerryID, FINISH_PASSING);
            pthread_mutex_lock(&ferry.mutex);
            ferry.num_carss0--;
            pthread_mutex_unlock(&ferry.mutex);
            return;
        }

    }
    else{
        pthread_mutex_lock(&ferry.mutex1);
        ferry.num_carss1++;
        pthread_mutex_unlock(&ferry.mutex1);
        if(ferry.num_carss1==ferry.capacity){

            WriteOutput(car.id, 'F', FerryID, START_PASSING);
            pthread_mutex_unlock(&ferry.mutex1);
            for (int i=1;i<ferry.capacity;i++){
                pthread_cond_signal(&ferry.cond1);
            }
            sleep_milli(ferry.travel_time);
            WriteOutput(car.id, 'F', FerryID, FINISH_PASSING);
            pthread_mutex_lock(&ferry.mutex1);
            ferry.num_carss1--;
            pthread_mutex_unlock(&ferry.mutex1);
            return;
        }
        else{
          struct timeval tv;
          struct timespec ts;
          gettimeofday(&tv, NULL);

          ts.tv_sec = tv.tv_sec + ferry.max_wait_limit / 1000;
          ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (ferry.max_wait_limit % 1000);
          ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
          ts.tv_nsec %= (1000 * 1000 * 1000);


          pthread_mutex_lock(&ferry.mutex1);
          int timed_wait_result = pthread_cond_timedwait(&ferry.cond1,&ferry.mutex1, &ts);
          pthread_mutex_unlock(&ferry.mutex1);


            if (timed_wait_result == ETIMEDOUT) {
                pthread_cond_broadcast(&ferry.cond1);
                WriteOutput(car.id, 'F', FerryID, START_PASSING);
                sleep_milli(ferry.travel_time);
                WriteOutput(car.id, 'F', FerryID, FINISH_PASSING);
                pthread_mutex_lock(&ferry.mutex1);
                ferry.num_carss1--;
                pthread_mutex_unlock(&ferry.mutex1);
                return;
            }


            WriteOutput(car.id, 'F', FerryID, START_PASSING);
            //pthread_cond_signal(&ferry.cond1);
            sleep_milli(ferry.travel_time);
            WriteOutput(car.id, 'F', FerryID, FINISH_PASSING);
            pthread_mutex_lock(&ferry.mutex1);
            ferry.num_carss1--;
            pthread_mutex_unlock(&ferry.mutex1);
            return;
        }

    }
}




void crossRoadWaitRoutine(Car &car, Cross_road &crossroad , int fromDirection, int toDirection) {

    if (fromDirection == 0){

        pthread_mutex_lock(&crossroad.mutex0);
        if (!car.same_car) {
            crossroad.waitingCars[0]++;
        }
        if (crossroad.current_direction == -1) {
            crossroad.current_direction = 0;
            crossroad.carhaspassed0 = false;
        }
        pthread_mutex_unlock(&crossroad.mutex0);



        if(crossroad.current_direction == fromDirection) {

            if(crossroad.carhaspassed0 == false) {
          //      cout << "first car " << car.id <<endl;
                pthread_mutex_lock(&crossroad.mutex0);
                crossroad.car_is_passing0++;
                crossroad.carhaspassed0 = true;
                pthread_mutex_unlock(&crossroad.mutex0);

                WriteOutput(car.id, 'C', crossroad.id, START_PASSING);
                pthread_cond_signal(&crossroad.cond0);
                sleep_milli(crossroad.travel_time);
                WriteOutput(car.id, 'C', crossroad.id, FINISH_PASSING);

                pthread_mutex_lock(&crossroad.mutex0);
                crossroad.waitingCars[0]--;
                crossroad.car_is_passing0--;
                car.same_car = false;
                pthread_mutex_unlock(&crossroad.mutex0);

                if (crossroad.waitingCars[fromDirection] <= 0&&  crossroad.car_is_passing0 == 0) {
                    int next_direction = findNextNonzeroIndex(crossroad.waitingCars, 0, 4);
                //    cout << "next direction is " << next_direction << endl;
                    if (next_direction == 0) {

                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.current_direction = 0;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.carhaspassed0=false;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_cond_signal(&crossroad.cond0);
                        return;
                    } else if (next_direction == 1) {
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.current_direction = 1;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.carhaspassed1=false;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_cond_signal(&crossroad.cond1);
                        return;

                    } else if (next_direction == 2) {
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.current_direction = 2;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.carhaspassed2=false;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        pthread_cond_signal(&crossroad.cond2);
                        return;

                    } else if (next_direction == 3) {
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.current_direction = 3;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex3);
                        pthread_cond_signal(&crossroad.cond3);
                        return;

                    }
                    else{
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.carhaspassed0=false;
                        crossroad.carhaspassed1=false;
                        crossroad.carhaspassed2=false;
                        crossroad.carhaspassed3=false;

                        pthread_mutex_unlock(&crossroad.mutex0);
                        return;
                    }

                }
                return;

            }

            else {
              //  cout << "passing along " << car.id <<endl;
                pthread_mutex_lock(&crossroad.mutex0);
                crossroad.car_is_passing0++;
                pthread_mutex_unlock(&crossroad.mutex0);


                pthread_mutex_lock(&crossroad.mutex00);
                sleep_milli(PASS_DELAY);
                pthread_mutex_unlock(&crossroad.mutex00);

                WriteOutput(car.id, 'C', crossroad.id, START_PASSING);
                pthread_cond_signal(&crossroad.cond0);
                sleep_milli(crossroad.travel_time);
                WriteOutput(car.id, 'C', crossroad.id, FINISH_PASSING);

                pthread_mutex_lock(&crossroad.mutex0);
                crossroad.waitingCars[0]--;
                crossroad.car_is_passing0--;
                car.same_car = false;
                pthread_mutex_unlock(&crossroad.mutex0);

                if (crossroad.waitingCars[fromDirection] <= 0 &&  crossroad.car_is_passing0 == 0) {
                    int next_direction = findNextNonzeroIndex(crossroad.waitingCars, 0, 4);
                  //  cout << "next direction is " << next_direction << endl;
                    if (next_direction == 0) {
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.current_direction = 0;

                        crossroad.carhaspassed0=false;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_cond_signal(&crossroad.cond0);
                        return;
                    } else if (next_direction == 1) {
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.current_direction = 1;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.carhaspassed1=false;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_cond_signal(&crossroad.cond1);
                        return;

                    } else if (next_direction == 2) {
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.current_direction = 2;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.carhaspassed2=false;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        pthread_cond_signal(&crossroad.cond2);
                        return;

                    } else if (next_direction == 3) {
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.current_direction = 3;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex3);
                        pthread_cond_signal(&crossroad.cond3);
                        return;

                    }
                    else{
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.carhaspassed0=false;
                        crossroad.carhaspassed1=false;
                        crossroad.carhaspassed2=false;
                        crossroad.carhaspassed3=false;

                        pthread_mutex_unlock(&crossroad.mutex0);
                        return;
                    }
                }
                return;

            }
        }
        else if(crossroad.waitingCars[crossroad.current_direction]<=0 && (crossroad.get_car_is_passing(crossroad.current_direction) == 0)){
            pthread_mutex_lock(&crossroad.mutex0);
            crossroad.current_direction = fromDirection;
            car.same_car= true;
            crossroad.carhaspassed0=false;
            pthread_mutex_unlock(&crossroad.mutex0);
            pthread_cond_signal(&crossroad.cond0);

            crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
            return;
        }
        else{
            if(crossroad.waitingCars[0] == 1){
            //  cout << "timer0 " <<endl;
                struct timeval tv;
                struct timespec ts;
                gettimeofday(&tv, NULL);

                ts.tv_sec = tv.tv_sec + crossroad.max_wait_limit / 1000;
                ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (crossroad.max_wait_limit % 1000);
                ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
                ts.tv_nsec %= (1000 * 1000 * 1000);

                pthread_mutex_lock(&crossroad.mutex0);
                int timed_wait_result = pthread_cond_timedwait(&crossroad.cond0,&crossroad.mutex0, &ts);
                pthread_mutex_unlock(&crossroad.mutex0);
                if (timed_wait_result == ETIMEDOUT) {
                    int next_direction = findNextNonzeroIndex(crossroad.waitingCars, 0, 4);
                  //cout << "timeout0 " << car.id<< endl;
                    if (next_direction == 0) {
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.current_direction = 0;
                        crossroad.carhaspassed0=false;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        while(crossroad.car_is_passing1>0);
                        while(crossroad.car_is_passing2>0);
                        while(crossroad.car_is_passing3>0);
                        pthread_cond_signal(&crossroad.cond0);
                    } else if (next_direction == 1) {
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.current_direction = 1;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.carhaspassed1=false;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        while(crossroad.car_is_passing0>0);
                        while(crossroad.car_is_passing2>0);
                        while(crossroad.car_is_passing3>0);


                        pthread_cond_signal(&crossroad.cond1);

                    } else if (next_direction == 2) {
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.current_direction = 2;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.carhaspassed2=false;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        while(crossroad.car_is_passing1>0);
                        while(crossroad.car_is_passing0>0);
                        while(crossroad.car_is_passing3>0);


                        pthread_cond_signal(&crossroad.cond2);

                    } else if (next_direction == 3) {
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.current_direction = 3;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex3);
                        while(crossroad.car_is_passing1>0);
                        while(crossroad.car_is_passing2>0);
                        while(crossroad.car_is_passing0>0);


                        pthread_cond_signal(&crossroad.cond3);

                    }
                    else{
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.carhaspassed0=false;
                        crossroad.carhaspassed1=false;
                        crossroad.carhaspassed2=false;
                        crossroad.carhaspassed3=false;

                        pthread_mutex_unlock(&crossroad.mutex0);
                    }
                    car.same_car= true;
                    crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
                    return;
                }

                else{
                //  cout << "signaled0 " << car.id << endl;
                    pthread_mutex_lock(&crossroad.mutex0);
                    car.same_car= true;
                    pthread_mutex_unlock(&crossroad.mutex0);
                    pthread_cond_signal(&crossroad.cond0);

                    crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);

                    return;
                }
            }
            else{
            //    cout << "wait " << car.id << endl;
                pthread_mutex_lock(&crossroad.mutex0);
                pthread_cond_wait(&crossroad.cond0,&crossroad.mutex0);
                car.same_car= true;
                pthread_mutex_unlock(&crossroad.mutex0);
                pthread_cond_signal(&crossroad.cond0);
                crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
                return;
            }


        }
    }
    else if(fromDirection == 1){
        pthread_mutex_lock(&crossroad.mutex1);
        if (!car.same_car) {
            crossroad.waitingCars[1]++;
        }
        if (crossroad.current_direction == -1) {
            crossroad.current_direction = 1;
            crossroad.carhaspassed1=false;
        }
        pthread_mutex_unlock(&crossroad.mutex1);



        if(crossroad.current_direction == fromDirection) {
            while(crossroad.car_is_passing0>0);
            while(crossroad.car_is_passing2>0);
            while(crossroad.car_is_passing3>0);


            if(crossroad.carhaspassed1 == false) {
                pthread_mutex_lock(&crossroad.mutex1);
                crossroad.carhaspassed1 = true;
                crossroad.car_is_passing1++;
                pthread_mutex_unlock(&crossroad.mutex1);


                WriteOutput(car.id, 'C', crossroad.id, START_PASSING);
                pthread_cond_signal(&crossroad.cond1);
                sleep_milli(crossroad.travel_time);
                WriteOutput(car.id, 'C', crossroad.id, FINISH_PASSING);

                pthread_mutex_lock(&crossroad.mutex1);
                crossroad.waitingCars[1]--;
                crossroad.car_is_passing1--;
                car.same_car = false;
                pthread_mutex_unlock(&crossroad.mutex1);

                if (crossroad.waitingCars[fromDirection] <= 0 &&  crossroad.car_is_passing1 == 0) {
                    int next_direction = findNextNonzeroIndex(crossroad.waitingCars, 1, 4);
                  //  cout << "next direction is " << next_direction << endl;
                    if (next_direction == 0) {
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.current_direction = 0;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.carhaspassed0=false;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_cond_signal(&crossroad.cond0);
                        return;
                    } else if (next_direction == 1) {
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.current_direction = 1;
                        crossroad.carhaspassed1=false;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_cond_signal(&crossroad.cond1);
                        return;

                    } else if (next_direction == 2) {
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.current_direction = 2;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.carhaspassed2=false;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        pthread_cond_signal(&crossroad.cond2);
                        return;

                    } else if (next_direction == 3) {
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.current_direction = 3;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex3);
                        pthread_cond_signal(&crossroad.cond3);
                        return;

                    }
                    else{
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.carhaspassed0=false;
                        crossroad.carhaspassed1=false;
                        crossroad.carhaspassed2=false;
                        crossroad.carhaspassed3=false;

                        pthread_mutex_unlock(&crossroad.mutex1);
                        return;
                    }
                }
                return;

            }
            else {
                pthread_mutex_lock(&crossroad.mutex1);
                crossroad.car_is_passing1++;
                pthread_mutex_unlock(&crossroad.mutex1);

                pthread_mutex_lock(&crossroad.mutex11);
                sleep_milli(PASS_DELAY);
                pthread_mutex_unlock(&crossroad.mutex11);

                WriteOutput(car.id, 'C', crossroad.id, START_PASSING);
                pthread_cond_signal(&crossroad.cond1);
                sleep_milli(crossroad.travel_time);
                WriteOutput(car.id, 'C', crossroad.id, FINISH_PASSING);

                pthread_mutex_lock(&crossroad.mutex1);

                crossroad.waitingCars[1]--;
                crossroad.car_is_passing1--;
                car.same_car = false;
                pthread_mutex_unlock(&crossroad.mutex1);

                if (crossroad.waitingCars[fromDirection] <= 0 &&  crossroad.car_is_passing1 == 0) {
                    int next_direction = findNextNonzeroIndex(crossroad.waitingCars, 1, 4);
                //    cout << "next direction is " << next_direction << endl;
                    if (next_direction == 0) {
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.current_direction = 0;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.carhaspassed0=false;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_cond_signal(&crossroad.cond0);
                        return;
                    } else if (next_direction == 1) {
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.current_direction = 1;


                        crossroad.carhaspassed1=false;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_cond_signal(&crossroad.cond1);
                        return;

                    } else if (next_direction == 2) {
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.current_direction = 2;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.carhaspassed2=false;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        pthread_cond_signal(&crossroad.cond2);
                        return;

                    } else if (next_direction == 3) {
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.current_direction = 3;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex3);
                        pthread_cond_signal(&crossroad.cond3);
                        return;

                    }
                    else{
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.carhaspassed0=false;
                        crossroad.carhaspassed1=false;
                        crossroad.carhaspassed2=false;
                        crossroad.carhaspassed3=false;

                        pthread_mutex_unlock(&crossroad.mutex1);
                        return;
                    }
                }
                return;

            }

        }
        else if(crossroad.waitingCars[crossroad.current_direction]<=0 && (crossroad.get_car_is_passing(crossroad.current_direction) == 0)){
            pthread_mutex_lock(&crossroad.mutex1);
            crossroad.current_direction = fromDirection;
            car.same_car= true;
            crossroad.carhaspassed1=false;
            pthread_mutex_unlock(&crossroad.mutex1);

            pthread_cond_signal(&crossroad.cond1);

            crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
            return;
        }
        else{
            if(crossroad.waitingCars[1] == 1){
            //    cout<< "timer1 : " <<car.id<<endl;
                //pthread_mutex_unlock(&crossroad.mutex0);

                //pthread_mutex_unlock(&crossroad.mutex111);
              //  cout << "wait timerr " << car.id << endl;
                struct timeval tv;
                struct timespec ts;
                gettimeofday(&tv, NULL);

                ts.tv_sec = tv.tv_sec + crossroad.max_wait_limit / 1000;
                ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (crossroad.max_wait_limit % 1000);
                ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
                ts.tv_nsec %= (1000 * 1000 * 1000);

                pthread_mutex_lock(&crossroad.mutex1);
                int timed_wait_result = pthread_cond_timedwait(&crossroad.cond1,&crossroad.mutex1, &ts);

                pthread_mutex_unlock(&crossroad.mutex1);
                if (timed_wait_result == ETIMEDOUT) {
                  // cout<< "timeout : " <<car.id<<endl;
                    int next_direction = findNextNonzeroIndex(crossroad.waitingCars, 1, 4);
                  //  cout << "next direction is " << next_direction << endl;
                    if (next_direction == 0) {
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.current_direction = 0;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.carhaspassed0=false;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        while(crossroad.car_is_passing1>0);
                        while(crossroad.car_is_passing2>0);
                        while(crossroad.car_is_passing3>0);




                        pthread_cond_signal(&crossroad.cond0);
                    } else if (next_direction == 1) {
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.current_direction = 1;

                        crossroad.carhaspassed1=false;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        while(crossroad.car_is_passing0>0);
                        while(crossroad.car_is_passing2>0);
                        while(crossroad.car_is_passing3>0);



                        pthread_cond_signal(&crossroad.cond1);
                    } else if (next_direction == 2) {
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.current_direction = 2;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.carhaspassed2=false;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        while(crossroad.car_is_passing1>0);
                        while(crossroad.car_is_passing0>0);
                        while(crossroad.car_is_passing3>0);


                        pthread_cond_signal(&crossroad.cond2);
                    } else if (next_direction == 3) {
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.current_direction = 3;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex3);
                        while(crossroad.car_is_passing1>0);
                        while(crossroad.car_is_passing2>0);
                        while(crossroad.car_is_passing0>0);


                        pthread_cond_signal(&crossroad.cond3);
                    }
                    else{
                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.carhaspassed0=false;
                        crossroad.carhaspassed1=false;
                        crossroad.carhaspassed2=false;
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex1);
                    }
                    car.same_car= true;
                    crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
                    return;
                }

                else{
                  //  cout<< "sleep : " <<car.id<<endl;
                    pthread_mutex_lock(&crossroad.mutex1);
                    car.same_car= true;
                    pthread_mutex_unlock(&crossroad.mutex1);
                    pthread_cond_signal(&crossroad.cond1);
                    crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
                    return;
                }
            }
            else{
              //  cout << "waitt  " << car.id << endl;
                pthread_mutex_lock(&crossroad.mutex1);
                pthread_cond_wait(&crossroad.cond1,&crossroad.mutex1);
                //cout<< "sleep : " <<car.id<<endl;

                car.same_car= true;
                pthread_mutex_unlock(&crossroad.mutex1);
                pthread_cond_signal(&crossroad.cond1);
                crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
                return;
            }


        }

    }
    else if(fromDirection == 2){
        pthread_mutex_lock(&crossroad.mutex2);
        if (!car.same_car) {
            crossroad.waitingCars[2]++;
        }

        if (crossroad.current_direction == -1) {
            crossroad.current_direction = 2;
            crossroad.carhaspassed2=false;
        }
        pthread_mutex_unlock(&crossroad.mutex2);


        if(crossroad.current_direction == fromDirection) {
            while(crossroad.car_is_passing1>0);
            while(crossroad.car_is_passing0>0);
            while(crossroad.car_is_passing3>0);

            if(crossroad.carhaspassed2 == false) {
            //  cout << "first " << car.id << endl;


                pthread_mutex_lock(&crossroad.mutex2);
                crossroad.carhaspassed2 = true;
                crossroad.car_is_passing2++;
                pthread_mutex_unlock(&crossroad.mutex2);

                WriteOutput(car.id, 'C', crossroad.id, START_PASSING);
                pthread_cond_signal(&crossroad.cond2);
                sleep_milli(crossroad.travel_time);
                WriteOutput(car.id, 'C', crossroad.id, FINISH_PASSING);

                pthread_mutex_lock(&crossroad.mutex2);
                crossroad.waitingCars[2]--;
                crossroad.car_is_passing2--;
                car.same_car = false;
                pthread_mutex_unlock(&crossroad.mutex2);

                if (crossroad.waitingCars[fromDirection] <= 0  &&  crossroad.car_is_passing2 == 0) {
                    int next_direction = findNextNonzeroIndex(crossroad.waitingCars, 2, 4);
                    if (next_direction == 0) {
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.current_direction = 0;
                        pthread_mutex_unlock(&crossroad.mutex2);

                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.carhaspassed0=false;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_cond_signal(&crossroad.cond0);
                        return;
                    }
                    else if (next_direction == 1) {
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.current_direction = 1;
                        pthread_mutex_unlock(&crossroad.mutex2);

                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.carhaspassed1=false;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_cond_signal(&crossroad.cond1);
                        return;

                    } else if (next_direction == 2) {
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.current_direction = 2;
                        crossroad.carhaspassed2=false;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        pthread_cond_signal(&crossroad.cond2);
                        return;

                    } else if (next_direction == 3) {
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.current_direction = 3;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex3);
                        pthread_cond_signal(&crossroad.cond3);
                        return;

                    }
                    else{
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.carhaspassed0=false;
                        crossroad.carhaspassed1=false;
                        crossroad.carhaspassed2=false;
                        crossroad.carhaspassed3=false;

                        pthread_mutex_unlock(&crossroad.mutex2);
                        return;
                    }
                }
                return;
            }
            else {
            //  cout <<   "pass along " << car.id << endl;

                pthread_mutex_lock(&crossroad.mutex2);
                crossroad.car_is_passing2++;
                pthread_mutex_unlock(&crossroad.mutex2);

                pthread_mutex_lock(&crossroad.mutex22);
                sleep_milli(PASS_DELAY);
                pthread_mutex_unlock(&crossroad.mutex22);

                WriteOutput(car.id, 'C', crossroad.id, START_PASSING);
                pthread_cond_signal(&crossroad.cond2);
                sleep_milli(crossroad.travel_time);
                WriteOutput(car.id, 'C', crossroad.id, FINISH_PASSING);

                pthread_mutex_lock(&crossroad.mutex2);
                crossroad.waitingCars[2]--;
                crossroad.car_is_passing2--;
                car.same_car = false;
                pthread_mutex_unlock(&crossroad.mutex2);

                if (crossroad.waitingCars[fromDirection] <= 0  &&  crossroad.car_is_passing2 == 0) {
                    int next_direction = findNextNonzeroIndex(crossroad.waitingCars, 2, 4);
                    if (next_direction == 0) {
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.current_direction = 0;
                        pthread_mutex_unlock(&crossroad.mutex2);

                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.carhaspassed0=false;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        pthread_cond_signal(&crossroad.cond0);
                        return;
                    } else if (next_direction == 1) {
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.current_direction = 1;
                        pthread_mutex_unlock(&crossroad.mutex2);

                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.carhaspassed1=false;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        pthread_cond_signal(&crossroad.cond1);
                        return;

                    } else if (next_direction == 2) {
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.current_direction = 2;
                        crossroad.carhaspassed2=false;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        pthread_cond_signal(&crossroad.cond2);
                        return;

                    } else if (next_direction == 3) {
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.current_direction = 3;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex3);
                        pthread_cond_signal(&crossroad.cond3);
                        return;

                    }
                    else{
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.carhaspassed0=false;
                        crossroad.carhaspassed1=false;
                        crossroad.carhaspassed2=false;
                        crossroad.carhaspassed3=false;

                        pthread_mutex_unlock(&crossroad.mutex2);
                        return;
                    }
                }
                return;

            }

        }
        else if(crossroad.waitingCars[crossroad.current_direction]<=0 && (crossroad.get_car_is_passing(crossroad.current_direction) == 0)){

            pthread_mutex_lock(&crossroad.mutex2);
            crossroad.current_direction = fromDirection;
            car.same_car= true;
            crossroad.carhaspassed2=false;
            pthread_mutex_unlock(&crossroad.mutex2);
            pthread_cond_signal(&crossroad.cond2);
            crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
            return;
        }
        else{
            if(crossroad.waitingCars[2] == 1){
                struct timeval tv;
                struct timespec ts;
                gettimeofday(&tv, NULL);

                ts.tv_sec = tv.tv_sec + crossroad.max_wait_limit / 1000;
                ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (crossroad.max_wait_limit % 1000);
                ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
                ts.tv_nsec %= (1000 * 1000 * 1000);

              pthread_mutex_lock(&crossroad.mutex2);
                int timed_wait_result = pthread_cond_timedwait(&crossroad.cond2,&crossroad.mutex2, &ts);
                pthread_mutex_unlock(&crossroad.mutex2);


                if (timed_wait_result == ETIMEDOUT) {
                    int next_direction = findNextNonzeroIndex(crossroad.waitingCars, 2, 4);
                    if (next_direction == 0) {
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.current_direction = 0;
                        pthread_mutex_unlock(&crossroad.mutex2);

                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.carhaspassed0=false;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        while(crossroad.car_is_passing1>0);
                        while(crossroad.car_is_passing2>0);
                        while(crossroad.car_is_passing3>0);


                        pthread_cond_signal(&crossroad.cond0);
                    } else if (next_direction == 1) {
                      pthread_mutex_lock(&crossroad.mutex2);
                      crossroad.current_direction = 1;
                      pthread_mutex_unlock(&crossroad.mutex2);

                        pthread_mutex_lock(&crossroad.mutex1);
                        crossroad.carhaspassed1=false;
                        pthread_mutex_unlock(&crossroad.mutex1);
                        while(crossroad.car_is_passing0>0);
                        while(crossroad.car_is_passing2>0);
                        while(crossroad.car_is_passing3>0);


                        pthread_cond_signal(&crossroad.cond1);

                    } else if (next_direction == 2) {
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.current_direction = 2;
                        crossroad.carhaspassed2=false;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        while(crossroad.car_is_passing1>0);
                        while(crossroad.car_is_passing0>0);
                        while(crossroad.car_is_passing3>0);


                        pthread_cond_signal(&crossroad.cond2);

                    } else if (next_direction == 3) {

                      pthread_mutex_lock(&crossroad.mutex2);
                      crossroad.current_direction = 3;
                      pthread_mutex_unlock(&crossroad.mutex2);


                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex3);
                        while(crossroad.car_is_passing1>0);
                        while(crossroad.car_is_passing2>0);
                        while(crossroad.car_is_passing0>0);


                        pthread_cond_signal(&crossroad.cond3);

                    }
                    else{
                        pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.carhaspassed0=false;
                        crossroad.carhaspassed1=false;
                        crossroad.carhaspassed2=false;
                        crossroad.carhaspassed3=false;

                        pthread_mutex_unlock(&crossroad.mutex2);
                    }
                    car.same_car= true;
                    crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
                    return;
                }

                else{
                    pthread_mutex_lock(&crossroad.mutex2);
                    car.same_car= true;
                    pthread_mutex_unlock(&crossroad.mutex2);
                    pthread_cond_signal(&crossroad.cond2);
                    crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
                    return;

                }
            }
            else{
                pthread_mutex_lock(&crossroad.mutex2);
                pthread_cond_wait(&crossroad.cond2,&crossroad.mutex2);
                //cout<< "sleep : " <<car.id<<endl;

                car.same_car= true;
                pthread_mutex_unlock(&crossroad.mutex2);
                pthread_cond_signal(&crossroad.cond2);
                crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
                return;
            }

        }
    }
    else if(fromDirection == 3){
        pthread_mutex_lock(&crossroad.mutex3);
        if (!car.same_car) {
            crossroad.waitingCars[3]++;
        }
        if (crossroad.current_direction == -1) {
            crossroad.current_direction = 3;
            crossroad.carhaspassed3=false;
        }
        pthread_mutex_unlock(&crossroad.mutex3);



        if(crossroad.current_direction == fromDirection) {
            while(crossroad.car_is_passing1>0);
            while(crossroad.car_is_passing2>0);
            while(crossroad.car_is_passing0>0);

            if(crossroad.carhaspassed3 == false) {

                pthread_mutex_lock(&crossroad.mutex3);
                crossroad.carhaspassed3 = true;
                crossroad.car_is_passing3++;
                pthread_mutex_unlock(&crossroad.mutex3);

                WriteOutput(car.id, 'C', crossroad.id, START_PASSING);
                pthread_cond_signal(&crossroad.cond3);
                sleep_milli(crossroad.travel_time);
                WriteOutput(car.id, 'C', crossroad.id, FINISH_PASSING);

                pthread_mutex_lock(&crossroad.mutex3);
                crossroad.waitingCars[3]--;
                crossroad.car_is_passing3--;
                car.same_car = false;
                pthread_mutex_unlock(&crossroad.mutex3);

                if (crossroad.waitingCars[fromDirection] <= 0 &&  crossroad.car_is_passing3 == 0) {
                    int next_direction = findNextNonzeroIndex(crossroad.waitingCars, 3, 4);
                    if (next_direction == 0) {
                      pthread_mutex_lock(&crossroad.mutex3);
                      crossroad.current_direction = 0;
                      pthread_mutex_unlock(&crossroad.mutex3);
                      pthread_mutex_lock(&crossroad.mutex0);
                      crossroad.carhaspassed0=false;
                      pthread_mutex_unlock(&crossroad.mutex0);
                      pthread_cond_signal(&crossroad.cond0);
                      return;
                    } else if (next_direction == 1) {
                      pthread_mutex_lock(&crossroad.mutex3);
                      crossroad.current_direction = 1;
                      pthread_mutex_unlock(&crossroad.mutex3);
                      pthread_mutex_lock(&crossroad.mutex1);
                      crossroad.carhaspassed1=false;
                      pthread_mutex_unlock(&crossroad.mutex1);
                      pthread_cond_signal(&crossroad.cond1);
                      return;

                    } else if (next_direction == 2) {
                      pthread_mutex_lock(&crossroad.mutex3);
                      crossroad.current_direction = 2;
                      pthread_mutex_unlock(&crossroad.mutex3);
                      pthread_mutex_lock(&crossroad.mutex2);
                        crossroad.carhaspassed2=false;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        pthread_cond_signal(&crossroad.cond2);
                        return;

                    } else if (next_direction == 3) {
                      pthread_mutex_lock(&crossroad.mutex3);
                      crossroad.current_direction = 3;
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex3);
                        pthread_cond_signal(&crossroad.cond3);
                        return;
                    }
                    else{
                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.carhaspassed0=false;
                        crossroad.carhaspassed1=false;
                        crossroad.carhaspassed2=false;
                        crossroad.carhaspassed3=false;

                        pthread_mutex_unlock(&crossroad.mutex3);
                        return;
                    }
                }
                return;
            }
            else {
              pthread_mutex_lock(&crossroad.mutex3);
                crossroad.car_is_passing3++;
                pthread_mutex_unlock(&crossroad.mutex3);


                pthread_mutex_lock(&crossroad.mutex33);
                sleep_milli(PASS_DELAY);
                pthread_mutex_unlock(&crossroad.mutex33);


                WriteOutput(car.id, 'C', crossroad.id, START_PASSING);
                pthread_cond_signal(&crossroad.cond3);
                sleep_milli(crossroad.travel_time);
                WriteOutput(car.id, 'C', crossroad.id, FINISH_PASSING);

                pthread_mutex_lock(&crossroad.mutex3);
                crossroad.waitingCars[3]--;
                crossroad.car_is_passing3--;
                car.same_car = false;
                pthread_mutex_unlock(&crossroad.mutex3);

                if (crossroad.waitingCars[fromDirection] <= 0  &&  crossroad.car_is_passing3 == 0) {
                    int next_direction = findNextNonzeroIndex(crossroad.waitingCars, 3, 4);
                    if (next_direction == 0) {
                      pthread_mutex_lock(&crossroad.mutex3);
                      crossroad.current_direction = 0;
                      pthread_mutex_unlock(&crossroad.mutex3);
                      pthread_mutex_lock(&crossroad.mutex0);
                      crossroad.carhaspassed0=false;
                      pthread_mutex_unlock(&crossroad.mutex0);
                      pthread_cond_signal(&crossroad.cond0);
                      return;
                    } else if (next_direction == 1) {
                      pthread_mutex_lock(&crossroad.mutex3);
                      crossroad.current_direction = 1;
                      pthread_mutex_unlock(&crossroad.mutex3);
                      pthread_mutex_lock(&crossroad.mutex1);
                      crossroad.carhaspassed1=false;
                      pthread_mutex_unlock(&crossroad.mutex1);
                      pthread_cond_signal(&crossroad.cond1);
                      return;

                    } else if (next_direction == 2) {
                      pthread_mutex_lock(&crossroad.mutex3);
                      crossroad.current_direction = 2;
                      pthread_mutex_unlock(&crossroad.mutex3);

                      pthread_mutex_lock(&crossroad.mutex2);

                        crossroad.carhaspassed2=false;
                        pthread_mutex_unlock(&crossroad.mutex2);
                        pthread_cond_signal(&crossroad.cond2);
                        return;

                    } else if (next_direction == 3) {
                      pthread_mutex_lock(&crossroad.mutex3);
                      crossroad.current_direction = 3;
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex3);
                        pthread_cond_signal(&crossroad.cond3);
                        return;
                    }
                    else{
                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.carhaspassed0=false;
                        crossroad.carhaspassed1=false;
                        crossroad.carhaspassed2=false;
                        crossroad.carhaspassed3=false;

                        pthread_mutex_unlock(&crossroad.mutex3);
                        return;
                    }
                }
                return;

            }

        }
        else if(crossroad.waitingCars[crossroad.current_direction]<=0 && (crossroad.get_car_is_passing(crossroad.current_direction) == 0)){

            pthread_mutex_lock(&crossroad.mutex3);
            crossroad.current_direction = fromDirection;
            car.same_car= true;
            crossroad.carhaspassed3=false;
            pthread_mutex_unlock(&crossroad.mutex3);
            pthread_cond_signal(&crossroad.cond3);
            crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
            return;
        }
        else{
            if(crossroad.waitingCars[3] == 1){
              struct timeval tv;
                struct timespec ts;
                gettimeofday(&tv, NULL);

                ts.tv_sec = tv.tv_sec + crossroad.max_wait_limit / 1000;
                ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (crossroad.max_wait_limit % 1000);
                ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
                ts.tv_nsec %= (1000 * 1000 * 1000);
                pthread_mutex_lock(&crossroad.mutex3);
                int timed_wait_result = pthread_cond_timedwait(&crossroad.cond3,&crossroad.mutex3, &ts);
                pthread_mutex_unlock(&crossroad.mutex3);

                if (timed_wait_result == ETIMEDOUT) {
                    int next_direction = findNextNonzeroIndex(crossroad.waitingCars, 3, 4);
                    if (next_direction == 0) {
                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.current_direction = 0;
                        pthread_mutex_unlock(&crossroad.mutex3);

                        pthread_mutex_lock(&crossroad.mutex0);
                        crossroad.carhaspassed0=false;
                        pthread_mutex_unlock(&crossroad.mutex0);
                        while(crossroad.car_is_passing1>0);
                        while(crossroad.car_is_passing2>0);
                        while(crossroad.car_is_passing3>0);


                        pthread_cond_signal(&crossroad.cond0);

                    } else if (next_direction == 1) {
                      pthread_mutex_lock(&crossroad.mutex3);
                      crossroad.current_direction = 1;
                      pthread_mutex_unlock(&crossroad.mutex3);

                      pthread_mutex_lock(&crossroad.mutex1);
                      crossroad.carhaspassed1=false;
                      pthread_mutex_unlock(&crossroad.mutex1);
                      while(crossroad.car_is_passing0>0);
                      while(crossroad.car_is_passing2>0);
                      while(crossroad.car_is_passing3>0);


                        pthread_cond_signal(&crossroad.cond1);

                    } else if (next_direction == 2) {
                      pthread_mutex_lock(&crossroad.mutex3);
                      crossroad.current_direction = 2;
                      pthread_mutex_unlock(&crossroad.mutex3);

                      pthread_mutex_lock(&crossroad.mutex2);
                      crossroad.carhaspassed2=false;
                      pthread_mutex_unlock(&crossroad.mutex2);
                      while(crossroad.car_is_passing1>0);
                      while(crossroad.car_is_passing0>0);
                      while(crossroad.car_is_passing3>0);


                        pthread_cond_signal(&crossroad.cond2);

                    } else if (next_direction == 3) {
                      pthread_mutex_lock(&crossroad.mutex3);
                      crossroad.current_direction = 3;
                      crossroad.carhaspassed3=false;
                      pthread_mutex_unlock(&crossroad.mutex3);
                      while(crossroad.car_is_passing1>0);
                      while(crossroad.car_is_passing2>0);
                      while(crossroad.car_is_passing0>0);


                        pthread_cond_signal(&crossroad.cond3);

                    }
                    else{
                        pthread_mutex_lock(&crossroad.mutex3);
                        crossroad.carhaspassed0=false;
                        crossroad.carhaspassed1=false;
                        crossroad.carhaspassed2=false;
                        crossroad.carhaspassed3=false;
                        pthread_mutex_unlock(&crossroad.mutex3);
                    }
                    car.same_car= true;
                    crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
                    return;
                }

                else{
                   pthread_mutex_lock(&crossroad.mutex3);
                   car.same_car= true;
                    pthread_mutex_unlock(&crossroad.mutex3);
                    pthread_cond_signal(&crossroad.cond3);
                    crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
                    return;
                }
            }
            else{
                pthread_mutex_lock(&crossroad.mutex3);
                pthread_cond_wait(&crossroad.cond3,&crossroad.mutex3);
                car.same_car= true;
                pthread_mutex_unlock(&crossroad.mutex3);
                pthread_cond_signal(&crossroad.cond3);
                crossRoadWaitRoutine(car,crossroad,fromDirection,toDirection);
                return;
            }

        }

    }


}






string parseConnectorType(string connectorType) {
    if (connectorType[0] == 'N') {
        return "N";
    }
    if (connectorType[0] == 'F') {
        return "F";
    }
    if (connectorType[0] == 'C' ) {
        return "C";
    }
    return connectorType;
}
void* carThreadRoutine(void* arg) {
    int d_id= *((int *)arg);
    Car *car = &cars[d_id];

    for (int i = 0; i < car->pathLength; ++i) {
        //cout <<"car "<<car->id << " i: " << i <<endl;
        string connectorType = car->pathObjects[i].connectorType;
        int fromDirection = car->pathObjects[i].fromDirection;
        int toDirection = car->pathObjects[i].toDirection;
        string contype = parseConnectorType(connectorType);

        if (contype == "N") {
            int narrowBridgeId = stoi(connectorType.substr(1)); // Extract the narrow bridge ID from the connectorType
            WriteOutput(car->id, 'N', narrowBridgeId, TRAVEL);
            sleep_milli(car->travel_time);
            WriteOutput(car->id, 'N', narrowBridgeId, ARRIVE);
            narrowBridgeWaitRoutine(*car, narrowBridges[narrowBridgeId], fromDirection,toDirection);
        }
        else if (contype == "F") {
            //cout << "car " << car->id << "coming "  << endl;
            int FerryId = stoi(connectorType.substr(1));
            WriteOutput(car->id, 'F', FerryId, TRAVEL);
            sleep_milli(car->travel_time);
            WriteOutput(car->id, 'F', FerryId, ARRIVE);
            FerryWaitingRoutine(*car,ferries[FerryId],FerryId,fromDirection,toDirection);

        }
        else if (contype == "C") {
            int CrossRoadId = stoi(connectorType.substr(1));

            WriteOutput(car->id, 'C', CrossRoadId, TRAVEL);
            sleep_milli(car->travel_time);
            WriteOutput(car->id, 'C', CrossRoadId, ARRIVE);

            crossRoadWaitRoutine(*car,crossroads[CrossRoadId],fromDirection,toDirection);
        }
    }
    return nullptr; // Exit the thread
}

int main() {
    cin >> numNarrowBridges;
    narrowBridges.resize(numNarrowBridges);
    for (int i = 0; i < numNarrowBridges; ++i) {
        int travelTime, maxWaitLimit;
        cin >> travelTime >> maxWaitLimit;
        narrowBridges[i] = Narrow_bridge(i, travelTime, maxWaitLimit);
    }
    cin >> numFerries;
    ferries.resize(numFerries);
    for (int i = 0; i < numFerries; ++i) {
        int travelTime, max_wait_limit, capacity;
        cin >> travelTime >> max_wait_limit >> capacity;
        ferries[i] = FerryMonitor(i,travelTime,max_wait_limit,capacity);
        //FerryMonitor f(i,travelTime,max_wait_limit,capacity);
        //ferries.emplace_back(i,travelTime,max_wait_limit,capacity);
        //ferries[i] = Ferry(i, travelTime, max_wait_limit,capacity);
    }

    cin >> numCrossroads;
    crossroads.resize(numCrossroads);
    for (int i = 0; i < numCrossroads; ++i) {
        int travelTime, max_wait_limit;
        cin >> travelTime >> max_wait_limit;

        crossroads[i] = Cross_road(i,travelTime,max_wait_limit);

    }

    cin >> numCars;
    cars.resize(numCars);
    for (int i = 0; i < numCars; ++i) {
        cars[i].id = i;
        cin >> cars[i].travel_time >> cars[i].pathLength;
        string connectorType;
        int fromDirection, toDirection;
        for (int j = 0; j < cars[i].pathLength; ++j) {
            cin >> connectorType >> fromDirection >> toDirection;
            PathObject pathObj;
            pathObj.connectorType = connectorType;
            pathObj.fromDirection = fromDirection;
            pathObj.toDirection = toDirection;
            cars[i].pathObjects.push_back(pathObj);
            cars[i].same_car= false;
        }
    }

    // Print out the parsed information
    /*cout << "Narrow Bridges:" << endl;
    for (int i = 0; i < numNarrowBridges; ++i) {
        cout << "Bridge " << i << ": Travel Time = " << narrowBridges[i].travel_time
             << ", Max Wait Time = " << narrowBridges[i].max_wait_limit << endl;
    }
    cout << "Ferries:" << endl;
    for (int i = 0; i < numFerries; ++i) {
        cout << "Ferry " << i << ": Travel Time = " << ferries[i].travel_time
             << ", Max Wait Time = " << ferries[i].max_wait_limit << ", Capacity = " << ferries[i].capacity << endl;
    }
    cout << "Crossroads:" << endl;
    for (int i = 0; i < numCrossroads; ++i) {
        cout << "Crossroad " << i << ": Travel Time = " << crossroads[i].travel_time
             << ", Max Wait Time = " << crossroads[i].max_wait_limit << endl;
    }
    cout << "Cars:" << endl;
    for (int i = 0; i < numCars; ++i) {
        cout << "Car " << i << ": Travel Time = " << cars[i].travel_time
             << ", Path Length = " << cars[i].pathLength << endl;
        cout << "Path Objects:" << endl;
        for (int j = 0; j < cars[i].pathLength; ++j) {
            cout << "   " << cars[i].pathObjects[j].connectorType << ": From " << cars[i].pathObjects[j].fromDirection
                 << " to " << cars[i].pathObjects[j].toDirection << endl;
        }
    }*/
    InitWriteOutput();
    pthread_barrier_t barrier; // Declare a pthread barrier
    pthread_barrier_init(&barrier, NULL, numCars + 1); // "+ 1" to account for the main thread
    pthread_t carThreads[numCars];
    int *carIds;
    carIds = new int [numCars];
    for (int i = 0; i < numCars; ++i) {
        // Create arguments for the car thread
        carIds[i] = i;
        pthread_create(&carThreads[i], NULL, carThreadRoutine,  (void *)&carIds[i]); // Pass the index of the car
    }

// Wait for all car threads to be created before proceeding


// All car threads are created, now start them concurrently
    for (int i = 0; i < numCars; ++i) {
        pthread_join(carThreads[i], NULL);

    }

// Destroy the barrier after its use

    return 0;

}
