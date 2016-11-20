#include <LedControl.h>
typedef enum {NONE=0, SNAKE=1, APPLE=2} PixelType;

typedef struct {
  PixelType type;
} Pixel;

typedef struct {
  int x;
  int y;
} Position, Direction;


//Screen
const int screenWidth = 8;
const int screenHeight = 8;
int canvas[screenWidth*screenHeight];
Pixel game[screenWidth*screenHeight];

struct {
  Position pos[64];
  unsigned int length;
  Direction dir;
  double speed;
  unsigned long lastTimeMoved;
} snake;

unsigned long time = 0;
unsigned long deltaTime; 

//Joystick
const int SW_pin = 2;
const int X_pin = 1;
const int Y_pin = 0;
const double zeroError = 0.03;
bool switchWasPressed = false;

//LED
int dataPin = 12; 
int clkPin = 11; 
int csPin = 10;
int numDevices = 1;

LedControl lc = LedControl::LedControl(dataPin,clkPin,csPin,numDevices);

//Apples
Position applePos;

void setup()
{
    calcTime();
    randomSeed(analogRead(3));

    pinMode(SW_pin, INPUT);
    digitalWrite(SW_pin, HIGH);
    pinMode(X_pin, INPUT);
    pinMode(Y_pin, INPUT);
    
     // the zero refers to the MAX7219 number, it is zero for 1 chip
    lc.shutdown(0,false);// turn off power saving, enables display
    lc.setIntensity(0,1);// sets brightness (0~15 possible values)
    lc.clearDisplay(0);// clear screen
    clearCanvas(true);

    //setLed(0, 0, true);
    // delay(2000);
    // setLed(0, screenWidth-1, true);
    // delay(2000);
    // setLed({screenHeight-1, 0}, true);
    // delay(2000);
    // setLed({screenHeight-1, screenWidth-1}, true);
    // delay(2000);

    Serial.begin(9600);
    Serial.println("started");
    
    restart();
}

void restart(){
    calcTime();

    snake.pos[0] = {0,0};
    snake.pos[1] = {0,0};
    snake.length = 2;
    snake.dir = {1, 0};
    snake.speed = 2;

    clearCanvas(true);
    
    setApple(randomPosition(snake.pos));

    writeCanvasToDisplay();
}

void loop()
{
    calcTime();
    clearCanvas(false);

    readInputs();
    if(moveSnake()){
      Serial.println("eat apple");
      setApple(randomPosition(snake.pos));
    }
    drawSnake();
    
    writeGameToCanvas();
    writeCanvasToDisplay();
}

void calcTime(){
  unsigned long mm = millis();
  deltaTime = mm - time;
  time = mm;
}

void readInputs(){
  double x = jX();
  double y = jY();
  if(abs(x) > 0.5 || abs(y) > 0.5){
    Direction newDir = {
      (abs(x) > abs(y) ? sign(x) : 0), 
      (abs(y) > abs(x) ? sign(y) : 0)
    };
    
    if(!isSnakeHere(snake.pos[0].x+newDir.x, snake.pos[0].y+newDir.y))
      snake.dir = newDir;
  }

  if(!switchWasPressed && isSwitchPressed()){
    switchWasPressed = true;
  }
  else if(switchWasPressed && isSwitchPressed()){
    switchWasPressed = false;
    restart();
  }
}

int sign(double v){
  return v < 0.0 ? -1 : v > 0.0 ? 1 : 0;
}

bool moveSnake(){
  long delta = ((time - snake.lastTimeMoved) - (1000 / snake.speed));
  if(delta >= 0 && canMove(snake.dir)){
    snake.lastTimeMoved = time;
    return moveSnake(snake.dir);
  }
  return false;
}

bool moveSnake(Direction direction){
  Position old = snake.pos[0];
  Position newPos = {snake.pos[0].x + direction.x, snake.pos[0].y + direction.y};

  bool eats = get(newPos).type == APPLE;
  if(eats) {
    snake.length++;
    snake.pos[snake.length-1] = snake.pos[snake.length-2];
    Serial.println(snake.length);
  }

  snake.pos[0] = newPos;
  for(int i=1; i<snake.length; i++){
    Position temp = snake.pos[i];
    snake.pos[i] = old;
    old = temp;
  }

  return eats;
}

void drawSnake(){
  for(int x=0; x<screenWidth; x++){
    for(int y=0; y<screenHeight; y++){
      if(isSnakeHere(x,y))
        set(x, y, {SNAKE});
      else if(get(x,y).type == SNAKE){
        set(x, y, {NONE});
      }
    }
  }
}

bool isSnakeHere(int x, int y){
  for(int i=0; i<snake.length; i++){
    if(snake.pos[i].x == x && snake.pos[i].y == y) return true;
  }
  return false;
}

bool canMove(Direction dir){
  return snake.pos[0].x + dir.x < screenWidth && snake.pos[0].x + dir.x >= 0
    &&  snake.pos[0].y + dir.y < screenHeight && snake.pos[0].y + dir.y >= 0;
}

// X-Joystick position between [-1, 1]
double jX(){
    int pinValue = analogRead(X_pin);
    return filterError((-1) * clamp((pinValue-512.0)/512, -1.0, 1.0));
}

// X-Joystick position between [-1, 1]
double jY(){
    int pinValue = analogRead(Y_pin);
    return filterError((-1) * clamp((pinValue-512.0)/512, -1.0, 1.0));
}

bool isSwitchPressed(){
  return !digitalRead(SW_pin);
}

double clamp(double value, double min, double max){
  if(value < min) return min;
  else if(value > max) return max;
  else return value;
}

double filterError(double rawValue){
  if(rawValue > -zeroError && rawValue < zeroError)
    return 0;
  else
    return rawValue; 
}

void setLed(int x, int y, bool v){
  canvas[y*screenWidth+x] = v ? 15 : 0;
}

void setLed(Position p, bool v){
  setLed(p.x, p.y, v);
}

Pixel get(int x, int y){
  return game[y*screenWidth+x];
}

Pixel get(Position position){
  return get(position.x, position.y);
}

void set(int x, int y, Pixel pixel){
  game[y*screenWidth+x] = pixel;
}

void set(Position position, Pixel pixel){
  set(position.x, position.y, pixel);
}

void setApple(Position position){
  Serial.print("apple at: (");
  Serial.print(position.x);
  Serial.print(", ");
  Serial.print(position.y);
  Serial.print(")\n");
  set(position, {APPLE});
}

//Canvas functions
void clearCanvas(bool andGameToo){
  for(int i=0; i<screenWidth*screenHeight; i++){
    if(andGameToo) game[i] = {NONE};
    canvas[i] = 0;
  }
}

void writeGameToCanvas(){
  for(int x=0; x<screenWidth; x++){
    for(int y=0; y<screenHeight; y++){
      setLed(x, y, game[y*screenWidth+x].type != NONE);
    }
  }
}

void writeCanvasToDisplay(){
  for(int x=0; x<screenWidth; x++){
    for(int y=0; y<screenHeight; y++){
      int v = canvas[y*screenWidth+x];
      //lc.setIntensity(0, v);
      lc.setLed(0, x, y, v);
    }
  }
}

Position randomPosition(bool notSnake){
  int x,y;
  do{
    x = random(screenWidth);
    y = random(screenHeight);
  }while(notSnake && isSnakeHere(x, y));
  
  return {x, y};
}

//lc.setLed(DEVICENR, ROW, COLUMN, BOOL)
//lc.setRow(DEVICENR, ROW, BYTE)
//lc.setColumn(DEVICENR, COLUMN, BYTE)