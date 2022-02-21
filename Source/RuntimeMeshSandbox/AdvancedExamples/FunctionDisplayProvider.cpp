// Fill out your copyright notice in the Description page of Project Settings.

#include "FunctionDisplayProvider.h"


UFunctionDisplayProvider::UFunctionDisplayProvider()
{
	MinX = -PI * 2; MaxX = PI * 2; MinY = MinX; MaxY = MaxX; MinValue = -1; MaxValue = 1;
	PointsSX = 128; PointsSY = PointsSX;
	SizeX = 1000; SizeY = 1000; SizeZ = 300;
	RadiusDefault = 500;
	PointsNumber_Z = SizeZ / 30;
	//PointsNumber_R = 2 * PI * RadiusDefault / 10;	
	PointsNumber_R = 360 / 10 ;
	/*RadiusList[PointsNumber_Z] = { RadiusDefault };*/
	PercentOfInsideRadius = 0.8;
	RadiusList.Init(RadiusDefault, PointsNumber_Z);	

	float dz = SizeZ / PointsNumber_Z;
	for (int32 i = 0; i < PointsNumber_Z; i++)
	{
		R_ZList.Add(i*dz);
	}

	
	//RadiusList[4] = RadiusDefault * 0.2;
	/*RadiusList[5] = RadiusDefault * 0.5;
	RadiusList[6] = RadiusDefault * 0.7;*/

	CalculateBounds();
}

void UFunctionDisplayProvider::ReCalculateConstructParams()
{	

	RadiusList.Init(RadiusDefault, PointsNumber_Z);
	float dz = SizeZ / PointsNumber_Z;
	R_ZList.Empty();
	for (int32 i = 0; i < PointsNumber_Z; i++)
	{
		R_ZList.Add(i * dz);
	}
	CalculateBounds();
}

UMaterialInterface* UFunctionDisplayProvider::GetDisplayMaterial() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return DisplayMaterial;
}
void UFunctionDisplayProvider::SetDisplayMaterial(UMaterialInterface* InMaterial)
{
	FScopeLock Lock(&PropertySyncRoot);
	DisplayMaterial = InMaterial;
}

float UFunctionDisplayProvider::GetTime() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return Time;
}
void UFunctionDisplayProvider::SetTime(float InTime)
{
	{
		FScopeLock Lock(&PropertySyncRoot);
		Time = InTime;
	}
	MarkLODDirty(0);
}

float UFunctionDisplayProvider::GetSizeZ() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return SizeZ;
}
void UFunctionDisplayProvider::SetSizeZ(float height)
{
	{
		FScopeLock Lock(&PropertySyncRoot);
		SizeZ = height;
	}
	MarkLODDirty(0);
}
float UFunctionDisplayProvider::GetRadiusDefault() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return RadiusDefault;
}
void UFunctionDisplayProvider::SetRadiusDefault(float r)
{
	{
		FScopeLock Lock(&PropertySyncRoot);
		RadiusDefault = r;
	}
	MarkLODDirty(0);
}
int32 UFunctionDisplayProvider::GetPointsNumberZ() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return PointsNumber_Z;
}
void UFunctionDisplayProvider::SetPointsNumberZ(int32 number)
{
	{
		FScopeLock Lock(&PropertySyncRoot);
		PointsNumber_Z = number;
	}
	MarkLODDirty(0);
}
int32 UFunctionDisplayProvider::GetPointsNumberR() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return PointsNumber_R;
}
void UFunctionDisplayProvider::SetPointsNumberR(int32 number)
{
	{
		FScopeLock Lock(&PropertySyncRoot);
		PointsNumber_R = number;
	}
	MarkLODDirty(0);
}

void UFunctionDisplayProvider::Reset() {
	RadiusList.Init(RadiusDefault, PointsNumber_Z);
	float dz = SizeZ / PointsNumber_Z;
	R_ZList.Empty();
	for (int32 i = 0; i < PointsNumber_Z; i++)
	{
		R_ZList.Add(i * dz);
	}
	CalculateBounds();
	MarkLODDirty(0);	
	
}

