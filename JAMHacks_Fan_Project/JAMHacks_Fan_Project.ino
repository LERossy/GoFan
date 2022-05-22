#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // put your setup code here, to run once:
  pinMode(3, OUTPUT);
  pinMode(8, INPUT_PULLUP);
  pinMode(A0, INPUT);
  Serial.begin(9600);
  dht.begin();
}

bool kill = false;
int button = 0;
int prev_button = 0;
int motor_speed;
int base_pwm = 100;
int prev_base_pwm = 100;

int photo = 750;
int previous_photo;
int new_photo;
const int array_size = 4;
int prev_photo[array_size] = {750, 750, 750, 750};
int state = 1;
int increase = 0;

float humidity = 60.0;
float temp_humidity;
float temperature = 23.0;
float temp_temperature;
float heat_index;   //human-comfort value based on temp and humidity

int calculateBasePWM(int heat_index) {
  int pwm;

  pwm = (heat_index - 20) * (heat_index - 20) + 125;

  return pwm;

}

void pushBack(int new_val) {
  
  for (int i = array_size-2; i >= 0; i--) {
    prev_photo[i+1] = prev_photo[i] ;
  }

  prev_photo[0] = new_val;
}

int avg(int values[], int array_size) {

  int sum;
  int avg;

  for (int i = 0; i < array_size; i++) {
    sum += values[i];
  }

  avg = sum / array_size;

  return avg;
}




void loop() {
  // put your main code here, to run repeatedly:
  button = digitalRead(8);
  
  if (button == 0 && prev_button == 1) {
    kill = !kill;
  }

  prev_button = button;
  

  temp_humidity = dht.readHumidity();          //will need filter

  if (abs(temp_humidity - humidity) > 2)
    humidity = temp_humidity;
    

  temp_temperature = dht.readTemperature();    //will need filter

  if (abs(temp_temperature - temperature) > 2)
    temperature = temp_temperature;

  

  if (!(isnan(humidity) || isnan(temperature))) {
    heat_index = dht.computeHeatIndex(temperature, humidity, false);
  }

  
  base_pwm = calculateBasePWM(heat_index);
   

  new_photo = analogRead(A0);   //may need to make a filter; using 5k resistor; vals vary between 0-1023

  if (abs(new_photo - photo) < 25) {
    pushBack(new_photo);
    photo = avg(prev_photo, array_size);
  }

 
  if (base_pwm == prev_base_pwm) {
    if (new_photo < photo - 100) {
      if (state == 2) {
        state = 1;
        motor_speed = base_pwm;
        increase = -1;
      }
      else if (increase != -1){
        state = 0;
        motor_speed = base_pwm * 0.65;
      }
    }
    else if (new_photo > photo + 100) {
      if (state == 0) {
        state = 1;
        motor_speed = base_pwm;
        increase = 1;
      }
      else if (increase != 1) {
        state = 2;
        motor_speed = base_pwm * 1.5;
        if (motor_speed > 255) motor_speed = 255;
      }
    }
    else if (state == 0) {
      state = 0;
      motor_speed = base_pwm * 0.65;
      increase = 0;
    }
    else if (state == 2) {
      state = 2;
      motor_speed = base_pwm * 1.5;
      if (motor_speed > 255) motor_speed = 255;
      increase = 0;
    }
    else {
      state = 1;
      motor_speed = base_pwm;
      increase = 0;
    }
  }
  else /* if (base_pwm > prev_base_pwm)*/ {
    state = 1;
    motor_speed = base_pwm;
  }

  pushBack(new_photo);
  photo = avg(prev_photo, array_size);

  if (kill) {
    analogWrite(3, 0);
  }
  else {
    analogWrite(3, motor_speed);
  }

  prev_base_pwm = base_pwm;
  previous_photo = photo;

  Serial.println(photo);
  Serial.println(temperature);
  Serial.println(humidity);
  Serial.println(heat_index);
  Serial.println(motor_speed);
  Serial.println(state);
  Serial.println("\n");
  delay(500);
  
}
