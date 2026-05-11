// Calibra questi due valori con il tuo sensore reale
const int FLEX_DRITTO  = 300;  // valore a dito disteso
const int FLEX_PIEGATO = 220;  // valore a dito completamente piegato

const int N_CAMPIONI = 10;
int campioni[N_CAMPIONI];
int indice = 0;

void setup() {
  Serial.begin(115200);
  // Precarica il buffer con il valore attuale
  for (int i = 0; i < N_CAMPIONI; i++) {
    campioni[i] = analogRead(A0);
  }
}

void loop() {
  // Media mobile su 10 campioni
  campioni[indice] = analogRead(A0);
  indice = (indice + 1) % N_CAMPIONI;

  int somma = 0;
  for (int i = 0; i < N_CAMPIONI; i++) somma += campioni[i];
  int medio = somma / N_CAMPIONI;

  // Mappa su 0 (disteso) → 100 (piegato)
  int flessione = map(medio, FLEX_DRITTO, FLEX_PIEGATO, 0, 100);
  flessione = constrain(flessione, 0, 100);

  Serial.print("Grezzo: ");
  Serial.print(medio);
  Serial.print("  |  Flessione: ");
  Serial.print(flessione);
  Serial.println("%");

  delay(50);
}