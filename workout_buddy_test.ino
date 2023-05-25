// Treadmill buddy
// This code allows the Arduino to play music, changing the tempo and the song in accordance with the user's
// button presses.  It is intended for use on a treadmill, and thus displays tempo as a speed (in half-miles-per-hour),
// as well as the total traveled distance, as a binary on sets of LEDs.  One button increases the speed; another decreases
// it; a third moves from one song to the next.

// Pinout
const byte audioPin = 2; // Connect this pin in series with resistor and speaker to ground.
const byte distPins[] = {4,5,6,7}; // Each of these, with a green LED and resistor in series to ground.
const byte speedPins[] = {8,9,10,11}; // These, with a blue LED and resistor in series to ground.
const byte inputPins[] = {12,13,3}; // These, with buttons and (for now) resistors in series to ground.

// Debounced button variables
bool inputPinStats[] = {false,false,false};
int inputPinWaits[] = {0,0,0};
const int debounceDelay = 10;

// State variables
// byte song; // The index of the song currently playing, times two (or an odd number in between songs)
byte note = 0; // The index of the note in the song currently playing
int music_speed = 1023; // Current number of milliseconds corresponding to a sixteenth note
unsigned long noteStartTime = 0; // Time in ms when the current note started
int noteDuration = 0; // Duration in ms of the current note
byte loops = 0; // The number of times that the player has looped through all the songs.
byte transpose = 0; // The notes up which to transpose songs.

int my_speed = 0; // measured in half-miles per hour.
long my_long_dist = 0; // increased by speed every second.
float my_dist = 0; // in quarters of miles
bool increased_this_second = false;

//////////////// Music as chiptunes
// "Auld Lang Syne" - tune from Wikipedia, CC 3.0
byte auld_lang_notes[] = { 48,    53,  53, 53, 57,  55, 53, 55, 57, 53, 53, 57, 60, 62,
//                         Should auld acquaintance be forgot and never brought to mind?
                            62, 60, 57, 57, 53,     55, 53, 55, 57, 53, 50, 50, 48, 53,
//                         Should auld acquaintance be forgot and days of auld lang syne?
                            62, 60, 57, 57, 53, 55, 53, 55, 62, 60, 57, 57, 60, 62,
//                         For auld lang syne, my dear, for au-auld la-ang syne
                            62, 60, 57, 57, 53, 55, 53, 55, 57, 53, 50, 50, 48, 53, 0 };
//                         We'll take a cup o' kindness yet for au-auld la-ang syne.
byte auld_lang_times[] = { 4, 6, 2, 4, 4, 6, 2, 4, 4, 6, 2, 4, 4, 12, 4, 6, 2, 4, 4, 6, 2, 4, 4, 6, 2, 4, 4, 12,
                          4, 6, 2, 4, 4, 6, 2, 4, 4, 6, 2, 4, 4, 12, 4, 6, 2, 4, 4, 6, 2, 4, 4, 6, 2, 4, 4, 12, 0};

// "Pop Goes the Weasel" - tune worked out by me
byte weasel_notes[] = { 48, 48, 50, 50, 52, 55, 52, 40,   43, 48, 48, 50, 50, 52, 48,   43, 48, 48, 50, 50, 52, 55, 52, 48,   57, 50, 53, 52, 48,
                          60, 60, 57, 57, 59, 59, 55,   60, 60, 57, 57, 59, 55,   48, 48, 50, 50, 52, 52, 55,   57, 50, 53, 52, 48,  0 };
byte weasel_times[] = { 4, 2, 4, 2, 2, 2, 2, 4, 2, 4, 2, 4, 2, 6, 4, 2, 4, 2, 4, 2, 2, 2, 2, 6, 6, 4, 2, 6, 6,
                          4, 2, 4, 2, 4, 2, 6, 4, 2, 4, 2, 6, 6,    4, 2, 4, 2, 4, 2, 6,  6, 4, 2, 6, 6, 0 };

