/*
    COMP3511 Fall 2022 
    PA1: Simplified Linux Shell (MyShell)

    Your name: Lee Wai Kiu
    Your ITSC email: wkleeak@connect.ust.hk 

    Declaration:

    I declare that I am not involved in plagiarism
    I understand that both parties (i.e., students providing the codes and students copying the codes) will receive 0 marks. 

*/

// Note: Necessary header files are included
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h> // For constants that are required in open/read/write/close syscalls
#include <sys/wait.h> // For wait() - suppress warning messages
#include <fcntl.h> // For open/read/write/close syscalls

// Assume that each command line has at most 256 characters (including NULL)
#define MAX_CMDLINE_LEN 256

// Assume that we have at most 8 arguments
#define MAX_ARGUMENTS 8

// Assume that we only need to support 2 types of space characters: 
// " " (space) and "\t" (tab)
#define SPACE_CHARS " \t"

// The pipe character
#define PIPE_CHAR "|"

// Assume that we only have at most 8 pipe segements, 
// and each segment has at most 256 characters
#define MAX_PIPE_SEGMENTS 8

// Assume that we have at most 8 arguments for each segment
//
// We also need to add an extra NULL item to be used in execvp
//
// Thus: 8 + 1 = 9
//
// Example: 
//   echo a1 a2 a3 a4 a5 a6 a7 
//
// execvp system call needs to store an extra NULL to represent the end of the parameter list
//
//   char *arguments[MAX_ARGUMENTS_PER_SEGMENT]; 
//
//   strings stored in the array: echo a1 a2 a3 a4 a5 a6 a7 NULL
//
#define MAX_ARGUMENTS_PER_SEGMENT 9

// Define the  Standard file descriptors here
#define STDIN_FILENO    0       // Standard input
#define STDOUT_FILENO   1       // Standard output 


 
// This function will be invoked by main()
// TODO: Implement the multi-level pipes below
void process_cmd(char *cmdline);

// read_tokens function is given
// This function helps you parse the command line
// Note: Before calling execvp, please remember to add NULL as the last item 
void read_tokens(char **argv, char *line, int *numTokens, char *token); 
// divide the [line] into segment and put in the [argv], also add up the numTokens

// Here is an example code that illustrates how to use the read_tokens function
// int main() {
//     char *pipe_segments[MAX_PIPE_SEGMENTS]; // character array buffer to store the pipe segements
//     int num_pipe_segments; // an output integer to store the number of pipe segment parsed by this function
//     char cmdline[MAX_CMDLINE_LEN]; // the input argument of the process_cmd function
//     int i, j;
//     char *arguments[MAX_ARGUMENTS_PER_SEGMENT] = {NULL}; 
//     int num_arguments;
//     strcpy(cmdline, "ls | sort -r | sort | sort -r | sort | sort -r | sort | sort -r");
//     read_tokens(pipe_segments, cmdline, &num_pipe_segments, PIPE_CHAR);
//     for (i=0; i< num_pipe_segments; i++) {
//         printf("%d : %s\n", i, pipe_segments[i] );    
//         read_tokens(arguments, pipe_segments[i], &num_arguments, SPACE_CHARS);
//         for (j=0; j<num_arguments; j++) {
//             printf("\t%d : %s\n", j, arguments[j]);
//         }
//     }
//     return 0;
// }


/* The main function implementation */
int main()
{
    char cmdline[MAX_CMDLINE_LEN];
    fgets(cmdline, MAX_CMDLINE_LEN, stdin); //put stdin(input stream) in the cmdline(variable)
    process_cmd(cmdline);
    return 0;
}

