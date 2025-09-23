#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

char *seps = " -\r\t\n./,";

int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "Usage: sixfive <file>...\n");
    exit(1);
  }

  char buf[1];           // to read one char at a time
  char numbuf[32];       // buffer for digits
  int fd, n, idx;

  for(int i = 1; i < argc; i++){
    fd = open(argv[i], O_RDONLY);
    if(fd < 0){
      fprintf(2, "sixfive: cannot open %s\n", argv[i]);
      exit(1);
    }

    idx = 0; // reset digit buffer index

    while((n = read(fd, buf, 1)) > 0){
      char c = buf[0];

      if(strchr(seps, c)){
        // Separator → check if we have a number
        if(idx > 0){
          numbuf[idx] = '\0';       // terminate string
          int val = atoi(numbuf);   // convert to int
          if(val % 5 == 0 || val % 6 == 0){
            printf("%d\n", val);
          }
          idx = 0;  // reset buffer
        }
      } else if(c >= '0' && c <= '9'){
        // digit → store in buffer
        if(idx < sizeof(numbuf)-1){
          numbuf[idx++] = c;
        }
      } else {
        // non-digit non-separator → treat as separator
        if(idx > 0){
          numbuf[idx] = '\0';
          int val = atoi(numbuf);
          if(val % 5 == 0 || val % 6 == 0){
            printf("%d\n", val);
          }
          idx = 0;
        }
      }
    }

    // handle last number if file ends without separator
    if(idx > 0){
      numbuf[idx] = '\0';
      int val = atoi(numbuf);
      if(val % 5 == 0 || val % 6 == 0){
        printf("%d\n", val);
      }
    }

    close(fd);
  }

  exit(0);
}