void UFunctionDisplayProvider::Clip(TArray<FVector> points, float clipZBlurThreshold)
{
	if (points.Num() == 0) {
		return;
	}

	for (int32 i = 0; i < points.Num(); i++)
	{
		FVector point = points[i];
		float distane_p_o =FMath::Sqrt(point.X * point.X + point.Y * point.Y);
		TArray<int32> nearbyZPointsIndex = FindNearestAndInRangeIndex(R_ZList, point.Z, clipZBlurThreshold, 0, PointsNumber_Z-1);

		for (int32 j = 0; j < nearbyZPointsIndex.Num(); j++)
		{
			int32 index = nearbyZPointsIndex[j];
			if (RadiusList[index] > distane_p_o) {
				RadiusList[index] = distane_p_o;
			}		 				
		}
	}

	MarkLODDirty(0);	
}

float UFunctionDisplayProvider::GetPercentOfInsideRadius() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return PercentOfInsideRadius;
}
void UFunctionDisplayProvider::SetPercentOfInsideRadius(float percent)
{
	{
		FScopeLock Lock(&PropertySyncRoot);
		PercentOfInsideRadius = percent;
	}
	MarkLODDirty(0);
}

TArray<float> UFunctionDisplayProvider::GetRadius() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return RadiusList;
}
void UFunctionDisplayProvider::SetRadius(TArray<float> Radius)
{
	{
		FScopeLock Lock(&PropertySyncRoot);
		RadiusList = Radius;
	}
	MarkLODDirty(0);
}

void UFunctionDisplayProvider::Initialize()
{
	SetupMaterialSlot(0, FName("Material"), DisplayMaterial);
	TArray<FRuntimeMeshLODProperties> LODs;
	FRuntimeMeshLODProperties LODProperties;
	LODProperties.ScreenSize = 0.0f;
	LODs.Add(LODProperties);
	ConfigureLODs(LODs);

	FRuntimeMeshSectionProperties Properties;
	Properties.bCastsShadow = true;
	Properties.bIsVisible = true;
	Properties.MaterialSlot = 0;
	Properties.bWants32BitIndices = true;
	Properties.UpdateFrequency = ERuntimeMeshUpdateFrequency::Frequent;
	CreateSection(0, 0, Properties);
}

FBoxSphereBounds UFunctionDisplayProvider::GetBounds()
{
	return LocalBounds;
}

