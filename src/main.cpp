#include <Arduino.h>
#include <SPI.h>
#include "cube.h"

#define BUTTON_PIN 8
#define RED_LED 5
#define GREEN_LED 7

#define TOTAL_EFFECTS 8

#define RAIN_TIME 1000
#define PLANE_BOING_TIME 220
#define SEND_VOXELS_TIME 140
#define WOOP_WOOP_TIME 350
#define CUBE_JUMP_TIME 700
#define GLOW_TIME 30
#define TEXT_TIME 1000

const int EFFECT_CHANGE_TIME = 5000;    //tempo che intercorre tra un effetto e il successivo in ms

uint8_t characters[10][5] = {
        {0x0E, 0x11, 0x11, 0x11, 0x0E}, //0
        {0x02, 0x06, 0x0A, 0x02, 0x02}, //1
        {0x0E, 0x02, 0x0E, 0x08, 0x0E}, //2
        {0x0E, 0x02, 0x0E, 0x02, 0x0E}, //3
        {0x0A, 0x0A, 0x0E, 0x02, 0x02}, //4
        {0x0E, 0x08, 0x0E, 0x02, 0x0E}, //5
        {0x0E, 0x08, 0x0E, 0x0A, 0x0E}, //6
        {0x0E, 0x02, 0x02, 0x02, 0x02}, //7
        {0x0E, 0x0A, 0x0E, 0x0A, 0x0E}, //8
        {0x0E, 0x0A, 0x0E, 0x02, 0x0E}, //9
};

uint8_t cube[5][5];
uint8_t currentEffect;

uint16_t timer;

uint64_t randomTimer;

bool loading = true;    //true se ho appena cambiato l'effetto

void renderCube();
void rain();
void planeBoing();
void sendVoxels();
void woopWoop();
void cubeJump();
void glow();
void text(char string[], uint8_t len);
void lit();
void setVoxel(uint8_t x, uint8_t y, uint8_t z);
bool getVoxel(uint8_t x, uint8_t y, uint8_t z);
void clearVoxel(uint8_t x, uint8_t y, uint8_t z);
void setPlane(uint8_t axis, uint8_t position);
void shift(uint8_t direction);
void drawCube(uint8_t x, uint8_t y, uint8_t z, uint8_t s);
void lightCube();
void clearCube();

unsigned long targetTimer = 0;

void light_test() {
    setVoxel(0, 0, 1);
    setVoxel(0, 0, 2);
    setVoxel(0, 0, 3);

    setVoxel(1, 0, 0);
    setVoxel(1, 0, 4);

    setVoxel(2, 0, 0);
    setVoxel(2, 0, 4);

    setVoxel(3, 0, 0);
    setVoxel(3, 0, 4);

    setVoxel(4, 0, 1);
    setVoxel(4, 0, 2);
    setVoxel(4, 0, 3);
}

void setup() {

    loading = true;
    randomTimer = 0;
    currentEffect = RAIN;

    SPI.begin();
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

    Serial.begin(9600);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);

    randomSeed(analogRead(0));
    digitalWrite(GREEN_LED, HIGH);

    targetTimer = millis() + EFFECT_CHANGE_TIME;

    light_test();
}

enum CubeAngle_t {
    TOP_FRONT_LEFT = 1,
    TOP_FRONT_RIGHT,
    TOP_REAR_LEFT,
    TOP_REAR_RIGHT,
    BOTTOM_FRONT_LEFT,
    BOTTOM_FRONT_RIGHT,
    BOTTOM_REAR_LEFT,
    BOTTOM_REAR_RIGHT,
} lit_angle;

//Variazioni su X
const int TOP = 0;
const int BOTTOM = 4;

//Variazioni su Y
const int REAR = 0;
const int FRONT = 4;

//Variazioni su Z
const int RIGHT = 0;
const int LEFT = 4;

void lightAngle(enum CubeAngle_t);

void next_effect() {
    switch(currentEffect) {
        case LIT:
            currentEffect = RAIN;
            break;
        case RAIN:
            currentEffect = CUBE_JUMP;
            break;
        case CUBE_JUMP:
            currentEffect = LIT;
            break;
        default:
            currentEffect = LIT;
    }
}

void loop() {

    if(millis() > targetTimer) {
        targetTimer += EFFECT_CHANGE_TIME;

        loading = true;
        next_effect();
    }

    switch (currentEffect) {
        case RAIN:
            rain();
            break;
        case CUBE_JUMP:
            cubeJump();
            break;
        case GLOW:
            glow();
            break;
        case LIT:
            lit();
            break;
    }

    renderCube();
}

