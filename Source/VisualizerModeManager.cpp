#include "VisualizerModeManager.h"
#include "VisualizerComponent.h"
#include "ModeWaveform.h"
#include "ModeFilledSpectrum.h"
#include "ModeCircular.h"
#include "ModeMirror.h"
#include "ModeParticles.h"

VisualizerModeManager::VisualizerModeManager()
{
    setOpaque(false);

    // Order matches the reference design's mode-selector row: Spectrum,
    // Waveform, Filled, Circular, Mirror, Particles.
    modes.push_back(std::make_unique<VisualizerComponent>());
    modes.push_back(std::make_unique<ModeWaveform>());
    modes.push_back(std::make_unique<ModeFilledSpectrum>());
    modes.push_back(std::make_unique<ModeCircular>());
    modes.push_back(std::make_unique<ModeMirror>());
    modes.push_back(std::make_unique<ModeParticles>());

    // Spectrum mode owns the "click plot to clear peak hold" gesture; wire it
    // through to whatever MainComponent hands us.
    if (auto* spectrum = dynamic_cast<VisualizerComponent*>(modes.front().get()))
        spectrum->onResetPeaks = [this] { if (onResetPeaks != nullptr) onResetPeaks(); };

    for (auto& m : modes)
    {
        addChildComponent(*m);
        m->setVisible(false);
    }

    modes[(size_t) activeIndex]->setVisible(true);
}

void VisualizerModeManager::prepare(double sampleRate)
{
    for (auto& m : modes)
        m->prepare(sampleRate);
}

void VisualizerModeManager::updateData(const SpectrumAnalyzer::Snapshot& snapshot,
                                        const std::vector<float>& waveform)
{
    for (auto& m : modes)
        m->updateData(snapshot, waveform);
}

juce::String VisualizerModeManager::getModeName(int index) const
{
    if (index < 0 || index >= (int) modes.size())
        return {};
    return modes[(size_t) index]->getModeName();
}

void VisualizerModeManager::setActiveIndex(int index)
{
    if (index < 0 || index >= (int) modes.size() || index == activeIndex)
        return;

    modes[(size_t) activeIndex]->setVisible(false);
    activeIndex = index;
    modes[(size_t) activeIndex]->setVisible(true);

    if (onActiveModeChanged != nullptr)
        onActiveModeChanged(activeIndex);
}

void VisualizerModeManager::resized()
{
    const auto bounds = getLocalBounds();
    for (auto& m : modes)
        m->setBounds(bounds);
}
