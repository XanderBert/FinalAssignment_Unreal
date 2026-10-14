// Copyright 2019 yangxiangyun
// All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Version.h"

#include "SGraphPanel.h"

#include "Layout/Geometry.h"

//Graph nodes return their position as FVector2f since 5.6
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6) || ENGINE_MAJOR_VERSION > 5
#define NGA_NODE_POSITION(Node) FVector2D((Node)->GetPosition2f())
#else
#define NGA_NODE_POSITION(Node) (Node)->GetPosition()
#endif

//SNodePanel::GetChildren is deprecated since 5.8, it always returned the visible children
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 8) || ENGINE_MAJOR_VERSION > 5
#define NGA_PANEL_VISIBLE_CHILDREN(Panel) (Panel)->GetVisibleChildren()
#else
#define NGA_PANEL_VISIBLE_CHILDREN(Panel) (Panel)->GetChildren()
#endif

//FEditorStyle is deprecated in favour of FAppStyle since 5.1
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1) || ENGINE_MAJOR_VERSION > 5
#include "Styling/AppStyle.h"
typedef FAppStyle FNGAAppStyle;
#else
#include "EditorStyleSet.h"
typedef FEditorStyle FNGAAppStyle;
#endif

UENUM()
enum EAlignDirection
{
	AD_InputDirection,
	AD_OutputDirection,
};

struct FNodeHelper
{
	//pull#4006
	static void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin);

	// get all nodes that linked to these nodes.
	static TArray<UEdGraphNode*> GetLinkedNodes(SGraphPanel* graphPanel, TArray<UEdGraphNode*> SourceNodes, bool bDownStream, bool bUpStream);

	static FVector2D GetSplinePointDistanceIfClose(FVector2D SplineStart, FVector2D SplineEnd, FVector2D PointPosition, const UGraphEditorSettings* Settings);

	//
	static float GetWirePointDistanceIfClose(const FArrangedWidget& ArrangedGraphPanel, SGraphPin* APin, SGraphPin* BPin, FVector2D HitPosition, const UGraphEditorSettings* Settings);


	//if given position can hit the spline defined by two give pins.
	static bool GetWirePointHitResult(const FArrangedWidget& ArrangedGraphPanel, SGraphPin* APin, SGraphPin* BPin, FVector2D HitPosition, float WireThickness, const UGraphEditorSettings* Settings);

	/*
	* try to remove nodes but keep wire flow.
	*
	* @param	ForceBypass	    break pin links event when can not bypass them(input pin type not compatible with output pin type).
	* @param	ForceKeepNode	do not remove node event it is bypassed.
	*/
	static bool BypassNodes(UEdGraph* Graph, TArray<UEdGraphNode*> TargetNodes, bool ForceBypass, bool ForceKeepNode);

	//
	static TArray<TArray<UEdGraphNode*>> CalculateNodeshierarchy(TArray<UEdGraphNode*> sourceNodes, EAlignDirection AlignDirection);

	//reorder nodes' position into grid like structure.
	static bool RearrangeSelectedNodes(SGraphPanel* graphPanel, FIntPoint Spacing, float SpacingRelax);

	//for ai graph.
	static bool RearrangeSelectedNodes_AIGraph(SGraphPanel* graphPanel, FIntPoint Spacing, float SpacingRelax);

	static TArray<TSharedRef<SGraphPin>> GetPins(TSharedRef<SGraphNode> GraphNode);

	static void GetAutoConnectablePins(const UEdGraphSchema* GraphSchema, float MaxConnectionDistance, TArray<TSharedRef<SGraphNode>> InSourceNodes, TArray<TSharedRef<SGraphNode>> InTargetNodes, TArray<TWeakPtr<SGraphPin>>& OutStartPins, TArray<TWeakPtr<SGraphPin>>& OutEndPins, TArray<TSharedRef<SGraphPin>> DirtyPins ={});

	inline static FVector2D GetAbsolutePosition(FGeometry Geometry)
	{
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 16) || ENGINE_MAJOR_VERSION > 4
		return Geometry.GetAbsolutePosition();
#else
		return Geometry.AbsolutePosition;
#endif
	}

private:
};
	