/*
 *  Writes cube state on external shift registers.
 */
void renderCube() {
    for (uint8_t i = 0; i < 5; i++) { // i - layer
        digitalWrite(SS, LOW);

        SPI.transfer(0x01 << i); // Attiva il GND al layer i
        for (uint8_t j = 0; j < 5; j++) { // j - colonne
            SPI.transfer(cube[i][j]); // Attiva i LED per una riga di colonne (5 colonne adiacenti)
        }
        digitalWrite(SS, HIGH);
    }
}


void lightAngle(enum CubeAngle_t angle) {
    Serial.print("Current angle: ");

    switch(angle) {
        case TOP_FRONT_LEFT:
            setVoxel(TOP, FRONT, LEFT);
            Serial.println("TOP_FRONT_LEFT");
            break;
        case TOP_FRONT_RIGHT:
            setVoxel(TOP, FRONT, RIGHT);
            Serial.println("TOP_FRONT_RIGHT");
            break;
        case BOTTOM_FRONT_LEFT:
            setVoxel(BOTTOM, FRONT, LEFT);
            Serial.println("BOTTOM_FRONT_LEFT");
            break;
        case BOTTOM_FRONT_RIGHT:
            setVoxel(BOTTOM, FRONT, RIGHT);
            Serial.println("BOTTOM_FRONT_RIGHT");
            break;
        case TOP_REAR_LEFT:
            setVoxel(TOP, REAR, LEFT);
            Serial.println("TOP_REAR_LEFT");
            break;
        case TOP_REAR_RIGHT:
            setVoxel(TOP, REAR, RIGHT);
            Serial.println("TOP_REAR_RIGHT");
            break;
        case BOTTOM_REAR_LEFT:
            setVoxel(BOTTOM, REAR, LEFT);
            Serial.println("BOTTOM_REAR_LEFT");
            break;
        case BOTTOM_REAR_RIGHT:
            setVoxel(BOTTOM, REAR, RIGHT);
            Serial.println("BOTTOM_REAR_RIGHT");
            break;
    }
}


// Effects
void rain() {
    if (loading) {
        clearCube();
        loading = false;
    }
    timer++;
    if (timer > RAIN_TIME) {
        timer = 0;
        shift(POS_X);
        uint8_t numDrops = random(0, 5);
        for (uint8_t i = 0; i < numDrops; i++) {
            setVoxel(TOP, random(0, 5), random(0, 5));
        }
    }
}


uint8_t planePosition = 0;
uint8_t planeDirection = 0;
bool looped = false;

void planeBoing() {
    if (loading) {
        clearCube();
        uint8_t axis = random(0, 3);        //select random axis
        planePosition = random(0, 2) * 4;   //position can be at beginning or end
        setPlane(axis, planePosition);
        switch (axis) {
            case X_AXIS:
                planeDirection = planePosition ? NEG_X : POS_X;
                break;
            case Y_AXIS:
                planeDirection = planePosition ? NEG_Y : POS_Y;
                break;
            case Z_AXIS:
                planeDirection = planePosition ? POS_Z : NEG_Z;
                break;
        }
        timer = 0;
        looped = false;
        loading = false;
    }

    timer++;
    if (timer > PLANE_BOING_TIME) {
        timer = 0;
        shift(planeDirection);
        switch (planeDirection) {
            case POS_X:
            case POS_Y:
            case POS_Z:
                planePosition++;
                if (planePosition == 5) {
                    if (looped) {
                        planePosition = 0;
                        setPlane(planeDirection, planePosition);
                    } else {
                        loading = true;
                    }
                }
                break;
            case NEG_X:
            case NEG_Y:
            case NEG_Z:
                planePosition--;
                if (planePosition == 0) {
                    if (looped) {
                        planePosition = 4;
                        setPlane(planeDirection, planePosition);
                    } else {
                        loading = true;
                    }
                }
                break;
        }
    }
}

uint8_t selX = 0;
uint8_t selY = 0;
uint8_t selZ = 0;
uint8_t sendDirection = 0;
bool sending = false;

