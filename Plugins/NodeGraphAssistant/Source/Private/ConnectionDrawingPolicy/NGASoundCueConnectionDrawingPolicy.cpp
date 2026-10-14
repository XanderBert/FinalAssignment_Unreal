// Copyright 2019 yangxiangyun
// All Rights Reserved

#include "NGASoundCueConnectionDrawingPolicy.h"

#ifdef NGA_WITH_ENGINE_CPP
#include "SoundCueGraphConnectionDrawingPolicy.cpp"
#else
#include "../EngineCppFiles/SoundCueGraphConnectionDrawingPolicy.inl"
#endif


void FNGASoundCueGraphConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry, const FNGASlateVector2D& StartPoint, const FNGASlateVector2D& EndPoint, UEdGraphPin* Pin)
{
	OverrideDrawPreviewConnector(PinGeometry, FVector2D(StartPoint), FVector2D(EndPoint), Pin);
}


void FNGASoundCueGraphConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes)
{
	ResetPayloadData();
	FSoundCueGraphConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
	OverrideDraw(InPinGeometries, ArrangedNodes);
}


void FNGASoundCueGraphConnectionDrawingPolicy::DrawConnection(int32 LayerId, const FNGASlateVector2D& Start, const FNGASlateVector2D& End, const FConnectionParams& Params)
{
	OverrideDrawConnection(LayerId, FVector2D(Start), FVector2D(End), Params);
}