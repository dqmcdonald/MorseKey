/*
  Morse Key receiver - interprets morse code received from a key

  Quentin McDonald
  August 2016

*/


#include <SoftwareSerial.h>
#include <Bounce2.h>

// Pin definitions:
#define RX_PIN           2
#define TX_PIN           3 // LCD Screen
#define RESET_PIN        4 // Reset Switch (N.O.)
#define LED_ORANGE_PIN   6 // BiColor LED
#define LED_GREEN_PIN    5 // BiColor LED
#define BUZZER_PIN      10 // Piezo Buzzer
#define KEY_IN_PIN      14 // Morse Key (N.C.)

#define TONE_FREQUENCY 120 // The fequency of the buzzer tone
#define DOT_CUTOFF     175 // Pulses below this length are dots
#define LED_OFF    0
#define LED_ORANGE 1
#define LED_GREEN  2

#define DOT 0
#define DASH 1

#define MAX_PULSES  4  // No code has more than 4 pulses

#define  A_CODE  ((DOT<<0 ) | (DASH << 1 ) )
#define  A_LEN 2

#define  B_CODE  ((DASH<<0 ) | (DOT << 1 ) | (DOT << 2) | (DOT << 3 ) )
#define  B_LEN 4

#define  C_CODE  ((DASH<<0 ) | (DOT << 1 ) | (DASH << 2) | (DOT << 3 ) )
#define  C_LEN 4

#define D_CODE ((DASH<<0 ) | (DOT << 1 ) | (DOT << 2) )
#define D_LEN 3

#define E_CODE ((DOT<<0 ))
#define E_LEN 1

#define F_CODE ((DOT<<0 ) | (DOT << 1 ) | (DASH << 2) | (DOT << 3 ) )
#define F_LEN 4

#define G_CODE ((DASH<<0 ) | (DASH << 1 ) | (DOT << 2) )
#define G_LEN 3

#define H_CODE ((DOT<<0 ) | (DOT << 1 ) | (DOT << 2) | (DOT << 3 ) )
#define H_LEN 4

#define  I_CODE  ((DOT<<0 ) | (DOT << 1 ) )
#define  I_LEN 2

#define J_CODE ((DOT<<0 ) | (DASH << 1 ) | (DASH << 2) | (DASH << 3 ) )
#define J_LEN 4

#define K_CODE ((DASH<<0 ) | (DOT << 1 ) | (DASH << 2) )
#define K_LEN 3

#define L_CODE ((DOT<<0 ) | (DASH << 1 ) | (DOT << 2) | (DOT << 3 ) )
#define L_LEN 4

#define M_CODE ((DASH<<0 ) | (DASH << 1 ))
#define M_LEN 2

#define N_CODE ((DASH<<0 ) | (DOT << 1 ))
#define N_LEN 2

#define  O_CODE  ((DASH<<0) | (DASH << 1 ) | (DASH<<2))
#define  O_LEN 3

#define P_CODE ((DOT<<0 ) | (DASH << 1 ) | (DASH << 2) | (DOT << 3 ) )
#define P_LEN 4

#define Q_CODE ((DASH<<0 ) | (DASH << 1 ) | (DOT << 2) | (DASH << 3 ) )
#define Q_LEN 4

#define  R_CODE  ((DOT<<0) | (DASH << 1 ) | (DOT<<2))
#define  R_LEN 3

#define  S_CODE  ((DOT<<0) | (DOT << 1 ) | (DOT<<2))
#define  S_LEN 3

#define T_CODE ((DASH<<0 ))
#define T_LEN 1

#define  U_CODE  ((DOT<<0) | (DOT << 1 ) | (DASH<<2))
#define  U_LEN 3

#define  V_CODE  ((DOT<<0 ) | (DOT << 1 ) | (DOT << 2) | (DASH << 3 ) )
#define  V_LEN 4

#define  W_CODE  ((DOT<<0 ) | (DASH << 1 ) | (DASH << 2) )
#define  W_LEN 3

#define X_CODE ((DASH<<0 ) | (DOT << 1 ) | (DOT << 2) | (DASH << 3 ) )
#define X_LEN 4

