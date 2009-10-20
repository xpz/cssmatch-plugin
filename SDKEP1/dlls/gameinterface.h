//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Expose things from GameInterface.cpp. Mostly the engine interfaces.
//
// $NoKeywords: $
//=============================================================================//

#ifndef GAMEINTERFACE_H
#define GAMEINTERFACE_H

#ifdef _WIN32
#pragma once
#endif

extern INetworkStringTable *g_pStringTableInfoPanel;

// Player / Client related functions
// Most of this is implemented in gameinterface.cpp, but some of it is per-mod in files like cs_gameinterface.cpp, etc.
class CServerGameClients : public IServerGameClients
{
public:
	virtual bool			ClientConnect( edict_t *pEntity, char const* pszName, char const* pszAddress, char *reject, int maxrejectlen );
	virtual void			ClientActive( edict_t *pEntity, bool bLoadGame );
	virtual void			ClientDisconnect( edict_t *pEntity );
	virtual void			ClientPutInServer( edict_t *pEntity, const char *playername );
	virtual void			ClientCommand( edict_t *pEntity );
	virtual void			ClientSettingsChanged( edict_t *pEntity );
	virtual void			ClientSetupVisibility( edict_t *pViewEntity, edict_t *pClient, unsigned char *pvs, int pvssize );
	virtual float			ProcessUsercmds( edict_t *player, bf_read *buf, int numcmds, int totalcmds,
								int dropped_packets, bool ignore, bool paused );
	// Player is running a command
	virtual void			PostClientMessagesSent( void );
	virtual void			SetCommandClient( int index );
	virtual CPlayerState	*GetPlayerState( edict_t *player );
	virtual void			ClientEarPosition( edict_t *pEntity, Vector *pEarOrigin );

	virtual void			GetPlayerLimits( int& minplayers, int& maxplayers, int &defaultMaxPlayers ) const;
	
	// returns number of delay ticks if player is in Replay mode (0 = no delay)
	virtual int				GetReplayDelay( edict_t *player, int& entity );
	// Anything this game .dll wants to add to the bug reporter text (e.g., the entity/model under the picker crosshair)
	//  can be added here
	virtual void			GetBugReportInfo( char *buf, int buflen );
};


class CServerGameDLL : public IServerGameDLL
{
public:
	virtual bool			DLLInit(CreateInterfaceFn engineFactory, CreateInterfaceFn physicsFactory, 
										CreateInterfaceFn fileSystemFactory, CGlobalVars *pGlobals);
	virtual void			DLLShutdown( void );
	// Get the simulation interval (must be compiled with identical values into both client and game .dll for MOD!!!)
	virtual float			GetTickInterval( void ) const;
	virtual bool			GameInit( void );
	virtual void			GameShutdown( void );
	virtual bool			LevelInit( const char *pMapName, char const *pMapEntities, char const *pOldLevel, char const *pLandmarkName, bool loadGame, bool background );
	virtual void			ServerActivate( edict_t *pEdictList, int edictCount, int clientMax );
	virtual void			LevelShutdown( void );
	virtual void			GameFrame( bool simulating ); // could be called multiple times before sending data to clients
	virtual void			PreClientUpdate( bool simulating ); // called after all GameFrame() calls, before sending data to clients

	virtual ServerClass*	GetAllServerClasses( void );
	virtual const char     *GetGameDescription( void );      
	virtual void			CreateNetworkStringTables( void );
	
	// Save/restore system hooks
	virtual CSaveRestoreData  *SaveInit( int size );
	virtual void			SaveWriteFields( CSaveRestoreData *, char const* , void *, datamap_t *, typedescription_t *, int );
	virtual void			SaveReadFields( CSaveRestoreData *, char const* , void *, datamap_t *, typedescription_t *, int );
	virtual void			SaveGlobalState( CSaveRestoreData * );
	virtual void			RestoreGlobalState( CSaveRestoreData * );
	virtual int				CreateEntityTransitionList( CSaveRestoreData *, int );
	virtual void			BuildAdjacentMapList( void );

	virtual void			PreSave( CSaveRestoreData * );
	virtual void			Save( CSaveRestoreData * );
	virtual void			GetSaveComment( char *comment, int maxlength ) { return GetSaveCommentEx( comment, maxlength, 0, 0 ); }
	virtual void			GetSaveCommentEx( char *comment, int maxlength, float flMinutes, float flSeconds );
#ifdef _XBOX
	virtual void			GetTitleName( const char *pMapName, char* pTitleBuff, int titleBuffSize );
#endif
	virtual void			WriteSaveHeaders( CSaveRestoreData * );

	virtual void			ReadRestoreHeaders( CSaveRestoreData * );
	virtual void			Restore( CSaveRestoreData *, bool );
	virtual bool			IsRestoring();

	// Retrieve info needed for parsing the specified user message
	virtual bool			GetUserMessageInfo( int msg_type, char *name, int maxnamelength, int& size );

	virtual CStandardSendProxies*	GetStandardSendProxies();

	virtual void			PostInit();
	virtual void			Think( bool finalTick );

	float	m_fAutoSaveDangerousTime;

private:

	// This can just be a wrapper on MapEntity_ParseAllEntities, but CS does some tricks in here
	// with the entity list.
	void LevelInit_ParseAllEntities( const char *pMapEntities );
	void LoadMessageOfTheDay();
};


// Normally, when the engine calls ClientPutInServer, it calls a global function in the game DLL
// by the same name. Use this to override the function that it calls. This is used for bots.
typedef CBasePlayer* (*ClientPutInServerOverrideFn)( edict_t *pEdict, const char *playername );

void ClientPutInServerOverride( ClientPutInServerOverrideFn fn );


#endif // GAMEINTERFACE_H

