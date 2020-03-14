/* Constants */
const byte ANODE_PINS[8] = {13, 12, 11, 10, 9, 8, 7, 6};
const byte CATHODE_PINS[8] = {A3, A2, A1, A0, 5, 4, 3, 2};
const byte LEFT = A5;
const byte RIGHT = 1;
const byte FIRE = A4;
const unsigned long PAUSE = 500;
const unsigned long GAME_OVER = 1000;
const byte FIRE_SPEED = 1; // (delay) the smaller, the faster
const int DEBOUNCE_DELAY = 50; // in milliseconds
const byte ALIEN_ROWS = 4;
const byte ALIEN_COLS = 7;
const unsigned long ALIEN_SPEED = 5000; // (delay) the smaller, the faster
const unsigned long ALIEN_FIRE_RATE = 3000; // (delay) the smaller, the faster
const unsigned long ALIEN_BULLET_SPEED = 500;

/* Global variables */
static byte alienCount;
static byte ledOn[8][8];
static byte lowestAlien;

/* define structs for alien... shield... spaceship... bullet ... etc */

struct Alien {
  byte alive;
  byte row;
  byte column;
} alien;

static Alien aliens[ALIEN_ROWS][ALIEN_COLS];

struct Bullet {
  byte column;
  byte row;
  bool live;
} bullet;

static Bullet alienBullets[8][8]; // stores bullets that aliens fire

struct SpaceShip {
  byte column;
  byte lives;
} spaceShip;

/* Helper functions */

void display(byte pattern[8][8]) {
  // For each anode row
  for (int i = 0; i < 8; i++) {
    // For each cathode column
    for (int j = 0; j < 8; j++) {
      if (pattern[i][j]) {
        digitalWrite(CATHODE_PINS[j], LOW);
      } else digitalWrite(CATHODE_PINS[j], HIGH);
    }
    digitalWrite(ANODE_PINS[i], LOW);
    delayMicroseconds(50);
    digitalWrite(ANODE_PINS[i], HIGH);
  }
}

void winOrLose () {
  if (spaceShip.lives == 0 || lowestAlien == 7) {
    byte loseArray[8][8] = {
      {0, 1, 0, 0, 0, 0, 0, 0},
      {0, 1, 0, 0, 0, 0, 0, 0},
      {0, 1, 0, 0, 0, 0, 0, 0},
      {0, 1, 0, 0, 0, 0, 0, 0},
      {0, 1, 0, 0, 0, 0, 0, 0},
      {0, 1, 0, 0, 0, 0, 0, 0},
      {0, 1, 1, 1, 1, 1, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0}
    };
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        ledOn[i][j] = loseArray[i][j];
      }
    }
    unsigned long endTime = millis();
    while (millis() - endTime < GAME_OVER) display(ledOn);
    setup();
  } else if (alienCount == 0) {
    byte winArray[8][8] = {
      {0, 1, 0, 0, 0, 1, 0, 0},
      {0, 1, 0, 0, 0, 1, 0, 0},
      {0, 1, 0, 0, 0, 1, 0, 0},
      {0, 1, 0, 1, 0, 1, 0, 0},
      {0, 1, 0, 1, 0, 1, 0, 0},
      {0, 1, 0, 1, 0, 1, 0, 0},
      {0, 0, 1, 0, 1, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0}
    };
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        ledOn[i][j] = winArray[i][j];
      }
    }
    unsigned long endTime = millis();
    while (millis() - endTime < GAME_OVER) display(ledOn);
    setup();
  }
}

void moveLeft() {
  byte curr_col = spaceShip.column;
  if (curr_col == 0) return;
  ledOn[7][curr_col] = 0;
  ledOn[7][curr_col - 1] = 1;
  spaceShip.column = curr_col - 1;
}

void moveRight() {
  byte curr_col = spaceShip.column;
  if (curr_col == 7) return;
  ledOn[7][curr_col] = 0;
  ledOn[7][curr_col + 1] = 1;
  spaceShip.column = curr_col + 1;
}

byte findAlienRow(byte row) {
  for (int i = 0; i < ALIEN_ROWS; i++) {
    for (int j = 0; j < ALIEN_COLS; j++) {
      if (aliens[i][j].row == row) return i;
    }
  }
  return 8; // alien was not hit
}

byte findAlienColumn(byte column, byte row) {
  for (int j = 0; j < ALIEN_COLS; j++) {
    if (aliens[row][j].column == column) return j;
  }
  return 8; // alien was not hit
}

byte findAlienBulletRow(byte col) {
  byte row = 8;
  for (int i = 7; i >= 0; i--) {
    if (alienBullets[i][col].live) return i;
  }
  return row;
}

