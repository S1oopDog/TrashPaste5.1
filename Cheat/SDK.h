#pragma once
#include <Windows.h>
#include <UE4/UE4.h>
#include <string>

#ifdef _MSC_VER
#pragma pack(push, 0x8)
#endif



struct FNameEntry
{
	uint32_t Index;
	uint32_t pad;
	FNameEntry* HashNext;
	char AnsiName[1024];

	const int GetIndex() const { return Index >> 1; }
	const char* GetAnsiName() const { return AnsiName; }
};

template<class TEnum>
class TEnumAsByte
{
public:
	inline TEnumAsByte()
	{
	}

	inline TEnumAsByte(TEnum _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(int32_t _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(uint8_t _value)
		: value(_value)
	{
	}

	inline operator TEnum() const
	{
		return (TEnum)value;
	}

	inline TEnum GetValue() const
	{
		return (TEnum)value;
	}

private:
	uint8_t value;
};

class TNameEntryArray
{
public:

	bool IsValidIndex(uint32_t index) const { return index < NumElements; }

	FNameEntry const* GetById(uint32_t index) const { return *GetItemPtr(index); }

	FNameEntry const* const* GetItemPtr(uint32_t Index) const {
		const auto ChunkIndex = Index / 16384;
		const auto WithinChunkIndex = Index % 16384;
		const auto Chunk = Chunks[ChunkIndex];
		return Chunk + WithinChunkIndex;
	}

	FNameEntry** Chunks[128];
	uint32_t NumElements = 0;
	uint32_t NumChunks = 0;
};

struct FName
{
	int ComparisonIndex = 0;
	int Number = 0;

	static inline TNameEntryArray* GNames = nullptr;

	static const char* GetNameByIdFast(int Id) {
		auto NameEntry = GNames->GetById(Id);
		if (!NameEntry) return nullptr;
		return NameEntry->GetAnsiName();
	}

	static std::string GetNameById(int Id) {
		auto NameEntry = GNames->GetById(Id);
		if (!NameEntry) return std::string();
		return NameEntry->GetAnsiName();
	}

	const char* GetNameFast() const {
		auto NameEntry = GNames->GetById(ComparisonIndex);
		if (!NameEntry) return nullptr;
		return NameEntry->GetAnsiName();
	}

	const std::string GetName() const {
		auto NameEntry = GNames->GetById(ComparisonIndex);
		if (!NameEntry) return std::string();
		return NameEntry->GetAnsiName();
	};

	inline bool operator==(const FName& other) const {
		return ComparisonIndex == other.ComparisonIndex;
	};

	FName() {}

	FName(const char* find) {
		for (auto i = 6000u; i < GNames->NumElements; i++)
		{
			auto name = GetNameByIdFast(i);
			if (!name) continue;
			if (strcmp(name, find) == 0) {
				ComparisonIndex = i;
				return;
			};
		}
	}
};

struct FUObjectItem
{
	class UObject* Object;
	int Flags;
	int ClusterIndex;
	int SerialNumber;
	int pad;
};

struct TUObjectArray
{
	FUObjectItem* Objects;
	int MaxElements;
	int NumElements;

	class UObject* GetByIndex(int index) { return Objects[index].Object; }
};

template<typename Fn>
inline Fn GetVFunction(const void* instance, std::size_t index)
{
	auto vtable = *reinterpret_cast<const void***>(const_cast<void*>(instance));
	return reinterpret_cast<Fn>(vtable[index]);
}


class UClass;
class UObject
{
public:
	UObject(UObject* addr) { *this = addr; }
	static inline TUObjectArray* GObjects = nullptr;
	void* Vtable; // 0x0
	int ObjectFlags; // 0x8
	int InternalIndex; // 0xC
	UClass* Class; // 0x10
	FName Name; // 0x18
	UObject* Outer; // 0x20

	std::string GetName() const;

	const char* GetNameFast() const;

	std::string GetFullName() const;

	template<typename T>
	static T* FindObject(const std::string& name)
	{
		for (int i = 0; i < GObjects->NumElements; ++i)
		{
			auto object = GObjects->GetByIndex(i);

			if (object == nullptr)
			{
				continue;
			}

			if (object->GetFullName() == name)
			{
				return static_cast<T*>(object);
			}
		}
		return nullptr;
	}

	static UClass* FindClass(const std::string& name)
	{
		return FindObject<UClass>(name);
	}

	template<typename T>
	static T* GetObjectCasted(uint32_t index)
	{
		return static_cast<T*>(GObjects->GetByIndex(index));
	}

	bool IsA(UClass* cmp) const;

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindObject<UClass>("Class CoreUObject.Object");
		return ptr;
	}

	inline void dada(class UFunction* function, void* parms)
	{
		return GetVFunction<void(*)(UObject*, class UFunction*, void*)>(this, 58)(this, function, parms);
	}
};


class UField : public UObject
{
public:
	using UObject::UObject;
	UField* Next;
};

class UProperty : public UField
{
public:
	int ArrayDim;
	int ElementSize;
	uint64_t PropertyFlags;
	char pad[0xC];
	int Offset_Internal;
	UProperty* PropertyLinkNext;
	UProperty* NextRef;
	UProperty* DestructorLinkNext;
	UProperty* PostConstructLinkNext;
};


class UStruct : public UField
{
public:
	using UField::UField;

	UStruct* SuperField;
	UField* Children;
	int PropertySize;
	int MinAlignment;
	TArray<uint8_t> Script;
	UProperty* PropertyLink;
	UProperty* RefLink;
	UProperty* DestructorLink;
	UProperty* PostConstructLink;
	TArray<UObject*> ScriptObjectReferences;
};

class UFunction : public UStruct
{
public:
	int FunctionFlags;
	uint16_t RepOffset;
	uint8_t NumParms;
	char pad;
	uint16_t ParmsSize;
	uint16_t ReturnValueOffset;
	uint16_t RPCId;
	uint16_t RPCResponseId;
	UProperty* FirstPropertyToInit;
	UFunction* EventGraphFunction; //0x00A0
	int EventGraphCallOffset;
	char pad_0x00AC[0x4]; //0x00AC
	void* Func; //0x00B0
};


inline void ProcessEvent(void* obj, UFunction* function, void* parms)
{
	auto vtable = *reinterpret_cast<void***>(obj);
	reinterpret_cast<void(*)(void*, UFunction*, void*)>(vtable[55])(obj, function, parms);
}

class UClass : public UStruct
{
public:
	using UStruct::UStruct;
	unsigned char                                      UnknownData00[0x138];                                     // 0x0088(0x0138) MISSED OFFSET

	template<typename T>
	inline T* CreateDefaultObject()
	{
		return static_cast<T*>(CreateDefaultObject());
	}

	inline UObject* CreateDefaultObject()
	{
		return GetVFunction<UObject* (*)(UClass*)>(this, 86)(this);
	}
};

class FString : public TArray<wchar_t>
{
public:
	inline FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? static_cast<int>(std::wcslen(other)) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};
	FString(const wchar_t* other, int count)
	{
		Data = const_cast<wchar_t*>(other);;
		Max = Count = count;
	};

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	inline const wchar_t* wide() const
	{
		return Data;
	}

	int multi(char* name, int size) const
	{
		return WideCharToMultiByte(CP_UTF8, 0, Data, Count, name, size, nullptr, nullptr) - 1;
	}

	inline const wchar_t* c_str() const
	{
		if (Data)
			return Data;
		return L"";
	}
};
enum class EPlayerActivityType : uint8_t
{
	None = 0,
	Bailing = 1,
	Cannon = 2,
	Cannon_END = 3,
	Capstan = 4,
	Capstan_END = 5,
	CarryingBooty = 6,
	CarryingBooty_END = 7,
	Dead = 8,
	Dead_END = 9,
	Digging = 10,
	Dousing = 11,
	EmptyingBucket = 12,
	Harpoon = 13,
	Harpoon_END = 14,
	LoseHealth = 15,
	Repairing = 16,
	Sails = 17,
	Sails_END = 18,
	UndoingRepair = 19,
	Wheel = 20,
	Wheel_END = 21,
};

struct FPirateDescription
{

};

struct AAthenaPlayerCharacter {
};

struct APlayerState
{
	char pad[0x03D0];
	float Score;                                                     // 0x03D0(0x0004) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float Ping;                                                      // 0x03D4(0x0001) (BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	FString	PlayerName;                                                // 0x03D8 (BlueprintVisible, BlueprintReadOnly, Net, ZeroConstructor, RepNotify, HasGetValueTypeHash)


	EPlayerActivityType GetPlayerActivity()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaPlayerState.GetPlayerActivity");
		EPlayerActivityType activity;
		ProcessEvent(this, fn, &activity);
		return activity;
	}


	FPirateDescription GetPirateDesc()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaPlayerState.GetPirateDesc");
		FPirateDescription desc;
		ProcessEvent(this, fn, &desc);
		return desc;
	}

};

struct FMinimalViewInfo {
	FVector Location;
	FRotator Rotation;
	char UnknownData_18[0x10];
	float FOV;
};

struct UFOVHandlerFunctions_GetTargetFOV_Params
{
	class AAthenaPlayerCharacter* Character;
	float ReturnValue;
};

struct UFOVHandlerFunctions_SetTargetFOV_Params
{
	class AAthenaPlayerCharacter* Character;
	float TargetFOV;

	void SetTargetFOV(class AAthenaPlayerCharacter* Character, float TargetFOV)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.FOVHandlerFunctions.SetTargetFOV");
		UFOVHandlerFunctions_SetTargetFOV_Params params;
		params.Character = Character;
		params.TargetFOV = TargetFOV;
		ProcessEvent(this, fn, &params);
	}
};

struct FCameraCacheEntry {
	float TimeStamp;
	char pad[0xc];
	FMinimalViewInfo POV;
};

struct FTViewTarget
{	
	struct AActor* Target; // 0x00(0x08)
	unsigned char UnknownData_8[0x8]; // 0x08(0x08)
	struct FMinimalViewInfo POV; // 0x10(0x5a0)
	struct APlayerState* PlayerState; // 0x5b0(0x08)
	unsigned char UnknownData_5B8[0x8]; // 0x5b8(0x08)
};

struct AController_K2_GetPawn_Params
{
	class APawn* ReturnValue;
};

struct APlayerCameraManager {
	char pad[0x0440];
	FCameraCacheEntry CameraCache; // 0x0440
	FCameraCacheEntry LastFrameCameraCache;
	FTViewTarget ViewTarget;
	FTViewTarget PendingViewTarget;


