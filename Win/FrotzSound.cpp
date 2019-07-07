/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz sound classes
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "FrotzSound.h"
#include "FrotzWnd.h"
#include "sndfile.h"

#include <math.h>
#include <map>

extern "C"
{
#include "blorblow.h"
void end_of_sound(zword routine);
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// The single application instance
extern FrotzApp theApp;

FrotzSound::FrotzSound(int sound, unsigned short eos, BYTE* data, int length, bool del)
{
  m_sound = sound;
  m_eos = eos;
  m_data = data;
  m_delete = del;
  m_length = length;
}

FrotzSound::~FrotzSound()
{
  if (m_delete)
    delete[] m_data;
}

// Get the number of the sound
int FrotzSound::GetSoundNumber(void)
{
  return m_sound;
}

// Get the Z-code to routine at the end
unsigned short FrotzSound::GetEndRoutine(void)
{
  return m_eos;
}

// Get the sound volume in decibels
int FrotzSound::GetDecibelVolume(int volume)
{
  if (volume < 1)
    volume = 1;
  else if (volume > 8)
    volume = 8;

  // Convert volume from the range 1 to 8 into decibels
  double decibels = -10.0 * (log(8.0/(double)volume) / log(2.0));
  if (decibels > 0.0)
    decibels = 0.0;
  else if (decibels < -100.0)
    decibels = -100.0;
  return (int)decibels;
}

// Initialize the sound engine
bool FrotzSound::Init(FrotzWnd* wnd)
{
  if (CDSoundEngine::GetSoundEngine().Initialize(NULL,wnd,WM_SOUND_NOTIFY))
    return true;

  ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(),
    CResString(IDS_FAIL_DIRECTSOUND),CResString(IDS_FROTZ),MB_ICONWARNING|MB_OK);
  return false;
}

// Stop the sound engine
void FrotzSound::ShutDown(void)
{
  Stop(0);
  TextToSpeech::GetSpeechEngine().Destroy();
  CDSoundEngine::GetSoundEngine().Destroy();
}

// Start playing a sound
void FrotzSound::Play(int sound, bb_map_t* map, int repeat, int volume, unsigned short eos)
{
  if (CDSoundEngine::GetSoundEngine().GetStatus() == CDSoundEngine::STATUS_READY)
  {
    bb_result_t result;
    if (bb_load_resource_snd(map,bb_method_Memory,&result,sound,NULL) == bb_err_None)
    {
      BYTE* data = (BYTE*)result.data.ptr;
      int length = result.length;
      unsigned int id = map->chunks[result.chunknum].type;

      // Look for a recognized format
      if (id == bb_make_id('F','O','R','M'))
      {
        delete m_soundEffect;
        m_soundEffect = new FrotzSoundAIFF(sound,eos,data,length);
        m_soundEffect->Play(repeat,volume);
      }
      else if (id == bb_make_id('M','O','D',' '))
      {
        delete m_soundMusic;
        m_soundMusic = new FrotzSoundMOD(sound,eos,data,length,false);
        m_soundMusic->Play(repeat,volume);
      }
      else if (id == bb_make_id('S','O','N','G'))
      {
        delete m_soundMusic;

        MODBuilder build;
        m_soundMusic = build.CreateMODFromSONG(sound,eos,data,length,map);
        if (m_soundMusic)
          m_soundMusic->Play(repeat,volume);
      }
      else if (id == bb_make_id('O','G','G','V'))
      {
        delete m_soundMusic;
        m_soundMusic = new FrotzSoundOGG(sound,eos,data,length);
        m_soundMusic->Play(repeat,volume);
      }
    }
  }
}

// Stop playing a sound
void FrotzSound::Stop(int sound)
{
  if (m_soundEffect != NULL)
  {
    if ((sound == 0) || (m_soundEffect->GetSoundNumber() == sound))
    {
      delete m_soundEffect;
      m_soundEffect = NULL;
    }
  }
  if (m_soundMusic != NULL)
  {
    if ((sound == 0) || (m_soundMusic->GetSoundNumber() == sound))
    {
      delete m_soundMusic;
      m_soundMusic = NULL;
    }
  }
}

// Called to check if a sound has finished
void FrotzSound::OnNotify(void)
{
  if (m_soundEffect != NULL)
  {
    if (m_soundEffect->IsPlaying() == false)
    {
      zword routine = m_soundEffect->GetEndRoutine();
      delete m_soundEffect;
      m_soundEffect = NULL;
      end_of_sound(routine);
    }
  }
  if (m_soundMusic != NULL)
  {
    if (m_soundMusic->IsPlaying() == false)
    {
      zword routine = m_soundMusic->GetEndRoutine();
      delete m_soundMusic;
      m_soundMusic = NULL;
      end_of_sound(routine);
    }
  }
}

enum BlorbTaskState
{
  BlorbTaskNotInit,
  BlorbTaskNeverShow,
  BlorbTaskCanShow,
  BlorbTaskShowed
};

