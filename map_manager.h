#pragma once

#include "algorithm.h"

#include <vector>
#include <string>
#include <stdint.h>
#include <map>
#include <memory>
#include <filesystem>
#include <type_traits>
#include <fstream>
#include <shared_mutex>
#include <iostream>
#include <utility>
#include <algorithm>
// #include <nlohmann/json.hpp>

// ...



/**
 * @brief 停车地图的来源或业务类型。
 *
 * 枚举值用于区分地图的存储来源和使用场景。数值后缀表示当前协议中的
 * 对外编码，修改枚举顺序或数值可能影响持久化数据和通信接口。
 */
enum class PrkgMapType : uint8_t
{
    INVALID_0,
    SELF_BUILT_MAP_1,
    OFFICIAL_MAP_2,
    PARK_TO_PARK_MAP_3,
    SHARED_MAP_4
};


/**
 * @brief 停车地图的基础元数据信息。
 *
 * PrkgMapMetaInfo 保存地图管理和上层应用最常用的公共属性，包括地图
 * 标识、名称、创建时间、地图类型、路径长度、文件大小、规划路径以及楼层
 * 列表。该类不负责地图文件解析，也不负责地图数据的持久化。
 *
 * @note SetPath() 或通过 GetPath() 修改路径后，不会自动重新计算 distance_。
 *       如果路径和长度需要保持一致，应由调用方在更新路径后同步更新距离。
 */
class PrkgMapMetaInfo
{
public:
    using MapID = uint64_t;
    using FloorID = int8_t;
    using MapName = std::string;
    using CreateTime = uint64_t;
    using MapIDList = std::vector<uint64_t>;
    using PlanningPath = std::vector<Point3D<double>>;
    using FloorIDList = std::vector<FloorID>;
    
private:
    MapID id_{0};
    MapName name_{""};
    CreateTime time_{0};
    PrkgMapType type_{PrkgMapType::INVALID_0};
    double distance_{0.0};
    uint64_t size_{0};
    PlanningPath path_;
    FloorIDList floor_ids_;

public:
    /**
     * @brief 获取地图唯一标识。
     * @return 地图唯一标识。
     */
    MapID GetId() const noexcept
    {
        return id_;
    }

    /**
     * @brief 设置地图唯一标识。
     * @param id 要设置的地图唯一标识。
     */
    void SetId(MapID id) noexcept
    {
        id_ = id;
    }

    /**
     * @brief 获取地图名称。
     * @return 地图名称的只读引用；引用在当前对象未被销毁或移动前有效。
     */
    const MapName& GetName() const noexcept
    {
        return name_;
    }

    /**
     * @brief 设置地图名称。
     * @tparam T 可转换为 MapName 的字符串类型。
     * @param name 新地图名称。支持字符串、字符串字面量以及其他可转换类型。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, MapName>>>
    void SetName(T&& name)
    {
        name_ = std::forward<T>(name);
    }

    /**
     * @brief 获取地图创建时间。
     * @return 地图创建时间，单位由上层时间约定定义。
     */
    CreateTime GetTime() const noexcept
    {
        return time_;
    }

    /**
     * @brief 设置地图创建时间。
     * @tparam T 可转换为 CreateTime 的数值类型。
     * @param time 新的地图创建时间，单位由上层时间约定定义。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, CreateTime>>>
    void SetTime(T&& time) noexcept
    {
        time_ = std::forward<T>(time);
    }

    /**
     * @brief 获取地图类型。
     * @return 地图类型枚举值。
     */
    PrkgMapType GetType() const noexcept
    {
        return type_;
    }

    /**
     * @brief 设置地图类型。
     * @param type 新的地图类型。
     */
    void SetType(PrkgMapType type) noexcept
    {
        type_ = type;
    }

    /**
     * @brief 获取地图路径长度。
     * @return 地图路径长度，单位为米。
     */
    double GetDistance() const noexcept
    {
        return distance_;
    }

    /**
     * @brief 设置地图路径长度。
     * @param distance 地图路径长度，单位为米。
     */
    void SetDistance(double distance) noexcept
    {
        distance_ = distance;
    }

    /**
     * @brief 获取地图数据大小。
     * @return 地图数据大小，单位为字节。
     */
    uint64_t GetSize() const noexcept
    {
        return size_;
    }

