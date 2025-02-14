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

#include "dm_common.h"
#include "fold_screen_state_internel.h"
#include "multi_screen_manager.h"
#include "pipeline/rs_node_map.h"
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
#include "screen_sensor_plugin.h"
#ifdef FOLD_ABILITY_ENABLE
#include "fold_screen_controller/super_fold_sensor_manager.h"
#include "fold_screen_controller/super_fold_state_manager.h"
#include "fold_screen_controller/secondary_fold_sensor_manager.h"
#include "fold_screen_controller/super_fold_policy.h"
#endif

namespace OHOS::Rosen {
namespace {
const std::string SCREEN_SESSION_MANAGER_THREAD = "OS_ScreenSessionManager";
const std::string SCREEN_SESSION_MANAGER_SCREEN_POWER_THREAD = "OS_ScreenSessionManager_ScreenPower";
const std::string SCREEN_CAPTURE_PERMISSION = "ohos.permission.CAPTURE_SCREEN";
const std::string CUSTOM_SCREEN_CAPTURE_PERMISSION = "ohos.permission.CUSTOM_SCREEN_CAPTURE";
const std::string BOOTEVENT_BOOT_COMPLETED = "bootevent.boot.completed";
const int32_t CV_WAIT_SCREENON_MS = 300;
const int32_t CV_WAIT_SCREENOFF_MS = 1500;
const int32_t CV_WAIT_SCREENOFF_MS_MAX = 3000;
const int32_t CV_WAIT_SCBSWITCH_MS = 3000;
const int64_t SWITCH_USER_DISPLAYMODE_CHANGE_DELAY = 500;
const int32_t SCREEN_OFF_MIN_DELAY_TIME = 300;
const std::string STATUS_FOLD_HALF = "-z";
const std::string STATUS_EXPAND = "-y";
const std::string STATUS_FOLD = "-p";
const std::string SETTING_LOCKED_KEY = "settings.general.accelerometer_rotation_status";
const ScreenId SCREEN_ID_DEFAULT = 0;
const ScreenId SCREEN_ID_FULL = 0;
const ScreenId SCREEN_ID_MAIN = 5;
const ScreenId SCREEN_ID_PC_MAIN = 9;
const ScreenId MINIMUM_VIRTUAL_SCREEN_ID = 1000;
constexpr int32_t INVALID_UID = -1;
constexpr int32_t INVALID_USER_ID = -1;
constexpr int32_t INVALID_SCB_PID = -1;
constexpr int32_t BASE_USER_RANGE = 200000;
static bool g_foldScreenFlag = system::GetParameter("const.window.foldscreen.type", "") != "";
static const int32_t g_screenRotationOffSet = system::GetIntParameter<int32_t>("const.fold.screen_rotation.offset", 0);
static const int32_t g_outerOnly = system::GetIntParameter<int32_t>("persist.display.outeronly", 0);
static const int32_t ROTATION_90 = 1;
static const int32_t ONLY_OUTER_SCREEN_VALUE = 1;
#ifdef FOLD_ABILITY_ENABLE
static const int32_t ROTATION_270 = 3;
constexpr int32_t REMOVE_DISPLAY_MODE = 0;
#endif
static const int32_t AUTO_ROTATE_OFF = 0;
static const int NOTIFY_EVENT_FOR_DUAL_FAILED = 0;
static const int NOTIFY_EVENT_FOR_DUAL_SUCESS = 1;
static const int NO_NEED_NOTIFY_EVENT_FOR_DUAL = 2;
static bool g_isPcDevice = false;
static uint32_t g_internalWidth = 3120;
const unsigned int XCOLLIE_TIMEOUT_10S = 10;
constexpr int32_t CAST_WIRED_PROJECTION_START = 1005;
constexpr int32_t CAST_WIRED_PROJECTION_STOP = 1007;
constexpr int32_t RES_FAILURE_FOR_PRIVACY_WINDOW = -2;
constexpr int32_t IRREGULAR_REFRESH_RATE_SKIP_THRETHOLD = 10;
constexpr uint32_t SCREEN_MAIN_IN_DATA = 0;
constexpr uint32_t SCREEN_MIRROR_IN_DATA = 1;
constexpr uint32_t SCREEN_EXTEND_IN_DATA = 2;
constexpr uint32_t NUMBER_OF_PHYSICAL_SCREEN = 2;
constexpr bool ADD_VOTE = true;
constexpr bool REMOVE_VOTE = false;
constexpr uint32_t OLED_60_HZ = 60;
constexpr uint32_t FOUR_K_WIDTH = 3840;

const int32_t ROTATE_POLICY = system::GetIntParameter("const.window.device.rotate_policy", 0);
constexpr int32_t FOLDABLE_DEVICE { 2 };
constexpr float DEFAULT_PIVOT = 0.5f;
constexpr float DEFAULT_SCALE = 1.0f;
static const constexpr char* SET_SETTING_DPI_KEY {"default_display_dpi"};

const std::string SCREEN_EXTEND = "extend";
const std::string SCREEN_MIRROR = "mirror";
const std::string SCREEN_UNKNOWN = "unknown";
const std::string MULTI_SCREEN_EXIT_STR = "exit";
const std::string MULTI_SCREEN_ENTER_STR = "enter";

// based on the bundle_util
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
} // namespace

WM_IMPLEMENT_SINGLE_INSTANCE(ScreenSessionManager)

const bool REGISTER_RESULT = !SceneBoardJudgement::IsSceneBoardEnabled() ? false :
    SystemAbility::MakeAndRegisterAbility(&ScreenSessionManager::GetInstance());

ScreenSessionManager::ScreenSessionManager()
    : SystemAbility(DISPLAY_MANAGER_SERVICE_SA_ID, true), rsInterface_(RSInterfaces::GetInstance())
{
    screenEventTracker_.RecordEvent("Dms construct.");
    LoadScreenSceneXml();
    screenOffDelay_ = CV_WAIT_SCREENOFF_MS;
    screenOnDelay_ = CV_WAIT_SCREENON_MS;
    taskScheduler_ = std::make_shared<TaskScheduler>(SCREEN_SESSION_MANAGER_THREAD);
    screenPowerTaskScheduler_ = std::make_shared<TaskScheduler>(SCREEN_SESSION_MANAGER_SCREEN_POWER_THREAD);
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
    if (g_foldScreenFlag) {
        HandleFoldScreenPowerInit();
    }
    WatchParameter(BOOTEVENT_BOOT_COMPLETED.c_str(), BootFinishedCallback, this);
}

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

void ScreenSessionManager::HandleFoldScreenPowerInit()
{
#ifdef FOLD_ABILITY_ENABLE
    TLOGI(WmsLogTag::DMS, "Enter");
    foldScreenController_ = new (std::nothrow) FoldScreenController(displayInfoMutex_, screenPowerTaskScheduler_);
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
        ScreenId currentScreenId = foldScreenController_->GetCurrentScreenId();
        if (currentScreenId == SCREEN_ID_FULL) {
            TLOGI(WmsLogTag::DMS, "ScreenSessionManager Fold Screen Power Full animation Init 1.");
            rsInterface_.SetScreenPowerStatus(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF_FAKE);
            rsInterface_.SetScreenPowerStatus(SCREEN_ID_MAIN, ScreenPowerStatus::POWER_STATUS_ON);
            std::this_thread::sleep_for(std::chrono::milliseconds(timeStamp));
            TLOGI(WmsLogTag::DMS, "ScreenSessionManager Fold Screen Power Full animation Init 2.");
#ifdef TP_FEATURE_ENABLE
            rsInterface_.SetTpFeatureConfig(tpType, fullTpChange.c_str());
#endif
            rsInterface_.SetScreenPowerStatus(SCREEN_ID_MAIN, ScreenPowerStatus::POWER_STATUS_OFF);
            foldScreenController_->AddOrRemoveDisplayNodeToTree(SCREEN_ID_MAIN, REMOVE_DISPLAY_MODE);
            rsInterface_.SetScreenPowerStatus(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_ON);
        } else if (currentScreenId == SCREEN_ID_MAIN) {
            TLOGI(WmsLogTag::DMS, "ScreenSessionManager Fold Screen Power Main animation Init 3.");
            rsInterface_.SetScreenPowerStatus(SCREEN_ID_MAIN, ScreenPowerStatus::POWER_STATUS_OFF_FAKE);
            rsInterface_.SetScreenPowerStatus(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_ON);
            std::this_thread::sleep_for(std::chrono::milliseconds(timeStamp));
            TLOGI(WmsLogTag::DMS, "ScreenSessionManager Fold Screen Power Main animation Init 4.");
#ifdef TP_FEATURE_ENABLE
            rsInterface_.SetTpFeatureConfig(tpType, mainTpChange.c_str());
#endif
            rsInterface_.SetScreenPowerStatus(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF);
            foldScreenController_->AddOrRemoveDisplayNodeToTree(SCREEN_ID_FULL, REMOVE_DISPLAY_MODE);
            rsInterface_.SetScreenPowerStatus(SCREEN_ID_MAIN, ScreenPowerStatus::POWER_STATUS_ON);
        } else {
            TLOGI(WmsLogTag::DMS, "ScreenSessionManager Fold Screen Power Init, invalid active screen id");
        }
        FixPowerStatus();
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
    if (system::GetParameter("soc.boot.mode", "") != "rescue") {
        constexpr uint64_t interval = 5 * 1000; // 5 second
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

    if (ScreenSceneConfig::GetExternalScreenDefaultMode() == "none") {
        g_isPcDevice = true;
    }

    if (!LoadMotionSensor()) {
        screenEventTracker_.RecordEvent("Dms load motion plugin failed.");
        TLOGW(WmsLogTag::DMS, "load motion plugin failed.");
    }

    RegisterScreenChangeListener();
    if (!ScreenSceneConfig::IsSupportRotateWithSensor()) {
        TLOGI(WmsLogTag::DMS, "Current type not support SetSensorSubscriptionEnabled.");
    } else if (GetScreenPower(SCREEN_ID_FULL) == ScreenPowerState::POWER_ON) {
        // 多屏设备只要有屏幕亮,GetScreenPower获取的任意一块屏幕状态均是ON
        SetSensorSubscriptionEnabled();
        screenEventTracker_.RecordEvent("Dms subscribed to sensor successfully.");
    }

    // publish init
    ScreenSessionPublish::GetInstance().InitPublishEvents();
    screenEventTracker_.RecordEvent("Dms init end.");
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
    screenEventTracker_.RecordEvent("Dms AddSystemAbilityListener finished.");
    TLOGI(WmsLogTag::DMS, "end");
    screenEventTracker_.RecordEvent("Dms onstart end.");
}

void ScreenSessionManager::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "OnAddSystemAbility: %d", systemAbilityId);
    TLOGI(WmsLogTag::DMS, "receive sa add:%{public}d", systemAbilityId);
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
            if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
                SecondaryFoldSensorManager::GetInstance().RegisterPostureCallback();
            } else {
                FoldScreenSensorManager::GetInstance().RegisterPostureCallback();
            }
            TLOGI(WmsLogTag::DMS, "Recover Posture sensor finished");
        }

        if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
            SecondaryFoldSensorManager::GetInstance().RegisterHallCallback();
        } else {
            FoldScreenSensorManager::GetInstance().RegisterHallCallback();
        }
        TLOGI(WmsLogTag::DMS, "Recover Hall sensor finished");
        screenEventTracker_.RecordEvent("Dms recover Posture and Hall sensor finished.");
#endif
    }
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
    return dmAgentContainer_.RegisterAgent(displayManagerAgent, type) ? DMError::DM_OK :DMError::DM_ERROR_NULLPTR;
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
    return dmAgentContainer_.UnregisterAgent(displayManagerAgent, type) ? DMError::DM_OK :DMError::DM_ERROR_NULLPTR;
}

void ScreenSessionManager::LoadScreenSceneXml()
{
    if (ScreenSceneConfig::LoadConfigXml()) {
        ScreenSceneConfig::DumpConfig();
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
    allDisplayPhysicalResolution_ = ScreenSceneConfig::GetAllDisplayPhysicalConfig();
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

void ScreenSessionManager::RegisterScreenChangeListener()
{
    TLOGI(WmsLogTag::DMS, "start");
    auto res = rsInterface_.SetScreenChangeCallback(
        [this](ScreenId screenId, ScreenEvent screenEvent, ScreenChangeReason reason) {
            OnScreenChange(screenId, screenEvent, reason);
        });
    if (res != StatusCode::SUCCESS) {
        auto task = [this]() { RegisterScreenChangeListener(); };
        taskScheduler_->PostAsyncTask(task, "RegisterScreenChangeListener", 50);  // Retry after 50 ms.
        screenEventTracker_.RecordEvent("Dms OnScreenChange register failed.");
    } else {
        screenEventTracker_.RecordEvent("Dms OnScreenChange register success.");
    }
}

void ScreenSessionManager::RegisterRefreshRateChangeListener()
{
    static bool isRegisterRefreshRateListener = false;
    if (!isRegisterRefreshRateListener) {
        TLOGW(WmsLogTag::DMS, "call rsInterface_ RegisterHgmRefreshRateUpdateCallback");
        auto res = rsInterface_.RegisterHgmRefreshRateUpdateCallback(
            [this](uint32_t refreshRate) { OnHgmRefreshRateChange(refreshRate); });
        TLOGW(WmsLogTag::DMS, "call rsInterface_ RegisterHgmRefreshRateUpdateCallback end");
        if (res != StatusCode::SUCCESS) {
            TLOGE(WmsLogTag::DMS, "failed");
            screenEventTracker_.RecordEvent("Dms RefreshRateChange register failed.");
        } else {
            isRegisterRefreshRateListener = true;
            screenEventTracker_.RecordEvent("Dms RefreshRateChange register success.");
        }
    }
}

void ScreenSessionManager::OnVirtualScreenChange(ScreenId screenId, ScreenEvent screenEvent)
{
    TLOGI(WmsLogTag::DMS, "Notify scb virtual screen change, ScreenId: %{public}" PRIu64 ", ScreenEvent: %{public}d",
        screenId, static_cast<int>(screenEvent));
    auto screenSession = GetScreenSession(screenId);
    if (!screenSession) {
        TLOGE(WmsLogTag::DMS, "screenSession is nullptr");
        return;
    }
    if (screenEvent == ScreenEvent::CONNECTED) {
        if (clientProxy_) {
            clientProxy_->OnScreenConnectionChanged(screenId, ScreenEvent::CONNECTED,
                screenSession->GetRSScreenId(), screenSession->GetName(), screenSession->GetIsExtend());
        }
        return;
    }
    if (screenEvent == ScreenEvent::DISCONNECTED) {
        if (clientProxy_) {
            clientProxy_->OnScreenConnectionChanged(screenId, ScreenEvent::DISCONNECTED,
                screenSession->GetRSScreenId(), screenSession->GetName(), screenSession->GetIsExtend());
        }
    }
}

bool ScreenSessionManager::IsDefaultMirrorMode(ScreenId screenId)
{
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
        if (!g_isPcDevice) {
            hdmiScreenCount_ = hdmiScreenCount_ > 0 ? hdmiScreenCount_ - 1 : 0;
            NotifyCaptureStatusChanged();
        }
        displayNode->RemoveFromTree();
        mirrorSession->ReleaseDisplayNode();
        displayNode = nullptr;
    }
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        TLOGI(WmsLogTag::DMS, "free displayNode");
        transactionProxy->FlushImplicitTransaction();
    }
}

void ScreenSessionManager::SetScreenCorrection()
{
    std::ostringstream oss;
    if (g_foldScreenFlag) {
        if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
            auto ret = rsInterface_.SetScreenCorrection(SCREEN_ID_MAIN,
                static_cast<ScreenRotation>(ROTATION_90));
            oss << "main screenRotationOffSet: " << g_screenRotationOffSet << " ret value: " << ret;
        } else {
            auto ret = rsInterface_.SetScreenCorrection(SCREEN_ID_FULL,
                static_cast<ScreenRotation>(g_screenRotationOffSet));
            oss << "full screenRotationOffSet: " << g_screenRotationOffSet << " ret value: " << ret;
        }
    } else {
        std::vector<std::string> phyOffsets = FoldScreenStateInternel::GetPhyRotationOffset();
        int32_t phyOffset = static_cast<int32_t>(std::stoi(phyOffsets[0]));
        ScreenRotation correctRotation = ConvertOffsetToCorrectRotation(phyOffset);
        auto ret = rsInterface_.SetScreenCorrection(SCREEN_ID_DEFAULT, correctRotation);
        oss << "phyOffset: " << phyOffset << " correctRotation value: " <<
            static_cast<int32_t>(correctRotation) << " ret value: " << ret;
    }
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    screenEventTracker_.RecordEvent(oss.str());
}

void ScreenSessionManager::OnScreenChange(ScreenId screenId, ScreenEvent screenEvent, ScreenChangeReason reason)
{
    if (reason == ScreenChangeReason::HWCDEAD) {
        NotifyAbnormalScreenConnectChange(screenId);
        TLOGW(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " ScreenChangeReason: %{public}d",
            screenId, static_cast<int>(reason));
        return;
    }
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
#ifdef FOLD_ABILITY_ENABLE
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
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
        screenSession->SetFoldScreen(true);
    }
#endif
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

