// Copyright (C)  2006  Emanuele Paolini <paolini@math.unifi.it>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

// adattato da Maurizio Paolini <paolini@dmf.unicatt.it>

#define VERSION "0.1"

/*
 * esempi di uso


 ./quindici --shuffle 50

 trova la sequenza ottimale per risolvere il gioco del 15 dopo averlo
 mescolato con una sequenza di 50 mosse casuali

 ./quindici 213456789ABCDFE. -e 28

 trova le eventuali sequenze con al piu' 28 mosse per risolvere
 il gioco a partire dalla configurazione

    2  1  3  4
    5  6  7  8
    9 10 11 12
   13 15 14 --

  NOTA: 10 e' sostituito da A, ecc.

 ./quindici 213456789ABCDFE. -s 26 -e 28

 prova solo le sequenze di 26,27,28 mosse

 ./quindici --help

 visualizza un elenco di opzioni disponibili.

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define MAXTASSELLI 62
static int numrighe = 4;
static int numcolonne = 4;
static int numtasselli = -1; // viene inizializzato in init

char mossamenouno[256]="A"; // e' utile avere una A alla posizione -1 
char *mossa=mossamenouno+1; // mosse fatte: valori in N, S, E, O
int nmosse=0;     
char configurazione[MAXTASSELLI];

static int x,y; // posizione casella vuota, a partire da 0 (tipicamente 3,3)
static int start=0;
static int end=10000;
static int shuffle=0;
static int quiet=0;
static unsigned int seed=1;

// #define schema(x,y) (configurazione[(x)%(numcolonne)+(y)*(numcolonne)])
#define schema(x,y) (configurazione[(x)+(y)*(numcolonne)])

// la casella vuota DEVE chiamarsi '.'
static char iniziale[MAXTASSELLI]="";
char finale[MAXTASSELLI]="";
//char *iniziale="123456789ABCDEF.";
//char *finale=  "123456789ACBDFE.";
//char *finale=  "132.56749AB8DFEC";
char *defaultconf= "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

time_t start_time;
unsigned long long count=0;

/* prototipi di funzioni */

void usage (int argc, char *argv[]);

void print_stato() {
  int i,j;
  for (i=0;i<numrighe;++i) {
    for (j=0;j<numcolonne;++j) {
      printf("%c",schema(j,i));
    }
    if (!quiet) printf("|");
  }
  printf(" ");
  if (!quiet) printf("(");
  for (i=0;i<nmosse;++i) {
    printf("%c",mossa[i]);
  }
  if (!quiet) printf(")");
  if (quiet) printf (" %d", nmosse);
  printf ("\n");
}

inline int check_fine() {
  int i;
  for (i=0;i<numtasselli && configurazione[i]==finale[i];++i);
  return i==numtasselli;
};

inline int valida(char m) {
  switch(m) {
  case 'N':
    return y>0 && mossa[nmosse-1]!='S';
  case 'S':
    return y<numrighe-1 && mossa[nmosse-1]!='N';
  case 'E':
    return x<numcolonne-1 && mossa[nmosse-1]!='O';
  default:
    assert(m=='O');
    return x>0 && mossa[nmosse-1]!='E';
  }
};

inline char mossa_casuale() {
  char m,mosse[]="NSEO";
  while (1) {
    m=mosse[random()%4];
    if (valida(m)) return m;
  }
}

inline char mossa_successiva(char m) {
  // m in {A, N, S, E, O} 
  // return {N, S, E, X}
  switch(m) {
  case 'A':
    if (valida('N')) return 'N';
    if (valida('S')) return 'S';
    if (valida('E')) return 'E';
    if (valida('O')) return 'O';
    return 'X';
  case 'N':
    if (valida('S')) return 'S';
    if (valida('E')) return 'E';
    if (valida('O')) return 'O';
    return 'X';
  case 'S':
    if (valida('E')) return 'E';
    if (valida('O')) return 'O';
    return 'X';
  case 'E':
    if (valida('O')) return 'O';
    return 'X';
  default:
    assert(m=='O');
    return 'X';
  }
};

inline void muovi(char m) {
  assert(schema(x,y)=='.');
  switch(m) {
  case 'N':
    assert(y>0);
    schema(x,y)=schema(x,y-1);
    y--;
    break;
  case 'S':
    assert(y<numrighe-1);
    schema(x,y)=schema(x,y+1);
    y++;
    break;
  case 'E':
    assert(x<numcolonne-1);
    schema(x,y)=schema(x+1,y);
    x++;
    break;
  case 'O':
    assert(x>0);
    schema(x,y)=schema(x-1,y);
    x--;
    break;
  default:
    assert(0);
  };
  schema(x,y)='.';
  mossa[nmosse++]=m;
};

inline void smuovi() {
  char m;
  assert(nmosse>0);
  m=mossa[nmosse-1];
  assert(schema(x,y)=='.');
  switch(m) {
  case 'S':
    assert(y>0);
    schema(x,y)=schema(x,y-1);
    y--;
    break;
  case 'N':
    assert(y<numrighe-1);
    schema(x,y)=schema(x,y+1);
    y++;
    break;
  case 'O':
    assert(x<numcolonne-1);
    schema(x,y)=schema(x+1,y);
    x++;
    break;
  case 'E':
    assert(x>0);
    schema(x,y)=schema(x-1,y);
    x--;
    break;
  default:
    assert(0);
  };
  schema(x,y)='.';
  nmosse--;
};
  