void sendVoxels() {
    if (loading) {
        clearCube();
        for (uint8_t y = 0; y < 5; y++) {
            for (uint8_t z = 0; z < 5; z++) {
                setVoxel(random(0, 2) * 4, y, z);
            }
        }
        loading = false;
    }

    timer++;
    if (timer > SEND_VOXELS_TIME) {
        timer = 0;

        if (!sending) {

            selY = random(0, 5);
            selZ = random(0, 5);
            if (getVoxel(0, selY, selZ)) {
                selX = 0;
                sendDirection = POS_X;
            } else if (getVoxel(7, selY, selZ)) {
                selX = 7;
                sendDirection = NEG_X;
            }
            sending = true;

        } else {

            if (sendDirection == POS_X) {
                selX++;
                setVoxel(selX, selY, selZ);
                clearVoxel(selX - 1, selY, selZ);
                if (selY == 4) {
                    sending = false;
                }
            } else {
                selX--;
                setVoxel(selX, selY, selZ);
                clearVoxel(selX + 1, selY, selZ);
                if (selY == 0) {
                    sending = false;
                }
            }

        }
    }
}


uint8_t cubeSize = 0;
bool cubeExpanding = true;

void woopWoop() {
    if (loading) {
        clearCube();
        cubeSize = 2;
        cubeExpanding = true;
        loading = false;
    }

    timer++;
    if (timer > WOOP_WOOP_TIME) {
        timer = 0;
        if (cubeExpanding) {
            cubeSize += 2;
            if (cubeSize == 5)
                cubeExpanding = false;

        } else {
            cubeSize -= 2;
            if (cubeSize == 1)
                cubeExpanding = true;

        }
        clearCube();

        if (cubeSize == 5 || cubeSize == 3) {
            drawCube(0, 0, 0, cubeSize);
        } else { //cubeSize == 1
            drawCube(3, 3, 3, cubeSize);
        }
    }
}

uint8_t xPos;
uint8_t yPos;
uint8_t zPos;

void cubeJump() {
    if (loading) {
        clearCube();
        xPos = random(0, 2) * 4;
        yPos = random(0, 2) * 4;
        zPos = random(0, 2) * 4;
        cubeSize = 5;
        cubeExpanding = false;
        loading = false;
    }

    timer++;
    if (timer > CUBE_JUMP_TIME) {
        timer = 0;
        clearCube();
        if (xPos == 0 && yPos == 0 && zPos == 0) {
            drawCube(xPos, yPos, zPos, cubeSize);
        } else if (xPos == 4 && yPos == 4 && zPos == 4) {
            drawCube(xPos + 1 - cubeSize, yPos + 1 - cubeSize, zPos + 1 - cubeSize, cubeSize);
        } else if (xPos == 4 && yPos == 0 && zPos == 0) {
            drawCube(xPos + 1 - cubeSize, yPos, zPos, cubeSize);
        } else if (xPos == 0 && yPos == 4 && zPos == 0) {
            drawCube(xPos, yPos + 1 - cubeSize, zPos, cubeSize);
        } else if (xPos == 0 && yPos == 0 && zPos == 4) {
            drawCube(xPos, yPos, zPos + 1 - cubeSize, cubeSize);
        } else if (xPos == 4 && yPos == 4 && zPos == 0) {
            drawCube(xPos + 1 - cubeSize, yPos + 1 - cubeSize, zPos, cubeSize);
        } else if (xPos == 0 && yPos == 4 && zPos == 4) {
            drawCube(xPos, yPos + 1 - cubeSize, zPos + 1 - cubeSize, cubeSize);
        } else if (xPos == 4 && yPos == 0 && zPos == 4) {
            drawCube(xPos + 1 - cubeSize, yPos, zPos + 1 - cubeSize, cubeSize);
        }
        if (cubeExpanding) {
            cubeSize++;
            if (cubeSize == 5) {
                cubeExpanding = false;
                xPos = random(0, 2) * 4;
                yPos = random(0, 2) * 4;
                zPos = random(0, 2) * 4;
            }
        } else {
            cubeSize--;
            if (cubeSize == 1) {
                cubeExpanding = true;
            }
        }
    }
}

bool glowing;
uint16_t glowCount = 0;

void glow() {
    if (loading) {
        clearCube();
        glowCount = 0;
        glowing = true;
        loading = false;
    }

    timer++;
    if (timer > GLOW_TIME) {
        timer = 0;

        if (glowing) {  //accendi pixel random se true
            if (glowCount < 50) {
                do {
                    selX = random(0, 5);
                    selY = random(0, 5);
                    selZ = random(0, 5);
                } while (getVoxel(selX, selY, selZ));

                setVoxel(selX, selY, selZ);
                glowCount++;

            } else {
                glowing = false;
                glowCount = 0;
            }

        } else {    //spegni pixel random
            if (glowCount < 50) {
                do {
                    selX = random(0, 5);
                    selY = random(0, 5);
                    selZ = random(0, 5);
                } while (!getVoxel(selX, selY, selZ));
                clearVoxel(selX, selY, selZ);
                glowCount++;
            } else {
                glowing = true;
                glowCount = 0;
            }
        }
    }
}