    /**
     * @brief 设置地图数据大小。
     * @param size 地图数据大小，单位为字节。
     */
    void SetSize(uint64_t size) noexcept
    {
        size_ = size;
    }

    /**
     * @brief 获取规划路径的只读引用。
     * @return 规划路径的只读引用，路径点类型为 Point3D<double>。
     * @note 返回引用不会复制路径数据，但引用的有效期受当前对象生命周期影响。
     */
    const PlanningPath& GetPath() const noexcept
    {
        return path_;
    }

    /**
     * @brief 获取可修改的规划路径。
     * @return 规划路径的可修改引用。
     * @note 通过该引用修改路径不会自动更新 distance_，调用方需要自行维护二者一致性。
     */
    PlanningPath& GetPath() noexcept
    {
        return path_;
    }

    /**
     * @brief 替换规划路径。
     * @tparam T 可转换为 PlanningPath 的路径容器类型。
     * @param path 新的规划路径，支持拷贝或移动赋值。
     * @note 替换路径不会自动重新计算 distance_。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, PlanningPath>>>
    void SetPath(T&& path)
    {
        path_ = std::forward<T>(path);
    }

    /**
     * @brief 获取地图包含的楼层编号列表。
     * @return 楼层编号列表的只读引用。
     * @note 返回引用不会复制容器，引用的有效期受当前对象生命周期影响。
     */
    const FloorIDList& GetFloorIds() const noexcept
    {
        return floor_ids_;
    }

    /**
     * @brief 获取可修改的楼层编号列表。
     * @return 楼层编号列表的可修改引用。
     */
    FloorIDList& GetFloorIds() noexcept
    {
        return floor_ids_;
    }

    /**
     * @brief 替换楼层编号列表。
     * @tparam T 可转换为 FloorIDList 的容器类型。
     * @param floor_ids 新的楼层编号列表，支持拷贝或移动赋值。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, FloorIDList>>>
    void SetFloorIds(T&& floor_ids)
    {
        floor_ids_ = std::forward<T>(floor_ids);
    }
};

class PrkgMapPoi
{
public:
    enum class PrkgMapPoiType : uint8_t
    {
        UNKNOWN_0,
        PARKING_LOT_ENTRANCE_1,
        PARKING_LOT_EXIT_2,
        TILOT_3,
        ELEVATOR_4
    };
    using PoiID = uint64_t;
    using Position = Point3D<double>;
private:
    PoiID poi_{0};
    PrkgMapPoiType type_{PrkgMapPoiType::UNKNOWN_0};
    Position position_{};

public:
    /**
     * @brief 获取 POI 唯一标识。
     * @return POI 唯一标识。
     */
    PoiID GetPoiId() const noexcept
    {
        return poi_;
    }

    /**
     * @brief 设置 POI 唯一标识。
     * @param poi_id 新的 POI 唯一标识。
     */
    void SetPoiId(PoiID poi_id) noexcept
    {
        poi_ = poi_id;
    }

    /**
     * @brief 获取 POI 类型。
     * @return POI 类型枚举值。
     */
    PrkgMapPoiType GetType() const noexcept
    {
        return type_;
    }

    /**
     * @brief 设置 POI 类型。
     * @param type 新的 POI 类型。
     */
    void SetType(PrkgMapPoiType type) noexcept
    {
        type_ = type;
    }

    /**
     * @brief 获取 POI 的三维位置。
     * @return POI 位置的只读引用，单位由地图坐标系约定，通常为米。
     * @note 返回引用不会复制位置数据，其有效期受当前对象生命周期影响。
     */
    const Position& GetPosition() const noexcept
    {
        return position_;
    }

    /**
     * @brief 获取可修改的 POI 三维位置。
     * @return POI 位置的可修改引用。
     */
    Position& GetPosition() noexcept
    {
        return position_;
    }

