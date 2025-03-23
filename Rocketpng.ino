#include <Adafruit_GFX.h>
#include <TFT_22_ILI9225.h>  
#include <Encoder.h>
#include <SPI.h>
#include "rocket-red.h"
#include "rocket-orange.h"
#include "rocket-blue.h"
#include <TimerOne.h>
#include "satellite.h" 
#include "asteroid.h" 

void(* resetArduino) (void) = 0;

// TFT display connections
#define TFT_RST  9
#define TFT_RS   8
#define TFT_CS   10
#define TFT_SDI  11
#define TFT_CLK  13

#define TFT_LENGTH 220
#define TFT_WIDTH  176


// Color Definitions
#define COLOR_WHITE  0xFFFF
#define COLOR_RED    0xF800
#define COLOR_YELLOW 0xFFE0
#define COLOR_BLACK  0x0000
#define COLOR_GRAY   0x8410


#define BAR_X  10  // Start position of the bar
#define BAR_Y  5   // Top position of the bar
#define BAR_WIDTH  200 // Total length of the bar
#define BAR_HEIGHT  15  // Thickness of the bar

// Create TFT object
TFT_22_ILI9225 tft(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK);

// Rotary encoders
Encoder player1Encoder(2, 4);
Encoder player2Encoder(3, 5);

// Player positions and previous positions
int player1X = 50, player2X = 50;
int player1PrevX = 50, player2PrevX = 50;
bool player1attack=false,player2attack=false;
int attacktime1=0,attacktime2=0;
#define attacktime 3
unsigned long freezeEndTime2 = 5000 , freezeEndTime1 = 5000;



// Button Pins
#define BTN_Player1 6
#define BTN_Player2 7

// Rocket dimensions
#define ROCKET_WIDTH  13
#define ROCKET_HEIGHT 28

// Satellite dimensions
#define SATELLITE_WIDTH  28
#define SATELLITE_HEIGHT 29

// Asteroid dimensions
#define ASTEROID_WIDTH  23
#define ASTEROID_HEIGHT 20

// Restrict movement range
#define P1_MIN_X (TFT_LENGTH / 2 +5)
#define P1_MAX_X (TFT_LENGTH - ROCKET_WIDTH-5)
#define P2_MIN_X 5
#define P2_MAX_X (TFT_LENGTH / 2 - ROCKET_WIDTH-5)

// Number of stars
#define NUM_STARS_P1 12  // Half for Player 1 (Layer 1)
#define NUM_STARS_P2 12  // Half for Player 2 (Layer 1)
#define NUM_STARS_LAYER2_P1 7  // Half for Player 1 (Layer 2)
#define NUM_STARS_LAYER2_P2 7  // Half for Player 2 (Layer 2)

// Star structure
struct Star {
    int x, y, speed;
};



// ✅ Separate arrays for each layer of stars (Left for P1, Right for P2)
Star starsP1[NUM_STARS_P1];           // Layer 1 - Player 1's Side
Star starsP2[NUM_STARS_P2];           // Layer 1 - Player 2's Side
Star starsLayer2P1[NUM_STARS_LAYER2_P1];  // Layer 2 - Player 1's Side
Star starsLayer2P2[NUM_STARS_LAYER2_P2];  // Layer 2 - Player 2's Side


long newPos1 = 0;
long newPos2 = 0;
long lastPos1 = 0, lastPos2 = 0;  // Store last stable positions
int state_menu = 0; // For deciding which menu item is highlighted

long targetDist = 100;
long timeMax = 120;
bool start=false;

volatile unsigned long gameTimer1 = timeMax;  // Game timer (incremented every second) for player 1
volatile  unsigned long gameTimer2 = timeMax;  // Game timer (incremented every second) for player 2


bool timerRunning1 = 0;
bool timerRunning2 = 0;

volatile unsigned long p1_dist = 0;  // Distance of player 1 
volatile unsigned long p2_dist = 0;  // Distance of player 2
bool dist1_active = false;  
bool dist2_active = false;

//const unsigned long delayBeforeTimer = 1000;  // 2-second delay for game start

bool gameStarted = false;


unsigned int buttonPressStartTime = 0;
bool bothPressed = false;
unsigned int timeforMenu = 2000;

bool helpActive = false;
long pos1, pos2, error1 = 0, error2 = 0;

// Freeze state & timers for each player
bool player1Frozen = false, player2Frozen = false;
unsigned long freezeStartTime1 = 0, freezeStartTime2 = 0;

#define BOOSTER_WIDTH  6   // Booster circle size
#define BOOSTER_HEIGHT 6
#define BOOSTER_COLOR  0x07E0  // Green color
#define BOOSTER_SPEED  3       // Speed of booster falling
#define BOOSTER_SPAWN_INTERVAL 15000  // Time between booster spawns (in milliseconds)

struct Booster {
    int x, y;
    int speed;
    bool active;
    unsigned long lastSpawnTime;
};

// Booster instances for both players
Booster player1Booster = {0, 10, BOOSTER_SPEED, false, 0};
Booster player2Booster = {0, 10, BOOSTER_SPEED, false, 0};



// Updated number of enemy satellites
#define NUM_ENEMY_SATELLITES 3 

// Enemy satellite structure
struct EnemySatellite {
    int x, y;
    int speed;
    bool active;
};

// Enemy satellites for both players
EnemySatellite player1Satellites[NUM_ENEMY_SATELLITES];
EnemySatellite player2Satellites[NUM_ENEMY_SATELLITES];



// Number of asteroids per player
#define NUM_ASTEROIDS 2  

// Asteroid structure
struct Asteroid {
    int x, y;
    int speedX, speedY; // Horizontal and vertical speed
    bool active;
};

// Asteroids for both players
Asteroid player1Asteroids[NUM_ASTEROIDS];
Asteroid player2Asteroids[NUM_ASTEROIDS];

bool P1Active = false;
bool P2Active = false;
//Player colors
int P1_COLOR=2,P2_COLOR=3;
int prev_P1_COLOR = 0;  // Stores the last selected rocket
int prev_P2_COLOR = 0;

//levels
byte p1level=1;
byte p2level=1;
// bool gameOver=false;