uint8_t charCounter = 0;
uint8_t charPosition = 0;

void text(char string[], uint8_t len) {
    if (loading) {
        clearCube();
        charPosition = -1;
        charCounter = 0;
        loading = false;
    }

    timer++;
    if (timer > TEXT_TIME) {
        timer = 0;

        shift(POS_Y);
        charPosition++;

        if (charPosition == 5) {
            charCounter++;
            if (charCounter > len - 1) {
                charCounter = 0;
            }
            charPosition = 0;
        }

        if (charPosition == 0) {
            for (uint8_t i = 0; i < 5; i++) {
                cube[i][0] = characters[string[charCounter] - '0'][i];
            }
        }
    }
}

void lit() {
    if (loading) {
        lightCube();
        loading = false;
    }
}

void setVoxel(uint8_t x, uint8_t y, uint8_t z) {
    if (z < 5)
        cube[x][y] |= (0x01 << z);
}

void clearVoxel(uint8_t x, uint8_t y, uint8_t z) {
    if (z < 5)
        cube[x][z] &= ~(0x01 << x);
}

bool getVoxel(uint8_t x, uint8_t y, uint8_t z) {
    if (z < 5)
        return (bool) cube[x][y] & (0x01 << x);
}

void setPlane(uint8_t axis, uint8_t position) {
    uint8_t x, y, z;

    switch (axis) {
        case X_AXIS:
            for (y = 0; y < 5; y++) {
                cube[position][y] = 0x1F;
            }
            break;
        case Y_AXIS:
            for (x = 0; x < 5; x++) {
                cube[x][position] = 0x1F;
            }
            break;
        case Z_AXIS:
            for (x = 0; x < 5; x++) {
                for (y = 0; y < 5; y++) {
                    cube[x][y] = 0x01 << position;
                }
            }
            break;
        default:
            break;
    }
}

void shift(uint8_t direction) {
    uint8_t x, y, z;

    switch (direction) {
        case POS_X:
            for (x = 4; x > 0; x--) {
                for (y = 0; y < 5; y++) {
                    cube[x][y] = cube[x - 1][y];
                }
            }
            for (y = 0; y < 5; y++) {
                cube[0][y] = 0x00;
            }
            break;
        case NEG_X:
            for (x = 0; x < 4; x++) {
                for (y = 0; y < 5; y++) {
                    cube[x][y] = cube[x + 1][y];
                }
            }
            for (y = 0; y < 5; y++) {
                cube[5][y] = 0x00;
            }
            break;
        case POS_Y:
            for (x = 0; x < 5; x++) {
                for (y = 4; y > 0; y--) {
                    cube[x][y] = cube[x][y - 1];
                }
                cube[x][0] = 0x00;
            }
            break;
        case NEG_Y:
            for (x = 0; x < 5; x++) {
                for (y = 0; y < 4; y++) {
                    cube[x][y] = cube[x][y + 1];
                }
                cube[x][4] = 0x00;
            }
            break;
        case POS_Z:
            for (x = 0; x < 5; x++) {
                for (y = 0; y < 5; y++) {
                    cube[x][y] >> 1;
                }
            }
            break;
        case NEG_Z:
            for (x = 0; x < 5; x++) {
                for (y = 0; y < 5; y++) {
                    cube[x][y] << 1;
                }
            }
            break;
        default:
            break;
    }
}

// TODO revision drawCube routine, what does it do?
void drawCube(uint8_t x, uint8_t y, uint8_t z, uint8_t s) {
    for (uint8_t i = 0; i < s; i++) {
        setVoxel(x, y + i, z);
        setVoxel(x + i, y, z);
        setVoxel(x, y, z + i);
        setVoxel(x + s - 1, y + i, z + s - 1);
        setVoxel(x + i, y + s - 1, z + s - 1);
        setVoxel(x + s - 1, y + s - 1, z + i);
        setVoxel(x + s - 1, y + i, z);
        setVoxel(x, y + i, z + s - 1);
        setVoxel(x + i, y + s - 1, z);
        setVoxel(x + i, y, z + s - 1);
        setVoxel(x + s - 1, y, z + i);
        setVoxel(x, y + s - 1, z + i);
    }
}

void lightCube() {
    uint8_t x, y;
    for (x = 0; x < 5; x++) {
        for (y = 0; y < 5; y++) {
            cube[x][y] = 0xFF;
        }
    }
}

void clearCube() {
    uint8_t x, y;
    for (x = 0; x < 5; x++) {
        for (y = 0; y < 5; y++) {
            cube[x][y] = 0x00;
        }
    }
}
