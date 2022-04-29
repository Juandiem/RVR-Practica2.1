#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------


void ChatMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data
    char* tmp = _data;
    memcpy(tmp, &type, sizeof(uint8_t));
    tmp += sizeof(uint8_t);
    nick[nick.size()+1]='\0';
    memcpy(tmp, nick.c_str(), sizeof(char) * 8);
    tmp += sizeof(char) * 8;
    message[message.size()+1]='\0';
    memcpy(tmp, message.c_str(), sizeof(char) * 80);
}

int ChatMessage::from_bin(char * bobj)
{
  alloc_data(MESSAGE_SIZE);

  memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

  //Reconstruir la clase usando el buffer _data
  char* tmp = _data;
  memcpy(&type, tmp,sizeof(int8_t));
  tmp += sizeof(int8_t);
  nick.resize(sizeof(char) * 8);
  memcpy(&nick[0], tmp, sizeof(char) * 8);
  tmp += sizeof(char) * 8;
  message.resize(sizeof(char) * 80);
  memcpy(&message[0], tmp, sizeof(char) * 80);
  return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatServer::do_messages()
{
  while (true)
  {
      //Recibir Mensajes en y en función del tipo de mensaje
      // - LOGIN: Añadir al vector clients
      // - LOGOUT: Eliminar del vector clients
      // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
      while (true)
      {

          Socket* client;
          ChatMessage msg;
          socket.recv(msg, client);

          int count = 0;
          switch(msg.type)
          {
              case ChatMessage::LOGIN:
                  clients.push_back(client);
                  std::cout << msg.nick.c_str() << " logged in" << std::endl;
              break;
              case ChatMessage::LOGOUT:
                  std::cout << msg.nick.c_str() << " logged out" << std::endl;
                  for(Socket* sock: clients)
                  {
                      if((*sock == *client))
                      {
                          clients.erase(clients.begin() + count);
                          break;
                      }
                      count++;
                  }
              break;
              case ChatMessage::MESSAGE:
                  for(Socket* sock: clients)
                  {
                      if(!(*sock == *client))
                          socket.send(msg, *sock);
                  }

                  std::cout << msg.nick.c_str() << " sent a message" << std::endl;
              break;
          }

          std::cout << "Conected: " << clients.size() << std::endl;

      }
  }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatClient::login()
{
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGIN;

    socket.send(em, socket);
}

void ChatClient::logout()
{
    // Completar
}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        // Enviar al servidor usando socket
    }
}

void ChatClient::net_thread()
{
    while(true)
    {
        //Recibir Mensajes de red
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
    }
}
