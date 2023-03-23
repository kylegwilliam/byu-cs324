// Replace PUT_USERID_HERE with your actual BYU CS user id, which you can find
// by running `id -u` on a CS lab machine.
#define USERID 1823701031

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFSIZE 64

int verbose = 0;

void print_bytes(unsigned char *bytes, int byteslen);

int main(int argc, char *argv[]) {

	// host and port are char * straight from command line

	//char* [] post =  
	unsigned int userID = htonl(USERID);
	char* host = argv[1];
	char* port = argv[2];
	int level = atoi(argv[3]);
	unsigned short seed = atol(argv[4]);
	seed = htons(seed);

	unsigned char buf[BUFSIZE];
	bzero(buf, BUFSIZE);

	// htonl atol - read man pages
	memcpy(&buf[1], &level, 1);
	memcpy(&buf[2], &userID, 4);
	memcpy(&buf[6], &seed, 2);


	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	size_t len;
	ssize_t nread;
	int hostindex;
	int af = AF_INET;

	int addr_fam;
	socklen_t addr_len;

	struct sockaddr_in remote_addr_in;
	struct sockaddr_in6 remote_addr_in6;
	struct sockaddr *remote_addr;
	char remote_addr_str[INET6_ADDRSTRLEN];
	unsigned short remote_port;

	struct sockaddr_in local_addr_in;
	struct sockaddr_in6 local_addr_in6;
	struct sockaddr *local_addr;
	char local_addr_str[INET6_ADDRSTRLEN];
	unsigned short local_port;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = af;    /* Allow IPv4, IPv6, or both, depending on
				    what was specified on the command line. */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;  /* Any protocol */


	
	s = getaddrinfo(host, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}


	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (sfd == -1)
			continue;

		addr_fam = rp->ai_family;
		addr_len = rp->ai_addrlen;
		if (addr_fam == AF_INET) {
			remote_addr_in = *(struct sockaddr_in *)rp->ai_addr;
			inet_ntop(addr_fam, &remote_addr_in.sin_addr,
					remote_addr_str, addr_len);
			remote_port = ntohs(remote_addr_in.sin_port);
			remote_addr = (struct sockaddr *)&remote_addr_in;
			local_addr = (struct sockaddr *)&local_addr_in;
		} else {
			remote_addr_in6 = *(struct sockaddr_in6 *)rp->ai_addr;
			inet_ntop(addr_fam, &remote_addr_in6.sin6_addr,
					remote_addr_str, addr_len);
			remote_port = ntohs(remote_addr_in6.sin6_port);
			remote_addr = (struct sockaddr *)&remote_addr_in6;
			local_addr = (struct sockaddr *)&local_addr_in6;
		}
		//fprintf(stderr, "Connecting to %s:%d (family: %d, len: %d)\n",
		//		remote_addr_str, remote_port, addr_fam,
		//		addr_len);

		break;
	}

	if (rp == NULL) {   /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);   /* No longer needed */


	if (sendto(sfd, buf, 8, 0, remote_addr, addr_len) <= -1) {
		fprintf(stderr, "partial/failed write\n");
	};

	unsigned char recv_buf[256];
	unsigned char TREASURE[1024];
	int value;
	value = recvfrom(sfd, recv_buf, 256, 0, remote_addr, &addr_len);

	//print_bytes(recv_buf, value);
	
	unsigned char chunk_length = recv_buf[0];
	unsigned char op_code = recv_buf[chunk_length + 1];
	unsigned short op_param; // network byte order
	unsigned int nonce;

	//printf("were starting");

	int offset = 0;

	unsigned int nonce_4;

	memcpy(&TREASURE[offset], &recv_buf[1], chunk_length);
	memcpy(&op_param, &recv_buf[chunk_length + 2], 2);
	memcpy(&nonce, &recv_buf[chunk_length + 4], 4);

	//printf("before the while loop");

	if (op_code == 1) {
		remote_addr_in.sin_port = op_param;
	}

	if (op_code == 2) {

		close(sfd);

		local_addr_in.sin_family = AF_INET; // use AF_INET (IPv4)
		local_addr_in.sin_port = op_param; // specific port // unsigned short
		local_addr_in.sin_addr.s_addr = 0; // any/all local addresses

		sfd = socket(AF_INET, SOCK_DGRAM, 0);

		local_addr = (struct sockaddr *) &local_addr_in;

		if (bind(sfd, local_addr, addr_len) < 0) {
			perror("bind()");
		}
	}

	if (op_code == 3) {

		//printf("wer are here 1");

		unsigned short m;
		m = ntohs(op_param);

		unsigned int total_value = 0;
		int int_array[m];

		struct sockaddr_in new_address;

		for (int i = 0; i < m; i++) {
			recvfrom(sfd, recv_buf, 256, 0, (struct sockaddr *) &new_address, &addr_len);
			int_array[i] = new_address.sin_port;
		}
		//printf("wer are here 2");

		int q;
		for (int i = 0; i < m; i++) {
			q = ntohs(int_array[i]);
			total_value += q;
		}
		//printf("wer are here 3");

		//printf("%d\n", m);
		//printf("%d\n", total_value);

		nonce_4 = total_value;
		nonce_4 += 1;
		nonce_4 = ntohl(nonce_4);

	}

	offset += chunk_length;

    //printf("%s \n", chunk);
	//printf("%x \n", op_code);
	//printf("%x \n", op_param);
	//printf("%x \n", nonce);

	if (chunk_length >= 127) {
		printf("There is an error in my code: Error Message ");
	}

	while (chunk_length != 0) {

		//printf("inside the while loop");

		if (op_code != 3) {
			nonce_4 = ntohl(nonce);
			nonce_4 += 1;
			nonce_4 = ntohl(nonce_4);
		}

		if (sendto(sfd, &nonce_4, 4, 0, remote_addr, addr_len) <= -1) {
			fprintf(stderr, "partial/failed write\n");
		};

		value = recvfrom(sfd, recv_buf, 256, 0, remote_addr, &addr_len);

		chunk_length = recv_buf[0];

		if (chunk_length < 0) {
			printf("The chunk length is less than 0");
			break;
		}

		if (chunk_length > 127) {
			printf("There is an error: ");
			printf("%d\n", chunk_length);
		}

		op_code = recv_buf[chunk_length + 1];

		memcpy(&TREASURE[offset], &recv_buf[1], chunk_length);
		memcpy(&op_param, &recv_buf[chunk_length + 2], 2);
		memcpy(&nonce, &recv_buf[chunk_length + 4], 4);

		//printf("This is the op code: ");
		//printf("%d\n", op_code);

		offset += chunk_length;

		if (op_code == 1) {
			remote_addr_in.sin_port = op_param;
		}

		if (op_code == 2) {

			close(sfd);

			local_addr_in.sin_family = AF_INET; // use AF_INET (IPv4)
			local_addr_in.sin_port = op_param; // specific port // unsigned short
			local_addr_in.sin_addr.s_addr = 0; // any/all local addresses

			sfd = socket(AF_INET, SOCK_DGRAM, 0);

			local_addr = (struct sockaddr *) &local_addr_in;

			if (bind(sfd, local_addr, addr_len) < 0) {
				perror("bind()");
			}
		}

		if (op_code == 3) {

			//printf("wer are here 1");

			unsigned short m;
			m = ntohs(op_param);

			unsigned int total_value = 0;
			int int_array[m];

			struct sockaddr_in new_address;

			for (int i = 0; i < m; i++) {
				recvfrom(sfd, recv_buf, 256, 0, (struct sockaddr *) &new_address, &addr_len);
				int_array[i] = new_address.sin_port;
			}
			//printf("wer are here 2");

			int q;
			for (int i = 0; i < m; i++) {
				q = ntohs(int_array[i]);
				total_value += q;
			}
			//printf("wer are here 3");

			//printf("%d\n", m);
			//printf("%d\n", total_value);

			nonce_4 = total_value;
			nonce_4 += 1;
			nonce_4 = ntohl(nonce_4);

		}

		//print_bytes(recv_buf, value);

		//printf("%x \n", op_code);
		//printf("%x \n", op_param);
		//printf("%x \n", nonce);

		
	}
	TREASURE[offset] = '\0';
	printf("%s\n", TREASURE);

	//close(sfd);
}

void print_bytes(unsigned char *bytes, int byteslen) {
	int i, j, byteslen_adjusted;

	if (byteslen % 8) {
		byteslen_adjusted = ((byteslen / 8) + 1) * 8;
	} else {
		byteslen_adjusted = byteslen;
	}
	for (i = 0; i < byteslen_adjusted + 1; i++) {
		if (!(i % 8)) {
			if (i > 0) {
				for (j = i - 8; j < i; j++) {
					if (j >= byteslen_adjusted) {
						printf("  ");
					} else if (j >= byteslen) {
						printf("  ");
					} else if (bytes[j] >= '!' && bytes[j] <= '~') {
						printf(" %c", bytes[j]);
					} else {
						printf(" .");
					}
				}
			}
			if (i < byteslen_adjusted) {
				printf("\n%02X: ", i);
			}
		} else if (!(i % 4)) {
			printf(" ");
		}
		if (i >= byteslen_adjusted) {
			continue;
		} else if (i >= byteslen) {
			printf("   ");
		} else {
			printf("%02X ", bytes[i]);
		}
	}
	printf("\n");
}
