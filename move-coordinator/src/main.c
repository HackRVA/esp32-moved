
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include "psmove_protocol.h"

#define VERSION "0.0.1"

#define DEFAULT_GAME_PORT 17777
#define DEFAULT_GAME_ADDRESS "127.0.0.1"
#define GAME_BUFFER_SIZE 9 /* Size of packets from the game */
#define CONTROLLER_BUFFER_SIZE 50 /* Size of packets from the controllers */
#define DEFAULT_CONTROLLER_FILE "/etc/jsjoust/controllers.conf"

#define REQUEST_CONTROLLER_TYPE_IDX 0
#define REQUEST_CONTROLLER_ID_IDX 1

typedef struct DirectorContext {
  int verbose;
  int game_socket;
  struct sockaddr_in game_addr;
  int controller_socket;
  int num_controllers;
  struct sockaddr_in *controller_addrs;
} DirectorContext;

int director_context_init(DirectorContext* ctx);
int director_context_cleanup(DirectorContext* ctx);
int director_context_start_game_socket(DirectorContext* ctx);
int director_context_start_controller_socket(DirectorContext* ctx);
int director_context_set_game_address(DirectorContext *ctx, const char* address);
int director_context_load_controller_addresses(DirectorContext *ctx, const char* file);
int director_context_run(DirectorContext* ctx);

int interrupted;
static void sigint_handler(int info) {
  interrupted = 1;
}

int main(int argc, char** argv) {
  int opt_char;
  char* controller_file;
  DirectorContext ctx;

  interrupted = 0;

  /* Capture sigint so that I can clean things up properly */
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = sigint_handler;
  sigaction(SIGINT, &action, NULL);

  /* Set defaults on the context */
  controller_file = DEFAULT_CONTROLLER_FILE;
  director_context_init(&ctx);

  /* Load the configuration */
  while(opt_char = getopt(argc, argv, "hvg:p:c:") != -1) {
    switch(opt_char) {
    case 'h':
      printf("Move Coordinator " VERSION " -- Snapshot\n");
      printf("-v, print what's going on verbosely\n");
      printf("-h, print this help information\n");
      printf("-g game_ip, set the IP address the game is using\n");
      printf("-c controller_file, sets the file containing IP addresses of the controllers\n");
      return 0;
      break;
    case 'v':
      ctx.verbose = 1;
      break;
    case 'c':
      /* TODO Set the path to the controller file */
      /* controller_file = ?; */
      break;
    case 'g':
      /* TODO Assign the game address */
      director_context_set_game_address(&ctx, DEFAULT_GAME_ADDRESS);
      break;
    }
  }

  /* Load from the controller file */
  if(director_context_load_controller_addresses(&ctx, DEFAULT_CONTROLLER_FILE)) {
    director_context_cleanup(&ctx);
    return 1;
  }
  /* Connect to the game */
  if(director_context_start_game_socket(&ctx)) {
    director_context_cleanup(&ctx);
    return 1;
  }
  /* Connect to the controllers */
  if(director_context_start_controller_socket(&ctx)) {
    director_context_cleanup(&ctx);
    return 1;
  }

  if(director_context_run(&ctx)) {
    director_context_cleanup(&ctx);
    return 1;
  }

  director_context_cleanup(&ctx);
  return 0;
}

