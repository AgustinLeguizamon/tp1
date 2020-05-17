#define _POSIX_C_SOURCE 201112L
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include "common_socket.h"	

#define ERROR -1
#define BACKLOG 10; //cantidad maxima de solicitudes en espera

int socket_create(socket_t *self){
	self->socket_file_descriptor = -1;
	return 0;
}

int socket_destroy(socket_t *self){
	socket_shutdown(self, SHUT_RDWR);
	return close(self->socket_file_descriptor);
}

int socket_bind_and_listen(socket_t *self, const char* service){
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; //TCP
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	struct addrinfo *result;
	getaddrinfo(NULL, service, &hints, &result);

    struct addrinfo *rp = result;
     
	bool success = false;
	while (!success && rp != NULL){
		int socket_file_descriptor = socket(rp->ai_family, rp->ai_socktype, 
			rp->ai_protocol);
	
		if(bind(socket_file_descriptor, rp->ai_addr, rp->ai_addrlen) == 0){
			success = true;

			int backlog = BACKLOG;
			listen(socket_file_descriptor, backlog); 

			self->socket_file_descriptor = socket_file_descriptor;
		} else {	
			rp = rp->ai_next;
            close(socket_file_descriptor);
        }
	}

	freeaddrinfo(result);

	return 0;
}

int socket_accept(socket_t *self, socket_t *server_socket){
	int acep_file_descriptor = self->socket_file_descriptor;
	socklen_t peer_addr_size = sizeof(struct sockaddr_in);

	struct sockaddr_in peer_addr;
	server_socket->socket_file_descriptor = accept(acep_file_descriptor, 
		(struct sockaddr *) &peer_addr, &peer_addr_size);

	return 0;
}

int socket_connect(socket_t *self, const char* host_name, const char* service){
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; //TCP
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	
	struct addrinfo *result;
	getaddrinfo(host_name, service, &hints, &result);

    struct addrinfo *rp = result;

	bool success = false;
	while (!success && rp != NULL){
		int socket_file_descriptor = socket(rp->ai_family, rp->ai_socktype, 
			rp->ai_protocol);
	
		if(connect(socket_file_descriptor, rp->ai_addr, rp->ai_addrlen) != ERROR){
			success = true;
			self->socket_file_descriptor = socket_file_descriptor;
		} else {	
			rp = rp->ai_next;
            close(socket_file_descriptor);
        }
	}

	freeaddrinfo(result);

	return 0;
}

int socket_send(socket_t *self, const char* buffer, size_t length){
	ssize_t n_send = 1;
	int bytes_sent = 0;

	while (bytes_sent < length && n_send > 0){
		n_send = send(self->socket_file_descriptor, buffer, length, MSG_NOSIGNAL);
		bytes_sent += n_send;
	}
	
	return n_send;
}


int socket_receive(socket_t *self, char* buffer, size_t length){
	ssize_t n_recv = 1;
	int bytes_recv = 0;

	while (bytes_recv < length && n_recv > 0){
		n_recv = recv(self->socket_file_descriptor, buffer, length, 0);
		bytes_recv += n_recv;
	}	
	return n_recv;
}

int socket_shutdown(socket_t *self, int channel){
	return shutdown(self->socket_file_descriptor, channel);
}