void ScreenSessionManager::NotifyScreenModeChange(ScreenId disconnectedScreenId)
{
    auto task = [=] {
        auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_MODE_CHANGE_EVENT_LISTENER);
        if (agents.empty()) {
            return;
        }
        std::vector<sptr<ScreenInfo>> screenInfos;
        std::vector<ScreenId> screenIds = GetAllScreenIds();
        for (auto screenId : screenIds) {
            if (disconnectedScreenId == screenId) {
                continue;
            }
            auto screenSession = GetScreenSession(screenId);
            screenInfos.emplace_back(screenSession->ConvertToScreenInfo());
        }
        for (auto& agent : agents) {
            int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
            if (!IsFreezed(agentPid, DisplayManagerAgentType::SCREEN_MODE_CHANGE_EVENT_LISTENER)) {
                agent->NotifyScreenModeChange(screenInfos);
            }
        }
    };
    taskScheduler_->PostAsyncTask(task, "NotifyScreenModeChange");
}

void ScreenSessionManager::NotifyAbnormalScreenConnectChange(ScreenId screenId)
{
    auto agents = dmAgentContainer_.GetAgentsByType(
        DisplayManagerAgentType::ABNORMAL_SCREEN_CONNECT_CHANGE_LISTENER);
    if (agents.empty()) {
        TLOGE(WmsLogTag::DMS, "agents is empty");
        return;
    }
    for (auto& agent : agents) {
        agent->NotifyAbnormalScreenConnectChange(screenId);
    }
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
    if (ScreenSceneConfig::GetExternalScreenDefaultMode() == "mirror") {
        TLOGI(WmsLogTag::DMS, "Connect to an external screen to wakeup the phone screen");
        FixPowerStatus();
    }
}

bool ScreenSessionManager::IsScreenRestored(sptr<ScreenSession> screenSession)
{
    const ScreenProperty& screenProperty = screenSession->GetScreenProperty();
    ScreenId currentScreenId = screenSession->GetScreenId();
    TLOGI(WmsLogTag::DMS, "currentScreenId: %{public}" PRIu64, currentScreenId);
    std::set<ScreenId> restoredScreenId;
    bool getRestoredScreenId = ScreenSettingHelper::GetSettingRecoveryResolutionSet(restoredScreenId);
    if (!getRestoredScreenId) {
        TLOGE(WmsLogTag::DMS, "get restored screenId failed");
        return false;
    }
    if (restoredScreenId.find(currentScreenId) == restoredScreenId.end()) {
        TLOGE(WmsLogTag::DMS, "screenId not exist");
        return false;
    }
    return true;
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

bool ScreenSessionManager::GetMultiScreenInfo(MultiScreenInfo &info)
{
    std::map<ScreenId, uint32_t> screenMode;
    bool getMode = ScreenSettingHelper::GetSettingScreenModeMap(screenMode);
    if (!getMode) {
        TLOGE(WmsLogTag::DMS, "get last screen mode failed");
        return false;
    }
    std::map<ScreenId, std::pair<uint32_t, uint32_t>> position;
    bool getPosition = ScreenSettingHelper::GetSettingRelativePositionMap(position);
    if (!getPosition) {
        TLOGE(WmsLogTag::DMS, "get relative position failed");
        return false;
    }
    bool getMain = false;
    bool getSecondary = false;
    for (auto& [screenId, mode] : screenMode) {
        if (!GetIsCurrentInUseById(screenId)) {
            TLOGE(WmsLogTag::DMS, "session not currently in use, screenId: %{public}" PRIu64 "", screenId);
            continue;
        }
        if (mode == SCREEN_MAIN_IN_DATA) {
            if (position.find(screenId) == position.end()) {
                TLOGE(WmsLogTag::DMS, "position not found, screenId: %{public}" PRIu64 "", screenId);
                continue;
            }
            info.mainScreenOption = {screenId, position[screenId].first, position[screenId].second};
            getMain = true;
        } else {
            if (mode == SCREEN_MIRROR_IN_DATA) {
                info.multiScreenMode = MultiScreenMode::SCREEN_MIRROR;
            } else if (mode == SCREEN_EXTEND_IN_DATA) {
                info.multiScreenMode = MultiScreenMode::SCREEN_EXTEND;
            } else {
                TLOGE(WmsLogTag::DMS, "screen mode error!");
                continue;
            }
            if (position.find(screenId) == position.end()) {
                TLOGE(WmsLogTag::DMS, "position not found, screenId: %{public}" PRIu64 "", screenId);
                continue;
            }
            info.secondaryScreenOption = {screenId, position[screenId].first, position[screenId].second};
            getSecondary = true;
        }
    }
    if (!getMain || !getSecondary) {
        TLOGE(WmsLogTag::DMS, "screen info error!");
        return false;
    }
    return true;
}

void ScreenSessionManager::SetMultiScreenDefaultRelativePosition()
{
    MultiScreenPositionOptions mainOptions;
    MultiScreenPositionOptions extendOptions;
    sptr<ScreenSession> mainSession = nullptr;
    sptr<ScreenSession> extendSession = nullptr;
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
    if (extendSession != nullptr && mainSession != nullptr) {
        ScreenProperty mainProperty = mainSession->GetScreenProperty();
        int32_t mainWidth = mainProperty.GetBounds().rect_.GetWidth();
        mainOptions = { mainSession->GetScreenId(), 0, 0 };
        extendOptions = { extendSession->GetScreenId(), mainWidth, 0 };
        auto ret = SetMultiScreenRelativePosition(mainOptions, extendOptions);
        if (ret != DMError::DM_OK) {
            TLOGE(WmsLogTag::DMS, "set Relative Position failed, DMError:%{public}d", static_cast<int32_t>(ret));
        }
    }
}

bool ScreenSessionManager::CheckMultiScreenInfo(MultiScreenInfo &info, sptr<ScreenSession> screenSession)
{
    bool isRestored = IsScreenRestored(screenSession);
    if (!isRestored) {
        TLOGE(WmsLogTag::DMS, "no restored screen found");
        ReportHandleScreenEvent(ScreenEvent::CONNECTED, ScreenCombination::SCREEN_EXTEND);
        SetMultiScreenDefaultRelativePosition();
        return false;
    }
    bool getInfo = GetMultiScreenInfo(info);
    if (!getInfo) {
        TLOGE(WmsLogTag::DMS, "get option failed");
        return false;
    }
    if (info.mainScreenOption.screenId_ == info.secondaryScreenOption.screenId_) {
        TLOGE(WmsLogTag::DMS, "screenId error");
        return false;
    }
    if (info.mainScreenOption.screenId_ != screenSession->GetScreenId() &&
        info.secondaryScreenOption.screenId_ != screenSession->GetScreenId()) {
        TLOGE(WmsLogTag::DMS, "no current screenId");
        return false;
    }
    TLOGW(WmsLogTag::DMS, "read param success");
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

void ScreenSessionManager::HandleScreenConnectEvent(sptr<ScreenSession> screenSession,
    ScreenId screenId, ScreenEvent screenEvent)
{
    bool phyMirrorEnable = IsDefaultMirrorMode(screenId);
    if (phyMirrorEnable) {
        PhyMirrorConnectWakeupScreen();
        NotifyCastWhenScreenConnectChange(true);
    }
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ != nullptr) {
        if ((screenId == 0 || (screenId == SCREEN_ID_MAIN && isCoordinationFlag_ == true)) &&
            clientProxy_) {
            TLOGW(WmsLogTag::DMS, "event: connect %{public}" PRIu64 ", %{public}" PRIu64 ", "
                "name=%{public}s", screenId, screenSession->GetRSScreenId(), screenSession->GetName().c_str());
            clientProxy_->OnScreenConnectionChanged(screenId, screenEvent,
                screenSession->GetRSScreenId(), screenSession->GetName(), screenSession->GetIsExtend());
        }
        return;
    }
#endif
    if (clientProxy_ && (!phyMirrorEnable || g_isPcDevice)) {
        if (g_outerOnly == ONLY_OUTER_SCREEN_VALUE) {
            if (screenId != SCREEN_ID_FULL) {
                defaultScreenId_ = screenId;
                TLOGW(WmsLogTag::DMS, "event screen %{public}" PRIu64, screenId);
                clientProxy_->OnScreenConnectionChanged(screenId, screenEvent,
                    screenSession->GetRSScreenId(), screenSession->GetName(), screenSession->GetIsExtend());
                CallRsSetScreenPowerStatusSync(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF);
            }
        }
    }
    if (clientProxy_ && !g_isPcDevice && !phyMirrorEnable) {
        TLOGW(WmsLogTag::DMS, "screen connect and notify to scb.");
        clientProxy_->OnScreenConnectionChanged(screenId, screenEvent,
            screenSession->GetRSScreenId(), screenSession->GetName(), screenSession->GetIsExtend());
    }
    if (clientProxy_ && g_isPcDevice) {
        RecoverRestoredMultiScreenMode(screenSession);
        SetExtendedScreenFallbackPlan(screenId);
    }
    if (phyMirrorEnable) {
        NotifyScreenConnected(screenSession->ConvertToScreenInfo());
        NotifyDisplayCreate(screenSession->ConvertToDisplayInfo());
        isPhyScreenConnected_ = true;
    }
    TLOGW(WmsLogTag::DMS, "connect end. ScreenId: %{public}" PRIu64, screenId);
}

void ScreenSessionManager::HandleScreenDisconnectEvent(sptr<ScreenSession> screenSession,
    ScreenId screenId, ScreenEvent screenEvent)
{
    bool phyMirrorEnable = IsDefaultMirrorMode(screenId);
    if (phyMirrorEnable) {
        NotifyDisplayDestroy(screenSession->GetScreenId());
        NotifyCastWhenScreenConnectChange(false);
        FreeDisplayMirrorNodeInner(screenSession);
    }
    ScreenCombination screenCombination = screenSession->GetScreenCombination();
    if (g_isPcDevice) {
        if (screenCombination == ScreenCombination::SCREEN_MAIN ||
            screenCombination == ScreenCombination::SCREEN_EXTEND) {
            TLOGW(WmsLogTag::DMS, "need to change screen");
            ScreenId internalScreenId = GetInternalScreenId();
            sptr<ScreenSession> internalSession = GetScreenSession(internalScreenId);

            if (screenCombination == ScreenCombination::SCREEN_EXTEND) {
                MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_EXTEND, MULTI_SCREEN_EXIT_STR);
            }
        } else {
            MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_MIRROR, MULTI_SCREEN_EXIT_STR);
            NotifyCaptureStatusChanged(false);
        }
        MultiScreenPositionOptions defaultOptions = { GetDefaultScreenId(), 0, 0 };
        SetRelativePositionForDisconnect(defaultOptions);
        FreeDisplayMirrorNodeInner(screenSession);
    }
    if (clientProxy_) {
        TLOGW(WmsLogTag::DMS, "screen disconnect and notify to scb.");
        clientProxy_->OnScreenConnectionChanged(screenId, ScreenEvent::DISCONNECTED,
            screenSession->GetRSScreenId(), screenSession->GetName(), screenSession->GetIsExtend());
    }
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        screenSessionMap_.erase(screenId);
    }
    if (g_isPcDevice) {
        ReportHandleScreenEvent(ScreenEvent::DISCONNECTED, screenCombination);
        SetMultiScreenFrameControl();
    }
    if (!(screenId == SCREEN_ID_MAIN && isCoordinationFlag_ == true)) {
        std::lock_guard<std::recursive_mutex> lock_phy(phyScreenPropMapMutex_);
        phyScreenPropMap_.erase(screenId);
    }
    if (phyMirrorEnable) {
        NotifyScreenDisconnected(screenSession->GetScreenId());
        isPhyScreenConnected_ = false;
    }
    TLOGW(WmsLogTag::DMS, "disconnect success. ScreenId: %{public}" PRIu64 "", screenId);
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
    } else {
        TLOGE(WmsLogTag::DMS, "Get default screen session failed.");
    }
    return;
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
}

sptr<ScreenSession> ScreenSessionManager::GetScreenSession(ScreenId screenId) const
{
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    if (screenSessionMap_.empty()) {
        screenEventTracker_.LogWarningAllInfos();
    }
    auto iter = screenSessionMap_.find(screenId);
    if (iter == screenSessionMap_.end()) {
        TLOGW(WmsLogTag::DMS, "not found screen session id: %{public}" PRIu64" and create new.", screenId);
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
    if (displayHookMap_.find(uid) != displayHookMap_.end()) {
        auto info = displayHookMap_[uid];
        TLOGI(WmsLogTag::DMS, "hW: %{public}u, hH: %{public}u, hD: %{public}f, hR: %{public}u, hER: %{public}d, "
            "dW: %{public}u, dH: %{public}u, dR: %{public}u, dO: %{public}u", info.width_, info.height_, info.density_,
            info.rotation_, info.enableHookRotation_, displayInfo->GetWidth(), displayInfo->GetHeight(),
            displayInfo->GetRotation(), displayInfo->GetDisplayOrientation());

        displayInfo->SetWidth(info.width_);
        displayInfo->SetHeight(info.height_);
        displayInfo->SetVirtualPixelRatio(info.density_);
        if (info.enableHookRotation_) {
            if (screenSession) {
                Rotation targetRotation = screenSession->ConvertIntToRotation(static_cast<int32_t>(info.rotation_));
                displayInfo->SetRotation(targetRotation);
                DisplayOrientation targetOrientation = screenSession->CalcDisplayOrientation(targetRotation,
                    FoldDisplayMode::UNKNOWN, IsOrientationNeedChanged());
                TLOGI(WmsLogTag::DMS, "tR: %{public}u, tO: %{public}u", targetRotation, targetOrientation);
                displayInfo->SetDisplayOrientation(targetOrientation);
            } else {
                TLOGI(WmsLogTag::DMS, "ConvertToDisplayInfo error, screenSession is nullptr.");
                return nullptr;
            }
        }
    }
    return displayInfo;
}

sptr<DisplayInfo> ScreenSessionManager::GetDefaultDisplayInfo()
{
    DmsXcollie dmsXcollie("DMS:GetDefaultDisplayInfo", XCOLLIE_TIMEOUT_10S);
    GetDefaultScreenId();
    sptr<ScreenSession> screenSession = GetScreenSession(defaultScreenId_);
    std::lock_guard<std::recursive_mutex> lock_info(displayInfoMutex_);
    if (screenSession) {
        sptr<DisplayInfo> displayInfo = screenSession->ConvertToDisplayInfo();
        if (displayInfo == nullptr) {
            TLOGI(WmsLogTag::DMS, "ConvertToDisplayInfo error, displayInfo is nullptr.");
            return nullptr;
        }
        // 在PC/PAD上安装的竖屏应用以及白名单中的应用在显示状态非全屏时需要hook displayinfo
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
    DmsXcollie dmsXcollie("DMS:GetDisplayInfoById", XCOLLIE_TIMEOUT_10S);
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
        DisplayId fakeDisplayId = fakeDisplayInfo->GetDisplayId();
        if (displayId == fakeDisplayId) {
            TLOGD(WmsLogTag::DMS, "find fake success");
            return fakeDisplayInfo;
        }
    }
    TLOGE(WmsLogTag::DMS, "GetDisplayInfoById failed. displayId: %{public}" PRIu64" ", displayId);
    return nullptr;
}

sptr<DisplayInfo> ScreenSessionManager::GetVisibleAreaDisplayInfoById(DisplayId displayId)
{
    TLOGD(WmsLogTag::DMS, "enter, displayId: %{public}" PRIu64" ", displayId);
    DmsXcollie dmsXcollie("DMS:GetVisibleAreaDisplayInfoById", XCOLLIE_TIMEOUT_10S);
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
        displayInfo = HookDisplayInfoByUid(displayInfo, screenSession);
        if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
            return displayInfo;
        }
        SuperFoldStatus status = SuperFoldStateManager::GetInstance().GetCurrentStatus();
        RRect bounds = screenSession->GetScreenProperty().GetBounds();
        auto screenWdith = bounds.rect_.GetWidth();
        auto screenHeight = bounds.rect_.GetHeight();
        if (status == SuperFoldStatus::KEYBOARD) {
            if (screenWdith > screenHeight) {
                auto temp = screenWdith;
                screenWdith = screenHeight;
                screenHeight = temp;
            }
            displayInfo->SetWidth(screenWdith);
            displayInfo->SetHeight(screenHeight / HALF_SCREEN_PARAM);
        } else {
            displayInfo->SetWidth(screenWdith);
            displayInfo->SetHeight(screenHeight);
        }
        return displayInfo;
    }
    TLOGE(WmsLogTag::DMS, "GetVisibleAreaDisplayInfoById failed. displayId: %{public}" PRIu64" ", displayId);
    return nullptr;
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

std::vector<DisplayId> ScreenSessionManager::GetAllDisplayIds()
{
    TLOGI(WmsLogTag::DMS, "enter");
    std::vector<DisplayId> res;
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr, ScreenId:%{public}" PRIu64"",
                sessionIt.first);
            continue;
        }
        sptr<DisplayInfo> displayInfo = screenSession->ConvertToDisplayInfo();
        if (displayInfo == nullptr) {
            TLOGE(WmsLogTag::DMS, "error, displayInfo is nullptr.");
            continue;
        }
        DisplayId displayId = displayInfo->GetDisplayId();
        res.push_back(displayId);
        if (!FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
            continue;
        }
        if (!screenSession->GetScreenProperty().GetIsFakeInUse()) {
            continue;
        }
        DisplayId fakeDisplayId = GetFakeDisplayId(screenSession);
        if (fakeDisplayId == DISPLAY_ID_FAKE) {
            res.push_back(fakeDisplayId);
            TLOGI(WmsLogTag::DMS, "add fakeDisplayId: %{public}" PRIu64 "", fakeDisplayId);
        }
    }
    return res;
}