// TODO: implementation of process_cmd
void process_cmd(char *cmdline)
{
  // You can try to write: printf("%s\n", cmdline); to check the content of cmdline

    // printf("%s\n", cmdline,"%s\n"); // already can check the command in the input file
    //1. read the command "./myshell < in01.txt" and dertermine the input file
//     int num_arguments_inShell;
//     char *arguments_inShell[MAX_ARGUMENTS_PER_SEGMENT]; //dont need equal null as dont need to implement this line in execlp
//     read_tokens(arguments_inShell, cmdline, &num_arguments_inShell, SPACE_CHARS);

//     //check what is the input file.
//     char inputFile[MAX_ARGUMENTS_PER_SEGMENT]; //use char* or char??

//     for(int i=0; i<num_arguments_inShell; i++){
//         if(arguments_inShell[i] == "<"){
//             strcpy(inputFile,arguments_inShell[i+1]);
//         }
//     }
//     printf("%s\n Following is inputFile %s\n", inputFile);
// //////////////////////////////////////////////////////////
// //already get the input file from the command line in shell

    // read the command in the file
    int fd;
    int num_arguments;
    char *arguments[MAX_ARGUMENTS_PER_SEGMENT*3]={NULL}; // need to hvae null here because later need to use execvp

    // fd = open(inputFile, /* output file name */
    //     O_RDONLY,
    //     S_IRUSR | S_IWUSR );
    // read(fd, buffer, MAX_CMDLINE_LEN); // read fd and ssore content in buffer(strinf type)
    read_tokens(arguments, cmdline, &num_arguments, SPACE_CHARS); // break doen cmdline into arguments
    // Now [command stored in the files] is broken downa and store  in arguments[]
    // for(int i=0; i<num_arguments; i++){
    //     printf(" %d : %s\n", i, arguments[i]);
    // }

    // char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
    // strcpy(arguments_for_real_1,arguments[0]);
    // char arguments_for_real_2[MAX_ARGUMENTS_PER_SEGMENT];
    // strcpy(arguments_for_real_2,arguments[1]);
    // char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1,arguments_for_real_2, NULL};

    // printf("%s\n", arguments_for_real[0],"%s\n");
    // printf("%s\n", arguments_for_real[1],"%s\n");
    // printf("%s\n", arguments_for_real[2],"%s\n");

    //Tested: can break down the cmdline into array
    char input[MAX_CMDLINE_LEN];
    char output[MAX_CMDLINE_LEN];
    int has_input=0;
    int index_of_input=0;
    int has_output=0;
    int index_of_output=0;
    int has_pipe=0;
    int no_of__pipe=0;

    for(int i=0; i<num_arguments; i++){
        if(strcmp(arguments[i],"<") == 0){
            // printf("\n this is arguments[i+1] %s",arguments[i+1]);
            //dont know why cannot use this 
            strcpy(input,arguments[i+1]);
            // strncpy(input,arguments[i+1],strlen(arguments[i+1]));
            // printf("\n this is input %s",input);
            has_input+=1;
            index_of_input=i;
        } 
        else if(strcmp(arguments[i],">") == 0){
            // printf("%s\n this is arguments[i+1]", arguments[i+1],"%s\n");
            //dont know why cannot use this 
            // strncpy(output,arguments[i+1],strlen(arguments[i+1]));
            strcpy(output,arguments[i+1]);
            has_output+=1;
            index_of_output=i;
        }
        else if(strcmp(arguments[i],"|") == 0){
            has_pipe+= 1;
            no_of__pipe+= 1;
        }
    }
//found the input file, output file and wheteher there have  < , > , |

    // Following will cause segmentation error
    // printf("%s\n Following is arguments_inShell  before if else %s\n");
    // for(int i=0; i<num_arguments; i++){
    //     printf("%s\n",arguments[i],"%s\n");
    // }

    if(has_input==0 && has_output==0 && has_pipe==0){ //ensure no < , > , |
        char command[MAX_CMDLINE_LEN];
        strcpy(command,arguments[0]);
        execvp(command, arguments);
    }
    else if(has_input==1 && has_output==0){
        int inputid = open(input, /* input file name */
                    O_RDONLY,
                    S_IRUSR | S_IWUSR );
        // pfds[1]=inputid;
        close(0); //close stdin
        dup2(inputid, 0);// make the input file as the stdin

        char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
        strcpy(arguments_for_real_1,arguments[0]);
        char arguments_for_real_2[MAX_ARGUMENTS_PER_SEGMENT];
        strcpy(arguments_for_real_2,arguments[1]);
        char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1,arguments_for_real_2, NULL};

        
        // printf("\n this is arguments[0] %s", arguments[0]);
        // printf("\n this is arguments[1] %s", arguments[1]);
        // printf("\n this is arguments[2] %s", arguments[2]);
        // printf("\n this is arguments[3] %s", arguments[3]);

        // printf("\n this is arguments_for_real_1 %s ", arguments_for_real_1);
        // printf("\n this is arguments_for_real_2  %s", arguments_for_real_2);
        
        // printf("\n this is arguments_for_real[0] %s", arguments_for_real[0]);
        // printf("\n this is arguments_for_real[1] %s", arguments_for_real[1]);
        // printf("\n this is arguments_for_real[2] %s", arguments_for_real[2]);

        char command[MAX_CMDLINE_LEN];
        strcpy(command,arguments_for_real[0]);
        execvp(command, arguments_for_real);

        // cannot change the content of char*
        
        // read(inputid, buffer_in_file, MAX_CMDLINE_LEN);
        // read_tokens(arguments, buffer, &num_arguments, SPACE_CHARS);

        // for(int i=0; i<num_arguments; i++){
        //     if(strcmp(arguments[i],"<") == 0){
        //         printf("%s\n  Inside if %s\n");
        //         for(int j=i; j<num_arguments; i++){
        //             arguments[j]=NULL;
        //         }
        //         break;
        //     }   
        // }

        // char command[MAX_CMDLINE_LEN];
        // strcpy(command,arguments[0]);
        // execvp(command, arguments);


        // printf("%s\n", command ,"%s\n");
    }
    else if(has_input==0 && has_output==1){
        FILE * fp;
        fp = fopen(output, "w+");
        int outputid = open(output, /* output file name */
                    O_CREAT | O_WRONLY , /* flags */
                    S_IRUSR | S_IWUSR ); /* user permission: 600 */

        close(1); /* Close stdout */
        dup2(outputid,1); /* use the output file as the stdout  */

        char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
        strcpy(arguments_for_real_1,arguments[0]);
        char arguments_for_real_2[MAX_ARGUMENTS_PER_SEGMENT];
        strcpy(arguments_for_real_2,arguments[1]);
        char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1,arguments_for_real_2, NULL};

        // printf("%s\n", arguments_for_real[0]);
        // printf("%s\n", arguments_for_real[1]);
        // printf("%s\n", arguments_for_real[2]);

        char command[MAX_CMDLINE_LEN];
        strcpy(command,arguments_for_real[0]);
        execvp(command, arguments_for_real);
    }
    else if(has_input==1 && has_output==1){
        int inputid = open(input, /* input file name */
                    O_RDONLY,
                    S_IRUSR | S_IWUSR );

        // pfds[1]=inputid;
        close(0); //close stdin
        dup2(inputid, 0);// make the input file as the stdin

        FILE * fp;
        fp = fopen(output, "w+");
        int outputid = open(output, /* output file name */
                    O_CREAT | O_WRONLY , /* flags */
                    S_IRUSR | S_IWUSR ); /* user permission: 600 */

        close(1); /* Close stdout */
        dup2(outputid,1); /* use the output file as the stdout  */

        char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
        strcpy(arguments_for_real_1,arguments[0]);
        char arguments_for_real_2[MAX_ARGUMENTS_PER_SEGMENT];
        strcpy(arguments_for_real_2,arguments[1]);
        char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1,arguments_for_real_2, NULL};

        char command[MAX_CMDLINE_LEN];
        strcpy(command,arguments_for_real[0]);
        execvp(command, arguments_for_real);
    }

