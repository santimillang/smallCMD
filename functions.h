#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>

/* Declarations */

#define INPUTSIZE 2048

/* Functions */

int IsDir (char * name) {
	struct stat s;
	if (stat(name, &s) == -1)
		return 0;
	return S_ISDIR(s.st_mode);
}

int IsLink(char * name) {
	struct stat s;
	if (lstat(name, &s) == -1)
		return 0;
	return S_ISLNK(s.st_mode);
}

int truncateString(char * command, char * chunks[]) {
	int i = 1;
	if ((chunks[0] = strtok(command, " \n\t")) == NULL)
		return 0;
	while ((chunks[i] = strtok(NULL, " \n\t")) != NULL)
		i++;
	return i;
}

void changeDir(char * chunks[]) {
	char cwd[256];
	if (chunks[0] == NULL) {
		if (getcwd(cwd, 256) != NULL)
			printf("%s\n", cwd);
		else
			printf("chdir: '%s': %s\n", chunks[0], strerror(errno));
		return;
	}

	if (chdir(chunks[0]) == -1)
		printf("chdir: %s: %s\n", chunks[0], strerror(errno));
}

void processId(char * chunks[]) {
	if (chunks[0] != NULL && strcmp(chunks[0], "-p") == 0)
		printf("%d\n", getppid());
	else
		printf("%d\n", getpid());
}

void autores(char * chunks[]) {
	if (chunks[0] == NULL || strcmp(chunks[0], "-n") == 0)
		printf("Author 1: Millan Gonzalez, Santiago\nAuthor 2: Amatria Barral, Inaki\n");
	if (chunks[0] == NULL || strcmp(chunks[0], "-l") == 0)
		printf("Login 1: santiago.millang\nLogin 2: i.amatria\n");
}

void getDate() {
	time_t timer;
	char buffer[26];
	struct tm * tm_info;
	
	time(&timer);
	tm_info = localtime(&timer);
	
	strftime(buffer, 26, "%d/%m/%Y", tm_info);
	
	printf("%s\n", buffer);
}

void getTime() {
	time_t timer;
	char buffer[26];
	struct tm * tm_info;
	
	time(&timer);
	tm_info = localtime(&timer);
	
	strftime(buffer, 26, "%H:%M:%S", tm_info);
	
	printf("%s\n", buffer);
}

void createFile(char * chunks[]) {
	int result;
	if (chunks[0] == NULL)
		return;
    if (strcmp(chunks[0], "-d") == 0) {
		if (chunks[1] != NULL && mkdir(chunks[1], 0777) == -1) 
			printf("create: cannot create directory '%s': %s\n", chunks[1], strerror(errno));
		return;
	}
	if ((result = open(chunks[0], O_RDONLY | O_CREAT | O_EXCL, 0777)) == -1) {
		printf("create: cannot create file '%s': %s\n", chunks[0], strerror(errno));
		return;
	}
	close(result);
}

void delete(char * chunk) {
	DIR * dir;
	struct dirent * ent;
	char path[256];
	if ((dir = opendir(chunk)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			sprintf(path, "%s/%s", chunk, ent->d_name);
			if ((strcmp(ent->d_name, ".") * strcmp(ent->d_name, "..")) == 0)
				continue;
			delete(path);
		}
	} else {
		if (unlink(chunk) == -1)
			printf("delete: cannot remove '%s': %s\n", chunk, strerror(errno));
		return;
	}
	closedir(dir);
	if (rmdir(chunk) == -1)
		printf("delete: cannot remove '%s': %s\n", chunk, strerror(errno));
}

void removeFile(char * chunks[]) {
	if (chunks[0] == NULL)
		return;
	if (strcmp(chunks[0], "-r") == 0) {
		delete(chunks[1]);
		return;
	}
	if (IsDir(chunks[0])) {
		if (rmdir(chunks[0]) == -1)
			printf("delete: cannot remove: '%s': %s\n", chunks[0], strerror(errno));
		return;
	}
	if (unlink(chunks[0]) == -1)
		printf("delete: cannot remove '%s': %s\n", chunks[0], strerror(errno));	
}

char TipoFichero (mode_t m) {
	switch (m&S_IFMT) {
		case S_IFSOCK: return 's';
		case S_IFLNK: return 'l';
		case S_IFREG: return '-';
		case S_IFBLK: return 'b';
		case S_IFDIR: return 'd';
		case S_IFCHR: return 'c';
		case S_IFIFO: return 'p';
		default: return '?';
	}
}