	FVector GetCameraLocation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerCameraManager.GetCameraLocation");
		FVector location;
		ProcessEvent(this, fn, &location);
		return location;
	};
	FRotator GetCameraRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerCameraManager.GetCameraRotation");
		FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}
};


struct FKey
{
	FName KeyName;
	unsigned char UnknownData00[0x18] = {};

	FKey() {};
	FKey(const char* InName) : KeyName(FName(InName)) {}
};

struct AController {

	char pad_0000[0x3E8];
	class ACharacter* Character; //0x03E8
	char pad_0480[0x70];
	APlayerCameraManager* PlayerCameraManager; //0x0460
	char pad_04f8[0x1031];
	bool IdleDisconnectEnabled; // 0x14D1

	void UnPossess()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.UnPossess");
		ProcessEvent(this, fn, nullptr);
	}	

	void SendToConsole(FString& cmd) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.SendToConsole");
		ProcessEvent(this, fn, &cmd);
	}

	bool WasInputKeyJustPressed(const FKey& Key) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.WasInputKeyJustPressed");
		struct
		{
			FKey Key;

			bool ReturnValue = false;
		} params;

		params.Key = Key;
		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}

	APawn* K2_GetPawn() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.K2_GetPawn");
		AController_K2_GetPawn_Params params;
		class APawn* ReturnValue;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}

	bool ProjectWorldLocationToScreen(const FVector& WorldLocation, FVector2D& ScreenLocation) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.ProjectWorldLocationToScreen");
		struct
		{
			FVector WorldLocation;
			FVector2D ScreenLocation;
			bool ReturnValue = false;
		} params;

		params.WorldLocation = WorldLocation;
		ProcessEvent(this, fn, &params);
		ScreenLocation = params.ScreenLocation;
		return params.ReturnValue;
	}

	FRotator GetControlRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Pawn.GetControlRotation");
		struct FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}

	FRotator GetDesiredRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Pawn.GetDesiredRotation");
		struct FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}

	void AddYawInput(float Val) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.AddYawInput");
		ProcessEvent(this, fn, &Val);
	}

	void AddPitchInput(float Val) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.AddPitchInput");
		ProcessEvent(this, fn, &Val);
	}

	void FOV(float NewFOV) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.FOV");
		ProcessEvent(this, fn, &NewFOV);
	}

	bool LineOfSightTo(ACharacter* Other, const FVector& ViewPoint, const bool bAlternateChecks) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.LineOfSightTo");
		struct {
			ACharacter* Other = nullptr;
			FVector ViewPoint;
			bool bAlternateChecks = false;
			bool ReturnValue = false;
		} params;
		params.Other = Other;
		params.ViewPoint = ViewPoint;
		params.bAlternateChecks = bAlternateChecks;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}
};

struct UHealthComponent {
	char pad[0xD8];
	float MinTimeSinceLastPredictedHealthChangeToFireChangedEventAgain; // 0x00D8(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float MaxHealth;                                                 // 0x00E0(0x0004) (Edit, Net, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float GetMaxHealth() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.HealthComponent.GetMaxHealth");
		float health = 0.f;
		ProcessEvent(this, fn, &health);
		return health;
	}
	float GetCurrentHealth() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.HealthComponent.GetCurrentHealth");
		float health = 0.f;
		ProcessEvent(this, fn, &health);
		return health;
	};
};



struct USkeletalMeshComponent {
	char pad[0x58];
	TArray<FTransform> SpaceBasesArray[2];
	int CurrentEditableSpaceBases;
	int CurrentReadSpaceBases;

	FName GetBoneName(int BoneIndex)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.SkinnedMeshComponent.GetBoneName");
		struct
		{
			int BoneIndex = 0;
			FName ReturnValue;
		} params;
		params.BoneIndex = BoneIndex;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}

	FTransform K2_GetComponentToWorld() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.SceneComponent.K2_GetComponentToWorld");
		FTransform CompToWorld;
		ProcessEvent(this, fn, &CompToWorld);
		return CompToWorld;
	}

	bool GetBone(const uint32_t id, const FMatrix& componentToWorld, FVector& pos) {
		try {
			auto bones = SpaceBasesArray[CurrentReadSpaceBases];

			if (id >= bones.Count) return false;
			const auto& bone = bones[id];
			auto boneMatrix = bone.ToMatrixWithScale();
			auto world = boneMatrix * componentToWorld;
			pos = { world.M[3][0], world.M[3][1], world.M[3][2] };
			return true;
		}
		catch (...)
		{
			printf("error %d\n", __LINE__);
			return false;
		}
	}
};

struct AShipInternalWater {
	float GetNormalizedWaterAmount() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ShipInternalWater.GetNormalizedWaterAmount");
		float params = 0.f;
		ProcessEvent(this, fn, &params);
		return params;
	}
};

struct AHullDamage {
	char pad[0x0428];
	TArray<class ACharacter*> ActiveHullDamageZones; // 0x0420
	bool IsShipSinking()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.HullDamage.IsShipSinking");
		bool IsShipSinking = true;
		ProcessEvent(this, fn, &IsShipSinking);
		return IsShipSinking;
	}
};

struct ADamageZone
{
	char pad1[0x480];
	char LocationOnShip; // 0x480(0x01)
	int GetDamageLevel()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.DamageZone.GetDamageLevel");
		int num;
		ProcessEvent(this, fn, &num);
		return num;
	}
	int isDamageZoneShip()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.DamageZone.GetShipOwner");
		int params;
		ProcessEvent(this, fn, &params);
		
		return params;
	}

	FVector K2_GetActorLocation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.K2_GetActorLocation");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}
};

struct UDrowningComponent {
	float GetOxygenLevel() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.DrowningComponent.GetOxygenLevel");
		float oxygen;
		ProcessEvent(this, fn, &oxygen);
		return oxygen;
	}
};

struct AFauna {
	char pad1[0x0808];
	FString* DisplayName; // 0x0808
};

enum class ESwimmingCreatureType : uint8_t
{
	SwimmingCreature = 0,
	Shark = 1,
	TinyShark = 2,
	Siren = 3,
	ESwimmingCreatureType_MAX = 4
};

struct ASharkPawn {
	char pad1[0x04C8];
	USkeletalMeshComponent* Mesh; // 0x04C8
	char pad2[0x5C];
	ESwimmingCreatureType SwimmingCreatureType; // 0x052C
};

struct FAIEncounterSpecification
{
	char pad[0x80];
	FString* LocalisableName; // 0x0080
};

struct UWieldedItemComponent {
	char pad[0x2f0];
	ACharacter* CurrentlyWieldedItem; // 0x2f0(0x08)
};

struct FWeaponProjectileParams  // ScriptStruct Athena.WeaponProjectileParams
{
	float Damage; // 0x00(0x04)
	float DamageMultiplierAtMaximumRange; // 0x04(0x04)
	float LifeTime; // 0x08(0x04)
	float TrailFadeOutTime; // 0x0c(0x04)
	float Velocity; // 0x10(0x04)
	float TimeBeforeApplyingGravity; // 0x14(0x04)
	float DownForceVelocityFractionPerSecond; // 0x18(0x04)
	float VelocityDampeningPerSecond; // 0x1c(0x04)
	struct FLinearColor Color; // 0x20(0x10)
	struct UClass* ProjectileId; // 0x30(0x08)
	char HealthChangeReason; // 0x38(0x01)
	char UnknownData_39[0x3]; // 0x39(0x03)
	char pad1[0x68];
	int SuggestedMaxSimulationIterations; // 0xa4(0x04)
	float SuggestedMinTickTimeSecs; // 0xa8(0x04)
	float SuggestedMaxSimulationTimeStep; // 0xac(0x04)
	float HitScanTrailUpdateModifier; // 0xb0(0x04)
	float HitScanTrailFadeOutTime; // 0xb4(0x04)
	float HitScanTrailGrowthSpeed; // 0xb8(0x04)
	char UnknownData_BC[0x4]; // 0xbc(0x04)
};

struct FProjectileShotParams {
	int Seed; // 0x00(0x04)
	float ProjectileDistributionMaxAngle; // 0x04(0x04)
	int NumberOfProjectiles; // 0x08(0x04)
	float ProjectileMaximumRange; // 0x0c(0x04)
	float ProjectileHitScanMaximumRange; // 0x10(0x04)
	float ProjectileDamage; // 0x14(0x04)
	float ProjectileDamageMultiplierAtMaximumRange; // 0x18(0x04)
};

struct FProjectileWeaponParameters {
	int AmmoClipSize; // 0x00(0x04)
	int AmmoCostPerShot; // 0x04(0x04)
	float EquipDuration; // 0x08(0x04)
	float IntoAimingDuration; // 0x0c(0x04)
	float RecoilDuration; // 0x10(0x04)
	float ReloadDuration; // 0x14(0x04)
	struct FProjectileShotParams HipFireProjectileShotParams; // 0x18(0x1c)
	struct FProjectileShotParams AimDownSightsProjectileShotParams; // 0x34(0x1c)
	int Seed; // 0x50(0x04)
	float ProjectileDistributionMaxAngle; // 0x54(0x04)
	int NumberOfProjectiles; // 0x58(0x04)
	float ProjectileMaximumRange; // 0x5c(0x04)
	float ProjectileHitScanMaximumRange; // 0x60(0x04)
	float ProjectileDamage; // 0x64(0x04)
	float ProjectileDamageMultiplierAtMaximumRange; // 0x68(0x04)
	char UnknownData_6C[0x4]; // 0x6c(0x04)
	struct UClass* DamagerType; // 0x70(0x08)
	struct UClass* ProjectileId; // 0x78(0x08)
	struct FWeaponProjectileParams AmmoParams; // 0x80(0xc0)
	bool UsesScope; // 0x140(0x01)
	char UnknownData_141[0x3]; // 0x141(0x03)
	float ZoomedRecoilDurationIncrease; // 0x144(0x04)
	float SecondsUntilZoomStarts; // 0x148(0x04)
	float SecondsUntilPostStarts; // 0x14c(0x04)
	float WeaponFiredAINoiseRange; // 0x150(0x04)
	float MaximumRequestPositionDelta; // 0x154(0x04)
	float MaximumRequestAngleDelta; // 0x158(0x04)
	float TimeoutTolerance; // 0x15c(0x04)
	float AimingMoveSpeedScalar; // 0x160(0x04)
	char AimSensitivitySettingCategory; // 0x164(0x01)
	char UnknownData_165[0x3]; // 0x165(0x03)
	float InAimFOV; // 0x168(0x04)
	float BlendSpeed; // 0x16c(0x04)
	char pad1[0x20];
	bool KnockbackEnabled; // 0x190(0x01)
	char UnknownData_191[0x3]; // 0x191(0x03)
	char pad2[0x54];
	float StunDuration; // 0x1e8(0x04)
	struct FVector TargetingOffset; // 0x1ec(0x0c)
};

