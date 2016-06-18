John Nelson - jpnelson

Project 3 Submission

Contents:

-README.txt
-problem_explanation.txt
-mutex_output.txt - the output for a test on the mutex solution
-semaphore_output.txt - the output for a test on the semaphore solution

Mutex Folder (folder containing the mutex implementation of the problem):
-Makefile
-tooManyCooks.c	-source code and main function
-linkedList.c		-linkedList source code
-linkedList.h		-linkedList header

Semaphore Folder (folder containing the semaphore implementation of the problem):
-Makefile
-tooManyCooks.c	-source code and main function
-linkedList.c		-linkedList source code
-linkedList.h		-linkedList header

For this project, I chose to solve Problem 2: Too Many Cooks.
The wait time between orders coming in is 1-10 seconds.
The random seed is currently set at 0 in main.

To run each program, navigate to the solution's folder and run:

make

Then run:

./tooManyCooks

The program will output the events of the kitchen.
