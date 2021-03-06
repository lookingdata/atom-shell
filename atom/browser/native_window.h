// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_NATIVE_WINDOW_H_
#define ATOM_BROWSER_NATIVE_WINDOW_H_

#include <map>
#include <string>
#include <vector>

#include "atom/browser/native_window_observer.h"
#include "atom/browser/ui/accelerator_util.h"
#include "base/cancelable_callback.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "brightray/browser/default_web_contents_delegate.h"
#include "brightray/browser/inspectable_web_contents_delegate.h"
#include "brightray/browser/inspectable_web_contents_impl.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/readback_types.h"
#include "native_mate/persistent_dictionary.h"
#include "ui/gfx/image/image.h"

namespace base {
class CommandLine;
}

namespace content {
class BrowserContext;
class WebContents;
struct WebPreferences;
}

namespace gfx {
class Point;
class Rect;
class Size;
}

namespace mate {
class Dictionary;
}

namespace ui {
class MenuModel;
}

namespace atom {

class AtomJavaScriptDialogManager;
struct DraggableRegion;
class WebDialogHelper;

class NativeWindow : public brightray::DefaultWebContentsDelegate,
                     public brightray::InspectableWebContentsDelegate,
                     public content::WebContentsObserver,
                     public content::NotificationObserver {
 public:
  typedef base::Callback<void(const SkBitmap& bitmap)> CapturePageCallback;

  class DialogScope {
   public:
    explicit DialogScope(NativeWindow* window)
        : window_(window) {
      if (window_ != NULL)
        window_->set_has_dialog_attached(true);
    }

    ~DialogScope() {
      if (window_ != NULL)
        window_->set_has_dialog_attached(false);
    }

   private:
    NativeWindow* window_;

    DISALLOW_COPY_AND_ASSIGN(DialogScope);
  };

  virtual ~NativeWindow();

  // Create window with existing WebContents, the caller is responsible for
  // managing the window's live.
  static NativeWindow* Create(content::WebContents* web_contents,
                              const mate::Dictionary& options);

  // Create window with new WebContents, the caller is responsible for
  // managing the window's live.
  static NativeWindow* Create(const mate::Dictionary& options);

  // Find a window from its process id and routing id.
  static NativeWindow* FromRenderView(int process_id, int routing_id);

  void InitFromOptions(const mate::Dictionary& options);

  virtual void Close() = 0;
  virtual void CloseImmediately() = 0;
  virtual void Move(const gfx::Rect& pos) = 0;
  virtual void Focus(bool focus) = 0;
  virtual bool IsFocused() = 0;
  virtual void Show() = 0;
  virtual void ShowInactive() = 0;
  virtual void Hide() = 0;
  virtual bool IsVisible() = 0;
  virtual void Maximize() = 0;
  virtual void Unmaximize() = 0;
  virtual bool IsMaximized() = 0;
  virtual void Minimize() = 0;
  virtual void Restore() = 0;
  virtual bool IsMinimized() = 0;
  virtual void SetFullScreen(bool fullscreen) = 0;
  virtual bool IsFullscreen() = 0;
  virtual void SetSize(const gfx::Size& size) = 0;
  virtual gfx::Size GetSize() = 0;
  virtual void SetContentSize(const gfx::Size& size) = 0;
  virtual gfx::Size GetContentSize() = 0;
  virtual void SetMinimumSize(const gfx::Size& size) = 0;
  virtual gfx::Size GetMinimumSize() = 0;
  virtual void SetMaximumSize(const gfx::Size& size) = 0;
  virtual gfx::Size GetMaximumSize() = 0;
  virtual void SetResizable(bool resizable) = 0;
  virtual bool IsResizable() = 0;
  virtual void SetAlwaysOnTop(bool top) = 0;
  virtual bool IsAlwaysOnTop() = 0;
  virtual void Center() = 0;
  virtual void SetPosition(const gfx::Point& position) = 0;
  virtual gfx::Point GetPosition() = 0;
  virtual void SetTitle(const std::string& title) = 0;
  virtual std::string GetTitle() = 0;
  virtual void FlashFrame(bool flash) = 0;
  virtual void SetSkipTaskbar(bool skip) = 0;
  virtual void SetKiosk(bool kiosk) = 0;
  virtual bool IsKiosk() = 0;
  virtual void SetRepresentedFilename(const std::string& filename);
  virtual std::string GetRepresentedFilename();
  virtual void SetDocumentEdited(bool edited);
  virtual bool IsDocumentEdited();
  virtual void SetMenu(ui::MenuModel* menu);
  virtual bool HasModalDialog();
  virtual gfx::NativeWindow GetNativeWindow() = 0;
  virtual void SetProgressBar(double progress) = 0;
  virtual void SetOverlayIcon(const gfx::Image& overlay,
                              const std::string& description) = 0;
  virtual void SetVisibleOnAllWorkspaces(bool visible) = 0;
  virtual bool IsVisibleOnAllWorkspaces() = 0;

  virtual bool IsClosed() const { return is_closed_; }
  virtual void OpenDevTools(bool can_dock);
  virtual void CloseDevTools();
  virtual bool IsDevToolsOpened();
  virtual void InspectElement(int x, int y);

  virtual void FocusOnWebView();
  virtual void BlurWebView();
  virtual bool IsWebViewFocused();

  // Captures the page with |rect|, |callback| would be called when capturing is
  // done.
  virtual void CapturePage(const gfx::Rect& rect,
                           const CapturePageCallback& callback);

  // Print current page.
  virtual void Print(bool silent, bool print_background);

  // Show popup dictionary.
  virtual void ShowDefinitionForSelection();

  // Toggle the menu bar.
  virtual void SetAutoHideMenuBar(bool auto_hide);
  virtual bool IsMenuBarAutoHide();
  virtual void SetMenuBarVisibility(bool visible);
  virtual bool IsMenuBarVisible();

  // The same with closing a tab in a real browser.
  //
  // Should be called by platform code when user want to close the window.
  virtual void CloseWebContents();

  // Destroy the WebContents immediately.
  virtual void DestroyWebContents();

  base::WeakPtr<NativeWindow> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  content::WebContents* GetWebContents() const;
  content::WebContents* GetDevToolsWebContents() const;

  // Called when renderer process is going to be started.
  void AppendExtraCommandLineSwitches(base::CommandLine* command_line,
                                      int child_process_id);
  void OverrideWebkitPrefs(const GURL& url, content::WebPreferences* prefs);

  // Public API used by platform-dependent delegates and observers to send UI
  // related notifications.
  void NotifyWindowClosed();
  void NotifyWindowBlur();
  void NotifyWindowFocus();
  void NotifyWindowMaximize();
  void NotifyWindowUnmaximize();
  void NotifyWindowMinimize();
  void NotifyWindowRestore();
  void NotifyWindowEnterFullScreen();
  void NotifyWindowLeaveFullScreen();

  void AddObserver(NativeWindowObserver* obs) {
    observers_.AddObserver(obs);
  }

  void RemoveObserver(NativeWindowObserver* obs) {
    observers_.RemoveObserver(obs);
  }

  bool has_frame() const { return has_frame_; }

  void set_has_dialog_attached(bool has_dialog_attached) {
    has_dialog_attached_ = has_dialog_attached;
  }

 protected:
  explicit NativeWindow(content::WebContents* web_contents,
                        const mate::Dictionary& options);

  brightray::InspectableWebContentsImpl* inspectable_web_contents() const {
    return static_cast<brightray::InspectableWebContentsImpl*>(
        inspectable_web_contents_.get());
  }

  // Called when the window needs to update its draggable region.
  virtual void UpdateDraggableRegions(
      const std::vector<DraggableRegion>& regions) = 0;

  // Implementations of content::WebContentsDelegate.
  bool ShouldCreateWebContents(
      content::WebContents* web_contents,
      int route_id,
      int main_frame_route_id,
      WindowContainerType window_container_type,
      const base::string16& frame_name,
      const GURL& target_url,
      const std::string& partition_id,
      content::SessionStorageNamespace* session_storage_namespace) override;
  content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) override;
  content::JavaScriptDialogManager* GetJavaScriptDialogManager(
      content::WebContents* source) override;
  void BeforeUnloadFired(content::WebContents* tab,
                         bool proceed,
                         bool* proceed_to_fire_unload) override;
  content::ColorChooser* OpenColorChooser(
      content::WebContents* web_contents,
      SkColor color,
      const std::vector<content::ColorSuggestion>& suggestions) override;
  void RunFileChooser(content::WebContents* web_contents,
                      const content::FileChooserParams& params) override;
  void EnumerateDirectory(content::WebContents* web_contents,
                          int request_id,
                          const base::FilePath& path) override;
  void RequestToLockMouse(content::WebContents* web_contents,
                          bool user_gesture,
                          bool last_unlocked_by_target) override;
  bool CanOverscrollContent() const override;
  void ActivateContents(content::WebContents* contents) override;
  void DeactivateContents(content::WebContents* contents) override;
  void MoveContents(content::WebContents* source,
                    const gfx::Rect& pos) override;
  void CloseContents(content::WebContents* source) override;
  bool IsPopupOrPanel(
      const content::WebContents* source) const override;
  void RendererUnresponsive(content::WebContents* source) override;
  void RendererResponsive(content::WebContents* source) override;