struct FText
{
	char UnknownData[0x38];
};

struct UKismetTextLibrary
{
public:

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindObject<UClass>("Class Engine.KismetTextLibrary");
		return ptr;
	}

	static class FString Conv_TextToString(const struct FText& InText);
};


struct URepairableComponent {
	unsigned char                                      UnknownData_118[0x18]; // 0x118(0x18)                                    // 0x0118(0x0018) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	float                                              InteractionPointDepthOffset;                               // 0x0160(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaximumRepairAngleToRepairer;                              // 0x0164(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaximumRepairDistance;                                     // 0x0168(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              RepairTime;                                                // 0x016C(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	class UClass* RepairType;                                                // 0x0170(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash)
	class USceneComponent* RepairMountParent;                                         // 0x0178(0x0008) (Edit, BlueprintVisible, ExportObject, ZeroConstructor, InstancedReference, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	struct FTransform                                  RepairMountOffset;                                         // 0x0180(0x0030) (Edit, BlueprintVisible, IsPlainOldData, NoDestructor)
	struct FName                                       RepairMountSocket;                                         // 0x01B0(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                MaxDamageLevel;                                            // 0x01B8(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	unsigned char                                      UnknownData_CZX3[0x4];                                     // 0x01BC(0x0004) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	class UClass* AIInteractionType;                                         // 0x01C0(0x0008) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash)
	struct FVector                                     AIInteractionOffset;                                       // 0x01C8(0x000C) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor)
	unsigned char                                      UnknownData_SYYG[0x4];                                     // 0x01D4(0x0004) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	unsigned char                                      UnknownData_EU9A[0x3];                                     // 0x01F1(0x0003) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	int                                                DamageLevel;                                               // 0x01F4(0x0004) (Net, ZeroConstructor, IsPlainOldData, RepNotify, NoDestructor, HasGetValueTypeHash)
	unsigned char                                      UnknownData_Z10X[0x87];                                    // 0x01F9(0x0087) MISSED OFFSET (PADDING)
};

struct FStorageContainerNode
{
	class UClass* ItemDesc;                                                  // 0x0000(0x0008) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash)
	int NumItems; // 0x08(0x04)
};

struct FStorageContainerBackingStore
{
	char pad1[0x0020];
	TArray<struct FStorageContainerNode>               ContainerNodes;                                            // 0x0020(0x0010) (ZeroConstructor)
};

struct UStorageContainerComponent
{
	char pad1[0x0150];
	FStorageContainerBackingStore               ContainerNodes;                                            // 0x0150(0x0040) (Edit, Net, DisableEditOnInstance, RepNotify, Protected)
};

struct AProjectileWeapon {
	char pad[0x7D0]; // 0
	FProjectileWeaponParameters WeaponParameters; // 0x7d0

	bool CanFire()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ProjectileWeapon.CanFire");
		bool canfire;
		ProcessEvent(this, fn, &canfire);
		return canfire;
	}


};

struct FXMarksTheSpotMapMark
{
	struct FVector2D Position;                                                  // 0x0000(0x0008) (BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor)
	float Rotation;                                                  // 0x0008(0x0004) (BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	bool IsUnderground;                                             // 0x000C(0x0001) (BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor)
};

struct FWorldMapIslandDataCaptureParams
{
	char pad1[0x18];
	struct FVector WorldSpaceCameraPosition;                                  // 0x0018(0x000C) (Edit, ZeroConstructor, Transient, EditConst, IsPlainOldData, NoDestructor)
	char pad2[0x8];
	float CameraOrthoWidth;                                          // 0x002C(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

struct AXMarksTheSpotMap
{
	char pad1[0x818];
	struct FString MapTexturePath;                                            // 0x0818(0x0010) (Net, ZeroConstructor, RepNotify, HasGetValueTypeHash)
	char pad2[0x80];
	TArray<struct FXMarksTheSpotMapMark> Marks;                                                     // 0x08A8(0x0010) (Net, ZeroConstructor, RepNotify)
	char pad3[0x18];
	float Rotation;                                                  // 0x08D0(0x0004) (Net, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

struct FHitResult
{
	unsigned char bBlockingHit : 1; // 0x0000(0x0001)
	unsigned char bStartPenetrating : 1; // 0x0000(0x0001)
	unsigned char UnknownData00[0x3]; // char UnknownData_1[0x3]; 0x01(0x03)
	float Time; // 0x0004(0x0004)
	float Distance; // 0x0008(0x0004)
	char pad1[0x48];
	float PenetrationDepth; // 0x0054(0x0004)
	int Item; // 0x0058(0x0004)
	char pad2[0x18];
	struct FName BoneName; // 0x0074(0x0008)
	int FaceIndex; // 0x007C(0x0004)
};

struct UWorldMapIslandDataAsset {
	char pad_UWMIDA[0x30];
	char pad_WMIDCP[0x18];
	struct FVector WorldSpaceCameraPosition; // 0x18(0x0c)
};																	  

struct UIslandDataAssetEntry {
	char pad1[0x0028];
	struct FName IslandName; // 0x0028(0x0008) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	char pad2[0x10];
	struct UWorldMapIslandDataAsset* WorldMapData; // 0x0040
	char pad3[0x68];
	FString* LocalisedName; // 0x00B0
};

struct UIslandDataAsset {
	char pad[0x48];
	struct TArray<UIslandDataAssetEntry*> IslandDataEntries; // 0x0048
};

struct AIslandService {
	char pad[0x460];
	UIslandDataAsset* IslandDataAsset; // 0x460
};

struct ASlidingDoor {
	char pad_0x0[0x52C];
	FVector InitialDoorMeshLocation; // 0x052C

	void OpenDoor() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.SkeletonFortDoor.OpenDoor");
		ProcessEvent(this, fn, nullptr);
	}
};

enum class EDrawDebugTrace : uint8_t
{
	EDrawDebugTrace__None = 0,
	EDrawDebugTrace__ForOneFrame = 1,
	EDrawDebugTrace__ForDuration = 2,
	EDrawDebugTrace__Persistent = 3,
	EDrawDebugTrace__EDrawDebugTrace_MAX = 4
};

enum class ETraceTypeQuery : uint8_t
{
	TraceTypeQuery1 = 0,
	TraceTypeQuery2 = 1,
	TraceTypeQuery3 = 2,
	TraceTypeQuery4 = 3,
	TraceTypeQuery5 = 4,
	TraceTypeQuery6 = 5,
	TraceTypeQuery7 = 6,
	TraceTypeQuery8 = 7,
	TraceTypeQuery9 = 8,
	TraceTypeQuery10 = 9,
	TraceTypeQuery11 = 10,
	TraceTypeQuery12 = 11,
	TraceTypeQuery13 = 12,
	TraceTypeQuery14 = 13,
	TraceTypeQuery15 = 14,
	TraceTypeQuery16 = 15,
	TraceTypeQuery17 = 16,
	TraceTypeQuery18 = 17,
	TraceTypeQuery19 = 18,
	TraceTypeQuery20 = 19,
	TraceTypeQuery21 = 20,
	TraceTypeQuery22 = 21,
	TraceTypeQuery23 = 22,
	TraceTypeQuery24 = 23,
	TraceTypeQuery25 = 24,
	TraceTypeQuery26 = 25,
	TraceTypeQuery27 = 26,
	TraceTypeQuery28 = 27,
	TraceTypeQuery29 = 28,
	TraceTypeQuery30 = 29,
	TraceTypeQuery31 = 30,
	TraceTypeQuery32 = 31,
	TraceTypeQuery_MAX = 32,
	ETraceTypeQuery_MAX = 33
};

struct APuzzleVault {
	char pad[0x1000];
	ASlidingDoor* OuterDoor; // 0x1000
};

struct FGuid
{
	int                                                A;                                                         // 0x0000(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                B;                                                         // 0x0004(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                C;                                                         // 0x0008(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                D;                                                         // 0x000C(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

struct FSessionTemplate
{
	struct FString                                     TemplateName;                                              // 0x0000(0x0010) (ZeroConstructor, Protected, HasGetValueTypeHash)
	unsigned char             SessionType;                                               // 0x0010(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	unsigned char                                      UnknownData_2Q1C[0x3];                                     // 0x0011(0x0003) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	int                                                MaxPlayers;                                                // 0x0014(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)

};

// ScriptStruct Sessions.CrewSessionTemplate
// 0x0020 (0x0038 - 0x0018)
struct FCrewSessionTemplate : public FSessionTemplate
{
	struct FString MatchmakingHopper;                                         // 0x0018(0x0010) (ZeroConstructor, HasGetValueTypeHash)
	class UClass* ShipSize;                                                  // 0x0028(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash)
	int MaxMatchmakingPlayers;                                     // 0x0030(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	unsigned char  UnknownData_JPXK[0x4];                                     // 0x0034(0x0004) MISSED OFFSET (PADDING)

};

struct FCrew
{
	struct FGuid CrewId;                                                   // 0x0000(0x0010) (ZeroConstructor, IsPlainOldData)
	struct FGuid SessionId;                                                // 0x0010(0x0010) (ZeroConstructor, IsPlainOldData)
	TArray<class APlayerState*> Players;                                                  // 0x0020(0x0010) (ZeroConstructor)
	struct FCrewSessionTemplate CrewSessionTemplate;                                      // 0x0030(0x0038)
	struct FGuid LiveryID;                                                 // 0x0068(0x0010) (ZeroConstructor, IsPlainOldData)
	unsigned char UnknownData00[0x8];                                       // 0x0078(0x0008) MISSED OFFSET
	TArray<class AActor*> AssociatedActors;                                         // 0x0080(0x0010) (ZeroConstructor)
};

struct ACrewService {
	char pad[0x04A8];
	TArray<FCrew> Crews; // 0x04A8
};

class CookingPot
{
	char pad[0x0400];
	class UStaticMeshComponent* MeshComponent;                                             // 0x0400(0x0008) (Edit, ExportObject, ZeroConstructor, EditConst, InstancedReference, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	class UActionRulesInteractableComponent* InteractableComponent;                                     // 0x0408(0x0008) (Edit, ExportObject, ZeroConstructor, DisableEditOnInstance, EditConst, InstancedReference, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	class UCookerComponent* CookerComponent;                                           // 0x0410(0x0008) (Edit, ExportObject, ZeroConstructor, DisableEditOnInstance, EditConst, InstancedReference, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              HoldToInteractTime;                                        // 0x0418(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

class UCookerComponent
{
	char pad1[0x00D0];
	TArray<struct FStatus>                             StatusToApplyToContents;                                   // 0x00D0(0x0010) (Edit, ZeroConstructor)
	TArray<struct FCookerSmokeFeedbackEntry>           VFXFeedback;                                               // 0x00E0(0x0010) (Edit, ZeroConstructor)
	class UStaticMeshMemoryConstraintComponent* CookableStaticMeshComponent;                               // 0x00F0(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	class USkeletalMeshMemoryConstraintComponent* CookableSkeletalMeshComponent;                             // 0x00F8(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	struct FName                                       CookedMaterialParameterName;                               // 0x0100(0x0008) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	struct FName                                       BurnDownDirectionParameterName;                            // 0x0108(0x0008) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              PlacementVarianceAngleBound;                               // 0x0110(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	bool                                               OnByDefault;                                               // 0x0114(0x0001) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor)
	unsigned char                                      UnknownData_VY4R[0x3];                                     // 0x0115(0x0003) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	class UCookingComponentAudioParams* AudioParams;                                               // 0x0118(0x0008) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	unsigned char                                      UnknownData_J410[0x7];                                     // 0x0121(0x0007) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	class AItemInfo* CurrentlyCookingItem;                                      // 0x0128(0x0008) (ZeroConstructor, Transient, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	struct FCookingClientRepresentation* CookingState;                                              // 0x0130(0x0068) (Net, Transient, RepNotify)
	class UParticleSystemComponent* SmokeParticleComponent;                                    // 0x0198(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	class UMaterialInstanceDynamic* VisibleCookableMaterial;                                   // 0x01A0(0x0008) (ZeroConstructor, Transient, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	bool                                               TurnedOn;                                                  // 0x01A8(0x0001) (ZeroConstructor, Transient, IsPlainOldData, NoDestructor)
	bool                                               OnIsland;                                                  // 0x01A9(0x0001) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor)
	unsigned char                                      UnknownData_UJO5[0x9E];                                    // 0x01AA(0x009E) MISSED OFFSET (PADDING)
};
// Class Athena.Ship
// Size: 0x1570 (Inherited: 0x3d0)
struct AShip
{