void setup() {
    Serial.begin(9600);
    tft.begin();
    tft.setOrientation(1);
    tft.clear();

    Timer1.initialize(1000000); // 1,000,000 microseconds = 1 second
    Timer1.attachInterrupt(updateGameTimer); // Attach the function

    Timer1.stop();  // Stop timer initially (won't run until game starts)
    // Initialize star positions
    // ✅ Spawn Layer 1 stars for Player 1 (left side)
    for (int i = 0; i < NUM_STARS_P1; i++) {
        starsP1[i].x = random(5, TFT_LENGTH / 2 - 5);
        starsP1[i].y = random(0, TFT_WIDTH);
        starsP1[i].speed = random(3, 6);
    }

    // ✅ Spawn Layer 1 stars for Player 2 (right side)
    for (int i = 0; i < NUM_STARS_P2; i++) {
        starsP2[i].x = random(TFT_LENGTH / 2 + 5, TFT_LENGTH - 5);
        starsP2[i].y = random(0, TFT_WIDTH);
        starsP2[i].speed = random(3, 6);
    }

    // ✅ Spawn Layer 2 stars for Player 1 (dimmer, slower stars)
    for (int i = 0; i < NUM_STARS_LAYER2_P1; i++) {
        starsLayer2P1[i].x = random(5, TFT_LENGTH / 2 - 5);
        starsLayer2P1[i].y = random(0, TFT_WIDTH);
        starsLayer2P1[i].speed = random(1, 3);
    }

    // ✅ Spawn Layer 2 stars for Player 2 (dimmer, slower stars)
    for (int i = 0; i < NUM_STARS_LAYER2_P2; i++) {
        starsLayer2P2[i].x = random(TFT_LENGTH / 2 + 5, TFT_LENGTH - 5);
        starsLayer2P2[i].y = random(0, TFT_WIDTH);
        starsLayer2P2[i].speed = random(1, 3);
    }


    pinMode(BTN_Player1, INPUT_PULLUP);
    pinMode(BTN_Player2, INPUT_PULLUP);
    
    // ✅ Initialize enemy satellites
    initializeEnemies();
    displayStartMenu();
}
void loop() {
    if (!gameStarted) {
      if( helpActive)  helpMenu();
      else if (P1Active) P1Color();
      else if (P2Active) P2Color();
      else  {

        handleMenu();
      }
    }
    else  {
        playGame();
    } 
}
   
void updateGameTimer() {
  
    if(timerRunning1 == 1){
      gameTimer1--;
    }
    if(timerRunning2 == 1){
      gameTimer2--;
    }                              //  CHECK THIS PART
    
    if(dist1_active){
      p1_dist++;
    }
    if(dist2_active){
      p2_dist++;
    }
    if(player1attack&&attacktime1<5*attacktime){
      attacktime1++;
    }
    else{
      player1attack=false;
      attacktime1=0;
    }
    if(player2attack&&attacktime2<5*attacktime){
      attacktime2++;
    }
    else{
      player2attack=false;
      attacktime2=0;
    }
}

// ✅ Fix movement without inward shifting
void get_pos() {   

    if(player1Frozen||(player2attack&&attacktime2<attacktime)){
      pos1=pos1;
      error1=player1Encoder.read()-map(pos1,TFT_LENGTH/2,TFT_LENGTH,100,-100);
    }
    else if(player1Encoder.read()-error1>90){
      pos1=TFT_LENGTH/2+5;
      error1=player1Encoder.read()-90;
    }
    else if(player1Encoder.read()-error1<-90){
      pos1=TFT_LENGTH-5;
      error1=player1Encoder.read()+90;
    }
    else{
      pos1 = map(player1Encoder.read()-error1,100,-100,TFT_LENGTH/2,TFT_LENGTH);
    }
    if(player2Frozen||(player1attack&&attacktime1<attacktime)){
      pos2=pos2;
      error2=player2Encoder.read()-map(pos2,0,TFT_LENGTH/2,100,-100);
    }
    else if(player2Encoder.read()-error2>90){
      pos2=5;
      error2=player2Encoder.read()-90;
    }
    else if(player2Encoder.read()-error2<-90){
      pos2=TFT_LENGTH/2-5;
      error2=player2Encoder.read()+90;
    }
    else{
      pos2 = map(player2Encoder.read()-error2,100,-100,0,TFT_LENGTH/2);
    }

    // Ensure strict boundary restriction (prevents inward shift)
    player1X = constrain(pos1, P1_MIN_X, P1_MAX_X);
    player2X = constrain(pos2, P2_MIN_X, P2_MAX_X);
    
}

// ✅ Erase only the rocket area and redraw stars
void clearRocket(int x, int y,int nx) {
  int left, right;
  if(x==nx) return;

    if (nx < x)
    {
        left = nx + ROCKET_WIDTH;
        right = x + ROCKET_WIDTH;
    }
    else
    {
        left = x;
        right = nx;
    }

    tft.fillRectangle(left, y,right, y + ROCKET_HEIGHT, COLOR_BLACK);
}

// ✅ Draw the rocket at the new position
void drawRocket(int x, int y,int color) {
  int buffidx=0;
  const uint16_t* selectedRocket;
  if(color == 1) selectedRocket = rocket1;
  else if(color == 2) selectedRocket = rocket2;
  else selectedRocket = rocket3;

  for (int row = 0; row < ROCKET_HEIGHT; row++) {
    for (int col = 0; col < ROCKET_WIDTH; col++) {
      tft.drawPixel(x + col, y + row, pgm_read_word(&selectedRocket[buffidx]));
      buffidx++;
    }
  }

}

