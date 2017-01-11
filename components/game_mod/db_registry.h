#pragma once
#include "bg_weapons_def.h"

enum FF_DIR : DWORD
{
	FFD_DEFAULT,
	FFD_MOD_DIR,
	FFD_USER_MAP
};

struct XZoneName
{
	char name[64];
	int flags;
	int fileSize;
	FF_DIR dir;
	bool loaded;
};

struct XZoneInfoInternal
{
	char name[64];
	int flags;
};

struct XZoneInfo
{
	const char *name;
	int allocFlags;
	int freeFlags;
};

enum XAssetType
{
	ASSET_TYPE_XMODELPIECES,
	ASSET_TYPE_PHYSPRESET,
	ASSET_TYPE_PHYSCONSTRAINTS,
	ASSET_TYPE_DESTRUCTIBLEDEF,
	ASSET_TYPE_XANIMPARTS,
	ASSET_TYPE_XMODEL,
	ASSET_TYPE_MATERIAL,
	ASSET_TYPE_TECHNIQUE_SET,
	ASSET_TYPE_IMAGE,
	ASSET_TYPE_SOUND,
	ASSET_TYPE_SOUND_PATCH,
	ASSET_TYPE_CLIPMAP,
	ASSET_TYPE_CLIPMAP_PVS,
	ASSET_TYPE_COMWORLD,
	ASSET_TYPE_GAMEWORLD_SP,
	ASSET_TYPE_GAMEWORLD_MP,
	ASSET_TYPE_MAP_ENTS,
	ASSET_TYPE_GFXWORLD,
	ASSET_TYPE_LIGHT_DEF,
	ASSET_TYPE_UI_MAP,
	ASSET_TYPE_FONT,
	ASSET_TYPE_MENULIST,
	ASSET_TYPE_MENU,
	ASSET_TYPE_LOCALIZE_ENTRY,
	ASSET_TYPE_WEAPON,
	ASSET_TYPE_WEAPONDEF,
	ASSET_TYPE_WEAPON_VARIANT,
	ASSET_TYPE_SNDDRIVER_GLOBALS,
	ASSET_TYPE_FX,
	ASSET_TYPE_IMPACT_FX,
	ASSET_TYPE_AITYPE,
	ASSET_TYPE_MPTYPE,
	ASSET_TYPE_MPBODY,
	ASSET_TYPE_MPHEAD,
	ASSET_TYPE_CHARACTER,
	ASSET_TYPE_XMODELALIAS,
	ASSET_TYPE_RAWFILE,
	ASSET_TYPE_STRINGTABLE,
	ASSET_TYPE_PACK_INDEX,
	ASSET_TYPE_XGLOBALS,
	ASSET_TYPE_DDL,
	ASSET_TYPE_GLASSES,
	ASSET_TYPE_EMBLEMSET,
	ASSET_TYPE_COUNT
};

union XAssetHeader
{
	struct Material *material;
	struct MaterialPixelShader *pixelShader;
	struct MaterialVertexShader *vertexShader;
	struct MaterialTechniqueSet *techniqueSet;
	struct GfxImage *image;
	
	struct GfxLightDef *lightDef;
	
	WeaponVariantDef *weapon;

	void *data;
};

// NOTE: These are NOT accurate for singleplayer
enum
{
	DB_ZONE_CODE = 0x1,
	DB_ZONE_CODE_LOC = 0x2,
	DB_ZONE_DEV = 0x4,
	DB_ZONE_DEV_LOC = 0x8,
	DB_ZONE_PATCH = 0x10,
	DB_ZONE_PATCH_LOC = 0x20,
	DB_ZONE_MOD = 0x40,
	DB_ZONE_MOD_LOC = 0x80,
	DB_ZONE_COMMON = 0x100,
	DB_ZONE_COMMON_LOC = 0x200,
	DB_ZONE_FFOTD = 0x400,
	DB_ZONE_FFOTD_LOC = 0x800,
	DB_ZONE_LEVELCOMMON = 0x1000,
	DB_ZONE_LEVELCOMMON_LOC = 0x2000,
	DB_ZONE_LEVEL = 0x4000,
	DB_ZONE_LEVEL_LOC = 0x8000,
	DB_ZONE_LEVELOVERLAY = 0x10000,
	DB_ZONE_LEVELOVERLAY_LOC = 0x20000,
	DB_ZONE_GUMP = 0x40000,
	DB_ZONE_GUMP_LOC = 0x80000,
	DB_ZONE_LOW_MARKER = 0x100000,
	DB_ZONE_MYCHANGES_LOC = 0x200000,
	DB_ZONE_MYCHANGES = 0x400000,
	DB_ZONE_UI_VIEWER_LOC = 0x800000,
	DB_ZONE_UI_VIEWER = 0x1000000,
	DB_ZONE_FRONTEND_LOC = 0x2000000,
	DB_ZONE_FRONTEND = 0x4000000,
	DB_ZONE_HIGH_MARKER = 0x8000000,
	DB_ZONE_LOAD = 0x10000000,
	DB_FLAG_RSTREAM = 0x40000000,
	DB_FLAG_STRICTFREE = 0x80000000,
};