	FVector GetCurrentVelocity() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Ship.GetCurrentVelocity");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FVector GetVelocity() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetVelocity");
		FVector velocity;
		ProcessEvent(this, fn, &velocity);
		return velocity;
	}
};

struct AShipService
{
	int GetNumShips()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ShipService.GetNumShips");
		int num;
		ProcessEvent(this, fn, &num);
		return num;
	}
};

struct UBootyStorageSettings
{
	char pad1[0x0038];
	float                                              StoreHoldTime;                                             // 0x0038(0x0004) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              RetrieveHoldTime;                                          // 0x003C(0x0004) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              PickupPointSpawnDepth;                                     // 0x0040(0x0004) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              PickupDismissDuration;                                     // 0x0044(0x0004) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              PickupDismissDistanceCheck;                                // 0x0048(0x0004) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	unsigned char                                      MaxStoragePerLocation;                                     // 0x004C(0x0001) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	unsigned char                                      UnknownData_V0LN[0x3];                                     // 0x004D(0x0003) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	TArray<class UClass*>                              BlacklistedCategories;                                     // 0x0060(0x0010) (Edit, ZeroConstructor, Config, UObjectWrapper)
};

struct ABootyStorageService
{
	char pad1[0x0450];
	class UBootyStorageSettings* BootyStorageSettings;                                      // 0x0450(0x0008) (ZeroConstructor, Transient, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	class UBootyStorageSettingsAsset* BootyStorageSettingsAsset;                                 // 0x0458(0x0008) (ZeroConstructor, Transient, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	TArray<struct FCrewBootyStorage>                   Storage;                                                   // 0x0460(0x0010) (Net, ZeroConstructor, RepNotify)
	unsigned char                                      UnknownData_7TKK[0x8];                                     // 0x0750(0x0008) MISSED OFFSET (PADDING)
};

struct ATimeService
{
	char pad1[0x0414];
	uint32_t                                           TimeScalar;                                                // 0x0414(0x0004) (Edit, Net, ZeroConstructor, Config, DisableEditOnInstance, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
};

struct AAthenaGameState {
	char pad[0x05B8];
	struct AWindService* WindService; // 0x5b8(0x08)
	struct APlayerManagerService* PlayerManagerService; // 0x5c0(0x08)
	struct AShipService* ShipService; // 0x5c8(0x08)
	struct AWatercraftService* WatercraftService; // 0x5d0(0x08)
	struct ATimeService* TimeService; // 0x5d8(0x08)
	struct UHealthCustomizationService* HealthService; // 0x5e0(0x08)
	struct UCustomWeatherService* CustomWeatherService; // 0x5e8(0x08)
	struct UCustomStatusesService* CustomStatusesService; // 0x5f0(0x08)
	struct AFFTWaterService* WaterService; // 0x5f8(0x08)
	struct AStormService* StormService; // 0x600(0x08)
	struct ACrewService* CrewService; // 0x608(0x08)
	struct AContestZoneService* ContestZoneService; // 0x610(0x08)
	struct AContestRowboatsService* ContestRowboatsService; // 0x618(0x08)
	struct AIslandService* IslandService; // 0x620(0x08)
	struct ANPCService* NPCService; // 0x628(0x08)
	struct ASkellyFortService* SkellyFortService; // 0x630(0x08)
	struct ADeepSeaRegionService* DeepSeaRegionService; // 0x638(0x08)
	struct AAIDioramaService* AIDioramaService; // 0x640(0x08)
	struct AAshenLordEncounterService* AshenLordEncounterService; // 0x648(0x08)
	struct AAggressiveGhostShipsEncounterService* AggressiveGhostShipsEncounterService; // 0x650(0x08)
	struct ATallTaleService* TallTaleService; // 0x658(0x08)
	struct AAIShipObstacleService* AIShipObstacleService; // 0x660(0x08)
	struct AAIShipService* AIShipService; // 0x668(0x08)
	struct AAITargetService* AITargetService; // 0x670(0x08)
	struct UShipLiveryCatalogueService* ShipLiveryCatalogueService; // 0x678(0x08)
	struct AContestManagerService* ContestManagerService; // 0x680(0x08)
	struct ADrawDebugService* DrawDebugService; // 0x688(0x08)
	struct AWorldEventZoneService* WorldEventZoneService; // 0x690(0x08)
	struct UWorldResourceRegistry* WorldResourceRegistry; // 0x698(0x08)
	struct AKrakenService* KrakenService; // 0x6a0(0x08)
	struct UPlayerNameService* PlayerNameService; // 0x6a8(0x08)
	struct ATinySharkService* TinySharkService; // 0x6b0(0x08)
	struct AProjectileService* ProjectileService; // 0x6b8(0x08)
	struct ULaunchableProjectileService* LaunchableProjectileService; // 0x6c0(0x08)
	struct UServerNotificationsService* ServerNotificationsService; // 0x6c8(0x08)
	struct AAIManagerService* AIManagerService; // 0x6d0(0x08)
	struct AAIEncounterService* AIEncounterService; // 0x6d8(0x08)
	struct AAIEncounterGenerationService* AIEncounterGenerationService; // 0x6e0(0x08)
	struct UEncounterService* EncounterService; // 0x6e8(0x08)
	struct UGameEventSchedulerService* GameEventSchedulerService; // 0x6f0(0x08)
	struct UHideoutService* HideoutService; // 0x6f8(0x08)
	struct UAthenaStreamedLevelService* StreamedLevelService; // 0x700(0x08)
	struct ULocationProviderService* LocationProviderService; // 0x708(0x08)
	struct AHoleService* HoleService; // 0x710(0x08)
	struct APlayerBuriedItemService* PlayerBuriedItemService; // 0x718(0x08)
	struct ULoadoutService* LoadoutService; // 0x720(0x08)
	struct UOcclusionService* OcclusionService; // 0x728(0x08)
	struct UPetsService* PetsService; // 0x730(0x08)
	struct UAthenaAITeamsService* AthenaAITeamsService; // 0x738(0x08)
	struct AAllianceService* AllianceService; // 0x740(0x08)
	struct UMaterialAccessibilityService* MaterialAccessibilityService; // 0x748(0x08)
	struct UNPCLoadedOnClientService* NPCLoadedOnClientService; // 0x750(0x08)
	struct AReapersMarkService* ReapersMarkService; // 0x758(0x08)
	struct AEmissaryLevelService* EmissaryLevelService; // 0x760(0x08)
	struct AFactionService* FactionService; // 0x768(0x08)
	struct ACampaignService* CampaignService; // 0x770(0x08)
	struct AStoryService* StoryService; // 0x778(0x08)
	struct AStorySpawnedActorsService* StorySpawnedActorsService; // 0x780(0x08)
	struct UGlobalVoyageDirectorService* GlobalVoyageDirector; // 0x788(0x08)
	struct AFlamesOfFateSettingsService* FlamesOfFateSettingsService; // 0x790(0x08)
	struct AServiceStatusNotificationsService* ServiceStatusNotificationsService; // 0x798(0x08)
	struct UMigrationService* MigrationService; // 0x7a0(0x08)
	struct AShroudBreakerService* ShroudBreakerService; // 0x7a8(0x08)
	struct UServerUpdateReportingService* ServerUpdateReportingService; // 0x7b0(0x08)
	struct AGenericMarkerService* GenericMarkerService; // 0x7b8(0x08)
	struct AMechanismsService* MechanismsService; // 0x7c0(0x08)
	struct UMerchantContractsService* MerchantContractsService; // 0x7c8(0x08)
	struct UShipFactory* ShipFactory; // 0x7d0(0x08)
	struct URewindPhysicsService* RewindPhysicsService; // 0x7d8(0x08)
	struct UNotificationMessagesDataAsset* NotificationMessagesDataAsset; // 0x7e0(0x08)
	struct AProjectileCooldownService* ProjectileCooldownService; // 0x7e8(0x08)
	struct UIslandReservationService* IslandReservationService; // 0x7f0(0x08)
	struct APortalService* PortalService; // 0x7f8(0x08)
	struct UMeshMemoryConstraintService* MeshMemoryConstraintService; // 0x800(0x08)
	struct ABootyStorageService* BootyStorageService; // 0x808(0x08)
	struct ASpireService* SpireService; // 0x810(0x08)
	struct AFireworkService* FireworkService; // 0x818(0x08)
	struct UAirGivingService* AirGivingService; // 0x820(0x08)
	struct UCutsceneService* CutsceneService; // 0x828(0x08)
	struct ACargoRunService* CargoRunService; // 0x830(0x08)
	struct ACommodityDemandService* CommodityDemandService; // 0x838(0x08)
	struct ADebugTeleportationDestinationService* DebugTeleportationDestinationService; // 0x840(0x08)
	struct ASeasonProgressionUIService* SeasonProgressionUIService; // 0x848(0x08)
	struct UTransientActorService* TransientActorService; // 0x850(0x08)
	struct UTunnelsOfTheDamnedService* TunnelsOfTheDamnedService; // 0x858(0x08)
	struct UWorldSequenceService* WorldSequenceService; // 0x860(0x08)
	struct UItemLifetimeManagerService* ItemLifetimeManagerService; // 0x868(0x08)
	struct ASeaFortsService* SeaFortsService; // 0x870(0x08)
	struct ACustomServerLocalisationService* CustomServerLocalisationService; // 0x878(0x08)
	struct ABeckonService* BeckonService; // 0x880(0x08)
	struct UVolcanoService* VolcanoService; // 0x888(0x08)
	struct UShipAnnouncementService* ShipAnnouncementService; // 0x890(0x08)
};