  // Implementations of content::WebContentsObserver.
  void RenderViewCreated(content::RenderViewHost* render_view_host) override;
  void BeforeUnloadFired(const base::TimeTicks& proceed_time) override;
  bool OnMessageReceived(const IPC::Message& message) override;

  // Implementations of content::NotificationObserver.
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  // Implementations of brightray::InspectableWebContentsDelegate.
  void DevToolsSaveToFile(const std::string& url,
                          const std::string& content,
                          bool save_as) override;
  void DevToolsAppendToFile(const std::string& url,
                            const std::string& content) override;

  // Whether window has standard frame.
  bool has_frame_;

  // Whether window is transparent.
  bool transparent_;

  // Whether window can be resized larger than screen.
  bool enable_larger_than_screen_;

  // Window icon.
  gfx::ImageSkia icon_;

 private:
  // Schedule a notification unresponsive event.
  void ScheduleUnresponsiveEvent(int ms);

  // Dispatch unresponsive event to observers.
  void NotifyWindowUnresponsive();

  // Call a function in devtools.
  void CallDevToolsFunction(const std::string& function_name,
                            const base::Value* arg1 = NULL,
                            const base::Value* arg2 = NULL,
                            const base::Value* arg3 = NULL);

  // Called when CapturePage has done.
  void OnCapturePageDone(const CapturePageCallback& callback,
                         const SkBitmap& bitmap,
                         content::ReadbackResponse response);