sptr<ScreenInfo> ScreenSessionManager::GetScreenInfoById(ScreenId screenId)
{
    DmsXcollie dmsXcollie("DMS:GetScreenInfoById", XCOLLIE_TIMEOUT_10S);
    if (!SessionPermission::IsSystemCalling()) {
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

DMError ScreenSessionManager::SetScreenActiveMode(ScreenId screenId, uint32_t modeId)
{
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
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "Get ScreenSession failed");
        return DMError::DM_ERROR_NULLPTR;
    }
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    if (!screenIdManager_.ConvertToRsScreenId(screenId, rsScreenId)) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsId");
        return DMError::DM_ERROR_NULLPTR;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetScreenActiveMode(%" PRIu64", %u)", screenId, modeId);
    rsInterface_.SetScreenActiveMode(rsScreenId, modeId);
    screenSession->activeIdx_ = static_cast<int32_t>(modeId);
    screenSession->UpdatePropertyByActiveMode();
    ScreenProperty property = screenSession->GetScreenProperty();
    property.SetPropertyChangeReason("active mode change");
    screenSession->PropertyChange(property, ScreenPropertyChangeReason::CHANGE_MODE);
    NotifyScreenChanged(screenSession->ConvertToScreenInfo(), ScreenChangeEvent::CHANGE_MODE);
    NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(), DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    return DMError::DM_OK;
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

    std::unique_lock<std::shared_mutex> lock(hookInfoMutex_);
    if (enable) {
        if (uid != 0) {
            displayHookMap_[uid] = hookInfo;
        }
    } else {
        displayHookMap_.erase(uid);
    }
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
        auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
        TLOGNI(WmsLogTag::DMS, "screenId:%{public}" PRIu64", agent size: %{public}u",
            screenInfo->GetScreenId(), static_cast<uint32_t>(agents.size()));
        if (agents.empty()) {
            return;
        }
        for (auto& agent : agents) {
            int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
            if (!IsFreezed(agentPid, DisplayManagerAgentType::SCREEN_EVENT_LISTENER)) {
                agent->OnScreenChange(screenInfo, event);
            }
        }
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
    // less to 1e-6 mean equal
    if (fabs(screenSession->GetScreenProperty().GetVirtualPixelRatio() - virtualPixelRatio) < 1e-6) {
        TLOGE(WmsLogTag::DMS,
            "The density is equivalent to the original value, no update operation is required, aborted.");
        return DMError::DM_OK;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetVirtualPixelRatio(%" PRIu64", %f)", screenId,
        virtualPixelRatio);
    screenSession->SetVirtualPixelRatio(virtualPixelRatio);
    std::map<DisplayId, sptr<DisplayInfo>> emptyMap;
    OnPropertyChange(screenSession->GetScreenProperty(), ScreenPropertyChangeReason::VIRTUAL_PIXEL_RATIO_CHANGE,
        screenId);
    NotifyDisplayStateChange(screenId, screenSession->ConvertToDisplayInfo(),
        emptyMap, DisplayStateChangeType::VIRTUAL_PIXEL_RATIO_CHANGE);
    NotifyScreenChanged(screenSession->ConvertToScreenInfo(), ScreenChangeEvent::VIRTUAL_PIXEL_RATIO_CHANGED);
    NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(),
        DisplayChangeEvent::DISPLAY_VIRTUAL_PIXEL_RATIO_CHANGED);
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetVirtualPixelRatioSystem(ScreenId screenId, float virtualPixelRatio)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }

    if (clientProxy_) {
        clientProxy_->SetVirtualPixelRatioSystem(screenId, virtualPixelRatio);
    }
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetResolution(ScreenId screenId, uint32_t width, uint32_t height, float virtualPixelRatio)
{
    TLOGI(WmsLogTag::DMS,
        "ScreenId: %{public}" PRIu64 ", w: %{public}u, h: %{public}u, virtualPixelRatio: %{public}f",
        screenId, width, height, virtualPixelRatio);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
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
    screenSession->SetDensityInCurResolution(virtualPixelRatio);
    DMError ret = SetVirtualPixelRatio(screenId, virtualPixelRatio);
    if (ret != DMError::DM_OK) {
        TLOGE(WmsLogTag::DMS, "Failed to setVirtualPixelRatio when settingResolution");
        screenSession->SetDensityInCurResolution(screenSession->GetScreenProperty().GetVirtualPixelRatio());
        return ret;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetResolution(%" PRIu64", %u, %u, %f)",
        screenId, width, height, virtualPixelRatio);
    screenSession->UpdatePropertyByResolution(width, height);
    screenSession->PropertyChange(screenSession->GetScreenProperty(), ScreenPropertyChangeReason::CHANGE_MODE);
    NotifyScreenChanged(screenSession->ConvertToScreenInfo(), ScreenChangeEvent::CHANGE_MODE);
    NotifyDisplayChanged(screenSession->ConvertToDisplayInfo(), DisplayChangeEvent::DISPLAY_SIZE_CHANGED);
    return DMError::DM_OK;
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
}

DMError ScreenSessionManager::SetScreenColorGamut(ScreenId screenId, int32_t colorGamutIdx)
{
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
}

DMError ScreenSessionManager::GetScreenGamutMap(ScreenId screenId, ScreenGamutMap& gamutMap)
{
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
}

DMError ScreenSessionManager::SetScreenGamutMap(ScreenId screenId, ScreenGamutMap gamutMap)
{
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
    if (g_isPcDevice) {
        if (g_outerOnly == ONLY_OUTER_SCREEN_VALUE) {
            config = {
                .screenId = screenId,
                .rsId = screenId,
                .property = property,
            };
        } else {
            config = {
                .screenId = screenId,
                .rsId = screenId,
                .defaultScreenId = defScreenId,
                .property = property,
            };
        }
        screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_REAL);
    } else {
        sptr<ScreenSession> defaultScreen = GetDefaultScreenSession();
        if (defaultScreen == nullptr || defaultScreen->GetDisplayNode() == nullptr) {
            TLOGE(WmsLogTag::DMS, "default screen null");
            return screenSession;
        }
        NodeId nodeId = defaultScreen->GetDisplayNode()->GetId();
        TLOGI(WmsLogTag::DMS, "physical mirror screen nodeId: %{public}" PRIu64, nodeId);
        config = {
            .screenId = screenId,
            .defaultScreenId = defScreenId,
            .mirrorNodeId = nodeId,
            .property = property,
        };
        screenSession = new ScreenSession(config, ScreenSessionReason::CREATE_SESSION_FOR_MIRROR);
        screenSession->SetIsPhysicalMirrorSwitch(true);
        if (foldScreenController_ != nullptr && FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
            DMRect mainScreenRegion = {0, 0, 0, 0};
            foldScreenController_->SetMainScreenRegion(mainScreenRegion);
            screenSession->SetMirrorScreenRegion(screenId, mainScreenRegion);
            screenSession->EnableMirrorScreenRegion();
        }
    }
    return screenSession;
}

sptr<ScreenSession> ScreenSessionManager::CreatePhysicalMirrorSessionInner(ScreenId screenId, ScreenId defScreenId,
    ScreenProperty property)
{
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
    if (g_isPcDevice) {
        // pc is none, pad&&phone is mirror
        InitExtendScreenProperty(screenId, screenSession, property);
        screenSession->SetName("ExtendedDisplay");
        if (g_outerOnly == ONLY_OUTER_SCREEN_VALUE) {
            screenSession->SetIsExtend(false);
            screenSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
        } else {
            screenSession->SetIsExtend(true);
            screenSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
        }
    } else {
        screenSession->SetIsExtend(true);
        screenSession->SetName("CastEngine");
        screenSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    }
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
}

sptr<ScreenSession> ScreenSessionManager::GetScreenSessionInner(ScreenId screenId, ScreenProperty property)
{
    ScreenId defScreenId = GetDefaultScreenId();
    TLOGW(WmsLogTag::DMS, "screenId:%{public}" PRIu64 "", screenId);
    if (IsDefaultMirrorMode(screenId)) {
        return CreatePhysicalMirrorSessionInner(screenId, defScreenId, property);
    }
    std::string screenName = "UNKNOWN";
    if (screenId == SCREEN_ID_MAIN) {
        screenName = "SubScreen";
    }
    ScreenSessionConfig config = {
        .screenId = screenId,
        .defaultScreenId = defScreenId,
        .name = screenName,
        .property = property,
    };
    sptr<ScreenSession> screenSession = nullptr;
    if (g_isPcDevice && g_outerOnly == ONLY_OUTER_SCREEN_VALUE) {
        return screenSession;
    }
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

void ScreenSessionManager::CreateScreenProperty(ScreenId screenId, ScreenProperty& property)
{
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("CreateScreenPropertyCallRS", XCOLLIE_TIMEOUT_10S, nullptr,
        nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
    TLOGW(WmsLogTag::DMS, "Call rsInterface_ GetScreenActiveMode ScreenId: %{public}" PRIu64, screenId);
    auto screenMode = rsInterface_.GetScreenActiveMode(screenId);
    TLOGW(WmsLogTag::DMS, "get screenWidth: %{public}d, screenHeight: %{public}d",
        static_cast<uint32_t>(screenMode.GetScreenWidth()), static_cast<uint32_t>(screenMode.GetScreenHeight()));
    auto screenBounds = RRect({ 0, 0, screenMode.GetScreenWidth(), screenMode.GetScreenHeight() }, 0.0f, 0.0f);
    auto screenRefreshRate = screenMode.GetScreenRefreshRate();
    TLOGW(WmsLogTag::DMS, "Call rsInterface_ GetScreenCapability ScreenId: %{public}" PRIu64, screenId);
    auto screenCapability = rsInterface_.GetScreenCapability(screenId);
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    TLOGW(WmsLogTag::DMS, "Call RS interface end, create ScreenProperty begin");
    InitScreenProperty(screenId, screenMode, screenCapability, property);

    if (isDensityDpiLoad_) {
        if (screenId == SCREEN_ID_MAIN) {
            TLOGW(WmsLogTag::DMS, "subDensityDpi_ = %{public}f", subDensityDpi_);
            property.SetVirtualPixelRatio(subDensityDpi_);
            property.SetDefaultDensity(subDensityDpi_);
            property.SetDensityInCurResolution(subDensityDpi_);
        } else {
            TLOGW(WmsLogTag::DMS, "densityDpi_ = %{public}f", densityDpi_);
            property.SetVirtualPixelRatio(densityDpi_);
            property.SetDefaultDensity(densityDpi_);
            property.SetDensityInCurResolution(densityDpi_);
        }
    } else {
        property.UpdateVirtualPixelRatio(screenBounds);
    }
    property.SetRefreshRate(screenRefreshRate);
    property.SetDefaultDeviceRotationOffset(defaultDeviceRotationOffset_);
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ != nullptr && screenId == 0
        && (g_screenRotationOffSet == ROTATION_90 || g_screenRotationOffSet == ROTATION_270)) {
        screenBounds = RRect({ 0, 0, screenMode.GetScreenHeight(), screenMode.GetScreenWidth() }, 0.0f, 0.0f);
        property.SetBounds(screenBounds);
    }
    auto creaseRegion = GetCurrentFoldCreaseRegion();
    if (creaseRegion != nullptr) {
        property.SetCreaseRects(creaseRegion->GetCreaseRects());
    }
#endif
    property.CalcDefaultDisplayOrientation();
    property.SetScreenShape(ScreenSettingHelper::GetScreenShape(screenId));
}

void ScreenSessionManager::InitScreenProperty(ScreenId screenId, RSScreenModeInfo& screenMode,
    RSScreenCapability& screenCapability, ScreenProperty& property)
{
    auto screenBounds = RRect({ 0, 0, screenMode.GetScreenWidth(), screenMode.GetScreenHeight() }, 0.0f, 0.0f);
    property.SetRotation(0.0f);
    property.SetPhyWidth(screenCapability.GetPhyWidth());
    property.SetPhyHeight(screenCapability.GetPhyHeight());
    property.SetValidWidth(screenBounds.rect_.width_);
    property.SetValidHeight(screenBounds.rect_.height_);
    property.SetDpiPhyBounds(screenCapability.GetPhyWidth(), screenCapability.GetPhyHeight());
    property.SetPhyBounds(screenBounds);
    property.SetBounds(screenBounds);
    property.SetAvailableArea({0, 0, screenMode.GetScreenWidth(), screenMode.GetScreenHeight()});
    property.SetPhysicalTouchBounds(FoldScreenStateInternel::IsSecondaryDisplayFoldDevice());
    property.SetInputOffsetY(FoldScreenStateInternel::IsSecondaryDisplayFoldDevice(), GetFoldDisplayMode());
    property.SetCurrentOffScreenRendering(false);
    property.SetScreenRealWidth(property.GetBounds().rect_.GetWidth());
    property.SetScreenRealHeight(property.GetBounds().rect_.GetHeight());
    property.SetScreenRealPPI();
    property.SetScreenRealDPI();
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
        session->SetExtendProperty(screenBounds, false, false);
        session->SetAvailableArea({0, 0, FOUR_K_WIDTH, screenAdjustHeight});
        TLOGD(WmsLogTag::DMS, "screenWidth > 4K, screenId:%{public}" PRIu64"", screenId);
    } else if (screenWidth < g_internalWidth && screenPPI < offScreenPPIThreshold) {
        float scale = static_cast<float>(g_internalWidth) / screenWidth;
        uint32_t screenAdjustHeight = static_cast<uint32_t>(std::round(screenHeight * scale));
        auto screenBounds = RRect({ 0, 0, g_internalWidth, screenAdjustHeight }, 0.0f, 0.0f);
        session->SetExtendProperty(screenBounds, false, true);
        session->SetAvailableArea({0, 0, g_internalWidth, screenAdjustHeight});
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
        return;
    }
    ScreenProperty screenProperty = screenSession->GetScreenProperty();
    if (!screenProperty.GetCurrentOffScreenRendering()) {
        TLOGI(WmsLogTag::DMS, "ScreenId: %{public}" PRIu64", propertyCurrentOffScreenRendering is false", screenId);
        return;
    }
    uint32_t screenAdjustWidth;
    uint32_t screenAdjustHeight;
    if (screenSession ->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
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
    int32_t res = -1;
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetPhysicalScreenResolution(%" PRIu64")", screenId);
    res = rsInterface_.SetPhysicalScreenResolution(rsScreenId, screenAdjustWidth, screenAdjustHeight);
    if (res != StatusCode::SUCCESS) {
        TLOGE(WmsLogTag::DMS, "RS SetPhysicalScreenResolution failed.");
        screenEventTracker_.RecordEvent("SetPhysicalScreenResolution failed.");
    } else {
        TLOGD(WmsLogTag::DMS, "RS SetPhysicalScreenResolution success.");
        screenEventTracker_.RecordEvent("SetPhysicalScreenResolution success.");
    }
    std::ostringstream oss;
    oss << "screenId:" << screenId
        << ", screenAdjustWidth: " <<  screenAdjustWidth
        << ", screenAdjustHeight: " << screenAdjustHeight;
    TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
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
    TLOGW(WmsLogTag::DMS, "extendDensity = %{public}f", extendDensity);
    session->SetVirtualPixelRatio(extendDensity);
    session->SetDefaultDensity(extendDensity);
    session->SetDensityInCurResolution(curResolution);
    ScreenId screenId = session->GetScreenId();
    property.SetVirtualPixelRatio(extendDensity);
    property.SetDefaultDensity(extendDensity);
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

    if (g_isPcDevice && phyScreenPropMap_.size() > 1) {
        // pc is none, pad&&phone is mirror
        TLOGW(WmsLogTag::DMS, "Only Support one External screen.");
        return nullptr;
    }
    screenIdManager_.UpdateScreenId(screenId, screenId);

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
        screenSessionMap_[screenId] = session;
    }
    if (g_isPcDevice) {
        SetMultiScreenFrameControl();
    }
    screenEventTracker_.RecordEvent("create screen session success.");
    SetHdrFormats(screenId, session);
    SetColorSpaces(screenId, session);
    RegisterRefreshRateChangeListener();
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
            ScreenSensorConnector::SubscribeTentSensor();
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
    currentWakeUpReason_ = reason;
    TLOGI(WmsLogTag::DMS, "[UL_POWER]WakeUpBegin reason: %{public}u", reason);
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
    // 该接口当前只有Power调用
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "[UL_POWER]ssm:SuspendBegin(%u)", reason);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return false;
    }

    gotScreenlockFingerprint_ = false;
    TLOGI(WmsLogTag::DMS, "[UL_POWER]Reason: %{public}u", static_cast<uint32_t>(reason));
    lastWakeUpReason_ = PowerStateChangeReason::STATE_CHANGE_REASON_INIT;
    if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF) {
        lastWakeUpReason_ = PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
    }
    // 多屏协作灭屏不通知锁屏
    gotScreenOffNotify_  = false;
    sessionDisplayPowerController_->canCancelSuspendNotify_ = true;
    sessionDisplayPowerController_->isSuspendBegin_ = true;
    sessionDisplayPowerController_->SuspendBegin(reason);
    if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION) {
        isMultiScreenCollaboration_ = true;
        return true;
    }
    return NotifyDisplayPowerEvent(DisplayPowerEvent::SLEEP, EventStatus::BEGIN, reason);
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

    sptr<ScreenSession> internalSession;
    sptr<ScreenSession> externalSession;
    GetInternalAndExternalSession(internalSession, externalSession);
    ScreenPowerStatus status;
    switch (state) {
        case ScreenPowerState::POWER_ON: {
            status = ScreenPowerStatus::POWER_STATUS_ON;
            MultiScreenManager::GetInstance().InternalScreenOnChange(internalSession, externalSession);
            TLOGI(WmsLogTag::DMS, "[UL_POWER]Set ScreenPowerStatus: POWER_STATUS_ON");
            break;
        }
        case ScreenPowerState::POWER_OFF: {
            status = ScreenPowerStatus::POWER_STATUS_OFF;
            MultiScreenManager::GetInstance().InternalScreenOffChange(internalSession, externalSession);
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
            TLOGW(WmsLogTag::DMS, "[UL_POWER] cannot get ScreenSession, screenId: %{public}" PRIu64"",
                screenId);
            continue;
        }
        screenSession->UpdateDisplayState(state);
        TLOGI(WmsLogTag::DMS, "[UL_POWER]set screenSession displayState property: %{public}u",
            screenSession->GetScreenProperty().GetDisplayState());
    }
}

