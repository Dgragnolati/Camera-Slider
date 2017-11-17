
// Pulse and direction pins wired to Stepper Driver
#define S_Pulse 4
#define S_Direction 5
// Joystick and Potentiometer (to control speed and jogging)
#define Y_Stick_Pos A5
#define D_Speed A4
// Butons to set start/stop points and button to play.  SetA is joystick Button B Play is white play button
#define B_SetA 3
#define B_Play 2
// Homes switch 
#define S_Home 6
// RGB LED 
#define LED_Red 12
#define LED_Blue 10
#define LED_Green 11
// Used to define the direction, if pullies are added that change direction, or system is wired differently, use these to change system direction 
#define fwd 1
#define rvs 0
// set and defines the speed at which the system homes 
#define h_speed 150
// Used in a mapping function to define the lowest and fastests speeds of the system 
#define speed_lower_limit 50
#define speed_upper_limit 120
#define max_buffer 16
// Define the lower and upper limit of travel, based on step count 
long steps_rev =3200;
long distance_lower_limit= 0.00 ;
long distance_upper_limit = steps_rev*22.4;
// This is not really used but if you ever needed some type of scaler (Prob should take it out but I am not going to) 
float steps_per_distance= 1;
// variable to hold the set points of start and stop 
long start_location = 0.00 ;
long end_location =0.00 ;
// Define what state of the program it is in 
int program_status=0;
// Current location.  Varaible that store the location, each time the system steps it adds or subtracts to this value 
long current_location=99999.00;
//Varible from serial port to start location
long incoming_start_location = 0.00;
//Varible from serial port to take a location and drive to it 
long incoming_stop_location = 0.00;
//Variable from serial port to take a speed and drive at that speed
long incoming_speed=0;
// Control Mode 
float mode=0;
// Used to track debouncing of buttons 
int debounce_delay = 100;
// Used to keep track of the speed of the system 
long current_speed=0;
// Tracks the button states after they have been pressed 
int B_SetA_State=1;
int B_Play_State=1;
// delay varaible 
int ZZ =0;
//Used to ++/-- the step counts (There is likely a prettier way to do this but I am not rewriring the code, maybe in rev 2) 
int step_multiplier = 0;
// serial buffer shit 
char serial_buffer[max_buffer];
//counter for serial buffer
int serial_count=0;
boolean start_program = 0;
String command = "";

void setup() {
// Define all the inputs and outputs. See above for def's 
Serial.begin(9600);  
Serial.println("Welcome to JS Camera Dolly Shit is about to get real");
pinMode(LED_Red,OUTPUT);
pinMode(LED_Blue,OUTPUT);
pinMode(LED_Green,OUTPUT);
pinMode(S_Pulse,OUTPUT);
pinMode(S_Direction,OUTPUT);
pinMode(Y_Stick_Pos,INPUT);
pinMode(D_Speed,INPUT);
pinMode(B_SetA,INPUT);
pinMode(B_Play,INPUT);
pinMode(S_Home,INPUT);
digitalWrite(S_Home,HIGH);
digitalWrite(B_Play,HIGH);
digitalWrite(B_SetA,HIGH);

// Start program flash LED to let you know system is starting up 
// Flash Red
digitalWrite(LED_Red,HIGH);
delay(1000);
digitalWrite(LED_Red,LOW);
// Flash Green
digitalWrite(LED_Green,HIGH);
delay(1000);
digitalWrite(LED_Green,LOW);
// Flash Blue
digitalWrite(LED_Blue,HIGH);
delay(1000);
digitalWrite(LED_Blue,LOW);
//
}

