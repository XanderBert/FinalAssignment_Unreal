// Copyright 2019 yangxiangyun
// All Rights Reserved

#include "NGAMaterialGraphConnectionDrawingPolicy.h"

#ifdef NGA_WITH_ENGINE_CPP
#include "MaterialGraphConnectionDrawingPolicy.cpp"
#else
#include "../EngineCppFiles/MaterialGraphConnectionDrawingPolicy.inl"
#endif


void FNGAMaterialGraphConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry, const FNGASlateVector2D& StartPoint, const FNGASlateVector2D& EndPoint, UEdGraphPin* Pin)
{
	OverrideDrawPreviewConnector(PinGeometry, FVector2D(StartPoint), FVector2D(EndPoint), Pin);
}


void FNGAMaterialGraphConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes)
{
	ResetPayloadData();
	FMaterialGraphConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
	OverrideDraw(InPinGeometries, ArrangedNodes);
}


void FNGAMaterialGraphConnectionDrawingPolicy::DrawConnection(int32 LayerId, const FNGASlateVector2D& Start, const FNGASlateVector2D& End, const FConnectionParams& Params)
{
	OverrideDrawConnection(LayerId, FVector2D(Start), FVector2D(End), Params);
}