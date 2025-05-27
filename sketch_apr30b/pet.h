#include "carrier.h"
#include "const.h"
typedef struct pet
{
  int hunger;
  int happiness;
  int tiredness;
  unsigned long birth_time; // When the pet was created
  unsigned long lifespan;   // Current lifespan in milliseconds
  bool is_alive;            // Is the pet still alive
  bool status_change;

  String feedback_message;
  String old_feedback_message;

  unsigned feedback_timeout;

} pet_t;

pet_t *pet_init(int hunger, int happiness, int tiredness)
{
  pet_t *pet = (pet_t *)calloc(1, sizeof(pet_t));
  pet->hunger = hunger;
  pet->happiness = happiness;
  pet->tiredness = tiredness;
  pet->birth_time = millis(); // Set the birth time to the current time
  pet->lifespan = 0;          // Initialize lifespan
  pet->is_alive = true;       // Pet is alive when created
  pet->status_change = true;
  return pet;
}

void status_text(int x, int y, const char *label, int currentValue, int valueXOffset, int eraseWidth)
{
  carrier.display.setTextSize(1);
  int numberStartX = x + valueXOffset;
  carrier.display.fillRect(numberStartX, y, eraseWidth, 8, ST77XX_BLACK); // Viske ud
  carrier.display.setTextColor(ST77XX_WHITE);
  carrier.display.setCursor(x, y);
  carrier.display.print(label);
  carrier.display.print(currentValue);
  carrier.display.print("/");
  carrier.display.print(MAX_STAT);
}

void pet_draw_status(pet_t *pet)
{
  if (pet->status_change)
  {
    status_text(25, 50, "Sult: ", pet->hunger, 36, 45);
    status_text(20, 60, "Glaede: ", pet->happiness, 48, 45);
    status_text(20, 70, "Traethed: ", pet->tiredness, 60, 45);
    pet->status_change = false;
  }
}

void pet_handle_events(pet_t *pet)
{
  carrier.Buttons.update();

  // Knap 0: Mad
  if (carrier.Button0.onTouchDown())
  {
    Serial.println("Knap 0 trykket (Mad)");
    pet->hunger = constrain(pet->hunger - 20, MIN_STAT, MAX_STAT);      // Reducer sult
    pet->tiredness = constrain(pet->tiredness + 1, MIN_STAT, MAX_STAT); // Lidt træt af at spise (juster tal efter ønske)
    pet->feedback_message = "Mums!";
    pet->feedback_timeout = millis() + FEEDBACK_DURATION;
    pet->status_change = true;
  }

  // Knap 1: Leg
  if (carrier.Button1.onTouchDown())
  {
    Serial.println("Knap 1 trykket (Leg)");
    pet->happiness = constrain(pet->happiness + 4, MIN_STAT, MAX_STAT);
    pet->hunger = constrain(pet->hunger + 5, MIN_STAT, MAX_STAT);
    pet->tiredness = constrain(pet->tiredness + 3, MIN_STAT, MAX_STAT);
    pet->feedback_message = "Sjovt!";
    pet->feedback_timeout = millis() + FEEDBACK_DURATION;
    pet->status_change = true;
  }

  // Knap 3: Lur
  if (carrier.Button3.onTouchDown())
  {
    Serial.println("Knap 3 trykket (Lur)");
    pet->tiredness = constrain(pet->tiredness - 10, MIN_STAT, MAX_STAT);
    pet->feedback_message = "Zzz";
    pet->feedback_timeout = millis() + FEEDBACK_DURATION;
    pet->status_change = true;
  }
}

void pet_draw_feedback(pet_t *pet)
{
  String current_feedback = "";
  if (pet->feedback_message != "" && millis() < pet->feedback_timeout)
  {
    current_feedback = pet->feedback_message;
  }
  else if (pet->feedback_message != "" && millis() >= pet->feedback_timeout)
  {
    pet->feedback_message = ""; // Nulstil besked
    current_feedback = "";
  }

  if (current_feedback != pet->old_feedback_message)
  {
    carrier.display.setTextSize(2);
    int old_text_width = pet->old_feedback_message.length() * 12;
    carrier.display.fillRect(center_x - (old_text_width / 2) - 1, carrier.display.height() - 30 - 1, old_text_width + 2, 16 + 2, ST77XX_BLACK); // Visk gammel ud

    if (current_feedback != "")
    {
      carrier.display.setTextColor(ST77XX_GREEN);
      int text_width = current_feedback.length() * 12;
      carrier.display.setCursor(center_x - (text_width / 2), carrier.display.height() - 30);
      carrier.display.print(current_feedback);
    }
    pet->old_feedback_message = current_feedback;
  }
}

// Maximum lifespan (1 week in milliseconds)
#define MAX_LIFESPAN (7UL * 24 * 60 * 60 * 1000)     // 7 days in milliseconds
#define LIFESPAN_DECREMENT_INTERVAL (10 * 60 * 1000) // Check lifespan every 10 minutes

void pet_update_lifespan(pet_t *pet)
{
  if (pet->is_alive)
  {
    unsigned long current_time = millis();
    // Check if it's time to decrement the lifespan
    if (current_time - pet->birth_time >= LIFESPAN_DECREMENT_INTERVAL)
    {
      pet->lifespan += LIFESPAN_DECREMENT_INTERVAL;
      pet->birth_time = current_time; // Reset the birth time
    }
    // Check if the pet has reached the maximum lifespan
    if (pet->lifespan >= MAX_LIFESPAN)
    {
      pet->is_alive = false; // Set the pet as not alive
      pet->feedback_message = "Din kæledyr er død.";
      pet->feedback_timeout = millis() + FEEDBACK_DURATION;
      pet->status_change = true;
    }
  }
}

bool pet_is_alive(pet_t *pet)
{
  return pet->is_alive;
}

void pet_draw_death(pet_t *pet)
{
  if (!pet->is_alive)
  {
    carrier.display.setTextSize(2);
    carrier.display.setTextColor(ST77XX_RED);
    carrier.display.setCursor(center_x - 50, center_y - 10);
    carrier.display.print("Din kæledyr er død.");
    carrier.display.setCursor(center_x - 30, center_y + 10);
    carrier.display.print("Tryk for at genstarte");
  }
}
