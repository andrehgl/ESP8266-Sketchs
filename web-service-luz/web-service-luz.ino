#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

//Inicializacao do WiFiManager
WiFiManager wifiManager;
//Inicializacao so servidor http na porta 80
WiFiServer server(80);
//Status da GPIO
uint8_t status_gpio = 0;

void setup() {

    //Configura a serial
    Serial.begin(115200);

    //Configura a GPIO como saida
    pinMode(D3, OUTPUT);
    //Coloca a GPIO em sinal logico baixo
    digitalWrite(D3, LOW);

    //Define o auto connect e o SSID do modo AP
    wifiManager.setConfigPortalTimeout(180);
    wifiManager.autoConnect("MeuWebServer");
    //Log na serial se conectar
    Serial.println("Conectado");
    //Inicia o webserver de controle da GPIO
    server.begin();
}

void reset_config(void) {
  //Reset das definicoes de rede
  wifiManager.resetSettings();
  delay(1500);
  ESP.reset();
}

void loop() {

  //Aguarda uma nova conexao
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.println("Nova conexao requisitada...");

  while(!client.available()){
    delay(1);
  }

  Serial.println("Nova conexao OK...");

  //Le a string enviada pelo cliente
  String req = client.readStringUntil('\r');
  //Mostra a string enviada
  Serial.println(req);
  //Limpa dados/buffer
  client.flush();

  //Trata a string do cliente em busca de comandos
  if (req.indexOf("rele_on") != -1){
    digitalWrite(D3, HIGH);
    status_gpio = HIGH;
  } else if (req.indexOf("rele_off") != -1) {
    digitalWrite(D3, LOW);
    status_gpio = LOW;
  }
  else {
    Serial.println("Requisicao invalida");
  }

  //Prepara a resposta para o cliente
  String buf = "";
  buf += "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
  buf += "<html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>\r\n";
  buf += "<title>WebServer ESP8266</title>";
  buf += "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:80%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;}</style>";
  buf += "</head>";
  buf += "<h3> ESP8266 Web Server</h3>";

  //De acordo com o status da GPIO envia o comando
  if(status_gpio)
    buf += "<div><h4>Lampada</h4><a href=\"?function=rele_off\"><button>Desligar</button></a></div>";
  else
    buf += "<div><h4>Lampada</h4><a href=\"?function=rele_on\"><button>Ligar</button></a></div>";

  buf += "<h4>Criado por Pedro Minatel</h4>";
  buf += "</html>\n";

  //Envia a resposta para o cliente
  client.print(buf);
  client.flush();
  client.stop();

  Serial.println("Cliente desconectado!");
}
