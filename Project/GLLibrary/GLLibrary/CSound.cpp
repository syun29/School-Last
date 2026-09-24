#include "CSound.h"
#include "CCamera.h"
#include <assert.h>
#include <sndfile.h>
#define MINIMP3_IMPLEMENTATION
#include <minimp3_ex.h>

#pragma comment(lib, "OpenAL32.lib")
#pragma comment(lib, "alsoft.common.lib")
#pragma comment(lib, "alsoft.excommon.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "avrt.lib")
#pragma comment(lib, "sndfile.lib")

/* Effect object functions */
static LPALGENEFFECTS alGenEffects;
static LPALDELETEEFFECTS alDeleteEffects;
static LPALISEFFECT alIsEffect;
static LPALEFFECTI alEffecti;
static LPALEFFECTIV alEffectiv;
static LPALEFFECTF alEffectf;
static LPALEFFECTFV alEffectfv;
static LPALGETEFFECTI alGetEffecti;
static LPALGETEFFECTIV alGetEffectiv;
static LPALGETEFFECTF alGetEffectf;
static LPALGETEFFECTFV alGetEffectfv;

/* Auxiliary Effect Slot object functions */
static LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
static LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
static LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
static LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
static LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
static LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
static LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
static LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
static LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
static LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
static LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;
static void GetALFunctionAdress() {

	/* Effect object functions */
	alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
	alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
	alIsEffect = (LPALISEFFECT)alGetProcAddress("alIsEffect");
	alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
	alEffectiv = (LPALEFFECTIV)alGetProcAddress("alEffectiv");
	alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
	alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
	alGetEffecti = (LPALGETEFFECTI)alGetProcAddress("alGetEffecti");
	alGetEffectiv = (LPALGETEFFECTIV)alGetProcAddress("alGetEffectiv");
	alGetEffectf = (LPALGETEFFECTF)alGetProcAddress("alGetEffectf");
	alGetEffectfv = (LPALGETEFFECTFV)alGetProcAddress("alGetEffectfv");

	/* Auxiliary Effect Slot object functions */
	alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
	alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
	alIsAuxiliaryEffectSlot = (LPALISAUXILIARYEFFECTSLOT)alGetProcAddress("alIsAuxiliaryEffectSlot");
	alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
	alAuxiliaryEffectSlotiv = (LPALAUXILIARYEFFECTSLOTIV)alGetProcAddress("alAuxiliaryEffectSlotiv");
	alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
	alAuxiliaryEffectSlotfv = (LPALAUXILIARYEFFECTSLOTFV)alGetProcAddress("alAuxiliaryEffectSlotfv");
	alGetAuxiliaryEffectSloti = (LPALGETAUXILIARYEFFECTSLOTI)alGetProcAddress("alGetAuxiliaryEffectSloti");
	alGetAuxiliaryEffectSlotiv = (LPALGETAUXILIARYEFFECTSLOTIV)alGetProcAddress("alGetAuxiliaryEffectSlotiv");
	alGetAuxiliaryEffectSlotf = (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");
	alGetAuxiliaryEffectSlotfv = (LPALGETAUXILIARYEFFECTSLOTFV)alGetProcAddress("alGetAuxiliaryEffectSlotfv");
}
// エラーをチェックするヘルパー関数
void checkALError(const std::string& errorMessage) {
	ALenum error = alGetError();
	if (error != AL_NO_ERROR) {
		printf("%s:%s\n", errorMessage.c_str(), alGetString(error));
		exit(EXIT_FAILURE);
	}
}


/* LoadEffect loads the given reverb properties into a new OpenAL effect
 * object, and returns the new effect ID. */
static ALuint LoadEffect(ALuint effect, const EFXEAXREVERBPROPERTIES* reverb)
{
	ALenum err;

	/* Clear error state. */
	alGetError();
	alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
	err = alGetError();
	if (err == AL_NO_ERROR)
	{
		printf("Using EAX Reverb\n");

		alEffectf(effect, AL_EAXREVERB_DENSITY, reverb->flDensity);
		alEffectf(effect, AL_EAXREVERB_DIFFUSION, reverb->flDiffusion);
		alEffectf(effect, AL_EAXREVERB_GAIN, reverb->flGain);
		alEffectf(effect, AL_EAXREVERB_GAINHF, reverb->flGainHF);
		alEffectf(effect, AL_EAXREVERB_GAINLF, reverb->flGainLF);
		alEffectf(effect, AL_EAXREVERB_DECAY_TIME, reverb->flDecayTime);
		alEffectf(effect, AL_EAXREVERB_DECAY_HFRATIO, reverb->flDecayHFRatio);
		alEffectf(effect, AL_EAXREVERB_DECAY_LFRATIO, reverb->flDecayLFRatio);
		alEffectf(effect, AL_EAXREVERB_REFLECTIONS_GAIN, reverb->flReflectionsGain);
		alEffectf(effect, AL_EAXREVERB_REFLECTIONS_DELAY, reverb->flReflectionsDelay);
		alEffectfv(effect, AL_EAXREVERB_REFLECTIONS_PAN, reverb->flReflectionsPan);
		alEffectf(effect, AL_EAXREVERB_LATE_REVERB_GAIN, reverb->flLateReverbGain);
		alEffectf(effect, AL_EAXREVERB_LATE_REVERB_DELAY, reverb->flLateReverbDelay);
		alEffectfv(effect, AL_EAXREVERB_LATE_REVERB_PAN, reverb->flLateReverbPan);
		alEffectf(effect, AL_EAXREVERB_ECHO_TIME, reverb->flEchoTime);
		alEffectf(effect, AL_EAXREVERB_ECHO_DEPTH, reverb->flEchoDepth);
		alEffectf(effect, AL_EAXREVERB_MODULATION_TIME, reverb->flModulationTime);
		alEffectf(effect, AL_EAXREVERB_MODULATION_DEPTH, reverb->flModulationDepth);
		alEffectf(effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, reverb->flAirAbsorptionGainHF);
		alEffectf(effect, AL_EAXREVERB_HFREFERENCE, reverb->flHFReference);
		alEffectf(effect, AL_EAXREVERB_LFREFERENCE, reverb->flLFReference);
		alEffectf(effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, reverb->flRoomRolloffFactor);
		alEffecti(effect, AL_EAXREVERB_DECAY_HFLIMIT, reverb->iDecayHFLimit);
	}
	else
	{
		printf("Using Standard Reverb\n");

		/* No EAX Reverb. Set the standard reverb effect type then load the
		 * available reverb properties.
		 */
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);

		alEffectf(effect, AL_REVERB_DENSITY, reverb->flDensity);
		alEffectf(effect, AL_REVERB_DIFFUSION, reverb->flDiffusion);
		alEffectf(effect, AL_REVERB_GAIN, reverb->flGain);
		alEffectf(effect, AL_REVERB_GAINHF, reverb->flGainHF);
		alEffectf(effect, AL_REVERB_DECAY_TIME, reverb->flDecayTime);
		alEffectf(effect, AL_REVERB_DECAY_HFRATIO, reverb->flDecayHFRatio);
		alEffectf(effect, AL_REVERB_REFLECTIONS_GAIN, reverb->flReflectionsGain);
		alEffectf(effect, AL_REVERB_REFLECTIONS_DELAY, reverb->flReflectionsDelay);
		alEffectf(effect, AL_REVERB_LATE_REVERB_GAIN, reverb->flLateReverbGain);
		alEffectf(effect, AL_REVERB_LATE_REVERB_DELAY, reverb->flLateReverbDelay);
		alEffectf(effect, AL_REVERB_AIR_ABSORPTION_GAINHF, reverb->flAirAbsorptionGainHF);
		alEffectf(effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, reverb->flRoomRolloffFactor);
		alEffecti(effect, AL_REVERB_DECAY_HFLIMIT, reverb->iDecayHFLimit);
	}

	/* Check if an error occurred, and clean up if so. */
	err = alGetError();
	if (err != AL_NO_ERROR)
	{
		fprintf(stderr, "OpenAL error: %s\n", alGetString(err));
		if (alIsEffect(effect))
			alDeleteEffects(1, &effect);
		return 0;
	}

	return effect;
}