    /**
     * @brief 设置 POI 的三维位置。
     * @tparam T 可转换为 Position 的位置类型。
     * @param position 新的 POI 三维位置。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, Position>>>
    void SetPosition(T&& position) noexcept
    {
        position_ = std::forward<T>(position);
    }
};

/**
 * @brief 单层地图中的语义信息集合。
 *
 * FloorSmtcInfo 按楼层组织障碍物、车位和墙体等静态或动态语义
 * 元素，供地图显示、定位匹配、路径规划和安全检查使用。
 *
 * @tparam Container 语义元素使用的序列容器模板，默认使用 std::vector。
 *                   容器应支持默认构造、范围访问和移动赋值等基本操作。
 */
template <template <typename, typename...> class Container = std::vector>
class FloorSmtcInfo
{
public:
    using ObstacleList = Container<Obstacle<double>>;
    using PrkgSlotList = Container<PrkgSlot<double>>;
    using Wall = Container<Point3D<double>>;
    using WallList = Container<Wall>;
    using PrkgMapPoiList = Container<PrkgMapPoi>;

private:
    /** @brief 当前楼层的障碍物列表。 */
    ObstacleList obstacles_{};

    /** @brief 当前楼层的车位列表。 */
    PrkgSlotList Prkg_slots_{};

    /**
     * @brief 当前楼层的墙体边界列表。
     *
     * 每面墙由一组按顺序排列的三维点表示，点序列可表示墙体中心线、
     * 折线或边界轮廓，具体解释由地图格式约定。
     */
    WallList walls_{};
    PrkgMapPoiList pois_{};

public:
    /**
     * @brief 获取当前楼层的障碍物列表。
     * @return 障碍物列表的只读引用。
     * @note 返回引用不会复制数据，其有效期受当前对象生命周期影响。
     */
    const ObstacleList& GetObstacles() const noexcept
    {
        return obstacles_;
    }

    /**
     * @brief 获取可修改的障碍物列表。
     * @return 障碍物列表的可修改引用。
     */
    ObstacleList& GetObstacles() noexcept
    {
        return obstacles_;
    }

    /**
     * @brief 替换当前楼层的障碍物列表。
     * @param obstacles 新的障碍物列表，支持拷贝或移动赋值。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, ObstacleList>>>
    void SetObstacles(T&& obstacles)
    {
        obstacles_ = std::forward<T>(obstacles);
    }

    /**
     * @brief 获取当前楼层的车位列表。
     * @return 车位列表的只读引用。
     * @note 返回引用不会复制数据，其有效期受当前对象生命周期影响。
     */
    const PrkgSlotList& GetPrkgSlots() const noexcept
    {
        return Prkg_slots_;
    }

    /**
     * @brief 获取可修改的车位列表。
     * @return 车位列表的可修改引用。
     */
    PrkgSlotList& GetPrkgSlots() noexcept
    {
        return Prkg_slots_;
    }

    /**
     * @brief 替换当前楼层的车位列表。
     * @param Prkg_slots 新的车位列表，支持拷贝或移动赋值。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, PrkgSlotList>>>
    void SetPrkgSlots(T&& Prkg_slots)
    {
        Prkg_slots_ = std::forward<T>(Prkg_slots);
    }

    /**
     * @brief 获取当前楼层的 POI 列表。
     * @return POI 列表的只读引用。
     * @note 返回引用不会复制数据，其有效期受当前对象生命周期影响。
     */
    const PrkgMapPoiList& GetPois() const noexcept
    {
        return pois_;
    }

    /**
     * @brief 获取可修改的当前楼层 POI 列表。
     * @return POI 列表的可修改引用。
     */
    PrkgMapPoiList& GetPois() noexcept
    {
        return pois_;
    }

    /**
     * @brief 替换当前楼层的 POI 列表。
     * @param pois 新的 POI 列表，支持拷贝或移动赋值。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, PrkgMapPoiList>>>
    void SetPois(T&& pois)
    {
        pois_ = std::forward<T>(pois);
    }

    /**
     * @brief 获取当前楼层的墙体列表。
     * @return 墙体列表的只读引用。
     * @note 每面墙由一组按顺序排列的三维点表示。
     */
    const WallList& GetWalls() const noexcept
    {
        return walls_;
    }

    /**
     * @brief 获取可修改的墙体列表。
     * @return 墙体列表的可修改引用。
     */
    WallList& GetWalls() noexcept
    {
        return walls_;
    }