bool UFunctionDisplayProvider::GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData& MeshData)
{

	//check(LODIndex == 0 && SectionId == 0);
	//float dx = (MaxX - MinX) / (float)PointsSX; //change of x between two points
	//float dy = (MaxY - MinY) / (float)PointsSY; //change of y between two points
	//for (int32 yindex = 0; yindex < PointsSY; yindex++)
	//{
	//	float yalpha = (float)yindex / (float)(PointsSY - 1);
	//	float ypos = FMath::Lerp(-SizeY / 2, SizeY / 2, yalpha);
	//	float yvalue = FMath::Lerp(MinY, MaxY, yalpha);
	//	for (int32 xindex = 0; xindex < PointsSX; xindex++)
	//	{
	//		float xalpha = (float)xindex / (float)(PointsSX - 1);
	//		float xpos = FMath::Lerp(-SizeX / 2, SizeX / 2, xalpha);
	//		float xvalue = FMath::Lerp(MinX, MaxX, xalpha);

	//		FVector Position(xpos, ypos,
	//			FMath::GetMappedRangeValueClamped(
	//				FVector2D(MinValue, MaxValue),
	//				FVector2D(-SizeZ / 2, SizeZ / 2),
	//				CalculateHeightForPoint(xvalue, yvalue)
	//				)
	//			);

	//		float dfdx = (CalculateHeightForPoint(xvalue + dx, yvalue) - CalculateHeightForPoint(xvalue - dx, yvalue)) / (2 * dx); //derivative of f over x
	//		float dfdy = (CalculateHeightForPoint(xvalue, yvalue + dy) - CalculateHeightForPoint(xvalue, yvalue - dy)) / (2 * dy); //derivative of f over y

	//		FVector Normal(-dfdx, -dfdy, 1);
	//		Normal.Normalize();
	//		FVector Tangent(1, 0, dfdx);
	//		Tangent.Normalize();

	//		FVector2D UV(xalpha, yalpha);

	//		MeshData.Positions.Add(Position);
	//		MeshData.Tangents.Add(Normal, Tangent);
	//		MeshData.TexCoords.Add(UV);
	//		MeshData.Colors.Add(FColor::White);

	//		if (xindex != PointsSX - 1 && yindex != PointsSY - 1)
	//		{
	//			int32 AIndex = xindex + yindex * PointsSX;
	//			int32 BIndex = AIndex + 1;
	//			int32 CIndex = AIndex + PointsSX;
	//			int32 DIndex = CIndex + 1;
	//			MeshData.Triangles.AddTriangle(AIndex, CIndex, BIndex);
	//			MeshData.Triangles.AddTriangle(BIndex, CIndex, DIndex);
	//		}
	//	}
	//}
	//return true;

	check(LODIndex == 0 && SectionId == 0);
	R_ZList.Empty();
	float dz = SizeZ / PointsNumber_Z;
	float dAlpha = 2 * PI / PointsNumber_R;
	float offsetZ = 0.0f;
	float lastZpos = 0;
	int32 insideStartIndex = 0;
	float areaCoefficient = PercentOfInsideRadius;
	if (RadiusList.Num() <= 1) {
		return true;
	}
	// 外侧
	DrawBody(RadiusList, MeshData, lastZpos, dz, dAlpha, offsetZ, 0, 0, true, 1 - areaCoefficient * areaCoefficient);

	//S = PI * R^2 - PI *r^2
	//r = k*R 
	//=> S = PI*R*R *( 1 - k*k )
	//=> r_outside = S_out/S = 1/(1-k*k)
	//=> r_inside = S_out/S  ;  S_out* k*k = S_in  
	//            =  S_in/(S*k*k)
	//            = (r*r)/(k*k*(R*R-r*r) )
	//            =  1/ [ (k)^2 * ( {R/r}^2 -1 )  ]

	// 里侧
	TArray<float> insideRadiusList;
	for (int32 i = 0; i < RadiusList.Num(); i++)
	{
		insideRadiusList.Add(RadiusList[i] * areaCoefficient);
	}
	/*int32 index_offset = zStartIndex * PointsNumber_R;*/
	insideStartIndex = MeshData.Positions.Num();
	DrawBody(insideRadiusList, MeshData, lastZpos, dz, dAlpha, offsetZ, insideStartIndex, 0, false, (1 - areaCoefficient * areaCoefficient) / (areaCoefficient * areaCoefficient));



	//上下
	DrawHead(MeshData, 0,insideStartIndex-1, insideStartIndex, MeshData.Positions.Num()-1);

	return true;



	int xpos = 500;
	int ypos = 0;
	int z = 100;
	FVector Position(xpos, ypos,
		FMath::GetMappedRangeValueClamped(
			FVector2D(MinValue, MaxValue),
			FVector2D(-SizeZ / 2, SizeZ / 2),
			z
		)
	);
	MeshData.Positions.Add(Position);
	FVector Normal(0, 1, 0);
	Normal.Normalize();
	FVector Tangent(1, 1, 1);
	Tangent.Normalize();
	MeshData.Tangents.Add(Normal, Tangent);
	MeshData.TexCoords.Add(FVector2D(1, 1));
	MeshData.Colors.Add(FColor::White);


	xpos = 500;
	ypos = 0;
	z = -100;
	FVector Position2(xpos, ypos,
		FMath::GetMappedRangeValueClamped(
			FVector2D(MinValue, MaxValue),
			FVector2D(-SizeZ / 2, SizeZ / 2),
			z
		)
	);
	MeshData.Positions.Add(Position2);
	FVector Normal2(0, 1, 0);
	Normal2.Normalize();
	FVector Tangent2(1, 1, 1);
	Tangent2.Normalize();
	MeshData.Tangents.Add(Normal2, Tangent2);
	MeshData.TexCoords.Add(FVector2D(1, 1));
	MeshData.Colors.Add(FColor::White);

	xpos = 0;
	ypos = 0;
	z = 100;
	FVector Position3(xpos, ypos,
		FMath::GetMappedRangeValueClamped(
			FVector2D(MinValue, MaxValue),
			FVector2D(-SizeZ / 2, SizeZ / 2),
			z
		)
	);
	MeshData.Positions.Add(Position3);
	FVector Normal3(0, 1, 0);
	Normal3.Normalize();
	FVector Tangent3(1, 1, 1);
	Tangent3.Normalize();
	MeshData.Tangents.Add(Normal3, Tangent3);
	MeshData.TexCoords.Add(FVector2D(1, 1));
	MeshData.Colors.Add(FColor::White);





	MeshData.Triangles.AddTriangle(0, 1, 2);
	MeshData.Triangles.AddTriangle(0, 2, 1);
	return true;


	float dx = (MaxX - MinX) / (float)PointsSX; //change of x between two points
	float dy = (MaxY - MinY) / (float)PointsSY; //change of y between two points
	for (int32 yindex = 0; yindex < PointsSY; yindex++)
	{
		float yalpha = (float)yindex / (float)(PointsSY - 1);
		float ypos = FMath::Lerp(-SizeY / 2, SizeY / 2, yalpha);
		float yvalue = FMath::Lerp(MinY, MaxY, yalpha);
		for (int32 xindex = 0; xindex < PointsSX; xindex++)
		{
			float xalpha = (float)xindex / (float)(PointsSX - 1);
			float xpos = FMath::Lerp(-SizeX / 2, SizeX / 2, xalpha);
			float xvalue = FMath::Lerp(MinX, MaxX, xalpha);

			FVector Position(xpos, ypos,
				FMath::GetMappedRangeValueClamped(
					FVector2D(MinValue, MaxValue),
					FVector2D(-SizeZ / 2, SizeZ / 2),
					CalculateHeightForPoint(xvalue, yvalue)
				)
			);

			float dfdx = (CalculateHeightForPoint(xvalue + dx, yvalue) - CalculateHeightForPoint(xvalue - dx, yvalue)) / (2 * dx); //derivative of f over x
			float dfdy = (CalculateHeightForPoint(xvalue, yvalue + dy) - CalculateHeightForPoint(xvalue, yvalue - dy)) / (2 * dy); //derivative of f over y

			FVector Normal(-dfdx, -dfdy, 1);
			Normal.Normalize();
			FVector Tangent(1, 0, dfdx);
			Tangent.Normalize();

			FVector2D UV(xalpha, yalpha);

			MeshData.Positions.Add(Position);
			MeshData.Tangents.Add(Normal, Tangent);
			MeshData.TexCoords.Add(UV);
			MeshData.Colors.Add(FColor::White);

			if (xindex != PointsSX - 1 && yindex != PointsSY - 1)
			{
				int32 AIndex = xindex + yindex * PointsSX;
				int32 BIndex = AIndex + 1;
				int32 CIndex = AIndex + PointsSX;
				int32 DIndex = CIndex + 1;
				MeshData.Triangles.AddTriangle(AIndex, CIndex, BIndex);
				MeshData.Triangles.AddTriangle(BIndex, CIndex, DIndex);
			}
		}
	}
	return true;
}

