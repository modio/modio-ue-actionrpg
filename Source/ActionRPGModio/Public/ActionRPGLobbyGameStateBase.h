/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "ActionRPGLobbyGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerStateJoined, APlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerStateLeft, APlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapUpdated, FName, SelectedMap);

/**
 * @nodoc
 */
UCLASS()
class ACTIONRPGMODIO_API AActionRPGLobbyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPlayerStateJoined PlayerJoined;

	UPROPERTY(BlueprintAssignable)
	FPlayerStateLeft PlayerLeft;

	UPROPERTY(BlueprintAssignable)
	FMapUpdated MapUpdated;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ChangeMap(FName InMap);

	UPROPERTY(ReplicatedUsing = OnRep_SelectedMap, BlueprintReadOnly)
	FName SelectedMap;

	UPROPERTY(ReplicatedUsing = OnRep_AllowPlayerStart, BlueprintReadOnly)
	bool AllowPlayerStart = false;

private:
	UFUNCTION()
	void OnRep_SelectedMap();

	UFUNCTION()
	void OnRep_AllowPlayerStart();
};