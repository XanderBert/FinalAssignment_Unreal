// Copyright 2019 yangxiangyun
// All Rights Reserved

#include "NGABlueprintConnectionDrawingPolicy.h"


void FNGABPGraphConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry, const FNGASlateVector2D& StartPoint, const FNGASlateVector2D& EndPoint, UEdGraphPin* Pin)
{
	OverrideDrawPreviewConnector(PinGeometry, FVector2D(StartPoint), FVector2D(EndPoint), Pin);
}


void FNGABPGraphConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes)
{
	ResetPayloadData();
	FKismetConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
	OverrideDraw(InPinGeometries, ArrangedNodes);
}


void FNGABPGraphConnectionDrawingPolicy::DrawConnection(int32 LayerId, const FNGASlateVector2D& Start, const FNGASlateVector2D& End, const FConnectionParams& Params)
{
	OverrideDrawConnection(LayerId, FVector2D(Start), FVector2D(End), Params);
}