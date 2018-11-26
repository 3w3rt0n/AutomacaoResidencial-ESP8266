
#include <ESP8266WiFi.h>   
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

//Pinos onde estão os Reles
#define PIN_RELE_1 12
#define PIN_RELE_2 13
//Pino onde está o LDR
#define PIN_LDR 0
//Pinos onde estão os botões
#define PIN_BTN_1 4
#define PIN_BTN_2 14


//Intervalo entre as checagens de novas mensagens
#define INTERVAL 1000

//Token do seu bot. Troque pela que o BotFather te mostrar
#define BOT_TOKEN "xxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
//t.me/UserCasaBot
//CasaBot
                  

//Troque pelo ssid e senha da sua rede WiFi
#define SSID "SSID da rede WiFi"
#define PASSWORD "Senha da rede WiFi"

//Comandos aceitos
const String LIGAR1 = "LIGAR 1";
const String LIGAR2 = "LIGAR 2";
const String DESLIGAR1 = "DESLIGAR 1";
const String DESLIGAR2 = "DESLIGAR 2";
const String CHAVE1 = "CHAVE 1";
const String CHAVE2 = "CHAVE 2";
const String LDR = "LUMINOSIDADE";
const String ESTADO = "ESTADO";
const String INICIA = "/start";


//Cliente para conexões seguras
WiFiClientSecure client;
//Objeto com os métodos para comunicarmos pelo Telegram
UniversalTelegramBot bot(BOT_TOKEN, client);
//Tempo em que foi feita a última checagem
uint32_t lastCheckTime = 0;

//Quantidade de usuários que podem interagir com o bot
#define SENDER_ID_COUNT 2
//Ids dos usuários que podem interagir com o bot. 
//É possível verificar seu id pelo monitor serial ao enviar uma mensagem para o bot
String validSenderIds[SENDER_ID_COUNT] = {"xxxxxxxxx", "xxxxxxxxx"};

void setup()
{
  Serial.begin(115200);

  //Inicializa o WiFi e se conecta à rede
  setupWiFi();

  //Coloca o pino do relê como saída e enviamos o estado atual
  pinMode(PIN_RELE_1, OUTPUT);
  pinMode(PIN_RELE_2, OUTPUT);

  pinMode(PIN_BTN_1, INPUT);
  pinMode(PIN_BTN_2, INPUT);
  
  digitalWrite(PIN_RELE_1, LOW);
  digitalWrite(PIN_RELE_2, LOW);  

  bot.sendMessage("xxxxxxxxx", "BOT Online!", "HTML");
}

void setupWiFi()
{
  Serial.print("Connecting to SSID: ");
  Serial.println(SSID);

  //Inicia em modo station e se conecta à rede WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  //Enquanto não estiver conectado à rede
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  //Se chegou aqui está conectado
  Serial.println();
  Serial.println("Connected");
}

void handleNewMessages(int numNewMessages)
{
  for (int i=0; i<numNewMessages; i++) //para cada mensagem nova
  {
    String chatId = String(bot.messages[i].chat_id); //id do chat 
    String senderId = String(bot.messages[i].from_id); //id do contato
    
    Serial.println("senderId: " + senderId); //mostra no monitor serial o id de quem mandou a mensagem
    Serial.println("chatId: " + chatId);

    boolean validSender = validateSender(senderId); //verifica se é o id de um remetente da lista de remetentes válidos

    if(!validSender) //se não for um remetente válido
    {
      bot.sendMessage(chatId, "Desculpe mas você não tem permissão", "HTML"); //envia mensagem que não possui permissão e retorna sem fazer mais nada
      continue; //continua para a próxima iteração do for (vai para próxima mensgem, não executa o código abaixo)
    }
    
    String text = bot.messages[i].text; //texto que chegou

    if (text.equalsIgnoreCase(INICIA)) {
      Inicia(chatId, bot.messages[i].from_name); //mostra as opções
    }else if (text.equalsIgnoreCase(LIGAR1)){
      LigarRele1(chatId); //liga o relê
    }else if(text.equalsIgnoreCase(DESLIGAR1)){
      DesligarRele1(chatId); //desliga o relê
    }else if(text.equalsIgnoreCase(LIGAR2)){
      LigarRele2(chatId); //envia mensagem com a temperatura e umidade
    }else if(text.equalsIgnoreCase(DESLIGAR2)){
      DesligarRele2(chatId); //envia mensagem com a temperatura e umidade
    }else if(text.equalsIgnoreCase(CHAVE1)){
      VerificarChave1(chatId); //envia mensagem com a temperatura e umidade
    }else if(text.equalsIgnoreCase(CHAVE2)){
      VerificarChave2(chatId); //envia mensagem com a temperatura e umidade
    }else if(text.equalsIgnoreCase(LDR)){
      VerificarLuminosidade(chatId); //envia mensagem com a temperatura e umidade
    }else if (text.equalsIgnoreCase(ESTADO)){
      VerificarEstado(chatId); //envia mensagem com o estado do relê, temperatura e umidade
    }else if (text == "/options") {
      String keyboardJson = "[[\"LED Azul\", \"LED Verde\", \"LED Vermelho\"],[\"botao\", \"luminosidade\"],[\"Estado\"]]";
      bot.sendMessageWithReplyKeyboard(chatId, "Escolha uma da opções:", "", keyboardJson, true);
      //bot.sendMessageWithInlineKeyboard(chatId, "Escolha uma opção:", "", keyboardJson);
    }
    else
    {
      handleNotFound(chatId); //mostra mensagem que a opção não é válida e mostra as opções
    }
  }//for
}

