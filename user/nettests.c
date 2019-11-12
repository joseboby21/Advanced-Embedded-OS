#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

//
// send a UDP packet to the localhost (outside of qemu),
// and receive a response.
//
static void
ping(uint16 sport, uint16 dport)
{
  int fd;
  char obuf[13] = "hello world!";
  uint32 dst;

  // 10.0.2.2, which qemu remaps to the external host,
  // i.e. the machine you're running qemu on.
  dst = (10 << 24) | (0 << 16) | (2 << 8) | (2 << 0);

  // you can send a UDP packet to any Internet address
  // by using a different dst.
  
  if((fd = connect(dst, sport, dport)) < 0){
    fprintf(2, "ping: connect() failed\n");
    exit(1);
  }

  if(write(fd, obuf, sizeof(obuf)) < 0){
    fprintf(2, "ping: send() failed\n");
    exit(1);
  }

  char ibuf[128];
  int cc = read(fd, ibuf, sizeof(ibuf));
  if(cc < 0){
    fprintf(2, "ping: recv() failed\n");
    exit(1);
  }

  close(fd);
  if (strcmp(obuf, ibuf) || cc != sizeof(obuf)){
    fprintf(2, "ping didn't receive correct payload\n");
    exit(1);
  }
}

int
main(int argc, char *argv[])
{
  int i, ret;
  uint16 dport;

  if (argc < 2){
    fprintf(2, "missing port number\n");
    fprintf(2, "provide the port number printed by 'make server' as an arg\n");
    exit(1);
  }

  dport = atoi(argv[1]);

  printf("testing single-process pings: ");
  for (i = 0; i < 100; i++)
    ping(2000, dport);
  printf("OK\n");

  printf("testing multi-process pings: ");
  for (i = 0; i < 10; i++){
    int pid = fork();
    if (pid == 0){
      ping(2000 + i + 1, dport);
      exit(0);
    }
  }
  for (i = 0; i < 10; i++){
    wait(&ret);
    if (ret != 0)
      exit(1);
  }
  printf("OK\n");

  printf("all tests passed.\n");
  exit(0);
}