void ScreenSessionManager::BlockScreenOnByCV(void)
{
    if (keyguardDrawnDone_ == false) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]screenOnCV_ set");
        needScreenOnWhenKeyguardNotify_ = true;
        std::unique_lock<std::mutex> lock(screenOnMutex_);
        if (screenOnCV_.wait_for(lock, std::chrono::milliseconds(screenOnDelay_)) == std::cv_status::timeout) {
            TLOGI(WmsLogTag::DMS, "[UL_POWER]wait ScreenOnCV_ timeout");
        }
    }
}

void ScreenSessionManager::BlockScreenOffByCV(void)
{
    if (gotScreenOffNotify_ == false) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER]screenOffCV_ set, delay:%{public}d", screenOffDelay_);
        needScreenOffNotify_ = true;
        std::unique_lock<std::mutex> lock(screenOffMutex_);
        if (screenOffCV_.wait_for(lock, std::chrono::milliseconds(screenOffDelay_)) == std::cv_status::timeout) {
            isScreenLockSuspend_ = false;
            needScreenOffNotify_ = false;
            TLOGI(WmsLogTag::DMS, "[UL_POWER]wait ScreenOffCV_ timeout, isScreenLockSuspend_ is false");
        }
    }
}

bool ScreenSessionManager::TryToCancelScreenOff()
{
    std::lock_guard<std::mutex> notifyLock(sessionDisplayPowerController_->notifyMutex_);
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
    TLOGD(WmsLogTag::DMS, "screenId:%{public}" PRIu64", level:%{public}u,", screenId, level);
    RSInterfaces::GetInstance().SetScreenBacklight(screenId, level);
    return true;
}

uint32_t ScreenSessionManager::GetScreenBrightness(uint64_t screenId)
{
    uint32_t level = static_cast<uint32_t>(RSInterfaces::GetInstance().GetScreenBacklight(screenId));
    TLOGI(WmsLogTag::DMS, "screenId:%{public}" PRIu64", level:%{public}u,", screenId, level);
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
}

bool ScreenSessionManager::IsScreenLockSuspend(void)
{
    return isScreenLockSuspend_;
}

void ScreenSessionManager::NotifyDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type)
{
    if (clientProxy_) {
        clientProxy_->OnDisplayStateChanged(defaultDisplayId, displayInfo, displayInfoMap, type);
    }
}

void ScreenSessionManager::NotifyScreenshot(DisplayId displayId)
{
    if (clientProxy_) {
        clientProxy_->OnScreenshot(displayId);
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
    TLOGI(WmsLogTag::DMS, "screen id:%{public}" PRIu64 ", state:%{public}u",
        screenId, state);

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

    CallRsSetScreenPowerStatusSync(screenId, status);
    if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION) {
        return true;
    }
    return NotifyDisplayPowerEvent(state == ScreenPowerState::POWER_ON ? DisplayPowerEvent::DISPLAY_ON :
        DisplayPowerEvent::DISPLAY_OFF, EventStatus::END, reason);
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
    ScreenPowerStatus status;

    if (!GetPowerStatus(state, reason, status)) {
        return false;
    }
    gotScreenOffNotify_  = false;
    keyguardDrawnDone_ = false;
    TLOGI(WmsLogTag::DMS, "keyguardDrawnDone_ is false");
    prePowerStateChangeReason_ = reason;
    return SetScreenPower(status, reason);
}

bool ScreenSessionManager::GetPowerStatus(ScreenPowerState state, PowerStateChangeReason reason,
    ScreenPowerStatus& status)
{
    switch (state) {
        case ScreenPowerState::POWER_ON: {
            if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT) {
                // 预亮屏
                status = ScreenPowerStatus::POWER_STATUS_ON_ADVANCED;
                TLOGI(WmsLogTag::DMS, "[UL_POWER]Set ScreenPowerStatus: POWER_STATUS_ON_ADVANCED");
            } else {
                status = ScreenPowerStatus::POWER_STATUS_ON;
                TLOGI(WmsLogTag::DMS, "[UL_POWER]Set ScreenPowerStatus: POWER_STATUS_ON");
            }
            break;
        }
        case ScreenPowerState::POWER_OFF: {
            if (reason == PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF) {
                // 预亮屏时指纹认证失败
                status = ScreenPowerStatus::POWER_STATUS_OFF_ADVANCED;
                TLOGI(WmsLogTag::DMS, "[UL_POWER]Set ScreenPowerStatus: POWER_STATUS_OFF_ADVANCED");
            } else {
                status = ScreenPowerStatus::POWER_STATUS_OFF;
                TLOGI(WmsLogTag::DMS, "[UL_POWER]Set ScreenPowerStatus: POWER_STATUS_OFF");
            }
            rsInterface_.MarkPowerOffNeedProcessOneFrame();
            break;
        }
        case ScreenPowerState::POWER_SUSPEND: {
            status = ScreenPowerStatus::POWER_STATUS_SUSPEND;
            TLOGI(WmsLogTag::DMS, "[UL_POWER]Set ScreenPowerStatus: POWER_SUSPEND");
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
            TLOGW(WmsLogTag::DMS, "[UL_POWER]SetScreenPowerStatus state not support");
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
    TLOGI(WmsLogTag::DMS, "[UL_POWER] enter status:%{public}u", status);
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
        for (auto screenId : screenIds) {
            CallRsSetScreenPowerStatusSync(screenId, status);
        }
    }
#else
    for (auto screenId : screenIds) {
        CallRsSetScreenPowerStatusSync(screenId, status);
    }
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
    rsInterface_.SetScreenPowerStatus(screenId, status);
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
        rsInterface_.SetScreenPowerStatus(screenId, status);
    };
    screenPowerTaskScheduler_->PostVoidSyncTask(rsSetScreenPowerStatusTask, "rsInterface_.SetScreenPowerStatus task");
}

void ScreenSessionManager::CallRsSetScreenPowerStatusSyncForFold(ScreenPowerStatus status)
{
#ifdef FOLD_ABILITY_ENABLE
    auto rsSetScreenPowerStatusTask = [=] {
        if (foldScreenController_ == nullptr) {
            TLOGNW(WmsLogTag::DMS, "foldScreenController_ is null");
            return;
        }
        rsInterface_.SetScreenPowerStatus(foldScreenController_->GetCurrentScreenId(), status);
    };
    screenPowerTaskScheduler_->PostVoidSyncTask(rsSetScreenPowerStatusTask, "rsInterface_.SetScreenPowerStatus task");
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
            if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
                SecondaryFoldSensorManager::GetInstance().RegisterPostureCallback();
                SecondaryFoldSensorManager::GetInstance().PowerKeySetScreenActiveRect();
            } else {
                FoldScreenSensorManager::GetInstance().RegisterPostureCallback();
            }
        } else {
            TLOGI(WmsLogTag::DMS, "not fold product, switch screen reason, failed register posture.");
        }
#endif
    } else if (status == ScreenPowerStatus::POWER_STATUS_OFF || status == ScreenPowerStatus::POWER_STATUS_SUSPEND ||
        status == ScreenPowerStatus::POWER_STATUS_DOZE || status == ScreenPowerStatus::POWER_STATUS_DOZE_SUSPEND) {
        TLOGI(WmsLogTag::DMS, "unsubscribe rotation and posture sensor when phone turn off");
        if (isMultiScreenCollaboration_) {
            TLOGI(WmsLogTag::DMS, "[UL_POWER]MultiScreenCollaboration, not unsubscribe rotation sensor");
        } else {
            DmsXcollie dmsXcollie("DMS:UnsubscribeRotationSensor", XCOLLIE_TIMEOUT_10S);
            ScreenSensorConnector::UnsubscribeRotationSensor();
        }
#if defined(SENSOR_ENABLE) && defined(FOLD_ABILITY_ENABLE)
        if (g_foldScreenFlag && reason != PowerStateChangeReason::STATE_CHANGE_REASON_DISPLAY_SWITCH) {
            if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
                SecondaryFoldSensorManager::GetInstance().UnRegisterPostureCallback();
                SecondaryFoldSensorManager::GetInstance().isPowerRectExe_ = false;
            } else {
                FoldScreenSensorManager::GetInstance().UnRegisterPostureCallback();
            }
        } else {
            TLOGI(WmsLogTag::DMS, "not fold product, switch screen reason, failed unregister posture.");
        }
#endif
    } else {
        TLOGI(WmsLogTag::DMS, "SetScreenPower state not support");
        screenEventTracker_.RecordEvent("HandlerSensor start!");
    }
}

void ScreenSessionManager::BootFinishedCallback(const char *key, const char *value, void *context)
{
    if (strcmp(key, BOOTEVENT_BOOT_COMPLETED.c_str()) == 0 && strcmp(value, "true") == 0) {
        TLOGI(WmsLogTag::DMS, "boot animation finished");
        auto &that = *reinterpret_cast<ScreenSessionManager *>(context);
        that.SetRotateLockedFromSettingData();
        that.SetDpiFromSettingData();
        that.SetDisplayState(DisplayState::ON);
        that.RegisterSettingDpiObserver();
        if (that.foldScreenPowerInit_ != nullptr) {
            that.foldScreenPowerInit_();
        }
        that.RegisterSettingRotationObserver();
        that.RegisterSettingscreenSkipProtectedWindowObserver();
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
            that.OnSuperFoldStatusChange(screenId, status);
            float sensorRotation = screenSession->GetSensorRotation();
            if (sensorRotation >= 0.0f) {
                that.OnSensorRotationChange(sensorRotation, screenId);
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

void ScreenSessionManager::RegisterSettingDpiObserver()
{
    TLOGI(WmsLogTag::DMS, "Register Setting Dpi Observer");
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) { SetDpiFromSettingData(); };
    ScreenSettingHelper::RegisterSettingDpiObserver(updateFunc);
}

void ScreenSessionManager::SetDpiFromSettingData()
{
    uint32_t settingDpi;
    bool ret = ScreenSettingHelper::GetSettingDpi(settingDpi);
    if (!ret) {
        TLOGW(WmsLogTag::DMS, "get setting dpi failed,use default dpi");
        settingDpi = defaultDpi;
    } else {
        TLOGI(WmsLogTag::DMS, "get setting dpi success,settingDpi: %{public}u", settingDpi);
    }
    if (settingDpi >= DOT_PER_INCH_MINIMUM_VALUE && settingDpi <= DOT_PER_INCH_MAXIMUM_VALUE
        && cachedSettingDpi_ != settingDpi) {
        cachedSettingDpi_ = settingDpi;
        float dpi = static_cast<float>(settingDpi) / BASELINE_DENSITY;
        ScreenId defaultScreenId = GetDefaultScreenId();
        SetVirtualPixelRatio(defaultScreenId, dpi);
        if (g_isPcDevice) {
            SetExtendPixelRatio(dpi);
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
    auto state = static_cast<ScreenPowerState>(RSInterfaces::GetInstance().GetScreenPowerStatus(screenId));
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
    if (!g_foldScreenFlag || FoldScreenStateInternel::IsSuperFoldDisplayDevice()) {
        state = static_cast<ScreenPowerState>(RSInterfaces::GetInstance()
            .GetScreenPowerStatus(GetDefaultScreenId()));
    } else {
        state = static_cast<ScreenPowerState>(RSInterfaces::GetInstance()
            .GetScreenPowerStatus(foldScreenController_->GetCurrentScreenId()));
    }
    std::ostringstream oss;
    oss << "GetScreenPower state:" << static_cast<uint32_t>(state);
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    screenEventTracker_.RecordEvent(oss.str());
    return state;
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

void ScreenSessionManager::UpdateScreenDirectionInfo(ScreenId screenId, float screenComponentRotation, float rotation,
    float phyRotation, ScreenPropertyChangeType screenPropertyChangeType)
{
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
    screenSession->SetPhysicalRotation(phyRotation);
    screenSession->SetScreenComponentRotation(screenComponentRotation);
    screenSession->UpdateRotationOrientation(rotation, GetFoldDisplayMode(), IsOrientationNeedChanged());
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 ", rotation: %{public}f, screenComponentRotation: %{public}f",
        screenId, rotation, screenComponentRotation);
}

void ScreenSessionManager::UpdateScreenRotationProperty(ScreenId screenId, const RRect& bounds, float rotation,
    ScreenPropertyChangeType screenPropertyChangeType)
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
            return;
        } else if (screenPropertyChangeType == ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY ||
            screenPropertyChangeType == ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY_NOT_NOTIFY) {
            sptr<DisplayInfo> displayInfo = screenSession->ConvertToDisplayInfo();
            TLOGI(WmsLogTag::DMS, "Update Screen Rotation Property Only");
            {
                std::lock_guard<std::recursive_mutex> lock_info(displayInfoMutex_);
                screenSession->UpdatePropertyOnly(bounds, rotation, GetFoldDisplayMode(), IsOrientationNeedChanged());
            }
            if (screenPropertyChangeType != ScreenPropertyChangeType::ROTATION_UPDATE_PROPERTY_ONLY_NOT_NOTIFY) {
                NotifyDisplayChanged(displayInfo, DisplayChangeEvent::UPDATE_ROTATION);
                NotifyScreenChanged(screenSession->ConvertToScreenInfo(), ScreenChangeEvent::UPDATE_ROTATION);
            }
            return;
        }
    }
    {
        std::lock_guard<std::recursive_mutex> lock_info(displayInfoMutex_);
        screenSession->UpdatePropertyAfterRotation(bounds, rotation, GetFoldDisplayMode(), IsOrientationNeedChanged());
    }
    sptr<DisplayInfo> displayInfo = screenSession->ConvertToDisplayInfo();
    NotifyAndPublishEvent(displayInfo, screenId, screenSession);
}

void ScreenSessionManager::NotifyDisplayChanged(sptr<DisplayInfo> displayInfo, DisplayChangeEvent event)
{
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "error, displayInfo is nullptr.");
        return;
    }
    auto task = [=] {
        auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
        if (event == DisplayChangeEvent::UPDATE_REFRESHRATE) {
            TLOGND(WmsLogTag::DMS, "evevt:%{public}d, displayId:%{public}" PRIu64", agent size: %{public}u",
                event, displayInfo->GetDisplayId(), static_cast<uint32_t>(agents.size()));
        } else {
            TLOGNI(WmsLogTag::DMS, "evevt:%{public}d, displayId:%{public}" PRIu64", agent size: %{public}u",
                event, displayInfo->GetDisplayId(), static_cast<uint32_t>(agents.size()));
        }
        if (agents.empty()) {
            return;
        }
        for (auto& agent : agents) {
            int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
            if (!IsFreezed(agentPid, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER)) {
                agent->OnDisplayChange(displayInfo, event);
            }
        }
    };
    taskScheduler_->PostAsyncTask(task, "NotifyDisplayChanged");
}

DMError ScreenSessionManager::SetOrientation(ScreenId screenId, Orientation orientation)
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
    screenSession->ScreenOrientationChange(orientation, GetFoldDisplayMode());
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
    SetDisplayBoundary(screenSession);
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
    if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        SecondaryFoldSensorManager::GetInstance().RegisterPostureCallback();
        SecondaryFoldSensorManager::GetInstance().RegisterHallCallback();
    } else {
        FoldScreenSensorManager::GetInstance().RegisterPostureCallback();
        FoldScreenSensorManager::GetInstance().RegisterHallCallback();
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
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_POWER_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER] agents is empty");
        return false;
    }
    TLOGD(WmsLogTag::DMS, "[UL_POWER] start");
    for (auto& agent : agents) {
        agent->NotifyDisplayPowerEvent(event, status);
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
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_STATE_LISTENER);
    if (agents.empty()) {
        TLOGI(WmsLogTag::DMS, "agents is empty");
        return false;
    }
    TLOGI(WmsLogTag::DMS, "notify enter!");
    for (auto& agent : agents) {
        agent->NotifyDisplayStateChanged(id, state);
    }
    return true;
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
}

DMError ScreenSessionManager::GetScreenHDRFormat(ScreenId screenId, ScreenHDRFormat& hdrFormat)
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
    return screenSession->GetScreenHDRFormat(hdrFormat);
}

DMError ScreenSessionManager::SetScreenHDRFormat(ScreenId screenId, int32_t modeIdx)
{
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
}

DMError ScreenSessionManager::GetSupportedColorSpaces(ScreenId screenId,
    std::vector<GraphicCM_ColorSpaceType>& colorSpaces)
{
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
}

DMError ScreenSessionManager::GetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType& colorSpace)
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
    return screenSession->GetScreenColorSpace(colorSpace);
}

