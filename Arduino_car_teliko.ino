#include <Servo.h>
Servo servo;

// Ορισμός Pin του HC-SR04
#define TRIG 7
#define ECHO 8

// Ορισμός Pin του L298N
#define IN1 5
#define IN2 6
#define IN3 9
#define IN4 10
#define ENA 3
#define ENB 11

// Pin  σερβοκινητήρα
#define SERVO_PIN 2

// Pin buzzer
#define BUZZER_PIN 4

enum Motor { LEFT, RIGHT };

// Set motor speed: 255 full ahead, -255 full reverse, 0 stop
void go(enum Motor m, int speed)
{
    digitalWrite(m == LEFT ? IN1 : IN3, speed > 0 ? HIGH : LOW);
    digitalWrite(m == LEFT ? IN2 : IN4, speed <= 0 ? HIGH : LOW);
    analogWrite(m == LEFT ? ENA : ENB, speed < 0 ? -speed : speed);
}

// Initial motor test:
// left motor forward then back
// right motor forward then back
void testMotors()
{
    static int speed[8] = { 128, 255, 128, 0, -128, -255, -128, 0 };
    go(RIGHT, 0);

    for (unsigned char i = 0; i < 8; i++)
        go(LEFT, speed[i]), delay(200);

    for (unsigned char i = 0; i < 8; i++)
        go(RIGHT, speed[i]), delay(200);
}

// Διαβαζει την τιμή από τον ultrasonic αισθητήρα και επιστρέφει την απόσταση σε mm
unsigned int readDistance()
{
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    unsigned long period = pulseIn(ECHO, HIGH);
    return period * 343 / 2000;
}

#define NUM_ANGLES 7
unsigned char sensorAngle[NUM_ANGLES] = { 60, 70, 80, 90, 100, 110, 120 };
unsigned int distance[NUM_ANGLES];

// Σκανάρει την περιοχή περιστρέφοντας τον αισθητήρα υπερήχων δεξιά και αριστερά.
void readNextDistance()
{
    static unsigned char angleIndex = 0;
    static signed char step = 1;

    distance[angleIndex] = readDistance();
    angleIndex += step;
    if (angleIndex == NUM_ANGLES - 1) step = -1;
    else if (angleIndex == 0) step = 1;
    servo.write(sensorAngle[angleIndex]);
}

// Initial configuration
void setup() {
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    digitalWrite(TRIG, LOW);

    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENB, OUTPUT);

    // Ρύθμιση του buzzer ως έξοδος
    pinMode(BUZZER_PIN, OUTPUT);

    servo.attach(SERVO_PIN);
    servo.write(90);

    go(LEFT, 0);
    go(RIGHT, 0);

    testMotors();

    // Scan the surroundings before starting
    servo.write(sensorAngle[0]);
    delay(200);
    for (unsigned char i = 0; i < NUM_ANGLES; i++)
        readNextDistance(), delay(200);
}

void loop() {
    readNextDistance();

    // See if something is too close at any angle
    unsigned char tooClose = 0;
    for (unsigned char i = 0; i < NUM_ANGLES; i++) {
        if (distance[i] < 300) {
            tooClose = 1;
        }

        // Έλεγχος αν η απόσταση είναι μεταξύ 30 cm και 50 cm
        if (distance[i] >= 300 && distance[i] <= 500) {
            // Ενεργοποίηση του buzzer
            tone(BUZZER_PIN, 1000); // 1000 Hz προειδοποιητικός ήχος
        } else {
            // Απενεργοποίηση του buzzer
            noTone(BUZZER_PIN);
        }
    }

    if (tooClose) {
        // Something's nearby: back up left
        go(LEFT, -180);
        go(RIGHT, -80);
    } else {
        // Nothing in our way: go forward
        go(LEFT, 255);
        go(RIGHT, 255);
    }

    // Check the next direction in 50 ms
    delay(50);
}