void successiva() {
  char m='X';
  int nmosse_save=nmosse;
  
  while (nmosse>0) {
    smuovi();
    m=mossa_successiva(mossa[nmosse]);
    if (m!='X') { // ho trovato una mossa da fare
      muovi(m);
      for (;nmosse<nmosse_save; muovi(mossa_successiva('A')));
      count++;
      return;
    }       
  }
  // devo aumentare il numero di mosse
  {
    time_t now=time(0);
    if (!quiet) printf("%llu configurazioni controllate con %d mosse\n",count,nmosse_save);
    if (now>=start_time+5 && !quiet)
      printf("tempo impiegato: %d sec. (%llu config/sec)\n",
	   (int)(now-start_time),count/(int)(now-start_time));
  }
  nmosse_save++;
  for (;nmosse<nmosse_save;muovi(mossa_successiva('A')));
  if (!quiet && nmosse < end) printf("n. mosse: %d\n",nmosse);
  count=1;  
  start_time=time(0);
  return;
};

void init(int start) {
  int i;
  srandom(seed);
  numtasselli = numrighe*numcolonne;
  if (iniziale[0] == 0) {
    strcpy (iniziale, defaultconf);
    iniziale[numtasselli-1] = '.';
    iniziale[numtasselli] = 0;
  }
  if (finale[0] == 0) {
    strcpy (finale, defaultconf);
    finale[numtasselli-1] = '.';
    finale[numtasselli] = 0;
  }
  for (i=0;i<numtasselli;++i) {
    configurazione[i]=iniziale[i];
  }
  configurazione[numtasselli] = 0;
  for (x = 0; x < numcolonne; x++) {
    for (y = 0; y < numrighe; y++) {
      if (schema(x,y) == '.') break;
    }
    if (schema(x,y) == '.') break;
  }
  if (!quiet) printf ("coordinate casella vuota: riga %d, colonna %d\n",y+1,x+1);
  if (x >= numcolonne || y >= numrighe) {
    printf ("errore nella configurazione iniziale!\n");
    exit(3);
  }
  nmosse=0;
  count=1;
  if (shuffle > 0) {
    for (i = 0; i < shuffle; i++) {
      muovi(mossa_casuale());
    }
    if (!quiet) {
      printf ("situazione dopo le mosse casuali:\n");
      print_stato();
    }
    nmosse = 0;
  }
  for (;nmosse<start;muovi(mossa_successiva('A')));
  start_time=time(0);
};

/*********/

int parse(int argc, char *argv[]) {
  int i;
  if (argc <= 1) {
    usage (argc, argv);
    return 0;
  }
  for (i = 1; i < argc; i++) {
    if (*argv[i] == '-')
    {
      if (strcmp(argv[i],"-s") == 0) {
        start = atoi(argv[++i]);
        if (!quiet) printf ("inizio con %d mosse\n", start);
      } else if (strcmp(argv[i],"-e") == 0) {
        end = atoi(argv[++i]);
        if (!quiet) printf ("finisco ricerca con %d mosse\n", end);
      } else if (strcmp(argv[i],"-i") == 0) {
        strcpy (iniziale, argv[++i]);
        if (!quiet) printf ("situazione iniziale: %s\n", iniziale);
      } else if (strcmp(argv[i],"-f") == 0) {
        strcpy (finale, argv[++i]);
        if (!quiet) printf ("situazione finale: %s\n", finale);
      } else if (strcmp(argv[i],"--seed") == 0) {
        seed = atoi(argv[++i]);
        if (!quiet) printf ("nuovo seme per sequenza casuale: %d\n", seed);
      } else if (strcmp(argv[i],"--shuffle") == 0) {
        shuffle = atoi(argv[++i]);
        if (!quiet) printf ("esegui %d mosse casuali\n", shuffle);
      } else if (strcmp(argv[i],"--size") == 0) {
        sscanf(argv[++i],"%dx%d", &numrighe, &numcolonne);
        if (!quiet) printf ("dimensione schema: %dx%d\n\n", numrighe, numcolonne);
      } else if (strcmp(argv[i],"-q") == 0) {
        quiet = 1;
      } else if (strcmp(argv[i],"--version") == 0) {
        printf ("VERSIONE: %s\n", VERSION);
        return 0;
      } else if (strcmp(argv[i],"--help") == 0) {
        usage (argc, argv);
        return 0;
      } else {
        printf ("opzione invalida: %s\n", argv[i]);
        exit(1);
      }
    } else {
      if (*iniziale == 0) {
        strcpy (iniziale, argv[i]);
        if (!quiet) printf ("situazione iniziale: %s\n", iniziale);
      } else {
        printf ("situazione iniziale gia definita!\n");
        exit(2);
      }
    }
  }
  return 1;
}

void usage(int argc, char *argv[])
{
  printf ("usage: %s situazione_iniziale (default:123456789ABCDEF.)\n", argv[0]);
  printf ("          [-f situazione_finale]   (default:123456789ABCDEF.)\n");
  printf ("          [-s start_moves]\n");
  printf ("          [-e end_moves]\n");
  printf ("          [--size <rows>x<cols>]   (dimensione schema)\n");
  printf ("          [--seed <intero>]   (inizializzazione sequenza casuale)\n");
  printf ("          [--shuffle n]       (muovi a caso n volte)\n");
  printf ("          [-q]     (pochi messaggi)\n");
  printf ("          [--help] (questo messaggio)\n");
  printf ("          [--version] (stampa il numero di versione)\n");
}

int main(int argc, char *argv[]) {
  iniziale[0] = finale[0] = 0;
  if (parse(argc, argv) == 0) return 0;
  init(start);
  //  print_stato();
  while(nmosse<=end) {
    if (check_fine()) {print_stato(); end=nmosse;}
    successiva();
  }
  return 0;
}
