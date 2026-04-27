// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectSoccer/Entities/Field/ArcMovementComponent.h"

DEFINE_LOG_CATEGORY(LogArcMovementComponent);

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Solves the equation: ax^2 + bx + c
///		@param a : First constant.
///		@param b : Second constant.
///		@param c : Third constant.
///		@param solution1 : The first solution to the quadratic equation. Will be invalid if the function returns 0.
///		@param solution2 : The second solution to the quadratic equation. Will be invalid if the function returns <= 1.
///		@returns : Number of valid Solutions.
//----------------------------------------------------------------------------------------------------
static int SolveQuadratic(const double a, const double b, const double c, double& solution1, double& solution2)
{
	solution1 = NAN;
    solution2 = NAN;

    // Normal form: x^2 + px + q = 0
    const double p = b / (2.0 * a);
    const double q = c / a;
    const double D = p * p - q;

    // One solution:
    if (FMath::IsNearlyZero(D))
    {
        solution1 = -p;
        return 1;
    }

    // Zero solutions:
    if (D < 0.0)
    {
        return 0;
    }

    // 2 Solutions:
    const double sqrtD = FMath::Sqrt(D);
    solution1 = -p + sqrtD;
    solution2 = -p - sqrtD;
    return 2;
}

static bool SolveBallisticArcFixedLateralSpeed(const FVector& start, const FVector& target, const FVector& targetVelocity, const float lateralSpeed, const float maxHeightOffset, FVector& initialVelocity, float& gravity, FVector& impactPoint, float& time)
{
    // Source:
    // https://github.com/forrestthewoods/lib_fts/blob/master/code/fts_ballistic_trajectory.cs

    check(start != target && lateralSpeed > 0);

    initialVelocity = FVector::ZeroVector;
    gravity = NAN;
    impactPoint = FVector::ZeroVector;

    // Ground Plane Terms
    FVector targetVelocityXY = FVector(targetVelocity.X, targetVelocity.Y, 0.f);
    FVector toXY = target - start;
    toXY.Z = 0.f;

    // Derivation
	//   (1) Base formula: |P + V*t| = S*t
    //   (2) Substitute variables: |toXY + targetVelocityXY*t| = S*t
    //   (3) Square both sides: Dot(toXY,toXY) + 2*Dot(toXY, targetVelocityXY)*t + Dot(targetVelocityXY, targetVelocityXY)*t^2 = S^2 * t^2
	//   (4) Quadratic: (Dot(targetVelocityXY,targetVelocityXY) - S^2)t^2 + (2*Dot(toXY, targetVelocityXY))*t + Dot(toXY, toXY) = 0
    float a = FVector::DotProduct(targetVelocityXY, targetVelocityXY) - lateralSpeed * lateralSpeed;
    float b = 2.f * FVector::DotProduct(toXY, targetVelocityXY);
    float c = FVector::DotProduct(toXY, toXY);
    double timeSolution0, timeSolution1;
    const int numSolutions = SolveQuadratic(a, b, c, timeSolution0, timeSolution1);

    // Check for valid solutions
    const bool valid0 = numSolutions > 0 && timeSolution0 > 0;
    const bool valid1 = numSolutions > 1 && timeSolution1 > 0;

    // No Valid Solutions:
    if (!valid0 && !valid1)
    {
        return false;
    }

    if (valid0 && valid1)
    {
        time = FMath::Min(timeSolution0, timeSolution1);
    }

    else
    {
        time = valid0 ? timeSolution0 : timeSolution1;
    }

    // Calculate the impact point
    impactPoint = target + targetVelocity * time;

    // Calculate the initial velocity on the XY plane
    FVector to = impactPoint - start;
    to.Z = 0.f;
    initialVelocity = to.GetSafeNormal() * lateralSpeed;

    a = start.Z;                                                   // Initial Height
    b = FMath::Max(start.Z, impactPoint.Z) + maxHeightOffset; // Peak Height
    c = impactPoint.Z;                                             // Final Height

    gravity = -4 * (a - 2 * b + c) / (time * time);
    initialVelocity.Z = -(3 * a - 4 * b + c) / time;

    return true;
}

