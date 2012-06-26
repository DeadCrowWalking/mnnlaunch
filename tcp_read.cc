#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <iostream>

#include "tcp_read.h"

/**
 * Initialisierung der TCP-Netzwerk-Anbindung
 * erzeugt und speichert im Objekt einen socket-Descriptor
 * für die TCP-Verbindung zu diesem Port
 * 
 * @param port TCP-Portnummer, ueber welchen Daten empfangen werden
 *
 */
Beleg::SS12::TCPSocket::TCPSocket(unsigned short port):socketfd(0) {
  int err = 0, opt = 1;
  socketfd = socket(PF_INET, SOCK_STREAM, 0);

  if (socketfd >= 0) {
    sockaddr_in sa;
    std::memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port   = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if((err = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt))) < 0) {
      socketfd = err;
    } 
    else {
      if ((err = bind(socketfd, (const sockaddr *) &sa, sizeof(sa))) < 0) {
        socketfd = err;
      }
      else {
        if ((err = listen(socketfd, 1)) != 0) {
          socketfd = err;
        }
      }
    }
  }
}

/**
 * De-Initialisierung der TCP-Netzwerk-Anbindung
 * Server-Socket wird geschlossen, so dass keine Verbindungsanfragen 
 * mehr akzeptiert werden
 *
 */
Beleg::SS12::TCPSocket::~TCPSocket() {
  if (socketfd >= 0) {
      close(socketfd);
  }
}

/**
 * Funktion liest eine Nachricht vom Netzwerk. Fuer die 
 * Nachricht wird dynamisch ein Puffer alloziert, der der Groesse der
 * empfangenen Nachricht entspricht. Der Zeiger auf diesen Puffer wird
 * ueber den Referenzparameter msg zurueck gegeben.
 *
 * @param buffer   Puffer, in welchen die Nachricht gelesen wird
 * @return         Die Funktion gibt die Anzahl der empfangenen Bytes zurueck.
 *                 Im Fehlerfall gibt die Funktion einen Wert kleiner 0 zurueck.
 */
long Beleg::SS12::TCPSocket::readMessageFromNet(char*& buffer) {
  int len_inet, con;
  long n = 0;
  uint32_t msg_len=0, msg_lenBE = 0;
  struct sockaddr_in adr_clnt;

  buffer = NULL;

  if (socketfd >= 0) {
    len_inet = sizeof(adr_clnt);
    if( (con = accept(socketfd, (struct sockaddr *) &adr_clnt, (socklen_t *) &len_inet)) >= 0) {

      n = recv(con, &msg_lenBE, sizeof(msg_lenBE), MSG_WAITALL);
      msg_len = htonl(msg_lenBE);
      
      if(n > 0 && msg_len > 0) {
         buffer = new char[msg_len];
         n = recv(con, buffer, msg_len, MSG_WAITALL);
      }

      close(con);
    }
    else {
      n = con;
    }
  }
  else {
    n = socketfd;
  }

  if(n < 0) {
    perror("Error: ");
  }

  return n;
}