// ✅ Function to update stars with two layers
void updateStars() {
    // ✅ Update Player 1's Layer 1 stars (only if not frozen)
    if (!player1Frozen) {
        for (int i = 0; i < NUM_STARS_P1; i++) {
            tft.drawPixel(starsP1[i].x, starsP1[i].y, COLOR_BLACK);  // Erase previous star
            starsP1[i].y += starsP1[i].speed;

            if (starsP1[i].y >= TFT_WIDTH) {
                starsP1[i].y = 0;
                starsP1[i].x = random(TFT_LENGTH / 2 , TFT_LENGTH );
            }

            tft.drawPixel(starsP1[i].x, starsP1[i].y, COLOR_WHITE);  // Draw new star
        }
    }

    // ✅ Update Player 2's Layer 1 stars (only if not frozen)
    if (!player2Frozen) {
        for (int i = 0; i < NUM_STARS_P2; i++) {
            tft.drawPixel(starsP2[i].x, starsP2[i].y, COLOR_BLACK);  // Erase previous star
            starsP2[i].y += starsP2[i].speed;

            if (starsP2[i].y >= TFT_WIDTH) {
                starsP2[i].y = 0;
                starsP2[i].x = random(0, TFT_LENGTH / 2);
            }

            tft.drawPixel(starsP2[i].x, starsP2[i].y, COLOR_WHITE);  // Draw new star
        }
    }

    // ✅ Update Player 1's Layer 2 stars (dimmer stars)
    if (!player1Frozen) {
        for (int i = 0; i < NUM_STARS_LAYER2_P1; i++) {
            tft.drawPixel(starsLayer2P1[i].x, starsLayer2P1[i].y, COLOR_BLACK);  // Erase previous star
            starsLayer2P1[i].y += starsLayer2P1[i].speed;

            if (starsLayer2P1[i].y >= TFT_WIDTH) {
                starsLayer2P1[i].y = 0;
                starsLayer2P1[i].x = random(TFT_LENGTH / 2 , TFT_LENGTH);
            }

            tft.drawPixel(starsLayer2P1[i].x, starsLayer2P1[i].y, COLOR_GRAY);  // Draw new dimmer star
        }
    }

    // ✅ Update Player 2's Layer 2 stars (dimmer stars)
    if (!player2Frozen) {
        for (int i = 0; i < NUM_STARS_LAYER2_P2; i++) {
            tft.drawPixel(starsLayer2P2[i].x, starsLayer2P2[i].y, COLOR_BLACK);  // Erase previous star
            starsLayer2P2[i].y += starsLayer2P2[i].speed;

            if (starsLayer2P2[i].y >= TFT_WIDTH) {
                starsLayer2P2[i].y = 0;
                starsLayer2P2[i].x = random(0, TFT_LENGTH / 2);
            }

            tft.drawPixel(starsLayer2P2[i].x, starsLayer2P2[i].y, COLOR_GRAY);  // Draw new dimmer star
        }
    }
}


// ✅ Function to draw the separation line
void drawSeparationLine() {
    tft.drawLine(TFT_LENGTH / 2, 0, TFT_LENGTH / 2, TFT_WIDTH, COLOR_WHITE);
}

long increment(long val,int min,int max){
  if (val == max) val = min;
  else val +=1;
  return val;
}

long decrement(long val,int min,int max){
  if (val == min) val = max;
  else val -=1;
  return val;
}

void displayStartMenu() {

    
      tft.setFont(Terminal11x16); 
      tft.drawText(TFT_LENGTH/3.2, 10, "MAIN MENU", COLOR_DARKGREEN);
      tft.setFont(Terminal6x8); 
      tft.drawText(TFT_LENGTH/2.3, 40, "Help", state_menu == 0 ? COLOR_YELLOW : COLOR_WHITE);
      tft.drawText(TFT_LENGTH/3.6, 65, "Start/Resume Game", state_menu == 1 ? COLOR_YELLOW : COLOR_WHITE);
      tft.drawText(TFT_LENGTH/2.8, 90, "Reset Game", state_menu == 2 ? COLOR_YELLOW : COLOR_WHITE);
      tft.drawText(TFT_LENGTH/2.6, 115, "P1 Color", state_menu == 3 ? COLOR_YELLOW : COLOR_WHITE);
      tft.drawText(TFT_LENGTH/2.6, 140, "P2 Color", state_menu == 4 ? COLOR_YELLOW : COLOR_WHITE);
      if (gameStarted || helpActive || P1Active || P2Active) {
          tft.clear();
      }
      
    
}

void handleMenu() {
  


    newPos1 = player1Encoder.read();
    newPos2 = player2Encoder.read();

    // Calculate movement difference
    long delta1 = newPos1 - lastPos1;
    long delta2 = newPos2 - lastPos2;

    // Detect exact jumps of 2 (ignore +1/-1 fluctuations)
    if (abs(delta2) >= 2) {  // If movement is at least 2
        while (abs(delta2) >= 2) {  // Process multiple steps if needed
            if (delta2 > 0) {
                state_menu = decrement(state_menu,0,4);
                lastPos2 += 2;  // Move forward by 2
            } else if ((delta2 < 0))  {
                state_menu = increment(state_menu,0,4);
                lastPos2 -= 2;  // Move backward by 2
            }
            delta2 = newPos2 - lastPos2;  // Update delta after each step
        }
    }

    // Detect exact jumps of 2 (ignore +1/-1 fluctuations)
    if (abs(delta1) >= 2) {  // If movement is at least 2
        while (abs(delta1) >= 2) {  // Process multiple steps if needed
            if (delta1 > 0) {
                state_menu = decrement(state_menu,0,4);
                lastPos1 += 2;  // Move forward by 2
            } else if ((delta1 < 0))  {
                state_menu = increment(state_menu,0,4);
                lastPos1 -= 2;  // Move backward by 2
            }
            delta1 = newPos1 - lastPos1;  // Update delta after each step
        }
    }

  
    

    
   if (state_menu == 1){   // START or RESUME
      
      if (digitalRead(BTN_Player1)== LOW && digitalRead(BTN_Player2) == LOW) {
        gameStarted = true;
        timerRunning1 = true;
        timerRunning2 = true;
        if(gameTimer1 = timeMax){
          start=true;
        }
        dist1_active = true;
        dist2_active = true;
        Timer1.start();  // ✅ Resume Timer1
      }
    }
    else if (state_menu == 2){ // RESET
      if (digitalRead(BTN_Player1) == LOW && digitalRead(BTN_Player2) == LOW) {
        gameStarted = true;
        gameTimer1 = timeMax;  // Reset timer
        gameTimer2 = timeMax;
        start=true;
        p1_dist = 0;
        p2_dist = 0;
        dist1_active = true;
        dist2_active = true;

        timerRunning1 = true;
        timerRunning2 = true;
        Timer1.start();  // Restart Timer1
      
        initializeEnemies();
      }

    }
    else if (state_menu == 3){ // Player 1 Colour
      gameStarted = false;
      if (digitalRead(BTN_Player1) == LOW ) {
            P1Active = true;
      }

    }
    else if (state_menu == 4){ // Player 2 Colour
      gameStarted = false;
      if ( digitalRead(BTN_Player2) == LOW) {
            P2Active = true;
      }

    }
    else if (state_menu == 0){ // Help
      gameStarted = false;
      if (digitalRead(BTN_Player1) == LOW || digitalRead(BTN_Player2) == LOW) {
        helpActive = true;
      }
    }
  
  displayStartMenu(); // Refresh menu display after changing selection
}

