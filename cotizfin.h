#ifndef COTIZADOR_H
#define COTIZADOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

/* --- Constantes y Configuracion --- */
#define PUERTO 32323
#define BACKLOG 10 
#define IPSERVIDOR "127.0.0.1"
#define MENSUAL 200 

// Constantes de Negocio
#define KMAIRE   10.0
#define KMTIERRA 2.5

// Codigos de Error
#define EXITO        0
#define ERRSOCKET   -1
#define ERRBIND     -2
#define ERRLISTEN   -3
#define ERRACCEPT   -4
#define ERRCONEXION -5

/* --- Estructuras de Datos --- */

typedef struct {
    char tipo_cotizacion; // 'A': Aire, 'T': Tierra
    char pais_destino[40];
    float kilometros;
    int dimension[3]; 
    float peso;
    char raza[30];
    int edadmeses;
} solicitud;

typedef struct {
    char estadocotizacion; // 'A', 'R', 'H', 'O'
    char detalle[200];
    float costousd;
    float costlocal;
    float costoeuro;
} respuesta;

/*menu */

void menu() {
    printf("\n--- SISTEMA DE COTIZACION ---\n");
    printf(" 1. Nueva Cotizacion\n");
    printf(" 2. Admin\n");
    printf(" 0. Salir\n");
    printf("Ingrese opcion: ");
}

/* razas debiles que generan recargo */

int razadebil(char *raza) {
    const char *razas_debiles[10] = {
        "Chihuahua", "Pomerania", "Yorkshire Terrier", "Dachshund", "Shih Tzu",
        "Maltés", "Papillón", "Bichón Frisé", "Cavalier King Charles Spaniel", "Toy Poodle"
    };
    int i;
    for (i = 0; i < 10; i++) {
        if (strcasecmp(raza, razas_debiles[i]) == 0) {
            return 1;
        }
    }
    return 0;
}
 /* precios fijos por paises recurrentes */
float paisesfijos(char *pais) {
    char *nombres[] = {
        "Estados Unidos", "España", "Brasil", "Mexico", "Chile", 
        "Uruguay", "Colombia", "Canada", "Francia", "Italia"
    };
    float precios[] = {
        500.00, 450.50, 200.00, 300.00, 150.00, 
        120.00, 350.00, 550.00, 480.00, 470.00
    };
    int i;
    int cantidad = 10;
    
    for (i = 0; i < cantidad; i++) {
        if (strcasecmp(pais, nombres[i]) == 0) {
            return precios[i]; 
        }
    }
    return -1; 
}



int abrir_conexion(int port, int backlog, int debug) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // Crear socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        if(debug) perror("socket failed");
        return ERRSOCKET;
    }

    // Opciones de socket (Reutilizar direccion)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        if(debug) perror("setsockopt failed");
        return ERRSOCKET;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        if(debug) perror("bind error");
        return ERRBIND;
    }

    // Listen
    if (listen(server_fd, backlog) < 0) {
        if(debug) perror("listen error");
        return ERRLISTEN;
    }
    
    if(debug) printf("servidor eschucando en puerto %d\n", port);
    return server_fd;
}

int aceptar_pedidos(int server_fd, int debug) {
    int new_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (new_socket < 0) {
        if(debug) perror("accept error");
        return ERRACCEPT;
    }
    return new_socket;
}

int conectar(char *hostname, int port, int debug) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        if(debug) perror("socket creation error");
        return ERRSOCKET;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, hostname, &serv_addr.sin_addr) <= 0) {
        if(debug) fprintf(stderr, "direccion invalida \n");
        return ERRCONEXION;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        if(debug) perror("Conneccion fallida");
        return ERRCONEXION;
    }
    return sock;
}
#endif