//now trying
if(has_pipe>0){
    int pfds[2]; 
    int pfds_X[2];
    // pipe(pfds); //Create a pipe
    int closeNo=0;
    int closeNo_X=0;
    int current_index=0;
    int how_many_command=0;
    int count_no_of__pipe = no_of__pipe+1; 
    int using_pfds=0;
    int using_pfds_X=0;

    for(int i=0; i<no_of__pipe+1; i++){
        count_no_of__pipe-=1;
        if(count_no_of__pipe!=no_of__pipe && (i%2==0)){
            using_pfds=0;
            close(pfds[0]);
            close(pfds[1]);
        }

        if(count_no_of__pipe!=no_of__pipe && (i%2==1)){
            using_pfds_X=0;
            close(pfds_X[0]);
            close(pfds_X[1]);
        }

        if(count_no_of__pipe>=1 && (i%2==1)){
            pipe(pfds_X);
            using_pfds_X+=1;
        }
        else if(count_no_of__pipe>=1 && (i%2==0)){
            pipe(pfds);
            using_pfds+=1;
        }

        how_many_command=0; 
///////////////////////
        if(i%2==1){
            closeNo=1;
            closeNo_X=0;
        }
        else{
            closeNo=0;
            closeNo_X=1;
        }

        //check the posiiton of the pointer(which check the command segment) and how_many_command
        int y =current_index; //first loop  y =current_index =0
        for(int i=y; i<num_arguments; i++){
            if(strcmp(arguments[i],"|") == 0){
                current_index+=1; 
                break;  //first loop   current_index=2 
            }
            else{
                current_index+=1;
                how_many_command+=1; 
            }
        }

        // printf("\n count_no_of__pipe : %d",count_no_of__pipe);
        // printf("\n using_pfds : %d",using_pfds);
        // printf("\n closeNo : %d",closeNo);
        // printf("\n using_pfds_X : %d",using_pfds_X);
        // printf("\n closeNo_X : %d",closeNo_X);
        // printf("\n");
        
        if(fork() == 0){
            // printf("\n INSIDE FORK");
            // printf("\n");

            if(using_pfds>0){
                close(pfds[closeNo]); //first loop will be close(0) first
                if (closeNo == 1){
                    // printf("\n INSIDE using_pfds closeNo == 1");
                    // printf("\n how_many_command = %d",how_many_command);
                    // printf("\n");
                    dup2(pfds[0],0);
                    close(pfds[0]);
                }
                else if (closeNo == 0){
                    // printf("\n INSIDE using_pfds closeNo == 0");
                    // printf("\n how_many_command = %d",how_many_command);
                    // printf("\n");
                    dup2(pfds[1],1);  //first loop will 
                    close(pfds[1]);
                }
            }
            if(using_pfds_X>0){
                close(pfds_X[closeNo_X]); //first loop will be close(0) first
                if (closeNo_X == 1){
                    // printf("\n INSIDE using_pfds_X closeNo_X == 1");
                    // printf("\n how_many_command = %d",how_many_command);
                    // printf("\n");
                    dup2(pfds_X[0],0);
                    close(pfds_X[0]);
                }
                else if (closeNo_X == 0){
                    // printf("\n INSIDE using_pfds_X closeNo_X == 0");
                    // printf("\n how_many_command = %d",how_many_command);
                    // printf("\n");
                    dup2(pfds_X[1],1);  //first loop will 
                    close(pfds_X[1]);
                }
            }

            if(how_many_command==1){
                // printf("\n INSIDE how_many_command==1");
                // printf("\n");
                char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
                if(current_index==num_arguments){ //because it wont goto the if(="|") so it odent -1 as ell
                    strcpy(arguments_for_real_1,arguments[current_index-1]);
                }else{
                    strcpy(arguments_for_real_1,arguments[current_index-2]); //arguments_for_real_1 = arguments[0]
                }
                char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1, NULL};

                // printf("\n there is 1 command which is %s",arguments_for_real[0]);
                // printf("\n there is 1 command which is %s",arguments_for_real[1]);
                // printf("\n");

                if(count_no_of__pipe!=no_of__pipe){
                    // printf("\n Waiting");
                    // printf("\n");
                    wait(0);
                }

                char command[MAX_CMDLINE_LEN];
                strcpy(command,arguments_for_real[0]);
                execvp(command, arguments_for_real);
            }
            else if(how_many_command==2){
                // printf("\n INSIDE how_many_command==2");
                // printf("\n");
                char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
                char arguments_for_real_2[MAX_ARGUMENTS_PER_SEGMENT];

                if(current_index==num_arguments){ //because it wont goto the if(="|") so it odent -1 as ell
                    strcpy(arguments_for_real_1,arguments[current_index-2]);
                    strcpy(arguments_for_real_2,arguments[current_index-1]);
                }else{
                    strcpy(arguments_for_real_1,arguments[current_index-3]);
                    strcpy(arguments_for_real_2,arguments[current_index-2]);//arguments_for_real_1 = arguments[0]
                } 
                char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1,arguments_for_real_2, NULL};

                // printf("\n there is 2 command which is %s",arguments_for_real[0]);
                // printf("\n there is 2 command which is %s",arguments_for_real[1]);
                // printf("\n there is 2 command which is %s",arguments_for_real[2]);
                // printf("\n");

                if(count_no_of__pipe!=no_of__pipe){
                    // printf("\n Waiting");
                    // printf("\n");
                    wait(0);
                }

                char command[MAX_CMDLINE_LEN];
                strcpy(command,arguments_for_real[0]);
                execvp(command, arguments_for_real);
            }
        }
    }
    // printf("\n LAST");
    // printf("\n");
    if(using_pfds==1){
        using_pfds=0;
        close(pfds[0]);
        close(pfds[1]);
    }

    if(using_pfds_X==1){
        using_pfds_X=0;
        close(pfds_X[0]);
        close(pfds_X[1]);
    }
    wait(0);
}