char * ConvierteModo (mode_t m, char * permisos) {
	strcpy (permisos,"----------");
	permisos[0] = TipoFichero(m);
	if (m&S_IRUSR) permisos[1]='r';
	if (m&S_IWUSR) permisos[2]='w';
	if (m&S_IXUSR) permisos[3]='x';
	if (m&S_IRGRP) permisos[4]='r';
	if (m&S_IWGRP) permisos[5]='w';
	if (m&S_IXGRP) permisos[6]='x';
	if (m&S_IROTH) permisos[7]='r';
	if (m&S_IWOTH) permisos[8]='w';
	if (m&S_IXOTH) permisos[9]='x';
	if (m&S_ISUID) permisos[3]='s';
	if (m&S_ISGID) permisos[6]='s';
	if (m&S_ISVTX) permisos[9]='t';
	return permisos;
}

void do_query(char * name, int l) {
	struct stat ent;
   	char privileges[10], path[256], fileName[1024];
	if (lstat(name, &ent) == -1) {
		printf("query: cannot access '%s': %s\n", name, strerror(errno));
		return;
	}
	struct passwd * p = getpwuid(ent.st_uid); struct group * g = getgrgid(ent.st_gid);
	strcpy(fileName, name);
	if (IsLink(name)) {
		realpath(fileName, path); strcat(fileName, " -> "); strcat(fileName, path);
	}
	ConvierteModo(ent.st_mode, privileges);
	if (l)
		printf("%8ld  %s %4ld %7s %7s %7ld %.16s %s\n", ent.st_ino, ConvierteModo(ent.st_mode, privileges),
		ent.st_nlink, p == NULL ? "unknown" : p->pw_name, g == NULL ? "unknown" : g->gr_name, ent.st_size,
		ctime(&ent.st_mtime), fileName);
	else
		printf("%7ld %s\n", ent.st_size, fileName);
}

void queryFiles(char * chunks[]) {
	while (chunks[0] != NULL)
		do_query(chunks++[0], 1);
}

void do_list(char * name, int l, int h, int r) {
	DIR * dir;
	char buf[256], path[1024];
	struct dirent * ent;
	struct list {
		struct list * next; char * content;
	} * linkedList = NULL;
	if ((dir = opendir(name)) == NULL) {
		printf("list: cannot access '%s': %s\n", name, strerror(errno));
		return;
	}
	realpath(name, buf);
	if (r) printf("%s:\n", buf);
	while ((ent = readdir(dir)) != NULL) {
		if (!h && ent->d_name[0] == '.') continue;
		if ((strcmp(ent->d_name, ".") * strcmp(ent->d_name, "..")) == 0) continue;
		sprintf(path, "%s/%s", name, ent->d_name);
		if (IsDir(path) && !IsLink(path) && r) {
			struct list * aux; aux = (struct list *) malloc(sizeof(struct list));
			aux->content = (char *) malloc(sizeof(path)); strcpy(aux->content, path);
			aux->next = linkedList; linkedList = aux;
		}
		do_query(path, l);
	}
	closedir(dir);
	while (linkedList != NULL) {
		struct list * aux = linkedList;
		printf("\n"); do_list(aux->content, l, h, r);
		linkedList = linkedList->next; free(aux->content); free(aux);
	}
}

void listFiles(char * chunks[]) {
	int hidden = 0, recursive = 0, longlist = 1;
	int i;
	for (i = 0; chunks[i] != NULL; i++)
		if (!strcmp(chunks[i], "-h")) hidden=1;
		else if (!strcmp(chunks[i], "-n")) longlist = 0;
		else if (!strcmp(chunks[i], "-r")) recursive = 1;
		else break;
   	if (chunks[i] == NULL)
		do_list(".", longlist, hidden, recursive);
   	for ( ; chunks[i] != NULL; i++)
		if (IsDir(chunks[i]))
			do_list(chunks[i], longlist, hidden, recursive);
		else 
			do_query(chunks[i], longlist);
}

void parseCommand(char * command) {
	int i;
	char * chunks[INPUTSIZE / 2];
	i = truncateString(command, chunks);
	/* #chunks = 0 => Do nothing */
	if (i == 0)
		return;
	if ((strcmp(chunks[0], "exit") * strcmp(chunks[0], "fin") * strcmp(chunks[0], "end")) == 0)
		exit(0);
	else if (strcmp(chunks[0], "chdir") == 0)
		changeDir(chunks + 1);
	else if (strcmp(chunks[0], "pid") == 0)
		processId(chunks + 1);
	else if (strcmp(chunks[0], "autores") == 0)
		autores(chunks + 1);
	else if (strcmp(chunks[0], "fecha") == 0)
		getDate();
	else if (strcmp(chunks[0], "hora") == 0)
		getTime();
	else if (strcmp(chunks[0], "create") == 0)
		createFile(chunks + 1);
	else if (strcmp(chunks[0], "delete") == 0)
		removeFile(chunks + 1);
	else if (strcmp(chunks[0], "query") == 0)
		queryFiles(chunks + 1);
	else if (strcmp(chunks[0], "list") == 0)
		listFiles(chunks + 1);
	else
		printf("%s: command not found\n", chunks[0]);
}
