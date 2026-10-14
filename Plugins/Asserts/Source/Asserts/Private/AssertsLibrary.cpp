// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssertsLibrary.h"
#include "Asserts.h"
#include "Blueprint/BlueprintExceptionInfo.h"


UAssertsLibrary::UAssertsLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void ASSERTS_API UAssertsLibrary::CheckNoEntry(const FString& breakReason)
{
	check(0);
}

void UAssertsLibrary::CheckNoEntryImpl(FFrame& StackFrame, const FString& breakReason)
{
#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
#if DO_BLUEPRINT_GUARD
	const FString Trace = StackFrame.GetStackTrace();
	UE_LOG(LogAsserts, Error, TEXT("\nAssert hit for object %s:\n%s\nCallstack:\n%s"), *StackFrame.Object->GetName(), *breakReason, *Trace);

	FBlueprintExceptionInfo ExceptionInfo = FBlueprintExceptionInfo(EBlueprintExceptionType::Breakpoint);
	FBlueprintCoreDelegates::ThrowScriptException(StackFrame.Object, StackFrame, ExceptionInfo);
#else
	UE_LOG(LogAsserts, Error, TEXT("\nAssert hit for object %s:\n%s"), *StackFrame.Object->GetName(), *breakReason);
#endif

#if !WITH_EDITOR
	UE_DEBUG_BREAK_AND_PROMPT_FOR_REMOTE();
#endif
#endif
}

void ASSERTS_API UAssertsLibrary::Check(bool condition, const FString& breakReason)
{
	check(0);
}

void UAssertsLibrary::CheckImpl(FFrame& StackFrame, bool condition, const FString& breakReason)
{
#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
	if (condition)
	{
		return;
	}

#if DO_BLUEPRINT_GUARD
	const FString Trace = StackFrame.GetStackTrace();
	UE_LOG(LogAsserts, Error, TEXT("\nAssert hit for object %s:\n%s\nCallstack:\n%s"), *StackFrame.Object->GetName(), *breakReason, *Trace);

	FBlueprintExceptionInfo ExceptionInfo = FBlueprintExceptionInfo(EBlueprintExceptionType::Breakpoint);
	FBlueprintCoreDelegates::ThrowScriptException(StackFrame.Object, StackFrame, ExceptionInfo);
#else
	UE_LOG(LogAsserts, Error, TEXT("\nAssert hit for object %s:\n%s"), *StackFrame.Object->GetName(), *breakReason);
#endif

#if !WITH_EDITOR
		UE_DEBUG_BREAK_AND_PROMPT_FOR_REMOTE();
#endif
#endif
}

void UAssertsLibrary::BlueprintBreak(bool condition, const FString& breakReason)
{

#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
	if (condition)
	{
		return;
	}

	TArrayView<FFrame* const> Stack = FBlueprintContextTracker::Get().GetCurrentScriptStackWritable();
	if (!Stack.IsEmpty())
	{
		FFrame& StackFrame = *Stack.Last();

#if DO_BLUEPRINT_GUARD
		const FString Trace = StackFrame.GetStackTrace();
		const FBlueprintExceptionInfo ExceptionInfo = FBlueprintExceptionInfo(EBlueprintExceptionType::Breakpoint);
		FBlueprintCoreDelegates::ThrowScriptException(StackFrame.Object, StackFrame, ExceptionInfo);

		UE_LOG(LogAsserts, Error, TEXT("\nAssert hit for object %s:\n%s\nCallstack:\n%s"), *StackFrame.Object->GetName(), *breakReason, *Trace);
#else
		UE_LOG(LogAsserts, Error, TEXT("\nAssert hit for object %s:\n%s"), *StackFrame.Object->GetName(), *breakReason);
#endif
	}
	else 
	{
		//We should not end up here, but if the assert was called from cpp instead of blueprints we will
		UE_LOG(LogAsserts, Error, TEXT("\nAssert hit: %s"), *breakReason);
		UE_DEBUG_BREAK();
	}

#if !WITH_EDITOR
	UE_DEBUG_BREAK_AND_PROMPT_FOR_REMOTE();
#endif
#endif
}