// if(has_pipe<0 && no_of__pipe==1){
//     // for(int i=0; i<num_arguments; i++){
//     //     printf("%s\n",arguments[i]);
//     // }

//     int pfds[2]; 
//     pipe(pfds); //Create a pipe
//     pid_t pid = fork();
//     int current_index=num_arguments-1; // 3
//     int how_many_command=0;

//     // printf("\n Runnning ");

//     if (pid == 0 ){
//         // close(1); //first loop will be close(0) first
//         // dup(pfds[1]);
//         // close(pfds[0]);
//         close(pfds[0]); //first loop will be close(0) first
//         dup2(pfds[1],1);
//         close(pfds[1]);

//         char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
//         strcpy(arguments_for_real_1,arguments[0]); //arguments_for_real_1 = arguments[0]
//         char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1, NULL};

//         char command[MAX_CMDLINE_LEN];
//         strcpy(command,arguments_for_real[0]);
//         execvp(command, arguments_for_real);
//     }
//     else{
//         // close(0); //first loop will be close(0) first
//         // dup(pfds[0]);
//         // close(pfds[1]);
//         close(pfds[1]); //first loop will be close(0) first
//         dup2(pfds[0],0);
//         close(pfds[0]);

//         char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
//         char arguments_for_real_2[MAX_ARGUMENTS_PER_SEGMENT];
//         strcpy(arguments_for_real_1,arguments[2]);
//         strcpy(arguments_for_real_2,arguments[3]);
//         char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1,arguments_for_real_2, NULL};