struct AWheel
{
	int GetLevelOfDamage()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Capstan.GetLevelOfDamage");
		int num;
		ProcessEvent(this, fn, &num);
		return num;
	}
};

struct AMast
{
	int GetDamageLevel()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Mast.GetDamageLevel");
		int num;
		ProcessEvent(this, fn, &num);
		return num;
	}
};

struct ACapstan
{
	int GetLevelOfDamage()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Capstan.GetLevelOfDamage");
		int num;
		ProcessEvent(this, fn, &num);
		return num;
	}
};

struct ASail
{
	void SetBillowAmount(float InBillowingAmount)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Sail.SetBillowAmount");
		struct {
			float InBillowingAmount;
		} params;
		params.InBillowingAmount = InBillowingAmount;

		ProcessEvent(this, fn, &params);
	}
};

struct USceneComponent {
	char pad1[0x108];
	struct FVector RelativeLocation; // 0x108(0x0c)
	struct FRotator RelativeRotation; // 0x114(0x0c)
	struct FVector RelativeScale3D; // 0x120(0x0c)
	char pad2[0x110];
	struct FVector ComponentVelocity; // 0x23c(0x0c)

	void DetachFromParent(bool bMaintainWorldPosition, bool bCallModify)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.SceneComponent.DetachFromParent");
		struct {
			bool bMaintainWorldPosition;
			bool bCallModify;
		} params;
		params.bMaintainWorldPosition = bMaintainWorldPosition;
		params.bCallModify = bCallModify;

		ProcessEvent(this, fn, &params);
	}

	FVector K2_GetComponentLocation() {
		FVector location;
		static auto fn = UObject::FindObject<UFunction>("Function Engine.SceneComponent.K2_GetComponentLocation");
		ProcessEvent(this, fn, &location);
		return location;
	}
};

struct UAthenaCharacterMovementSettings
{
	char pad1[0x0038];
	float SwimmingClientTrustThreshold;                              // 0x0038(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float MaxSwimmingTimeStep;                                       // 0x003C(0x0004) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int MaxSwimmingIterations;                                     // 0x0040(0x0004) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float RebaseToServerErrorSqrDist;                                // 0x0044(0x0004) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float OnlyInterpolationAfterSqrDist;                             // 0x0048(0x0004) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float NoVisualUpdateAfterSqrDist;                                // 0x004C(0x0004) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float TimeBetweenPlayerDistanceChecks;                           // 0x0050(0x0004) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float TimeToSimulateNewCharacters;                               // 0x0054(0x0004) (Edit, ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

struct UCharacterMovementComponent {
	char pad1[0x0180];
	float                                              GravityScale;                                              // 0x0180(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaxStepHeight;                                             // 0x0184(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              JumpZVelocity;                                             // 0x0188(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              JumpOffJumpZFactor;                                        // 0x018C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash)
	float                                              WalkableFloorAngle;                                        // 0x0190(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              WalkableFloorZ;                                            // 0x0194(0x0004) (Edit, ZeroConstructor, EditConst, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              GroundFriction;                                            // 0x01C0(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaxWalkSpeed;                                              // 0x01C4(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaxWalkSpeedBackwards;                                     // 0x01C8(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              WalkBackwardsMinAngle;                                     // 0x01CC(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              WalkBackwardsMaxAngle;                                     // 0x01D0(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaxWalkSpeedCrouched;                                      // 0x01D4(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaxSwimSpeed;                                              // 0x01D8(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaxFlySpeed;                                               // 0x01DC(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaxCustomMovementSpeed;                                    // 0x01E0(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaxAcceleration;                                           // 0x01E4(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaxChargeAcceleration;                                     // 0x01E8(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              BrakingFrictionFactor;                                     // 0x01EC(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              BrakingFriction;                                           // 0x01F0(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              BrakingDecelerationWalking;                                // 0x01F8(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              BrakingDecelerationFalling;                                // 0x01FC(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              BrakingDecelerationSwimming;                               // 0x0200(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              BrakingDecelerationFlying;                                 // 0x0204(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              AirControl;                                                // 0x0208(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              AirControlBoostMultiplier;                                 // 0x020C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              AirControlBoostVelocityThreshold;                          // 0x0210(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              FallingLateralFriction;                                    // 0x0214(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              CrouchedHalfHeight;                                        // 0x0218(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              Buoyancy;                                                  // 0x021C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              PerchRadiusThreshold;                                      // 0x0220(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash)
	float                                              PerchAdditionalHeight;                                     // 0x0224(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash)
	class USceneComponent* DeferredUpdatedMoveComponent;                              // 0x0238(0x0008) (ExportObject, ZeroConstructor, InstancedReference, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaxOutOfWaterStepHeight;                                   // 0x0240(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash)
	float                                              OutofWaterZ;                                               // 0x0244(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash)
	float                                              Mass;                                                      // 0x0248(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	bool                                               bEnablePhysicsInteraction;                                 // 0x024C(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor)
	bool                                               bTouchForceScaledToMass;                                   // 0x024D(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor)
	bool                                               bPushForceScaledToMass;                                    // 0x024E(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor)
	bool                                               bScalePushForceToVelocity;                                 // 0x024F(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor)
	float                                              StandingDownwardForceScale;                                // 0x0250(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              InitialPushForceFactor;                                    // 0x0254(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              PushForceFactor;                                           // 0x0258(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              PushForcePointZOffsetFactor;                               // 0x025C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              TouchForceFactor;                                          // 0x0260(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MinTouchForce;                                             // 0x0264(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaxTouchForce;                                             // 0x0268(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              RepulsionForce;                                            // 0x026C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              NetMoveDelta;                                              // 0x0270(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              CrouchedSpeedMultiplier;                                   // 0x0278(0x0004) (ZeroConstructor, Deprecated, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              UpperImpactNormalScale;                                    // 0x027C(0x0004) (ZeroConstructor, Deprecated, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              AnalogInputModifier;                                       // 0x02B0(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	float                                              MaxSimulationTimeStep;                                     // 0x02C4(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash)
	int                                                MaxSimulationIterations;                                   // 0x02C8(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash)
	float                                              MaxSmoothNetUpdateDist;                                    // 0x02CC(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              NoSmoothNetUpdateDist;                                     // 0x02D0(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              NetworkSimulatedSmoothLocationTime;                        // 0x02D4(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash)
	float                                              NetworkSimulatedSmoothRotationTime;                        // 0x02D8(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash)
	float                                              LedgeCheckThreshold;                                       // 0x02DC(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash)
	float                                              JumpOutOfWaterPitch;                                       // 0x02E0(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash)
	float                                              SmoothingClientPositionTolerance;                          // 0x038C(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              AvoidanceConsiderationRadius;                              // 0x0390(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                AvoidanceUID;                                              // 0x03A0(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, EditConst, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash)
	float                                              AvoidanceWeight;                                           // 0x03B0(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              NavMeshProjectionInterval;                                 // 0x0458(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              NavMeshProjectionTimer;                                    // 0x045C(0x0004) (ZeroConstructor, Transient, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	bool                                               bUseNavMeshProjectionTimeModifier;                         // 0x0460(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor)
	float                                              MaxNavMeshProjectionInterval;                              // 0x0464(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	float                                              NavMeshProjectionInterpSpeed;                              // 0x046C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	bool                                               bUsePlaneBasedNavMeshProjectionInterpSystem;               // 0x0470(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor)
	float                                              MaxAllowedZOffsetFromCurrentPlane;                         // 0x0474(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              MaxZDiffFromPrevPlane;                                     // 0x0478(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              NavMeshProjectionHeightScaleUp;                            // 0x0498(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              NavMeshProjectionHeightScaleDown;                          // 0x049C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	bool                                               UseControllerRotationInNavWalkingMoveComponent;            // 0x04A0(0x0001) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor)
	float                                              MinTimeBetweenTimeStampResets;                             // 0x050C(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	bool                                               bWasSimulatingRootMotion;                                  // 0x0550(0x0001) (ZeroConstructor, Transient, IsPlainOldData, NoDestructor)



	FVector GetCurrentAcceleration() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.CharacterMovementComponent.GetCurrentAcceleration");
		FVector acceleration;
		ProcessEvent(this, fn, &acceleration);
		return acceleration;
	}
};


struct FFloatRange {
	float pad1;
	float min;
	float pad2;
	float max;
};