DMError ScreenSessionManager::SetScreenColorSpace(ScreenId screenId, GraphicCM_ColorSpaceType colorSpace)
{
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
}

void ScreenSessionManager::AddVirtualScreenDeathRecipient(const sptr<IRemoteObject>& displayManagerAgent,
    ScreenId smsScreenId)
{
    if (deathRecipient_ == nullptr) {
        TLOGW(WmsLogTag::DMS, "Create deathRecipient");
        deathRecipient_ =
            new(std::nothrow) AgentDeathRecipient([this](const sptr<IRemoteObject>& agent) { OnRemoteDied(agent); });
    }
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
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
    if (!Permission::IsSystemCalling() && !SessionPermission::IsShellCall()) {
        return ERROR_ID_NOT_SYSTEM_APP;
    }
    if (!(Permission::IsSystemCalling() && Permission::CheckCallingPermission(SCREEN_CAPTURE_PERMISSION)) &&
        !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return SCREEN_ID_INVALID;
    }
    ExitCoordination("CreateVirtualScreen");
    TLOGI(WmsLogTag::DMS, "ENTER");
    if (SessionPermission::IsBetaVersion()) {
        CheckAndSendHiSysEvent("CREATE_VIRTUAL_SCREEN", "hmos.screenrecorder");
    }
    if (clientProxy_ && option.missionIds_.size() > 0) {
        std::vector<uint64_t> surfaceNodeIds;
        clientProxy_->OnGetSurfaceNodeIdsFromMissionIdsChanged(option.missionIds_, surfaceNodeIds, false);
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
        screenSession->SetMirrorScreenType(MirrorScreenType::VIRTUAL_MIRROR);
        {
            std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
            screenSessionMap_.insert(std::make_pair(smsScreenId, screenSession));
        }
        if (option.name_ == "CastEngine") {
            screenSession->SetVirtualScreenFlag(VirtualScreenFlag::CAST);
        }
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
    return smsScreenId;
}

DMError ScreenSessionManager::SetVirtualScreenSurface(ScreenId screenId, sptr<IBufferProducer> surface)
{
    if (!(Permission::IsSystemCalling() && Permission::CheckCallingPermission(SCREEN_CAPTURE_PERMISSION)) &&
        !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (surface == nullptr) {
        TLOGE(WmsLogTag::DMS, "surface is null");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    sptr<ScreenSession> screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "No such screen.");
        return DMError::DM_ERROR_INVALID_PARAM;
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
    if (!SessionPermission::IsSystemCalling()) {
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
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
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
        if (CheckScreenInScreenGroup(screen)) {
            NotifyDisplayDestroy(screenId);
        }
        {
            std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
            auto screenGroup = RemoveFromGroupLocked(screen);
            if (screenGroup != nullptr) {
                NotifyScreenGroupChanged(screen->ConvertToScreenInfo(), ScreenGroupChangeEvent::REMOVE_FROM_GROUP);
            }
            screenSessionMap_.erase(screenId);
        }
        NotifyScreenDisconnected(screenId);
        TLOGW(WmsLogTag::DMS, "destroy success, id: %{public}" PRIu64, screenId);
    }
    screenIdManager_.DeleteScreenId(screenId);
    virtualScreenCount_ = virtualScreenCount_ > 0 ? virtualScreenCount_ - 1 : 0;
    NotifyCaptureStatusChanged();
    if (rsScreenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::DMS, "No corresponding rsScreenId");
        return DMError::DM_ERROR_INVALID_PARAM;
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
    DMRect mainScreenRegion, ScreenId& screenGroupId)
{
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
    if (allMirrorScreenIds.empty()) {
        TLOGW(WmsLogTag::DMS, "make mirror screen id empty");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:MakeMirror start");
    for (ScreenId screenId : allMirrorScreenIds) {
        OnVirtualScreenChange(screenId, ScreenEvent::DISCONNECTED);
    }
    if (foldScreenController_ != nullptr && FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        foldScreenController_->SetMainScreenRegion(mainScreenRegion);
    }
    DMError makeResult = MultiScreenManager::GetInstance().MirrorSwitch(mainScreenId,
        allMirrorScreenIds, mainScreenRegion, screenGroupId);
    for (ScreenId screenId : allMirrorScreenIds) {
        MirrorSwitchNotify(screenId);
    }
    RegisterCastObserver(allMirrorScreenIds);
    TLOGW(WmsLogTag::DMS, "make mirror notify scb end makeResult=%{public}d", makeResult);
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "dms:MakeMirror end");
    return makeResult;
}

DMError ScreenSessionManager::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
    ScreenId& screenGroupId)
{
    return DoMakeMirror(mainScreenId, mirrorScreenIds, DMRect::NONE(), screenGroupId);
}

DMError ScreenSessionManager::MakeMirrorForRecord(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
    ScreenId& screenGroupId)
{
    TLOGW(WmsLogTag::DMS, "start");
    ScreenId realScreenId = mainScreenId;
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && mainScreenId == DISPLAY_ID_FAKE) {
        if (!SuperFoldPolicy::GetInstance().IsFakeDisplayExist()) {
            TLOGE(WmsLogTag::DMS, "fake display is not exist!");
            return DMError::DM_ERROR_INVALID_PARAM;
        }
        realScreenId = 0;
    }
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() &&
        SuperFoldPolicy::GetInstance().IsNeedSetSnapshotRect(mainScreenId)) {
        DMRect mainScreenRect = SuperFoldPolicy::GetInstance().GetRecordRect(mainScreenId);
        return DoMakeMirror(realScreenId, mirrorScreenIds, mainScreenRect, screenGroupId);
    }
    return DoMakeMirror(mainScreenId, mirrorScreenIds, DMRect::NONE(), screenGroupId);
}


DMError ScreenSessionManager::MakeMirror(ScreenId mainScreenId, std::vector<ScreenId> mirrorScreenIds,
                                         DMRect mainScreenRegion, ScreenId& screenGroupId)
{
    return DoMakeMirror(mainScreenId, mirrorScreenIds, mainScreenRegion, screenGroupId);
}

void ScreenSessionManager::RegisterCastObserver(std::vector<ScreenId>& mirrorScreenIds)
{
    mirrorScreenIds_ = mirrorScreenIds;
    TLOGI(WmsLogTag::DMS, "Register Setting cast Observer");
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) { SetCastFromSettingData(); };
    ScreenSettingHelper::RegisterSettingCastObserver(updateFunc);
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
    for (ScreenId screenId : mirrorScreenIds_) {
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
    ScreenSettingHelper::RegisterSettingRotationObserver(updateFunc);
}

void ScreenSessionManager::RegisterSettingscreenSkipProtectedWindowObserver()
{
    TLOGI(WmsLogTag::DMS, "start");
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) { SetScreenSkipProtectedWindowInner(); };
    ScreenSettingHelper::RegisterSettingscreenSkipProtectedWindowObserver(updateFunc);
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
    } else {
        screenSession->UpdateRefreshRate(defaultScreenSession->GetRefreshRate() / refreshInterval);
    }
    TLOGW(WmsLogTag::DMS, "refreshInterval is %{public}d", refreshInterval);
    return DMError::DM_OK;
}

DMError ScreenSessionManager::VirtualScreenUniqueSwitch(const std::vector<ScreenId>& screenIds)
{
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
    DMError uniqueSwitchRet = MultiScreenManager::GetInstance().VirtualScreenUniqueSwitch(defaultScreen, screenIds);
    TLOGW(WmsLogTag::DMS, "result: %{public}d", uniqueSwitchRet);
    return uniqueSwitchRet;
}

DMError ScreenSessionManager::MakeUniqueScreen(const std::vector<ScreenId>& screenIds,
    std::vector<DisplayId>& displayIds)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    TLOGW(WmsLogTag::DMS, "enter!");
    if (screenIds.empty()) {
        TLOGE(WmsLogTag::DMS, "screen is empty");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    ScreenId uniqueScreenId = screenIds[0];
    auto uniqueScreen = GetScreenSession(uniqueScreenId);
    if (uniqueScreen != nullptr) {
        if (uniqueScreen->GetSourceMode() == ScreenSourceMode::SCREEN_UNIQUE) {
            TLOGW(WmsLogTag::DMS, "make unique ignore");
            return DMError::DM_OK;
        }
        return MultiScreenManager::GetInstance().UniqueSwitch(screenIds, displayIds);
    }
    for (auto screenId : screenIds) {
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
        // notify scb to build Screen widget
        OnVirtualScreenChange(screenId, ScreenEvent::CONNECTED);
    }
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        TLOGD(WmsLogTag::DMS, "flush data");
        transactionProxy->FlushImplicitTransaction();
    }
    TLOGI(WmsLogTag::DMS, "end!");
    return DMError::DM_OK;
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
        rsDisplayNode = GetRSDisplayNodeByScreenId(allExpandScreenIds[i]);
        points.emplace_back(pointsMap[allExpandScreenIds[i]]);
        if (rsDisplayNode != nullptr) {
            rsDisplayNode->SetDisplayOffset(pointsMap[allExpandScreenIds[i]].posX_,
                pointsMap[allExpandScreenIds[i]].posY_);
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
    if (defaultScreen != nullptr && defaultScreen->GetActiveScreenMode() != nullptr) {
        info->refreshRate_ = defaultScreen->GetActiveScreenMode()->refreshRate_;
    }
    screenSession->modes_.emplace_back(info);
    screenSession->activeIdx_ = 0;
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenSession->SetVirtualPixelRatio(option.density_);
    screenSession->SetIsPcUse(g_isPcDevice ? true : false);
    screenSession->SetDisplayBoundary(RectF(0, 0, option.width_, option.height_), 0);
    screenSession->RegisterScreenChangeListener(this);
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
        TLOGW(WmsLogTag::DMS, "fill screen idx:%{public}d w/h:%{public}d/%{public}d",
            rsScreenModeInfo.GetScreenModeId(), info->width_, info->height_);
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
    if (ScreenSceneConfig::GetExternalScreenDefaultMode() == "none") {
        if (IsExtendMode()) {
            point = {newScreen->GetScreenProperty().GetStartX(), newScreen->GetScreenProperty().GetStartY()};
        }
    }
    if (!screenGroup->AddChild(newScreen, point, GetScreenSession(GetDefaultScreenId()))) {
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
    bool res = screenGroup->RemoveChild(screen);
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        transactionProxy->FlushImplicitTransaction();
        TLOGI(WmsLogTag::DMS, "remove child and call flush.");
    }
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

DMError ScreenSessionManager::SetMirror(ScreenId screenId, std::vector<ScreenId> screens, DMRect mainScreenRegion)
{
    TLOGI(WmsLogTag::DMS, "screenId:%{public}" PRIu64"", screenId);
    sptr<ScreenSession> screen = GetScreenSession(screenId);
    if (screen == nullptr || screen->GetScreenProperty().GetScreenType() != ScreenType::REAL) {
        TLOGE(WmsLogTag::DMS, "screen is nullptr, or screenType is not real.");
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
        group->combination_ == ScreenCombination::SCREEN_MIRROR && group->mirrorScreenId_ == screen->screenId_;
    group->mirrorScreenId_ = screen->screenId_;
    ChangeScreenGroup(group, screens, startPoints, filterMirroredScreen, ScreenCombination::SCREEN_MIRROR,
        mainScreenRegion);
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
    const std::vector<Point>& startPoints, bool filterScreen, ScreenCombination combination, DMRect mainScreenRegion)
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
        if (filterScreen && screen->groupSmsId_ == group->screenId_ && group->HasChild(screen->screenId_)) {
            // screen already in group
            if (combination != ScreenCombination::SCREEN_MIRROR ||
                screen->GetMirrorScreenRegion().second == mainScreenRegion) {
                continue;
            }
            // mirror mode and mirror area change
            TLOGI(WmsLogTag::DMS, "Screen: %{public}" PRIu64
                ", apply new region, x:%{public}d y:%{public}d w:%{public}u h:%{public}u",
                screenId, mainScreenRegion.posX_, mainScreenRegion.posY_,
                mainScreenRegion.width_, mainScreenRegion.height_);
        }
        if (CheckScreenInScreenGroup(screen)) {
            NotifyDisplayDestroy(screenId);
        }
        auto originGroup = RemoveFromGroupLocked(screen);
        addChildPos.emplace_back(startPoints[i]);
        removeChildResMap[screenId] = originGroup != nullptr;
        addScreens.emplace_back(screenId);
        if (combination == ScreenCombination::SCREEN_MIRROR) {
            auto mirrorScreenId = group->mirrorScreenId_;
            ScreenId rsScreenId = SCREEN_ID_INVALID;
            if (!ConvertScreenIdToRsScreenId(screenId, rsScreenId)) {
                TLOGE(WmsLogTag::DMS, "Screen: %{public}" PRIu64" convert to rs id failed", mirrorScreenId);
            } else {
                screen->SetMirrorScreenRegion(rsScreenId, mainScreenRegion);
                screen->SetIsPhysicalMirrorSwitch(false);
                TLOGI(WmsLogTag::DMS, "Screen: %{public}" PRIu64" mirror to %{public}"
                    PRIu64" with region, x:%{public}d y:%{public}d w:%{public}u h:%{public}u",
                    screenId, mirrorScreenId, mainScreenRegion.posX_, mainScreenRegion.posY_,
                    mainScreenRegion.width_, mainScreenRegion.height_);
            }
        }
    }
    group->combination_ = combination;
    AddScreenToGroup(group, addScreens, addChildPos, removeChildResMap);
}

void ScreenSessionManager::AddScreenToGroup(sptr<ScreenSessionGroup> group,
    const std::vector<ScreenId>& addScreens, const std::vector<Point>& addChildPos,
    std::map<ScreenId, bool>& removeChildResMap)
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
        bool addChildRes = group->AddChild(screen, expandPoint, GetScreenSession(group->mirrorScreenId_));
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

std::shared_ptr<Media::PixelMap> ScreenSessionManager::GetScreenSnapshot(DisplayId displayId, bool isUseDma)
{
    DisplayId realDisplayId = displayId;
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() && displayId == DISPLAY_ID_FAKE) {
        if (!SuperFoldPolicy::GetInstance().IsFakeDisplayExist()) {
            TLOGE(WmsLogTag::DMS, "fake display is not exist!");
            return nullptr;
        }
        realDisplayId = 0;
    }
    std::shared_ptr<RSDisplayNode> displayNode = GetDisplayNodeByDisplayId(realDisplayId);
    if (displayNode == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayNode is null!");
        return nullptr;
    }
    std::shared_ptr<SurfaceCaptureFuture> callback = std::make_shared<SurfaceCaptureFuture>();
    RSSurfaceCaptureConfig config;
    config.useDma = isUseDma;
    TLOGW(WmsLogTag::DMS, "take surface capture with dma=%{public}d", isUseDma);
    if (foldScreenController_ != nullptr && FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        config.mainScreenRect = foldScreenController_->GetScreenSnapshotRect();
    }
    if (FoldScreenStateInternel::IsSuperFoldDisplayDevice() &&
        SuperFoldPolicy::GetInstance().IsNeedSetSnapshotRect(displayId)) {
        config.mainScreenRect = SuperFoldPolicy::GetInstance().GetSnapshotRect(displayId);
    }
    bool ret = rsInterface_.TakeSurfaceCapture(displayNode, callback, config);
    if (!ret) {
        TLOGE(WmsLogTag::DMS, "TakeSurfaceCapture failed");
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> screenshot = callback->GetResult(2000);  // wait for <= 2000ms
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

std::shared_ptr<Media::PixelMap> ScreenSessionManager::GetDisplaySnapshot(DisplayId displayId,
    DmErrorCode* errorCode, bool isUseDma)
{
    TLOGD(WmsLogTag::DMS, "enter!");
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsShellCall() && errorCode) {
        *errorCode = DmErrorCode::DM_ERROR_NOT_SYSTEM_APP;
        return nullptr;
    }
    if (system::GetBoolParameter("persist.edm.disallow_screenshot", false)) {
        TLOGI(WmsLogTag::DMS, "snapshot disabled by edm!");
        *errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
        return nullptr;
    }
    if (displayId == DISPLAY_ID_FAKE && !IsFakeDisplayExist()) {
        *errorCode = DmErrorCode::DM_ERROR_INVALID_SCREEN;
        TLOGE(WmsLogTag::DMS, "fake display not exist!");
        return nullptr;
    }
    if ((Permission::IsSystemCalling() && Permission::CheckCallingPermission(SCREEN_CAPTURE_PERMISSION)) ||
        SessionPermission::IsShellCall()) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetDisplaySnapshot(%" PRIu64")", displayId);
        auto res = GetScreenSnapshot(displayId, isUseDma);
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

std::shared_ptr<Media::PixelMap> ScreenSessionManager::GetDisplaySnapshotWithOption(const CaptureOption& option,
    DmErrorCode* errorCode)
{
    TLOGD(WmsLogTag::DMS, "enter!");
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsShellCall() && errorCode) {
        *errorCode = DmErrorCode::DM_ERROR_NOT_SYSTEM_APP;
        return nullptr;
    }
    if (system::GetBoolParameter("persist.edm.disallow_screenshot", false)) {
        TLOGI(WmsLogTag::DMS, "snapshot was disabled by edm!");
        return nullptr;
    }
    if (option.displayId_ == DISPLAY_ID_FAKE && !IsFakeDisplayExist()) {
        TLOGE(WmsLogTag::DMS, "fake display not exist!");
        *errorCode = DmErrorCode::DM_ERROR_INVALID_SCREEN;
        return nullptr;
    }
    if ((Permission::IsSystemCalling() && Permission::CheckCallingPermission(SCREEN_CAPTURE_PERMISSION)) ||
        SessionPermission::IsShellCall()) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetDisplaySnapshot(%" PRIu64")", option.displayId_);
        auto res = GetScreenSnapshot(option.displayId_, true);
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
    auto agentIter = screenAgentMap_.find(agent);
    if (agentIter != screenAgentMap_.end()) {
        while (screenAgentMap_[agent].size() > 0) {
            auto diedId = screenAgentMap_[agent][0];
            TLOGI(WmsLogTag::DMS, "destroy screenId in OnRemoteDied: %{public}" PRIu64"", diedId);
            DMError res = DestroyVirtualScreen(diedId);
            if (res != DMError::DM_OK) {
                TLOGE(WmsLogTag::DMS, "destroy failed in OnRemoteDied: %{public}" PRIu64"", diedId);
            }
        }
        screenAgentMap_.erase(agent);
    }
    return true;
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
    if (displayInfo == nullptr) {
        return;
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    TLOGI(WmsLogTag::DMS, "start, agent size: %{public}u", static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!IsFreezed(agentPid, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER)) {
            agent->OnDisplayCreate(displayInfo);
        }
    }
}

