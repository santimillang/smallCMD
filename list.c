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

int main (int argc, char * * argv) {
	listFiles(argv + 1);
	return 0;
}