    /**
     * @brief 替换当前楼层的墙体列表。
     * @param walls 新的墙体列表，支持拷贝或移动赋值。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, WallList>>>
    void SetWalls(T&& walls)
    {
        walls_ = std::forward<T>(walls);
    }
};

using PrkgMapSmtcInfo = std::map<PrkgMapMetaInfo::FloorID, FloorSmtcInfo<>>;

/**
 * @brief 完整停车地图信息。
 *
 * 由地图基础元数据和按楼层组织的语义信息组成。
 */
class PrkgMapInfo
{
private:
    PrkgMapMetaInfo base_info_;
    PrkgMapSmtcInfo smtc_info_;

public:
    /**
     * @brief 获取地图基础元数据信息。
     * @return 基础元数据的只读引用。
     * @note 返回引用不会复制数据，其有效期受当前对象生命周期影响。
     */
    const PrkgMapMetaInfo& GetBaseInfo() const noexcept
    {
        return base_info_;
    }

    /**
     * @brief 获取可修改的地图基础元数据信息。
     * @return 基础元数据的可修改引用。
     */
    PrkgMapMetaInfo& GetBaseInfo() noexcept
    {
        return base_info_;
    }

    /**
     * @brief 替换地图基础元数据信息。
     * @param base_info 新的地图基础元数据，支持拷贝或移动赋值。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, PrkgMapMetaInfo>>>
    void SetBaseInfo(T&& base_info)
    {
        base_info_ = std::forward<T>(base_info);
    }

    /**
     * @brief 获取按楼层组织的地图语义信息。
     * @return 楼层语义信息映射的只读引用。
     * @note 映射键为楼层编号，返回引用不会复制数据。
     */
    const PrkgMapSmtcInfo& GetSmtcInfo() const noexcept
    {
        return smtc_info_;
    }

    /**
     * @brief 获取可修改的地图语义信息。
     * @return 楼层语义信息映射的可修改引用。
     */
    PrkgMapSmtcInfo& GetSmtcInfo() noexcept
    {
        return smtc_info_;
    }

    /**
     * @brief 替换地图语义信息。
     * @param smtc_info 新的楼层语义信息映射，支持拷贝或移动赋值。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, PrkgMapSmtcInfo>>>
    void SetSmtcInfo(T&& smtc_info)
    {
        smtc_info_ = std::forward<T>(smtc_info);
    }
};

/**
 * @brief 统一管理不同来源的停车地图数据。
 *
 * MapManager 负责地图存储路径和地图类型的统一抽象，为 AVP、导航及
 * 其他应用提供加载、删除等基础接口。具体的地图解析和业务筛选逻辑
 * 应由对应的应用模块在这些接口之上实现。
 *
 * @note 当前加载接口仍为框架接口，返回的地图对象尚未填充 JSON 解析结果。
 */

class MapManager
{
public:
    /** @brief 地图唯一标识类型。 */
    using MapID = PrkgMapMetaInfo::MapID;

    /** @brief 停车地图信息对象的共享指针类型。 */
    using PrkgMapInfoSPtr = std::shared_ptr<PrkgMapInfo>;

    /** @brief 停车地图信息列表的共享指针类型。 */
    using PrkgMapInfoListSPtr = std::shared_ptr<std::vector<PrkgMapInfo>>;

private:
    /**
     * @brief 地图数据根目录。
     *
     * 其他地图目录通常以该路径为基准进行拼接。
     */
    std::filesystem::path base_path_{""};

    /** @brief 用户自建地图的相对目录或目录名称。 */
    std::string self_built_map_directory_{""};

    /** @brief 官方地图的相对目录或目录名称。 */
    std::string official_map_directory_{""};

    /** @brief 车位到车位地图的相对目录或目录名称。 */
    std::string park2park_map_directory_{""};

    /** @brief 共享地图的相对目录或目录名称。 */
    std::string shared_map_directory_{""};

    /** @brief 用户自建地图信息列表。 */
    PrkgMapInfoListSPtr self_built_map_list_{nullptr};

    /** @brief 官方地图信息列表。 */
    PrkgMapInfoListSPtr official_map_list_{nullptr};

    /** @brief 车位到车位地图信息列表。 */
    PrkgMapInfoListSPtr park2park_map_list_{nullptr};

