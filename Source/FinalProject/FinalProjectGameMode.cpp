#include "FinalProjectGameMode.h"
#include "Player/PlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

AFinalProjectGameMode::AFinalProjectGameMode()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
}

void AFinalProjectGameMode::FinishGame_Implementation(APlayerCharacter* player)
{
	APlayerController* PlayerController = player ? Cast<APlayerController>(player->GetController()) : nullptr;
	UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, false);
}
