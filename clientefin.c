#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cotizfin.h" 

void flush_in() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    int opcion = 0;
    int sock = 0;

    solicitud s;
    respuesta r;
    while (1) {
        menu();
        scanf("%d", &opcion);
        flush_in();

        if (opcion == 0)
            break;

        if (opcion == 1) {
            sock = conectar(IPSERVIDOR, PUERTO, 1); 
            if (sock < 0) {
                printf("Error: No se pudo conectar con el servidor.\n");
                continue;
            }

            memset(&s, 0, sizeof(solicitud));

            printf("\n--- NUEVA COTIZACION ---\n");

            // Transporte
            printf("Transporte (A = Aire / T = Tierra): ");
            scanf("%c", &s.tipo_cotizacion);
            flush_in(); 
            // Raza
            printf("Raza: ");
            fgets(s.raza, sizeof(s.raza), stdin);
            s.raza[strcspn(s.raza, "\r\n")] = 0;

            // País destino
            printf("Pais Destino: ");
            fgets(s.pais_destino, sizeof(s.pais_destino), stdin);
             s.pais_destino[strcspn(s.pais_destino, "\r\n")] = 0;
            // Edad
            printf("Edad (meses): ");
            scanf("%d", &s.edadmeses);
            flush_in();

            // Peso
            printf("Peso (KG): ");
            scanf("%f", &s.peso);
            flush_in();

            // Distancia
            printf("Distancia (KM): ");
            scanf("%f", &s.kilometros);
            flush_in();

            // Enviar al servidor
            send(sock, &s, sizeof(solicitud), 0);
            printf("\nEnviando datos...\n");

            memset(&r, 0, sizeof(respuesta));

            // Recibir respuesta (SOLO UNA VEZ)
            if (recv(sock, &r, sizeof(respuesta), 0) > 0) {
                
                // terminador nulo para evitar Segmentation Fault
                r.detalle[sizeof(r.detalle) - 1] = '\0';

                printf("\n---------------------------------------\n");

                if (r.estadocotizacion == 'A')      printf(" ESTADO: APROBADO\n");
                else if (r.estadocotizacion == 'H') printf(" ESTADO: HOTEL + VUELO\n");
                else if (r.estadocotizacion == 'O') printf(" ESTADO: OBSERVADO (Raza Debil)\n");
                else                                printf(" ESTADO: RECHAZADO\n");

                printf(" DETALLE: %s\n", r.detalle);
                printf(" COSTO FINAL: %.2f USD | %.2f ARS\n", r.costousd, r.costlocal);
                printf("---------------------------------------\n");
            } else {
                printf("Error al recibir respuesta.\n");
            }

            close(sock); 
        }
    }

    return 0;
}