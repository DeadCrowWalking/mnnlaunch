/* 
 * File:   beleg_client.cc
 * @uthor  Holger
 * 
 * Test-Client, Beleg Programmieren II
 * SS 2012
 * 
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

const int DEFAULT_PORT = 65000;


/**
 * initialisiert die Netzwerkschnittstelle und oeffnet einen TCP-Port
 *
 * return: socket descriptor
 */
int init_socket(unsigned short port, sockaddr_in* psa) {
  int socketfd = socket(PF_INET, SOCK_STREAM, 0);

  memset(psa, 0, sizeof(sockaddr_in));

  psa->sin_family = AF_INET;
  psa->sin_port   = htons(port);
  psa->sin_addr.s_addr = inet_addr("127.0.0.1");

  return socketfd;
}

/**
 * Ermittelt die Dateigroesse eines input file streams
 * @param   ifs filestream, von dem die Dateigroesse ermittelt werden soll 
 * @return  Dateigroesse des Streams
 */
long getFileSize(const ifstream& ifs) {
    long size = 0;

    if(ifs.good()) {
        filebuf *pbuf = ifs.rdbuf();
        size = pbuf->pubseekoff (0,ios::end,ios::in);
        pbuf->pubseekpos (0,ios::in);
    }
    
    return size;
}

/**
 * startet den eigentlichen Test und sendet die Nachrichten raus.
 *
 */
void runTest(int socketfd, const sockaddr_in& sa, const int nofiles, const char* const* filenames) {
  int n = connect(socketfd, (const sockaddr *) &sa, sizeof(sa));

  uint32_t cumulative_size = 0;
  
  if (n >= 0 && nofiles > 0) {
    ifstream* ifs = NULL;
    uint32_t* filesizes = NULL;
    try {
        ifs = new ifstream[nofiles];
        filesizes = new uint32_t[nofiles];
        
        for(int i=0; i < nofiles; i++) {
            ifs[i].open(filenames[i]);
            filesizes[i] = static_cast<uint32_t>(getFileSize(ifs[i]));
            cumulative_size += filesizes[i];
            cout << "Dateigroesse der Datei " << filenames [i];
            cout << ": " << filesizes[i] << " Bytes" << endl;
        }

        cout << "Groesse aller Dateien zusammen: " << cumulative_size << " Bytes" << endl; 

        /**
         * Kopiere alle Dateien zum Senden in einen Puffer, jeweils
         * mit der Länge der jeweiligen Datei davor
         */
        char *buffer = new char[cumulative_size+sizeof(filesizes[0])*nofiles];

        filebuf *pbuf = NULL;
        char *buf = buffer;
        uint32_t filesizeBE;

        for(int i=0; i < nofiles; i++) {
            pbuf = ifs[i].rdbuf();
            filesizeBE = htonl(filesizes[i]); 
            memcpy(buf, &filesizeBE, sizeof(filesizeBE));
            buf += sizeof(filesizeBE);
            pbuf->sgetn(buf, filesizes[i]);
            buf += filesizes[i];
        }
        
        /**
         * Groesse der hinzugefuegten Dateigroessen im Buffer
         * zur Gesamtgroesse addieren
         */
        cumulative_size += sizeof(filesizes[0])*nofiles;
        
        filesizeBE = htonl(cumulative_size);
        
        n = sendto(socketfd, &filesizeBE, sizeof(filesizeBE), 0, NULL, 0);
        cout << "Puffergroesse gesendet: " << cumulative_size << endl;
        
        n = sendto(socketfd, buffer, cumulative_size, 0, NULL, 0);
        cout << "Puffer gesendet mit " << n << " Bytes " << endl;
        
    }
    catch (bad_alloc& e) {
        cerr << "Dateien scheinen zu groß zum Senden zu sein: " << endl;
        cerr << e.what() << endl;
        perror("");
    }

    if(ifs != NULL) {
        for(int i=0; i < nofiles && ifs[i] != NULL; i++) {
                ifs[i].close(); 
        }
    }
  }
  else {
      perror("");
  }
}

/**
 * Gibt eine Fehlermeldung und die Aufrufsytax des Programms aus
 */
void printUsage(const char* progName, const char* text) {
  cerr << text << endl;
  cerr << "Usage: " << progName << " [-p <TCP port>] filename1 filename2 ..." << endl;
  cerr << endl;
}


/**
 * Startet das Test-Programm. Wenn als keine Portnummer als Parameter uebergeben wurde,
 * wird als Port-Nummer der Wert von DEFAULT_PORT verwendet, an welchen
 * die Nachrichten an localhost geschickt werden
 */
int main(int argc, char** argv) {
  sockaddr_in sa;
  int socketfd = -1;
  int port = DEFAULT_PORT;
  char** filenames = NULL;
  int ch, nofiles = 0;
  

  /*
   * Lesen des optional Kommandozeilen-Arguments -p <port> für die Portnummer
   */
  if((ch = getopt(argc, argv, "+:p:")) != -1) {
    if(ch == 'p') {
    
        port = atoi(optarg);
    
        if (port < 0 || port > 65535) {
                port = DEFAULT_PORT;
                cerr << "Portnummer ungueltig, Port " << DEFAULT_PORT << " wird stattdessen verwendet" << endl;
        }
        if (port == 0) {
                printUsage(argv[0], "TCP portnummer ungueltig");
                return EXIT_FAILURE;
        }
    }
  }
  

  
  /*
   * Lesen der Dateinamen
   */
  if(optind < argc) {
        nofiles = argc - optind;
        filenames = new char*[nofiles];
        for(; optind < argc; optind++) {
            filenames[nofiles-1 -(argc - optind -1)] = argv[optind];
        }
  }
  else {
        printUsage(argv[0], "Kein Dateiname angegeben");
        return EXIT_FAILURE;
  }

  socketfd = init_socket(port, &sa);

  if(socketfd <= 0) {
        perror("Fehler beim Initialisieren des Netzwerks");
  }
  else {
        runTest(socketfd, sa, nofiles, filenames);
  }
  
  if(filenames != NULL) {
        delete[] filenames;
  }

  if(socketfd > -1) {
      close(socketfd);
  }

  return 0;
}

