#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 2002

int main() {
  const int fd = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(SERVER_PORT);

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("bind error:");
    return 1;
  }

  socklen_t addr_len = sizeof(addr);
  getsockname(fd, (struct sockaddr *)&addr, &addr_len);
  printf("Server is listening on port %d\n", (int)ntohs(addr.sin_port));

  if (listen(fd, 1)) {
    perror("listen error:");
    return 1;
  }

  struct sockaddr_storage caddr;
  socklen_t caddr_len = sizeof(caddr);
  const int cfd = accept(fd, (struct sockaddr *)&caddr, &caddr_len);

  char buf[1024];
  recv(cfd, buf, sizeof(buf), 0);

  printf("Client message:\n    %s\n", buf);

  close(cfd);
  close(fd);
}
