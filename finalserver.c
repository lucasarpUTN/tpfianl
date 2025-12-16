#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "cotizfin.h"
#include "monedas.h"

float total_ventas_usd = 0.0;
float total_ventas_ars = 0.0;
pthread_mutex_t mutex_ventas = PTHREAD_MUTEX_INITIALIZER;

volatile int keep_running = 1; // para manejar señales
int server_sock = 0;

// Handler de señal Ctrl+C
void signal_handler(int sig)
{
    printf("\nApagando servidor...\n");
    keep_running = 0;
    close(server_sock); // cerrar socket para desbloquear accept()
}

void *atender_cliente(void *arg)
{
    int sock = *(int *)arg;
    free(arg);
    solicitud sol;
    respuesta res;
    float precio_base = 0.0;
    float cot_dolar = 0.0;
    float basura_euro = 0.0; // Variable temporal
    float basura_yuan = 0.0; 

    /* Obtener cotización real del dólar */
    obtener_cotizaciones_bcra(&cot_dolar, NULL, NULL);

    if (recv(sock, &sol, sizeof(sol), 0) <= 0)
    {
        close(sock);
        return NULL;
    }

    memset(&res, 0, sizeof(res));

    /* Calculo del precio base */
    if (sol.tipo_cotizacion == 'A' || sol.tipo_cotizacion == 'a')
    {
        precio_base = paisesfijos(sol.pais_destino);
        if (precio_base < 0)
            precio_base = 300.0 + sol.kilometros * KMAIRE;

        if (sol.edadmeses < 4)
        {
            int meses = 4 - sol.edadmeses;
            res.estadocotizacion = 'H';
            precio_base += meses * MENSUAL;
            snprintf(res.detalle, sizeof(res.detalle),
                     "Requiere hotel (%d meses)", meses);
        }
        else
        {
            res.estadocotizacion = 'A';
            strcpy(res.detalle, "Vuelo directo");
        }
    }
    else
    {
        res.estadocotizacion = 'A';
        precio_base = sol.kilometros * KMTIERRA;
        strcpy(res.detalle, "Transporte terrestre");
    }

    /* Aplicar recargo si la raza es débil */
    if (razadebil(sol.raza))
    {
        res.estadocotizacion = 'O';
        precio_base *= 1.30;
        strcat(res.detalle, " [Raza debil]");
    }

    /* Guardar precios */
    res.costousd = precio_base;
    res.costlocal = precio_base * cot_dolar;

    /* Guardar cotización aprobada y actualizar totales */
    if (res.estadocotizacion == 'A' || res.estadocotizacion == 'H')
    {
        pthread_mutex_lock(&mutex_ventas);

        total_ventas_usd += res.costousd;
        total_ventas_ars += res.costlocal;

        printf("Venta: %.2f USD | %.2f ARS | Total USD: %.2f | Total ARS: %.2f\n",
               res.costousd, res.costlocal, total_ventas_usd, total_ventas_ars);

        FILE *f = fopen("cotizaciones_aprobadas.txt", "a");
        if (f)
        {
            fprintf(f, "%s | %s | %c | %.2f USD | %.2f ARS\n",
                    sol.pais_destino,
                    sol.raza,
                    sol.tipo_cotizacion,
                    res.costousd,
                    res.costlocal);
            fclose(f);
        }

        pthread_mutex_unlock(&mutex_ventas);
    }

    send(sock, &res, sizeof(res), 0);
    close(sock);
    return NULL;
}


int main()
{
    signal(SIGINT, signal_handler); // registrar señal
    server_sock = abrir_conexion(PUERTO, BACKLOG, 1);

    if (server_sock < 0)
    {
        printf("Error al iniciar servidor\n");
        return -1;
    }

    printf("Servidor escuchando en puerto %d\n", PUERTO);

    while (keep_running)
    {
        int client_sock = aceptar_pedidos(server_sock, 1);
        if (client_sock < 0)
            continue;

        int *pclient = malloc(sizeof(int));
        *pclient = client_sock;

        pthread_t tid;
        pthread_create(&tid, NULL, atender_cliente, pclient);
        pthread_detach(tid);
    }

    printf("Servidor finalizado\n");
    pthread_mutex_destroy(&mutex_ventas);
    return 0;
}