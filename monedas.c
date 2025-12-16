#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "monedas.h"

struct Memory
{
    char buffer[102400];
    int size;
};

size_t write_data(void *ptr, size_t size, size_t nmemb, void *userp)
{
    struct Memory *mem = (struct Memory *)userp;
    size_t total = size * nmemb;

    if (mem->size + total >= 102400)
    {
        printf("Alerta: Buffer lleno, se cortaron datos.\n");
        return 0;
    }

    memcpy(mem->buffer + mem->size, ptr, total);
    mem->size += total;
    mem->buffer[mem->size] = '\0';

    return total;
}

void obtener_cotizaciones_bcra(float *dolar, float *euro, float *yuan)
{
    CURL *curl;
    CURLcode res;
    struct Memory chunk;

    chunk.size = 0;
    chunk.buffer[0] = '\0';

    curl = curl_easy_init();
    if (!curl)
        return;

    // URL del BCRA
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.bcra.gob.ar/estadisticascambiarias/v1.0/Cotizaciones");

    
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);

    res = curl_easy_perform(curl);

    if (res == CURLE_OK)
    {
        cJSON *json = cJSON_Parse(chunk.buffer);
        if (json)
        {
            cJSON *results = cJSON_GetObjectItem(json, "results");

            if (results)
            {
                cJSON *lista = cJSON_GetObjectItem(results, "detalle");
                if (!lista)
                    lista = results;

                cJSON *item;
                cJSON_ArrayForEach(item, lista)
                {
                    cJSON *cod = cJSON_GetObjectItem(item, "codigoMoneda");
                    cJSON *val = cJSON_GetObjectItem(item, "tipoCotizacion");

                    if (cod && val)
                    {
                        // USD
                        if (strcmp(cod->valuestring, "USD") == 0)
                        {
                            if (dolar != NULL)
                                *dolar = (float)val->valuedouble;
                        }
                        // EURO
                        else if (strcmp(cod->valuestring, "EUR") == 0)
                        {
                            if (euro != NULL)
                                *euro = (float)val->valuedouble;
                        }
                        // YUAN
                        else if (strcmp(cod->valuestring, "CNY") == 0)
                        {
                            if (yuan != NULL)
                                *yuan = (float)val->valuedouble;
                        }
                    }
                }
            }
            cJSON_Delete(json); 
        } 
    }
    else
    {
        printf("Error API BCRA: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
}