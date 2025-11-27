#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <random>


 
struct Student {
    int student_number;
    bool graded[6];
};


struct SharedMemory {
    Student student[20];
    char rubric[6];
    int counter;
    int num_ta;
    int num_wait;
};

void idle_rubric(SharedMemory *S){
    S->num_wait++;
    while(S->num_wait > 0){
        if(S->num_wait >= S->num_ta){
            S->num_wait = 0;
        }
    }
}

void idle_questions(SharedMemory *S){
    S->num_wait++;
    while(S->num_wait > 0){
        if(S->num_wait >= S->num_ta){
            S->counter++;
            S->num_wait = 0;
        }
    }
}


int main(int argc, char *argv[]) {
    int shmid = atoi(argv[1]);
    SharedMemory *data = (SharedMemory *) shmat(shmid, nullptr, 0);
    
    data->num_ta++;
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(500, 1000);
    for(int j=1;j<6;j++){
        std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
        if(rand() % 2 == 1.0){
            std::cout << "Change rubric on question " << j << " from " << data->rubric[j] << " to " << ++ data->rubric[j] << std::endl;
        }
    }
    idle_rubric(data);

    dist = std::uniform_int_distribution<int>(1000, 2000);
    
    while(data->counter < 20){

        for (int j=1;j<6;j++){
            if(data->student[data->counter].graded[j] == false){
                data->student[data->counter].graded[j] = true;
                
                std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
                std::cout << "Grading student " << data->student[data->counter].student_number << " on question " << j << std::endl;
            }
        }
        idle_questions(data);

    }

    std::cout << "Process 2 finished" << std::endl;

    shmdt(data);
    return 0;
}
