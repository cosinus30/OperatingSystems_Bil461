#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Soru1.h"
#include <sys/types.h>
#include <unistd.h>
#include<sys/wait.h>

int main(int argc, char  *argv[]){
	if (argc != 3){
		printf("This program takes two arguments as input.\n");
		exit(0);
	}

	char *inputFileName = argv[1];
	int whichPartToBeRun = atoi(argv[2]);
	FILE *fileToBeProcessed = fopen(inputFileName,"r");
	if(fileToBeProcessed == NULL){
		printf("File could not be opened.");
		exit(0);
	}

	char historyOfCommands[findTheNumberOfCommands(fileToBeProcessed)][128];

	fclose(fileToBeProcessed);

	fileToBeProcessed = fopen(inputFileName,"r");
	char line[128];
	printf("%s, %d\n", inputFileName,whichPartToBeRun);
	int commandIterator = 0;

	while (fgets(line, sizeof(line),fileToBeProcessed) != NULL){
		if (line[strlen(line) - 1] == '\n'){ 	//trim the '\n' because it causes problem while parsing. No problem is needed.
			line[strlen(line) - 1] = '\0';
		}
		strcpy(historyOfCommands[commandIterator],line);


	//	history icin ayri bir metod yazmak yerine mainin icinde isleme koydum.
		if(strcmp("history",line) == 0){
			printf("Command will be executed: %s\n", line);
			int j = 0;
			for (int k = 0; k <= commandIterator; k++){
				printf("%s,%d\n", historyOfCommands[k],k);
			}
      commandIterator = commandIterator + 1;
			continue;
		}
    if((strstr(line,"cd")) != NULL){
      printf("Command will be executed: %s\n", line);
      char *parsedString[16];
      parseSpace(line,parsedString);
      chdir(parsedString[1]);
      commandIterator = commandIterator + 1;
      continue;
    }



		commandIterator = commandIterator + 1;

		switch (whichPartToBeRun) {
			case 1:
				part1(line);
				break;
			case 2:
				part2(line);
				break;
			case 3:
				part3(line);
				break;
			case 4:
				part4(line);
				break;
			default:
				printf("You may enter 1,2,3 or 4 as second argument\n");
		}
	}
	printf("KOMUTLAR BITTI\n");
	return 0;
}

// odevin 1.sorusuna ait kisim.
int part1(char *line){
	char *parsedString[8];		//komutlari kelimelere ayirip saklamamiz gerekiyor. (whatis ls ==> {[1]whatis, [2]ls} gibi)
	printf("Command will be executed: %s\n", line);
	parseSpace(line, parsedString);

	pid_t pid;
	if ((pid = fork()) < 0){	//fork gerekli keza execvp bir satir icin calistirildiktan sonra yeni satira gecip isleme devam etmemiz gerekiyor.
		printf("ERROR: Fork failed!\n");
		exit(1);
	}
	else if(pid == 0){		//child process is running
		if(execvp(parsedString[0], parsedString) < 0){
			printf("ERROR: execvp failed.\n");	//this line should not be executed.
			exit(1);
		}
	}
	else{	//parent process is running.
		wait(NULL);		//wait for child.
	}
	return 0;
}

// odevin 2.sorusuna ait kisim.
int part2(char *line){
	char *parsedString[8];			//Kolaylik olsun diye bir satirda sekizden fazla komut bulunmayacagini varsiyoyurum. Numara istenildigi gibi degistirilebilir.
	int howManyProcess = findTheNumberOfSemiColon(line) + 1;		//kac tane fork atmam gerektigini olcmem icin.

	//paralel calistirilacak her bir islemi ayirt et.
	int k = 0;
	char *ptr1 = strtok(line,";");
	while(ptr1 != NULL){
		parsedString[k] = ptr1;
		ptr1 = strtok(NULL,";");
		k++;
	}

	pid_t pid[howManyProcess];
	int i;
	for(i = 0; i < howManyProcess; i++){
		if((pid[i] = fork()) == 0){		//islemlerin bir kismini (parsing gibi) forkladiktan sonra yapmakta beis yok keza childlar da ayni islemleri yapma kapasitesine sahip.
			char *forAProcess[8];
			int counter = 0;
			parseSpace(parsedString[i],forAProcess);
			printf("Command will be executed: ");
			while (forAProcess[counter] != NULL){
				printf("%s ", forAProcess[counter]);
				counter++;
			}
			printf("\n");
			execvp(forAProcess[0],forAProcess);
			_exit(1);		//signal safety is important.
		}
		if(pid[i] < 0){
			printf("ERROR: Fork failed\n");
			_exit(1);		//signal safety is important.
		}
	}
	for (i = 0; i < howManyProcess; i++){
		if (pid[i] > 0){ //means it is parent
			wait(NULL);				//wait for children.
		}
	}
	return 0;
}



