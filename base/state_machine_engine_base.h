#pragma once

#include "state_machine_param_base.h"
#include "state_machine_inputer_base.h"
#include "state_machine_switcher_base.h"
#include "state_machine_outputer_base.h"

#include <string>
#include <thread>
#include <atomic>
#include <iostream>
#include <stdexcept>

template <typename State, typename Param, typename Inputer, typename Switcher, typename Outputer, 
typename = typename std::enable_if_t<std::is_enum_v<State>>,
// 类模板的模板声明（the declaration of class template, including 1 default template argument）
typename = typename std::enable_if_t<std::is_base_of_v<StateMachineParamBase, Param>>,
typename = typename std::enable_if_t<std::is_base_of_v<StateMachineInputer, Inputer>>,
typename = typename std::enable_if_t<std::is_base_of_v<StateMachineSwitcher<State>, Switcher>>,
typename = typename std::enable_if_t<std::is_base_of_v<StateMachineOutputer<State>, Outputer>>>
class StateMachineEngineBase;

// 在定义类模板时，不再需要指定默认模板参数
// don't need to specify that default template argument when define the class template 
template <typename State, typename Param, typename Inputer, typename Switcher, typename Outputer, typename, typename, typename, typename, typename>
/**
 * @brief 以固定频率驱动状态机各组件的通用引擎。
 *
 * @details 引擎拥有参数、输入、状态切换和输出四类组件的共享所有权。调用 Init()
 * 后会先依次初始化这四个组件，再创建后台循环线程。调用 Start() 后，循环线程按
 * 参数更新、事件更新、状态更新的顺序执行一个处理周期；输出动作调用当前处于注释
 * 状态。调用 Stop() 会关闭业务执行并请求循环线程退出。
 *
 * Init() 与 Start() 分离：前者完成组件及线程初始化，后者才允许状态机处理业务。
 * 状态标志使用原子变量同步访问；组件实例应在初始化完成后保持有效，并自行保证其
 * 被循环线程访问时所需的线程安全。
 *
 * @tparam State 状态枚举类型，必须为枚举。
 * @tparam Param 参数组件类型，必须派生自 StateMachineParamBase。
 * @tparam Inputer 输入组件类型，必须派生自 StateMachineInputer。
 * @tparam Switcher 状态切换组件类型，必须派生自 StateMachineSwitcher<State>。
 * @tparam Outputer 输出组件类型，必须派生自 StateMachineOutputer<State>。
 *
 * @note 需先调用 Init()，再调用 Start()。频率以 Hz 表示，调用方应传入非零值。
 */