// "Oh, My Darling Clementine" - from 8notes.com, CC 3.0
byte clemen_notes[] = { 53, 53, 53, 48, 57, 57, 57, 53, 53, 57, 60, 60, 58, 57, 55,
                          55, 57, 58, 58, 57, 55, 57, 53, 53, 57, 55, 48, 52, 55, 53, 0 };
byte clemen_times[] = { 3, 1, 4, 4, 3, 1, 4, 4, 3, 1, 7, 1, 3, 1, 8, 3, 1, 4, 4, 3, 1, 4, 4, 3, 1, 4, 4, 3, 1, 8, 0};

// "Yankee Doodle" - from 8notes.com, CC 3.0
byte yd_notes[] = { 67, 67, 69, 71, 67, 71, 69, 62, 67, 67, 69, 71, 67, 62, 67, 67, 69, 71, 72, 71, 69, 67, 66, 62, 64, 65, 67, 67,
                        64, 66, 64, 62, 64, 66, 67, 62, 64, 62, 60, 59, 52, 64, 66, 64, 62, 64, 66, 67, 64, 62, 67, 66, 69, 67, 67, 0};
byte yd_times[] = { 4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  8, 8,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 8, 8,
                        6, 2, 4, 4, 4, 4, 8, 6, 2, 4, 4, 8, 8, 6, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 0};

// "A Hot Time in the Old Town Tonight" - from public-domain arrangement by the Remick corporation
byte hot_notes_in_old_town_tonight[] = { 71, 59, 71, 59, 71, 59, 59, 70, 71, 72, 71, 70, 71, 67, 62, 62, 67, 62, 67, 71, 
                                        74, 59, 74, 59, 74, 59, 59, 73, 74, 76, 74, 73, 74, 71, 62, 64, 67,
                                        71, 59, 71, 59, 71, 59, 59, 70, 71, 72, 71, 70, 71, 67, 67, 66, 67,
                                        71, 69, 62, 62, 71, 69, 62, 67, 64, 62, 59, 55, 67, 79, 0};
byte hot_times_in_old_town_tonight[] = { 4, 4, 4, 4, 4, 4, 4, 3, 1, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 
                                        4, 4, 4, 4, 4, 4, 4, 3, 1, 4, 4, 4, 4, 4, 4, 4, 4, 
                                        4, 4, 4, 4, 4, 4, 4, 3, 1, 4, 4, 4, 4, 4, 4, 4, 4, 
                                        4, 8, 2, 2, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 0};

// "Seeing Nelly Home" - tune worked out by me, though different covers sing it differently
byte quilting_notes[] = { 64, 65, 67, 67, 69, 69, 67, 72, 67, 72, 72, 72, 71, 69, 72, 67, 48, 52, 55, 60, 64, 65, 
                          67, 72, 76, 72, 74, 72, 69, 72, 72, 72, 71, 72, 74, 74, 72, 60, 48, 0};
byte quilting_times[] = { 2, 2, 2, 6, 2, 6, 4, 4, 4, 2, 2, 6, 2, 2, 6, 2, 2, 2, 2, 4, 2, 2, 
                          4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 6, 2, 2, 6, 4, 4, 4, 0};

// "Michael, Row the Boat Ashore" - 8notes.com, CC 3.0
byte michael_notes_the_boat_ashore[] = { 48, 52, 55, 52, 55, 57, 55, 52, 55, 57, 55, 52, 55, 55, 52, 53, 52, 
                                        50, 48, 50, 52, 50, 48, 48, 52, 55, 52, 55, 57, 55, 52, 55, 57, 55, 52, 55, 55, 52, 53, 52, 50, 48, 50, 52, 50, 48, 0 };
byte michael_times_the_boat_ashore[] = { 4, 4, 6, 2, 4, 4, 8, 4, 4, 16, 8, 4, 4, 6, 2, 4, 4, 8, 4, 4, 8, 8, 8, 4, 4, 6, 2, 4, 4, 8, 4, 4, 16, 8, 4, 4, 6, 2, 4, 4, 8, 4, 4, 8, 8, 8, 0};

