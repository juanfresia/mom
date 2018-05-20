# Cola de mensajes publisher-subscriber distribuida

TP Sistemas Distribuidos (7574)

## Compilación

Para compilar todos los binarios puede usarse el Makefile, sencillamente `make` generará los tres binarios `broker_server`, `lb_daemon` y `cliente`.

## Inicializar el proceso del broker.

Para hacer que una máquina haga de broker, debe correrse en ésta el programa `broker_server`.
Pueden pasar se las siguientes opciones por variables de entorno al ejecutar:
  - `LISTEN_IP`: la IP donde el broker escuchará conexiones nuevas. (Default 127.0.0.1)
  - `LISTEN_PORT`: el puerto donde el broker escuchará conexiones nuevas. (Defautl 12345)

## Inicializar el daemon del broker local

Cualquier máquina en la que vayan a correr clientes deberá tener un daemon de local broker corriendo, correspondiente al programa `lb_daemon`.
Pueden pasarse las siguientes opciones por variables de entorno:
  - `BROKER_IP`: la IP del broker al que se va a conectar. (Default 127.0.0.1)
  - `BROKER_PORT`: el puerto del broker. (Default 12345)

## Iniciar un cliente CLI

Correr el programa `client` para iniciar un cliente, el cual se registrará automáticamente con el daemon local, y brindará un prompt para ingresar comandos.
Ingresando `?` se imprimirá el usage.
