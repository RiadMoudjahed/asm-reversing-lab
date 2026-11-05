<!-- source: README.md @ sha256:c943aa4625a0 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 04. Orden de bytes

Level 2. Tiempo: aproximadamente 1,5 horas. Arquitectura: x86-64 Linux. Módulos: [01. Representación de datos](../../modules/01-data-representation/README.es.md), [04. Mover datos y modos de direccionamiento](../../modules/04-moving-data/README.es.md)

## Escenario

Un pequeño servicio guarda su configuración en un registro fijo de 12 bytes. El registro mezcla anchuras de campo y órdenes de bytes porque se montó a partir de varios formatos más antiguos. La única documentación es un programa llamado `config_probe` que lee el registro y lo comprueba. Debes deducir la estructura a partir de ese programa y después escribir las rutinas de decodificación para un componente nuevo.

## Objetivos

- Recuperar desplazamientos, anchuras y órdenes de bytes de los campos a partir del desensamblado.
- Decodificar a mano valores de varios bytes de un volcado hexadecimal.
- Implementar cargas big-endian y little-endian sin instrucciones de intercambio de bytes.

## Parte 1: Reverse

El binario es `bin/config_probe`. Decodifica cuatro campos (llámalos A, B, C y D) de un registro de su sección `.rodata` y comprueba que los valores decodificados suman una suma de verificación esperada.

```bash
bin/config_probe
objdump -d -M intel bin/config_probe
objdump -s -j .rodata bin/config_probe
```

Responde estas preguntas:

1. Para cada campo de A a D: ¿cuál es su desplazamiento en el registro, su anchura y su orden de bytes, tal como los trata el código?
2. ¿Cuál es el valor decodificado de cada campo? Da A como dos números (sus 16 bits altos y sus 16 bits bajos), B y D en decimal, y C en hexadecimal.
3. ¿Suman los cuatro valores decodificados la constante de la instrucción `cmp`? Si no, hay un orden de bytes equivocado.

Construye la bandera con tus respuestas:

```text
ARL{v<A high>_<A low>_p<B>_t<D>}
```

Por ejemplo, si A fuera `0x00020005`, B fuera 100 y D fuera 2000, la bandera sería `ARL{v2_5_p100_t2000}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar tres funciones que decodifiquen el registro que analizaste. Usan la convención de llamada System V: el primer argumento llega en `rdi`, el segundo en `rsi` y el resultado vuelve en `rax`.

```c
#include <stdint.h>

struct config { uint32_t version; uint16_t port; uint16_t flags; uint32_t timeout_ms; };

uint32_t load_be32(const uint8_t *p);   // 4 bytes at p, most significant byte first
uint16_t load_le16(const uint8_t *p);   // 2 bytes at p, least significant byte first
void decode_record(const uint8_t *rec, struct config *out);
```

`decode_record` lee el registro de 12 bytes que hay en `rec` y rellena `out`: `version` es el campo A, `port` es el campo B, `flags` es el campo C y `timeout_ms` es el campo D. La estructura del registro es la que recuperaste en la Parte 1; no se repite aquí.

## Reglas

- No uses `bswap` ni `movbe`. Construye los valores con bytes, desplazamientos y `or`, o usa cargas simples donde el orden de bytes lo permita.
- Lee solo los bytes que pertenecen a cada campo. Las pruebas colocan cada entrada justo antes de una página inaccesible, así que una carga que pase del final provoca un fallo.
- Escribe en `out` solo los bytes de cada campo. Las pruebas comprueban que los campos vecinos no se tocan.

## Comprueba tu trabajo

```bash
make check EX=04 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y de la Parte 2 por separado. Sin `--flag`, comprueba solo la Parte 2.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
