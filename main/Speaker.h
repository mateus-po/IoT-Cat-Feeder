#include <stdint.h>
#include <Arduino.h>
#include "MusicDefinitions.h"
#include "XT_DAC_Audio.h"


class Speaker {
  public:
  void playTwinkleTwinkle();
  void playShortMelody();
  void fillBuffer();
  Speaker(int speakerPin);
  ~Speaker();

  private:

  int8_t PROGMEM TwinkleTwinkle[57] = {
      NOTE_C5,NOTE_C5,NOTE_G5,NOTE_G5,NOTE_A5,NOTE_A5,NOTE_G5,BEAT_2,
      NOTE_F5,NOTE_F5,NOTE_E5,NOTE_E5,NOTE_D5,NOTE_D5,NOTE_C5,BEAT_2,
      NOTE_G5,NOTE_G5,NOTE_F5,NOTE_F5,NOTE_E5,NOTE_E5,NOTE_D5,BEAT_2,
      NOTE_G5,NOTE_G5,NOTE_F5,NOTE_F5,NOTE_E5,NOTE_E5,NOTE_D5,BEAT_2,
      NOTE_C5,NOTE_C5,NOTE_G5,NOTE_G5,NOTE_A5,NOTE_A5,NOTE_G5,BEAT_2,
      NOTE_F5,NOTE_F5,NOTE_E5,NOTE_E5,NOTE_D5,NOTE_D5,NOTE_C5,BEAT_4,  
      NOTE_SILENCE,BEAT_5,SCORE_END
  };
  int8_t PROGMEM ShortMelody[7] = {
    NOTE_C5,NOTE_C5,NOTE_F5,NOTE_F5,NOTE_C5,BEAT_4,SCORE_END
  };

  int speakerPin;
  XT_DAC_Audio_Class* DacAudio;
  XT_MusicScore_Class* Music;  
  XT_MusicScore_Class* ShortMusic;  
};

