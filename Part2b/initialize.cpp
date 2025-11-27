#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>    
#include <fstream>
#include <string>
#include <filesystem>
#include <semaphore.h>

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
    sem_t semaphore;
};




namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    int shmid = shmget(IPC_PRIVATE, sizeof(SharedMemory), IPC_CREAT | 0666);
    SharedMemory *data = (SharedMemory *) shmat(shmid, nullptr, 0);
    if (shmid == -1) {
    perror("shmget failed");
    exit(1);
    }

    data->counter = 0;
    data->num_ta = 0;
    data->num_wait = 0;
    sem_init(&data->semaphore, 1, 1);
    std::ifstream file("rubric.txt");
    std::string line;

    while (std::getline(file, line)) {
        size_t comma_pos = line.find(',');
        if (comma_pos == std::string::npos) continue;

        int number = std::stoi(line.substr(0, comma_pos));
        char letter = line[comma_pos + 1];
        data->rubric[number] = letter;
        
    }

    std::string folder = "./exams";  
    int counter = 0;
    for (const auto& entry : fs::directory_iterator(folder)) {
        std::ifstream file(entry.path());
            std::getline(file, line);
            int stu_num = std::stoi(line);
            if(stu_num<9999){
                data->student[counter] = Student{stu_num,{false,false,false,false,false,false}};
                counter++;
            }
    }
    data->student[counter] = Student{9999,{false,false,false,false,false,false}};

    if(argc < 2){
        std::cout << "Error: No inputs" << std::endl;
        return 0;
    }
    int limit = std::atoi(argv[1]);
    for(int i = 0; i < limit; i++){
        pid_t pid = fork();
        if (pid == 0) {
            char shmid_str[16];
            snprintf(shmid_str, sizeof(shmid_str), "%d", shmid);
            execl("./ta", "ta", shmid_str, nullptr);
            perror("execl failed");
            _exit(1);
        }
    }
    int status;
    pid_t finished_pid = wait(&status);
    
}

