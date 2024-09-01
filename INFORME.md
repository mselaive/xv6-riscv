# Tarea 0: Instalación CORRECTA
Este informe describe los pasos seguidos para la instalación del sistema operativo xv6 en un entorno Mac OS, destacando los comandos utilizados y cualquier problema encontrado durante el proceso.

## Pasos de Instalación en Mac OS
### 1. Instalar Homebrew
Homebrew es un gestor de paquetes para Mac OS que facilita la instalación de software.

### 2. Instalar dependencias de QEMU
QEMU es un emulador y máquina virtual que permite ejecutar xv6. Para instalar QEMU, se utilizan los siguientes comandos: brew install qemu

### 3. Instalar dependencias de xv6-riscv
Para poder compilar y ejecutar xv6, es necesario instalar las herramientas y dependencias específicas para la arquitectura RISC-V. Se puede hacer con:
brew tap riscv/riscv
brew install riscv-tools

### 4. Fork y Clonación del Repositorio del Profesor
Se realiza un fork del repositorio original del proyecto en GitHub y luego se clona el repositorio fork en el entorno local: git clone 

### 5. Crear una Nueva Rama y Pushearla al Origin
Es importante trabajar en una rama separada para mantener el control de versiones

### 6. Crear el Archivo INFORME.md
Se crea un archivo Markdown para documentar el progreso y resultados del trabajo

### 7. Ejecutar make qemu
Para compilar y ejecutar xv6, se utiliza el comando make qemu, que debería lanzar una instancia de QEMU corriendo el sistema operativo: make qemu

### 8. Ejecutar Comandos Básicos en xv6
Una vez que el sistema operativo está en ejecución, se pueden probar comandos básicos
ls
echo "Hola xv6"
cat README

Dejo fotos del progrma funcionando en la entrega.

## Problemas Encontrados
No se presentaron problemas durante la instalación. El proceso fue exitoso sin necesidad de resolver errores o dependencias faltantes.