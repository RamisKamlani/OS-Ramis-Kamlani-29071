#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

char* fmtname(char *path) {
  static char buf[DIRSIZ+1];
  char *p;
  for(p=path+strlen(path); p >= path && *p != '/'; p--);
  p++;
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  buf[strlen(p)] = 0;
  return buf;
}

int use_exec = 0;        // flag if -exec was given
char *exec_argv[MAXARG]; // store exec command

void run_exec(char *file) {
  int pid = fork();
  if(pid == 0){
    // child: copy exec_argv and append file
    char *argv[MAXARG];
    int i = 0;
    while(exec_argv[i] != 0 && i < MAXARG-1){
      argv[i] = exec_argv[i];
      i++;
    }
    argv[i++] = file;
    argv[i] = 0;
    exec(argv[0], argv);
    fprintf(2, "exec %s failed\n", argv[0]);
    exit(1);
  } else {
    wait(0);
  }
}

void find(char *path, char *name) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type) {
  case T_FILE:
    if(strcmp(fmtname(path), name) == 0) {
      if(use_exec)
        run_exec(path);
      else
        printf("%s\n", path);
    }
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("find: cannot stat %s\n", buf);
        continue;
      }
      find(buf, name);
    }
    break;
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  if(argc < 3){
    fprintf(2, "usage: find <path> <filename> [-exec cmd ...]\n");
    exit(1);
  }

  // check if "-exec" present
  for(int i = 3; i < argc; i++){
    if(strcmp(argv[i], "-exec") == 0){
      use_exec = 1;
      int j = 0;
      for(int k = i+1; k < argc && j < MAXARG-1; k++){
        exec_argv[j++] = argv[k];
      }
      exec_argv[j] = 0;
      break;
    }
  }

  find(argv[1], argv[2]);
  exit(0);
}
