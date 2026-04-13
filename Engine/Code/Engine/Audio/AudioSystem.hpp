#pragma once


//-----------------------------------------------------------------------------------------------
#include "ThirdParty/fmod/fmod.hpp"
#include <string>
#include <vector>
#include <map>
#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------
typedef size_t SoundID;
typedef size_t SoundPlaybackID;
constexpr size_t MISSING_SOUND_ID = (size_t) (-1); // for bad SoundIDs and SoundPlaybackIDs

struct AudioConfig
{

};


struct AudioListener
{
	Vec3 m_position;
	Vec3 m_velocity;
	Vec3 m_forward;
	Vec3 m_up;
};

//-----------------------------------------------------------------------------------------------
class AudioSystem;


/////////////////////////////////////////////////////////////////////////////////////////////////
class AudioSystem
{
public:
	AudioSystem();
	AudioSystem(AudioConfig const& config);
	virtual ~AudioSystem();

public:
	void						Startup();
	void						Shutdown();
	virtual void				BeginFrame();
	virtual void				EndFrame();

	virtual SoundID				CreateOrGetSound(const std::string& soundFilePath);
	virtual SoundPlaybackID		StartSound(SoundID soundID, bool isLooped = false, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false);
	virtual void				StopSound(SoundPlaybackID soundPlaybackID);
	virtual void				SetSoundPlaybackVolume(SoundPlaybackID soundPlaybackID, float volume);	// volume is in [0,1]
	virtual void				SetSoundPlaybackBalance(SoundPlaybackID soundPlaybackID, float balance);	// balance is in [-1,1], where 0 is L/R centered
	virtual void				SetSoundPlaybackSpeed(SoundPlaybackID soundPlaybackID, float speed);		// speed is frequency multiplier (1.0 == normal)

	virtual void				ValidateResult(FMOD_RESULT result);

	SoundPlaybackID PlaySoundAt(SoundID soundID, Vec3 const& pos, Vec3 const& velocity = Vec3(0.0f, 0.0f, 0.0f), float volume = 1.f, bool isLooped = false);
	void UpdateSoundPosition(SoundPlaybackID soundPlaybackID, Vec3 const& pos);
	void UpdateSoundPositionAndVelocity(SoundPlaybackID soundPlaybackID, Vec3 const& newPos, Vec3 const& newVel);
	void UpdateListeners(int count, AudioListener const* listeners);
	FMOD_VECTOR Vec3ToFMOD(Vec3 vec);
protected:
	FMOD::System* m_fmodSystem;
	std::map< std::string, SoundID >	m_registeredSoundIDs;
	std::vector< FMOD::Sound* >			m_registeredSounds;
	AudioConfig							m_audioConfig;
};

