/*
  A simple Pong game.
  https://notabug.org/Maverick/WokwiPong
  
  Based on Arduino Pong by eholk
  https://github.com/eholk/Arduino-Pong
*/



#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h> // Hardware-specific library

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF



#define UP_BUTTON1 10
#define DOWN_BUTTON1 11
#define UP_BUTTON2 12
#define DOWN_BUTTON2 13

const unsigned long PADDLE_RATE = 64;
const unsigned long BALL_RATE = 16;
const uint8_t PADDLE_HEIGHT = 24;
const uint8_t SCORE_LIMIT = 4;

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

bool game_over, win;

uint8_t player1_score, player2_score;
uint8_t ball_x = 53, ball_y = 26;
uint8_t ball_dir_x = 1, ball_dir_y = 1;

unsigned long ball_update;
unsigned long paddle_update;

const uint8_t player2_X = 12;
uint8_t player2_y = 16;

const uint8_t PLAYER1_X = tft.width();
uint8_t player1_y = 16;

void setup(void) 
{
  Serial.begin(9600);

  uint16_t identifier = tft.readID();

  tft.begin(identifier);

  tft.setRotation(3);

   tft.fillScreen(BLACK);

    // Display the splash screen (we're legally required to do so)
    
    unsigned long start = millis();

    pinMode(UP_BUTTON1, INPUT_PULLUP);
    pinMode(DOWN_BUTTON1, INPUT_PULLUP);
    pinMode(UP_BUTTON2, INPUT_PULLUP);
    pinMode(DOWN_BUTTON2, INPUT_PULLUP);
    

    
    drawCourt();

    while(millis() - start < 2000);

    

    ball_update = millis();
    paddle_update = ball_update;
}

void loop() 
{
    bool update_needed = false;
    unsigned long time = millis();

    static bool up_state1 = false;
    static bool down_state1 = false;
    static bool up_state2 = false;
    static bool down_state2 = false;

    
    up_state1 |= (digitalRead(UP_BUTTON1) == LOW);
    down_state1 |= (digitalRead(DOWN_BUTTON1) == LOW);
    up_state2 |= (digitalRead(UP_BUTTON2) == LOW);
    down_state2 |= (digitalRead(DOWN_BUTTON2) == LOW);

    if(time > ball_update) 
    {
        uint8_t new_x = ball_x + ball_dir_x;
        uint8_t new_y = ball_y + ball_dir_y;

        // Check if we hit the vertical walls
        if(new_x == 0 || new_x == tft.width()) 
        {
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;

            if (new_x < tft.width()/2)
            {
                
                player1_score++;
            }
            else
            {
               
                player2_score++;
            }

            if (player1_score == SCORE_LIMIT || player2_score == SCORE_LIMIT)
            {
                win = player1_score > player2_score;
                game_over = true;
            }
        }

        // Check if we hit the horizontal walls.
        if(new_y == 0 || new_y == tft.height()) 
        {
            
            ball_dir_y = -ball_dir_y;
            new_y += ball_dir_y + ball_dir_y;
        }

        // Check if we hit the CPU paddle
        if(new_x == player2_X && new_y >= player2_y && new_y <= player2_y + PADDLE_HEIGHT) 
        {
            
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the player paddle
        if(new_x == PLAYER1_X && new_y >= player1_y && new_y <= player1_y + PADDLE_HEIGHT)
        {
            
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
        }

        tft.drawCircle(ball_x, ball_y,3, BLACK);
        tft.drawCircle(new_x, new_y,3, WHITE);
        ball_x = new_x;
        ball_y = new_y;

        ball_update += BALL_RATE;

        update_needed = true;
    }

    if(time > paddle_update) 
    {
        paddle_update += PADDLE_RATE;

        // CPU paddle
        tft.drawFastVLine(PLAYER1_X, player1_y, PADDLE_HEIGHT, WHITE);
        tft.drawFastVLine(player2_X, player2_y, PADDLE_HEIGHT, BLACK);

        if(up_state2) 
        {
            player2_y -= 1;
        }

        if(down_state2) 
        {
            player2_y += 1;
        }

        up_state2 = down_state2 = false;

        if(player2_y < 1) 
        {
            player2_y = 1;
        }

        if(player2_y + PADDLE_HEIGHT > tft.height()/2) 
        {
            player2_y = tft.height()/2 - PADDLE_HEIGHT;
        }

        tft.drawFastVLine(player2_X, player2_y, PADDLE_HEIGHT, BLACK);

        update_needed = true;
    


        // Player paddle
        tft.drawFastVLine(player2_X, player2_y, PADDLE_HEIGHT, WHITE);
        tft.drawFastVLine(PLAYER1_X, player1_y, PADDLE_HEIGHT, BLACK);

        if(up_state1) 
        {
            player1_y -= 1;
        }

        if(down_state1) 
        {
            player1_y += 1;
        }

        up_state1 = down_state1 = false;

        if(player1_y < 1) 
        {
            player1_y = 1;
        }

        if(player1_y + PADDLE_HEIGHT > tft.height()/2) 
        {
            player1_y = tft.height()/2 - PADDLE_HEIGHT;
        }

        tft.drawFastVLine(PLAYER1_X, player1_y, PADDLE_HEIGHT, WHITE);

        update_needed = true;
}

    if(update_needed)
    {
        if (game_over)
        {
            const char* text = win ? "PLAYER1 WINS!!" : "PLAYER2 WINS!";
            tft.fillScreen(BLACK);
            tft.setTextColor(RED);
            tft.setTextSize(5);
            tft.setCursor(40, 28);
            tft.print(text);
            

            delay(5000);

           tft.fillScreen(BLACK);
            ball_x = 53;
            ball_y = 26;
            ball_dir_x = 1;
            ball_dir_y = 1;
            player2_y = 16;
            player1_y = 16;
            player2_score = 0;
            player1_score = 0;
            game_over = false;
            drawCourt();
        }

        tft.setTextColor(WHITE, BLACK);
        tft.setCursor(5, tft.height() + 3);
         tft.setTextSize(5);
        tft.print(player2_score);
        tft.setCursor(tft.width()-6, tft.height() + 3);
         tft.setTextSize(5);
        tft.print(player1_score);
        
    }
}



void drawCourt() 
{
    tft.drawRect(0, 0, tft.width(),  tft.height()+1,WHITE);
}