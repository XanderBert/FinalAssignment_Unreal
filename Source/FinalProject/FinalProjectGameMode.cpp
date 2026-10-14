#include "FinalProjectGameMode.h"
#include "Player/PlayerCharacter.h"

AFinalProjectGameMode::AFinalProjectGameMode()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
}