void fireBullet() {
  byte curr_col = spaceShip.column;
  bullet.column = curr_col;
  bullet.row = 6;
  bullet.live = true;
  while (bullet.live) {
    if (bullet.row == 0 || ledOn[bullet.row][bullet.column]) {
      ledOn[bullet.row][bullet.column] = 0;

      // get row coordinate in aliens[][] for alien that is in bullet row
      byte alienArrayRow = findAlienRow(bullet.row);

      // get column coordinate in aliens[][] for alien that is in bullet column
      byte alienArrayColumn = findAlienColumn(bullet.column, alienArrayRow);

      // alien was hit
      if (alienArrayRow != 8 && alienArrayColumn != 8) {  
        aliens[alienArrayRow][alienArrayColumn].alive = false;
        alienCount--;
        
      // alien bullet or shield was hit
      } else {  
        byte alienBulletRow = findAlienBulletRow(bullet.column);
        if (alienBulletRow != 8 ) alienBullets[alienBulletRow][bullet.column].live = false; // Bullet was hit
      }
      bullet.live = false;
    } else {
      ledOn[bullet.row][bullet.column] = 1;
      display(ledOn);
      delay(FIRE_SPEED);
      ledOn[bullet.row][bullet.column] = 0;
      display(ledOn);
      bullet.row = bullet.row - 1;
    }
  }
}

void moveAlienBullets() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      byte row = i;
      byte col = j;
      Bullet alienBullet = alienBullets[row][col];

      // Move bullet down while turning its LED off
      if (alienBullet.live) {
        byte alienArrayRow = findAlienRow(alienBullet.row);
        byte alienArrayCol = findAlienColumn(alienBullet.column, alienBullet.row);
        if (!aliens[alienArrayRow][alienArrayCol].alive) ledOn[alienBullet.row][alienBullet.column] = 0;
      }
      alienBullet.row = (alienBullet.row + 1);

      // Hit player
      if (alienBullet.row == 7 && alienBullet.column == spaceShip.column) {
        ledOn[7][spaceShip.column] = 0;
        unsigned long timeHit = millis();
        while (millis() - timeHit < PAUSE) display(ledOn);
        ledOn[7][spaceShip.column] = 1;
        spaceShip.lives--;
        alienBullet.live = false;
      }

      // Hit anythig else (shield, player bullet)
      if (ledOn[alienBullet.row][alienBullet.column]) {
        ledOn[alienBullet.row][alienBullet.column] = 0;
        alienBullet.live = false;
      }
      alienBullets[row][col] = alienBullet;
    }
  }
}

void fireAlienBullet() {
  Bullet alienBullet;
  alienBullet.row = 8;    // garbage value until alien is found
  alienBullet.column = 8; // garbage value until alien is found
  alienBullet.live = true;

  // find a column to drop bullet from
  while (alienBullet.row == 8 || alienBullet.column == 8) { // once a randomly chosen column that still has an alien is found, the bullet will fire from it
    byte randColumn = random(0, ALIEN_COLS);
    // find row with living alien for which to fire bullet from
    for (int i = (ALIEN_ROWS - 1); i >= 0; i--) {
      if (aliens[i][randColumn].alive) {
        alienBullet.row = (aliens[i][randColumn].row);
        alienBullet.column = aliens[i][randColumn].column;
        break;
      }
    }
  }
  alienBullets[alienBullet.row][alienBullet.column] = alienBullet;
}

void checkButtonPush(byte button, byte *state, byte *lastReading, long *lastReadingChange) {
  byte reading = digitalRead(button);
  unsigned long now = millis();

  // Ignore button state changes (state) within DEBOUNCE_DELAY milliseconds of the last reading change, otherwise accept.
  if (now - *lastReadingChange > DEBOUNCE_DELAY) {
    if (reading == LOW && *state == HIGH) { // button pressed down (HIGH to LOW)
      if (button == LEFT) {
        moveLeft();
      } else if (button == RIGHT) {
        moveRight();
      } else {
        fireBullet();
      }
    }
    *state = reading;
  }

  // Prepare for next loop()
  if (reading != *lastReading) *lastReadingChange = now;
  *lastReading = reading;
}

void turnOnAliens() {
  for (int i = 0; i < ALIEN_ROWS; i++) {
    for (int j = 0; j < ALIEN_COLS; j++) {
      byte row = aliens[i][j].row;
      byte col = aliens[i][j].column;
      if (aliens[i][j].alive) ledOn[row][col] = 1;
    }
  }
}

void turnOnBullets() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      byte row = alienBullets[i][j].row;
      byte col = alienBullets[i][j].column;
      if (alienBullets[i][j].live) ledOn[row][col] = 1;
    }
  }
}

void moveAliensDown() {
  for (int i = 0; i < ALIEN_ROWS; i++) {
    for (int j = 0; j < ALIEN_COLS; j++) {
      byte row = aliens[i][j].row;
      byte column = aliens[i][j].column;
      aliens[i][j].row = (row + 1);
      if (aliens[i][j].alive) ledOn[row][column] = 0;
    }
  }
  lowestAlien++;
}