boolean validateSender(String senderId)
{
  //Para cada id de usuário que pode interagir com este bot
  for(int i=0; i<SENDER_ID_COUNT; i++)
  {
    //Se o id do remetente faz parte do array retornamos que é válido
    if(senderId == validSenderIds[i])
    {
      return true;
    }
  }

  //Se chegou aqui significa que verificou todos os ids e não encontrou no array
  return false;
}

void Inicia(String chatId, String fromName)
{
  //Mostra Olá e o nome do contato seguido das mensagens válidas
  String message = "<b>Olá " + fromName + ".</b>\n";
  message += getCommands();
  bot.sendMessage(chatId, message, "HTML");
}

String getCommands()
{
  //String com a lista de mensagens que são válidas e explicação sobre o que faz
  String message = "Os comandos disponíveis são:\n\n";
  message += "Para ligar o relé 1 <b>" + LIGAR1 + "</b>\n";
  message += "Para desligar o relé 1 <b>" + DESLIGAR1 + "</b>\n";
  message += "Para ligar relé 2 <b>" + LIGAR2 + "</b>\n";
  message += "Para desligar o relé 2 <b>" + DESLIGAR2 + "</b>\n";
  message += "Para verificar o estado da chave 1 <b>" + CHAVE1 + "</b>\n";
  message += "Para verificar o estado da chave 2 <b>" + CHAVE2 + "</b>\n";
  message += "Para verificar a <b>" + LDR + "</b>\n";
  message += "Para verificar o estado geral: <b>" + ESTADO + "</b>";
  return message;
}

void LigarRele1(String chatId){
  digitalWrite(PIN_RELE_1, HIGH);
  bot.sendMessage(chatId, "O relé 1 está <b>ativo!</b>  \xF0\x9F\x92\xA1", "HTML");  
}

void DesligarRele1(String chatId){
  digitalWrite(PIN_RELE_1, LOW);
  bot.sendMessage(chatId, "O relé 1 está <b>desativado!</b> \xF0\x9F\x92\xA1", "HTML");  
}

void LigarRele2(String chatId){
  digitalWrite(PIN_RELE_2, HIGH);
  bot.sendMessage(chatId, "O relé 2 está <b>ativo!</b>  \xF0\x9F\x92\xA1", "HTML");  
}

void DesligarRele2(String chatId){
  digitalWrite(PIN_RELE_2, LOW);
  bot.sendMessage(chatId, "O relé 2 está <b>desativado!</b> \xF0\x9F\x92\xA1", "HTML");  
}

void VerificarLuminosidade(String chatId){
  //Envia mensagem com o valor da temperatura e da umidade
  bot.sendMessage(chatId, getLDRMessage(), "HTML");
}

void VerificarChave1(String chatId){
  //Envia mensagem com o valor da temperatura e da umidade
  bot.sendMessage(chatId, getBTN1Message(), "HTML");
}

void VerificarChave2(String chatId){
  //Envia mensagem com o valor da temperatura e da umidade
  bot.sendMessage(chatId, getBTN2Message(), "HTML");
}

String getLDRMessage(){
  int val_ldr = analogRead(PIN_LDR);

  String message = "";
  message += "A luminosidade é de " + String(val_ldr)+ " LUX\n";
  return message;
  
}

String getBTN1Message(){
  String message = "";
  if(digitalRead(PIN_BTN_1)){
    message += "O botão está solto!\n";        
  }else{
    message += "O botão está pressionado! \xF0\x9F\x92\xA1\n";          
  }
  return message;  
}

String getBTN2Message(){
  String message = "";
  if(digitalRead(PIN_BTN_2)){     
    message += "O botão está solto!\n";        
  }else{
    message += "O botão está pressionado! \xF0\x9F\x92\xA1\n";     
  }
  return message;  
}

void VerificarEstado(String chatId)
{
  String message = "";

  if(digitalRead(PIN_RELE_1)){
    message += "O Relé 1 está <b>ativo!</b> \xF0\x9F\x92\xA1\n";  
  }else{
    message += "O Relé 1 está <b>desativado!</b>\n";  
  }

  if(digitalRead(PIN_RELE_2)){
    message += "O Relé 2 está <b>ativo!</b> \xF0\x9F\x92\xA1\n";  
  }else{
    message += "O Relé 2 está <b>desativado!</b>\n";  
  }

  //Adiciona à mensagem o valor da temperatura e umidade
  message += getLDRMessage();

  message += getBTN1Message();
  message += getBTN2Message();

  //Envia a mensagem para o contato
  bot.sendMessage(chatId, message, "HTML");
}

void handleNotFound(String chatId)
{
  //Envia mensagem dizendo que o comando não foi encontrado e mostra opções de comando válidos
  String message = "Comando não encontrado\n";
  message += getCommands();
  bot.sendMessage(chatId, message, "HTML");
}

void loop()
{
  //Tempo agora desde o boot
  uint32_t now = millis();

  if(!digitalRead(PIN_BTN_1)){
    bot.sendMessage("xxxxxxxxx", "BOTÃO 1 pressionado! \xF0\x9F\x92\xA1", "HTML");
  }

  if(!digitalRead(PIN_BTN_2)){
    bot.sendMessage("xxxxxxxxx", "BOTÃO 2 pressionado! \xF0\x9F\x92\xA1", "HTML");
  }

  //Se o tempo passado desde a última checagem for maior que o intervalo determinado
  if (now - lastCheckTime > INTERVAL) 
  {
    //Coloca o tempo de útlima checagem como agora e checa por mensagens
    lastCheckTime = now;
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    handleNewMessages(numNewMessages);
  }
}