struct ACannon {
	char pad_9[0x598];
	struct UClass* ProjectileClass; // 0x598(0x08)
	char pad_1[0x4];
	float ProjectileSpeed; // 0x5a4(0x04)
	float ProjectileGravityScale; // 0x5a8(0x04)
	struct FFloatRange PitchRange; // 0x5ac(0x10)
	struct FFloatRange YawRange; // 0x5bc(0x10)
	float PitchSpeed; // 0x5cc(0x04)
	float YawSpeed; // 0x5d0(0x04)
	char pad_2[0x094];
	struct UClass* UseCannonInputId; // 0x668(0x08)
	struct UClass* StartLoadingCannonInputId; // 0x670(0x08)
	struct UClass* StopLoadingCannonInputId; // 0x678(0x08)
	float DefaultFOV; // 0x680(0x04)
	float AimFOV; // 0x684(0x04)
	float IntoAimBlendSpeed; // 0x688(0x04)
	float OutOfAimBlendSpeed; // 0x68c(0x04)
	char pad_3[0x050];
	struct UWwiseEvent* StartPitchMovement; // 0x6e0(0x08)
	struct UWwiseEvent* StopPitchMovement; // 0x6e8(0x08)
	struct UWwiseEvent* StartYawMovement; // 0x6f0(0x08)
	struct UWwiseEvent* StopYawMovement; // 0x6f8(0x08)
	struct UWwiseEvent* StopMovementAtEnd; // 0x700(0x08)
	char pad_4[0x040];
	struct UClass* DefaultLoadedItemDesc; // 0x748(0x08)
	float ClientRotationBlendTime; // 0x750(0x04)
	char pad_5[0x004];
	struct AItemInfo* LoadedItemInfo; // 0x758(0x08)
	char pad_55[0x006];
	float ServerPitch; // 0x76c(0x04)
	float ServerYaw; // 0x770(0x04)
	char pad_6[0x4A4];
	char InteractionState; // 0xc18(0x01)

	void HandlePitchInput(float Pitch)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.HandlePitchInput");
		struct {
			float Pitch;
		} params;
		params.Pitch = Pitch;

		ProcessEvent(this, fn, &params);
	}

	void HandleYawInput(float Yaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.HandleYawInput");
		struct {
			float Yaw;
		} params;
		params.Yaw = Yaw;

		ProcessEvent(this, fn, &params);
	}

	void ForceAimCannon(float Pitch, float Yaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.ForceAimCannon");
		struct {
			float Pitch;
			float Yaw;
		} params;
		params.Pitch = Pitch;
		params.Yaw = Yaw;
		ProcessEvent(this, fn, &params);
	}

	bool IsReadyToFire() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.IsReadyToFire");
		bool is_ready = true;
		ProcessEvent(this, fn, &is_ready);
		return is_ready;
	}

	void Fire() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.Fire");
		ProcessEvent(this, fn, nullptr);
	}

	bool IsReadyToReload() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.IsReadyToReload");
		bool is_ready = true;
		ProcessEvent(this, fn, &is_ready);
		return is_ready;
	}

	void Reload() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.Reload");
		ProcessEvent(this, fn, nullptr);
	}

	void Server_Fire(float Pitch, float Yaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.Server_Fire");
		struct {
			float Pitch;
			float Yaw;
		} params;
		params.Pitch = Pitch;
		params.Yaw = Yaw;
		ProcessEvent(this, fn, &params);
	}

};

struct UItemDesc {
	char pad[0x28];
	FString* Title; // 0x28(0x38) Class Athena.EntitlementDesc
};

struct AItemInfo {
	char pad[0x430];
	UItemDesc* Desc; // 0x0430(0x08)
};

struct UInteractableComponent
{
	char pad1[0x0104];
	float                                              InteractionRadius;                                         // 0x0104(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
};

struct AHarpoonLauncher {
	char pad1[0xB04];
	FRotator rotation; // 0xB04
	// ROTATION OFFSET FOUND USING RECLASS.NET: https://www.unknowncheats.me/forum/sea-of-thieves/470590-reclass-net-plugin.html

	void Server_RequestAim(float InPitch, float InYaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.HarpoonLauncher.Server_RequestAim");
		struct {
			float InPitch;
			float InYaw;
		} params;
		params.InPitch = InPitch;
		params.InYaw = InYaw;
		ProcessEvent(this, fn, &params);
	}
};


struct UInventoryManipulatorComponent {
	bool ConsumeItem(ACharacter* item) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.InventoryManipulatorComponent.ConsumeItem");

		struct
		{
			ACharacter* item;
			bool ReturnValue;
		} params;

		params.item = item;
		params.ReturnValue = false;

		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}
};

class UActorComponent
{
	char UnknownData_28[0x8]; // 0x28(0x08)
	char pad_22[0x50];
	TArray<struct FName> ComponentTags; // 0x80(0x10)
	TArray<struct FSimpleMemberReference> UCSModifiedProperties; // 0x90(0x10)
	char UnknownData_A0[0x10]; // 0xa0(0x10)
	TArray<struct UAssetUserData*> AssetUserData; // 0xb0(0x10)
	char UnknownData_C0_0 : 3; // 0xc0(0x01)
	char bReplicates : 1; // 0xc0(0x01)
	char bNetAddressable : 1; // 0xc0(0x01)
	char UnknownData_C0_5 : 3; // 0xc0(0x01)
	char UnknownData_C1_0 : 6; // 0xc1(0x01)
	char bCreatedByConstructionScript : 1; // 0xc1(0x01)
	char bInstanceComponent : 1; // 0xc1(0x01)
	char bAutoActivate : 1; // 0xc2(0x01)
	char bIsActive : 1; // 0xc2(0x01)
	char bEditableWhenInherited : 1; // 0xc2(0x01)
	char UnknownData_C2_3 : 5; // 0xc2(0x01)
	char UnknownData_C3_0 : 3; // 0xc3(0x01)
	char bNeedsLoadForClient : 1; // 0xc3(0x01)
	char bNeedsLoadForServer : 1; // 0xc3(0x01)
	char pad_1[0x03];
	char UnknownData_C6[0x2]; // 0xc6(0x02)
};

struct FRepMovement {
	FVector LinearVelocity; // 0x00(0x0c)
	FVector AngularVelocity; // 0x0c(0x0c)
	char pad1[0x8];
	FVector AngularVelocityz; // 0x14(0x4)
	FVector Location; // 0x18(0x0c)
	FRotator Rotation; // 0x24(0x0c)
	char bSimulatedPhysicSleep : 1; // 0x30(0x01)
	char bRepPhysics : 1; // 0x30(0x01)
	char UnknownData_30_2 : 6; // 0x30(0x01)
	char LocationQuantizationLevel; // 0x31(0x01)
	char VelocityQuantizationLevel; // 0x32(0x01)
	char RotationQuantizationLevel; // 0x33(0x01)
	char UnknownData_34[0x4]; // 0x34(0x04)
};

struct AActor {
	char pad1[0x94];
	struct FRepMovement ReplicatedMovement;  // 0x94(0x38)
};

struct APickupableObject
{
	char pad1[0x0418];
	float                                              PickupTime;                                                // 0x0418(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
};

struct UPickupableComponent
{
	char pad1[0x00D0];
	float                                              PickupTime;                                                // 0x00D0(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

enum class Athena_ELoadableState : uint8_t
{
	ELoadableState__Unloaded = 0,
	ELoadableState__Unloading = 1,
	ELoadableState__Loading = 2,
	ELoadableState__Loaded = 3,
	ELoadableState__ELoadableState_MAX = 4,
};

struct ULoadableComponent
{
	char pad1[0x00D0];
	float LoadTime;                                                  // 0x00D0(0x0004) (Edit, BlueprintVisible, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float UnloadTime;                                                // 0x00D4(0x0004) (Edit, BlueprintVisible, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	char pad2[0xA4];
	Athena_ELoadableState LoadableComponentState;                                    // 0x0178(0x0010) (Net, RepNotify)
};

enum class Athena_ERegenerationState : uint8_t
{
	ERegenerationState__Regenerating = 0,
	ERegenerationState__Stopped = 1,
	ERegenerationState__Paused = 2,
	ERegenerationState__ERegenerationState_MAX = 3,
};

enum class OlderFHealthRegenState : uint8_t
{
	ERegenerationState__Regenerating = 0,
	ERegenerationState__Stopped = 1,
	ERegenerationState__Paused = 2,
	ERegenerationState__ERegenerationState_MAX = 3,
};

struct UHealthRegenerationPoolComponent_OnRep_RegenerationState_Params
{
	OlderFHealthRegenState OldRegenerationState;                                      // 0x0000(0x0008)  (ConstParm, Parm, OutParm, ReferenceParm)

	void OnRep_RegenerationState(OlderFHealthRegenState& OldRegenerationState)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.HealthRegenerationPoolComponent.OnRep_RegenerationState");
		UHealthRegenerationPoolComponent_OnRep_RegenerationState_Params params;
		params.OldRegenerationState = OldRegenerationState;
		ProcessEvent(this, fn, &params);
	}
};

struct UHealthRegenerationPoolComponent
{
	char pad1[0x00D0];
	float                                              MaxCapacity;                                               // 0x00D0(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	float                                              HealingRate;                                               // 0x00D4(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	float                                              HealingDelayWhenDamaged;                                   // 0x00D8(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	char pad2[0x14];
	struct FHealthRegenState* RegenerationState;                                         // 0x00E8(0x0008) (Net, Transient, RepNotify, Protected)

	void OnRep_RegenerationState(OlderFHealthRegenState& OldRegenerationState)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.HealthRegenerationPoolComponent.OnRep_RegenerationState");
		UHealthRegenerationPoolComponent_OnRep_RegenerationState_Params params;
		params.OldRegenerationState = OldRegenerationState;
		ProcessEvent(this, fn, &params);
	}
};

struct FHealthRegenState
{
	char pad1[0x4];
	Athena_ERegenerationState           State;                                                     // 0x0004(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

struct UActorDamageableComponent
{
	char pad1[0x180];
	UHealthComponent* HealthComp;                                                // 0x0170(0x0008) (ExportObject, ZeroConstructor, Transient, InstancedReference, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};


class ACharacter : public UObject {
public:

	char pad1[0x3C8];	//0x28 from inherit
	APlayerState* PlayerState;  // 0x3F0(0x8) // Class Engine.Pawn
	char pad2[0x10];
	AController* Controller; // 0x408(0x08) // Class Engine.Pawn
	char pad3[0x38];
	USkeletalMeshComponent* Mesh; // 0x448(0x08) // Class Engine.Character
	UCharacterMovementComponent* CharacterMovement; // 0x450 // Class Engine.Character
	char pad4[0x3D8];
	UWieldedItemComponent* WieldedItemComponent; // 0x830(0x08) // Class Athena.AthenaCharacter
	char pad43[0x8];
	UInventoryManipulatorComponent* InventoryManipulatorComponent; // 0x840(0x08) // Class Athena.AthenaCharacter
	char pad5[0x10];
	UHealthComponent* HealthComponent; // 0x858(0x08) // Class Athena.AthenaCharacter
	char pad6[0x10];
	UActorDamageableComponent* ActorDamageableComponent;  // 0x870(0x08) Class Athena.AthenaCharacter
	char pad7[0x430];
	USceneComponent* FirstPersonMeshOffsetComponent;   // 0xca8(0x08) // Class Athena.AthenaPlayerCharacter
	char pad9[0x58];
	UDrowningComponent* DrowningComponent; // 0xd08(0x08) // Class Athena.AthenaPlayerCharacter
	char pad10[0x10];
	UClass* ShipSizeObject; // 0xd20(0x08) // Class Athena.Ship


