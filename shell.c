/* Authors
 *
 * 	Author 1: Millan Gonzalez, Santiago	Login 1: santiago.millang
 * 	Author 2: Amatria Barral, Inaki	Login 2: i.amatria
 */

#include "functions.h"

/* Commands
 *
 *	autores [-l|-n] Prints the names and logins of the program authors.
 *			autores -l prints only the logins
 *			autores -n prints the names
 *
 *	pid [-p]	Prints the pid of the process executing the shell.
 *			pid -p prints the pid of its parent process
 *
 *	chdir [direct]	Changes the current working directory of the shell
 *			to "direct". When invoking without arguments it 
 *			prints the current working directory of the shell.
 *
 *	fecha		Prints the current date.
 *
 *	hora		Prints the current time.
 *
 *	fin/end/exit	Ends the shell.
 *
 *	create [-d]	Creates a file or directory in the file system.
 *	
 *	delete [-r]	Deletes a file or a directory.
 *
 *	query		Gives info on the files and/or directories supplied as its arguments.
 *
 *	list [-n|-h|-r]	Lists the directories and/or files supplied to it as a command line
 *			arguments. -n only name and size. -r directories listed recursively.
 *			-h shows hidden. If no names are given cwd will be printed.
 */

/* Main */

int main( void ) {
	char input[INPUTSIZE];
	/* Main Loop */
	while (1) {
		/* Print prompt */
		printf("$ ");
		/* Read from stdin */
		fgets(input, INPUTSIZE, stdin);
		/* Process command */
		parseCommand(input);
	}
	return 1;
}