void ScreenSessionManager::NotifyDisplayDestroy(DisplayId displayId)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_EVENT_LISTENER);
    TLOGI(WmsLogTag::DMS, "agent size: %{public}u", static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!IsFreezed(agentPid, DisplayManagerAgentType::DISPLAY_EVENT_LISTENER)) {
            agent->OnDisplayDestroy(displayId);
        }
    }
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
    if (hasPrivate == screenPrivacyStates) {
        TLOGD(WmsLogTag::DMS, "screen session state is not changed, return");
        return;
    }
    TLOGI(WmsLogTag::DMS, "PrivateSession status : %{public}u", hasPrivate);
    screenPrivacyStates = hasPrivate;
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::PRIVATE_WINDOW_LISTENER);
    if (agents.empty()) {
        return;
    }
    for (auto& agent : agents) {
        agent->NotifyPrivateWindowStateChanged(hasPrivate);
    }
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
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get default screen now");
        return;
    }
    screenSession->SetPrivateSessionForeground(hasPrivate);
    NotifyPrivateSessionStateChanged(hasPrivate);
}

void ScreenSessionManager::SetPrivacyStateByDisplayId(DisplayId id, bool hasPrivate)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    TLOGI(WmsLogTag::DMS, "enter, hasPrivate: %{public}d", hasPrivate);
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
    screenSession->SetPrivateSessionForeground(hasPrivate);
    NotifyPrivateSessionStateChanged(hasPrivate);
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
    TLOGI(WmsLogTag::DMS, "Notify displayid: %{public}" PRIu64"", id);
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::PRIVATE_WINDOW_LIST_LISTENER);
    if (agents.empty()) {
        return;
    }
    for (auto& agent : agents) {
        agent->NotifyPrivateStateWindowListChanged(id, privacyWindowList);
    }
}

DMError ScreenSessionManager::HasPrivateWindow(DisplayId id, bool& hasPrivateWindow)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permmision Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (id == DISPLAY_ID_FAKE) {
        auto displayInfo = GetDefaultDisplayInfo();
        if (displayInfo) {
            id = displayInfo->GetDisplayId();
            TLOGI(WmsLogTag::DMS, "change displayId: %{public}" PRIu64" to displayId: %{public}" PRIu64,
                DISPLAY_ID_FAKE, id);
        }
    }
    std::vector<ScreenId> screenIds = GetAllScreenIds();
    auto iter = std::find(screenIds.begin(), screenIds.end(), id);
    if (iter == screenIds.end()) {
        TLOGE(WmsLogTag::DMS, "invalid displayId");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    auto screenSession = GetScreenSession(id);
    if (screenSession == nullptr) {
        return DMError::DM_ERROR_NULLPTR;
    }
    hasPrivateWindow = screenSession->HasPrivateSessionForeground();
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
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    std::vector<sptr<ScreenInfo>> infos;
    for (auto& screenInfo : screenInfos) {
        if (screenInfo != nullptr) {
            infos.emplace_back(screenInfo);
        }
    }
    if (agents.empty() || infos.empty()) {
        return;
    }
    for (auto& agent : agents) {
        agent->OnScreenGroupChange(trigger, infos, groupEvent);
    }
}

void ScreenSessionManager::OnScreenConnect(const sptr<ScreenInfo> screenInfo)
{
    if (screenInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "screenInfo nullptr");
        return;
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGI(WmsLogTag::DMS, "agents empty");
        return;
    }
    TLOGI(WmsLogTag::DMS, "start");
    for (auto& agent : agents) {
        agent->OnScreenConnect(screenInfo);
    }
    NotifyScreenModeChange();
}

void ScreenSessionManager::OnScreenDisconnect(ScreenId screenId)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGI(WmsLogTag::DMS, "agents empty");
        return;
    }
    TLOGI(WmsLogTag::DMS, "start");
    for (auto& agent : agents) {
        agent->OnScreenDisconnect(screenId);
    }
    NotifyScreenModeChange(screenId);
}

void ScreenSessionManager::OnScreenshot(sptr<ScreenshotInfo> info)
{
    if (info == nullptr) {
        TLOGE(WmsLogTag::DMS, "info is null");
        return;
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREENSHOT_EVENT_LISTENER);
    if (agents.empty()) {
        TLOGD(WmsLogTag::DMS, "agents empty");
        return;
    }
    TLOGI(WmsLogTag::DMS, "start");
    for (auto& agent : agents) {
        agent->OnScreenshot(info);
    }
}

sptr<CutoutInfo> ScreenSessionManager::GetCutoutInfo(DisplayId displayId)
{
    DmsXcollie dmsXcollie("DMS:GetCutoutInfo", XCOLLIE_TIMEOUT_10S);
    return screenCutoutController_ ? screenCutoutController_->GetScreenCutoutInfo(displayId) : nullptr;
}

DMError ScreenSessionManager::HasImmersiveWindow(ScreenId screenId, bool& immersive)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied!");
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    if (!clientProxy_) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    clientProxy_->OnImmersiveStateChanged(screenId, immersive);
    return DMError::DM_OK;
}

void ScreenSessionManager::SetDisplayBoundary(const sptr<ScreenSession> screenSession)
{
    if (screenSession && screenCutoutController_) {
        RectF rect =
            screenCutoutController_->CalculateCurvedCompression(screenSession->GetScreenProperty());
        if (!rect.IsEmpty()) {
            screenSession->SetDisplayBoundary(rect, screenCutoutController_->GetOffsetY());
        }
    } else {
        TLOGW(WmsLogTag::DMS, "screenSession or screenCutoutController_ is null");
    }
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

void ScreenSessionManager::SetFoldDisplayMode(const FoldDisplayMode displayMode)
{
    SetFoldDisplayModeInner(displayMode);
}

DMError ScreenSessionManager::SetFoldDisplayModeInner(const FoldDisplayMode displayMode)
{
#ifdef FOLD_ABILITY_ENABLE
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
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
        return DMError::DM_ERROR_INVALID_MODE_ID;
    } else if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice() &&
        IsScreenCasting() && displayMode == FoldDisplayMode::COORDINATION) {
        TLOGW(WmsLogTag::DMS, "is phone casting, SetFoldDisplayMode to %{public}d is not allowed", displayMode);
        return DMError::DM_ERROR_INVALID_MODE_ID;
    }
    foldScreenController_->SetDisplayMode(displayMode);
    NotifyClientProxyUpdateFoldDisplayMode(displayMode);
    return DMError::DM_OK;
#endif
}

DMError ScreenSessionManager::SetFoldDisplayModeFromJs(const FoldDisplayMode displayMode)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    return SetFoldDisplayModeInner(displayMode);
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
    if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        if (FoldDisplayMode::MAIN == GetFoldDisplayMode()) {
            CalcDisplayNodeTranslateOnPocketFoldRotation(session, scaleX, scaleY, pivotX, pivotY,
                translateX, translateY);
        } else {
            CalcDisplayNodeTranslateOnRotation(session, scaleX, scaleY, pivotX, pivotY, translateX, translateY);
        }
    } else if (ROTATE_POLICY == FOLDABLE_DEVICE && FoldDisplayMode::FULL == GetFoldDisplayMode()) {
        CalcDisplayNodeTranslateOnFoldableRotation(session, scaleX, scaleY, pivotX, pivotY, translateX, translateY);
    } else {
        CalcDisplayNodeTranslateOnRotation(session, scaleX, scaleY, pivotX, pivotY, translateX, translateY);
    }
    TLOGD(WmsLogTag::DMS,
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
    return foldScreenController_->GetDisplayMode();
#else
    return FoldDisplayMode::UNKNOWN;
#endif
}

bool ScreenSessionManager::IsFoldable()
{
#ifdef FOLD_ABILITY_ENABLE
    DmsXcollie dmsXcollie("DMS:IsFoldable", XCOLLIE_TIMEOUT_10S);
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

bool ScreenSessionManager::GetCameraMode()
{
    if (!g_foldScreenFlag) {
        return false;
    }
    if (foldScreenController_ == nullptr) {
        TLOGI(WmsLogTag::DMS, "foldScreenController_ is null");
        return false;
    }
    return foldScreenController_->GetCameraMode();
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

uint32_t ScreenSessionManager::GetCurvedCompressionArea()
{
    return ScreenSceneConfig::GetCurvedCompressionAreaInLandscape();
}

void ScreenSessionManager::NotifyFoldStatusChanged(FoldStatus foldStatus)
{
#ifdef FOLD_ABILITY_ENABLE
    sptr<ScreenSession> screenSession = GetDefaultScreenSession();
    if (screenSession != nullptr) {
        if (foldStatus == FoldStatus::FOLDED) {
            screenSession->SetDefaultDeviceRotationOffset(0);
        } else {
            screenSession->SetDefaultDeviceRotationOffset(defaultDeviceRotationOffset_);
        }
    }
    if (screenSession != nullptr && FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice()) {
        // 维护外屏独立dpi
        if (foldStatus == FoldStatus::FOLDED) {
            auto property = screenSession->GetScreenProperty();
            densityDpi_ = property.GetDensity();
            SetVirtualPixelRatio(GetDefaultScreenId(), subDensityDpi_);
        } else {
            SetVirtualPixelRatio(GetDefaultScreenId(), densityDpi_);
        }
    }
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER);
    TLOGI(WmsLogTag::DMS, "foldStatus:%{public}d, agent size: %{public}u",
        foldStatus, static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!IsFreezed(agentPid, DisplayManagerAgentType::FOLD_STATUS_CHANGED_LISTENER)) {
            agent->NotifyFoldStatusChanged(foldStatus);
        }
    }
#endif
}

void ScreenSessionManager::NotifyFoldAngleChanged(std::vector<float> foldAngles)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::FOLD_ANGLE_CHANGED_LISTENER);
    if (agents.empty()) {
        TLOGI(WmsLogTag::DMS, "agents is empty");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(lastStatusUpdateMutex_);
        lastFoldAngles_ = foldAngles;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!IsFreezed(agentPid, DisplayManagerAgentType::FOLD_ANGLE_CHANGED_LISTENER)) {
            agent->NotifyFoldAngleChanged(foldAngles);
        }
    }
}

void ScreenSessionManager::NotifyCaptureStatusChanged()
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::CAPTURE_STATUS_CHANGED_LISTENER);
    bool isCapture = IsCaptured();
    isScreenShot_ = false;
    if (agents.empty()) {
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!IsFreezed(agentPid, DisplayManagerAgentType::CAPTURE_STATUS_CHANGED_LISTENER)) {
            agent->NotifyCaptureStatusChanged(isCapture);
        }
    }
}

void ScreenSessionManager::NotifyCaptureStatusChanged(bool isCapture)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::CAPTURE_STATUS_CHANGED_LISTENER);
    if (agents.empty()) {
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!IsFreezed(agentPid, DisplayManagerAgentType::CAPTURE_STATUS_CHANGED_LISTENER)) {
            agent->NotifyCaptureStatusChanged(isCapture);
        }
    }
}

void ScreenSessionManager::NotifyDisplayChangeInfoChanged(const sptr<DisplayChangeInfo>& info)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_UPDATE_LISTENER);
    if (agents.empty()) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(lastStatusUpdateMutex_);
        lastDisplayChangeInfo_ = info;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!IsFreezed(agentPid, DisplayManagerAgentType::DISPLAY_UPDATE_LISTENER)) {
            agent->NotifyDisplayChangeInfoChanged(info);
        }
    }
}

void ScreenSessionManager::RefreshMirrorScreenRegion(ScreenId screenId)
{
    auto screenSession = GetScreenSession(screenId);
    if (screenSession->GetName() == "CastEngine" || screenSession->GetName() == "Cooperation") {
        DMRect mainScreenRegion = {0, 0, 0, 0};
        foldScreenController_->SetMainScreenRegion(mainScreenRegion);
        ScreenId rsScreenId = SCREEN_ID_INVALID;
        if (!ConvertScreenIdToRsScreenId(screenId, rsScreenId)) {
            TLOGE(WmsLogTag::DMS, "Screen: %{public}" PRIu64" convert to rs id failed", screenId);
        } else {
            screenSession->SetMirrorScreenRegion(rsScreenId, mainScreenRegion);
            screenSession->EnableMirrorScreenRegion();
        }
    }
}

void ScreenSessionManager::NotifyDisplayModeChanged(FoldDisplayMode displayMode)
{
    NotifyClientProxyUpdateFoldDisplayMode(displayMode);
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER);
    TLOGI(WmsLogTag::DMS, "DisplayMode:%{public}d, agent size: %{public}u",
        displayMode, static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!IsFreezed(agentPid,
            DisplayManagerAgentType::DISPLAY_MODE_CHANGED_LISTENER)) {
            agent->NotifyDisplayModeChanged(displayMode);
        }
    }
    if (foldScreenController_ != nullptr && FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        auto screenIds = GetAllScreenIds();
        for (auto screenId : screenIds) {
            RefreshMirrorScreenRegion(screenId);
        }
    }
}

void ScreenSessionManager::NotifyScreenMagneticStateChanged(bool isMagneticState)
{
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::SCREEN_MAGNETIC_STATE_CHANGED_LISTENER);
    TLOGI(WmsLogTag::DMS, "IsScreenMagneticState:%{public}u, agent size: %{public}u",
        static_cast<uint32_t>(isMagneticState), static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!IsFreezed(agentPid,
            DisplayManagerAgentType::SCREEN_MAGNETIC_STATE_CHANGED_LISTENER)) {
            agent->NotifyScreenMagneticStateChanged(isMagneticState);
        }
    }
}

void ScreenSessionManager::SetDisplayNodeScreenId(ScreenId screenId, ScreenId displayNodeScreenId)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " displayNodeScreenId: %{public}" PRIu64,
        screenId, displayNodeScreenId);
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:SetDisplayNodeScreenId");
    if (!clientProxy_) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy_->SetDisplayNodeScreenId(screenId, displayNodeScreenId);
#ifdef DEVICE_STATUS_ENABLE
    SetDragWindowScreenId(screenId, displayNodeScreenId);
#endif // DEVICE_STATUS_ENABLE
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
    if (!clientProxy_) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy_->OnPropertyChanged(screenId, newProperty, reason);
}

void ScreenSessionManager::OnPowerStatusChange(DisplayPowerEvent event, EventStatus status,
    PowerStateChangeReason reason)
{
    TLOGI(WmsLogTag::DMS, "[UL_POWER]event: %{public}d, status: %{public}d, reason: %{public}d",
        static_cast<int>(event),
        static_cast<int>(status), static_cast<int>(reason));
    if (!clientProxy_) {
        TLOGI(WmsLogTag::DMS, "[UL_POWER] clientProxy_ is null");
        return;
    }
    clientProxy_->OnPowerStatusChanged(event, status, reason);
}

void ScreenSessionManager::OnSensorRotationChange(float sensorRotation, ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " sensorRotation: %{public}f", screenId, sensorRotation);
    if (!clientProxy_) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy_->OnSensorRotationChanged(screenId, sensorRotation);
}

void ScreenSessionManager::OnHoverStatusChange(int32_t hoverStatus, bool needRotate, ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " hoverStatus: %{public}d", screenId, hoverStatus);
    if (!clientProxy_) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy_->OnHoverStatusChanged(screenId, hoverStatus, needRotate);
}

void ScreenSessionManager::OnScreenOrientationChange(float screenOrientation, ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " screenOrientation: %{public}f", screenId, screenOrientation);
    if (!clientProxy_) {
        TLOGI(WmsLogTag::DMS, "ClientProxy_ is null");
        return;
    }
    clientProxy_->OnScreenOrientationChanged(screenId, screenOrientation);
}

void ScreenSessionManager::OnScreenRotationLockedChange(bool isLocked, ScreenId screenId)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 " isLocked: %{public}d", screenId, isLocked);
    if (!clientProxy_) {
        TLOGI(WmsLogTag::DMS, "ClientProxy_ is null");
        return;
    }
    clientProxy_->OnScreenRotationLockedChanged(screenId, isLocked);
}

void ScreenSessionManager::NotifyClientProxyUpdateFoldDisplayMode(FoldDisplayMode displayMode)
{
    if (clientProxy_) {
        TLOGI(WmsLogTag::DMS, "displayMode = %{public}d",
            static_cast<int>(displayMode));
        clientProxy_->OnUpdateFoldDisplayMode(displayMode);
    }
}

