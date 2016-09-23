#include <SimpleTimer.h>

//*******************************************************************************
//----------------------------------Variables------------------------------------
//*******************************************************************************
int soundPin = 5;
int soundLength = 200; 

int LED[3] = {8,9,10};
int BUT[3] = {2, 3, 4};
int STARTBUT = 12;

double SOUND[3] = {220, 262, 330};


int comboCount = 0; 
int* comboLight;
int levelCount = 0; 
bool gameInProgress = false; 
int globalTime; 

struct light { 
  boolean isOn;
  int lightCol;
};

SimpleTimer gameTimer; 
int timeOut = 5000; 
int timerId;
int clockId; 
bool timerRestart = false;
long randNumber;

//*******************************************************************************
//-------------------------------FUNCTIONS---------------------------------------
//*******************************************************************************

//**************************************************
//
//Generates a random number for the the light combination 
//
//Returns the next light to light up. 
//
//**************************************************
int generateRandomLight(long randNumber){
  randNumber = random(300);
  Dprint("RandomNumber ", randNumber);
  if(randNumber <= 100){
    return(LED[0]);
  }
  else if(randNumber <=200){
    return(LED[1]);
  }
  else if(randNumber <=300){
    return(LED[2]);
  }
}

//**************************************************
//
//Return the an array with 1 more space
//
//**************************************************
int* updateLevelSize(int* newLightCombo){ 
  newLightCombo = (int*)realloc(comboLight, (levelCount+1) * sizeof(int));
  return newLightCombo;
}

//**************************************************
//
//Clock timer Function. For console and debug
//
//**************************************************
void DigitalClockDisplay() {
  int h,m, t;
  if(timerRestart){
    Serial.println();
    globalTime = millis() / 1000 -1;
    t = 1; 
    timerRestart = false;
  }
  else{
    t = millis() / 1000 - globalTime;
  }
  m = t / 60;
  h = t / 3600;
  t = t - m * 60;
  m = m - h * 60;
  Serial.print(h);
  printDigits(m);
  printDigits(t);
  Serial.println();
}

//**************************************************
//
//Clock function console print format
//
//**************************************************
void printDigits(int digits) {
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void Dprint(char *desc, int num){
  Serial.print(desc); 
  Serial.println(num);
}

//**************************************************
//
//handle the start of the game
//      - turn of first lights
//      - turn on the timers
//
//**************************************************
void gameStart(){
  if (digitalRead(STARTBUT)){
    comboLight = (int*)malloc(1 * sizeof(int));
    comboLight[levelCount] = generateRandomLight(randNumber);
    Serial.println(comboLight[levelCount]);
    delay(500);
    Serial.println("Starting the first one");
    digitalWrite(comboLight[comboCount],HIGH);
    delay(500); 
    digitalWrite(comboLight[comboCount],LOW);
    levelCount = 0; 
    gameInProgress = true; 
    timerRestart = true;
    gameTimer.enable(timerId);
    gameTimer.enable(clockId);
    gameTimer.restartTimer(timerId);
  }
}

//**************************************************
//
//Check if a button if pressed, then turn light on.
//
//**************************************************
struct light lightOn(){  
  struct light result; 
   for(int col = 0; col <= 2;col++){ //Loop through the buttons
    if (digitalRead(BUT[col]) == HIGH){ //If a button is pressed, return that a light is on, and button. 
        while(digitalRead(BUT[col]) == HIGH){
        digitalWrite(LED[col],HIGH);
        tone(soundPin, SOUND[col]);
      }
      Dprint("The button pressed is: ",col);
      digitalWrite(LED[col], LOW);
      noTone(soundPin);
      result.isOn = true; 
      result.lightCol = col; 
      return result; 
    }
    else if(col ==2){
      digitalWrite(LED[col],LOW);
      result.isOn = false; 
      result.lightCol = col; 
      return result;
    }
  }
}

//**************************************************
//
//Play the lights at the beginning of every stage
//
//**************************************************
void playLights(){
  delay(1000); 
  for(int combination = 0; combination <= levelCount; combination++){
      digitalWrite(comboLight[combination],HIGH);
      Serial.print(comboLight[combination]);
      delay(500); 
      digitalWrite(comboLight[combination],LOW);
      delay(500);
    };
}

//**************************************************
//
//Check to see if you move on to next stage or still 
//have more buttons to press
//
//Return: boolean - true if moving on to next stage
//
//**************************************************
bool checkComboLevel(){
  if(comboCount == levelCount){ //If you reach the last level of your stage, then replay the lights
    return true;
  }
  else {
    Serial.println(levelCount);
    Dprint("Level end not yet reached. comboCount=", comboCount);
    return false; 
  }
};

//**************************************************
//
//Handle the new Stage when checkComboLevel() 
//
//returns true
//
//**************************************************
void handleNewStage(){
  gameTimer.disable(timerId);
  gameTimer.disable(clockId);
  levelCount ++;
  comboCount = 0; 

  comboLight = updateLevelSize(comboLight);
  comboLight[levelCount] = generateRandomLight(randNumber);
  
  playLights();
  gameTimer.restartTimer(timerId);
  timerRestart = true;
  gameTimer.enable(timerId);
  gameTimer.enable(clockId);
}

//**************************************************
//
//Handle Timer during end Game
// 
//**************************************************
void handleEndGameTimer(){
  gameTimer.restartTimer(timerId);
  gameTimer.disable(timerId);
  gameTimer.disable(clockId);
  timerRestart = true;
}

//**************************************************
//
//Handle the end game process
//
//**************************************************
void handleEndGame(){
   for(int j = 0; j < 2; j++){
    for(int soundTone = 0; soundTone <3; soundTone++){
      tone(soundPin, SOUND[soundTone]);
      delay(100);
      noTone(soundPin);
      delay(100);
    }
  }
  comboCount = 0; 
  levelCount = 0; 
  gameInProgress = false;
  handleEndGameTimer();
  free(comboLight);
}

//**************************************************
//
//Handle what happens when a button is pressed
//
//**************************************************
void handlePressedButton(struct light pressResult){
  Serial.println(pressResult.isOn);
  int butPressed = pressResult.lightCol;
    if (LED[butPressed] == comboLight[comboCount]){
      Serial.println("MATCH");
      int newLevel = checkComboLevel();
      if (newLevel){
        handleNewStage();  
      }
      else{
        comboCount++; 
        gameTimer.restartTimer(timerId);
      }
    }
    else { 
      handleEndGame();
    }
    timerRestart = true;
}

//**************************************************
//
//Set up-Initialization
//
//**************************************************
void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600); //Console
  // put your setup code here, to run once:
  for(int i = 8; i <=10; i++){ //Lights
    pinMode(i, OUTPUT);
  };

  for(int i=2;i<=4;i++){ //Buttons
    pinMode(i,INPUT);
  }
  pinMode(STARTBUT, INPUT);
  timerId = gameTimer.setInterval(timeOut, handleEndGame);
  clockId = gameTimer.setInterval(1000, DigitalClockDisplay);
  Serial.println("Set-Up Complete");
}

void loop() {
  gameStart();
  while(gameInProgress) {
    gameTimer.run();
    struct light pressResult = lightOn();
    if(pressResult.isOn){
       handlePressedButton(pressResult);
    }
  }

}
