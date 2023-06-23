# Network-Protocols-POP3
# Trabajo Práctico Especial - Protocolos de Comunicación
Implementación de un servidor POP3.

Alumnos:
- [Occhipinti, Abril		61159]
- [Ranucci, Santino Augusto	62092]
- [Rossi, Victoria			61131]
- [Zakalik, Agustín	        62068]


### Compilación
En la raíz del proyecto ejecutar:

```
make clean
make all
```

Esto genera el binario `project` en la carpeta `bin` y `client` en la carpeta `Client-Interface`. El primero es el servidor POP3 y el segundo es el cliente de monitoreo.


### Ejecución
Para correr el servidor, en la raíz del proyecto correr el comando:
```./bin/project <puerto_pasivo> -u <usuario>:<contraseña>```

Reemplazar ```<puerto_pasivo>``` por el puerto en el que el servidor tiene que atender nuevas conexiones.
A considerar: el ```<usuario>``` debe coincidir con un nombre de directorio en la carpeta mails. De lo contrario, el usuario podrá loggearse al servidor pero no tendrá mails.

Si se desea correr el servidor con más de un usuario se puede agregar con otro -u. Por ejemplo:

```
./bin/project <puerto_pasivo> -u <usuario1>:<contraseña1> -u <usuario2>:<contraseña2> ... -u <usuarioN>:<contraseñaN>
```


Para correr el cliente de monitoreo, en la carpeta `Client-Interface` correr el siguiente comando:
```./client <IPv4|IPv6> <Server IP> <Server Port/Service>```

Donde  `<Server Port/Service>` es `6000` para IPv4 y `6001` para IPv6

### Materiales
En la carpeta `docs` se encuentra el informe de este trabajo.
En la carpeta `test` se encuentran los archivos que se utilizaron para probar el servidor. `client.py` simula un cliente que hace infinitos requests. `runMultipleClients.sh` es un script para generar `X`conexiones simultáneas. Recibe como argumento la cantidad de procesos `client.py` que se desean crear.

