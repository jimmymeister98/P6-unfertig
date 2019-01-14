#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define ANKUNFT 0
#define VERLASSEN 1
#define MESSUNG 2
#define ENDE 100

struct queue_t {
    char *name ;
    int size ;
    int entries ;
    double time ;
    struct packet_t **packets;
    int read;
    int write;
    long lost;

};typedef struct queue_t queue_t;

struct packet_t {
    char *name;
    struct queue_t *queue;
};

typedef struct event_t{
    int type;
    double time;
    long serial;
    struct event_t* next;
    struct event_t* prev;
}event_t;

typedef struct eventlist_t{
    int entries;
    double time;
    long serial;
    struct event_t* head;
    struct event_t* tail;
}eventlist_t;

eventlist_t *eventlist;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void event_init(){
    eventlist= (eventlist_t*) malloc (sizeof(eventlist_t)); //Alles null setzen (Liste, nicht element!)
    eventlist->entries = 0;
    eventlist->serial = 0L;             //L weil long
    eventlist->time = 0;
    eventlist->head = NULL;
    eventlist->tail = NULL;
}

double myrand(){
    double r = (double)rand()/(RAND_MAX)+1;
    return r;                               //Doublerand
}

long event_store(int type, double time){
    long serNr;
    event_t *event, *pos;  //Erstelle elemente die auf event vom typ pointer auf event_t

    event = (event_t*) malloc (sizeof(event_t));    //Speicherzuweisung in höhe vom element
    eventlist->serial++;                            //Serial +1 da neues element
    serNr = eventlist->serial;                      //serial von liste wird element zugewiesen
    event->serial = serNr;                          //s.o
    event->prev = NULL;                             //zunächst auf NULL setzen da noch nicht eingereiht
    event->next = NULL;
    eventlist->entries++;                           //Neuer eintrag -> einträge +1
    event->type = type;                             //Übergebener type wird dem event zugewiesen
    event->time = /*eventlist->time +*/ time;           //0+randtime


    if(eventlist->head == NULL){        // Wenn eventlist leer dann->
        eventlist->head = event;        //head und tail auf event
        eventlist->tail = event;
        return serNr;                   //raus bzw wieder in main
    }
    pos = eventlist->head;              //Zeige auf head
    while(pos != NULL && event->time > pos->time) //Wenn Position ungleich null und Ereig etime=ptime   Nach time sortieren<----------------------
        pos=pos->next;


    //1. Fall Anfang einfügen
    if(pos == eventlist->head){ //Wenn mom. pos. der head ist dann s.o
        event->next = pos;      //
        pos->prev = event;
        eventlist->head=event;  //Das element event wird an den head von der eventlist geschrieben da
        //eventlist->tail->next=NULL;
        return serNr;
    }
    //2. Fall Ende einfügen tritt ein sobald pos == NULL
    if(pos==NULL){                          //mit pos=pos->next zu letztem Ereignis. dessen next = NULL
        eventlist->tail->next = event;      //Der ursprüngliche tail , davon der next wird auf's momentane event geändert um den ursprünglichen NULL pointer aufs neue event zu verweisen
        event->prev = eventlist->tail;      //tail ist bisher das Ende; event->prev soll auf bisheriges ende-event zeigen
        eventlist->tail = event;            //Neuer tail wird verpointert (das event von 2 zeilen vorher)
        return serNr;
    }
    //3. Fall mittendrin einfügen
    pos->prev->next = event;                //vom vorherigen element der next pointer zeigt jetzt auf neues element
    event->prev = pos->prev;                //der "vorher" pointer vom neuen elem. wird gleich der vorher pointer von der mom. pos
    event->next = pos;
    pos->prev = event;
    return serNr;
}

long event_retrieve(int* type, double* time){
    long serNr;
    event_t *event;

    event = eventlist->head;                    //Nimm head der liste
    *type = event->type;
    *time = event->time;
    serNr=eventlist->head->serial;              //Seriennummer vom head beziehen
    eventlist->head=eventlist->head->next;      //mach nächstes elem zum neuen head
    eventlist->entries--;                       //verringere einträge um 1

    if(eventlist->entries >= 0){                //Solange elem. vorhanden sind dann elem löschen sonst liste löschen
        free(event);
        if(eventlist->entries == 0){
            free(eventlist);
        }
        return serNr;
    }
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////CODE VON P5 ZUENDE///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct queue_t *queue_create(char *name, int size){                          //ich erstelle mir eine queue
    struct queue_t *queue;
    queue = (struct queue_t *) malloc(sizeof(struct queue_t));
    // queue->name = (struct queue_t*)malloc(50+1);
    queue->packets = (struct packet_t **)malloc(sizeof(struct packet_t)*size);
    //queue->name = (struct queue_t *) malloc(sizeof(queue_t->name));
    if(queue == NULL) exit(1337);//wenn speicher nicht freigegeben wird dann raus
    if(size <=0){printf("Queue zu klein, beende"); exit(8008);}
    queue->name = name;          //Siehe notizen 30.11.18 " a->eigenschaft == (a*).
    queue->size = size;         //verändere das element size in der erstellten struktur hilf Über die übergebene variable size
    queue->write = 0;
    queue->time = 0.0;
    queue->read = 0;
    queue->entries = 0;
    queue->lost = 0;
    // queue->packets = 0;
    queue->time = 0;
    //printf("%d",sizeof(hilf->read));     //Prüfstruktur : irrelevant
    return queue;       //returnwert entspricht der hilfsvariable/hilfsstruktur
}