void UFunctionDisplayProvider::DrawBody(TArray<float> RadiusList, FRuntimeMeshRenderableMeshData& MeshData, float lastZpos, float dz, float dAlpha, float offsetZ, int32 index_offset, int32 rStartIndex, bool isFaceOut, float areaCoefficient) {

	float accumulateOffsetZ = 0;
	float V_inDebt = 0;

	for (int32 zindex = 1; zindex < PointsNumber_Z; zindex++)
	{
		float radius = RadiusList[zindex];
		if (V_inDebt > 0) {
			float increaseH = V_inDebt / (PI * radius * radius * areaCoefficient); // 需要增加的高度偏移		
			//TODO: check this damage number
			increaseH = FMath::Min(dz * 10* PercentOfInsideRadius, increaseH);
			accumulateOffsetZ += increaseH;
			V_inDebt = 0;
			//UE_LOG(LogTemp, Warning, TEXT("increaseH: %s - %s"), increaseH,accumulateOffsetZ);
		}

		float zpos = dz * zindex + accumulateOffsetZ + offsetZ;
		R_ZList.Add(zpos);
		//int32 zindex = _zindex + zStartIndex;

		if (radius < RadiusDefault) {
			float reduceV = dz * (PI * RadiusDefault * RadiusDefault - PI * radius * radius * areaCoefficient); // 减少的体积
			V_inDebt += reduceV;
			//UE_LOG(LogTemp, Warning, TEXT("reduceV: %s  - %s"), reduceV,V_inDebt);									
		}

		// 绘制一圈
		//TODO: 优化	
		// 逆时针 
		// Z UP 0-N
		// B --  D 
		// |  \     
		// A --  C
		// 三角 -  出队
		// ACB  -  A
		// BCD  -  B
		// C => A  D=>B
		TQueue<int32> pointsStack;
		int32 pindexLastRound;
		int32 pindexCurrentRound;
		int32 pindex_A;
		int32 pindex_B;
		int32 pindex_C;
		int32 flag = 0;
		for (int32 rindex = 0; rindex < PointsNumber_R; rindex++)
		{
			// Last Round
			//A
			pindexLastRound = this->TryGetMeshPointIndex(zindex - 1, rindex, lastZpos, radius, dAlpha, MeshData, zindex - 1 <= 1, index_offset);
			pointsStack.Enqueue(pindexLastRound);
			flag++;
			if (flag >= 3)
			{
				//append triangle				
				pointsStack.Dequeue(pindex_A);
				pindex_B = pindexCurrentRound;
				pindex_C = pindexLastRound;

				//MeshData.Triangles.AddTriangle(0, 1, 2);
				//MeshData.Triangles.AddTriangle(pindex_A, pindex_B, pindex_C);
				if (isFaceOut) {
					MeshData.Triangles.AddTriangle(pindex_A, pindex_C, pindex_B);
				}
				else {
					MeshData.Triangles.AddTriangle(pindex_A, pindex_B, pindex_C);
				}
			}

			// Current Round
			// B
			pindexCurrentRound = this->TryGetMeshPointIndex(zindex, rindex, zpos, radius, dAlpha, MeshData, zindex <= 1, index_offset);
			pointsStack.Enqueue(pindexCurrentRound);
			flag++;
			if (flag >= 3)
			{
				//append triangle				
				pointsStack.Dequeue(pindex_B);
				pindex_A = pindexCurrentRound; // 其实就是D				
				pindex_C = pindexLastRound;

				//MeshData.Triangles.AddTriangle(0, 1, 2);
				//MeshData.Triangles.AddTriangle(pindex_B, pindex_A, pindex_C);
				if (isFaceOut) {
					MeshData.Triangles.AddTriangle(pindex_B, pindex_C, pindex_A);
				}
				else {
					MeshData.Triangles.AddTriangle(pindex_B, pindex_A, pindex_C);
				}
			}
			// 封口
			if (rindex == PointsNumber_R - 1) {

				int32 pStartA = this->TryGetMeshPointIndex(zindex - 1, 0, lastZpos, radius, dAlpha, MeshData, zindex - 1 <= 1, index_offset); //c				
				int32 pStartB = this->TryGetMeshPointIndex(zindex, 0, zpos, radius, dAlpha, MeshData, zindex <= 1, index_offset); //d				
				int32 endA = pindexLastRound;  // A
				int32 endB = pindexCurrentRound; // B
				if (isFaceOut) {
					MeshData.Triangles.AddTriangle(endA, pStartA, endB);
					MeshData.Triangles.AddTriangle(endB, pStartA, pStartB);
				}
				else {
					MeshData.Triangles.AddTriangle(endA, endB, pStartA);
					MeshData.Triangles.AddTriangle(endB, pStartB, pStartA);
				}

			}

		}

		lastZpos = zpos;

	}

}