// "London Bridge" - tune worked out by me
byte london_notes[] = { 67, 69, 67, 65, 64, 65, 67, 62, 64, 65, 64, 65, 67, 67, 69,67, 65, 64, 65, 67, 62, 67, 64, 60, 0 };
byte london_times[] = { 3, 1, 2, 2, 2, 2, 4, 2, 2, 4, 2, 2, 4, 3, 1, 2, 2, 2, 2, 4, 4, 4, 2, 6, 0 };

// The locations of these songs in memory are given by byte pointers, which can be indexed as if they were the byte arrays themselves.
// These arrays of pointers (each of which points to the first note in the above arrays) determine the order in which songs are played.
byte *song_note_pointers[] = { &auld_lang_notes[0], &weasel_notes[0], &london_notes[0], &yd_notes[0], &hot_notes_in_old_town_tonight[0], &clemen_notes[0], &quilting_notes[0], &michael_notes_the_boat_ashore[0] };
byte *song_time_pointers[] = { &auld_lang_times[0], &weasel_times[0], &london_times[0], &yd_times[0], &hot_times_in_old_town_tonight[0], &clemen_times[0], &quilting_times[0], &michael_times_the_boat_ashore[0] };
byte sixteenths_in_beats[] = { 6, 12, 8, 16, 16, 8, 16, 8 }; // The number of sixteenth notes to constitute one beat of each song above.

const int song_count = sizeof(song_time_pointers) / sizeof(song_time_pointers[0]); // The number of songs
byte song = song_count * 2 - 1; // The index of the current song, times two (or odd, if between songs).
// Initially, do not be playing a song, but be ready to play the first.

////////////////////////////////////////////////////////////////////
// Real code begins here.
////////////////////////////////////////////////////////////////////

// The express functions display a value as binary on the array of four pins
// sent to int.  Integers are shown by their last four digits in binary;
// floats are converted to integers and the same thing done.
void express(int val, byte pins[]) {
  for (byte i = 0; i < 4; i++) {
    int ander = 1 << i;
    digitalWrite(pins[i], val & ander);
  }
}

void express(float val, byte pins[]) {
  int val_as_int = val;
  for (byte i = 0; i < 4; i++) {
    int ander = 1 << i;
    digitalWrite(pins[i], val_as_int & ander);
  }
}

// This function converts the traveled distance from half-miles per hour times seconds
// into quarter miles, for ease of display.  As it is one line, it may need not be
// a function; its being a function reflects a version which used more complicated units.
float quarter_mileize(long long_value) {
  return long_value / 1800;
}

// This function calculates the length of the smallest note at a given speed
// and for a given quantity of such note in a beat.
int millipernote(int given_speed, byte notes_per_beat) {
  long long_speed = given_speed;
  // The output is the number of milliseconds in a sixteenth note
  // for a given speed (in half-miles per hour) and the number of
  // sixteenth notes in a beat, such that someone moving at that
  // speed can easily step on the beat.  It is calibrated for my stride:
  // a step is 2.5 feet walking (3mph), four feet running (6 mph).
  // Thus the length of a step in feet is about one more than the speed in half-mph.
  // Speed in ft/s is, of course, 2640/3600 times the speed in half-mph,
  // Dividing the stride by that speed is the length of a step or beat in seconds;
  // multiplying by 1000 and dividing by the number of notes in a
  // beat is the length of a note in milliseconds.  This simplifies to:
  
  // if (long_speed) return ((15000 * long_speed + 15000) / ( 11 * long_speed * notes_per_beat ));
  
  // but it is more efficient and more musically interesting to write:

  if (long_speed) return ((long_speed+16)<<9) / (long_speed * notes_per_beat);
  else return 1023; // If speed is zero, play very slowly.
}