void loop() {
// Check if the system is in startup mode and home it if it is. 
if (current_location==99999.00) {
homeing();
}
// check the dial and set the speed of the system     
current_speed= map(analogRead(D_Speed),0,1024,speed_upper_limit,speed_lower_limit);
//Used to read out values of inputs.  Have the ZZ to reduce number of times printing to serial monitor
//Used to interface with computer
if (Serial.available() > 0) {
  if (readSerial()==true){
      command = serial_buffer;
      Serial.println (command);             
    if (command=="Start" && start_program==0){
    Serial.println("Please enter start location, end location, and speed '>'");
    start_program=1;
    }

  if (command.indexOf(',') > 0 && start_program==1){
      start_program=0;
      parsecommand(command);
  }
                                 
}
}

//Serial.print("Current Location - ");
//Serial.println(current_location);
//Serial.println(current_speed);
//Serial.println(analogRead(Y_Stick_Pos));
//Serial.println(digitalRead(S_Home));
// Check valuces of the joystick to see  if someone is jogging the thing, it uses the current speed to drive the system 
// Chcek if fwd
  if (analogRead(Y_Stick_Pos)<400) {
   if (current_location<=distance_upper_limit) {
    digitalWrite(S_Direction,fwd);  
      delayMicroseconds(10);
        digitalWrite(S_Pulse,HIGH);
        delayMicroseconds(current_speed);
        digitalWrite(S_Pulse,LOW);
        current_location = current_location + 1;
   }
  }
// check if backward 
  if (analogRead(Y_Stick_Pos)>500) {    
    if (digitalRead(S_Home)==1){
        digitalWrite(S_Direction,rvs);    
        digitalWrite(S_Pulse,HIGH);
        delayMicroseconds(current_speed);
        digitalWrite(S_Pulse,LOW);
        current_location = current_location + -1;
   
    }
  }

// Check that status of the Joystick buttons, incldues a delay to debounce the button press
  if (digitalRead(B_SetA)==0) {
   delay(debounce_delay);
   if (digitalRead(B_SetA)==0) {
    B_SetA_State=0;
    Serial.println("Button A Pressed");
    }
  }
// Check that status Play button has a number of delays to check if pressed, short, medium or long.  Short, plays, medium resests the start and end points, and Long homes the system
  if (digitalRead(B_Play)==0) {
   delay(debounce_delay);
    if (digitalRead(B_Play)==0) {
      B_Play_State=0;
      Serial.println("Button B Pressed");
      delay(1000);
      if (digitalRead(B_Play)==0) {
        Serial.println("Button B Held, Reseting System ");
        B_Play_State=1;
        program_status=0;
        digitalWrite(LED_Green,HIGH);
        digitalWrite(LED_Blue,LOW);
        digitalWrite(LED_Red,LOW);
        delay(4000);
        if (digitalRead(B_Play)==0) {
          Serial.println("Button B Held Longtime, Homeing System  ");
          digitalWrite(LED_Green,LOW);
          homeing();
        }
      }
  }
}

// Runs through the programs and sets start and stop, and plays the set points if necessary
  if (B_SetA_State == 0 && program_status == 0 ) {
   start_location=current_location;
   digitalWrite(LED_Green,LOW);
   digitalWrite(LED_Blue,LOW);
   digitalWrite(LED_Red,HIGH);
   program_status=1;
   B_SetA_State=1;
   Serial.println("Set Start Postion at ");
   Serial.println(current_location);
   } 
  
  if (B_SetA_State == 0 && program_status == 1 ) {
   end_location=current_location; 
   digitalWrite(LED_Red,LOW);
   digitalWrite(LED_Blue,HIGH);
   program_status=2;
   B_SetA_State=1;
   Serial.println("Set Stop Position at  ");
   Serial.println(current_location);
   }
  
  if (B_SetA_State == 0 && program_status >= 2 ) {
    Serial.println("Currently at   ");
    Serial.print(current_location);
    Serial.print("Moving to  ");
    Serial.println(start_location);
    travel(start_location);
    digitalWrite(LED_Green,HIGH);
    program_status= 3; 
    B_SetA_State=1;
    Serial.println("Holding at  ");
    Serial.println(current_location);
  } 
  
  if (B_Play_State == 0 && program_status ==3)  {
  Serial.println("Moving to end location");
  travel(end_location); 
  B_Play_State=1;
  }
  
}


