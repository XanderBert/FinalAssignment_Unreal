#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "AssertsLibrary.generated.h"

UCLASS()
class UAssertsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
public:
	/**
	* Checks the code path is never taken and trigger a BP breakpoint otherwise.
	* Also log the BP callstack and object context
	* In debug and development builds, all available information is logged and a code breakpoint is triggered.
	* In shipping builds, the check does not run.
	* @param breakReason The reason logged when this check is failed.
	*/
	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CompactNodeTitle = "CheckNoEntry", DisplayName = "CheckNoEntry", Keywords = "Asserts assert check ensure", DevelopmentOnly), Category = "Asserts")
	static ASSERTS_API void CheckNoEntry(const FString& breakReason = FString(TEXT("Break reason.")));
	static void CheckNoEntryImpl(FFrame& StackFrame, const FString& breakReason);
	DECLARE_FUNCTION(execCheckNoEntry)
	{
		P_GET_PROPERTY(FStrProperty, BreakReason);
		P_FINISH;
		P_NATIVE_BEGIN;
		CheckNoEntryImpl(Stack, BreakReason);
		P_NATIVE_END;
	}

	/**
	* Checks if the condition is met and trigger a BP breakpoint otherwise.
	* Also log the BP callstack and object context
	* In debug and development builds, all available information is logged and a code breakpoint is triggered.
	* In shipping builds, the check does not run.
	* @param condition The condition to be met. Will break if false.
	* @param breakReason The reason logged when this check is failed.
	*/
	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CompactNodeTitle="Check", DisplayName = "Check", Keywords = "Asserts assert check ensure", DevelopmentOnly), Category = "Asserts")
	static ASSERTS_API void Check(bool condition, const FString& breakReason = FString(TEXT("Break reason.")));
	static void CheckImpl(FFrame& stackFrame, bool condition, const FString& breakReason);
	DECLARE_FUNCTION(execCheck)
	{
		P_GET_UBOOL(bCondition);
		P_GET_PROPERTY(FStrProperty, BreakReason);
		P_FINISH;
		P_NATIVE_BEGIN;
		CheckImpl(Stack, bCondition, BreakReason);
		P_NATIVE_END;
	}

	/**
	* This can only be used in functions that where called from blueprints and from the game thread.
	* Checks if the condition is met and trigger a BP breakpoint otherwise.
	* Also log the BP callstack and object context
	* In debug and development builds, all available information is logged and a code breakpoint is triggered.
	* In shipping builds, the check does not run.
	* @param condition The condition to be met. Will break if false.
	* @param breakReason The reason logged when this check is failed.
	*/
	static ASSERTS_API void BlueprintBreak(bool condition, const FString& breakReason = FString(TEXT("Break reason.")));
};

