#include "types.h"
#include "user.h"

void cpu_bound() {
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 1000000; j++) {
            asm("nop");
        }
    }
}

void short_cpu() {
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 1000000; j++) {
            if ((j+1) % 10000 == 0)
                yield();
            asm("nop");
        }
    }
}

void io_bound() {
    for (int i = 0; i < 100; i++) {
        sleep(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(1, "Usage: sanity <n>\n");
        exit();
    }
    
    int n;
    int i;
    int r;
    int process_type = 0;
    char* process_types_str[20] = {"CPU-Bound", "S-CPU", "IO-Bound"};

    int run_avg_cpu_b_rutime[5];
    int run_avg_cpu_b_retime[5];
    int run_avg_cpu_b_stime[5];

    int run_avg_s_cpu_rutime[5];
    int run_avg_s_cpu_retime[5];
    int run_avg_s_cpu_stime[5];

    int run_avg_io_b_rutime[5];
    int run_avg_io_b_retime[5];
    int run_avg_io_b_stime[5];

    int avg_cpu_b_rutime = 0;
    int avg_cpu_b_retime = 0;
    int avg_cpu_b_stime = 0;

    int avg_s_cpu_rutime = 0;
    int avg_s_cpu_retime = 0;
    int avg_s_cpu_stime = 0;

    int avg_io_b_rutime = 0;
    int avg_io_b_retime = 0;
    int avg_io_b_stime = 0;

    n = atoi(argv[1]);

    int pid;

    for(r = 0; r < 5; r++) {

        printf(1, "\n--------------------------------- RUN %d ---------------------------------\n\n", r+1);

        int n_cpu_b_proc = 0;
        int n_s_cpu_proc = 0;
        int n_io_b_proc = 0;

        for (i = 0; i < n; i++) {
            pid = fork();
            if (pid == 0) { //child
                process_type = (getpid() - 4) % 3;
                switch (process_type) {
                case 0:
                    set_prio(2);
                    cpu_bound();
                    break;
                case 1:
                    set_prio(1);
                    short_cpu();
                    break;
                case 2:
                    set_prio(0);
                    io_bound();
                    break;
                }
                exit(); // children exit here
            }
            continue; // father continues to spawn the next child
        }

        int sum_cpu_b_rutime = 0;
        int sum_cpu_b_retime = 0;
        int sum_cpu_b_stime = 0;

        int sum_s_cpu_rutime = 0;
        int sum_s_cpu_retime = 0;
        int sum_s_cpu_stime = 0;

        int sum_io_b_rutime = 0;
        int sum_io_b_retime = 0;
        int sum_io_b_stime = 0;

        for (i = 0; i < n; i++) {
            int retime = 0;
            int rutime = 0;
            int stime = 0;
            
            pid = wait2(&retime, &rutime, &stime);
            int process_type = (pid - 4) % 3;

            printf(1, "PID: %d, Type: %s, Running: %d, Ready: %d, Sleeping: %d, Turnaround: %d\n", 
                pid, process_types_str[process_type], rutime, retime, stime, retime + rutime + stime);

            if (process_type == 0) {
                n_cpu_b_proc++;
            } else if (process_type == 1) {
                n_s_cpu_proc++;
            } else if (process_type == 2) {
                n_io_b_proc++;
            }

            if (process_type == 0) {
                sum_cpu_b_rutime += rutime;
                sum_cpu_b_retime += retime;
                sum_cpu_b_stime += stime;
            } else if (process_type == 1) {
                sum_s_cpu_rutime += rutime;
                sum_s_cpu_retime += retime;
                sum_s_cpu_stime += stime;
            } else if (process_type == 2) {
                sum_io_b_rutime += rutime;
                sum_io_b_retime += retime;
                sum_io_b_stime += stime;
            }
        }

        run_avg_cpu_b_rutime[r] = sum_cpu_b_rutime / n_cpu_b_proc;
        run_avg_cpu_b_retime[r] = sum_cpu_b_retime / n_cpu_b_proc;
        run_avg_cpu_b_stime[r] = sum_cpu_b_stime / n_cpu_b_proc;

        run_avg_s_cpu_rutime[r] = sum_s_cpu_rutime / n_s_cpu_proc;
        run_avg_s_cpu_retime[r] = sum_s_cpu_retime / n_s_cpu_proc;
        run_avg_s_cpu_stime[r] = sum_s_cpu_stime / n_s_cpu_proc;

        run_avg_io_b_rutime[r] = sum_io_b_rutime / n_io_b_proc;
        run_avg_io_b_retime[r] = sum_io_b_retime / n_io_b_proc;
        run_avg_io_b_stime[r] = sum_io_b_stime / n_io_b_proc;

    }

    for (i = 0; i < 5; i++) {
        avg_cpu_b_rutime += run_avg_cpu_b_rutime[i];
        avg_cpu_b_retime += run_avg_cpu_b_retime[i];
        avg_cpu_b_stime += run_avg_cpu_b_stime[i];
        avg_s_cpu_rutime += run_avg_s_cpu_rutime[i];
        avg_s_cpu_retime += run_avg_s_cpu_retime[i];
        avg_s_cpu_stime += run_avg_s_cpu_stime[i];
        avg_io_b_rutime += run_avg_io_b_rutime[i];
        avg_io_b_retime += run_avg_io_b_retime[i];
        avg_io_b_stime += run_avg_io_b_stime[i];
    }

    avg_cpu_b_rutime /= 5;
    avg_cpu_b_retime /= 5;
    avg_cpu_b_stime /= 5; 
    avg_s_cpu_rutime /= 5;
    avg_s_cpu_retime /= 5;
    avg_s_cpu_stime /= 5; 
    avg_io_b_rutime /= 5; 
    avg_io_b_retime /= 5; 
    avg_io_b_stime /= 5; 

    printf(1, "\nCPU-Bound:\nAverage running time: %d\nAverage ready time: %d\nAverage sleeping time: %d\nAverage turnaround time: %d\n", 
        avg_cpu_b_rutime, avg_cpu_b_retime, avg_cpu_b_stime, avg_cpu_b_rutime + avg_cpu_b_retime + avg_cpu_b_stime);
    printf(1, "\nS-CPU:\nAverage running time: %d\nAverage ready time: %d\nAverage sleeping time: %d\nAverage turnaround time: %d\n",
        avg_s_cpu_rutime, avg_s_cpu_retime, avg_s_cpu_stime, avg_s_cpu_rutime + avg_s_cpu_retime + avg_s_cpu_stime);
    printf(1, "\nIO-Bound:\nAverage running time: %d\nAverage ready time: %d\nAverage sleeping time: %d\nAverage turnaround time: %d\n\n", 
        avg_io_b_rutime, avg_io_b_retime, avg_io_b_stime, avg_io_b_rutime + avg_io_b_retime + avg_io_b_stime);

    exit();
}

