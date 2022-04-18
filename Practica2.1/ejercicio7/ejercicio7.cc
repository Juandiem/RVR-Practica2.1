#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <thread>

#include <iostream>

class MessageThread
{
	int fd_;

public:
	MessageThread(int fd) : fd_(fd) {}
	~MessageThread() { close(fd_); }

	void run() const noexcept
	{
		// RECEPCIÓN MENSAJE DE CLIENTE
		while (true)
		{
			// RECEPCIÓN DEL MENSAJE DEL CLIENTE //
			char buffer[80];

			ssize_t bytes = recv(fd_, buffer, sizeof(char) * 79, 0);
			if (bytes <= 0)
				break;

			// ECO DEL MENSAJE DEL CLIENTE //
			if (send(fd_, buffer, bytes, 0) == -1)
				break;
		}
	}
};

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cerr << "Se debe proveer una dirección inicial"
				  << std::endl;
		return EXIT_FAILURE;
	}

	if (argc < 3)
	{
		std::cerr << "Se debe proveer un puerto."
				  << std::endl;
		return EXIT_FAILURE;
	}

	struct addrinfo hints;
	struct addrinfo *res;

	// INICIALIZACIÓN SOCKET & BIND //

	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// res contiene la representación como sockaddr de dirección + puerto
	int rc = getaddrinfo(argv[1], argv[2], &hints, &res);

	if (rc != 0)
	{
		std::cerr << "getaddrinfo: " << gai_strerror(rc) << std::endl;
		return EXIT_FAILURE;
	}

	int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (bind(sd, res->ai_addr, res->ai_addrlen) != 0)
	{
		std::cerr << "bind: " << std::endl;
		return EXIT_FAILURE;
	}

	// Libera la información de la dirección una vez ya hemos usado sus datos:
	freeaddrinfo(res);

	// PUBLICAR EL SERVIDOR (LISTEN) //
	if (listen(sd, 16) == -1)
	{
		std::cerr << "LISTEN devolvió un error." << std::endl;
		return EXIT_FAILURE;
	}

	// GESTION DE LAS CONEXIONES AL SERVIDOR //
	struct sockaddr client_addr;
	socklen_t client_len = sizeof(struct sockaddr);

	char host[NI_MAXHOST];
	char service[NI_MAXSERV];

	while (true)
	{
		// Accepta conexiones al servidor TCP, esta llamada bloquea la ejecución:
		int sd_client = accept(sd, &client_addr, &client_len);

		// Escribe algo de información del cliente:
		getnameinfo(&client_addr, client_len, host, NI_MAXHOST, service,
					NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

		std::cout << "Conexión desde: " << host << " " << service
				  << std::endl;

		// Crea una conexión, el thread se encarga de liberar la memoria tan
		// pronto como haya terminado de trabajar:
		const auto *connection = new MessageThread(sd_client);
		std::thread([connection]() {
			connection->run();
			delete connection;

			std::cout << "Conexión finalizada." << std::endl;
		}).detach();
	}

	// Cierra el socket:
	close(sd);

	return EXIT_SUCCESS;
}