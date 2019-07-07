/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz sound classes
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DSoundEngine.h"
#include "vorbis/vorbisfile.h"

extern "C"
{
#include "blorb.h"
}

#define WM_SOUND_NOTIFY WM_APP+1

class FrotzWnd;

class FrotzSound
{
public:
  FrotzSound(int sound, unsigned short eos, BYTE* data, int length, bool del);
  virtual ~FrotzSound();

  // Play the sound
  virtual bool Play(int repeat, int volume) = 0;
  // Check if the sound is still playing
  virtual bool IsPlaying(void) = 0;

  // Get the number of the sound
  int GetSoundNumber(void);
  // Get the Z-code to routine at the end
  unsigned short GetEndRoutine(void);
  // Get the sound volume in decibels
  int GetDecibelVolume(int volume);

public:
  // Initialize the sound engine
  static bool Init(FrotzWnd* wnd);
  // Stop the sound engine
  static void ShutDown(void);

  // Start playing a sound
  static void Play(int sound, bb_map_t* map, int repeat, int volume, unsigned short eos);
  // Stop playing a sound
  static void Stop(int sound);
  // Called to check if a sound has finished
  static void OnNotify(void);

  // Show message about missing Infocom Blorb file, if appropriate
  static void MsgInfocomBlorb(void);

protected:
  int m_sound;
  unsigned short m_eos;
  BYTE* m_data;
  bool m_delete;
  int m_length;
  int m_repeat;

protected:
  static FrotzSound* m_soundEffect;
  static FrotzSound* m_soundMusic;
};

// Class for AIFF sounds
class FrotzSoundAIFF : public FrotzSound, public CDSound
{
public:
  FrotzSoundAIFF(int sound, unsigned short eos, BYTE* data, int length);
  virtual ~FrotzSoundAIFF();

  virtual bool Play(int repeat, int volume);
  virtual bool IsPlaying(void);

  virtual void WriteSampleData(unsigned char* sample, int len);
  virtual bool IsSoundOver(DWORD tick);
  virtual int GetType(void);

  // Details of the sample
  struct SampleData
  {
    unsigned short channels;
    unsigned long samples;
    unsigned short bits;
    double rate;
    BYTE *data;

    unsigned long repeat1;
    unsigned long repeat2;
  };

  // Get details of the sample
  bool GetSampleData(SampleData& data);

protected:
  bool CheckRenderPtr(void);

  // Helper routines for reading AIFF data
  BYTE* FindChunk(LPCTSTR chunk);
  static unsigned short ReadShort(const unsigned char *bytes);
  static unsigned long ReadLong(const unsigned char *bytes);
  static double ReadExtended(const unsigned char *bytes);

protected:
  BYTE* m_renderPtr;
  BYTE* m_renderMin;
  BYTE* m_renderMax;

  // The duration of the sample
  int m_duration;
};

class CSoundFile;

// Class for MOD music
class FrotzSoundMOD : public FrotzSound, public CDSound
{
public:
  FrotzSoundMOD(int sound, unsigned short eos, BYTE* data, int length, bool del);
  virtual ~FrotzSoundMOD();

  virtual bool Play(int repeat, int volume);
  virtual bool IsPlaying(void);

  virtual void WriteSampleData(unsigned char* sample, int len);
  virtual bool IsSoundOver(DWORD tick);
  virtual int GetType(void);

protected:
  // The actual MODPlug player object
  CSoundFile* m_player;

  // The duration of the music
  int m_duration;
};

// Class to build a MOD from a SONG and AIFF samples
class MODBuilder
{
public:
  MODBuilder();
  ~MODBuilder();

  // Create a MOD sound
  FrotzSoundMOD* CreateMODFromSONG(int sound, unsigned short eos, BYTE* data, int length, bb_map_t* map);

  // Write a value into the MOD header
  void WriteHeaderValue(int sample, int offset, unsigned long value);

protected:
  BYTE* m_mod;
  CMap<int,int,FrotzSoundAIFF*,FrotzSoundAIFF*> m_samples;
};

// Class for Ogg Vorbis music
class FrotzSoundOGG : public FrotzSound, public CDSound
{
public:
  FrotzSoundOGG(int sound, unsigned short eos, BYTE* data, int length);
  virtual ~FrotzSoundOGG();

  virtual bool Play(int repeat, int volume);
  virtual bool IsPlaying(void);

  virtual void WriteSampleData(unsigned char* sample, int len);
  virtual bool IsSoundOver(DWORD tick);
  virtual int GetType(void);

protected:
  static size_t VorbisRead(void*, size_t, size_t, void*);
  static int VorbisSeek(void*, ogg_int64_t, int);
  static int VorbisClose(void*);
  static long VorbisTell(void*);

  BYTE* m_renderPtr;
  int m_duration;

  // Descriptor for the audio stream
  OggVorbis_File m_stream;
  // Whether the stream has been opened
  bool m_streamOpen;
};

#ifndef __ISpVoice_FWD_DEFINED__
#define __ISpVoice_FWD_DEFINED__
typedef interface ISpVoice ISpVoice;
#endif

class TextToSpeech
{
public:
  static TextToSpeech& GetSpeechEngine(void);

  bool IsAvailable(void);
  void GetVoices(CStringArray& names, CString& defaultName);

  void Initialize(LPCSTR voice, int speed);
  void Destroy(void);
  void Update(LPCSTR voice, int speed);
  void Speak(LPCWSTR speech);

protected:
  TextToSpeech();
  ~TextToSpeech();

  static TextToSpeech engine;

  enum Available
  {
    NotTested,
    NotAvailable,
    SAPI5
  };

  Available m_available;
  bool m_initialized;

  CComPtr<ISpVoice> m_voice;
};
