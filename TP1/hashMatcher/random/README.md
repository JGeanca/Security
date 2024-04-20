# Programa de Comparación de Contraseñas

Este programa compara las contraseñas almacenadas en un archivo CSV con combinaciones de palabras extraídas de un archivo de palabras.

## Requisitos del Sistema

- Compilador de C (GCC)
- Biblioteca OpenSSL
- OpenMP (opcional, pero recomendado para aprovechar el procesamiento en paralelo)

## Compilación del Programa

Para compilar el programa, ejecuta el siguiente comando en la terminal:

~~~bash
make
~~~

Esto generará un ejecutable llamado `program`.

## Uso del Programa

El programa espera tres argumentos de línea de comandos:

`Usage: ./program <start_index> <end_index> <Stage_2? 0 : 1>`

1. start_index: el índice de inicio donde se empezará en el array de palabras para la generación de combinaciones.
2. end_index: el índice final donde se terminará en el array de palabras para la generación de combinaciones.
3. Etapa 2: un indicador booleano (0 o 1) para especificar si se debe realizar la Etapa 2 de comparación de contraseñas. La etapa 2 compara combinaciones de 2 palabras. En caso de omitirla solo se van a comarar combinaciones de 3 palabras.

Ejemplo de uso:

~~~bash
make

./program 0 1000 1
~~~

Una vez terminado el programa o finalizado manualmente, se generará un archivo txt llamado `passwords_found` con las contraseñas que coinciden.