void UFunctionDisplayProvider::DrawHead(FRuntimeMeshRenderableMeshData& MeshData,int32 outsideStartIndex, int32 outsideEndIndex, int32 insideStartIndex,int32 insideEndIndex) {

	TArray<int32> LineOut;
	TArray<int32> LineInside;
	int32 lastIndex = MeshData.Positions.Num()-1;
	//top
	for (int32 i = 0; i < PointsNumber_R; i++)
	{
		LineOut.Add(outsideEndIndex - PointsNumber_R +i+1);
		LineInside.Add(insideEndIndex - PointsNumber_R + i + 1);
	}
	Fill(MeshData, LineOut, LineInside, false);



	//bottom
	LineOut.Empty(LineOut.Num());
	LineInside.Empty(LineInside.Num());
	for (int32 i = 0; i < PointsNumber_R; i++)
	{
		LineOut.Add(i * 2+ outsideStartIndex);
		LineInside.Add(i * 2 + insideStartIndex);
	}
	Fill(MeshData, LineOut, LineInside, true);


}

void UFunctionDisplayProvider::Fill(FRuntimeMeshRenderableMeshData& MeshData, TArray<int32> lineTop, TArray<int32> lineBottom, bool isFaceOut)
{
	// 绘制一圈
	//TODO: 优化	
	// 逆时针 
	// Z UP 0-N
	// B --  D 
	// |  \     
	// A --  C
	// 三角 -  出队
	// ACB  -  A
	// BCD  -  B
	// C => A  D=>B
	TQueue<int32> pointsStack;
	int32 pindexLastRound;
	int32 pindexCurrentRound;
	int32 pindex_A;
	int32 pindex_B;
	int32 pindex_C;
	int32 flag = 0;
	for (int32 i = 0; i < lineTop.Num(); i++)
	{
		// Last Round
		//A
		/*pindexLastRound = this->TryGetMeshPointIndex(zindex - 1, rindex, lastZpos, radius, dAlpha, MeshData, zindex - 1 <= 1, index_offset);*/
		pindexLastRound = lineBottom[i];
		pointsStack.Enqueue(pindexLastRound);
		flag++;
		if (flag >= 3)
		{
			//append triangle				
			pointsStack.Dequeue(pindex_A);
			pindex_B = pindexCurrentRound;
			pindex_C = pindexLastRound;

			//MeshData.Triangles.AddTriangle(0, 1, 2);
			//MeshData.Triangles.AddTriangle(pindex_A, pindex_B, pindex_C);
			if (isFaceOut) {
				MeshData.Triangles.AddTriangle(pindex_A, pindex_C, pindex_B);
			}
			else {
				MeshData.Triangles.AddTriangle(pindex_A, pindex_B, pindex_C);
			}
		}

		// Current Round
		// B
		//pindexCurrentRound = this->TryGetMeshPointIndex(zindex, rindex, zpos, radius, dAlpha, MeshData, zindex <= 1, index_offset);
		pindexCurrentRound = lineTop[i];
		pointsStack.Enqueue(pindexCurrentRound);
		flag++;
		if (flag >= 3)
		{
			//append triangle				
			pointsStack.Dequeue(pindex_B);
			pindex_A = pindexCurrentRound; // 其实就是D				
			pindex_C = pindexLastRound;

			//MeshData.Triangles.AddTriangle(0, 1, 2);
			//MeshData.Triangles.AddTriangle(pindex_B, pindex_A, pindex_C);
			if (isFaceOut) {
				MeshData.Triangles.AddTriangle(pindex_B, pindex_C, pindex_A);
			}
			else {
				MeshData.Triangles.AddTriangle(pindex_B, pindex_A, pindex_C);
			}
		}
		// 封口
		if (i == lineTop.Num() - 1) {

			int32 pStartA = lineBottom[0];
			int32 pStartB = lineTop[0];
			int32 endA = pindexLastRound;  // A
			int32 endB = pindexCurrentRound; // B
			if (isFaceOut) {
				MeshData.Triangles.AddTriangle(endA, pStartA, endB);
				MeshData.Triangles.AddTriangle(endB, pStartA, pStartB);
			}
			else {
				MeshData.Triangles.AddTriangle(endA, endB, pStartA);
				MeshData.Triangles.AddTriangle(endB, pStartB, pStartA);
			}

		}

	}


}




