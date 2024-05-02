#!/bin/bash

# Verificați dacă sunt furnizate argumentele corecte
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <port>"
    exit 1
fi

# Portul pentru care se va căuta PID-ul procesului
PORT=$1

# Găsește PID-ul procesului care ascultă la portul specificat
PID=$(sudo lsof -t -i :$PORT)

# Verifică dacă s-a găsit un PID
if [ -z "$PID" ]; then
    echo "Nu există proces care ascultă la portul $PORT."
else
    echo "Procesul cu PID-ul $PID ascultă la portul $PORT."

    # Oprește procesul folosind comanda kill
    sudo kill $PID
    echo "Procesul cu PID-ul $PID a fost oprit."
fi
