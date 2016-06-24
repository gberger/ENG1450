#include "RFC.c"


/* Random
 ****************/

#define RAND_MAX 32767
unsigned int rand_interval(unsigned int min, unsigned int max) {
  // from http://stackoverflow.com/a/17554531
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

char disp1[17] = "Inicializando...";
char disp2[17] = "                ";


/* Teclado
 ****************/

char tecla = 'x';
char teclaTemp;
char tratou_key = 0;

#define DISABLE_KB() INTCON.INT0IE = 0
#define ENABLE_KB()  INTCON.INT0IE = 1

#define LOTTERY    '1'
#define CHANCE     '2'
#define MARRIAGE   '3'
#define HOUSE      '5'
#define CAR        '6'
#define BABY       '7'
#define YEARS      '9'
#define SALARY     '0'
#define SPIN       'A'
#define NEXT       'B'
#define UNDO       'C'
#define ENTER      'D'
#define TOGGLE_SLP '*'
#define TOGGLE_MM  '#'
#define IS_NUMERIC(key) (key >= '0' && key <= '9')
#define NUMERIC_VAL(key) (key - '0')


#define APPEND(num, digit) (num = num*10 + digit)


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
sbit MFRC522_CS_Direction at TRISB3_Bit;

#define PLAYER1 0x03
#define PLAYER2 0x13
#define PLAYER3 0x55
#define PLAYER4 0x95

char rfid_enabled = 1;


/* Estado
 ****************/

// state -- ver defines abaixo
char state = 0;

#define ST_INIT         0
#define ST_SET_YEARS    1
#define ST_BEFORE_GAME  2
#define ST_TURN         3
#define ST_SPIN         4
#define ST_CHANCE       5
#define ST_MARRIAGE     6
#define ST_HOUSE        7
#define ST_CAR          8
#define ST_BABY         9
#define ST_ADD_SUB      10
#define ST_SALARY       11
#define ST_LOTTERY      12
#define ST_LOTTERY_WAIT 13
#define ST_END          14

#define GO_STATE(S) state = S; StartState();


// variaveis internas
long tmp_input;
char tmp_toggle = '$';

int current_casa = 0;
long casas_precos[3] = {200000, 500000, 1000000};
char *casas_nomes[3] = {"P", "M", "G"};

int current_carro = 0;
long carros_precos[2] = {10000, 50000};
char *carros_nomes[2] = {"Econ", "Luxo"};

int current_bebes = 0;

long val_lottery = 0;
int num_lottery = -1;


// Estado do jogo
int total_anos;
int ano_atual = 0;
char jogando[4] = {0, 0, 0, 0};
char girou[4] = {0, 0, 0, 0};
int current_player = -1;
int winner;
long ratio_lp_money;

long saldos[4] = {0, 0, 0, 0};
long pontos[4] = {0, 0, 0, 0};
long salarios[4] = {5000, 5000, 5000, 5000};
char casados[4] = {0, 0, 0, 0};
char bebes[4] = {0, 0, 0, 0};
char carros[2][4] = {{0,0,0,0}, {0,0,0,0}};
char casas[3][4] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}};

#define CASAS_PEQ casas[0]
#define CASAS_MED casas[1]
#define CASAS_GRA casas[2]
#define CARROS_ECON carros[0]
#define CARROS_LUXO carros[1]


/* Mostradores
 ****************/

int __muda_tmp;
#define MUDA_S(antigo, novo)  antigo = (novo);
#define MUDA_LP(antigo, novo) antigo = (novo);

void mostra_stats(int n) {
  sprintf(disp1, "$%lld   jog%d", saldos[n], current_player+1);
  sprintf(disp2, "LP %lld ano%d", pontos[n], ano_atual);
}

void mostra_preco_casa() {
  if (casas[current_casa][current_player]) {
    sprintf(disp1, "%s: +$%lld",
                 casas_nomes[current_casa],
                 casas_precos[current_casa]);
  } else {
    sprintf(disp1, "%s: -$%lld",
                 casas_nomes[current_casa],
                 casas_precos[current_casa]);
  }

  if (CASAS_PEQ[current_player] == 0 && CASAS_MED[current_player] == 0 && CASAS_GRA[current_player] == 0) {
    sprintf(disp2, "Possui: NENHUM");
  } else {
    strcpy(disp2, "Possui: ");
    if (CASAS_PEQ[current_player]) {
      strcat(disp2, "P ");
    }
    if (CASAS_MED[current_player]) {
      strcat(disp2, "M ");
    }
    if (CASAS_GRA[current_player]) {
      strcat(disp2, "G");
    }
  }
}