bool UFunctionDisplayProvider::IsThreadSafe()
{
	return true;
}

void UFunctionDisplayProvider::CalculateBounds()
{
	FVector Ext(SizeX, SizeY, SizeZ);
	FBox Box = FBox(-Ext / 2, Ext / 2);
	LocalBounds = FBoxSphereBounds(Box);
}

float UFunctionDisplayProvider::CalculateHeightForPoint(float x, float y)
{
	/*return FMath::Sin(sqrt(x * x + y * y) + Time);*/
	return FMath::Sin(sqrt(x * x + y * y));
}


int32 UFunctionDisplayProvider::TryGetMeshPointIndex(int32 zindex, int32 rindex, int z, float radius, float dAlpha, FRuntimeMeshRenderableMeshData& MeshData, bool isFirstRow, int32 indexOffset) {
	int32 index;
	if (!isFirstRow) { //后边几行
		index = zindex * PointsNumber_R + rindex;
	}
	else { //头一行, 交错的
		index = zindex + rindex * 2;
	}
	index += indexOffset;

	if (index >= MeshData.Positions.Num()) {
		//append 								
		float xpos = radius * FMath::Sin(rindex * dAlpha);
		float b = FMath::Cos(rindex * dAlpha);
		float ypos = radius * FMath::Cos(rindex * dAlpha);
		float zpos = z;
		/*	FVector Position(xpos, ypos,
				FMath::GetMappedRangeValueClamped(
					FVector2D(MinValue, MaxValue),
					FVector2D(0, SizeZ),
					z
				)
			);*/
		FVector Position(xpos, ypos, zpos);
		MeshData.Positions.Add(Position);

		FVector Normal(xpos, ypos, -zpos);
		Normal.Normalize();
		/*设
			P 1 P 2 -> = (x1, y1, z1) =>  n = (xpos,ypos,-zpos)
			P 1 P 3 -> = (x2, y2, z2) =>  z = (0,0,1)
			tangent -> 即

			x = y1z2-y2z1  = y1
			y = z1x2-z2x1 = -x1
			z = x1y2-x2y1 = 0
			*/
		FVector Tangent(ypos, -xpos, 0);
		Tangent.Normalize();
		MeshData.Tangents.Add(Normal, Tangent);

		float xalpha = rindex + 1 / PointsNumber_R;
		float yalpha = zindex + 1 / PointsNumber_Z;
		FVector2D UV(xalpha, yalpha);
		MeshData.TexCoords.Add(UV);
		MeshData.Colors.Add(FColor::White);

		//FVector Normal(0, 1, 0);
		//Normal.Normalize();
		//FVector Tangent(1, 1, 1);
		//Tangent.Normalize();
		//MeshData.Tangents.Add(Normal, Tangent);
		//MeshData.TexCoords.Add(FVector2D(1, 1));
		//MeshData.Colors.Add(FColor::White);
	}
	return index;
}

TArray<int32> UFunctionDisplayProvider::FindNearestAndInRangeIndex(TArray<float>list, float value, float Threshold,int32 startIndex,int32 endIndex) {
	
	if (endIndex < 0||endIndex>=list.Num()) {
		endIndex = list.Num()-1;
	}

	int32 nearleastIndex = 0;
	float nearleastValue = -1;
	TArray<int32> res;

	for (int32 i = startIndex; i < endIndex+1; i++)
	{
		float item = list[i];
		float diff = FMath::Abs(item - value);
		//UE_LOG(LogTemp, Warning, TEXT("diff: %f"),diff);
		if (diff < nearleastValue|| nearleastValue==-1) {
			nearleastIndex = i;
			nearleastValue = diff;
			//UE_LOG(LogTemp, Warning, TEXT("nearleastValue: %f"), nearleastValue);
		}
		if (diff < Threshold) {
			res.Add(i);
		}
	}

	if (!res.Contains(nearleastIndex)) {
		res.Add(nearleastIndex);
	}

	return res;
}