int mid_to_freq(byte midi_value) { // Converts MIDI notes to frequencies in real time
  int octave = 55 * pow(2,((midi_value - 33) / 12)); // The C at 32 in MIDI is 110 Hz.
                                                     // Each C being 12 higher in MIDI, is double the frequency.
  float tuning = pow(2.0, (double)((midi_value - 33) % 12) / 12);
                                                    // If the note is not C, equal-temperament tuning gives its frequency
                                                    // as a fractional power of 2 above the C below it.
  return octave * tuning;
}

int transposition(byte loops) {
  return loops + 6 * (loops % 2); // The number of keys up by which one transposes to go however many key changes.
  // This is initially 7 (to go up a fifth, as from C to G), then 2 (to go up a whole step, or two fifths but back down an octave, as from C to G to D).
}

/////////////////////////////////////////////////////////////////////////////
// Main code begins here.
////////////////////////////////////////////////////////////////////////////

void setup() {
  // Set up pins  
  for (byte i = 0; i < 4; i++) {
    pinMode(distPins[i],OUTPUT);
    pinMode(speedPins[i],OUTPUT);
  }
  express(my_speed,speedPins);
  express(my_dist,distPins);
  for (byte i = 0; i < 3; i++) {
    pinMode(inputPins[i],INPUT);
  }
}

void loop() {
  // Check buttons for pressing - if a button remains in an altered state from the last acknowledged for n loops of this code,
  // acknowledge a change in its state.  If that state is that it is now pressed, perform the function of that button.
  for (byte i = 0; i < 3; i++) {
    if (digitalRead(inputPins[i]) != inputPinStats[i]) {
      ++inputPinWaits[i];
      if (inputPinWaits[i] == debounceDelay) {
        if ( !inputPinStats[i] ) {
          Serial.print("Button #");
          Serial.print(i);
          Serial.println(" has been pressed.");
          inputPinStats[i] = true;
          // The first button decreases the speed, assuming it isn't already 0.
          if (i == 0 && my_speed > 0) {
            --my_speed;
            music_speed = millipernote(my_speed, sixteenths_in_beats[song >> 1]);
            express(my_speed,speedPins);
          }
          // The second button increases the speed.
          else if (i == 1) {
            ++my_speed;
            music_speed = millipernote(my_speed, sixteenths_in_beats[song >> 1]);
            express(my_speed,speedPins);
          }
          // The last one moves to the next song or to the silence between songs,
          // and may need to loop around the playlist, updating transposition accordingly.
          else {
            ++song;
            if (song == song_count << 1)
              { song = 0; transpose = transposition(loops); ++loops; }
            note = 0;
            Serial.print("Song #: "); Serial.println(song);
            music_speed = millipernote(my_speed, sixteenths_in_beats[song >> 1]);         
          }
        } else inputPinStats[i] = false;
      }
    } else inputPinWaits[i] = 0;
  }
  // If this code has not run since the last one-second tick, run it again:
  // that is, increase the distance traveled by the latest second's worth of running.
  if ( !increased_this_second && (millis() % 1000 < 100 )) {
    my_long_dist += my_speed;
    increased_this_second = true;
    my_dist = quarter_mileize(my_long_dist);
    express(my_dist,distPins);
  } else if ( millis() % 1000 > 900 ) increased_this_second = false;
  // Play music!
  if ( !(song & 1) && (millis() - noteStartTime > noteDuration )) { // If a song is playing, and a note's allotted time is over.    
    noteStartTime = millis();
    noteDuration = song_time_pointers[song >> 1][note] * music_speed;
    tone(audioPin, mid_to_freq(song_note_pointers[song >> 1][note] + transpose), music_speed); // Play note!  Buzz for the length of a sixteenth note, no matter how long note is.
    ++note; if (!(song_note_pointers[song >> 1][note])) note = 0;
  }
  delay(1); // Wait a millisecond, for convenience of debouncing.
}