void mostra_preco_carro() {
  if (carros[current_carro][current_player]) {
    sprintf(disp1, "%s: +$%lld",
                 carros_nomes[current_carro],
                 carros_precos[current_carro]);
  } else {
    sprintf(disp1, "%s: -$%lld",
                 carros_nomes[current_carro],
                 carros_precos[current_carro]);
  }
  
  if (CARROS_ECON[current_player] == 0 && CARROS_LUXO[current_player] == 0) {
    sprintf(disp2, "Possui: NENHUM");
  } else if (CARROS_ECON[current_player] == 1 && CARROS_LUXO[current_player] == 0) {
    sprintf(disp2, "Possui: ECON");
  } else if (CARROS_ECON[current_player] == 0 && CARROS_LUXO[current_player] == 1) {
    sprintf(disp2, "Possui: LUXO");
  } else {
    sprintf(disp2, "Possui: AMBOS");
  }
}

void mostra_bebes() {
  sprintf(disp1, "# bebes: %d + %d", bebes[current_player], current_bebes);
}

void mostra_add_sub() {
  if (tmp_toggle == '$') {
    sprintf(disp2, "$ %lld", tmp_input);
  } else {
    sprintf(disp2, "LP %lld", tmp_input);
  }
}


/* Giradores
 ****************/

int gira_roleta(int bias) {
  int resultado = rand_interval(1, 10);
  resultado = min(resultado+bias, 10);

  // mostrar girando no display

  return resultado;
}

int gira_chance() {
  int resultado = rand_interval(0, 2);

  // mostrar girando no display

  return resultado;
}


/* Jogabilidade
 ****************/

int turno(int n) {
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
  if (CASAS_PEQ[n] || CASAS_MED[n] || CASAS_GRA[n]) {
    MUDA_LP(pontos[n], pontos[n] + 100*(CASAS_PEQ[n]+CASAS_MED[n]+CASAS_GRA[n]));
  }

  // Soma LP carro_econ
  // Subtrai $ carro_econ
  if (CARROS_ECON[n] > 0) {
    MUDA_LP(pontos[n], pontos[n] + 100);
    MUDA_S(saldos[n], saldos[n] - 1000 * CARROS_ECON[n]);
  }

  // Soma LP carro_luxo
  // Subtrai $ carro_luxo
  if (CARROS_LUXO[n] > 0) {
    MUDA_LP(pontos[n], pontos[n] + 200);
    MUDA_S(saldos[n], saldos[n] - 5000 * CARROS_LUXO[n]);
  }

  // Gira a roleta
  return gira_roleta((CARROS_ECON[n] ? 1 : 0) + (CARROS_LUXO[n] ? 2 : 0));
}

void casar(int n) {
  int i;
  long ganho = 0;
  if (!casados[current_player]) {
    // Casar, ganha 1000 de cada player
    for (i = 0; i < 4; i++) {
      if (jogando[i] && i != current_player) {
        saldos[i] -= 1000;
        ganho += 1000;
      }
    }
  }
  else {
    // Aniversario, ganha 500 de cada player
    for (i = 0; i < 4; i++) {
      if (jogando[i] && i != current_player) {
        saldos[i] -= 500;
        ganho += 500;
      }
    }
  }
  // Ganha o pot
  MUDA_S(saldos[current_player], saldos[current_player] + ganho);

  // Muda variavel
  casados[current_player] = 1;
  
  // Ganha 3000 LP
  MUDA_LP(pontos[current_player], pontos[current_player]+3000);
}

long calculate_points(int n) {

}

int calculate_winner() {


}

/* Interações
 ****************/
 
