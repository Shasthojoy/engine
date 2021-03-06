// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_COMMON_ENGINE_H_
#define SHELL_COMMON_ENGINE_H_

#include "flutter/assets/zip_asset_store.h"
#include "flutter/glue/drain_data_pipe_job.h"
#include "flutter/lib/ui/window/platform_message.h"
#include "flutter/runtime/runtime_controller.h"
#include "flutter/runtime/runtime_delegate.h"
#include "flutter/services/engine/sky_engine.mojom.h"
#include "flutter/shell/common/rasterizer.h"
#include "lib/ftl/macros.h"
#include "lib/ftl/memory/weak_ptr.h"
#include "mojo/public/cpp/application/service_provider_impl.h"
#include "mojo/public/cpp/bindings/binding.h"
#include "mojo/public/cpp/bindings/binding_set.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "mojo/public/cpp/system/handle.h"
#include "mojo/public/interfaces/application/service_provider.mojom.h"
#include "mojo/services/asset_bundle/interfaces/asset_bundle.mojom.h"
#include "third_party/skia/include/core/SkPicture.h"

namespace blink {
class DirectoryAssetBundle;
class ZipAssetBundle;
}  // namespace blink

namespace shell {
class PlatformView;
class Animator;
using PointerDataPacket = blink::PointerDataPacket;

class Engine : public sky::SkyEngine, public blink::RuntimeDelegate {
 public:
  explicit Engine(PlatformView* platform_view);

  ~Engine() override;

  ftl::WeakPtr<Engine> GetWeakPtr();

  static void Init();

  void BeginFrame(ftl::TimePoint frame_time);

  void RunFromSource(const std::string& main,
                     const std::string& packages,
                     const std::string& bundle);

  Dart_Port GetUIIsolateMainPort();

  std::string GetUIIsolateName();

  void ConnectToEngine(mojo::InterfaceRequest<SkyEngine> request);
  void OnOutputSurfaceCreated(const ftl::Closure& gpu_continuation);
  void OnOutputSurfaceDestroyed(const ftl::Closure& gpu_continuation);
  void DispatchPlatformMessage(ftl::RefPtr<blink::PlatformMessage> message);
  void DispatchPointerDataPacket(const PointerDataPacket& packet);
  void DispatchSemanticsAction(int id, blink::SemanticsAction action);
  void SetSemanticsEnabled(bool enabled);

 private:
  // SkyEngine implementation:
  void SetServices(sky::ServicesDataPtr services) override;
  void OnViewportMetricsChanged(sky::ViewportMetricsPtr metrics) override;
  void OnLocaleChanged(const mojo::String& language_code,
                       const mojo::String& country_code) override;

  void RunFromFile(const mojo::String& main,
                   const mojo::String& packages,
                   const mojo::String& bundle) override;
  void RunFromPrecompiledSnapshot(const mojo::String& bundle_path) override;
  void RunFromBundle(const mojo::String& script_uri,
                     const mojo::String& bundle_path) override;
  void RunFromBundleAndSnapshot(const mojo::String& script_uri,
                                const mojo::String& bundle_path,
                                const mojo::String& snapshot_path) override;
  void PushRoute(const mojo::String& route) override;
  void PopRoute() override;
  void OnAppLifecycleStateChanged(sky::AppLifecycleState state) override;

  // RuntimeDelegate methods:
  void ScheduleFrame() override;
  void Render(std::unique_ptr<flow::LayerTree> layer_tree) override;
  void UpdateSemantics(std::vector<blink::SemanticsNode> update) override;
  void HandlePlatformMessage(
      ftl::RefPtr<blink::PlatformMessage> message) override;
  void DidCreateMainIsolate(Dart_Isolate isolate) override;
  void DidCreateSecondaryIsolate(Dart_Isolate isolate) override;

  void BindToServiceProvider(
      mojo::InterfaceRequest<mojo::ServiceProvider> request);

  void RunFromSnapshotStream(const std::string& script_uri,
                             mojo::ScopedDataPipeConsumerHandle snapshot);

  void StopAnimator();
  void StartAnimatorIfPossible();

  void ConfigureAssetBundle(const std::string& path);
  void ConfigureRuntime(const std::string& script_uri);

  void HandleAssetPlatformMessage(ftl::RefPtr<blink::PlatformMessage> message);

  ftl::WeakPtr<PlatformView> platform_view_;
  std::unique_ptr<Animator> animator_;

  sky::ServicesDataPtr services_;
  mojo::ServiceProviderImpl service_provider_impl_;
  mojo::ServiceProviderPtr incoming_services_;
  mojo::BindingSet<mojo::ServiceProvider> service_provider_bindings_;

  mojo::asset_bundle::AssetBundlePtr root_bundle_;
  std::unique_ptr<blink::RuntimeController> runtime_;

  std::unique_ptr<glue::DrainDataPipeJob> snapshot_drainer_;

  std::string initial_route_;
  sky::ViewportMetricsPtr viewport_metrics_;
  std::string language_code_;
  std::string country_code_;
  bool semantics_enabled_ = false;
  mojo::Binding<SkyEngine> binding_;

  ftl::RefPtr<blink::ZipAssetStore> asset_store_;
  std::unique_ptr<blink::DirectoryAssetBundle> directory_asset_bundle_;
  std::unique_ptr<blink::ZipAssetBundle> zip_asset_bundle_;

  // TODO(eseidel): This should move into an AnimatorStateMachine.
  bool activity_running_;
  bool have_surface_;

  ftl::WeakPtrFactory<Engine> weak_factory_;

  FTL_DISALLOW_COPY_AND_ASSIGN(Engine);
};

}  // namespace shell

#endif  // SHELL_COMMON_ENGINE_H_
