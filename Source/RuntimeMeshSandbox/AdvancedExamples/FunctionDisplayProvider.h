// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshProvider.h"
#include "FunctionDisplayProvider.generated.h"


UCLASS(HideCategories = Object, BlueprintType)
class RUNTIMEMESHSANDBOX_API UFunctionDisplayProvider : public URuntimeMeshProvider
{
	GENERATED_BODY()
private:
	mutable FCriticalSection PropertySyncRoot;
	FBoxSphereBounds LocalBounds;

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetDisplayMaterial, BlueprintSetter = SetDisplayMaterial)
	UMaterialInterface* DisplayMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetTime, BlueprintSetter = SetTime)
	float Time;



	float MinValue, MaxValue;
	float MinX, MaxX, MinY, MaxY;
	int32 PointsSX, PointsSY;
	float SizeX, SizeY;			

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetPointsNumberR, BlueprintSetter = SetPointsNumberR)
	int32 PointsNumber_R;

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetPointsNumberZ, BlueprintSetter = SetPointsNumberZ)
	int32 PointsNumber_Z;

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetSizeZ, BlueprintSetter = SetSizeZ)
	float SizeZ;

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetRadiusDefault, BlueprintSetter = SetRadiusDefault)
	float RadiusDefault;

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetRadius, BlueprintSetter = SetRadius)
	TArray<float> RadiusList;
	TArray<float> R_ZList;	

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetPercentOfInsideRadius, BlueprintSetter = SetPercentOfInsideRadius)
	float PercentOfInsideRadius;
public:
	UFunctionDisplayProvider();


	UFUNCTION(BlueprintCallable)
	UMaterialInterface* GetDisplayMaterial() const;
	UFUNCTION(BlueprintCallable)
	void SetDisplayMaterial(UMaterialInterface* InMaterial);

	UFUNCTION(BlueprintCallable)
	float GetTime() const;
	UFUNCTION(BlueprintCallable)
	void SetTime(float InTime);

	UFUNCTION(BlueprintCallable)
	TArray<float> GetRadius() const;
	UFUNCTION(BlueprintCallable)
	void SetRadius(TArray<float> Radius);

	UFUNCTION(BlueprintCallable)
	float GetSizeZ() const;
	UFUNCTION(BlueprintCallable)
	void SetSizeZ(float height);

	UFUNCTION(BlueprintCallable)
	float GetRadiusDefault() const;
	UFUNCTION(BlueprintCallable)
	void SetRadiusDefault(float r);

	UFUNCTION(BlueprintCallable)
	int32 GetPointsNumberZ() const;
	UFUNCTION(BlueprintCallable)
	void SetPointsNumberZ(int32 number);

	UFUNCTION(BlueprintCallable)
	int32 GetPointsNumberR() const;
	UFUNCTION(BlueprintCallable)
	void SetPointsNumberR(int32 number);

	/// <summary>
	/// Ïà¶Ô×ø±ê
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void Clip(TArray<FVector> points, float clipZBlurThreshold);
	UFUNCTION(BlueprintCallable)
	void Reset();

	UFUNCTION(BlueprintCallable)
	float GetPercentOfInsideRadius() const;
	UFUNCTION(BlueprintCallable)
	void SetPercentOfInsideRadius(float percent);		
	
	UFUNCTION(BlueprintCallable)
	void ReCalculateConstructParams() ;

protected:

	void Initialize() override;
	FBoxSphereBounds GetBounds() override;
	bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData) override;
	bool IsThreadSafe() override;

private:
	void CalculateBounds();
	void DrawBody(TArray<float> RadiusList, FRuntimeMeshRenderableMeshData& MeshData, float lastZpos=0, float dz=0 , float dAlpha=0.314 , float offset=0, int32 index_offset =0 , int32 rStartIndex=0,bool isFaceOut = true , float areaCoefficient=1);
	void DrawHead(FRuntimeMeshRenderableMeshData& MeshData, int32 outsideStartIndex, int32 outsideEndIndex, int32 insideStartIndex, int32 insideEndIndex);
	void Fill(FRuntimeMeshRenderableMeshData& MeshData, TArray<int32> lineTop, TArray<int32> lineBottom, bool isFaceOut);
	float CalculateHeightForPoint(float x, float y);	
	int32 TryGetMeshPointIndex(int32 zindex, int32 rindex, int z, float radius, float dAlpha, FRuntimeMeshRenderableMeshData& MeshData, bool isFirstRow, int32 indexOffset = 0);
	TArray<int32> FindNearestAndInRangeIndex(TArray<float>list,float value, float Threshold = 0,  int32 startIndex=0, int32 endIndex=-1);
};
