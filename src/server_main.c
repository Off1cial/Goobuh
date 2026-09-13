#include "engine/server/server.h"
#include "platform/time.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

double accumulator = 0.0;
double previous = 0.0f;



int main(void){

  if (!sv_init()){
    printf("Server failed to initialise..\n");
    return 1;
  }


  previous = plt_timemillis();

  char hostname[256];
  char hostip[256];

  gethostname(hostname, 256);
  struct hostent *host = gethostbyname(hostname);
  strcpy(hostip, inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));

  printf("Server started (%0.1fHz): %s:%d\n", sv_tickrate, hostip, sv_main.localaddr.port);


  while(1){
    double now = plt_timemillis();
    double dt = (now - previous) / 1000;
    previous = now;
    if (dt > 0.25)
      dt = 0.25;
    accumulator += dt;

    while(accumulator >= (1.0f / sv_tickrate)){
      sv_think();
      printf("think\n");
      accumulator -= (1.0f / sv_tickrate);
    }
  }

  return 0;
}
