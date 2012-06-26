/* 
 * File:   tcp_read.h
 * @author Holger Ahl
 *
 */

#ifndef TCP_READ_H
#define	TCP_READ_H

namespace Beleg {
  namespace SS12 {

    class TCPSocket { 
        #ifdef DEFAULT_TCP_PORT
            static const unsigned short DEFAULT_TCP_PORT = ::DEFAULT_TCP_PORT;
        #else
            static const unsigned short DEFAULT_TCP_PORT = 65000;
        #endif

        int socketfd;

    public:
       /**
        * Constructor
        * erzeugt ein neues TCP socket, bindet sich an den uebergebenen 
        * TCP port für alle IP-Adressen des hosts
        * und wartet auf eingehende Nachrichten
        * 
        * @param port TCP port zum Binden
        */
       TCPSocket(unsigned short port=DEFAULT_TCP_PORT);
       
       /**
        * Destructor
        * schließt die Netzwerkverbindung zum im Konstruktor 
        * angegebenen Port, so dass auf diesen nicht mehr auf
        * Verbindungsanfragen gelauscht wird
        * 
        */
       ~TCPSocket();
       
       
       /**
        * Funktion liest eine Nachricht vom Netzwerk. Fuer die 
        * Nachricht wird dynamisch ein Puffer alloziert, der der Groesse der
        * empfangenen Nachricht entspricht. Der Zeiger auf diesen Puffer wird
        * ueber den Referenzparameter msg zurueck gegeben.
        *
        * @param  msg Puffer, in welchen die Nachricht gelesen wird
        * 
        * @return Die Funktion gibt die Anzahl der empfangenen Bytes zurueck. Im
        *         Fehlerfall gibt die Funktion einen Wert kleiner 0 zurueck.
        */
       long readMessageFromNet(char*& buffer);
    };
  }
}


#endif	/* TCP_READ_H */

