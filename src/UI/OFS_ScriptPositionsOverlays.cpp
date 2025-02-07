#include "OFS_ScriptPositionsOverlays.h"
#include "OpenFunscripter.h"

void FrameOverlay::DrawScriptPositionContent(const OverlayDrawingCtx& ctx) noexcept
{
    auto app = OpenFunscripter::ptr;
    auto frameTime = app->player->getFrameTime();

    float visibleFrames = ctx.visibleTime / frameTime;
    constexpr float maxVisibleFrames = 400.f;
   
    if (visibleFrames <= (maxVisibleFrames * 0.75f)) {
        //render frame dividers
        float offset = -std::fmod(ctx.offsetTime, frameTime);
        const int lineCount = visibleFrames + 2;
        int alpha = 255 * (1.f - (visibleFrames / maxVisibleFrames));
        for (int i = 0; i < lineCount; i++) {
            ctx.draw_list->AddLine(
                ctx.canvas_pos + ImVec2(((offset + (i * frameTime)) / ctx.visibleTime) * ctx.canvas_size.x, 0.f),
                ctx.canvas_pos + ImVec2(((offset + (i * frameTime)) / ctx.visibleTime) * ctx.canvas_size.x, ctx.canvas_size.y),
                IM_COL32(80, 80, 80, alpha),
                1.f
            );
        }
    }

    // time dividers
    constexpr float maxVisibleTimeDividers = 150.f;
    const float timeIntervalMs = std::round(app->player->getFps() * 0.1f) * frameTime;
    const float visibleTimeIntervals = ctx.visibleTime / timeIntervalMs;
    if (visibleTimeIntervals <= (maxVisibleTimeDividers * 0.8f)) {
        float offset = -std::fmod(ctx.offsetTime, timeIntervalMs);
        const int lineCount = visibleTimeIntervals + 2;
        int alpha = 255 * (1.f - (visibleTimeIntervals / maxVisibleTimeDividers));
        for (int i = 0; i < lineCount; i++) {
            ctx.draw_list->AddLine(
                ctx.canvas_pos + ImVec2(((offset + (i * timeIntervalMs)) / ctx.visibleTime) * ctx.canvas_size.x, 0.f),
                ctx.canvas_pos + ImVec2(((offset + (i * timeIntervalMs)) / ctx.visibleTime) * ctx.canvas_size.x, ctx.canvas_size.y),
                IM_COL32(80, 80, 80, alpha),
                3.f
            );
        }
    }
    BaseOverlay::DrawHeightLines(ctx);
    timeline->DrawAudioWaveform(ctx);
    BaseOverlay::DrawActionLines(ctx);
    BaseOverlay::DrawSecondsLabel(ctx);
    BaseOverlay::DrawScriptLabel(ctx);
 
    // out of sync line
    if (BaseOverlay::SyncLineEnable) {
        float realFrameTime = app->player->getRealCurrentPositionSeconds() - ctx.offsetTime;
        ctx.draw_list->AddLine(
            ctx.canvas_pos + ImVec2((realFrameTime / ctx.visibleTime) * ctx.canvas_size.x, 0.f),
            ctx.canvas_pos + ImVec2((realFrameTime / ctx.visibleTime) * ctx.canvas_size.x, ctx.canvas_size.y),
            IM_COL32(255, 0, 0, 255),
            1.f
        );
    }
}

void FrameOverlay::nextFrame() noexcept
{
    OpenFunscripter::ptr->player->nextFrame();
}

void FrameOverlay::previousFrame() noexcept
{
    OpenFunscripter::ptr->player->previousFrame();
}

float FrameOverlay::steppingIntervalBackward(float fromTime) noexcept
{
    return -timeline->frameTime;
}

float FrameOverlay::steppingIntervalForward(float fromTime) noexcept
{
    return timeline->frameTime;
}