    /** @brief 共享地图信息列表。 */
    PrkgMapInfoListSPtr shared_map_list_{nullptr};

public:
    /**
     * @brief 获取地图根目录。
     * @return 地图根目录的只读引用。
     */
    const std::filesystem::path& GetBasePath() const noexcept
    {
        return base_path_;
    }

    /**
     * @brief 设置地图根目录。
     * @param path 新的地图根目录。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, std::filesystem::path>>>
    void SetBasePath(T&& path)
    {
        base_path_ = std::forward<T>(path);
    }

    /**
     * @brief 获取用户自建地图目录。
     * @return 用户自建地图目录的只读引用。
     */
    const std::string& GetSelfBuiltMapDirectory() const noexcept
    {
        return self_built_map_directory_;
    }

    /**
     * @brief 获取用户自建地图的完整存储路径。
     * @return 地图根目录与用户自建地图目录拼接后的路径。
     */
    const std::filesystem::path GetSelfBuiltMapPath() const noexcept
    {
        return base_path_ / self_built_map_directory_;
    }

    /**
     * @brief 设置用户自建地图目录。
     * @param path 用户自建地图目录。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, std::string>>>
    void SetSelfBuiltMapDirectory(T&& path)
    {
        self_built_map_directory_ = std::forward<T>(path);
    }

    /**
     * @brief 获取官方地图目录。
     * @return 官方地图目录的只读引用。
     */
    const std::string& GetOfficialMapDirectory() const noexcept
    {
        return official_map_directory_;
    }

    /**
     * @brief 获取官方地图的完整存储路径。
     * @return 地图根目录与官方地图目录拼接后的路径。
     */
    const std::filesystem::path GetOfficialMapPath() const noexcept
    {
        return base_path_ / official_map_directory_;
    }

    /**
     * @brief 设置官方地图目录。
     * @param path 官方地图目录。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, std::string>>>
    void SetOfficialMapDirectory(T&& path)
    {
        official_map_directory_ = std::forward<T>(path);
    }

    /**
     * @brief 获取车位到车位地图目录。
     * @return 车位到车位地图目录的只读引用。
     */
    const std::string& GetParkToParkMapDirectory() const noexcept
    {
        return park2park_map_directory_;
    }

    /**
     * @brief 获取车位到车位地图的完整存储路径。
     * @return 地图根目录与车位到车位地图目录拼接后的路径。
     */
    const std::filesystem::path GetParkToParkMapPath() const noexcept
    {
        return base_path_ / park2park_map_directory_;
    }

    /**
     * @brief 设置车位到车位地图目录。
     * @param path 车位到车位地图目录。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, std::string>>>
    void SetParkToParkMapDirectory(T&& path)
    {
        park2park_map_directory_ = std::forward<T>(path);
    }

    /**
     * @brief 获取共享地图目录。
     * @return 共享地图目录的只读引用。
     */
    const std::string& GetSharedMapDirectory() const noexcept
    {
        return shared_map_directory_;
    }
    /**
     * @brief 获取共享地图的完整存储路径。
     * @return 地图根目录与共享地图目录拼接后的路径。
     */
    const std::filesystem::path GetSharedMapPath() const noexcept
    {
        return base_path_ / shared_map_directory_;
    }
    /**
     * @brief 设置共享地图目录。
     * @param path 共享地图目录。
     */
    template <typename T, typename = std::enable_if_t<std::is_convertible_v<T, std::string>>>
    void SetSharedMapDirectory(T&& path)
    {
        shared_map_directory_ = std::forward<T>(path);
    }

    /**
     * @brief 获取用户自建地图列表。
     * @return 用户自建地图列表的只读智能指针引用。
     */
    const PrkgMapInfoListSPtr& GetSelfBuiltMapList() const noexcept
    {
        return self_built_map_list_;
    }

    /**
     * @brief 获取可修改的用户自建地图列表指针。
     * @return 用户自建地图列表的智能指针引用。
     */
    PrkgMapInfoListSPtr& GetSelfBuiltMapList() noexcept
    {
        return self_built_map_list_;
    }

    /**
     * @brief 设置用户自建地图列表。
     * @param map_list 新的用户自建地图列表指针。
     */
    void SetSelfBuiltMapList(PrkgMapInfoListSPtr map_list) noexcept
    {
        self_built_map_list_ = std::move(map_list);
    }