//         // printf("\n there is 2 command which is %s",arguments_for_real[0]);
//         // printf("\n there is 2 command which is %s",arguments_for_real[1]);
//         // printf("\n there is 2 command which is %s",arguments_for_real[2]);

//         wait(0);
//         char command[MAX_CMDLINE_LEN];
//         strcpy(command,arguments_for_real[0]);
//         execvp(command, arguments_for_real);
//     }
// }
// if(has_pipe<0 && no_of__pipe==2){
//     int pfds[2]; 
//     int pfds_X[2]; 
//     pipe(pfds); //Create a pipe

//     if(fork() == 0){
//         close(pfds[0]); //close the stdout and dont output the result to the terminal
//         dup2(pfds[1],1);// instead, output in the pipe pfds_X
//         close(pfds[1]);

//         char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
//         char arguments_for_real_2[MAX_ARGUMENTS_PER_SEGMENT];
//         strcpy(arguments_for_real_1,arguments[0]);
//         strcpy(arguments_for_real_2,arguments[1]);
//         char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1,arguments_for_real_2, NULL};

//         // printf("In ls -l \n");
//         // printf("arguments_for_ ls -l : %s  %s \n",arguments_for_real_1, arguments_for_real_2);
        
//         char command[MAX_CMDLINE_LEN];
//         strcpy(command,arguments_for_real[0]);
//         execvp(command, arguments_for_real);
//     }

