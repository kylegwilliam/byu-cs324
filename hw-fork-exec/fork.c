#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>

int main(int argc, char *argv[]) {
	int pid;
	int pipefd[2];

	printf("Starting program; process has pid %d\n", getpid());

	FILE *fp = fopen("fork-output.txt", "w");
	fputs("BEFORE FORK\n", fp);
	fflush(fp);

	pipe(pipefd);
	if ((pid = fork()) < 0) {
		fprintf(stderr, "Could not fork()");
		exit(1);
	}

	/* BEGIN SECTION A */

	printf("Section A;  pid %d\n", getpid());
	//sleep(30);

	/* END SECTION A */
	if (pid == 0) {
		/* BEGIN SECTION B */

		printf("Section B\n");
		
		close(pipefd[0]);
		write(pipefd[1], "hello from Section B\n", 20);


		sleep(5);
		dup2;
		int int_file = fileno(fp);
		fprintf(fp, "SECTION B (%d)\n", int_file);
		

		//sleep(30);
		//sleep(30);
		//printf("Section B done sleeping\n");

		exit(0);

		char *newenviron[] = { NULL };

		printf("Program \"%s\" has pid %d. Sleeping.\n", argv[0], getpid());
		sleep(30);

		if (argc <= 1) {
			printf("No program to exec.  Exiting...\n");
			exit(0);
		}

		printf("Running exec of \"%s\"\n", argv[1]);
		execve(argv[1], &argv[1], newenviron);
		printf("End of program \"%s\".\n", argv[0]);

		/* END SECTION B */
	} else {
		/* BEGIN SECTION C */

		printf("Section C\n");

		char my_str[70];
		close(pipefd[1]);
		int value = read(pipefd[0], my_str, 70);
		my_str[value] = 0;
		printf("%s\n", my_str);

		dup2;
		int int_file = fileno(fp);
		fprintf(fp, "SECTION C (%d)\n", int_file);
		fclose(fp);

		//wait(NULL);
		//sleep(30);
		//printf("Section C done sleeping\n");

		exit(0);

		/* END SECTION C */
	}
	/* BEGIN SECTION D */

	printf("Section D\n");
	//sleep(30);

	/* END SECTION D */
}

