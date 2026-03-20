#!/bin/bash
# Prueba FDISK con primaria + extendida. Ejecutar desde raíz del proyecto con servidor en marcha.
BASE="http://localhost:8080/execute"
DISCO="disks/disco2.mia"

rm -f "$DISCO"

post() { curl -s -X POST "$BASE" -H "Content-Type: application/json" --data-binary "$1"; echo ""; }

post '{"input": "mkdisk -size=20 -path=disks/disco2.mia -unit=M"}'
sleep 1
post '{"input": "fdisk -size=5 -path=disks/disco2.mia -name=Prim1 -unit=M -type=P"}'
sleep 1
post '{"input": "fdisk -size=5 -path=disks/disco2.mia -name=Extendida -unit=M -type=E"}'
sleep 1
post '{"input": "debugmbr -path=disks/disco2.mia"}'
echo "--- Segunda extendida (debe dar error): ---"
post '{"input": "fdisk -size=2 -path=disks/disco2.mia -name=OtraExt -unit=M -type=E"}'
