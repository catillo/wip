int bpm;
unsigned long ms_per_led;
unsigned long ms_per_cycle;
unsigned long diastole_delay;

unsigned char ledPins[] = {2,3,4,5,6,7,8,9};

void setup() {
  bpm = 60;

  ms_per_cycle = ((unsigned long) 60 * 1000) / bpm;
  ms_per_led = ((unsigned long) ms_per_cycle) / 24;
  diastole_delay = ((unsigned long) ms_per_cycle * 2) / 3;

  for(int i = 0; i < sizeof(ledPins); i++){
    pinMode(ledPins[i], OUTPUT);
  }

  for(int i=0; i < sizeof(ledPins); i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

void loop() {
  for(int i=0; i < sizeof(ledPins); i++) {
    digitalWrite(ledPins[i], HIGH);
    delay(ms_per_led);

    if (i < sizeof(ledPins) - 1) {
      digitalWrite(ledPins[i], LOW);
    }
  }

  delay(diastole_delay);
  digitalWrite(ledPins[sizeof(ledPins)-1], LOW);


}
