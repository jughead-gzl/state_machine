#pragma once

#include "structure.h"

#include <array>
#include <stdint.h>
#include <vector>

template <typename T>
using Odometry = Point6D<T>;

template <typename T>
using PathPoint = Point6D<T>;

template <typename T>
class PlanningPath
{
private:
    Timestamp timestamp_;
    std::vector<PathPoint<T>> path_points_;
};

using UUID = uint64_t;

enum class ObstacleType : uint8_t
{
    UNKNOWN_0,
};

template <typename T>
class Obstacle
{
public:
    using PredictPath = std::vector<PathPoint<T>>;
private:
    Timestamp timestamp_;
    UUID id_{0};
    ObstacleType type_{ObstacleType::UNKNOWN_0};
    bool is_dynamic_{false};
    Postion<T> position_{};
    PredictPath predict_path_{};
};

enum class ParkingSlotGeometryType : uint8_t
{
    UNKNOWN_0,
    PERPENDICULAR_1,
    PARALLEL_2,
    DIAGONAL_3
};
enum class ParkingSlotPhysicalType : uint8_t
{
    UNKNOWN_0,
    COMMON_1,
    SPACE_2,
    MECHANICAL_3
};


template <typename T>
class ParkingSlot
{
public:
    using PakringSlotVertics = std::array<Point3D<T>, 4>;
    using PakringSlotCenter = Point3D<T>;
private:
    Timestamp timestamp_;
    UUID id_{0};
    ParkingSlotGeometryType geometry_type_{ParkingSlotGeometryType::UNKNOWN_0};
    ParkingSlotPhysicalType physical_type_{ParkingSlotPhysicalType::UNKNOWN_0};
    PakringSlotCenter center_;
    PakringSlotVertics vertices_; 
};

// Keep the map domain model vocabulary compatible with existing callers.
template <typename T>
using PrkgSlot = ParkingSlot<T>;