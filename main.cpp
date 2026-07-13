#include <algorithm>
#include <chrono>
#include <cstdio>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <RmlUi/Core.h>
#include <RmlUi_Backend.h>

static Rml::ElementText* GetReadoutText(Rml::Element* readout) {
  if (!readout) return nullptr;
  return rmlui_dynamic_cast<Rml::ElementText*>(readout->GetFirstChild());
}

class ReadoutListener : public Rml::EventListener {
public:
  Rml::Element* readout = nullptr;

  void ProcessEvent(Rml::Event& event) override {
    const Rml::String value = event.GetParameter<Rml::String>("value", "");
    if (auto* text = GetReadoutText(readout)) text->SetText(value);
  }
};

int main() {
  if (!Backend::Initialize("rmlui layout repro (" REPRO_LABEL ")", 760, 900, true)) return 1;

  Rml::SetSystemInterface(Backend::GetSystemInterface());
  Rml::SetRenderInterface(Backend::GetRenderInterface());
  Rml::Initialise();

  Rml::Context* context = Rml::CreateContext("main", Rml::Vector2i(760, 900));
  Rml::LoadFontFace(REPRO_FONT);

  Rml::ElementDocument* document = context->LoadDocument(REPRO_DOCUMENT);
  if (!document) return 1;

  document->Show();

  std::setvbuf(stdout, nullptr, _IOLBF, 0);

  ReadoutListener listener;
  if (Rml::Element* slider = document->GetElementById("native-slider")) {
    listener.readout = document->GetElementById("native-readout");
    slider->AddEventListener("change", &listener);
  }

  const int warmup_frames = 5;
  const int benchmark_mutations = 100;
  double benchmark_milliseconds = 0;
  double benchmark_maximum = 0;

  int frame_index = 0;
  int stats_frames = 0;
  double stats_milliseconds = 0;
  double stats_maximum = 0;

  while (Backend::ProcessEvents(context, nullptr, false)) {
    const bool benchmark_frame =
        frame_index >= warmup_frames && frame_index < warmup_frames + benchmark_mutations;

    if (benchmark_frame) {
      char buffer[32];
      std::snprintf(buffer, sizeof(buffer), "%.3f", 0.5 + 0.001 * (frame_index - warmup_frames));
      if (auto* text = GetReadoutText(listener.readout)) text->SetText(buffer);
    }
    const auto started = std::chrono::steady_clock::now();
    context->Update();
    const double elapsed =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started)
            .count();

    if (benchmark_frame) {
      benchmark_milliseconds += elapsed;
      benchmark_maximum = std::max(benchmark_maximum, elapsed);
    }
    if (frame_index == warmup_frames + benchmark_mutations) {
      std::printf("%s: %d text mutations, one full document layout each: %.3f ms average, %.3f ms maximum\n",
                  REPRO_LABEL, benchmark_mutations, benchmark_milliseconds / benchmark_mutations,
                  benchmark_maximum);
    }

    if (frame_index > warmup_frames + benchmark_mutations) {
      stats_milliseconds += elapsed;
      stats_maximum = std::max(stats_maximum, elapsed);
      if (++stats_frames >= 120) {
        std::printf("%s: context update %.3f ms average, %.3f ms maximum over 120 frames\n", REPRO_LABEL,
                    stats_milliseconds / stats_frames, stats_maximum);
        stats_frames = 0;
        stats_milliseconds = 0;
        stats_maximum = 0;
      }
    }

    Backend::BeginFrame();
    context->Render();
    Backend::PresentFrame();
    ++frame_index;
  }

  Rml::Shutdown();
  Backend::Shutdown();
  return 0;
}
