/** Translate host name into IPv4
 *
 * Resolve IPv4 address for a given host name. The host name is specified as
 * the first command line argument to the program.
 *
 * Build program:
 *  $ gcc -Wall -g -o resolve <file>.cpp
 */

#include <stdio.h>
#include <stddef.h>

#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <arpa/inet.h>

void print_usage(const char* aProgramName);
char* ipv4_address_to_string(const void* address);
char* ipv6_address_to_string(const void* address);

/* HOST_NAME_MAX may be missing, e.g. if you're running this on an MacOS X
 * machine. In that case, use MAXHOSTNAMELEN from <sys/param.h>. Otherwise
 * generate an compiler error.
 */
#if !defined(HOST_NAME_MAX)
#	if defined(__APPLE__)
#		include <sys/param.h>
#		define HOST_NAME_MAX MAXHOSTNAMELEN
#	else
#		error "HOST_NAME_MAX undefined!"
#	endif
#endif

int main(int argc, char* argv[]) {
	if (argc != 2) {
		print_usage(argv[0]);
		return 1;
	}

	const char* remoteHostName = argv[1];

	const size_t hostNameMaxLength = HOST_NAME_MAX+1;
	char localHostName[hostNameMaxLength];

	if (gethostname(localHostName, hostNameMaxLength) == -1) {
		perror("gethostname(): ");
		return 1;
	}

	printf("Resolving `%s' from `%s':\n", remoteHostName, localHostName);

	struct addrinfo hints;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo* resultList;
	int error = getaddrinfo(remoteHostName, NULL, &hints, &resultList);
	if (error) {
		printf("getaddrinfo(): %s\n", gai_strerror(error));
		return 1;
	}

	for (struct addrinfo* result = resultList; result != NULL; result = result->ai_next) {
		sockaddr* socketAddress = result->ai_addr;
		sa_family_t sa_family = socketAddress->sa_family;
		if (sa_family == AF_INET) {
			sockaddr_in* inAddress = (sockaddr_in*) socketAddress;
			printf("IPv4: %s\n", ipv4_address_to_string(&(inAddress->sin_addr)));
		} else if (sa_family == AF_INET6) {
			sockaddr_in6* inAddress = (sockaddr_in6*) socketAddress;
			printf("IPv6: %s\n", ipv6_address_to_string(&(inAddress->sin6_addr)));
		}
	}

	freeaddrinfo(resultList);

	return 0;
}

void print_usage(const char* programName) {
	fprintf(stderr, "Usage: %s <hostname>\n", programName);
}

char* ipv4_address_to_string(const void* address) {
	char* dst = (char*) malloc(sizeof(char) * INET_ADDRSTRLEN);
	inet_ntop(AF_INET, address, dst, INET_ADDRSTRLEN);
	return dst;
}

char* ipv6_address_to_string(const void* address) {
	char* dst = (char*) malloc(sizeof(char) * INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, address, dst, INET6_ADDRSTRLEN);
	return dst;
}
