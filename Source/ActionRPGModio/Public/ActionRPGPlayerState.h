/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "ActionRPGPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerNameUpdated, FString, PlayerName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerReadyUpdated, bool, bIsReady);

/**
 * @nodoc
 */
UCLASS()
class ACTIONRPGMODIO_API AActionRPGPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerNameUpdated PlayerNameUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerReadyUpdated PlayerReadyStatusUpdated;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUpdatePlayerName(const FString& NewPlayerName);

	UFUNCTION(Server, Reliable, Blueprintcallable)
	void ServerUpdateReadyStatus(const bool bInIsReady);

	UFUNCTION(BlueprintCallable, Category = "Player State")
	FString GetCustomPlayerName() const
	{
		return CustomPlayerName;
	}

	UFUNCTION(BlueprintCallable, Category = "Player State")
	bool GetReadyStatus() const
	{
		return bIsReady;
	}

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CustomPlayerName, BlueprintReadOnly, Category = "Player State")
	FString CustomPlayerName;

	UPROPERTY(ReplicatedUsing = OnRep_ReadyStatus, BlueprintReadOnly, Category = "Player State")
	bool bIsReady;

	UFUNCTION()
	void OnRep_CustomPlayerName();

	UFUNCTION()
	void OnRep_ReadyStatus();
};