int part3(char* line){
	char *parsedString[8];			//Kolaylik olsun diye bir satirda sekizden fazla komut bulunmayacagini varsiyoyurum. Numara istenildigi gibi degistirilebilir.
	int howManyProcess = findTheNumberOfPipes(line) + 1;

	int k = 0;
	char *ptr1 = strtok(line,"|");
	while(ptr1 != NULL){
		parsedString[k] = ptr1;
		ptr1 = strtok(NULL,"|");
		printf("Command will be executed: %s\n", parsedString[k]);
		k++;
	}

	int p[2];
	int fd_in = 0;
	pid_t pid;
	int i = 0;

	while (parsedString[i] != NULL){
		pipe(p);
		if ((pid = fork()) == -1){		//FAILED
			_exit(1);
		}
		else if(pid == 0){
			dup2(fd_in, 0);		//Now piping is starting. Change the standart input.
			if(parsedString[i+1] != NULL)
				dup2(p[1],1);
			close(p[0]);
			int counter = 0;
			char *forAProcess[8];
			parseSpace(parsedString[i],forAProcess);
			execvp(forAProcess[0],forAProcess);
			_exit(1);
		}
		else{				//Parents always wait... SAD...
			wait(NULL);
			close(p[1]);
			fd_in = p[0];
			i++;
		}
	}
	return 0;
}

int part4(char* line){
	int numberOfPipes = findTheNumberOfPipes(line);
	int numberOfSemiColon = findTheNumberOfSemiColon(line);
	if (numberOfSemiColon > 0){
		part2(line);
  }
  else if (numberOfPipes > 0){
		part3(line);
  }
	else{
		part1(line);
  }
	return 0;
}

//A method for finding the number of commands on the file.
//It is needed because of declaration for historyOfCommands list.
int findTheNumberOfCommands(FILE *fileToBeProcessed){
	int numberOfCommands = 0;
	char chunk[128];
	while (fgets(chunk, sizeof(chunk),fileToBeProcessed) != NULL){
		numberOfCommands += 1;
		int numberOfSemiColon = 0;
		int numberOfPipe = 0;
		for (int i = 0; i < strlen(chunk); i++){
			if(chunk[i] == ';')
				numberOfSemiColon += 1;
		}
		numberOfCommands += numberOfSemiColon;

		for (int i = 0; i < strlen(chunk); i++){
			if(chunk[i] == '|')
				numberOfPipe += 1;
		}
		numberOfCommands += numberOfPipe;
	}
	return numberOfCommands;
}



int findTheNumberOfSemiColon(char *line){
	int numberOfSemiColon = 0;
	for (int i = 0; i < strlen(line); i++){
		if(line[i] == ';')
			numberOfSemiColon += 1;
	}
	return numberOfSemiColon;
}

int findTheNumberOfPipes(char *line){
	int numberOfPipes = 0;
	for (int i = 0; i < strlen(line); i++){
		if(line[i] == '|')
			numberOfPipes += 1;
	}
	return numberOfPipes;
}


void parseSpace (char *string, char* parsedString[]){
	int i = 0;
	char *ptr = strtok(string, " ");
	while(ptr != NULL){
		parsedString[i] = ptr;
		ptr = strtok(NULL, " ");
		i++;
	}
	parsedString[i] = NULL; //execvp icin sonuncuyu null yapmam gerekli.
}
