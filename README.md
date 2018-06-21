# Cola de mensajes publisher-subscriber distribuida

TP Sistemas Distribuidos (7574)

## Compilación

Para compilar todos los binarios puede usarse el Makefile, sencillamente `make` generará los tres binarios principales `broker_server`, `lb_daemon` y `cliente`, así como también los programas auxiliares.

## Inicializar el proceso del broker

Para hacer que una máquina haga de broker, debe correrse en ésta el programa `broker_server`.
Pueden pasar se las siguientes opciones por variables de entorno al ejecutar:
  - `LISTEN_IP`: la IP donde el broker escuchará conexiones nuevas. (Default 127.0.0.1)
  - `LISTEN_PORT`: el puerto donde el broker escuchará conexiones nuevas. (Default 12345)
  - `DB_DIR`: directorio que se utilizará como base de datos. (Default broker_data)

### Brokers distribuido

Se puede optar por distribuir el broker entre varias máquinas, siguiendo una topología de anillo. Al hacerlo deben ser conocidas las IPs de todos las máquinas broker, y ser cada proceso `broker_server` levantado con las siguientes opciones:

- `BROKER_IP`: la IP donde el broker escuchará conexiones de sus pares en el anillo. (Default 127.0.0.1)
- `BROKER_PEER_PORT`: el puerto donde el broker escuchará conexiones de sus pares. (Default 12346)
- `BROKER_NEXT_IP`: la IP del siguiente broker en la topología del anillo.
- `BROKER_NEXT_PORT`: el puerto del siguiente broker en la topología anillo.
- `BROKER_ID`: la ID del broker. Los brokers deben tener IDs distintas para, y debe existir un broker con ID 1.
- `KEY_PATH`: directorio que se utilizará para generar las colas de mensaje internas del broker (Default ./msgqueue.h).

Para levantar un anillo se deben especificar correctamente las IP de pares y siguiente; y luego inicializar los procesos de a uno. La única restricción es que el broker con ID 1 sea _el último_ que se levanta.

*Importante*: si se quiere levantar múltiples broker a la vez, es necesario especificar archivos distintos para la cola de mensajes mediante la variable de entorno `KEY_PATH`.

## Inicializar el daemon del broker local

Cualquier máquina en la que vayan a correr clientes deberá tener un daemon de local broker corriendo, correspondiente al programa `lb_daemon`.
Pueden pasarse las siguientes opciones por variables de entorno:
  - `BROKER_IP`: la IP del broker al que se va a conectar. (Default 127.0.0.1)
  - `BROKER_PORT`: el puerto del broker. (Default 12345)
- `DB_DIR`: directorio que se utilizará como base de datos. (Default lb_data)

## Iniciar un cliente CLI

Correr el programa `client` para iniciar un cliente, el cual se registrará automáticamente con el daemon local, y brindará un prompt para ingresar comandos.
Ingresando `?` se imprimirá el usage.