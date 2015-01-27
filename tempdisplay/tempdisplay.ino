#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);

const int temps[]  = {984, 982, 980, 978, 976, 974, 972, 969, 967, 964,
962, 959, 956, 953, 950, 946, 943, 940, 936, 932, 928, 924, 920, 916, 911, 907, 
902, 897, 892, 887, 882, 876, 871, 865, 859, 853, 846, 840, 833, 827, 820, 813,
806, 798, 791, 783, 775, 768, 759, 751, 743, 735, 726, 717, 709, 700, 691, 682,
673, 663, 654, 645, 635, 626, 616, 607, 597, 588, 578, 569, 559, 550, 540, 530,
521, 512, 502, 493, 483, 474, 465, 456, 447, 438, 429, 420, 411, 403, 394, 386,
378, 370, 362, 354, 346, 338, 331, 323, 316, 309, 302, 295, 288, 281, 275, 268,
262, 256, 250, 244, 238, 232, 227, 222, 216, 211, 206, 201, 196, 191, 187, 182,
178, 174, 170, 165, 161, 158, 154, 150, 146, 143, 139, 136, 133, 130, 127, 124,
121, 118, 115 };


void setup() {
  // put your setup code here, to run once:
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp 1:  0C");
  lcd.setCursor(0,1);
  lcd.print("Temp 2:  0C");
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  int temp1 = analogRead(A1);
  int temp2 = analogRead(A2);
  
  int i = 0;
  while(temp1<temps[i])  {
    i++;
  }
  temp1 = i-50;
  
  i = 0;
  while(temp2<temps[i])  {
    i++;
  }
  temp2 = i-50;
  
  
  lcd.setCursor(9,0);
  lcd.print(temp1);
  lcd.setCursor(9,1);
  lcd.print(temp2);
  \
  delay(300);
  
}

  
