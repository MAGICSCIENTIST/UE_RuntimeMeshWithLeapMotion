//

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"
/**
 *
 */
class FUltraleapAnimCustomDetailsPanel : public IDetailCustomization
{
public:
	FUltraleapAnimCustomDetailsPanel();
	~FUltraleapAnimCustomDetailsPanel();

private:
	/* Contains references to all selected objects inside in the viewport */
	TArray<TWeakObjectPtr<UObject>> SelectedObjects;

public:
	/* Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/* IDetalCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	/* The code that fires when we click the "ChangeColor" button */
	FReply ClickedOnButton();
};