void helpMenu() {

    
    // Set font for title
    tft.setFont(Terminal6x8);
    tft.drawText(70, 10, "HELP MENU", COLOR_YELLOW);

    // Set font for text
    
    
    // Help content stored in an array for easy scrolling
    const char* helpText[] = { 
        ">Start/Reset - Both players", " press button simultaneously",
        ">Rotate to move left/right",
        ">Attack by pressing button",
        ">Attack freezes enemy,cooldown 15s",
        ">1st to Reach 100m in 120s wins",
        ">Collect Green dots to add time ",
        ">Hold button 2s for pause",
        ">Obstacle freezes all except timer"
    };

    
    for (int i = 0; i < 10; i++) {
        tft.drawText(0, 25 + (i * 15), helpText[ i], COLOR_WHITE);
    }
    
    tft.drawText(30, 25 + 135,">Press any Button to Exit ", COLOR_RED);
   
    // Exit help menu if any button is pressed
    if (digitalRead(BTN_Player1) == LOW || digitalRead(BTN_Player2) == LOW) {
        helpActive = false;
        tft.clear();
    }
}

void P1Color() {

    tft.drawRectangle(30, 50, 190, 120, COLOR_WHITE);  // Draw white border
    tft.setFont(Terminal6x8);
    tft.drawText(40, 60, "Player 1 Select Rocket", COLOR_YELLOW);

    drawRocket(60,80,1);
    drawRocket(100,80,2);
    drawRocket(140,80,3);

    newPos1 = player1Encoder.read();


    // Calculate movement difference
    long delta1 = newPos1 - lastPos1;


    // Detect exact jumps of 2 (ignore +1/-1 fluctuations)
    if (abs(delta1) >= 2) {  // If movement is at least 2
        while (abs(delta1) >= 2) {  // Process multiple steps if needed
            if (delta1 > 0) {
                P1_COLOR=decrement(P1_COLOR,1,3);
                lastPos1 += 2;  // Move forward by 2
            } else if ((delta1 < 0))  {
                P1_COLOR=increment(P1_COLOR,1,3);
                lastPos1 -= 2;  // Move backward by 2
            }
            delta1 = newPos1 - lastPos1;  // Update delta after each step
        }
    }

    if (prev_P1_COLOR != P1_COLOR){
      if (prev_P1_COLOR == 1) {
          tft.drawRectangle(60 - 3, 80 - 3, 60 + ROCKET_WIDTH + 3, 80 + ROCKET_HEIGHT + 3, COLOR_BLACK);
      } else if (prev_P1_COLOR == 2) {
          tft.drawRectangle(100 - 3, 80 - 3, 100 + ROCKET_WIDTH + 3, 80 + ROCKET_HEIGHT + 3, COLOR_BLACK);
      } else if (prev_P1_COLOR == 3) {
          tft.drawRectangle(140 - 3, 80 - 3, 140 + ROCKET_WIDTH + 3, 80 + ROCKET_HEIGHT + 3, COLOR_BLACK);
      }
    }

    if (P1_COLOR == 1) {
        // Draw a border to highlight selection
        tft.drawRectangle(60 - 3, 80 - 3, 60 + ROCKET_WIDTH +3, 80 + ROCKET_HEIGHT + 3, COLOR_LIGHTGREEN);
    }else if (P1_COLOR == 2) {
        // Draw a border to highlight selection
        tft.drawRectangle(100 - 3, 80 - 3, 100 + ROCKET_WIDTH + 3, 80 + ROCKET_HEIGHT + 3, COLOR_LIGHTGREEN);
    }else if (P1_COLOR == 3) {
        // Draw a border to highlight selection
        tft.drawRectangle(140 - 3, 80 - 3, 140 + ROCKET_WIDTH + 3, 80 + ROCKET_HEIGHT + 3, COLOR_LIGHTGREEN);
    }


    prev_P1_COLOR = P1_COLOR;

    if (digitalRead(BTN_Player1) == LOW ) {
      P1Active = false;
      tft.clear();
    }
}

void P2Color() {

    tft.drawRectangle(30, 50, 190, 120, COLOR_WHITE);  // Draw white border
    tft.setFont(Terminal6x8);
    tft.drawText(40, 60, "Player 2 Select Rocket", COLOR_YELLOW);

    drawRocket(60,80,1);
    drawRocket(100,80,2);
    drawRocket(140,80,3);


    newPos2 = player2Encoder.read();


    // Calculate movement difference
    long delta2 = newPos2 - lastPos2;


    // Detect exact jumps of 2 (ignore +1/-1 fluctuations)
    if (abs(delta2) >= 2) {  // If movement is at least 2
        while (abs(delta2) >= 2) {  // Process multiple steps if needed
            if (delta2 > 0) {
                P2_COLOR=decrement(P2_COLOR,1,3);
                lastPos2 += 2;  // Move forward by 2
            } else if ((delta2 < 0))  {
                P2_COLOR=increment(P2_COLOR,1,3);
                lastPos2 -= 2;  // Move backward by 2
            }
            delta2 = newPos2 - lastPos2;  // Update delta after each step
        }
    }

    if (prev_P2_COLOR != P2_COLOR){
      if (prev_P2_COLOR == 1) {
          tft.drawRectangle(60 - 3, 80 - 3, 60 + ROCKET_WIDTH + 3, 80 + ROCKET_HEIGHT + 3, COLOR_BLACK);
      } else if (prev_P2_COLOR == 2) {
          tft.drawRectangle(100 - 3, 80 - 3, 100 + ROCKET_WIDTH + 3, 80 + ROCKET_HEIGHT + 3, COLOR_BLACK);
      } else if (prev_P2_COLOR == 3) {
          tft.drawRectangle(140 - 3, 80 - 3, 140 + ROCKET_WIDTH + 3, 80 + ROCKET_HEIGHT + 3, COLOR_BLACK);
      }
    }

    if (P2_COLOR == 1) {
        // Draw a border to highlight selection
        tft.drawRectangle(60 - 3, 80 - 3, 60 + ROCKET_WIDTH +3, 80 + ROCKET_HEIGHT + 3, COLOR_LIGHTGREEN);
    }else if (P2_COLOR == 2) {
        // Draw a border to highlight selection
        tft.drawRectangle(100 - 3, 80 - 3, 100 + ROCKET_WIDTH + 3, 80 + ROCKET_HEIGHT + 3, COLOR_LIGHTGREEN);
    }else if (P2_COLOR == 3) {
        // Draw a border to highlight selection
        tft.drawRectangle(140 - 3, 80 - 3, 140 + ROCKET_WIDTH + 3, 80 + ROCKET_HEIGHT + 3, COLOR_LIGHTGREEN);
    }


    prev_P2_COLOR = P2_COLOR;

    if (digitalRead(BTN_Player2) == LOW) {
      P2Active = false;
      tft.clear();
    }
}



