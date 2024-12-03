
// realtime database


typedef struct {
    // leds
    int led0;
    int led1;
    int led2;
    int led3;

    // buttons
    int button0;
    int button1;
    int button2;
    int button3;

} RT_db;


// Function to initialize the database
void RT_db_init(RT_db *db);
int RT_db_update(RT_db *db, char* command);
void RT_db_print(RT_db *db);