    /**
     * @brief 获取官方地图列表。
     * @return 官方地图列表的只读智能指针引用。
     */
    const PrkgMapInfoListSPtr& GetOfficialMapList() const noexcept
    {
        return official_map_list_;
    }

    /**
     * @brief 获取可修改的官方地图列表指针。
     * @return 官方地图列表的智能指针引用。
     */
    PrkgMapInfoListSPtr& GetOfficialMapList() noexcept
    {
        return official_map_list_;
    }

    /**
     * @brief 设置官方地图列表。
     * @param map_list 新的官方地图列表指针。
     */
    void SetOfficialMapList(PrkgMapInfoListSPtr map_list) noexcept
    {
        official_map_list_ = std::move(map_list);
    }

    /**
     * @brief 获取车位到车位地图列表。
     * @return 车位到车位地图列表的只读智能指针引用。
     */
    const PrkgMapInfoListSPtr& GetParkToParkMapList() const noexcept
    {
        return park2park_map_list_;
    }

    /**
     * @brief 获取可修改的车位到车位地图列表指针。
     * @return 车位到车位地图列表的智能指针引用。
     */
    PrkgMapInfoListSPtr& GetParkToParkMapList() noexcept
    {
        return park2park_map_list_;
    }

    /**
     * @brief 设置车位到车位地图列表。
     * @param map_list 新的车位到车位地图列表指针。
     */
    void SetParkToParkMapList(PrkgMapInfoListSPtr map_list) noexcept
    {
        park2park_map_list_ = std::move(map_list);
    }

    /**
     * @brief 获取共享地图列表。
     * @return 共享地图列表的只读智能指针引用。
     */
    const PrkgMapInfoListSPtr& GetSharedMapList() const noexcept
    {
        return shared_map_list_;
    }

    /**
     * @brief 获取可修改的共享地图列表指针。
     * @return 共享地图列表的智能指针引用。
     */
    PrkgMapInfoListSPtr& GetSharedMapList() noexcept
    {
        return shared_map_list_;
    }

