# Trabajo práctico Nro 1
## Protocolo DBUS

Estudiante: Agustin Manuel Leguizamón

Padrón: 99535

Link Github: https://github.com/AgustinLeguizamon/tp1

Instancia de entrega: 2

### Introducción:
A continuación se da una breve descripción de los TDAs que componen la implementación del protocolo dbus para la comunicación cliente/servidor a través de sockets TCP en lenguaje C.

![diagrama](img/TallerTP1.png)
Figura 1 - Relación entre los TDAs

#### Socket

##### Manejo de errores

La funcion `socket_connect` puede fallar en la conexion del socket, por lo tanto en el caso de que ninguna de la lista de direcciones dadas por `getaddrinfo` se retorno un -1 y se imprimira por pantalla el mensaje correspondiente.
Lo mismo pasa con la función `socket_bind_and_listen` que puede fallar al momento de enlazar a la direccion IP, en dicho caso se retornara un -1 y se imprimira por pantalla el mensaje de error.
Las funciones `socket_send` y `socket_recieve` pueden devolver tres valores, 0 que es el caso del que socket se haya cerrado que conforma parte de la ejecución normal, el numero de bytes enviados/recibidos, y -1 uno en caso de un error inesperado que provoca que la funcion retorne -1 y se imprima por pantalla el mensaje de error. 


#### File Reader

Para la lectura del archivo se usan 3 buffers, uno local a la función `file_reader_read_line`de tamaño fijo de 32 bytes para cada lectura, uno dinamico que alojará la linea entera y un buffer estatico de 32 bytes atributo del struct `file_reader_t` que posee los caracteres de la linea siguiente.
Basicamente se leen bloques de 32 bytes y se expande el tamaño del buffer dinamico hasta encontra un salto de linea, en ese momento se guardan los caracteres que se encuentran despues del salto de linea (pertenecen a la siguiente) y se borran de la linea actual de manera de solo tener esa linea en el buffer dinamico.
Luego en el siguiente llamado al comienzo  se agrega el buffer del struct (que tiene la primera parte de la ahora actual linea) al buffer dinamico y se repite el proceso.

#### Translator

La función `_translator_transform_line` se encarga de transformar la linea de texto `inpit_line` cambiando la separacion entre cada palabra y parámetro con un '\0'.
Por ejemplo:
```
taller.hellodbus /taller/greeter taller.Dbus.Greeter printHello(Hola!)
```
Al aplicar la función
```
taller\0hellodbus\0/taller/greeter\0taller.Dbus.Greeter\0printHello\0Hola!\0
```
De esta manera se utiliza el mismo array, sin utilizar mas memoria dinamica, para la construcción del mensaje segun el protocolo. Se recorre con un cursor que apunta al comienzo de cada "string" y al que se le pueden aplicar las operaciones estandar de string dado que cada uno termina en \0, luego se mueve el cursor una cantidad de bytes igual al largo del string actual de manera de que apunte al siguiente y asi hasta haber recorrido toda la linea y realizado las acciones correspondientes con cada "string".

#### Server

Para la recepcion del mensaje se utilizan 2 buffers dinamicos y uno estatico de 16 bytes(igual al tamaño de la firma del header del protocolo DBUS).
Con el primer `socket_recieve` se reciben los primeros 16 bytes del header de donde se obtiene la longitud del cuerpo y la longitud del array del header. A partir de estos dos valores se crean los otros dos buffers dinámicos `header_buffer` y `body_buffer`.
Luego se obtiene del `header_buffer` los parametros a imprimir en pantalla (ruta, destino, interfaz, etc) que se guardan en arrays dinamicos cuyo tamaño se obtiene del entero con la longitud del dato que conforma parte del parámetro.


#### Comentarios sobre Protocolo DBUS
Para el armado del mensaje plantee primero calcular el largo de todos los parámetros con su padding incluido y asignar un array en el heap del tamaño igual a la suma de los largos. Luego recorro el array con un cursor agregando byte a byte los distintos parámetros, como se puede ver en las funciones con `_append_` incluido. Por ejemplo:
```
int _translator_append_path(char** cursor, char* word){
	**cursor = 1;
	(*cursor)++;
	**cursor = 1;
	(*cursor)++;
	**cursor = 'o';
	(*cursor)++;
	**cursor = 0;
	(*cursor)++;

	*((uint32_t*)(*cursor)) = _translator_value_to_little_endian(strlen(word));
	(*cursor) += 4;

	int path_with_padding = _translator_round_up(strlen(word)+1);
	for (int i = 0; i < path_with_padding; ++i){
		**cursor = word[i];
		(*cursor)++;
	}

	return 0;
  ```
Luego la siguiente función tipo `_append_` sigue utiliza el cursor para seguir agregando mas parametros tipo BYTE o tipo UINT32. Así hasta completar el header.

En el lado del **server** con la misma lógica se recorre la tira de bytes con un cursor y se detiene hasta encontrar un tipo de parámetro (1 para ruta del objeto, 6 para string, etc) y desde ahí empieza a leer para obtener el string, esto se realiza en la función `_server_read_option`.

```
char* _server_read_option(char** cursor, char* word){
	(*cursor) += sizeof(uint32_t); //cursor apunta a long
	uint32_t option_len = *((uint32_t*) (*cursor));
	word = realloc(word, option_len+1);
	memset(word, 0, option_len+1);
					
	(*cursor) += sizeof(uint32_t); //cursor apunta array
	for (int i = 0; i < option_len+1; ++i){
		word[i]=(**cursor);
		(*cursor)++;
	}

	return word;
}
```

#### Manejo de errores

En el caso de una falla en la ejecución del programa o de un comportamiento inseperado se busca la terminacion del programa (con la liberacion de los recursos) indicando donde se provoca el error. En este trabajo no se intenta recuperar de los errores por mas que haya casos en los que sea posible reintenar y poder seguir con la ejecución.