void ScreenSessionManager::ScbClientDeathCallback(int32_t deathScbPid)
{
    std::unique_lock<std::mutex> lock(oldScbPidsMutex_);
    if (deathScbPid == currentScbPId_ || currentScbPId_ == INVALID_SCB_PID) {
        clientProxy_ = nullptr;
        TLOGE(WmsLogTag::DMS, "death callback and set clientProxy null");
    }
    if (scbSwitchCV_.wait_for(lock, std::chrono::milliseconds(CV_WAIT_SCBSWITCH_MS))
        == std::cv_status::timeout) {
        TLOGE(WmsLogTag::DMS, "set client task deathScbPid:%{public}d, timeout: %{public}d",
            deathScbPid, CV_WAIT_SCBSWITCH_MS);
    }
    std::ostringstream oss;
    oss << "Scb client death: " << deathScbPid;
    TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    screenEventTracker_.RecordEvent(oss.str());
    oldScbPids_.erase(std::remove(oldScbPids_.begin(), oldScbPids_.end(), deathScbPid), oldScbPids_.end());
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
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DMS, "permission denied, clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    auto userId = GetUserIdByCallingUid();
    auto newScbPid = IPCSkeleton::GetCallingPid();
    SwitchScbNodeHandle(userId, newScbPid, false);
    MockSessionManagerService::GetInstance().NotifyWMSConnected(userId, GetDefaultScreenId(), false);
}

void ScreenSessionManager::ScbStatusRecoveryWhenSwitchUser(std::vector<int32_t> oldScbPids, int32_t newScbPid)
{
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
        if (foldStatus == FoldStatus::EXPAND || foldStatus == FoldStatus::HALF_FOLD ||
            FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
            if (FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
                screenSession->UpdatePropertyByFoldControl(screenSession->GetScreenProperty());
            } else {
                screenSession->UpdatePropertyByFoldControl(GetPhyScreenProperty(SCREEN_ID_FULL));
            }
            screenSession->PropertyChange(screenSession->GetScreenProperty(),
                ScreenPropertyChangeReason::FOLD_SCREEN_EXPAND);
        } else if (foldStatus == FoldStatus::FOLDED) {
            screenSession->UpdatePropertyByFoldControl(GetPhyScreenProperty(SCREEN_ID_MAIN));
            screenSession->PropertyChange(screenSession->GetScreenProperty(),
                ScreenPropertyChangeReason::FOLD_SCREEN_FOLDING);
        } else {
            TLOGE(WmsLogTag::DMS, "unsupport foldStatus: %{public}u", foldStatus);
        }
    } else {
        screenSession->UpdateValidRotationToScb();
    }
    auto task = [=] {
        clientProxy_->SwitchUserCallback(oldScbPids, newScbPid);
    };
    taskScheduler_->PostAsyncTask(task, "clientProxy_ SwitchUserCallback task", delayTime);
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
    clientProxy_ = client;
    auto userId = GetUserIdByCallingUid();
    auto newScbPid = IPCSkeleton::GetCallingPid();

    std::ostringstream oss;
    oss << "set client userId: " << userId
        << " clientName: " << SysCapUtil::GetClientName();
    TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    screenEventTracker_.RecordEvent(oss.str());

    MockSessionManagerService::GetInstance().NotifyWMSConnected(userId, GetDefaultScreenId(), true);
    NotifyClientProxyUpdateFoldDisplayMode(GetFoldDisplayMode());
    SetClientInner();
    SwitchScbNodeHandle(userId, newScbPid, true);
    RecoverMultiScreenMode();
    AddScbClientDeathRecipient(client, newScbPid);
}

void ScreenSessionManager::RecoverMultiScreenMode()
{
    if (!g_isPcDevice) {
        TLOGW(WmsLogTag::DMS, "not PC device");
        return;
    }
    if (!clientProxy_) {
        TLOGE(WmsLogTag::DMS, "clientProxy is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (const auto& iter : screenSessionMap_) {
        if (!iter.second) {
            continue;
        }
        if (iter.second->GetScreenProperty().GetScreenType() == ScreenType::REAL &&
            !iter.second->GetIsInternal() && iter.second->GetIsCurrentInUse()) {
            TLOGW(WmsLogTag::DMS, "found external screen, screenId: %{public}" PRIu64, iter.first);
            RecoverRestoredMultiScreenMode(iter.second);
            SetExtendedScreenFallbackPlan(iter.first);
        }
    }
}

void ScreenSessionManager::RecoverRestoredMultiScreenMode(sptr<ScreenSession> screenSession)
{
    MultiScreenInfo info;
    bool restored = CheckMultiScreenInfo(info, screenSession);
    if (!restored) {
        TLOGW(WmsLogTag::DMS, "no info, use default mode");
        clientProxy_->OnScreenConnectionChanged(screenSession->GetScreenId(), ScreenEvent::CONNECTED,
            screenSession->GetRSScreenId(), screenSession->GetName(), screenSession->GetIsExtend());
        return;
    }
    if (info.multiScreenMode == MultiScreenMode::SCREEN_MIRROR) {
        SetMultiScreenMode(info.mainScreenOption.screenId_, info.secondaryScreenOption.screenId_,
            info.multiScreenMode);
        TLOGW(WmsLogTag::DMS, "mirror, return befor OnScreenConnectionChanged");
        ReportHandleScreenEvent(ScreenEvent::CONNECTED, ScreenCombination::SCREEN_MIRROR);
        return;
    }
    ReportHandleScreenEvent(ScreenEvent::CONNECTED, ScreenCombination::SCREEN_EXTEND);
    clientProxy_->OnScreenConnectionChanged(screenSession->GetScreenId(), ScreenEvent::CONNECTED,
        screenSession->GetRSScreenId(), screenSession->GetName(), screenSession->GetIsExtend());
    auto ret = SetMultiScreenRelativePosition(info.mainScreenOption, info.secondaryScreenOption);
    if (ret != DMError::DM_OK) {
        SetMultiScreenDefaultRelativePosition();
    }
}

void ScreenSessionManager::SwitchScbNodeHandle(int32_t newUserId, int32_t newScbPid, bool coldBoot)
{
    std::ostringstream oss;
    oss << "currentUserId: " << currentUserId_
        << "  currentScbPId: " << currentScbPId_
        << "  newUserId: " << newUserId
        << "  newScbPid: " << newScbPid
        << "  coldBoot: " << static_cast<int32_t>(coldBoot);
    TLOGI(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    screenEventTracker_.RecordEvent(oss.str());

    std::unique_lock<std::mutex> lock(oldScbPidsMutex_);
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
    if (!clientProxy_) {
        TLOGE(WmsLogTag::DMS, "clientProxy is null");
        return;
    }
    if (coldBoot) {
        clientProxy_->SwitchUserCallback(oldScbPids_, newScbPid);
        clientProxyMap_[newUserId] = clientProxy_;
    } else {
        // hot switch
        if (clientProxyMap_.count(newUserId) == 0) {
            TLOGE(WmsLogTag::DMS, "not found client proxy. userId:%{public}d.", newUserId);
            return;
        }
        if (newUserId == currentUserId_) {
            TLOGI(WmsLogTag::DMS, "switch user not change");
            return;
        }
        clientProxy_ = clientProxyMap_[newUserId];
        ScbStatusRecoveryWhenSwitchUser(oldScbPids_, newScbPid);
    }
    UpdateDisplayScaleState(GetDefaultScreenId());
    currentUserId_ = newUserId;
    currentScbPId_ = newScbPid;
    scbSwitchCV_.notify_all();
    oldScbDisplayMode_ = GetFoldDisplayMode();
}

int32_t ScreenSessionManager::GetCurrentUserId()
{
    return currentUserId_;
}

void ScreenSessionManager::SetClientInner()
{
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (const auto& iter : screenSessionMap_) {
        if (!iter.second) {
            continue;
        }
        // In the rotating state, after scb restarts, the screen information needs to be reset.
        float phyWidth = 0.0f;
        float phyHeight = 0.0f;
        bool isReset = true;
        GetCurrentScreenPhyBounds(phyWidth, phyHeight, isReset, iter.first);
        auto localRotation = iter.second->GetRotation();
        TLOGI(WmsLogTag::DMS, "phyWidth = :%{public}f, phyHeight = :%{public}f, localRotation = :%{public}u",
            phyWidth, phyHeight, localRotation);
        bool isModeChanged = localRotation != Rotation::ROTATION_0;
        if (isModeChanged && isReset) {
            TLOGI(WmsLogTag::DMS, "screen(id:%{public}" PRIu64 ") current is not default mode, reset it", iter.first);
            SetRotation(iter.first, Rotation::ROTATION_0, false);
            iter.second->SetDisplayBoundary(RectF(0, 0, phyWidth, phyHeight), 0);
        }
        if (!clientProxy_) {
            TLOGE(WmsLogTag::DMS, "clientProxy is null");
            return;
        }
        if (iter.second->GetScreenCombination() == ScreenCombination::SCREEN_MIRROR) {
            TLOGI(WmsLogTag::DMS, "current screen is extend and mirror, return before OnScreenConnectionChanged");
            continue;
        }
        if (g_isPcDevice && g_outerOnly == ONLY_OUTER_SCREEN_VALUE && !iter.second->GetIsExtend()) {
            defaultScreenId_ = iter.first;
        }
        if (g_isPcDevice && iter.second->GetScreenCombination() != ScreenCombination::SCREEN_MAIN) {
            TLOGI(WmsLogTag::DMS, "PC device, return before OnScreenConnectionChanged");
            continue;
        }
        clientProxy_->OnScreenConnectionChanged(iter.first, ScreenEvent::CONNECTED,
            iter.second->GetRSScreenId(), iter.second->GetName(), iter.second->GetIsExtend());
    }
}

void ScreenSessionManager::GetCurrentScreenPhyBounds(float& phyWidth, float& phyHeight,
                                                     bool& isReset, const ScreenId& screenid)
{
#ifdef FOLD_ABILITY_ENABLE
    if (foldScreenController_ != nullptr) {
        FoldDisplayMode displayMode = GetFoldDisplayMode();
        TLOGI(WmsLogTag::DMS, "fold screen with displayMode = %{public}u", displayMode);
        if (FoldScreenStateInternel::IsDualDisplayFoldDevice()) {
            auto phyBounds = GetPhyScreenProperty(screenid).GetPhyBounds();
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
        } else if (displayMode == FoldDisplayMode::FULL) {
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
    auto remoteScreenMode = rsInterface_.GetScreenActiveMode(screenid);
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    phyWidth = remoteScreenMode.GetScreenWidth();
    phyHeight = remoteScreenMode.GetScreenHeight();
}

ScreenProperty ScreenSessionManager::GetScreenProperty(ScreenId screenId)
{
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
    dumper->DumpFreezedPidList(freezedPidList_);
    dumper->DumpEventTracker(screenEventTracker_);
    dumper->DumpMultiUserInfo(oldScbPids_, currentUserId_, currentScbPId_);
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
    auto agents = dmAgentContainer_.GetAgentsByType(DisplayManagerAgentType::AVAILABLE_AREA_CHANGED_LISTENER);
    TLOGI(WmsLogTag::DMS, "entry, agent size: %{public}u", static_cast<uint32_t>(agents.size()));
    if (agents.empty()) {
        return;
    }
    for (auto& agent : agents) {
        int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
        if (!IsFreezed(agentPid,
            DisplayManagerAgentType::AVAILABLE_AREA_CHANGED_LISTENER)) {
            agent->NotifyAvailableAreaChanged(area, displayId);
        }
    }
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

    auto screenSession = GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "can not get default screen now");
        return;
    }
    if (!screenSession->UpdateAvailableArea(area)) {
        return;
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
        NotifyAvailableAreaChanged(bArea, screenId);
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
    /* Avoid fold to expand process queues */
    if (foldScreenController_ != nullptr) {
        foldScreenController_->SetdisplayModeChangeStatus(false);
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
    TLOGW(WmsLogTag::DMS, "%{public}s", description.c_str());
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
        auto agents = dmAgentContainer_.GetAgentsByType(agentType);
        for (auto agent : agents) {
            int32_t agentPid = dmAgentContainer_.GetAgentPid(agent);
            if (agentPid != pid || unfreezedPidList.count(pid) == 0) {
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
    for (auto iter = pidAgentTypeMap_.begin(); iter != pidAgentTypeMap_.end();) {
        int32_t pid = iter->first;
        auto pidAgentTypes = iter->second;
        NotifyUnfreezedAgents(pid, unfreezedPidList, pidAgentTypes, screenSession);
        if (pidAgentTypeMap_[pid].empty()) {
            iter = pidAgentTypeMap_.erase(iter);
        } else {
            iter++;
        }
    }
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
    if (!FoldScreenStateInternel::IsSecondaryDisplayFoldDevice()) {
        FoldScreenSensorManager::GetInstance().RegisterApplicationStateObserver();
    }
    IPCSkeleton::SetCallingIdentity(identify);
#endif
}

void ScreenSessionManager::SetVirtualScreenBlackList(ScreenId screenId, std::vector<uint64_t>& windowIdList,
    std::vector<uint64_t> surfaceIdList)
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
    if (windowIdList.empty()) {
        TLOGI(WmsLogTag::DMS, "WindowIdList is empty");
        rsInterface_.SetVirtualScreenBlackList(rsScreenId, surfaceIdList);
        return;
    }
    if (!clientProxy_) {
        TLOGE(WmsLogTag::DMS, "clientProxy_ is nullptr");
        return;
    }
    std::vector<uint64_t> surfaceNodeIdsToRS;
    clientProxy_->OnGetSurfaceNodeIdsFromMissionIdsChanged(windowIdList, surfaceNodeIdsToRS, true);
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
    oss << "surfaceNodeIdsToRS: ";
    for (auto val : surfaceNodeIdsToRS) {
        oss << val << " ";
    }
    TLOGW(WmsLogTag::DMS, "%{public}s", oss.str().c_str());
    rsInterface_.SetVirtualScreenBlackList(rsScreenId, surfaceNodeIdsToRS);
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
    if (clientProxy_) {
        auto screenInfo = GetDefaultScreenSession();
        int32_t rotation = -1;
        if (screenInfo != nullptr) {
            rotation = static_cast<int32_t>(screenInfo->GetRotation());
        }
        screenFoldInfo.emplace_back(std::to_string(rotation));

        clientProxy_->OnFoldStatusChangedReportUE(screenFoldInfo);
    }
}

std::vector<DisplayPhysicalResolution> ScreenSessionManager::GetAllDisplayPhysicalResolution()
{
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
    std::vector<DMRect> creaseRects = screenProperty.GetCreaseRects();
    if (creaseRects.size() > 0) {
        fakeScreenHeight = screenHeight - (creaseRects[0].posY_ + creaseRects[0].height_);
    }
    fakeScreenSession->UpdatePropertyByResolution(screenWidth, fakeScreenHeight);
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
    return clientProxy_;
}

DMError ScreenSessionManager::SetMultiScreenMode(ScreenId mainScreenId, ScreenId secondaryScreenId,
    MultiScreenMode screenMode)
{
    TLOGW(WmsLogTag::DMS, "mainScreenId:%{public}" PRIu64",secondaryScreenId:%{public}" PRIu64",Mode:%{public}u",
        mainScreenId, secondaryScreenId, screenMode);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
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
    NotifyScreenModeChange();
    return DMError::DM_OK;
}

DMError ScreenSessionManager::SetMultiScreenRelativePosition(MultiScreenPositionOptions mainScreenOptions,
    MultiScreenPositionOptions secondScreenOption)
{
    TLOGI(WmsLogTag::DMS,
        "mID:%{public}" PRIu64", X:%{public}u, Y:%{public}u,sID:%{public}" PRIu64", X:%{public}u, Y:%{public}u",
        mainScreenOptions.screenId_, mainScreenOptions.startX_, mainScreenOptions.startY_,
        secondScreenOption.screenId_, secondScreenOption.startX_, secondScreenOption.startY_);
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return DMError::DM_ERROR_NOT_SYSTEM_APP;
    }
    sptr<ScreenSession> firstScreenSession = GetScreenSession(mainScreenOptions.screenId_);
    sptr<ScreenSession> secondScreenSession = GetScreenSession(secondScreenOption.screenId_);
    if (!firstScreenSession || !secondScreenSession) {
        TLOGE(WmsLogTag::DMS, "ScreenSession is null");
        return DMError::DM_ERROR_NULLPTR;
    }
    if (!MultiScreenManager::GetInstance().AreScreensTouching(firstScreenSession, secondScreenSession,
        mainScreenOptions, secondScreenOption)) {
        TLOGE(WmsLogTag::DMS, "Options incorrect!");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    firstScreenSession->SetStartPosition(mainScreenOptions.startX_, mainScreenOptions.startY_);
    firstScreenSession->PropertyChange(firstScreenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE);
    secondScreenSession->SetStartPosition(secondScreenOption.startX_, secondScreenOption.startY_);
    secondScreenSession->PropertyChange(secondScreenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE);
    std::shared_ptr<RSDisplayNode> firstDisplayNode = firstScreenSession->GetDisplayNode();
    std::shared_ptr<RSDisplayNode> secondDisplayNode = secondScreenSession->GetDisplayNode();
    if (firstDisplayNode && secondDisplayNode) {
        firstDisplayNode->SetDisplayOffset(mainScreenOptions.startX_, mainScreenOptions.startY_);
        secondDisplayNode->SetDisplayOffset(secondScreenOption.startX_, secondScreenOption.startY_);
    } else {
        TLOGW(WmsLogTag::DMS, "DisplayNode is null");
    }
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        TLOGI(WmsLogTag::DMS, "free displayNode");
        transactionProxy->FlushImplicitTransaction();
    }
    return DMError::DM_OK;
}

void ScreenSessionManager::SetRelativePositionForDisconnect(MultiScreenPositionOptions defaultScreenOptions)
{
    TLOGI(WmsLogTag::DMS, "mID:%{public}" PRIu64", X:%{public}u, Y:%{public}u",
        defaultScreenOptions.screenId_, defaultScreenOptions.startX_, defaultScreenOptions.startY_);
    sptr<ScreenSession> defaultScreenSession = GetScreenSession(defaultScreenOptions.screenId_);
    if (!defaultScreenSession) {
        TLOGE(WmsLogTag::DMS, "ScreenSession is null");
        return;
    }
    defaultScreenSession->SetStartPosition(defaultScreenOptions.startX_, defaultScreenOptions.startY_);
    defaultScreenSession->PropertyChange(defaultScreenSession->GetScreenProperty(),
        ScreenPropertyChangeReason::RELATIVE_POSITION_CHANGE);
    std::shared_ptr<RSDisplayNode> defaultDisplayNode = defaultScreenSession->GetDisplayNode();
    if (defaultDisplayNode) {
        defaultDisplayNode->SetDisplayOffset(defaultScreenOptions.startX_, defaultScreenOptions.startY_);
    } else {
        TLOGW(WmsLogTag::DMS, "DisplayNode is null");
    }
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        TLOGI(WmsLogTag::DMS, "free displayNode");
        transactionProxy->FlushImplicitTransaction();
    }
}

void ScreenSessionManager::MultiScreenModeChange(ScreenId mainScreenId, ScreenId secondaryScreenId,
    const std::string& operateMode)
{
    TLOGW(WmsLogTag::DMS, "mainId=%{public}" PRIu64" secondId=%{public}" PRIu64" operateType: %{public}s",
        mainScreenId, secondaryScreenId, operateMode.c_str());
    sptr<ScreenSession> firstSession = nullptr;
    sptr<ScreenSession> secondarySession = nullptr;
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
            if (sessionIt.first == mainScreenId) {
                firstSession = screenSession;
            }
            if (sessionIt.first == secondaryScreenId) {
                secondarySession = screenSession;
            }
        }
    }

    if (firstSession != nullptr && secondarySession != nullptr) {
        ScreenCombination firstCombination = firstSession->GetScreenCombination();
        ScreenCombination secondaryCombination = secondarySession->GetScreenCombination();
        MultiScreenManager::GetInstance().MultiScreenModeChange(firstSession, secondarySession, operateMode);
        if ((firstCombination == ScreenCombination::SCREEN_MIRROR ||
            secondaryCombination == ScreenCombination::SCREEN_MIRROR) &&
            operateMode == SCREEN_EXTEND) {
            MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_MIRROR, MULTI_SCREEN_EXIT_STR);
            MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_EXTEND, MULTI_SCREEN_ENTER_STR);
        } else if ((firstCombination == ScreenCombination::SCREEN_EXTEND ||
            secondaryCombination == ScreenCombination::SCREEN_EXTEND) &&
            operateMode == SCREEN_MIRROR) {
            MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_EXTEND, MULTI_SCREEN_EXIT_STR);
            MultiScreenManager::GetInstance().MultiScreenReportDataToRss(SCREEN_MIRROR, MULTI_SCREEN_ENTER_STR);
        }
    } else {
        TLOGE(WmsLogTag::DMS, "params error");
    }
}

