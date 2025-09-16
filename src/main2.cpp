#include <Arduino.h>
#include <WiFi.h>

//Pines motor propulsor
const int motorPPin1 = 27; 
const int motorPPin2 = 26; 
const int enablePPin = 14; 
//Pines bomba peristaltica
const int bombaPin1 = 25; 
const int bombaPin2 = 33; 
const int enableBPin = 32; 


// --- Configuración de Red ---
const char* ssid = "Telecentro-ece8";
const char* password = "DPYD4HHFQ37C";

// Usamos una IP estatica para facilitarle al usuario la conexion con el submarino
IPAddress local_IP(192, 168, 0, 24);      // Cambia a la IP deseada
IPAddress gateway(192, 168, 1, 1);         // Puerta de enlace de tu red
IPAddress subnet(255, 255, 255, 0);        // Máscara de subred
IPAddress primaryDNS(8, 8, 8, 8);          // DNS primario
IPAddress secondaryDNS(8, 8, 4, 4);        // DNS secundario 

// --- Configuración del Servidor---
WiFiServer server(80); // Puerto 80 es el estándar para HTTP

//Página de control
// Se guarda en memoria de programa (PROGMEM) para ahorrar RAM. Es más eficiente que usar un String.
const char pagina_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Submarino Microcontrolado</title>
  <style>
    body {
      margin: 0;
      font-family: Arial, sans-serif;
      display: flex;
      height: 100vh;
      background: #0d0d0d;
      color: #e0e0e0;
    }

    .panel {
      flex: 1;
      padding: 20px;
      box-sizing: border-box;
      border: 2px solid #111;
    }

    #controles {
      background: #141414;
      max-width: 20%;
      text-align: center;
    }

    #camara {
      background: #1b1b1b;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      flex: 1.2;
    }

    #info {
      background: #141414;
      max-width: 20%;
    }

    h2 {
      text-align: center;
      color: #00aaff;
      text-shadow: 0px 0px 8px #005577;
    }

    img {
      max-width: 90%;
      max-height: 80%;
      border: 3px solid #ff3333;
      border-radius: 12px;
      box-shadow: 0 0 20px #ff000050;
    }

    .control-btn {
      background: linear-gradient(145deg, #222, #111);
      border: 2px solid #ff3333;
      color: #e0e0e0;
      font-size: 22px;
      font-weight: bold;
      border-radius: 8px;
      cursor: pointer;
      transition: 0.2s;
      box-shadow: 0 0 10px #000;
      width: 80px;
      height: 80px;
    }

    .control-btn:hover {
      background: #222;
      color: #00aaff;
      border-color: #00aaff;
      box-shadow: 0 0 15px #00aaff88;
    }

     /*coso del joystick */
    .joystick {
      display: grid;
      grid-template-columns: 80px 80px 80px;
      grid-template-rows: 80px 80px 80px;
      gap: 10px;
      justify-content: center;
      margin-top: 20px;
    }

    .up { grid-column: 2; grid-row: 1; }
    .left { grid-column: 1; grid-row: 2; }
    .right { grid-column: 3; grid-row: 2; }
    .subir { grid-column: 1; grid-row: 3; }
    .bajar { grid-column: 3; grid-row: 3; }
  </style>

  <script>
    const ESP32_IP = 'http://{{ESP32_IP}}';
    function sendCommand(cmd) {
      fetch(`${ESP32_IP}/${cmd}`)
        .then(response => console.log("Comando enviado:", cmd))
        .catch(error => console.error("error:", error));
    }
  </script>
</head>
<body>
      <!--panel de los controles-->
  <div id="controles" class="panel">
    <h2>Controles</h2>
    <div class="joystick">
     
      <button class="control-btn up" onclick="sendCommand('adelante')">⬆️</button>
      <button class="control-btn left" onclick="sendCommand('izquierda')">⬅️</button>
      <button class="control-btn right" onclick="sendCommand('derecha')">➡️</button>
      <button class="control-btn subir" onclick="sendCommand('subir')">⏫</button>
      <button class="control-btn bajar" onclick="sendCommand('bajar')">⏬</button>
    </div>
  </div>
 <!--panel de cam-->
  <div id="camara" class="panel">
    <h2>Cámara</h2>
    <img src="http://ipdelacam" alt="Transmisión ESP32-CAM">
  </div>
<!--info -->
  <div id="info" class="panel">
    <h2>Info</h2>
    <p><b>Proyecto:</b> Nautilus VI</p>
    <p><b>Equipo:</b> Gonzalo, Benicio, Nicolás y Brenda</p>
    <p><b>Colegio:</b> Instituto Técnico San Judas Tadeo</p>
    <p><b>Profesor:</b> Franco Zapata</p>
  </div>
</body>
</html>
)rawliteral";
void setup() {
  Serial.begin(115200);

  // Set motor pins as outputs
    pinMode(motorPPin1, OUTPUT);
    pinMode(motorPPin2, OUTPUT);
    pinMode(enablePPin, OUTPUT);
    pinMode(bombaPin1, OUTPUT);
    pinMode(bombaPin2, OUTPUT);
    pinMode(enableBPin, OUTPUT);
    Serial.print("Conectandose con la red: ");
    Serial.println(ssid);
    /*if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("Fallo al configurar IP estática");
    }*/
    WiFi.begin(ssid, password);
    int timeout = 20; // 20 intentos de 500ms = 10 segundos
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(500);
        Serial.print(".");
        timeout--;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFallo la conexion. Reiniciando...");
        delay(1000);
        ESP.restart();
    }

    // --- Iniciar Servidor ---
    Serial.println("\nConexion Wi-Fi exitosa!");
    Serial.print("IP del servidor: http://");
    Serial.println(WiFi.localIP());
    server.begin();
}
void loop() {
  //Esperar a que un cliente se conecte
  WiFiClient client = server.available();
  if (!client) {
    return; // Si no hay cliente, no hacer nada y salir del loop
  }

  Serial.println("\n[Cliente conectado, bienvenido abordo capitan]");

  // 2. Leer la petición del cliente línea por línea
  String header = "";           // Para guardar la petición HTTP completa
  String currentLine = "";      // Para guardar la línea actual que estamos leyendo

  // El bucle se ejecuta mientras el cliente esté conectado
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();   // Leer un caracter a la vez
      header += c;              // Añadirlo a la petición completa

      // Si el caracter es un salto de línea, procesamos la línea
      if (c == '\n') {
        // Una línea en blanco (dos saltos de línea seguidos) indica el fin de la petición HTTP
        if (currentLine.length() == 0) {

          // 3. Interpretar la petición y actuar
          if (header.indexOf("GET /adelante") >= 0) {
            Serial.println("avanzando");
            digitalWrite(enablePPin, HIGH);
            digitalWrite(motorPPin2, LOW);
            digitalWrite(motorPPin1, HIGH);
          } else if (header.indexOf("GET /izquierda") >= 0) {
            Serial.print("doblar a la izquierda");
          }else if (header.indexOf("GET /derecha") >= 0) {
            Serial.print("doblar a la derecha");
          }else if (header.indexOf("GET /subir") >= 0) {
            Serial.print("subiendo");
            digitalWrite(enableBPin, HIGH);
            digitalWrite(bombaPin2, HIGH);
            digitalWrite(bombaPin1, LOW);
            
          }else if (header.indexOf("GET /bajar") >= 0) {
            Serial.print("bajando");
            digitalWrite(enableBPin, HIGH);
            digitalWrite(bombaPin1, HIGH);
            digitalWrite(bombaPin2, LOW);
            
          }

          // 4. Enviar la respuesta HTTP (construida manualmente)
          // Encabezado de la respuesta
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println(); // Esta línea en blanco es OBLIGATORIA para separar encabezado y cuerpo

          // Cuerpo de la respuesta (la página web)
          String pagina = String(pagina_html);
          pagina.replace("{{ESP32_IP}}", WiFi.localIP().toString());
          client.print(pagina);

          // Salimos del bucle 'while' ya que hemos respondido
          break;
        } else {
          // Si no es una línea en blanco, la reseteamos para leer la siguiente
          currentLine = "";
        }
      } else if (c != '\r') {
        // Si no es un salto de línea ni un retorno de carro, lo añadimos a la línea actual
        currentLine += c;
      }
    }
  }

  // 5. Cerrar la conexión
  client.stop();
  Serial.println("[Cliente Desconectado]");
}