void level1_animation(){
  Timer1.stop();
  for(int i=0;i<TFT_WIDTH;i+=20){
    drawRocket(player1X, 130,P1_COLOR);
    drawRocket(player2X, 130,P2_COLOR);
    tft.drawText(TFT_LENGTH/2-70, TFT_WIDTH/2+i, "ESCAPING EARTH ATMOSPHERE", COLOR_WHITE);
    delay(500);
    tft.drawText(TFT_LENGTH/2-70, TFT_WIDTH/2+i, "ESCAPING EARTH ATMOSPHERE", COLOR_BLACK);
    tft.fillRectangle(0,TFT_WIDTH+i-10,TFT_LENGTH,TFT_WIDTH+i,COLOR_GREEN);
    tft.fillRectangle(0,TFT_WIDTH+i-50,TFT_LENGTH,TFT_WIDTH+i-40,COLOR_BLACK);
    drawSeparationLine();
  }
  tft.clear();
  Timer1.start();
}

void get_level(){
  if(p1_dist<targetDist/2){
    p1level=1;
  }
  
  else if(p1_dist<targetDist){
      for (int i = 0; i < NUM_ENEMY_SATELLITES; i++) {
        player1Satellites[i].active = false;
        // player2Satellites[i].active = false;
        tft.fillRectangle(player1Satellites[i].x, player1Satellites[i].y, player1Satellites[i].x+SATELLITE_WIDTH, player1Satellites[i].y+SATELLITE_HEIGHT, COLOR_BLACK);
        // tft.fillRectangle(player2Satellites[i].x, player2Satellites[i].y, player2Satellites[i].x+SATELLITE_WIDTH, player2Satellites[i].y+SATELLITE_HEIGHT, COLOR_BLACK);
      }
      p1level=2;
  }
  else{
    if(p1level==2){
      for (int i = 0; i < NUM_ASTEROIDS; i++) {
        player1Asteroids[i].active = false;
        // player2Asteroids[i].active = false;
        tft.fillRectangle(player1Asteroids[i].x, player1Asteroids[i].y, player1Asteroids[i].x+ASTEROID_WIDTH, player1Asteroids[i].y+ASTEROID_HEIGHT, COLOR_BLACK);
        // tft.fillRectangle(player2Asteroids[i].x, player2Asteroids[i].y, player2Asteroids[i].x+ASTEROID_WIDTH, player2Asteroids[i].y+ASTEROID_HEIGHT, COLOR_BLACK);
      }
      tft.clear();
    }
    p1level=0;
    //game_over();
  }

//change
  if(p2_dist<targetDist/2){
    p2level=1;
  }  
  else if(p2_dist<targetDist){
      for (int i = 0; i < NUM_ENEMY_SATELLITES; i++) {
        // player1Satellites[i].active = false;
        player2Satellites[i].active = false;
        // tft.fillRectangle(player1Satellites[i].x, player1Satellites[i].y, player1Satellites[i].x+SATELLITE_WIDTH, player1Satellites[i].y+SATELLITE_HEIGHT, COLOR_BLACK);
        tft.fillRectangle(player2Satellites[i].x, player2Satellites[i].y, player2Satellites[i].x+SATELLITE_WIDTH, player2Satellites[i].y+SATELLITE_HEIGHT, COLOR_BLACK);
      }
    p2level=2;
  }
  else{
    if(p2level==2){
      for (int i = 0; i < NUM_ASTEROIDS; i++) {
        // player1Asteroids[i].active = false;
        player2Asteroids[i].active = false;
        // tft.fillRectangle(player1Asteroids[i].x, player1Asteroids[i].y, player1Asteroids[i].x+ASTEROID_WIDTH, player1Asteroids[i].y+ASTEROID_HEIGHT, COLOR_BLACK);
        tft.fillRectangle(player2Asteroids[i].x, player2Asteroids[i].y, player2Asteroids[i].x+ASTEROID_WIDTH, player2Asteroids[i].y+ASTEROID_HEIGHT, COLOR_BLACK);
      }
      tft.clear();
    }
    p2level=0;
    //game_over();
  }
}

// void game_over(){
//   gameOver=true;
//   tft.drawText(65, 50, "Game Over", COLOR_RED);
//   tft.drawText(65,70,"Player Won");
// }

void playGame() {
    
    drawSeparationLine();
    get_pos();  // Allow movement only if not frozen

    if(digitalRead(BTN_Player1) == LOW && !player1attack){
      player1attack=true;
      Serial.println(player1attack);
    }
    if(digitalRead(BTN_Player2) == LOW && !player2attack){
      player2attack=true;
      Serial.println("hi");
    }
    get_level();
    if(start){
      start=false;
      level1_animation();
    }

    // Erase only the previous rocket area while keeping stars
    clearRocket(player1PrevX, 130,player1X);
    clearRocket(player2PrevX, 130,player2X);

    // Redraw stars before drawing the new rocket
    updateStars();

    // Draw new rocket position
    drawRocket(player1X, 130,P1_COLOR);
    drawRocket(player2X, 130,P2_COLOR);
    
    // Update previous position to current position
    player1PrevX = player1X;
    player2PrevX = player2X;
    
    displayScore();

    checkCollision();  // ✅ Check for collisions
    checkBoosterCollision();   // ✅ Check booster collisions

    // Update enemies & boosters
    if (!player1Frozen) updateBooster(player1Booster);
    if (!player2Frozen) updateBooster(player2Booster);

    if (!player1Frozen) spawnBooster(player1Booster, P1_MIN_X, P1_MAX_X);
    if (!player2Frozen) spawnBooster(player2Booster, P2_MIN_X, P2_MAX_X);

    // Serial.println("Booster");
    // Serial.println(player1Booster.x);
    // Serial.println(player1Booster.y);
    // Serial.println(millis());
    // Serial.println(player1Booster.lastSpawnTime);
    // Serial.print("Booster Active: ");
    // Serial.println(player1Booster.active);


    
    if (p1level==1) { // Before 30 sec: normal satellites
        if (!player1Frozen) updateEnemies(player1Satellites);
        // if (!player2Frozen) updateEnemies(player2Satellites);

        if (!player1Frozen && random(0, 100) < 5) spawnEnemy(player1Satellites, P1_MIN_X, P1_MAX_X);
        // if (!player2Frozen && random(0, 100) < 5) spawnEnemy(player2Satellites, P2_MIN_X, P2_MAX_X);
    } 
    if(p1level==2) { // After 30 sec: only asteroids
        if (!player1Frozen) updateAsteroids(player1Asteroids, P1_MIN_X, P1_MAX_X);
        // if (!player2Frozen) updateAsteroids(player2Asteroids, P2_MIN_X, P2_MAX_X);

        if (!player1Frozen && random(0, 100) < 7) spawnAsteroid(player1Asteroids, P1_MIN_X, P1_MAX_X);
        // if (!player2Frozen && random(0, 100) < 7) spawnAsteroid(player2Asteroids, P2_MIN_X, P2_MAX_X);
    }

    if (p2level==1) { // Before 30 sec: normal satellites
        // if (!player1Frozen) updateEnemies(player1Satellites);
        if (!player2Frozen) updateEnemies(player2Satellites);

        // if (!player1Frozen && random(0, 100) < 5) spawnEnemy(player1Satellites, P1_MIN_X, P1_MAX_X);
        if (!player2Frozen && random(0, 100) < 5) spawnEnemy(player2Satellites, P2_MIN_X, P2_MAX_X);
    } 
    if(p2level==2) { // After 30 sec: only asteroids
        // if (!player1Frozen) updateAsteroids(player1Asteroids, P1_MIN_X, P1_MAX_X);
        if (!player2Frozen) updateAsteroids(player2Asteroids, P2_MIN_X, P2_MAX_X);

        // if (!player1Frozen && random(0, 100) < 7) spawnAsteroid(player1Asteroids, P1_MIN_X, P1_MAX_X);
        if (!player2Frozen && random(0, 100) < 7) spawnAsteroid(player2Asteroids, P2_MIN_X, P2_MAX_X);
    }

    resumeMovement();  // ✅ Check if freeze time has ended
    
    if (digitalRead(BTN_Player1) == LOW && digitalRead(BTN_Player2) == LOW) {
        if (!bothPressed) {  // First time detection
            buttonPressStartTime = millis();  // Start timing
            bothPressed = true;
        }
        
        // Check if they have been held for at least 2000ms (2 sec)
        if (millis() - buttonPressStartTime >= timeforMenu) {
            tft.clear();
            gameStarted = false;
            state_menu = 0;
            timerRunning1 = false;
            timerRunning2 = false;
            dist1_active = false;
            dist2_active = false;
            Timer1.stop();
            displayStartMenu();
            bothPressed = false;  // Reset flag after menu opens
        }
    } else {
        bothPressed = false;  // Reset if buttons are released
    }
    // delay(50);
    if (p1_dist >= targetDist || p2_dist >= targetDist || gameTimer1 == 0 || gameTimer2 == 0) {
      checkWinner();
    }

}