	void ReceiveTick(float DeltaSeconds)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.ActorComponent.ReceiveTick");
		ProcessEvent(this, fn, &DeltaSeconds);
	}

	void GetActorBounds(bool bOnlyCollidingComponents, FVector& Origin, FVector& BoxExtent) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorBounds");
		struct
		{
			bool bOnlyCollidingComponents = false;
			FVector Origin;
			FVector BoxExtent;
		} params;

		params.bOnlyCollidingComponents = bOnlyCollidingComponents;

		ProcessEvent(this, fn, &params);

		Origin = params.Origin;
		BoxExtent = params.BoxExtent;
	}

	bool K2_SetActorLocation(FVector& NewLocation, bool bSweep, FHitResult& SweepHitResult, bool bTeleport)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.K2_SetActorLocation");
		struct
		{
			FVector NewLocation;
			bool bSweep = false;
			FHitResult SweepHitResult;
			bool bTeleport = false;
		} params;

		params.NewLocation = NewLocation;
		params.bSweep = bSweep;
		params.bTeleport = bTeleport;

		ProcessEvent(this, fn, &params);

		SweepHitResult = params.SweepHitResult;
	}

	bool BlueprintUpdateCamera(FVector& NewCameraLocation, FRotator& NewCameraRotation, float& NewCameraFOV)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerCameraManager.BlueprintUpdateCamera");
		struct
		{
			FVector NewCameraLocation;
			FRotator NewCameraRotation;
			float NewCameraFOV;
		}   params;

		params.NewCameraLocation = NewCameraLocation;
		params.NewCameraRotation = NewCameraRotation;
		params.NewCameraFOV = NewCameraFOV;

		ProcessEvent(this, fn, &params);

		NewCameraLocation = params.NewCameraLocation;
		NewCameraRotation = params.NewCameraRotation;
		NewCameraFOV = params.NewCameraFOV;
	}

	ACharacter* GetCurrentShip() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.GetCurrentShip");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	}

	ACharacter* GetAttachParentActor() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetAttachParentActor");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	};

	ACharacter* GetParentActor() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetParentActor");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	};

	ACharacter* GetWieldedItem() {
		if (!WieldedItemComponent) return nullptr;
		return WieldedItemComponent->CurrentlyWieldedItem;
	}

	
	FVector GetVelocity() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetVelocity");
		FVector velocity;
		ProcessEvent(this, fn, &velocity);
		return velocity;
	}
	FVector GetRepMovement() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.OnRep_ReplicatedMovement");
		FVector Movement;
		ProcessEvent(this, fn, &Movement);
		return Movement;
	}
	FVector GetForwardVelocity() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorForwardVector");
		FVector ForwardVelocity;
		ProcessEvent(this, fn, &ForwardVelocity);
		return ForwardVelocity;
	}
	FVector GetActorRightVector() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorRightVector");
		FVector RightVelocity;
		ProcessEvent(this, fn, &RightVelocity);
		return RightVelocity;
	}
	AItemInfo* GetItemInfo() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ItemProxy.GetItemInfo");
		AItemInfo* info = nullptr;
		ProcessEvent(this, fn, &info);
		return info;
	}

	void CureAllAilings() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.CureAllAilings");
		ProcessEvent(this, fn, nullptr);
	}

	void Kill(uint8_t DeathType) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.Kill");
		ProcessEvent(this, fn, &DeathType);
	}

	bool IsDead() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.IsDead");
		bool isDead = true;
		ProcessEvent(this, fn, &isDead);
		return isDead;
	}

	bool IsInWater() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.IsInWater");
		bool isInWater = false;
		ProcessEvent(this, fn, &isInWater);
		return isInWater;
	}

	bool IsLoading() {
		static auto fn = UObject::FindObject<UFunction>("Function AthenaLoadingScreen.AthenaLoadingScreenBlueprintFunctionLibrary.IsLoadingScreenVisible");
		bool isLoading = true;
		ProcessEvent(this, fn, &isLoading);
		return isLoading;
	}

	AHullDamage* IsShipSinking()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.HullDamage.IsShipSinking");
		AHullDamage* IsShipSinking = 0;
		ProcessEvent(this, fn, &IsShipSinking);
		return IsShipSinking;
	}

	bool IsSinking()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.SinkingComponent.IsSinking");
		bool IsSinking = true;
		ProcessEvent(this, fn, &IsSinking);
		return IsSinking;
	}

	bool PlayerIsTeleporting()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.StreamingTelemetryBaseEvent.PlayerIsTeleporting");
		bool PlayerIsTeleporting = true;
		ProcessEvent(this, fn, &PlayerIsTeleporting);
		return PlayerIsTeleporting;
	}

	FRotator K2_GetActorRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.K2_GetActorRotation");
		FRotator params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	void SetActorHiddenInGame(bool bNewHidden)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.SetActorHiddenInGame");
		struct
		{
			bool bNewHidden = false;
		} params;
		params.bNewHidden = bNewHidden;

		ProcessEvent(this, fn, &params);
	}

	FVector K2_GetActorLocation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.K2_GetActorLocation");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FVector GetActorForwardVector() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorForwardVector");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FVector GetActorUpVector() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorUpVector");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	inline bool isSkeleton() {
		static auto obj = UObject::FindClass("Class Athena.AthenaAICharacter");
		return IsA(obj);
	}

	inline bool isPlayer() {
		static auto obj = UObject::FindClass("Class Athena.AthenaPlayerCharacter");
		return IsA(obj);
	}

	inline bool isPuzzleVault() {
		static auto obj = UObject::FindClass("Class Athena.PuzzleVault");
		return IsA(obj);
	}

	inline bool isShip() {
		static auto obj = UObject::FindClass("Class Athena.Ship");
		return IsA(obj);
	}

	inline bool isSloop() {
		static auto obj = UObject::FindClass("Class Athena.SmallShip");
		return IsA(obj);
	}

	inline bool isBrig() {
		static auto obj = UObject::FindClass("Class Athena.MediumShip");
		return IsA(obj);
	}

	inline bool isGally() {
		static auto obj = UObject::FindClass("Class Athena.LargeShip");
		return IsA(obj);
	}

	inline bool isCannonProjectile() {
		static auto obj = UObject::FindClass("Class Athena.CannonProjectile");
		return IsA(obj);
	}

	inline bool isMapTable() {
		static auto obj = UObject::FindClass("Class Athena.MapTable");
		return IsA(obj);
	}

	inline bool isTreasureLocation() {
		static auto obj = UObject::FindClass("Class Athena.BuriedTreasureLocation");
		return IsA(obj);
	}

	inline bool isShipwreck() {
		static auto obj = UObject::FindClass("Class Athena.Shipwreck");
		return IsA(obj);
	}

	inline bool isXMarkMap() {
		static auto obj = UObject::FindClass("Class Athena.XMarksTheSpotMap");
		return IsA(obj);
	}

	inline bool isNetProxy() {
		static auto obj = UObject::FindClass("Class Athena.NetProxy");
		return IsA(obj);
	}

	inline bool isGoldHoardersShopKeeper() {
		static auto obj = UObject::FindClass("BlueprintGeneratedClass BP_Campaign005_CursedSails_ShopkeeperTim_PreCursedSails.BP_Campaign005_CursedSails_ShopkeeperTim_PreCursedSails_C");
		return IsA(obj);
	}

	inline bool MastEmissaryFlag() {
		static auto obj = UObject::FindClass("Class Athena.MastEmissaryFlag");
		return IsA(obj);
	}

	inline bool isCompanyKeeper() {
		static auto obj = UObject::FindClass("BlueprintGeneratedClass BP_Orderofsouls_MadameOlwen.BP_Orderofsouls_MadameOlwen_C");
		return IsA(obj);
	}

	inline bool isCookingSpot() {
		static auto obj = UObject::FindClass("BlueprintGeneratedClass BP_ShipCookingPot.BP_ShipCookingPot_C");
		return IsA(obj);
	}

	inline bool isHarpoon() {
		static auto obj = UObject::FindClass("Class Athena.HarpoonLauncher");
		return IsA(obj);
	}

	inline bool isCannon() {
		static auto obj = UObject::FindClass("Class Athena.Cannon");
		return IsA(obj);
	}

	inline bool isFarShip() {
		static auto obj = UObject::FindClass("Class Athena.ShipNetProxy");
		return IsA(obj);
	}

	inline bool isEmissaryTable() {
		static auto obj = UObject::FindClass("Class Athena.EmissaryVoteInteractionActor");
		return IsA(obj);
	}

	inline bool isItem() {
		static auto obj = UObject::FindClass("Class Athena.ItemProxy");
		return IsA(obj);
	}

	inline bool isAmmoChest() {
		static auto obj = UObject::FindClass("Class Athena.AmmoChest");
		return IsA(obj);
	}

	inline bool isNewHole()
	{
		static auto obj = UObject::FindClass("BlueprintGeneratedClass BP_BaseInternalDamageZone.BP_BaseInternalDamageZone_C");
		return IsA(obj);
	}

	inline bool isKeg() {
		static auto obj = UObject::FindClass("Class Athena.ItemProxy");
		return IsA(obj);
	}

	inline bool isShark() {
		static auto obj = UObject::FindClass("Class Athena.SharkPawn");
		return IsA(obj);
	}

	inline bool isMegalodon() {
		static auto obj = UObject::FindClass("Class Athena.TinyShark");
		return IsA(obj);
	}

	inline bool isMermaid() {
		static auto obj = UObject::FindClass("Class Athena.Mermaid");
		return IsA(obj);
	}

	inline bool isRowboat() {
		static auto obj = UObject::FindClass("Class Watercrafts.Rowboat");
		return IsA(obj);
	}

	inline bool isAnimal() {
		static auto obj = UObject::FindClass("Class AthenaAI.Fauna");
		return IsA(obj);
	}

	inline bool isEvent() {
		static auto obj = UObject::FindClass("Class Athena.GameplayEventSignal");
		return IsA(obj);
	}

	inline bool isGhostShip() {
		static auto obj = UObject::FindClass("Class Athena.AggressiveGhostShip");
		return IsA(obj);
	}

	inline bool isSail() {
		static auto obj = UObject::FindClass("Class Athena.Sail");
		return IsA(obj);
	}

	inline bool isCapstan() {
		static auto obj = UObject::FindClass("Class Athena.Capstan");
		return IsA(obj);
	}

	inline bool isWheel() {
		static auto obj = UObject::FindClass("Class Athena.Wheel");
		return IsA(obj);
	}

	inline bool isStorm() {
		static auto obj = UObject::FindClass("Class Athena.Storm");
		return IsA(obj);
	}

	inline bool isFog() {
		static auto obj = UObject::FindClass("Class Athena.FogBank");
		return IsA(obj);
	}

	inline bool isMast() {
		static auto obj = UObject::FindClass("Class Athena.Mast");
		return IsA(obj);
	}

	inline bool isDamageZone() {
		static auto obj = UObject::FindClass("Class Athena.DamageZone");
		return IsA(obj);
	}

	bool isWeapon() {
		static auto obj = UObject::FindClass("Class Athena.ProjectileWeapon");
		return IsA(obj);
	}

	bool isBucket() {
		static auto obj = UObject::FindClass("Class Athena.Bucket");
		return IsA(obj);
	}

	bool isSword() {
		static auto obj = UObject::FindClass("Class Athena.MeleeWeapon");
		return IsA(obj);
	}

	bool isGrenade() {
		static auto obj = UObject::FindClass("Class Athena.Grenade");
		return IsA(obj);
	}

	inline bool isBarrel() {
		static auto obj = UObject::FindClass("Class Athena.StorageContainer");
		return IsA(obj);
	}

	inline bool Seagulls() {
		static auto obj = UObject::FindClass("Class Athena.Seagulls");
		return IsA(obj);
	}

	inline bool Seagulls2() {
		static auto obj = UObject::FindClass("BlueprintGeneratedClass BP_Seagull01_32POI_Circling_Shipwreck.BP_Seagull01_32POI_Circling_Shipwreck_C");
		return IsA(obj);
	}

	bool isBuriedTreasure() {
		static auto obj = UObject::FindClass("Class Athena.BuriedTreasureLocation");
		return IsA(obj);
	}

	AHullDamage* GetHullDamage() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Ship.GetHullDamage");
		AHullDamage* params = nullptr;
		ProcessEvent(this, fn, &params);
		return params;
	}

	AShipInternalWater* GetInternalWater() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Ship.GetInternalWater");
		AShipInternalWater* params = nullptr;
		ProcessEvent(this, fn, &params);
		return params;
	}

	bool CanJump() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Character.CanJump");
		bool can_jump = false;
		ProcessEvent(this, fn, &can_jump);
		return can_jump;
	}

	void Jump() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Character.Jump");
		ProcessEvent(this, fn, nullptr);
	}

	bool IsMastDown() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Mast.IsMastFullyDamaged");
		bool ismastdown = false;
		ProcessEvent(this, fn, &ismastdown);
		return ismastdown;
	}

	float GetTargetFOV(class AAthenaPlayerCharacter* Character) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.FOVHandlerFunctions.GetTargetFOV");
		UFOVHandlerFunctions_GetTargetFOV_Params params;
		params.Character = Character;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}

	void SetTargetFOV(class AAthenaPlayerCharacter* Character, float TargetFOV) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.FOVHandlerFunctions.SetTargetFOV");
		UFOVHandlerFunctions_SetTargetFOV_Params params;
		params.Character = Character;
		params.TargetFOV = TargetFOV;
		ProcessEvent(this, fn, &params);
	}
};