void StartState() {
  if (state == ST_SET_YEARS) {
    tmp_input = 0;
    strcpy(disp1, "Digite anos: ");
  } 
  else if (state == ST_BEFORE_GAME) {
    strcpy(disp1, "O jogo comecou!");
    strcpy(disp2, "Insira cartao");
  }
  else if (state == ST_TURN) {
    mostra_stats(current_player);
  }
  else if (state == ST_MARRIAGE) {
    if (casados[current_player]) {
      strcpy(disp1, "Aniv Casamento?");
    } else {
      strcpy(disp1, "Casamento?");
    }
  }
  else if (state == ST_HOUSE) {
    current_casa = 0;
    mostra_preco_casa();
  }
  else if (state == ST_CAR) {
    current_carro = 0;
    mostra_preco_carro();
  }
  else if (state == ST_BABY) {
    current_bebes = 0;
    mostra_bebes();
  }
  else if (state == ST_ADD_SUB) {
    strcpy(disp1, "Valor add/sub");
    strcpy(disp2, "");
    tmp_input = 0;
    tmp_toggle = '$';
  } 
  else if (state == ST_SALARY) {
    tmp_input = 0;
    strcpy(disp1, "Digite salario");
    strcpy(disp2, "");
  }
  else if (state == ST_LOTTERY) {
    val_lottery = rand_interval(ano_atual*10, ano_atual*50)*1000;
    strcpy(disp1, "Prontos?!");
    sprintf(disp2, "Valor: $%lld", val_lottery);
  }
  else if (state == ST_LOTTERY_WAIT) {
    sprintf(disp1, "No. sorteado: %d", num_lottery);
    sprintf(disp2, "Valor: $%lld", val_lottery);
  }
  else if (state == ST_END) {
    winner = calculate_winner();
    sprintf(disp1, "GANHADOR %d", winner+1);
    sprintf(disp2, "LP %lld", pontos[winner]);
  }
}


void GotCard(int card) {
  // card: 0-3

  if (state == ST_BEFORE_GAME) {
    if (card == 0) {
      current_player = card;
      ano_atual++;
      GO_STATE(ST_TURN);
    }
  }

  else if (state == ST_TURN) {
    current_player = card;
    GO_STATE(ST_TURN);
  }

  else if (state == ST_LOTTERY_WAIT) {
    MUDA_S(saldos[card], saldos[card] + val_lottery);
    val_lottery = 0;
    GO_STATE(ST_TURN);
  }
}

