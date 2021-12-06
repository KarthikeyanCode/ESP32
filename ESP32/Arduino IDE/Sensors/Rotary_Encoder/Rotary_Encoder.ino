//Rotary encoder

#define CLK 5
#define DT 18

int initial = 0;
int present = 0;
int rot_pos = 0;

void setup() 
{
  Serial.begin(115200);
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  initial = digitalRead(CLK);
}

void loop() 
{
  present = digitalRead(CLK);
  if(initial != present)
  {
    //we read DT pin to understand direction of rotation 
    if(present != digitalRead(DT))//clockwise
    {
      rot_pos++;
    }
    else //anti-clockwise
    {
      rot_pos--;
    }
  }
  initial = present;
  //Serial.print("Position of rotary encoder: ");
  Serial.println(rot_pos); 
}
