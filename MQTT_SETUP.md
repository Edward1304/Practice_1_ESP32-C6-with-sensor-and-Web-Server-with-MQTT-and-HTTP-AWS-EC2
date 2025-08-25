# Configuración de MQTT con Amazon EC2

**Autor:** Edward Fabian Goyeneche Velandia  
**Universidad:** Universidad Nacional de Colombia - Sede Manizales  
**Grupo:** Percepción de Control Inteligente  
**Asignatura:** Desarrollo de Sistemas IoT  
**Período:** 2025-II  

---

# Introduction

Esta guía detalla la configuración completa de un servidor MQTT usando Amazon EC2 con Mosquitto para proyectos IoT. Incluye desde la creación de la instancia EC2 hasta la configuración del broker MQTT con autenticación.

# Get Started

## Amazon Web Services

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/8bb4e472-958c-4e61-b830-31edf1b0a468/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/b9ae762d-d235-4022-a466-d6b811153d1a/Untitled.png)

### Amazon EC2

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/6b39e865-ca62-4c8b-bb35-9632b54f3038/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/f3ff63ce-452c-44d8-acf5-d092b36c66df/Untitled.png)

### Security Groups

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/b81b3854-56c9-4ee9-9716-a417f17da4f2/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/09218ec2-34b3-483b-a137-4c56ebc8f9e1/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/e9fc8cba-79b7-4687-8d4d-a757a83d7aca/Untitled.png)

### Key Pair

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/d74da38b-0290-4d7c-aa1f-abd8ef63c7e3/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/db3b1778-6d8d-467b-b0d4-895855c0c586/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/95942023-a888-491c-a0d6-296e7b8c9107/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/da583bd0-c85c-4dfb-af2c-d5ceefb2714e/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/22e17da0-1141-4f7c-96ef-0a1e73952be6/Untitled.png)

## How to access the instance

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/a560e242-73c0-46f5-b656-eb1a74b5ab3e/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/5b5d4a18-a3db-4cad-8617-8673f2dae60b/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/b9ed5c84-9195-407a-85a6-ed17b760d679/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/960270d1-929e-4030-96f6-b72b74686373/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/367b60c0-1517-45a5-aa72-c0a2ebfad835/Untitled.png)

## Ubuntu Virtual Machine on EC2 Instance

### Apache Web Server (HTTP)

**Install Apache**

Si aún no tienes Apache instalado, puedes hacerlo usando el gestor de paquete **apt**. Abre una terminal y ejecuta estos comandos:

```bash
sudo apt update
sudo apt install apache2
```

Una vez instalado, Apache debería iniciarse automáticamente, y su página de inicio predeterminada estará disponible en tu servidor.

**Crea la estructura de directorios**

Por convención, las páginas web se almacenan en el directorio **/var/www/html**. Puedes utilizar este directorio para tu página web sencilla. Puedes crear un directorio para tu sitio y cambiar los permisos para que puedas editar los archivos:

```bash
sudo mkdir /var/www/html/mi-sitio
sudo chown -R tu_usuario:tu_usuario /var/www/html/mi-sitio
```

Reemplaza **`tu_usuario`** con tu nombre de usuario.

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/cfea5a5b-c8ef-452c-be65-b490ceb6b4d5/Untitled.png)

**Comprobar el Estado del Servidor**

Asegúrate de que Apache esté en ejecución. Puedes verificar su estado con el siguiente comando:

```bash
sudo systemctl status apache2
```

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/a96c3533-9a74-43f7-91dd-dd0be76e6656/Untitled.png)

Si Apache no se está ejecutando, puedes iniciar el servicio con:

```bash
sudo systemctl start apache2
```

**Accede a tu página web**

Ahora, puedes acceder a tu página web en un navegador web utilizando la dirección IP de tu servidor.

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/dfd15854-a78d-4d11-b229-3eb9fcc78b2a/Untitled.png)

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/927dbd1d-5193-4319-a9f9-04f34a5b1619/Untitled.png)

**Important:** The default server does not support HTTPS so be sure to log in using HTTP.

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/64fec25f-d499-41b7-8ebb-e56b07a2abf1/Untitled.png)

### Mosquitto Server (MQTT)

Para crear un servidor MQTT en Ubuntu, puedes utilizar el popular servidor MQTT llamado Mosquitto. Mosquitto es de código abierto y ampliamente utilizado en la comunidad de IoT y desarrollo de aplicaciones que requieren comunicación de mensajes entre dispositivos.

**Paso 1: Instalar Mosquitto**

```bash
sudo apt update
sudo apt install mosquitto mosquitto-clients
```

Abre una terminal y ejecuta los siguientes comandos para instalar Mosquitto en tu sistema. Estos comandos actualizarán la lista de paquetes disponibles y luego instalarán el servidor Mosquitto y la utilidad de línea de comandos Mosquitto Clients.

**Paso 2: Iniciar y Habilitar el Servicio**

Una vez que Mosquitto esté instalado, puedes habilitar el servicio y asegurarte de que se inicie automáticamente al arrancar el sistema con los siguientes comandos:

```bash
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```

**Paso 3: Comprobar el Estado del Servidor**

Puedes verificar si Mosquitto se está ejecutando correctamente ejecutando el siguiente comando:

```bash
sudo systemctl status mosquitto
```

Deberías ver un mensaje que indique que el servicio está activo y en funcionamiento.

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/f53b4565-af12-41c2-8e13-3a438dd5ca13/Untitled.png)

