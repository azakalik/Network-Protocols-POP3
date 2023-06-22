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

Esto genera el binario `project` en la carpeta `bin`

### Ejecución
En la raíz del proyecto correr el comando:
```./bin/project {puerto_pasivo} -u {usuario}:{contraseña}```

Reemplazar ```{puerto_pasivo}``` por el puerto en el que el servidor tiene que atender nuevas conexiones.
A considerar: el ```{usuario}``` debe coincidir con un nombre de directorio en la carpeta mails. De lo contrario, el usuario podrá loggearse al servidor pero no tendrá mails.

### Materiales
En la carpeta `docs` se encuentra el informe de este trabajo.