    /**
     * @brief 设置共享地图列表。
     * @param map_list 新的共享地图列表指针。
     */
    void SetSharedMapList(PrkgMapInfoListSPtr map_list) noexcept
    {
        shared_map_list_ = std::move(map_list);
    }
public:
    /**
     * @brief 扫描指定目录中的地图文件。
     *
     * 当前实现递归遍历目录并识别 `.json` 文件，具体 JSON 解析和地图列表
     * 更新逻辑尚未完成。
     *
     * @param path 待扫描的地图目录。
     * @return 当前加载操作的执行结果。
     */
    bool LoadMapInfoList(const std::filesystem::path& path) const
    {
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
        {
            return false;
        }

        bool ret = false;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json") 
            {
                std::ifstream f(entry.path().c_str());
                // json data = json::parse(f);
                ret = f.is_open();
            }
        }
        return ret;
    }
    /**
     * @brief 加载用户自建地图列表。
     * @return 用户自建地图目录扫描结果。
     */
    bool LoadSelfBuiltMapList() const
    {
        return LoadMapInfoList(base_path_ / self_built_map_directory_);
    }
    /**
     * @brief 加载官方地图列表。
     * @return 官方地图目录扫描结果。
     */
    bool LoadOfficialMapList() const
    {
        return LoadMapInfoList(base_path_ / official_map_directory_);
    }
    /**
     * @brief 加载车位到车位地图列表。
     * @return 车位到车位地图目录扫描结果。
     */
    bool LoadPark2ParkMapList() const
    {
        return LoadMapInfoList(base_path_ / park2park_map_directory_);
    }
    /**
     * @brief 加载共享地图列表。
     * @return 共享地图目录扫描结果。
     */
    bool LoadSharedMapList() const
    {
        return LoadMapInfoList(base_path_ / shared_map_directory_);
    }
public:
    /**
     * @brief 按地图类型删除指定地图。
     * @param type 地图类型。
     * @param id 待删除地图的唯一标识。
     * @return 删除成功返回 true；类型无效或删除失败返回 false。
     */
    bool DeleteMap(PrkgMapType type, MapID id)
    {
        switch (type)
        {
            case PrkgMapType::SELF_BUILT_MAP_1:
                return DeleteSelfBuiltMap(id);
            case PrkgMapType::OFFICIAL_MAP_2:
                return DeleteOfficialMap(id);
            case PrkgMapType::PARK_TO_PARK_MAP_3:
                return DeletePark2ParkMap(id);
            case PrkgMapType::SHARED_MAP_4:
                return DeleteSharedMap(id);
            default:
                return false;
        }
    }
    /**
     * @brief 按地图类型批量删除地图。
     * @param type 待删除地图的类型。
     * @param id 待删除地图的 ID 列表。
     * @return 批量删除结果。
     * @note 当前函数体尚未实现具体删除逻辑。
     */
    bool DeleteMapList(PrkgMapType type, std::vector<MapID> id)
    {
        bool ret = true;
        for (const MapID map_id : id)
        {
            bool deleted = false;
            switch (type)
            {
                case PrkgMapType::SELF_BUILT_MAP_1:
                    deleted = DeleteSelfBuiltMap(map_id);
                    break;
                case PrkgMapType::OFFICIAL_MAP_2:
                    deleted = DeleteOfficialMap(map_id);
                    break;
                case PrkgMapType::PARK_TO_PARK_MAP_3:
                    deleted = DeletePark2ParkMap(map_id);
                    break;
                case PrkgMapType::SHARED_MAP_4:
                    deleted = DeleteSharedMap(map_id);
                    break;
                default:
                    return false;
            }
            ret = ret && deleted;
        }
        return ret;
    }
    /**
     * @brief 删除指定的用户自建地图。
     * @param id 用户自建地图的唯一标识。
     * @return 删除成功返回 true，否则返回 false。
     */
    bool DeleteSelfBuiltMap(MapID id)
    {
        bool ret = false;
        if (self_built_map_list_)
        {
            auto iter = std::find_if(std::begin(*self_built_map_list_), std::end(*self_built_map_list_), [id](const auto& map_info) { return map_info.GetBaseInfo().GetId() == id; });
            if (iter != std::end(*self_built_map_list_))
            {
                self_built_map_list_->erase(iter);
                ret = true;
            }
            else
            {
                std::cerr << "" << std::endl;
            }
        }
        return ret;
    }
    /**
     * @brief 批量删除用户自建地图。
     * @param ids 用户自建地图 ID 列表。
     * @return 批量删除结果。
     */
    bool DeleteSelfBuiltMapList(std::vector<MapID> ids)
    {
        return std::all_of(std::begin(ids), std::end(ids), [this](MapID id) {
            return DeleteSelfBuiltMap(id);
        });
    }
    /**
     * @brief 删除指定的官方地图。
     * @param id 官方地图的唯一标识。
     * @return 删除成功返回 true，否则返回 false。
     */
    bool DeleteOfficialMap(MapID id)
    {
        bool ret = false;
        if (official_map_list_)
        {
            auto iter = std::find_if(std::begin(*official_map_list_), std::end(*official_map_list_), [id](const auto& map_info) { return map_info.GetBaseInfo().GetId() == id; });
            if (iter != std::end(*official_map_list_))
            {
                official_map_list_->erase(iter);
                ret = true;
            }
            else
            {
                std::cerr << "" << std::endl;
            }
        }
        return ret;
    }
    /**
     * @brief 批量删除官方地图。
     * @param ids 官方地图 ID 列表。
     * @return 批量删除结果。
     */
    bool DeleteOfficialMapList(std::vector<MapID> ids)
    {
        return std::all_of(std::begin(ids), std::end(ids), [this](MapID id) {
            return DeleteOfficialMap(id);
        });
    }
    /**
     * @brief 删除指定的车位到车位地图。
     * @param id 车位到车位地图的唯一标识。
     * @return 删除成功返回 true，否则返回 false。
     */
    bool DeletePark2ParkMap(MapID id)
    {
        bool ret = false;
        if (park2park_map_list_)
        {
            auto iter = std::find_if(std::begin(*park2park_map_list_), std::end(*park2park_map_list_), [id](const auto& map_info) { return map_info.GetBaseInfo().GetId() == id; });
            if (iter != std::end(*park2park_map_list_))
            {
                park2park_map_list_->erase(iter);
                ret = true;
            }
            else
            {
                std::cerr << "" << std::endl;
            }
        }
        return ret;
    }
    /**
     * @brief 批量删除车位到车位地图。
     * @param ids 车位到车位地图 ID 列表。
     * @return 批量删除结果。
     */
    bool DeletePark2ParkMapList(std::vector<MapID> ids)
    {
        return std::all_of(std::begin(ids), std::end(ids), [this](MapID id) {
            return DeletePark2ParkMap(id);
        });
    }
    /**
     * @brief 删除指定的共享地图。
     * @param id 共享地图的唯一标识。
     * @return 删除成功返回 true，否则返回 false。
     */
    bool DeleteSharedMap(MapID id)
    {
        bool ret = false;
        if (shared_map_list_)
        {
            auto iter = std::find_if(std::begin(*shared_map_list_), std::end(*shared_map_list_), [id](const auto& map_info) { return map_info.GetBaseInfo().GetId() == id; });
            if (iter != std::end(*shared_map_list_))
            {
                shared_map_list_->erase(iter);
                ret = true;
            }
            else
            {
                std::cerr << "" << std::endl;
            }
        }
        return ret;
    }
    /**
     * @brief 批量删除共享地图。
     * @param ids 共享地图 ID 列表。
     * @return 批量删除结果。
     */
    bool DeleteSharedMapList(std::vector<MapID> ids)
    {
        return std::all_of(std::begin(ids), std::end(ids), [this](MapID id) {
            return DeleteSharedMap(id);
        });
    }
public:
    /**
     * @brief 创建地图管理器并初始化地图目录。
     * @tparam T 可转换为 std::string 的路径类型。
     * @param base_path 地图数据根目录。
     * @param self_built_map_path 用户自建地图目录。
     * @param official_map_path 官方地图目录。
     * @param park2park_map_path 车位到车位地图目录。
     * @param shared_map_path 共享地图目录。
     * @note 构造过程中会检查根目录，并尝试创建缺失的目录。
     */
    template <typename T, typename = typename std::enable_if_t<std::is_same_v<std::decay_t<T>, std::string>>>
    MapManager(T&& base_path, T&& self_built_map_path, T&& official_map_path, T&& park2park_map_path, T&& shared_map_path) :
    base_path_(std::forward<T>(base_path)),
    self_built_map_directory_(std::forward<T>(self_built_map_path)),
    official_map_directory_(std::forward<T>(official_map_path)),
    park2park_map_directory_(std::forward<T>(park2park_map_path)),
    shared_map_directory_(std::forward<T>(shared_map_path))
    {
        if (std::filesystem::exists(base_path_) && std::filesystem::is_directory(base_path_))
        {
            std::filesystem::path self_built_map_path = base_path_ / self_built_map_directory_;
            std::filesystem::path official_map_path = base_path_ / official_map_directory_;
            std::filesystem::path park2park_map_path = base_path_ / park2park_map_directory_;
            std::filesystem::path shared_map_path = base_path_ / shared_map_directory_;

            if (std::filesystem::exists(self_built_map_path) && std::filesystem::is_directory(self_built_map_path))
            {
                std::cout << "self-built map path : " << self_built_map_path.c_str() << std::endl;
            }
            else
            {
                std::filesystem::create_directories(self_built_map_path);
            }

            if (std::filesystem::exists(official_map_path) && std::filesystem::is_directory(official_map_path))
            {
                std::cout << "official map path : " << official_map_path.c_str() << std::endl;
            }
            else
            {
                std::filesystem::create_directories(official_map_path);
            }

            if (std::filesystem::exists(park2park_map_path) && std::filesystem::is_directory(park2park_map_path))
            {
                std::cout << "park2park map path : " << park2park_map_path.c_str() << std::endl;
            }
            else
            {
                std::filesystem::create_directories(park2park_map_path);
            }

            if (std::filesystem::exists(shared_map_path) && std::filesystem::is_directory(shared_map_path))
            {
                std::cout << "shared map path : " << shared_map_path.c_str() << std::endl;
            }
            else
            {
                std::filesystem::create_directories(shared_map_path);
            }
        }
        else
        {
            throw std::runtime_error("Base path does not exist or is not a directory: " + base_path_.string());
        }
    }
};