//     pipe(pfds_X); //Create a pipe

//     if ( fork() == 0){
//         close(pfds[1]); //dont listen to the input
//         dup2(pfds[0],0); // instead listen to the pipe output
//         close(pfds[0]);

//         close(pfds_X[0]); //dont listen to the input
//         dup2(pfds_X[1],1); // instead listen to the pipe output
//         close(pfds_X[1]);

//         char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
//         strcpy(arguments_for_real_1,arguments[3]); //arguments_for_real_1 = arguments[0]
//         char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1, NULL};

//         // printf("In sort \n");
//         // printf("arguments_for sort : %s \n",arguments_for_real_1);
//         // wait(0);
        
//         char command[MAX_CMDLINE_LEN];
//         strcpy(command,arguments_for_real[0]);
//         execvp(command, arguments_for_real);
//     }

//     close(pfds[1]);
//     close(pfds[0]);

//     if ( fork() == 0){
//         close(pfds_X[1]); //first loop will be close(0) first
//         dup2(pfds_X[0],0);
//         close(pfds_X[0]);

//         char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
//         char arguments_for_real_2[MAX_ARGUMENTS_PER_SEGMENT];
//         strcpy(arguments_for_real_1,arguments[5]);
//         strcpy(arguments_for_real_2,arguments[6]);
//         char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1,arguments_for_real_2, NULL};
//         // printf("In wc -l \n");
//         // printf("arguments_for_ wc -l : %s %s \n",arguments_for_real_1, arguments_for_real_2);

//         char command[MAX_CMDLINE_LEN];
//         strcpy(command,arguments_for_real[0]);
//         execvp(command, arguments_for_real);
//     }
//     close(pfds_X[1]);
//     close(pfds_X[0]);
// }
        // for(int i=current_index; i>=0; i--){ //start from 3 to 0
        //     if(strcmp(arguments[i],"|") == 0){
        //         current_index-=1;// Now the current_index will be 0 (indicate where stop, aslo the location of "|"")
        //         break;  //current_index=2 after this loop
        //     }
        //     else{
        //         current_index-=1;  //will finally = -1
        //         how_many_command+=1; //how_many_command=2 after this loop
        //         printf("\n Inside for loop %d : %s", i, arguments[i]);
        //     }
        // }
}
//Follow  function can run in08.txt

// if(has_pipe>0){
//     int pfds[2]; 
//     pipe(pfds); //Create a pipe
//     int closeNo=0;
//     int current_index=0;
//     int how_many_command=0;

//     for(int i=0; i<no_of__pipe+1; i++){
//         how_many_command=0; 