  // Notification manager.
  content::NotificationRegistrar registrar_;

  // Observers of this window.
  ObserverList<NativeWindowObserver> observers_;

  // The windows has been closed.
  bool is_closed_;

  // Whether node integration is enabled.
  bool node_integration_;

  // There is a dialog that has been attached to window.
  bool has_dialog_attached_;

  // Closure that would be called when window is unresponsive when closing,
  // it should be cancelled when we can prove that the window is responsive.
  base::CancelableClosure window_unresposive_closure_;

  // Web preferences.
  mate::PersistentDictionary web_preferences_;

  // The script to load before page's JavaScript starts to run.
  base::FilePath preload_script_;

  // Page's default zoom factor.
  double zoom_factor_;

  base::WeakPtrFactory<NativeWindow> weak_factory_;

  scoped_ptr<WebDialogHelper> web_dialog_helper_;
  scoped_ptr<AtomJavaScriptDialogManager> dialog_manager_;

  // Notice that inspectable_web_contents_ must be placed after dialog_manager_,
  // so we can make sure inspectable_web_contents_ is destroyed before
  // dialog_manager_, otherwise a crash would happen.
  scoped_ptr<brightray::InspectableWebContents> inspectable_web_contents_;

  // Maps url to file path, used by the file requests sent from devtools.
  typedef std::map<std::string, base::FilePath> PathsMap;
  PathsMap saved_files_;

  DISALLOW_COPY_AND_ASSIGN(NativeWindow);
};

}  // namespace atom

#endif  // ATOM_BROWSER_NATIVE_WINDOW_H_
