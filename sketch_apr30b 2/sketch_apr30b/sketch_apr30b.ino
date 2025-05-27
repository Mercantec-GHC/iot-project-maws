#include "carrier.h"
#include "image.h"
#include "pet.h"
#include "structs.h"

animation_t* smile;
pet_t* pet;
void setup() {
  Serial.begin(9600);

  while(!carrier.begin());
  delay(1500); // cant read temp without the delay
  carrier.display.setRotation(2);
  carrier.display.fillScreen(0x0000); //clear screen

  center_x = carrier.display.width()/2;
  center_y = carrier.display.height()/2;


  pet = pet_init(50, 50, 50);
  smile = animation_create(2,100);
  animation_add_frame(smile,epd_bitmap_image_1);
  animation_add_frame(smile,epd_bitmap_image_2);

  carrier.leds.begin();
  carrier.leds.setBrightness(LED_BRIGHTNESS_PERCENT);
  carrier.leds.fill(0,0,0);
  carrier.leds.show();
}

unsigned long last_sent = 0;

void loop() {
  pet_read_light_sensor(pet);
  //Serial.println(pet->current_light_level);
  animation_step(smile);
  animation_draw_frame(center_x-32, center_y-32, 64, smile);
  pet_handle_events(pet);
  pet_draw_feedback(pet);
  pet_update_time_based_status(pet);
  pet_draw_status(pet);
  
  if (pet->age > 5) pet->alive = false;

  if (millis() - last_sent > 30000) {
    pet_send_data_to_server(pet);
    last_sent = millis();
  }
}