long total = 0;
long queue_store(struct queue_t *queue, struct packet_t *packet){

    if(queue->entries >= queue->size){
        queue->lost++;
        //  printf("Paket verworfen, queue voll?");
        return 0;
    }

    //  printf("Speichere Paket: (%s) in Queue: (%s) ", packet->name, queue->name);

    queue->entries++;

    // queue->packets = (struct packet_t **)realloc(queue->packets, sizeof(struct packet_t) * queue->entries); //dynamische speicherneuzuweisung um minimalen speicher zu benutzen
    queue->packets[queue->write] = packet;

    if (queue->size == queue->write)
        queue->write=0;
    else
        queue->write++; //ringbuffer
    packet->queue = queue;
    total++;

    //printf("Abgespeichert!\n"),queue->packets[queue->write-1];

    return total;
}

struct packet_t * packet_create(char *name) {
    static int a = 0;
    struct packet_t *packet = (struct packet_t *) malloc(sizeof(struct packet_t));
    if (NULL == packet)
        printf("Fehler bei speicherreservierung für paket");
    // packet
    // packet->queue = malloc(sizeof(struct queue_t));
    packet->name = malloc (sizeof(name));
    if (NULL == packet->name)
        printf("Fehler bei speicherreservierung mit name(paket)");
    strcpy(packet->name,name);
    packet->queue = NULL;
    //todo   printf("(Paketadresse : %x)\n",packet);
    //struct packet_t* pointer = &packet;
    //printf("%dtes. paket\n",a);
    a++;
    return packet;
}
int packet_destroy(struct packet_t* packet) {
    static int i = 1;
    if (packet == NULL){
        printf("Fehlerhaftes Paket! (P. Destroy");
    }
    //  free(packet->name);
    // free(packet->queue); //TODO Packet aus der queue lösen ohne namen zu löschen
    //packet->queue=NULL;
    //  free(packet->name);
    //printf("%d Adresse %x\n",i,packet);
    free(packet);
    i++;//free packet
    return 1;

}

struct packet_t* queue_retrieve(struct queue_t* queue) {
    if (NULL == queue) {
        printf("Queue Leer (Retrieve)");
        return NULL;
    }
    if (queue->entries > 0) {
        queue->entries--;
        struct  packet_t* packetPointer = queue->packets[queue->read];
//        packet_destroy(packetPointer);
        queue->packets[queue->read] = NULL;
        if (queue->size == queue->read)
            queue->read=0;
        else
            queue->read++; //ringbuffer
        // queue->read = (queue->read++) % queue->size; //read pos. ändern
        return packetPointer;
    }
    return NULL;
}

int queue_destroy(struct queue_t * queue){
    int hilfsvariable = (queue->entries = 0);
    // free(queue->name);
    free(queue->packets);
    free(queue);
    return hilfsvariable;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////CODE VON P4 ZUENDE///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





int main(void) {
    int anzahl = 0;
    struct queue_t *Q;                             // Zeiger auf Warteschlange
    struct packet_t *P;
    int n, type;
    long serNr;
    double zeit;
    static int kundenmenge = 0;
    srand(time(NULL));
    clock_t start_t, ende_t;
    start_t = clock();

    event_init(eventlist);
   // event_store(0, 0.1);
    event_store(100, 100);
    for (n = 0; n < 98; n++) {
        event_store(rand() % 3, myrand() * 10); //rand0-2,myrand*10 -> 10er Schritte
    }


    do {
        serNr = event_retrieve(&type, &time);
        //for(n=0;n<100;n++){
        anzahl++;
        switch (type) {

            case ANKUNFT:

                if (kundenmenge <= 0)
                    queue_create("Kasse", 15); //Bedienung
                packet_create("Kunde");
                queue_store("Kasse", "Kunde");
                break;

            case VERLASSEN:
                queue_retrieve("Kasse");
                break;
            case MESSUNG:
                printf("Queue hat %i einträge,%li kunden sind verloren gegangen(größe%i)", Q->entries, Q->lost,
                       Q->size);
                break;
            case 100:
                exit (0);
        }
    } while(anzahl!=100);


}
      //  serNr = event_retrieve (&type, &zeit);

      //  printf("Seriennummer:%3ld Zeit:\t%g Type:\t%d\n",serNr,zeit,type);
     //   ende_t=clock();
   // printf("Berechnung dauert %.4f Sekunden.\n", (double)(ende_t-start_t)/CLOCKS_PER_SEC);}
//////////////////////////////////////////////////
///P5 Block zum erstellen eine prioritätsmenge////
//////////////////////////////////////////////////