// function to home the system using limit switch at back end
void homeing(){
 Serial.println("System Homeing");
  if (digitalRead(S_Home) ==0) {
        Serial.println("Started in Home");
        current_location=0.00;
        travel(10.0);
  }
digitalWrite(S_Direction,rvs);    
  while (digitalRead(S_Home)==1){
      digitalWrite(LED_Red,HIGH);
      digitalWrite(S_Pulse,HIGH);
      delayMicroseconds(h_speed);
      digitalWrite(S_Pulse,LOW);
     // Serial.println("Moving System Backwards");
     // Serial.println(digitalRead(S_Home));
     
  }
digitalWrite(S_Direction,fwd);
  while (digitalRead(S_Home)==0) {
  digitalWrite(S_Pulse,HIGH);
  delayMicroseconds(h_speed);
  digitalWrite(S_Pulse,LOW);
  //  Serial.print("Moving System Forwards");
  //  Serial.println(digitalRead(S_Home));
    if (digitalRead(S_Home) != 1){
      current_location=0.00;
      digitalWrite(LED_Red,LOW);
      digitalWrite(LED_Green,HIGH);
      // Serial.println("Setting Current Location");
     }
  }
}

// fucntion to move the system basedo where it currently "Thinks" it is.  (NO encoder steps only, jaming/ missed steps  make this a risk)
void travel(float location) {
 
 if (location - current_location < 0) { 
  digitalWrite(S_Direction,rvs);
  step_multiplier = -1 ;
  Serial.println("Moving Backward");
 }
 else {
  digitalWrite(S_Direction,fwd);
 step_multiplier = 1 ;
  Serial.println("Moving Forward");
 }
 float dis= abs (location - current_location ) ; 
 Serial.print("Moving___");
 Serial.println(dis);
 float temp_speed=current_speed*10;
 for ( long i=0; i<= long(dis*steps_per_distance);i++){
    digitalWrite(S_Pulse,HIGH);
    delayMicroseconds(temp_speed);
    digitalWrite(S_Pulse,LOW);
    current_location= current_location + (1*(step_multiplier));
    Serial.println(current_location);
    if (current_location >= distance_upper_limit || current_location <= distance_lower_limit) {
      Serial.println("Range Exceeded");
      break ;
    if (temp_speed>current_speed){
      temp_speed=temp_speed-1;
    }
    }
  }   
  Serial.println("Travel Complete");
}

//Read serial buffer looking for > to end the thing 

boolean readSerial()  {

while (Serial.available() > 0) {
    char incoming_byte=Serial.read();
    
    if (incoming_byte != '>'){ 
       serial_buffer[serial_count]= incoming_byte;
       Serial.print(incoming_byte);
       serial_count++;
       
    if (serial_count>=max_buffer) {
       serial_count=max_buffer-1;
       }
       return false;
    }
      
    
    else {
       serial_buffer[serial_count]= '\0';
       serial_count=0;
       Serial.println("");
       return true;
    }
} 
  
}

//Parses through everthing looking for commas, takes those commands and feeds them to create set points and speed

boolean parsecommand(String command) {

  Serial.println("Executing Command");
  int commaIndex = command.indexOf(',');
    //  Search for the next comma just after the first
  int secondCommaIndex = command.indexOf(',', commaIndex + 1);

  Serial.println (" Strings ");
  String strstart = command.substring(0, commaIndex);
  Serial.println (strstart);
  String strstop = command.substring(commaIndex + 1, secondCommaIndex);
  Serial.println (strstop);
  String strspeed= command.substring(secondCommaIndex + 1); // To the end of
  Serial.println (strspeed);
  
  Serial.println (" Floats ");
  incoming_start_location=strstart.toFloat();
  Serial.println(incoming_start_location);
  incoming_stop_location=strstop.toFloat();
  Serial.println(incoming_stop_location);
  incoming_speed=strspeed.toFloat();
  Serial.println( incoming_speed);
 
  if (incoming_start_location < distance_upper_limit && incoming_stop_location < distance_upper_limit) {
    travel(incoming_start_location);
    current_speed = incoming_speed;
    travel(incoming_stop_location);           
    Serial.println("Command Complete");
     }
   else {
    Serial.println("Start or Stop location out of range, don't be a shit");
     }

}


