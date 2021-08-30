#define TIMEVISIBLE 4000
#define SPLASHDURATION 3000
#define VICTORYDURATION 3000
enum gameStates {DUMMY, READY, MAKEPATH, MAKEWATER, WAITING, STEPPED, SPLASH, VICTORY, BLANK};
byte gameState = READY;
bool firstStone;
bool lastStone;
byte f;
byte openFaces[6];
byte neighbourData[6];
byte previousStone = 7;
byte nextStone = 7;
byte redFace;
bool pathReady;
bool imWater;
bool stepped;
bool splashTile;
Timer showPathTimer;
Timer splashTimer;
Timer victoryTimer;
Timer delayTimer;

void sendToNeighbours() {
  FOREACH_FACE(f) {
    switch (gameState) {
      case READY:
        setValueSentOnAllFaces(READY);
        break;
      case MAKEPATH:
        byte b;
        if (pathReady == true) {
          FOREACH_FACE(b) {
            if (b == nextStone) {
              setValueSentOnFace(MAKEPATH, nextStone);
            }
            else {
              setValueSentOnFace(MAKEWATER, b);
            }
          }
        }
        break;
      case MAKEWATER:
      case WAITING:
        setValueSentOnAllFaces(WAITING);
        break;
      case STEPPED:
        setValueSentOnFace(STEPPED, nextStone);
        break;
      case SPLASH:
        setValueSentOnAllFaces(SPLASH);
        break;
      case VICTORY:
        setValueSentOnAllFaces(VICTORY);
        break;
    }
  }
}
void displayLoop() {
  switch (gameState) {
    case READY:
      setColor(dim(WHITE, 150)); // Fog
      break;
    case MAKEPATH:
      setColor(WHITE); // Stone
      break;
    case WAITING:
    case MAKEWATER:
      setColor(BLUE); // Water
      break;
    case STEPPED:
      setColor(GREEN); // Stepped stone
      break;
    case SPLASH:
      if (splashTile) {
        setColor(RED);
        setColorOnFace(dim(RED, 150), random(6));
      }
      else if (stepped) {
        setColor(GREEN);
        //setColorOnFace(YELLOW, nextStone);
      }
      else if (imWater) {
        setColor(BLUE);
      }
      else {
        setColor(WHITE);
      }
      break;
    case VICTORY:
      if (stepped) {
        setColor(GREEN);
      }
      else if (imWater) {
        setColor(BLUE);
      }
      setColorOnFace(MAGENTA, random(6));
      break;
  }
  if (showPathTimer.isExpired() && victoryTimer.isExpired() && splashTimer.isExpired() && !stepped) {
    setColor(dim(WHITE, 150)); // Fog
  }
}
void resetVariables() {
  previousStone = 7;
  nextStone = 7;
  imWater = false;
  stepped = false;
  firstStone = false;
  splashTile = false;
  lastStone = false;
  pathReady = false;
}
void setup() {
  randomize();
}
void loop() {
  byte n;
  FOREACH_FACE(n) {
    if (!isValueReceivedOnFaceExpired(n)) {
      neighbourData[n] = getLastValueReceivedOnFace(n);
    }
    else {
      neighbourData[n] = DUMMY;
    }
  }
  switch (gameState) {
    case READY:
      if (buttonPressed()) {
        gameState = MAKEPATH;
        firstStone = true;
      }
      else {
        FOREACH_FACE(f) {
          if (neighbourData[f] == MAKEWATER) {
            gameState = WAITING;
            imWater = true;
            pathReady = true;
            showPathTimer.set(TIMEVISIBLE);
          }
          if (neighbourData[f] == MAKEPATH) {
            gameState = MAKEPATH;
            previousStone = f;
            pathReady = false;
          }
        }
      }
      break;
    case WAITING:
    case MAKEPATH:
      if (pathReady) {
        if (showPathTimer.isExpired()) { // dont begin till the fog has thickened!
          if (buttonPressed()) {
            if (imWater) {
              gameState = SPLASH;
              splashTile = true;
              splashTimer.set(SPLASHDURATION);
            }
            else {
              if (firstStone || neighbourData[previousStone] == STEPPED) {
                if (lastStone) {
                  gameState = VICTORY;
                  victoryTimer.set(VICTORYDURATION);
                }
                else {
                  gameState = STEPPED;
                  stepped = true;
                }
              }
              else {
                gameState = SPLASH;
                splashTimer.set(SPLASHDURATION);
              }
            }
          }
        }
        else {
          buttonPressed(); // dump button presses
        }
      }
      else {           // Find the next stone
        byte n;
        byte a = 0;
        byte ff;
        if (firstStone) {
          FOREACH_FACE(ff) {
            if (neighbourData[ff] == READY) {
              openFaces[a++] = ff;
            }
          }
        }
        else {
          for (n = 2; n < 5; n++) {
            ff = (previousStone + n) % 6;
            if (neighbourData[ff] == READY) {
              openFaces[a++] = ff;
            }
          }
        }
        if (a == 0) {
          lastStone = true;
        }
        if (!lastStone) {
          nextStone = openFaces[random(a)];
        }
        if (neighbourData[nextStone] == READY) {
          showPathTimer.set(TIMEVISIBLE);
          pathReady = true;
        }

      }
      break;
    case SPLASH:
      if (splashTimer.isExpired()) {
        gameState = READY;
        resetVariables();
      }
      break;
    case VICTORY:
      if (victoryTimer.isExpired()) {
        gameState = READY;
        resetVariables();
      }
      break;
  }
  if (gameState != READY && gameState != VICTORY && gameState != SPLASH) {  // Listening for splash or victory
    FOREACH_FACE(f) {
      if (neighbourData[f] == SPLASH) {
        gameState = SPLASH;
        splashTimer.set(SPLASHDURATION);
      }
      if (neighbourData[f] == VICTORY) {
        gameState = VICTORY;
        victoryTimer.set(VICTORYDURATION);
      }
    }
  }
  sendToNeighbours();
  displayLoop();
}