static HRESULT CALLBACK BlorbTaskCallback(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
{
  switch (msg)
  {
  case TDN_BUTTON_CLICKED:
    return S_OK;
  case TDN_HYPERLINK_CLICKED:
    ::ShellExecute(0,NULL,CString(CStringW((LPCWSTR)lParam)),NULL,NULL,SW_SHOWNORMAL);
    ::EndDialog(dlg,IDOK);
    return S_FALSE;
  case TDN_VERIFICATION_CLICKED:
    {
      BlorbTaskState* state = (BlorbTaskState*)lpRefData;
      *state = wParam ? BlorbTaskNeverShow : BlorbTaskCanShow;
      theApp.WriteProfileInt("Files","Show Infocom Blorb Message",(*state == BlorbTaskNeverShow) ? 0 : 1);
    }
    return S_FALSE;
  }
  return S_FALSE;
}

// Show message about missing Infocom Blorb file, if appropriate
void FrotzSound::MsgInfocomBlorb(void)
{
  static BlorbTaskState state = BlorbTaskNotInit;
  static HMODULE comctl32 = 0;

  if (state == BlorbTaskNotInit)
    state = theApp.GetProfileInt("Files","Show Infocom Blorb Message",1) ? BlorbTaskCanShow : BlorbTaskNeverShow;
  if (state != BlorbTaskCanShow)
    return;

  if (comctl32 == 0)
    comctl32 = ::LoadLibrary("comctl32.dll");
  if (comctl32 == 0)
    return;

  typedef HRESULT(__stdcall *TASKDIALOGINDIRECT)(const TASKDIALOGCONFIG*, int*, int*, BOOL*);
  TASKDIALOGINDIRECT taskDialogIndirect = (TASKDIALOGINDIRECT)::GetProcAddress(comctl32,"TaskDialogIndirect");
  if (taskDialogIndirect == NULL)
    return;

  TASKDIALOGCONFIG task = { sizeof(TASKDIALOGCONFIG), 0 };
  task.hwndParent = AfxGetMainWnd()->GetSafeHwnd();
  task.hInstance = AfxGetResourceHandle();
  task.dwFlags = TDF_ENABLE_HYPERLINKS|TDF_ALLOW_DIALOG_CANCELLATION|TDF_POSITION_RELATIVE_TO_WINDOW|TDF_SIZE_TO_CONTENT;
  task.dwCommonButtons = TDCBF_OK_BUTTON;
  task.pszMainIcon = TD_INFORMATION_ICON;
  task.pszWindowTitle = MAKEINTRESOURCEW(IDS_TITLE);
  task.pszMainIcon = MAKEINTRESOURCEW(IDI_INFOCOM);
  task.pszMainInstruction = MAKEINTRESOURCEW(IDS_INFOCOM_BLORB1);
  CStringW contentFormat;
  contentFormat.LoadString(IDS_INFOCOM_BLORB2);
  CStringW content;
  LPCSTR url = "https://ifarchive.org/indexes/if-archive/infocom/media/blorb/";
  content.Format(contentFormat,url,url);
  task.pszContent = content;
  task.pszVerificationText = MAKEINTRESOURCEW(IDS_INFOCOM_BLORB3);
  task.pfCallback = BlorbTaskCallback;
  task.lpCallbackData = (LONG_PTR)&state;
  BOOL dontShowAgain = FALSE;
  (*taskDialogIndirect)(&task,NULL,NULL,&dontShowAgain);
  state = BlorbTaskShowed;
}

FrotzSound* FrotzSound::m_soundEffect = NULL;
FrotzSound* FrotzSound::m_soundMusic = NULL;

/////////////////////////////////////////////////////////////////////////////
// Class for AIFF sounds
/////////////////////////////////////////////////////////////////////////////

FrotzSoundAIFF::FrotzSoundAIFF(int sound, unsigned short eos, BYTE* data, int length) :
  FrotzSound(sound,eos,data,length,false)
{
  m_renderPtr = NULL;
  m_renderMin = NULL;
  m_renderMax = NULL;
  m_duration = 0;
}

FrotzSoundAIFF::~FrotzSoundAIFF()
{
  RemoveFromList();
}

bool FrotzSoundAIFF::Play(int repeat, int volume)
{
  SampleData data;
  if (GetSampleData(data) == false)
    return false;

  // Create a buffer
  if (CreateBuffer(data.channels,(int)data.rate,data.bits) == false)
    return false;

  // Set the duration of the sample
  if (repeat > 0)
    m_duration = (DWORD)ceil((data.samples * repeat * 1000.0) / data.rate);
  else
    m_duration = -1;

  // Set up the current position for rendering wave data
  m_renderPtr = data.data;
  m_renderMin = m_renderPtr;
  m_renderMax = data.data + ((data.bits>>3)*data.samples*data.channels);

  // Fill the buffer with sample data
  m_repeat = (repeat < 0) ? -1 : repeat - 1;
  if (FillBuffer(GetBufferSize()) == false)
    return false;

  // Set the volume for the buffer
  SetBufferVolume(GetDecibelVolume(volume) * 100L);

  // Start the buffer playing
  return PlayBuffer(false);
}

bool FrotzSoundAIFF::IsPlaying(void)
{
  return m_Active;
}

// Write sample data into the supplied PCM sample buffers
void FrotzSoundAIFF::WriteSampleData(unsigned char* sample, int len)
{
  switch (m_Format.wBitsPerSample)
  {
  case 8:
    {
      for (int i = 0; i < len; i++)
      {
        if (CheckRenderPtr())
          *(sample++) = (unsigned char)((*(m_renderPtr++)) ^ 0x80);
        else
          *(sample++) = 0x80;
      }
    }
    break;
  case 16:
    {
      unsigned short* writePtr = (unsigned short*)sample;
      for (int i = 0; i < len; i += 2)
      {
        if (CheckRenderPtr())
        {
          *(writePtr++) = (unsigned short)
            (((*m_renderPtr)<<8) | *(m_renderPtr+1));
          m_renderPtr += 2;
        }
        else
          *(writePtr++) = 0;
      }
    }
    break;
  }
}

// Test that the current point into the AIFF buffer is valid
bool FrotzSoundAIFF::CheckRenderPtr(void)
{
  if (m_renderPtr >= m_renderMax)
  {
    // Fail if this is the end of the last repeat
    if (m_repeat == 0)
      return false;

    // If not looping forever, decrement the repeat counter
    if (m_repeat > 0)
      m_repeat--;

    // Reset the pointer
    m_renderPtr = m_renderMin;
  }
  return true;
}

// Check if the sound has finished playing
bool FrotzSoundAIFF::IsSoundOver(DWORD tick)
{
  if (m_Active == false)
    return true;

  // Check if sound is playing forever
  if (m_duration < 0)
    return false;
  return (tick > m_StartTime + m_duration);
}

// Get a type identifier for the sound
int FrotzSoundAIFF::GetType(void)
{
  return (int)'A';
}

// Get details of the sample
bool FrotzSoundAIFF::GetSampleData(SampleData& data)
{
  if (m_data == NULL)
    return false;

  // Check for AIFF header
  if (strncmp((char*)m_data,"FORM",4) != 0)
    return false;
  if (strncmp((char*)(m_data+8),"AIFF",4) != 0)
    return false;

  // Find the COMM chunk
  BYTE* chunk = FindChunk("COMM");
  if (chunk == NULL)
    return false;

  // Read in details of the sample
  data.channels = ReadShort(chunk);
  data.samples = ReadLong(chunk+2);
  data.bits = ReadShort(chunk+6);
  data.rate = ReadExtended(chunk+8);

  // Find the SSND chunk
  data.data = FindChunk("SSND");
  if (data.data == NULL)
    return false;

  // Set up default repeat information
  data.repeat1 = 0;
  data.repeat2 = 0;

  // Find the INST chunk
  chunk = FindChunk("INST");
  if (chunk != NULL)
  {
    // Look for a sustain loop
    if (ReadShort(chunk+8) != 0)
    {
      unsigned short mark1 = ReadShort(chunk+10);
      unsigned short mark2 = ReadShort(chunk+12);

      // Find the MARK chunk
      chunk = FindChunk("MARK");
      if (chunk != NULL)
      {
        unsigned short markers = ReadShort(chunk);

        BYTE* mark = chunk+2;
        for (int i = 0; i < markers; i++)
        {
          unsigned short id = ReadShort(mark);
          unsigned long pos = ReadLong(mark+2);

          if (id == mark1)
            data.repeat1 = pos;
          else if (id == mark2)
            data.repeat2 = pos;

          unsigned char nameLen = mark[6]+1;
          if ((nameLen % 2) == 1)
            nameLen++;
          mark += nameLen+6;
        }
      }
    }
  }

  return true;
}

// Find an AIFF chunk
BYTE* FrotzSoundAIFF::FindChunk(LPCTSTR chunk)
{
  BYTE* data = m_data+12;
  while (true)
  {
    if (strncmp((char*)data,chunk,4) == 0)
      return data+8;

    // Move to the next chunk
    data += ReadLong(data+4)+8;

    if ((data - m_data) > m_length-8)
      break;
  }
  return NULL;
}

unsigned short FrotzSoundAIFF::ReadShort(const unsigned char *bytes)
{
  return (unsigned short)(
    ((unsigned short)(bytes[0] & 0xFF) << 8) |
    ((unsigned short)(bytes[1] & 0xFF)));
}

unsigned long FrotzSoundAIFF::ReadLong(const unsigned char *bytes)
{
  return (unsigned long)(
    ((unsigned long)(bytes[0] & 0xFF) << 24) |
    ((unsigned long)(bytes[1] & 0xFF) << 16) |
    ((unsigned long)(bytes[2] & 0xFF) << 8) |
    ((unsigned long)(bytes[3] & 0xFF)));
}

/* 
 * Copyright (C) 1988-1991 Apple Computer, Inc.
 * All rights reserved.
 *
 * Machine-independent I/O routines for IEEE floating-point numbers.
 *
 * NaN's and infinities are converted to HUGE_VAL or HUGE, which
 * happens to be infinity on IEEE machines.  Unfortunately, it is
 * impossible to preserve NaN's in a machine-independent way.
 * Infinities are, however, preserved on IEEE machines.
 *
 * These routines have been tested on the following machines:
 *    Apple Macintosh, MPW 3.1 C compiler
 *    Apple Macintosh, THINK C compiler
 *    Silicon Graphics IRIS, MIPS compiler
 *    Cray X/MP and Y/MP
 *    Digital Equipment VAX
 *
 * Implemented by Malcolm Slaney and Ken Turkowski.
 *
 * Malcolm Slaney contributions during 1988-1990 include big- and little-
 * endian file I/O, conversion to and from Motorola's extended 80-bit
 * floating-point format, and conversions to and from IEEE single-
 * precision floating-point format.
 *
 * In 1991, Ken Turkowski implemented the conversions to and from
 * IEEE double-precision format, added more precision to the extended
 * conversions, and accommodated conversions involving +/- infinity,
 * NaN's, and denormalized numbers.
 */
#define UnsignedToFloat(u) (((double)((long)(u - 2147483647L - 1))) + 2147483648.0)
double FrotzSoundAIFF::ReadExtended(const unsigned char *bytes)
{
  double f;
  int expon;
  unsigned long hiMant, loMant;

  expon = ((bytes[0] & 0x7F) << 8) | (bytes[1] & 0xFF);
  hiMant = ReadLong(bytes+2);
  loMant = ReadLong(bytes+6);

  if (expon == 0 && hiMant == 0 && loMant == 0)
    f = 0;
  else
  {
    if (expon == 0x7FFF) /* Infinity or NaN */
      f = -1;
    else
    {
      expon -= 16383;
      f = ldexp(UnsignedToFloat(hiMant),expon -= 31);
      f += ldexp(UnsignedToFloat(loMant),expon -= 32);
    }
  }

  if (bytes[0] & 0x80)
    return -f;
  return f;
}

/////////////////////////////////////////////////////////////////////////////
// Class for MOD music
/////////////////////////////////////////////////////////////////////////////

FrotzSoundMOD::FrotzSoundMOD(int sound, unsigned short eos, BYTE* data, int length, bool del) :
  FrotzSound(sound,eos,data,length,del)
{
  m_player = new CSoundFile;
  m_duration = 0;
}

FrotzSoundMOD::~FrotzSoundMOD()
{
  RemoveFromList();
  delete m_player;
}

bool FrotzSoundMOD::Play(int repeat, int volume)
{
  if (m_data == NULL)
    return false;

  // Only one MOD can be playing at any given time, so stop
  // any currently playing MODs
  CDSoundEngine& engine = CDSoundEngine::GetSoundEngine();
  engine.StopSounds(GetType());

  WAVEFORMATEX& format = engine.GetPrimaryFormat();
  CSoundFile::SetWaveConfig(format.nSamplesPerSec,format.wBitsPerSample,format.nChannels);
  CSoundFile::SetWaveConfigEx(TRUE,FALSE,FALSE,TRUE,TRUE,TRUE,FALSE);

  // Load the song into MODPlug
  if (!m_player->Create(m_data,m_length))
    return false;

  // Set the number of repeats and the duration
  if (repeat > 0)
  {
    m_player->SetRepeatCount(repeat-1);
    m_duration = m_player->GetSongTime() * repeat;
  }
  else
  {
    m_player->SetRepeatCount(-1);
    m_duration = -1;
  }

  // Create a buffer
  if (CreateBuffer(format.nChannels,format.nSamplesPerSec,format.wBitsPerSample) == false)
    return false;

  // Fill the buffer with sample data
  if (FillBuffer(GetBufferSize()) == false)
    return false;

  // Set the volume for the buffer
  SetBufferVolume(GetDecibelVolume(volume) * 100L);

  // Start the buffer playing
  return PlayBuffer(false);
}

bool FrotzSoundMOD::IsPlaying(void)
{
  return m_Active;
}

void FrotzSoundMOD::WriteSampleData(unsigned char* sample, int len)
{
  WAVEFORMATEX& format = CDSoundEngine::GetSoundEngine().GetPrimaryFormat();

  // Render audio
  int bytes = m_player->Read(sample,len) * format.nBlockAlign;

  // Has the MOD been completely rendered?
  if (bytes < len)
  {
    for (int i = bytes; i < len; i++)
    {
      sample[i] = (unsigned char)
        ((format.wBitsPerSample == 8) ? 0x80 : 0x00);
    }
  }
}

// Check if the sound has finished playing
bool FrotzSoundMOD::IsSoundOver(DWORD tick)
{
  if (m_Active == false)
    return true;

  // Check if sound is playing forever
  if (m_duration < 0)
    return false;
  return (tick > m_StartTime + m_duration);
}

// Get a type identifier for the sound
int FrotzSoundMOD::GetType(void)
{
  return (int)'M';
}

/////////////////////////////////////////////////////////////////////////////
// Class to build a MOD from a SONG and AIFF samples
/////////////////////////////////////////////////////////////////////////////

MODBuilder::MODBuilder() : m_mod(NULL)
{
}

MODBuilder::~MODBuilder()
{
  delete[] m_mod;

  int index = 0;
  FrotzSoundAIFF* aiff = NULL;

  // Discard all the loaded samples
  POSITION pos = m_samples.GetStartPosition();
  while (pos != NULL)
  {
    m_samples.GetNextAssoc(pos,index,aiff);
    delete aiff;
  }
}

// Convert a SONG and AIFF samples into a MOD
FrotzSoundMOD* MODBuilder::CreateMODFromSONG(int sound, unsigned short eos, BYTE* data, int length, bb_map_t* map)
{
  const int SAMPLE_NAME = 0;
  const int SAMPLE_LENGTH = 22;
  const int SAMPLE_REPEAT_POS = 26;
  const int SAMPLE_REPEAT_LEN = 28;
  const int SAMPLE_SIZE = 30;

  int modLen = length;

  // Loop over the samples
  for (int i = 0; i < 31; i++)
  {
    BYTE* sample = data + 20 + i*SAMPLE_SIZE;
    if (*(sample + SAMPLE_NAME) != 0)
    {
      // Get the sample resource index
      int index = 0;
      if (sscanf((const char*)(sample + SAMPLE_NAME),"SND%d",&index) != 1)
        return NULL;

      // Attempt to load the sample
      bb_result_t result;
      if (bb_load_resource_snd(map,bb_method_Memory,&result,index,NULL) != bb_err_None)
        return NULL;
      if (map->chunks[result.chunknum].type != bb_make_id('F','O','R','M'))
        return NULL;
      FrotzSoundAIFF* aiff = new FrotzSoundAIFF(index,0,(BYTE*)result.data.ptr,result.length);
      m_samples[index] = aiff;

      // Get details of the sample
      FrotzSoundAIFF::SampleData data;
      if (aiff->GetSampleData(data) == false)
        return NULL;

      // Add to the total size of the MOD
      modLen += data.samples + 2;
    }
  }

  // Create a buffer for the MOD and put the SONG data at the start
  m_mod = new BYTE[modLen];
  ::CopyMemory(m_mod,data,length);

  // Copy the samples into the buffer
  int writePos = length;
  for (int i = 0; i < 31; i++)
  {
    BYTE* sample = data + 20 + i*SAMPLE_SIZE;
    if (*(sample + SAMPLE_NAME) != 0)
    {
      // Get the sample resource index
      int index = 0;
      sscanf((const char*)(sample + SAMPLE_NAME),"SND%d",&index);

      // Get the sample
      FrotzSoundAIFF* aiff = m_samples[index];
      FrotzSoundAIFF::SampleData data;
      aiff->GetSampleData(data);

      // Update the sample length
      WriteHeaderValue(i,SAMPLE_LENGTH,data.samples+2);

      // Update repeat information
      if ((data.repeat1 != 0) && (data.repeat2 != 0))
      {
        WriteHeaderValue(i,SAMPLE_REPEAT_POS,data.repeat1+2);
        WriteHeaderValue(i,SAMPLE_REPEAT_LEN,data.repeat2-data.repeat1);
      }

      // Copy into the buffer, converting to 8-bit mono
      *(m_mod+(writePos++)) = 0;
      *(m_mod+(writePos++)) = 0;
      for (unsigned long j = 0; j < data.samples; j++)
        *(m_mod+writePos+j) = *(data.data+(j*data.channels*(data.bits>>3)));
      writePos += data.samples;
    }
  }

  FrotzSoundMOD* mod = new FrotzSoundMOD(sound,eos,m_mod,modLen,true);
  m_mod = NULL;
  return mod;
}

// Write a value into the MOD header
void MODBuilder::WriteHeaderValue(int sample, int offset, unsigned long value)
{
  const int SAMPLE_SIZE = 30;
  unsigned short val = (unsigned short)(value>>1);

  BYTE* header = m_mod + 20 + (sample*SAMPLE_SIZE);
  header[offset] = (BYTE)(val >> 8);
  header[offset+1] = (BYTE)(val - (header[offset] << 8));
}

/////////////////////////////////////////////////////////////////////////////
// Class for Ogg Vorbis music
/////////////////////////////////////////////////////////////////////////////

FrotzSoundOGG::FrotzSoundOGG(int sound, unsigned short eos, BYTE* data, int length) :
  FrotzSound(sound,eos,data,length,false)
{
  m_renderPtr = NULL;
  m_duration = 0;
}

FrotzSoundOGG::~FrotzSoundOGG()
{
  RemoveFromList();
  if (m_streamOpen)
    ov_clear(&m_stream);
}

bool FrotzSoundOGG::Play(int repeat, int volume)
{
  m_renderPtr = m_data;
  if (m_renderPtr == NULL)
    return false;

  // Open the stream
  ov_callbacks VorbisCBs;
  VorbisCBs.read_func = VorbisRead;
  VorbisCBs.close_func = VorbisClose;
  VorbisCBs.seek_func = VorbisSeek;
  VorbisCBs.tell_func = VorbisTell;
  if (ov_open_callbacks(this,&m_stream,NULL,0,VorbisCBs) < 0)
    return false;
  m_streamOpen = true;
  vorbis_info* info = ov_info(&m_stream,-1);

  // Create a buffer
  if (CreateBuffer(info->channels,info->rate,16) == false)
    return false;

  // Set the duration of the sample
  if (repeat > 0)
    m_duration = (DWORD)ceil(ov_time_total(&m_stream,-1) * repeat * 1000.0);
  else
    m_duration = -1;

  // Fill the buffer with sample data
  m_repeat = (repeat < 0) ? -1 : repeat - 1;
  if (FillBuffer(GetBufferSize()) == false)
    return false;

  // Set the volume for the buffer
  SetBufferVolume(GetDecibelVolume(volume) * 100L);

  // Start the buffer playing
  return PlayBuffer(false);
}

bool FrotzSoundOGG::IsPlaying(void)
{
  return m_Active;
}

// Write sample data into the supplied PCM sample buffers
void FrotzSoundOGG::WriteSampleData(unsigned char* sample, int len)
{
  int stream, current = 0;
  while (current < len)
  {
    long read =
      ov_read(&m_stream,(char*)(sample+current),(len-current),0,2,1,&stream);
    if (read > 0)
      current += read;
    else
    {
      if (m_repeat > 0)
      {
        ov_pcm_seek(&m_stream,0);
        m_repeat--;
      }
      else if (m_repeat == -1)
        ov_pcm_seek(&m_stream,0);
      else
      {
        while (current < len)
          sample[current++] = 0;
      }
    }
  }
}

// Check if the sound has finished playing
bool FrotzSoundOGG::IsSoundOver(DWORD tick)
{
  if (m_Active == false)
    return true;

  // Check if sound is playing forever
  if (m_duration < 0)
    return false;
  return (tick > m_StartTime + m_duration);
}

// Get a type identifier for the sound
int FrotzSoundOGG::GetType(void)
{
  return (int)'O';
}

size_t FrotzSoundOGG::VorbisRead(void* ptr, size_t byteSize, size_t sizeToRead, void* src)
{
  FrotzSoundOGG* sound = (FrotzSoundOGG*)src;

  int read = sizeToRead;
  int maxRead = (sound->m_data + sound->m_length - sound->m_renderPtr) / byteSize;
  if (read > maxRead)
    read = maxRead;

  memcpy(ptr,sound->m_renderPtr,read * byteSize);
  sound->m_renderPtr += read * byteSize;
  return read;
}

int FrotzSoundOGG::VorbisSeek(void* src, ogg_int64_t offset, int whence)
{
  FrotzSoundOGG* sound = (FrotzSoundOGG*)src;

  switch (whence)
  {
  case SEEK_SET:
    sound->m_renderPtr = sound->m_data + offset;
    return 0;
  case SEEK_CUR:
    sound->m_renderPtr += offset;
    return 0;
  case SEEK_END:
    sound->m_renderPtr = sound->m_data + sound->m_length;
    return 0;
  }
  return -1;
}

int FrotzSoundOGG::VorbisClose(void*)
{
  return 0;
}

long FrotzSoundOGG::VorbisTell(void* src)
{
  FrotzSoundOGG* sound = (FrotzSoundOGG*)src;

  return sound->m_renderPtr - sound->m_data;
}

/////////////////////////////////////////////////////////////////////
// Interface to SAPI 5
/////////////////////////////////////////////////////////////////////

#ifndef __ISpVoice_INTERFACE_DEFINED__

typedef enum SPVPRIORITY {} SPVPRIORITY;
typedef enum SPEVENTENUM {} SPEVENTENUM;
typedef enum SPDATAKEYLOCATION {} SPDATAKEYLOCATION;

typedef enum SPEAKFLAGS {
  SPF_ASYNC = (1L<<0)
} SPEAKFLAGS;

MIDL_INTERFACE("5EFF4AEF-8487-11D2-961C-00C04F8EE628")
ISpNotifySource : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetNotifySink(IUnknown*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetNotifyWindowMessage(HWND, UINT, WPARAM, LPARAM) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetNotifyCallbackFunction(VOID*, WPARAM, LPARAM) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetNotifyCallbackInterface(IUnknown*, WPARAM, LPARAM) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetNotifyWin32Event(VOID) = 0;
  virtual HRESULT STDMETHODCALLTYPE WaitForNotifyEvent(DWORD) = 0;
  virtual HANDLE STDMETHODCALLTYPE GetNotifyEventHandle(VOID) = 0;
};

MIDL_INTERFACE("BE7A9CCE-5F9E-11D2-960F-00C04F8EE628")
ISpEventSource : public ISpNotifySource
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetInterest(ULONGLONG, ULONGLONG) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetEvents(ULONG, VOID*, ULONG*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetInfo(VOID*) = 0;
};

MIDL_INTERFACE("6C44DF74-72B9-4992-A1EC-EF996E0422D4")
ISpVoice : public ISpEventSource
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetOutput(IUnknown*, BOOL) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetOutputObjectToken(IUnknown**) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetOutputStream(IUnknown**) = 0;
  virtual HRESULT STDMETHODCALLTYPE Pause(VOID) = 0;
  virtual HRESULT STDMETHODCALLTYPE Resume(VOID) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetVoice(IUnknown*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetVoice(IUnknown**) = 0;
  virtual HRESULT STDMETHODCALLTYPE Speak(LPCWSTR, DWORD, ULONG*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SpeakStream(IStream*, DWORD, ULONG*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetStatus(VOID*, LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE Skip(LPCWSTR, long, ULONG*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetPriority(SPVPRIORITY) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetPriority(SPVPRIORITY*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetAlertBoundary(SPEVENTENUM) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetAlertBoundary(SPEVENTENUM*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetRate(long) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetRate(long*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetVolume(USHORT) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetVolume(USHORT*) = 0;
  virtual HRESULT STDMETHODCALLTYPE WaitUntilDone(ULONG) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetSyncSpeakTimeout(ULONG) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetSyncSpeakTimeout(ULONG*) = 0;
  virtual HANDLE STDMETHODCALLTYPE SpeakCompleteEvent(VOID) = 0;
  virtual HRESULT STDMETHODCALLTYPE IsUISupported(LPCWSTR, VOID*, ULONG, BOOL*) = 0;
  virtual  HRESULT STDMETHODCALLTYPE DisplayUI(HWND, LPCWSTR, LPCWSTR, VOID*, ULONG) = 0;
};

MIDL_INTERFACE("14056581-E16C-11D2-BB90-00C04F8EE6C0")
ISpDataKey : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetData(LPCWSTR, ULONG, const BYTE*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetData(LPCWSTR, ULONG*, BYTE*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetStringValue(LPCWSTR, LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetStringValue(LPCWSTR, LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetDWORD(LPCWSTR, DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDWORD(LPCWSTR, DWORD*) = 0;
  virtual HRESULT STDMETHODCALLTYPE OpenKey(LPCWSTR, ISpDataKey**) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateKey(LPCWSTR, ISpDataKey**) = 0;
  virtual HRESULT STDMETHODCALLTYPE DeleteKey(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE DeleteValue(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE EnumKeys(ULONG, LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE EnumValues(ULONG, LPWSTR*) = 0;
};

typedef interface ISpObjectTokenCategory ISpObjectTokenCategory;
MIDL_INTERFACE("14056589-E16C-11D2-BB90-00C04F8EE6C0")
ISpObjectToken : public ISpDataKey
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetId(LPCWSTR, LPCWSTR, BOOL) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetId(LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCategory(ISpObjectTokenCategory**) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown*, DWORD, REFIID, void**) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetStorageFileName(REFCLSID, LPCWSTR, LPCWSTR, ULONG, LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE RemoveStorageFileName(REFCLSID clsidCaller, LPCWSTR, BOOL) = 0;
  virtual HRESULT STDMETHODCALLTYPE Remove(const CLSID*) = 0;
  virtual HRESULT STDMETHODCALLTYPE IsUISupported(LPCWSTR, void*, ULONG, IUnknown*, BOOL*) = 0;
  virtual HRESULT STDMETHODCALLTYPE DisplayUI(HWND, LPCWSTR, LPCWSTR, void*, ULONG, IUnknown*) = 0;
  virtual HRESULT STDMETHODCALLTYPE MatchesAttributes(LPCWSTR, BOOL*) = 0;
};

MIDL_INTERFACE("06B64F9E-7FDA-11D2-B4F2-00C04F797396")
IEnumSpObjectTokens : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE Next(ULONG, ISpObjectToken**, ULONG*) = 0;
  virtual HRESULT STDMETHODCALLTYPE Skip(ULONG) = 0;
  virtual HRESULT STDMETHODCALLTYPE Reset(void) = 0;
  virtual HRESULT STDMETHODCALLTYPE Clone(IEnumSpObjectTokens**) = 0;
  virtual HRESULT STDMETHODCALLTYPE Item(ULONG, ISpObjectToken**) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCount(ULONG*) = 0;
};

MIDL_INTERFACE("2D3D3845-39AF-4850-BBF9-40B49780011D")
ISpObjectTokenCategory : public ISpDataKey
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetId(LPCWSTR, BOOL) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetId(LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDataKey(SPDATAKEYLOCATION, ISpDataKey**) = 0;
  virtual HRESULT STDMETHODCALLTYPE EnumTokens(LPCWSTR, LPCWSTR, IEnumSpObjectTokens**) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetDefaultTokenId(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDefaultTokenId(LPWSTR*) = 0;
};

class DECLSPEC_UUID("96749377-3391-11D2-9EE3-00C04F797396") SpVoice;
class DECLSPEC_UUID("A910187F-0C7A-45AC-92CC-59EDAFB77B53") SpObjectTokenCategory;

#endif // __ISpVoice_INTERFACE_DEFINED__

/////////////////////////////////////////////////////////////////////
// Interface to the speech engine
/////////////////////////////////////////////////////////////////////

namespace {

// Get details of all installed voices
void GetAllVoices(std::map<CString,CComPtr<ISpObjectToken> >& voices, CString& defaultVoice)
{
  CComPtr<ISpObjectTokenCategory> category;
  if (SUCCEEDED(category.CoCreateInstance(__uuidof(SpObjectTokenCategory))))
  {
    if (SUCCEEDED(category->SetId(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices",FALSE)))
    {
      CStringW defaultId;
      {
        LPWSTR id = NULL;
        if (SUCCEEDED(category->GetDefaultTokenId(&id)))
        {
          defaultId = id;
          ::CoTaskMemFree(id);
        }
      }

      CComPtr<IEnumSpObjectTokens> tokens;
      if (SUCCEEDED(category->EnumTokens(NULL,NULL,&tokens)))
      {
        ULONG count = 0;
        if (SUCCEEDED(tokens->GetCount(&count)))
        {
          for (ULONG i = 0; i < count; i++)
          {
            CComPtr<ISpObjectToken> token;
            if (SUCCEEDED(tokens->Item(i,&token)))
            {
              CStringW tokenId;
              {
                LPWSTR id = NULL;
                if (SUCCEEDED(token->GetId(&id)))
                {
                  tokenId = id;
                  ::CoTaskMemFree(id);
                }
              }

              LPWSTR name = NULL;
              if (SUCCEEDED(token->GetStringValue(NULL,&name)))
              {
                CString tokenName(name);
                voices[tokenName] = token;
                if (defaultId.Compare(tokenId) == 0)
                  defaultVoice = tokenName;
                ::CoTaskMemFree(name);
              }
            }
          }
        }
      }
    }
  }
}

} // unnamed namespace

// The only instance of the speech object
TextToSpeech TextToSpeech::engine;

// Get the only instance of the speech engine
TextToSpeech& TextToSpeech::GetSpeechEngine(void)
{
  return engine;
}

// Constructor
TextToSpeech::TextToSpeech()
{
  m_available = NotTested;
  m_initialized = false;
}

// Destructor
TextToSpeech::~TextToSpeech()
{
  Destroy();
}

// Check if the engine is available
bool TextToSpeech::IsAvailable(void)
{
  if (m_available == NotTested)
  {
    m_available = NotAvailable;

    CComPtr<ISpVoice> voice;
    if (SUCCEEDED(voice.CoCreateInstance(__uuidof(SpVoice))))
      m_available = SAPI5;
  }

  if (m_available == SAPI5)
    return true;
  return false;
}

// Enumerate the voices that can be used by the speech engine
void TextToSpeech::GetVoices(CStringArray& names, CString& defaultName)
{
  if (IsAvailable())
  {
    std::map<CString,CComPtr<ISpObjectToken> > voices;
    GetAllVoices(voices,defaultName);

    for (std::map<CString,CComPtr<ISpObjectToken> >::iterator it = voices.begin(); it != voices.end(); ++it)
      names.Add(it->first);
  }
}

// Initialize the speech engine
void TextToSpeech::Initialize(LPCSTR voice, int speed)
{
  if (m_initialized)
    return;
  m_initialized = true;

  // Create an object to interface with the speech engine
  if ((m_available == SAPI5) && (m_voice == NULL))
  {
    m_voice.CoCreateInstance(__uuidof(SpVoice));
    Update(voice,speed);
  }
}

// Close the speech engine
void TextToSpeech::Destroy(void)
{
  if (m_voice != NULL)
    m_voice.Release();
  m_initialized = false;
}

// Update the speech engine settings
void TextToSpeech::Update(LPCSTR voice, int speed)
{
  if (m_voice != NULL)
  {
    m_voice->SetRate(speed);

    std::map<CString,CComPtr<ISpObjectToken> > voices;
    CString defaultVoice;
    GetAllVoices(voices,defaultVoice);
    std::map<CString,CComPtr<ISpObjectToken> >::iterator it = voices.find(voice);
    if (it != voices.end())
      m_voice->SetVoice(it->second);
    else
    {
      it = voices.find(defaultVoice);
      if (it != voices.end())
        m_voice->SetVoice(it->second);
    }
  }
}

// Speak some text
void TextToSpeech::Speak(LPCWSTR speech)
{
  if (m_voice != NULL)
    m_voice->Speak(speech,SPF_ASYNC,NULL);
}
