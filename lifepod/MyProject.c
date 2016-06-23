#include "RFC.c"


/* LCD
 ****************/

// LCD pinout
sbit LCD_RS at RE2_bit;
sbit LCD_EN at RE1_bit;
sbit LCD_D7 at RD3_bit;
sbit LCD_D6 at RD2_bit;
sbit LCD_D5 at RD1_bit;
sbit LCD_D4 at RD0_bit;

// LCD pin direction
sbit LCD_RS_Direction at TRISE2_bit;
sbit LCD_EN_Direction at TRISE1_bit;
sbit LCD_D7_Direction at TRISD3_bit;
sbit LCD_D6_Direction at TRISD2_bit;
sbit LCD_D5_Direction at TRISD1_bit;
sbit LCD_D4_Direction at TRISD0_bit;


/* Teclado
 ****************/
 
char tecla = 'x';
char teclaTemp;

char key[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

#define DISABLE_KB() INTCON.INT0IE = 0
#define ENABLE_KB()  INTCON.INT0IE = 1

#define LOTTERY    '1'
#define CHANCE     '2'
#define MARRIAGE   '3'
#define HOUSE      '5'
#define CAR        '6'
#define BABY       '7'
#define YEARS      '9'
#define SPIN       'A'
#define NEXT       'B'
#define UNDO       'C'
#define ENTER      'D'
#define TOGGLE_SLP '*'
#define TOGGLE_MM  '#'



/* RFID
 ****************/

// RFID pinout
sbit MFRC522_Rst at RB7_Bit;
sbit SoftSPI_SDI at RB6_Bit;
sbit SoftSPI_SDO at RB5_Bit;
sbit SoftSPI_CLK at RB4_Bit;
sbit MFRC522_CS  at RB3_Bit;

// RFID pin direction
sbit MFRC522_Rst_Direction at TRISB7_Bit;
sbit SoftSPI_SDI_Direction at TRISB6_Bit;
sbit SoftSPI_SDO_Direction at TRISB5_Bit;
sbit SoftSPI_CLK_Direction at TRISB4_Bit;
sbit MFRC522_CS_Direction  at TRISB3_Bit;

#define PLAYER1 0x03
#define PLAYER2 0x13
#define PLAYER3 0x55
#define PLAYER4 0x95


/* Random
 ****************/

#define RAND_MAX 32767
// from http://stackoverflow.com/a/17554531
unsigned int rand_interval(unsigned int min, unsigned int max) {
    int r;
    unsigned int range = 1 + max - min;
    unsigned int buckets = RAND_MAX / range;
    unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}


/* Estado
 ****************/

char state = 0;

char num_jogadores = 0;
int saldos[4] = {0, 0, 0, 0};
int pontos[4] = {0, 0, 0, 0};
int salarios[4] = {5000, 5000, 5000, 5000};
char casados[4] = {0, 0, 0, 0};
char bebes[4] = {0, 0, 0, 0};
char carros_econ[4] = {0, 0, 0, 0};
char carros_luxo[4] = {0, 0, 0, 0};
char casas_peq[4] = {0, 0, 0, 0};
char casas_med[4] = {0, 0, 0, 0};
char casas_gra[4] = {0, 0, 0, 0};


/* Mostradores
 ****************/

void mostra_subindo(int inicial, int final) {

}

void mostra_S(int valor) {

}

void mostra_subindo_S(int inicial, int final) {

}

void mostra_LP(int valor) {

}

void mostra_subindo_LP(int inicial, int final) {

}

#define MUDA_S(antigo, novo)  tmp = (novo);  mostra_subindo_S(antigo, (novo));  antigo = (novo);
#define MUDA_LP(antigo, novo) tmp = (novo); mostra_subindo_LP(antigo, (novo));  antigo = (novo);

int gira_roleta(int ate, int bias) {
  int resultado = rand_interval(0, ate);
  resultado = max(resultado+bias, ate);
  
  // mostrar no display
  
  return resultado;
}


/* Jogabilidade
 ****************/

void turno(int num_jogador) {
  int n = num_jogador;
  int tmp, bias;
  
  // Subtrai $ juros
  if (saldos[n] < 0) {
    MUDA_S(saldos[n], saldos[n] * 1.1);
  }
  
  // Soma $ salario
  MUDA_S(saldos[n], saldos[n] + salarios[n]);

  // Subtrai $ por crianca
  // Soma LP por crianca
  if (bebes[n] > 0) {
    MUDA_S(saldos[n], saldos[n] - salarios[n] * max(bebes[n], 4) / 10);
    MUDA_LP(pontos[n], pontos[n] + 350 * bebes[n]);
  }
  
  // Soma LP casamento
  if (casados[n]) {
    MUDA_LP(pontos[n], pontos[n] + 1500);
  }
  
  // Soma LP casa
  if (casas_peq[n] || casas_med[n] || casas_gra[n]) {
    MUDA_LP(pontos[n], pontos[n] + 100*(casas_peq[n]+casas_med[n]+casas_gra[n]));
  }

  // Soma LP carro_econ
  // Subtrai $ carro_econ
  if (carros_econ[n] > 0) {
    MUDA_LP(pontos[n], pontos[n] + 100);
    MUDA_S(saldos[n], saldos[n] - 1000 * carros_econ[n]);
  }
  
  // Soma LP carro_luxo
  // Subtrai $ carro_luxo
  if (carros_luxo[n] > 0) {
    MUDA_LP(pontos[n], pontos[n] + 200 * carros_luxo[n]);
    MUDA_S(saldos[n], saldos[n] - 5000 * carros_luxo[n]);
  }
  
  // Gira a roleta
  bias = (carros_econ[n] ? 1 : 0) + (carros_luxo[n] ? 2 : 0);
  gira_roleta(10, bias);
}

/* Interações
 ****************/
 
void GotCard(int card) {

}

void GotKey(char key) {

}



/* MAIN
 ****************/

void main() {
  char tmp; 
  char i, size;
  unsigned TagType;
  char msg[12];
  char UID[6];
  
  ADCON1 = 0X06;

  TRISB = 255;  // entrada
  TRISC = 0b11110000;  // alto entrada, baixo saida
  PORTC = 0;
  TRISD = 0;           // saida

  // liga LCD
  Lcd_Init();
  Lcd_Cmd(_LCD_CURSOR_OFF);
  Lcd_Cmd(_LCD_CLEAR);
  delay_ms(10);
  Lcd_Out(1, 1, "picles");
  
  // liga teclado
  INTCON.GIE = 1;       // global interrupt enable
  INTCON.PEIE = 1;      // peripheral interrupt enable
  PORTC.F0 = PORTC.F1 = PORTC.F2 = PORTC.F3 = 1;
  INTCON.INT0IE = 1;
  INTCON.INT0IF = 0;
  
  // liga RFID
  Soft_SPI_Init();
  MFRC522_Init();

  do {
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Chr(2, 1, tecla);
    
   if (MFRC522_isCard(&TagType)) {
     if (MFRC522_ReadCardSerial(&UID)) {
       if (UID[0] == PLAYER1) {
         GotCard(0);
       } else if (UID[0] == PLAYER2) {
         GotCard(1);
       } else if (UID[0] == PLAYER3) {
         GotCard(2);
       } else if (UID[0] == PLAYER4) {
         GotCard(3);
       }
     }
   }
    
    delay_ms(33);
  } while(1);
}


/* Auxiliares teclado e RFDI
 ****************/

char getKey() {
  char var;

  PORTC.F0 = PORTC.F1 = PORTC.F2 = PORTC.F3 = 0;
  delay_ms(5);

  PORTC.F0 = 1;
  var = PORTC;
  if      (var.F4 == 1) { return '1'; }
  else if (var.F5 == 1) { return '2'; }
  else if (var.F6 == 1) { return '3'; }
  else if (var.F7 == 1) { return 'A'; }
  Delay_ms(5);
  PORTC.F0 = 0;

  PORTC.F1 = 1;
  var = PORTC;
  if      (var.F4 == 1) { return '4'; }
  else if (var.F5 == 1) { return '5'; }
  else if (var.F6 == 1) { return '6'; }
  else if (var.F7 == 1) { return 'B'; }
  Delay_ms(5);
  PORTC.F1 = 0;

  PORTC.F2 = 1;
  var = PORTC;
  if      (var.F4 == 1) { return '7'; }
  else if (var.F5 == 1) { return '8'; }
  else if (var.F6 == 1) { return '9'; }
  else if (var.F7 == 1) { return 'C'; }
  Delay_ms(5);
  PORTC.F2 = 0;

  PORTC.F3 = 1;
  var = PORTC;
  if      (var.F4 == 1) { return '*'; }
  else if (var.F5 == 1) { return '0'; }
  else if (var.F6 == 1) { return '#'; }
  else if (var.F7 == 1) { return 'D'; }
  Delay_ms(5);
  PORTC.F3 = 0;
  
  return 'x';
}


/* Interrupt
 ****************/

void interrupt() {
  // keyboard interrupt
  if(INTCON.INT0IF == 1) {
    DISABLE_KB();
    teclaTemp = GetKey();
    if (teclaTemp != 'x') {
      tecla = teclaTemp;
    }
    PORTC.F0 = PORTC.F1 = PORTC.F2 = PORTC.F3 = 1;
    
    GotKey(tecla);

    // clear flag
    ENABLE_KB();
    INTCON.INT0IF = 0;
  }
}
