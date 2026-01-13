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

#include "session_manager/include/scene_session_manager.h"

#include <regex>
#include <string>
#include <sys/stat.h>

#include <ability_context.h>
#include <ability_manager_client.h>
#include <application_context.h>
#include <bundlemgr/launcher_service.h>
#include <common/rs_common_def.h>
#include <hisysevent.h>
#include <hitrace_meter.h>
#include <parameters.h>
#include <ui/rs_node.h>
#include "hitrace/hitracechain.h"
#include "ffrt_serial_queue_helper.h"
#include "parameter.h"
#include "publish/scb_dump_subscriber.h"
#include "resource_manager.h"
#include "session/host/include/pc_fold_screen_manager.h"
#include "wm_common.h"

#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
#include <display_power_mgr_client.h>
#include <display_brightness_listener_stub.h>
#endif

#ifdef POWER_MANAGER_ENABLE
#include <power_mgr_client.h>
#endif

#ifdef RES_SCHED_ENABLE
#include "res_type.h"
#include "res_sched_client.h"
#endif

#include "anomaly_detection.h"
#include "color_parser.h"
#include "common/include/fold_screen_state_internel.h"
#include "common/include/session_permission.h"
#include "display_manager.h"
#ifdef WINDOW_MANAGER_FEATURE_SUPPORT_DMSFWK
#include "distributed_client.h"
#endif
#include "dms_reporter.h"
#include "hidump_controller.h"
#include "image_source.h"
#include "perform_reporter.h"
#include "rdb/scope_guard.h"
#include "rdb/starting_window_rdb_manager.h"
#include "res_sched_client.h"
#include "rs_adapter.h"
#include "scene_input_manager.h"
#include "scene_screen_change_listener.h"
#include "scene_system_ability_listener.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/host/include/ability_info_manager.h"
#include "session/host/include/main_session.h"
#include "session/host/include/move_drag_controller.h"
#include "session/host/include/multi_instance_manager.h"
#include "session/host/include/scb_system_session.h"
#include "session/host/include/scene_persistent_storage.h"
#include "session/host/include/session_change_recorder.h"
#include "session/host/include/session_utils.h"
#include "session/host/include/sub_session.h"
#include "session/host/include/ws_snapshot_helper.h"
#include "session_helper.h"
#include "session_manager_agent_controller.h"
#include "singleton_container.h"
#ifdef WINDOW_MANAGER_FEATURE_SUPPORT_DSOFTBUS
#include "softbus_bus_center.h"
#endif
#include "user_switch_reporter.h"
#include "window_helper.h"
#include "xcollie/watchdog.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#include "session_permission.h"
#include "sys_mgr_client.h"
#include "get_snapshot_callback.h"
#include "image_type.h"

#include "ui_effect_manager.h"
#ifdef MEMMGR_WINDOW_ENABLE
#include "mem_mgr_client.h"
#include "mem_mgr_window_info.h"
#endif

#ifdef SECURITY_COMPONENT_MANAGER_ENABLE
#include "sec_comp_enhance_kit.h"
#endif

#ifdef IMF_ENABLE
#include <input_method_controller.h>
#endif // IMF_ENABLE
#include "dms_global_mutex.h"


namespace OHOS::Rosen {
namespace {
const std::string SCENE_BOARD_BUNDLE_NAME = "com.ohos.sceneboard";
const std::string SCENE_BOARD_APP_IDENTIFIER = "";
const std::string SCENE_SESSION_MANAGER_THREAD = "OS_SceneSessionManager";
const std::string WINDOW_INFO_REPORT_THREAD = "OS_WindowInfoReportThread";
const std::string SUPER_LAUNCHER = "SuperLauncher";
constexpr const char* PREPARE_TERMINATE_ENABLE_PARAMETER = "persist.sys.prepare_terminate";
constexpr const char* ATOMIC_SERVICE_SESSION_ID = "com.ohos.param.sessionId";
constexpr uint32_t MAX_BRIGHTNESS = 255;
constexpr int32_t PREPARE_TERMINATE_ENABLE_SIZE = 6;
constexpr int32_t SCALE_DIMENSION = 2;
constexpr int32_t TRANSLATE_DIMENSION = 2;
constexpr int32_t ROTAION_DIMENSION = 4;
constexpr int32_t CURVE_PARAM_DIMENSION = 4;
const std::string DM_PKG_NAME = "ohos.distributedhardware.devicemanager";
constexpr int32_t NON_ANONYMIZE_LENGTH = 6;
const std::string EMPTY_DEVICE_ID = "";
const std::string STARTWINDOW_TYPE = "startWindowType";
const std::string STARTWINDOW_COLOR_MODE_TYPE = "startWindowColorModeType";
const int32_t MAX_NUMBER_OF_DISTRIBUTED_SESSIONS = 20;
const int32_t MAX_SESSION_LIMIT_ALL_APP = 512;

constexpr int WINDOW_NAME_MAX_WIDTH = 21;
constexpr int DISPLAY_NAME_MAX_WIDTH = 10;
constexpr int VALUE_MAX_WIDTH = 5;
constexpr int MAX_RESEND_TIMES = 6;
constexpr int ORIEN_MAX_WIDTH = 12;
constexpr int OFFSET_MAX_WIDTH = 8;
constexpr int SCALE_MAX_WIDTH = 8;
constexpr int PID_MAX_WIDTH = 8;
constexpr int PARENT_ID_MAX_WIDTH = 6;
constexpr int WINDOW_NAME_MAX_LENGTH = 20;
constexpr int32_t CANCEL_POINTER_ID = 99999999;
constexpr int32_t STATUS_BAR_AVOID_AREA = 0;
const std::string ARG_DUMP_ALL = "-a";
const std::string ARG_DUMP_WINDOW = "-w";
const std::string ARG_DUMP_SCREEN = "-s";
const std::string ARG_DUMP_DISPLAY = "-d";
const std::string ARG_DUMP_PIPLINE = "-p";
const std::string ARG_DUMP_SCB = "-b";
const std::string ARG_DUMP_DETAIL = "-c";
const std::string ARG_DUMP_RECORD = "-v";
constexpr uint64_t NANO_SECOND_PER_SEC = 1000000000; // ns
const int32_t LOGICAL_DISPLACEMENT_32 = 32;
constexpr int32_t GET_TOP_WINDOW_DELAY = 100;
constexpr char SMALL_FOLD_PRODUCT_TYPE = '2';
constexpr uint64_t VIRTUAL_DISPLAY_ID = 999;
constexpr uint32_t DEFAULT_LOCK_SCREEN_ZORDER = 2000;
constexpr int32_t MAX_LOCK_STATUS_CACHE_SIZE = 1000;
constexpr std::size_t MAX_SNAPSHOT_IN_RECENT_PC = 50;
constexpr std::size_t MAX_SNAPSHOT_IN_RECENT_PAD = 0;
constexpr std::size_t MAX_SNAPSHOT_IN_RECENT_PHONE = 0;
constexpr uint64_t NOTIFY_START_ABILITY_TIMEOUT = 4000;
constexpr uint64_t START_UI_ABILITY_TIMEOUT = 5000;
constexpr int32_t FORCE_SPLIT_MODE = 5;
constexpr int32_t NAV_FORCE_SPLIT_MODE = 6;
const std::string FB_PANEL_NAME = "Fb_panel";
constexpr std::size_t MAX_APP_BOUND_TRAY_MAP_SIZE = 50;
constexpr int32_t RS_CMD_BLOCKING_TIMEOUT_MS = 50;

constexpr int32_t FLUSH_WINDOW_INFO_MAX_COUNT = 3;
constexpr int32_t FLUSH_WINDOW_INFO_DELAY_INTERVAL = 2000;
constexpr int32_t FLUSH_WINDOW_INFO_START_DELAY_INTERVAL = 1000;
const std::string FLUSH_WINDOW_INFO_TASK_NAME = "DelayedFlushWindowInfoToMMITask";

const std::map<std::string, OHOS::AppExecFwk::DisplayOrientation> STRING_TO_DISPLAY_ORIENTATION_MAP = {
    {"unspecified",                         OHOS::AppExecFwk::DisplayOrientation::UNSPECIFIED},
    {"landscape",                           OHOS::AppExecFwk::DisplayOrientation::LANDSCAPE},
    {"portrait",                            OHOS::AppExecFwk::DisplayOrientation::PORTRAIT},
    {"follow_recent",                       OHOS::AppExecFwk::DisplayOrientation::FOLLOWRECENT},
    {"landscape_inverted",                  OHOS::AppExecFwk::DisplayOrientation::LANDSCAPE_INVERTED},
    {"portrait_inverted",                   OHOS::AppExecFwk::DisplayOrientation::PORTRAIT_INVERTED},
    {"auto_rotation",                       OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION},
    {"auto_rotation_landscape",             OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION_LANDSCAPE},
    {"auto_rotation_portrait",              OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION_PORTRAIT},
    {"auto_rotation_restricted",            OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION_RESTRICTED},
    {"auto_rotation_landscape_restricted",  OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED},
    {"auto_rotation_portrait_restricted",   OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION_PORTRAIT_RESTRICTED},
    {"locked",                              OHOS::AppExecFwk::DisplayOrientation::LOCKED},
    {"auto_rotation_unspecified",           OHOS::AppExecFwk::DisplayOrientation::AUTO_ROTATION_UNSPECIFIED},
    {"follow_desktop",                      OHOS::AppExecFwk::DisplayOrientation::FOLLOW_DESKTOP},
};

const std::map<std::string, StartWindowType> CONVERT_STRING_TO_START_WINDOW_TYPE_MAP = {
    {"REQUIRED_SHOW", StartWindowType::DEFAULT},
    {"REQUIRED_HIDE", StartWindowType::RETAIN_AND_INVISIBLE},
    {"OPTIONAL_SHOW", StartWindowType::DEFAULT},
};

const std::unordered_set<std::string> LAYOUT_INFO_WHITELIST = {
    "SCBSmartDock",
    "SCBExtScreenDock",
    "status_bar_tray",
    "status_bar_personal",
    "status_bar_sound_panel",
    "status_bar_notification_panel",
    "status_bar_input_panel",
    "status_bar_control_center",
    "status_bar_wifi_panel",
    "status_bar_input_method",
    "status_bar_assistant_translate",
    "status_bar_quick_note",
    "status_bar_bluetooth_panel",
    "status_bar_battery_panel",
    "status_bar_focus_mode_paddle",
    "SCBStatusBar"
};

const std::unordered_set<std::string> PIP_SCENE_FORBID_LIST = {
    "PadWithCar",
    "HiCar"
};

const std::chrono::milliseconds WAIT_TIME(3 * 1000); // 3 * 1000 wait for 3s

std::string GetCurrentTime()
{
    struct timespec tn;
    clock_gettime(CLOCK_REALTIME, &tn);
    uint64_t uTime = static_cast<uint64_t>(tn.tv_sec) * NANO_SECOND_PER_SEC +
        static_cast<uint64_t>(tn.tv_nsec);
    return std::to_string(uTime);
}
int Comp(const std::pair<uint64_t, WindowVisibilityState>& a, const std::pair<uint64_t, WindowVisibilityState>& b)
{
    return a.first < b.first;
}

bool GetSingleIntItem(const WindowSceneConfig::ConfigItem& item, int32_t& value)
{
    if (item.IsInts() && item.intsValue_ && item.intsValue_->size() == 1) {
        value = (*item.intsValue_)[0];
        return true;
    }
    return false;
}

int32_t GetPid()
{
    static int32_t pid = static_cast<int32_t>(getpid());
    return pid;
}

bool GetEnableRemoveStartingWindowFromBMS(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    auto& metadata = abilityInfo->metadata;
    for (const auto& item : metadata) {
        if (item.name == "enable.remove.starting.window") {
            TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "enable.remove.starting.window=%{public}s", item.value.c_str());
            return item.value == "true";
        }
    }
    TLOGI(WmsLogTag::WMS_STARTUP_PAGE, "enable.remove.starting.window default false");
    return false;
}

bool IsUIExtCanShowOnLockScreen(const AppExecFwk::ElementName& element, uint32_t callingTokenId,
    AppExecFwk::ExtensionAbilityType extensionAbilityType)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "UIExtOnLock: bundleName: %{public}s, moduleName: %{public}s, ablilityName: %{public}s",
          element.GetBundleName().c_str(), element.GetModuleName().c_str(), element.GetAbilityName().c_str());
    static const std::unordered_set<AppExecFwk::ExtensionAbilityType> extensionAbilityTypeWhitelist = {
        AppExecFwk::ExtensionAbilityType::LIVEVIEW_LOCKSCREEN
    };
    static const std::vector<std::tuple<std::string, std::string, std::string>> elementNameWhitelist = {
        std::make_tuple("com.huawei.hmos.settings", "AccessibilityReConfirmDialog", "phone_settings"),
        std::make_tuple("com.huawei.hmos.settings", "AccessibilityShortKeyDialog", "phone_settings"),
        std::make_tuple("com.huawei.hmos.settings", "DefaultIntentUiExtensionAbility", "phone_settings"),
        std::make_tuple("com.ohos.sceneboard", "ScbIntentUIExtensionAbility", "phone_sceneboard"),
        std::make_tuple("com.ohos.sceneboard", "PoweroffAbility", "phone_sceneboard"),
        std::make_tuple("com.ohos.sceneboard", "com.ohos.sceneboard.MetaBallsAbility", "metaBallsTurbo"),
        std::make_tuple("com.huawei.hmos.motiongesture", "IntentUIExtensionAbility", "entry"),
        std::make_tuple("com.ohos.useriam.authwidget", "userauthuiextensionability", "entry"),
        std::make_tuple("com.ohos.sceneboard", "AodStyleAbility", "phone_sceneboard"),
        std::make_tuple("com.ohos.sceneboard", "HomeThemeComponentExtAbility", "themecomponent"),
        std::make_tuple("com.ohos.sceneboard", "ThemePersonalizedResourceAbility", "engineservice"),
        std::make_tuple("com.ohos.sceneboard", "ThemePersonalizedEditingAbility", "engineservice"),
        std::make_tuple("com.ohos.sceneboard", "CoverExtensionAbility", "coverthemecomponent"),
        std::make_tuple("com.huawei.hmos.findservice", "SystemDialogAbility", "entry"),
        std::make_tuple("com.huawei.hmos.mediacontroller", "UIExtAbility", "phone_deviceswitch"),
        std::make_tuple("com.huawei.hmos.mediacontroller", "AnahsDialogAbility", "phone_deviceswitch"),
        std::make_tuple("com.huawei.hmos.security.privacycenter", "SuperPrivacyProtectedAbility", "superprivacy"),
        std::make_tuple("com.huawei.hmos.security.privacycenter", "PermDisabledReminderAbility", "superprivacy"),
        std::make_tuple("com.huawei.hmos.audioaccessorymanager", "NearbyAbility", "phone"),
        std::make_tuple("com.huawei.hmos.wallet", "WalletDialogUIExtensionAbility", "entry"),
        std::make_tuple("com.huawei.hmos.settings", "WifiWindowSettingsAbility", "pc_settings"),
        std::make_tuple("com.ohos.mms", "DeskDialogExtensionAbility", "entry"),
        std::make_tuple("com.ohos.sceneboard", "ThemeChargingAbility", "engineservice"),
        std::make_tuple("com.ohos.commondialog", "FoldStatusAbility", "phone_commondialog"),
        std::make_tuple("com.ohos.commondialog", "SecondaryReflexionAbility", "phone_commondialog"),
        std::make_tuple("com.huawei.hmos.clock", "ScreenLockCoverExtensionAbility", "entry"),
        std::make_tuple("com.huawei.hmos.clock", "ClockScreenLockExtensionAbility", "entry"),
        std::make_tuple("com.huawei.hmos.penglaimodeassistant", "ExpandedDialogUIExtAbility", "entry"),
        std::make_tuple("com.huawei.hmos.penglaimodeassistant", "CameraDialogAbility", "entry"),
    };

    if (extensionAbilityTypeWhitelist.find(extensionAbilityType) != extensionAbilityTypeWhitelist.end()) {
        TLOGI(WmsLogTag::WMS_UIEXT, "ability in white list");
        return true;
    }

    auto it = std::find_if(elementNameWhitelist.begin(), elementNameWhitelist.end(), [&element](const auto& item) {
        auto& [bundleName, abilityName, _] = item;
        return (element.GetBundleName() == bundleName && element.GetAbilityName() == abilityName);
    });
    if (it != elementNameWhitelist.end()) {
        return true;
    }

    TLOGD(WmsLogTag::WMS_UIEXT, "UIExtOnLock: not in white list");
    return SessionPermission::VerifyPermissionByCallerToken(callingTokenId,
        PermissionConstants::PERMISSION_CALLED_EXTENSION_ON_LOCK_SCREEN);
}

class BundleStatusCallback : public IRemoteStub<AppExecFwk::IBundleStatusCallback> {
public:
    BundleStatusCallback() = default;
    virtual ~BundleStatusCallback() = default;

    void OnBundleStateChanged(const uint8_t installType,
        const int32_t resultCode, const std::string& resultMsg, const std::string& bundleName) override {}

    void OnBundleAdded(const std::string& bundleName, const int userId) override
    {
        SceneSessionManager::GetInstance().OnBundleUpdated(bundleName, userId);
    }

    void OnBundleUpdated(const std::string& bundleName, const int userId) override
    {
        SceneSessionManager::GetInstance().OnBundleUpdated(bundleName, userId);
    }

    void OnBundleRemoved(const std::string& bundleName, const int userId) override
    {
        SceneSessionManager::GetInstance().OnBundleUpdated(bundleName, userId);
    }
};

#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
    class OverrideChangeListener : public DisplayPowerMgr::DisplayBrightnessListenerStub {
    public:
        void OnDataChanged(const std::string& params) override
        {
            SceneSessionManager::GetInstance().NotifyBrightnessModeChange(params);
        }
    };
#endif

bool CheckAvoidAreaForAINavigationBar(bool isVisible, const AvoidArea& avoidArea, int32_t sessionBottom)
{
    if (!avoidArea.topRect_.IsUninitializedRect() || !avoidArea.leftRect_.IsUninitializedRect() ||
        !avoidArea.rightRect_.IsUninitializedRect()) {
        return false;
    }
    if (avoidArea.bottomRect_.IsUninitializedRect()) {
        return true;
    }
    auto diff =
        std::abs(avoidArea.bottomRect_.posY_ + static_cast<int32_t>(avoidArea.bottomRect_.height_) - sessionBottom);
    return isVisible && diff <= 1;
}

enum class UpdateStartingWindowColorCacheResult : uint32_t {
    SUCCESS = 0,
    COLOR_MAP_BUNDLE_NOT_FOUND,
    COLOR_MAP_KEY_PAIR_NOT_FOUND,
    INFO_MAP_BUNDLE_NOT_FOUND,
    INFO_MAP_KEY_PAIR_NOT_FOUND,
};
} // namespace

sptr<SceneSessionManager> SceneSessionManager::CreateInstance()
{
    sptr<SceneSessionManager> sessionManager = new SceneSessionManager();
    sessionManager->Init();
    return sessionManager;
}

SceneSessionManager& SceneSessionManager::GetInstance()
{
    static sptr<SceneSessionManager> instance = CreateInstance();
    return *instance;
}

SceneSessionManager::SceneSessionManager() : rsInterface_(RSInterfaces::GetInstance())
{
    taskScheduler_ = std::make_shared<TaskScheduler>(SCENE_SESSION_MANAGER_THREAD);
    if (!mainHandler_) {
        auto runner = AppExecFwk::EventRunner::GetMainEventRunner();
        mainHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    currentUserId_ = DEFAULT_USERID;
    launcherService_ = sptr<AppExecFwk::LauncherService>::MakeSptr();
    if (!launcherService_->RegisterCallback(new BundleStatusCallback())) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to register bundle status callback.");
    }

    scbDumpSubscriber_ = ScbDumpSubscriber::Subscribe();

    listenerController_ = std::make_shared<SessionListenerController>(taskScheduler_);
    windowFocusController_ = sptr<WindowFocusController>::MakeSptr();
    ffrtQueueHelper_ = std::make_shared<FfrtQueueHelper>();
}

SceneSessionManager::~SceneSessionManager()
{
    ScbDumpSubscriber::UnSubscribe(scbDumpSubscriber_);
    SessionChangeRecorder::GetInstance().stopLogFlag_.store(true);
    if (appMgrClient_ != nullptr && appStateObserver_ != nullptr) {
        auto ret = appMgrClient_->UnregisterApplicationStateObserver(appStateObserver_);
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "unregister app observer result=%{public}d", ret);
    }
    UnregisterBrightnessDataChangeListener();
}

void SceneSessionManager::Init()
{
    bool isScbCoreEnabled = system::GetParameter("persist.window.scbcore.enable", "1") == "1";
    Session::SetScbCoreEnabled(isScbCoreEnabled);
    bool isBackgroundWindowNotifyEnabled = system::GetParameter("persist.window.background.notify.enable", "0") == "1";
    Session::SetBackgroundUpdateRectNotifyEnabled(isBackgroundWindowNotifyEnabled);

    constexpr uint64_t interval = 5 * 1000; // 5 second
    if (HiviewDFX::Watchdog::GetInstance().AddThread(
        SCENE_SESSION_MANAGER_THREAD, taskScheduler_->GetEventHandler(), interval)) {
        TLOGW(WmsLogTag::DEFAULT, "Add thread %{public}s to watchdog failed.", SCENE_SESSION_MANAGER_THREAD.c_str());
    }

    bundleMgr_ = GetBundleManager();

    // Parse configuration.
    LoadWindowSceneXml();
    LoadWindowParameter();
    InitPrepareTerminateConfig();

    ScreenSessionManagerClient::GetInstance().RegisterDisplayChangeListener(sptr<DisplayChangeListener>::MakeSptr());
    ScreenSessionManagerClient::GetInstance().RegisterScreenConnectionChangeListener(
        sptr<ScreenConnectionChangeListener>::MakeSptr());

    // create handler for inner command at server
    eventLoop_ = AppExecFwk::EventRunner::Create(WINDOW_INFO_REPORT_THREAD);
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(eventLoop_);
    int ret = HiviewDFX::Watchdog::GetInstance().AddThread(WINDOW_INFO_REPORT_THREAD, eventHandler_);
    if (ret != 0) {
        TLOGW(WmsLogTag::DEFAULT, "Add thread %{public}s to watchdog failed.", WINDOW_INFO_REPORT_THREAD.c_str());
    }
    taskScheduler_->SetExportHandler(eventHandler_);

    scbSessionHandler_ = sptr<ScbSessionHandler>::MakeSptr();
    AAFwk::AbilityManagerClient::GetInstance()->RegisterSessionHandler(scbSessionHandler_);
    StartWindowInfoReportLoop();
    TLOGI(WmsLogTag::DEFAULT, "SSM init success.");

    RegisterAppListener();
    openDebugTrace_ = std::atoi((system::GetParameter("persist.sys.graphic.openDebugTrace", "0")).c_str()) != 0;
    isKeyboardPanelEnabled_ = system::GetParameter("persist.sceneboard.keyboardPanel.enabled", "1")  == "1";
    isTrayAppForeground_ = system::GetParameter("persist.window.tray_foreground", "") == "true";

    // window recover
    RegisterSessionRecoverStateChangeListener();
    RegisterRecoverStateChangeListener();

    // Input init.
    SceneInputManager::GetInstance().Init();
    RegisterFlushWindowInfoCallback();

    // DFX
    SessionChangeRecorder::GetInstance().Init();

    if (MultiInstanceManager::IsSupportMultiInstance(systemConfig_)) {
        MultiInstanceManager::GetInstance().Init(bundleMgr_, taskScheduler_);
        MultiInstanceManager::GetInstance().SetCurrentUserId(currentUserId_);
    }
    AbilityInfoManager::GetInstance().Init(bundleMgr_);
    AbilityInfoManager::GetInstance().SetCurrentUserId(currentUserId_);

    InitVsyncStation();
    UpdateDarkColorModeToRS();
    CreateRootSceneSession();
    foldChangeCallback_ = std::make_shared<FoldScreenStatusChangeCallback>(
        std::bind(&SceneSessionManager::UpdateSessionWithFoldStateChange, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
    PcFoldScreenManager::GetInstance().RegisterFoldScreenStatusChangeCallback(0, foldChangeCallback_);
    RegisterAppStateObserver();

    InitWindowPattern();
    WSSnapshotHelper::GetInstance()->SetWindowScreenStatus(DisplayManager::GetInstance().GetFoldStatus());

    //Subscribe power manager service
    #ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
        SubscribePowerManagerServiceSa();
    #endif
}

void SceneSessionManager::RegisterBrightnessDataChangeListener()
{
    #ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
        auto ret = DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().RegisterDataChangeListener(
            new OverrideChangeListener(), DisplayPowerMgr::DisplayDataChangeListenerType::FORCE_EXIT_OVERRIDDEN_MODE);
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "ret: %{public}d", static_cast<int32_t>(ret));
    #endif
}

void SceneSessionManager::UnregisterBrightnessDataChangeListener()
{
    #ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
        auto ret = DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().UnregisterDataChangeListener(
            DisplayPowerMgr::DisplayDataChangeListenerType::FORCE_EXIT_OVERRIDDEN_MODE);
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "ret: %{public}d", static_cast<int32_t>(ret));
    #endif
}

void SceneSessionManager::SubscribePowerManagerServiceSa()
{
    #ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
        auto task = [this, where = __func__]() {
            SCBThreadInfo threadInfo {};
            sptr<ISystemAbilityManager> systemAbilityManager =
                SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (!systemAbilityManager) {
                TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s, failed to get system ability manager client", where);
                return;
            }
            auto statusChangeListener = sptr<SceneSystemAbilityListener>::MakeSptr(threadInfo);
            int32_t ret = systemAbilityManager->SubscribeSystemAbility(POWER_MANAGER_SERVICE_ID, statusChangeListener);
            if (ret != ERR_OK) {
                TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s, failed to subscribe system ability manager", where);
            }
        };
        taskScheduler_->PostAsyncTask(task, "SubscribePowerMrgTask");
    #endif
}

void SceneSessionManager::InitWindowPattern()
{
    InitSnapshotCache();
    InitStartingWindow();
    InitDmaReclaimParam();
}

void SceneSessionManager::InitStartingWindow()
{
    syncLoadStartingWindow_ = system::GetBoolParameter("const.window.sync_startingWindow", false);
    TLOGI(WmsLogTag::WMS_PATTERN, "Sync Load StartingWindow: %{public}d", syncLoadStartingWindow_);
}

void SceneSessionManager::InitDmaReclaimParam()
{
    enableDmaReclaim_ = system::GetBoolParameter("resourceschedule.memmgr.dma.reclaimable", false);
    TLOGI(WmsLogTag::WMS_PATTERN, "Dma reclaim enabled: %{public}d", enableDmaReclaim_);
}

void SceneSessionManager::RegisterSessionRecoverStateChangeListener()
{
    sessionRecoverStateChangeFunc_ = [this](const SessionRecoverState& state,
        const sptr<WindowSessionProperty>& property) THREAD_SAFETY_GUARD(SCENE_GUARD) {
            this->OnSessionRecoverStateChange(state, property);
        };
}

void SceneSessionManager::OnSessionRecoverStateChange(const SessionRecoverState& state,
        const sptr<WindowSessionProperty>& property)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "id: %{public}d, state:%{public}u", property->GetPersistentId(), state);
    switch (state) {
        case SessionRecoverState::SESSION_START_RECONNECT:
            // The server session has not request yet.
            UpdateRecoverPropertyForSuperFold(property);
            RecoverSessionInfo(property);
            break;
        case SessionRecoverState::SESSION_FINISH_RECONNECT: {
            // The server session has been request.
            auto sessionInfo = property->GetSessionInfo();
            auto persistentId = sessionInfo.persistentId_;
            auto sceneSession = GetSceneSession(persistentId);
            if (sceneSession == nullptr) {
                break;
            }
            if (SessionHelper::IsMainWindow(sceneSession->GetWindowType())) {
                sceneSession->SetRecovered(true);
                recoverSceneSessionFunc_(sceneSession, sessionInfo);
                sceneSession->SetWindowShadowEnabled(property->GetWindowShadowEnabled());
            } else {
                sceneSession->NotifyFollowParentMultiScreenPolicy(sessionInfo.isFollowParentMultiScreenPolicy);
                NotifyCreateSpecificSession(sceneSession, property, property->GetWindowType());
                CacheSpecificSessionForRecovering(sceneSession, property);
                sceneSession->SetWindowAnchorInfo(property->GetWindowAnchorInfo());
            }
            NotifySessionUnfocusedToClient(persistentId);
            break;
        }
        default:
            break;
    }
}

void SceneSessionManager::RegisterRecoverStateChangeListener()
{
    recoverStateChangeFunc_ = [this](const RecoverState& state) THREAD_SAFETY_GUARD(SCENE_GUARD) {
        OnRecoverStateChange(state);
    };
}

void SceneSessionManager::OnRecoverStateChange(const RecoverState& state)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "state: %{public}u", state);
    recoverState_ = state;
    switch(state) {
        case RecoverState::RECOVER_INITIAL:
            break;
        case RecoverState::RECOVER_ENABLE_INPUT:
            SetDisplayBrightness(INVALID_BRIGHTNESS);
            // All session info has been recovered then recover the window outline if need.
            if (needRecoverOutline_) {
                TLOGI(WmsLogTag::WMS_ANIMATION, "Recover the window outline.");
                UpdateOutlineInner(outlineRemoteObject_, recoverOutlineParams_);
                needRecoverOutline_ = false;
            }
            break;
        default:
            break;
    }
}

void SceneSessionManager::RegisterFlushWindowInfoCallback()
{
    SceneInputManager::GetInstance().
        RegisterFlushWindowInfoCallback([this]() { FlushWindowInfoToMMI(); });
}

void SceneSessionManager::InitVsyncStation()
{
    NodeId nodeId = 0;
    vsyncStation_ = std::make_shared<VsyncStation>(nodeId);
}

void SceneSessionManager::InitScheduleUtils()
{
#ifdef RES_SCHED_ENABLE
    SCBThreadInfo threadInfo = {
        .scbUid_ = std::to_string(getuid()), .scbPid_ = std::to_string(getprocpid()),
        .scbTid_ = std::to_string(getproctid()), .scbBundleName_ = SCENE_BOARD_BUNDLE_NAME
    };
    std::unordered_map<std::string, std::string> payload {
        { "pid", threadInfo.scbPid_ },
        { "tid", threadInfo.scbTid_ },
        { "uid", threadInfo.scbUid_ },
        { "bundleName", threadInfo.scbBundleName_ },
    };
    uint32_t type = OHOS::ResourceSchedule::ResType::RES_TYPE_REPORT_SCENE_BOARD;
    OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, 0, payload);
    auto task = [threadInfo = std::move(threadInfo)]() mutable {
        threadInfo.ssmThreadName_ = "OS_SceneSession";
        threadInfo.ssmTid_ = std::to_string(gettid());
        const int32_t userInteraction = 2;
        std::unordered_map<std::string, std::string> payload{
            { "pid", threadInfo.scbPid_ },
            { "tid", threadInfo.ssmTid_ },
            { "uid", threadInfo.scbUid_ },
            { "extType", "10002" },
            { "cgroupPrio", "1" },
            { "isSa", "0" },
            { "threadName", threadInfo.ssmThreadName_ }
        };
        uint32_t type = ResourceSchedule::ResType::RES_TYPE_KEY_PERF_SCENE;
        OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, userInteraction, payload);
        TLOGNI(WmsLogTag::WMS_LIFE, "set RES_TYPE_KEY_PERF_SCENE success");
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityManager) {
            TLOGNE(WmsLogTag::WMS_MAIN, "failed to get system ability manager client");
            return;
        }
        auto statusChangeListener = sptr<SceneSystemAbilityListener>::MakeSptr(threadInfo);
        int32_t ret = systemAbilityManager->SubscribeSystemAbility(RES_SCHED_SYS_ABILITY_ID, statusChangeListener);
        if (ret != ERR_OK) {
            TLOGNI(WmsLogTag::WMS_MAIN, "failed to subscribe system ability manager");
        }
    };
    taskScheduler_->PostAsyncTask(task, "changeQosTask");
#endif
}

void SceneSessionManager::UpdateSessionWithFoldStateChange(DisplayId displayId, SuperFoldStatus status,
    SuperFoldStatus prevStatus)
{
    auto task = [this, displayId, status, where = __func__]() {
        TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: status=%{public}u, display=%{public}" PRIu64,
            where, static_cast<uint32_t>(status), displayId);
        if (status == SuperFoldStatus::HALF_FOLDED) {
            auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(displayId);
            if (display != nullptr) {
                UpdateDisplayRegion(display->GetDisplayInfo());
            }
        }
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                TLOGE(WmsLogTag::WMS_MAIN, "session is nullptr");
                continue;
            }
            sceneSession->UpdateCrossAxis();
        }
    };
    taskScheduler_->PostAsyncTask(task, __func__);
}

void SceneSessionManager::SetBehindWindowFilterEnabled(bool enabled)
{
    bool isSuccessful = RSInterfaces::GetInstance().SetBehindWindowFilterEnabled(enabled);
    TLOGI(WmsLogTag::WMS_PC, "is successful:  %{public}d", isSuccessful);
}

void SceneSessionManager::RegisterAppListener()
{
    appAnrListener_ = sptr<AppAnrListener>::MakeSptr();
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "appMgrClient is nullptr.");
    } else {
        auto ret = static_cast<int32_t>(appMgrClient->RegisterAppDebugListener(appAnrListener_));
        TLOGI(WmsLogTag::DEFAULT, "Register app debug listener, %{public}d.", ret);
    }
}

void SceneSessionManager::LoadWindowParameter()
{
    const std::string multiWindowUIType = system::GetParameter("const.window.multiWindowUIType", "HandsetSmartWindow");
    const bool isPcMode = system::GetBoolParameter("persist.sceneboard.ispcmode", false);
    if (multiWindowUIType == "HandsetSmartWindow") {
        systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    } else if (multiWindowUIType == "FreeFormMultiWindow" || isPcMode) {
        systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    } else if (multiWindowUIType == "TabletSmartWindow") {
        systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    } else {
        TLOGE(WmsLogTag::DEFAULT, "unknown multiWindowUIType:%{public}s.", multiWindowUIType.c_str());
    }
    appWindowSceneConfig_.multiWindowUIType_ = multiWindowUIType;
}

void SceneSessionManager::LoadWindowSceneXml()
{
    if (WindowSceneConfig::LoadConfigXml()) {
        if (WindowSceneConfig::GetConfig().IsMap()) {
            WindowSceneConfig::DumpConfig(*WindowSceneConfig::GetConfig().mapValue_);
        }
        ConfigWindowSceneXml();
    } else {
        TLOGE(WmsLogTag::DEFAULT, "Load window scene xml failed");
    }
    ConfigDefaultKeyboardAnimation(appWindowSceneConfig_.keyboardAnimationIn_,
        appWindowSceneConfig_.keyboardAnimationOut_);
}

bool SceneSessionManager::IsPrepareTerminateEnabled() const
{
    char value[PREPARE_TERMINATE_ENABLE_SIZE] = "false";
    int32_t retSysParam = GetParameter(PREPARE_TERMINATE_ENABLE_PARAMETER, "false", value,
        PREPARE_TERMINATE_ENABLE_SIZE);
    TLOGI(WmsLogTag::DEFAULT, "%{public}s value is %{public}s.", PREPARE_TERMINATE_ENABLE_PARAMETER, value);
    if (retSysParam > 0 && !std::strcmp(value, "true")) {
        return true;
    }
    return false;
}

void SceneSessionManager::InitPrepareTerminateConfig()
{
    if (IsPrepareTerminateEnabled()) {
        isPrepareTerminateEnable_ = true;
    }
}

void SceneSessionManager::ConfigWindowSceneXml()
{
    const auto& config = WindowSceneConfig::GetConfig();
    WindowSceneConfig::ConfigItem item = config["windowEffect"];
    if (item.IsMap()) {
        ConfigWindowEffect(item, appWindowSceneConfig_);
    }

    item = config["decor"];
    if (item.IsMap()) {
        ConfigDecor(item);
    }

    item = config["backgroundswitch"];
    int32_t param = -1;
    systemConfig_.backgroundswitch = GetSingleIntItem(item, param) && param == 1;
    TLOGD(WmsLogTag::DEFAULT, "Load backgroundswitch %{public}d", systemConfig_.backgroundswitch);
    item = config["defaultWindowMode"];
    if (GetSingleIntItem(item, param) &&
        (param == static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN) ||
        param == static_cast<int32_t>(WindowMode::WINDOW_MODE_FLOATING))) {
        systemConfig_.defaultWindowMode_ = static_cast<WindowMode>(static_cast<uint32_t>(param));
    }
    item = config["defaultMaximizeMode"];
    if (GetSingleIntItem(item, param) &&
        (param == static_cast<int32_t>(MaximizeMode::MODE_AVOID_SYSTEM_BAR) ||
         param == static_cast<int32_t>(MaximizeMode::MODE_FULL_FILL))) {
        SceneSession::maximizeMode_ = static_cast<MaximizeMode>(param);
    }
    item = config["keyboardAnimation"];
    if (item.IsMap()) {
        ConfigKeyboardAnimation(item);
    }
    item = config["maxFloatingWindowSize"];
    if (GetSingleIntItem(item, param)) {
        systemConfig_.maxFloatingWindowSize_ = static_cast<uint32_t>(param);
    }
    item = config["windowAnimation"];
    if (item.IsMap()) {
        ConfigWindowAnimation(item);
    }
    item = config["startWindowTransitionAnimation"];
    if (item.IsMap()) {
        ConfigStartingWindowAnimation(item);
    }
    item = config["maxMidSceneNum"];
    if (GetSingleIntItem(item, param)) {
        systemConfig_.maxMidSceneNum_ = static_cast<uint32_t>(param);
    }
    ConfigFreeMultiWindow();
    ConfigWindowSizeLimits();
    ConfigAppsWithDeduplicatedWindowStatus();
    ConfigSnapshotScale();
    ConfigWindowSceneXml(config);
    ConfigWindowLayout(config["windowLayout"]);
}

void SceneSessionManager::ConfigWindowSceneXml(const WindowSceneConfig::ConfigItem& config)
{
    WindowSceneConfig::ConfigItem item = config["systemUIStatusBar"];
    if (item.IsMap()) {
        ConfigSystemUIStatusBar(item);
    }
    item = config["backgroundScreenLock"].GetProp("enable");
    if (item.IsBool()) {
        appWindowSceneConfig_.backgroundScreenLock_ = item.boolValue_;
    }
    item = config["rotationMode"];
    if (item.IsString()) {
        appWindowSceneConfig_.rotationMode_ = item.stringValue_;
    }
    item = config["immersive"];
    if (item.IsMap()) {
        ConfigWindowImmersive(item);
    }
    item = config["supportTypeFloatWindow"].GetProp("enable");
    if (item.IsBool()) {
        systemConfig_.supportTypeFloatWindow_ = item.boolValue_;
    }
    item = config["singleHandCompatibleMode"];
    if (item.IsMap()) {
        ConfigSingleHandCompatibleMode(item);
    }
}

void SceneSessionManager::ConfigWindowImmersive(const WindowSceneConfig::ConfigItem& immersiveConfig)
{
    AppWindowSceneConfig config;
    WindowSceneConfig::ConfigItem item = immersiveConfig["inDesktopStatusBarConfig"];
    if (item.IsMap()) {
        if (ConfigStatusBar(item, config.windowImmersive_.desktopStatusBarConfig_)) {
            appWindowSceneConfig_.windowImmersive_.desktopStatusBarConfig_ =
                config.windowImmersive_.desktopStatusBarConfig_;
        }
    }
    item = immersiveConfig["inSplitStatusBarConfig"]["upDownSplit"];
    if (item.IsMap()) {
        if (ConfigStatusBar(item, config.windowImmersive_.upDownStatusBarConfig_)) {
            appWindowSceneConfig_.windowImmersive_.upDownStatusBarConfig_ =
                config.windowImmersive_.upDownStatusBarConfig_;
        }
    }
    item = immersiveConfig["inSplitStatusBarConfig"]["leftRightSplit"];
    if (item.IsMap()) {
        if (ConfigStatusBar(item, config.windowImmersive_.leftRightStatusBarConfig_)) {
            appWindowSceneConfig_.windowImmersive_.leftRightStatusBarConfig_ =
                config.windowImmersive_.leftRightStatusBarConfig_;
        }
    }
}

bool SceneSessionManager::ConfigStatusBar(const WindowSceneConfig::ConfigItem& config,
    StatusBarConfig& statusBarConfig)
{
    WindowSceneConfig::ConfigItem item = config["showHide"].GetProp("enable");
    if (item.IsBool()) {
        statusBarConfig.showHide_ = item.boolValue_;
    }
    item = config["contentColor"];
    if (item.IsString()) {
        statusBarConfig.contentColor_ = item.stringValue_;
    }
    item = config["backgroundColor"];
    if (item.IsString()) {
        statusBarConfig.backgroundColor_ = item.stringValue_;
    }
    return true;
}

void SceneSessionManager::ConfigFreeMultiWindow()
{
    const auto& config = WindowSceneConfig::GetConfig();
    WindowSceneConfig::ConfigItem freeMultiWindowConfig = config["freeMultiWindow"];
    if (freeMultiWindowConfig.IsMap()) {
        auto supportItem = freeMultiWindowConfig.GetProp("enable");
        if (supportItem.IsBool()) {
            systemConfig_.freeMultiWindowSupport_ = supportItem.boolValue_;
        }
        auto item = freeMultiWindowConfig["decor"];
        if (item.IsMap()) {
            ConfigDecor(item, false);
        }
        int32_t param = -1;
        item = freeMultiWindowConfig["defaultWindowMode"];
        if (GetSingleIntItem(item, param) &&
            (param == static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN) ||
             param == static_cast<int32_t>(WindowMode::WINDOW_MODE_FLOATING))) {
            systemConfig_.freeMultiWindowConfig_.defaultWindowMode_ =
                static_cast<WindowMode>(static_cast<uint32_t>(param));
        }
        item = freeMultiWindowConfig["maxMainFloatingWindowNumber"];
        if (GetSingleIntItem(item, param) && (param > 0)) {
            systemConfig_.freeMultiWindowConfig_.maxMainFloatingWindowNumber_ = static_cast<uint32_t>(param);
        }
        param = -1;
        item = freeMultiWindowConfig["defaultDragResizeType"];
        if (GetSingleIntItem(item, param) && param >= 0 &&
            param < static_cast<int32_t>(DragResizeType::RESIZE_MAX_VALUE)) {
            systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ =
                static_cast<DragResizeType>(static_cast<uint32_t>(param));
        }
        item = freeMultiWindowConfig["windowEffect"];
        if (item.IsMap()) {
            ConfigWindowEffect(item, systemConfig_.freeMultiWindowConfig_.appWindowSceneConfig_);
        } else {
            item = config["windowEffect"];
            if (item.IsMap()) {
                ConfigWindowEffect(item, systemConfig_.freeMultiWindowConfig_.appWindowSceneConfig_);
            }
        }
    }
}

void SceneSessionManager::LoadFreeMultiWindowConfig(bool enable)
{
    FreeMultiWindowConfig freeMultiWindowConfig = systemConfig_.freeMultiWindowConfig_;
    if (enable) {
        systemConfig_.defaultWindowMode_ = freeMultiWindowConfig.defaultWindowMode_;
        systemConfig_.decorWindowModeSupportType_ = freeMultiWindowConfig.decorWindowModeSupportType_;
        systemConfig_.isSystemDecorEnable_ = freeMultiWindowConfig.isSystemDecorEnable_;
        appWindowSceneConfig_.focusedShadow_ = freeMultiWindowConfig.appWindowSceneConfig_.focusedShadow_;
        appWindowSceneConfig_.unfocusedShadow_ = freeMultiWindowConfig.appWindowSceneConfig_.unfocusedShadow_;
        appWindowSceneConfig_.focusedShadowDark_ = freeMultiWindowConfig.appWindowSceneConfig_.focusedShadowDark_;
        appWindowSceneConfig_.unfocusedShadowDark_ = freeMultiWindowConfig.appWindowSceneConfig_.unfocusedShadowDark_;
    } else {
        const auto& config = WindowSceneConfig::GetConfig();
        auto item = config["decor"];
        if (item.IsMap()) {
            ConfigDecor(item, true);
        }
        int32_t param = -1;
        item = config["defaultWindowMode"];
        if (GetSingleIntItem(item, param) &&
            (param == static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN) ||
             param == static_cast<int32_t>(WindowMode::WINDOW_MODE_FLOATING))) {
            systemConfig_.defaultWindowMode_ = static_cast<WindowMode>(static_cast<uint32_t>(param));
        }
        item = config["windowEffect"];
        if (item.IsMap()) {
            ConfigWindowEffect(item, appWindowSceneConfig_);
        }
    }
    systemConfig_.freeMultiWindowEnable_ = enable;
    rsInterface_.SetFreeMultiWindowStatus(enable);
}

const SystemSessionConfig& SceneSessionManager::GetSystemSessionConfig() const
{
    return systemConfig_;
}

WSError SceneSessionManager::SwitchFreeMultiWindow(bool enable)
{
    if (!systemConfig_.freeMultiWindowSupport_) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "device not support");
        return WSError::WS_ERROR_DEVICE_NOT_SUPPORT;
    }
    LoadFreeMultiWindowConfig(enable);
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                continue;
            }
            sceneSession->SwitchFreeMultiWindow(systemConfig_);
        }
    }
    UpdateAppHookWindowInfoWhenSwitchFreeMultiWindow(enable);
    WindowStyleType type = enable ?
            WindowStyleType::WINDOW_STYLE_FREE_MULTI_WINDOW : WindowStyleType::WINDOW_STYLE_DEFAULT;
    SessionManagerAgentController::GetInstance().NotifyWindowStyleChange(type);
    return WSError::WS_OK;
}

WSError SceneSessionManager::GetFreeMultiWindowEnableState(bool& enable)
{
    enable = systemConfig_.freeMultiWindowEnable_;
    return WSError::WS_OK;
}

WMError SceneSessionManager::SetForegroundWindowNum(uint32_t windowNum)
{
    if (!SessionPermission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::WMS_PC, "The caller is not system service.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (!systemConfig_.freeMultiWindowSupport_) {
        TLOGE(WmsLogTag::WMS_PC, "device not support");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    taskScheduler_->PostAsyncTask([this, windowNum]() {
        if (setForegroundWindowNumFunc_) {
            TLOGNI(WmsLogTag::WMS_PC, "max foreground windowNum: %{public}d", windowNum);
            setForegroundWindowNumFunc_(windowNum);
        }
    }, __func__);
    return WMError::WM_OK;
}

void SceneSessionManager::RegisterSetForegroundWindowNumCallback(SetForegroundWindowNumFunc&& func)
{
    setForegroundWindowNumFunc_ = std::move(func);
}

WSError SceneSessionManager::SetSessionContinueState(const sptr<IRemoteObject>& token,
    const ContinueState& continueState)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    return taskScheduler_->PostSyncTask([this, token, continueState, callingUid, where = __func__] {
        sptr <SceneSession> sceneSession = FindSessionByToken(token);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::DEFAULT, "fail to find session by token.");
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        sceneSession->SetSessionInfoContinueState(continueState);
#ifdef WINDOW_MANAGER_FEATURE_SUPPORT_DMSFWK
        DistributedClient::GetInstance().SetMissionContinueState(sceneSession->GetPersistentId(),
            static_cast<AAFwk::ContinueState>(continueState), callingUid);
#endif
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s: id:%{public}d, continueState:%{public}d",
            where, sceneSession->GetPersistentId(), continueState);
        return WSError::WS_OK;
    }, __func__);
}

void SceneSessionManager::ConfigDecor(const WindowSceneConfig::ConfigItem& decorConfig, bool mainConfig)
{
    WindowSceneConfig::ConfigItem item = decorConfig.GetProp("enable");
    if (item.IsBool()) {
        if (mainConfig) {
            systemConfig_.isSystemDecorEnable_ = item.boolValue_;
        } else {
            systemConfig_.freeMultiWindowConfig_.isSystemDecorEnable_ = item.boolValue_;
        }
        bool decorEnable = item.boolValue_;
        uint32_t support = 0;
        std::vector<std::string> supportedModes;
        item = decorConfig["supportedMode"];
        if (item.IsStrings()) {
            supportedModes = *item.stringsValue_;
        }
        for (auto mode : supportedModes) {
            if (mode == "fullscreen") {
                support |= WindowModeSupport::WINDOW_MODE_SUPPORT_FULLSCREEN;
            } else if (mode == "floating") {
                support |= WindowModeSupport::WINDOW_MODE_SUPPORT_FLOATING;
            } else if (mode == "pip") {
                support |= WindowModeSupport::WINDOW_MODE_SUPPORT_PIP;
            } else if (mode == "split") {
                support |= WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_PRIMARY |
                    WindowModeSupport::WINDOW_MODE_SUPPORT_SPLIT_SECONDARY;
            } else if (mode == "fb") {
                support |= WindowModeSupport::WINDOW_MODE_SUPPORT_FB;
            } else {
                TLOGW(WmsLogTag::DEFAULT, "Invalid supporedMode");
                support = WindowModeSupport::WINDOW_MODE_SUPPORT_ALL;
                break;
            }
        }
        if (mainConfig && item.IsStrings()) {
            systemConfig_.decorWindowModeSupportType_ = support;
        }
        if (!mainConfig && item.IsStrings()) {
            systemConfig_.freeMultiWindowConfig_.decorWindowModeSupportType_ = support;
        }
    }
}

static void AddAlphaToColor(float alpha, std::string& color)
{
    if (color.size() == 9 || alpha > 1.0f) { // size 9: color is ARGB
        return;
    }

    uint32_t alphaValue = 0xFF * alpha;
    std::ostringstream ss;
    ss << std::hex << alphaValue;
    std::string strAlpha = ss.str();
    if (strAlpha.size() == 1) {
        strAlpha.append(1, '0');
    }

    color.insert(1, strAlpha);
}

static inline bool IsAtomicServiceFreeInstall(const SessionInfo& sessionInfo)
{
    return sessionInfo.isAtomicService_ && sessionInfo.want != nullptr &&
        (sessionInfo.want->GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND;
}

void SceneSessionManager::ConfigWindowEffect(const WindowSceneConfig::ConfigItem& effectConfig,
    AppWindowSceneConfig& appWindowSceneConfig)
{
    AppWindowSceneConfig config;
    // config corner radius
    WindowSceneConfig::ConfigItem item = effectConfig["appWindows"]["cornerRadius"];
    if (item.IsMap()) {
        if (ConfigAppWindowCornerRadius(item["float"], config.floatCornerRadius_)) {
            // set default corner radius of window by system config
            appWindowSceneConfig.floatCornerRadius_ = config.floatCornerRadius_;
            systemConfig_.defaultCornerRadius_ = config.floatCornerRadius_;
        }
    }

    // config shadow
    item = effectConfig["appWindows"]["shadow"]["focused"];
    if (item.IsMap()) {
        if (ConfigAppWindowShadow(item, config.focusedShadow_)) {
            appWindowSceneConfig.focusedShadow_ = config.focusedShadow_;
        }
    }
    item = effectConfig["appWindows"]["shadow"]["unfocused"];
    if (item.IsMap()) {
        if (ConfigAppWindowShadow(item, config.unfocusedShadow_)) {
            appWindowSceneConfig.unfocusedShadow_ = config.unfocusedShadow_;
        }
    }
    AddAlphaToColor(appWindowSceneConfig.focusedShadow_.alpha_, appWindowSceneConfig.focusedShadow_.color_);
    AddAlphaToColor(appWindowSceneConfig.unfocusedShadow_.alpha_, appWindowSceneConfig.unfocusedShadow_.color_);

    // config shadow in dark mode
    item = effectConfig["appWindows"]["shadowDark"]["focused"];
    if (item.IsMap()) {
        if (ConfigAppWindowShadow(item, config.focusedShadowDark_)) {
            appWindowSceneConfig.focusedShadowDark_ = config.focusedShadowDark_;
        }
    }
    item = effectConfig["appWindows"]["shadowDark"]["unfocused"];
    if (item.IsMap()) {
        if (ConfigAppWindowShadow(item, config.unfocusedShadowDark_)) {
            appWindowSceneConfig.unfocusedShadowDark_ = config.unfocusedShadowDark_;
        }
    }
    AddAlphaToColor(appWindowSceneConfig.focusedShadowDark_.alpha_, appWindowSceneConfig.focusedShadowDark_.color_);
    AddAlphaToColor(appWindowSceneConfig.unfocusedShadowDark_.alpha_,
        appWindowSceneConfig.unfocusedShadowDark_.color_);

    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "successfully");
}

bool SceneSessionManager::ConfigAppWindowCornerRadius(const WindowSceneConfig::ConfigItem& item, float& out)
{
    static const std::unordered_map<std::string, float> stringToCornerRadius = {
        {"off", 0.0f}, {"defaultCornerRadiusXS", 4.0f}, {"defaultCornerRadiusS", 8.0f},
        {"defaultCornerRadiusM", 12.0f}, {"defaultCornerRadiusL", 16.0f}, {"defaultCornerRadiusXL", 24.0f}
    };

    if (item.IsString()) {
        auto value = item.stringValue_;
        if (stringToCornerRadius.find(value) != stringToCornerRadius.end()) {
            out = stringToCornerRadius.at(value);
            return true;
        }
    }
    return false;
}

void SceneSessionManager::SetEnableInputEvent(bool enabled)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "enabled: %{public}u", enabled);
    enableInputEvent_ = enabled;

    if (recoverStateChangeFunc_ == nullptr) {
        return;
    }

    if (enabled) {
        recoverStateChangeFunc_(RecoverState::RECOVER_ENABLE_INPUT);
    } else {
        recoverStateChangeFunc_(RecoverState::RECOVER_INITIAL);
    }

}

bool SceneSessionManager::IsInputEventEnabled() const
{
    return enableInputEvent_;
}

void SceneSessionManager::ClearUnrecoveredSessions(const std::vector<int32_t>& recoveredPersistentIds)
{
    for (const auto persistentId : alivePersistentIds_) {
        if (std::find(recoveredPersistentIds.begin(), recoveredPersistentIds.end(), persistentId) !=
            recoveredPersistentIds.end()) {
            continue;
        }
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGE(WmsLogTag::WMS_RECOVER, "Session is nullptr, persistentId=%{public}d", persistentId);
            continue;
        }
        if (sceneSession->IsRecovered()) {
            TLOGI(WmsLogTag::WMS_RECOVER, "persistentId=%{public}d", persistentId);
            std::unique_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
            visibleWindowCountMap_.erase(sceneSession->GetCallingPid());
            EraseSceneSessionAndMarkDirtyLocked(persistentId);
            if (MultiInstanceManager::IsSupportMultiInstance(systemConfig_) &&
                MultiInstanceManager::GetInstance().IsMultiInstance(sceneSession->GetSessionInfo().bundleName_)) {
                MultiInstanceManager::GetInstance().DecreaseInstanceKeyRefCount(sceneSession);
            }
        }
    }
}

void SceneSessionManager::UpdateRecoveredSessionInfo(const std::vector<int32_t>& recoveredPersistentIds)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "persistentIds recovered=%{public}zu. CurrentUserId=%{public}d",
        recoveredPersistentIds.size(), currentUserId_.load());

    taskScheduler_->PostAsyncTask([this, recoveredPersistentIds]() THREAD_SAFETY_GUARD(SCENE_GUARD) {
        ClearUnrecoveredSessions(recoveredPersistentIds);
        std::list<AAFwk::SessionInfo> abilitySessionInfos;
        for (const auto persistentId : recoveredPersistentIds) {
            if (failRecoveredPersistentIdSet_.count(persistentId)) {
                TLOGNI(WmsLogTag::WMS_RECOVER, "failRecoveredPersistentId=%{public}d, continue", persistentId);
                continue;
            }
            auto sceneSession = GetSceneSession(persistentId);
            if (sceneSession == nullptr) {
                TLOGNE(WmsLogTag::WMS_RECOVER, "Session is nullptr, persistentId=%{public}d", persistentId);
                continue;
            }
            const auto& abilitySessionInfo = SetAbilitySessionInfo(sceneSession);
            TLOGND(WmsLogTag::WMS_RECOVER, "recovered persistentId=%{public}d", persistentId);
            abilitySessionInfos.emplace_back(*abilitySessionInfo);
        }
        std::vector<int32_t> unrecoverableSessionIds;
        AAFwk::AbilityManagerClient::GetInstance()->UpdateSessionInfoBySCB(
            abilitySessionInfos, currentUserId_, unrecoverableSessionIds);
        TLOGNI(WmsLogTag::WMS_RECOVER, "Number of unrecoverableSessionIds=%{public}zu",
            unrecoverableSessionIds.size());
        for (const auto sessionId : unrecoverableSessionIds) {
            auto sceneSession = GetSceneSession(sessionId);
            if (sceneSession == nullptr) {
                TLOGNW(WmsLogTag::WMS_RECOVER, "There is no session corresponding to persistentId=%{public}d ",
                    sessionId);
                continue;
            }
            const auto& sceneSessionInfo = SetAbilitySessionInfo(sceneSession);
            TLOGNI(WmsLogTag::WMS_RECOVER, "unrecoverable persistentId=%{public}d", sessionId);
            ExceptionInfo exceptionInfo;
            sceneSession->NotifySessionExceptionInner(sceneSessionInfo, exceptionInfo);
        }
        RemoveFailRecoveredSession();
    }, __func__);
}

bool SceneSessionManager::ConfigAppWindowShadow(const WindowSceneConfig::ConfigItem& shadowConfig,
    WindowShadowConfig& outShadow)
{
    WindowSceneConfig::ConfigItem item = shadowConfig["color"];
    if (item.IsString()) {
        auto color = item.stringValue_;
        uint32_t colorValue;
        if (!ColorParser::Parse(color, colorValue)) {
            return false;
        }
        outShadow.color_ = color;
    }

    item = shadowConfig["offsetX"];
    if (item.IsFloats()) {
        auto offsetX = *item.floatsValue_;
        if (offsetX.size() != 1) {
            return false;
        }
        outShadow.offsetX_ = offsetX[0];
    }

    item = shadowConfig["offsetY"];
    if (item.IsFloats()) {
        auto offsetY = *item.floatsValue_;
        if (offsetY.size() != 1) {
            return false;
        }
        outShadow.offsetY_ = offsetY[0];
    }

    item = shadowConfig["alpha"];
    if (item.IsFloats()) {
        auto alpha = *item.floatsValue_;
        if (alpha.size() != 1 ||
            (MathHelper::LessNotEqual(alpha[0], 0.0) && MathHelper::GreatNotEqual(alpha[0], 1.0))) {
            return false;
        }
        outShadow.alpha_ = alpha[0];
    }

    item = shadowConfig["radius"];
    if (item.IsFloats()) {
        auto radius = *item.floatsValue_;
        if (radius.size() != 1 || MathHelper::LessNotEqual(radius[0], 0.0)) {
            return false;
        }
        outShadow.radius_ = radius[0];
    }

    return true;
}

void SceneSessionManager::LoadKeyboardAnimation(const WindowSceneConfig::ConfigItem& item,
    KeyboardSceneAnimationConfig& config)
{
    if (item.IsMap() && item.mapValue_->count("curve")) {
        const auto& [curveType, curveParams] = CreateCurve(item["curve"]);
        config.curveType_ = curveType;
        if (curveParams.size() == CURVE_PARAM_DIMENSION) {
            config.ctrlX1_ = curveParams[0]; // 0: ctrl x1 index
            config.ctrlY1_ = curveParams[1]; // 1: ctrl y1 index
            config.ctrlX2_ = curveParams[2]; // 2: ctrl x2 index
            config.ctrlY2_ = curveParams[3]; // 3: ctrl y2 index
        }
    }

    const WindowSceneConfig::ConfigItem& duration = item["duration"];
    if (duration.IsInts()) {
        auto numbers = *duration.intsValue_;
        if (numbers.size() == 1) {
            config.duration_ = static_cast<uint32_t>(numbers[0]);
        }
    }
}

void SceneSessionManager::ConfigKeyboardAnimation(const WindowSceneConfig::ConfigItem& animationConfig)
{
    LoadKeyboardAnimation(animationConfig["animationIn"]["timing"], appWindowSceneConfig_.keyboardAnimationIn_);
    LoadKeyboardAnimation(animationConfig["animationOut"]["timing"], appWindowSceneConfig_.keyboardAnimationOut_);

    // config system animation
    const auto& appConfigIn = appWindowSceneConfig_.keyboardAnimationIn_;
    systemConfig_.animationIn_ = KeyboardAnimationCurve(appConfigIn.curveType_,
        {appConfigIn.ctrlX1_, appConfigIn.ctrlY1_, appConfigIn.ctrlX2_, appConfigIn.ctrlY2_},
        appConfigIn.duration_);
    const auto& appConfigOut = appWindowSceneConfig_.keyboardAnimationOut_;
    systemConfig_.animationOut_ = KeyboardAnimationCurve(appConfigOut.curveType_,
        {appConfigOut.ctrlX1_, appConfigOut.ctrlY1_, appConfigOut.ctrlX2_, appConfigOut.ctrlY2_},
        appConfigOut.duration_);
}

void SceneSessionManager::ConfigDefaultKeyboardAnimation(KeyboardSceneAnimationConfig& animationIn,
    KeyboardSceneAnimationConfig& animationOut)
{
    if (!systemConfig_.animationIn_.curveType_.empty() && !systemConfig_.animationOut_.curveType_.empty()) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "product config, curveIn:[%{public}s, %{public}u], "
            "curveOut:[%{public}s, %{public}u]", systemConfig_.animationIn_.curveType_.c_str(),
            systemConfig_.animationIn_.duration_, systemConfig_.animationOut_.curveType_.c_str(),
            systemConfig_.animationOut_.duration_);
        return;
    }

    // default animation curve params
    constexpr char CURVETYPE[] = "interpolatingSpring";
    constexpr float IN_CTRLX1 = 0.0f;
    constexpr float OUT_CTRLX1 = 4.0f;
    constexpr float CTRLY1 = 1.0f;
    constexpr float CTRLX2 = 342.0f;
    constexpr float CTRLY2 = 37.0f;
    constexpr uint32_t DURATION = 150;

    if (systemConfig_.animationIn_.curveType_.empty()) {
        std::vector<float> in = { IN_CTRLX1, CTRLY1, CTRLX2, CTRLY2 };
        // update system config for client
        systemConfig_.animationIn_ = KeyboardAnimationCurve(CURVETYPE, in, DURATION);
        // update app config for server
        animationIn.curveType_ = CURVETYPE;
        animationIn.ctrlX1_ = in[0]; // 0: ctrl x1 index
        animationIn.ctrlY1_ = in[1]; // 1: ctrl y1 index
        animationIn.ctrlX2_ = in[2]; // 2: ctrl x2 index
        animationIn.ctrlY2_ = in[3]; // 3: ctrl y2 index
        animationIn.duration_ = DURATION;
        TLOGI(WmsLogTag::WMS_KEYBOARD, "config default animationIn");
    }

    if (systemConfig_.animationOut_.curveType_.empty()) {
        std::vector<float> out = { OUT_CTRLX1, CTRLY1, CTRLX2, CTRLY2 };
        // update system config for client
        systemConfig_.animationOut_ = KeyboardAnimationCurve(CURVETYPE, out, DURATION);
        // update app config for server
        animationOut.curveType_ = CURVETYPE;
        animationOut.ctrlX1_ = out[0]; // 0: ctrl x1 index
        animationOut.ctrlY1_ = out[1]; // 1: ctrl y1 index
        animationOut.ctrlX2_ = out[2]; // 2: ctrl x2 index
        animationOut.ctrlY2_ = out[3]; // 3: ctrl y2 index
        animationOut.duration_ = DURATION;
        TLOGI(WmsLogTag::WMS_KEYBOARD, "config default animationOut");
    }
}

void SceneSessionManager::ConfigWindowAnimation(const WindowSceneConfig::ConfigItem& windowAnimationConfig)
{
    WindowSceneConfig::ConfigItem item = windowAnimationConfig["timing"];
    if (item.IsMap() && item.mapValue_->count("curve")) {
        const auto& [curveType, curveParams] = CreateCurve(item["curve"]);
        appWindowSceneConfig_.windowAnimation_.curveType_ = curveType;
        if (curveParams.size() == CURVE_PARAM_DIMENSION) {
            appWindowSceneConfig_.windowAnimation_.ctrlX1_ = curveParams[0]; // 0: ctrl x1 index
            appWindowSceneConfig_.windowAnimation_.ctrlY1_ = curveParams[1]; // 1: ctrl y1 index
            appWindowSceneConfig_.windowAnimation_.ctrlX2_ = curveParams[2]; // 2: ctrl x2 index
            appWindowSceneConfig_.windowAnimation_.ctrlY2_ = curveParams[3]; // 3: ctrl y2 index
        }
    }
    item = windowAnimationConfig["timing"]["duration"];
    if (item.IsInts() && item.intsValue_->size() == 1) {
        auto duration = *item.intsValue_;
        appWindowSceneConfig_.windowAnimation_.duration_ = duration[0];
    }
    item = windowAnimationConfig["scale"];
    if (item.IsFloats() && item.floatsValue_->size() == SCALE_DIMENSION) {
        auto scales = *item.floatsValue_;
        appWindowSceneConfig_.windowAnimation_.scaleX_ = scales[0];
        appWindowSceneConfig_.windowAnimation_.scaleY_ = scales[1];
    }
    item = windowAnimationConfig["rotation"];
    if (item.IsFloats() && item.floatsValue_->size() == ROTAION_DIMENSION) {
        auto rotations = *item.floatsValue_;
        appWindowSceneConfig_.windowAnimation_.rotationX_ = rotations[0]; // 0 ctrlX1
        appWindowSceneConfig_.windowAnimation_.rotationY_ = rotations[1]; // 1 ctrlY1
        appWindowSceneConfig_.windowAnimation_.rotationZ_ = rotations[2]; // 2 ctrlX2
        appWindowSceneConfig_.windowAnimation_.angle_ = rotations[3]; // 3 ctrlY2
    }
    item = windowAnimationConfig["translate"];
    if (item.IsFloats() && item.floatsValue_->size() == TRANSLATE_DIMENSION) {
        auto translates = *item.floatsValue_;
        appWindowSceneConfig_.windowAnimation_.translateX_ = translates[0];
        appWindowSceneConfig_.windowAnimation_.translateY_ = translates[1];
    }
    item = windowAnimationConfig["opacity"];
    if (item.IsFloats() && item.floatsValue_->size() == 1) {
        auto opacity = *item.floatsValue_;
        appWindowSceneConfig_.windowAnimation_.opacity_ = opacity[0];
    }
}

void SceneSessionManager::ConfigStartingWindowAnimation(const WindowSceneConfig::ConfigItem& configItem)
{
    auto& config = appWindowSceneConfig_.startingWindowAnimationConfig_;
    auto item = configItem.GetProp("enable");
    if (item.IsBool()) {
        config.enabled_ = item.boolValue_;
    }
    item = configItem["timing"];
    if (item.IsMap() && item.mapValue_->count("curve")) {
        config.curve_ = std::get<std::string>(CreateCurve(item["curve"]));
    }
    item = configItem["timing"]["duration"];
    if (item.IsInts() && item.intsValue_->size() == 1) {
        config.duration_ = (*item.intsValue_)[0];
    }
    item = configItem["opacityStart"];
    if (item.IsFloats() && item.floatsValue_->size() == 1) {
        config.opacityStart_ = (*item.floatsValue_)[0];
    }
    item = configItem["opacityEnd"];
    if (item.IsFloats() && item.floatsValue_->size() == 1) {
        config.opacityEnd_ = (*item.floatsValue_)[0];
    }
}

std::tuple<std::string, std::vector<float>> SceneSessionManager::CreateCurve(
    const WindowSceneConfig::ConfigItem& curveConfig)
{
    static std::unordered_set<std::string> curveSet = { "easeOut", "ease", "easeIn", "easeInOut", "default",
        "linear", "spring", "interactiveSpring", "interpolatingSpring" };
    static std::unordered_set<std::string> paramCurveSet = {
        "spring", "interactiveSpring", "interpolatingSpring", "cubic" };

    std::string curveName = "easeOut";
    const auto& nameItem = curveConfig.GetProp("name");
    if (!nameItem.IsString()) {
        return {curveName, {}};
    }

    std::string name = nameItem.stringValue_;
    std::vector<float> curveParams;

    if (paramCurveSet.find(name) != paramCurveSet.end()) {
        curveName = name;
        curveParams = std::vector<float>(CURVE_PARAM_DIMENSION);
        if (curveConfig.IsFloats() && curveConfig.floatsValue_->size() <= CURVE_PARAM_DIMENSION) {
            std::copy(curveConfig.floatsValue_->begin(), curveConfig.floatsValue_->end(),
                curveParams.begin());
        }
    } else {
        auto iter = curveSet.find(name);
        if (iter != curveSet.end()) {
            curveName = name;
        }
    }

    return {curveName, curveParams};
}


void SceneSessionManager::ConfigSingleHandCompatibleMode(const WindowSceneConfig::ConfigItem& configItem)
{
    auto& config = singleHandCompatibleModeConfig_;
    auto item = configItem.GetProp("enable");
    if (item.IsBool()) {
        config.enabled = item.boolValue_;
    }
    item = configItem["singleHandScale"];
    if (item.IsFloats() && item.floatsValue_->size() == 1) {
        config.singleHandScale = (*item.floatsValue_)[0];
    }
    item = configItem["heightChangeRatio"];
    if (item.IsFloats() && item.floatsValue_->size() == 1) {
        config.heightChangeRatio = (*item.floatsValue_)[0];
    }
    item = configItem["widthChangeRatio"];
    if (item.IsFloats() && item.floatsValue_->size() == 1) {
        config.widthChangeRatio = (*item.floatsValue_)[0];
    }
}

void SceneSessionManager::ConfigAppsWithDeduplicatedWindowStatus()
{
    const auto& config = WindowSceneConfig::GetConfig();
    WindowSceneConfig::ConfigItem item = config["appsWithDeduplicatedWindowStatus"];
    std::vector<std::string> appsWithDeduplicatedWindowStatus;
    if (item.IsStrings()) {
        appsWithDeduplicatedWindowStatus = *item.stringsValue_;
    }
    for (auto&& app : appsWithDeduplicatedWindowStatus) {
        if (!app.empty()) {
            TLOGI(WmsLogTag::WMS_LAYOUT, "app:%{public}s", app.c_str());
            appsWithDeduplicatedWindowStatus_.insert(std::move(app));
        }
    }
}

void SceneSessionManager::SetWindowStatusDeduplicationBySystemConfig(const SessionInfo& sessionInfo,
    SystemSessionConfig& systemConfig)
{
    std::string bundleName = sessionInfo.bundleName_;
    bool deduplicationEnabled = false;
    if (appsWithDeduplicatedWindowStatus_.find(bundleName) != appsWithDeduplicatedWindowStatus_.end()) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "need skip redundant windowStatus notifications, name:%{public}s",
            bundleName.c_str());
        deduplicationEnabled = true;
    }
    systemConfig.skipRedundantWindowStatusNotifications_ = deduplicationEnabled;
}

bool SceneSessionManager::ConfigWindowLayout(const WindowSceneConfig::ConfigItem& windowLayoutConfig)
{
    if (!windowLayoutConfig.IsMap()) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "The windowLayoutConfig is invalid (not a map).");
        return false;
    }
    ConfigMoveDrag(windowLayoutConfig["moveDrag"]);
    return true;
}

bool SceneSessionManager::ConfigMoveDrag(const WindowSceneConfig::ConfigItem& moveDragConfig)
{
    if (!moveDragConfig.IsMap()) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "The moveDrag config is invalid (not a map).");
        return false;
    }
    ConfigMoveResample(moveDragConfig["moveResample"]);
    return true;
}

bool SceneSessionManager::ConfigMoveResample(const WindowSceneConfig::ConfigItem& moveResampleConfig)
{
    if (!moveResampleConfig.IsMap()) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "The moveResample config is invalid (not a map).");
        return false;
    }
    const auto& enableMoveResampleProp = moveResampleConfig.GetProp("enable");
    if (!enableMoveResampleProp.IsBool()) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "The moveResample.enable prop is missing or not a bool.");
        return false;
    }
    bool enable = enableMoveResampleProp.boolValue_;
    const auto& resampleFpsRangeConfig = moveResampleConfig["resampleFpsRange"];
    if (!resampleFpsRangeConfig.IsInts() || !resampleFpsRangeConfig.intsValue_) {
        TLOGW(WmsLogTag::WMS_LAYOUT,
              "The resampleFpsRange config is invalid (not int array), fallback to default.");
        MoveDragController::SetMoveResampleSystemConfig(enable, std::nullopt, std::nullopt);
        return true;
    }
    auto fpsRange = *resampleFpsRangeConfig.intsValue_;
    if (fpsRange.size() != 2) { // 2: only contain minFps and maxFps
        TLOGW(WmsLogTag::WMS_LAYOUT,
              "The resampleFpsRange config size is invalid (expect 2), fallback to default.");
        MoveDragController::SetMoveResampleSystemConfig(enable, std::nullopt, std::nullopt);
        return true;
    }
    int minVal = std::min(fpsRange[0], fpsRange[1]);
    int maxVal = std::max(fpsRange[0], fpsRange[1]);
    MoveDragController::SetMoveResampleSystemConfig(
        enable, static_cast<uint32_t>(minVal), static_cast<uint32_t>(maxVal));
    return true;
}

void SceneSessionManager::ConfigWindowSizeLimits()
{
    const auto& config = WindowSceneConfig::GetConfig();
    WindowSceneConfig::ConfigItem item = config["mainWindowSizeLimits"];
    if (item.IsMap()) {
        ConfigMainWindowSizeLimits(item);
    }

    item = config["subWindowSizeLimits"];
    if (item.IsMap()) {
        ConfigSubWindowSizeLimits(item);
    }

    item = config["dialogWindowSizeLimits"];
    if (item.IsMap()) {
        ConfigDialogWindowSizeLimits(item);
    }
}

void SceneSessionManager::ConfigDialogWindowSizeLimits(const WindowSceneConfig::ConfigItem& dialogWindowSizeConifg)
{
    auto item = dialogWindowSizeConifg["miniWidth"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1) {
            systemConfig_.miniWidthOfDialogWindow_ = static_cast<uint32_t>(numbers[0]);
        }
    }

    item = dialogWindowSizeConifg["miniHeight"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1) {
            systemConfig_.miniHeightOfDialogWindow_ = static_cast<uint32_t>(numbers[0]);
        }
    }
}

void SceneSessionManager::ConfigMainWindowSizeLimits(const WindowSceneConfig::ConfigItem& mainWindowSizeConifg)
{
    auto item = mainWindowSizeConifg["miniWidth"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1) {
            systemConfig_.miniWidthOfMainWindow_ = static_cast<uint32_t>(numbers[0]);
        }
    }

    item = mainWindowSizeConifg["miniHeight"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1) {
            systemConfig_.miniHeightOfMainWindow_ = static_cast<uint32_t>(numbers[0]);
        }
    }
}

void SceneSessionManager::ConfigSubWindowSizeLimits(const WindowSceneConfig::ConfigItem& subWindowSizeConifg)
{
    auto item = subWindowSizeConifg["miniWidth"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1) {
            systemConfig_.miniWidthOfSubWindow_ = static_cast<uint32_t>(numbers[0]);
        }
    }

    item = subWindowSizeConifg["miniHeight"];
    if (item.IsInts()) {
        auto numbers = *item.intsValue_;
        if (numbers.size() == 1) {
            systemConfig_.miniHeightOfSubWindow_ = static_cast<uint32_t>(numbers[0]);
        }
    }
}

void SceneSessionManager::ConfigSnapshotScale()
{
    const auto& config = WindowSceneConfig::GetConfig();
    WindowSceneConfig::ConfigItem item = config["snapshotScale"];
    if (item.IsFloats()) {
        auto snapshotScale = *item.floatsValue_;
        if (snapshotScale.size() != 1 || snapshotScale[0] <= 0 || snapshotScale[0] > 1) {
            return;
        }
        snapshotScale_ = snapshotScale[0];
    }
}

void SceneSessionManager::ConfigSystemUIStatusBar(const WindowSceneConfig::ConfigItem& statusBarConfig)
{
    TLOGI(WmsLogTag::WMS_IMMS, "load");
    WindowSceneConfig::ConfigItem item = statusBarConfig["showInLandscapeMode"];
    if (item.IsInts() && item.intsValue_->size() == 1) {
        bool showInLandscapeMode = (*item.intsValue_)[0] > 0;
        appWindowSceneConfig_.systemUIStatusBarConfig_.showInLandscapeMode_ = showInLandscapeMode;
        TLOGI(WmsLogTag::WMS_IMMS, "showInLandscapeMode %{public}d",
            appWindowSceneConfig_.systemUIStatusBarConfig_.showInLandscapeMode_);
    }

    item = statusBarConfig["immersiveStatusBarBgColor"];
    if (item.IsString()) {
        auto color = item.stringValue_;
        uint32_t colorValue;
        if (!ColorParser::Parse(color, colorValue)) {
            return;
        }
        appWindowSceneConfig_.systemUIStatusBarConfig_.immersiveStatusBarBgColor_ = color;
        TLOGI(WmsLogTag::WMS_IMMS, "immersiveStatusBarBgColor %{public}s",
            appWindowSceneConfig_.systemUIStatusBarConfig_.immersiveStatusBarBgColor_.c_str());
    }

    item = statusBarConfig["immersiveStatusBarContentColor"];
    if (item.IsString()) {
        auto color = item.stringValue_;
        uint32_t colorValue;
        if (!ColorParser::Parse(color, colorValue)) {
            return;
        }
        appWindowSceneConfig_.systemUIStatusBarConfig_.immersiveStatusBarContentColor_ = color;
        TLOGI(WmsLogTag::WMS_IMMS, "immersiveStatusBarContentColor %{public}s",
            appWindowSceneConfig_.systemUIStatusBarConfig_.immersiveStatusBarContentColor_.c_str());
    }
}

void SceneSessionManager::ConfigSupportFollowParentWindowLayout()
{
    TLOGI(WmsLogTag::WMS_SUB, "support");
    auto task = [this] {
        systemConfig_.supportFollowParentWindowLayout_ = true;
    };
    taskScheduler_->PostAsyncTask(task, "ConfigSupportFollowParentWindowLayout");
}

void SceneSessionManager::ConfigSupportFollowRelativePositionToParent()
{
    TLOGI(WmsLogTag::WMS_SUB, "support");
    auto task = [this] {
        systemConfig_.supportFollowRelativePositionToParent_ = true;
    };
    taskScheduler_->PostAsyncTask(task, "ConfigSupportFollowRelativePositionToParent");
}

void SceneSessionManager::SetRootSceneContext(const std::weak_ptr<AbilityRuntime::Context>& contextWeak)
{
    rootSceneContextWeak_ = contextWeak;
}

void SceneSessionManager::CreateRootSceneSession()
{
    system::SetParameter("bootevent.wms.fullscreen.ready", "true");
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onGetSceneSessionVectorByTypeAndDisplayId_ = [this](WindowType type, uint64_t displayId) {
        return this->GetSceneSessionVectorByTypeAndDisplayId(type, displayId);
    };
    specificCb->onGetSceneSessionVectorByType_ = [this](WindowType type) {
        return this->GetSceneSessionVectorByType(type);
    };
    specificCb->onGetAINavigationBarArea_ = [this](uint64_t displayId, bool ignoreVisibility) {
        return this->GetAINavigationBarArea(displayId, ignoreVisibility);
    };
    specificCb->onUpdateAvoidArea_ = [this](int32_t persistentId) {
        this->UpdateAvoidArea(persistentId);
    };
    specificCb->onNotifyAvoidAreaChange_ = [this](const sptr<AvoidArea>& avoidArea, AvoidAreaType type) {
        onNotifyAvoidAreaChangeForRootFunc_(avoidArea, type, nullptr);
    };
    rootSceneSession_ = sptr<RootSceneSession>::MakeSptr(specificCb);
    rootSceneSession_->isKeyboardPanelEnabled_ = isKeyboardPanelEnabled_;
    rootSceneSession_->SetEventHandler(taskScheduler_->GetEventHandler());
    rootSceneSession_->SetSystemConfig(systemConfig_);
    rootSceneSession_->RegisterGetStatusBarAvoidHeightFunc([this](DisplayId displayId, WSRect& barArea) {
        return this->GetStatusBarAvoidHeight(displayId, barArea);
    });
    rootSceneSession_->RegisterGetStatusBarConstantlyShowFunc([this](DisplayId displayId, bool& isVisible) {
        return this->GetStatusBarConstantlyShow(displayId, isVisible);
    });
}

void SceneSessionManager::RegisterRootSceneSession()
{
    if (rootSceneSession_ == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "rootSceneSession is null");
        return;
    }
    AppExecFwk::RunningProcessInfo info;
    DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetRunningProcessInfoByPid(getpid(), info);
    TLOGI(WmsLogTag::WMS_LIFE, "pid: %{public}d, processName: %{public}s, currentUserId: %{public}d", getpid(),
        info.processName_.c_str(), currentUserId_.load());
    if (SCENE_BOARD_BUNDLE_NAME == info.processName_ && currentUserId_ != DEFAULT_USERID) {
        AAFwk::AbilityManagerClient::GetInstance()->SetRootSceneSession(rootSceneSession_->AsObject());
        TLOGI(WmsLogTag::WMS_LIFE, "SetRootSceneSession success.");
    }
}

void SceneSessionManager::RegisterWindowStateErrorCallbackToMMI()
{
    // MMI window state error check
    int32_t retCode =
        MMI::InputManager::GetInstance()->RegisterWindowStateErrorCallback([this](int32_t pid, int32_t persistentId) {
        this->NotifyWindowStateErrorFromMMI(pid, persistentId);
    });
    TLOGW(WmsLogTag::WMS_EVENT, "register WindowStateError callback to MMI with ret: %{public}d", retCode);
}    

sptr<RootSceneSession> SceneSessionManager::GetRootSceneSession()
{
    return rootSceneSession_;
}

void SceneSessionManager::UpdateRootSceneAvoidArea()
{
    UpdateAvoidArea(rootSceneSession_->GetPersistentId());
}

void SceneSessionManager::RegisterNotifyRootSceneAvoidAreaChangeFunc(NotifyRootSceneAvoidAreaChangeFunc&& func)
{
    onNotifyAvoidAreaChangeForRootFunc_ = std::move(func);
}

AvoidArea SceneSessionManager::GetRootSessionAvoidAreaByType(AvoidAreaType type, bool ignoreVisibility)
{
    if (auto rootSession = GetRootSceneSession()) {
        return ignoreVisibility ?
            rootSession->GetAvoidAreaByType(type) : rootSession->GetAvoidAreaByTypeIgnoringVisibility(type);
    }
    return {};
}

uint32_t SceneSessionManager::GetRootSceneStatusBarHeight() const
{
    return static_cast<uint32_t>(rootSceneSession_->GetStatusBarHeight());
}

sptr<SceneSession> SceneSessionManager::GetSceneSession(int32_t persistentId)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    if (auto it = sceneSessionMap_.find(persistentId); it != sceneSessionMap_.end()) {
        return it->second;
    }
    TLOGD(WmsLogTag::DEFAULT, "Not found scene session with id: %{public}d", persistentId);
    return nullptr;
}

bool SceneSessionManager::CheckAndGetAbilityInfoByWant(const std::shared_ptr<AAFwk::Want>& want,
    AppExecFwk::AbilityInfo& abilityInfo)
{
    if (!bundleMgr_) {
        TLOGE(WmsLogTag::WMS_LIFE, "bundleMgr is nullptr");
        return false;
    }
    if (!want) {
        TLOGE(WmsLogTag::WMS_LIFE, "want is nullptr");
        return false;
    }
    if (!bundleMgr_->QueryAbilityInfo(*want,
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, currentUserId_, abilityInfo)) {
        TLOGE(WmsLogTag::WMS_LIFE, "ability not found %{public}s %{public}s %{public}s",
            want->GetElement().GetBundleName().c_str(), want->GetElement().GetModuleName().c_str(),
            want->GetElement().GetAbilityName().c_str());
        return false;
    }
    return true;
}

bool SceneSessionManager::IsMainWindowByPersistentId(int32_t persistentId)
{
    if (persistentId <= INVALID_SESSION_ID) {
        return false;
    }
    if (auto sceneSession = GetSceneSession(persistentId)) {
        return SessionHelper::IsMainWindow(sceneSession->GetWindowType());
    }
    return false;
}

sptr<SceneSession> SceneSessionManager::GetMainSessionByPersistentId(int32_t persistentId) const
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    auto it = sceneSessionMap_.find(persistentId);
    if (it != sceneSessionMap_.end() && it->second && SessionHelper::IsMainWindow(it->second->GetWindowType())) {
        return it->second;
    }
    return nullptr;
}

void SceneSessionManager::GetMainSessionByBundleNameAndAppIndex(
    const std::string& bundleName, int32_t appIndex, std::vector<sptr<SceneSession>>& mainSessions)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession && sceneSession->GetSessionInfo().bundleName_ == bundleName &&
            sceneSession->GetSessionInfo().appIndex_ == appIndex &&
            SessionHelper::IsMainWindow(sceneSession->GetWindowType())) {
            mainSessions.push_back(sceneSession);
        }
    }
}

void SceneSessionManager::GetMainSessionByAbilityInfo(const AbilityInfoBase& abilityInfo,
    std::vector<sptr<SceneSession>>& mainSessions) const
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (!sceneSession || !SessionHelper::IsMainWindow(sceneSession->GetWindowType())) {
            continue;
        }
        if (sceneSession->GetSessionInfo().bundleName_ == abilityInfo.bundleName &&
            sceneSession->GetSessionInfo().moduleName_ == abilityInfo.moduleName &&
            sceneSession->GetSessionInfo().abilityName_ == abilityInfo.abilityName &&
            sceneSession->GetSessionInfo().appIndex_ == abilityInfo.appIndex) {
            mainSessions.push_back(sceneSession);
        }
    }
}

sptr<SceneSession> SceneSessionManager::GetSceneSessionByIdentityInfo(const SessionIdentityInfo& info)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (!sceneSession) {
            return nullptr;
        }
        if (sceneSession->GetSessionInfo().bundleName_ != info.bundleName_ ||
            sceneSession->GetSessionInfo().appIndex_ != info.appIndex_ ||
            sceneSession->GetSessionInfo().appInstanceKey_ != info.instanceKey_ ||
            sceneSession->GetSessionInfo().specifiedFlag_ != info.specifiedFlag_ ||
            sceneSession->GetSessionInfo().windowType_ != info.windowType_) {
            continue;
        }
        const auto& sessionModuleName = sceneSession->GetSessionInfo().moduleName_;
        const auto& sessionAbilityName = sceneSession->GetSessionInfo().abilityName_;
        if (info.isAtomicService_) {
            if ((sessionModuleName.empty() || sessionModuleName == info.moduleName_) &&
                (sessionAbilityName.empty() || sessionAbilityName == info.abilityName_)) {
                return sceneSession;
            }
        } else if (sessionModuleName == info.moduleName_ && sessionAbilityName == info.abilityName_) {
            return sceneSession;
        }
    }
    return nullptr;
}

sptr<SceneSession> SceneSessionManager::GetSceneSessionByType(WindowType type)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession && sceneSession->GetWindowType() == type) {
            return sceneSession;
        }
    }
    return nullptr;
}

std::vector<sptr<SceneSession>> SceneSessionManager::GetSceneSessionByBundleName(const std::string& bundleName)
{
    std::vector<sptr<SceneSession>> sceneSessionVector;
    if (bundleName.empty()) {
        return sceneSessionVector;
    }
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession && sceneSession->GetSessionInfo().bundleName_ == bundleName) {
            sceneSessionVector.emplace_back(sceneSession);
        }
    }
    return sceneSessionVector;
}

std::vector<sptr<SceneSession>> SceneSessionManager::GetSceneSessionVectorByTypeAndDisplayId(
    WindowType type, uint64_t displayId)
{
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::WMS_LIFE, "displayId is invalid");
        return {};
    }
    std::vector<sptr<SceneSession>> sceneSessionVector;
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession->GetWindowType() == type &&
            sceneSession->GetSessionProperty()->GetDisplayId() == displayId) {
            sceneSessionVector.emplace_back(sceneSession);
        }
    }
    return sceneSessionVector;
}

std::vector<sptr<SceneSession>> SceneSessionManager::GetSceneSessionVectorByType(WindowType type)
{
    std::vector<sptr<SceneSession>> sceneSessionVector;
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession->GetWindowType() == type) {
            sceneSessionVector.emplace_back(sceneSession);
        }
    }
    return sceneSessionVector;
}

WSError SceneSessionManager::UpdateParentSessionForDialog(const sptr<SceneSession>& sceneSession,
    sptr<WindowSessionProperty> property)
{
    if (sceneSession == nullptr || property == nullptr) {
        TLOGD(WmsLogTag::WMS_DIALOG, "Session or property is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    auto parentPersistentId = property->GetParentPersistentId();
    sceneSession->SetParentPersistentId(parentPersistentId);
    if (property->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG && parentPersistentId != INVALID_SESSION_ID) {
        auto parentSession = GetSceneSession(parentPersistentId);
        if (parentSession == nullptr) {
            TLOGE(WmsLogTag::WMS_DIALOG, "Parent session is nullptr, parentId:%{public}d", parentPersistentId);
            return WSError::WS_ERROR_NULLPTR;
        }
        sceneSession->GetSessionProperty()->SetSubWindowZLevel(property->GetSubWindowZLevel());
        parentSession->BindDialogSessionTarget(sceneSession);
        parentSession->BindDialogToParentSession(sceneSession);
        sceneSession->SetParentSession(parentSession);
        TLOGI(WmsLogTag::WMS_DIALOG, "Update parent of dialog success, id %{public}d, parentId %{public}d",
            sceneSession->GetPersistentId(), parentPersistentId);
    }
    return WSError::WS_OK;
}

sptr<SceneSession::SpecificSessionCallback> SceneSessionManager::CreateSpecificSessionCallback()
{
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onCreate_ = [this](const SessionInfo& sessionInfo, sptr<WindowSessionProperty> property) {
        return this->RequestSceneSession(sessionInfo, property);
    };
    specificCb->onDestroy_ = [this](const int32_t persistentId) {
        return this->DestroyAndDisconnectSpecificSessionInner(persistentId);
    };
    specificCb->onClearDisplayStatusBarTemporarilyFlags_ = [this] {
        this->ClearDisplayStatusBarTemporarilyFlags();
    };
    specificCb->onCameraFloatSessionChange_ = [this](uint32_t accessTokenId, bool isShowing) {
        this->UpdateCameraFloatWindowStatus(accessTokenId, isShowing);
    };
    specificCb->onGetSceneSessionVectorByTypeAndDisplayId_ = [this](WindowType type, uint64_t displayId) {
        return this->GetSceneSessionVectorByTypeAndDisplayId(type, displayId);
    };
    specificCb->onGetSceneSessionVectorByType_ = [this](WindowType type) {
        return this->GetSceneSessionVectorByType(type);
    };
    specificCb->onUpdateAvoidArea_ = [this](int32_t persistentId) {
        this->UpdateAvoidArea(persistentId);
    };
    specificCb->onGetStatusBarDefaultVisibilityByDisplayId_ = [this](DisplayId displayId) {
        return this->GetStatusBarDefaultVisibilityByDisplayId(displayId);
    };
    specificCb->onWindowInfoUpdate_ = [this](int32_t persistentId, WindowUpdateType type) {
        this->NotifyWindowInfoChange(persistentId, type);
    };
    specificCb->onWindowInputPidChangeCallback_ = [this](int32_t windowId, bool startMoving) {
        this->NotifyMMIWindowPidChange(windowId, startMoving);
    };
    specificCb->onSessionTouchOutside_ = [this](int32_t persistentId, DisplayId displayId) {
        this->NotifySessionTouchOutside(persistentId, displayId);
    };
    specificCb->onGetAINavigationBarArea_ = [this](uint64_t displayId, bool ignoreVisibility) {
        return this->GetAINavigationBarArea(displayId, ignoreVisibility);
    };
    specificCb->onGetNextAvoidAreaRectInfo_ = [this](
        DisplayId displayId, AvoidAreaType type, std::pair<WSRect, WSRect>& nextSystemBarAvoidAreaRectInfo) {
        return this->GetNextAvoidRectInfo(displayId, type, nextSystemBarAvoidAreaRectInfo);
    };
    specificCb->onGetLSState_ = [this]() {
        return this->GetLSState();
    };
    specificCb->onOutsideDownEvent_ = [this](int32_t x, int32_t y) {
        this->OnOutsideDownEvent(x, y);
    };
    specificCb->onHandleSecureSessionShouldHide_ = [this](const sptr<SceneSession>& sceneSession) {
        return this->HandleSecureSessionShouldHide(sceneSession);
    };
    specificCb->onNotifyWindowSystemBarPropertyChangeFunc_ = [this](
        WindowType type, const SystemBarProperty& systemBarProperty) {
        return this->NotifyWindowSystemBarPropertyChange(type, systemBarProperty);
    };
    specificCb->onCameraSessionChange_ = [this](uint32_t accessTokenId, bool isShowing) {
        this->UpdateCameraWindowStatus(accessTokenId, isShowing);
    };
    specificCb->onSetSkipSelfWhenShowOnVirtualScreen_ = [this](uint64_t surfaceNodeId, bool isSkip) {
        this->SetSkipSelfWhenShowOnVirtualScreen(surfaceNodeId, isSkip);
    };
    specificCb->onSetSkipEventOnCastPlus_ = [this](int32_t windowId, bool isSkip) {
        this->SetSkipEventOnCastPlusInner(windowId, isSkip);
    };
    specificCb->onPiPStateChange_ = [this](const std::string& bundleName, bool isForeground) {
        this->UpdatePiPWindowStateChanged(bundleName, isForeground);
    };
    specificCb->onUpdateGestureBackEnabled_ = [this](int32_t persistentId) {
        this->UpdateGestureBackEnabled(persistentId);
    };
    specificCb->onKeyboardRotationChange_ = [this](int32_t persistentId, Rotation rotation,
        std::vector<std::pair<bool, WSRect>>& avoidAreas) {
        this->GetKeyboardOccupiedAreaWithRotation(persistentId, rotation, avoidAreas);
    };
    specificCb->onGetSceneSessionByIdCallback_ = [this](int32_t persistentId) {
        return this->GetSceneSession(persistentId);
    };
    specificCb->onCheckAndGetAbilityInfoByWantCallback_ = [this](const std::shared_ptr<AAFwk::Want>& want,
        AppExecFwk::AbilityInfo& abilityInfo) {
        return this->CheckAndGetAbilityInfoByWant(want, abilityInfo);
    };
    return specificCb;
}

void SceneSessionManager::SetSkipSelfWhenShowOnVirtualScreen(uint64_t surfaceNodeId, bool isSkip)
{
    TLOGI(WmsLogTag::WMS_SCB, "surfaceNodeId: %{public}" PRIu64, surfaceNodeId);
    auto it = std::find(skipSurfaceNodeIds_.begin(), skipSurfaceNodeIds_.end(), surfaceNodeId);
    if (isSkip) {
        if (it == skipSurfaceNodeIds_.end()) {
            skipSurfaceNodeIds_.push_back(surfaceNodeId);
        } else {
            return;
        }
    } else {
        if (it != skipSurfaceNodeIds_.end()) {
            skipSurfaceNodeIds_.erase(it);
        } else {
            return;
        }
    }
    rsInterface_.SetVirtualScreenBlackList(INVALID_SCREEN_ID, skipSurfaceNodeIds_);
}

WMError SceneSessionManager::AddSkipSelfWhenShowOnVirtualScreenList(const std::vector<int32_t>& persistentIds)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, &persistentIds] {
        for (const auto persistentId : persistentIds) {
            auto session = GetSceneSession(persistentId);
            if (!session || SessionHelper::IsSubWindow(session->GetWindowType())) {
                TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "[win: %{public}d] not found or is sub window", persistentId);
                continue;
            }
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "[win: %{public}d] add to virtual screen black list", persistentId);
            auto surfaceNode = session->GetSurfaceNode();
            if (surfaceNode) {
                auto surfaceNodeId = surfaceNode->GetId();
                if (std::count(skipSurfaceNodeIds_.begin(), skipSurfaceNodeIds_.end(), surfaceNodeId) == 0) {
                    skipSurfaceNodeIds_.push_back(surfaceNodeId);
                }
            }
            auto leashWinSurfaceNode = session->GetLeashWinSurfaceNode();
            if (leashWinSurfaceNode) {
                auto leashWinSurfaceNodeId = leashWinSurfaceNode->GetId();
                if (std::count(skipSurfaceNodeIds_.begin(), skipSurfaceNodeIds_.end(), leashWinSurfaceNodeId) == 0) {
                    skipSurfaceNodeIds_.push_back(leashWinSurfaceNodeId);
                }
            }
            SetSkipEventOnCastPlusInner(persistentId, true);
        }
        if (!isUserBackground_) {
            rsInterface_.SetVirtualScreenBlackList(INVALID_SCREEN_ID, skipSurfaceNodeIds_);
        }
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

WMError SceneSessionManager::RemoveSkipSelfWhenShowOnVirtualScreenList(const std::vector<int32_t>& persistentIds)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, &persistentIds] {
        for (const auto persistentId : persistentIds) {
            auto session = GetSceneSession(persistentId);
            if (!session || SessionHelper::IsSubWindow(session->GetWindowType())) {
                TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "[win: %{public}d] not found or is sub window", persistentId);
                continue;
            }
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "[win: %{public}d] remove from virtual screen black list", persistentId);
            auto surfaceNode = session->GetSurfaceNode();
            if (surfaceNode) {
                auto surfaceNodeId = surfaceNode->GetId();
                auto iter = std::find(skipSurfaceNodeIds_.begin(), skipSurfaceNodeIds_.end(), surfaceNodeId);
                if (iter != skipSurfaceNodeIds_.end()) {
                    skipSurfaceNodeIds_.erase(iter);
                }
            }
            auto leashWinSurfaceNode = session->GetLeashWinSurfaceNode();
            if (leashWinSurfaceNode) {
                auto leashWinSurfaceNodeId = leashWinSurfaceNode->GetId();
                auto iter = std::find(skipSurfaceNodeIds_.begin(), skipSurfaceNodeIds_.end(), leashWinSurfaceNodeId);
                if (iter != skipSurfaceNodeIds_.end()) {
                    skipSurfaceNodeIds_.erase(iter);
                }
            }
            SetSkipEventOnCastPlusInner(persistentId, false);
        }
        if (!isUserBackground_) {
            rsInterface_.SetVirtualScreenBlackList(INVALID_SCREEN_ID, skipSurfaceNodeIds_);
        }
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

WMError SceneSessionManager::SetScreenPrivacyWindowTagSwitch(
    uint64_t screenId, const std::vector<std::string>& privacyWindowTags, bool enable)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "screenId: %{public}" PRIu64 ", tagsize: %{public}zu, enable: %{public}d",
        screenId, privacyWindowTags.size(), enable);
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    auto task = [this, screenId, &privacyWindowTags, enable]() {
        if (enable) {
            for (const auto& privacyWindowTag : privacyWindowTags) {
                screenRSBlackListConfigMap_[screenId].insert({ .privacyWindowTag = privacyWindowTag });
            }
            FlushSessionBlackListInfoMapWhenAdd(screenId);
        } else {
            for (const auto& privacyWindowTag : privacyWindowTags) {
                screenRSBlackListConfigMap_[screenId].erase({ .privacyWindowTag = privacyWindowTag });
                if (screenRSBlackListConfigMap_[screenId].empty()) {
                    screenRSBlackListConfigMap_.erase(screenId);
                    break;
                }
            }
            FlushSessionBlackListInfoMapWhenRemove(screenId);
        }
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

WMError SceneSessionManager::NotifyBrightnessModeChange(const std::string& brightnessMode)
{
    if (GetDisplayBrightness() == UNDEFINED_BRIGHTNESS) {
        return WMError::WM_DO_NOTHING;
    }
    auto brightnessSession = GetSceneSession(brightnessSessionId_);
    if (brightnessSession != nullptr && brightnessSession->IsSessionForeground()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Restore brightness, wid: %{public}d", brightnessSession->GetWindowId());
        PostBrightnessTask(UNDEFINED_BRIGHTNESS);
        brightnessSession->SetBrightness(UNDEFINED_BRIGHTNESS);
        return WMError::WM_OK;
    }
    return WMError::WM_DO_NOTHING;
}

void SceneSessionManager::SetSkipEventOnCastPlusInner(int32_t windowId, bool isSkip)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Wid: %{public}d, isSkip: %{public}d", windowId, isSkip);
    auto sceneSession = GetSceneSession(windowId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "SceneSession is nullptr, Wid: %{public}d", windowId);
        return;
    }
    sceneSession->GetSessionProperty()->SetSkipEventOnCastPlus(isSkip);
    if (!isUserBackground_) {
        FlushWindowInfoToMMI(true);
    }
}

sptr<KeyboardSession::KeyboardSessionCallback> SceneSessionManager::CreateKeyboardSessionCallback()
{
    auto keyboardCb = sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    keyboardCb->onGetSceneSession = [this](int32_t persistentId) {
        return this->GetSceneSession(persistentId);
    };
    keyboardCb->onGetFocusedSessionId = [this] {
        return this->GetFocusedSessionId();
    };
    keyboardCb->onSystemKeyboardAvoidChange = [this](DisplayId displayId, SystemKeyboardAvoidChangeReason reason) {
        this->HandleKeyboardAvoidChange(nullptr, displayId, reason);
    };
    keyboardCb->onNotifyOccupiedAreaChange = [this](const sptr<OccupiedAreaChangeInfo>& info) {
        this->onNotifyAvoidAreaChangeForRootFunc_(nullptr, AvoidAreaType::TYPE_KEYBOARD, info);
    };
    keyboardCb->isLastFrameLayoutFinished = [this]() {
        if (isRootSceneLastFrameLayoutFinishedFunc_ == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "isRootSceneLastFrameLayoutFinishedFunc_ is nullptr");
            return true;
        }
        return isRootSceneLastFrameLayoutFinishedFunc_();
    };
    return keyboardCb;
}

WMError SceneSessionManager::CheckWindowId(int32_t windowId, int32_t& pid)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_EVENT, "permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }

    auto task = [this, windowId, &pid] {
        pid = INVALID_PID;
        auto sceneSession = GetSceneSession(windowId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "sceneSession(%{public}d) is nullptr", windowId);
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        pid = sceneSession->GetCallingPid();
        TLOGND(WmsLogTag::WMS_EVENT, "Window(%{public}d) to set the cursor style, pid:%{public}d", windowId, pid);
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, "CheckWindowId:" + std::to_string(windowId));
}

WMError SceneSessionManager::GetWindowLimits(int32_t windowId, WindowLimits& windowLimits)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto sceneSession = GetSceneSession(windowId);
    if (!sceneSession) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "sceneSession(%{public}d) is nullptr", windowId);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (!sessionProperty) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "sessionProperty(%{public}d) is nullptr", windowId);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    WindowLimits userWindowLimits = sessionProperty->GetUserWindowLimits();
    bool useVPLimits = (userWindowLimits.pixelUnit_ == PixelUnit::VP);
    windowLimits = useVPLimits ? sessionProperty->GetWindowLimitsVP() : sessionProperty->GetWindowLimits();
    bool isInitialized = !windowLimits.IsUninitialized();
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "GetWindowLimits minWidth:%{public}u, minHeight:%{public}u, "
        "maxWidth:%{public}u, maxHeight:%{public}u, vpRatio:%{public}f, pixelUnit:%{public}u, "
        "isInitialized:%{public}u", windowLimits.minWidth_, windowLimits.minHeight_, windowLimits.maxWidth_,
        windowLimits.maxHeight_, windowLimits.vpRatio_, windowLimits.pixelUnit_, isInitialized);
    return WMError::WM_OK;
}

void SceneSessionManager::ConfigDockAutoHide(bool isDockAutoHide) {
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "ConfigDockAutoHide: isDockAutoHide %{public}d", isDockAutoHide);
    auto task = [this, isDockAutoHide] {
        systemConfig_.isDockAutoHide_ = isDockAutoHide;
    };
    return taskScheduler_->PostAsyncTask(task, "ConfigDockAutoHide");
}

  void SceneSessionManager::SchedulePcAppInPadLifecycle(bool isBackground)
{
    auto task = [this, isBackground] {
        std::map<int32_t, sptr<SceneSession>> sceneSessionMapCopy;
        {
            std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
            sceneSessionMapCopy = sceneSessionMap_;
        }
        for (const auto& [_, sceneSession] : sceneSessionMapCopy) {
            if (sceneSession == nullptr) {
                TLOGE(WmsLogTag::WMS_LIFE, "session is null");
                continue;
            }
            if (!WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
                continue;
            }
            bool isPcAppInPad = sceneSession->GetSessionProperty()->GetIsPcAppInPad();
            if (!isPcAppInPad) {
                continue;
            }
            StartOrMinimizePcAppInPadUIAbilityBySCB(sceneSession, isBackground);
        }
    };
    return taskScheduler_->PostAsyncTask(task, "SchedulePcAppInPadLifecycle");
}

WSError SceneSessionManager::StartOrMinimizePcAppInPadUIAbilityBySCB(const sptr<SceneSession>& sceneSession, bool isBackground)
{
    auto sessionState = sceneSession->GetSessionState();
    auto isInvalidMainSession = sessionState == SessionState::STATE_DISCONNECT ||
                                sessionState == SessionState::STATE_END;
    if (isInvalidMainSession) {
        return WSError::WS_DO_NOTHING;
    }
    bool isTrayApp = !trayAppList_.empty() && 
                     std::find(trayAppList_.begin(), trayAppList_.end(), sceneSession->GetSessionInfo().bundleName_) !=
                     trayAppList_.end();
    if (isTrayApp) {
        return WSError::WS_DO_NOTHING;
    }
    auto persistentId = sceneSession->GetPersistentId();
    auto abilitySessionInfo = SetAbilitySessionInfo(sceneSession);
    if (isBackground) {
        TLOGI(WmsLogTag::WMS_LIFE,
            "MinimizePcAppInPadUIAbilityBySCB with persistentId: %{public}d, type: %{public}d, state: %{public}d", persistentId,
            sceneSession->GetWindowType(), sceneSession->GetSessionState());
        sceneSession->UpdateLifecyclePausedInner();
        int32_t errCode = AAFwk::AbilityManagerClient::GetInstance()->MinimizeUIAbilityBySCB(abilitySessionInfo);
        if (errCode != ERR_OK) {
            TLOGE(WmsLogTag::WMS_LIFE, "minimize failed! errCode: %{public}d", errCode);
        }
    } else {
        TLOGI(WmsLogTag::WMS_LIFE,
            "StartPcAppInPadUIAbilityBySCB with persistentId: %{public}d, type: %{public}d, state: %{public}d", persistentId,
            sceneSession->GetWindowType(), sceneSession->GetSessionState());
        bool isColdStart = false;
        abilitySessionInfo->isNewWant = false;
        int32_t errCode = StartUIAbilityBySCBTimeoutCheck(sceneSession,
            abilitySessionInfo, static_cast<uint32_t>(WindowStateChangeReason::NORMAL), isColdStart);
        if (errCode != ERR_OK) {
            TLOGE(WmsLogTag::WMS_LIFE, "start failed! errCode: %{public}d", errCode);
            ExceptionInfo exceptionInfo;
            exceptionInfo.needRemoveSession = true;
            sceneSession->NotifySessionExceptionInner(abilitySessionInfo, exceptionInfo, false, true);
            if (startUIAbilityErrorFunc_ && static_cast<WSError>(errCode) == WSError::WS_ERROR_EDM_CONTROLLED) {
                startUIAbilityErrorFunc_(
                    static_cast<uint32_t>(WS_JS_TO_ERROR_CODE_MAP.at(WSError::WS_ERROR_EDM_CONTROLLED)));
            }
        }
    }
    return WSError::WS_OK;
}

void SceneSessionManager::SetTrayAppList(std::vector<std::string>&& trayAppList)
{
    trayAppList_ = std::move(trayAppList);
}

uint32_t SceneSessionManager::GetLockScreenZOrder()
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [persistentId, session] : sceneSessionMap_) {
        if (session && session->IsScreenLockWindow()) {
            TLOGI(WmsLogTag::WMS_UIEXT, "window %{public}d-%{public}d", persistentId,
                session->GetZOrder());
            return session->GetZOrder() < DEFAULT_LOCK_SCREEN_ZORDER ? DEFAULT_LOCK_SCREEN_ZORDER :
                session->GetZOrder();
        }
    }
    TLOGE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: not found");
    return DEFAULT_LOCK_SCREEN_ZORDER;
}

WMError SceneSessionManager::CheckUIExtensionCreation(int32_t windowId, uint32_t callingTokenId,
    const AppExecFwk::ElementName& element, AppExecFwk::ExtensionAbilityType extensionAbilityType, int32_t& pid)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, windowId, callingTokenId, &element, extensionAbilityType, &pid] {
        pid = INVALID_PID;
        auto sceneSession = GetSceneSession(windowId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: sceneSession(%{public}d) is nullptr", windowId);
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        pid = sceneSession->GetCallingPid();
        if (!IsScreenLocked()) {
            TLOGND(WmsLogTag::WMS_UIEXT, "UIExtOnLock: not in lock screen");
            return WMError::WM_OK;
        }
        if (IsUserAuthPassed()) {
            TLOGNI(WmsLogTag::WMS_UIEXT, "UIExtOnLock: auth passed");
            return WMError::WM_OK;
        }
        // 1. check window whether can show on main window
        if (!sceneSession->IsShowOnLockScreen(GetLockScreenZOrder())) {
            TLOGNI(WmsLogTag::WMS_UIEXT, "UIExtOnLock: not called on lock screen");
            return WMError::WM_OK;
        }
        // 2. check permission
        if (!IsUIExtCanShowOnLockScreen(element, callingTokenId, extensionAbilityType)) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: no permisson, window id %{public}d, %{public}d", windowId,
                callingTokenId);
            return WMError::WM_ERROR_INVALID_PERMISSION;
        }
        TLOGNI(WmsLogTag::WMS_UIEXT, "UIExtOnLock: pass");
        return WMError::WM_OK;
    };

    std::ostringstream ss;
    ss << "UIExtOnLockCheck" << "_" << windowId << "_" << callingTokenId;
    return taskScheduler_->PostSyncTask(task, ss.str());
}

// windowIds are all main window
void SceneSessionManager::OnNotifyAboveLockScreen(const std::vector<int32_t>& windowIds)
{
    taskScheduler_->PostSyncTask([this, &windowIds] {
        // check every window
        for (auto windowId : windowIds) {
            auto sceneSession = GetSceneSession(windowId);
            if (!sceneSession) {
                TLOGNE(WmsLogTag::WMS_UIEXT, "UIExtOnLock: sesssion is null for %{public}d", windowId);
                continue;
            }
            TLOGNI(WmsLogTag::WMS_UIEXT, "UIExtOnLock: check for %{public}d", windowId);
            sceneSession->OnNotifyAboveLockScreen();
        }
        return WMError::WM_OK;
    }, __func__);
}

sptr<SceneSession> SceneSessionManager::GetKeyboardSession(DisplayId displayId, bool isSystemKeyboard)
{
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "displayId is invalid.");
        return nullptr;
    }
    sptr<SceneSession> keyboardSession = nullptr;
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession && sceneSession->GetScreenId() == displayId &&
            sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT &&
            sceneSession->IsSystemKeyboard() == isSystemKeyboard) {
            keyboardSession = sceneSession;
            break;
        }
    }
    return keyboardSession;
}

void SceneSessionManager::HandleKeyboardAvoidChange(const sptr<SceneSession>& sceneSession, DisplayId displayId,
    SystemKeyboardAvoidChangeReason reason)
{
    if (!systemConfig_.IsPcWindow()) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "this device is not pc.");
        return;
    }
    switch (reason) {
        // if the system keyboard's avoid area is active first, deactivate the other keyboard's avoid area
        case SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED: {
            if (!sceneSession || sceneSession->IsSystemKeyboard()) {
                return;
            }
            sptr<SceneSession> systemKeyboardSession = GetKeyboardSession(displayId, true);
            if (systemKeyboardSession && systemKeyboardSession->IsSessionForeground() &&
                systemKeyboardSession->GetKeyboardGravity() == SessionGravity::SESSION_GRAVITY_BOTTOM) {
                sceneSession->ActivateKeyboardAvoidArea(false, false);
            }
            break;
        }
        /*
         * activate the system keyboard's avoid area, while it is showing or it's gravity is bottom
         * and deactivate other keyboard's avoid area
         */
        case SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW:
        case SystemKeyboardAvoidChangeReason::KEYBOARD_GRAVITY_BOTTOM: {
            UpdateKeyboardAvoidAreaActive(true);
            break;
        }
        /*
         * when the system keyboard is hiden, disconnect or it's gravity is float
         * check for whether other keyboard can be avoided: if yes, avoids the other keyboard
         *                                                  if no, restores the system keyboard
         */
        case SystemKeyboardAvoidChangeReason::KEYBOARD_HIDE:
        case SystemKeyboardAvoidChangeReason::KEYBOARD_DISCONNECT:
        case SystemKeyboardAvoidChangeReason::KEYBOARD_GRAVITY_FLOAT: {
            bool keyboardRecalculate = false;
            if (auto keyboardSession = GetKeyboardSession(displayId, false)) {
                if (keyboardSession->IsSessionForeground() &&
                    keyboardSession->GetKeyboardGravity() == SessionGravity::SESSION_GRAVITY_BOTTOM) {
                    keyboardRecalculate = true;
                }
                keyboardSession->ActivateKeyboardAvoidArea(true, keyboardRecalculate);
            }
            if (auto sysKeyboardSession = GetKeyboardSession(displayId, true)) {
                sysKeyboardSession->ActivateKeyboardAvoidArea(false, !keyboardRecalculate);
            }
            break;
        }
        default:
            break;
    }
}

void SceneSessionManager::UpdateKeyboardAvoidAreaActive(bool systemKeyboardAvoidAreaActive)
{
    const auto& keyboardSessionVec = GetSceneSessionVectorByType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    if (keyboardSessionVec.empty()) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "there is no keyboard window in the map");
        return;
    }
    for (const auto& keyboardSession : keyboardSessionVec) {
        if (!keyboardSession) {
            continue;
        }
        if (keyboardSession->IsSystemKeyboard()) {
            keyboardSession->ActivateKeyboardAvoidArea(systemKeyboardAvoidAreaActive, false);
        } else {
            keyboardSession->ActivateKeyboardAvoidArea(!systemKeyboardAvoidAreaActive, false);
        }
    }
}

sptr<SceneSession> SceneSessionManager::RequestKeyboardPanelSession(const std::string& panelName, uint64_t displayId)
{
    SessionInfo panelInfo = {
        .bundleName_ = panelName,
        .moduleName_ = panelName,
        .abilityName_ = panelName,
        .isSystem_ = true,
        .sceneType_ = SceneType::SYSTEM_WINDOW_SCENE,
        .windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_KEYBOARD_PANEL),
        .screenId_ = displayId,
        .isRotable_ = true,
    };
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Set panel surfaceNode");
    return RequestSceneSession(panelInfo, nullptr);
}

void SceneSessionManager::CreateKeyboardPanelSession(sptr<SceneSession> keyboardSession)
{
    if (!isKeyboardPanelEnabled_) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "keyboardPanel is not enabled");
        return;
    }
    if (keyboardSession == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboardSession is nullptr");
        return;
    }
    const auto& panelVec = GetSceneSessionVectorByType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sptr<SceneSession> panelSession;
    for (const auto& session : panelVec) {
        if (session && session->IsSystemKeyboard() == keyboardSession->IsSystemKeyboard()) {
            panelSession = session;
            break;
        }
    }
    /*
     * Only 2 scenarios of panelSession is nullptr:
     * 1: neither keyboard panel session nor system keyboard panel session is created.
     * 2: a panel session has been created already, but it's isSystemKeyboard is not match with the keyboardSesion's
     */
    if (panelSession == nullptr) {
        if (panelVec.size() >= 2) { // 2 is max number of keyboard panel, one input method and one system keyboard
            TLOGE(WmsLogTag::WMS_KEYBOARD, "Error size of keyboardPanel, size: %{public}zu", panelVec.size());
            ReportKeyboardCreateException(keyboardSession);
            return;
        }
        std::string panelName = keyboardSession->IsSystemKeyboard() ? "SCBSystemKeyboardPanel" : "SCBKeyboardPanel";
        panelSession = RequestKeyboardPanelSession(panelName,
            static_cast<uint64_t>(keyboardSession->GetSessionProperty()->GetDisplayId()));
        if (panelSession == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "PanelSession is nullptr");
            return;
        }
    } else {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "keyboardPanel is created, panelId: %{public}d",
            panelSession->GetPersistentId());
    }
    panelSession->SetIsSystemKeyboard(keyboardSession->IsSystemKeyboard());
    keyboardSession->BindKeyboardPanelSession(panelSession);
    panelSession->BindKeyboardSession(keyboardSession);
    TLOGI(WmsLogTag::WMS_KEYBOARD, "success, panelId:%{public}d, keyboardId:%{public}d",
        panelSession->GetPersistentId(), keyboardSession->GetPersistentId());
}

sptr<SceneSession> SceneSessionManager::CreateSceneSession(const SessionInfo& sessionInfo,
    sptr<WindowSessionProperty> property)
{
    sptr<SceneSession::SpecificSessionCallback> specificCb = CreateSpecificSessionCallback();
    sptr<SceneSession> sceneSession = nullptr;
    if (sessionInfo.isSystem_) {
        sceneSession = new SCBSystemSession(sessionInfo, specificCb, GetCurrentUserId());
        TLOGI(WmsLogTag::DEFAULT, "[WMSSCB]Create SCBSystemSession, type: %{public}d", sessionInfo.windowType_);
    } else if (property == nullptr && SessionHelper::IsMainWindow(static_cast<WindowType>(sessionInfo.windowType_))) {
        sceneSession = new MainSession(sessionInfo, specificCb, GetCurrentUserId());
        TLOGI(WmsLogTag::WMS_MAIN, "Create MainSession, id: %{public}d", sceneSession->GetPersistentId());
    } else if (property != nullptr && SessionHelper::IsSubWindow(property->GetWindowType())) {
        sceneSession = new SubSession(sessionInfo, specificCb, GetCurrentUserId());
        TLOGI(WmsLogTag::WMS_SUB, "Create SubSession, type: %{public}d", property->GetWindowType());
    } else if (property != nullptr && property->GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
        sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb = CreateKeyboardSessionCallback();
        sceneSession = new KeyboardSession(sessionInfo, specificCb, keyboardCb, GetCurrentUserId());
        sceneSession->SetIsSystemKeyboard(property->IsSystemKeyboard());
        CreateKeyboardPanelSession(sceneSession);
        HandleKeyboardAvoidChange(sceneSession, sceneSession->GetScreenId(),
            SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Create KeyboardSession, type: %{public}d", property->GetWindowType());
    } else if (property != nullptr && SessionHelper::IsSystemWindow(property->GetWindowType())) {
        sceneSession = new SystemSession(sessionInfo, specificCb, GetCurrentUserId());
        TLOGI(WmsLogTag::WMS_SYSTEM, "Create SystemSession, type: %{public}d", property->GetWindowType());
    } else {
        TLOGE(WmsLogTag::WMS_LIFE, "Invalid window type");
    }
    if (sceneSession != nullptr) {
        sceneSession->SetWindowAnimationDuration(appWindowSceneConfig_.windowAnimation_.duration_);
        sceneSession->SetSessionInfoPersistentId(sceneSession->GetPersistentId());
        sceneSession->isKeyboardPanelEnabled_ = isKeyboardPanelEnabled_;
        sceneSession->RegisterForceSplitListener([this](const std::string& bundleName) {
            return this->GetAppForceLandscapeConfig(bundleName);
        });
        sceneSession->RegisterAppHookWindowInfoFunc([this](const std::string& bundleName) {
            return this->GetAppHookWindowInfo(bundleName);
        });
        sceneSession->SetUpdatePrivateStateAndNotifyFunc([this](int32_t persistentId) {
            this->UpdatePrivateStateAndNotify(persistentId);
        });
        sceneSession->SetNotifyScreenshotAppEventRegisteredFunc([this](int32_t persistentId, bool isRegister) {
            this->UpdateSessionScreenshotAppEventListener(persistentId, isRegister);
        });
        sceneSession->SetNotifyVisibleChangeFunc([this](int32_t persistentId) {
            this->NotifyVisibleChange(persistentId);
        });
        sceneSession->SetIsLastFrameLayoutFinishedFunc([this](bool& isLayoutFinished) {
            return this->IsLastFrameLayoutFinished(isLayoutFinished);
        });
        sceneSession->SetIsAINavigationBarAvoidAreaValidFunc([this](DisplayId displayId,
                const AvoidArea& avoidArea, int32_t sessionBottom) {
            return CheckAvoidAreaForAINavigationBar(isAINavigationBarVisible_[displayId], avoidArea, sessionBottom);
        });
        sceneSession->RegisterGetStatusBarAvoidHeightFunc([this](DisplayId displayId, WSRect& barArea) {
            return this->GetStatusBarAvoidHeight(displayId, barArea);
        });
        sceneSession->RegisterGetStatusBarConstantlyShowFunc([this](DisplayId displayId, bool& isVisible) {
            return this->GetStatusBarConstantlyShow(displayId, isVisible);
        });
        sceneSession->SetHasRequestedVsyncFunc([this](bool& hasRequestedVsync) {
            return this->HasRootSceneRequestedVsync(hasRequestedVsync);
        });
        sceneSession->SetRequestNextVsyncWhenModeChangeFunc(
            [this](const std::shared_ptr<VsyncCallback>& vsyncCallback) {
            return this->RequestVsyncByRootSceneWhenModeChange(vsyncCallback);
        });
        sceneSession->SetGetAllAppUseControlMapFunc([this]() ->
            std::unordered_map<std::string, std::unordered_map<ControlAppType, ControlInfo>>& {
            return allAppUseControlMap_;
        });
        sceneSession->RegisterGetFbPanelWindowIdFunc([this](uint32_t& windowId) {
            return this->GetFbPanelWindowId(windowId);
        });
        sceneSession->SetFindScenePanelRsNodeByZOrderFunc([this](uint64_t screenId, uint32_t targetZOrder) {
            return this->findScenePanelRsNodeByZOrderFunc_(screenId, targetZOrder);
        });
        sceneSession->RegisterGetIsDockAutoHideFunc([this]() {
            return systemConfig_.isDockAutoHide_;
        });
        sceneSession->RegisterForceSplitFullScreenChangeCallback([this](uint32_t uid, bool isFullScreen) {
            this->NotifyIsFullScreenInForceSplitMode(uid, isFullScreen);
        });
        sceneSession->RegisterGetIsRecentStateFunc([this]() {
            return this->scbIsRecent_.load();
        });

        sceneSession->RegisterNotifyOccupiedAreaChangeCallback([this](DisplayId displayId) {
            auto keyboardSession = GetKeyboardSession(displayId, false);
            if (keyboardSession == nullptr) {
                TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboardSession is nullptr, displayId: %{public}" PRIu64, displayId);
                return;
            }
            keyboardSession->ForceProcessKeyboardOccupiedAreaInfo();
        });

        if (SessionHelper::IsMainWindow(sceneSession->GetWindowType())) {
            sceneSession->RegisterForceSplitEnableListener([this](const std::string& bundleName) {
                return this->GetAppForceLandscapeConfigEnable(bundleName);
            });
        }
        DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
        GetAppDragResizeType(sessionInfo.bundleName_, dragResizeType);
        sceneSession->SetAppDragResizeType(dragResizeType);
        sceneSession->SetDragKeyFramePolicy(GetAppKeyFramePolicy(sessionInfo.bundleName_));
        sceneSession->SetSingleHandTransform(singleHandTransform_);
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, displayId: %{public}" PRIu64,
            sceneSession->GetPersistentId(), sceneSession->GetSessionProperty()->GetDisplayId());
        SetBufferAvailable(sceneSession);
    }
    return sceneSession;
}

void SceneSessionManager::SetBufferAvailable(sptr<SceneSession>& sceneSession)
{
    if (!sceneSession) {
        TLOGE(WmsLogTag::WMS_SCB, "scene session is nullptr");
        return;
    }
    const auto type = sceneSession->GetWindowType();
    TLOGI(WmsLogTag::WMS_SCB, "id: %{public}d, type: %{public}d", sceneSession->GetPersistentId(), type);
    if (type == WindowType::WINDOW_TYPE_DESKTOP || type == WindowType::WINDOW_TYPE_KEYGUARD ||
        type == WindowType::WINDOW_TYPE_WALLPAPER) {
        TLOGI(WmsLogTag::WMS_SCB, "enter buffer available callback");
        auto surfaceNode = sceneSession->GetSurfaceNode();
        
        if (!surfaceNode) {
            TLOGE(WmsLogTag::WMS_SCB, "surface node is nullptr");
            return;
        }
        TLOGI(WmsLogTag::WMS_SCB, "config surface node");
        surfaceNode->SetBufferAvailableCallback([where = __func__]() {
            TLOGNE(WmsLogTag::WMS_SCB, "%{public}s BufferAvailableCallback called", where);
            ScreenSessionManagerClient::GetInstance().NotifySwitchUserAnimationFinishByWindow();
        });
    }
}

void SceneSessionManager::GetEffectiveDragResizeType(DragResizeType& dragResizeType)
{
    if (dragResizeType != DragResizeType::RESIZE_TYPE_UNDEFINED) {
        return;
    }
    if (systemConfig_.freeMultiWindowSupport_) {
        if (systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ != DragResizeType::RESIZE_TYPE_UNDEFINED) {
            dragResizeType = systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_;
            return;
        }
        dragResizeType = DragResizeType::RESIZE_WHEN_DRAG_END;
    } else {
        dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    }
}

WMError SceneSessionManager::SetGlobalDragResizeType(DragResizeType dragResizeType)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "dragResizeType: %{public}d", dragResizeType);
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    std::lock_guard<std::mutex> lock(dragResizeTypeMutex_);
    globalDragResizeType_ = dragResizeType;
    taskScheduler_->PostAsyncTask([this] {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession != nullptr && WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
                const std::string& bundleName = sceneSession->GetSessionInfo().bundleName_;
                DragResizeType appDragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
                GetAppDragResizeType(bundleName, appDragResizeType);
                TLOGND(WmsLogTag::WMS_LAYOUT, "SetGlobalDragResizeType persistentId: %{public}d, bundleName: %{public}s, "
                    "dragResizeType: %{public}d", sceneSession->GetPersistentId(), bundleName.c_str(), appDragResizeType);
                sceneSession->SetAppDragResizeType(appDragResizeType);
            }
        }
    }, __func__);
    return WMError::WM_OK;
}

WMError SceneSessionManager::GetGlobalDragResizeType(DragResizeType& dragResizeType)
{
    std::lock_guard<std::mutex> lock(dragResizeTypeMutex_);
    dragResizeType = globalDragResizeType_;
    GetEffectiveDragResizeType(dragResizeType);
    TLOGI(WmsLogTag::WMS_LAYOUT, "dragResizeType: %{public}d", dragResizeType);
    return WMError::WM_OK;
}

WMError SceneSessionManager::SetAppDragResizeType(const std::string& bundleName, DragResizeType dragResizeType)
{
    TLOGD(WmsLogTag::WMS_LAYOUT, "dragResizeType: %{public}d, bundleName: %{public}s",
        dragResizeType, bundleName.c_str());
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    return SetAppDragResizeTypeInner(bundleName, dragResizeType);
}

WMError SceneSessionManager::SetAppDragResizeTypeInner(const std::string& bundleName, DragResizeType dragResizeType)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "dragResizeType: %{public}d, bundleName: %{public}s",
        dragResizeType, bundleName.c_str());
    if (bundleName.empty()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "bundleName empty");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> dragResizeTypeLock(dragResizeTypeMutex_);
    appDragResizeTypeMap_[bundleName] = dragResizeType;
    GetAppDragResizeTypeInner(bundleName, dragResizeType);
    taskScheduler_->PostAsyncTask([this, bundleName, dragResizeType] {
        auto allMatchSession = GetSceneSessionByBundleName(bundleName);
        for (const auto& sceneSession : allMatchSession) {
            if (sceneSession != nullptr) {
                TLOGNI(WmsLogTag::WMS_LAYOUT, "SetAppDragResizeType persistentId: %{public}d, bundleName: %{public}s, "
                    "dragResizeType: %{public}d", sceneSession->GetPersistentId(), bundleName.c_str(), dragResizeType);
                sceneSession->SetAppDragResizeType(dragResizeType);
            }
        }
    }, __func__);
    return WMError::WM_OK;
}

WMError SceneSessionManager::GetAppDragResizeType(const std::string& bundleName, DragResizeType& dragResizeType)
{
    std::lock_guard<std::mutex> lock(dragResizeTypeMutex_);
    return GetAppDragResizeTypeInner(bundleName, dragResizeType);
}

WMError SceneSessionManager::GetAppDragResizeTypeInner(const std::string& bundleName, DragResizeType& dragResizeType)
{
    if (bundleName.empty()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "bundleName empty");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (globalDragResizeType_ != DragResizeType::RESIZE_TYPE_UNDEFINED) {
        TLOGI(WmsLogTag::WMS_LAYOUT, "use global value");
        dragResizeType = globalDragResizeType_;
        return WMError::WM_OK;
    }
    dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    if (auto iter = appDragResizeTypeMap_.find(bundleName); iter != appDragResizeTypeMap_.end()) {
        dragResizeType = iter->second;
    }
    GetEffectiveDragResizeType(dragResizeType);
    TLOGI(WmsLogTag::WMS_LAYOUT, "dragResizeType: %{public}d, bundleName: %{public}s",
        dragResizeType, bundleName.c_str());
    return WMError::WM_OK;
}

WMError SceneSessionManager::SetAppKeyFramePolicy(const std::string& bundleName,
    const KeyFramePolicy& keyFramePolicy)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "keyFramePolicy enabled: %{public}d, bundleName: %{public}s",
        keyFramePolicy.enabled(), bundleName.c_str());
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (bundleName.empty()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "bundleName empty");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    {
        std::lock_guard<std::mutex> lock(keyFrameMutex_);
        appKeyFramePolicyMap_[bundleName] = keyFramePolicy;
    }
    taskScheduler_->PostAsyncTask([this, bundleName, keyFramePolicy, where = __func__] {
        auto allMatchSession = GetSceneSessionByBundleName(bundleName);
        for (const auto& sceneSession : allMatchSession) {
            if (sceneSession != nullptr && systemConfig_.IsPcWindow() &&
                WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
                TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s: pc main window id: %{public}d, bundleName: %{public}s",
                    where, sceneSession->GetPersistentId(), bundleName.c_str());
                sceneSession->SetDragKeyFramePolicy(keyFramePolicy);
            }
        }
    }, __func__);
    return WMError::WM_OK;
}

KeyFramePolicy SceneSessionManager::GetAppKeyFramePolicy(const std::string& bundleName)
{
    {
        std::lock_guard<std::mutex> lock(keyFrameMutex_);
        if (auto iter = appKeyFramePolicyMap_.find(bundleName); iter != appKeyFramePolicyMap_.end()) {
            return iter->second;
        }
    }
    return KeyFramePolicy();
}

sptr<SceneSession> SceneSessionManager::GetSceneSessionBySessionInfo(const SessionInfo& sessionInfo)
{
    if (sessionInfo.isPersistentRecover_) {
        TLOGI(WmsLogTag::WMS_LIFE, "session id: %{public}d is persistent recover.", sessionInfo.persistentId_);
        return nullptr;
    }

    if (sessionInfo.persistentId_ != INVALID_SESSION_ID) {
        if (auto session = GetSceneSession(sessionInfo.persistentId_)) {
            TLOGI(WmsLogTag::WMS_LIFE, "get exist session persistentId: %{public}d", sessionInfo.persistentId_);
            return session;
        }
    }

    if (WindowHelper::IsMainWindow(static_cast<WindowType>(sessionInfo.windowType_)) &&
        (sessionInfo.isAncoApplication_ || CheckCollaboratorType(sessionInfo.collaboratorType_))) {
        TLOGI(WmsLogTag::WMS_LIFE, "ancoApplication not reuse session by sceneInfo defaultly.");
        return nullptr;
    }

    if (WindowHelper::IsMainWindow(static_cast<WindowType>(sessionInfo.windowType_))) {
        TLOGI(WmsLogTag::WMS_LIFE, "mainWindow bundleName: %{public}s, moduleName: %{public}s, "
            "abilityName: %{public}s, appIndex: %{public}d, appInstanceKey:%{public}s, isAtomicService:%{public}d",
            sessionInfo.bundleName_.c_str(), sessionInfo.moduleName_.c_str(), sessionInfo.abilityName_.c_str(),
            sessionInfo.appIndex_, sessionInfo.appInstanceKey_.c_str(), sessionInfo.isAtomicService_);
        SessionIdentityInfo identityInfo = { sessionInfo.bundleName_, sessionInfo.moduleName_,
            sessionInfo.abilityName_, sessionInfo.appIndex_, sessionInfo.appInstanceKey_, sessionInfo.windowType_,
            sessionInfo.isAtomicService_ };
        auto sceneSession = GetSceneSessionByIdentityInfo(identityInfo);
        bool isSingleStart = sceneSession && sceneSession->GetAbilityInfo() &&
            sceneSession->GetAbilityInfo()->launchMode == AppExecFwk::LaunchMode::SINGLETON;
        if (isSingleStart) {
            TLOGI(WmsLogTag::WMS_LIFE, "get exist singleton session persistentId: %{public}d",
                sessionInfo.persistentId_);
            return sceneSession;
        }
    }
    return nullptr;
}

sptr<SceneSession> SceneSessionManager::GetHookedSessionByModuleName(const SessionInfo& sessionInfo)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    sptr<SceneSession> session = nullptr;
    for (const auto &[_, sceneSession] : sceneSessionMap_) {
        if (!sceneSession || !SessionHelper::IsMainWindow(sceneSession->GetWindowType())) {
            continue;
        }
        if (sceneSession->GetSessionInfo().bundleName_ != sessionInfo.bundleName_ ||
            sceneSession->GetSessionInfo().moduleName_ != sessionInfo.moduleName_ ||
            sceneSession->GetSessionInfo().appIndex_ != sessionInfo.appIndex_ ||
            sceneSession->GetSessionInfo().appInstanceKey_ != sessionInfo.appInstanceKey_) {
            continue;
        }
        if (sceneSession->GetSessionInfo().disableDelegator) {
            return nullptr;
        } else {
            session = sceneSession;
        }
    }
    return session;
}

void SceneSessionManager::UpdateAbilityHookState(sptr<SceneSession>& sceneSession, bool isAbilityHook)
{
    if (!sceneSession || !isAbilityHook)  {
        TLOGE(WmsLogTag::WMS_LIFE, "sceneSession is null or isAbilityHook is false");
        return;
    }
    sceneSession->EditSessionInfo().isAbilityHook_ = isAbilityHook;
    RegisterHookSceneSessionActivationFunc(sceneSession);
}

sptr<SceneSession> SceneSessionManager::RequestSceneSession(const SessionInfo& sessionInfo,
    sptr<WindowSessionProperty> property)
{
    auto task = [this, sessionInfo, property, where = __func__] {
        if (auto session = GetSceneSessionBySessionInfo(sessionInfo)) {
            UpdateSessionDisplayIdBySessionInfo(session, sessionInfo);
            NotifySessionUpdate(sessionInfo, ActionType::SINGLE_START);
            UpdateAbilityHookState(session, sessionInfo.isAbilityHook_);
            return session;
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s: appName: [%{public}s %{public}s %{public}s] "
            "appIndex %{public}d, type %{public}u system %{public}u, isPersistentRecover %{public}u",
            where, sessionInfo.bundleName_.c_str(), sessionInfo.moduleName_.c_str(),
            sessionInfo.abilityName_.c_str(), sessionInfo.appIndex_, sessionInfo.windowType_,
            static_cast<uint32_t>(sessionInfo.isSystem_), static_cast<uint32_t>(sessionInfo.isPersistentRecover_));
        sptr<SceneSession> sceneSession = CreateSceneSession(sessionInfo, property);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "sceneSession is nullptr!");
            return sceneSession;
        }
        sceneSession->RecoverSnapshotPersistence(sessionInfo);
        if (sessionInfo.isAbilityHook_) {
            auto session = GetHookedSessionByModuleName(sessionInfo);
            if (session) {
                TLOGNW(WmsLogTag::WMS_LIFE, "session disableDelegator %{public}d is still hook, return hook session",
                    session->GetSessionInfo().disableDelegator);
                return session;
            }
            RegisterHookSceneSessionActivationFunc(sceneSession);
        }
        if (MultiInstanceManager::IsSupportMultiInstance(systemConfig_) &&
            WindowHelper::IsMainWindow(sceneSession->GetWindowType()) &&
            MultiInstanceManager::GetInstance().IsMultiInstance(sceneSession->GetSessionInfo().bundleName_)) {
            MultiInstanceManager::GetInstance().FillInstanceKeyIfNeed(sceneSession);
        }
        LOCK_GUARD_EXPR(SCENE_GUARD, InitSceneSession(sceneSession, sessionInfo, property));
        if (CheckCollaboratorType(sceneSession->GetCollaboratorType())) {
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s: ancoSceneState: %{public}d",
                where, sceneSession->GetSessionInfo().ancoSceneState);
            bool isPreHandleSuccess = PreHandleCollaboratorStartAbility(sceneSession);
            const auto& sessionAffinity = sceneSession->GetSessionInfo().sessionAffinity;
            if (auto reusedSceneSession = SceneSessionManager::GetInstance().FindSessionByAffinity(sessionAffinity)) {
                TLOGNI(WmsLogTag::WMS_LIFE,
                    "%{public}s: session reuse id:%{public}d type:%{public}d affinity:%{public}s",
                    where, reusedSceneSession->GetPersistentId(),
                    reusedSceneSession->GetWindowType(), sessionAffinity.c_str());
                NotifySessionUpdate(reusedSceneSession->GetSessionInfo(), ActionType::SINGLE_START);
                return reusedSceneSession;
            }
            if (isPreHandleSuccess) {
                NotifySessionCreate(sceneSession, sceneSession->GetSessionInfo());
                sceneSession->SetSessionInfoAncoSceneState(AncoSceneState::NOTIFY_CREATE);
            } else {
                sceneSession->SetSessionInfoAncoSceneState(AncoSceneState::NOTIFY_START_FAILED);
            }
        }
        {
            std::unique_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
            sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
            if (MultiInstanceManager::IsSupportMultiInstance(systemConfig_) &&
                MultiInstanceManager::GetInstance().IsMultiInstance(sceneSession->GetSessionInfo().bundleName_)) {
                MultiInstanceManager::GetInstance().IncreaseInstanceKeyRefCount(sceneSession);
            }
        }
        PerformRegisterInRequestSceneSession(sceneSession);
        NotifySessionUpdate(sessionInfo, ActionType::SINGLE_START);
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s: id: %{public}d, type: %{public}d, instanceKey: %{public}s, "
               "displayId: %{public}" PRIu64, where, sceneSession->GetPersistentId(), sceneSession->GetWindowType(),
               sceneSession->GetAppInstanceKey().c_str(), sceneSession->GetSessionProperty()->GetDisplayId());
        return sceneSession;
    };
    return taskScheduler_->PostSyncTask(task, "RequestSceneSession:PID" + std::to_string(sessionInfo.persistentId_));
}

void SceneSessionManager::InitSceneSession(sptr<SceneSession>& sceneSession, const SessionInfo& sessionInfo,
    const sptr<WindowSessionProperty>& property)
{
    auto callerSession = GetSceneSession(sessionInfo.callerPersistentId_);
    DisplayId currDisplayId = DISPLAY_ID_INVALID;
    if (sessionInfo.screenId_ != SCREEN_ID_INVALID) {
        currDisplayId = sessionInfo.screenId_;
    } else if (callerSession) {
        currDisplayId = callerSession->GetSessionProperty()->GetDisplayId();
    }
    sceneSession->GetSessionProperty()->SetDisplayId(currDisplayId);
    sceneSession->SetScreenId(currDisplayId);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "synchronous screenId with displayId %{public}" PRIu64, currDisplayId);

    sceneSession->SetEventHandler(taskScheduler_->GetEventHandler(), eventHandler_);
    sceneSession->RegisterIsScreenLockedCallback([this] { return IsScreenLocked(); });
    if (sessionInfo.isSystem_) {
        sceneSession->SetCallingPid(IPCSkeleton::GetCallingRealPid());
        sceneSession->SetCallingUid(IPCSkeleton::GetCallingUid());
        auto rootContext = rootSceneContextWeak_.lock();
        sceneSession->SetAbilityToken(rootContext != nullptr ? rootContext->GetToken() : nullptr);
    } else {
        TLOGD(WmsLogTag::WMS_LIFE, "id: %{public}d, bundleName: %{public}s, "
            "moduleName: %{public}s, abilityName: %{public}s want: %{public}s", sceneSession->GetPersistentId(),
            sessionInfo.bundleName_.c_str(), sessionInfo.moduleName_.c_str(), sessionInfo.abilityName_.c_str(),
            sessionInfo.want == nullptr ? "nullptr" : sessionInfo.want->ToString().c_str());
    }
    RegisterSessionExceptionFunc(sceneSession);
    RegisterVisibilityChangedDetectFunc(sceneSession);
    RegisterSaveSnapshotFunc(sceneSession);
    RegisterIsAppBoundSystemTrayFunc(sceneSession);
    if (systemConfig_.IsPcOrPcMode()) {
        RegisterGetStartWindowConfigCallback(sceneSession);
    }
    // Skip FillSessionInfo when atomicService free-install start.
    if (!IsAtomicServiceFreeInstall(sessionInfo)) {
        FillSessionInfo(sceneSession);
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:RequestSceneSession(%d )", sceneSession->GetPersistentId());
    if (WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
        WindowInfoReporter::GetInstance().InsertCreateReportInfo(sessionInfo.bundleName_);
    }
    if (property != nullptr && WindowHelper::IsPipWindow(property->GetWindowType())) {
        sceneSession->SetPiPTemplateInfo(property->GetPiPTemplateInfo());
    }
    InitFbWindow(sceneSession, property);
    auto systemConfig = systemConfig_;
    SetWindowStatusDeduplicationBySystemConfig(sessionInfo, systemConfig);
    sceneSession->SetSystemConfig(systemConfig);
    sceneSession->InitSnapshotCapacity();
    sceneSession->SetSnapshotScale(snapshotScale_);
    UpdateParentSessionForDialog(sceneSession, property);
    std::string key = sessionInfo.bundleName_ + "_" + sessionInfo.moduleName_ + "_" + sessionInfo.abilityName_ + "_" +
        std::to_string(sessionInfo.appIndex_);
    if (sessionLockedStateCacheSet_.find(key) != sessionLockedStateCacheSet_.end()) {
        sceneSession->NotifySessionLockStateChange(true);
    }
}

void SceneSessionManager::InitFbWindow(const sptr<SceneSession>& sceneSession,
    const sptr<WindowSessionProperty>& property)
{
    if (property != nullptr && WindowHelper::IsFbWindow(property->GetWindowType())) {
        sceneSession->SetFbTemplateInfo(property->GetFbTemplateInfo());
    }
}

void SceneSessionManager::NotifySessionUpdate(const SessionInfo& sessionInfo, ActionType action, ScreenId fromScreenId)
{
    taskScheduler_->PostAsyncTask([abilityName = sessionInfo.abilityName_,
        bundleName = sessionInfo.bundleName_, toScreenId = sessionInfo.screenId_, action, fromScreenId] {
        sptr<DisplayChangeInfo> info = sptr<DisplayChangeInfo>::MakeSptr();
        info->action_ = action;
        info->abilityName_ = std::move(abilityName);
        info->bundleName_ = std::move(bundleName);
        info->toScreenId_ = toScreenId;
        info->fromScreenId_ = fromScreenId;
        ScreenSessionManagerClient::GetInstance().NotifyDisplayChangeInfoChanged(info);
        TLOGNI(WmsLogTag::DMS, "Notify ability %{public}s bundle %{public}s update toScreen id: %{public}" PRIu64,
            info->abilityName_.c_str(), info->bundleName_.c_str(), info->toScreenId_);
    }, __func__);
}

void SceneSessionManager::PerformRegisterInRequestSceneSession(sptr<SceneSession>& sceneSession)
{
    RegisterSessionSnapshotFunc(sceneSession);
    RegisterSessionStateChangeNotifyManagerFunc(sceneSession);
    RegisterSessionInfoChangeNotifyManagerFunc(sceneSession);
    RegisterDisplayIdChangedNotifyManagerFunc(sceneSession);
    RegisterRequestFocusStatusNotifyManagerFunc(sceneSession);
    RegisterGetStateFromManagerFunc(sceneSession);
    RegisterSessionChangeByActionNotifyManagerFunc(sceneSession);
    RegisterAcquireRotateAnimationConfigFunc(sceneSession);
    RegisterSceneSessionDestructNotifyManagerFunc(sceneSession);
    RegisterSessionPropertyChangeNotifyManagerFunc(sceneSession);
    RegisterClientDisplayIdChangeNotifyManagerFunc(sceneSession);
    RegisterGetRsCmdBlockingCountFunc(sceneSession);
    BindVsyncStation(sceneSession);
}

void SceneSessionManager::UpdateSceneSessionWant(const SessionInfo& sessionInfo)
{
    taskScheduler_->PostSyncTask([this, &sessionInfo] {
        DoUpdateSceneSessionWant(sessionInfo);
        return WMError::WM_OK;
    }, __func__);
}

void SceneSessionManager::DoUpdateSceneSessionWant(const SessionInfo& sessionInfo)
{
    if (sessionInfo.persistentId_ != 0) {
        auto session = GetSceneSession(sessionInfo.persistentId_);
        if (session != nullptr && sessionInfo.want != nullptr) {
            TLOGI(WmsLogTag::WMS_MAIN, "Get session id:%{public}d", sessionInfo.persistentId_);
            if (!CheckCollaboratorType(session->GetCollaboratorType())) {
                session->SetSessionInfoWant(sessionInfo.want);
                TLOGI(WmsLogTag::WMS_MAIN, "Want updated, id:%{public}d", sessionInfo.persistentId_);
            } else {
                UpdateCollaboratorSessionWant(session, sessionInfo.persistentId_);
            }
            AddRequestTaskInfo(session, sessionInfo.requestId);
        } else {
            TLOGI(WmsLogTag::WMS_MAIN, "Get session fail(%{public}d), id:%{public}d",
                session == nullptr, sessionInfo.persistentId_);
        }
    } else {
        TLOGI(WmsLogTag::WMS_MAIN, "sessionInfo.Id == 0");
    }
}

void SceneSessionManager::UpdateCollaboratorSessionWant(sptr<SceneSession>& session, int32_t persistentId)
{
    if (session != nullptr && session->GetSessionInfo().ancoSceneState < AncoSceneState::NOTIFY_CREATE) {
        FillSessionInfo(session);
        if (CheckCollaboratorType(session->GetCollaboratorType())) {
            PreHandleCollaborator(session, persistentId);
        }
    }
}

sptr<AAFwk::SessionInfo> SceneSessionManager::SetAbilitySessionInfo(const sptr<SceneSession>& sceneSession,
    int32_t requestId, bool useRequestTaskInfo)
{
    const auto& sessionInfo = sceneSession->GetSessionInfo();
    auto abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilitySessionInfo->sessionToken = sptr<ISession>(sceneSession)->AsObject();
    abilitySessionInfo->identityToken = std::to_string(std::chrono::time_point_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now()).time_since_epoch().count());
    abilitySessionInfo->callerToken = sessionInfo.callerToken_;
    abilitySessionInfo->sessionName = SessionUtils::ConvertSessionName(sessionInfo.bundleName_,
        sessionInfo.abilityName_, sessionInfo.moduleName_, sessionInfo.appIndex_);
    abilitySessionInfo->persistentId = sceneSession->GetPersistentId();
    abilitySessionInfo->requestCode = sessionInfo.requestCode;
    abilitySessionInfo->resultCode = sessionInfo.resultCode;
    abilitySessionInfo->uiAbilityId = sessionInfo.uiAbilityId_;
    abilitySessionInfo->startSetting = sessionInfo.startSetting;
    abilitySessionInfo->callingTokenId = sessionInfo.callingTokenId_;
    abilitySessionInfo->userId = currentUserId_;
    abilitySessionInfo->isClearSession = sessionInfo.isClearSession;
    abilitySessionInfo->processOptions = sessionInfo.processOptions;
    abilitySessionInfo->requestId = sessionInfo.requestId;
    abilitySessionInfo->reuseDelegatorWindow = sessionInfo.reuseDelegatorWindow;
    abilitySessionInfo->specifiedFlag = sessionInfo.specifiedFlag_;
    std::shared_ptr<AAFwk::Want> requestWant = nullptr;
    if (useRequestTaskInfo &&
        (requestWant = GetRequestWantFromTaskInfoMap(sceneSession->GetPersistentId(), requestId)) != nullptr) {
        abilitySessionInfo->want = *requestWant;
    } else if (sessionInfo.want != nullptr) {
        abilitySessionInfo->want = sessionInfo.GetWantSafely();
    } else {
        abilitySessionInfo->want.SetElementName("", sessionInfo.bundleName_, sessionInfo.abilityName_,
            sessionInfo.moduleName_);
    }
    int appIndex = abilitySessionInfo->want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
    bool hasAppIndex = abilitySessionInfo->want.HasParameter(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY);
    if (hasAppIndex && sessionInfo.appIndex_ != appIndex) {
        TLOGW(WmsLogTag::WMS_LIFE, "appIndex not match want.appIndex:%{public}d, sessionInfo.appIndex_:%{public}d",
            appIndex, sessionInfo.appIndex_);
    }
    if (!hasAppIndex && sessionInfo.appIndex_ > 0) {
        TLOGI(WmsLogTag::WMS_LIFE, "want.appIndex is null, set want.appIndex:%{public}d", sessionInfo.appIndex_);
        abilitySessionInfo->want.SetParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, sessionInfo.appIndex_);
    }
    abilitySessionInfo->want.SetParam(AAFwk::Want::PARAM_RESV_DISPLAY_ID,
        static_cast<int>(sceneSession->GetSessionProperty()->GetDisplayId()));
    abilitySessionInfo->instanceKey = sessionInfo.appInstanceKey_;
    if (sessionInfo.callState_ >= static_cast<uint32_t>(AAFwk::CallToState::UNKNOW) &&
        sessionInfo.callState_ <= static_cast<uint32_t>(AAFwk::CallToState::BACKGROUND)) {
        abilitySessionInfo->state = static_cast<AAFwk::CallToState>(sessionInfo.callState_);
    } else {
        TLOGW(WmsLogTag::WMS_LIFE, "Invalid callState:%{public}d", sessionInfo.callState_);
    }
    abilitySessionInfo->scenarios = sessionInfo.scenarios;
    if (sessionInfo.isRestartApp_) {
        auto callerSession = GetSceneSession(sessionInfo.restartCallerPersistentId_);
        if (!callerSession) {
            callerSession = sceneSession;
        }
        TLOGI(WmsLogTag::WMS_LIFE, "restart app caller session id: %{public}d",
            callerSession->GetSessionInfo().persistentId_);
        abilitySessionInfo->callerSession = sptr<ISession>(callerSession)->AsObject();
    }
    abilitySessionInfo->isPrelaunch = sessionInfo.isPrelaunch_;
    abilitySessionInfo->frameNum = sessionInfo.frameNum_;
    TLOGI(WmsLogTag::WMS_LIFE, "Is SCB Call, set flag:%{public}d, persistentId:%{public}d, requestId:%{public}d",
        requestId == DEFAULT_REQUEST_FROM_SCB_ID, abilitySessionInfo->persistentId, requestId);
    return abilitySessionInfo;
}

WSError SceneSessionManager::PrepareTerminate(int32_t persistentId, bool& isPrepareTerminate)
{
    if (!isPrepareTerminateEnable_) { // not support prepareTerminate
        isPrepareTerminate = false;
        TLOGE(WmsLogTag::WMS_MAIN, "not support prepareTerminate, Id:%{public}d", persistentId);
        if (IsPrepareTerminateEnabled()) {
            TLOGE(WmsLogTag::WMS_MAIN, "prepareTerminateConfig is enable.");
        }
        return WSError::WS_OK;
    }
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "sceneSession is null, Id:%{public}d", persistentId);
        isPrepareTerminate = false;
        return WSError::WS_ERROR_NULLPTR;
    }
    auto sceneSessionInfo = SetAbilitySessionInfo(sceneSession);
    auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->
        PrepareTerminateAbilityBySCB(sceneSessionInfo, isPrepareTerminate);
    TLOGI(WmsLogTag::WMS_MAIN, "Id:%{public}d isPrepareTerminate:%{public}d "
        "errorCode:%{public}d", persistentId, isPrepareTerminate, errorCode);
    return WSError::WS_OK;
}

WSError SceneSessionManager::RequestSceneSessionActivation(const sptr<SceneSession>& sceneSession, bool isNewActive,
    int32_t requestId)
{
    auto task = [this, weakSceneSession = wptr(sceneSession), isNewActive,
        requestId]() THREAD_SAFETY_GUARD(SCENE_GUARD) {
        OHOS::HiviewDFX::HiTraceId hiTraceId = OHOS::HiviewDFX::HiTraceChain::GetId();
        bool isValid = hiTraceId.IsValid();
        if (!isValid) {
            hiTraceId = OHOS::HiviewDFX::HiTraceChain::Begin("WindowCreateOnCall",
                HiTraceFlag::HITRACE_FLAG_INCLUDE_ASYNC | HiTraceFlag::HITRACE_FLAG_NO_BE_INFO |
                HiTraceFlag::HITRACE_FLAG_DONOT_CREATE_SPAN);
        }
        sptr<SceneSession> sceneSession = weakSceneSession.promote();
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "Request active session is nullptr");
            if (!isValid) {
                OHOS::HiviewDFX::HiTraceChain::End(hiTraceId);
            }
            return WSError::WS_ERROR_NULLPTR;
        }
        if (!Session::IsScbCoreEnabled()) {
            sceneSession->SetForegroundInteractiveStatus(true);
        }
        auto persistentId = sceneSession->GetPersistentId();
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:RequestSceneSessionActivation(%d )", persistentId);
        TLOGNI(WmsLogTag::WMS_MAIN,
            "Request active id:%{public}d, system:%{public}d, isNewActive:%{public}d, requestId:%{public}d",
            persistentId, sceneSession->GetSessionInfo().isSystem_,
            isNewActive, sceneSession->GetSessionInfo().requestId);
        if (!GetSceneSession(persistentId)) {
            TLOGNE(WmsLogTag::WMS_MAIN, "Request active session invalid by %{public}d", persistentId);
            if (!isValid) {
                OHOS::HiviewDFX::HiTraceChain::End(hiTraceId);
            }
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        auto ret = RequestSceneSessionActivationInner(sceneSession, isNewActive, requestId);
        if (ret == WSError::WS_OK) {
            sceneSession->SetExitSplitOnBackground(false);
        }
        abilityInfoMap_.clear(); // clear cache after terminate
        RemoveRequestTaskInfo(sceneSession->GetPersistentId(), requestId);
        if (!isValid) {
            OHOS::HiviewDFX::HiTraceChain::End(hiTraceId);
        }
        return ret;
    };
    std::string taskName = "RequestSceneSessionActivation:PID:" +
        (sceneSession != nullptr ? std::to_string(sceneSession->GetPersistentId()) : "nullptr");
    taskScheduler_->PostAsyncTask(task, taskName);
    return WSError::WS_OK;
}

void SceneSessionManager::NotifyAmsPendingSessionWhenFail(uint32_t resultCode, std::string resultMessage,
        int32_t requestId, int32_t persistentId)
{
    if (requestId <= 0) {
        return;
    }
    TLOGE(WmsLogTag::WMS_LIFE, "failed, requestId:%{public}d", requestId);
    RemoveRequestTaskInfo(persistentId, requestId);
    ffrtQueueHelper_->SubmitTask([requestId]{
         AAFwk::AbilityManagerClient::GetInstance()->NotifyStartupExceptionBySCB(requestId);
    });
}

bool SceneSessionManager::IsKeyboardForeground()
{
    bool isKeyboardForeground = false;
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession != nullptr && sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
            isKeyboardForeground = sceneSession->IsSessionForeground();
            break;
        }
    }
    return isKeyboardForeground;
}

void SceneSessionManager::RequestInputMethodCloseKeyboard(const int32_t persistentId)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is nullptr");
        return;
    }
    // Hide keyboard when app is cold started, if keyboard is showing and screen is unlocked.
    if (!sceneSession->IsSessionValid() && IsKeyboardForeground() &&
        !sceneSession->GetStateFromManager(ManagerState::MANAGER_STATE_SCREEN_LOCKED)) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "Session is invalid, id: %{public}d state: %{public}u",
            persistentId, sceneSession->GetSessionState());
        sceneSession->RequestHideKeyboard(true);
    }
}

void SceneSessionManager::ResetSceneMissionInfo(const sptr<AAFwk::SessionInfo>& abilitySessionInfo)
{
    abilitySessionInfo->scenarios = 0; // 0 after app started, scenarios reset to initial value
}

int32_t SceneSessionManager::StartUIAbilityBySCBTimeoutCheck(const sptr<SceneSession>& sceneSession,
    const sptr<AAFwk::SessionInfo>& abilitySessionInfo, const uint32_t& windowStateChangeReason, bool& isColdStart)
{
    std::shared_ptr<int32_t> retCode = std::make_shared<int32_t>(0);
    std::shared_ptr<bool> coldStartFlag = std::make_shared<bool>(false);
    bool isTimeout = ffrtQueueHelper_->SubmitTaskAndWait([this, sceneSession, abilitySessionInfo,
        coldStartFlag, retCode, windowStateChangeReason] {
        int timerId = HiviewDFX::XCollie::GetInstance().SetTimer("WMS:SSM:StartUIAbilityBySCB",
            START_UI_ABILITY_TIMEOUT/1000, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
        OHOS::AbilityRuntime::StartParamsBySCB startParams;
        startParams.sceneFlag = windowStateChangeReason;
        startParams.isRestart = sceneSession->GetSessionInfo().isRestartApp_;
        startParams.pageConfig = sceneSession->GetSessionInfo().pageConfig;
        auto result = AAFwk::AbilityManagerClient::GetInstance()->StartUIAbilityBySCB(abilitySessionInfo, startParams,
            *coldStartFlag);
        CloseAllFd(sceneSession->GetSessionInfo().want);
        HiviewDFX::XCollie::GetInstance().CancelTimer(timerId);
        *retCode = static_cast<int32_t>(result);
        TLOGNI(WmsLogTag::WMS_LIFE, "start ui ability sceneFlag:%{public}d isRestart:%{public}d pageConfig:%{public}zu retCode: %{public}d",
            startParams.sceneFlag, startParams.isRestart, startParams.pageConfig.size(), *retCode);
    }, START_UI_ABILITY_TIMEOUT);

    if (isTimeout) {
        TLOGE(WmsLogTag::WMS_LIFE, "start ui ability timeout, currentUserId: %{public}d", currentUserId_.load());
        return static_cast<int32_t>(WSError::WS_ERROR_START_UI_ABILITY_TIMEOUT);
    }
    ResetSceneMissionInfo(abilitySessionInfo);
    isColdStart = *coldStartFlag;
    return *retCode;
}

void SceneSessionManager::CloseAllFd(std::shared_ptr<AAFwk::Want>& want)
{
    if (want) {
        want->CloseAllFd();
    }
}

int32_t SceneSessionManager::StartUIAbilityBySCB(sptr<SceneSession>& sceneSession)
{
    auto abilitySessionInfo = SetAbilitySessionInfo(sceneSession);
    return StartUIAbilityBySCB(abilitySessionInfo, sceneSession);
}

int32_t SceneSessionManager::StartUIAbilityBySCB(sptr<AAFwk::SessionInfo>& abilitySessionInfo,
    sptr<SceneSession>& sceneSession)
{
    bool isColdStart = false;
    return StartUIAbilityBySCBTimeoutCheck(sceneSession, abilitySessionInfo,
        static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL), isColdStart);
}

int32_t SceneSessionManager::ChangeUIAbilityVisibilityBySCB(const sptr<SceneSession>& sceneSession,
    bool visibility, bool isNewWant, bool isFromClient)
{
    auto abilitySessionInfo = SetAbilitySessionInfo(sceneSession);
    if (!isFromClient) {
        sceneSession->RemoveLifeCycleTask(LifeCycleTaskType::START);
    }
    if(visibility) {
        abilitySessionInfo->isNewWant = isNewWant;
        TLOGI(WmsLogTag::WMS_MAIN, "ChangUIAbilityVisibility isNewActive:%{public}d, isVisibility: %{public}d",
            abilitySessionInfo->isNewWant, visibility);
    }
    return AAFwk::AbilityManagerClient::GetInstance()->ChangeUIAbilityVisibilityBySCB(abilitySessionInfo, visibility);
}

WSError SceneSessionManager::RequestSceneSessionActivationInner(
    sptr<SceneSession>& sceneSession, bool isNewActive, int32_t requestId)
{
    auto persistentId = sceneSession->GetPersistentId();
    RequestInputMethodCloseKeyboard(persistentId);
    if (WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
        sceneSession->SetIsStarting(true);
    }
    if (WindowHelper::IsMainWindow(sceneSession->GetWindowType()) && sceneSession->IsFocusedOnShow()) {
        if (Session::IsScbCoreEnabled()) {
            if (sceneSession->IsVisibleForeground() && !sceneSession->GetSessionInfo().isRestartApp_) {
                RequestSessionFocusImmediately(persistentId);
            } else {
                PostProcessFocusState state = { true, true, true, FocusChangeReason::SCB_START_APP };
                sceneSession->SetPostProcessFocusState(state);
            }
        } else {
            RequestSessionFocusImmediately(persistentId);
        }
    }
    if (sceneSession->GetSessionInfo().ancoSceneState == AncoSceneState::NOTIFY_START_FAILED) {
        TLOGE(WmsLogTag::WMS_LIFE, "[id: %{public}d] preHandle collaborator failed when requestSession.", persistentId);
        sceneSession->NotifySessionExceptionWithOptions(SetAbilitySessionInfo(sceneSession), "preHandleAncoFailed", true);
        NotifyAmsPendingSessionWhenFail(static_cast<uint32_t>(RequestResultCode::FAIL),
            "", sceneSession->GetSessionInfo().requestId, sceneSession->GetPersistentId());
        return WSError::WS_ERROR_PRE_HANDLE_COLLABORATOR_FAILED;
    }
    if (sceneSession->GetSessionInfo().ancoSceneState < AncoSceneState::NOTIFY_CREATE) {
        FillSessionInfo(sceneSession);
        if (CheckCollaboratorType(sceneSession->GetCollaboratorType()) &&
            !PreHandleCollaborator(sceneSession, persistentId)) {
            TLOGE(WmsLogTag::WMS_LIFE, "[id: %{public}d] preHandle collaborator failed, ancoSceneState: %{public}d",
                persistentId, sceneSession->GetSessionInfo().ancoSceneState);
            sceneSession->NotifySessionExceptionWithOptions(SetAbilitySessionInfo(sceneSession), "preHandleAncoFailed", true);
            NotifyAmsPendingSessionWhenFail(static_cast<uint32_t>(RequestResultCode::FAIL),
                "", sceneSession->GetSessionInfo().requestId, sceneSession->GetPersistentId());
            return WSError::WS_ERROR_PRE_HANDLE_COLLABORATOR_FAILED;
        }
    }
    sceneSession->NotifyActivation();
    auto sceneSessionInfo = SetAbilitySessionInfo(sceneSession, requestId, true);
    sceneSessionInfo->isNewWant = isNewActive;
    sceneSessionInfo->targetGrantBundleName =
        sceneSession->GetSessionInfo().GetWantSafely().GetStringParam("targetGrantBundleName");
    if (CheckCollaboratorType(sceneSession->GetCollaboratorType())) {
        sceneSessionInfo->want.SetParam(AncoConsts::ANCO_MISSION_ID, sceneSessionInfo->persistentId);
        sceneSessionInfo->collaboratorType = sceneSession->GetCollaboratorType();
    }
    TLOGI(WmsLogTag::WMS_LIFE, "[id: %{public}d] want-ability: %{public}s, bundle: %{public}s, "
        "module: %{public}s, uri: %{public}s, appIndex: %{public}d, requestId:%{public}d", persistentId,
        sceneSessionInfo->want.GetElement().GetAbilityName().c_str(),
        sceneSessionInfo->want.GetElement().GetBundleName().c_str(),
        sceneSessionInfo->want.GetElement().GetModuleName().c_str(),
        sceneSessionInfo->want.GetElement().GetURI().c_str(),
        sceneSessionInfo->want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0),
        sceneSessionInfo->requestId);
    int32_t errCode = ERR_OK;
    bool isColdStart = false;
    if (!sceneSession->IsSessionForeground()) {
        listenerController_->NotifySessionLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::FOREGROUND,
            sceneSession->GetSessionInfo());
    }
    if (!systemConfig_.backgroundswitch || sceneSession->GetSessionProperty()->GetIsAppSupportPhoneInPc()) {
        TLOGI(WmsLogTag::WMS_MAIN, "[id: %{public}d] Begin StartUIAbility, system: %{public}u", persistentId,
            static_cast<uint32_t>(sceneSession->GetSessionInfo().isSystem_));
        if (sceneSession->GetSessionInfo().want != nullptr) {
            sceneSession->GetSessionInfo().want->RemoveParam("targetGrantBundleName");
        }
        errCode = StartUIAbilityBySCBTimeoutCheck(sceneSession, sceneSessionInfo,
            static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL), isColdStart);
        ResetSessionInfoAfterStartUIAbility(sceneSession);
    } else {
        TLOGI(WmsLogTag::WMS_MAIN, "[id: %{public}d] Background switch on, isNewActive %{public}d state %{public}u "
            "reuseDelegatorWindow %{public}d", persistentId,
            isNewActive, sceneSession->GetSessionState(), sceneSession->GetSessionInfo().reuseDelegatorWindow);
        if (isNewActive || sceneSession->GetSessionState() == SessionState::STATE_DISCONNECT ||
            sceneSession->GetSessionState() == SessionState::STATE_END ||
            sceneSession->GetSessionInfo().reuseDelegatorWindow) {
            TLOGI(WmsLogTag::WMS_MAIN, "Call StartUIAbility: %{public}d system: %{public}u", persistentId,
                static_cast<uint32_t>(sceneSession->GetSessionInfo().isSystem_));
            if (sceneSession->GetSessionInfo().want != nullptr) {
                sceneSession->GetSessionInfo().want->RemoveParam("targetGrantBundleName");
            }
            errCode = StartUIAbilityBySCBTimeoutCheck(sceneSession, sceneSessionInfo,
                static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL), isColdStart);
        } else {
            TLOGI(WmsLogTag::WMS_MAIN, "NotifySessionForeground: %{public}d", persistentId);
            sceneSession->NotifySessionForeground(1, true);
        }
    }
    if (WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
        WindowInfoReporter::GetInstance().InsertShowReportInfo(sceneSession->GetSessionInfo().bundleName_);
    }
    NotifyCollaboratorAfterStart(sceneSession, sceneSessionInfo);
    if (errCode != ERR_OK) {
        TLOGI(WmsLogTag::WMS_MAIN, "failed! errCode: %{public}d", errCode);
        ExceptionInfo exceptionInfo;
        exceptionInfo.needRemoveSession = true;
        sceneSession->NotifySessionExceptionInner(sceneSessionInfo, exceptionInfo, false, true);
        if (startUIAbilityErrorFunc_ && static_cast<WSError>(errCode) == WSError::WS_ERROR_EDM_CONTROLLED) {
            startUIAbilityErrorFunc_(
                static_cast<uint32_t>(WS_JS_TO_ERROR_CODE_MAP.at(WSError::WS_ERROR_EDM_CONTROLLED)));
        }
    }
    if (isColdStart) {
        TLOGI(WmsLogTag::WMS_MAIN, "Cold start, identityToken:%{public}s, bundleName:%{public}s",
            sceneSessionInfo->identityToken.c_str(), sceneSession->GetSessionInfo().bundleName_.c_str());
        sceneSession->SetClientIdentityToken(sceneSessionInfo->identityToken);
        sceneSession->ResetSessionConnectState();
        sceneSession->ResetIsActive();
        sceneSession->UpdatePrivacyModeControlInfo();
    }
    return WSError::WS_OK;
}

void SceneSessionManager::ResetSessionInfoAfterStartUIAbility(const sptr<SceneSession>& sceneSession) {
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    if (sceneSession->GetRestartApp()) {
        sceneSession->SetRestartApp(false);
        sceneSession->GetSessionProperty()->SetIsNeedUpdateWindowMode(true);
        sceneSession->SetRestartInSameProcess(true);
    }
}

void SceneSessionManager::AddRequestTaskInfo(sptr<SceneSession> sceneSession, int32_t requestId) {
    if (sceneSession == nullptr) {
        return;
    }
    const SessionInfo& sessionInfo = sceneSession->GetSessionInfo();
    int32_t persistentId = sceneSession->GetPersistentId();
    if (requestId < MIN_REQUEST_ID_FROM_ABILITY ||
        persistentId == INVALID_SESSION_ID ||
        sessionInfo.want == nullptr) {
        return;
    }
    AAFwk::Want wantTmp = sessionInfo.GetWantSafely();
    size_t requestIdToWantMapSize = 0;
    {
        std::lock_guard<std::mutex> lock(requestTaskInfoMapMutex_);
        if (requestTaskInfoMap.find(persistentId) == requestTaskInfoMap.end()) {
            requestTaskInfoMap.emplace(persistentId, std::make_shared<RequestTaskInfo>());
        }
        auto& requestIdToWantMap = requestTaskInfoMap[persistentId]->requestIdToWantMap;
        requestIdToWantMap[requestId] = wantTmp;
        requestIdToWantMapSize = requestIdToWantMap.size();
    }
    TLOGI(WmsLogTag::WMS_LIFE, "persistentId:%{public}d, requestId:%{public}d, "
        "infoMap size:%{public}u, wantMap size:%{public}u",
        persistentId, requestId, requestTaskInfoMap.size(), requestIdToWantMapSize);
}
 
void SceneSessionManager::RemoveRequestTaskInfo(int32_t persistentId, int32_t requestId) {
    if (requestId < MIN_REQUEST_ID_FROM_ABILITY || persistentId == INVALID_SESSION_ID) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(requestTaskInfoMapMutex_);
        if (requestTaskInfoMap.find(persistentId) == requestTaskInfoMap.end()) {
            return;
        }
        requestTaskInfoMap[persistentId]->requestIdToWantMap.erase(requestId);
    }
    TLOGI(WmsLogTag::WMS_LIFE, "persistentId:%{public}d, requestId:%{public}d, "
        "infoMap size:%{public}u, wantMap size:%{public}u",
        persistentId, requestId, requestTaskInfoMap.size(),
        requestTaskInfoMap[persistentId]->requestIdToWantMap.size());
}

void SceneSessionManager::ClearRequestTaskInfo(int32_t persistentId) {
    if (persistentId == INVALID_SESSION_ID) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(requestTaskInfoMapMutex_);
        requestTaskInfoMap.erase(persistentId);
    }
    TLOGI(WmsLogTag::WMS_LIFE, "persistentId:%{public}d", persistentId);
}

std::shared_ptr<AAFwk::Want> SceneSessionManager::GetRequestWantFromTaskInfoMap(
    int32_t persistentId, int32_t requestId) {
    if (requestId < MIN_REQUEST_ID_FROM_ABILITY || persistentId == INVALID_SESSION_ID) {
        return nullptr;
    }
    std::shared_ptr<AAFwk::Want> retPtr = nullptr;
    {
        std::lock_guard<std::mutex> lock(requestTaskInfoMapMutex_);
        if (requestTaskInfoMap.find(persistentId) == requestTaskInfoMap.end()) {
            return nullptr;
        }
        auto& requestIdToWantMap = requestTaskInfoMap[persistentId]->requestIdToWantMap;
        if (requestIdToWantMap.find(requestId) == requestIdToWantMap.end()) {
            return nullptr;
        }
        retPtr = std::make_shared<AAFwk::Want>(requestIdToWantMap[requestId]);
    }
    TLOGI(WmsLogTag::WMS_LIFE, "exsit, persistentId:%{public}d, requestId:%{public}d",
        persistentId, requestId);
    return retPtr;
}

void SceneSessionManager::NotifyCollaboratorAfterStart(sptr<SceneSession>& sceneSession,
    sptr<AAFwk::SessionInfo>& sceneSessionInfo)
{
    if (sceneSession == nullptr || sceneSessionInfo == nullptr) {
        return;
    }
    if (CheckCollaboratorType(sceneSession->GetCollaboratorType())) {
        NotifyLoadAbility(sceneSession->GetCollaboratorType(),
            sceneSessionInfo, sceneSession->GetSessionInfo().abilityInfo);
        NotifyUpdateSessionInfo(sceneSession);
        NotifyMoveSessionToForeground(sceneSession->GetCollaboratorType(), sceneSessionInfo->persistentId);
        sceneSession->SetSessionInfoAncoSceneState(AncoSceneState::NOTIFY_FOREGROUND);
    }
}

bool SceneSessionManager::IsPcSceneSessionLifecycle(const sptr<SceneSession>& sceneSession)
{
    bool isPcAppInLargeScreenDevice = sceneSession->GetSessionProperty()->GetIsPcAppInPad();
    bool isAppSupportPhoneInPc = sceneSession->GetSessionProperty()->GetIsAppSupportPhoneInPc();
    bool isScreenLock = IsScreenLocked();
    if ((systemConfig_.backgroundswitch && !isAppSupportPhoneInPc) ||
        (isPcAppInLargeScreenDevice && !isScreenLock && !systemConfig_.IsPhoneWindow())) {
        return true;
    }
    if (isTrayAppForeground_ && isPcAppInLargeScreenDevice) {
        TLOGI(WmsLogTag::WMS_PC, "isPcAppInPad check trayApp success");
        return true;
    }
    return false;
}

void SceneSessionManager::InitSnapshotCache()
{
    const static std::unordered_map<WindowUIType, std::size_t> SNAPSHOT_CACHE_CAPACITY_MAP = {
        {WindowUIType::PC_WINDOW,      MAX_SNAPSHOT_IN_RECENT_PC},
        {WindowUIType::PAD_WINDOW,     MAX_SNAPSHOT_IN_RECENT_PAD},
        {WindowUIType::PHONE_WINDOW,   MAX_SNAPSHOT_IN_RECENT_PHONE},
        {WindowUIType::INVALID_WINDOW, MAX_SNAPSHOT_IN_RECENT_PHONE},
    };

    snapshotCapacity_ = SNAPSHOT_CACHE_CAPACITY_MAP.at(WindowUIType::INVALID_WINDOW);
    auto uiType = systemConfig_.windowUIType_;
    if (SNAPSHOT_CACHE_CAPACITY_MAP.find(uiType) != SNAPSHOT_CACHE_CAPACITY_MAP.end()) {
        snapshotCapacity_ = SNAPSHOT_CACHE_CAPACITY_MAP.at(uiType);
    }
    TLOGI(WmsLogTag::WMS_PATTERN, "type: %{public}hhu", uiType);
    snapshotLruCache_ = std::make_unique<LruCache>(snapshotCapacity_);
}

void SceneSessionManager::PutSnapshotToCache(int32_t persistentId)
{
    TLOGD(WmsLogTag::WMS_PATTERN, "session:%{public}d", persistentId);
    if (int32_t removedCacheId = snapshotLruCache_->Put(persistentId);
        removedCacheId != UNDEFINED_REMOVED_KEY) {
        if (auto removedCacheSession = GetSceneSession(removedCacheId)) {
            removedCacheSession->ResetSnapshot();
        } else {
            TLOGW(WmsLogTag::WMS_PATTERN, "removedCacheSession:%{public}d nullptr", removedCacheId);
        }
    }
}

void SceneSessionManager::VisitSnapshotFromCache(int32_t persistentId)
{
    TLOGD(WmsLogTag::WMS_PATTERN, "session:%{public}d", persistentId);
    if (!snapshotLruCache_->Visit(persistentId)) {
        TLOGD(WmsLogTag::WMS_PATTERN, "session:%{public}d not in cache", persistentId);
    }
}

void SceneSessionManager::RemoveSnapshotFromCache(int32_t persistentId)
{
    TLOGD(WmsLogTag::WMS_PATTERN, "session:%{public}d", persistentId);
    snapshotLruCache_->Remove(persistentId);
    if (auto sceneSession = GetSceneSession(persistentId)) {
        sceneSession->ResetSnapshot();
    }
}

WSError SceneSessionManager::RegisterSaveSnapshotFunc(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "session is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (!WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    auto persistentId = sceneSession->GetPersistentId();
    sceneSession->SetSaveSnapshotCallback([this, persistentId]() {
        this->PutSnapshotToCache(persistentId);
    });
    return WSError::WS_OK;
}

void SceneSessionManager::RegisterIsAppBoundSystemTrayFunc(const sptr<SceneSession> &sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "session is nullptr");
        return;
    }
    if (!WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_MAIN, "id: %{public}d is not main window", sceneSession->GetPersistentId());
        return;
    }
    sceneSession->RegisterIsAppBoundSystemTrayCallback(
        [this](int32_t callingPid, uint32_t callingToken, const std::string &instanceKey) {
            return IsAppBoundSystemTray(callingPid, callingToken, instanceKey);
        });
}

void SceneSessionManager::ConfigSupportSnapshotAllSessionStatus()
{
    TLOGI(WmsLogTag::WMS_PATTERN, "support");
    auto task = [this] {
        systemConfig_.supportSnapshotAllSessionStatus_ = true;
    };
    taskScheduler_->PostAsyncTask(task, "ConfigSupportSnapshotAllSessionStatus");
}

void SceneSessionManager::ConfigSupportCacheLockedSessionSnapshot()
{
    TLOGI(WmsLogTag::WMS_PATTERN, "support");
    auto task = [this] {
        systemConfig_.supportCacheLockedSessionSnapshot_ = true;
    };
    taskScheduler_->PostAsyncTask(task, "ConfigSupportCacheLockedSessionSnapshot");
}

void SceneSessionManager::ConfigSupportPreloadStartingWindow()
{
    TLOGI(WmsLogTag::WMS_PATTERN, "support");
    auto task = [this] {
        systemConfig_.supportPreloadStartingWindow_ = true;
    };
    taskScheduler_->PostAsyncTask(task, "ConfigSupportPreloadStartingWindow");
}

WMError SceneSessionManager::CreateUIEffectController(const sptr<IUIEffectControllerClient>& controllerClient,
        sptr<IUIEffectController>& controller, int32_t& controllerId)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "not system calling, permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (!systemConfig_.IsPhoneWindow() && (!systemConfig_.IsPadWindow() || systemConfig_.IsFreeMultiWindowMode())) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "device not support!");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    return UIEffectManager::GetInstance().CreateUIEffectController(controllerClient, controller, controllerId);
}

WSError SceneSessionManager::RequestSceneSessionBackground(const sptr<SceneSession>& sceneSession,
    const bool isDelegator, const bool isToDesktop, const bool isSaveSnapshot, LifeCycleChangeReason reason)
{
    auto task = [this, weakSceneSession = wptr(sceneSession), isDelegator, isToDesktop, isSaveSnapshot, reason] {
        auto sceneSession = weakSceneSession.promote();
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "session is nullptr");
            return WSError::WS_ERROR_NULLPTR;
        }
        auto persistentId = sceneSession->GetPersistentId();
        TLOGNI(WmsLogTag::WMS_MAIN, "[id: %{public}d] Request background, isDelegator:%{public}d "
            "isToDesktop:%{public}d isSaveSnapshot:%{public}d",
            persistentId, isDelegator, isToDesktop, isSaveSnapshot);
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:RequestSceneSessionBackground (%d )", persistentId);
        TLOGNI(WmsLogTag::WMS_LIFE, "Notify scene session id: %{public}d paused", sceneSession->GetPersistentId());
        sceneSession->UpdateLifecyclePausedInner();
        sceneSession->SetActive(false);
        sceneSession->UpdatePrivacyModeControlInfo();

        if (isToDesktop) {
            sceneSession->EditSessionInfo().callerToken_ = nullptr;
            sceneSession->EditSessionInfo().callingTokenId_ = 0;
        }

        sceneSession->BackgroundTask(isSaveSnapshot, reason);
        listenerController_->NotifySessionLifecycleEvent(
            ISessionLifecycleListener::SessionLifecycleEvent::BACKGROUND, sceneSession->GetSessionInfo(), reason);
        if (!GetSceneSession(persistentId)) {
            TLOGNE(WmsLogTag::WMS_MAIN, "Request background session invalid by %{public}d", persistentId);
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        if (persistentId == brightnessSessionId_) {
            auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
            auto focusedSessionId = windowFocusController_->GetFocusedSessionId(displayId);
            UpdateBrightness(focusedSessionId);
        }
        if (IsPcSceneSessionLifecycle(sceneSession)) {
            TLOGNI(WmsLogTag::WMS_MAIN, "[id: %{public}d] Notify session background", persistentId);
            sceneSession->NotifySessionBackground(1, true, true);
        } else {
            TLOGNI(WmsLogTag::WMS_MAIN, "[id: %{public}d] begin MinimzeUIAbility, system: %{public}u",
                persistentId, static_cast<uint32_t>(sceneSession->GetSessionInfo().isSystem_));
            auto sceneSessionInfo = SetAbilitySessionInfo(sceneSession);
            if (!isDelegator) {
                AAFwk::AbilityManagerClient::GetInstance()->MinimizeUIAbilityBySCB(sceneSessionInfo, false,
                    static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL));
            } else {
                AAFwk::AbilityManagerClient::GetInstance()->MinimizeUIAbilityBySCB(sceneSessionInfo, true,
                    static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL));
            }
        }

        if (WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
            WindowInfoReporter::GetInstance().InsertHideReportInfo(sceneSession->GetSessionInfo().bundleName_);
        }
        return WSError::WS_OK;
    };
    std::string taskName = "RequestSceneSessionBackground:PID:" +
        (sceneSession != nullptr ? std::to_string(sceneSession->GetPersistentId()):"nullptr");
    taskScheduler_->PostAsyncTask(task, taskName);
    return WSError::WS_OK;
}

void SceneSessionManager::NotifyForegroundInteractiveStatus(const sptr<SceneSession>& sceneSession, bool interactive)
{
    taskScheduler_->PostAsyncTask([this, weakSceneSession = wptr(sceneSession), interactive] {
        auto sceneSession = weakSceneSession.promote();
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is nullptr");
            return;
        }
        auto persistentId = sceneSession->GetPersistentId();
        TLOGNI(WmsLogTag::WMS_LIFE, "NotifyForeInteractive id: %{public}d, status: %{public}d", persistentId, interactive);
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:NotifyForegroundInteractiveStatus (%d )", persistentId);
        if (!GetSceneSession(persistentId)) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is invalid with %{public}d", persistentId);
            return;
        }
        sceneSession->NotifyForegroundInteractiveStatus(interactive);
    }, __func__);
}

WSError SceneSessionManager::DestroyDialogWithMainWindow(const sptr<SceneSession>& sceneSession)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:DestroyDialogWithMainWindow");
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "session is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        TLOGI(WmsLogTag::WMS_DIALOG, "Begin to destroy dialog, parentId: %{public}d", sceneSession->GetPersistentId());
        const auto& dialogVec = sceneSession->GetDialogVector();
        for (const auto& dialog : dialogVec) {
            if (dialog == nullptr) {
                TLOGE(WmsLogTag::WMS_DIALOG, "dialog is nullptr");
                continue;
            }
            auto sceneSession = GetSceneSession(dialog->GetPersistentId());
            if (sceneSession == nullptr) {
                TLOGE(WmsLogTag::WMS_DIALOG, "dialog is invalid, id: %{public}d", dialog->GetPersistentId());
                return WSError::WS_ERROR_INVALID_SESSION;
            }
            WindowDestroyNotifyVisibility(sceneSession);
            dialog->NotifyDestroy();
            dialog->Disconnect();

            auto dialogSceneSession = GetSceneSession(dialog->GetPersistentId());
            if (dialogSceneSession != nullptr) {
                dialogSceneSession->ClearSpecificSessionCbMap();
            }
            {
                std::unique_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
                EraseSceneSessionAndMarkDirtyLocked(dialog->GetPersistentId());
                systemTopSceneSessionMap_.erase(dialog->GetPersistentId());
                nonSystemFloatSceneSessionMap_.erase(dialog->GetPersistentId());
            }
        }
        sceneSession->ClearDialogVector();
        return WSError::WS_OK;
    }
    return WSError::WS_ERROR_INVALID_SESSION;
}

void SceneSessionManager::DestroySubSession(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGW(WmsLogTag::WMS_SUB, "sceneSession is nullptr");
        return;
    }
    for (const auto& subSession : sceneSession->GetSubSession()) {
        if (subSession != nullptr) {
            const auto persistentId = subSession->GetPersistentId();
            TLOGI(WmsLogTag::WMS_SUB, "id: %{public}d", persistentId);
            DestroyAndDisconnectSpecificSessionInner(persistentId);
        }
    }
}

void SceneSessionManager::DestroyToastSession(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "ToastSession is nullptr");
        return;
    }
    for (const auto& toastSession : sceneSession->GetToastSession()) {
        if (toastSession != nullptr) {
            const auto persistentId = toastSession->GetPersistentId();
            TLOGI(WmsLogTag::WMS_TOAST, "id: %{public}d", persistentId);
            DestroyAndDisconnectSpecificSessionInner(persistentId);
        }
    }
}

void SceneSessionManager::BuildCancelPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
                                                  int32_t fingerId, int32_t action, int32_t wid)
{
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "pointerEvent is null, wid:%{public}d fingerId:%{public}d action:%{public}d",
                  wid, fingerId, action);
        return;
    }
    pointerEvent->SetId(CANCEL_POINTER_ID);
    pointerEvent->SetTargetWindowId(wid);
    pointerEvent->SetPointerId(fingerId);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_CANCEL);
    MMI::PointerEvent::PointerItem item;
    item.SetPointerId(fingerId);
    pointerEvent->AddPointerItem(item);
    if (action == MMI::PointerEvent::POINTER_ACTION_DOWN) {
        pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    } else {
        pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    }
}

void SceneSessionManager::SendCancelEventBeforeEraseSession(const sptr<SceneSession>& sceneSession)
{
    auto task = [this, needCancelEventSceneSession = sceneSession] {
        if (needCancelEventSceneSession == nullptr) {
            TLOGI(WmsLogTag::WMS_EVENT, "scenesession is nullptr, needn't send cancel event");
            return;
        }
        auto wid = needCancelEventSceneSession->GetPersistentId();
        if (needCancelEventSceneSession->GetMousePointerDownEventStatus()) {
            std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
            BuildCancelPointerEvent(pointerEvent, 1, MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN, wid);
            TLOGI(WmsLogTag::WMS_EVENT, "erasing sceneSession need send mouse cancel. wid:%{public}d", wid);
            needCancelEventSceneSession->SetMousePointerDownEventStatus(false);
            needCancelEventSceneSession->SendPointerEventToUI(pointerEvent);
        }
        std::unordered_set<int32_t> fingerPointerDownStatusList = needCancelEventSceneSession->GetFingerPointerDownStatusList();
        if (fingerPointerDownStatusList.empty()) {
            return;
        }
        for (auto fingerId : fingerPointerDownStatusList) {
            std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
            BuildCancelPointerEvent(pointerEvent, fingerId, MMI::PointerEvent::POINTER_ACTION_DOWN, wid);
            TLOGI(WmsLogTag::WMS_EVENT, "erasing sceneSession need send touch cancel. wid:%{public}d fingerId:%{public}d",
                  wid, fingerId);
            needCancelEventSceneSession->SendPointerEventToUI(pointerEvent);
            needCancelEventSceneSession->RemoveFingerPointerDownStatus(fingerId);
        }
    };
    mainHandler_->PostTask(std::move(task), "wms:sendCancelBeforeEraseSession", 0, AppExecFwk::EventQueue::Priority::VIP);
}

void SceneSessionManager::EraseSceneSessionMapById(int32_t persistentId)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession != nullptr) {
        RemovePreLoadStartingWindowFromMap(sceneSession->GetSessionInfo());
        sceneSession->ClearLifeCycleTask();
        sceneSession->ClearSnapshotPersistence();
    } else {
        TLOGW(WmsLogTag::WMS_PATTERN, "session is nullptr id: %{public}d", persistentId);
    }
    std::unique_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    EraseSceneSessionAndMarkDirtyLocked(persistentId);
    systemTopSceneSessionMap_.erase(persistentId);
    nonSystemFloatSceneSessionMap_.erase(persistentId);
    SendCancelEventBeforeEraseSession(sceneSession);
    if (sceneSession && MultiInstanceManager::IsSupportMultiInstance(systemConfig_) &&
        MultiInstanceManager::GetInstance().IsMultiInstance(sceneSession->GetSessionInfo().bundleName_)) {
        MultiInstanceManager::GetInstance().DecreaseInstanceKeyRefCount(sceneSession);
    }
}

/**
 * if visible session is erased, mark dirty
 * lock-free
 */
void SceneSessionManager::EraseSceneSessionAndMarkDirtyLocked(int32_t persistentId)
{
    // get scene session lock-free
    auto iter = sceneSessionMap_.find(persistentId);
    if (iter == sceneSessionMap_.end()) {
        TLOGW(WmsLogTag::WMS_MAIN, "id: %{public}d not exist", persistentId);
        return;
    }
    const auto& sceneSession = iter->second;
    if (sceneSession != nullptr && sceneSession->IsVisible()) {
        sessionMapDirty_ |= static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE);
    }
    sceneSessionMap_.erase(persistentId);
}

WSError SceneSessionManager::RequestSceneSessionDestruction(const sptr<SceneSession>& sceneSession,
    bool needRemoveSession, bool isSaveSnapshot, bool isForceClean, bool isUserRequestedExit,
    LifeCycleChangeReason reason)
{
    auto task = [this, weakSceneSession = wptr(sceneSession), needRemoveSession, isSaveSnapshot, isForceClean,
                 isUserRequestedExit, reason]() THREAD_SAFETY_GUARD(SCENE_GUARD) {
        auto sceneSession = weakSceneSession.promote();
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "Destruct session is nullptr");
            return WSError::WS_ERROR_NULLPTR;
        }
        auto persistentId = sceneSession->GetPersistentId();
        TLOGNI(WmsLogTag::WMS_MAIN, "[id: %{public}d] Destruct session, remove:%{public}d isSaveSnapshot:%{public}d "
            "isForceClean:%{public}d isUserRequestedExit:%{public}d", persistentId, needRemoveSession, isSaveSnapshot,
            isForceClean, isUserRequestedExit);
        RequestSessionUnfocus(persistentId, FocusChangeReason::SCB_SESSION_REQUEST_UNFOCUS);
        avoidAreaListenerSessionSet_.erase(persistentId);
        screenshotAppEventListenerSessionSet_.erase(persistentId);
        RemoveSessionFromBlackList(sceneSession);
        DestroyDialogWithMainWindow(sceneSession);
        DestroyToastSession(sceneSession);
        DestroySubSession(sceneSession); // destroy sub session by destruction
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:RequestSceneSessionDestruction (%" PRIu32" )", persistentId);
        if (WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
            WindowInfoReporter::GetInstance().InsertDestroyReportInfo(sceneSession->GetSessionInfo().bundleName_);
        }
        WindowDestroyNotifyVisibility(sceneSession);
        NotifySessionUpdate(sceneSession->GetSessionInfo(), ActionType::SINGLE_CLOSE);
        sceneSession->SetSaveSnapshotCallback([this, persistentId]() {
            this->RemoveSnapshotFromCache(persistentId);
        });
        sceneSession->DisconnectTask(false, isSaveSnapshot);
        if (!GetSceneSession(persistentId)) {
            TLOGNE(WmsLogTag::WMS_MAIN, "Destruct session invalid by %{public}d", persistentId);
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        auto sceneSessionInfo = SetAbilitySessionInfo(sceneSession);
        sceneSession->GetCloseAbilityWantAndClean(sceneSessionInfo->want);
        ResetSceneSessionInfoWant(sceneSessionInfo);
        return RequestSceneSessionDestructionInner(
            sceneSession, sceneSessionInfo, needRemoveSession, isForceClean, isUserRequestedExit, reason);
    };
    std::string taskName = "RequestSceneSessionDestruction:PID:" +
        (sceneSession != nullptr ? std::to_string(sceneSession->GetPersistentId()) : "nullptr");
    taskScheduler_->PostAsyncTask(task, taskName);
    return WSError::WS_OK;
}

void SceneSessionManager::ResetSceneSessionInfoWant(const sptr<AAFwk::SessionInfo>& sceneSessionInfo)
{
    if (sceneSessionInfo->resultCode == -1) {
        AAFwk::Want want;
        std::string keySessionId = sceneSessionInfo->want.GetStringParam(ATOMIC_SERVICE_SESSION_ID);
        want.SetParam(ATOMIC_SERVICE_SESSION_ID, keySessionId);
        sceneSessionInfo->want = std::move(want);
        TLOGI(WmsLogTag::WMS_MAIN, "keySessionId: %{public}s", keySessionId.c_str());
    }
}

void SceneSessionManager::ResetWantInfo(const sptr<SceneSession>& sceneSession)
{
    if (const auto sessionInfoWant = sceneSession->GetSessionInfo().want) {
        const auto& bundleName = sessionInfoWant->GetElement().GetBundleName();
        const auto& abilityName = sessionInfoWant->GetElement().GetAbilityName();
        const auto& keySessionId = sessionInfoWant->GetStringParam(ATOMIC_SERVICE_SESSION_ID);
        AppExecFwk::ElementName element;
        element.SetBundleName(bundleName);
        element.SetAbilityName(abilityName);
        auto want = std::make_shared<AAFwk::Want>();
        want->SetElement(element);
        want->SetBundle(bundleName);
        if (!keySessionId.empty()) {
            want->SetParam(ATOMIC_SERVICE_SESSION_ID, keySessionId);
        }
        sceneSession->SetSessionInfoWant(want);
    }
}

WSError SceneSessionManager::RequestSceneSessionDestructionInner(sptr<SceneSession>& sceneSession,
    sptr<AAFwk::SessionInfo> sceneSessionInfo, const bool needRemoveSession, const bool isForceClean,
    bool isUserRequestedExit, LifeCycleChangeReason reason)
{
    auto persistentId = sceneSession->GetPersistentId();
    TLOGI(WmsLogTag::WMS_MAIN, "[id: %{public}d] Begin CloseUIAbility, system: %{public}d",
        persistentId, sceneSession->GetSessionInfo().isSystem_);
    if (isForceClean) {
        AAFwk::AbilityManagerClient::GetInstance()->CleanUIAbilityBySCB(sceneSessionInfo, isUserRequestedExit,
            static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL));
    } else {
        ffrtQueueHelper_->SubmitTask([sceneSessionInfo, persistentId, isUserRequestedExit, where = __func__] {
            auto ret = AAFwk::AbilityManagerClient::GetInstance()->CloseUIAbilityBySCB(sceneSessionInfo,
                isUserRequestedExit, static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL));
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s close ability ret:%{public}d, persistentId:%{public}d",
                where, static_cast<int32_t>(ret), persistentId);
        });
    }
    sceneSession->SetIsUserRequestedExit(isUserRequestedExit);
    sceneSession->SetSessionInfoAncoSceneState(AncoSceneState::DEFAULT_STATE);
    if (needRemoveSession) {
        if (CheckCollaboratorType(sceneSession->GetCollaboratorType())) {
            NotifyClearSession(sceneSession->GetCollaboratorType(), sceneSessionInfo->persistentId);
        }
        EraseSceneSessionMapById(persistentId);
    } else {
        // if terminate, reset want. so start from recent, start a new one.
        TLOGI(WmsLogTag::WMS_MAIN, "reset want: %{public}d", persistentId);
        if (CheckCollaboratorType(sceneSession->GetCollaboratorType())) {
            sceneSession->SetSessionInfoWant(nullptr);
        }
        ResetWantInfo(sceneSession);
        sceneSession->ResetSessionInfoResultCode();
        sceneSession->EditSessionInfo().isSetStartWindowType_ = false;
    }
    ClearRequestTaskInfo(persistentId);
    NotifySessionForCallback(sceneSession, needRemoveSession, reason);
    // Clear js cb map if needed.
    sceneSession->ClearJsSceneSessionCbMap(needRemoveSession);
    return WSError::WS_OK;
}

void SceneSessionManager::AddClientDeathRecipient(const sptr<ISessionStage>& sessionStage,
    const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr || sessionStage == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "sessionStage(%{public}d) or sceneSession is null", sessionStage == nullptr);
        return;
    }

    auto remoteObject = sessionStage->AsObject();
    remoteObjectMap_.insert(std::make_pair(remoteObject, sceneSession->GetPersistentId()));
    if (!remoteObject->AddDeathRecipient(windowDeath_)) {
        TLOGE(WmsLogTag::WMS_LIFE, "failed to add death recipient");
        return;
    }
    TLOGD(WmsLogTag::WMS_LIFE, "Id: %{public}d", sceneSession->GetPersistentId());
}

void SceneSessionManager::DestroySpecificSession(const sptr<IRemoteObject>& remoteObject)
{
    taskScheduler_->PostAsyncTask([this, remoteObject] {
        auto iter = remoteObjectMap_.find(remoteObject);
        if (iter == remoteObjectMap_.end()) {
            TLOGNE(WmsLogTag::WMS_DIALOG, "Invalid remoteObject");
            return;
        }
        TLOGND(WmsLogTag::WMS_DIALOG, "Remote died, id: %{public}d", iter->second);
        auto sceneSession = GetSceneSession(iter->second);
        if (sceneSession == nullptr) {
            TLOGNW(WmsLogTag::WMS_DIALOG, "Remote died, session is nullptr, id: %{public}d", iter->second);
            return;
        }
        DestroyAndDisconnectSpecificSessionInner(iter->second);
    }, __func__);
}

WSError SceneSessionManager::CreateAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
    const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
    sptr<WindowSessionProperty> property, int32_t& persistentId, sptr<ISession>& session,
    SystemSessionConfig& systemConfig, sptr<IRemoteObject> token)
{
    if (!CheckSystemWindowPermission(property) || !CheckModalSubWindowPermission(property)) {
        TLOGE(WmsLogTag::WMS_LIFE, "create system window or modal subwindow permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    auto parentSession = GetSceneSession(property->GetParentPersistentId());
    if (parentSession) {
        auto parentProperty = parentSession->GetSessionProperty();
        if (parentProperty->GetSubWindowLevel() >= MAX_SUB_WINDOW_LEVEL &&
            !WindowHelper::IsToastSubWindow(property->GetWindowType(), property->GetWindowFlags())) {
            TLOGE(WmsLogTag::WMS_SUB, "sub window level exceeds limit");
            return WSError::WS_ERROR_INVALID_WINDOW;
        }
        property->SetSubWindowLevel(parentProperty->GetSubWindowLevel() + 1);
        if (parentSession->GetSessionInfo().isSystem_ && property->GetIsUIExtFirstSubWindow() &&
            systemConfig_.supportUIExtensionSubWindow_) {
            property->SetWindowType(WindowType::WINDOW_TYPE_SCB_SUB_WINDOW);
        }
    }
    auto initClientDisplayId = UpdateSpecificSessionClientDisplayId(property);
    bool shouldBlock = false;
    bool isSystemCalling = SessionPermission::IsSystemCalling();
    if (!isSystemCalling) {
        if (property->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT && !systemConfig_.IsPcWindow()) {
            shouldBlock = property->IsFloatingWindowAppType() && shouldHideNonSecureFloatingWindows_.load();
        } else if (SessionHelper::IsNonSecureToUIExtension(property->GetWindowType()) && parentSession) {
            shouldBlock = parentSession->GetCombinedExtWindowFlags().hideNonSecureWindowsFlag;
        }
    }
    if (shouldBlock) {
        TLOGE(WmsLogTag::WMS_UIEXT, "create non-secure window permission denied!");
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    bool isPhoneOrPad = systemConfig_.IsPhoneWindow() || systemConfig_.IsPadWindow();
    if (!isPhoneOrPad && property->GetWindowType() == WindowType::WINDOW_TYPE_MUTISCREEN_COLLABORATION) {
        TLOGE(WmsLogTag::WMS_LIFE, "only phone or pad can create mutiScreen collaboration window");
        return WSError::WS_ERROR_INVALID_OPERATION;
    }

    if (property->GetWindowType() == WindowType::WINDOW_TYPE_APP_SUB_WINDOW) {
        WSError err = CheckSubSessionStartedByExtension(token, property);
        if (err != WSError::WS_OK) {
            return err;
        }
        SetExtensionSubSessionDisplayId(property, sessionStage);
    }
    // WINDOW_TYPE_SYSTEM_ALARM_WINDOW has been deprecated, will be deleted after 5 versions.
    if (property->GetWindowType() == WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW) {
        TLOGE(WmsLogTag::DEFAULT, "The alarm window has been deprecated!");
        return WSError::WS_ERROR_INVALID_WINDOW;
    }

    if (property->GetWindowType() == WindowType::WINDOW_TYPE_FB) {
        auto ret = IsFloatingBallValid(parentSession);
        if (ret != WSError::WS_OK) {
            return ret;
        }
    }

    TLOGI(WmsLogTag::WMS_LIFE, "create specific start, name:%{public}s, type:%{public}d, touchable:%{public}d",
        property->GetWindowName().c_str(), property->GetWindowType(), property->GetTouchable());

    // Get pid and uid before posting task.
    auto pid = IPCSkeleton::GetCallingRealPid();
    auto uid = IPCSkeleton::GetCallingUid();
    auto task = [this, sessionStage, eventChannel, surfaceNode, property, &persistentId, &session, &systemConfig, token,
                 pid, uid, isSystemCalling, initClientDisplayId]() {
        if (property == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "property is nullptr");
            return WSError::WS_ERROR_NULLPTR;
        }
        if (property->GetWindowType() == WindowType::WINDOW_TYPE_PIP && !IsEnablePiPCreate(property)) {
            TLOGNE(WmsLogTag::WMS_PIP, "pip window is not enable to create.");
            return WSError::WS_DO_NOTHING;
        }
        const auto type = property->GetWindowType();
        // create specific session
        SessionInfo info;
        info.windowType_ = static_cast<uint32_t>(type);
        info.bundleName_ = property->GetSessionInfo().bundleName_;
        info.abilityName_ = property->GetSessionInfo().abilityName_;
        info.moduleName_ = property->GetSessionInfo().moduleName_;
        info.screenId_ = property->GetDisplayId();

        if (IsPiPForbidden(property, type)) {
            TLOGNE(WmsLogTag::WMS_PIP, "forbid pip");
            NotifyMulScreenPipStart(property, type);
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        ClosePipWindowIfExist(type);
        sptr<SceneSession> newSession = RequestSceneSession(info, property);
        if (newSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is nullptr");
            return WSError::WS_ERROR_NULLPTR;
        }
        newSession->SetClientDisplayId(initClientDisplayId);
        property->SetSystemCalling(isSystemCalling);
        auto errCode = newSession->ConnectInner(
            sessionStage, eventChannel, surfaceNode, systemConfig_, property, token, pid, uid);
        newSession->SetIsSystemSpecificSession(isSystemCalling);
        systemConfig = systemConfig_;
        persistentId = property->GetPersistentId();

        NotifyCreateSpecificSession(newSession, property, type);
        session = newSession;
        AddClientDeathRecipient(sessionStage, newSession);

        if (property->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT) {
            CheckFloatWindowIsAnco(pid, newSession);
        }

        UpdateSubSessionBlackList(newSession);
        TLOGNI(WmsLogTag::WMS_LIFE, "create specific session success, id: %{public}d, "
            "parentId: %{public}d, type: %{public}d",
            newSession->GetPersistentId(), newSession->GetParentPersistentId(), type);
        return errCode;
    };

    return taskScheduler_->PostSyncTask(task, "CreateAndConnectSpecificSession");
}

WSError SceneSessionManager::CheckSubSessionStartedByExtension(const sptr<IRemoteObject>& token,
    const sptr<WindowSessionProperty>& property)
{
    sptr<SceneSession> extensionParentSession = GetSceneSession(property->GetParentPersistentId());
    if (extensionParentSession == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionParentSession is invalid with %{public}d",
            property->GetParentPersistentId());
        return WSError::WS_ERROR_NULLPTR;
    }
    if (extensionParentSession->GetSessionInfo().isSystem_) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionParentSession is SCBSystemSession: %{public}d",
            property->GetParentPersistentId());
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    auto pid = IPCSkeleton::GetCallingRealPid();
    auto parentPid = extensionParentSession->GetCallingPid();
    if (pid == parentPid) { // Determine Whether to create a sub window in the same process.
        TLOGI(WmsLogTag::WMS_UIEXT, "pid == parentPid");
        return WSError::WS_OK;
    }
    AAFwk::UIExtensionSessionInfo info;
    AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionSessionInfo(token, info);
    if (info.persistentId != INVALID_SESSION_ID && info.hostWindowId != INVALID_SESSION_ID) {
        int32_t parentId = static_cast<int32_t>(info.hostWindowId);
        // Check the parent ids are the same in cross-process scenarios.
        if (parentId == property->GetParentPersistentId()) {
            TLOGD(WmsLogTag::WMS_UIEXT, "parentId == property->GetParentPersistentId(parentId:%{public}d)", parentId);
            return WSError::WS_OK;
        }
    }
    if (SessionPermission::IsSystemCalling()) {
        TLOGD(WmsLogTag::WMS_UIEXT, "is system app");
        return WSError::WS_OK;
    }

    TLOGE(WmsLogTag::WMS_UIEXT,
        "can't create sub window: persistentId:%{public}d, parentPersistentId:%{public}d, hostWindowId:%{public}d",
        property->GetPersistentId(), property->GetParentPersistentId(), info.hostWindowId);
    return WSError::WS_ERROR_INVALID_WINDOW;
}

void SceneSessionManager::SetExtensionSubSessionDisplayId(const sptr<WindowSessionProperty>& property,
    const sptr<ISessionStage>& sessionStage)
{
    sptr<SceneSession> extensionParentSession = GetSceneSession(property->GetParentPersistentId());
    if (extensionParentSession == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "extensionParentSession is invalid with %{public}d",
            property->GetParentPersistentId());
        return;
    }
    sptr<WindowSessionProperty> parentProperty = extensionParentSession->GetSessionProperty();
    if (sessionStage && property->GetIsUIExtFirstSubWindow()) {
        sessionStage->UpdateDisplayId(extensionParentSession->GetClientDisplayId() == VIRTUAL_DISPLAY_ID
                                            ? VIRTUAL_DISPLAY_ID
                                            : parentProperty->GetDisplayId());
        property->SetDisplayId(parentProperty->GetDisplayId());
    }
}

void SceneSessionManager::ReportSubWindowCreationFailure(const int32_t& pid, const std::string& abilityName,
        const std::string& parentBundleName, const std::string& hostBundleName)
{
    taskScheduler_->PostAsyncTask([pid, abilityName, parentBundleName, hostBundleName]() {
        std::ostringstream oss;
        oss << "The hostWindow is not this parentwindow ! parentwindow bundleName: " << parentBundleName;
        oss << ", hostwindow bundleName: " << hostBundleName;
        oss << ", abilityName: " << abilityName;
        SingletonContainer::Get<WindowInfoReporter>().ReportWindowException(
            static_cast<int32_t>(WindowDFXHelperType::WINDOW_CREATE_SUB_WINDOW_FAILED), pid, oss.str());
    }, __func__);
}

void SceneSessionManager::CheckFloatWindowIsAnco(pid_t pid, const sptr<SceneSession>& newSession)
{
    std::map<int32_t, sptr<SceneSession>> sceneSessionMapCopy;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        sceneSessionMapCopy = sceneSessionMap_;
    }
    for (const auto& [_, session] : sceneSessionMapCopy) {
        if (session && session->GetCallingPid() == pid &&
            session->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
            auto sessionInfo = session->GetSessionInfo();
            if (AbilityInfoManager::GetInstance().IsAnco(sessionInfo.bundleName_,
                    sessionInfo.abilityName_, sessionInfo.moduleName_)) {
                newSession->SetIsAncoForFloatingWindow(true);
                TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Set float window is anco, wid: %{public}d", newSession->GetWindowId());
                break;
            }
        }
    }
}

void SceneSessionManager::ClosePipWindowIfExist(WindowType type)
{
    if (type != WindowType::WINDOW_TYPE_PIP) {
        return;
    }
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, session] : sceneSessionMap_) {
        if (session && session->GetWindowType() == WindowType::WINDOW_TYPE_PIP) {
            session->NotifyCloseExistPipWindow();
            break;
        }
    }
}

bool SceneSessionManager::CheckPiPPriority(const PiPTemplateInfo& pipTemplateInfo, DisplayId displayId)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, session] : sceneSessionMap_) {
        if (session && session->GetWindowMode() == WindowMode::WINDOW_MODE_PIP &&
            pipTemplateInfo.priority < session->GetPiPTemplateInfo().priority &&
            session->IsSessionForeground()) {
            if (startPiPFailedFunc_) {
                startPiPFailedFunc_(displayId);
            }
            TLOGE(WmsLogTag::WMS_PIP, "create pip window failed, reason: low priority.");
            return false;
        }
    }
    return true;
}

std::string SceneSessionManager::GetScreenName(int32_t persistentId)
{
    auto session = GetSceneSession(persistentId);
    if (session == nullptr) {
        TLOGI(WmsLogTag::WMS_PIP, "session is null, persistentId=%{public}d", persistentId);
        return "";
    }
    sptr<WindowSessionProperty> sessionProperty = session->GetSessionProperty();
    DisplayId screenId = sessionProperty->GetDisplayId();
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid screenId");
        return "";
    }
    sptr<ScreenSession> screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid screenSession");
        return "";
    }
    return screenSession->GetName();
}

bool SceneSessionManager::IsEnablePiPCreate(const sptr<WindowSessionProperty>& property)
{
    if (property == nullptr) {
        TLOGI(WmsLogTag::WMS_PIP, "property is nullptr.");
        return false;
    }
    std::string screenName = GetScreenName(property->GetParentPersistentId());
    if (isScreenLocked_ && screenName != SUPER_LAUNCHER) {
        TLOGI(WmsLogTag::WMS_PIP, "skip create pip window as screen locked.");
        return false;
    }
    Rect pipRect = property->GetRequestRect();
    if (pipRect.width_ == 0 || pipRect.height_ == 0) {
        TLOGI(WmsLogTag::WMS_PIP, "pip rect is invalid.");
        return false;
    }
    if (!CheckPiPPriority(property->GetPiPTemplateInfo(), property->GetDisplayId())) {
        TLOGI(WmsLogTag::WMS_PIP, "skip create pip window by priority");
        return false;
    }
    auto parentSession = GetSceneSession(property->GetParentPersistentId());
    if (parentSession == nullptr || parentSession->GetSessionState() == SessionState::STATE_DISCONNECT) {
        TLOGI(WmsLogTag::WMS_PIP, "skip create pip window, maybe parentSession is null or disconnected");
        return false;
    }
    return true;
}

bool SceneSessionManager::IsPiPForbidden(const sptr<WindowSessionProperty>& property, const WindowType& type)
{
    sptr<SceneSession> parentSession = GetSceneSession(property->GetParentPersistentId());
    if (parentSession == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid parentSession");
        return false;
    }
    sptr<WindowSessionProperty> parentProperty = parentSession->GetSessionProperty();
    if (parentProperty == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid parentProperty");
        return false;
    }
    DisplayId screenId = parentProperty->GetDisplayId();
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid screenId");
        return false;
    }
    sptr<ScreenSession> screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSession(screenId);
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid screenSession");
        return false;
    }
    std::string screenName = screenSession->GetName();
    if (type == WindowType::WINDOW_TYPE_PIP && PIP_SCENE_FORBID_LIST.find(screenName) != PIP_SCENE_FORBID_LIST.end()) {
        TLOGI(WmsLogTag::WMS_PIP, "screen name %{public}s", screenName.c_str());
        return true;
    }

    if (type == WindowType::WINDOW_TYPE_PIP && !GetPipDeviceCollaborationPolicy(screenId)) {
        return true;
    }
    return false;
}

void SceneSessionManager::NotifyMulScreenPipStart(const sptr<WindowSessionProperty>& property, WindowType type)
{
    sptr<SceneSession> parentSession = GetSceneSession(property->GetParentPersistentId());
    if (parentSession == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid parentSession");
        return;
    }
    sptr<WindowSessionProperty> parentProperty = parentSession->GetSessionProperty();
    if (parentProperty == nullptr) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid parentProperty");
        return;
    }
    DisplayId screenId = parentProperty->GetDisplayId();
    if (screenId == SCREEN_ID_INVALID) {
        TLOGE(WmsLogTag::WMS_PIP, "invalid screenId");
        return;
    }

    int32_t windowId = parentSession->GetWindowId();
    TLOGI(WmsLogTag::WMS_PIP, "Notify MulScreen, type:%{public}d, %{public}d, screenId:%{public}" PRIu64,
        type, windowId, screenId);
    if (type != WindowType::WINDOW_TYPE_PIP || GetPipDeviceCollaborationPolicy(screenId)) {
        return;
    }

    std::shared_lock<std::shared_mutex> lock(pipChgListenerMapMutex_);
    auto iter = pipChgListenerMap_.find(screenId);
    if (iter != pipChgListenerMap_.end()) {
        if (iter->second != nullptr) {
            iter->second->OnPipStart(windowId);
        }
    }
}

WSError SceneSessionManager::IsFloatingBallValid(const sptr<SceneSession>& parentSession)
{
    if (parentSession == nullptr || parentSession->GetSessionState() == SessionState::STATE_DISCONNECT ||
        parentSession->GetSessionState() == SessionState::STATE_BACKGROUND) {
        TLOGE(WmsLogTag::WMS_LIFE, "Parent is null or state invalid");
        return WSError::WS_ERROR_INVALID_PARENT;
    }
    return WSError::WS_OK;
}

void SceneSessionManager::NotifyPiPWindowVisibleChange(bool screenLocked) {
    sptr<SceneSession> session = SelectSesssionFromMap(pipWindowSurfaceId_);
    if (session != nullptr) {
        std::vector<std::pair<uint64_t, WindowVisibilityState>> pipVisibilityChangeInfos;
        if (screenLocked) {
            pipVisibilityChangeInfos.emplace_back(pipWindowSurfaceId_, WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
        } else {
            pipVisibilityChangeInfos.emplace_back(pipWindowSurfaceId_, WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
        }
        DealwithVisibilityChange(pipVisibilityChangeInfos, lastVisibleData_);
    }
}

bool SceneSessionManager::IsLastPiPWindowVisible(uint64_t surfaceId, WindowVisibilityState lastVisibilityState) {
    sptr<SceneSession> session = SelectSesssionFromMap(surfaceId);
    if (session == nullptr || session->GetWindowMode() != WindowMode::WINDOW_MODE_PIP) {
        TLOGD(WmsLogTag::WMS_PIP, "session is null or windowMode is not PIP");
        return false;
    }
    if (isScreenLocked_) {
        TLOGD(WmsLogTag::WMS_PIP, "pipWindow occlusion because of screen locked");
        return false;
    }
    if (lastVisibilityState != WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
        // no visibility notification processing after pip is occlusion
        TLOGI(WmsLogTag::WMS_PIP, "pipWindow occlusion success. pipWindowSurfaceId: %{public}" PRIu64, surfaceId);
        pipWindowSurfaceId_ = surfaceId;
        return true;
    }
    return false;
}

bool SceneSessionManager::CheckModalSubWindowPermission(const sptr<WindowSessionProperty>& property)
{
    WindowType type = property->GetWindowType();
    if (!WindowHelper::IsSubWindow(type) || !property->IsTopmost()) {
        return true;
    }

    if (!WindowHelper::IsModalSubWindow(type, property->GetWindowFlags())) {
        TLOGE(WmsLogTag::WMS_SUB, "Normal subwindow has topmost");
        return false;
    }

    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_SUB, "Modal subwindow has topmost, but no system permission");
        return false;
    }

    return true;
}

bool SceneSessionManager::CheckSystemWindowPermission(const sptr<WindowSessionProperty>& property)
{
    WindowType type = property->GetWindowType();
    if (WindowHelper::IsUIExtensionWindow(type)) {
        // UIExtension window disallowed.
        return false;
    }
    if (!WindowHelper::IsSystemWindow(type)) {
        // type is not system
        return true;
    }
    if (type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT && property->IsSystemKeyboard()) {
        // system keyboard window can only be created by virtual keyboard service
        if (SessionPermission::VerifyCallingPermission("ohos.permission.VIRTUAL_KEYBOARD_WINDOW")) {
            TLOGD(WmsLogTag::WMS_KEYBOARD, "create system keyboard, permission check sucess.");
            return true;
        }
        TLOGE(WmsLogTag::WMS_KEYBOARD, "create system keyboard, permission check failed.");
        return false;
    }
    if ((type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT ||
         type == WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR) && SessionPermission::IsStartedByInputMethod()) {
        // WINDOW_TYPE_INPUT_METHOD_FLOAT could be created by input method app
        TLOGD(WmsLogTag::WMS_KEYBOARD, "check create permission success, input method app create input method window.");
        return true;
    }
    if (type == WindowType::WINDOW_TYPE_DIALOG || type == WindowType::WINDOW_TYPE_PIP) {
        // some system types could be created by normal app
        return true;
    }
    if (type == WindowType::WINDOW_TYPE_FB) {
        return SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_FLOATING_BALL);
    }
    if (type == WindowType::WINDOW_TYPE_FLOAT) {
        // WINDOW_TYPE_FLOAT could be created with the corresponding permission
        if (SessionPermission::VerifyCallingPermission("ohos.permission.SYSTEM_FLOAT_WINDOW") &&
            (SessionPermission::IsSystemCalling() || SessionPermission::IsStartByHdcd() ||
                systemConfig_.supportTypeFloatWindow_)) {
            TLOGI(WmsLogTag::WMS_SYSTEM, "check float permission success.");
            return true;
        } else if (SessionPermission::VerifyCallingPermission("ohos.permission.SYSTEM_FLOAT_WINDOW") &&
            systemConfig_.supportCreateFloatWindow_) {
            TLOGI(WmsLogTag::WMS_SYSTEM, "check float permission success. SupportCreateFloatWindow.");
            return true;
        } else {
            TLOGI(WmsLogTag::WMS_SYSTEM, "check float permission failed.");
            return false;
        }
    }
    if (type == WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW) {
        int32_t parentId = property->GetParentPersistentId();
        auto parentSession = GetSceneSession(parentId);
        if (parentSession != nullptr && parentSession->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT &&
            SessionPermission::VerifyCallingPermission("ohos.permission.SYSTEM_FLOAT_WINDOW")) {
            TLOGI(WmsLogTag::WMS_SYSTEM, "check system subWindow permission success, parentId:%{public}d.", parentId);
            return true;
        } else {
            TLOGW(WmsLogTag::WMS_SYSTEM, "check system subWindow permission warning, parentId:%{public}d.", parentId);
        }
    }
    if (SessionHelper::IsNeedSACalling(type)) {
        if (SessionPermission::IsSACalling()) {
            TLOGI(WmsLogTag::WMS_SYSTEM, "check sa permission success, create with SA calling.");
            return true;
        }
        TLOGE(WmsLogTag::WMS_SYSTEM, "check sa permission failed");
        return false;
    }
    if (SessionPermission::IsSystemCalling() || SessionPermission::IsStartByHdcd()) {
        TLOGI(WmsLogTag::WMS_SYSTEM, "check create permission success, create with system calling.");
        return true;
    }
    TLOGE(WmsLogTag::WMS_SYSTEM, "finally check permission failed.");
    return false;
}

void SceneSessionManager::RecoverSessionInfo(const sptr<WindowSessionProperty>& property)
{
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "property is nullptr");
        return;
    }
    SessionInfo& sessionInfo = property->EditSessionInfo();
    sessionInfo.persistentId_ = property->GetPersistentId();
    sessionInfo.windowMode = static_cast<int32_t>(property->GetWindowMode());
    sessionInfo.windowType_ = static_cast<uint32_t>(property->GetWindowType());
    sessionInfo.requestOrientation_ = static_cast<uint32_t>(property->GetRequestedOrientation());
    sessionInfo.specificSessionRequestOrientation_ = property->GetIsSpecificSessionRequestOrientation()
        ? static_cast<int32_t>(property->GetRequestedOrientation())
        : -1;
    sessionInfo.sessionState_ = (property->GetWindowState() == WindowState::STATE_SHOWN)
                                    ? SessionState::STATE_ACTIVE
                                    : SessionState::STATE_BACKGROUND;
    sessionInfo.isPersistentRecover_ = true;
    sessionInfo.appInstanceKey_ = property->GetAppInstanceKey();
    sessionInfo.appIndex_ = property->GetAppIndex();
    sessionInfo.screenId_ = property->GetDisplayId();
    sessionInfo.isAbilityHook_ = property->GetIsAbilityHook();
    sessionInfo.supportedWindowModes =
        WindowHelper::ConvertSupportTypeToSupportModes(property->GetWindowModeSupportType());
    TLOGI(WmsLogTag::WMS_RECOVER,
        "Recover and reconnect session with: bundleName=%{public}s, moduleName=%{public}s, "
        "abilityName=%{public}s, windowMode=%{public}d, windowType=%{public}u, persistentId=%{public}d, "
        "windowState=%{public}u, appInstanceKey=%{public}s, appIndex=%{public}d, "
        "isFollowParentMultiScreenPolicy=%{public}d, screenId=%{public}" PRIu64 ", isAbilityHook=%{public}d",
        sessionInfo.bundleName_.c_str(), sessionInfo.moduleName_.c_str(), sessionInfo.abilityName_.c_str(),
        sessionInfo.windowMode, sessionInfo.windowType_, sessionInfo.persistentId_, sessionInfo.sessionState_,
        sessionInfo.appInstanceKey_.c_str(), sessionInfo.appIndex_, sessionInfo.isFollowParentMultiScreenPolicy,
        sessionInfo.screenId_, sessionInfo.isAbilityHook_);
}

void SceneSessionManager::SetAlivePersistentIds(const std::vector<int32_t>& alivePersistentIds)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "Size of PersistentIds need to be recovered=%{public}zu, CurrentUserId=%{public}d",
        alivePersistentIds.size(), currentUserId_.load());
    alivePersistentIds_ = alivePersistentIds;
}

bool SceneSessionManager::IsNeedRecover(const int32_t persistentId)
{
    if (auto it = std::find(alivePersistentIds_.begin(), alivePersistentIds_.end(), persistentId);
        it == alivePersistentIds_.end()) {
        TLOGW(WmsLogTag::WMS_RECOVER, "recovered persistentId=%{public}d is not in alivePersistentIds_", persistentId);
        return false;
    }
    return true;
}

WSError SceneSessionManager::CheckSessionPropertyOnRecovery(const sptr<WindowSessionProperty>& property,
    bool isSpecificSession)
{
    if (property == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "property is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (!CheckSystemWindowPermission(property)) {
        TLOGE(WmsLogTag::WMS_RECOVER, "create system window permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (isSpecificSession) {
        if (property->GetParentPersistentId() > 0 && !IsNeedRecover(property->GetParentPersistentId())) {
            TLOGE(WmsLogTag::WMS_RECOVER, "no need to recover.");
            return WSError::WS_ERROR_INVALID_PARAM;
        }
    } else {
        if (property->GetPersistentId() > 0 && !IsNeedRecover(property->GetPersistentId())) {
            TLOGE(WmsLogTag::WMS_RECOVER, "no need to recover.");
            return WSError::WS_ERROR_INVALID_PARAM;
        }
    }
    return WSError::WS_OK;
}

WSError SceneSessionManager::RecoverAndConnectSpecificSession(const sptr<ISessionStage>& sessionStage,
    const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
    sptr<WindowSessionProperty> property, sptr<ISession>& session, sptr<IRemoteObject> token)
{
    auto propCheckRet = CheckSessionPropertyOnRecovery(property, true);
    if (propCheckRet != WSError::WS_OK) {
        return propCheckRet;
    }
    auto pid = IPCSkeleton::GetCallingRealPid();
    auto uid = IPCSkeleton::GetCallingUid();
    auto task = [this, sessionStage, eventChannel, surfaceNode, property, &session, token, pid, uid]() {
        if (recoveringFinished_ || sessionRecoverStateChangeFunc_ == nullptr) {
            TLOGNW(WmsLogTag::WMS_RECOVER, "Recover finished, not recovery anymore");
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        sessionRecoverStateChangeFunc_(SessionRecoverState::SESSION_START_RECONNECT, property);
        SessionInfo info = property->GetSessionInfo();
        TLOGNI(WmsLogTag::WMS_RECOVER, "callingWindowId=%{public}" PRIu32, property->GetCallingSessionId());
        ClosePipWindowIfExist(property->GetWindowType());
        sptr<SceneSession> sceneSession = RequestSceneSession(info, property);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_RECOVER, "RequestSceneSession failed");
            sessionRecoverStateChangeFunc_(SessionRecoverState::SESSION_NOT_RECONNECT, property);
            return WSError::WS_ERROR_NULLPTR;
        }

        auto persistentId = sceneSession->GetPersistentId();
        if (persistentId != info.persistentId_) {
            TLOGNE(WmsLogTag::WMS_RECOVER,
                "SpecificSession PersistentId changed, from %{public}d to %{public}d, parentPersistentId is %{public}d",
                info.persistentId_, persistentId, property->GetParentPersistentId());
            failRecoveredPersistentIdSet_.insert(property->GetParentPersistentId());
            EraseSceneSessionMapById(persistentId);
            sessionRecoverStateChangeFunc_(SessionRecoverState::SESSION_NOT_RECONNECT, property);
            return WSError::WS_ERROR_INVALID_SESSION;
        }

        auto errCode = sceneSession->Reconnect(sessionStage, eventChannel, surfaceNode, property, token, pid, uid);
        if (errCode != WSError::WS_OK) {
            TLOGNE(WmsLogTag::WMS_RECOVER, "SceneSession reconnect failed");
            EraseSceneSessionMapById(persistentId);
            sessionRecoverStateChangeFunc_(SessionRecoverState::SESSION_NOT_RECONNECT, property);
            return errCode;
        }
        sessionRecoverStateChangeFunc_(SessionRecoverState::SESSION_FINISH_RECONNECT, property);
        AddClientDeathRecipient(sessionStage, sceneSession);
        session = sceneSession;
        return errCode;
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

void SceneSessionManager::NotifyRecoveringFinished()
{
    taskScheduler_->PostAsyncTask([this]() {
        TLOGNI(WmsLogTag::WMS_RECOVER, "RecoverFinished clear recoverSubSessionCacheMap");
        recoveringFinished_ = true;
        recoverSubSessionCacheMap_.clear();
        recoverDialogSessionCacheMap_.clear();
    }, __func__);
}

void SceneSessionManager::CacheSpecificSessionForRecovering(
    const sptr<SceneSession>& sceneSession, const sptr<WindowSessionProperty>& property)
{
    if (!IsWindowSupportCacheForRecovering(sceneSession, property)) {
        return;
    }

    auto windowType = property->GetWindowType();
    auto parentId = property->GetParentPersistentId();
    TLOGI(WmsLogTag::WMS_RECOVER, "Cache specific session persistentId=%{public}d, parent persistentId="
        "%{public}d, window type=%{public}d", sceneSession->GetPersistentId(), parentId, windowType);

    if (WindowHelper::IsSubWindow(windowType)) {
        recoverSubSessionCacheMap_[parentId].emplace_back(sceneSession);
    } else if (WindowHelper::IsDialogWindow(windowType)) {
        recoverDialogSessionCacheMap_[parentId].emplace_back(sceneSession);
    }
}

bool SceneSessionManager::IsWindowSupportCacheForRecovering(
    const sptr<SceneSession>& sceneSession, const sptr<WindowSessionProperty>& property)
{
    if (recoveringFinished_) {
        TLOGW(WmsLogTag::WMS_RECOVER, "recovering is finished");
        return false;
    }

    if (sceneSession == nullptr || property == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "sceneSession or property is nullptr");
        return false;
    }

    auto windowType = property->GetWindowType();
    if (!WindowHelper::IsSubWindow(windowType) && !WindowHelper::IsDialogWindow(windowType)) {
        return false;
    }

    auto parentId = property->GetParentPersistentId();
    if ((WindowHelper::IsSubWindow(windowType) &&
         createSubSessionFuncMap_.find(parentId) != createSubSessionFuncMap_.end()) ||
        (WindowHelper::IsDialogWindow(windowType) &&
         bindDialogTargetFuncMap_.find(parentId) != bindDialogTargetFuncMap_.end())) {
        return false;
    }

    return true;
}

void SceneSessionManager::RecoverCachedSubSession(int32_t persistentId)
{
    auto iter = recoverSubSessionCacheMap_.find(persistentId);
    if (iter == recoverSubSessionCacheMap_.end()) {
        return;
    }
    TLOGI(WmsLogTag::WMS_RECOVER, "Id=%{public}d", persistentId);
    for (auto& sceneSession : iter->second) {
        NotifyCreateSubSession(persistentId, sceneSession);
    }
    recoverSubSessionCacheMap_.erase(iter);
}

void SceneSessionManager::RecoverCachedDialogSession(int32_t persistentId)
{
    auto iter = recoverDialogSessionCacheMap_.find(persistentId);
    if (iter == recoverDialogSessionCacheMap_.end()) {
        return;
    }
    TLOGI(WmsLogTag::WMS_RECOVER, "Id=%{public}d", persistentId);
    for (auto& sceneSession : iter->second) {
        UpdateParentSessionForDialog(sceneSession, sceneSession->GetSessionProperty());
    }
    recoverDialogSessionCacheMap_.erase(iter);
}

void SceneSessionManager::NotifySessionUnfocusedToClient(int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "Id=%{public}d", persistentId);
    listenerController_->NotifySessionUnfocused(persistentId);
}

WSError SceneSessionManager::RecoverAndReconnectSceneSession(const sptr<ISessionStage>& sessionStage,
    const sptr<IWindowEventChannel>& eventChannel, const std::shared_ptr<RSSurfaceNode>& surfaceNode,
    sptr<ISession>& session, sptr<WindowSessionProperty> property, sptr<IRemoteObject> token)
{
    auto result = CheckSessionPropertyOnRecovery(property, false);
    if (result != WSError::WS_OK) {
        return result;
    }
    auto pid = IPCSkeleton::GetCallingRealPid();
    auto uid = IPCSkeleton::GetCallingUid();
    auto task = [this, sessionStage, eventChannel, surfaceNode, &session, property, token, pid, uid]() {
        if (recoveringFinished_ || sessionRecoverStateChangeFunc_ == nullptr) {
            TLOGNW(WmsLogTag::WMS_RECOVER, "Recover finished, not recovery anymore");
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        if (recoverSceneSessionFunc_ == nullptr) {
            TLOGNE(WmsLogTag::WMS_RECOVER, "recoverSceneSessionFunc_ is null");
            return WSError::WS_ERROR_NULLPTR;
        }
        sessionRecoverStateChangeFunc_(SessionRecoverState::SESSION_START_RECONNECT, property);
        SessionInfo sessionInfo = property->GetSessionInfo();
        sptr<SceneSession> sceneSession = RequestSceneSession(sessionInfo, nullptr);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_RECOVER, "Request sceneSession failed");
            sessionRecoverStateChangeFunc_(SessionRecoverState::SESSION_NOT_RECONNECT, property);
            return WSError::WS_ERROR_NULLPTR;
        }
        int32_t persistentId = sceneSession->GetPersistentId();
        if (persistentId != sessionInfo.persistentId_) {
            TLOGNE(WmsLogTag::WMS_RECOVER, "SceneSession PersistentId changed, from %{public}d to %{public}d",
                sessionInfo.persistentId_, persistentId);
            EraseSceneSessionMapById(persistentId);
            sessionRecoverStateChangeFunc_(SessionRecoverState::SESSION_NOT_RECONNECT, property);
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        auto ret = sceneSession->Reconnect(sessionStage, eventChannel, surfaceNode, property, token, pid, uid);
        if (ret != WSError::WS_OK) {
            TLOGNE(WmsLogTag::WMS_RECOVER, "Reconnect failed");
            EraseSceneSessionMapById(sessionInfo.persistentId_);
            sessionRecoverStateChangeFunc_(SessionRecoverState::SESSION_NOT_RECONNECT, property);
            return ret;
        }
        sceneSession->NotifyIsFullScreenInForceSplitMode(property->IsFullScreenInForceSplitMode());
        sessionRecoverStateChangeFunc_(SessionRecoverState::SESSION_FINISH_RECONNECT, property);
        session = sceneSession;
        return WSError::WS_OK;
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

void SceneSessionManager::UpdateRecoverPropertyForSuperFold(const sptr<WindowSessionProperty>& property)
{
    if (property->GetDisplayId() != VIRTUAL_DISPLAY_ID) {
        return;
    }
    static auto foldCreaseRegion = SingletonContainer::Get<DisplayManager>().GetCurrentFoldCreaseRegion();
    if (foldCreaseRegion == nullptr) {
        return;
    }
    Rect recoverWindowRect = property->GetWindowRect();
    Rect recoverRequestRect = property->GetRequestRect();
    TLOGD(WmsLogTag::WMS_RECOVER,
        "WindowRect: %{public}s, RequestRect: %{public}s, DisplayId: %{public}d",
        recoverWindowRect.ToString().c_str(), recoverRequestRect.ToString().c_str(),
        static_cast<uint32_t>(property->GetDisplayId()));

    auto foldCrease = foldCreaseRegion->GetCreaseRects().front();
    recoverWindowRect.posY_ += foldCrease.posY_ + static_cast<int32_t>(foldCrease.height_);
    recoverRequestRect.posY_ += foldCrease.posY_ + static_cast<int32_t>(foldCrease.height_);
    property->SetWindowRect(recoverWindowRect);
    property->SetRequestRect(recoverRequestRect);
    property->SetDisplayId(0);
    TLOGD(WmsLogTag::WMS_RECOVER,
        "WindowRect: %{public}s, RequestRect: %{public}s, DisplayId: %{public}d",
        recoverWindowRect.ToString().c_str(), recoverRequestRect.ToString().c_str(),
        static_cast<uint32_t>(property->GetDisplayId()));
}

void SceneSessionManager::SetRecoverSceneSessionListener(const NotifyRecoverSceneSessionFunc& func)
{
    TLOGI(WmsLogTag::WMS_RECOVER, "in");
    recoverSceneSessionFunc_ = func;
}

void SceneSessionManager::SetCreateSystemSessionListener(const NotifyCreateSystemSessionFunc& func)
{
    createSystemSessionFunc_ = func;
}

void SceneSessionManager::SetSpecificWindowZIndexListener(const NotifySetSpecificWindowZIndexFunc& func)
{
    setSpecificWindowZIndexFunc_ = func;
}

void SceneSessionManager::SetCreateKeyboardSessionListener(const NotifyCreateKeyboardSessionFunc& func)
{
    createKeyboardSessionFunc_ = func;
}

void SceneSessionManager::SetStartPiPFailedListener(NotifyStartPiPFailedFunc&& func)
{
    startPiPFailedFunc_ = std::move(func);
}

void SceneSessionManager::SetSupportRotationRegisteredListener(NotifySupportRotationRegisteredFunc&& func)
{
    supportRotationRegisteredListener_ = std::move(func);
}

void SceneSessionManager::RegisterCreateSubSessionListener(int32_t persistentId,
    const NotifyCreateSubSessionFunc& func)
{
    TLOGI(WmsLogTag::WMS_SUB, "id: %{public}d", persistentId);
    taskScheduler_->PostSyncTask([this, persistentId, func]() {
        createSubSessionFuncMap_[persistentId] = func;
        RecoverCachedSubSession(persistentId);
        return WMError::WM_OK;
    }, __func__);
}

void SceneSessionManager::RegisterBindDialogTargetListener(const sptr<SceneSession>& session,
    NotifyBindDialogSessionFunc&& func)
{
    int32_t persistentId = session->GetPersistentId();
    TLOGI(WmsLogTag::WMS_DIALOG, "Id: %{public}d", persistentId);
    taskScheduler_->PostTask([this, session, persistentId, func = std::move(func)] {
        session->RegisterBindDialogSessionCallback(func);
        bindDialogTargetFuncMap_[persistentId] = std::move(func);
        RecoverCachedDialogSession(persistentId);
    }, __func__);
}

void SceneSessionManager::SetFocusedSessionDisplayIdIfNeeded(sptr<SceneSession>& newSession)
{
    if (newSession->GetSessionProperty()->GetDisplayId() == DISPLAY_ID_INVALID) {
        uint64_t defaultDisplayId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
        int32_t focusSessionId = windowFocusController_->GetFocusedSessionId(defaultDisplayId);
        auto focusedSession = GetSceneSession(focusSessionId);
        DisplayId displayId = defaultDisplayId;
        if (focusedSession != nullptr && focusedSession->GetSessionProperty()->GetDisplayId() != DISPLAY_ID_INVALID) {
            displayId = focusedSession->GetSessionProperty()->GetDisplayId();
            TLOGI(WmsLogTag::WMS_ATTRIBUTE, "id: %{public}d, focus id %{public}d, display id: %{public}" PRIu64,
                newSession->GetPersistentId(), focusedSession->GetPersistentId(), displayId);
        } else {
            TLOGE(WmsLogTag::WMS_ATTRIBUTE, "get focus id failed, use default displayId %{public}" PRIu64, displayId);
        }
        newSession->SetScreenId(displayId);
        newSession->GetSessionProperty()->SetDisplayId(displayId);
    }
}

void SceneSessionManager::NotifyCreateSpecificSession(sptr<SceneSession> newSession,
    sptr<WindowSessionProperty> property, const WindowType& type)
{
    if (newSession == nullptr || property == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "newSession or property is nullptr");
        return;
    }
    if (SessionHelper::IsSystemWindow(type)) {
        if (type == WindowType::WINDOW_TYPE_FLOAT) {
            auto parentSession = GetSceneSession(property->GetParentPersistentId());
            if (parentSession != nullptr) {
                newSession->SetParentSession(parentSession);
            }
        }
        if (type == WindowType::WINDOW_TYPE_TOAST) {
            NotifyCreateToastSession(property->GetParentPersistentId(), newSession);
        }
        if (type != WindowType::WINDOW_TYPE_DIALOG) {
            if (WindowHelper::IsSystemSubWindow(type)) {
                NotifyCreateSubSession(property->GetParentPersistentId(), newSession);
            } else if (isKeyboardPanelEnabled_ && type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT
                && createKeyboardSessionFunc_) {
                createKeyboardSessionFunc_(newSession, newSession->GetKeyboardPanelSession());
            } else if (createSystemSessionFunc_) {
                SetFocusedSessionDisplayIdIfNeeded(newSession);
                property->SetDisplayId(newSession->GetSessionProperty()->GetDisplayId());
                createSystemSessionFunc_(newSession);
            }
            TLOGD(WmsLogTag::WMS_LIFE, "Create system session, id:%{public}d, type: %{public}d",
                newSession->GetPersistentId(), type);
        } else {
            TLOGW(WmsLogTag::WMS_LIFE, "Didn't create jsSceneSession for this system type, id:%{public}d, "
                "type:%{public}d", newSession->GetPersistentId(), type);
            return;
        }
    } else if (SessionHelper::IsSubWindow(type)) {
        NotifyCreateSubSession(property->GetParentPersistentId(), newSession);
        TLOGD(WmsLogTag::WMS_LIFE, "Notify sub jsSceneSession, id:%{public}d, parentId:%{public}d, type:%{public}d",
            newSession->GetPersistentId(), property->GetParentPersistentId(), type);
    } else {
        TLOGW(WmsLogTag::WMS_LIFE, "Invalid session type, id:%{public}d, type:%{public}d",
            newSession->GetPersistentId(), type);
    }
}

void SceneSessionManager::NotifyCreateSubSession(int32_t persistentId, sptr<SceneSession> session, uint32_t windowFlags)
{
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "SubSession is nullptr");
        return;
    }
    auto iter = createSubSessionFuncMap_.find(persistentId);
    if (iter == createSubSessionFuncMap_.end()) {
        recoverSubSessionCacheMap_[persistentId].push_back(session);
        TLOGW(WmsLogTag::WMS_LIFE, "Can't find CreateSubSessionListener, parentId: %{public}d", persistentId);
        return;
    }
    const auto& createSubSessionFunc = iter->second;
    sptr<SceneSession> parentSession = nullptr;
    if (windowFlags & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_TOAST)) {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        parentSession = GetMainParentSceneSession(persistentId, sceneSessionMap_);
    } else {
        parentSession = GetSceneSession(persistentId);
    }
    if (parentSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Can't find CreateSubSessionListener, parentId: %{public}d, subId: %{public}d",
            persistentId, session->GetPersistentId());
        return;
    }
    parentSession->AddSubSession(session);
    session->SetParentSession(parentSession);
    if (createSubSessionFunc) {
        createSubSessionFunc(session);
    }
    TLOGD(WmsLogTag::WMS_LIFE, "Notify success, parentId: %{public}d, subId: %{public}d",
        persistentId, session->GetPersistentId());
}

sptr<SceneSession> SceneSessionManager::GetMainParentSceneSession(int32_t persistentId,
    const std::map<int32_t, sptr<SceneSession>>& sessionMap)
{
    if (persistentId == INVALID_SESSION_ID) {
        TLOGW(WmsLogTag::WMS_LIFE, "invalid persistentId id");
        return nullptr;
    }
    auto iter = sessionMap.find(persistentId);
    if (iter == sessionMap.end()) {
        TLOGD(WmsLogTag::WMS_LIFE, "Error found scene session with id: %{public}d", persistentId);
        return nullptr;
    }
    sptr<SceneSession> parentSession = iter->second;
    if (parentSession == nullptr) {
        TLOGW(WmsLogTag::WMS_LIFE, "not find parent session");
        return nullptr;
    }
    bool isNoParentSystemSession = WindowHelper::IsSystemWindow(parentSession->GetWindowType()) &&
        parentSession->GetParentPersistentId() == INVALID_SESSION_ID;
    if (WindowHelper::IsMainWindow(parentSession->GetWindowType()) || isNoParentSystemSession) {
        TLOGD(WmsLogTag::WMS_LIFE, "find main session, id:%{public}u", persistentId);
        return parentSession;
    }
    return GetMainParentSceneSession(parentSession->GetParentPersistentId(), sessionMap);
}

void SceneSessionManager::NotifyCreateToastSession(int32_t persistentId, sptr<SceneSession> session)
{
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "toastSession is nullptr");
        return;
    }

    auto parentSession = GetSceneSession(persistentId);
    if (parentSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Can't find parentSession, parentId: %{public}d, ToastId: %{public}d",
            persistentId, session->GetPersistentId());
        return;
    }
    parentSession->AddToastSession(session);
    session->SetParentSession(parentSession);
    TLOGD(WmsLogTag::WMS_LIFE, "Notify success, parentId: %{public}d, toastId: %{public}d",
        persistentId, session->GetPersistentId());
}

void SceneSessionManager::UnregisterSpecificSessionCreateListener(int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_SUB, "id: %{public}d", persistentId);
    taskScheduler_->PostSyncTask([this, persistentId]() {
        if (createSubSessionFuncMap_.find(persistentId) != createSubSessionFuncMap_.end()) {
            createSubSessionFuncMap_.erase(persistentId);
        }
        if (bindDialogTargetFuncMap_.find(persistentId) != bindDialogTargetFuncMap_.end()) {
            bindDialogTargetFuncMap_.erase(persistentId);
        }
        return WMError::WM_OK;
    }, __func__);
}

void SceneSessionManager::SetStatusBarEnabledChangeListener(const ProcessStatusBarEnabledChangeFunc& func)
{
    if (!func) {
        TLOGD(WmsLogTag::WMS_EVENT, "set func is null");
    }
    statusBarEnabledChangeFunc_ = func;
}

void SceneSessionManager::SetGestureNavigationEnabledChangeListener(
    const ProcessGestureNavigationEnabledChangeFunc& func)
{
    if (!func) {
        TLOGD(WmsLogTag::WMS_EVENT, "set func is null");
    }
    gestureNavigationEnabledChangeFunc_ = func;
}

void SceneSessionManager::OnOutsideDownEvent(int32_t x, int32_t y)
{
    TLOGD(WmsLogTag::WMS_EVENT, "x=%{private}d, y=%{private}d", x, y);
    if (outsideDownEventFunc_) {
        outsideDownEventFunc_(x, y);
    }
}

WSError SceneSessionManager::GetDisplayGroupIdFromSceneSession(const sptr<SceneSession>& session,
    DisplayId& displayGroupId) const
{
    if (session == nullptr) {
        return WSError::WS_ERROR_NULLPTR;
    }
    displayGroupId = GetDisplayGroupId(session->GetDisplayId());
    return WSError::WS_OK;
}

bool SceneSessionManager::IsSameDisplayGroupId(const sptr<SceneSession>& session,
    const DisplayId touchDisplayGroupId) const
{
    DisplayId displayGroupId = 0;
    if (GetDisplayGroupIdFromSceneSession(session, displayGroupId) != WSError::WS_OK) {
        TLOGD(WmsLogTag::WMS_EVENT, "session is null.");
        return false;
    }
    return displayGroupId == touchDisplayGroupId;
}

void SceneSessionManager::NotifySessionTouchOutside(int32_t persistentId, DisplayId displayId)
{
    auto task = [this, persistentId, displayId]() {
        int32_t callingSessionId = INVALID_WINDOW_ID;
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession != nullptr && sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) {
            callingSessionId = static_cast<int32_t>(sceneSession->GetCallingSessionId());
            TLOGND(WmsLogTag::WMS_KEYBOARD, "persistentId: %{public}d, callingSessionId: %{public}d",
                persistentId, callingSessionId);
        }
        DisplayId touchDisplayGroupId = GetDisplayGroupId(displayId);
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                continue;
            }
            if (!(sceneSession->IsVisible() ||
                  sceneSession->GetSessionState() == SessionState::STATE_FOREGROUND ||
                  sceneSession->GetSessionState() == SessionState::STATE_ACTIVE)) {
                continue;
            }
            if (!IsSameDisplayGroupId(sceneSession, touchDisplayGroupId)) {
                continue;
            }
            auto sessionId = sceneSession->GetPersistentId();
            if (!sceneSession->CheckTouchOutsideCallbackRegistered() &&
                (touchOutsideListenerSessionSet_.find(sessionId) == touchOutsideListenerSessionSet_.end())) {
                TLOGND(WmsLogTag::WMS_KEYBOARD, "id: %{public}d is not in touchOutsideListenerNodes, don't notify.",
                    sessionId);
                continue;
            }
            if (sessionId == callingSessionId || sessionId == persistentId) {
                TLOGND(WmsLogTag::WMS_KEYBOARD, "No need to notify touch window, id: %{public}d", sessionId);
                continue;
            }
            sceneSession->NotifyTouchOutside();
        }
    };

    taskScheduler_->PostAsyncTask(task, "NotifySessionTouchOutside:PID" + std::to_string(persistentId));
    return;
}

void SceneSessionManager::SetOutsideDownEventListener(const ProcessOutsideDownEventFunc& func)
{
    TLOGD(WmsLogTag::WMS_EVENT, "in");
    outsideDownEventFunc_ = func;
}

WMError SceneSessionManager::NotifyWatchGestureConsumeResult(int32_t keyCode, bool isConsumed)
{
    TLOGD(WmsLogTag::WMS_EVENT, "isConsumed:%{public}d", isConsumed);
    if (onWatchGestureConsumeResultFunc_) {
        onWatchGestureConsumeResultFunc_(keyCode, isConsumed);
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "onWatchGestureConsumeResultFunc is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    return WMError::WM_OK;
}

void SceneSessionManager::RegisterWatchGestureConsumeResultCallback(NotifyWatchGestureConsumeResultFunc&& func)
{
    TLOGD(WmsLogTag::WMS_EVENT, "in");
    onWatchGestureConsumeResultFunc_ = std::move(func);
}

WMError SceneSessionManager::NotifyWatchFocusActiveChange(bool isActive)
{
    TLOGD(WmsLogTag::WMS_EVENT, "isActive:%{public}d", isActive);
    if (onWatchFocusActiveChangeFunc_) {
        onWatchFocusActiveChangeFunc_(isActive);
    } else {
        TLOGE(WmsLogTag::WMS_EVENT, "onWatchFocusActiveChangeFunc is null");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    return WMError::WM_OK;
}

void SceneSessionManager::RegisterWatchFocusActiveChangeCallback(NotifyWatchFocusActiveChangeFunc&& func)
{
    TLOGD(WmsLogTag::WMS_EVENT, "in");
    onWatchFocusActiveChangeFunc_ = std::move(func);
}

void SceneSessionManager::ClearSpecificSessionRemoteObjectMap(int32_t persistentId)
{
    for (auto iter = remoteObjectMap_.begin(); iter != remoteObjectMap_.end(); ++iter) {
        if (iter->second != persistentId) {
            continue;
        }
        if (iter->first == nullptr || !iter->first->RemoveDeathRecipient(windowDeath_)) {
            TLOGE(WmsLogTag::WMS_LIFE, "failed to remove death recipient");
        }
        remoteObjectMap_.erase(iter);
        break;
    }
}

WSError SceneSessionManager::DestroyAndDisconnectSpecificSessionInner(const int32_t persistentId)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        return WSError::WS_ERROR_NULLPTR;
    }
    auto ret = sceneSession->UpdateActiveStatus(false);
    RemoveSessionFromBlackList(sceneSession);
    WindowDestroyNotifyVisibility(sceneSession);
    auto windowType = sceneSession->GetWindowType();
    if (windowType == WindowType::WINDOW_TYPE_DIALOG) {
        auto parentSession = GetSceneSession(sceneSession->GetParentPersistentId());
        if (parentSession == nullptr) {
            TLOGE(WmsLogTag::WMS_DIALOG, "Dialog not bind parent");
        } else {
            parentSession->RemoveDialogToParentSession(sceneSession);
            parentSession->UnregisterNotifySurfaceBoundsChangeFunc(persistentId);
        }
    } else if (windowType == WindowType::WINDOW_TYPE_TOAST) {
        auto parentSession = GetSceneSession(sceneSession->GetParentPersistentId());
        if (parentSession != nullptr) {
            TLOGD(WmsLogTag::WMS_TOAST, "Find parentSession, id: %{public}d", persistentId);
            parentSession->RemoveToastSession(persistentId);
        } else {
            TLOGW(WmsLogTag::WMS_TOAST, "ParentSession is nullptr, id: %{public}d", persistentId);
        }
    } else if (windowType == WindowType::WINDOW_TYPE_FLOAT) {
        DestroySubSession(sceneSession);
    }
    ret = sceneSession->Disconnect();
    sceneSession->ClearSpecificSessionCbMap();
    if (SessionHelper::IsSubWindow(sceneSession->GetWindowType())) {
        DestroySubSession(sceneSession);
        auto parentSession = GetSceneSession(sceneSession->GetParentPersistentId());
        if (parentSession != nullptr) {
            TLOGD(WmsLogTag::WMS_SUB, "Find parentSession, id: %{public}d", persistentId);
            parentSession->RemoveSubSession(sceneSession->GetPersistentId());
            parentSession->UnregisterNotifySurfaceBoundsChangeFunc(persistentId);
        } else {
            TLOGW(WmsLogTag::WMS_SUB, "ParentSession is nullptr, id: %{public}d", persistentId);
        }
        DestroyUIServiceExtensionSubWindow(sceneSession);
    }
    {
        std::unique_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        EraseSceneSessionAndMarkDirtyLocked(persistentId);
        systemTopSceneSessionMap_.erase(persistentId);
        nonSystemFloatSceneSessionMap_.erase(persistentId);
        UnregisterSpecificSessionCreateListener(persistentId);
    }
    ClearSpecificSessionRemoteObjectMap(persistentId);
    TLOGI(WmsLogTag::WMS_LIFE, "Destroy specific session end, id: %{public}d", persistentId);
    return ret;
}

WSError SceneSessionManager::DestroyAndDisconnectSpecificSession(const int32_t persistentId)
{
    const auto& callingPid = IPCSkeleton::GetCallingRealPid();
    auto task = [this, persistentId, callingPid]() {
        TLOGNI(WmsLogTag::WMS_LIFE, "Destroy specific session start, id: %{public}d", persistentId);
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is nullptr, persistentId:%{public}d", persistentId);
            return WSError::WS_ERROR_NULLPTR;
        }

        if (callingPid != sceneSession->GetCallingPid()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "Permission denied, not destroy by the same process");
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        return DestroyAndDisconnectSpecificSessionInner(persistentId);
    };

    return taskScheduler_->PostSyncTask(task, "DestroyAndDisConnect:PID:" + std::to_string(persistentId));
}

WSError SceneSessionManager::DestroyAndDisconnectSpecificSessionWithDetachCallback(const int32_t persistentId,
    const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        return WSError::WS_ERROR_NULLPTR;
    }
    const auto callingPid = IPCSkeleton::GetCallingRealPid();
    auto task = [this, persistentId, callingPid, callback]() {
        TLOGNI(WmsLogTag::WMS_LIFE, "Destroy specific session start, id: %{public}d", persistentId);
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is nullptr, persistentId:%{public}d", persistentId);
            return WSError::WS_ERROR_NULLPTR;
        }

        if (callingPid != sceneSession->GetCallingPid()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "Permission denied, not destroy by the same process");
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        sceneSession->RegisterDetachCallback(iface_cast<IPatternDetachCallback>(callback));
        return DestroyAndDisconnectSpecificSessionInner(persistentId);
    };

    return taskScheduler_->PostSyncTask(task, "DestroyAndDisConnect:PID:" + std::to_string(persistentId));
}

void SceneSessionManager::DestroyUIServiceExtensionSubWindow(const sptr<SceneSession>& sceneSession)
{
    if (!sceneSession) {
        TLOGE(WmsLogTag::WMS_SUB, "sceneSession is null");
        return;
    }
    auto sessionProperty = sceneSession->GetSessionProperty();
    if (sessionProperty->GetIsUIExtFirstSubWindow() &&
        !sessionProperty->GetIsUIExtensionAbilityProcess()) {
        sceneSession->NotifyDestroy();
        int32_t errCode = AAFwk::AbilityManagerClient::GetInstance()->
            TerminateUIServiceExtensionAbility(sceneSession->GetAbilityToken());
        TLOGI(WmsLogTag::WMS_SUB,"TerminateUIServiceExtensionAbility id:%{public}d errCode:%{public}d",
            sceneSession->GetPersistentId(), errCode);
    }
}

const AppWindowSceneConfig& SceneSessionManager::GetWindowSceneConfig() const
{
    return appWindowSceneConfig_;
}

void SceneSessionManager::UpdateRotateAnimationConfig(const RotateAnimationConfig& config)
{
    taskScheduler_->PostAsyncTask([this, config] {
        TLOGNI(WmsLogTag::DEFAULT, "update rotate animation config duration: %{public}d", config.duration_);
        rotateAnimationConfig_.duration_ = config.duration_;
    }, __func__);
}

WSError SceneSessionManager::ProcessBackEvent()
{
    taskScheduler_->PostAsyncTask([this]() {
        auto focusGroup = windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
        if (focusGroup == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "focus group is nullptr: %{public}" PRIu64, DEFAULT_DISPLAY_ID);
            return WSError::WS_ERROR_NULLPTR;
        }
        auto focusedSessionId = focusGroup->GetFocusedSessionId();
        auto needBlockNotifyFocusStatusUntilForeground = focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground();
        auto session = GetSceneSession(focusedSessionId);
        if (!session) {
            TLOGNE(WmsLogTag::WMS_MAIN, "session is nullptr: %{public}d", focusedSessionId);
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        TLOGNI(WmsLogTag::WMS_MAIN, "ProcessBackEvent session persistentId:%{public}d needBlock:%{public}d",
            focusedSessionId, needBlockNotifyFocusStatusUntilForeground);
        if (needBlockNotifyFocusStatusUntilForeground ||
            (!session->IsSessionValid() && !session->IsSystemSession())) {
            TLOGND(WmsLogTag::WMS_MAIN, "RequestSessionBack when start session");
            if (session->GetSessionInfo().abilityInfo != nullptr &&
                session->GetSessionInfo().abilityInfo->unclearableMission) {
                TLOGNI(WmsLogTag::WMS_MAIN, "backPress unclearableMission");
                return WSError::WS_OK;
            }
            session->RequestSessionBack(true);
            return WSError::WS_OK;
        }
        if (session->GetSessionInfo().isSystem_ && rootSceneProcessBackEventFunc_) {
            rootSceneProcessBackEventFunc_();
        } else {
            session->ProcessBackEvent();
        }
        return WSError::WS_OK;
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSessionManager::InitUserInfo(int32_t userId, std::string& fileDir)
{
    if (userId == DEFAULT_USERID || fileDir.empty()) {
        TLOGE(WmsLogTag::WMS_MAIN, "params invalid");
        return WSError::WS_DO_NOTHING;
    }
    TLOGI(WmsLogTag::WMS_MAIN, "userId: %{public}d, path: %{public}s", userId, fileDir.c_str());
    InitStartingWindowRdb(fileDir + "/StartingWindowRdb/");
    return taskScheduler_->PostSyncTask([this, userId, &fileDir]() {
        if (!ScenePersistence::CreateSnapshotDir(fileDir)) {
            TLOGND(WmsLogTag::WMS_MAIN, "Create snapshot directory failed");
        }
        if (!ScenePersistence::CreateUpdatedIconDir(fileDir)) {
            TLOGND(WmsLogTag::WMS_MAIN, "Create icon directory failed");
        }
        currentUserId_ = userId;
        SceneInputManager::GetInstance().SetCurrentUserId(currentUserId_);
        if (MultiInstanceManager::IsSupportMultiInstance(systemConfig_)) {
            MultiInstanceManager::GetInstance().SetCurrentUserId(currentUserId_);
        }
        AbilityInfoManager::GetInstance().SetCurrentUserId(currentUserId_);
        RegisterRootSceneSession();
        RegisterSecSurfaceInfoListener();
        RegisterConstrainedModalUIExtInfoListener();
        InitStartingWindowRdb(fileDir + "/StartingWindowRdb/");
        RegisterWindowStateErrorCallbackToMMI();
        return WSError::WS_OK;
    }, __func__);
}

bool SceneSessionManager::IsNeedChangeLifeCycleOnUserSwitch(const sptr<SceneSession>& sceneSession, int32_t pid)
{
    auto sessionState = sceneSession->GetSessionState();
    auto isInvalidMainSession = !WindowHelper::IsMainWindow(sceneSession->GetWindowType()) ||
                                sessionState == SessionState::STATE_DISCONNECT ||
                                sessionState == SessionState::STATE_END;
    if (isInvalidMainSession) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "persistentId: %{public}d, type: %{public}d, state: %{public}d",
            sceneSession->GetPersistentId(), sceneSession->GetWindowType(), sceneSession->GetSessionState());
    }
    return sceneSession->GetCallingPid() != pid && IsPcSceneSessionLifecycle(sceneSession) && !isInvalidMainSession;
}

WSError SceneSessionManager::StartOrMinimizeUIAbilityBySCB(const sptr<SceneSession>& sceneSession, bool isUserActive)
{
    auto persistentId = sceneSession->GetPersistentId();
    auto abilitySessionInfo = SetAbilitySessionInfo(sceneSession);
    if (!isUserActive) {
        TLOGI(WmsLogTag::WMS_MULTI_USER,
            "MinimizeUIAbilityBySCB with persistentId: %{public}d, type: %{public}d, state: %{public}d", persistentId,
            sceneSession->GetWindowType(), sceneSession->GetSessionState());
        bool isFromUser = false;
        TLOGNI(WmsLogTag::WMS_LIFE, "MinimizeUIAbilityBySCB, Notify scene session id: %{public}d paused", persistentId);
        sceneSession->UpdateLifecyclePausedInner();
        int32_t errCode = AAFwk::AbilityManagerClient::GetInstance()->MinimizeUIAbilityBySCB(
            abilitySessionInfo, isFromUser, static_cast<uint32_t>(WindowStateChangeReason::USER_SWITCH));
        if (errCode == ERR_OK) {
            sceneSession->SetMinimizedFlagByUserSwitch(true);
        } else {
            TLOGE(WmsLogTag::WMS_MULTI_USER, "minimize failed! errCode: %{public}d", errCode);
        }
    } else if (sceneSession->IsMinimizedByUserSwitch()) {
        TLOGI(WmsLogTag::WMS_MULTI_USER,
            "StartUIAbilityBySCB with persistentId: %{public}d, type: %{public}d, state: %{public}d", persistentId,
            sceneSession->GetWindowType(), sceneSession->GetSessionState());
        sceneSession->SetMinimizedFlagByUserSwitch(false);
        bool isColdStart = false;
        abilitySessionInfo->isNewWant = false;
        int32_t errCode = StartUIAbilityBySCBTimeoutCheck(sceneSession,
            abilitySessionInfo, static_cast<uint32_t>(WindowStateChangeReason::USER_SWITCH), isColdStart);
        if (errCode != ERR_OK) {
            TLOGE(WmsLogTag::WMS_MULTI_USER, "start failed! errCode: %{public}d", errCode);
            ExceptionInfo exceptionInfo;
            exceptionInfo.needRemoveSession = true;
            sceneSession->NotifySessionExceptionInner(abilitySessionInfo, exceptionInfo, false, true);
            if (startUIAbilityErrorFunc_ && static_cast<WSError>(errCode) == WSError::WS_ERROR_EDM_CONTROLLED) {
                startUIAbilityErrorFunc_(
                    static_cast<uint32_t>(WS_JS_TO_ERROR_CODE_MAP.at(WSError::WS_ERROR_EDM_CONTROLLED)));
            }
        }
    }
    return WSError::WS_OK;
}

void SceneSessionManager::ProcessUIAbilityOnUserSwitch(bool isUserActive)
{
    int32_t pid = GetPid();
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession == nullptr) {
            TLOGE(WmsLogTag::WMS_MULTI_USER, "session is null");
            continue;
        }
        // Change app life cycle in pc when user switch, do app freeze or unfreeze
        if (IsNeedChangeLifeCycleOnUserSwitch(sceneSession, pid)) {
            StartOrMinimizeUIAbilityBySCB(sceneSession, isUserActive);
        }
    }
}

FlushWindowInfoTask SceneSessionManager::CreateDelayedFlushWindowInfoToMMITask() {
    FlushWindowInfoTask task = [this]() {
        if (isUserBackground_) {
            isDelayFlushWindowInfoMode_ = false;
            flushWindowInfoCount_ = 0;
            TLOGNI(WmsLogTag::WMS_EVENT, "DelayedFlushWindowInfoToMMI interrupt");
            return;
        }
        flushWindowInfoCount_.fetch_add(1, std::memory_order_relaxed);
        TLOGNI(WmsLogTag::WMS_EVENT, "DelayedFlushWindowInfoToMMI count: %{public}d", flushWindowInfoCount_.load());
        FlushWindowInfoToMMI(true);

        if (isDelayFlushWindowInfoMode_) {
            if (flushWindowInfoCount_ >= FLUSH_WINDOW_INFO_MAX_COUNT) {
                isDelayFlushWindowInfoMode_ = false;
                flushWindowInfoCount_ = 0;
                return;
            }
        }
        TLOGNI(WmsLogTag::WMS_EVENT, "DelayedFlushWindowInfoToMMI add task");
        auto nextTask = CreateDelayedFlushWindowInfoToMMITask();
        PostFlushWindowInfoTask(std::move(nextTask), FLUSH_WINDOW_INFO_TASK_NAME, FLUSH_WINDOW_INFO_DELAY_INTERVAL);
    };
    return task;
}

void SceneSessionManager::StartDelayedFlushWindowInfoToMMITask() {
    flushWindowInfoCount_ = 0;
    if (isDelayFlushWindowInfoMode_) {
        TLOGNI(WmsLogTag::WMS_EVENT, "DelayedFlushWindowInfoToMMI duplicate task");
        return;
    }
    isDelayFlushWindowInfoMode_ = true;
    auto task = CreateDelayedFlushWindowInfoToMMITask();
    PostFlushWindowInfoTask(std::move(task), FLUSH_WINDOW_INFO_TASK_NAME, FLUSH_WINDOW_INFO_START_DELAY_INTERVAL);
    TLOGNI(WmsLogTag::WMS_EVENT, "DelayedFlushWindowInfoToMMI start task");
}

void SceneSessionManager::StopDelayedFlushWindowInfoToMMITask() {
    flushWindowInfoCount_ = 0;
    isDelayFlushWindowInfoMode_ = false;
    TLOGNI(WmsLogTag::WMS_EVENT, "DelayedFlushWindowInfoToMMI stop task");
}

void SceneSessionManager::HandleUserSwitching(bool isUserActive)
{
    // A brief screen freeze may occur when handling a switching event.
    // Record the duration and report it to HiSysEvent for troubleshooting.
    UserSwitchReporter reporter(isUserActive);

    isUserBackground_ = !isUserActive;
    SceneInputManager::GetInstance().SetUserBackground(!isUserActive);
    if (isUserActive) { // switch to current user
        SceneInputManager::GetInstance().SetCurrentUserId(currentUserId_);
        if (MultiInstanceManager::IsSupportMultiInstance(systemConfig_)) {
            MultiInstanceManager::GetInstance().SetCurrentUserId(currentUserId_);
        }
        AbilityInfoManager::GetInstance().SetCurrentUserId(currentUserId_);
        // notify screenSessionManager to recover current user
        FlushWindowInfoToMMI(true);
        StartDelayedFlushWindowInfoToMMITask();
        NotifyAllAccessibilityInfo();
        rsInterface_.AddVirtualScreenBlackList(INVALID_SCREEN_ID, skipSurfaceNodeIds_);
        UpdatePrivateStateAndNotifyForAllScreens();
    } else { // switch to another user
        StopDelayedFlushWindowInfoToMMITask();
        SceneInputManager::GetInstance().FlushEmptyInfoToMMI();
        // minimized UI abilities when the user is switching and inactive
        ProcessUIAbilityOnUserSwitch(isUserActive);
    }
}

void SceneSessionManager::HandleUserSwitched(bool isUserActive)
{
    if (isUserActive) {
        ScreenSessionManagerClient::GetInstance().SwitchingCurrentUser();
        // start UI abilities only after the user has switched and become active
        ProcessUIAbilityOnUserSwitch(isUserActive);
    } else {
        rsInterface_.RemoveVirtualScreenBlackList(INVALID_SCREEN_ID, skipSurfaceNodeIds_);
    }
}

void SceneSessionManager::HandleUserSwitch(const UserSwitchEventType type, const bool isUserActive)
{
    if (type == UserSwitchEventType::SWITCHING && !isUserActive) {
        ScreenSessionManagerClient::GetInstance().DisconnectAllExternalScreen();
    }

    taskScheduler_->PostSyncTask([this, type, isUserActive, where = __func__] {
        TLOGNI(WmsLogTag::WMS_MULTI_USER,
               "%{public}s: currentUserId: %{public}d, switchEventType: %{public}u, isUserActive: %{public}u",
               where, currentUserId_.load(), type, isUserActive);
        switch (type) {
            case UserSwitchEventType::SWITCHING: {
                HandleUserSwitching(isUserActive);
                break;
            }
            case UserSwitchEventType::SWITCHED: {
                HandleUserSwitched(isUserActive);
                break;
            }
            default: {
                TLOGNW(WmsLogTag::WMS_MULTI_USER, "%{public}s: Invalid switchEventType: %{public}u", where, type);
                break;
            }
        }
        return WSError::WS_OK;
    }, __func__);
}

sptr<AppExecFwk::IBundleMgr> SceneSessionManager::GetBundleManager()
{
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to get SystemAbilityManager.");
        return nullptr;
    }
    auto bmsProxy = systemAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bmsProxy == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to get BundleManagerService.");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IBundleMgr>(bmsProxy);
}

std::shared_ptr<Global::Resource::ResourceManager> SceneSessionManager::GetResourceManager(
    const AppExecFwk::AbilityInfo& abilityInfo, Global::Resource::ColorMode colorMode)
{
    auto context = rootSceneContextWeak_.lock();
    if (!context) {
        TLOGE(WmsLogTag::DEFAULT, "context is nullptr.");
        return nullptr;
    }
    auto resourceMgr = context->GetResourceManager();
    if (!resourceMgr) {
        TLOGE(WmsLogTag::DEFAULT, "resourceMgr is nullptr.");
        return nullptr;
    }
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (!resConfig) {
        TLOGE(WmsLogTag::DEFAULT, "resConfig is nullptr.");
        return nullptr;
    }
    resourceMgr->GetResConfig(*resConfig);
    if (colorMode != Global::Resource::ColorMode::COLOR_MODE_NOT_SET) {
        resConfig->SetColorMode(colorMode);
        TLOGI(WmsLogTag::WMS_PATTERN, "resConfig colorMode: %{public}d", colorMode);
    }
    resourceMgr = Global::Resource::CreateResourceManager(
        abilityInfo.bundleName, abilityInfo.moduleName, "", {}, *resConfig);
    if (!resourceMgr) {
        TLOGE(WmsLogTag::DEFAULT, "resourceMgr is nullptr.");
        return nullptr;
    }
    resourceMgr->UpdateResConfig(*resConfig);

    std::string loadPath;
    if (!abilityInfo.hapPath.empty()) { // zipped hap
        loadPath = abilityInfo.hapPath;
    } else {
        loadPath = abilityInfo.resourcePath;
    }

    if (!resourceMgr->AddResource(loadPath.c_str(), Global::Resource::SELECT_COLOR | Global::Resource::SELECT_MEDIA |
        Global::Resource::SELECT_PROF)) {
        TLOGW(WmsLogTag::DEFAULT, "Add resource %{private}s failed.", loadPath.c_str());
    }
    return resourceMgr;
}

bool SceneSessionManager::GetPathInfoFromResource(const std::shared_ptr<Global::Resource::ResourceManager> resourceMgr,
    bool hapPathEmpty, uint32_t resourceId, std::string& path)
{
    if (resourceId == 0) {
        TLOGD(WmsLogTag::WMS_PATTERN, "resource invalid:%{public}u", resourceId);
        return false;
    }
    if (resourceMgr->GetMediaById(resourceId, path) != Global::Resource::RState::SUCCESS) {
        TLOGE(WmsLogTag::WMS_PATTERN, "failed id:%{public}u", resourceId);
        return false;
    }
    if (!hapPathEmpty) {
        auto pos = path.find_last_of('.');
        if (pos == std::string::npos) {
            TLOGE(WmsLogTag::WMS_PATTERN, "Format error, path %{private}s", path.c_str());
            return false;
        }
        path = "resource:///" + std::to_string(resourceId) + path.substr(pos);
    }
    return true;
}

bool SceneSessionManager::CheckStartWindowColorFollowApp(const AppExecFwk::AbilityInfo& abilityInfo,
    const std::shared_ptr<Global::Resource::ResourceManager>& resourceMgr)
{
    const std::string followSystemMode = "FOLLOW_SYSTEM";
    const std::string followAPPMode = "FOLLOW_APPLICATION";
    std::string colorModeType = startingWindowRdbMgr_->GetStartWindowValFromProfile(
        abilityInfo, resourceMgr, STARTWINDOW_COLOR_MODE_TYPE, followSystemMode);
    if (colorModeType != followAPPMode) {
        TLOGI(WmsLogTag::WMS_PATTERN, "follow sys: %{public}s", abilityInfo.bundleName.c_str());
        return false;
    }
    std::string key = abilityInfo.moduleName + abilityInfo.name;
    TLOGI(WmsLogTag::WMS_PATTERN, "follow app: %{public}s %{public}s", abilityInfo.bundleName.c_str(), key.c_str());
    std::unique_lock<std::shared_mutex> lock(startingWindowFollowAppMapMutex_);
    auto iter = startingWindowFollowAppMap_.find(abilityInfo.bundleName);
    if (iter != startingWindowFollowAppMap_.end()) {
        iter->second.insert(key);
        return true;
    }
    std::unordered_set<std::string> infoSet({ key });
    startingWindowFollowAppMap_.emplace(abilityInfo.bundleName, infoSet);
    return true;
}

bool SceneSessionManager::GetStartupPageFromResource(const AppExecFwk::AbilityInfo& abilityInfo,
    StartingWindowInfo& startingWindowInfo, Global::Resource::ColorMode defaultColorMode,
    bool appColorModeChanged, Global::Resource::ColorMode appColorMode)
{
    const std::map<std::string, std::string> START_WINDOW_KEY_AND_DEFAULT_MAP = {
        {STARTWINDOW_TYPE, "REQUIRED_SHOW"},
    };

    auto resourceMgr = GetResourceManager(abilityInfo, defaultColorMode);
    if (!resourceMgr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "resourceMgr is nullptr.");
        return false;
    }
    bool hapPathEmpty = abilityInfo.hapPath.empty();
    if (hapPathEmpty) {
        TLOGD(WmsLogTag::WMS_PATTERN, "hapPath empty:%{public}s", abilityInfo.bundleName.c_str());
    }
    if (CheckStartWindowColorFollowApp(abilityInfo, resourceMgr) && appColorModeChanged) {
        resourceMgr = GetResourceManager(abilityInfo, appColorMode);
        if (!resourceMgr) {
            TLOGE(WmsLogTag::WMS_PATTERN, "resourceMgr is nullptr.");
            return false;
        }
    }

    const auto startWindowType = START_WINDOW_KEY_AND_DEFAULT_MAP.find(STARTWINDOW_TYPE);
    if (startWindowType != START_WINDOW_KEY_AND_DEFAULT_MAP.end()) {
        startingWindowInfo.startWindowType_ = startingWindowRdbMgr_->GetStartWindowValFromProfile(
            abilityInfo, resourceMgr, startWindowType->first, startWindowType->second);
    }

    if (resourceMgr->GetColorById(abilityInfo.startWindowBackgroundId,
            startingWindowInfo.backgroundColorEarlyVersion_) != Global::Resource::RState::SUCCESS ||
        !GetPathInfoFromResource(resourceMgr, hapPathEmpty,
            abilityInfo.startWindowIconId, startingWindowInfo.iconPathEarlyVersion_)) {
        if (!emptyStartupResource_.count(abilityInfo.bundleName)) {
            emptyStartupResource_.insert(abilityInfo.bundleName);
            TLOGE(WmsLogTag::WMS_PATTERN, "failed:%{public}s.", abilityInfo.bundleName.c_str());
        }
        return false;
    }
    // check start_window.json enabled by required field
    startingWindowInfo.configFileEnabled_ = abilityInfo.startWindowResource.startWindowBackgroundColorId != 0;
    if (!startingWindowInfo.configFileEnabled_) {
        return true;
    }
    if (resourceMgr->GetColorById(abilityInfo.startWindowResource.startWindowBackgroundColorId,
            startingWindowInfo.backgroundColor_) != Global::Resource::RState::SUCCESS) {
        TLOGE(WmsLogTag::WMS_PATTERN, "failed:%{public}s.", abilityInfo.bundleName.c_str());
        return false;
    }
    auto res = GetPathInfoFromResource(resourceMgr, hapPathEmpty,
        abilityInfo.startWindowResource.startWindowAppIconId, startingWindowInfo.iconPath_);
    res = GetPathInfoFromResource(resourceMgr, hapPathEmpty,
        abilityInfo.startWindowResource.startWindowIllustrationId, startingWindowInfo.illustrationPath_) && res;
    res = GetPathInfoFromResource(resourceMgr, hapPathEmpty,
        abilityInfo.startWindowResource.startWindowBackgroundImageId, startingWindowInfo.backgroundImagePath_) && res;
    res = GetPathInfoFromResource(resourceMgr, hapPathEmpty,
        abilityInfo.startWindowResource.startWindowBrandingImageId, startingWindowInfo.brandingPath_) && res;
    TLOGD(WmsLogTag::WMS_PATTERN, "all image configured:%{public}s, %{public}d", abilityInfo.bundleName.c_str(), res);
    startingWindowInfo.backgroundImageFit_ = abilityInfo.startWindowResource.startWindowBackgroundImageFit;
    return true;
}

bool SceneSessionManager::GetIsDarkFromConfiguration(const std::string& appColorMode)
{
    if (appColorMode == AppExecFwk::ConfigurationInner::COLOR_MODE_DARK) {
        return true;
    }
    if (appColorMode == AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT) {
        return false;
    }
    std::shared_ptr<AbilityRuntime::ApplicationContext> appContext = AbilityRuntime::Context::GetApplicationContext();
    if (appContext == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "app context is nullptr");
        return false;
    }
    std::shared_ptr<AppExecFwk::Configuration> config = appContext->GetConfiguration();
    if (config == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "app configuration is nullptr");
        return false;
    }
    std::string colorMode = config->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    TLOGI(WmsLogTag::WMS_PATTERN, "colorMode: %{public}s", colorMode.c_str());
    return (colorMode == AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
}

void SceneSessionManager::GetBundleStartingWindowInfos(bool isDark, const AppExecFwk::BundleInfo& bundleInfo,
    std::vector<std::pair<StartingWindowRdbItemKey, StartingWindowInfo>>& outValues)
{
    for (const auto& moduleInfo : bundleInfo.hapModuleInfos) {
        for (const auto& abilityInfo : moduleInfo.abilityInfos) {
            if (abilityInfo.startWindowId == 0 && abilityInfo.startWindowBackgroundId == 0) {
                TLOGD(WmsLogTag::WMS_PATTERN, "not ability:%{public}s", abilityInfo.bundleName.c_str());
                continue;
            }
            StartingWindowRdbItemKey itemKey = {
                .bundleName = abilityInfo.bundleName,
                .moduleName = abilityInfo.moduleName,
                .abilityName = abilityInfo.name,
                .darkMode = isDark,
            };
            StartingWindowInfo startingWindowInfo;
            auto colorMode = isDark ? Global::Resource::ColorMode::DARK : Global::Resource::ColorMode::LIGHT;
            if (!GetStartupPageFromResource(abilityInfo, startingWindowInfo, colorMode)) {
                continue;
            }
            outValues.emplace_back(std::make_pair(itemKey, startingWindowInfo));
        }
    }
}

bool SceneSessionManager::GetIconFromDesk(const SessionInfo& sessionInfo, std::string& startupPagePath) const
{
    auto& want = sessionInfo.want;
    if (want == nullptr) {
        TLOGI(WmsLogTag::WMS_PATTERN, "want is nullPtr");
        return false;
    }
    startupPagePath = want->GetStringParam("realAppIcon");
    if (startupPagePath.empty()) {
        return false;
    }
    return true;
}

void SceneSessionManager::InitStartingWindowRdb(const std::string& rdbPath)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:InitStartingWindowRdb");
    WmsRdbConfig config;
    config.dbPath = rdbPath;
    if (mkdir(rdbPath.c_str(), S_IRWXU)) {
        TLOGW(WmsLogTag::WMS_PATTERN, "mkdir failed or already exists");
    }
    startingWindowRdbMgr_ = std::make_shared<StartingWindowRdbManager>(config,
        systemConfig_.supportPreloadStartingWindow_ ? taskScheduler_->GetEventHandler() : nullptr);
    if (startingWindowRdbMgr_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "create rdb manager fail");
        return;
    }
    if (!startingWindowRdbMgr_->Init()) {
        TLOGE(WmsLogTag::WMS_PATTERN, "init fail %{private}s", config.dbPath.c_str());
        return;
    }
    bool deleteAllDataRes = startingWindowRdbMgr_->DeleteAllData();
    TLOGI(WmsLogTag::WMS_PATTERN, "delete all data res: %{public}d", deleteAllDataRes);
}

void SceneSessionManager::UpdateAllStartingWindowRdb()
{
    const char* const where = __func__;
    auto loadTask = [this, where]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:UpdateAllStartingWindowRdb");
        if (bundleMgr_ == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s bundleMgr is nullptr", where);
            return;
        }
        std::vector<AppExecFwk::BundleInfo> bundleInfos;
        int32_t ret = static_cast<int32_t>(bundleMgr_->GetBundleInfosV9(
            static_cast<uint32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_DISABLE) |
            static_cast<uint32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) |
            static_cast<uint32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) |
            static_cast<uint32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_ONLY_WITH_LAUNCHER_ABILITY),
            bundleInfos, currentUserId_));
        if (ret != 0) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s GetBundleInfosV9 error:%{public}d", where, ret);
            return;
        }
        std::vector<std::pair<StartingWindowRdbItemKey, StartingWindowInfo>> inputValues;
        bool isDark = GetIsDarkFromConfiguration(std::string(AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO));
        for (const auto& bundleInfo : bundleInfos) {
            GetBundleStartingWindowInfos(isDark, bundleInfo, inputValues);
        }
        if (startingWindowRdbMgr_ == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s rdb manager is nullptr", where);
            return;
        }
        int64_t outInsertNum = -1;
        auto batchInsertRes = startingWindowRdbMgr_->BatchInsert(outInsertNum, inputValues);
        TLOGNI(WmsLogTag::WMS_PATTERN, "%{public}s res: %{public}d, bundles: %{public}zu, insert: %{public}" PRId64,
            where, batchInsertRes, bundleInfos.size(), outInsertNum);
    };
    ffrtQueueHelper_->SubmitTask(loadTask);
}

std::string SceneSessionManager::GetCallerSessionColorMode(const SessionInfo& sessionInfo)
{
    if (sessionInfo.bundleName_ != sessionInfo.callerBundleName_) {
        TLOGI(WmsLogTag::WMS_PATTERN, "bundle %{public}s, caller %{public}s",
            sessionInfo.bundleName_.c_str(), sessionInfo.callerBundleName_.c_str());
        return AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO;
    }
    auto callerId = sessionInfo.callerPersistentId_;
    auto callerSession = GetSceneSession(callerId);
    if (callerSession == nullptr) {
        TLOGI(WmsLogTag::WMS_PATTERN, "caller session: %{public}d, nullptr", callerId);
        return AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO;
    }
    auto colorMode = callerSession->GetAbilityColorMode();
    return colorMode;
}

bool SceneSessionManager::GetStartWindowColorFollowApp(const SessionInfo& sessionInfo)
{
    std::shared_lock<std::shared_mutex> lock(startingWindowFollowAppMapMutex_);
    auto iter = startingWindowFollowAppMap_.find(sessionInfo.bundleName_);
    if (iter == startingWindowFollowAppMap_.end()) {
        TLOGI(WmsLogTag::WMS_PATTERN, "follow sys: %{public}s", sessionInfo.bundleName_.c_str());
        return false;
    }
    std::string key = sessionInfo.moduleName_ + sessionInfo.abilityName_;
    if (iter->second.count(key) != 0) {
        TLOGI(WmsLogTag::WMS_PATTERN, "follow app: %{public}s", key.c_str());
        return true;
    }
    TLOGI(WmsLogTag::WMS_PATTERN, "ability follow sys: %{public}s, %{public}s",
        sessionInfo.bundleName_.c_str(), key.c_str());
    return false;
}

void SceneSessionManager::GetStartupPage(const SessionInfo& sessionInfo, StartingWindowInfo& startingWindowInfo)
{
    if (GetIconFromDesk(sessionInfo, startingWindowInfo.iconPathEarlyVersion_)) {
        TLOGI(WmsLogTag::WMS_PATTERN, "get icon from desk success");
        return;
    }
    if (!bundleMgr_) {
        TLOGE(WmsLogTag::WMS_PATTERN, "bundleMgr_ is nullptr.");
        return;
    }
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetStartupPage");
    bool isAppDark = GetIsDarkFromConfiguration(GetCallerSessionColorMode(sessionInfo));
    bool isSystemDark = GetIsDarkFromConfiguration(std::string(AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO));
    bool isDark = GetStartWindowColorFollowApp(sessionInfo) ? isAppDark : isSystemDark;
    if (GetStartingWindowInfoFromCache(sessionInfo, startingWindowInfo, isDark)) {
        return;
    }
    startingWindowInfo.backgroundColorEarlyVersion_ = isDark ? COLOR_BLACK : COLOR_WHITE;
    if (GetStartingWindowInfoFromRdb(sessionInfo, startingWindowInfo, isDark)) {
        CacheStartingWindowInfo(
            sessionInfo.bundleName_, sessionInfo.moduleName_, sessionInfo.abilityName_, startingWindowInfo, isDark);
        uint32_t updateRes = UpdateCachedColorToAppSet(
            sessionInfo.bundleName_, sessionInfo.moduleName_, sessionInfo.abilityName_, startingWindowInfo);
        TLOGI(WmsLogTag::WMS_PATTERN, "updateRes %{public}u, %{public}x",
            updateRes, startingWindowInfo.backgroundColor_);
        return;
    }
    AAFwk::Want want;
    want.SetElementName("", sessionInfo.bundleName_, sessionInfo.abilityName_, sessionInfo.moduleName_);
    AppExecFwk::AbilityInfo abilityInfo;
    if (!bundleMgr_->QueryAbilityInfo(
        want, AppExecFwk::GET_ABILITY_INFO_DEFAULT, AppExecFwk::Constants::ANY_USERID, abilityInfo)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "Get ability info from BMS failed!");
        return;
    }
    auto appColorMode = isAppDark ? Global::Resource::ColorMode::DARK : Global::Resource::ColorMode::LIGHT;
    if (GetStartupPageFromResource(abilityInfo, startingWindowInfo, Global::Resource::ColorMode::COLOR_MODE_NOT_SET,
                                   isAppDark != isSystemDark, appColorMode)) {
        isDark = GetStartWindowColorFollowApp(sessionInfo) ? isAppDark : isSystemDark;
        CacheStartingWindowInfo(
            sessionInfo.bundleName_, sessionInfo.moduleName_, sessionInfo.abilityName_, startingWindowInfo, isDark);
        if (startingWindowRdbMgr_ != nullptr) {	
            StartingWindowRdbItemKey itemKey = {
                .bundleName = sessionInfo.bundleName_,
                .moduleName = sessionInfo.moduleName_,
                .abilityName = sessionInfo.abilityName_,
                .darkMode = isDark,
            };
            if (!startingWindowRdbMgr_->InsertData(itemKey, startingWindowInfo)) {
                TLOGW(WmsLogTag::WMS_PATTERN, "rdb insert faild");
            }
        }
        uint32_t updateRes = UpdateCachedColorToAppSet(
            sessionInfo.bundleName_, sessionInfo.moduleName_, sessionInfo.abilityName_, startingWindowInfo);
        TLOGI(WmsLogTag::WMS_PATTERN, "updateRes %{public}u, %{public}x",
            updateRes, startingWindowInfo.backgroundColor_);
    }
    TLOGW(WmsLogTag::WMS_PATTERN, "%{public}d, %{public}x", startingWindowInfo.configFileEnabled_,
        startingWindowInfo.configFileEnabled_ ? startingWindowInfo.backgroundColor_ :
                                                startingWindowInfo.backgroundColorEarlyVersion_);
}

bool SceneSessionManager::GetStartingWindowInfoFromCache(
    const SessionInfo& sessionInfo, StartingWindowInfo& startingWindowInfo, bool isDark)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetStartingWindowInfoFromCache");
    std::shared_lock<std::shared_mutex> lock(startingWindowMapMutex_);
    auto iter = startingWindowMap_.find(sessionInfo.bundleName_);
    if (iter == startingWindowMap_.end()) {
        return false;
    }
    auto key = sessionInfo.moduleName_ + sessionInfo.abilityName_ + std::to_string(isDark);
    const auto& infoMap = iter->second;
    auto infoMapIter = infoMap.find(key);
    if (infoMapIter == infoMap.end()) {
        return false;
    }
    startingWindowInfo = infoMapIter->second;
    TLOGW(WmsLogTag::WMS_PATTERN, "%{public}x, %{public}s, %{public}x, %{public}s, %{public}d",
        startingWindowInfo.backgroundColorEarlyVersion_, startingWindowInfo.iconPathEarlyVersion_.c_str(),
        startingWindowInfo.backgroundColor_, startingWindowInfo.iconPath_.c_str(), isDark);
    return true;
}

bool SceneSessionManager::GetStartingWindowInfoFromRdb(
    const SessionInfo& sessionInfo, StartingWindowInfo& startingWindowInfo, bool darkMode)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetStartingWindowInfoFromRdb");
    if (startingWindowRdbMgr_ == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "rdb is nullptr");
        return false;
    }
    StartingWindowRdbItemKey itemKey = {
        .bundleName = sessionInfo.bundleName_,
        .moduleName = sessionInfo.moduleName_,
        .abilityName = sessionInfo.abilityName_,
        .darkMode = darkMode,
    };
    bool res = startingWindowRdbMgr_->QueryData(itemKey, startingWindowInfo);
    TLOGW(WmsLogTag::WMS_PATTERN, "%{public}x, %{public}s, %{public}x, %{public}s, %{public}d",
        startingWindowInfo.backgroundColorEarlyVersion_, startingWindowInfo.iconPathEarlyVersion_.c_str(),
        startingWindowInfo.backgroundColor_, startingWindowInfo.iconPath_.c_str(), res);
    return res;
}

uint32_t SceneSessionManager::UpdateCachedColorToAppSet(const std::string& bundleName, const std::string& moduleName,
    const std::string& abilityName, StartingWindowInfo& info)
{
    auto key = moduleName + abilityName;
    uint32_t color = 0;
    std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> colorFromAppMap;
    {
        std::shared_lock<std::shared_mutex> lock(startingWindowColorFromAppMapMutex_);
        colorFromAppMap = startingWindowColorFromAppMap_;
    }
    auto colorMapIter = colorFromAppMap.find(bundleName);
    if (colorMapIter == colorFromAppMap.end()) {
        return static_cast<uint32_t>(UpdateStartingWindowColorCacheResult::COLOR_MAP_BUNDLE_NOT_FOUND);
    }
    auto& colorMap = colorMapIter->second;
    auto colorIter = colorMap.find(key);
    if (colorIter == colorMap.end()) {
        return static_cast<uint32_t>(UpdateStartingWindowColorCacheResult::COLOR_MAP_KEY_PAIR_NOT_FOUND);
    }
    color = colorIter->second;
    info.backgroundColor_ = color;
    info.backgroundColorEarlyVersion_ = color;
    {
        std::unique_lock<std::shared_mutex> lock(startingWindowMapMutex_);
        auto iter = startingWindowMap_.find(bundleName);
        if (iter == startingWindowMap_.end()) {
            return static_cast<uint32_t>(UpdateStartingWindowColorCacheResult::INFO_MAP_BUNDLE_NOT_FOUND);
        }
        auto& infoMap = iter->second;
        bool keyPairFound = false;
        for (bool darkMode : {false, true}) {
            auto infoIter = infoMap.find(key + std::to_string(darkMode));
            if (infoIter != infoMap.end()) {
                infoIter->second.backgroundColorEarlyVersion_ = color;
                infoIter->second.backgroundColor_ = color;
                keyPairFound = true;
            }
        }
        if (!keyPairFound) {
            return static_cast<uint32_t>(UpdateStartingWindowColorCacheResult::INFO_MAP_KEY_PAIR_NOT_FOUND);
        }
    }
    return static_cast<uint32_t>(UpdateStartingWindowColorCacheResult::SUCCESS);
}

void SceneSessionManager::CacheStartingWindowInfo(const std::string& bundleName, const std::string& moduleName,
    const std::string& abilityName, const StartingWindowInfo& startingWindowInfo, bool isDark)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:CacheStartingWindowInfo");
    auto key = moduleName + abilityName + std::to_string(isDark);
    std::unique_lock<std::shared_mutex> lock(startingWindowMapMutex_);
    auto iter = startingWindowMap_.find(bundleName);
    if (iter != startingWindowMap_.end()) {
        auto& infoMap = iter->second;
        infoMap.emplace(key, startingWindowInfo);
        return;
    }
    if (startingWindowMap_.size() >= MAX_CACHE_COUNT) {
        startingWindowMap_.erase(startingWindowMap_.begin());
    }
    std::map<std::string, StartingWindowInfo> infoMap({{ key, startingWindowInfo }});
    startingWindowMap_.emplace(bundleName, infoMap);
}

std::shared_ptr<Media::PixelMap> SceneSessionManager::GetPreLoadStartingWindow(const SessionInfo& sessionInfo)
{
    std::unordered_map<std::string, std::shared_ptr<Media::PixelMap>> preLoadMap;
    {
        std::shared_lock<std::shared_mutex> lock(preLoadstartingWindowMapMutex_);
        preLoadMap = preLoadStartingWindowMap_;
    }
    std::string key = sessionInfo.bundleName_ + '_' + sessionInfo.moduleName_ + '_' + sessionInfo.abilityName_;
    auto iter = preLoadMap.find(key);
    if (iter == preLoadMap.end()) {
        TLOGI(WmsLogTag::WMS_PATTERN, "%{public}s not found", key.c_str());
        return nullptr;
    }
    return iter->second;
}

void SceneSessionManager::RemovePreLoadStartingWindowFromMap(const SessionInfo& sessionInfo)
{
    std::string key = sessionInfo.bundleName_ + '_' + sessionInfo.moduleName_ + '_' + sessionInfo.abilityName_;
    std::unique_lock<std::shared_mutex> lock(preLoadstartingWindowMapMutex_);
    auto iter = preLoadStartingWindowMap_.find(key);
    if (iter != preLoadStartingWindowMap_.end()) {
        preLoadStartingWindowMap_.erase(iter);
        TLOGI(WmsLogTag::WMS_PATTERN, "%{public}s", key.c_str());
    }
}

void SceneSessionManager::PreLoadStartingWindow(sptr<SceneSession> sceneSession)
{
    if (!systemConfig_.supportPreloadStartingWindow_) {
        TLOGD(WmsLogTag::WMS_PATTERN, "not supported");
        return;
    }
    if (IsSyncLoadStartingWindow()) {
        TLOGD(WmsLogTag::WMS_PATTERN, "sync load starting window");
        return;
    }
    const char* const where = __func__;
    auto loadTask = [this, weakSceneSession = wptr(sceneSession), where]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:PreLoadStartingWindow");
        sptr<SceneSession> sceneSession = weakSceneSession.promote();
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s session is nullptr", where);
            return;
        }
        auto sessionInfo = sceneSession->GetSessionInfo();
        if (!SessionHelper::IsMainWindow(static_cast<WindowType>(sessionInfo.windowType_))) {
            TLOGND(WmsLogTag::WMS_PATTERN, "%{public}s id: %{public}d is not main window",
                where, sceneSession->GetPersistentId());
            return;
        }
        if (sceneSession->GetSessionState() != SessionState::STATE_DISCONNECT) {
            TLOGND(WmsLogTag::WMS_PATTERN, "%{public}s id: %{public}d is not disconnect",
                where, sceneSession->GetPersistentId());
            sceneSession->PreloadSnapshot();
            return;
        }
        StartingWindowInfo startingWindowInfo;
        GetStartupPage(sessionInfo, startingWindowInfo);
        uint32_t resId = 0;
        if (!CheckAndGetPreLoadResourceId(startingWindowInfo, resId)) {
            TLOGND(WmsLogTag::WMS_PATTERN, "%{public}s check no need preLoad", where);
            return;
        }
        auto abilityInfo = sceneSession->GetSessionInfoAbilityInfo();
        if (abilityInfo == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s id: %{public}d abilityInfo is nullptr",
                where, sceneSession->GetPersistentId());
            return;
        }
        sceneSession->SetPreloadingStartingWindow(true);
        ScopeGuard stateGuard([&sceneSession] { sceneSession->SetPreloadingStartingWindow(false); });
        auto pixelMap = GetPixelMap(resId, abilityInfo, true);
        if (pixelMap == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s pixelMap is nullptr", where);
            return;
        }
        sceneSession->SetBufferNameForPixelMap(where, pixelMap);
        std::string key = sessionInfo.bundleName_ + '_' + sessionInfo.moduleName_ + '_' + sessionInfo.abilityName_;
        {
            std::unique_lock<std::shared_mutex> lock(preLoadstartingWindowMapMutex_);
            preLoadStartingWindowMap_[key] = pixelMap;
        }
        sceneSession->NotifyPreLoadStartingWindowFinished();
    };
    ffrtQueueHelper_->SubmitTask(loadTask);
}

bool SceneSessionManager::CheckAndGetPreLoadResourceId(const StartingWindowInfo& startingWindowInfo, uint32_t& resId)
{
    if (startingWindowInfo.configFileEnabled_) {
        TLOGD(WmsLogTag::WMS_PATTERN, "config file enabled");
        return false;
    }
    const auto& iconPath= startingWindowInfo.iconPathEarlyVersion_;
    auto pos = iconPath.find_last_of('.');
    constexpr uint32_t RESOURCE_PATH_HEAD_LENGTH = 12;
    if (pos == std::string::npos || pos <= RESOURCE_PATH_HEAD_LENGTH) {
        TLOGE(WmsLogTag::WMS_PATTERN, "format error: %{private}s", iconPath.c_str());
        return false;
    }
    static const std::unordered_set<std::string> supportImageFormat = {
        ".png", ".jpg", ".webp", ".astc"
    };
    const auto& extName = iconPath.substr(pos);
    if (!supportImageFormat.count(extName)) {
        TLOGI(WmsLogTag::WMS_PATTERN, "format not need preLoad: %{private}s", iconPath.c_str());
        return false;
    }
    const auto& resIdStr = iconPath.substr(RESOURCE_PATH_HEAD_LENGTH, pos - RESOURCE_PATH_HEAD_LENGTH);
    if (!WindowHelper::IsNumber(resIdStr)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "number format error: %{public}s", resIdStr.c_str());
        return false;
    }
    resId = std::stoul(resIdStr);
    return true;
}

void SceneSessionManager::ClearStartWindowColorFollowApp(const std::string& bundleName)
{
    std::unique_lock<std::shared_mutex> lock(startingWindowFollowAppMapMutex_);
    if (auto iter = startingWindowFollowAppMap_.find(bundleName); iter != startingWindowFollowAppMap_.end()) {
        TLOGI(WmsLogTag::WMS_PATTERN, "erase %{public}s", bundleName.c_str());
        startingWindowFollowAppMap_.erase(iter);
    }
}

void SceneSessionManager::OnBundleUpdated(const std::string& bundleName, int userId)
{
    taskScheduler_->PostAsyncTask([this, bundleName]() {
        ClearStartWindowColorFollowApp(bundleName);
    }, __func__);
    auto task = [this, bundleName, where = __func__] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:OnBundleUpdated [%s]", bundleName.c_str());
        if (startingWindowRdbMgr_ == nullptr || bundleMgr_ == nullptr) {
            TLOGNE(WmsLogTag::WMS_PATTERN, "%{public}s: rdb or bms is nullptr", where);
            return;
        }
        startingWindowRdbMgr_->DeleteDataByBundleName(bundleName);
        TLOGNI(WmsLogTag::WMS_PATTERN, "%{public}s delete start window rdb", where);
        AppExecFwk::BundleInfo bundleInfo;
        bool ret = bundleMgr_->GetBundleInfoV9(bundleName,
            static_cast<uint32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_DISABLE) |
            static_cast<uint32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) |
            static_cast<uint32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY),
            bundleInfo, currentUserId_);
        if (ret == 0) {
            std::vector<std::pair<StartingWindowRdbItemKey, StartingWindowInfo>> inputValues;
            GetBundleStartingWindowInfos(GetIsDarkFromConfiguration(std::string(AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO)),
                bundleInfo, inputValues);
            int64_t outInsertNum = -1;
            auto batchInsertRes = startingWindowRdbMgr_->BatchInsert(outInsertNum, inputValues);
            TLOGNI(WmsLogTag::WMS_PATTERN, "%{public}s updated rdb res:%{public}d, insert num:%{public}" PRId64,
                where, batchInsertRes, outInsertNum);
        }
        std::unique_lock<std::shared_mutex> lock(startingWindowMapMutex_);
        if (auto iter = startingWindowMap_.find(bundleName); iter != startingWindowMap_.end()) {
            startingWindowMap_.erase(iter);
        }
        TLOGNI(WmsLogTag::WMS_PATTERN, "%{public}s delete start window cache", where);
    };
    FfrtSerialQueueHelper::GetInstance().SubmitTask(task);
}

void SceneSessionManager::OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration>& configuration)
{
    taskScheduler_->PostAsyncTask([this]() {
        std::unique_lock<std::shared_mutex> lock(startingWindowMapMutex_);
        startingWindowMap_.clear();
    }, __func__);
}

std::vector<AppExecFwk::SupportWindowMode> SceneSessionManager::ExtractSupportWindowModeFromMetaData(
    const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    std::vector<AppExecFwk::SupportWindowMode> updateWindowModes = {};
    if (systemConfig_.IsPcWindow() || systemConfig_.IsFreeMultiWindowMode()) {
        auto metadata = abilityInfo->metadata;
        for (auto item : metadata) {
            if (item.name == "ohos.ability.window.supportWindowModeInFreeMultiWindow") {
                updateWindowModes = ParseWindowModeFromMetaData(item.value);
            }
        }
    }
    if (updateWindowModes.empty()) {
        updateWindowModes = abilityInfo->windowModes;
    }
    return updateWindowModes;
}

std::vector<AppExecFwk::SupportWindowMode> SceneSessionManager::ParseWindowModeFromMetaData(
    const std::string& supportModesInFreeMultiWindow)
{
    std::vector<AppExecFwk::SupportWindowMode> updateWindowModes = {};
    std::stringstream supportModes(supportModesInFreeMultiWindow);
    std::string supportWindowMode;
    while (getline(supportModes, supportWindowMode, ',')) {
        if (supportWindowMode == "fullscreen") {
            updateWindowModes.push_back(AppExecFwk::SupportWindowMode::FULLSCREEN);
        } else if (supportWindowMode == "split") {
            updateWindowModes.push_back(AppExecFwk::SupportWindowMode::SPLIT);
        } else if (supportWindowMode == "floating") {
            updateWindowModes.push_back(AppExecFwk::SupportWindowMode::FLOATING);
        }
    }
    return updateWindowModes;
}


void SceneSessionManager::FillSessionInfo(sptr<SceneSession>& sceneSession)
{
    const auto& sessionInfo = sceneSession->GetSessionInfo();
    if (sessionInfo.bundleName_.empty()) {
        TLOGE(WmsLogTag::DEFAULT, "bundleName_ is empty");
        return;
    }
    if (sessionInfo.isSystem_) {
        TLOGD(WmsLogTag::DEFAULT, "is system scene!");
        return;
    }
    auto abilityInfo = QueryAbilityInfoFromBMS(currentUserId_, sessionInfo.bundleName_, sessionInfo.abilityName_,
        sessionInfo.moduleName_, IsAtomicServiceFreeInstall(sessionInfo));
    if (abilityInfo == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Null");
        return;
    }
    if (sessionInfo.supportedWindowModes.empty()) {
        sceneSession->SetSessionInfoSupportedWindowModes(ExtractSupportWindowModeFromMetaData(abilityInfo));
    }
    sceneSession->SetEnableRemoveStartingWindow(GetEnableRemoveStartingWindowFromBMS(abilityInfo));
    sceneSession->SetSessionInfoAbilityInfo(abilityInfo);
    sceneSession->SetSessionInfoTime(GetCurrentTime());
    if (abilityInfo->applicationInfo.codePath == std::to_string(CollaboratorType::RESERVE_TYPE)) {
        sceneSession->SetCollaboratorType(CollaboratorType::RESERVE_TYPE);
    } else if (abilityInfo->applicationInfo.codePath == std::to_string(CollaboratorType::OTHERS_TYPE)) {
        sceneSession->SetCollaboratorType(CollaboratorType::OTHERS_TYPE);
    }
    SetSessionInfoStartWindowType(sceneSession);
    TLOGI(WmsLogTag::WMS_MAIN, "bundleName:%{public}s removeMissionAfterTerminate:%{public}d "
        "excludeFromMissions:%{public}d label:%{public}s iconPath:%{public}s collaboratorType:%{public}s.",
        abilityInfo->bundleName.c_str(), abilityInfo->removeMissionAfterTerminate, abilityInfo->excludeFromMissions,
        abilityInfo->label.c_str(), abilityInfo->iconPath.c_str(), abilityInfo->applicationInfo.codePath.c_str());
}

void SceneSessionManager::SetSessionInfoStartWindowType(const sptr<SceneSession>& sceneSession)
{
    if (systemConfig_.IsPcOrPcMode() && !sceneSession->GetSessionInfo().isSetStartWindowType_) {
        StartingWindowInfo startingWindowInfo;
        GetStartupPage(sceneSession->GetSessionInfo(), startingWindowInfo);
        TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d, startWindowType from rdb:%{public}s",
            sceneSession->GetPersistentId(), startingWindowInfo.startWindowType_.c_str());
        if (CONVERT_STRING_TO_START_WINDOW_TYPE_MAP.count(startingWindowInfo.startWindowType_) > 0) {
            sceneSession->EditSessionInfo().startWindowType_ =
                CONVERT_STRING_TO_START_WINDOW_TYPE_MAP.at(startingWindowInfo.startWindowType_);
        }
        if (sceneSession->GetSessionInfo().startWindowType_ == StartWindowType::RETAIN_AND_INVISIBLE) {
            sceneSession->SetHidingStartingWindow(true);
        }
        TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d, startWindowType:%{public}d",
            sceneSession->GetPersistentId(), sceneSession->GetSessionInfo().startWindowType_);
    }
}

void SceneSessionManager::RegisterGetStartWindowConfigCallback(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr");
        return;
    }
    sceneSession->RegisterGetStartWindowConfigFunc([this](const SessionInfo& sessionInfo, std::string& startWindowType) {
        StartingWindowInfo startingWindowInfo;
        GetStartupPage(sessionInfo, startingWindowInfo);
        startWindowType = startingWindowInfo.startWindowType_;
        TLOGI(WmsLogTag::WMS_LIFE, "id:%{public}d, startWindowType:%{public}s",
            sessionInfo.persistentId_, startWindowType.c_str());
    });
}


std::shared_ptr<AppExecFwk::AbilityInfo> SceneSessionManager::QueryAbilityInfoFromBMS(const int32_t uId,
    const std::string& bundleName, const std::string& abilityName, const std::string& moduleName,
    bool isAtomicServiceFreeInstall)
{
    if (!bundleMgr_) {
        TLOGE(WmsLogTag::DEFAULT, "bundleMgr_ is nullptr");
        return nullptr;
    }
    SessionInfoList list = {
        .uid_ = uId, .bundleName_ = bundleName, .abilityName_ = abilityName, .moduleName_ = moduleName
    };
    if (abilityInfoMap_.count(list)) {
        return abilityInfoMap_[list];
    }
    if (isAtomicServiceFreeInstall && abilityName.empty() && moduleName.empty()) {
        AppExecFwk::BundleInfo bundleInfo;
        auto flag = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) |
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE);
        bool ret = bundleMgr_->GetBundleInfoV9(bundleName, flag, bundleInfo, currentUserId_);
        if (ret) {
            TLOGE(WmsLogTag::WMS_LIFE, "Get bundle:%{public}s info from BMS failed!", bundleName.c_str());
            return nullptr;
        }
        for (const auto& hapModuleInfo : bundleInfo.hapModuleInfos) {
            if (!hapModuleInfo.abilityInfos.empty()) {
                TLOGI(WmsLogTag::WMS_LIFE, "Get abilityInfo success in AtomicFreeInstall, bundle:%{public}s.",
                    bundleName.c_str());

                // Atomic services only have one ability.
                return std::make_shared<AppExecFwk::AbilityInfo>(hapModuleInfo.abilityInfos[0]);
            }
        }
        TLOGE(WmsLogTag::WMS_LIFE, "Get abilityInfo failed, bundleName:%{public}s.", bundleName.c_str());
        return nullptr;
    }

    AAFwk::Want want;
    want.SetElementName("", bundleName, abilityName, moduleName);
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    if (abilityInfo == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "abilityInfo is nullptr!");
        return nullptr;
    }
    auto abilityInfoFlag = (AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA);
    bool ret = bundleMgr_->QueryAbilityInfo(want, abilityInfoFlag, uId, *abilityInfo);
    if (!ret) {
        TLOGE(WmsLogTag::DEFAULT, "Failed");
        return nullptr;
    }
    abilityInfoMap_[list] = abilityInfo;
    return abilityInfo;
}

static void GetTopWindowByTraverseSessionTree(const sptr<SceneSession>& session,
    uint32_t& topWinId, uint32_t& zOrder)
{
    const auto& subVec = session->GetSubSession();
    for (const auto& subSession : subVec) {
        if (subSession == nullptr || subSession->GetCallingPid() != session->GetCallingPid()) {
            TLOGW(WmsLogTag::WMS_SUB,
                "subSession is null or subWin's callingPid is not equal to mainWin's callingPid");
            continue;
        }
        if ((subSession->GetSessionState() == SessionState::STATE_FOREGROUND ||
             subSession->GetSessionState() == SessionState::STATE_ACTIVE) &&
            subSession->GetZOrder() > zOrder) {
            topWinId = static_cast<uint32_t>(subSession->GetPersistentId());
            zOrder = subSession->GetZOrder();
            TLOGD(WmsLogTag::WMS_SUB, "Current zorder is larger than mainWin, mainId: %{public}d, "
                "topWinId: %{public}d, zOrder: %{public}d", session->GetPersistentId(), topWinId, zOrder);
        }
        if (subSession->GetSubSession().size() > 0) {
            GetTopWindowByTraverseSessionTree(subSession, topWinId, zOrder);
        }
    }
}

/** @note @window.hierarchy */
WMError SceneSessionManager::GetTopWindowId(uint32_t mainWinId, uint32_t& topWinId)
{
    const auto& callingPid = IPCSkeleton::GetCallingRealPid();
    auto task = [this, mainWinId, &topWinId, callingPid]() {
        const auto& mainSession = GetSceneSession(mainWinId);
        if (mainSession == nullptr) {
            return WMError::WM_ERROR_INVALID_WINDOW;
        }

        if (callingPid != mainSession->GetCallingPid()) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "Permission denied, not destroy by the same process");
            return WMError::WM_ERROR_INVALID_PERMISSION;
        }
        uint32_t zOrder = mainSession->GetZOrder();
        topWinId = mainWinId;
        GetTopWindowByTraverseSessionTree(mainSession, topWinId, zOrder);
        TLOGNI(WmsLogTag::WMS_SUB, "[GetTopWin] Get top window, mainId: %{public}d, topWinId: %{public}d, "
            "zOrder: %{public}d", mainWinId, topWinId, zOrder);
        return WMError::WM_OK;
    };

    if (!Session::IsScbCoreEnabled()) {
        return taskScheduler_->PostSyncTask(task, "GetTopWindowId");
    }
    bool postNow = false;
    auto mainSession = GetSceneSession(mainWinId);
    if (mainSession != nullptr) {
        postNow = !(mainSession->GetUIStateDirty());
    }
    auto mainEventRunner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (postNow || (mainEventRunner && mainEventRunner->IsCurrentRunnerThread()) ||
        taskScheduler_->GetEventHandler()->GetEventRunner()->IsCurrentRunnerThread()) {
        return taskScheduler_->PostSyncTask(task, "GetTopWindowId");
    }
    TLOGI(WmsLogTag::WMS_PIPELINE, "wait for flush UI, id: %{public}d", mainWinId);
    {
        std::unique_lock<std::mutex> lock(nextFlushCompletedMutex_);
        if (DmUtils::safe_wait_for(nextFlushCompletedCV_, lock, std::chrono::milliseconds(GET_TOP_WINDOW_DELAY)) ==
            std::cv_status::timeout) {
            TLOGW(WmsLogTag::WMS_PIPELINE, "wait for 100ms");
        }
    }
    return taskScheduler_->PostSyncTask(task, "GetTopWindowId");
}

static WMError GetParentMainWindowIdInner(const std::map<int32_t, sptr<SceneSession>>& sceneSessionMap,
    int32_t windowId, int32_t& mainWindowId)
{
    auto iter = sceneSessionMap.find(windowId);
    if (iter == sceneSessionMap.end()) {
        TLOGD(WmsLogTag::WMS_SUB, "not found scene session with id: %{public}d", windowId);
        return WMError::WM_ERROR_NULLPTR;
    }
    sptr<SceneSession> sceneSession = iter->second;
    if (sceneSession == nullptr) {
        TLOGW(WmsLogTag::WMS_SUB, "not find parent session");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
        mainWindowId = sceneSession->GetPersistentId();
        return WMError::WM_OK;
    }
    if (WindowHelper::IsSubWindow(sceneSession->GetWindowType()) ||
        WindowHelper::IsDialogWindow(sceneSession->GetWindowType())) {
        return GetParentMainWindowIdInner(sceneSessionMap, sceneSession->GetParentPersistentId(), mainWindowId);
    }
    // not sub window, dialog, return invalid id
    mainWindowId = INVALID_SESSION_ID;
    return WMError::WM_OK;
}

WMError SceneSessionManager::GetParentMainWindowId(int32_t windowId, int32_t& mainWindowId)
{
    if (windowId == INVALID_SESSION_ID) {
        TLOGW(WmsLogTag::WMS_SUB, "invalid windowId id");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    return GetParentMainWindowIdInner(sceneSessionMap_, windowId, mainWindowId);
}

void SceneSessionManager::HandleSpecificSystemBarProperty(WindowType type, const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession)
{
    auto systemBarProperties = property->GetSystemBarProperty();
    for (auto iter : systemBarProperties) {
        if (iter.first == type) {
            sceneSession->SetSystemBarProperty(iter.first, iter.second);
            TLOGD(WmsLogTag::WMS_IMMS, "type %{public}d enable %{public}d",
                static_cast<int32_t>(iter.first), iter.second.enable_);
        }
    }
    NotifyWindowInfoChange(property->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_PROPERTY);
}

/** @note @window.hierarchy */
WMError SceneSessionManager::UpdateTopmostProperty(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "UpdateTopmostProperty permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }

    sceneSession->SetTopmost(property->IsTopmost());
    return WMError::WM_OK;
}

void SceneSessionManager::HandleHideNonSystemFloatingWindows(const sptr<WindowSessionProperty>& property,
    const sptr<SceneSession>& sceneSession)
{
    auto propertyOld = sceneSession->GetSessionProperty();
    bool hideNonSystemFloatingWindowsOld = propertyOld->GetHideNonSystemFloatingWindows();
    bool hideNonSystemFloatingWindowsNew = property->GetHideNonSystemFloatingWindows();
    if (hideNonSystemFloatingWindowsOld == hideNonSystemFloatingWindowsNew) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "property hideNonSystemFloatingWindows not change");
        return;
    }

    if (IsSessionVisibleForeground(sceneSession)) {
        if (hideNonSystemFloatingWindowsOld) {
            UpdateForceHideState(sceneSession, propertyOld, false);
        } else {
            UpdateForceHideState(sceneSession, property, true);
        }
    }
}

void SceneSessionManager::UpdateForceHideState(const sptr<SceneSession>& sceneSession,
    const sptr<WindowSessionProperty>& property, bool add)
{
    if (systemConfig_.IsPcWindow()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "IsPcWindow, ineffective");
        return;
    }
    if (property == nullptr) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "property is null");
        return;
    }
    auto persistentId = sceneSession->GetPersistentId();
    bool forceHideFloatOld = !systemTopSceneSessionMap_.empty();
    bool notifyAll = false;
    if (add) {
        if (property->GetHideNonSystemFloatingWindows() && sceneSession->GetWindowMode() != WindowMode::WINDOW_MODE_PIP) {
            systemTopSceneSessionMap_.insert({ persistentId, sceneSession });
            notifyAll = !forceHideFloatOld;
        } else if ((property->IsFloatingWindowAppType() && !property->GetSystemCalling() &&
            sceneSession->GetWindowMode() != WindowMode::WINDOW_MODE_PIP) ||
            sceneSession->GetIsAncoForFloatingWindow()) {
            nonSystemFloatSceneSessionMap_.insert({ persistentId, sceneSession });
            if (forceHideFloatOld) {
                sceneSession->NotifyForceHideChange(true);
            }
        }
    } else {
        if (property->GetHideNonSystemFloatingWindows()) {
            systemTopSceneSessionMap_.erase(persistentId);
            notifyAll = forceHideFloatOld && systemTopSceneSessionMap_.empty();
        } else if ((property->IsFloatingWindowAppType() && !property->GetSystemCalling()) ||
            sceneSession->GetIsAncoForFloatingWindow()) {
            nonSystemFloatSceneSessionMap_.erase(persistentId);
            if (property->GetForceHide()) {
                sceneSession->NotifyForceHideChange(false);
            }
        }
    }
    if (notifyAll) {
        bool forceHideFloatNew = !systemTopSceneSessionMap_.empty();
        for (const auto& item : nonSystemFloatSceneSessionMap_) {
            auto forceHideSceneSession = item.second;
            if (forceHideFloatNew != forceHideSceneSession->GetSessionProperty()->GetForceHide()) {
                forceHideSceneSession->NotifyForceHideChange(forceHideFloatNew);
            }
        }
    }
}

void SceneSessionManager::HandleTurnScreenOn(const sptr<SceneSession>& sceneSession)
{
#ifdef POWER_MANAGER_ENABLE
    auto task = [this, sceneSession]() {
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "session is invalid");
            return;
        }
        TLOGND(WmsLogTag::WMS_ATTRIBUTE, "Win: %{public}s, is turn on%{public}d",
            sceneSession->GetWindowName().c_str(), sceneSession->IsTurnScreenOn());
        std::string identity = IPCSkeleton::ResetCallingIdentity();
        if (sceneSession->IsTurnScreenOn() && !PowerMgr::PowerMgrClient::GetInstance().IsScreenOn()) {
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "turn screen on");
            PowerMgr::PowerMgrClient::GetInstance().WakeupDevice();
        }
        // set ipc identity to raw
        IPCSkeleton::SetCallingIdentity(identity);
    };
    taskScheduler_->PostAsyncTask(task, "HandleTurnScreenOn");

#else
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Can not found the sub system of PowerMgr");
#endif
}

void SceneSessionManager::HandleKeepScreenOn(const sptr<SceneSession>& sceneSession, bool requireLock,
    const std::string& screenLockPrefix, std::shared_ptr<PowerMgr::RunningLock>& screenLock)
{
#ifdef POWER_MANAGER_ENABLE
    wptr<SceneSession> weakSceneSession(sceneSession);
    auto task = [this, weakSceneSession, requireLock, &screenLockPrefix, &screenLock]() {
        auto sceneSession = weakSceneSession.promote();
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "session is invalid");
            return;
        }
        if (requireLock && screenLock == nullptr) {
            // reset ipc identity
            std::string identity = IPCSkeleton::ResetCallingIdentity();
            screenLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock(
                screenLockPrefix + std::to_string(sceneSession->GetPersistentId()),
                PowerMgr::RunningLockType::RUNNINGLOCK_SCREEN);
            // set ipc identity to raw
            IPCSkeleton::SetCallingIdentity(identity);
        }
        if (screenLock == nullptr) {
            return;
        }
        auto currScreenId = sceneSession->GetSessionInfo().screenId_;
        auto sourceMode = ScreenSourceMode::SCREEN_ALONE;
        if (auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(currScreenId)) {
            sourceMode = screenSession->GetSourceMode();
        }
        bool shouldLock = requireLock && IsSessionVisibleForeground(sceneSession) && sourceMode != ScreenSourceMode::SCREEN_UNIQUE;
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE,
            "keep screen on: [%{public}s, %{public}d, %{public}d, %{public}d, %{public}d, %{public}" PRIu64
            ", %{public}d, %{public}s]", sceneSession->GetWindowName().c_str(), sceneSession->GetSessionState(),
            sceneSession->IsVisible(), requireLock, shouldLock, currScreenId, sourceMode, screenLockPrefix.c_str());
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:HandleKeepScreenOn");
        ErrCode res;
        std::string identity = IPCSkeleton::ResetCallingIdentity();
        if (shouldLock) {
            res = screenLock->Lock();
        } else {
            res = screenLock->UnLock();
        }
        // set ipc identity to raw
        IPCSkeleton::SetCallingIdentity(identity);
        if (res != ERR_OK) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "handle keep screen running lock failed: [operation: %{public}d, "
                "err: %{public}d]", requireLock, res);
        }
    };
    taskScheduler_->PostAsyncTask(task, "HandleKeepScreenOn");
#else
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Can not found the sub system of PowerMgr");
#endif
}

bool SceneSessionManager::NotifyVisibleChange(int32_t persistentId)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        return false;
    }
    HandleKeepScreenOn(sceneSession, sceneSession->IsKeepScreenOn(), WINDOW_SCREEN_LOCK_PREFIX,
                       sceneSession->keepScreenLock_);
    HandleKeepScreenOn(sceneSession, sceneSession->IsViewKeepScreenOn(), VIEW_SCREEN_LOCK_PREFIX,
                       sceneSession->viewKeepScreenLock_);
    return true;
}

WSError SceneSessionManager::SetBrightness(const sptr<SceneSession>& sceneSession, float brightness)
{
#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
    if (GetDisplayBrightness() != brightness &&
        GetFocusedSessionId() == sceneSession->GetPersistentId()) {
        PostBrightnessTask(brightness);
    }
#else
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Can not found the sub system of DisplayPowerMgr");
#endif
    brightnessSessionId_ = sceneSession->GetPersistentId();
    return WSError::WS_OK;
}

void SceneSessionManager::PostBrightnessTask(float brightness)
{
#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
    bool postTaskRet = true;
    bool isPC = systemConfig_.IsPcWindow();
    if (std::fabs(brightness - UNDEFINED_BRIGHTNESS) < std::numeric_limits<float>::min()) {
        if (!isPC) {
            auto task = [] {
                DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().RestoreBrightness();
            };
            postTaskRet = eventHandler_->PostTask(task, "DisplayPowerMgr:RestoreBrightness", 0);
        }
        SetDisplayBrightness(UNDEFINED_BRIGHTNESS); // UNDEFINED_BRIGHTNESS means system default brightness
    } else {
        auto task = [brightness, isPC] {
            if (isPC) {
                DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().SetBrightness(
                    static_cast<uint32_t>(brightness * MAX_BRIGHTNESS));
            } else {
                DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().OverrideBrightness(
                    static_cast<uint32_t>(brightness * MAX_BRIGHTNESS));
            }
        };
        postTaskRet = eventHandler_->PostTask(task, "DisplayPowerMgr:OverrideBrightness", 0);
        SetDisplayBrightness(brightness);
    }
    if (!postTaskRet) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "post task failed. task is SetBrightness");
    }
#endif
}

WSError SceneSessionManager::UpdateBrightness(int32_t persistentId)
{
#ifdef POWERMGR_DISPLAY_MANAGER_ENABLE
    if (systemConfig_.IsPcWindow()) {
        return WSError::WS_OK;
    }
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "session is invalid");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (!(sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW ||
          sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD ||
          sceneSession->GetSessionInfo().isSystem_)) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "only app main window or wallet swipe card can set brightness");
        return WSError::WS_DO_NOTHING;
    }
    auto brightness = sceneSession->GetBrightness();
    if (GetDisplayBrightness() != brightness) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Brightness: [%{public}f, %{public}f]", GetDisplayBrightness(), brightness);
    }
    if (std::fabs(brightness - UNDEFINED_BRIGHTNESS) < std::numeric_limits<float>::min()) {
        if (IsNeedUpdateBrightness(persistentId, brightness)) {
            TLOGI(WmsLogTag::WMS_ATTRIBUTE, "adjust brightness with default value");
            DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().RestoreBrightness();
            SetDisplayBrightness(UNDEFINED_BRIGHTNESS); // UNDEFINED_BRIGHTNESS means system default brightness
            brightnessSessionId_ = INVALID_WINDOW_ID;
        }
    } else {
        if (std::fabs(brightness - GetDisplayBrightness()) > std::numeric_limits<float>::min()) {
            TLOGI(WmsLogTag::WMS_ATTRIBUTE, "adjust brightness with value");
            DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().OverrideBrightness(
                static_cast<uint32_t>(brightness * MAX_BRIGHTNESS));
            SetDisplayBrightness(brightness);
        }
        brightnessSessionId_ = sceneSession->GetPersistentId();
    }
#else
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Can not found the sub system of DisplayPowerMgr");
#endif
    return WSError::WS_OK;
}

bool SceneSessionManager::IsNeedUpdateBrightness(int32_t persistentId, float brightness)
{
    if (std::fabs(brightness - GetDisplayBrightness()) < std::numeric_limits<float>::min()) {
        return false;
    }
    if (brightnessSessionId_ == persistentId) {
        return true;
    }
    auto brightnessSession = GetSceneSession(brightnessSessionId_);
    if (brightnessSession != nullptr && brightnessSession->IsSessionForeground()) {
        return false;
    }
    return true;
}

int32_t SceneSessionManager::GetCurrentUserId() const
{
    return currentUserId_;
}

void SceneSessionManager::SetDisplayBrightness(float brightness)
{
    displayBrightness_ = brightness;
}

float SceneSessionManager::GetDisplayBrightness() const
{
    return displayBrightness_;
}

WMError SceneSessionManager::SetGestureNavigationEnabled(bool enable)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_EVENT, "permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    std::string callerBundleName = SessionPermission::GetCallingBundleName();
    TLOGD(WmsLogTag::WMS_EVENT, "enable:%{public}d name:%{public}s", enable, callerBundleName.c_str());
    auto task = [this, enable, bundleName = std::move(callerBundleName)]() {
        SessionManagerAgentController::GetInstance().NotifyGestureNavigationEnabledResult(enable);
        if (!gestureNavigationEnabledChangeFunc_ && !statusBarEnabledChangeFunc_) {
            TLOGNE(WmsLogTag::WMS_EVENT, "callback func is null");
            return WMError::WM_OK;
        }
        if (gestureNavigationEnabledChangeFunc_) {
            gestureNavigationEnabledChangeFunc_(enable, bundleName, GestureBackType::GESTURE_ALL);
        }
        if (statusBarEnabledChangeFunc_) {
            statusBarEnabledChangeFunc_(enable, bundleName);
        }
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, "SetGestureNavigationEnabled");
}

WSError SceneSessionManager::SetFocusedSessionId(const int32_t persistentId, const DisplayId displayId)
{
    return windowFocusController_->UpdateFocusedSessionId(displayId, persistentId);
}

int32_t SceneSessionManager::GetFocusedSessionId(DisplayId displayId) const
{
    return windowFocusController_->GetFocusedSessionId(displayId);
}

DisplayId SceneSessionManager::GetDisplayGroupId(DisplayId displayId) const
{
    return windowFocusController_->GetDisplayGroupId(displayId);
}

std::vector<std::pair<DisplayId, int32_t>> SceneSessionManager::GetAllFocusedSessionList() const
{
    return windowFocusController_->GetAllFocusedSessionList();
}

void SceneSessionManager::GetFocusWindowInfo(FocusChangeInfo& focusInfo, DisplayId displayId)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "permission denied!");
        return;
    }
    taskScheduler_->PostSyncTask([this, &focusInfo, displayId] {
        auto focusGroup = windowFocusController_->GetFocusGroup(displayId);
        if (focusGroup == nullptr) {
            TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, displayId);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        if (auto sceneSession = GetSceneSession(focusGroup->GetFocusedSessionId())) {
            focusInfo.windowId_ = sceneSession->GetWindowId();
            focusInfo.displayId_ = focusGroup->GetDisplayGroupId() == DEFAULT_DISPLAY_ID
                                        ? DEFAULT_DISPLAY_ID
                                        : sceneSession->GetDisplayId();
            focusInfo.pid_ = sceneSession->GetCallingPid();
            focusInfo.uid_ = sceneSession->GetCallingUid();
            focusInfo.windowType_ = sceneSession->GetWindowType();
            focusInfo.abilityToken_ = sceneSession->GetAbilityToken();
            TLOGND(WmsLogTag::WMS_FOCUS, "Get focus session info success");
            return WSError::WS_OK;
        }
        return WSError::WS_ERROR_DESTROYED_OBJECT;
    }, __func__);
}

void SceneSessionManager::GetFocusWindowInfoByAbilityToken(FocusChangeInfo& focusInfo,
    const sptr<IRemoteObject>& abilityToken)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "permission denied!");
        return;
    }
    const char* const where = __func__;
    taskScheduler_->PostSyncTask([this, &focusInfo, abilityToken, where] {
        sptr<SceneSession> currSceneSession = nullptr;
        {
            std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
            for (const auto& [_, sceneSession] : sceneSessionMap_) {
                if (sceneSession == nullptr) {
                    continue;
                }
                if (sceneSession->GetAbilityToken() == abilityToken) {
                    currSceneSession = sceneSession;
                    break;
                }
            }
        }
        if (currSceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s not found scene session by ability token", where);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        auto focusGroup = windowFocusController_->GetFocusGroup(currSceneSession->GetDisplayId());
        if (focusGroup == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, currSceneSession->GetDisplayId());
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        int32_t focusedSessionId = focusGroup->GetFocusedSessionId();
        if (focusedSessionId != currSceneSession->GetPersistentId()) {
            currSceneSession = GetSceneSession(focusedSessionId);
        }
        if (currSceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s scene session is nullptr, id: %{public}d", where,
                focusedSessionId);
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        focusInfo.windowId_ = currSceneSession->GetWindowId();
        focusInfo.displayId_ = focusGroup->GetDisplayGroupId() == DEFAULT_DISPLAY_ID
                                    ? DEFAULT_DISPLAY_ID
                                    : currSceneSession->GetDisplayId();
        focusInfo.pid_ = currSceneSession->GetCallingPid();
        focusInfo.uid_ = currSceneSession->GetCallingUid();
        focusInfo.windowType_ = currSceneSession->GetWindowType();
        focusInfo.abilityToken_ = currSceneSession->GetAbilityToken();
        TLOGNI(WmsLogTag::WMS_FOCUS, "%{public}s success, id: %{public}d ", where, focusedSessionId);
        return WSError::WS_OK;
    }, where);
}

void SceneSessionManager::GetAllGroupInfo(std::unordered_map<DisplayId, DisplayGroupId>& displayId2GroupIdMap,
                                          std::vector<sptr<FocusChangeInfo>>& allFocusInfoList)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "non-SA calling, permission denied!");
        return;
    }
    taskScheduler_->PostSyncTask([this, &displayId2GroupIdMap, &allFocusInfoList, where = __func__] {
        std::unordered_map<DisplayId, DisplayGroupId> groupInfoMap = windowFocusController_->GetDisplayId2GroupIdMap();
        if (groupInfoMap.empty()) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "displayId2GroupIdMap is empty");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        displayId2GroupIdMap = groupInfoMap;
        std::unordered_map<DisplayGroupId, sptr<FocusGroup>> allFocusGroupMap;
        windowFocusController_->GetAllFocusGroup(allFocusGroupMap);
        for (const auto& elem : allFocusGroupMap) {
            auto curFocusGroup = elem.second;
            if (curFocusGroup == nullptr) {
                TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s focus group is null", where);
                continue;
            }
            auto focusedSession = GetSceneSession(curFocusGroup->GetFocusedSessionId());
            if (focusedSession == nullptr) {
                TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s focused session is null, id: %{public}d", where,
                    curFocusGroup->GetFocusedSessionId());
                continue;
            }
            auto focusInfo = sptr<FocusChangeInfo>::MakeSptr(
                focusedSession->GetWindowId(), curFocusGroup->GetDisplayGroupId() == DEFAULT_DISPLAY_ID ?
                DEFAULT_DISPLAY_ID : focusedSession->GetDisplayId(), focusedSession->GetDisplayId(),
                curFocusGroup->GetDisplayGroupId(), focusedSession->GetCallingPid(), focusedSession->GetCallingUid(),
                focusedSession->GetWindowType(), focusedSession->GetAbilityToken());
            allFocusInfoList.emplace_back(focusInfo);
        }
        if (allFocusInfoList.empty()) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "allFocusInfoList is empty");
            return WSError::WS_ERROR_DESTROYED_OBJECT;
        }
        TLOGNI(WmsLogTag::WMS_FOCUS, "%{public}s success", where);
        return WSError::WS_OK;
    }, __func__);
}

WSError SceneSessionManager::AddFocusGroup(DisplayGroupId displayGroupId, DisplayId displayId)
{
    return windowFocusController_->AddFocusGroup(displayGroupId, displayId);
}

WSError SceneSessionManager::RemoveFocusGroup(DisplayGroupId displayGroupId, DisplayId displayId)
{
    return windowFocusController_->RemoveFocusGroup(displayGroupId, displayId);
}

WSError SceneSessionManager::SendPointerEventForHover(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_EVENT, "permission denied!");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    if (pointerEvent == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "pointerEvent is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    bool isHoverDown = pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER &&
        pointerEvent->GetSourceType() == MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    if (!isHoverDown) {
        TLOGE(WmsLogTag::WMS_EVENT, "pointer event is not hover down");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    return taskScheduler_->PostSyncTask([this, &pointerEvent]() {
        int32_t windowId = pointerEvent->GetAgentWindowId();
        TLOGNI(WmsLogTag::WMS_EVENT, "windowId: %{public}d", windowId);
        auto sceneSession = GetSceneSession(windowId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "session is nullptr");
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        return sceneSession->SendPointerEventForHover(pointerEvent);
    }, __func__);
}

static bool IsValidDigitString(const std::string& windowIdStr)
{
    if (windowIdStr.empty()) {
        return false;
    }
    for (char ch : windowIdStr) {
        if (ch >= '0' && ch <= '9') {
            continue;
        }
        TLOGE(WmsLogTag::DEFAULT, "invalid window id");
        return false;
    }
    return true;
}

void SceneSessionManager::RegisterSessionExceptionFunc(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr");
        return;
    }
    sceneSession->SetSessionExceptionListener([this, where = __func__](
        const SessionInfo& info, const ExceptionInfo& exceptionInfo , bool startFail = false) {
        auto task = [this, info, where] {
            auto session = GetSceneSession(info.persistentId_);
            if (session == nullptr) {
                TLOGNW(WmsLogTag::WMS_LIFE, "%{public}s Not found session, id:%{public}d", where, info.persistentId_);
                return;
            }
            if (session->GetSessionInfo().isSystem_) {
                TLOGNW(WmsLogTag::WMS_LIFE, "%{public}s id: %{public}d is system", where, session->GetPersistentId());
                return;
            }
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s errorCode: %{public}d, id: %{public}d",
                where, info.errorCode, info.persistentId_);
            if (info.errorCode == static_cast<int32_t>(AAFwk::ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT) ||
                info.errorCode == static_cast<int32_t>(AAFwk::ErrorLifecycleState::ABILITY_STATE_FOREGROUND_TIMEOUT)) {
                TLOGND(WmsLogTag::WMS_LIFE, "NotifySessionClosed when ability load timeout "
                    "or foreground timeout, id: %{public}d", info.persistentId_);
                listenerController_->NotifySessionClosed(session->GetSessionInfo());
            }
        };
        taskScheduler_->PostVoidSyncTask(task, "sessionException");
    }, false);
    TLOGD(WmsLogTag::WMS_LIFE, "success, id: %{public}d", sceneSession->GetPersistentId());
}

void SceneSessionManager::RegisterVisibilityChangedDetectFunc(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr");
        return;
    }
    sceneSession->SetVisibilityChangedDetectFunc(
        [this](int32_t pid, bool isVisible, bool newIsVisible) THREAD_SAFETY_GUARD(SCENE_GUARD) {
        if (isVisible == newIsVisible || pid == -1) {
            return;
        }
        auto windowPidVisibilityInfo = sptr<WindowPidVisibilityInfo>::MakeSptr();
        windowPidVisibilityInfo->pid_ = pid;
        int32_t currentCount = 0;
        int32_t beforeCount = 0;
        if (visibleWindowCountMap_.find(pid) != visibleWindowCountMap_.end()) {
            beforeCount = visibleWindowCountMap_[pid];
        }
        currentCount = newIsVisible ? beforeCount + 1 : beforeCount - 1;
        visibleWindowCountMap_[pid] = currentCount;
        if (visibleWindowCountMap_[pid] == 0) {
            visibleWindowCountMap_.erase(pid);
        }
        if (beforeCount == 0 && currentCount == 1) {
            TLOGNI(WmsLogTag::WMS_LIFE, "The windows of pid %{public}d change to visibility.", pid);
            windowPidVisibilityInfo->visibilityState_ = WindowPidVisibilityState::VISIBILITY_STATE;
            SessionManagerAgentController::GetInstance().NotifyWindowPidVisibilityChanged(windowPidVisibilityInfo);
        } else if (beforeCount == 1 && currentCount == 0) {
            TLOGNI(WmsLogTag::WMS_LIFE, "The windows of pid %{public}d change to invisibility.", pid);
            windowPidVisibilityInfo->visibilityState_ = WindowPidVisibilityState::INVISIBILITY_STATE;
            SessionManagerAgentController::GetInstance().NotifyWindowPidVisibilityChanged(windowPidVisibilityInfo);
        } else if (beforeCount < 0 || currentCount < 0) {
            TLOGNE(WmsLogTag::WMS_LIFE, "The count of visible windows in same pid:%{public}d is less than 0.", pid);
            RecoveryVisibilityPidCount(pid);
        }
    });
}

void SceneSessionManager::RecoveryVisibilityPidCount(int32_t pid)
{
    int32_t count = 0;
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, session] : sceneSessionMap_) {
        if (session && session->GetCallingPid() == pid && session->IsVisible()) {
            count++;
        }
    }
    if (count > 0) {
        visibleWindowCountMap_[pid] = count;
    }
}

void SceneSessionManager::RegisterSessionSnapshotFunc(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is nullptr");
        return;
    }
    NotifySessionSnapshotFunc sessionSnapshotFunc = [this](int32_t persistentId) {
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGNW(WmsLogTag::WMS_SYSTEM, "NotifySessionSnapshotFunc, Not found session, id: %{public}d", persistentId);
            return;
        }
        if (sceneSession->GetSessionInfo().isSystem_) {
            TLOGNW(WmsLogTag::WMS_SYSTEM, "NotifySessionSnapshotFunc, id: %{public}d is system",
                sceneSession->GetPersistentId());
            return;
        }
        auto abilityInfo = sceneSession->GetSessionInfoAbilityInfo();
        if (abilityInfo == nullptr) {
            TLOGNW(WmsLogTag::WMS_SYSTEM, "NotifySessionSnapshotFunc, abilityInfo is nullptr");
            return;
        }
        if (!abilityInfo->excludeFromMissions) {
            listenerController_->NotifySessionSnapshotChanged(persistentId);
        }
    };
    sceneSession->SetSessionSnapshotListener(sessionSnapshotFunc);
    TLOGD(WmsLogTag::DEFAULT, "success, id: %{public}d", sceneSession->GetPersistentId());
}

void SceneSessionManager::BindVsyncStation(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "sceneSession is nullptr");
        return;
    }
    sceneSession->SetVsyncStation(vsyncStation_);
}

void SceneSessionManager::RegisterAcquireRotateAnimationConfigFunc(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is nullptr");
        return;
    }
    AcquireRotateAnimationConfigFunc acquireRotateAnimationConfigFunc = [this](RotateAnimationConfig& config) {
        config.duration_ = rotateAnimationConfig_.duration_;
    };
    sceneSession->SetAcquireRotateAnimationConfigFunc(acquireRotateAnimationConfigFunc);
    TLOGD(WmsLogTag::DEFAULT, "success, id: %{public}d",
        sceneSession->GetPersistentId());
}

void SceneSessionManager::CloseSyncTransaction(std::function<void()> func)
{
    auto task = [this, param = std::move(func), where = __func__] () {
        if (!closeSyncFunc_) {
            closeSyncFunc_ = std::move(param);
        }
        bool isLastFrameLayoutFinished = true;
        IsLastFrameLayoutFinished(isLastFrameLayoutFinished);
        if (isLastFrameLayoutFinished) {
            closeSyncFunc_();
        } else {
            needCloseSync_ = true;
        }
    };
    taskScheduler_->PostAsyncTask(task, __func__);
}

void SceneSessionManager::NotifySessionForCallback(const sptr<SceneSession>& sceneSession, const bool needRemoveSession,
    LifeCycleChangeReason reason)
{
    if (sceneSession == nullptr) {
        TLOGW(WmsLogTag::DEFAULT, "session is null");
        return;
    }
    if (sceneSession->GetSessionInfo().isSystem_) {
        TLOGW(WmsLogTag::DEFAULT, "id: %{public}d is system", sceneSession->GetPersistentId());
        return;
    }
    if (sceneSession->GetSessionInfo().appIndex_ != 0) {
        TLOGI(WmsLogTag::DEFAULT, "NotifyDestroy, appIndex: %{public}d, id: %{public}d",
               sceneSession->GetSessionInfo().appIndex_, sceneSession->GetPersistentId());
        listenerController_->NotifySessionLifecycleEvent(
            ISessionLifecycleListener::SessionLifecycleEvent::DESTROYED, sceneSession->GetSessionInfo(), reason);
        return;
    }
    if (needRemoveSession) {
        TLOGI(WmsLogTag::DEFAULT, "NotifyDestroy, needRemoveSession, id: %{public}d", sceneSession->GetPersistentId());
        listenerController_->NotifySessionLifecycleEvent(
            ISessionLifecycleListener::SessionLifecycleEvent::DESTROYED, sceneSession->GetSessionInfo(), reason);
        return;
    }
    if (sceneSession->GetSessionInfo().abilityInfo == nullptr) {
        TLOGW(WmsLogTag::DEFAULT, "abilityInfo is null, id: %{public}d", sceneSession->GetPersistentId());
    } else if ((sceneSession->GetSessionInfo().abilityInfo)->removeMissionAfterTerminate ||
               (sceneSession->GetSessionInfo().abilityInfo)->excludeFromMissions) {
        TLOGI(WmsLogTag::DEFAULT, "NotifyDestroy, removeMissionAfterTerminate or excludeFromMissions, id: %{public}d",
            sceneSession->GetPersistentId());
        listenerController_->NotifySessionLifecycleEvent(
            ISessionLifecycleListener::SessionLifecycleEvent::DESTROYED, sceneSession->GetSessionInfo(), reason);
        return;
    }
    TLOGI(WmsLogTag::DEFAULT, "NotifyClosed, id: %{public}d", sceneSession->GetPersistentId());
    listenerController_->NotifySessionClosed(sceneSession->GetSessionInfo(), reason);
}

void SceneSessionManager::NotifyWindowInfoChangeFromSession(int32_t persistentId)
{
    TLOGD(WmsLogTag::DEFAULT, "persistentId=%{public}d", persistentId);
    sptr<SceneSession> sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "sceneSession nullptr");
        return;
    }

    SceneInputManager::GetInstance().NotifyWindowInfoChangeFromSession(sceneSession);
}

bool SceneSessionManager::IsSessionVisible(const sptr<SceneSession>& session) const
{
    if (session == nullptr) {
        return false;
    }
    if (Session::IsScbCoreEnabled()) {
        return session->IsVisible();
    }
    const auto& state = session->GetSessionState();
    if (WindowHelper::IsSubWindow(session->GetWindowType())) {
        const auto& mainOrFloatSession = session->GetMainOrFloatSession();
        if (mainOrFloatSession == nullptr) {
            TLOGE(WmsLogTag::WMS_SUB, "Can not find parent for this sub window, id: %{public}d",
                session->GetPersistentId());
            return false;
        }
        if (session->IsVisible() || (state == SessionState::STATE_ACTIVE || state == SessionState::STATE_FOREGROUND)) {
            const auto mainOrFloatSessionState = mainOrFloatSession->GetSessionState();
            if (mainOrFloatSessionState == SessionState::STATE_INACTIVE ||
                mainOrFloatSessionState == SessionState::STATE_BACKGROUND) {
                TLOGD(WmsLogTag::WMS_SUB, "Parent of this sub window is at background, id: %{public}d",
                    session->GetPersistentId());
                return false;
            }
            TLOGD(WmsLogTag::WMS_SUB, "Sub window is at foreground, id: %{public}d", session->GetPersistentId());
            return true;
        }
        TLOGD(WmsLogTag::WMS_SUB, "Sub window is at background, id: %{public}d", session->GetPersistentId());
        return false;
    }

    if (session->IsVisible() || (state == SessionState::STATE_ACTIVE || state == SessionState::STATE_FOREGROUND)) {
        TLOGD(WmsLogTag::WMS_LIFE, "Window is at foreground, id: %{public}d", session->GetPersistentId());
        return true;
    }
    TLOGD(WmsLogTag::WMS_LIFE, "Window is at background, id: %{public}d", session->GetPersistentId());
    return false;
}

bool SceneSessionManager::IsSessionVisibleForeground(const sptr<SceneSession>& session) const
{
    if (session == nullptr) {
        return false;
    }
    if (Session::IsScbCoreEnabled()) {
        return session->IsVisibleForeground();
    }
    return IsSessionVisible(session);
}

void SceneSessionManager::DumpSessionInfo(const sptr<SceneSession>& session, std::ostringstream& oss)
{
    if (session == nullptr) {
        return;
    }
    int32_t zOrder = IsSessionVisibleForeground(session) ? static_cast<int32_t>(session->GetZOrder()) : -1;
    WSRect rect = session->GetSessionRect();
    std::string sName;
    if (session->GetSessionInfo().isSystem_) {
        sName = session->GetSessionInfo().abilityName_;
    } else {
        sName = session->GetWindowName();
    }
    uint32_t flag = session->GetSessionProperty()->GetWindowFlags();
    uint64_t displayId = session->GetSessionProperty()->GetDisplayId();
    uint32_t orientation = 0;
    const std::string& windowName = sName.size() <= WINDOW_NAME_MAX_LENGTH ?
        sName : sName.substr(0, WINDOW_NAME_MAX_LENGTH);
    // std::setw is used to set the output width and different width values are set to keep the format aligned.
    oss << std::left << std::setw(WINDOW_NAME_MAX_WIDTH) << windowName
        << std::left << std::setw(DISPLAY_NAME_MAX_WIDTH) << displayId
        << std::left << std::setw(PID_MAX_WIDTH) << session->GetCallingPid()
        << std::left << std::setw(PARENT_ID_MAX_WIDTH) << session->GetPersistentId()
        << std::left << std::setw(VALUE_MAX_WIDTH) << static_cast<uint32_t>(session->GetWindowType())
        << std::left << std::setw(VALUE_MAX_WIDTH) << static_cast<uint32_t>(session->GetWindowMode())
        << std::left << std::setw(VALUE_MAX_WIDTH) << flag
        << std::left << std::setw(VALUE_MAX_WIDTH) << zOrder
        << std::left << std::setw(ORIEN_MAX_WIDTH) << orientation
        << "[ "
        << std::left << std::setw(VALUE_MAX_WIDTH) << rect.posX_
        << std::left << std::setw(VALUE_MAX_WIDTH) << rect.posY_
        << std::left << std::setw(VALUE_MAX_WIDTH) << rect.width_
        << std::left << std::setw(VALUE_MAX_WIDTH) << rect.height_
        << "]"
        << " [ "
        << std::left << std::setw(OFFSET_MAX_WIDTH) << GetFloatWidth(OFFSET_MAX_WIDTH, session->GetOffsetX())
        << std::left << std::setw(OFFSET_MAX_WIDTH) << GetFloatWidth(OFFSET_MAX_WIDTH, session->GetOffsetY())
        << "]"
        << " [ "
        << std::left << std::setw(SCALE_MAX_WIDTH) << GetFloatWidth(SCALE_MAX_WIDTH, session->GetScaleX())
        << std::left << std::setw(SCALE_MAX_WIDTH) << GetFloatWidth(SCALE_MAX_WIDTH, session->GetScaleY())
        << std::left << std::setw(SCALE_MAX_WIDTH) << GetFloatWidth(SCALE_MAX_WIDTH, session->GetPivotX())
        << std::left << std::setw(SCALE_MAX_WIDTH) << GetFloatWidth(SCALE_MAX_WIDTH, session->GetPivotY())
        << "]"
        << std::endl;
}

std::string SceneSessionManager::GetFloatWidth(const int width, float value)
{
    std::ostringstream oss;
    oss << value;
    std::string strValue = oss.str();
    return (strValue.size() > static_cast<size_t>(width)) ? strValue.substr(0, width) : strValue;
}

void SceneSessionManager::DumpFocusInfo(std::ostringstream& oss)
{
    auto defaultFocusedSessionId = windowFocusController_->GetFocusedSessionId(DEFAULT_DISPLAY_ID);
    oss << "Focus window: " << defaultFocusedSessionId << std::endl;
    std::vector<std::pair<DisplayId, int32_t>> allFocusedSessionList =
        windowFocusController_->GetAllFocusedSessionList();
    oss << "All Focus window: " << std::endl;
    if (allFocusedSessionList.size() > 0) {
        for (const auto& focusState : allFocusedSessionList) {
            oss << "DisplayId: " << focusState.first << " WindowId: " << focusState.second << std::endl;
        }
    }
}

WSError SceneSessionManager::GetAllSessionDumpInfo(std::string& dumpInfo)
{
    std::ostringstream oss;
    oss << "-------------------------------------ScreenGroup 0"
        << "-------------------------------------" << std::endl;
    oss << "WindowName           DisplayId Pid     WinId Type Mode Flag ZOrd Orientation [ x    y    w    h    ]"
        << " [ OffsetX OffsetY ] [ ScaleX  ScaleY  PivotX  PivotY  ]" << std::endl;
    std::vector<sptr<SceneSession>> allSession;
    std::vector<sptr<SceneSession>> backgroundSession;
    std::map<int32_t, sptr<SceneSession>> sceneSessionMapCopy;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        sceneSessionMapCopy = sceneSessionMap_;
    }
    for (const auto& elem : sceneSessionMapCopy) {
        auto curSession = elem.second;
        if (curSession == nullptr) {
            TLOGD(WmsLogTag::DEFAULT, "nullptr");
            continue;
        }
        if (!curSession->GetSessionInfo().isSystem_ &&
            (curSession->GetSessionState() < SessionState::STATE_FOREGROUND ||
            curSession->GetSessionState() > SessionState::STATE_BACKGROUND)) {
            TLOGW(WmsLogTag::DEFAULT, "id:%{public}d,invalid state:%{public}u",
                 curSession->GetPersistentId(), curSession->GetSessionState());
             continue;
         }
        if (IsSessionVisibleForeground(curSession)) {
            allSession.push_back(curSession);
        } else {
            backgroundSession.push_back(curSession);
        }
    }
    allSession.insert(allSession.end(), backgroundSession.begin(), backgroundSession.end());
    uint32_t count = 0;
    for (const auto& session : allSession) {
        if (count == static_cast<uint32_t>(allSession.size() - backgroundSession.size())) {
            oss << "---------------------------------------------------------------------------------------"
                << std::endl;
        }
        DumpSessionInfo(session, oss);
        count++;
    }
    DumpFocusInfo(oss);
    oss << "SingleHand: X[" << singleHandTransform_.posX << "] Y[" << singleHandTransform_.posY << "] scale["
        << singleHandTransform_.scaleX << "]" << std::endl;
    oss << "Total window num: " << sceneSessionMapCopy.size() << std::endl;
    oss << "Highlighted windows: " << GetHighlightIdsStr() << std::endl;
    dumpInfo.append(oss.str());
    return WSError::WS_OK;
}

WSError SceneSessionManager::GetAllSessionDumpDetailInfo(std::string& dumpInfo)
{
    std::ostringstream oss;
    std::vector<sptr<SceneSession>> allSession;
    std::vector<sptr<SceneSession>> backgroundSession;

    std::map<int32_t, sptr<SceneSession>> sceneSessionMapCopy;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        sceneSessionMapCopy = sceneSessionMap_;
    }
    for (const auto& elem : sceneSessionMapCopy) {
        auto curSession = elem.second;
        if (curSession == nullptr) {
            continue;
        }
        if (IsSessionVisibleForeground(curSession)) {
            allSession.push_back(curSession);
        } else {
            backgroundSession.push_back(curSession);
        }
    }
    allSession.insert(allSession.end(), backgroundSession.begin(), backgroundSession.end());
    HidumpController::GetInstance().GetAllSessionDumpDetailedInfo(oss, allSession, backgroundSession);
    dumpInfo.append(oss.str());
    return WSError::WS_OK;
}

void SceneSessionManager::SetDumpRootSceneElementInfoListener(const DumpRootSceneElementInfoFunc& func)
{
    dumpRootSceneFunc_ = func;
}

void SceneSessionManager::DumpSessionElementInfo(const sptr<SceneSession>& session,
    const std::vector<std::string>& params, std::string& dumpInfo)
{
    std::vector<std::string> resetParams;
    resetParams.assign(params.begin() + 2, params.end()); // 2: params num
    if (resetParams.empty()) {
        TLOGND(WmsLogTag::DEFAULT, "do not dump ui info");
        return;
    }

    if (!session->GetSessionInfo().isSystem_) {
        TLOGD(WmsLogTag::DEFAULT, "Dump normal session, not system");
        dumpInfoFuture_.ResetLock({});
        session->DumpSessionElementInfo(resetParams);
        std::vector<std::string> infos = dumpInfoFuture_.GetResult(2000); // 2000: wait for 2000ms
        for (auto& info: infos) {
            dumpInfo.append(info).append("\n");
        }
    } else {
        TLOGD(WmsLogTag::DEFAULT, "Dump system session");
        std::vector<std::string> infos;
        dumpRootSceneFunc_(session, resetParams, infos);
        for (auto& info: infos) {
            dumpInfo.append(info).append("\n");
        }
    }
}

WSError SceneSessionManager::GetSpecifiedSessionDumpInfo(std::string& dumpInfo, const std::vector<std::string>& params,
    const std::string& strId)
{
    uint64_t persistentId = std::stoull(strId);
    auto session = GetSceneSession(persistentId);
    if (session == nullptr) {
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    auto sessionProperty = session->GetSessionProperty();
    WSRect rect = session->GetSessionRect();
    std::string isVisible = session->IsVisible() ? "true" : "false";
    std::string focusable = session->GetFocusable() ? "true" : "false";
    std::string decoStatus = sessionProperty->IsDecorEnable() ? "true" : "false";
    bool privacyMode = sessionProperty->GetSystemPrivacyMode() || sessionProperty->GetPrivacyMode();
    std::string isPrivacyMode = privacyMode ? "true" : "false";
    bool isFirstFrameAvailable = true;
    std::ostringstream oss;
    oss << "WindowName: " << session->GetWindowName()  << std::endl;
    oss << "DisplayId: " << session->GetSessionProperty()->GetDisplayId() << std::endl;
    oss << "WinId: " << session->GetPersistentId() << std::endl;
    oss << "Pid: " << session->GetCallingPid() << std::endl;
    oss << "Type: " << static_cast<uint32_t>(session->GetWindowType()) << std::endl;
    oss << "Mode: " << static_cast<uint32_t>(session->GetWindowMode()) << std::endl;
    oss << "Flag: " << sessionProperty->GetWindowFlags() << std::endl;
    oss << "Orientation: " << static_cast<uint32_t>(session->GetRequestedOrientation()) << std::endl;
    oss << "FirstFrameCallbackCalled: " << isFirstFrameAvailable << std::endl;
    oss << "IsVisible: " << isVisible << std::endl;
    oss << "isRSVisible: " << (session->GetRSVisible() ? "true" : "false") << std::endl;
    oss << "Focusable: "  << focusable << std::endl;
    oss << "DecoStatus: "  << decoStatus << std::endl;
    oss << "isPrivacyMode: "  << isPrivacyMode << std::endl;
    oss << "WindowRect: " << "[ "
        << rect.posX_ << ", " << rect.posY_ << ", " << rect.width_ << ", " << rect.height_
        << " ]" << std::endl;
    oss << "scaleX: " << session->GetScaleX() << std::endl;
    oss << "scaleY: " << session->GetScaleY() << std::endl;
    oss << "Offset: " << "[ "
        << session->GetOffsetX() << ", " << session->GetOffsetY() << " ]" << std::endl;
    oss << "Scale: " << "[ "
        << session->GetScaleX() << ", " << session->GetScaleY() << ", "
        << session->GetPivotX() << ", " << session->GetPivotY()
        << " ]" << std::endl;
    oss << "ParentWindowId: " << session->GetParentPersistentId() << std::endl;
    dumpInfo.append(oss.str());

    DumpSessionElementInfo(session, params, dumpInfo);
    return WSError::WS_OK;
}

WSError SceneSessionManager::GetSCBDebugDumpInfo(std::string&& cmd, std::string& dumpInfo)
{
    const std::string writeFileCmd = "-f";
    bool isWriteFile = false;
    std::string filePath = "";
    // 2: writeFileCmd length
    if (cmd.size() >= 2 && (cmd.substr(cmd.size() - 2, 2) == writeFileCmd)) {
        isWriteFile = true;
    }
    {
        auto rootContext = rootSceneContextWeak_.lock();
        if (isWriteFile && rootContext != nullptr) {
            auto dumpTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            filePath = rootContext->GetFilesDir() + "/wms_" + std::to_string(dumpTimeStamp) + ".dump";
            size_t pos = cmd.find(writeFileCmd);
            cmd.erase(pos, writeFileCmd.length());
        }
    }
    // publish data
    TLOGD(WmsLogTag::DEFAULT, "filePath: %{public}s, cmd: %{public}s", filePath.c_str(), cmd.c_str());
    bool ret = eventHandler_->PostSyncTask(
        [this, filePath = std::move(filePath), cmd = std::move(cmd)] {
            return scbDumpSubscriber_->Publish(cmd, filePath);
        }, "PublishSCBDumper");
    if (!ret) {
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    // get response event
    auto task = [this, &dumpInfo] {
        dumpInfo.append(scbDumpSubscriber_->GetDebugDumpInfo(WAIT_TIME));
        return WSError::WS_OK;
    };
    eventHandler_->PostSyncTask(task, "GetDataSCBDumper");
    return WSError::WS_OK;
}

void SceneSessionManager::NotifyDumpInfoResult(const std::vector<std::string>& info)
{
    dumpInfoFuture_.SetValue(info);
    TLOGD(WmsLogTag::DEFAULT, "NotifyDumpInfoResult");
}

WSError SceneSessionManager::GetSessionDumpInfo(const std::vector<std::string>& params, std::string& dumpInfo)
{
    if (!(SessionPermission::IsSACalling() || SessionPermission::IsStartByHdcd())) {
        TLOGE(WmsLogTag::DEFAULT, "GetSessionDumpInfo permission denied!");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }

    if (params.size() == 1 && params[0] == ARG_DUMP_ALL) { // 1: params num
        return GetAllSessionDumpInfo(dumpInfo);
    }
    if (params.size() == 1 && params[0] == ARG_DUMP_DETAIL) { // 1: params num
        return GetAllSessionDumpDetailInfo(dumpInfo);
    }
    if (params.size() >= 2 && params[0] == ARG_DUMP_WINDOW && IsValidDigitString(params[1])) { // 2: params num
        return GetSpecifiedSessionDumpInfo(dumpInfo, params, params[1]);
    }
    if (params.size() >= 2 && params[0] == ARG_DUMP_SCB) { // 2: params num
        std::string cmd;
        std::for_each(params.begin() + 1, params.end(),
                        [&cmd](const std::string& value) {
                            cmd += value;
                            cmd += ' ';
                        });
        return GetSCBDebugDumpInfo(std::move(cmd), dumpInfo);
    }
    if (params.size() >= 1 && params[0] == ARG_DUMP_PIPLINE) { // 1: params num
        return GetTotalUITreeInfo(dumpInfo);
    }
    if (params.size() >= 2 && params[0] == ARG_DUMP_RECORD) { // 2: params num
        std::vector<std::string> resetParams;
        resetParams.assign(params.begin() + 1, params.end());
        SessionChangeRecorder::GetInstance().GetSceneSessionNeedDumpInfo(resetParams, dumpInfo);
        return WSError::WS_OK;
    }
    return WSError::WS_ERROR_INVALID_OPERATION;
}

WSError SceneSessionManager::GetTotalUITreeInfo(std::string& dumpInfo)
{
    TLOGI(WmsLogTag::WMS_PIPELINE, "begin");
    if (dumpUITreeFunc_) {
        dumpUITreeFunc_(dumpInfo);
    } else {
        TLOGE(WmsLogTag::WMS_PIPELINE, "dumpUITreeFunc is null");
    }
    return WSError::WS_OK;
}

void SceneSessionManager::SetDumpUITreeFunc(const DumpUITreeFunc& func)
{
    dumpUITreeFunc_ = func;
}

void SceneSessionManager::SetOnFlushUIParamsFunc(OnFlushUIParamsFunc&& func)
{
    onFlushUIParamsFunc_ = std::move(func);
}

void SceneSessionManager::SetIsRootSceneLastFrameLayoutFinishedFunc(IsRootSceneLastFrameLayoutFinishedFunc&& func)
{
    isRootSceneLastFrameLayoutFinishedFunc_ = std::move(func);
}

void SceneSessionManager::SetStatusBarDefaultVisibilityPerDisplay(DisplayId displayId, bool visible)
{
    taskScheduler_->PostAsyncTask([this, displayId, visible] {
        statusBarDefaultVisibilityPerDisplay_[displayId] = visible;
        TLOGNI(WmsLogTag::WMS_IMMS, "set default visibility, "
            "display id %{public}" PRIu64 " visible %{public}d", displayId, visible);
    }, __func__);
}

void SceneSessionManager::SetFindScenePanelRsNodeByZOrderFunc(FindScenePanelRsNodeByZOrderFunc&& func)
{
    findScenePanelRsNodeByZOrderFunc_ = std::move(func);
}

bool SceneSessionManager::GetStatusBarDefaultVisibilityByDisplayId(DisplayId displayId)
{
    return statusBarDefaultVisibilityPerDisplay_.count(displayId) != 0 ?
           statusBarDefaultVisibilityPerDisplay_[displayId] : true;
}

void FocusIDChange(int32_t persistentId, const sptr<SceneSession>& sceneSession)
{
    // notify RS
    TLOGD(WmsLogTag::WMS_FOCUS, "current focus session: windowId: %{public}d, windowName: %{public}s, bundleName: %{public}s, "
        "abilityName: %{public}s, pid: %{public}d, uid: %{public}d", persistentId,
        sceneSession->GetSessionProperty()->GetWindowName().c_str(),
        sceneSession->GetSessionInfo().bundleName_.c_str(),
        sceneSession->GetSessionInfo().abilityName_.c_str(),
        sceneSession->GetCallingPid(), sceneSession->GetCallingUid());
    uint64_t focusNodeId = 0; // 0 means invalid
    if (sceneSession->GetSurfaceNode() == nullptr) {
        TLOGW(WmsLogTag::WMS_FOCUS, "focused window surfaceNode is null");
    } else {
        focusNodeId = sceneSession->GetSurfaceNode()->GetId();
    }
    FocusAppInfo appInfo = {
        sceneSession->GetCallingPid(), sceneSession->GetCallingUid(),
        sceneSession->GetSessionInfo().bundleName_,
        sceneSession->GetSessionInfo().abilityName_, focusNodeId};
    RSInterfaces::GetInstance().SetFocusAppInfo(appInfo);
}

// ordered vector by compare func
std::vector<std::pair<int32_t, sptr<SceneSession>>> SceneSessionManager::GetSceneSessionVector(CmpFunc cmp)
{
    std::vector<std::pair<int32_t, sptr<SceneSession>>> ret;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (auto& iter : sceneSessionMap_) {
            ret.push_back(iter);
        }
    }
    std::sort(ret.begin(), ret.end(), cmp);
    return ret;
}

void SceneSessionManager::TraverseSessionTree(TraverseFunc func, bool isFromTopToBottom)
{
    if (isFromTopToBottom) {
        TraverseSessionTreeFromTopToBottom(func);
    } else {
        TraverseSessionTreeFromBottomToTop(func);
    }
    return;
}

void SceneSessionManager::TraverseSessionTreeFromTopToBottom(TraverseFunc func)
{
    CmpFunc cmp = [](std::pair<int32_t, sptr<SceneSession>>& lhs, std::pair<int32_t, sptr<SceneSession>>& rhs) {
        uint32_t lhsZOrder = lhs.second != nullptr ? lhs.second->GetZOrder() : 0;
        uint32_t rhsZOrder = rhs.second != nullptr ? rhs.second->GetZOrder() : 0;
        return lhsZOrder < rhsZOrder;
    };
    auto sceneSessionVector = GetSceneSessionVector(cmp);

    for (auto iter = sceneSessionVector.rbegin(); iter != sceneSessionVector.rend(); ++iter) {
        auto session = iter->second;
        if (session == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "session is nullptr");
            continue;
        }
        if (func(session)) {
            return;
        }
    }
    return;
}

void SceneSessionManager::TraverseSessionTreeFromBottomToTop(TraverseFunc func)
{
    // std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    CmpFunc cmp = [](std::pair<int32_t, sptr<SceneSession>>& lhs, std::pair<int32_t, sptr<SceneSession>>& rhs) {
        uint32_t lhsZOrder = lhs.second != nullptr ? lhs.second->GetZOrder() : 0;
        uint32_t rhsZOrder = rhs.second != nullptr ? rhs.second->GetZOrder() : 0;
        return lhsZOrder < rhsZOrder;
    };
    auto sceneSessionVector = GetSceneSessionVector(cmp);
    // std::map<int32_t, sptr<SceneSession>>::iterator iter;
    for (auto iter = sceneSessionVector.begin(); iter != sceneSessionVector.end(); ++iter) {
        auto session = iter->second;
        if (session == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "session is nullptr");
            continue;
        }
        if (func(session)) {
            return;
        }
    }
    return;
}

WMError SceneSessionManager::RequestFocusStatus(int32_t persistentId, bool isFocused, bool byForeground,
    FocusChangeReason reason)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "id: %{public}d, reason: %{public}d", persistentId, reason);
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "sceneSession is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    if (callingPid != sceneSession->GetCallingPid() &&
        !SessionPermission::IsSameAppAsCalling(SCENE_BOARD_BUNDLE_NAME, SCENE_BOARD_APP_IDENTIFIER)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "permission denied, not call by the same process");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    auto task = [this, persistentId, isFocused, byForeground, reason]() {
        if (isFocused) {
            RequestSessionFocus(persistentId, byForeground, reason);
        } else {
            RequestSessionUnfocus(persistentId, reason);
        }
    };
    taskScheduler_->PostAsyncTask(task, "RequestFocusStatus" + std::to_string(persistentId));
    focusChangeReason_ = reason;
    return WMError::WM_OK;
}

WMError SceneSessionManager::RequestFocusStatusBySCB(int32_t persistentId, bool isFocused, bool byForeground,
    FocusChangeReason reason, DisplayId displayId)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "id: %{public}d, reason: %{public}d, displayId: %{public}" PRIu64, persistentId, reason, displayId);
    auto task = [this, persistentId, isFocused, byForeground, reason, displayId]() {
        if (isFocused) {
            if (reason == FocusChangeReason::FOREGROUND) {
                RequestSessionFocusImmediately(persistentId);
                return;
            }
            if (reason == FocusChangeReason::MOVE_UP) {
                auto session = GetSceneSession(persistentId);
                if (session && !session->IsFocused()) {
                    PostProcessFocusState state = { true, true, byForeground, reason };
                    session->SetPostProcessFocusState(state);
                }
                return;
            }
            // need modifying the RequestFocusReason in SCBSceneSession.onClick() before remove this
            if (reason == FocusChangeReason::CLICK) {
                return;
            }
            if (RequestSessionFocus(persistentId, byForeground, reason) != WSError::WS_OK) {
                auto session = GetSceneSession(persistentId);
                if (session && !session->IsFocused()) {
                    PostProcessFocusState state = { true, true, byForeground, reason };
                    session->SetPostProcessFocusState(state);
                }
            }
        } else {
            RequestSessionUnfocus(persistentId, reason, displayId);
        }
    };
    taskScheduler_->PostAsyncTask(task, "RequestFocusStatusBySCB" + std::to_string(persistentId));
    return WMError::WM_OK;
}

WMError SceneSessionManager::RequestFocusStatusBySA(int32_t persistentId, bool isFocused,
    bool byForeground, FocusChangeReason reason)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "id: %{public}d, reason: %{public}d", persistentId, reason);
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "permission denied, only support SA calling.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "sceneSession is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto task = [this, persistentId, isFocused, byForeground, reason]() {
        if (isFocused) {
            RequestSessionFocus(persistentId, byForeground, reason);
        } else {
            RequestSessionUnfocus(persistentId, reason);
        }
    };
    taskScheduler_->PostAsyncTask(task, "RequestFocusOnPreviousWindow" + std::to_string(persistentId));
    return WMError::WM_OK;
}

void SceneSessionManager::RequestAllAppSessionUnfocus()
{
    auto task = [this]() {
        RequestAllAppSessionUnfocusInner();
    };
    taskScheduler_->PostAsyncTask(task, "RequestAllAppSessionUnfocus");
    return;
}

/**
 * request focus and ignore its state
 * only used when app main window start before foreground
 */
WSError SceneSessionManager::RequestSessionFocusImmediately(int32_t persistentId, bool blockNotifyUntilVisible)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "id: %{public}d, blockNotify: %{public}d", persistentId, blockNotifyUntilVisible);
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[WMSComm]session is nullptr");
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    auto focusGroup = windowFocusController_->GetFocusGroup(displayId);
    if (focusGroup == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, displayId);
        return WSError::WS_ERROR_NULLPTR;
    }
    // base block
    WSError basicCheckRet = RequestFocusBasicCheck(persistentId, focusGroup);
    if (basicCheckRet != WSError::WS_OK) {
        return basicCheckRet;
    }
    if (!sceneSession->CheckFocusable()) {
        TLOGD(WmsLogTag::WMS_FOCUS, "session is not focusable!");
        return WSError::WS_DO_NOTHING;
    }
    if (!sceneSession->IsFocusedOnShow()) {
        TLOGD(WmsLogTag::WMS_FOCUS, "session is not focused on show!");
        return WSError::WS_DO_NOTHING;
    }

    // specific block
    FocusChangeReason reason = FocusChangeReason::SCB_START_APP;
    WSError specificCheckRet = RequestFocusSpecificCheck(displayId, sceneSession, true, reason);
    if (specificCheckRet != WSError::WS_OK) {
        return specificCheckRet;
    }
    auto needBlockNotifyFocusStatusUntilForeground = focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground();
    focusGroup->SetNeedBlockNotifyUnfocusStatus(needBlockNotifyFocusStatusUntilForeground);
    if (!sceneSession->GetSessionInfo().isSystem_ && !blockNotifyUntilVisible && systemConfig_.isSupportPCMode()) {
        if (!sceneSession->IsSessionForeground()) {
            focusGroup->SetNeedBlockNotifyFocusStatusUntilForeground(true);
        }
    } else if (!sceneSession->GetSessionInfo().isSystem_ && !IsSessionVisibleForeground(sceneSession)) {
        focusGroup->SetNeedBlockNotifyFocusStatusUntilForeground(true);
    }
    ShiftFocus(displayId, sceneSession, false, reason);
    return WSError::WS_OK;
}

WSError SceneSessionManager::RequestSessionFocus(int32_t persistentId, bool byForeground, FocusChangeReason reason)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "id: %{public}d, by foreground: %{public}d, reason: %{public}d",
        persistentId, byForeground, reason);
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "[WMSComm]session is nullptr");
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    auto focusGroup = windowFocusController_->GetFocusGroup(displayId);
    if (focusGroup == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, displayId);
        return WSError::WS_ERROR_NULLPTR;
    }
    WSError checkRet = RequestSessionFocusCheck(sceneSession, focusGroup, persistentId, byForeground, reason);
    if (checkRet != WSError::WS_OK) {
        return checkRet;
    }
    focusGroup->SetNeedBlockNotifyUnfocusStatus(focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground());
    focusGroup->SetNeedBlockNotifyFocusStatusUntilForeground(false);
    ShiftFocus(displayId, sceneSession, false, reason);
    return WSError::WS_OK;
}

WSError SceneSessionManager::RequestSessionUnfocus(int32_t persistentId, FocusChangeReason reason, DisplayId displayId)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "id: %{public}d, displayId: %{public}" PRIu64, persistentId, displayId);
    if (persistentId == INVALID_SESSION_ID) {
        TLOGE(WmsLogTag::WMS_FOCUS, "id is invalid: %{public}d", persistentId);
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "session is nullptr: %{public}d", persistentId);
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    displayId = displayId != DISPLAY_ID_INVALID ? displayId :
        sceneSession->GetSessionProperty()->GetDisplayId();
    auto focusGroup = windowFocusController_->GetFocusGroup(displayId);
    if (focusGroup == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, displayId);
        return WSError::WS_ERROR_NULLPTR;
    }
    auto focusedSession = GetSceneSession(focusGroup->GetFocusedSessionId());
    if (persistentId != focusGroup->GetFocusedSessionId() &&
        !(focusedSession && focusedSession->GetParentPersistentId() == persistentId)) {
        TLOGD(WmsLogTag::WMS_FOCUS, "session unfocused!");
        return WSError::WS_DO_NOTHING;
    }
    // if pop menu created by desktop request unfocus, back to desktop
    auto lastSession = GetSceneSession(focusGroup->GetLastFocusedSessionId());
    if (focusedSession && focusedSession->GetWindowType() == WindowType::WINDOW_TYPE_SYSTEM_FLOAT &&
        lastSession && lastSession->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP &&
        RequestSessionFocus(focusGroup->GetLastFocusedSessionId(), false) == WSError::WS_OK) {
            TLOGD(WmsLogTag::WMS_FOCUS, "focus is back to desktop");
            return WSError::WS_OK;
    }
    auto nextSession = GetNextFocusableSession(displayId, persistentId);
    if (nextSession == nullptr) {
        DumpAllSessionFocusableInfo(persistentId);
    }
    focusGroup->SetNeedBlockNotifyUnfocusStatus(focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground());
    focusGroup->SetNeedBlockNotifyFocusStatusUntilForeground(false);

    if (CheckLastFocusedAppSessionFocus(focusedSession, nextSession)) {
        return WSError::WS_OK;
    }
    if (nextSession && !nextSession->IsSessionForeground() && !nextSession->GetSessionInfo().isSystem_) {
        focusGroup->SetNeedBlockNotifyFocusStatusUntilForeground(true);
    }
    return ShiftFocus(displayId, nextSession, true, reason);
}

WSError SceneSessionManager::RequestAllAppSessionUnfocusInner()
{
    TLOGI(WmsLogTag::WMS_FOCUS, "in");
    auto focusGroup = windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    if (focusGroup == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, DEFAULT_DISPLAY_ID);
        return WSError::WS_ERROR_NULLPTR;
    }
    auto focusedSession = GetSceneSession(focusGroup->GetFocusedSessionId());
    if (!focusedSession) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focused session is null");
        return WSError::WS_DO_NOTHING;
    }
    if (!focusedSession->IsAppSession()) {
        TLOGW(WmsLogTag::WMS_FOCUS, "Focused session is non app: %{public}d", focusGroup->GetFocusedSessionId());
        return WSError::WS_DO_NOTHING;
    }
    auto nextSession = GetTopFocusableNonAppSession();

    focusGroup->SetNeedBlockNotifyUnfocusStatus(focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground());
    focusGroup->SetNeedBlockNotifyFocusStatusUntilForeground(false);
    return ShiftFocus(DEFAULT_DISPLAY_ID, nextSession, true, FocusChangeReason::WIND);
}

WSError SceneSessionManager::RequestSessionFocusCheck(const sptr<SceneSession>& sceneSession,
    const sptr<FocusGroup>& focusGroup, int32_t persistentId, bool byForeground, FocusChangeReason reason)
{
    if (reason == FocusChangeReason::REQUEST_WITH_CHECK_SUB_WINDOW &&
        CheckRequestFocusSubWindowImmediately(sceneSession)) {
        TLOGD(WmsLogTag::WMS_FOCUS, "sub window focused");
        return WSError::WS_DO_NOTHING;
    }
    WSError basicCheckRet = RequestFocusBasicCheck(persistentId, focusGroup);
    if (basicCheckRet != WSError::WS_OK) {
        return basicCheckRet;
    }
    if (!sceneSession->CheckFocusable() || !IsSessionVisibleForeground(sceneSession)) {
        TLOGD(WmsLogTag::WMS_FOCUS, "session is not focusable or not visible!");
        return WSError::WS_DO_NOTHING;
    }
    if (!sceneSession->IsFocusedOnShow()) {
        TLOGD(WmsLogTag::WMS_FOCUS, "session is not focused on show!");
        return WSError::WS_DO_NOTHING;
    }
    if (!sceneSession->IsFocusableOnShow() &&
        (reason == FocusChangeReason::FOREGROUND || reason == FocusChangeReason::APP_FOREGROUND)) {
        TLOGD(WmsLogTag::WMS_FOCUS, "session is not focusable on show!");
        return WSError::WS_DO_NOTHING;
    }
    // subwindow/dialog state block
    if ((WindowHelper::IsSubWindow(sceneSession->GetWindowType()) ||
        sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) &&
        GetSceneSession(sceneSession->GetParentPersistentId()) &&
        !IsSessionVisibleForeground(GetSceneSession(sceneSession->GetParentPersistentId()))) {
            TLOGD(WmsLogTag::WMS_FOCUS, "parent session id: %{public}d is not visible!",
                sceneSession->GetParentPersistentId());
            return WSError::WS_DO_NOTHING;
    }
    // specific block
    WSError specificCheckRet = RequestFocusSpecificCheck(sceneSession->GetSessionProperty()->GetDisplayId(),
        sceneSession, byForeground, reason);
    if (specificCheckRet != WSError::WS_OK) {
        return specificCheckRet;
    }
    return WSError::WS_OK;
}

WSError SceneSessionManager::RequestFocusBasicCheck(int32_t persistentId, const sptr<FocusGroup>& focusGroup)
{
    // basic focus rule
    if (persistentId == INVALID_SESSION_ID) {
        TLOGE(WmsLogTag::WMS_FOCUS, "id is invalid!");
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (focusGroup == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (persistentId == focusGroup->GetFocusedSessionId()) {
        TLOGD(WmsLogTag::WMS_FOCUS, "request id has been focused!");
        return WSError::WS_DO_NOTHING;
    }
    return WSError::WS_OK;
}

/**
 * @note @window.focus
 * When high zOrder System Session unfocus, check if the last focused app window can focus.
 */
bool SceneSessionManager::CheckLastFocusedAppSessionFocus(const sptr<SceneSession>& focusedSession,
    const sptr<SceneSession>& nextSession)
{
    if (focusedSession == nullptr || nextSession == nullptr) {
        return false;
    }
    auto displayId = focusedSession->GetSessionProperty()->GetDisplayId();
    auto focusGroup = windowFocusController_->GetFocusGroup(displayId);
    if (focusGroup == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, displayId);
        return false;
    }
    auto lastFocusedAppSessionId = focusGroup->GetLastFocusedAppSessionId();
    TLOGI(WmsLogTag::WMS_FOCUS, "last=%{public}d, next=%{public}d",
        lastFocusedAppSessionId, nextSession->GetPersistentId());

    if (lastFocusedAppSessionId == INVALID_SESSION_ID || nextSession->GetPersistentId() == lastFocusedAppSessionId) {
        return false;
    }

    if (!focusedSession->IsSystemSessionAboveApp()) {
        return false;
    }

    auto mode = nextSession->GetWindowMode();
    // only when next session is app, and in split or floation
    if (nextSession->IsAppSession() &&
        (mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY || mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY ||
         mode == WindowMode::WINDOW_MODE_FLOATING)) {
        if (RequestSessionFocus(lastFocusedAppSessionId, false, FocusChangeReason::LAST_FOCUSED_APP) ==
            WSError::WS_OK) {
            return true;
        }
        focusGroup->SetLastFocusedAppSessionId(INVALID_SESSION_ID);
    }
    return false;
}

/**
 * When switching focus, check if the blockingType window has been  traversed downwards.
 *
 * @return true: traversed downwards, false: not.
 */
bool SceneSessionManager::CheckFocusIsDownThroughBlockingType(const sptr<SceneSession>& requestSceneSession,
    const sptr<SceneSession>& focusedSession, bool includingAppSession)
{
    uint32_t requestSessionZOrder = requestSceneSession->GetZOrder();
    uint32_t focusedSessionZOrder = focusedSession->GetZOrder();
    auto displayId = requestSceneSession->GetSessionProperty()->GetDisplayId();
    TLOGD(WmsLogTag::WMS_FOCUS, "requestSessionZOrder: %{public}d, focusedSessionZOrder: %{public}d",
        requestSessionZOrder, focusedSessionZOrder);
    if  (requestSessionZOrder < focusedSessionZOrder)  {
        auto topNearestBlockingFocusSession = GetTopNearestBlockingFocusSession(displayId, requestSessionZOrder,
            includingAppSession);
        uint32_t topNearestBlockingZOrder = 0;
        if  (topNearestBlockingFocusSession)  {
            topNearestBlockingZOrder = topNearestBlockingFocusSession->GetZOrder();
            TLOGI(WmsLogTag::WMS_FOCUS,
                  "requestSessionZOrder: %{public}d, focusedSessionZOrder: %{public}d\
                  topNearestBlockingZOrder:  %{public}d, topNearestBlockingId: %{public}d",
                  requestSessionZOrder, focusedSessionZOrder,
                  topNearestBlockingZOrder, topNearestBlockingFocusSession->GetPersistentId());
        }
        if  (focusedSessionZOrder >=  topNearestBlockingZOrder && requestSessionZOrder < topNearestBlockingZOrder)  {
            TLOGD(WmsLogTag::WMS_FOCUS,  "focus pass through, needs to be intercepted");
            return true;
        }
    }
    TLOGD(WmsLogTag::WMS_FOCUS, "not through");
    return false;
}

bool SceneSessionManager::CheckTopmostWindowFocus(const sptr<SceneSession>& focusedSession,
    const sptr<SceneSession>& sceneSession)
{
    bool isFocusedMainSessionTopmost =
        focusedSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW && focusedSession->IsTopmost();
    auto parentSession = GetSceneSession(focusedSession->GetParentPersistentId());
    bool isFocusedSessionParentTopmost = parentSession &&
        parentSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW && parentSession->IsTopmost();
    if ((isFocusedMainSessionTopmost || isFocusedSessionParentTopmost) && sceneSession->IsAppSession() &&
        (sceneSession->GetMissionId() != focusedSession->GetMissionId())) {
        return true;
    }
    return false;
}

bool SceneSessionManager::CheckRequestFocusImmediately(const sptr<SceneSession>& sceneSession)
{
    if ((sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW ||
         (SessionHelper::IsSubWindow(sceneSession->GetWindowType()) && !sceneSession->IsModal())) &&
        (ProcessModalTopmostRequestFocusImmediately(sceneSession) == WSError::WS_OK ||
         ProcessDialogRequestFocusImmediately(sceneSession) == WSError::WS_OK)) {
        TLOGD(WmsLogTag::WMS_FOCUS, "dialog or modal subwindow get focused");
        return true;
    }
    return false;
}

bool SceneSessionManager::CheckRequestFocusSubWindowImmediately(const sptr<SceneSession>& sceneSession)
{
    if ((sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW ||
         SessionHelper::IsSubWindow(sceneSession->GetWindowType())) &&
        (ProcessSubWindowRequestFocusImmediately(sceneSession) == WSError::WS_OK ||
         ProcessDialogRequestFocusImmediately(sceneSession) == WSError::WS_OK)) {
        TLOGD(WmsLogTag::WMS_FOCUS, "dialog or sub window get focused");
        return true;
    }
    return false;
}

bool SceneSessionManager::CheckClickFocusIsDownThroughFullScreen(const sptr<SceneSession>& focusedSession,
    const sptr<SceneSession>& sceneSession, FocusChangeReason reason)
{
    if (reason != FocusChangeReason::CLICK) {
        return false;
    }
    if (focusedSession == nullptr || sceneSession == nullptr) {
        TLOGW(WmsLogTag::WMS_FOCUS, "session is null");
        return false;
    }
    if (focusedSession->GetDisplayId() != sceneSession->GetDisplayId()) {
        TLOGD(WmsLogTag::WMS_FOCUS, "click on the different screen");
        return false;
    }
    if (IsBlockingFocusWindowType(focusedSession)) {
        return sceneSession->GetZOrder() < focusedSession->GetZOrder();
    }
    return false;
}

bool SceneSessionManager::IsBlockingFocusWindowType(const sptr<SceneSession>& sceneSession) const
{
    if (sceneSession == nullptr) {
        TLOGD(WmsLogTag::WMS_FOCUS, "session is nullptr");
        return false;
    }
    if (!(systemConfig_.IsPhoneWindow() || systemConfig_.IsPadWindow())) {
        TLOGD(WmsLogTag::WMS_FOCUS, "device type unmatched");
        return false;
    }
    bool blockingFocus = sceneSession->GetBlockingFocus();
    WindowType windowType = sceneSession->GetWindowType();
    bool isBlockingSystemWindowType =
        blockingFocus && (windowType == WindowType::WINDOW_TYPE_PANEL ||
        windowType == WindowType::WINDOW_TYPE_GLOBAL_SEARCH ||
        windowType == WindowType::WINDOW_TYPE_NEGATIVE_SCREEN);
    if (!isBlockingSystemWindowType && !WindowHelper::IsMainWindow(windowType)) {
        TLOGD(WmsLogTag::WMS_FOCUS, "not blocking focus window type");
        return false;
    }
    //Get height and width of current screen
    uint64_t displayId = sceneSession->GetDisplayId();
    auto display = DisplayManager::GetInstance().GetDisplayById(displayId);
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "get display object failed of display: %{public}" PRIu64, displayId);
        return false;
    }
    auto displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "get display info failed of display: %{public}" PRIu64, displayId);
        return false;
    }
    std::vector<MMI::Rect> touchHotAreas;
    std::vector<MMI::Rect> pointerHotAreas;
    SceneInputManager::GetInstance().UpdateHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
    auto posX = sceneSession->GetSessionRect().posX_;
    auto posY = sceneSession->GetSessionRect().posY_;
    for (const auto& area : touchHotAreas) {
        auto hotAreaWidth = ceil(posX + area.width * sceneSession->GetScaleX());
        auto hotAreaHeight = ceil(posY + area.height * sceneSession->GetScaleY());
        if (posX + area.x <= 0 && hotAreaWidth >= displayInfo->GetWidth() && posY + area.y <= 0 &&
            hotAreaHeight >= displayInfo->GetHeight()) {
            TLOGI(WmsLogTag::WMS_FOCUS, "current session is full-screen, screen w: %{public}d, h: %{public}d, "
                "window x: %{public}d, y: %{public}d, w: %{public}f, h: %{public}f", displayInfo->GetWidth(),
                displayInfo->GetHeight(), area.x, area.y, hotAreaWidth, hotAreaHeight);
            return true;
        }
    }
    return false;
}

WSError SceneSessionManager::RequestFocusSpecificCheck(DisplayId displayId, const sptr<SceneSession>& sceneSession,
    bool byForeground, FocusChangeReason reason)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "FocusChangeReason: %{public}d", reason);
    int32_t persistentId = sceneSession->GetPersistentId();
    if (sceneSession->GetForceHideState() != ForceHideState::NOT_HIDDEN) {
        TLOGD(WmsLogTag::WMS_FOCUS, "the window hide id: %{public}d", persistentId);
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    // dialog get focus
    if (CheckRequestFocusImmediately(sceneSession)) {
        return WSError::WS_DO_NOTHING;
    }
    // blocking-type session will block lower zOrder request focus
    auto focusedSessionId = windowFocusController_->GetFocusedSessionId(displayId);
    auto focusedSession = GetSceneSession(focusedSessionId);
    if (focusedSession) {
        TLOGD(WmsLogTag::WMS_FOCUS, "reason: %{public}d, byForeground: %{public}d",  reason,
            byForeground);
        if (CheckTopmostWindowFocus(focusedSession, sceneSession)) {
            // return ok if focused session is topmost
            return WSError::WS_OK;
        }
        if (reason == FocusChangeReason::CLIENT_REQUEST && sceneSession->IsAppSession() &&
            sceneSession->GetMissionId() == focusedSession->GetMissionId()) {
            TLOGD(WmsLogTag::WMS_FOCUS, "client request from the same app, skip blocking check");
            byForeground = false;
        }
        if (byForeground && CheckFocusIsDownThroughBlockingType(sceneSession,  focusedSession,  true))  {
            TLOGD(WmsLogTag::WMS_FOCUS, "check, need to be intercepted");
            return WSError::WS_DO_NOTHING;
        }
        if ((reason == FocusChangeReason::SPLIT_SCREEN || reason == FocusChangeReason::FLOATING_SCENE) &&
            !byForeground)  {
            if (!CheckFocusIsDownThroughBlockingType(sceneSession, focusedSession, false)
                && focusedSession->IsAppSession()) {
                TLOGD(WmsLogTag::WMS_FOCUS, "in split or floting , ok");
                return WSError::WS_OK;
            }
        }
        bool isBlockingType = focusedSession->IsAppSession() ||
            (focusedSession->GetSessionInfo().isSystem_ && focusedSession->GetBlockingFocus());
        // temp check
        if (isBlockingType && focusedSession->GetWindowType() == WindowType::WINDOW_TYPE_KEYGUARD &&
            sceneSession->GetZOrder() < focusedSession->GetZOrder()) {
                TLOGD(WmsLogTag::WMS_FOCUS, "Lower session %{public}d cannot request focus from keyguard!",
                    persistentId);
                return WSError::WS_DO_NOTHING;
        }
        // desktop click temp check
        if (CheckClickFocusIsDownThroughFullScreen(focusedSession, sceneSession, reason)) {
            TLOGD(WmsLogTag::WMS_FOCUS, "click cannot request focus from full screen window!");
            return WSError::WS_DO_NOTHING;
        }
        if (!ScreenSessionManagerClient::GetInstance().GetSupportsFocus(displayId)) {
            TLOGW(WmsLogTag::WMS_FOCUS, "screen of current window is not allowed to be focused");
            return WSError::WS_DO_NOTHING;
        }
    }
    return WSError::WS_OK;
}

bool SceneSessionManager::IsParentSessionVisible(const sptr<SceneSession>& session)
{
    if (WindowHelper::IsSubWindow(session->GetWindowType()) ||
        session->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) {
        auto parentSession = GetSceneSession(session->GetParentPersistentId());
        return parentSession == nullptr || IsSessionVisibleForeground(parentSession);
    }
    return true;
}

void SceneSessionManager::DumpAllSessionFocusableInfo(int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "id: %{public}d", persistentId);
    auto func = [this](const sptr<SceneSession>& session) {
        if (session == nullptr) {
            return false;
        }
        bool parentVisible = IsParentSessionVisible(session);
        bool sessionVisible = IsSessionVisible(session);
        TLOGNI(WmsLogTag::WMS_FOCUS, "%{public}d, winType:%{public}d, hide:%{public}d, "
            "focusable:%{public}d, visible:%{public}d, parentVisible:%{public}d",
            session->GetPersistentId(), session->GetWindowType(), session->GetForceHideState(),
            session->GetFocusable(), sessionVisible, parentVisible);
        return false;
    };
    TraverseSessionTree(func, true);
}

sptr<SceneSession> SceneSessionManager::GetNextFocusableSession(DisplayId displayId, int32_t persistentId)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "id: %{public}d", persistentId);
    bool preFocusedSessionFound = false;
    DisplayId displayGroupId = windowFocusController_->GetDisplayGroupId(displayId);
    sptr<SceneSession> nextFocusableSession = nullptr;
    auto func = [this, persistentId, &preFocusedSessionFound, &nextFocusableSession,
        displayGroupId, displayId](sptr<SceneSession> session) {
        if (session == nullptr) {
            return false;
        }
        if (session->GetPersistentId() == persistentId) {
            preFocusedSessionFound = true;
            return false;
        }
        if (!preFocusedSessionFound) {
            return false;
        }
        auto currentSessionDisplayId = session->GetDisplayId();
        if (windowFocusController_->GetDisplayGroupId(currentSessionDisplayId) != displayGroupId) {
            return false;
        }
        if (!ScreenSessionManagerClient::GetInstance().GetSupportsFocus(currentSessionDisplayId)) {
            TLOGW(WmsLogTag::WMS_FOCUS, "screen of current window is not allowed to be focused");
            return false;
        }
        if (session->GetForceHideState() != ForceHideState::NOT_HIDDEN) {
            TLOGND(WmsLogTag::WMS_FOCUS, "the window hide id: %{public}d", persistentId);
            return false;
        }
        if (preFocusedSessionFound && session->CheckFocusable() &&
            session->IsVisibleNotBackground() && IsParentSessionVisible(session)) {
            if (!systemConfig_.IsPcWindow() || session->GetWindowType() != WindowType::WINDOW_TYPE_DESKTOP ||
                currentSessionDisplayId == displayId) {
                nextFocusableSession = session;
                return true;
            }
        }
        return false;
    };
    TraverseSessionTree(func, true);
    sptr<SceneSession> topFloatingSession = GetNextFocusableSessionWhenFloatWindowExist(displayGroupId, persistentId);
    if (topFloatingSession != nullptr && nextFocusableSession != nullptr && nextFocusableSession->GetWindowType() == WindowType::WINDOW_TYPE_DESKTOP) {
        TLOGI(WmsLogTag::WMS_FOCUS, "topFloatingSessionId: %{public}d", topFloatingSession->GetPersistentId());
        return topFloatingSession;
    }
    return nextFocusableSession;
}

sptr<SceneSession> SceneSessionManager::GetNextFocusableSessionWhenFloatWindowExist(DisplayId displayGroupId,
                                                                                    int32_t persistentId)
{
    bool isPhoneOrPadWithoutPcMode =
        (systemConfig_.IsPhoneWindow() || systemConfig_.IsPadWindow()) && !systemConfig_.IsFreeMultiWindowMode();
    if (!isPhoneOrPadWithoutPcMode) {
        return nullptr;
    }
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr || !SessionHelper::IsMainWindow(sceneSession->GetWindowType()) ||
        sceneSession->GetWindowMode() != WindowMode::WINDOW_MODE_FULLSCREEN) {
        TLOGD(WmsLogTag::WMS_FOCUS, "current scene session unqualified, id: %{public}d", persistentId);
        return nullptr;
    }
    auto topFloatingSession = GetTopFloatingSession(displayGroupId, persistentId);
    if (topFloatingSession == nullptr) {
        TLOGD(WmsLogTag::WMS_FOCUS, "no top floating session found");
        return nullptr;
    }
    if (!ScreenSessionManagerClient::GetInstance().GetSupportsFocus(topFloatingSession->GetDisplayId())) {
        TLOGW(WmsLogTag::WMS_FOCUS, "screen of current window is not allowed to be focused");
        return nullptr;
    }
    return topFloatingSession;
}

sptr<SceneSession> SceneSessionManager::GetTopFloatingSession(DisplayId displayGroupId, int32_t persistentId)
{
    bool previousFocusedSessionFound = false;
    sptr<SceneSession> topFloatingSession = nullptr;
    auto func = [this, persistentId, &previousFocusedSessionFound, &topFloatingSession, displayGroupId](sptr<SceneSession> session) {
        if (session == nullptr || topFloatingSession != nullptr || previousFocusedSessionFound) {
            return false;
        }
        if (windowFocusController_->GetDisplayGroupId(session->GetSessionProperty()->GetDisplayId()) !=
            displayGroupId) {
            return false;
        }
        if (session->GetForceHideState() != ForceHideState::NOT_HIDDEN) {
            TLOGND(WmsLogTag::WMS_FOCUS, "the window hide id: %{public}d", persistentId);
            return false;
        }
        // need to be floating window
        if (session->GetWindowMode() != WindowMode::WINDOW_MODE_FLOATING) {
            return false;
        }
        // need to be main window
        if (!SessionHelper::IsMainWindow(session->GetWindowType())) {
            return false;
        }

        if (session->CheckFocusable() && session->IsVisible()) {
            topFloatingSession = session;
            return true;
        }
        if (session->GetPersistentId() == persistentId) {
            previousFocusedSessionFound = true;
        }
        return false;
    };
    TraverseSessionTree(func, true);
    return topFloatingSession;
}

/**
 * Find the session through the specific zOrder, it is located abve it, its' blockingFocus attribute is true,
 * and it is the closest;
 */
sptr<SceneSession> SceneSessionManager::GetTopNearestBlockingFocusSession(DisplayId displayId, uint32_t zOrder,
    bool includingAppSession)
{
    sptr<SceneSession> ret = nullptr;
    DisplayId displayGroupId = windowFocusController_->GetDisplayGroupId(displayId);
    auto func = [this, &ret, zOrder, includingAppSession, displayGroupId](sptr<SceneSession> session) {
        if (session == nullptr) {
            return false;
        }
        if (windowFocusController_->GetDisplayGroupId(session->GetSessionProperty()->GetDisplayId()) !=
            displayGroupId) {
            return false;
        }
        uint32_t sessionZOrder = session->GetZOrder();
        if (sessionZOrder <= zOrder) { // must be above the target session
            return false;
        }
        if (session->IsTopmost() && session->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
            TLOGND(WmsLogTag::WMS_FOCUS, "topmost window do not block");
            return false;
        }
        auto parentSession = GetSceneSession(session->GetParentPersistentId());
        if (SessionHelper::IsSubWindow(session->GetWindowType()) && parentSession != nullptr &&
            parentSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW && parentSession->IsTopmost()) {
            TLOGND(WmsLogTag::WMS_FOCUS, "sub window of topmost do not block");
            return false;
        }

        if (IsSessionVisibleForeground(session) && CheckBlockingFocus(session, includingAppSession)) {
            ret = session;
            return true;
        }
        return false;
    };
    TraverseSessionTree(func, false);
    return ret;
}

bool SceneSessionManager::CheckBlockingFocus(const sptr<SceneSession>& session, bool includingAppSession)
{
    if (session->GetSessionInfo().isSystem_ && session->GetBlockingFocus()) {
        TLOGD(WmsLogTag::WMS_FOCUS, "system window blocked");
        return true;
    }

    bool isPhoneOrPad = systemConfig_.IsPhoneWindow() || systemConfig_.IsPadWindow();
    if (isPhoneOrPad && (session->GetWindowType() == WindowType::WINDOW_TYPE_VOICE_INTERACTION ||
        session->GetWindowType() == WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD)) {
        return true;
    }
    if (includingAppSession && session->IsAppSession()) {
        TLOGD(WmsLogTag::WMS_FOCUS,
              "id: %{public}d, isFloatType: %{public}d, isFloatMode: %{public}d", session->GetPersistentId(),
              session->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT,
              session->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING);
        bool isPcOrPcMode =
            systemConfig_.IsPcWindow() || (isPhoneOrPad && systemConfig_.IsFreeMultiWindowMode());
        if (isPcOrPcMode && session->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT) {
            return false;
        }
        bool isPhoneOrPadWithoutPcMode = isPhoneOrPad && !systemConfig_.IsFreeMultiWindowMode();
        if (isPhoneOrPadWithoutPcMode && session->GetWindowMode() == WindowMode::WINDOW_MODE_FLOATING &&
            SessionHelper::IsMainWindow(session->GetWindowType()) && !session->GetIsMidScene()) {
            return false;
        }
        return true;
    }
    return false;
}

sptr<SceneSession> SceneSessionManager::GetTopFocusableNonAppSession()
{
    TLOGD(WmsLogTag::WMS_FOCUS, "in.");
    sptr<SceneSession> ret = nullptr;
    auto func = [this, &ret](sptr<SceneSession> session) {
        if (session == nullptr) {
            return false;
        }
        if (windowFocusController_->GetDisplayGroupId(session->GetSessionProperty()->GetDisplayId()) !=
            DEFAULT_DISPLAY_ID) {
            return false;
        }
        if (session->IsAppSession()) {
            return session->GetZOrder() == 0 ? false : true;
        }
        if (session->CheckFocusable() && IsSessionVisibleForeground(session)) {
            ret = session;
        }
        return false;
    };
    TraverseSessionTree(func, false);
    return ret;
}

void SceneSessionManager::SetShiftFocusListener(const ProcessShiftFocusFunc& func)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "in");
    shiftFocusFunc_ = func;
}

void SceneSessionManager::SetSCBFocusedListener(const NotifySCBAfterUpdateFocusFunc& func)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "in");
    notifySCBAfterFocusedFunc_ = func;
}

void SceneSessionManager::SetSCBUnfocusedListener(const NotifySCBAfterUpdateFocusFunc& func)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "in");
    notifySCBAfterUnfocusedFunc_ = func;
}

void SceneSessionManager::SetSCBFocusChangeListener(const NotifyDiffSCBAfterUpdateFocusFunc&& func)
{
    TLOGD(WmsLogTag::WMS_FOCUS, "in");
    notifyDiffSCBAfterUnfocusedFunc_ = std::move(func);
}

void SceneSessionManager::SetStartUIAbilityErrorListener(const ProcessStartUIAbilityErrorFunc& func)
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    startUIAbilityErrorFunc_ = func;
}

void SceneSessionManager::SetAbilityManagerCollaboratorRegisteredFunc(
    const AbilityManagerCollaboratorRegisteredFunc& func)
{
    auto task = [this, func] {
        abilityManagerCollaboratorRegisteredFunc_ = func;
    };
    taskScheduler_->PostAsyncTask(task, __func__);
}

sptr<FocusNotifyInfo> SceneSessionManager::GetFocusNotifyInfo(DisplayId displayId,
    const sptr<SceneSession>& nextSession)
{
    auto focusGroup = windowFocusController_->GetFocusGroup(displayId);
    if (focusGroup == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, displayId);
        return nullptr;
    }
    auto focusNotifyTimeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    focusGroup->SetUpdateFocusTimeStamp(focusNotifyTimeStamp);
    auto focusedSessionId = focusGroup->GetFocusedSessionId();
    auto focusedSession = GetSceneSession(focusedSessionId);
    int32_t nextSessionId = nextSession == nullptr ? INVALID_SESSION_ID : nextSession->GetPersistentId();
    auto focusNotifyInfo = sptr<FocusNotifyInfo>::MakeSptr(focusGroup->GetUpdateFocusTimeStamp(), focusedSessionId,
        nextSessionId, false);
    if (focusedSession && nextSession) {
        focusNotifyInfo->isSyncNotify_ = focusedSession->GetCallingPid() == nextSession->GetCallingPid() &&
            !focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground();
    }
    return focusNotifyInfo;
}

WSError SceneSessionManager::ShiftFocus(DisplayId displayId, const sptr<SceneSession>& nextSession,
    bool isProactiveUnfocus, FocusChangeReason reason)
{
    auto focusNotifyInfo = GetFocusNotifyInfo(displayId, nextSession);
    if (focusNotifyInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focusNotifyInfo is nullptr");
        return WSError::WS_OK;
    }
    // unfocus
    auto focusedSessionId = windowFocusController_->GetFocusedSessionId(displayId);
    int32_t focusedId = focusedSessionId;
    auto focusedSession = GetSceneSession(focusedSessionId);
    UpdateFocusStatus(displayId, focusedSession, false, focusNotifyInfo);
    // focus
    int32_t nextId = INVALID_SESSION_ID;
    if (nextSession == nullptr) {
        std::string sessionLog(GetAllSessionFocusInfo());
        TLOGW(WmsLogTag::WMS_FOCUS, "next session nullptr! id: %{public}d, info: %{public}s",
            focusedSessionId, sessionLog.c_str());
    } else {
        nextId = nextSession->GetPersistentId();
    }
    UpdateFocusStatus(displayId, nextSession, true, focusNotifyInfo);
    UpdateGestureBackEnabled(focusedId, nextId);
    UpdateHighlightStatus(displayId, focusedSession, nextSession, isProactiveUnfocus);
    if (shiftFocusFunc_ != nullptr) {
        auto displayGroupId = windowFocusController_->GetDisplayGroupId(displayId);
        shiftFocusFunc_(nextId, displayGroupId);
    }
    bool scbPrevFocus = focusedSession && focusedSession->GetSessionInfo().isSystem_;
    bool scbCurrFocus = nextSession && nextSession->GetSessionInfo().isSystem_;
    if (!scbPrevFocus && scbCurrFocus) {
        if (notifySCBAfterFocusedFunc_ != nullptr) {
            notifySCBAfterFocusedFunc_(nextSession->GetSessionProperty()->GetDisplayId());
        }
    } else if (scbPrevFocus && !scbCurrFocus) {
        if (notifySCBAfterUnfocusedFunc_ != nullptr) {
            notifySCBAfterUnfocusedFunc_(focusedSession->GetSessionProperty()->GetDisplayId());
        }
    } else if (scbPrevFocus && scbCurrFocus) {
        DisplayId focusedSessionDisplayId = focusedSession->GetSessionProperty()->GetDisplayId();
        DisplayId nextSessionDisplayId = nextSession->GetSessionProperty()->GetDisplayId();
        if (notifyDiffSCBAfterUnfocusedFunc_ != nullptr && focusedSessionDisplayId != nextSessionDisplayId) {
            notifyDiffSCBAfterUnfocusedFunc_(focusedSessionDisplayId, nextSessionDisplayId);
        }
    }
    TLOGI(WmsLogTag::WMS_FOCUS, "[%{public}" PRIu64 ",%{public}d,%{public}d,%{public}d",
        displayId, focusedId, nextId, reason);
    return WSError::WS_OK;
}

void SceneSessionManager::UpdateFocusStatus(DisplayId displayId, const sptr<SceneSession>& sceneSession,
    bool isFocused, const sptr<FocusNotifyInfo>& focusNotifyInfo)
{
    auto focusGroup = windowFocusController_->GetFocusGroup(displayId);
    if (focusGroup == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, displayId);
        return;
    }
    if (focusNotifyInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focusNotifyInfo is nullptr");
        return;
    }
    bool needBlockNotifyFocusStatusUntilForeground = focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground();
    bool needBlockNotifyUnfocusStatus = focusGroup->GetNeedBlockNotifyUnfocusStatus();
    if (sceneSession == nullptr) {
        TLOGW(WmsLogTag::WMS_FOCUS, "session is nullptr");
        if (isFocused) {
            SetFocusedSessionId(INVALID_SESSION_ID, displayId);
            focusGroup->SetLastFocusedAppSessionId(INVALID_SESSION_ID);
            auto prevSession = GetSceneSession(focusGroup->GetLastFocusedSessionId());
            NotifyUnFocusedByMission(prevSession);
        }
        return;
    }
    TLOGD(WmsLogTag::WMS_FOCUS, "name: %{public}s, id: %{public}d, isFocused: %{public}d, displayId: %{public}" PRIu64,
          sceneSession->GetWindowNameAllType().c_str(), sceneSession->GetPersistentId(), isFocused, displayId);
    // set focused
    if (isFocused) {
        SetFocusedSessionId(sceneSession->GetPersistentId(), displayId);
        if (sceneSession->IsAppOrLowerSystemSession()) {
            focusGroup->SetLastFocusedAppSessionId(sceneSession->GetPersistentId());
        }
    }
    sceneSession->UpdateFocus(isFocused);
    // notify listenerController unfocused
    auto prevSession = GetSceneSession(focusGroup->GetLastFocusedSessionId());
    if (isFocused && MissionChanged(prevSession, sceneSession)) {
        NotifyUnFocusedByMission(prevSession);
    }
    if ((isFocused && !needBlockNotifyFocusStatusUntilForeground) || (!isFocused && !needBlockNotifyUnfocusStatus)) {
        NotifyFocusStatus(sceneSession, isFocused, focusGroup, focusNotifyInfo);
    }
}

/** @note @window.focus */
void SceneSessionManager::UpdateHighlightStatus(DisplayId displayId, const sptr<SceneSession>& preSceneSession,
    const sptr<SceneSession>& currSceneSession, bool isProactiveUnfocus)
{
    auto focusGroup = windowFocusController_->GetFocusGroup(displayId);
    if (focusGroup == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, displayId);
        return;
    }
    bool needBlockHighlightNotify = focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground();
    if(isProactiveUnfocus){
        TLOGD(WmsLogTag::WMS_FOCUS, "proactiveUnfocus");
        RemoveHighlightSessionIds(preSceneSession);
    }
    if (currSceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "currSceneSession is nullptr");
        return;
    }
    if(currSceneSession->GetSessionProperty()->GetExclusivelyHighlighted()) {
        TLOGD(WmsLogTag::WMS_FOCUS, "exclusively highlighted");
        SetHighlightSessionIds(currSceneSession, needBlockHighlightNotify, focusGroup->GetUpdateFocusTimeStamp());
        return;
    }
    if(SessionHelper::IsSystemWindow(currSceneSession->GetWindowType())) {
        TLOGD(WmsLogTag::WMS_FOCUS, "system highlighted");
        AddHighlightSessionIds(currSceneSession, needBlockHighlightNotify);
        return;
    }
    if(currSceneSession->IsSameMainSession(preSceneSession)) {
        TLOGD(WmsLogTag::WMS_FOCUS, "related highlighted");
        AddHighlightSessionIds(currSceneSession, needBlockHighlightNotify);
        return;
    }
    TLOGD(WmsLogTag::WMS_FOCUS, "highlighted");
    SetHighlightSessionIds(currSceneSession, needBlockHighlightNotify, focusGroup->GetUpdateFocusTimeStamp());
}

/** @note @window.focus */
void SceneSessionManager::SetHighlightSessionIds(const sptr<SceneSession>& sceneSession, bool needBlockHighlightNotify,
    int64_t timeStamp)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "sceneSession is nullptr");
        return;
    }
    {
        std::unordered_set<int32_t> highlightIdsBack;
        {
            std::lock_guard<std::mutex> lock(highlightIdsMutex_);
            highlightIdsBack = highlightIds_;
            highlightIds_.clear();
            highlightIds_.insert(sceneSession->GetPersistentId());
        }
        std::vector<int32_t> highlightIdVector;
        std::copy_if(highlightIdsBack.begin(), highlightIdsBack.end(), std::back_inserter(highlightIdVector),
            [this, sceneSession](int32_t persistentId) {
                auto session = GetSceneSession(persistentId);
                if (session == nullptr) {
                    TLOGE(WmsLogTag::WMS_FOCUS, "session is nullptr");
                    return false;
                }
                return sceneSession->GetPersistentId() != persistentId &&
                    session->GetCallingPid() == sceneSession->GetCallingPid();
            });
        auto highlightNotifyInfo = sptr<HighlightNotifyInfo>::MakeSptr(timeStamp, highlightIdVector,
            sceneSession->GetPersistentId(), false);
        for (auto persistentId : highlightIdsBack) {
            auto session = GetSceneSession(persistentId);
            if (session == nullptr) {
                TLOGE(WmsLogTag::WMS_FOCUS, "session is nullptr");
                continue;
            }
            if (sceneSession->GetPersistentId() != persistentId) {
                highlightNotifyInfo->isSyncNotify_ = !needBlockHighlightNotify &&
                    session->GetCallingPid() == sceneSession->GetCallingPid();
                session->UpdateHighlightStatus(highlightNotifyInfo, false, false);
            }
        }
        highlightNotifyInfo->isSyncNotify_ = true;
        sceneSession->UpdateHighlightStatus(highlightNotifyInfo, true, needBlockHighlightNotify);

    }
    TLOGI(WmsLogTag::WMS_FOCUS, "%{public}s", GetHighlightIdsStr().c_str());
}

/** @note @window.focus */
void SceneSessionManager::AddHighlightSessionIds(const sptr<SceneSession>& sceneSession, bool needBlockHighlightNotify)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "sceneSession is nullptr");
        return;
    }
    auto highlightNotifyInfo = sptr<HighlightNotifyInfo>::MakeSptr(INVALID_TIME_STAMP, std::vector<int32_t>(),
        sceneSession->GetPersistentId(), false);
    sceneSession->UpdateHighlightStatus(highlightNotifyInfo, true, needBlockHighlightNotify);
    {
        std::lock_guard<std::mutex> lock(highlightIdsMutex_);
        highlightIds_.insert(sceneSession->GetPersistentId());
    }
    TLOGI(WmsLogTag::WMS_FOCUS, "highlightIds: %{public}s", GetHighlightIdsStr().c_str());
}

/** @note @window.focus */
void SceneSessionManager::RemoveHighlightSessionIds(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "sceneSession is nullptr");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(highlightIdsMutex_);
        if (highlightIds_.find(sceneSession->GetPersistentId()) != highlightIds_.end()) {
            auto highlightNotifyInfo = sptr<HighlightNotifyInfo>::MakeSptr(INVALID_TIME_STAMP,
                std::vector<int32_t>{sceneSession->GetPersistentId()}, INVALID_SESSION_ID, false);
            sceneSession->UpdateHighlightStatus(highlightNotifyInfo, false, false);
            highlightIds_.erase(sceneSession->GetPersistentId());
        } else {
            TLOGE(WmsLogTag::WMS_FOCUS, "not found scene session with id: %{public}d", sceneSession->GetPersistentId());
        }

    }
    TLOGI(WmsLogTag::WMS_FOCUS, "%{public}s", GetHighlightIdsStr().c_str());
}

/** @note @window.focus */
std::string SceneSessionManager::GetHighlightIdsStr()
{
    std::ostringstream oss;
    {
        std::lock_guard<std::mutex> lock(highlightIdsMutex_);
        for (auto it = highlightIds_.begin(); it != highlightIds_.end(); it++) {
            oss << *it;
            if(std::next(it) != highlightIds_.end()) {
                oss << ", ";
            }
        }

    }
    return oss.str();
}

void SceneSessionManager::NotifyFocusStatus(const sptr<SceneSession>& sceneSession, bool isFocused,
    const sptr<FocusGroup>& focusGroup, const sptr<FocusNotifyInfo>& focusNotifyInfo)
{
    if (focusGroup == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr");
        return;
    }
    if (focusNotifyInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focusNotifyInfo is nullptr");
        return;
    }
    int32_t persistentId = sceneSession->GetPersistentId();

    TLOGI(WmsLogTag::WMS_FOCUS,
        "name: %{public}s/%{public}s/%{public}s, id: %{public}d, isFocused: %{public}d",
        sceneSession->GetSessionInfo().bundleName_.c_str(),
        sceneSession->GetSessionInfo().abilityName_.c_str(),
        sceneSession->GetWindowNameAllType().c_str(),
        persistentId, isFocused);
    if (isFocused) {
        if (IsSessionVisibleForeground(sceneSession)) {
            NotifyWindowInfoChange(persistentId, WindowUpdateType::WINDOW_UPDATE_FOCUSED);
        }
        UpdateBrightness(focusGroup->GetFocusedSessionId());
        FocusIDChange(sceneSession->GetPersistentId(), sceneSession);
    }
    DisplayId focusGroupId = focusGroup->GetDisplayGroupId() == DEFAULT_DISPLAY_ID ? DEFAULT_DISPLAY_ID : sceneSession->GetDisplayId();
    // notify window manager
    sptr<FocusChangeInfo> focusChangeInfo = sptr<FocusChangeInfo>::MakeSptr(
        sceneSession->GetWindowId(),
        focusGroupId,
        sceneSession->GetDisplayId(),
        focusGroup->GetDisplayGroupId(),
        sceneSession->GetCallingPid(),
        sceneSession->GetCallingUid(),
        sceneSession->GetWindowType(),
        sceneSession->GetAbilityToken()
    );
    SceneSessionManager::NotifyRssThawApp(focusChangeInfo->uid_, "", "THAW_BY_FOCUS_CHANGED");
    SessionManagerAgentController::GetInstance().UpdateFocusChangeInfo(focusChangeInfo, isFocused);
    sceneSession->NotifyFocusStatus(focusNotifyInfo, isFocused);
    // notify listenerController focused
    auto prevSession = GetSceneSession(focusGroup->GetLastFocusedSessionId());
    if (isFocused && MissionChanged(prevSession, sceneSession)) {
        NotifyFocusedByMission(sceneSession);
    }
}

int32_t SceneSessionManager::NotifyRssThawApp(const int32_t uid, const std::string& bundleName,
    const std::string& reason)
{
    uint32_t resType = ResourceSchedule::ResType::SYNC_RES_TYPE_THAW_ONE_APP;
    nlohmann::json payload;
    payload.emplace("uid", uid);
    payload.emplace("bundleName", bundleName);
    payload.emplace("reason", reason);
    nlohmann::json reply;
    return ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(resType, 0, payload, reply);
}

void SceneSessionManager::NotifyFocusStatusByMission(const sptr<SceneSession>& prevSession,
    const sptr<SceneSession>& currSession)
{
    if (prevSession && !prevSession->GetSessionInfo().isSystem_) {
        TLOGD(WmsLogTag::WMS_FOCUS, "Unfocused, id: %{public}d", prevSession->GetMissionId());
        listenerController_->NotifySessionUnfocused(prevSession->GetMissionId());
    }
    if (currSession && !currSession->GetSessionInfo().isSystem_) {
        TLOGD(WmsLogTag::WMS_FOCUS, "Focused, id: %{public}d", currSession->GetMissionId());
        listenerController_->NotifySessionFocused(currSession->GetMissionId());
    }
}

void SceneSessionManager::NotifyUnFocusedByMission(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession && !sceneSession->GetSessionInfo().isSystem_) {
        TLOGD(WmsLogTag::WMS_FOCUS, "id: %{public}d", sceneSession->GetMissionId());
        listenerController_->NotifySessionUnfocused(sceneSession->GetMissionId());
    }
}

void SceneSessionManager::NotifyFocusedByMission(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession && !sceneSession->GetSessionInfo().isSystem_) {
        TLOGD(WmsLogTag::WMS_FOCUS, "Focused, id: %{public}d", sceneSession->GetMissionId());
        listenerController_->NotifySessionFocused(sceneSession->GetMissionId());
    }
}

bool SceneSessionManager::MissionChanged(const sptr<SceneSession>& prevSession, const sptr<SceneSession>& currSession)
{
    if (prevSession == nullptr && currSession == nullptr) {
        return false;
    }
    if (prevSession == nullptr || currSession == nullptr) {
        return true;
    }
    return prevSession->GetMissionId() != currSession->GetMissionId();
}

std::string SceneSessionManager::GetAllSessionFocusInfo()
{
    std::ostringstream os;
    auto func = [&os](sptr<SceneSession> session) {
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "sceneSession is nullptr");
            return false;
        }
        os << "WindowName: " << session->GetWindowName() << ", id: " << session->GetPersistentId() <<
              ", focusable: " << session->GetFocusable() << ";";
        return false;
    };
    TraverseSessionTree(func, true);
    return os.str();
}

WSError SceneSessionManager::UpdateWindowMode(int32_t persistentId, int32_t windowMode)
{
    TLOGD(WmsLogTag::DEFAULT, "id: %{public}d, mode: %{public}d", persistentId, windowMode);
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "could not find window, Id:%{public}d", persistentId);
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    WindowMode mode = static_cast<WindowMode>(windowMode);
    return sceneSession->UpdateWindowMode(mode);
}

SingleHandTransform SceneSessionManager::GetNormalSingleHandTransform() const
{
    return singleHandTransform_;
}

SingleHandScreenInfo SceneSessionManager::GetSingleHandScreenInfo() const
{
    return singleHandScreenInfo_;
}

WSRect SceneSessionManager::GetOriginRect() const
{
    return originRect_;
}

WSRect SceneSessionManager::GetSingleHandRect() const
{
    return singleHandRect_;
}

void SceneSessionManager::NotifySingleHandInfoChange(SingleHandScreenInfo singleHandScreenInfo, WSRect originRect, WSRect singleHandRect)
{
    const char* const funcName = __func__;
    taskScheduler_->PostAsyncTask([this, singleHandScreenInfo, originRect, singleHandRect, funcName] {
        if (!systemConfig_.IsPhoneWindow()) {
            TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s: only support phone", funcName);
            return;
        }
        int32_t displayWidth = 0;
        int32_t displayHeight = 0;
        ScreenId defaultScreenId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
        if (!GetDisplaySizeById(defaultScreenId, displayWidth, displayHeight)) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "%{public}s: get display size failed", funcName);
            return;
        }
        singleHandScreenInfo_ = singleHandScreenInfo;
        originRect_ = originRect;
        singleHandRect_ = singleHandRect;
        singleHandTransform_.posX = singleHandRect.posX_;
        singleHandTransform_.posY = singleHandRect.posY_;
        singleHandTransform_.scaleX = static_cast<float>(singleHandScreenInfo_.scaleRatio) / 100;
        singleHandTransform_.scaleY = singleHandTransform_.scaleX;
        {
            std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
            for (const auto& [_, sceneSession] : sceneSessionMap_) {
                if (sceneSession == nullptr || !IsInDefaultScreen(sceneSession) ||
                    sceneSession->GetWindowName().find("OneHandModeBackground", 0) != std::string::npos) {
                    continue;
                }
                sceneSession->SetSingleHandModeFlag(true);
                sceneSession->SetSingleHandTransform(singleHandTransform_);
                sceneSession->NotifySingleHandTransformChange(singleHandTransform_);
            }
        }
        FlushWindowInfoToMMI();
    }, funcName);
}

void SceneSessionManager::RegisterGetRSNodeByStringIDFunc(GetRSNodeByStringIDFunc&& func)
{
    getRSNodeByStringIDFunc_ = std::move(func);
}

void SceneSessionManager::RegisterSetTopWindowBoundaryByIDFunc(SetTopWindowBoundaryByIDFunc&& func)
{
    setTopWindowBoundaryByIDFunc_ = std::move(func);
}

void SceneSessionManager::RegisterSingleHandContainerNode(const std::string& stringId)
{
    if (getRSNodeByStringIDFunc_ == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "func is nullptr");
        return;
    }
    auto rsNode = getRSNodeByStringIDFunc_(stringId);
    if (rsNode == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "node is nullptr");
        return;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "get OneHandModeBox node, id: %{public}" PRIu64, rsNode->GetId());
    setTopWindowBoundaryByIDFunc_(stringId);
    rsInterface_.SetWindowContainer(rsNode->GetId(), true);
}

const SingleHandCompatibleModeConfig& SceneSessionManager::GetSingleHandCompatibleModeConfig() const
{
    return singleHandCompatibleModeConfig_;
}

#ifdef SECURITY_COMPONENT_MANAGER_ENABLE
static void FillSecCompEnhanceData(const std::shared_ptr<MMI::PointerEvent>& pointerEvent,
    MMI::PointerEvent::PointerItem& pointerItem)
{
    struct PointerEventData {
        double x;
        double y;
        uint64_t time;
    } pointerEventData = {
        .x = pointerItem.GetDisplayX(),
        .y = pointerItem.GetDisplayY(),
        .time = pointerEvent->GetActionTime()
    };

    const uint32_t MAX_HMAC_SIZE = 64;
    uint8_t outBuf[MAX_HMAC_SIZE] = { 0 };
    uint8_t *enhanceData = reinterpret_cast<uint8_t *>(&outBuf[0]);
    uint32_t enhanceDataLen = MAX_HMAC_SIZE;
    if (Security::SecurityComponent::SecCompEnhanceKit::GetPointerEventEnhanceData(&pointerEventData,
        sizeof(pointerEventData), enhanceData, enhanceDataLen) == 0) {
        pointerEvent->SetEnhanceData(std::vector<uint8_t>(outBuf, outBuf + enhanceDataLen));
    }
}
#endif // SECURITY_COMPONENT_MANAGER_ENABLE

WSError SceneSessionManager::SendTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent, uint32_t zIndex)
{
    if (!pointerEvent) {
        TLOGE(WmsLogTag::WMS_EVENT, "pointerEvent is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        TLOGE(WmsLogTag::WMS_EVENT, "Failed to get pointerItem");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
#ifdef SECURITY_COMPONENT_MANAGER_ENABLE
    FillSecCompEnhanceData(pointerEvent, pointerItem);
#endif
    TLOGI(WmsLogTag::WMS_EVENT, "eid=%{public}d,ac=%{public}d,deviceId=%{public}d,zIndex=%{public}ud",
        pointerEvent->GetPointerId(), pointerEvent->GetPointerAction(), pointerEvent->GetDeviceId(), zIndex);
    pointerEvent->AddFlag(MMI::PointerEvent::EVENT_FLAG_NO_INTERCEPT);
    MMI::InputManager::GetInstance()->SimulateInputEvent(pointerEvent, static_cast<float>(zIndex));
    return WSError::WS_OK;
}

void SceneSessionManager::SetScreenLocked(const bool isScreenLocked)
{
    taskScheduler_->PostTask([this, isScreenLocked] {
        isScreenLocked_ = isScreenLocked;
        DeleteStateDetectTask();
        NotifyPiPWindowVisibleChange(isScreenLocked);
    }, __func__);
    NotifySessionScreenLockedChange(isScreenLocked);
    if (isTrayAppForeground_) {
        SchedulePcAppInPadLifecycle(isScreenLocked);
    }
}

void SceneSessionManager::SetUserAuthPassed(bool isUserAuthPassed)
{
    taskScheduler_->PostTask([this, isUserAuthPassed] {
        isUserAuthPassed_ = isUserAuthPassed;
    }, __func__);
}

void SceneSessionManager::DeleteStateDetectTask()
{
    if (!IsScreenLocked()) {
        return;
    }
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (auto iter : sceneSessionMap_) {
        auto& session = iter.second;
        if (session && session->GetDetectTaskInfo().taskState != DetectTaskState::NO_TASK) {
            taskScheduler_->GetEventHandler()->RemoveTask(session->GetWindowDetectTaskName());
            DetectTaskInfo detectTaskInfo;
            session->SetDetectTaskInfo(detectTaskInfo);
        }
    }
}

bool SceneSessionManager::IsScreenLocked() const
{
    return isScreenLocked_;
}

bool SceneSessionManager::IsUserAuthPassed() const
{
    return isUserAuthPassed_;
}

void SceneSessionManager::RegisterWindowChanged(const WindowChangedFunc& func)
{
    WindowChangedFunc_ = func;
}

bool SceneSessionManager::JudgeNeedNotifyPrivacyInfo(DisplayId displayId,
    const std::unordered_set<std::string>& privacyBundles)
{
    bool needNotify = false;
    static int reSendTimes = MAX_RESEND_TIMES;
    std::unique_lock<std::mutex> lock(privacyBundleMapMutex_);
    do {
        if (privacyBundleMap_.find(displayId) == privacyBundleMap_.end()) {
            TLOGD(WmsLogTag::WMS_MAIN, "can not find display[%{public}" PRIu64 "].", displayId);
            needNotify = !privacyBundles.empty();
            break;
        }
        const auto& lastPrivacyBundles = privacyBundleMap_[displayId];
        if (lastPrivacyBundles.size() != privacyBundles.size()) {
            TLOGD(WmsLogTag::WMS_MAIN, "privacy bundle list size is not equal, %{public}zu != %{public}zu.",
                  lastPrivacyBundles.size(), privacyBundles.size());
            needNotify = true;
            break;
        }
        for (const auto& bundle : lastPrivacyBundles) {
            if (privacyBundles.find(bundle) == privacyBundles.end()) {
                needNotify = true;
                break;
            }
        }
    } while (false);

    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "display=%{public}" PRIu64 ", needNotify=%{public}d, sendTimes=%{public}d",
          displayId, needNotify, reSendTimes);
    if (needNotify) {
        reSendTimes = MAX_RESEND_TIMES;
        privacyBundleMap_[displayId] = privacyBundles;
    } else if (reSendTimes > 0) {
        needNotify = true;
        reSendTimes--;
        privacyBundleMap_[displayId] = privacyBundles;
    }
    return needNotify;
}

void SceneSessionManager::UpdatePrivateStateAndNotify(uint32_t persistentId)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "update privacy state failed, scene is nullptr, wid=%{public}u.", persistentId);
        return;
    }

    auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    std::unordered_map<DisplayId, std::unordered_set<std::string>> privacyBundleList;
    GetSceneSessionPrivacyModeBundles(displayId, privacyBundleList);
    for (const auto& iter : privacyBundleList) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "display=%{public}" PRIu64 ", privacy bundle size=%{public}zu, uecPrivace:"
            "%{public}d", iter.first, iter.second.size(), specialExtWindowHasPrivacyMode_.load());
        for (const auto& bundle : iter.second) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "privacy bundle: %{public}s", bundle.c_str());
        }
    }
    if (isUserBackground_) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "user is in background, win=[%{public}u, %{public}s]",
            persistentId, sceneSession->GetWindowName().c_str());
        return;
    }

    bool isNeedUpdatePrivateState = JudgeNeedNotifyPrivacyInfo(displayId, privacyBundleList[displayId]);
    if (PcFoldScreenManager::GetInstance().IsHalfFolded(displayId)){
        isNeedUpdatePrivateState |= JudgeNeedNotifyPrivacyInfo(VIRTUAL_DISPLAY_ID,
            privacyBundleList[VIRTUAL_DISPLAY_ID]);
    }
    if (!isNeedUpdatePrivateState) {
        return;
    }
    std::unordered_map<DisplayId, bool> privacyBundleDisplayId;
    std::unordered_map<DisplayId, std::vector<std::string>> notifyPrivacyBundleList;
    for (const auto& iter : privacyBundleList) {
        privacyBundleDisplayId[iter.first] = !iter.second.empty() || specialExtWindowHasPrivacyMode_.load();
        notifyPrivacyBundleList[iter.first] = std::vector<std::string>(iter.second.begin(), iter.second.end());
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "notify privacy, win=[%{public}u, %{public}s], uecPrivace=%{public}d",
        persistentId, sceneSession->GetWindowName().c_str(), specialExtWindowHasPrivacyMode_.load());
    ScreenSessionManagerClient::GetInstance().SetPrivacyStateByDisplayId(privacyBundleDisplayId);
    ScreenSessionManagerClient::GetInstance().SetScreenPrivacyWindowList(notifyPrivacyBundleList);
}

void SceneSessionManager::UpdatePrivateStateAndNotifyForAllScreens()
{
    std::unordered_map<DisplayId, std::unordered_set<std::string>> privacyBundleList;
    auto screenProperties = ScreenSessionManagerClient::GetInstance().GetAllScreensProperties();
    for (auto& iter : screenProperties) {
        GetSceneSessionPrivacyModeBundles(iter.first, privacyBundleList);
    }
    std::unordered_map<DisplayId, bool> privacyBundleDisplayId;
    for (const auto& iter : privacyBundleList) {
        privacyBundleDisplayId[iter.first] = !iter.second.empty() || specialExtWindowHasPrivacyMode_.load();
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "isEmptyPrivateBundles=%{public}d", privacyBundleDisplayId.empty());
    ScreenSessionManagerClient::GetInstance().SetPrivacyStateByDisplayId(privacyBundleDisplayId);
}

void SceneSessionManager::GetSceneSessionPrivacyModeBundles(DisplayId displayId,
    std::unordered_map<DisplayId, std::unordered_set<std::string>>& privacyBundles)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [persistentId, sceneSession] : sceneSessionMap_) {
        if (sceneSession == nullptr) {
            TLOGE(WmsLogTag::WMS_MAIN, "scene session is nullptr, wid=%{public}d.", persistentId);
            continue;
        }
        auto sessionProperty = sceneSession->GetSessionProperty();
        auto currentDisplayId = sessionProperty->GetDisplayId();
        if (displayId != currentDisplayId) {
            continue;
        }
        bool isForeground = sceneSession->GetSessionState() == SessionState::STATE_FOREGROUND ||
                            sceneSession->GetSessionState() == SessionState::STATE_ACTIVE;
        if (isForeground && sceneSession->GetParentSession() != nullptr) {
            isForeground = isForeground &&
                (sceneSession->GetParentSession()->GetSessionState() == SessionState::STATE_FOREGROUND ||
                 sceneSession->GetParentSession()->GetSessionState() == SessionState::STATE_ACTIVE);
        }
        bool isPrivate = sessionProperty->GetPrivacyMode() ||
            sceneSession->GetCombinedExtWindowFlags().privacyModeFlag;
        bool IsSystemWindowVisible = sceneSession->GetSessionInfo().isSystem_ && sceneSession->IsVisible();
        if ((isForeground || IsSystemWindowVisible) && isPrivate) {
            TLOGI(WmsLogTag::WMS_ATTRIBUTE, "found privacy win=[%{public}d, %{public}s], display=%{public}" PRIu64,
                sceneSession->GetWindowId(), sceneSession->GetWindowName().c_str(), displayId);
            if (PcFoldScreenManager::GetInstance().IsHalfFolded(displayId) &&
                !PcFoldScreenManager::GetInstance().HasSystemKeyboard()) {
                UpdateSessionPrivacyForSuperFold(sceneSession, displayId, privacyBundles);
                continue;
            }
            if (!sceneSession->GetSessionInfo().bundleName_.empty()) {
                privacyBundles[displayId].insert(sceneSession->GetSessionInfo().bundleName_);
            } else {
                TLOGD(WmsLogTag::WMS_ATTRIBUTE, "bundle name is empty, wid=%{public}d.", persistentId);
                privacyBundles[displayId].insert(sceneSession->GetWindowName());
            }
        }
    }
    if (privacyBundles.empty()) {
        privacyBundles[displayId] = std::unordered_set<std::string>();
    }
}

void SceneSessionManager::UpdateSessionPrivacyForSuperFold(const sptr<SceneSession>& sceneSession, DisplayId displayId,
    std::unordered_map<DisplayId, std::unordered_set<std::string>>& privacyBundles)
{
    const auto& [defaultDisplayRect, virtualDisplayRect, foldCreaseRect] =
        PcFoldScreenManager::GetInstance().GetDisplayRects();
    auto sessionRect = sceneSession->GetSessionRect();
    if (sessionRect.IsInvalid()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "skip wid: %{public}d, invalid rect: %{public}s",
            sceneSession->GetPersistentId(), sessionRect.ToString().c_str());
        return;
    }
    auto sessionRectTop = sessionRect.posY_;
    auto sessionRectBottom = sessionRect.posY_ + sessionRect.height_;
    bool isSkipPrivacy = sessionRectTop > defaultDisplayRect.height_ && sessionRectBottom < virtualDisplayRect.posY_;
    bool isContainsDefaultScreen = sessionRectTop <= defaultDisplayRect.height_;
    bool isContainsVirtualScreen = sessionRectBottom >= virtualDisplayRect.posY_;
    std::string bundleName = !sceneSession->GetSessionInfo().bundleName_.empty()
        ? sceneSession->GetSessionInfo().bundleName_
        : sceneSession->GetWindowName();
    if (isSkipPrivacy) {
        return;
    }
    if (isContainsDefaultScreen) {
        privacyBundles[DEFAULT_DISPLAY_ID].insert(bundleName);
    }
    if (isContainsVirtualScreen) {
        privacyBundles[VIRTUAL_DISPLAY_ID].insert(bundleName);
    }
}


void SceneSessionManager::RegisterSessionStateChangeNotifyManagerFunc(sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is nullptr");
        return;
    }
    sceneSession->SetSessionStateChangeNotifyManagerListener(
        [this](int32_t persistentId, const SessionState& state) THREAD_SAFETY_GUARD(SCENE_GUARD) {
            OnSessionStateChange(persistentId, state);
        });
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void SceneSessionManager::RegisterSessionInfoChangeNotifyManagerFunc(sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is nullptr");
        return;
    }
    sceneSession->SetSessionInfoChangeNotifyManagerListener([this](int32_t persistentId) {
        NotifyWindowInfoChangeFromSession(persistentId);
    });
}

void SceneSessionManager::RegisterDisplayIdChangedNotifyManagerFunc(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is nullptr");
        return;
    }
    sceneSession->SetDisplayIdChangedNotifyManagerListener([this](int32_t persistentId, uint64_t displayId) {
        NotifyDisplayIdChanged(persistentId, displayId);
    });
}

void SceneSessionManager::RegisterRequestFocusStatusNotifyManagerFunc(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is nullptr");
        return;
    }
    sceneSession->SetRequestFocusStatusNotifyManagerListener(
        [this](int32_t persistentId, const bool isFocused, const bool byForeground, FocusChangeReason reason) {
            RequestFocusStatus(persistentId, isFocused, byForeground, reason);
        });
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void SceneSessionManager::RegisterGetStateFromManagerFunc(sptr<SceneSession>& sceneSession)
{
    GetStateFromManagerFunc func = [this](const ManagerState key) {
        switch (key) {
            case ManagerState::MANAGER_STATE_SCREEN_LOCKED:
                return this->IsScreenLocked();
                break;
            default:
                return false;
                break;
        }
    };
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is nullptr");
        return;
    }
    sceneSession->SetGetStateFromManagerListener(func);
    TLOGD(WmsLogTag::DEFAULT, "success");
}

void SceneSessionManager::RegisterSessionChangeByActionNotifyManagerFunc(sptr<SceneSession>& sceneSession)
{
    SessionChangeByActionNotifyManagerFunc func = [this](const sptr<SceneSession>& sceneSession,
        const sptr<WindowSessionProperty>& property, WSPropertyChangeAction action) {
        if (sceneSession == nullptr || property == nullptr) {
            TLOGNE(WmsLogTag::DEFAULT, "params is nullptr");
            return;
        }
        switch (action) {
            case WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON:
                HandleKeepScreenOn(sceneSession, property->IsKeepScreenOn(), WINDOW_SCREEN_LOCK_PREFIX,
                                   sceneSession->keepScreenLock_);
                break;
            case WSPropertyChangeAction::ACTION_UPDATE_VIEW_KEEP_SCREEN_ON:
                HandleKeepScreenOn(sceneSession, property->IsViewKeepScreenOn(), VIEW_SCREEN_LOCK_PREFIX,
                                   sceneSession->viewKeepScreenLock_);
                break;
            case WSPropertyChangeAction::ACTION_UPDATE_FOCUSABLE:
            case WSPropertyChangeAction::ACTION_UPDATE_TOUCHABLE:
            case WSPropertyChangeAction::ACTION_UPDATE_OTHER_PROPS:
            case WSPropertyChangeAction::ACTION_UPDATE_STATUS_PROPS:
            case WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_PROPS:
            case WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS:
            case WSPropertyChangeAction::ACTION_UPDATE_FOLLOW_SCREEN_CHANGE:
                NotifyWindowInfoChange(property->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_PROPERTY);
                break;
            case WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS:
                SetBrightness(sceneSession, property->GetBrightness());
                break;
            case WSPropertyChangeAction::ACTION_UPDATE_PRIVACY_MODE:
            case WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE:
                UpdatePrivateStateAndNotify(property->GetPersistentId());
                break;
            case WSPropertyChangeAction::ACTION_UPDATE_FLAGS:
                CheckAndNotifyWaterMarkChangedResult();
                NotifyWindowInfoChange(property->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_PROPERTY);
                break;
            case WSPropertyChangeAction::ACTION_UPDATE_MODE:
                ProcessWindowModeType();
                NotifyWindowInfoChange(property->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_PROPERTY);
                break;
            case WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS:
                HandleHideNonSystemFloatingWindows(property, sceneSession);
                break;
            case WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK:
                FlushWindowInfoToMMI();
                break;
            default:
                break;
        }
    };
    if (sceneSession != nullptr) {
        sceneSession->SetSessionChangeByActionNotifyManagerListener(func);
    }
}

__attribute__((no_sanitize("cfi"))) void SceneSessionManager::OnSessionStateChange(
    int32_t persistentId, const SessionState& state)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:OnSessionStateChange%d", persistentId);
    TLOGD(WmsLogTag::DEFAULT, "id: %{public}d, state:%{public}u", persistentId, state);
    SceneSessionChangeInfo changeInfo {
        .persistentId_ = persistentId,
        .changeInfo_ = "Session state change to " + std::to_string(static_cast<uint32_t>(state)),
        .logTag_ = WmsLogTag::WMS_LIFE,
    };
    SessionChangeRecorder::GetInstance().RecordSceneSessionChange(RecordType::SESSION_STATE_RECORD, changeInfo);
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGD(WmsLogTag::DEFAULT, "session is nullptr");
        return;
    }
    switch (state) {
        case SessionState::STATE_FOREGROUND:
            ProcessFocusWhenForeground(sceneSession);
            if (!IsSessionVisibleForeground(sceneSession)) {
                sceneSession->SetPostProcessProperty(true);
                break;
            }
            UpdateForceHideState(sceneSession, sceneSession->GetSessionProperty(), true);
            NotifyWindowInfoChange(persistentId, WindowUpdateType::WINDOW_UPDATE_ADDED);
            HandleKeepScreenOn(sceneSession, sceneSession->IsKeepScreenOn(), WINDOW_SCREEN_LOCK_PREFIX,
                               sceneSession->keepScreenLock_);
            HandleKeepScreenOn(sceneSession, sceneSession->IsViewKeepScreenOn(), VIEW_SCREEN_LOCK_PREFIX,
                               sceneSession->viewKeepScreenLock_);
            UpdatePrivateStateAndNotify(persistentId);
            if (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
                ProcessSubSessionForeground(sceneSession);
            }
            break;
        case SessionState::STATE_BACKGROUND:
            NotifySessionUpdate(sceneSession->GetSessionInfo(), ActionType::SINGLE_BACKGROUND);
            RequestSessionUnfocus(persistentId, FocusChangeReason::APP_BACKGROUND);
            UpdateForceHideState(sceneSession, sceneSession->GetSessionProperty(), false);
            NotifyWindowInfoChange(persistentId, WindowUpdateType::WINDOW_UPDATE_REMOVED);
            HandleKeepScreenOn(sceneSession, false, WINDOW_SCREEN_LOCK_PREFIX, sceneSession->keepScreenLock_);
            HandleKeepScreenOn(sceneSession, false, VIEW_SCREEN_LOCK_PREFIX, sceneSession->viewKeepScreenLock_);
            UpdatePrivateStateAndNotify(persistentId);
            if (persistentId == brightnessSessionId_) {
                auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
                auto focusedSessionId = windowFocusController_->GetFocusedSessionId(displayId);
                UpdateBrightness(focusedSessionId);
            }
            if (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
                ProcessSubSessionBackground(sceneSession);
            }
            break;
        case SessionState::STATE_CONNECT:
            SetSessionSnapshotSkipForAppProcess(sceneSession);
            SetSessionSnapshotSkipForAppBundleName(sceneSession);
            SetSessionWatermarkForAppProcess(sceneSession);
            SetWatermarkForSession(sceneSession);
            break;
        case SessionState::STATE_DISCONNECT:
            ClearWatermarkForSession(sceneSession);
            break;
        default:
            break;
    }
}

void SceneSessionManager::ProcessFocusWhenForeground(sptr<SceneSession>& sceneSession)
{
    auto persistentId = sceneSession->GetPersistentId();
    auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    auto focusGroup = windowFocusController_->GetFocusGroup(displayId);
    if (focusGroup == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, displayId);
        return;
    }
    bool needBlockNotifyFocusStatusUntilForeground = focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground();
    bool needBlockNotifyUnfocusStatus = focusGroup->GetNeedBlockNotifyUnfocusStatus();
    if (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW &&
        persistentId == focusGroup->GetFocusedSessionId()) {
        if (focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground()) {
            focusGroup->SetNeedBlockNotifyFocusStatusUntilForeground(false);
            focusGroup->SetNeedBlockNotifyUnfocusStatus(false);
            auto focusNotifyInfo = sptr<FocusNotifyInfo>::MakeSptr(focusGroup->GetUpdateFocusTimeStamp(),
                focusGroup->GetLastFocusedSessionId(), sceneSession->GetPersistentId(), false);
            NotifyFocusStatus(sceneSession, true, focusGroup, focusNotifyInfo);
            auto highlightNotifyInfo = sptr<HighlightNotifyInfo>::MakeSptr(focusGroup->GetUpdateFocusTimeStamp(),
                std::vector<int32_t>(), sceneSession->GetPersistentId(), false);
            sceneSession->NotifyHighlightChange(highlightNotifyInfo, true);
        }
    } else if (!sceneSession->IsFocusedOnShow()) {
        if (IsSessionVisibleForeground(sceneSession)) {
            sceneSession->SetFocusedOnShow(true);
        }
    } else {
        if (Session::IsScbCoreEnabled()) {
            ProcessFocusWhenForegroundScbCore(sceneSession);
        } else {
            RequestSessionFocus(persistentId, true, FocusChangeReason::APP_FOREGROUND);
        }
    }
}

void SceneSessionManager::ProcessFocusWhenForegroundScbCore(sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGD(WmsLogTag::WMS_FOCUS, "session is nullptr");
        return;
    }
    if (sceneSession->IsFocusableOnShow()) {
        if (IsSessionVisibleForeground(sceneSession)) {
            RequestSessionFocus(sceneSession->GetPersistentId(), true, FocusChangeReason::APP_FOREGROUND);
        } else {
            PostProcessFocusState state = {true, true, true, FocusChangeReason::APP_FOREGROUND};
            sceneSession->SetPostProcessFocusState(state);
        }
    } else {
        TLOGD(WmsLogTag::WMS_FOCUS, "win: %{public}d ignore request focus when foreground",
            sceneSession->GetPersistentId());
    }
}

void SceneSessionManager::ProcessWindowModeType()
{
    if (isScreenLocked_) {
        return;
    }
    NotifyRSSWindowModeTypeUpdate();
}

static bool IsSmallFoldProduct()
{
    static const std::string foldScreenType = system::GetParameter("const.window.foldscreen.type", "");
    if (foldScreenType.empty()) {
        return false;
    }
    return foldScreenType[0] == SMALL_FOLD_PRODUCT_TYPE;
}

bool SceneSessionManager::IsInDefaultScreen(const sptr<SceneSession>& sceneSession)
{
    ScreenId defaultScreenId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    return sceneSession->GetSessionProperty()->GetDisplayId() == defaultScreenId;
}

bool SceneSessionManager::IsNeedSkipWindowModeTypeCheck(const sptr<SceneSession>& sceneSession, bool isSmallFold)
{
    if (sceneSession == nullptr ||
        !WindowHelper::IsMainWindow(sceneSession->GetWindowType()) ||
        !sceneSession->GetRSVisible() ||
        !sceneSession->IsSessionForeground()) {
        return true;
    }
    if (isSmallFold && !IsInDefaultScreen(sceneSession)) {
        return true;
    }
    return false;
}

WindowModeType SceneSessionManager::CheckWindowModeType()
{
    bool inSplit = false;
    bool inFloating = false;
    bool fullScreen = false;
    bool isSmallFold = IsSmallFoldProduct();
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& session : sceneSessionMap_) {
            if (IsNeedSkipWindowModeTypeCheck(session.second, isSmallFold)) {
                continue;
            }
            auto mode = session.second->GetWindowMode();
            if (mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY || mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
                inSplit = true;
            }
            if (mode == WindowMode::WINDOW_MODE_FLOATING) {
                inFloating = true;
            }
            if (WindowHelper::IsFullScreenWindow(mode)) {
                fullScreen = true;
            }
        }
    }

    WindowModeType type;
    if (inSplit) {
        if (inFloating) {
            type = WindowModeType::WINDOW_MODE_SPLIT_FLOATING;
        } else {
            type = WindowModeType::WINDOW_MODE_SPLIT;
        }
    } else {
        if (inFloating) {
            if (fullScreen) {
                type = WindowModeType::WINDOW_MODE_FULLSCREEN_FLOATING;
            } else {
                type = WindowModeType::WINDOW_MODE_FLOATING;
            }
        } else if (fullScreen) {
            type = WindowModeType::WINDOW_MODE_FULLSCREEN;
        } else {
            type = WindowModeType::WINDOW_MODE_OTHER;
        }
    }
    return type;
}

void SceneSessionManager::NotifyRSSWindowModeTypeUpdate()
{
    WindowModeType type = CheckWindowModeType();
    if (lastWindowModeType_ == type) {
        return;
    }
    lastWindowModeType_ = type;
    TLOGI(WmsLogTag::WMS_MAIN, "Notify RSS Window Mode Type Update, type : %{public}d",
        static_cast<uint8_t>(type));
    SessionManagerAgentController::GetInstance().UpdateWindowModeTypeInfo(type);
}

void SceneSessionManager::ProcessSubSessionForeground(sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGD(WmsLogTag::WMS_SUB, "session is nullptr");
        return;
    }
    std::vector<sptr<Session>> modalVec = sceneSession->GetDialogVector();
    for (auto& subSession : sceneSession->GetSubSession()) {
        if (subSession == nullptr) {
            TLOGD(WmsLogTag::WMS_SUB, "sub session is nullptr");
            continue;
        }
        if (!subSession->GetSubSession().empty()) {
            ProcessSubSessionForeground(subSession);
        }
        if (subSession->IsTopmost()) {
            modalVec.push_back(subSession);
            TLOGD(WmsLogTag::WMS_SUB, "sub session is topmost modal sub window");
            continue;
        }
        const auto& state = subSession->GetSessionState();
        if (state != SessionState::STATE_FOREGROUND && state != SessionState::STATE_ACTIVE) {
            TLOGD(WmsLogTag::WMS_SUB, "sub session is not active");
            continue;
        }
        RequestSessionFocus(subSession->GetPersistentId(), true);
        NotifyWindowInfoChange(subSession->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_ADDED);
        HandleKeepScreenOn(subSession, subSession->IsKeepScreenOn(), WINDOW_SCREEN_LOCK_PREFIX,
                           subSession->keepScreenLock_);
        HandleKeepScreenOn(subSession, subSession->IsViewKeepScreenOn(), VIEW_SCREEN_LOCK_PREFIX,
                           subSession->viewKeepScreenLock_);
    }

    for (const auto& modal : modalVec) {
        if (modal == nullptr) {
            TLOGD(WmsLogTag::WMS_DIALOG, "dialog or topmost modal sub window is nullptr");
            continue;
        }
        const auto& state = modal->GetSessionState();
        if (state != SessionState::STATE_FOREGROUND && state != SessionState::STATE_ACTIVE) {
            TLOGD(WmsLogTag::WMS_DIALOG, "dialog or topmost modal sub window is not active");
            continue;
        }
        auto modalSession = GetSceneSession(modal->GetPersistentId());
        if (modalSession == nullptr) {
            TLOGD(WmsLogTag::WMS_DIALOG, "modalSession is null");
            continue;
        }
        NotifyWindowInfoChange(modal->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_ADDED);
        auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
        auto focusGroup = windowFocusController_->GetFocusGroup(displayId);
        if (focusGroup == nullptr) {
            TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, displayId);
            return;
        }
        auto focusedSessionId = focusGroup->GetFocusedSessionId();
        bool needBlockNotifyFocusStatusUntilForeground = focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground();
        if (modal->GetPersistentId() == focusedSessionId && needBlockNotifyFocusStatusUntilForeground) {
            focusGroup->SetNeedBlockNotifyFocusStatusUntilForeground(false);
            focusGroup->SetNeedBlockNotifyUnfocusStatus(false);
            auto focusNotifyInfo = sptr<FocusNotifyInfo>::MakeSptr(focusGroup->GetUpdateFocusTimeStamp(),
                focusGroup->GetLastFocusedSessionId(), modalSession->GetPersistentId(), false);
            NotifyFocusStatus(modalSession, true, focusGroup, focusNotifyInfo);
        }
        HandleKeepScreenOn(modalSession, modalSession->IsKeepScreenOn(), WINDOW_SCREEN_LOCK_PREFIX,
                           modalSession->keepScreenLock_);
        HandleKeepScreenOn(modalSession, modalSession->IsViewKeepScreenOn(), VIEW_SCREEN_LOCK_PREFIX,
                           modalSession->viewKeepScreenLock_);
    }
}

WSError SceneSessionManager::ProcessModalTopmostRequestFocusImmediately(const sptr<SceneSession>& sceneSession)
{
    // focus must on modal topmost subwindow when APP_MAIN_WINDOW or sub window request focus
    sptr<SceneSession> mainSession = nullptr;
    if (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        mainSession = sceneSession;
    } else if (SessionHelper::IsSubWindow(sceneSession->GetWindowType())) {
        mainSession = GetSceneSession(sceneSession->GetParentPersistentId());
    }
    if (mainSession == nullptr) {
        TLOGD(WmsLogTag::WMS_FOCUS, "main window is nullptr");
        return WSError::WS_DO_NOTHING;
    }

    std::vector<sptr<SceneSession>> topmostVec;
    for (auto subSession : mainSession->GetSubSession()) {
        if (subSession && subSession->IsTopmost()) {
            topmostVec.push_back(subSession);
        }
    }
    auto displayId = mainSession->GetSessionProperty()->GetDisplayId();
    auto focusedSessionId = windowFocusController_->GetFocusedSessionId(displayId);
    auto conditionFunc =  [this, focusedSessionId](const sptr<SceneSession>& iter) {
        return iter && iter->GetPersistentId() == focusedSessionId;
    };
    if (std::find_if(topmostVec.begin(), topmostVec.end(), std::move(conditionFunc)) != topmostVec.end()) {
        TLOGD(WmsLogTag::WMS_SUB, "modal topmost subwindow id: %{public}d has been focused!", focusedSessionId);
        return WSError::WS_OK;
    }
    WSError ret = WSError::WS_DO_NOTHING;
    for (auto topmostSession : topmostVec) {
        if (topmostSession == nullptr) {
            continue;
        }
        // no need to consider order, since rule of zOrder
        if (RequestSessionFocusImmediately(topmostSession->GetPersistentId(), false) == WSError::WS_OK) {
            ret = WSError::WS_OK;
        }
    }
    return ret;
}

WSError SceneSessionManager::ProcessSubWindowRequestFocusImmediately(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGD(WmsLogTag::WMS_FOCUS, "session is null");
        return WSError::WS_DO_NOTHING;
    }
    std::vector<sptr<SceneSession>> subSessionVec = sceneSession->GetSubSession();
    if (subSessionVec.empty()) {
        TLOGD(WmsLogTag::WMS_FOCUS, "has no sub window");
        return WSError::WS_DO_NOTHING;
    }
    auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    auto focusedSessionId = windowFocusController_->GetFocusedSessionId(displayId);
    auto conditionFunc =  [this, focusedSessionId](const sptr<SceneSession>& iter) {
        return iter && iter->GetPersistentId() == focusedSessionId;
    };
    if (std::find_if(subSessionVec.begin(), subSessionVec.end(), std::move(conditionFunc)) != subSessionVec.end()) {
        TLOGD(WmsLogTag::WMS_SUB, "sub window id: %{public}d has been focused!", focusedSessionId);
        return WSError::WS_OK;
    }
    WSError ret = WSError::WS_DO_NOTHING;
    for (auto session : subSessionVec) {
        if (session == nullptr) {
            TLOGD(WmsLogTag::WMS_SUB, "sub session is nullptr");
            continue;
        }
        if (!IsSessionVisibleForeground(session)) {
            continue;
        }
        if (RequestSessionFocus(session->GetPersistentId(), true, FocusChangeReason::REQUEST_WITH_CHECK_SUB_WINDOW) ==
            WSError::WS_OK) {
            ret = WSError::WS_OK;
        }
    }
    return ret;
}

WSError SceneSessionManager::ProcessDialogRequestFocusImmediately(const sptr<SceneSession>& sceneSession)
{
    // focus must on dialog when APP_MAIN_WINDOW or sub window request focus
    sptr<SceneSession> mainSession = nullptr;
    if (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        mainSession = sceneSession;
    } else if (SessionHelper::IsSubWindow(sceneSession->GetWindowType())) {
        mainSession = GetSceneSession(sceneSession->GetParentPersistentId());
    }
    if (mainSession == nullptr) {
        TLOGD(WmsLogTag::WMS_FOCUS, "main window is nullptr");
        return WSError::WS_DO_NOTHING;
    }
    std::vector<sptr<Session>> dialogVec = mainSession->GetDialogVector();
    auto displayId = mainSession->GetSessionProperty()->GetDisplayId();
    auto focusedSessionId = windowFocusController_->GetFocusedSessionId(displayId);
    auto conditionFunc =  [this, focusedSessionId](const sptr<Session>& iter) {
        return iter && iter->GetPersistentId() == focusedSessionId;
    };
    if (std::find_if(dialogVec.begin(), dialogVec.end(), std::move(conditionFunc)) != dialogVec.end()) {
        TLOGD(WmsLogTag::WMS_DIALOG, "dialog id: %{public}d has been focused!", focusedSessionId);
        return WSError::WS_OK;
    }
    WSError ret = WSError::WS_DO_NOTHING;
    for (auto dialog : dialogVec) {
        if (dialog == nullptr) {
            continue;
        }
        // no need to consider order, since rule of zOrder
        if (RequestSessionFocusImmediately(dialog->GetPersistentId(), false) == WSError::WS_OK) {
            ret = WSError::WS_OK;
        }
    }
    return ret;
}

void SceneSessionManager::ProcessSubSessionBackground(sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGD(WmsLogTag::WMS_SUB, "session is nullptr");
        return;
    }
    for (auto& subSession : sceneSession->GetSubSession()) {
        if (subSession == nullptr) {
            TLOGD(WmsLogTag::WMS_SUB, "sub session is nullptr");
            continue;
        }
        if (!subSession->GetSubSession().empty()) {
            ProcessSubSessionBackground(subSession);
        }
        const auto& state = subSession->GetSessionState();
        if (state != SessionState::STATE_FOREGROUND && state != SessionState::STATE_ACTIVE) {
            TLOGD(WmsLogTag::WMS_SUB, "sub session is not active");
            continue;
        }
        NotifyWindowInfoChange(subSession->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_REMOVED);
        HandleKeepScreenOn(subSession, false, WINDOW_SCREEN_LOCK_PREFIX, subSession->keepScreenLock_);
        HandleKeepScreenOn(subSession, false, VIEW_SCREEN_LOCK_PREFIX, subSession->viewKeepScreenLock_);
        UpdatePrivateStateAndNotify(subSession->GetPersistentId());
    }
    std::vector<sptr<Session>> dialogVec = sceneSession->GetDialogVector();
    for (const auto& dialog : dialogVec) {
        if (dialog == nullptr) {
            TLOGD(WmsLogTag::WMS_DIALOG, "dialog is nullptr");
            continue;
        }
        auto dialogSession = GetSceneSession(dialog->GetPersistentId());
        if (dialogSession == nullptr) {
            TLOGD(WmsLogTag::WMS_DIALOG, "dialogSession is null");
            continue;
        }
        NotifyWindowInfoChange(dialog->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_REMOVED);
        HandleKeepScreenOn(dialogSession, false, WINDOW_SCREEN_LOCK_PREFIX, dialogSession->keepScreenLock_);
        HandleKeepScreenOn(dialogSession, false, VIEW_SCREEN_LOCK_PREFIX, dialogSession->viewKeepScreenLock_);
        UpdatePrivateStateAndNotify(dialog->GetPersistentId());
    }
    for (const auto& toastSession : sceneSession->GetToastSession()) {
        if (toastSession == nullptr) {
            TLOGD(WmsLogTag::WMS_TOAST, "toastSession session is nullptr");
            continue;
        }
        const auto& state = toastSession->GetSessionState();
        if (state != SessionState::STATE_FOREGROUND && state != SessionState::STATE_ACTIVE) {
            TLOGD(WmsLogTag::WMS_TOAST, "toast session is not active");
            continue;
        }
        NotifyWindowInfoChange(toastSession->GetPersistentId(), WindowUpdateType::WINDOW_UPDATE_REMOVED);
        HandleKeepScreenOn(toastSession, false, WINDOW_SCREEN_LOCK_PREFIX, toastSession->keepScreenLock_);
        HandleKeepScreenOn(toastSession, false, VIEW_SCREEN_LOCK_PREFIX, toastSession->viewKeepScreenLock_);
        UpdatePrivateStateAndNotify(toastSession->GetPersistentId());
        toastSession->SetActive(false);
        toastSession->BackgroundTask();
    }
}

WSError SceneSessionManager::SetWindowFlags(const sptr<SceneSession>& sceneSession,
    const sptr<WindowSessionProperty>& property)
{
    if (sceneSession == nullptr) {
        TLOGD(WmsLogTag::DEFAULT, "session is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    auto sessionProperty = sceneSession->GetSessionProperty();
    uint32_t flags = property->GetWindowFlags();
    uint32_t oldFlags = sessionProperty->GetWindowFlags();
    if (((oldFlags ^ flags) == static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED) ||
         (oldFlags ^ flags) == static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK)) &&
        !property->GetSystemCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "permission denied");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    sessionProperty->SetWindowFlags(flags);
    CheckAndNotifyWaterMarkChangedResult();
    if ((oldFlags ^ flags) == static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED)) {
        sceneSession->OnShowWhenLocked(flags & static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED));
    }
    TLOGI(WmsLogTag::DEFAULT, "set flags: %{public}u", flags);
    return WSError::WS_OK;
}

void SceneSessionManager::CheckAndNotifyWaterMarkChangedResult()
{
    bool currentWaterMarkShowState = false;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, session] : sceneSessionMap_) {
            if (!session) {
                continue;
            }
            bool hasWaterMark = session->GetSessionProperty()->GetWindowFlags() &
                static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK);
            bool isExtWindowHasWaterMarkFlag = session->GetCombinedExtWindowFlags().waterMarkFlag;
            if ((hasWaterMark && session->GetRSVisible()) || isExtWindowHasWaterMarkFlag) {
                currentWaterMarkShowState = true;
                break;
            }
        }
        if (combinedExtWindowFlags_.waterMarkFlag) {
            TLOGI(WmsLogTag::WMS_UIEXT, "CheckAndNotifyWaterMarkChangedResult scb uiext has water mark");
            currentWaterMarkShowState = true;
        }
    }
    if (lastWaterMarkShowState_ != currentWaterMarkShowState) {
        lastWaterMarkShowState_ = currentWaterMarkShowState;
        NotifyWaterMarkFlagChangedResult(currentWaterMarkShowState);
    }
}

WSError SceneSessionManager::NotifyWaterMarkFlagChangedResult(bool hasWaterMark)
{
    TLOGD(WmsLogTag::DEFAULT, "hasWaterMark: %{public}u", static_cast<uint32_t>(hasWaterMark));
    SessionManagerAgentController::GetInstance().NotifyWaterMarkFlagChangedResult(hasWaterMark);
    return WSError::WS_OK;
}

void SceneSessionManager::ProcessPreload(const AppExecFwk::AbilityInfo& abilityInfo) const
{
    if (!bundleMgr_) {
        TLOGE(WmsLogTag::DEFAULT, "bundle manager is nullptr.");
        return;
    }

    AAFwk::Want want;
    want.SetElementName(abilityInfo.deviceId, abilityInfo.bundleName, abilityInfo.name, abilityInfo.moduleName);
    auto uid = abilityInfo.uid;
    want.SetParam("uid", uid);
    bundleMgr_->ProcessPreload(want);
}

void SceneSessionManager::NotifyCompleteFirstFrameDrawing(int32_t persistentId)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, " sceneSession is nullptr.");
        return;
    }

    const auto& sessionInfo = sceneSession->GetSessionInfo();
    if (IsAtomicServiceFreeInstall(sessionInfo)) {
        TLOGI(WmsLogTag::WMS_LIFE, "AtomicService free-install start, id: %{public}d, type: %{public}d",
            sceneSession->GetPersistentId(), sceneSession->GetWindowType());
        FillSessionInfo(sceneSession);
    }
    TLOGI(WmsLogTag::WMS_MAIN, " id: %{public}d, app info: [%{public}s %{public}s %{public}s]",
        sceneSession->GetPersistentId(), sessionInfo.bundleName_.c_str(),
        sessionInfo.moduleName_.c_str(), sessionInfo.abilityName_.c_str());
    auto abilityInfo = sceneSession->GetSessionInfoAbilityInfo();
    if (abilityInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, " abilityInfo is null, Id: %{public}d", persistentId);
        return;
    }

    [this, persistentId] {
        auto task = [persistentId] {
            AAFwk::AbilityManagerClient::GetInstance()->CompleteFirstFrameDrawing(persistentId);
        };
        TLOGNI(WmsLogTag::DEFAULT, "Post CompleteFirstFrameDrawing task. Id: %{public}d", persistentId);
        eventHandler_->PostTask(task, "wms:CompleteFirstFrameDrawing", 0);
    }();

    auto task = [this, abilityInfo, sceneSession, persistentId] {
        if (!sceneSession->GetSessionInfo().isSystem_ && !(abilityInfo->excludeFromMissions)) {
            TLOGND(WmsLogTag::WMS_PATTERN, "NotifyCreated, id: %{public}d", persistentId);
            listenerController_->NotifySessionLifecycleEvent(
                ISessionLifecycleListener::SessionLifecycleEvent::CREATED, sceneSession->GetSessionInfo());
        }
        ProcessPreload(*abilityInfo);
    };

    if (sceneSession->GetLeashWinSurfaceNode()) {
        SetSkipSelfWhenShowOnVirtualScreen(sceneSession->GetLeashWinSurfaceNode()->GetId(),
            sceneSession->GetSkipSelfWhenShowOnVirtualScreen());
    }
    return taskScheduler_->PostAsyncTask(task, "NotifyCompleteFirstFrameDrawing" + std::to_string(persistentId));
}

void SceneSessionManager::NotifySessionMovedToFront(int32_t persistentId)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session is invalid with %{public}d", persistentId);
        return;
    }
    TLOGI(WmsLogTag::DEFAULT, "id: %{public}d, system: %{public}d", sceneSession->GetPersistentId(),
           sceneSession->GetSessionInfo().isSystem_);
    if (!sceneSession->GetSessionInfo().isSystem_ &&
        sceneSession->GetSessionInfo().abilityInfo &&
        !(sceneSession->GetSessionInfo().abilityInfo)->excludeFromMissions) {
        listenerController_->NotifySessionMovedToFront(persistentId);
    }
}

WSError SceneSessionManager::SetSessionLabel(const sptr<IRemoteObject>& token, const std::string& label)
{
    TLOGI(WmsLogTag::WMS_MAIN, "in");
    const char* const where = __func__;
    auto task = [this, &token, &label, where]() {
        auto sceneSession = FindSessionByToken(token);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "fail to find session by token");
            return WSError::WS_ERROR_SET_SESSION_LABEL_FAILED;
        }
        sceneSession->SetSessionLabel(label);
        TLOGNI(WmsLogTag::WMS_MAIN, "%{public}s id: %{public}d, system: %{public}d",
            where, sceneSession->GetPersistentId(), sceneSession->GetSessionInfo().isSystem_);
        if (!sceneSession->GetSessionInfo().isSystem_) {
            TLOGND(WmsLogTag::WMS_MAIN, "%{public}s id: %{public}d", where, sceneSession->GetPersistentId());
            listenerController_->NotifySessionLabelUpdated(sceneSession->GetPersistentId());
        }
        return WSError::WS_OK;
    };
    return taskScheduler_->PostSyncTask(task, where);
}

WSError SceneSessionManager::SetSessionIcon(const sptr<IRemoteObject>& token,
    const std::shared_ptr<Media::PixelMap>& icon)
{
    TLOGI(WmsLogTag::WMS_MAIN, "in");
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::WMS_MAIN, "The caller is not system-app, can not use system-api");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    return SetSessionIconInner(token, icon);
}

WSError SceneSessionManager::SetSessionIconInner(const sptr<IRemoteObject>& token,
    const std::shared_ptr<Media::PixelMap>& icon)
{
    TLOGI(WmsLogTag::WMS_MAIN, "in");
    const char* const where = __func__;
    auto task = [this, &token, &icon, where]() {
        auto sceneSession = FindSessionByToken(token);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "fail to find session by token");
            return WSError::WS_ERROR_SET_SESSION_LABEL_FAILED;
        }
        sceneSession->SetSessionIcon(icon);
        TLOGNI(WmsLogTag::WMS_MAIN, "%{public}s id: %{public}d, system: %{public}d",
            where, sceneSession->GetPersistentId(), sceneSession->GetSessionInfo().isSystem_);
        if (!sceneSession->GetSessionInfo().isSystem_ &&
            sceneSession->GetSessionInfo().abilityInfo &&
            !(sceneSession->GetSessionInfo().abilityInfo)->excludeFromMissions) {
            TLOGND(WmsLogTag::WMS_MAIN, "%{public}s id: %{public}d", where, sceneSession->GetPersistentId());
            listenerController_->NotifySessionIconChanged(sceneSession->GetPersistentId(), icon);
        }
        return WSError::WS_OK;
    };
    return taskScheduler_->PostSyncTask(task, where);
}

WSError SceneSessionManager::SetSessionIconForThirdParty(const sptr<IRemoteObject>& token,
    const std::shared_ptr<Media::PixelMap>& icon)
{
    TLOGI(WmsLogTag::WMS_MAIN, "in");
    if (token == nullptr || icon == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "invalid param");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (!systemConfig_.IsPcWindow()) {
        TLOGE(WmsLogTag::WMS_MAIN, "device not support");
        return WSError::WS_ERROR_DEVICE_NOT_SUPPORT;
    }
    auto sceneSession = FindSessionByToken(token);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "fail to find session by token");
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    if (!WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_MAIN, "id: %{public}d is not main window", sceneSession->GetPersistentId());
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    if (icon->GetWidth() != icon->GetHeight()) {
        TLOGE(WmsLogTag::WMS_MAIN, "icon is not a square");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    if (icon->GetCapacity() > ICON_MAX_SIZE) {
        TLOGE(WmsLogTag::WMS_MAIN, "icon is too large");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    return SetSessionIconInner(token, icon);
}

WSError SceneSessionManager::IsValidSessionIds(
    const std::vector<int32_t>& sessionIds, std::vector<bool>& results)
{
    TLOGI(WmsLogTag::DEFAULT, "in");
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (auto i = 0; i < static_cast<int32_t>(sessionIds.size()); ++i) {
        auto search = sceneSessionMap_.find(sessionIds.at(i));
        if (search == sceneSessionMap_.end() || search->second == nullptr) {
            results.push_back(false);
            continue;
        }
        results.push_back(true);
    }
    return WSError::WS_OK;
}

WSError SceneSessionManager::RegisterSessionListener(const sptr<ISessionListener>& listener)
{
    TLOGI(WmsLogTag::DEFAULT, "in");
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::DEFAULT, "not system-app, can not use system-api");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "permission denied");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    auto task = [this, &listener] {
        WSError ret = listenerController_->AddSessionListener(listener);
        // app continue report for distributed scheduled service
        SingletonContainer::Get<DmsReporter>().ReportContinueApp(ret == WSError::WS_OK,
            static_cast<int32_t>(ret));
        return ret;
    };
    return taskScheduler_->PostSyncTask(task, "AddSessionListener");
}

WSError SceneSessionManager::UnRegisterSessionListener(const sptr<ISessionListener>& listener)
{
    TLOGI(WmsLogTag::DEFAULT, "in");
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::DEFAULT, "not system-app, can not use system-api");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "permission denied");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    auto task = [this, &listener] {
        listenerController_->DelSessionListener(listener);
        return WSError::WS_OK;
    };
    return taskScheduler_->PostSyncTask(task, "DelSessionListener");
}

WSError SceneSessionManager::GetSessionInfos(const std::string& deviceId, int32_t numMax,
                                             std::vector<SessionInfoBean>& sessionInfos)
{
    TLOGI(WmsLogTag::DEFAULT, "num max %{public}d", numMax);
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::DEFAULT, "The caller is not system-app, can not use system-api");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!SessionPermission::VerifySessionPermission()) {
        TLOGE(WmsLogTag::DEFAULT, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    auto task = [this, &deviceId, numMax, &sessionInfos]() {
        if (CheckIsRemote(deviceId)) {
            int ret = GetRemoteSessionInfos(deviceId, numMax, sessionInfos);
            if (ret != ERR_OK) {
                return WSError::WS_ERROR_INVALID_PARAM;
            } else {
                return WSError::WS_OK;
            }
        }
        std::map<int32_t, sptr<SceneSession>>::iterator iter;
        std::vector<sptr<SceneSession>> sceneSessionInfos;
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                TLOGNE(WmsLogTag::WMS_LIFE, "session is nullptr");
                continue;
            }
            const auto& sessionInfo = sceneSession->GetSessionInfo();
            if (sessionInfo.isSystem_) {
                TLOGND(WmsLogTag::WMS_LIFE, "sessionId: %{public}d is SystemScene", sceneSession->GetPersistentId());
                continue;
            }
            auto want = sessionInfo.want;
            if (want == nullptr || sessionInfo.bundleName_.empty() || want->GetElement().GetBundleName().empty()) {
                TLOGNE(WmsLogTag::WMS_LIFE, "session: %{public}d, want is null or bundleName is empty "
                    "or want bundleName is empty", sceneSession->GetPersistentId());
                continue;
            }
            if (static_cast<int>(sceneSessionInfos.size()) >= numMax) {
                break;
            }
            TLOGND(WmsLogTag::WMS_LIFE, "GetSessionInfos session: %{public}d, bundleName:%{public}s",
                sceneSession->GetPersistentId(), sessionInfo.bundleName_.c_str());
            sceneSessionInfos.emplace_back(sceneSession);
        }
        return SceneSessionConverter::ConvertToMissionInfos(sceneSessionInfos, sessionInfos);
    };
    return taskScheduler_->PostSyncTask(task, "GetSessionInfos");
}

WSError SceneSessionManager::GetMainWindowStatesByPid(int32_t pid, std::vector<MainWindowState>& windowStates)
{
    TLOGI(WmsLogTag::WMS_LIFE, "pid:%{public}d", pid);
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_LIFE, "Get all mainWindow states failed, only support SA calling.");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    if (pid < 0) {
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    auto task = [this, pid, &windowStates] {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession != nullptr && sceneSession->GetCallingPid() == pid &&
                WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
                MainWindowState windowState;
                windowState.state_ = static_cast<int32_t>(sceneSession->GetSessionState());
                windowState.isVisible_ = sceneSession->GetRSVisible();
                windowState.isForegroundInteractive_ = sceneSession->GetForegroundInteractiveStatus();
                windowState.isPcOrPadEnableActivation_ = sceneSession->IsPcOrPadEnableActivation();
                windowStates.emplace_back(windowState);
            }
        }
        return WSError::WS_OK;
    };
    return taskScheduler_->PostSyncTask(task, "GetMainWindowStatesByPid");
}

#ifdef WINDOW_MANAGER_FEATURE_SUPPORT_DMSFWK
int SceneSessionManager::GetRemoteSessionInfos(const std::string& deviceId, int32_t numMax,
                                               std::vector<SessionInfoBean>& sessionInfos)
{
    TLOGI(WmsLogTag::DEFAULT, "begin");
    int result = DistributedClient::GetInstance().GetMissionInfos(deviceId, numMax, sessionInfos);
    if (result != ERR_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed, result=%{public}d", result);
        return result;
    }
    return ERR_OK;
}
#endif

WSError SceneSessionManager::GetSessionInfo(const std::string& deviceId,
                                            int32_t persistentId, SessionInfoBean& sessionInfo)
{
    TLOGI(WmsLogTag::DEFAULT, "id %{public}d", persistentId);
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::DEFAULT, "The caller is not system-app, can not use system-api");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!SessionPermission::VerifySessionPermission()) {
        TLOGE(WmsLogTag::DEFAULT, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    return taskScheduler_->PostSyncTask([this, &deviceId, persistentId, &sessionInfo]() {
        if (CheckIsRemote(deviceId)) {
            if (GetRemoteSessionInfo(deviceId, persistentId, sessionInfo) != ERR_OK) {
                return WSError::WS_ERROR_INVALID_PARAM;
            } else {
                return WSError::WS_OK;
            }
        }

        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        if (auto iter = sceneSessionMap_.find(persistentId); iter != sceneSessionMap_.end()) {
            auto sceneSession = iter->second;
            if (sceneSession == nullptr) {
                TLOGNE(WmsLogTag::WMS_LIFE, "session: %{public}d is nullptr", persistentId);
                return WSError::WS_ERROR_INVALID_PARAM;
            }
            const auto& sceneSessionInfo = sceneSession->GetSessionInfo();
            if (sceneSessionInfo.isSystem_) {
                TLOGND(WmsLogTag::WMS_LIFE, "sessionId: %{public}d  isSystemScene", persistentId);
                return WSError::WS_ERROR_INVALID_PARAM;
            }
            auto want = sceneSessionInfo.want;
            if (want == nullptr || sceneSessionInfo.bundleName_.empty() ||
                want->GetElement().GetBundleName().empty()) {
                TLOGNE(WmsLogTag::WMS_LIFE, "session: %{public}d, want is null or bundleName is empty "
                    "or want bundleName is empty", persistentId);
                return WSError::WS_ERROR_INTERNAL_ERROR;
            }
            TLOGND(WmsLogTag::WMS_LIFE, "GetSessionInfo sessionId:%{public}d bundleName:%{public}s", persistentId,
                sceneSessionInfo.bundleName_.c_str());
            return SceneSessionConverter::ConvertToMissionInfo(sceneSession, sessionInfo);
        } else {
            TLOGNW(WmsLogTag::WMS_LIFE, "sessionId: %{public}d not found", persistentId);
            return WSError::WS_ERROR_INVALID_PARAM;
        }
    }, __func__);
}

WSError SceneSessionManager::GetSessionInfoByContinueSessionId(const std::string& continueSessionId,
    SessionInfoBean& sessionInfo)
{
    TLOGI(WmsLogTag::WMS_LIFE, "query session info with continueSessionId: %{public}s",
        continueSessionId.c_str());
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The interface only support for system service.");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!SessionPermission::VerifySessionPermission()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted.");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    return taskScheduler_->PostSyncTask([this, continueSessionId, &sessionInfo]() {
        WSError ret = WSError::WS_ERROR_INVALID_SESSION;
        {
            std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
            for (const auto& [_, sceneSession] : sceneSessionMap_) {
                if (sceneSession && sceneSession->GetSessionInfo().continueSessionId_ == continueSessionId) {
                    ret = SceneSessionConverter::ConvertToMissionInfo(sceneSession, sessionInfo);
                    break;
                }
            }
        }

        TLOGNI(WmsLogTag::WMS_LIFE, "get session info finished with ret code: %{public}d", ret);
        // app continue report for distributed scheduled service
        SingletonContainer::Get<DmsReporter>().ReportQuerySessionInfo(ret == WSError::WS_OK,
            static_cast<int32_t>(ret));
        return ret;
    }, __func__);
}

int SceneSessionManager::GetRemoteSessionInfo(const std::string& deviceId,
                                              int32_t persistentId, SessionInfoBean& sessionInfo)
{
    TLOGI(WmsLogTag::DEFAULT, "in");
    std::vector<SessionInfoBean> sessionVector;
#ifdef WINDOW_MANAGER_FEATURE_SUPPORT_DMSFWK
    int result = GetRemoteSessionInfos(deviceId, MAX_NUMBER_OF_DISTRIBUTED_SESSIONS, sessionVector);
    if (result != ERR_OK) {
        return result;
    }
#endif
    for (auto iter = sessionVector.begin(); iter != sessionVector.end(); iter++) {
        if (iter->id == persistentId) {
            sessionInfo = *iter;
            return ERR_OK;
        }
    }
    TLOGW(WmsLogTag::DEFAULT, "missionId not found");
    return ERR_INVALID_VALUE;
}

bool SceneSessionManager::CheckIsRemote(const std::string& deviceId)
{
    if (deviceId.empty()) {
        TLOGI(WmsLogTag::DEFAULT, "empty");
        return false;
    }
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId)) {
        TLOGE(WmsLogTag::DEFAULT, "GetLocalDeviceId failed");
        return false;
    }
    if (localDeviceId == deviceId) {
        TLOGI(WmsLogTag::DEFAULT, "deviceId is local.");
        return false;
    }
    TLOGD(WmsLogTag::DEFAULT, "Checked deviceId=%{public}s", AnonymizeDeviceId(deviceId).c_str());
    return true;
}

bool SceneSessionManager::GetLocalDeviceId(std::string& localDeviceId)
{
#ifdef WINDOW_MANAGER_FEATURE_SUPPORT_DSOFTBUS
    auto localNode = std::make_unique<NodeBasicInfo>();
    int32_t errCode = GetLocalNodeDeviceInfo(DM_PKG_NAME.c_str(), localNode.get());
    if (errCode != ERR_OK) {
        TLOGE(WmsLogTag::DEFAULT, "GetLocalNodeDeviceInfo errCode=%{public}d", errCode);
        return false;
    }
    if (localNode != nullptr) {
        localDeviceId = localNode->networkId;
        TLOGD(WmsLogTag::DEFAULT, "get local deviceId, deviceId=%{public}s", AnonymizeDeviceId(localDeviceId).c_str());
        return true;
    }
#endif
    TLOGE(WmsLogTag::DEFAULT, "localDeviceId null");
    return false;
}

std::string SceneSessionManager::AnonymizeDeviceId(const std::string& deviceId)
{
    if (deviceId.length() < NON_ANONYMIZE_LENGTH) {
        return EMPTY_DEVICE_ID;
    }
    std::string anonDeviceId = deviceId.substr(0, NON_ANONYMIZE_LENGTH);
    anonDeviceId.append("******");
    return anonDeviceId;
}

WSError SceneSessionManager::DumpSessionAll(std::vector<std::string>& infos)
{
    TLOGI(WmsLogTag::DEFAULT, "in.");
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }

    return taskScheduler_->PostSyncTask([this, &infos]() {
        infos.push_back("User ID #" + std::to_string(currentUserId_));
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, session] : sceneSessionMap_) {
            if (session) {
                session->DumpSessionInfo(infos);
            }
        }
        return WSError::WS_OK;
    }, __func__);
}

WSError SceneSessionManager::DumpSessionWithId(int32_t persistentId, std::vector<std::string>& infos)
{
    TLOGI(WmsLogTag::DEFAULT, "id %{public}d", persistentId);
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }

    return taskScheduler_->PostSyncTask([this, persistentId, &infos]() {
        infos.push_back("User ID #" + std::to_string(currentUserId_));
        auto session = GetSceneSession(persistentId);
        if (session) {
            session->DumpSessionInfo(infos);
        } else {
            infos.push_back("error: invalid mission number, please see 'aa dump --mission-list'.");
        }
        return WSError::WS_OK;
    }, __func__);
}

__attribute__((no_sanitize("cfi"))) WSError SceneSessionManager::GetAllAbilityInfos(
    const AAFwk::Want& want, int32_t userId, std::vector<SCBAbilityInfo>& scbAbilityInfos)
{
    if (bundleMgr_ == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "bundleMgr_ is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    const auto& elementName = want.GetElement();
    int32_t ret{0};
    auto flag = (AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA |
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) |
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) |
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE));
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    if (elementName.GetBundleName().empty() && elementName.GetAbilityName().empty()) {
        TLOGD(WmsLogTag::DEFAULT, "want is empty queryAllAbilityInfos");
        ret = static_cast<int32_t>(bundleMgr_->GetBundleInfosV9(flag, bundleInfos, userId));
        if (ret) {
            TLOGE(WmsLogTag::DEFAULT, "Query all ability infos from BMS failed!");
            return WSError::WS_ERROR_INVALID_PARAM;
        }
    } else if (!elementName.GetBundleName().empty()) {
        AppExecFwk::BundleInfo bundleInfo;
        TLOGD(WmsLogTag::DEFAULT, "bundleName is not empty, query abilityInfo of %{public}s", elementName.GetBundleName().c_str());
        ret = static_cast<int32_t>(bundleMgr_->GetBundleInfoV9(elementName.GetBundleName(), flag, bundleInfo, userId));
        if (ret) {
            TLOGE(WmsLogTag::DEFAULT, "Query ability info from BMS failed!");
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        bundleInfos.push_back(bundleInfo);
    } else {
        TLOGE(WmsLogTag::DEFAULT, "invalid want:%{public}s", want.ToString().c_str());
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    return GetAbilityInfosFromBundleInfo(bundleInfos, scbAbilityInfos, userId);
}

__attribute__((no_sanitize("cfi"))) WSError SceneSessionManager::GetBatchAbilityInfos(
    const std::vector<std::string>& bundleNames, int32_t userId, std::vector<SCBAbilityInfo>& scbAbilityInfos)
{
    if (bundleMgr_ == nullptr) {
        TLOGE(WmsLogTag::WMS_RECOVER, "bundleMgr is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    if (bundleNames.empty()) {
        TLOGE(WmsLogTag::WMS_RECOVER, "bundleNames is empty");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    auto flag = AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
                AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
                AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA |
                static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) |
                static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) |
                static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    auto ret = static_cast<int32_t>(bundleMgr_->BatchGetBundleInfo(bundleNames, flag, bundleInfos, userId));
    if (ret) {
        TLOGE(WmsLogTag::WMS_RECOVER, "Query batch ability infos from BMS failed!");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    return GetAbilityInfosFromBundleInfo(bundleInfos, scbAbilityInfos, userId);
}

WSError SceneSessionManager::GetAbilityInfo(const std::string& bundleName, const std::string& moduleName,
    const std::string& abilityName, int32_t userId, SCBAbilityInfo& scbAbilityInfo)
{
    if (bundleMgr_ == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "bundleMgr_ is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    auto flags = (AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA |
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) |
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) |
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE));
    AppExecFwk::BundleInfo bundleInfo;
    if (bundleMgr_->GetBundleInfoV9(bundleName, flags, bundleInfo, userId)) {
        TLOGE(WmsLogTag::DEFAULT, "Query ability info from BMS failed, ability:%{public}s", abilityName.c_str());
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    auto& hapModulesList = bundleInfo.hapModuleInfos;
    if (hapModulesList.empty()) {
        TLOGD(WmsLogTag::DEFAULT, "hapModulesList is empty, ability:%{public}s", abilityName.c_str());
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    auto sdkVersion = bundleInfo.targetVersion % 100; // % 100 to get the real version
    for (auto& hapModule : hapModulesList) {
        bool isModuleAbilityHook = false;
        if (!hapModule.abilitySrcEntryDelegator.empty() && !hapModule.abilityStageSrcEntryDelegator.empty()) {
            isModuleAbilityHook = true;
        }
        auto& abilityInfoList = hapModule.abilityInfos;
        for (auto& abilityInfo : abilityInfoList) {
            abilityInfo.windowModes = ExtractSupportWindowModeFromMetaData(
                std::make_shared<OHOS::AppExecFwk::AbilityInfo>(abilityInfo));
            if (abilityInfo.moduleName == moduleName && abilityInfo.name == abilityName) {
                scbAbilityInfo.abilityInfo_ = abilityInfo;
                scbAbilityInfo.sdkVersion_ = sdkVersion;
                scbAbilityInfo.codePath_ = bundleInfo.applicationInfo.codePath;
                scbAbilityInfo.isAbilityHook_ = isModuleAbilityHook;
                GetOrientationFromResourceManager(scbAbilityInfo.abilityInfo_);
                scbAbilityInfo.isForceRotate_ = bundleInfo.applicationInfo.isForceRotate;
                return WSError::WS_OK;
            }
        }
    }
    TLOGW(WmsLogTag::DEFAULT, "Ability info not found, ability:%{public}s", abilityName.c_str());
    return WSError::WS_ERROR_INVALID_PARAM;
}

WSError SceneSessionManager::GetAbilityInfosFromBundleInfo(const std::vector<AppExecFwk::BundleInfo>& bundleInfos,
    std::vector<SCBAbilityInfo>& scbAbilityInfos, int32_t userId)
{
    if (bundleInfos.empty()) {
        TLOGE(WmsLogTag::DEFAULT, "bundleInfos is empty");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    std::vector<AppExecFwk::BundleInfo> collaboratorBundleInfos;
    for (auto& bundleInfo : bundleInfos) {
        auto& hapModulesList = bundleInfo.hapModuleInfos;
        auto sdkVersion = bundleInfo.targetVersion % 100; // %100 to get the real version
        if (hapModulesList.empty()) {
            TLOGD(WmsLogTag::DEFAULT, "hapModulesList is empty");
            continue;
        }
        if (WindowHelper::IsNumber(bundleInfo.applicationInfo.codePath) &&
            CheckCollaboratorType(std::stoi(bundleInfo.applicationInfo.codePath))) {
            collaboratorBundleInfos.emplace_back(bundleInfo);
            continue;
        }
        for (auto& hapModule : hapModulesList) {
            auto& abilityInfoList = hapModule.abilityInfos;
            bool isModuleAbilityHook = false;
            if (!hapModule.abilitySrcEntryDelegator.empty() && !hapModule.abilityStageSrcEntryDelegator.empty()) {
                isModuleAbilityHook = true;
            }
            for (auto& abilityInfo : abilityInfoList) {
                SCBAbilityInfo scbAbilityInfo;
                scbAbilityInfo.abilityInfo_ = abilityInfo;
                scbAbilityInfo.abilityInfo_.windowModes = ExtractSupportWindowModeFromMetaData(
                    std::make_shared<OHOS::AppExecFwk::AbilityInfo>(abilityInfo));
                scbAbilityInfo.abilityInfo_.applicationInfo = bundleInfo.applicationInfo;
                scbAbilityInfo.sdkVersion_ = sdkVersion;
                scbAbilityInfo.isAbilityHook_ = isModuleAbilityHook;
                GetOrientationFromResourceManager(scbAbilityInfo.abilityInfo_);
                scbAbilityInfos.push_back(scbAbilityInfo);
            }
        }
    }
    GetCollaboratorAbilityInfos(collaboratorBundleInfos, scbAbilityInfos, userId);
    return WSError::WS_OK;
}

void SceneSessionManager::GetCollaboratorAbilityInfos(const std::vector<AppExecFwk::BundleInfo>& bundleInfos,
    std::vector<SCBAbilityInfo>& scbAbilityInfos, int32_t userId)
{
    if (bundleInfos.empty()) {
        TLOGD(WmsLogTag::DEFAULT, "bundleInfos is empty");
        return;
    }
    std::vector<AppExecFwk::AbilityInfo> launcherAbilityInfos;
    AAFwk::Want want;
    want.SetAction(AAFwk::Want::ACTION_HOME);
    want.AddEntity(AAFwk::Want::ENTITY_HOME);
    if (!bundleMgr_ || bundleMgr_->QueryLauncherAbilityInfos(want, userId, launcherAbilityInfos) != ERR_OK) {
        TLOGE(WmsLogTag::DEFAULT, "Query launcher ability infos from BMS failed!");
        return;
    }
    std::unordered_map<std::string, AppExecFwk::AbilityInfo> abilityInfoMap;
    for (auto& abilityInfo : launcherAbilityInfos) {
        abilityInfoMap.emplace(abilityInfo.bundleName, abilityInfo);
    }
    for (auto& bundleInfo : bundleInfos) {
        AppExecFwk::AbilityInfo abilityInfo;
        auto& hapModulesList = bundleInfo.hapModuleInfos;
        auto iter = abilityInfoMap.find(bundleInfo.name);
        if (iter == abilityInfoMap.end()) {
            TLOGW(WmsLogTag::DEFAULT, "launcher ability not found, bundle:%{public}s", bundleInfo.name.c_str());
            auto hapModuleListIter = std::find_if(hapModulesList.begin(), hapModulesList.end(),
                [](const AppExecFwk::HapModuleInfo& hapModule) { return !hapModule.abilityInfos.empty(); });
            if (hapModuleListIter != hapModulesList.end()) {
                abilityInfo = hapModuleListIter->abilityInfos[0];
            } else {
                continue;
            }
        } else {
            if (!AbilityInfoManager::FindAbilityInfo(
                bundleInfo, iter->second.moduleName, iter->second.name, abilityInfo)) {
                continue;
            }
        }
        SCBAbilityInfo scbAbilityInfo;
        scbAbilityInfo.abilityInfo_ = abilityInfo;
        scbAbilityInfo.sdkVersion_ = bundleInfo.targetVersion % 100; // %100 to get the real version
        scbAbilityInfo.codePath_ = bundleInfo.applicationInfo.codePath;
        GetOrientationFromResourceManager(scbAbilityInfo.abilityInfo_);
        scbAbilityInfo.isForceRotate_ = bundleInfo.applicationInfo.isForceRotate;
        scbAbilityInfos.push_back(scbAbilityInfo);
    }
}

void SceneSessionManager::GetOrientationFromResourceManager(AppExecFwk::AbilityInfo& abilityInfo)
{
    if (abilityInfo.orientationId == 0) {
        return;
    }

    auto context = rootSceneContextWeak_.lock();
    if (!context) {
        TLOGE(WmsLogTag::DEFAULT, "context is nullptr.");
        return;
    }

    auto resourceMgr = context->GetResourceManager();
    if (!resourceMgr) {
        TLOGE(WmsLogTag::DEFAULT, "resourceMgr is nullptr");
        return;
    }

    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "resConfig is nullptr.");
        return;
    }

    resourceMgr->GetResConfig(*resConfig);
    TLOGE(WmsLogTag::DEFAULT, "deviceType:%{public}d", resConfig->GetDeviceType());
    resourceMgr = Global::Resource::CreateResourceManager(
        abilityInfo.bundleName, abilityInfo.moduleName, "", {}, *resConfig);
    if (resourceMgr == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "resourceMgr is nullptr.");
        return;
    }
    std::string loadPath = abilityInfo.hapPath.empty() ? abilityInfo.resourcePath : abilityInfo.hapPath;
    if (!resourceMgr->AddResource(loadPath.c_str(), Global::Resource::SELECT_STRING)) {
        TLOGE(WmsLogTag::DEFAULT, "Add resource %{private}s failed.", loadPath.c_str());
    }
    resourceMgr->UpdateResConfig(*resConfig);
    std::string orientation;
    auto ret = resourceMgr->GetStringById(abilityInfo.orientationId, orientation);
    if (ret != Global::Resource::RState::SUCCESS) {
        TLOGE(WmsLogTag::DEFAULT, "GetStringById failed errcode:%{public}d, labelId:%{public}d",
            static_cast<int32_t>(ret), abilityInfo.orientationId);
        return;
    }
    if (STRING_TO_DISPLAY_ORIENTATION_MAP.find(orientation) == STRING_TO_DISPLAY_ORIENTATION_MAP.end()) {
        TLOGE(WmsLogTag::DEFAULT, "Do not support this orientation:%{public}s", orientation.c_str());
        return;
    }
    abilityInfo.orientation = STRING_TO_DISPLAY_ORIENTATION_MAP.at(orientation);
}

WSError SceneSessionManager::TerminateSessionNew(
    const sptr<AAFwk::SessionInfo> info, bool needStartCaller, bool isFromBroker)
{
    if (info == nullptr) {
        TLOGI(WmsLogTag::WMS_LIFE, "sessionInfo is nullptr.");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    TLOGI(WmsLogTag::WMS_LIFE,
        "id:%{public}d bundleName:%{public}s needStartCaller:%{public}d isFromBroker:%{public}d",
        info->persistentId, info->want.GetElement().GetBundleName().c_str(), needStartCaller, isFromBroker);
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    uint32_t callerToken = IPCSkeleton::GetCallingTokenID();
    auto task = [this, info, needStartCaller, isFromBroker, callingPid, callerToken]() {
        sptr<SceneSession> sceneSession = FindSessionByToken(info->sessionToken);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "TerminateSessionNew:fail to find session by token.");
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        const bool pidCheck = (callingPid != -1) && (callingPid == sceneSession->GetCallingPid());
        if (!pidCheck &&
            !SessionPermission::VerifyPermissionByCallerToken(callerToken,
                PermissionConstants::PERMISSION_MANAGE_MISSION)) {
            TLOGNE(WmsLogTag::WMS_LIFE,
                "The caller has not permission granted, callingPid_:%{public}d, callingPid:%{public}d",
                sceneSession->GetCallingPid(), callingPid);
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        WSError errCode = sceneSession->TerminateSessionNew(info, needStartCaller, isFromBroker);
        return errCode;
    };
    return taskScheduler_->PostSyncTask(task, "TerminateSessionNew");
}

WSError SceneSessionManager::SetVmaCacheStatus(bool flag)
{
    TLOGI(WmsLogTag::DEFAULT, "flag: %{public}d", flag);
    RSInterfaces::GetInstance().SetVmaCacheStatus(flag);
    return WSError::WS_OK;
}

WSError SceneSessionManager::GetSessionSnapshot(const std::string& deviceId, int32_t persistentId,
                                                SessionSnapshot& snapshot, bool isLowResolution)
{
    TLOGI(WmsLogTag::DEFAULT, "id: %{public}d isLowResolution: %{public}d", persistentId, isLowResolution);
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::DEFAULT, "The caller is not system-app, can not use system-api");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!SessionPermission::VerifySessionPermission()) {
        TLOGE(WmsLogTag::DEFAULT, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    return taskScheduler_->PostSyncTask([this, &deviceId, persistentId, &snapshot, isLowResolution]() {
        if (CheckIsRemote(deviceId)) {
            if (GetRemoteSessionSnapshotInfo(deviceId, persistentId, snapshot) != ERR_OK) {
                return WSError::WS_ERROR_INVALID_PARAM;
            } else {
                return WSError::WS_OK;
            }
        }
        auto sceneSession = GetSceneSession(persistentId);
        if (!sceneSession) {
            TLOGNE(WmsLogTag::WMS_SYSTEM, "fail to find session by persistentId: %{public}d", persistentId);
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        const auto& sessionInfo = sceneSession->GetSessionInfo();
        if (sessionInfo.abilityName_.empty() || sessionInfo.moduleName_.empty() || sessionInfo.bundleName_.empty()) {
            TLOGNW(WmsLogTag::WMS_SYSTEM, "sessionInfo: %{public}d, abilityName or moduleName or bundleName is empty",
                   sceneSession->GetPersistentId());
        }
        snapshot.topAbility.SetElementBundleName(&(snapshot.topAbility), sessionInfo.bundleName_.c_str());
        snapshot.topAbility.SetElementModuleName(&(snapshot.topAbility), sessionInfo.moduleName_.c_str());
        snapshot.topAbility.SetElementAbilityName(&(snapshot.topAbility), sessionInfo.abilityName_.c_str());
        if (auto oriSnapshot = sceneSession->Snapshot()) {
            if (isLowResolution) {
                OHOS::Media::InitializationOptions options;
                options.size.width = oriSnapshot->GetWidth() / 2; // low resolution ratio
                options.size.height = oriSnapshot->GetHeight() / 2; // low resolution ratio
                std::unique_ptr<OHOS::Media::PixelMap> reducedPixelMap = OHOS::Media::PixelMap::Create(*oriSnapshot, options);
                snapshot.snapshot = std::shared_ptr<OHOS::Media::PixelMap>(reducedPixelMap.release());
            } else {
                snapshot.snapshot = oriSnapshot;
            }
        }
        return WSError::WS_OK;
    }, __func__);
}

WMError SceneSessionManager::GetSessionSnapshotById(int32_t persistentId, SessionSnapshot& snapshot)
{
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI() && !SessionPermission::IsShellCall()) {
        TLOGW(WmsLogTag::WMS_SYSTEM, "Get snapshot failed, Get snapshot by id must be system app!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    return taskScheduler_->PostSyncTask([this, persistentId, &snapshot]() {
        auto sceneSession = GetSceneSession(persistentId);
        if (!sceneSession) {
            TLOGNW(WmsLogTag::WMS_SYSTEM, "fail to find session by persistentId: %{public}d", persistentId);
            return WMError::WM_ERROR_INVALID_PARAM;
        }
        const auto& sessionInfo = sceneSession->GetSessionInfo();
        if (sessionInfo.abilityName_.empty() || sessionInfo.moduleName_.empty() || sessionInfo.bundleName_.empty()) {
            TLOGNW(WmsLogTag::WMS_SYSTEM, "sessionInfo: %{public}d, abilityName or moduleName or bundleName is empty",
                sceneSession->GetPersistentId());
        }
        snapshot.topAbility.SetBundleName(sessionInfo.bundleName_.c_str());
        snapshot.topAbility.SetModuleName(sessionInfo.moduleName_.c_str());
        snapshot.topAbility.SetAbilityName(sessionInfo.abilityName_.c_str());
        float snapShotScale = sceneSession->GetFloatingScale() > 1.0f ? 1.0f : sceneSession->GetFloatingScale();
        if (auto oriSnapshot = sceneSession->Snapshot(false, snapShotScale, false)) {
            if (sceneSession->GetFloatingScale() > 1.0f) {
                oriSnapshot->scale(sceneSession->GetFloatingScale(), sceneSession->GetFloatingScale());
            }
            snapshot.snapshot = oriSnapshot;
            TLOGNI(WmsLogTag::WMS_SYSTEM, "snapshot WxH=%{public}dx%{public}d",
                oriSnapshot->GetWidth(), oriSnapshot->GetHeight());
            return WMError::WM_OK;
        }
        return WMError::WM_ERROR_NULLPTR;
    }, __func__);
}

WSError SceneSessionManager::GetUIContentRemoteObj(int32_t persistentId, sptr<IRemoteObject>& uiContentRemoteObj)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Permission denied!");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "PersistentId=%{public}d", persistentId);
    sptr<SceneSession> sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "sceneSession is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    return sceneSession->GetUIContentRemoteObj(uiContentRemoteObj);
}

WMError SceneSessionManager::GetRootUIContentRemoteObj(DisplayId displayId, sptr<IRemoteObject>& uiContentRemoteObj)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied, displayId: %{public}" PRIu64, displayId);
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    return GetRootUIContentRemoteObjInner(displayId, uiContentRemoteObj);
}

WMError SceneSessionManager::GetRootUIContentRemoteObjInner(DisplayId displayId,
    sptr<IRemoteObject>& uiContentRemoteObj)
{
    if (rootSceneSession_ == nullptr) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "root scene is null, displayId: %{public}" PRIu64, displayId);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (PcFoldScreenManager::GetInstance().IsHalfFolded(displayId)) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "change fold screen displayId: %{public}" PRIu64, displayId);
        displayId = DEFAULT_DISPLAY_ID;
    }
    auto uiContent = rootSceneSession_->GetUIContent(displayId);
    if (uiContent == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "uiContent is nullptr, displayId: %{public}" PRIu64, displayId);
        return WMError::WM_ERROR_NULLPTR;
    }
    uiContentRemoteObj = uiContent->GetRemoteObj();
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "isNullUiContentObj=%{public}d, displayId: %{public}" PRIu64,
        uiContentRemoteObj == nullptr, displayId);
    return WMError::WM_OK;
}

#ifdef WINDOW_MANAGER_FEATURE_SUPPORT_DMSFWK
int SceneSessionManager::GetRemoteSessionSnapshotInfo(const std::string& deviceId, int32_t sessionId,
                                                      AAFwk::MissionSnapshot& sessionSnapshot)
{
    TLOGI(WmsLogTag::DEFAULT, "begin");
    int result = DistributedClient::GetInstance().GetRemoteMissionSnapshotInfo(deviceId,
        sessionId, sessionSnapshot);
    if (result != ERR_OK) {
        TLOGE(WmsLogTag::DEFAULT, "failed, result=%{public}d", result);
        return result;
    }
    return ERR_OK;
}
#endif

sptr<AAFwk::IAbilityManagerCollaborator> SceneSessionManager::GetCollaboratorByType(int32_t collaboratorType)
{
    sptr<AAFwk::IAbilityManagerCollaborator> collaborator = nullptr;
    std::shared_lock<std::shared_mutex> lock(collaboratorMapLock_);
    auto iter = collaboratorMap_.find(collaboratorType);
    if (iter == collaboratorMap_.end()) {
        TLOGE(WmsLogTag::WMS_MAIN, "Fail to found collaborator with type: %{public}d", collaboratorType);
        return collaborator;
    }
    collaborator = iter->second;
    if (collaborator == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "Find collaborator type %{public}d, but value is nullptr!", collaboratorType);
    }
    return collaborator;
}

sptr<AgentDeathRecipient> SceneSessionManager::GetCollaboratorDeathRecipient(int32_t collaboratorType)
{
    sptr<AgentDeathRecipient> collaboratorDeathRecipient = nullptr;
    std::shared_lock<std::shared_mutex> lock(collaboratorDeathRecipientMapLock_);
    auto iter = collaboratorDeathRecipientMap_.find(collaboratorType);
    if (iter == collaboratorDeathRecipientMap_.end()) {
        TLOGE(WmsLogTag::WMS_MAIN, "Fail to found collaborator deathRecipient, type: %{public}d", collaboratorType);
        return collaboratorDeathRecipient;
    }
    collaboratorDeathRecipient = iter->second;
    if (collaboratorDeathRecipient == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "collaborator deathRecipient is nullptr, type: %{public}d", collaboratorType);
    }
    return collaboratorDeathRecipient;
}

WSError SceneSessionManager::RequestSceneSessionByCall(const sptr<SceneSession>& sceneSession, int32_t requestId)
{
    const char* const where = __func__;
    auto task = [this, weakSceneSession = wptr<SceneSession>(sceneSession),
        requestId, where]() THREAD_SAFETY_GUARD(SCENE_GUARD) {
        auto sceneSession = weakSceneSession.promote();
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s: session is nullptr", where);
            return WSError::WS_ERROR_NULLPTR;
        }
        auto persistentId = sceneSession->GetPersistentId();
        if (!GetSceneSession(persistentId)) {
            TLOGNE(WmsLogTag::WMS_MAIN, "%{public}s: session is invalid, id:%{public}d", where, persistentId);
            RemoveRequestTaskInfo(persistentId, requestId);
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        const auto& sessionInfo = sceneSession->GetSessionInfo();
        TLOGNI(WmsLogTag::WMS_MAIN, "%{public}s: state:%{public}d, id:%{public}d, pageConfig:%{public}zu",
            where, sessionInfo.callState_, persistentId, sceneSession->GetSessionInfo().pageConfig.size());
        auto abilitySessionInfo = SetAbilitySessionInfo(sceneSession, requestId, true);
        bool isColdStart = false;
        OHOS::AbilityRuntime::StartParamsBySCB startParams;
        startParams.pageConfig = sceneSession->GetSessionInfo().pageConfig;
        startParams.sceneFlag = static_cast<uint32_t>(WindowStateChangeReason::ABILITY_CALL);
        AAFwk::AbilityManagerClient::GetInstance()->CallUIAbilityBySCB(abilitySessionInfo, startParams, isColdStart);
        CloseAllFd(sessionInfo.want);
        if (isColdStart) {
            TLOGNI(WmsLogTag::WMS_MAIN, "Cold start, identityToken:%{public}s, bundleName:%{public}s",
                abilitySessionInfo->identityToken.c_str(), sessionInfo.bundleName_.c_str());
            sceneSession->SetClientIdentityToken(abilitySessionInfo->identityToken);
            sceneSession->ResetSessionConnectState();
            sceneSession->UpdatePrivacyModeControlInfo();
        }
        RemoveRequestTaskInfo(persistentId, requestId);
        return WSError::WS_OK;
    };
    std::string taskName = "RequestSceneSessionByCall:PID:" +
        (sceneSession != nullptr ? std::to_string(sceneSession->GetPersistentId()):"nullptr");
    taskScheduler_->PostAsyncTask(task, taskName);
    return WSError::WS_OK;
}

void SceneSessionManager::StartAbilityBySpecified(const SessionInfo& sessionInfo)
{
    const char* const where = __func__;
    ffrtQueueHelper_->SubmitTask([this, sessionInfo, where] {
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s: bundleName: %{public}s, "
            "moduleName: %{public}s, abilityName: %{public}s", where,
            sessionInfo.bundleName_.c_str(), sessionInfo.moduleName_.c_str(), sessionInfo.abilityName_.c_str());
        AAFwk::Want want;
        want.SetElementName("", sessionInfo.bundleName_, sessionInfo.abilityName_, sessionInfo.moduleName_);
        if (sessionInfo.want != nullptr) {
            want.SetParams(sessionInfo.want->GetParams());
            want.SetParam(AAFwk::Want::PARAM_RESV_DISPLAY_ID, static_cast<int>(sessionInfo.screenId_));
        }
        AAFwk::StartSpecifiedAbilityParams spcifiedParams(sessionInfo.persistentId_,
            static_cast<AAFwk::SpecifiedReason>(static_cast<int32_t>(sessionInfo.specifiedReason_)));
        auto result = AAFwk::AbilityManagerClient::GetInstance()->StartSpecifiedAbilityBySCB(want, spcifiedParams);
        CloseAllFd(sessionInfo.want);
        TLOGNI(WmsLogTag::WMS_LIFE, "start specified ability by SCB result: %{public}d", result);
        if (result == ERR_OK) {
            return;
        }
        auto task = [bundleName = sessionInfo.bundleName_, appInstanceKey = sessionInfo.appInstanceKey_] {
            MultiInstanceManager::GetInstance().DecreaseInstanceKeyRefCountByBundleNameAndInstanceKey(
                bundleName, appInstanceKey);
        };
        taskScheduler_->PostAsyncTask(task, where);
    });
}

void SceneSessionManager::NotifyWindowStateErrorFromMMI(int32_t pid, int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_LIFE, "pid: %{public}d, persistentId: %{public}d", pid, persistentId);
    if (pid == -1) {
        TLOGE(WmsLogTag::WMS_LIFE, "invalid pid");
        return;
    }
    int32_t ret = HiSysEventWrite(
        HiviewDFX::HiSysEvent::Domain::WINDOW_MANAGER,
        "WINDOW_STATE_ERROR",
        HiviewDFX::HiSysEvent::EventType::FAULT,
        "PID", pid,
        "PERSISTENT_ID", persistentId);
    if (ret != 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "write HiSysEvent error, ret: %{public}d", ret);
    }
    auto task = [this, pid] {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (!sceneSession || pid != sceneSession->GetCallingPid() ||
                !WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
                continue;
            }
            auto abilitySessionInfo = SetAbilitySessionInfo(sceneSession);
            TLOGNI(WmsLogTag::WMS_LIFE, "notify session exception from MMI, persistentId: %{public}d",
                abilitySessionInfo->persistentId);
            sceneSession->NotifySessionExceptionWithOptions(abilitySessionInfo, "NotifyWindowStateErrorFromMMI", true);
        }
    };
    // delay 2000ms, wait for hidumper
    taskScheduler_->PostAsyncTask(task, __func__, 2000);
}

sptr<SceneSession> SceneSessionManager::FindMainWindowWithToken(sptr<IRemoteObject> targetToken)
{
    if (!targetToken) {
        TLOGE(WmsLogTag::DEFAULT, "Token is null, cannot find main window");
        return nullptr;
    }

    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    auto iter = std::find_if(sceneSessionMap_.begin(), sceneSessionMap_.end(),
        [targetToken](const std::map<uint64_t, sptr<SceneSession>>::value_type& pair) {
            if (pair.second->IsTerminated()) {
                return false;
            }
            if (pair.second->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
                return pair.second->GetAbilityToken() == targetToken;
            }
            return false;
        });
    if (iter == sceneSessionMap_.end()) {
        TLOGE(WmsLogTag::DEFAULT, "Cannot find session");
        return nullptr;
    }
    return iter->second;
}

void SceneSessionManager::ConfigSupportCreateFloatWindow()
{
    auto task = [this] {
        systemConfig_.supportCreateFloatWindow_ = true;
    };
    taskScheduler_->PostAsyncTask(task, "ConfigSupportCreateFloatWindow");
}

WSError SceneSessionManager::BindDialogSessionTarget(uint64_t persistentId, sptr<IRemoteObject> targetToken)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_DIALOG, "permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (targetToken == nullptr) {
        TLOGE(WmsLogTag::WMS_DIALOG, "Target token is null");
        return WSError::WS_ERROR_NULLPTR;
    }

    auto task = [this, persistentId, targetToken]() {
        auto sceneSession = GetSceneSession(static_cast<int32_t>(persistentId));
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_DIALOG, "Session is nullptr, persistentId:%{public}" PRIu64, persistentId);
            return WSError::WS_ERROR_NULLPTR;
        }
        if (sceneSession->GetWindowType() != WindowType::WINDOW_TYPE_DIALOG) {
            TLOGNE(WmsLogTag::WMS_DIALOG, "Session is not dialog, type:%{public}u", sceneSession->GetWindowType());
            return WSError::WS_OK;
        }
        sceneSession->dialogTargetToken_ = targetToken;
        sptr<SceneSession> parentSession = FindMainWindowWithToken(targetToken);
        if (parentSession == nullptr) {
            sceneSession->NotifyDestroy();
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        sptr<WindowSessionProperty> parentProperty = parentSession->GetSessionProperty();
        sptr<WindowSessionProperty> property = sceneSession->GetSessionProperty();
        auto displayId = parentProperty->GetDisplayId();
        property->SetDisplayId(displayId);
        sceneSession->SetScreenId(displayId);
        sceneSession->SetParentSession(parentSession);
        sceneSession->SetParentPersistentId(parentSession->GetPersistentId());
        sceneSession->SetClientDisplayId(parentSession->GetClientDisplayId());
        UpdateParentSessionForDialog(sceneSession, sceneSession->GetSessionProperty());
        TLOGNI(WmsLogTag::WMS_DIALOG, "Bind dialog success, dialog id %{public}" PRIu64 ", parentId %{public}d",
            persistentId, parentSession->GetPersistentId());
        return WSError::WS_OK;
    };
    return taskScheduler_->PostSyncTask(task, "BindDialogTarget:PID:" + std::to_string(persistentId));
}

void DisplayChangeListener::OnGetSurfaceNodeIdsFromMissionIds(std::vector<uint64_t>& missionIds,
    std::vector<uint64_t>& surfaceNodeIds, const std::vector<uint32_t>& needWindowTypeList, bool isNeedForceCheck)
{
    SceneSessionManager::GetInstance().GetSurfaceNodeIdsFromMissionIds(missionIds, surfaceNodeIds,
        needWindowTypeList, isNeedForceCheck);
}

WMError SceneSessionManager::GetSurfaceNodeIdsFromMissionIds(std::vector<uint64_t>& missionIds,
    std::vector<uint64_t>& surfaceNodeIds, const std::vector<uint32_t>& needWindowTypeList, bool isNeedForceCheck)
{
    auto isSaCall = SessionPermission::IsSACalling();
    if (!isSaCall) {
        TLOGE(WmsLogTag::DEFAULT, "The interface only support for sa call");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    auto task = [this, &missionIds, &surfaceNodeIds, &needWindowTypeList, isNeedForceCheck]() {
        std::map<int32_t, sptr<SceneSession>>::iterator iter;
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (auto missionId : missionIds) {
            iter = sceneSessionMap_.find(static_cast<int32_t>(missionId));
            if (iter == sceneSessionMap_.end()) {
                if (isNeedForceCheck) {
                    surfaceNodeIds.clear();
                    return WMError::WM_ERROR_INVALID_WINDOW;
                }
                continue;
            }
            auto sceneSession = iter->second;
            if (sceneSession == nullptr || sceneSession->GetSurfaceNode() == nullptr) {
                if (isNeedForceCheck) {
                    surfaceNodeIds.clear();
                    return WMError::WM_ERROR_INVALID_WINDOW;
                }
                continue;
            }
            if (!needWindowTypeList.empty() && std::find(needWindowTypeList.begin(), needWindowTypeList.end(),
                static_cast<uint32_t>(sceneSession->GetWindowType())) == needWindowTypeList.end()) {
                if (isNeedForceCheck) {
                    surfaceNodeIds.clear();
                    return WMError::WM_ERROR_INVALID_WINDOW;
                }
                continue;
            }
            surfaceNodeIds.push_back(sceneSession->GetSurfaceNode()->GetId());
            GetSurfaceNodeIdsFromSubSession(sceneSession, surfaceNodeIds);
            surfaceNodeIds.push_back(missionId);
            if (sceneSession->GetLeashWinSurfaceNode()) {
                surfaceNodeIds.push_back(sceneSession->GetLeashWinSurfaceNode()->GetId());
            }
        }
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, "GetSurfaceNodeIdsFromMissionIds");
}

WMError SceneSessionManager::GetSurfaceNodeIdsFromSubSession(
    const sptr<SceneSession>& sceneSession, std::vector<uint64_t>& surfaceNodeIds)
{
    if (sceneSession == nullptr) {
        return WMError::WM_DO_NOTHING;
    }
    for (const auto& subSession : sceneSession->GetSubSession()) {
        if (subSession == nullptr) {
            continue;
        }
        GetSurfaceNodeIdsFromSubSession(subSession, surfaceNodeIds);
    }
    if (sceneSession->GetSurfaceNode() == nullptr) {
        return WMError::WM_DO_NOTHING;
    }
    surfaceNodeIds.push_back(sceneSession->GetSurfaceNode()->GetId());
    if (sceneSession->GetLeashWinSurfaceNode()) {
        surfaceNodeIds.push_back(static_cast<uint64_t>(sceneSession->GetPersistentId()));
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, surfaceId: %{public}" PRIu64,
        sceneSession->GetPersistentId(), sceneSession->GetSurfaceNode()->GetId());
    return WMError::WM_OK;
}

void DisplayChangeListener::OnSetSurfaceNodeIds(DisplayId displayId, const std::vector<uint64_t>& surfaceNodeIds)
{
    SceneSessionManager::GetInstance().SetSurfaceNodeIds(displayId, surfaceNodeIds);
}

WMError SceneSessionManager::SetSurfaceNodeIds(DisplayId displayId, const std::vector<uint64_t>& surfaceNodeIds)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "displayId: %{public}" PRIu64, displayId);
    auto task = [this, displayId, &surfaceNodeIds, where = __func__]() {
        for (auto it = sessionBlackListInfoMap_[displayId].begin(); it != sessionBlackListInfoMap_[displayId].end(); ) {
            if (it->privacyWindowTag == WMS_DEFAULT) {
                it = sessionBlackListInfoMap_[displayId].erase(it);
            } else {
                ++it;
            }
        }
        for (auto surfaceNodeId : surfaceNodeIds) {
            sptr<SceneSession> sceneSession = SelectSesssionFromMap(surfaceNodeId);
            if (sceneSession == nullptr) {
                continue;
            }
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: %{public}d, %{public}" PRIu64,
                where, sceneSession->GetPersistentId(), surfaceNodeId);
            sessionBlackListInfoMap_[displayId].insert({ .windowId = sceneSession->GetPersistentId() });
        }
        UpdateVirtualScreenBlackList(displayId);
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

void DisplayChangeListener::OnVirtualScreenDisconnected(DisplayId displayId)
{
    SceneSessionManager::GetInstance().OnVirtualScreenDisconnected(displayId);
}

WMError SceneSessionManager::OnVirtualScreenDisconnected(DisplayId displayId)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "displayId: %{public}" PRIu64, displayId);
    auto task = [this, displayId, where = __func__]() {
        bool isSessionBlackListErased = sessionBlackListInfoMap_.erase(displayId) > 0;
        bool isScreenConfigErased = screenRSBlackListConfigMap_.erase(displayId) > 0;
        if (isSessionBlackListErased || isScreenConfigErased) {
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: displayId %{public}" PRIu64, where, displayId);
            return WMError::WM_OK;
        }
        return WMError::WM_DO_NOTHING;
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

WMError SceneSessionManager::AddSessionBlackList(
    const std::unordered_set<std::string>& bundleNames, const std::unordered_set<std::string>& privacyWindowTags)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    auto task = [this, &bundleNames, &privacyWindowTags, where = __func__]() {
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s, bundleSize: %{public}zu, privacySize: %{public}zu",
            where, bundleNames.size(), privacyWindowTags.size());
        for (const auto& bundleName : bundleNames) {
            bundleRSBlackListConfigMap_.insert({ bundleName, {} });
            bundleRSBlackListConfigMap_[bundleName].insert(privacyWindowTags.begin(), privacyWindowTags.end());
        }
        std::vector<sptr<SceneSession>> sceneSessionList;
        {
            std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
            for (const auto& [_, sceneSession] : sceneSessionMap_) {
                if (sceneSession && bundleNames.find(sceneSession->GetSessionInfo().bundleName_) != bundleNames.end()) {
                    sceneSessionList.emplace_back(sceneSession);
                }
            }
        }
        return AddSessionBlackList(sceneSessionList, privacyWindowTags);
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

WMError SceneSessionManager::AddSessionBlackList(const std::vector<sptr<SceneSession>>& sceneSessionList,
    const std::unordered_set<std::string>& privacyWindowTags)
{
    for (const auto& sceneSession : sceneSessionList) {
        for (const auto& privacyWindowTag : privacyWindowTags) {
            sessionRSBlackListConfigSet_.insert(
                { .windowId = sceneSession->GetPersistentId(), .privacyWindowTag = privacyWindowTag });
        }
    }
    return FlushSessionBlackListInfoMapWhenAdd();
}

WMError SceneSessionManager::RemoveSessionBlackList(
    const std::unordered_set<std::string>& bundleNames, const std::unordered_set<std::string>& privacyWindowTags)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    auto task = [this, &bundleNames, &privacyWindowTags, where = __func__]() {
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s, bundleSize: %{public}zu, privacySize: %{public}zu",
            where, bundleNames.size(), privacyWindowTags.size());
        for (const auto& bundleName : bundleNames) {
            bundleRSBlackListConfigMap_.insert({ bundleName, {} });
            for(const auto& privacyWindowTag : privacyWindowTags) {
                bundleRSBlackListConfigMap_[bundleName].erase(privacyWindowTag);
            }
            if (bundleRSBlackListConfigMap_[bundleName].empty()) {
                bundleRSBlackListConfigMap_.erase(bundleName);
            }
        }
        std::vector<sptr<SceneSession>> sceneSessionList;
        {
            std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
            for (const auto& [_, sceneSession] : sceneSessionMap_) {
                if (sceneSession && bundleNames.find(sceneSession->GetSessionInfo().bundleName_) != bundleNames.end()) {
                    sceneSessionList.emplace_back(sceneSession);
                }
            }
        }
        return RemoveSessionBlackList(sceneSessionList, privacyWindowTags);
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

WMError SceneSessionManager::RemoveSessionBlackList(const std::vector<sptr<SceneSession>>& sceneSessionList,
    const std::unordered_set<std::string>& privacyWindowTags)
{
    for (const auto& sceneSession : sceneSessionList) {
        for (const auto& privacyWindowTag : privacyWindowTags) {
            sessionRSBlackListConfigSet_.erase(
                { .windowId = sceneSession->GetPersistentId(), .privacyWindowTag = privacyWindowTag });
        }
    }
    return FlushSessionBlackListInfoMapWhenRemove();
}

WMError SceneSessionManager::FlushSessionBlackListInfoMapWhenAdd()
{
    for (const auto& [screenId, screenBlackListInfo] : screenRSBlackListConfigMap_) {
        for (const auto& info : sessionRSBlackListConfigSet_) {
            if (screenBlackListInfo.find({ .privacyWindowTag = info.privacyWindowTag }) != screenBlackListInfo.end()) {
                sessionBlackListInfoMap_[screenId].insert(info);
            }
        }
        UpdateVirtualScreenBlackList(screenId);
    }
    return WMError::WM_OK;
}

WMError SceneSessionManager::FlushSessionBlackListInfoMapWhenAdd(ScreenId screenId)
{
    auto screenBlackListInfo = screenRSBlackListConfigMap_[screenId];
    for (const auto& info : sessionRSBlackListConfigSet_) {
        if (screenBlackListInfo.find({ .privacyWindowTag = info.privacyWindowTag }) != screenBlackListInfo.end()) {
            sessionBlackListInfoMap_[screenId].insert(info);
        }
    }
    UpdateVirtualScreenBlackList(screenId);
    return WMError::WM_OK;
}

WMError SceneSessionManager::FlushSessionBlackListInfoMapWhenRemove()
{
    for (auto& [screenId, infoSet] : sessionBlackListInfoMap_) {
        for (auto it = infoSet.begin(); it != infoSet.end(); ) {
            const auto& info = *it;
            if (info.privacyWindowTag == WMS_DEFAULT) {
                ++it;
                continue;
            }
            bool notInScreenConfigMap = screenRSBlackListConfigMap_[screenId].find(
                { .privacyWindowTag = info.privacyWindowTag }) == screenRSBlackListConfigMap_[screenId].end();
            bool notInSessionConfigSet = sessionRSBlackListConfigSet_.find(info) == sessionRSBlackListConfigSet_.end();
            if (notInScreenConfigMap || notInSessionConfigSet) {
                it = infoSet.erase(it);
            } else {
                ++it;
            }
        }
        UpdateVirtualScreenBlackList(screenId);
    }
    return WMError::WM_OK;
}

WMError SceneSessionManager::FlushSessionBlackListInfoMapWhenRemove(ScreenId screenId)
{
    if (sessionBlackListInfoMap_.find(screenId) == sessionBlackListInfoMap_.end()) {
        std::vector<uint64_t> emptyList;
        rsInterface_.SetVirtualScreenBlackList(screenId, emptyList);
        return WMError::WM_OK;
    }
    auto& infoSet = sessionBlackListInfoMap_[screenId];
    for (auto it = infoSet.begin(); it != infoSet.end(); ) {
        const auto& info = *it;
        if (info.privacyWindowTag == WMS_DEFAULT) {
            ++it;
            continue;
        }
        bool notInScreenConfigMap = screenRSBlackListConfigMap_[screenId].find(
            { .privacyWindowTag = info.privacyWindowTag }) == screenRSBlackListConfigMap_[screenId].end();
        bool notInSessionConfigSet = sessionRSBlackListConfigSet_.find(info) == sessionRSBlackListConfigSet_.end();
        if (notInScreenConfigMap || notInSessionConfigSet) {
            it = infoSet.erase(it);
        } else {
            ++it;
        }
    }
    UpdateVirtualScreenBlackList(screenId);
    return WMError::WM_OK;
}

void SceneSessionManager::UpdateVirtualScreenBlackList(ScreenId screenId)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "Configsize: [%{public}zu, %{public}zu], %{public}zu",
        sessionRSBlackListConfigSet_.size(), screenRSBlackListConfigMap_.size(), sessionBlackListInfoMap_.size());
    std::unordered_set<uint64_t> skipSurfaceNodeIdSet;
    for (const auto& info : sessionBlackListInfoMap_[screenId]) {
        AddskipSurfaceNodeIdSet(info.windowId, skipSurfaceNodeIdSet);
    }
    std::vector<uint64_t> skipSurfaceNodeIds(skipSurfaceNodeIdSet.begin(), skipSurfaceNodeIdSet.end());
    std::ostringstream oss;
    oss << "surfaceNodeIds[" << screenId << "]: ";
    for (auto id : skipSurfaceNodeIds) {
        oss << id << " ";
    }
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "%{public}s", oss.str().c_str());
    rsInterface_.SetVirtualScreenBlackList(screenId, skipSurfaceNodeIds);
}

void SceneSessionManager::NotifyOnAttachToFrameNode(const sptr<Session>& session)
{
    auto where = __func__;
    wptr<Session> weakSession(session);
    auto task = [this, weakSession, where] {
        auto session = weakSession.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s, session is nullptr", where);
            return;
        }
        TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s, wid: %{public}d", where, session->GetPersistentId());
        if (WindowHelper::IsMainWindow(session->GetWindowType())) {
            AddSkipSurfaceNodeWhenAttach(session->GetPersistentId(),
                session->GetSessionInfo().bundleName_, static_cast<uint64_t>(session->GetPersistentId()));
        } else {
            auto surfaceNode = session->GetSurfaceNode();
            if (surfaceNode == nullptr) {
                TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s, surfaceNode is nullptr", where);
                return;
            }
            AddSkipSurfaceNodeWhenAttach(session->GetPersistentId(),
                session->GetSessionInfo().bundleName_, surfaceNode->GetId());
        }
    };
    taskScheduler_->PostAsyncTask(task, where);
}

void SceneSessionManager::AddSkipSurfaceNodeWhenAttach(
    int32_t windowId, const std::string& bundleName, uint64_t surfaceNodeId)
{
    if (bundleRSBlackListConfigMap_.find(bundleName) != bundleRSBlackListConfigMap_.end()) {
        for (const auto& tag : bundleRSBlackListConfigMap_[bundleName]) {
            sessionRSBlackListConfigSet_.insert({ .windowId = windowId, .privacyWindowTag = tag });
            for (const auto& [screenId, infoSet] : screenRSBlackListConfigMap_) {
                if (infoSet.find({ .privacyWindowTag = tag }) != infoSet.end()) {
                    sessionBlackListInfoMap_[screenId].insert({ .windowId = windowId, .privacyWindowTag = tag });
                    std::vector<uint64_t> skipSurfaceNodeIds = { surfaceNodeId };
                    rsInterface_.AddVirtualScreenBlackList(screenId, skipSurfaceNodeIds);
                }
            }
        }
    }
}

void SceneSessionManager::AddskipSurfaceNodeIdSet(int32_t windowId, std::unordered_set<uint64_t>& skipSurfaceNodeIdSet)
{
    auto sceneSession = GetSceneSession(windowId);
    if (sceneSession == nullptr) {
        return;
    }
    if (SessionHelper::IsMainWindow(static_cast<WindowType>(sceneSession->GetWindowType()))) {
        auto leashWinSurfaceNode = sceneSession->GetLeashWinSurfaceNode();
        if (leashWinSurfaceNode != nullptr) {
            skipSurfaceNodeIdSet.insert(static_cast<uint64_t>(sceneSession->GetPersistentId()));
        }
    }
    auto surfaceNode = sceneSession->GetSurfaceNode();
    if (surfaceNode != nullptr) {
        skipSurfaceNodeIdSet.insert(surfaceNode->GetId());
    }
}

WMError SceneSessionManager::UpdateSubSessionBlackList(const sptr<SceneSession>& sceneSession)
{
    WMError ret = WMError::WM_DO_NOTHING;
    auto parentSession = sceneSession->GetParentSession();
    if (parentSession == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "win: %{public}d parent session is null", sceneSession->GetPersistentId());
        return ret;
    }
    SessionBlackListInfo parentSessionBlackListInfo = { .windowId = parentSession->GetPersistentId() };
    ScreenId rsScreenId = SCREEN_ID_INVALID;
    for (auto& [displayId, sessionBlackListInfoSet] : sessionBlackListInfoMap_) {
        if (sessionBlackListInfoSet.count(parentSessionBlackListInfo) > 0) {
            sessionBlackListInfoSet.insert({ .windowId = sceneSession->GetPersistentId() });
            rsScreenId = displayId;
            ret = WMError::WM_OK;
        }
    }
    if (ret != WMError::WM_OK) {
        return ret;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "rsScreenId: %{public}" PRIu64 " win: %{public}d parentId: %{public}d",
        rsScreenId, sceneSession->GetPersistentId(), parentSession->GetPersistentId());
    SessionBlackListInfoSet funcSet = sessionBlackListInfoMap_[rsScreenId];
    std::vector<uint64_t> surfaceNodeIds;
    for (const auto& sessionBlackListInfo : funcSet) {
        sptr<SceneSession> session = GetSceneSession(sessionBlackListInfo.windowId);
        if (session == nullptr) {
            TLOGW(WmsLogTag::WMS_ATTRIBUTE, "session %{public}d is null", sessionBlackListInfo.windowId);
            continue;
        }
        if (session->GetSurfaceNode() == nullptr) {
            TLOGW(WmsLogTag::WMS_ATTRIBUTE, "surfaceNode %{public}d is null", sessionBlackListInfo.windowId);
            continue;
        }
        surfaceNodeIds.push_back(session->GetSurfaceNode()->GetId());
        if (session->GetLeashWinSurfaceNode()) {
            surfaceNodeIds.push_back(static_cast<uint64_t>(session->GetPersistentId()));
        }
    }
    std::ostringstream oss;
    oss << "surfaceNodeIds[" << rsScreenId << "]: ";
    for (auto id : surfaceNodeIds) {
        oss << id << " ";
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s", oss.str().c_str());
    rsInterface_.SetVirtualScreenBlackList(rsScreenId, surfaceNodeIds);
    return ret;
}

WMError SceneSessionManager::RemoveSessionFromBlackList(const sptr<SceneSession>& sceneSession)
{
    WMError ret = WMError::WM_DO_NOTHING;
    if (sceneSession == nullptr) {
        return ret;
    }
    RemoveSessionFromBlackListInfoSet(sceneSession, sessionRSBlackListConfigSet_);
    for (auto it = sessionBlackListInfoMap_.begin(); it != sessionBlackListInfoMap_.end();) {
        auto& sessionBlackListInfoSet = it->second;
        RemoveSessionFromBlackListInfoSet(sceneSession, sessionBlackListInfoSet);
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winId: %{public}d, displayId: %{public}" PRIu64,
            sceneSession->GetPersistentId(), it->first);
        ret = WMError::WM_OK;
        if (sessionBlackListInfoSet.empty()) {
            it = sessionBlackListInfoMap_.erase(it);
        } else {
            ++it;
        }
    }
    return ret;
}

void SceneSessionManager::RemoveSessionFromBlackListInfoSet(
    const sptr<SceneSession>& sceneSession, SessionBlackListInfoSet& sessionBlackListInfoSet)
{
    for (auto it = sessionBlackListInfoSet.begin(); it != sessionBlackListInfoSet.end();) {
        if (it->windowId == sceneSession->GetPersistentId()) {
            it = sessionBlackListInfoSet.erase(it);
        } else {
            ++it;
        }
    }
}

WMError SceneSessionManager::RecoverWindowPropertyChangeFlag(uint32_t observedFlags, uint32_t interestedFlags)
{
    observedFlags_ |= observedFlags;
    interestedFlags_ |= interestedFlags;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "observedFlags: %{public}u, interestedFlags: %{public}u",
        observedFlags_, interestedFlags_);
    return WMError::WM_OK;
}

WMError SceneSessionManager::RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
    uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    observedFlags_ |= static_cast<uint32_t>(windowInfoKey);
    interestedFlags_ |= interestInfo;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "observedFlags: %{public}u, interestedFlags: %{public}u",
        observedFlags_, interestedFlags_);
    return RegisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY, windowManagerAgent);
}

WMError SceneSessionManager::UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
    uint32_t interestInfo, const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    observedFlags_ &= ~(static_cast<uint32_t>(windowInfoKey));
    interestedFlags_ &= ~interestInfo;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "observedFlags: %{public}u, interestedFlags: %{public}u",
        observedFlags_, interestedFlags_);
    return UnregisterWindowManagerAgent(
        WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY, windowManagerAgent);
}

WMError SceneSessionManager::RegisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    if (type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_SYSTEM_BAR ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GESTURE_NAVIGATION_ENABLED ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WATER_MARK_FLAG) {
        if (!SessionPermission::IsSystemCalling()) {
            TLOGE(WmsLogTag::DEFAULT, "permission denied!");
            return WMError::WM_ERROR_NOT_SYSTEM_APP;
        }
    } else if (type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE) {
        if (!SessionPermission::IsSystemServiceCalling()) {
            return WMError::WM_ERROR_INVALID_PERMISSION;
        }
    }
    if (type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_VISIBLE_WINDOW_NUM ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_DISPLAYGROUP_INFO ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_MODE ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_PID_VISIBILITY ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY) {
        if (!SessionPermission::IsSACalling()) {
            TLOGE(WmsLogTag::WMS_LIFE, "permission denied!");
            return WMError::WM_ERROR_INVALID_PERMISSION;
        }
    }
    if ((windowManagerAgent == nullptr) || (windowManagerAgent->AsObject() == nullptr)) {
        TLOGE(WmsLogTag::DEFAULT, "windowManagerAgent is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    const auto callingPid = IPCSkeleton::GetCallingRealPid();
    auto task = [this, windowManagerAgent, type, callingPid]() {
        return SessionManagerAgentController::GetInstance()
            .RegisterWindowManagerAgent(windowManagerAgent, type, callingPid);
    };
    return taskScheduler_->PostSyncTask(task, "RegisterWindowManagerAgent");
}

WMError SceneSessionManager::UnregisterWindowManagerAgent(WindowManagerAgentType type,
    const sptr<IWindowManagerAgent>& windowManagerAgent)
{
    if (type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_SYSTEM_BAR ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GESTURE_NAVIGATION_ENABLED ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WATER_MARK_FLAG ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_SUPPORT_ROTATION) {
        if (!SessionPermission::IsSystemCalling()) {
            TLOGE(WmsLogTag::DEFAULT, "IsSystemCalling permission denied!");
            return WMError::WM_ERROR_NOT_SYSTEM_APP;
        }
    }
    if (type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_VISIBLE_WINDOW_NUM ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_DISPLAYGROUP_INFO ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_MODE ||
        type == WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_PROPERTY) {
        if (!SessionPermission::IsSACalling()) {
            TLOGE(WmsLogTag::WMS_LIFE, "IsSACalling permission denied!");
            return WMError::WM_ERROR_INVALID_PERMISSION;
        }
    }
    if ((windowManagerAgent == nullptr) || (windowManagerAgent->AsObject() == nullptr)) {
        TLOGE(WmsLogTag::DEFAULT, "windowManagerAgent is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    const auto callingPid = IPCSkeleton::GetCallingRealPid();
    auto task = [this, windowManagerAgent, type, callingPid]() {
        return SessionManagerAgentController::GetInstance()
            .UnregisterWindowManagerAgent(windowManagerAgent, type, callingPid);
    };
    return taskScheduler_->PostSyncTask(task, "UnregisterWindowManagerAgent");
}

void SceneSessionManager::UpdateCameraFloatWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    SessionManagerAgentController::GetInstance().UpdateCameraFloatWindowStatus(accessTokenId, isShowing);
}

void SceneSessionManager::UpdateCameraWindowStatus(uint32_t accessTokenId, bool isShowing)
{
    SessionManagerAgentController::GetInstance().UpdateCameraWindowStatus(accessTokenId, isShowing);
}

void SceneSessionManager::StartWindowInfoReportLoop()
{
    TLOGD(WmsLogTag::WMS_STARTUP_PAGE, "in");
    if (isReportTaskStart_) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "Report is ReportTask Start");
        return;
    }
    auto task = [this] {
        WindowInfoReporter::GetInstance().ReportRecordedInfos();
        ReportWindowProfileInfos();
        isReportTaskStart_ = false;
        StartWindowInfoReportLoop();
    };
    int64_t delayTime = 1000 * 60 * 60; // an hour.
    bool ret = eventHandler_->PostTask(task, "wms:WindowInfoReport", delayTime);
    if (!ret) {
        TLOGE(WmsLogTag::WMS_STARTUP_PAGE, "failed. task is WindowInfoReport");
        return;
    }
    isReportTaskStart_ = true;
}

void SceneSessionManager::InitPersistentStorage()
{
    if (ScenePersistentStorage::HasKey("maximize_state", ScenePersistentStorageType::MAXIMIZE_STATE)) {
        int32_t storageMode = -1;
        ScenePersistentStorage::Get("maximize_state", storageMode, ScenePersistentStorageType::MAXIMIZE_STATE);
        if (storageMode == static_cast<int32_t>(MaximizeMode::MODE_AVOID_SYSTEM_BAR) ||
            storageMode == static_cast<int32_t>(MaximizeMode::MODE_FULL_FILL)) {
            TLOGI(WmsLogTag::DEFAULT, "init MaximizeMode as %{public}d from persistent storage", storageMode);
            SceneSession::maximizeMode_ = static_cast<MaximizeMode>(storageMode);
        }
    }
}

WMError SceneSessionManager::GetAccessibilityWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos)
{
    TLOGD(WmsLogTag::DEFAULT, "in.");
    if (!SessionPermission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "Only support for system service.");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, &infos]() {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetAccessibilityWindowInfo");
        std::map<int32_t, sptr<SceneSession>>::iterator iter;
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (iter = sceneSessionMap_.begin(); iter != sceneSessionMap_.end(); iter++) {
            sptr<SceneSession> sceneSession = iter->second;
            if (sceneSession == nullptr) {
                TLOGNW(WmsLogTag::WMS_ATTRIBUTE, "null scene session");
                continue;
            }
            bool isVisibleForAccessibility = Session::IsScbCoreEnabled() ?
                sceneSession->IsVisibleForAccessibility() :
                sceneSession->IsVisibleForAccessibility() && IsSessionVisibleForeground(sceneSession);
            TLOGND(WmsLogTag::WMS_ATTRIBUTE, "name=%{public}s, isSystem=%{public}d, persistentId=%{public}d, "
                "winType=%{public}d, state=%{public}d, visible=%{public}d", sceneSession->GetWindowName().c_str(),
                sceneSession->GetSessionInfo().isSystem_, iter->first, sceneSession->GetWindowType(),
                sceneSession->GetSessionState(), isVisibleForAccessibility);
            if (isVisibleForAccessibility) {
                FillWindowInfo(infos, iter->second);
            }
        }
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, "GetAccessibilityWindowInfo");
}

WMError SceneSessionManager::ConvertToRelativeCoordinateExtended(const Rect& rect, Rect& newRect, DisplayId& newDisplayId)
{
    newRect = rect;
    const auto& pcFoldScreenManagerInstance = PcFoldScreenManager::GetInstance();
    SuperFoldStatus foldStatus = pcFoldScreenManagerInstance.GetScreenFoldStatus();
    TLOGD(WmsLogTag::WMS_LAYOUT, "foldStatus=%{public}d", static_cast<uint32_t>(foldStatus));
    const auto& [defaultDisplayRect, virtualDisplayRect, foldCreaseRect] =
        pcFoldScreenManagerInstance.GetDisplayRects();
    bool isCPlane = rect.posY_ > (defaultDisplayRect.height_ + foldCreaseRect.height_);
    if (foldStatus == SuperFoldStatus::HALF_FOLDED && isCPlane) {
        newRect.posY_ = rect.posY_ - defaultDisplayRect.height_ - foldCreaseRect.height_;
        newDisplayId = VIRTUAL_DISPLAY_ID;
        return WMError::WM_OK;
    } 
    return WMError::WM_DO_NOTHING;
}

static bool CheckUnreliableWindowType(WindowType windowType)
{
    if (windowType == WindowType::WINDOW_TYPE_APP_SUB_WINDOW ||
        windowType == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT ||
        windowType == WindowType::WINDOW_TYPE_TOAST) {
        return true;
    }
    TLOGD(WmsLogTag::DEFAULT, "false, WindowType=%{public}d", windowType);
    return false;
}

static void FillUnreliableWindowInfo(const sptr<SceneSession>& sceneSession,
    std::vector<sptr<UnreliableWindowInfo>>& infos)
{
    if (sceneSession == nullptr) {
        TLOGW(WmsLogTag::DEFAULT, "null scene session.");
        return;
    }
    if (sceneSession->GetSessionInfo().bundleName_.find("SCBGestureBack") != std::string::npos ||
        sceneSession->GetSessionInfo().bundleName_.find("SCBGestureNavBar") != std::string::npos ||
        sceneSession->GetSessionInfo().bundleName_.find("SCBGestureTopBar") != std::string::npos) {
        TLOGD(WmsLogTag::DEFAULT, "filter gesture window.");
        return;
    }
    sptr<UnreliableWindowInfo> info = sptr<UnreliableWindowInfo>::MakeSptr();
    info->windowId_ = sceneSession->GetPersistentId();
    WSRect windowRect = sceneSession->GetSessionRect();
    info->windowRect_ = { windowRect.posX_, windowRect.posY_, windowRect.width_, windowRect.height_ };
    info->zOrder_ = sceneSession->GetZOrder();
    info->floatingScale_ = sceneSession->GetFloatingScale();
    info->scaleX_ = sceneSession->GetScaleX();
    info->scaleY_ = sceneSession->GetScaleY();
    infos.emplace_back(info);
    TLOGD(WmsLogTag::WMS_MAIN, "wid=%{public}d", info->windowId_);
}

void SceneSessionManager::ApplyFeatureConfig(const std::unordered_map<std::string, std::string>& configMap)
{
    auto task = [this, where = __func__, &configMap] {
        if (convertConfigMap_.empty()) {
            convertConfigMap_ = {
                {"supportUIExtensionSubWindow", std::bind(&SystemSessionConfig::ConvertSupportUIExtensionSubWindow,
                    &systemConfig_, std::placeholders::_1)},
            };
        }
        for (const auto& [configName, configValue] : configMap) {
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s, configEntry is %{public}s: %{public}s",
                where, configName.c_str(), configValue.c_str());
            auto convertIter = convertConfigMap_.find(configName);
            if (convertIter != convertConfigMap_.end()) {
                auto convertFunc = convertIter->second;
                convertFunc(configValue);
            }
        }
        return WMError::WM_OK;
    };
    taskScheduler_->PostSyncTask(task, "ApplyFeatureConfig");
}

WMError SceneSessionManager::GetUnreliableWindowInfo(int32_t windowId,
    std::vector<sptr<UnreliableWindowInfo>>& infos)
{
    TLOGD(WmsLogTag::DEFAULT, "in.");
    if (!SessionPermission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::DEFAULT, "only support for system service.");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, windowId, &infos]() {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [sessionId, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                TLOGNW(WmsLogTag::DEFAULT, "null scene session");
                continue;
            }
            if (sessionId == windowId) {
                TLOGNI(WmsLogTag::DEFAULT, "persistentId: %{public}d is parameter chosen", sessionId);
                FillUnreliableWindowInfo(sceneSession, infos);
                continue;
            }
            if (sceneSession->GetSystemTouchable() && sceneSession->GetForegroundInteractiveStatus()) {
                TLOGND(WmsLogTag::DEFAULT, "persistentId: %{public}d is system touchable", sessionId);
                continue;
            }
            if (!sceneSession->GetRSVisible()) {
                TLOGND(WmsLogTag::DEFAULT, "persistentId: %{public}d is not visible", sessionId);
                continue;
            }
            TLOGND(WmsLogTag::DEFAULT, "name=%{public}s, isSystem=%{public}d, "
                "persistentId=%{public}d, winType=%{public}d, state=%{public}d, visible=%{public}d",
                sceneSession->GetWindowName().c_str(), sceneSession->GetSessionInfo().isSystem_, sessionId,
                sceneSession->GetWindowType(), sceneSession->GetSessionState(), sceneSession->GetRSVisible());
            if (CheckUnreliableWindowType(sceneSession->GetWindowType())) {
                TLOGI(WmsLogTag::DEFAULT, "persistentId=%{public}d, "
                    "WindowType=%{public}d", sessionId, sceneSession->GetWindowType());
                FillUnreliableWindowInfo(sceneSession, infos);
            }
        }
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, "GetUnreliableWindowInfo");
}

void SceneSessionManager::NotifyWindowInfoChange(int32_t persistentId, WindowUpdateType type)
{
    TLOGD(WmsLogTag::DEFAULT, "persistentId=%{public}d, updateType=%{public}d", persistentId, type);
    sptr<SceneSession> sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGD(WmsLogTag::DEFAULT, "sceneSession nullptr!");
        return;
    }
    wptr<SceneSession> weakSceneSession(sceneSession);
    if (processingFlushUIParams_.load()) {
        TLOGD(WmsLogTag::WMS_PIPELINE, "Processing flush, notify later.");
        auto task = [this, weakSceneSession, type]() {
            auto sceneSession = weakSceneSession.promote();
            if (WindowChangedFunc_ != nullptr && sceneSession != nullptr &&
                sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
                WindowChangedFunc_(sceneSession->GetPersistentId(), type);
            }
        };
        taskScheduler_->PostAsyncTask(task, "WindowChangeFunc:id:" + std::to_string(persistentId));
        return;
    }
    auto task = [this, weakSceneSession, type]() {
        auto sceneSession = weakSceneSession.promote();
        NotifyAllAccessibilityInfo();
        if (WindowChangedFunc_ != nullptr && sceneSession != nullptr &&
            sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
            WindowChangedFunc_(sceneSession->GetPersistentId(), type);
        }
    };
    taskScheduler_->PostAsyncTask(task, "NotifyWindowInfoChange:PID:" + std::to_string(persistentId));
    auto notifySceneInputTask = [weakSceneSession, type]() {
        auto sceneSession = weakSceneSession.promote();
        if (sceneSession == nullptr) {
            return;
        }
        SceneInputManager::GetInstance().NotifyWindowInfoChange(sceneSession, type);
    };
    taskScheduler_->PostAsyncTask(notifySceneInputTask, "notifySceneInputTask");
}

bool SceneSessionManager::FillWindowInfo(std::vector<sptr<AccessibilityWindowInfo>>& infos,
    const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "null scene session.");
        return false;
    }
    if (sceneSession->GetSessionInfo().bundleName_.find("SCBGestureBack") != std::string::npos ||
        sceneSession->GetSessionInfo().bundleName_.find("SCBGestureNavBar") != std::string::npos ||
        sceneSession->GetSessionInfo().bundleName_.find("SCBGestureTopBar") != std::string::npos) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "filter gesture window.");
        return false;
    }
    if (sceneSession->GetSessionInfo().bundleName_.find("SCBDragScale") != std::string::npos) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "filter DragScale window.");
        return false;
    }
    if (sceneSession->GetHidingStartingWindow()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "filter hiding starting win: %{public}d", sceneSession->GetPersistentId());
        return false;
    }
    sptr<AccessibilityWindowInfo> info = sptr<AccessibilityWindowInfo>::MakeSptr();
    if (sceneSession->GetSessionInfo().isSystem_) {
        info->wid_ = 1;
        info->innerWid_ = static_cast<int32_t>(sceneSession->GetPersistentId());
    } else {
        info->wid_ = static_cast<int32_t>(sceneSession->GetPersistentId());
    }
    info->uiNodeId_ = sceneSession->GetUINodeId();
    info->type_ = sceneSession->GetWindowType();
    info->mode_ = sceneSession->GetWindowMode();
    info->layer_ = sceneSession->GetZOrder();
    info->scaleVal_ = sceneSession->GetFloatingScale();
    info->scaleX_ = sceneSession->GetScaleX();
    info->scaleY_ = sceneSession->GetScaleY();
    info->isCompatScaleMode_ = sceneSession->IsInCompatScaleMode();
    info->bundleName_ = sceneSession->GetSessionInfo().bundleName_;
    info->touchHotAreas_ = sceneSession->GetTouchHotAreas();
    info->isDecorEnable_ = sceneSession->GetSessionProperty()->IsDecorEnable();
    WSRect wsRect = sceneSession->GetSessionGlobalRectWithSingleHandScale(); // only accessability and mmi need global
    DisplayId displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    if (!sceneSession->GetSessionInfo().isSystem_ &&
        PcFoldScreenManager::GetInstance().IsHalfFoldedOnMainDisplay(displayId) &&
        !sceneSession->GetSessionProperty()->IsSystemKeyboard()) {
        displayId = sceneSession->TransformGlobalRectToRelativeRect(wsRect);
    }
    info->displayId_ = displayId;
    info->focused_ = sceneSession->GetPersistentId() == GetFocusedSessionId(displayId);
    info->windowRect_ = { wsRect.posX_, wsRect.posY_, wsRect.width_, wsRect.height_ };
    sceneSession->GetGlobalScaledRect(info->scaleRect_);
    infos.emplace_back(info);
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "wid: %{public}d, innerWid: %{public}d, nodeId: %{public}d"
        ", bundleName: %{public}s, displayId: %{public}" PRIu64 ", rect: %{public}s",
        info->wid_, info->innerWid_, info->uiNodeId_, info->bundleName_.c_str(),
        info->displayId_, info->windowRect_.ToString().c_str());
    return true;
}

sptr<SceneSession> SceneSessionManager::SelectSesssionFromMap(const uint64_t& surfaceId)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession == nullptr) {
            continue;
        }
        if (sceneSession->GetSurfaceNode() == nullptr) {
            continue;
        }
        if (surfaceId == sceneSession->GetSurfaceNode()->GetId()) {
            return sceneSession;
        }
    }
    return nullptr;
}

void SceneSessionManager::WindowLayerInfoChangeCallback(std::shared_ptr<RSOcclusionData> occlusiontionData)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "in");
    auto task = [this, weak = std::weak_ptr<RSOcclusionData>(occlusiontionData)]() {
        auto weakOcclusionData = weak.lock();
        if (weakOcclusionData == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "weak occlusionData is nullptr");
            return;
        }
        std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
        std::vector<std::pair<uint64_t, bool>> currDrawingContentData;
        GetWindowLayerChangeInfo(weakOcclusionData, currVisibleData, currDrawingContentData);
        std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfos;
        if (currVisibleData.size() != 0) {
            visibilityChangeInfos = GetWindowVisibilityChangeInfo(currVisibleData);
        }
        if (visibilityChangeInfos.size() != 0) {
            DealwithVisibilityChange(visibilityChangeInfos, currVisibleData);
            CacVisibleWindowNum();
        }

        std::vector<std::pair<uint64_t, bool>> drawingContentChangeInfos;
        if (currDrawingContentData.size() != 0) {
            drawingContentChangeInfos = GetWindowDrawingContentChangeInfo(currDrawingContentData);
        }
        if (drawingContentChangeInfos.size() != 0) {
            DealwithDrawingContentChange(drawingContentChangeInfos);
        }
    };
    taskScheduler_->PostVoidSyncTask(task, "WindowLayerInfoChangeCallback");
}

void SceneSessionManager::GetWindowLayerChangeInfo(std::shared_ptr<RSOcclusionData> occlusiontionData,
    std::vector<std::pair<uint64_t, WindowVisibilityState>>& currVisibleData,
    std::vector<std::pair<uint64_t, bool>>& currDrawingContentData)
{
    VisibleData& rsVisibleData = occlusiontionData->GetVisibleData();
    for (auto iter = rsVisibleData.begin(); iter != rsVisibleData.end(); iter++) {
        WindowLayerState windowLayerState = static_cast<WindowLayerState>(iter->second);
        auto visibilityState = static_cast<WindowVisibilityState>(iter->second);
        sptr<SceneSession> session = SelectSesssionFromMap(iter->first);
        switch (windowLayerState) {
            case WINDOW_ALL_VISIBLE:
            case WINDOW_SEMI_VISIBLE:
                currVisibleData.emplace_back(iter->first, static_cast<WindowVisibilityState>(iter->second));
                break;
            case WINDOW_IN_VISIBLE:
                if (session != nullptr && session->GetHidingStartingWindow()) {
                    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "change to visible: %{public}d", session->GetPersistentId());
                    visibilityState = WINDOW_VISIBILITY_STATE_NO_OCCLUSION;
                }
                currVisibleData.emplace_back(iter->first, visibilityState);
                break;
            case WINDOW_LAYER_DRAWING:
                currDrawingContentData.emplace_back(iter->first, true);
                break;
            case WINDOW_LAYER_NO_DRAWING:
                currDrawingContentData.emplace_back(iter->first, false);
                break;
            default:
                break;
        }
    }
}

void SceneSessionManager::UpdateSubWindowVisibility(const sptr<SceneSession>& session,
    WindowVisibilityState visibleState,
    const std::vector<std::pair<uint64_t, WindowVisibilityState>>& visibilityChangeInfo,
    std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos, std::string& visibilityInfo,
    const std::vector<std::pair<uint64_t, WindowVisibilityState>>& currVisibleData)
{
    if (WindowHelper::IsMainWindow(session->GetWindowType()) &&
            visibleState < WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
        auto subSessions = GetSubSceneSession(session->GetWindowId());
        if (subSessions.empty()) {
            return;
        }

        RemoveDuplicateSubSession(visibilityChangeInfo, subSessions);

        for (const auto& subSession : subSessions) {
            if (subSession == nullptr) {
                continue;
            }
            WindowVisibilityState subWinVisibleState = visibleState;
            if (GetSessionRSVisible(subSession, currVisibleData, subWinVisibleState)) {
                TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Update subwindow visibility for winId: %{public}d",
                    subSession->GetWindowId());
                SetSessionVisibilityInfo(subSession, subWinVisibleState, windowVisibilityInfos, visibilityInfo);
            }
        }
    }
}

bool SceneSessionManager::GetSessionRSVisible(const sptr<Session>& session,
    const std::vector<std::pair<uint64_t, WindowVisibilityState>>& currVisibleData,
    WindowVisibilityState& sessionVisibleState)
{
    bool sessionRSVisible = false;
    for (const auto& [surfaceId, visibleState] : currVisibleData) {
        sptr<SceneSession> visibilitySession = SelectSesssionFromMap(surfaceId);
        if (visibilitySession == nullptr) {
            continue;
        }
        if (session->GetWindowId() == visibilitySession->GetWindowId()) {
            if (visibleState < WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
                sessionRSVisible = true;
                sessionVisibleState = visibleState;
            }
            break;
        }
    }
    return sessionRSVisible;
}

void SceneSessionManager::SetSessionVisibilityInfo(const sptr<SceneSession>& session,
    WindowVisibilityState visibleState, std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfos,
    std::string& visibilityInfo)
{
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "Session is invalid!");
        return;
    }
    session->SetRSVisible(visibleState < WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    session->SetVisibilityState(visibleState);
    int32_t windowId = session->GetWindowId();
    if (windowVisibilityListenerSessionSet_.find(windowId) != windowVisibilityListenerSessionSet_.end()) {
        session->NotifyWindowVisibility();
    }
    if (occlusionStateListenerSessionSet_.count(windowId) > 0) {
        session->NotifyOcclusionState();
    }
    auto windowVisibilityInfo = sptr<WindowVisibilityInfo>::MakeSptr(
        windowId, session->GetCallingPid(), session->GetCallingUid(), visibleState, session->GetWindowType());
    windowVisibilityInfo->SetAppIndex(session->GetSessionInfo().appIndex_);
    windowVisibilityInfo->SetBundleName(session->GetSessionInfo().bundleName_);
    windowVisibilityInfo->SetAbilityName(session->GetSessionInfo().abilityName_);
    windowVisibilityInfo->SetIsSystem(session->GetSessionInfo().isSystem_);
    windowVisibilityInfo->SetZOrder(session->GetZOrder());

    int32_t callingWindowId = session->GetSessionInfo().callerPersistentId_;
    sptr<SceneSession> callerSession = GetSceneSession(callingWindowId);
    if (callerSession) {
        windowVisibilityInfo->SetCallingPid(callerSession->GetCallingPid());
    }
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "callingWindowId %{public}d, callingPid %{public}d",
        callingWindowId, windowVisibilityInfo->GetCallingPid());

    windowVisibilityInfos.emplace_back(windowVisibilityInfo);

    visibilityInfo +=
        "[" + session->GetWindowName() + ", " + std::to_string(windowId) + ", " + std::to_string(visibleState) + "], ";
}

void SceneSessionManager::RemoveDuplicateSubSession(
    const std::vector<std::pair<uint64_t, WindowVisibilityState>>& visibilityChangeInfo,
    std::vector<sptr<SceneSession>>& subSessions)
{
    for (const auto& elem : visibilityChangeInfo) {
        uint64_t surfaceId = elem.first;
        sptr<SceneSession> session = SelectSesssionFromMap(surfaceId);
        if (session == nullptr) {
            continue;
        }
        for (auto iterator = subSessions.begin(); iterator != subSessions.end();) {
            auto subSession = *iterator;
            if (subSession && subSession->GetWindowId() == session->GetWindowId()) {
                iterator = subSessions.erase(iterator);
            } else {
                ++iterator;
            }
        }
    }
}

std::vector<sptr<SceneSession>> SceneSessionManager::GetSubSceneSession(int32_t parentWindowId)
{
    std::vector<sptr<SceneSession>> subSessions;
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& iter : sceneSessionMap_) {
        auto sceneSession = iter.second;
        if (sceneSession == nullptr) {
            continue;
        }
        if (WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
            continue;
        }
        const auto& mainOrFloatSession = sceneSession->GetMainOrFloatSession();
        if (mainOrFloatSession != nullptr && mainOrFloatSession->GetWindowId() == parentWindowId) {
            subSessions.push_back(sceneSession);
        }
    }
    return subSessions;
}

std::vector<std::pair<uint64_t, WindowVisibilityState>> SceneSessionManager::GetWindowVisibilityChangeInfo(
    std::vector<std::pair<uint64_t, WindowVisibilityState>>& currVisibleData)
{
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfo;
    std::sort(currVisibleData.begin(), currVisibleData.end(), Comp);
    uint32_t i, j;
    i = j = 0;
    for (; i < lastVisibleData_.size() && j < currVisibleData.size();) {
        if (lastVisibleData_[i].first < currVisibleData[j].first) {
            if (IsLastPiPWindowVisible(lastVisibleData_[i].first, lastVisibleData_[i].second)) {
                i++;
                continue;
            }
            if (lastVisibleData_[i].second != WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
                visibilityChangeInfo.emplace_back(lastVisibleData_[i].first, WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
            }
            i++;
        } else if (lastVisibleData_[i].first > currVisibleData[j].first) {
            if (currVisibleData[j].second != WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
                visibilityChangeInfo.emplace_back(currVisibleData[j].first, currVisibleData[j].second);
            }
            j++;
        } else {
            if (lastVisibleData_[i].second != currVisibleData[j].second &&
                !IsLastPiPWindowVisible(lastVisibleData_[i].first, lastVisibleData_[i].second)) {
                visibilityChangeInfo.emplace_back(currVisibleData[j].first, currVisibleData[j].second);
            }
            i++;
            j++;
        }
    }
    for (; i < lastVisibleData_.size(); ++i) {
        if (IsLastPiPWindowVisible(lastVisibleData_[i].first, lastVisibleData_[i].second)) {
            continue;
        }
        if (lastVisibleData_[i].second != WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
            visibilityChangeInfo.emplace_back(lastVisibleData_[i].first, WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
        }
    }
    for (; j < currVisibleData.size(); ++j) {
        if (currVisibleData[j].second != WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
            visibilityChangeInfo.emplace_back(currVisibleData[j].first, currVisibleData[j].second);
        }
    }
    lastVisibleData_ = currVisibleData;
    return visibilityChangeInfo;
}

void SceneSessionManager::DealwithVisibilityChange(const std::vector<std::pair<uint64_t, WindowVisibilityState>>&
    visibilityChangeInfo, const std::vector<std::pair<uint64_t, WindowVisibilityState>>& currVisibleData)
{
    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
#ifdef MEMMGR_WINDOW_ENABLE
    std::vector<sptr<Memory::MemMgrWindowInfo>> memMgrWindowInfos;
#endif

    std::string visibilityInfo = "WindowVisibilityInfos [name, winId, visibleState]: ";
    for (const auto& elem : visibilityChangeInfo) {
        uint64_t surfaceId = elem.first;
        WindowVisibilityState visibleState = elem.second;
        bool isVisible = visibleState < WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION;
        sptr<SceneSession> session = SelectSesssionFromMap(surfaceId);
        if (session == nullptr) {
            continue;
        }
        if ((WindowHelper::IsSubWindow(session->GetWindowType()) ||
            session->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) && isVisible == true) {
            WindowVisibilityState parentWinVisibleState = visibleState;
            if (session->GetParentSession() != nullptr &&
                !session->GetParentSession()->IsSessionForeground() &&
                !GetSessionRSVisible(session->GetParentSession(), currVisibleData, parentWinVisibleState)) {
                continue;
            }
        }
        SetSessionVisibilityInfo(session, visibleState, windowVisibilityInfos, visibilityInfo);
        UpdateSubWindowVisibility(session, visibleState, visibilityChangeInfo, windowVisibilityInfos, visibilityInfo, currVisibleData);
#ifdef MEMMGR_WINDOW_ENABLE
        memMgrWindowInfos.emplace_back(new Memory::MemMgrWindowInfo(session->GetWindowId(), session->GetCallingPid(),
            session->GetCallingUid(), isVisible));
#endif
        CheckAndNotifyWaterMarkChangedResult();
    }
    if (windowVisibilityInfos.size() != 0) {
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "Visibility changed, size: %{public}zu, %{public}s", windowVisibilityInfos.size(),
            visibilityInfo.c_str());
        SessionManagerAgentController::GetInstance().UpdateWindowVisibilityInfo(windowVisibilityInfos);
    }
    ProcessWindowModeType();
#ifdef MEMMGR_WINDOW_ENABLE
    if (memMgrWindowInfos.size() != 0) {
        taskScheduler_->PostAsyncTaskToExportHandler([memMgrWindowInfos = std::move(memMgrWindowInfos)]() {
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "memMgrWindowInfos size: %{public}zu", memMgrWindowInfos.size());
            Memory::MemMgrClient::GetInstance().OnWindowVisibilityChanged(memMgrWindowInfos);
        }, "notifyMemMgr");
    }
#endif
}

void SceneSessionManager::DealwithDrawingContentChange(const std::vector<std::pair<uint64_t, bool>>&
    drawingContentChangeInfo)
{
    std::vector<sptr<WindowDrawingContentInfo>> windowDrawingContenInfos;
    for (const auto& [surfaceId, drawingState] : drawingContentChangeInfo) {
        int32_t windowId = 0;
        int32_t pid = 0;
        int32_t uid = 0;
        WindowType type = WindowType::APP_WINDOW_BASE;
        sptr<SceneSession> session = SelectSesssionFromMap(surfaceId);
        if (session == nullptr) {
            if (!GetSpecifiedDrawingData(surfaceId, pid, uid)) {
                continue;
            }
            RemoveSpecifiedDrawingData(surfaceId);
        } else {
            windowId = session->GetWindowId();
            pid = session->GetCallingPid();
            uid = session->GetCallingUid();
            type = session->GetWindowType();
        }
        windowDrawingContenInfos.emplace_back(new WindowDrawingContentInfo(windowId, pid, uid, drawingState, type));
        if (openDebugTrace_) {
            HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "Drawing status changed pid:(%d ) surfaceId:(%" PRIu64 ")"
                "drawingState:(%d )", pid, surfaceId, drawingState);
        }
        TLOGD(WmsLogTag::DEFAULT, "drawing status changed pid:%{public}d, "
            "surfaceId:%{public}" PRIu64 ", drawingState:%{public}d", pid, surfaceId, drawingState);
    }
    if (windowDrawingContenInfos.size() != 0) {
        TLOGD(WmsLogTag::DEFAULT, "Notify WindowDrawingContenInfo changed start");
        SessionManagerAgentController::GetInstance().UpdateWindowDrawingContentInfo(windowDrawingContenInfos);
    }
}

void SceneSessionManager::NotifySupportRotationChange(const SupportRotationInfo& supportRotationInfo)
{
    SessionManagerAgentController::GetInstance().NotifySupportRotationChange(supportRotationInfo);
}

WSError SceneSessionManager::NotifyAppUseControlList(
    ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList)
{
    TLOGI(WmsLogTag::WMS_LIFE,
        "controlApptype: %{public}d userId: %{public}d controlList size: %{public}zu",
        static_cast<int>(type), userId, controlList.size());
    if (!SessionPermission::IsSACalling()) {
        TLOGW(WmsLogTag::WMS_LIFE, "The caller is not system-app, can not use system-api");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    if (type == ControlAppType::APP_LOCK &&
        !SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_WRITE_APP_LOCK)) {
        TLOGW(WmsLogTag::WMS_LIFE, "write app lock permission denied");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    if (currentUserId_ != userId) {
        if (currentUserId_ != DEFAULT_USERID) {
            TLOGW(WmsLogTag::WMS_LIFE, "invalid userId, currentUserId_:%{public}d userId:%{public}d",
                currentUserId_.load(), userId);
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        int32_t userIdByUid = GetUserIdByUid(getuid());
        if (userId != userIdByUid) {
            TLOGW(WmsLogTag::WMS_LIFE,
                "invalid userId, currentUserId_:%{public}d userId:%{public}d GetUserIdByUid:%{public}d",
                currentUserId_.load(), userId, userIdByUid);
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
    }
    taskScheduler_->PostAsyncTask([this, type, userId, controlList] {
        NotifyAppUseControlListInner(type, userId, controlList);
    }, __func__);
    return WSError::WS_OK;
}

void SceneSessionManager::NotifyAppUseControlListInner(
    ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList)
{
    std::vector<sptr<SceneSession>> mainSessions;
    std::vector<AppUseControlInfo> controlByBundleList;
    for (const auto& appUseControlInfo : controlList) {
        if (appUseControlInfo.persistentId_ > INVALID_WINDOW_ID) {
            // control by peristentId
            TLOGI(WmsLogTag::WMS_MAIN, "control by id:%{public}d", appUseControlInfo.persistentId_);
            if (auto session = GetSessionForAppUseControl(appUseControlInfo)) {
                mainSessions.push_back(session);
            }
        } else {
            // control by bundleName and appIndex
            RefreshAllAppUseControlMap(appUseControlInfo, type);
            GetMainSessionByBundleNameAndAppIndex(appUseControlInfo.bundleName_, appUseControlInfo.appIndex_, mainSessions);
            controlByBundleList.push_back(appUseControlInfo);
        }
        if (mainSessions.empty()) {
            continue;
        }
        ControlInfo controlInfo = {
            .isNeedControl = appUseControlInfo.isNeedControl_,
            .isControlRecentOnly = appUseControlInfo.isControlRecentOnly_
        };
        for (const auto& session : mainSessions) {
            session->NotifyUpdateAppUseControl(type, controlInfo);
        }
        mainSessions.clear();
    }
    if (notifyAppUseControlListFunc_ != nullptr && controlByBundleList.size() > 0) {
        notifyAppUseControlListFunc_(type, userId, controlByBundleList);
    }
}

sptr<SceneSession> SceneSessionManager::GetSessionForAppUseControl(const AppUseControlInfo& appUseControlInfo)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    auto iter = sceneSessionMap_.find(appUseControlInfo.persistentId_);
    if (iter == sceneSessionMap_.end() || iter->second == nullptr) {
        TLOGW(WmsLogTag::WMS_MAIN, "session not found, id:%{public}d", appUseControlInfo.persistentId_);
        return nullptr;
    }
    if (iter->second->GetSessionInfo().bundleName_ != appUseControlInfo.bundleName_ ||
        iter->second->GetSessionInfo().appIndex_ != appUseControlInfo.appIndex_ ||
        !SessionHelper::IsMainWindow(iter->second->GetWindowType())) {
        TLOGW(WmsLogTag::WMS_MAIN, "invalid session, id:%{public}d", appUseControlInfo.persistentId_);
        return nullptr;
    }
    return iter->second;
}

void SceneSessionManager::RefreshAllAppUseControlMap(const AppUseControlInfo& appUseControlInfo, ControlAppType type)
{
    ControlInfo controlInfo = {
        .isNeedControl = appUseControlInfo.isNeedControl_,
        .isControlRecentOnly = appUseControlInfo.isControlRecentOnly_
    };
    std::string key = SessionUtils::GetAppLockKey(appUseControlInfo.bundleName_, appUseControlInfo.appIndex_);
    if (!controlInfo.isNeedControl && !controlInfo.isControlRecentOnly) {
        if (allAppUseControlMap_.find(key) != allAppUseControlMap_.end()) {
            allAppUseControlMap_[key].erase(type);
            if (allAppUseControlMap_[key].empty()){
                allAppUseControlMap_.erase(key);
            }
        }
    } else {
        allAppUseControlMap_[key][type] = controlInfo;
    }
}

void SceneSessionManager::RegisterNotifyAppUseControlListCallback(NotifyAppUseControlListFunc&& func)
{
    taskScheduler_->PostAsyncTask([this, callback = std::move(func)] {
        notifyAppUseControlListFunc_ = std::move(callback);
    }, __func__);
}

bool SceneSessionManager::GetSpecifiedDrawingData(uint64_t surfaceId, int32_t& pid, int32_t& uid)
{
    auto it = lastDrawingSessionInfoMap_.find(surfaceId);
    if (it != lastDrawingSessionInfoMap_.end()) {
        pid = it->second.pid_;
        uid = it->second.uid_;
        return true;
    }
    return false;
}

void SceneSessionManager::RemoveSpecifiedDrawingData(uint64_t surfaceId)
{
    auto it = lastDrawingSessionInfoMap_.find(surfaceId);
    if (it != lastDrawingSessionInfoMap_.end()) {
        lastDrawingSessionInfoMap_.erase(it);
    }
}

std::vector<std::pair<uint64_t, bool>> SceneSessionManager::GetWindowDrawingContentChangeInfo(
    const std::vector<std::pair<uint64_t, bool>>& currDrawingContentData)
{
    std::vector<std::pair<uint64_t, bool>> processDrawingContentChangeInfo;
    for (const auto& [surfaceId, isWindowDrawing] : currDrawingContentData) {
        int32_t pid = 0;
        sptr<SceneSession> session = SelectSesssionFromMap(surfaceId);
        bool isDrawingStateChanged =
            session == nullptr || (GetPreWindowDrawingState(surfaceId, isWindowDrawing, pid) != isWindowDrawing &&
                                   GetProcessDrawingState(surfaceId, pid));
        if (isDrawingStateChanged) {
            processDrawingContentChangeInfo.emplace_back(surfaceId, isWindowDrawing);
        }
    }
    return processDrawingContentChangeInfo;
}

bool SceneSessionManager::GetPreWindowDrawingState(uint64_t surfaceId, bool currentWindowDrawing, int32_t& pid)
{
    sptr<SceneSession> session = SelectSesssionFromMap(surfaceId);
    if (session == nullptr) {
        return false;
    }
    pid = session->GetCallingPid();
    bool preWindowDrawing = session->GetDrawingContentState();
    session->SetDrawingContentState(currentWindowDrawing);
    UpdateWindowDrawingData(surfaceId, pid, session->GetCallingUid());
    return preWindowDrawing;
}

void SceneSessionManager::UpdateWindowDrawingData(uint64_t surfaceId, int32_t pid, int32_t uid)
{
    lastDrawingSessionInfoMap_[surfaceId] = { pid, uid };
}

bool SceneSessionManager::GetProcessDrawingState(uint64_t surfaceId, int32_t pid)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession == nullptr) {
            continue;
        }
        if (sceneSession->GetCallingPid() == pid && sceneSession->GetSurfaceNode() != nullptr &&
            surfaceId != sceneSession->GetSurfaceNode()->GetId()) {
            if (sceneSession->GetDrawingContentState()) {
                return false;
            }
        }
    }
    return true;
}

void SceneSessionManager::InitWithRenderServiceAdded()
{
    auto windowVisibilityChangeCb = [this](std::shared_ptr<RSOcclusionData> occlusiontionData) {
        this->WindowLayerInfoChangeCallback(occlusiontionData);
    };
    TLOGI(WmsLogTag::DEFAULT, "RegisterWindowVisibilityChangeCallback");
    if (rsInterface_.RegisterOcclusionChangeCallback(windowVisibilityChangeCb) != WM_OK) {
        TLOGE(WmsLogTag::DEFAULT, "RegisterWindowVisibilityChangeCallback failed");
    }
}

WMError SceneSessionManager::SetSystemAnimatedScenes(SystemAnimatedSceneType sceneType, bool isRegularAnimation)
{
    if (sceneType > SystemAnimatedSceneType::SCENE_OTHERS) {
        TLOGE(WmsLogTag::DEFAULT, "The input scene type is valid, scene type is %{public}d", sceneType);
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    auto task = [this, sceneType, isRegularAnimation]() {
        TLOGND(WmsLogTag::WMS_PC, "Set system animated scene %{public}d.", sceneType);
        bool ret = rsInterface_.SetSystemAnimatedScenes(
            static_cast<SystemAnimatedScenes>(sceneType), isRegularAnimation);
        if (!ret) {
            TLOGNE(WmsLogTag::WMS_PC, "Set system animated scene failed.");
        }
    };
    taskScheduler_->PostAsyncTask(task, "SetSystemAnimatedScenes");
    return WMError::WM_OK;
}

WSError SceneSessionManager::NotifyWindowExtensionVisibilityChange(int32_t pid, int32_t uid, bool visible)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (pid != IPCSkeleton::GetCallingRealPid() || uid != IPCSkeleton::GetCallingUid()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "pid and uid check failed!");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    TLOGI(WmsLogTag::WMS_UIEXT, "visibility change to %{public}s for pid: %{public}d, uid: %{public}d",
        visible ? "VISIBLE" : "INVISIBLE", pid, uid);
    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
    windowVisibilityInfos.emplace_back(new WindowVisibilityInfo(INVALID_WINDOW_ID, pid, uid,
        visible ? WINDOW_VISIBILITY_STATE_NO_OCCLUSION : WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION,
        WindowType::WINDOW_TYPE_APP_COMPONENT));
    SessionManagerAgentController::GetInstance().UpdateWindowVisibilityInfo(windowVisibilityInfos);
    return WSError::WS_OK;
}

void SceneSessionManager::WindowDestroyNotifyVisibility(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "sceneSession is nullptr!");
        return;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "in, wid: %{public}d, RSVisible: %{public}d, WindowMode: %{public}u",
        sceneSession->GetWindowId(), sceneSession->GetRSVisible(), sceneSession->GetWindowMode());
    if (sceneSession->GetRSVisible() || sceneSession->GetWindowMode() == WindowMode::WINDOW_MODE_PIP) {
        std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
#ifdef MEMMGR_WINDOW_ENABLE
        std::vector<sptr<Memory::MemMgrWindowInfo>> memMgrWindowInfos;
#endif
        sceneSession->SetRSVisible(false);
        sceneSession->SetVisibilityState(WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
        sceneSession->ClearExtWindowFlags();
        auto windowVisibilityInfo = new WindowVisibilityInfo(sceneSession->GetWindowId(),
            sceneSession->GetCallingPid(), sceneSession->GetCallingUid(),
            WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION, sceneSession->GetWindowType());
        windowVisibilityInfo->SetAppIndex(sceneSession->GetSessionInfo().appIndex_);
        windowVisibilityInfo->SetBundleName(sceneSession->GetSessionInfo().bundleName_);
        windowVisibilityInfo->SetAbilityName(sceneSession->GetSessionInfo().abilityName_);
        windowVisibilityInfo->SetIsSystem(sceneSession->GetSessionInfo().isSystem_);
        windowVisibilityInfo->SetZOrder(sceneSession->GetZOrder());
        windowVisibilityInfos.emplace_back(windowVisibilityInfo);
#ifdef MEMMGR_WINDOW_ENABLE
        memMgrWindowInfos.emplace_back(new Memory::MemMgrWindowInfo(sceneSession->GetWindowId(),
            sceneSession->GetCallingPid(), sceneSession->GetCallingUid(), false));
#endif
        TLOGD(WmsLogTag::DEFAULT, "covered status changed window:%{public}u, isVisible:%{public}d",
            sceneSession->GetWindowId(), sceneSession->GetRSVisible());
        CheckAndNotifyWaterMarkChangedResult();
        SessionManagerAgentController::GetInstance().UpdateWindowVisibilityInfo(windowVisibilityInfos);
#ifdef MEMMGR_WINDOW_ENABLE
        TLOGD(WmsLogTag::DEFAULT, "Notify memMgrWindowInfos changed start");
        Memory::MemMgrClient::GetInstance().OnWindowVisibilityChanged(memMgrWindowInfos);
#endif
    }
}

sptr<SceneSession> SceneSessionManager::FindSessionByToken(const sptr<IRemoteObject>& token, WindowType type)
{
    if (token == nullptr) {
        TLOGW(WmsLogTag::WMS_MAIN, "token is nullptr");
        return nullptr;
    }
    sptr<SceneSession> session = nullptr;
    auto cmpFunc = [token, type](const std::map<uint64_t, sptr<SceneSession>>::value_type& pair) {
        if (pair.second == nullptr) {
            return false;
        }
        if (pair.second->GetWindowType() == type) {
            return pair.second->GetAbilityToken() == token || pair.second->AsObject() == token;
        }
        return false;
    };
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    auto iter = std::find_if(sceneSessionMap_.begin(), sceneSessionMap_.end(), cmpFunc);
    if (iter != sceneSessionMap_.end()) {
        session = iter->second;
    }
    return session;
}

sptr<SceneSession> SceneSessionManager::FindSessionByAffinity(const std::string& affinity)
{
    if (affinity.size() == 0) {
        TLOGI(WmsLogTag::DEFAULT, "AbilityInfo affinity is empty");
        return nullptr;
    }
    sptr<SceneSession> session = nullptr;
    auto cmpFunc = [this, &affinity](const auto& pair) {
        auto sceneSession = pair.second;
        if (sceneSession == nullptr || !CheckCollaboratorType(sceneSession->GetCollaboratorType())) {
            return false;
        }
        return sceneSession->GetSessionInfo().sessionAffinity == affinity;
    };
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    if (auto iter = std::find_if(sceneSessionMap_.begin(), sceneSessionMap_.end(), cmpFunc);
        iter != sceneSessionMap_.end()) {
        session = iter->second;
    }
    return session;
}

void SceneSessionManager::PreloadInLakeApp(const std::string& bundleName)
{
    TLOGD(WmsLogTag::DEFAULT, "name %{public}s", bundleName.c_str());
    if (auto collaborator = GetCollaboratorByType(CollaboratorType::RESERVE_TYPE)) {
        TLOGI(WmsLogTag::DEFAULT, "NotifyPreloadAbility: %{public}s", bundleName.c_str());
        collaborator->NotifyPreloadAbility(bundleName);
    }
}

WSError SceneSessionManager::PendingSessionToForeground(const sptr<IRemoteObject>& token, int32_t windowMode)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    auto pid = IPCSkeleton::GetCallingRealPid();
    if (!SessionPermission::IsSACalling() && !SessionPermission::CheckCallingIsUserTestMode(pid)) {
        TLOGE(WmsLogTag::DEFAULT, "Permission denied for going to foreground!");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }

    return taskScheduler_->PostSyncTask([this, &token, windowMode]() {
        if (auto session = FindSessionByToken(token)) {
            if (windowMode != DEFAULT_INVALID_WINDOW_MODE) {
                TLOGI(WmsLogTag::WMS_LIFE, "Modify window mode: %{public}d", windowMode);
                session->SetSessionInfoWindowMode(windowMode);
            }
            return session->PendingSessionToForeground();
        }
        TLOGNE(WmsLogTag::DEFAULT, "PendingForeground: fail to find token");
        return WSError::WS_ERROR_INVALID_PARAM;
    }, __func__);
}

WSError SceneSessionManager::PendingSessionToBackground(const sptr<IRemoteObject>& token,
    const BackgroundParams& params)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::DEFAULT, "Permission denied for going to background!");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    return taskScheduler_->PostSyncTask([this, &token, params] {
        sptr<SceneSession> session = nullptr;
        if (params.persistentId > INVALID_SESSION_ID) {
            TLOGNI(WmsLogTag::WMS_LIFE, "Find scene sesion by persistentId: %{public}d", params.persistentId);
            session = GetSceneSession(params.persistentId);
        } else if (token != nullptr) {
            TLOGNI(WmsLogTag::WMS_LIFE, "Find scene sesion by token");
            session = FindSessionByToken(token);
        }
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "fail to find session");
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        return session->PendingSessionToBackground(params);
    }, __func__);
}

WSError SceneSessionManager::PendingSessionToBackgroundForDelegator(const sptr<IRemoteObject>& token,
    bool shouldBackToCaller)
{
    return taskScheduler_->PostSyncTask([this, &token, shouldBackToCaller] {
        if (auto session = FindSessionByToken(token)) {
            return session->PendingSessionToBackgroundForDelegator(shouldBackToCaller);
        }
        TLOGNE(WmsLogTag::WMS_LIFE, "fail to find token");
        return WSError::WS_ERROR_INVALID_PARAM;
    }, __func__);
}

void SceneSessionManager::ClearDisplayStatusBarTemporarilyFlags()
{
    for (auto persistentId : avoidAreaListenerSessionSet_) {
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            continue;
        }
        sceneSession->SetIsDisplayStatusBarTemporarily(false);
    }
}

WSError SceneSessionManager::GetFocusSessionToken(sptr<IRemoteObject>& token, DisplayId displayId)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "permission denied!");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    return taskScheduler_->PostSyncTask([this, &token, where = __func__, displayId]() {
        auto focusGroup = windowFocusController_->GetFocusGroup(displayId);
        if (focusGroup == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, displayId);
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        TLOGND(WmsLogTag::WMS_FOCUS, "%{public}s with focusedSessionId: %{public}d",
            where, focusGroup->GetFocusedSessionId());
        if (auto sceneSession = GetSceneSession(focusGroup->GetFocusedSessionId())) {
            token = sceneSession->GetAbilityToken();
            if (token == nullptr) {
                TLOGNE(WmsLogTag::WMS_FOCUS, "token is nullptr");
                return WSError::WS_ERROR_INVALID_PARAM;
            }
            return WSError::WS_OK;
        }
        return WSError::WS_ERROR_INVALID_SESSION;
    }, __func__);
}

WSError SceneSessionManager::GetFocusSessionElement(AppExecFwk::ElementName& element, DisplayId displayId)
{
    auto pid = IPCSkeleton::GetCallingRealPid();
    AppExecFwk::RunningProcessInfo info;
    DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetRunningProcessInfoByPid(pid, info);
    if (!info.isTestProcess && !SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "permission denied!");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    return taskScheduler_->PostSyncTask([this, &element, where = __func__, displayId]() {
        auto focusGroup = windowFocusController_->GetFocusGroup(displayId);
        if (focusGroup == nullptr) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, displayId);
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        TLOGND(WmsLogTag::WMS_FOCUS, "%{public}s with focusedSessionId: %{public}d",
            where, focusGroup->GetFocusedSessionId());
        if (auto sceneSession = GetSceneSession(focusGroup->GetFocusedSessionId())) {
            const auto& sessionInfo = sceneSession->GetSessionInfo();
            element = AppExecFwk::ElementName("", sessionInfo.bundleName_,
                sessionInfo.abilityName_, sessionInfo.moduleName_);
            return WSError::WS_OK;
        }
        return WSError::WS_ERROR_INVALID_SESSION;
    }, __func__);
}

WSError SceneSessionManager::IsFocusWindowParent(const sptr<IRemoteObject>& token, bool& isParent)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "permission denied!");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    return taskScheduler_->PostSyncTask([this, &token, &isParent, where = __func__]() {
        std::vector<std::pair<DisplayId, int32_t>> allFocusedSessionList =
            windowFocusController_->GetAllFocusedSessionList();
        if (allFocusedSessionList.size() == 0) {
            TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s has no focus group",  where);
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        for (const auto& focusState : allFocusedSessionList) {
            auto focusedSession = GetSceneSession(focusState.second);
            if (focusedSession == nullptr) {
                TLOGNE(WmsLogTag::WMS_FOCUS, "%{public}s session is nullptr: %{public}d",  where, focusState.second);
                return WSError::WS_ERROR_INVALID_SESSION;
            }
            if (focusedSession->GetAbilityToken() == token || focusedSession->HasParentSessionWithToken(token)) {
                isParent = true;
                return WSError::WS_OK;
            }
        }
        isParent = false;
        return WSError::WS_OK;
    }, __func__);
}

WSError SceneSessionManager::UpdateSessionAvoidAreaListener(int32_t persistentId, bool haveListener)
{
    const auto callingPid = IPCSkeleton::GetCallingRealPid();
    auto task = [this, persistentId, haveListener, callingPid, where = __func__]() {
        TLOGNI(WmsLogTag::WMS_IMMS, "%{public}s win %{public}d haveListener %{public}d",
            where, persistentId, haveListener);
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGND(WmsLogTag::WMS_IMMS, "%{public}s sceneSession is nullptr", where);
            return WSError::WS_DO_NOTHING;
        }
        if (callingPid != sceneSession->GetCallingPid()) {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s Permission denied, not called by the same process", where);
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        if (haveListener) {
            avoidAreaListenerSessionSet_.insert(persistentId);
            UpdateAvoidArea(persistentId);
        } else {
            avoidAreaListenerSessionSet_.erase(persistentId);
        }
        return WSError::WS_OK;
    };
    return taskScheduler_->PostSyncTask(task, "UpdateSessionAvoidAreaListener:PID:" + std::to_string(persistentId));
}

WMError SceneSessionManager::UpdateSessionScreenshotAppEventListener(int32_t persistentId, bool haveListener)
{
    return taskScheduler_->PostSyncTask([this, persistentId, haveListener, where = __func__]() {
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s win: %{public}d haveListener: %{public}u",
            where, persistentId, haveListener);
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s win: %{public}d sceneSession is nullptr", where, persistentId);
            return WMError::WM_ERROR_NULLPTR;
        }
        if (haveListener) {
            screenshotAppEventListenerSessionSet_.insert(persistentId);
        } else {
            screenshotAppEventListenerSessionSet_.erase(persistentId);
        }
        return WMError::WM_OK;
    }, __func__);
}

void SceneSessionManager::UpdateAvoidSessionAvoidArea(WindowType type)
{
    AvoidAreaType avoidType = (type == WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT) ?
        AvoidAreaType::TYPE_KEYBOARD : AvoidAreaType::TYPE_SYSTEM;
    AvoidArea avoidArea = rootSceneSession_->GetAvoidAreaByType(avoidType);
    rootSceneSession_->UpdateAvoidArea(new AvoidArea(avoidArea), avoidType);

    for (auto persistentId : avoidAreaListenerSessionSet_) {
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr || !IsSessionVisibleForeground(sceneSession)) {
            continue;
        }
        AvoidArea avoidArea = sceneSession->GetAvoidAreaByType(avoidType);
        sceneSession->UpdateAvoidArea(new AvoidArea(avoidArea), avoidType);
    }
}

void SceneSessionManager::UpdateNormalSessionAvoidArea(
    int32_t persistentId, const sptr<SceneSession>& sceneSession, bool& needUpdate)
{
    if (rootSceneSession_->GetPersistentId() == persistentId) {
        UpdateRootSceneSessionAvoidArea(persistentId, needUpdate);
        return;
    }
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "session is nullptr, win %{public}d", persistentId);
        needUpdate = false;
        return;
    }
    if (!IsSessionVisibleForeground(sceneSession)) {
        TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u isVisible %{public}u sessionState %{public}u",
            persistentId, sceneSession->IsVisible(), sceneSession->GetSessionState());
        needUpdate = false;
        return;
    }
    if (avoidAreaListenerSessionSet_.find(persistentId) == avoidAreaListenerSessionSet_.end()) {
        TLOGD(WmsLogTag::WMS_IMMS,
            "win %{public}d not in avoidAreaListenerNodes, cannot update avoid area", persistentId);
        needUpdate = false;
        return;
    }
    sceneSession->UpdateSizeChangeReason(SizeChangeReason::AVOID_AREA_CHANGE);
    sceneSession->NotifyClientToUpdateRect("AvoidAreaChange", nullptr);
}

void SceneSessionManager::UpdateRootSceneSessionAvoidArea(int32_t persistentId, bool& needUpdate)
{
    using T = std::underlying_type_t<AvoidAreaType>;
    for (T avoidAreaType = static_cast<T>(AvoidAreaType::TYPE_START);
        avoidAreaType < static_cast<T>(AvoidAreaType::TYPE_END); avoidAreaType++) {
        AvoidArea avoidArea = rootSceneSession_->GetAvoidAreaByType(static_cast<AvoidAreaType>(avoidAreaType));
        if (avoidAreaType == static_cast<T>(AvoidAreaType::TYPE_NAVIGATION_INDICATOR) &&
            !CheckAvoidAreaForAINavigationBar(
                isAINavigationBarVisible_[rootSceneSession_->GetSessionProperty()->GetDisplayId()], avoidArea,
                rootSceneSession_->GetSessionRect().height_)) {
            continue;
        }
        rootSceneSession_->UpdateAvoidArea(new AvoidArea(avoidArea), static_cast<AvoidAreaType>(avoidAreaType));
    }
    needUpdate = true;
}

void SceneSessionManager::NotifyMMIWindowPidChange(int32_t windowId, bool startMoving)
{
    int32_t pid = startMoving ? static_cast<int32_t>(getpid()) : -1;
    auto sceneSession = GetSceneSession(windowId);
    if (sceneSession == nullptr) {
        TLOGW(WmsLogTag::WMS_EVENT, "window not exist: %{public}d", windowId);
        return;
    }

    TLOGI(WmsLogTag::WMS_EVENT, "Notify window:%{public}d, pid:%{public}d", windowId, pid);
    taskScheduler_->PostAsyncTask([weakSceneSession = wptr<SceneSession>(sceneSession), startMoving] {
        auto sceneSession = weakSceneSession.promote();
        if (sceneSession == nullptr) {
            TLOGNW(WmsLogTag::WMS_EVENT, "session is null");
            return;
        }
        SceneInputManager::GetInstance().NotifyMMIWindowPidChange(sceneSession, startMoving);
    }, __func__);
}

void SceneSessionManager::UpdateAvoidArea(int32_t persistentId)
{
    taskScheduler_->PostAsyncTask([this, persistentId] {
        bool needUpdate = false;
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession != nullptr && sceneSession->IsImmersiveType()) {
            UpdateAvoidSessionAvoidArea(sceneSession->GetWindowType());
        } else {
            UpdateNormalSessionAvoidArea(persistentId, sceneSession, needUpdate);
        }
        if (needUpdate) {
            NotifyWindowInfoChange(persistentId, WindowUpdateType::WINDOW_UPDATE_BOUNDS);
        }
    }, "UpdateAvoidArea:PID:" + std::to_string(persistentId));
}

void SceneSessionManager::GetKeyboardOccupiedAreaWithRotation(
    int32_t persistentId, Rotation rotation, std::vector<std::pair<bool, WSRect>>& avoidAreas)
{
    if (systemConfig_.IsPcWindow()) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "The PC device is not compatible, id: %{public}d", persistentId);
        return;
    }

    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "sceneSession is nullptr, id: %{public}d", persistentId);
        return;
    }
    auto keyboardSession = GetKeyboardSession(sceneSession->GetSessionProperty()->GetDisplayId(), false);
    if (keyboardSession == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "keyboardSession is nullptr, id: %{public}d", persistentId);
        return;
    }

    std::pair<bool, WSRect> keyboardOccupiedArea = {true, {0, 0, 0, 0}};
    const KeyboardLayoutParams& keyboardLayoutParams = keyboardSession->GetSessionProperty()->GetKeyboardLayoutParams();
    Rect nextRect;
    if (rotation == Rotation::ROTATION_0 || rotation == Rotation::ROTATION_180) {
        nextRect = keyboardLayoutParams.PortraitPanelRect_;
    } else if (rotation == Rotation::ROTATION_90 || rotation == Rotation::ROTATION_270) {
        nextRect = keyboardLayoutParams.LandscapePanelRect_;
    } else {
        TLOGE(WmsLogTag::WMS_KEYBOARD,
            "Rotation is invalid, id: %{public}d, rotation: %{public}u", persistentId, rotation);
        return;
    }
    keyboardOccupiedArea.second = {
        nextRect.posX_, nextRect.posY_, static_cast<int32_t>(nextRect.width_), static_cast<int32_t>(nextRect.height_)
    };

    if (!keyboardSession->IsSessionForeground() ||
        keyboardLayoutParams.gravity_ == WindowGravity::WINDOW_GRAVITY_FLOAT) {
        keyboardOccupiedArea.first = false;
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "next keyboardOccupiedArea: [%{public}d, %{public}s]", keyboardOccupiedArea.first,
        keyboardOccupiedArea.second.ToString().c_str());
    avoidAreas.emplace_back(keyboardOccupiedArea);
}

void SceneSessionManager::UpdateGestureBackEnabled(int32_t curId, int32_t nextId)
{
    auto task = [this, curId, nextId, where = __func__] {
        auto sceneSession = GetSceneSession(curId);
        if (sceneSession == nullptr || !sceneSession->GetEnableGestureBackHadSet()) {
            TLOGND(WmsLogTag::WMS_IMMS, "sceneSession is nullptr or not set Gesture Back enable");
            return;
        }
        auto needEnableGestureBack = sceneSession->GetGestureBackEnabled();
        if (needEnableGestureBack) {
            gestureBackEnableWindowIdSet_.erase(curId);
        } else {
            gestureBackEnableWindowIdSet_.insert(curId);
        }
        if (sceneSession->GetWindowType() != WindowType::WINDOW_TYPE_APP_MAIN_WINDOW ||
            sceneSession->GetWindowMode() != WindowMode::WINDOW_MODE_FULLSCREEN ||
            (sceneSession->GetSessionState() != SessionState::STATE_FOREGROUND &&
             sceneSession->GetSessionState() != SessionState::STATE_ACTIVE) ||
            enterRecent_.load() || (nextId == INVALID_SESSION_ID && !sceneSession->IsFocused())) {
            needEnableGestureBack = true;
        }
        if (nextId != INVALID_SESSION_ID) {
            auto nextSession = GetSceneSession(nextId);
            if (nextSession != nullptr && !sceneSession->IsFocused()) {
                auto type = nextSession->GetSessionProperty()->GetWindowType();
                needEnableGestureBack = type != WindowType::WINDOW_TYPE_PIP ? true : needEnableGestureBack;
            }
        }
        if (gestureNavigationEnabledChangeFunc_ != nullptr) {
            gestureNavigationEnabledChangeFunc_(needEnableGestureBack,
                sceneSession->GetSessionInfo().bundleName_, GestureBackType::GESTURE_SIDE);
        } else {
            TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s callback func is null", where);
        }
    };
    taskScheduler_->PostAsyncTask(task, "UpdateGestureBackEnabled: PID: " + std::to_string(curId));
}

WSError SceneSessionManager::NotifyStatusBarShowStatus(int32_t persistentId, bool isVisible)
{
    TLOGD(WmsLogTag::WMS_IMMS, "win %{public}u isVisible %{public}u",
        persistentId, isVisible);
    taskScheduler_->PostTask([this, persistentId, isVisible] {
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_IMMS, "scene session is nullptr");
            return;
        }
        sceneSession->SetIsStatusBarVisible(isVisible);
    }, __func__);
    return WSError::WS_OK;
}

void SceneSessionManager::UpdateAvoidAreaForLSStateChange(int32_t curState, int32_t preState)
{
    if (curState == preState) {
        return;
    }
    constexpr int32_t nonLSState = 0;
    SetLSState(curState > nonLSState);
    TLOGI(WmsLogTag::WMS_IMMS, "curState %{public}d, preState %{public}d", curState, preState);
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (auto& iter : sceneSessionMap_) {
            auto session = iter.second;
            if (session == nullptr || !IsSessionVisibleForeground(session)) {
                continue;
            }
            session->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_END);
        }
    }
}

void SceneSessionManager::NotifyStatusBarConstantlyShow(DisplayId displayId, bool isVisible)
{
    TLOGD(WmsLogTag::WMS_IMMS, "displayId %{public}" PRIu64 " isVisible %{public}u", displayId, isVisible);
    const char* const where = __func__;
    auto task = [this, displayId, isVisible] {
        statusBarConstantlyShowMap_[displayId] = isVisible;
        UpdateRootSceneAvoidArea();
        return WMError::WM_OK;
    };
    taskScheduler_->PostAsyncTask(task, where);
}

void SceneSessionManager::GetStatusBarConstantlyShow(DisplayId displayId, bool& isVisible) const
{
    auto it = statusBarConstantlyShowMap_.find(displayId);
    if (it != statusBarConstantlyShowMap_.end()) {
        isVisible = it->second;
    } else {
        isVisible = false;
    }
}

WSError SceneSessionManager::NotifyAINavigationBarShowStatus(bool isVisible, WSRect barArea, uint64_t displayId)
{
    TLOGI(WmsLogTag::WMS_IMMS, "isVisible %{public}u "
        "area %{public}s, displayId %{public}" PRIu64, isVisible, barArea.ToString().c_str(), displayId);
    taskScheduler_->PostAsyncTask([this, isVisible, barArea, displayId, where = __func__] {
        bool isNeedUpdate = true;
        {
            std::unique_lock<std::shared_mutex> lock(currAINavigationBarAreaMapMutex_);
            if (isAINavigationBarVisible_.find(displayId) == isAINavigationBarVisible_.end()) {
                isAINavigationBarVisible_[displayId] = false;
            }
            isNeedUpdate = isAINavigationBarVisible_[displayId] != isVisible ||
                           currAINavigationBarAreaMap_.count(displayId) == 0 ||
                           currAINavigationBarAreaMap_[displayId] != barArea;
            if (isNeedUpdate) {
                isAINavigationBarVisible_[displayId] = isVisible;
                currAINavigationBarAreaMap_[displayId] = barArea;
            }
        }
        if (isNeedUpdate) {
            TLOGNI(WmsLogTag::WMS_IMMS, "%{public}s isVisible %{public}u bar area %{public}s",
                where, isVisible, barArea.ToString().c_str());
            for (auto persistentId : avoidAreaListenerSessionSet_) {
                NotifySessionAINavigationBarChange(persistentId);
            }
            rootSceneSession_->UpdateAvoidArea(
                new AvoidArea(rootSceneSession_->GetAvoidAreaByType(AvoidAreaType::TYPE_NAVIGATION_INDICATOR)),
                AvoidAreaType::TYPE_NAVIGATION_INDICATOR);
        }
    }, __func__);
    return WSError::WS_OK;
}

WSError SceneSessionManager::NotifyNextAvoidRectInfo(AvoidAreaType type,
    const WSRect& portraitRect, const WSRect& landspaceRect, DisplayId displayId)
{
    TLOGD(WmsLogTag::WMS_IMMS, "type %{public}d "
        "portraitRect %{public}s, portraitRect %{public}s, displayId %{public}" PRIu64,
        type, portraitRect.ToString().c_str(), landspaceRect.ToString().c_str(), displayId);
    std::lock_guard<std::mutex> lock(nextAvoidRectInfoMapMutex_);
    nextAvoidRectInfoMap_[type][displayId] = { portraitRect, landspaceRect };
    return WSError::WS_OK;
}

WSError SceneSessionManager::GetNextAvoidRectInfo(DisplayId displayId, AvoidAreaType type,
    std::pair<WSRect, WSRect>& nextSystemBarAvoidAreaRectInfo)
{
    {
        std::lock_guard<std::mutex> lock(nextAvoidRectInfoMapMutex_);
        if (nextAvoidRectInfoMap_.count(type) == 0 || nextAvoidRectInfoMap_[type].count(displayId) == 0) {
            TLOGI(WmsLogTag::WMS_IMMS, "get failed, type %{public}d displayId %{public}" PRIu64, type, displayId);
            return WSError::WS_DO_NOTHING;
        }
        nextSystemBarAvoidAreaRectInfo = nextAvoidRectInfoMap_[type][displayId];
    }
    return WSError::WS_OK;
}

void SceneSessionManager::NotifySessionAINavigationBarChange(int32_t persistentId)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr || !IsSessionVisibleForeground(sceneSession)) {
        TLOGD(WmsLogTag::WMS_IMMS, "scene session is nullptr or not visible");
        return;
    }
    sceneSession->HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_NAVIGATION_INDICATOR);
}

WSRect SceneSessionManager::GetAINavigationBarArea(uint64_t displayId, bool ignoreVisibility)
{
    std::shared_lock<std::shared_mutex> lock(currAINavigationBarAreaMapMutex_);
    if (currAINavigationBarAreaMap_.count(displayId) == 0 ||
        isAINavigationBarVisible_.count(displayId) == 0) {
        return {};
    }
    if (!isAINavigationBarVisible_[displayId] && !ignoreVisibility) {
        return {};
    }
    return currAINavigationBarAreaMap_[displayId];
}

WSError SceneSessionManager::UpdateSessionTouchOutsideListener(int32_t& persistentId, bool haveListener)
{
    const auto callingPid = IPCSkeleton::GetCallingRealPid();
    auto task = [this, persistentId, haveListener, callingPid]() {
        TLOGNI(WmsLogTag::WMS_EVENT, "persistentId:%{public}d haveListener:%{public}d",
            persistentId, haveListener);
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_EVENT, "sceneSession is nullptr.");
            return WSError::WS_DO_NOTHING;
        }
        if (callingPid != sceneSession->GetCallingPid()) {
            TLOGNE(WmsLogTag::WMS_EVENT, "Permission denied");
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        if (haveListener) {
            touchOutsideListenerSessionSet_.insert(persistentId);
        } else {
            touchOutsideListenerSessionSet_.erase(persistentId);
        }
        return WSError::WS_OK;
    };
    return taskScheduler_->PostSyncTask(task, "UpdateSessionTouchOutsideListener" + std::to_string(persistentId));
}

WSError SceneSessionManager::UpdateSessionWindowVisibilityListener(int32_t persistentId, bool haveListener)
{
    const auto callingPid = IPCSkeleton::GetCallingRealPid();
    return taskScheduler_->PostSyncTask([this, persistentId, haveListener, callingPid]() -> WSError {
        TLOGNI(WmsLogTag::WMS_LIFE, "persistentId: %{public}d haveListener:%{public}d",
            persistentId, haveListener);
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGND(WmsLogTag::WMS_LIFE, "sceneSession is nullptr.");
            return haveListener ? WSError::WS_DO_NOTHING : WSError::WS_OK;
        }
        if (callingPid != sceneSession->GetCallingPid()) {
            TLOGNE(WmsLogTag::WMS_LIFE, "Permission denied, neither register nor unreigster allowed by other process");
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        if (haveListener) {
            windowVisibilityListenerSessionSet_.insert(persistentId);
            sceneSession->NotifyWindowVisibility();
        } else {
            windowVisibilityListenerSessionSet_.erase(persistentId);
        }
        return WSError::WS_OK;
    }, __func__);
}

WMError SceneSessionManager::UpdateSessionOcclusionStateListener(int32_t persistentId, bool haveListener)
{
    const char* const where = __func__;
    return taskScheduler_->PostSyncTask([this, persistentId, haveListener, where]() -> WMError {
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: no session for winId=%{public}d, haveListener=%{public}d",
                where, persistentId, haveListener);
            return WMError::WM_DO_NOTHING;
        }
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: win=[%{public}d, %{public}s], haveListener=%{public}d",
            where, persistentId, sceneSession->GetWindowName().c_str(), haveListener);
        if (haveListener) {
            occlusionStateListenerSessionSet_.insert(persistentId);
            sceneSession->NotifyOcclusionState();
        } else {
            occlusionStateListenerSessionSet_.erase(persistentId);
        }
        return WMError::WM_OK;
    }, where);
}

void SceneSessionManager::UpdateDarkColorModeToRS()
{
    std::shared_ptr<AbilityRuntime::ApplicationContext> appContext = AbilityRuntime::Context::GetApplicationContext();
    if (appContext == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "app context is nullptr");
        return;
    }
    std::shared_ptr<AppExecFwk::Configuration> config = appContext->GetConfiguration();
    if (config == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "app configuration is nullptr");
        return;
    }
    std::string colorMode = config->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    bool isDark = (colorMode == AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
    bool ret = RSInterfaces::GetInstance().SetGlobalDarkColorMode(isDark);
    TLOGI(WmsLogTag::DEFAULT, "colorMode: %{public}s, ret: %{public}d", colorMode.c_str(), ret);
}

void SceneSessionManager::SetVirtualPixelRatioChangeListener(const ProcessVirtualPixelRatioChangeFunc& func)
{
    processVirtualPixelRatioChangeFunc_ = func;
    TLOGI(WmsLogTag::DEFAULT, "end");
}

void SceneSessionManager::ProcessVirtualPixelRatioChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type)
{
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "displayInfo is nullptr.");
        return;
    }
    taskScheduler_->PostSyncTask([this, displayInfo, type, where = __func__]() {
        if (processVirtualPixelRatioChangeFunc_ != nullptr &&
            type == DisplayStateChangeType::RESOLUTION_CHANGE &&
            displayInfo->GetVirtualPixelRatio() == displayInfo->GetDensityInCurResolution()) {
            Rect rect = { displayInfo->GetOffsetX(), displayInfo->GetOffsetY(),
                          displayInfo->GetWidth(), displayInfo->GetHeight() };
            processVirtualPixelRatioChangeFunc_(displayInfo->GetVirtualPixelRatio(), rect);
        }
        if (onVirtualPixelChangeCallback_ != nullptr && type == DisplayStateChangeType::VIRTUAL_PIXEL_RATIO_CHANGE) {
            onVirtualPixelChangeCallback_(displayInfo->GetVirtualPixelRatio() * DOT_PER_INCH, displayInfo->GetScreenId());
        }
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s null scene session", where);
                continue;
            }
            if (sceneSession->GetSessionProperty()->GetDisplayId() != displayInfo->GetDisplayId()) {
                continue;
            }
            if (sceneSession->GetSessionInfo().isSystem_) {
                continue;
            }
            if (sceneSession->GetSessionState() == SessionState::STATE_FOREGROUND ||
                sceneSession->GetSessionState() == SessionState::STATE_ACTIVE) {
                sceneSession->UpdateDensity();
                TLOGND(WmsLogTag::WMS_ATTRIBUTE, "UpdateDensity name=%{public}s, persistentId=%{public}d, "
                    "winType=%{public}d, state=%{public}d, visible-%{public}d", sceneSession->GetWindowName().c_str(),
                    sceneSession->GetPersistentId(), sceneSession->GetWindowType(),
                    sceneSession->GetSessionState(), sceneSession->IsVisible());
            } else if (sceneSession->GetSessionState() == SessionState::STATE_BACKGROUND && systemConfig_.IsPcWindow()) {
                sceneSession->SaveLastDensity();
                TLOGND(WmsLogTag::WMS_ATTRIBUTE, "SaveLastDensity name=%{public}s, persistentId=%{public}d, "
                    "winType=%{public}d, state=%{public}d, visible-%{public}d", sceneSession->GetWindowName().c_str(),
                    sceneSession->GetPersistentId(), sceneSession->GetWindowType(),
                    sceneSession->GetSessionState(), sceneSession->IsVisible());
            }
        }
        UpdateDisplayRegion(displayInfo);
        return WSError::WS_OK;
    }, "ProcessVirtualPixelRatioChange:DID:" + std::to_string(defaultDisplayId));
}

void SceneSessionManager::ProcessUpdateRotationChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type)
{
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayInfo is nullptr.");
        return;
    }
    taskScheduler_->PostSyncTask([this, displayInfo, where = __func__]() {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                TLOGNE(WmsLogTag::DMS, "%{public}s null scene session", where);
                continue;
            }
            if (sceneSession->GetSessionState() != SessionState::STATE_FOREGROUND &&
                sceneSession->GetSessionState() != SessionState::STATE_ACTIVE) {
                continue;
            }
            if (NearEqual(sceneSession->GetBounds().width_, static_cast<float>(displayInfo->GetWidth())) &&
                NearEqual(sceneSession->GetBounds().height_, static_cast<float>(displayInfo->GetHeight())) &&
                sceneSession->GetRotation() != displayInfo->GetRotation()) {
                sceneSession->UpdateRotationAvoidArea();
                TLOGND(WmsLogTag::DMS, "UpdateRotationAvoidArea name=%{public}s, persistentId=%{public}d, "
                    "winType=%{public}d, state=%{public}d, visible-%{public}d", sceneSession->GetWindowName().c_str(),
                    sceneSession->GetPersistentId(), sceneSession->GetWindowType(), sceneSession->GetSessionState(),
                    sceneSession->IsVisible());
            }
            sceneSession->SetRotation(displayInfo->GetRotation());
            sceneSession->UpdateOrientation();
        }
        WSSnapshotHelper::GetInstance()->SetWindowOrientationStatus(displayInfo->GetRotation());
        UpdateDisplayRegion(displayInfo);
        return WSError::WS_OK;
    }, "ProcessUpdateRotationChange" + std::to_string(defaultDisplayId));
}

void SceneSessionManager::ProcessDisplayScale(sptr<DisplayInfo>& displayInfo)
{
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::DMS, "displayInfo is nullptr");
        return;
    }

    taskScheduler_->PostAsyncTask([this, displayInfo] {
        ScreenSessionManagerClient::GetInstance().UpdateDisplayScale(displayInfo->GetScreenId(),
            displayInfo->GetScaleX(),
            displayInfo->GetScaleY(),
            displayInfo->GetPivotX(),
            displayInfo->GetPivotY(),
            displayInfo->GetTranslateX(),
            displayInfo->GetTranslateY());
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSessionManager::FlushWindowInfoToMMI");
        FlushWindowInfoToMMI(true);
    }, __func__);
}

void DisplayChangeListener::OnDisplayStateChange(DisplayId defaultDisplayId, sptr<DisplayInfo> displayInfo,
    const std::map<DisplayId, sptr<DisplayInfo>>& displayInfoMap, DisplayStateChangeType type)
{
    TLOGD(WmsLogTag::DEFAULT, "type: %{public}u", type);
    switch (type) {
        case DisplayStateChangeType::RESOLUTION_CHANGE:
        case DisplayStateChangeType::VIRTUAL_PIXEL_RATIO_CHANGE: {
            SceneSessionManager::GetInstance().ProcessVirtualPixelRatioChange(defaultDisplayId,
                displayInfo, displayInfoMap, type);
            SceneSessionManager::GetInstance().FlushWindowInfoToMMI();
            break;
        }
        case DisplayStateChangeType::UPDATE_ROTATION: {
            SceneSessionManager::GetInstance().ProcessUpdateRotationChange(defaultDisplayId,
                displayInfo, displayInfoMap, type);
            break;
        }
        case DisplayStateChangeType::UPDATE_SCALE: {
            SceneSessionManager::GetInstance().ProcessDisplayScale(displayInfo);
            break;
        }
        default:
            return;
    }
}

void ScreenConnectionChangeListener::OnScreenConnected(const sptr<ScreenSession>& screenSession)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "screenSession is nullptr");
        return;
    }
    TLOGI(WmsLogTag::WMS_FOCUS,
          "name: %{public}s, screenId: %{public}" PRIu64 ", displayGroupId: %{public}" PRIu64
          " ,screenType: %{public}u",
          screenSession->GetName().c_str(),
          screenSession->GetScreenId(),
          screenSession->GetDisplayGroupId(),
          screenSession->GetScreenProperty().GetScreenType());
    SceneSessionManager::GetInstance().AddFocusGroup(screenSession->GetDisplayGroupId(), screenSession->GetScreenId());

    // Window Layout Global Coordinate System: monitors screen position changes in the global coordinate system
    screenSession->RegisterScreenChangeListener(&SceneScreenChangeListener::GetInstance());
}

void ScreenConnectionChangeListener::OnScreenDisconnected(const sptr<ScreenSession>& screenSession)
{
    if (screenSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "screenSession is nullptr");
        return;
    }
    TLOGI(WmsLogTag::WMS_FOCUS,
          "name: %{public}s, screenId: %{public}" PRIu64 ", displayGroupId: %{public}" PRIu64
          " ,screenType: %{public}u",
          screenSession->GetName().c_str(),
          screenSession->GetScreenId(),
          screenSession->GetDisplayGroupId(),
          screenSession->GetScreenProperty().GetScreenType());
    SceneSessionManager::GetInstance().RemoveFocusGroup(screenSession->GetDisplayGroupId(),
        screenSession->GetScreenId());

    // Window Layout Global Coordinate System
    screenSession->UnregisterScreenChangeListener(&SceneScreenChangeListener::GetInstance());
}

void DisplayChangeListener::OnScreenshot(DisplayId displayId)
{
    SceneSessionManager::GetInstance().OnScreenshot(displayId);
}

void SceneSessionManager::OnScreenshot(DisplayId displayId)
{
    taskScheduler_->PostAsyncTask([this, displayId]() {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                continue;
            }
            auto state = sceneSession->GetSessionState();
            if (state == SessionState::STATE_FOREGROUND || state == SessionState::STATE_ACTIVE) {
                sceneSession->NotifyScreenshot();
            }
        }
    }, "OnScreenshot:PID:" + std::to_string(displayId));
}

WMError SceneSessionManager::NotifyScreenshotEvent(ScreenshotEventType type)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "event: %{public}d", type);
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    taskScheduler_->PostAsyncTask([this, type, where = __func__] {
        for (auto persistentId : screenshotAppEventListenerSessionSet_) {
            auto sceneSession = GetSceneSession(persistentId);
            if (sceneSession == nullptr) {
                TLOGNW(WmsLogTag::WMS_ATTRIBUTE, "%{public}s win: %{public}d session is null", where, persistentId);
                continue;
            }
            auto state = sceneSession->GetSessionState();
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s win: %{public}d, state: %{public}u, event: %{public}d",
                where, sceneSession->GetPersistentId(), state, type);
            if (state == SessionState::STATE_FOREGROUND || state == SessionState::STATE_ACTIVE) {
                sceneSession->NotifyScreenshotAppEvent(type);
            }
        }
    }, __func__);
    return WMError::WM_OK;
}

WSError SceneSessionManager::ClearSession(int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_LIFE, "id: %{public}d", persistentId);
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is not system-app, can not use system-api");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    taskScheduler_->PostAsyncTask([this, persistentId]() {
        auto sceneSession = GetSceneSession(persistentId);
        return ClearSession(sceneSession);
    }, "ClearSession:PID:" + std::to_string(persistentId));
    return WSError::WS_OK;
}

WSError SceneSessionManager::ClearSession(sptr<SceneSession> sceneSession)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr");
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (!IsSessionClearable(sceneSession)) {
        TLOGE(WmsLogTag::WMS_LIFE, "session cannot be clear, Id %{public}d.", sceneSession->GetPersistentId());
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    const WSError errCode = sceneSession->Clear(false, true);
    return errCode;
}

WSError SceneSessionManager::ClearAllSessions()
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is not system-app, can not use system-api");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    auto task = [this]() {
        std::vector<sptr<SceneSession>> sessionVector;
        GetAllClearableSessions(sessionVector);
        for (uint32_t i = 0; i < sessionVector.size(); i++) {
            ClearSession(sessionVector[i]);
        }
        return WSError::WS_OK;
    };
    taskScheduler_->PostAsyncTask(task, __func__);
    return WSError::WS_OK;
}

void SceneSessionManager::GetAllClearableSessions(std::vector<sptr<SceneSession>>& sessionVector)
{
    TLOGD(WmsLogTag::WMS_LIFE, "in");
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (IsSessionClearable(sceneSession)) {
            sessionVector.push_back(sceneSession);
        }
    }
}

WSError SceneSessionManager::LockSession(int32_t sessionId)
{
    TLOGI(WmsLogTag::DEFAULT, "id: %{public}d", sessionId);
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::DEFAULT, "The caller is not system-app, can not use system-api");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    const char* const where = __func__;
    auto task = [this, sessionId, where]() {
        auto sceneSession = GetSceneSession(sessionId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "%{public}s cannot find session, id: %{public}d", where, sessionId);
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        sceneSession->SetSessionInfoLockedState(true);
        return WSError::WS_OK;
    };
    return taskScheduler_->PostSyncTask(task, "LockSession:SID:" + std::to_string(sessionId));
}

WSError SceneSessionManager::UnlockSession(int32_t sessionId)
{
    TLOGI(WmsLogTag::DEFAULT, "id: %{public}d", sessionId);
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::DEFAULT, "The caller is not system-app, can not use system-api");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    const char* const where = __func__;
    auto task = [this, sessionId, where]() {
        auto sceneSession = GetSceneSession(sessionId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::DEFAULT, "%{public}s cannot find session, id: %{public}d", where, sessionId);
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        sceneSession->SetSessionInfoLockedState(false);
        return WSError::WS_OK;
    };
    return taskScheduler_->PostSyncTask(task, "UnlockSession" + std::to_string(sessionId));
}

WSError SceneSessionManager::MoveSessionsToForeground(const std::vector<int32_t>& sessionIds, int32_t topSessionId)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is not system-app, can not use system-api");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!SessionPermission::VerifySessionPermission()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }

    return WSError::WS_OK;
}

WSError SceneSessionManager::MoveSessionsToBackground(const std::vector<int32_t>& sessionIds,
    std::vector<int32_t>& result)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    if (!SessionPermission::JudgeCallerIsAllowedToUseSystemAPI()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is not system-app, can not use system-api");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!SessionPermission::VerifySessionPermission()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }

    result.insert(result.end(), sessionIds.begin(), sessionIds.end());
    return WSError::WS_OK;
}

bool SceneSessionManager::IsSessionClearable(sptr<SceneSession> sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGI(WmsLogTag::WMS_MAIN, "sceneSession is nullptr");
        return false;
    }
    const auto& sessionInfo = sceneSession->GetSessionInfo();
    if (sessionInfo.abilityInfo == nullptr) {
        TLOGI(WmsLogTag::WMS_MAIN, "sceneSession abilityInfo is nullptr");
        return false;
    }
    if (sessionInfo.abilityInfo->excludeFromMissions && !sceneSession->IsAnco()) {
        TLOGI(WmsLogTag::WMS_MAIN, "persistentId %{public}d is excludeFromMissions", sceneSession->GetPersistentId());
        return false;
    }
    if (sessionInfo.abilityInfo->unclearableMission) {
        TLOGI(WmsLogTag::WMS_MAIN, "persistentId %{public}d is unclearable", sceneSession->GetPersistentId());
        return false;
    }
    if (sessionInfo.isSystem_) {
        TLOGI(WmsLogTag::WMS_MAIN, "persistentId %{public}d is system app", sceneSession->GetPersistentId());
        return false;
    }
    if (sessionInfo.lockedState) {
        TLOGI(WmsLogTag::WMS_MAIN, "persistentId %{public}d is in lockedState", sceneSession->GetPersistentId());
        return false;
    }

    return true;
}

WSError SceneSessionManager::RegisterIAbilityManagerCollaborator(int32_t type,
    const sptr<AAFwk::IAbilityManagerCollaborator>& impl)
{
    TLOGI(WmsLogTag::DEFAULT, "type: %{public}d", type);
    auto isSaCall = SessionPermission::IsSACalling();
    if (!isSaCall || !SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::DEFAULT, "The caller has not permission granted or not SACalling");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    if (!CheckCollaboratorType(type)) {
        TLOGW(WmsLogTag::DEFAULT, "collaborator register failed, invalid type.");
        return WSError::WS_ERROR_INVALID_TYPE;
    }
    if (impl == nullptr || impl->AsObject() == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "Collaborator is nullptr");
        return WSError::WS_ERROR_NULLPTR;
    }
    sptr<AgentDeathRecipient> collaboratorDeathRecipient = sptr<AgentDeathRecipient>::MakeSptr(
        [this, type](const sptr<IRemoteObject>& remoteObject) { this->ClearCollaboratorSessionsByType(type); });
    if (!impl->AsObject()->AddDeathRecipient(collaboratorDeathRecipient)) {
        TLOGE(WmsLogTag::DEFAULT, "Failed to add collaborator death recipient");
        return WSError::WS_ERROR_IPC_FAILED;
    }
    {
        std::unique_lock<std::shared_mutex> lock(collaboratorMapLock_);
        collaboratorMap_[type] = impl;
    }
    {
        std::unique_lock<std::shared_mutex> lock(collaboratorDeathRecipientMapLock_);
        collaboratorDeathRecipientMap_[type] = collaboratorDeathRecipient;
    }
    auto task = [this] {
        if (abilityManagerCollaboratorRegisteredFunc_) {
            abilityManagerCollaboratorRegisteredFunc_();
        }
    };
    taskScheduler_->PostTask(task, __func__);
    return WSError::WS_OK;
}

WSError SceneSessionManager::UnregisterIAbilityManagerCollaborator(int32_t type)
{
    TLOGI(WmsLogTag::DEFAULT, "type: %{public}d", type);
    auto isSaCall = SessionPermission::IsSACalling();
    if (!isSaCall || !SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::DEFAULT, "The caller has not permission granted or not SACalling");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    if (!CheckCollaboratorType(type)) {
        TLOGE(WmsLogTag::DEFAULT, "collaborator unregister failed, invalid type.");
        return WSError::WS_ERROR_INVALID_TYPE;
    }
    sptr<AAFwk::IAbilityManagerCollaborator> collaborator = GetCollaboratorByType(type);
    sptr<AgentDeathRecipient> collaboratorDeathRecipient = GetCollaboratorDeathRecipient(type);
    if (collaborator != nullptr && collaborator->AsObject() != nullptr && collaboratorDeathRecipient != nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "Remove collaborator death recipient");
        collaborator->AsObject()->RemoveDeathRecipient(collaboratorDeathRecipient);
    }
    {
        std::unique_lock<std::shared_mutex> lock(collaboratorMapLock_);
        collaboratorMap_.erase(type);
    }
    {
        std::unique_lock<std::shared_mutex> lock(collaboratorDeathRecipientMapLock_);
        collaboratorDeathRecipientMap_.erase(type);
    }
    return WSError::WS_OK;
}

void SceneSessionManager::ClearCollaboratorSessionsByType(int32_t type)
{
    TLOGI(WmsLogTag::WMS_MAIN, "type: %{public}d", type);
    std::vector<sptr<SceneSession>> collaboratorSessions;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession != nullptr && type == sceneSession->GetCollaboratorType()) {
                collaboratorSessions.push_back(sceneSession);
            }
        }
    }
    for (const auto& sceneSession : collaboratorSessions) {
        TLOGI(WmsLogTag::WMS_MAIN, "clear session id: %{public}d", sceneSession->GetPersistentId());
        sceneSession->Clear();
    }
}

bool SceneSessionManager::CheckCollaboratorType(int32_t type)
{
    if (type != CollaboratorType::RESERVE_TYPE && type != CollaboratorType::OTHERS_TYPE &&
        type != CollaboratorType::REDIRECT_TYPE) {
        TLOGD(WmsLogTag::WMS_MAIN, "type is invalid");
        return false;
    }
    return true;
}

BrokerStates SceneSessionManager::CheckIfReuseSession(SessionInfo& sessionInfo)
{
    auto abilityInfo = QueryAbilityInfoFromBMS(currentUserId_, sessionInfo.bundleName_, sessionInfo.abilityName_,
        sessionInfo.moduleName_);
    if (abilityInfo == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "abilityInfo is nullptr!");
        return BrokerStates::BROKER_UNKOWN;
    }
    sessionInfo.abilityInfo = abilityInfo;
    int32_t collaboratorType = CollaboratorType::DEFAULT_TYPE;
    if (abilityInfo->applicationInfo.codePath == std::to_string(CollaboratorType::RESERVE_TYPE)) {
        collaboratorType = CollaboratorType::RESERVE_TYPE;
    } else if (abilityInfo->applicationInfo.codePath == std::to_string(CollaboratorType::OTHERS_TYPE)) {
        collaboratorType = CollaboratorType::OTHERS_TYPE;
    } else if (abilityInfo->applicationInfo.codePath == std::to_string(CollaboratorType::REDIRECT_TYPE)) {
        collaboratorType = CollaboratorType::REDIRECT_TYPE;
    }
    if (!CheckCollaboratorType(collaboratorType)) {
        TLOGW(WmsLogTag::DEFAULT, "checked not collaborator!");
        return BrokerStates::BROKER_UNKOWN;
    }
    BrokerStates resultValue = NotifyStartAbility(collaboratorType, sessionInfo);
    sessionInfo.collaboratorType_ = collaboratorType;
    sessionInfo.sessionAffinity = sessionInfo.want->GetStringParam(Rosen::PARAM_KEY::PARAM_MISSION_AFFINITY_KEY);
    if (FindSessionByAffinity(sessionInfo.sessionAffinity) != nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "FindSessionByAffinity: %{public}s, try to reuse", sessionInfo.sessionAffinity.c_str());
        sessionInfo.reuse = true;
    } else {
        sessionInfo.reuse = false;
    }
    TLOGI(WmsLogTag::DEFAULT, "end: affinity %{public}s type %{public}d reuse %{public}d",
        sessionInfo.sessionAffinity.c_str(), collaboratorType, sessionInfo.reuse);
    return resultValue;
}

BrokerStates SceneSessionManager::NotifyStartAbility(
    int32_t collaboratorType, const SessionInfo& sessionInfo, int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_LIFE, "type %{public}d id %{public}d windowMode %{public}d",
        collaboratorType, persistentId, sessionInfo.windowMode);
    sptr<AAFwk::IAbilityManagerCollaborator> collaborator = GetCollaboratorByType(collaboratorType);
    if (collaborator == nullptr) {
        return BrokerStates::BROKER_UNKOWN;
    }
    if (sessionInfo.want == nullptr) {
        TLOGI(WmsLogTag::WMS_LIFE, "sessionInfo.want is nullptr, init");
        sessionInfo.want = std::make_shared<AAFwk::Want>();
        sessionInfo.want->SetElementName("", sessionInfo.bundleName_, sessionInfo.abilityName_,
            sessionInfo.moduleName_);
    }
    auto accessTokenIDEx = sessionInfo.callingTokenId_;
    if (collaborator != nullptr) {
        containerStartAbilityTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        std::string affinity = sessionInfo.want->GetStringParam(Rosen::PARAM_KEY::PARAM_MISSION_AFFINITY_KEY);
        if (!affinity.empty() && FindSessionByAffinity(affinity) != nullptr) {
            TLOGI(WmsLogTag::WMS_LIFE, "want affinity exit %{public}s.", affinity.c_str());
            return BrokerStates::BROKER_UNKOWN;
        }
        sessionInfo.want->SetParam("oh_persistentId", persistentId);
        std::shared_ptr<int32_t> ret = std::make_shared<int32_t>(0);
        std::shared_ptr<AAFwk::Want> notifyWant = std::make_shared<AAFwk::Want>(sessionInfo.GetWantSafely());
        bool isTimeout = ffrtQueueHelper_->SubmitTaskAndWait([this, collaborator, accessTokenIDEx,
            notifyWant, abilityInfo = sessionInfo.abilityInfo, windowMode = sessionInfo.windowMode, ret] {
            int timerId = HiviewDFX::XCollie::GetInstance().SetTimer("WMS:SSM:NotifyStartAbility",
                NOTIFY_START_ABILITY_TIMEOUT/1000, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
            auto result = collaborator->NotifyStartAbility(*abilityInfo, currentUserId_, *notifyWant,
                static_cast<uint64_t>(accessTokenIDEx), windowMode);
            HiviewDFX::XCollie::GetInstance().CancelTimer(timerId);
            *ret = static_cast<int32_t>(result);
        }, NOTIFY_START_ABILITY_TIMEOUT);

        if (isTimeout) {
            TLOGE(WmsLogTag::WMS_LIFE, "notify start ability timeout, id: %{public}d", persistentId);
            return BrokerStates::BROKER_NOT_START;
        }
        sessionInfo.SetWantSafely(*notifyWant);
        TLOGI(WmsLogTag::WMS_LIFE, "collaborator ret: %{public}d", *ret);
        if (*ret == 0) {
            return BrokerStates::BROKER_STARTED;
        } else {
            return BrokerStates::BROKER_NOT_START;
        }
    }
    return BrokerStates::BROKER_UNKOWN;
}

void SceneSessionManager::NotifySessionCreate(sptr<SceneSession> sceneSession, const SessionInfo& sessionInfo)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "sceneSession is nullptr");
        return;
    }
    if (sessionInfo.want == nullptr) {
        TLOGI(WmsLogTag::DEFAULT, "sessionInfo.want is nullptr");
        return;
    }
    if (auto collaborator = GetCollaboratorByType(sceneSession->GetCollaboratorType())) {
        auto abilitySessionInfo = SetAbilitySessionInfo(sceneSession);
        abilitySessionInfo->want = sessionInfo.GetWantSafely();
        int32_t missionId = abilitySessionInfo->persistentId;
        std::string bundleName = sessionInfo.bundleName_;
        int64_t timestamp = containerStartAbilityTime_;
        WindowInfoReporter::GetInstance().ReportContainerStartBegin(missionId, bundleName, timestamp);
        TLOGI(WmsLogTag::DEFAULT, "call NotifyMissionCreated, persistentId: %{public}d, bundleName: %{public}s",
            missionId, bundleName.c_str());
        collaborator->NotifyMissionCreated(abilitySessionInfo);
    }
}

void SceneSessionManager::NotifyLoadAbility(int32_t collaboratorType,
    sptr<AAFwk::SessionInfo> abilitySessionInfo, std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo)
{
    TLOGD(WmsLogTag::DEFAULT, "type: %{public}d", collaboratorType);
    if (auto collaborator = GetCollaboratorByType(collaboratorType)) {
        TLOGI(WmsLogTag::DEFAULT, "called");
        collaborator->NotifyLoadAbility(*abilityInfo, abilitySessionInfo);
    }
}


void SceneSessionManager::NotifyUpdateSessionInfo(sptr<SceneSession> sceneSession)
{
    TLOGD(WmsLogTag::DEFAULT, "in");
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "sceneSession is nullptr");
        return;
    }
    if (auto collaborator = GetCollaboratorByType(sceneSession->GetCollaboratorType())) {
        TLOGI(WmsLogTag::DEFAULT, "called UpdateMissionInfo");
        auto abilitySessionInfo = SetAbilitySessionInfo(sceneSession);
        collaborator->UpdateMissionInfo(abilitySessionInfo);
    }
}

void SceneSessionManager::NotifyMoveSessionToForeground(int32_t collaboratorType, int32_t persistentId)
{
    TLOGD(WmsLogTag::DEFAULT, "id: %{public}d, type: %{public}d", persistentId, collaboratorType);
    if (auto collaborator = GetCollaboratorByType(collaboratorType)) {
        TLOGI(WmsLogTag::DEFAULT, "called %{public}d", persistentId);
        collaborator->NotifyMoveMissionToForeground(persistentId, currentUserId_);
    }
}

void SceneSessionManager::NotifyClearSession(int32_t collaboratorType, int32_t persistentId)
{
    TLOGD(WmsLogTag::WMS_LIFE, "id: %{public}d, type: %{public}d", persistentId, collaboratorType);
    if (auto collaborator = GetCollaboratorByType(collaboratorType)) {
        const char* const where = __func__;
        ffrtQueueHelper_->SubmitTask([userId = currentUserId_.load(), collaborator, persistentId, where] {
            int timerId = HiviewDFX::XCollie::GetInstance().SetTimer("WMS:SSM:NotifyClearMission",
                NOTIFY_START_ABILITY_TIMEOUT/1000, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
            int32_t ret = collaborator->NotifyClearMission(persistentId, userId);
            HiviewDFX::XCollie::GetInstance().CancelTimer(timerId);
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s called clear mission ret: %{public}d, persistent id: %{public}d",
                where, ret, persistentId);
        });
    }
}

bool SceneSessionManager::PreHandleCollaboratorStartAbility(sptr<SceneSession>& sceneSession, int32_t persistentId)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "sceneSession is null");
        return false;
    }
    std::string sessionAffinity;
    TLOGI(WmsLogTag::WMS_LIFE, "call");
    if (sceneSession->GetSessionInfo().want != nullptr) {
        sessionAffinity = sceneSession->GetSessionInfo().want
            ->GetStringParam(Rosen::PARAM_KEY::PARAM_MISSION_AFFINITY_KEY);
    }
    if (sessionAffinity.empty()) {
        TLOGI(WmsLogTag::WMS_LIFE, "Session affinity is empty");
        BrokerStates notifyReturn = NotifyStartAbility(
            sceneSession->GetCollaboratorType(), sceneSession->GetSessionInfo(), persistentId);
        if (notifyReturn == BrokerStates::BROKER_NOT_START) {
            TLOGE(WmsLogTag::WMS_LIFE, "notifyReturn not BROKER_STARTED!");
            return false;
        }
    }
    if (sceneSession->GetSessionInfo().want != nullptr) {
        sceneSession->SetSessionInfoAffinity(sceneSession->GetSessionInfo().want
            ->GetStringParam(Rosen::PARAM_KEY::PARAM_MISSION_AFFINITY_KEY));
        TLOGI(WmsLogTag::WMS_LIFE, "ANCO_SESSION_ID: %{public}d, want affinity: %{public}s.",
            sceneSession->GetSessionInfo().want->GetIntParam(AncoConsts::ANCO_SESSION_ID, 0),
            sceneSession->GetSessionInfo().sessionAffinity.c_str());
    } else {
        TLOGI(WmsLogTag::WMS_LIFE, "sceneSession->GetSessionInfo().want is nullptr");
    }
    return true;
}

bool SceneSessionManager::PreHandleCollaborator(sptr<SceneSession>& sceneSession, int32_t persistentId)
{
    if (!PreHandleCollaboratorStartAbility(sceneSession, persistentId)) {
        return false;
    }
    NotifySessionCreate(sceneSession, sceneSession->GetSessionInfo());
    sceneSession->SetSessionInfoAncoSceneState(AncoSceneState::NOTIFY_CREATE);
    return true;
}

void SceneSessionManager::AddWindowDragHotArea(DisplayId displayId, uint32_t type, WSRect& area)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "displayId: %{public}" PRIu64 ", "
        "type: %{public}d, posX: %{public}d, posY: %{public}d, width: %{public}d, "
        "height: %{public}d", displayId, type, area.posX_, area.posY_, area.width_, area.height_);
    SceneSession::AddOrUpdateWindowDragHotArea(displayId, type, area);
}

WSError SceneSessionManager::UpdateMaximizeMode(int32_t persistentId, bool isMaximize)
{
    auto task = [this, persistentId, isMaximize]() {
        TLOGND(WmsLogTag::WMS_PC, "update maximize mode, id: %{public}d, isMaximize: %{public}d",
            persistentId, isMaximize);
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_PC, "could not find window, persistentId:%{public}d", persistentId);
            return WSError::WS_ERROR_INVALID_WINDOW;
        }
        sceneSession->UpdateMaximizeMode(isMaximize);
        return WSError::WS_OK;
    };
    taskScheduler_->PostAsyncTask(task, "UpdateMaximizeMode:PID:" + std::to_string(persistentId));
    return WSError::WS_OK;
}

WSError SceneSessionManager::GetIsLayoutFullScreen(bool& isLayoutFullScreen)
{
    auto task = [this, &isLayoutFullScreen]() {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                TLOGNE(WmsLogTag::WMS_IMMS, "Session is nullptr");
                continue;
            }
            if (!WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
                continue;
            }
            auto state = sceneSession->GetSessionState();
            if (state != SessionState::STATE_FOREGROUND && state != SessionState::STATE_ACTIVE) {
                continue;
            }
            if (sceneSession->GetWindowMode() != WindowMode::WINDOW_MODE_FULLSCREEN) {
                continue;
            }
            isLayoutFullScreen = sceneSession->GetSessionProperty()->IsLayoutFullScreen();
            if (isLayoutFullScreen) {
                TLOGND(WmsLogTag::WMS_IMMS, "Current window is immersive, persistentId:%{public}d",
                    sceneSession->GetPersistentId());
                return WSError::WS_OK;
            } else {
                TLOGND(WmsLogTag::WMS_IMMS, "Current window is not immersive, persistentId:%{public}d",
                    sceneSession->GetPersistentId());
            }
        }
        TLOGND(WmsLogTag::WMS_IMMS, "No immersive window");
        return WSError::WS_OK;
    };

    taskScheduler_->PostSyncTask(task, "GetIsLayoutFullScreen");
    return WSError::WS_OK;
}

WSError SceneSessionManager::SetMaximizeFullScreen(int32_t persistentId, bool isMaximizeFullScreen)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (!sceneSession) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "session is nullptr");
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
 
    if (!WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "only main window support setMaximizeFullScreen.");
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    return sceneSession->SetIsShowDecorInFreeMultiWindow(!isMaximizeFullScreen);
}

WSError SceneSessionManager::UpdateSessionDisplayId(int32_t persistentId, uint64_t screenId)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (!sceneSession) {
        TLOGE(WmsLogTag::DEFAULT, "session is nullptr");
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    auto fromScreenId = sceneSession->GetSessionInfo().screenId_;
    sceneSession->SetScreenId(screenId);
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "wid: %{public}d, move display %{public}" PRIu64 " from %{public}" PRIu64,
        sceneSession->GetPersistentId(), screenId, fromScreenId);
    NotifySessionUpdate(sceneSession->GetSessionInfo(), ActionType::MOVE_DISPLAY, fromScreenId);
    sceneSession->NotifyDisplayMove(fromScreenId, screenId);
    sceneSession->UpdateDensity();
    if (fromScreenId != screenId) {
        sceneSession->AddPropertyDirtyFlags(static_cast<uint32_t>(SessionPropertyFlag::DISPLAY_ID));
        sceneSession->MarkAvoidAreaAsDirty();
    }
    uint64_t displayId = screenId;
    if (sceneSession->IsPcFoldDevice() && PcFoldScreenManager::GetInstance().IsHalfFolded(displayId)) {
        displayId = sceneSession->GetClientDisplayId();
    }
    NotifyDisplayIdChanged(persistentId, displayId);
    return WSError::WS_OK;
}

void SceneSessionManager::RegisterClientDisplayIdChangeNotifyManagerFunc(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "sceneSession is nullptr");
        return;
    }
    sceneSession->SetClientDisplayIdChangeListener([this](uint32_t windowId) {
        UpdatePrivateStateAndNotify(windowId);
    });
}

void SceneSessionManager::RegisterGetRsCmdBlockingCountFunc(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "session is nullptr");
        return;
    }
    sceneSession->SetGetRsCmdBlockingCountFunc([this]() {
        return GetOrResetRsCmdBlockingCount();
    });
    TLOGD(WmsLogTag::WMS_LAYOUT, "success");
}

WSError SceneSessionManager::NotifyStackEmpty(int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_LIFE, "persistentId %{public}d", persistentId);
    auto task = [this, persistentId]() {
        auto sceneSession = GetSceneSession(persistentId);
        if (!sceneSession) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is nullptr");
            return WSError::WS_ERROR_INVALID_WINDOW;
        }
        NotifySessionUpdate(sceneSession->GetSessionInfo(), ActionType::STACK_EMPTY);
        return WSError::WS_OK;
    };
    taskScheduler_->PostAsyncTask(task, "NotifyStackEmpty:PID:" + std::to_string(persistentId));
    return WSError::WS_OK;
}

void DisplayChangeListener::OnImmersiveStateChange(ScreenId screenId, bool& immersive)
{
    immersive = SceneSessionManager::GetInstance().GetImmersiveState(screenId);
}

bool SceneSessionManager::GetImmersiveState(ScreenId screenId)
{
    return taskScheduler_->PostSyncTask([this, screenId, where = __func__] {
        bool isPcOrPadFreeMultiWindowMode = false;
        IsPcOrPadFreeMultiWindowMode(isPcOrPadFreeMultiWindowMode);
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                TLOGNE(WmsLogTag::WMS_IMMS, "%{public}s session is nullptr", where);
                continue;
            }
            if (!WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
                continue;
            }
            auto state = sceneSession->GetSessionState();
            if (state != SessionState::STATE_FOREGROUND && state != SessionState::STATE_ACTIVE) {
                continue;
            }
            if (sceneSession->GetWindowMode() != WindowMode::WINDOW_MODE_FULLSCREEN) {
                continue;
            }
            if (sceneSession->GetSessionProperty()->GetDisplayId() != screenId) {
                continue;
            }
            if (isPcOrPadFreeMultiWindowMode) {
                if (sceneSession->IsLayoutFullScreen()) {
                    return true;
                }
                continue;
            }
            auto sysBarProperty = sceneSession->GetSessionProperty()->GetSystemBarProperty();
            if (sysBarProperty[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ == false) {
                TLOGNI(WmsLogTag::WMS_IMMS, "%{public}s window is immersive. id: %{public}d", where,
                    sceneSession->GetPersistentId());
                return true;
            } else {
                TLOGNI(WmsLogTag::WMS_IMMS, "%{public}s statusBar is enabled. id: %{public}d", where,
                    sceneSession->GetPersistentId());
                break;
            }
        }
        TLOGNI(WmsLogTag::WMS_IMMS, "%{public}s not immersive", where);
        return false;
    }, __func__);
}

void SceneSessionManager::NotifySessionForeground(const sptr<SceneSession>& session, uint32_t reason,
    bool withAnimation)
{
    session->NotifySessionForeground(reason, withAnimation);
}

void SceneSessionManager::NotifySessionBackground(const sptr<SceneSession>& session, uint32_t reason,
    bool withAnimation, bool isFromInnerkits)
{
    session->NotifySessionBackground(reason, withAnimation, isFromInnerkits);
}

WSError SceneSessionManager::UpdateTitleInTargetPos(int32_t persistentId, bool isShow, int32_t height)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "could not find window, persistentId:%{public}d", persistentId);
        return WSError::WS_ERROR_INVALID_WINDOW;
    }
    return sceneSession->UpdateTitleInTargetPos(isShow, height);
}

void AppAnrListener::OnAppDebugStarted(const std::vector<AppExecFwk::AppDebugInfo>& debugInfos)
{
    TLOGI(WmsLogTag::DEFAULT, "AppAnrListener OnAppDebugStarted");
    if (debugInfos.empty()) {
        TLOGE(WmsLogTag::DEFAULT, "AppAnrListener OnAppDebugStarted debugInfos is empty");
        return;
    }
}

void AppAnrListener::OnAppDebugStoped(const std::vector<AppExecFwk::AppDebugInfo>& debugInfos)
{
    TLOGI(WmsLogTag::DEFAULT, "AppAnrListener OnAppDebugStoped");
    if (debugInfos.empty()) {
        TLOGE(WmsLogTag::DEFAULT, "AppAnrListener OnAppDebugStoped debugInfos is empty");
        return;
    }
}

void SceneSessionManager::SetHasRootSceneRequestedVsyncFunc(HasRootSceneRequestedVsyncFunc&& func)
{
    if (!func) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "func is null");
        return;
    }
    hasRootSceneRequestedVsyncFunc_ = std::move(func);
}

WSError SceneSessionManager::HasRootSceneRequestedVsync(bool& hasRootSceneRequestedVsync)
{
    if (!hasRootSceneRequestedVsyncFunc_) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "func is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    hasRootSceneRequestedVsync = hasRootSceneRequestedVsyncFunc_();
    return WSError::WS_OK;
}

void SceneSessionManager::SetRequestVsyncByRootSceneWhenModeChangeFunc(
    RequestVsyncByRootSceneWhenModeChangeFunc&& func)
{
    if (!func) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "func is null");
        return;
    }
    requestVsyncByRootSceneWhenModeChangeFunc_ = std::move(func);
}

void SceneSessionManager::RunAfterNVsyncs(uint32_t vsyncCount, OnCallback&& callback)
{
    if (!vsyncStation_) {
        TLOGE(WmsLogTag::DEFAULT, "could not request next vsync");
        return;
    }
    auto counter = std::make_shared<std::atomic<uint32_t>>(0);
    auto vsyncCallback = std::make_shared<VsyncCallback>();

    vsyncCallback->onCallback = [weakStation = std::weak_ptr<VsyncStation>(vsyncStation_),
                                 weakVsyncCallback = std::weak_ptr<VsyncCallback>(vsyncCallback),
                                 counter,
                                 vsyncCount,
                                 callback = std::move(callback),
                                 where =  __func__](int64_t timestamp, int64_t frameCount) mutable {
        if (++(*counter) >= vsyncCount) {
            callback(timestamp, frameCount);
            return;
        }
        auto vsyncStation = weakStation.lock();
        if (!vsyncStation) {
            TLOGE(WmsLogTag::DEFAULT, "Could not request next vsync");
            return;
        }
        if (auto nextCallback = weakVsyncCallback.lock()) {
            vsyncStation->RequestVsync(nextCallback);
        }
    };
    vsyncStation_->RequestVsync(vsyncCallback);
}

WMError SceneSessionManager::UpdateWindowModeByIdForUITest(int32_t windowId, int32_t updateMode)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    auto task = [this, windowId, updateMode, where = __func__] {
        auto sceneSession = GetSceneSession(windowId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_LAYOUT, "sceneSession is nullptr, windowId: %{public}d, updateMode: %{public}d",
                windowId, updateMode);
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s, windowId: %{public}d, updateMode: %{public}d", where , windowId, updateMode);
        return sceneSession->UpdateWindowModeForUITest(updateMode);
    };
    return taskScheduler_->PostSyncTask(task,
        "UpdateWindowModeByIdForUITest windowId: " + std::to_string(windowId) + " updateMode: " + std::to_string(updateMode));
}

WSError SceneSessionManager::RequestVsyncByRootSceneWhenModeChange(const std::shared_ptr<VsyncCallback>& vsyncCallback)
{
    if (!requestVsyncByRootSceneWhenModeChangeFunc_) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "func is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    requestVsyncByRootSceneWhenModeChangeFunc_(vsyncCallback);
    return WSError::WS_OK;
}

void SceneSessionManager::FlushUIParams(ScreenId screenId, std::unordered_map<int32_t, SessionUIParam>&& uiParams)
{
    if (!Session::IsScbCoreEnabled()) {
        return;
    }
    if (onFlushUIParamsFunc_ != nullptr) {
        onFlushUIParamsFunc_();
    }
    const char* const where = __func__;
    taskScheduler_->PostAsyncTask([this, screenId, where,
        uiParams = std::move(uiParams)]() THREAD_SAFETY_GUARD(SCENE_GUARD) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSessionManager::FlushUIParams");
        TLOGND(WmsLogTag::WMS_PIPELINE, "%{public}s", where);
        {
            std::unique_lock<std::mutex> lock(nextFlushCompletedMutex_);
            nextFlushCompletedCV_.notify_all();
        }
        std::vector<std::pair<uint32_t, uint32_t>> appZOrderList;
        processingFlushUIParams_.store(true);
        auto keyboardSession = GetKeyboardSession(screenId, false);
        {
            std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
            for (const auto& [_, sceneSession] : sceneSessionMap_) {
                if (sceneSession == nullptr) {
                    continue;
                }
                if (isNotCurrentScreen(sceneSession, screenId)) {
                    TLOGND(WmsLogTag::WMS_PIPELINE, "%{public}s The screenId of this Scenesession is invalid", where);
                    continue;
                }
                if (auto iter = uiParams.find(sceneSession->GetPersistentId()); iter != uiParams.end()) {
                    if (sceneSession->IsAppSession()) {
                        if (!sceneSession->IsVisible()) {
                            appZOrderList.push_back(std::make_pair(0, iter->second.zOrder_));
                        }
                        appZOrderList.push_back(std::make_pair(sceneSession->GetZOrder(), iter->second.zOrder_));
                    }
                    sessionMapDirty_ |= sceneSession->UpdateUIParam(iter->second);
                } else {
                    sessionMapDirty_ |= sceneSession->UpdateUIParam();
                }
            }
            if (keyboardSession != nullptr) {
                keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
            }
        }
        processingFlushUIParams_.store(false);

        // post process if dirty
        if ((sessionMapDirty_ & (~static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA))) !=
            static_cast<uint32_t>(SessionUIDirtyFlag::NONE)) {
            TLOGND(WmsLogTag::WMS_PIPELINE, "%{public}s found dirty: %{public}d", where, sessionMapDirty_);
            for (const auto& item : uiParams) {
                TLOGND(WmsLogTag::WMS_PIPELINE, "id: %{public}d, zOrder: %{public}d, rect: %{public}s, transX:%{public}f,"
                    " transY:%{public}f, needSync:%{public}d, interactive:%{public}d", item.first, item.second.zOrder_,
                    item.second.rect_.ToString().c_str(), item.second.transX_, item.second.transY_,
                    item.second.needSync_, item.second.interactive_);
            }
            ProcessUpdateLastFocusedAppId(appZOrderList);
            ProcessFocusZOrderChange(sessionMapDirty_);
            PostProcessFocus();
            PostProcessProperty(sessionMapDirty_);
            NotifyAllAccessibilityInfo();
            AnomalyDetection::SceneZOrderCheckProcess();
        } else if (sessionMapDirty_ == static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA)) {
            PostProcessProperty(sessionMapDirty_);
        }
        FlushWindowInfoToMMI();
        NotifyWindowPropertyChange(screenId);
        sessionMapDirty_ = 0;
        {
            std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
            for (const auto& [_, sceneSession] : sceneSessionMap_) {
                if (sceneSession == nullptr) {
                    continue;
                }
                if (isNotCurrentScreen(sceneSession, screenId)) {
                    TLOGND(WmsLogTag::WMS_PIPELINE, "%{public}s The screenId of this Scenesession is invalid", where);
                    continue;
                }
                sceneSession->ResetSizeChangeReasonIfDirty();
                sceneSession->ResetDirtyFlags();
                if (WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
                    sceneSession->SetUIStateDirty(false);
                }
            }
        }
        if (needCloseSync_) {
            if (closeSyncFunc_) {
                closeSyncFunc_();
            }
            needCloseSync_ = false;
        }
    }, __func__);
}

void SceneSessionManager::ProcessUpdateLastFocusedAppId(const std::vector<std::pair<uint32_t, uint32_t>>& zOrderList)
{
    auto focusGroup = windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    if (focusGroup == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "focus group is nullptr: %{public}" PRIu64, DEFAULT_DISPLAY_ID);
        return;
    }
    auto lastFocusedAppSessionId = focusGroup->GetLastFocusedAppSessionId();
    TLOGD(WmsLogTag::WMS_FOCUS, "last focused app: %{public}d, list size %{public}zu",
        lastFocusedAppSessionId, zOrderList.size());
    if (lastFocusedAppSessionId == INVALID_SESSION_ID || zOrderList.empty()) {
        return;
    }
    auto lastFocusedAppSession = GetSceneSession(lastFocusedAppSessionId);
    if (lastFocusedAppSession == nullptr) {
        return;
    }
    uint32_t lastFocusedAppZOrder = lastFocusedAppSession->GetZOrder();
    auto it = std::find_if(zOrderList.begin(), zOrderList.end(), [lastFocusedAppZOrder]
        (const std::pair<uint32_t, uint32_t>& pair) {
        return pair.first < lastFocusedAppZOrder && pair.second > lastFocusedAppZOrder;
    });
    if (it != zOrderList.end()) {
        TLOGD(WmsLogTag::WMS_FOCUS, "clear with high zOrder app visible");
        focusGroup->SetLastFocusedAppSessionId(INVALID_SESSION_ID);
    }
}

void SceneSessionManager::ProcessFocusZOrderChange(uint32_t dirty)
{
    if (!(dirty & static_cast<uint32_t>(SessionUIDirtyFlag::Z_ORDER))) {
        return;
    }
    if (!systemConfig_.IsPhoneWindow() && !systemConfig_.IsPadWindow()) {
        return;
    }
    TLOGD(WmsLogTag::WMS_FOCUS, "has zOrder dirty");
    auto focusedSessionId = GetFocusedSessionId(DEFAULT_DISPLAY_ID);
    auto focusedSession = GetSceneSession(focusedSessionId);
    // only when it's from a high zOrder to a low zOrder
    if (focusedSession == nullptr || focusedSession->GetWindowType() == WindowType::WINDOW_TYPE_VOICE_INTERACTION ||
        focusedSession->GetLastZOrder() <= focusedSession->GetZOrder()) {
        return;
    }
    auto voiceInteractionSession = GetSceneSessionByType(WindowType::WINDOW_TYPE_VOICE_INTERACTION);
    if (voiceInteractionSession == nullptr) {
        return;
    }
    TLOGD(WmsLogTag::WMS_FOCUS, "interactionSession: id %{public}d zOrder %{public}d, focusedSession: lastZOrder "
          "%{public}d zOrder %{public}d", voiceInteractionSession->GetPersistentId(),
          voiceInteractionSession->GetZOrder(), focusedSession->GetLastZOrder(), focusedSession->GetZOrder());
    if (focusedSession->GetLastZOrder() < voiceInteractionSession->GetZOrder() ||
        focusedSession->GetZOrder() > voiceInteractionSession->GetZOrder()) {
        return;
    }
    RequestSessionFocus(voiceInteractionSession->GetPersistentId(), true, FocusChangeReason::VOICE_INTERACTION);
}

void SceneSessionManager::PostProcessFocus()
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSessionManager::PostProcessFocus");
    // priority process focus requests from top to bottom
    std::vector<std::pair<int32_t, sptr<SceneSession>>> processingSessions;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (auto& iter : sceneSessionMap_) {
            auto session = iter.second;
            if (session == nullptr || !session->GetPostProcessFocusState().enabled_ ||
                !session->IsVisibleForeground()) {
                continue;
            }
            processingSessions.push_back(iter);
        }
    }
    CmpFunc cmp = [](std::pair<int32_t, sptr<SceneSession>>& lhs, std::pair<int32_t, sptr<SceneSession>>& rhs) {
        bool focusCmp = lhs.second->GetPostProcessFocusState().isFocused_ &&
            !rhs.second->GetPostProcessFocusState().isFocused_;
        uint32_t lhsZOrder = lhs.second != nullptr ? lhs.second->GetZOrder() : 0;
        uint32_t rhsZOrder = rhs.second != nullptr ? rhs.second->GetZOrder() : 0;
        return focusCmp || lhsZOrder > rhsZOrder;
    };
    std::sort(processingSessions.begin(), processingSessions.end(), cmp);

    // only change focus one time
    std::unordered_set<DisplayId> focusChangedSet;
    for (auto iter = processingSessions.begin(); iter != processingSessions.end(); ++iter) {
        auto session = iter->second;
        if (session == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "session is nullptr");
            continue;
        }
        auto displayId = session->GetSessionProperty()->GetDisplayId();
        auto displayGroupId = windowFocusController_->GetDisplayGroupId(displayId);
        TLOGD(WmsLogTag::WMS_PIPELINE,
            "id: %{public}d, isFocused: %{public}d, reason: %{public}d, focusableOnShow: %{public}d",
            session->GetPersistentId(), session->GetPostProcessFocusState().isFocused_,
            session->GetPostProcessFocusState().reason_, session->IsFocusableOnShow());
        if (focusChangedSet.find(displayGroupId) != focusChangedSet.end()) {
            session->ResetPostProcessFocusState();
            continue;
        }
        if (!session->IsFocusedOnShow()) {
            if (IsSessionVisibleForeground(session)) {
                session->SetFocusedOnShow(true);
            }
            session->ResetPostProcessFocusState();
            continue;
        }

        WSError ret = WSError::WS_DO_NOTHING;
        if (session->GetPostProcessFocusState().isFocused_) {
            if (session->GetPostProcessFocusState().reason_ == FocusChangeReason::SCB_START_APP &&
                session->IsDelayFocusChange()) {
                TLOGI(WmsLogTag::WMS_FOCUS, "delay focus change until the window is visible");
                continue;
            } else if (session->GetPostProcessFocusState().reason_ == FocusChangeReason::SCB_START_APP) {
                ret = RequestSessionFocusImmediately(session->GetPersistentId());
            } else if (session->GetPostProcessFocusState().reason_ == FocusChangeReason::RECENT) {
                ret = RequestSessionFocus(session->GetPersistentId(),
                                          session->GetPostProcessFocusState().byForeground_,
                                          session->GetPostProcessFocusState().reason_);
            } else {
                ret = RequestSessionFocus(session->GetPersistentId(), true,
                                          session->GetPostProcessFocusState().reason_);
            }
        } else {
            ret = RequestSessionUnfocus(session->GetPersistentId(), session->GetPostProcessFocusState().reason_);
        }
        session->ResetPostProcessFocusState();
        // if succeed then end process
        if (ret == WSError::WS_OK) {
            focusChangedSet.insert(displayGroupId);
        }
    }
}

void SceneSessionManager::PostProcessProperty(uint32_t dirty)
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSessionManager::PostProcessProperty");
    if (dirty == static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA)) {
        // only trigger update avoid area
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (auto& iter : sceneSessionMap_) {
            auto session = iter.second;
            if (session == nullptr) {
                continue;
            }
            session->PostProcessNotifyAvoidArea();
        }
        return;
    }

    std::vector<std::pair<int32_t, sptr<SceneSession>>> processingSessions;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (auto& iter : sceneSessionMap_) {
            auto session = iter.second;
            if (session == nullptr || !session->GetPostProcessProperty()) {
                continue;
            }
            processingSessions.push_back(iter);
        }
    }

    for (auto iter = processingSessions.begin(); iter != processingSessions.end(); ++iter) {
        auto session = iter->second;
        if (session == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "session is nullptr");
            continue;
        }
        TLOGD(WmsLogTag::WMS_PIPELINE, "id: %{public}d", session->GetPersistentId());
        UpdateForceHideState(session, session->GetSessionProperty(), true);
        HandleKeepScreenOn(session, session->IsKeepScreenOn(), WINDOW_SCREEN_LOCK_PREFIX, session->keepScreenLock_);
        HandleKeepScreenOn(session, session->IsViewKeepScreenOn(), VIEW_SCREEN_LOCK_PREFIX,
                           session->viewKeepScreenLock_);
        UpdatePrivateStateAndNotify(session->GetPersistentId());
        if (session->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
            ProcessSubSessionForeground(session);
        }
        session->SetPostProcessProperty(false);
    }

    // update avoid area
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (auto& iter : sceneSessionMap_) {
            auto session = iter.second;
            if (session == nullptr) {
                continue;
            }
            session->PostProcessNotifyAvoidArea();
        }
    }
}

/** @note @window.hierarchy */
WSError SceneSessionManager::RaiseWindowToTop(int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "id %{public}d", persistentId);
    auto isSaCall = SessionPermission::IsSACalling();
    if (!isSaCall) {
        TLOGE(WmsLogTag::WMS_HIERARCHY, "The interface only support for sa call");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    auto task = [this, persistentId]() {
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "session is nullptr");
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        if (!IsSessionVisibleForeground(sceneSession)) {
            TLOGND(WmsLogTag::WMS_HIERARCHY, "session is not visible!");
            return WSError::WS_DO_NOTHING;
        }
        FocusChangeReason reason = FocusChangeReason::MOVE_UP;
        RequestSessionFocus(persistentId, true, reason);
        if (WindowHelper::IsSubWindow(sceneSession->GetWindowType())) {
            sceneSession->RaiseToAppTop();
        }
        if (WindowHelper::IsSubWindow(sceneSession->GetWindowType()) ||
            sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_DIALOG) {
            TLOGND(WmsLogTag::WMS_HIERARCHY, "parent session id: %{public}d", sceneSession->GetParentPersistentId());
            sceneSession = GetSceneSession(sceneSession->GetParentPersistentId());
        }
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "parent session is nullptr");
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        if (sceneSession->GetWindowType() == WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
            sceneSession->NotifyClick(true, false);
            return WSError::WS_OK;
        } else {
            TLOGNE(WmsLogTag::WMS_HIERARCHY, "session is not app main window!");
            return WSError::WS_ERROR_INVALID_SESSION;
        }
    };
    taskScheduler_->PostAsyncTask(task, "RaiseWindowToTop");
    return WSError::WS_OK;
}

WSError SceneSessionManager::ShiftAppWindowFocus(int32_t sourcePersistentId, int32_t targetPersistentId)
{
    TLOGI(WmsLogTag::WMS_FOCUS, "from id: %{public}d to id: %{public}d", sourcePersistentId, targetPersistentId);
    sptr<SceneSession> sourceSession = GetSceneSession(sourcePersistentId);
    if (sourceSession == nullptr) {
        TLOGE(WmsLogTag::WMS_FOCUS, "session is nullptr, id: %{public}d", sourcePersistentId);
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    auto displayId = sourceSession->GetSessionProperty()->GetDisplayId();
    auto focusedSessionId = GetFocusedSessionId(displayId);
    if (sourcePersistentId != focusedSessionId) {
        TLOGE(WmsLogTag::WMS_FOCUS, "source session need be focused, focusedSessionId: %{public}d", focusedSessionId);
        return WSError::WS_ERROR_INVALID_OPERATION;
    }
    if (targetPersistentId == focusedSessionId) {
        TLOGE(WmsLogTag::WMS_FOCUS, "target session has been focused, focusedSessionId: %{public}d", focusedSessionId);
        return WSError::WS_DO_NOTHING;
    }
    WSError ret = GetAppMainSceneSession(sourcePersistentId, sourceSession);
    if (ret != WSError::WS_OK) {
        return ret;
    }
    sptr<SceneSession> targetSession = nullptr;
    ret = GetAppMainSceneSession(targetPersistentId, targetSession);
    if (ret != WSError::WS_OK) {
        return ret;
    }
    if (sourceSession->GetSessionInfo().bundleName_ != targetSession->GetSessionInfo().bundleName_) {
        TLOGE(WmsLogTag::WMS_FOCUS, "verify bundle failed, source name is %{public}s but target name is %{public}s)",
            sourceSession->GetSessionInfo().bundleName_.c_str(), targetSession->GetSessionInfo().bundleName_.c_str());
        return WSError::WS_ERROR_INVALID_CALLING;
    }
    if (!SessionPermission::IsSameBundleNameAsCalling(targetSession->GetSessionInfo().bundleName_)) {
        return WSError::WS_ERROR_INVALID_CALLING;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    if (callingPid != targetSession->GetCallingPid()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "permission denied, not call by the same process");
        return WSError::WS_ERROR_INVALID_CALLING;
    }
    targetSession->NotifyClick(true, false);
    FocusChangeReason reason = FocusChangeReason::CLIENT_REQUEST;
    return RequestSessionFocus(targetPersistentId, false, reason);
}

/** @note @window.hierarchy */
WSError SceneSessionManager::SetSpecificWindowZIndex(WindowType windowType, int32_t zIndex)
{
    if (!SessionPermission::IsSystemAppCall()) {
        TLOGE(WmsLogTag::WMS_FOCUS, "permission denied");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    if (!WindowHelper::IsSupportSetZIndexWindow(windowType)) {
        TLOGE(WmsLogTag::WMS_FOCUS, "windowType not support %{public}d", windowType);
        return WSError::WS_ERROR_INVALID_CALLING;
    }
    const auto callingPid = IPCSkeleton::GetCallingRealPid();
    const char* const where = __func__;
    return taskScheduler_->PostSyncTask([this, windowType, zIndex, callingPid, where] {
        TLOGNI(WmsLogTag::WMS_FOCUS, "windowType: %{public}d, zIndex: %{public}d", windowType, zIndex);
        if (setSpecificWindowZIndexFunc_) {
            setSpecificWindowZIndexFunc_(windowType, zIndex, SetSpecificZIndexReason::SET);
            {
                std::lock_guard<std::mutex> lock(specificZIndexByPidMapMutex_);
                specificZIndexByPidMap_[windowType] = callingPid;
            }
        } else {
            TLOGNE(WmsLogTag::WMS_FOCUS, "setSpecificWindowZIndexFunc_ is nullptr");
        }
        return WSError::WS_OK;
    }, __func__);
}

/** @note @window.hierarchy */
WSError SceneSessionManager::ResetSpecificWindowZIndex(int32_t pid)
{
    const char* const where = __func__;
    return taskScheduler_->PostSyncTask([this, pid, where] {
        TLOGNI(WmsLogTag::WMS_FOCUS, "id: %{public}d", pid);
        std::vector<WindowType> windowTypeList;
        {
            std::lock_guard<std::mutex> lock(specificZIndexByPidMapMutex_);
            for (const auto& item : specificZIndexByPidMap_) {
                if (item.second == pid) {
                    windowTypeList.push_back(item.first);
                }
            }
        }
        if (windowTypeList.size() == 0) {
            return WSError::WS_ERROR_NULLPTR;
        }
        if (setSpecificWindowZIndexFunc_) {
            for (const auto& windowType : windowTypeList) {
                setSpecificWindowZIndexFunc_(windowType, 0, SetSpecificZIndexReason::RESET);
            }
            {
                std::lock_guard<std::mutex> lock(specificZIndexByPidMapMutex_);
                for (const auto& windowType : windowTypeList) {
                    specificZIndexByPidMap_.erase(windowType);
                }
            }
        } else {
            TLOGNE(WmsLogTag::WMS_FOCUS, "setSpecificWindowZIndexFunc_ is nullptr");
        }
        return WSError::WS_OK;
    }, __func__);
}

WSError SceneSessionManager::GetAppMainSceneSession(int32_t persistentId, sptr<SceneSession>& sceneSession)
{
    sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::DEFAULT, "session(%{public}d) is nullptr", persistentId);
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    if (sceneSession->GetWindowType() != WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
        if (sceneSession->GetWindowType() != WindowType::WINDOW_TYPE_APP_SUB_WINDOW) {
            TLOGE(WmsLogTag::DEFAULT, "session(%{public}d) is not main window or sub window", persistentId);
            return WSError::WS_ERROR_INVALID_CALLING;
        }
        sceneSession = GetSceneSession(sceneSession->GetParentPersistentId());
        if (sceneSession == nullptr) {
            TLOGE(WmsLogTag::DEFAULT, "session(%{public}d) parent is nullptr", persistentId);
            return WSError::WS_ERROR_INVALID_SESSION;
        }
    }
    return WSError::WS_OK;
}

std::shared_ptr<Media::PixelMap> SceneSessionManager::GetSessionSnapshotPixelMap(const int32_t persistentId,
    const float scaleParam, SnapshotNodeType snapshotNode, bool needSnapshot)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (!sceneSession) {
        TLOGE(WmsLogTag::WMS_MAIN, "get scene session is nullptr");
        return nullptr;
    }

    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetSessionSnapshotPixelMap(%d )", persistentId);

    bool isPc = systemConfig_.IsPcWindow() || systemConfig_.IsFreeMultiWindowMode();
    bool useCurWindow = (snapshotNode == SnapshotNodeType::DEFAULT_NODE) ?
        isPc : (snapshotNode == SnapshotNodeType::LEASH_NODE) ? false : true;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    if (needSnapshot && !sceneSession->IsPersistentImageFit()) {
        pixelMap = sceneSession->Snapshot(true, scaleParam, useCurWindow);
    }
    if (!pixelMap) {
        TLOGI(WmsLogTag::WMS_MAIN, "get local snapshot pixelmap start");
        pixelMap = sceneSession->GetSnapshotPixelMap(snapshotScale_, scaleParam);
    }
    return pixelMap;
}

const std::map<int32_t, sptr<SceneSession>> SceneSessionManager::GetSceneSessionMap()
{
    std::map<int32_t, sptr<SceneSession>> retSceneSessionMap;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        retSceneSessionMap = sceneSessionMap_;
    }
    EraseIf(retSceneSessionMap, [this](const auto& pair) {
        if (pair.second == nullptr) {
            return true;
        }

        if (pair.second->GetWindowType() == WindowType::WINDOW_TYPE_KEYBOARD_PANEL) {
            if (pair.second->IsVisible()) {
                return false;
            }
            return true;
        }

        if (pair.second->IsSystemInput()) {
            return false;
        } else if (pair.second->IsSystemSession() && pair.second->IsVisible() && pair.second->IsSystemActive()) {
            return false;
        }

        if (!Rosen::SceneSessionManager::GetInstance().IsSessionVisible(pair.second)) {
            return true;
        }
        return false;
    });
    return retSceneSessionMap;
}

void SceneSessionManager::NotifyUpdateRectAfterLayout()
{
    std::shared_ptr<RSTransaction> rsTransaction = nullptr;
    std::shared_ptr<RSUIContext> rsUIContext;
    if (auto rootSceneSession = GetRootSceneSession()) {
        rsUIContext = rootSceneSession->GetRSUIContext();
    }
    rsTransaction = RSSyncTransactionAdapter::GetRSTransaction(rsUIContext);
    auto task = [this, rsTransaction] {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession && sceneSession->IsDirtyWindow()) {
                sceneSession->NotifyClientToUpdateRect("AfterLayoutFromPersistentTask", rsTransaction);
            }
        }
    };
    // need sync task since animation transcation need
    taskScheduler_->PostAsyncTask(task, __func__);
}

WMError SceneSessionManager::ListWindowInfo(const WindowInfoOption& windowInfoOption,
    std::vector<sptr<WindowInfo>>& infos)
{
    if (!(SessionPermission::IsSACalling())) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    const char* const where = __func__;
    return taskScheduler_->PostSyncTask([this, windowInfoOption, &infos, where] {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSessionManager::ListWindowInfo (%u) "
            "(%u) (%" PRIu64 ") (%d)",
            static_cast<WindowInfoFilterOptionDataType>(windowInfoOption.windowInfoFilterOption),
            static_cast<WindowInfoTypeOptionDataType>(windowInfoOption.windowInfoTypeOption),
            windowInfoOption.displayId, windowInfoOption.windowId);
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                TLOGNW(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: session is null", where);
                continue;
            }
            if (!FilterForListWindowInfo(windowInfoOption, sceneSession)) {
                TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: filter win: %{public}d",
                    where, sceneSession->GetWindowId());
                continue;
            }
            auto windowInfo = sptr<WindowInfo>::MakeSptr();
            if (IsChosenWindowOption(windowInfoOption.windowInfoTypeOption, WindowInfoTypeOption::WINDOW_UI_INFO)) {
                windowInfo->windowUIInfo = sceneSession->GetWindowUIInfoForWindowInfo();
            }
            if (IsChosenWindowOption(windowInfoOption.windowInfoTypeOption, WindowInfoTypeOption::WINDOW_DISPLAY_INFO)) {
                windowInfo->windowDisplayInfo = sceneSession->GetWindowDisplayInfoForWindowInfo();
            }
            if (IsChosenWindowOption(windowInfoOption.windowInfoTypeOption, WindowInfoTypeOption::WINDOW_LAYOUT_INFO)) {
                windowInfo->windowLayoutInfo = sceneSession->GetWindowLayoutInfoForWindowInfo();
            }
            if (IsChosenWindowOption(windowInfoOption.windowInfoTypeOption, WindowInfoTypeOption::WINDOW_META_INFO)) {
                windowInfo->windowMetaInfo = sceneSession->GetWindowMetaInfoForWindowInfo();
            }
            infos.emplace_back(windowInfo);
        }
        return WMError::WM_OK;
    }, __func__);
}

bool SceneSessionManager::FilterForListWindowInfo(const WindowInfoOption& windowInfoOption,
    const sptr<SceneSession>& sceneSession) const
{
    DisplayId displayId = windowInfoOption.displayId;
    if (PcFoldScreenManager::GetInstance().IsHalfFoldedOnMainDisplay(sceneSession->GetSessionProperty()->GetDisplayId())) {
        if (displayId == DEFAULT_DISPLAY_ID && sceneSession->GetSessionGlobalRect().posY_ >= GetFoldLowerScreenPosY()) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "fold lower screen win: %{public}d", sceneSession->GetWindowId());
            return false;
        }
        if (displayId == VIRTUAL_DISPLAY_ID) {
            if (sceneSession->GetSessionGlobalRect().posY_ +
                sceneSession->GetSessionGlobalRect().height_ < GetFoldLowerScreenPosY()) {
                TLOGD(WmsLogTag::WMS_ATTRIBUTE, "fold virtual screen win: %{public}d", sceneSession->GetWindowId());
                return false;
            }
            displayId = DEFAULT_DISPLAY_ID;
        }
    }
    if (displayId != DISPLAY_ID_INVALID && sceneSession->GetSessionProperty()->GetDisplayId() != displayId) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "win: %{public}d, target displayId: %{public}" PRIu64,
            sceneSession->GetWindowId(), displayId);
        return false;
    }
    if (windowInfoOption.windowId != 0 && sceneSession->GetWindowId() != windowInfoOption.windowId) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "win: %{public}d, target winId: %{public}d",
            sceneSession->GetWindowId(), windowInfoOption.windowId);
        return false;
    }
    if (IsChosenWindowOption(windowInfoOption.windowInfoFilterOption, WindowInfoFilterOption::EXCLUDE_SYSTEM) &&
        sceneSession->GetSessionInfo().isSystem_) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "exclude system win: %{public}d", sceneSession->GetWindowId());
        return false;
    }
    if (IsChosenWindowOption(windowInfoOption.windowInfoFilterOption, WindowInfoFilterOption::VISIBLE) &&
        (sceneSession->GetVisibilityState() == WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION ||
        sceneSession->GetVisibilityState() == WINDOW_LAYER_STATE_MAX)) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "exclude unvisible win: %{public}d", sceneSession->GetWindowId());
        return false;
    }
    if (IsChosenWindowOption(windowInfoOption.windowInfoFilterOption, WindowInfoFilterOption::FOREGROUND) &&
        !IsSessionVisibleForeground(sceneSession)) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "exclude background win: %{public}d", sceneSession->GetWindowId());
        return false;
    }
    return true;
}

WMError SceneSessionManager::GetAllWindowLayoutInfo(DisplayId displayId,
    std::vector<sptr<WindowLayoutInfo>>& infos)
{
    auto task = [this, displayId, &infos, funcName = __func__]() mutable {
        bool isVirtualDisplay = false;
        if (displayId == VIRTUAL_DISPLAY_ID) {
            displayId = DEFAULT_DISPLAY_ID;
            isVirtualDisplay = true;
        }
        std::vector<sptr<SceneSession>> filteredSessions;
        FilterForGetAllWindowLayoutInfo(displayId, isVirtualDisplay, filteredSessions);
        for (const auto& session : filteredSessions) {
            Rect globalScaledRect;
            session->GetGlobalScaledRect(globalScaledRect);
            if (isVirtualDisplay) {
                globalScaledRect.posY_ -= GetFoldLowerScreenPosY();
            }
            HookWindowInfo hookWindowInfo = GetAppHookWindowInfo(session->GetSessionInfo().bundleName_);
            if (hookWindowInfo.enableHookWindow && !session->IsFullScreenInForceSplit() &&
                WindowHelper::IsMainWindow(session->GetWindowType()) &&
                !MathHelper::NearEqual(hookWindowInfo.widthHookRatio, HookWindowInfo::DEFAULT_WINDOW_SIZE_HOOK_RATIO)) {
                globalScaledRect.width_ =
                    static_cast<uint32_t>(globalScaledRect.width_ * hookWindowInfo.widthHookRatio);
                TLOGD(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}u, hook window width, hooked width:%{public}u, "
                    "widthHookRatio:%{public}f.", funcName, session->GetPersistentId(), globalScaledRect.width_,
                    hookWindowInfo.widthHookRatio);
            }
            auto windowLayoutInfo = sptr<WindowLayoutInfo>::MakeSptr();
            windowLayoutInfo->rect = globalScaledRect;
            infos.emplace_back(windowLayoutInfo);
        }
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

void SceneSessionManager::FilterForGetAllWindowLayoutInfo(DisplayId displayId, bool isVirtualDisplay,
    std::vector<sptr<SceneSession>>& filteredSessions)
{
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, session] : sceneSessionMap_) {
            if (session == nullptr) {
                continue;
            }
            if (session->GetSessionGlobalRect().IsInvalid()) {
                continue;
            }
            if (PcFoldScreenManager::GetInstance().IsHalfFoldedOnMainDisplay(session->GetSessionProperty()->GetDisplayId()) &&
                displayId == DEFAULT_DISPLAY_ID) {
                if (isVirtualDisplay &&
                    session->GetSessionRect().posY_ + session->GetSessionRect().height_ < GetFoldLowerScreenPosY()) {
                    continue;
                }
                if (!isVirtualDisplay && session->GetSessionRect().posY_ >= GetFoldLowerScreenPosY()) {
                    continue;
                }
            }
            if (IsGetWindowLayoutInfoNeeded(session) && session->GetSessionProperty()->GetDisplayId() == displayId &&
                session->GetVisibilityState() != WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
                filteredSessions.emplace_back(session);
            }
        }
    }
    std::sort(filteredSessions.begin(), filteredSessions.end(),
              [](const sptr<SceneSession>& lhs, const sptr<SceneSession>& rhs) {
                  return lhs->GetZOrder() > rhs->GetZOrder();
              });
}

int32_t SceneSessionManager::GetFoldLowerScreenPosY() const
{
    const auto& [defaultDisplayRect, virtualDisplayRect, foldCreaseRect] =
        PcFoldScreenManager::GetInstance().GetDisplayRects();
    constexpr int32_t SUPER_FOLD_DIVIDE_FACTOR = 2;
    return foldCreaseRect.height_ != 0 ?
           defaultDisplayRect.height_ - foldCreaseRect.height_ / SUPER_FOLD_DIVIDE_FACTOR + foldCreaseRect.height_ :
           defaultDisplayRect.height_;
}

bool SceneSessionManager::IsGetWindowLayoutInfoNeeded(const sptr<SceneSession>& session) const
{
    std::string name = session->GetWindowName();
    size_t pos = name.size();
    while (pos > 0 && std::isdigit(static_cast<unsigned char>(name[pos - 1]))) {
        --pos;
    }
    name.resize(pos);
    return !session->GetSessionInfo().isSystem_ || LAYOUT_INFO_WHITELIST.find(name) != LAYOUT_INFO_WHITELIST.end();
}

WMError SceneSessionManager::GetGlobalWindowMode(DisplayId displayId, GlobalWindowMode& globalWinMode)
{
    return taskScheduler_->PostSyncTask([this, displayId, &globalWinMode, where = __func__] {
        globalWinMode = GlobalWindowMode::UNKNOWN;
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: displayId=%{public}" PRIu64, where, displayId);
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, session] : sceneSessionMap_) {
            if (session == nullptr) {
                TLOGNW(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: session is null", where);
                continue;
            }
            if (displayId != DISPLAY_ID_INVALID && !IsSessionInSpecificDisplay(session, displayId)) {
                TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: not in display, win: [%{public}d, %{public}s]",
                    where, session->GetWindowId(), session->GetWindowName().c_str());
                continue;
            }
            if (!session->IsSessionForeground()) {
                TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: skip win=[%{public}d, %{public}s], stat: %{public}u",
                    where, session->GetWindowId(), session->GetWindowName().c_str(),
                    static_cast<uint32_t>(session->GetSessionState()));
                continue;
            }
            WindowMode winMode = session->GetWindowMode();
            if (WindowHelper::IsPipWindowMode(winMode)) {
                TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: found pip win=[%{public}d, %{public}s]",
                    where, session->GetWindowId(), session->GetWindowName().c_str());
                globalWinMode = globalWinMode | GlobalWindowMode::PIP;
            } else if (WindowHelper::IsFullScreenWindow(winMode)) {
                TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: found fullscreen win=[%{public}d, %{public}s]",
                    where, session->GetWindowId(), session->GetWindowName().c_str());
                globalWinMode = globalWinMode | GlobalWindowMode::FULLSCREEN;
            } else if (WindowHelper::IsSplitWindowMode(winMode)) {
                TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: found split win=[%{public}d, %{public}s]",
                    where, session->GetWindowId(), session->GetWindowName().c_str());
                globalWinMode = globalWinMode | GlobalWindowMode::SPLIT;
            } else if (WindowHelper::IsFloatingWindow(winMode)) {
                TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: found floating win=[%{public}d, %{public}s]",
                    where, session->GetWindowId(), session->GetWindowName().c_str());
                globalWinMode = globalWinMode | GlobalWindowMode::FLOAT;
            }
            if (globalWinMode == GlobalWindowMode::ALL) {
                break;
            }
        }
        return WMError::WM_OK;
    }, __func__);
}

WMError SceneSessionManager::GetTopNavDestinationName(int32_t windowId, std::string& topNavDestName)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied!");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto session = GetSceneSession(windowId);
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "no session with id: %{public}d", windowId);
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (!session->IsSessionForeground()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "invalid operation: win=%{public}d, stat: %{public}u",
            session->GetWindowId(), static_cast<uint32_t>(session->GetSessionState()));
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    auto wsErrCode = session->GetTopNavDestinationName(topNavDestName);
    if (wsErrCode != WSError::WS_OK) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "get failed: win=%{public}d, errCode=%{public}d",
            session->GetWindowId(), static_cast<int32_t>(wsErrCode));
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    return WMError::WM_OK;
}

bool SceneSessionManager::IsSessionInSpecificDisplay(const sptr<SceneSession>& session, DisplayId displayId) const
{
    if (session == nullptr) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "session is null");
        return false;
    }
    if (PcFoldScreenManager::GetInstance().IsHalfFoldedOnMainDisplay(session->GetSessionProperty()->GetDisplayId()) &&
        (displayId == VIRTUAL_DISPLAY_ID || displayId == DEFAULT_DISPLAY_ID)) {
        bool isVirtualDisplay = displayId == VIRTUAL_DISPLAY_ID;
        if (isVirtualDisplay &&
            session->GetSessionRect().posY_ + session->GetSessionRect().height_ < GetFoldLowerScreenPosY()) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "not in virtual display, win: [%{public}d, %{public}s]",
                session->GetWindowId(), session->GetWindowName().c_str());
            return false;
        }
        if (!isVirtualDisplay && session->GetSessionRect().posY_ >= GetFoldLowerScreenPosY()) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "in virtual display, win: [%{public}d, %{public}s]",
                session->GetWindowId(), session->GetWindowName().c_str());
            return false;
        }
        return true;
    }
    return displayId == session->GetSessionProperty()->GetDisplayId();
}

WMError SceneSessionManager::GetVisibilityWindowInfo(std::vector<sptr<WindowVisibilityInfo>>& infos)
{
    auto task = [this, &infos, where = __func__]() {
        for (auto [surfaceId, _] : lastVisibleData_) {
            sptr<SceneSession> session = SelectSesssionFromMap(surfaceId);
            if (session == nullptr) {
                continue;
            }
            WSRect hostRect = session->GetSessionRect();
            Rect rect = {hostRect.posX_, hostRect.posY_,
                         static_cast<uint32_t>(hostRect.width_), static_cast<uint32_t>(hostRect.height_)};
            auto windowStatus = GetWindowStatus(session->GetWindowMode(), session->GetSessionState(),
                                                session->GetSessionProperty());
            auto windowVisibilityInfo = sptr<WindowVisibilityInfo>::MakeSptr(session->GetWindowId(), session->GetCallingPid(),
                session->GetCallingUid(), session->GetVisibilityState(), session->GetWindowType(), windowStatus, rect,
                session->GetSessionInfo().bundleName_, session->GetSessionInfo().abilityName_,
                session->IsFocused());
            windowVisibilityInfo->SetAppIndex(session->GetSessionInfo().appIndex_);
            windowVisibilityInfo->SetIsSystem(session->GetSessionInfo().isSystem_);
            windowVisibilityInfo->SetZOrder(session->GetZOrder());
            Rect globalDisplayRect = session->GetSessionProperty()->GetGlobalDisplayRect();
            windowVisibilityInfo->SetGlobalDisplayRect(globalDisplayRect);
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: wid=%{public}d, globalDisplayRect=%{public}s",
                where, static_cast<int32_t>(session->GetPersistentId()), globalDisplayRect.ToString().c_str());
            HookWindowInfo hookWindowInfo = GetAppHookWindowInfo(session->GetSessionInfo().bundleName_);
            if (hookWindowInfo.enableHookWindow && !session->IsFullScreenInForceSplit() &&
                WindowHelper::IsMainWindow(session->GetWindowType()) &&
                !MathHelper::NearEqual(hookWindowInfo.widthHookRatio, HookWindowInfo::DEFAULT_WINDOW_SIZE_HOOK_RATIO)) {
                rect.width_ = static_cast<uint32_t>(rect.width_ * hookWindowInfo.widthHookRatio);
                globalDisplayRect.width_ =
                    static_cast<uint32_t>(globalDisplayRect.width_ * hookWindowInfo.widthHookRatio);
                windowVisibilityInfo->rect_ = rect;
                windowVisibilityInfo->SetGlobalDisplayRect(globalDisplayRect);
                TLOGD(WmsLogTag::WMS_LAYOUT, "%{public}s: id:%{public}u, hook window width, hooked width:%{public}u, "
                    "hooked globalDisplay width:%{public}u, widthHookRatio:%{public}f.", where,
                    session->GetPersistentId(), rect.width_, globalDisplayRect.width_, hookWindowInfo.widthHookRatio);
            }
            infos.emplace_back(windowVisibilityInfo);
        }
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, "GetVisibilityWindowInfo");
}

void SceneSessionManager::GetAllWindowVisibilityInfos(std::vector<std::pair<int32_t, uint32_t>>& windowVisibilityInfos)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [id, session] : sceneSessionMap_) {
        if (session == nullptr) {
            continue;
        }
        uint32_t visibilityState = static_cast<uint32_t>(session->GetVisibilityState());
        windowVisibilityInfos.push_back(std::make_pair(id, visibilityState));
    }
}

void SceneSessionManager::FlushWindowInfoToMMI(const bool forceFlush)
{
    auto task = [this, forceFlush] {
        if (isUserBackground_) {
            TLOGND(WmsLogTag::WMS_MULTI_USER, "The user is in the background, no need to flush info to MMI");
            return;
        }
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSessionManager::FlushWindowInfoToMMI");
        SceneInputManager::GetInstance().ResetSessionDirty();
        auto [windowInfoList, pixelMapList] = SceneInputManager::GetInstance().GetFullWindowInfoList();
        TLOGND(WmsLogTag::WMS_EVENT, "windowInfoList size: %{public}d", static_cast<int32_t>(windowInfoList.size()));
        SceneInputManager::GetInstance().
            FlushDisplayInfoToMMI(std::move(windowInfoList), std::move(pixelMapList), forceFlush);
    };
    TLOGD(WmsLogTag::WMS_EVENT, "in");
    taskScheduler_->PostAsyncTask(task, __func__);
}

void SceneSessionManager::PostFlushWindowInfoTask(FlushWindowInfoTask&& task,
    const std::string& taskName, const int delayTime)
{
    taskScheduler_->PostAsyncTask(std::move(task), taskName, delayTime);
}

bool SceneSessionManager::GetExtensionWindowIds(const sptr<IRemoteObject>& token, int32_t& persistentId,
    int32_t& parentId)
{
    // This function should be called in task
    auto iter = extSessionInfoMap_.find(token);
    if (iter == extSessionInfoMap_.end()) {
        return false;
    }
    persistentId = iter->second.persistentId;
    parentId = iter->second.parentId;
    return true;
}

void SceneSessionManager::DestroyExtensionSession(const sptr<IRemoteObject>& remoteExtSession, bool isConstrainedModal)
{
    const char* const where = __func__;
    auto task = [this, remoteExtSession, isConstrainedModal, where]() {
        auto iter = remoteExtSessionMap_.find(remoteExtSession);
        if (iter == remoteExtSessionMap_.end()) {
            TLOGNI(WmsLogTag::WMS_UIEXT, "Invalid remoteExtSession");
            return;
        }
        int32_t persistentId = INVALID_SESSION_ID;
        int32_t parentId = INVALID_SESSION_ID;
        auto abilityToken = iter->second;
        if (!GetExtensionWindowIds(abilityToken, persistentId, parentId)) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "Get UIExtension window ids by token failed");
            return;
        }

        TLOGNI(WmsLogTag::WMS_UIEXT, "%{public}s: persistentId=%{public}d, parentId=%{public}d",
            where, persistentId, parentId);
        auto sceneSession = GetSceneSession(parentId);
        if (sceneSession != nullptr) {
            auto oldFlags = sceneSession->GetCombinedExtWindowFlags();
            sceneSession->RemoveExtWindowFlags(persistentId);
            if (oldFlags.hideNonSecureWindowsFlag) {
                HandleSecureSessionShouldHide(sceneSession);
            }
            if (oldFlags.waterMarkFlag) {
                CheckAndNotifyWaterMarkChangedResult();
            }
            if (oldFlags.privacyModeFlag) {
                UpdatePrivateStateAndNotify(parentId);
            }
            if (!isConstrainedModal) {
                sceneSession->RemoveNormalModalUIExtension(persistentId);
            }
            sceneSession->RemoveUIExtSurfaceNodeId(persistentId);
            sceneSession->RemoveExtensionTokenInfo(abilityToken);
        } else {
            ExtensionWindowFlags actions;
            actions.SetAllActive();
            HandleSpecialExtWindowFlagsChange(persistentId, ExtensionWindowFlags(), actions);
        }
        remoteExtSession->RemoveDeathRecipient(extensionDeath_);
        extSessionInfoMap_.erase(iter->second);
        remoteExtSessionMap_.erase(iter);
    };
    taskScheduler_->PostAsyncTask(task, "DestroyExtensionSession");
}

void SceneSessionManager::UpdateModalExtensionRect(const sptr<IRemoteObject>& token, Rect rect)
{
    auto pid = IPCSkeleton::GetCallingRealPid();
    const char* const where = __func__;
    auto task = [this, token, pid, rect, where]() {
        int32_t persistentId = INVALID_SESSION_ID;
        int32_t parentId = INVALID_SESSION_ID;
        if (!GetExtensionWindowIds(token, persistentId, parentId)) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "Get UIExtension window ids by token failed");
            return;
        }
        TLOGNI(WmsLogTag::WMS_UIEXT, "%{public}s: pid=%{public}d, persistentId=%{public}d, "
            "parentId=%{public}d, rect:[%{public}d %{public}d %{public}d %{public}d]",
            where, pid, persistentId, parentId, rect.posX_, rect.posY_, rect.width_, rect.height_);
        auto parentSession = GetSceneSession(parentId);
        if (parentSession) {
            auto parentTransX = parentSession->GetSessionGlobalRect().posX_ - parentSession->GetClientRect().posX_;
            auto parentTransY = parentSession->GetSessionGlobalRect().posY_ - parentSession->GetClientRect().posY_;
            Rect globalRect = { rect.posX_ + parentTransX, rect.posY_ + parentTransY, rect.width_, rect.height_ };
            WSRect transRect = { globalRect.posX_, globalRect.posY_, globalRect.width_, globalRect.height_ };
            parentSession->TransformRelativeRectToGlobalRect(transRect);
            globalRect.posY_ = transRect.posY_;
            ExtensionWindowEventInfo extensionInfo { persistentId, pid, -1, globalRect, rect, true };
            TLOGNI(WmsLogTag::WMS_UIEXT, "%{public}s: pid: %{public}d, persistentId: %{public}d, "
                "parentId: %{public}d, rect: %{public}s, globalRect: %{public}s, parentGlobalRect: %{public}s",
                where, pid, persistentId, parentId, rect.ToString().c_str(), globalRect.ToString().c_str(),
                parentSession->GetSessionGlobalRect().ToString().c_str());
            parentSession->UpdateNormalModalUIExtension(extensionInfo);
        }
    };
    taskScheduler_->PostAsyncTask(task, "UpdateModalExtensionRect");
}

void SceneSessionManager::ProcessModalExtensionPointDown(const sptr<IRemoteObject>& token, int32_t posX, int32_t posY)
{
    auto pid = IPCSkeleton::GetCallingRealPid();
    const char* const where = __func__;
    auto task = [this, token, pid, posX, posY, where]() {
        int32_t persistentId = INVALID_SESSION_ID;
        int32_t parentId = INVALID_SESSION_ID;
        if (!GetExtensionWindowIds(token, persistentId, parentId)) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "Get UIExtension window ids by token failed");
            return;
        }
        TLOGNI(WmsLogTag::WMS_UIEXT, "%{public}s: pid=%{public}d, persistentId=%{public}d, "
            "parentId=%{public}d", where, pid, persistentId, parentId);
        auto parentSession = GetSceneSession(parentId);
        if (parentSession) {
            auto modalUIExtensionEventInfo = parentSession->GetLastModalUIExtensionEventInfo();
            if (modalUIExtensionEventInfo && modalUIExtensionEventInfo.value().pid == pid &&
                modalUIExtensionEventInfo.value().persistentId == persistentId) {
                parentSession->ProcessPointDownSession(posX, posY);
            }
        }
    };
    taskScheduler_->PostAsyncTask(task, "ProcessModalExtensionPointDown");
}

void SceneSessionManager::AddExtensionWindowStageToSCB(const sptr<ISessionStage>& sessionStage,
    const sptr<IRemoteObject>& token, uint64_t surfaceNodeId, int64_t startModalExtensionTimeStamp,
    bool isConstrainedModal)
{
    auto pid = IPCSkeleton::GetCallingRealPid();
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    const char* const where = __func__;
    auto task = [this, sessionStage, token, surfaceNodeId, startModalExtensionTimeStamp,
        isConstrainedModal, pid, callingTokenId, where]() {
        if (sessionStage == nullptr || token == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "input is nullptr");
            return;
        }
        auto remoteExtSession = sessionStage->AsObject();
        if (remoteExtSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "sessionStage object is nullptr");
            return;
        }
        if (extensionDeath_ == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "failed to create death recipient");
            return;
        }
        if (!remoteExtSession->AddDeathRecipient(extensionDeath_)) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "failed to add death recipient");
            return;
        }

        AAFwk::UIExtensionSessionInfo info;
        AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionSessionInfo(token, info);
        if (info.persistentId == INVALID_SESSION_ID || info.hostWindowId == INVALID_SESSION_ID) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "Get UIExtension session info failed");
            return;
        }

        int32_t persistentId = info.persistentId;
        int32_t parentId = static_cast<int32_t>(info.hostWindowId);
        UIExtensionUsage usage = static_cast<UIExtensionUsage>(info.uiExtensionUsage);
        TLOGNI(WmsLogTag::WMS_UIEXT, "%{public}s: persistentId=%{public}d, parentId=%{public}d, "
            "usage=%{public}u, surfaceNodeId=%{public}" PRIu64 ", pid=%{public}d",
            where, persistentId, parentId, usage, surfaceNodeId, pid);

        remoteExtSessionMap_.insert(std::make_pair(remoteExtSession, token));
        extSessionInfoMap_.insert(std::make_pair(token, ExtensionWindowAbilityInfo{ persistentId, parentId, usage }));

        auto parentSession = GetSceneSession(parentId);
        if (!parentSession) {
            TLOGNI(WmsLogTag::WMS_UIEXT, "no parent session for %{public}d", persistentId);
            return;
        }

        UIExtensionTokenInfo tokenInfo;
        tokenInfo.abilityToken = token;
        tokenInfo.callingTokenId = callingTokenId;
        tokenInfo.canShowOnLockScreen = IsUIExtCanShowOnLockScreen(info.elementName, callingTokenId,
            info.extensionAbilityType);
        parentSession->AddExtensionTokenInfo(tokenInfo);
        parentSession->AddUIExtSurfaceNodeId(surfaceNodeId, persistentId);
        if (usage == UIExtensionUsage::MODAL && parentSession->GetCallingPid() != GetPid()) {
            ExtensionWindowEventInfo extensionInfo {
                .persistentId = persistentId,
                .pid = pid,
                .startModalExtensionTimeStamp = startModalExtensionTimeStamp,
            };
            if (!isConstrainedModal) {
                parentSession->AddNormalModalUIExtension(extensionInfo);
            }
        }
    };
    taskScheduler_->PostAsyncTask(task, "AddExtensionWindowStageToSCB");
}

void SceneSessionManager::RemoveExtensionWindowStageFromSCB(const sptr<ISessionStage>& sessionStage,
    const sptr<IRemoteObject>& token, bool isConstrainedModal)
{
    TLOGD(WmsLogTag::WMS_UIEXT, "in");
    auto task = [this, sessionStage, token, isConstrainedModal]() {
        if (sessionStage == nullptr || token == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "input is nullptr");
            return;
        }
        auto remoteExtSession = sessionStage->AsObject();
        if (remoteExtSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "sessionStage object is nullptr");
            return;
        }
        auto iter = remoteExtSessionMap_.find(remoteExtSession);
        if (iter == remoteExtSessionMap_.end() || iter->second != token) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "token not match");
            return;
        }

        DestroyExtensionSession(remoteExtSession, isConstrainedModal);
    };
    taskScheduler_->PostAsyncTask(task, "RemoveExtensionWindowStageFromSCB");
}

void SceneSessionManager::CalculateCombinedExtWindowFlags()
{
    // Only correct when each flag is true when active, and once a uiextension is active, the host is active
    combinedExtWindowFlags_.bitData = 0;
    for (const auto& iter: extWindowFlagsMap_) {
        combinedExtWindowFlags_.bitData |= iter.second.bitData;
    }
    specialExtWindowHasPrivacyMode_.store(combinedExtWindowFlags_.privacyModeFlag);
}

void SceneSessionManager::UpdateSpecialExtWindowFlags(int32_t persistentId, ExtensionWindowFlags flags,
    ExtensionWindowFlags actions)
{
    auto iter = extWindowFlagsMap_.find(persistentId);
    // Each flag is false when inactive, 0 means all flags are inactive
    auto oldFlags = iter != extWindowFlagsMap_.end() ? iter->second : ExtensionWindowFlags();
    ExtensionWindowFlags newFlags((flags.bitData & actions.bitData) | (oldFlags.bitData & ~actions.bitData));
    if (newFlags.bitData == 0) {
        extWindowFlagsMap_.erase(persistentId);
    } else {
        extWindowFlagsMap_[persistentId] = newFlags;
    }
    CalculateCombinedExtWindowFlags();
}

void SceneSessionManager::HideNonSecureFloatingWindows()
{
    if (systemConfig_.IsPcWindow()) {
        TLOGI(WmsLogTag::WMS_UIEXT, "PC window don't hide");
        return;
    }
    bool shouldHide = false;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& iter: sceneSessionMap_) {
            auto& session = iter.second;
            if (session && session->GetCombinedExtWindowFlags().hideNonSecureWindowsFlag) {
                shouldHide = true;
                break;
            }
        }
    }
    if (combinedExtWindowFlags_.hideNonSecureWindowsFlag) {
        TLOGI(WmsLogTag::WMS_UIEXT, "SCB UIExtension hide non-secure windows");
        shouldHide = true;
    }
    if (shouldHide == shouldHideNonSecureFloatingWindows_.load()) {
        return;
    }

    shouldHideNonSecureFloatingWindows_.store(shouldHide);
    for (const auto& [persistentId, session] : nonSystemFloatSceneSessionMap_) {
        if (session && session->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT) {
            session->NotifyForceHideChange(shouldHide);
            TLOGI(WmsLogTag::WMS_UIEXT, "name=%{public}s, persistentId=%{public}d, shouldHide=%{public}u",
                session->GetWindowName().c_str(), persistentId, shouldHide);
        }
    }
}

void SceneSessionManager::HideNonSecureSubWindows(const sptr<SceneSession>& sceneSession)
{
    // don't let sub-window show when switching secure host window to background
    if (!sceneSession->IsSessionForeground()) {
        return;
    }

    auto parentId = sceneSession->GetPersistentId();
    bool shouldHide = sceneSession->GetCombinedExtWindowFlags().hideNonSecureWindowsFlag;
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, session] : sceneSessionMap_) {
        if (!session) {
            continue;
        }
        auto sessionProperty = session->GetSessionProperty();
        if (sessionProperty->GetParentPersistentId() != parentId) {
            continue;
        }
        if (SessionHelper::IsNonSecureToUIExtension(sessionProperty->GetWindowType()) &&
            !session->IsSystemSpecificSession()) {
            session->NotifyForceHideChange(shouldHide);
            TLOGI(WmsLogTag::WMS_UIEXT, "name=%{public}s, persistentId=%{public}d, shouldHide=%{public}u",
                session->GetWindowName().c_str(), session->GetPersistentId(), shouldHide);
        }
    }
}

WSError SceneSessionManager::HandleSecureSessionShouldHide(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_UIEXT, "sceneSession is nullptr");
        return WSError::WS_ERROR_INVALID_SESSION;
    }

    HideNonSecureFloatingWindows();
    HideNonSecureSubWindows(sceneSession);
    return WSError::WS_OK;
}

void SceneSessionManager::HandleSpecialExtWindowFlagsChange(int32_t persistentId, ExtensionWindowFlags flags,
    ExtensionWindowFlags actions)
{
    UpdateSpecialExtWindowFlags(persistentId, flags, actions);
    if (actions.waterMarkFlag) {
        CheckAndNotifyWaterMarkChangedResult();
    }
    if (actions.hideNonSecureWindowsFlag) {
        HideNonSecureFloatingWindows();
    }
    if (actions.privacyModeFlag) {
        UpdatePrivateStateAndNotifyForAllScreens();
    }
}

WSError SceneSessionManager::AddOrRemoveSecureSession(int32_t persistentId, bool shouldHide)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "persistentId=%{public}d, shouldHide=%{public}u", persistentId, shouldHide);
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "HideNonSecureWindows permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    const auto callingPid = IPCSkeleton::GetCallingRealPid();
    const char* const where = __func__;
    auto task = [this, persistentId, shouldHide, callingPid, where]() {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        auto iter = sceneSessionMap_.find(persistentId);
        if (iter == sceneSessionMap_.end()) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s: Session with persistentId %{public}d not found",
                where, persistentId);
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        auto sceneSession = iter->second;
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s: sceneSession is nullptr.", where);
            return WSError::WS_ERROR_NULLPTR;
        }
        if (callingPid != sceneSession->GetCallingPid()) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "%{public}s: Permission denied", where);
            return WSError::WS_ERROR_INVALID_PERMISSION;
        }
        sceneSession->SetShouldHideNonSecureWindows(shouldHide);
        return HandleSecureSessionShouldHide(sceneSession);
    };

    taskScheduler_->PostAsyncTask(task, "AddOrRemoveSecureSession");
    return WSError::WS_OK;
}

WSError SceneSessionManager::CheckExtWindowFlagsPermission(ExtensionWindowFlags& actions) const
{
    auto ret = WSError::WS_OK;
    bool needSystemCalling = actions.hideNonSecureWindowsFlag || actions.waterMarkFlag;
    if (needSystemCalling && !SessionPermission::IsSystemCalling()) {
        actions.hideNonSecureWindowsFlag = false;
        actions.waterMarkFlag = false;
        TLOGE(WmsLogTag::WMS_UIEXT, "system calling permission denied!");
        ret = WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    auto needPrivacyWindow = actions.privacyModeFlag;
    if (needPrivacyWindow && !SessionPermission::VerifyCallingPermission("ohos.permission.PRIVACY_WINDOW")) {
        actions.privacyModeFlag = false;
        TLOGE(WmsLogTag::WMS_UIEXT, "privacy window permission denied!");
        ret = WSError::WS_ERROR_INVALID_PERMISSION;
    }
    return ret;
}

WSError SceneSessionManager::UpdateExtWindowFlags(const sptr<IRemoteObject>& token, uint32_t extWindowFlags,
    uint32_t extWindowActions)
{
    ExtensionWindowFlags actions(extWindowActions);
    auto ret = CheckExtWindowFlagsPermission(actions);
    if (ret != WSError::WS_OK) {
        return ret;
    }

    ExtensionWindowFlags flags(extWindowFlags);
    const char* const where = __func__;
    auto task = [this, token, flags, actions, where]() {
        int32_t persistentId = INVALID_SESSION_ID;
        int32_t parentId = INVALID_SESSION_ID;
        if (!GetExtensionWindowIds(token, persistentId, parentId)) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "Get UIExtension window ids by token failed");
            return WSError::WS_ERROR_INVALID_OPERATION;
        }

        TLOGNI(WmsLogTag::WMS_UIEXT, "%{public}s: parentId=%{public}d, persistentId=%{public}d, "
            "extWindowFlags=%{public}d, actions=%{public}d", where, parentId, persistentId,
            flags.bitData, actions.bitData);
        auto sceneSession = GetSceneSession(parentId);
        if (sceneSession == nullptr) {
            TLOGND(WmsLogTag::WMS_UIEXT, "%{public}s: Parent session with persistentId %{public}d not found",
                where, parentId);
            HandleSpecialExtWindowFlagsChange(persistentId, flags, actions);
            return WSError::WS_OK;
        }

        auto oldFlags = sceneSession->GetCombinedExtWindowFlags();
        sceneSession->UpdateExtWindowFlags(persistentId, flags, actions);
        auto newFlags = sceneSession->GetCombinedExtWindowFlags();
        if (oldFlags.hideNonSecureWindowsFlag != newFlags.hideNonSecureWindowsFlag) {
            HandleSecureSessionShouldHide(sceneSession);
        }
        if (oldFlags.waterMarkFlag != newFlags.waterMarkFlag) {
            CheckAndNotifyWaterMarkChangedResult();
        }
        if (oldFlags.privacyModeFlag != newFlags.privacyModeFlag) {
            UpdatePrivateStateAndNotify(parentId);
        }
        return WSError::WS_OK;
    };

    taskScheduler_->PostAsyncTask(task, "UpdateExtWindowFlags");
    return ret;
}

WSError SceneSessionManager::GetHostWindowRect(int32_t hostWindowId, Rect& rect)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "hostWindowId:%{public}d", hostWindowId);
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_UIEXT, "permission denied!");
        return WSError::WS_ERROR_NOT_SYSTEM_APP;
    }
    auto task = [this, hostWindowId, &rect]() {
        auto sceneSession = GetSceneSession(hostWindowId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "Session with persistentId %{public}d not found", hostWindowId);
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        WSRect hostRect = sceneSession->GetSessionScreenRelativeRect();
        auto currScreenFoldStatus = PcFoldScreenManager::GetInstance().GetScreenFoldStatus();
        auto needTransRect = currScreenFoldStatus != SuperFoldStatus::UNKNOWN &&
            currScreenFoldStatus != SuperFoldStatus::FOLDED && currScreenFoldStatus != SuperFoldStatus::EXPANDED;
        auto isSystemKeyboard = sceneSession->GetSessionProperty() != nullptr &&
            sceneSession->GetSessionProperty()->IsSystemKeyboard();
        if (!isSystemKeyboard && needTransRect) {
            WSRect globalRect = hostRect;
            sceneSession->TransformGlobalRectToRelativeRect(hostRect);
            TLOGI(WmsLogTag::WMS_UIEXT, "Transform globalRect: %{public}s to relativeRect: %{public}s",
                globalRect.ToString().c_str(), hostRect.ToString().c_str());
        }
        rect = {hostRect.posX_, hostRect.posY_, hostRect.width_, hostRect.height_};
        return WSError::WS_OK;
    };
    taskScheduler_->PostSyncTask(task, "GetHostWindowRect");
    return WSError::WS_OK;
}

WSError SceneSessionManager::GetHostGlobalScaledRect(int32_t hostWindowId, Rect& globalScaledRect)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "hostWindowId:%{public}d", hostWindowId);
    auto sceneSession = GetSceneSession(hostWindowId);
    if (sceneSession == nullptr) {
        TLOGNE(WmsLogTag::WMS_UIEXT, "Session with persistentId %{public}d not found", hostWindowId);
        return WSError::WS_ERROR_INVALID_SESSION;
    }
    sceneSession->GetGlobalScaledRect(globalScaledRect);
    return WSError::WS_OK;
}

int32_t SceneSessionManager::ReclaimPurgeableCleanMem()
{
#ifdef MEMMGR_WINDOW_ENABLE
    return Memory::MemMgrClient::GetInstance().ReclaimPurgeableCleanMem();
#else
    return -1;
#endif
}

bool SceneSessionManager::IsVectorSame(const std::vector<VisibleWindowNumInfo>& lastInfo,
    const std::vector<VisibleWindowNumInfo>& currentInfo)
{
    if (lastInfo.size() != currentInfo.size()) {
        TLOGE(WmsLogTag::DEFAULT, "last and current info is not Same");
        return false;
    }
    int sizeOfLastInfo = static_cast<int>(lastInfo.size());
    for (int i = 0; i < sizeOfLastInfo; i++) {
        if (lastInfo[i].displayId != currentInfo[i].displayId ||
            lastInfo[i].visibleWindowNum != currentInfo[i].visibleWindowNum) {
            TLOGE(WmsLogTag::DEFAULT, "last and current visible window num is not Same");
            return false;
        }
    }
    return true;
}

void SceneSessionManager::CacVisibleWindowNum()
{
    std::map<int32_t, sptr<SceneSession>> sceneSessionMapCopy;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        sceneSessionMapCopy = sceneSessionMap_;
    }
    std::vector<VisibleWindowNumInfo> visibleWindowNumInfo;
    bool isFullScreen = true;
    for (const auto& elem : sceneSessionMapCopy) {
        auto curSession = elem.second;
        if (curSession == nullptr) {
            continue;
        }
        bool isTargetWindow = (WindowHelper::IsMainWindow(curSession->GetWindowType()) ||
            curSession->GetWindowType() == WindowType::WINDOW_TYPE_WALLPAPER);
        if (!isTargetWindow || curSession->GetSessionState() == SessionState::STATE_BACKGROUND) {
            continue;
        }

        bool isWindowVisible = curSession->GetRSVisible();
        if (isWindowVisible) {
            auto windowMode = curSession->GetWindowMode();
            if (windowMode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY ||
                windowMode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY ||
                windowMode == WindowMode::WINDOW_MODE_FLOATING || windowMode == WindowMode::WINDOW_MODE_PIP) {
                isFullScreen = false;
            }
            int32_t displayId = static_cast<int32_t>(curSession->GetSessionProperty()->GetDisplayId());
            auto it = std::find_if(visibleWindowNumInfo.begin(), visibleWindowNumInfo.end(),
                [displayId](const VisibleWindowNumInfo& info) {
                    return (static_cast<int32_t>(info.displayId)) == displayId;
                });
            if (it == visibleWindowNumInfo.end()) {
                visibleWindowNumInfo.push_back({displayId, 1});
            } else {
                it->visibleWindowNum++;
            }
        }
    }
    if (isFullScreen) {
        std::for_each(visibleWindowNumInfo.begin(), visibleWindowNumInfo.end(),
                      [](auto& info) { info.visibleWindowNum = 1; });
    }
    std::unique_lock<std::shared_mutex> lock(lastInfoMutex_);
    if (visibleWindowNumInfo.size() > 0 && !IsVectorSame(lastInfo_, visibleWindowNumInfo)) {
        SessionManagerAgentController::GetInstance().UpdateVisibleWindowNum(visibleWindowNumInfo);
        lastInfo_ = visibleWindowNumInfo;
    }
}

void SceneSessionManager::ReportWindowProfileInfos()
{
    if (IsScreenLocked()) {
        return;
    }
    std::map<int32_t, sptr<SceneSession>> sceneSessionMapCopy;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        sceneSessionMapCopy = sceneSessionMap_;
    }

    WindowProfileSum windowProfileSum;
    int windowCount = 0;
    int visibleWindowCount = 0;
    int invisibleWindowCount = 0;
    int minimizeWindowCount = 0;
    for (const auto& [_, currSession] : sceneSessionMapCopy) {
        if (currSession == nullptr || currSession->GetSessionInfo().isSystem_ ||
            currSession->GetWindowType() != WindowType::WINDOW_TYPE_APP_MAIN_WINDOW ||
            currSession->GetVisibilityState() == WINDOW_LAYER_STATE_MAX) {
            continue;
        }
        if (currSession->GetSessionState() == SessionState::STATE_BACKGROUND) {
            minimizeWindowCount += 1;
        } else if (currSession->GetVisibilityState() == WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
            invisibleWindowCount += 1;
        } else {
            visibleWindowCount += 1;
        }
        std::string windowInfo = "";
        auto focusWindowId = GetFocusedSessionId();
        windowInfo = FillWindowProfileInfo(currSession, focusWindowId);
        if (static_cast<size_t>(windowCount) < windowProfileSum.windowInfo.size()) {
            windowProfileSum.windowInfo[windowCount] = windowInfo;
        }
        windowCount += 1;
    }
    windowProfileSum.totalWindowCount = windowCount;
    windowProfileSum.visibleWindowCount = visibleWindowCount;
    windowProfileSum.invisibleWindowCount = invisibleWindowCount;
    windowProfileSum.minimizeWindowCount = minimizeWindowCount;
    WindowInfoReporter::GetInstance().ReportWindowProfileInfo(windowProfileSum);
}

std::string SceneSessionManager::FillWindowProfileInfo(const OHOS::sptr<OHOS::Rosen::SceneSession>& currSession, int32_t focusWindowId)
{
    enum class WindowVisibleState : int32_t {
        FOCUSBLE = 0,
        FULLY_VISIBLE,
        MINIMIZED,
        TOTALLY_OCCLUSION,
        PARTLY_OCCLUSION
    };
    WindowProfileInfo windowProfileInfo;
    WSRect rect = currSession->GetSessionRect();
    std::stringstream rectStr;
    rectStr << "[" << rect.posX_ << " " << rect.posY_ << " " << rect.width_ << " " << rect.height_ << "]";
    windowProfileInfo.rect = rectStr.str();
    windowProfileInfo.zorder = static_cast<int32_t>(currSession->GetZOrder());
    windowProfileInfo.bundleName = currSession->GetSessionInfo().bundleName_;
    windowProfileInfo.windowLocatedScreen = static_cast<int32_t>(
        currSession->GetSessionProperty()->GetDisplayId());
    windowProfileInfo.windowSceneMode = static_cast<int32_t>(currSession->GetWindowMode());
    if (focusWindowId == static_cast<int32_t>(currSession->GetWindowId())) {
        windowProfileInfo.windowVisibleState = static_cast<int32_t>(WindowVisibleState::FOCUSBLE);
    } else if (currSession->GetSessionState() == SessionState::STATE_BACKGROUND) {
        windowProfileInfo.windowVisibleState = static_cast<int32_t>(WindowVisibleState::MINIMIZED);
    } else if (currSession->GetVisibilityState() == WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
        windowProfileInfo.windowVisibleState = static_cast<int32_t>(WindowVisibleState::TOTALLY_OCCLUSION);
    } else if (currSession->GetVisibilityState() == WINDOW_VISIBILITY_STATE_NO_OCCLUSION) {
        windowProfileInfo.windowVisibleState = static_cast<int32_t>(WindowVisibleState::FULLY_VISIBLE);
    } else {
        windowProfileInfo.windowVisibleState = static_cast<int32_t>(WindowVisibleState::PARTLY_OCCLUSION);
    }
    std::stringstream windowInfoStr;
    windowInfoStr << windowProfileInfo.bundleName << ", " <<
        std::to_string(windowProfileInfo.windowVisibleState) << ", "<<
        std::to_string(windowProfileInfo.windowLocatedScreen) << ", " <<
        std::to_string(windowProfileInfo.windowSceneMode) << ", " <<
        std::to_string(windowProfileInfo.zorder) << ", " <<
        windowProfileInfo.rect << "; ";
    return windowInfoStr.str();
}

int32_t SceneSessionManager::GetCustomDecorHeight(int32_t persistentId)
{
    int32_t height = 0;
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_DECOR, "session is invalid, id: %{public}d", persistentId);
        return 0;
    }
    height = sceneSession->GetCustomDecorHeight();
    TLOGD(WmsLogTag::WMS_DECOR, "decor height: %{public}d", height);
    return height;
}

void SceneSessionManager::RemoveFailRecoveredSession()
{
    for (const auto persistentId : failRecoveredPersistentIdSet_) {
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGE(WmsLogTag::WMS_RECOVER, "Session is nullptr, persistentId=%{public}d", persistentId);
            continue;
        }
        if (!sceneSession->IsRecovered()) {
            TLOGW(WmsLogTag::WMS_RECOVER, "not recovered session persistentId=%{public}d", persistentId);
            continue;
        }
        TLOGI(WmsLogTag::WMS_RECOVER, "remove recover failed persistentId=%{public}d", persistentId);
        ExceptionInfo exceptionInfo;
        exceptionInfo.needRemoveSession = true;
        sceneSession->NotifySessionExceptionInner(SetAbilitySessionInfo(sceneSession), exceptionInfo);
    }
    failRecoveredPersistentIdSet_.clear();
}

std::shared_ptr<SkRegion> SceneSessionManager::GetDisplayRegion(DisplayId displayId)
{
    if (displayRegionMap_.find(displayId) != displayRegionMap_.end()) {
        return std::make_shared<SkRegion>(displayRegionMap_[displayId]->getBounds());
    }
    TLOGI(WmsLogTag::WMS_MAIN, "can not find display info from mem, sync dispslay region from dms.");
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(displayId);
    if (display == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "get display object failed of display: %{public}" PRIu64, displayId);
        return nullptr;
    }
    auto displayInfo = display->GetDisplayInfo();
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "get display info failed of display: %{public}" PRIu64, displayId);
        return nullptr;
    }
    int32_t displayWidth = displayInfo->GetWidth();
    int32_t displayHeight = displayInfo->GetHeight();
    if (displayWidth == 0 || displayHeight == 0) {
        TLOGE(WmsLogTag::WMS_MAIN, "invalid display size of display: %{public}" PRIu64, displayId);
        return nullptr;
    }
    if (PcFoldScreenManager::GetInstance().IsHalfFolded(displayId)) {
        const auto& [defaultDisplayRect, virtualDisplayRect, foldCreaseRect] =
            PcFoldScreenManager::GetInstance().GetDisplayRects();
        displayHeight = virtualDisplayRect.posY_ + virtualDisplayRect.height_;
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "update display height in pc fold");
    }
    SkIRect rect {.fLeft = 0, .fTop = 0, .fRight = displayWidth, .fBottom = displayHeight};
    auto region = std::make_shared<SkRegion>(rect);
    displayRegionMap_[displayId] = region;
    TLOGI(WmsLogTag::WMS_MAIN, "update display region to w=%{public}d, h=%{public}d", displayWidth, displayHeight);
    return std::make_shared<SkRegion>(rect);
}

void SceneSessionManager::UpdateDisplayRegion(const sptr<DisplayInfo>& displayInfo)
{
    if (displayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "update display region failed, displayInfo is nullptr.");
        return;
    }
    auto displayId = displayInfo->GetDisplayId();
    int32_t displayWidth = displayInfo->GetWidth();
    int32_t displayHeight = displayInfo->GetHeight();
    if (displayWidth == 0 || displayHeight == 0) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "invalid display size of display: %{public}" PRIu64, displayId);
        return;
    }
    if (PcFoldScreenManager::GetInstance().IsHalfFolded(displayId)) {
        const auto& [defaultDisplayRect, virtualDisplayRect, foldCreaseRect] =
            PcFoldScreenManager::GetInstance().GetDisplayRects();
        displayHeight = virtualDisplayRect.posY_ + virtualDisplayRect.height_;
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "update display height in pc fold");
    }
    SkIRect rect {.fLeft = 0, .fTop = 0, .fRight = displayWidth, .fBottom = displayHeight};
    auto region = std::make_shared<SkRegion>(rect);
    displayRegionMap_[displayId] = region;
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "update display region to w: %{public}d, h: %{public}d",
        displayWidth, displayHeight);
}

bool SceneSessionManager::GetDisplaySizeById(DisplayId displayId, int32_t& displayWidth, int32_t& displayHeight)
{
    auto region = GetDisplayRegion(displayId);
    if (region == nullptr) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "failed, displayId:%{public}" PRIu64, displayId);
        return false;
    }
    const SkIRect& rect = region->getBounds();
    displayWidth = rect.fRight;
    displayHeight = rect.fBottom;
    return true;
}

void SceneSessionManager::GetAllSceneSessionForAccessibility(std::vector<sptr<SceneSession>>& sceneSessionList)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession == nullptr) {
            TLOGW(WmsLogTag::WMS_ATTRIBUTE, "session is null");
            continue;
        }
        if (Session::IsScbCoreEnabled()) {
            if (!sceneSession->IsVisibleForAccessibility()) {
                TLOGD(WmsLogTag::WMS_ATTRIBUTE, "unvisible: isSys=%{public}d, inWid=%{public}d, bundle=%{public}s, "
                    "zOrder=%{public}u, displayId=%{public}" PRIu64 ", nodeId=%{public}d, "
                    "sysTouchable=%{public}d, interactive=%{public}d, visibleForeground=%{public}d",
                    sceneSession->GetSessionInfo().isSystem_, static_cast<int32_t>(sceneSession->GetPersistentId()),
                    sceneSession->GetSessionInfo().bundleName_.c_str(), sceneSession->GetZOrder(),
                    sceneSession->GetSessionProperty()->GetDisplayId(), sceneSession->GetUINodeId(),
                    sceneSession->GetSystemTouchable(), sceneSession->GetForegroundInteractiveStatus(),
                    sceneSession->IsVisibleForeground());
                continue;
            }
        } else {
            if (!sceneSession->IsVisibleForAccessibility() || !IsSessionVisible(sceneSession)) {
                TLOGD(WmsLogTag::WMS_ATTRIBUTE, "unvisible2: isSys=%{public}d, inWid=%{public}d, bundle=%{public}s, "
                    "zOrder=%{public}u, displayId=%{public}" PRIu64 ", nodeId=%{public}d, "
                    "sysTouchable=%{public}d, interactive=%{public}d, visibleForeground=%{public}d",
                    sceneSession->GetSessionInfo().isSystem_, static_cast<int32_t>(sceneSession->GetPersistentId()),
                    sceneSession->GetSessionInfo().bundleName_.c_str(), sceneSession->GetZOrder(),
                    sceneSession->GetSessionProperty()->GetDisplayId(), sceneSession->GetUINodeId(),
                    sceneSession->GetSystemTouchable(), sceneSession->GetForegroundInteractiveStatus(),
                    sceneSession->IsVisibleForeground());
                continue;
            }
        }
        if (sceneSession->GetSessionInfo().bundleName_.find("SCBGestureBack") != std::string::npos ||
            sceneSession->GetSessionInfo().bundleName_.find("SCBGestureNavBar") != std::string::npos ||
            sceneSession->GetSessionInfo().bundleName_.find("SCBGestureTopBar") != std::string::npos) {
            continue;
        }
        if (sceneSession->GetSessionInfo().bundleName_.find("SCBDragScale") != std::string::npos) {
            continue;
        }
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "isSys=%{public}d, inWid=%{public}d, bundle=%{public}s, zOrder=%{public}u",
            sceneSession->GetSessionInfo().isSystem_, static_cast<int32_t>(sceneSession->GetPersistentId()),
            sceneSession->GetSessionInfo().bundleName_.c_str(), sceneSession->GetZOrder());
        sceneSessionList.push_back(sceneSession);
    }
}

void SceneSessionManager::FillAccessibilityInfo(std::vector<sptr<SceneSession>>& sceneSessionList,
    std::vector<sptr<AccessibilityWindowInfo>>& accessibilityInfo)
{
    for (const auto& sceneSession : sceneSessionList) {
        if (!FillWindowInfo(accessibilityInfo, sceneSession)) {
            TLOGW(WmsLogTag::WMS_MAIN, "fill accessibilityInfo failed");
        }
    }
}

void SceneSessionManager::FilterSceneSessionCovered(std::vector<sptr<SceneSession>>& sceneSessionList)
{
    std::sort(sceneSessionList.begin(), sceneSessionList.end(), [](sptr<SceneSession> a, sptr<SceneSession> b) {
        return a->GetZOrder() > b->GetZOrder();
    });
    std::vector<sptr<SceneSession>> result;
    std::unordered_map<DisplayId, std::shared_ptr<SkRegion>> unaccountedSpaceMap;
    for (const auto& sceneSession : sceneSessionList) {
        if (sceneSession == nullptr) {
            TLOGE(WmsLogTag::WMS_MAIN, "invalid scene session");
            continue;
        }
        std::shared_ptr<SkRegion> unaccountedSpace = nullptr;
        auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
        if (unaccountedSpaceMap.find(displayId) != unaccountedSpaceMap.end()) {
            unaccountedSpace = unaccountedSpaceMap[displayId];
        } else {
            unaccountedSpace = GetDisplayRegion(displayId);
            if (unaccountedSpace == nullptr) {
                TLOGE(WmsLogTag::WMS_MAIN, "get display region of display: %{public}" PRIu64, displayId);
                continue;
            }
            unaccountedSpaceMap[displayId] = unaccountedSpace;
        }
        if (SubtractIntersectArea(unaccountedSpace, sceneSession)) {
            result.push_back(sceneSession);
        }
        if (unaccountedSpace->isEmpty()) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "break after: inWid=%{public}d, zOrder=%{public}u",
                static_cast<int32_t>(sceneSession->GetPersistentId()), sceneSession->GetZOrder());
            break;
        }
    }
    sceneSessionList = result;
}

bool SceneSessionManager::SubtractIntersectArea(std::shared_ptr<SkRegion>& unaccountedSpace,
    const sptr<SceneSession>& sceneSession)
{
    if (unaccountedSpace == nullptr || sceneSession == nullptr) {
        TLOGW(WmsLogTag::WMS_ATTRIBUTE, "space or session is null");
        return false;
    }
    auto hotAreas = sceneSession->GetTouchHotAreas();
    WSRect wsRect = sceneSession->GetSessionRect();
    for (auto& rect : hotAreas) {
        TLOGD(WmsLogTag::WMS_ATTRIBUTE, "id=%{public}d, rect=%{public}s, hotArea=%{public}s",
            static_cast<int32_t>(sceneSession->GetPersistentId()), wsRect.ToString().c_str(), rect.ToString().c_str());
        if (rect != Rect::EMPTY_RECT) {
            rect.posX_ += wsRect.posX_;
            rect.posY_ += wsRect.posY_;
        }
    }
    if (hotAreas.empty()) {
        hotAreas.push_back({.posX_ = wsRect.posX_, .posY_ = wsRect.posY_,
                            .width_ = wsRect.width_, .height_ = wsRect.height_});
    }
    bool hasIntersectArea = false;
    for (const auto& rect : hotAreas) {
        SkIRect windowBounds {.fLeft = rect.posX_, .fTop = rect.posY_,
                              .fRight = rect.posX_ + rect.width_, .fBottom = rect.posY_ + rect.height_};
        SkRegion windowRegion(windowBounds);
        if (unaccountedSpace->quickReject(windowRegion)) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "quick reject: inWid=%{public}d, "
                "bounds=[l=%{public}d, t=%{public}d, r=%{public}d, b=%{public}d]",
                static_cast<int32_t>(sceneSession->GetPersistentId()),
                windowBounds.fLeft, windowBounds.fTop, windowBounds.fRight, windowBounds.fBottom);
            continue;
        }
        if (!unaccountedSpace->intersects(windowRegion)) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "no intersects: inWid=%{public}d, "
                "bounds=[l=%{public}d, t=%{public}d, r=%{public}d, b=%{public}d]",
                static_cast<int32_t>(sceneSession->GetPersistentId()),
                windowBounds.fLeft, windowBounds.fTop, windowBounds.fRight, windowBounds.fBottom);
            continue;
        }
        hasIntersectArea = true;
        unaccountedSpace->op(windowRegion, SkRegion::Op::kDifference_Op);
        if (unaccountedSpace->isEmpty()) {
            TLOGD(WmsLogTag::WMS_ATTRIBUTE, "break hot area: inWid=%{public}d, "
                "bounds=[l=%{public}d, t=%{public}d, r=%{public}d, b=%{public}d]",
                static_cast<int32_t>(sceneSession->GetPersistentId()),
                windowBounds.fLeft, windowBounds.fTop, windowBounds.fRight, windowBounds.fBottom);
            break;
        }
    }
    return hasIntersectArea;
}

void SceneSessionManager::NotifyAllAccessibilityInfo()
{
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:NotifyAllAccessibilityInfo");
    if (isUserBackground_) {
        TLOGD(WmsLogTag::WMS_MULTI_USER, "The user is in the background");
        return;
    }
    std::vector<sptr<SceneSession>> sceneSessionList;
    GetAllSceneSessionForAccessibility(sceneSessionList);
    FilterSceneSessionCovered(sceneSessionList);

    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;
    FillAccessibilityInfo(sceneSessionList, accessibilityInfo);

    for (const auto& item : accessibilityInfo) {
        TLOGD(WmsLogTag::WMS_MAIN, "notify accessibilityWindow wid=%{public}d, inWid=%{public}d, "
            "bundle=%{public}s,bounds=(x=%{public}d, y=%{public}d, w=%{public}d, h=%{public}d)",
            item->wid_, item->innerWid_, item->bundleName_.c_str(),
            item->windowRect_.posX_, item->windowRect_.posY_, item->windowRect_.width_, item->windowRect_.height_);
        for (const auto& rect : item->touchHotAreas_) {
            TLOGD(WmsLogTag::WMS_MAIN, "window touch hot areas rect[x=%{public}d,y=%{public}d,"
                "w=%{public}d,h=%{public}d]", rect.posX_, rect.posY_, rect.width_, rect.height_);
        }
    }

    SessionManagerAgentController::GetInstance().NotifyAccessibilityWindowInfo(accessibilityInfo,
        WindowUpdateType::WINDOW_UPDATE_ALL);
}

WindowStatus SceneSessionManager::GetWindowStatus(WindowMode mode, SessionState sessionState,
    const sptr<WindowSessionProperty>& property)
{
    auto windowStatus = WindowStatus::WINDOW_STATUS_UNDEFINED;
    if (property == nullptr) {
        return windowStatus;
    }
    if (mode == WindowMode::WINDOW_MODE_FLOATING) {
        windowStatus = WindowStatus::WINDOW_STATUS_FLOATING;
        if (property->GetMaximizeMode() == MaximizeMode::MODE_AVOID_SYSTEM_BAR) { // maximize floating
            windowStatus = WindowStatus::WINDOW_STATUS_MAXIMIZE;
        }
    } else if (mode == WindowMode::WINDOW_MODE_SPLIT_PRIMARY || mode == WindowMode::WINDOW_MODE_SPLIT_SECONDARY) {
        windowStatus = WindowStatus::WINDOW_STATUS_SPLITSCREEN;
    } else if (mode == WindowMode::WINDOW_MODE_FULLSCREEN) {
        windowStatus = WindowStatus::WINDOW_STATUS_FULLSCREEN;
    } else if (sessionState != SessionState::STATE_FOREGROUND && sessionState != SessionState::STATE_ACTIVE) {
        windowStatus = WindowStatus::WINDOW_STATUS_MINIMIZE;
    }
    return windowStatus;
}

WMError SceneSessionManager::GetCallingWindowWindowStatus(uint32_t callingWindowId, WindowStatus& windowStatus)
{
    if (!SessionPermission::IsStartedByInputMethod()) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "permission is not allowed, callingWindowId: %{public}u", callingWindowId);
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    auto sceneSession = GetSceneSession(callingWindowId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "sceneSession is null, callingWindowId: %{public}u", callingWindowId);
        return WMError::WM_ERROR_NULLPTR;
    }
    auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    auto focusedSessionId = GetFocusedSessionId(displayId);
    TLOGD(WmsLogTag::WMS_KEYBOARD, "callingWindowId: %{public}u, focusedSessionId: %{public}d, windowType: %{public}d",
        callingWindowId, focusedSessionId, sceneSession->GetWindowType());
    auto callingSession = GetSceneSession(callingWindowId);
    if (callingSession == nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "callingsSession is null, focusedSessionId: %{public}d", focusedSessionId);
        callingSession = GetSceneSession(focusedSessionId);
        if (callingSession == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "callingsSession obtained through focusedSession fail");
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
    }
    int32_t pid = callingSession->GetCallingPid();
    if (!SessionPermission::IsKeyboardCallingProcess(pid, callingWindowId)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Is not keyboard calling process, callingWindowId: %{public}u", callingWindowId);
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (callingSession->IsSystemSession()) {
        windowStatus = WindowStatus::WINDOW_STATUS_FULLSCREEN;
    } else {
        windowStatus = GetWindowStatus(callingSession->GetWindowMode(), callingSession->GetSessionState(),
            callingSession->GetSessionProperty());
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Get WindowStatus callingWindowId: %{public}d windowstatus: %{public}d",
        callingWindowId, windowStatus);
    return WMError::WM_OK;
}

WMError SceneSessionManager::GetCallingWindowRect(uint32_t callingWindowId, Rect& rect)
{
    if (!SessionPermission::IsStartedByInputMethod()) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "permission is not allowed, callingWindowId: %{public}u", callingWindowId);
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    auto sceneSession = GetSceneSession(callingWindowId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "sceneSession is null, callingWindowId: %{public}u", callingWindowId);
        return WMError::WM_ERROR_NULLPTR;
    }
    auto displayId = sceneSession->GetSessionProperty()->GetDisplayId();
    auto focusedSessionId = GetFocusedSessionId(displayId);
    TLOGD(WmsLogTag::WMS_KEYBOARD, "callingWindowId: %{public}u, windowType: %{public}d",
        callingWindowId, sceneSession->GetWindowType());
    auto callingSession = GetSceneSession(callingWindowId);
    if (callingSession == nullptr) {
        TLOGI(WmsLogTag::WMS_KEYBOARD, "callingsSession is null, focusedSessionId: %{public}d", focusedSessionId);
        callingSession = GetSceneSession(focusedSessionId);
        if (callingSession == nullptr) {
            TLOGE(WmsLogTag::WMS_KEYBOARD, "callingsSession obtained through focusedSession fail");
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
    }
    int32_t pid = callingSession->GetCallingPid();
    if (!SessionPermission::IsKeyboardCallingProcess(pid, callingWindowId)) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Is not keyboard calling process, callingWindowId: %{public}u", callingWindowId);
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    WSRect sessionRect = callingSession->GetSessionRect();
    rect = {sessionRect.posX_, sessionRect.posY_, sessionRect.width_, sessionRect.height_};
    TLOGI(WmsLogTag::WMS_KEYBOARD, "Get Rect callingWindowId: %{public}u, x: %{public}d, y: %{public}d, "
        "height: %{public}u, width: %{public}u", callingWindowId, rect.posX_, rect.posY_, rect.width_, rect.height_);
    return WMError::WM_OK;
}

WMError SceneSessionManager::GetWindowModeType(WindowModeType& windowModeType)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::DEFAULT, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    windowModeType = CheckWindowModeType();
    return WMError::WM_OK;
}

WMError SceneSessionManager::GetWindowStyleType(WindowStyleType& windowStyleType)
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (systemConfig_.IsPcWindow()) {
        windowStyleType = WindowStyleType::WINDOW_STYLE_FREE_MULTI_WINDOW;
        return WMError::WM_OK;
    }
    windowStyleType = systemConfig_.freeMultiWindowSupport_ && systemConfig_.freeMultiWindowEnable_ ?
        WindowStyleType::WINDOW_STYLE_FREE_MULTI_WINDOW : WindowStyleType::WINDOW_STYLE_DEFAULT;
    return WMError::WM_OK;
}

void SceneSessionManager::CheckSceneZOrder()
{
    auto task = [this]() {
        AnomalyDetection::SceneZOrderCheckProcess();
    };
    taskScheduler_->PostAsyncTask(task, "CheckSceneZOrder");
}

WSError SceneSessionManager::NotifyEnterRecentTask(bool enterRecent)
{
    TLOGD(WmsLogTag::WMS_IMMS, "enterRecent %{public}u", enterRecent);
    enterRecent_.store(enterRecent);
    SetSystemAnimatedScenes(enterRecent ?
        SystemAnimatedSceneType::SCENE_ENTER_RECENTS : SystemAnimatedSceneType::SCENE_EXIT_RECENTS);
    auto task = [this] {
        for (auto persistentId : gestureBackEnableWindowIdSet_) {
            auto sceneSession = GetSceneSession(persistentId);
            if (sceneSession == nullptr || !IsSessionVisible(sceneSession)) {
                continue;
            }
            UpdateGestureBackEnabled(persistentId);
        }
    };
    taskScheduler_->PostAsyncTask(task, "UpdateGestureBackEnabledTask");
    return WSError::WS_OK;
}

void SceneSessionManager::NotifySCBRecentStateChange(bool isRecent)
{
    TLOGI(WmsLogTag::WMS_MAIN, "isRecent update %{public}u", isRecent);
    scbIsRecent_.store(isRecent);
}

WMError SceneSessionManager::GetMainWindowInfos(int32_t topNum, std::vector<MainWindowInfo>& topNInfo)
{
    if (!(SessionPermission::IsSACalling() || SessionPermission::IsStartByHdcd())) {
        TLOGE(WmsLogTag::WMS_MAIN, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }

    if (!topNInfo.empty() || (topNum <= 0)) {
        return WMError::WM_ERROR_INVALID_PARAM;
    }

    TLOGD(WmsLogTag::WMS_MAIN, "topNum: %{public}d", topNum);
    auto func = [this, &topNum, &topNInfo](sptr<SceneSession> session) {
        if (session == nullptr) {
            return false;
        }

        if (topNum == 0) {
            return true;
        }

        if (!WindowHelper::IsMainWindow(session->GetWindowType()) || !IsSessionVisibleForeground(session)) {
            TLOGND(WmsLogTag::WMS_MAIN, "not main window %{public}d", session->GetWindowType());
            return false;
        }

        MainWindowInfo info;
        info.pid_ = session->GetCallingPid();
        info.bundleName_ = session->GetSessionInfo().bundleName_;
        topNInfo.push_back(info);
        topNum--;
        TLOGNE(WmsLogTag::WMS_MAIN, "topnNum: %{public}d, pid: %{public}d, bundleName: %{public}s",
            topNum, info.pid_, info.bundleName_.c_str());
        return false;
    };
    TraverseSessionTree(func, true);

    return WMError::WM_OK;
}

WMError SceneSessionManager::GetCallingWindowInfo(CallingWindowInfo& callingWindowInfo)
{
    int32_t curUserId = GetUserIdByUid(getuid());
    if (curUserId != callingWindowInfo.userId_) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "Target user not exists, targetUserId: %{public}d, curUserId: %{public}d, id: %{public}d",
            callingWindowInfo.userId_, curUserId, callingWindowInfo.windowId_);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto sceneSession = GetSceneSession(callingWindowInfo.windowId_);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "sceneSession is nullptr, id: %{public}d", callingWindowInfo.windowId_);
        return WMError::WM_ERROR_NULLPTR;
    }
    callingWindowInfo.callingPid_ = sceneSession->GetCallingPid();
    callingWindowInfo.displayId_ = sceneSession->GetSessionProperty()->GetDisplayId();
    if (sceneSession->IsPcFoldDevice() && PcFoldScreenManager::GetInstance().IsHalfFolded(callingWindowInfo.displayId_)) {
        callingWindowInfo.displayId_ = sceneSession->GetClientDisplayId();
    }
    TLOGI(WmsLogTag::WMS_KEYBOARD, "info: [id: %{public}d, pid: %{public}d, "
        "displayId: %{public}" PRIu64" , userId: %{public}d]", callingWindowInfo.windowId_,
        callingWindowInfo.callingPid_, callingWindowInfo.displayId_, callingWindowInfo.userId_);
    return WMError::WM_OK;
}

void SceneSessionManager::NotifyDisplayIdChanged(int32_t persistentId, uint64_t displayId)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (!sceneSession) {
        TLOGE(WmsLogTag::WMS_KEYBOARD, "session is nullptr");
        return;
    }
    // Find keyboard session.
    const auto& keyboardSessionVec = GetSceneSessionVectorByType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    for (const auto& keyboardSession : keyboardSessionVec) {
        if (!keyboardSession) {
            continue;
        }
        TLOGD(WmsLogTag::WMS_KEYBOARD, "isSystemKeyboard: %{public}d, callingSessionId: %{public}d",
            keyboardSession->IsSystemKeyboard(), keyboardSession->GetCallingSessionId());
        if (!keyboardSession->IsSystemKeyboard() &&
            static_cast<int32_t>(keyboardSession->GetCallingSessionId()) == persistentId) {
            CallingWindowInfo callingWindowInfo(persistentId, sceneSession->GetCallingPid(), displayId, GetUserIdByUid(getuid()));
            SessionManagerAgentController::GetInstance().NotifyCallingWindowDisplayChanged(callingWindowInfo);
            break;
        }
    }
    return;
}

WMError SceneSessionManager::GetWindowIdsByCoordinate(DisplayId displayId, int32_t windowNumber,
    int32_t x, int32_t y, std::vector<int32_t>& windowIds)
{
    TLOGD(WmsLogTag::DEFAULT, "displayId %{public}" PRIu64 " windowNumber %{public}d x %{public}d y %{public}d",
          displayId, windowNumber, x, y);
    if (displayId == DISPLAY_ID_INVALID) {
        TLOGE(WmsLogTag::DEFAULT, "displayId is invalid");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    bool findAllWindow = windowNumber <= 0;
    std::string callerBundleName = SessionPermission::GetCallingBundleName();
    ChangeWindowRectYInVirtualDisplay(displayId, y);
    bool checkPoint = (x >= 0 && y >= 0);
    auto func = [displayId, callerBundleName = std::move(callerBundleName), checkPoint, x, y,
        findAllWindow, &windowNumber, &windowIds](const sptr<SceneSession>& session) {
        if (session == nullptr) {
            return false;
        }
        if (!findAllWindow && windowNumber == 0) {
            return true;
        }
        bool isSameBundleName = session->GetSessionInfo().bundleName_ == callerBundleName;
        bool isSameDisplayId = session->GetSessionProperty()->GetDisplayId() == displayId;
        bool isRsVisible = session->GetRSVisible();
        WSRect windowRect = session->GetSessionRect();
        bool isPointInWindowRect = SessionHelper::IsPointInRect(x, y, SessionHelper::TransferToRect(windowRect));
        TLOGND(WmsLogTag::DEFAULT, "persistentId %{public}d bundleName %{public}s displayId %{public}" PRIu64
               " isRsVisible %{public}d checkPoint %{public}d isPointInWindowRect %{public}d",
               session->GetPersistentId(), session->GetSessionInfo().bundleName_.c_str(),
               session->GetSessionProperty()->GetDisplayId(), isRsVisible, checkPoint, isPointInWindowRect);
        if (!isSameBundleName || !isSameDisplayId || !isRsVisible || (checkPoint && !isPointInWindowRect)) {
            return false;
        }
        windowIds.emplace_back(session->GetPersistentId());
        windowNumber--;
        return false;
    };
    return taskScheduler_->PostSyncTask([this, func = std::move(func)] {
        TraverseSessionTree(func, true);
        return WMError::WM_OK;
    }, __func__);
}

void SceneSessionManager::ChangeWindowRectYInVirtualDisplay(DisplayId& displayId, int32_t& y)
{
    if (displayId != VIRTUAL_DISPLAY_ID) {
        TLOGI(WmsLogTag::WMS_LAYOUT_PC, "This is not VIRTUAL_DISPLAY_ID");
        return;
    }
    auto defaultScreenDisplay = DisplayManager::GetInstance().GetDisplayById(DEFAULT_DISPLAY_ID);
    if (defaultScreenDisplay == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "get display object failed of defaultScreenDisplay");
        return;
    }
    auto defaultScreenDisplayInfo = defaultScreenDisplay->GetDisplayInfo();
    if (defaultScreenDisplayInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_LAYOUT_PC, "get display info failed of defaultScreenDisplay");
        return;
    }
    int32_t defaultScreenPhysicalHeight = defaultScreenDisplayInfo->GetPhysicalHeight();
    auto screenDisplay = DisplayManager::GetInstance().GetDisplayById(displayId);
    int32_t screenHightByDisplayId = defaultScreenPhysicalHeight;
    if (screenDisplay != nullptr) {
        if (screenDisplay->GetDisplayInfo() != nullptr) {
            screenHightByDisplayId = screenDisplay->GetDisplayInfo()->GetHeight();
        }
    }
    TLOGI(WmsLogTag::WMS_LAYOUT_PC, "defaultScreenPhysicalHeight %{public}d screenHightByDisplayId %{public}d",
        defaultScreenPhysicalHeight, screenHightByDisplayId);
    if (displayId == VIRTUAL_DISPLAY_ID) {
        displayId = DEFAULT_DISPLAY_ID;
        y = y + defaultScreenPhysicalHeight - screenHightByDisplayId;
    }
}

WMError SceneSessionManager::GetAllMainWindowInfos(std::vector<MainWindowInfo>& infos) const
{
    if (!infos.empty()) {
        TLOGE(WmsLogTag::WMS_MAIN, "Input param invalid, infos must be empty.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_MAIN, "Get all mainWindow infos failed, only support SA calling.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& iter : sceneSessionMap_) {
        auto& session = iter.second;
        if (session == nullptr || !WindowHelper::IsMainWindow(session->GetWindowType())) {
            continue;
        }
        MainWindowInfo info;
        auto abilityInfo = session->GetSessionInfo().abilityInfo;
        info.pid_ = session->GetCallingPid();
        info.bundleName_ = session->GetSessionInfo().bundleName_;
        info.persistentId_ = session->GetPersistentId();
        if (IsAtomicServiceFreeInstall(session->GetSessionInfo())) {
            TLOGI(WmsLogTag::WMS_MAIN, "id:%{public}d is atomicServiceInstall", session->GetPersistentId());
            info.bundleType_ = static_cast<int32_t>(AppExecFwk::BundleType::ATOMIC_SERVICE);
            infos.push_back(info);
        } else if (abilityInfo != nullptr) {
            info.bundleType_ = static_cast<int32_t>(abilityInfo->applicationInfo.bundleType);
            infos.push_back(info);
            TLOGD(WmsLogTag::WMS_MAIN, "Get mainWindow info: Session id:%{public}d, "
                "bundleName:%{public}s, bundleType:%{public}d", session->GetPersistentId(),
                info.bundleName_.c_str(), info.bundleType_);
        }
    }
    return WMError::WM_OK;
}

WMError SceneSessionManager::GetMainWindowInfoByToken(const sptr<IRemoteObject>& abilityToken,
    MainWindowInfo& windowInfo) const
{
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_MAIN, "permission denied.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    auto task = [this, abilityToken, &windowInfo] {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, session] : sceneSessionMap_) {
            if (session && session->GetAbilityToken() == abilityToken) {
                windowInfo.pid_ = session->GetCallingPid();
                windowInfo.bundleName_ = session->GetSessionInfo().bundleName_;
                windowInfo.persistentId_ = session->GetPersistentId();
                windowInfo.displayId_ = session->GetDisplayId();
                TLOGI(WmsLogTag::WMS_MAIN, "find success id:%{public}d, displayId:%{public}" PRIu64"",
                    session->GetPersistentId(), session->GetDisplayId());
                return WMError::WM_OK;
            }
        }
        TLOGW(WmsLogTag::WMS_MAIN, "find failed.");
        return WMError::WM_ERROR_INVALID_PARAM;
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

WMError SceneSessionManager::GetAllMainWindowInfo(std::vector<sptr<MainWindowInfo>>& infos)
{
    if (!systemConfig_.IsPcWindow()) {
        TLOGE(WmsLogTag::WMS_LIFE, "device not support");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (!SessionPermission::VerifyCallingPermission("ohos.permission.CUSTOM_SCREEN_CAPTURE")) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, session] : sceneSessionMap_) {
        if (session == nullptr || session->GetSessionState() == SessionState::STATE_DISCONNECT) {
            TLOGW(WmsLogTag::WMS_LIFE, "session is nullptr or sessionState is disconnect");
            continue;
        }
        if (WindowHelper::IsMainWindow(session->GetWindowType())) {
            auto mainWindowInfo = sptr<MainWindowInfo>::MakeSptr();
            mainWindowInfo->displayId_ = session->GetSessionProperty()->GetDisplayId();
            mainWindowInfo->persistentId_ = session->GetPersistentId();
            mainWindowInfo->showing_ = (session->GetSessionState() == SessionState::STATE_ACTIVE ||
                session->GetSessionState() == SessionState::STATE_FOREGROUND);
            mainWindowInfo->label_ = session->GetSessionLabel();
            infos.emplace_back(mainWindowInfo);
        }
    }
    return WMError::WM_OK;
}
 
WMError SceneSessionManager::GetMainWindowSnapshot(const std::vector<int32_t>& windowIds,
    const WindowSnapshotConfiguration& config, const sptr<IRemoteObject>& callback)
{
    if (!systemConfig_.IsPcWindow()) {
        TLOGE(WmsLogTag::WMS_LIFE, "device not support");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    WMError ret = CheckWindowIds(windowIds, callback);
    if (WMError::WM_OK != ret) {
        return ret;
    }
    const char* const where = __func__;
    taskScheduler_->PostAsyncTask([this, callback, config, windowIds, where] {
        std::vector<std::shared_ptr<Media::PixelMap>> pixelMaps;
        WMError errCode = WMError::WM_OK;
        for (const auto windowId : windowIds) {
            auto sceneSession = GetSceneSession(windowId);
            if (sceneSession && !WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
                TLOGNW(WmsLogTag::WMS_LIFE, "window: %{public}d is not mainWindow.", windowId);
                pixelMaps.emplace_back(nullptr);
                continue;
            }
            auto pixelMap = SceneSessionManager::GetSessionSnapshotPixelMap(
                windowId, 1.0f, SnapshotNodeType::LEASH_NODE, !config.useCache);
            if (config.useCache && !pixelMap) {
                pixelMap = SceneSessionManager::GetSessionSnapshotPixelMap(
                    windowId, 1.0f, SnapshotNodeType::LEASH_NODE, true);
            }
            if (!pixelMap) {
                TLOGNW(WmsLogTag::WMS_LIFE, "Get snapshot failed");
            }
            pixelMaps.emplace_back(pixelMap);
        }
        TLOGNI(WmsLogTag::WMS_LIFE, "windowIds size: %{public}zu, pixelMaps size: %{public}zu",
            windowIds.size(), pixelMaps.size());
        sptr<IGetSnapshotCallback> getSnapshotCallback = iface_cast<IGetSnapshotCallback>(callback);
        if (getSnapshotCallback) {
            getSnapshotCallback->OnReceived(errCode, pixelMaps);
        } else {
            TLOGNE(WmsLogTag::WMS_LIFE, "getSnapshotCallback is null");
        }
    }, __func__);
    return ret;
}
 
WMError SceneSessionManager::CheckWindowIds(
    const std::vector<int32_t>& windowIds, const sptr<IRemoteObject>& callback)
{
    if (!SessionPermission::VerifyCallingPermission("ohos.permission.CUSTOM_SCREEN_CAPTURE")) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (windowIds.empty()) {
        TLOGE(WmsLogTag::WMS_LIFE, "Input param invalid, windowIds empty.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (callback == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Input param invalid, callback is null.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (windowIds.size() > MAX_SESSION_LIMIT_ALL_APP) {
        TLOGE(WmsLogTag::WMS_LIFE, "Input param invalid, windowIds is too long.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    std::unordered_set<int32_t> uniqueIds(windowIds.begin(), windowIds.end());
    if (uniqueIds.size() != windowIds.size()) {
        TLOGE(WmsLogTag::WMS_LIFE, "Input param invalid, duplicate windowId exist.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    bool sessionIdsIsInvalid = std::any_of(windowIds.begin(), windowIds.end(), [](int windowId) {
        return windowId <= INVALID_SESSION_ID;
    });
    if (sessionIdsIsInvalid) {
        TLOGE(WmsLogTag::WMS_LIFE, "invalid windowId.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    return WMError::WM_OK;
}

WMError SceneSessionManager::ClearMainSessions(const std::vector<int32_t>& persistentIds,
    std::vector<int32_t>& clearFailedIds)
{
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_MAIN, "Clear main sessions failed, only support SA calling.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    clearFailedIds.clear();
    for (const auto persistentId : persistentIds) {
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGW(WmsLogTag::WMS_MAIN, "Session id:%{public}d is not found.", persistentId);
            clearFailedIds.push_back(persistentId);
            continue;
        }
        if (!WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
            TLOGW(WmsLogTag::WMS_MAIN, "Session id:%{public}d is not mainWindow.", persistentId);
            clearFailedIds.push_back(persistentId);
            continue;
        }
        sceneSession->Clear();
        TLOGD(WmsLogTag::WMS_MAIN, "Clear succeed: session id:%{public}d.", persistentId);
    }
    return WMError::WM_OK;
}

WMError SceneSessionManager::UpdateDisplayHookInfo(int32_t uid, uint32_t width, uint32_t height, float_t density,
    bool enable)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "width: %{public}u, height: %{public}u, density: %{public}f, enable: %{public}d",
        width, height, density, enable);

    DMHookInfo dmHookInfo;
    dmHookInfo.width_ = width;
    dmHookInfo.height_ = height;
    dmHookInfo.density_ = density;
    dmHookInfo.rotation_ = 0;
    dmHookInfo.enableHookRotation_ = false;
    dmHookInfo.displayOrientation_ = 0;
    dmHookInfo.enableHookDisplayOrientation_ = false;
    {
        std::shared_lock lock(appHookWindowInfoMapMutex_);
        dmHookInfo.isFullScreenInForceSplit_ = fullScreenInForceSplitUidSet_.find(uid) != fullScreenInForceSplitUidSet_.end();
    }
    ScreenSessionManagerClient::GetInstance().UpdateDisplayHookInfo(uid, enable, dmHookInfo);
    return WMError::WM_OK;
}

WMError SceneSessionManager::UpdateAppHookDisplayInfo(int32_t uid, const HookInfo& hookInfo, bool enable)
{
    TLOGI(WmsLogTag::WMS_COMPAT, "hookInfo: [%{public}s], enable: %{public}d", hookInfo.ToString().c_str(), enable);
    if (enable && (uid <= 0 || hookInfo.width_ <= 0 || hookInfo.height_ <= 0 || hookInfo.density_ <= 0)) {
        TLOGE(WmsLogTag::WMS_COMPAT, "App hookInfo param error.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    DMHookInfo dmHookInfo;
    dmHookInfo.width_ = hookInfo.width_;
    dmHookInfo.height_ = hookInfo.height_;
    dmHookInfo.density_ = hookInfo.density_;
    dmHookInfo.rotation_ = hookInfo.rotation_;
    dmHookInfo.enableHookRotation_ = hookInfo.enableHookRotation_;
    dmHookInfo.displayOrientation_ = hookInfo.displayOrientation_;
    dmHookInfo.enableHookDisplayOrientation_ = hookInfo.enableHookDisplayOrientation_;
    {
        std::shared_lock lock(appHookWindowInfoMapMutex_);
        dmHookInfo.isFullScreenInForceSplit_ = fullScreenInForceSplitUidSet_.find(uid) != fullScreenInForceSplitUidSet_.end();
    }
    ScreenSessionManagerClient::GetInstance().UpdateDisplayHookInfo(uid, enable, dmHookInfo);
    return WMError::WM_OK;
}

WMError SceneSessionManager::UpdateAppHookWindowInfo(const std::string& bundleName,
                                                     const HookWindowInfo& hookWindowInfo)
{
    if (bundleName.empty()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Bundle name is empty");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (hookWindowInfo.widthHookRatio < 0.0f) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "Invalid hook window parameters: widthHookRatio:%{public}f, "
            "bundleName:%{public}s", hookWindowInfo.widthHookRatio, bundleName.c_str());
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    TLOGI(WmsLogTag::WMS_LAYOUT, "bundleName:%{public}s, hookWindowInfo:[%{public}s]", bundleName.c_str(),
        hookWindowInfo.ToString().c_str());

    HookWindowInfo preInfo;
    {
        std::unique_lock lock(appHookWindowInfoMapMutex_);
        if (appHookWindowInfoMap_.count(bundleName)) {
            preInfo = appHookWindowInfoMap_[bundleName];
        } else {
            preInfo = {};
        }
        appHookWindowInfoMap_[bundleName] = hookWindowInfo;
    }

    if (preInfo.enableHookWindow != hookWindowInfo.enableHookWindow ||
        !MathHelper::NearZero(preInfo.widthHookRatio - hookWindowInfo.widthHookRatio)) {
        //Notify the client of the info change
        std::shared_lock lock(sceneSessionMapMutex_);
        for (const auto& [_, session] : sceneSessionMap_) {
            if (session && session->GetSessionInfo().bundleName_ == bundleName) {
                session->NotifyAppHookWindowInfoUpdated();
            }
        }
    }
    return WMError::WM_OK;
}

HookWindowInfo SceneSessionManager::GetAppHookWindowInfo(const std::string& bundleName)
{
    if (bundleName.empty()) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "Empty bundle name requested");
        return {};
    }
    std::shared_lock lock(appHookWindowInfoMapMutex_);
    const auto& it = appHookWindowInfoMap_.find(bundleName);
    if (it == appHookWindowInfoMap_.end()) {
        TLOGD(WmsLogTag::WMS_LAYOUT, "app: %{public}s, hookWindowInfo not find", bundleName.c_str());
        return {};
    }
    return it->second;
}

void SceneSessionManager::UpdateAppHookWindowInfoWhenSwitchFreeMultiWindow(bool isOpenFreeMultiWindow)
{
    std::unordered_set<std::string> bundleNames;
    {
        std::unique_lock lock(appHookWindowInfoMapMutex_);
        for (auto& [bundleName, hookWindowInfo] : appHookWindowInfoMap_) {
            hookWindowInfo.enableHookWindow = !isOpenFreeMultiWindow;
            bundleNames.insert(bundleName);
        }
    }
    {
        std::shared_lock lock(sceneSessionMapMutex_);
        for (const auto& [_, session] : sceneSessionMap_) {
            if (session && bundleNames.count(session->GetSessionInfo().bundleName_)) {
                session->NotifyAppHookWindowInfoUpdated();
            }
        }
    }
}

void SceneSessionManager::UpdateRsCmdBlockingCount(bool enable)
{
    taskScheduler_->PostAsyncTask([this, enable, where = __func__]() {
        if (enable) {     
            rsCmdBlockingFlag_.count++;
            rsCmdBlockingFlag_.startTime = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
        } else {
            rsCmdBlockingFlag_.count--;
        }
        TLOGNI(WmsLogTag::WMS_LAYOUT, "%{public}s, enable: %{public}d, count: %{public}d",
            where, enable, rsCmdBlockingFlag_.count);
    }, __func__);  

    if (enable) {
        // blocking 2 vsyncs to wait creating node cmd flushed
        constexpr uint32_t rsCmdDelayVsyncs = 2; 
        RunAfterNVsyncs(rsCmdDelayVsyncs, [this] (int64_t, int64_t) {
            taskScheduler_->PostAsyncTask([this]() {
                TLOGNI(WmsLogTag::WMS_LAYOUT, "update rsCmdBlockingCount after vsyncs");
                rsCmdBlockingFlag_.count--;
            }, __func__);
        });
    }
}

int32_t SceneSessionManager::GetOrResetRsCmdBlockingCount()
{
    return taskScheduler_->PostSyncTask([this]() {
        if (rsCmdBlockingFlag_.count == 0) {
            return rsCmdBlockingFlag_.count;
        }
        auto endTime = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        auto validTime = endTime - rsCmdBlockingFlag_.startTime;
        if (validTime > RS_CMD_BLOCKING_TIMEOUT_MS || rsCmdBlockingFlag_.count < 0) {
            rsCmdBlockingFlag_.count = 0;
            rsCmdBlockingFlag_.startTime = 0;
        }
        return rsCmdBlockingFlag_.count;
    }, __func__);
}

WMError SceneSessionManager::NotifyHookOrientationChange(int32_t persistentId)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_COMPAT, "session id:%{public}d is not found", persistentId);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    sceneSession->UpdateOrientation();
    return WMError::WM_OK;
}

void DisplayChangeListener::OnScreenFoldStatusChanged(const std::vector<std::string>& screenFoldInfo)
{
    SceneSessionManager::GetInstance().SetDelayRemoveSnapshot(true);
    SceneSessionManager::GetInstance().ReportScreenFoldStatusChange(screenFoldInfo);
}

void SceneSessionManager::SetDelayRemoveSnapshot(bool delayRemoveSnapshot)
{
    TLOGD(WmsLogTag::WMS_PATTERN, "delayRemoveSnapshot %{public}d", delayRemoveSnapshot);
    delayRemoveSnapshot_ = delayRemoveSnapshot;
}

bool SceneSessionManager::GetDelayRemoveSnapshot() const
{
    return delayRemoveSnapshot_;
}

WMError SceneSessionManager::ReportScreenFoldStatusChange(const std::vector<std::string>& screenFoldInfo)
{
    ScreenFoldData screenFoldData;
    WMError ret = MakeScreenFoldData(screenFoldInfo, screenFoldData);
    if (ret != WMError::WM_OK) {
        return ret;
    }
    return CheckAndReportScreenFoldStatus(screenFoldData);
}

WMError SceneSessionManager::MakeScreenFoldData(const std::vector<std::string>& screenFoldInfo,
    ScreenFoldData& screenFoldData)
{
    if (screenFoldInfo.size() < ScreenFoldData::DMS_PARAM_NUMBER) {
        TLOGI(WmsLogTag::DMS, "Error: Init DMS param number is wrong.");
        return WMError::WM_DO_NOTHING;
    }

    screenFoldData.currentScreenFoldStatus_ = std::stoi(screenFoldInfo[0]); // 0: current screen fold status
    screenFoldData.nextScreenFoldStatus_ = std::stoi(screenFoldInfo[1]); // 1: next screen fold status
    screenFoldData.currentScreenFoldStatusDuration_ = std::stoi(screenFoldInfo[2]); // 2: current duration
    screenFoldData.postureAngle_ = std::atof(screenFoldInfo[3].c_str()); // 3: posture angle (type: float)
    screenFoldData.screenRotation_ = std::stoi(screenFoldInfo[4]); // 4: screen rotation
    WSSnapshotHelper::GetInstance()->SetWindowScreenStatus(
        static_cast<FoldStatus>(screenFoldData.nextScreenFoldStatus_));
    if (!screenFoldData.GetTypeCThermalWithUtil()) {
        TLOGI(WmsLogTag::DMS, "Error: fail to get typeC thermal.");
        return WMError::WM_DO_NOTHING;
    }
    AppExecFwk::ElementName element = {};
    WSError ret = GetFocusSessionElement(element);
    auto sceneSession = GetSceneSession(windowFocusController_->GetFocusedSessionId(DEFAULT_DISPLAY_ID));
    if (sceneSession == nullptr || ret != WSError::WS_OK) {
        TLOGI(WmsLogTag::DMS, "Error: fail to get focused package name.");
        return WMError::WM_DO_NOTHING;
    }
    screenFoldData.SetFocusedPkgName(element.GetURI());
    return WMError::WM_OK;
}

WMError SceneSessionManager::CheckAndReportScreenFoldStatus(ScreenFoldData& data)
{
    static ScreenFoldData lastScreenHalfFoldData;
    if (data.nextScreenFoldStatus_ == static_cast<int32_t>(FoldStatus::HALF_FOLD)) {
        lastScreenHalfFoldData = data;
        return WMError::WM_DO_NOTHING;
    }
    WMError lastScreenHalfFoldReportRet = WMError::WM_OK;
    if (data.currentScreenFoldStatus_ == static_cast<int32_t>(FoldStatus::HALF_FOLD)) {
        if (data.currentScreenFoldStatusDuration_ >= ScreenFoldData::HALF_FOLD_REPORT_TRIGGER_DURATION) {
            lastScreenHalfFoldReportRet = ReportScreenFoldStatus(lastScreenHalfFoldData);
        } else if (lastScreenHalfFoldData.currentScreenFoldStatus_ != ScreenFoldData::INVALID_VALUE) {
            // if stay at half-fold less than 15s, combine this change with last
            data.currentScreenFoldStatus_ = lastScreenHalfFoldData.currentScreenFoldStatus_;
            data.currentScreenFoldStatusDuration_ += lastScreenHalfFoldData.currentScreenFoldStatusDuration_;
            data.postureAngle_ = lastScreenHalfFoldData.postureAngle_;
        }
        lastScreenHalfFoldData.SetInvalid();
    }
    WMError currentScreenFoldStatusReportRet = ReportScreenFoldStatus(data);
    return (currentScreenFoldStatusReportRet == WMError::WM_OK) ? lastScreenHalfFoldReportRet :
        currentScreenFoldStatusReportRet;
}

// report screen_fold_status event when it changes to fold/expand or stays 15s at half-fold
WMError SceneSessionManager::ReportScreenFoldStatus(const ScreenFoldData& data)
{
    if (data.currentScreenFoldStatus_ == ScreenFoldData::INVALID_VALUE) {
        return WMError::WM_DO_NOTHING;
    }

    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::FOLDSTATE_UE,
        "FOLDSCREEN_STATE_CHANGE",
        OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "PNAMEID", "OCCUPATION", "PVERSIONID", "OCCUPATION",
        "LASTFOLDSTATE", data.currentScreenFoldStatus_,
        "CURRENTFOLDSTATE", data.nextScreenFoldStatus_,
        "STATE", -1,
        "TIME", data.currentScreenFoldStatusDuration_,
        "ROTATION", data.screenRotation_,
        "PACKAGE", data.focusedPackageName_,
        "ANGLE", data.postureAngle_,
        "TYPECTHERMAL", data.typeCThermal_,
        "SCREENTHERMAL", -1,
        "SCANGLE", -1,
        "ISTENT", -1);
    if (ret != 0) {
        TLOGE(WmsLogTag::DMS, "Write HiSysEvent error, ret: %{public}d.", ret);
        return WMError::WM_DO_NOTHING;
    }
    return WMError::WM_OK;
}

void SceneSessionManager::UpdateSecSurfaceInfo(std::shared_ptr<RSUIExtensionData> secExtensionData, uint64_t userId)
{
    if (currentUserId_ != static_cast<int32_t>(userId)) {
        TLOGW(WmsLogTag::WMS_MULTI_USER, "currentUserId_:%{public}d userId:%{public}" PRIu64,
            currentUserId_.load(), userId);
        return;
    }
    if (secExtensionData == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "invalid secExtensionData");
        return;
    }
    auto secSurfaceInfoMap = secExtensionData->GetSecData();
    auto task = [secSurfaceInfoMap]()-> WSError {
        SceneInputManager::GetInstance().UpdateSecSurfaceInfo(secSurfaceInfoMap);
        return WSError::WS_OK;
    };
    taskScheduler_->PostAsyncTask(task, "UpdateSecSurfaceInfo");
}

void SceneSessionManager::RegisterSecSurfaceInfoListener()
{
    auto callBack = [this](std::shared_ptr<RSUIExtensionData> secExtensionData, uint64_t userid) {
        this->UpdateSecSurfaceInfo(secExtensionData, userid);
    };
    TLOGI(WmsLogTag::WMS_EVENT, "in");
    if (rsInterface_.RegisterUIExtensionCallback(currentUserId_, callBack) != WM_OK) {
        TLOGE(WmsLogTag::WMS_EVENT, "failed");
    }
}

void SceneSessionManager::UpdateConstrainedModalUIExtInfo(std::shared_ptr<RSUIExtensionData> constrainedModalUIExtData,
    uint64_t userId)
{
    if (currentUserId_ != static_cast<int32_t>(userId)) {
        TLOGW(WmsLogTag::WMS_MULTI_USER, "currentUserId_:%{public}d userId:%{public}" PRIu64,
            currentUserId_.load(), userId);
        return;
    }
    if (constrainedModalUIExtData == nullptr) {
        TLOGE(WmsLogTag::WMS_EVENT, "invalid constrainedModalUIExtData");
        return;
    }
    auto constrainedModalUIExtInfoMap = constrainedModalUIExtData->GetSecData();
    auto task = [constrainedModalUIExtInfoMap]()-> WSError {
        SceneInputManager::GetInstance().UpdateConstrainedModalUIExtInfo(constrainedModalUIExtInfoMap);
        return WSError::WS_OK;
    };
    taskScheduler_->PostAsyncTask(task, "UpdateConstrainedModalUIExtInfo");
}

void SceneSessionManager::RegisterConstrainedModalUIExtInfoListener()
{
    auto callBack = [this](std::shared_ptr<RSUIExtensionData> constrainedModalUIExtData, uint64_t userid) {
        this->UpdateConstrainedModalUIExtInfo(constrainedModalUIExtData, userid);
    };
    TLOGI(WmsLogTag::WMS_EVENT, "in");
    if (rsInterface_.RegisterUIExtensionCallback(currentUserId_, callBack, true) != WM_OK) {
        TLOGE(WmsLogTag::WMS_EVENT, "failed");
    }
}

WSError SceneSessionManager::SetAppForceLandscapeConfig(const std::string& bundleName,
    AppForceLandscapeConfig& config)
{
    if (bundleName.empty()) {
        TLOGE(WmsLogTag::WMS_COMPAT, "bundle name is empty");
        return WSError::WS_ERROR_NULLPTR;
    }
    AppForceLandscapeConfig preConfig;
    {
        std::unique_lock<std::shared_mutex> lock(appForceLandscapeMutex_);
        if (appForceLandscapeMap_.count(bundleName)) {
            preConfig = appForceLandscapeMap_[bundleName];
        } else {
            preConfig = {};
        }
        config.hasChanged_ = !AppForceLandscapeConfig::IsSameForceSplitConfig(preConfig, config);
        appForceLandscapeMap_[bundleName] = config;
    }

    TLOGI(WmsLogTag::WMS_COMPAT,
        "bundleName:%{public}s, config:[mode_%{public}d, supportSplit_%{public}d, ignoreOrientation_%{public}d, "
        "containsSysConfig_%{public}d, isSysRouter_%{public}d, sysHomePage_%{public}s, sysConfigJsonStr_%{public}s, "
        "containsAppConfig_%{public}d, isAppRouter_%{public}d, appConfigJsonStr_%{public}s]",
        bundleName.c_str(),
        config.mode_,
        config.supportSplit_,
        config.ignoreOrientation_,
        config.containsSysConfig_,
        config.isSysRouter_,
        config.sysHomePage_.c_str(),
        config.sysConfigJsonStr_.c_str(),
        config.containsAppConfig_,
        config.isAppRouter_,
        config.appConfigJsonStr_.c_str());
    if (preConfig.mode_ == FORCE_SPLIT_MODE || config.mode_ == FORCE_SPLIT_MODE ||
        preConfig.mode_ == NAV_FORCE_SPLIT_MODE || config.mode_ == NAV_FORCE_SPLIT_MODE) {
        //Notify the client of the mode change
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& iter : sceneSessionMap_) {
            auto& session = iter.second;
            if (session && session->GetSessionInfo().bundleName_ == bundleName) {
                session->NotifyAppForceLandscapeConfigUpdated();
            }
        }
    }
    return WSError::WS_OK;
}

WSError SceneSessionManager::SetAppForceLandscapeConfigEnable(const std::string& bundleName,
    const bool enableForceSplit)
{
    if (bundleName.empty()) {
        TLOGE(WmsLogTag::WMS_COMPAT, "bundle name is empty");
        return WSError::WS_ERROR_NULLPTR;
    }

    AppForceLandscapeConfig config;
    {
        std::unique_lock<std::shared_mutex> lock(appForceLandscapeMutex_);
        if (appForceLandscapeMap_.count(bundleName)) {
            config = appForceLandscapeMap_[bundleName];
        } else {
            TLOGI(WmsLogTag::WMS_COMPAT, "app: %{public}s, config not find", bundleName.c_str());
        }
        config.configEnable_ = enableForceSplit;
        appForceLandscapeMap_[bundleName] = config;
    }
    std::map<int32_t, sptr<SceneSession>> sceneSessionMapCopy;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        sceneSessionMapCopy = sceneSessionMap_;
    }
    for (const auto& iter : sceneSessionMapCopy) {
        auto& session = iter.second;
        if (session && session->GetSessionInfo().bundleName_ == bundleName &&
            SessionHelper::IsMainWindow(session->GetWindowType())) {
            session->NotifyAppForceLandscapeConfigEnableUpdated();
        }
    }
    TLOGI(WmsLogTag::WMS_COMPAT, "bundleName:%{public}s, enable:%{public}d", bundleName.c_str(), enableForceSplit);
    return WSError::WS_OK;
}

AppForceLandscapeConfig SceneSessionManager::GetAppForceLandscapeConfig(const std::string& bundleName)
{
    if (bundleName.empty()) {
        return {};
    }
    std::shared_lock<std::shared_mutex> lock(appForceLandscapeMutex_);
    if (appForceLandscapeMap_.empty() ||
        appForceLandscapeMap_.find(bundleName) == appForceLandscapeMap_.end()) {
        TLOGD(WmsLogTag::WMS_COMPAT, "app: %{public}s, config not find", bundleName.c_str());
        return {};
    }
    return appForceLandscapeMap_[bundleName];
}

bool SceneSessionManager::GetAppForceLandscapeConfigEnable(const std::string& bundleName)
{
    if (bundleName.empty()) {
        return false;
    }
    std::shared_lock<std::shared_mutex> lock(appForceLandscapeMutex_);
    if (appForceLandscapeMap_.empty() ||
        appForceLandscapeMap_.find(bundleName) == appForceLandscapeMap_.end()) {
        TLOGD(WmsLogTag::WMS_COMPAT, "app: %{public}s, config not find", bundleName.c_str());
        return false;
    }
    TLOGI(WmsLogTag::WMS_COMPAT, "bundleName:%{public}s, enable:%{public}d",
        bundleName.c_str(), appForceLandscapeMap_[bundleName].configEnable_);
    return appForceLandscapeMap_[bundleName].configEnable_;
}

WMError SceneSessionManager::TerminateSessionByPersistentId(int32_t persistentId)
{
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_KILL_APP_PROCESS)) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has no permission granted.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is not system app.");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Session id:%{public}d is not found.", persistentId);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (!WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_MAIN, "Session id:%{public}d is not mainWindow.", persistentId);
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    sceneSession->Clear(true);
    TLOGI(WmsLogTag::WMS_LIFE, "Terminate success, id:%{public}d.", persistentId);
    return WMError::WM_OK;
}

WSError SceneSessionManager::PendingSessionToBackgroundByPersistentId(const int32_t persistentId,
    bool shouldBackToCaller)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    uint32_t callerToken = IPCSkeleton::GetCallingTokenID();
    if (!SessionPermission::VerifyPermissionByCallerToken(callerToken,
            PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "Permission denied, no manage misson permission");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    return taskScheduler_->PostSyncTask([this, persistentId, shouldBackToCaller] {
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGE(WmsLogTag::WMS_LIFE, "Session id:%{public}d is not found.", persistentId);
            return WSError::WS_ERROR_INVALID_PARAM;
        }
        if (!WindowHelper::IsMainWindow(sceneSession->GetWindowType())) {
            TLOGE(WmsLogTag::WMS_MAIN, "Session id:%{public}d is not mainWindow.", persistentId);
            return WSError::WS_ERROR_INVALID_OPERATION;
        }
        return sceneSession->PendingSessionToBackgroundForDelegator(shouldBackToCaller);
    }, __func__);
}

void SceneSessionManager::SetRootSceneProcessBackEventFunc(const RootSceneProcessBackEventFunc& processBackEventFunc)
{
    rootSceneProcessBackEventFunc_ = processBackEventFunc;
    TLOGI(WmsLogTag::WMS_EVENT, "end");
}

WMError SceneSessionManager::GetProcessSurfaceNodeIdByPersistentId(const int32_t pid,
    const std::vector<int32_t>& persistentIds, std::vector<uint64_t>& surfaceNodeIds)
{
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "The caller has no permission granted.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }

    surfaceNodeIds.clear();
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Get process surfaceNodeId by persistentId, pid:%{public}d", pid);
    for (auto persistentId : persistentIds) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "convert wid:%{public}d", persistentId);
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            continue;
        }
        auto callingPid = sceneSession->GetCallingPid();
        auto surfaceNode = sceneSession->GetSurfaceNode();
        if (surfaceNode != nullptr && callingPid == pid) {
            surfaceNodeIds.push_back(surfaceNode->GetId());
            auto leashWinSurfaceNode = sceneSession->GetLeashWinSurfaceNode();
            if (leashWinSurfaceNode != nullptr) {
                surfaceNodeIds.push_back(leashWinSurfaceNode->GetId());
                surfaceNodeIds.push_back(persistentId);
            }
        }
    }

    return WMError::WM_OK;
}

void SceneSessionManager::SetCloseTargetFloatWindowFunc(const ProcessCloseTargetFloatWindowFunc& func)
{
    TLOGD(WmsLogTag::WMS_MULTI_WINDOW, "in");
    auto task = [this, func] {
        closeTargetFloatWindowFunc_ = func;
    };
    taskScheduler_->PostTask(task, __func__);
}

WMError SceneSessionManager::CloseTargetFloatWindow(const std::string& bundleName)
{
    if (!SessionPermission::IsSystemServiceCalling(false)) {
        TLOGE(WmsLogTag::WMS_MULTI_WINDOW, "failed, not system service called.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    auto task = [this, bundleName] {
        if (closeTargetFloatWindowFunc_) {
            TLOGI(WmsLogTag::WMS_MULTI_WINDOW, "bundleName:%{public}s", bundleName.c_str());
            closeTargetFloatWindowFunc_(bundleName);
        }
    };
    taskScheduler_->PostTask(task, __func__);
    return WMError::WM_OK;
}

void SceneSessionManager::UpdatePiPWindowStateChanged(const std::string& bundleName, bool isForeground)
{
    SessionManagerAgentController::GetInstance().UpdatePiPWindowStateChanged(bundleName, isForeground);
}

WMError SceneSessionManager::CloseTargetPiPWindow(const std::string& bundleName)
{
    if (!SessionPermission::IsSystemServiceCalling(false)) {
        TLOGE(WmsLogTag::WMS_PIP, "failed, not system service called.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& iter : sceneSessionMap_) {
        auto& session = iter.second;
        if (session && session->GetWindowType() == WindowType::WINDOW_TYPE_PIP &&
            session->GetSessionInfo().bundleName_ == bundleName) {
            session->NotifyCloseExistPipWindow();
            break;
        }
    }
    return WMError::WM_OK;
}

WMError SceneSessionManager::GetCurrentPiPWindowInfo(std::string& bundleName)
{
    if (!SessionPermission::IsSystemServiceCalling(false)) {
        TLOGE(WmsLogTag::WMS_PIP, "failed, not system service called.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& iter : sceneSessionMap_) {
        auto& session = iter.second;
        if (session && session->GetWindowType() == WindowType::WINDOW_TYPE_PIP) {
            bundleName = session->GetSessionInfo().bundleName_;
            return WMError::WM_OK;
        }
    }
    TLOGW(WmsLogTag::WMS_PIP, "no PiP window");
    return WMError::WM_OK;
}

void SceneSessionManager::RefreshPcZOrderList(uint32_t startZOrder, std::vector<int32_t>&& persistentIds)
{
    const char* const where = __func__;
    auto task = [this, startZOrder, persistentIds = std::move(persistentIds), where] {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < persistentIds.size(); i++) {
            int32_t persistentId = persistentIds[i];
            oss << persistentId;
            if (i < persistentIds.size() - 1) {
                oss << ",";
            }
            auto sceneSession = GetSceneSession(persistentId);
            if (sceneSession == nullptr) {
                TLOGNE(WmsLogTag::WMS_LAYOUT, "sceneSession is nullptr persistentId=%{public}d", persistentId);
                continue;
            }
            if (i > UINT32_MAX - startZOrder) {
                TLOGNE(WmsLogTag::WMS_LAYOUT, "Z order overflow, stop refresh");
                break;
            }
            sceneSession->SetPcScenePanel(true);
            sceneSession->UpdatePCZOrderAndMarkDirty(i + startZOrder);
        }
        oss << "]";
        TLOGND(WmsLogTag::WMS_LAYOUT, "%{public}s: %{public}s", where, oss.str().c_str());
        return WSError::WS_OK;
    };
    taskScheduler_->PostTask(task, __func__);
}

WMError SceneSessionManager::SkipSnapshotForAppProcess(int32_t pid, bool skip)
{
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (pid < 0) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "invalid pid!");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    TLOGI(WmsLogTag::WMS_LIFE, "pid:%{public}d, skip:%{public}u", pid, skip);
    auto task = [this, pid, skip]() THREAD_SAFETY_GUARD(SCENE_GUARD) {
        if (skip) {
            snapshotSkipPidSet_.insert(pid);
        } else {
            snapshotSkipPidSet_.erase(pid);
        }
        std::unique_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [persistentId, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                continue;
            }
            if (pid == sceneSession->GetCallingPid()) {
                TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "send rs set snapshot skip, persistentId:%{public}d, skip:%{public}u",
                    persistentId, skip);
                sceneSession->SetSnapshotSkip(skip);
            }
        }
    };
    taskScheduler_->PostTask(task, "SkipSnapshotForAppProcess");
    return WMError::WM_OK;
}

void SceneSessionManager::SetSessionSnapshotSkipForAppProcess(const sptr<SceneSession>& sceneSession)
{
    auto callingPid = sceneSession->GetCallingPid();
    if (snapshotSkipPidSet_.find(callingPid) != snapshotSkipPidSet_.end()) {
        sceneSession->SetSnapshotSkip(true);
    }
}

WMError SceneSessionManager::SkipSnapshotByUserIdAndBundleNames(int32_t userId,
    const std::vector<std::string>& bundleNameList)
{
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "The caller has no permission granted.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "userId:%{public}d", userId);
    auto task = [this, userId, bundleNameList]() THREAD_SAFETY_GUARD(SCENE_GUARD) {
        snapshotSkipBundleNameSet_.clear();
        for (auto& bundleName : bundleNameList) {
            snapshotSkipBundleNameSet_.insert(std::move(bundleName));
        }
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                continue;
            }
            const std::string& bundleName = sceneSession->GetSessionInfo().bundleName_;
            if (snapshotSkipBundleNameSet_.find(bundleName) != snapshotSkipBundleNameSet_.end()) {
                TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "set RS snapshot skip true, name:%{public}s",
                    bundleName.c_str());
                sceneSession->SetSnapshotSkip(true);
                continue;
            }
            sceneSession->SetSnapshotSkip(false);
        }
    };
    taskScheduler_->PostTask(task, __func__);
    return WMError::WM_OK;
}

void SceneSessionManager::SetSessionSnapshotSkipForAppBundleName(const sptr<SceneSession>& sceneSession)
{
    const std::string& name = sceneSession->GetSessionInfo().bundleName_;
    if (snapshotSkipBundleNameSet_.find(name) != snapshotSkipBundleNameSet_.end()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "set RS snapshot skip true, name:%{public}s",
            name.c_str());
        sceneSession->SetSnapshotSkip(true);
    }
}

WMError SceneSessionManager::SetProcessWatermark(int32_t pid, const std::string& watermarkName, bool isEnabled)
{
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "pid:%{public}d, watermarkName:%{public}s, isEnabled:%{public}u",
        pid, watermarkName.c_str(), isEnabled);
    if (isEnabled && watermarkName.empty()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "watermarkName is empty!");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto task = [this, pid, watermarkName, isEnabled] {
        if (isEnabled) {
            processWatermarkPidMap_.insert_or_assign(pid, watermarkName);
        } else {
            processWatermarkPidMap_.erase(pid);
        }

        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                continue;
            }
            if (pid == sceneSession->GetCallingPid()) {
                sceneSession->SetWatermarkEnabled(watermarkName, isEnabled);
            }
        }
    };
    taskScheduler_->PostTask(task, __func__);
    return WMError::WM_OK;
}

bool SceneSessionManager::SetSessionWatermarkForAppProcess(const sptr<SceneSession>& sceneSession)
{
    if (auto iter = processWatermarkPidMap_.find(sceneSession->GetCallingPid());
        iter != processWatermarkPidMap_.end()) {
        sceneSession->SetWatermarkEnabled(iter->second, true);
        return true;
    }
    return false;
}

WMError SceneSessionManager::SetWatermarkImageForApp(const std::shared_ptr<Media::PixelMap>& pixelMap,
    std::string& watermarkName)
{
    const char* const where = __func__;
    int32_t pid = IPCSkeleton::GetCallingRealPid();
    return taskScheduler_->PostSyncTask([this, pid, pixelMap, &watermarkName, where]() {
        if (pixelMap == nullptr) {
            auto iter = appWatermarkPidMap_.find(pid);
            if (iter == appWatermarkPidMap_.end()) {
                TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: no watermark for pid=%{public}d", where, pid);
                return WMError::WM_OK;
            }
            watermarkName = iter->second;
            appWatermarkPidMap_.erase(pid);
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: cancel watermark=%{public}s, pid=%{public}d",
                where, watermarkName.c_str(), pid);
            RSInterfaces::GetInstance().ClearSurfaceWatermark(pid, watermarkName);
            watermarkName = "";
            return WMError::WM_OK;
        }
        std::string newWatermarkName;
        auto nodeIds = GetSessionNodeIdsAndWatermarkNameByPid(pid, newWatermarkName);
        auto rsErrCode = RSInterfaces::GetInstance().SetSurfaceWatermark(pid, newWatermarkName, pixelMap, nodeIds,
            CUSTOM_WATER_MARK);
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: pid=%{public}d, curWatermark=%{public}s, rsErrCode=%{public}u",
            where, pid, newWatermarkName.c_str(), static_cast<uint32_t>(rsErrCode));
        if (rsErrCode == WATER_MARK_SUCCESS) {
            watermarkName = newWatermarkName;
            appWatermarkPidMap_.insert_or_assign(pid, watermarkName);
            return WMError::WM_OK;
        }
        if (rsErrCode & WATER_MARK_IMG_SIZE_ERROR) {
            return WMError::WM_ERROR_ILLEGAL_PARAM;
        }
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }, where);
}

WMError SceneSessionManager::RecoverWatermarkImageForApp(const std::string& watermarkName)
{
    int32_t pid = IPCSkeleton::GetCallingRealPid();
    taskScheduler_->PostAsyncTask([this, pid, watermarkName, where = __func__]() {
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: watermark=%{public}s, pid=%{public}d",
            where, watermarkName.c_str(), pid);
        if (!watermarkName.empty() && appWatermarkPidMap_[pid] != watermarkName) {
            appWatermarkPidMap_[pid] = watermarkName;
        }
    }, __func__);
    return WMError::WM_OK;
}

std::vector<NodeId> SceneSessionManager::GetSessionNodeIdsAndWatermarkNameByPid(int32_t pid, std::string& watermarkName)
{
    std::vector<NodeId> nodeIds;
    std::string bundleName;
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, session] : sceneSessionMap_) {
        if (session == nullptr || session->GetCallingPid() != pid) {
            continue;
        }
        bundleName = session->GetSessionInfo().bundleName_;
        auto surfaceNode = session->GetSurfaceNode();
        if (surfaceNode == nullptr) {
            TLOGW(WmsLogTag::WMS_ATTRIBUTE, "empty node win=[%{public}d, %{public}s], pid=%{public}d",
                session->GetWindowId(), session->GetWindowName().c_str(), pid);
            continue;
        }
        nodeIds.push_back(surfaceNode->GetId());
    }
    watermarkName = bundleName + "#" + std::to_string(pid);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "pid=%{public}d, bundle=%{public}s, nodesSize=%{public}u",
        pid, bundleName.c_str(), static_cast<uint32_t>(nodeIds.size()));
    return nodeIds;
}

void SceneSessionManager::SetWatermarkForSession(const sptr<SceneSession>& session)
{
    const char* const where = __func__;
    taskScheduler_->PostSyncTask([this, weakSession = wptr(session), where]() {
        sptr<SceneSession> sceneSession = weakSession.promote();
        if (sceneSession == nullptr) {
            TLOGNW(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: session is null", where);
            return WSError::WS_DO_NOTHING;
        }
        auto pid = sceneSession->GetCallingPid();
        auto iter = appWatermarkPidMap_.find(pid);
        if (iter == appWatermarkPidMap_.end()) {
            return WSError::WS_OK;
        }
        std::string watermarkName = iter->second;
        auto surfaceNode = sceneSession->GetSurfaceNode();
        if (surfaceNode == nullptr) {
            TLOGNW(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: empty node win=[%{public}d, %{public}s], pid=%{public}d",
                where, sceneSession->GetWindowId(), sceneSession->GetWindowName().c_str(), pid);
            return WSError::WS_ERROR_INVALID_WINDOW;
        }
        std::vector<NodeId> nodeIds;
        nodeIds.push_back(surfaceNode->GetId());
        auto rsErrCode = RSInterfaces::GetInstance().SetSurfaceWatermark(pid, watermarkName, nullptr, nodeIds,
            CUSTOM_WATER_MARK);
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: win=[%{public}d, %{public}s], pid=%{public}d, "
            "watermark=%{public}s, rsErrCode=%{public}u", where, sceneSession->GetWindowId(),
            sceneSession->GetWindowName().c_str(), pid, watermarkName.c_str(), static_cast<uint32_t>(rsErrCode));
        return WSError::WS_OK;
    }, where);
}

void SceneSessionManager::ClearWatermarkForSession(const sptr<SceneSession>& session)
{
    taskScheduler_->PostAsyncTask([this, weakSession = wptr(session), where = __func__]() {
        sptr<SceneSession> sceneSession = weakSession.promote();
        if (sceneSession == nullptr) {
            TLOGNW(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: session is null", where);
            return;
        }
        auto pid = sceneSession->GetCallingPid();
        auto iter = appWatermarkPidMap_.find(pid);
        if (iter == appWatermarkPidMap_.end()) {
            return;
        }
        std::string watermarkName = iter->second;
        auto surfaceNode = sceneSession->GetSurfaceNode();
        if (surfaceNode == nullptr) {
            TLOGNW(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: empty node win=[%{public}d, %{public}s], pid=%{public}d",
                where, sceneSession->GetWindowId(), sceneSession->GetWindowName().c_str(), pid);
            return;
        }
        std::vector<NodeId> nodeIds;
        nodeIds.push_back(surfaceNode->GetId());
        RSInterfaces::GetInstance().ClearSurfaceWatermarkForNodes(pid, watermarkName, nodeIds);
        TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: win=[%{public}d, %{public}s], pid=%{public}d, "
            "watermark=%{public}s", where, sceneSession->GetWindowId(), sceneSession->GetWindowName().c_str(),
            pid, watermarkName.c_str());
    }, __func__);
}

void SceneSessionManager::ClearProcessRecordWhenAppExit(const AppExecFwk::ProcessData& processData)
{
    taskScheduler_->PostAsyncTask([this, processData, where = __func__]() THREAD_SAFETY_GUARD(SCENE_GUARD) {
        TLOGND(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: pid=%{public}d, bundleName=%{public}s",
            where, processData.pid, processData.bundleName.c_str());
        appWatermarkPidMap_.erase(processData.pid);
        snapshotSkipPidSet_.erase(processData.pid);
    }, __func__);
}

WMError SceneSessionManager::GetRootMainWindowId(int32_t persistentId, int32_t& hostWindowId)
{
    if (!SessionPermission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::WMS_MAIN, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    const char* const where = __func__;
    auto task = [this, persistentId, &hostWindowId, where]() {
        hostWindowId = INVALID_WINDOW_ID;
        sptr<Session> session = GetSceneSession(persistentId);
        while (session && SessionHelper::IsSubWindow(session->GetWindowType())) {
            session = session->GetParentSession();
        }
        if (session && SessionHelper::IsMainWindow(session->GetWindowType())) {
            hostWindowId = session->GetPersistentId();
        }
        TLOGNI(WmsLogTag::WMS_MAIN, "%{public}s: persistentId:%{public}d hostWindowId:%{public}d",
            where, persistentId, hostWindowId);
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, where);
}

uint32_t SceneSessionManager::GetMaxInstanceCount(const std::string& bundleName)
{
    if (MultiInstanceManager::IsSupportMultiInstance(systemConfig_)) {
        return MultiInstanceManager::GetInstance().GetMaxInstanceCount(bundleName);
    } else {
        return 0u;
    }
}

uint32_t SceneSessionManager::GetInstanceCount(const std::string& bundleName)
{
    if (MultiInstanceManager::IsSupportMultiInstance(systemConfig_)) {
        return MultiInstanceManager::GetInstance().GetInstanceCount(bundleName);
    } else {
        return 0u;
    }
}

std::string SceneSessionManager::GetLastInstanceKey(const std::string& bundleName)
{
    if (MultiInstanceManager::IsSupportMultiInstance(systemConfig_)) {
        return MultiInstanceManager::GetInstance().GetLastInstanceKey(bundleName);
    } else {
        return "";
    }
}

void SceneSessionManager::RefreshAppInfo(const std::string& bundleName)
{
    if (MultiInstanceManager::IsSupportMultiInstance(systemConfig_)) {
        MultiInstanceManager::GetInstance().RefreshAppInfo(bundleName);
    }
    AbilityInfoManager::GetInstance().RemoveAppInfo(bundleName);
}

WMError SceneSessionManager::UpdateScreenLockStatusForApp(const std::string& bundleName, bool isRelease)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "bundleName=%{public}s, isRelease=%{public}d", bundleName.c_str(), isRelease);
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
#ifdef POWER_MANAGER_ENABLE
    if (isRelease) {
        return ReleaseScreenLockForApp(bundleName);
    } else {
        return RelockScreenLockForApp(bundleName);
    }
#else
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "Can not find the sub system of PowerMgr");
    return WMError::WM_OK;
#endif
}

WMError SceneSessionManager::ReleaseScreenLockForApp(const std::string& bundleName)
{
    std::vector<sptr<SceneSession>> sessionsToReleaseScreenLock;
    GetAllSessionsToReleaseScreenLock(sessionsToReleaseScreenLock, bundleName);
    auto task = [this, bundleName, sessionsToReleaseScreenLock] {
        for (const auto& sceneSession : sessionsToReleaseScreenLock) {
            if (sceneSession->keepScreenLock_ == nullptr || !sceneSession->keepScreenLock_->IsUsed()) {
                continue;
            }
            auto res = sceneSession->keepScreenLock_->UnLock();
            if (res != ERR_OK) {
                TLOGNE(WmsLogTag::WMS_ATTRIBUTE,
                    "release screenlock failed, window:[%{public}d, %{public}s], err:%{public}d",
                    sceneSession->GetPersistentId(), sceneSession->GetWindowName().c_str(), res);
                continue;
            }
            auto [iter, emplaced] = releasedScreenLockMap_.try_emplace(bundleName, std::unordered_set<int32_t>{});
            iter->second.insert(sceneSession->GetPersistentId());
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "release screenlock success, window:[%{public}d, %{public}s]",
                sceneSession->GetPersistentId(), sceneSession->GetWindowName().c_str());
        }
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

void SceneSessionManager::GetAllSessionsToReleaseScreenLock(
    std::vector<sptr<SceneSession>>& sessionsToReleaseScreenLock, const std::string& bundleName) {
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [persistentId, sceneSession] : sceneSessionMap_) {
        if (sceneSession->GetSessionInfo().bundleName_ == bundleName && sceneSession->keepScreenLock_ != nullptr) {
            sessionsToReleaseScreenLock.push_back(sceneSession);
        }
    }
}

WMError SceneSessionManager::RelockScreenLockForApp(const std::string& bundleName)
{
    auto task = [this, bundleName] {
        auto iter = releasedScreenLockMap_.find(bundleName);
        if (iter == releasedScreenLockMap_.end()) {
            TLOGNE(WmsLogTag::WMS_ATTRIBUTE, "%{public}s: not found in map", bundleName.c_str());
            return WMError::WM_ERROR_INVALID_OPERATION;
        }
        const auto& persistentIds = iter->second;
        for (const int32_t persistentId : persistentIds) {
            sptr<SceneSession> sceneSession = GetSceneSession(static_cast<int32_t>(persistentId));
            if (sceneSession == nullptr) {
                continue;
            }
            auto sourceMode = ScreenSourceMode::SCREEN_ALONE;
            auto screenSession = ScreenSessionManagerClient::GetInstance().GetScreenSessionById(
                sceneSession->GetScreenId());
            if (screenSession) {
                sourceMode = screenSession->GetSourceMode();
            }
            if (sceneSession->IsKeepScreenOn() && IsSessionVisibleForeground(sceneSession) &&
                sourceMode != ScreenSourceMode::SCREEN_UNIQUE && sceneSession->keepScreenLock_ != nullptr) {
                auto res = sceneSession->keepScreenLock_->Lock();
                TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "relock screenlock, window: [%{public}d, %{public}s], res:%{public}d",
                    persistentId, sceneSession->GetWindowName().c_str(), res);
            }
        }
        releasedScreenLockMap_.erase(bundleName);
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

WMError SceneSessionManager::IsPcWindow(bool& isPcWindow)
{
    isPcWindow = systemConfig_.IsPcWindow();
    return WMError::WM_OK;
}

WMError SceneSessionManager::IsFreeMultiWindow(bool& isFreeMultiWindow)
{
    isFreeMultiWindow = systemConfig_.freeMultiWindowEnable_;
    return WMError::WM_OK;
}

WMError SceneSessionManager::IsPcOrPadFreeMultiWindowMode(bool& isPcOrPadFreeMultiWindowMode)
{
    isPcOrPadFreeMultiWindowMode = (systemConfig_.IsPcWindow() || systemConfig_.IsFreeMultiWindowMode());
    return WMError::WM_OK;
}

WMError SceneSessionManager::GetDisplayIdByWindowId(const std::vector<uint64_t>& windowIds,
    std::unordered_map<uint64_t, DisplayId>& windowDisplayIdMap)
{
    if (!SessionPermission::IsSystemCalling()) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "permission denied!");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }

    auto task = [this, windowIds, &windowDisplayIdMap] {
        for (const uint64_t windowId : windowIds) {
            sptr<SceneSession> session = GetSceneSession(static_cast<int32_t>(windowId));
            if (session == nullptr) {
                continue;
            }
            DisplayId displayId = session->GetSessionProperty()->GetDisplayId();
            if (PcFoldScreenManager::GetInstance().IsHalfFoldedOnMainDisplay(displayId)) {
                displayId = session->GetClientDisplayId();
            }
            TLOGNI(WmsLogTag::WMS_ATTRIBUTE, "windowId:%{public}" PRIu64 ", displayId:%{public}" PRIu64,
                windowId, displayId);
            windowDisplayIdMap.insert({windowId, displayId});
        }
        return WMError::WM_OK;
    };
    return taskScheduler_->PostSyncTask(task, __func__);
}

WSError SceneSessionManager::IsLastFrameLayoutFinished(bool& isLayoutFinished)
{
    if (isRootSceneLastFrameLayoutFinishedFunc_ == nullptr) {
        TLOGE(WmsLogTag::WMS_IMMS, "isRootSceneLastFrameLayoutFinishedFunc is null");
        return WSError::WS_ERROR_NULLPTR;
    }
    isLayoutFinished = isRootSceneLastFrameLayoutFinishedFunc_();
    return WSError::WS_OK;
}

WMError SceneSessionManager::IsWindowRectAutoSave(const std::string& key, bool& enabled, int persistentId)
{
    return taskScheduler_->PostSyncTask([key, &enabled, persistentId, this] {
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_MAIN, "sceneSession %{public}d is nullptr", persistentId);
            return WMError::WM_ERROR_INVALID_SESSION;
        }
        std::string specifiedKey = key;
        if (auto iter = this->isSaveBySpecifiedFlagMap_.find(key);
            iter != this->isSaveBySpecifiedFlagMap_.end()) {
            if (iter->second) {
                specifiedKey = key + sceneSession->GetSessionInfo().specifiedFlag_;
            }
        }
        TLOGND(WmsLogTag::WMS_MAIN, "windowId: %{public}d, specifiedKey: %{public}s", persistentId, specifiedKey.c_str());
        if (auto iter = this->isWindowRectAutoSaveMap_.find(specifiedKey);
            iter != this->isWindowRectAutoSaveMap_.end()) {
            enabled = iter->second;
        } else {
            enabled = false;
        }
        return WMError::WM_OK;
    });

}

WMError SceneSessionManager::SetImageForRecent(uint32_t imgResourceId, ImageFit imageFit, int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_PATTERN, "%{public}d", persistentId);
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "sceneSession %{public}d is null", persistentId);
        return WMError::WM_ERROR_NULLPTR;
    }

    if (sceneSession->GetSessionState() == SessionState::STATE_BACKGROUND) {
        TLOGE(WmsLogTag::WMS_PATTERN, "sessionState is invalid");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto abilityInfo = sceneSession->GetSessionInfoAbilityInfo();
    if (abilityInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "abilityInfo is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!abilityInfo->applicationInfo.isSystemApp) {
        TLOGE(WmsLogTag::WMS_PATTERN, "%{public}d is not a systemApp", persistentId);
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    std::shared_ptr<Media::PixelMap> pixelMap = GetPixelMap(imgResourceId, abilityInfo);
    if (!pixelMap) {
        TLOGE(WmsLogTag::WMS_PATTERN, "get pixelMap failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto scenePersistence = sceneSession->GetScenePersistence();
    if (scenePersistence == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "scenePersistence is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    sceneSession->SaveSnapshot(true, true, pixelMap);
    scenePersistence->SetHasSnapshot(true);
    ScenePersistentStorage::Insert("SetImageForRecent_" + std::to_string(persistentId),
        static_cast<int32_t>(imageFit), ScenePersistentStorageType::MAXIMIZE_STATE);
    sceneSession->SetPersistentImageFit(static_cast<int32_t>(imageFit));
    return WMError::WM_OK;
}

WMError SceneSessionManager::SetImageForRecentPixelMap(const std::shared_ptr<Media::PixelMap>& pixelMap,
    ImageFit imageFit, int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_PATTERN, "%{public}d", persistentId);
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "sceneSession %{public}d is null", persistentId);
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    auto scenePersistence = sceneSession->GetScenePersistence();
    if (scenePersistence == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "scenePersistence is null");
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    auto abilityInfo = sceneSession->GetSessionInfo().abilityInfo;
    if (abilityInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "abilityInfo is null");
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    if (!abilityInfo->applicationInfo.isSystemApp) {
        TLOGE(WmsLogTag::WMS_PATTERN, "%{public}d is not a systemApp", persistentId);
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (sceneSession->GetSessionState() == SessionState::STATE_BACKGROUND) {
        TLOGE(WmsLogTag::WMS_PATTERN, "sessionState is invalid");
        return WMError::WM_ERROR_INVALID_WINDOW;
    }
    if (pixelMap == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "get pixelMap failed");
        return WMError::WM_ERROR_NULLPTR;
    }
    sceneSession->SaveSnapshot(true, true, pixelMap);
    scenePersistence->SetHasSnapshot(true);
    ScenePersistentStorage::Insert("SetImageForRecent_" + std::to_string(persistentId),
        static_cast<int32_t>(imageFit), ScenePersistentStorageType::MAXIMIZE_STATE);
    sceneSession->SetPersistentImageFit(static_cast<int32_t>(imageFit));
    return WMError::WM_OK;
}

WMError SceneSessionManager::RemoveImageForRecent(int32_t persistentId)
{
    TLOGI(WmsLogTag::WMS_PATTERN, "%{public}d", persistentId);
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "sceneSession %{public}d is null", persistentId);
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    auto abilityInfo = sceneSession->GetSessionInfo().abilityInfo;
    if (abilityInfo == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "abilityInfo is null");
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    if (!abilityInfo->applicationInfo.isSystemApp) {
        TLOGE(WmsLogTag::WMS_PATTERN, "%{public}d is not a systemApp", persistentId);
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    sceneSession->ResetSnapshot();
    sceneSession->DeletePersistentImageFit();
    return WMError::WM_OK;
}

std::shared_ptr<Media::PixelMap> SceneSessionManager::GetPixelMap(uint32_t resourceId,
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo, bool needCrop)
{
    auto resourceMgr = GetResourceManager(*abilityInfo.get());
    if (resourceMgr == nullptr) {
        TLOGW(WmsLogTag::WMS_PATTERN, "resource manager is null");
        return nullptr;
    }

    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource;
    if (!abilityInfo->hapPath.empty()) {
        std::unique_ptr<uint8_t[]> imageOut;
        size_t len;
        if (resourceMgr->GetMediaDataById(resourceId, len, imageOut) != Global::Resource::RState::SUCCESS) {
            return nullptr;
        }
        imageSource = Media::ImageSource::CreateImageSource(imageOut.get(), len, opts, errorCode);
    } else {
        std::string imagePath;
        if (resourceMgr->GetMediaById(resourceId, imagePath) != Global::Resource::RState::SUCCESS) {
            return nullptr;
        }
        imageSource = Media::ImageSource::CreateImageSource(imagePath, opts, errorCode);
    }

    if (errorCode != 0 || imageSource == nullptr) {
        TLOGE(WmsLogTag::WMS_PATTERN, "failed id %{private}d err %{public}d", resourceId, errorCode);
        return nullptr;
    }

    Media::DecodeOptions decodeOpts;
    if (needCrop) {
        HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "ssm:GetCropInfoByDisplaySize");
        Media::ImageInfo imageInfo;
        imageSource->GetImageInfo(imageInfo);
        GetCropInfoByDisplaySize(imageInfo, decodeOpts);
    }
    auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        TLOGE(WmsLogTag::WMS_PATTERN, "failed id %{private}d err %{public}d", resourceId, errorCode);
        return nullptr;
    }
    return std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
}

void SceneSessionManager::GetCropInfoByDisplaySize(const Media::ImageInfo& imageInfo, Media::DecodeOptions& decodeOpts)
{
    int32_t displayWidth = 0;
    int32_t displayHeight = 0;
    ScreenId defaultScreenId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    if (!GetDisplaySizeById(defaultScreenId, displayWidth, displayHeight)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "get display size failed");
        return;
    }
    int32_t cropSize = std::max(displayWidth, displayHeight);
    if (imageInfo.size.width > cropSize || imageInfo.size.height > cropSize) {
        decodeOpts.CropRect = {
            .left = std::max(0, (imageInfo.size.width - cropSize) / 2),
            .top = std::max(0, (imageInfo.size.height - cropSize) / 2),
            .width = std::min(imageInfo.size.width, cropSize),
            .height = std::min(imageInfo.size.height, cropSize),
        };
        decodeOpts.cropAndScaleStrategy = Media::CropAndScaleStrategy::CROP_FIRST;
        TLOGI(WmsLogTag::WMS_PATTERN, "crop: %{public}d, %{public}d, %{public}d, %{public}d",
            decodeOpts.CropRect.left, decodeOpts.CropRect.top, decodeOpts.CropRect.width, decodeOpts.CropRect.height);
    }
}

void SceneSessionManager::SetIsWindowRectAutoSave(const std::string& key, bool enabled,
    const std::string& abilityKey, bool isSaveBySpecifiedFlag)
{
    taskScheduler_->PostSyncTask([key, enabled, abilityKey, isSaveBySpecifiedFlag ,this] {
        if (auto iter = this->isSaveBySpecifiedFlagMap_.find(abilityKey);
            iter != this->isSaveBySpecifiedFlagMap_.end()) {
            if (!isSaveBySpecifiedFlag) {
                this->isSaveBySpecifiedFlagMap_.erase(abilityKey);
            } else {
                iter->second = isSaveBySpecifiedFlag;
            }
        } else {
            if (isSaveBySpecifiedFlag) {
                this->isSaveBySpecifiedFlagMap_.insert({abilityKey, isSaveBySpecifiedFlag});
            }
        }
        if (auto iter = this->isWindowRectAutoSaveMap_.find(key);
            iter != this->isWindowRectAutoSaveMap_.end()) {
            if (!enabled) {
                this->isWindowRectAutoSaveMap_.erase(key);
            } else {
                iter->second = enabled;
            }
        } else {
            if (enabled) {
                this->isWindowRectAutoSaveMap_.insert({key, enabled});
            }
        }
        return WMError::WM_OK;
    });
}

WMError SceneSessionManager::MinimizeMainSession(const std::string& bundleName, int32_t appIndex, int32_t userId)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if ((currentUserId_ != userId && currentUserId_ != DEFAULT_USERID) ||
        (currentUserId_ == DEFAULT_USERID && userId != GetUserIdByUid(getuid()))) {
        TLOGW(WmsLogTag::WMS_LIFE, "currentUserId:%{public}d userId:%{public}d GetUserIdByUid:%{public}d",
            currentUserId_.load(), userId, GetUserIdByUid(getuid()));
        return WMError::WM_ERROR_INVALID_OPERATION;
    }
    const char* const where = __func__;
    taskScheduler_->PostAsyncTask([this, bundleName, appIndex, where] {
        std::vector<sptr<SceneSession>> mainSessions;
        GetMainSessionByBundleNameAndAppIndex(bundleName, appIndex, mainSessions);
        if (mainSessions.empty()) {
            TLOGNW(WmsLogTag::WMS_LIFE, "%{public}s, not found any main session", where);
            return;
        }
        for (const auto& session : mainSessions) {
            session->OnSessionEvent(SessionEvent::EVENT_MINIMIZE);
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s, id:%{public}d.", where, session->GetPersistentId());
        }
    }, __func__);
    return WMError::WM_OK;
}

WMError SceneSessionManager::SetStartWindowBackgroundColor(
    const std::string& moduleName, const std::string& abilityName, uint32_t color, int32_t uid)
{
    if (!bundleMgr_) {
        TLOGE(WmsLogTag::WMS_PATTERN, "bundleMgr is nullptr");
        return WMError::WM_ERROR_NO_MEM;
    }
    std::string bundleName;
    if (!bundleMgr_->GetBundleNameForUid(uid, bundleName)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "get bundle name failed");
        return WMError::WM_ERROR_NO_MEM;
    }
    AppExecFwk::AbilityInfo abilityInfo;
    if (!bundleMgr_->GetAbilityInfo(bundleName, moduleName, abilityName, abilityInfo)) {
        TLOGE(WmsLogTag::WMS_PATTERN, "ability not found %{public}s %{public}s %{public}s",
            bundleName.c_str(), moduleName.c_str(), abilityName.c_str());
        return WMError::WM_ERROR_NO_MEM;
    }
    auto key = moduleName + abilityName;
    {
        std::unique_lock<std::shared_mutex> lock(startingWindowColorFromAppMapMutex_);
        auto iter = startingWindowColorFromAppMap_.find(bundleName);
        if (iter != startingWindowColorFromAppMap_.end()) {
            iter->second[key] = color;
        } else {
            std::unordered_map<std::string, uint32_t> colorMap({{ key, color}});
            startingWindowColorFromAppMap_.emplace(bundleName, colorMap);
        }
    }
    StartingWindowInfo info;
    uint32_t updateRes = UpdateCachedColorToAppSet(bundleName, moduleName, abilityName, info);
    TLOGI(WmsLogTag::WMS_PATTERN, "success %{public}s %{public}s, %{public}x, %{public}u",
        bundleName.c_str(), key.c_str(), color, updateRes);
    return WMError::WM_OK;
}

WMError SceneSessionManager::ShiftAppWindowPointerEvent(int32_t sourcePersistentId, int32_t targetPersistentId,
                                                        int32_t fingerId)
{
    TLOGD(WmsLogTag::WMS_PC, "sourcePersistentId %{public}d targetPersistentId %{public}d",
        sourcePersistentId, targetPersistentId);
    sptr<SceneSession> sourceSession = GetSceneSession(sourcePersistentId);
    if (sourceSession == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "sourceSession %{public}d is nullptr", sourcePersistentId);
        return WMError::WM_ERROR_INVALID_SESSION;
    }
    if (sourceSession->GetSessionProperty()->GetPcAppInpadCompatibleMode() &&
        !systemConfig_.IsFreeMultiWindowMode()) {
        TLOGE(WmsLogTag::WMS_PC, "This is PcAppInPad, not supported");
        return WMError::WM_OK;
    }
    if (!(systemConfig_.IsPcWindow() || systemConfig_.IsFreeMultiWindowMode())) {
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    if (sourcePersistentId == targetPersistentId) {
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (!WindowHelper::IsAppWindow(sourceSession->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_PC, "sourceSession %{public}d is not app window", sourcePersistentId);
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    sptr<SceneSession> targetSession = GetSceneSession(targetPersistentId);
    if (targetSession == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "targetSession %{public}d is nullptr", targetPersistentId);
        return WMError::WM_ERROR_INVALID_SESSION;
    }
    if (!WindowHelper::IsAppWindow(targetSession->GetWindowType())) {
        TLOGE(WmsLogTag::WMS_PC, "targetSession %{public}d is not app window", targetPersistentId);
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (sourceSession->GetSessionInfo().bundleName_ != targetSession->GetSessionInfo().bundleName_) {
        TLOGE(WmsLogTag::WMS_PC, "verify bundle failed, source name is %{public}s but target name is %{public}s",
            sourceSession->GetSessionInfo().bundleName_.c_str(), targetSession->GetSessionInfo().bundleName_.c_str());
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    if (!SessionPermission::IsSameBundleNameAsCalling(targetSession->GetSessionInfo().bundleName_)) {
        TLOGE(WmsLogTag::WMS_PC, "targetSession %{public}d is not same bundle as calling", targetPersistentId);
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    if (callingPid != targetSession->GetCallingPid()) {
        TLOGE(WmsLogTag::WMS_PC, "permission denied, not call by the same process");
        return WMError::WM_ERROR_INVALID_CALLING;
    }
    return ShiftAppWindowPointerEventInner(sourcePersistentId, targetPersistentId,
        targetSession->GetSessionProperty()->GetDisplayId(), fingerId);
}

WMError SceneSessionManager::ShiftAppWindowPointerEventInner(
    int32_t sourceWindowId, int32_t targetWindowId, DisplayId targetDisplayId, int32_t fingerId)
{
    return taskScheduler_->PostSyncTask([sourceWindowId, targetWindowId, targetDisplayId, fingerId] {
        auto display = DisplayManager::GetInstance().GetDisplayById(targetDisplayId);
    	float vpr = 1.5f; // 1.5f: default virtual pixel ratio
    	if (display) {
        	vpr = display->GetVirtualPixelRatio();
    	}
    	int32_t outside = static_cast<int32_t>(HOTZONE_TOUCH * vpr * 2); // double touch hotzone
    	MMI::ShiftWindowParam param;
    	param.sourceWindowId = sourceWindowId;
    	param.targetWindowId = targetWindowId;
    	param.x = -outside;
    	param.y = -outside;
        if (fingerId == INVALID_FINGER_ID) {
            param.sourceType = MMI::PointerEvent::SOURCE_TYPE_MOUSE;
        } else {
            param.sourceType = MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
            param.fingerId = fingerId;
        }
        int ret = MMI::InputManager::GetInstance()->ShiftAppPointerEvent(param, true);
        TLOGNI(WmsLogTag::WMS_PC, "sourceWindowId %{public}d targetWindowId %{public}d vpr %{public}f ret %{public}d",
            param.sourceWindowId, param.targetWindowId, vpr, ret);
        return ret == 0 ? WMError::WM_OK : WMError::WM_ERROR_INVALID_CALLING;
    }, __func__);
}

WMError SceneSessionManager::LockSessionByAbilityInfo(const AbilityInfoBase& abilityInfo, bool isLock)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (!SessionPermission::VerifyCallingPermission(PermissionConstants::PERMISSION_MANAGE_MISSION)) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller has not permission granted");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    TLOGI(WmsLogTag::WMS_LIFE,
        "bundleName:%{public}s moduleName:%{public}s abilityName:%{public}s appIndex:%{public}d isLock:%{public}d",
        abilityInfo.bundleName.c_str(), abilityInfo.moduleName.c_str(), abilityInfo.abilityName.c_str(),
        abilityInfo.appIndex, isLock);
    if (!abilityInfo.IsValid()) {
        TLOGE(WmsLogTag::WMS_LIFE, "abilityInfo not valid");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    taskScheduler_->PostAsyncTask([this, abilityInfo, isLock, where = __func__] {
        std::vector<sptr<SceneSession>> mainSessions;
        GetMainSessionByAbilityInfo(abilityInfo, mainSessions);
        if (!mainSessions.empty()) {
            for (const auto& mainSession : mainSessions) {
                TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s, set isLockedState true, persistentId:%{public}d",
                    where, mainSession->GetPersistentId());
                mainSession->NotifySessionLockStateChange(isLock);
            }
            if (isLock) {
                return;
            }
        }
        if (isLock) {
            if (sessionLockedStateCacheSet_.size() > MAX_LOCK_STATUS_CACHE_SIZE) {
                auto iter = sessionLockedStateCacheSet_.begin();
                TLOGNW(WmsLogTag::WMS_LIFE, "%{public}s, reach max erase begin:%{public}s", where, (*iter).c_str());
                sessionLockedStateCacheSet_.erase(iter);
            }
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s, update isLockedState into cache set", where);
            sessionLockedStateCacheSet_.insert(abilityInfo.ToKeyString());
        } else {
            sessionLockedStateCacheSet_.erase(abilityInfo.ToKeyString());
        }
    }, __func__);
    return WMError::WM_OK;
}

void SceneSessionManager::ConfigSupportFunctionType(SupportFunctionType funcType)
{
    systemConfig_.supportFunctionType_ = funcType;
}

WMError SceneSessionManager::HasFloatingWindowForeground(const sptr<IRemoteObject>& abilityToken, bool& hasOrNot)
{
    if (!abilityToken) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "AbilityToken is null");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_SYSTEM, "Permission denied, only for SA");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }

    return taskScheduler_->PostSyncTask([this, &abilityToken, &hasOrNot, where = __func__] {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, session] : sceneSessionMap_) {
            if (session && session->GetAbilityToken() == abilityToken &&
                session->GetWindowType() == WindowType::WINDOW_TYPE_FLOAT && session->IsSessionForeground()) {
                TLOGNI(WmsLogTag::WMS_SYSTEM, "%{public}s found", where);
                hasOrNot = true;
                return WMError::WM_OK;
            }
        }
        TLOGNI(WmsLogTag::WMS_SYSTEM, "%{public}s not found", where);
        hasOrNot = false;
        return WMError::WM_OK;
    }, __func__);
}

WMError SceneSessionManager::GetFbPanelWindowId(uint32_t& windowId)
{
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& iter : sceneSessionMap_) {
        auto& session = iter.second;
        if (session && session->GetWindowType() == WindowType::WINDOW_TYPE_FB &&
            session->GetWindowName().find(FB_PANEL_NAME) != std::string::npos) {
            windowId = static_cast<uint32_t>(session->GetWindowId());
            return WMError::WM_OK;
        }
    }
    TLOGW(WmsLogTag::WMS_SYSTEM, "No Fb panel");
    return WMError::WM_ERROR_FB_INTERNAL_ERROR;
}

void SceneSessionManager::SetStatusBarAvoidHeight(DisplayId displayId, int32_t height)
{
    const char* const where = __func__;
    auto task = [this, where, displayId, height] {
        statusBarAvoidHeight_[displayId] = height >= 0 ? height : INVALID_STATUS_BAR_AVOID_HEIGHT;
        TLOGNI(WmsLogTag::WMS_IMMS, "%{public}s, displayId %{public}" PRIu64 " height %{public}d",
            where, displayId, statusBarAvoidHeight_[displayId]);
        return WMError::WM_OK;
    };
    taskScheduler_->PostSyncTask(task, where);
}

void SceneSessionManager::GetStatusBarAvoidHeight(DisplayId displayId, WSRect& barArea)
{
    auto it = statusBarAvoidHeight_.find(displayId);
    if (it == statusBarAvoidHeight_.end()) {
        return;
    }
    if (it->second == INVALID_STATUS_BAR_AVOID_HEIGHT) {
        return;
    }
    TLOGD(WmsLogTag::WMS_IMMS, "displayId %{public}" PRIu64 " height %{public}d", displayId, it->second);
    barArea.height_ = it->second;
}

WMError SceneSessionManager::MinimizeAllAppWindows(DisplayId displayId, int32_t excludeWindowId)
{
    if (!SessionPermission::IsSystemCalling() && !SessionPermission::IsStartByHdcd()) {
        TLOGE(WmsLogTag::WMS_LIFE, "Not system app, no right.");
        return WMError::WM_ERROR_NOT_SYSTEM_APP;
    }
    if (!systemConfig_.IsPhoneWindow()) {
        TLOGE(WmsLogTag::WMS_LIFE, "Device not support!");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }
    {
        if (excludeWindowId > 0) {
            std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
            auto iter = sceneSessionMap_.find(excludeWindowId);
            if (iter == sceneSessionMap_.end()) {
                TLOGW(WmsLogTag::WMS_LIFE, "excludeWindowId: %{public}d not exist", excludeWindowId);
                return WMError::WM_ERROR_INVALID_OPERATION;
            }
        }
    }

    const char* const where = __func__;
    taskScheduler_->PostAsyncTask([this, displayId, excludeWindowId, where] {
        if (minimizeAllFunc_) {
            minimizeAllFunc_(displayId, excludeWindowId);
            TLOGI(WmsLogTag::WMS_LIFE, "%{public}s has minimized all windows, except ID: %{public}d.",
                where, excludeWindowId);
        }
    }, __func__);
    return WMError::WM_OK;
}

WSError SceneSessionManager::CloneWindow(int32_t fromPersistentId, int32_t toPersistentId, bool needOffScreen)
{
    auto toSceneSession = GetSceneSession(toPersistentId);
    if (toSceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_PC, "Session is nullptr, id: %{public}d", toPersistentId);
        return WSError::WS_ERROR_NULLPTR;
    }
    NodeId nodeId = INVALID_NODEID;
    if (fromPersistentId >= 0) { // if fromPersistentId < 0, excute CloneWindow(0) to cancel cloneWindow
        auto fromSceneSession = GetSceneSession(fromPersistentId);
        if (fromSceneSession == nullptr) {
            TLOGE(WmsLogTag::WMS_PC, "Session is nullptr, id: %{public}d", fromPersistentId);
            return WSError::WS_ERROR_NULLPTR;
        }
        if (auto surfaceNode = fromSceneSession->GetSurfaceNode()) {
            nodeId = surfaceNode->GetId();
        }
    }
    toSceneSession->CloneWindow(nodeId, needOffScreen);
    TLOGI(WmsLogTag::WMS_PC, "fromSurfaceId: %{public}" PRIu64, nodeId);
    return WSError::WS_OK;
}

DisplayId SceneSessionManager::UpdateSpecificSessionClientDisplayId(const sptr<WindowSessionProperty>& property)
{
    auto initClientDisplayId = DEFAULT_DISPLAY_ID;
    //  SubWindow
    if (auto parentSession = GetSceneSession(property->GetParentPersistentId())) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "displayId:%{public}" PRIu64 ", parentClientDisplayId:%{public}" PRIu64
            ", isFollowParentWindowDisplayId: %{public}u", property->GetDisplayId(),
            parentSession->GetClientDisplayId(), property->IsFollowParentWindowDisplayId());
        initClientDisplayId =
            property->IsFollowParentWindowDisplayId() ? parentSession->GetClientDisplayId() : property->GetDisplayId();
        if (property->GetDisplayId() == VIRTUAL_DISPLAY_ID) {
            property->SetDisplayId(DEFAULT_DISPLAY_ID);
        }
    }
    // SystemWindow
    if (property->GetDisplayId() == VIRTUAL_DISPLAY_ID) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "displayId:%{public}" PRIu64, property->GetDisplayId());
        property->SetDisplayId(DEFAULT_DISPLAY_ID);
        initClientDisplayId = VIRTUAL_DISPLAY_ID;
    }
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "winName:%{public}s, type:%{public}u, displayId:%{public}" PRIu64
        ", clientDisplayId:%{public}" PRIu64 ", parentId:%{public}d", property->GetWindowName().c_str(),
        property->GetWindowType(), property->GetDisplayId(), initClientDisplayId, property->GetParentPersistentId());
    return initClientDisplayId;
}

void SceneSessionManager::UpdateSessionDisplayIdBySessionInfo(
    sptr<SceneSession> sceneSession, const SessionInfo& sessionInfo)
{
    if (sessionInfo.screenId_ != sceneSession->GetScreenId()) {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "%{public}s move display %{public}" PRIu64 " from %{public}" PRIu64,
            sessionInfo.bundleName_.c_str(), sessionInfo.screenId_, sceneSession->GetScreenId());
        sceneSession->SetScreenId(sessionInfo.screenId_);
        sceneSession->GetSessionProperty()->SetDisplayId(sessionInfo.screenId_);
    }
}

void SceneSessionManager::RemoveLifeCycleTaskByPersistentId(int32_t persistentId,
    const LifeCycleTaskType taskType)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session:%{public}d is nullptr", persistentId);
        return;
    }
    sceneSession->RemoveLifeCycleTask(taskType);
}

WMError SceneSessionManager::RegisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener,
    const std::vector<int32_t>& persistentIdList)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (persistentIdList.empty()) {
        TLOGE(WmsLogTag::WMS_LIFE, "persistentIdList is empty!");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is nullptr!");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (listenerController_->IsListenerMapByIdSizeReachLimit()) {
        TLOGW(WmsLogTag::WMS_LIFE, "The number of listeners has reached the upper limit.");
        return WMError::WM_ERROR_NO_MEM;
    }
    return taskScheduler_->PostSyncTask([this, listener, persistentIdList, where = __func__] {
        WMError ret = listenerController_->RegisterSessionLifecycleListener(listener, persistentIdList);
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s, ret:%{public}d", where, ret);
        return ret;
    }, __func__);
}

WMError SceneSessionManager::RegisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener,
    const std::vector<std::string>& bundleNameList)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is nullptr!");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (listenerController_->IsListenerMapByBundleSizeReachLimit(bundleNameList.empty())) {
        TLOGW(WmsLogTag::WMS_LIFE, "The number of listeners has reached the upper limit.");
        return WMError::WM_ERROR_NO_MEM;
    }
    taskScheduler_->PostAsyncTask([this, listener, bundleNameList, where = __func__] {
        WMError ret = listenerController_->RegisterSessionLifecycleListener(listener, bundleNameList);
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s, ret:%{public}d", where, ret);
    }, __func__);
    return WMError::WM_OK;
}

WMError SceneSessionManager::UnregisterSessionLifecycleListener(const sptr<ISessionLifecycleListener>& listener)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is nullptr!");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    taskScheduler_->PostAsyncTask([this, listener, where = __func__] {
        WMError ret = listenerController_->UnregisterSessionLifecycleListener(listener);
        TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s, ret:%{public}d", where, ret);
    }, __func__);
    return WMError::WM_OK;
}

WMError SceneSessionManager::SetParentWindowInner(const sptr<SceneSession>& subSession,
    const sptr<SceneSession>& oldParentSession, const sptr<SceneSession>& newParentSession)
{
    uint32_t oldSubWindowLevel = oldParentSession->GetSessionProperty()->GetSubWindowLevel();
    uint32_t newSubWindowLevel = newParentSession->GetSessionProperty()->GetSubWindowLevel();
    if (oldSubWindowLevel < newSubWindowLevel &&
        subSession->GetMaxSubWindowLevel() + newSubWindowLevel > MAX_SUB_WINDOW_LEVEL) {
        TLOGE(WmsLogTag::WMS_SUB, "newParentSession sub level limit");
        return WMError::WM_ERROR_INVALID_PARENT;
    }
    int32_t oldParentWindowId = oldParentSession->GetPersistentId();
    int32_t newParentWindowId = newParentSession->GetPersistentId();
    subSession->NotifySetParentSession(oldParentWindowId, newParentWindowId);
    int32_t subWindowId = subSession->GetPersistentId();
    oldParentSession->RemoveSubSession(subWindowId);
    newParentSession->AddSubSession(subSession);
    subSession->SetParentSession(newParentSession);
    subSession->SetParentPersistentId(newParentWindowId);
    subSession->UpdateSubWindowLevel(newSubWindowLevel + 1);
    if (oldSubWindowLevel == 0) {
        oldParentSession->UnregisterNotifySurfaceBoundsChangeFunc(subWindowId);
        if (newSubWindowLevel == 0 && subSession->GetIsFollowParentLayout()) {
            subSession->SetFollowParentWindowLayoutEnabled(true);
        }
    }
    if (!oldParentSession->IsSameMainSession(newParentSession) && subSession->IsFocused() &&
        !subSession->GetSessionProperty()->GetExclusivelyHighlighted()) {
        SetHighlightSessionIds(subSession, true, std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    }
    return WMError::WM_OK;
}

WMError SceneSessionManager::SetParentWindow(int32_t subWindowId, int32_t newParentWindowId)
{
    return taskScheduler_->PostSyncTask([this, subWindowId, newParentWindowId, where = __func__] {
        auto subSession = GetSceneSession(subWindowId);
        if (!subSession || !WindowHelper::IsSubWindow(subSession->GetWindowType())) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s subSession is nullptr or type invalid", where);
            return WMError::WM_ERROR_INVALID_WINDOW;
        }
        int32_t oldParentWindowId = subSession->GetParentPersistentId();
        auto oldParentSession = GetSceneSession(oldParentWindowId);
        if (oldParentSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s oldParentSession is nullptr", where);
            return WMError::WM_ERROR_INVALID_PARENT;
        }
        auto oldWindowType = oldParentSession->GetWindowType();
        if (!WindowHelper::IsMainWindow(oldWindowType) && !WindowHelper::IsFloatOrSubWindow(oldWindowType)) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s oldParentSession window type invalid", where);
            return WMError::WM_ERROR_INVALID_PARENT;
        }
        auto newParentSession = GetSceneSession(newParentWindowId);
        if (newParentSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s newParentSession is nullptr", where);
            return WMError::WM_ERROR_INVALID_PARENT;
        }
        TLOGND(WmsLogTag::WMS_SUB, "%{public}s subWindowId: %{public}d oldParentWindowId: %{public}d "
            "newParentWindowId: %{public}d", where, subWindowId, oldParentWindowId, newParentWindowId);
        auto newWindowType = newParentSession->GetWindowType();
        if (!WindowHelper::IsMainWindow(newWindowType) && !WindowHelper::IsFloatOrSubWindow(newWindowType)) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s newParentSession window type invalid", where);
            return WMError::WM_ERROR_INVALID_PARENT;
        }
        if (oldParentSession->GetCallingPid() != newParentSession->GetCallingPid()) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s callingPid not same", where);
            return WMError::WM_ERROR_INVALID_PARENT;
        }
        if (newParentSession->IsAncestorsSession(subWindowId)) {
            TLOGNE(WmsLogTag::WMS_SUB, "%{public}s newParentSession is subsession ancestor", where);
            return WMError::WM_ERROR_INVALID_PARENT;
        }
        return SetParentWindowInner(subSession, oldParentSession, newParentSession);
    });
}

void SceneSessionManager::NotifyWindowSystemBarPropertyChange(
    WindowType type, const SystemBarProperty& systemBarProperty)
{
    {
        std::lock_guard<std::mutex> lock(lastSystemBarPropertyMapMutex_);
        auto iter = lastSystemBarPropertyMap_.find(type);
        if (iter != lastSystemBarPropertyMap_.end() && iter->second == systemBarProperty) {
            TLOGD(WmsLogTag::WMS_IMMS, "type %{public}d prop same as last time.", type);
            return;
        }
        lastSystemBarPropertyMap_[type] = systemBarProperty;
    }
    SessionManagerAgentController::GetInstance().NotifyWindowSystemBarPropertyChange(type, systemBarProperty);
}

WMError SceneSessionManager::MinimizeByWindowId(const std::vector<int32_t>& windowIds)
{
    if (!SessionPermission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is not system service.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (windowIds.empty()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The vector of windowids is empty.");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    taskScheduler_->PostAsyncTask([this, windowIds, where = __func__]() {
        if (minimizeByWindowIdFunc_) {
            TLOGNI(WmsLogTag::WMS_PC, "%{public}s", where);
            minimizeByWindowIdFunc_(windowIds);
        }
    }, __func__);
    return WMError::WM_OK;
}

void SceneSessionManager::RegisterMinimizeByWindowIdCallback(MinimizeByWindowIdFunc&& func){
    minimizeByWindowIdFunc_ = std::move(func);
}

WMError SceneSessionManager::UpdateAnimationSpeedWithPid(pid_t pid, float speed)
{
    if (!SessionPermission::IsSystemServiceCalling()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "The caller is not system service.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, session] : sceneSessionMap_) {
        if (session && session->GetCallingPid() == pid && session->IsVisible()) {
            session->UpdateAnimationSpeed(speed);
            return WMError::WM_OK;
        }
    }
    return WMError::WM_OK;
}

const std::vector<sptr<SceneSession>> SceneSessionManager::GetActiveSceneSessionCopy()
{
    std::map<int32_t, sptr<SceneSession>> sceneSessionMapCopy;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        sceneSessionMapCopy = sceneSessionMap_;
    }
    std::vector<sptr<SceneSession>> activeSession;
    for (const auto& elem : sceneSessionMapCopy) {
        auto curSession = elem.second;
        if (curSession == nullptr) {
            TLOGW(WmsLogTag::DEFAULT, "curSession nullptr");
            continue;
        }
        if (curSession->GetSessionInfo().isSystem_ ||
            (!curSession->IsSessionForeground())) {
             continue;
         }
        activeSession.push_back(curSession);
    }
    return activeSession;
}

void SceneSessionManager::RegisterHookSceneSessionActivationFunc(const sptr<SceneSession>& sceneSession)
{
    TLOGI(WmsLogTag::WMS_LIFE, "in");
    sceneSession->HookSceneSessionActivation([](const sptr<SceneSession>& session, bool isNewWant) {
        SceneSessionManager::GetInstance().RequestSceneSessionActivation(session, isNewWant);
    });
}

void SceneSessionManager::ReportKeyboardCreateException(sptr<SceneSession>& keyboardSession) {
    std::string msg = "UITYPE:" + std::to_string(static_cast<int32_t>(systemConfig_.windowUIType_)) + ",PanelId:[";
    for (const auto& session : GetSceneSessionVectorByType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL)) {
        if (!session) {
            TLOGW(WmsLogTag::DEFAULT, "session nullptr");
            continue;
        }
        msg +=  "PanelId:" + std::to_string(session->GetPersistentId()) + ",";
        msg +=  "screenId:" + std::to_string(session->GetScreenId()) + ",";
    }
    msg += "],";
    for (const auto& session : GetSceneSessionVectorByType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT)) {
        if (!session) {
            TLOGW(WmsLogTag::DEFAULT, "session nullptr");
            continue;
        }
        msg += "keyboardId:" + std::to_string(session->GetPersistentId()) + ",";
        msg += "screenId:" + std::to_string(session->GetScreenId()) + ",";
    }
    WindowInfoReporter::GetInstance().ReportKeyboardLifeCycleException(
        keyboardSession->GetPersistentId(),
        KeyboardLifeCycleException::CREATE_EXCEPTION,
        msg);
}

void SceneSessionManager::RegisterSceneSessionDestructCallback(NotifySceneSessionDestructFunc&& func)
{
    onSceneSessionDestruct_ = std::move(func);
}

void SceneSessionManager::RegisterSceneSessionDestructNotifyManagerFunc(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "session is nullptr");
        return;
    }
    sceneSession->SetSceneSessionDestructNotificationFunc([this](int32_t persistentId) {
        if (onSceneSessionDestruct_) {
            onSceneSessionDestruct_(persistentId);
        }
    });
}

void SceneSessionManager::RegisterSessionPropertyChangeNotifyManagerFunc(const sptr<SceneSession>& sceneSession)
{
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "session is nullptr");
        return;
    }
    sceneSession->SetSessionPropertyChangeNotifyManagerListener(
        [this](int32_t persistentId, WindowInfoKey windowInfoKey) {
        NotifySessionPropertyChangeFromSession(persistentId, windowInfoKey);
    });
}

void SceneSessionManager::NotifySessionPropertyChangeFromSession(int32_t persistentId, WindowInfoKey windowInfoKey)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "persistentId: %{public}d, windowInfoKey: %{public}u",
        persistentId, static_cast<uint32_t>(windowInfoKey));
    sptr<SceneSession> sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "sceneSession nullptr");
        return;
    }
    NotifyWindowPropertyChangeByWindowInfoKey(sceneSession, windowInfoKey);
}

void SceneSessionManager::ConfigSupportZLevel()
{
    TLOGI(WmsLogTag::WMS_HIERARCHY, "support zLevel");
    auto task = [this] {
        systemConfig_.supportZLevel_ = true;
    };
    taskScheduler_->PostAsyncTask(task, "ConfigSupportZLevel");
}

void SceneSessionManager::NotifyWindowPropertyChangeByWindowInfoKey(
    const sptr<SceneSession>& sceneSession, WindowInfoKey windowInfoKey)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "windowInfoKey: %{public}u", static_cast<uint32_t>(windowInfoKey));
    WindowInfoList windowInfoList;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowPropertyChangeInfo;
    PackWindowPropertyChangeInfo(sceneSession, windowPropertyChangeInfo);
    windowInfoList.push_back(windowPropertyChangeInfo);
    SessionManagerAgentController::GetInstance().NotifyWindowPropertyChange(
        static_cast<uint32_t>(windowInfoKey), windowInfoList);
}

void SceneSessionManager::NotifyWindowPropertyChange(ScreenId screenId)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "ObservedFlags: %{public}u, interestedFlags: %{public}u",
        observedFlags_, interestedFlags_);
    WindowInfoList windowInfoList;
    uint32_t propertyDirtyFlags = 0;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        for (const auto& [_, sceneSession] : sceneSessionMap_) {
            if (sceneSession == nullptr) {
                TLOGW(WmsLogTag::WMS_ATTRIBUTE, "sceneSession nullptr");
                continue;
            }
            if (isNotCurrentScreen(sceneSession, screenId)) {
                continue;
            }
            if (!(sceneSession->GetPropertyDirtyFlags() & observedFlags_)) {
                continue;
            }
            propertyDirtyFlags |= sceneSession->GetPropertyDirtyFlags();
            std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowPropertyChangeInfo;
            PackWindowPropertyChangeInfo(sceneSession, windowPropertyChangeInfo);
            windowInfoList.emplace_back(windowPropertyChangeInfo);
            sceneSession->SetPropertyDirtyFlags(0);
        }
    }
    SessionManagerAgentController::GetInstance().NotifyWindowPropertyChange(propertyDirtyFlags, windowInfoList);
}

void SceneSessionManager::PackWindowPropertyChangeInfo(const sptr<SceneSession>& sceneSession,
    std::unordered_map<WindowInfoKey, WindowChangeInfoType>& windowPropertyChangeInfo)
{
    if (interestedFlags_ & static_cast<uint32_t>(SessionPropertyFlag::WINDOW_ID)) {
        windowPropertyChangeInfo[WindowInfoKey::WINDOW_ID] = sceneSession->GetWindowId();
    }
    if (interestedFlags_ & static_cast<uint32_t>(SessionPropertyFlag::BUNDLE_NAME)) {
        windowPropertyChangeInfo[WindowInfoKey::BUNDLE_NAME] = sceneSession->GetSessionInfo().bundleName_;
    }
    if (interestedFlags_ & static_cast<uint32_t>(SessionPropertyFlag::ABILITY_NAME)) {
        windowPropertyChangeInfo[WindowInfoKey::ABILITY_NAME] = sceneSession->GetSessionInfo().abilityName_;
    }
    if (interestedFlags_ & static_cast<uint32_t>(SessionPropertyFlag::APP_INDEX)) {
        windowPropertyChangeInfo[WindowInfoKey::APP_INDEX] = sceneSession->GetSessionInfo().appIndex_;
    }
    if (interestedFlags_ & static_cast<uint32_t>(SessionPropertyFlag::VISIBILITY_STATE)) {
        windowPropertyChangeInfo[WindowInfoKey::VISIBILITY_STATE] = sceneSession->GetVisibilityState();
    }
    if (interestedFlags_ & static_cast<uint32_t>(SessionPropertyFlag::DISPLAY_ID)) {
        if (PcFoldScreenManager::GetInstance().IsHalfFoldedOnMainDisplay(
            sceneSession->GetSessionProperty()->GetDisplayId())) {
            WSRect sessionGlobalRect = sceneSession->GetSessionGlobalRect();
            windowPropertyChangeInfo[WindowInfoKey::DISPLAY_ID] =
                sceneSession->TransformGlobalRectToRelativeRect(sessionGlobalRect);
        } else {
            windowPropertyChangeInfo[WindowInfoKey::DISPLAY_ID] = sceneSession->GetSessionProperty()->GetDisplayId();
        }
    }
    if (interestedFlags_ & static_cast<uint32_t>(SessionPropertyFlag::WINDOW_RECT)) {
        WSRect wsrect = sceneSession->GetSessionRect();
        Rect rect = { wsrect.posX_, wsrect.posY_, wsrect.width_, wsrect.height_ };
        windowPropertyChangeInfo[WindowInfoKey::WINDOW_RECT] = rect;
    }
    if (interestedFlags_ & static_cast<uint32_t>(SessionPropertyFlag::WINDOW_MODE)) {
        windowPropertyChangeInfo[WindowInfoKey::WINDOW_MODE] = sceneSession->GetWindowMode();
    }
    if (interestedFlags_ & static_cast<uint32_t>(SessionPropertyFlag::FLOATING_SCALE)) {
        windowPropertyChangeInfo[WindowInfoKey::FLOATING_SCALE] = sceneSession->GetFloatingScale();
    }
    if (interestedFlags_ & static_cast<uint32_t>(SessionPropertyFlag::MID_SCENE)) {
        windowPropertyChangeInfo[WindowInfoKey::MID_SCENE] = sceneSession->GetIsMidScene();
    }
    if (interestedFlags_ & static_cast<uint32_t>(SessionPropertyFlag::WINDOW_GLOBAL_RECT)) {
        WSRect wsrect = sceneSession->GetSessionGlobalRect();
        Rect globalRect = { wsrect.posX_, wsrect.posY_, wsrect.width_, wsrect.height_ };
        windowPropertyChangeInfo[WindowInfoKey::WINDOW_GLOBAL_RECT] = globalRect;
    }
}

WSError SceneSessionManager::UseImplicitAnimation(int32_t hostWindowId, bool useImplicit)
{
    TLOGI(WmsLogTag::WMS_UIEXT, "hostWindowId:%{public}d, useImplicit:%{public}d", hostWindowId, useImplicit);
    auto task = [this, hostWindowId, useImplicit]() {
        auto sceneSession = GetSceneSession(hostWindowId);
        if (sceneSession == nullptr) {
            TLOGNE(WmsLogTag::WMS_UIEXT, "Session with persistentId %{public}d not found", hostWindowId);
            return WSError::WS_ERROR_INVALID_SESSION;
        }
        return sceneSession->UseImplicitAnimation(useImplicit);
    };

    return taskScheduler_->PostSyncTask(task, "UseImplicitAnimation");
}

WSError SceneSessionManager::GetRecentMainSessionInfoList(std::vector<RecentSessionInfo>& recentSessionInfoList)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WSError::WS_ERROR_INVALID_PERMISSION;
    }
    if (!recentSessionInfoList.empty()) {
        TLOGE(WmsLogTag::WMS_LIFE, "the recent session info list is already loaded");
        return WSError::WS_ERROR_INVALID_PARAM;
    }
    return taskScheduler_->PostSyncTask([this, &recentSessionInfoList, where = __func__] {
        for (auto& recentSessionInfo : this->recentMainSessionInfoList_) {
            if (auto session = GetMainSessionByPersistentId(recentSessionInfo.missionId)) {
                session->SetRecentSessionState(recentSessionInfo, session->GetSessionState());
                TLOGI(WmsLogTag::WMS_LIFE, "get recent main session info of id:%{public}d, bundleName:%{public}s, "
                    "moduleName:%{public}s, abilityName:%{public}s, sessionState:%{public}d, windowType:%{public}d",
                    recentSessionInfo.missionId, recentSessionInfo.bundleName.c_str(),
                    recentSessionInfo.moduleName.c_str(), recentSessionInfo.abilityName.c_str(),
                    recentSessionInfo.sessionState, recentSessionInfo.windowType);
                recentSessionInfoList.emplace_back(recentSessionInfo);
            }
        }
        return WSError::WS_OK;
    });
}

void SceneSessionManager::UpdateRecentMainSessionInfos(const std::vector<int32_t>& recentMainSessionIdList)
{
    taskScheduler_->PostAsyncTask([this, recentMainSessionIdList, where = __func__]() {
        this->recentMainSessionInfoList_.clear();

        for (int32_t persistentId : recentMainSessionIdList) {
            if (auto session = GetMainSessionByPersistentId(persistentId)) {
                const auto& sessionInfo  = session->GetSessionInfo();
                RecentSessionInfo info(persistentId);
                info.bundleName = sessionInfo.bundleName_;
                info.moduleName = sessionInfo.moduleName_;
                info.abilityName = sessionInfo.abilityName_;
                info.appIndex = sessionInfo.appIndex_;
                info.windowType = session->GetWindowType();

                this->recentMainSessionInfoList_.emplace_back(info);
            }
        }
    }, __func__);
}

void SceneSessionManager::RegisterTransferSessionToTargetScreenCallback(NotifyTransferSessionToTargetScreenFunc&& func)
{
    taskScheduler_->PostAsyncTask([this, func] {
        onTransferSessionToTargetScreen_ = std::move(func);
    }, __func__);
}

WMError SceneSessionManager::NotifyTransferSessionToTargetScreen(const TransferSessionInfo& info)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (info.persistentId < 0 || info.toScreenId < 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "Param is invalid!");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    taskScheduler_->PostAsyncTask([this, info, where = __func__]() {
        if (onTransferSessionToTargetScreen_) {
            TLOGNI(WmsLogTag::WMS_LIFE, "%{public}s, persistentId: %{public}d toScreenId: %{public}d",
                where, info.persistentId, info.toScreenId);
            onTransferSessionToTargetScreen_(info);
        }
    }, __func__);
    return WMError::WM_OK;
}

void SceneSessionManager::NotifySessionTransferToTargetScreenEvent(const int32_t persistentId,
    const uint32_t resultCode, const uint64_t fromScreenId, const uint64_t toScreenId, LifeCycleChangeReason reason)
{
    auto sceneSession = GetSceneSession(persistentId);
    if (sceneSession == nullptr) {
        TLOGE(WmsLogTag::WMS_MAIN, "sceneSession is nullptr");
        return;
    }
    listenerController_->NotifySessionTransferToTargetScreenEvent(
        sceneSession->GetSessionInfo(), resultCode, fromScreenId, toScreenId, reason);
}

WMError SceneSessionManager::AnimateTo(int32_t windowId, const WindowAnimationProperty& animationProperty,
        const WindowAnimationOption& animationOption)
{
    if (!SessionPermission::IsSACalling() && !SessionPermission::IsShellCall()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Permission denied, only for SA");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    auto session = GetSceneSession(windowId);
    if (session == nullptr || SessionHelper::IsSystemWindow(session->GetWindowType())) {
        TLOGNE(WmsLogTag::WMS_ANIMATION, "Can not find window or is system window");
        return WMError::WM_DO_NOTHING;
    }
    session->AnimateTo(animationProperty, animationOption);
    return WMError::WM_OK;
}

WMError SceneSessionManager::GetRouterStackInfo(int32_t persistentId,
    const sptr<ISessionRouterStackListener>& listener)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (listener == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "listener is nullptr.");
        return WMError::WM_ERROR_NULLPTR;
    }
    auto session = GetSceneSession(persistentId);
    if (session == nullptr) {
        TLOGE(WmsLogTag::WMS_LIFE, "Session with persistentId %{public}d not found", persistentId);
        return WMError::WM_ERROR_NULLPTR;
    }
    auto task = [this, weakSceneSession = wptr(session), listener, where = __func__] {
        sptr<SceneSession> session = weakSceneSession.promote();
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "session is nullptr");
            return;
        }
        std::string routerStack;
        auto ret = session->GetRouterStackInfo(routerStack);
        sptr<RouterStackInfo> routerStackInfo = sptr<RouterStackInfo>::MakeSptr(session->GetPersistentId(), routerStack);
        if (ret != WMError::WM_OK) {
            TLOGNE(WmsLogTag::WMS_LIFE, "failed, persistentId %{public}d", session->GetPersistentId());
            routerStackInfo->errCode_ = ret;
            listener->SendRouterStackInfo(routerStackInfo);
            return;
        }
        listener->SendRouterStackInfo(routerStackInfo);
    };
    ffrtQueueHelper_->SubmitTask(task);
    return WMError::WM_OK;
}

WMError SceneSessionManager::CreateNewInstanceKey(const std::string& bundleName, std::string& instanceKey)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (bundleName.empty()) {
        TLOGE(WmsLogTag::WMS_LIFE, "empty bundleName");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    uint32_t maxInstanceCount = MultiInstanceManager::GetInstance().GetMaxInstanceCount(bundleName);
    uint32_t instanceCount = GetInstanceCount(bundleName);
    if (instanceCount < maxInstanceCount) {
        instanceKey = MultiInstanceManager::GetInstance().CreateNewInstanceKey(bundleName);
    } else {
        instanceKey = MultiInstanceManager::GetInstance().GetLastInstanceKey(bundleName);
    }
    TLOGI(WmsLogTag::WMS_LIFE, "create new instanceKey:%{public}s of bundle:%{public}s",
        instanceKey.c_str(), bundleName.c_str());
    return WMError::WM_OK;
}

WMError SceneSessionManager::RemoveInstanceKey(const std::string& bundleName, const std::string& instanceKey)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (bundleName.empty()) {
        TLOGE(WmsLogTag::WMS_LIFE, "empty bundleName");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    if (instanceKey.empty()) {
        TLOGE(WmsLogTag::WMS_LIFE, "empty instanceKey");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    auto task = [bundleName, instanceKey] {
        MultiInstanceManager::GetInstance().DecreaseInstanceKeyRefCountByBundleNameAndInstanceKey(
            bundleName, instanceKey);
    };
    taskScheduler_->PostAsyncTask(task, __func__);
    TLOGI(WmsLogTag::WMS_LIFE, "remove instanceKey:%{public}s of bundle:%{public}s",
        instanceKey.c_str(), bundleName.c_str());
    return WMError::WM_OK;
}

WMError SceneSessionManager::UpdateKioskAppList(const std::vector<std::string>& kioskAppList)
{
    TLOGI(WmsLogTag::WMS_LIFE, "kioskAppList size: %{public}zu", kioskAppList.size());
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    if (kioskAppList.empty()) {
        TLOGE(WmsLogTag::WMS_LIFE, "empty kioskAppList");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    taskScheduler_->PostAsyncTask([this, kioskAppList] {
        if (updateKioskAppListFunc_ != nullptr) {
            updateKioskAppListFunc_(kioskAppList);
            return WMError::WM_OK;
        }
        kioskAppListCache_ = kioskAppList;
        return WMError::WM_OK;
    }, __func__);
    return WMError::WM_OK;
}

void SceneSessionManager::RegisterUpdateKioskAppListCallback(UpdateKioskAppListFunc&& func)
{
    taskScheduler_->PostAsyncTask([this, callback = std::move(func)] {
        updateKioskAppListFunc_ = std::move(callback);
        if (!kioskAppListCache_.empty()) {
            updateKioskAppListFunc_(kioskAppListCache_);
        }
    }, __func__);
}

WMError SceneSessionManager::EnterKioskMode(const sptr<IRemoteObject>& token)
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    taskScheduler_->PostAsyncTask([this, token, where = __func__] {
        auto session = FindSessionByToken(token, WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
        if (session == nullptr) {
            TLOGNE(WmsLogTag::WMS_LIFE, "token is invalid");
            return WMError::WM_ERROR_INVALID_PARAM;
        }
        if (kioskModeChangeFunc_ != nullptr) {
            kioskModeChangeFunc_(true, session->GetPersistentId());
            return WMError::WM_OK;
        }
        isKioskMode_ = true;
        kioskAppPersistentId_ = session->GetPersistentId();
        return WMError::WM_OK;
    }, __func__);
    return WMError::WM_OK;
}

WMError SceneSessionManager::ExitKioskMode()
{
    if (!SessionPermission::IsSystemAppCall() && !SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_LIFE, "The caller is neither a system app nor an SA.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }
    taskScheduler_->PostAsyncTask([this, where = __func__] {
        if (kioskModeChangeFunc_ != nullptr) {
            kioskModeChangeFunc_(false, INVALID_SESSION_ID);
            return WMError::WM_OK;
        }
        isKioskMode_ = false;
        kioskAppPersistentId_ = INVALID_SESSION_ID;
        return WMError::WM_OK;
    }, __func__);
    return WMError::WM_OK;
}

void SceneSessionManager::RegisterKioskModeChangeCallback(KioskModeChangeFunc&& func)
{
    taskScheduler_->PostAsyncTask([this, callback = std::move(func)] {
        kioskModeChangeFunc_ = std::move(callback);
        kioskModeChangeFunc_(isKioskMode_, kioskAppPersistentId_);
    }, __func__);
}

std::vector<sptr<SceneSession>> SceneSessionManager::GetSceneSessions(ScreenId screenId)
{
    std::vector<sptr<SceneSession>> sceneSessions;
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession != nullptr && sceneSession->GetScreenId() == screenId) {
            sceneSessions.emplace_back(sceneSession);
        }
    }
    TLOGD(WmsLogTag::WMS_LAYOUT, "screenId: %{public}" PRIu64 ", sceneSession count: %{public}zu",
        screenId, sceneSessions.size());
    return sceneSessions;
}

void SceneSessionManager::SetPiPSettingSwitchStatus(bool switchStatus)
{
    std::lock_guard<std::mutex> lock(pipSettingSwitchMutex_);
    pipSwitchStatus_ = switchStatus;
}

WMError SceneSessionManager::GetPiPSettingSwitchStatus(bool& switchStatus)
{
    std::lock_guard<std::mutex> lock(pipSettingSwitchMutex_);
    switchStatus = pipSwitchStatus_;
    return WMError::WM_OK;
}

void SceneSessionManager::SetIsPipEnabled(bool isPipEnabled)
{
    pipIsPipEnabled_ = isPipEnabled;
}

WMError SceneSessionManager::GetIsPipEnabled(bool& isPipEnabled)
{
    isPipEnabled = pipIsPipEnabled_;
    return WMError::WM_OK;
}

WMError SceneSessionManager::SetPipEnableByScreenId(int32_t screenId, bool isEnabled)
{
    TLOGI(WmsLogTag::WMS_PIP, "SetPipEnableByScreenId: %{public}d, isEnable: %{public}d", screenId, isEnabled);
    std::unique_lock<std::shared_mutex> lock(screenPipEnabledMapLock_);
    screenPipEnabledMap_.insert_or_assign(screenId, isEnabled);
    return WMError::WM_OK;
}

WMError SceneSessionManager::UnsetPipEnableByScreenId(int32_t screenId)
{
    TLOGI(WmsLogTag::WMS_PIP, "UnsetPipEnableByScreenId: %{public}d", screenId);
    std::unique_lock<std::shared_mutex> lock(screenPipEnabledMapLock_);
    screenPipEnabledMap_.erase(screenId);
    return WMError::WM_OK;
}

bool SceneSessionManager::GetPipDeviceCollaborationPolicy(int32_t screenId)
{
    TLOGI(WmsLogTag::WMS_PIP, "GetPipDeviceCollaborationPolicy: %{public}d", screenId);
    std::shared_lock<std::shared_mutex> lock(screenPipEnabledMapLock_);
    auto iter = screenPipEnabledMap_.find(screenId);
    if (iter == screenPipEnabledMap_.end()) {
        return true; // 默认支持
    }
    return iter->second;
}

WMError SceneSessionManager::RegisterPipChgListenerByScreenId(int32_t screenId,
    const sptr<IPipChangeListener>& listener)
{
    TLOGI(WmsLogTag::WMS_PIP, "RegisterPipChgListenerByScreenId: %{public}d", screenId);
    if (listener == nullptr) {
        TLOGNE(WmsLogTag::WMS_LIFE, "listener is nullptr");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    std::unique_lock<std::shared_mutex> lock(pipChgListenerMapMutex_);
    pipChgListenerMap_.insert_or_assign(screenId, listener);
    return WMError::WM_OK;
}

WMError SceneSessionManager::UnregisterPipChgListenerByScreenId(int32_t screenId)
{
    TLOGI(WmsLogTag::WMS_PIP, "UnregisterPipChgListenerByScreenId: %{public}d", screenId);
    std::unique_lock<std::shared_mutex> lock(pipChgListenerMapMutex_);
    pipChgListenerMap_.erase(screenId);
    return WMError::WM_OK;
}

WMError SceneSessionManager::UpdateScreenLockState(int32_t persistentId)
{
    if (persistentId < 0) {
        TLOGE(WmsLogTag::WMS_LIFE, "Param is invalid!");
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    taskScheduler_->PostAsyncTask([this, persistentId, where = __func__]() {
        auto sceneSession = GetSceneSession(persistentId);
        if (sceneSession != nullptr) {
            HandleKeepScreenOn(sceneSession, sceneSession->IsKeepScreenOn(), WINDOW_SCREEN_LOCK_PREFIX,
                               sceneSession->keepScreenLock_);
        }
    }, __func__);
    return WMError::WM_OK;
}

void SceneSessionManager::NotifySessionScreenLockedChange(bool isScreenLocked) {
    if (!systemConfig_.freeMultiWindowSupport_) {
        return;
    }
    const bool isPcMode = system::GetBoolParameter("persist.sceneboard.ispcmode", false);
    const bool isShow = !(isScreenLocked && !isPcMode);
    std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
    for (const auto& [_, sceneSession] : sceneSessionMap_) {
        if (sceneSession == nullptr || !(sceneSession->GetSessionProperty()->GetWindowFlags() &
            static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED)) ||
            (isScreenLocked && !sceneSession->IsSessionForeground())) {
            continue;
        }
        sceneSession->GetSessionProperty()->SetIsShowDecorInFreeMultiWindow(isShow);
        sceneSession->SetIsShowDecorInFreeMultiWindow(isShow);
    }
}

bool SceneSessionManager::NeedOutline(int32_t persistentId, const std::vector<int32_t>& persistentIdList)
{
    if (std::find(persistentIdList.begin(), persistentIdList.end(), persistentId) != persistentIdList.end()) {
        return true;
    }
    return false;
}

bool SceneSessionManager::CacheOutlineParamsIfNeed(const OutlineParams& outlineParams)
{
    if (recoverState_ == RecoverState::RECOVER_INITIAL) {
        recoverOutlineParams_= outlineParams;
        needRecoverOutline_ = true;
        TLOGI(WmsLogTag::WMS_ANIMATION, "Recovering has not finished, cache outline params.");
        return true;
    }
    return false;
}

WMError SceneSessionManager::UpdateOutline(const sptr<IRemoteObject>& remoteObject, const OutlineParams& outlineParams)
{
    TLOGI(WmsLogTag::WMS_ANIMATION, "%{public}s", outlineParams.ToString().c_str());
    if (!SessionPermission::IsSACalling()) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Update outline permission denied.");
        return WMError::WM_ERROR_INVALID_PERMISSION;
    }

    if (!(systemConfig_.IsPcWindow() || systemConfig_.IsFreeMultiWindowMode())) {
        // The isFreeMultiWindowMode has not recovered in pcmode device, cache the outline params.
        bool shouldCacheOutlineParams = CacheOutlineParamsIfNeed(outlineParams);
        if (shouldCacheOutlineParams) {
            AddOutlineRemoteDeathRecipient(remoteObject);
            return WMError::WM_OK;
        }
        TLOGE(WmsLogTag::WMS_ANIMATION, "This device can not update outline.");
        return WMError::WM_ERROR_DEVICE_NOT_SUPPORT;
    }

    UpdateOutlineInner(remoteObject, outlineParams);
    return WMError::WM_OK;
}

void SceneSessionManager::UpdateOutlineInner(const sptr<IRemoteObject>& remoteObject,
                                             const OutlineParams& outlineParams)
{
    taskScheduler_->PostAsyncTask([this, remoteObject, outlineParams]() {
        std::map<int32_t, sptr<SceneSession>> sceneSessionMapCopy;
        {
            std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
            sceneSessionMapCopy = sceneSessionMap_;
        }

        for (const auto& [persistentId, session] : sceneSessionMapCopy) {
            if (session == nullptr) {
                TLOGNI(WmsLogTag::WMS_ANIMATION, "session is null, id: %{public}d.", persistentId);
                continue;
            }
            SessionState state = session->GetSessionState();
            if (state <= SessionState::STATE_DISCONNECT || state >= SessionState::STATE_END) {
                TLOGND(WmsLogTag::WMS_ANIMATION, "session state: %{public}d invalid, id: %{public}d.",
                      state, persistentId);
                continue;
            }
            if (NeedOutline(persistentId, outlineParams.persistentIds_)) {
                session->UpdateSessionOutline(true, outlineParams.outlineStyleParams_);
            } else {
                OutlineStyleParams defaultParams;
                session->UpdateSessionOutline(false, defaultParams);
            }
        }
        AddOutlineRemoteDeathRecipient(remoteObject);
        CacheOutlineParamsIfNeed(outlineParams);
    }, __func__);
}

void SceneSessionManager::AddOutlineRemoteDeathRecipient(const sptr<IRemoteObject>& remoteObject)
{
    if (remoteObject == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Remote object is null.");
        return;
    }
    if (outlineRemoteObject_ == remoteObject) {
        TLOGI(WmsLogTag::WMS_ANIMATION, "This remote object has already registered.");
        return;
    }
    if (outlineRemoteObject_ && !outlineRemoteObject_->RemoveDeathRecipient(outlineRemoteDeath_)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Remove the old remote object's death recipient failed.");
        return;
    }
    outlineRemoteObject_ = remoteObject;
    if (!outlineRemoteObject_->AddDeathRecipient(outlineRemoteDeath_)) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Add death recipient failed.");
        return;
    }
}

void SceneSessionManager::DeleteAllOutline(const sptr<IRemoteObject>& remoteObject)
{
    if (outlineRemoteObject_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "Outline remote object is nullptr.");
        return;
    }
    if (remoteObject != outlineRemoteObject_) {
        TLOGE(WmsLogTag::WMS_ANIMATION, "This is not outline remote object died.");
        return;
    }
    std::map<int32_t, sptr<SceneSession>> sceneSessionMapCopy;
    {
        std::shared_lock<std::shared_mutex> lock(sceneSessionMapMutex_);
        sceneSessionMapCopy = sceneSessionMap_;
    }
    for (const auto& [persistentId, session] : sceneSessionMapCopy) {
        if (session == nullptr) {
            TLOGI(WmsLogTag::WMS_ANIMATION, "invalid session, id: %{public}d.", persistentId);
            continue;
        }
        SessionState state = session->GetSessionState();
        if (state <= SessionState::STATE_DISCONNECT || state >= SessionState::STATE_END) {
            TLOGD(WmsLogTag::WMS_ANIMATION, "session state: %{public}d invalid, id: %{public}d.", state, persistentId);
            continue;
        }
        OutlineStyleParams defaultParams;
        session->UpdateSessionOutline(false, defaultParams);
    }
    outlineRemoteObject_ = nullptr;
}

void SceneSessionManager::NotifyIsFullScreenInForceSplitMode(uint32_t uid, bool isFullScreen)
{
    if (isFullScreen) {
        std::unique_lock lock(appHookWindowInfoMapMutex_);
        fullScreenInForceSplitUidSet_.insert(uid);
    } else {
        std::unique_lock lock(appHookWindowInfoMapMutex_);
        fullScreenInForceSplitUidSet_.erase(uid);
    }
    ScreenSessionManagerClient::GetInstance().NotifyIsFullScreenInForceSplitMode(uid, isFullScreen);
}

void SceneSessionManager::RegisterVirtualPixelChangeCallback(NotifyVirtualPixelChangeFunc&& func)
{
    taskScheduler_->PostAsyncTask([this, func] {
        onVirtualPixelChangeCallback_ = std::move(func);
    }, __func__);
}

void SceneSessionManager::RegisterAppStateObserver()
{
    auto sysMgrClient = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMgrClient == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "sysMgrClient is nullptr");
        return;
    }
    auto sysAbilityObj = sysMgrClient->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (sysAbilityObj == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "sysAbilityObj is nullptr");
        return;
    }
    appMgrClient_ = iface_cast<AppExecFwk::IAppMgr>(sysAbilityObj);
    if (appMgrClient_ == nullptr) {
        TLOGE(WmsLogTag::WMS_ATTRIBUTE, "appMgrClient is nullptr");
        return;
    }
    appStateObserver_ = sptr<AppStateObserver>::MakeSptr();
    appStateObserver_->RegisterProcessDiedNotifyFunc([this](const AppExecFwk::ProcessData& processData) {
        ClearProcessRecordWhenAppExit(processData);
    });
    auto ret = appMgrClient_->RegisterApplicationStateObserver(appStateObserver_);
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "register observer result=%{public}d", ret);
}

void AppStateObserver::OnProcessDied(const AppExecFwk::ProcessData& processData)
{
    TLOGD(WmsLogTag::WMS_ATTRIBUTE, "pid=%{public}d, bundleName=%{public}s, hasCallback=%{public}d",
        processData.pid, processData.bundleName.c_str(), procDiedCallback_ != nullptr);
    if (procDiedCallback_) {
        procDiedCallback_(processData);
    }
}

void AppStateObserver::RegisterProcessDiedNotifyFunc(NotifyAppProcessDiedFunc&& func)
{
    TLOGI(WmsLogTag::WMS_ATTRIBUTE, "register");
    procDiedCallback_ = std::move(func);
}

void SceneSessionManager::UpdateAppBoundSystemTrayStatus(const std::string &key, int32_t pid, bool enabled)
{
    if (appsWithBoundSystemTrayMap_.size() >= MAX_APP_BOUND_TRAY_MAP_SIZE) {
        TLOGE(WmsLogTag::WMS_MAIN, "App bound tray map size exceeds %{public}zu.", MAX_APP_BOUND_TRAY_MAP_SIZE);
        return;
    }
    auto &pidSet = appsWithBoundSystemTrayMap_[key];

    if (enabled) {
        if (pidSet.find(pid) == pidSet.end()) {
            pidSet.insert(pid);
        } else {
            TLOGE(WmsLogTag::WMS_MAIN, "appsWithBoundSystemTrayMap_ has pid: %{public}d", pid);
        }
    } else {
        if (pidSet.erase(pid) > 0) {
            if (pidSet.empty()) {
                appsWithBoundSystemTrayMap_.erase(key);
            }
        } else {
            TLOGE(WmsLogTag::WMS_MAIN, "appsWithBoundSystemTrayMap_ does not have pid: %{public}d", pid);
        }
    }
}

bool SceneSessionManager::IsAppBoundSystemTray(int32_t callingPid, uint32_t callingToken, const std::string &instanceKey)
{
    std::string combinedKey = std::to_string(callingToken) + "-" + instanceKey;
    auto it = appsWithBoundSystemTrayMap_.find(combinedKey);
    if (it != appsWithBoundSystemTrayMap_.end()) {
        return it->second.count(callingPid) > 0;
    }
    return false;
}

WMError SceneSessionManager::NotifySupportRotationRegistered()
{
    if (supportRotationRegisteredListener_) {
        supportRotationRegisteredListener_();
    } else {
        TLOGE(WmsLogTag::WMS_ROTATION, "supportRotationRegisteredListener_ is null");
    }
    return WMError::WM_OK;
}

WMError SceneSessionManager::GetAllJsonProfile(AppExecFwk::ProfileType profileType, int32_t userId,
    std::vector<AppExecFwk::JsonProfileInfo>& jsonProfileInfos)
{
    if (!bundleMgr_) {
        TLOGE(WmsLogTag::WMS_COMPAT, "bundleMgr is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_COMPAT, "GetAllJsonProfile start %{public}d , %{public}d",
        static_cast<int8_t>(profileType), userId);
    bool ret = bundleMgr_->GetAllJsonProfile(profileType, userId, jsonProfileInfos);
    TLOGI(WmsLogTag::WMS_COMPAT, "GetAllJsonProfile end ret:%{public}d size:%{public}d ",
        ret, static_cast<int>(jsonProfileInfos.size()));
    if (ret) {
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    return WMError::WM_OK;
}

WMError SceneSessionManager::GetJsonProfile(AppExecFwk::ProfileType profileType, const std::string& bundleName,
    const std::string& moduleName,  int32_t userId, std::string& profileInfo)
{
    if (!bundleMgr_) {
        TLOGI(WmsLogTag::WMS_COMPAT, "bundleMgr is nullptr");
        return WMError::WM_ERROR_NULLPTR;
    }
    TLOGI(WmsLogTag::WMS_COMPAT, "GetJsonProfile start %{public}d , %{public}d , %{public}s",
        static_cast<int8_t>(profileType), userId, bundleName.c_str());
    bool ret = bundleMgr_->GetJsonProfile(profileType, bundleName, moduleName, profileInfo, userId);
    TLOGI(WmsLogTag::WMS_COMPAT, "GetJsonProfile end ret:%{public}d", ret);
    if (ret) {
        return WMError::WM_ERROR_SYSTEM_ABNORMALLY;
    }
    return WMError::WM_OK;
}

void SceneSessionManager::RegisterMinimizeAllCallback(MinimizeAllFunc&& func){
    minimizeAllFunc_ = std::move(func);
}

} // namespace OHOS::Rosen