//         closeNo=i%2;

//         // printf("\n this closeNo is %d",closeNo);
//         // printf("\n this no_of__pipe is %d",no_of__pipe);

//         int y =current_index; //first loop  y =current_index =0
//         // printf("\n current_index == %d",current_index);
//         for(int i=y; i<num_arguments; i++){
//             if(strcmp(arguments[i],"|") == 0){
//                 current_index+=1; 
//                 break;  //first loop   current_index=2 
//             }
//             else{
//                 current_index+=1;
//                 how_many_command+=1; 
//             }
//         }

//         if(fork() == 0){
//             close(pfds[closeNo]); //first loop will be close(0) first
//             if (closeNo == 1){
//                 // printf("\n Calling  closeNo == 1");
//                 dup2(pfds[0],0);
//                 close(pfds[0]);
//             }
//             else if (closeNo == 0){
//                 // printf("\n Calling  closeNo == 0");
//                 dup2(pfds[1],1);  //first loop will 
//                 close(pfds[1]);
//             }
//             /////////////////////////////////////////
//             // int y =current_index; //first loop  y =current_index =0
//             // printf("\n current_index == %d",current_index);
//             // for(int i=y; i<num_arguments; i++){
//             //     if(strcmp(arguments[i],"|") == 0){
//             //         current_index+=1; 
//             //         break;  //first loop   current_index=2 
//             //     }
//             //     else{
//             //         current_index+=1;
//             //         how_many_command+=1; 
//             //     }
//             // }

//             if(how_many_command==1){
//                 char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
//                 if(current_index==4){ //because it wont goto the if(="|") so it odent -1 as ell
//                     strcpy(arguments_for_real_1,arguments[current_index-1]);
//                 }else{
//                     strcpy(arguments_for_real_1,arguments[current_index-2]); //arguments_for_real_1 = arguments[0]
//                 }
//                 char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1, NULL};

//                 // printf("\n there is 1 command which is %s",arguments_for_real[0]);
//                 // printf("\n there is 1 command which is %s",arguments_for_real[1]);
//                 // printf("\n the current index is %d",current_index);

//                 char command[MAX_CMDLINE_LEN];
//                 strcpy(command,arguments_for_real[0]);
//                 execvp(command, arguments_for_real);
//             }
//             else if(how_many_command==2){
//                 char arguments_for_real_1[MAX_ARGUMENTS_PER_SEGMENT];
//                 char arguments_for_real_2[MAX_ARGUMENTS_PER_SEGMENT];

//                 if(current_index==4){ //because it wont goto the if(="|") so it odent -1 as ell
//                     strcpy(arguments_for_real_1,arguments[current_index-2]);
//                     strcpy(arguments_for_real_2,arguments[current_index-1]);
//                 }else{
//                     strcpy(arguments_for_real_1,arguments[current_index-3]);
//                     strcpy(arguments_for_real_2,arguments[current_index-2]);//arguments_for_real_1 = arguments[0]
//                 } 
//                 char *arguments_for_real[MAX_ARGUMENTS_PER_SEGMENT]={arguments_for_real_1,arguments_for_real_2, NULL};

//                 // printf("\n there is 2 command which is %s",arguments_for_real[0]);
//                 // printf("\n there is 2 command which is %s",arguments_for_real[1]);
//                 // printf("\n there is 2 command which is %s",arguments_for_real[2]);
//                 // printf("\n the current index is %d",current_index);

//                 char command[MAX_CMDLINE_LEN];
//                 strcpy(command,arguments_for_real[0]);
//                 execvp(command, arguments_for_real);
//             }
//         }
//     }
//     wait(0);
// }

// Implementation of read_tokens function
void read_tokens(char **argv, char *line, int *numTokens, char *delimiter)
{
    int argc = 0;
    char *token = strtok(line, delimiter); //delimiter is pipe character : "|"
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}