#include "Speaker.h"
#include <stdint.h>
#include <Arduino.h>
#include "MusicDefinitions.h"
#include "XT_DAC_Audio.h"

Speaker::Speaker(int speakerPin): speakerPin(speakerPin) {
  DacAudio = new XT_DAC_Audio_Class(speakerPin, 0);
  Music = new XT_MusicScore_Class(TwinkleTwinkle, TEMPO_PRESTISSIMO, INSTRUMENT_ORGAN);
  ShortMusic = new XT_MusicScore_Class(ShortMelody, TEMPO_PRESTISSIMO, INSTRUMENT_ORGAN);
  DacAudio->DacVolume = 90;
}
Speaker::~Speaker() {
  delete DacAudio;
  delete Music;
  delete ShortMusic;
}
void Speaker::playTwinkleTwinkle() {
  Serial.println("Checking if playing long melody");
  if (Music->Playing || ShortMusic->Playing) return;

  Serial.println("Playing long melody");
  DacAudio->Play(Music);
}
void Speaker::playShortMelody() {
  Serial.println("Checking if playing short melody");
  if (Music->Playing || ShortMusic->Playing) return;

  Serial.println("Playing short melody");
  DacAudio->Play(ShortMusic);
}
void Speaker::fillBuffer() {
  DacAudio->FillBuffer();
}