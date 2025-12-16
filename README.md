# TP Sockets - Cotizador de Mascotas

Sistema cliente-servidor hecho en C para la materia. Permite cotizar viajes de mascotas calculando el precio según la raza, el peso y si es por aire o tierra.


1. Para el servidor:
gcc finalserver.c monedas.c -o servidor -lpthread -lcjson
./servidor

3. Para el cliente:
gcc cliente.c -o cliente
./cliente

## Detalles
* El programa soporta múltiples clientes a la vez.
* Valida mayúsculas y minúsculas automáticamente (ej: toma "bulldog" igual que "Bulldog").
* Si el país es uno de los fijos (Brasil, USA, etc) cobra precio base, sino cobra por KM.