int director_context_start_game_socket(DirectorContext* ctx) {
  if((ctx->game_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    printf("Failed while opening socket for the game\n");
    return 1;
  }

  if(bind(ctx->game_socket, (struct sockaddr*) &ctx->game_addr, sizeof(ctx->game_addr)) == -1) {
    printf("Failed while binding socket for the game: %s\n", strerror(errno));
    return 1;
  }

  /* Continue onward */
  return 0;
}

int director_context_start_controller_socket(DirectorContext* ctx) {
  ssize_t nread;
  char buffer[CONTROLLER_BUFFER_SIZE];

  /* Create socket for controllers */
  if((ctx->controller_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    printf("Failed creating socket for controllers\n");
    return 1;
  }

  /* onward */
  return 0;
}

int director_context_init(DirectorContext* ctx) {
  ctx->verbose = 0;
  ctx->game_socket = -1;
  ctx->game_addr.sin_family = AF_INET;
  ctx->game_addr.sin_port = htons(DEFAULT_GAME_PORT);
  director_context_set_game_address(ctx, DEFAULT_GAME_ADDRESS);
  ctx->num_controllers = 0;
  ctx->controller_addrs = NULL;
  return 0;
}

int director_context_set_game_address(DirectorContext* ctx, const char* address) {
  /* TODO use getaddrinfo instead */
  inet_aton(address, &ctx->game_addr.sin_addr);
}

int director_context_load_controller_addresses(DirectorContext* ctx, const char* controller_file) {
  FILE* f;
  char *lineptr;
  size_t n;
  ssize_t nread;
  if(!(f = fopen(controller_file, "r"))) {
    printf("Failed opening controller file\n");
    return 1;
  }
  lineptr = NULL;
  n = 0;
  ctx->num_controllers = 0;
  ctx->controller_addrs = NULL;
  while(nread = getline(&lineptr, &n, f) != -1) {
    if(nread == 1) /* Quits at the first empty line */
      break;
    /* Remove the newline character */
    lineptr[nread - 1] = 0;
    /* Add a new controller */
    int controller_idx = ctx->num_controllers;
    ctx->num_controllers++;
    ctx->controller_addrs = realloc((void*) ctx->controller_addrs, ctx->num_controllers * sizeof(struct sockaddr_in));
    printf("Read controller address: %s, %i\n", lineptr, n);
    /* TODO use getaddrinfo instead */
    if(inet_aton(lineptr, &ctx->controller_addrs[controller_idx].sin_addr) == 0) {
      printf("Failed parsing controller address: %s\n", lineptr);
      free(lineptr);
      return 1;
    }
    /* Uses ephemeral ports */
    ctx->controller_addrs[controller_idx].sin_family = AF_INET;
    ctx->controller_addrs[controller_idx].sin_port = 0;
    free(lineptr);
    lineptr = NULL;
    n = 0;
  }
  /* Need to free lineptr even after failures */
  free(lineptr);
  fclose(f);
}

int director_context_cleanup(DirectorContext* ctx) {
  if(ctx->game_socket != -1)
    close(ctx->game_socket);
  ctx->game_socket = -1;
  ctx->num_controllers = 0;
  free(ctx->controller_addrs);
  ctx->controller_addrs = NULL;
  return 0;
}

/* Things need to be fairly narrowly controlled, state-wise */
typedef void (*CommunicationState)(
    DirectorContext* ctx,
    char* request_buffer,
    char* response_buffer,
    void* next);

static void read_from_game(
    DirectorContext* ctx,
    char* request_buffer,
    char* response_buffer,
    void* next);
static void read_from_controller(
    DirectorContext* ctx,
    char* request_buffer,
    char* response_buffer,
    void* next);
static void write_to_game(
    DirectorContext* ctx,
    char* request_buffer,
    char* response_buffer,
    void* next);
static void write_to_controller(
    DirectorContext* ctx,
    char* request_buffer,
    char* response_buffer,
    void* next);

static void read_from_game(
    DirectorContext* ctx,
    char* request_buffer,
    char* response_buffer,
    void* next) {
  /* game socket is ready to read */
  if(recvfrom(ctx->game_socket, &request_buffer, MOVED_SIZE_REQUEST, 0, NULL, NULL) == -1) {
    printf("Error while reading from game: %s\n", strerror(errno));
    return;
  }
  /* Check the game buffer for a "COUNT" request */
  unsigned char request_type = request_buffer[REQUEST_CONTROLLER_TYPE_IDX];
  if(request_type == MOVED_REQ_COUNT_CONNECTED) {
    /* Service the count request yourself */
    /* Set the data read from the controller */
    memset(&response_buffer, 0, 50);
    response_buffer[0] = ctx->num_controllers;
    next = write_to_game;
  } else {
    next = write_to_controller;
  }
  /* Send the request onward to the controller */
}

static void write_to_controller(
    DirectorContext* ctx,
    char* request_buffer,
    char* response_buffer,
    void* next) {
  /* Map the controller to write to */
  struct sockaddr_in* controller_addr =
    &ctx->controller_addrs[request_buffer[REQUEST_CONTROLLER_ID_IDX]];

  /* Write the request buffer onward */
  sendto(
    ctx->controller_socket,
    &request_buffer,
    MOVED_SIZE_REQUEST,
    0,
    (struct sockaddr*) controller_addr,
    sizeof(controller_addr));

  switch(request_buffer[REQUEST_CONTROLLER_TYPE_IDX]) {
  case MOVED_REQ_COUNT_CONNECTED: /* Technically shouldn't happen */
    printf("Something bad happened, requested count from controller!\n");
  case MOVED_REQ_READ:
    /* Listen for a response */
    next = (void*) read_from_controller;
    break;
  default:
  case MOVED_REQ_WRITE:
  case MOVED_REQ_SERIAL:
    /* No response expected,  */
    next = (void*) read_from_game;
    break;
  }
}

static void write_to_game(
    DirectorContext* ctx,
    char* request_buffer,
    char* response_buffer,
    void* next) {
  sendto(
    ctx->game_socket,
    response_buffer,
    MOVED_SIZE_READ_RESPONSE,
    0,
    (struct sockaddr*) &ctx->game_addr,
    sizeof(ctx->game_addr));
  next = (void*) read_from_game;
}

static void read_from_controller(
    DirectorContext* ctx,
    char* request_buffer,
    char* response_buffer,
    void* next) {
  recvfrom(
    ctx->controller_socket,
    response_buffer,
    MOVED_SIZE_READ_RESPONSE,
    0,
    NULL,
    NULL);
  /* Should pass data on without changes */
  next = (void*) write_to_game;
}

int director_context_run(DirectorContext* ctx) {
  /* Wait for messages to be received from the game */
  char request_buffer[MOVED_SIZE_REQUEST];
  char response_buffer[MOVED_SIZE_READ_RESPONSE];
  struct sockaddr_in active_controller;
  struct timeval tout;
  CommunicationState state;

  state = read_from_game;
  /* Things need to be fairly narrowly controlled, state-wise */
  while(!interrupted) {
    state(ctx, request_buffer, response_buffer, (void*) &state);
  }
  /* Successfully exits */
  return 0;
}

