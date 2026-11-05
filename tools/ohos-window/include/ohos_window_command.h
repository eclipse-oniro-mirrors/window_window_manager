/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_CLAW_WINDOW_COMMAND_H
#define OHOS_ROSEN_CLAW_WINDOW_COMMAND_H

#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "dm_common.h"
#include "display_manager.h"
#include "wm_common.h"
#include "shell_command.h"
#include "zidl/scene_session_manager_lite_interface.h"

namespace OHOS {
namespace Rosen {
namespace {
const std::string TOOL_NAME = "ohos-window";

const std::string HELP_MSG =
    "ohos-window - Window management utility for manipulating windows and querying window information\n\n"
    "Usage:\n"
    "  ohos-window [command] [options]\n\n"
    "Parameters:\n"
    "  --help                    Display this help message\n\n"
    "SubCommands:\n"
    "  restore-window            restore a main window to foreground\n"
    "  list-windows              query window information\n"
    "  screenshot                capture a screenshot of the display\n"
    "\nExamples:\n"
    "  # Restore a main window to foreground\n"
    "  ohos-window restore-window --windowId 100\n\n"
    "  # List all visible windows\n"
    "  ohos-window list-windows --filter visible\n\n"
    "  # Capture a full screen screenshot\n"
    "  ohos-window screenshot\n\n";

const std::string VERSION_MSG = "1.0.0\n";

const std::string HELP_MSG_RESTORE_WINDOW =
    "ohos-window restore-window - Restore a main window to foreground\n\n"
    "Usage:\n"
    "  ohos-window restore-window [options]\n\n"
    "Parameters:\n"
    "  --help                                             Display this help message\n"
    "  --windowId <windowId>                       windowId of the session to be restored to foreground\n"
    "Examples:\n"
    "  # Restore a main window to foreground\n"
    "  ohos-window restore-window --windowId 100\n";

const std::string STRING_RESTORE_WINDOW_OK = "restore main window to foreground successfully.";
const std::string STRING_RESTORE_WINDOW_NG = "error: failed to restore main window to foreground.";

const std::string HELP_MSG_LIST_WINDOWS =
    "ohos-window list-windows - Query window information\n\n"
    "Usage:\n"
    "  ohos-window list-windows [options]\n\n"
    "Parameters:\n"
    "  --help                                           display this help message\n"
    "  --windowId <windowId>                            windowId of the only session to query\n"
    "  --displayId <displayId>                          displayId of the display to query\n"
    "  --filter <visible|excludeSystem|foreground>      window filter to query, can specify multiple filters\n"
    "  --type <ui|display|layout|meta>                  window information to return, can specify multiple filters\n"
    "\nExamples:\n"
    "  # List all visible windows\n"
    "  ohos-window list-windows --filter visible\n";

const std::string HELP_MSG_SCREENSHOT =
    "ohos-window screenshot - Capture a screenshot of the display\n\n"
    "Usage:\n"
    "  ohos-window screenshot [options]\n\n"
    "Parameters:\n"
    "  --help                                 display this help message\n"
    "  --displayId <displayId>                displayId of the display to capture (default: default display)\n"
    "  --outputPath <directory>"
    "   output file directory (default: /data/local/tmp/; custom path must be writable)\n"
    "  --rect <x,y,w,h>                       capture the specified rectangle region of the screen\n"
    "  --excludeWindowIds <id1,id2,...>"
    "   full-screen capture excluding the specified window ids (windowId semantic, cannot combine with --rect)\n"
    "  --windowId <windowId>"
    "   windowId of the window to snapshot, cannot use with --displayId, --rect and --excludeWindowIds options\n"
    "  --format <jpeg|png>                              format of image, default: jpeg\n"
    "\nBehavior notes:\n"
    "  - Default (no options): uses legacy GetScreenshot, supported on all device types.\n"
    "  - --rect: captures full screen then crops locally (GetScreenshotwithConfig), supported on all device types.\n"
    "  - --excludeWindowIds: "
    "   uses GetScreenCapture, whose service-side implementation exists only on some device types (e.g. phone).\n"
    "\nExamples:\n"
    "  # Full screen screenshot\n"
    "  ohos-window screenshot\n"
    "  # Screenshot the specified rectangle region (x=100, y=200, width=300, height=400)\n"
    "  ohos-window screenshot --rect 100,200,300,400\n"
    "  # Full screen screenshot excluding windows 100 and 200 (windowId semantic)\n"
    "  ohos-window screenshot --excludeWindowIds 100,200\n";

}  // namespace

class ClawWindowShellCommand : public ShellCommand {
public:
    ClawWindowShellCommand(int argc, char* argv[]);
    ~ClawWindowShellCommand() override
    {}

    int32_t CreateErrorInfoMap();
    WmToolErrorInfo GetErrorInfoFromCode(const int32_t code);

protected:
    virtual sptr<ISceneSessionManagerLite> GetSceneSessionManagerLiteProxy();

private:
    int32_t init() override;
    int32_t CreateCommandMap() override;

    int32_t RunAsHelpCommand();
    int32_t RunAsRestoreWindow();
    int32_t DoRestoreWindow(int32_t persistentId);

    int32_t RunAsListWindowInfo();
    bool ParseListWindowInfoOption(WindowInfoOption& infoOption);
    bool ParseWindowIdOption(const std::string& idStr, int32_t& windowId);
    bool ParseDisplayIdOption(const std::string& idStr, DisplayId& displayId);
    bool ParseWindowInfoFilterOption(const std::string& valStr, WindowInfoFilterOption& filterOption);
    bool ParseWindowInfoTypeOption(const std::string& valStr, WindowInfoTypeOption& typeOption);
    void BuildListWindowInfoResultJson(const std::vector<sptr<WindowInfo>>& infos,
        const WindowInfoTypeOption& typeOption);
    void FillWindowMetaInfoJson(const WindowMetaInfo& metaInfo, nlohmann::json& windowJson);

    int32_t RunAsScreenshot();
    bool ParseScreenshotOption(DisplayId& displayId, std::string& outputPath,
        bool& isRectSet, Media::Rect& imageRect, std::vector<NodeId>& excludeWindowIds,
        int32_t& windowId, std::string& imgFormat);
    bool ParseImageFormatOption(const std::string& valStr, std::string& imgFormat);
    bool ParseOutputPathOption(const std::string& valStr, std::string& outputDir);
    bool ParseRectOption(const std::string& rectStr, Media::Rect& imageRect);
    bool ParseExcludeIdsOption(const std::string& idsStr, std::vector<NodeId>& excludeIds);
    bool CheckScreenshotOptionConflict(bool isRectSet, const std::vector<NodeId>& excludeWindowIds,
        int32_t windowId, DisplayId& displayId);
    std::string GenerateScreenshotPath(const std::string& outputDir, const std::string& imgFormat);
    std::shared_ptr<Media::PixelMap> GetSnapshot(DisplayId displayId, bool isRectSet, const Media::Rect& imageRect,
        const std::vector<NodeId>& excludeWindowIds, int32_t windowId);
    bool SavePixelMapToFile(const std::string& filePath, Media::PixelMap& pixelMap, const std::string& imgFormat);

    std::map<int32_t, WmToolErrorInfo> errorInfoMap_;
};
}  // namespace Rosen
}  // namespace OHOS

#endif  // OHOS_ROSEN_CLAW_WINDOW_COMMAND_H