class UKismetMathLibrary {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetMathLibrary");
	}
	static FRotator NormalizedDeltaRotator(const struct FRotator& A, const struct FRotator& B) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.NormalizedDeltaRotator");

		struct
		{
			struct FRotator A;
			struct FRotator B;
			struct FRotator ReturnValue;
		} params;

		params.A = A;
		params.B = B;

		ProcessEvent(defaultObj, fn, &params);

		return params.ReturnValue;

	};
	static FRotator FindLookAtRotation(const FVector& Start, const FVector& Target) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.FindLookAtRotation");

		struct {
			FVector Start;
			FVector Target;
			FRotator ReturnValue;
		} params;
		params.Start = Start;
		params.Target = Target;

		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}

	static FVector Conv_RotatorToVector(const struct FRotator& InRot) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.Conv_RotatorToVector");

		struct
		{
			struct FRotator                InRot;
			struct FVector                 ReturnValue;
		} params;
		params.InRot = InRot;

		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}

	static bool LineTraceSingle_NEW(class UObject* WorldContextObject, const struct FVector& Start, const struct FVector& End, ETraceTypeQuery TraceChannel, bool bTraceComplex, TArray<class AActor*> ActorsToIgnore, EDrawDebugTrace DrawDebugType, bool bIgnoreSelf, struct FHitResult* OutHit)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.LineTraceSingle_NEW");

		struct
		{
			class UObject* WorldContextObject;
			struct FVector Start;
			struct FVector End;
			ETraceTypeQuery	TraceChannel;
			bool bTraceComplex;
			TArray<class AActor*> ActorsToIgnore;
			EDrawDebugTrace	DrawDebugType;
			struct FHitResult OutHit;
			bool bIgnoreSelf;
			bool ReturnValue;
		}   params;

		params.WorldContextObject = WorldContextObject;
		params.Start = Start;
		params.End = End;
		params.TraceChannel = TraceChannel;
		params.bTraceComplex = bTraceComplex;
		params.ActorsToIgnore = ActorsToIgnore;
		params.DrawDebugType = DrawDebugType;
		params.bIgnoreSelf = bIgnoreSelf;

		ProcessEvent(defaultObj, fn, &params);

		if (OutHit != nullptr)
			*OutHit = params.OutHit;

		return params.ReturnValue;
	}

	
	static void DrawDebugBox(UObject* WorldContextObject, const FVector& Center, const FVector& Extent, const FLinearColor& LineColor, const FRotator& Rotation, float Duration) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugBox");
		struct
		{
			UObject* WorldContextObject = nullptr;
			FVector Center;
			FVector Extent;
			FLinearColor LineColor;
			FRotator Rotation;
			float Duration = INFINITY;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.Center = Center;
		params.Extent = Extent;
		params.LineColor = LineColor;
		params.Rotation = Rotation;
		params.Duration = Duration;
		ProcessEvent(defaultObj, fn, &params);
	}
	static void DrawDebugArrow(UObject* WorldContextObject, const FVector& LineStart, const FVector& LineEnd, float ArrowSize, const FLinearColor& LineColor, float Duration) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugBox");
		struct
		{
			class UObject* WorldContextObject = nullptr;
			struct FVector LineStart;
			struct FVector LineEnd;
			float ArrowSize = 1.f;
			struct FLinearColor LineColor;
			float Duration = 1.f;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.LineStart = LineStart;
		params.LineEnd = LineEnd;
		params.ArrowSize = ArrowSize;
		params.LineColor = LineColor;
		params.Duration = Duration;

		ProcessEvent(defaultObj, fn, &params);
	}
};

struct UCrewFunctions {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Athena.CrewFunctions");
	}
	static bool AreCharactersInSameCrew(ACharacter* Player1, ACharacter* Player2) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.CrewFunctions.AreCharactersInSameCrew");
		struct
		{
			ACharacter* Player1;
			ACharacter* Player2;
			bool ReturnValue;
		} params;
		params.Player1 = Player1;
		params.Player2 = Player2;
		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}
};

struct UPlayer {
	char UnknownData00[0x30];
	AController* PlayerController; // 0x30(0x08) // Class Engine.Player
};

struct UGameInstance {
	char UnknownData00[0x38];
	TArray<UPlayer*> LocalPlayers; // 0x38(0x10) // Class Engine.GameInstance
};

struct ULevel {
	char UnknownData00[0xA0]; // 0x28(0xa0) // Class Engine.Level
	TArray<ACharacter*> AActors;
};

struct UNetConnection
{
	char pad[0xA0];
	int                                                MaxPacket;                                                 // 0x00A0(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	double                                             LastReceiveTime;                                           // 0x03A0(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};


struct UNetDriver
{
	char pad[0x0048];
	int                                                MaxDownloadSize;                                           // 0x0048(0x0004) (ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                NetServerMaxTickRate;                                      // 0x0050(0x0004) (ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                MaxInternetClientRate;                                     // 0x0054(0x0004) (ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                MaxClientRate;                                             // 0x0058(0x0004) (ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              ServerTravelPause;                                         // 0x005C(0x0004) (ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              SpawnPrioritySeconds;                                      // 0x0060(0x0004) (ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              RelevantTimeout;                                           // 0x0064(0x0004) (ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              KeepAliveTime;                                             // 0x0068(0x0004) (ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              InitialConnectTimeout;                                     // 0x006C(0x0004) (ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              ConnectionTimeout;                                         // 0x0070(0x0004) (ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	unsigned char                                      UnknownData_QNLT[0x4];                                     // 0x0074(0x0004) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	class UNetConnection* ServerConnection;                                          // 0x0078(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	class UNetConnection* ClientConnections;                                         // 0x0080(0x0010) (ZeroConstructor)
	class UWorld* World;                                                     // 0x0090(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	class UClass* NetConnectionClass;                                        // 0x00C8(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	class UProperty* RoleProperty;                                              // 0x00D0(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	class UProperty* RemoteRoleProperty;                                        // 0x00D8(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	struct FName                                       NetDriverName;                                             // 0x00E0(0x0008) (ZeroConstructor, Config, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

struct UWorld {
	static inline UWorld** GWorld = nullptr;
	char pad1[0x30];
	ULevel* PersistentLevel; // 0x30(0x08) // Class Engine.World
	char pad3[0x20];
	AAthenaGameState* GameState; //0x58(0x08) // Class Engine.World // AGameState* GameState;
	char pad4[0xF0];
	TArray<ULevel*> Levels;  // 0x150(0x10) // Class Engine.World // TArray<struct ULevel*> Levels;
	char pad5[0x50];
	ULevel* CurrentLevel; // 0x1b0(0x08) // Class Engine.World 
	char pad6[0x8];
	UGameInstance* GameInstance; // 0x1C0(0x08) // Class Engine.World // UGameInstance* OwningGameInstance;
};

struct AMapTable
{
	char pad[0x4E8];
	TArray<struct FVector2D> MapPins; // 0x4e8(0x10)
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif