#!/bin/bash

# Configuración de colores para la terminal
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Función de ayuda
usage() {
    echo "Uso: $0 [debug|release]"
    echo "  debug   : Compila con símbolos de depuración (GDB) y sin optimizar."
    echo "  release : Compila con optimización máxima (-O3) para rendimiento."
    exit 1
}

# Verificar que se pasó un argumento
if [ -z "$1" ]; then
    usage
fi

MODE=$1

echo -e "${BLUE}==> Configurando en modo: ${MODE^^} <==${NC}"

# Ejecutar CMake con el tipo de build correspondiente
if [ "$MODE" == "debug" ]; then
    cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Debug 
elif [ "$MODE" == "release" ]; then
    cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Release 
else
    usage
fi

# Compilar
echo -e "${BLUE}==> Compilando...${NC}"
make 

if [ $? -eq 0 ]; then
    echo -e "${GREEN} [OK] Compilación exitosa.${NC}"
fi