void ScreenSessionManager::SwitchScrollParam(FoldDisplayMode displayMode)
{
    std::map<FoldDisplayMode, ScrollableParam> scrollableParams = ScreenSceneConfig::GetAllScrollableParam();
    std::string srollVelocityScale = scrollableParams.count(displayMode) != 0 ?
        scrollableParams[displayMode].velocityScale_ : "0";
    std::string srollFriction = scrollableParams.count(displayMode) != 0 ?
        scrollableParams[displayMode].friction_ : "0";
    system::SetParameter("persist.scrollable.velocityScale", srollVelocityScale);
    system::SetParameter("persist.scrollable.friction", srollFriction);
}

void ScreenSessionManager::MultiScreenModeChange(const std::string& firstScreenIdStr,
    const std::string& secondaryScreenIdStr, const std::string& secondaryChandeMode)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! clientName: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    ScreenId firstScreenId = static_cast<ScreenId>(std::stoi(firstScreenIdStr));
    ScreenId secondaryScreenId = static_cast<ScreenId>(std::stoi(secondaryScreenIdStr));
    if (secondaryChandeMode == "mirror" || secondaryChandeMode == "extend") {
        MultiScreenModeChange(firstScreenId, secondaryScreenId, secondaryChandeMode);
    } else {
        TLOGE(WmsLogTag::DMS, "dumper params error");
    }
}

void ScreenSessionManager::OnScreenExtendChange(ScreenId mainScreenId, ScreenId extendScreenId)
{
    if (!clientProxy_) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy_->OnScreenExtendChanged(mainScreenId, extendScreenId);
}

void ScreenSessionManager::OnTentModeChanged(bool isTentMode)
{
#ifdef FOLD_ABILITY_ENABLE
    if (!foldScreenController_) {
        TLOGI(WmsLogTag::DMS, "foldScreenController_ is null");
        return;
    }
    foldScreenController_->OnTentModeChanged(isTentMode);
#endif
}

void ScreenSessionManager::SetCoordinationFlag(bool isCoordinationFlag)
{
    TLOGI(WmsLogTag::DMS, "set coordination flag %{public}d", isCoordinationFlag);
    isCoordinationFlag_ = isCoordinationFlag;
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
    return DMError::DM_OK;
}

void ScreenSessionManager::OnScreenCaptureNotify(ScreenId mainScreenId, int32_t uid, const std::string& clientName)
{
    if (!clientProxy_) {
        TLOGI(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy_->ScreenCaptureNotify(mainScreenId, uid, clientName);
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
    if (errorCode == nullptr) {
        TLOGE(WmsLogTag::DMS, "param is null.");
        return nullptr;
    }
    if (system::GetBoolParameter("persist.edm.disallow_screenshot", false)) {
        TLOGW(WmsLogTag::DMS, "capture disabled by edm!");
        *errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
        return nullptr;
    }
    if (!ScreenSceneConfig::IsSupportCapture()) {
        TLOGW(WmsLogTag::DMS, "device not support capture.");
        *errorCode = DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT;
        return nullptr;
    }
    if (!Permission::CheckCallingPermission(CUSTOM_SCREEN_CAPTURE_PERMISSION) && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::DMS, "Permission Denied! clientName: %{public}s, pid: %{public}d.",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingRealPid());
        *errorCode = DmErrorCode::DM_ERROR_NO_PERMISSION;
        return nullptr;
    }
    if (captureOption.displayId_ == DISPLAY_ID_INVALID ||
        (captureOption.displayId_ == DISPLAY_ID_FAKE && !IsFakeDisplayExist())) {
        TLOGE(WmsLogTag::DMS, "display id invalid.");
        *errorCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        return nullptr;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetScreenCapture(%" PRIu64")", captureOption.displayId_);
    auto res = GetScreenSnapshot(captureOption.displayId_, false);
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

sptr<DisplayInfo> ScreenSessionManager::GetPrimaryDisplayInfo()
{
    DmsXcollie dmsXcollie("DMS:GetPrimaryDisplayInfo", XCOLLIE_TIMEOUT_10S);
    sptr<ScreenSession> screenSession = nullptr;
    {
        std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
        for (auto sessionIt : screenSessionMap_) {
            screenSession = sessionIt.second;
            if (screenSession == nullptr) {
                TLOGE(WmsLogTag::DMS, "screenSession is nullptr!");
                continue;
            }
            if (!screenSession->GetIsExtend()) {
                TLOGE(WmsLogTag::DMS, "find primary %{public}" PRIu64, screenSession->screenId_);
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

void ScreenSessionManager::OnSuperFoldStatusChange(ScreenId screenId, SuperFoldStatus superFoldStatus)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 ", superFoldStatus: %{public}d", screenId,
        static_cast<uint32_t>(superFoldStatus));
    if (!clientProxy_) {
        TLOGE(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy_->OnSuperFoldStatusChanged(screenId, superFoldStatus);
}

void ScreenSessionManager::OnSecondaryReflexionChange(ScreenId screenId, bool isSecondaryReflexion)
{
    TLOGI(WmsLogTag::DMS, "screenId: %{public}" PRIu64 ", isSecondaryReflexion: %{public}d", screenId,
        isSecondaryReflexion);
    if (!clientProxy_) {
        TLOGE(WmsLogTag::DMS, "clientProxy_ is null");
        return;
    }
    clientProxy_->OnSecondaryReflexionChanged(screenId, isSecondaryReflexion);
}

int32_t ScreenSessionManager::GetCameraStatus()
{
    return cameraStatus_;
}

int32_t ScreenSessionManager::GetCameraPosition()
{
    return cameraPosition_;
}

bool ScreenSessionManager::IsScreenCasting()
{
    if (virtualScreenCount_ > 0 || hdmiScreenCount_ > 0) {
        TLOGI(WmsLogTag::DMS, "virtualScreenCount_: %{public}" PRIu32 ", hdmiScreenCount_: %{public}d",
            virtualScreenCount_, hdmiScreenCount_);
        return true;
    }
    TLOGI(WmsLogTag::DMS, "not casting");
    return false;
}

void ScreenSessionManager::SetMultiScreenOuterMode(sptr<ScreenSession>& innerSession,
    sptr<ScreenSession>& outerSession)
{
    std::string screenMode = "";
    if (clientProxy_ == nullptr) {
        TLOGE(WmsLogTag::DMS, "client proxy is null.");
        return;
    }
    TLOGI(WmsLogTag::DMS, "switch outer start");
    ScreenCombination outerCombination = outerSession->GetScreenCombination();
    if (outerCombination != ScreenCombination::SCREEN_MIRROR && outerCombination != ScreenCombination::SCREEN_EXTEND) {
        TLOGE(WmsLogTag::DMS, "outer mode error.");
        return;
    }
    screenMode = outerCombination == ScreenCombination::SCREEN_MIRROR ? SCREEN_MIRROR : SCREEN_EXTEND;
    MultiScreenManager::GetInstance().MultiScreenModeChange(outerSession, innerSession, screenMode);
    clientProxy_->OnScreenConnectionChanged(innerSession->GetScreenId(), ScreenEvent::DISCONNECTED,
        innerSession->GetRSScreenId(), innerSession->GetName(), innerSession->GetIsExtend());
    {
        std::shared_ptr<RSDisplayNode> displayNode = innerSession->GetDisplayNode();
        if (displayNode != nullptr) {
            displayNode->RemoveFromTree();
            innerSession->ReleaseDisplayNode();
        }
        displayNode = nullptr;
    }
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        TLOGI(WmsLogTag::DMS, "free displayNode");
        transactionProxy->FlushImplicitTransaction();
    }
    ScreenSettingHelper::SetSettingValue("only_second_screen_info", screenMode);
    CallRsSetScreenPowerStatusSync(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_OFF);
    TLOGI(WmsLogTag::DMS, "switch out end.");
}

void ScreenSessionManager::RecoveryMultiScreenNormalMode(sptr<ScreenSession>& innerSession,
    sptr<ScreenSession>& outerSession)
{
    std::string screenMode = "";
    TLOGI(WmsLogTag::DMS, "recovery normal mode.");
    ScreenSettingHelper::GetSettingValue("only_second_screen_info", screenMode);
    if (clientProxy_ == nullptr || screenMode == "") {
        TLOGE(WmsLogTag::DMS, "client proxy is null.");
        return;
    }
    Rosen::RSDisplayNodeConfig rsConfig;
    rsConfig = {.screenId = innerSession->GetScreenId()};
    innerSession->CreateDisplayNode(rsConfig);
    innerSession->SetIsExtend(false);
    outerSession->SetIsExtend(true);
    CallRsSetScreenPowerStatusSync(SCREEN_ID_FULL, ScreenPowerStatus::POWER_STATUS_ON);
    clientProxy_->OnScreenConnectionChanged(outerSession->GetScreenId(), ScreenEvent::DISCONNECTED,
        outerSession->GetRSScreenId(), outerSession->GetName(), outerSession->GetIsExtend());
    int64_t timeStamp = 50;  // wait notify
    std::this_thread::sleep_for(std::chrono::milliseconds(timeStamp));
    if (screenMode == SCREEN_MIRROR) {
        NodeId nodeId = innerSession->GetDisplayNode()->GetId();
        rsConfig = {.screenId = outerSession->GetScreenId(), .isMirrored = true, .mirrorNodeId = nodeId};
        outerSession->CreateDisplayNode(rsConfig);
        outerSession->SetScreenCombination(ScreenCombination::SCREEN_MIRROR);
    } else if (screenMode == SCREEN_EXTEND) {
        rsConfig = {.screenId = outerSession->GetScreenId()};
        outerSession->CreateDisplayNode(rsConfig);
        outerSession->SetScreenCombination(ScreenCombination::SCREEN_EXTEND);
    }
    innerSession->SetScreenCombination(ScreenCombination::SCREEN_MAIN);
    clientProxy_->OnScreenConnectionChanged(innerSession->GetScreenId(), ScreenEvent::CONNECTED,
        innerSession->GetRSScreenId(), innerSession->GetName(), innerSession->GetIsExtend());
    std::this_thread::sleep_for(std::chrono::milliseconds(timeStamp));
    if (screenMode == SCREEN_EXTEND) {
        clientProxy_->OnScreenConnectionChanged(outerSession->GetScreenId(), ScreenEvent::CONNECTED,
            outerSession->GetRSScreenId(), outerSession->GetName(), outerSession->GetIsExtend());
    }
    ScreenSettingHelper::SetSettingValue("only_second_screen_info", "");
    TLOGI(WmsLogTag::DMS, "recovery normal mode end.");
}

void ScreenSessionManager::MultiScreenChangeOuter(const std::string& outerFlag)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::DMS, "permission denied! calling: %{public}s, pid: %{public}d",
            SysCapUtil::GetClientName().c_str(), IPCSkeleton::GetCallingPid());
        return;
    }
    if (!clientProxy_ || (outerFlag != "1" && outerFlag != "0")) {
        TLOGE(WmsLogTag::DMS, "change params error.");
        return;
    }
    sptr<ScreenSession> innerSession = nullptr;
    sptr<ScreenSession> outerSession = nullptr;
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
            if (screenSession->GetIsInternal()) {
                innerSession = screenSession;
            } else {
                outerSession = screenSession;
            }
        }
    }
    if (innerSession == nullptr || outerSession == nullptr) {
        TLOGE(WmsLogTag::DMS, "innerSession or outerSession is nullptr.");
        return;
    }
    if (outerFlag == "1") {
        SetMultiScreenOuterMode(innerSession, outerSession);
    } else {
        RecoveryMultiScreenNormalMode(innerSession, outerSession);
    }
    FixPowerStatus();
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
                screenSession->SetShareProtect(isEnable);
            }
        }
    }
    SetScreenSkipProtectedWindowInner();
    return DMError::DM_OK;
}

void ScreenSessionManager::SetScreenSkipProtectedWindowInner()
{
    TLOGI(WmsLogTag::DMS, "enter");
    bool screenSkipProtectedWindowValue = false;
    bool ret = ScreenSettingHelper::GetSettingscreenSkipProtectedWindow(screenSkipProtectedWindowValue);
    if (!ret) {
        TLOGE(WmsLogTag::DMS, "get setting failed, default value false");
    }
    std::lock_guard<std::recursive_mutex> lock(screenSessionMapMutex_);
    for (auto sessionIt : screenSessionMap_) {
        auto screenSession = sessionIt.second;
        if (screenSession == nullptr) {
            TLOGE(WmsLogTag::DMS, "screenSession is nullptr, ScreenId:%{public}" PRIu64"",
                sessionIt.first);
            continue;
        }
        if (screenSession->GetScreenProperty().GetScreenType() == ScreenType::VIRTUAL) {
            ScreenId rsScreenId;
            if (!screenIdManager_.ConvertToRsScreenId(screenSession->GetScreenId(), rsScreenId)) {
                TLOGE(WmsLogTag::DMS, "No corresponding rsId.");
                continue;
            }
            bool requiredSkipWindow = screenSession->GetShareProtect() && screenSkipProtectedWindowValue;
            TLOGI(WmsLogTag::DMS, "virtualScreenId:%{public}" PRIu64 " requiredSkipWindow:%{public}d",
                sessionIt.first, requiredSkipWindow);
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
                "SetCastScreenEnableSkipWindow(%" PRIu64")", sessionIt.first);
            rsInterface_.SetCastScreenEnableSkipWindow(rsScreenId, requiredSkipWindow);
        }
    }
}

bool ScreenSessionManager::IsOrientationNeedChanged()
{
    if (GetFoldDisplayMode() == FoldDisplayMode::GLOBAL_FULL) {
        return true;
    }
    if (FoldScreenStateInternel::IsSingleDisplayPocketFoldDevice() &&
        GetFoldDisplayMode() == FoldDisplayMode::MAIN &&
        !GetTentMode() && !GetCameraMode()) {
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
} // namespace OHOS::Rosen