#define Y_CODE ((DASH<<0 ) | (DOT << 1 ) | (DASH << 2) | (DASH << 3 ) )
#define Y_LEN 4

#define Z_CODE ((DASH<<0 ) | (DASH << 1 ) | (DOT << 2) | (DOT << 3 ) )
#define Z_LEN 4



// 20x2 Serial Controlled LCD:
class SerialLCD {
  public:
    SerialLCD();
    void displayScreen( const char* theText );
    void displayLine(int lineNum, const char *theText);
    void displayChar(int lineNum, int charNum, const char theChar);
    void clear();
    void backlight(int Percentage);

  private:
    int d_pin;

};

//*******************************************************
// Variables:
//*******************************************************
// Software serial is used for the LCD display
SoftwareSerial ss =  SoftwareSerial( RX_PIN, TX_PIN );

SerialLCD lcd;

int num_pulses = 0;
int num_chars = 0;
const int MAX_CHARS = 16;

char words[MAX_CHARS];
char dots_dashes[MAX_CHARS];

int pulse_bits = 0;

Bounce key_bouncer = Bounce();
Bounce reset_bouncer = Bounce();

const int pulse_codes[] = { A_CODE, B_CODE, C_CODE, D_CODE, E_CODE,
                      F_CODE, G_CODE, H_CODE, I_CODE, J_CODE,
                      K_CODE, L_CODE, M_CODE, N_CODE, O_CODE,
                      P_CODE, Q_CODE, R_CODE, S_CODE, T_CODE,
                      U_CODE, V_CODE, W_CODE, X_CODE, Y_CODE,
                      Z_CODE };
const int pulse_lengths[] = {  A_LEN, B_LEN, C_LEN, D_LEN, E_LEN,
                      F_LEN, G_LEN, H_LEN, I_LEN, J_LEN,
                      K_LEN, L_LEN, M_LEN, N_LEN, O_LEN,
                      P_LEN, Q_LEN, R_LEN, S_LEN, T_LEN,
                      U_LEN, V_LEN, W_LEN, X_LEN, Y_LEN,
                      Z_LEN };
                      
const int NUM_CODES = sizeof(pulse_codes) / sizeof(int);

 
//*******************************************************
// Functions:
//*******************************************************

// Clear the words and dots arrays
void clearWordsArray();
void clearDotsDashesArray();

// Display the current dots and dashes and words:
void updateLCD();

// Sets the BiColor LED state
void setLED( const int led_state );

// Adds a dot or dash to the dotdash array
void addDotDash( const char& dotdash );

// Handles the Morse Key Down situation
void handleKeyDown(long int& button_down_time, long int& pause_time);
void handleKeyUp( long int& button_down_time, long int& pause_time);

// Handles the reset of state
void handleReset();

// Tries to convert the current pulse bit pattern into a character
// Returns ' ' if there's no match
char getCharacterFromPulses();

// Adds a character to the array and updates the LCD:
void addCharacter( const char& c );

//****************************************************

void setup( ) {

  Serial.begin(9600);
  clearWordsArray();
  clearDotsDashesArray();
  pinMode(RESET_PIN, INPUT );
  digitalWrite( RESET_PIN, HIGH );
  reset_bouncer.attach( RESET_PIN );
  reset_bouncer.interval(10);


  pinMode(KEY_IN_PIN, INPUT );
  digitalWrite( KEY_IN_PIN, HIGH );
  key_bouncer.attach( KEY_IN_PIN );
  key_bouncer.interval(10);

  pinMode(BUZZER_PIN, OUTPUT );

  lcd.displayLine( 1, "Morse DeCoder");
  lcd.displayLine( 2, "v1.0     ");
  delay(2000);
  ss.flush();
  lcd.clear();
  pinMode(LED_ORANGE_PIN, OUTPUT );
  pinMode(LED_GREEN_PIN, OUTPUT );
  setLED(LED_GREEN);

}