class StateMachineEngineBase
{
public:
    /** @brief 状态枚举类型。 */
    using StateType = State;
    /** @brief 参数组件类型。 */
    using ParamType = Param;
    /** @brief 输入组件类型。 */
    using InputerType = Inputer;
    /** @brief 状态切换组件类型。 */
    using SwitcherType = Switcher;
    /** @brief 输出组件类型。 */
    using OutputerType = Outputer;
    /** @brief 参数组件的共享指针类型。 */
    using ParamPtr = std::shared_ptr<ParamType>;
    /** @brief 输入组件的共享指针类型。 */
    using InputerSPtr = std::shared_ptr<InputerType>;
    /** @brief 状态切换组件的共享指针类型。 */
    using SwitcherSPtr = std::shared_ptr<SwitcherType>;
    /** @brief 输出组件的共享指针类型。 */
    using OutputerSPtr = std::shared_ptr<OutputerType>;
private:
    std::string name_;
    uint32_t freq_{20};
    std::atomic_bool init_flag_{false};
    std::atomic_bool run_flag_{false};
    ParamPtr param_sptr_{std::make_shared<ParamType>()};
    InputerSPtr input_sptr_{std::make_shared<InputerType>()};
    SwitcherSPtr switch_sptr_{std::make_shared<SwitcherType>()};
    OutputerSPtr output_sptr_{std::make_shared<OutputerType>()};
    std::unique_ptr<std::thread> thrd_uptr_{nullptr};
protected:
    /**
     * @brief 构造状态机引擎。
     * @param name 用于诊断输出的引擎名称；为空时使用默认名称。
     * @param freq 循环执行频率，单位为 Hz，默认值为 20。
     * @throws std::invalid_argument 当任一默认创建的组件为空时抛出。
     */
    StateMachineEngineBase(std::string name, uint32_t freq = 20) : name_(name), freq_{freq}
    {
        if (name_.empty())
        {
            name_ = "StateMachineEngineBase";
        }
        if (!param_sptr_)
        {
            throw std::invalid_argument("StateMachineEngineBase: param_sptr_ is nullptr");
        }
        if (!input_sptr_)
        {
            throw std::invalid_argument("StateMachineEngineBase: input_sptr_ is nullptr");
        }
        if (!switch_sptr_)
        {
            throw std::invalid_argument("StateMachineEngineBase: switch_sptr_ is nullptr");
        }
        if (!output_sptr_)
        {
            throw std::invalid_argument("StateMachineEngineBase: output_sptr_ is nullptr");
        }
        std::cout << "\n[StateMachine] ***************************************\n" << std::endl;
        std::cout << "[StateMachine] StateMachineEngineBase: " << name_ << " is Created, Frequency : " << freq_ << std::endl;
    }
    /**
     * @brief 销毁引擎并回收后台循环线程。
     * @details 若循环线程仍可连接，会在析构期间等待其结束；随后清除初始化和运行标志。
     */
    ~StateMachineEngineBase()
    {
        if (thrd_uptr_ && thrd_uptr_->joinable())
        {
            thrd_uptr_->join();
        }
        Stop();
    }
public:
    /**
     * @brief 初始化组件并创建后台循环线程。
     * @details 按参数、输入、切换器、输出器的顺序调用 Init()，随后设置初始化标志，
     * 并启动 Run()。本方法不会自动进入业务运行状态，需再调用 Start()。
     */
    void Init() 
    {
        param_sptr_->Init();
        input_sptr_->Init();;
        switch_sptr_->Init();
        output_sptr_->Init();

        SetInitFlag(true);

        thrd_uptr_ = std::make_unique<std::thread>(&StateMachineEngineBase::Run, this);
    }
    /**
     * @brief 允许已初始化的循环线程执行状态机业务逻辑。
     * @details 未初始化时不会修改运行状态，并输出提示信息。
     */
    void Start()
    {
        if (GetInitFlag())
        {
            SetRunFlag(true);
        }
        else
        {
            std::cout << "StateMachineEngineBase::Start() failed, please call Init() first!" << std::endl;
        }
    }
    /**
     * @brief 停止业务处理并请求后台循环退出。
     * @details 本方法不直接 join 线程；循环线程会在下一次检查初始化标志时结束。
     */
    void Stop()
    {
        SetRunFlag(false);
        SetInitFlag(false);
    }
    /**
     * @brief 在后台按配置频率执行状态机周期。
     * @details 初始化标志有效期间持续循环。运行标志有效时，依次更新参数、采集事件
     * 并计算状态转换；每轮根据耗时休眠至下一个周期。若周期执行超时则输出诊断信息。
     * 该方法由 Init() 创建的线程调用，通常不应由调用方直接执行。
     */
    void Run()
    {
        while (GetInitFlag())
        {
            auto start_time = steady_clock::now();
            if (GetRunFlag())
            {
                param_sptr_->UpdateParam();
                input_sptr_->UpdateEvent();
                switch_sptr_->UpdateState();
                // output_sptr_->UpdateAction();
            }
            auto elapsed_time = duration_cast<milliseconds>(steady_clock::now() - start_time).count();
            if (elapsed_time < (1000 / freq_))  
            {
                std::this_thread::sleep_for(milliseconds((1000 / freq_) - elapsed_time));
            }
            else
            {
                std::cout << "StateMachineEngineBase::Run() is running slower than expected, elapsed time: " << elapsed_time << " ms" << std::endl;
            }
        }
    }
private:
    /** @brief 获取组件是否已完成初始化。 */
    bool GetInitFlag() const noexcept
    {
        return init_flag_.load();
    }
    /** @brief 设置组件初始化状态。 */
    void SetInitFlag(bool flag = false) noexcept
    {
        init_flag_.store(flag);
    }
    /** @brief 获取循环线程是否应执行业务处理。 */
    bool GetRunFlag() const noexcept
    {
        return run_flag_.load();
    }
    /** @brief 设置循环线程的业务处理开关。 */
    void SetRunFlag(bool flag = false) noexcept
    {
        run_flag_.store(flag);
    }
    /** @brief 获取配置的循环频率，单位为 Hz。 */
    uint32_t GetFrequency() const noexcept
    {
        return freq_;
    }
    /** @brief 设置循环频率，单位为 Hz。 */
    void  SetFrequency(uint32_t freq = 20) noexcept
    {
        freq_ = freq;
    }
};