void GotKey(char key) {
  int t;
  // key: '0'- '9', 'A'-'D', '*', '#'

  if (state == ST_INIT) {
    if (key == YEARS) {
      GO_STATE(ST_SET_YEARS);
    }
  }

  else if (state == ST_SET_YEARS) {
    if (IS_NUMERIC(key)) {
      APPEND(tmp_input, NUMERIC_VAL(key));
      sprintf(disp1, "Anos: %lld", tmp_input);
    } 
    
    if (key == ENTER || tmp_input > 100) {
      total_anos = tmp_input;
      GO_STATE(ST_BEFORE_GAME);
    }
  }

  else if (state == ST_TURN) {
    if (key == SPIN) {
      if (girou[0] && (girou[1] || girou[2] || girou[3]) && current_player == 0) {
        // muda de ano
        ano_atual++;
        girou[0] = girou[1] = girou[2] = girou[3] = 0;
      }
      if (ano_atual >= total_anos) {
        GO_STATE(ST_END);
      } else if (!girou[current_player]) {
        // pode girar
        girou[current_player] = 1;
        jogando[current_player] = 1;
        t = turno(current_player);
        mostra_stats(current_player);
        disp2[15] = t + '0';
      }
    }
    else if (key == CHANCE) {
      t = gira_chance();
      sprintf(disp1, "Sorteio: %d", t);
    }
    else if (key == MARRIAGE) {
      GO_STATE(ST_MARRIAGE);
    }
    else if (key == HOUSE) {
      GO_STATE(ST_HOUSE);
    }
    else if (key == CAR) {
      GO_STATE(ST_CAR);
    }
    else if (key == BABY) {
      GO_STATE(ST_BABY);
    }
    else if (key == TOGGLE_SLP || key == TOGGLE_MM) {
      GO_STATE(ST_ADD_SUB);
    }
    else if (key == SALARY) {
      GO_STATE(ST_SALARY);
    }
    else if (key == LOTTERY) {
      GO_STATE(ST_LOTTERY);
    }
  }

  else if (state == ST_MARRIAGE) {
    if (key == ENTER) {
      casar(current_player);
      GO_STATE(ST_TURN);
    } else if (key == UNDO) {
      GO_STATE(ST_TURN);
    }
  }

  else if (state == ST_HOUSE) {
    if (key == NEXT) {
      current_casa++;
      current_casa %= 3;
      mostra_preco_casa();
    }
    else if (key == ENTER) {
      int mult = casas[current_casa][current_player] ? 1 : -1;
      long preco = casas_precos[current_casa];
      long delta = mult * preco;
      MUDA_S(saldos[current_player], saldos[current_player] + delta);
      casas[current_casa][current_player] = !casas[current_casa][current_player];
      GO_STATE(ST_TURN);
    }
    else if (key == UNDO) {
      GO_STATE(ST_TURN);
    }
  }

  else if (state == ST_CAR) {
    if (key == NEXT) {
      current_carro++;
      current_carro %= 2;
      mostra_preco_carro();
    }
    else if (key == ENTER) {
      int mult = carros[current_carro][current_player] ? 1 : -1;
      long preco = carros_precos[current_carro];
      long delta = mult * preco;
      MUDA_S(saldos[current_player], saldos[current_player] + delta);
      carros[current_carro][current_player] = !carros[current_carro][current_player];
      GO_STATE(ST_TURN);
    }
    else if (key == UNDO) {
      GO_STATE(ST_TURN);
    }
  }

  else if (state == ST_BABY) {
    if (key == TOGGLE_MM || key == NEXT) {
      current_bebes++;
      current_bebes %= 3;
      mostra_bebes();
    }
    else if (key == ENTER) {
      bebes[current_player] += current_bebes;
      current_bebes = 0;
      GO_STATE(ST_TURN);
    }
    else if (key == UNDO) {
      GO_STATE(ST_TURN);
    }
  }

  else if (state == ST_ADD_SUB) {
    if (IS_NUMERIC(key)) {
      APPEND(tmp_input, NUMERIC_VAL(key));
      mostra_add_sub();
    }
    else if (key == TOGGLE_MM) {
      tmp_input = -tmp_input;
      mostra_add_sub();
    }
    else if (key == TOGGLE_SLP) {
      tmp_toggle = (tmp_toggle == '$' ? 'L' : '$');
      mostra_add_sub();
    }
    else if (key == ENTER) {
      mostra_stats(current_player);
      if (tmp_toggle == '$') {
        MUDA_S(saldos[current_player], saldos[current_player] + tmp_input);
      } else {
        MUDA_LP(pontos[current_player], pontos[current_player] + tmp_input);
      }
      GO_STATE(ST_TURN);
    }
    else if (key == UNDO) {
      GO_STATE(ST_TURN);
    }
  }

  else if (state == ST_SALARY) {
    if (IS_NUMERIC(key)) {
      APPEND(tmp_input, NUMERIC_VAL(key));
      sprintf(disp1, "Sal. $%lld", tmp_input);
    }
    else if (key == ENTER) {
      salarios[current_player] = tmp_input;
      GO_STATE(ST_TURN);
    }
    else if (key == UNDO) {
      GO_STATE(ST_TURN);
    }
  }

  else if (state == ST_LOTTERY) {
    if (key == LOTTERY || key == ENTER || key == SPIN) {
      num_lottery = gira_roleta(0);
      GO_STATE(ST_LOTTERY_WAIT);
    }
    else if (key == UNDO) {
      GO_STATE(ST_TURN);
    }
  }
  
  else if (state == ST_LOTTERY_WAIT) {
    if (key == LOTTERY || key == ENTER || key == SPIN) {
      val_lottery += rand_interval(ano_atual*10, ano_atual*50)*1000;
      num_lottery = gira_roleta(0);
      GO_STATE(ST_LOTTERY_WAIT);
    }
    else if (key == UNDO) {
      GO_STATE(ST_TURN);
    }
  }
}


/* MAIN
 ****************/

void main() {
  char tmp;
  char i, size;
  unsigned TagType;
  char msg[12];
  char UID[6];

  ratio_lp_money = rand_interval(80, 120);

  ADCON1 = 0X06;

  TRISB = 255;         // entrada
  TRISC = 0b11110000;  // alto entrada, baixo saida
  PORTC = 0;
  TRISD = 0;           // saida

  // liga LCD
  Lcd_Init();
  Lcd_Cmd(_LCD_CURSOR_OFF);
  Lcd_Cmd(_LCD_CLEAR);
  delay_ms(10);

  // liga teclado
  INTCON.GIE = 1;       // global interrupt enable
  INTCON.PEIE = 1;      // peripheral interrupt enable
  PORTC.F0 = PORTC.F1 = PORTC.F2 = PORTC.F3 = 1;
  INTCON.INT0IE = 1;
  INTCON.INT0IF = 0;

  // liga RFID
  Soft_SPI_Init();
  MFRC522_Init();
  
  strcpy(disp1, "Aperte YEARS (9)");

  do {
    if(!tratou_key) {
      tratou_key = 1;
      GotKey(tecla);
    }

    if (rfid_enabled) {
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
    }

    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Out(1, 1, disp1);
    Lcd_Out(2, 1, disp2);

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

    tratou_key = 0;

    // clear flag
    ENABLE_KB();
    INTCON.INT0IF = 0;
  }
}
