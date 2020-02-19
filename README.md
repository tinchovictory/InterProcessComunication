# InterProcessComunication

## Introducción

El trabajo práctico consiste en aprender a utilizar los distintos tipos de IPCs presentes en un sistema POSIX. Para ello se implementará un sistema que distribuirá tareas de cálculo pesadas entre varios pares.

## Grupos

Se formarán grupos de hasta tres personas. En caso de quedar grupos de uno o dos integrantes, se resolverá durante la clase de práctica caso por caso.

## Requerimientos

El sistema calcula los hashes MD5 de múltiples archivos de forma distribuida. Para ello cuenta con procesos aplicación, vista, y esclavos

### Proceso Aplicación
  * Recibe por línea de comando los archivos a procesar, por ejemplo:
```
$ hash files/*
```
  * Arranca los procesos esclavos.
  * Envía una cierta cantidad de archivos a procesar *distribuyendo la carga* entre los esclavos por cantidad de archivos.
  * Cuando un esclavo se libera, la aplicación le entrega más tareas.  
  * Espera los cálculos de los esclavos y los agrega a un buffer a medida que llegan, *por orden de llegada*.
  * Espera a que aparezca un proceso vista, si lo hace le comparte el buffer de llegada.
  * Termina cuando la operación esté completada.
  * El resultado, aparezca el proceso de vista o no, lo escribe a un archivo en disco.

### Proceso Vista

  * Recibe por línea de comando el PID del proceso aplicación (corren en el mismo sistema).
  * Se conecta al proceso cliente, muestra y actualiza en pantalla el contenido del buffer de llegada usando el formato: `<nombre de archivo>: <hash md5>` para cada archivo procesado.
  
### Proceso Esclavo

  * Procesa los pedidos del cliente, recibiendo los archivos y calculando los hashes MD5 uno por uno, utilizando una cola de pedidos.
  * Envía los hashes correspondientes una vez que están listos.
  * El proceso esclavo puede (y la cátedra sugiere) ejecutar el comando md5sum para calcular los hashes. Queda libre al ingenio del grupo descubrir cómo recolectar el resultado de md5sum desde el esclavo, pero queda **prohibido** volcarlo a un archivo en disco, para luego leerlo desde el esclavo. IPCs y primitivas de sincronización requeridos
  * Para conectar el proceso aplicación con sus esclavos deberán utilizar FIFOs, pipes o named pipes.
  * Para compartir el buffer del proceso aplicación es necesario usar Shared Memory con Semáforos.

## Entrega
**Fecha:** 05/09/2017
**Entregables:** Mail con un archivo ZIP adjunto con el código fuente y el informe, el mail debe incluir el hash MD5 del archivo.
**Defensa del trabajo práctico:** el mismo día de la entrega.