UArcMovementComponent::UArcMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bTickBeforeOwner = true;
    PrimaryComponentTick.TickInterval = 0.1 / 1000; // 0.1ms 
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Stop any arc simulation that is currently happening, and sets the Component Velocity to 0.
//----------------------------------------------------------------------------------------------------
void UArcMovementComponent::CancelArc()
{
    //UE_LOG(LogArcMovementComponent, Log, TEXT("Cancelling Arc."));
    CurrentTime = 0.f;
    
    StopMovementImmediately();
    SetComponentTickEnabled(false);
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Stops the physics simulation of the UpdatedComponent and begins moving it along an arc path.
///		@param params : Parameters to build the arced path.
///		@returns : False if there is no UpdatedComponent or there is an issue with the final destination (todo).
//----------------------------------------------------------------------------------------------------
bool UArcMovementComponent::BeginMoveAlongArc(const FMoveAlongArcParams& params, float& duration)
{
    // If the updated component is null
    if (!UpdatedComponent)
    {
        return false;
    }

    // Stop the physics simulation of the UpdatedComponent, we are going to move it manually.
	StopMovementImmediately();
    UPrimitiveComponent* pPrimitive = Cast<UPrimitiveComponent>(UpdatedComponent);
    if (pPrimitive)
    {
        pPrimitive->SetSimulatePhysics(false);
    }

    // Set the parameters for the arc path.
    StartLocation = params.StartLocation;
    CurrentTime = 0.f;

    float gravity;
    FVector impactPoint;
	[[maybe_unused]] const bool isValid = SolveBallisticArcFixedLateralSpeed(StartLocation, params.TargetLocation, params.InitialVelocity, params.Speed, params.MaxHeightOffset, InitialVelocity, gravity, impactPoint, Duration);
	check(isValid);

    //LastLocation = StartLocation;
    Acceleration = -gravity * FVector::UpVector;
    //GEngine->AddOnScreenDebugMessage(45, 5.f, FColor::Cyan, FString::Printf(TEXT("Duration: %f"), Duration));
    //GEngine->AddOnScreenDebugMessage(47, 5.f, FColor::Yellow, FString::Printf(TEXT("Acceleration: %s"), *Acceleration.ToString()));
    //GEngine->AddOnScreenDebugMessage(48, 5.f, FColor::Red, FString::Printf(TEXT("StartLocation.Z: %f | EndLocation.Z %f"), StartLocation.Z, impactPoint.Z));

    // Begin our Tick, which moves the UpdatedComponent along the arc path.
    SetComponentTickEnabled(true);

    duration = Duration;

    return true;
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Returns the Delegate for when the ArcMovement has finished or been interrupted. This delegate
///             returns a boolean value. If it returns true, then Physics will be turned back on.
//----------------------------------------------------------------------------------------------------
UArcMovementComponent::FOnArcMovementFinishedOrInterrupted& UArcMovementComponent::OnArcMovementFinishedOrInterrupted()
{
    return OnArcMovementFinishedOrInterruptedDelegate;
}

void UArcMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
    if (NewUpdatedComponent == nullptr)
    {
        Super::SetUpdatedComponent(NewUpdatedComponent);
        return;
    }

    UPrimitiveComponent* pPrimitive = Cast<UPrimitiveComponent>(NewUpdatedComponent);
    if (!pPrimitive)
    {
        UE_LOG(LogArcMovementComponent, Error, TEXT("UpdatedComponent must be a child of UPrimitiveComponent!"));
        return;
    }

    Super::SetUpdatedComponent(NewUpdatedComponent);
}

void UArcMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
    if (!UpdatedComponent)
    {
        EndArcSimulation();
        return;
    }

    // Update the Current Time.
    CurrentTime += DeltaTime;
    if (CurrentTime >= Duration)
    {
        EndArcSimulation();
        return;
    }

    const float t = CurrentTime / Duration;
    const FVector currentLocation = UpdatedComponent->GetComponentLocation();

    // Calculate the new Velocity:
    Velocity = InitialVelocity + (Acceleration * (t * t));
    //GEngine->AddOnScreenDebugMessage(46, 5.f, FColor::Cyan, FString::Printf(TEXT("Velocity: %s"), *Velocity.ToString()));
	const FVector newLocation = currentLocation + Velocity * DeltaTime;

    // If we hit something, stop the simulation:
    FHitResult hit;
    UpdatedComponent->SetWorldLocation(newLocation, true, &hit);
    // TODO: How are we going to handle walls - Do we care?
    if (hit.GetActor() != nullptr)
    {
        UE_LOG(LogArcMovementComponent, Log, TEXT("Hit an actor, ending Arc."));
	    EndArcSimulation();
    }
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Return the UpdatedComponent to its physics state, and unregister our Tick.
//----------------------------------------------------------------------------------------------------
void UArcMovementComponent::EndArcSimulation()
{
    CurrentTime = 0.f;
	SetComponentTickEnabled(false);
    bool bReenablePhysics = true;

    if (OnArcMovementFinishedOrInterruptedDelegate.IsBound())
    {
        bReenablePhysics = OnArcMovementFinishedOrInterruptedDelegate.Execute();
    }

    // Return the UpdatedPrimitive to its physics state.
    if (bReenablePhysics)
    {
        
        if (auto pPrimitive = Cast<UPrimitiveComponent>(UpdatedComponent))
        {
            pPrimitive->SetSimulatePhysics(true);
        }
    }

    UE_LOG(LogArcMovementComponent, Log, TEXT("Ending Arc, Physics Re-enabled: %s"), (bReenablePhysics? TEXT("True") : TEXT("False")));
}

//static double GetCubicRoot(const double value)
//{
//	if (value > 0.0)
//	{
//        return FMath::Pow(value, 1.0 / 3.0);
//	}
//
//    if (value < 0.0)
//    {
//        return -FMath::Pow(-value, 1.0 / 3.0);
//    }
//
//    return 0.0;
//}

#pragma region Cubic and Quartic Solvers
//static int SolveCubic(const double a, const double b, const double c, const double d, double& solution1, double& solution2, double& solution3)
//{
//    solution1 = NAN;
//    solution2 = NAN;
//    solution3 = NAN;
//
//    // Normal form: x^3 + ax^2 + bx + c = 0
//    const double A = b / a;
//    const double B = c / a;
//    const double C = d / a;
//
//    // Substitute x = y - A/3 to eliminate quadric term: x^3 + px + q = 0
//    const double squareA = A * A;
//    const double p = 1.0 / 3.0 * (-1.0 / 3.0 * squareA + B);
//    const double q = 1.0 / 2.0 * (2.0 / 27.0 * A * squareA - 1.0 / 3.0 * A * B + C);
//
//    // Use Cardano's formula
//    const double cubeP = p * p * p;
//    const double D = q * q + cubeP;
//
//    if (FMath::IsNearlyZero(D))
//    {
//        // Single Solution:
//        if (FMath::IsNearlyZero(q))
//        {
//            solution1 = 0.0;
//            return 1;
//        }
//
//        const double u = GetCubicRoot(-q);
//        solution1 = 2.0 * u;
//        solution2 = -u;
//        return 2;
//    }
//
//    if (D < 0.0)
//    {
//        const double phi = 1.0 / 3.0 * FMath::Acos(-q / FMath::Sqrt(-cubeP));
//        const double t = 2.0 * FMath::Sqrt(-p);
//
//        solution1 = t * FMath::Cos(phi / 3.0);
//        solution2 = -t * FMath::Cos((phi + PI) / 3.0);
//        solution3 = -t * FMath::Cos((phi - PI) / 3.0);
//        return 3;
//    }
//
//    const double sqrtD = FMath::Sqrt(D);
//    const double u = GetCubicRoot(-q + sqrtD);
//    const double v = GetCubicRoot(q + sqrtD);
//
//    solution1 = u - v;
//    return 1;
//}
//
//static int SolveQuartic(const double a, const double b, const double c, const double d, const double e, double& solution1, double& solution2, double& solution3, double& solution4)
//{
//    solution1 = NAN;
//    solution2 = NAN;
//    solution3 = NAN;
//    solution4 = NAN;
//
//    double coefficients[4] = {};
//    int num = 0;
//
//    // Normal form: x^4 + ax^3 + bx^2 + cx + d = 0
//    const double A = b / a;
//    const double B = c / a;
//    const double C = d / a;
//    const double D = e / a;
//
//    // Substitute x = y - A/4 to eliminate cubic term: x^4 + px^2 + qx + r = 0
//    const double squareA = A * A;
//    const double p = -3.0 / 8.0 * squareA + B;
//    const double q = 1.0 / 8.0 * squareA * A - 1.0 / 2.0 * A * B + C;
//    const double r = -3.0 / 256.0 * squareA * squareA + 1.0 / 16.0 * squareA * B - 1.0 / 4.0 * A * C + D;
//
//    if (FMath::IsNearlyZero(r))
//    {
//        // No Absolute Term: y(y^3 + py + q) = 0
//        coefficients[3] = q;
//        coefficients[2] = p;
//        coefficients[1] = 0.0;
//        coefficients[0] = 1.0;
//
//        num = SolveCubic(coefficients[0], coefficients[1], coefficients[2], coefficients[3], solution1, solution2, solution3);
//    }
//
//    else
//    {
//	    // Solve the Resolvent Cubic
//        coefficients[3] = 1.0 / 2.0 * r * p - 1.0 / 8.0 * q * q;
//        coefficients[2] = -r;
//        coefficients[1] = -1.0 / 2.0 * p;
//        coefficients[0] = 1.0;
//
//        SolveCubic(coefficients[0], coefficients[1], coefficients[2], coefficients[3], solution1, solution2, solution3);
//
//        // only take the one real solution
//        const double z = solution1;
//
//        // Build two quadratic equations
//        double u = z * z - r;
//        double v = 2.0 * z - p;
//
//        if (FMath::IsNearlyZero(u))
//        {
//            u = 0.0;
//        }
//        else if (u > 0.0)
//        {
//            u = FMath::Sqrt(u);
//        }
//        else
//        {
//            return 0;
//        }
//
//        if (FMath::IsNearlyZero(v))
//        {
//            v = 0.0;
//        }
//        else if (v > 0.0)
//        {
//            v = FMath::Sqrt(v);
//        }
//        else
//        {
//            return 0;
//        }
//
//        coefficients[2] = z - u;
//        coefficients[1] = q < 0.0 ? -v : v;
//        coefficients[0] = 1.0;
//
//        num = SolveQuadratic(coefficients[0], coefficients[1], coefficients[2], solution1, solution2);
//
//        coefficients[2] = z + u;
//        coefficients[1] = q < 0.0 ? v : -v;
//        coefficients[0] = 1.0;
//
//        if (num == 0)
//        {
//            num += SolveQuadratic(coefficients[0], coefficients[1], coefficients[2], solution1, solution2);
//        }
//
//        else if (num == 1)
//        {
//            num += SolveQuadratic(coefficients[0], coefficients[1], coefficients[2], solution2, solution3);
//        }
//
//        else if (num == 2)
//        {
//            num += SolveQuadratic(coefficients[0], coefficients[1], coefficients[2], solution3, solution4);
//        }
//    }
//
//    // Resubstitute
//    const double sub = 1.0 / 4.0 * A;
//
//    if (num > 0)
//        solution1 -= sub;
//    else if (num > 1)
//        solution2 -= sub;
//    else if (num > 2)
//        solution3 -= sub;
//    else if (num > 3)
//        solution4 -= sub;
//
//    return num;
//}
#pragma endregion

//static float MaxBallisticRange(const float speed, const float gravity, const float initialHeight)
//{
//    check(speed > 0.0f && gravity > 0.f && initialHeight >= 0);
//
//    // How to Derive:
//    // 1) x = speed * time * cos(theta)
//    // 2) y = initialHeight + speed * time * sin(theta) - 0.5 * gravity * time^2
//    // 3) via quadratic: t = (speed * sin(theta)/gravity + sqrt(speed^2 * sin(theta)^2 + 2 * gravity * initialHeight))/gravity
//    // 4) Solution: range = (speed * cos(theta))/gravity * sqrt(speed^2 * sin(theta)^2 + 2 * gravity * initialHeight)
//    //constexpr float kAngle = FMath::DegreesToRadians(45.f);
//    constexpr float kSinCosTheta = 0.70710678118654752440084436210485f; // FMath::Cos(45);
//
//    const float range = (speed * kSinCosTheta / gravity) * (speed * kSinCosTheta + FMath::Sqrt(speed * speed * kSinCosTheta * kSinCosTheta + 2 * gravity * initialHeight));
//    return range;
//}