void checkWinner() {
    // Prevent timers from going negative
    gameTimer1 = max(0, gameTimer1);
    gameTimer2 = max(0, gameTimer2);
    bool p1Finished = (p1_dist >= targetDist);
    bool p2Finished = (p2_dist >= targetDist);

    // Ensure we only check while at least one timer is running
    if (gameTimer1 > 0 || gameTimer2 > 0) {
        

        // Case 1: Both players finish before their timers reach 0
        if (p1Finished && p2Finished) {
            if (gameTimer1 > gameTimer2) {
                displayWinner(1);
            } else if (gameTimer2 > gameTimer1) {
                displayWinner(2);
            } else {
                displayBothWin();  // Both win separately handled
            }
            return;
        }

        // Case 2: One player finishes first
        if (p1Finished && !p2Finished) {
            displayWinner(1);
            return;
        }
        if (p2Finished && !p1Finished) {
            displayWinner(2);
            return;
        }
    }

    // If we reach here, at least one timer is zero
    bool p1TimeUp = (gameTimer1 == 0);
    bool p2TimeUp = (gameTimer2 == 0);

    // Case 3: One player's time runs out but they haven't reached target
    if (p1TimeUp && !p1Finished && !p2TimeUp) {
        stopPlayer(1);  // Player 1 stops moving, Player 2 continues
        return;
    }
    if (p2TimeUp && !p2Finished && !p1TimeUp) {
        stopPlayer(2);  // Player 2 stops moving, Player 1 continues
        return;
    }

    // Case 4: Both players’ time runs out
    if (p1TimeUp && p2TimeUp) {
        if (p1Finished) {
            displayWinner(1);
        } else if (p2Finished) {
            displayWinner(2);
        } else {
            displayBothLose();  // Both lose separately handled
        }
    }
}
void displayWinner(int player) {
    tft.clear();  // Clear the screen
    tft.setFont(Terminal11x16);  // Use a compatible font
    tft.drawText(30, 50, "Player ", COLOR_WHITE);
    char scoreText[4];  // To store score as text

    
    sprintf(scoreText, "%d", player);


    tft.drawText(110, 50,scoreText,  COLOR_WHITE);
    tft.drawText(140, 50, " Wins!", COLOR_WHITE);
    
    delay(6000);  // Show result for 3 seconds
    resetArduino();
    
}

void displayBothWin() {
    tft.clear();
    tft.setFont(Terminal11x16);
    tft.drawText(50, 50, "Both Players Win!", COLOR_WHITE);
    delay(6000);
    resetArduino();
}

void displayBothLose() {
    tft.clear();
    tft.setFont(Terminal11x16);
    tft.drawText(50, 50, "Both Lose!", COLOR_RED);
    delay(6000);
    resetArduino();
}


void stopPlayer(int player) {
    if (player == 1) {
        timerRunning1 = false;  // Disable movement for Player 1
        dist1_active = false;
        freezeEndTime1 = 200000;
        

    } else if (player == 2) {
        timerRunning2 = false; // Disable movement for Player 2
        dist2_active = false;
        freezeEndTime2 = 200000;
    }
}



 void displayScore(){
  

    char scoreText[4];  // To store score as text

    tft.drawText(10, 10, "Time P2:", COLOR_WHITE);
    sprintf(scoreText, "%d", gameTimer2);
    tft.drawText(70, 10, scoreText, COLOR_WHITE);

    tft.drawText(120, 10, "Time P1:", COLOR_WHITE);
    sprintf(scoreText, "%d", gameTimer1);
    tft.drawText(180, 10, scoreText, COLOR_WHITE);
   
    tft.drawText(10, 30, "Distance P2:", COLOR_WHITE);
    sprintf(scoreText, "%d", p2_dist);
    tft.drawText(90, 30, scoreText, COLOR_WHITE);

    tft.drawText(120, 30, "Distance P1:", COLOR_WHITE);
    sprintf(scoreText, "%d", p1_dist);
    tft.drawText(200, 30, scoreText, COLOR_WHITE);
    
 }


 
// ✅ Initialize enemy satellites
void initializeEnemies() {
    for (int i = 0; i < NUM_ENEMY_SATELLITES; i++) {
        player1Satellites[i].active = false;
        player2Satellites[i].active = false;
    }
}


