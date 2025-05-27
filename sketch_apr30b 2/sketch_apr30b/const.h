
#define FEEDBACK_DURATION 200
#define MAX_STAT 100
#define MIN_STAT 0
#define BACKGROUND_COLOR ST77XX_BLACK



// Feedback Beskeder Varighed (millisekunder)
#define FEEDBACK_DURATION 1500           // Standard varighed (din oprindelige værdi)
#define FEEDBACK_DURATION_SHORT 1000     // For kortere beskeder som "Sover..."
#define FEEDBACK_DURATION_MEDIUM 2500    // For lidt længere beskeder
#define FEEDBACK_DURATION_LONG 3500      // For vigtige beskeder
#define FEEDBACK_DURATION_VERY_LONG 5000 // For kritiske beskeder som "SULTEN!"


#define PIR_PIN A6                       // Pin til PIR sensor (din oprindelige værdi)
#define PIR_WARMUP_TIME 15000            // Opvarmningstid for PIR sensor (ms) (din oprindelige værdi)

// --- LED Indstillinger ---
#define MOOD_LED_INDEX 2                 // Index for LED der bruges til humør (din oprindelige værdi)
#define LED_BRIGHTNESS_PERCENT 30        // Generel lysstyrke for LEDs (0-100) (din oprindelige LED_BRIGHTNESS)
#define MOOD_LED_UPDATE_INTERVAL 250     // Hvor ofte Mood LED opdateres (ms)


#define STATUS_UPDATE_INTERVAL 5000      // Millisekunder (f.eks. 5 sekunder)
#define HUNGER_INCREASE_RATE 2           // Sult stiger med denne værdi
#define HUNGER_INCREASE_WHILE_ASLEEP 1   // Sult stiger langsommere under søvn
#define HAPPINESS_DECREASE_RATE 1        // Glæde falder med denne værdi
#define TIREDNESS_INCREASE_LIGHT 3       // Træthed stiger i lys
#define TIREDNESS_INCREASE_DARK 1        // Træthed stiger langsommere i mørke
#define FEED_AMOUNT 5

#define LIGHT_THRESHOLD_DARK 50          // Lysniveau (fra sensor) under hvilket det betragtes som mørkt
#define TIRED_FOR_SLEEP_THRESHOLD 80     // Træthedsniveau hvor kæledyret vil forsøge at sove automatisk
#define TIRED_FOR_SLEEP_THRESHOLD_MANUAL 60 // Træthedsniveau for manuel søvn (din oprindelige værdi)

#define AWAKE_ENOUGH_THRESHOLD 20        // Træthedsniveau hvor kæledyret vågner
#define SLEEP_RECOVERY_INTERVAL 10000    // Millisekunder (hvor ofte træthed reduceres under søvn - din oprindelige værdi)
#define SLEEP_RECOVERY_AMOUNT 5          // Hvor meget træthed reduceres med pr. interval (din oprindelige værdi)

#define BACKGROUND_COLOR ST77XX_BLACK // Baggrundsfarve for displayet
