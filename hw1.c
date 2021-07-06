#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#define BUFFER_SIZE 256

void read_write_pipe(int fdr, int fdw)
{ // read, write
    FILE *stream_r, *stream_w;
    char buf[BUFFER_SIZE];
    stream_r = fdopen(fdr, "r");
    stream_w = fdopen(fdw, "w");

    while (fgets(buf, BUFFER_SIZE, stream_r) != NULL)
        fputs(buf, stream_w);

    fclose(stream_r);
    fclose(stream_w);
}
void Upper_write(int fdr, int fdw)
{
    FILE *stream_r, *stream_w;
    char buf[BUFFER_SIZE];
    stream_r = fdopen(fdr, "r");
    stream_w = fdopen(fdw, "w");

    while (fgets(buf, BUFFER_SIZE, stream_r) != NULL)
    {
        for (int i = 0; i < strlen(buf); i++)
        {
            // small -> upper
            if (buf[i] >= 'a' && buf[i] <= 'z')
            {
                buf[i] = buf[i] - 32;
            }
            else
            {
                continue;
            }
        }
        fputs(buf, stream_w);
    }

    fclose(stream_r);
    fclose(stream_w);
}

void write_line_word(int fdr, int fdw)
{
    FILE *stream_r, *stream_w;
    char buf[BUFFER_SIZE];
    stream_r = fdopen(fdr, "r");
    stream_w = fdopen(fdw, "w");

    char line_word[BUFFER_SIZE];
    int line_cnt = 0, word_cnt = 0;
    int real_word = 0;

    while (fgets(buf, BUFFER_SIZE, stream_r) != NULL)
    {
        for (int i = 0; i < strlen(buf); i++)
        {
            if (buf[i] == '\n')
            {
                line_cnt += 1;
                if (real_word)
                {
                    word_cnt += 1;
                    real_word = 1;
                }
            }
            else if (buf[i] == 32 || buf[i] == '\t' || buf[i] == '\r' || buf[i] == '\v' || buf[i] == '\f')
            {
                if (real_word)
                {
                    word_cnt += 1;
                    real_word = 0;
                }
            }
            if (buf[i] > 32)
                real_word = 1;
        }
        fputs(buf, stream_w);
    }
    sprintf(line_word, "\n %d %d \n", line_cnt, word_cnt);
    fputs(line_word, stream_w);

    fclose(stream_r);
    fclose(stream_w);
}

int main(int argc, char *argv[])
{
    int pid_P, pid_Q, pid_R;
    int fd_pq[2], fd_qr[2], fd_rp[2];
    int statusq, statusr;
    int input_txt;
    FILE *steram_input;

    FILE *fp;
    fp = fopen("result_read.txt", "w+");
    int fd = fileno(fp);

    // Create the pipe
    pipe(fd_pq);
    pipe(fd_rp);

    // Create the child process
    if (fork() == 0)
    {

        // Create the pipe
        pipe(fd_qr);

        // Create the child process
        if (fork() == 0)
        {
            // R process
            pid_R = getpid();

            close(fd_rp[1]);
            close(fd_rp[0]);
            close(fd_pq[0]);
            close(fd_pq[1]);
            close(fd_qr[1]); // write close

            write_line_word(fd_qr[0], fd);

            close(fd_qr[0]);
            close(fd);
            exit(0);
        }

        else
        {
            // Q process
            pid_Q = getpid();

            close(fd_rp[0]);
            close(fd_rp[1]);
            close(fd_pq[1]); //write close
            close(fd_qr[0]); // read close

            Upper_write(fd_pq[0], fd_qr[1]);

            close(fd_pq[0]);
            close(fd_qr[1]);

            waitpid(pid_R, &statusr, 0);
            exit(0);
        }
    }
    else
    {
        // P process
        close(fd_pq[0]);
        close(fd_rp[0]);

        // open text file for reading
        if ((input_txt = open(argv[1], 0)) == -1)
        {
            perror(argv[1]);
            return 1;
        }

        read_write_pipe(input_txt, fd_pq[1]);
        close(fd_pq[1]);

        // printf data from R
        waitpid(pid_R, &statusr, 0);

        waitpid(pid_Q, &statusq, 0);
        exit(0);
    }

    return 0;
}
