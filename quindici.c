#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

char mossamenouno[256]="A"; // e' utile avere una A alla posizione -1 
char *mossa=mossamenouno+1; // mosse fatte: valori in N, S, E, O
int nmosse=0;     
char configurazione[16];
char schema[4][4]; //
int x,y; // posizione casella vuota (0-3)

#define schema(x,y) (configurazione[(x)%4+(y)*4])

// la casella vuota DEVE chiamarsi '*'
char *iniziale="123456789ABCDEF*";
//char *finale=  "123456789ACBDFE*";
char *finale=  "132*56749AB8DFEC";

time_t start_time;
unsigned long count=0;

void print_stato() {
  int i;
  for (i=0;i<4;++i) {
    printf("%c%c%c%c|",schema(0,i),schema(1,i),schema(2,i),schema(3,i));
  }
  printf(" (");
  for (i=0;i<nmosse;++i) {
    printf("%c",mossa[i]);
  }
  printf(")\n");
}

inline int check_fine() {
  int i;
  for (i=0;i<16 && configurazione[i]==finale[i];++i);
  return i==16;
};

inline int valida(char m) {
  switch(m) {
  case 'N':
    return y>0 && mossa[nmosse-1]!='S';
  case 'S':
    return y<3 && mossa[nmosse-1]!='N';
  case 'E':
    return x<3 && mossa[nmosse-1]!='O';
  default:
    assert(m=='O');
    return x>0 && mossa[nmosse-1]!='E';
  }
};

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
  assert(schema(x,y)=='*');
  switch(m) {
  case 'N':
    assert(y>0);
    schema(x,y)=schema(x,y-1);
    y--;
    break;
  case 'S':
    assert(y<3);
    schema(x,y)=schema(x,y+1);
    y++;
    break;
  case 'E':
    assert(x<3);
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
  schema(x,y)='*';
  mossa[nmosse++]=m;
};

inline void smuovi() {
  char m;
  assert(nmosse>0);
  m=mossa[nmosse-1];
  assert(schema(x,y)=='*');
  switch(m) {
  case 'S':
    assert(y>0);
    schema(x,y)=schema(x,y-1);
    y--;
    break;
  case 'N':
    assert(y<3);
    schema(x,y)=schema(x,y+1);
    y++;
    break;
  case 'O':
    assert(x<3);
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
  schema(x,y)='*';
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
    printf("%lu configurazioni controllate con %d mosse\n",count,nmosse_save);
    if (now!=start_time)
      printf("tempo impiegato: %d sec. (%f config/sec)\n",
	   (int)(now-start_time),(float)count/(float)(now-start_time));
  }
  nmosse_save++;
  for (;nmosse<nmosse_save;muovi(mossa_successiva('A')));
  printf("n. mosse: %d\n",nmosse);
  count=1;  
  start_time=time(0);
  return;
};

void init(int start) {
  int i;
  for (i=0;i<16;++i) {
    configurazione[i]=iniziale[i];
  }
  x=3;y=3;
  nmosse=0;
  count=1;
  for (;nmosse<start;muovi(mossa_successiva('A')));
  start_time=time(0);
};

int main(int argc, char *argv[]) {
  int start=0, end=10000;
  if (argc==2) {
    end=atoi(argv[1]);
  } else if (argc==3) {
    start=atoi(argv[1]);
    end=atoi(argv[2]);
  }
  init(start);
  //  print_stato();
  while(nmosse<=end) {
    if (check_fine()) print_stato();
    successiva();
  }
  return 0;
}
