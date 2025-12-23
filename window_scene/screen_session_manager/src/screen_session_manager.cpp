/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "screen_session_manager/include/screen_session_manager.h"

#include <csignal>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <string_ex.h>
#include <unique_fd.h>
#include <unordered_set>
#include <cmath>
#include "input_manager.h"

#include <hitrace_meter.h>
#ifdef DEVICE_STATUS_ENABLE
#include <interaction_manager.h>
#endif // DEVICE_STATUS_ENABLE
#include <ipc_skeleton.h>
#include <parameter.h>
#include <parameters.h>
#include <privacy_kit.h>
#include <system_ability_definition.h>
#include <transaction/rs_interfaces.h>
#include <xcollie/watchdog.h>
#include <hisysevent.h>
#include <power_mgr_client.h>
#include <screen_power_utils.h>

#include "dm_common.h"
#include "fold_screen_state_internel.h"
#ifdef WM_MULTI_SCREEN_ENABLE
#include "multi_screen_manager.h"
#include "multi_screen_power_change_manager.h"
#include "multi_screen_change_utils.h"
#endif
#include "os_account_manager.h"
#include "pipeline/rs_node_map.h"
#include "rs_adapter.h"
#include "scene_board_judgement.h"
#include "session_permission.h"
#include "screen_scene_config.h"
#include "surface_capture_future.h"
#include "sys_cap_util.h"
#include "permission.h"
#include "window_manager_hilog.h"
#include "screen_rotation_property.h"
#include "screen_sensor_connector.h"
#include "screen_setting_helper.h"
#include "screen_session_dumper.h"
#include "mock_session_manager_service.h"
#include "connection/screen_snapshot_picker_connection.h"
#include "connection/screen_cast_connection.h"
#include "publish/screen_session_publish.h"
#include "dms_xcollie.h"
#include "screen_scene_config.h"
#include "screen_sensor_plugin.h"
#include "screen_cache.h"
#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
#include <display_power_mgr_client.h>
#endif
#ifdef FOLD_ABILITY_ENABLE
#include "fold_screen_controller/super_fold_sensor_manager.h"
#include "fold_screen_controller/super_fold_state_manager.h"
#include "fold_screen_controller/secondary_fold_sensor_manager.h"
#include "fold_screen_controller/super_fold_policy.h"
#endif
#include "screen_aod_plugin.h"
#include "wm_single_instance.h"
#include "dms_global_mutex.h"
#include "dms_task_scheduler.h"
#include "screen_session_manager_adapter.h"
#include "zidl/idisplay_manager_agent.h"
#include "wm_common.h"
#include "screen_sensor_mgr.h"
#include "fold_screen_base_controller.h"

namespace OHOS::Rosen {
namespace {
#if (defined(__aarch64__) || defined(__x86_64__))
const std::string EXT_PLUGIN_SO_PATH = "/system/lib64/libdm_extension.z.so";
#else
const std::string EXT_PLUGIN_SO_PATH = "/system/lib/libdm_extension.z.so";
#endif
const std::string SCREEN_SESSION_MANAGER_THREAD = "OS_ScreenSessionManager";
const std::string SCREEN_SESSION_MANAGER_SCREEN_POWER_THREAD = "OS_ScreenSessionManager_ScreenPower";
const std::string SCREEN_CAPTURE_PERMISSION = "ohos.permission.CAPTURE_SCREEN";
const std::string CUSTOM_SCREEN_CAPTURE_PERMISSION = "ohos.permission.CUSTOM_SCREEN_CAPTURE";
const std::string CUSTOM_SCREEN_RECORDING_PERMISSION = "ohos.permission.CUSTOM_SCREEN_RECORDING";
const std::string BOOTEVENT_BOOT_COMPLETED = "bootevent.boot.completed";
const std::string ACCESS_VIRTUAL_SCREEN_PERMISSION = "ohos.permission.ACCESS_VIRTUAL_SCREEN";
const std::string IS_PC_MODE_KEY = "persist.sceneboard.ispcmode";
const std::string PC_MODE_DPI_KEY = "pcModeDpi";
const int32_t CV_WAIT_SCREENON_MS = 300;
const int32_t CV_WAIT_SCREENOFF_MS = 1500;
const int32_t CV_WAIT_SCREENOFF_MS_MAX = 3000;
const int32_t CV_WAIT_SCBSWITCH_MS = 3000;
const int32_t CV_WAIT_USERSWITCH_MS = 3000;
#ifdef WM_MULTI_USR_ABILITY_ENABLE
const int64_t SWITCH_USER_DISPLAYMODE_CHANGE_DELAY = 500;
#endif
const int32_t SCREEN_OFF_MIN_DELAY_TIME = 300;
const int32_t SCREEN_WAIT_PICTURE_FRAME_TIME = 1500;
const std::string STATUS_FOLD_HALF = "-z";
const std::string STATUS_EXPAND = "-y";
const std::string STATUS_EXPAND_WITH_SECOND_EXPAND = "-yy";
const std::string STATUS_FOLD = "-p";
const std::string SETTING_LOCKED_KEY = "settings.general.accelerometer_rotation_status";
const ScreenId SCREEN_ID_DEFAULT = 0;
const ScreenId RS_ID_DEFAULT = 0;
const ScreenId SCREEN_ID_FULL = 0;
const ScreenId SCREEN_ID_MAIN = 5;
const ScreenId SCREEN_ID_PC_MAIN = 9;
const ScreenId MINIMUM_VIRTUAL_SCREEN_ID = 1000;
constexpr int32_t INVALID_SCB_PID = -1;
static bool g_foldScreenFlag = system::GetParameter("const.window.foldscreen.type", "") != "";
static const int32_t g_screenRotationOffSet = system::GetIntParameter<int32_t>("const.fold.screen_rotation.offset", 0);
static const int32_t ROTATION_90 = 1;
#ifdef FOLD_ABILITY_ENABLE
static const int32_t ROTATION_270 = 3;
constexpr int32_t REMOVE_DISPLAY_MODE = 0;
const uint32_t SCREEN_STATE_AOD_OP_TIMEOUT_MS = 300;
#endif
const unsigned int XCOLLIE_TIMEOUT_10S = 10;
constexpr int32_t CAST_WIRED_PROJECTION_START = 1005;
constexpr int32_t CAST_WIRED_PROJECTION_STOP = 1007;
constexpr uint32_t SLEEP_TIME_US = 10000;
constexpr int32_t RES_FAILURE_FOR_PRIVACY_WINDOW = -2;
constexpr int32_t IRREGULAR_REFRESH_RATE_SKIP_THRETHOLD = 10;
constexpr float EXTEND_SCREEN_DPI_DEFAULT_PARAMETER = 1.0f;
static const int32_t AUTO_ROTATE_OFF = 0;
static const int NOTIFY_EVENT_FOR_DUAL_FAILED = 0;
static const int NOTIFY_EVENT_FOR_DUAL_SUCESS = 1;
static const int NO_NEED_NOTIFY_EVENT_FOR_DUAL = 2;
static bool g_isPcDevice = false;
static float g_extendScreenDpiCoef = EXTEND_SCREEN_DPI_DEFAULT_PARAMETER;
static uint32_t g_internalWidth = 3120;
#ifdef WM_MULTI_SCREEN_CTL_ABILITY_ENABLE
constexpr uint32_t NUMBER_OF_PHYSICAL_SCREEN = 2;
constexpr bool ADD_VOTE = true;
constexpr bool REMOVE_VOTE = false;
constexpr uint32_t OLED_60_HZ = 60;
#endif
constexpr uint32_t FOUR_K_WIDTH = 3840;
constexpr uint32_t THREE_K_WIDTH = 3000;

const int32_t ROTATE_POLICY = system::GetIntParameter("const.window.device.rotate_policy", 0);
constexpr int32_t FOLDABLE_DEVICE { 2 };
constexpr float DEFAULT_PIVOT = 0.5f;
constexpr float DEFAULT_SCALE = 1.0f;
constexpr float SECONDARY_ROTATION_90 = 90.0F;
constexpr float SECONDARY_ROTATION_270 = 270.0F;
static const constexpr char* SET_SETTING_DPI_KEY {"default_display_dpi"};
const std::vector<std::string> ROTATION_DEFAULT = {"0", "1", "2", "3"};
const std::vector<std::string> ORIENTATION_DEFAULT = {"0", "1", "2", "3"};
const uint32_t MAX_INTERVAL_US = 1800000000; // 30分钟
const int32_t MAP_SIZE = 300;
const std::string NO_EXIST_BUNDLE_MANE = "null";
ScreenCache<int32_t, std::string> g_uidVersionMap(MAP_SIZE, NO_EXIST_BUNDLE_MANE);

const int32_t SCREEN_SCAN_TYPE = system::GetIntParameter<int32_t>("const.window.screen.scan_type", 0);
constexpr int32_t SCAN_TYPE_VERTICAL = 1;
constexpr uint32_t ROTATION_MOD = 4;

#ifdef WM_MULTI_SCREEN_ENABLE
const ScreenId SCREEN_ID_OUTER_ONLY = 0;
const std::string SCREEN_EXTEND = "extend";
const std::string SCREEN_MIRROR = "mirror";
const std::string MULTI_SCREEN_EXIT_STR = "exit";
const std::string MULTI_SCREEN_ENTER_STR = "enter";
const int32_t CV_WAIT_SCREEN_MASK_MS = 500;
#endif
const bool IS_COORDINATION_SUPPORT =
    OHOS::system::GetBoolParameter("const.window.foldabledevice.is_coordination_support", false);

const std::string FAULT_DESCRIPTION = "842003014";
const std::string FAULT_SUGGESTION = "542003014";
constexpr uint32_t COMMON_EVENT_SERVICE_ID = 3299;
const long GET_HDR_PIXELMAP_TIMEOUT = 2000;
const int32_t CV_WAIT_UPDATE_AVAILABLE_MS = 300;

const static uint32_t PIXMAP_VECTOR_SIZE = 2;
static const uint32_t SDR_PIXMAP = 0;
const bool IS_SUPPORT_PC_MODE = system::GetBoolParameter("const.window.support_window_pcmode_switch", false);
const ScreenId SCREEN_GROUP_ID_DEFAULT = 1;
const std::string SCREEN_NAME_EXTEND = "ExtendedDisplay";
const std::string SCREEN_NAME_CAST = "CastEngine";
const std::set<std::string> INDIVIDUAL_SCREEN_GROUP_SET = {"CeliaView", "DevEcoViewer", "Cooperation-multi", "HwCast_AppModeDisplay"};

const int32_t MAIN_STATUS_WIDTH = 0;
const int32_t MAIN_STATUS_HEIGHT = 1;
const int32_t FULL_STATUS_WIDTH = 2;
const int32_t FULL_STATUS_HEIGHT = 3;
const int32_t GLOBAL_FULL_STATUS_WIDTH = 4;
const int32_t SCREEN_HEIGHT = 5;
const int32_t FULL_STATUS_OFFSET_X = 6;
const int32_t FULL_STATUS_OFFSET_Y = 7;
const int32_t GLOBAL_FULL_STATUS_OFFSET_Y = 8;

constexpr int32_t STATUS_PARAM_VALID_INDEX = 4;
constexpr uint32_t MAIN_STATUS_DEFAULT_WIDTH = 1008;
constexpr uint32_t SCREEN_DEFAULT_HEIGHT = 2232;

const bool CORRECTION_ENABLE = system::GetIntParameter<int32_t>("const.system.sensor_correction_enable", 0) == 1;
const std::string DISPLAYMODE_CORRECTION = system::GetParameter("const.dms.rotation_correction", "");
constexpr uint32_t EXPECT_DISPLAY_MODE_CORRECTION_SIZE = 2;
constexpr int32_t PARAM_NUM_TEN = 10;
const int32_t AOD_POWER_ON = 0;
const int32_t AOD_POWER_OFF = 1;

const std::string DEVICE_TYPE = system::GetParameter("const.product.devicetype", "unknown");
const std::vector<std::string> CAPTURE_DEVICE = {"phone", "2in1", "tablet"};
constexpr uint32_t FLOATING_BALL_TYPE = 2145;

constexpr int32_t COLD_SWITCH_ANIMATE_TIMEOUT_MILLISECONDS = 3000;
constexpr int32_t HOT_SWITCH_ANIMATE_TIMEOUT_MILLISECONDS = 0;

constexpr float POSITION_Z_DEFAULT = 2.0f;
constexpr float POSITION_Z_HIGH = 3.0f;
constexpr float POSITION_Z_LOW = 1.0f;

const uint32_t FOLD_SCREEN_STATE_MACHINE_REF_COUNT = 1;
const uint32_t SCREEN_STATE_MACHINE_REF_COUNT = 0;

const uint32_t FREEZE_SCREEN_MAX_COUNT = 150;
const uint32_t FREEZE_SCREEN_RETRY_DELAY_MS = 2000;
const uint32_t UNFREEZE_SCREEN_DELAY_MS = 2000;
const std::string REAL_DEVICE_RADIUS = system::GetParameter("const.product.real_device_radius", "");

// based on the bundle_util
// LCOV_EXCL_START
inline int32_t GetUserIdByCallingUid()
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    TLOGD(WmsLogTag::WMS_MULTI_USER, "get calling uid(%{public}d)", uid);
    if (uid <= INVALID_UID) {
        TLOGE(WmsLogTag::WMS_MULTI_USER, "uid is illegal: %{public}d", uid);
        return INVALID_USER_ID;
    }
    return uid / BASE_USER_RANGE;
}
// LCOV_EXCL_STOP
} // namespace

WM_IMPLEMENT_SINGLE_INSTANCE(ScreenSessionManager)

void ScreenSessionManager::LoadDmsExtension()
{
    TLOGE(WmsLogTag::DMS, "LoadDmsExtension start");
    void* handler = nullptr;
    int32_t cnt = 0;
    int32_t retryTimes = 3;
    do {
        cnt++;
        handler = dlopen(EXT_PLUGIN_SO_PATH.c_str(), RTLD_NOW | RTLD_NODELETE);
        TLOGI(WmsLogTag::DMS, "dlopen %{public}s, retry cnt: %{public}d", EXT_PLUGIN_SO_PATH.c_str(), cnt);
        if (handler == nullptr) {
            TLOGE(WmsLogTag::DMS, "dlopen failed: %{public}s", dlerror());
        }
        usleep(SLEEP_TIME_US);
    } while (!handler && cnt < retryTimes);
}
bool ScreenSessionManager::GetScreenSessionMngSystemAbility()
{
    ScreenSessionManager::LoadDmsExtension();
    return SystemAbility::MakeAndRegisterAbility(&ScreenSessionManager::GetInstance());
}
const bool REGISTER_RESULT = !SceneBoardJudgement::IsSceneBoardEnabled() ? false : ScreenSessionManager::GetScreenSessionMngSystemAbility();

ScreenSessionManager::ScreenSessionManager()
    : SystemAbility(DISPLAY_MANAGER_SERVICE_SA_ID, true), rsInterface_(RSInterfaces::GetInstance())
{
    screenEventTracker_.RecordEvent("Dms construct.");
    LoadScreenSceneXml();
    screenOffDelay_ = CV_WAIT_SCREENOFF_MS;
    screenOnDelay_ = CV_WAIT_SCREENON_MS;
    taskScheduler_ = std::make_shared<SafeTaskScheduler>(SCREEN_SESSION_MANAGER_THREAD);
    screenPowerTaskScheduler_ = std::make_shared<SafeTaskScheduler>(SCREEN_SESSION_MANAGER_SCREEN_POWER_THREAD);
    ffrtQueueHelper_ = std::make_shared<FfrtQueueHelper>();
    screenCutoutController_ = new (std::nothrow) ScreenCutoutController();
    if (!screenCutoutController_) {
        TLOGE(WmsLogTag::DMS, "screenCutoutController_ is nullptr");
        return;
    }
    sessionDisplayPowerController_ = new SessionDisplayPowerController(
        std::bind(&ScreenSessionManager::NotifyDisplayStateChange, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        g_foldScreenFlag = false;
    }
    ScreenStateMachine::GetInstance().SetTransitionState(ScreenTransitionState::SCREEN_INIT);
    if (g_foldScreenFlag) {
        ScreenStateMachine::GetInstance().InitStateMachine(FOLD_SCREEN_STATE_MACHINE_REF_COUNT);
        HandleFoldScreenPowerInit();
    } else {
        ScreenStateMachine::GetInstance().InitStateMachine(SCREEN_STATE_MACHINE_REF_COUNT);
    }
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        InitSecondaryDisplayPhysicalParams();
    }
    SetFirstSCBConnect(true);
    WatchParameter(BOOTEVENT_BOOT_COMPLETED.c_str(), BootFinishedCallback, this);
    isSupportCapture_ = IsSupportCapture();
}

bool SortByScreenId(const ScreenId& screenIdA, const ScreenId& screenIdB)
{
    return static_cast<int32_t>(screenIdA) < static_cast<int32_t>(screenIdB);
}

bool ScreenSessionManager::GetPcStatus() const
{
    std::lock_guard<std::mutex> lock(setPcStatusMutex_);
    return g_isPcDevice;
}

void ScreenSessionManager::SetPcStatus(bool isPc) {
    std::lock_guard<std::mutex> lock(setPcStatusMutex_);
    g_isPcDevice = isPc;
}

// LCOV_EXCL_START
ScreenRotation ScreenSessionManager::ConvertOffsetToCorrectRotation(int32_t phyOffset)
{
    ScreenRotation offsetRotation = ScreenRotation::ROTATION_0;
    switch (phyOffset) {
        case 90: // Rotation 90 degree
            offsetRotation = ScreenRotation::ROTATION_270;
            break;
        case 180: // Rotation 180 degree
            offsetRotation = ScreenRotation::ROTATION_180;
            break;
        case 270: // Rotation 270 degree
            offsetRotation = ScreenRotation::ROTATION_90;
            break;
        default:
            offsetRotation = ScreenRotation::ROTATION_0;
            break;
    }
    return offsetRotation;
}

Rotation ScreenSessionManager::ConvertIntToRotation(int32_t rotation)
{
    Rotation targetRotation = Rotation::ROTATION_0;
    switch (rotation) {
        case 90:
            targetRotation = Rotation::ROTATION_90;
            break;
        case 180:
            targetRotation = Rotation::ROTATION_180;
            break;
        case 270:
            targetRotation = Rotation::ROTATION_270;
            break;
        default:
            targetRotation = Rotation::ROTATION_0;
            break;
    }
    return targetRotation;
}

void ScreenSessionManager::HandleFoldScreenPowerInit()
{
#ifdef FOLD_ABILITY_ENABLE
    TLOGI(WmsLogTag::DMS, "Enter");
    if (FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) {
        foldScreenController_ = new (std::nothrow) DMS::FoldScreenBaseController();
    } else {
        foldScreenController_ = new (std::nothrow) FoldScreenController(displayInfoMutex_, screenPowerTaskScheduler_);
    }
    
    if (!foldScreenController_) {
        TLOGE(WmsLogTag::DMS, "foldScreenController_ is nullptr");
        return;
    }
    foldScreenController_->SetOnBootAnimation(true);
    if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        SetFoldScreenPowerInit([&]() {
            foldScreenController_->BootAnimationFinishPowerInit();
            FixPowerStatus();
            foldScreenController_->SetOnBootAnimation(false);
            RegisterApplicationStateObserver();
        });
    } else {
        // 后续其他设备rs上电规格将陆续迁移到BootAnimationFinishPowerInit中
        FoldScreenPowerInit();
    }
#endif
}

bool ScreenSessionManager::IsSupportCoordination()
{
    return !FoldScreenStateInternel::IsDualDisplayFoldDevice() || IS_COORDINATION_SUPPORT;
}

void ScreenSessionManager::FoldScreenPowerInit()
{
#ifdef FOLD_ABILITY_ENABLE
    SetFoldScreenPowerInit([&]() {
        int64_t timeStamp = 50;
#ifdef TP_FEATURE_ENABLE
        int32_t tpType = 12;
        std::string fullTpChange = "0";
        std::string mainTpChange = "1";
#endif
        if (!foldScreenController_) {
            TLOGE(WmsLogTag::DMS, "foldScreenController_ is nullptr");
            return;
        }
        foldScreenController_->SetIsClearingBootAnimation(true);
        ScreenId currentScreenId = foldScreenController_->GetCurrentScreenId();
        if (currentScreenId == SCREEN_ID_FULL) {
            TLOGI(WmsLogTag::DMS, "ScreenSessionManager Fold Screen Power Full animation Init 1.");
            SetRSScreenPowerStatusExt(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF_FAKE);
            if (IsSupportCoordination()) {
                SetRSScreenPowerStatusExt(SCREEN_ID_MAIN, ScreenPowerStatus::POWER_STATUS_ON);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(timeStamp));
            TLOGI(WmsLogTag::DMS, "ScreenSessionManager Fold Screen Power Full animation Init 2.");
#ifdef TP_FEATURE_ENABLE
            rsInterface_.SetTpFeatureConfig(tpType, fullTpChange.c_str());
#endif
            SetRSScreenPowerStatusExt(SCREEN_ID_MAIN, ScreenPowerStatus::POWER_STATUS_OFF);
            foldScreenController_->AddOrRemoveDisplayNodeToTree(SCREEN_ID_MAIN, REMOVE_DISPLAY_MODE);
            SetRSScreenPowerStatusExt(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_ON);
        } else if (currentScreenId == SCREEN_ID_MAIN) {
            TLOGI(WmsLogTag::DMS, "ScreenSessionManager Fold Screen Power Main animation Init 3.");
            SetRSScreenPowerStatusExt(SCREEN_ID_MAIN, ScreenPowerStatus::POWER_STATUS_OFF_FAKE);
            if (IsSupportCoordination()) {
                SetRSScreenPowerStatusExt(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_ON);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(timeStamp));
            TLOGI(WmsLogTag::DMS, "ScreenSessionManager Fold Screen Power Main animation Init 4.");
#ifdef TP_FEATURE_ENABLE
            rsInterface_.SetTpFeatureConfig(tpType, mainTpChange.c_str());
#endif
            SetRSScreenPowerStatusExt(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF);
            foldScreenController_->AddOrRemoveDisplayNodeToTree(SCREEN_ID_FULL, REMOVE_DISPLAY_MODE);
            SetRSScreenPowerStatusExt(SCREEN_ID_MAIN, ScreenPowerStatus::POWER_STATUS_ON);
        } else {
            TLOGI(WmsLogTag::DMS, "ScreenSessionManager Fold Screen Power Init, invalid active screen id");
        }
        ScreenStateMachine::GetInstance().IncScreenStateInitRef();
        FixPowerStatus();
        foldScreenController_->SetIsClearingBootAnimation(false);
        foldScreenController_->SetOnBootAnimation(false);
        RegisterApplicationStateObserver();
    });
#endif
}

void ScreenSessionManager::FixPowerStatus()
{
    if (!PowerMgr::PowerMgrClient::GetInstance().IsScreenOn()) {
        PowerMgr::PowerMgrClient::GetInstance().WakeupDeviceAsync();
        TLOGI(WmsLogTag::DMS, "Fix Screen Power State");
    }
}

void ScreenSessionManager::Init()
{
    if (ScreenSceneConfig::GetExternalScreenDefaultMode() == "none" ||
        system::GetBoolParameter(IS_PC_MODE_KEY, false)) {
        g_isPcDevice = true;
    }
    if (system::GetParameter("soc.boot.mode", "") != "rescue") {
        uint64_t interval = g_isPcDevice ? 10 * 1000 : 5 * 1000; // 10 second for PC
        if (HiviewDFX::Watchdog::GetInstance().AddThread(
            SCREEN_SESSION_MANAGER_THREAD, taskScheduler_->GetEventHandler(), interval)) {
            TLOGW(WmsLogTag::DMS, "Add thread %{public}s to watchdog failed.", SCREEN_SESSION_MANAGER_THREAD.c_str());
        }

        if (HiviewDFX::Watchdog::GetInstance().AddThread(
            SCREEN_SESSION_MANAGER_SCREEN_POWER_THREAD, screenPowerTaskScheduler_->GetEventHandler(), interval)) {
            TLOGW(WmsLogTag::DMS, "Add thread %{public}s to watchdog failed.",
                SCREEN_SESSION_MANAGER_SCREEN_POWER_THREAD.c_str());
        }
    } else {
        TLOGI(WmsLogTag::DMS, "Dms in rescue mode, not need watchdog.");
        screenEventTracker_.RecordEvent("Dms in rescue mode, not need watchdog.");
    }
    auto stringConfig = ScreenSceneConfig::GetStringConfig();
    if (stringConfig.count("defaultDisplayCutoutPath") != 0) {
        std::string defaultDisplayCutoutPath = static_cast<std::string>(stringConfig["defaultDisplayCutoutPath"]);
        TLOGD(WmsLogTag::DMS, "defaultDisplayCutoutPath = %{public}s.", defaultDisplayCutoutPath.c_str());
        ScreenSceneConfig::SetCutoutSvgPath(GetDefaultScreenId(), defaultDisplayCutoutPath);
    }
    if (!LoadMotionSensor()) {
        screenEventTracker_.RecordEvent("Dms load motion plugin failed.");
        TLOGW(WmsLogTag::DMS, "load motion plugin failed.");
    }
    AodLibInit();
    RegisterScreenChangeListener();
    if(FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        RegisterFoldNotSwitchingListener();
    }
    if (!ScreenSceneConfig::IsSupportRotateWithSensor()) {
        TLOGI(WmsLogTag::DMS, "Current type not support SetSensorSubscriptionEnabled.");
    } else if (GetScreenPower(SCREEN_ID_FULL) == ScreenPowerState::POWER_ON || GetScreenPower(SCREEN_ID_MAIN) == ScreenPowerState::POWER_ON) {
        // 多屏设备只要有屏幕亮,GetScreenPower获取的任意一块屏幕状态均是ON
        SetSensorSubscriptionEnabled();
        screenEventTracker_.RecordEvent("Dms subscribed to sensor successfully.");
    }
    // publish init
    ScreenSessionPublish::GetInstance().InitPublishEvents();
    screenEventTracker_.RecordEvent("Dms init end.");
}

void ScreenSessionManager::AodLibInit()
{
    static bool isNeedLoadAodLib = FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice() ||
        FoldScreenStateInternel::IsDualDisplayFoldDevice() ||
        FoldScreenStateInternel::IsSingleDisplayFoldDevice() ||
        FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice();
    if (isNeedLoadAodLib) {
        if (!LoadAodLib()) {
            TLOGE(WmsLogTag::DMS, "load aod lib failed");
        }
    }
}

void ScreenSessionManager::OnStart()
{
    TLOGI(WmsLogTag::DMS, "start");
    DmsXcollie dmsXcollie("DMS:OnStart", XCOLLIE_TIMEOUT_10S,
        [this](void *) { screenEventTracker_.LogWarningAllInfos(); });
    Init();
    sptr<ScreenSessionManager> dms(this);
    dms->IncStrongRef(nullptr);
    if (!Publish(dms)) {
        TLOGE(WmsLogTag::DMS, "Publish DMS failed");
        return;
    }
    TLOGI(WmsLogTag::DMS, "DMS SA AddSystemAbilityListener");
    (void)AddSystemAbilityListener(SENSOR_SERVICE_ABILITY_ID);
    (void)AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    (void)AddSystemAbilityListener(MULTIMODAL_INPUT_SERVICE_ID);
    screenEventTracker_.RecordEvent("Dms AddSystemAbilityListener finished.");
    TLOGI(WmsLogTag::DMS, "end");
    screenEventTracker_.RecordEvent("Dms onstart end.");
}

void ScreenSessionManager::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "OnAddSystemAbility: %d", systemAbilityId);
    TLOGI(WmsLogTag::DMS, "receive sa add:%{public}d", systemAbilityId);
    if (systemAbilityId == MULTIMODAL_INPUT_SERVICE_ID) {
        if (!IsNeedAddInputServiceAbility()) {
            return;
        }
        SwitchSubscriberInit();
        TLOGI(WmsLogTag::DMS, " SwitchSubscriber finished.");
    }
    if (systemAbilityId == SENSOR_SERVICE_ABILITY_ID) {
#if defined(SENSOR_ENABLE) && defined(FOLD_ABILITY_ENABLE)
        if (!g_foldScreenFlag) {
            TLOGI(WmsLogTag::DMS, "current device is not fold phone.");
            return;
        }
        if (!foldScreenController_ || isFoldScreenOuterScreenReady_) {
            TLOGI(WmsLogTag::DMS, "foldScreenController_ is null or outer screen is not ready.");
            return;
        }
        if (GetDisplayState(foldScreenController_->GetCurrentScreenId()) == DisplayState::ON) {
            DMS::ScreenSensorMgr::GetInstance().RegisterPostureCallback();
            TLOGI(WmsLogTag::DMS, "Recover Posture sensor finished");
        }

        DMS::ScreenSensorMgr::GetInstance().RegisterHallCallback();
        TLOGI(WmsLogTag::DMS, "Recover Hall sensor finished");
        screenEventTracker_.RecordEvent("Dms recover Posture and Hall sensor finished.");
#endif
    } else if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        auto task = []() {
            ScreenSessionPublish::GetInstance().RegisterLowTempSubscriber();
            ScreenSessionPublish::GetInstance().RegisterUserSwitchedSubscriber();
        };
        taskScheduler_->PostAsyncTask(task, "RegisterCommonEventSubscriber");
    }
}

bool ScreenSessionManager::IsNeedAddInputServiceAbility()
{
    TLOGI(WmsLogTag::DMS, "current device is not pc");
    return false;
}

DMError ScreenSessionManager::CheckDisplayMangerAgentTypeAndPermission(
    const sptr<IDisplayManagerAgent>& displayManagerAgent, DisplayManagerAgentType type)
{
    if ((type == DisplayManagerAgentType::SCREEN_EVENT_LISTENER ||
        type == DisplayManagerAgentType::PRIVATE_WINDOW_LISTENER) &&
        !SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    if ((displayManagerAgent == nullptr) || (displayManagerAgent->AsObject() == nullptr)) {
        TLOGE(WmsLogTag::DMS, "displayManagerAgent invalid");
        return DMError::DM_ERROR_NULLPTR;
    }
    return DMError::DM_OK;
}

DMError ScreenSessionManager::RegisterDisplayManagerAgent(
    const sptr<IDisplayManagerAgent>& displayManagerAgent, DisplayManagerAgentType type)
{
    TLOGI(WmsLogTag::DMS, " called type: %{public}u", type);
    DmsXcollie dmsXcollie("DMS:RegisterDisplayManagerAgent", XCOLLIE_TIMEOUT_10S);
    DMError ret;

    ret = CheckDisplayMangerAgentTypeAndPermission(displayManagerAgent, type);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "call CheckDisplayMangerAgentTypeAndPermission fail!");
        return ret;
    }

    if (type < DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER
        || type >= DisplayManagerAgentType::DISPLAY_MANAGER_MAX_AGENT_TYPE) {
        TLOGE(WmsLogTag::DMS, "DisplayManagerAgentType: %{public}u", static_cast<uint32_t>(type));
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (type == DisplayManagerAgentType::DISPLAY_EVENT_LISTENER) {
        auto uid = IPCSkeleton::GetCallingUid();
        auto pid = IPCSkeleton::GetCallingPid();
        {
            std::shared_lock<std::shared_mutex> lock(hookInfoMutex_);
            uidAndPidMap_[uid] = pid;
        }
    }
    if (type == DisplayManagerAgentType::BRIGHTNESS_INFO_CHANGED_LISTENER &&
        ScreenSessionManagerAdapter::GetInstance().dmAgentContainer_.GetAgentsByType(
        DisplayManagerAgentType::BRIGHTNESS_INFO_CHANGED_LISTENER).size() == 0) {
        RegisterBrightnessInfoChangeListener();
        TLOGI(WmsLogTag::DMS, "RegisterBrightnessInfoChangeListener");
    }
    return ScreenSessionManagerAdapter::GetInstance().dmAgentContainer_.RegisterAgent(
        displayManagerAgent, type) ? DMError::DM_OK : DMError::DM_ERROR_NULLPTR;
}

DMError ScreenSessionManager::UnregisterDisplayManagerAgent(
    const sptr<IDisplayManagerAgent>& displayManagerAgent, DisplayManagerAgentType type)
{
    TLOGI(WmsLogTag::DMS, " called type: %{public}u", type);
    DMError ret;

    ret = CheckDisplayMangerAgentTypeAndPermission(displayManagerAgent, type);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "call CheckDisplayMangerAgentTypeAndPermission fail!");
        return ret;
    }
    auto res = ScreenSessionManagerAdapter::GetInstance().dmAgentContainer_.UnregisterAgent(displayManagerAgent, type);
    if (type == DisplayManagerAgentType::BRIGHTNESS_INFO_CHANGED_LISTENER &&
        ScreenSessionManagerAdapter::GetInstance().dmAgentContainer_.GetAgentsByType(
        DisplayManagerAgentType::BRIGHTNESS_INFO_CHANGED_LISTENER).size() == 0) {
        UnregisterBrightnessInfoChangeListener();
        TLOGI(WmsLogTag::DMS, "UnregisterBrightnessInfoChangeListener");
    }
    return res ? DMError::DM_OK : DMError::DM_ERROR_NULLPTR;
}
// LCOV_EXCL_STOP

void ScreenSessionManager::LoadScreenSceneXml()
{
    if (ScreenSceneConfig::LoadConfigXml()) {
        ScreenSceneConfig::DumpConfig();
        if (!ScreenSceneConfig::GetDisplaysConfigs().empty()) {
            ScreenSceneConfig::DumpDisplaysConfigs();
        }
        ConfigureScreenScene();
    }
}

void ScreenSessionManager::ConfigureScreenScene()
{
    auto numbersConfig = ScreenSceneConfig::GetIntNumbersConfig();
    auto enableConfig = ScreenSceneConfig::GetEnableConfig();
    auto stringConfig = ScreenSceneConfig::GetStringConfig();
    ConfigureDpi();
    if (numbersConfig.count("defaultDeviceRotationOffset") != 0) {
        defaultDeviceRotationOffset_ = static_cast<uint32_t>(numbersConfig["defaultDeviceRotationOffset"][0]);
        TLOGD(WmsLogTag::DMS, "defaultDeviceRotationOffset = %{public}u", defaultDeviceRotationOffset_);
    }
    if (enableConfig.count("isWaterfallDisplay") != 0) {
        bool isWaterfallDisplay = static_cast<bool>(enableConfig["isWaterfallDisplay"]);
        TLOGD(WmsLogTag::DMS, "isWaterfallDisplay = %{public}d", isWaterfallDisplay);
    }
    if (numbersConfig.count("curvedScreenBoundary") != 0) {
        std::vector<int> vtBoundary = static_cast<std::vector<int>>(numbersConfig["curvedScreenBoundary"]);
        TLOGD(WmsLogTag::DMS, "vtBoundary.size=%{public}u", static_cast<uint32_t>(vtBoundary.size()));
    }
    if (stringConfig.count("subDisplayCutoutPath") != 0) {
        std::string subDisplayCutoutPath = stringConfig["subDisplayCutoutPath"];
        TLOGD(WmsLogTag::DMS, "subDisplayCutoutPath = %{public}s.", subDisplayCutoutPath.c_str());
        ScreenSceneConfig::SetSubCutoutSvgPath(subDisplayCutoutPath);
    }
    if (stringConfig.count("rotationPolicy") != 0) {
        std::string rotationPolicy = stringConfig["rotationPolicy"];
        TLOGD(WmsLogTag::DMS, "rotationPolicy = %{public}s.", rotationPolicy.c_str());
        deviceScreenConfig_.rotationPolicy_ = rotationPolicy;
    }
    if (stringConfig.count("defaultRotationPolicy") != 0) {
        std::string defaultRotationPolicy = stringConfig["defaultRotationPolicy"];
        TLOGD(WmsLogTag::DMS, "defaultRotationPolicy = %{public}s.", defaultRotationPolicy.c_str());
        deviceScreenConfig_.defaultRotationPolicy_ = defaultRotationPolicy;
    }
    if (enableConfig.count("isRightPowerButton") != 0) {
        bool isRightPowerButton = static_cast<bool>(enableConfig["isRightPowerButton"]);
        TLOGD(WmsLogTag::DMS, "isRightPowerButton = %{public}d", isRightPowerButton);
        deviceScreenConfig_.isRightPowerButton_ = isRightPowerButton;
    }
    ConfigureWaterfallDisplayCompressionParams();
    ConfigureCastParams();

    if (numbersConfig.count("buildInDefaultOrientation") != 0) {
        Orientation orientation = static_cast<Orientation>(numbersConfig["buildInDefaultOrientation"][0]);
        TLOGD(WmsLogTag::DMS, "orientation = %{public}d", orientation);
    }
    {
        std::lock_guard<std::mutex> lock(allDisplayPhysicalResolutionMutex_);
        allDisplayPhysicalResolution_ = ScreenSceneConfig::GetAllDisplayPhysicalConfig();
    }
}

void ScreenSessionManager::ConfigureDpi()
{
    auto numbersConfig = ScreenSceneConfig::GetIntNumbersConfig();
    if (numbersConfig.count("dpi") != 0) {
        uint32_t densityDpi = static_cast<uint32_t>(numbersConfig["dpi"][0]);
        TLOGI(WmsLogTag::DMS, "densityDpi = %{public}u", densityDpi);
        if (densityDpi >= DOT_PER_INCH_MINIMUM_VALUE && densityDpi <= DOT_PER_INCH_MAXIMUM_VALUE) {
            isDensityDpiLoad_ = true;
            defaultDpi = densityDpi;
            cachedSettingDpi_ = defaultDpi;
            densityDpi_ = static_cast<float>(densityDpi) / BASELINE_DENSITY;
        }
    }
    if (numbersConfig.count("subDpi") != 0) {
        uint32_t subDensityDpi = static_cast<uint32_t>(numbersConfig["subDpi"][0]);
        TLOGI(WmsLogTag::DMS, "subDensityDpi = %{public}u", subDensityDpi);
        if (subDensityDpi >= DOT_PER_INCH_MINIMUM_VALUE && subDensityDpi <= DOT_PER_INCH_MAXIMUM_VALUE) {
            isDensityDpiLoad_ = true;
            subDensityDpi_ = static_cast<float>(subDensityDpi) / BASELINE_DENSITY;
        }
    }
    const bool isPcMode = system::GetBoolParameter(IS_PC_MODE_KEY, false);
    if (isPcMode && numbersConfig.count(PC_MODE_DPI_KEY) != 0 && numbersConfig[PC_MODE_DPI_KEY].size() > 0) {
        uint32_t pcModeDpi = static_cast<uint32_t>(numbersConfig[PC_MODE_DPI_KEY][0]);
        if (pcModeDpi >= DOT_PER_INCH_MINIMUM_VALUE && pcModeDpi <= DOT_PER_INCH_MAXIMUM_VALUE) {
            pcModeDpi_ = pcModeDpi;
            isDensityDpiLoad_ = true;
            defaultDpi = pcModeDpi_;
            cachedSettingDpi_ = pcModeDpi_;
            densityDpi_ = static_cast<float>(pcModeDpi_) / BASELINE_DENSITY;
        }
        TLOGI(WmsLogTag::DMS, "config pcmode densityDpi: %{public}f", densityDpi_);
    }
}

void ScreenSessionManager::ConfigureCastParams()
{
    auto stringConfig = ScreenSceneConfig::GetStringConfig();
    if (stringConfig.count("castBundleName") == 0) {
        TLOGE(WmsLogTag::DMS, "not find cast bundleName in config xml");
        return;
    }
    std::string castBundleName = static_cast<std::string>(stringConfig["castBundleName"]);
    TLOGD(WmsLogTag::DMS, "castBundleName = %{public}s", castBundleName.c_str());
    ScreenCastConnection::GetInstance().SetBundleName(castBundleName);
    if (stringConfig.count("castAbilityName") == 0) {
        TLOGE(WmsLogTag::DMS, "not find cast ability in config xml");
        return;
    }
    std::string castAbilityName = static_cast<std::string>(stringConfig["castAbilityName"]);
    TLOGD(WmsLogTag::DMS, "castAbilityName = %{public}s", castAbilityName.c_str());
    ScreenCastConnection::GetInstance().SetAbilityName(castAbilityName);
}

void ScreenSessionManager::ConfigureWaterfallDisplayCompressionParams()
{
    auto enableConfig = ScreenSceneConfig::GetEnableConfig();
    if (enableConfig.count("isWaterfallAreaCompressionEnableWhenHorizontal") != 0) {
        bool enable = static_cast<bool>(enableConfig["isWaterfallAreaCompressionEnableWhenHorizontal"]);
        TLOGD(WmsLogTag::DMS, "isWaterfallAreaCompressionEnableWhenHorizontal=%{public}d.", enable);
    }
    ScreenSceneConfig::SetCurvedCompressionAreaInLandscape();
}

// LCOV_EXCL_START
void ScreenSessionManager::ConfigureScreenSnapshotParams()
{
    auto stringConfig = ScreenSceneConfig::GetStringConfig();
    if (stringConfig.count("screenSnapshotBundleName") == 0) {
        TLOGE(WmsLogTag::DMS, "not find screen snapshot bundleName in config xml");
        return;
    }
    std::string screenSnapshotBundleName = static_cast<std::string>(stringConfig["screenSnapshotBundleName"]);
    TLOGD(WmsLogTag::DMS, "screenSnapshotBundleName = %{public}s.", screenSnapshotBundleName.c_str());
    ScreenSnapshotPickerConnection::GetInstance().SetBundleName(screenSnapshotBundleName);
    if (stringConfig.count("screenSnapshotAbilityName") == 0) {
        TLOGE(WmsLogTag::DMS, "not find screen snapshot ability in config xml");
        return;
    }
    std::string screenSnapshotAbilityName = static_cast<std::string>(stringConfig["screenSnapshotAbilityName"]);
    TLOGD(WmsLogTag::DMS, "screenSnapshotAbilityName = %{public}s.", screenSnapshotAbilityName.c_str());
    ScreenSnapshotPickerConnection::GetInstance().SetAbilityName(screenSnapshotAbilityName);
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
void ScreenSessionManager::RegisterScreenChangeListener()
{
    TLOGI(WmsLogTag::DMS, "start");
    auto res = rsInterface_.SetScreenChangeCallback(
        DmUtils::wrap_callback([this](ScreenId screenId, ScreenEvent screenEvent, ScreenChangeReason reason) {
            OnScreenChange(screenId, screenEvent, reason);
        })
    );
    if (res != StatusCode::SUCCESS) {
        auto task = [this]() { RegisterScreenChangeListener(); };
        taskScheduler_->PostAsyncTask(task, "RegisterScreenChangeListener", 50);  // Retry after 50 ms.
        screenEventTracker_.RecordEvent("Dms OnScreenChange register failed.");
    } else {
        screenEventTracker_.RecordEvent("Dms OnScreenChange register success.");
    }
}

void ScreenSessionManager::RegisterFoldNotSwitchingListener()
{
    TLOGI(WmsLogTag::DMS, "start");
    auto res = rsInterface_.SetScreenSwitchingNotifyCallback(
        [this](bool isSwitching) {
            OnFoldStatusChange(isSwitching);
        });
    if (res != StatusCode::SUCCESS) {
        auto task = [this]() { RegisterFoldNotSwitchingListener(); };
        taskScheduler_->PostAsyncTask(task, "RegisterFoldNotSwitchingListener", 50);  // Retry after 50 ms.
        screenEventTracker_.RecordEvent("Dms fold not switching register failed.");
    } else {
        screenEventTracker_.RecordEvent("Dms fold not switching register success.");
    }
}

void ScreenSessionManager::ReportFoldDisplayTime(uint64_t screenId, int64_t rsFirstFrameTime)
{
#ifdef FOLD_ABILITY_ENABLE
    TLOGI(WmsLogTag::DMS, "[UL_FOLD]ReportFoldDisplayTime rsFirstFrameTime: %{public}" PRId64
            ", screenId: %{public}" PRIu64, rsFirstFrameTime, screenId);
    if (foldScreenController_ != nullptr && foldScreenController_->GetIsFirstFrameCommitReported() == false) {
        int64_t foldStartTime = foldScreenController_->GetStartTimePoint().time_since_epoch().count();
        int32_t ret = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
            "DISPLAY_MODE",
            OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "FOLD_TIME", rsFirstFrameTime - foldStartTime);
        foldScreenController_->SetIsFirstFrameCommitReported(true);
        if (ret != 0) {
            TLOGE(WmsLogTag::DMS, "[UL_FOLD]ReportFoldDisplayTime Write HiSysEvent error, ret: %{public}d", ret);
        }
    }
#endif
}

void ScreenSessionManager::RegisterFirstFrameCommitCallback()
{
    TLOGI(WmsLogTag::DMS, "[UL_FOLD]RegisterFirstFrameCommitCallback start");
    auto callback = [=](uint64_t screenId, int64_t rsFirstFrameTime) {
        ReportFoldDisplayTime(screenId, rsFirstFrameTime);
    };
    RSInterfaces::GetInstance().RegisterFirstFrameCommitCallback(DmUtils::wrap_callback(callback));
}
// LCOV_EXCL_STOP

void ScreenSessionManager::RegisterRefreshRateChangeListener()
{
    static bool isRegisterRefreshRateListener = false;
    // LCOV_EXCL_START
    if (!isRegisterRefreshRateListener) {
        TLOGW(WmsLogTag::DMS, "call rsInterface_ RegisterHgmRefreshRateUpdateCallback");
        auto res = rsInterface_.RegisterHgmRefreshRateUpdateCallback(
            DmUtils::wrap_callback([this](uint32_t refreshRate) { OnHgmRefreshRateChange(refreshRate); })
        );
        TLOGW(WmsLogTag::DMS, "call rsInterface_ RegisterHgmRefreshRateUpdateCallback end");
        if (res != StatusCode::SUCCESS) {
            TLOGE(WmsLogTag::DMS, "failed");
            screenEventTracker_.RecordEvent("Dms RefreshRateChange register failed.");
        } else {
            isRegisterRefreshRateListener = true;
            screenEventTracker_.RecordEvent("Dms RefreshRateChange register success.");
        }
    }
    // LCOV_EXCL_STOP
}

void ScreenSessionManager::NotifyTentModeChange(TentMode tentMode)
{
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGW(WmsLogTag::DMS, "clientProxy null");
        return;
    }
    tentMode_ = tentMode;
    TLOGD(WmsLogTag::DMS, "change tentMode to: %{public}" PRIu32, static_cast<uint32_t>(tentMode));
    clientProxy->OnTentModeChange(tentMode);
}

void ScreenSessionManager::OnVirtualScreenChange(ScreenId screenId, ScreenEvent screenEvent,
    const UniqueScreenRotationOptions& rotationOptions)
{
    TLOGI(WmsLogTag::DMS, "Notify scb virtual screen change, ScreenId: %{public}" PRIu64 ", ScreenEvent: %{public}d",
        screenId, static_cast<int>(screenEvent));
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    if (screenEvent == ScreenEvent::CONNECTED) {
        auto clientProxy = GetClientProxy();
        TLOGI(WmsLogTag::DMS, "screenCombination:%{public}d, screenName:%{public}s",
            screenSession->GetScreenCombination(), screenSession->GetName().c_str());
        if(screenSession->GetScreenCombination() == ScreenCombination::SCREEN_UNIQUE &&
            INDIVIDUAL_SCREEN_GROUP_SET.find(screenSession->GetName()) != INDIVIDUAL_SCREEN_GROUP_SET.end()) {
            screenSession->SetDisplayGroupId(displayGroupNum_++);
            screenSession->SetMainDisplayIdOfGroup(screenId);
        } else {
            // Unique screens outside of INDIVIDUAL_SCREEN_GROUP_SET are placed in the default group.
            screenSession->SetDisplayGroupId(DISPLAY_GROUP_ID_DEFAULT);
            screenSession->SetMainDisplayIdOfGroup(MAIN_SCREEN_ID_DEFAULT);
        }
        if (clientProxy) {
            clientProxy->OnScreenConnectionChanged(GetSessionOption(screenSession, screenId, rotationOptions),
                ScreenEvent::CONNECTED);
        }
        return;
    }
    if (screenEvent == ScreenEvent::DISCONNECTED) {
        std::map<int32_t, sptr<IScreenSessionManagerClient>> multiClientProxyMap;
        {
            std::lock_guard<std::mutex> lock(multiClientProxyMapMutex_);
            multiClientProxyMap = multiClientProxyMap_;
        }
        for (const auto& [userId, clientProxy] : multiClientProxyMap) {
            if (clientProxy != nullptr) {
                clientProxy->OnScreenConnectionChanged(GetSessionOption(screenSession, screenId),
                    ScreenEvent::DISCONNECTED);
            }
        }
    }
}

void ScreenSessionManager::OnVirtualScreenChange(ScreenId screenId, ScreenEvent screenEvent)
{
    return OnVirtualScreenChange(screenId, screenEvent, UniqueScreenRotationOptions());
}

// LCOV_EXCL_START
bool ScreenSessionManager::IsDefaultMirrorMode(ScreenId screenId)
{
    if (IsConcurrentUser()) {
        return false;
    }
    if (screenId != SCREEN_ID_MAIN && screenId != SCREEN_ID_FULL && screenId != SCREEN_ID_PC_MAIN) {
        return true;
    }
    return false;
}

void ScreenSessionManager::FreeDisplayMirrorNodeInner(const sptr<ScreenSession> mirrorSession)
{
    if (mirrorSession == nullptr) {
        return;
    }
    {
        std::shared_ptr<RSDisplayNode> displayNode = mirrorSession->GetDisplayNode();
        if (displayNode == nullptr) {
            return;
        }
        NotifyCaptureStatusChangedGlobal();
        displayNode->RemoveFromTree();
        mirrorSession->ReleaseDisplayNode();
        displayNode = nullptr;
    }
    TLOGI(WmsLogTag::DMS, "free displayNode");
    RSTransactionAdapter::FlushImplicitTransaction(mirrorSession->GetRSUIContext());
}

void ScreenSessionManager::NotifyCaptureStatusChangedGlobal()
{
    hdmiScreenCount_ = hdmiScreenCount_ > 0 ? hdmiScreenCount_ - 1 : 0;
    NotifyCaptureStatusChanged();
}

void ScreenSessionManager::SetScreenCorrection()
{
    ScreenRotation screenRotation = ScreenRotation::ROTATION_0;
    ScreenId screenId = SCREEN_ID_DEFAULT;
    std::ostringstream oss;
    if (g_foldScreenFlag) {
        if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
            screenRotation = static_cast<ScreenRotation>(ROTATION_90);
            screenId = SCREEN_ID_MAIN;
        } else {
            screenRotation = static_cast<ScreenRotation>(g_screenRotationOffSet);
            screenId = SCREEN_ID_FULL;
        }
    } else {
        std::vector<std::string> phyOffsets = FoldScreenStateInternel::GetPhyRotationOffset();
        int32_t phyOffset = static_cast<int32_t>(std::stoi(phyOffsets[0]));
        screenRotation = ConvertOffsetToCorrectRotation(phyOffset);
    }
    auto rotationOffset = GetConfigCorrectionByDisplayMode(GetFoldDisplayMode());
    auto rotation = (static_cast<int32_t>(screenRotation) + static_cast<int32_t>(rotationOffset)) % ROTATION_MOD;
    auto ret = rsInterface_.SetScreenCorrection(screenId, static_cast<ScreenRotation>(rotation));
    oss << "screenRotation: " << static_cast<int32_t>(screenRotation) << " ret value: " << ret;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    screenEventTracker_.RecordEvent(oss.str());
}

void ScreenSessionManager::AdaptSuperHorizonalBoot(sptr<ScreenSession> screenSession, ScreenId screenId)
{
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && screenSession->isInternal_) {
        auto screenMode = rsInterface_.GetScreenActiveMode(screenId);
        int32_t screenWidth = screenMode.GetScreenWidth();
        int32_t screenHeight = screenMode.GetScreenHeight();
        RRect screenBounds = RRect({ 0, 0, screenHeight, screenWidth }, 0.0f, 0.0f);
        screenSession->SetBounds(screenBounds);
        screenSession->SetHorizontalRotation();
        screenSession->SetValidWidth(screenHeight);
        screenSession->SetValidHeight(screenWidth);
    }
}

ScreenId ScreenSessionManager::GenerateSmsScreenId(ScreenId rsScreenId)
{
    const std::vector<DisplayConfig>& displaysConfigs = ScreenSceneConfig::GetDisplaysConfigs();
    for (const auto& conf : displaysConfigs) {
        if (conf.physicalId != rsScreenId || conf.logicalId == INVALID_SCREEN_ID) {
            continue;
        }
        screenIdManager_.UpdateScreenId(rsScreenId, conf.logicalId);
        return conf.logicalId;
    }
    screenIdManager_.UpdateScreenId(rsScreenId, rsScreenId);
    return rsScreenId;
}

void ScreenSessionManager::OnScreenChange(ScreenId screenId, ScreenEvent screenEvent, ScreenChangeReason reason)
{
    if (reason == ScreenChangeReason::HWCDEAD && screenEvent == ScreenEvent::DISCONNECTED) {
        TLOGW(WmsLogTag::DMS, "composer dead, ignore");
        return;
    }
    if (reason == ScreenChangeReason::HWCDEAD && GetScreenSession(GenerateSmsScreenId(screenId))) {
        TLOGW(WmsLogTag::DMS, "composer restart and screensession connected");
        // composer restart, notify powermgr poweron pc outer screen
        if (screenId != SCREEN_ID_DEFAULT && g_isPcDevice) {
            NotifyAbnormalScreenConnectChange(screenId);
            TLOGW(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " ScreenChangeReason: %{public}d",
                screenId, static_cast<int>(reason));
        }
        return;
    }
    OnScreenChangeDefault(screenId, screenEvent, reason);
}

void ScreenSessionManager::OnScreenChangeDefault(ScreenId screenId, ScreenEvent screenEvent, ScreenChangeReason reason)
{
    std::ostringstream oss;
    oss << "OnScreenChange triggered. screenId: " << static_cast<int32_t>(screenId)
        << "  screenEvent: " << static_cast<int32_t>(screenEvent);
    screenEventTracker_.RecordEvent(oss.str());
    TLOGW(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " screenEvent: %{public}d",
        screenId, static_cast<int>(screenEvent));
    SetScreenCorrection();
    auto screenSession = GetOrCreateScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    if (g_isPcDevice) {
        auto physicalScreenSession = GetOrCreatePhysicalScreenSession(screenId);
        if (!physicalScreenSession) {
            TLOGE(WmsLogTag::DMS, "physicalScreenSession is nullptr");
            return;
        }
    }
    OnFoldScreenChange(screenSession);
    if (screenEvent == ScreenEvent::CONNECTED) {
        connectScreenNumber_ ++;
        HandleScreenConnectEvent(screenSession, screenId, screenEvent);
    } else if (screenEvent == ScreenEvent::DISCONNECTED) {
        connectScreenNumber_ --;
        HandleScreenDisconnectEvent(screenSession, screenId, screenEvent);
    } else {
        TLOGE(WmsLogTag::DMS, "screenEvent error!");
    }
    NotifyScreenModeChange();
}

void ScreenSessionManager::OnFoldScreenChange(sptr<ScreenSession>& screenSession)
{
#ifdef FOLD_ABILITY_ENABLE
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        SuperFoldSensorManager::GetInstance().SetTaskScheduler(screenPowerTaskScheduler_);
        SuperFoldSensorManager::GetInstance().RegisterPostureCallback();
        SuperFoldSensorManager::GetInstance().RegisterHallCallback();
        SetSensorSubscriptionEnabled();
        screenEventTracker_.RecordEvent("Dms subscribed to sensor successfully.");
    }

    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        SecondaryFoldSensorManager::GetInstance().RegisterPostureCallback();
        SecondaryFoldSensorManager::GetInstance().RegisterHallCallback();
        SetSensorSubscriptionEnabled();
        screenEventTracker_.RecordEvent("secondary device: Dms subscribed to sensor successfully.");
    }

    if (foldScreenController_ != nullptr) {
        RegisterFirstFrameCommitCallback();
        screenSession->SetFoldScreen(true);
    }
#endif
}

void ScreenSessionManager::OnFoldStatusChange(bool isSwitching)
{
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "fold screen controller is not initialized.");
        return;
    }
    if (!isSwitching) {
        TLOGI(WmsLogTag::DMS, "receive switching end.");
        foldScreenController_->SetdisplayModeChangeStatus(false);
    }
#endif
}

void ScreenSessionManager::DestroyExtendVirtualScreen()
{
    TLOGI(WmsLogTag::DMS, "start");
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMap;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        screenSessionMap = screenSessionMap_;
    }
    for (auto sessionIt : screenSessionMap) {
        sptr<ScreenSession> screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr, ScreenId: %{public}" PRIu64,
                sessionIt.first);
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::VIRTUAL &&
            screenSession->GetIsExtendVirtual()) {
            DestroyVirtualScreen(screenSession->GetScreenId());
            TLOGI(WmsLogTag::DMS, "destory screenId: %{public}" PRIu64, screenSession->GetScreenId());
        }
    }
}

void ScreenSessionManager::NotifyScreenModeChange(ScreenId disconnectedScreenId)
{
    TLOGI(WmsLogTag::DMS, "NotifyScreenModeChange start");
    auto task = [=] {
        std::vector<sptr<ScreenInfo>> screenInfos;
        std::vector<ScreenId> screenIds = GetAllScreenIds();
        for (auto screenId : screenIds) {
            if (disconnectedScreenId == screenId) {
                continue;
            }
            TLOGNI(WmsLogTag::DMS, "screenId:%{public}" PRIu64, screenId);
            auto screenSession = GetScreenSession(screenId);
            screenInfos.emplace_back(screenSession->ConvertToScreenInfo());
        }
        ScreenSessionManagerAdapter::GetInstance().NotifyScreenModeChange(screenInfos);
    };
    taskScheduler_->PostAsyncTask(task, "NotifyScreenModeChange");
}

void ScreenSessionManager::NotifyAbnormalScreenConnectChange(ScreenId screenId)
{
    ScreenSessionManagerAdapter::GetInstance().NotifyAbnormalScreenConnectChange(screenId);
}

void ScreenSessionManager::SendCastEvent(const bool &isPlugIn)
{
    TLOGI(WmsLogTag::DMS, "isPlugIn:%{public}d", isPlugIn);
    if (!ScreenCastConnection::GetInstance().CastConnectExtension(static_cast<int32_t>(isPlugIn))) {
        TLOGE(WmsLogTag::DMS, "CastConnectionExtension failed");
        return;
    }
    if (!ScreenCastConnection::GetInstance().IsConnectedSync()) {
        TLOGE(WmsLogTag::DMS, "CastConnectionExtension connected failed");
        ScreenCastConnection::GetInstance().CastDisconnectExtension();
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    if (isPlugIn) {
        ScreenCastConnection::GetInstance().SendMessageToCastService(CAST_WIRED_PROJECTION_START, data, reply);
    } else {
        ScreenCastConnection::GetInstance().SendMessageToCastService(CAST_WIRED_PROJECTION_STOP, data, reply);
    }
    ScreenCastConnection::GetInstance().CastDisconnectExtension();
}

void ScreenSessionManager::NotifyCastWhenScreenConnectChange(bool isConnected)
{
    if (g_isPcDevice) {
        TLOGI(WmsLogTag::DMS, "pc device");
        return;
    }
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    if (isConnected) {
        auto task = [this]() {
            SendCastEvent(true);
            ScreenSessionPublish::GetInstance().PublishCastPlugInEvent();
        };
        taskScheduler_->PostAsyncTask(task, "SendCastEventTrue");
        TLOGI(WmsLogTag::DMS, "PostAsyncTask SendCastEventTrue");
    } else {
        auto task = [this]() {
            SendCastEvent(false);
            ScreenSessionPublish::GetInstance().PublishCastPlugOutEvent();
        };
        taskScheduler_->PostAsyncTask(task, "SendCastEventFalse");
        TLOGI(WmsLogTag::DMS, "PostAsyncTask SendCastEventFalse");
    }
}

void ScreenSessionManager::PhyMirrorConnectWakeupScreen()
{
#ifdef WM_MULTI_SCREEN_ENABLE
    if (ScreenSceneConfig::GetExternalScreenDefaultMode() == "mirror") {
        TLOGI(WmsLogTag::DMS, "Connect to an external screen to wakeup the phone screen");
        if (!IS_SUPPORT_PC_MODE) {
            FixPowerStatus();
        }
    }
#endif
}

bool ScreenSessionManager::GetIsCurrentInUseById(ScreenId screenId)
{
    auto session = GetScreenSession(screenId);
    if (session == nullptr) {
        TLOGE(WmsLogTag::DMS, "session not found");
        return false;
    }
    if (!session->GetIsCurrentInUse()) {
        TLOGE(WmsLogTag::DMS, "session not in use");
        return false;
    }
    return true;
}

void ScreenSessionManager::SetMultiScreenDefaultRelativePosition()
{
#ifdef WM_MULTI_SCREEN_ENABLE
    MultiScreenPositionOptions mainOptions;
    MultiScreenPositionOptions extendOptions;
    sptr<ScreenSession> mainSession = nullptr;
    sptr<ScreenSession> extendSession = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (auto sessionIt : screenSessionMap_) {
            auto screenSession = sessionIt.second;
            if (screenSession == nullptr) {
                TLOGI(WmsLogTag::DMS, "screenSession is nullptr, ScreenId: %{public}" PRIu64,
                    sessionIt.first);
                continue;
            }
            if (screenSession->GetIsRealScreen()) {
                if (screenSession->GetIsExtend() && extendSession == nullptr) {
                    extendSession = screenSession;
                } else {
                    mainSession = screenSession;
                }
            }
        }
    }
    if (extendSession != nullptr && mainSession != nullptr) {
        ScreenProperty mainProperty = mainSession->GetScreenProperty();
        int32_t mainWidth = mainProperty.GetBounds().rect_.GetWidth();
        int32_t mainHeight = mainProperty.GetBounds().rect_.GetHeight();
        if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && GetIsPhysicalExtendScreenConnected() &&
            mainHeight > mainWidth) {
            mainWidth = mainHeight;
        }
        if (g_isPcDevice) {
            mainOptions = { mainSession->GetRSScreenId(), 0, 0 };
            extendOptions = { extendSession->GetRSScreenId(), mainWidth, 0 };
        } else {
            mainOptions = { mainSession->GetScreenId(), 0, 0 };
            extendOptions = { extendSession->GetScreenId(), mainWidth, 0 };
        }
        auto ret = SetMultiScreenRelativePosition(mainOptions, extendOptions);
        if (ret != DMError::DM_OK) {
            TLOGE(WmsLogTag::DMS, "set Relative Position failed, DMError:%{public}d", static_cast<int32_t>(ret));
        }
    }
#endif
}

void ScreenSessionManager::GetAndMergeEdidInfo(sptr<ScreenSession> screenSession)
{
    ScreenId rsScreenId = screenSession->GetRSScreenId();
    struct BaseEdid edid;
    int32_t fillInfo = 4;
    if (!GetEdid(rsScreenId, edid)) {
        TLOGE(WmsLogTag::DMS, "get EDID failed.");
        return;
    }
    std::string serialNumber = ConvertEdidToString(edid);
    TLOGI(WmsLogTag::DMS, "serialNumber: %{public}s", serialNumber.c_str());
    screenSession->SetSerialNumber(serialNumber);
}

std::string ScreenSessionManager::ConvertEdidToString(const struct BaseEdid edid)
{
    std::string edidInfo = edid.manufacturerName_ + std::to_string(edid.productCode_)
        + std::to_string(edid.serialNumber_) + std::to_string(edid.weekOfManufactureOrModelYearFlag_)
        + std::to_string(edid.yearOfManufactureOrModelYear_);
    TLOGI(WmsLogTag::DMS, "edidInfo: %{public}s", edidInfo.c_str());
    std::hash<std::string> hasher;
    std::size_t hashValue = hasher(edidInfo);
    std::ostringstream oss;
    oss << std::hex << std::uppercase << hashValue;
    return oss.str();
}

void ScreenSessionManager::LockLandExtendIfScreenInfoNull(sptr<ScreenSession>& screenSession)
{
#ifdef FOLD_ABILITY_ENABLE
   if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        TLOGI(WmsLogTag::DMS, "not superFoldDisplayDevice");
        return;
   }
   if (GetIsPhysicalExtendScreenConnected()) {
        SetIsExtendModelocked(true);
        SetExpandAndHorizontalLocked(true);
   }
#endif
}

bool ScreenSessionManager::RecoverRestoredMultiScreenMode(sptr<ScreenSession> screenSession)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    if (screenSession->GetScreenProperty().GetScreenType() != ScreenType::REAL) {
        TLOGI(WmsLogTag::DMS, "not real screen, return before recover.");
        return true;
    }
    std::map<std::string, MultiScreenInfo> multiScreenInfoMap = ScreenSettingHelper::GetMultiScreenInfo();
    std::string serialNumber = screenSession->GetSerialNumber();
    if (!CheckMultiScreenInfoMap(multiScreenInfoMap, serialNumber)) {
        if (IsDefaultMirrorMode(screenSession->GetRSScreenId())) {
                TLOGI(WmsLogTag::DMS, "new user recover recommended resolution rsid: %{public}" PRIu64,
                    screenSession->GetRSScreenId());
                RecoverScreenActiveMode(screenSession->GetRSScreenId());
        }
        LockLandExtendIfScreenInfoNull(screenSession);
        return false;
    }
    auto info = multiScreenInfoMap[serialNumber];
    if (info.outerOnly) {
        SetIsOuterOnlyMode(true);
        RecoverScreenActiveMode(info.secondaryScreenOption.screenId_);
        MultiScreenModeChange(SCREEN_ID_OUTER_ONLY, SCREEN_ID_OUTER_ONLY, "off");
        return true;
    }
    sptr<ScreenSession> internalSession = GetInternalScreenSession();
    if (internalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "internalSession is nullptr");
        return false;
    }
    if (info.isExtendMain) {
        TLOGI(WmsLogTag::DMS, "extend screen is main");
        info.mainScreenOption.screenId_ = screenSession->GetRSScreenId();
        info.secondaryScreenOption.screenId_ = internalSession->GetRSScreenId();
        RecoverScreenActiveMode(info.mainScreenOption.screenId_);
    } else {
        TLOGI(WmsLogTag::DMS, "extend screen is not main");
        info.mainScreenOption.screenId_ = internalSession->GetRSScreenId();
        info.secondaryScreenOption.screenId_ = screenSession->GetRSScreenId();
        RecoverScreenActiveMode(info.secondaryScreenOption.screenId_);
    }
    if (info.multiScreenMode == MultiScreenMode::SCREEN_MIRROR) {
        SetMultiScreenMode(info.mainScreenOption.screenId_, info.secondaryScreenOption.screenId_, info.multiScreenMode);
        TLOGW(WmsLogTag::DMS, "mirror, return befor OnScreenConnectionChanged");
        ReportHandleScreenEvent(ScreenEvent::CONNECTED, ScreenCombination::SCREEN_MIRROR);
        return true;
    }
    SetMultiScreenMode(info.mainScreenOption.screenId_, info.secondaryScreenOption.screenId_, info.multiScreenMode);
    auto ret = SetMultiScreenRelativePosition(info.mainScreenOption, info.secondaryScreenOption);
    if (ret != DMError::DM_OK) {
        SetMultiScreenDefaultRelativePosition();
    }
    ReportHandleScreenEvent(ScreenEvent::CONNECTED, ScreenCombination::SCREEN_EXTEND);
    return true;
#else
    return false;
#endif
}

void ScreenSessionManager::RecoverScreenActiveMode(ScreenId rsScreenId)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetScreenPower");
    auto state = RSInterfaces::GetInstance().GetScreenPowerStatus(rsScreenId);
    if(state != ScreenPowerStatus::POWER_STATUS_ON){
        TLOGW(WmsLogTag::DMS,"rsid:%{public}" PRIu64",screen power is : %{public}u,on need recover",
            rsScreenId,static_cast<uint32_t>(state));
        return;
    }
    sptr<ScreenSession> screenSession = GetScreenSessionByRsId(rsScreenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr, rsid: %{public}" PRIu64, rsScreenId);
        return;
    }
    std::vector<sptr<SupportedScreenModes>> modes = screenSession->GetScreenModes();
    if(modes.empty()) {
        TLOGE(WmsLogTag::DMS, "mode is empty");
        return;
    }
    int32_t activeId = -1;
    std::map<std::string, SupportedScreenModes> resolutionMap = ScreenSettingHelper::GetResolutionMode();
    std::string serialNumber = screenSession->GetSerialNumber();
    if(CheckResolutionMode(resolutionMap, serialNumber)) {
        auto info = resolutionMap[serialNumber];
        activeId = GetActiveIdxInModes(modes,info);
    } else {
        // interface not support parameter zero
        activeId = static_cast<int32_t>(modes.size()) - 2;
    }
    TLOGI(WmsLogTag::DMS, "activeId: %{public}d", activeId);
    if(activeId < 0) {
        TLOGE(WmsLogTag::DMS, "no matching resolution, activeId: %{public}d", activeId);
        return;
    }
    DMError ret = SetScreenActiveMode(screenSession->GetScreenId(), static_cast<uint32_t>(activeId));
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "recover error, rsid:%{public}"  PRIu64", activeIdx:%{public}d",
            rsScreenId, activeId);
    }      
}

int32_t ScreenSessionManager::GetActiveIdxInModes(const std::vector<sptr<SupportedScreenModes>>& modes,
                          const SupportedScreenModes& edidInfo)
{
    for(size_t i = 0; i < modes.size() - 1; ++i)
    {
        const auto& mode = modes[i];
        if(mode == nullptr){
            TLOGE(WmsLogTag::DMS, "mode is nullptr!");
            continue;
        }
        TLOGI(WmsLogTag::DMS, "supportmodelist, id:%{public}u, w:%{public}u, h:%{public}u, refreshRate:%{public}u",
            mode->id_, mode->width_, mode->height_, mode->refreshRate_);
        if(mode->width_ == edidInfo.width_ &&
           mode->height_ == edidInfo.height_ &&
           mode->refreshRate_ == edidInfo.refreshRate_) {
            TLOGW(WmsLogTag::DMS,
                "remode, index:%{public}d, modeid: %{public}u, w:%{public}u, h:%{public}u, refreshRate:%{public}u",
                static_cast<int32_t>(i),mode->id_, mode->width_, mode->height_, mode->refreshRate_);
            return static_cast<int32_t>(i);
           }
    }
    return -1;
}

bool ScreenSessionManager::CheckMultiScreenInfoMap(std::map<std::string, MultiScreenInfo> multiScreenInfoMap,
    const std::string& serialNumber)
{
    if (multiScreenInfoMap.empty()) {
        TLOGE(WmsLogTag::DMS, "no restored screen, use default mode!");
        return false;
    }
    if (serialNumber.size() == 0) {
        TLOGE(WmsLogTag::DMS, "serialNumber empty!");
        return false;
    }
    if (multiScreenInfoMap.find(serialNumber) == multiScreenInfoMap.end()) {
        TLOGE(WmsLogTag::DMS, "screen not found, use default mode!");
        return false;
    }
    return true;
}

bool ScreenSessionManager::CheckResolutionMode(std::map<std::string, SupportedScreenModes> resolutionMap,
    const std::string& serialNumber)
{
    if (resolutionMap.empty()) {
        TLOGE(WmsLogTag::DMS, "no restored screen, use default mode!");
        return false;
    }
    if (serialNumber.size() == 0) {
        TLOGE(WmsLogTag::DMS, "serialNumber empty!");
        return false;
    }
    if (resolutionMap.find(serialNumber) == resolutionMap.end()) {
        TLOGE(WmsLogTag::DMS, "screen not found, use default mode!");
        return false;
    }
    return true;
}
void ScreenSessionManager::ReportHandleScreenEvent(ScreenEvent screenEvent, ScreenCombination screenCombination)
{
    MultiScreenMode multiScreenMode;
    if (screenCombination == ScreenCombination::SCREEN_EXTEND) {
        multiScreenMode = MultiScreenMode::SCREEN_EXTEND;
    } else {
        multiScreenMode = MultiScreenMode::SCREEN_MIRROR;
    }
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "EXTEND_DISPLAY_PLUG_IN_AND_OUT",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "PLUG_IN_AND_OUT", static_cast<int32_t>(screenEvent),
        "DISPLAY_CONNECT_NUM", connectScreenNumber_,
        "DISPLAY_INUSED_NUM", GetCurrentInUseScreenNumber(),
        "EXTENSION_DISPLAY_MODE_STATUS", static_cast<int32_t>(multiScreenMode),
        "MAIN_DISPLAY_ID", GetInternalScreenId());
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "Write HiSysEvent error, ret: %{public}d", ret);
    }
}

int32_t ScreenSessionManager::GetCurrentInUseScreenNumber()
{
    int32_t inUseScreenNumber_ = 0;
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::REAL &&
            screenSession->GetIsCurrentInUse()) {
            TLOGI(WmsLogTag::DMS, "found screenId = %{public}" PRIu64, sessionIt.first);
            inUseScreenNumber_++;
        }
    }
    return inUseScreenNumber_;
}

bool ScreenSessionManager::SetCastPrivacyToRS(sptr<ScreenSession> screenSession, bool enable)
{
    bool phyMirrorEnable = IsDefaultMirrorMode(screenSession->GetScreenId());
    if (screenSession->GetScreenProperty().GetScreenType() != ScreenType::REAL || !phyMirrorEnable) {
        TLOGE(WmsLogTag::DMS, "screen is not real or external, screenId:%{public}" PRIu64"",
            screenSession->GetScreenId());
        return false;
    }
    ScreenId rsScreenId = INVALID_SCREEN_ID;
    if (!screenIdManager_.ConvertToRsScreenId(screenSession->GetScreenId(), rsScreenId) ||
        rsScreenId == INVALID_SCREEN_ID) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId");
        return false;
    }
    rsInterface_.SetCastScreenEnableSkipWindow(rsScreenId, enable);
    return true;
}

void ScreenSessionManager::SetCastPrivacyFromSettingData()
{
    bool enable = true;
    bool isOK = ScreenSettingHelper::GetSettingCast(enable);
    TLOGI(WmsLogTag::DMS, "get setting cast done, isOK: %{public}u, enable: %{public}u", isOK, enable);
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (const auto& sessionIt : screenSessionMap_) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr, screenId:%{public}" PRIu64"", sessionIt.first);
            continue;
        }
        bool isSuc = SetCastPrivacyToRS(screenSession, enable);
        TLOGI(WmsLogTag::DMS, "set cast privacy done, isSuc:%{public}d, enable:%{public}d, screenId:%{public}" PRIu64"",
        isSuc, enable, sessionIt.first);
    }
}

void ScreenSessionManager::RegisterSettingWireCastObserver(sptr<ScreenSession>& screenSession)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    bool phyMirrorEnable = IsDefaultMirrorMode(screenSession->GetScreenId());
    if (!g_isPcDevice && phyMirrorEnable && screenSession->GetScreenProperty().GetScreenType() == ScreenType::REAL) {
        TLOGI(WmsLogTag::DMS, "Register Setting wire cast Observer");
        SettingObserver::UpdateFunc updateFunc = [this](const std::string& key) { SetCastPrivacyFromSettingData(); };
        ScreenSettingHelper::RegisterSettingWireCastObserver(DmUtils::wrap_callback(updateFunc));
    }
}

void ScreenSessionManager::UnregisterSettingWireCastObserver(ScreenId screenId)
{
    if (g_isPcDevice) {
        return;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (const auto& sessionIt : screenSessionMap_) {
            auto screenSession = sessionIt.second;
            if (screenSession == nullptr) {
                TLOGE(WmsLogTag::DMS, "screenSession is nullptr, screenId:%{public}" PRIu64"", sessionIt.first);
                continue;
            }
            bool phyMirrorEnable = IsDefaultMirrorMode(screenSession->GetScreenId());
            if (screenSession->GetScreenProperty().GetScreenType() != ScreenType::REAL || !phyMirrorEnable) {
                TLOGE(WmsLogTag::DMS, "screen is not real or external, screenId:%{public}" PRIu64"", sessionIt.first);
                continue;
            }
            if (screenSession ->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR &&
                screenSession->GetScreenId() != screenId) {
                return;
            }
        }
    }
    ScreenSettingHelper::UnregisterSettingWireCastObserver();
    TLOGI(WmsLogTag::DMS, "unregister Setting wire cast Observer");
}

void ScreenSessionManager::SwitchSubscriberInit()
{
    switchId_ = MMI::InputManager::GetInstance()->SubscribeSwitchEvent(
        [this](std::shared_ptr<OHOS::MMI::SwitchEvent> switchEvent) {
            if (switchEvent->GetSwitchValue() == MMI::SwitchEvent::SWITCH_OFF) {
                SetLapTopLidOpenStatus(false);
            } else {
                SetLapTopLidOpenStatus(true);
            }
        }
    );
    TLOGD(WmsLogTag::DMS, "switchId is: %{public}d", switchId_);
}

bool ScreenSessionManager::IsLapTopLidOpen() const
{
    return isLapTopLidOpen_.load();
}

void ScreenSessionManager::SetLapTopLidOpenStatus(bool isLapTopLidOpen)
{
    isLapTopLidOpen_.store(isLapTopLidOpen);
    TLOGI(WmsLogTag::DMS, "isLapTopLidOpen is: %{public}d", static_cast<int32_t>(isLapTopLidOpen));
}

void ScreenSessionManager::NotifyUserClientProxy(sptr<ScreenSession> screenSession, ScreenId screenId,
                                                 ScreenEvent screenEvent)
{
    TLOGW(WmsLogTag::DMS, "screen connect and notify to scb");
    DisplayId displayId = screenSession->GetDisplayId();
    int32_t userId = GetForegroundConcurrentUser(displayId);
    if (userId == INVALID_USER_ID) {
        TLOGW(WmsLogTag::DMS, "not notify to scb, scb not start, displayId: %{public}" PRIu64, displayId);
        return;
    }
    std::lock_guard<std::mutex> lock(multiClientProxyMapMutex_);
    auto iter = multiClientProxyMap_.find(userId);
    if (iter == multiClientProxyMap_.end()) {
        TLOGE(WmsLogTag::DMS, "not notify to scb, client proxy not found, displayId: %{public}" PRIu64"", displayId);
        return;
    }
    auto userClientProxy = iter->second;
    if (!userClientProxy) {
        TLOGE(WmsLogTag::DMS, "notify to scb, client proxy is nullptr, displayId: %{public}" PRIu64"", displayId);
        return;
    }
    userClientProxy->OnScreenConnectionChanged(GetSessionOption(screenSession, displayId), screenEvent);
    TLOGW(WmsLogTag::DMS, "screen connect and notify to scb, user: %{public}d, displayId: %{public}" PRIu64,
        currentUserIdForSettings_, screenId);
}

void ScreenSessionManager::HandleScreenConnectEvent(sptr<ScreenSession> screenSession,
    ScreenId screenId, ScreenEvent screenEvent)
{
    InitRotationCorrectionMap(DISPLAYMODE_CORRECTION);
    screenSession->SetRotationCorrectionMap(rotationCorrectionMap_);
    bool phyMirrorEnable = IsDefaultMirrorMode(screenId);
    HandlePhysicalMirrorConnect(screenSession, phyMirrorEnable);
    ScreenConnectionChanged(screenSession, screenId, screenEvent, phyMirrorEnable);
    if (phyMirrorEnable || (IsConcurrentUser() && screenId != SCREEN_ID_DEFAULT)) {
        sptr<ScreenSession> internalSession = GetScreenSession(SCREEN_ID_DEFAULT);
        TLOGW(WmsLogTag::DMS, "HandleScreenConnectEven. SCreenId: %{public}" PRIu64, internalSession->GetScreenId());
        NotifyScreenConnected(internalSession->ConvertToScreenInfo());
        // when display add, need wait update available area, ensure display info is accurate
        WaitUpdateAvailableAreaForPc();
        NotifyDisplayCreate(internalSession->ConvertToDisplayInfo());
    }
    TLOGW(WmsLogTag::DMS, "connect end. ScreenId: %{public}" PRIu64, screenId);
}

void ScreenSessionManager::HandleFoldDeviceScreenConnect(ScreenId screenId, const sptr<ScreenSession>& screenSession,
    bool phyMirrorEnable, ScreenEvent screenEvent)
{
    if (screenSession == nullptr) {
        return;
    }
    auto clientProxy = GetClientProxy();
    if ((screenId == 0 || (screenId == SCREEN_ID_MAIN && isCoordinationFlag_)) && clientProxy) {
        TLOGW(WmsLogTag::DMS, "event: connect %{public}" PRIu64 ", %{public}" PRIu64 ", "
            "name=%{public}s", screenId, screenSession->GetRSScreenId(), screenSession->GetName().c_str());
        clientProxy->OnScreenConnectionChanged(GetSessionOption(screenSession, screenId), screenEvent);
    }
    if (phyMirrorEnable) {
        NotifyScreenConnected(screenSession->ConvertToScreenInfo());
        NotifyDisplayCreate(screenSession->ConvertToDisplayInfo());
    }
}

void ScreenSessionManager::ScreenConnectionChanged(sptr<ScreenSession> screenSession,
    ScreenId screenId, ScreenEvent screenEvent, bool phyMirrorEnable)
{
    auto clientProxy = GetClientProxy();
#ifdef FOLD_ABILITY_ENABLE
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        SuperFoldStateManager::GetInstance().RefreshExternalRegion();
    }
    if (foldScreenController_ != nullptr) {
        HandleFoldDeviceScreenConnect(screenId, screenSession, phyMirrorEnable, screenEvent);
        return;
    }
#endif
    if (IsConcurrentUser()) {
        NotifyUserClientProxy(screenSession, screenId, screenEvent);
    } else if (clientProxy && !phyMirrorEnable) {
        TLOGW(WmsLogTag::DMS, "screen connect and notify to scb.");
        clientProxy->OnScreenConnectionChanged(GetSessionOption(screenSession, screenId), screenEvent);
    }
}

void ScreenSessionManager::WaitUpdateAvailableAreaForPc()
{
    std::unique_lock<std::mutex> lock(displayAddMutex_);
    TLOGI(WmsLogTag::DMS, "begin wait notify display. need wait: %{public}d", needWaitAvailableArea_);
    if (g_isPcDevice && needWaitAvailableArea_) {
        TLOGI(WmsLogTag::DMS, "need wait update available area");
        if (DmUtils::safe_wait_for(displayAddCV_, lock, std::chrono::milliseconds(CV_WAIT_UPDATE_AVAILABLE_MS)) ==
            std::cv_status::timeout) {
            TLOGE(WmsLogTag::DMS, "wait update available area timeout");
            needWaitAvailableArea_ = false;
        }
    }
}

void ScreenSessionManager::HandleScreenDisconnectEvent(sptr<ScreenSession> screenSession,
    ScreenId screenId, ScreenEvent screenEvent)
{
    if (!screenSession) {
       TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
       return;
    }
    ScreenId matchscreenId = screenSession->GetScreenId();
    if (IsConcurrentUser()) {
        std::map<DisplayId, std::map<int32_t, UserInfo>> tempMap;
        {
            std::lock_guard<std::mutex> lock(displayConcurrentUserMapMutex_);
            tempMap = displayConcurrentUserMap_;
        }
        // delete all displaynode if screenId matched in displayConcurrentUserMap_
        for (const auto& it : tempMap[matchscreenId]) {
            int32_t targetUserId = it.first;
            TLOGI(WmsLogTag::DMS, "find match userId %{public}d with screenId %{public}" PRIu64 "from userMap",
                targetUserId, matchscreenId);
            RemoveUserDisplayNode(targetUserId, matchscreenId);
        }
    } else {
        RemoveUserDisplayNode(currentUserId_, matchscreenId);
    }
    RemoveScreenCastInfo(screenId);
    if (g_isPcDevice) {
        ScreenId rsId = screenSession->GetRSScreenId();
        if (screenId != rsId && screenSession->GetScreenProperty().GetScreenType() == ScreenType::REAL) {
            screenSession = GetScreenSessionByRsId(screenId);
            if (!screenSession) {
                TLOGE(WmsLogTag::DMS, "screenSession is nullptr, rsid: %{public}" PRIu64, rsId);
                return;
            }
        }
    }
    HandlePCScreenDisconnect(screenSession);
    bool phyMirrorEnable = IsDefaultMirrorMode(screenId);
    HandlePhysicalMirrorDisconnect(screenSession, screenId, phyMirrorEnable);
    if (IsConcurrentUser()) {
        DisplayId displayId = screenSession->GetDisplayId();
        std::map<DisplayId, std::map<int32_t, UserInfo>> tempMap;
        {
            std::lock_guard<std::mutex> lock(displayConcurrentUserMapMutex_);
            tempMap = displayConcurrentUserMap_;
        }
        auto displayIt = tempMap.find(displayId);
        if (displayIt == tempMap.end()) {
            TLOGW(WmsLogTag::DMS, "Can't find screen in screenConcurrentUserMap,"
                "invalid displayId: %{public}" PRIu64, displayId);
            return;
        }

        const auto& userMap = displayIt->second;
        for (const auto& it : userMap) {
            sptr<IScreenSessionManagerClient> clientProxy;
            {
                std::lock_guard<std::mutex> lock(multiClientProxyMapMutex_);
                clientProxy = multiClientProxyMap_[it.first];
            }
            if (clientProxy) {
                TLOGI(WmsLogTag::DMS, "disconnect screen: %{public}" PRIu64 "userId: %{public}d", displayId, it.first);
                clientProxy->OnScreenConnectionChanged(GetSessionOption(screenSession, displayId),
                    ScreenEvent::DISCONNECTED);
            }
        }
    } else {
        auto clientProxy = GetClientProxy();
        if (clientProxy) {
            clientProxy->OnScreenConnectionChanged(GetSessionOption(screenSession, screenId),
                ScreenEvent::DISCONNECTED);
        }
    }
#ifdef WM_MULTI_SCREEN_ENABLE
    SetIsExtendModelocked(false);
    SetIsPhysicalExtendScreenConnected(false);
    SetExpandAndHorizontalLocked(false);
#endif
    HandleMapWhenScreenDisconnect(screenId);
    if (g_isPcDevice) {
        ScreenCombination screenCombination = screenSession->GetScreenCombination();
        ReportHandleScreenEvent(ScreenEvent::DISCONNECTED, screenCombination);
    }
    if (g_isPcDevice || IS_SUPPORT_PC_MODE) {
        SetMultiScreenFrameControl();
    }
    if (!(screenId == SCREEN_ID_MAIN && isCoordinationFlag_ == true)) {
        std::lock_guard<std::recursive_mutex> lock_phy(phyScreenPropMapMutex_);
        phyScreenPropMap_.erase(screenId);
    }
    if (phyMirrorEnable || (IsConcurrentUser() && screenId != SCREEN_ID_DEFAULT)) {
        NotifyScreenDisconnected(screenSession->GetScreenId());
        NotifyDisplayDestroy(screenSession->GetScreenId());
    }
    if (!g_isPcDevice && phyMirrorEnable) {
        UnregisterSettingWireCastObserver(screenId);
    }
    TLOGW(WmsLogTag::DMS, "disconnect success. ScreenId: %{public}" PRIu64 "", screenId);
}

void ScreenSessionManager::HandlePhysicalMirrorConnect(sptr<ScreenSession> screenSession, bool phyMirrorEnable)
{
    if (phyMirrorEnable) {
        PhyMirrorConnectWakeupScreen();
        NotifyCastWhenScreenConnectChange(true);
        isPhyScreenConnected_ = true;
        RegisterSettingWireCastObserver(screenSession);
        if (!g_isPcDevice) {
            ScreenPowerUtils::EnablePowerForceTimingOut();
            DisablePowerOffRenderControl(0);
        }
    }
}

void ScreenSessionManager::HandlePhysicalMirrorDisconnect(sptr<ScreenSession> screenSession, ScreenId screenId,
    bool phyMirrorEnable)
{
    if (phyMirrorEnable) {
        RecoveryResolutionEffect();
        NotifyCastWhenScreenConnectChange(false);
        FreeDisplayMirrorNodeInner(screenSession);
        isPhyScreenConnected_ = false;
        if (!g_isPcDevice) {
            std::vector<ScreenId> screenIdsToExclude = { screenId };
            if (!HasCastEngineOrPhyMirror(screenIdsToExclude)) {
                ScreenPowerUtils::DisablePowerForceTimingOut();
                ScreenPowerUtils::LightAndLockScreen("light and lock screen");
            }
        }
    }
}

void ScreenSessionManager::HandleMapWhenScreenDisconnect(ScreenId screenId)
{
    {
        ScreenId smsId = screenId;
        if (IsConcurrentUser()) {
            if (!screenIdManager_.ConvertToSmsScreenId(screenId, smsId)) {
                smsId = screenId;
            }
        }
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        screenSessionMap_.erase(smsId);
    }
    {
        std::lock_guard<std::recursive_mutex> lock(physicalScreenSessionMapMutex_);
        physicalScreenSessionMap_.erase(screenId);
    }
}

void ScreenSessionManager::HandlePCScreenDisconnect(sptr<ScreenSession>& screenSession)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    ScreenCombination screenCombination = screenSession->GetScreenCombination();
    if (!g_isPcDevice) {
        TLOGE(WmsLogTag::DMS, "not PC device, return before process.");
        return;
    }
    if (screenCombination == ScreenCombination::SCREEN_MAIN) {
        TLOGW(WmsLogTag::DMS, "reset to inner screen to main.");
        HandleMainScreenDisconnect(screenSession);
    }
    screenCombination = screenSession->GetScreenCombination();
    if (screenCombination == ScreenCombination::SCREEN_EXTEND) {
        MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_EXTEND, MULTI_SCREEN_EXIT_STR);
    } else {
        MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_MIRROR, MULTI_SCREEN_EXIT_STR);
        NotifyCaptureStatusChanged(false);
    }
    MultiScreenPositionOptions defaultOptions = { GetDefaultScreenId(), 0, 0 };
    SetRelativePositionForDisconnect(defaultOptions);
#endif
}

void ScreenSessionManager::HandleMainScreenDisconnect(sptr<ScreenSession>& screenSession)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    if (!g_isPcDevice) {
        TLOGE(WmsLogTag::DMS, "not PC device, return before process.");
        return;
    }
    sptr<ScreenSession> internalSession = GetInternalScreenSession();
    if (internalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "internalSession is nullptr");
        return;
    }
    if (GetIsOuterOnlyMode()) {
        TLOGI(WmsLogTag::DMS, "exit outer only mode.");
        SetIsOuterOnlyMode(false);
        ExitOuterOnlyMode(internalSession->GetRSScreenId(), screenSession->GetRSScreenId(),
            MultiScreenMode::SCREEN_MIRROR);
    } else {
        SetIsOuterOnlyMode(false);
        MultiScreenModeChange(internalSession->GetRSScreenId(), screenSession->GetRSScreenId(),
            SCREEN_MIRROR);
    }
    if (screenSession->GetScreenId() == screenSession->GetRSScreenId()) {
        TLOGW(WmsLogTag::DMS, "main screen changed, reset screenSession.");
        screenSession = internalSession;
    } else {
        if (!screenSession->GetIsInternal() && screenSession->GetIsCurrentInUse()) {
            TLOGW(WmsLogTag::DMS, "main screen not changed, reset internal session.");
            sptr<ScreenSession> newInternalSession = GetInternalScreenSession();
            ResetInternalScreenSession(newInternalSession, screenSession);
            screenSession = newInternalSession;
        }
    }
#endif
}

void ScreenSessionManager::ResetInternalScreenSession(sptr<ScreenSession>& innerScreen, sptr<ScreenSession>& externalScreen)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    if (innerScreen == nullptr || externalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen sessions null.");
        return;
    }
    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerScreen->GetScreenId()
        << ", rsId: " << innerScreen->GetRSScreenId()
        << ", externalScreen screenId: " << externalScreen->GetScreenId()
        << ", rsId: " << externalScreen->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

    /* reset display node */
    auto mainNode = innerScreen->GetDisplayNode();
    auto extendNode = externalScreen->GetDisplayNode();
    ScreenId innerRSId = innerScreen->GetRSScreenId();
    ScreenId externalRSId = externalScreen->GetRSScreenId();
    if (mainNode != nullptr) {
        mainNode->SetScreenId(externalRSId);
        RSInterfaces::GetInstance().SetScreenOffset(externalRSId, 0, 0);
    }
    if (extendNode != nullptr) {
        extendNode->SetScreenId(innerRSId);
        RSInterfaces::GetInstance().SetScreenOffset(innerRSId, 0, 0);
    }
    RSTransactionAdapter::FlushImplicitTransaction(
        {innerScreen->GetRSUIContext(), externalScreen->GetRSUIContext()});

    /* reset physical info */
    MultiScreenChangeUtils::ScreenPhysicalInfoChange(innerScreen, externalScreen);

    /* reset relative position */
    innerScreen->SetStartPosition(0, 0);
    externalScreen->SetStartPosition(0, 0);
    SetPhysicalStartPosition(innerScreen, 0, 0);
    SetPhysicalXYPosition(externalScreen, 0, 0);
    
    /* reset combination */
    innerScreen->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    externalScreen->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    innerScreen->SetIsExtend(true);
    externalScreen->SetIsExtend(false);

    /* set screen available */
    innerScreen->SetScreenAvailableStatus(true);
    externalScreen->SetScreenAvailableStatus(true);
#endif
}

void ScreenSessionManager::OnHgmRefreshRateChange(uint32_t refreshRate)
{
    GetDefaultScreenId();
    TLOGD(WmsLogTag::DMS, "Set refreshRate: %{public}u, defaultScreenId: %{public}" PRIu64"",
        refreshRate, defaultScreenId_);
    sptr<ScreenSession> screenSession = GetScreenSession(defaultScreenId_);
    if (screenSession) {
        screenSession->UpdateRefreshRate(refreshRate);
        NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(),
            DisplayChangeEvent::UPDATE_REFRESHRATE);
        UpdateCoordinationRefreshRate(refreshRate);
        UpdateSuperFoldRefreshRate(screenSession, refreshRate);
        std::map<ScreenId, sptr<ScreenSession>> screenSessionMap;
        {
            std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
            screenSessionMap = screenSessionMap_;
        }
        for (const auto &sessionItem : screenSessionMap) {
            const auto &screenSessionItem = sessionItem.second;
            const auto &screenType = screenSessionItem->GetScreenProperty().GetScreenType();
            if (screenType == ScreenType::VIRTUAL) {
                screenSessionItem->UpdateRefreshRate(refreshRate);
                screenSessionItem->SetSupportedRefreshRate({refreshRate});
                NotifyDisplayChanged(screenSessionItem->ConvertToDisplayInfo(),
                    DisplayChangeEvent::UPDATE_REFRESHRATE);
            }
        }
    } else {
        TLOGE(WmsLogTag::DMS, "Get default screen session failed.");
    }
    return;
}

void ScreenSessionManager::UpdateSuperFoldRefreshRate(sptr<ScreenSession> screenSession, uint32_t refreshRate)
{
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        TLOGD(WmsLogTag::DMS, "not super fold display device.");
        return;
    }
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    if (fakeScreenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "fakeScreenSession is nullptr");
        return;
    }
    fakeScreenSession->UpdateRefreshRate(refreshRate);
    if (screenSession->GetScreenProperty().GetIsFakeInUse()) {
        NotifyDisplayChanged(fakeScreenSession->ConvertToDisplayInfo(), DisplayChangeEvent::UPDATE_REFRESHRATE);
        TLOGE(WmsLogTag::DMS, "notify success");
    }
}

bool ScreenSessionManager::IsPhysicalScreenAndInUse(sptr<ScreenSession> screenSession) const
{
    if (!screenSession) {
        return false;
    }
    if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::REAL &&
        screenSession->GetIsCurrentInUse()) {
        return true;
    }
    return false;
}

void ScreenSessionManager::SetMultiScreenFrameControl(void)
{
#ifdef WM_MULTI_SCREEN_CTL_ABILITY_ENABLE
    uint32_t count = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (auto sessionIt : screenSessionMap_) {
            if (IsPhysicalScreenAndInUse(sessionIt.second)) {
                count++;
            }
        }
    }
    if (count >= NUMBER_OF_PHYSICAL_SCREEN) {
        TLOGW(WmsLogTag::DMS, "MultiScreen control frame rate to 60");
        EventInfo event = { "VOTER_MUTIPHSICALSCREEN", ADD_VOTE, OLED_60_HZ, OLED_60_HZ };
        rsInterface_.NotifyRefreshRateEvent(event);
    } else {
        TLOGW(WmsLogTag::DMS, "Disabling frame rate control");
        EventInfo event = { "VOTER_MUTIPHSICALSCREEN", REMOVE_VOTE };
        rsInterface_.NotifyRefreshRateEvent(event);
    }
#endif
}

sptr<ScreenSession> ScreenSessionManager::GetScreenSession(ScreenId screenId) const
{
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    if (screenSessionMap_.empty()) {
        screenEventTracker_.LogWarningAllInfos();
    }
    ScreenId smsId = screenId;
    if (IsConcurrentUser()) {
        if (!screenIdManager_.ConvertToSmsScreenId(screenId, smsId)) {
            smsId = screenId;
        }
    }
    auto iter = screenSessionMap_.find(smsId);
    if (iter == screenSessionMap_.end()) {
        TLOGW(WmsLogTag::DMS, "not found screen session id: %{public}" PRIu64".", screenId);
        return nullptr;
    }
    return iter->second;
}

sptr<ScreenSession> ScreenSessionManager::GetDefaultScreenSession()
{
    GetDefaultScreenId();
    return GetScreenSession(defaultScreenId_);
}

sptr<DisplayInfo> ScreenSessionManager::HookDisplayInfoByUid(sptr<DisplayInfo> displayInfo,
    const sptr<ScreenSession>& screenSession)
{
    if (displayInfo == nullptr) {
        TLOGI(WmsLogTag::DMS, "ConvertToDisplayInfo error, displayInfo is nullptr.");
        return nullptr;
    }
    auto uid = IPCSkeleton::GetCallingUid();
    std::shared_lock<std::shared_mutex> lock(hookInfoMutex_);
    if (displayHookMap_.find(uid) != displayHookMap_.end() && !displayHookMap_[uid].isFullScreenInForceSplit_) {
        auto info = displayHookMap_[uid];
        std::ostringstream oss;
        oss << "hW: " << info.width_ << ", hH: " << info.height_ << ", hD: " << info.density_
            << ", hR: " << info.rotation_ << ", hER: " << info.enableHookRotation_
            << " hO: " << info.displayOrientation_ << ", hEO: " << info.enableHookDisplayOrientation_
            << ", dW: " << displayInfo->GetHeight() << ", dH: " << displayInfo->GetHeight()
            << ", dR: " << static_cast<uint32_t>(displayInfo->GetRotation())
            << ", dO: " << static_cast<uint32_t>(displayInfo->GetDisplayOrientation());
        TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());

        displayInfo->SetWidth(info.width_);
        displayInfo->SetHeight(info.height_);
        displayInfo->SetVirtualPixelRatio(info.density_);
        if (info.enableHookRotation_) {
            if (screenSession) {
                Rotation targetRotation = screenSession->ConvertIntToRotation(static_cast<int32_t>(info.rotation_));
                displayInfo->SetRotation(targetRotation);
                DisplayOrientation targetOrientation = screenSession->CalcDisplayOrientation(targetRotation,
                    FoldDisplayMode::UNKNOWN);
                TLOGI(WmsLogTag::DMS, "tR: %{public}u, tO: %{public}u", targetRotation, targetOrientation);
                displayInfo->SetDisplayOrientation(targetOrientation);
            } else {
                TLOGI(WmsLogTag::DMS, "ConvertToDisplayInfo error, screenSession is nullptr.");
                return nullptr;
            }
        }
        if (info.enableHookDisplayOrientation_ &&
            info.displayOrientation_ < static_cast<uint32_t>(DisplayOrientation::UNKNOWN)) {
            displayInfo->SetDisplayOrientation(static_cast<DisplayOrientation>(info.displayOrientation_));
        }
    }
    return displayInfo;
}

sptr<DisplayInfo> ScreenSessionManager::GetDefaultDisplayInfo(int32_t userId)
{
    if (IsConcurrentUser()) {
        if (userId == CONCURRENT_USER_ID_DEFAULT) {
            userId = GetUserIdByCallingUid();
            TLOGI(WmsLogTag::DMS, "concurrentuser open, get userId %{public}u by calling uid", userId);
        }
    } else {
        userId = USER_ID_DEFAULT;
    }
    ScreenId screenId = INVALID_SCREEN_ID;
    if (userId == USER_ID_DEFAULT) {
        screenId = GetDefaultScreenId();
    } else {
        screenId = GetUserDisplayId(userId);
    }
    TLOGD(WmsLogTag::DMS, "get screenId %{public}" PRIu64" with userId %{public}u", screenId, userId);
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    std::lock_guard<std::recursive_mutex> lock_info(displayInfoMutex_);
    if (screenSession) {
        sptr<DisplayInfo> displayInfo = screenSession->ConvertToDisplayInfo();
        if (displayInfo == nullptr) {
            TLOGI(WmsLogTag::DMS, "ConvertToDisplayInfo error, displayInfo is nullptr.");
            return nullptr;
        }
        HandleRotationCorrectionExemption(displayInfo);
        displayInfo = HookDisplayInfoByUid(displayInfo, screenSession);
        return displayInfo;
    } else {
        TLOGE(WmsLogTag::DMS, "failed");
        return nullptr;
    }
}

sptr<DisplayInfo> ScreenSessionManager::GetDisplayInfoById(DisplayId displayId)
{
    TLOGD(WmsLogTag::DMS, "enter, displayId: %{public}" PRIu64" ", displayId);
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGI(WmsLogTag::DMS, "screenSession is nullptr, ScreenId: %{public}" PRIu64 "",
                sessionIt.first);
            continue;
        }
        sptr<DisplayInfo> displayInfo = screenSession->ConvertToDisplayInfo();
        if (displayInfo == nullptr) {
            TLOGI(WmsLogTag::DMS, "ConvertToDisplayInfo error, displayInfo is nullptr.");
            continue;
        }
        HandleRotationCorrectionExemption(displayInfo);
        if (displayId == displayInfo->GetDisplayId()) {
            TLOGD(WmsLogTag::DMS, "success");
            displayInfo = HookDisplayInfoByUid(displayInfo, screenSession);
            return displayInfo;
        }
        if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
            continue;
        }
        if (!screenSession->GetScreenProperty().GetIsFakeInUse()) {
            continue;
        }
        sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
        if (fakeScreenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "error, fakeScreenSession is nullptr.");
            continue;
        }
        sptr<DisplayInfo> fakeDisplayInfo = fakeScreenSession->ConvertToDisplayInfo();
        if (fakeDisplayInfo == nullptr) {
            TLOGE(WmsLogTag::DMS, "error, fakeDisplayInfo is nullptr.");
            continue;
        }
        HandleRotationCorrectionExemption(fakeDisplayInfo);
        fakeDisplayInfo = HookDisplayInfoByUid(fakeDisplayInfo, fakeScreenSession);
        DisplayId fakeDisplayId = fakeDisplayInfo->GetDisplayId();
        if (displayId == fakeDisplayId) {
            TLOGD(WmsLogTag::DMS, "find fake success");
            return fakeDisplayInfo;
        }
    }
    TLOGE(WmsLogTag::DMS, "failed. displayId: %{public}" PRIu64" ", displayId);
    return nullptr;
}

void ScreenSessionManager::HandleRotationCorrectionExemption(sptr<DisplayInfo>& displayInfo)
{
    if (!CORRECTION_ENABLE) {
        return;
    }
    std::vector<std::string> rotationCorrectionExemptionList;
    {
        std::shared_lock<std::shared_mutex> lock(rotationCorrectionExemptionMutex_);
        if (rotationCorrectionExemptionList_.empty()) {
            TLOGI(WmsLogTag::DMS, "rotationCorrectionExemptionList empty, return");
            return;
        }
        rotationCorrectionExemptionList = rotationCorrectionExemptionList_;
    }
    std::string bundleName = SysCapUtil::GetBundleName();
    if (std::find(rotationCorrectionExemptionList.begin(), rotationCorrectionExemptionList.end(), bundleName) !=
        rotationCorrectionExemptionList.end()) {
        FoldDisplayMode foldDisplayMode = GetFoldDisplayMode();
        FoldDisplayMode foldDisplayModeAfterRotation = GetFoldDisplayModeAfterRotation();
        if (foldDisplayModeAfterRotation != FoldDisplayMode::UNKNOWN) {
            foldDisplayMode = foldDisplayModeAfterRotation;
        }
        Rotation rotation = RemoveRotationCorrection(displayInfo->GetRotation(), foldDisplayMode);
        displayInfo->SetRotation(rotation);
        TLOGI(WmsLogTag::DMS, "rotation: %{public}d, bundleName: %{public}s", rotation, bundleName.c_str());
    }
}
 
void ScreenSessionManager::GetRotationCorrectionExemptionListFromDatabase(bool isForce)
{
    {
        std::shared_lock<std::shared_mutex> lock(rotationCorrectionExemptionMutex_);
        if (!isForce && !(rotationCorrectionExemptionList_.empty() && needReinstallExemptionList_)) {
            TLOGD(WmsLogTag::DMS, "no need query databese. needReinstall: %{public}d, exemptionListIsempty: %{public}d",
                rotationCorrectionExemptionList_.empty(), needReinstallExemptionList_);
            return;
        }
    }
    std::vector<std::string> rotationCorrectionExemptionList;
    bool ret = ScreenSettingHelper::GetRotationCorrectionExemptionList(rotationCorrectionExemptionList);
    if (!ret) {
        TLOGE(WmsLogTag::DMS, "get correction exemption list failed");
        std::unique_lock<std::shared_mutex> lock(rotationCorrectionExemptionMutex_);
        needReinstallExemptionList_ = true;
        return;
    }
    std::unique_lock<std::shared_mutex> lock(rotationCorrectionExemptionMutex_);
    rotationCorrectionExemptionList_ = rotationCorrectionExemptionList;
    if (rotationCorrectionExemptionList.size() <= 0) {
        TLOGE(WmsLogTag::DMS, "rotationCorrectionExemptionList is empty");
        needReinstallExemptionList_ = false;
        return;
    }
    TLOGI(WmsLogTag::DMS, "success");
}
 
void ScreenSessionManager::RegisterRotationCorrectionExemptionListObserver()
{
    if (!CORRECTION_ENABLE) {
        return;
    }
    TLOGI(WmsLogTag::DMS, "register start");
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) { GetRotationCorrectionExemptionListFromDatabase(true); };
    ScreenSettingHelper::RegisterRotationCorrectionExemptionListObserver(updateFunc);
}

sptr<DisplayInfo> ScreenSessionManager::GetVisibleAreaDisplayInfoById(DisplayId displayId)
{
    TLOGD(WmsLogTag::DMS, "enter, displayId: %{public}" PRIu64" ", displayId);
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGI(WmsLogTag::DMS, "screenSession is nullptr, ScreenId: %{public}" PRIu64 "",
                sessionIt.first);
            continue;
        }
        sptr<DisplayInfo> displayInfo = screenSession->ConvertToRealDisplayInfo();
        if (displayInfo == nullptr) {
            TLOGI(WmsLogTag::DMS, "ConvertToDisplayInfo error, displayInfo is nullptr.");
            continue;
        }
        if (displayId != displayInfo->GetDisplayId()) {
            continue;
        }
        TLOGD(WmsLogTag::DMS, "success");
        HandleRotationCorrectionExemption(displayInfo);
        displayInfo = HookDisplayInfoByUid(displayInfo, screenSession);
        if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
            return displayInfo;
        }
#ifdef FOLD_ABILITY_ENABLE
        HandleSuperFoldDisplayInfoWhenKeyboardOn(screenSession, displayInfo);
        return displayInfo;
#endif
    }
    TLOGE(WmsLogTag::DMS, "GetVisibleAreaDisplayInfoById failed. displayId: %{public}" PRIu64" ", displayId);
    return nullptr;
}

void ScreenSessionManager::HandleSuperFoldDisplayInfoWhenKeyboardOn(
    const sptr<ScreenSession>& screenSession, sptr<DisplayInfo>& displayInfo)
{
#ifdef FOLD_ABILITY_ENABLE
    if (screenSession == nullptr) {
        TLOGI(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    if (displayInfo == nullptr) {
        TLOGI(WmsLogTag::DMS, "displayInfo is nullptr");
        return;
    }
    SuperFoldStatus status = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    bool isSystemKeyboardOn = SuperFoldStateManager::GetInstance().GetSystemKeyboardStatus();
    RRect bounds = screenSession->GetScreenProperty().GetBounds();
    uint32_t screenWidth = bounds.rect_.GetWidth();
    uint32_t screenHeight = bounds.rect_.GetHeight();
    // Adjust screen height when physical keyboard attach or touchPad virtual Keyboard on
    if (status == SuperFoldStatus::KEYBOARD || isSystemKeyboardOn) {
        if (screenWidth > screenHeight) {
            std::swap(screenWidth, screenHeight);
        }
        DMRect creaseRect = screenSession->GetScreenProperty().GetCreaseRect();
        if (creaseRect.posY_ > 0) {
            displayInfo->SetHeight(creaseRect.posY_);
        } else {
            displayInfo->SetHeight(screenHeight / HALF_SCREEN_PARAM);
        }
        displayInfo->SetWidth(screenWidth);
    } else {
        displayInfo->SetWidth(screenWidth);
        displayInfo->SetHeight(screenHeight);
    }
#endif
}

sptr<DisplayInfo> ScreenSessionManager::GetDisplayInfoByScreen(ScreenId screenId)
{
    TLOGD(WmsLogTag::DMS, "enter, screenId: %{public}" PRIu64"", screenId);
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGI(WmsLogTag::DMS, "screenSession is nullptr, ScreenId:%{public}" PRIu64"",
                sessionIt.first);
            continue;
        }
        sptr<DisplayInfo> displayInfo = screenSession->ConvertToDisplayInfo();
        if (displayInfo == nullptr) {
            TLOGI(WmsLogTag::DMS, "error, displayInfo is nullptr.");
            continue;
        }
        if (screenId == displayInfo->GetScreenId()) {
            return displayInfo;
        }
    }
    TLOGE(WmsLogTag::DMS, "failed. screenId: %{public}" PRIu64" ", screenId);
    return nullptr;
}

DisplayId ScreenSessionManager::GetFakeDisplayId(sptr<ScreenSession> screenSession)
{
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    if (fakeScreenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "error, fakeScreenSession is nullptr.");
        return DISPLAY_ID_INVALID;
    }
    sptr<DisplayInfo> fakeDisplayInfo = fakeScreenSession->ConvertToDisplayInfo();
    if (fakeDisplayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "error, fakeDisplayInfo is nullptr.");
        return DISPLAY_ID_INVALID;
    }
    DisplayId fakeDisplayId = fakeDisplayInfo->GetDisplayId();
    return fakeDisplayId;
}

std::vector<DisplayId> ScreenSessionManager::GetAllDisplayIds(int32_t userId)
{
    TLOGD(WmsLogTag::DMS, "enter");
    ScreenId filterScreenId = INVALID_SCREEN_ID;
    if (!IsConcurrentUser()) {
        userId = USER_ID_DEFAULT;
    }
    if (IsConcurrentUser() && userId == CONCURRENT_USER_ID_DEFAULT) {
        userId = GetUserIdByCallingUid();
        filterScreenId = GetUserDisplayId(userId);
        TLOGI(WmsLogTag::DMS, "concurrentuser open, userId: %{public}u, filterScreenId: %{public}" PRIu64"",
              userId, filterScreenId);
    }
    std::vector<DisplayId> res;
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (const auto& [screenId, screenSession] : screenSessionMap_) {
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr, ScreenId:%{public}" PRIu64"", screenId);
            continue;
        }
        const ScreenProperty& prop = screenSession->GetScreenProperty();
        
        // collect all virtual screens, regardless of filter
        if (prop.GetScreenType() == ScreenType::VIRTUAL) {
            DisplayId displayId = screenSession->GetDisplayId();
            if (displayId != DISPLAY_ID_INVALID) {
                res.push_back(displayId);
            }
            continue;
        }
        if (filterScreenId != INVALID_SCREEN_ID && screenId != filterScreenId) {
            continue;
        }
        DisplayId displayId = screenSession->GetDisplayId();
        if (displayId == DISPLAY_ID_INVALID) {
            continue;
        }
        res.push_back(displayId);
        if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && prop.GetIsFakeInUse()) {
            DisplayId fakeDisplayId = GetFakeDisplayId(screenSession);
            if (fakeDisplayId == DISPLAY_ID_FAKE) {
                res.push_back(fakeDisplayId);
                TLOGI(WmsLogTag::DMS, "add fakeDisplayId: %{public}" PRIu64 "", fakeDisplayId);
            }
        }
    }
    return res;
}

void ScreenSessionManager::CalculateXYPosition(sptr<ScreenSession> firstScreenSession,
    sptr<ScreenSession> secondaryScreenSession)
{
    if (firstScreenSession != nullptr &&
        firstScreenSession->GetScreenCombination() == ScreenCombination::SCREEN_MAIN) {
        firstScreenSession->SetXYPosition(0, 0);
        SetPhysicalXYPosition(firstScreenSession, 0, 0);
        NotifyDisplayChanged(firstScreenSession->ConvertToDisplayInfo(), DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
        CalculateSecondryXYPosition(firstScreenSession, secondaryScreenSession);
    } else if (secondaryScreenSession != nullptr &&
        secondaryScreenSession->GetScreenCombination() == ScreenCombination::SCREEN_MAIN) {
        secondaryScreenSession->SetXYPosition(0, 0);
        SetPhysicalXYPosition(secondaryScreenSession, 0, 0);
        NotifyDisplayChanged(secondaryScreenSession->ConvertToDisplayInfo(), DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
        CalculateSecondryXYPosition(secondaryScreenSession, firstScreenSession);
    } else {
        TLOGE(WmsLogTag::DMS, "CalculateXYPosition error!");
    }
}

void ScreenSessionManager::CalculateSecondryXYPosition(sptr<ScreenSession> firstScreenSession,
    sptr<ScreenSession> secondaryScreenSession)
{
    if (firstScreenSession == nullptr || secondaryScreenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    ScreenProperty firstScreenProperty = firstScreenSession->GetScreenProperty();
    ScreenProperty secondaryScreenProperty = secondaryScreenSession->GetScreenProperty();
    int32_t firstStartX = static_cast<int32_t>(firstScreenProperty.GetStartX());
    int32_t firstStartY = static_cast<int32_t>(firstScreenProperty.GetStartY());
    int32_t secondaryStartX = static_cast<int32_t>(secondaryScreenProperty.GetStartX());
    int32_t secondaryStartY = static_cast<int32_t>(secondaryScreenProperty.GetStartY());
    int32_t secondaryX = -firstStartX + secondaryStartX;
    int32_t secondaryY = -firstStartY + secondaryStartY;
    secondaryScreenSession->SetXYPosition(secondaryX, secondaryY);
    SetPhysicalXYPosition(secondaryScreenSession, secondaryX, secondaryY);
    NotifyDisplayChanged(secondaryScreenSession->ConvertToDisplayInfo(), DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
}

void ScreenSessionManager::SetPhysicalXYPosition(sptr<ScreenSession> screenSession, int32_t x, int32_t y)
{
    if(screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    sptr<ScreenSession> physicalScreen = GetPhysicalScreenSession(screenSession->GetRSScreenId());
    if(physicalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "physicalScreen is nullptr");
        return;
    }
    physicalScreen->SetXYPosition(x,y);
}

void ScreenSessionManager::SetPhysicalStartPosition(sptr<ScreenSession> screenSession, uint32_t startX, uint32_t startY)
{
    if(screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    sptr<ScreenSession> physicalScreen = GetPhysicalScreenSession(screenSession->GetRSScreenId());
    if(physicalScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "physicalScreen is nullptr");
        return;
    }
    physicalScreen->SetStartPosition(startX, startY);
}

DMError ScreenSessionManager::CheckVirtualScreenPermission()
{
    bool isCallingByThirdParty = Permission::CheckCallingPermission(ACCESS_VIRTUAL_SCREEN_PERMISSION);
    if (isCallingByThirdParty) {
        return DMError::DM_OK;
    }
    if (SessionPermission::IsShellCall()) {
        return DMError::DM_OK;
    }
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied, sys or shell");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (!Permission::CheckCallingPermission(SCREEN_CAPTURE_PERMISSION)) {
        TLOGE(WmsLogTag::DMS, "Permission Denied, capture");
        return DMError::DM_ERROR_INVALID_PERMISSION;
    }
    return DMError::DM_OK;
}

sptr<ScreenInfo> ScreenSessionManager::GetScreenInfoById(ScreenId screenId)
{
    if (!SessionPermission::IsSystemCalling() &&
        !Permission::CheckCallingPermission(ACCESS_VIRTUAL_SCREEN_PERMISSION)) {
        TLOGE(WmsLogTag::DMS, "Permission Denied.calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return nullptr;
    }
    auto screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "cannot find screenInfo: %{public}" PRIu64"", screenId);
        return nullptr;
    }
    return screenSession->ConvertToScreenInfo();
}

void ScreenSessionManager::ReportScreenModeChangeEvent(RSScreenModeInfo screenmode, uint32_t result)
{
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "SCREEN_MODE_CHANGE",
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "SCREEN_WIDTH", static_cast<uint16_t>(screenmode.GetScreenWidth()),
        "SCREEN_HEIGHT", static_cast<uint16_t>(screenmode.GetScreenHeight()),
        "SCREEN_REFRASH_RATE", static_cast<uint16_t>(screenmode.GetScreenRefreshRate()),
        "ERROR_CODE", static_cast<uint32_t>(result));
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "Write HiSysEvent error, ret: %{public}d", ret);
    }
}

void ScreenSessionManager:: ReportRelativePositionChangeEvent(MultiScreenPositionOptions& mainScreenOptions,
        MultiScreenPositionOptions& secondScreenOption, const std::string& errMsg)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "SCREEN_RELATIVE_POSITION_CHANGE",
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "INNER_SCREEN_CODE", mainScreenOptions.screenId_,
        "INNER_SCREEN_STARTX", static_cast<uint16_t>(mainScreenOptions.startX_),
        "INNER_SCREEN_STARTY", static_cast<uint16_t>(mainScreenOptions.startY_),
        "EXTEND_SCREEN_CODE", secondScreenOption.screenId_,
        "EXTEND_SCREEN_STARTX", static_cast<uint16_t>(secondScreenOption.startX_),
        "EXTEND_SCREEN_STARTY", static_cast<uint16_t>(secondScreenOption.startY_),
        "ERROR_MSG",errMsg);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "Write HiSysEvent error, ret: %{public}d", ret);
    }
#endif
}

void ScreenSessionManager::CheckAndNotifyRefreshRate(uint32_t refreshRate, sptr<ScreenSession> updateScreenSession)
{
    if (updateScreenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    bool isRefreshRateChanged = refreshRate != updateScreenSession->GetRefreshRate();
    if (isRefreshRateChanged) {
        NotifyDisplayChanged(updateScreenSession->ConvertToDisplayInfo(), DisplayChangeEvent::UPDATE_REFRESHRATE);
        TLOGI(WmsLogTag::DMS, "notify end");
    } else {
        TLOGI(WmsLogTag::DMS, "no notify");
    }
}

void ScreenSessionManager::CheckAndNotifyChangeMode(const RRect& bounds, sptr<ScreenSession> updateScreenSession)
{
    if (updateScreenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    ScreenProperty property = updateScreenSession->GetScreenProperty();
    property.SetPropertyChangeReason(ScreenPropertyChangeReason::ACTIVE_MODE_CHANGE);
    updateScreenSession->PropertyChange(property, ScreenPropertyChangeReason::CHANGE_MODE);
    RRect updateBounds = property.GetBounds();
    bool isSizeChanged =
        std::fabs(bounds.rect_.width_ - updateBounds.rect_.width_) >= FLT_EPSILON ||
        std::fabs(bounds.rect_.height_ - updateBounds.rect_.height_) >= FLT_EPSILON;
    if (isSizeChanged) {
        NotifyDisplayChanged(updateScreenSession->ConvertToDisplayInfo(), DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
        TLOGI(WmsLogTag::DMS, "notify end");
    } else {
        TLOGI(WmsLogTag::DMS, "no notify");
    }
    NotifyScreenChanged(updateScreenSession->ConvertToScreenInfo(), ScreenChangeEvent::CHANGE_MODE);
    NotifyScreenModeChange();
}


DMError ScreenSessionManager::SetScreenActiveMode(ScreenId screenId, uint32_t modeId)
{
#ifdef WM_SCREEN_ACTIVE_MODE_ENABLE
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64", modeId: %{public}u", screenId, modeId);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied!  calling: %{public}s, calling pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "invalid screenId");
        return DMError::DM_ERROR_NULLPTR;
    }
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId");
        return DMError::DM_ERROR_NULLPTR;
    }
    sptr<ScreenSession> screenSession = GetScreenSessionByRsId(rsScreenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    RRect screenBounds = screenSession->GetScreenProperty().GetBounds();
    uint32_t refreshRate = screenSession->GetRefreshRate();
    RSScreenModeInfo screenMode = RSScreenModeInfo();
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetScreenActiveMode(%" PRIu64", %u)", screenId, modeId);
    auto ret = rsInterface_.SetScreenActiveMode(rsScreenId, modeId); // modeId is activeIdx;
    const char* logPreFix = (ret = StatusCode::SUCCESS) ? "RS success" : "RS fail";
    TLOGW(WmsLogTag::DMS, "%{public}s, rsScreenId: %{public}" PRIu64",modeId:%{public}u, ret%{public}d",
        logPreFix, rsScreenId, modeId, ret);
    if (ret != StatusCode::SUCCESS) {
        ReportScreenModeChangeEvent(screenMode, ret);
        if (ret == StatusCode::HDI_ERR_NOT_SUPPORT) {
            return DMError::DM_ERROR_INVALID_MODE_ID;
        } else {
            return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
        }
    }
    int setScreenActiveId = HiviewDFX::XCollie::GetInstance().SetTimer("SetScreenActiveModeCallRS",
        XCOLLIE_TIMEOUT_10S, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
    TLOGW(WmsLogTag::DMS, "Call rsInterface_ GetScreenActiveMode rsid: %{public}" PRIu64, rsScreenId);
    screenMode = rsInterface_.GetScreenActiveMode(rsScreenId);
    HiviewDFX::XCollie::GetInstance().CancelTimer(setScreenActiveId);
    ReportScreenModeChangeEvent(screenMode, ret);
    int32_t activeId = screenMode.GetScreenModeId();
    UpdateSessionByActiveModeChange(screenSession, activeId);
    CheckAndNotifyChangeMode(screenBounds, screenSession);
    CheckAndNotifyRefreshRate(refreshRate, screenSession);
#endif
    return DMError::DM_OK;
}

void ScreenSessionManager::UpdateSessionByActiveModeChange(sptr<ScreenSession> screenSession, int32_t activeIdx)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    if (!g_isPcDevice) {
        screenSession->activeIdx_ = activeIdx;
        screenSession->UpdatePropertyByActiveMode();
    } else {
        sptr<ScreenSession>  phyScreenSession = GetPhysicalScreenSession(screenSession->GetRSScreenId());
        if (phyScreenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "phyScreenSession is null");
            return;
    }
        ScreenProperty property = screenSession->GetScreenProperty();
        uint32_t startXcopy = property.GetStartX();
        uint32_t startYcopy = property.GetStartY();
        int32_t retxCopy = property.GetX();
        int32_t retyCopy = property.GetY();
        phyScreenSession->activeIdx_ = activeIdx;
        phyScreenSession->UpdatePropertyByActiveModeChange();
        InitExtendScreenProperty(phyScreenSession->GetScreenId(), phyScreenSession,
            phyScreenSession->GetScreenProperty());
        ScreenProperty PhyProperty = phyScreenSession->GetScreenProperty();
        screenSession->SetScreenProperty(PhyProperty);
        screenSession->activeIdx_ = activeIdx;
        screenSession->SetStartPosition(startXcopy, startYcopy);
        screenSession->SetXYPosition(retxCopy, retyCopy);
        screenSession->SetScreenOffScreenRendering();
        if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
            SuperFoldStateManager::GetInstance().RefreshExternalRegion();
        }
        HandleResolutionEffectChange();
    }
    TLOGI(WmsLogTag::DMS, "end");
}

bool ScreenSessionManager::ConvertScreenIdToRsScreenId(ScreenId screenId, ScreenId& rsScreenId)
{
    return screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId);
}

void ScreenSessionManager::UpdateDisplayHookInfo(int32_t uid, bool enable, const DMHookInfo& hookInfo)
{
    TLOGD(WmsLogTag::DMS, "DisplayHookInfo will update");
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return;
    }
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMapCopy;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        screenSessionMapCopy = screenSessionMap_;
    }
    std::unique_lock<std::shared_mutex> lock(hookInfoMutex_);
    if (enable) {
        if (uid != 0) {
            displayHookMap_[uid] = hookInfo;
        }
    } else {
        displayHookMap_.erase(uid);
    }
    NotifyDisplayChangedByUid(screenSessionMapCopy, DisplayChangeEvent::DISPLAY_SIZE_CHANGED, uid);
}

void ScreenSessionManager::NotifyDisplayChangedByUid(const std::map<ScreenId, sptr<ScreenSession>>& screenSessionMap,
    DisplayChangeEvent event, int32_t uid)
{
    for (const auto& sessionIt : screenSessionMap) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
            continue;
        }
        NotifyDisplayChangedByUidInner(screenSession->ConvertToDisplayInfo(), event, uid);
    }
}

void ScreenSessionManager::NotifyDisplayChangedByUidInner(sptr<DisplayInfo> displayInfo,
    DisplayChangeEvent event, int32_t uid)
{
    auto task = [=] {
        ScreenSessionManagerAdapter::GetInstance().OnDisplayChange(displayInfo, event, uid);
    };
    taskScheduler_->PostAsyncTask(task, "NotifyDisplayChanged");
    TLOGI(WmsLogTag::DMS, "notify end");
}

void ScreenSessionManager::GetDisplayHookInfo(int32_t uid, DMHookInfo& hookInfo)
{
    std::shared_lock<std::shared_mutex> lock(hookInfoMutex_);
    if (displayHookMap_.find(uid) != displayHookMap_.end()) {
        hookInfo = displayHookMap_[uid];
    }
}

void ScreenSessionManager::NotifyIsFullScreenInForceSplitMode(int32_t uid, bool isFullScreen)
{
    TLOGI(WmsLogTag::DMS, "uid: %{public}d isFullScreen: %{public}d", uid, isFullScreen);
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return;
    }
    {
        std::unique_lock<std::shared_mutex> lock(hookInfoMutex_);
        if (uid == 0 || displayHookMap_.find(uid) == displayHookMap_.end()) {
            TLOGE(WmsLogTag::DMS, "invalid uid: %{public}d", uid);
            return;
        }
        displayHookMap_[uid].isFullScreenInForceSplit_ = isFullScreen;
    }
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMapCopy;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        screenSessionMapCopy = screenSessionMap_;
    }
    NotifyDisplayChangedByUid(screenSessionMapCopy, DisplayChangeEvent::DISPLAY_SIZE_CHANGED, uid);
}

bool ScreenSessionManager::IsFreezed(const int32_t& agentPid, const DisplayManagerAgentType& agentType)
{
    std::lock_guard<std::mutex> lock(freezedPidListMutex_);
    if (freezedPidList_.count(agentPid) == 0) {
        return false;
    }
    // 冻结的应用记录应用 pid 和注册的 agentType
    if (pidAgentTypeMap_.count(agentPid) == 0) {
        std::set agentTypes = { agentType };
        pidAgentTypeMap_[agentPid] = agentTypes;
    } else {
        pidAgentTypeMap_[agentPid].insert(agentType);
    }
    TLOGD(WmsLogTag::DMS, "Agent is freezed, no need notify. PID: %{public}d.", agentPid);
    return true;
}

const std::map<DisplayId, std::map<int32_t, ScreenSessionManager::UserInfo>>
    ScreenSessionManager::GetDisplayConcurrentUserMap() const
{
    std::lock_guard<std::mutex> lock(displayConcurrentUserMapMutex_);
    return displayConcurrentUserMap_;
}

int32_t ScreenSessionManager::GetForegroundConcurrentUser(DisplayId displayId) const
{
    std::lock_guard<std::mutex> lock(displayConcurrentUserMapMutex_);
    auto screenIt = displayConcurrentUserMap_.find(displayId);
    int32_t foregroundUserId = INVALID_USER_ID;
    if (screenIt == displayConcurrentUserMap_.end()) {
        TLOGI(WmsLogTag::DMS, "Can't find display in playConcurrentUsersMap,"
            "invalid displayId: %{public}" PRIu64, displayId);
        return foregroundUserId;
    }

    const auto& userMap = screenIt->second;
    for (const auto& it : userMap) {
        if (it.second.isForeground) {
            foregroundUserId = it.first;
            TLOGI(WmsLogTag::DMS, "Get foreground userId: %{public}d on displayId: %{public}" PRIu64,
                foregroundUserId, displayId);
            break;
        }
    }
    return foregroundUserId;
}

void ScreenSessionManager::SetDisplayConcurrentUserMap(DisplayId displayId, int32_t userId,
                                                       bool isForeground, int32_t pid)
{
    std::lock_guard<std::mutex> lock(displayConcurrentUserMapMutex_);
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGI(WmsLogTag::DMS, "Invalid displayId: %{public}" PRIu64, displayId);
        return;
    }
    displayConcurrentUserMap_[displayId][userId] = { isForeground, pid };
}

void ScreenSessionManager::RemoveUserByPid(int32_t deathPid)
{
    std::lock_guard<std::mutex> lock(displayConcurrentUserMapMutex_);
    for (auto displayIt = displayConcurrentUserMap_.begin(); displayIt != displayConcurrentUserMap_.end();) {
        auto& userMap = displayIt->second;
        for (auto it = userMap.begin(); it != userMap.end();) {
            if (it->second.pid == deathPid) {
                TLOGI(WmsLogTag::DMS, "Remove userId: %{public}d by pid: %{public}d", it->first, it->second.pid);
                it = userMap.erase(it);
                break;
            } else {
                it++;
            }
        }
        displayIt++;
    }
}

bool ScreenSessionManager::CheckPidInDeathPidVector(int32_t pid) const
{
    auto iter = std::find(deathPidVector_.begin(), deathPidVector_.end(), pid);
    if (iter != deathPidVector_.end()) {
        TLOGI(WmsLogTag::DMS, "Pid: %{public}d already been killed", pid);
        return true;
    } else {
        return false;
    }
}

void ScreenSessionManager::NotifyScreenChanged(sptr<ScreenInfo> screenInfo, ScreenChangeEvent event)
{
    if (screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "error, screenInfo is nullptr.");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(lastStatusUpdateMutex_);
        lastScreenChangeEvent_ = event;
    }
    auto task = [=] {
      ScreenSessionManagerAdapter::GetInstance().OnScreenChange(screenInfo, event);
    };
    taskScheduler_->PostAsyncTask(task, "NotifyScreenChanged:SID:" + std::to_string(screenInfo->GetScreenId()));
}

DMError ScreenSessionManager::SetVirtualPixelRatio(ScreenId screenId, float virtualPixelRatio)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screen session is nullptr");
        return DMError::DM_ERROR_UNKNOWN;
    }
    if (screenSession->isScreenGroup_) {
        TLOGE(WmsLogTag::DMS, "cannot set virtual pixel ratio to the combination. screen: %{public}" PRIu64"",
            screenId);
        return DMError::DM_ERROR_NULLPTR;
    }
    virtualPixelRatio *= screenSession->GetVprScaleRatio();
    // less to 1e-6 mean equal
    if (fabs(screenSession->GetScreenProperty().GetVirtualPixelRatio() - virtualPixelRatio) < 1e-6) {
        TLOGE(WmsLogTag::DMS,
            "The density is equivalent to the original value, no update operation is required, aborted.");
        return DMError::DM_OK;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetVirtualPixelRatio(%" PRIu64", %f)", screenId,
        virtualPixelRatio);
    screenSession->SetVirtualPixelRatio(virtualPixelRatio);
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
        if (fakeScreenSession != nullptr) {
            fakeScreenSession->SetVirtualPixelRatio(virtualPixelRatio);
            NotifyDisplayChanged(fakeScreenSession->ConvertToDisplayInfo(),
                DisplayChangeEvent::DISPLAY_VIRTUAL_PIXEL_RATIO_CHANGED);
        }
    }
    std::map<DisplayId, sptr<DisplayInfo>> emptyMap;
    OnPropertyChange(screenSession->GetScreenProperty(), ScreenPropertyChangeReason::VIRTUAL_PIXEL_RATIO_CHANGE,
        screenId);
    NotifyDisplayStateChange(screenId, screenSession->ConvertToDisplayInfo(),
        emptyMap, DisplayStateChangeType::VIRTUAL_PIXEL_RATIO_CHANGE);
    NotifyScreenChanged(screenSession->ConvertToScreenInfo(), ScreenChangeEvent::VIRTUAL_PIXEL_RATIO_CHANGED);
    NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(),
        DisplayChangeEvent::DISPLAY_VIRTUAL_PIXEL_RATIO_CHANGED);
    if (g_isPcDevice) {
        sptr<ScreenSession> physicalScreen = GetPhysicalScreenSession(screenSession->GetRSScreenId());
        if (physicalScreen) {
            physicalScreen->SetVirtualPixelRatio(virtualPixelRatio);
        }
    }
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    auto clientProxy = GetClientProxy();
    if (clientProxy) {
        clientProxy->SetVirtualPixelRatioSystem(screenId, virtualPixelRatio);
    }
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetDefaultDensityDpi(ScreenId screenId, float virtualPixelRatio)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screen session is nullptr");
        return DMError::DM_ERROR_UNKNOWN;
    }
    if (screenSession->isScreenGroup_) {
        TLOGE(WmsLogTag::DMS, "cannot set virtual pixel ratio to the combination. screen: %{public}" PRIu64"",
            screenId);
        return DMError::DM_ERROR_NULLPTR;
    }
    // less to 1e-6 mean equal
    if (fabs(screenSession->GetScreenProperty().GetDefaultDensity() - virtualPixelRatio) < 1e-6) {
        TLOGE(WmsLogTag::DMS,
            "The density is equivalent to the original value, no update operation is required, aborted.");
        return DMError::DM_OK;
    }
    screenSession->SetDefaultDensity(virtualPixelRatio);
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetResolution(ScreenId screenId, uint32_t width, uint32_t height, float virtualPixelRatio)
{
    TLOGI(WmsLogTag::DMS,
        "ScreenId: %{public}" PRIu64 ", w: %{public}u, h: %{public}u, virtualPixelRatio: %{public}f",
        screenId, width, height, virtualPixelRatio);
    DMError ret = CheckSetResolutionIsValid(screenId, width, height, virtualPixelRatio);
    if (ret != DMError::DM_OK) {
        return ret;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "Get ScreenSession failed");
        return DMError::DM_ERROR_NULLPTR;
    }
    screenSession->FreezeScreen(true);
    if (rsInterface_.SetRogScreenResolution(screenId, width, height) != 0) {
        TLOGE(WmsLogTag::DMS, "Failed to SetRogScreenResolution");
        screenSession->FreezeScreen(false);
        rsInterface_.ForceRefreshOneFrameWithNextVSync();
        return DMError::DM_ERROR_IPC_FAILED;
    }
    uint32_t defaultResolutionDpi = virtualPixelRatio * BASELINE_DENSITY;
    (void)ScreenSettingHelper::SetSettingDefaultDpi(defaultResolutionDpi, SET_SETTING_DPI_KEY);
    float vprScaleRatio = virtualPixelRatio / densityDpi_;
    screenSession->SetVprScaleRatio(vprScaleRatio);
    screenSession->SetDensityInCurResolution(virtualPixelRatio);
    screenSession->SetDefaultDensity(virtualPixelRatio);
    screenSession->SetVirtualPixelRatio(virtualPixelRatio);

    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetResolution(%" PRIu64", %u, %u, %f)",
        screenId, width, height, virtualPixelRatio);
    screenSession->UpdatePropertyByResolution(width, height);
    std::map<DisplayId, sptr<DisplayInfo>> emptyMap;
    auto displayInfo = screenSession->ConvertToDisplayInfo();
    auto screenInfo = screenSession->ConvertToScreenInfo();
    NotifyDisplayChanged(displayInfo, DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    NotifyScreenChanged(screenInfo, ScreenChangeEvent::CHANGE_MODE);
    NotifyDisplayStateChange(screenId, displayInfo, emptyMap, DisplayStateChangeType::RESOLUTION_CHANGE);
    screenSession->PropertyChange(screenSession->GetScreenProperty(), ScreenPropertyChangeReason::CHANGE_MODE);
    WatchParameter(BOOTEVENT_BOOT_COMPLETED.c_str(), BootFinishedUnfreezeCallback, this);
    AddScreenUnfreezeTask(screenSession, 0);
    return DMError::DM_OK;
}

static inline bool IsVertical(Rotation rotation)
{
    return rotation == Rotation::ROTATION_0 || rotation == Rotation::ROTATION_180;
}

void ScreenSessionManager::HandleResolutionEffectChangeWhenRotate()
{
    TLOGI(WmsLogTag::DMS, "start");
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        TLOGE(WmsLogTag::DMS, "not support");
        return;
    }
#ifdef FOLD_ABILITY_ENABLE
    sptr<ScreenSession> internalSession = GetInternalScreenSession();
    if (internalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "Internal Session null");
        return;
    }
    if (!IsVertical(internalSession->GetRotation())) {
        HandleResolutionEffectChange();
    }
#endif
}

bool ScreenSessionManager::HandleResolutionEffectChange()
{
    if (!g_isPcDevice) {
        TLOGE(WmsLogTag::DMS, "not support");
        return false;
    }
    TLOGI(WmsLogTag::DMS, "start");
    sptr<ScreenSession> internalSession = nullptr;
    sptr<ScreenSession> externalSession = nullptr;
    GetInternalAndExternalSession(internalSession, externalSession);
    if (internalSession == nullptr || externalSession == nullptr ||
        externalSession->GetScreenCombination() != ScreenCombination::SCREEN_MIRROR) {
        TLOGE(WmsLogTag::DMS, "Session null or not mirror");
        RecoveryResolutionEffect();
        return false;
    }
    bool effectFlag = false;
    ScreenSettingHelper::GetResolutionEffect(effectFlag, externalSession->GetSerialNumber());
    uint32_t targetWidth = 0;
    uint32_t targetHeight = 0;
    CalculateTargetResolution(internalSession, externalSession, effectFlag, targetWidth, targetHeight);
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && (IsVertical(internalSession->GetRotation()) ||
        GetSuperFoldStatus() != SuperFoldStatus::EXPANDED)){
        TLOGI(WmsLogTag::DMS, "SuperFoldDisplayDevice status not support");
        return false;
    }
    if (targetWidth != 0 && targetHeight != 0) {
        SetResolutionEffect(internalSession->GetScreenId(), targetWidth, targetHeight);
    }
    return true;
}

void ScreenSessionManager::CalculateTargetResolution(const sptr<ScreenSession>& internalSession,
    const sptr<ScreenSession>& externalSession, const bool& effectFlag,
    uint32_t& targetWidth, uint32_t& targetHeight)
{
    uint32_t innerWidth = internalSession->GetScreenProperty().GetScreenRealWidth();
    uint32_t innerHeight = internalSession->GetScreenProperty().GetScreenRealHeight();
    uint32_t externalWidth = externalSession->GetScreenProperty().GetScreenRealWidth();
    uint32_t externalHeight = externalSession->GetScreenProperty().GetScreenRealHeight();
    if (IsVertical(internalSession->GetRotation()) != IsVertical(externalSession->GetRotation())) {
        std::swap(externalWidth, externalHeight);
    }
    targetWidth = innerWidth;
    targetHeight = innerHeight;
    if (innerHeight == 0 || externalHeight == 0) {
        return;
    }
    float innerResolution = static_cast<float>(innerWidth) / innerHeight;
    float externalResolution = static_cast<float>(externalWidth) / externalHeight;
    float diffResolution = innerResolution - externalResolution;
    TLOGI(WmsLogTag::DMS, "innerResolution:%{public}f, externalResolution %{public}f",
        innerResolution, externalResolution);
    if (externalResolution == 0) {
        return;
    }
    if (effectFlag && std::fabs(diffResolution) >= FLT_EPSILON) {
        if (externalResolution > innerResolution ) {
            targetHeight = static_cast<uint32_t>(innerWidth / externalResolution);
        } else {
            targetWidth = static_cast<uint32_t>(innerHeight * externalResolution);
        }
        curResolutionEffectEnable_.store(true);
    } else {
        curResolutionEffectEnable_.store(false);
    }
}

bool ScreenSessionManager::SetResolutionEffect(ScreenId screenId,  uint32_t width, uint32_t height)
{
    if (!g_isPcDevice) {
        TLOGW(WmsLogTag::DMS, "not support");
        return false;
    }
    TLOGI(WmsLogTag::DMS, "change resolution according to extend screen when in mirror screen combination: "
        "%{public}" PRIu64 " %{public}d %{public}d", screenId, width, height);
    sptr<ScreenSession> internalSession = nullptr;
    sptr<ScreenSession> externalSession = nullptr;
    GetInternalAndExternalSession(internalSession, externalSession);
    if (externalSession == nullptr ||
        externalSession->GetScreenCombination() != ScreenCombination::SCREEN_MIRROR) {
        return false;
    }
    if (internalSession == nullptr || internalSession->GetScreenId() != screenId) {
        TLOGE(WmsLogTag::DMS, "only internal session valid");
        return false;
    }
    uint32_t realWidth = internalSession->GetScreenProperty().GetScreenRealWidth();
    uint32_t realHeight = internalSession->GetScreenProperty().GetScreenRealHeight();
    // Calculate the center position
    DMRect toRect = {
        std::floor((realWidth - width) / 2),
        std::floor((realHeight - height) / 2),
        width,
        height
    };
    TLOGI(WmsLogTag::DMS, "toRect %{public}d %{public}d %{public}d %{public}d",
        toRect.posX_, toRect.posY_, toRect.width_, toRect.height_);
    SetInternalScreenResolutionEffect(internalSession, toRect);
    SetExternalScreenResolutionEffect(externalSession, toRect);
    HandleCastVirtualScreenMirrorRegion();
    NotifyScreenModeChange();
    return true;
}

bool ScreenSessionManager::RecoveryResolutionEffect()
{
    if (!g_isPcDevice) {
        TLOGW(WmsLogTag::DMS, "not support");
        return false;
    }
    TLOGI(WmsLogTag::DMS, "recovery inner and external screen resolution");
    sptr<ScreenSession> internalSession = nullptr;
    sptr<ScreenSession> externalSession = nullptr;
    GetInternalAndExternalSession(internalSession, externalSession);
    if (internalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "internalSession null");
        return false;
    }
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && (IsVertical(internalSession->GetRotation()) ||
        GetSuperFoldStatus() != SuperFoldStatus::EXPANDED)){
        TLOGI(WmsLogTag::DMS, "SuperFoldDisplayDevice status not support");
        return false;
    }
    auto internalProperty = internalSession->GetScreenProperty();
    DMRect realResolutionRect = { 0, 0, internalProperty.GetScreenRealWidth(),
        internalProperty.GetScreenRealHeight()};
    TLOGI(WmsLogTag::DMS, "realResolutionRect %{public}d %{public}d %{public}d %{public}d",
        realResolutionRect.posX_, realResolutionRect.posY_, realResolutionRect.width_, realResolutionRect.height_);
    SetInternalScreenResolutionEffect(internalSession, realResolutionRect);
    curResolutionEffectEnable_.store(false);
    if (externalSession != nullptr) {
        auto externalProperty = externalSession->GetScreenProperty();
        DMRect externalRealRect = { 0, 0, externalProperty.GetScreenRealWidth(),
            externalProperty.GetScreenRealHeight()};
        SetExternalScreenResolutionEffect(externalSession, externalRealRect);
    }
    HandleCastVirtualScreenMirrorRegion();
    NotifyScreenModeChange();
    return true;
}

void ScreenSessionManager::SetInternalScreenResolutionEffect(const sptr<ScreenSession>& internalSession, DMRect& targetRect)
{
    if (internalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "internalSession null");
        return;
    }
    // Setting the display area of the internal screen
    auto oldProperty = internalSession->GetScreenProperty();
    internalSession->UpdatePropertyByResolution(targetRect);
    auto newProperty = internalSession->GetScreenProperty();
    newProperty.SetPropertyChangeReason(ScreenPropertyChangeReason::RESOLUTION_EFFECT_CHANGE);
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        newProperty.SetSuperFoldStatusChangeEvent(SuperFoldStatusChangeEvents::RESOLUTION_EFFECT_CHANGE);
    }
    internalSession->PropertyChange(newProperty, ScreenPropertyChangeReason::CHANGE_MODE);
    if (oldProperty.GetScreenAreaWidth() != targetRect.width_ ||
        oldProperty.GetScreenAreaHeight() != targetRect.height_) {
        NotifyScreenChanged(internalSession->ConvertToScreenInfo(), ScreenChangeEvent::CHANGE_MODE);
        NotifyDisplayChanged(internalSession->ConvertToDisplayInfo(), DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    }
    // Black out invalid area
    auto clientProxy = GetClientProxy();
    if (clientProxy == nullptr) {
        TLOGE(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy->SetInternalClipToBounds(internalSession->GetScreenId(), true);
}

void ScreenSessionManager::SetExternalScreenResolutionEffect(const sptr<ScreenSession>& externalSession, DMRect& targetRect)
{
    if (externalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "externalSession null");
        return;
    }
    //all zero, means full screen mirror.
    DMRect mirrorRegion = targetRect;
    if (!curResolutionEffectEnable_.load()) {
        mirrorRegion = {0, 0, 0, 0};
    }
    externalSession->SetMirrorScreenRegion(externalSession->GetScreenId(), mirrorRegion);
    externalSession->EnableMirrorScreenRegion();
    // Parameters required for multiinput cursor
    externalSession->UpdateMirrorWidth(targetRect.width_);
    externalSession->UpdateMirrorHeight(targetRect.height_);
    sptr<ScreenSession> phyScreenSession = GetPhysicalScreenSession(externalSession->GetRSScreenId());
    if (phyScreenSession) {
        phyScreenSession->UpdateMirrorWidth(targetRect.width_);
        phyScreenSession->UpdateMirrorHeight(targetRect.height_);
    }
}

bool ScreenSessionManager::HandleCastVirtualScreenMirrorRegion()
{
    if (!g_isPcDevice) {
        TLOGW(WmsLogTag::DMS, "not support");
        return false;
    }
    TLOGI(WmsLogTag::DMS, "SetVirtualScreenMirrorRegion");
    sptr<ScreenSession> internalSession = nullptr;
    sptr<ScreenSession> virtualSession = nullptr;
    GetCastVirtualMirrorSession(virtualSession);
    internalSession = GetInternalScreenSession();
    if (virtualSession == nullptr || internalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "ScreenSession Null");
        return false;
    }
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && IsVertical(internalSession->GetRotation())) {
        TLOGI(WmsLogTag::DMS, "SuperFoldDisplayDevice Vertical");
        return false;
    }
    //all zero, means full screen mirror.
    DMRect mirrorRegion = DMRect::NONE();
    if (curResolutionEffectEnable_.load()) {
        auto property = internalSession->GetScreenProperty();
        auto bounds = property.GetBounds();
        mirrorRegion = DMRect{bounds.rect_.left_, bounds.rect_.top_,
            property.GetScreenAreaWidth(), property.GetScreenAreaHeight()};
    }
    virtualSession->SetMirrorScreenRegion(virtualSession->GetScreenId(), mirrorRegion);
    virtualSession->EnableMirrorScreenRegion();
    return true;
}

void ScreenSessionManager::GetCastVirtualMirrorSession(sptr<ScreenSession>& virtualSession)
{
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
            continue;
        }
        if (screenSession->GetVirtualScreenFlag() == VirtualScreenFlag::CAST &&
            screenSession->GetMirrorScreenType() == MirrorScreenType::VIRTUAL_MIRROR) {
            TLOGI(WmsLogTag::DMS, "virtual id: %{public}" PRIu64"", screenSession->GetScreenId());
            virtualSession = screenSession;
            return;
        }
    }
    TLOGI(WmsLogTag::DMS, "no virtual mirror screen");
}

DMError ScreenSessionManager::GetDensityInCurResolution(ScreenId screenId, float& virtualPixelRatio)
{
    DmsXcollie dmsXcollie("DMS:GetDensityInCurResolution", XCOLLIE_TIMEOUT_10S);
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "Get ScreenSession failed");
        return DMError::DM_ERROR_NULLPTR;
    }

    virtualPixelRatio = screenSession->GetScreenProperty().GetDensityInCurResolution();
    return DMError::DM_OK;
}

DMError ScreenSessionManager::GetScreenColorGamut(ScreenId screenId, ScreenColorGamut& colorGamut)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 "", screenId);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "Get ScreenSession failed");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screenSession->GetScreenColorGamut(colorGamut);
#else
    return DMError::DM_OK;
#endif
}

DMError ScreenSessionManager::SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 ", colorGamutIdx %{public}d",
        screenId, colorGamutIdx);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "Get ScreenSession failed");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screenSession->SetScreenColorGamut(colorGamutIdx);
#else
    return DMError::DM_OK;
#endif
}

DMError ScreenSessionManager::GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 "", screenId);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "Get ScreenSession failed");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screenSession->GetScreenGamutMap(gamutMap);
#else
    return DMError::DM_OK;
#endif
}

DMError ScreenSessionManager::SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap)
{
#ifdef WM_SCREEN_COLOR_GAMUT_ENABLE
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 ", ScreenGamutMap %{public}u",
        screenId, static_cast<uint32_t>(gamutMap));
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "Get ScreenSession failed");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screenSession->SetScreenGamutMap(gamutMap);
#else
    return DMError::DM_OK;
#endif
}

DMError ScreenSessionManager::SetScreenColorTransform(ScreenId screenId)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 "", screenId);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "Get ScreenSession failed");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screenSession->SetScreenColorTransform();
}

sptr<ScreenSession> ScreenSessionManager::GetPhysicalScreenSession(ScreenId screenId, ScreenId defScreenId,
    ScreenProperty property)
{
    sptr<ScreenSession> screenSession = nullptr;
    ScreenSessionConfig config;
    sptr<ScreenSession> defaultScreen = GetDefaultScreenSession();
    if (defaultScreen == nullptr || defaultScreen->GetDisplayNode() == nullptr) {
        TLOGE(WmsLogTag::DMS, "default screen null");
        return screenSession;
    }
    NodeId nodeId = defaultScreen->GetDisplayNode()->GetId();
    TLOGI(WmsLogTag::DMS, "physical mirror screen nodeId: %{public}" PRIu64, nodeId);
    config = {
        .screenId = screenId,
        .rsId = screenId,
        .defaultScreenId = defScreenId,
        .mirrorNodeId = nodeId,
        .property = property,
    };
    screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
    screenSession->SetIsPhysicalMirrorSwitch(true);
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ != nullptr && FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        DMRect mainScreenRegion = { 0, 0, 0, 0 };
        foldScreenController_->SetMainScreenRegion(mainScreenRegion);
        screenSession->SetMirrorScreenRegion(screenId, mainScreenRegion);
        screenSession->SetIsEnableRegionRotation(true);
        screenSession->EnableMirrorScreenRegion();
    }
#endif
    return screenSession;
}

sptr<ScreenSession> ScreenSessionManager::CreatePhysicalMirrorSessionInner(ScreenId screenId, ScreenId defScreenId,
    ScreenProperty property)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    sptr<ScreenSession> screenSession = nullptr;
    if (system::GetBoolParameter("persist.edm.disallow_mirror", false)) {
        TLOGW(WmsLogTag::DMS, "mirror disabled by edm!");
        return screenSession;
    }
    screenSession = GetPhysicalScreenSession(screenId, defScreenId, property);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return nullptr;
    }
    MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_EXTEND, MULTI_SCREEN_ENTER_STR);
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        TLOGI(WmsLogTag::DMS, "set ExtendConnect flag = true");
        SetIsPhysicalExtendScreenConnected(true);
        extendScreenConnectStatus_.store(ExtendScreenConnectStatus::CONNECT);
        OnExtendScreenConnectStatusChange(screenId, ExtendScreenConnectStatus::CONNECT);
    }
    if (g_isPcDevice) {
        // pc is none, pad&&phone is mirror
        InitExtendScreenProperty(screenId, screenSession, property);
        screenSession->SetName(SCREEN_NAME_EXTEND);
        screenSession->SetIsExtend(true);
        screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    } else {
        screenSession->SetIsExtend(true);
        screenSession->SetName(SCREEN_NAME_CAST);
        screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
        screenSession->SetVirtualScreenFlag(VirtualScreenFlag::CAST);
    }
    GetAndMergeEdidInfo(screenSession);
    screenSession->SetMirrorScreenType(MirrorScreenType::PHYSICAL_MIRROR);
    screenSession->SetIsPcUse(g_isPcDevice ? true : false);
    screenSession->SetIsInternal(false);
    screenSession->SetIsRealScreen(true);
    screenSession->SetIsCurrentInUse(true);
    NotifyScreenChanged(screenSession->ConvertToScreenInfo(), ScreenChangeEvent::SCREEN_SWITCH_CHANGE);
    if (!g_isPcDevice) {
        hdmiScreenCount_ = hdmiScreenCount_ + 1;
        NotifyCaptureStatusChanged();
    }
    return screenSession;
#else
    return nullptr;
#endif
}

sptr<ScreenSession> ScreenSessionManager::GetScreenSessionInner(ScreenId screenId, ScreenProperty property)
{
    ScreenId defScreenId = GetDefaultScreenId();
    TLOGW(WmsLogTag::DMS, "screenId:%{public}" PRIu64 "", screenId);
    if (IsDefaultMirrorMode(screenId)) {
#ifdef WM_MULTI_SCREEN_ENABLE
        return CreatePhysicalMirrorSessionInner(screenId, defScreenId, property);
#else
        return nullptr;
#endif
    }
    std::string screenName = GetScreenName(screenId);
    if (screenId == SCREEN_ID_MAIN) {
        screenName = "SubScreen";
    }
    ScreenId smsId = screenId;
    if (IsConcurrentUser()) {
        if (!screenIdManager_.ConvertToSmsScreenId(screenId, smsId)) {
            smsId = screenId;
        }
    }
    ScreenSessionConfig config = {
        .screenId = smsId,
        .rsId = screenId,
        .defaultScreenId = defScreenId,
        .name = screenName,
        .property = property,
    };
    sptr<ScreenSession> screenSession = nullptr;
    screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    screenSession->SetIsExtend(false);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    screenSession->SetIsInternal(true);
    screenSession->SetIsRealScreen(true);
    screenSession->SetIsCurrentInUse(true);
    screenSession->SetIsPcUse(g_isPcDevice ? true : false);
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        InitFakeScreenSession(screenSession);
    }
    return screenSession;
}

void ScreenSessionManager::SetupScreenDensityProperties(ScreenId screenId, ScreenProperty& property, RRect bounds)
{
    TLOGD(WmsLogTag::DMS, "Start to setup screen density properties");
    ScreenId phyScreenId = GetPhyScreenId(screenId);
    if (IsConcurrentUser()) {
        bool found = false;
        const std::vector<DisplayConfig>& displayConfigs = ScreenSceneConfig::GetDisplaysConfigs();
        for (const auto& it : displayConfigs) {
            if (it.physicalId == screenId && it.dpi) {
                float screenDpi = static_cast<float>(it.dpi) / BASELINE_DENSITY;
                TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 ", DensityDpi: %{public}d", screenId, it.dpi);
                property.SetScreenDensityProperties(screenDpi);
                found = true;
                break;
            }
        }
        if (!found) {
            TLOGI(WmsLogTag::DMS, "find no match dpi, use default densityDpi_ = %{public}f", densityDpi_);
            property.SetScreenDensityProperties(densityDpi_);
        }
    } else if (isDensityDpiLoad_) {
        if (phyScreenId == SCREEN_ID_MAIN) {
            TLOGI(WmsLogTag::DMS, "subDensityDpi_ = %{public}f", subDensityDpi_);
            property.SetScreenDensityProperties(subDensityDpi_);
        } else {
            TLOGI(WmsLogTag::DMS, "densityDpi_ = %{public}f", densityDpi_);
            property.SetScreenDensityProperties(densityDpi_);
        }
    } else {
        property.UpdateVirtualPixelRatio(bounds);
    }
}

void ScreenSessionManager::CreateScreenProperty(ScreenId screenId, ScreenProperty& property)
{
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("CreateScreenPropertyCallRS", XCOLLIE_TIMEOUT_10S, nullptr,
        nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
    TLOGW(WmsLogTag::DMS, "Call rsInterface_ GetScreenActiveMode ScreenId: %{public}" PRIu64, screenId);
    ScreenId phyScreenId = GetPhyScreenId(screenId);
    auto screenMode = rsInterface_.GetScreenActiveMode(phyScreenId);
    TLOGW(WmsLogTag::DMS, "get screenWidth: %{public}d, screenHeight: %{public}d",
        static_cast<uint32_t>(screenMode.GetScreenWidth()), static_cast<uint32_t>(screenMode.GetScreenHeight()));
    auto screenBounds = GetScreenBounds(screenId, screenMode);
    auto screenRefreshRate = screenMode.GetScreenRefreshRate();
    TLOGW(WmsLogTag::DMS, "Call rsInterface_ GetScreenCapability ScreenId: %{public}" PRIu64, phyScreenId);
    auto screenCapability = rsInterface_.GetScreenCapability(phyScreenId);
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    TLOGW(WmsLogTag::DMS, "Call RS interface end, create ScreenProperty begin");
    InitScreenProperty(screenId, screenMode, screenCapability, property);
    SetupScreenDensityProperties(screenId, property, screenBounds);
    property.SetRefreshRate(screenRefreshRate);
    property.SetDefaultDeviceRotationOffset(defaultDeviceRotationOffset_);
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ != nullptr && screenId == 0
        && (g_screenRotationOffSet == ROTATION_90 || g_screenRotationOffSet == ROTATION_270)) {
        screenBounds = RRect({ 0, 0, screenMode.GetScreenHeight(), screenMode.GetScreenWidth() }, 0.0f, 0.0f);
        property.SetBounds(screenBounds);
    }
    sptr<FoldCreaseRegion> creaseRegion = GetCurrentFoldCreaseRegion();
    if (creaseRegion != nullptr) {
        std::vector<DMRect> creaseRects = creaseRegion->GetCreaseRects();
        if (creaseRects.size() > 0) {
            property.SetCreaseRect(creaseRects[0]);
        }
    }
#endif
    property.CalcDefaultDisplayOrientation();
    property.SetScreenShape(ScreenSettingHelper::GetScreenShape(screenId));
    DisplayGroupId groupId = DISPLAY_GROUP_ID_DEFAULT;
    ScreenId mainDisplayId = MAIN_SCREEN_ID_DEFAULT;
    if (IsConcurrentUser() && screenId == SCREEN_ID_MAIN) {
        groupId = displayGroupNum_++;
        mainDisplayId = screenId;
        screenIdManager_.ConvertToSmsScreenId(screenId, mainDisplayId);
    }
    if (property.GetDisplayGroupId() == DISPLAY_GROUP_ID_INVALID) {
        property.SetDisplayGroupId(groupId);
    }
    if (property.GetMainDisplayIdOfGroup() == SCREEN_ID_INVALID) {
        property.SetMainDisplayIdOfGroup(mainDisplayId);
    }
}

const std::string ScreenSessionManager::GetScreenName(ScreenId screenId)
{
    if (!IsConcurrentUser()) {
        TLOGI(WmsLogTag::DMS, "Not a concurrent user, returning default screen name");
        return "UNKNOWN";
    }
    std::string screenName("UNKNOWN");
    const std::vector<DisplayConfig>& displayConfigs = ScreenSceneConfig::GetDisplaysConfigs();
    for (const auto& it : displayConfigs) {
        if (it.physicalId == screenId && !it.name.empty()) {
            screenName = it.name;
            TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 ", ScreenName: %{public}s", screenId,
                screenName.c_str());
            break;
        }
    }
    return screenName;
}

void ScreenSessionManager::InitScreenProperty(ScreenId screenId, RSScreenModeInfo& screenMode,
    RSScreenCapability& screenCapability, ScreenProperty& property)
{
    auto screenBounds = GetScreenBounds(screenId, screenMode);
    property.SetRotation(0.0f);
    property.SetPhyWidth(screenCapability.GetPhyWidth());
    property.SetPhyHeight(screenCapability.GetPhyHeight());
    property.SetValidWidth(screenBounds.rect_.width_);
    property.SetValidHeight(screenBounds.rect_.height_);
    property.SetDpiPhyBounds(screenCapability.GetPhyWidth(), screenCapability.GetPhyHeight());
    property.SetPhyBounds(screenBounds);
    property.SetBounds(screenBounds);
    property.SetAvailableArea({0, 0, screenMode.GetScreenWidth(), screenMode.GetScreenHeight()});
    property.SetPhysicalTouchBounds(GetConfigCorrectionByDisplayMode(GetFoldDisplayMode()));
    property.SetCurrentOffScreenRendering(false);
    property.SetScreenRealWidth(property.GetBounds().rect_.GetWidth());
    property.SetScreenRealHeight(property.GetBounds().rect_.GetHeight());
    property.SetMirrorWidth(property.GetBounds().rect_.GetWidth());
    property.SetMirrorHeight(property.GetBounds().rect_.GetHeight());
    property.SetScreenRealPPI();
    property.SetScreenRealDPI();
    property.SetScreenAreaOffsetX(property.GetPhyBounds().rect_.GetLeft());
    property.SetScreenAreaOffsetY(property.GetPhyBounds().rect_.GetTop());
    property.SetScreenAreaWidth(property.GetPhyBounds().rect_.GetWidth());
    property.SetScreenAreaHeight(property.GetPhyBounds().rect_.GetHeight());
}

RRect ScreenSessionManager::GetScreenBounds(ScreenId screenId, RSScreenModeInfo& screenMode)
{
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice() &&
        GetCoordinationFlag() && screenId == SCREEN_ID_MAIN) {
        if (screenParams_.size() < STATUS_PARAM_VALID_INDEX) {
            TLOGE(WmsLogTag::DMS, "invalid param num, use default");
            return RRect({ 0, 0, MAIN_STATUS_DEFAULT_WIDTH, SCREEN_DEFAULT_HEIGHT }, 0.0f, 0.0f);
        }
        return RRect({ 0, 0, screenParams_[MAIN_STATUS_WIDTH], screenParams_[MAIN_STATUS_HEIGHT] }, 0.0f, 0.0f);
    }
    return RRect({ 0, 0, screenMode.GetScreenWidth(), screenMode.GetScreenHeight() }, 0.0f, 0.0f);
}

void ScreenSessionManager::GetInternalWidth()
{
    ScreenId screenId = GetInternalScreenId();
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen session is null");
        return;
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    uint32_t screenWidth = screenProperty.GetScreenRealWidth();
    uint32_t screenHeight = screenProperty.GetScreenRealHeight();
    g_internalWidth = (screenWidth > screenHeight) ? screenWidth : screenHeight;
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64", g_internalWidth is:%{public}u",
        screenId, static_cast<uint32_t>(g_internalWidth));
    return;
}

void ScreenSessionManager::InitExtendScreenProperty(ScreenId screenId, sptr<ScreenSession> session,
    ScreenProperty property)
{
    if (!g_isPcDevice) {
        TLOGW(WmsLogTag::DMS, "not PC device");
        return;
    }
    bool isSupportOffScreenRendering = ScreenSceneConfig::IsSupportOffScreenRendering();
    if (!isSupportOffScreenRendering) {
        TLOGW(WmsLogTag::DMS, "xml isSupportOffScreenRendering is fasle");
        return;
    }
    GetInternalWidth();
    TLOGD(WmsLogTag::DMS, "g_internalWidth: %{public}u", static_cast<uint32_t>(g_internalWidth));
    float offScreenPPIThreshold = static_cast<float>(ScreenSceneConfig::GetOffScreenPPIThreshold());
    uint32_t screenWidth = property.GetBounds().rect_.GetWidth();
    uint32_t screenHeight = property.GetBounds().rect_.GetHeight();
    float screenPPI = property.GetScreenRealPPI();
    if (screenWidth == 0) {
        TLOGE(WmsLogTag::DMS, "screenWidth is zero");
        return;
    }
    if (screenWidth > FOUR_K_WIDTH) {
        float scale = static_cast<float>(FOUR_K_WIDTH) / screenWidth;
        uint32_t screenAdjustHeight = static_cast<uint32_t>(std::round(screenHeight * scale));
        auto screenBounds = RRect({ 0, 0, FOUR_K_WIDTH, screenAdjustHeight }, 0.0f, 0.0f);
        session->SetExtendProperty(screenBounds, true);
        session->SetAvailableArea({0, 0, FOUR_K_WIDTH, screenAdjustHeight});
        session->SetValidWidth(FOUR_K_WIDTH);
        session->SetValidHeight(screenAdjustHeight);
        TLOGD(WmsLogTag::DMS, "screenWidth > 4K, screenId:%{public}" PRIu64"", screenId);
    } else if (screenWidth < THREE_K_WIDTH && screenPPI < offScreenPPIThreshold) {
        float scale = static_cast<float>(g_internalWidth) / screenWidth;
        uint32_t screenAdjustHeight = static_cast<uint32_t>(std::round(screenHeight * scale));
        auto screenBounds = RRect({ 0, 0, g_internalWidth, screenAdjustHeight }, 0.0f, 0.0f);
        session->SetExtendProperty(screenBounds, true);
        session->SetAvailableArea({0, 0, g_internalWidth, screenAdjustHeight});
        session->SetValidWidth(g_internalWidth);
        session->SetValidHeight(screenAdjustHeight);
        TLOGD(WmsLogTag::DMS, "screenWidth < g_internalWidth, screenId:%{public}" PRIu64"", screenId);
    } else {
        TLOGW(WmsLogTag::DMS, "no need adjust, screenId:%{public}" PRIu64"", screenId);
        return;
    }
    std::ostringstream oss;
    oss << "screenId:" << screenId
        << ", screenWidth: " <<  session->GetScreenProperty().GetBounds().rect_.GetWidth()
        << ", screenHeight: " << session->GetScreenProperty().GetBounds().rect_.GetHeight()
        << ", propertyCurrentOffScreenRendering: " << session->GetScreenProperty().GetCurrentOffScreenRendering()
        << ", screenPPI: " << screenPPI;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
}

void ScreenSessionManager::SetExtendedScreenFallbackPlan(ScreenId screenId)
{
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen session is null");
        return;
    }
    if (screenSession->GetIsInternal()) {
        TLOGW(WmsLogTag::DMS, "screen is internal");
        SetInnerScreenFallbackPlan(screenSession);
        return;
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    if (!screenProperty.GetCurrentOffScreenRendering()) {
        TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64", propertyCurrentOffScreenRendering is false", screenId);
        return;
    }
    uint32_t screenAdjustWidth = 0;
    uint32_t screenAdjustHeight = 0;
    if (screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
        TLOGD(WmsLogTag::DMS, "Screen is mirror");
        screenAdjustWidth = screenProperty.GetScreenRealWidth();
        screenAdjustHeight = screenProperty.GetScreenRealHeight();
    } else {
        TLOGD(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64", setPhysicalScreenResolution", screenId);
        screenAdjustWidth = screenProperty.GetBounds().rect_.GetWidth();
        screenAdjustHeight = screenProperty.GetBounds().rect_.GetHeight();
    }
    ScreenId rsScreenId;
    if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId.");
        return;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetPhysicalScreenResolution(%" PRIu64")", screenId);
    int32_t res = rsInterface_.SetPhysicalScreenResolution(rsScreenId, screenAdjustWidth, screenAdjustHeight);
    if ((screenSession->GetScreenCombination() == ScreenCombination::SCREEN_EXTEND ||
        screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MAIN) && !screenSession->GetIsInternal()) {
        screenSession->PropertyChange(screenSession->GetScreenProperty(), ScreenPropertyChangeReason::UNDEFINED);
        screenSession->SetFrameGravity(Rosen::Gravity::RESIZE);
    } else {
        screenSession->SetFrameGravity(Rosen::Gravity::TOP_LEFT);
    }
    SetExtendedScreenFallbackPlanEvent(res);
    TLOGI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 ", screenAdjustWidth:%{public}u, screenAdjustHeight:%{public}u",
        screenId, screenAdjustWidth, screenAdjustHeight);
}

void ScreenSessionManager::SetExtendedScreenFallbackPlanEvent(int32_t res)
{
    if (res != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS, "RS SetPhysicalScreenResolution failed.");
        screenEventTracker_.RecordEvent("SetPhysicalScreenResolution failed.");
    } else {
        TLOGI(WmsLogTag::DMS, "RS SetPhysicalScreenResolution success.");
        screenEventTracker_.RecordEvent("SetPhysicalScreenResolution success.");
    }
}


void ScreenSessionManager::SetInnerScreenFallbackPlan(sptr<ScreenSession> screenSession)
{
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    uint32_t screenAdjustWidth = screenProperty.GetScreenRealWidth();
    uint32_t screenAdjustHeight = screenProperty.GetScreenRealHeight();
    ScreenId screenId = screenSession->GetScreenId();
    ScreenId rsScreenId;
    if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId.");
        return;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetPhysicalScreenResolution(%" PRIu64")", screenId);
    int32_t res = rsInterface_.SetPhysicalScreenResolution(rsScreenId, screenAdjustWidth, screenAdjustHeight);
    if (res != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS, "RS SetPhysicalScreenResolution failed.");
        screenEventTracker_.RecordEvent("SetPhysicalScreenResolution failed.");
    } else {
        TLOGI(WmsLogTag::DMS, "RS SetPhysicalScreenResolution success.");
        screenEventTracker_.RecordEvent("SetPhysicalScreenResolution success.");
    }
    TLOGI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 ", screenAdjustWidth:%{public}u, screenAdjustHeight:%{public}u",
        screenId, screenAdjustWidth, screenAdjustHeight);
}

void ScreenSessionManager::InitExtendScreenDensity(sptr<ScreenSession> session, ScreenProperty property)
{
    if (session->GetScreenProperty().GetScreenType() != ScreenType::REAL || session->isInternal_) {
        // 表示非拓展屏
        TLOGW(WmsLogTag::DMS, "Not expandable screen, no need to set dpi");
        return;
    }
    ScreenId mainScreenId = GetDefaultScreenId();
    sptr<ScreenSession> screenSession = GetScreenSession(mainScreenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    float extendDensity = screenSession->GetScreenProperty().GetDensity();
    float curResolution = screenSession->GetScreenProperty().GetDensityInCurResolution();
    float defaultDensity = screenSession->GetScreenProperty().GetDefaultDensity();
    TLOGW(WmsLogTag::DMS, "extendDensity = %{public}f", extendDensity);
    session->SetVirtualPixelRatio(extendDensity * g_extendScreenDpiCoef);
    session->SetDefaultDensity(defaultDensity * EXTEND_SCREEN_DPI_DEFAULT_PARAMETER);
    session->SetDensityInCurResolution(curResolution);
    ScreenId screenId = session->GetScreenId();
    property.SetVirtualPixelRatio(extendDensity * g_extendScreenDpiCoef);
    property.SetDefaultDensity(defaultDensity * EXTEND_SCREEN_DPI_DEFAULT_PARAMETER);
    property.SetDensityInCurResolution(curResolution);
    {
        std::lock_guard<std::recursive_mutex> lock_phy(phyScreenPropMapMutex_);
        phyScreenPropMap_[screenId] = property;
    }
    return;
}

sptr<ScreenSession> ScreenSessionManager::GetOrCreateScreenSession(ScreenId screenId)
{
    TLOGW(WmsLogTag::DMS, "ENTER. ScreenId: %{public}" PRIu64 "", screenId);
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession) {
        TLOGW(WmsLogTag::DMS, "screenSession Exist ScreenId: %{public}" PRIu64, screenId);
        return screenSession;
    }
    if (g_isPcDevice) {
        std::lock_guard<std::recursive_mutex> lock_phy(phyScreenPropMapMutex_);
        if (phyScreenPropMap_.size() > 1) {
            // pc is none, pad&&phone is mirror
            TLOGW(WmsLogTag::DMS, "only support one external screen");
            return nullptr;
        }
    }
    ScreenId smsScreenId = GenerateSmsScreenId(screenId);
    TLOGI(WmsLogTag::DMS, "get the mapping between rsScreenId: %{public}" PRIu64 " and smsScreenId: %{public}" PRIu64,
    screenId, smsScreenId);
    ScreenProperty property;
    CreateScreenProperty(screenId, property);
    TLOGW(WmsLogTag::DMS, "call create screen property end");
    screenEventTracker_.RecordEvent("CreateScreenProperty by rsInterface success.");
    {
        std::lock_guard<std::recursive_mutex> lock_phy(phyScreenPropMapMutex_);
        phyScreenPropMap_[screenId] = property;
    }

    if (HandleFoldScreenSessionCreate(screenId) == false) {
        return nullptr;
    }

    sptr<ScreenSession> session = GetScreenSessionInner(screenId, property);
    if (session == nullptr) {
        TLOGE(WmsLogTag::DMS, "get screen session fail ScreenId: %{public}" PRIu64, screenId);
        return session;
    }
    session->RegisterScreenChangeListener(this);
    InitExtendScreenDensity(session, property);
    InitAbstractScreenModesInfo(session);
    session->groupSmsId_ = 1;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        screenSessionMap_[smsScreenId] = session;
    }
    if (g_isPcDevice || IS_SUPPORT_PC_MODE) {
        SetMultiScreenFrameControl();
    }
    screenEventTracker_.RecordEvent("create screen session success.");
    SetHdrFormats(GetPhyScreenId(screenId), session);
    SetColorSpaces(GetPhyScreenId(screenId), session);
    SetSupportedRefreshRate(session);
    if (session->GetFakeScreenSession() != nullptr) {
        std::vector<uint32_t> supportedRefreshRateFake = session->GetSupportedRefreshRate();
        sptr<ScreenSession> screenSessionFake = session->GetFakeScreenSession();
        screenSessionFake->SetSupportedRefreshRate(std::move(supportedRefreshRateFake));
    }
    RegisterRefreshRateChangeListener();
    session->SetPhyScreenId(screenId);
    TLOGW(WmsLogTag::DMS, "CreateScreenSession success. ScreenId: %{public}" PRIu64 "", screenId);
    return session;
}

bool ScreenSessionManager::HandleFoldScreenSessionCreate(ScreenId screenId)
{
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ != nullptr) {
        // sensor may earlier than screen connect, when physical screen property changed, update
        foldScreenController_->UpdateForPhyScreenPropertyChange();
        /* folder screen outer screenId is 5 */
        if (screenId == SCREEN_ID_MAIN) {
            SetPostureAndHallSensorEnabled();
            if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()
                || FoldScreenStateInternel::IsDualDisplayFoldDevice()
                || FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) {
                ScreenSensorConnector::SubscribeTentSensor();
            }
            isFoldScreenOuterScreenReady_ = true;
            if (!FoldScreenStateInternel::IsDualDisplayFoldDevice() && isCoordinationFlag_ == false) {
                return false;
            }
        }
    }
#endif
    return true;
}

void ScreenSessionManager::SetHdrFormats(ScreenId screenId, sptr<ScreenSession>& session)
{
    TLOGI(WmsLogTag::DMS, "SetHdrFormats %{public}" PRIu64, screenId);
    std::vector<ScreenHDRFormat> rsHdrFormat;
    auto status = rsInterface_.GetScreenSupportedHDRFormats(screenId, rsHdrFormat);
    if (static_cast<StatusCode>(status) != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS, "get hdr format failed! status code: %{public}d", status);
    } else {
        std::vector<uint32_t> hdrFormat(rsHdrFormat.size());
        std::transform(rsHdrFormat.begin(), rsHdrFormat.end(), hdrFormat.begin(), [](int val) {
            return static_cast<uint32_t>(val);
        });
        session->SetHdrFormats(std::move(hdrFormat));
    }
}

void ScreenSessionManager::SetColorSpaces(ScreenId screenId, sptr<ScreenSession>& session)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "spaces permission denied!");
        return;
    }

    TLOGI(WmsLogTag::DMS, "SetColorSpaces %{public}" PRIu64, screenId);
    std::vector<GraphicCM_ColorSpaceType> rsColorSpace;
    auto status = rsInterface_.GetScreenSupportedColorSpaces(screenId, rsColorSpace);
    if (static_cast<StatusCode>(status) != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS, "get color space failed! status code: %{public}d", status);
    } else {
        std::vector<uint32_t> colorSpace(rsColorSpace.size());
        std::transform(rsColorSpace.begin(), rsColorSpace.end(), colorSpace.begin(), [](int val) {
            return static_cast<uint32_t>(val);
        });
        session->SetColorSpaces(std::move(colorSpace));
    }
}

DMError ScreenSessionManager::GetBrightnessInfo(DisplayId displayId, ScreenBrightnessInfo& brightnessInfo)
{
    TLOGI(WmsLogTag::DMS, "start");
    sptr<ScreenSession> screenSession = GetScreenSession(displayId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetScreenSession failed");
        return DMError::DM_ERROR_ILLEGAL_PARAM;
    }
    BrightnessInfo rsBrightnessInfo;
    auto status = rsInterface_.GetBrightnessInfo(screenSession->GetPhyScreenId(), rsBrightnessInfo);
    if (static_cast<StatusCode>(status) != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS, "get screen brightness info failed! status code:%{public}d", status);
        return DMError::DM_ERROR_ILLEGAL_PARAM;
    }
    TLOGI(WmsLogTag::DMS, "RS brightnessInfo currentHeadroom:%{public}f maxHeadroom:%{public}f sdrNits:%{public}f",
          rsBrightnessInfo.currentHeadroom, rsBrightnessInfo.maxHeadroom, rsBrightnessInfo.sdrNits);
    brightnessInfo.currentHeadroom = rsBrightnessInfo.currentHeadroom;
    brightnessInfo.maxHeadroom = rsBrightnessInfo.maxHeadroom;
    brightnessInfo.sdrNits = rsBrightnessInfo.sdrNits;
    return DMError::DM_OK;
}

DMError ScreenSessionManager::GetSupportsInput(DisplayId displayId, bool& supportsInput)
{
    TLOGI(WmsLogTag::DMS, "start");
    sptr<ScreenSession> screenSession = GetScreenSession(displayId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetScreenSession failed");
        return DMError::DM_ERROR_ILLEGAL_PARAM;
    }
    supportsInput = screenSession->GetSupportsInput();
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetSupportsInput(DisplayId displayId, bool supportsInput)
{
    TLOGI(WmsLogTag::DMS, "start");
    sptr<ScreenSession> screenSession = GetScreenSession(displayId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetScreenSession failed");
        return DMError::DM_ERROR_ILLEGAL_PARAM;
    }
    screenSession->SetSupportsInput(supportsInput);
    return DMError::DM_OK;
}

static std::vector<int> GetDeviceRadiusFormConfig(float dpi)
{
    std::vector<int> result;
    std::string token;
    std::string radiusStr(REAL_DEVICE_RADIUS);
    std::vector<std::string> radius = FoldScreenStateInternel::StringSplit(radiusStr, ',');
    for (const auto& item : radius) {
        float value = std::stof(item);
        if (value > 0) {
            int radiusPx = static_cast<int>(std::ceil(dpi * value)); //convert radius vp to px
            result.push_back(radiusPx);
        } else {
            break;
        }
    }
    return result;
}

DMError ScreenSessionManager::GetRoundedCorner(DisplayId displayId, int& radius)
{
    TLOGI(WmsLogTag::DMS, "start");
    sptr<ScreenSession> screenSession = GetScreenSession(displayId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "GetScreenSession failed");
        return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
    }
    auto deviceRadius = GetDeviceRadiusFormConfig(screenSession->GetVirtualPixelRatio());
    if (deviceRadius.empty()) {
        return DMError::DM_OK;
    }
    if (screenSession->GetPhyScreenId() == 0) {
        radius = deviceRadius[0];
    } else if (screenSession->GetPhyScreenId() == 5 && deviceRadius.size() > 1) {
        radius = deviceRadius[1];
    }
    return DMError::DM_OK;
}

void ScreenSessionManager::SetSupportedRefreshRate(sptr<ScreenSession>& session)
{
    std::vector<RSScreenModeInfo> allModes = rsInterface_.GetScreenSupportedModes(
        screenIdManager_.ConvertToRsScreenId(GetPhyScreenId(session->screenId_)));
    if (allModes.size() == 0) {
        TLOGE(WmsLogTag::DMS, "allModes.size() == 0, screenId=%{public}" PRIu64"", session->rsId_);
        return;
    }
    std::set<uint32_t> uniqueRefreshRates;
    for (const RSScreenModeInfo& rsScreenModeInfo : allModes) {
        uniqueRefreshRates.insert(rsScreenModeInfo.GetScreenRefreshRate());
    }
    std::vector<uint32_t> supportedRefreshRate(uniqueRefreshRates.begin(), uniqueRefreshRates.end());
    session->SetSupportedRefreshRate(std::move(supportedRefreshRate));
}

ScreenId ScreenSessionManager::GetDefaultScreenId()
{
    if (defaultScreenId_ == INVALID_SCREEN_ID) {
        defaultScreenId_ = rsInterface_.GetDefaultScreenId();
        std::ostringstream oss;
        oss << "Default screen id : " << defaultScreenId_;
        TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
        screenEventTracker_.RecordEvent(oss.str());
    }
    return defaultScreenId_;
}

bool ScreenSessionManager::WakeUpBegin(PowerStateChangeReason reason)
{
    // 该接口当前只有Power调用
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[UL_POWER]ssm:WakeUpBegin(%u)", reason);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "[UL_POWER]reason: %{public}u", reason);
    ScreenPowerInfoType type = reason;
    if (ScreenStateMachine::GetInstance().GetTransitionState() == ScreenTransitionState::SCREEN_INIT) {
        return DoWakeUpBegin(reason);
    } else {
        if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS ||
            reason == PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF) {
            TLOGI(WmsLogTag::DMS, "[UL_POWER]WakeUpBegin reason: %{public}u", reason);
            return ScreenStateMachine::GetInstance().HandlePowerStateChange(ScreenPowerEvent::WAKEUP_BEGIN_ADVANCED, type);
        }
        return ScreenStateMachine::GetInstance().HandlePowerStateChange(ScreenPowerEvent::WAKEUP_BEGIN, type);
    }
}

bool ScreenSessionManager::DoWakeUpBegin(PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS, "[UL_POWER]reason: %{public}u", reason);
    if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_START_DREAM) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]wakeup cannot start dream");
        return false;
    }
    if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_END_DREAM) {
        NotifyDisplayPowerEvent(DisplayPowerEvent::DISPLAY_END_DREAM, EventStatus::BEGIN, reason);
        return BlockScreenWaitPictureFrameByCV(false);
    }
    currentWakeUpReason_ = reason;
    // 多屏协作灭屏不通知锁屏
    if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION) {
        isMultiScreenCollaboration_ = true;
        return true;
    }
    lastWakeUpReason_ = reason;
    return NotifyDisplayPowerEvent(DisplayPowerEvent::WAKE_UP, EventStatus::BEGIN, reason);
}

bool ScreenSessionManager::WakeUpEnd()
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[UL_POWER]ssm:WakeUpEnd");
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "[UL_POWER]WakeUpEnd enter");
    // 多屏协作灭屏不通知锁屏
    if (isMultiScreenCollaboration_) {
        isMultiScreenCollaboration_ = false;
        return true;
    }
    return NotifyDisplayPowerEvent(DisplayPowerEvent::WAKE_UP, EventStatus::END,
        PowerStateChangeReason::STATE_CHANGE_REASON_INIT);
}

bool ScreenSessionManager::SuspendBegin(PowerStateChangeReason reason)
{
    // only power use
    powerStateChangeReason_ = reason;
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[UL_POWER]ssm:SuspendBegin(%u)", reason);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "[UL_POWER]Reason: %{public}u", static_cast<uint32_t>(reason));
    ScreenPowerInfoType type = reason;
    if (ScreenStateMachine::GetInstance().GetTransitionState() == ScreenTransitionState::SCREEN_INIT) {
        return DoSuspendBegin(reason);
    } else {
        return ScreenStateMachine::GetInstance().HandlePowerStateChange(ScreenPowerEvent::SUSPEND_BEGIN, type);
    }
}

bool ScreenSessionManager::DoSuspendBegin(PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS, "[UL_POWER]Reason: %{public}u", static_cast<uint32_t>(reason));
    if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_END_DREAM) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]suspend cannot end dream");
        return false;
    }
    if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_START_DREAM) {
        NotifyDisplayPowerEvent(DisplayPowerEvent::DISPLAY_START_DREAM, EventStatus::BEGIN, reason);
        return BlockScreenWaitPictureFrameByCV(true);
    }
    gotScreenlockFingerprint_ = false;
    lastWakeUpReason_ = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF) {
        lastWakeUpReason_ = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
    }
    // 多屏协作灭屏不通知锁屏
    gotScreenOffNotify_  = false;
    if (!g_isPcDevice && reason != PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT &&
        reason != PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS &&
        reason != PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON &&
        reason != PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF &&
        reason != PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION) {
        sessionDisplayPowerController_->canCancelSuspendNotify_ = true;
    }
    sessionDisplayPowerController_->isSuspendBegin_ = true;
    sessionDisplayPowerController_->SuspendBegin(reason);
    if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION) {
        isMultiScreenCollaboration_ = true;
        return true;
    }
    return NotifyDisplayPowerEvent(DisplayPowerEvent::SLEEP, EventStatus::BEGIN, reason);
}

bool ScreenSessionManager::IsSystemSleep()
{
    return powerStateChangeReason_ == PowerStateChangeReason::STATE_CHANGE_REASON_SYSTEM ||
        powerStateChangeReason_ == PowerStateChangeReason::STATE_CHANGE_REASON_HARD_KEY;
}

bool ScreenSessionManager::SuspendEnd()
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "[UL_POWER] enter");
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[UL_POWER]ssm:SuspendEnd");
    // 多屏协作灭屏不通知锁屏
    if (isMultiScreenCollaboration_) {
        isMultiScreenCollaboration_ = false;
        return true;
    }
    return NotifyDisplayPowerEvent(DisplayPowerEvent::SLEEP, EventStatus::END,
        PowerStateChangeReason::STATE_CHANGE_REASON_INIT);
}

ScreenId ScreenSessionManager::GetInternalScreenId()
{
    ScreenId screenId = SCREEN_ID_INVALID;
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::REAL && screenSession->isInternal_) {
            TLOGI(WmsLogTag::DMS, "found screenId = %{public}" PRIu64, sessionIt.first);
            screenId = sessionIt.first;
            break;
        }
    }
    return screenId;
}

sptr<ScreenSession> ScreenSessionManager::GetInternalScreenSession()
{
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        sptr<ScreenSession> screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::REAL && screenSession->isInternal_) {
            TLOGI(WmsLogTag::DMS, "found screenSession, Id = %{public}" PRIu64, sessionIt.first);
            return screenSession;
        }
    }
    return nullptr;
}

void ScreenSessionManager::GetInternalAndExternalSession(sptr<ScreenSession>& internalSession,
    sptr<ScreenSession>& externalSession)
{
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
            continue;
        }
        if (!screenSession->GetIsCurrentInUse()) {
            TLOGE(WmsLogTag::DMS, "screenSession not in use!");
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::REAL && screenSession->isInternal_) {
            TLOGI(WmsLogTag::DMS, "found internalSession, screenId = %{public}" PRIu64, sessionIt.first);
            internalSession = screenSession;
        } else {
            TLOGI(WmsLogTag::DMS, "found externalSession, screenId = %{public}" PRIu64, sessionIt.first);
            externalSession = screenSession;
        }
    }
}

bool ScreenSessionManager::SetScreenPowerById(ScreenId screenId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "screen id:%{public}" PRIu64
    ", state:%{public}u, reason:%{public}u", screenId, state, static_cast<uint32_t>(reason));

    bool isPowerSet = false;
    if (g_isPcDevice) {
        isPowerSet = SetScreenPowerByIdForPC(screenId, state);
    } else {
        isPowerSet = SetScreenPowerByIdDefault(screenId, state);
    }
    return isPowerSet;
}

bool ScreenSessionManager::SetScreenPowerByIdForPC(ScreenId screenId, ScreenPowerState state)
{
    std::lock_guard<std::mutex> lock(screenPowerMutex_);
    switch (state) {
        case ScreenPowerState::POWER_ON: {
            TLOGI(WmsLogTag::DMS, "[UL_POWER]Set ScreenPowerStatus: POWER_STATUS_ON");
#ifdef WM_MULTI_SCREEN_ENABLE
            if (GetIsOuterOnlyMode() && !GetIsOuterOnlyModeBeforePowerOff()) {
                MultiScreenModeChange(screenId, screenId, "on");
            }
#endif
            break;
        }
        case ScreenPowerState::POWER_OFF: {
            TLOGI(WmsLogTag::DMS, "[UL_POWER]Set ScreenPowerStatus: POWER_STATUS_OFF");
#ifdef WM_MULTI_SCREEN_ENABLE
            if (!GetIsOuterOnlyMode() && !GetIsOuterOnlyModeBeforePowerOff()) {
                MultiScreenModeChange(screenId, screenId, "off");
                SetIsOuterOnlyModeBeforePowerOff(false);
            }
#endif
            break;
        }
        default: {
            TLOGW(WmsLogTag::DMS, "[UL_POWER]SetScreenPowerById state not support");
            return false;
        }
    }
    return true;
}

bool ScreenSessionManager::SetScreenPowerByIdDefault(ScreenId screenId, ScreenPowerState state)
{
    ScreenPowerStatus status;
    switch (state) {
        case ScreenPowerState::POWER_ON: {
            status = ScreenPowerStatus::POWER_STATUS_ON;
            TLOGI(WmsLogTag::DMS, "[UL_POWER]Set ScreenPowerStatus: POWER_STATUS_ON");
            break;
        }
        case ScreenPowerState::POWER_OFF: {
            status = ScreenPowerStatus::POWER_STATUS_OFF;
            TLOGI(WmsLogTag::DMS, "[UL_POWER]Set ScreenPowerStatus: POWER_STATUS_OFF");
            break;
        }
        default: {
            TLOGW(WmsLogTag::DMS, "[UL_POWER]SetScreenPowerById state not support");
            return false;
        }
    }
    CallRsSetScreenPowerStatusSync(screenId, status);
    return true;
}

void ScreenSessionManager::SetLastScreenMode(sptr<ScreenSession> firstSession, sptr<ScreenSession> secondarySession)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    if (firstSession == nullptr || secondarySession == nullptr) {
        TLOGE(WmsLogTag::DMS, "first or second screen is null");
        return;
    }

    ScreenId mainScreenId = SCREEN_ID_INVALID;
    MultiScreenMode secondaryScreenMode = MultiScreenMode::SCREEN_MIRROR;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (auto sessionIt : screenSessionMap_) {
            auto screenSession = sessionIt.second;
            if (screenSession == nullptr) {
                TLOGW(WmsLogTag::DMS, "screenSession is nullptr!");
                continue;
            }
            if (screenSession != firstSession && screenSession != secondarySession) {
                continue;
            }
            if (!screenSession->GetIsCurrentInUse()) {
                TLOGE(WmsLogTag::DMS, "screenSession not in use!");
                continue;
            }
            ScreenCombination screenCombination = screenSession->GetScreenCombination();
            if (screenCombination == ScreenCombination::SCREEN_MAIN) {
                mainScreenId = sessionIt.first;
                TLOGI(WmsLogTag::DMS, "found main screen");
            } else if (screenCombination == ScreenCombination::SCREEN_MIRROR) {
                secondaryScreenMode = MultiScreenMode::SCREEN_MIRROR;
                TLOGI(WmsLogTag::DMS, "found mirror screen");
            } else if (screenCombination == ScreenCombination::SCREEN_EXTEND) {
                secondaryScreenMode = MultiScreenMode::SCREEN_EXTEND;
                TLOGI(WmsLogTag::DMS, "found extend screen");
            } else {
                TLOGE(WmsLogTag::DMS, "screen id or screen mode error");
            }
        }
    }

    if (mainScreenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "param error!");
        return;
    }
    MultiScreenManager::GetInstance().SetLastScreenMode(mainScreenId, secondaryScreenMode);
#endif
}

bool ScreenSessionManager::IsPreBrightAuthFail(void)
{
    return lastWakeUpReason_ == PowerStateChangeReason::
        STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
}

bool ScreenSessionManager::BlockSetDisplayState(void)
{
    return prePowerStateChangeReason_ == PowerStateChangeReason::POWER_BUTTON;
}

bool ScreenSessionManager::SetDisplayState(DisplayState state)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "[UL_POWER]state: %{public}d", state);
    if (!sessionDisplayPowerController_) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]sessionDisplayPowerController_ is null");
        return false;
    }
    if (ScreenStateMachine::GetInstance().GetTransitionState() == ScreenTransitionState::SCREEN_INIT) {
        return DoSetDisplayState(state);
    } else {
        ScreenPowerInfoType type = state;
        // mark has received aod notification during power callback.
        aodNotifyFlag_ = AodStatus::UNKNOWN;
        ScreenPowerEvent screenPowerEvent = ScreenPowerEvent::SET_DISPLAY_STATE;
        auto iter = POWER_STATE_CHANGE_MAP.find(state);
        if (iter != POWER_STATE_CHANGE_MAP.end()) {
            screenPowerEvent = iter->second;
        }
        bool ret = ScreenStateMachine::GetInstance().HandlePowerStateChange(screenPowerEvent, type);
        ScreenPowerStatus status = ScreenStateMachine::GetInstance().GetCurrentPowerStatus();
        if (status == ScreenPowerStatus::POWER_STATUS_SUSPEND) {
            ScreenStateMachine::GetInstance().HandlePowerStateChange(ScreenPowerEvent::DMS_POWER_CB_END, type);
            if (aodNotifyFlag_ == AodStatus::SUCCESS) {
                ret = ScreenStateMachine::GetInstance().HandlePowerStateChange(ScreenPowerEvent::AOD_ENTER_SUCCESS, type);
            } else if (aodNotifyFlag_ == AodStatus::FAILURE) {
                ret = ScreenStateMachine::GetInstance().HandlePowerStateChange(ScreenPowerEvent::AOD_ENTER_FAIL, type);
            }
        }
        return ret;
    }
}

bool ScreenSessionManager::DoSetDisplayState(DisplayState state)
{
    if (!sessionDisplayPowerController_) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]sessionDisplayPowerController_ is null");
        return false;
    }
    TLOGI(WmsLogTag::DMS, "[UL_POWER] enter");
    auto screenIds = GetAllScreenIds();
    if (screenIds.empty()) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]no screen info");
        return sessionDisplayPowerController_->SetDisplayState(state);
    }

    UpdateDisplayState(screenIds, state);
    bool ret = sessionDisplayPowerController_->SetDisplayState(state);
    if (!ret && state == DisplayState::OFF) {
        state = lastDisplayState_;
        UpdateDisplayState(screenIds, state);
    }
    lastDisplayState_ = state;
    return ret;
}

void ScreenSessionManager::UpdateDisplayState(std::vector<ScreenId> screenIds, DisplayState state)
{
    for (auto screenId : screenIds) {
        sptr<ScreenSession> screenSession = GetScreenSession(screenId);
        if (screenSession == nullptr) {
            TLOGW(WmsLogTag::DMS, "[UL_POWER]screenId: %{public}" PRIu64, screenId);
            continue;
        }
        screenSession->UpdateDisplayState(state);
        TLOGI(WmsLogTag::DMS, "[UL_POWER]displayState: %{public}u",
            screenSession->GetScreenProperty().GetDisplayState());
    }
}

void ScreenSessionManager::BlockScreenOnByCV(void)
{
    if (keyguardDrawnDone_ == false) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]screenOnCV_ set");
        needScreenOnWhenKeyguardNotify_ = true;
        std::unique_lock<std::mutex> lock(screenOnMutex_);
        if (DmUtils::safe_wait_for(screenOnCV_, lock, std::chrono::milliseconds(screenOnDelay_)) == std::cv_status::timeout) {
            TLOGI(WmsLogTag::DMS, "[UL_POWER]wait ScreenOnCV_ timeout");
        }
    }
}

void ScreenSessionManager::BlockScreenOffByCV(void)
{
    if (gotScreenOffNotify_ == false) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]delay:%{public}d", screenOffDelay_);
        needScreenOffNotify_ = true;
        std::unique_lock<std::mutex> lock(screenOffMutex_);
        if (DmUtils::safe_wait_for(screenOffCV_, lock, std::chrono::milliseconds(screenOffDelay_)) == std::cv_status::timeout) {
            isScreenLockSuspend_ = false;
            needScreenOffNotify_ = false;
            TLOGI(WmsLogTag::DMS, "[UL_POWER]wait ScreenOffCV_ timeout, isScreenLockSuspend_ is false");
        }
    }
}

bool ScreenSessionManager::TryToCancelScreenOff()
{
    std::lock_guard<std::mutex> notifyLock(sessionDisplayPowerController_->notifyMutex_);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission denied!");
        return false;
    }
    TLOGI(WmsLogTag::DMS, "[UL_POWER]about to cancel suspend, can:%{public}d, got:%{public}d, need:%{public}d",
        sessionDisplayPowerController_->canCancelSuspendNotify_, gotScreenOffNotify_, needScreenOffNotify_);
    if (sessionDisplayPowerController_->canCancelSuspendNotify_) {
        sessionDisplayPowerController_->needCancelNotify_ = true;
        TLOGI(WmsLogTag::DMS, "[UL_POWER]notify cancel screenoff");
        ScreenSessionManager::GetInstance().NotifyDisplayPowerEvent(DisplayPowerEvent::DISPLAY_OFF_CANCELED,
            EventStatus::BEGIN, PowerStateChangeReason::STATE_CHANGE_REASON_INIT);
        return true;
    }
    if (gotScreenOffNotify_ == false && needScreenOffNotify_ == true) {
        std::unique_lock <std::mutex> lock(screenOffMutex_);
        sessionDisplayPowerController_->canceledSuspend_ = true;
        screenOffCV_.notify_all();
        needScreenOffNotify_ = false;
        TLOGI(WmsLogTag::DMS, "[UL_POWER]cancel wait and notify cancel screenoff");
        ScreenSessionManager::GetInstance().NotifyDisplayPowerEvent(DisplayPowerEvent::DISPLAY_OFF_CANCELED,
            EventStatus::BEGIN, PowerStateChangeReason::STATE_CHANGE_REASON_INIT);
        return true;
    }
    TLOGW(WmsLogTag::DMS, "[UL_POWER]failed to cancel suspend");
    return false;
}

void ScreenSessionManager::ForceSkipScreenOffAnimation()
{
    std::lock_guard<std::mutex> notifyLock(sessionDisplayPowerController_->notifyMutex_);
    TLOGI(WmsLogTag::DMS, "[UL_POWER]about to skip animation, can:%{public}d, got:%{public}d, need:%{public}d",
        sessionDisplayPowerController_->canCancelSuspendNotify_, gotScreenOffNotify_, needScreenOffNotify_);
    if (sessionDisplayPowerController_->canCancelSuspendNotify_) {
        sessionDisplayPowerController_->skipScreenOffBlock_ = true;
        TLOGI(WmsLogTag::DMS, "[UL_POWER]skip screenoff animation");
        return;
    }
    if (gotScreenOffNotify_ == false && needScreenOffNotify_ == true) {
        std::unique_lock <std::mutex> lock(screenOffMutex_);
        screenOffCV_.notify_all();
        needScreenOffNotify_ = false;
        TLOGI(WmsLogTag::DMS, "[UL_POWER]skip wait");
        return;
    }
}

bool ScreenSessionManager::SetScreenBrightness(uint64_t screenId, uint32_t level)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "set screen brightness permission denied!");
        return false;
    }
    TLOGD(WmsLogTag::DMS, "screenId: %{public}" PRIu64", level: %{public}u", screenId, level);
    RSInterfaces::GetInstance().SetScreenBacklight(screenId, level);
    return true;
}

uint32_t ScreenSessionManager::GetScreenBrightness(uint64_t screenId)
{
    uint32_t level = static_cast<uint32_t>(RSInterfaces::GetInstance().GetScreenBacklight(screenId));
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64", level: %{public}u", screenId, level);
    return level;
}

int32_t ScreenSessionManager::SetScreenOffDelayTime(int32_t delay)
{
    DmsXcollie dmsXcollie("DMS:SetScreenOffDelayTime", XCOLLIE_TIMEOUT_10S);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "set screen off delay time permission denied!");
        return 0;
    }

    if (delay > SCREEN_OFF_MIN_DELAY_TIME && delay < CV_WAIT_SCREENOFF_MS) {
        screenOffDelay_ = CV_WAIT_SCREENOFF_MS;
    } else if (delay > CV_WAIT_SCREENOFF_MS_MAX) {
        screenOffDelay_ = CV_WAIT_SCREENOFF_MS_MAX;
    } else {
        screenOffDelay_ = delay;
    }
    TLOGI(WmsLogTag::DMS, "delay:%{public}d, screenOffDelay_:%{public}d",
        delay, screenOffDelay_);
    return screenOffDelay_;
}

int32_t ScreenSessionManager::SetScreenOnDelayTime(int32_t delay)
{
    DmsXcollie dmsXcollie("DMS:SetScreenOnDelayTime", XCOLLIE_TIMEOUT_10S);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "set screen on delay time permission denied!");
        return 0;
    }

    if (delay > CV_WAIT_SCREENON_MS) {
        screenOnDelay_ = CV_WAIT_SCREENON_MS;
    } else {
        screenOnDelay_ = delay;
    }
    TLOGI(WmsLogTag::DMS, "delay:%{public}d, screenOnDelay_:%{public}d",
        delay, screenOnDelay_);
    return screenOnDelay_;
}

void ScreenSessionManager::SetCameraStatus(int32_t cameraStatus, int32_t cameraPosition)
{
#ifdef WM_CAM_MODE_ABILITY_ENABLE
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "set camera status permission denied!");
        return;
    }

    if ((cameraStatus_ == cameraStatus) && (cameraPosition_ == cameraPosition)) {
        return;  // no need to update
    }
    cameraStatus_ = cameraStatus;
    cameraPosition_ = cameraPosition;
    TLOGI(WmsLogTag::DMS, "SetCameraStatus, cameraStatus:%{public}d, cameraPosition:%{public}d",
        cameraStatus, cameraPosition);
#endif
}

bool ScreenSessionManager::IsScreenLockSuspend(void)
{
    return isScreenLockSuspend_;
}

void ScreenSessionManager::NotifyDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type)
{
    auto clientProxy = GetClientProxy();
    if (clientProxy) {
        clientProxy->OnDisplayStateChanged(defaultDisplayId, displayInfo, displayInfoMap, type);
    }
}

void ScreenSessionManager::NotifyScreenshot(DisplayId displayId)
{
    auto clientProxy = GetClientProxy();
    if (clientProxy) {
        clientProxy->OnScreenshot(displayId);
    }
}

bool ScreenSessionManager::SetSpecifiedScreenPower(ScreenId screenId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "screen id:%{public}" PRIu64 ", state:%{public}u, reason:%{public}u",
        screenId, state, reason);

    ScreenPowerStatus status;
    switch (state) {
        case ScreenPowerState::POWER_ON: {
            status = ScreenPowerStatus::POWER_STATUS_ON;
            break;
        }
        case ScreenPowerState::POWER_OFF: {
            status = ScreenPowerStatus::POWER_STATUS_OFF;
            break;
        }
        default: {
            TLOGW(WmsLogTag::DMS, "[UL_POWER]SetScreenPowerStatus state not support");
            return false;
        }
    }

    powerStateChangeReason_ = reason;
    CallRsSetScreenPowerStatusSync(screenId, status);
    if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION) {
        return true;
    }
    return NotifyDisplayPowerEvent(state == ScreenPowerState::POWER_ON ? DisplayPowerEvent::DISPLAY_ON :
        DisplayPowerEvent::DISPLAY_OFF, EventStatus::END, reason);
}

bool ScreenSessionManager::DoSetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS, "[UL_POWER]state: %{public}u, reason: %{public}u",
        static_cast<uint32_t>(state), static_cast<uint32_t>(reason));  
    ScreenPowerStatus status;

    if (!GetPowerStatus(state, reason, status)) {
        return false;
    }
    if (g_isPcDevice && reason == PowerStateChangeReason::POWER_BUTTON && state == ScreenPowerState::POWER_OFF) {
        isDeviceShutDown_ = true;
    } else {
        isDeviceShutDown_ = false;
    }
    gotScreenOffNotify_  = false;
    keyguardDrawnDone_ = false;
    TLOGI(WmsLogTag::DMS, "keyguardDrawnDone_ is false");
    prePowerStateChangeReason_ = reason;
    return SetScreenPower(status, reason);
}

bool ScreenSessionManager::SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "[UL_POWER]state: %{public}u, reason: %{public}u",
        static_cast<uint32_t>(state), static_cast<uint32_t>(reason));
    ScreenTransitionState screenTransitionState = ScreenStateMachine::GetInstance().GetTransitionState();
    if (screenTransitionState == ScreenTransitionState::WAIT_SCREEN_ADVANCED_ON_READY){
        ScreenPowerInfoType type = std::make_pair(state,reason);
        if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS ||
        reason == PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON){
            return ScreenStateMachine::GetInstance().HandlePowerStateChange(
                ScreenPowerEvent::SET_SCREEN_POWER_FOR_ALL_POWER_ON, type);
        } else if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF){
            return ScreenStateMachine::GetInstance().HandlePowerStateChange(
                ScreenPowerEvent::SET_SCREEN_POWER_FOR_ALL_POWER_OFF, type);
        }
    }
    return DoSetScreenPowerForAll(state, reason);
}

bool ScreenSessionManager::GetPowerStatus(ScreenPowerState state, PowerStateChangeReason reason,
    ScreenPowerStatus& status)
{
    switch (state) {
        case ScreenPowerState::POWER_ON: {
            if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT) {
                // 预亮屏
                status = ScreenPowerStatus::POWER_STATUS_ON_ADVANCED;
                TLOGI(WmsLogTag::DMS, "[UL_POWER]POWER_STATUS_ON_ADVANCED");
            } else {
                status = ScreenPowerStatus::POWER_STATUS_ON;
                TLOGI(WmsLogTag::DMS, "[UL_POWER]POWER_STATUS_ON");
            }
            break;
        }
        case ScreenPowerState::POWER_OFF: {
            if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF) {
                // 预亮屏时指纹认证失败
                status = ScreenPowerStatus::POWER_STATUS_OFF_ADVANCED;
                TLOGI(WmsLogTag::DMS, "[UL_POWER]POWER_STATUS_OFF_ADVANCED");
            } else {
                status = ScreenPowerStatus::POWER_STATUS_OFF;
                TLOGI(WmsLogTag::DMS, "[UL_POWER]POWER_STATUS_OFF");
            }
            rsInterface_.MarkPowerOffNeedProcessOneFrame();
            break;
        }
        case ScreenPowerState::POWER_SUSPEND: {
            status = ScreenPowerStatus::POWER_STATUS_SUSPEND;
            TLOGI(WmsLogTag::DMS, "[UL_POWER] POWER_SUSPEND");
            rsInterface_.MarkPowerOffNeedProcessOneFrame();
            break;
        }
        case ScreenPowerState::POWER_DOZE: {
            status = ScreenPowerStatus::POWER_STATUS_DOZE;
            TLOGI(WmsLogTag::DMS, "[UL_POWER]Set ScreenPowerStatus: POWER_STATUS_DOZE");
            break;
        }
        case ScreenPowerState::POWER_DOZE_SUSPEND: {
            status = ScreenPowerStatus::POWER_STATUS_DOZE_SUSPEND;
            TLOGI(WmsLogTag::DMS, "[UL_POWER]Set ScreenPowerStatus: POWER_STATUS_DOZE_SUSPEND");
            break;
        }
        default: {
            TLOGW(WmsLogTag::DMS, "[UL_POWER]not support");
            return false;
        }
    }
    return true;
}

void ScreenSessionManager::ExitCoordination(const std::string& reason)
{
#ifdef FOLD_ABILITY_ENABLE
    if (GetFoldDisplayMode() != FoldDisplayMode::COORDINATION) {
        return;
    }
    if (foldScreenController_ != nullptr) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER] reason:%{public}s", reason.c_str());
        foldScreenController_->ExitCoordination();
    }
#endif
}

void ScreenSessionManager::TryToRecoverFoldDisplayMode(ScreenPowerStatus status)
{
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "foldScreenController_ is null");
        return;
    }
    if (status == ScreenPowerStatus::POWER_STATUS_OFF || status == ScreenPowerStatus::POWER_STATUS_OFF_ADVANCED ||
        status == ScreenPowerStatus::POWER_STATUS_OFF_FAKE || status == ScreenPowerStatus::POWER_STATUS_SUSPEND ||
        status == ScreenPowerStatus::POWER_STATUS_DOZE) {
        foldScreenController_->RecoverDisplayMode();
    }
#endif
}

bool ScreenSessionManager::SetScreenPower(ScreenPowerStatus status, PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS, "[UL_POWER] enter status:%{public}u, reason:%{public}u", status, reason);
    auto screenIds = GetAllScreenIds();
    if (screenIds.empty()) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER] screenIds empty");
        return false;
    }

    if (status == ScreenPowerStatus::POWER_STATUS_OFF || status == ScreenPowerStatus::POWER_STATUS_SUSPEND ||
        status == ScreenPowerStatus::POWER_STATUS_DOZE) {
        ExitCoordination("Press PowerKey");
    }
    DisplayPowerEvent notifyEvent = DisplayPowerEvent::DISPLAY_OFF;
    auto iter = SCREEN_STATUS_POWER_EVENT_MAP.find(status);
    if (iter != SCREEN_STATUS_POWER_EVENT_MAP.end()) {
        notifyEvent = iter->second;
    }
    if (((status == ScreenPowerStatus::POWER_STATUS_OFF || status == ScreenPowerStatus::POWER_STATUS_SUSPEND ||
        status == ScreenPowerStatus::POWER_STATUS_DOZE) &&
        gotScreenlockFingerprint_ == true) &&
        prePowerStateChangeReason_ != PowerStateChangeReason::STATE_CHANGE_REASON_SHUT_DOWN) {
        gotScreenlockFingerprint_ = false;
        TLOGI(WmsLogTag::DMS, "[UL_POWER] screenlockFingerprint or shutdown");
        return NotifyDisplayPowerEvent(notifyEvent, EventStatus::END, reason);
    }
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ != nullptr) {
        CallRsSetScreenPowerStatusSyncForFold(status);
        CallRsSetScreenPowerStatusSyncForExtend(screenIds, status);
        TryToRecoverFoldDisplayMode(status);
    } else {
        SetRsSetScreenPowerStatusSync(screenIds, status);
    }
#else
    SetRsSetScreenPowerStatusSync(screenIds, status);
#endif
    HandlerSensor(status, reason);
    if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION) {
        return true;
    }
    if ((status == ScreenPowerStatus::POWER_STATUS_OFF || status == ScreenPowerStatus::POWER_STATUS_SUSPEND ||
        status == ScreenPowerStatus::POWER_STATUS_DOZE) &&
        gotScreenlockFingerprint_ == true) {
        gotScreenlockFingerprint_ = false;
    }

    return NotifyDisplayPowerEvent(notifyEvent, EventStatus::END, reason);
}

void ScreenSessionManager::SetRsSetScreenPowerStatusSync(std::vector<ScreenId>& screenIds,
    ScreenPowerStatus status)
{
    if (g_isPcDevice && status == ScreenPowerStatus::POWER_STATUS_ON) {
        std::sort(screenIds.begin(), screenIds.end(), SortByScreenId);
        for (auto screenId : screenIds) {
            TLOGI(WmsLogTag::DMS, "[UL_POWER] Power on, screen id is %{public}d", (int)screenId);
            if (GetIsOuterOnlyMode() == true && screenId == SCREEN_ID_FULL) {
                TLOGI(WmsLogTag::DMS, "Power on skip");
                continue;
            }
            CallRsSetScreenPowerStatusSync(screenId, status);
        }
    } else if (g_isPcDevice && (status == ScreenPowerStatus::POWER_STATUS_OFF ||
        status == ScreenPowerStatus::POWER_STATUS_SUSPEND)) {
            std::sort(screenIds.begin(), screenIds.end(), SortByScreenId);
            std::reverse(screenIds.begin(), screenIds.end());
            for (auto screenId : screenIds) {
                TLOGI(WmsLogTag::DMS, "[UL_POWER] Power off, screen id is %{public}d", (int)screenId);
                CallRsSetScreenPowerStatusSync(screenId, status);
            }
    } else {
        for (auto screenId : screenIds) {
            CallRsSetScreenPowerStatusSync(screenId, status);
        }
    }
}

void ScreenSessionManager::CallRsSetScreenPowerStatusSyncForExtend(const std::vector<ScreenId>& screenIds,
    ScreenPowerStatus status)
{
    for (auto screenId : screenIds) {
        auto session = GetScreenSession(screenId);
        if (session && session->GetScreenProperty().GetScreenType() == ScreenType::REAL && !session->isInternal_) {
            CallRsSetScreenPowerStatusSync(screenId, status);
        }
    }
}

ScreenPowerEvent ScreenSessionManager::ConvertScreenStateEvent(ScreenPowerStatus status)
{
    switch (status) {
        case ScreenPowerStatus::POWER_STATUS_OFF:
        case ScreenPowerStatus::POWER_STATUS_OFF_FAKE:
            return ScreenPowerEvent::POWER_OFF;
        case ScreenPowerStatus::POWER_STATUS_ON:
            return ScreenPowerEvent::POWER_ON;
        case ScreenPowerStatus::POWER_STATUS_SUSPEND:
            return ScreenPowerEvent::SUSPEND;
        case ScreenPowerStatus::POWER_STATUS_ON_ADVANCED:
            return ScreenPowerEvent::E_ADVANCED_ON;
        case ScreenPowerStatus::POWER_STATUS_OFF_ADVANCED:
            return ScreenPowerEvent::E_ADVANCED_OFF;
        case ScreenPowerStatus::POWER_STATUS_DOZE:
            return ScreenPowerEvent::E_DOZE;
        case ScreenPowerStatus::POWER_STATUS_DOZE_SUSPEND:
            return ScreenPowerEvent::E_DOZE_SUSPEND;
        default:
            return ScreenPowerEvent::POWER_ON;
    }
}

ScreenTransitionState ScreenSessionManager::ConvertPowerStatus2ScreenState(ScreenPowerStatus status)
{
    switch (status) {
        case ScreenPowerStatus::POWER_STATUS_OFF:
        case ScreenPowerStatus::POWER_STATUS_OFF_FAKE:
        case ScreenPowerStatus::POWER_STATUS_OFF_ADVANCED:
            return ScreenTransitionState::SCREEN_OFF;
        case ScreenPowerStatus::POWER_STATUS_ON:
            return ScreenTransitionState::SCREEN_ON;
        case ScreenPowerStatus::POWER_STATUS_SUSPEND:
            return ScreenTransitionState::WAIT_SCREEN_CTRL_RSP;
        case ScreenPowerStatus::POWER_STATUS_ON_ADVANCED:
            return ScreenTransitionState::SCREEN_ADVANCED_ON;
        case ScreenPowerStatus::POWER_STATUS_DOZE:
            return ScreenTransitionState::SCREEN_DOZE;
        case ScreenPowerStatus::POWER_STATUS_DOZE_SUSPEND:
            return ScreenTransitionState::SCREEN_DOZE_SUSPEND;
        default:
            return ScreenTransitionState::SCREEN_ON;
    }
}

#ifdef FOLD_ABILITY_ENABLE
void ScreenSessionManager::SetScreenPowerForFold(ScreenPowerStatus status)
{
    if (foldScreenController_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "foldScreenController_ is null");
        return;
    }
    SetScreenPowerForFold(foldScreenController_->GetCurrentScreenId(), status);
}

void ScreenSessionManager::SetScreenPowerForFold(ScreenId screenId, ScreenPowerStatus status)
{
    if (ScreenStateMachine::GetInstance().GetTransitionState() == ScreenTransitionState::SCREEN_INIT) {
        SetRSScreenPowerStatusExt(screenId, status);
    } else if (status == ScreenPowerStatus::POWER_STATUS_OFF) {
        SetRSScreenPowerStatus(screenId, status, ScreenPowerEvent::POWER_OFF_DIRECTLY);
    } else if (status == ScreenPowerStatus::POWER_STATUS_ON) {
        SetRSScreenPowerStatus(screenId, status, ScreenPowerEvent::POWER_ON_DIRECTLY);
    }
}

void ScreenSessionManager::TriggerDisplayModeUpdate(FoldDisplayMode targetDisplayMode)
{
    auto updateDisplayModeTask = [=] {
        TLOGNI(WmsLogTag::DMS, "start change displaymode to lastest mode");
        foldScreenController_->SetDisplayMode(targetDisplayMode);
    };
    taskScheduler_->PostAsyncTask(updateDisplayModeTask, "updateDisplayModeTask");
}
#endif
void ScreenSessionManager::CallRsSetScreenPowerStatusSync(ScreenId screenId, ScreenPowerStatus status)
{
    auto rsSetScreenPowerStatusTask = [=] {
        bool phyMirrorEnable = IsDefaultMirrorMode(screenId);
        if (phyMirrorEnable && !g_isPcDevice) {
            auto screenSession = GetScreenSession(screenId);
            if (screenSession == nullptr) {
                return;
            }
            auto sourceMode = screenSession->GetSourceMode();
            if (status != ScreenPowerStatus::POWER_STATUS_ON &&
                (powerStateChangeReason_ == PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION ||
                powerStateChangeReason_ == PowerStateChangeReason::STATE_CHANGE_REASON_APPCAST)) {
                return;
            }
            if (sourceMode == ScreenSourceMode::SCREEN_UNIQUE &&
                status != ScreenPowerStatus::POWER_STATUS_ON &&
                powerStateChangeReason_ == PowerStateChangeReason::STATE_CHANGE_REASON_HARD_KEY) {
                return;
            }
        }
        auto transState = ConvertPowerStatus2ScreenState(status);
        SetRSScreenPowerStatusExt(screenId, status);
        if (ScreenStateMachine::GetInstance().GetTransitionState() != ScreenTransitionState::SCREEN_INIT) {
            ScreenStateMachine::GetInstance().ToTransition(transState, false);
        }
    };
    screenPowerTaskScheduler_->PostVoidSyncTask(rsSetScreenPowerStatusTask, "rsInterface_.SetScreenPowerStatus task");
#ifdef WM_MULTI_SCREEN_ENABLE
    if (g_isPcDevice && status == ScreenPowerStatus::POWER_STATUS_ON && IsDefaultMirrorMode(screenId)) {
        TLOGI(WmsLogTag::DMS, "set screen active mode, rsScreenId:%{public}" PRId64, screenId);
        RecoverScreenActiveMode(screenId);
        RecoverMultiScreenRelativePosition(screenId);
    }
#endif
}

void ScreenSessionManager::RecoverMultiScreenRelativePosition(ScreenId screenId)
{
    sptr<ScreenSession> screenSession = GetScreenSessionByRsId(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
        return;
    }
    std::map<std::string, MultiScreenInfo> multiScreenInfoMap = ScreenSettingHelper::GetMultiScreenInfo();
    std::string serialNumber = screenSession->GetSerialNumber();
    if (!CheckMultiScreenInfoMap(multiScreenInfoMap, serialNumber)) {
        return;
    }
    auto info = multiScreenInfoMap[serialNumber];
    sptr<ScreenSession> internalSession = GetInternalScreenSession();
    if(internalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "internalSession is nullptr!");
        return;
    }
    if(info.isExtendMain) {
        TLOGI(WmsLogTag::DMS, "extend screen is main");
        info.mainScreenOption.screenId_ = screenSession->GetRSScreenId();
        info.secondaryScreenOption.screenId_ = internalSession->GetRSScreenId();
    } else {
        TLOGI(WmsLogTag::DMS, "extend screen is not main");
        info.mainScreenOption.screenId_ = internalSession->GetRSScreenId();
        info.secondaryScreenOption.screenId_ = screenSession->GetRSScreenId();
    }
    auto ret = SetMultiScreenRelativePosition(info.mainScreenOption, info.secondaryScreenOption);
    if (ret != DMError::DM_OK) {
        SetMultiScreenDefaultRelativePosition();
    }
}

void ScreenSessionManager::CallRsSetScreenPowerStatusSyncForFold(ScreenPowerStatus status)
{
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ == nullptr) {
        TLOGNW(WmsLogTag::DMS, "foldScreenController_ is null");
        return;
    }
    ScreenId screenId = foldScreenController_->GetCurrentScreenId();
    lastPowerForAllStatus_.store(status);
    lastScreenId_.store(screenId);
    auto transState = ConvertPowerStatus2ScreenState(status);
    SetRSScreenPowerStatusExt(screenId, status);
    if (ScreenStateMachine::GetInstance().GetTransitionState() != ScreenTransitionState::SCREEN_INIT) {
        ScreenStateMachine::GetInstance().ToTransition(transState, false);
    }
#endif
}

void ScreenSessionManager::SetKeyguardDrawnDoneFlag(bool flag)
{
    keyguardDrawnDone_ = flag;
}

void ScreenSessionManager::HandlerSensor(ScreenPowerStatus status, PowerStateChangeReason reason)
{
    if (!ScreenSceneConfig::IsSupportRotateWithSensor()) {
        TLOGW(WmsLogTag::DMS, "not supportRotateWithSensor.");
        return;
    }
    if (status == ScreenPowerStatus::POWER_STATUS_ON) {
        DmsXcollie dmsXcollie("DMS:SubscribeRotationSensor", XCOLLIE_TIMEOUT_10S);
        TLOGI(WmsLogTag::DMS, "subscribe rotation and posture sensor when phone turn on");
        ScreenSensorConnector::SubscribeRotationSensor();
#if defined(SENSOR_ENABLE) && defined(FOLD_ABILITY_ENABLE)
        if (g_foldScreenFlag && reason != PowerStateChangeReason::STATE_CHANGE_REASON_DISPLAY_SWITCH) {
            DMS::ScreenSensorMgr::GetInstance().RegisterPostureCallback();
            if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
                SecondaryFoldSensorManager::GetInstance().PowerKeySetScreenActiveRect();
            }
        } else {
            TLOGI(WmsLogTag::DMS, "not fold product, switch screen reason, failed register posture.");
        }
#endif
    } else if (status == ScreenPowerStatus::POWER_STATUS_OFF || status == ScreenPowerStatus::POWER_STATUS_SUSPEND ||
        status == ScreenPowerStatus::POWER_STATUS_DOZE || status == ScreenPowerStatus::POWER_STATUS_DOZE_SUSPEND) {
        UnregisterInHandlerSensorWithPowerOff(reason);
    } else {
        TLOGI(WmsLogTag::DMS, "SetScreenPower state not support");
        screenEventTracker_.RecordEvent("HandlerSensor start!");
    }
}

void ScreenSessionManager::UnregisterInHandlerSensorWithPowerOff(PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS, "unsubscribe sensor when off");
    if (isMultiScreenCollaboration_) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]MultiScreenCollaboration, not unsubscribe rotation sensor");
    } else {
        DmsXcollie dmsXcollie("DMS:UnsubscribeRotationSensor", XCOLLIE_TIMEOUT_10S);
        ScreenSensorConnector::UnsubscribeRotationSensor();
    }
#if defined(SENSOR_ENABLE) && defined(FOLD_ABILITY_ENABLE)
    if (g_foldScreenFlag && reason != PowerStateChangeReason::STATE_CHANGE_REASON_DISPLAY_SWITCH &&
        !FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        DMS::ScreenSensorMgr::GetInstance().UnRegisterPostureCallback();
    } else {
        TLOGI(WmsLogTag::DMS, "not fold product, failed unregister posture.");
    }
#endif
}

void ScreenSessionManager::BootFinishedCallback(const char *key, const char *value, void *context)
{
    if (strcmp(key, BOOTEVENT_BOOT_COMPLETED.c_str()) == 0 && strcmp(value, "true") == 0) {
        TLOGI(WmsLogTag::DMS, "boot animation finished");
        auto &that = *reinterpret_cast<ScreenSessionManager *>(context);
        that.SetRotateLockedFromSettingData();
        that.SetDpiFromSettingData();
        that.SetExtendScreenDpi();
        that.UpdateDisplayState(that.GetAllScreenIds(), DisplayState::ON);
        that.RegisterSettingDpiObserver();
        that.RegisterSettingExtendScreenDpiObserver();
        that.RegisterRotationCorrectionExemptionListObserver();
        if (FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
            that.RegisterSettingDuringCallStateObserver();
        }
        if (that.foldScreenPowerInit_ != nullptr) {
            that.foldScreenPowerInit_();
        }
        that.RegisterSettingRotationObserver();
        that.RegisterSettingResolutionEffectObserver();
        if (that.defaultDpi) {
            auto ret = ScreenSettingHelper::SetSettingDefaultDpi(that.defaultDpi, SET_SETTING_DPI_KEY);
            if (!ret) {
                TLOGE(WmsLogTag::DMS, "set setting defaultDpi failed");
            } else {
                TLOGI(WmsLogTag::DMS, "set setting defaultDpi:%{public}d", that.defaultDpi);
            }
        }
#ifdef FOLD_ABILITY_ENABLE
        if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
            auto screenSession = that.GetDefaultScreenSession();
            if (screenSession == nullptr) {
                TLOGE(WmsLogTag::DMS, "screen session is null!");
                return;
            }
            ScreenId screenId = screenSession->GetScreenId();
            SuperFoldStatus status = SuperFoldStateManager::GetInstance().GetCurrentStatus();
            TLOGI(WmsLogTag::DMS, "SuperFoldStatus: %{public}u", status);
            that.OnSuperFoldStatusChange(screenId, status);
            float sensorRotation = screenSession->GetSensorRotation();
            TLOGI(WmsLogTag::DMS, "sensorRotation: %{public}f", sensorRotation);
            if (sensorRotation >= 0.0f) {
                that.OnSensorRotationChange(sensorRotation, screenId, false);
            }
            screenSession->PropertyChange(screenSession->GetScreenProperty(),
                ScreenPropertyChangeReason::SUPER_FOLD_STATUS_CHANGE);
        }
#endif
    }
}

void ScreenSessionManager::SetFoldScreenPowerInit(std::function<void()> foldScreenPowerInit)
{
    foldScreenPowerInit_ = foldScreenPowerInit;
}

void ScreenSessionManager::SetRotateLockedFromSettingData()
{
    uint32_t autoRotateStatus = AUTO_ROTATE_OFF;  // 0代表自动旋转关闭,1代表自动旋转打开
    bool islocked = true;
    // ret为true表示从数据库读到了值，并赋给了autoRotateStatus
    bool ret = ScreenSettingHelper::GetSettingValue(autoRotateStatus, SETTING_LOCKED_KEY);
    TLOGI(WmsLogTag::DMS, "get autoRotateStatus from settingdata: %{public}u", autoRotateStatus);
    if (autoRotateStatus) {
        islocked =false;
    }
    if (ret) {
        TLOGI(WmsLogTag::DMS, "get islocked success");
        SetScreenRotationLockedFromJs(islocked);
    }
}

void ScreenSessionManager::RegisterSettingResolutionEffectObserver()
{
    TLOGI(WmsLogTag::DMS, "Register Setting Observer");
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) { SetResolutionEffectFromSettingData(); };
    ScreenSettingHelper::RegisterSettingResolutionEffectObserver(updateFunc);
}

void ScreenSessionManager::SetResolutionEffectFromSettingData()
{
    TLOGI(WmsLogTag::DMS, "update ResolutionEffect enable");
    HandleResolutionEffectChange();
}

void ScreenSessionManager::RegisterSettingDpiObserver()
{
    TLOGI(WmsLogTag::DMS, "Register Setting Dpi Observer");
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) { SetDpiFromSettingData(); };
    ScreenSettingHelper::RegisterSettingDpiObserver(DmUtils::wrap_callback(updateFunc));
}

void ScreenSessionManager::SetDpiFromSettingData()
{
    uint32_t settingDpi;
    bool ret = ScreenSettingHelper::GetSettingDpi(settingDpi);
    if (!ret) {
        settingDpi = defaultDpi;
        TLOGW(WmsLogTag::DMS, "get setting dpi failed,use default dpi,defaultDpi: %{public}u", settingDpi);
    } else {
        TLOGI(WmsLogTag::DMS, "get setting dpi success,settingDpi: %{public}u", settingDpi);
    }
    if (settingDpi >= DOT_PER_INCH_MINIMUM_VALUE && settingDpi <= DOT_PER_INCH_MAXIMUM_VALUE
        && cachedSettingDpi_ != settingDpi) {
        cachedSettingDpi_ = settingDpi;
        float dpi = static_cast<float>(settingDpi) / BASELINE_DENSITY;
        ScreenId defaultScreenId = GetDefaultScreenId();
            if (g_isPcDevice) {
                ScreenId screenId = screenIdManager_.ConvertToSmsScreenId(RS_ID_DEFAULT);
                if (screenId != SCREEN_ID_INVALID) {
                    TLOGI(WmsLogTag::DMS, "get ScreenId:%{public}" PRIu64" for rsId successful", screenId);
                    defaultScreenId = screenId;
                }
            }
            SetVirtualPixelRatio(defaultScreenId, dpi);
            if (g_isPcDevice) {
                SetExtendPixelRatio(dpi * g_extendScreenDpiCoef);
            }
        } else {
            TLOGE(WmsLogTag::DMS, "setting dpi error, settingDpi: %{public}d", settingDpi);
        }
}

void ScreenSessionManager::SetExtendPixelRatio(const float& dpi)
{
    auto screenIds = GetAllScreenIds();
    if (screenIds.empty()) {
        TLOGE(WmsLogTag::DMS, "no screenId");
        return;
    }
    for (auto screenId : screenIds) {
        auto screenSession = GetScreenSession(screenId);
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screensession is nullptr, screenId: %{public}" PRIu64"", screenId);
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::REAL && !screenSession->isInternal_) {
            SetVirtualPixelRatio(screenId, dpi);
        }
    }
}

std::vector<ScreenId> ScreenSessionManager::GetAllScreenIds()
{
    std::vector<ScreenId> res;
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (const auto& iter : screenSessionMap_) {
        res.emplace_back(iter.first);
    }
    return res;
}

DMError ScreenSessionManager::GetPhysicalScreenIds(std::vector<ScreenId>& screenIds)
{
    TLOGI(WmsLogTag::DMS, "enter");
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (const auto& iter : screenSessionMap_) {
        auto screenSession = iter.second;
        auto screenId = iter.first;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screensession is nullptr, screenId: %{public}" PRIu64, screenId);
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::REAL
            && screenId != SCREEN_ID_INVALID) {
            screenIds.emplace_back(screenId);
        }
    }
    return DMError::DM_OK;
}

DisplayState ScreenSessionManager::GetDisplayState(DisplayId displayId)
{
    return sessionDisplayPowerController_->GetDisplayState(displayId);
}

void ScreenSessionManager::NotifyDisplayEvent(DisplayEvent event)
{
    TLOGI(WmsLogTag::DMS, "[UL_POWER] receive keyguardDrawnDone");
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    sessionDisplayPowerController_->NotifyDisplayEvent(event);
    if (event == DisplayEvent::KEYGUARD_DRAWN) {
        keyguardDrawnDone_ = true;
        TLOGI(WmsLogTag::DMS, "[UL_POWER]keyguardDrawnDone_ is true");
        if (needScreenOnWhenKeyguardNotify_) {
            std::unique_lock <std::mutex> lock(screenOnMutex_);
            screenOnCV_.notify_all();
            TLOGI(WmsLogTag::DMS, "[UL_POWER]screenOnCV_ notify one");
            needScreenOnWhenKeyguardNotify_ = false;
        }
    }
    if (event == DisplayEvent::SCREEN_LOCK_SUSPEND) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]screen lock suspend");
        if (isPhyScreenConnected_) {
            isScreenLockSuspend_ = false;
            TLOGI(WmsLogTag::DMS, "[UL_POWER]isScreenLockSuspend__  is false");
        } else {
            isScreenLockSuspend_ = true;
        }
        SetGotScreenOffAndWakeUpBlock();
    }
    if (event == DisplayEvent::SCREEN_LOCK_OFF) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]screen lock off");
        isScreenLockSuspend_ = false;
        TLOGI(WmsLogTag::DMS, "[UL_POWER]isScreenLockSuspend__  is false");
        SetGotScreenOffAndWakeUpBlock();
    }
    if (event == DisplayEvent::SCREEN_LOCK_FINGERPRINT) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]screen lock fingerprint");
        gotScreenlockFingerprint_ = true;
        SetGotScreenOffAndWakeUpBlock();
    }
    if (event == DisplayEvent::SCREEN_LOCK_DOZE_FINISH) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]screen lock doze finish");
        SetGotScreenOffAndWakeUpBlock();
    }
    WakeUpPictureFrameBlock(event);
}

void ScreenSessionManager::SetGotScreenOffAndWakeUpBlock()
{
    gotScreenOffNotify_ = true;
    if (needScreenOffNotify_) {
        ScreenOffCVNotify();
    }
}

void ScreenSessionManager::ScreenOffCVNotify(void)
{
    std::unique_lock <std::mutex> lock(screenOffMutex_);
    screenOffCV_.notify_all();
    needScreenOffNotify_ = false;
    TLOGI(WmsLogTag::DMS, "[UL_POWER]screenOffCV_ notify one");
}

ScreenPowerState ScreenSessionManager::GetScreenPower(ScreenId screenId)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return ScreenPowerState::INVALID_STATE;
    }
    ScreenId rsScreenId = screenId;
    if (IsConcurrentUser()) {
        if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
            TLOGE(WmsLogTag::DMS, "convert rsScreenId failed, screenId: %{public}" PRIu64"", screenId);
            rsScreenId = screenId;
        }
    }
    auto state = static_cast<ScreenPowerState>(RSInterfaces::GetInstance().GetScreenPowerStatus(rsScreenId));
    std::ostringstream oss;
    oss << "GetScreenPower state:" << static_cast<uint32_t>(state) << " screenId:" << static_cast<uint64_t>(screenId);
    TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    screenEventTracker_.RecordEvent(oss.str());
    return state;
}

ScreenPowerState ScreenSessionManager::GetScreenPower()
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return ScreenPowerState::INVALID_STATE;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetScreenPower");
    ScreenPowerState state = ScreenPowerState::INVALID_STATE;
#ifdef FOLD_ABILITY_ENABLE
    if (!g_foldScreenFlag || FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        state = static_cast<ScreenPowerState>(RSInterfaces::GetInstance()
            .GetScreenPowerStatus(GetDefaultScreenId()));
    } else {
        state = static_cast<ScreenPowerState>(RSInterfaces::GetInstance()
            .GetScreenPowerStatus(foldScreenController_->GetCurrentScreenId()));
    }
#else
    state = static_cast<ScreenPowerState>(RSInterfaces::GetInstance()
            .GetScreenPowerStatus(foldScreenController_->GetCurrentScreenId()));
#endif
    std::ostringstream oss;
    oss << "GetScreenPower state:" << static_cast<uint32_t>(state);
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    screenEventTracker_.RecordEvent(oss.str());
    return state;
}

void ScreenSessionManager::SyncScreenPowerState(ScreenPowerState state)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    if (state == ScreenPowerState::POWER_ON) {
        ScreenStateMachine::GetInstance().HandlePowerStateChange(
            ScreenPowerEvent::SYNC_POWER_ON, PowerStateChangeReason::STATE_CHANGE_REASON_UNKNOWN);
    }
    TLOGI(WmsLogTag::DMS, "force sync power state: %{public}d", static_cast<int>(state));
}

DMError ScreenSessionManager::IsScreenRotationLocked(bool& isLocked)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    sptr<ScreenSession> screenSession = GetDefaultScreenSession();
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail to get default screenSession");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    isLocked = screenSession->IsScreenRotationLocked();
    TLOGI(WmsLogTag::DMS, "isLocked: %{public}u", isLocked);
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetScreenRotationLocked(bool isLocked)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    sptr<ScreenSession> screenSession = GetDefaultScreenSession();
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail to get default screenSession");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    screenSession->SetScreenRotationLocked(isLocked);
    TLOGI(WmsLogTag::DMS, "isLocked: %{public}u", isLocked);
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetScreenRotationLockedFromJs(bool isLocked)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    sptr<ScreenSession> screenSession = GetDefaultScreenSession();
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail to get default screenSession");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    screenSession->SetScreenRotationLockedFromJs(isLocked);
    TLOGI(WmsLogTag::DMS, "isLocked: %{public}u", isLocked);
    return DMError::DM_OK;
}

void ScreenSessionManager::NotifyAndPublishEvent(sptr<DisplayInfo> displayInfo, ScreenId screenId,
    sptr<ScreenSession> screenSession)
{
    if (displayInfo == nullptr || screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "error, displayInfo or screenSession is nullptr");
        return;
    }
    NotifyDisplayChanged(displayInfo, DisplayChangeEvent::UPDATE_ROTATION);
    NotifyScreenChanged(screenSession->ConvertToScreenInfo(), ScreenChangeEvent::UPDATE_ROTATION);
    UpdateDisplayScaleState(screenId);
    std::map<DisplayId, sptr<DisplayInfo>> emptyMap;
    NotifyDisplayStateChange(GetDefaultScreenId(), screenSession->ConvertToDisplayInfo(),
        emptyMap, DisplayStateChangeType::UPDATE_ROTATION);
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    ScreenSessionPublish::GetInstance().PublishDisplayRotationEvent(
        displayInfo->GetScreenId(), displayInfo->GetRotation());
    IPCSkeleton::SetCallingIdentity(identity);
}

void ScreenSessionManager::UpdateScreenDirectionInfo(ScreenId screenId, const ScreenDirectionInfo& directionInfo,
    ScreenPropertyChangeType screenPropertyChangeType, const RRect& bounds)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    if (screenPropertyChangeType == ScreenPropertyChangeType::ROTATION_END) {
        TLOGI(WmsLogTag::DMS, "ROTATION_END");
        return;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail, cannot find screen %{public}" PRIu64"",
            screenId);
        return;
    }
    float screenComponentRotation = directionInfo.screenRotation_;
    float rotation = directionInfo.rotation_;
    float phyRotation = directionInfo.phyRotation_;

    // Rotation update flow: server → client → server, involving two state changes:
    // 1st change: triggered on server when displayMode is modified;
    // 2nd change: triggered by client after completing the displayMode transition;
    // Due to unkown reason, the response to the 1st change may arrive after the 2nd,
    // causing rotation state inconsistency. A locking mechanism is introduced here to enforce sequential updates.
    std::lock_guard<std::recursive_mutex> lock_info(displayInfoMutex_);
    screenSession->SetPhysicalRotation(phyRotation);
    screenSession->SetScreenComponentRotation(screenComponentRotation);
    screenSession->UpdateRotationOrientation(rotation, GetFoldDisplayMode(), bounds);
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 ", rotation: %{public}f, screenComponentRotation: %{public}f",
        screenId, rotation, screenComponentRotation);
}

void ScreenSessionManager::UpdateScreenRotationProperty(ScreenId screenId, const RRect& bounds, float rotation,
    ScreenPropertyChangeType screenPropertyChangeType, bool isSwitchUser)
{
    std::ostringstream oss;
    std::string changeType = TransferPropertyChangeTypeToString(screenPropertyChangeType);
    oss << "screenId: " << screenId << " rotation: " << rotation << " width: " << bounds.rect_.width_ \
        << " height: " << bounds.rect_.height_ << " type: " << changeType;
    screenEventTracker_.RecordBoundsEvent(oss.str());
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail, cannot find screen %{public}" PRIu64"", screenId);
        return;
    }
    UpdateScreenRotationPropertyForRs(screenSession, screenPropertyChangeType, bounds, rotation, isSwitchUser);
    {
        std::lock_guard<std::recursive_mutex> lock_info(displayInfoMutex_);
        screenSession->UpdatePropertyAfterRotation(bounds, rotation, GetFoldDisplayMode());
    }
    if (g_isPcDevice) {
        sptr<ScreenSession> physicalScreen = GetPhysicalScreenSession(screenSession->GetRSScreenId());
        if (physicalScreen) {
            physicalScreen->UpdatePropertyAfterRotation(bounds, rotation, GetFoldDisplayMode());
        }
        NotifyScreenModeChange();
    }
    SetFoldDisplayModeAfterRotation(GetFoldDisplayMode());
    SetFirstSCBConnect(false);
    sptr<DisplayInfo> displayInfo = screenSession->ConvertToDisplayInfo();
    NotifyAndPublishEvent(displayInfo, screenId, screenSession);
}

void ScreenSessionManager::UpdateScreenRotationPropertyForRs(sptr<ScreenSession>& screenSession,
    ScreenPropertyChangeType screenPropertyChangeType, const RRect& bounds, float rotation, bool isSwitchUser)
{
    DmsXcollie dmsXcollie("DMS:UpdateScreenRotationProperty:CacheForRotation", XCOLLIE_TIMEOUT_10S);
    if (screenPropertyChangeType == ScreenPropertyChangeType::ROTATION_BEGIN) {
        // Rs is used to mark the start of the rotation animation
        TLOGI(WmsLogTag::DMS, "EnableCacheForRotation");
        RSInterfaces::GetInstance().EnableCacheForRotation();
    } else if (screenPropertyChangeType == ScreenPropertyChangeType::ROTATION_END) {
        // Rs is used to mark the end of the rotation animation
        TLOGI(WmsLogTag::DMS, "DisableCacheForRotation");
        RSInterfaces::GetInstance().DisableCacheForRotation();
        HandleResolutionEffectChangeWhenRotate();
        return;
    } else if (screenPropertyChangeType == ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY ||
        screenPropertyChangeType == ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY_NOT_NOTIFY) {
        sptr<DisplayInfo> displayInfo = screenSession->ConvertToDisplayInfo();
        TLOGI(WmsLogTag::DMS, "Update Screen Rotation Property Only");
        {
            std::lock_guard<std::recursive_mutex> lock_info(displayInfoMutex_);
            screenSession->UpdatePropertyOnly(bounds, rotation, GetFoldDisplayMode());
        }
        if (screenPropertyChangeType != ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY_NOT_NOTIFY) {
            NotifyDisplayChanged(displayInfo, DisplayChangeEvent::UPDATE_ROTATION);
            NotifyScreenChanged(screenSession->ConvertToScreenInfo(), ScreenChangeEvent::UPDATE_ROTATION);
        }
        if(!isSwitchUser) {
            screenSession->UpdateDisplayNodeRotation(rotation);
        }
        return;
    }
}

void ScreenSessionManager::NotifyDisplayChanged(sptr<DisplayInfo> displayInfo, DisplayChangeEvent event)
{
    auto task = [=] {
        ScreenSessionManagerAdapter::GetInstance().OnDisplayChange(displayInfo, event);
    };
    taskScheduler_->PostAsyncTask(task, "NotifyDisplayChanged");
}

DMError ScreenSessionManager::SetOrientation(ScreenId screenId, Orientation orientation, bool isFromNapi)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (orientation < Orientation::UNSPECIFIED || orientation > Orientation::REVERSE_HORIZONTAL) {
        TLOGE(WmsLogTag::DMS, "set: %{public}u", static_cast<uint32_t>(orientation));
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetOrientation");
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail, cannot find screen %{public}" PRIu64"", screenId);
        return DMError::DM_ERROR_NULLPTR;
    }
    // just for get orientation test
    screenSession->SetOrientation(orientation);
    screenSession->ScreenOrientationChange(orientation, GetFoldDisplayMode(), isFromNapi);
    return DMError::DM_OK;
}

bool ScreenSessionManager::SetRotation(ScreenId screenId, Rotation rotationAfter, bool isFromWindow)
{
    TLOGI(WmsLogTag::DMS,
        "Enter, screenId: %{public}" PRIu64 ", rotation: %{public}u, isFromWindow: %{public}u,",
        screenId, rotationAfter, isFromWindow);
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "error, cannot get screen with screenId: %{public}" PRIu64, screenId);
        return false;
    }
    if (rotationAfter == screenSession->GetRotation()) {
        TLOGE(WmsLogTag::DMS, "rotation not changed. screen %{public}" PRIu64" rotation %{public}u",
            screenId, rotationAfter);
        return false;
    }
    TLOGI(WmsLogTag::DMS, "set orientation. rotation %{public}u", rotationAfter);
    screenSession->SetRotation(rotationAfter);
    screenSession->PropertyChange(screenSession->GetScreenProperty(), ScreenPropertyChangeReason::ROTATION);
    NotifyScreenChanged(screenSession->ConvertToScreenInfo(), ScreenChangeEvent::UPDATE_ROTATION);
    NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(), DisplayChangeEvent::UPDATE_ROTATION);
    return true;
}

void ScreenSessionManager::SetSensorSubscriptionEnabled()
{
    isAutoRotationOpen_ = system::GetParameter("persist.display.ar.enabled", "1") == "1";
    if (!isAutoRotationOpen_) {
        TLOGE(WmsLogTag::DMS, "autoRotation is not open");
        return;
    }
    ScreenSensorConnector::SubscribeRotationSensor();
    TLOGI(WmsLogTag::DMS, "subscribe rotation sensor successful");
}

void ScreenSessionManager::SetPostureAndHallSensorEnabled()
{
#if defined(SENSOR_ENABLE) && defined(FOLD_ABILITY_ENABLE)
    if (!g_foldScreenFlag) {
        TLOGI(WmsLogTag::DMS, "current device is not fold phone.");
        return;
    }

    DMS::ScreenSensorMgr::GetInstance().RegisterPostureCallback();
    DMS::ScreenSensorMgr::GetInstance().RegisterHallCallback();
    if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice() &&
        !FoldScreenSensorManager::GetInstance().GetSensorRegisterStatus()) {
        TLOGI(WmsLogTag::DMS, "subscribe hall and posture failed, force change to full screen!");
        TriggerFoldStatusChange(FoldStatus::EXPAND);
    }
    TLOGI(WmsLogTag::DMS, "successful");
    screenEventTracker_.RecordEvent("Dms subscribe Posture and Hall sensor finished.");
#endif
}

bool ScreenSessionManager::SetRotationFromWindow(Rotation targetRotation)
{
    sptr<DisplayInfo> displayInfo = GetDefaultDisplayInfo();
    if (displayInfo == nullptr) {
        return false;
    }
    return SetRotation(displayInfo->GetScreenId(), targetRotation, true);
}

sptr<SupportedScreenModes> ScreenSessionManager::GetScreenModesByDisplayId(DisplayId displayId)
{
    auto displayInfo = GetDisplayInfoById(displayId);
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get display.");
        return nullptr;
    }
    auto screenInfo = GetScreenInfoById(displayInfo->GetScreenId());
    if (screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get screen.");
        return nullptr;
    }
    auto modes = screenInfo->GetModes();
    auto id = screenInfo->GetModeId();
    if (id >= modes.size()) {
        TLOGE(WmsLogTag::DMS, "can not get screenMode.");
        return nullptr;
    }
    return modes[id];
}

sptr<ScreenInfo> ScreenSessionManager::GetScreenInfoByDisplayId(DisplayId displayId)
{
    auto displayInfo = GetDisplayInfoById(displayId);
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get displayInfo.");
        return nullptr;
    }
    return GetScreenInfoById(displayInfo->GetScreenId());
}

int ScreenSessionManager::NotifyPowerEventForDualDisplay(DisplayPowerEvent event, EventStatus status,
    PowerStateChangeReason reason)
{
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    if (screenSessionMap_.empty()) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]screenSessionMap is empty");
        return NOTIFY_EVENT_FOR_DUAL_FAILED;
    }
#ifdef FOLD_ABILITY_ENABLE
    // The on/off screen will send a notification based on the number of screens.
    // The dual display device just notify the current screen usage
    if (FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
        ScreenId currentScreenId = foldScreenController_->GetCurrentScreenId();
        auto iter = screenSessionMap_.find(currentScreenId);
        if (iter != screenSessionMap_.end() && iter->second != nullptr) {
            iter->second->PowerStatusChange(event, status, reason);
        }
        return NOTIFY_EVENT_FOR_DUAL_SUCESS;
    }
#endif
    return NO_NEED_NOTIFY_EVENT_FOR_DUAL;
}

bool ScreenSessionManager::NotifyDisplayPowerEvent(DisplayPowerEvent event, EventStatus status,
    PowerStateChangeReason reason)
{
    bool notifyResult = ScreenSessionManagerAdapter::GetInstance().NotifyDisplayPowerEvent(event, status);
    if (!notifyResult) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER] Agents failed to notify");
        return false;
    }
    auto ret = NotifyPowerEventForDualDisplay(event, status, reason);
    if (ret == NOTIFY_EVENT_FOR_DUAL_FAILED) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]NotifyPowerEventForDualDisplay ret false");
        return false;
    } else if (ret == NOTIFY_EVENT_FOR_DUAL_SUCESS) {
        TLOGD(WmsLogTag::DMS, "[UL_POWER]NotifyPowerEventForDualDisplay ret sucess");
        return true;
    }
    auto screenIds = GetAllScreenIds();
    if (screenIds.empty()) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]no screenID");
        return false;
    }
    auto screenId = screenIds[0];
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "[UL_POWER]Cannot get ScreenSession, screenId: %{public}" PRIu64"", screenId);
        return false;
    }
    screenSession->PowerStatusChange(event, status, reason);
    return true;
}

bool ScreenSessionManager::NotifyDisplayStateChanged(DisplayId id, DisplayState state)
{
    return ScreenSessionManagerAdapter::GetInstance().NotifyDisplayStateChanged(id, state);
}

DMError ScreenSessionManager::GetAllScreenInfos(std::vector<sptr<ScreenInfo>>& screenInfos)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    std::vector<ScreenId> screenIds = GetAllScreenIds();
    if (screenIds.empty()) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]no screen info");
        return DMError::DM_OK;
    }
    for (auto screenId : screenIds) {
        auto screenInfo = GetScreenInfoById(screenId);
        if (screenInfo == nullptr) {
            TLOGE(WmsLogTag::DMS, "cannot find screenInfo: %{public}" PRIu64"", screenId);
            continue;
        }
        screenInfos.emplace_back(screenInfo);
    }
    return DMError::DM_OK;
}

std::vector<ScreenId> ScreenSessionManager::GetAllScreenIds() const
{
    std::vector<ScreenId> res;
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (const auto& iter : screenSessionMap_) {
        res.emplace_back(iter.first);
    }
    return res;
}

DMError ScreenSessionManager::GetScreenSupportedColorGamuts(ScreenId screenId,
    std::vector<ScreenColorGamut>& colorGamuts)
{
    TLOGI(WmsLogTag::DMS, "ENTER");
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    sptr<ScreenSession> screen = GetScreenSession(screenId);
    if (screen == nullptr) {
        TLOGE(WmsLogTag::DMS, "nullptr");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screen->GetScreenSupportedColorGamuts(colorGamuts);
}

DMError ScreenSessionManager::GetPixelFormat(ScreenId screenId, GraphicPixelFormat& pixelFormat)
{
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64, screenId);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screenSession->GetPixelFormat(pixelFormat);
}

DMError ScreenSessionManager::SetPixelFormat(ScreenId screenId, GraphicPixelFormat pixelFormat)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 ", pixelFormat %{public}d",
        screenId, pixelFormat);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screenSession->SetPixelFormat(pixelFormat);
}

DMError ScreenSessionManager::GetSupportedHDRFormats(ScreenId screenId,
    std::vector<ScreenHDRFormat>& hdrFormats)
{
#ifdef WM_SCREEN_HDR_FORMAT_ENABLE
    TLOGI(WmsLogTag::DMS, "start %{public}" PRIu64, screenId);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screen = GetScreenSession(screenId);
    if (screen == nullptr) {
        TLOGE(WmsLogTag::DMS, "nullptr");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screen->GetSupportedHDRFormats(hdrFormats);
#else
    return DMError::DM_OK;
#endif
}

DMError ScreenSessionManager::GetScreenHDRFormat(ScreenId screenId, ScreenHDRFormat& hdrFormat)
{
#ifdef WM_SCREEN_HDR_FORMAT_ENABLE
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64, screenId);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screenSession->GetScreenHDRFormat(hdrFormat);
#else
    return DMError::DM_OK;
#endif
}

DMError ScreenSessionManager::SetScreenHDRFormat(ScreenId screenId, int32_t modeIdx)
{
#ifdef WM_SCREEN_HDR_FORMAT_ENABLE
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 ", modeIdx %{public}d", screenId, modeIdx);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screenSession->SetScreenHDRFormat(modeIdx);
#else
    return DMError::DM_OK;
#endif
}

DMError ScreenSessionManager::GetSupportedColorSpaces(ScreenId screenId,
    std::vector<GraphicCM_ColorSpaceType>& colorSpaces)
{
#ifdef WM_SCREEN_COLOR_SPACE_ENABLE
    TLOGI(WmsLogTag::DMS, "start %{public}" PRIu64, screenId);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "nullptr");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screenSession->GetSupportedColorSpaces(colorSpaces);
#else
    return DMError::DM_OK;
#endif
}

DMError ScreenSessionManager::GetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType& colorSpace)
{
#ifdef WM_SCREEN_COLOR_SPACE_ENABLE
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64, screenId);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screenSession->GetScreenColorSpace(colorSpace);
#else
    return DMError::DM_OK;
#endif
}

DMError ScreenSessionManager::SetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType colorSpace)
{
#ifdef WM_SCREEN_COLOR_SPACE_ENABLE
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 ", colorSpace %{public}d",
        screenId, colorSpace);
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "screenId invalid");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return screenSession->SetScreenColorSpace(colorSpace);
#else
    return DMError::DM_OK;
#endif
}

void ScreenSessionManager::AddVirtualScreenDeathRecipient(const sptr<IRemoteObject>& displayManagerAgent,
    ScreenId smsScreenId)
{
    if (deathRecipient_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "Create deathRecipient");
        deathRecipient_ =
            new(std::nothrow) AgentDeathRecipient([this](const sptr<IRemoteObject>& agent) { OnRemoteDied(agent); });
    }
    std::lock_guard<std::mutex> lock(screenAgentMapMutex_);
    if (deathRecipient_ != nullptr) {
        auto agIter = screenAgentMap_.find(displayManagerAgent);
        if (agIter == screenAgentMap_.end()) {
            displayManagerAgent->AddDeathRecipient(deathRecipient_);
        }
    }
    screenAgentMap_[displayManagerAgent].emplace_back(smsScreenId);
}

ScreenId ScreenSessionManager::CreateVirtualScreen(VirtualScreenOption option,
                                                   const sptr<IRemoteObject>& displayManagerAgent)
{
    DMError err = CheckVirtualScreenPermission();
    if (err == DMError::DM_ERROR_INVALID_PERMISSION) {
        return ERROR_ID_NO_PERMISSION;
    } else if (err == DMError::DM_ERROR_NOT_SYSTEM_APP) {
        return ERROR_ID_NOT_SYSTEM_APP;
    }
    if (option.virtualScreenType_ != VirtualScreenType::SCREEN_RECORDING) {
        ExitCoordination("CreateVirtualScreen(cast)");
    }
    TLOGI(WmsLogTag::DMS, "ENTER, virtualScreenType: %{public}u", static_cast<uint32_t>(option.virtualScreenType_));
    if (SessionPermission::IsBetaVersion()) {
        CheckAndSendHiSysEvent("CREATE_VIRTUAL_SCREEN", "hmos.screenrecorder");
    }
    auto clientProxy = GetClientProxy();
    if (clientProxy && option.missionIds_.size() > 0) {
        std::vector<uint64_t> surfaceNodeIds;
        clientProxy->OnGetSurfaceNodeIdsFromMissionIdsChanged(option.missionIds_, surfaceNodeIds);
        option.missionIds_ = surfaceNodeIds;
    }
    ScreenId rsId = rsInterface_.CreateVirtualScreen(option.name_, option.width_,
        option.height_, option.surface_, SCREEN_ID_INVALID, option.flags_);
    if (rsId == SCREEN_ID_INVALID) {
        TLOGI(WmsLogTag::DMS, "rsId is invalid");
        return SCREEN_ID_INVALID;
    }
    TLOGW(WmsLogTag::DMS, "rsId: %{public}" PRIu64"", rsId);
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:CreateVirtualScreen(%s)", option.name_.c_str());
    ScreenId smsScreenId = SCREEN_ID_INVALID;
    if (!screenIdManager_.ConvertToSmsScreenId(rsId, smsScreenId)) {
        TLOGW(WmsLogTag::DMS, "!ConvertToSmsScreenId(rsId, smsScreenId)");
        smsScreenId = screenIdManager_.CreateAndGetNewScreenId(rsId);
        auto screenSession = InitVirtualScreen(smsScreenId, rsId, option);
        if (screenSession == nullptr) {
            TLOGW(WmsLogTag::DMS, "screenSession is nullptr");
            screenIdManager_.DeleteScreenId(smsScreenId);
            return SCREEN_ID_INVALID;
        }
        screenSession->SetName(option.name_);
        screenSession->SetVirtualScreenType(option.virtualScreenType_);
        screenSession->SetMirrorScreenType(MirrorScreenType::VIRTUAL_MIRROR);
        screenSession->SetSecurity(option.isSecurity_);
        {
            std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
            screenSessionMap_.insert(std::make_pair(smsScreenId, screenSession));
        }
        screenSession->SetVirtualScreenFlag(option.virtualScreenFlag_);
        TLOGI(WmsLogTag::DMS, "virtualScreenFlag: %{public}d", option.virtualScreenFlag_);
        NotifyScreenConnected(screenSession->ConvertToScreenInfo());
        TLOGW(WmsLogTag::DMS, "create success. ScreenId: %{public}" PRIu64", rsId: %{public}" PRIu64"",
            smsScreenId, rsId);
        if (displayManagerAgent == nullptr) {
            virtualScreenCount_ = virtualScreenCount_ + 1;
            NotifyCaptureStatusChanged();
            return smsScreenId;
        }
        AddVirtualScreenDeathRecipient(displayManagerAgent, smsScreenId);
    }
    virtualScreenCount_ = virtualScreenCount_ + 1;
    NotifyCaptureStatusChanged();
    SetConfigForInputmethod(smsScreenId, option);
    return smsScreenId;
}

DMError ScreenSessionManager::SetVirtualScreenSurface(ScreenId screenId, sptr<IBufferProducer> surface)
{
    bool isCallingByThirdParty = Permission::CheckCallingPermission(ACCESS_VIRTUAL_SCREEN_PERMISSION);
    DMError err = CheckVirtualScreenPermission();
    if (err != DMError::DM_OK) {
        return err;
    }
    if (surface == nullptr) {
        TLOGE(WmsLogTag::DMS, "surface is null");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "No such screen.");
        return isCallingByThirdParty ? DMError::DM_ERROR_NULLPTR : DMError::DM_ERROR_INVALID_PARAM;
    }
    TLOGW(WmsLogTag::DMS, "enter set virtual screen surface");
    ScreenId rsScreenId;
    int32_t res = -1;
    if (screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
        sptr<Surface> pSurface = Surface::CreateSurfaceAsProducer(surface);
        if (pSurface != nullptr) {
            res = rsInterface_.SetVirtualScreenSurface(rsScreenId, pSurface);
        }
    }
    if (res != 0) {
        TLOGE(WmsLogTag::DMS, "fail to set virtual screen surface in RenderService");
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    return DMError::DM_OK;
}

DMError ScreenSessionManager::AddVirtualScreenBlockList(const std::vector<int32_t>& persistentIds)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling clientName: %{public}s, calling pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    MockSessionManagerService::GetInstance().AddSkipSelfWhenShowOnVirtualScreenList(
        persistentIds, GetUserIdByCallingUid());
    return DMError::DM_OK;
}

DMError ScreenSessionManager::RemoveVirtualScreenBlockList(const std::vector<int32_t>& persistentIds)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling clientName: %{public}s, calling pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    MockSessionManagerService::GetInstance().RemoveSkipSelfWhenShowOnVirtualScreenList(
        persistentIds, GetUserIdByCallingUid());
    return DMError::DM_OK;
}

DMError ValidateParameters(ScreenId screenId, const std::vector<uint64_t>& missionIds, const char* logPrefix)
{
    if (screenId == INVALID_SCREEN_ID) {
        TLOGE(WmsLogTag::DMS, "%{public}s::screenId is invalid", logPrefix);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (missionIds.empty()) {
        TLOGE(WmsLogTag::DMS, "%{public}s::missionIds is empty", logPrefix);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    TLOGI(WmsLogTag::DMS, "%{public}s::screenId:%{public}" PRIu64, logPrefix, screenId);
    return DMError::DM_OK;
}

void LogMissionIds(ScreenId screenId, const std::vector<uint64_t>& ids, const char* prefix)
{
    std::ostringstream oss;
    oss << prefix << "[" << screenId << "]: ";
    for (auto val : ids) {
        oss << val << " ";
    }
    TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
}

DMError ScreenSessionManager::AddVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds)
{
    auto err = ValidateParameters(screenId, missionIds, "AddWhiteList");
    if (err != DMError::DM_OK) {
        return err;
    }
    auto rsId = screenIdManager_.ConvertToRsScreenId(screenId);
    TLOGI(WmsLogTag::DMS, "AddWhiteList::rsId:%{public}" PRIu64, rsId);

    LogMissionIds(screenId, missionIds, "AddWhiteList::Get from ScreenRecord: missionIds");
    std::vector<uint64_t> surfaceNodeIds = ProcessMissionIdsToSurfaceNodeIds(missionIds);
    LogMissionIds(screenId, surfaceNodeIds, "AddWhiteList::Transfer to RS: surfaceNodeIds");
    if (surfaceNodeIds.empty()) {
        TLOGE(WmsLogTag::DMS, "AddWhiteList::surfaceNodeIds is empty");
    }
    TLOGI(WmsLogTag::DMS, "AddWhiteList::surfaceNodeIds size Add: %{public}ud", static_cast<uint32_t>(surfaceNodeIds.size()));

    int32_t rsErrCode = rsInterface_.AddVirtualScreenWhiteList(rsId, surfaceNodeIds);
    TLOGI(WmsLogTag::DMS, "AddWhiteList::rsErrCode:%{public}d", rsErrCode);
    return (rsErrCode == static_cast<int32_t>(DMError::DM_OK)) ? DMError::DM_OK : DMError::DM_ERROR_INVALID_PARAM;
}

DMError ScreenSessionManager::RemoveVirtualScreenWhiteList(ScreenId screenId, const std::vector<uint64_t>& missionIds)
{
    auto err = ValidateParameters(screenId, missionIds, "RemoveWhiteList");
    if (err != DMError::DM_OK) {
        return err;
    }
    auto rsId = screenIdManager_.ConvertToRsScreenId(screenId);
    TLOGI(WmsLogTag::DMS, "RemoveWhiteList::rsId:%{public}" PRIu64, rsId);
    LogMissionIds(screenId, missionIds, "RemoveWhiteList::Get from ScreenRecord: missionIds");

    std::vector<uint64_t> surfaceNodeIds = ProcessMissionIdsToSurfaceNodeIds(missionIds);
    LogMissionIds(screenId, surfaceNodeIds, "RemoveWhiteList::Transfer to RS: surfaceNodeIds");
    if (surfaceNodeIds.empty()) {
        TLOGE(WmsLogTag::DMS, "AddWhiteList::surfaceNodeIds is empty");
    }

    TLOGI(WmsLogTag::DMS, "surfaceNodeIds size Remove: %{public}ud", static_cast<uint32_t>(surfaceNodeIds.size()));
    int32_t rsErrCode = rsInterface_.RemoveVirtualScreenWhiteList(rsId, surfaceNodeIds);
    TLOGI(WmsLogTag::DMS, "RemoveWhiteList::rsErrCode:%{public}d", rsErrCode);
    return (rsErrCode == static_cast<int32_t>(DMError::DM_OK)) ? DMError::DM_OK : DMError::DM_ERROR_INVALID_PARAM;
}

std::vector<uint64_t> ScreenSessionManager::ProcessMissionIdsToSurfaceNodeIds(const std::vector<uint64_t>& missionIds)
{
    if (missionIds.empty()) {
        return {};
    }
    auto clientProxy = GetClientProxy();
    if (clientProxy) {
        std::vector<uint64_t> surfaceNodeIds;
        std::vector<uint64_t> missionIdsCopy = missionIds;
        clientProxy->OnGetSurfaceNodeIdsFromMissionIdsChanged(missionIdsCopy, surfaceNodeIds);
        return surfaceNodeIds;
    }
    return {};
}

DMError ScreenSessionManager::SetScreenPrivacyMaskImage(ScreenId screenId,
    const std::shared_ptr<Media::PixelMap>& privacyMaskImg)
{
    if (!Permission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling clientName: %{public}s, calling pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "No such screen.");
        return DMError::DM_ERROR_NULLPTR;
    }
    ScreenId rsScreenId;
    if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId.");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    int32_t res = -1;
    if (privacyMaskImg == nullptr) {
        TLOGI(WmsLogTag::DMS, "Clearing screen privacy mask image for screenId: %{public}" PRIu64"",
            static_cast<uint64_t>(screenId));
        res = rsInterface_.SetScreenSecurityMask(rsScreenId, nullptr);
        if (res != 0) {
            TLOGE(WmsLogTag::DMS, "Fail to set privacy mask image in RenderService");
            return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
        }
        return DMError::DM_OK;
    }
    TLOGI(WmsLogTag::DMS, "Setting screen privacy mask image for screenId: %{public}" PRIu64"",
        static_cast<uint64_t>(screenId));
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetScreenSecurityMask(%" PRIu64")", screenId);
    res = rsInterface_.SetScreenSecurityMask(rsScreenId, privacyMaskImg);
    if (res != 0) {
        TLOGE(WmsLogTag::DMS, "Fail to set privacy mask image in RenderService");
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetVirtualMirrorScreenScaleMode(ScreenId screenId, ScreenScaleMode scaleMode)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "No such screen.");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    ScreenId rsScreenId;
    if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    bool res = rsInterface_.SetVirtualMirrorScreenScaleMode(rsScreenId, scaleMode);
    if (!res) {
        TLOGE(WmsLogTag::DMS, "failed in RenderService");
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetVirtualMirrorScreenCanvasRotation(ScreenId screenId, bool autoRotate)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "No such screen.");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    TLOGW(WmsLogTag::DMS, "enter set virtual mirror screen canvas rotation");
    bool res = false;
    ScreenId rsScreenId;
    if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    res = rsInterface_.SetVirtualMirrorScreenCanvasRotation(rsScreenId, autoRotate);
    if (!res) {
        TLOGE(WmsLogTag::DMS, "failed in RenderService");
        return DMError::DM_ERROR_RENDER_SERVICE_FAILED;
    }
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        screenSession->SetIsEnableCanvasRotation(autoRotate);
    }
    TLOGW(WmsLogTag::DMS, "success");
    return DMError::DM_OK;
}

DMError ScreenSessionManager::ResizeVirtualScreen(ScreenId screenId, uint32_t width, uint32_t height)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGW(WmsLogTag::DMS, "screenId: %{public}" PRIu64", width: %{public}u, height: %{public}u.",
        screenId, width, height);
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "No such screen.");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    ScreenId rsScreenId;
    if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    rsInterface_.ResizeVirtualScreen(rsScreenId, width, height);
    screenSession->Resize(width, height);
    screenSession->PropertyChange(screenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::VIRTUAL_SCREEN_RESIZE);
    return DMError::DM_OK;
}

DMError ScreenSessionManager::DestroyVirtualScreen(ScreenId screenId)
{
    bool isCallingByThirdParty = Permission::CheckCallingPermission(ACCESS_VIRTUAL_SCREEN_PERMISSION);
    if (!SessionPermission::IsSystemCalling() && !isCallingByThirdParty) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (static_cast<uint64_t>(screenId) < static_cast<uint64_t>(MINIMUM_VIRTUAL_SCREEN_ID)) {
        TLOGE(WmsLogTag::DMS, "virtual screenId is invalid, id: %{public}" PRIu64"", static_cast<uint64_t>(screenId));
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    // virtual screen destroy callback to notify scb
    TLOGW(WmsLogTag::DMS, "start");
    OnVirtualScreenChange(screenId, ScreenEvent::DISCONNECTED);
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    {
        std::lock_guard<std::mutex> lock(screenAgentMapMutex_);
        screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId);
        for (auto &agentIter : screenAgentMap_) {
            auto iter = std::find(agentIter.second.begin(), agentIter.second.end(), screenId);
            if (iter != agentIter.second.end()) {
                iter = agentIter.second.erase(iter);
                if (agentIter.first != nullptr && agentIter.second.empty()) {
                    screenAgentMap_.erase(agentIter.first);
                }
                break;
            }
        }
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:DestroyVirtualScreen(%" PRIu64")", screenId);
    auto screen = GetScreenSession(screenId);
    if (rsScreenId != SCREEN_ID_INVALID && screen != nullptr) {
        NotifyDisplayDestroy(screenId);
        {
            std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
            auto screenGroup = RemoveFromGroupLocked(screen);
            if (screenGroup != nullptr) {
                NotifyScreenGroupChanged(screen->ConvertToScreenInfo(), ScreenGroupChangeEvent::REMOVE_FROM_GROUP);
            }
            screenSessionMap_.erase(screenId);
        }
        RemoveScreenCastInfo(screenId);
        NotifyScreenDisconnected(screenId);
        if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && screen->GetIsExtendVirtual()) {
            SetIsExtendModelocked(false);
            SetExpandAndHorizontalLocked(false);
            TLOGW(WmsLogTag::DMS, "statemachine unlocked");
        }
        if (auto clientProxy = GetClientProxy()) {
            clientProxy->OnVirtualScreenDisconnected(rsScreenId);
        }
        TLOGW(WmsLogTag::DMS, "destroy success, id: %{public}" PRIu64 ", rsId: %{public}" PRIu64, screenId, rsScreenId);
    }
    screenIdManager_.DeleteScreenId(screenId);
    virtualScreenCount_ = virtualScreenCount_ > 0 ? virtualScreenCount_ - 1 : 0;
    NotifyCaptureStatusChanged();
    if (rsScreenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsScreenId");
        return isCallingByThirdParty ? DMError::DM_ERROR_NULLPTR : DMError::DM_ERROR_INVALID_PARAM;
    }
    rsInterface_.RemoveVirtualScreen(rsScreenId);
    return DMError::DM_OK;
}

DMError ScreenSessionManager::DisableMirror(bool disableOrNot)
{
    TLOGW(WmsLogTag::DMS, "start %{public}d", disableOrNot);
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGI(WmsLogTag::DMS, "enter %{public}d", disableOrNot);
    if (disableOrNot) {
        std::vector<ScreenId> screenIds;
        auto allScreenIds = GetAllScreenIds();
        for (auto screenId : allScreenIds) {
            auto screen = GetScreenSession(screenId);
            if (screen && screen->GetScreenProperty().GetScreenType() == ScreenType::VIRTUAL) {
                screenIds.push_back(screenId);
            }
        }
        StopMirror(screenIds);
    }
    return DMError::DM_OK;
}

void ScreenSessionManager::MirrorSwitchNotify(ScreenId screenId)
{
    auto mirrorScreen = GetScreenSession(screenId);
    if (mirrorScreen != nullptr) {
        mirrorScreen->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
        NotifyScreenChanged(mirrorScreen->ConvertToScreenInfo(), ScreenChangeEvent::SCREEN_SWITCH_CHANGE);
    }
}

DMError ScreenSessionManager::DoMakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
    DMRect mainScreenRegion, ScreenId& screenGroupId, const RotationOption& rotationOption, bool forceMirror)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    TLOGW(WmsLogTag::DMS, "enter!");
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (system::GetBoolParameter("persist.edm.disallow_mirror", false)) {
        TLOGW(WmsLogTag::DMS, "disabled by edm!");
        return DMError::DM_ERROR_INVALID_PERMISSION;
    }

    TLOGW(WmsLogTag::DMS, "mainScreenId :%{public}" PRIu64"", mainScreenId);
    auto allMirrorScreenIds = GetAllValidScreenIds(mirrorScreenIds);
    auto iter = std::find(allMirrorScreenIds.begin(), allMirrorScreenIds.end(), mainScreenId);
    if (iter != allMirrorScreenIds.end()) {
        allMirrorScreenIds.erase(iter);
    }
    auto mainScreen = GetScreenSession(mainScreenId);
    if (mainScreen == nullptr || allMirrorScreenIds.empty()) {
        TLOGE(WmsLogTag::DMS, "MakeMirror fail. mainScreen :%{public}" PRIu64", screens size:%{public}u",
            mainScreenId, static_cast<uint32_t>(allMirrorScreenIds.size()));
        return DMError::DM_ERROR_NULLPTR;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:MakeMirror start");
    for (ScreenId screenId : allMirrorScreenIds) {
        OnVirtualScreenChange(screenId, ScreenEvent::DISCONNECTED);
    }
    DMError makeResult = MultiScreenManager::GetInstance().MirrorSwitch(mainScreenId,
        allMirrorScreenIds, mainScreenRegion, screenGroupId, rotationOption, forceMirror);
    if (makeResult != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "MakeMirror set mirror failed.");
        return makeResult;
    }
    for (ScreenId screenId : allMirrorScreenIds) {
        MirrorSwitchNotify(screenId);
        auto screenSession = GetScreenSession(screenId);
        if (screenSession != nullptr && mainScreen != nullptr) {
            screenSession->SetDisplayGroupId(mainScreen->GetDisplayGroupId());
            screenSession->SetMainDisplayIdOfGroup(mainScreen->GetMainDisplayIdOfGroup());
        }
    }
    RegisterCastObserver(allMirrorScreenIds);
    TLOGW(WmsLogTag::DMS, "make mirror notify scb end makeResult=%{public}d", makeResult);
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:MakeMirror end");
    return makeResult;
#else
    return DMError::DM_OK;
#endif
}

DMError ScreenSessionManager::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
    ScreenId& screenGroupId, const RotationOption& rotationOption, bool forceMirror)
{
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ != nullptr && FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        DMRect mainScreenRegion = DMRect::NONE();
        foldScreenController_->SetMainScreenRegion(mainScreenRegion);
        return DoMakeMirror(mainScreenId, mirrorScreenIds, mainScreenRegion, screenGroupId, rotationOption,
            forceMirror);
    }
#endif
    return DoMakeMirror(mainScreenId, mirrorScreenIds, DMRect::NONE(), screenGroupId, rotationOption, forceMirror);
}

DMError ScreenSessionManager::MakeMirrorForRecord(const std::vector<ScreenId>& mainScreenIds,
    std::vector<ScreenId>& mirrorScreenIds, ScreenId& screenGroupId)
{
#ifdef FOLD_ABILITY_ENABLE
    auto realScreenId = SuperFoldPolicy::GetInstance().GetRealScreenId(mainScreenIds);
    TLOGI(WmsLogTag::DMS, "realScreenId: %{public}llu", static_cast<uint64_t>(realScreenId));
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && realScreenId != SCREEN_ID_INVALID) {
        DMRect mainScreenRect = SuperFoldPolicy::GetInstance().GetRecordRect(mainScreenIds);
        std::ostringstream oss;
        oss << "Rect: x: " << mainScreenRect.posX_ << 
            "y: " << mainScreenRect.posY_ <<
            "width: " << mainScreenRect.width_ <<
            "height: " << mainScreenRect.height_;
        TLOGI(WmsLogTag::DMS, "mainScreenRect: %{public}s", oss.str().c_str());
        auto makeResult = DoMakeMirror(realScreenId, mirrorScreenIds, mainScreenRect, screenGroupId);
        if (makeResult == DMError::DM_OK) {
            SuperFoldStateManager::GetInstance().AddMirrorVirtualScreenIds(mirrorScreenIds, mainScreenRect);
        }
        return makeResult;
    }
    return DMError::DM_ERROR_INVALID_PARAM;
#else
    if (mainScreenIds.size() == 1) {
        return DoMakeMirror(mainScreenIds[0], mirrorScreenIds, DMRect::NONE(), screenGroupId);
    } else {
        return DMError::DM_ERROR_INVALID_PARAM;
    }
#endif
}


DMError ScreenSessionManager::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
                                         DMRect mainScreenRegion, ScreenId& screenGroupId)
{
    return DoMakeMirror(mainScreenId, mirrorScreenIds, mainScreenRegion, screenGroupId);
}

void ScreenSessionManager::RegisterCastObserver(std::vector<ScreenId>& mirrorScreenIds)
{
    {
        std::lock_guard<std::mutex> lock(mirrorScreenIdsMutex_);
        mirrorScreenIds_ = mirrorScreenIds;
    }
    TLOGI(WmsLogTag::DMS, "Register Setting cast Observer");
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) { SetCastFromSettingData(); };
    ScreenSettingHelper::RegisterSettingCastObserver(DmUtils::wrap_callback(updateFunc));
}

void ScreenSessionManager::SetCastFromSettingData()
{
    bool enable;
    bool ret = ScreenSettingHelper::GetSettingCast(enable);
    if (!ret) {
        TLOGW(WmsLogTag::DMS, "get setting cast failed, default enable");
        enable = true;
    } else {
        TLOGI(WmsLogTag::DMS, "get setting cast success, enable: %{public}u", enable);
    }

    std::vector<ScreenId> mirrorScreenIdsCopy;
    {
    std::lock_guard<std::mutex> lock(mirrorScreenIdsMutex_);
        mirrorScreenIdsCopy = mirrorScreenIds_;
    }
    for (ScreenId screenId : mirrorScreenIdsCopy) {
        ScreenId rsScreenId;
        if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
            TLOGE(WmsLogTag::DMS, "No corresponding rsId");
            continue;
        }
        rsInterface_.SetCastScreenEnableSkipWindow(rsScreenId, enable);
    }
}

void ScreenSessionManager::RegisterSettingRotationObserver()
{
    TLOGI(WmsLogTag::DMS, "start");
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) {
        int32_t rotation = -1;
        int32_t screenId = -1;
        if (ScreenSettingHelper::GetSettingRotation(rotation) &&
            ScreenSettingHelper::GetSettingRotationScreenID(screenId)) {
            TLOGNI(WmsLogTag::DMS, "current dms setting rotation:%{public}d, screenId:%{public}d",
                rotation, screenId);
        } else {
            TLOGNI(WmsLogTag::DMS, "get current dms setting rotation and screenId failed");
        }
    };
    ScreenSettingHelper::RegisterSettingRotationObserver(DmUtils::wrap_callback(updateFunc));
}

DMError ScreenSessionManager::StopMirror(const std::vector<ScreenId>& mirrorScreenIds)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    auto allMirrorScreenIds = GetAllValidScreenIds(mirrorScreenIds);
    if (allMirrorScreenIds.empty()) {
        TLOGW(WmsLogTag::DMS, "done. screens' size:%{public}u",
            static_cast<uint32_t>(allMirrorScreenIds.size()));
        return DMError::DM_OK;
    }

    DMError ret = StopScreens(allMirrorScreenIds, ScreenCombination::SCREEN_MIRROR);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "failed.");
        return ret;
    }
#ifdef FOLD_ABILITY_ENABLE
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        SuperFoldStateManager::GetInstance().ClearMirrorVirtualScreenIds(mirrorScreenIds);
    }
#endif
    ScreenSettingHelper::UnregisterSettingCastObserver();

    return DMError::DM_OK;
}

DMError ScreenSessionManager::StopScreens(const std::vector<ScreenId>& screenIds, ScreenCombination stopCombination)
{
    for (ScreenId screenId : screenIds) {
        TLOGW(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64"", screenId);
        auto screen = GetScreenSession(screenId);
        if (screen == nullptr) {
            TLOGW(WmsLogTag::DMS, "screen:%{public}" PRIu64" is nullptr", screenId);
            continue;
        }
        sptr<ScreenSessionGroup> screenGroup = GetAbstractScreenGroup(screen->groupSmsId_);
        if (!screenGroup) {
            TLOGW(WmsLogTag::DMS, "groupDmsId:%{public}" PRIu64"is not in smsScreenGroupMap_",
                screen->groupSmsId_);
            continue;
        }
        if (screenGroup->combination_ != stopCombination) {
            TLOGW(WmsLogTag::DMS, "try to stop screen in another combination");
            continue;
        }
        if (screenGroup->combination_ == ScreenCombination::SCREEN_MIRROR &&
            screen->screenId_ == screenGroup->mirrorScreenId_) {
            TLOGW(WmsLogTag::DMS, "try to stop main mirror screen");
            continue;
        }
        bool res = RemoveChildFromGroup(screen, screenGroup);
        if (res) {
            NotifyScreenGroupChanged(screen->ConvertToScreenInfo(), ScreenGroupChangeEvent::REMOVE_FROM_GROUP);
        }
    }
    return DMError::DM_OK;
}

VirtualScreenFlag ScreenSessionManager::GetVirtualScreenFlag(ScreenId screenId)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return VirtualScreenFlag::DEFAULT;
    }
    auto screen = GetScreenSession(screenId);
    if (screen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen session null");
        return VirtualScreenFlag::DEFAULT;
    }
    return screen->GetVirtualScreenFlag();
}

DMError ScreenSessionManager::SetVirtualScreenFlag(ScreenId screenId, VirtualScreenFlag screenFlag)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (screenFlag < VirtualScreenFlag::DEFAULT || screenFlag >= VirtualScreenFlag::MAX) {
        TLOGE(WmsLogTag::DMS, "range error");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto screen = GetScreenSession(screenId);
    if (screen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen session null");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    screen->SetVirtualScreenFlag(screenFlag);
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetVirtualScreenRefreshRate(ScreenId screenId, uint32_t refreshInterval)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGW(WmsLogTag::DMS, "screenId: %{public}" PRIu64", refreshInterval:  %{public}u",
        screenId, refreshInterval);
    if (screenId == GetDefaultScreenId()) {
        TLOGE(WmsLogTag::DMS, "cannot set refresh rate of main screen id: %{public}" PRIu64, GetDefaultScreenId());
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (refreshInterval == 0) {
        TLOGE(WmsLogTag::DMS, "refresh interval is 0.");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto screenSession = GetScreenSession(screenId);
    auto defaultScreenSession = GetDefaultScreenSession();
    if (screenSession == nullptr || defaultScreenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null.");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    ScreenId rsScreenId;
    if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId.");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    int32_t res = rsInterface_.SetScreenSkipFrameInterval(rsScreenId, refreshInterval);
    if (res != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS, "rsInterface error: %{public}d", res);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    // when skipFrameInterval > 10 means the skipFrameInterval is the virtual screen refresh rate
    if (refreshInterval > IRREGULAR_REFRESH_RATE_SKIP_THRETHOLD) {
        screenSession->UpdateRefreshRate(refreshInterval);
        screenSession->SetSupportedRefreshRate({refreshInterval});
    } else {
        screenSession->UpdateRefreshRate(defaultScreenSession->GetRefreshRate() / refreshInterval);
        screenSession->SetSupportedRefreshRate({defaultScreenSession->GetRefreshRate() / refreshInterval});
    }
    NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(), DisplayChangeEvent::UPDATE_REFRESHRATE);
    TLOGW(WmsLogTag::DMS, "refreshInterval is %{public}d", refreshInterval);
    return DMError::DM_OK;
}

DMError ScreenSessionManager::VirtualScreenUniqueSwitch(const std::vector<ScreenId>& screenIds,
    const UniqueScreenRotationOptions& rotationOptions)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    TLOGW(WmsLogTag::DMS, "enter");
    auto defaultScreen = GetDefaultScreenSession();
    if (!defaultScreen) {
        TLOGE(WmsLogTag::DMS, "default screen is nullptr");
        return DMError::DM_ERROR_NULLPTR;
    }
    defaultScreen->groupSmsId_ = 1;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        auto iter = smsScreenGroupMap_.find(defaultScreen->groupSmsId_);
        if (iter != smsScreenGroupMap_.end()) {
            smsScreenGroupMap_.erase(iter);
        }
    }
    // Update the rotation-orientation mapping based on the device form factor
    Rotation rotation = defaultScreen->GetScreenProperty().GetDeviceRotation();
    int32_t orientation = GetDeviceOrientationAPI14(defaultScreen, rotation);
    TLOGD(WmsLogTag::DMS, "default screenSession deviceRotation: %{public}d, orientationAPI14: %{public}d",
        rotation, orientation);
    UniqueScreenRotationOptions modifiedOptions = rotationOptions;
    if (defaultScreen->UpdateRotationOrientationMap(modifiedOptions, static_cast<int32_t>(rotation),
        orientation)) {
        TLOGI(WmsLogTag::DMS, "update rotationOrientationMap success");
    } else {
        TLOGI(WmsLogTag::DMS, "update rotationOrientationMap failed");
    }

    DMError uniqueSwitchRet = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(defaultScreen, screenIds,
        modifiedOptions);
    TLOGW(WmsLogTag::DMS, "result: %{public}d", uniqueSwitchRet);
    return uniqueSwitchRet;
#else
    return DMError::DM_OK;
#endif
}

int32_t ScreenSessionManager::GetDeviceOrientationAPI14(sptr<ScreenSession> screenSession, Rotation rotation)
{
    RRect bounds = screenSession->GetScreenProperty().GetBounds();
    DisplayOrientation deviceOrientation =
        screenSession->CalcDeviceOrientationWithBounds(rotation, GetFoldDisplayMode(), bounds);
    return static_cast<int32_t>(deviceOrientation);
}

DMError ScreenSessionManager::MakeUniqueScreen(const std::vector<ScreenId>& screenIds,
    std::vector<DisplayId>& displayIds, const UniqueScreenRotationOptions& rotationOptions)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    TLOGD(WmsLogTag::DMS,
          "rotationOptions passes through, isRotationLocked: %{public}d, rotation: %{public}d",
          rotationOptions.isRotationLocked_, rotationOptions.rotation_);
    bool isCallingByThirdParty = Permission::CheckCallingPermission(ACCESS_VIRTUAL_SCREEN_PERMISSION);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd() && !isCallingByThirdParty) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGW(WmsLogTag::DMS, "enter!");
    if (screenIds.empty()) {
        TLOGE(WmsLogTag::DMS, "screen is empty");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    std::vector<ScreenId> validScreenIds;
    for (auto screenId : screenIds) {
        if (!IsDefaultMirrorMode(screenId)) {
            continue;
        }
        validScreenIds.emplace_back(screenId);
    }
    const auto& allUniqueScreenIds = GetAllValidScreenIds(validScreenIds);
    if (allUniqueScreenIds.empty()) {
        TLOGE(WmsLogTag::DMS, "screenIds is invalid.");
        return DMError::DM_ERROR_NULLPTR;
    }
    ScreenId uniqueScreenId = validScreenIds[0];
    auto uniqueScreen = GetScreenSession(uniqueScreenId);
    if (uniqueScreen != nullptr) {
        if (uniqueScreen->GetSourceMode() == ScreenSourceMode::SCREEN_UNIQUE) {
            TLOGW(WmsLogTag::DMS, "make unique ignore");
            return DMError::DM_OK;
        }
        if (isCallingByThirdParty || uniqueScreen->GetName() == "Cooperation" ||
            uniqueScreen->GetName() == "Cooperation-multi") {
            uniqueScreen->SetInnerName("CustomScbScreen");
        }
        return MultiScreenManager::GetInstance().UniqueSwitch(allUniqueScreenIds, displayIds, rotationOptions);
    }
    return DoMakeUniqueScreenOld(allUniqueScreenIds, displayIds, isCallingByThirdParty);
#else
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
#endif
}

DMError ScreenSessionManager::DoMakeUniqueScreenOld(const std::vector<ScreenId>& allUniqueScreenIds,
    std::vector<DisplayId>& displayIds, bool isCallingByThirdParty)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    std::unordered_set<std::shared_ptr<RSUIContext>> rsUIContexts;
    for (auto screenId : allUniqueScreenIds) {
        ScreenId rsScreenId = SCREEN_ID_INVALID;
        bool res = ConvertScreenIdToRsScreenId(screenId, rsScreenId);
        TLOGI(WmsLogTag::DMS, "unique screenId: %{public}" PRIu64" rsScreenId: %{public}" PRIu64,
            screenId, rsScreenId);
        if (!res) {
            TLOGE(WmsLogTag::DMS, "convert screenId to rsScreenId failed");
            continue;
        }
        auto screenSession = GetScreenSession(screenId);
        if (!screenSession) {
            TLOGE(WmsLogTag::DMS, "screen session is nullptr");
            continue;
        }
        displayIds.emplace_back(static_cast<uint64_t>(screenId));
        Rosen::RSDisplayNodeConfig rsConfig;
        rsConfig.screenId = rsScreenId;
        screenSession->CreateDisplayNode(rsConfig);
        screenSession->SetDisplayNodeScreenId(rsScreenId);
        if (isCallingByThirdParty) {
            screenSession->SetInnerName("CustomScbScreen");
        }
        // notify scb to build Screen widget
        OnVirtualScreenChange(screenId, ScreenEvent::CONNECTED);
        rsUIContexts.insert(screenSession->GetRSUIContext());
    }
    TLOGD(WmsLogTag::DMS, "flush data");
    RSTransactionAdapter::FlushImplicitTransaction(rsUIContexts);
#endif
    return DMError::DM_OK;
}

void ScreenSessionManager::NotifyScreenConnectCompletion(ScreenId screenId)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    TLOGI(WmsLogTag::DMS, "ENTER, screenId:%{public}" PRIu64, screenId);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return;
    }
    MultiScreenManager::GetInstance().NotifyScreenConnectCompletion(screenId);
#endif
}

DMError ScreenSessionManager::MakeExpand(std::vector<ScreenId> screenId,
                                         std::vector<Point> startPoint,
                                         ScreenId& screenGroupId)
{
    TLOGI(WmsLogTag::DMS, "enter!");
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! pid: %{public}d", IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (screenId.empty() || startPoint.empty() || screenId.size() != startPoint.size()) {
        TLOGE(WmsLogTag::DMS, "create expand fail, screenId size:%{public}ud,startPoint size:%{public}ud",
            static_cast<uint32_t>(screenId.size()), static_cast<uint32_t>(startPoint.size()));
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    std::map<ScreenId, Point> pointsMap;
    uint32_t size = screenId.size();
    for (uint32_t i = 0; i < size; i++) {
        if (pointsMap.find(screenId[i]) != pointsMap.end()) {
            continue;
        }
        pointsMap[screenId[i]] = startPoint[i];
    }
    ScreenId defaultScreenId = GetDefaultScreenId();
    auto allExpandScreenIds = GetAllValidScreenIds(screenId);
    auto iter = std::find(allExpandScreenIds.begin(), allExpandScreenIds.end(), defaultScreenId);
    if (iter != allExpandScreenIds.end()) {
        allExpandScreenIds.erase(iter);
    }
    if (allExpandScreenIds.empty()) {
        TLOGE(WmsLogTag::DMS, "allExpandScreenIds is empty. make expand failed.");
        return DMError::DM_ERROR_NULLPTR;
    }
    std::shared_ptr<RSDisplayNode> rsDisplayNode;
    std::vector<Point> points;
    for (uint32_t i = 0; i < allExpandScreenIds.size(); i++) {
        ScreenId expandScreenId = allExpandScreenIds[i];
        if (pointsMap.find(expandScreenId) == pointsMap.end()) {
            continue;
        }
        points.emplace_back(pointsMap[expandScreenId]);
        rsDisplayNode = GetRSDisplayNodeByScreenId(expandScreenId);
        if (rsDisplayNode != nullptr) {
            SetScreenOffset(expandScreenId, pointsMap[expandScreenId].posX_, pointsMap[expandScreenId].posY_);
        }
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:MakeExpand");
    if (!OnMakeExpand(allExpandScreenIds, points)) {
        return DMError::DM_ERROR_NULLPTR;
    }
    auto screen = GetScreenSession(allExpandScreenIds[0]);
    if (screen == nullptr || GetAbstractScreenGroup(screen->groupSmsId_) == nullptr) {
        return DMError::DM_ERROR_NULLPTR;
    }
    screenGroupId = screen->groupSmsId_;
    return DMError::DM_OK;
}

bool ScreenSessionManager::OnMakeExpand(std::vector<ScreenId> screenId, std::vector<Point> startPoint)
{
    ScreenId defaultScreenId = GetDefaultScreenId();
    TLOGI(WmsLogTag::DMS, "defaultScreenId:%{public}" PRIu64"", defaultScreenId);
    auto defaultScreen = GetScreenSession(defaultScreenId);
    if (defaultScreen == nullptr) {
        TLOGI(WmsLogTag::DMS, "failed.");
        return false;
    }
    auto group = GetAbstractScreenGroup(defaultScreen->groupSmsId_);
    if (group == nullptr) {
        group = AddToGroupLocked(defaultScreen);
        if (group == nullptr) {
            TLOGE(WmsLogTag::DMS, "group is nullptr");
            return false;
        }
        NotifyScreenGroupChanged(defaultScreen->ConvertToScreenInfo(), ScreenGroupChangeEvent::ADD_TO_GROUP);
    }
    bool filterExpandScreen = group->combination_ == ScreenCombination::SCREEN_EXPAND;
    ChangeScreenGroup(group, screenId, startPoint, filterExpandScreen, ScreenCombination::SCREEN_EXPAND);
    TLOGI(WmsLogTag::DMS, "success");
    return true;
}

DMError ScreenSessionManager::StopExpand(const std::vector<ScreenId>& expandScreenIds)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    auto allExpandScreenIds = GetAllValidScreenIds(expandScreenIds);
    if (allExpandScreenIds.empty()) {
        TLOGI(WmsLogTag::DMS, "done. screens' size:%{public}u",
            static_cast<uint32_t>(allExpandScreenIds.size()));
        return DMError::DM_OK;
    }

    DMError ret = StopScreens(allExpandScreenIds, ScreenCombination::SCREEN_EXPAND);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "stop expand failed.");
        return ret;
    }

    return DMError::DM_OK;
}

bool ScreenSessionManager::ScreenIdManager::ConvertToRsScreenId(ScreenId smsScreenId, ScreenId& rsScreenId) const
{
    std::shared_lock lock(screenIdMapMutex_);
    auto iter = sms2RsScreenIdMap_.find(smsScreenId);
    if (iter == sms2RsScreenIdMap_.end()) {
        return false;
    }
    rsScreenId = iter->second;
    return true;
}

ScreenId ScreenSessionManager::ScreenIdManager::ConvertToRsScreenId(ScreenId screenId) const
{
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    ConvertToRsScreenId(screenId, rsScreenId);
    return rsScreenId;
}

ScreenId ScreenSessionManager::ScreenIdManager::ConvertToSmsScreenId(ScreenId rsScreenId) const
{
    ScreenId smsScreenId = SCREEN_ID_INVALID;
    ConvertToSmsScreenId(rsScreenId, smsScreenId);
    return smsScreenId;
}

bool ScreenSessionManager::ScreenIdManager::ConvertToSmsScreenId(ScreenId rsScreenId, ScreenId& smsScreenId) const
{
    std::shared_lock lock(screenIdMapMutex_);
    auto iter = rs2SmsScreenIdMap_.find(rsScreenId);
    if (iter == rs2SmsScreenIdMap_.end()) {
        return false;
    }
    smsScreenId = iter->second;
    return true;
}

ScreenId ScreenSessionManager::ScreenIdManager::CreateAndGetNewScreenId(ScreenId rsScreenId)
{
    std::unique_lock lock(screenIdMapMutex_);
    ScreenId smsScreenId = smsScreenCount_++;
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64"", smsScreenId);
    if (sms2RsScreenIdMap_.find(smsScreenId) != sms2RsScreenIdMap_.end()) {
        TLOGW(WmsLogTag::DMS, "screenId: %{public}" PRIu64" exit", smsScreenId);
    }
    sms2RsScreenIdMap_[smsScreenId] = rsScreenId;
    if (rsScreenId == SCREEN_ID_INVALID) {
        return smsScreenId;
    }
    if (rs2SmsScreenIdMap_.find(rsScreenId) != rs2SmsScreenIdMap_.end()) {
        TLOGW(WmsLogTag::DMS, "rsScreenId: %{public}" PRIu64" exit", rsScreenId);
    }
    rs2SmsScreenIdMap_[rsScreenId] = smsScreenId;
    return smsScreenId;
}

void ScreenSessionManager::ScreenIdManager::UpdateScreenId(ScreenId rsScreenId, ScreenId smsScreenId)
{
    std::unique_lock lock(screenIdMapMutex_);
    rs2SmsScreenIdMap_[rsScreenId] = smsScreenId;
    sms2RsScreenIdMap_[smsScreenId] = rsScreenId;
    TLOGI(WmsLogTag::DMS, "rsScreenId: %{public}" PRIu64" smsScreenId: %{public}" PRIu64"", rsScreenId, smsScreenId);
}

bool ScreenSessionManager::ScreenIdManager::DeleteScreenId(ScreenId smsScreenId)
{
    std::unique_lock lock(screenIdMapMutex_);
    auto iter = sms2RsScreenIdMap_.find(smsScreenId);
    if (iter == sms2RsScreenIdMap_.end()) {
        return false;
    }
    ScreenId rsScreenId = iter->second;
    sms2RsScreenIdMap_.erase(smsScreenId);
    rs2SmsScreenIdMap_.erase(rsScreenId);
    TLOGD(WmsLogTag::DMS, "DeleteScreenId: smsScreenId: %{public}d",static_cast<uint32_t>(smsScreenId));
    return true;
}

bool ScreenSessionManager::ScreenIdManager::HasRsScreenId(ScreenId smsScreenId) const
{
    std::shared_lock lock(screenIdMapMutex_);
    return rs2SmsScreenIdMap_.find(smsScreenId) != rs2SmsScreenIdMap_.end();
}

sptr<ScreenSession> ScreenSessionManager::InitVirtualScreen(ScreenId smsScreenId, ScreenId rsId,
    VirtualScreenOption option)
{
    TLOGI(WmsLogTag::DMS, "Enter");
    ScreenSessionConfig config = {
        .screenId = smsScreenId,
        .rsId = rsId,
        .defaultScreenId = GetDefaultScreenId(),
        .name = option.name_,
    };
    sptr<ScreenSession> screenSession =
        new(std::nothrow) ScreenSession(config, ScreenSessionReason::CREATE_SESSION_WITHOUT_DISPLAY_NODE);
    sptr<SupportedScreenModes> info = new(std::nothrow) SupportedScreenModes();
    if (screenSession == nullptr || info == nullptr) {
        TLOGI(WmsLogTag::DMS, "new screenSession or info failed");
        screenIdManager_.DeleteScreenId(smsScreenId);
        rsInterface_.RemoveVirtualScreen(rsId);
        return nullptr;
    }
    info->width_ = option.width_;
    info->height_ = option.height_;
    auto defaultScreen = GetScreenSession(GetDefaultScreenId());
    if (defaultScreen != nullptr) {
        info->refreshRate_ = defaultScreen->GetRefreshRate();
        screenSession->UpdateRefreshRate(info->refreshRate_);
        std::vector<uint32_t> virtualRefreshRateVec = {info->refreshRate_};
        screenSession->SetSupportedRefreshRate(std::move(virtualRefreshRateVec));
    }
    screenSession->modes_.emplace_back(info);
    screenSession->activeIdx_ = 0;
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenSession->SetVirtualPixelRatio(option.density_);
    screenSession->SetIsPcUse(g_isPcDevice ? true : false);
    screenSession->SetDisplayBoundary(RectF(0, 0, option.width_, option.height_), 0);
    screenSession->RegisterScreenChangeListener(this);
    screenSession->SetValidWidth(option.width_);
    screenSession->SetValidHeight(option.height_);
    screenSession->SetRealWidth(option.width_);
    screenSession->SetRealHeight(option.height_);
    screenSession->SetScreenAreaWidth(option.width_);
    screenSession->SetScreenAreaHeight(option.height_);
    InitRotationCorrectionMap(DISPLAYMODE_CORRECTION);
    screenSession->SetRotationCorrectionMap(rotationCorrectionMap_);
    return screenSession;
}

bool ScreenSessionManager::InitAbstractScreenModesInfo(sptr<ScreenSession>& screenSession)
{
    TLOGW(WmsLogTag::DMS, "Call rsInterface_ GetScreenSupportedModes");
    std::vector<RSScreenModeInfo> allModes = rsInterface_.GetScreenSupportedModes(
        screenIdManager_.ConvertToRsScreenId(screenSession->screenId_));
    if (allModes.size() == 0) {
        TLOGE(WmsLogTag::DMS, "allModes.size() == 0, screenId=%{public}" PRIu64"", screenSession->rsId_);
        return false;
    }
    for (const RSScreenModeInfo& rsScreenModeInfo : allModes) {
        sptr<SupportedScreenModes> info = new(std::nothrow) SupportedScreenModes();
        if (info == nullptr) {
            TLOGE(WmsLogTag::DMS, "create SupportedScreenModes failed");
            return false;
        }
        info->id_ = static_cast<uint32_t>(rsScreenModeInfo.GetScreenModeId());
        info->width_ = static_cast<uint32_t>(rsScreenModeInfo.GetScreenWidth());
        info->height_ = static_cast<uint32_t>(rsScreenModeInfo.GetScreenHeight());
        info->refreshRate_ = rsScreenModeInfo.GetScreenRefreshRate();
        screenSession->modes_.push_back(info);
        TLOGW(WmsLogTag::DMS, "fill screen idx:%{public}d w/h:%{public}d/%{public}d/%{public}u",
            rsScreenModeInfo.GetScreenModeId(), info->width_, info->height_, info->refreshRate_);
    }
    TLOGW(WmsLogTag::DMS, "Call rsInterface_ GetScreenActiveMode");
    int32_t activeModeId = rsInterface_.GetScreenActiveMode(screenSession->rsId_).GetScreenModeId();
    TLOGW(WmsLogTag::DMS, "fill screen activeModeId:%{public}d", activeModeId);
    if (static_cast<std::size_t>(activeModeId) >= allModes.size()) {
        TLOGE(WmsLogTag::DMS, "activeModeId exceed, screenId=%{public}" PRIu64", activeModeId:%{public}d/%{public}ud",
            screenSession->rsId_, activeModeId, static_cast<uint32_t>(allModes.size()));
        return false;
    }
    screenSession->activeIdx_ = activeModeId;
    return true;
}

sptr<ScreenSession> ScreenSessionManager::InitAndGetScreen(ScreenId rsScreenId)
{
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    ScreenId smsScreenId = screenIdManager_.CreateAndGetNewScreenId(rsScreenId);
    RSScreenCapability screenCapability = rsInterface_.GetScreenCapability(rsScreenId);
    TLOGI(WmsLogTag::DMS, "Screen name is %{public}s, phyWidth is %{public}u, phyHeight is %{public}u",
        screenCapability.GetName().c_str(), screenCapability.GetPhyWidth(), screenCapability.GetPhyHeight());
    ScreenSessionConfig config = {
        .screenId = smsScreenId,
        .rsId = rsScreenId,
        .defaultScreenId = GetDefaultScreenId(),
        .name = screenCapability.GetName(),
    };
    sptr<ScreenSession> screenSession =
        new(std::nothrow) ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_VIRTUAL);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession == nullptr.");
        screenIdManager_.DeleteScreenId(smsScreenId);
        return nullptr;
    }
    if (!InitAbstractScreenModesInfo(screenSession)) {
        screenIdManager_.DeleteScreenId(smsScreenId);
        TLOGE(WmsLogTag::DMS, "failed.");
        return nullptr;
    }
    TLOGI(WmsLogTag::DMS, "screenSessionMap_ add screenId=%{public}" PRIu64"", smsScreenId);
    screenSessionMap_.insert(std::make_pair(smsScreenId, screenSession));
    return screenSession;
}

bool ScreenSessionManager::IsExtendMode()
{
    std::vector<ScreenId> mainVector;
    std::vector<ScreenId> extendVector;
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (const auto& pair : screenSessionMap_) {
        sptr<ScreenSession> session = pair.second;
        if (!session) {
            TLOGE(WmsLogTag::DMS, "screenId=%{public}" PRIu64", session is null", pair.first);
            continue;
        }
        if (session->GetScreenCombination() == ScreenCombination::SCREEN_EXTEND) {
            extendVector.push_back(session->GetScreenId());
        }
        if (session->GetScreenCombination() == ScreenCombination::SCREEN_MAIN) {
            mainVector.push_back(session->GetScreenId());
        }
    }
    std::ostringstream oss;
    oss << "main screenId:";
    for (const auto& screenId : mainVector) {
        oss << static_cast<uint64_t>(screenId);
    }
    oss << ", extend screenId:";
    for (const auto& screenId : extendVector) {
        oss << static_cast<uint64_t>(screenId);
    }
    oss << std::endl;
    TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    return mainVector.size() > 0 && extendVector.size() > 0;
}

sptr<ScreenSessionGroup> ScreenSessionManager::AddToGroupLocked(sptr<ScreenSession> newScreen, bool isUnique)
{
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    sptr<ScreenSessionGroup> res;
    if (smsScreenGroupMap_.empty()) {
        TLOGI(WmsLogTag::DMS, "connect the first screen");
        res = AddAsFirstScreenLocked(newScreen, isUnique);
    } else {
        res = AddAsSuccedentScreenLocked(newScreen);
    }
    return res;
}

sptr<ScreenSessionGroup> ScreenSessionManager::AddAsFirstScreenLocked(sptr<ScreenSession> newScreen, bool isUnique)
{
    ScreenId smsGroupScreenId(1);
    std::ostringstream buffer;
    buffer << "ScreenGroup_" << smsGroupScreenId;
    std::string name = buffer.str();
    // default ScreenCombination is mirror
    isExpandCombination_ = system::GetParameter("persist.display.expand.enabled", "0") == "1";
    sptr<ScreenSessionGroup> screenGroup;
    if (isExpandCombination_) {
        screenGroup = new(std::nothrow) ScreenSessionGroup(smsGroupScreenId,
            SCREEN_ID_INVALID, name, ScreenCombination::SCREEN_EXPAND);
    } else if (isUnique) {
        screenGroup = new(std::nothrow) ScreenSessionGroup(smsGroupScreenId,
            SCREEN_ID_INVALID, name, ScreenCombination::SCREEN_UNIQUE);
    } else {
        screenGroup = new(std::nothrow) ScreenSessionGroup(smsGroupScreenId,
            SCREEN_ID_INVALID, name, ScreenCombination::SCREEN_MIRROR);
    }
    if (screenGroup == nullptr) {
        TLOGE(WmsLogTag::DMS, "new ScreenSessionGroup failed");
        screenIdManager_.DeleteScreenId(smsGroupScreenId);
        return nullptr;
    }
    screenGroup->groupSmsId_ = 1;
    Point point;
    if (g_isPcDevice) {
        if (IsExtendMode()) {
            point = {newScreen->GetScreenProperty().GetStartX(), newScreen->GetScreenProperty().GetStartY()};
        }
    }
    if (!screenGroup->AddChild(newScreen, point, GetScreenSession(GetDefaultScreenId()), g_isPcDevice)) {
        TLOGE(WmsLogTag::DMS, "fail to add screen to group. screen=%{public}" PRIu64"", newScreen->screenId_);
        screenIdManager_.DeleteScreenId(smsGroupScreenId);
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    auto iter = smsScreenGroupMap_.find(smsGroupScreenId);
    if (iter != smsScreenGroupMap_.end()) {
        TLOGE(WmsLogTag::DMS, "group screen existed. id=%{public}" PRIu64"", smsGroupScreenId);
        smsScreenGroupMap_.erase(iter);
    }
    smsScreenGroupMap_.insert(std::make_pair(smsGroupScreenId, screenGroup));
    screenGroup->mirrorScreenId_ = newScreen->screenId_;
    TLOGI(WmsLogTag::DMS, "connect new group screen, screenId: %{public}" PRIu64", screenGroupId: %{public}" PRIu64", "
        "combination:%{public}u", newScreen->screenId_, smsGroupScreenId,
        newScreen->GetScreenProperty().GetScreenType());
    return screenGroup;
}

sptr<ScreenSessionGroup> ScreenSessionManager::AddAsSuccedentScreenLocked(sptr<ScreenSession> newScreen)
{
    ScreenId defaultScreenId = GetDefaultScreenId();
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    auto iter = screenSessionMap_.find(defaultScreenId);
    if (iter == screenSessionMap_.end()) {
        TLOGE(WmsLogTag::DMS, "defaultScreenId:%{public}" PRIu64" is not in screenSessionMap_.",
            defaultScreenId);
        return nullptr;
    }
    auto screen = iter->second;
    auto screenGroupIter = smsScreenGroupMap_.find(screen->groupSmsId_);
    if (screenGroupIter == smsScreenGroupMap_.end()) {
        TLOGE(WmsLogTag::DMS, "groupSmsId:%{public}" PRIu64" is not in smsScreenGroupMap_.",
            screen->groupSmsId_);
        return nullptr;
    }
    auto screenGroup = screenGroupIter->second;
    Point point;
    if (screenGroup->combination_ == ScreenCombination::SCREEN_EXPAND) {
        point = {screen->GetActiveScreenMode()->width_, 0};
    }
    screenGroup->AddChild(newScreen, point, screen);
    return screenGroup;
}

sptr<ScreenSessionGroup> ScreenSessionManager::RemoveFromGroupLocked(sptr<ScreenSession> screen)
{
    TLOGI(WmsLogTag::DMS, "start");
    auto groupSmsId = screen->groupSmsId_;
    sptr<ScreenSessionGroup> screenGroup = GetAbstractScreenGroup(groupSmsId);
    if (!screenGroup) {
        TLOGE(WmsLogTag::DMS, "groupSmsId:%{public}" PRIu64"is not in smsScreenGroupMap_.",
            groupSmsId);
        return nullptr;
    }
    if (!RemoveChildFromGroup(screen, screenGroup)) {
        return nullptr;
    }
    return screenGroup;
}

bool ScreenSessionManager::RemoveChildFromGroup(sptr<ScreenSession> screen, sptr<ScreenSessionGroup> screenGroup)
{
    auto rsUIContext = screen ? screen->GetRSUIContext() : nullptr;
    bool res = screenGroup->RemoveChild(screen);
    RSTransactionAdapter::FlushImplicitTransaction(rsUIContext);
    TLOGI(WmsLogTag::DMS, "remove child and call flush.");
    if (!res) {
        TLOGE(WmsLogTag::DMS, "remove screen:%{public}" PRIu64" failed from screenGroup:%{public}" PRIu64".",
            screen->screenId_, screen->groupSmsId_);
        return false;
    }
    if (screenGroup->GetChildCount() == 0) {
        // Group removed, need to do something.
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        smsScreenGroupMap_.erase(screenGroup->screenId_);
        screenSessionMap_.erase(screenGroup->screenId_);
        TLOGE(WmsLogTag::DMS, "screenSessionMap_ remove screen:%{public}" PRIu64, screenGroup->screenId_);
    }
    return true;
}

DMError ScreenSessionManager::SetMirror(ScreenId screenId, std::vector<ScreenId> screens, DMRect mainScreenRegion,
    const RotationOption& rotationOption, bool forceMirror)
{
    TLOGI(WmsLogTag::DMS, "screenId:%{public}" PRIu64"", screenId);
    sptr<ScreenSession> screen = GetScreenSession(screenId);
    if (screen == nullptr || screen->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
        TLOGE(WmsLogTag::DMS, "screen is nullptr, or screenCommbination is mirror.");
        return DMError::DM_ERROR_NULLPTR;
    }
    screen->groupSmsId_ = 1;
    auto group = GetAbstractScreenGroup(screen->groupSmsId_);
    if (group == nullptr) {
        group = AddToGroupLocked(screen);
        if (group == nullptr) {
            TLOGE(WmsLogTag::DMS, "group is nullptr");
            return DMError::DM_ERROR_NULLPTR;
        }
        NotifyScreenGroupChanged(screen->ConvertToScreenInfo(), ScreenGroupChangeEvent::ADD_TO_GROUP);
    }
    Point point;
    std::vector<Point> startPoints;
    startPoints.insert(startPoints.begin(), screens.size(), point);
    bool filterMirroredScreen =
        group->combination_ == ScreenCombination::SCREEN_MIRROR && group->mirrorScreenId_ == screen->screenId_
            && !forceMirror;
    group->mirrorScreenId_ = screen->screenId_;
    ChangeScreenGroup(group, screens, startPoints, filterMirroredScreen, ScreenCombination::SCREEN_MIRROR,
        mainScreenRegion, rotationOption);
    TLOGI(WmsLogTag::DMS, "success");
    return DMError::DM_OK;
}

sptr<ScreenSessionGroup> ScreenSessionManager::GetAbstractScreenGroup(ScreenId smsScreenId)
{
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    auto iter = smsScreenGroupMap_.find(smsScreenId);
    if (iter == smsScreenGroupMap_.end()) {
        TLOGE(WmsLogTag::DMS, "did not find screen:%{public}" PRIu64"", smsScreenId);
        return nullptr;
    }
    return iter->second;
}

bool ScreenSessionManager::CheckScreenInScreenGroup(sptr<ScreenSession> screen) const
{
    if (screen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen is nullptr");
        return false;
    }
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    auto groupSmsId = screen->groupSmsId_;
    auto iter = smsScreenGroupMap_.find(groupSmsId);
    if (iter == smsScreenGroupMap_.end()) {
        TLOGE(WmsLogTag::DMS, "groupSmsId:%{public}" PRIu64"is not in smsScreenGroupMap_.", groupSmsId);
        return false;
    }
    sptr<ScreenSessionGroup> screenGroup = iter->second;
    return screenGroup->HasChild(screen->screenId_);
}

void ScreenSessionManager::ChangeScreenGroup(sptr<ScreenSessionGroup> group, const std::vector<ScreenId>& screens,
    const std::vector<Point>& startPoints, bool filterScreen, ScreenCombination combination, DMRect mainScreenRegion,
    const RotationOption& rotationOption)
{
    std::map<ScreenId, bool> removeChildResMap;
    std::vector<ScreenId> addScreens;
    std::vector<Point> addChildPos;
    for (uint64_t i = 0; i != screens.size(); i++) {
        ScreenId screenId = screens[i];
        TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64"", screenId);
        auto screen = GetScreenSession(screenId);
        if (screen == nullptr) {
            TLOGE(WmsLogTag::DMS, "screen:%{public}" PRIu64" is nullptr", screenId);
            continue;
        }
        TLOGI(WmsLogTag::DMS, "Screen->groupSmsId_: %{public}" PRIu64"", screen->groupSmsId_);
        screen->groupSmsId_ = 1;
        if (!HasSameScreenCastInfo(screen->GetScreenId(), group->mirrorScreenId_, combination)) {
            TLOGI(WmsLogTag::DMS, "has not same cast info");
            filterScreen = false;
        }
        if (filterScreen && screen->groupSmsId_ == group->screenId_ && group->HasChild(screen->screenId_)) {
            // screen already in group
            if (combination != ScreenCombination::SCREEN_MIRROR ||
                (screen->GetMirrorScreenRegion().second == mainScreenRegion &&
                screen->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR)) {
                continue;
            }
            // mirror mode and mirror area change
            TLOGI(WmsLogTag::DMS, "Screen: %{public}" PRIu64
                ", apply new region, x:%{public}d y:%{public}d w:%{public}u h:%{public}u", screenId,
                mainScreenRegion.posX_, mainScreenRegion.posY_, mainScreenRegion.width_, mainScreenRegion.height_);
        }
        if (CheckScreenInScreenGroup(screen)) {
            NotifyDisplayDestroy(screenId);
        }
        auto originGroup = RemoveFromGroupLocked(screen);
        addChildPos.emplace_back(startPoints[i]);
        removeChildResMap[screenId] = originGroup != nullptr;
        addScreens.emplace_back(screenId);
        if (combination == ScreenCombination::SCREEN_MIRROR) {
            ChangeMirrorScreenConfig(group, mainScreenRegion, screen);
        }
        NotifyScreenChanged(screen->ConvertToScreenInfo(), ScreenChangeEvent::SCREEN_SOURCE_MODE_CHANGE);
        NotifyDisplayChanged(screen->ConvertToDisplayInfo(), DisplayChangeEvent::SOURCE_MODE_CHANGED);
        SetScreenCastInfo(screen->GetScreenId(), group->mirrorScreenId_, combination);
    }
    group->combination_ = combination;
    AddScreenToGroup(group, addScreens, addChildPos, removeChildResMap, rotationOption);
}

void ScreenSessionManager::ChangeMirrorScreenConfig(const sptr<ScreenSessionGroup>& group,
        const DMRect& mainScreenRegion, sptr<ScreenSession>& screen)
{
    if (screen == nullptr) {
        return;
    }
    auto screenId = screen->GetScreenId();
    if (group == nullptr) {
        return;
    }
    auto mirrorScreenId = group->mirrorScreenId_;
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    if (!ConvertScreenIdToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "Screen: %{public}" PRIu64" convert to rs id failed", mirrorScreenId);
        return;
    }
    screen->SetMirrorScreenRegion(rsScreenId, mainScreenRegion);
    screen->SetIsPhysicalMirrorSwitch(false);
    IsEnableRegionRotation(screen);
    TLOGI(WmsLogTag::DMS, "Screen: %{public}" PRIu64" mirror to %{public}"
        PRIu64" with region, x:%{public}d y:%{public}d w:%{public}u h:%{public}u",
        screenId, mirrorScreenId, mainScreenRegion.posX_, mainScreenRegion.posY_,
        mainScreenRegion.width_, mainScreenRegion.height_);
}

bool ScreenSessionManager::HasSameScreenCastInfo(ScreenId screenId,
    ScreenId castScreenId, ScreenCombination screenCombination)
{
    std::shared_lock<std::shared_mutex> lock(screenCastInfoMapMutex_);
    auto iter = screenCastInfoMap_.find(screenId);
    if (iter != screenCastInfoMap_.end() && iter->second.first == castScreenId &&
        iter->second.second == screenCombination) {
        return true;
    }
    return false;
}

void ScreenSessionManager::SetScreenCastInfo(ScreenId screenId,
    ScreenId castScreenId, ScreenCombination screenCombination)
{
    std::unique_lock<std::shared_mutex> lock(screenCastInfoMapMutex_);
    screenCastInfoMap_.insert(std::make_pair(screenId, std::make_pair(castScreenId, screenCombination)));
    TLOGI(WmsLogTag::DMS,
        "screenId:%{public}" PRIu64 ",castScreenId:%{public}" PRIu64 ",screenCombination:%{public}d",
        screenId, castScreenId, screenCombination);
}

void ScreenSessionManager::RemoveScreenCastInfo(ScreenId screenId)
{
    std::unique_lock<std::shared_mutex> lock(screenCastInfoMapMutex_);
    auto iter = screenCastInfoMap_.find(screenId);
    if (iter == screenCastInfoMap_.end()) {
        return;
    }
    screenCastInfoMap_.erase(iter);
    TLOGI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 "", screenId);
}

void ScreenSessionManager::IsEnableRegionRotation(sptr<ScreenSession> screenSession)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        screenSession->SetIsEnableRegionRotation(false);
        TLOGI(WmsLogTag::DMS, "Region rotation is not supported");
        return;
    }
    if (screenSession->GetName() == "screen_capture_file" || screenSession->GetName() == "screen_capture") {
        screenSession->SetIsEnableRegionRotation(false);
    } else {
        screenSession->SetIsEnableRegionRotation(true);
    }
    TLOGI(WmsLogTag::DMS, "Screen session name: %{public}s", screenSession->GetName().c_str());
}

void ScreenSessionManager::AddScreenToGroup(sptr<ScreenSessionGroup> group,
    const std::vector<ScreenId>& addScreens, const std::vector<Point>& addChildPos,
    std::map<ScreenId, bool>& removeChildResMap, const RotationOption& rotationOption)
{
    std::vector<sptr<ScreenInfo>> addToGroup;
    std::vector<sptr<ScreenInfo>> removeFromGroup;
    std::vector<sptr<ScreenInfo>> changeGroup;
    for (uint64_t i = 0; i != addScreens.size(); i++) {
        ScreenId screenId = addScreens[i];
        sptr<ScreenSession> screen = GetScreenSession(screenId);
        if (screen == nullptr) {
            continue;
        }
        Point expandPoint = addChildPos[i];
        TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64", Point: %{public}d, %{public}d",
            screen->screenId_, expandPoint.posX_, expandPoint.posY_);
        bool addChildRes = group->AddChild(screen, expandPoint, GetScreenSession(group->mirrorScreenId_),
            false, rotationOption);
        if (removeChildResMap[screenId] && addChildRes) {
            changeGroup.emplace_back(screen->ConvertToScreenInfo());
            TLOGD(WmsLogTag::DMS, "changeGroup");
        } else if (removeChildResMap[screenId]) {
            TLOGD(WmsLogTag::DMS, "removeChild");
            removeFromGroup.emplace_back(screen->ConvertToScreenInfo());
        } else if (addChildRes) {
            TLOGD(WmsLogTag::DMS, "AddChild");
            addToGroup.emplace_back(screen->ConvertToScreenInfo());
        } else {
            TLOGD(WmsLogTag::DMS, "default, AddChild failed");
        }
        NotifyDisplayCreate(screen->ConvertToDisplayInfo());
    }

    NotifyScreenGroupChanged(removeFromGroup, ScreenGroupChangeEvent::REMOVE_FROM_GROUP);
    NotifyScreenGroupChanged(changeGroup, ScreenGroupChangeEvent::CHANGE_GROUP);
    NotifyScreenGroupChanged(addToGroup, ScreenGroupChangeEvent::ADD_TO_GROUP);
}

void ScreenSessionManager::RemoveVirtualScreenFromGroup(std::vector<ScreenId> screens)
{
    TLOGW(WmsLogTag::DMS, "enter!");
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    if (screens.empty()) {
        return;
    }
    std::vector<sptr<ScreenInfo>> removeFromGroup;
    for (ScreenId screenId : screens) {
        auto screen = GetScreenSession(screenId);
        if (screen == nullptr || screen->GetScreenProperty().GetScreenType() != ScreenType::VIRTUAL) {
            continue;
        }
        auto originGroup = GetAbstractScreenGroup(screen->groupSmsId_);
        if (originGroup == nullptr) {
            continue;
        }
        if (!originGroup->HasChild(screenId)) {
            continue;
        }
        RemoveFromGroupLocked(screen);
        removeFromGroup.emplace_back(screen->ConvertToScreenInfo());
    }
    NotifyScreenGroupChanged(removeFromGroup, ScreenGroupChangeEvent::REMOVE_FROM_GROUP);
}

const std::shared_ptr<RSDisplayNode> ScreenSessionManager::GetRSDisplayNodeByScreenId(ScreenId smsScreenId) const
{
    static std::shared_ptr<RSDisplayNode> notFound = nullptr;
    sptr<ScreenSession> screen = GetScreenSession(smsScreenId);
    if (screen == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen == nullptr!");
        return notFound;
    }
    if (screen->GetDisplayNode() == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode_ == nullptr!");
        return notFound;
    }
    TLOGI(WmsLogTag::DMS, "screen: %{public}" PRIu64", nodeId: %{public}" PRIu64" ",
        screen->screenId_, screen->GetDisplayNode()->GetId());
    return screen->GetDisplayNode();
}

std::shared_ptr<RSDisplayNode> ScreenSessionManager::GetDisplayNodeByDisplayId(DisplayId displayId)
{
    std::shared_ptr<RSDisplayNode> displayNode = nullptr;
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
            continue;
        }
        sptr<DisplayInfo> displayInfo = screenSession->ConvertToDisplayInfo();
        if (displayInfo == nullptr) {
            TLOGE(WmsLogTag::DMS, "displayInfo is nullptr!");
            continue;
        }
        if (displayId == displayInfo->GetDisplayId()) {
            if (sessionIt.first == SCREEN_ID_INVALID) {
                TLOGE(WmsLogTag::DMS, "screenId is invalid!");
                continue;
            }
            displayNode = screenSession->GetDisplayNode();
            break;
        }
    }
    return displayNode;
}

bool ScreenSessionManager::IsFakeDisplayExist()
{
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() &&
        SuperFoldPolicy::GetInstance().IsFakeDisplayExist()) {
        return true;
    }
    return false;
}

std::shared_ptr<Media::PixelMap> ScreenSessionManager::GetScreenSnapshot(DisplayId displayId, bool isUseDma,
    bool isCaptureFullOfScreen, const std::vector<NodeId>& surfaceNodesList, SnapshotScaleInfo scaleInfo)
{
    DisplayId realDisplayId = displayId;
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && displayId == DISPLAY_ID_FAKE) {
        if (!SuperFoldPolicy::GetInstance().IsFakeDisplayExist()) {
            TLOGE(WmsLogTag::DMS, "fake display is not exist!");
            return nullptr;
        }
        realDisplayId = 0;
    }
    TLOGW(WmsLogTag::DMS, "dma=%{public}d, displayId:%{public}" PRIu64, isUseDma, realDisplayId);
    std::shared_ptr<RSDisplayNode> displayNode = GetDisplayNodeByDisplayId(realDisplayId);
    if (displayNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode is null!");
        return nullptr;
    }
    std::shared_ptr<SurfaceCaptureFuture> callback = std::make_shared<SurfaceCaptureFuture>();
    RSSurfaceCaptureConfig config;
    config.isHdrCapture = false;
    config.useDma = isUseDma;
    config.scaleX = scaleInfo.scaleX;
    config.scaleY = scaleInfo.scaleY;
    config.mainScreenRect = scaleInfo.rect;
#ifdef FOLD_ABILITY_ENABLE
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() &&
        SuperFoldPolicy::GetInstance().IsNeedSetSnapshotRect(displayId)) {
        config.mainScreenRect = SuperFoldPolicy::GetInstance().GetSnapshotRect(displayId, isCaptureFullOfScreen);
    }
#endif
    config.blackList = surfaceNodesList;
    if (surfaceNodesList.size() > 0) {
        TLOGI(WmsLogTag::DMS, "Snapshot filtering surfaceNodesList surfaceNodes, size:%{public}ud",
            static_cast<uint32_t>(surfaceNodesList.size()));
    }
    bool ret = rsInterface_.TakeSurfaceCapture(displayNode, callback, config);
    if (!ret) {
        TLOGE(WmsLogTag::DMS, "TakeSurfaceCapture failed");
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> screenshot = callback->GetResult(GET_HDR_PIXELMAP_TIMEOUT);  // wait for <= 2000ms
    if (screenshot == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get pixelmap from RS, return nullptr!");
    } else {
        TLOGD(WmsLogTag::DMS, "Sucess to get pixelmap from RS!");
    }
    // notify dm listener
    sptr<ScreenshotInfo> snapshotInfo = new ScreenshotInfo();
    snapshotInfo->SetTrigger(SysCapUtil::GetClientName());
    snapshotInfo->SetDisplayId(displayId);
    OnScreenshot(snapshotInfo);
    return screenshot;
}

std::vector<std::shared_ptr<Media::PixelMap>> ScreenSessionManager::GetScreenHDRSnapshot(
    DisplayId displayId, bool isUseDma, bool isCaptureFullOfScreen, const std::vector<NodeId>& surfaceNodesList)
{
    DisplayId realDisplayId = displayId;
#ifdef FOLD_ABILITY_ENABLE
    static bool isSuperFoldDisplayDevice = FoldScreenStateInternel::IsSuperFoldDisplayDevice();
    if (isSuperFoldDisplayDevice && displayId == DISPLAY_ID_FAKE) {
        if (!SuperFoldPolicy::GetInstance().IsFakeDisplayExist()) {
            TLOGE(WmsLogTag::DMS, "fake display is not exist!");
            return {nullptr, nullptr};
        }
        realDisplayId = 0;
    }
#endif
    std::shared_ptr<RSDisplayNode> displayNode = GetDisplayNodeByDisplayId(realDisplayId);
    if (displayNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode is null!");
        return {nullptr, nullptr};
    }
    std::shared_ptr<SurfaceCaptureFuture> callback = std::make_shared<SurfaceCaptureFuture>();
    RSSurfaceCaptureConfig config;
    config.isHdrCapture = true;
    config.useDma = isUseDma;
    TLOGI(WmsLogTag::DMS, "take surface capture with dma=%{public}d", isUseDma);
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ != nullptr && FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        config.mainScreenRect = foldScreenController_->GetScreenSnapshotRect();
    }
    if (isSuperFoldDisplayDevice && SuperFoldPolicy::GetInstance().IsNeedSetSnapshotRect(displayId)) {
        config.mainScreenRect = SuperFoldPolicy::GetInstance().GetSnapshotRect(displayId, isCaptureFullOfScreen);
    }
#endif
    config.blackList = surfaceNodesList;
    if (surfaceNodesList.size() > 0) {
        TLOGI(WmsLogTag::DMS, "Snapshot filter, size:%{public}ud", static_cast<uint32_t>(surfaceNodesList.size()));
    }
    bool ret = rsInterface_.TakeSurfaceCapture(displayNode, callback, config);
    if (!ret) {
        TLOGE(WmsLogTag::DMS, "TakeSurfaceCapture failed");
        return {nullptr, nullptr};
    }
    // wait for <= 2000ms
    std::vector<std::shared_ptr<Media::PixelMap>> screenshotVec = callback->GetHDRResult(GET_HDR_PIXELMAP_TIMEOUT);
    if (screenshotVec.size() != PIXMAP_VECTOR_SIZE || screenshotVec[0] == nullptr) {
        TLOGE(WmsLogTag::DMS, "Failed to get pixelmap vector from RS, return nullptr!");
        return {nullptr, nullptr};
    }
    // notify dm listener
    sptr<ScreenshotInfo> snapshotInfo = new ScreenshotInfo();
    snapshotInfo->SetTrigger(SysCapUtil::GetClientName());
    snapshotInfo->SetDisplayId(displayId);
    OnScreenshot(snapshotInfo);
    return screenshotVec;
}

std::shared_ptr<Media::PixelMap> ScreenSessionManager::GetDisplaySnapshot(DisplayId displayId,
    DmErrorCode* errorCode, bool isUseDma, bool isCaptureFullOfScreen)
{
    TLOGD(WmsLogTag::DMS, "enter!");
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsShellCall() && errorCode) {
        *errorCode = DmErrorCode::DM_ERROR_NOT_SYSTEM_APP;
        return nullptr;
    }
    if (system::GetBoolParameter("persist.edm.disallow_screenshot", false) && errorCode) {
        TLOGI(WmsLogTag::DMS, "snapshot disabled by edm!");
        *errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
        return nullptr;
    }
    if (displayId == DISPLAY_ID_FAKE && !IsFakeDisplayExist() && errorCode) {
        *errorCode = DmErrorCode::DM_ERROR_INVALID_SCREEN;
        TLOGE(WmsLogTag::DMS, "fake display not exist!");
        return nullptr;
    }
    if ((Permission::IsSystemCalling() && (Permission::CheckCallingPermission(SCREEN_CAPTURE_PERMISSION) ||
        Permission::CheckCallingPermission(CUSTOM_SCREEN_RECORDING_PERMISSION))) || SessionPermission::IsShellCall()) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetDisplaySnapshot(%" PRIu64")", displayId);
        auto res = GetScreenSnapshot(displayId, isUseDma, isCaptureFullOfScreen);
        if (res != nullptr) {
            NotifyScreenshot(displayId);
            if (SessionPermission::IsBetaVersion()) {
                CheckAndSendHiSysEvent("GET_DISPLAY_SNAPSHOT", "hmos.screenshot");
            }
        }
        isScreenShot_ = true;
        NotifyCaptureStatusChanged();
        return res;
    } else if (errorCode) {
        *errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Media::PixelMap>> ScreenSessionManager::GetDisplayHDRSnapshot(DisplayId displayId,
    DmErrorCode& errorCode, bool isUseDma, bool isCaptureFullOfScreen)
{
    TLOGI(WmsLogTag::DMS, "enter!");
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsShellCall()) {
        errorCode = DmErrorCode::DM_ERROR_NOT_SYSTEM_APP;
        return {nullptr, nullptr};
    }
    if (system::GetBoolParameter("persist.edm.disallow_screenshot", false)) {
        TLOGE(WmsLogTag::DMS, "snapshot disabled by edm!");
        errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
        return {nullptr, nullptr};
    }
    if (displayId == DISPLAY_ID_FAKE && !IsFakeDisplayExist()) {
        TLOGE(WmsLogTag::DMS, "fake display not exist!");
        errorCode = DmErrorCode::DM_ERROR_INVALID_SCREEN;
        return {nullptr, nullptr};
    }
    if ((Permission::IsSystemCalling() && (Permission::CheckCallingPermission(SCREEN_CAPTURE_PERMISSION) ||
        Permission::CheckCallingPermission(CUSTOM_SCREEN_RECORDING_PERMISSION))) || SessionPermission::IsShellCall()) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetDisplayHDRSnapshot(%" PRIu64")", displayId);
        std::vector<std::shared_ptr<Media::PixelMap>> res = GetScreenHDRSnapshot(
            displayId, isUseDma, isCaptureFullOfScreen);
        if (res.size() == PIXMAP_VECTOR_SIZE && res[SDR_PIXMAP] != nullptr) {
            NotifyScreenshot(displayId);
            if (SessionPermission::IsBetaVersion()) {
                CheckAndSendHiSysEvent("GET_DISPLAY_SNAPSHOT", "hmos.screenshot");
            }
        } else {
            TLOGE(WmsLogTag::DMS, "GetScreenHDRSnapshot get PixelMap vector failed");
        }
        isScreenShot_ = true;
        NotifyCaptureStatusChanged();
        return res;
    } else {
        errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
    }
    return {nullptr, nullptr};
}

std::shared_ptr<Media::PixelMap> ScreenSessionManager::GetDisplaySnapshotWithOption(const CaptureOption& option,
    DmErrorCode* errorCode)
{
    TLOGD(WmsLogTag::DMS, "enter!");
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsShellCall() &&
        !SessionPermission::IsSACalling()) {
        if (errorCode != nullptr) {
            *errorCode = DmErrorCode::DM_ERROR_NOT_SYSTEM_APP;
        }
        return nullptr;
    }
    if (system::GetBoolParameter("persist.edm.disallow_screenshot", false)) {
        TLOGI(WmsLogTag::DMS, "snapshot was disabled by edm!");
        return nullptr;
    }
    if (option.displayId_ == DISPLAY_ID_FAKE && !IsFakeDisplayExist() && errorCode) {
        TLOGE(WmsLogTag::DMS, "fake display not exist!");
        *errorCode = DmErrorCode::DM_ERROR_INVALID_SCREEN;
        return nullptr;
    }
    if ((Permission::IsSystemCalling() && (Permission::CheckCallingPermission(SCREEN_CAPTURE_PERMISSION) ||
        Permission::CheckCallingPermission(CUSTOM_SCREEN_RECORDING_PERMISSION))) || SessionPermission::IsShellCall()) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetDisplaySnapshot(%" PRIu64")", option.displayId_);
        Drawing::Rect rect = { static_cast<float>(option.rect.posX_), static_cast<float>(option.rect.posY_),
            static_cast<float>(option.rect.posX_ + option.rect.width_),
            static_cast<float>(option.rect.posY_ + option.rect.height_) };
        SnapshotScaleInfo scaleInfo = {option.scaleX_, option.scaleY_, rect};
        auto res = GetScreenSnapshot(option.displayId_, true, option.isCaptureFullOfScreen_, option.surfaceNodesList_,
            scaleInfo);
        if (res != nullptr) {
            if (SessionPermission::IsBetaVersion()) {
                CheckAndSendHiSysEvent("GET_DISPLAY_SNAPSHOT", "hmos.screenshot");
            }
            TLOGI(WmsLogTag::DMS, "isNeedNotify_:%{public}d", option.isNeedNotify_);
            if (option.isNeedNotify_) {
                isScreenShot_ = true;
                NotifyScreenshot(option.displayId_);
                NotifyCaptureStatusChanged();
            }
        }
        return res;
    } else if (errorCode) {
        *errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Media::PixelMap>> ScreenSessionManager::GetDisplayHDRSnapshotWithOption(
    const CaptureOption& option, DmErrorCode& errorCode)
{
    TLOGD(WmsLogTag::DMS, "enter!");
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsShellCall() &&
        !SessionPermission::IsSACalling()) {
        errorCode = DmErrorCode::DM_ERROR_NOT_SYSTEM_APP;
        return {nullptr, nullptr};
    }
    if (system::GetBoolParameter("persist.edm.disallow_screenshot", false)) {
        TLOGE(WmsLogTag::DMS, "snapshot was disabled by edm!");
        errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
        return {nullptr, nullptr};
    }
    if (option.displayId_ == DISPLAY_ID_FAKE && !IsFakeDisplayExist()) {
        TLOGE(WmsLogTag::DMS, "fake display not exist!");
        errorCode = DmErrorCode::DM_ERROR_INVALID_SCREEN;
        return {nullptr, nullptr};
    }
    if ((Permission::IsSystemCalling() && (Permission::CheckCallingPermission(SCREEN_CAPTURE_PERMISSION) ||
        Permission::CheckCallingPermission(CUSTOM_SCREEN_RECORDING_PERMISSION))) || SessionPermission::IsShellCall()) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
            "ssm:GetDisplayHDRSnapshotWithOption(%" PRIu64")", option.displayId_);
        std::vector<std::shared_ptr<Media::PixelMap>> res = GetScreenHDRSnapshot(
            option.displayId_, true, option.isCaptureFullOfScreen_, option.surfaceNodesList_);
        if (res.size() == PIXMAP_VECTOR_SIZE && res[SDR_PIXMAP] != nullptr) {
            if (SessionPermission::IsBetaVersion()) {
                CheckAndSendHiSysEvent("GET_DISPLAY_SNAPSHOT", "hmos.screenshot");
            }
            TLOGI(WmsLogTag::DMS, "isNeedNotify_:%{public}d", option.isNeedNotify_);
            if (option.isNeedNotify_) {
                isScreenShot_ = true;
                NotifyScreenshot(option.displayId_);
                NotifyCaptureStatusChanged();
            }
        }
        return res;
    } else {
        errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
    }
    return {nullptr, nullptr};
}

std::shared_ptr<Media::PixelMap> ScreenSessionManager::GetSnapshotByPicker(Media::Rect &rect, DmErrorCode* errorCode)
{
    TLOGD(WmsLogTag::DMS, "ENTER!");
    *errorCode = DmErrorCode::DM_ERROR_SYSTEM_INNORMAL;
    std::lock_guard<std::mutex> lock(snapBypickerMutex_);

    if (system::GetBoolParameter("persist.edm.disallow_screenshot", false)) {
        *errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
        TLOGI(WmsLogTag::DMS, "snapshot was disabled by edm!");
        return nullptr;
    }
    ScreenId screenId = SCREEN_ID_INVALID;
    // get snapshot area frome Screenshot extension
    if (!GetSnapshotArea(rect, errorCode, screenId)) {
        return nullptr;
    }
    auto screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get screen session");
        return nullptr;
    }
    sptr<DisplayInfo> displayInfo = screenSession->ConvertToDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get default display");
        return nullptr;
    }
    DisplayId displayId = displayInfo->GetDisplayId();
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetSnapshotByPicker(%" PRIu64")", displayId);
    auto pixelMap = GetScreenSnapshot(displayId, false);
    if (pixelMap != nullptr && SessionPermission::IsBetaVersion()) {
        CheckAndSendHiSysEvent("GET_DISPLAY_SNAPSHOT", "hmos.screenshot");
    }
    isScreenShot_ = true;
    NotifyCaptureStatusChanged();
    *errorCode = DmErrorCode::DM_OK;
    return pixelMap;
}

bool ScreenSessionManager::GetSnapshotArea(Media::Rect &rect, DmErrorCode* errorCode, ScreenId &screenId)
{
    ConfigureScreenSnapshotParams();
    if (ScreenSnapshotPickerConnection::GetInstance().SnapshotPickerConnectExtension()) {
        int32_t ret = ScreenSnapshotPickerConnection::GetInstance().GetScreenSnapshotInfo(rect, screenId);
        if (ret != 0) {
            TLOGE(WmsLogTag::DMS, "GetScreenSnapshotInfo failed");
            ScreenSnapshotPickerConnection::GetInstance().SnapshotPickerDisconnectExtension();
            if (ret == RES_FAILURE_FOR_PRIVACY_WINDOW) {
                *errorCode = DmErrorCode::DM_ERROR_INVALID_CALLING;
            } else {
                *errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
            }
            return false;
        }
        ScreenSnapshotPickerConnection::GetInstance().SnapshotPickerDisconnectExtension();
    } else {
        *errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
        TLOGE(WmsLogTag::DMS, "SnapshotPickerConnectExtension failed");
        return false;
    }
    return true;
}

bool ScreenSessionManager::OnRemoteDied(const sptr<IRemoteObject>& agent)
{
    if (agent == nullptr) {
        return false;
    }
    std::vector<ScreenId> screenVecCopy;
    {
        std::lock_guard<std::mutex> lock(screenAgentMapMutex_);
        auto agentIter = screenAgentMap_.find(agent);
        if (agentIter != screenAgentMap_.end()) {
            screenVecCopy = agentIter->second;
        }
    }
    bool ret = true;
    for (const auto screenId : screenVecCopy) {
        auto screenSession = GetScreenSession(screenId);
        if (screenSession && screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
            TLOGI(WmsLogTag::DMS, "stop mirror in OnRemoteDied: %{public}" PRIu64, screenId);
            StopMirror(std::vector<ScreenId>(1, screenId));
        }
        TLOGI(WmsLogTag::DMS, "destroy screenId in OnRemoteDied: %{public}" PRIu64, screenId);
        DMError res = DestroyVirtualScreen(screenId);
        if (res != DMError::DM_OK) {
            TLOGE(WmsLogTag::DMS, "destroy failed in OnRemoteDied: %{public}" PRIu64, screenId);
            ret = false;
        }
    }
    return ret;
}

std::vector<ScreenId> ScreenSessionManager::GetAllValidScreenIds(const std::vector<ScreenId>& screenIds) const
{
    std::vector<ScreenId> validScreenIds;
    for (ScreenId screenId : screenIds) {
        auto screenIdIter = std::find(validScreenIds.begin(), validScreenIds.end(), screenId);
        if (screenIdIter != validScreenIds.end()) {
            continue;
        }
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        auto iter = screenSessionMap_.find(screenId);
        if (iter != screenSessionMap_.end() && iter->second != nullptr &&
                iter->second->GetScreenProperty().GetScreenType() != ScreenType::UNDEFINED) {
            validScreenIds.emplace_back(screenId);
        }
    }
    return validScreenIds;
}

sptr<ScreenGroupInfo> ScreenSessionManager::GetScreenGroupInfoById(ScreenId screenId)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return nullptr;
    }
    auto screenSessionGroup = GetAbstractScreenGroup(screenId);
    if (screenSessionGroup == nullptr) {
        TLOGE(WmsLogTag::DMS, "cannot find screenGroupInfo: %{public}" PRIu64"", screenId);
        return nullptr;
    }
    return screenSessionGroup->ConvertToScreenGroupInfo();
}

void ScreenSessionManager::NotifyScreenConnected(sptr<ScreenInfo> screenInfo)
{
    if (screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "error, screenInfo is nullptr.");
        return;
    }
    auto task = [=] {
        TLOGNI(WmsLogTag::DMS, "screenId:%{public}" PRIu64"", screenInfo->GetScreenId());
        OnScreenConnect(screenInfo);
    };
    taskScheduler_->PostAsyncTask(task, "NotifyScreenConnected");
}

void ScreenSessionManager::NotifyScreenDisconnected(ScreenId screenId)
{
    auto task = [=] {
        TLOGNI(WmsLogTag::DMS, "notify screenId:%{public}" PRIu64"", screenId);
        OnScreenDisconnect(screenId);
    };
    taskScheduler_->PostAsyncTask(task, "NotifyScreenDisconnected");
}

void ScreenSessionManager::NotifyDisplayCreate(sptr<DisplayInfo> displayInfo)
{
    ScreenSessionManagerAdapter::GetInstance().OnDisplayCreate(displayInfo);
}

void ScreenSessionManager::NotifyDisplayDestroy(DisplayId displayId)
{
    ScreenSessionManagerAdapter::GetInstance().OnDisplayDestroy(displayId);
}

void ScreenSessionManager::NotifyScreenGroupChanged(
    const sptr<ScreenInfo>& screenInfo, ScreenGroupChangeEvent event)
{
    if (screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenInfo is nullptr.");
        return;
    }
    std::string trigger = SysCapUtil::GetClientName();
    auto task = [=] {
        TLOGNI(WmsLogTag::DMS, "screenId:%{public}" PRIu64", trigger:[%{public}s]",
            screenInfo->GetScreenId(), trigger.c_str());
        OnScreenGroupChange(trigger, screenInfo, event);
    };
    taskScheduler_->PostAsyncTask(task, "NotifyScreenGroupChanged:PID");
}

void ScreenSessionManager::NotifyScreenGroupChanged(
    const std::vector<sptr<ScreenInfo>>& screenInfo, ScreenGroupChangeEvent event)
{
    if (screenInfo.empty()) {
        return;
    }
    std::string trigger = SysCapUtil::GetClientName();
    auto task = [=] {
        TLOGNI(WmsLogTag::DMS, "trigger:[%{public}s]", trigger.c_str());
        OnScreenGroupChange(trigger, screenInfo, event);
    };
    taskScheduler_->PostAsyncTask(task, "NotifyScreenGroupChanged");
}

void ScreenSessionManager::NotifyPrivateSessionStateChanged(bool hasPrivate)
{
    ScreenSessionManagerAdapter::GetInstance().NotifyPrivateWindowStateChanged(hasPrivate);
}

void ScreenSessionManager::SetScreenPrivacyState(bool hasPrivate)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    TLOGI(WmsLogTag::DMS, "enter, hasPrivate: %{public}d", hasPrivate);
    ScreenId id = GetDefaultScreenId();
    auto screenSession = GetScreenSession(id);
    hasPrivateWindowForeground_.clear();
    hasPrivateWindowForeground_[id] = hasPrivate;
    NotifyPrivateSessionStateChanged(hasPrivate);
}

void ScreenSessionManager::SetPrivacyStateByDisplayId(std::unordered_map<DisplayId, bool>& privacyBundleDisplayId)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    std::ostringstream oss; 
    for (const auto& [displayId, isPrivate] : privacyBundleDisplayId) {
        oss << "[displayId: " << displayId
            << ", private: " << (isPrivate ? "true" : "false") << "] ";
    }
    TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    const std::vector<DisplayId> displayIds = GetAllDisplayIds();
    // all display use or to calculate private state
    if (CheckNeedNotify(displayIds, privacyBundleDisplayId)) {
        bool allDisplayHasPrivate = false;
        {
            std::lock_guard<std::mutex> lock(hasPrivateWindowForegroundMutex_);
            allDisplayHasPrivate = std::any_of(hasPrivateWindowForeground_.begin(), hasPrivateWindowForeground_.end(),
                [](const auto& pair) { return pair.second; });
        }
        NotifyPrivateSessionStateChanged(allDisplayHasPrivate);
    }
}

bool ScreenSessionManager::CheckNeedNotify(const std::vector<DisplayId>& displayIds,
        std::unordered_map<DisplayId, bool>& privacyBundleDisplayId)
{
    bool isNeedNotify = false;
    std::lock_guard<std::mutex> lock(hasPrivateWindowForegroundMutex_);
    if (privacyBundleDisplayId.empty()) {
        TLOGE(WmsLogTag::DMS, "privacyBundleDisplayId is empty.");
        return isNeedNotify;
    }
    // determine whether the hasPrivate value corresponding to displayId in hasPrivateWindowForeground_
    // matches the status in privacyBundleDisplayId
    for (auto it = privacyBundleDisplayId.begin(); it != privacyBundleDisplayId.end();) {
        DisplayId displayId = it->first;
        bool hasPrivate = it->second;
        if (std::find(displayIds.begin(), displayIds.end(), displayId) == displayIds.end()) {
            TLOGW(WmsLogTag::DMS, "invalid displayId");
            it = privacyBundleDisplayId.erase(it);
            continue;
        }
        auto iter = hasPrivateWindowForeground_.find(displayId);
        if (iter != hasPrivateWindowForeground_.end() && iter->second != hasPrivate) {
            isNeedNotify = true;
            iter->second = hasPrivate;
        } else if (iter == hasPrivateWindowForeground_.end()) {
            isNeedNotify = true;
            hasPrivateWindowForeground_[displayId] = hasPrivate;
        }
        ++it;
    }
    // clear hasPrivateWindowForeground_ not include id that not in displayIds
    for (auto iter = hasPrivateWindowForeground_.begin(); iter != hasPrivateWindowForeground_.end();) {
        if (std::find(displayIds.begin(), displayIds.end(), iter->first) == displayIds.end()) {
            iter = hasPrivateWindowForeground_.erase(iter);
        } else {
            ++iter;
        }
    }
    return isNeedNotify;
}

void ScreenSessionManager::SetScreenPrivacyWindowList(DisplayId id, std::vector<std::string> privacyWindowList)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permmission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    TLOGW(WmsLogTag::DMS, "enter");
    std::vector<ScreenId> screenIds = GetAllScreenIds();
    auto iter = std::find(screenIds.begin(), screenIds.end(), id);
    if (iter == screenIds.end()) {
        TLOGE(WmsLogTag::DMS, "invalid displayId");
        return;
    }
    auto screenSession = GetScreenSession(id);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get id: %{public}" PRIu64" screen now", id);
        return;
    }
    NotifyPrivateWindowListChanged(id, privacyWindowList);
}

void ScreenSessionManager::NotifyPrivateWindowListChanged(DisplayId id, std::vector<std::string> privacyWindowList)
{
    ScreenSessionManagerAdapter::GetInstance().NotifyPrivateStateWindowListChanged(id, privacyWindowList);
}

DMError ScreenSessionManager::HasPrivateWindow(DisplayId id, bool& hasPrivateWindow)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permmision Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    std::vector<DisplayId> displayIds = GetAllDisplayIds();
    auto iter = std::find(displayIds.begin(), displayIds.end(), id);
    if (iter == displayIds.end()) {
        TLOGE(WmsLogTag::DMS, "invalid displayId");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    {
        std::lock_guard<std::mutex> lock(hasPrivateWindowForegroundMutex_);
        auto iter = hasPrivateWindowForeground_.find(id);
        if (iter != hasPrivateWindowForeground_.end()) {
            hasPrivateWindow = iter->second;
        } else {
            hasPrivateWindow = false;
        }
    }
    TLOGI(WmsLogTag::DMS, "id: %{public}" PRIu64" has private window: %{public}u",
        id, static_cast<uint32_t>(hasPrivateWindow));
    return DMError::DM_OK;
}

void ScreenSessionManager::OnScreenGroupChange(const std::string& trigger,
    const sptr<ScreenInfo>& screenInfo, ScreenGroupChangeEvent groupEvent)
{
    if (screenInfo == nullptr) {
        return;
    }
    std::vector<sptr<ScreenInfo>> screenInfos;
    screenInfos.push_back(screenInfo);
    OnScreenGroupChange(trigger, screenInfos, groupEvent);
}

void ScreenSessionManager::OnScreenGroupChange(const std::string& trigger,
    const std::vector<sptr<ScreenInfo>>& screenInfos, ScreenGroupChangeEvent groupEvent)
{
    ScreenSessionManagerAdapter::GetInstance().OnScreenGroupChange(trigger, screenInfos, groupEvent);
}

void ScreenSessionManager::OnScreenConnect(const sptr<ScreenInfo> screenInfo)
{
    ScreenSessionManagerAdapter::GetInstance().OnScreenConnect(screenInfo);
    NotifyScreenModeChange();
}

void ScreenSessionManager::OnScreenDisconnect(ScreenId screenId)
{
    ScreenSessionManagerAdapter::GetInstance().OnScreenDisconnect(screenId);
    NotifyScreenModeChange(screenId);
}

void ScreenSessionManager::OnScreenshot(sptr<ScreenshotInfo> info)
{
    ScreenSessionManagerAdapter::GetInstance().OnScreenshot(info);
}

sptr<CutoutInfo> ScreenSessionManager::GetCutoutInfo(DisplayId displayId)
{
    DmsXcollie dmsXcollie("DMS:GetCutoutInfo", XCOLLIE_TIMEOUT_10S);
    return screenCutoutController_ ? screenCutoutController_->GetScreenCutoutInfo(displayId) : nullptr;
}

sptr<CutoutInfo> ScreenSessionManager::GetCutoutInfo(DisplayId displayId, int32_t width,
                                                    int32_t height, Rotation rotation)
{
    DmsXcollie dmsXcollie("DMS:GetCutoutInfo", XCOLLIE_TIMEOUT_10S);
    return screenCutoutController_ ? screenCutoutController_->GetScreenCutoutInfo(displayId, width, height, rotation) :
            nullptr;
}

DMError ScreenSessionManager::HasImmersiveWindow(ScreenId screenId, bool& immersive)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    clientProxy->OnImmersiveStateChanged(screenId, immersive);
    return DMError::DM_OK;
}

std::string ScreenSessionManager::TransferTypeToString(ScreenType type) const
{
    std::string screenType;
    switch (type) {
        case ScreenType::REAL:
            screenType = "REAL";
            break;
        case ScreenType::VIRTUAL:
            screenType = "VIRTUAL";
            break;
        default:
            screenType = "UNDEFINED";
            break;
    }
    return screenType;
}

std::string ScreenSessionManager::TransferPropertyChangeTypeToString(ScreenPropertyChangeType type) const
{
    std::string screenType;
    switch (type) {
        case ScreenPropertyChangeType::UNSPECIFIED:
            screenType = "UNSPECIFIED";
            break;
        case ScreenPropertyChangeType::ROTATION_BEGIN:
            screenType = "ROTATION_BEGIN";
            break;
        case ScreenPropertyChangeType::ROTATION_END:
            screenType = "ROTATION_END";
            break;
        case ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY:
            screenType = "ROTATION_UPDATE_PROPERTY_ONLY";
            break;
        case ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY_NOT_NOTIFY:
            screenType = "ROTATION_UPDATE_PROPERTY_ONLY_NOT_NOTIFY";
            break;
        default:
            screenType = "UNDEFINED";
            break;
    }
    return screenType;
}

void ScreenSessionManager::DumpAllScreensInfo(std::string& dumpInfo)
{
    if (!(SessionPermission::IsSACalling() || SessionPermission::IsStartByHdcd())) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    std::ostringstream oss;
    oss << "--------------------------------------Free Screen"
        << "--------------------------------------"
        << std::endl;
    oss << "ScreenName           Type     IsGroup DmsId RsId                 ActiveIdx VPR Rotation Orientation "
        << "RequestOrientation NodeId               "
        << std::endl;
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            continue;
        }
        sptr<ScreenInfo> screenInfo = screenSession->ConvertToScreenInfo();
        if (screenInfo == nullptr) {
            continue;
        }
        std::string screenType = TransferTypeToString(screenInfo->GetType());
        NodeId nodeId = (screenSession->GetDisplayNode() == nullptr) ?
            SCREEN_ID_INVALID : screenSession->GetDisplayNode()->GetId();
        oss << std::left << std::setw(21) << screenInfo->GetName()  // 21 is width
            << std::left << std::setw(9) << screenType  // 9 is width
            << std::left << std::setw(8) << (screenSession->isScreenGroup_ ? "true" : "false")  // 8 is width
            << std::left << std::setw(6) << screenSession->screenId_  // 6 is width
            << std::left << std::setw(21) << screenSession->rsId_  // 21 is width
            << std::left << std::setw(10) << screenSession->activeIdx_  // 10 is width
            << std::left << std::setw(4) << screenInfo->GetVirtualPixelRatio()  // 4 is width
            << std::left << std::setw(9) << static_cast<uint32_t>(screenInfo->GetRotation())  // 9 is width
            << std::left << std::setw(12) << static_cast<uint32_t>(screenInfo->GetOrientation())  // 12 is width
            << std::left << std::setw(19)  // 19 is width
                << static_cast<uint32_t>(screenSession->GetScreenRequestedOrientation())
            << std::left << std::setw(21) << nodeId  // 21 is width
            << std::endl;
    }
    oss << "total screen num: " << screenSessionMap_.size() << std::endl;
    dumpInfo.append(oss.str());
}

void ScreenSessionManager::DumpSpecialScreenInfo(ScreenId id, std::string& dumpInfo)
{
    if (!(SessionPermission::IsSACalling() || SessionPermission::IsStartByHdcd())) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    std::ostringstream oss;
    sptr<ScreenSession> session = GetScreenSession(id);
    if (!session) {
        TLOGE(WmsLogTag::DMS, "Get screen session failed.");
        oss << "This screen id is invalid.";
        dumpInfo.append(oss.str());
        return;
    }
    sptr<ScreenInfo> screenInfo = GetScreenInfoById(id);
    if (screenInfo == nullptr) {
        return;
    }
    std::string screenType = TransferTypeToString(screenInfo->GetType());
    NodeId nodeId = (session->GetDisplayNode() == nullptr) ?
        SCREEN_ID_INVALID : session->GetDisplayNode()->GetId();
    oss << "ScreenName: " << screenInfo->GetName() << std::endl;
    oss << "Type: " << screenType << std::endl;
    oss << "IsGroup: " << (session->isScreenGroup_ ? "true" : "false") << std::endl;
    oss << "DmsId: " << id << std::endl;
    oss << "RsId: " << session->rsId_ << std::endl;
    oss << "ActiveIdx: " << session->activeIdx_ << std::endl;
    oss << "VPR: " << screenInfo->GetVirtualPixelRatio() << std::endl;
    oss << "Rotation: " << static_cast<uint32_t>(screenInfo->GetRotation()) << std::endl;
    oss << "Orientation: " << static_cast<uint32_t>(screenInfo->GetOrientation()) << std::endl;
    oss << "RequestOrientation: " << static_cast<uint32_t>(session->GetScreenRequestedOrientation()) << std::endl;
    oss << "NodeId: " << nodeId << std::endl;
    dumpInfo.append(oss.str());
}

// --- Fold Screen ---
ScreenProperty ScreenSessionManager::GetPhyScreenProperty(ScreenId screenId)
{
    std::lock_guard<std::recursive_mutex> lock_phy(phyScreenPropMapMutex_);
    ScreenProperty property;
    auto iter = phyScreenPropMap_.find(screenId);
    if (iter == phyScreenPropMap_.end()) {
        TLOGI(WmsLogTag::DMS, "Error found physic screen config with id: %{public}" PRIu64, screenId);
        return property;
    }
    return iter->second;
}

void ScreenSessionManager::UpdateCameraBackSelfie(bool isCameraBackSelfie)
{
    if (isCameraBackSelfie_ == isCameraBackSelfie) {
        return;
    }
    isCameraBackSelfie_ = isCameraBackSelfie;

    auto screenSession = GetDefaultScreenSession();
    if (!screenSession) {
        TLOGW(WmsLogTag::DMS, "screenSession is null, notify camera back selfie failed");
        return;
    }
    screenSession->HandleCameraBackSelfieChange(isCameraBackSelfie);

    if (isCameraBackSelfie) {
        TLOGI(WmsLogTag::DMS, "isBackSelfie, SetScreenCorrection MAIN to 270");
        rsInterface_.SetScreenCorrection(SCREEN_ID_MAIN, static_cast<ScreenRotation>(ROTATION_270));
    } else {
        TLOGI(WmsLogTag::DMS, "exit BackSelfie, SetScreenCorrection MAIN to 90");
        SetScreenCorrection();
    }
}

void ScreenSessionManager::SetFoldDisplayMode(const FoldDisplayMode displayMode)
{
    SetFoldDisplayModeInner(displayMode);
}

DMError ScreenSessionManager::SetFoldDisplayModeInner(const FoldDisplayMode displayMode, std::string reason)
{
#ifdef FOLD_ABILITY_ENABLE
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d, reason: %{public}s",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid(), reason.c_str());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (!g_foldScreenFlag) {
        return DMError::DM_ERROR_INVALID_MODE_ID;
    }
    if (foldScreenController_ == nullptr) {
        TLOGI(WmsLogTag::DMS, "foldScreenController_ is null");
        return DMError::DM_ERROR_INVALID_MODE_ID;
    }
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGI(WmsLogTag::DMS, "calling clientName: %{public}s, calling pid: %{public}d, setmode: %{public}d",
        SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid(), displayMode);
    if (foldScreenController_->GetTentMode() &&
        (displayMode == FoldDisplayMode::FULL || displayMode == FoldDisplayMode::COORDINATION)) {
        TLOGW(WmsLogTag::DMS, "in TentMode, SetFoldDisplayMode to %{public}d failed", displayMode);
        return DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_TENTMODE;
    } else if ((FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice() ||
        FoldScreenStateInternel::IsSecondaryDisplayFoldDevice() || 
        FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) &&
        displayMode == FoldDisplayMode::COORDINATION) {
        DMError err = CanEnterCoordination();
        if (err != DMError::DM_OK) {
            return err;
        }
    }
    if (reason.compare("backSelfie") == 0) {
        UpdateCameraBackSelfie(true);
    }
    foldScreenController_->SetDisplayMode(displayMode);
    NotifyClientProxyUpdateFoldDisplayMode(displayMode);
#endif
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetFoldDisplayModeFromJs(const FoldDisplayMode displayMode, std::string reason)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    return SetFoldDisplayModeInner(displayMode, reason);
}

void ScreenSessionManager::UpdateDisplayScaleState(ScreenId screenId)
{
    auto session = GetScreenSession(screenId);
    if (session == nullptr) {
        TLOGE(WmsLogTag::DMS, "session is null");
        return;
    }
    const ScreenProperty& property = session->GetScreenProperty();
    if (std::fabs(property.GetScaleX() - DEFAULT_SCALE) < FLT_EPSILON &&
        std::fabs(property.GetScaleY() - DEFAULT_SCALE) < FLT_EPSILON &&
        std::fabs(property.GetPivotX() - DEFAULT_PIVOT) < FLT_EPSILON &&
        std::fabs(property.GetPivotY() - DEFAULT_PIVOT) < FLT_EPSILON) {
        TLOGD(WmsLogTag::DMS, "The scale and pivot is default value now. There is no need to update");
        return;
    }
    SetDisplayScaleInner(screenId, property.GetScaleX(), property.GetScaleY(), property.GetPivotX(),
                         property.GetPivotY());
}

void ScreenSessionManager::SetDisplayScaleInner(ScreenId screenId, const float& scaleX, const float& scaleY,
                                                const float& pivotX, const float& pivotY)
{
    auto session = GetScreenSession(screenId);
    if (session == nullptr) {
        TLOGE(WmsLogTag::DMS, "session is null");
        return;
    }
    if (pivotX > 1.0f || pivotX < 0.0f || pivotY > 1.0f || pivotY < 0.0f) {
        TLOGE(WmsLogTag::DMS, "pivotX [%{public}f] and pivotY [%{public}f] should be in [0.0~1.0f]", pivotX, pivotY);
        return;
    }
    float translateX = 0.0f;
    float translateY = 0.0f;
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        CalcDisplayNodeTranslateOnPocketFoldRotation(session, scaleX, scaleY, pivotX, pivotY,
            translateX, translateY);
    } else if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        if (FoldDisplayMode::MAIN == GetFoldDisplayMode()) {
            CalcDisplayNodeTranslateOnPocketFoldRotation(session, scaleX, scaleY, pivotX, pivotY,
                translateX, translateY);
        } else {
            CalcDisplayNodeTranslateOnRotation(session, scaleX, scaleY, pivotX, pivotY, translateX, translateY);
        }
    } else if (ROTATE_POLICY == FOLDABLE_DEVICE && FoldDisplayMode::FULL == GetFoldDisplayMode()) {
        CalcDisplayNodeTranslateOnFoldableRotation(session, scaleX, scaleY, pivotX, pivotY, translateX, translateY);
    } else if (SCREEN_SCAN_TYPE == SCAN_TYPE_VERTICAL) {
        CalcDisplayNodeTranslateOnVerticalScanRotation(session, ScaleProperty(scaleX, scaleY, pivotX, pivotY),
                                                       translateX, translateY);
    } else {
        CalcDisplayNodeTranslateOnRotation(session, scaleX, scaleY, pivotX, pivotY, translateX, translateY);
    }
    TLOGW(WmsLogTag::DMS,
          "screenId %{public}" PRIu64 ", scale [%{public}f, %{public}f], "
          "pivot [%{public}f, %{public}f], translate [%{public}f, %{public}f]",
          screenId, scaleX, scaleY, pivotX, pivotY, translateX, translateY);
    session->SetScreenScale(scaleX, scaleY, pivotX, pivotY, translateX, translateY);
    NotifyDisplayStateChange(GetDefaultScreenId(), session->ConvertToDisplayInfo(), {},
                             DisplayStateChangeType::UPDATE_SCALE);
}

void ScreenSessionManager::CalcDisplayNodeTranslateOnFoldableRotation(sptr<ScreenSession>& session, const float& scaleX,
                                                                      const float& scaleY, const float& pivotX,
                                                                      const float& pivotY, float& translateX,
                                                                      float& translateY)
{
    if (session == nullptr) {
        TLOGE(WmsLogTag::DMS, "session is nullptr");
        return;
    }
    const ScreenProperty& screenProperty = session->GetScreenProperty();
    auto screenWidth = screenProperty.GetBounds().rect_.GetWidth();
    auto screenHeight = screenProperty.GetBounds().rect_.GetHeight();
    Rotation rotation = session->GetRotation();
    float rotatedPivotX = DEFAULT_PIVOT;
    float rotatedPivotY = DEFAULT_PIVOT;
    float width = 0.0f;
    float height = 0.0f;
    switch (rotation) {
        case Rotation::ROTATION_0:
            rotatedPivotX = pivotY;
            rotatedPivotY = 1.0f - pivotX;
            width = screenHeight;
            height = screenWidth;
            break;
        case Rotation::ROTATION_90:
            rotatedPivotX = 1.0f - pivotX;
            rotatedPivotY = 1.0f - pivotY;
            width = screenWidth;
            height = screenHeight;
            break;
        case Rotation::ROTATION_180:
            rotatedPivotX = 1.0f - pivotY;
            rotatedPivotY = pivotX;
            width = screenHeight;
            height = screenWidth;
            break;
        case Rotation::ROTATION_270:
            rotatedPivotX = pivotX;
            rotatedPivotY = pivotY;
            width = screenWidth;
            height = screenHeight;
            break;
        default:
            TLOGE(WmsLogTag::DMS, "Unknown Rotation %{public}d", rotation);
            break;
    }
    translateX = (DEFAULT_PIVOT - rotatedPivotX) * (scaleX - DEFAULT_SCALE) * width;
    translateY = (DEFAULT_PIVOT - rotatedPivotY) * (scaleY - DEFAULT_SCALE) * height;
}

void ScreenSessionManager::CalcDisplayNodeTranslateOnPocketFoldRotation(sptr<ScreenSession>& session,
                                                                        const float& scaleX, const float& scaleY,
                                                                        const float& pivotX, const float& pivotY,
                                                                        float& translateX, float& translateY)
{
    if (session == nullptr) {
        TLOGE(WmsLogTag::DMS, "session is nullptr");
        return;
    }
    const ScreenProperty& screenProperty = session->GetScreenProperty();
    auto screenWidth = screenProperty.GetBounds().rect_.GetWidth();
    auto screenHeight = screenProperty.GetBounds().rect_.GetHeight();
    Rotation rotation = session->GetRotation();
    float rotatedPivotX = DEFAULT_PIVOT;
    float rotatedPivotY = DEFAULT_PIVOT;
    float width = 0.0f;
    float height = 0.0f;
    switch (rotation) {
        case Rotation::ROTATION_0:
            rotatedPivotX = 1.0f - pivotY;
            rotatedPivotY = pivotX;
            width = screenHeight;
            height = screenWidth;
            break;
        case Rotation::ROTATION_90:
            rotatedPivotX = pivotX;
            rotatedPivotY = pivotY;
            width = screenWidth;
            height = screenHeight;
            break;
        case Rotation::ROTATION_180:
            rotatedPivotX = pivotY;
            rotatedPivotY = 1.0f - pivotX;
            width = screenHeight;
            height = screenWidth;
            break;
        case Rotation::ROTATION_270:
            rotatedPivotX = 1.0f - pivotX;
            rotatedPivotY = 1.0f - pivotY;
            width = screenWidth;
            height = screenHeight;
            break;
        default:
            TLOGE(WmsLogTag::DMS, "Unknown Rotation %{public}d", rotation);
            break;
    }
    translateX = (DEFAULT_PIVOT - rotatedPivotX) * (scaleX - DEFAULT_SCALE) * width;
    translateY = (DEFAULT_PIVOT - rotatedPivotY) * (scaleY - DEFAULT_SCALE) * height;
}

void ScreenSessionManager::CalcDisplayNodeTranslateOnVerticalScanRotation(const sptr<ScreenSession>& session,
                                                                          const ScaleProperty& scalep,
                                                                          float& translateX, float& translateY)
{
    if (session == nullptr) {
        TLOGE(WmsLogTag::DMS, "session is nullptr");
        return;
    }
    auto screenProperty = session->GetScreenProperty();
    auto screenWidth = screenProperty.GetBounds().rect_.GetWidth();
    auto screenHeight = screenProperty.GetBounds().rect_.GetHeight();
    Rotation rotation = session->GetRotation();
    float rotatedPivotX = DEFAULT_PIVOT;
    float rotatedPivotY = DEFAULT_PIVOT;
    float width = 0.0f;
    float height = 0.0f;
    switch (rotation) {
        case Rotation::ROTATION_90:
            rotatedPivotX = scalep.pivotX;
            rotatedPivotY = scalep.pivotY;
            width = screenWidth;
            height = screenHeight;
            break;
        case Rotation::ROTATION_180:
            rotatedPivotX = scalep.pivotY;
            rotatedPivotY = 1.0f - scalep.pivotX;
            width = screenHeight;
            height = screenWidth;
            break;
        case Rotation::ROTATION_270:
            rotatedPivotX = 1.0f - scalep.pivotX;
            rotatedPivotY = 1.0f - scalep.pivotY;
            width = screenWidth;
            height = screenHeight;
            break;
        case Rotation::ROTATION_0:
            rotatedPivotX = 1.0f - scalep.pivotY;
            rotatedPivotY = scalep.pivotX;
            width = screenHeight;
            height = screenWidth;
            break;
        default:
            TLOGE(WmsLogTag::DMS, "Unknown Rotation %{public}d", rotation);
            break;
    }
    translateX = (DEFAULT_PIVOT - rotatedPivotX) * (scalep.scaleX - DEFAULT_SCALE) * width;
    translateY = (DEFAULT_PIVOT - rotatedPivotY) * (scalep.scaleY - DEFAULT_SCALE) * height;
}

void ScreenSessionManager::CalcDisplayNodeTranslateOnRotation(sptr<ScreenSession>& session, const float& scaleX,
                                                              const float& scaleY, const float& pivotX,
                                                              const float& pivotY, float& translateX, float& translateY)
{
    if (session == nullptr) {
        TLOGE(WmsLogTag::DMS, "session is nullptr");
        return;
    }
    const ScreenProperty& screenProperty = session->GetScreenProperty();
    auto screenWidth = screenProperty.GetBounds().rect_.GetWidth();
    auto screenHeight = screenProperty.GetBounds().rect_.GetHeight();
    Rotation rotation = session->GetRotation();
    float rotatedPivotX = DEFAULT_PIVOT;
    float rotatedPivotY = DEFAULT_PIVOT;
    float width = 0.0f;
    float height = 0.0f;
    switch (rotation) {
        case Rotation::ROTATION_0:
            rotatedPivotX = pivotX;
            rotatedPivotY = pivotY;
            width = screenWidth;
            height = screenHeight;
            break;
        case Rotation::ROTATION_90:
            rotatedPivotX = pivotY;
            rotatedPivotY = 1.0f - pivotX;
            width = screenHeight;
            height = screenWidth;
            break;
        case Rotation::ROTATION_180:
            rotatedPivotX = 1.0f - pivotX;
            rotatedPivotY = 1.0f - pivotY;
            width = screenWidth;
            height = screenHeight;
            break;
        case Rotation::ROTATION_270:
            rotatedPivotX = 1.0f - pivotY;
            rotatedPivotY = pivotX;
            width = screenHeight;
            height = screenWidth;
            break;
        default:
            TLOGE(WmsLogTag::DMS, "Unknown Rotation %{public}d", rotation);
            break;
    }
    translateX = (DEFAULT_PIVOT - rotatedPivotX) * (scaleX - DEFAULT_SCALE) * width;
    translateY = (DEFAULT_PIVOT - rotatedPivotY) * (scaleY - DEFAULT_SCALE) * height;
}

void ScreenSessionManager::SetDisplayScale(ScreenId screenId, float scaleX, float scaleY, float pivotX,
    float pivotY)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    SetDisplayScaleInner(screenId, scaleX, scaleY, pivotX, pivotY);
}

void ScreenSessionManager::SetFoldStatusLocked(bool locked)
{
#ifdef FOLD_ABILITY_ENABLE
    HandleSuperFoldStatusLocked(locked);
    if (!g_foldScreenFlag) {
        return;
    }
    if (foldScreenController_ == nullptr) {
        TLOGI(WmsLogTag::DMS, "foldScreenController_ is null");
        return;
    }
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    foldScreenController_->LockDisplayStatus(locked);
#endif
}

DMError ScreenSessionManager::SetFoldStatusLockedFromJs(bool locked)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    SetFoldStatusLocked(locked);
    return DMError::DM_OK;
}

DMError ScreenSessionManager::ForceSetFoldStatusAndLock(FoldStatus targetFoldStatus)
{
#ifdef FOLD_ABILITY_ENABLE
    TLOGD(WmsLogTag::DMS, "Entry ForceSetFoldStatusAndLock");
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (foldScreenController_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "foldScreenController is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    return foldScreenController_->ForceSetFoldStatusAndLock(targetFoldStatus);
#else
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
#endif
}

DMError ScreenSessionManager::RestorePhysicalFoldStatus()
{
#ifdef FOLD_ABILITY_ENABLE
    TLOGD(WmsLogTag::DMS, "Entry ForceSetFoldStatusAndLock");
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (foldScreenController_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "foldScreenController is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    return foldScreenController_->RestorePhysicalFoldStatus();
#else
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
#endif
}

bool ScreenSessionManager::GetPhysicalFoldLockFlag() const
{
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "foldScreenController is null");
        return false;
    }
    return foldScreenController_->GetPhysicalFoldLockFlag();
#else
    return false;
#endif
}

FoldStatus ScreenSessionManager::GetPhysicalFoldStatus() const
{
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "foldScreenController is null");
        return FoldStatus::UNKNOWN;
    }
    return foldScreenController_->GetPhysicalFoldStatus();
#else
    return FoldStatus::UNKNOWN;
#endif
}

FoldDisplayMode ScreenSessionManager::GetFoldDisplayMode()
{
#ifdef FOLD_ABILITY_ENABLE
    DmsXcollie dmsXcollie("DMS:GetFoldDisplayMode", XCOLLIE_TIMEOUT_10S);
    if (!g_foldScreenFlag) {
        return FoldDisplayMode::UNKNOWN;
    }
    if (foldScreenController_ == nullptr) {
        TLOGD(WmsLogTag::DMS, "foldScreenController_ is null");
        return FoldDisplayMode::UNKNOWN;
    }
    if (IsSpecialApp()) {
        return FoldDisplayMode::MAIN;
    }
    return foldScreenController_->GetDisplayMode();
#else
    return FoldDisplayMode::UNKNOWN;
#endif
}

bool ScreenSessionManager::IsFoldable()
{
#ifdef FOLD_ABILITY_ENABLE
    // Most applications do not adapt to Lem rotation and are temporarily treated as non fold device
    if (FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
        return false;
    }

    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        return true;
    }

    if (!g_foldScreenFlag) {
        return false;
    }
    if (foldScreenController_ == nullptr) {
        TLOGI(WmsLogTag::DMS, "foldScreenController_ is null");
        return false;
    }
    if (IsSpecialApp()) {
        return false;
    }
    return foldScreenController_->IsFoldable();
#else
    return false;
#endif
}

bool ScreenSessionManager::IsCaptured()
{
    if (ScreenSceneConfig::GetExternalScreenDefaultMode() == "none") {
        // 如果当前是PC拓展模式，非截屏，录屏，投屏，则返回false
        return isScreenShot_ || virtualScreenCount_ > 0 || (hdmiScreenCount_ > 0 && !IsExtendMode());
    } else {
        return isScreenShot_ || virtualScreenCount_ > 0 || hdmiScreenCount_ > 0;
    }
}

bool ScreenSessionManager::IsMultiScreenCollaboration()
{
    return isMultiScreenCollaboration_;
}

bool ScreenSessionManager::HasCastEngineOrPhyMirror(const std::vector<ScreenId>& screenIdsToExclude)
{
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionItem : screenSessionMap_) {
        auto screenId = sessionItem.first;
        auto screenSession = sessionItem.second;
        if (screenSession == nullptr) {
            TLOGI(WmsLogTag::DMS, "screenSession is null");
            continue;
        }
        if (std::find(screenIdsToExclude.begin(), screenIdsToExclude.end(), screenId) != screenIdsToExclude.end()) {
            continue;
        }
        auto screenType = screenSession->GetScreenProperty().GetScreenType();
        if (screenType == ScreenType::VIRTUAL
            && screenSession->GetName() == SCREEN_NAME_CAST) {
            return true;
        }

        if (IsDefaultMirrorMode(screenId) && screenType == ScreenType::REAL &&
            screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
            return true;
        }
    }
    return false;
}

FoldStatus ScreenSessionManager::GetFoldStatus()
{
#ifdef FOLD_ABILITY_ENABLE
    DmsXcollie dmsXcollie("DMS:GetFoldStatus", XCOLLIE_TIMEOUT_10S);
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        SuperFoldStatus status = SuperFoldStateManager::GetInstance().GetCurrentStatus();
        return SuperFoldStateManager::GetInstance().MatchSuperFoldStatusToFoldStatus(status);
    }
    if (!g_foldScreenFlag) {
        return FoldStatus::UNKNOWN;
    }
    if (foldScreenController_ == nullptr) {
        TLOGI(WmsLogTag::DMS, "foldScreenController_ is null");
        return FoldStatus::UNKNOWN;
    }
    if (IsSpecialApp()) {
        return FoldStatus::UNKNOWN;
    }
    return foldScreenController_->GetFoldStatus();
#else
    return FoldStatus::UNKNOWN;
#endif
}

SuperFoldStatus ScreenSessionManager::GetSuperFoldStatus()
{
#ifdef FOLD_ABILITY_ENABLE
    DmsXcollie dmsXcollie("DMS:GetSuperFoldStatus", XCOLLIE_TIMEOUT_10S);
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        return SuperFoldStatus::UNKNOWN;
    }
    SuperFoldStatus status = SuperFoldStateManager::GetInstance().GetCurrentStatus();
    return status;
#else
    return SuperFoldStatus::UNKNOWN;
#endif
}

float ScreenSessionManager::GetSuperRotation()
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return -1.f;
    }
    DmsXcollie dmsXcollie("DMS:GetSuperRotation", XCOLLIE_TIMEOUT_10S);
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        return -1.f;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (auto sessionIt : screenSessionMap_) {
            auto screenSession = sessionIt.second;
            if (screenSession == nullptr) {
                TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
                continue;
            }
            if (screenSession->GetIsInternal()) {
                float rotation = static_cast<int>(screenSession->GetRotation()) * 90.f;
                TLOGI(WmsLogTag::DMS, "is internal, screenId = %{public}" PRIu64 ", rotation = %{public}f",
                    sessionIt.first, rotation);
                return rotation;
            } else {
                TLOGI(WmsLogTag::DMS, "not internal, screenId = %{public}" PRIu64"", sessionIt.first);
            }
        }
    }
    TLOGE(WmsLogTag::DMS, "all screenSession is nullptr or not internal");
    return -1.f;
}

bool ScreenSessionManager::GetIsLandscapeLockStatus()
{
    return isLandscapeLockStatus_;
}

void ScreenSessionManager::SetIsLandscapeLockStatus(bool isLandscapeLockStatus)
{
    isLandscapeLockStatus_ = isLandscapeLockStatus;
    TLOGW(WmsLogTag::DMS, "statemachine landscape locked is: %{public}d", static_cast<int32_t>(isLandscapeLockStatus_));
}

void ScreenSessionManager::SetLandscapeLockStatus(bool isLocked)
{
#ifdef FOLD_ABILITY_ENABLE
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! pid: %{public}d", IPCSkeleton::GetCallingPid());
        return;
    }
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        TLOGI(WmsLogTag::DMS, "not super fold display device.");
        return;
    }
    if (isLocked) {
        SetIsLandscapeLockStatus(true);
        SuperFoldSensorManager::GetInstance().DriveStateMachineToExpand();
    } else {
        SetIsLandscapeLockStatus(false);
        SuperFoldSensorManager::GetInstance().SetStateMachineToActived();
    }
#endif
}

ExtendScreenConnectStatus ScreenSessionManager::GetExtendScreenConnectStatus()
{
#ifdef WM_MULTI_SCREEN_ENABLE
    DmsXcollie dmsXcollie("DMS:GetExtendScreenConnectStatus", XCOLLIE_TIMEOUT_10S);
    return extendScreenConnectStatus_.load();
#else
    return ExtendScreenConnectStatus::UNKNOWN;
#endif
}

bool ScreenSessionManager::GetIsPhysicalExtendScreenConnected()
{
    return isExtendScreenConnected_;
}

void ScreenSessionManager::SetIsPhysicalExtendScreenConnected(bool isExtendScreenConnected)
{
    isExtendScreenConnected_ = isExtendScreenConnected;
}

void ScreenSessionManager::SetExpandAndHorizontalLocked(bool isLocked)
{
#ifdef FOLD_ABILITY_ENABLE
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        TLOGI(WmsLogTag::DMS, "not super fold display device.");
        return;
    }
    if (isLocked) {
        TLOGI(WmsLogTag::DMS, "drive statemachine to expand");
        // drive state machine to expand
        SuperFoldSensorManager::GetInstance().DriveStateMachineToExpand();
        // to connect
        extendScreenConnectStatus_.store(ExtendScreenConnectStatus::CONNECT);
        OnExtendScreenConnectStatusChange(SCREEN_ID_DEFAULT, ExtendScreenConnectStatus::CONNECT);

    } else {
        TLOGI(WmsLogTag::DMS, "active statemachine");
        // drive state machine to actived
        SuperFoldSensorManager::GetInstance().SetStateMachineToActived();
        // to disconnect
        OnExtendScreenConnectStatusChange(SCREEN_ID_DEFAULT, ExtendScreenConnectStatus::DISCONNECT);
        extendScreenConnectStatus_.store(ExtendScreenConnectStatus::DISCONNECT);
    }
#endif
}

bool ScreenSessionManager::GetIsExtendModelocked()
{
    return isExtendMode_;
}
 
void ScreenSessionManager::SetIsExtendModelocked(bool isExtendMode)
{
    isExtendMode_ = isExtendMode;
    TLOGW(WmsLogTag::DMS, "statemachine extend modelocked is: %{puclic}d", isExtendMode);
}

bool ScreenSessionManager::GetIsFoldStatusLocked()
{
    return isFoldStatusLocked_;
}

void ScreenSessionManager::SetIsFoldStatusLocked(bool isFoldStatusLocked)
{
    isFoldStatusLocked_ = isFoldStatusLocked;
    TLOGW(WmsLogTag::DMS, "statemachine foldstates locked is: %{puclic}d", isFoldStatusLocked);
}

void ScreenSessionManager::SetFoldStatusExpandAndLocked(bool isLocked)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return;
    }
#ifdef FOLD_ABILITY_ENABLE
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        TLOGI(WmsLogTag::DMS, "not super fold display device.");
        return;
    }
    SetIsFoldStatusLocked(isLocked);
    if (isLocked == true) {
        SuperFoldSensorManager::GetInstance().HandleFoldStatusLockedToExpand();
    } else {
        SuperFoldSensorManager::GetInstance().HandleFoldStatusUnlocked();
    }
#endif
}

void ScreenSessionManager::HandleSuperFoldStatusLocked(bool isLocked)
{
#ifdef FOLD_ABILITY_ENABLE
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        TLOGI(WmsLogTag::DMS, "not super fold display device.");
        return;
    }
    SetIsFoldStatusLocked(isLocked);
    if (isLocked == true) {
        TLOGI(WmsLogTag::DMS, "Fold status is locked.");
    } else {
        SuperFoldSensorManager::GetInstance().HandleFoldStatusUnlocked();
    }
#endif
}

bool ScreenSessionManager::GetIsOuterOnlyMode()
{
    return isOuterOnlyMode_;
}

void ScreenSessionManager::SetIsOuterOnlyMode(bool isOuterOnlyMode)
{
    isOuterOnlyMode_ = isOuterOnlyMode;
}

bool ScreenSessionManager::GetIsOuterOnlyModeBeforePowerOff()
{
    return isOuterOnlyModeBeforePowerOff_;
}

void ScreenSessionManager::SetIsOuterOnlyModeBeforePowerOff(bool isOuterOnlyModeBeforePowerOff)
{
    isOuterOnlyModeBeforePowerOff_ = isOuterOnlyModeBeforePowerOff;
}

bool ScreenSessionManager::GetTentMode()
{
#ifdef FOLD_ABILITY_ENABLE
    if (!g_foldScreenFlag) {
        return false;
    }
    if (foldScreenController_ == nullptr) {
        TLOGI(WmsLogTag::DMS, "foldScreenController_ is null");
        return false;
    }
    return foldScreenController_->GetTentMode();
#else
    return false;
#endif
}

sptr<FoldCreaseRegion> ScreenSessionManager::GetCurrentFoldCreaseRegion()
{
#ifdef FOLD_ABILITY_ENABLE
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        return SuperFoldStateManager::GetInstance().GetCurrentFoldCreaseRegion();
    }
    if (!g_foldScreenFlag) {
        return nullptr;
    }
    if (foldScreenController_ == nullptr) {
        TLOGI(WmsLogTag::DMS, "foldScreenController_ is null");
        return nullptr;
    }
    return foldScreenController_->GetCurrentFoldCreaseRegion();
#else
    return nullptr;
#endif
}

DMError ScreenSessionManager::GetLiveCreaseRegion(FoldCreaseRegion& region)
{
#ifdef FOLD_ABILITY_ENABLE
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        region = SuperFoldStateManager::GetInstance().GetLiveCreaseRegion();
        return DMError::DM_OK;
    }
    if (!g_foldScreenFlag) {
        region = FoldCreaseRegion(0, {});
        return DMError::DM_OK;
    }
    if (foldScreenController_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "foldScreenController_ is null");
        return DMError::DM_ERROR_INVALID_MODE_ID;
    }
    region = foldScreenController_->GetLiveCreaseRegion();
    return DMError::DM_OK;
#else
    region = FoldCreaseRegion(0, {});
    return DMError::DM_OK;
#endif
}

uint32_t ScreenSessionManager::GetCurvedCompressionArea()
{
    return ScreenSceneConfig::GetCurvedCompressionAreaInLandscape();
}

void ScreenSessionManager::NotifyFoldStatusChanged(FoldStatus foldStatus)
{
#ifdef FOLD_ABILITY_ENABLE
    sptr<ScreenSession> screenSession = GetDefaultScreenSession();
    if (screenSession != nullptr) {
        if (foldStatus == FoldStatus::FOLDED && !FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
            screenSession->SetDefaultDeviceRotationOffset(0);
        } else {
            screenSession->SetDefaultDeviceRotationOffset(defaultDeviceRotationOffset_);
        }
    }
    if (screenSession != nullptr && FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        if (foldStatus == FoldStatus::FOLDED) {
            // sub screen default rotation offset is 270
            screenSession->SetDefaultDeviceRotationOffset(270);
            auto property = screenSession->GetScreenProperty();
            SetVirtualPixelRatio(GetDefaultScreenId(), subDensityDpi_);
        } else {
            float dpi = static_cast<float>(cachedSettingDpi_) / BASELINE_DENSITY;
            TLOGI(WmsLogTag::DMS, "Set inner screen dpi: %{public}f", dpi);
            SetVirtualPixelRatio(GetDefaultScreenId(), dpi);
        }
    }
    ScreenSessionManagerAdapter::GetInstance().NotifyFoldStatusChanged(foldStatus);
    if (lowTemp_ == LowTempMode::LOW_TEMP_ON) {
        ScreenSessionPublish::GetInstance().PublishSmartNotificationEvent(FAULT_DESCRIPTION, FAULT_SUGGESTION);
    }
#endif
}

void ScreenSessionManager::SetLowTemp(LowTempMode lowTemp)
{
    std::lock_guard<std::mutex> lock(lowTempMutex_);
    if (lowTemp == LowTempMode::LOW_TEMP_ON && lowTemp_ != lowTemp) {
        TLOGI(WmsLogTag::DMS, "device enter low temperature mode.");
        ScreenSessionPublish::GetInstance().PublishSmartNotificationEvent(FAULT_DESCRIPTION, FAULT_SUGGESTION);
    }
    if (lowTemp == LowTempMode::LOW_TEMP_OFF) {
        TLOGI(WmsLogTag::DMS, "device exit low temperature mode.");
    }
    lowTemp_ = lowTemp;
}

void ScreenSessionManager::NotifyFoldAngleChanged(std::vector<float> foldAngles)
{
    {
        std::lock_guard<std::mutex> lock(lastStatusUpdateMutex_);
        lastFoldAngles_ = foldAngles;
    }
    ScreenSessionManagerAdapter::GetInstance().NotifyFoldAngleChanged(foldAngles);
}

void ScreenSessionManager::NotifyCaptureStatusChanged()
{
    ScreenSessionManagerAdapter::GetInstance().NotifyCaptureStatusChanged();
}

void ScreenSessionManager::NotifyCaptureStatusChanged(bool isCapture)
{
    ScreenSessionManagerAdapter::GetInstance().NotifyCaptureStatusChanged(isCapture);
}

void ScreenSessionManager::NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info)
{
    {
        std::lock_guard<std::mutex> lock(lastStatusUpdateMutex_);
        lastDisplayChangeInfo_ = info;
    }
    ScreenSessionManagerAdapter::GetInstance().NotifyDisplayChangeInfoChanged(info);
}

void ScreenSessionManager::RefreshMirrorScreenRegion(ScreenId screenId)
{
#if defined(FOLD_ABILITY_ENABLE) && defined(WM_MULTI_SCREEN_ENABLE)
    auto screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get screen session");
        return;
    }
    DMRect mainScreenRegion = {0, 0, 0, 0};
    foldScreenController_->SetMainScreenRegion(mainScreenRegion);
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    if (!ConvertScreenIdToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "Screen: %{public}" PRIu64" convert to rs id failed", screenId);
    } else {
        screenSession->SetMirrorScreenRegion(rsScreenId, mainScreenRegion);
        screenSession->EnableMirrorScreenRegion();
    }
#endif
}

void ScreenSessionManager::NotifyDisplayModeChanged(FoldDisplayMode displayMode)
{
    NotifyClientProxyUpdateFoldDisplayMode(displayMode);
    SetScreenCorrection();
    ScreenSessionManagerAdapter::GetInstance().NotifyDisplayModeChanged(displayMode);
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ != nullptr && FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        auto screenIds = GetAllScreenIds();
        for (auto screenId : screenIds) {
            RefreshMirrorScreenRegion(screenId);
        }
    }
#endif
}

void ScreenSessionManager::NotifyScreenMagneticStateChanged(bool isMagneticState)
{
    ScreenSessionManagerAdapter::GetInstance().NotifyScreenMagneticStateChanged(isMagneticState);
}

void ScreenSessionManager::RegisterBrightnessInfoChangeListener()
{
    TLOGI(WmsLogTag::DMS, "start");
    auto res = rsInterface_.SetBrightnessInfoChangeCallback(
        [this](ScreenId screenId, BrightnessInfo brightnessInfo) {
            NotifyBrightnessInfoChanged(screenId, brightnessInfo);
        });
    if (res != StatusCode::SUCCESS) {
        auto task = [this]() {RegisterBrightnessInfoChangeListener();};
        taskScheduler_->PostAsyncTask(task, "RegisterBrightnessInfoChangeListener", 50); // retry after 50ms.
        screenEventTracker_.RecordEvent("dms brightness info register failed");
        TLOGW(WmsLogTag::DMS, "dms brightness info register failed");
    } else {
        screenEventTracker_.RecordEvent("dms brightness info register success");
    }
}

void ScreenSessionManager::UnregisterBrightnessInfoChangeListener()
{
    TLOGI(WmsLogTag::DMS, "start");
    auto res = rsInterface_.SetBrightnessInfoChangeCallback(nullptr);
    if (res != StatusCode::SUCCESS) {
        auto task = [this]() {
            UnregisterBrightnessInfoChangeListener();
        };
        taskScheduler_->PostAsyncTask(task, "UnregisterBrightnessInfoChangeListener", 50); // retry after 50ms.
        screenEventTracker_.RecordEvent("dms brightness info unregister failed");
        TLOGW(WmsLogTag::DMS, "dms brightness info unregister failed");
    } else {
        screenEventTracker_.RecordEvent("dms brightness info unregister success");
    }
}

void ScreenSessionManager::NotifyBrightnessInfoChanged(ScreenId rsId, const BrightnessInfo& info)
{
    TLOGD(WmsLogTag::DMS, "notify brightness info");
    ScreenId smsScreenId = SCREEN_ID_INVALID;
    auto agents = ScreenSessionManagerAdapter::GetInstance().dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::BRIGHTNESS_INFO_CHANGED_LISTENER);
    if (agents.empty()) {
        TLOGW(WmsLogTag::DMS, "Agent is empty");
        return;
    }
    if (!screenIdManager_.ConvertToSmsScreenId(rsId, smsScreenId)) {
        return;
    }
    if (smsScreenId == SCREEN_ID_INVALID) {
        TLOGW(WmsLogTag::DMS, "smsScreenId is invalid");
        return;
    }
    ScreenId logicalScreenId = smsScreenId;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (const auto& iter : screenSessionMap_) {
            auto session = iter.second;
            if (session != nullptr && session->GetPhyScreenId() == smsScreenId) {
                logicalScreenId = session->GetScreenId();
                TLOGI(WmsLogTag::DMS, "transform rsId %{public}" PRIu64"to logicalScreenId %{public}" PRIu64" ", rsId, logicalScreenId);
                break;
            }
        }
    }
    if (logicalScreenId != 0) {
        TLOGD(WmsLogTag::DMS, "ignore transform rsId %{public}" PRIu64"to logicalScreenId %{public}" PRIu64" ", rsId, logicalScreenId);
        return;
    }
    
    ScreenBrightnessInfo screenBrightnessInfo;
    screenBrightnessInfo.currentHeadroom = info.currentHeadroom;
    screenBrightnessInfo.maxHeadroom = info.maxHeadroom;
    screenBrightnessInfo.sdrNits = info.sdrNits;
    for (auto& agent : agents) {
        int32_t agentPid = ScreenSessionManagerAdapter::GetInstance().dmAgentContainer_.GetAgentPid(agent);
        if (!IsFreezed(agentPid, DisplayManagerAgentType::BRIGHTNESS_INFO_CHANGED_LISTENER) && agent != nullptr) {
            agent->NotifyBrightnessInfoChanged(logicalScreenId, screenBrightnessInfo);
        }
    }
}

void ScreenSessionManager::NotifyRecordingDisplayChanged(const std::vector<DisplayId>& displayIds)
{
    auto agents = ScreenSessionManagerAdapter::GetInstance().
        dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGI(WmsLogTag::DMS, "Agent is empty");
        return;
    }
    for (auto& agent : agents) {
        agent->NotifyRecordingDisplayChanged(displayIds);
    }
}

void ScreenSessionManager::SetDisplayNodeScreenId(ScreenId screenId, ScreenId displayNodeScreenId)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " displayNodeScreenId: %{public}" PRIu64,
        screenId, displayNodeScreenId);
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetDisplayNodeScreenId");
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy->SetDisplayNodeScreenId(screenId, displayNodeScreenId);
#ifdef DEVICE_STATUS_ENABLE
    SetDragWindowScreenId(screenId, displayNodeScreenId);
#endif // DEVICE_STATUS_ENABLE
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SetMultiWindowScreenId");
    MMI::InputManager::GetInstance()->SetMultiWindowScreenId(screenId, displayNodeScreenId);
}

#ifdef DEVICE_STATUS_ENABLE
void ScreenSessionManager::SetDragWindowScreenId(ScreenId screenId, ScreenId displayNodeScreenId)
{
    auto interactionManager = Msdp::DeviceStatus::InteractionManager::GetInstance();
    if (interactionManager != nullptr) {
        interactionManager->SetDragWindowScreenId(screenId, displayNodeScreenId);
    }
}
#endif // DEVICE_STATUS_ENABLE

void ScreenSessionManager::OnPropertyChange(const ScreenProperty& newProperty, ScreenPropertyChangeReason reason,
    ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " reason: %{public}d", screenId, static_cast<int>(reason));
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        if (foldScreenController_ != nullptr && !FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
            foldScreenController_->SetdisplayModeChangeStatus(false);
        }
        return;
    }
    clientProxy->OnPropertyChanged(screenId, newProperty, reason);
}

void ScreenSessionManager::OnFoldPropertyChange(ScreenId screenId, const ScreenProperty& newProperty,
    ScreenPropertyChangeReason reason, FoldDisplayMode displayMode)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " reason: %{public}d", screenId, static_cast<int>(reason));
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
#ifdef FOLD_ABILITY_ENABLE
        if (foldScreenController_ != nullptr && !FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
            foldScreenController_->SetdisplayModeChangeStatus(false);
        }
#endif
        TLOGE(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }

    // 1. Configure properties prior to notifying the application.
    // 2. The reset operation is triggered subsequent to SCB completing property calculations.
    ScreenProperty midProperty;
    bool result = clientProxy->OnFoldPropertyChange(screenId, newProperty, reason, displayMode, midProperty);
    if (!result) {
        TLOGE(WmsLogTag::DMS, "result is failed");
        return;
    }
    TLOGI(WmsLogTag::DMS, "OnFoldPropertyChange get process data,screenId: %{public}" PRIu64
        ", rotation:%{public}f, width:%{public}f, height:%{public}f", screenId,
        midProperty.GetRotation(), midProperty.GetBounds().rect_.GetWidth(), midProperty.GetBounds().rect_.GetHeight());

    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "Cannot get ScreenSession, screenId: %{public}" PRIu64 "", screenId);
        return;
    }
    screenSession->UpdateScbScreenPropertyToServer(midProperty);

    // Temporarily set screen property for app while property change is in progress
    // SuperFoldDisplayDevice notify app when property is ready,so no need to modify
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        Rotation rotation = Rotation::ROTATION_0;
        screenSession->AddRotationCorrection(rotation, displayMode);
        screenSession->SetRotationAndScreenRotationOnly(rotation);
        screenSession->SetOrientationMatchRotation(rotation, displayMode);
        TLOGD(WmsLogTag::DMS, "init rotation= %{public}u", rotation);
    }
}

void ScreenSessionManager::OnPowerStatusChange(DisplayPowerEvent event, EventStatus status,
    PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS, "[UL_POWER]event: %{public}d, status: %{public}d, reason: %{public}d",
        static_cast<int>(event),
        static_cast<int>(status), static_cast<int>(reason));
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER] clientProxy_ is null");
        return;
    }
    clientProxy->OnPowerStatusChanged(event, status, reason);
}

void ScreenSessionManager::OnSensorRotationChange(float sensorRotation, ScreenId screenId, bool isSwitchUser)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " sensorRotation: %{public}f", screenId, sensorRotation);
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy->OnSensorRotationChanged(screenId, sensorRotation, isSwitchUser);
}

void ScreenSessionManager::OnHoverStatusChange(int32_t hoverStatus, bool needRotate, ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " hoverStatus: %{public}d", screenId, hoverStatus);
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy->OnHoverStatusChanged(screenId, hoverStatus, needRotate);
}

void ScreenSessionManager::OnScreenOrientationChange(float screenOrientation, ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " screenOrientation: %{public}f", screenId, screenOrientation);
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGI(WmsLogTag::DMS, "ClientProxy_ is null");
        return;
    }
    clientProxy->OnScreenOrientationChanged(screenId, screenOrientation);
}

void ScreenSessionManager::OnScreenRotationLockedChange(bool isLocked, ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " isLocked: %{public}d", screenId, isLocked);
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGI(WmsLogTag::DMS, "ClientProxy_ is null");
        return;
    }
    clientProxy->OnScreenRotationLockedChanged(screenId, isLocked);
}

void ScreenSessionManager::OnCameraBackSelfieChange(bool isCameraBackSelfie, ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " isCameraBackSelfie: %{public}d", screenId, isCameraBackSelfie);
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy->OnCameraBackSelfieChanged(screenId, isCameraBackSelfie);
}

void ScreenSessionManager::NotifyClientProxyUpdateFoldDisplayMode(FoldDisplayMode displayMode)
{
    auto clientProxy = GetClientProxy();
    if (clientProxy) {
        TLOGI(WmsLogTag::DMS, "displayMode = %{public}d",
            static_cast<int>(displayMode));
        clientProxy->OnUpdateFoldDisplayMode(displayMode);
    }
}

void ScreenSessionManager::ScbClientDeathCallback(int32_t deathScbPid)
{
    TLOGI(WmsLogTag::DMS, "Enter ScbClientDeathCallback, deathScbPid: %{public}d", deathScbPid);
    std::unique_lock<std::mutex> lock(oldScbPidsMutex_);
    if (deathScbPid == currentScbPId_ || currentScbPId_ == INVALID_SCB_PID) {
        SetClientProxy(nullptr);
        TLOGE(WmsLogTag::DMS, "death callback and set clientProxy null");
    }
    if (DmUtils::safe_wait_for(scbSwitchCV_, lock, std::chrono::milliseconds(CV_WAIT_SCBSWITCH_MS))
        == std::cv_status::timeout) {
        TLOGE(WmsLogTag::DMS, "set client task deathScbPid:%{public}d, timeout: %{public}d",
            deathScbPid, CV_WAIT_SCBSWITCH_MS);
    }
    std::ostringstream oss;
    oss << "Scb client death: " << deathScbPid;
    TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    screenEventTracker_.RecordEvent(oss.str());
    oldScbPids_.erase(std::remove(oldScbPids_.begin(), oldScbPids_.end(), deathScbPid), oldScbPids_.end());
    deathPidVector_.push_back(deathScbPid);
}

void ScreenSessionManager::AddScbClientDeathRecipient(const sptr<IScreenSessionManagerClient>& scbClient,
    int32_t scbPid)
{
    sptr<ScbClientListenerDeathRecipient> scbClientDeathListener =
        new (std::nothrow) ScbClientListenerDeathRecipient(scbPid);
    if (scbClientDeathListener == nullptr) {
        TLOGE(WmsLogTag::DMS, "add scb: %{public}d death listener failed", scbPid);
        return;
    }
    if (scbClient != nullptr && scbClient->AsObject() != nullptr) {
        TLOGI(WmsLogTag::DMS, "add scb: %{public}d death listener", scbPid);
        scbClient->AsObject()->AddDeathRecipient(scbClientDeathListener);
    }
}

void ScreenSessionManager::SwitchUser()
{
#ifdef WM_MULTI_USR_ABILITY_ENABLE
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied, clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    auto userId = GetUserIdByCallingUid();
    auto newScbPid = IPCSkeleton::GetCallingPid();
    if (userId == currentUserId_) {
        TLOGE(WmsLogTag::DMS, "switch user not change");
        return;
    }
    currentUserIdForSettings_ = userId;
    if (g_isPcDevice && userSwitching_) {
        std::unique_lock<std::mutex> lock(switchUserMutex_);
        if (DmUtils::safe_wait_for(switchUserCV_, lock, std::chrono::milliseconds(CV_WAIT_USERSWITCH_MS)) == std::cv_status::timeout) {
            TLOGI(WmsLogTag::DMS, "wait switchUserCV_ timeout");
            userSwitching_ = false;
        }
    }
    {
        std::lock_guard<std::mutex> lock(oldScbPidsMutex_);
        SwitchScbNodeHandle(userId, IPCSkeleton::GetCallingPid(), false);
    }
    DisplayId defaultId = GetDefaultScreenId();
    int32_t oldUserId = currentUserId_;
    if (IsConcurrentUser()) {
        if (ActiveUser(userId, oldUserId, newScbPid)) {
            TLOGI(WmsLogTag::DMS, "Activeuser success, oldUserId: %{public}d, newUserId: %{public}d",
                oldUserId, userId);
        } else {
            TLOGI(WmsLogTag::DMS, "Activeuser failed, user oldUserId: %{public}d", oldUserId);
        }
        DisplayId id = GetUserDisplayId(userId);
        if (id != DISPLAY_ID_INVALID) {
            defaultId = id;
        } else {
            TLOGE(WmsLogTag::DMS, "get invalid display id, user: %{public}d", userId);
        }
    }
    SwitchScbNodeHandle(userId, newScbPid, false);
    MockSessionManagerService::GetInstance().NotifyWMSConnected(userId, defaultId, false);
#endif
}

void ScreenSessionManager::SetDefaultMultiScreenModeWhenSwitchUser()
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return;
    }
    TLOGI(WmsLogTag::DMS, "switching user, change screen mode.");
    if (!g_isPcDevice) {
        TLOGW(WmsLogTag::DMS, "not pc device");
        return;
    }
#ifdef WM_MULTI_USR_ABILITY_ENABLE
    sptr<ScreenSession> innerSession = nullptr;
    sptr<ScreenSession> externalSession = nullptr;
    GetInternalAndExternalSession(innerSession, externalSession);
    if (!innerSession || !externalSession) {
        TLOGE(WmsLogTag::DMS, "screen session is null.");
        return;
    }
    userSwitching_ = true;
    std::ostringstream oss;
    oss << "innerScreen screenId: " << innerSession->GetScreenId()
        << ", rsId: " << innerSession->GetRSScreenId()
        << ", externalScreen screenId: " << externalSession->GetScreenId()
        << ", rsId: " << externalSession->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    if (externalSession->GetScreenCombination() != ScreenCombination::SCREEN_MIRROR) {
        if (GetIsOuterOnlyMode()) {
            TLOGI(WmsLogTag::DMS, "exit outer only mode.");
            SetIsOuterOnlyMode(false);
            ExitOuterOnlyMode(innerSession->GetRSScreenId(), externalSession->GetRSScreenId(),
                MultiScreenMode::SCREEN_MIRROR);
            switchUserCV_.notify_all();
            userSwitching_ = false;
        } else {
            TLOGI(WmsLogTag::DMS, "change to mirror.");
            SetIsOuterOnlyMode(false);
            MultiScreenModeChange(innerSession->GetRSScreenId(), externalSession->GetRSScreenId(), SCREEN_MIRROR);
        }
    } else {
        TLOGI(WmsLogTag::DMS, "already mirror.");
        switchUserCV_.notify_all();
        userSwitching_ = false;
    }
#endif
}

void ScreenSessionManager::ScbStatusRecoveryWhenSwitchUser(std::vector<int32_t> oldScbPids, int32_t newScbPid)
{
#ifdef WM_MULTI_USR_ABILITY_ENABLE
    sptr<ScreenSession> screenSession = GetDefaultScreenSession();
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail to get default screenSession");
        return;
    }
    if (g_foldScreenFlag) {
        NotifyFoldStatusChanged(GetFoldStatus());
        NotifyDisplayModeChanged(GetFoldDisplayMode());
    }
    int64_t delayTime = 0;
    if (g_foldScreenFlag && oldScbDisplayMode_ != GetFoldDisplayMode() &&
        !FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
        delayTime = SWITCH_USER_DISPLAYMODE_CHANGE_DELAY;
        auto foldStatus = GetFoldStatus();
        TLOGE(WmsLogTag::DMS, "old mode: %{public}u, cur mode: %{public}u", oldScbDisplayMode_, GetFoldDisplayMode());
        if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
            screenSession->UpdatePropertyByFoldControl(screenSession->GetScreenProperty());
            OnVerticalChangeBoundsWhenSwitchUser(screenSession, oldScbDisplayMode_);
            screenSession->PropertyChange(screenSession->GetScreenProperty(),
                FoldDisplayMode::MAIN == GetFoldDisplayMode() ? ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING :
                ScreenPropertyChangeReason::FOLD_SCREEN_EXPAND_SWITCH_USER);
        } else if (foldStatus == FoldStatus::EXPAND || foldStatus == FoldStatus::HALF_FOLD) {
            screenSession->UpdatePropertyByFoldControl(GetPhyScreenProperty(SCREEN_ID_FULL));
            OnBeforeScreenPropertyChange(foldStatus);
            screenSession->PropertyChange(screenSession->GetScreenProperty(),
                ScreenPropertyChangeReason::FOLD_SCREEN_EXPAND_SWITCH_USER);
        } else if (foldStatus == FoldStatus::FOLDED) {
            screenSession->UpdatePropertyByFoldControl(GetPhyScreenProperty(SCREEN_ID_MAIN));
            OnBeforeScreenPropertyChange(foldStatus);
            screenSession->PropertyChange(screenSession->GetScreenProperty(),
                ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING_SWITCH_USER);
        } else {
            TLOGE(WmsLogTag::DMS, "unsupport foldStatus: %{public}u", foldStatus);
        }
    } else {
        screenSession->UpdateValidRotationToScb();
    }
    auto task = [=] {
        auto clientProxy = GetClientProxy();
        if (!clientProxy) {
            TLOGNE(WmsLogTag::DMS, "ScbStatusRecoveryWhenSwitchUser clientProxy_ is null");
            return;
        }
        clientProxy->SwitchUserCallback(oldScbPids, newScbPid);
        RecoverMultiScreenModeWhenSwitchUser(oldScbPids, newScbPid);
    };
    taskScheduler_->PostAsyncTask(task, "clientProxy_ SwitchUserCallback task", delayTime);
#endif
}

Rotation ScreenSessionManager::GetOldDisplayModeRotation(FoldDisplayMode oldDisplayMode, Rotation rotation)
{
    auto currentDisplayMode = GetFoldDisplayMode();
    if (currentDisplayMode == oldDisplayMode) {
        return rotation;
    }
    auto oldRotationoffset = GetConfigCorrectionByDisplayMode(oldDisplayMode);
    auto rotationoffset = GetConfigCorrectionByDisplayMode(GetFoldDisplayMode());
    return static_cast<Rotation>(
        (static_cast<uint32_t>(rotation) - static_cast<uint32_t>(rotationoffset) +
        static_cast<uint32_t>(oldRotationoffset) + ROTATION_MOD) % ROTATION_MOD);
}
 
void ScreenSessionManager::SwapScreenWeightAndHeight(sptr<ScreenSession>& screenSession)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    RRect bounds = screenSession->GetScreenProperty().GetBounds();
    auto lastWidth = bounds.rect_.GetWidth();
    auto lastHeight = bounds.rect_.GetHeight();
    RRect afterBounds =
        RRect({ 0, bounds.rect_.GetTop(), lastHeight, lastWidth}, 0.0f, 0.0f);
    screenSession->SetBounds(afterBounds);
    TLOGI(WmsLogTag::DMS, "before width:%{public}f, height:%{public}f,after width:%{public}f, height:%{public}f",
        lastWidth, lastHeight, afterBounds.rect_.GetWidth(), afterBounds.rect_.GetHeight());
}
 
void ScreenSessionManager::OnVerticalChangeBoundsWhenSwitchUser(sptr<ScreenSession>& screenSession,
    FoldDisplayMode oldScbDisplayMode)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    auto bounds = screenSession->GetScreenProperty().GetBounds();
    auto rotation = screenSession->GetRotation();
    // set rotation to old displayMode
    auto oldRotation = GetOldDisplayModeRotation(oldScbDisplayMode_, screenSession->GetRotation());
    screenSession->SetRotation(oldRotation);
    screenSession->SetBounds(bounds);
    auto correctionRotation = GetConfigCorrectionByDisplayMode(GetFoldDisplayMode());
    float rotationValue = (static_cast<uint32_t>(rotation) - static_cast<uint32_t>(correctionRotation) +
        ROTATION_MOD) % ROTATION_MOD * SECONDARY_ROTATION_90;
    if (std::fabs(rotationValue - SECONDARY_ROTATION_90) < FLT_EPSILON ||
        std::fabs(rotationValue - SECONDARY_ROTATION_270) < FLT_EPSILON ) {
        RRect afterBounds =
            RRect({ 0, bounds.rect_.GetTop(), bounds.rect_.GetHeight(), bounds.rect_.GetWidth()}, 0.0f, 0.0f);
        screenSession->SetBounds(afterBounds);
        TLOGI(WmsLogTag::DMS, "before width:%{public}f, height:%{public}f,after width:%{public}f, height:%{public}f",
            bounds.rect_.GetWidth(), bounds.rect_.GetHeight(),
            afterBounds.rect_.GetWidth(), afterBounds.rect_.GetHeight());
    }
}

void ScreenSessionManager::RecoverMultiScreenModeWhenSwitchUser(std::vector<int32_t> oldScbPids, int32_t newScbPid)
{
    if (!g_isPcDevice) {
        TLOGI(WmsLogTag::DMS, "not PC device, return before recover.");
        return;
    }
    bool extendScreenConnected = false;
    ScreenId extendScreenId = SCREEN_ID_INVALID;
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMap;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        screenSessionMap = screenSessionMap_;
    }
    for (const auto& [screenId, screenSession] : screenSessionMap) {
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::REAL &&
            screenSession->GetIsExtend()) {
            TLOGI(WmsLogTag::DMS, "recover extend screen, screenId = %{public}" PRIu64, screenId);
            extendScreenConnected = true;
            extendScreenId = screenId;
            RecoverMultiScreenMode(screenSession);
        } else if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::REAL &&
            !screenSession->GetIsExtend()) {
            SetExtendedScreenFallbackPlan(screenId);
        }
    }
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        if (extendScreenConnected) {
            OnExtendScreenConnectStatusChange(extendScreenId, ExtendScreenConnectStatus::CONNECT);
        } else {
            OnExtendScreenConnectStatusChange(extendScreenId, ExtendScreenConnectStatus::DISCONNECT);
        }
    }
}

bool ScreenSessionManager::HandleSwitchPcMode()
{
    if (!IS_SUPPORT_PC_MODE) {
        return g_isPcDevice;
    }
    std::lock_guard<std::mutex> lock(pcModeSwitchMutex_);
    if (system::GetBoolParameter(IS_PC_MODE_KEY, false)) {
        TLOGI(WmsLogTag::DMS, "PcMode change isPcDevice true");
        g_isPcDevice = true;
    } else {
        TLOGI(WmsLogTag::DMS, "PadMode change isPcDevice false");
        g_isPcDevice = false;
    }
    return g_isPcDevice;
}

void ScreenSessionManager::SwitchModeHandleExternalScreen(bool isSwitchToPcMode)
{
    if(!IS_SUPPORT_PC_MODE){
        return;
    }
    ConfigureDpi();
    std::ostringstream oss;
    std::vector<ScreenId> externalScreenIds;
    bool hasExternalScreen = false;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (const auto& iter : screenSessionMap_) {
            auto screenSession = iter.second;
            if (screenSession == nullptr) {
                continue;
            }
            ScreenId screenId = screenSession->GetScreenId();
            if (IsDefaultMirrorMode(screenId) && screenSession->GetIsRealScreen()) {
                externalScreenIds.emplace_back(screenId);
                hasExternalScreen = true;
                SetScreenNameWhenSwitchMode(screenSession, isSwitchToPcMode);
                screenSession->SetVirtualScreenFlag(isSwitchToPcMode ?
                    VirtualScreenFlag::DEFAULT : VirtualScreenFlag::CAST);
                oss << screenId << ",";
                SwitchModeOffScreenRenderingResetScreenProperty(screenSession, isSwitchToPcMode);
                screenSession->SetDensityInCurResolution(densityDpi_);
            }
        }
    }
    TLOGI(WmsLogTag::DMS, "screenIds:%{public}s", oss.str().c_str());
    if (hasExternalScreen) {
        ScreenId screenGroupId = SCREEN_GROUP_ID_DEFAULT;
        MakeMirror(SCREEN_ID_DEFAULT, externalScreenIds, screenGroupId);
        if (!isSwitchToPcMode) {
            TLOGI(WmsLogTag::DMS, "notify cast screen connect");
            NotifyCastWhenScreenConnectChange(true);
            NotifyScreenModeChange();
            ScreenPowerUtils::EnablePowerForceTimingOut();
            DisablePowerOffRenderControl(SCREEN_ID_FULL);
        } else {
            ScreenPowerUtils::DisablePowerForceTimingOut();
        }
    }
    SwitchModeOffScreenRenderingAdapter(externalScreenIds);
    SetExtendScreenDpi();
}

void ScreenSessionManager::SwitchModeOffScreenRenderingResetScreenProperty(const sptr<ScreenSession>& externalScreenSession,
    bool isSwitchToPcMode)
{
    if (externalScreenSession == nullptr) {
        TLOGI(WmsLogTag::DMS, "externalScreenSession is nullptr");
        return;
    }
    ScreenId screenId = externalScreenSession->GetScreenId();
    auto screenProperty = externalScreenSession->GetScreenProperty();
    if (isSwitchToPcMode) {
        InitExtendScreenProperty(screenId, externalScreenSession, screenProperty);
    } else {
        uint32_t screenRealWidth = screenProperty.GetScreenRealWidth();
        uint32_t screenRealHeight = screenProperty.GetScreenRealHeight();
        auto screenBounds = RRect({ 0, 0, screenRealWidth, screenRealHeight }, 0.0f, 0.0f);
        externalScreenSession->SetExtendProperty(screenBounds, true);
        TLOGI(WmsLogTag::DMS, "SetExtendProperty screenId:%{public}" PRIu64", width:%{public}d, height:%{public}d",
            screenId, screenRealWidth, screenRealHeight);
    }
}

void ScreenSessionManager::SwitchModeOffScreenRenderingAdapter(const std::vector<ScreenId>& externalScreenIds)
{
    if (externalScreenIds.empty()) {
        TLOGI(WmsLogTag::DMS, "externalScreenIds is empty");
        return;
    }
    for (const auto& screenId : externalScreenIds) {
        SetExtendedScreenFallbackPlan(screenId);
    }
}

void ScreenSessionManager::SetScreenNameWhenSwitchMode(const sptr<ScreenSession>& screenSession, bool isSwitchToPcMode)
{
    if (screenSession == nullptr) {
        TLOGI(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    std::string screenName = "UNKNOWN";
    if (isSwitchToPcMode) {
        ScreenId rsScreenId = screenIdManager_.ConvertToRsScreenId(screenSession->GetScreenId());
        struct BaseEdid edid;
        if (GetEdid(rsScreenId, edid)) {
            screenName = edid.displayProductName_;
        } else {
            screenName = SCREEN_NAME_EXTEND;
        }
    } else {
        screenName = SCREEN_NAME_CAST;
    }
    screenSession->SetName(screenName);
    TLOGI(WmsLogTag::DMS, "screenName:%{public}s", screenName.c_str());
}

bool ScreenSessionManager::IsConcurrentUser()
{
    return ScreenSceneConfig::IsConcurrentUser();
}

bool ScreenSessionManager::ActiveUser(int32_t newUserId, int32_t& oldUserId, int32_t newScbPid)
{
    DisplayId displayId = DISPLAY_ID_INVALID;
    ErrCode err = AccountSA::OsAccountManager::GetForegroundOsAccountDisplayId(newUserId, displayId);
    if (err != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "active user failed, get user display failed, errorCode: %{public}d, user: %{public}d",
            err, newUserId);
        return false;
    }
    
    std::lock_guard<std::mutex> lock(displayConcurrentUserMapMutex_);
    oldUserId = INVALID_USER_ID;
    displayConcurrentUserMap_[displayId][newUserId] = { true, newScbPid };
    TLOGI(WmsLogTag::DMS, "Get user display success, add or update user info in displayConcurrentUserMap,"
          "newUsrId: %{public}d, displayId: %{public}" PRIu64 ", newScbPid: %{public}d",
          newUserId, displayId, newScbPid);
    for (auto& [userId, UserInfo] : displayConcurrentUserMap_[displayId]) {
        if (userId == newUserId) {
            continue;
        } else {
            if (UserInfo.isForeground) {
                oldUserId = userId;
                UserInfo.isForeground = false;
                TLOGI(WmsLogTag::DMS, "deactivate user, userId: %{public}d, displayId: %{public}" PRIu64,
                    userId, displayId);
            }
        }
    }
    return true;
}

DisplayId ScreenSessionManager::GetUserDisplayId(int32_t userId) const
{
    std::lock_guard<std::mutex> lock(displayConcurrentUserMapMutex_);
    for (const auto& [displayId, userMap] : displayConcurrentUserMap_ ) {
        auto it = userMap.find(userId);
        if (it != userMap.end()) {
            // If the pid in deathPidVector, it means the user already been deactivated
            if (!CheckPidInDeathPidVector(it->second.pid)) {
                TLOGI(WmsLogTag::DMS, "find displayId: %{public}" PRIu64 "with userId: %{public}d",
                    displayId, userId);
                return displayId;
            }
        }
    }
    TLOGI(WmsLogTag::DMS, "find no displayId with userId: %{public}d", userId);
    return DISPLAY_ID_INVALID;
}

void ScreenSessionManager::SetClient(const sptr<IScreenSessionManagerClient>& client)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied, clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    if (!client) {
        TLOGE(WmsLogTag::DMS, "SetClient client is null");
        return;
    }
    bool isPcMode = HandleSwitchPcMode();
    if (g_isPcDevice && userSwitching_) {
        std::unique_lock<std::mutex> lock(switchUserMutex_);
        if (DmUtils::safe_wait_for(switchUserCV_, lock, std::chrono::milliseconds(CV_WAIT_USERSWITCH_MS)) == std::cv_status::timeout) {
            TLOGW(WmsLogTag::DMS, "wait switchUserCV_ timeout");
            userSwitching_ = false;
        }
    }
    {
        std::unique_lock<std::mutex> lock(oldScbPidsMutex_);
        SetClientProxy(client);
        SwitchModeHandleExternalScreen(isPcMode);
        auto userId = GetUserIdByCallingUid();
        auto newScbPid = IPCSkeleton::GetCallingPid();
        {
            std::lock_guard<std::recursive_mutex> lock(userPidMapMutex_);
            userPidMap_[userId] = newScbPid;
        }
        std::ostringstream oss;
        oss << "set client userId: " << userId << " clientName: " << SysCapUtil::GetClientName();
        TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
        screenEventTracker_.RecordEvent(oss.str());
        currentUserIdForSettings_ = userId;
        int32_t oldUserId = currentUserId_;
        DisplayId defaultId = GetDefaultScreenId();
        if (IsConcurrentUser()) {
            if (ActiveUser(userId, oldUserId, newScbPid)) {
                TLOGI(WmsLogTag::DMS, "Activeuser success, oldUserId: %{public}d, newUserId: %{public}d",
                    oldUserId, userId);
            } else {
                TLOGI(WmsLogTag::DMS, "Activeuser failed, user oldUserId: %{public}d", oldUserId);
            }
            DisplayId id = GetUserDisplayId(userId);
            if (id != DISPLAY_ID_INVALID) {
                defaultId = id;
            } else {
                TLOGE(WmsLogTag::DMS, "get invalid display id, user: %{public}d", userId);
            }
        }
        MockSessionManagerService::GetInstance().NotifyWMSConnected(userId, defaultId, true);
        NotifyClientProxyUpdateFoldDisplayMode(GetFoldDisplayMode());
        SetClientInner(userId, client);
        SwitchScbNodeHandle(userId, newScbPid, true);
    }
    AddScbClientDeathRecipient(client, IPCSkeleton::GetCallingPid());

    static bool isNeedSwitchScreen = FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice() ||
        FoldScreenStateInternel::IsSingleDisplayFoldDevice() ||
        FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice();
    if (isNeedSwitchScreen) {
        FoldDisplayMode displayMode = GetFoldDisplayMode();
        if (displayMode == FoldDisplayMode::FULL) {
            TLOGI(WmsLogTag::DMS, "switch screen to full");
            SetDisplayNodeScreenId(SCREEN_ID_FULL, SCREEN_ID_FULL);
        } else if (displayMode == FoldDisplayMode::MAIN) {
            TLOGI(WmsLogTag::DMS, "switch screen to main");
            SetDisplayNodeScreenId(SCREEN_ID_FULL, SCREEN_ID_MAIN);
        }
    }
}

void ScreenSessionManager::SwitchScbNodeHandle(int32_t newUserId, int32_t newScbPid, bool coldBoot)
{
#ifdef WM_MULTI_USR_ABILITY_ENABLE
    std::ostringstream oss;
    oss << "currentUserId: " << currentUserId_
        << "  currentScbPId: " << currentScbPId_
        << "  newUserId: " << newUserId
        << "  newScbPid: " << newScbPid
        << "  coldBoot: " << static_cast<int32_t>(coldBoot);
    TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    screenEventTracker_.RecordEvent(oss.str());

    if (currentUserId_ != newUserId) {
        HandleNewUserDisplayNode(newUserId, coldBoot);
    }

    if (currentScbPId_ != INVALID_SCB_PID) {
        auto pidIter = std::find(oldScbPids_.begin(), oldScbPids_.end(), currentScbPId_);
        if (pidIter == oldScbPids_.end() && currentScbPId_ > 0) {
            oldScbPids_.emplace_back(currentScbPId_);
        }
        oldScbPids_.erase(std::remove(oldScbPids_.begin(), oldScbPids_.end(), newScbPid), oldScbPids_.end());
        if (oldScbPids_.size() == 0) {
            TLOGE(WmsLogTag::DMS, "swicth user failed, oldScbPids is null");
            screenEventTracker_.RecordEvent("swicth user failed, oldScbPids is null");
        }
    }
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGE(WmsLogTag::DMS, "clientProxy is null");
        return;
    }
    if (coldBoot) {
        clientProxy->SwitchUserCallback(oldScbPids_, newScbPid);
        {
            std::lock_guard<std::mutex> lock(multiClientProxyMapMutex_);
            multiClientProxyMap_[newUserId] = clientProxy;
        }
    } else {
        HotSwitch(newUserId, newScbPid);
    }
    UpdateDisplayScaleState(GetDefaultScreenId());
    if (currentUserId_ != newUserId) {
        WaitSwitchUserAnimateFinish(newUserId, coldBoot);
    }
    currentUserId_ = newUserId;
    currentScbPId_ = newScbPid;
    // After switching users, the listener will become invalid and needs to be re-registered.
    ScreenSettingHelper::UnregisterSettingResolutionEffectObserver();
    RegisterSettingResolutionEffectObserver();
    scbSwitchCV_.notify_all();
    oldScbDisplayMode_ = GetFoldDisplayMode();
#endif
}

void ScreenSessionManager::NotifyCastWhenSwitchScbNode()
{
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMapCopy;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        screenSessionMapCopy = screenSessionMap_;
    }
    for (const auto& sessionIt : screenSessionMapCopy) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr, screenId:%{public}" PRIu64"", sessionIt.first);
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() != ScreenType::REAL ||
            !IsDefaultMirrorMode(screenSession->GetScreenId())) {
            TLOGE(WmsLogTag::DMS, "screen is not real or external, screenId:%{public}" PRIu64"", sessionIt.first);
            continue;
        }
        bool isScreenMirror = screenSession ->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR;
        NotifyCastWhenScreenConnectChange(isScreenMirror);
        return;
    }
}

void ScreenSessionManager::HotSwitch(int32_t newUserId, int32_t newScbPid)
{
    // hot switch
    if (multiClientProxyMap_.count(newUserId) == 0) {
        TLOGE(WmsLogTag::DMS, "not found client proxy. userId:%{public}d.", newUserId);
        return;
    }
    if (newUserId == currentUserId_) {
        TLOGI(WmsLogTag::DMS, "switch user not change");
        return;
    }
    if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice() ||
        FoldScreenStateInternel::IsSingleDisplaySuperFoldDevice()) {
        // Delete the screen whose ID is 5 generated by Coordination before switching the private space.
        SessionOption option = {
            .screenId_ = SCREEN_ID_MAIN,
        };
        auto clientProxy = GetClientProxy();
        if (clientProxy) {
            clientProxy->OnScreenConnectionChanged(option, ScreenEvent::DISCONNECTED);
        }
    }
    SetClientProxy(multiClientProxyMap_[newUserId]);
    ScbStatusRecoveryWhenSwitchUser(oldScbPids_, newScbPid);
}

int32_t ScreenSessionManager::GetCurrentUserId()
{
    return currentUserIdForSettings_;
}

void ScreenSessionManager::SetClientInner(int32_t newUserId, const sptr<IScreenSessionManagerClient>& clientProxy)
{
    if (!clientProxy) {
        TLOGE(WmsLogTag::DMS, "clientProxy is null");
        return;
    }
    SwitchUserDealUserDisplayNode(newUserId);
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    DisplayId targetDisplay = GetUserDisplayId(newUserId);
    bool isSuperFoldDeviceBootUp = FoldScreenStateInternel::IsSuperFoldDisplayDevice() && GetFirstSCBConnect();
    for (auto iter : screenSessionMap_) {
        if (!iter.second) {
            TLOGI(WmsLogTag::DMS, "not notify screen: %{public}" PRIu64 " to user: %{public}d", iter.first, newUserId);
            continue;
        }
        if (isSuperFoldDeviceBootUp && iter.second->isInternal_) {
            iter.second->SetHorizontalRotation();
        }
        if (IsConcurrentUser()) {
            if (targetDisplay != DISPLAY_ID_INVALID && targetDisplay != iter.first) {
                continue;
            }
        }
        TLOGI(WmsLogTag::DMS, "notify screen: %{public}" PRIu64" to user: %{public}d", iter.first, newUserId);
        // In the rotating state, after scb restarts, the screen information needs to be reset.
        HandleScreenRotationAndBoundsWhenSetClient(iter.second);
        if (iter.second->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
            TLOGI(WmsLogTag::DMS, "current screen is extend and mirror, return before OnScreenConnectionChanged");
            RecoverMultiScreenMode(iter.second);
            continue;
        }
        clientProxy->OnScreenConnectionChanged(GetSessionOption(iter.second, iter.first), ScreenEvent::CONNECTED);
        RecoverMultiScreenMode(iter.second);
    }
}  

void ScreenSessionManager::HandleScreenRotationAndBoundsWhenSetClient(sptr<ScreenSession>& screenSession)
{
    float phyWidth = 0.0f;
    float phyHeight = 0.0f;
    bool isReset = true;
    int boundaryOffset = 0;
    ScreenId screenId = screenSession->GetScreenId();
    GetCurrentScreenPhyBounds(phyWidth, phyHeight, isReset, screenId);
    auto localRotation = screenSession->GetRotation();
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        FoldDisplayMode displayMode = GetFoldDisplayMode();
        if (displayMode == FoldDisplayMode::FULL) {
            boundaryOffset = screenParams_[FULL_STATUS_OFFSET_X];
            phyWidth = screenParams_[FULL_STATUS_WIDTH];
        } else if (displayMode == FoldDisplayMode::MAIN) {
            phyWidth = screenParams_[MAIN_STATUS_WIDTH];
            phyHeight = screenParams_[MAIN_STATUS_HEIGHT];
        }
    }
    TLOGI(WmsLogTag::DMS, "phyWidth:%{public}f, phyHeight:%{public}f, localRotation:%{public}u",
        phyWidth, phyHeight, localRotation);
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        auto bounds = screenSession->GetScreenProperty().GetBounds();
        screenSession->SetRotation(Rotation::ROTATION_0);
        TLOGI(WmsLogTag::DMS, "rotation:%{public}d", screenSession->GetRotation());
        screenSession->SetBounds(bounds);
        auto rotationWithoutCorrection = RemoveRotationCorrection(localRotation);
        if (rotationWithoutCorrection == Rotation::ROTATION_90 || rotationWithoutCorrection == Rotation::ROTATION_270) {
            SwapScreenWeightAndHeight(screenSession);
        }
    } else if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        bool isModeChanged = localRotation != Rotation::ROTATION_0;
        if (isModeChanged && isReset) {
            TLOGI(WmsLogTag::DMS, "screen(id:%{public}" PRIu64 ") current is not default mode, reset it", screenId);
            SetRotation(screenId, Rotation::ROTATION_0, false);
            SetPhysicalRotationClientInner(screenId, 0);
            screenSession->SetDisplayBoundary(RectF(0, boundaryOffset, phyWidth, phyHeight), 0);
        }
    }
}

void ScreenSessionManager::SetPhysicalRotationClientInner(ScreenId screenId, int rotation)
{
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail, cannot find screen %{public}" PRIu64"",
            screenId);
        return;
    }
    screenSession->SetPhysicalRotation(rotation);
    screenSession->SetScreenComponentRotation(rotation);
    TLOGI(WmsLogTag::DMS, "SetPhysicalRotationClientInner end");
}

void ScreenSessionManager::HandleDefaultMultiScreenMode(sptr<ScreenSession> internalSession,
    sptr<ScreenSession> screenSession)
{
    if (internalSession == nullptr || screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "Session is nullptr");
        return;
    }
    ScreenId innerRsId = internalSession->GetRSScreenId();
    ScreenId externalRsId = screenSession->GetRSScreenId();
    if (innerRsId == externalRsId) {
        TLOGW(WmsLogTag::DMS, "same rsId: %{public}" PRIu64, innerRsId);
        return;
    }
    if (IS_SUPPORT_PC_MODE) {
        TLOGI(WmsLogTag::DMS, "default mode mirror");
        SetMultiScreenMode(innerRsId, externalRsId, MultiScreenMode::SCREEN_MIRROR);
        ReportHandleScreenEvent(ScreenEvent::CONNECTED, ScreenCombination::SCREEN_MIRROR);
    } else {
        TLOGI(WmsLogTag::DMS, "default mode extend");
        SetMultiScreenMode(innerRsId, externalRsId, MultiScreenMode::SCREEN_EXTEND);
        SetMultiScreenDefaultRelativePosition();
        ReportHandleScreenEvent(ScreenEvent::CONNECTED, ScreenCombination::SCREEN_EXTEND);
    }
}

void ScreenSessionManager::RecoverMultiScreenMode(sptr<ScreenSession> screenSession)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null!");
        return;
    }
    if (!g_isPcDevice || screenSession->GetScreenProperty().GetScreenType() != ScreenType::REAL) {
        TLOGI(WmsLogTag::DMS, "not PC or not real screen, no need recover!");
        return;
    }
    if (isDeviceShutDown_) {
        TLOGI(WmsLogTag::DMS, "device shut down, no need recover!");
        return;
    }
    sptr<ScreenSession> internalSession = GetInternalScreenSession();
    if (!RecoverRestoredMultiScreenMode(screenSession)) {
        HandleDefaultMultiScreenMode(internalSession, screenSession);
    }
    sptr<ScreenSession> newInternalSession = GetInternalScreenSession();
    if (newInternalSession != nullptr && internalSession != nullptr &&
        internalSession->GetScreenId() != newInternalSession->GetScreenId()) {
        TLOGW(WmsLogTag::DMS, "main screen changed, reset screenSession.");
        screenSession = internalSession;
    }
    SetExtendedScreenFallbackPlan(screenSession->GetScreenId());
    if (screenSession->GetScreenCombination() == ScreenCombination::SCREEN_EXTEND) {
        screenSession->PropertyChange(screenSession->GetScreenProperty(), ScreenPropertyChangeReason::UNDEFINED);
    }
}

void ScreenSessionManager::GetCurrentScreenPhyBounds(float& phyWidth, float& phyHeight,
                                                     bool& isReset, const ScreenId& screenId)
{
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ != nullptr) {
        FoldDisplayMode displayMode = GetFoldDisplayMode();
        TLOGI(WmsLogTag::DMS, "fold screen with displayMode = %{public}u", displayMode);
        if (FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
            auto phyBounds = GetPhyScreenProperty(screenId).GetPhyBounds();
            phyWidth = phyBounds.rect_.width_;
            phyHeight = phyBounds.rect_.height_;
            if (displayMode == FoldDisplayMode::UNKNOWN) {
                isReset = false;
            }
            return;
        }
        if (displayMode == FoldDisplayMode::MAIN) {
            auto phyBounds = GetPhyScreenProperty(SCREEN_ID_MAIN).GetPhyBounds();
            phyWidth = phyBounds.rect_.width_;
            phyHeight = phyBounds.rect_.height_;
        } else if (displayMode == FoldDisplayMode::FULL || displayMode == FoldDisplayMode::GLOBAL_FULL) {
            auto phyBounds = GetPhyScreenProperty(SCREEN_ID_FULL).GetPhyBounds();
            phyWidth = phyBounds.rect_.width_;
            phyHeight = phyBounds.rect_.height_;
            if (g_screenRotationOffSet == ROTATION_90 || g_screenRotationOffSet == ROTATION_270) {
                std::swap(phyWidth, phyHeight);
            }
        } else {
            isReset = false;
        }
        return;
    }
#endif
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("GetCurrentScreenPhyBounds", XCOLLIE_TIMEOUT_10S, nullptr,
        nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
    ScreenId rsScreenId = screenId;
    if (IsConcurrentUser()) {
        if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
            rsScreenId = screenId;
        }
    }
    auto remoteScreenMode = rsInterface_.GetScreenActiveMode(rsScreenId);
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    phyWidth = remoteScreenMode.GetScreenWidth();
    phyHeight = remoteScreenMode.GetScreenHeight();
}

ScreenProperty ScreenSessionManager::GetScreenProperty(ScreenId screenId)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied.calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return {};
    }
    DmsXcollie dmsXcollie("DMS:GetScreenProperty", XCOLLIE_TIMEOUT_10S);
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGI(WmsLogTag::DMS, "screenSession is null");
        return {};
    }
    return screenSession->GetScreenProperty();
}

std::shared_ptr<RSDisplayNode> ScreenSessionManager::GetDisplayNode(ScreenId screenId)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied.calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return nullptr;
    }
    DmsXcollie dmsXcollie("DMS:GetDisplayNode", XCOLLIE_TIMEOUT_10S);
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return nullptr;
    }
    return screenSession->GetDisplayNode();
}

ScreenCombination ScreenSessionManager::GetScreenCombination(ScreenId screenId)
{
    DmsXcollie dmsXcollie("DMS:GetScreenCombination", XCOLLIE_TIMEOUT_10S);
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGI(WmsLogTag::DMS, "screenSession is null");
        return ScreenCombination::SCREEN_ALONE;
    }
    return screenSession->GetScreenCombination();
}

int ScreenSessionManager::Dump(int fd, const std::vector<std::u16string>& args)
{
    TLOGI(WmsLogTag::DMS, "Dump begin");
    sptr<ScreenSessionDumper> dumper = new ScreenSessionDumper(fd, args);
    if (dumper == nullptr) {
        TLOGE(WmsLogTag::DMS, "dumper is nullptr");
        return -1;
    }
    {
        std::lock_guard<std::mutex> lock(freezedPidListMutex_);
        dumper->DumpFreezedPidList(freezedPidList_);
    }
    dumper->DumpEventTracker(screenEventTracker_);
    {
        std::lock_guard<std::mutex> lock(oldScbPidsMutex_);
        dumper->DumpMultiUserInfo(oldScbPids_, currentUserId_, currentScbPId_);
    }
    dumper->ExecuteDumpCmd();
    TLOGI(WmsLogTag::DMS, "dump end");
    return 0;
}

void ScreenSessionManager::TriggerFoldStatusChange(FoldStatus foldStatus)
{
#ifdef FOLD_ABILITY_ENABLE
    TLOGI(WmsLogTag::DMS, "enter foldStatus = %{public}d.", foldStatus);
    if (foldScreenController_ == nullptr) {
        return;
    }
    foldScreenController_->SetFoldStatus(foldStatus);
    FoldDisplayMode displayMode = foldScreenController_->GetModeMatchStatus();
    SetFoldDisplayMode(displayMode);
    NotifyFoldStatusChanged(foldStatus);
#endif
}

int ScreenSessionManager::NotifyFoldStatusChanged(const std::string& statusParam)
{
#ifdef FOLD_ABILITY_ENABLE
    TLOGI(WmsLogTag::DMS, "is dump log");
    if (statusParam.empty()) {
        return -1;
    }
    FoldStatus foldStatus = FoldStatus::UNKNOWN;
    FoldDisplayMode displayMode = FoldDisplayMode::UNKNOWN;
    if (statusParam == STATUS_FOLD_HALF) {
        foldStatus = FoldStatus::HALF_FOLD;
        displayMode = FoldDisplayMode::FULL;
    } else if (statusParam == STATUS_EXPAND) {
        foldStatus = FoldStatus::EXPAND;
        displayMode = FoldDisplayMode::FULL;
    } else if (statusParam == STATUS_FOLD) {
        foldStatus = FoldStatus::FOLDED;
        displayMode = FoldDisplayMode::MAIN;
    } else if (statusParam == STATUS_EXPAND_WITH_SECOND_EXPAND) {
        foldStatus = FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND;
        displayMode = FoldDisplayMode::GLOBAL_FULL;
    } else {
        TLOGW(WmsLogTag::DMS, "status not support");
        return -1;
    }
    SetFoldDisplayMode(displayMode);
    if (foldScreenController_ != nullptr) {
        foldScreenController_->SetFoldStatus(foldStatus);
    }
    NotifyFoldStatusChanged(foldStatus);
#endif
    return 0;
}

void ScreenSessionManager::NotifyAvailableAreaChanged(DMRect area, DisplayId displayId)
{
    ScreenSessionManagerAdapter::GetInstance().NotifyAvailableAreaChanged(area, displayId);
}

DMError ScreenSessionManager::GetAvailableArea(DisplayId displayId, DMRect& area)
{
    auto displayInfo = GetDisplayInfoById(displayId);
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get displayInfo.");
        return DMError::DM_ERROR_NULLPTR;
    }
    sptr<ScreenSession> screenSession;
    if (displayId == DISPLAY_ID_FAKE) {
        if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
            return DMError::DM_ERROR_NULLPTR;
        }
        ScreenId internalScreenId = GetInternalScreenId();
        sptr<ScreenSession> internalScreenSession = GetScreenSession(internalScreenId);
        if (internalScreenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "internal session is nullptr.");
            return DMError::DM_ERROR_NULLPTR;
        }
        if (!internalScreenSession->GetScreenProperty().GetIsFakeInUse()) {
            return DMError::DM_ERROR_NULLPTR;
        }
        screenSession = internalScreenSession->GetFakeScreenSession();
    } else {
        screenSession = GetScreenSession(displayInfo->GetScreenId());
    }
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get screen now");
        return DMError::DM_ERROR_NULLPTR;
    }
    area = screenSession->GetAvailableArea();
    return DMError::DM_OK;
}

DMError ScreenSessionManager::GetExpandAvailableArea(DisplayId displayId, DMRect& area)
{
    auto displayInfo = GetDisplayInfoById(displayId);
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get displayInfo.");
        return DMError::DM_ERROR_NULLPTR;
    }
    sptr<ScreenSession> screenSession;
    screenSession = GetScreenSession(displayInfo->GetScreenId());
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get screen now");
        return DMError::DM_ERROR_NULLPTR;
    }
    area = screenSession->GetExpandAvailableArea();
    return DMError::DM_OK;
}

void ScreenSessionManager::UpdateAvailableArea(ScreenId screenId, DMRect area)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "update available area permission denied!");
        return;
    }
    if (GetUserIdByCallingUid() != currentUserId_) {
        TLOGE(WmsLogTag::DMS, "not currentuser, calling uid:%{public}d, current uid:%{public}d",
            GetUserIdByCallingUid(), currentUserId_);
        return;
    }
    auto screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get default screen now");
        return;
    }
    if (!screenSession->UpdateAvailableArea(area)) {
        return;
    }
    if (g_isPcDevice) {
        {
            std::unique_lock<std::mutex> lock(displayAddMutex_);
            if (needWaitAvailableArea_) {
                TLOGI(WmsLogTag::DMS, "need notify add display.");
                displayAddCV_.notify_all();
                needWaitAvailableArea_ = false;
            }
        }
        sptr<ScreenSession> physicalScreen = GetPhysicalScreenSession(screenSession->GetRSScreenId());
        if (physicalScreen) {
            physicalScreen->UpdateAvailableArea(area);
        }
    }
    NotifyAvailableAreaChanged(area, screenId);
}

void ScreenSessionManager::UpdateSuperFoldAvailableArea(ScreenId screenId, DMRect bArea, DMRect cArea)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "update super fold available area permission denied!");
        return;
    }

    auto screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get default screen now");
        return;
    }
    if (screenSession->UpdateAvailableArea(bArea)) {
        if (g_isPcDevice) {
            sptr<ScreenSession> physicalScreen = GetPhysicalScreenSession(screenSession->GetRSScreenId());
            if (physicalScreen) {
                physicalScreen->UpdateAvailableArea(bArea);
            }
        }
        NotifyAvailableAreaChanged(bArea, screenId);
        NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(),
            DisplayChangeEvent::SUPER_FOLD_AVAILABLE_AREA_UPDATE);
    }
    if (!screenSession->GetIsFakeInUse()) {
        TLOGE(WmsLogTag::DMS, "fake screen session is not in use");
        return;
    }
    auto fakeScreenSession = screenSession->GetFakeScreenSession();
    if (fakeScreenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get fake screen now");
        return;
    }
    if (fakeScreenSession->UpdateAvailableArea(cArea) && cArea.width_ > 0) {
        NotifyAvailableAreaChanged(cArea, fakeScreenSession->GetScreenId());
        NotifyDisplayChanged(fakeScreenSession->ConvertToDisplayInfo(),
            DisplayChangeEvent::SUPER_FOLD_AVAILABLE_AREA_UPDATE);
    }
}

void ScreenSessionManager::UpdateSuperFoldExpandAvailableArea(ScreenId screenId, DMRect area)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "update super fold available area permission denied!");
        return;
    }

    auto screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get default screen now");
        return;
    }
    if (screenSession->UpdateExpandAvailableArea(area)) {
        TLOGI(WmsLogTag::DMS,
            "ExpandAvailableArea x: %{public}d, y: %{public}d, width: %{public}d, height: %{public}d",
            area.posX_, area.posY_, area.width_, area.height_);
    }
    if (g_isPcDevice) {
        sptr<ScreenSession> physicalScreen = GetPhysicalScreenSession(screenSession->GetRSScreenId());
        if (physicalScreen) {
            physicalScreen->UpdateExpandAvailableArea(area);
        }
    }
}

void ScreenSessionManager::NotifyFoldToExpandCompletion(bool foldToExpand)
{
#ifdef FOLD_ABILITY_ENABLE
    TLOGI(WmsLogTag::DMS, "ENTER");
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied, clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    if (!FoldScreenStateInternel::IsDualDisplayFoldDevice() &&
        !FoldScreenStateInternel::IsSuperFoldDisplayDevice() &&
        !FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        SetDisplayNodeScreenId(SCREEN_ID_FULL, foldToExpand ? SCREEN_ID_FULL : SCREEN_ID_MAIN);
    }
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        SetDisplayNodeScreenId(SCREEN_ID_FULL, SCREEN_ID_FULL);
    }
    /* Avoid fold to expand process queues */
    if (foldScreenController_ != nullptr) {
        foldScreenController_->SetdisplayModeChangeStatus(false);
        foldScreenController_->NotifyRunSensorFoldStateManager();
    }
    sptr<ScreenSession> screenSession = GetDefaultScreenSession();
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "fail to get default screenSession");
        return;
    }
    screenSession->UpdateRotationAfterBoot(foldToExpand);
#endif
}

void ScreenSessionManager::RecordEventFromScb(std::string description, bool needRecordEvent)
{
    TLOGD(WmsLogTag::DMS, "%{public}s", description.c_str());
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied, clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    if (needRecordEvent) {
        screenEventTracker_.RecordEvent(description);
    }
}

void ScreenSessionManager::CheckAndSendHiSysEvent(const std::string& eventName, const std::string& bundleName) const
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:CheckAndSendHiSysEvent");
    if (eventName != "CREATE_VIRTUAL_SCREEN") {
        if (!Permission::CheckIsCallingBundleName(bundleName)) {
            TLOGD(WmsLogTag::DMS, "BundleName not in whitelist!");
            return;
        }
    }
    int32_t eventRet = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        eventName, // CREATE_VIRTUAL_SCREEN, GET_DISPLAY_SNAPSHOT
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "PID", getpid(),
        "UID", getuid());
    TLOGI(WmsLogTag::DMS, "%{public}s: Write HiSysEvent ret:%{public}d", eventName.c_str(), eventRet);
}

DMError ScreenSessionManager::ProxyForFreeze(const std::set<int32_t>& pidList, bool isProxy)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    {
        std::lock_guard<std::mutex> lock(freezedPidListMutex_);
        for (auto pid : pidList) {
            if (isProxy) {
                freezedPidList_.insert(pid);
            } else {
                freezedPidList_.erase(pid); // set删除不存在的元素不会引发异常
            }
        }
    }
    if (isProxy) {
        return DMError::DM_OK;
    }

    // 进程解冻时刷新一次displaychange
    sptr<ScreenSession> screenSession = GetScreenSession(GetDefaultScreenId());
    if (!screenSession) {
        return DMError::DM_ERROR_NULLPTR;
    }
    auto task = [=] {
        NotifyUnfreezed(pidList, screenSession);
    };
    taskScheduler_->PostAsyncTask(task, "ProxyForUnFreeze NotifyDisplayChanged");
    return DMError::DM_OK;
}

void ScreenSessionManager::NotifyUnfreezedAgents(const int32_t& pid, const std::set<int32_t>& unfreezedPidList,
    const std::set<DisplayManagerAgentType>& pidAgentTypes, const sptr<ScreenSession>& screenSession)
{
    bool isAgentTypeNotify = false;
    for (auto agentType : pidAgentTypes) {
        auto agents = ScreenSessionManagerAdapter::GetInstance().dmAgentContainer_.GetAgentsByType(agentType);
        for (auto agent : agents) {
            int32_t agentPid = ScreenSessionManagerAdapter::GetInstance().dmAgentContainer_.GetAgentPid(agent);
            if (agent == nullptr|| agentPid != pid || unfreezedPidList.count(pid) == 0) {
                continue;
            }
            isAgentTypeNotify = true;
            if (agentType == DisplayManagerAgentType::DISPLAY_EVENT_LISTENER) {
                agent->OnDisplayChange(screenSession->ConvertToDisplayInfo(), DisplayChangeEvent::DISPLAY_UNFREEZED);
            } else if (agentType == DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER) {
                FoldDisplayMode displayMode = GetFoldDisplayMode();
                agent->NotifyDisplayModeChanged(displayMode);
            } else if (agentType == DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER) {
                FoldStatus foldStatus = GetFoldStatus();
                agent->NotifyFoldStatusChanged(foldStatus);
            } else if (agentType == DisplayManagerAgentType::FOLD_ANGLE_CHANGED_LISTENER) {
                std::lock_guard<std::mutex> lock(lastStatusUpdateMutex_);
                agent->NotifyFoldAngleChanged(lastFoldAngles_);
            } else if (agentType == DisplayManagerAgentType::SCREEN_EVENT_LISTENER) {
                auto displayInfo = screenSession->ConvertToDisplayInfo();
                auto screenInfo = GetScreenInfoById(displayInfo->GetScreenId());
                std::lock_guard<std::mutex> lock(lastStatusUpdateMutex_);
                agent->OnScreenChange(screenInfo, lastScreenChangeEvent_);
            } else if (agentType ==  DisplayManagerAgentType::DISPLAY_UPDATE_LISTENER) {
                std::lock_guard<std::mutex> lock(lastStatusUpdateMutex_);
                agent->NotifyDisplayChangeInfoChanged(lastDisplayChangeInfo_);
            } else if (agentType ==  DisplayManagerAgentType::AVAILABLE_AREA_CHANGED_LISTENER) {
                auto area = screenSession->GetAvailableArea();
                auto displayId = screenSession->ConvertToDisplayInfo()->GetDisplayId();
                std::lock_guard<std::mutex> lock(lastStatusUpdateMutex_);
                agent->NotifyAvailableAreaChanged(area, displayId);
            } else {
                isAgentTypeNotify = false;
                TLOGI(WmsLogTag::DMS, "Unknown agentType.");
            }
        }
        if (isAgentTypeNotify) {
            pidAgentTypeMap_[pid].erase(agentType);
        }
    }
}

void ScreenSessionManager::NotifyUnfreezed(const std::set<int32_t>& unfreezedPidList,
    const sptr<ScreenSession>& screenSession)
{
    std::lock_guard<std::mutex> lock(freezedPidListMutex_);
    std::ostringstream oss;
    oss << "pid,type:";
    for (auto iter = pidAgentTypeMap_.begin(); iter != pidAgentTypeMap_.end();) {
        int32_t pid = iter->first;
        auto pidAgentTypes = iter->second;
        NotifyUnfreezedAgents(pid, unfreezedPidList, pidAgentTypes, screenSession);
        if (pidAgentTypeMap_[pid].empty()) {
            iter = pidAgentTypeMap_.erase(iter);
        } else {
            iter++;
        }
        oss << pid << ",";
        for (auto type : pidAgentTypes) {
            oss << static_cast<int32_t>(type) << " ";
        }
        oss << "|";
    }
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
}

DMError ScreenSessionManager::ResetAllFreezeStatus()
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    std::lock_guard<std::mutex> lock(freezedPidListMutex_);
    freezedPidList_.clear();
    pidAgentTypeMap_.clear();
    TLOGI(WmsLogTag::DMS, "freezedPidList_ has been clear.");
    return DMError::DM_OK;
}

DeviceScreenConfig ScreenSessionManager::GetDeviceScreenConfig()
{
    DmsXcollie dmsXcollie("DMS:GetDeviceScreenConfig", XCOLLIE_TIMEOUT_10S);
    return deviceScreenConfig_;
}

void ScreenSessionManager::RegisterApplicationStateObserver()
{
#if defined(SENSOR_ENABLE) && defined(FOLD_ABILITY_ENABLE)
    std::string identify = IPCSkeleton::ResetCallingIdentity();
    DMS::ScreenSensorMgr::GetInstance().RegisterApplicationStateObserver();
    IPCSkeleton::SetCallingIdentity(identify);
#endif
}

void ScreenSessionManager::SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList,
    std::vector<uint64_t> surfaceIdList, std::vector<uint8_t> typeBlackList)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return;
    }
    TLOGW(WmsLogTag::DMS, "screenId: %{public}" PRIu64, screenId);
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    if (!ConvertScreenIdToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId");
        return;
    }
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGE(WmsLogTag::DMS, "clientProxy_ is nullptr");
        return;
    }
    if (windowIdList.empty()) {
        TLOGI(WmsLogTag::DMS, "WindowIdList is empty");
        clientProxy->OnSetSurfaceNodeIdsChanged(rsScreenId, surfaceIdList);
        rsInterface_.SetVirtualScreenTypeBlackList(rsScreenId, typeBlackList);
        return;
    }
    std::vector<uint64_t> surfaceNodeIdsToRS;
    clientProxy->OnGetSurfaceNodeIdsFromMissionIdsChanged(windowIdList, surfaceNodeIdsToRS);
    if (!surfaceIdList.empty()) {
        for (auto surfaceId : surfaceIdList) {
            auto it = std::find(surfaceNodeIdsToRS.begin(), surfaceNodeIdsToRS.end(), surfaceId);
            if (it != surfaceNodeIdsToRS.end()) {
                continue;
            }
            surfaceNodeIdsToRS.push_back(surfaceId);
        }
    }
    std::ostringstream oss;
    oss << "surfaceNodeIdsToRS[" << rsScreenId << "]: ";
    for (auto val : surfaceNodeIdsToRS) {
        oss << val << " ";
    }
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    rsInterface_.SetVirtualScreenTypeBlackList(rsScreenId, typeBlackList);
    clientProxy->OnSetSurfaceNodeIdsChanged(rsScreenId, surfaceNodeIdsToRS);
}

void ScreenSessionManager::SetVirtualDisplayMuteFlag(ScreenId screenId, bool muteFlag)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return;
    }
    sptr<ScreenSession> virtualScreenSession = GetScreenSession(screenId);
    if (!virtualScreenSession) {
        TLOGE(WmsLogTag::DMS, "ScreenSession is null");
        return;
    }
    std::shared_ptr<RSDisplayNode> virtualDisplayNode = virtualScreenSession->GetDisplayNode();
    if (virtualDisplayNode) {
        virtualDisplayNode->SetVirtualScreenMuteStatus(muteFlag);
    } else {
        TLOGE(WmsLogTag::DMS, "DisplayNode is null");
        return;
    }
    TLOGI(WmsLogTag::DMS, "flush displayNode mute");
    RSTransactionAdapter::FlushImplicitTransaction(virtualScreenSession->GetRSUIContext());
    TLOGW(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " muteFlag: %{public}d", screenId, muteFlag);
}

void ScreenSessionManager::DisablePowerOffRenderControl(ScreenId screenId)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return;
    }
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64, screenId);
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    if (!ConvertScreenIdToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId");
        return;
    }
    rsInterface_.DisablePowerOffRenderControl(rsScreenId);
}

void ScreenSessionManager::ReportFoldStatusToScb(std::vector<std::string>& screenFoldInfo)
{
    auto clientProxy = GetClientProxy();
    if (clientProxy) {
        auto screenInfo = GetDefaultScreenSession();
        int32_t rotation = -1;
        if (screenInfo != nullptr) {
            rotation = static_cast<int32_t>(screenInfo->GetRotation());
        }
        screenFoldInfo.emplace_back(std::to_string(rotation));

        clientProxy->OnFoldStatusChangedReportUE(screenFoldInfo);
    }
}

std::vector<DisplayPhysicalResolution> ScreenSessionManager::GetAllDisplayPhysicalResolution()
{
    std::lock_guard<std::mutex> lock(allDisplayPhysicalResolutionMutex_);
    if (allDisplayPhysicalResolution_.empty()) {
        sptr<ScreenSession> defaultScreen = GetDefaultScreenSession();
        if (defaultScreen == nullptr) {
            TLOGE(WmsLogTag::DMS, "default screen null");
            return allDisplayPhysicalResolution_;
        }
        ScreenProperty defaultScreenProperty = defaultScreen->GetScreenProperty();
        DisplayPhysicalResolution defaultSize;
        defaultSize.foldDisplayMode_ = FoldDisplayMode::UNKNOWN;
        defaultSize.physicalWidth_ = defaultScreenProperty.GetPhyBounds().rect_.width_;
        defaultSize.physicalHeight_ = defaultScreenProperty.GetPhyBounds().rect_.height_;
        allDisplayPhysicalResolution_.emplace_back(defaultSize);
    }
    for (auto& info : allDisplayPhysicalResolution_) {
        if (info.foldDisplayMode_ == FoldDisplayMode::GLOBAL_FULL) {
            info.foldDisplayMode_ = FoldDisplayMode::FULL;
            break;
        }
    }
    return allDisplayPhysicalResolution_;
}

void ScreenSessionManager::InitSecondaryDisplayPhysicalParams()
{
    std::vector<DisplayPhysicalResolution> resolutions = ScreenSceneConfig::GetAllDisplayPhysicalConfig();
    for (auto &resolution : resolutions) {
        if (FoldDisplayMode::MAIN == resolution.foldDisplayMode_) {
            screenParams_.push_back(resolution.physicalWidth_);
            screenParams_.push_back(resolution.physicalHeight_);
        } else if (FoldDisplayMode::FULL == resolution.foldDisplayMode_) {
            screenParams_.push_back(resolution.physicalWidth_);
            screenParams_.push_back(resolution.physicalHeight_);
        } else if (FoldDisplayMode::GLOBAL_FULL == resolution.foldDisplayMode_) {
            screenParams_.push_back(resolution.physicalWidth_);
            screenParams_.push_back(resolution.physicalHeight_);
        } else {
            TLOGW(WmsLogTag::DMS, "unKnown displayMode");
        }
    }
    if (screenParams_.size() < STATUS_PARAM_VALID_INDEX) {
        TLOGE(WmsLogTag::DMS, "invalid param num");
        return;
    }
    // M mode offsetX
    screenParams_.push_back(screenParams_[GLOBAL_FULL_STATUS_WIDTH] - screenParams_[FULL_STATUS_WIDTH]);
    // M mode offsetY
    screenParams_.push_back(screenParams_[MAIN_STATUS_HEIGHT] - screenParams_[FULL_STATUS_HEIGHT]);
    // G mode offsetY
    screenParams_.push_back(screenParams_[MAIN_STATUS_HEIGHT] - screenParams_[SCREEN_HEIGHT]);
    TLOGI(WmsLogTag::DMS,
        "PhysicalResolution : mainStatusWidth_= %{public}d, fullStatusWidth_= %{public}d, gloablFullStatusWidth_="
        "%{public}d, screenHeight_= %{public}d, m_offsetX= %{public}d, m_offsetY= %{public}d, g_offsetY= %{public}d,",
        screenParams_[MAIN_STATUS_WIDTH], screenParams_[FULL_STATUS_WIDTH],
        screenParams_[GLOBAL_FULL_STATUS_WIDTH], screenParams_[SCREEN_HEIGHT],
        screenParams_[FULL_STATUS_OFFSET_X], screenParams_[FULL_STATUS_OFFSET_Y],
        screenParams_[GLOBAL_FULL_STATUS_OFFSET_Y]);
}

nlohmann::ordered_json ScreenSessionManager::GetCapabilityJson(FoldStatus foldStatus, FoldDisplayMode displayMode,
    std::vector<std::string> rotation, std::vector<std::string> orientation)
{
    nlohmann::ordered_json capabilityInfo;
    capabilityInfo["foldStatus"] = std::to_string(static_cast<int32_t>(foldStatus));
    capabilityInfo["foldDisplayMode"] = std::to_string(static_cast<int32_t>(displayMode));
    capabilityInfo["rotation"] = rotation;
    capabilityInfo["orientation"] = orientation;
    return capabilityInfo;
}

DMError ScreenSessionManager::GetDisplayCapability(std::string& capabilitInfo)
{
    if (g_foldScreenFlag) {
        if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
            return GetSecondaryDisplayCapability(capabilitInfo);
        }
        return GetFoldableDeviceCapability(capabilitInfo);
    }
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        return GetSuperFoldCapability(capabilitInfo);
    }

    std::string deviceTypeTmp = DEVICE_TYPE;
    std::transform(deviceTypeTmp.begin(), deviceTypeTmp.end(), deviceTypeTmp.begin(), ::tolower);
    bool isTvDevice = (deviceTypeTmp == "tv");
    std::vector<std::string> orientation = ORIENTATION_DEFAULT;
    if ((g_isPcDevice && !FoldScreenStateInternel::IsSuperFoldDisplayDevice()) || isTvDevice) {
        orientation = {"1", "0", "3", "2"};
    }
    nlohmann::ordered_json jsonDisplayCapabilityList;
    jsonDisplayCapabilityList["capability"] = nlohmann::json::array();
    nlohmann::ordered_json capabilityInfo = GetCapabilityJson(FoldStatus::UNKNOWN, FoldDisplayMode::UNKNOWN,
        ROTATION_DEFAULT, orientation);
    jsonDisplayCapabilityList["capability"].push_back(std::move(capabilityInfo));

    capabilitInfo = jsonDisplayCapabilityList.dump();
    return DMError::DM_OK;
}

DMError ScreenSessionManager::GetSecondaryDisplayCapability(std::string& capabilitInfo)
{
    nlohmann::ordered_json jsonDisplayCapabilityList;
    jsonDisplayCapabilityList["capability"] = nlohmann::json::array();

    nlohmann::ordered_json fCapabilityInfo = GetCapabilityJson(FoldStatus::FOLDED, FoldDisplayMode::MAIN,
        ROTATION_DEFAULT, ORIENTATION_DEFAULT);
    jsonDisplayCapabilityList["capability"].push_back(std::move(fCapabilityInfo));
    nlohmann::ordered_json nCapability = GetCapabilityJson(FoldStatus::FOLD_STATE_FOLDED_WITH_SECOND_EXPAND,
        FoldDisplayMode::MAIN, ROTATION_DEFAULT, ORIENTATION_DEFAULT);
    jsonDisplayCapabilityList["capability"].push_back(std::move(nCapability));
    nlohmann::ordered_json mCapabilityInfo = GetCapabilityJson(FoldStatus::EXPAND, FoldDisplayMode::FULL,
        ROTATION_DEFAULT, ORIENTATION_DEFAULT);
    jsonDisplayCapabilityList["capability"].push_back(std::move(mCapabilityInfo));
    std::vector<std::string> orientation = {"3", "0", "1", "2"};
    if (CORRECTION_ENABLE) {
        orientation = ORIENTATION_DEFAULT;
    }
    nlohmann::ordered_json gCapability = GetCapabilityJson(FoldStatus::FOLD_STATE_EXPAND_WITH_SECOND_EXPAND,
        FoldDisplayMode::FULL, ROTATION_DEFAULT, orientation);
    jsonDisplayCapabilityList["capability"].push_back(std::move(gCapability));
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_) {
        nlohmann::ordered_json foldCreaseRegion = foldScreenController_->GetFoldCreaseRegionJson();
        jsonDisplayCapabilityList["allCreaseRegion"] = foldCreaseRegion;
    }
#endif

    capabilitInfo = jsonDisplayCapabilityList.dump();
    return DMError::DM_OK;
}

DMError ScreenSessionManager::GetFoldableDeviceCapability(std::string& capabilitInfo)
{
    nlohmann::ordered_json jsonDisplayCapabilityList;
    jsonDisplayCapabilityList["capability"] = nlohmann::json::array();
    FoldStatus expandStatus = FoldStatus::EXPAND;
    FoldStatus foldStatus = FoldStatus::FOLDED;
    FoldDisplayMode expandDisplayMode = FoldDisplayMode::FULL;
    FoldDisplayMode foldDisplayMode = FoldDisplayMode::MAIN;
    if (FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
        expandDisplayMode = FoldDisplayMode::MAIN;
        foldDisplayMode = FoldDisplayMode::SUB;
    }
    nlohmann::ordered_json expandCapabilityInfo = GetCapabilityJson(expandStatus, expandDisplayMode,
        ROTATION_DEFAULT, ORIENTATION_DEFAULT);
    jsonDisplayCapabilityList["capability"].push_back(std::move(expandCapabilityInfo));
    nlohmann::ordered_json foldCapabilityInfo = GetCapabilityJson(foldStatus, foldDisplayMode,
        ROTATION_DEFAULT, ORIENTATION_DEFAULT);
    jsonDisplayCapabilityList["capability"].push_back(std::move(foldCapabilityInfo));
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_) {
        nlohmann::ordered_json foldCreaseRegion = foldScreenController_->GetFoldCreaseRegionJson();
        jsonDisplayCapabilityList["allCreaseRegion"] = foldCreaseRegion;
    }
#endif

    capabilitInfo = jsonDisplayCapabilityList.dump();
    return DMError::DM_OK;
}

DMError ScreenSessionManager::GetSuperFoldCapability(std::string& capabilitInfo)
{
    nlohmann::ordered_json jsonDisplayCapabilityList;
    jsonDisplayCapabilityList["capability"] = nlohmann::json::array();

    nlohmann::ordered_json expandCapabilityInfo = GetCapabilityJson(FoldStatus::EXPAND, FoldDisplayMode::UNKNOWN,
        ROTATION_DEFAULT, ORIENTATION_DEFAULT);
    jsonDisplayCapabilityList["capability"].push_back(std::move(expandCapabilityInfo));
    nlohmann::ordered_json foldCapabilityInfo = GetCapabilityJson(FoldStatus::FOLDED, FoldDisplayMode::UNKNOWN,
        ROTATION_DEFAULT, ORIENTATION_DEFAULT);
    jsonDisplayCapabilityList["capability"].push_back(std::move(foldCapabilityInfo));
    nlohmann::ordered_json halfFoldCapabilityInfo = GetCapabilityJson(FoldStatus::HALF_FOLD, FoldDisplayMode::UNKNOWN,
        ROTATION_DEFAULT, ORIENTATION_DEFAULT);
    jsonDisplayCapabilityList["capability"].push_back(std::move(halfFoldCapabilityInfo));
#ifdef FOLD_ABILITY_ENABLE
    nlohmann::ordered_json foldCreaseRegion = SuperFoldStateManager::GetInstance().GetFoldCreaseRegionJson();
    jsonDisplayCapabilityList["allCreaseRegion"] = foldCreaseRegion;
#endif

    capabilitInfo = jsonDisplayCapabilityList.dump();
    return DMError::DM_OK;
}

bool ScreenSessionManager::SetVirtualScreenStatus(ScreenId screenId, VirtualScreenStatus screenStatus)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return false;
    }
    TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64 " screenStatus: %{public}d",
        screenId, static_cast<int32_t>(screenStatus));
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    if (!ConvertScreenIdToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId");
        return false;
    }

    return rsInterface_.SetVirtualScreenStatus(rsScreenId, screenStatus);
}

sptr<ScreenSession> ScreenSessionManager::GetOrCreateFakeScreenSession(sptr<ScreenSession> screenSession)
{
    sptr<ScreenSession> fakeScreenSession = screenSession->GetFakeScreenSession();
    if (fakeScreenSession != nullptr) {
        TLOGI(WmsLogTag::DMS, "fake screen session has exist");
        return fakeScreenSession;
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    ScreenSessionConfig config = {
        .screenId = SCREEN_ID_FAKE,
        .defaultScreenId = SCREEN_ID_INVALID,
        .property = screenProperty,
    };
    fakeScreenSession =
        new(std::nothrow) ScreenSession(config, ScreenSessionReason::CREATE_SESSION_WITHOUT_DISPLAY_NODE);
    if (fakeScreenSession == nullptr) {
        return nullptr;
    }
    return fakeScreenSession;
}

void ScreenSessionManager::InitFakeScreenSession(sptr<ScreenSession> screenSession)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screen session is null");
        return;
    }
    sptr<ScreenSession> fakeScreenSession = GetOrCreateFakeScreenSession(screenSession);
    if (fakeScreenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "get or create fake screen session failed");
        return;
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    uint32_t screenWidth = screenProperty.GetBounds().rect_.GetWidth();
    uint32_t screenHeight = screenProperty.GetBounds().rect_.GetHeight();
    uint32_t fakeScreenHeight = screenHeight / HALF_SCREEN_PARAM;
    DMRect creaseRect = screenProperty.GetCreaseRect();
    if (creaseRect.height_ > 0) {
        fakeScreenHeight = screenHeight - (static_cast<uint32_t>(creaseRect.posY_) + creaseRect.height_);
    }
    fakeScreenSession->SetIsFakeSession(true);
    fakeScreenSession->UpdatePropertyByResolution(screenWidth, fakeScreenHeight);
    fakeScreenSession->SetXYPosition(0, DISPLAY_B_HEIGHT);
    fakeScreenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    screenSession->UpdatePropertyByFakeBounds(screenWidth, fakeScreenHeight);
    screenSession->SetFakeScreenSession(fakeScreenSession);
    screenSession->SetIsFakeInUse(true);
}

DMError ScreenSessionManager::SetVirtualScreenSecurityExemption(ScreenId screenId, uint32_t pid,
    std::vector<uint64_t>& windowIdList)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_INVALID_CALLING;
    }
    std::vector<uint64_t> surfaceNodeIds;
    if (!windowIdList.empty()) {
        MockSessionManagerService::GetInstance().GetProcessSurfaceNodeIdByPersistentId(
            pid, windowIdList, surfaceNodeIds);
    }
    auto rsId = screenIdManager_.ConvertToRsScreenId(screenId);
    auto ret = rsInterface_.SetVirtualScreenSecurityExemptionList(rsId, surfaceNodeIds);

    std::ostringstream oss;
    oss << "screenId:" << screenId << ", rsID: " << rsId << ", pid: " << pid
        << ", winListSize:[ ";
    for (auto val : windowIdList) {
        oss << val << " ";
    }
    oss << "]" << ", surfaceListSize:[ ";
    for (auto val : surfaceNodeIds) {
        oss << val << " ";
    }
    oss << "]" << ", ret: " << ret;
    TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    return ret == 0 ? DMError::DM_OK : DMError::DM_ERROR_UNKNOWN;
}

void ScreenSessionManager::SetDefaultScreenId(ScreenId defaultScreenId)
{
    defaultScreenId_ = defaultScreenId;
}

sptr<IScreenSessionManagerClient> ScreenSessionManager::GetClientProxy()
{
    std::lock_guard<std::mutex> lock(clientProxyMutex_);
    return clientProxy_;
}

void ScreenSessionManager::SetClientProxy(const sptr<IScreenSessionManagerClient>& client)
{
    std::lock_guard<std::mutex> lock(clientProxyMutex_);
    clientProxy_ = client;
}

DMError ScreenSessionManager::SetMultiScreenMode(ScreenId mainScreenId, ScreenId secondaryScreenId,
    MultiScreenMode screenMode)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    TLOGW(WmsLogTag::DMS, "mainScreenId:%{public}" PRIu64",secondaryScreenId:%{public}" PRIu64",Mode:%{public}u",
        mainScreenId, secondaryScreenId, screenMode);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (IsPhysicalExtendScreenInUse(mainScreenId, secondaryScreenId) == DMError::DM_OK) {
        return DMError::DM_ERROR_INVALID_MODE_ID;
    }
    CreateExtendVirtualScreen(secondaryScreenId);
    if (mainScreenId == secondaryScreenId && mainScreenId == SCREEN_ID_OUTER_ONLY) {
        TLOGW(WmsLogTag::DMS, "set to outer only mode.");
        SetIsOuterOnlyMode(true);
        MultiScreenModeChange(mainScreenId, mainScreenId, "off");
        return DMError::DM_OK;
    }
    if (GetIsOuterOnlyMode()) {
        SetIsOuterOnlyMode(false);
        TLOGI(WmsLogTag::DMS, "exit outer only mode.");
        ExitOuterOnlyMode(mainScreenId, secondaryScreenId, screenMode);
        return DMError::DM_OK;
    }
    SetMultiScreenModeInner(mainScreenId, secondaryScreenId, screenMode);
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        SetIsExtendModelocked(screenMode == MultiScreenMode::SCREEN_EXTEND);
    }
    if (screenMode == MultiScreenMode::SCREEN_EXTEND) {
        SetExpandAndHorizontalLocked(true);
    }
    auto combination = screenMode == MultiScreenMode::SCREEN_MIRROR ?
        ScreenCombination::SCREEN_MIRROR : ScreenCombination::SCREEN_EXPAND;
    SetScreenCastInfo(secondaryScreenId, mainScreenId, combination);
    UnlockStateMachineInExtendAndVirtualScreen(secondaryScreenId);
    NotifyScreenModeChange();
#endif
    return DMError::DM_OK;
}

void ScreenSessionManager::UnlockStateMachineInExtendAndVirtualScreen(ScreenId screenId) 
{
    TLOGI(WmsLogTag::DMS, "Enter");
    if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        return;
    }
    sptr<ScreenSession> screenSession = GetScreenSessionByRsId(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
        return;
    }
    if (screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR &&
        screenSession->GetIsExtendVirtual()) {
        SetIsExtendModelocked(false);
        SetExpandAndHorizontalLocked(false);
        TLOGI(WmsLogTag::DMS, "unlock");
    }
}

void ScreenSessionManager::SetMultiScreenModeInner(ScreenId mainScreenId, ScreenId secondaryScreenId,
    MultiScreenMode screenMode)
{
    TLOGI(WmsLogTag::DMS, "enter");
    if (screenMode == MultiScreenMode::SCREEN_MIRROR) {
        MultiScreenModeChange(mainScreenId, secondaryScreenId, "mirror");
        SetExtendedScreenFallbackPlan(secondaryScreenId);
        sptr<ScreenSession> screenSession = GetScreenSession(secondaryScreenId);
        if (screenSession && screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
            MultiScreenPositionOptions defaultOptions = { GetDefaultScreenId(), 0, 0 };
            SetRelativePositionForDisconnect(defaultOptions);
            NotifyCaptureStatusChanged(true);
        }
    } else if (screenMode == MultiScreenMode::SCREEN_EXTEND) {
        bool lastScreenMirror = false;
        sptr<ScreenSession> screenSession = GetScreenSession(secondaryScreenId);
        if (screenSession && screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
            lastScreenMirror = true;
        }
        MultiScreenModeChange(mainScreenId, secondaryScreenId, "extend");
        SetExtendedScreenFallbackPlan(secondaryScreenId);
        if (screenSession && screenSession->GetScreenCombination() == ScreenCombination::SCREEN_EXTEND &&
            lastScreenMirror) {
            NotifyCaptureStatusChanged(false);
        }
    } else {
        TLOGE(WmsLogTag::DMS, "operate mode error");
    }
}

void ScreenSessionManager::ExitOuterOnlyMode(ScreenId mainScreenId, ScreenId secondaryScreenId,
    MultiScreenMode screenMode)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    ScreenCombination innerCombination = ScreenCombination::SCREEN_MAIN;
    ScreenCombination externalCombination = ScreenCombination::SCREEN_EXTEND;
    sptr<ScreenSession> internalSession = GetInternalScreenSession();
    if (internalSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "internalSession is nullptr");
        return;
    }
    ScreenCombination combination = ScreenCombination::SCREEN_EXTEND;
    if (screenMode == MultiScreenMode::SCREEN_MIRROR) {
        combination = ScreenCombination::SCREEN_MIRROR;
    }
    if (internalSession->GetRSScreenId() == mainScreenId) {
        innerCombination = ScreenCombination::SCREEN_MAIN;
        externalCombination = combination;
    } else if (internalSession->GetRSScreenId() == secondaryScreenId) {
        innerCombination = combination;
        externalCombination = ScreenCombination::SCREEN_MAIN;
    } else {
        TLOGE(WmsLogTag::DMS, "invalid param, use default mode.");
    }
    MultiScreenPowerChangeManager::GetInstance().SetInnerAndExternalCombination(innerCombination,
        externalCombination);
    MultiScreenModeChange(mainScreenId, mainScreenId, "on");
#endif
}

DMError ScreenSessionManager::IsPhysicalExtendScreenInUse(ScreenId mainScreenId, ScreenId secondaryScreenId)
{
    TLOGI(WmsLogTag::DMS, "Enter");
    sptr<ScreenSession> mainScreenSession = GetScreenSessionByRsId(mainScreenId);
    sptr<ScreenSession> secondaryScreenSession = GetScreenSessionByRsId(secondaryScreenId);
    if (mainScreenSession == nullptr || secondaryScreenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "ScreenSession is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    if (mainScreenSession->GetScreenProperty().GetScreenType() == ScreenType::VIRTUAL ||
        secondaryScreenSession->GetScreenProperty().GetScreenType() == ScreenType::VIRTUAL) {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (auto sessionIt : screenSessionMap_) {
            auto screenSession = sessionIt.second;
            if (screenSession == nullptr) {
                TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
                continue;
            }
            if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::REAL &&
                (screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR ||
                 screenSession->GetScreenCombination() == ScreenCombination::SCREEN_EXTEND)) {
                TLOGI(WmsLogTag::DMS, "physical extend screen in use screenId: %{public}" PRIu64,
                    screenSession->GetScreenId());
                return DMError::DM_OK;
            }
        }
    }
    return DMError::DM_ERROR_UNKNOWN;
}

void ScreenSessionManager::CreateExtendVirtualScreen(ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "Enter");
    sptr<ScreenSession> screenSession = GetScreenSessionByRsId(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
        return;
    }
    if (screenSession->GetScreenProperty().GetScreenType() != ScreenType::VIRTUAL) {
        return;
    }
    if (screenSession->GetDisplayNode() == nullptr) {
        Rosen::RSDisplayNodeConfig rsConfig;
        ScreenId rsScreenId = screenSession->GetRSScreenId();
        rsConfig.screenId = rsScreenId;
        screenSession->CreateDisplayNode(rsConfig);
        screenSession->SetDisplayNodeScreenId(rsScreenId);
        screenSession->SetDisplayNodeSecurity();
        screenSession->SetIsCurrentInUse(true);
        screenSession->SetIsExtend(true);
        screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
        screenSession->SetIsExtendVirtual(true);
        screenSession->SetDisplayGroupId(DISPLAY_GROUP_ID_DEFAULT);
        screenSession->SetMainDisplayIdOfGroup(DISPLAY_GROUP_ID_DEFAULT);
        TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64", rsScreenId:%{public}" PRIu64"",
            screenId, rsScreenId);
    }
    NotifyDisplayCreate(screenSession->ConvertToDisplayInfo());
}

DMError ScreenSessionManager::SetMultiScreenRelativePosition(MultiScreenPositionOptions mainScreenOptions,
    MultiScreenPositionOptions secondScreenOption)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    TLOGI(WmsLogTag::DMS,
        "mID:%{public}" PRIu64", X:%{public}u, Y:%{public}u,sID:%{public}" PRIu64", X:%{public}u, Y:%{public}u",
        mainScreenOptions.screenId_, mainScreenOptions.startX_, mainScreenOptions.startY_,
        secondScreenOption.screenId_, secondScreenOption.startX_, secondScreenOption.startY_);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    sptr<ScreenSession> firstScreenSession = nullptr;
    sptr<ScreenSession> secondScreenSession = nullptr;
    if (g_isPcDevice) {
        firstScreenSession = GetScreenSessionByRsId(mainScreenOptions.screenId_);
        secondScreenSession = GetScreenSessionByRsId(secondScreenOption.screenId_);
    } else {
        firstScreenSession = GetScreenSession(mainScreenOptions.screenId_);
        secondScreenSession = GetScreenSession(secondScreenOption.screenId_);
    }
    if (!firstScreenSession || !secondScreenSession) {
        TLOGE(WmsLogTag::DMS, "ScreenSession is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    if ( !FoldScreenStateInternel::IsSuperFoldDisplayDevice() &&
        !MultiScreenManager::GetInstance().AreScreensTouching(firstScreenSession, secondScreenSession,
        mainScreenOptions, secondScreenOption)) {
        const std::string errMsg = "Options incorrect";
        ReportRelativePositionChangeEvent(mainScreenOptions, secondScreenOption, errMsg);     
        TLOGE(WmsLogTag::DMS, "Options incorrect!");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    SetMultiScreenRelativePositionInner(firstScreenSession, secondScreenSession, mainScreenOptions,
        secondScreenOption);
#endif
    return DMError::DM_OK;
}

void ScreenSessionManager::SetMultiScreenRelativePositionInner(sptr<ScreenSession>& firstScreenSession,
    sptr<ScreenSession>& secondScreenSession, MultiScreenPositionOptions mainScreenOptions,
    MultiScreenPositionOptions secondScreenOption)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    firstScreenSession->SetStartPosition(mainScreenOptions.startX_, mainScreenOptions.startY_);
    secondScreenSession->SetStartPosition(secondScreenOption.startX_, secondScreenOption.startY_);
    CalculateXYPosition(firstScreenSession, secondScreenSession);
    firstScreenSession->PropertyChange(firstScreenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE);
    secondScreenSession->PropertyChange(secondScreenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE);
    if (g_isPcDevice) {
        sptr<ScreenSession> firstPhysicalScreen = GetPhysicalScreenSession(firstScreenSession->GetRSScreenId());
        sptr<ScreenSession> secondPhysicalScreen = GetPhysicalScreenSession(secondScreenSession->GetRSScreenId());
        if (firstPhysicalScreen && secondPhysicalScreen) {
            firstPhysicalScreen->SetStartPosition(mainScreenOptions.startX_, mainScreenOptions.startY_);
            secondPhysicalScreen->SetStartPosition(secondScreenOption.startX_, secondScreenOption.startY_);
            const std::string errMsg = "success";
            ReportRelativePositionChangeEvent(mainScreenOptions, secondScreenOption, errMsg);     
        }
    }
    std::shared_ptr<RSDisplayNode> firstDisplayNode = firstScreenSession->GetDisplayNode();
    std::shared_ptr<RSDisplayNode> secondDisplayNode = secondScreenSession->GetDisplayNode();
    if (firstDisplayNode && secondDisplayNode) {
        SetScreenOffset(firstScreenSession->GetScreenId(), mainScreenOptions.startX_, mainScreenOptions.startY_);
        SetScreenOffset(secondScreenSession->GetScreenId(), secondScreenOption.startX_, secondScreenOption.startY_);
    } else {
        TLOGW(WmsLogTag::DMS, "DisplayNode is null");
    }
    TLOGI(WmsLogTag::DMS, "free displayNode");
    RSTransactionAdapter::FlushImplicitTransaction(
        {firstScreenSession->GetRSUIContext(), secondScreenSession->GetRSUIContext()});
#endif
}

void ScreenSessionManager::SetRelativePositionForDisconnect(MultiScreenPositionOptions defaultScreenOptions)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    TLOGI(WmsLogTag::DMS, "mID:%{public}" PRIu64", X:%{public}u, Y:%{public}u",
        defaultScreenOptions.screenId_, defaultScreenOptions.startX_, defaultScreenOptions.startY_);
    sptr<ScreenSession> defaultScreenSession = GetScreenSession(defaultScreenOptions.screenId_);
    if (!defaultScreenSession) {
        TLOGE(WmsLogTag::DMS, "ScreenSession is null");
        return;
    }
    defaultScreenSession->SetStartPosition(defaultScreenOptions.startX_, defaultScreenOptions.startY_);
    SetPhysicalStartPosition(defaultScreenSession, defaultScreenOptions.startX_, defaultScreenOptions.startY_);
    const std::string errMsg = "default position";
    ReportRelativePositionChangeEvent(defaultScreenOptions, defaultScreenOptions, errMsg);
    CalculateXYPosition(defaultScreenSession);
    defaultScreenSession->PropertyChange(defaultScreenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE);
    std::shared_ptr<RSDisplayNode> defaultDisplayNode = defaultScreenSession->GetDisplayNode();
    if (defaultDisplayNode) {
        SetScreenOffset(defaultScreenSession->GetScreenId(),
            defaultScreenOptions.startX_, defaultScreenOptions.startY_);
    } else {
        TLOGW(WmsLogTag::DMS, "DisplayNode is null");
    }
    TLOGI(WmsLogTag::DMS, "free displayNode");
    RSTransactionAdapter::FlushImplicitTransaction(defaultScreenSession->GetRSUIContext());
#endif
}

void ScreenSessionManager::MultiScreenModeChange(ScreenId mainScreenId, ScreenId secondaryScreenId,
    const std::string& operateMode)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    std::lock_guard<std::mutex> lock(screenModeChangeMutex_);
    TLOGW(WmsLogTag::DMS, "mainId=%{public}" PRIu64" secondId=%{public}" PRIu64" operateType: %{public}s",
        mainScreenId, secondaryScreenId, operateMode.c_str());
    OnScreenModeChange(ScreenModeChangeEvent::BEGIN);
    if (g_isPcDevice) {
        std::unique_lock<std::mutex> lock(screenMaskMutex_);
        if (DmUtils::safe_wait_for(screenMaskCV_, lock,
            std::chrono::milliseconds(CV_WAIT_SCREEN_MASK_MS)) == std::cv_status::timeout) {
            TLOGI(WmsLogTag::DMS, "wait screenMaskMutex_ timeout");
        }
    }
    sptr<ScreenSession> firstSession = nullptr;
    sptr<ScreenSession> secondarySession = nullptr;
    OperateModeChange(mainScreenId, secondaryScreenId, firstSession, secondarySession, operateMode);
    if (firstSession != nullptr && secondarySession != nullptr) {
        ScreenCombination firstCombination = firstSession->GetScreenCombination();
        ScreenCombination secondaryCombination = secondarySession->GetScreenCombination();
        MultiScreenManager::GetInstance().MultiScreenModeChange(firstSession, secondarySession, operateMode);
        if ((firstCombination == ScreenCombination::SCREEN_MIRROR ||
            secondaryCombination == ScreenCombination::SCREEN_MIRROR) &&
            operateMode == SCREEN_EXTEND) {
            MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_MIRROR, MULTI_SCREEN_EXIT_STR);
            MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_EXTEND, MULTI_SCREEN_ENTER_STR);
            NotifyDisplayChanged(secondarySession->ConvertToDisplayInfo(),
                DisplayChangeEvent::SOURCE_MODE_CHANGED);
        } else if ((firstCombination == ScreenCombination::SCREEN_EXTEND ||
            secondaryCombination == ScreenCombination::SCREEN_EXTEND) &&
            operateMode == SCREEN_MIRROR) {
            MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_EXTEND, MULTI_SCREEN_EXIT_STR);
            MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_MIRROR, MULTI_SCREEN_ENTER_STR);
            NotifyDisplayChanged(secondarySession->ConvertToDisplayInfo(),
                DisplayChangeEvent::SOURCE_MODE_CHANGED);
        }
    } else {
        TLOGE(WmsLogTag::DMS, "params error");
    }
    NotifyScreenModeChange();
    OnScreenModeChange(ScreenModeChangeEvent::END);
#endif
}

void ScreenSessionManager::OperateModeChange(ScreenId mainScreenId, ScreenId secondaryScreenId,
    sptr<ScreenSession>& firstSession, sptr<ScreenSession>& secondarySession, const std::string& operateMode)
{
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (auto sessionIt : screenSessionMap_) {
            auto screenSession = sessionIt.second;
            if (screenSession == nullptr) {
                TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
                continue;
            }
            if (!screenSession->GetIsCurrentInUse()) {
                TLOGE(WmsLogTag::DMS, "current screen: %{public}" PRIu64" is not in user!", sessionIt.first);
                continue;
            }
            if (screenSession->GetRSScreenId() == mainScreenId) {
                firstSession = screenSession;
            }
            if (screenSession->GetRSScreenId() == secondaryScreenId) {
                secondarySession = screenSession;
            }
        }
    }
    if (operateMode == "off" || operateMode == "on") {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (auto sessionIt : screenSessionMap_) {
            auto screenSession = sessionIt.second;
            if (screenSession == nullptr) {
                TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
                continue;
            }
            if (!screenSession->GetIsCurrentInUse()) {
                TLOGW(WmsLogTag::DMS, "current screen: %{public}" PRIu64" is not in user!", sessionIt.first);
                continue;
            }
            if (screenSession->GetRSScreenId() == mainScreenId) {
                firstSession = screenSession;
            }
            if (screenSession->GetRSScreenId() == secondaryScreenId) {
                secondarySession = screenSession;
            }
        }
    }
}

bool ScreenSessionManager::HasExtendVirtualScreen()
{
    TLOGI(WmsLogTag::DMS, "start");
    bool hasExtendVirtualScreen = false ;
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMap;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        screenSessionMap = screenSessionMap_;
    }
    for (const auto& sessionIt : screenSessionMap) {
        sptr<ScreenSession> screenSession = sessionIt.second ;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr, ScreenId: %{public}" PRIu64,
                sessionIt.first);
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::VIRTUAL &&
            screenSession->GetIsExtendVirtual()) {
                hasExtendVirtualScreen = true;
                TLOGI(WmsLogTag::DMS, "is true, screenId: %{public}" PRIu64, screenSession->GetScreenId());
                break;
            }
    }
    return hasExtendVirtualScreen;
}

void ScreenSessionManager::SwitchScrollParam(FoldDisplayMode displayMode)
{
    auto task = [=]() {
        std::map<FoldDisplayMode, ScrollableParam> scrollableParams = ScreenSceneConfig::GetAllScrollableParam();
        std::string scrollVelocityScale = scrollableParams.count(displayMode) != 0 ?
            scrollableParams[displayMode].velocityScale_ : "0";
        std::string scrollFriction = scrollableParams.count(displayMode) != 0 ?
            scrollableParams[displayMode].friction_ : "0";
        system::SetParameter("persist.scrollable.velocityScale", scrollVelocityScale);
        system::SetParameter("persist.scrollable.friction", scrollFriction);
    };
    taskScheduler_->PostAsyncTask(task, "SwitchScrollParam");
}

void ScreenSessionManager::MultiScreenModeChange(const std::string& firstScreenIdStr,
    const std::string& secondaryScreenIdStr, const std::string& secondaryChandeMode)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    if (!ScreenSettingHelper::IsNumber(firstScreenIdStr) || !ScreenSettingHelper::IsNumber(secondaryScreenIdStr)) {
        TLOGE(WmsLogTag::DMS, "param denied!");
        return;
    }
    ScreenId firstScreenId = DISPLAY_ID_INVALID;
    ScreenId secondaryScreenId = DISPLAY_ID_INVALID;
    if (!ScreenSettingHelper::ConvertStrToUint64(firstScreenIdStr, firstScreenId) ||
        !ScreenSettingHelper::ConvertStrToUint64(secondaryScreenIdStr, secondaryScreenId)) {
        TLOGE(WmsLogTag::DMS, "dumper screenId params error!");
        return;
    }
    if (secondaryChandeMode == "mirror" || secondaryChandeMode == "extend") {
        MultiScreenModeChange(firstScreenId, secondaryScreenId, secondaryChandeMode);
    } else {
        TLOGE(WmsLogTag::DMS, "dumper params error");
    }
#endif
}

void ScreenSessionManager::OnScreenExtendChange(ScreenId mainScreenId, ScreenId extendScreenId)
{
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy->OnScreenExtendChanged(mainScreenId, extendScreenId);
}

void ScreenSessionManager::OnTentModeChanged(int tentType, int32_t hall)
{
#ifdef FOLD_ABILITY_ENABLE
    if (!foldScreenController_) {
        TLOGI(WmsLogTag::DMS, "foldScreenController_ is null");
        return;
    }
    foldScreenController_->OnTentModeChanged(tentType, hall);
#endif
}

void ScreenSessionManager::SetCoordinationFlag(bool isCoordinationFlag)
{
    TLOGI(WmsLogTag::DMS, "set coordination flag %{public}d", isCoordinationFlag);
    isCoordinationFlag_ = isCoordinationFlag;
}

bool ScreenSessionManager::GetCoordinationFlag(void)
{
    return isCoordinationFlag_;
}

void ScreenSessionManager::NotifyRSCoordination(bool isEnterCoordination) const
{
    TLOGI(WmsLogTag::DMS, "isEnterCoordination:%{public}d", isEnterCoordination);
    auto ret = rsInterface_.SetDualScreenState(SCREEN_ID_FULL, isEnterCoordination ?
        DualScreenStatus::DUAL_SCREEN_ENTER : DualScreenStatus::DUAL_SCREEN_EXIT);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "rsInterface failed! ret:%{public}d", ret);
    }
}

DMError ScreenSessionManager::SetVirtualScreenMaxRefreshRate(ScreenId id, uint32_t refreshRate,
    uint32_t& actualRefreshRate)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied! clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGI(WmsLogTag::DMS, "ID:%{public}" PRIu64", refreshRate:%{public}u, actualRefreshRate:%{public}u",
        id, refreshRate, actualRefreshRate);
    if (id == GetDefaultScreenId()) {
        TLOGE(WmsLogTag::DMS, "cannot set refresh main screen id: %{public}" PRIu64".", GetDefaultScreenId());
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto screenSession = GetScreenSession(id);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null.");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    ScreenId rsScreenId;
    if (!screenIdManager_.ConvertToRsScreenId(id, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId.");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    int32_t res = rsInterface_.SetVirtualScreenRefreshRate(rsScreenId, refreshRate, actualRefreshRate);
    TLOGI(WmsLogTag::DMS, "refreshRate:%{public}u, actualRefreshRate:%{public}u", refreshRate, actualRefreshRate);
    if (res != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS, "rsInterface error: %{public}d", res);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    screenSession->UpdateRefreshRate(actualRefreshRate);
    screenSession->SetSupportedRefreshRate({actualRefreshRate});
    NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(), DisplayChangeEvent::UPDATE_REFRESHRATE);
    return DMError::DM_OK;
}

void ScreenSessionManager::OnScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName)
{
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy->ScreenCaptureNotify(mainScreenId, uid, clientName);
}

void ScreenSessionManager::AddPermissionUsedRecord(const std::string& permission, int32_t successCount,
    int32_t failCount)
{
    int32_t ret = Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(IPCSkeleton::GetCallingTokenID(),
        permission, successCount, failCount);
    if (ret != 0) {
        TLOGW(WmsLogTag::DMS, "permission:%{public}s, successCount %{public}d, failedCount %{public}d",
            permission.c_str(), successCount, failCount);
    }
}

std::shared_ptr<Media::PixelMap> ScreenSessionManager::GetScreenCapture(const CaptureOption& captureOption,
    DmErrorCode* errorCode)
{
    TLOGI(WmsLogTag::DMS, "enter!");
    if (!checkCaptureParam(captureOption, errorCode)) {
        return nullptr;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetScreenCapture(%" PRIu64")", captureOption.displayId_);
    std::vector<uint64_t> surfaceNodesList;
    ConvertWindowIdsToSurfaceNodeList(captureOption.blackWindowIdList_, surfaceNodesList, errorCode);
    if (*errorCode != DmErrorCode::DM_OK) {
        return nullptr;
    }

    auto res = GetScreenSnapshot(captureOption.displayId_, false, false, surfaceNodesList);
    AddPermissionUsedRecord(CUSTOM_SCREEN_CAPTURE_PERMISSION,
        static_cast<int32_t>(res != nullptr), static_cast<int32_t>(res == nullptr));
    if (res == nullptr) {
        TLOGE(WmsLogTag::DMS, "get capture null.");
        *errorCode = DmErrorCode::DM_ERROR_SYSTEM_INNORMAL;
        return nullptr;
    }
    NotifyScreenshot(captureOption.displayId_);
    if (SessionPermission::IsBetaVersion()) {
        CheckAndSendHiSysEvent("GET_DISPLAY_SNAPSHOT", "hmos.screenshot");
    }
    *errorCode = DmErrorCode::DM_OK;
    isScreenShot_ = true;
    /* notify scb to do toast */
    OnScreenCaptureNotify(GetDefaultScreenId(), IPCSkeleton::GetCallingUid(), SysCapUtil::GetClientName());
    /* notify application capture happend */
    NotifyCaptureStatusChanged();
    return res;
}

bool ScreenSessionManager::checkCaptureParam(const CaptureOption& captureOption, DmErrorCode* errorCode)
{
    if (errorCode == nullptr) {
        TLOGE(WmsLogTag::DMS, "param is null.");
        return false;
    }
    if (system::GetBoolParameter("persist.edm.disallow_screenshot", false)) {
        TLOGW(WmsLogTag::DMS, "capture disabled by edm!");
        *errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
        return false;
    }
    if (!isSupportCapture_) {
        TLOGW(WmsLogTag::DMS, "device not support capture.");
        *errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
        return false;
    }
    if (!Permission::CheckCallingPermission(CUSTOM_SCREEN_CAPTURE_PERMISSION) &&
        !Permission::CheckCallingPermission(CUSTOM_SCREEN_RECORDING_PERMISSION) && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! clientName: %{public}s, pid: %{public}d.",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingRealPid());
        *errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
        return false;
    }
    if (captureOption.displayId_ == DISPLAY_ID_INVALID ||
        (captureOption.displayId_ == DISPLAY_ID_FAKE && !IsFakeDisplayExist())) {
        TLOGE(WmsLogTag::DMS, "display id invalid.");
        *errorCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        return false;
    }
    return true;
}

void ScreenSessionManager::ConvertWindowIdsToSurfaceNodeList(std::vector<uint64_t> windowIdList, 
    std::vector<uint64_t>& surfaceNodesList, DmErrorCode* errorCode)
{
    if (windowIdList.empty()) {
        return;
    }
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGE(WmsLogTag::DMS, "clientProxy_ is nullptr");
        return;
    }
    const std::vector<uint32_t> needWindowTypeList = { FLOATING_BALL_TYPE };
    clientProxy->OnGetSurfaceNodeIdsFromMissionIdsChanged(windowIdList, surfaceNodesList, needWindowTypeList, true);
    if (surfaceNodesList.empty()) {
        *errorCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        TLOGE(WmsLogTag::DMS, "has invalid windowId. cannot trans surfaceNodeId.");
        return;
    }
    std::ostringstream oss;
    for (size_t i = 0; i < surfaceNodesList.size(); ++i) {
        oss << surfaceNodesList[i] << ", ";
    }
    TLOGD(WmsLogTag::DMS, "capture option surfaceNodesList=%{public}s", oss.str().c_str());
}

bool ScreenSessionManager::IsSupportCapture()
{
    std::string tmp = DEVICE_TYPE;
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
    return std::find(CAPTURE_DEVICE.begin(), CAPTURE_DEVICE.end(), tmp) != CAPTURE_DEVICE.end();
}

sptr<DisplayInfo> ScreenSessionManager::GetPrimaryDisplayInfo()
{
    sptr<ScreenSession> screenSession = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (auto sessionIt : screenSessionMap_) {
            sptr<ScreenSession> session = sessionIt.second;
            if (session == nullptr) {
                TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
                continue;
            }
            if (!session->GetIsExtend()) {
                TLOGD(WmsLogTag::DMS, "find primary %{public}" PRIu64, session->screenId_);
                screenSession = session;
                break;
            }
        }
    }
    if (screenSession == nullptr) {
        TLOGW(WmsLogTag::DMS, "get extend screen faild use default!");
        screenSession = GetScreenSession(GetDefaultScreenId());
    }
    if (screenSession) {
        std::lock_guard<std::recursive_mutex> lock_info(displayInfoMutex_);
        sptr<DisplayInfo> displayInfo = screenSession->ConvertToDisplayInfo();
        if (displayInfo == nullptr) {
            TLOGI(WmsLogTag::DMS, "convert display error.");
            return nullptr;
        }
        displayInfo = HookDisplayInfoByUid(displayInfo, screenSession);
        return displayInfo;
    } else {
        TLOGE(WmsLogTag::DMS, "failed");
        return nullptr;
    }
}

DisplayId ScreenSessionManager::GetPrimaryDisplayId()
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(),IPCSkeleton::GetCallingPid());
        return SCREEN_ID_INVALID;
    }
    sptr<ScreenSession> screenSession = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (auto sessionIt : screenSessionMap_) {
            sptr<ScreenSession> session = sessionIt.second;
            if (session == nullptr) {
                TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
                continue;
            }
            if (!session->GetIsExtend()) {
                TLOGD(WmsLogTag::DMS, "find primary %{public}" PRIu64, session->screenId_);
                screenSession = session;
                break;
            }
        }
    }
    if (screenSession && screenSession->IsScreenAvailable()) {
        return screenSession->GetDisplayId();
    } else {
        return SCREEN_ID_INVALID;
    }
}

void ScreenSessionManager::OnSuperFoldStatusChange(ScreenId screenId, SuperFoldStatus superFoldStatus)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 ", superFoldStatus: %{public}d", screenId,
        static_cast<uint32_t>(superFoldStatus));
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGE(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy->OnSuperFoldStatusChanged(screenId, superFoldStatus);
}

void ScreenSessionManager::OnExtendScreenConnectStatusChange(ScreenId screenId,
    ExtendScreenConnectStatus extendScreenConnectStatus)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 ", extendScreenConnectStatus: %{public}d", screenId,
        static_cast<uint32_t>(extendScreenConnectStatus));
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGE(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy->OnExtendScreenConnectStatusChanged(screenId, extendScreenConnectStatus);
}

void ScreenSessionManager::OnSecondaryReflexionChange(ScreenId screenId, bool isSecondaryReflexion)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 ", isSecondaryReflexion: %{public}d", screenId,
        isSecondaryReflexion);
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGE(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy->OnSecondaryReflexionChanged(screenId, isSecondaryReflexion);
}

void ScreenSessionManager::OnBeforeScreenPropertyChange(FoldStatus foldStatus)
{
    TLOGI(WmsLogTag::DMS, "foldstatus: %{public}d", foldStatus);
    if (!FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        return;
    }
    if (!clientProxy_) {
        TLOGE(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy_->OnBeforeScreenPropertyChanged(foldStatus);
}

int32_t ScreenSessionManager::GetCameraStatus()
{
    return cameraStatus_;
}

int32_t ScreenSessionManager::GetCameraPosition()
{
    return cameraPosition_;
}

DMError ScreenSessionManager::CanEnterCoordination()
{
    if (hdmiScreenCount_ > 0) {
        TLOGW(WmsLogTag::DMS, "hdmiScreenCount_: %{public}d cannot enter coordination", hdmiScreenCount_);
        return DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_WIRED_CASTING;
    }

    if (virtualScreenCount_ == 0) {
        return DMError::DM_OK;
    }
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (const auto& pair : screenSessionMap_) {
        sptr<ScreenSession> session = pair.second;
        if (!session) {
            TLOGW(WmsLogTag::DMS, "screenId=%{public}" PRIu64", session is null", pair.first);
            continue;
        }
        if (session->GetIsRealScreen()) {
            continue;
        }
        if (session->GetVirtualScreenType() != VirtualScreenType::SCREEN_RECORDING) {
            TLOGW(WmsLogTag::DMS, "wireless casting: %{public}u cannot enter coordination", session->GetVirtualScreenType());
            return DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_WIRLESS_CASTING;
        } else {
            TLOGW(WmsLogTag::DMS, "screen recording cannot enter coordination");
            return DMError::DM_ERROR_NOT_SUPPORT_COOR_WHEN_RECORDING;
        }
    }
    return DMError::DM_OK;
}

SessionOption ScreenSessionManager::GetSessionOption(sptr<ScreenSession> screenSession)
{
    SessionOption option = {
        .rsId_ = screenSession->GetRSScreenId(),
        .name_ = screenSession->GetName(),
        .isExtend_ = screenSession->GetIsExtend(),
        .innerName_ = screenSession->GetInnerName(),
        .screenId_ = screenSession->GetScreenId(),
        .rotationCorrectionMap_ = screenSession->GetRotationCorrectionMap(),
        .supportsFocus_ = screenSession->GetSupportsFocus()
    };
    return option;
}

SessionOption ScreenSessionManager::GetSessionOption(sptr<ScreenSession> screenSession, ScreenId screenId,
    const UniqueScreenRotationOptions& rotationOptions)
{
    SessionOption option = {
        .rsId_ = screenSession->GetRSScreenId(),
        .name_ = screenSession->GetName(),
        .isExtend_ = screenSession->GetIsExtend(),
        .innerName_ = screenSession->GetInnerName(),
        .screenId_ = screenId,
        .rotationCorrectionMap_ = screenSession->GetRotationCorrectionMap(),
        .supportsFocus_ = screenSession->GetSupportsFocus(),
        .isRotationLocked_ = rotationOptions.isRotationLocked_,
        .rotation_ = rotationOptions.rotation_,
        .rotationOrientationMap_ = rotationOptions.rotationOrientationMap_
    };
    return option;
}

SessionOption ScreenSessionManager::GetSessionOption(sptr<ScreenSession> screenSession, ScreenId screenId)
{
    return GetSessionOption(screenSession, screenId, UniqueScreenRotationOptions());
}

DMError ScreenSessionManager::SetScreenSkipProtectedWindow(const std::vector<ScreenId>& screenIds, bool isEnable)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    std::ostringstream oss;
    for (ScreenId screenId : screenIds) {
        oss << screenId << " ";
    }
    TLOGI(WmsLogTag::DMS, "screenIds:%{public}s, isEnable:%{public}d", oss.str().c_str(), isEnable);
    {
        std::lock_guard<std::mutex> lock(shareProtectMutex_);
        for (ScreenId screenId : screenIds) {
            sptr<ScreenSession> screenSession = GetScreenSession(screenId);
            if (screenSession == nullptr) {
                continue;
            }
            if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::VIRTUAL) {
                ScreenId rsScreenId = INVALID_SCREEN_ID;
                if (!screenIdManager_.ConvertToRsScreenId(screenSession->GetScreenId(), rsScreenId) ||
                    rsScreenId == INVALID_SCREEN_ID) {
                    TLOGE(WmsLogTag::DMS, "No corresponding rsId:%{public}" PRIu64 "", rsScreenId);
                    continue;
                }
                TLOGI(WmsLogTag::DMS, "virtualScreenId:%{public}" PRIu64 "", screenId);
                HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
                    "SetCastScreenEnableSkipWindow(%" PRIu64")", screenId);
                rsInterface_.SetCastScreenEnableSkipWindow(rsScreenId, isEnable);
            }
        }
    }
    return DMError::DM_OK;
}

bool ScreenSessionManager::IsSpecialApp()
{
    if (!FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        return false;
    }
    static std::chrono::steady_clock::time_point lastRequestTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    auto interval = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastRequestTime).count();
    std::string bundleName = NO_EXIST_BUNDLE_MANE;
    int32_t currentPid = IPCSkeleton::GetCallingPid();
    if (interval < MAX_INTERVAL_US) {
        bundleName = g_uidVersionMap.Get(currentPid);
    }
    if (bundleName == NO_EXIST_BUNDLE_MANE) {
        bundleName = SysCapUtil::GetBundleName();
        TLOGI(WmsLogTag::DMS, "Get BundleName from IPC pid: %{public}d name: %{public}s",
            currentPid, bundleName.c_str());
        g_uidVersionMap.Set(currentPid, bundleName);
    }
    lastRequestTime = currentTime;
    TLOGD(WmsLogTag::DMS, "bundleName: %{public}s", bundleName.c_str());
    auto it = g_packageNames_.find(bundleName);
    if (it != g_packageNames_.end()) {
        TLOGI(WmsLogTag::DMS, "Is Special App");
        return true;
    }
    return false;
}

void ScreenSessionManager::UpdateValidArea(ScreenId screenId, uint32_t validWidth, uint32_t validHeight)
{
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    screenSession->SetValidWidth(validWidth);
    screenSession->SetValidHeight(validHeight);
}

bool ScreenSessionManager::GetIsRealScreen(ScreenId screenId)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied.calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return false;
    }
    return screenSession->GetIsRealScreen();
}

DMError ScreenSessionManager::SetSystemKeyboardStatus(bool isTpKeyboardOn)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
#ifdef FOLD_ABILITY_ENABLE
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        SuperFoldStateManager::GetInstance().SetSystemKeyboardStatus(isTpKeyboardOn);
        return DMError::DM_OK;
    }
#endif // FOLD_ABILITY_ENABLE
    return DMError::DM_ERROR_DEVICE_NOT_SUPPORT;
}

void ScreenSessionManager::WakeUpPictureFrameBlock(DisplayEvent event)
{
    std::unique_lock <std::mutex> lock(screenWaitPictureFrameMutex_);
    if (event == DisplayEvent::SCREEN_LOCK_START_DREAM) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]get pictureFrameReady");
        pictureFrameReady_ = true;
    } else if (event == DisplayEvent::SCREEN_LOCK_END_DREAM) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]get pictureFrameBreak");
        pictureFrameBreak_ = true;
    } else {
        return;
    }
    TLOGI(WmsLogTag::DMS, "[UL_POWER]notify block");
    screenWaitPictureFrameCV_.notify_all();
}

bool ScreenSessionManager::BlockScreenWaitPictureFrameByCV(bool isStartDream)
{
    TLOGI(WmsLogTag::DMS, "[UL_POWER]enter");
    std::unique_lock <std::mutex> lock(screenWaitPictureFrameMutex_);
    pictureFrameReady_ = false;
    pictureFrameBreak_ = false;
    if (DmUtils::safe_wait_for(screenWaitPictureFrameCV_, lock, std::chrono::milliseconds(SCREEN_WAIT_PICTURE_FRAME_TIME))
        == std::cv_status::timeout) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]wait picture frame timeout");
        return true;
    }
    bool pictureFrameIsOk = isStartDream ? pictureFrameReady_ : pictureFrameBreak_;
    TLOGI(WmsLogTag::DMS, "[UL_POWER]pictureFrameIsOk:%{public}d", pictureFrameIsOk);
    return pictureFrameIsOk;
}

void ScreenSessionManager::SetForceCloseHdr(ScreenId screenId, bool isForceCloseHdr)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return;
    }
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    TLOGI(WmsLogTag::DMS, "[UL_POWER]screenId:%{public}" PRIu64 "isForceCloseHdr:%{public}d",
        screenId, isForceCloseHdr);
    screenSession->SetForceCloseHdr(isForceCloseHdr);
}

void ScreenSessionManager::RegisterSettingExtendScreenDpiObserver()
{
    TLOGI(WmsLogTag::DMS, "Register Setting Extend Dpi Observer");
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) { SetExtendScreenDpi(); };
    ScreenSettingHelper::RegisterSettingExtendScreenDpiObserver(DmUtils::wrap_callback(updateFunc));
}

void ScreenSessionManager::SetExtendScreenDpi()
{
    float extendScreenDpiCoef = EXTEND_SCREEN_DPI_DEFAULT_PARAMETER;
    bool ret = ScreenSettingHelper::GetSettingExtendScreenDpi(extendScreenDpiCoef);
    if (!ret) {
        TLOGE(WmsLogTag::DMS, "get setting extend screen dpi failed");
        g_extendScreenDpiCoef = EXTEND_SCREEN_DPI_DEFAULT_PARAMETER;
    } else {
        g_extendScreenDpiCoef = extendScreenDpiCoef;
    }
    float dpi = static_cast<float>(cachedSettingDpi_) / BASELINE_DENSITY;
    SetExtendPixelRatio(dpi * g_extendScreenDpiCoef);
    TLOGI(WmsLogTag::DMS, "get setting extend screen dpi is : %{public}f", g_extendScreenDpiCoef);
}

sptr<ScreenSession> ScreenSessionManager::GetFakePhysicalScreenSession(ScreenId screenId, ScreenId defScreenId,
    ScreenProperty property)
{
    sptr<ScreenSession> screenSession = nullptr;
    ScreenSessionConfig config;
    if (g_isPcDevice) {
        config = {
            .screenId = screenId,
            .rsId = screenId,
            .defaultScreenId = defScreenId,
            .property = property,
        };
        screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_WITHOUT_DISPLAY_NODE);
    }
    return screenSession;
}

sptr<ScreenSession> ScreenSessionManager::CreateFakePhysicalMirrorSessionInner(ScreenId screenId, ScreenId defScreenId,
    ScreenProperty property)
{
#ifdef WM_MULTI_SCREEN_ENABLE
    sptr<ScreenSession> screenSession = nullptr;
    if (system::GetBoolParameter("persist.edm.disallow_mirror", false)) {
        TLOGW(WmsLogTag::DMS, "mirror disabled by edm!");
        return screenSession;
    }
    screenSession = GetFakePhysicalScreenSession(screenId, defScreenId, property);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return nullptr;
    }
    if (g_isPcDevice) {
        InitExtendScreenProperty(screenId, screenSession, property);
        screenSession->SetName(SCREEN_NAME_EXTEND);
        screenSession->SetIsExtend(true);
        screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    } else {
        screenSession->SetIsExtend(true);
        screenSession->SetName(SCREEN_NAME_CAST);
        screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
        screenSession->SetVirtualScreenFlag(VirtualScreenFlag::CAST);
    }
    GetAndMergeEdidInfo(screenSession);
    screenSession->SetMirrorScreenType(MirrorScreenType::PHYSICAL_MIRROR);
    screenSession->SetIsPcUse(g_isPcDevice ? true : false);
    screenSession->SetIsInternal(false);
    screenSession->SetIsRealScreen(true);
    screenSession->SetIsCurrentInUse(true);
    return screenSession;
#else
    return nullptr;
#endif
}

sptr<ScreenSession> ScreenSessionManager::GetPhysicalScreenSessionInner(ScreenId screenId, ScreenProperty property)
{
    ScreenId defScreenId = GetDefaultScreenId();
    TLOGW(WmsLogTag::DMS, "screenId:%{public}" PRIu64, screenId);
    if (IsDefaultMirrorMode(screenId)) {
#ifdef WM_MULTI_SCREEN_ENABLE
        return CreateFakePhysicalMirrorSessionInner(screenId, defScreenId, property);
#else
        return nullptr;
#endif
    }
    std::string screenName = "UNKNOWN";
    if (screenId == SCREEN_ID_MAIN) {
        screenName = "SubScreen";
    }
    ScreenId smsId = screenId;
    if (IsConcurrentUser()) {
        if (!screenIdManager_.ConvertToSmsScreenId(screenId, smsId)) {
            smsId = screenId;
        }
    }
    ScreenSessionConfig config = {
        .screenId = smsId,
        .rsId = screenId,
        .defaultScreenId = defScreenId,
        .name = screenName,
        .property = property,
    };
    sptr<ScreenSession> screenSession = nullptr;
    screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_WITHOUT_DISPLAY_NODE);
    screenSession->SetIsExtend(false);
    screenSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    screenSession->SetIsInternal(true);
    screenSession->SetIsRealScreen(true);
    screenSession->SetIsCurrentInUse(true);
    screenSession->SetIsPcUse(g_isPcDevice ? true : false);
    return screenSession;
}

sptr<ScreenSession> ScreenSessionManager::GetOrCreatePhysicalScreenSession(ScreenId screenId)
{
    TLOGW(WmsLogTag::DMS, "ENTER. ScreenId: %{public}" PRIu64, screenId);
    sptr<ScreenSession> screenSession = GetPhysicalScreenSession(screenId);
    if (screenSession) {
        TLOGW(WmsLogTag::DMS, "physical screen Exist ScreenId: %{public}" PRIu64, screenId);
        return screenSession;
    }

    ScreenProperty property;
    CreateScreenProperty(screenId, property);
    TLOGW(WmsLogTag::DMS, "create screen property end");

    sptr<ScreenSession> session = GetPhysicalScreenSessionInner(screenId, property);
    if (session == nullptr) {
        TLOGE(WmsLogTag::DMS, "get physical screen fail ScreenId: %{public}" PRIu64, screenId);
        return session;
    }
    InitExtendScreenDensity(session, property);
    InitAbstractScreenModesInfo(session);
    session->groupSmsId_ = 1;
    {
        std::lock_guard<std::recursive_mutex> lock(physicalScreenSessionMapMutex_);
        physicalScreenSessionMap_[screenId] = session;
    }
    SetHdrFormats(screenId, session);
    SetColorSpaces(screenId, session);
    SetSupportedRefreshRate(session);
    TLOGW(WmsLogTag::DMS, "Create success. ScreenId: %{public}" PRIu64, screenId);
    return session;
}

sptr<ScreenSession> ScreenSessionManager::GetScreenSessionByRsId(ScreenId rsScreenId)
{
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        sptr<ScreenSession> screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
            continue;
        }
        if (screenSession->GetRSScreenId() == rsScreenId) {
            TLOGI(WmsLogTag::DMS, "found rsScreenId = %{public}" PRIu64, rsScreenId);
            return screenSession;
        }
    }
    return nullptr;
}

sptr<ScreenSession> ScreenSessionManager::GetPhysicalScreenSession(ScreenId screenId) const
{
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        if (screenSessionMap_.empty()) {
            screenEventTracker_.LogWarningAllInfos();
        }
    }

    std::lock_guard<std::recursive_mutex> lock(physicalScreenSessionMapMutex_);
    auto iter = physicalScreenSessionMap_.find(screenId);
    if (iter == physicalScreenSessionMap_.end()) {
        TLOGW(WmsLogTag::DMS, "not found screen id: %{public}" PRIu64, screenId);
        return nullptr;
    }
    return iter->second;
}

void ScreenSessionManager::NotifyExtendScreenCreateFinish()
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return;
    }
    if (!g_isPcDevice) {
        TLOGW(WmsLogTag::DMS, "not pc device.");
        return;
    }
    sptr<ScreenSession> mainScreen = nullptr;
    sptr<ScreenSession> extendScreen = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (auto sessionIt : screenSessionMap_) {
            auto screenSession = sessionIt.second;
            if (screenSession == nullptr) {
                TLOGE(WmsLogTag::DMS, "screenSession is nullptr.");
                continue;
            }
            if (!screenSession->GetIsCurrentInUse()) {
                continue;
            }
            if (screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MAIN) {
                mainScreen = screenSession;
            } else {
                extendScreen = screenSession;
            }
        }
    }
    if (mainScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "main screen is null");
        return;
    }
    NotifyCreatedScreen(mainScreen);
    if (extendScreen == nullptr) {
        TLOGE(WmsLogTag::DMS, "extend screen is null");
        return;
    }
    NotifyCreatedScreen(extendScreen);
}

void ScreenSessionManager::UpdateScreenIdManager(sptr<ScreenSession>& innerScreen,
    sptr<ScreenSession>& externalScreen)
{
    ScreenId innerScreenId = innerScreen->GetRSScreenId();
    ScreenId externalScreenId = externalScreen->GetRSScreenId();
    screenIdManager_.UpdateScreenId(externalScreenId, innerScreen->GetScreenId());
    screenIdManager_.UpdateScreenId(innerScreenId, externalScreen->GetScreenId());
}

std::string ScreenSessionManager::DumperClientScreenSessions()
{
    std::string clientSessions = "";
    auto clientProxy = GetClientProxy();
    if (clientProxy) {
        clientSessions = clientProxy->OnDumperClientScreenSessions();
    }
    return clientSessions;
}

void ScreenSessionManager::SetMultiScreenModeChangeTracker(std::string changeProc)
{
    screenEventTracker_.RecordEvent(changeProc);
}

void ScreenSessionManager::NotifyCreatedScreen(sptr<ScreenSession> screenSession)
{
    if (!g_isPcDevice) {
        TLOGW(WmsLogTag::DMS, "not pc device.");
        return;
    }
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null");
        return;
    }
    std::ostringstream oss;
    oss << "screenId: " << screenSession->GetScreenId()
        << ", rsId: " << screenSession->GetRSScreenId();
    oss << std::endl;
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    if (screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
        TLOGW(WmsLogTag::DMS, "mirror, no need to notify.");
        return;
    }
    ScreenProperty property = screenSession->GetScreenProperty();
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        TLOGW(WmsLogTag::DMS, "super fold device, change by rotation.");
        screenSession->PropertyChange(property, ScreenPropertyChangeReason::ROTATION);
    } else {
        screenSession->PropertyChange(property, ScreenPropertyChangeReason::UNDEFINED);
    }
    NotifyScreenChanged(screenSession->ConvertToScreenInfo(), ScreenChangeEvent::CHANGE_MODE);
    NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(), DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
}

void ScreenSessionManager::NotifyExtendScreenDestroyFinish()
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return;
    }
    if (!g_isPcDevice) {
        TLOGW(WmsLogTag::DMS, "not pc device.");
        return;
    }
    TLOGI(WmsLogTag::DMS, "destroy finish, notify block");
    switchUserCV_.notify_all();
    userSwitching_ = false;
}

void ScreenSessionManager::SetRSScreenPowerStatus(ScreenId screenId, ScreenPowerStatus status, ScreenPowerEvent event)
{
    ScreenId rsScreenId = screenId;
    if (IsConcurrentUser()) {
        if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
            rsScreenId = screenId;
       }
    }
    ScreenPowerInfoType type = std::make_pair(rsScreenId, status);
    ScreenStateMachine::GetInstance().HandlePowerStateChange(event, type);
}

bool ScreenSessionManager::SetRSScreenPowerStatusExt(ScreenId screenId, ScreenPowerStatus status)
{
#ifdef FOLD_ABILITY_ENABLE
    bool isNeedToCancelSetScreenStatus = false;
    CheckAnotherScreenStatus(screenId, status, isNeedToCancelSetScreenStatus);
    if (isNeedToCancelSetScreenStatus) {
        TLOGW(WmsLogTag::DMS, "Cancel set screen:%{public}" PRIu64 "status:%{public}u", screenId,
            static_cast<uint32_t>(status));
        return false;
    }
#endif
    ScreenId rsScreenId = screenId;
    if (IsConcurrentUser()) {
        if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
            rsScreenId = screenId;
       }
    }
	rsInterface_.SetScreenPowerStatus(rsScreenId, status);
    if (status == ScreenPowerStatus::POWER_STATUS_ON) {
#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
        uint32_t ret = DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().NotifyBrightnessManagerScreenPowerStatus(
            static_cast<uint32_t>(screenId), static_cast<uint32_t>(status));
        TLOGI(WmsLogTag::DMS, "notify brightness, screenId:%{public}" PRIu64 ", status:%{public}u, ret = %{public}u",
            screenId, static_cast<uint32_t>(status), ret);
#endif
    }
    return true;
}

#ifdef FOLD_ABILITY_ENABLE
void ScreenSessionManager::CheckAnotherScreenStatus(ScreenId screenId, ScreenPowerStatus status,
    bool& isNeedToCancelSetScreenStatus) {
    if (!isCoordinationFlag_ && status == ScreenPowerStatus::POWER_STATUS_ON) {
        WaitAodOpNotify();
        ScreenId secondScreenId = screenId == SCREEN_ID_MAIN ? SCREEN_ID_FULL : SCREEN_ID_MAIN;
        PanelPowerStatus secondScreenStatus = PanelPowerStatus::INVALID_PANEL_POWER_STATUS;
        GetScreenLcdStatus(secondScreenId, secondScreenStatus);
        if (secondScreenStatus == PANEL_POWER_STATUS_ON) {
            TLOGI(WmsLogTag::DMS, "Another screen is on while set screen: %{public}" PRIu64 " on, do on and off",
                screenId);
            rsInterface_.SetScreenPowerStatus(secondScreenId, ScreenPowerStatus::POWER_STATUS_ON);
            rsInterface_.SetScreenPowerStatus(secondScreenId, ScreenPowerStatus::POWER_STATUS_OFF);
        } else {
            TLOGI(WmsLogTag::DMS, "Another screen status is not on while set screen: %{public}" PRIu64 " on,"
                " set screen power status directly", screenId);
        }
    } else if (!isCoordinationFlag_ && status == ScreenPowerStatus::POWER_STATUS_ON_ADVANCED) {
        WaitAodOpNotify();
        ScreenId secondScreenId = screenId == SCREEN_ID_MAIN ? SCREEN_ID_FULL : SCREEN_ID_MAIN;
        PanelPowerStatus secondScreenStatus = PanelPowerStatus::INVALID_PANEL_POWER_STATUS;
        GetScreenLcdStatus(secondScreenId, secondScreenStatus);
        if (secondScreenStatus == PANEL_POWER_STATUS_ON) {
            TLOGI(WmsLogTag::DMS, "Another screen is on while set screen: %{public}" PRIu64 " on advance, cancel",
                screenId);
            isNeedToCancelSetScreenStatus = true;
            return;
        } else {
            TLOGI(WmsLogTag::DMS, "Another screen status is not on while set screen: %{public}" PRIu64 " on advance,"
                " set screen power status directly", screenId);
        }
    }
}
#endif

void ScreenSessionManager::OnScreenModeChange(ScreenModeChangeEvent screenModeChangeEvent)
{
    TLOGI(WmsLogTag::DMS, "screenModeChangeEvent: %{public}d", static_cast<uint32_t>(screenModeChangeEvent));
    if (HasExtendVirtualScreen()) {
        TLOGE(WmsLogTag::DMS, "has extend virtual screen");
        return;
    }
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGE(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy->OnScreenModeChanged(screenModeChangeEvent);
}

void ScreenSessionManager::NotifyScreenMaskAppear()
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied.calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    if (!g_isPcDevice) {
        TLOGW(WmsLogTag::DMS, "not pc device.");
        return;
    }
    TLOGI(WmsLogTag::DMS, "screen mask appeared, notify block");
    screenMaskCV_.notify_all();
}

bool ScreenSessionManager::GetKeyboardState()
{
#ifdef FOLD_ABILITY_ENABLE
    return SuperFoldStateManager::GetInstance().GetKeyboardState();
#endif
    return false;
}

DMError ScreenSessionManager::GetScreenAreaOfDisplayArea(DisplayId displayId, const DMRect& displayArea,
    ScreenId& screenId, DMRect& screenArea)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied!  calling: %{public}s, calling pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGI(WmsLogTag::DMS, "displayId:%{public}" PRIu64 ",displayArea:%{public}d,%{public}d,%{public}d,%{public}d",
        displayId, displayArea.posX_, displayArea.posY_, displayArea.width_, displayArea.height_);
    auto displayInfo = GetDisplayInfoById(displayId);
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get displayInfo");
        return DMError::DM_ERROR_NULLPTR;
    }
    DMRect displayRegion =
        { displayInfo->GetOffsetX(), displayInfo->GetOffsetY(), displayInfo->GetWidth(), displayInfo->GetHeight() };
    if (!displayArea.IsInsideOf(displayRegion)) {
        TLOGE(WmsLogTag::DMS, "displayArea is outSide of displayRegion");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    screenId = displayInfo->GetScreenId();
    auto screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    RRect bounds = screenSession->GetScreenProperty().GetPhyBounds();
    DMRect screenRegion =
        { bounds.rect_.GetLeft(), bounds.rect_.GetTop(), bounds.rect_.GetWidth(), bounds.rect_.GetHeight() };
    DMRect displayAreaFixed = displayArea;
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && GetFoldStatus() == FoldStatus::HALF_FOLD) {
        if (displayId == DISPLAY_ID_FAKE) {
            displayAreaFixed.posY_ += screenRegion.height_ - displayRegion.height_;
        }
        displayRegion.height_ = screenRegion.height_;
    } else if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice() && GetFoldDisplayMode() == FoldDisplayMode::FULL) {
        SetDisplayRegionAndAreaFixed(displayInfo->GetRotation(), displayRegion, displayAreaFixed);
    }
    CalculateRotatedDisplay(displayInfo->GetRotation(), screenRegion, displayRegion, displayAreaFixed);
    CalculateScreenArea(displayRegion, displayAreaFixed, screenRegion, screenArea);
    TLOGI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 ",screenArea:%{public}d,%{public}d,%{public}d,%{public}d",
        screenId, screenArea.posX_, screenArea.posY_, screenArea.width_, screenArea.height_);
    return DMError::DM_OK;
}
void ScreenSessionManager::SetDisplayRegionAndAreaFixed(Rotation rotation, DMRect& displayRegion,
    DMRect& displayAreaFixed)
{
    switch (rotation) {
        case Rotation::ROTATION_0:
            displayRegion.posX_ = screenParams_[FULL_STATUS_OFFSET_X];
            displayAreaFixed.posX_ += screenParams_[FULL_STATUS_OFFSET_X];
            break;
        case Rotation::ROTATION_90:
            displayRegion.posY_ = screenParams_[FULL_STATUS_OFFSET_X];
            displayAreaFixed.posY_ += screenParams_[FULL_STATUS_OFFSET_X];
            break;
        default:
            break;
    }
}

void ScreenSessionManager::CalculateRotatedDisplay(Rotation rotation, const DMRect& screenRegion,
    DMRect& displayRegion, DMRect& displayArea)
{
    std::vector<std::string> phyOffsets = FoldScreenStateInternel::GetPhyRotationOffset();
    int32_t phyOffset = 0;
    if (phyOffsets.size() > 1 &&
        (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice() || GetFoldStatus() != FoldStatus::FOLDED)) {
        if (!ScreenSettingHelper::ConvertStrToInt32(phyOffsets[1], phyOffset)) {
            TLOGE(WmsLogTag::DMS, "transfer phyOffset1 failed.");
            return;
        }
    } else if (phyOffsets.size() > 0) {
        if (!ScreenSettingHelper::ConvertStrToInt32(phyOffsets[0], phyOffset)) {
            TLOGE(WmsLogTag::DMS, "transfer phyOffset0 failed.");
            return;
        }
    }
    Rotation phyOffsetRotation = ConvertIntToRotation(phyOffset);
    auto rotaionOffset = GetConfigCorrectionByDisplayMode(GetFoldDisplayMode());
    uint32_t correctedRotation = (static_cast<uint32_t>(rotation) + static_cast<uint32_t>(phyOffsetRotation)
        - static_cast<uint32_t>(rotaionOffset)) % ROTATION_MOD;
    DMRect displayRegionCopy = displayRegion;
    DMRect displayAreaCopy = displayArea;
    switch (static_cast<Rotation>(correctedRotation)) {
        case Rotation::ROTATION_90:
            displayRegion.width_ = displayRegionCopy.height_;
            displayRegion.height_ = displayRegionCopy.width_;
            displayArea.width_ = displayAreaCopy.height_;
            displayArea.height_ = displayAreaCopy.width_;
            displayRegion.posX_ = displayRegionCopy.posY_;
            displayRegion.posY_ = static_cast<int32_t>(screenRegion.width_) -
                (displayRegionCopy.posX_ + static_cast<int32_t>(displayRegionCopy.width_));
            displayArea.posX_ = displayAreaCopy.posY_;
            displayArea.posY_ = static_cast<int32_t>(screenRegion.width_) -
                (displayAreaCopy.posX_ + static_cast<int32_t>(displayAreaCopy.width_));
            break;
        case Rotation::ROTATION_180:
            displayRegion.posX_ = static_cast<int32_t>(screenRegion.width_) -
                (displayRegionCopy.posX_ + static_cast<int32_t>(displayRegionCopy.width_));
            displayRegion.posY_ = static_cast<int32_t>(screenRegion.height_) -
                (displayRegionCopy.posY_ + static_cast<int32_t>(displayRegionCopy.height_));
            displayArea.posX_ = static_cast<int32_t>(screenRegion.width_) -
                (displayAreaCopy.posX_ + static_cast<int32_t>(displayAreaCopy.width_));
            displayArea.posY_ = static_cast<int32_t>(screenRegion.height_) -
                (displayAreaCopy.posY_ + static_cast<int32_t>(displayAreaCopy.height_));
            break;
        case Rotation::ROTATION_270:
            displayRegion.width_ = displayRegionCopy.height_;
            displayRegion.height_ = displayRegionCopy.width_;
            displayArea.width_ = displayAreaCopy.height_;
            displayArea.height_ = displayAreaCopy.width_;
            displayRegion.posX_ = static_cast<int32_t>(screenRegion.height_) -
                (displayRegionCopy.posY_ + static_cast<int32_t>(displayRegionCopy.height_));
            displayRegion.posY_ = displayRegionCopy.posX_;
            displayArea.posX_ = static_cast<int32_t>(screenRegion.height_) -
                (displayAreaCopy.posY_ + static_cast<int32_t>(displayAreaCopy.height_));
            displayArea.posY_ = displayAreaCopy.posX_;
            break;
        default:
            break;
    }
}

void ScreenSessionManager::CalculateScreenArea(const DMRect& displayRegion, const DMRect& displayArea,
    const DMRect& screenRegion, DMRect& screenArea)
{
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
       screenArea = displayArea;
       return;
    }
    if (displayRegion == screenRegion) {
        screenArea = displayArea;
        return;
    }
    float ratioX = static_cast<float>(displayArea.posX_ - displayRegion.posX_) /
        static_cast<float>(displayRegion.width_);
    float ratioY = static_cast<float>(displayArea.posY_ - displayRegion.posY_) /
        static_cast<float>(displayRegion.height_);
    float ratioWidth = static_cast<float>(displayArea.width_) / static_cast<float>(displayRegion.width_);
    float ratioHeight = static_cast<float>(displayArea.height_) / static_cast<float>(displayRegion.height_);
    screenArea.posX_ = screenRegion.posX_ + static_cast<int32_t>(ratioX * screenRegion.width_);
    screenArea.posY_ = screenRegion.posY_ + static_cast<int32_t>(ratioY * screenRegion.height_);
    screenArea.width_ = static_cast<uint32_t>(ratioWidth * screenRegion.width_);
    screenArea.height_ = static_cast<uint32_t>(ratioHeight * screenRegion.height_);
}

DMError ScreenSessionManager::SetPrimaryDisplaySystemDpi(float virtualPixelRatio)
{
    sptr<DisplayInfo> displayInfo = GetPrimaryDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayInfo is null.");
        return DMError::DM_ERROR_NULLPTR;
    }
    ScreenId screenId = displayInfo->GetScreenId();
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null.");
        return DMError::DM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::DMS, "displayId: %{public}" PRIu64 " densityInCurResolution: %{public}f",
        displayInfo->GetDisplayId(), screenSession->GetDensityInCurResolution());
    screenSession->SetDensityInCurResolution(virtualPixelRatio);
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetVirtualScreenAutoRotation(ScreenId screenId, bool enable)
{
    TLOGI(WmsLogTag::DMS, "enter");
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_INVALID_PERMISSION;
    }

    ScreenId rsScreenId = SCREEN_ID_INVALID;
    bool res = ConvertScreenIdToRsScreenId(screenId, rsScreenId);
    if (!res) {
        TLOGE(WmsLogTag::DMS, "convert screenId to rsScreenId failed");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    TLOGI(WmsLogTag::DMS, "unique screenId: %{public}" PRIu64 " rsScreenId: %{public}" PRIu64, screenId, rsScreenId);

    auto ret = rsInterface_.SetVirtualScreenAutoRotation(rsScreenId, enable);
    if (ret != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS, "rsInterface error: %{public}d", ret);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return DMError::DM_OK;
}

void ScreenSessionManager::CheckPidAndClearModifiers(int32_t userId, std::shared_ptr<RSDisplayNode>& displayNode)
{
    if (displayNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "diplay node is nullptr");
        return;
    }
    TLOGI(WmsLogTag::DMS, "userId: %{public}d, displayNodeId: %{public}" PRIu64 "", userId, displayNode->GetId());
    int32_t newScbPid = -1;
    {
        std::lock_guard<std::recursive_mutex> lock(userPidMapMutex_);
        auto pidIter = userPidMap_.find(userId);
        if (pidIter == userPidMap_.end()) {
            TLOGE(WmsLogTag::DMS, "user not found in userPidMap");
            return;
        }
        newScbPid = pidIter->second;
    }
    int32_t currentNodePid = -1;
    {
        std::lock_guard<std::recursive_mutex> lock(userDisplayNodeMapMutex_);
        auto nodeIter = displayNodePidMap_.find(displayNode->GetId());
        if (nodeIter == displayNodePidMap_.end()) {
            TLOGE(WmsLogTag::DMS, "display node not found in nodePidMap");
            return;
        }
        currentNodePid = nodeIter->second;
        TLOGI(WmsLogTag::DMS, "displayNode's old pid: %{public}d, newScbPid: %{public}d", nodeIter->second, newScbPid);
        if (currentNodePid == newScbPid) {
            return;
        }
        nodeIter->second = newScbPid;
    }
    TLOGI(WmsLogTag::DMS, "pid changed, clear modifiers");
    displayNode->ClearModifierByPid(currentNodePid);
}

void ScreenSessionManager::AddOrUpdateUserDisplayNode(int32_t userId, ScreenId screenId,
    std::shared_ptr<RSDisplayNode>& displayNode)
{
    TLOGI(WmsLogTag::DMS, "userId: %{public}d, screenId: %{public}" PRIu64 "", userId, screenId);
    if (displayNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "display node is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(userDisplayNodeMapMutex_);
    userDisplayNodeMap_[userId].insert_or_assign(screenId, displayNode);
    {
        std::lock_guard<std::recursive_mutex> lock(userPidMapMutex_);
        auto pidIter = userPidMap_.find(userId);
        if (pidIter != userPidMap_.end()) {
            displayNodePidMap_[displayNode->GetId()] = pidIter->second;
        }
    }
    TLOGI(WmsLogTag::DMS, "success");
}

void ScreenSessionManager::RemoveUserDisplayNode(int32_t userId, ScreenId screenId)
{
    std::lock_guard<std::recursive_mutex> lock(userDisplayNodeMapMutex_);
    auto iter = userDisplayNodeMap_.find(userId);
    if (iter == userDisplayNodeMap_.end()) {
        TLOGE(WmsLogTag::DMS, "userId: %{public}d not found", userId);
        return;
    }
    if (userDisplayNodeMap_[userId].find(screenId) == userDisplayNodeMap_[userId].end()) {
        TLOGE(WmsLogTag::DMS, "displayNode not exist, userId: %{public}d, screenId %{public}" PRIu64 "",
            userId, screenId);
        return;
    }
    TLOGI(WmsLogTag::DMS, "remove displayNode, userId: %{public}d, screenId %{public}" PRIu64 "",
        userId, screenId);
    auto displayNode = userDisplayNodeMap_[userId][screenId];
    if (displayNode != nullptr) {
        displayNodePidMap_.erase(displayNode->GetId());
    }
    userDisplayNodeMap_[userId].erase(screenId);
    if (userDisplayNodeMap_[userId].size() <= 0) {
        userDisplayNodeMap_.erase(userId);
    }
}

std::map<ScreenId, std::shared_ptr<RSDisplayNode>> ScreenSessionManager::GetUserDisplayNodeMap(int32_t userId)
{
    std::lock_guard<std::recursive_mutex> lock(userDisplayNodeMapMutex_);
    auto iter = userDisplayNodeMap_.find(userId);
    if (iter == userDisplayNodeMap_.end()) {
        TLOGE(WmsLogTag::DMS, "userId: %{public}d not found", userId);
        return {};
    }
    return iter->second;
}

void ScreenSessionManager::SwitchUserDealUserDisplayNode(int32_t newUserId)
{
    auto newUserDisplayNodeMap = GetUserDisplayNodeMap(newUserId);
    std::ostringstream oss;
    for (auto sessionIt : newUserDisplayNodeMap) {
        oss << sessionIt.first << ",";
    }
    std::vector<ScreenId> phyScreenIds;
    std::map<ScreenId, sptr<ScreenSession>> screenSessionMapCopy;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        screenSessionMapCopy = screenSessionMap_;
    }
    for (auto sessionIt : screenSessionMapCopy) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGI(WmsLogTag::DMS, "screenSession is nullptr, screenId: %{public}" PRIu64 "", sessionIt.first);
            continue;
        }
        if (!screenSession->GetIsRealScreen()) {
            continue;
        }
        TLOGI(WmsLogTag::DMS, "start set new user displayNode, newUserId: %{public}d", newUserId);
        auto screenId = screenSession->GetScreenId();
        phyScreenIds.emplace_back(screenId);
        if (newUserDisplayNodeMap.find(screenId) == newUserDisplayNodeMap.end()) {
            Rosen::RSDisplayNodeConfig rsConfig;
            rsConfig.screenId = screenId;
            screenSession->CreateDisplayNode(rsConfig);
            std::shared_ptr<RSDisplayNode> displayNode = screenSession->GetDisplayNode();
            if (displayNode == nullptr) {
                TLOGI(WmsLogTag::DMS, "displayNode is null");
                continue;
            }
            TLOGI(WmsLogTag::DMS, "screen id %{public}" PRIu64 "", screenId);
            AddOrUpdateUserDisplayNode(newUserId, screenId, displayNode);
        } else {
            auto displayNode = newUserDisplayNodeMap[screenId];
            if (displayNode == nullptr) {
                TLOGE(WmsLogTag::DMS, "displayNode in map is nullptr");
                continue;
            }
            CheckPidAndClearModifiers(newUserId, displayNode);
            screenSession->SetDisplayNode(displayNode);
            RSTransactionAdapter::FlushImplicitTransaction(screenSession->GetRSUIContext());
        }
    }
    for (auto newUserMapIt : newUserDisplayNodeMap) {
        auto screenId = newUserMapIt.first;
        if (std::find(phyScreenIds.begin(), phyScreenIds.end(), screenId) == phyScreenIds.end()) {
            RemoveUserDisplayNode(newUserId, screenId);
        }
    }
}

void ScreenSessionManager::AddUserDisplayNodeOnTree(int32_t userId)
{
    TLOGI(WmsLogTag::DMS, "userId: %{public}d", userId);
    if (!CheckUserIsForeground(userId)) {
        TLOGI(WmsLogTag::DMS, "user: %{public}d is background not on tree", userId);
        return;
    }
    auto userDisplayNodeMap = GetUserDisplayNodeMap(userId);
    std::unordered_set<std::shared_ptr<RSUIContext>> rsUIContexts;
    for (auto userDisplayNodeIt : userDisplayNodeMap) {
        auto displayNode = userDisplayNodeIt.second;
        if (displayNode == nullptr) {
            continue;
        }
        displayNode->AddDisplayNodeToTree();
        auto screenId = userDisplayNodeIt.first;
        auto screenSession = GetScreenSession(screenId);
        if (!screenSession) {
            TLOGW(WmsLogTag::DMS, "screen Session is null");
            continue;
        }
        rsUIContexts.insert(screenSession->GetRSUIContext());
    }
    RSTransactionAdapter::FlushImplicitTransaction(rsUIContexts);
}

void ScreenSessionManager::RemoveUserDisplayNodeFromTree(int32_t userId)
{
    TLOGI(WmsLogTag::DMS, "userId: %{public}d", userId);
    if (CheckUserIsForeground(userId)) {
        TLOGI(WmsLogTag::DMS, "user: %{public}d is foreground remove from tree", userId);
        return;
    }
    auto userDisplayNodeMap = GetUserDisplayNodeMap(userId);
    std::unordered_set<std::shared_ptr<RSUIContext>> rsUIContexts;
    for (auto userDisplayNodeIt : userDisplayNodeMap) {
        auto displayNode = userDisplayNodeIt.second;
        if (displayNode == nullptr) {
            continue;
        }
        displayNode->RemoveDisplayNodeFromTree();
        auto screenId = userDisplayNodeIt.first;
        auto screenSession = GetScreenSession(screenId);
        if (!screenSession) {
            TLOGW(WmsLogTag::DMS, "screen Session is null");
            continue;
        }
        rsUIContexts.insert(screenSession->GetRSUIContext());
    }
    RSTransactionAdapter::FlushImplicitTransaction(rsUIContexts);
}

bool ScreenSessionManager::CheckUserIsForeground(int32_t userId)
{
    bool isForeground = false;
    ErrCode err = AccountSA::OsAccountManager::IsOsAccountForeground(userId, isForeground);
    if (err != ERR_OK) {
        TLOGE(WmsLogTag::DMS, "errorCode: %{public}d, user: %{public}d", err, userId);
    }
    return isForeground;
}

void ScreenSessionManager::SetUserDisplayNodePositionZ(int32_t userId, float positionZ)
{
    TLOGI(WmsLogTag::DMS, "userId: %{public}d, positionZ: %{public}f", userId, positionZ);
    auto userDisplayNodeMap = GetUserDisplayNodeMap(userId);
    std::unordered_set<std::shared_ptr<RSUIContext>> rsUIContexts;
    for (auto userDisplayNodeIt : userDisplayNodeMap) {
        auto displayNode = userDisplayNodeIt.second;
        if (displayNode == nullptr) {
            continue;
        }
        displayNode->SetPositionZ(positionZ);
        auto screenId = userDisplayNodeIt.first;
        auto screenSession = GetScreenSession(screenId);
        if (!screenSession) {
            TLOGW(WmsLogTag::DMS, "screen Session is null");
            continue;
        }
        rsUIContexts.insert(screenSession->GetRSUIContext());
    }
    RSTransactionAdapter::FlushImplicitTransaction(rsUIContexts);
}

void ScreenSessionManager::HandleNewUserDisplayNode(int32_t newUserId, bool coldBoot)
{
    std::unique_lock<std::mutex> lock(switchUserDisplayNodeMutex_);
    TLOGI(WmsLogTag::DMS, "newUserId: %{public}d, coldBoot: %{public}d", newUserId, coldBoot);
    SetUserDisplayNodePositionZ(currentUserId_, POSITION_Z_DEFAULT);
    if (!coldBoot) {
        TLOGI(WmsLogTag::DMS, "deal with userDisplayNode");
        SwitchUserDealUserDisplayNode(newUserId);
    }
    SetUserDisplayNodePositionZ(newUserId, POSITION_Z_LOW);
    AddUserDisplayNodeOnTree(newUserId);
    MakeMirrorAfterSwitchUser();
}

void ScreenSessionManager::WaitSwitchUserAnimateFinish(int32_t newUserId, bool isColdSwitch)
{
    TLOGI(WmsLogTag::DMS, "enter, newUserId: %{public}d, isColdSwitch: %{public}d", newUserId, isColdSwitch);
    std::unique_lock<std::mutex> lock(switchUserDisplayNodeMutex_);
    uint32_t waitTimes = isColdSwitch ? COLD_SWITCH_ANIMATE_TIMEOUT_MILLISECONDS :
                                        HOT_SWITCH_ANIMATE_TIMEOUT_MILLISECONDS;
    if (!DmUtils::safe_wait_for(switchUserDisplayNodeCV_, lock, std::chrono::milliseconds(waitTimes),
        [this] { return animateFinishAllNotified_; })) {
        TLOGI(WmsLogTag::DMS, "switch user animate timeout");
    }
    animateFinishAllNotified_ = false;
    auto clientProxy = GetClientProxy();
    if (!clientProxy) {
        TLOGE(WmsLogTag::DMS, "clientProxy is null");
        return;
    }
    clientProxy->OnAnimationFinish();
    SetUserDisplayNodePositionZ(newUserId, POSITION_Z_HIGH);
    RemoveUserDisplayNodeFromTree(currentUserId_);
    TLOGI(WmsLogTag::DMS, "success");
}

void ScreenSessionManager::MakeMirrorAfterSwitchUser()
{
    TLOGI(WmsLogTag::DMS, "start to make mirror");
    sptr<ScreenSession> mainSession = nullptr;
    std::vector<sptr<ScreenSession>> mirrorScreenSessions;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (auto it : screenSessionMap_) {
            auto screenSession = it.second;
            if (!screenSession) {
                TLOGW(WmsLogTag::DMS, "screen session is null");
                continue;
            }
            if (screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
                mirrorScreenSessions.emplace_back(screenSession);
                continue;
            }
            if (screenSession->GetScreenCombination() == ScreenCombination::SCREEN_MAIN) {
                mainSession = screenSession;
            }
        }
        if (mirrorScreenSessions.empty()) {
            TLOGI(WmsLogTag::DMS, "no mirror screens, no need to make mirror");
            return;
        }
        if (mainSession == nullptr || mainSession->GetDisplayNode() == nullptr) {
            TLOGE(WmsLogTag::DMS, "make mirror error, no main screen");
            return;
        }
    }
    for (auto session : mirrorScreenSessions) {
        session->ReuseDisplayNode({ session->rsId_, true, mainSession->GetDisplayNode()->GetId() });
    }
}

void ScreenSessionManager::NotifySwitchUserAnimationFinish()
{
    TLOGI(WmsLogTag::DMS, "enter");
    std::unique_lock<std::mutex> lock(switchUserDisplayNodeMutex_);
    animateFinishAllNotified_ = true;
    switchUserDisplayNodeCV_.notify_all();
}

bool ScreenSessionManager::SetScreenOffset(ScreenId screenId, float offsetX, float offsetY)
{
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is null.");
        return false;
    }
    TLOGI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 " rsId_:%{public}" PRIu64 \
        " GetRSScreenId:%{public}" PRIu64 " offsetX:%{public}f, offsetY:%{public}f",
        screenId, screenSession->rsId_, screenSession->GetRSScreenId(), offsetX, offsetY);
    RSInterfaces::GetInstance().SetScreenOffset(screenSession->rsId_, offsetX, offsetY);
    return true;
}

DMError ScreenSessionManager::SetScreenPrivacyWindowTagSwitch(ScreenId screenId,
    const std::vector<std::string>& privacyWindowTag, bool enable)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    auto screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "cannot find screenInfo: %{public}" PRIu64, screenId);
        return DMError::DM_ERROR_NULLPTR;
    }
    MockSessionManagerService::GetInstance().SetScreenPrivacyWindowTagSwitch(
        screenIdManager_.ConvertToRsScreenId(screenId), privacyWindowTag, enable);
    std::ostringstream oss;
    for (auto tag : privacyWindowTag) {
        oss << tag << ",";
    }
    TLOGI(WmsLogTag::DMS, "screenId:%{public}" PRIu64 " privacyWindowTag:%{public}s, enable:%{public}d", screenId,
        oss.str().c_str(), enable);
    return DMError::DM_OK;
}
// LCOV_EXCL_STOP

ScreenId ScreenSessionManager::GetPhyScreenId(ScreenId screenId)
{
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice() &&
        GetCoordinationFlag() && screenId == SCREEN_ID_MAIN) {
        TLOGW(WmsLogTag::DMS, "Coordination phyScreen SCREEN_ID_FULL");
        return SCREEN_ID_FULL;
    }
    return screenId;
}

void ScreenSessionManager::UpdateCoordinationRefreshRate(uint32_t refreshRate)
{
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice() || !GetCoordinationFlag()) {
        return;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(SCREEN_ID_MAIN);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession SCREEN_ID_MAIN is null");
        return;
    }
    screenSession->UpdateRefreshRate(refreshRate);
    NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(), DisplayChangeEvent::UPDATE_REFRESHRATE);
}

// LCOV_EXCL_START
bool ScreenSessionManager::SynchronizePowerStatus(ScreenPowerState state)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }
    TLOGI(WmsLogTag::DMS, "[UL_POWER] Synchronize power state: %{public}u", static_cast<uint32_t>(state));
#ifdef FOLD_ABILITY_ENABLE
    ScreenPowerStatus status = ScreenPowerStatus::INVALID_POWER_STATUS;
    if (!GetPowerStatus(state, PowerStateChangeReason::STATE_CHANGE_REASON_SYNCHRONIZE_POWER_STATE, status)) {
        TLOGE(WmsLogTag::DMS, "GetPowerStatus failed");
        return false;
    }
    if (foldScreenController_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "foldScreenController_ is null");
        return false;
    }
    ScreenId screenId = foldScreenController_->GetCurrentScreenId();
    auto rsSetScreenPowerStatusTask = [this, screenId, status] {
        auto event = ConvertScreenStateEvent(status);
        SetRSScreenPowerStatus(screenId, status, event);
    };
    screenPowerTaskScheduler_->PostVoidSyncTask(rsSetScreenPowerStatusTask, "rsInterface_.SetScreenPowerStatus task");
#endif
    return true;
}

Rotation ScreenSessionManager::RemoveRotationCorrection(Rotation rotation)
{
    return RemoveRotationCorrection(rotation, GetFoldDisplayMode());
}

Rotation ScreenSessionManager::RemoveRotationCorrection(Rotation rotation, FoldDisplayMode foldDisplayMode)
{
    auto correctionRotation = GetConfigCorrectionByDisplayMode(foldDisplayMode);
    return static_cast<Rotation>((static_cast<uint32_t>(rotation) -
        static_cast<uint32_t>(correctionRotation) + ROTATION_MOD) % ROTATION_MOD);
}

FoldDisplayMode ScreenSessionManager::GetFoldDisplayModeAfterRotation() const
{
    return foldDisplayModeAfterRotation_.load();
}

void ScreenSessionManager::SetFoldDisplayModeAfterRotation(FoldDisplayMode foldDisplayMode)
{
    foldDisplayModeAfterRotation_.store(foldDisplayMode);
    TLOGI(WmsLogTag::DMS, "foldDisplayModeAfterRotation: %{public}d", foldDisplayMode);
}
 
Rotation ScreenSessionManager::GetConfigCorrectionByDisplayMode(FoldDisplayMode displayMode)
{
    if (!CORRECTION_ENABLE) {
        return Rotation::ROTATION_0;
    }
    InitRotationCorrectionMap(DISPLAYMODE_CORRECTION);
    std::shared_lock<std::shared_mutex> lock(rotationCorrectionMutex_);
    auto iter = rotationCorrectionMap_.find(displayMode);
    if (iter == rotationCorrectionMap_.end()) {
        return Rotation::ROTATION_0;
    }
    TLOGI(WmsLogTag::DMS, "offset:%{public}d", iter->second);
    return static_cast<Rotation>(iter->second);
}
 
void ScreenSessionManager::InitRotationCorrectionMap(std::string displayModeCorrectionConfig)
{
    if (!rotationCorrectionMap_.empty()) {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(rotationCorrectionMutex_);
    rotationCorrectionMap_.clear();
    std::vector<std::string> displayModeCorrections = {};
    bool splitSuccess = ScreenSettingHelper::SplitString(displayModeCorrections, displayModeCorrectionConfig, ';');
    if (!splitSuccess) {
        TLOGE(WmsLogTag::DMS, "invalid config");
        return;
    }
    for (const std::string& displayModeCorrection : displayModeCorrections) {
        std::vector<std::string> corrections = {};
        splitSuccess = ScreenSettingHelper::SplitString(corrections, displayModeCorrection, ',');
        uint32_t dataSize = corrections.size();
        if (!splitSuccess || dataSize != EXPECT_DISPLAY_MODE_CORRECTION_SIZE) {
            TLOGE(WmsLogTag::DMS, "invalid mode config");
            continue;
        }
        if (!ScreenSettingHelper::IsNumber(corrections[0]) ||
            !ScreenSettingHelper::IsNumber(corrections[1])) {
            TLOGE(WmsLogTag::DMS, "config not int:%{public}s,%{public}s",
                corrections[0].c_str(), corrections[1].c_str());
            continue;
        }
        auto displayModeValue =
            static_cast<uint32_t>(strtoll(corrections[0].c_str(), nullptr, PARAM_NUM_TEN));
        auto offset = static_cast<uint32_t>(strtoll(corrections[1].c_str(), nullptr, PARAM_NUM_TEN));
        FoldDisplayMode displayMode = static_cast<FoldDisplayMode>(displayModeValue);
        rotationCorrectionMap_.insert({displayMode, offset});
        TLOGI(WmsLogTag::DMS, "rotationCorrectionMap add:%{public}d-%{public}d", displayMode, offset);
    }
}

std::shared_ptr<TaskScheduler> ScreenSessionManager::GetPowerTaskScheduler() const
{
    return screenPowerTaskScheduler_;
}

std::shared_ptr<FfrtQueueHelper> ScreenSessionManager::GetFfrtQueueHelper() const
{
    return ffrtQueueHelper_;
}

bool ScreenSessionManager::GetCancelSuspendStatus() const
{
    if (!sessionDisplayPowerController_) {
        TLOGE(WmsLogTag::DMS, "sessionDisplayPowerController_ is null");
        return false;
    }
    std::lock_guard<std::mutex> notifyLock(sessionDisplayPowerController_->notifyMutex_);
    return sessionDisplayPowerController_->needCancelNotify_ ||
            sessionDisplayPowerController_->canceledSuspend_;
}

void ScreenSessionManager::RegisterSettingDuringCallStateObserver()
{
    TLOGI(WmsLogTag::DMS, "Register Setting During Call State Observer");
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) { UpdateDuringCallState(); };
    ScreenSettingHelper::RegisterSettingDuringCallStateObserver(updateFunc);
}

void ScreenSessionManager::UpdateDuringCallState()
{
    TLOGI(WmsLogTag::DMS, "update during call state, current state: %{public}d", duringCallState_);
    bool ret = ScreenSettingHelper::GetSettingDuringCallState(duringCallState_);
    if (!ret) {
        TLOGE(WmsLogTag::DMS, "get setting during call state failed");
        return;
    }
    TLOGI(WmsLogTag::DMS, "get setting during call state: %{public}d", duringCallState_);
#ifdef FOLD_ABILITY_ENABLE
    if (ScreenSceneConfig::IsSupportDuringCall() && !duringCallState_ && foldScreenController_ != nullptr &&
        foldScreenController_->GetDisplayMode() == FoldDisplayMode::SUB) {
        TLOGI(WmsLogTag::DMS, "duringcallstate exit, recover displaymode");
        foldScreenController_->RecoverDisplayMode();
    }
#endif
}

void ScreenSessionManager::SetDuringCallState(bool value)
{
    bool ret = ScreenSettingHelper::SetSettingDuringCallState("during_call_state", value);
    TLOGI(WmsLogTag::DMS, "set during call state to %{public}d, ret:%{public}d", value, ret);
}

bool ScreenSessionManager::GetFirstSCBConnect()
{
    return firstSCBConnect_;
}

void ScreenSessionManager::SetFirstSCBConnect(bool firstSCBConnect)
{
    firstSCBConnect_ = firstSCBConnect;
}

DMError ScreenSessionManager::SyncScreenPropertyChangedToServer(ScreenId screenId, const ScreenProperty& screenProperty)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "SetResolution permission denied! calling pid: %{public}d", IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    auto bounds = screenProperty.GetBounds();
    TLOGI(WmsLogTag::DMS, "SyncScreenPropertyChangedToServer screenId: %{public}" PRIu64
        ", rotation:%{public}f, width:%{public}f, height:%{public}f", screenId,
        screenProperty.GetRotation(), bounds.rect_.GetWidth(), bounds.rect_.GetHeight());

    auto serverScreenSession = GetScreenSession(screenId);
    if (serverScreenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "cannot get screenInfo: %{public}" PRIu64, screenId);
        return DMError::DM_ERROR_NULLPTR;
    }
    serverScreenSession->UpdateScbScreenPropertyToServer(screenProperty);
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (propertyChangedCallback_) {
        TLOGI(WmsLogTag::DMS, "screensessionmanager callback propertychanged");
        // only super fold device enter, so the second parameter is super fold change event
        propertyChangedCallback_(serverScreenSession, screenProperty.GetSuperFoldStatusChangeEvent());
    }
    if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice() ||
        FoldScreenStateInternel::IsSingleDisplayFoldDevice()) {
        NotifyDisplayChanged(serverScreenSession->ConvertToDisplayInfo(), DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    }
    return DMError::DM_OK;
}

void ScreenSessionManager::SetPropertyChangedCallback(
    std::function<void(sptr<ScreenSession>& screenSession, SuperFoldStatusChangeEvents changeEvent)> callback)
{
    TLOGI(WmsLogTag::DMS, "set callback is valid: %{public}s",callback ? "true" : "false");
    std::lock_guard<std::mutex> lock(callbackMutex_);
    propertyChangedCallback_ = callback;
}

bool ScreenSessionManager::GetStoredPidFromUid(int32_t uid, int32_t& agentPid) const
{
    auto iter = uidAndPidMap_.find(uid);
    if (iter == uidAndPidMap_.end() || iter->second != agentPid) {
        TLOGNI(WmsLogTag::DMS, "no notify");
        return true;
    }
    return false;
}

void ScreenSessionManager::NotifyAodOpCompletion(AodOP operation, int32_t result)
{
    TLOGI(WmsLogTag::DMS, "AOD operation completed, operation: %{public}d, result: %{public}d", operation, result);
    ScreenPowerInfoType params;
    if (operation == AodOP::ENTER_FINISH) {
        isInAodOperation_ = false;
        aodOpCompleteCV_.notify_all();
        if (ScreenStateMachine::GetInstance().GetTransitionState() == ScreenTransitionState::WAIT_LOCK_SCREEN_IND) {
            aodNotifyFlag_ = (result == 0) ? AodStatus::SUCCESS : AodStatus::FAILURE;
        } else {
            ScreenStateMachine::GetInstance().HandlePowerStateChange(result == 0 ?
                ScreenPowerEvent::AOD_ENTER_SUCCESS : ScreenPowerEvent::AOD_ENTER_FAIL, params);
        }
    } else if (operation == AodOP::ENTER_START) {
        isInAodOperation_ = true;
    } else if (operation == AodOP::EXIT_START) {
        isInAodOperation_ = true;
    } else if (operation == AodOP::EXIT_FINISH) {
        isInAodOperation_ = false;
        aodOpCompleteCV_.notify_all();
    }
}

#ifdef FOLD_ABILITY_ENABLE
bool ScreenSessionManager::WaitAodOpNotify()
{
    if (isInAodOperation_) {
        TLOGI(WmsLogTag::DMS, "Start wait aod op notify");
        std::unique_lock<std::mutex> lock(aodOpCompleteMutex_);
        if (DmUtils::safe_wait_for(aodOpCompleteCV_, lock,
            std::chrono::milliseconds(SCREEN_STATE_AOD_OP_TIMEOUT_MS)) == std::cv_status::timeout) {
            TLOGW(WmsLogTag::DMS, "AOD operation timeout");
            isInAodOperation_ = false;
            return false;
        }
    }
    return true;
}
#endif

void ScreenSessionManager::DoAodExitAndSetPower(ScreenId screenId, ScreenPowerStatus status)
{
    TLOGI(WmsLogTag::DMS, "Enter, screenId: %{public}" PRIu64 ", status: %{public}d", screenId, status);
    int32_t screenStatus = (status == ScreenPowerStatus::POWER_STATUS_ON) ? AOD_POWER_ON : AOD_POWER_OFF;
    if (IsInAod()) {
        StopAod(screenStatus);
    }
    if (status == ScreenPowerStatus::POWER_STATUS_ON) {
        SetRSScreenPowerStatusExt(screenId, ScreenPowerStatus::POWER_STATUS_ON);
        if (!PowerMgr::PowerMgrClient::GetInstance().IsScreenOn()) {
            PowerMgr::PowerMgrClient::GetInstance().WakeupDeviceAsync();
        }
    } else if (status == ScreenPowerStatus::POWER_STATUS_OFF) {
        SetRSScreenPowerStatusExt(screenId, ScreenPowerStatus::POWER_STATUS_ON);
        SetRSScreenPowerStatusExt(screenId, ScreenPowerStatus::POWER_STATUS_OFF);
    }
}

void ScreenSessionManager::DoAodExitAndSetPowerAllOff()
{
    TLOGI(WmsLogTag::DMS, "Enter");
    if (IsInAod()) {
        StopAod(AOD_POWER_OFF);
    }
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_) {
        ScreenId screenId = foldScreenController_->GetCurrentScreenId();
        SetRSScreenPowerStatusExt(screenId, ScreenPowerStatus::POWER_STATUS_ON);
        SetRSScreenPowerStatusExt(screenId, ScreenPowerStatus::POWER_STATUS_OFF);
    }
#endif
}

void ScreenSessionManager::SetConfigForInputmethod(ScreenId screenId, VirtualScreenOption option)
{
    auto screenSession = GetScreenSession(screenId);
    TLOGD(WmsLogTag::DMS, "screenId:%{public}" PRIu64", supportsFocus:%{public}d, supportsInput:%{public}d",
        screenId, option.supportsFocus_, option.supportsInput_);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    screenSession->SetSupportsFocus(option.supportsFocus_);
    if (option.supportsFocus_) {
        screenSession->SetSupportsInput(option.supportsInput_);
    } else {
        screenSession->SetSupportsInput(false);
    }
}

void ScreenSessionManager::BootFinishedUnfreezeCallback(const char *key, const char *value, void *context)
{
    if (strcmp(key, BOOTEVENT_BOOT_COMPLETED.c_str()) == 0 && strcmp(value, "true") == 0) {
        TLOGI(WmsLogTag::DMS, "boot finished to unfreeze");
        auto &that = *reinterpret_cast<ScreenSessionManager *>(context);
        auto screenSession = that.GetDefaultScreenSession();
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
            return;
        }
        screenSession->PropertyChange(screenSession->GetScreenProperty(), ScreenPropertyChangeReason::CHANGE_MODE);
        screenSession->FreezeScreen(false);
        that.rsInterface_.ForceRefreshOneFrameWithNextVSync();
    }
}

void ScreenSessionManager::AddScreenUnfreezeTask(const sptr<ScreenSession>& screenSession, uint32_t freezeCount)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    // maximum boot time can reach 5 minutes, 300s = 150 * 2s
    if (freezeCount >= FREEZE_SCREEN_MAX_COUNT) {
        TLOGE(WmsLogTag::DMS, "boot error and freeze screen over 5 minutes");
        screenSession->FreezeScreen(false);
        rsInterface_.ForceRefreshOneFrameWithNextVSync();
        return;
    }
    if (system::GetBoolParameter(BOOTEVENT_BOOT_COMPLETED, false)) {
        auto unfreezeTask = [this, screenSession]() {
            screenSession->FreezeScreen(false);
            rsInterface_.ForceRefreshOneFrameWithNextVSync();
        };
        // delay 2000ms to wait app layout and send to display completed
        taskScheduler_->PostAsyncTask(unfreezeTask, "unfreezeTask", UNFREEZE_SCREEN_DELAY_MS);
        return;
    }
    freezeCount++;
    screenSession->FreezeScreen(true);
    auto task = [this, screenSession, freezeCount]() {
        AddScreenUnfreezeTask(screenSession, freezeCount);
    };
    // delay 2000ms to freeze again if not boot completed, avoid rs unfreeze after 3000ms
    taskScheduler_->PostAsyncTask(task, "AddScreenUnfreezeTask", FREEZE_SCREEN_RETRY_DELAY_MS);
}

DMError ScreenSessionManager::CheckSetResolutionIsValid(ScreenId screenId, uint32_t width, uint32_t height,
    float virtualPixelRatio)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "SetResolution permission denied! calling pid: %{public}d", IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "invalid screenId");
        return DMError::DM_ERROR_NULLPTR;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "Get ScreenSession failed");
        return DMError::DM_ERROR_NULLPTR;
    }
    sptr<SupportedScreenModes> screenSessionModes = screenSession->GetActiveScreenMode();
    if (screenSessionModes == nullptr) {
        TLOGE(WmsLogTag::DMS, "SetResolution: Get active screenMode failed");
        return DMError::DM_ERROR_NULLPTR;
    }
    if (width <= 0 || width > screenSessionModes->width_ ||
        height <= 0 || height > screenSessionModes->height_ ||
        virtualPixelRatio < (static_cast<float>(DOT_PER_INCH_MINIMUM_VALUE) / DOT_PER_INCH) ||
        virtualPixelRatio > (static_cast<float>(DOT_PER_INCH_MAXIMUM_VALUE) / DOT_PER_INCH)) {
        TLOGE(WmsLogTag::DMS, "invalid param! w:%{public}u h:%{public}u min:%{public}f max:%{public}f",
            screenSessionModes->width_, screenSessionModes->height_,
            static_cast<float>(DOT_PER_INCH_MINIMUM_VALUE) / DOT_PER_INCH,
            static_cast<float>(DOT_PER_INCH_MAXIMUM_VALUE) / DOT_PER_INCH);
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    return DMError::DM_OK;
}

bool ScreenSessionManager::GetScreenLcdStatus(ScreenId screenId, PanelPowerStatus& status)
{
    TLOGI(WmsLogTag::DMS, "Enter, screenId: %{public}" PRIu64, screenId);
    ScreenId rsScreenId = INVALID_SCREEN_ID;
    if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId");
        return false;
    }
    status = rsInterface_.GetPanelPowerStatus(rsScreenId);
    return true;
}

std::shared_ptr<TaskScheduler> ScreenSessionManager::GetScreenPowerTaskScheduler()
{
    return screenPowerTaskScheduler_;
}
// LCOV_EXCL_STOP
} // namespace OHOS::Rosen