**Paso 4: Configurar el Servidor**

La configuración predeterminada de Mosquitto generalmente es suficiente para un uso básico. Sin embargo, si deseas personalizar la configuración, puedes editar el archivo de configuración principal de Mosquitto:

```bash
sudo nano /etc/mosquitto/conf.d/mosquitto.conf
```

Realiza las modificaciones que desees y guarda el archivo:

```bash
listener 1883
protocol mqtt

listener 9001
protocol websockets

password_file /etc/mosquitto/passwd
allow_anonymous false
```

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/8c07cfc2-b45d-4869-a8a4-efbb7965c1c2/Untitled.png)

**Crear un archivo de contraseñas:**

Abre una terminal y ejecuta el siguiente comando para crear un archivo que almacene los usuarios y contraseñas:

```bash
sudo mosquitto_passwd -c /etc/mosquitto/passwd <nombre-de-usuario>
```

Reemplaza **`<nombre-de-usuario>`** con el nombre de usuario que desees. Serás solicitado a ingresar una contraseña para ese usuario.

![Untitled](https://prod-files-secure.s3.us-west-2.amazonaws.com/01d46c33-83a2-4229-b47b-329fdf8e1a74/824b725d-1a53-4b1a-a894-7a5a8789f746/Untitled.png)

Reinicia el servidor Mosquitto para que los cambios surtan efecto:

```bash
sudo systemctl restart mosquitto
```

Ahora, cuando te conectes al servidor MQTT Mosquitto, deberás proporcionar un nombre de usuario y contraseña válidos para autenticarte.

**Paso 5: Prueba el Servidor**

Puedes utilizar el cliente Mosquitto para probar la funcionalidad del servidor MQTT. Abre una terminal y utiliza el siguiente comando para suscribirte a un tema y ver los mensajes que llegan:

```bash
mosquitto_sub -h localhost -t test -u esp32 -P esp32
```

Abre otra terminal y publica un mensaje en el mismo tema:

```bash
mosquitto_pub -h localhost -t test -m "Hola, MQTT!" -u esp32 -P esp32
```

Deberías ver el mensaje "Hola, MQTT!" en la terminal donde te suscribiste al tema.

---

# Configuración Específica para ESP32-C6

Para que tu ESP32-C6 pueda conectarse a tu broker Mosquitto en EC2, necesitas configurar usuarios y permisos específicos.

## 1. Conectarse a  EC2
```bash
chmod 400 "Edward_psw.pem"
ssh -i "Edward_psw.pem" ubuntu@ec2-13-220-220-130.compute-1.amazonaws.com
```

## 2. Crear usuario MQTT
```bash
# Crear archivo de passwords si no existe
sudo touch /etc/mosquitto/passwd

# Agregar usuario 'esp32_user' (te pedirá la password)
sudo mosquitto_passwd /etc/mosquitto/passwd esp32_user
# Cuando te pida password, usa: esp32_pass

# Verificar que el usuario fue creado
sudo cat /etc/mosquitto/passwd
```

## 3. Configurar permisos para el usuario
Crear archivo de ACL (Access Control List):
```bash
sudo nano /etc/mosquitto/acl
```

Agregar el siguiente contenido:
```
# ACL para ESP32
user esp32_user
topic readwrite esp32/dht11/#
topic readwrite esp32/+/status
topic read esp32/dht11/commands
topic read esp32/dht11/config
```

## 4. Actualizar configuración de Mosquitto
Editar el archivo de configuración:
```bash
sudo nano /etc/mosquitto/mosquitto.conf
```

Asegúrate que tenga:
```
listener 1883
protocol mqtt

listener 9001
protocol websockets

password_file /etc/mosquitto/passwd
acl_file /etc/mosquitto/acl
allow_anonymous false

# Logging (opcional)
log_dest file /var/log/mosquitto/mosquitto.log
log_type all
```

## 5. Reiniciar Mosquitto
```bash
sudo systemctl restart mosquitto
sudo systemctl status mosquitto
```

## 6. Probar la conexión
Desde tu EC2, puedes probar:
```bash
# Suscribirse a los topics del ESP32
mosquitto_sub -h localhost -p 1883 -u esp32_user -P esp32_pass -t "esp32/dht11/data"

# En otra terminal, publicar un comando
mosquitto_pub -h localhost -p 1883 -u esp32_user -P esp32_pass -t "esp32/dht11/commands" -m "status"
```

## 7. Configurar Security Group
En la consola de AWS EC2, asegúrate que el Security Group permita:
- Puerto 1883 (MQTT) desde la IP de tu ESP32
- Puerto 9001 (WebSocket) si planeas usar WebSocket

## 8. Verificar firewall en EC2
```bash
# Ver reglas actuales
sudo ufw status

# Permitir puertos si es necesario
sudo ufw allow 1883
sudo ufw allow 9001
```

## Credenciales para tu ESP32
En el archivo `config.h` de tu ESP32, usa:
```c
#define MQTT_USERNAME "esp32_user"
#define MQTT_PASSWORD "esp32_pass"
```

## Topics que usará tu ESP32:
- **Publicación**: `esp32/dht11/data` (datos del sensor)
- **Publicación**: `esp32/dht11/status` (estado del dispositivo)
- **Suscripción**: `esp32/dht11/commands` (comandos remotos)
- **Suscripción**: `esp32/dht11/config` (configuración)
