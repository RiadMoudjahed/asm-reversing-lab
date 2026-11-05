<!-- source: CONTRIBUTING.md @ sha256:1e4e4a084d48 -->
[English](CONTRIBUTING.md) | [العربية](CONTRIBUTING.ar.md) | [Deutsch](CONTRIBUTING.de.md) | [Español](CONTRIBUTING.es.md) | [Français](CONTRIBUTING.fr.md)

# Contribuir

Gracias por ayudar a otras personas a aprender. Esta página explica cómo contribuir bien.

## Formas de contribuir

- Informar de un error técnico, una explicación poco clara o un ejemplo que no funciona.
- Mejorar o revisar una traducción. La revisión por hablantes nativos es especialmente valiosa.
- Proponer una mejora para una pista o una explicación de solución.
- Corregir erratas y formato.

Para módulos o ejercicios nuevos, abre primero un issue para acordar el alcance antes de empezar.

## Reglas básicas

1. **Solo trabajo original.** Escribe tus propias explicaciones, ejemplos y retos. No copies ni parafrasees de cerca ningún libro, curso, blog o vídeo, y no cites obras comerciales como fuentes. Se pueden citar especificaciones oficiales.
2. **Respeta el alcance.** Nada de desarrollo de exploits, shellcode, desarrollo de malware ni contenido sobre cómo eludir la protección contra copia o las licencias. Los programas de los retos se compilan desde su código fuente en este repositorio; no se admiten binarios de terceros.
3. **Sin spoilers.** Las banderas y las soluciones completas solo van en `solutions/`. Nunca las publiques en issues ni en debates.
4. **Presentación profesional.** Sin emojis ni símbolos decorativos. Lenguaje claro y directo.
5. **Todo funciona.** Los bloques de código con las etiquetas `nasm`, `c` o `bash` deben funcionar en la imagen del laboratorio.

## Flujo de trabajo

1. Haz un fork del repositorio y crea una rama temática.
2. Realiza tu cambio siguiendo las reglas básicas de arriba y el estilo de las páginas existentes.
3. Ejecuta `make all-checks` dentro del laboratorio. Todas las comprobaciones deben pasar.
4. Abre un pull request y completa la lista de verificación de la plantilla.

## Traducciones

Los archivos en inglés son la fuente. Una traducción se guarda junto al archivo inglés con un sufijo de idioma: `README.fr.md`, `README.de.md`, `README.es.md`, `README.ar.md`.

- Empieza el archivo con `<!-- source: FILE.md @ sha256:<12 hex> -->` (los 12 primeros caracteres de `sha256sum FILE.md`).
- No traduzcas código, mnemónicos, nombres de registros, comandos, nombres de archivo ni banderas.
- Usa los términos de `i18n/terminology.csv`. Añade una fila cuando necesites un término nuevo.
- Sigue `i18n/STYLE.md` para el tono y la tipografía de cada idioma.
- Ejecuta `make i18n` para ver qué páginas faltan o están desactualizadas.

## Informar de un problema de seguridad

Si crees que una página o un programa de este repositorio podría causar daño, contacta con los mantenedores de forma privada en lugar de abrir un issue público.