static volatile unsigned int& g_zoneInfoCount = *(volatile unsigned int *)0x00E72984;
static XZoneInfoInternal *g_zoneInfo = (XZoneInfoInternal *)0x00C84308;
static XZoneName *g_zoneNames = (XZoneName *)0x010C6648;
static void **DB_XAssetPool = (void **)0x00B741B8;
static DWORD *g_poolSize = (DWORD *)0x00B73EF8;

typedef void (__cdecl* DB_LoadXAssets_t)(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
static DB_LoadXAssets_t DB_LoadXAssets = (DB_LoadXAssets_t)0x00631B10;

typedef int (__cdecl* DB_ReleaseXAssets_t)();
static DB_ReleaseXAssets_t DB_ReleaseXAssets = (DB_ReleaseXAssets_t)0x0062C260;

typedef bool (__cdecl* DB_ModFileExists_t)();
static DB_ModFileExists_t DB_ModFileExists = (DB_ModFileExists_t)0x005EEBF0;

typedef void (__cdecl* DB_ResetZoneSize_t)(int trackLoadProgress);
static DB_ResetZoneSize_t DB_ResetZoneSize = (DB_ResetZoneSize_t)0x00621530;

typedef int(__cdecl* Sys_SyncDatabase_t)();
static Sys_SyncDatabase_t Sys_SyncDatabase = (Sys_SyncDatabase_t)0x0040DC40;

typedef void (__cdecl* R_BeginRemoteScreenUpdate_t)();
static R_BeginRemoteScreenUpdate_t R_BeginRemoteScreenUpdate = (R_BeginRemoteScreenUpdate_t)0x006D7E60;

typedef void (__cdecl* R_EndRemoteScreenUpdate_t)(void (__cdecl *pumpfunc)());
static R_EndRemoteScreenUpdate_t R_EndRemoteScreenUpdate = (R_EndRemoteScreenUpdate_t)0x006D7EC0;

typedef void (__cdecl* SocketRouter_EmergencyFrame_t)(const char *from);
static SocketRouter_EmergencyFrame_t SocketRouter_EmergencyFrame = (SocketRouter_EmergencyFrame_t)0x004F11D0;

typedef void (__cdecl* DB_PostLoadXZone_t)();
static DB_PostLoadXZone_t DB_PostLoadXZone = (DB_PostLoadXZone_t)0x007A48D0;

typedef int (__cdecl* DB_GetXAssetTypeSize_t)(int type);
static DB_GetXAssetTypeSize_t DB_GetXAssetTypeSize = (DB_GetXAssetTypeSize_t)0x00514840;

typedef void(__cdecl* DB_LogMissingAsset_t)(XAssetType type, const char *name);
static DB_LogMissingAsset_t DB_LogMissingAsset = (DB_LogMissingAsset_t)0x004AEC20;

typedef void(__cdecl* DB_EnumXAssets_t)(XAssetType type, void(__cdecl *func)(XAssetHeader, void *), void *inData, bool includeOverride);
static DB_EnumXAssets_t DB_EnumXAssets = (DB_EnumXAssets_t)0x0054C1C0;

bool DB_IsZonePending(const char *name);
bool DB_IsZoneLoaded(const char *name);

void DB_SyncXAssets();
void DB_LoadGraphicsAssetsForPC();
void DB_ModXFileHandle_hk();
void DB_ModXFileHandle(HANDLE *zoneFile, char* zoneName, FF_DIR *zoneDir);

void* DB_ReallocXAssetPool(XAssetType type, unsigned int size);