// ✅ Spawn enemy satellites ensuring enough escape space
void spawnEnemy(EnemySatellite satellites[], int minX, int maxX) {
    for (int i = 0; i < NUM_ENEMY_SATELLITES; i++) {
        if (!satellites[i].active) {  // Find an inactive enemy slot
            bool validPosition = false;
            int newX;
            int attempts = 0;  // ✅ SAFEGUARD against infinite loop

            // Ensure new enemy doesn't overlap with existing ones and leaves escape space
            while (!validPosition && attempts < 10) {  // ✅ Prevent infinite loop
                newX = random(minX, maxX - SATELLITE_WIDTH/2);
                validPosition = true;
                attempts++;

                for (int j = 0; j < NUM_ENEMY_SATELLITES; j++) {
                    if (satellites[j].active) {
                        int distance = abs(satellites[j].x - newX);

                        // Prevent overlap and leave escape gap (at least ROCKET_WIDTH * 2)
                        if (distance < SATELLITE_WIDTH + ROCKET_WIDTH * 1.5) {
                            validPosition = false;
                            break;
                        }
                    }
                }
            }

            if (validPosition) {  // ✅ Only spawn if a valid position was found
                satellites[i].x = newX;
                satellites[i].y = 0;
                satellites[i].speed = random(2, 5);
                satellites[i].active = true;
            }

            break;  // ✅ Ensures only one satellite is spawned per function call
        }
    }
}

// ✅ Move and update enemy satellites
// ✅ Move and update enemy satellites
void updateEnemies(EnemySatellite satellites[])
{
    for (int i = 0; i < NUM_ENEMY_SATELLITES; i++)
    {
        if (satellites[i].active)
        {
            int y = satellites[i].y;
            int ny = satellites[i].y + satellites[i].speed;
            int top, bottom;
            top = y;
            bottom = ny;
            if (ny > TFT_WIDTH)
            {
                bottom = satellites[i].y;
            }
            // Erase previous position
            tft.fillRectangle(satellites[i].x, y,
                              satellites[i].x + SATELLITE_WIDTH,
                              ny, COLOR_BLACK);

            // ✅ Increase speed after 40 sec
            if (gameTimer1 <= 3*timeMax/4)
            {
                satellites[i].speed += 1; // Increase speed dynamically
                if (satellites[i].speed > 8)
                    satellites[i].speed = 8; // Cap max speed
            }

            // Move satellite down
            satellites[i].y += satellites[i].speed;

            // If out of bounds, deactivate
            if (satellites[i].y >= TFT_WIDTH) {
                satellites[i].active = false;
            } 
            else {
                // Draw enemy satellite
                drawSatellite(satellites[i].x, satellites[i].y);
            }
        }
    }
}
// ✅ Function to draw satellite using satellite.h array
void drawSatellite(int x, int y) {
    int buffidx = 0;
    for (int row = 0; row < SATELLITE_HEIGHT; row++) {
        for (int col = 0; col < SATELLITE_WIDTH; col++) {
            uint16_t pixelColor = pgm_read_word(&satellite[buffidx]);
            tft.drawPixel(x + col, y + row, pixelColor);
            buffidx++;
        }
    }
}



void checkCollision() {
    // Check for Player 1 collision
    if (!player1Frozen) {
      if(p1level==1){
        for (int i = 0; i < NUM_ENEMY_SATELLITES; i++) {
            if (player1Satellites[i].active) {
              if (player1X < player1Satellites[i].x + SATELLITE_WIDTH &&
                  player1X + ROCKET_WIDTH > player1Satellites[i].x &&
                  130 < player1Satellites[i].y + SATELLITE_HEIGHT &&
                  130 + ROCKET_HEIGHT > player1Satellites[i].y) {
                  player1Frozen = true; // Freeze player 1
                  freezeStartTime1 = millis(); // Record freeze start time
                  player1Satellites[i].active = false; // Remove satellite
                  tft.fillRectangle(player1Satellites[i].x, player1Satellites[i].y, player1Satellites[i].x+SATELLITE_WIDTH, player1Satellites[i].y+SATELLITE_HEIGHT, COLOR_BLACK);


                  dist1_active = false;
              }
            }
        }
      }
      else if(p1level==2){
        for (int i = 0; i < NUM_ASTEROIDS; i++) {
          if (player1Asteroids[i].active) {
            if (player1X < player1Asteroids[i].x + ASTEROID_WIDTH &&
                    player1X + ROCKET_WIDTH > player1Asteroids[i].x &&
                    130 < player1Asteroids[i].y + ASTEROID_HEIGHT &&
                    130 + ROCKET_HEIGHT > player1Asteroids[i].y) {

                    player1Frozen = true;  // ❄ Freeze Player 1
                    freezeStartTime1 = millis();  // Start 5 sec timer
                    player1Asteroids[i].active = false;  // Remove asteroid immediately
                    tft.fillRectangle(player1Asteroids[i].x, player1Asteroids[i].y, player1Asteroids[i].x+ASTEROID_WIDTH, player1Asteroids[i].y+ASTEROID_HEIGHT, COLOR_BLACK);

                    dist1_active = false;
            }
          }
        }
      }
    }
    
                  

    // Check for Player 2 collision
    if (!player2Frozen) {
      if(p2level==1){
        for (int i = 0; i < NUM_ENEMY_SATELLITES; i++) {
          if (player2Satellites[i].active) {
                if (player2X < player2Satellites[i].x + SATELLITE_WIDTH &&
                    player2X + ROCKET_WIDTH > player2Satellites[i].x &&
                    130 < player2Satellites[i].y + SATELLITE_HEIGHT &&
                    130 + ROCKET_HEIGHT > player2Satellites[i].y) {

                    player2Frozen = true; // Freeze player 2
                    freezeStartTime2 = millis(); // Record freeze start time
                    player2Satellites[i].active = false; // Remove satellite
                    tft.fillRectangle(player2Satellites[i].x, player2Satellites[i].y, player2Satellites[i].x+SATELLITE_WIDTH, player2Satellites[i].y+SATELLITE_HEIGHT, COLOR_BLACK);

                    dist2_active = false;
                }
            }
        }
      }
      else if(p2level==2){
        for (int i = 0; i < NUM_ASTEROIDS; i++) {
          if (player2Asteroids[i].active) {
            if (player2X < player2Asteroids[i].x + ASTEROID_WIDTH &&
              player2X + ROCKET_WIDTH > player2Asteroids[i].x &&
              130 < player2Asteroids[i].y + ASTEROID_HEIGHT &&
              130 + ROCKET_HEIGHT > player2Asteroids[i].y) {
              player2Frozen = true;  // ❄ Freeze Player 2
              freezeStartTime2 = millis();  // Start 5 sec timer
              player2Asteroids[i].active = false;  // Remove asteroid immediately
              tft.fillRectangle(player2Asteroids[i].x, player2Asteroids[i].y, player2Asteroids[i].x+ASTEROID_WIDTH, player2Asteroids[i].y+ASTEROID_HEIGHT, COLOR_BLACK);


              dist2_active = false;
            }
          }
        }
      }
    }
}

