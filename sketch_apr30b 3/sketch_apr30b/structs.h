#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct pet{
  int hunger;
  int happiness;
  int tiredness;
  
  float age;
  bool status_change;
  bool sleeping;
  bool alive;
  bool feels_cold;
  int feedback_color;
  String feedback_message; 
  String old_feedback_message; 

  int current_light_level;
  int pir_state;


  unsigned feedback_timeout;

  unsigned next_status_update;

}pet_t;

#endif