void loop ( ) {

  static long int button_down_time = 0;
  static long int pause_time = 0;

  key_bouncer.update();
  reset_bouncer.update();

  // Morse key is normally closed so it's high (connected to 5v) when the key is down
  if ( key_bouncer.rose() ) {
    handleKeyDown(button_down_time, pause_time);
  }

  // Morse key is normally closed so it's low (connected to ground) when the key is up
  if ( key_bouncer.fell() ) {
    handleKeyUp(button_down_time, pause_time);
  }

  if ( reset_bouncer.fell() ) {
    handleReset();
  }

  if ( pause_time > 0 && ( ( millis() - pause_time) > DOT_CUTOFF * 5) ) {
    pause_time = 0;
    char c = getCharacterFromPulses();
    if ( c != ' '  ) {
      addCharacter(c);
      clearDotsDashesArray();
      num_pulses = 0;
      pulse_bits = 0;
      updateLCD();
    }
  }
}


/*******************************************************/

void handleKeyUp(long int& button_down_time ,
                 long int& pause_time )
{
  char pulse;
  long int button_elapsed_time = 0;

  setLED( LED_GREEN);
  noTone(BUZZER_PIN );
  if ( button_down_time > 0 ) {
    button_elapsed_time = millis() - button_down_time;
    Serial.print("Button down time = ");
    Serial.println(button_elapsed_time);
    if ( button_elapsed_time < DOT_CUTOFF ) {
      pulse = '.';
    }
    else {
      pulse = '-';
    }

    addDotDash(pulse);
    updateLCD();

  }
  button_down_time = 0;
  pause_time = millis();
}

/*******************************************************/

void handleKeyDown(long int& button_down_time,
                   long int& pause_time)
{
  setLED( LED_ORANGE);
  tone(BUZZER_PIN, TONE_FREQUENCY );
  button_down_time = millis();
  pause_time = 0;
}


/*******************************************************/

void handleReset()
{
  lcd.clear();
  clearWordsArray();
  clearDotsDashesArray();
  num_chars = 0;
  num_pulses = 0;
  pulse_bits = 0;
}


/*******************************************************/

// Sets the BiColor LED state
void setLED( const int led_state )
{
  switch ( led_state ) {

    case LED_OFF:
      digitalWrite( LED_ORANGE_PIN, LOW );
      digitalWrite( LED_GREEN_PIN, LOW );
      break;

    case LED_ORANGE:
      digitalWrite( LED_ORANGE_PIN, HIGH );
      digitalWrite( LED_GREEN_PIN, LOW );
      break;

    case LED_GREEN:
      digitalWrite( LED_ORANGE_PIN, LOW );
      digitalWrite( LED_GREEN_PIN, HIGH );
      break;


    default:
      break;
  }
}

/*********************************************************/

// Clear the words and dots arrays
void clearDotsDashesArray()
{
  for ( char& d : dots_dashes )
  {
    d = ' ';
  }
}

/*********************************************************/

void clearWordsArray()
{
  for ( char& w : words )
  {
    w = ' ';
  }
}

/********************************************************/

void updateLCD() {
  lcd.displayLine(1, dots_dashes);
  lcd.displayLine(2, words );
}

/* ******************************************************/

// Adds a dot or dash to the dotdash array
void addDotDash( const char& dotdash )
{

  if ( num_pulses >= MAX_PULSES) {
    num_pulses = 0;
    pulse_bits = 0;
    clearDotsDashesArray();
    return;
  }
  dots_dashes[num_pulses] = dotdash;

  if ( dotdash == '-')
    pulse_bits |= (DASH << num_pulses);

  Serial.print("Pulse bits = ");
  Serial.println(pulse_bits, BIN );

  num_pulses++;


}

/* ******************************************************/

// Adds a a character to the words array
void addCharacter( const char& c )
{

  if ( num_chars >= MAX_CHARS) {
    num_chars = 0;
    clearWordsArray();
  }
  words[num_chars] = c;

  num_chars++;
}

/* ******************************************************/
/* This function checks the current pulse pattern against the array of known codes. If
 *  the bit pattern and the number of pulses matches a known code then return the 
 *  corresponding character. Otherwise return ' ' 
 */
char getCharacterFromPulses()
{
  char ret_char = ' ';

 
  for ( auto i = 0; i < NUM_CODES; i++ ) {

    if ( pulse_bits == pulse_codes[i] && num_pulses == pulse_lengths[i] ) {
      ret_char = 'A' + i;
      break;
    }

  }

  return ret_char;
}