CSoundBase::CSoundBase(): m_source(nullptr), /*_effect(nullptr), m_effectSlot(nullptr),*/m_buffer(0), m_current(0), m_layer_max(0), m_channels(0), m_sample_rate(0) {

}
static ALuint LoadMp3File(const char* filename) {
	// minimp3の初期化
	mp3dec_ex_t mp3;
	if (mp3dec_ex_open(&mp3, filename, MP3D_SEEK_TO_SAMPLE) != 0) {
		printf("Failed to open MP3 file:%s\n", filename);
		return 0;
	}

	// PCMフォーマットを取得
	int channels = mp3.info.channels;
	int sample_rate = mp3.info.hz;

	// OpenALのフォーマットを決定
	ALenum format;
	if (channels == 1) {
		format = AL_FORMAT_MONO16;
	}
	else if (channels == 2) {
		format = AL_FORMAT_STEREO16;
	}
	else {
		printf("Unsupported number of channels:%d\n", channels);
		mp3dec_ex_close(&mp3);
		return 0;
	}
	// PCMデータのデコード
	short* audioData=new short[mp3.samples];
	size_t samplesDecoded = mp3dec_ex_read(&mp3, audioData, mp3.samples);
	if (samplesDecoded != mp3.samples) {
		printf("Failed to decode all samples.");
		mp3dec_ex_close(&mp3);
		return 0;
	}
	// OpenALバッファにデータをロード
	ALuint buffer;
	alGenBuffers(1, &buffer);
	checkALError("Failed to generate buffer");
	alBufferData(buffer, format, audioData, mp3.samples * sizeof(short), sample_rate);
	checkALError("Loading buffer data");

	return buffer;

}
static ALuint LoadWaveFile(const char* filename) {
	
	// WAVファイルの読み込み
	SF_INFO sfInfo;
	SNDFILE* sndFile = sf_open(filename, SFM_READ, &sfInfo);
	if (!sndFile) {
		printf("Failed to open WAV file:%s\n", filename);
		return 0;
	}

	if (sfInfo.channels > 2) {
		printf("Only mono or stereo files are supported%s\n", filename);
		sf_close(sndFile);
		return 0;
	}

	ALenum format = (sfInfo.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	short* data = new short[sfInfo.frames * sfInfo.channels];
	sf_read_short(sndFile, data, sfInfo.frames * sfInfo.channels);
	sf_close(sndFile);

	// OpenALバッファにデータをロード
	ALuint buffer;
	alGenBuffers(1, &buffer);
	checkALError("Failed to generate buffer");
	alBufferData(buffer, format, data, sfInfo.frames * sfInfo.channels * sizeof(short), sfInfo.samplerate);
	checkALError("Failed to load data into buffer");

	return buffer;

}
void CSoundBase::Release() {
	Stop();
	// ソースの破棄
	if (m_source) {
		alDeleteSources(m_layer_max, m_source);
		if (m_source) delete m_source;
		m_source = nullptr;
	}
	/*
	if (m_effect) {
		alDeleteEffects(m_layer_max, m_effect);
		if (m_effect) delete m_effect;
		m_effect = nullptr;
	}
	if (m_effectSlot) {
		alDeleteAuxiliaryEffectSlots(m_layer_max, m_effectSlot);
		if (m_effectSlot) delete m_effectSlot;
		m_effectSlot = nullptr;
	}
	*/
	if (m_buffer) {
		alDeleteBuffers(1, &m_buffer);
		m_buffer = 0;
	}

}
CSoundBase::~CSoundBase() {
	Release();
}
bool CSoundBase::Load(const char* file, int layer, bool is3D)
{
	if (m_buffer) return true;

	const char* c = strrchr(file, '.') + 1;

	if (strcmp(c, "mp3") == 0 || strcmp(c, "MP3") == 0)
		m_buffer = LoadMp3File(file);
	else
		m_buffer = LoadWaveFile(file);
	if (!m_buffer) {
		MessageBox(GL::hWnd, (std::string(file) + "がありません").c_str(), "", MB_OK);
		return false;
	}
	m_layer_max = layer;
	m_source = new ALuint[m_layer_max];
	//m_effect = new ALuint[m_layer_max];
	//m_effectSlot = new ALuint[m_layer_max];
	//alGenEffects(m_layer_max, m_effect);
	//checkALError("Failed to generate effect");

	//alGenAuxiliaryEffectSlots(m_layer_max, m_effectSlot);
	//checkALError("Failed to generate auxiliary effect slot");

	/*for (int i = 0; i < m_layer_max; i++) {
		EFXEAXREVERBPROPERTIES reverb = EFX_REVERB_PRESET_GENERIC;
		LoadEffect(m_effect[i], &reverb);
		// エフェクトスロットにエフェクトを適用
		alAuxiliaryEffectSloti(m_effectSlot[i], AL_EFFECTSLOT_EFFECT, m_effect[i]);
	}
	checkALError("Failed to attach effect to effect slot");
	*/
	alGenSources(m_layer_max, m_source);

	m_param.is3D = is3D;


	return true;
}
bool CSoundBase::Create(const void* data, ALsizei size, ALenum format, ALsizei freq)
{

	m_param.is3D = false;
	m_layer_max = 1;
	m_source = new ALuint[m_layer_max];
	/*
	m_effect = new ALuint[m_layer_max];
	m_effectSlot = new ALuint[m_layer_max];
	alGenEffects(m_layer_max, m_effect);
	checkALError("Failed to generate effect");

	alGenAuxiliaryEffectSlots(m_layer_max, m_effectSlot);
	checkALError("Failed to generate auxiliary effect slot");

	for (int i = 0; i < m_layer_max; i++) {
		EFXEAXREVERBPROPERTIES reverb = EFX_REVERB_PRESET_GENERIC;
		LoadEffect(m_effect[i],	&reverb);
		// エフェクトスロットにエフェクトを適用
		alAuxiliaryEffectSloti(m_effectSlot[i], AL_EFFECTSLOT_EFFECT, m_effect[i]);
	}
	checkALError("Failed to attach effect to effect slot");
	*/
	alGenBuffers(1, &m_buffer); //曲データ１つにつきバッファ１つ．
	alGenSources(m_layer_max, m_source); //空間に配置する数の分生成する

	alBufferData(m_buffer, format, data, size, freq); // bufferにデータを登録

	// source と buffer の接続
	alSourcei(m_source[0], AL_BUFFER, m_buffer);

	return false;
}

int CSoundBase::Play(bool boLoop, bool effect ,const EFXEAXREVERBPROPERTIES& reverb) {
	if (m_layer_max == 0) return -1;

	const ALuint& s = m_source[m_current];
	//const ALuint& e = m_effect[m_current];
	//const ALuint& es = m_effectSlot[m_current];
	Stop(m_current);
	alSourcei(s, AL_LOOPING, boLoop ? AL_TRUE : AL_FALSE);
	alSourcei(s, AL_BUFFER, m_buffer);
	alSourcef(s, AL_PITCH, m_param.pitch);
	alSourcef(s, AL_GAIN, m_param.vol);
	float position[3] = { m_param.pan, 0.0f, 0.0f };
	alSourcei(s, AL_SOURCE_RELATIVE, AL_TRUE);
	alSourcefv(s, AL_POSITION, position);
	alSourcef(s, AL_MAX_DISTANCE, 1.0f);

	// エフェクトスロットを音源にリンク
	/*if (effect) {
		LoadEffect(e, &reverb);
		alSource3i(s, AL_AUXILIARY_SEND_FILTER, es, 0, AL_FILTER_NULL);
	}
	else {
		alSource3i(s, AL_AUXILIARY_SEND_FILTER, 0, 0, AL_FILTER_NULL);
	}
	checkALError("Failed to link effect slot to source");
	*/

	alSourcePlay(s);
	int c = m_current;
	m_current = (m_current + 1) % m_layer_max;
	return c;
}

void CSoundBase::Resume(bool boLoop, int layer) {
	if (m_layer_max == 0) return;

	alSourcePlay(m_source[layer]);

}
int CSoundBase::Play3D(const CVector3D& pos, const CVector3D& velocity,bool boLoop, bool effect, const EFXEAXREVERBPROPERTIES& reverb) {
	if (m_layer_max == 0) return -1;

	const ALuint &s = m_source[m_current];
	//const ALuint& e = m_effect[m_current];
	//const ALuint& es = m_effectSlot[m_current];
	Stop(m_current);
	alSourcei(s, AL_LOOPING, boLoop ? AL_TRUE : AL_FALSE);
	alSourcei(s, AL_BUFFER, m_buffer);
	alSourcef(s, AL_PITCH, m_param.pitch);
	alSourcef(s, AL_GAIN, m_param.vol);
	alSourcef(s, AL_ROLLOFF_FACTOR, m_param.rolloff_factor);
	alSourcef(s, AL_REFERENCE_DISTANCE, m_param.reference_distance);
	alSourcei(s, AL_SOURCE_RELATIVE, AL_FALSE);
	alSourcefv(s, AL_POSITION, pos.v);
	alSourcefv(s, AL_VELOCITY, velocity.v);
	alSourcef(s, AL_MAX_DISTANCE, m_param.dist);


	// エフェクトスロットを音源にリンク
	/*if (effect) {
		LoadEffect(e, &reverb);
		alSource3i(s, AL_AUXILIARY_SEND_FILTER, es, 0, AL_FILTER_NULL);
		checkALError("Failed to link effect slot to source");
	}
	else {
		alSource3i(s, AL_AUXILIARY_SEND_FILTER, 0, 0, AL_FILTER_NULL);
	}
	*/
	alSourcePlay(s);
	int c = m_current;
	m_current = (m_current + 1) % m_layer_max;
	return c;
}

void CSoundBase::SetPos(CVector3D pos, int layer) {
	if (m_layer_max == 0) return;
	alSourcefv(m_source[layer], AL_POSITION, pos.v);
}
void CSoundBase::Stop(int layer){
	if (m_layer_max == 0) return;
	if (layer >= 0) {
		alSourceStop(m_source[layer]);
	} else {
		for (int i=0;i<m_layer_max;i++)
			alSourceStop(m_source[i]);
	}
}
void CSoundBase::Pause(int layer){
	if (m_layer_max == 0) return;
	if (layer >= 0) {
		alSourcePause(m_source[layer]);
	} else {
		for (int i = 0; i<m_layer_max; i++)
			alSourcePause(m_source[i]);
	}

}
void CSoundBase::Pitch(float pitch, int layer) {
	if (m_layer_max == 0) return;
	if (layer == -1) {
		m_param.pitch = pitch;
	}
	else {
		alSourcef(m_source[layer], AL_PITCH, pitch);
	}
}

void CSoundBase::Volume(float volumes, int layer){
	if (m_layer_max == 0) return;
	if (layer == -1) {
		m_param.vol = volumes;
	} else {
		alSourcef(m_source[layer], AL_GAIN, volumes);
	}
}

void CSoundBase::Pan(float pan, int layer) {
	if (m_layer_max == 0) return;
	if (layer == -1) {
		m_param.pan = pan;
	} else {
		float position[3] = { pan, 0.0f, 0.0f };
		alSourcei(m_source[layer], AL_SOURCE_RELATIVE, AL_TRUE);
		alSourcefv(m_source[layer], AL_POSITION, position);
	}


}

void CSoundBase::SetDistance(float max_distance,float reference_distance,float rolloff_factor, int layer){
	if (m_layer_max == 0) return;
	if (layer == -1) {
		m_param.rolloff_factor = rolloff_factor;
		m_param.reference_distance = reference_distance;
		m_param.dist = max_distance;
	}
	else {
		alSourcef(m_source[layer], AL_ROLLOFF_FACTOR, rolloff_factor);
		alSourcef(m_source[layer], AL_MAX_DISTANCE, max_distance);
		alSourcef(m_source[layer], AL_REFERENCE_DISTANCE, reference_distance);
	}
}

bool CSoundBase::CheckEnd(int layer)
{
	if (m_layer_max == 0) return true;
	ALint state = 0;
	alGetSourcei(m_source[layer], AL_SOURCE_STATE, &state);
	return state == AL_STOPPED || state == AL_INITIAL;


}

float CSoundBase::GetOffset(int layer)
{
	float offset;

	alGetSourcef(m_source[layer], AL_SEC_OFFSET, &offset);

	return offset;
}


CSound*	CSound::m_instance=nullptr;

CSound::CSound(){
	
	mp_device = alcOpenDevice(nullptr);
	if (!mp_device) {
		printf("Failed to open audio device!\n");
		return;
	}

	mp_context = alcCreateContext(mp_device, nullptr);
	if (!mp_context || alcMakeContextCurrent(mp_context) == ALC_FALSE) {
		printf("Failed to create or set audio context!\n");
		if (mp_context) alcDestroyContext(mp_context);
		alcCloseDevice(mp_device);
		return;
	}
	// EFXエクステンションのサポートを確認
	if (!alcIsExtensionPresent(mp_device, "ALC_EXT_EFX")) {
		printf("EFX extension not supported!");
		alcDestroyContext(mp_context);
		alcCloseDevice(mp_device);
		return;
	}
	GetALFunctionAdress();


}
CSound::~CSound() {
	auto it = m_list.begin();
	while (it != m_list.end()) {
		if (it->second) delete it->second;
		it++;
	}

	alcDestroyContext(mp_context);
	alcCloseDevice(mp_device);

}

CSound* CSound::GetInstance()
{
	if (!m_instance) m_instance = new CSound();
	return m_instance;
}

void CSound::ClearInstance()
{
	if (!m_instance) return;
	delete m_instance;
	m_instance = nullptr;
}

CSoundBase* CSound::GetSound(const char* name)
{
	if (!m_list[name]) m_list[name] = new CSoundBase();
	return m_list[name];
}

void CSound::ReleaseSound(const char * name)
{
	auto it = m_list.find(name);
	if (it == m_list.end()) return;
	if (it->second) delete it->second;
	m_list.erase(it);
}

void CSound::UpdateListener() {
	const CVector3D& p = CCamera::GetCurrent()->GetPos();
	const CVector3D& f = CCamera::GetCurrent()->GetDir();
	const CVector3D& u = CCamera::GetCurrent()->GetUp();
	float orientation[6];
	orientation[0] = f.x;
	orientation[1] = f.y;
	orientation[2] = f.z;
	orientation[3] = u.x;
	orientation[4] = u.y;
	orientation[5] = u.z;

	alListenerfv(AL_POSITION, p.v);
	alListenerfv(AL_ORIENTATION, orientation);
	alListenerfv(AL_VELOCITY, CVector3D(0,0,0).v);
	
}

void CSound::StopAll()
{
	for (auto it = m_list.begin(); it != m_list.end(); it++) {
		it->second->Stop();
	}
}