void TempoOverlay::DrawSettings() noexcept
{
    BaseOverlay::DrawSettings();
    auto app = OpenFunscripter::ptr;
    auto& tempo = app->LoadedProject->Settings.tempoSettings;
    if (ImGui::InputInt("BPM", &tempo.bpm, 1, 100)) {
        tempo.bpm = std::max(1, tempo.bpm);
    }

    ImGui::DragFloat("Offset", &tempo.beatOffsetSeconds, 0.001f, -10.f, 10.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

    if (ImGui::BeginCombo("Snap", beatMultiplesStrings[tempo.measureIndex], ImGuiComboFlags_PopupAlignLeft)) {
        for (int i = 0; i < beatMultiples.size(); i++) {
            if (ImGui::Selectable(beatMultiplesStrings[i])) {
                tempo.measureIndex = i;
            }
            else if (ImGui::IsItemHovered()) {
                tempo.measureIndex = i;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Text("Interval: %dms", static_cast<int32_t>(((60.f * 1000.f) / tempo.bpm) * beatMultiples[tempo.measureIndex]));
}

void TempoOverlay::DrawScriptPositionContent(const OverlayDrawingCtx& ctx) noexcept
{
    auto app = OpenFunscripter::ptr;
    auto& tempo = app->LoadedProject->Settings.tempoSettings;
    BaseOverlay::DrawHeightLines(ctx);
    timeline->DrawAudioWaveform(ctx);
    BaseOverlay::DrawActionLines(ctx);
    BaseOverlay::DrawSecondsLabel(ctx);
    BaseOverlay::DrawScriptLabel(ctx);

    float beatTime = (60.f / tempo.bpm) * beatMultiples[tempo.measureIndex];
    int32_t visibleBeats = ctx.visibleTime / beatTime;
    int32_t invisiblePreviousBeats = ctx.offsetTime / beatTime;

#ifndef NDEBUG
    static int32_t prevInvisiblePreviousBeats = 0;
    if (prevInvisiblePreviousBeats != invisiblePreviousBeats) {
        LOGF_INFO("%d", invisiblePreviousBeats);
    }
    prevInvisiblePreviousBeats = invisiblePreviousBeats;
#endif

    float offset = -std::fmod(ctx.offsetTime, beatTime) + tempo.beatOffsetSeconds;

    const int lineCount = visibleBeats + 2;
    auto& style = ImGui::GetStyle();
    char tmp[32];

    int32_t lineOffset = tempo.beatOffsetSeconds / beatTime;
    for (int i = -lineOffset; i < lineCount - lineOffset; i++) {
        int32_t beatIdx = invisiblePreviousBeats + i;
        const int32_t thing = (int32_t)(1.f / ((beatMultiples[tempo.measureIndex] / 4.f)));
        const bool isWholeMeasure = beatIdx % thing == 0;

        ctx.draw_list->AddLine(
            ctx.canvas_pos + ImVec2(((offset + (i * beatTime)) / ctx.visibleTime) * ctx.canvas_size.x, 0.f),
            ctx.canvas_pos + ImVec2(((offset + (i * beatTime)) / ctx.visibleTime) * ctx.canvas_size.x, ctx.canvas_size.y),
            isWholeMeasure ? beatMultipleColor[tempo.measureIndex] : IM_COL32(255, 255, 255, 153),
            isWholeMeasure ? 5.f : 3.f
        );

        if (isWholeMeasure) {
            stbsp_snprintf(tmp, sizeof(tmp), "%d", thing == 0 ? beatIdx : beatIdx / thing);
            const float textOffsetX = app->settings->data().default_font_size / 2.f;
            ctx.draw_list->AddText(OpenFunscripter::DefaultFont2, app->settings->data().default_font_size * 2.f,
                ctx.canvas_pos + ImVec2((((offset + (i * beatTime)) / ctx.visibleTime) * ctx.canvas_size.x) + textOffsetX, 0.f),
                ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Text]),
                tmp
            );
        }
    }
}

static int32_t GetNextPosition(float beatTime, float currentTime, float beatOffset) noexcept
{
    float beatIdx = ((currentTime - beatOffset) / beatTime);
    beatIdx = std::floor(beatIdx);

    beatIdx += 1.f;

    int32_t newPositionMs = (beatIdx * beatTime) + (beatOffset * 1000.f);

    if (newPositionMs == std::round(currentTime)) {
        // ugh
        newPositionMs += beatTime;
    }

    return newPositionMs;
}

static int32_t GetPreviousPosition(float beatTime, float currentTime, float beatOffset) noexcept
{
    float beatIdx = ((currentTime - beatOffset) / beatTime);
    beatIdx = std::ceil(beatIdx);

    beatIdx -= 1.f;
    int32_t newPositionMs = (beatIdx * beatTime) + (beatOffset * 1000.f);

    if(newPositionMs == std::round(currentTime)) {
        // ugh
        newPositionMs -= beatTime;
    }

    return newPositionMs;
}

void TempoOverlay::nextFrame() noexcept
{
    auto app = OpenFunscripter::ptr;
    auto& tempo = app->LoadedProject->Settings.tempoSettings;

    float beatTime = (60.f / tempo.bpm) * beatMultiples[tempo.measureIndex];
    float currentTime = app->player->getCurrentPositionSecondsInterp();
    int32_t newPositionMs = GetNextPosition(beatTime, currentTime, tempo.beatOffsetSeconds);

    app->player->setPositionExact(newPositionMs);
}

void TempoOverlay::previousFrame() noexcept
{
    auto app = OpenFunscripter::ptr;
    auto& tempo = app->LoadedProject->Settings.tempoSettings;

    float beatTime = (60.f/ tempo.bpm) * beatMultiples[tempo.measureIndex];
    float currentTime = app->player->getCurrentPositionSecondsInterp();
    int32_t newPositionMs = GetPreviousPosition(beatTime, currentTime, tempo.beatOffsetSeconds);

    app->player->setPositionExact(newPositionMs);
}

float TempoOverlay::steppingIntervalForward(float fromTime) noexcept
{
    auto app = OpenFunscripter::ptr;
    auto& tempo = app->LoadedProject->Settings.tempoSettings;
    float beatTime = (60.f / tempo.bpm) * beatMultiples[tempo.measureIndex];
    return GetNextPosition(beatTime, fromTime, tempo.beatOffsetSeconds) - fromTime;
}

float TempoOverlay::steppingIntervalBackward(float fromTime) noexcept
{
    auto app = OpenFunscripter::ptr;
    auto& tempo = app->LoadedProject->Settings.tempoSettings;
    float beatTime = (60.f / tempo.bpm) * beatMultiples[tempo.measureIndex];
    return GetPreviousPosition(beatTime, fromTime, tempo.beatOffsetSeconds) - fromTime;
}