void resumeMovement() {
    // Resume Player 1 movement after 5 sec
    if (player1Frozen && millis() - freezeStartTime1 >= 5000) {
        player1Frozen = false;
        dist1_active = true;

    }

    // Resume Player 2 movement after 5 sec
    if (player2Frozen && millis() - freezeStartTime2 >= 5000) {
        player2Frozen = false;
        dist2_active = true;
        
    }
}


void initializeAsteroids() {
    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        player1Asteroids[i].active = false;
        player2Asteroids[i].active = false;
    }
}

// ✅ Spawn asteroids instead of satellites after 30 sec
void spawnAsteroid(Asteroid asteroids[], int minX, int maxX) {
    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        if (!asteroids[i].active) {
            bool validPosition = false;
            int newX;
            int attempts = 0;

            while (!validPosition && attempts < 10) {  
                newX = random(minX + 20, maxX - 20);
                validPosition = true;
                attempts++;

                for (int j = 0; j < NUM_ASTEROIDS; j++) {
                    if (asteroids[j].active) {
                        int distance = abs(asteroids[j].x - newX);
                        if (distance < ROCKET_WIDTH * 2) {
                            validPosition = false;
                            break;
                        }
                    }
                }
            }

            if (validPosition) {
                asteroids[i].x = newX;
                asteroids[i].y = 0;
                asteroids[i].speedX = (random(0, 2) == 0) ? -2 : 2; // Move left or right
                asteroids[i].speedY = random(3, 6); // Move down at random speed
                asteroids[i].active = true;
            }
            break;
        }
    }
}

// ✅ Update asteroid movement (both horizontal & vertical)
void updateAsteroids(Asteroid asteroids[], int minX, int maxX) {
    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        if (asteroids[i].active) {
            int oldX = asteroids[i].x;
            int oldY = asteroids[i].y;

            asteroids[i].x += asteroids[i].speedX;
            asteroids[i].y += asteroids[i].speedY;

            // Bounce off left and right walls
            if (asteroids[i].x <= minX || asteroids[i].x >= maxX - 12) {
                asteroids[i].speedX *= -1; // Reverse direction
                asteroids[i].x += asteroids[i].speedX; // Prevent sticking
            }

            // If asteroid reaches the bottom, deactivate it
            if (asteroids[i].y >= TFT_WIDTH) {
                asteroids[i].active = false;
            } else {
                // Clear old asteroid position
                tft.fillRectangle(oldX, oldY, oldX + ASTEROID_WIDTH, oldY + ASTEROID_HEIGHT, COLOR_BLACK);
                
                // Draw new asteroid position
                drawAsteroid(asteroids[i].x, asteroids[i].y);
            }
        }
    }
}

void drawAsteroid(int x, int y) {
    int buffidx = 0;
    for (int row = 0; row < ASTEROID_HEIGHT; row++) {  
        for (int col = 0; col < ASTEROID_WIDTH; col++) {
            uint16_t pixelColor = pgm_read_word(&asteroid[buffidx]);
            tft.drawPixel(x + col, y + row, pixelColor);
            buffidx++;
        }
    }
}


void spawnBooster(Booster &booster, int minX, int maxX) {
    if (!booster.active && millis() - booster.lastSpawnTime > BOOSTER_SPAWN_INTERVAL) {  // Find an inactive booster slot
        bool validPosition = false;
        int newX;
        int attempts = 0;

        // Generate a position within bounds
        while (!validPosition && attempts < 10) {  
            newX = random(minX, maxX - BOOSTER_WIDTH / 2);
            validPosition = true;  // Assume position is valid
            attempts++;

            /*
            // (🔹 Overlap check is disabled, uncomment if needed)
            for (int j = 0; j < NUM_ENEMY_SATELLITES; j++) {
                if (player1Satellites[j].active) {
                    int distance = abs(player1Satellites[j].x - newX);
                    if (distance < SATELLITE_WIDTH + BOOSTER_WIDTH * 1.5) {
                        validPosition = false;
                        break;
                    }
                }
            }
            */
        }

        if (validPosition) {  // ✅ Only spawn if a valid position was found
            booster.x = newX;
            booster.y = 20;
            booster.speed = BOOSTER_SPEED;
            booster.active = true;
            booster.lastSpawnTime = millis();
        }
    }
}
void updateBooster(Booster &booster) {
    if (booster.active) {
        int oldY = booster.y;
        booster.y += booster.speed;

        // Erase old position
        tft.fillCircle(booster.x, oldY, BOOSTER_WIDTH / 2, COLOR_BLACK);

        // Check if booster is off-screen
        if (booster.y >= TFT_WIDTH) {
            booster.active = false; // Remove if it reaches the bottom
        } else {
            // Draw new position
            tft.fillCircle(booster.x, booster.y, BOOSTER_WIDTH / 2, BOOSTER_COLOR);
        }
    }
}


void checkBoosterCollision() {
    // Player 1 Booster Collision
    if (player1Booster.active) {
        if (player1X < player1Booster.x + BOOSTER_WIDTH &&
            player1X + ROCKET_WIDTH > player1Booster.x &&
            130 < player1Booster.y + BOOSTER_HEIGHT &&
            130 + ROCKET_HEIGHT > player1Booster.y) {

            gameTimer1 += 5; // ✅ Increase score by 10
            player1Booster.active = false;  // Remove booster
            tft.fillCircle(player1Booster.x, player1Booster.y, BOOSTER_WIDTH / 2, COLOR_BLACK);
        }
    }

    // Player 2 Booster Collision
    if (player2Booster.active) {
        if (player2X < player2Booster.x + BOOSTER_WIDTH &&
            player2X + ROCKET_WIDTH > player2Booster.x &&
            130 < player2Booster.y + BOOSTER_HEIGHT &&
            130 + ROCKET_HEIGHT > player2Booster.y) {

            gameTimer2 += 5;  // ✅ Increase score by 10
            player2Booster.active = false;  // Remove booster
            tft.fillCircle(player2Booster.x, player2Booster.y, BOOSTER_WIDTH / 2, COLOR_BLACK);
        }
    }
}