For redirection part,
instead of typing the command on the console, the input can be redirected from a text file.
Moreover the  output can also be redirected to a text file. 

For Multi-level pipe part,
you can run the line of code in sequence,
for example you can type "$> ls | sort -r | sort | sort -r | sort | sort -r | sort | sort -r "

To compile the file, please type "gcc -std=c99 -o myshell myshell.c"
and run it by "chmod u+x myshell.exe"
To use the input file, please type "./myshell_sample < in01.txt"
