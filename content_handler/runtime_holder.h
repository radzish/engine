// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_CONTENT_HANDLER_RUNTIME_HOLDER_H_
#define FLUTTER_CONTENT_HANDLER_RUNTIME_HOLDER_H_

#include <fdio/namespace.h>
#include <zx/channel.h>

#include <unordered_set>

#include "dart-pkg/fuchsia/sdk_ext/fuchsia.h"
#include "flutter/assets/asset_provider.h"
#include "flutter/assets/directory_asset_bundle.h"
#include "flutter/assets/unzipper_provider.h"
#include "flutter/assets/zip_asset_store.h"
#include "flutter/content_handler/accessibility_bridge.h"
#include "flutter/flow/layers/layer_tree.h"
#include "flutter/lib/ui/window/viewport_metrics.h"
#include "flutter/runtime/runtime_controller.h"
#include "flutter/runtime/runtime_delegate.h"
#include "lib/app/cpp/application_context.h"
#include <fuchsia/cpp/component.h>
#include <fuchsia/cpp/component.h>
#include <fuchsia/cpp/modular.h>
#include "lib/fidl/cpp/binding.h"
#include "lib/fxl/functional/closure.h"
#include "lib/fxl/macros.h"
#include "lib/fxl/memory/weak_ptr.h"
#include "lib/ui/flutter/sdk_ext/src/natives.h"
#include <fuchsia/cpp/ui.h>
#include <fuchsia/cpp/ui.h>
#include <fuchsia/cpp/ui.h>

namespace flutter_runner {

class Rasterizer;

class RuntimeHolder : public blink::RuntimeDelegate,
                      public mozart::NativesDelegate,
                      public views_v1::ViewListener,
                      public input::InputListener,
                      public input::InputMethodEditorClient {
 public:
  RuntimeHolder();
  ~RuntimeHolder();

  void Init(fdio_ns_t* namespc,
            std::unique_ptr<component::ApplicationContext> context,
            fidl::InterfaceRequest<component::ServiceProvider> outgoing_services,
            std::vector<char> bundle);
  void CreateView(const std::string& script_uri,
                  fidl::InterfaceRequest<views_v1_token::ViewOwner> view_owner_request,
                  fidl::InterfaceRequest<component::ServiceProvider> services);

  Dart_Port GetUIIsolateMainPort();
  std::string GetUIIsolateName();

  int32_t return_code() { return return_code_; }

  void SetMainIsolateShutdownCallback(std::function<void()> callback);

 private:
  // |blink::RuntimeDelegate| implementation:
  std::string DefaultRouteName() override;
  void ScheduleFrame(bool regenerate_layer_tree = true) override;
  void Render(std::unique_ptr<flow::LayerTree> layer_tree) override;
  void UpdateSemantics(blink::SemanticsNodeUpdates update) override;
  void HandlePlatformMessage(
      fxl::RefPtr<blink::PlatformMessage> message) override;
  void DidCreateMainIsolate(Dart_Isolate isolate) override;
  void DidShutdownMainIsolate() override;

  // |mozart::NativesDelegate| implementation:
  views_v1::View* GetMozartView() override;

  // |input::InputListener| implementation:
  void OnEvent(input::InputEvent event,
               OnEventCallback callback) override;

  // |views_v1::ViewListener| implementation:
  void OnPropertiesChanged(
      views_v1::ViewProperties properties,
      OnPropertiesChangedCallback callback) override;

  // |input::InputMethodEditorClient| implementation:
  void DidUpdateState(input::TextInputState state,
                      input::InputEventPtr event) override;
  void OnAction(input::InputMethodAction action) override;

  fxl::WeakPtr<RuntimeHolder> GetWeakPtr();

  void InitRootBundle(std::vector<char> bundle);
  blink::UnzipperProvider GetUnzipperProviderForRootBundle();
  bool HandleAssetPlatformMessage(blink::PlatformMessage* message);
  bool GetAssetAsBuffer(const std::string& name, std::vector<uint8_t>* data);
  bool HandleTextInputPlatformMessage(blink::PlatformMessage* message);
  bool HandleFlutterPlatformMessage(blink::PlatformMessage* message);

  void InitDartIoInternal();
  void InitFuchsia();
  void InitZircon();
  void InitScenicInternal();

  void PostBeginFrame();
  void BeginFrame();
  void OnFrameComplete();
  void OnRedrawFrame();
  void Invalidate();

  fdio_ns_t* namespc_;
  int dirfd_;
  std::unique_ptr<component::ApplicationContext> context_;
  fidl::InterfaceRequest<component::ServiceProvider> outgoing_services_;
  std::vector<char> root_bundle_data_;
  // TODO(zarah): Remove asset_store_ when flx is completely removed
  fxl::RefPtr<blink::ZipAssetStore> asset_store_;
  fxl::RefPtr<blink::AssetProvider> asset_provider_;
  void* dylib_handle_ = nullptr;
  std::unique_ptr<Rasterizer> rasterizer_;
  std::unique_ptr<blink::RuntimeController> runtime_;
  blink::ViewportMetrics viewport_metrics_;
  views_v1::ViewManagerPtr view_manager_;
  fidl::Binding<views_v1::ViewListener> view_listener_binding_;
  fidl::Binding<input::InputListener> input_listener_binding_;
  input::InputConnectionPtr input_connection_;
  views_v1::ViewPtr view_;
  std::unordered_set<int> down_pointers_;
  input::InputMethodEditorPtr input_method_editor_;
  fidl::Binding<input::InputMethodEditorClient> text_input_binding_;
  int current_text_input_client_ = 0;
  fxl::TimePoint last_begin_frame_time_;
  bool frame_outstanding_ = false;
  bool frame_scheduled_ = false;
  bool frame_rendering_ = false;
  int32_t return_code_ = 0;

  fxl::WeakPtrFactory<RuntimeHolder> weak_factory_;

  std::unique_ptr<AccessibilityBridge> accessibility_bridge_;

  std::function<void()> main_isolate_shutdown_callback_;

  modular::ClipboardPtr clipboard_;

  FXL_DISALLOW_COPY_AND_ASSIGN(RuntimeHolder);
};

}  // namespace flutter_runner

#endif  // FLUTTER_CONTENT_HANDLER_RUNTIME_HOLDER_H_
