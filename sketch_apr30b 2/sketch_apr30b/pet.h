#include "variant.h"
#ifndef PET_H
#define PET_H
#include "structs.h"
#include "carrier.h"
#include "const.h"
#include "pet_helper.h"
#include <WiFiNINA.h>
#include <Arduino_HTS221.h>



pet_t* pet_init(int hunger, int happiness, int tiredness){
  pet_t* pet = (pet_t*)calloc(1, sizeof(pet_t));
  pet->hunger = hunger; 
  pet->happiness = happiness; 
  pet->tiredness = tiredness; 
  pet->status_change = true;
  pet->feedback_color = ST77XX_GREEN;
  pet->next_status_update = STATUS_UPDATE_INTERVAL;
  pet->alive = true;
  return pet;
}

void status_text(int x, int y, const char* label, int currentValue, int valueXOffset, int eraseWidth,bool max_stat){
    carrier.display.setTextSize(1);
    int numberStartX = x + valueXOffset;
    carrier.display.fillRect(numberStartX, y, eraseWidth, 8, ST77XX_BLACK); // Viske ud
    carrier.display.setTextColor(ST77XX_WHITE);
    carrier.display.setCursor(x, y);
    carrier.display.print(label);
    carrier.display.print(currentValue);
    if (!max_stat) return;
    carrier.display.print("/");
    carrier.display.print(MAX_STAT);
}

void pet_draw_status(pet_t* pet){
    if (pet->status_change){
    status_text(25, 50, "Sult: ", pet->hunger, 36, 45,true);
    status_text(20, 60, "Glaede: ", pet->happiness, 48, 45,true);
    status_text(20, 70, "Traethed: ", pet->tiredness, 60, 45,true);
    status_text(20, 80, "Alder: ", pet->age, 36, 45,false);
    pet->status_change = false;
  }
}

void pet_feedback_color(pet_t* pet, int color){
  pet->feedback_color = color;
}

void pet_handle_events(pet_t* pet){
  carrier.Buttons.update();

  if (!pet->alive){
    if (carrier.Button0.onTouchDown()) {
      //! NEW PET
      free(pet);
      pet = pet_init(50, 50, 50);
    }
    return;
  }
  
  // Knap 0: Mad
  if (carrier.Button1.onTouchDown()) {
    pet_feed(pet);
  }
 
  // Knap 1: Leg
  if (carrier.Button2.onTouchDown()) {
    pet_play(pet);
  }
 
  // Knap 2: Lur
  if (carrier.Button3.onTouchDown()) {
    pet_sleep(pet);
  } 
}

void pet_draw_feedback(pet_t* pet){
  String current_feedback = "";
  if (pet->feedback_message != "" && millis() < pet->feedback_timeout) {
    current_feedback = pet->feedback_message;
  } else if (pet->feedback_message != "" && millis() >= pet->feedback_timeout) {
    pet->feedback_message = ""; // Nulstil besked
    current_feedback = "";
  }
 
  if(current_feedback != pet->old_feedback_message) {
      carrier.display.setTextSize(2);
      int old_text_width = pet->old_feedback_message.length() * 12;
      carrier.display.fillRect(center_x - (old_text_width / 2) - 1, carrier.display.height() - 50 - 1, old_text_width + 2, 16 + 2, ST77XX_BLACK); // Visk gammel ud
 
      if (current_feedback != "") {
          carrier.display.setTextColor(pet->feedback_color);
          int text_width = current_feedback.length() * 12;
          carrier.display.setCursor(center_x - (text_width / 2), carrier.display.height() - 50);
          carrier.display.print(current_feedback);
      }
      pet->old_feedback_message = current_feedback;
  }
}

void pet_send_data_to_server(pet_t* pet) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected.");
    return;
  }

  String post_data = "{";
  post_data += "\"hunger\":" + String(pet->hunger) + ",";
  post_data += "\"happiness\":" + String(pet->happiness) + ",";
  post_data += "\"tiredness\":" + String(pet->tiredness);
  post_data += "}";

  http_client.beginRequest();
  http_client.post("/insert");
  http_client.sendHeader("Content-Type", "application/json");
  http_client.sendHeader("Content-Length", post_data.length());
  http_client.beginBody();
  http_client.print(post_data);
  http_client.endRequest();

  int status_code = http_client.responseStatusCode();
  String response_body = http_client.responseBody();
  http_client.stop();

  Serial.print("Status: ");
  Serial.println(status_code);
  Serial.println("Response: " + response_body);
}

void pet_update_time_based_status(pet_t* pet){

  bool activate_buzzer = false;

  if (!pet->alive){
    pet->feedback_message = "R.I.P tryk knap 0";
    pet->feedback_timeout = millis() + FEEDBACK_DURATION;
    pet->next_status_update = millis()+STATUS_UPDATE_INTERVAL;
    return;
  }

  if (millis() > pet->next_status_update){
    float temperature = carrier.Env.readTemperature();
    Serial.println(temperature);
    pet->feels_cold = (temperature < 20);



    if (pet->sleeping){
      pet->tiredness = constrain(pet->tiredness - 5, MIN_STAT, MAX_STAT);
      if (pet->tiredness == 0){
        
        pet_sleep(pet); // wake up
      }
    }else if (pet->tiredness == 100){ // auto sleep
      pet_sleep(pet);
      if (!pet->sleeping){ // could not sleep
        activate_buzzer = true;
      }
    }
    
    //increase hunger
    pet->hunger = constrain(pet->hunger + 5, MIN_STAT, MAX_STAT);
    //dont decrease happiness while sleepin prob having good dreams and stuff
    if (!pet->sleeping ) pet->happiness = constrain(pet->happiness - 2, MIN_STAT, MAX_STAT);       

    //if unhappy age is increased faster
    if (pet->happiness < 20){
      pet->age+=0.2;
    }else{
      pet->age+=0.1;
    }
    

    //STUFF TO CHECK FOR WHEN AWAKE
    if (!pet->sleeping){


      if (pet->feels_cold){ 
        pet->happiness = constrain(pet->happiness - 1, MIN_STAT, MAX_STAT); // not happy about being cold yk
        if (pet->feedback_message == ""){
          pet->feedback_message = "Det er koldt!";
          pet->feedback_timeout = millis() + FEEDBACK_DURATION;
        }
      }

      if (pet->hunger > 90){
        pet->age+=0.1;
        pet_set_mood_light(255, 0, 0);

        activate_buzzer = true;
        pet->feedback_message = "Jeg er sulten!";
        pet->feedback_timeout = millis() + FEEDBACK_DURATION;
      }else{
        if (pet->happiness < 20){
          pet_set_mood_light(255, 255, 0); // unhappy
        }else{
          pet_set_mood_light(0, 255, 0); // happy
        }
      }

    }

    if (activate_buzzer){ // makes sure that only 1 buzzer sound will play
      carrier.Buzzer.sound(1200);
      delay(80); 
      carrier.Buzzer.noSound();
    }

    pet->next_status_update = millis()+STATUS_UPDATE_INTERVAL;
    pet->status_change = true;
  }
  
}

#endif
