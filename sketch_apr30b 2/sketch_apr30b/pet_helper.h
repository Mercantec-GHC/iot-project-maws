#ifndef PET_HELPER_H
#define PET_HELPER_H
#include "structs.h"

void pet_feed(pet_t* pet){

  if (pet->sleeping){
      pet->feedback_message = "Zzzz";
      pet->feedback_timeout = millis() + FEEDBACK_DURATION;
      return;
  }


  pet->hunger = constrain(pet->hunger - FEED_AMOUNT, MIN_STAT, MAX_STAT);     // Reducer sult
  pet->tiredness = constrain(pet->tiredness + 1, MIN_STAT, MAX_STAT); // Lidt træt af at spise (juster tal efter ønske)
  pet->feedback_message = "Mums!";
  pet->feedback_timeout = millis() + FEEDBACK_DURATION;
  pet->status_change = true;
}


void pet_play(pet_t* pet){

  if (pet->sleeping){
      pet->feedback_message = "Zzzz";
      pet->feedback_timeout = millis() + FEEDBACK_DURATION;
      return;
  }

  pet->happiness = constrain(pet->happiness + 4, MIN_STAT, MAX_STAT); 
  pet->hunger = constrain(pet->hunger + 5, MIN_STAT, MAX_STAT);       
  pet->tiredness = constrain(pet->tiredness + 3, MIN_STAT, MAX_STAT); 
  pet->feedback_message = "Sjovt!";
  pet->feedback_timeout = millis() + FEEDBACK_DURATION;
  pet->status_change = true;
  pet->age = 0;
}

void pet_read_light_sensor(pet_t* p) {
    if (carrier.Light.colorAvailable()) {
        int r, g, b, a; 
        carrier.Light.readColor(r, g, b, a); 
        int light = g; 
        if (light != p->current_light_level) {
            p->current_light_level = light; 
        }
    }
}

void pet_set_mood_light(int r,int g,int b){
    uint32_t color = carrier.leds.Color(r, g, b); 
    carrier.leds.setPixelColor(MOOD_LED_INDEX, color);
    carrier.leds.show();
}

void pet_sleep(pet_t* pet){
  if (!pet->sleeping && pet->current_light_level > 1){
      pet->feedback_message = "giv m*rke!";
      pet->feedback_timeout = millis() + FEEDBACK_DURATION; 
      return;
  }

  if (pet->sleeping && pet->tiredness >=AWAKE_ENOUGH_THRESHOLD){
      pet->feedback_message = "lidt mere.. Zzzz";
      pet->feedback_timeout = millis() + FEEDBACK_DURATION; 
      return;
  }
  
  if (pet->sleeping && pet->tiredness <= AWAKE_ENOUGH_THRESHOLD){
    pet_set_mood_light(0, 255, 0);
    pet->sleeping = false;
    pet->feedback_message = "God morgen!";
    pet->feedback_timeout = millis() + FEEDBACK_DURATION;
    pet->status_change = true;
    return;
  }

  pet_set_mood_light(0, 0, 255);
  pet->feedback_message = "Zzz";
  pet->feedback_timeout = millis() + FEEDBACK_DURATION;
  pet->status_change = true;
  pet->sleeping = true;
}
#endif