byte rightMostPosition(int rowStart, int colStart) {
  byte pos = 0;
  for (int i = rowStart; i < ALIEN_ROWS; i++) {
    for (int j = colStart; j < ALIEN_COLS; j++) {
      if (aliens[i][j].alive) {
        if (aliens[i][j].column > pos) pos = aliens[i][j].column;
      }
    }
  }
  return pos;
}

byte leftMostPosition(int rowStart, int colStart) {
  byte pos = 7;
  for (int i = rowStart; i < ALIEN_ROWS; i++) {
    for (int j = colStart; j < ALIEN_COLS; j++) {
      if (aliens[i][j].alive) {
        if (aliens[i][j].column < pos) pos = aliens[i][j].column;
      }
    }
  }
  return pos;
}

byte lowestAlienRow() {
  byte pos = 0;
  for (int i = 0; i < ALIEN_ROWS; i++) {
    for (int j = 0; j < ALIEN_COLS; j++) {
      if (aliens[i][j].alive) {
        if (aliens[i][j].row > pos) pos = aliens[i][j].row;
      }
    }
  }
  return pos;
}

void moveAliens(bool *movingRight, byte *rightMost, byte *leftMost) {
  for (int i = 0; i < ALIEN_ROWS; i++) {
    for (int j = 0; j < ALIEN_COLS; j++) {
      byte row = aliens[i][j].row;
      byte column = aliens[i][j].column;
      *rightMost = rightMostPosition(i, j);
      *leftMost = leftMostPosition(i, j);

      // Move all aliens one row down OR one column right
      if ((*rightMost == 7 && *movingRight) || (*leftMost == 0 && !(*movingRight))) {
        moveAliensDown();
        *movingRight = !(*movingRight);
        return;

        // Move aliens right while turning off their LEDs
      } else if (*movingRight) {
        aliens[i][j].column = (column + 1);
        if (aliens[i][j].alive) ledOn[row][column] = 0;

        // Move aliens left while turning off their LEDs
      } else {
        aliens[i][j].column = (column - 1);
        if (aliens[i][j].alive) ledOn[row][column] = 0;
      }
    }
  }
}

/* Setup code, run once */

void setup() {
  spaceShip.column = 3;
  spaceShip.lives = 3;
  alienCount = 28;
  lowestAlien = 3;
  for (byte i = 0; i < 8; i++) {
    pinMode(ANODE_PINS[i], OUTPUT);
    digitalWrite(ANODE_PINS[i], HIGH);
    pinMode(CATHODE_PINS[i], OUTPUT);
    digitalWrite(CATHODE_PINS[i], HIGH);
  }
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(FIRE, INPUT_PULLUP);

  // Initialize alien states
  for (int i = 0; i < ALIEN_ROWS; i++) {
    for (int j = 0; j < ALIEN_COLS; j++) {
      aliens[i][j].row = i;
      aliens[i][j].column = j;
      aliens[i][j].alive = 1;
    }
  }

  // Initialize board
  turnOnAliens();
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (i == 6 && (j == 1 || j == 2 || j == 5 || j == 6)) { // Shields
        ledOn[i][j] = 1;
      } else if (i == 7 && j == 3) {                          // Player
        ledOn[i][j] = 1;
      } else ledOn[i][j] = 0;
    }
  }
}

/* Loop code, run continuously */

void loop() {
  static byte lState = HIGH, rState = HIGH, fState = HIGH;
  static byte last_lReading = HIGH, last_rReading = HIGH, last_fReading = HIGH;
  static long last_lReading_change = 0, last_rReading_change = 0, last_fReading_change = 0;

  static byte rightMostPosition = 6;
  static byte leftMostPosition = 0;
  static bool movingRight = true;
  static unsigned long lastMove = 0;
  if (millis() - lastMove > ALIEN_SPEED) {
    moveAliens(&movingRight, &rightMostPosition, &leftMostPosition);
    lastMove = millis();
  }
  turnOnAliens();

  static unsigned long lastFire = 0;
  lowestAlien = lowestAlienRow();
  if (millis() - lastFire > ALIEN_FIRE_RATE) {
    fireAlienBullet();
    lastFire = millis();
  }

  static unsigned long lastBulletMove = 0;
  if (millis() - lastBulletMove > ALIEN_BULLET_SPEED) {
    moveAlienBullets();
    lastBulletMove = millis();
  }

  checkButtonPush(LEFT, &lState, &last_lReading, &last_lReading_change);
  checkButtonPush(RIGHT, &rState, &last_rReading, &last_rReading_change);
  checkButtonPush(FIRE, &fState, &last_fReading, &last_fReading_change);

  turnOnBullets();

  display(ledOn);
  winOrLose();
}
