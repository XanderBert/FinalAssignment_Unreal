// Copyright 2019 yangxiangyun
// All Rights Reserved

#include "NGAAnimGraphConnectionDrawingPolicy.h"
#include "Version.h"

#ifdef NGA_WITH_ENGINE_CPP
	#if ENGINE_MAJOR_VERSION == 5
		#include "AnimationPins/SGraphPinPose.cpp"
	#endif
#include "AnimGraphConnectionDrawingPolicy.cpp"
#else
#include "../EngineCppFiles/AnimGraphConnectionDrawingPolicy.inl"
#endif


void FNGAAnimGraphConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry, const FNGASlateVector2D& StartPoint, const FNGASlateVector2D& EndPoint, UEdGraphPin* Pin)
{
	OverrideDrawPreviewConnector(PinGeometry, FVector2D(StartPoint), FVector2D(EndPoint), Pin);
}


void FNGAAnimGraphConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes)
{
	ResetPayloadData();
	FAnimGraphConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
	OverrideDraw(InPinGeometries, ArrangedNodes);
}


void FNGAAnimGraphConnectionDrawingPolicy::DrawConnection(int32 LayerId, const FNGASlateVector2D& Start, const FNGASlateVector2D& End, const FConnectionParams& Params)
{
	OverrideDrawConnection(LayerId, FVector2D(Start), FVector2D(End), Params);
}